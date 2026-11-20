// HolsterLogRate.h -- pure log rate-limit helpers of the holstered-weapons mod (no Win32, host-testable; tests in holster_lograte_test.cpp).
// Quiet mode (holster.ini log_verbose=0, the default) logs far fewer COMB/ATTACH/ANIMRES/PLACE/PLACEREFUSE lines than the old per-sample
// behaviour (log_verbose=1). The state is plain (non-atomic): the handlers run on the game thread and a lost count only shifts one sample.
//   KeyCounts      key (creature / gob pointer) -> hit count, fixed capacity; a full table evicts the oldest slot (ring) so a new key always starts at 1
//   combLogWanted  COMB: edges (old != new) are always logged, only edge=0 lines are rate-limited
//   creatureKey    maps a null pointer to a stable non-zero key so it is counted like any other
//   keyedWant      first N hits of a key, then every Mth
//   PlaceTable     per-gob PLACE sampler (full table: oldest gob evicted): first 3, every 200th, any change of the (cat,left,src) signature, or a change of the placement
//                  delta (displacement length / rotation angle between the engine pose and the written pose), at most 20 change lines per gob
//   RefuseTable    PLACEREFUSE: once per distinct (reason, gob), the first hit of each reason, and a summary every 10000th hit of a reason
#pragma once
#include <stdint.h>
#include <math.h>

