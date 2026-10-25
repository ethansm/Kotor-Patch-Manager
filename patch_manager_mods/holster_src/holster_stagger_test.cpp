// HW33a host tests: Slide::targetDelayed / pump / pending / touch (HolsterSlide.h) and isTwoHandedStem (HolsterLogic.h; doc 42c 4.11).
// Build/run: python3 scripts/run_tests.py holster_stagger
#include "HolsterLogic.h"
#include "HolsterSlide.h"
#include "check.h"
#include <cmath>
#include <stdint.h>

using namespace hs;

static bool closeTo(float a, float b) { return std::fabs(a - b) <= 1e-5f; }

// The slide state target() writes, plus pendOn. Floats compared with closeTo, the rest exactly.
static bool sameSlide(const Slide& a, const Slide& b) {
    return closeTo(a.from, b.from) && a.t0 == b.t0 && a.len == b.len && a.last == b.last && a.dir == b.dir && closeTo(a.w0, b.w0) &&
           closeTo(a.extractFrom, b.extractFrom) && a.oriOn == b.oriOn && a.pendOn == b.pendOn;
}

// delayMs == 0 is target() byte for byte, from rest and from a reversal half way through a draw.
static void testZeroDelayIsTarget() {
    Slide a, b;
    a.reset(false, 1000);
    b.reset(false, 1000);
    a.target(true, 1200, 300, 250, 0.4f);
    b.targetDelayed(true, 1200, 0, 300, 250, 0.4f);
    KT_CHECK(sameSlide(a, b));
    KT_CHECK(!b.pending());

    Slide c, d;
    c.reset(false, 0);
    d.reset(false, 0);
    c.target(true, 0, 400, 400);
    d.target(true, 0, 400, 400);
    c.target(false, 200, 400, 400, 0.2f);  // reversal at fraction 0.5
    d.targetDelayed(false, 200, 0, 400, 400, 0.2f);
    KT_CHECK(sameSlide(c, d));
    KT_CHECK(closeTo(d.from, 0.5f));
    KT_CHECK_EQ(d.len, 200u);

    // A delay-0 call also clears an armed target.
    Slide e, f;
    e.reset(false, 0);
    f.reset(false, 0);
    e.targetDelayed(true, 0, 100, 300, 300);
    KT_CHECK(e.pending());
    e.targetDelayed(true, 10, 0, 300, 300);
    f.target(true, 10, 300, 300);
    KT_CHECK(!e.pending());
    KT_CHECK(sameSlide(e, f));
}

// During the delay the slide reads exactly as one that was never targeted.
static void testDelayHoldsState() {
    Slide r, s;
    r.reset(false, 0);
    s.reset(false, 0);
    r.target(true, 0, 400, 400);  // running draw, never retargeted
    s.target(true, 0, 400, 400);
    s.targetDelayed(false, 100, 50, 400, 400);  // stow armed at 100, fires at 150
    KT_CHECK(s.pending());
    for (uint32_t now = 100; now < 150; ++now) {
        s.pump(now);
        KT_CHECK(s.pending());
        KT_CHECK_EQ(s.dir, 1);
        KT_CHECK(closeTo(s.frac(now), r.frac(now)));
        KT_CHECK(s.active(now) == r.active(now));
    }

    Slide x;
    x.reset(false, 1000);
    x.targetDelayed(true, 1000, 100, 300, 300);
    for (uint32_t now = 1000; now < 1100; ++now) {
        x.pump(now);
        KT_CHECK_EQ(x.dir, 0);
        KT_CHECK(closeTo(x.frac(now), 0.f));
        KT_CHECK(!x.active(now));
    }
}

// pump at pendAt + 5 starts the slide at pendAt, from the fraction the old slide had there (no jump, no lost time).
static void testPumpContinuity() {
    Slide r, s;
    r.reset(false, 0);
    s.reset(false, 0);
    r.target(true, 0, 400, 400);
    s.target(true, 0, 400, 400);
    const float oldF = r.frac(150);  // 0.375
    s.targetDelayed(false, 100, 50, 400, 400);
    s.pump(155);
    KT_CHECK(!s.pending());
    KT_CHECK_EQ(s.t0, 150u);
    KT_CHECK_EQ(s.dir, 0);
    KT_CHECK(closeTo(s.from, oldF));
    KT_CHECK(closeTo(s.frac(150), oldF));
    KT_CHECK_EQ(s.len, 150u);  // stow from 0.375 over 400 ms * 0.375
}

// A reversal during the delay cancels the armed target and leaves the slide as it was.
static void testReverseCancels() {
    Slide r;
    r.reset(false, 0);
    r.target(true, 0, 400, 400);

    // (a) targetDelayed back to the current dir
    Slide s;
    s.reset(false, 0);
    s.target(true, 0, 400, 400);
    s.targetDelayed(false, 100, 50, 400, 400);
    KT_CHECK(s.pending());
    s.targetDelayed(true, 110, 50, 400, 400);
    KT_CHECK(!s.pending());
    KT_CHECK(sameSlide(s, r));
    s.pump(150);
    KT_CHECK_EQ(s.dir, 1);
    KT_CHECK(sameSlide(s, r));

    // (b) plain target() back to the current dir
    Slide q;
    q.reset(false, 0);
    q.target(true, 0, 400, 400);
    q.targetDelayed(false, 100, 50, 400, 400);
    q.target(true, 110, 400, 400);
    KT_CHECK(!q.pending());
    KT_CHECK(sameSlide(q, r));
}

