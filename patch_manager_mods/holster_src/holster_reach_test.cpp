// T3 host tests for HolsterReach.h (two-bone reach solve, arc / delta / blend quaternion helpers, reach ramp and base cache of the
// holstered-weapons mod; doc 42).
// Build/run: python3 scripts/run_tests.py holster_reach
#include "HolsterReach.h"
#include "check.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stddef.h>
#include <stdint.h>

using namespace hs;

static const float kNaN = std::numeric_limits<float>::quiet_NaN();
static const float kInf = std::numeric_limits<float>::infinity();

static bool closeF(float a, float b, float tol = 1e-5f) { return std::fabs(a - b) <= tol; }
static bool closeV(const float* a, const float* b, float tol = 1e-5f) {
    return closeF(a[0], b[0], tol) && closeF(a[1], b[1], tol) && closeF(a[2], b[2], tol);
}
static bool closeQ(const Q& a, const Q& b, float tol = 1e-5f) {
    return closeF(a.w, b.w, tol) && closeF(a.x, b.x, tol) && closeF(a.y, b.y, tol) && closeF(a.z, b.z, tol);
}
static Q negQ(const Q& q) { return { -q.w, -q.x, -q.y, -q.z }; }
static bool closeQSign(const Q& a, const Q& b, float tol = 1e-5f) { return closeQ(a, b, tol) || closeQ(a, negQ(b), tol); }
static bool exactQ(const Q& a, const Q& b) { return a.w == b.w && a.x == b.x && a.y == b.y && a.z == b.z; }
static bool finiteV(const float* v) { return std::isfinite(v[0]) && std::isfinite(v[1]) && std::isfinite(v[2]); }
static float distV(const float* a, const float* b) {
    const float d[3] = { a[0] - b[0], a[1] - b[1], a[2] - b[2] };
    return r3len(d);
}
static void sub3(const float* a, const float* b, float* o) {
    for (int i = 0; i < 3; ++i) o[i] = a[i] - b[i];
}
static void unit3(const float* v, float* o) {
    const float l = r3len(v);
    for (int i = 0; i < 3; ++i) o[i] = v[i] / l;
}

// Deterministic generator (LCG): the same cases on every run.
static uint32_t gSeed = 0x9E3779B9u;
static float rnd01() {
    gSeed = gSeed * 1664525u + 1013904223u;
    return (float)(gSeed >> 8) * (1.f / 16777216.f);
}
static float rndSigned() { return 2.f * rnd01() - 1.f; }
static Q randQ() { return { rndSigned(), rndSigned(), rndSigned(), rndSigned() }; }

// Hand position after the solve: the forearm (H - E, length len2) rotated about the new elbow so it points at T.
static void handAfter(const float* E, const float* H, const float* Enew, const float* T, float* out) {
    float fore[3], toT[3], rot[3];
    sub3(H, E, fore);
    sub3(T, Enew, toT);
    qrotate(arcQuat(fore, toT), fore, rot);
    for (int i = 0; i < 3; ++i) out[i] = Enew[i] + rot[i];
}

// ---- arcQuat ----

static void testArcQuatMapsDirections() {
    const float dirs[6][3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0.3f, -0.7f, 0.2f}, {-0.5f, 0.1f, 0.9f}, {2.f, 3.f, -1.f} };
    for (size_t i = 0; i < 6; ++i) {
        for (size_t j = 0; j < 6; ++j) {
            float f[3], t[3], out[3];
            unit3(dirs[i], f);
            unit3(dirs[j], t);
            const Q q = arcQuat(dirs[i], dirs[j]);
            qrotate(q, f, out);
            KT_CHECK(closeV(out, t, 1e-4f));
            KT_CHECK(closeF(qdot(q, q), 1.f, 1e-5f));
        }
    }
}

static void testArcQuatParallelAndAntiParallel() {
    const float d[3] = { 0.6f, 0.8f, 0.f };
    KT_CHECK(closeQ(arcQuat(d, d), { 1, 0, 0, 0 }));
    const float dbl[3] = { 1.2f, 1.6f, 0.f };
    KT_CHECK(closeQ(arcQuat(d, dbl), { 1, 0, 0, 0 }));  // parallel, different length: identity
    const float axes[4][3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {0.577f, 0.577f, 0.577f} };
    for (size_t i = 0; i < 4; ++i) {
        const float neg[3] = { -axes[i][0], -axes[i][1], -axes[i][2] };
        const Q q = arcQuat(axes[i], neg);
        float out[3];
        qrotate(q, axes[i], out);
        KT_CHECK(closeV(out, neg, 1e-4f));  // 180 deg about some perpendicular axis
        KT_CHECK(closeF(qdot(q, q), 1.f, 1e-5f));
    }
    // anti-parallel of the unit-ish test vector d
    const float nd[3] = { -0.6f, -0.8f, 0.f };
    float out[3];
    qrotate(arcQuat(d, nd), d, out);
    KT_CHECK(closeV(out, nd, 1e-4f));
}

static void testArcQuatDegenerateIsIdentity() {
    const Q id = { 1, 0, 0, 0 };
    const float zero[3] = { 0, 0, 0 }, x[3] = { 1, 0, 0 }, tiny[3] = { 1e-7f, 0, 0 };
    const float bad[3] = { kNaN, 0, 0 }, inf[3] = { kInf, 0, 0 };
    KT_CHECK(exactQ(arcQuat(zero, x), id));
    KT_CHECK(exactQ(arcQuat(x, zero), id));
    KT_CHECK(exactQ(arcQuat(zero, zero), id));
    KT_CHECK(exactQ(arcQuat(tiny, x), id));
    KT_CHECK(exactQ(arcQuat(x, tiny), id));
    KT_CHECK(exactQ(arcQuat(bad, x), id));
    KT_CHECK(exactQ(arcQuat(x, bad), id));
    KT_CHECK(exactQ(arcQuat(inf, x), id));
    KT_CHECK(exactQ(arcQuat(x, inf), id));
}

static void testArcQuatUnnormalizedInputs() {
    // Lengths are divided out: (3,0,0) -> (0,0,0.5) is the same arc as x -> z.
    const float from[3] = { 3, 0, 0 }, to[3] = { 0, 0, 0.5f };
    const float ux[3] = { 1, 0, 0 }, uz[3] = { 0, 0, 1 };
    float out[3];
    qrotate(arcQuat(from, to), ux, out);
    KT_CHECK(closeV(out, uz, 1e-4f));
}

// ---- reachConj ----

static void testQconjBasics() {
    KT_CHECK(exactQ(reachConj({ 1, 2, 3, 4 }), { 1, -2, -3, -4 }));
    const Q u = qnormalize({ 0.5f, 0.5f, 0.5f, 0.5f });
    KT_CHECK(closeQ(qmul(u, reachConj(u)), { 1, 0, 0, 0 }, 1e-5f));
    KT_CHECK(closeQ(qmul(reachConj(u), u), { 1, 0, 0, 0 }, 1e-5f));
}

// ---- solveTwoBone ----

// S = origin, E at len1 0.5, H at len2 ~0.583, T at d ~0.806 (inside the reachable band).
static void testSolveReachableLandsOnTarget() {
    const float S[3] = { 0, 0, 0 }, E[3] = { 0.3f, 0.4f, 0 }, H[3] = { 0.8f, 0.4f, 0.3f }, T[3] = { 0.2f, 0.6f, 0.5f };
    const float len1 = 0.5f, len2 = distV(H, E);
    float en[3] = { 0, 0, 0 }, reach = -1.f;
    KT_CHECK(solveTwoBone(S, E, H, T, en, &reach));
    KT_CHECK(finiteV(en));
    KT_CHECK(reach == 1.f);
    KT_CHECK(closeF(distV(en, S), len1, 1e-4f));
    KT_CHECK(closeF(distV(T, en), len2, 1e-4f));
    float hand[3];
    handAfter(E, H, en, T, hand);
    KT_CHECK(closeF(distV(hand, en), len2, 1e-4f));
    KT_CHECK(distV(hand, T) < 1e-3f);
}

static void testSolveRandomReachableSweep() {
    int cases = 0;
    for (int k = 0; k < 400; ++k) {
        float S[3], E[3], H[3], ld[3], dir[3], T[3];
        for (int i = 0; i < 3; ++i) {
            S[i] = rndSigned();
            E[i] = rndSigned();
            H[i] = rndSigned();
            ld[i] = rndSigned();
        }
        const float len1 = distV(E, S), len2 = distV(H, E);
        if (len1 < 0.05f || len2 < 0.05f || r3len(ld) < 0.1f) continue;
        unit3(ld, dir);
        const float lo = std::fabs(len1 - len2), hi = len1 + len2;
        const float d = lo + (hi - lo) * (0.05f + 0.9f * rnd01());  // strictly inside the band: no clamp
        for (int i = 0; i < 3; ++i) T[i] = S[i] + dir[i] * d;
        float en[3], reach = -1.f;
        ++cases;
        KT_CHECK(solveTwoBone(S, E, H, T, en, &reach));
        KT_CHECK(reach == 1.f);
        KT_CHECK(closeF(distV(en, S), len1, 1e-4f));
        KT_CHECK(closeF(distV(T, en), len2, 1e-4f));
        float hand[3];
        handAfter(E, H, en, T, hand);
        KT_CHECK(distV(hand, T) < 1e-3f);
    }
    KT_CHECK(cases >= 300);
}

// T = H: the current arm already reaches it, so the elbow must come back to E.
static void testSolveTargetAtHandKeepsElbow() {
    const float S[3] = { 0, 0, 0 }, E[3] = { 0.3f, 0.4f, 0 }, H[3] = { 0.8f, 0.4f, 0.3f };
    float en[3] = { 0, 0, 0 }, reach = -1.f;
    KT_CHECK(solveTwoBone(S, E, H, H, en, &reach));
    KT_CHECK(closeV(en, E, 1e-4f));
    KT_CHECK(reach == 1.f);
}

// Too far: d is clamped to len1 + len2 - eps, the arm straightens toward the target, reach = d_clamped / d_target < 1.
static void testSolveTooFarClampsAndStraightens() {
    const float S[3] = { 0, 0, 0 }, E[3] = { 1, 0, 0 }, H[3] = { 1.5f, 1, 0 }, T[3] = { 0, 5, 0 };
    const float len1 = 1.f, len2 = distV(H, E);
    float en[3] = { 0, 0, 0 }, reach = -1.f;
    KT_CHECK(solveTwoBone(S, E, H, T, en, &reach));
    KT_CHECK(finiteV(en) && std::isfinite(reach));
    KT_CHECK(reach < 1.f && reach > 0.f);
    const float expect = (len1 + len2 - 1e-4f * (len1 < len2 ? len1 : len2)) / 5.f;
    KT_CHECK(closeF(reach, expect, 1e-4f));
    KT_CHECK(closeF(distV(en, S), len1, 1e-4f));
    const float up[3] = { 0, 1, 0 };
    KT_CHECK(closeV(en, up, 0.05f));  // the arm is straight along S->T (the eps keeps a small bend)
}

// Too close: d is clamped up to |len1 - len2| + eps, reach = d_clamped / d_target > 1 (per the contract, not a 0..1 fraction).
static void testSolveTooCloseClamps() {
    const float S[3] = { 0, 0, 0 }, E[3] = { 2, 0, 0 }, H[3] = { 2, 1, 0 }, T[3] = { 0.5f, 0, 0 };
    float en[3] = { 0, 0, 0 }, reach = -1.f;
    KT_CHECK(solveTwoBone(S, E, H, T, en, &reach));
    KT_CHECK(finiteV(en) && std::isfinite(reach));
    KT_CHECK(reach > 1.f);
    KT_CHECK(closeF(reach, (1.f + 1e-4f) / 0.5f, 1e-3f));
    KT_CHECK(closeF(distV(en, S), 2.f, 1e-4f));
}

// Target on the shoulder: dT ~ 0 uses the S->E axis, reach = 1, finite, elbow still len1 from S.
static void testSolveTargetAtShoulder() {
    const float S[3] = { 0, 0, 0 }, E[3] = { 0.3f, 0.4f, 0 }, H[3] = { 0.8f, 0.4f, 0.3f };
    float en[3] = { 0, 0, 0 }, reach = -1.f;
    KT_CHECK(solveTwoBone(S, E, H, S, en, &reach));
    KT_CHECK(finiteV(en) && reach == 1.f);
    KT_CHECK(closeF(distV(en, S), 0.5f, 1e-4f));
    const float tiny[3] = { 1e-7f, 0, 0 };
    float en2[3] = { 0, 0, 0 }, reach2 = -1.f;
    KT_CHECK(solveTwoBone(S, E, H, tiny, en2, &reach2));
    KT_CHECK(finiteV(en2) && reach2 == 1.f);
    KT_CHECK(closeF(distV(en2, S), 0.5f, 1e-4f));
}

// S, E, T collinear: the side hint is empty and a fallback perpendicular is used.
static void testSolveCollinearFallbackHint() {
    const float S[3] = { 0, 0, 0 }, E[3] = { 1, 0, 0 }, H[3] = { 1.8f, 0.6f, 0 };
    const float T[3] = { 1.5f, 0, 0 };
    float en[3] = { 0, 0, 0 }, reach = -1.f;
    KT_CHECK(solveTwoBone(S, E, H, T, en, &reach));
    KT_CHECK(reach == 1.f);
    const float want[3] = { 0.75f, 0.f, std::sqrt(1.f - 0.75f * 0.75f) };
    KT_CHECK(closeV(en, want, 1e-4f));
    KT_CHECK(closeF(distV(T, en), 1.f, 1e-4f));
    // Collinear with the target behind the shoulder: still finite, both lengths hold.
    const float Tb[3] = { -0.5f, 0, 0 };
    float eb[3] = { 0, 0, 0 }, rb = -1.f;
    KT_CHECK(solveTwoBone(S, E, H, Tb, eb, &rb));
    KT_CHECK(finiteV(eb) && rb == 1.f);
    KT_CHECK(closeF(distV(eb, S), 1.f, 1e-4f));
    KT_CHECK(closeF(distV(Tb, eb), 1.f, 1e-4f));
}