namespace hs {

template <int Cap>
struct KeyCounts {
    uintptr_t key[Cap];
    uint32_t cnt[Cap];
    int n, next;   // n = used slots, next = ring slot replaced when full
    KeyCounts() : n(0), next(0) {
        for (int i = 0; i < Cap; ++i) { key[i] = 0; cnt[i] = 0; }
    }
};

inline bool keyedWant(uint32_t count, uint32_t first, uint32_t every);

// Increments and returns the key's hit count (1 on the first hit). 0 = key 0. A full table replaces the oldest slot (ring), so a new key is
// always tracked from 1 (the evicted key restarts at 1 if it returns).
template <int Cap>
inline uint32_t keyBump(KeyCounts<Cap>& t, uintptr_t k) {
    if (k == 0) return 0;
    for (int i = 0; i < t.n; ++i)
        if (t.key[i] == k) return ++t.cnt[i];
    int slot;
    if (t.n < Cap) slot = t.n++;
    else { slot = t.next; t.next = (t.next + 1) % Cap; }
    t.key[slot] = k;
    t.cnt[slot] = 1;
    return 1;
}

// Null pointers (e.g. ATTACH with no creature) share one stable key instead of never being counted.
inline uintptr_t creatureKey(uintptr_t p) { return p ? p : static_cast<uintptr_t>(1); }

// COMB decision. edge = the combat flag really changes (old != new, or old unreadable): always logged. Non-edge repeats: first 3 per creature
// (callCount = running count of non-edge calls), then every 10th. verbose = log_verbose=1: everything.
inline bool combLogWanted(bool edge, uint32_t callCount, bool verbose) {
    return verbose || edge || keyedWant(callCount, 3, 10);
}

// True for hit counts 1..first and then every `every`-th (0 = never again). Count 0 (untracked key) is false.
inline bool keyedWant(uint32_t count, uint32_t first, uint32_t every) {
    if (count == 0) return false;
    if (count <= first) return true;
    return every != 0 && (count % every) == 0;
}

inline uint32_t fnv1a(const char* s, uint32_t h = 2166136261u) {
    for (; s && *s; ++s) h = (h ^ static_cast<uint8_t>(*s)) * 16777619u;
    return h;
}

// Rotation angle in degrees between two unit quaternions (|dot|, so q and -q are the same rotation).
inline float quatAngleDeg(const float a[4], const float b[4]) {
    float d = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
    d = fabsf(d);
    if (d > 1.0f) d = 1.0f;
    return 2.0f * acosf(d) * 57.29578f;
}

inline float vecDist(const float a[3], const float b[3]) {
    const float x = a[0] - b[0], y = a[1] - b[1], z = a[2] - b[2];
    return sqrtf(x * x + y * y + z * z);
}

struct PlaceGobState {
    uintptr_t gob;
    uint32_t count, changeLines, sig;
    float disp, ang;   // values at the last logged line
};

struct PlaceTable {
    static const int kCap = 64;
    static const uint32_t kFirst = 3, kEvery = 200, kMaxChange = 20;
    static constexpr float kDispTol = 0.005f, kAngTol = 1.0f;
    PlaceGobState g[kCap];
    int n;
    int next;   // ring slot replaced when the table is full
    PlaceTable() : n(0), next(0) {
        for (int i = 0; i < kCap; ++i) g[i] = PlaceGobState{0, 0, 0, 0, 0.f, 0.f};
    }
};

// Call once per successful placement. sig = signature of the gob's (cat, left, src); disp/ang = placement delta. Returns true to log.
inline bool placeWant(PlaceTable& t, uintptr_t gob, uint32_t sig, float disp, float ang) {
    PlaceGobState* s = nullptr;
    for (int i = 0; i < t.n; ++i)
        if (t.g[i].gob == gob) { s = &t.g[i]; break; }
    if (!s) {
        int slot;
        if (t.n < PlaceTable::kCap) slot = t.n++;
        else { slot = t.next; t.next = (t.next + 1) % PlaceTable::kCap; }
        s = &t.g[slot];
        *s = PlaceGobState{gob, 0, 0, sig, disp, ang};
    }
    const uint32_t c = ++s->count;
    bool log = c <= PlaceTable::kFirst || (c % PlaceTable::kEvery) == 0;
    if (!log) {
        if (sig != s->sig) log = true;
        else if (s->changeLines < PlaceTable::kMaxChange && (fabsf(disp - s->disp) > PlaceTable::kDispTol || fabsf(ang - s->ang) > PlaceTable::kAngTol)) {
            log = true;
            ++s->changeLines;
        }
    }
    if (log) { s->sig = sig; s->disp = disp; s->ang = ang; }
    return log;
}

struct RefuseTable {
    static const int kReasons = 16, kCap = 64;
    static const uint32_t kSummary = 10000;
    uint32_t total[kReasons];
    uintptr_t gob[kCap];
    int reason[kCap];
    int n, next;   // next = ring slot replaced when the pair table is full
    RefuseTable() : n(0), next(0) {
        for (int i = 0; i < kReasons; ++i) total[i] = 0;
        for (int i = 0; i < kCap; ++i) { gob[i] = 0; reason[i] = 0; }
    }
};

// Counts a refusal (*count = running total of that reason) and returns true when it should be logged: first hit of the reason, first hit of a
// new (reason, gob) pair (oldest of 64 evicted), or every 10000th hit of the reason (summary).
inline void refuseAdd(RefuseTable& t, int r, uintptr_t g) {
    int slot;
    if (t.n < RefuseTable::kCap) slot = t.n++;
    else { slot = t.next; t.next = (t.next + 1) % RefuseTable::kCap; }
    t.gob[slot] = g;
    t.reason[slot] = r;
}

inline bool refuseWant(RefuseTable& t, int r, uintptr_t g, uint32_t* count) {
    if (r < 0 || r >= RefuseTable::kReasons) r = 0;
    const uint32_t c = ++t.total[r];
    if (count) *count = c;
    if (c == 1 || (c % RefuseTable::kSummary) == 0) {
        if (c == 1) refuseAdd(t, r, g);
        return true;
    }
    for (int i = 0; i < t.n; ++i)
        if (t.reason[i] == r && t.gob[i] == g) return false;
    refuseAdd(t, r, g);   // a new pair always logs; the oldest pair is evicted when full
    return true;
}

}  // namespace hs
