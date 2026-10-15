// NwCore.h -- the npc-world core: consumer registry, one-visit state (salt, POP-STABLE latch, seen tags), spawn gating and the
// slow-consumer guard. Pure C++ (no Win32, no engine reads; clock, entropy and the native spawn call are injected), so every rule is
// host-tested (nw_registry_test.cpp, nw_world_test.cpp). Used by NpcWorldProvider.cpp. Design: doc 40 section 12.
// The gate and orientation helpers are copies of k1_spawn_probe_src/SpawnLogic.h (the probe becomes a client in V2a).
#pragma once
#include "NpcWorld.h"
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <string>

namespace nw {

constexpr int MaxConsumers = 16;
constexpr uint32_t SlowThresholdUs = 5000;   // a callback slower than this (strictly) is reported; dispatching continues
constexpr int StableTicks = 30;              // consecutive engine-add-free ticks that make an area "stable" (area-hook-probe 0.2.0: 7/7 loads)
constexpr int MaxSeenTags = 256;
constexpr int TagChars = 32;                 // tags compare on at most 32 chars
constexpr int ResChars = 16;                 // resrefs compare on at most 16 chars
constexpr int DefaultSpawnCap = 4;
constexpr int HardSpawnCap = 32;             // ini values above this are clamped

inline char lowerAscii(char c) { return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c; }

// Case-insensitive compare of at most n chars; stops at the first NUL in either string and never reads index n or later.
inline bool ciEqualsN(const char* a, const char* b, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        const char ca = a[i], cb = b[i];
        if (ca == '\0' || cb == '\0') return ca == cb;
        if (lowerAscii(ca) != lowerAscii(cb)) return false;
    }
    return true;
}
inline bool resrefEquals(const char* a, const char* b) { return a && b && ciEqualsN(a, b, ResChars); }

struct Vec3 { float x, y, z; };
// Unit facing vector in the XY plane (cos f, sin f, 0); a non-finite facing gives (1, 0, 0).
inline Vec3 orientationFromFacing(float f) {
    if (!isfinite(f)) return Vec3{1.0f, 0.0f, 0.0f};
    return Vec3{cosf(f), sinf(f), 0.0f};
}

// Murmur3 finalizer, same as npc_diversity's Fmix32.
inline uint32_t fmix32(uint32_t h) {
    h ^= h >> 16; h *= 0x85EBCA6Bu; h ^= h >> 13; h *= 0xC2B2AE35u; h ^= h >> 16;
    return h;
}
// Per-visit salt: mixes the load counter with injected entropy (QueryPerformanceCounter low bits in the provider). Never 0.
inline uint32_t makeVisitSalt(int32_t loadSeq, uint32_t entropy) {
    const uint32_t s = fmix32(entropy ^ 0x5BD1E995u ^ fmix32(uint32_t(loadSeq) * 0x9E3779B9u));
    return s != 0 ? s : 1u;
}

// snprintf-style copy of s: always NUL-terminates when cap > 0; returns s.size() (>= cap = truncated); 0 for the empty string.
inline int32_t copyText(const std::string& s, char* buf, uint32_t cap) {
    if (buf && cap) {
        const size_t n = s.size() < size_t(cap) - 1 ? s.size() : size_t(cap) - 1;
        memcpy(buf, s.data(), n);
        buf[n] = 0;
    }
    return int32_t(s.size());
}

inline bool singleKind(int32_t kind) { return (kind & K2NW_ALL) != 0 && (kind & ~K2NW_ALL) == 0 && (kind & (kind - 1)) == 0; }

struct Entry { int32_t token; int32_t mask; K2NwCallback fn; void* user; const char* owner; };

// Injectable timing: now() returns a free-running microsecond counter (wraps at 2^32, the subtraction handles it); slow() is called after
// a callback that took longer than thresholdUs. now == nullptr disables the guard.
struct Timing {
    uint32_t (*now)(void* ctx) = nullptr;
    void (*slow)(void* ctx, const Entry& e, uint32_t us, int32_t kind) = nullptr;
    void* ctx = nullptr;
    uint32_t thresholdUs = SlowThresholdUs;
};

