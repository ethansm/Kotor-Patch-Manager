// T3 host tests for HolsterReach.h (two-bone reach solve, arc / delta / blend quaternion helpers, reach ramp and base cache of the
// holstered-weapons mod; doc 42).
// Build/run: python3 scripts/run_tests.py holster_reach
#include "HolsterReach.h"
#include "check.h"
#include <cmath>
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
    return kt::Report("holster_reach");
}
