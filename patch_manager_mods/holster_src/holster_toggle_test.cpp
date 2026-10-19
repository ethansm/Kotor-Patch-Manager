// Host tests for HolsterToggle.h (holster toggle key logic; doc 42).
// Build/run: python3 scripts/run_tests.py holster_toggle
#include "HolsterToggle.h"
#include "check.h"
#include <stdint.h>

using namespace hs;

static void testKeyEdge() {
    KeyEdge e;
    KT_CHECK(!e.update(false));
    KT_CHECK(e.update(true));      // rising edge fires
    KT_CHECK(!e.update(true));     // held across frames: once only
    KT_CHECK(!e.update(true));
    KT_CHECK(!e.update(false));    // release does not fire
    KT_CHECK(e.update(true));      // re-armed
    KT_CHECK(!e.update(true));
    KeyEdge f;
    KT_CHECK(f.update(true));      // key already down on the first sample counts as an edge
}

static void testDecide() {
    // Unknown / not ready: nothing, whatever the rest says.
    for (int h = 0; h < 2; ++h)
        for (uint32_t c = 0; c < 2; ++c)
            for (int k = 0; k < 2; ++k) {
                KT_CHECK(toggleDecide(ToggleIn{false, h != 0, c, true, k != 0}) == ToggleNone);
                KT_CHECK(toggleDecide(ToggleIn{true, h != 0, c, false, k != 0}) == ToggleNone);
                KT_CHECK(toggleDecide(ToggleIn{false, h != 0, c, false, k != 0}) == ToggleNone);
            }
    // Full truth table when ready and the flag is known.
    for (int h = 0; h < 2; ++h)
        for (uint32_t c = 0; c < 3; ++c)
            for (int k = 0; k < 2; ++k) {
                const bool drawn = h != 0 || c != 0;
                const ToggleAct want = !drawn ? ToggleDraw : (k != 0 ? ToggleRefused : ToggleStow);
                KT_CHECK(toggleDecide(ToggleIn{true, h != 0, c, true, k != 0}) == want);
            }
    // Spot checks by name.
    KT_CHECK(toggleDecide(ToggleIn{true, false, 0, true, false}) == ToggleDraw);
    KT_CHECK(toggleDecide(ToggleIn{true, false, 0, true, true}) == ToggleDraw);        // hostile near does not block a draw
    KT_CHECK(toggleDecide(ToggleIn{true, true, 0, true, false}) == ToggleStow);        // hold only
    KT_CHECK(toggleDecide(ToggleIn{true, false, 1, true, false}) == ToggleStow);       // real combat flag, area clear
    KT_CHECK(toggleDecide(ToggleIn{true, false, 1, true, true}) == ToggleRefused);
    KT_CHECK(toggleDecide(ToggleIn{true, true, 1, true, true}) == ToggleRefused);
    KT_CHECK(toggleDecide(ToggleIn{true, false, 0xFFFFFFFFu, true, false}) == ToggleStow);
}

static void testClampKey() {
    KT_CHECK(clampKey(0) == 0);
    KT_CHECK(clampKey(-5) == 0);
    KT_CHECK(clampKey(1) == 1);
    KT_CHECK(clampKey(0x77) == 0x77);
    KT_CHECK(clampKey(0xFE) == 0xFE);
    KT_CHECK(clampKey(0xFF) == 0);
    KT_CHECK(clampKey(300) == 0);
    KT_CHECK(clampKey(100000L) == 0);
}

static void testParseKey() {
    int k = -1;
    KT_CHECK(parseKey("0x77", &k) && k == 0x77);
    KT_CHECK(parseKey("0X7a", &k) && k == 0x7A);
    KT_CHECK(parseKey("119", &k) && k == 119);
    k = -1;
    KT_CHECK(parseKey("0", &k) && k == 0);            // 0 = off
    k = -1;
    KT_CHECK(parseKey("0x0", &k) && k == 0);
    KT_CHECK(parseKey("0x00FE", &k) && k == 0xFE);
    KT_CHECK(parseKey("254", &k) && k == 254);
    KT_CHECK(parseKey("007", &k) && k == 7);
    // Rejections leave `out` untouched.
    k = 42;
    KT_CHECK(!parseKey("", &k) && k == 42);
    KT_CHECK(!parseKey("0x", &k) && k == 42);
    KT_CHECK(!parseKey("300", &k) && k == 42);
    KT_CHECK(!parseKey("255", &k) && k == 42);
    KT_CHECK(!parseKey("0xFF", &k) && k == 42);
    KT_CHECK(!parseKey("12x", &k) && k == 42);
    KT_CHECK(!parseKey("0x7G", &k) && k == 42);
    KT_CHECK(!parseKey("  ", &k) && k == 42);
    KT_CHECK(!parseKey(" 12", &k) && k == 42);
    KT_CHECK(!parseKey("12 ", &k) && k == 42);
    KT_CHECK(!parseKey("-1", &k) && k == 42);
    KT_CHECK(!parseKey("+1", &k) && k == 42);
    KT_CHECK(!parseKey("99999999999999999999999", &k) && k == 42);
    KT_CHECK(!parseKey("0xFFFFFFFFFFFFFFFFFFFF", &k) && k == 42);
    KT_CHECK(!parseKey("F8", &k) && k == 42);
    KT_CHECK(!parseKey(nullptr, &k) && k == 42);
    KT_CHECK(!parseKey("1", nullptr));
}

static void testClampDrawHold() {
    KT_CHECK(clampDrawHold(0) == 0);            // until the next press
    KT_CHECK(clampDrawHold(1) == 1000);
    KT_CHECK(clampDrawHold(999) == 1000);
    KT_CHECK(clampDrawHold(1000) == 1000);
    KT_CHECK(clampDrawHold(15000) == 15000);
    KT_CHECK(clampDrawHold(60000) == 60000);
    KT_CHECK(clampDrawHold(60001) == 60000);
    KT_CHECK(clampDrawHold(1000000L) == 60000);
    KT_CHECK(clampDrawHold(-1) == 1000);
}

int main() {
    testKeyEdge();
    testDecide();
    testClampKey();
    testParseKey();
    testClampDrawHold();
    return kt::Report("holster_toggle");
}