static void testSolveZeroLengthSegmentsReturnFalse() {
    const float S[3] = { 0, 0, 0 }, E[3] = { 0.3f, 0.4f, 0 }, H[3] = { 0.8f, 0.4f, 0.3f }, T[3] = { 0.2f, 0.6f, 0.5f };
    float en[3] = { 7, 7, 7 }, reach = 9.f;
    KT_CHECK(!solveTwoBone(S, S, H, T, en, &reach));  // len1 == 0
    KT_CHECK(en[0] == 7.f && en[1] == 7.f && en[2] == 7.f && reach == 9.f);
    KT_CHECK(!solveTwoBone(S, E, E, T, en, &reach));  // len2 == 0
    KT_CHECK(en[0] == 7.f && en[1] == 7.f && en[2] == 7.f && reach == 9.f);
    const float shortSeg[3] = { 1e-5f, 0, 0 };
    KT_CHECK(!solveTwoBone(S, shortSeg, H, T, en, &reach));  // len1 below 1e-4
    KT_CHECK(en[0] == 7.f && reach == 9.f);
    const float shortHand[3] = { 0.3f + 1e-5f, 0.4f, 0 };
    KT_CHECK(!solveTwoBone(S, E, shortHand, T, en, &reach));  // len2 below 1e-4
    KT_CHECK(en[0] == 7.f && reach == 9.f);
}

static void testSolveNonFiniteInputsReturnFalse() {
    const float base[4][3] = { {0, 0, 0}, {0.3f, 0.4f, 0}, {0.8f, 0.4f, 0.3f}, {0.2f, 0.6f, 0.5f} };
    const float bad[3] = { kNaN, kInf, -kInf };
    for (int slot = 0; slot < 4; ++slot) {
        for (int comp = 0; comp < 3; ++comp) {
            for (int b = 0; b < 3; ++b) {
                float in[4][3];
                for (int r = 0; r < 4; ++r)
                    for (int c = 0; c < 3; ++c) in[r][c] = base[r][c];
                in[slot][comp] = bad[b];
                float en[3] = { 7, 7, 7 }, reach = 9.f;
                KT_CHECK(!solveTwoBone(in[0], in[1], in[2], in[3], en, &reach));
                KT_CHECK(en[0] == 7.f && en[1] == 7.f && en[2] == 7.f && reach == 9.f);
            }
        }
    }
}

// Lengths overflow float: either refused (outputs untouched) or the outputs are finite. Never a NaN result.
static void testSolveOverflowGuard() {
    const float S[3] = { 0, 0, 0 }, E[3] = { 1e30f, 0, 0 }, H[3] = { 1e30f, 1e30f, 0 }, T[3] = { 1e30f, 0, 0 };
    float en[3] = { 7, 7, 7 }, reach = 9.f;
    const bool ok = solveTwoBone(S, E, H, T, en, &reach);
    if (ok) {
        KT_CHECK(finiteV(en) && std::isfinite(reach));
    } else {
        KT_CHECK(en[0] == 7.f && en[1] == 7.f && en[2] == 7.f && reach == 9.f);
    }
}

// ---- reachDeltaToLocal ----

static void testWorldDeltaIdentityParentIsDeltaTimesCur() {
    const Q id = { 1, 0, 0, 0 };
    const Q cur = qnormalize({ 0.9f, 0.1f, -0.3f, 0.2f });
    const Q delta = qnormalize({ 0.8f, -0.2f, 0.4f, 0.1f });
    KT_CHECK(closeQ(reachDeltaToLocal(id, delta, cur), qnormalize(qmul(delta, cur)), 1e-5f));
    KT_CHECK(closeQ(reachDeltaToLocal(id, id, cur), cur, 1e-5f));
}

static void testWorldDeltaRoundTrip() {
    for (int k = 0; k < 60; ++k) {
        const Q P = qnormalize(randQ()), D = qnormalize(randQ()), L = qnormalize(randQ());
        const Q nl = reachDeltaToLocal(P, D, L);
        KT_CHECK(closeF(qdot(nl, nl), 1.f, 1e-5f));
        // parentWorld * newLocal == deltaWorld * parentWorld * curLocal (up to sign)
        KT_CHECK(closeQSign(qmul(P, nl), qmul(D, qmul(P, L)), 1e-4f));
    }
}

static void testWorldDeltaNonFiniteKeepsBone() {
    const Q P = qnormalize({ 0.9f, 0.1f, -0.3f, 0.2f }), D = qnormalize({ 0.8f, -0.2f, 0.4f, 0.1f });
    const Q L = qnormalize({ 0.2f, 0.7f, 0.1f, -0.6f });
    const Q badQ = { kNaN, 0, 0, 0 };
    KT_CHECK(closeQ(reachDeltaToLocal(badQ, D, L), L, 1e-5f));
    KT_CHECK(closeQ(reachDeltaToLocal(P, badQ, L), L, 1e-5f));
    KT_CHECK(closeQ(reachDeltaToLocal(P, D, badQ), { 1, 0, 0, 0 }, 1e-5f));  // no finite current pose: identity
    const Q zero = { 0, 0, 0, 0 };
    const Q z = reachDeltaToLocal(zero, D, L);
    KT_CHECK(qfinite(z) && closeF(qdot(z, z), 1.f, 1e-5f));
}

// ---- reachWeight ----

static void testReachWeightEndpointsAndInactive() {
    Slide s;
    s.t0 = 1000u;
    s.len = 500u;
    KT_CHECK(reachWeight(s, 1000u) == 0.f);  // p = 0
    KT_CHECK(reachWeight(s, 1500u) == 0.f);  // p = 1
    KT_CHECK(reachWeight(s, 999u) == 0.f);   // before the slide
    KT_CHECK(reachWeight(s, 1600u) == 0.f);  // after the slide
    KT_CHECK(reachWeight(s, 1250u) == 1.f);  // p = 0.5
    KT_CHECK(reachWeight(Slide(), 5u) == 0.f);  // default slide: len 0, inactive
    Slide r;
    r.reset(true, 100u);
    KT_CHECK(reachWeight(r, 100u) == 0.f && reachWeight(r, 101u) == 0.f);  // silent placement has no ramp
}

static void testReachWeightRampShape() {
    Slide s;
    s.t0 = 1000u;
    s.len = 500u;
    float prev = 0.f;
    for (uint32_t k = 1; k <= 100; ++k) {  // first 20%: non-decreasing and positive
        const float w = reachWeight(s, 1000u + k);
        KT_CHECK(w >= prev && w > 0.f && w <= 1.f);
        prev = w;
    }
    for (uint32_t k = 100; k <= 400; ++k) {  // middle 60%: exactly 1
        KT_CHECK(closeF(reachWeight(s, 1000u + k), 1.f, 1e-5f));
    }
    prev = 2.f;
    for (uint32_t k = 400; k <= 499; ++k) {  // last 20%: non-increasing
        const float w = reachWeight(s, 1000u + k);
        KT_CHECK(w <= prev && w >= 0.f);
        prev = w;
    }
}

static void testReachWeightWrapSafe() {
    Slide s;
    s.t0 = 0xFFFFFF00u;
    s.len = 500u;
    KT_CHECK(closeF(reachWeight(s, 0xFFFFFF00u + 250u), 1.f, 1e-5f));
    KT_CHECK(reachWeight(s, 0xFFFFFF00u + 500u) == 0.f);  // wrapped to the end: inactive
    KT_CHECK(reachWeight(s, 0xFFFFFF00u + 499u) > 0.f);
}

// ---- blendQ ----

static void testBlendQExactAtZero() {
    const Q a = { 2.f, -1.f, 0.5f, 3.f };  // deliberately not unit: w == 0 must return it untouched
    const Q b = qnormalize({ 0.2f, 0.7f, 0.1f, -0.6f });
    KT_CHECK(exactQ(blendQ(a, b, 0.f), a));
    KT_CHECK(exactQ(blendQ(a, b, -0.f), a));
    KT_CHECK(exactQ(blendQ(a, b, -3.f), a));
    KT_CHECK(exactQ(blendQ(a, b, kNaN), a));  // NaN weight maps to 0
}

static void testBlendQEndsClampAndSign() {
    const Q a = qnormalize({ 0.9f, 0.1f, -0.3f, 0.2f });
    const Q b = qnormalize({ 0.8f, -0.2f, 0.4f, 0.1f });  // dot(a, b) > 0
    KT_CHECK(closeQ(blendQ(a, b, 1.f), b));
    KT_CHECK(closeQ(blendQ(a, b, 7.f), b));  // clamp above 1
    // -b has dot(a, -b) < 0: the blend takes the short arc, so w = 1 gives +b and w = 0.5 matches the b blend.
    KT_CHECK(closeQ(blendQ(a, negQ(b), 1.f), b));
    KT_CHECK(closeQ(blendQ(a, negQ(b), 0.5f), blendQ(a, b, 0.5f)));
    KT_CHECK(qdot(blendQ(a, negQ(b), 1.f), a) > 0.f);
}

static void testBlendQMidpointIsNormalizedNlerp() {
    const Q a = qnormalize({ 0.9f, 0.1f, -0.3f, 0.2f });
    const Q b = qnormalize({ 0.8f, -0.2f, 0.4f, 0.1f });
    const Q m = blendQ(a, b, 0.5f);
    KT_CHECK(closeF(qdot(m, m), 1.f, 1e-5f));
    KT_CHECK(closeQ(m, qnormalize({ a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z }), 1e-5f));
}

static void testBlendQNonFiniteInputs() {
    const Q id = { 1, 0, 0, 0 };
    const Q a = qnormalize({ 0.9f, 0.1f, -0.3f, 0.2f });
    const Q b = qnormalize({ 0.8f, -0.2f, 0.4f, 0.1f });
    const Q badA = { kNaN, 0, 0, 0 }, badB = { 0, kInf, 0, 0 };
    KT_CHECK(qfinite(blendQ(badA, b, 0.5f)));
    KT_CHECK(exactQ(blendQ(badA, b, 0.f), id));        // non-finite a is taken as identity
    KT_CHECK(exactQ(blendQ(a, badB, 0.5f), a));        // non-finite b keeps a
}

// ---- reachBase ----

static void testReachBaseFirstReadStores() {
    Base bs;
    KT_CHECK(!bs.have);
    const Q r1 = qnormalize({ 0.9f, 0.1f, -0.3f, 0.2f });
    const Q got = reachBase(bs, r1);
    KT_CHECK(bs.have && closeQ(got, r1) && closeQ(bs.q, r1));
    Base fresh;  // identity read on a fresh cache is stored, not treated as a resample-skip
    const Q id = { 1, 0, 0, 0 };
    KT_CHECK(closeQ(reachBase(fresh, id), id) && fresh.have);
}

static void testReachBaseCachesWhenEngineDidNotResample() {
    Base bs;
    const Q r1 = qnormalize({ 0.9f, 0.1f, -0.3f, 0.2f });
    reachBase(bs, r1);
    const float z[3] = { 0, 0, 1 };
    const Q W = qnormalize(qmul(r1, qaxis(z, 30.f)));  // what the mod wrote into the bone
    reachNoteWrite(bs, W);
    KT_CHECK(closeQ(reachBase(bs, W), r1));  // read still equals the write: cached base
    KT_CHECK(closeQ(reachBase(bs, W), r1));  // and again next frame
    KT_CHECK(closeQ(reachBase(bs, negQ(W)), r1));  // same rotation with the sign flipped also counts as unchanged
    KT_CHECK(closeQ(bs.q, r1));
}

static void testReachBaseRefreshesWhenEngineChanged() {
    Base bs;
    const Q r1 = qnormalize({ 0.9f, 0.1f, -0.3f, 0.2f });
    reachBase(bs, r1);
    const float z[3] = { 0, 0, 1 };
    const Q W = qnormalize(qmul(r1, qaxis(z, 30.f)));
    reachNoteWrite(bs, W);
    const Q r2 = qnormalize({ 0.2f, 0.5f, 0.1f, 0.8f });  // the engine resampled the bone to something else
    KT_CHECK(closeQ(reachBase(bs, r2), r2));
    KT_CHECK(closeQ(bs.q, r2));
    // the cache now follows the new base: writing W2 and reading W2 gives r2 back
    const Q W2 = qnormalize(qmul(r2, qaxis(z, 45.f)));
    reachNoteWrite(bs, W2);
    KT_CHECK(closeQ(reachBase(bs, W2), r2));
}

static void testReachBaseNonFiniteReadKeepsCache() {
    Base bs;
    const Q r1 = qnormalize({ 0.9f, 0.1f, -0.3f, 0.2f });
    reachBase(bs, r1);
    const Q before = bs.q;
    const Q bad = { kNaN, 0, 0, 0 };
    const Q got = reachBase(bs, bad);
    KT_CHECK(qfinite(got) && exactQ(got, before));
    KT_CHECK(exactQ(bs.q, before));
}

// ---- elbow pole / style ----
static float len3(const float* v) { return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }
static float dot3(const float* a, const float* b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }

static void testSolvePoleChoosesElbowSide() {
    // Shoulder at origin, forearm hangs down; target out to the side at the same depth. The elbow may bend up or down: the pole picks.
    const float S[3] = {0, 0, 0}, E[3] = {0, 0, -0.3f}, H[3] = {0, 0, -0.64f}, T[3] = {0.45f, 0, -0.2f};
    const float up[3] = {0, 0, 1}, down[3] = {0, 0, -1};
    float a[3], b[3], r = 0;
    KT_CHECK(solveTwoBone(S, E, H, T, a, &r, up));
    KT_CHECK(solveTwoBone(S, E, H, T, b, &r, down));
    KT_CHECK(a[2] > b[2] + 0.05f);                                     // up pole puts the elbow higher than the down pole
    KT_CHECK(std::fabs(len3(a) - 0.3f) < 1e-4f && std::fabs(len3(b) - 0.3f) < 1e-4f);   // bone length kept either way
    float c[3];
    KT_CHECK(solveTwoBone(S, E, H, T, c, &r, nullptr));                // null pole = old hint (current elbow side)
    KT_CHECK(solveTwoBone(S, E, H, T, b, &r));
    KT_CHECK(std::fabs(c[0] - b[0]) < 1e-6f && std::fabs(c[2] - b[2]) < 1e-6f);
}

