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

static void testWorldToScreen() {
    const float cam[3] = {0.0f, 0.0f, 0.0f};
    const float qI[4] = {1.0f, 0.0f, 0.0f, 0.0f};   // w,x,y,z identity: looks down -Z, up +Y, right +X
    const hse::HseRect ext = {100, 50, 200, 100};   // aspect 2
    const float fov = 60.0f;
    const float t = tanf(fov * 3.14159265358979f / 360.0f);

    // straight ahead -> centre
    const float ahead[3] = {0.0f, 0.0f, -3.0f};
    hse::WorldScreen s = hse::worldToScreen(cam, qI, fov, ext, ahead);
    KT_CHECK(s.ok);
    KT_CHECK(closeF(s.sx, 200.0f, 1e-2f));
    KT_CHECK(closeF(s.sy, 100.0f, 1e-2f));
    KT_CHECK(closeF(s.depth, 3.0f, 1e-4f));

    // top frustum edge -> sy == ext.y; bottom edge -> ext.y + h
    const float top[3] = {0.0f, 2.0f * t, -2.0f};
    s = hse::worldToScreen(cam, qI, fov, ext, top);
    KT_CHECK(s.ok);
    KT_CHECK(closeF(s.sy, 50.0f, 1e-2f));
    KT_CHECK(closeF(s.sx, 200.0f, 1e-2f));
    const float bottom[3] = {0.0f, -2.0f * t, -2.0f};
    s = hse::worldToScreen(cam, qI, fov, ext, bottom);
    KT_CHECK(closeF(s.sy, 150.0f, 1e-2f));

    // right edge -> sx == ext.x + w (the horizontal half extent is depth * t * aspect); left edge -> ext.x
    const float right[3] = {2.0f * t * 2.0f, 0.0f, -2.0f};
    s = hse::worldToScreen(cam, qI, fov, ext, right);
    KT_CHECK(s.ok);
    KT_CHECK(closeF(s.sx, 300.0f, 1e-2f));
    KT_CHECK(closeF(s.sy, 100.0f, 1e-2f));
    const float left[3] = {-2.0f * t * 2.0f, 0.0f, -2.0f};
    s = hse::worldToScreen(cam, qI, fov, ext, left);
    KT_CHECK(closeF(s.sx, 100.0f, 1e-2f));

    // +Y is up on screen (smaller sy), +X right (larger sx)
    const float up[3] = {0.0f, 0.5f, -3.0f};
    s = hse::worldToScreen(cam, qI, fov, ext, up);
    KT_CHECK(s.ok && s.sy < 100.0f);
    const float rt[3] = {0.5f, 0.0f, -3.0f};
    s = hse::worldToScreen(cam, qI, fov, ext, rt);
    KT_CHECK(s.ok && s.sx > 200.0f);

    // behind the camera and depth 0.05 boundary
    const float behind[3] = {0.0f, 0.0f, 3.0f};
    KT_CHECK(!hse::worldToScreen(cam, qI, fov, ext, behind).ok);
    const float atZero[3] = {0.0f, 0.0f, 0.0f};
    KT_CHECK(!hse::worldToScreen(cam, qI, fov, ext, atZero).ok);
    const float atBoundary[3] = {0.0f, 0.0f, -0.05f};
    KT_CHECK(!hse::worldToScreen(cam, qI, fov, ext, atBoundary).ok);
    const float justPast[3] = {0.0f, 0.0f, -0.0502f};
    KT_CHECK(hse::worldToScreen(cam, qI, fov, ext, justPast).ok);

    // 90-degree yawed camera (about +Y): looks down -X; a point on -X is the centre, a point on -Z is off to the side
    const float h = 0.70710678f;
    const float qY[4] = {h, 0.0f, h, 0.0f};
    const float alongX[3] = {-4.0f, 0.0f, 0.0f};
    s = hse::worldToScreen(cam, qY, fov, ext, alongX);
    KT_CHECK(s.ok);
    KT_CHECK(closeF(s.sx, 200.0f, 1e-2f));
    KT_CHECK(closeF(s.sy, 100.0f, 1e-2f));
    KT_CHECK(closeF(s.depth, 4.0f, 1e-3f));
    KT_CHECK(!hse::worldToScreen(cam, qY, fov, ext, ahead).ok);   // -Z is now 90 degrees to the side: depth 0
    KT_CHECK(!hse::worldToScreen(cam, qY, fov, ext, behind).ok);

    // camera offset: translating both camera and point changes nothing
    const float cam2[3] = {5.0f, -2.0f, 1.0f};
    const float ahead2[3] = {5.0f, -2.0f, -2.0f};
    s = hse::worldToScreen(cam2, qI, fov, ext, ahead2);
    KT_CHECK(s.ok && closeF(s.sx, 200.0f, 1e-2f) && closeF(s.sy, 100.0f, 1e-2f) && closeF(s.depth, 3.0f, 1e-4f));

    // non-finite input and degenerate extent / fov / quaternion
    const float nanP[3] = {std::nanf(""), 0.0f, -3.0f};
    KT_CHECK(!hse::worldToScreen(cam, qI, fov, ext, nanP).ok);
    const float infP[3] = {0.0f, INFINITY, -3.0f};
    KT_CHECK(!hse::worldToScreen(cam, qI, fov, ext, infP).ok);
    const float nanCam[3] = {0.0f, std::nanf(""), 0.0f};
    KT_CHECK(!hse::worldToScreen(nanCam, qI, fov, ext, ahead).ok);
    const float nanQ[4] = {1.0f, std::nanf(""), 0.0f, 0.0f};
    KT_CHECK(!hse::worldToScreen(cam, nanQ, fov, ext, ahead).ok);
    const float zeroQ[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    KT_CHECK(!hse::worldToScreen(cam, zeroQ, fov, ext, ahead).ok);
    KT_CHECK(!hse::worldToScreen(cam, qI, std::nanf(""), ext, ahead).ok);
    KT_CHECK(!hse::worldToScreen(cam, qI, 0.0f, ext, ahead).ok);
    KT_CHECK(!hse::worldToScreen(cam, qI, fov, hse::HseRect{100, 50, 0, 100}, ahead).ok);
    KT_CHECK(!hse::worldToScreen(cam, qI, fov, hse::HseRect{100, 50, 200, 0}, ahead).ok);
    KT_CHECK(!hse::worldToScreen(cam, qI, fov, hse::HseRect{100, 50, -5, -5}, ahead).ok);
    KT_CHECK(!hse::worldToScreen(nullptr, qI, fov, ext, ahead).ok);

    // a non-unit quaternion is normalised
    const float qBig[4] = {3.0f, 0.0f, 0.0f, 0.0f};
    s = hse::worldToScreen(cam, qBig, fov, ext, ahead);
    KT_CHECK(s.ok && closeF(s.sx, 200.0f, 1e-2f));
}

static void testSnapAccumRotation() {
    // RMB Ctrl snap: 5-degree steps with carry (the per-tick round() it replaces dropped slow rotation)
    const float e = 1e-4f;
    float acc = 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < 20; ++i) sum += hse::snapAccum(acc, 0.5f, 5.0f);   // 10 deg of slow drag
    KT_CHECK(closeF(sum, 10.0f, e));
    KT_CHECK(closeF(acc, 0.0f, e));
    acc = 0.0f;
    KT_CHECK(closeF(hse::snapAccum(acc, 2.4f, 5.0f), 0.0f, e));   // under one step: nothing yet
    KT_CHECK(closeF(acc, 2.4f, e));
    KT_CHECK(closeF(hse::snapAccum(acc, 2.7f, 5.0f), 5.0f, e));   // carry completes the step
    KT_CHECK(closeF(acc, 0.1f, e));
}