struct Dispatcher {
    Entry e[MaxConsumers] = {};
    int n = 0;
    int32_t nextToken = 1;
    Timing timing;
    uint32_t slowCalls = 0;

    // Token > 0, or -1 (null fn / no valid kind bit), -2 (table full). Unknown mask bits are ignored (a newer consumer on an older provider).
    int32_t add(int32_t mask, K2NwCallback fn, void* user, const char* owner) {
        mask &= K2NW_ALL;
        if (!fn || mask == 0) return -1;
        if (n >= MaxConsumers) return -2;
        e[n++] = { nextToken, mask, fn, user, owner ? owner : "" };
        return nextToken++;
    }
    bool remove(int32_t token) {
        for (int i = 0; i < n; ++i)
            if (e[i].token == token) {
                for (int k = i + 1; k < n; ++k) e[k - 1] = e[k];
                --n;
                return true;
            }
        return false;
    }
    bool live(int32_t token) const { for (int i = 0; i < n; ++i) if (e[i].token == token) return true; return false; }
    int count(int32_t kind) const { int c = 0; for (int i = 0; i < n; ++i) if (e[i].mask & kind) ++c; return c; }

    // Calls every consumer whose mask has ev.kind, in registration order; returns how many were called (0 for an invalid kind).
    // Iterates a snapshot: a consumer registered by a callback waits for the next event; one unregistered by a callback is skipped.
    int dispatch(const K2NwEvent& ev) {
        if (!singleKind(ev.kind) || n == 0) return 0;
        Entry snap[MaxConsumers];
        const int cnt = n;
        memcpy(snap, e, sizeof(Entry) * cnt);
        int called = 0;
        for (int i = 0; i < cnt; ++i) {
            if (!(snap[i].mask & ev.kind) || !live(snap[i].token)) continue;
            const uint32_t t0 = timing.now ? timing.now(timing.ctx) : 0;
            snap[i].fn(&ev, snap[i].user);
            ++called;
            if (timing.now) {
                const uint32_t us = timing.now(timing.ctx) - t0;
                if (us > timing.thresholdUs) { ++slowCalls; if (timing.slow) timing.slow(timing.ctx, snap[i], us, ev.kind); }
            }
        }
        return called;
    }
};

// ---- creature tag read (host-testable: the provider supplies the memory reader) --------------------------------------------------
// Where the tag lives in a creature object at AddToArea time:
//   primary  : the dword at c+0x18 is a char* to the NUL-terminated tag. Seen live 10-15 (doc 40 13.6): an engine-restored creature
//              (oid 0x348) matched its tag through this pointer in k1-spawn-probe's TAGSCAN. Verified for two creatures only.
//   fallback : [c+0x24] -> +0x18 -> char* (the first guess, doc 38). It was empty on every engine add in the 10-15 run.
// A candidate is accepted only when the string is readable, non-empty and holds printable ASCII only (a wrong pointer almost never does).
struct MemReader {
    virtual bool u32(uintptr_t addr, unsigned* v) = 0;
    // Copies the string at p into out (< cap bytes, NUL-terminated). nonPrintable = number of bytes outside 0x20..0x7e before the end.
    virtual bool cstr(uintptr_t p, char* out, int cap, int* nonPrintable) = 0;
    virtual ~MemReader() {}
};

enum TagVia { TagNone = 0, TagVia18 = 1, TagVia24 = 2 };

// out must hold TagChars+1 bytes. Returns which path produced the tag; out = "" when none did.
inline TagVia readCreatureTag(MemReader& m, uintptr_t c, char* out) {
    out[0] = 0;
    unsigned p = 0;
    int bad = 0;
    if (m.u32(c + 0x18, &p) && p && m.cstr(p, out, TagChars + 1, &bad) && bad == 0 && out[0]) return TagVia18;
    out[0] = 0;
    unsigned holder = 0;
    bad = 0;
    if (m.u32(c + 0x24, &holder) && holder && m.u32(holder + 0x18, &p) && p && m.cstr(p, out, TagChars + 1, &bad) && bad == 0 && out[0]) return TagVia24;
    out[0] = 0;
    return TagNone;
}

