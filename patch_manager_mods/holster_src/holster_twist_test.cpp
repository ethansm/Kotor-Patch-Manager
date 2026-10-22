// Host tests for the HW29 swing-twist split (HolsterQuat.h qSwingTwist, HolsterReach.h reachSplitWrist). Build/run: python3 scripts/run_tests.py holster_twist
#include "HolsterQuat.h"
#include "HolsterReach.h"
#include "check.h"
#include <cmath>
#include <stdint.h>
#include <string.h>
#include <vector>

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

// ---- HW44 (q11 candidates A and B) ----
// Wrist sweep: fixed dq axis, forearm axis (0,0,1), caps 80 (twist) / 60 (swing). The raw turn runs lo..hi deg in `step` steps. dq is given w >= 0 (the sign the blend
// hands the split), and the previous twist and combined rotation are fed back as hints. guard=false is the pre-HW44 path (no combPrev).
struct WristRun { std::vector<Q> sw, comb; int overrides = 0; };
static WristRun runWristSweep(const float* axis, bool guard, float lo, float hi, float step) {
    static const float z[3] = { 0.f, 0.f, 1.f };
    WristRun r;
    Q prevTw = { 1, 0, 0, 0 }, prevComb = { 1, 0, 0, 0 };
    bool have = false;
    const int n = static_cast<int>(std::floor((hi - lo) / step + 0.5f));
    for (int i = 0; i <= n; ++i) {
        const float deg = lo + step * static_cast<float>(i);
        Q dq = qaxis(axis, deg);
        if (dq.w < 0.f) dq = { -dq.w, -dq.x, -dq.y, -dq.z };
        Q tw, sw;
        bool ov = false;
        reachSplitWrist(dq, z, 80.f, 60.f, have ? &prevTw : nullptr, tw, sw, (guard && have) ? &prevComb : nullptr, &ov);
        if (ov) ++r.overrides;
        const Q comb = qmul(sw, tw);
        r.sw.push_back(sw);
        r.comb.push_back(comb);
        prevTw = tw;
        prevComb = comb;
        have = true;
    }
    return r;
}
static float maxStepDeg(const std::vector<Q>& v) {   // largest rotation between consecutive entries (sign-agnostic)
    float m = 0.f;
    for (size_t i = 1; i < v.size(); ++i) {
        const float d = reachQAngleDeg(qmul(v[i - 1], reachConj(v[i])));
        if (d > m) m = d;
    }
    return m;
}

// Three axes; the raw turn 120 -> 260 deg (0.1 deg steps). With the guard no step moves the swing or the combined rotation by 5 deg or more. The unguarded path
// jumps on axis 0 (the bug is pinned, so the test fails if the jump ever disappears without the guard being the reason).
static void testWristContinuousSweep() {
    const float axes[3][3] = { { 0.8f, 0.f, 0.6f }, { 0.6f, 0.48f, 0.64f }, { 0.f, 0.6f, 0.8f } };
    bool ok = true;
    for (int a = 0; a < 3; ++a) {
        const WristRun g = runWristSweep(axes[a], true, 120.f, 260.f, 0.1f);
        if (!(maxStepDeg(g.sw) < 5.f) || !(maxStepDeg(g.comb) < 5.f)) ok = false;
    }
    KT_CHECK(ok);
    const WristRun u = runWristSweep(axes[0], false, 120.f, 260.f, 0.1f);
    KT_CHECK(maxStepDeg(u.sw) > 60.f);   // q11: the old swing step is 92.7 deg
    KT_CHECK(maxStepDeg(u.comb) > 30.f);
}

// Away from the flip the guard changes nothing: raw 0 -> 100 deg (the leftover stays under 180 deg) gives the same swing and combined rotation, within 1e-6 (reachTargetQ renormalises, so not bit for bit),
// and the guard never overrides.
static void testWristAwayFromFlip() {
    const float axis[3] = { 0.8f, 0.f, 0.6f };
    const WristRun g = runWristSweep(axis, true, 0.f, 100.f, 0.1f);
    const WristRun u = runWristSweep(axis, false, 0.f, 100.f, 0.1f);
    bool same = g.overrides == 0 && g.sw.size() == u.sw.size();
    for (size_t i = 0; same && i < g.sw.size(); ++i) {
        if (!qNear(g.sw[i], u.sw[i], 1e-6f) || !qNear(g.comb[i], u.comb[i], 1e-6f)) same = false;
    }
    KT_CHECK(same);
}

// reachPickContinuous: no previous rotation = candidate 0 (the shorter arc, today's path). The previous combined rotation picks the swing sign that is nearer to it;
// a previous rotation on the same side keeps candidate 0, and an unclamped leftover keeps it too.
static void testPickContinuous() {
    const float z[3] = { 0.f, 0.f, 1.f };
    const Q dqn = qaxis(z, 170.f);                       // leftover of an identity roll: shorter arc = +60 about z after the 60 deg clamp
    Q tw = { 1, 0, 0, 0 }, sw = { 1, 0, 0, 0 };
    bool ov = true;
    KT_CHECK(!reachPickContinuous(dqn, tw, sw, 60.f, nullptr) && qNear(sw, qaxis(z, 60.f), 1e-6f) && qIdentity(tw));
    const Q prevPos = qaxis(z, 55.f), prevNeg = qaxis(z, -55.f);
    tw = { 1, 0, 0, 0 };
    ov = reachPickContinuous(dqn, tw, sw, 60.f, &prevPos);
    KT_CHECK(!ov && qNear(sw, qaxis(z, 60.f), 1e-6f));
    tw = { 1, 0, 0, 0 };
    ov = reachPickContinuous(dqn, tw, sw, 60.f, &prevNeg);
    KT_CHECK(ov && qNear(sw, qaxis(z, -60.f), 1e-6f) && qIdentity(tw));
    // unclamped leftover (10 deg): both roll signs give the same combined rotation, the default holds
    const Q small = qaxis(z, 10.f);
    tw = { 1, 0, 0, 0 };
    ov = reachPickContinuous(small, tw, sw, 60.f, &small);
    KT_CHECK(!ov && qNear(sw, small, 1e-6f));
}