static void testCalibScript() {
    hse::CalibScript s;
    KT_CHECK(s.step == 0 && s.round == 1 && s.clicks == 0);
    KT_CHECK(!strcmp(s.current(), "head centre"));
    KT_CHECK(!strcmp(hse::CalibScript::targetName(1), "belt"));
    KT_CHECK(!strcmp(hse::CalibScript::targetName(2), "right hand"));
    KT_CHECK(!strcmp(hse::CalibScript::targetName(3), "left hand"));
    KT_CHECK(!strcmp(hse::CalibScript::targetName(4), "right foot"));
    KT_CHECK(!strcmp(hse::CalibScript::targetName(5), "left foot"));
    KT_CHECK(!strcmp(hse::CalibScript::targetName(-1), ""));
    KT_CHECK(!strcmp(hse::CalibScript::targetName(6), ""));
    for (int i = 0; i < 5; ++i) KT_CHECK(!s.advance());   // 5 steps: no wrap yet
    KT_CHECK(s.step == 5 && s.round == 1 && !strcmp(s.current(), "left foot"));
    KT_CHECK(s.advance());                                 // the 6th wraps and bumps the round
    KT_CHECK(s.step == 0 && s.round == 2 && s.clicks == 6);
    for (int i = 0; i < 12; ++i) s.advance();
    KT_CHECK(s.step == 0 && s.round == 4 && s.clicks == 18);

    hse::CalibScript bad;   // out-of-range step recovers on advance
    bad.step = 99;
    KT_CHECK(!strcmp(bad.current(), ""));
    KT_CHECK(bad.advance() && bad.step == 0 && bad.round == 2);

    char buf[64];
    hse::CalibScript t;
    t.step = 2;
    hse::calibPrompt(buf, sizeof buf, &t);
    KT_CHECK(!strcmp(buf, "Calib 3/6 (round 1): click the right hand"));
    char tiny[8];
    hse::calibPrompt(tiny, sizeof tiny, &t);
    KT_CHECK(strlen(tiny) == 7 && !strncmp(tiny, "Calib 3", 7));
    hse::calibPrompt(buf, sizeof buf, nullptr);
    KT_CHECK(buf[0] == '\0');
    hse::calibPrompt(nullptr, 10, &t);       // must not crash
    hse::calibPrompt(buf, 0, &t);            // cap 0: untouched
}

