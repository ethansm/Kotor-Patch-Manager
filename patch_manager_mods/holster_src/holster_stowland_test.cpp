// HW41 host tests for the path-driven stow landing (HolsterSlide.h slideClampStowLandF, slideStowLandWindow and the pathF / stowLandF tail of
// slideViaProgress). Build/run: python3 scripts/run_tests.py holster_stowland
#include "HolsterSlide.h"
#include "HolsterReach.h"
#include "check.h"
#include <cmath>
#include <limits>
#include <stdint.h>

using namespace hs;

static const float kNaN = std::numeric_limits<float>::quiet_NaN();
static const float kInf = std::numeric_limits<float>::infinity();
static bool closeF(float a, float b) { return std::fabs(a - b) <= 1e-5f; }
// Weapon offset share (1 = on the mount, 0 = in the hand) the caller derives from a window.
static float kOf(float t, float ms) { return ms > 1.f ? 1.f - easeSmooth(t / ms) : 0.f; }

static void testClamp() {
    KT_CHECK(closeF(slideClampStowLandF(kNaN), 0.15f));
    KT_CHECK(slideClampStowLandF(0.f) == 0.f);
    KT_CHECK(slideClampStowLandF(-1.f) == 0.f);
    KT_CHECK(slideClampStowLandF(-kInf) == 0.f);
    KT_CHECK(closeF(slideClampStowLandF(0.01f), 0.02f));
    KT_CHECK(closeF(slideClampStowLandF(0.15f), 0.15f));
    KT_CHECK(closeF(slideClampStowLandF(0.9f), 0.5f));
    KT_CHECK(closeF(slideClampStowLandF(kInf), 0.5f));
}

static void testWindow() {
    const float L = 0.15f;
    float t, ms;
    slideStowLandWindow(1.f, L, t, ms);                       // far from the mount: in the hand
    KT_CHECK(closeF(ms, 150.f) && t == ms && kOf(t, ms) == 0.f);
    slideStowLandWindow(L, L, t, ms);                         // exactly at landF: still in the hand
    KT_CHECK(t == ms && kOf(t, ms) == 0.f);
    slideStowLandWindow(0.f, L, t, ms);                       // at the mount
    KT_CHECK(t == 0.f && kOf(t, ms) == 1.f);
    slideStowLandWindow(-0.3f, L, t, ms);                     // overshoot past the mount clamps to 0
    KT_CHECK(t == 0.f);
    float prev = 1e9f;
    for (int i = 20; i >= 0; --i) {                           // pathF falls 1 -> 0: t never rises
        slideStowLandWindow(i / 20.f, L, t, ms);
        KT_CHECK(t <= prev + 1e-6f);
        KT_CHECK(t >= 0.f && t <= ms);
        prev = t;
    }
    slideStowLandWindow(kNaN, L, t, ms);
    KT_CHECK(t == ms && std::isfinite(t) && std::isfinite(ms));
    slideStowLandWindow(kInf, L, t, ms);
    KT_CHECK(t == ms);
    slideStowLandWindow(-kInf, L, t, ms);
    KT_CHECK(t == ms);
    slideStowLandWindow(0.5f, 0.f, t, ms);                    // landF 0 = off
    KT_CHECK(ms == 0.f && t == 0.f);
    slideStowLandWindow(0.5f, kNaN, t, ms);                   // NaN landF -> default 0.15
    KT_CHECK(closeF(ms, 150.f));
}

// The 350 ms stow, grab 0.3: the carry starts at d = 105 ms. The old time window already has the weapon off the hand there (the bug).
static void testShortStowBug() {
    const uint32_t len = 350, t0 = 1000, res = 240;
    const float grab = 0.3f;
    float t, ms;
    slideResidStowWindow(t0 + 105, t0, len, grab, res, t, ms);   // p = 0 of the carry
    KT_CHECK(closeF(ms, 240.f));
    KT_CHECK(kOf(t, ms) > 0.1f);                                 // BUG: weapon already ~22% toward the mount
    slideStowLandWindow(1.f, 0.15f, t, ms);                      // new window, hand at the animation end
    KT_CHECK(kOf(t, ms) == 0.f);
    slideStowLandWindow(0.15f, 0.15f, t, ms);                    // hand just entering the landing zone
    KT_CHECK(kOf(t, ms) == 0.f);
    slideStowLandWindow(0.075f, 0.15f, t, ms);                   // halfway: part-way to the mount
    KT_CHECK(kOf(t, ms) > 0.f && kOf(t, ms) < 1.f);
}

static float via(bool dir, float carryFrom, uint32_t now, float pathF, float landF) {
    return slideViaProgress(true, 240, 0.5f, 0.f, false, dir, 0.f, carryFrom, now, 1000, 350, 0.3f, false, 0, 240, pathF, landF);
}
static float viaOld(bool dir, float carryFrom, uint32_t now) {
    return slideViaProgress(true, 240, 0.5f, 0.f, false, dir, 0.f, carryFrom, now, 1000, 350, 0.3f, false, 0, 240);
}

