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

// ---- per-weapon Slide ----

static bool exactPos(const Pose& a, const Pose& b) { return a.pos[0] == b.pos[0] && a.pos[1] == b.pos[1] && a.pos[2] == b.pos[2]; }

static void testSlideResetIsSilent() {
    Slide s;
    s.reset(true, 5000u);
    KT_CHECK(s.init == 1 && s.dir == 1 && s.len == 0 && s.t0 == 5000u && s.last == 5000u);
    KT_CHECK(s.frac(5000u) == 1.f && s.frac(5001u) == 1.f && s.frac(4000u) == 1.f);  // in the hand at once, even for a stale tick
    KT_CHECK(!s.active(5000u) && !s.active(5100u));
    s.reset(false, 6000u);
    KT_CHECK(s.dir == 0 && s.from == 0.f && s.frac(6000u) == 0.f && s.frac(6500u) == 0.f && !s.active(6100u));
    // reset in the middle of a slide cancels it
    s.target(true, 7000u, 350u, 350u);
    s.reset(true, 7100u);
    KT_CHECK(s.frac(7100u) == 1.f && s.from == 1.f && s.len == 0 && !s.active(7100u));
    KT_CHECK(Slide().frac(123u) == 0.f && !Slide().active(123u) && Slide().init == 0);  // default: at the hip
}

static void testSlideEndpointsAndMid() {
    const Pose hip = mountPose(), hand = handPose();
    const uint32_t t = 100000u;
    Slide s;
    s.reset(false, t - 1000u);
    s.target(true, t, 350u, 500u);
    KT_CHECK(s.dir == 1 && s.from == 0.f && s.t0 == t && s.len == 350u);
    KT_CHECK(s.frac(t) == 0.f);
    KT_CHECK(s.frac(t + 350u) == 1.f && s.frac(t + 351u) == 1.f && s.frac(t + 100000u) == 1.f);
    KT_CHECK(closeTo(s.frac(t + 175u), 0.5f));
    KT_CHECK(closeTo(s.frac(t + 35u), 0.1f));
    KT_CHECK(s.frac(t + 349u) < 1.f && s.frac(t + 349u) > 0.99f);
    KT_CHECK(s.frac(t + 1u) > 0.f && s.frac(t + 1u) < 0.01f);
    KT_CHECK(exactPos(slideShown(s, t, hip, hand), hip) && samePose(slideShown(s, t, hip, hand), hip));
    KT_CHECK(exactPos(slideShown(s, t + 350u, hip, hand), hand) && samePose(slideShown(s, t + 350u, hip, hand), hand));
    const Pose mid = slideShown(s, t + 175u, hip, hand);  // easeSmooth(0.5) = 0.5: the midpoint
    for (int c = 0; c < 3; ++c) KT_CHECK(closeTo(mid.pos[c], 0.5f * (hip.pos[c] + hand.pos[c])));
    KT_CHECK(poseSane(mid));
    // active only strictly between the ends
    KT_CHECK(!s.active(t) && s.active(t + 1u) && s.active(t + 349u) && !s.active(t + 350u) && !s.active(t + 351u));
    KT_CHECK(!s.active(t - 1u));
    // monotone
    float prev = 0.f;
    for (uint32_t k = 0; k <= 360u; ++k) {
        const float f = s.frac(t + k);
        KT_CHECK(f >= prev && f >= 0.f && f <= 1.f);
        prev = f;
    }
    // stow lands exactly on the hip
    s.target(false, t + 1000u, 350u, 500u);
    KT_CHECK(s.len == 500u && s.frac(t + 1000u) == 1.f && s.frac(t + 1500u) == 0.f && s.frac(t + 1499u) > 0.f);
    KT_CHECK(exactPos(slideShown(s, t + 1500u, hip, hand), hip) && exactPos(slideShown(s, t + 1000u, hip, hand), hand));
}

