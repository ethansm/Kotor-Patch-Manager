// Host tests for the HW31b via residual (HolsterReach.h reachResidualVia, reachViaOffset; doc 42c 4.14). Build/run: python3 scripts/run_tests.py holster_resvia
#include "HolsterQuat.h"
#include "HolsterReach.h"
#include "check.h"
#include <cmath>
#include <cstring>
#include <stdint.h>

using namespace hs;

static uint32_t gSeed = 12345u;                    // reset at the top of every random test
static float rnd() {                               // deterministic LCG, uniform in [-1, 1)
    gSeed = gSeed * 1664525u + 1013904223u;
    return static_cast<float>(gSeed >> 8) / 8388608.0f - 1.0f;
}
static Q randQ() { Q q = { rnd(), rnd(), rnd(), rnd() }; return qnormalize(q); }
static const Q kIdent = { 1.f, 0.f, 0.f, 0.f };
static Q negQ(const Q& q) { return Q{ -q.w, -q.x, -q.y, -q.z }; }
static bool qNear(const Q& a, const Q& b, float tol) {
    return std::fabs(a.w - b.w) <= tol && std::fabs(a.x - b.x) <= tol && std::fabs(a.y - b.y) <= tol && std::fabs(a.z - b.z) <= tol;
}
static bool qNearSign(const Q& a, const Q& b, float tol) { return qNear(a, b, tol) || qNear(a, negQ(b), tol); }   // same rotation
static bool qBitSame(const Q& a, const Q& b) { return std::memcmp(&a, &b, sizeof(Q)) == 0; }
static float qnorm(const Q& q) { return std::sqrt(qdot(q, q)); }
static float angleDeg(const Q& a, const Q& b) {    // rotation angle between two unit quats
    float d = std::fabs(qdot(a, b));
    if (d > 1.f) d = 1.f;
    return 2.f * static_cast<float>(std::acos(d)) * 57.2957795f;
}

// Steps t by 1 ms over [0, 240] (ms = 240): every consecutive pair is < 6 deg apart and keeps qdot > 0 (no sign flip).
static bool sweepContinuous(const Q& R, const Q& V) {
    Q prev = reachResidualVia(R, &V, 0.f, 240.f);
    for (int t = 1; t <= 240; ++t) {
        const Q cur = reachResidualVia(R, &V, static_cast<float>(t), 240.f);
        if (!(qdot(prev, cur) > 0.f) || !(angleDeg(prev, cur) < 6.f)) return false;
        prev = cur;
    }
    return true;
}

// (a) Null or non-finite via: bit for bit reachResidual, for any Rres, t in [-10, ms + 10] and ms in {0, 240, NaN}.
static void testNullViaIsResidual() {
    gSeed = 11111u;
    const float msVals[3] = { 0.f, 240.f, NAN };
    const Q nanVia = { NAN, 0.f, 0.f, 0.f };
    const Q infVia = { 1.f, INFINITY, 0.f, 0.f };
    bool okNull = true, okNonFinite = true;
    for (int i = 0; i < 200; ++i) {
        const Q R = randQ();
        for (int m = 0; m < 3; ++m) {
            const float ms = msVals[m];
            const float hi = std::isfinite(ms) ? ms + 10.f : 250.f;   // NaN ms: t anywhere in [-10, 250)
            for (int k = 0; k < 4; ++k) {
                const float t = -10.f + (rnd() + 1.f) * 0.5f * (hi + 10.f);
                const Q ref = reachResidual(R, t, ms);
                if (!qBitSame(reachResidualVia(R, nullptr, t, ms), ref)) okNull = false;
                if (!qBitSame(reachResidualVia(R, &nanVia, t, ms), ref)) okNonFinite = false;
                if (!qBitSame(reachResidualVia(R, &infVia, t, ms), ref)) okNonFinite = false;
            }
        }
    }
    KT_CHECK(okNull);
    KT_CHECK(okNonFinite);
}