static void testSolvePoleParallelAndBadFallBack() {
    const float S[3] = {0, 0, 0}, E[3] = {0, 0.1f, -0.3f}, H[3] = {0, 0.1f, -0.64f}, T[3] = {0, 0, -0.5f};
    const float axisPole[3] = {0, 0, -1};   // parallel to S->T: no usable side
    const float nanPole[3] = {NAN, 0, 0};
    float a[3], b[3], r = 0;
    KT_CHECK(solveTwoBone(S, E, H, T, a, &r, axisPole));
    KT_CHECK(std::isfinite(a[0]) && std::isfinite(a[1]) && std::isfinite(a[2]));
    KT_CHECK(solveTwoBone(S, E, H, T, a, &r, nanPole));                // non-finite pole ignored
    KT_CHECK(solveTwoBone(S, E, H, T, b, &r));
    KT_CHECK(std::fabs(a[0] - b[0]) < 1e-6f && std::fabs(a[1] - b[1]) < 1e-6f && std::fabs(a[2] - b[2]) < 1e-6f);
}

static void testReachOverWeight() {
    KT_CHECK(reachOverWeight(-0.8f * 0.64f, 0.64f) == 0.f);             // hip mount
    KT_CHECK(reachOverWeight(-0.1f * 0.64f, 0.64f) == 1.f);             // upper back
    KT_CHECK(reachOverWeight(0.3f, 0.64f) == 1.f);                      // above the shoulder
    const float mid = reachOverWeight(-0.5f * 0.64f, 0.64f);
    KT_CHECK(mid > 0.f && mid < 1.f);
    KT_CHECK(reachOverWeight(-0.55f * 0.64f, 0.64f) <= mid);            // monotone
    KT_CHECK(reachOverWeight(NAN, 0.64f) == 0.f && reachOverWeight(0.f, 0.f) == 0.f);
    // band -0.60..-0.38 (HW27): pelvis_g (-0.54) keeps the hip pole, an upper-back mount (-0.36) is fully over, and the weight never falls over -0.8..0.3
    KT_CHECK(reachOverWeight(-0.54f * 0.64f, 0.64f) < 0.2f);
    KT_CHECK(reachOverWeight(-0.36f * 0.64f, 0.64f) >= 0.999f);
    float prevW = -1.f;
    for (int i = 0; i <= 110; ++i) {
        const float w = reachOverWeight((-0.8f + 0.01f * i) * 0.64f, 0.64f);
        KT_CHECK(w >= prevW);
        prevW = w;
    }
}

static void testReachPoleStyleAndMirror() {
    const Q id = {1, 0, 0, 0};
    ReachStyle st = reachDefaultStyle(2);   // melee
    float h[3], o[3], l[3];
    KT_CHECK(reachPole(st, 0.f, false, id, nullptr, 0.f, h));
    KT_CHECK(reachPole(st, 1.f, false, id, nullptr, 0.f, o));
    KT_CHECK(h[2] < 0.f && o[2] > 0.5f);                                // hip bends down, over bends up
    KT_CHECK(std::fabs(len3(h) - 1.f) < 1e-5f && std::fabs(len3(o) - 1.f) < 1e-5f);
    KT_CHECK(reachPole(st, 1.f, true, id, nullptr, 0.f, l));
    KT_CHECK(std::fabs(l[0] + o[0]) < 1e-6f && std::fabs(l[2] - o[2]) < 1e-6f);   // left mirrors x only
    // A body turned 90 deg about z (right becomes +y... any rotation): the pole follows the body.
    const float z[3] = {0, 0, 1};
    const Q turn = qaxis(z, 90.f);
    float t[3], expect[3];
    KT_CHECK(reachPole(st, 1.f, false, turn, nullptr, 0.f, t));
    qrotate(turn, o, expect);
    KT_CHECK(std::fabs(t[0] - expect[0]) < 1e-5f && std::fabs(t[1] - expect[1]) < 1e-5f && std::fabs(t[2] - expect[2]) < 1e-5f);
    // animMix 1 = the animation direction; 0.5 lies between.
    const float anim[3] = {0, -3.f, 0};
    float a1[3], a5[3];
    KT_CHECK(reachPole(st, 1.f, false, id, anim, 1.f, a1));
    KT_CHECK(std::fabs(a1[1] + 1.f) < 1e-5f);
    KT_CHECK(reachPole(st, 1.f, false, id, anim, 0.5f, a5));
    KT_CHECK(dot3(a5, o) > 0.f && dot3(a5, a1) > 0.f && std::fabs(len3(a5) - 1.f) < 1e-5f);
    KT_CHECK(!reachPole(st, 1.f, false, Q{NAN, 0, 0, 0}, nullptr, 0.f, a5));
    // every category has a style with finite, non-zero directions
    for (int c = 0; c <= 6; ++c) {
        const ReachStyle s2 = reachDefaultStyle(c);
        KT_CHECK(len3(s2.hip) > 0.5f && len3(s2.over) > 0.5f && s2.over[2] > 0.3f && s2.hip[2] < 0.f);
    }
}

// ---- carry mode ----
static void testReachWeightRamp() {
    Slide sl;
    sl.reset(false, 1000);
    sl.target(true, 1000, 400, 400);   // draw: len 400, running 1000..1400
    KT_CHECK(reachWeightRamp(sl, 1000, 0.1f) == 0.f && reachWeightRamp(sl, 1400, 0.1f) == 0.f);
    KT_CHECK(reachWeightRamp(sl, 1200, 0.1f) == 1.f);
    const float a = reachWeightRamp(sl, 1020, 0.1f), b = reachWeightRamp(sl, 1020, 0.2f);
    KT_CHECK(a > b && a > 0.f && a < 1.f);                            // a shorter ramp is further along at the same time
    KT_CHECK(reachWeightRamp(sl, 1200, 0.f) == 0.f);                  // no ramp: off (never divides by zero)
    KT_CHECK(reachWeight(sl, 1020) == reachWeightRamp(sl, 1020, 0.2f));
}

static void testReachAnimMix() {
    KT_CHECK(reachAnimMix(0.f) == 0.f && reachAnimMix(0.3f) == 0.f && reachAnimMix(0.45f) == 0.f);   // the mount style holds up to 0.45
    KT_CHECK(reachAnimMix(0.95f) == 1.f && reachAnimMix(1.f) == 1.f);
    KT_CHECK(reachAnimMix(0.6f) > 0.f && reachAnimMix(0.6f) < 1.f);
    KT_CHECK(reachAnimMix(0.55f) < reachAnimMix(0.8f));               // monotone
    KT_CHECK(reachAnimMix(-1.f) == 0.f && reachAnimMix(2.f) == 1.f && reachAnimMix(NAN) == 0.f);
}

static void testReachCarryTarget() {
    const float m[3] = {1, 2, 3}, h[3] = {4, 2, 0}, up[3] = {0, 0, 1}, fw[3] = {0, 1, 0}, ou[3] = {1, 0, 0}, bl[3] = {0.15f, 0.45f, 0.10f};
    float o[3];
    reachCarryTarget(m, h, up, fw, ou, bl, 1.f, 0.64f, 0.f, o);
    KT_CHECK(std::fabs(o[0] - 1) < 1e-6f && std::fabs(o[1] - 2) < 1e-6f && std::fabs(o[2] - 3) < 1e-6f);   // mount at f = 0
    reachCarryTarget(m, h, up, fw, ou, bl, 1.f, 0.64f, 1.f, o);
    KT_CHECK(std::fabs(o[0] - 4) < 1e-5f && std::fabs(o[1] - 2) < 1e-5f && std::fabs(o[2] - 0) < 1e-5f);   // hand at f = 1
    float flat[3], arc[3];
    reachCarryTarget(m, h, up, fw, ou, bl, 0.f, 0.64f, 0.5f, flat);   // hip mount: no bow
    reachCarryTarget(m, h, up, fw, ou, bl, 1.f, 0.64f, 0.5f, arc);    // high mount: up 0.15, forward 0.45, outward 0.10 of the arm at the middle
    KT_CHECK(std::fabs(arc[2] - flat[2] - 0.15f * 0.64f) < 1e-5f && std::fabs(arc[1] - flat[1] - 0.45f * 0.64f) < 1e-5f && std::fabs(arc[0] - flat[0] - 0.10f * 0.64f) < 1e-5f);
    reachCarryTarget(m, h, nullptr, nullptr, nullptr, nullptr, 1.f, 0.64f, 0.5f, arc);   // no axes, no bulge: the straight line, finite
    KT_CHECK(std::isfinite(arc[0]) && std::isfinite(arc[1]) && std::isfinite(arc[2]));
    reachCarryTarget(m, h, up, fw, ou, bl, NAN, NAN, 0.5f, arc);
    KT_CHECK(std::isfinite(arc[2]) && std::isfinite(arc[1]) && std::isfinite(arc[0]));
    // heavy weapons: more up and out, less forward than a sword
    const ReachStyle sword = reachDefaultStyle(2), heavy = reachDefaultStyle(6);
    KT_CHECK(heavy.bulge[0] > sword.bulge[0] && heavy.bulge[2] > sword.bulge[2] && heavy.bulge[1] < sword.bulge[1]);
}

// reachPlan: draw = reach (weapon stays on the mount) then carry; stow = carry then release; both start and end with restMix 1 / pathF at rest.
static void testReachPlanDraw() {
    Slide sl;
    sl.reset(false, 1000);
    sl.target(true, 1000, 400, 400);   // draw from the mount: 1000..1400
    ReachPlan z0 = reachPlan(sl, 1000, 0.3f);                         // the retarget tick (d == 0) is inside the plan: rest end, weapon on the mount
    KT_CHECK(z0.active && z0.pathF == 1.f && z0.carryW == 0.f && !reachPlan(sl, 1400, 0.3f).active);
    ReachPlan a = reachPlan(sl, 1040, 0.3f);                          // p = 0.1: inside the reach phase
    KT_CHECK(a.active && a.carryW == 0.f && a.restMix == 0.f && a.pathF > 0.5f && a.pathF < 1.f);   // reach: walking the arc from the rest end toward the mount
    ReachPlan b = reachPlan(sl, 1120, 0.3f);                          // p = 0.3: the grab instant
    KT_CHECK(std::fabs(b.pathF) < 1e-5f && b.restMix < 1e-4f && b.carryW < 1e-4f);
    ReachPlan c = reachPlan(sl, 1260, 0.3f);                          // p = 0.65: q = 0.5 of the carry phase
    KT_CHECK(std::fabs(c.pathF - 0.45946f) < 1e-4f && c.restMix == 0.f && c.carryW == 1.f);
    ReachPlan d = reachPlan(sl, 1399, 0.3f);                          // almost at the end: at the rest hand, weapon carried
    KT_CHECK(d.pathF > 0.99f && d.carryW == 1.f && d.restMix == 0.f);
    float prev = 2.f;                                                  // draw: pathF falls to 0 during the reach, then rises to 1 during the carry
    uint32_t tMin = 1001;
    for (uint32_t t = 1001; t < 1400; ++t) { const float pf = reachPlan(sl, t, 0.3f).pathF; if (t < 1120) KT_CHECK(pf <= prev + 1e-6f); prev = pf; if (pf < reachPlan(sl, tMin, 0.3f).pathF) tMin = t; }
    KT_CHECK(tMin >= 1115 && tMin <= 1125);                            // the lowest point (the grab) is at 30% of the slide
    prev = reachPlan(sl, 1125, 0.3f).pathF;
    for (uint32_t t = 1126; t < 1400; ++t) { const float pf = reachPlan(sl, t, 0.3f).pathF; KT_CHECK(pf >= prev - 1e-6f); prev = pf; }
    ReachPlan z = reachPlan(sl, 1200, 0.f);                           // no reach phase: carry from the start
    KT_CHECK(std::fabs(z.pathF - 0.45946f) < 1e-4f && z.carryW == 1.f);
}

static void testReachPlanStow() {
    Slide sl;
    sl.reset(true, 1000);
    sl.target(false, 1000, 400, 400);   // stow from the hand: 1000..1400
    ReachPlan a = reachPlan(sl, 1040, 0.3f);                          // p = 0.1: carrying, near the rest end of the path
    KT_CHECK(a.active && a.pathF > 0.8f && a.restMix == 0.f && a.carryW == 1.f);
    ReachPlan b = reachPlan(sl, 1196, 0.3f);                          // p = 0.49: q = 0.7 of the carry
    KT_CHECK(std::fabs(b.pathF - 0.24324f) < 1e-4f && b.carryW == 1.f);
    ReachPlan c = reachPlan(sl, 1290, 0.3f);                          // p = 0.725: release phase, the weapon is on the mount
    KT_CHECK(c.carryW == 0.f && c.restMix == 0.f && c.pathF > 0.f && c.pathF < 1.f);   // release: walking the arc mount -> rest, weapon on the mount
    ReachPlan d = reachPlan(sl, 1399, 0.3f);
    KT_CHECK(d.pathF > 0.99f && d.carryW == 0.f);                     // the hand ends at the rest end of the path
    float prev = 1.f;                                                  // stow: pathF falls to 0 during the carry (70%), then rises again during the release
    for (uint32_t t = 1001; t < 1278; ++t) { const float pf = reachPlan(sl, t, 0.3f).pathF; KT_CHECK(pf <= prev + 1e-6f); prev = pf; }
    prev = reachPlan(sl, 1281, 0.3f).pathF;
    for (uint32_t t = 1282; t < 1400; ++t) { const float pf = reachPlan(sl, t, 0.3f).pathF; KT_CHECK(pf >= prev - 1e-6f); prev = pf; }
    ReachPlan z = reachPlan(sl, 1399, 0.f);                           // no release phase: carry to the end
    KT_CHECK(z.pathF < 0.01f && z.restMix == 0.f);
}