static void testSlideRetargetContinuity() {
    const uint32_t t = 40000u;
    Slide s;
    s.reset(false, t);
    s.target(true, t, 350u, 1000u);
    KT_CHECK(closeTo(s.frac(t + 175u), 0.5f));
    s.target(false, t + 175u, 350u, 1000u);  // stow edge at fraction 0.5
    KT_CHECK(s.dir == 0 && closeTo(s.from, 0.5f) && s.t0 == t + 175u);
    KT_CHECK(closeTo(s.frac(t + 175u), 0.5f));  // no pop right after the retarget
    KT_CHECK(s.len == 500u);                     // half of the 1000 ms stow
    KT_CHECK(s.frac(t + 175u + 500u) == 0.f && s.frac(t + 175u + 499u) > 0.f);
    KT_CHECK(closeTo(s.frac(t + 175u + 250u), 0.25f));
    // and back to the hand from 0.25: 3/4 of the draw time
    s.target(true, t + 175u + 250u, 400u, 1000u);
    KT_CHECK(closeTo(s.frac(t + 425u), 0.25f) && s.len == 300u && s.dir == 1);
    // distance is |want - from|, so a retarget from above the want and from below both measure the remaining gap
    Slide u;
    u.reset(true, t);
    u.target(false, t, 100u, 1000u);
    KT_CHECK(u.len == 1000u);            // full stow
    u.target(true, t + 500u, 200u, 1000u);  // at 0.5 heading back up: 0.5 * 200
    KT_CHECK(u.len == 100u && closeTo(u.from, 0.5f));
    // rounding and the minimum length of 1
    Slide r;
    r.reset(false, t);
    r.target(true, t, 3u, 3u);
    r.target(false, t + 1u, 3u, 3u);  // from 1/3, dist 1/3 * 3 = 1
    KT_CHECK(r.len == 1u);
    Slide m;
    m.reset(false, t);
    m.target(true, t, 1000u, 1000u);
    m.target(false, t + 1u, 1u, 1u);  // from 0.001, dist tiny, ms 1 -> rounds to 0 -> min 1
    KT_CHECK(m.len == 1u && m.dir == 0);
    Slide h;
    h.reset(false, t);
    h.target(true, t, 10u, 10u);
    h.target(false, t + 5u, 10u, 10u);  // from 0.5 -> 5 ms (round half up of 5.0)
    KT_CHECK(h.len == 5u);
    Slide q;
    q.reset(false, t);
    q.target(true, t, 100u, 100u);
    q.target(false, t + 4u, 11u, 11u);  // from 0.04, 0.44 -> rounds to 0
    KT_CHECK(q.len == 1u);
    Slide w;
    w.reset(false, t);
    w.target(true, t, 100u, 100u);
    w.target(false, t + 30u, 100u, 100u);  // 0.3 * 100 = 30
    KT_CHECK(w.len == 30u);
    Slide z;
    z.reset(false, t);
    z.target(true, t, 100u, 100u);
    z.target(false, t + 65u, 10u, 7u);  // 0.65 * 7 = 4.55 -> 5
    KT_CHECK(z.len == 5u);
    Slide y;
    y.reset(false, t);
    y.target(true, t, 100u, 100u);
    y.target(false, t + 62u, 10u, 7u);  // 0.62 * 7 = 4.34 -> 4
    KT_CHECK(y.len == 4u);
}

static void testSlideNoOpWhenAlreadyHeadingThere() {
    const uint32_t t = 9000u;
    Slide s;
    s.reset(false, t);
    s.target(false, t + 50u, 350u, 350u);  // already heading to the hip
    KT_CHECK(s.t0 == t && s.len == 0 && s.last == t && s.frac(t + 60u) == 0.f);
    s.target(true, t + 100u, 350u, 350u);
    KT_CHECK(s.t0 == t + 100u && s.len == 350u);
    s.target(true, t + 200u, 50u, 50u);  // already heading to the hand: nothing changes
    KT_CHECK(s.t0 == t + 100u && s.len == 350u && s.from == 0.f && s.dir == 1);
    KT_CHECK(closeTo(s.frac(t + 100u + 175u), 0.5f));
    // heading to the hand but already arrived: still a no-op
    s.target(true, t + 5000u, 50u, 50u);
    KT_CHECK(s.t0 == t + 100u && s.frac(t + 5000u) == 1.f);
}

static void testSlideInstantWhenZeroMs() {
    const uint32_t t = 77000u;
    Slide s;
    s.reset(false, t);
    s.target(true, t + 10u, 0u, 500u);  // draw ms 0: in the hand at once
    KT_CHECK(s.len == 0u && s.dir == 1);
    KT_CHECK(s.frac(t + 10u) == 1.f && s.frac(t + 11u) == 1.f && !s.active(t + 11u));
    s.target(false, t + 20u, 0u, 0u);  // stow ms 0: at the hip at once
    KT_CHECK(s.len == 0u && s.dir == 0 && s.frac(t + 20u) == 0.f && s.frac(t + 21u) == 0.f);
    // zero in the OTHER direction's ms does not make this one instant
    s.target(true, t + 30u, 0u, 0u);
    s.target(false, t + 40u, 0u, 300u);
    KT_CHECK(s.len == 300u && s.dir == 0);  // an instant hand edge left it at 1, so the stow takes the full 300 ms
}