// now near the uint32 wrap: the armed target fires at now + delay (wrapped), never early.
static void testWrapDelay() {
    const uint32_t bases[] = {0u, 5000u, 0xFFFFFC00u, 0xFFFFFFF0u};
    for (uint32_t base : bases) {
        Slide s;
        s.reset(false, base);
        s.targetDelayed(true, base, 60, 300, 300);
        s.pump(base + 59);
        KT_CHECK(s.pending());
        KT_CHECK_EQ(s.dir, 0);
        s.pump(base + 60);
        KT_CHECK(!s.pending());
        KT_CHECK_EQ(s.dir, 1);
        KT_CHECK_EQ(s.t0, base + 60u);
        KT_CHECK(closeTo(s.frac(base + 60), 0.f));
    }
}

// reset() drops an armed target: a later pump does nothing.
static void testResetCancels() {
    Slide s;
    s.reset(false, 0);
    s.targetDelayed(true, 0, 100, 300, 300);
    KT_CHECK(s.pending());
    s.reset(false, 500);
    KT_CHECK(!s.pending());
    s.pump(600);
    KT_CHECK_EQ(s.dir, 0);
    KT_CHECK_EQ(s.len, 0u);
    KT_CHECK(closeTo(s.frac(600), 0.f));
}

// The same want called every frame keeps the first pendAt; a re-arm at 1010 would fire at 1110 instead.
static void testNoRearm() {
    Slide s;
    s.reset(false, 1000);
    for (uint32_t now = 1000; now <= 1100; now += 10) {
        s.targetDelayed(true, now, 100, 300, 300);
        s.pump(now);
    }
    KT_CHECK(!s.pending());
    KT_CHECK_EQ(s.dir, 1);
    KT_CHECK_EQ(s.t0, 1100u);
}

// A 500 ms stall during the delay moves pendAt by 500 ms; a short gap (10 ms) does not.
static void testTouchShiftsPending() {
    Slide s;
    s.reset(false, 1000);
    s.targetDelayed(true, 1000, 100, 300, 300);  // pendAt 1100
    s.touch(1010);
    s.touch(1510);                               // pendAt 1600
    s.pump(1100);
    s.pump(1599);
    KT_CHECK(s.pending());
    KT_CHECK_EQ(s.dir, 0);
    s.pump(1600);
    KT_CHECK(!s.pending());
    KT_CHECK_EQ(s.dir, 1);
    KT_CHECK_EQ(s.t0, 1600u);
}

static void testTwoHandedStem() {
    KT_CHECK(isTwoHandedStem("w_Dblsbr_001"));
    KT_CHECK(isTwoHandedStem("W_DBLSBR_003"));
    KT_CHECK(isTwoHandedStem("w_ForcePike_001"));
    KT_CHECK(isTwoHandedStem("w_gaffi_002"));
    KT_CHECK(isTwoHandedStem("w_warblade_001"));
    KT_CHECK(isTwoHandedStem("w_Dblswrd_001"));
    KT_CHECK(isTwoHandedStem("w_VbrDblswd_002"));
    KT_CHECK(isTwoHandedStem("w_Qtrstaff_001"));   // CatQStaff
    KT_CHECK(isTwoHandedStem("w_IonRfl_001"));     // CatRifle
    KT_CHECK(isTwoHandedStem("w_RptnBlstr_001"));  // CatHeavy
    // Every table row that classifies as QStaff / Rifle / Heavy.
    for (int i = 0; i < kStemCount; ++i) {
        const Category c = kStems[i].cat;
        if (c == CatQStaff || c == CatRifle || c == CatHeavy) KT_CHECK(isTwoHandedStem(kStems[i].stem));
    }
    KT_CHECK(!isTwoHandedStem("w_waraxe_001"));  // one-handed
    KT_CHECK(!isTwoHandedStem("w_lghtsbr_001"));
    KT_CHECK(!isTwoHandedStem("w_BlstrPstl_001"));
    KT_CHECK(!isTwoHandedStem("w_Dblsbrx"));     // family prefix needs '_', a digit or the end
    KT_CHECK(!isTwoHandedStem(nullptr));
    KT_CHECK(!isTwoHandedStem(""));
}

// HW33a D3: staggerDualEligible only delays a left-hand slide when starting from rest (!selfSlide.active(now)) and when the
// right-hand sibling entry was initialized and touched within kStaggerLiveMs (250 ms).
static void testStaggerDualEligible() {
    Slide left, right;
    // Uninitialized right-hand entry: never eligible
    left.reset(false, 1000);
    KT_CHECK(!staggerDualEligible(left, right, 1000));

    // Both at rest and right touched within 250 ms: eligible
    right.reset(false, 1000);
    KT_CHECK(staggerDualEligible(left, right, 1000));
    KT_CHECK(staggerDualEligible(left, right, 1250));   // exact boundary (250 ms)

    // Stale / unequipped right-hand entry (> 250 ms old): not eligible (solo left weapon does not wait 60 ms)
    KT_CHECK(!staggerDualEligible(left, right, 1251));
    KT_CHECK(!staggerDualEligible(left, right, 5000));

    // Mid-slide reversal on the left hand (left.active(now) == true): not eligible (reverses immediately via reachRetarget)
    left.target(true, 1000, 400, 400);
    right.touch(1150);
    KT_CHECK(left.active(1150));
    KT_CHECK(!staggerDualEligible(left, right, 1150));

    // Once the left slide finishes (now == 1400, at rest again) and right is live: eligible again
    right.touch(1400);
    KT_CHECK(!left.active(1400));
    KT_CHECK(staggerDualEligible(left, right, 1400));
}

int main() {
    testZeroDelayIsTarget();
    testDelayHoldsState();
    testPumpContinuity();
    testReverseCancels();
    testWrapDelay();
    testResetCancels();
    testNoRearm();
    testTouchShiftsPending();
    testTwoHandedStem();
    testStaggerDualEligible();
    return kt::Report("holster_stagger");
}