static void testReachPlanPartialDraw() {
    Slide sl;
    sl.reset(false, 1000);
    sl.target(true, 1000, 400, 400);
    reachRetarget(sl, false, 1200, 400, 400, 0.3f);   // reversed half way: a stow from the draw's path position at 1200
    KT_CHECK(std::fabs(reachPlan(sl, 1200, 0.3f).pathF - 0.2278f) < 1e-4f);   // the retarget tick equals the old plan
    ReachPlan a = reachPlan(sl, 1210, 0.3f);
    KT_CHECK(a.active && std::fabs(a.pathF - 0.2135f) < 2e-3f);      // continues from where the arm was (no jump back to the rest end)
    sl.reset(false, 5000);
    sl.target(true, 5000, 400, 400);
    reachRetarget(sl, false, 5100, 400, 400, 0.3f);
    reachRetarget(sl, true, 5150, 400, 400, 0.3f);   // a draw from f > 0: no reach phase, carries on (the carry eases in from the stow's)
    ReachPlan b = reachPlan(sl, 5160, 0.3f);
    KT_CHECK(b.active && b.restMix == 0.f && std::fabs(b.carryW - 0.2804f) < 3e-3f);
}

// HW25: a retarget keeps the path position and carry weight continuous. Each case reverses a draw from the mount (1000..1400, g 0.3)
// to a stow at tick t, then checks the plan at t and the next tick.
static void testReachRetargetContinuity() {
    const uint32_t ticks[4] = { 1008u, 1040u, 1060u, 1200u };
    const float expPath[4] = { 0.98726f, 0.74074f, 0.50000f, 0.22780f };
    const float expCarry[4] = { 0.f, 0.f, 0.f, 1.f };
    for (int i = 0; i < 4; ++i) {
        const uint32_t t = ticks[i];
        Slide sl;
        sl.reset(false, 1000);
        sl.target(true, 1000, 400, 400);
        const ReachPlan before = reachPlan(sl, t, 0.3f);
        reachRetarget(sl, false, t, 400, 400, 0.3f);
        const ReachPlan at = reachPlan(sl, t, 0.3f), next = reachPlan(sl, t + 1u, 0.3f);
        KT_CHECK(std::fabs(at.pathF - before.pathF) < 1e-6f && std::fabs(at.carryW - before.carryW) < 1e-6f);   // no jump at the retarget
        // the next tick moves at the stow's normal speed. Do NOT compare with the tick before: the old reach moves 0.011-0.013 per tick there, as it should.
        KT_CHECK(std::fabs(next.pathF - at.pathF) < 0.01f && std::fabs(next.carryW - at.carryW) < 0.05f);
        KT_CHECK(std::fabs(at.pathF - expPath[i]) < 1e-4f && std::fabs(at.carryW - expCarry[i]) < 1e-4f);
    }
}

// HW25: the stow length after a reversal (Option B: ms * path distance / 2), the fresh-start lengths, the partial draw after a stow,
// and the carry-stage monotonicity after a reversal.
static void testReachRetargetEndsAndDuration() {
    const uint32_t ticks[4] = { 1008u, 1040u, 1060u, 1200u };
    const uint32_t lens[4] = { 397u, 348u, 300u, 246u };
    for (int i = 0; i < 4; ++i) {
        Slide sl;
        sl.reset(false, 1000);
        sl.target(true, 1000, 400, 400);
        reachRetarget(sl, false, ticks[i], 400, 400, 0.3f);
        KT_CHECK(sl.len == lens[i]);
    }
    Slide fs;                                                     // fresh stow from the hand: nothing running, frac length
    fs.reset(true, 1000);
    reachRetarget(fs, false, 1000, 400, 400, 0.3f);
    KT_CHECK(fs.len == 400u && fs.pathFrom == 1.f && fs.carryFrom == 1.f);
    Slide fd;                                                     // fresh draw from the mount: frac length
    fd.reset(false, 1000);
    reachRetarget(fd, true, 1000, 400, 400, 0.3f);
    KT_CHECK(fd.len == 400u);
    Slide same;                                                   // same direction: nothing stored, nothing changed
    same.reset(false, 1000);
    same.target(true, 1000, 400, 400);
    const float pf0 = same.pathFrom, cf0 = same.carryFrom;
    const uint32_t l0 = same.len;
    reachRetarget(same, true, 1050, 400, 400, 0.3f);
    KT_CHECK(same.pathFrom == pf0 && same.carryFrom == cf0 && same.len == l0);
    KT_CHECK(std::fabs(reachCarryF(0.f)) < 1e-6f && std::fabs(reachCarryF(1.f) - 1.f) < 1e-6f);
    // stow reversed from the carry at p 0.5 (len 246): over the carry share (172 ticks) pathF and carryW never rise; at 1200 pathF 0.2278, carryW 1
    Slide cs;
    cs.reset(false, 1000);
    cs.target(true, 1000, 400, 400);
    reachRetarget(cs, false, 1200, 400, 400, 0.3f);
    ReachPlan prevC = reachPlan(cs, 1200, 0.3f);
    KT_CHECK(std::fabs(prevC.pathF - 0.2278f) < 1e-4f && std::fabs(prevC.carryW - 1.f) < 1e-6f);
    for (uint32_t t = 1201u; t < 1200u + 172u; ++t) {
        const ReachPlan r = reachPlan(cs, t, 0.3f);
        KT_CHECK(r.pathF <= prevC.pathF + 1e-6f && r.carryW <= prevC.carryW + 1e-6f);
        prevC = r;
    }
    // stow from the hand at 1000, draw again at 1100: a partial draw (len 77) from P0 = 0.6139, pathF rises to 0.9946 with the carry on
    Slide dr;
    dr.reset(true, 1000);
    reachRetarget(dr, false, 1000, 400, 400, 0.3f);
    reachRetarget(dr, true, 1100, 400, 400, 0.3f);
    KT_CHECK(dr.len == 77u && std::fabs(dr.pathFrom - 0.6139f) < 1e-4f);
    float prevD = reachPlan(dr, 1100, 0.3f).pathF;
    KT_CHECK(std::fabs(prevD - 0.6139f) < 1e-4f);
    for (uint32_t t = 1101u; t < 1177u; ++t) {
        const ReachPlan r = reachPlan(dr, t, 0.3f);
        KT_CHECK(r.pathF >= prevD - 1e-6f && std::fabs(r.carryW - 1.f) < 1e-6f);
        prevD = r.pathF;
    }
    KT_CHECK(std::fabs(prevD - 0.9946f) < 1e-4f);                 // last tick of the draw
}

// HW25: the reach-to-carry hand is continuous at the grab. Its first ticks after the grab step by a fraction of a normal carry step.
static void testReachGrabSpeed() {
    const float g = 0.3f;                                        // grab share of the draw
    const float kSpeedFrac = 0.3f;                               // allowed first-tick step, as a share of the normal carry step 1 / ((1 - g) * 400)
    const float limit = kSpeedFrac * (1.f / (1.f - g) / 400.f);  // 0.00107
    Slide sl;
    sl.reset(false, 1000);
    sl.target(true, 1000, 400, 400);
    for (uint32_t k = 0; k < 5; ++k) {
        const float a = reachPlan(sl, 1120u + k, g).pathF, b = reachPlan(sl, 1121u + k, g).pathF;
        KT_CHECK(std::fabs(b - a) < limit);
    }
}

// HW25: holdW sets the reach weight at a retarget (w0). The weight starts there and joins the normal ramp at w = 1. Direct Slide::target calls
// (frac timing); a reversal in a test is always one of these, or reachRetarget.
static void testReachWeightHoldW() {
    // (a) reversal to the hip with holdW 1 (reset(false,1000), draw at 1000, reversal at 1200 = len 200): weight holds at 1 through the window, then 0 at the end
    Slide a;
    a.reset(false, 1000);
    a.target(true, 1000, 400, 400);
    a.target(false, 1200, 400, 400, 1.0f);
    KT_CHECK(closeF(reachWeight(a, 1200), 1.f) && closeF(reachWeight(a, 1210), 1.f) && closeF(reachWeight(a, 1220), 1.f) && closeF(reachWeight(a, 1240), 1.f));
    KT_CHECK(reachWeight(a, 1400) == 0.f);
    // (b) holdW = the weight at 1020 of the draw (0.15625): the value carries over at the retarget, then rises smoothly
    Slide b;
    b.reset(false, 1000);
    b.target(true, 1000, 400, 400);
    KT_CHECK(closeF(reachWeight(b, 1020), 0.15625f));
    b.target(false, 1020, 400, 400, 0.15625f);
    KT_CHECK(closeF(reachWeight(b, 1020), 0.15625f) && closeF(reachWeight(b, 1021), 0.28809f, 1e-4f));
    for (uint32_t t = 1020u; t < 1040u; ++t) KT_CHECK(reachWeight(b, t) <= 1.f);
    // (c) holdW 0: the weight is 0 at the draw's start and 1 mid-slide, as before HW25
    Slide c;
    c.reset(false, 1000);
    c.target(true, 1000, 400, 400, 0.f);
    KT_CHECK(reachWeight(c, 1000) == 0.f && reachWeight(c, 1200) == 1.f);
    // (d) holdW 1 with a 0.5 ramp: every tick of the slide stays in [0, 1]
    Slide d;
    d.reset(false, 1000);
    d.target(true, 1000, 400, 400, 1.f);
    for (uint32_t t = 1000u; t < 1400u; ++t) { const float w = reachWeightRamp(d, t, 0.5f); KT_CHECK(w >= 0.f && w <= 1.f); }
    // (e) NaN holdW: w0 is 0, so the weight starts at 0
    Slide e;
    e.reset(false, 1000);
    e.target(true, 1000, 400, 400, NAN);
    KT_CHECK(e.w0 == 0.f && reachWeight(e, 1000) == 0.f);
    // (f) a second same-direction target with another holdW is a no-op: the weight is unchanged at 20 sampled ticks
    Slide f;
    f.reset(false, 1000);
    f.target(true, 1000, 400, 400, 0.f);
    float before[20];
    for (uint32_t k = 0; k < 20u; ++k) before[k] = reachWeight(f, 1000u + 20u * k);
    f.target(true, 1050, 400, 400, 0.9f);   // same direction: no-op
    for (uint32_t k = 0; k < 20u; ++k) KT_CHECK(reachWeight(f, 1000u + 20u * k) == before[k]);
    // (g) a reset after a reversal clears w0: the weight is 0 at the reset tick
    Slide g;
    g.reset(false, 1000);
    g.target(true, 1000, 400, 400);
    g.target(false, 1200, 400, 400, 1.f);
    g.reset(true, 1210);
    KT_CHECK(g.w0 == 0.f && reachWeight(g, 1210) == 0.f);
    // (h) a zero-length reversal: the weight is 0 at every tick
    Slide h;
    h.reset(true, 1000);
    h.target(false, 1200, 0, 0, 1.f);
    KT_CHECK(h.len == 0u);
    for (uint32_t t = 1190u; t <= 1300u; ++t) KT_CHECK(reachWeight(h, t) == 0.f);
    // (i) a reversal across the counter wrap keeps holdW
    Slide wr;
    wr.reset(true, 0xFFFFFF00u - 10u);
    wr.target(false, 0xFFFFFF00u, 400, 400, 0.8f);
    KT_CHECK(closeF(reachWeight(wr, 0xFFFFFF00u), 0.8f, 1e-5f));
}

// HW25 review W2 (N1): reachRetarget captures the old arm weight into w0 before target(). A draw runs 1000..1400 (g 0.3); at tick 1010 the
// reach ramp (0.04) is mid-window, about 0.68. Reversing there must keep that weight: w0 equals the pre-reversal weight, and the next tick does not drop it.
static void testReachRetargetKeepsWeight() {
    Slide sl;
    sl.reset(false, 1000);
    sl.target(true, 1000, 400, 400);                        // draw from the mount: 1000..1400
    const uint32_t t = 1010u;
    const float pre = reachWeightRamp(sl, t, 0.04f);        // the old arm weight at the reversal
    KT_CHECK(pre > 0.3f);
    reachRetarget(sl, false, t, 400, 400, 0.3f, 0.04f);     // reverse to the hip
    KT_CHECK(std::fabs(sl.w0 - pre) < 1e-4f);               // (a) the stow starts at the old weight
    const float next = reachWeightRamp(sl, t + 1u, 0.04f);  // (b) the next tick does not drop it
    KT_CHECK(next >= sl.w0 - 0.05f);
}

// ---- wrist ----
static void testWristClampAndAngle() {
    const float z[3] = {0, 0, 1};
    KT_CHECK(std::fabs(reachQAngleDeg(qaxis(z, 90.f)) - 90.f) < 1e-3f);
    KT_CHECK(std::fabs(reachQAngleDeg(Q{-1, 0, 0, 0})) < 1e-3f);        // -identity is no rotation
    KT_CHECK(reachQAngleDeg(Q{NAN, 0, 0, 0}) == 0.f);
    const Q big = qaxis(z, 150.f);
    const Q lim = reachClampRot(big, 80.f);
    KT_CHECK(std::fabs(reachQAngleDeg(lim) - 80.f) < 1e-2f && std::fabs(lim.z) > 0.5f);   // same axis, angle limited
    const Q small = qaxis(z, 30.f);
    KT_CHECK(std::fabs(reachQAngleDeg(reachClampRot(small, 80.f)) - 30.f) < 1e-3f);       // within the limit: unchanged
    const Q neg = Q{-small.w, -small.x, -small.y, -small.z};                              // the same rotation with the other sign
    KT_CHECK(std::fabs(reachQAngleDeg(reachClampRot(neg, 80.f)) - 30.f) < 1e-3f);
    KT_CHECK(reachQAngleDeg(reachClampRot(Q{NAN, 1, 1, 1}, 80.f)) == 0.f);
}