static void testSlideTickWrap() {
    const Pose hip = mountPose(), hand = handPose();
    // forward: starts 16 ticks before the counter wraps
    {
        const uint32_t t0 = 0xFFFFFFF0u;
        Slide s;
        s.reset(false, t0 - 5u);
        s.target(true, t0, 350u, 350u);
        float prev = -1.f;
        for (uint32_t k = 0; k <= 360u; k += 10u) {
            const float f = s.frac(t0 + k);  // wraps past 0 at k = 16
            KT_CHECK(closeTo(f, std::fmin(1.f, k / 350.f)));
            KT_CHECK(f >= prev);
            prev = f;
            KT_CHECK(poseSane(slideShown(s, t0 + k, hip, hand)));
        }
        KT_CHECK(s.frac(t0 + 350u) == 1.f && s.frac(t0 + 349u) < 1.f && s.frac(t0) == 0.f);
        KT_CHECK(s.active(t0 + 20u) && s.active(t0 + 1u) && !s.active(t0) && !s.active(t0 + 350u));
        KT_CHECK(s.frac(t0 - 1u) == 0.f);  // stale tick on the other side of the wrap
    }
    // backward: the start is just after the wrap and `now` ticks run before it
    {
        const uint32_t t0 = 10u;
        Slide s;
        s.reset(false, 0u);
        s.target(true, t0, 350u, 350u);
        KT_CHECK(s.frac(t0 - 20u) == 0.f);  // 0xFFFFFFFA..: stale, d is negative
        KT_CHECK(s.frac(0xFFFFFFF0u) == 0.f);
        KT_CHECK(closeTo(s.frac(t0 + 175u), 0.5f) && s.frac(t0 + 350u) == 1.f);
    }
    // stow slide that crosses the wrap
    {
        const uint32_t t0 = 0xFFFFFF00u;
        Slide s;
        s.reset(true, t0);
        s.target(false, t0, 350u, 400u);
        KT_CHECK(closeTo(s.frac(t0 + 200u), 0.5f) && s.frac(t0 + 400u) == 0.f && s.frac(t0 + 399u) > 0.f);
    }
}

static void testSlideStaleReturnsFrom() {
    const uint32_t t = 5000u;
    Slide s;
    s.reset(false, t - 100u);
    s.target(true, t, 350u, 350u);
    KT_CHECK(s.frac(t - 1u) == 0.f && s.frac(t - 100u) == 0.f);
    s.target(false, t + 175u, 350u, 350u);  // from 0.5
    KT_CHECK(closeTo(s.frac(t + 100u), 0.5f) && closeTo(s.frac(t + 174u), 0.5f));  // now < t0: the slide has not started
}