// (b) Endpoints: t = 0 -> Rres, t = ms -> identity, s = 0.5 (t = ms / 2) -> viaH. All up to sign (same rotation).
static void testEndpoints() {
    gSeed = 22222u;
    bool okStart = true, okEnd = true, okMid = true;
    for (int i = 0; i < 200; ++i) {
        const Q R = randQ();
        const Q V = randQ();
        if (!qNearSign(reachResidualVia(R, &V, 0.f, 240.f), R, 1e-5f)) okStart = false;
        if (!qNearSign(reachResidualVia(R, &V, -5.f, 240.f), R, 1e-5f)) okStart = false;
        if (!qNearSign(reachResidualVia(R, &V, 240.f, 240.f), kIdent, 1e-5f)) okEnd = false;
        if (!qNearSign(reachResidualVia(R, &V, 300.f, 240.f), kIdent, 1e-5f)) okEnd = false;
        if (!qNearSign(reachResidualVia(R, &V, 120.f, 240.f), V, 1e-5f)) okMid = false;
    }
    KT_CHECK(okStart);
    KT_CHECK(okEnd);
    KT_CHECK(okMid);
}

// (c) Continuity over 200 random (Rres, viaH), plus the antipodal via (viaH = -Rres) and a via 179 deg from Rres (both signs).
static void testContinuity() {
    gSeed = 33333u;
    bool okRandom = true, okAnti = true, okFlip179 = true;
    for (int i = 0; i < 200; ++i) {
        const Q R = randQ();
        const Q V = randQ();
        if (!sweepContinuous(R, V)) okRandom = false;
        if (!sweepContinuous(R, negQ(R))) okAnti = false;
        float axis[3] = { rnd(), rnd(), rnd() };
        const Q V179 = qmul(R, qaxis(axis, 179.f));
        if (std::fabs(angleDeg(R, V179) - 179.f) > 0.05f) okFlip179 = false;   // the setup really is 179 deg away
        if (!sweepContinuous(R, V179) || !sweepContinuous(R, negQ(V179))) okFlip179 = false;
    }
    KT_CHECK(okRandom);
    KT_CHECK(okAnti);
    KT_CHECK(okFlip179);
}

// (d) Degenerate via == Rres or -Rres: finite, unit, endpoint rules and continuity hold. Non-finite Rres falls back to reachResidual.
static void testDegenerateVia() {
    gSeed = 44444u;
    const Q nanR = { NAN, 0.f, 0.f, 0.f };
    bool okUnit = true, okEnds = true, okCont = true, okNanR = true;
    for (int i = 0; i < 200; ++i) {
        const Q R = randQ();
        const Q vs[2] = { R, negQ(R) };
        for (int v = 0; v < 2; ++v) {
            for (int k = 0; k <= 240; ++k) {
                const Q out = reachResidualVia(R, &vs[v], static_cast<float>(k), 240.f);
                if (!qfinite(out) || std::fabs(qnorm(out) - 1.f) > 1e-5f) okUnit = false;
            }
            if (!qNearSign(reachResidualVia(R, &vs[v], 0.f, 240.f), R, 1e-5f)) okEnds = false;
            if (!qNearSign(reachResidualVia(R, &vs[v], 240.f, 240.f), kIdent, 1e-5f)) okEnds = false;
            if (!qNearSign(reachResidualVia(R, &vs[v], 120.f, 240.f), vs[v], 1e-5f)) okEnds = false;
            if (!sweepContinuous(R, vs[v])) okCont = false;
        }
        if (!qBitSame(reachResidualVia(nanR, &vs[0], 100.f, 240.f), reachResidual(nanR, 100.f, 240.f))) okNanR = false;
    }
    KT_CHECK(okUnit);
    KT_CHECK(okEnds);
    KT_CHECK(okCont);
    KT_CHECK(okNanR);
}