static void testViaProgress() {
    // Default tail (stowLandF 0) equals the old call for any pathF, in every branch.
    for (uint32_t now = 1000; now <= 1400; now += 7) {
        for (int dir = 0; dir < 2; ++dir) {
            const float o = viaOld(dir != 0, 1.f, now);
            KT_CHECK(via(dir != 0, 1.f, now, 0.07f, 0.f) == o);
            KT_CHECK(via(dir != 0, 1.f, now, kNaN, 0.f) == o);
        }
    }
    // Stow with stowLandF > 0 follows pathF, not the clock.
    KT_CHECK(via(false, 1.f, 1105, 1.f, 0.15f) == 0.f);          // hand far: weapon in hand, s = 0
    KT_CHECK(via(false, 1.f, 1105, 0.15f, 0.15f) == 0.f);
    KT_CHECK(closeF(via(false, 1.f, 1105, 0.075f, 0.15f), 0.5f));
    KT_CHECK(closeF(via(false, 1.f, 1105, 0.03f, 0.15f), 0.2f));
    KT_CHECK(via(false, 1.f, 1105, 0.f, 0.15f) == 0.f);          // t = 0 -> t < ms, s = 0 / 150 = 0 (weapon on the mount)
    KT_CHECK(closeF(via(false, 1.f, 1105, 0.075f, 0.15f), via(false, 1.f, 1400, 0.075f, 0.15f)));   // clock-independent
    KT_CHECK(via(false, 1.f, 1105, kNaN, 0.15f) == 0.f);
    // A draw ignores stowLandF / pathF.
    KT_CHECK(slideViaProgress(true, 240, 0.5f, 0.f, false, true, 0.f, 0.f, 120, 1000, 350, 0.f, true, 0, 240, 0.05f, 0.15f)
             == slideViaProgress(true, 240, 0.5f, 0.f, false, true, 0.f, 0.f, 120, 1000, 350, 0.f, true, 0, 240));
    // Non-stow guards stay: inactive plan and a non-full stow give 0 / the plain carry share.
    KT_CHECK(slideViaProgress(false, 240, 0.5f, 0.f, false, false, 0.f, 1.f, 1105, 1000, 350, 0.3f, false, 0, 240, 0.05f, 0.15f) == 0.f);
    KT_CHECK(slideViaProgress(true, 240, 0.5f, 0.f, false, false, 0.f, 0.5f, 1105, 1000, 350, 0.3f, false, 0, 240, 0.05f, 0.15f) == 0.5f);
}

// HW42: the turn has its own, longer path window than the position landing.
static void testRotClamp() {
    const float L = 0.15f;
    KT_CHECK(closeF(slideClampStowRotF(kNaN, L), 0.6f));
    KT_CHECK(closeF(slideClampStowRotF(0.f, L), L));
    KT_CHECK(closeF(slideClampStowRotF(-1.f, L), L));
    KT_CHECK(closeF(slideClampStowRotF(0.05f, L), L));
    KT_CHECK(closeF(slideClampStowRotF(0.6f, L), 0.6f));
    KT_CHECK(closeF(slideClampStowRotF(5.f, L), 1.f));
    KT_CHECK(slideClampStowRotF(0.6f, 0.f) == 0.f);           // land 0 = feature off
    KT_CHECK(slideClampStowRotF(kNaN, 0.f) == 0.f);
}

static void testRotWindow() {
    const float L = 0.15f, R = slideClampStowRotF(0.6f, L);
    float t, ms, tR, msR;
    slideStowLandWindow(0.4f, L, t, ms);
    slideStowFracWindow(0.4f, R, tR, msR);
    KT_CHECK(t >= ms && kOf(t, ms) == 0.f);                   // position already on the hand
    KT_CHECK(tR < msR);                                       // still turning
    const float s = easeSmooth(tR / msR);
    KT_CHECK(s > 0.f && s < 1.f);
    slideStowLandWindow(0.f, L, t, ms);
    slideStowFracWindow(0.f, R, tR, msR);
    KT_CHECK(t == 0.f && tR == 0.f && msR > 1.f);
    slideStowFracWindow(0.6f, R, tR, msR);
    KT_CHECK(tR >= msR);
    slideStowFracWindow(0.9f, R, tR, msR);
    KT_CHECK(tR >= msR);
    float prev = 1e9f;
    for (int i = 20; i >= 0; --i) {
        slideStowFracWindow(i / 20.f, R, tR, msR);
        KT_CHECK(tR <= prev + 1e-6f && tR >= 0.f && tR <= msR);
        prev = tR;
    }
    slideStowFracWindow(kNaN, R, tR, msR);
    KT_CHECK(tR == msR && std::isfinite(tR));
    slideStowFracWindow(0.3f, 0.f, tR, msR);
    KT_CHECK(msR == 0.f && tR == 0.f);
    // In the landing range the two window functions agree.
    slideStowLandWindow(0.07f, L, t, ms);
    slideStowFracWindow(0.07f, L, tR, msR);
    KT_CHECK(t == tR && ms == msR);
    // The shown orientation at pathF 0.4 is not the hand orientation (120 deg residual still partly there) while the position share is 0.
    const Q rres{ 0.5f, 0.f, 0.f, 0.8660254f };
    slideStowLandWindow(0.4f, L, t, ms);
    slideStowFracWindow(0.4f, R, tR, msR);
    const Q o = reachResidual(rres, tR, msR);
    KT_CHECK(qdot(o, Q{ 1, 0, 0, 0 }) < 0.9999f);             // differs from the hand orientation (identity residual)
    KT_CHECK(qdot(o, rres) < 0.9999f);                        // and has moved off the mount-side Rres
    KT_CHECK(kOf(t, ms) == 0.f);
    // With the old shared window the residual at pathF 0.4 is fully gone (the HW41 flip at the very end).
    const Q o41 = reachResidual(rres, t, ms);
    KT_CHECK(closeF(qdot(o41, Q{ 1, 0, 0, 0 }), 1.f));
}

int main() {
    testClamp();
    testRotClamp();
    testRotWindow();
    testWindow();
    testShortStowBug();
    testViaProgress();
    return kt::Report("holster_stowland");
}
