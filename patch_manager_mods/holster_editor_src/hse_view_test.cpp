// T1 host tests for HseView.h (holster editor 3D preview viewport math and strip layout helpers; doc 20).
// Build/run: python3 scripts/run_tests.py hse_view
#include "HseView.h"
#include "check.h"
#include <cmath>
#include <string.h>

static bool closeF(float a, float b, float eps = 1e-4f) {
    return fabsf(a - b) <= eps;
}

static bool sameRect(hse::HseRect a, hse::HseRect b) {
    return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
}

static bool sameView(hse::View a, hse::View b, float eps = 1e-4f) {
    return closeF(a.yaw, b.yaw, eps) && closeF(a.zoom, b.zoom, eps) && closeF(a.panZ, b.panZ, eps);
}

static void testYawAndSpin() {
    KT_CHECK(closeF(hse::wrapYaw(-10.0f), 350.0f));
    KT_CHECK(closeF(hse::wrapYaw(370.0f), 10.0f));
    KT_CHECK(closeF(hse::wrapYaw(360.0f), 0.0f));
    KT_CHECK(closeF(hse::wrapYaw(std::nanf("")), 0.0f));
    KT_CHECK(closeF(hse::wrapYaw(INFINITY), 0.0f));

    KT_CHECK(closeF(hse::spin(180.0f, 20.0f), 190.0f));
    KT_CHECK(closeF(hse::spin(359.0f, 4.0f), 1.0f));
    KT_CHECK(closeF(hse::spin(0.0f, -2.0f), 359.0f));

    KT_CHECK(closeF(hse::flipYaw(180.0f), 0.0f));
    KT_CHECK(closeF(hse::flipYaw(0.0f), 180.0f));
    KT_CHECK(closeF(hse::flipYaw(350.0f), 170.0f));
}

static void testZoomAndView() {
    KT_CHECK(closeF(hse::clampZoom(0.5f), 0.8f));
    KT_CHECK(closeF(hse::clampZoom(9.0f), 4.0f));
    KT_CHECK(closeF(hse::zoomIn(3.5f), 4.0f));
    KT_CHECK(closeF(hse::zoomOut(0.9f), 0.8f));
    KT_CHECK(closeF(hse::clampZoom(std::nanf("")), 2.2f));
    KT_CHECK(closeF(hse::clampZoom(INFINITY), 4.0f));
    KT_CHECK(closeF(hse::clampZoom(-INFINITY), 0.8f));
    KT_CHECK(closeF(hse::makeView(0.0f, 2.2f, INFINITY).panZ, 1.0f));
    KT_CHECK(closeF(hse::makeView(0.0f, 2.2f, -INFINITY).panZ, -1.0f));
    KT_CHECK(closeF(hse::panStep(INFINITY, 1), 1.0f));

    KT_CHECK(sameView(hse::makeView(180.0f, 2.2f, 0.0f), hse::View{180.0f, 2.2f, 0.0f}));
    KT_CHECK(sameView(hse::makeView(-90.0f, 9.0f, 5.0f), hse::View{270.0f, 4.0f, 1.0f}));

    KT_CHECK(closeF(hse::panStep(0.0f, 1), 0.1f));
    KT_CHECK(closeF(hse::panStep(0.95f, 1), 1.0f));
    KT_CHECK(closeF(hse::panStep(-0.95f, -1), -1.0f));
    KT_CHECK(closeF(hse::panStep(std::nanf(""), 1), 0.0f));
}

static void testIndex() {
    KT_CHECK_EQ(hse::wrapIndex(0, -1, 3), 2);
    KT_CHECK_EQ(hse::wrapIndex(2, 1, 3), 0);
    KT_CHECK_EQ(hse::wrapIndex(1, 1, 0), 0);
}

static void testRectHelpers() {
    hse::HseRect col{161, 134, 236, 284};
    hse::HseRect view{330, 134, 160, 284};
    KT_CHECK(sameRect(hse::leftStrip(col, view, 6), hse::HseRect{161, 134, 163, 284}));

    hse::HseRect viewBeforeCol{100, 134, 160, 284};
    KT_CHECK_EQ(hse::leftStrip(col, viewBeforeCol, 6).w, 0);

    hse::HseRect strip{161, 134, 163, 284};
    KT_CHECK(sameRect(hse::stripSlot(strip, 0), hse::HseRect{161, 134, 163, 32}));
    KT_CHECK_EQ(hse::stripSlot(strip, 1).y, 170);
    KT_CHECK_EQ(hse::stripSlot(strip, 4).y, 278);
    KT_CHECK_EQ(hse::stripSlot(strip, 7).y, 386);

    hse::HseRect slot0 = hse::stripSlot(strip, 0);
    KT_CHECK(sameRect(hse::half(slot0, 0, 6), hse::HseRect{161, 134, 78, 32}));
    KT_CHECK_EQ(hse::half(slot0, 1, 6).x, 245);
}