// (e) reachViaOffset: 0 at t = 0 and t = ms, outM at t = ms / 2, never above outM, 0 for bad ms or outM.
static void testViaOffset() {
    gSeed = 55555u;
    bool okEnds = true, okMid = true, okBound = true, okBad = true;
    for (int i = 0; i < 200; ++i) {
        const float outM = 0.5f * (rnd() + 1.f);                                  // [0, 1)
        if (reachViaOffset(0.f, 240.f, outM) != 0.f || reachViaOffset(240.f, 240.f, outM) != 0.f) okEnds = false;
        if (std::fabs(reachViaOffset(120.f, 240.f, outM) - outM) > 1e-6f) okMid = false;
        for (int k = 0; k <= 240; ++k) {
            const float v = reachViaOffset(static_cast<float>(k), 240.f, outM);
            if (!(v >= 0.f) || v > outM) okBound = false;
        }
        if (reachViaOffset(120.f, 0.f, outM) != 0.f || reachViaOffset(120.f, NAN, outM) != 0.f || reachViaOffset(120.f, -240.f, outM) != 0.f) okBad = false;
        if (reachViaOffset(120.f, 240.f, 0.f) != 0.f || reachViaOffset(120.f, 240.f, -outM - 0.01f) != 0.f) okBad = false;
        if (reachViaOffset(120.f, 240.f, NAN) != 0.f || reachViaOffset(120.f, 240.f, INFINITY) != 0.f) okBad = false;
    }
    if (std::fabs(reachViaOffset(120.f, 240.f, kReachViaOutM) - kReachViaOutM) > 1e-6f) okMid = false;
    KT_CHECK(okEnds);
    KT_CHECK(okMid);
    KT_CHECK(okBound);
    KT_CHECK(okBad);
}

