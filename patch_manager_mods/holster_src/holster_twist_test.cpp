// Host tests for the HW29 swing-twist split (HolsterQuat.h qSwingTwist, HolsterReach.h reachSplitWrist). Build/run: python3 scripts/run_tests.py holster_twist
#include "HolsterQuat.h"
#include "HolsterReach.h"
#include "check.h"
#include <cmath>
#include <stdint.h>

using namespace hs;

static uint32_t gSeed = 12345u;                    // reset to 12345u at the top of every random test
static float rnd() {                               // deterministic LCG, uniform in [-1, 1)
    gSeed = gSeed * 1664525u + 1013904223u;
    return static_cast<float>(gSeed >> 8) / 8388608.0f - 1.0f;
}
static Q randQ() { Q q = { rnd(), rnd(), rnd(), rnd() }; return qnormalize(q); }
static void randAxis(float* a) {                   // unit random axis
    a[0] = rnd();
    a[1] = rnd();
    a[2] = rnd();
    const float l = std::sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    a[0] /= l;
    a[1] /= l;
    a[2] /= l;
}
static bool qNear(const Q& a, const Q& b, float tol) {
    return std::fabs(a.w - b.w) <= tol && std::fabs(a.x - b.x) <= tol && std::fabs(a.y - b.y) <= tol && std::fabs(a.z - b.z) <= tol;
}
static bool qSame(const Q& a, const Q& b) { return a.w == b.w && a.x == b.x && a.y == b.y && a.z == b.z; }
static bool qIdentity(const Q& a) { return qSame(a, Q{ 1, 0, 0, 0 }); }

// q = swing * twist; twist is unit, w >= 0, and lies on the axis.
static void testSwingTwistRecompose() {
    gSeed = 12345u;
    bool ok = true;
    for (int i = 0; i < 200; ++i) {
        const Q q = randQ();
        float a[3];
        randAxis(a);
        Q sw, tw;
        qSwingTwist(q, a, sw, tw);
        const float len = std::sqrt(tw.w * tw.w + tw.x * tw.x + tw.y * tw.y + tw.z * tw.z);
        if (!qNear(qmul(sw, tw), q, 1e-5f) || !(tw.w >= 0.f) || std::fabs(len - 1.f) > 1e-5f) ok = false;
    }
    KT_CHECK(ok);
}

// The twist is a rotation about the axis, so the axis point it rotates is unchanged (the hand stays on the forearm line).
static void testTwistKeepsAxisPoint() {
    gSeed = 12345u;
    bool ok = true;
    for (int i = 0; i < 200; ++i) {
        const Q q = randQ();
        float a[3];
        randAxis(a);
        Q sw, tw;
        qSwingTwist(q, a, sw, tw);
        float out[3];
        qrotate(tw, a, out);
        for (int c = 0; c < 3; ++c) {
            if (std::fabs(out[c] - a[c]) > 1e-5f) ok = false;
        }
    }
    KT_CHECK(ok);
}

// Both outputs stay inside their limits. Also: a 10 deg rotation about a random axis is inside both limits (twist angle <= total angle,
// swing angle <= 2 * total), so the split recomposes it up to sign (no clamp fires).
static void testSplitWristClamp() {
    gSeed = 12345u;
    bool ok = true;
    for (int i = 0; i < 200; ++i) {
        const Q dq = randQ();
        float a[3];
        randAxis(a);
        Q tw, sw;
        reachSplitWrist(dq, a, 30.f, 40.f, nullptr, tw, sw);
        if (!(reachQAngleDeg(tw) <= 30.001f) || !(reachQAngleDeg(sw) <= 40.001f)) ok = false;
    }
    for (int i = 0; i < 200; ++i) {
        float a[3], b[3];
        randAxis(a);
        randAxis(b);
        const Q dq = qaxis(a, 10.f);
        Q tw, sw;
        reachSplitWrist(dq, b, 30.f, 40.f, nullptr, tw, sw);
        const Q re = qmul(sw, tw);
        const Q dqNeg = { -dq.w, -dq.x, -dq.y, -dq.z };
        if (!(qNear(re, dq, 1e-5f) || qNear(re, dqNeg, 1e-5f))) ok = false;
    }
    KT_CHECK(ok);
}

// twistMaxDeg <= 0 gives identity twist and the pre-HW29 swing (reachClampRot on the raw dq).
static void testSplitWristZeroTwist() {
    gSeed = 12345u;
    bool ok = true;
    for (int i = 0; i < 200; ++i) {
        const Q dq = randQ();
        float a[3];
        randAxis(a);
        const Q want = reachClampRot(dq, 120.f, nullptr);
        Q tw, sw;
        reachSplitWrist(dq, a, 0.f, 120.f, nullptr, tw, sw);
        if (!qIdentity(tw) || !qSame(sw, want)) ok = false;
        reachSplitWrist(dq, a, -5.f, 120.f, nullptr, tw, sw);
        if (!qIdentity(tw) || !qSame(sw, want)) ok = false;
    }
    KT_CHECK(ok);
}

