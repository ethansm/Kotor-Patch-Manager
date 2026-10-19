// HolsterDelay.h -- pure logic of the automatic shorter combat exit of the holstered-weapons mod (doc 42; no Win32, host-testable; tests in holster_delay_test.cpp).
// The engine keeps a creature in the combat state for kCombatMs (8000) after the last hostile event: server +0x524 counts down from 8000 while +0x520 == 1.
// This mod cuts the countdown (+0x524 = 0) early when nothing hostile is near, after a configurable elapsed time.
//   DelayCfg      combatExitMs / flourishExitMs (0 = off); clampExitMs: 0 stays 0, else 200..7800
//   DelayIn       raw reads of the server creature (+0x520 state, +0x524 countdown ms, +0x528, +0x11E8) and the client combat flag
//   elapsedMs     kCombatMs - plus524: time spent in the countdown so far
//   delayGate     cheap pre-check: None, or NeedHostileCheck (the caller then runs the costly hostile scan and calls shouldCut)
//   Throttle      next-allowed tick with wrap-safe compare (back-off between hostile scans)
//   shouldCut     the caller writes +0x524 = 0 only when the gate asked for a check AND nothing hostile is near
// Failure mode rule: unreadable / unexpected values never cut (the engine keeps its own 8 s).
#pragma once
#include <stdint.h>

namespace hs {

constexpr int kCombatMs = 8000;

struct DelayCfg { int combatExitMs = 1500; int flourishExitMs = 1500; };

inline int clampExitMs(long v) {
    if (v == 0) return 0;
    if (v < 200) return 200;
    if (v > 7800) return 7800;
    return static_cast<int>(v);
}

struct DelayIn {
    uint32_t plus520;      // server +0x520 combat state (1 = counting down)
    int32_t plus524;       // countdown ms
    uint32_t plus528;
    uint32_t plus11E8;     // 2 = flourish-only
    uint32_t clientCombat; // client +0x138
    bool clientKnown;      // client flag readable
    bool serverReadable;   // server creature fields readable
};

enum DelayAct : int { DelayNone = 0, DelayNeedHostileCheck = 1 };

inline int elapsedMs(int32_t plus524) { return kCombatMs - plus524; }

inline DelayAct delayGate(const DelayIn& in, const DelayCfg& cfg) {
    if (!in.serverReadable || !in.clientKnown) return DelayNone;
    if (in.plus520 != 1 || in.plus528 != 0) return DelayNone;
    if (in.plus524 <= 0 || in.plus524 > kCombatMs) return DelayNone;
    const int el = elapsedMs(in.plus524);
    if (in.clientCombat != 0) {
        if (cfg.combatExitMs > 0 && el >= cfg.combatExitMs) return DelayNeedHostileCheck;
        return DelayNone;
    }
    if (in.plus11E8 == 2) {
        if (cfg.flourishExitMs > 0 && el >= cfg.flourishExitMs) return DelayNeedHostileCheck;
        return DelayNone;
    }
    return DelayNone;   // client not in combat and not a flourish: unknown case, fail closed
}

struct Throttle {
    uint32_t next = 0;
    bool due(uint32_t now) const { return static_cast<int32_t>(now - next) >= 0; }
    void backoff(uint32_t now, uint32_t ms) { next = now + ms; }
};

inline bool shouldCut(DelayAct a, bool hostileNear) { return a == DelayNeedHostileCheck && !hostileNear; }

}  // namespace hs
