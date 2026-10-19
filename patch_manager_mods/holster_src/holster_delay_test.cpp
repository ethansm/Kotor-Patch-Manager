// Host tests for HolsterDelay.h (automatic shorter combat exit; doc 42).
// Build/run: python3 scripts/run_tests.py holster_delay
#include "HolsterDelay.h"
#include "check.h"
#include <stdint.h>

using namespace hs;

// A readable combat-countdown sample with `el` ms already elapsed.
static DelayIn combatIn(int el) { return DelayIn{1, kCombatMs - el, 0, 0, 1, true, true}; }
static DelayIn flourishIn(int el) { return DelayIn{1, kCombatMs - el, 0, 2, 0, true, true}; }

static void testElapsedAndClamp() {
    KT_CHECK(elapsedMs(8000) == 0);
    KT_CHECK(elapsedMs(6500) == 1500);
    KT_CHECK(elapsedMs(1) == 7999);
    KT_CHECK(clampExitMs(0) == 0);
    KT_CHECK(clampExitMs(1) == 200);
    KT_CHECK(clampExitMs(199) == 200);
    KT_CHECK(clampExitMs(200) == 200);
    KT_CHECK(clampExitMs(1500) == 1500);
    KT_CHECK(clampExitMs(7800) == 7800);
    KT_CHECK(clampExitMs(7801) == 7800);
    KT_CHECK(clampExitMs(99999L) == 7800);
    KT_CHECK(clampExitMs(-3) == 200);
    DelayCfg d;
    KT_CHECK(d.combatExitMs == 1500 && d.flourishExitMs == 1500);
}

static void testBoundary() {
    DelayCfg cfg;
    KT_CHECK(delayGate(combatIn(1500), cfg) == DelayNeedHostileCheck);   // exact boundary
    KT_CHECK(delayGate(combatIn(1499), cfg) == DelayNone);
    KT_CHECK(delayGate(combatIn(1501), cfg) == DelayNeedHostileCheck);
    KT_CHECK(delayGate(combatIn(0), cfg) == DelayNone);                  // plus524 == 8000: valid, nothing elapsed
    KT_CHECK(delayGate(combatIn(7999), cfg) == DelayNeedHostileCheck);   // plus524 == 1
    KT_CHECK(delayGate(flourishIn(1500), cfg) == DelayNeedHostileCheck);
    KT_CHECK(delayGate(flourishIn(1499), cfg) == DelayNone);
}

static void testPreconditions() {
    DelayCfg cfg;
    DelayIn in = combatIn(3000);
    KT_CHECK(delayGate(in, cfg) == DelayNeedHostileCheck);
    DelayIn x = in; x.serverReadable = false;
    KT_CHECK(delayGate(x, cfg) == DelayNone);
    x = in; x.clientKnown = false;
    KT_CHECK(delayGate(x, cfg) == DelayNone);
    x = in; x.plus520 = 0;
    KT_CHECK(delayGate(x, cfg) == DelayNone);
    x = in; x.plus520 = 2;
    KT_CHECK(delayGate(x, cfg) == DelayNone);
    x = in; x.plus528 = 1;
    KT_CHECK(delayGate(x, cfg) == DelayNone);
    x = in; x.plus528 = 0xFFFFFFFFu;
    KT_CHECK(delayGate(x, cfg) == DelayNone);
    x = in; x.plus524 = 0;
    KT_CHECK(delayGate(x, cfg) == DelayNone);       // countdown finished: engine exits by itself
    x = in; x.plus524 = -1;
    KT_CHECK(delayGate(x, cfg) == DelayNone);
    x = in; x.plus524 = INT32_MIN;
    KT_CHECK(delayGate(x, cfg) == DelayNone);
    x = in; x.plus524 = 8001;
    KT_CHECK(delayGate(x, cfg) == DelayNone);       // out of range: not a plain countdown
    x = in; x.plus524 = INT32_MAX;
    KT_CHECK(delayGate(x, cfg) == DelayNone);
    // Flourish preconditions apply the same way.
    DelayIn f = flourishIn(3000);
    KT_CHECK(delayGate(f, cfg) == DelayNeedHostileCheck);
    f.plus528 = 1;
    KT_CHECK(delayGate(f, cfg) == DelayNone);
    f = flourishIn(3000); f.plus520 = 0;
    KT_CHECK(delayGate(f, cfg) == DelayNone);
    f = flourishIn(3000); f.clientKnown = false;
    KT_CHECK(delayGate(f, cfg) == DelayNone);
    f = flourishIn(3000); f.serverReadable = false;
    KT_CHECK(delayGate(f, cfg) == DelayNone);
}