// Pure twist sweep 170..190 deg about z with a 180 deg limit. The hint must hold the sign across the 180 deg flip.
static void testSplitWristSweep() {
    const float z[3] = { 0.f, 0.f, 1.f };
    bool ok = true;
    Q prevTw = { 1, 0, 0, 0 };
    for (int i = 0; i <= 20; ++i) {
        const float deg = 170.f + static_cast<float>(i);
        const Q dq = qaxis(z, deg);
        const Q* hint = (i == 0) ? nullptr : &prevTw;
        Q tw, sw;
        reachSplitWrist(dq, z, 180.f, 180.f, hint, tw, sw);
        if (i > 0) {
            if (!(qdot(prevTw, tw) > 0.f)) ok = false;                                                    // no sign flip
            if (!(reachQAngleDeg(qmul(prevTw, { tw.w, -tw.x, -tw.y, -tw.z })) < 2.f)) ok = false;         // 1 deg input step
        }
        if (i <= 9 && std::fabs(reachQAngleDeg(tw) - deg) > 0.01f) ok = false;                            // 170..179 tracks the input
        prevTw = tw;
    }
    // Control without a hint: 179 and 181 must land on opposite signs, so the hint is what holds the sign.
    Q c179, c181, s;
    reachSplitWrist(qaxis(z, 179.f), z, 180.f, 180.f, nullptr, c179, s);
    reachSplitWrist(qaxis(z, 181.f), z, 180.f, 180.f, nullptr, c181, s);
    if (!(qdot(c179, c181) < 0.f)) ok = false;
    KT_CHECK(ok);
}

// Non-finite dq or foreAxis: both outputs identity.
static void testSplitWristNonFinite() {
    const float z[3] = { 0.f, 0.f, 1.f };
    const float nanAxis[3] = { NAN, 0.f, 1.f };
    const Q dqNan = { NAN, 0.f, 0.f, 0.f };
    const Q dqInf = { INFINITY, 0.f, 0.f, 0.f };
    const Q dqOk = qaxis(z, 40.f);
    Q tw, sw;
    reachSplitWrist(dqNan, z, 30.f, 40.f, nullptr, tw, sw);
    bool ok = qIdentity(tw) && qIdentity(sw);
    reachSplitWrist(dqInf, z, 30.f, 40.f, nullptr, tw, sw);
    ok = ok && qIdentity(tw) && qIdentity(sw);
    reachSplitWrist(dqOk, nanAxis, 30.f, 40.f, nullptr, tw, sw);
    ok = ok && qIdentity(tw) && qIdentity(sw);
    reachSplitWrist(dqOk, nullptr, 30.f, 40.f, nullptr, tw, sw);
    ok = ok && qIdentity(tw) && qIdentity(sw);
    KT_CHECK(ok);
    // twistMax 0 is the pre-HW29 path even with a bad axis: swing == reachClampRot(dq, swingMax).
    const Q big = qaxis(z, 100.f), ref = reachClampRot(big, 60.f, nullptr);
    reachSplitWrist(big, nanAxis, 0.f, 60.f, nullptr, tw, sw);
    KT_CHECK(qIdentity(tw) && sw.w == ref.w && sw.x == ref.x && sw.y == ref.y && sw.z == ref.z);
    reachSplitWrist(big, nullptr, 0.f, 60.f, nullptr, tw, sw);
    KT_CHECK(qIdentity(tw) && sw.w == ref.w && sw.z == ref.z);
}

// Degenerate axis or projection: twist identity, swing = input (or the whole dq for a zero foreAxis).
static void testSwingTwistDegenerate() {
    const float zero[3] = { 0.f, 0.f, 0.f };
    const float z[3] = { 0.f, 0.f, 1.f };
    const float x[3] = { 1.f, 0.f, 0.f };
    const float nanAxis[3] = { NAN, 0.f, 0.f };
    const float tiny[3] = { 1e-7f, 0.f, 0.f };
    const Q q0 = { 0.5f, 0.5f, 0.5f, 0.5f };      // unit
    const Q qx = { 0, 1, 0, 0 };                  // 180 deg about x: w = 0, projection on z = 0
    bool ok = true;
    Q sw, tw;
    // (a) zero axis
    qSwingTwist(q0, zero, sw, tw);
    if (!qIdentity(tw) || !qSame(sw, q0)) ok = false;
    // (b) projection and w both zero
    qSwingTwist(qx, z, sw, tw);
    if (!qIdentity(tw) || !qSame(sw, qx)) ok = false;
    // (c) NaN axis
    qSwingTwist(q0, nanAxis, sw, tw);
    if (!qIdentity(tw) || !qSame(sw, q0)) ok = false;
    // (d) zero foreAxis through reachSplitWrist: the whole dq goes to swing
    const Q dq = qaxis(x, 40.f);
    reachSplitWrist(dq, zero, 80.f, 60.f, nullptr, tw, sw);
    if (!qIdentity(tw) || !qNear(sw, reachClampRot(dq, 60.f, nullptr), 1e-6f)) ok = false;
    // (e) tiny axis (length 1e-7, above the 1e-12 cutoff) splits the same as the unit axis
    Q tu, su, tt, st;
    qSwingTwist(q0, x, su, tu);
    qSwingTwist(q0, tiny, st, tt);
    if (!qNear(tu, tt, 1e-5f) || !qNear(su, st, 1e-5f)) ok = false;
    KT_CHECK(ok);
}

int main() {
    testSwingTwistRecompose();
    testTwistKeepsAxisPoint();
    testSplitWristClamp();
    testSplitWristZeroTwist();
    testSplitWristSweep();
    testSplitWristNonFinite();
    testSwingTwistDegenerate();
    return kt::Report("holster_twist");
}