static void testWristDelta() {
    const float x[3] = {1, 0, 0}, z[3] = {0, 0, 1};
    const Q mount = qaxis(z, 90.f), rest = Q{1, 0, 0, 0}, cur = rest;
    // at the grab (t = 0) the hand must turn to the mount orientation: delta applied to cur gives mount
    const Q d0 = reachWristDelta(mount, rest, 0.f, cur, 180.f, 1.f);
    const Q r0 = qmul(d0, cur);
    KT_CHECK(std::fabs(std::fabs(r0.w * mount.w + r0.x * mount.x + r0.y * mount.y + r0.z * mount.z) - 1.f) < 1e-4f);
    // at the rest end (t = 1) nothing to do
    KT_CHECK(reachQAngleDeg(reachWristDelta(mount, rest, 1.f, cur, 180.f, 1.f)) < 1e-3f);
    // the limit and the weight
    KT_CHECK(reachQAngleDeg(reachWristDelta(mount, rest, 0.f, cur, 40.f, 1.f)) <= 40.01f);
    const float half = reachQAngleDeg(reachWristDelta(mount, rest, 0.f, cur, 180.f, 0.5f));
    KT_CHECK(half > 30.f && half < 60.f);                                                  // weight 0.5 of a 90 deg turn
    KT_CHECK(reachQAngleDeg(reachWristDelta(mount, rest, 0.f, cur, 180.f, 0.f)) == 0.f);
    KT_CHECK(reachQAngleDeg(reachWristDelta(Q{NAN, 0, 0, 0}, rest, 0.f, cur, 180.f, 1.f)) == 0.f);
    (void)x;
}

// ---- hinged back-mount reach ----
static void testReachHinge() {
    const float S[3] = {0.2f, 0.f, 1.4f}, up[3] = {0, 0, 1}, fwd[3] = {0, 1, 0}, ou[3] = {1, 0, 0};
    const float L1 = 0.30f, L2 = 0.34f;
    const float mount[3] = {0.12f, -0.12f, 1.38f};   // upper back, behind and inboard of the shoulder
    const float rest[3] = {0.35f, 0.05f, 1.0f}, rpole[3] = {0.2f, -0.3f, -1.f};
    float T[3], P[3];
    // the ends: the mount at f = 0, the animation hand and elbow plane at f = 1
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, 0.f, T, P));
    KT_CHECK(closeV(T, mount, 1e-4f));
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, 1.f, T, P));
    KT_CHECK(closeV(T, rest, 1e-5f));
    const float rl = r3len(rpole);
    KT_CHECK(closeF(P[0], rpole[0] / rl, 1e-5f) && closeF(P[2], rpole[2] / rl, 1e-5f));
    // stage joints are continuous (T and pole)
    for (int k = 0; k < 2; ++k) {
        const float j = k == 0 ? kHingeS3 : kHingeS2;
        float Ta[3], Pa[3], Tb[3], Pb[3];
        KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, j - 1e-4f, Ta, Pa));
        KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, j + 1e-4f, Tb, Pb));
        KT_CHECK(distV(Ta, Tb) < 5e-3f);
        KT_CHECK(distV(Pa, Pb) < 5e-3f);
    }
    // the ready pose: upper arm hangs, forearm points forward, elbow tucked behind the arm line
    float E[3], rr = 0.f, sh[3], fh[3];
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, kHingeS2, T, P));
    const float Eh[3] = {S[0], S[1], S[2] - L1}, Hh[3] = {S[0], S[1] + L2, S[2] - L1};
    KT_CHECK(closeV(T, Hh, 1e-4f));
    KT_CHECK(solveTwoBone(S, Eh, Hh, T, E, &rr, P));
    KT_CHECK(closeV(E, Eh, 1e-3f));
    (void)sh; (void)fh;
    // stage 2: the elbow stays at 90 deg (|T-S| = sqrt(L1^2 + L2^2)) and the upper arm pitches monotonically from down to forward
    float lastY = -2.f;
    for (int i = 0; i <= 20; ++i) {
        const float f = kHingeS3 + (kHingeS2 - kHingeS3) * (1.f - i / 20.f);
        KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, f, T, P));
        float ts[3];
        sub3(T, S, ts);
        const float want = std::sqrt(L1 * L1 + L2 * L2);
        KT_CHECK(std::fabs(r3len(ts) - want) < 0.02f);   // the outward offset of the hand adds a few mm at most
        KT_CHECK(solveTwoBone(S, Eh, Hh, T, E, &rr, P));
        float es[3];
        sub3(E, S, es);
        KT_CHECK(closeF(r3len(es), L1, 1e-4f));
        KT_CHECK(es[1] >= lastY - 1e-3f);                // the upper arm swings forward as f falls (i rises)
        lastY = es[1];
    }
    KT_CHECK(lastY > 0.9f * L1);                         // ends pointing (almost) straight forward
    // stage 3: the upper arm stays parked forward while the forearm closes; a mount behind the shoulder lifts the elbow up (not inboard) for the grab
    for (int i = 0; i <= 20; ++i) {
        const float f = kHingeS3 * i / 20.f;
        KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, f, T, P));
        KT_CHECK(solveTwoBone(S, Eh, Hh, T, E, &rr, P));
        float es[3], eh[3];
        sub3(E, S, es);
        unit3(es, es);
        KT_CHECK(es[0] > -0.05f);                        // the elbow never swings inboard across the face
        KT_CHECK(es[2] > -0.05f);                        // nor drops below horizontal
        if (f >= 0.5f * kHingeS3) KT_CHECK(es[1] > 0.8f);   // the first half of the close (from the top): upper arm parked forward
        KT_CHECK(closeF(rr, 1.f, 1e-4f));                // the mount is reachable, never clamped
        sub3(T, E, eh);
        KT_CHECK(closeF(r3len(eh), L2, 1e-3f));
    }
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, 0.f, T, P));
    KT_CHECK(solveTwoBone(S, Eh, Hh, T, E, &rr, P));
    {
        float es[3], te[3];
        sub3(E, S, es);
        sub3(T, E, te);
        const float c = r3dot(es, te) / (r3len(es) * r3len(te));
        const float interior = 180.f - std::acos(c) * 57.2957795f;   // angle at the elbow between upper arm and forearm
        KT_CHECK(interior > 25.f && interior < 60.f);
    }
    // left arm mirrors: outward axis flipped and the mount mirrored give the mirrored path
    {
        const float SL[3] = {-0.2f, 0.f, 1.4f}, ouL[3] = {-1, 0, 0}, mL[3] = {-0.12f, -0.12f, 1.38f}, rL[3] = {-0.35f, 0.05f, 1.0f}, pL[3] = {-0.2f, -0.3f, -1.f};
        for (int i = 0; i <= 10; ++i) {
            const float f = i / 10.f;
            float Tr[3], Pr[3], Tl[3], Pl[3];
            KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, f, Tr, Pr));
            KT_CHECK(reachHinge(SL, L1, L2, up, fwd, ouL, mL, rL, pL, f, Tl, Pl));
            KT_CHECK(closeF(Tl[0], -Tr[0], 1e-4f) && closeF(Tl[1], Tr[1], 1e-4f) && closeF(Tl[2], Tr[2], 1e-4f));
            KT_CHECK(closeF(Pl[0], -Pr[0], 1e-4f) && closeF(Pl[1], Pr[1], 1e-4f) && closeF(Pl[2], Pr[2], 1e-4f));
        }
    }
    // several shoulder-height back mounts (upper back left / centre / right, behind the neck, above the shoulder): the whole path is smooth (no elbow flip), the
    // mount is always reached, and during the grab the elbow never crosses inboard of the shoulder by more than a few cm (it stays off the face)
    {
        const float Ms[6][3] = {{.12f, -.12f, 1.38f}, {0, -.12f, 1.4f}, {.05f, -.15f, 1.3f}, {.3f, -.1f, 1.45f}, {-.05f, -.2f, 1.35f}, {.2f, -.05f, 1.5f}};
        for (int m = 0; m < 6; ++m) {
            float prev[3] = {0, 0, 0}, worst = 0.f, minX = 9.f;
            bool okAll = true, reached = true;
            for (int i = 0; i <= 600; ++i) {
                const float f = i / 600.f;
                okAll = okAll && reachHinge(S, L1, L2, up, fwd, ou, Ms[m], rest, rpole, f, T, P) && solveTwoBone(S, Eh, Hh, T, E, &rr, P);
                if (f <= kHingeS3) { reached = reached && closeF(rr, 1.f, 1e-4f); if (E[0] < minX) minX = E[0]; }
                if (i) { const float dd = distV(E, prev); if (dd > worst) worst = dd; }
                for (int k = 0; k < 3; ++k) prev[k] = E[k];
            }
            KT_CHECK(okAll);
            KT_CHECK(reached);
            KT_CHECK(worst < 0.02f);
            KT_CHECK(minX > S[0] - 0.06f);
        }
    }
    // the height gate: shoulder height and above take the hinge, a mid / low back mount keeps the bowed line
    KT_CHECK(reachHingeFits(0.f, 0.64f) && reachHingeFits(-0.15f, 0.64f) && reachHingeFits(0.3f, 0.64f));
    KT_CHECK(!reachHingeFits(-0.2f, 0.64f) && !reachHingeFits(-0.5f, 0.64f));
    KT_CHECK(!reachHingeFits(kNaN, 0.64f) && !reachHingeFits(0.f, 0.f) && !reachHingeFits(0.f, kNaN));
    // degenerate input
    KT_CHECK(!reachHinge(S, 0.f, L2, up, fwd, ou, mount, rest, rpole, 0.5f, T, P));
    KT_CHECK(!reachHinge(S, L1, kNaN, up, fwd, ou, mount, rest, rpole, 0.5f, T, P));
    const float bad[3] = {kNaN, 0, 0};
    KT_CHECK(!reachHinge(S, L1, L2, up, fwd, ou, bad, rest, rpole, 0.5f, T, P));
    KT_CHECK(!reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, kNaN, T, P));
    const float zero[3] = {0, 0, 0};
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, zero, 0.9f, T, P) && finiteV(T) && finiteV(P));   // zero rest pole falls back to the ready pole
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, S, rest, rpole, 0.1f, T, P) && finiteV(T));                    // mount at the shoulder
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, -3.f, T, P) && closeV(T, mount, 1e-4f));   // f clamps
}

// ---- back-mount reach topology (HW27) ----
static void testReachTopology() {
    // dzArm = mount height above the shoulder in arm lengths (male arm 0.630 m), hingeFits = reachHingeFits; fy / fx are not inputs
    KT_CHECK(reachTopology(-0.006f, true) == ReachTopo::HingeBack);    // torsoUpr_g + 0.22 melee: the hinge fits
    KT_CHECK(reachTopology(-0.36f, false) == ReachTopo::OverBack);     // torsoUpr_g no offset: hinge fails, over the shoulder
    // torso_g is Hip: it sits 0.01-0.02 arm from pelvis_g, so LowBack needs a bone-name hint (see the hw27 card); both take the hip pole
    KT_CHECK(reachTopology(-0.52f, false) == ReachTopo::Hip);          // torso_g no offset
    KT_CHECK(reachTopology(-0.54f, false) == ReachTopo::Hip);          // pelvis_g (x-only offset)
    KT_CHECK(reachTopology(-0.77f, false) == ReachTopo::Thigh);        // rthigh_g: below the thigh line
    KT_CHECK(reachTopology(-0.30f, false) == ReachTopo::OverBack);     // synthetic
    KT_CHECK(reachTopology(kNaN, false) == ReachTopo::Hip);            // non-finite dzArm: Hip, no throw
    KT_CHECK(reachTopology(kNaN, true) == ReachTopo::Hip);
    // boundary ties: -0.45 is OverBack (inclusive), -0.66 is not Thigh (exclusive), the next float below -0.66 is Thigh
    KT_CHECK(reachTopology(-0.45f, false) == ReachTopo::OverBack);
    KT_CHECK(reachTopology(-0.66f, false) == ReachTopo::Hip);
    KT_CHECK(reachTopology(std::nextafter(-0.66f, -1.f), false) == ReachTopo::Thigh);
    // Thigh is checked before the hinge: a mount below the thigh line is never HingeBack; a hinge-fitting mount above it is
    KT_CHECK(reachTopology(-0.77f, true) == ReachTopo::Thigh);
    KT_CHECK(reachTopology(0.30f, true) == ReachTopo::HingeBack);
}

// ---- HW46: the slide's topology latch holds the whole enum and is recomputed at the next slide ----
static void testReachTopoLatch() {
    uint8_t set = 0, back = 0;
    KT_CHECK(reachTopoLatch(ReachTopo::OverBack, set, back) == ReachTopo::OverBack && set != 0 && back == 1);   // slide start: latched
    KT_CHECK(reachTopoLatch(ReachTopo::Hip, set, back) == ReachTopo::OverBack && back == 1);                   // mid-slide flip: kept
    KT_CHECK(reachTopoLatch(ReachTopo::Thigh, set, back) == ReachTopo::OverBack && back == 1);                 // still kept
    set = 0;                                                                                                    // clear site: slide ended
    KT_CHECK(reachTopoLatch(ReachTopo::Hip, set, back) == ReachTopo::Hip && back == 0);                        // next slide: recomputed
    set = 0;
    KT_CHECK(reachTopoLatch(ReachTopo::HingeBack, set, back) == ReachTopo::HingeBack && back == 1);
    set = 0;
    KT_CHECK(reachTopoLatch(ReachTopo::Thigh, set, back) == ReachTopo::Thigh && back == 0);
}