static void testMemberLabel() {
    char buf[32];
    hse::memberLabel(buf, 32, 0, "Bastila");
    KT_CHECK_EQ(buf, "Bastila");

    hse::memberLabel(buf, 32, 2, "");
    KT_CHECK_EQ(buf, "Member 3");

    char smallBuf[4];
    hse::memberLabel(smallBuf, 4, 0, "Bastila");
    KT_CHECK_EQ(smallBuf, "Bas");
    KT_CHECK(smallBuf[3] == '\0');

    hse::memberLabel(buf, 32, 4, nullptr);
    KT_CHECK_EQ(buf, "Member 5");

    // cap <= 0 and null buffer safety
    hse::memberLabel(buf, 0, 0, "Bastila");
    hse::memberLabel(nullptr, 32, 0, "Bastila");
}

static void testScopeLabel() {
    KT_CHECK_EQ(hse::scopeLabel(0), "Scope: Weapon");
    KT_CHECK_EQ(hse::scopeLabel(1), "Scope: Category");
    KT_CHECK_EQ(hse::scopeLabel(2), "Scope: Body");
    KT_CHECK_EQ(hse::scopeLabel(99), "Scope: Weapon");
}

static void testSnapAccum() {
    const float e = 1e-5f;
    float acc = 0.0f;
    KT_CHECK(closeF(hse::snapAccum(acc, 0.004f, 0.01f), 0.0f, e));
    KT_CHECK(closeF(hse::snapAccum(acc, 0.004f, 0.01f), 0.0f, e));
    KT_CHECK(closeF(hse::snapAccum(acc, 0.004f, 0.01f), 0.01f, e));
    KT_CHECK(closeF(acc, 0.002f, e));

    acc = 0.0f;
    KT_CHECK(closeF(hse::snapAccum(acc, 0.037f, 0.01f), 0.03f, e));
    KT_CHECK(closeF(acc, 0.007f, e));
    KT_CHECK(closeF(hse::snapAccum(acc, -0.123f, 0.01f), -0.11f, e));
    KT_CHECK(closeF(acc, -0.006f, e));

    acc = 0.005f;
    KT_CHECK(closeF(hse::snapAccum(acc, -0.007f, 0.01f), 0.0f, e));
    KT_CHECK(closeF(acc, -0.002f, e));
    KT_CHECK(closeF(hse::snapAccum(acc, -0.009f, 0.01f), -0.01f, e));

    acc = 0.0f;
    KT_CHECK(closeF(hse::snapAccum(acc, -0.025f, 0.01f), -0.02f, e));
    KT_CHECK(closeF(acc, -0.005f, e));

    // invariant: sum(applied) + acc == sum(delta)
    acc = 0.0f;
    float sumApplied = 0.0f, sumDelta = 0.0f;
    const float deltas[] = {0.013f, -0.007f, 0.021f, -0.044f, 0.003f};
    for (float d : deltas) {
        sumApplied += hse::snapAccum(acc, d, 0.01f);
        sumDelta += d;
    }
    KT_CHECK(closeF(sumApplied + acc, sumDelta, e));
}

static void testGuard() {
    KT_CHECK(hse::guardProceed(false, false, 0, 5000, 2000));
    KT_CHECK(!hse::guardProceed(true, false, 0, 5000, 2000));
    KT_CHECK(hse::guardProceed(true, true, 1000, 2500, 2000));
    KT_CHECK(!hse::guardProceed(true, true, 1000, 3000, 2000));
    KT_CHECK(hse::guardProceed(true, true, 0xFFFFFF00u, 0x40u, 2000));
    KT_CHECK(!hse::guardArmLive(false, 1000, 1100, 2000));
    KT_CHECK(hse::guardArmLive(true, 1000, 1100, 2000));
}

int main() {
    testYawAndSpin();
    testZoomAndView();
    testIndex();
    testRectHelpers();
    testMemberLabel();
    testScopeLabel();
    testSnapAccum();
    testGuard();
    return kt::Report("hse_view");
}

