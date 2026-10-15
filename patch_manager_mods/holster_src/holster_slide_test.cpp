// T3 host tests for HolsterSlide.h (the cosmetic draw/stow slide of the holstered-weapons mod; doc 42).
// Build/run: python3 scripts/run_tests.py holster_slide
#include "HolsterSlide.h"
#include "check.h"
#include <cmath>
#include <limits>
#include <stdint.h>

using namespace hs;

static bool closeTo(float a, float b) { return std::fabs(a - b) <= 1e-5f; }

static bool sameQuat(const Q& a, const Q& b) {
    return closeTo(a.w, b.w) && closeTo(a.x, b.x) && closeTo(a.y, b.y) && closeTo(a.z, b.z);
}
static bool samePos(const float* a, const float* b) {
    return closeTo(a[0], b[0]) && closeTo(a[1], b[1]) && closeTo(a[2], b[2]);
}
static bool samePose(const Pose& a, const Pose& b) { return samePos(a.pos, b.pos) && sameQuat(a.q, b.q); }
static float dot(const Q& a, const Q& b) { return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z; }

// Mount and hand poses for most cases. The hand is 60 deg about z from the mount, so its quat dot with the mount is positive
// (no sign flip), and the two positions differ on every axis.
static Pose mountPose() { return { {0.1f, -0.2f, 0.3f}, qbase(10.f, 20.f, 30.f) }; }
static Pose handPose() {
    const Pose m = mountPose();
    const float z[3] = {0.f, 0.f, 1.f};
    return { {0.5f, 0.3f, -0.4f}, qnormalize(qmul(m.q, qaxis(z, 60.f))) };
}

static void testBlendEndpointsAndClamp() {
    const Pose m = mountPose(), h = handPose();
    KT_CHECK(samePose(blendPose(m, h, 0.f), m));
    KT_CHECK(samePose(blendPose(m, h, 1.f), h));
    KT_CHECK(samePose(blendPose(m, h, -0.5f), m));  // f below 0 clamps to the mount
    KT_CHECK(samePose(blendPose(m, h, 2.f), h));    // f above 1 clamps to the hand
    KT_CHECK(samePose(blendPose(m, h, std::numeric_limits<float>::quiet_NaN()), m));  // NaN maps to the mount
    KT_CHECK(poseSane(blendPose(m, h, 0.f)) && poseSane(blendPose(m, h, 0.37f)) && poseSane(blendPose(m, h, 1.f)));
}

static void testEaseSmooth() {
    KT_CHECK(closeTo(easeSmooth(0.f), 0.f));
    KT_CHECK(closeTo(easeSmooth(1.f), 1.f));
    KT_CHECK(closeTo(easeSmooth(0.5f), 0.5f));
    KT_CHECK(closeTo(easeSmooth(-3.f), 0.f));                                           // clamp below 0
    KT_CHECK(closeTo(easeSmooth(7.f), 1.f));                                            // clamp above 1
    KT_CHECK(closeTo(easeSmooth(std::numeric_limits<float>::quiet_NaN()), 0.f));        // NaN maps to 0
    float prev = easeSmooth(0.f);
    for (int i = 1; i <= 100; ++i) {
        const float e = easeSmooth(i * 0.01f);
        KT_CHECK(e >= prev);
        prev = e;
    }
}

static void testBlendPosMonotoneAlongSegment() {
    const Pose m = mountPose(), h = handPose();
    float prev[3] = {m.pos[0], m.pos[1], m.pos[2]};
    for (int i = 1; i <= 100; ++i) {
        const Pose p = blendPose(m, h, i * 0.01f);
        for (int c = 0; c < 3; ++c) {
            const float delta = h.pos[c] - m.pos[c];  // direction of the segment on this axis
            KT_CHECK((p.pos[c] - prev[c]) * delta >= -1e-7f);
            prev[c] = p.pos[c];
        }
    }
    const Pose mid = blendPose(m, h, 0.25f);
    for (int c = 0; c < 3; ++c) KT_CHECK(closeTo(mid.pos[c], m.pos[c] + 0.25f * (h.pos[c] - m.pos[c])));
}