static void testSlideGapGuard() {
    const uint32_t t = 20000u;
    Slide s;
    s.reset(false, t);
    s.target(true, t, 400u, 400u);
    s.touch(t + 50u);
    s.touch(t + 100u);  // normal 50 ms ticks: no shift
    KT_CHECK(s.t0 == t && s.last == t + 100u);
    const float before = s.frac(t + 100u);
    KT_CHECK(closeTo(before, 0.25f));
    s.touch(t + 200u);  // exactly 100 ms: not a stall
    KT_CHECK(s.t0 == t && s.last == t + 200u);
    s.touch(t + 301u);  // 101 ms gap: shifts t0 by the gap
    KT_CHECK(s.t0 == t + 101u && s.last == t + 301u);
    KT_CHECK(closeTo(s.frac(t + 301u), 0.5f));  // resumes exactly where it was at the last call
    s.touch(t + 301u + 5000u);  // long stall (pause)
    KT_CHECK(closeTo(s.frac(t + 301u + 5000u), 0.5f) && s.active(t + 301u + 5000u));
    KT_CHECK(s.frac(t + 301u + 5000u + 200u) == 1.f);  // the other half still plays out
    KT_CHECK(closeTo(s.frac(t + 301u + 5000u + 100u), 0.75f));
    // a finished slide is not shifted
    Slide d;
    d.reset(false, t);
    d.target(true, t, 100u, 100u);
    d.touch(t + 90u);
    d.touch(t + 400u);  // last = t+90 < t0 + len: it was still running at the last call, so it resumes
    KT_CHECK(d.t0 == t + 310u);
    d.touch(t + 410u);  // 10 ms tick; last is now exactly at the end of the slide (last - t0 == len)
    d.touch(t + 810u);  // gap 400 but the slide had finished at the last call: no shift
    KT_CHECK(d.t0 == t + 310u && d.frac(t + 810u) == 1.f);
    // an idle slide (reset, len 0) is never shifted
    Slide r;
    r.reset(true, t);
    r.touch(t + 9999u);
    KT_CHECK(r.t0 == t && r.last == t + 9999u && r.len == 0u);
    // wrap-safe gap
    Slide w;
    const uint32_t wt = 0xFFFFFFC0u;
    w.reset(false, wt);
    w.target(true, wt, 400u, 400u);
    w.touch(wt + 200u);  // across the wrap, gap 200
    KT_CHECK(w.t0 == wt + 200u && w.last == wt + 200u && w.frac(wt + 200u) == 0.f);
    w.touch(wt + 300u);
    KT_CHECK(w.t0 == wt + 200u && closeTo(w.frac(wt + 300u), 0.25f));
    // a clock that stepped backwards is not a stall
    Slide b;
    b.reset(false, t);
    b.target(true, t, 400u, 400u);
    b.touch(t + 50u);
    b.touch(t + 10u);
    KT_CHECK(b.t0 == t && b.last == t + 10u);
    // touch right after a retarget: target stamped last, so the retarget tick itself is not a gap
    Slide g;
    g.reset(false, t);
    g.touch(t + 10u);
    g.target(true, t + 5000u, 400u, 400u);
    g.touch(t + 5000u);
    KT_CHECK(g.t0 == t + 5000u);
}

static void testSlideLeftRightIndependent() {
    const uint32_t t = 3000u;
    Slide l, r;
    l.reset(false, t);
    r.reset(false, t);
    l.target(true, t, 350u, 350u);
    KT_CHECK(l.dir == 1 && r.dir == 0 && r.len == 0u);
    KT_CHECK(closeTo(l.frac(t + 175u), 0.5f) && r.frac(t + 175u) == 0.f);
    r.target(true, t + 100u, 200u, 200u);
    KT_CHECK(l.t0 == t && l.len == 350u && r.t0 == t + 100u && r.len == 200u);
    KT_CHECK(l.frac(t + 350u) == 1.f && closeTo(r.frac(t + 200u), 0.5f));
}

static void testSlideClamps() {
    KT_CHECK(clampDrawSlideMs(-5) == 0u && clampDrawSlideMs(-1) == 0u && clampDrawSlideMs(0) == 0u && clampDrawSlideMs(1) == 1u);
    KT_CHECK(clampDrawSlideMs(350) == 350u && clampDrawSlideMs(569) == 569u && clampDrawSlideMs(570) == 570u);
    KT_CHECK(clampDrawSlideMs(571) == 570u && clampDrawSlideMs(100000L) == 570u && clampDrawSlideMs(-100000L) == 0u);
    KT_CHECK(clampStowSlideMs(-5) == 0u && clampStowSlideMs(-1) == 0u && clampStowSlideMs(0) == 0u && clampStowSlideMs(1) == 1u);
    KT_CHECK(clampStowSlideMs(500) == 500u && clampStowSlideMs(999) == 999u && clampStowSlideMs(1000) == 1000u);
    KT_CHECK(clampStowSlideMs(1001) == 1000u && clampStowSlideMs(100000L) == 1000u && clampStowSlideMs(-100000L) == 0u);
}

static void testSlideNaNFree() {
    const Pose hip = mountPose(), hand = handPose();
    Slide s;
    s.reset(false, 0u);
    s.target(true, 1u, 1u, 1u);
    for (uint32_t k = 0; k < 6u; ++k) {
        const float f = s.frac(k);
        KT_CHECK(f == f && f >= 0.f && f <= 1.f);
        KT_CHECK(poseSane(slideShown(s, k, hip, hand)));
    }
}

