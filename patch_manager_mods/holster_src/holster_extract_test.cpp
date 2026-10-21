// Host tests for the HW31 extract schedule (doc 42, HW31 W6a): reachPlan extract share extractK and orientation progress oriF,
// reachRetarget extract continuity on a reversal, reachViaQ and blendPoseLocked2 (HolsterReach.h / HolsterSlide.h).
// Build/run: python3 scripts/run_tests.py holster_extract
#include "HolsterReach.h"
#include "check.h"
#include <math.h>
#include <stdint.h>

using namespace hs;

// Old reachPlan body (before HW31), kept as the reference: with extractPct 0 and extractFrom 0 the new plan must match it exactly.
struct RefPlan { bool active; float pathF, restMix, carryW; };
static RefPlan reachPlanRef(const Slide& s, uint32_t now, float g) {
    RefPlan r = { false, 0.f, 0.f, 0.f };
    const int32_t d = (int32_t)(now - s.t0);
    if (d < 0 || d >= (int32_t)s.len) return r;
    if (!(g > 0.f)) g = 0.f;
    if (g > 0.6f) g = 0.6f;
    const float p = slideClamp01((float)d / (float)s.len);
    const float from = slideClamp01(s.from);
    r.active = true;
    if (s.dir) {
        if (from <= 0.001f) {
            if (g > 0.f && p < g) { r.pathF = s.pathFrom * (1.f - easeSmooth(p / g)); return r; }
            const float q = (p - g) / (1.f - g);
            r.pathF = reachCarryF(q); r.carryW = easeSmooth(q / kCarryEaseLen); return r;
        }
        r.pathF = s.pathFrom + (1.f - s.pathFrom) * reachCarryF(p);
        r.carryW = s.carryFrom + (1.f - s.carryFrom) * easeSmooth(p / kCarryEaseLen);
        return r;
    }
    const float c = 1.f - g;
    if (p < c) {
        const float q = p / c;
        r.pathF = s.pathFrom * reachCarryF(1.f - q);
        r.carryW = s.carryFrom * easeSmooth((1.f - q) / kCarryEaseLen);
        return r;
    }
    r.pathF = easeSmooth((p - c) / g); r.carryW = 0.f;
    return r;
}

static bool closeTo(float a, float b, float tol) { return fabsf(a - b) <= tol; }

// All fields of two plans equal within tol (tol 0 = exact).
static bool samePlan(const ReachPlan& a, const ReachPlan& b, float tol) {
    return a.active == b.active && closeTo(a.pathF, b.pathF, tol) && closeTo(a.restMix, b.restMix, tol) && closeTo(a.carryW, b.carryW, tol)
        && closeTo(a.oriF, b.oriF, tol) && closeTo(a.extractK, b.extractK, tol);
}

// The new plan with extract share 0 matches the old reference bit for bit, and oriF == pathF, extractK == 0.
static bool sameAsRef(const ReachPlan& n, const RefPlan& r) {
    return n.active == r.active && n.pathF == r.pathF && n.restMix == r.restMix && n.carryW == r.carryW && n.oriF == n.pathF && n.extractK == 0.f;
}

// Every 1 ms step of the slide moves pathF, oriF and extractK by less than tol.
static bool plansContinuous(const Slide& s, float g, float e, float tol) {
    ReachPlan prev = reachPlan(s, 0, g, e);
    for (uint32_t t = 1; t < s.len; ++t) {
        const ReachPlan cur = reachPlan(s, t, g, e);
        if (!(fabsf(cur.pathF - prev.pathF) < tol && fabsf(cur.oriF - prev.oriF) < tol && fabsf(cur.extractK - prev.extractK) < tol)) return false;
        prev = cur;
    }
    return true;
}

static bool samePose(const Pose& a, const Pose& b, float tol) {
    for (int i = 0; i < 3; ++i) if (!closeTo(a.pos[i], b.pos[i], tol)) return false;
    return closeTo(a.q.w, b.q.w, tol) && closeTo(a.q.x, b.q.x, tol) && closeTo(a.q.y, b.q.y, tol) && closeTo(a.q.z, b.q.z, tol);
}

static void makeDraw(Slide& s, uint32_t ms) { s.reset(false, 0); s.target(true, 0, ms, ms); }
static void makeStow(Slide& s, uint32_t ms) { s.reset(true, 0); s.target(false, 0, ms, ms); }