// HW44 candidate B: sweep the blend target's `to` through 150 -> 210 deg relative to `from` (its dot with from changes sign at 180). With the previous target fed back the
// target moves < 1 deg per 0.1 deg step at t = 0.3 / 0.5 / 0.7. The blendQ path (the old one) jumps by ~180 deg at t = 0.5 (pinned).
static void testTargetHemisphere() {
    const float y[3] = { 0.f, 1.f, 0.f };
    const Q from = { 1, 0, 0, 0 };
    const float ts[3] = { 0.3f, 0.5f, 0.7f };
    bool ok = true;
    float oldJump = 0.f;
    for (int k = 0; k < 3; ++k) {
        Q prev = { 1, 0, 0, 0 }, oprev = { 1, 0, 0, 0 };
        bool have = false;
        float m = 0.f, mo = 0.f;
        for (int i = 0; i <= 600; ++i) {
            const Q to = qaxis(y, 150.f + 0.1f * static_cast<float>(i));
            const Q tgt = reachTargetQ(from, to, ts[k], have ? &prev : nullptr);
            if (have) {
                const float d = reachQAngleDeg(qmul(prev, reachConj(tgt)));
                if (d > m) m = d;
            }
            prev = tgt;
            const Q ob = blendQ(from, to, easeSmooth(ts[k]));
            if (have) {
                const float d = reachQAngleDeg(qmul(oprev, reachConj(ob)));
                if (d > mo) mo = d;
            }
            oprev = ob;
            have = true;
        }
        if (!(m < 1.f)) ok = false;
        if (k == 1) oldJump = mo;
    }
    KT_CHECK(ok);
    KT_CHECK(oldJump > 90.f);
}

// First frame (no previous target) is blendQ(from, to, easeSmooth(t)) bit for bit.
static void testTargetFirstFrameIsBlend() {
    gSeed = 777u;
    bool ok = true;
    for (int i = 0; i < 200; ++i) {
        const Q f = randQ(), tq = randQ();
        const float t = 0.5f + 0.5f * rnd();
        if (!qSame(reachTargetQ(f, tq, t, nullptr), blendQ(f, tq, easeSmooth(t)))) ok = false;
    }
    KT_CHECK(ok);
}

// reachDiagTail appends " lw= md= pk=" only while the piece stays <= 200 bytes; NaN lw prints 0.
static void testDiagTail() {
    char buf[256];
    memset(buf, 'a', 150);
    buf[150] = '\0';
    int n = reachDiagTail(buf, sizeof buf, 150, 0.125f, 1, 1);
    KT_CHECK(n == 168 && strlen(buf) == 168 && strcmp(buf + 150, " lw=0.12 md=1 pk=1") == 0);
    char full[256];
    memset(full, 'b', 185);
    full[185] = '\0';
    n = reachDiagTail(full, sizeof full, 185, -0.5f, -1, 0);
    KT_CHECK(n == 185 && strlen(full) == 185);
    char nanBuf[256];
    memset(nanBuf, 'c', 100);
    nanBuf[100] = '\0';
    n = reachDiagTail(nanBuf, sizeof nanBuf, 100, NAN, 0, 0);
    KT_CHECK(n == 118 && strcmp(nanBuf + 100, " lw=0.00 md=0 pk=0") == 0);
}

// HW44c release (review B1): an abrupt step of the previous rotation must not pin the wrist. Previous roll -70 about the forearm axis (combined -70), static dq +70 about
// it: no cap binds (70 <= 80, swing 0), so the first call is the exact split (roll 70, combined 70 deg, pk = 0), and so is every later call.
static void testSplitWristReleasesWhenExact() {
    const float z[3] = { 0.f, 0.f, 1.f };
    const Q dq = qaxis(z, 70.f), prevComb = qaxis(z, -70.f), prevTw = qaxis(z, -70.f);
    Q tw, sw;
    bool ov = true;
    reachSplitWrist(dq, z, 80.f, 60.f, &prevTw, tw, sw, &prevComb, &ov);
    KT_CHECK(!ov && qNear(qmul(sw, tw), dq, 1e-5f) && std::fabs(reachQAngleDeg(qmul(sw, tw)) - 70.f) < 0.01f);
    Q c = prevComb;
    bool anyOv = false;
    for (int i = 0; i < 6; ++i) {
        reachSplitWrist(dq, z, 80.f, 60.f, i ? &tw : &prevTw, tw, sw, &c, &ov);
        c = qmul(sw, tw);
        anyOv = anyOv || ov;
        KT_CHECK(std::fabs(reachQAngleDeg(c) - 70.f) < 0.01f);
    }
    KT_CHECK(!anyOv);
}

int main() {
    testSwingTwistRecompose();
    testTwistKeepsAxisPoint();
    testSplitWristClamp();
    testSplitWristZeroTwist();
    testSplitWristSweep();
    testSplitWristNonFinite();
    testSwingTwistDegenerate();
    testWristContinuousSweep();
    testSplitWristReleasesWhenExact();
    testWristAwayFromFlip();
    testPickContinuous();
    testTargetHemisphere();
    testTargetFirstFrameIsBlend();
    testDiagTail();
    return kt::Report("holster_twist");
}