static void testClassification() {
    DelayCfg cfg;
    // Client combat flag 0 and +0x11E8 != 2: unknown case, fail closed.
    for (uint32_t v : {0u, 1u, 3u, 0xFFFFFFFFu}) {
        DelayIn in = flourishIn(5000);
        in.plus11E8 = v;
        KT_CHECK(delayGate(in, cfg) == DelayNone);
    }
    // Real combat wins over the flourish marker.
    DelayIn both = combatIn(5000);
    both.plus11E8 = 2;
    KT_CHECK(delayGate(both, cfg) == DelayNeedHostileCheck);
    // Any non-zero client flag is combat.
    DelayIn c2 = combatIn(5000);
    c2.clientCombat = 2;
    KT_CHECK(delayGate(c2, cfg) == DelayNeedHostileCheck);
}

static void testConfig() {
    // combatExitMs = 0 turns the real-combat cut off; flourish unaffected.
    DelayCfg cfg;
    cfg.combatExitMs = 0;
    KT_CHECK(delayGate(combatIn(7000), cfg) == DelayNone);
    KT_CHECK(delayGate(flourishIn(7000), cfg) == DelayNeedHostileCheck);
    // flourishExitMs = 0 turns the flourish cut off; combat unaffected.
    cfg = DelayCfg{};
    cfg.flourishExitMs = 0;
    KT_CHECK(delayGate(flourishIn(7000), cfg) == DelayNone);
    KT_CHECK(delayGate(combatIn(7000), cfg) == DelayNeedHostileCheck);
    // Both off.
    cfg.combatExitMs = 0;
    KT_CHECK(delayGate(combatIn(7999), cfg) == DelayNone);
    KT_CHECK(delayGate(flourishIn(7999), cfg) == DelayNone);
    // Different thresholds: flourish 1200, combat 2000.
    cfg.flourishExitMs = 1200;
    cfg.combatExitMs = 2000;
    KT_CHECK(delayGate(flourishIn(1199), cfg) == DelayNone);
    KT_CHECK(delayGate(flourishIn(1200), cfg) == DelayNeedHostileCheck);
    KT_CHECK(delayGate(combatIn(1200), cfg) == DelayNone);               // same elapsed, combat still waits
    KT_CHECK(delayGate(combatIn(1999), cfg) == DelayNone);
    KT_CHECK(delayGate(combatIn(2000), cfg) == DelayNeedHostileCheck);
    // Negative config behaves as off (never a cut).
    cfg.combatExitMs = -1;
    KT_CHECK(delayGate(combatIn(7999), cfg) == DelayNone);
}

static void testThrottle() {
    Throttle t;
    KT_CHECK(t.due(0));                 // fresh: due immediately
    KT_CHECK(t.due(12345));
    t.backoff(1000, 1000);
    KT_CHECK(t.next == 2000);
    KT_CHECK(!t.due(1000));
    KT_CHECK(!t.due(1999));
    KT_CHECK(t.due(2000));
    KT_CHECK(t.due(5000));
    // Wrap-around: back-off crosses the 32-bit rollover.
    Throttle w;
    w.backoff(0xFFFFFC00u, 1000);        // 0xFFFFFC00 + 1000 = 0xFFFFFFE8, no wrap yet
    KT_CHECK(w.next == 0xFFFFFFE8u);
    KT_CHECK(!w.due(0xFFFFFC00u));
    KT_CHECK(!w.due(0xFFFFFFE7u));
    KT_CHECK(w.due(0xFFFFFFE8u));
    KT_CHECK(w.due(0u));                 // past the rollover: due
    Throttle v;
    v.backoff(0xFFFFFFF0u, 1000);        // next = 0x3D8, after the rollover
    KT_CHECK(v.next == 0x3D8u);
    KT_CHECK(!v.due(0xFFFFFFF0u));
    KT_CHECK(!v.due(0xFFFFFFFFu));
    KT_CHECK(!v.due(0u));
    KT_CHECK(!v.due(0x3D7u));
    KT_CHECK(v.due(0x3D8u));
    KT_CHECK(v.due(0x3E0u));
}

static void testShouldCut() {
    KT_CHECK(shouldCut(DelayNeedHostileCheck, false));
    KT_CHECK(!shouldCut(DelayNeedHostileCheck, true));
    KT_CHECK(!shouldCut(DelayNone, false));
    KT_CHECK(!shouldCut(DelayNone, true));
    // End to end: gate then cut.
    DelayCfg cfg;
    KT_CHECK(shouldCut(delayGate(combatIn(2000), cfg), false));
    KT_CHECK(!shouldCut(delayGate(combatIn(2000), cfg), true));
    KT_CHECK(!shouldCut(delayGate(combatIn(100), cfg), false));
}

int main() {
    testElapsedAndClamp();
    testBoundary();
    testPreconditions();
    testClassification();
    testConfig();
    testThrottle();
    testShouldCut();
    return kt::Report("holster_delay");
}