// Test 1: extract share 0 reproduces the old plan on a fresh draw, a fresh stow, and a mid-carry reversal.
static void testIdentityExtract0() {
    Slide s;
    s.reset(false, 0); s.target(true, 0, 1000, 1000);
    for (uint32_t t = 0; t < 1000; ++t) KT_CHECK(sameAsRef(reachPlan(s, t, 0.2f), reachPlanRef(s, t, 0.2f)));
    reachRetarget(s, false, 500, 1000, 1000, 0.2f);
    KT_CHECK(s.extractFrom == 0.f);
    for (uint32_t t = 500; t < 1500; ++t) KT_CHECK(sameAsRef(reachPlan(s, t, 0.2f), reachPlanRef(s, t, 0.2f)));
    s.reset(true, 0); s.target(false, 0, 1000, 1000);
    for (uint32_t t = 0; t < 1000; ++t) KT_CHECK(sameAsRef(reachPlan(s, t, 0.2f), reachPlanRef(s, t, 0.2f)));
}

// Test 2: pathF and carryW do not depend on the extract share; the endpoints of oriF and extractK hold.
static void testEndpoints() {
    Slide d, st;
    makeDraw(d, 1000); makeStow(st, 1000);
    for (uint32_t t = 0; t < 1000; ++t) {
        const ReachPlan a = reachPlan(d, t, 0.2f, 0.4f);
        const ReachPlan b = reachPlan(d, t, 0.2f);
        KT_CHECK(a.active == b.active && a.pathF == b.pathF && a.carryW == b.carryW);
        const ReachPlan c = reachPlan(st, t, 0.2f, 0.4f);
        const ReachPlan e = reachPlan(st, t, 0.2f);
        KT_CHECK(c.active == e.active && c.pathF == e.pathF && c.carryW == e.carryW);
    }
    KT_CHECK(reachPlan(d, 0, 0.2f, 0.4f).extractK == 0.f);
    KT_CHECK(reachPlan(st, 0, 0.2f, 0.4f).extractK == 0.f);
    KT_CHECK(reachPlan(d, 999, 0.2f, 0.4f).oriF >= 0.99f);
    KT_CHECK(reachPlan(st, 0, 0.2f, 0.4f).oriF >= 0.999f);
}

// Test 3: draw window is ticks 200..520 (g 0.2, e 0.4, len 1000); extractK peaks in the middle; oriF holds 0 inside it.
static void testDrawWindow() {
    Slide d;
    makeDraw(d, 1000);
    for (uint32_t t = 0; t < 1000; ++t) {
        const ReachPlan r = reachPlan(d, t, 0.2f, 0.4f);
        if (t < 200 || t > 520) KT_CHECK(r.extractK < 1e-4f);
        if (t >= 200 && t <= 520) KT_CHECK(r.oriF < 1e-4f);
    }
    KT_CHECK(reachPlan(d, 200, 0.2f, 0.4f).extractK < 1e-3f);
    KT_CHECK(reachPlan(d, 520, 0.2f, 0.4f).extractK < 1e-3f);
    KT_CHECK(fabsf(reachPlan(d, 360, 0.2f, 0.4f).extractK - 1.f) < 1e-3f);
}

// Test 4: stow window is ticks 480..800; oriF holds 0 inside it and falls from 1 to 0 across the carry before it.
static void testStowWindow() {
    Slide st;
    makeStow(st, 1000);
    for (uint32_t t = 0; t < 1000; ++t) {
        const ReachPlan r = reachPlan(st, t, 0.2f, 0.4f);
        if (t < 480 || t > 800) KT_CHECK(r.extractK < 1e-4f);
        if (t >= 480 && t <= 800) KT_CHECK(r.oriF < 1e-4f);
    }
    KT_CHECK(reachPlan(st, 480, 0.2f, 0.4f).extractK < 1e-3f);
    KT_CHECK(reachPlan(st, 800, 0.2f, 0.4f).extractK < 1e-3f);
    KT_CHECK(fabsf(reachPlan(st, 640, 0.2f, 0.4f).extractK - 1.f) < 1e-3f);
    KT_CHECK(reachPlan(st, 0, 0.2f, 0.4f).oriF >= 0.999f);
    float prevO = reachPlan(st, 0, 0.2f, 0.4f).oriF;
    for (uint32_t t = 1; t <= 480; ++t) {
        const float o = reachPlan(st, t, 0.2f, 0.4f).oriF;
        KT_CHECK(o <= prevO + 1e-6f);
        prevO = o;
    }
    KT_CHECK(prevO < 1e-4f);
}