// What the provider read from the engine at one AddToArea. Strings must be NUL-terminated (the provider guarantees it).
struct CreatureInfo {
    void* creature = nullptr;
    uint32_t oid = 0;          // 0 = unreadable
    const char* tag = "";      // "" = unreadable / none
    float x = 0, y = 0, z = 0;
    void* areaPtr = nullptr;   // the area argument of AddToArea
    const char* areaRes = "";  // area resref read from areaPtr+0x110, "" = unreadable
};

// The native spawn call, injected. Returns the new object id, or 0 when the engine step failed.
struct SpawnEngine {
    uint32_t (*spawn)(void* ctx, void* area, const char* resref, float x, float y, float z, float facing) = nullptr;
    void* ctx = nullptr;
};

// Why a Spawn() was refused (for the provider log and the tests).
enum class SpawnResult : int { Ok = 0, Disabled, NoEngine, NotStable, BadArgs, Reentrant, TagPresent, CapReached, EngineFailed };

class World {
public:
    Dispatcher disp;
    SpawnEngine eng;
    bool enabled = true;          // false: hooks stay passive (no events, no salt, no spawns)
    int spawnCap = DefaultSpawnCap;
    void (*preStable)(const World&) = nullptr;   // called right before the AREA_STABLE dispatch (the provider logs there, ahead of any spawn lines)

    // ---- hook entry points (all on the engine's main thread) ----
    // H2 LoadModuleInProgress: a new area load. Resets the per-visit state and picks the new salt.
    void onLoad(uint32_t entropy) {
        if (!enabled) return;
        ++loadSeq_;
        gateReset();
        area_[0] = 0;
        areaPtr_ = nullptr;
        seenCount_ = 0;
        seenOverflow_ = false;
        spawnedThisVisit_ = 0;
        spawning_ = false;
        visitSalt_ = makeVisitSalt(loadSeq_, entropy);
        K2NwEvent ev = baseEvent(K2NW_AREA_LOAD);
        disp.dispatch(ev);
    }

    // H3 AddToArea entry (engine adds and, re-entrantly, the library's own spawns).
    void onCreatureAdded(const CreatureInfo& ci) {
        if (!enabled) return;
        const bool lib = spawning_;
        if (ci.areaPtr && ci.areaRes && ci.areaRes[0]) {
            areaPtr_ = ci.areaPtr;
            if (!resrefEquals(area_, ci.areaRes)) {   // first add of the load, or an area change without a load event: re-arm the gate
                setArea(ci.areaRes);
                gateReset();
                spawnedThisVisit_ = 0;
            }
        }
        if (!lib) ++addsSinceTick_;   // after the re-arm, so the add that armed the area still keeps the next tick from counting as quiet
        if (ci.tag && ci.tag[0]) rememberTag(ci.tag);
        K2NwEvent ev = baseEvent(K2NW_CREATURE_ADDED);
        ev.creature = ci.creature;
        ev.oid = ci.oid;
        ev.tag = ci.tag ? ci.tag : "";
        ev.x = ci.x; ev.y = ci.y; ev.z = ci.z;
        ev.origin = lib ? K2NW_ORIGIN_LIB : K2NW_ORIGIN_ENGINE;
        disp.dispatch(ev);
    }

    // H1 MainLoop entry, every tick.
    void onTick() {
        if (!enabled) return;
        ++tick_;
        if (areaPtr_ && area_[0] && !stableFired_) {
            if (addsSinceTick_ != 0) stableRun_ = 0;
            else if (stableRun_ < StableTicks) ++stableRun_;
            if (stableRun_ >= StableTicks) {
                stableFired_ = true;
                ++visitSeq_;
                addsSinceTick_ = 0;
                if (preStable) preStable(*this);
                K2NwEvent ev = baseEvent(K2NW_AREA_STABLE);
                disp.dispatch(ev);
            }
        }
        addsSinceTick_ = 0;
        if (disp.count(K2NW_TICK) > 0) {
            K2NwEvent ev = baseEvent(K2NW_TICK);
            disp.dispatch(ev);
        }
    }

