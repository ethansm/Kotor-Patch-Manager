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

int main() {
    testNullViaIsResidual();
    testEndpoints();
    testContinuity();
    testDegenerateVia();
    testViaOffset();
    return kt::Report("holster_resvia");
}