// Test 5: no 1 ms step moves pathF, oriF or extractK by 0.02 or more (g 0 with len 500, and g 0.2 with len 1000, draw and stow).
static void testContinuity() {
    Slide d, st, d2, st2;
    makeDraw(d, 500); makeStow(st, 500);
    makeDraw(d2, 1000); makeStow(st2, 1000);
    KT_CHECK(plansContinuous(d, 0.f, 0.4f, 0.02f));
    KT_CHECK(plansContinuous(st, 0.f, 0.4f, 0.02f));
    KT_CHECK(plansContinuous(d2, 0.2f, 0.4f, 0.02f));
    KT_CHECK(plansContinuous(st2, 0.2f, 0.4f, 0.02f));
}

// Test 6: a draw reversed at tick 100 (inside its extract window) to a stow: extractK, pathF and oriF continue across the join and to the end.
static void testReversalMidExtract() {
    Slide s;
    makeDraw(s, 500);
    const Slide old = s;
    const ReachPlan before = reachPlan(old, 99, 0.f, 0.4f);
    const ReachPlan oldAt100 = reachPlan(old, 100, 0.f, 0.4f);
    KT_CHECK(oldAt100.extractK > 0.99f);
    reachRetarget(s, false, 100, 500, 500, 0.f, 0.04f, 0.4f);
    const ReachPlan at100 = reachPlan(s, 100, 0.f, 0.4f);
    KT_CHECK(fabsf(at100.extractK - oldAt100.extractK) < 1e-5f);
    KT_CHECK(s.extractFrom > 0.99f);
    bool cont = true;
    ReachPlan prev = before;
    ReachPlan cur = at100;
    uint32_t t = 100;
    while (cur.active) {
        cont = cont && fabsf(cur.pathF - prev.pathF) < 0.02f && fabsf(cur.oriF - prev.oriF) < 0.02f && fabsf(cur.extractK - prev.extractK) < 0.02f;
        prev = cur;
        ++t;
        cur = reachPlan(s, t, 0.f, 0.4f);
    }
    KT_CHECK(cont);
    KT_CHECK(prev.extractK < 1e-3f);   // last active tick
}

// Test 7: the extract share clamps to 1 - g - 0.05 (after g clamps to 0.6); NaN and negatives give 0; e 0.6 at g 0 is not clamped.
static void testExtractPctClamp() {
    Slide d, st;
    makeDraw(d, 1000); makeStow(st, 1000);
    bool ok = true;
    for (uint32_t t = 0; t < 1000; ++t) {
        ok = ok && samePlan(reachPlan(d, t, 0.6f, 0.9f), reachPlan(d, t, 0.6f, 1.f - 0.6f - 0.05f), 1e-6f);
        ok = ok && samePlan(reachPlan(st, t, 0.6f, 0.9f), reachPlan(st, t, 0.6f, 1.f - 0.6f - 0.05f), 1e-6f);
        ok = ok && samePlan(reachPlan(d, t, 0.6f, NAN), reachPlan(d, t, 0.6f, 0.f), 0.f);
        ok = ok && samePlan(reachPlan(st, t, 0.6f, NAN), reachPlan(st, t, 0.6f, 0.f), 0.f);
        ok = ok && samePlan(reachPlan(d, t, 0.6f, -0.5f), reachPlan(d, t, 0.6f, 0.f), 0.f);
        ok = ok && samePlan(reachPlan(st, t, 0.6f, -0.5f), reachPlan(st, t, 0.6f, 0.f), 0.f);
    }
    KT_CHECK(ok);
    Slide d6;
    makeDraw(d6, 500);   // g 0, e 0.6: the window is ticks 0..300 and peaks at tick 150; a 0.95 clamp would move the peak
    KT_CHECK(fabsf(reachPlan(d6, 150, 0.f, 0.6f).extractK - 1.f) < 1e-3f);
    KT_CHECK(reachPlan(d6, 300, 0.f, 0.6f).extractK < 1e-3f);
}

