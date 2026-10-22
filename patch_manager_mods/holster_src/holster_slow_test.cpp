// HW44 host tests for slow_holster_x (0.21.0-dev): slowHolsterClamp / slowMs (HolsterSlide.h), the tune key and slideResolve(..., x) (HolsterTune.h), the reversal length of
// reachRetarget (HolsterReach.h) and the path-shape rule: at x = 4 every slide ms and every fixed window that shapes the carry is 4x, so each window fraction equals the
// x = 1 one at matched slide progress. x = 1 is the 0.20.4 timing byte for byte. Build/run: python3 scripts/run_tests.py holster_slow
#include "HolsterTune.h"
#include "HolsterSlide.h"
#include "HolsterReach.h"
#include "check.h"
#include <cmath>
#include <initializer_list>
#include <limits>
#include <stdint.h>

using namespace hs;

static const float kNaN = std::numeric_limits<float>::quiet_NaN();
static const float kInf = std::numeric_limits<float>::infinity();
static bool closeF(float a, float b) { return std::fabs(a - b) <= 1e-4f; }

// ---- clamp and scale ----

// 1..8, NaN -> 1.
static void testClamp() {
    KT_CHECK(slowHolsterClamp(kNaN) == 1.f);
    KT_CHECK(slowHolsterClamp(-kInf) == 1.f);
    KT_CHECK(slowHolsterClamp(0.f) == 1.f);
    KT_CHECK(slowHolsterClamp(0.5f) == 1.f);
    KT_CHECK(slowHolsterClamp(1.f) == 1.f);
    KT_CHECK(slowHolsterClamp(2.5f) == 2.5f);
    KT_CHECK(slowHolsterClamp(8.f) == 8.f);
    KT_CHECK(slowHolsterClamp(8.5f) == 8.f);
    KT_CHECK(slowHolsterClamp(kInf) == 8.f);
}

// x = 1, NaN and x < 1 return every ms unchanged (the 0.20.4 path).
static void testScaleIdentity() {
    for (uint32_t ms = 0; ms <= 1000; ++ms) {
        KT_CHECK(slowMs(ms, 1.f) == ms);
        KT_CHECK(slowMs(ms, kNaN) == ms);
        KT_CHECK(slowMs(ms, 0.5f) == ms);
    }
}

// Rounded to the nearest ms; 0 stays 0 (instant stays instant); the factor is clamped to 8 inside.
static void testScaleValues() {
    KT_CHECK_EQ(slowMs(0, 8.f), 0u);
    KT_CHECK_EQ(slowMs(60, 4.f), 240u);     // dual stagger
    KT_CHECK_EQ(slowMs(240, 2.f), 480u);    // reach_resid_ms
    KT_CHECK_EQ(slowMs(350, 2.f), 700u);
    KT_CHECK_EQ(slowMs(350, 4.f), 1400u);
    KT_CHECK_EQ(slowMs(570, 4.f), 2280u);
    KT_CHECK_EQ(slowMs(570, 8.f), 4560u);   // above the 570 ini cap on purpose
    KT_CHECK_EQ(slowMs(1000, 8.f), 8000u);
    KT_CHECK_EQ(slowMs(350, 100.f), 2800u); // factor clamped to 8
    KT_CHECK_EQ(slowMs(350, kInf), 2800u);
    KT_CHECK_EQ(slowMs(1, 1.4f), 1u);       // 1.4 + 0.5 floors to 1
    KT_CHECK_EQ(slowMs(1, 1.5f), 2u);       // rounds half up
    KT_CHECK_EQ(slowMs(3, 1.5f), 5u);       // 4.5 rounds up
}

// ---- the ini key ----