static void testShortArcOnAntipodalHand() {
    const Pose m = mountPose();
    const float z[3] = {0.f, 0.f, 1.f};
    // turned = mount turned 5 deg about z. The hand stores -turned: the same rotation, but on the far side of the 4-sphere.
    const Q turned = qmul(m.q, qaxis(z, 5.f));
    Pose h = m;
    h.pos[0] = 0.5f; h.pos[1] = 0.3f; h.pos[2] = -0.4f;
    h.q = { -turned.w, -turned.x, -turned.y, -turned.z };
    KT_CHECK(dot(m.q, h.q) < 0.f);  // precondition: the raw hand quat is more than 90 deg from the mount
    for (int i = 0; i <= 10; ++i) {
        const float f = i * 0.1f;
        const Pose p = blendPose(m, h, f);
        KT_CHECK(dot(p.q, m.q) > 0.99f);      // short arc: the blend stays near the mount ...
        KT_CHECK(dot(p.q, turned) > 0.99f);   // ... and near the hand (f = 1 gives the turned quat, the hand's rotation)
        KT_CHECK(poseSane(p));
    }
}

static void testFsmDrawSlide() {
    const Pose m = mountPose(), h = handPose();
    const uint32_t t = 100000u;
    Fsm fsm;
    fsm.onCombat(1, t);
    KT_CHECK(fsm.inHand());                                  // usable at once; the slide is cosmetic
    KT_CHECK(samePose(slidePose(fsm, t, m, h), m));          // t + 0: at the mount
    const Pose mid = slidePose(fsm, t + kDrawMs / 2, m, h);  // half way: easeSmooth(0.5) = 0.5, so the midpoint
    for (int c = 0; c < 3; ++c) KT_CHECK(closeTo(mid.pos[c], 0.5f * (m.pos[c] + h.pos[c])));
    KT_CHECK(poseSane(mid));
    KT_CHECK(samePose(slidePose(fsm, t + kDrawMs, m, h), h));  // t + kDrawMs: in the hand
    fsm.tick(t + kDrawMs);
    KT_CHECK(fsm.phase == PhDrawn && fsm.inHand());
    KT_CHECK(samePose(slidePose(fsm, t + kDrawMs, m, h), h));
}

static void testFsmStowSlideReturnsToMount() {
    const Pose m = mountPose(), h = handPose();
    const uint32_t t = 200000u;
    Fsm fsm;
    fsm.onCombat(1, t);
    fsm.tick(t + kDrawMs);                                   // PhDrawn: in the hand
    fsm.onCombat(0, t + 400u);                               // combat ends: stow waits kStowDelayMs
    fsm.tick(t + 400u + kStowDelayMs);                       // stow begins: starts at the hand
    const uint32_t s = t + 400u + kStowDelayMs;
    KT_CHECK(fsm.phase == PhStowing);
    KT_CHECK(samePose(slidePose(fsm, s, m, h), h));
    KT_CHECK(samePose(slidePose(fsm, s + kDrawMs, m, h), m));  // slide done: back at the mount
    fsm.tick(s + kDrawMs);
    KT_CHECK(fsm.phase == PhHolstered && !fsm.inHand());
}

static void testSlideAcrossCounterWrap() {
    const Pose m = mountPose(), h = handPose();
    const uint32_t t0 = 0xFFFFFFF0u;  // 16 ticks before the 32-bit counter wraps to 0
    Fsm fsm;
    fsm.onCombat(1, t0);
    KT_CHECK(fsm.inHand());
    float prevFrac = -1.f, prevX = m.pos[0];
    for (uint32_t k = 0; k <= 40; ++k) {
        const uint32_t now = t0 + k * 10u;  // wraps past 0 at k = 2
        const float frac = fsm.handFraction(now);
        KT_CHECK(closeTo(frac, std::fmin(1.f, k * 10.f / 350.f)));  // the same ramp as without the wrap
        KT_CHECK(frac >= prevFrac);
        prevFrac = frac;
        const Pose p = slidePose(fsm, now, m, h);
        KT_CHECK(poseSane(p));
        KT_CHECK(p.pos[0] - prevX >= -1e-7f);  // x runs mount -> hand (0.1 -> 0.5): never backwards across the wrap
        prevX = p.pos[0];
    }
    fsm.tick(t0 + kDrawMs);  // 350 ms after the start, across the wrap
    KT_CHECK(fsm.phase == PhDrawn && fsm.inHand());
    KT_CHECK(samePose(slidePose(fsm, t0 + kDrawMs, m, h), h));
}

int main() {
    testBlendEndpointsAndClamp();
    testEaseSmooth();
    testBlendPosMonotoneAlongSegment();
    testShortArcOnAntipodalHand();
    testFsmDrawSlide();
    testFsmStowSlideReturnsToMount();
    testSlideAcrossCounterWrap();
    return kt::Report("holster_slide");
}
