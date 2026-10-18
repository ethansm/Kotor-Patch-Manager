// T-edit host tests for HolsterEdit.h (the equip-screen editor maths of the holstered-weapons mod; doc 42, HW-EDIT).
// Build/run: python3 scripts/run_tests.py holster_edit
#include "HolsterEdit.h"
#include "check.h"
#include <cmath>
#include <stdint.h>

using namespace hs;

static bool closeTo(float a, float b, float tol = 1e-5f) { return std::fabs(a - b) <= tol; }
static bool closeAng(float a, float b, float tol = 1e-3f) { return std::fabs(wrap180(a - b)) <= tol; }
static bool sameQuat(const Q& a, const Q& b, float tol = 1e-5f) {
    return closeTo(a.w, b.w, tol) && closeTo(a.x, b.x, tol) && closeTo(a.y, b.y, tol) && closeTo(a.z, b.z, tol);
}
static float absDot(const Q& a, const Q& b) {
    return std::fabs(a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z);
}
// Largest component difference between q and the sign-matched rotation rq (both normalized).
static float quatErr(const Q& q, const Q& rq) {
    const Q a = qnormalize(q), b = qnormalize(rq);
    const float s = (a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z) < 0.f ? -1.f : 1.f;
    float e = std::fabs(a.w - s * b.w);
    e = std::fmax(e, std::fabs(a.x - s * b.x));
    e = std::fmax(e, std::fabs(a.y - s * b.y));
    e = std::fmax(e, std::fabs(a.z - s * b.z));
    return e;
}

// Deterministic LCG (fixed seed) so every run sees the same sample set.
static uint32_t gRng = 0x2026u;
static float uni(float lo, float hi) {
    gRng = gRng * 1664525u + 1013904223u;
    const float u = static_cast<float>(gRng >> 8) * (1.0f / 16777216.0f);  // [0, 1)
    return lo + (hi - lo) * u;
}
static Q randQuat() {
    const Q q = {uni(-1.f, 1.f), uni(-1.f, 1.f), uni(-1.f, 1.f), uni(-1.f, 1.f)};
    return qnormalize(q);
}

static void testRoundTripRandom() {
    for (int k = 0; k < 500; ++k) {
        const Q q = randQuat();
        float e[3];
        eulerFromQ(q, e);
        KT_CHECK(e[1] >= -90.f - 1e-3f && e[1] <= 90.f + 1e-3f);
        KT_CHECK(e[0] > -180.f - 1e-3f && e[0] <= 180.f + 1e-3f);
        KT_CHECK(e[2] > -180.f - 1e-3f && e[2] <= 180.f + 1e-3f);
        KT_CHECK(absDot(qbase(e[0], e[1], e[2]), q) > 1.f - 1e-5f);
    }
}

static void testGimbalCases() {
    const float as[] = {-170.f, -45.f, 0.f, 33.f, 120.f};
    const float cs[] = {-150.f, 0.f, 77.f, 170.f};
    const float bs[] = {90.f, -90.f};
    for (float a : as)
        for (float c : cs)
            for (float b : bs) {
                const Q q = qbase(a, b, c);
                float e[3];
                eulerFromQ(q, e);
                KT_CHECK(closeTo(e[0], 0.f, 1e-3f));
                KT_CHECK(closeTo(e[1], b, 1e-3f));
                KT_CHECK(absDot(qbase(e[0], e[1], e[2]), q) > 1.f - 1e-5f);
            }
    // Just inside the gimbal: ry = 90 - tiny, and the mirrored value.
    const float near[] = {89.99f, 89.9f, 89.5f, -89.9f};
    for (float b : near) {
        const Q q = qbase(71.f, b, -42.f);
        float e[3];
        eulerFromQ(q, e);
        KT_CHECK(absDot(qbase(e[0], e[1], e[2]), q) > 1.f - 1e-5f);
    }
}

static void testKnownVectorsRoundTrip() {
    const float v[4][3] = {{-150.f, 7.f, 94.f}, {-60.f, 4.f, 93.f}, {-90.f, -3.f, -90.f}, {150.f, 7.f, 86.f}};
    for (const auto& in : v) {
        const Q q = qbase(in[0], in[1], in[2]);
        float e[3];
        eulerFromQ(q, e);
        // |ry| < 90 makes the ZYX triple unique, so the inverse must return the input angles.
        KT_CHECK(closeAng(e[0], in[0]) && closeAng(e[1], in[1]) && closeAng(e[2], in[2]));
        KT_CHECK(absDot(qbase(e[0], e[1], e[2]), q) > 1.f - 1e-5f);
    }
}