// (f) HW50 reach_via_tilt_deg_<cat>: reachViaUp. Tilt 0, NaN or negative = {0,0,1} bit for bit, with or without a body frame; 120 clamps to 90.
static bool bitsEq3(const float* a, const float* b) { return std::memcmp(a, b, 3 * sizeof(float)) == 0; }
// Whole turn (deg) mount -> via -> hand, with the full rotation angle of each leg (roll included), for a mount whose long axis (x, index 0)
// points aDeg from up, tilted toward -y (back, the live tilt per the HW50 README b2). The via is built as reachViaFor does (up = reachViaUp,
// tilted toward +y); the hand has the long axis forward {0, 1, 0}. direct = mount -> hand.
struct ViaPath { float legA, legB, direct; float total() const { return legA + legB; } };
static ViaPath viaPathDeg(float aDeg, float tiltDeg) {
    const float ax[3] = { 1.f, 0.f, 0.f }, fwd[3] = { 0.f, 1.f, 0.f };
    const float a = aDeg * 3.14159265358979f / 180.f;
    const float want[3] = { 0.f, -std::sin(a), std::cos(a) };
    const Q mq = qnormalize(arcQuat(ax, want));
    float wv[3], up[3];
    qrotate(mq, ax, wv);
    reachViaUp(tiltDeg, nullptr, up);
    const Q via = qnormalize(qmul(arcQuat(wv, up), mq));
    const Q hand = qnormalize(qmul(arcQuat(wv, fwd), mq));
    return { reachQAngleDeg(qmul(via, reachConj(mq))), reachQAngleDeg(qmul(hand, reachConj(via))), reachQAngleDeg(qmul(hand, reachConj(mq))) };
}
static void testViaTilt() {
    const float kUp[3] = { 0.f, 0.f, 1.f };
    const Q bodyQ = qnormalize(Q{ 0.6f, 0.3f, -0.5f, 0.55f });   // a body frame that is not identity
    const Q nanQ = { NAN, 0.f, 0.f, 0.f };
    const float zeroTilts[4] = { 0.f, NAN, -20.f, -0.f };
    bool okZero = true, okClamp = true, okNonFinite = true, okRot = true;
    float v[3], ref[3], rot[3];
    for (float t : zeroTilts) {
        reachViaUp(t, nullptr, v);
        if (!bitsEq3(v, kUp)) okZero = false;
        reachViaUp(t, &bodyQ, v);
        if (!bitsEq3(v, kUp)) okZero = false;
    }
    reachViaUp(120.f, nullptr, v);
    reachViaUp(90.f, nullptr, ref);
    if (!bitsEq3(v, ref) || std::fabs(v[1] - 1.f) > 1e-6f || std::fabs(v[2]) > 1e-6f) okClamp = false;
    if (reachViaTiltClamp(120.f) != 90.f || reachViaTiltClamp(NAN) != 0.f || reachViaTiltClamp(-5.f) != 0.f || reachViaTiltClamp(45.f) != 45.f) okClamp = false;
    reachViaUp(45.f, nullptr, ref);
    reachViaUp(45.f, &nanQ, v);   // non-finite body frame: unrotated, bit for bit
    if (!bitsEq3(v, ref)) okNonFinite = false;
    qrotate(qnormalize(bodyQ), ref, rot);
    reachViaUp(45.f, &bodyQ, v);  // finite body frame: the unrotated tilt turned by the body quat
    if (!(std::fabs(v[0] - rot[0]) <= 1e-6f && std::fabs(v[1] - rot[1]) <= 1e-6f && std::fabs(v[2] - rot[2]) <= 1e-6f)) okRot = false;
    KT_CHECK(okZero);
    KT_CHECK(okClamp);
    KT_CHECK(okNonFinite);
    KT_CHECK(okRot);

    // Whole via path (mount -> via -> hand), tilt 0 / 45 / 90. Measured: 177 deg mount (the realistic rifle back mount, near straight down)
    // 267 / 183 / 93; 90 deg mount 270 / 315 / 180 (tilt 45 is LONGER than 0 because the roll legs add, so only 90 < 0 is pinned); 0 deg mount 90 x3.
    const float tilts[3] = { 0.f, 45.f, 90.f };
    float p0[3], p90[3], p177[3];
    for (int k = 0; k < 3; ++k) { p0[k] = viaPathDeg(0.f, tilts[k]).total(); p90[k] = viaPathDeg(90.f, tilts[k]).total(); p177[k] = viaPathDeg(177.f, tilts[k]).total(); }
    KT_CHECK(viaPathDeg(0.f, 0.f).legA < 1e-3f);   // a mount already pointing up needs no via turn at tilt 0
    KT_CHECK(p177[0] > p177[1] + 1.f && p177[1] > p177[2] + 1.f);   // (i) strictly decreasing 0 -> 45 -> 90
    KT_CHECK(std::fabs(p177[0] - 267.f) < 1.f && std::fabs(p177[1] - 183.f) < 1.f && std::fabs(p177[2] - 93.f) < 1.f);
    KT_CHECK(std::fabs(p0[1] - p0[0]) < 0.5f && std::fabs(p0[2] - p0[0]) < 0.5f);   // (ii) constant
    KT_CHECK(std::fabs(p0[0] - 90.f) < 1.f);
    KT_CHECK(p90[2] < p90[0] - 1.f);   // (iii) 90 < 0
    KT_CHECK(std::fabs(p90[0] - 270.f) < 1.f && std::fabs(p90[1] - 315.f) < 1.f && std::fabs(p90[2] - 180.f) < 1.f);
    // At tilt 0 the whole path is never shorter than the direct mount -> hand angle (triangle inequality on SO(3)).
    const float mounts[3] = { 0.f, 90.f, 177.f };
    bool okDirect = true;
    for (float m : mounts) { const ViaPath vp = viaPathDeg(m, 0.f); if (vp.total() + 1e-2f < vp.direct) okDirect = false; }
    KT_CHECK(okDirect);
}

int main() {
    testNullViaIsResidual();
    testEndpoints();
    testContinuity();
    testDegenerateVia();
    testViaOffset();
    testViaTilt();
    return kt::Report("holster_resvia");
}