// Test 8: drawMs 0 and stowMs 0 give len 0 (inactive, all outputs 0); a ms-0 retarget on a fresh slide does not crash.
static void testMsZero() {
    Slide d, st;
    d.reset(false, 0); d.target(true, 0, 0, 0);
    st.reset(true, 0); st.target(false, 0, 0, 0);
    KT_CHECK(d.len == 0 && st.len == 0);
    for (uint32_t t = 0; t < 4; ++t) {
        const ReachPlan a = reachPlan(d, t, 0.2f, 0.4f);
        const ReachPlan b = reachPlan(st, t, 0.2f, 0.4f);
        KT_CHECK(!a.active && !b.active);
        KT_CHECK(a.pathF == 0.f && a.carryW == 0.f && a.oriF == 0.f && a.extractK == 0.f);
        KT_CHECK(b.pathF == 0.f && b.carryW == 0.f && b.oriF == 0.f && b.extractK == 0.f);
    }
    Slide f;
    f.reset(false, 0);
    reachRetarget(f, true, 0, 0, 0, 0.2f, 0.04f, 0.4f);
    KT_CHECK(f.len == 0 && f.extractFrom == 0.f);
}

// Test 9: reachViaQ: endpoints, via at t 0.5, NaN t clamps to 0, per-step continuity, and hemisphere invariance.
static void testViaQ() {
    const float zAx[3] = { 0.f, 0.f, 1.f };
    const float xAx[3] = { 1.f, 0.f, 0.f };
    const Q a = { 1.f, 0.f, 0.f, 0.f };
    const Q b = qaxis(zAx, 90.f);
    const Q via = qaxis(xAx, 90.f);
    const Q na = { -1.f, 0.f, 0.f, 0.f };
    const Q nvia = { -via.w, -via.x, -via.y, -via.z };
    KT_CHECK(fabsf(qdot(reachViaQ(a, b, via, 0.f), a)) > 1.f - 1e-5f);
    KT_CHECK(fabsf(qdot(reachViaQ(a, b, via, 1.f), b)) > 1.f - 1e-5f);
    KT_CHECK(fabsf(qdot(reachViaQ(a, b, via, 0.5f), via)) > 0.99985f);
    KT_CHECK(fabsf(qdot(reachViaQ(a, b, via, NAN), a)) > 1.f - 1e-5f);   // NaN t clamps to 0 (pinned)
    const Q nanQ = { NAN, 0.f, 0.f, 0.f };
    KT_CHECK(fabsf(qdot(reachViaQ(nanQ, b, via, 1.f), b)) > 1.f - 1e-5f);   // non-finite a is identity: ends at b
    KT_CHECK(fabsf(qdot(reachViaQ(a, nanQ, via, 0.7f), a)) > 1.f - 1e-5f);  // non-finite b keeps a (blendQ)
    Q prev = reachViaQ(a, b, via, 0.f);
    bool cont = true;
    for (int i = 1; i <= 1000; ++i) {
        const Q cur = reachViaQ(a, b, via, (float)i / 1000.f);
        cont = cont && fabsf(qdot(prev, cur)) > 1.f - 1e-4f;
        prev = cur;
    }
    KT_CHECK(cont);
    bool hemi = true;
    for (int i = 0; i <= 1000; ++i) {
        const float t = (float)i / 1000.f;
        const Q r0 = reachViaQ(a, b, via, t);
        const Q rv = reachViaQ(a, b, nvia, t);
        const Q ra = reachViaQ(na, b, via, t);
        hemi = hemi && fabsf(fabsf(qdot(rv, r0)) - 1.f) < 1e-5f && fabsf(fabsf(qdot(ra, r0)) - 1.f) < 1e-5f;
    }
    KT_CHECK(hemi);
}