// Int or float text, clamped to 1..8, junk / blank / nan / inf -> 1, a deleted key falls back to 1; the key is tune-owned (case-insensitive).
static void testKeyParse() {
    TuneCfg c;
    KT_CHECK(closeF(c.slowHolsterX, 1.f));   // default: off
    KT_CHECK(isSlowHolsterKey("slow_holster_x") && isSlowHolsterKey("SLOW_Holster_X"));
    KT_CHECK(!isSlowHolsterKey("slow_holster") && !isSlowHolsterKey("slow_holster_x2") && !isSlowHolsterKey(nullptr));
    KT_CHECK(isTuneKey("slow_holster_x") && isTuneKey("SLOW_HOLSTER_X"));
    KT_CHECK(applyTuneKey(c, "slow_holster_x", "4") && closeF(c.slowHolsterX, 4.f));         // an Options stepper writes ints
    KT_CHECK(applyTuneKey(c, "Slow_Holster_X", "2.5") && closeF(c.slowHolsterX, 2.5f));
    KT_CHECK(applyTuneKey(c, "slow_holster_x", "  3.0 ") && closeF(c.slowHolsterX, 3.f));
    KT_CHECK(applyTuneKey(c, "slow_holster_x", "0.5") && closeF(c.slowHolsterX, 1.f));       // clamped up to 1
    KT_CHECK(applyTuneKey(c, "slow_holster_x", "20") && closeF(c.slowHolsterX, 8.f));        // clamped down to 8
    KT_CHECK(applyTuneKey(c, "slow_holster_x", "8") && closeF(c.slowHolsterX, 8.f));         // inclusive bound
    KT_CHECK(applyTuneKey(c, "slow_holster_x", "1") && closeF(c.slowHolsterX, 1.f));
    for (const char* bad : {"fast", "nan", "inf", "-inf", "", "4x", "x4", "1,2", "--"}) {
        KT_CHECK(applyTuneKey(c, "slow_holster_x", "6") && closeF(c.slowHolsterX, 6.f));
        KT_CHECK(!applyTuneKey(c, "slow_holster_x", bad));
        KT_CHECK(closeF(c.slowHolsterX, 1.f));   // junk is 1, an earlier duplicate does not stay in force
    }
    KT_CHECK(applyTuneKey(c, "slow_holster_x", "3"));
    KT_CHECK(!applyTuneKey(c, "slow_holster_x", nullptr) && closeF(c.slowHolsterX, 3.f));    // a null value is no line at all
    clearCatTune(c);
    KT_CHECK(closeF(c.slowHolsterX, 1.f));   // a deleted key falls back to 1
}

// slowHolsterApply says what happened, so the ini reader can warn.
static void testApplyVerdict() {
    TuneCfg c;
    KT_CHECK(slowHolsterApply(c, "2") == SlowOk && closeF(c.slowHolsterX, 2.f));
    KT_CHECK(slowHolsterApply(c, "8") == SlowOk && closeF(c.slowHolsterX, 8.f));
    KT_CHECK(slowHolsterApply(c, "1") == SlowOk && closeF(c.slowHolsterX, 1.f));
    KT_CHECK(slowHolsterApply(c, "9") == SlowClamped && closeF(c.slowHolsterX, 8.f));
    KT_CHECK(slowHolsterApply(c, "0") == SlowClamped && closeF(c.slowHolsterX, 1.f));
    KT_CHECK(slowHolsterApply(c, "-3") == SlowClamped && closeF(c.slowHolsterX, 1.f));
    KT_CHECK(slowHolsterApply(c, "5") == SlowOk);
    KT_CHECK(slowHolsterApply(c, "slow") == SlowBad && closeF(c.slowHolsterX, 1.f));
    KT_CHECK(slowHolsterApply(c, "nan") == SlowBad && closeF(c.slowHolsterX, 1.f));
    KT_CHECK(slowHolsterApply(c, nullptr) == SlowBad && closeF(c.slowHolsterX, 1.f));
}