// ---- back-mount reach: per-category outward parameters (HW27) ----
static void testReachHingeOutParams() {
    const float S[3] = {0.2f, 0.f, 1.4f}, up[3] = {0, 0, 1}, fwd[3] = {0, 1, 0}, ou[3] = {1, 0, 0};
    const float L1 = 0.30f, L2 = 0.34f;
    const float mount[3] = {0.12f, -0.12f, 1.38f};
    const float rest[3] = {0.35f, 0.05f, 1.0f}, rpole[3] = {0.2f, -0.3f, -1.f};
    // goldens captured at HEAD (before the out parameters existed), default out values: the output must not move (tol 1e-6)
    struct Gold { float f; float T[3]; float P[3]; };
    const Gold gold[4] = {
        { 0.0f,  { 0.11999999f, -0.120000035f, 1.38f },       { 0.611836731f, 0.517095149f, 0.598555326f } },
        { 0.5f,  { 0.221216008f, 0.438516498f, 1.51533997f }, { 0.23872155f, 0.78227216f, -0.575380147f } },
        { 0.85f, { 0.230468765f, 0.281093717f, 1.07968736f }, { 0.0388822816f, -0.287130594f, -0.957101941f } },
        { 1.0f,  { 0.349999994f, 0.0500000007f, 1.f },        { 0.188144162f, -0.282216251f, -0.940720856f } },
    };
    for (int k = 0; k < 4; ++k) {
        float T[3], P[3];
        KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, gold[k].f, T, P));
        KT_CHECK(closeV(T, gold[k].T, 1e-6f) && closeV(P, gold[k].P, 1e-6f));
    }
    // outT moves the hand outward in stage 3 (f = 0.1) and stage 2 (f = 0.6): 0.0 against the default 0.10
    float T0[3], T1[3], Ta[3], Tb[3], Pa[3], Pb[3];
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, 0.1f, T0, Pa, 0.f));
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, 0.1f, T1, Pb));
    KT_CHECK(distV(T0, T1) > 1e-3f);
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, 0.6f, Ta, Pa, 0.f));
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, 0.6f, Tb, Pb));
    KT_CHECK(distV(Ta, Tb) > 1e-3f);
    // outPole moves the parked pole in stage 2 (f = 0.5): rifle 0.25 against heavy 0.45
    float Pr[3], Ph[3], Tr[3], Th[3];
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, 0.5f, Tr, Pr, kHingeOutT, kHingeOutPoleRifle));
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, 0.5f, Th, Ph, kHingeOutT, kHingeOutPoleHeavy));
    KT_CHECK(distV(Pr, Ph) > 1e-3f);
    // every category's out values keep the path finite across the whole sweep
    const float outPoles[3] = { kHingeOutPole, kHingeOutPoleRifle, kHingeOutPoleHeavy };
    for (int k = 0; k < 3; ++k)
        for (int i = 0; i <= 10; ++i) {
            float T[3], P[3];
            KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, i / 10.f, T, P, kHingeOutT, outPoles[k]) && finiteV(T) && finiteV(P));
        }
}

// ---- wrist sign hint (HW26) ----
// Continued input: the wrist turns 0..299 deg about +z in 1 deg steps. The input quat is the raw qaxis (its sign flips past 180 deg) and
// every odd step is negated (the same rotation). With the previous output as the hint, the axis stays +z and the angle clamps at 120 deg.
static void testWristHintKeepsAxis() {
    const float z[3] = {0, 0, 1};
    Q prev{ 1, 0, 0, 0 };
    for (int a = 0; a <= 299; ++a) {
        Q dq = qaxis(z, (float)a);
        if (a & 1) dq = negQ(dq);
        const Q out = reachClampRot(dq, 120.f, &prev);
        if (a > 0) KT_CHECK(out.z > 0.f);
        KT_CHECK(std::fabs(reachQAngleDeg(out) - (float)(a < 120 ? a : 120)) < 0.01f);
        prev = out;
    }
    // edge: at 299 the hint still holds (clamped, +z); at 301 it has snapped (documented; the input is not capped)
    const Q rot120 = qaxis(z, 120.f);
    KT_CHECK(reachClampRot(qaxis(z, 299.f), 120.f, &rot120).z > 0.f);
    KT_CHECK(reachClampRot(qaxis(z, 301.f), 120.f, &rot120).z < 0.f);
    // control: without the hint the same input past 180 deg takes the other sign (the old flip)
    KT_CHECK(reachClampRot(qaxis(z, 200.f), 120.f).z < 0.f);
}

// The no-hint forms are the old behaviour: the default argument equals an explicit nullptr bit for bit, and hard-coded values pin the
// no-hint path (the existing wrist tests pin it too).
static void testWristHintDefaultsUnchanged() {
    const float z[3] = {0, 0, 1}, x[3] = {1, 0, 0};
    const Q in = qaxis(z, 80.f);
    KT_CHECK(closeQ(reachClampRot(in, 80.f), in, 1e-6f) && reachClampRot(in, 80.f).z > 0.f);   // within the limit: input, sign kept
    const Q in30 = qaxis(z, 30.f);
    KT_CHECK(closeQ(reachClampRot(negQ(in30), 80.f), in30, 1e-6f));                             // negated input: shorter arc, sign kept
    const Q dqs[4] = { qaxis(z, 150.f), negQ(qaxis(z, 150.f)), qaxis(x, 40.f), Q{ 0.3f, 0.5f, -0.2f, 0.7f } };
    for (int k = 0; k < 4; ++k) {
        KT_CHECK(exactQ(reachClampRot(dqs[k], 80.f), reachClampRot(dqs[k], 80.f, nullptr)));
        KT_CHECK(exactQ(reachClampRot(dqs[k], 120.f), reachClampRot(dqs[k], 120.f, nullptr)));
    }
    const Q mount = qaxis(z, 90.f), rest = Q{ 1, 0, 0, 0 }, cur = qaxis(x, 25.f);
    const float ts[3] = { 0.f, 0.4f, 1.f };
    for (int k = 0; k < 3; ++k)
        KT_CHECK(exactQ(reachWristDelta(mount, rest, ts[k], cur, 120.f, 0.6f), reachWristDelta(mount, rest, ts[k], cur, 120.f, 0.6f, nullptr)));
}

// reachWristDelta with the hint: cur = mount * qaxis(z, a) (raw input, sign flips past 180 deg), the hint is the previous return (identity
// at the start). Each return's axis (xyz part) must keep a positive dot with the first return's axis, and the angle must be min(a, 120).
// Control: the same input without the hint flips the axis.
static void testWristDeltaHintSweep() {
    const float z[3] = {0, 0, 1};
    const Q mount = qaxis(z, 90.f), rest = Q{ 1, 0, 0, 0 };
    Q prev{ 1, 0, 0, 0 }, ref{ 1, 0, 0, 0 };
    bool haveRef = false;
    for (int a = 1; a <= 299; ++a) {
        const Q cur = qmul(mount, qaxis(z, (float)a));
        const Q out = reachWristDelta(mount, rest, 0.f, cur, 120.f, 1.f, &prev);
        if (!haveRef) { ref = out; haveRef = true; }
        KT_CHECK(out.x * ref.x + out.y * ref.y + out.z * ref.z > 0.f);
        KT_CHECK(std::fabs(reachQAngleDeg(out) - (float)(a < 120 ? a : 120)) < 0.01f);
        prev = out;
    }
    const Q flipped = reachWristDelta(mount, rest, 0.f, qmul(mount, qaxis(z, 200.f)), 120.f, 1.f);
    KT_CHECK(flipped.x * ref.x + flipped.y * ref.y + flipped.z * ref.z < 0.f);
}

// HW44c: the wrist target across the 180 deg crossing. The hand turns from the mount by 0 -> 300 deg in 0.1 deg steps (about x, under a non-identity mount). With
// the previous target fed back (reachTargetQ / reachWristDelta prevTgt) the target moves < 5 deg per step at t = 0.25 / 0.5 / 0.75. Control: the blendQ path jumps
// (> 90 deg). No previous target: reachWristDelta and the target are the old formula bit for bit.
static void testWristTargetContinuity() {
    const float x[3] = { 1, 0, 0 };
    const float z[3] = { 0, 0, 1 };
    const Q mount = qaxis(z, 37.f);
    const float ts[3] = { 0.25f, 0.5f, 0.75f };
    float worst = 0.f, ctlWorst = 0.f;
    bool ok = true;
    for (int k = 0; k < 3; ++k) {
        Q prevT{ 1, 0, 0, 0 }, ctlPrev{ 1, 0, 0, 0 };
        for (int i = 0; i <= 3000; ++i) {
            Q hand = qmul(mount, qaxis(x, 0.1f * static_cast<float>(i)));
            if (k == 2 && (i % 7) < 3) hand = { -hand.w, -hand.x, -hand.y, -hand.z };   // the engine matrix -> quat sign is arbitrary: the target must not care
            const Q tgt = reachTargetQ(mount, hand, ts[k], i > 0 ? &prevT : nullptr);
            const Q ctl = blendQ(mount, hand, easeSmooth(ts[k]));
            if (i > 0) {
                const float s = reachQAngleDeg(qmul(reachConj(prevT), tgt));
                const float c = reachQAngleDeg(qmul(reachConj(ctlPrev), ctl));
                if (s > worst) worst = s;
                if (c > ctlWorst) ctlWorst = c;
            }
            ok = ok && exactQ(reachTargetQ(mount, hand, ts[k], nullptr), blendQ(mount, hand, easeSmooth(ts[k])));
            prevT = tgt;
            ctlPrev = ctl;
        }
    }
    KT_CHECK(worst < 5.f);
    KT_CHECK(ctlWorst > 90.f);
    KT_CHECK(ok);
    // reachWristDelta: no previous target is the old result; tgtOut is the target.
    const Q cur = qmul(mount, qaxis(x, 50.f)), to = qmul(mount, qaxis(x, 200.f));
    const Q old = blendQ(Q{ 1, 0, 0, 0 }, reachClampRot(qmul(blendQ(mount, to, easeSmooth(0.4f)), reachConj(cur)), 120.f, nullptr), 0.6f);
    Q tgtOut{ 0, 0, 0, 0 };
    KT_CHECK(exactQ(reachWristDelta(mount, to, 0.4f, cur, 120.f, 0.6f), old));
    KT_CHECK(exactQ(reachWristDelta(mount, to, 0.4f, cur, 120.f, 0.6f, nullptr, nullptr, &tgtOut), old));
    KT_CHECK(exactQ(tgtOut, blendQ(mount, to, easeSmooth(0.4f))));
}

// ---- hand-frame residual (HW26) ----
// Largest angle between consecutive frames of reachResidual(qaxis(x, grab), t, ms), frames at t = k * 1000 / 60 and at t = ms (60 fps).
static float residualStepMaxDeg(float grab, float ms) {
    const float x[3] = {1, 0, 0};
    const Q R = qaxis(x, grab);
    Q prev = reachResidual(R, 0.f, ms);
    float worst = 0.f;
    for (int k = 1;; ++k) {
        const float t = (float)k * 1000.f / 60.f;
        const bool last = t >= ms;
        const Q cur = reachResidual(R, last ? ms : t, ms);
        const float step = reachQAngleDeg(qmul(reachConj(prev), cur));
        if (step > worst) worst = step;
        prev = cur;
        if (last) break;
    }
    return worst;
}

static void testResidualDecay() {
    const float x[3] = {1, 0, 0};
    const float ms = (float)kReachResidMs;
    const Q id{ 1, 0, 0, 0 };
    const float thetas[3] = { 60.f, 120.f, 180.f };
    for (int k = 0; k < 3; ++k) {
        const float th = thetas[k];
        const Q R = qaxis(x, th);
        KT_CHECK(std::fabs(reachQAngleDeg(reachResidual(R, 0.f, ms)) - th) < 1e-3f);   // Rres at t = 0
        KT_CHECK(exactQ(reachResidual(R, ms, ms), id));                                 // identity at t = ms, exactly
        KT_CHECK(exactQ(reachResidual(R, ms + 50.f, ms), id));                          // and after it
        KT_CHECK(closeQSign(reachResidual(R, -20.f, ms), R, 1e-5f));                    // t < 0: Rres (sign may be the other one)
        float prevA = reachQAngleDeg(reachResidual(R, 0.f, ms));
        for (float t = 5.f; t <= ms; t += 5.f) {                                        // non-increasing angle, 5 ms steps
            const float a = reachQAngleDeg(reachResidual(R, t, ms));
            KT_CHECK(a <= prevA + 1e-3f);
            prevA = a;
        }
    }
    const Q R90 = qaxis(x, 90.f);
    KT_CHECK(exactQ(reachResidual(R90, 0.f, 0.f), id) && exactQ(reachResidual(R90, 0.f, -5.f), id));   // ms <= 0: identity
    KT_CHECK(exactQ(reachResidual(R90, 0.f, kInf), id));                                                // ms = +inf: identity
    KT_CHECK(exactQ(reachResidual(R90, 0.f, kNaN), id));                                                // ms NaN: identity
    KT_CHECK(exactQ(reachResidual(Q{ kNaN, 0, 0, 0 }, 0.f, ms), id));                                   // non-finite Rres: identity
}

// Residual alone, 1 deg grab steps 60..180 at kReachResidMs: no frame step of 25 deg or more. Control: the old 130 ms default fails at 180 deg.
static void testResidualStepBound() {
    float worst = 0.f;
    int worstGrab = 0;
    for (int g = 60; g <= 180; ++g) {
        const float s = residualStepMaxDeg((float)g, (float)kReachResidMs);
        if (s > worst) { worst = s; worstGrab = g; }
    }
    if (!(worst < 25.f)) std::printf("residual step %.3f deg at grab %d\n", worst, worstGrab);
    KT_CHECK(worst < 25.f);
    KT_CHECK(residualStepMaxDeg(180.f, 130.f) > 25.f);
}

// ---- elbow pole bias from the target grip (HW29b) ----
static bool sameBits(const float* a, const float* b) { return std::memcmp(a, b, 3 * sizeof(float)) == 0; }
static float angleDeg(const float* a, const float* b) {   // unsigned angle between two 3-vectors (lengths need not be 1), degrees
    const float c[3] = { a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] };
    return std::atan2(r3len(c), r3dot(a, b)) * 57.2957795f;
}