// Test 10: blendPoseLocked is exactly blendPoseLocked2(f, f, nullptr); blendPoseLocked2 keeps the position and via contracts; NaN via = absent.
static void testBlendPoseLockedEq() {
    const float yAx[3] = { 0.f, 1.f, 0.f };
    const float xAx[3] = { 1.f, 0.f, 0.f };
    const Pose m = { { 0.f, 0.f, 0.f }, { 1.f, 0.f, 0.f, 0.f } };
    const float fs[5] = { 0.f, 0.25f, 0.5f, 1.f, NAN };
    Slide s1, s2;
    s1.reset(false, 0); s2.reset(false, 0);
    bool eq = true;
    for (int deg = 0; deg < 358; deg += 7) {
        const Pose h = { { 1.f, 2.f, 3.f }, qaxis(yAx, (float)deg) };
        for (int k = 0; k < 5; ++k) eq = samePose(blendPoseLocked(s1, m, h, fs[k]), blendPoseLocked2(s2, m, h, fs[k], fs[k], nullptr), 0.f) && eq;
    }
    Pose hn = { { 1.f, 2.f, 3.f }, qaxis(yAx, 42.f) };   // the same rotation with the quat negated
    hn.q = Q{ -hn.q.w, -hn.q.x, -hn.q.y, -hn.q.z };
    for (int k = 0; k < 5; ++k) eq = samePose(blendPoseLocked(s1, m, hn, fs[k]), blendPoseLocked2(s2, m, hn, fs[k], fs[k], nullptr), 0.f) && eq;
    KT_CHECK(eq);

    const Pose h1 = { { 1.f, 2.f, 3.f }, qaxis(yAx, 90.f) };
    Slide s3, s4, s5, s6;
    s3.reset(false, 0); s4.reset(false, 0); s5.reset(false, 0); s6.reset(false, 0);
    const Pose o = blendPoseLocked2(s3, m, h1, 0.f, 1.f, nullptr);
    KT_CHECK(o.pos[0] == m.pos[0] && o.pos[1] == m.pos[1] && o.pos[2] == m.pos[2]);
    KT_CHECK(fabsf(qdot(o.q, h1.q)) > 0.9999f);
    const Q via = qaxis(xAx, 90.f);
    const Pose ov = blendPoseLocked2(s4, m, h1, 0.5f, 0.5f, &via);
    KT_CHECK(fabsf(qdot(ov.q, via)) > 0.99985f);
    const Q nanV = { NAN, 0.f, 0.f, 0.f };
    KT_CHECK(samePose(blendPoseLocked2(s5, m, h1, 0.5f, 0.5f, &nanV), blendPoseLocked2(s6, m, h1, 0.5f, 0.5f, nullptr), 0.f));
}

// Test 11 (added for the HW31 decision on the oriF hold): the hold is confined to the carry window. Reach and release follow pathF.
static void testOriFOutsideCarry() {
    Slide d, st;
    makeDraw(d, 1000); makeStow(st, 1000);
    const ReachPlan dr = reachPlan(d, 150, 0.2f, 0.4f);     // draw reach tail: pathF 0.156 < X, no hold
    KT_CHECK(dr.oriF == dr.pathF && dr.oriF > 0.1f);
    const ReachPlan sr = reachPlan(st, 840, 0.2f, 0.4f);    // stow release head: pathF easeSmooth(0.2) = 0.104, no hold
    KT_CHECK(sr.oriF == sr.pathF && sr.oriF > 0.05f);
    for (uint32_t t = 0; t < 200; ++t) { const ReachPlan r = reachPlan(d, t, 0.2f, 0.4f); KT_CHECK(r.oriF == r.pathF); }
    for (uint32_t t = 800; t < 1000; ++t) { const ReachPlan r = reachPlan(st, t, 0.2f, 0.4f); KT_CHECK(r.oriF == r.pathF); }
    const float restDraw = reachPlan(d, 600, 0.2f, 0.4f).oriF;   // draw carry after the extract window: smooth, between 0 and 1
    KT_CHECK(restDraw > 0.05f && restDraw < 0.1f);
}

// Reverses s at tRev (hand = the new direction) and walks the new slide: |delta oriF| < 0.02 per tick across the join and along it.
static bool reverseKeepsOriF(Slide& s, uint32_t tRev, bool hand) {
    const Slide old = s;
    ReachPlan prev = reachPlan(old, tRev - 1, 0.2f, 0.4f);
    reachRetarget(s, hand, tRev, 1000, 1000, 0.2f, 0.04f, 0.4f);
    bool ok = true;
    uint32_t t = tRev;
    ReachPlan cur = reachPlan(s, t, 0.2f, 0.4f);
    while (cur.active) {
        ok = ok && fabsf(cur.oriF - prev.oriF) < 0.02f;
        prev = cur;
        ++t;
        cur = reachPlan(s, t, 0.2f, 0.4f);
    }
    return ok && t > tRev + 10;
}

// Test 12 (B1): oriF stays continuous on a reversal during the reach (draw at tick 100) and during the release (stow at tick 850).
static void testReversalOriFJoin() {
    Slide d, st;
    makeDraw(d, 1000); makeStow(st, 1000);
    KT_CHECK(reverseKeepsOriF(d, 100, false));    // draw in the reach, reversed to a stow
    KT_CHECK(reverseKeepsOriF(st, 850, true));    // stow in the release, reversed to a draw
}

int main() {
    testIdentityExtract0();
    testEndpoints();
    testDrawWindow();
    testStowWindow();
    testContinuity();
    testReversalMidExtract();
    testExtractPctClamp();
    testMsZero();
    testViaQ();
    testBlendPoseLockedEq();
    testOriFOutsideCarry();
    testReversalOriFJoin();
    return kt::Report("holster_extract");
}