// The key does not touch the other tune fields, and the placement keys do not touch it.
static void testKeyIsolated() {
    TuneCfg c;
    KT_CHECK(applyTuneKey(c, "place", "1") && applyTuneKey(c, "slow_holster_x", "4") && applyTuneKey(c, "place_rot", "1,2,3"));
    KT_CHECK(c.place && closeF(c.slowHolsterX, 4.f) && closeF(c.rot[2], 3.f));
    KT_CHECK(applyTuneKey(c, "slow_holster_x", "2") && c.place && closeF(c.rot[0], 1.f));
}

// ---- slideResolve ----

// slideResolve with no slow argument equals x = 1, for every category and for a tune with overrides.
static void testResolveIdentity() {
    SlideTune none, some;
    some.hasDraw[CatUnknown] = true;
    some.draw[CatUnknown] = 200;
    some.hasStow[CatPistol] = true;
    some.stow[CatPistol] = 0;
    for (int c = 0; c < CatCount; ++c) {
        const Category cat = static_cast<Category>(c);
        for (const SlideTune* t : {&none, &some}) {
            const SlideRes a = slideResolve(*t, cat);
            const SlideRes b = slideResolve(*t, cat, 1.f);
            const SlideRes n = slideResolve(*t, cat, kNaN);
            KT_CHECK(a.drawMs == b.drawMs && a.stowMs == b.stowMs && a.grabPct == b.grabPct);
            KT_CHECK(a.drawMs == n.drawMs && a.stowMs == n.stowMs);
            KT_CHECK_EQ(a.drawMs, slideDrawMsFor(*t, cat));
            KT_CHECK_EQ(a.stowMs, slideStowMsFor(*t, cat));
        }
    }
    KT_CHECK_EQ(slideResolve(none, CatLightsaber).drawMs, 350u);   // the pins of holster_tune_test, unchanged
    KT_CHECK_EQ(slideResolve(none, CatMelee).stowMs, 600u);
}

// x = 4 scales both directions of every category (built-in rows, and the instant 0 stays 0); grab is a path share and is never scaled.
static void testResolveScales() {
    SlideTune none, some;
    some.hasStow[CatPistol] = true;
    some.stow[CatPistol] = 0;
    for (int c = 0; c < CatCount; ++c) {
        const Category cat = static_cast<Category>(c);
        const SlideRes base = slideResolve(none, cat);
        const SlideRes x4 = slideResolve(none, cat, 4.f);
        KT_CHECK(x4.drawMs == base.drawMs * 4u);
        KT_CHECK(x4.stowMs == base.stowMs * 4u);
        KT_CHECK(x4.grabPct == base.grabPct);
        const SlideRes s4 = slideResolve(some, cat, 4.f);
        KT_CHECK(s4.stowMs == (cat == CatPistol ? 0u : slideStowMsFor(some, cat) * 4u));
    }
    SlideTune s;
    SlideRes r = slideResolve(s, CatLightsaber, 4.f);   // built-in 350 / 350
    KT_CHECK_EQ(r.drawMs, 1400u);
    KT_CHECK_EQ(r.stowMs, 1400u);
    KT_CHECK_EQ(r.grabPct, 30u);
    r = slideResolve(s, CatMelee, 4.f);                 // built-in 520 / 600
    KT_CHECK_EQ(r.drawMs, 2080u);
    KT_CHECK_EQ(r.stowMs, 2400u);
    KT_CHECK_EQ(r.grabPct, 45u);
    KT_CHECK_EQ(slideResolve(s, CatMelee, 99.f).drawMs, 4160u);   // above the max is the max (8)
    KT_CHECK_EQ(slideResolve(s, CatMelee, 0.5f).drawMs, 520u);    // below 1 is not a slowdown
}

