// Host tests for HolsterSnap.h (0.21.1-dev SNAP detector, log only). Build/run: python3 scripts/run_tests.py holster_snap
#include "HolsterSnap.h"
#include "check.h"
#include <limits>

using namespace hs;

static const float kNaN = std::numeric_limits<float>::quiet_NaN();

// A smooth path (3 mm per frame, then 8 mm per frame) never triggers, at any threshold.
static void testSmooth() {
    SnapState s;
    int snaps = 0;
    for (int i = 0; i < 200; ++i) {
        const float p[3] = {0.f, 0.f, (i < 100 ? 0.003f * i : 0.3f + 0.008f * (i - 100))};
        if (snapStep(s, p, 0.05f)) ++snaps;
    }
    KT_CHECK_EQ(snaps, 0);
}

// A smooth path then one 0.4 m jump: reported once, with the jump and the previous step; the smooth motion after it is quiet.
static void testJump() {
    SnapState s;
    int snaps = 0;
    float jumpAt = 0.f, stepAt = 0.f;
    float z = 0.f;
    for (int i = 0; i < 50; ++i) {
        z += 0.003f;
        const float p[3] = {0.f, 0.f, z};
        float j, ps;
        if (snapStep(s, p, 0.05f, &j, &ps)) { ++snaps; jumpAt = j; stepAt = ps; }
    }
    KT_CHECK_EQ(snaps, 0);
    {
        const float p[3] = {0.4f, 0.f, z};
        float j = 0.f, ps = 0.f;
        KT_CHECK(snapStep(s, p, 0.05f, &j, &ps));
        KT_CHECK(j > 0.39f && j < 0.41f);
        KT_CHECK(ps > 0.0029f && ps < 0.0031f);
    }
    for (int i = 0; i < 50; ++i) {
        z += 0.003f;
        const float p[3] = {0.4f, 0.f, z};
        if (snapStep(s, p, 0.05f)) ++snaps;
    }
    KT_CHECK_EQ(snaps, 0);
    (void)jumpAt; (void)stepAt;
}

// First and second sample after a reset cannot report, even with a big first move (a new slide / a reset to the hand).
static void testAfterReset() {
    SnapState s;
    const float a[3] = {0.f, 0.f, 0.f}, b[3] = {1.f, 0.f, 0.f}, c[3] = {2.f, 0.f, 0.f};
    KT_CHECK(!snapStep(s, a, 0.05f));
    KT_CHECK(!snapStep(s, b, 0.05f));   // second sample: only establishes a step (1 m)
    KT_CHECK(!snapStep(s, c, 0.05f));   // third: jump 1 m is not > 4 x 1 m
    snapReset(s);
    KT_CHECK_EQ(s.n, 0);
    KT_CHECK(!snapStep(s, a, 0.05f));
    KT_CHECK(!snapStep(s, b, 0.05f));
    // a still weapon (step 0) then a jump: reported
    snapReset(s);
    KT_CHECK(!snapStep(s, a, 0.05f));
    KT_CHECK(!snapStep(s, a, 0.05f));
    KT_CHECK(snapStep(s, b, 0.05f));
}

// Off (0, negative, nan): never reports, the state still follows.
static void testOff() {
    SnapState s;
    const float a[3] = {0.f, 0.f, 0.f}, b[3] = {5.f, 0.f, 0.f};
    KT_CHECK(!snapStep(s, a, 0.f));
    KT_CHECK(!snapStep(s, a, 0.f));
    KT_CHECK(!snapStep(s, b, 0.f));
    KT_CHECK_EQ(s.n, 2);
    KT_CHECK(!snapStep(s, a, -1.f));
    KT_CHECK(!snapStep(s, b, kNaN));
}

// Threshold: a 0.04 m jump from rest is under 0.05 (quiet), a 0.06 m jump reports; a jump above the threshold but inside 4x the previous step (a fast smooth sweep) is quiet.
static void testThreshold() {
    SnapState s;
    float p[3] = {0.f, 0.f, 0.f};
    snapStep(s, p, 0.05f); snapStep(s, p, 0.05f);
    p[0] = 0.04f;
    KT_CHECK(!snapStep(s, p, 0.05f));
    snapReset(s);
    p[0] = 0.f; snapStep(s, p, 0.05f); snapStep(s, p, 0.05f);
    p[0] = 0.06f;
    KT_CHECK(snapStep(s, p, 0.05f));
    // fast sweep: 0.06 m per frame steady, then 0.2 m (> 4 x 0.06 = 0.24? no: 0.2 < 0.24) quiet, 0.3 m reports
    snapReset(s);
    float x = 0.f;
    for (int i = 0; i < 10; ++i) { x += 0.06f; const float q[3] = {x, 0.f, 0.f}; KT_CHECK(!snapStep(s, q, 0.05f)); }
    x += 0.2f;
    { const float q[3] = {x, 0.f, 0.f}; KT_CHECK(!snapStep(s, q, 0.05f)); }
    snapReset(s);
    x = 0.f;
    for (int i = 0; i < 10; ++i) { x += 0.06f; const float q[3] = {x, 0.f, 0.f}; snapStep(s, q, 0.05f); }
    x += 0.3f;
    { const float q[3] = {x, 0.f, 0.f}; KT_CHECK(snapStep(s, q, 0.05f)); }
}

// A non-finite sample is ignored and does not poison the state.
static void testNonFinite() {
    SnapState s;
    const float a[3] = {0.f, 0.f, 0.f}, bad[3] = {kNaN, 0.f, 0.f}, b[3] = {1.f, 0.f, 0.f};
    snapStep(s, a, 0.05f); snapStep(s, a, 0.05f);
    KT_CHECK(!snapStep(s, bad, 0.05f));
    KT_CHECK_EQ(s.n, 2);
    KT_CHECK(snapStep(s, b, 0.05f));
}

// snap_log_m text.
static void testParse() {
    float v = -1.f;
    KT_CHECK(snapParse("0.05", v)); KT_CHECK(v == 0.05f);
    KT_CHECK(snapParse("0", v)); KT_CHECK(v == 0.f);
    KT_CHECK(snapParse("-3", v)); KT_CHECK(v == 0.f);
    KT_CHECK(snapParse("0.2", v)); KT_CHECK(v > 0.199f && v < 0.201f);
    KT_CHECK(snapParse("99", v)); KT_CHECK(v == kSnapLogMaxM);
    KT_CHECK(!snapParse("junk", v)); KT_CHECK(v == kSnapLogDefaultM);
    KT_CHECK(!snapParse("", v)); KT_CHECK(v == kSnapLogDefaultM);
    KT_CHECK(!snapParse(nullptr, v)); KT_CHECK(v == kSnapLogDefaultM);
    KT_CHECK(!snapParse("nan", v)); KT_CHECK(v == kSnapLogDefaultM);
    KT_CHECK(!snapParse("inf", v)); KT_CHECK(v == kSnapLogDefaultM);
}

int main() {
    testSmooth();
    testJump();
    testAfterReset();
    testOff();
    testThreshold();
    testNonFinite();
    testParse();
    return kt::Report("holster_snap");
}
