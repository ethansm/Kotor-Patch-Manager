// RangedAiLogic.h -- the pure (host-testable, no <windows.h>) helpers of ranged-ai: the stance mask parser and the
// E-H1/E-H3 weapon range test. Same semantics as cai::parseMask / cai::inWeaponRange in
// companion_ai_src/CompanionAiLogic.h (KPM builds each patch dir alone, so this is a copy; rai_logic_test.cpp is the
// drift guard). Host tests: rai_logic_test.cpp. Used by RangedAI.cpp (one translation unit).
#pragma once
#include <stdio.h>

namespace rai {

// "11,12,16" -> bit mask (values 0..31). F27: null-safe, and a digit run is capped at 1000 (no int overflow; such a
// value is >= 32 and ignored).
inline unsigned parseMask(const char* s) {
    unsigned m = 0;
    for (const char* p = s; p && *p;) {
        if (*p >= '0' && *p <= '9') { int v = 0; while (*p >= '0' && *p <= '9') { v = v * 10 + (*p++ - '0'); if (v > 1000) v = 1000; } if (v < 32) m |= 1u << v; }
        else ++p;
    }
    return m;
}

// E-H3 range test: same formula as E-H1's inline test (a zero/garbage weapon range never counts as in range)
inline bool inWeaponRange(float maxR, float dist2) {
    float lim = maxR + 0.1f; lim *= lim;
    return maxR > 1.0f && dist2 <= lim;
}

// ---- logging helpers (0.4.1) ----

// Per-creature slot lookup in a small fixed array (N a power of two): exact id, else a free slot, else the id's hash slot (reset, so a
// colliding id never inherits another creature's throttle state). S needs `int id` and a default constructor.
template <typename S, int N>
S* slotFor(S (&arr)[N], int id) {
    for (auto& s : arr) if (s.id == id) return &s;
    for (auto& s : arr) if (!s.id) { s = S{}; s.id = id; return &s; }
    S* s = &arr[static_cast<unsigned>(id) & (N - 1)];
    *s = S{}; s->id = id;
    return s;
}

// Decision-line throttle (H1 / H2): the first `firstN` lines of a creature, then a changed signature, then every `gateMs` (strictly more).
inline bool shouldLogDecision(int logged, int firstN, unsigned sig, unsigned lastSig, unsigned now, unsigned last, unsigned gateMs) {
    return logged < firstN || sig != lastSig || now - last > gateMs;
}
// H2 time gate: 2 s normally, 250 ms with advanced debug logging.
inline unsigned retargetGateMs(bool debugLog) { return debugLog ? 250u : 2000u; }
// H2 decision signature: gate, stance (8 bits, -1 -> 0xFF), ranged, los != 0.
inline unsigned retargetSig(bool gate, int stance, int ranged, int los) {
    return (gate ? 1u : 0u) | ((static_cast<unsigned>(stance) & 0xFFu) << 1) | ((ranged != 0 ? 1u : 0u) << 9) | ((los != 0 ? 1u : 0u) << 10);
}

// Change-only line with a minimum gap (debug D lines, H3 style): first time, or a changed signature at least `gateMs` after the last line.
inline bool shouldLogChange(bool seen, unsigned sig, unsigned lastSig, unsigned now, unsigned last, unsigned gateMs) {
    return !seen || (sig != lastSig && now - last >= gateMs);
}

// A message budget: Ok while used < cap, one Marker call when first exhausted, Drop afterwards.
enum class Take { Ok, Marker, Drop };
struct LineBudget {
    int used = 0; bool marked = false;
    Take take(int cap) {
        if (used < cap) { ++used; return Take::Ok; }
        if (!marked) { marked = true; return Take::Marker; }
        return Take::Drop;
    }
};
constexpr int MaxInfoLines = 6000, MaxWarnLines = 200;
// S1/S2 menu line budget (shared marker): 20, or 100 with advanced debug logging.
inline int menuLogBudget(bool debugLog) { return debugLog ? 100 : 20; }
// Take from a per-site counter, sharing one marker flag between sites.
inline Take menuTake(int& counter, int cap, bool& markerWritten) {
    if (counter < cap) { ++counter; return Take::Ok; }
    if (!markerWritten) { markerWritten = true; return Take::Marker; }
    return Take::Drop;
}

// One line per failure kind per interval; repeats are counted and reported on the next allowed line.
struct KindThrottle {
    bool seen = false; unsigned last = 0, suppressed = 0;
    bool allow(unsigned now, unsigned intervalMs, unsigned* suppressedOut) {
        if (seen && now - last < intervalMs) { ++suppressed; return false; }
        seen = true; last = now; *suppressedOut = suppressed; suppressed = 0;
        return true;
    }
};
// " (+N suppressed)" or "" for N == 0
inline void suppressedSuffix(char* buf, int size, unsigned n) {
    if (n) snprintf(buf, static_cast<size_t>(size), " (+%u suppressed)", n); else if (size > 0) buf[0] = 0;
}

// Why RangedAiHold did not hold a party creature (debug D H1 line); first failing condition in the order of the hold expression.
enum class NoHold { None, HoldOff, IsPC, StanceNotInMask, NoRanged, ShotNotBlocked, OutOfRange, AreaMismatch, TargetNotCreature };
inline NoHold noHoldReason(bool holdCfg, bool controlled, bool stanceInMask, bool ranged, bool blocked, bool inRange, bool sameArea, bool targetCreature) {
    if (!holdCfg) return NoHold::HoldOff;
    if (controlled) return NoHold::IsPC;
    if (!stanceInMask) return NoHold::StanceNotInMask;
    if (!ranged) return NoHold::NoRanged;
    if (!blocked) return NoHold::ShotNotBlocked;
    if (!inRange) return NoHold::OutOfRange;
    if (!sameArea) return NoHold::AreaMismatch;
    if (!targetCreature) return NoHold::TargetNotCreature;
    return NoHold::None;
}
inline const char* noHoldText(NoHold r) {
    switch (r) {
        case NoHold::HoldOff: return "hold=0";
        case NoHold::IsPC: return "controlled PC";
        case NoHold::StanceNotInMask: return "stance not in hold_stances";
        case NoHold::NoRanged: return "no ranged weapon";
        case NoHold::ShotNotBlocked: return "shot not blocked";
        case NoHold::OutOfRange: return "target out of weapon range";
        case NoHold::AreaMismatch: return "area mismatch";
        case NoHold::TargetNotCreature: return "target is not a creature";
        default: return "held";
    }
}

}  // namespace rai