static void testEulerNearBranchChoice() {
    // The same rotation as (170, 30, -170) and as (-10, 150, 10). prev picks which of the two equivalent triples comes back.
    const Q q = qbase(170.f, 30.f, -170.f);
    const float pPri[3] = {170.f, 30.f, -170.f};
    const float pAlt[3] = {-10.f, 150.f, 10.f};
    float out[3];
    eulerNear(q, pPri, out);
    KT_CHECK(closeAng(out[0], 170.f) && closeAng(out[1], 30.f) && closeAng(out[2], -170.f));
    eulerNear(q, pAlt, out);
    KT_CHECK(closeAng(out[0], -10.f) && closeAng(out[1], 150.f) && closeAng(out[2], 10.f));
    KT_CHECK(absDot(qbase(out[0], out[1], out[2]), q) > 1.f - 1e-5f);
}

static void testEulerNearContinuity() {
    // Random poses with |ry| < 80, then a 1 deg nudge about a random axis. Each shown angle must move by at most the amplification
    // of a 1 deg rotation through the Euler map: |d(rx)|, |d(rz)| <= 1/cos(ry) and |d(ry)| <= 1. That is 2 deg for |ry| <= 60 and
    // grows to about 5.8 deg as |ry| nears 80, so the bound is per pose rather than a flat 2 deg.
    for (int k = 0; k < 400; ++k) {
        const Q q = qbase(uni(-180.f, 180.f), uni(-79.f, 79.f), uni(-180.f, 180.f));
        float init[3];
        eulerFromQ(q, init);
        float prev[3];
        eulerNear(q, init, prev);
        float axis[3] = {uni(-1.f, 1.f), uni(-1.f, 1.f), uni(-1.f, 1.f)};
        const Q q2 = qmul(qaxis(axis, 1.f), q);
        float cur[3];
        eulerNear(q2, prev, cur);
        const float cosRy = std::cos(prev[1] * 3.14159265358979f / 180.f);
        const float bound = 1.01f / cosRy + 0.01f;
        KT_CHECK(std::fabs(wrap180(cur[0] - prev[0])) <= bound);
        KT_CHECK(std::fabs(wrap180(cur[2] - prev[2])) <= bound);
        KT_CHECK(std::fabs(wrap180(cur[1] - prev[1])) <= 1.01f);
        if (std::fabs(prev[1]) <= 60.f) {  // the flat 2 deg budget holds where 1/cos(ry) <= 2
            KT_CHECK(std::fabs(wrap180(cur[0] - prev[0])) <= 2.f);
            KT_CHECK(std::fabs(wrap180(cur[2] - prev[2])) <= 2.f);
        }
        KT_CHECK(absDot(qbase(cur[0], cur[1], cur[2]), q2) > 1.f - 1e-5f);
    }
}

static void testEulerNearSingularity() {
    // 80 <= |ry| < 90: the exact decomposition is used (no frozen rx since 0.1.4), so with prev[0] = a the primary triple wins and
    // rx comes back as a. Exactness for arbitrary prev is covered by testEulerNearNudgeSweep.
    const float bs[] = {80.f, 84.f, 89.9f, -80.f, -86.f, -89.9f};
    const float as[] = {-170.f, -20.f, 0.f, 45.f, 150.f};
    for (float b : bs)
        for (float a : as) {
            const Q q = qbase(a, b, 63.f);
            const float prev[3] = {a, 0.f, 0.f};
            float out[3];
            eulerNear(q, prev, out);
            KT_CHECK(closeAng(out[0], a, 0.01f));  // float eulerFromQ resolves rx to about 0.003 deg at ry = 89.9
            KT_CHECK(closeTo(out[1], b, 1e-3f));
            KT_CHECK(quatErr(qbase(out[0], out[1], out[2]), q) < 1e-4f);
        }
    // At exactly +-90 any rx gives the same rotation, so an arbitrary prev[0] must still reproduce q within 1e-4.
    const float gimbalPrev[] = {37.5f, -170.f};
    const float gimbalB[] = {90.f, -90.f};
    for (float pr : gimbalPrev)
        for (float b : gimbalB) {
            const Q q = qbase(-62.f, b, 118.f);
            const float prev[3] = {pr, 0.f, 0.f};
            float out[3];
            eulerNear(q, prev, out);
            KT_CHECK(closeAng(out[0], pr));
            KT_CHECK(closeTo(out[1], b, 1e-3f));
            KT_CHECK(quatErr(qbase(out[0], out[1], out[2]), q) < 1e-4f);
        }
}