static void testPoleBiasDisabled() {
    const float pole[3] = {0.2f, -0.3f, -1.f}, grip[3] = {0.5f, 0.8660254f, 0.f};
    const float nanPole[3] = {kNaN, -0.3f, -1.f}, nanGrip[3] = {0.5f, kNaN, 0.f};
    float out[3];
    reachPoleBias(pole, grip, 0.f, 0.f, kPoleBiasDeg, out);            KT_CHECK(sameBits(out, pole));      // amount 0
    reachPoleBias(pole, grip, 0.f, 1.f, 0.f, out);                     KT_CHECK(sameBits(out, pole));      // maxDeg 0
    reachPoleBias(pole, grip, kPoleBiasEndF, 1.f, kPoleBiasDeg, out); KT_CHECK(sameBits(out, pole));      // f at the end of the bias
    reachPoleBias(pole, grip, 0.9f, 1.f, kPoleBiasDeg, out);           KT_CHECK(sameBits(out, pole));      // f past the end
    reachPoleBias(pole, nanGrip, 0.f, 1.f, kPoleBiasDeg, out);         KT_CHECK(sameBits(out, pole));      // NaN grip
    reachPoleBias(nanPole, grip, 0.f, 1.f, kPoleBiasDeg, out);         KT_CHECK(sameBits(out, nanPole));   // NaN pole
}

static void testPoleBiasBoundAndPlane() {
    int n = 0;
    while (n < 200) {   // deterministic (pole, grip) pairs; near-zero vectors are redrawn
        const float p[3] = { rndSigned(), rndSigned(), rndSigned() }, g[3] = { rndSigned(), rndSigned(), rndSigned() };
        if (r3len(p) < 0.1f || r3len(g) < 0.1f) continue;
        ++n;
        const float f = 0.3f * rnd01(), amount = rnd01();   // f in [0, 0.3), amount in [0, 1)
        float out[3];
        reachPoleBias(p, g, f, amount, kPoleBiasDeg, out);
        KT_CHECK(angleDeg(out, p) <= kPoleBiasDeg * 1.0001f);            // never turns past the cap
        KT_CHECK(angleDeg(out, p) <= angleDeg(p, g) + 1e-4f);            // never past the grip
        KT_CHECK(angleDeg(out, g) <= angleDeg(p, g) + 1e-4f);            // turns toward the grip, never away
        KT_CHECK(std::fabs(r3len(out) - 1.f) < 1e-5f);                   // unit
        const float nrm[3] = { p[1] * g[2] - p[2] * g[1], p[2] * g[0] - p[0] * g[2], p[0] * g[1] - p[1] * g[0] };
        const float ln = r3len(nrm);
        if (ln > 1e-3f) KT_CHECK(std::fabs(r3dot(out, nrm)) / ln < 1e-5f);   // in the plane of p and g
    }
}

static void testPoleBiasFades() {
    const float pole[3] = {1.f, 0.f, 0.f}, grip[3] = {0.5f, 0.8660254f, 0.f};   // 60 deg away, past the 20 deg cap
    float o0[3], o1[3], o2[3], o3[3];
    reachPoleBias(pole, grip, 0.f, 1.f, kPoleBiasDeg, o0);
    reachPoleBias(pole, grip, 0.15f, 1.f, kPoleBiasDeg, o1);
    reachPoleBias(pole, grip, 0.29f, 1.f, kPoleBiasDeg, o2);
    reachPoleBias(pole, grip, kPoleBiasEndF, 1.f, kPoleBiasDeg, o3);
    const float a0 = angleDeg(o0, pole), a1 = angleDeg(o1, pole), a2 = angleDeg(o2, pole), a3 = angleDeg(o3, pole);
    KT_CHECK(closeF(a0, kPoleBiasDeg, 1e-3f));    // full cap at the start of the reach
    KT_CHECK(closeF(a1, 0.5f * kPoleBiasDeg, 1e-2f));   // half weight at f = 0.15 (easeSmooth(0.5) = 0.5)
    KT_CHECK(a0 >= a1 && a1 >= a2 && a2 > a3);
    KT_CHECK(a3 == 0.f);
}

static void testPoleBiasHingeInvariant() {
    // the testReachHinge setup
    const float S[3] = {0.2f, 0.f, 1.4f}, up[3] = {0, 0, 1}, fwd[3] = {0, 1, 0}, ou[3] = {1, 0, 0};
    const float L1 = 0.30f, L2 = 0.34f;
    const float mount[3] = {0.12f, -0.12f, 1.38f};
    const float rest[3] = {0.35f, 0.05f, 1.0f}, rpole[3] = {0.2f, -0.3f, -1.f};
    const float grip[3] = {-1.f, 0.f, 0.f};   // worst case: the bias pulls the pole inboard, toward the face
    float biased[3];
    reachPoleBias(rpole, grip, 0.f, 1.f, kPoleBiasDeg, biased);
    KT_CHECK(!sameBits(biased, rpole));
    float T0[3], P0[3], T[3], P[3], E[3], rr = 0.f;
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, rpole, 0.f, T0, P0));
    KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, biased, 0.f, T, P));
    KT_CHECK(closeV(T, T0, 1e-6f) && closeV(P, P0, 1e-6f));   // restPole is read only past kHingeS2, so the stage 3 hinge output does not move
    const float Eh[3] = {S[0], S[1], S[2] - L1}, Hh[3] = {S[0], S[1] + L2, S[2] - L1};
    for (int i = 0; i <= 20; ++i) {
        const float f = kHingeS3 * i / 20.f;
        KT_CHECK(reachHinge(S, L1, L2, up, fwd, ou, mount, rest, biased, f, T, P));
        KT_CHECK(solveTwoBone(S, Eh, Hh, T, E, &rr, P));
        float es[3];
        sub3(E, S, es);
        unit3(es, es);
        KT_CHECK(es[0] > -0.05f);   // the elbow never swings inboard across the face (kHingeMinOut)
        KT_CHECK(es[2] > -0.05f);   // nor drops below horizontal
    }
}

// HW45 R2: reachDqTail appends " dq=%.0f dj=%.0f" (deg) to a REACH piece 3. Never truncates: skipped when the result would pass 200 bytes or reach cap.
// NaN or a value outside [0, 360] prints 0. Returns the new length.
static void testDqTail() {
    char a[256];
    strcpy(a, "REACH3 x");
    int n = reachDqTail(a, sizeof a, 8, 181.4f, 3.2f);
    KT_CHECK(n == 20 && strcmp(a, "REACH3 x dq=181 dj=3") == 0);
    KT_CHECK(strlen(a) == static_cast<size_t>(n));
    // boundary: 188 + 12 = 200 is kept, 189 + 12 = 201 is skipped
    char e[256];
    memset(e, 'e', 188);
    e[188] = '\0';
    n = reachDqTail(e, sizeof e, 188, 181.4f, 3.2f);
    KT_CHECK(n == 200 && strlen(e) == 200 && strcmp(e + 188, " dq=181 dj=3") == 0);
    char f[256] = {};
    memset(f, 'f', 189);
    char fCopy[256] = {};
    memcpy(fCopy, f, 190);
    n = reachDqTail(f, sizeof f, 189, 181.4f, 3.2f);
    KT_CHECK(n == 189 && memcmp(f, fCopy, 190) == 0);
    // a 195-byte piece is left unchanged
    char b[256] = {};
    memset(b, 'b', 195);
    char bCopy[256] = {};
    memcpy(bCopy, b, 196);
    n = reachDqTail(b, sizeof b, 195, 181.4f, 3.2f);
    KT_CHECK(n == 195 && memcmp(b, bCopy, 196) == 0);
    // cap smaller than the piece: 88 + 12 = 100 >= cap 100 is skipped, 87 + 12 = 99 < cap 100 is kept
    char small[100];
    memset(small, 's', 88);
    small[88] = '\0';
    n = reachDqTail(small, 100, 88, 181.4f, 3.2f);
    KT_CHECK(n == 88 && strlen(small) == 88);
    char small2[100];
    memset(small2, 's', 87);
    small2[87] = '\0';
    n = reachDqTail(small2, 100, 87, 181.4f, 3.2f);
    KT_CHECK(n == 99 && strcmp(small2 + 87, " dq=181 dj=3") == 0);
    // NaN prints 0; out of range (negative, above 360, infinite) prints 0; 360 is kept
    char nanBuf[256];
    strcpy(nanBuf, "REACH3");
    n = reachDqTail(nanBuf, sizeof nanBuf, 6, kNaN, kNaN);
    KT_CHECK(n == 16 && strcmp(nanBuf + 6, " dq=0 dj=0") == 0);
    char oor[256];
    strcpy(oor, "R");
    n = reachDqTail(oor, sizeof oor, 1, -5.f, 361.f);
    KT_CHECK(n == 11 && strcmp(oor, "R dq=0 dj=0") == 0);
    char top[256];
    strcpy(top, "R");
    n = reachDqTail(top, sizeof top, 1, 360.f, kInf);
    KT_CHECK(n == 13 && strcmp(top, "R dq=360 dj=0") == 0);
    // null buf and used out of range return used unchanged
    KT_CHECK(reachDqTail(nullptr, 100, 10, 1.f, 1.f) == 10);
    KT_CHECK(reachDqTail(a, sizeof a, -1, 1.f, 1.f) == -1);
    KT_CHECK(reachDqTail(a, 20, 20, 1.f, 1.f) == 20);
}

// HW50 (reach_ease_soft_<cat>): soft = false is reachCarryF byte for byte, and the reachPlan default equals the explicit false. soft = true is smoothstep
// on the carry: f(0) = 0, f(1) = 1, zero slope at both ends, monotone; the reach and the release and carryW do not change.
static bool sameBits(float a, float b) { return std::memcmp(&a, &b, sizeof a) == 0; }
static void testEaseSoft() {
    for (int i = 0; i <= 512; ++i) KT_CHECK(sameBits(reachCarryEase(i / 512.f, false), reachCarryF(i / 512.f)));
    KT_CHECK(sameBits(reachCarryEase(-0.5f, false), reachCarryF(-0.5f)));
    KT_CHECK(sameBits(reachCarryEase(1.5f, false), reachCarryF(1.5f)));
    KT_CHECK(sameBits(reachCarryEase(kNaN, false), reachCarryF(kNaN)));
    Slide d, st, pd, ps;   // draw and stow from rest, and a draw / stow reversed half way (partial)
    d.reset(false, 1000);  d.target(true, 1000, 400, 400);
    st.reset(true, 1000);  st.target(false, 1000, 400, 400);
    pd.reset(false, 1000); pd.target(true, 1000, 400, 400); reachRetarget(pd, false, 1200, 400, 400, 0.3f);
    ps.reset(true, 1000);  ps.target(false, 1000, 400, 400); reachRetarget(ps, true, 1200, 400, 400, 0.3f);
    const Slide* const sls[4] = { &d, &st, &pd, &ps };
    const float es[2] = { 0.f, 0.4f };
    for (const Slide* s : sls)
        for (uint32_t t = 1000; t < 1400; ++t)
            for (const float e : es) {
                const ReachPlan a = reachPlan(*s, t, 0.3f, e), b = reachPlan(*s, t, 0.3f, e, false);
                KT_CHECK(a.active == b.active && sameBits(a.pathF, b.pathF) && sameBits(a.restMix, b.restMix) && sameBits(a.carryW, b.carryW));
                KT_CHECK(sameBits(a.extractK, b.extractK) && sameBits(a.oriF, b.oriF));
            }
    KT_CHECK(reachCarryEase(0.f, true) == 0.f && reachCarryEase(1.f, true) == 1.f);
    const float h = 1e-3f;
    KT_CHECK(std::fabs(reachCarryEase(h, true) - reachCarryEase(0.f, true)) / h < 0.01f);          // zero slope at q = 0
    KT_CHECK(std::fabs(reachCarryEase(1.f, true) - reachCarryEase(1.f - h, true)) / h < 0.01f);    // zero slope at q = 1
    float prev = -1.f;
    for (int i = 0; i <= 512; ++i) { const float f = reachCarryEase(i / 512.f, true); KT_CHECK(f >= prev); prev = f; }
    // the knob is wired: soft = true changes the carry of a draw and a stow; the reach (draw p < g), the release (stow p >= 1 - g) and carryW stay put
    bool carryDiffD = false, carryDiffS = false;
    for (uint32_t t = 1000; t < 1400; ++t) {
        const ReachPlan dh = reachPlan(d, t, 0.3f, 0.f, true), dn = reachPlan(d, t, 0.3f);
        const ReachPlan sh = reachPlan(st, t, 0.3f, 0.f, true), sn = reachPlan(st, t, 0.3f);
        KT_CHECK(sameBits(dh.carryW, dn.carryW) && sameBits(sh.carryW, sn.carryW));
        if (t < 1120) KT_CHECK(sameBits(dh.pathF, dn.pathF));
        if (t >= 1282) KT_CHECK(sameBits(sh.pathF, sn.pathF));   // past the carry / release boundary (p = 0.7) with float margin
        if (!sameBits(dh.pathF, dn.pathF)) carryDiffD = true;
        if (!sameBits(sh.pathF, sn.pathF)) carryDiffS = true;
    }
    KT_CHECK(carryDiffD && carryDiffS);
    // a reversal mid-carry keeps pathF and carryW continuous at the retarget tick when the retarget and the reader pass the same flag (draw reversed to a stow,
    // stow reversed to a draw), and moves at a normal speed on the next tick
    const uint32_t revT[2][3] = { { 1200u, 1250u, 1330u }, { 1040u, 1200u, 1240u } };   // draw: inside the carry (starts 1120); stow: inside the carry (until 1280)
    for (int dirHand = 0; dirHand < 2; ++dirHand)
        for (int i = 0; i < 3; ++i) {
            const uint32_t t = revT[dirHand][i];
            Slide sl;
            sl.reset(dirHand != 0, 1000);
            sl.target(dirHand == 0, 1000, 400, 400);
            const ReachPlan before = reachPlan(sl, t, 0.3f, 0.f, true);
            reachRetarget(sl, dirHand != 0, t, 400, 400, 0.3f, 0.04f, 0.f, true);
            const ReachPlan at = reachPlan(sl, t, 0.3f, 0.f, true), next = reachPlan(sl, t + 1u, 0.3f, 0.f, true);
            KT_CHECK(before.active && at.active);
            KT_CHECK(std::fabs(at.pathF - before.pathF) < 1e-6f && std::fabs(at.carryW - before.carryW) < 1e-6f);
            KT_CHECK(std::fabs(next.pathF - at.pathF) < 0.01f);
        }
    {   // the trap the flag guards against: a mid-carry draw reversed with soft off and read with soft on jumps at the retarget tick
        Slide sl;
        sl.reset(false, 1000);
        sl.target(true, 1000, 400, 400);
        const ReachPlan before = reachPlan(sl, 1200, 0.3f, 0.f, true);
        reachRetarget(sl, false, 1200, 400, 400, 0.3f);
        const ReachPlan at = reachPlan(sl, 1200, 0.3f, 0.f, true);
        KT_CHECK(std::fabs(at.pathF - before.pathF) > 0.02f);
    }
}