// Which half of the 4-sphere the blend takes is decided by the sign of the raw quat dot product. One case per term: a single
// strongly negative product, so flipping that term's sign in the dot would pick the long arc. Also the exact-zero dot.
static void testBlendShortArcPerTerm() {
    const Q m0 = qnormalize({0.5f, 0.5f, 0.5f, 0.5f});
    const Q hs4[4] = { {0.2f, -0.9f, 0.2f, 0.2f}, {0.2f, 0.2f, -0.9f, 0.2f}, {0.2f, 0.2f, 0.2f, -0.9f}, {-0.9f, 0.2f, 0.2f, 0.2f} };
    for (int i = 0; i < 4; ++i) {
        Pose m = { {0.f, 0.f, 0.f}, m0 };
        Pose h = { {1.f, 1.f, 1.f}, qnormalize(hs4[i]) };
        KT_CHECK(dot(m.q, h.q) < 0.f);
        const Q n = { -h.q.w, -h.q.x, -h.q.y, -h.q.z };
        const Q want = qnormalize({ 0.5f * m.q.w + 0.5f * n.w, 0.5f * m.q.x + 0.5f * n.x, 0.5f * m.q.y + 0.5f * n.y, 0.5f * m.q.z + 0.5f * n.z });
        KT_CHECK(sameQuat(blendPose(m, h, 0.5f).q, want));
    }
    // dot exactly 0: the hand quat is used as is (no flip)
    Pose m = { {0.f, 0.f, 0.f}, {1.f, 0.f, 0.f, 0.f} };
    Pose h = { {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f} };
    KT_CHECK(dot(m.q, h.q) == 0.f);
    KT_CHECK(sameQuat(blendPose(m, h, 0.5f).q, qnormalize({0.5f, 0.5f, 0.f, 0.f})));
}

static void testSlideDefaultsAndTinySlides() {
    const Slide d;
    KT_CHECK(d.from == 0.f && d.t0 == 0u && d.len == 0u && d.last == 0u && d.dir == 0 && d.init == 0);
    // a 1 ms slide has no tick strictly inside it, yet the gap guard still shifts it
    const uint32_t t = 8000u;
    Slide s;
    s.reset(false, t);
    s.target(true, t, 1u, 1u);
    KT_CHECK(s.len == 1u);
    s.touch(t + 500u);  // last == t0 so last - t0 = 0 < len 1: still pending, shifted by the stall
    KT_CHECK(s.t0 == t + 500u);
    // retarget on a stale tick right after a stow: from is 1 and the want is 1, nothing to travel
    Slide u;
    u.reset(true, t);
    u.target(false, t, 100u, 100u);
    KT_CHECK(u.frac(t - 5u) == 1.f && u.from == 1.f);
    u.target(true, t - 5u, 100u, 100u);
    KT_CHECK(u.len == 0u && u.dir == 1 && u.frac(t - 4u) == 1.f && u.frac(t + 1u) == 1.f);
    // and the mirror: a hip stale tick
    Slide v;
    v.reset(false, t);
    v.target(true, t, 100u, 100u);
    v.target(false, t - 5u, 100u, 100u);
    KT_CHECK(v.len == 0u && v.dir == 0 && v.frac(t + 1u) == 0.f);
}

// frac must land exactly on the end at d == len for every from / len (the d >= len branch, not the lerp formula).
static void testSlideExactEndSweep() {
    const uint32_t t = 1000u;
    for (uint32_t ms = 1u; ms <= 570u; ++ms) {
        for (uint32_t cut = 1u; cut < ms; cut += 7u) {
            Slide s;
            s.reset(false, t);
            s.target(true, t, ms, ms);
            s.target(false, t + cut, ms, ms);
            KT_CHECK(s.frac(t + cut + s.len) == 0.f);
            s.target(true, t + cut + s.len / 2u, ms, ms);
            KT_CHECK(s.frac(s.t0 + s.len) == 1.f);
        }
    }
}

int main() {
    testBlendEndpointsAndClamp();
    testEaseSmooth();
    testBlendPosMonotoneAlongSegment();
    testShortArcOnAntipodalHand();
    testFsmDrawSlide();
    testFsmStowSlideReturnsToMount();
    testSlideAcrossCounterWrap();
    testSlideResetIsSilent();
    testSlideEndpointsAndMid();
    testSlideRetargetContinuity();
    testSlideNoOpWhenAlreadyHeadingThere();
    testSlideInstantWhenZeroMs();
    testSlideTickWrap();
    testSlideStaleReturnsFrom();
    testSlideGapGuard();
    testSlideLeftRightIndependent();
    testSlideClamps();
    testSlideNaNFree();
    testBlendShortArcPerTerm();
    testSlideDefaultsAndTinySlides();
    testSlideExactEndSweep();
    return kt::Report("holster_slide");
}