// Angle between two rotations in degrees, from the vector part of a * conj(b) (acos of a dot near 1 only resolves ~0.04 deg in float).
static float angBetweenDeg(const Q& a, const Q& b) {
    const Q d = qmul(qnormalize(a), qconj(qnormalize(b)));
    return 2.f * std::atan2(std::sqrt(d.x * d.x + d.y * d.y + d.z * d.z), std::fabs(d.w)) * 57.29578f;
}

static void testEulerNearNudgeSweep() {
    // Port of research/holstered_weapons/ux_v2/s3_sim.cpp: 90 one-degree nudges about each bone axis, chaining the shown triple as
    // prev (as the overlay does). The old frozen-rx branch gave up to 18 deg render error and 47-67 deg display jumps for
    // 80 <= |ry| < 90. Per step: the shown triple must reproduce the true rotation within 0.05 deg, and the triple may move by at
    // most the Euler-map amplification of a 1 deg rotation, 1/cos(ry) (bound 1.1/cos + 0.1 using the larger |ry| of the two
    // steps). Steps whose |ry| exceeds 87 deg are exempt from the jump bound: there rx and rz legitimately swing by tens of
    // degrees and flip by 180 when the pose crosses the pole.
    const float starts[2][3] = {{30.f, 84.f, 10.f}, {30.f, 79.f, 10.f}};
    for (const auto& st : starts)
        for (int dir = 1; dir >= -1; dir -= 2)
            for (int axis = 0; axis < 3; ++axis) {
                Q q = qbase(st[0], st[1], st[2]);
                float prev[3] = {st[0], st[1], st[2]};
                for (int i = 0; i < 90; ++i) {
                    q = localAxisNudge(q, axis, static_cast<float>(dir));
                    float cur[3];
                    eulerNear(q, prev, cur);
                    KT_CHECK(angBetweenDeg(qbase(cur[0], cur[1], cur[2]), q) < 0.05f);
                    const float ry = std::fmax(std::fabs(prev[1]), std::fabs(cur[1]));
                    if (ry <= 87.f) {
                        const float bound = 1.1f / std::cos(ry * 0.01745329251994330f) + 0.1f;
                        for (int k = 0; k < 3; ++k) KT_CHECK(std::fabs(wrap180(cur[k] - prev[k])) <= bound);
                    }
                    for (int k = 0; k < 3; ++k) prev[k] = cur[k];
                }
            }
}

static void testEulerNearExactGimbal() {
    // Exactly ry = +-90: the rotation is reproduced exactly and rx stays at prev[0], whatever prev is.
    const float bs[] = {90.f, -90.f};
    const float prevRx[] = {-170.f, -20.f, 0.f, 37.5f, 150.f};
    for (float b : bs)
        for (float pr : prevRx) {
            const Q q = qbase(-62.f, b, 118.f);
            const float prev[3] = {pr, 12.f, -30.f};
            float out[3];
            eulerNear(q, prev, out);
            KT_CHECK(closeAng(out[0], pr));
            KT_CHECK(closeTo(out[1], b, 1e-3f));
            KT_CHECK(angBetweenDeg(qbase(out[0], out[1], out[2]), q) < 0.01f);
        }
    // Just off the pole (89.99, 89.5, -89.9) the decomposition is exact for any prev, including one far from the true rx.
    const float near[] = {89.99f, 89.5f, -89.9f, 89.0f};
    for (float b : near)
        for (float pr : prevRx) {
            const Q q = qbase(71.f, b, -42.f);
            const float prev[3] = {pr, 0.f, 0.f};
            float out[3];
            eulerNear(q, prev, out);
            KT_CHECK(angBetweenDeg(qbase(out[0], out[1], out[2]), q) < 0.05f);
        }
}