    // ---- services ----
    SpawnResult spawn(const char* resref, const char* tag, float x, float y, float z, float facing, uint32_t* oidOut) {
        if (oidOut) *oidOut = 0;
        if (!enabled) return SpawnResult::Disabled;
        if (!eng.spawn) return SpawnResult::NoEngine;
        if (spawning_) return SpawnResult::Reentrant;
        if (!resref || !resref[0] || !areaPtr_ || !area_[0]) return SpawnResult::BadArgs;
        if (!stableFired_) return SpawnResult::NotStable;
        if (tag && tag[0] && tagPresent(tag)) return SpawnResult::TagPresent;
        if (spawnedThisVisit_ >= effectiveCap()) return SpawnResult::CapReached;
        spawning_ = true;
        const uint32_t oid = eng.spawn(eng.ctx, areaPtr_, resref, x, y, z, facing);
        spawning_ = false;
        if (!oid) return SpawnResult::EngineFailed;
        ++spawnedThisVisit_;
        if (tag && tag[0]) rememberTag(tag);
        if (oidOut) *oidOut = oid;
        return SpawnResult::Ok;
    }

    bool tagPresent(const char* tag) const {
        if (!tag || !tag[0]) return false;
        for (int i = 0; i < seenCount_; ++i)
            if (ciEqualsN(seen_[i], tag, TagChars)) return true;
        return seenOverflow_;   // unknown counts as present
    }
    uint32_t visitSalt() const { return visitSalt_; }
    int32_t visitSeq() const { return visitSeq_; }
    int32_t loadSeq() const { return loadSeq_; }
    uint32_t tick() const { return tick_; }
    int32_t spawnedCount() const { return spawnedThisVisit_; }
    int32_t effectiveCap() const { return spawnCap < 0 ? 0 : (spawnCap > HardSpawnCap ? HardSpawnCap : spawnCap); }
    int32_t currentArea(char* buf, uint32_t cap) const { return copyText(area_, buf, cap); }
    const char* area() const { return area_; }
    bool stable() const { return stableFired_; }
    int stableRun() const { return stableRun_; }
    int seenTags() const { return seenCount_; }
    bool seenOverflow() const { return seenOverflow_; }
    bool spawning() const { return spawning_; }

private:
    int32_t loadSeq_ = 0, visitSeq_ = 0;
    uint32_t visitSalt_ = 0, tick_ = 0;
    char area_[ResChars + 1] = {};
    void* areaPtr_ = nullptr;
    int stableRun_ = 0;
    bool stableFired_ = false;
    int addsSinceTick_ = 0;
    char seen_[MaxSeenTags][TagChars + 1] = {};
    int seenCount_ = 0;
    bool seenOverflow_ = false;
    int32_t spawnedThisVisit_ = 0;
    bool spawning_ = false;

    void gateReset() { stableRun_ = 0; stableFired_ = false; addsSinceTick_ = 0; }
    void setArea(const char* res) {
        size_t n = 0;
        while (n < ResChars && res[n]) { area_[n] = res[n]; ++n; }
        area_[n] = 0;
    }
    void rememberTag(const char* tag) {
        for (int i = 0; i < seenCount_; ++i)
            if (ciEqualsN(seen_[i], tag, TagChars)) return;
        if (seenCount_ >= MaxSeenTags) { seenOverflow_ = true; return; }
        size_t n = 0;
        while (n < TagChars && tag[n]) { seen_[seenCount_][n] = tag[n]; ++n; }
        seen_[seenCount_][n] = 0;
        ++seenCount_;
    }
    K2NwEvent baseEvent(int32_t kind) const {
        K2NwEvent ev = {};
        ev.struct_size = sizeof(K2NwEvent);
        ev.kind = kind;
        ev.load_seq = loadSeq_;
        ev.visit_seq = visitSeq_;
        ev.visit_salt = visitSalt_;
        ev.tick = tick_;
        ev.area = area_;
        ev.area_ptr = areaPtr_;
        ev.tag = "";
        return ev;
    }
};

} // namespace nw