// The set-time clamp runs first (900 -> 570, 5000 -> 1000), then the factor: a pistol draw at x2 is 1140 ms. A 0 ms slide stays instant at any factor.
static void testClampThenScale() {
    SlideTune s;
    KT_CHECK(slideSetDraw(s, CatPistol, 900));
    KT_CHECK(slideSetStow(s, CatHeavy, 5000));
    KT_CHECK_EQ(slideResolve(s, CatPistol, 2.f).drawMs, 1140u);
    KT_CHECK_EQ(slideResolve(s, CatHeavy, 2.f).stowMs, 2000u);
    KT_CHECK_EQ(slideResolve(s, CatHeavy, 8.f).stowMs, 8000u);
    slideSetDraw(s, CatRifle, 0);
    KT_CHECK_EQ(slideResolve(s, CatRifle, 8.f).drawMs, 0u);
    SlideTune g;
    slideSetDraw(g, CatUnknown, 570);
    KT_CHECK_EQ(slideResolve(g, CatHeavy, 1.5f).drawMs, 855u);    // 570 x 1.5, over the 570 setter cap by design
    SlideTune r;
    slideSetDraw(r, CatUnknown, 301);
    KT_CHECK_EQ(slideResolve(r, CatUnknown, 1.5f).drawMs, 452u);  // 451.5 rounds up
}

// ---- path shape: x = 4 keeps every fraction of x = 1 at matched progress ----

// The slide itself: the x = 4 slide (len 4x) at tick t0 + 4k is at the same fraction as the x = 1 slide at t0 + k.
static void testSlideShape() {
    const uint32_t t0 = 5000;
    const SlideRes r1 = slideResolve(SlideTune(), CatLightsaber, 1.f);
    const SlideRes r4 = slideResolve(SlideTune(), CatLightsaber, 4.f);
    for (uint32_t k = 0; k <= r1.drawMs; k += 5) {
        Slide s1, s4;
        s1.reset(false, t0);
        s1.target(true, t0, r1.drawMs, r1.stowMs);
        s4.reset(false, t0);
        s4.target(true, t0, r4.drawMs, r4.stowMs);
        KT_CHECK(closeF(s1.frac(t0 + k), s4.frac(t0 + 4u * k)));
    }
}

// A reversal half way through a draw takes the same share of the slide at x1 and x4. reachRetarget (the reach arm) is fed the resolved ms, so its reversal length
// scales with them (rounding of the two halves allows +-3 ms on the x4 length).
static void testReversalScales() {
    const SlideRes r1 = slideResolve(SlideTune(), CatLightsaber, 1.f);
    const SlideRes r4 = slideResolve(SlideTune(), CatLightsaber, 4.f);
    Slide a, b;
    a.reset(false, 1000);
    a.target(true, 1000, r1.drawMs, r1.stowMs);
    b.reset(false, 1000);
    b.target(true, 1000, r4.drawMs, r4.stowMs);
    reachRetarget(a, false, 1000 + r1.drawMs / 2, r1.drawMs, r1.stowMs, 0.3f);
    reachRetarget(b, false, 1000 + r4.drawMs / 2, r4.drawMs, r4.stowMs, 0.3f);
    KT_CHECK(a.len > 0 && b.len > 0);
    KT_CHECK(std::fabs(static_cast<float>(b.len) - 4.f * static_cast<float>(a.len)) <= 3.f);
}

// reach_resid_ms residual of a fresh draw: the offset is captured at the slide start with msEff = min(len, residMs). Through slideViaProgress's draw branch the
// fraction at matched progress is the same at x = 1 and x = 4 (the window is 4x as long as the 4x slide).
static void testDrawResidualShape() {
    const uint32_t lens[3] = {350, 200, 570};   // 200: the draw ends before the residual window would
    const uint32_t R = 240;
    for (uint32_t L1 : lens) {
        const uint32_t L4 = L1 * 4u, R4 = slowMs(R, 4.f);
        const uint32_t msEff1 = L1 < R ? L1 : R, msEff4 = L4 < R4 ? L4 : R4;
        KT_CHECK(R4 == 4u * R && msEff4 == 4u * msEff1);
        for (uint32_t k = 0; k <= L1; k += 5) {
            const float v1 = slideViaProgress(true, R, 1.f, 0.f, false, true, 0.f, 0.f, 1000u + k, 1000u, L1, 0.3f, true, 1000u, msEff1);
            const float v4 = slideViaProgress(true, R4, 1.f, 0.f, false, true, 0.f, 0.f, 1000u + 4u * k, 1000u, L4, 0.3f, true, 1000u, msEff4);
            KT_CHECK(closeF(v1, v4));
        }
    }
}