static void testStripNameTag() {
    char o[64];
    KT_CHECK(hse::stripNameTag("{06}Vibrosword", o, sizeof o) && !strcmp(o, "Vibrosword"));
    KT_CHECK(hse::stripNameTag("{1234}Blaster {2}", o, sizeof o) && !strcmp(o, "Blaster {2}"));   // only the leading tag, once
    KT_CHECK(hse::stripNameTag("{06}{07}Twice", o, sizeof o) && !strcmp(o, "{07}Twice"));
    KT_CHECK(!hse::stripNameTag("Vibrosword", o, sizeof o) && !strcmp(o, "Vibrosword"));
    KT_CHECK(!hse::stripNameTag("{06", o, sizeof o) && !strcmp(o, "{06"));      // unterminated
    KT_CHECK(!hse::stripNameTag("{", o, sizeof o) && !strcmp(o, "{"));
    KT_CHECK(!hse::stripNameTag("{}Empty", o, sizeof o) && !strcmp(o, "{}Empty"));
    KT_CHECK(!hse::stripNameTag("{ab}Name", o, sizeof o) && !strcmp(o, "{ab}Name"));
    KT_CHECK(!hse::stripNameTag("", o, sizeof o) && o[0] == '\0');
    KT_CHECK(hse::stripNameTag("{06}", o, sizeof o) && o[0] == '\0');         // tag only
    KT_CHECK(!hse::stripNameTag(nullptr, o, sizeof o) && o[0] == '\0');
    KT_CHECK(!hse::stripNameTag("x", nullptr, 10));
    KT_CHECK(!hse::stripNameTag("x", o, 0));
    // cap safety: a 63-char name fits a 64 buffer; a longer one is cut and terminated
    char longName[120];
    memset(longName, 'a', sizeof longName);
    memcpy(longName, "{06}", 4);
    longName[119] = '\0';
    char c64[64];
    KT_CHECK(hse::stripNameTag(longName, c64, sizeof c64) && strlen(c64) == 63 && c64[63] == '\0');
    char c1[1] = {'z'};
    hse::stripNameTag("abc", c1, 1);
    KT_CHECK(c1[0] == '\0');
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
    testWorldToScreen();
    testSnapAccumRotation();
    testCalibScript();
    testStripNameTag();
    return kt::Report("hse_view");
}

