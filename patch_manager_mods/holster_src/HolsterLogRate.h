// HolsterLogRate.h -- pure log rate-limit helpers of the holstered-weapons mod (no Win32, host-testable; tests in holster_lograte_test.cpp).
// Quiet mode (holster.ini log_verbose=0, the default) logs far fewer COMB/ATTACH/ANIMRES/PLACE/PLACEREFUSE lines than the old per-sample
// behaviour (log_verbose=1). The state is plain (non-atomic): the handlers run on the game thread and a lost count only shifts one sample.
//   KeyCounts      key (creature / gob pointer) -> hit count, fixed capacity; a full table reports 0 for new keys (= "do not log")
//   keyedWant      first N hits of a key, then every Mth
//   PlaceTable     per-gob PLACE sampler: first 3, every 200th, any change of the (cat,left,src) signature, or a change of the placement
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
    int n;
    KeyCounts() : n(0) {
        for (int i = 0; i < Cap; ++i) { key[i] = 0; cnt[i] = 0; }
    }
};

// Increments and returns the key's hit count (1 on the first hit). 0 = key 0 or the table is full and the key is new.
template <int Cap>
inline uint32_t keyBump(KeyCounts<Cap>& t, uintptr_t k) {
    if (k == 0) return 0;
    for (int i = 0; i < t.n; ++i)
        if (t.key[i] == k) return ++t.cnt[i];
    if (t.n >= Cap) return 0;
    t.key[t.n] = k;
    t.cnt[t.n] = 1;
    return t.cnt[t.n++];
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
    uint32_t overflow;   // placements of gobs past the table: only every kEvery-th
    PlaceTable() : n(0), overflow(0) {
        for (int i = 0; i < kCap; ++i) g[i] = PlaceGobState{0, 0, 0, 0, 0.f, 0.f};
    }
};

// Call once per successful placement. sig = signature of the gob's (cat, left, src); disp/ang = placement delta. Returns true to log.
inline bool placeWant(PlaceTable& t, uintptr_t gob, uint32_t sig, float disp, float ang) {
    PlaceGobState* s = nullptr;
    for (int i = 0; i < t.n; ++i)
        if (t.g[i].gob == gob) { s = &t.g[i]; break; }
    if (!s) {
        if (t.n >= PlaceTable::kCap) return (++t.overflow % PlaceTable::kEvery) == 0;
        s = &t.g[t.n++];
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
    int n;
    RefuseTable() : n(0) {
        for (int i = 0; i < kReasons; ++i) total[i] = 0;
        for (int i = 0; i < kCap; ++i) { gob[i] = 0; reason[i] = 0; }
    }
};

// Counts a refusal (*count = running total of that reason) and returns true when it should be logged: first hit of the reason, first hit of a
// new (reason, gob) pair (first 64 pairs), or every 10000th hit of the reason (summary).
inline bool refuseWant(RefuseTable& t, int r, uintptr_t g, uint32_t* count) {
    if (r < 0 || r >= RefuseTable::kReasons) r = 0;
    const uint32_t c = ++t.total[r];
    if (count) *count = c;
    if (c == 1 || (c % RefuseTable::kSummary) == 0) {
        if (c == 1 && t.n < RefuseTable::kCap) { t.gob[t.n] = g; t.reason[t.n++] = r; }
        return true;
    }
    for (int i = 0; i < t.n; ++i)
        if (t.reason[i] == r && t.gob[i] == g) return false;
    if (t.n >= RefuseTable::kCap) return false;
    t.gob[t.n] = g;
    t.reason[t.n++] = r;
    return true;
}

}  // namespace hs