// Legacy stow window (reach_stow_land_f = 0): ms = min(carry length, residMs), t = carry length - elapsed. At x = 4 the carry and the window are 4x, so the via
// fraction at matched progress is unchanged. A stow with a path-driven landing (HW41 / HW42) never reads ms: x changes nothing there, so those windows need no scaling.
static void testStowResidualShape() {
    const uint32_t L1 = 350, R = 240, L4 = L1 * 4u, R4 = slowMs(R, 4.f);
    for (uint32_t k = 0; k <= L1; k += 5) {
        const float v1 = slideViaProgress(true, R, 1.f, 0.f, false, false, 1.f, 1.f, 2000u + k, 2000u, L1, 0.3f, false, 0u, 0u, 0.f, 0.f);
        const float v4 = slideViaProgress(true, R4, 1.f, 0.f, false, false, 1.f, 1.f, 2000u + 4u * k, 2000u, L4, 0.3f, false, 0u, 0u, 0.f, 0.f);
        KT_CHECK(closeF(v1, v4));
    }
    for (float pf : {0.1f, 0.3f, 0.5f}) {
        const float v1 = slideViaProgress(true, R, 1.f, 0.f, false, false, 1.f, 1.f, 2100u, 2000u, L1, 0.3f, false, 0u, 0u, pf, 0.6f);
        const float v4 = slideViaProgress(true, R4, 1.f, 0.f, false, false, 1.f, 1.f, 2100u, 2000u, L4, 0.3f, false, 0u, 0u, pf, 0.6f);
        KT_CHECK(v1 == v4);
    }
}

// Reversal fade (Holster.cpp carryShown): fadeMs = min(left, residMs) with left = slide time left in the carry. Copy of that formula; at a matched reversal point the
// fade fraction is the same at x = 1 and x = 4.
static float fadeMsOf(float left, float resMs) { return left < resMs ? (left > 1.f ? left : 1.f) : resMs; }
static void testFadeShape() {
    const uint32_t L1 = 350, R = 240;
    for (uint32_t d1 = 20; d1 < L1; d1 += 30) {
        const float left1 = static_cast<float>(L1 - d1), fm1 = fadeMsOf(left1, static_cast<float>(R));
        const float left4 = static_cast<float>(4u * (L1 - d1)), fm4 = fadeMsOf(left4, static_cast<float>(slowMs(R, 4.f)));
        KT_CHECK(closeF(fm4, 4.f * fm1));
        for (uint32_t k = 0; k <= 200; k += 10) {
            const float f1 = static_cast<float>(k) / fm1, f4 = static_cast<float>(4u * k) / fm4;
            KT_CHECK(closeF(f1, f4));
        }
    }
}

// Dual stagger: the delayed start keeps its share of the slide (60 / 350 at x = 1, 240 / 1400 at x = 4), and 0 stays 0 (no stagger, no tag).
static void testStaggerShape() {
    KT_CHECK_EQ(slowMs(60, 4.f), 240u);
    KT_CHECK(closeF(static_cast<float>(slowMs(60, 4.f)) / static_cast<float>(slideResolve(SlideTune(), CatLightsaber, 4.f).drawMs), 60.f / 350.f));
    KT_CHECK_EQ(slowMs(0, 4.f), 0u);
}

int main() {
    testClamp();
    testScaleIdentity();
    testScaleValues();
    testKeyParse();
    testApplyVerdict();
    testKeyIsolated();
    testResolveIdentity();
    testResolveScales();
    testClampThenScale();
    testSlideShape();
    testReversalScales();
    testDrawResidualShape();
    testStowResidualShape();
    testFadeShape();
    testStaggerShape();
    return kt::Report("holster_slow");
}