// Track 1: 3D dynamic mount topology and continuous over-weight across kOverDz (-0.45) and front/chest offsets (fyArm > kFrontFy).
static void testReachTopology3DAndOverWeight3D() {
    // Stock back/hip/thigh mounts (fy <= 0) match reachTopology bit for bit
    const float dzs[6] = { -0.77f, -0.66f, -0.54f, -0.45f, -0.36f, -0.01f };
    for (int i = 0; i < 6; ++i) {
        for (int hf = 0; hf < 2; ++hf) {
            const bool h = hf != 0;
            KT_CHECK(reachTopology3D(0.15f, -0.10f, dzs[i], h) == reachTopology(dzs[i], h));
            KT_CHECK(reachTopology3D(kNaN, kNaN, dzs[i], h) == reachTopology(dzs[i], h));
        }
    }
    // Front/chest mount (fyArm > kFrontFy = 0.05) above kThighDz resolves to Hip, never HingeBack or OverBack
    KT_CHECK(reachTopology3D(0.10f, 0.15f, -0.05f, true) == ReachTopo::Hip);
    KT_CHECK(reachTopology3D(0.10f, 0.15f, -0.35f, false) == ReachTopo::Hip);
    KT_CHECK(reachTopology3D(0.10f, 0.15f, -0.77f, true) == ReachTopo::Thigh);   // Thigh still wins below kThighDz

    // Stock torsoUpr_g (-0.36) is still exact 1.0, stock pelvis_g (-0.54) matches reachOverWeight
    KT_CHECK(reachOverWeight3D(0.15f, -0.10f, -0.36f, ReachTopo::OverBack) == 1.f);
    KT_CHECK(reachOverWeight3D(0.15f, -0.10f, -0.05f, ReachTopo::HingeBack) == 1.f);
    KT_CHECK(closeF(reachOverWeight3D(0.15f, 0.f, -0.54f, ReachTopo::Hip), reachOverWeight(-0.54f * 0.64f, 0.64f), 1e-6f));

    // C1 continuity across the dzArm = -0.45 (Hip <-> OverBack) boundary: no 0.76 -> 1.0 cliff
    const float uBelow = reachOverWeight3D(0.15f, -0.10f, -0.4501f, ReachTopo::Hip);
    const float uAbove = reachOverWeight3D(0.15f, -0.10f, -0.4499f, ReachTopo::OverBack);
    KT_CHECK(std::fabs(uAbove - uBelow) < 2e-3f);

    // Front/chest mount dampens u so high chest holsters don't raise the elbow above the head
    KT_CHECK(reachOverWeight3D(0.10f, 0.25f, -0.36f, ReachTopo::Hip) == 0.f);
    const float uMidChest = reachOverWeight3D(0.10f, 0.125f, -0.36f, ReachTopo::Hip);
    KT_CHECK(uMidChest > 0.f && uMidChest < 1.f);
}

// Track 1: 3D bulge and pole adaptation for thigh, cross-draw, and lumbar editor placements (stock mounts bit-identical).
static void testReachAdaptBulgeAndPole3D() {
    const ReachStyle st = reachDefaultStyle(4);   // pistol
    const Q id = { 1.f, 0.f, 0.f, 0.f };
    float bOut[3], overOut = -1.f;

    // 1. Stock pelvis_g (fx=0.28, fy=0, dz=-0.54) and torsoUpr_g (fx=0.15, fy=-0.10, dz=-0.36): bit-identical to st.bulge and reachPole
    reachAdaptBulge(st, 0.18f, 0.28f, 0.f, -0.54f, false, bOut, overOut);
    KT_CHECK(sameBits(bOut, st.bulge) && sameBits(overOut, 0.18f));
    reachAdaptBulge(st, 1.f, 0.15f, -0.10f, -0.36f, false, bOut, overOut);
    KT_CHECK(sameBits(bOut, st.bulge) && sameBits(overOut, 1.f));

    float pRef[3], p3D[3];
    KT_CHECK(reachPole(st, 0.18f, false, id, nullptr, 0.f, pRef));
    KT_CHECK(reachPole3D(st, 0.18f, 0.28f, 0.f, -0.54f, false, id, nullptr, 0.f, p3D));
    KT_CHECK(sameBits(pRef, p3D));

    // 2. Thigh mount (rthigh_g, dz=-0.77, u=0): previously had 0 bow (straight line); now gets outward (+0.12) and forward (+0.08) clearance
    reachAdaptBulge(st, 0.f, 0.16f, 0.f, -0.77f, false, bOut, overOut);
    KT_CHECK(overOut == 1.f && closeF(bOut[1], 0.08f, 1e-5f) && closeF(bOut[2], 0.12f, 1e-5f));

    // 3. Cross-draw mount (right hand reaching to left hip fx=-0.30, dz=-0.54): forward bow > 0.20, pole Y shifts forward (> 0)
    reachAdaptBulge(st, 0.f, -0.30f, 0.f, -0.54f, false, bOut, overOut);
    KT_CHECK(overOut == 1.f && bOut[1] >= 0.22f - 1e-5f && bOut[0] >= 0.05f - 1e-5f);
    KT_CHECK(reachPole3D(st, 0.f, -0.30f, 0.f, -0.54f, false, id, nullptr, 0.f, p3D));
    KT_CHECK(p3D[1] > 0.f && p3D[1] > pRef[1] + 0.4f);   // elbow stays in front of ribs instead of -0.70 behind

    // Left-hand mirror of cross-draw (left hand reaching to right hip fx=+0.30): mirrors X of pole and matches bulge
    float bOutL[3], overOutL = -1.f, p3DL[3];
    reachAdaptBulge(st, 0.f, 0.30f, 0.f, -0.54f, true, bOutL, overOutL);
    KT_CHECK(closeV(bOutL, bOut, 1e-6f) && overOutL == overOut);
    KT_CHECK(reachPole3D(st, 0.f, 0.30f, 0.f, -0.54f, true, id, nullptr, 0.f, p3DL));
    KT_CHECK(closeF(p3DL[0], -p3D[0], 1e-6f) && closeF(p3DL[1], p3D[1], 1e-6f) && closeF(p3DL[2], p3D[2], 1e-6f));

    // 4. Lumbar mount (fy=-0.30, dz=-0.52, u=0): outward bow >= 0.18 and outward elbow flare increases
    reachAdaptBulge(st, 0.f, 0.05f, -0.30f, -0.52f, false, bOut, overOut);
    KT_CHECK(overOut == 1.f && closeF(bOut[2], 0.18f, 1e-5f));
    KT_CHECK(reachPole3D(st, 0.f, 0.05f, -0.30f, -0.52f, false, id, nullptr, 0.f, p3D));
    KT_CHECK(p3D[0] > pRef[0] + 0.10f);
}

// Track 2: Topology-gated via and bidirectional sheath axis (eliminates the 177 deg barrel-down back-mount spin).
static void testReachViaTopoAndBiQuat() {
    KT_CHECK(reachViaTopoOk(ReachTopo::HingeBack) && reachViaTopoOk(ReachTopo::OverBack));
    KT_CHECK(!reachViaTopoOk(ReachTopo::Hip) && !reachViaTopoOk(ReachTopo::Thigh));

    // Upper-hemisphere and horizontal mounts (0..90 deg from +Z): bidirectional is bit-identical to unidirectional
    const float axY[3] = { 0.f, 1.f, 0.f }, fwd[3] = { 0.f, 1.f, 0.f };
    const float upAngles[3] = { 0.f, 45.f, 90.f };
    for (float aDeg : upAngles) {
        const float a = aDeg * 3.14159265358979f / 180.f;
        const float want[3] = { 0.f, -std::sin(a), std::cos(a) };
        const Q mq = qnormalize(arcQuat(axY, want));
        Q qBi, qUni;
        KT_CHECK(reachViaQuat(1u, mq, 0.f, nullptr, qBi, true));
        KT_CHECK(reachViaQuat(1u, mq, 0.f, nullptr, qUni, false));
        KT_CHECK(exactQ(qBi, qUni));
    }

    // Live barrel-down back mounts (heavy -90,-3,-90; rifle -60,4,93; melee -150,7,94):
    // Bidirectional via cuts total mount->via->hand turn by > 150 deg compared to unidirectional.
    const Pose base = { { 0.f, 0.f, 1.4f }, { 1.f, 0.f, 0.f, 0.f } };
    struct LiveBack { unsigned axis; float rot[3]; float maxBiTotal; float minUniTotal; } cases[3] = {
        { 1u, { -90.f, -3.f, -90.f }, 100.f, 250.f },   // heavy: 93 deg vs 267 deg
        { 1u, { -60.f,  4.f,  93.f }, 135.f, 300.f },   // rifle: 124 deg vs 322 deg
        { 2u, { -150.f, 7.f,  94.f }, 135.f, 310.f },   // melee: 125 deg vs 329 deg
    };
    for (const auto& c : cases) {
        Placement pl = {};
        for (int k = 0; k < 3; ++k) pl.rotDeg[k] = c.rot[k];
        const Pose mp = applyPlacement(base, pl);
        float ax[3] = { 0.f, 0.f, 0.f }, wv[3];
        ax[c.axis] = 1.f;
        qrotate(mp.q, ax, wv);
        const Q handQ = qnormalize(qmul(arcQuat(wv, fwd), mp.q));
        Q qBi, qUni;
        KT_CHECK(reachViaQuat(c.axis, mp.q, 0.f, nullptr, qBi, true));
        KT_CHECK(reachViaQuat(c.axis, mp.q, 0.f, nullptr, qUni, false));
        const float totBi = reachQAngleDeg(qmul(qBi, reachConj(mp.q))) + reachQAngleDeg(qmul(handQ, reachConj(qBi)));
        const float totUni = reachQAngleDeg(qmul(qUni, reachConj(mp.q))) + reachQAngleDeg(qmul(handQ, reachConj(qUni)));
        KT_CHECK(totBi < c.maxBiTotal && totUni > c.minUniTotal);
    }

    // Invalid inputs return false
    Q dummy = { 1.f, 0.f, 0.f, 0.f };
    KT_CHECK(!reachViaQuat(3u, dummy, 0.f, nullptr, dummy));
    KT_CHECK(!reachViaQuat(1u, Q{ kNaN, 0.f, 0.f, 0.f }, 0.f, nullptr, dummy));

    // HW49 D6: reachClearWristCont clears all 4 continuity flags (including twistOn) when the wrist block is skipped
    uint8_t wristOn = 1, twistOn = 1, combOn = 1, tgtOn = 1;
    reachClearWristCont(wristOn, twistOn, combOn, tgtOn);
    KT_CHECK(wristOn == 0 && twistOn == 0 && combOn == 0 && tgtOn == 0);
}

int main() {
    testArcQuatMapsDirections();
    testArcQuatParallelAndAntiParallel();
    testArcQuatDegenerateIsIdentity();
    testArcQuatUnnormalizedInputs();
    testQconjBasics();
    testSolveReachableLandsOnTarget();
    testSolveRandomReachableSweep();
    testSolveTargetAtHandKeepsElbow();
    testSolveTooFarClampsAndStraightens();
    testSolveTooCloseClamps();
    testSolveTargetAtShoulder();
    testSolveCollinearFallbackHint();
    testSolveZeroLengthSegmentsReturnFalse();
    testSolveNonFiniteInputsReturnFalse();
    testSolveOverflowGuard();
    testWorldDeltaIdentityParentIsDeltaTimesCur();
    testWorldDeltaRoundTrip();
    testWorldDeltaNonFiniteKeepsBone();
    testReachWeightEndpointsAndInactive();
    testReachWeightRampShape();
    testReachWeightWrapSafe();
    testBlendQExactAtZero();
    testBlendQEndsClampAndSign();
    testBlendQMidpointIsNormalizedNlerp();
    testBlendQNonFiniteInputs();
    testReachBaseFirstReadStores();
    testReachBaseCachesWhenEngineDidNotResample();
    testReachBaseRefreshesWhenEngineChanged();
    testReachBaseNonFiniteReadKeepsCache();
    testSolvePoleChoosesElbowSide();
    testSolvePoleParallelAndBadFallBack();
    testReachOverWeight();
    testReachPoleStyleAndMirror();
    testReachWeightRamp();
    testReachAnimMix();
    testReachCarryTarget();
    testReachPlanDraw();
    testReachPlanStow();
    testReachPlanPartialDraw();
    testReachRetargetContinuity();
    testReachRetargetEndsAndDuration();
    testReachGrabSpeed();
    testReachWeightHoldW();
    testReachRetargetKeepsWeight();
    testWristClampAndAngle();
    testWristDelta();
    testReachHinge();
    testReachTopology();
    testReachTopoLatch();
    testReachHingeOutParams();
    testWristHintKeepsAxis();
    testWristHintDefaultsUnchanged();
    testWristDeltaHintSweep();
    testWristTargetContinuity();
    testResidualDecay();
    testResidualStepBound();
    testPoleBiasDisabled();
    testPoleBiasBoundAndPlane();
    testPoleBiasFades();
    testPoleBiasHingeInvariant();
    testDqTail();
    testEaseSoft();
    testReachTopology3DAndOverWeight3D();
    testReachAdaptBulgeAndPole3D();
    testReachViaTopoAndBiQuat();
    return kt::Report("holster_reach");
}