static void testQconj() {
    const Q q = qbase(12.f, -34.f, 56.f);
    const Q id = qmul(q, qconj(q));
    KT_CHECK(sameQuat(id, Q{1.f, 0.f, 0.f, 0.f}));
}

static void testWorldDeltaToLocalMatchesNudge() {
    const Q identity = {1.f, 0.f, 0.f, 0.f};
    const Q placeQ = qbase(20.f, 30.f, 40.f);
    const float z[3] = {0.f, 0.f, 1.f};
    const Q dq = qaxis(z, 10.f);
    KT_CHECK(sameQuat(worldDeltaToLocal(identity, placeQ, dq), localAxisNudge(placeQ, 2, 10.f)));
}

static void testWorldMoveToLocal() {
    const float z[3] = {0.f, 0.f, 1.f};
    const Q boneQ = qaxis(z, 90.f);
    const float off[3] = {0.1f, 0.2f, 0.3f}, dp[3] = {1.f, 0.f, 0.f};
    float out[3];
    worldMoveToLocal(boneQ, off, dp, out);
    KT_CHECK(closeTo(out[0], 0.1f) && closeTo(out[1], -0.8f) && closeTo(out[2], 0.3f));
}

static void testLocalAxisNudge() {
    const Q identity = {1.f, 0.f, 0.f, 0.f};
    const Q placeQ = qbase(-15.f, 25.f, 5.f);
    const float x[3] = {1.f, 0.f, 0.f};
    KT_CHECK(sameQuat(localAxisNudge(placeQ, 0, 7.f), qnormalize(qmul(qaxis(x, 7.f), placeQ))));
    KT_CHECK(sameQuat(localAxisNudge(identity, 1, 0.f), identity));
    KT_CHECK(sameQuat(localAxisNudge(placeQ, 7, 9.f), qnormalize(placeQ)));  // invalid axis is a no-op
}

static void testScreenDrag() {
    const float right[3] = {1.f, 0.f, 0.f}, up[3] = {0.f, 0.f, 1.f};
    const Q identity = {1.f, 0.f, 0.f, 0.f};
    KT_CHECK(sameQuat(screenDrag(0.f, 0.f, right, up, 0.5f), identity));
    // Horizontal drag turns about screen up (z here), vertical about screen right (x here).
    KT_CHECK(sameQuat(screenDrag(10.f, 0.f, right, up, 1.f), qaxis(up, 10.f)));
    KT_CHECK(sameQuat(screenDrag(0.f, 5.f, right, up, 1.f), qaxis(right, 5.f)));
}

static void testSnapHelpers() {
    KT_CHECK(closeTo(snapDeg(12.4f, 5.f), 10.f));
    KT_CHECK(closeTo(snapDeg(12.6f, 5.f), 15.f));
    KT_CHECK(closeTo(snapDeg(-12.6f, 5.f), -15.f));
    KT_CHECK(closeTo(snapDeg(7.f, 0.f), 7.f));  // step <= 0 passes the value through
    KT_CHECK(closeTo(snapOff(0.137f, 0.05f), 0.15f));
    KT_CHECK(closeTo(snapOff(-0.02f, 0.05f), 0.f));
    KT_CHECK(closeTo(snapOff(0.3f, -1.f), 0.3f));
}

static void testWrap180() {
    KT_CHECK(closeTo(wrap180(190.f), -170.f));
    KT_CHECK(closeTo(wrap180(-190.f), 170.f));
    KT_CHECK(closeTo(wrap180(360.f), 0.f));
    KT_CHECK(closeTo(wrap180(540.f), 180.f));
    KT_CHECK(closeTo(wrap180(-45.f), -45.f));
}

int main() {
    testRoundTripRandom();
    testGimbalCases();
    testKnownVectorsRoundTrip();
    testEulerNearBranchChoice();
    testEulerNearContinuity();
    testEulerNearSingularity();
    testEulerNearNudgeSweep();
    testEulerNearExactGimbal();
    testQconj();
    testWorldDeltaToLocalMatchesNudge();
    testWorldMoveToLocal();
    testLocalAxisNudge();
    testScreenDrag();
    testSnapHelpers();
    testWrap180();
    return kt::Report("holster_edit");
}
