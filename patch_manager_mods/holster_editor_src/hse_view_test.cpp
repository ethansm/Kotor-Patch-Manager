// T1 host tests for HseView.h (holster editor 3D preview viewport math and strip layout helpers; doc 20).
// Build/run: python3 scripts/run_tests.py hse_view
#include "HseView.h"
#include "check.h"
#include "TextFit.h"
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

// ======================= U2: hse::Chord, weapon gestures, labels =======================

// Drives the chord like the editor does: buttons are forced false while the game is in the background, edges are level changes between ticks.
struct Sim {
    hse::Chord c;
    bool pl = false, pr = false, pm = false;
    bool fg = true;
    int x = 100, y = 100;
    hse::ChordOut o = {};

    hse::ChordOut tick(bool l, bool r, bool m, bool inView = true, int wheel = 0, bool shift = false, bool ctrl = false) {
        if (!fg) l = r = m = false;
        hse::ChordIn in = {};
        in.l = l;
        in.r = r;
        in.m = m;
        in.lEdge = l && !pl;
        in.rEdge = r && !pr;
        in.mEdge = m && !pm;
        in.inView = inView;
        in.fg = fg;
        in.shift = shift;
        in.ctrl = ctrl;
        in.x = x;
        in.y = y;
        in.wheel = wheel;
        pl = l;
        pr = r;
        pm = m;
        o = c.step(in);
        return o;
    }
    hse::ChordOut move(bool l, bool r, bool m, int nx, int ny, bool inView = true) {
        x = nx;
        y = ny;
        return tick(l, r, m, inView);
    }
};

static void testChordLmbOrbit() {
    Sim s;
    hse::ChordOut o = s.tick(false, false, false);
    KT_CHECK_EQ(o.state, hse::CS_IDLE);
    KT_CHECK_EQ(o.act, hse::CA_NONE);
    o = s.tick(true, false, false);   // LMB alone in the view: orbit begins on the press tick
    KT_CHECK_EQ(o.prevState, hse::CS_IDLE);
    KT_CHECK_EQ(o.state, hse::CS_ORBIT);
    KT_CHECK_EQ(o.began, hse::CA_ORBIT);
    KT_CHECK_EQ(o.act, hse::CA_ORBIT);
    KT_CHECK_EQ(o.why, hse::CW_LMB_DOWN);
    KT_CHECK(o.dx == 0 && o.dy == 0);
    o = s.move(true, false, false, 110, 95);
    KT_CHECK_EQ(o.act, hse::CA_ORBIT);
    KT_CHECK(o.began == hse::CA_NONE && o.dx == 10 && o.dy == -5);
    o = s.move(true, false, false, 110, 95);   // no movement: still orbit, zero delta
    KT_CHECK(o.act == hse::CA_ORBIT && o.dx == 0 && o.dy == 0);
    o = s.tick(false, false, false);
    KT_CHECK_EQ(o.state, hse::CS_IDLE);
    KT_CHECK_EQ(o.ended, hse::CA_ORBIT);
    KT_CHECK_EQ(o.why, hse::CW_LMB_UP);
    KT_CHECK_EQ(o.act, hse::CA_NONE);
}

static void testChordRmbAloneDoesNothing() {
    Sim s;
    hse::ChordOut o = s.tick(false, true, false);
    KT_CHECK_EQ(o.state, hse::CS_IDLE);
    KT_CHECK(o.act == hse::CA_NONE && o.began == hse::CA_NONE && o.why == hse::CW_NONE);
    KT_CHECK(s.c.rArmed && s.c.busy());
    o = s.move(false, true, false, 150, 150);   // RMB drag alone: no trackball any more
    KT_CHECK(o.act == hse::CA_NONE && o.state == hse::CS_IDLE && o.dx == 0 && o.dy == 0);
    o = s.tick(false, false, false);
    KT_CHECK(!s.c.rArmed && !s.c.busy());
    KT_CHECK_EQ(o.state, hse::CS_IDLE);
}

static void testChordRmbThenLmbIsWeapon() {
    Sim s;
    s.tick(false, true, false);
    hse::ChordOut o = s.tick(true, true, false);   // RMB first, then LMB
    KT_CHECK_EQ(o.state, hse::CS_WEAPON);
    KT_CHECK_EQ(o.began, hse::CA_WEAPON);
    KT_CHECK_EQ(o.why, hse::CW_CHORD_RL);
    KT_CHECK_EQ(o.act, hse::CA_WEAPON);
    o = s.move(true, true, false, 120, 90);
    KT_CHECK(o.act == hse::CA_WEAPON && o.dx == 20 && o.dy == -10);
    // release order: LMB first -> the gesture ends, RMB still held keeps the chord armed (a new LMB press starts another gesture)
    o = s.tick(false, true, false);
    KT_CHECK_EQ(o.ended, hse::CA_WEAPON);
    KT_CHECK_EQ(o.state, hse::CS_IDLE);
    KT_CHECK_EQ(o.why, hse::CW_LMB_UP);
    KT_CHECK(s.c.rArmed);
    o = s.tick(true, true, false);
    KT_CHECK(o.state == hse::CS_WEAPON && o.began == hse::CA_WEAPON && o.why == hse::CW_CHORD_RL);
    o = s.tick(false, false, false);   // both released in the same tick
    KT_CHECK(o.state == hse::CS_IDLE && o.ended == hse::CA_WEAPON && o.why == hse::CW_ALL_UP);
    KT_CHECK(!s.c.busy());
}

static void testChordRmbReleasedMidWeaponIsDead() {
    Sim s;
    s.tick(false, true, false);
    s.tick(true, true, false);
    hse::ChordOut o = s.tick(true, false, false);   // RMB released first while LMB stays down
    KT_CHECK_EQ(o.state, hse::CS_DEAD);
    KT_CHECK_EQ(o.ended, hse::CA_WEAPON);
    KT_CHECK_EQ(o.why, hse::CW_RMB_UP);
    KT_CHECK_EQ(o.act, hse::CA_NONE);   // no orbit jump
    o = s.move(true, false, false, 200, 200);
    KT_CHECK(o.state == hse::CS_DEAD && o.act == hse::CA_NONE && o.dx == 0 && o.dy == 0);
    o = s.tick(true, true, false);   // RMB pressed again while LMB is still down: nothing starts
    KT_CHECK(o.state == hse::CS_DEAD && o.act == hse::CA_NONE && o.began == hse::CA_NONE);
    o = s.tick(false, false, false);
    KT_CHECK(o.state == hse::CS_IDLE && o.why == hse::CW_ALL_UP);
    o = s.tick(true, false, false);   // a fresh press works again
    KT_CHECK(o.state == hse::CS_ORBIT);
}

static void testChordLmbThenRmbStaysOrbit() {
    Sim s;
    s.tick(true, false, false);
    hse::ChordOut o = s.tick(true, true, false);   // LMB first, then RMB: orbit is latched, the RMB press is ignored
    KT_CHECK_EQ(o.state, hse::CS_ORBIT);
    KT_CHECK_EQ(o.drop, hse::CW_RMB_DURING_ORBIT);
    KT_CHECK_EQ(o.act, hse::CA_ORBIT);
    KT_CHECK_EQ(o.began, hse::CA_NONE);
    o = s.move(true, true, false, 130, 100);
    KT_CHECK(o.act == hse::CA_ORBIT && o.dx == 30);
    o = s.tick(true, false, false);   // RMB released: still the same orbit
    KT_CHECK(o.state == hse::CS_ORBIT && o.ended == hse::CA_NONE);
    o = s.tick(true, true, false);    // and a second RMB press is ignored again
    KT_CHECK(o.state == hse::CS_ORBIT && o.drop == hse::CW_RMB_DURING_ORBIT);
    o = s.tick(false, true, false);   // LMB released with RMB held: orbit ends, nothing starts, RMB is not armed for a chord
    KT_CHECK(o.state == hse::CS_IDLE && o.ended == hse::CA_ORBIT);
    o = s.tick(true, true, false);    // LMB pressed again with the RMB press that was dropped during the orbit still held: dead, never a weapon chord
    KT_CHECK(o.state == hse::CS_DEAD && o.drop == hse::CW_STALE_HELD && o.began == hse::CA_NONE);
    o = s.tick(false, false, false);
    KT_CHECK_EQ(o.state, hse::CS_IDLE);
}

static void testChordSameTickWeaponWins() {
    Sim s;
    hse::ChordOut o = s.tick(true, true, false);
    KT_CHECK_EQ(o.state, hse::CS_WEAPON);
    KT_CHECK_EQ(o.why, hse::CW_CHORD_SAME);
    KT_CHECK_EQ(o.began, hse::CA_WEAPON);
    Sim t;   // same tick but outside the view: dead
    o = t.tick(true, true, false, false);
    KT_CHECK(o.state == hse::CS_DEAD && o.drop == hse::CW_PRESS_OUTSIDE && o.act == hse::CA_NONE);
}

static void testChordPressOutsideNeverStarts() {
    Sim s;
    hse::ChordOut o = s.tick(true, false, false, false);   // LMB pressed outside the view (a strip button)
    KT_CHECK_EQ(o.state, hse::CS_DEAD);
    KT_CHECK_EQ(o.why, hse::CW_PRESS_OUTSIDE);
    KT_CHECK_EQ(o.drop, hse::CW_PRESS_OUTSIDE);
    KT_CHECK_EQ(o.act, hse::CA_NONE);
    o = s.move(true, false, false, 300, 300, true);   // dragged into the view: still nothing
    KT_CHECK(o.state == hse::CS_DEAD && o.act == hse::CA_NONE && o.dx == 0);
    o = s.tick(true, true, false, true);              // RMB pressed in the view meanwhile: still dead
    KT_CHECK(o.state == hse::CS_DEAD && o.act == hse::CA_NONE && o.began == hse::CA_NONE);
    o = s.tick(true, false, false, true);
    KT_CHECK_EQ(o.state, hse::CS_DEAD);
    o = s.tick(false, false, false, true);
    KT_CHECK_EQ(o.state, hse::CS_IDLE);

    Sim r;   // RMB pressed outside, LMB pressed in the view: no gesture (the RMB press never counted)
    o = r.tick(false, true, false, false);
    KT_CHECK_EQ(o.state, hse::CS_DEAD);
    o = r.tick(true, true, false, true);
    KT_CHECK(o.state == hse::CS_DEAD && o.act == hse::CA_NONE);

    Sim q;   // LMB held outside, RMB pressed in the view: dead
    q.tick(true, false, false, false);
    o = q.tick(true, true, false, true);
    KT_CHECK(o.state == hse::CS_DEAD && o.act == hse::CA_NONE);
}

static void testChordForeground() {
    Sim s;
    s.tick(false, true, false);
    s.tick(true, true, false);
    s.move(true, true, false, 150, 150);
    s.fg = false;   // alt-tab mid-drag
    hse::ChordOut o = s.tick(true, true, false);
    KT_CHECK_EQ(o.state, hse::CS_IDLE);
    KT_CHECK_EQ(o.ended, hse::CA_WEAPON);
    KT_CHECK_EQ(o.why, hse::CW_FG_LOST);
    KT_CHECK_EQ(o.act, hse::CA_NONE);
    KT_CHECK(!s.c.rArmed && !s.c.busy());
    o = s.tick(true, true, false);   // still in the background: nothing
    KT_CHECK(o.state == hse::CS_IDLE && o.why == hse::CW_NONE && o.act == hse::CA_NONE);
    s.fg = true;   // back with the buttons still held: a fresh edge is needed, so this is dead until they are released
    o = s.tick(true, true, false);
    KT_CHECK_EQ(o.state, hse::CS_DEAD);
    KT_CHECK_EQ(o.why, hse::CW_FG_RETURN);
    KT_CHECK_EQ(o.act, hse::CA_NONE);
    o = s.move(true, true, false, 10, 10);
    KT_CHECK(o.state == hse::CS_DEAD && o.act == hse::CA_NONE);
    o = s.tick(false, false, false);
    KT_CHECK_EQ(o.state, hse::CS_IDLE);
    o = s.tick(true, false, false);
    KT_CHECK_EQ(o.state, hse::CS_ORBIT);

    Sim t;   // returning with nothing held does not go dead
    t.fg = false;
    t.tick(false, false, false);
    t.fg = true;
    o = t.tick(false, false, false);
    KT_CHECK(o.state == hse::CS_IDLE && o.why == hse::CW_NONE);
    o = t.tick(true, false, false);
    KT_CHECK_EQ(o.state, hse::CS_ORBIT);

    Sim u;   // foreground lost mid-orbit
    u.tick(true, false, false);
    u.fg = false;
    o = u.tick(true, false, false);
    KT_CHECK(o.ended == hse::CA_ORBIT && o.why == hse::CW_FG_LOST && o.state == hse::CS_IDLE);

    Sim v;   // foreground lost mid-spin
    v.tick(false, false, true);
    v.fg = false;
    o = v.tick(false, false, true);
    KT_CHECK(o.ended == hse::CA_SPIN && o.why == hse::CW_FG_LOST && !v.c.spinning);
}

static void testChordWheel() {
    Sim s;
    hse::ChordOut o = s.tick(false, false, false, true, 120);   // no RMB: zoom
    KT_CHECK(o.wheelAct == hse::CA_ZOOM && o.wheel == 120);
    o = s.tick(false, false, false, true, 0);
    KT_CHECK_EQ(o.wheelAct, hse::CA_NONE);
    o = s.tick(false, true, false, true, -120);   // RMB held (armed): depth
    KT_CHECK(o.wheelAct == hse::CA_DEPTH && o.wheel == -120);
    o = s.tick(true, true, false, true, 120);     // weapon gesture running: depth
    KT_CHECK(o.state == hse::CS_WEAPON && o.wheelAct == hse::CA_DEPTH);
    o = s.tick(false, true, false, true, 120);    // LMB lifted, RMB still held: depth
    KT_CHECK(o.wheelAct == hse::CA_DEPTH);
    o = s.tick(false, false, false, true, 120);   // RMB released: zoom again
    KT_CHECK_EQ(o.wheelAct, hse::CA_ZOOM);

    Sim t;   // RMB pressed in the same tick as the wheel notch
    o = t.tick(false, true, false, true, 120);
    KT_CHECK_EQ(o.wheelAct, hse::CA_DEPTH);

    Sim u;   // orbit latched, RMB held: zoom, not depth
    u.tick(true, false, false);
    o = u.tick(true, true, false, true, 120);
    KT_CHECK(o.state == hse::CS_ORBIT && o.wheelAct == hse::CA_ZOOM);

    Sim d;   // RMB pressed outside the view (dead): wheel zooms
    d.tick(false, true, false, false);
    o = d.tick(false, true, false, true, 120);
    KT_CHECK(o.state == hse::CS_DEAD && o.wheelAct == hse::CA_ZOOM);

    Sim f;   // background: no wheel action
    f.fg = false;
    o = f.tick(false, false, false, true, 120);
    KT_CHECK_EQ(o.wheelAct, hse::CA_NONE);
}

static void testChordMmbSpin() {
    Sim s;
    hse::ChordOut o = s.tick(false, false, true);
    KT_CHECK(o.began == hse::CA_SPIN && o.act == hse::CA_SPIN && o.why == hse::CW_MMB_DOWN && o.state == hse::CS_IDLE);
    o = s.move(false, false, true, 130, 100);
    KT_CHECK(o.act == hse::CA_SPIN && o.dx == 30);
    o = s.tick(false, false, false);
    KT_CHECK(o.ended == hse::CA_SPIN && o.why == hse::CW_MMB_UP && o.act == hse::CA_NONE);

    Sim t;   // MMB pressed outside the view: no spin, even after it moves in
    o = t.tick(false, false, true, false);
    KT_CHECK(o.began == hse::CA_NONE && o.drop == hse::CW_PRESS_OUTSIDE && !t.c.spinning);
    o = t.move(false, false, true, 200, 200, true);
    KT_CHECK(o.act == hse::CA_NONE);

    Sim u;   // MMB pressed while a weapon gesture runs: ignored, and it does not start when the gesture ends
    u.tick(true, true, false);
    o = u.tick(true, true, true);
    KT_CHECK(o.drop == hse::CW_MMB_BUSY && o.act == hse::CA_WEAPON && !u.c.spinning);
    o = u.tick(false, false, true);
    KT_CHECK(o.ended == hse::CA_WEAPON && o.act == hse::CA_NONE && !u.c.spinning);

    Sim v;   // LMB pressed while the spin runs: dead, the spin continues until MMB is up
    v.tick(false, false, true);
    o = v.tick(true, false, true);
    KT_CHECK(o.state == hse::CS_DEAD && o.why == hse::CW_MMB_ACTIVE && o.act == hse::CA_SPIN);
    o = v.tick(true, false, false);
    KT_CHECK(o.ended == hse::CA_SPIN && o.state == hse::CS_DEAD);
    o = v.tick(false, false, false);
    KT_CHECK_EQ(o.state, hse::CS_IDLE);
}

static void testChordModifiersAndDelta() {
    Sim s;
    s.tick(false, true, false);
    s.tick(true, true, false);
    hse::ChordOut o = s.tick(true, true, false, true, 0, true, false);
    KT_CHECK(o.fine && !o.snap);
    s.x = 140;
    o = s.tick(true, true, false, true, 0, false, true);   // Shift released and Ctrl pressed mid-drag: speed and snap only, the delta is unchanged
    KT_CHECK(!o.fine && o.snap && o.dx == 40 && o.act == hse::CA_WEAPON);
    s.x = 150;
    o = s.tick(true, true, false, true, 0, true, true);
    KT_CHECK(o.fine && o.snap && o.dx == 10);
    // a tick gap (the editor skipped some ticks): the delta spans the gap, no jump hidden
    s.x = 300;
    o = s.tick(true, true, false);
    KT_CHECK_EQ(o.dx, 150);
    // reset drops everything
    s.c.reset();
    KT_CHECK(!s.c.busy() && s.c.state == hse::CS_IDLE);
    o = s.tick(true, true, false);   // levels held, no edge any more: stale, not a gesture
    KT_CHECK(o.state == hse::CS_IDLE && o.act == hse::CA_NONE);
}

static void testChordStaleHeldAfterReset() {
    // The editor resets the chord when edit mode opens while LMB is still down (the click that entered). Levels without edges never start anything.
    Sim s;
    s.pl = true;   // as if resetInput() re-read the held button
    hse::ChordOut o = s.tick(true, false, false);
    KT_CHECK(o.state == hse::CS_IDLE && o.act == hse::CA_NONE);
    o = s.tick(true, true, false);   // RMB press while that stale LMB is down: dead, never a guess
    KT_CHECK(o.state == hse::CS_DEAD && o.drop == hse::CW_STALE_HELD);
    o = s.tick(false, false, false);
    KT_CHECK_EQ(o.state, hse::CS_IDLE);
    Sim t;
    t.pr = true;   // stale RMB then LMB press: dead
    t.tick(false, true, false);
    o = t.tick(true, true, false);
    KT_CHECK(o.state == hse::CS_DEAD && o.drop == hse::CW_STALE_HELD);
}

static void testChordNames() {
    KT_CHECK_EQ(hse::chordStateName(hse::CS_WEAPON), "WEAPON");
    KT_CHECK_EQ(hse::chordStateName(99), "?");
    KT_CHECK_EQ(hse::chordActName(hse::CA_DEPTH), "depth");
    KT_CHECK_EQ(hse::chordWhyName(hse::CW_CHORD_RL), "chord_rl");
    KT_CHECK_EQ(hse::chordWhyName(hse::CW_STALE_HELD), "stale_held");
    KT_CHECK_EQ(hse::chordWhyName(-1), "?");
    for (int w = 0; w < hse::CW_COUNT; ++w) KT_CHECK(strcmp(hse::chordWhyName(w), "?") != 0);
}

static bool nearF(float a, float b, float e = 1e-5f) { return fabsf(a - b) <= e; }

static void testWeaponGestureMove() {
    hse::SnapAcc acc;
    hse::WeaponStep st = hse::weaponGesture(hse::WPM_MOVE, 10, -4, 1.0f, false, false, acc);   // right 10 px, up 4 px
    KT_CHECK_EQ(st.n, 1);
    KT_CHECK_EQ(st.c[0].kind, hse::WK_WORLD);
    KT_CHECK(nearF(st.c[0].dp[0], 0.0f) && nearF(st.c[0].dp[1], 0.025f) && nearF(st.c[0].dp[2], 0.01f));
    KT_CHECK(nearF(st.a, 0.025f) && nearF(st.b, 0.01f));
    st = hse::weaponGesture(hse::WPM_MOVE, 10, -4, 2.0f, false, false, acc);   // zoom 2: half the metres per px
    KT_CHECK(nearF(st.c[0].dp[1], 0.0125f) && nearF(st.c[0].dp[2], 0.005f));
    st = hse::weaponGesture(hse::WPM_MOVE, 10, -4, 1.0f, true, false, acc);    // Shift: x0.2
    KT_CHECK(nearF(st.c[0].dp[1], 0.005f) && nearF(st.c[0].dp[2], 0.002f));
    st = hse::weaponGesture(hse::WPM_MOVE, 0, 0, 1.0f, false, false, acc);
    KT_CHECK_EQ(st.n, 0);
    st = hse::weaponGesture(hse::WPM_MOVE, 3, 0, 0.0f, false, false, acc);     // a bad zoom reads as 1
    KT_CHECK(nearF(st.c[0].dp[1], 0.0075f));
    float dp[3];
    hse::moveDp(4.0f, 8.0f, 0.5f, dp);
    KT_CHECK(nearF(dp[0], 0.0f) && nearF(dp[1], 2.0f) && nearF(dp[2], -4.0f));
}

static void testWeaponGestureMoveSnap() {
    // Ctrl snaps y and z to 1 cm; the remainder carries so slow drags still move; sum(applied) + acc == sum(raw)
    hse::SnapAcc acc;
    float sumY = 0.0f, sumRawY = 0.0f;
    int calls = 0;
    for (int i = 0; i < 20; ++i) {
        hse::WeaponStep st = hse::weaponGesture(hse::WPM_MOVE, 1, 0, 1.0f, false, true, acc);   // 2.5 mm per tick
        sumRawY += 0.0025f;
        if (st.n) {
            ++calls;
            sumY += st.c[0].dp[1];
            // every applied amount is a whole number of centimetres
            KT_CHECK(nearF(st.c[0].dp[1] * 100.0f, roundf(st.c[0].dp[1] * 100.0f), 1e-3f));
            KT_CHECK(nearF(st.c[0].dp[2], 0.0f));
        }
    }
    KT_CHECK(calls >= 4 && calls <= 6);   // 5 cm of drag = about 5 one-centimetre steps
    KT_CHECK(nearF(sumY + acc.a[0], sumRawY, 1e-5f));
    KT_CHECK(acc.nonzero() || fabsf(acc.a[0]) < 1e-9f);
    // releasing Ctrl clears the used remainders; the wheel remainder is separate
    acc.a[2] = 0.003f;
    hse::weaponGesture(hse::WPM_MOVE, 1, 1, 1.0f, false, false, acc);
    KT_CHECK(acc.a[0] == 0.0f && acc.a[1] == 0.0f && nearF(acc.a[2], 0.003f));
    acc.reset();
    KT_CHECK(!acc.nonzero());
    // negative drags snap toward zero, symmetric
    hse::SnapAcc n1, n2;
    float up = 0, down = 0;
    for (int i = 0; i < 9; ++i) {
        hse::WeaponStep a = hse::weaponGesture(hse::WPM_MOVE, 0, -4, 1.0f, false, true, n1);   // z = +0.01 per tick
        hse::WeaponStep b = hse::weaponGesture(hse::WPM_MOVE, 0, 4, 1.0f, false, true, n2);    // z = -0.01 per tick
        up += a.b;
        down += b.b;
    }
    KT_CHECK(nearF(up, -down, 1e-4f));
}

static void testWeaponGestureDepthTiltRoll() {
    hse::SnapAcc acc;
    hse::WeaponStep st = hse::weaponGesture(hse::WPM_DEPTH, 7, -8, 1.0f, false, false, acc);   // dx is ignored; up 8 px = forward 2 cm
    KT_CHECK_EQ(st.n, 1);
    KT_CHECK(st.c[0].kind == hse::WK_LOCAL && st.c[0].axis == 1 && nearF(st.c[0].metres, 0.02f) && nearF(st.c[0].deg, 0.0f));
    KT_CHECK(nearF(st.a, 0.02f) && nearF(st.b, 0.0f));
    st = hse::weaponGesture(hse::WPM_DEPTH, 9, 0, 1.0f, false, false, acc);
    KT_CHECK_EQ(st.n, 0);
    st = hse::weaponGesture(hse::WPM_DEPTH, 0, 8, 2.0f, true, false, acc);   // down, zoom 2, Shift
    KT_CHECK(nearF(st.c[0].metres, -0.002f, 1e-6f));

    st = hse::weaponGesture(hse::WPM_TILT, 50, -10, 1.0f, false, false, acc);   // Tilt = bone axis 0, vertical drag, 0.5 deg/px; dx ignored
    KT_CHECK(st.n == 1 && st.c[0].kind == hse::WK_LOCAL && st.c[0].axis == 0 && nearF(st.c[0].deg, 5.0f) && nearF(st.c[0].metres, 0.0f));
    st = hse::weaponGesture(hse::WPM_TILT, 0, -10, 1.0f, true, false, acc);
    KT_CHECK(nearF(st.c[0].deg, 1.0f));
    st = hse::weaponGesture(hse::WPM_TILT, 10, 0, 1.0f, false, false, acc);
    KT_CHECK_EQ(st.n, 0);

    // Roll/Turn: horizontal = Turn (bone axis 2), vertical = Roll (bone axis 1); a diagonal drag makes both calls (Turn first)
    st = hse::weaponGesture(hse::WPM_ROLLTURN, 10, 0, 1.0f, false, false, acc);
    KT_CHECK(st.n == 1 && st.c[0].axis == 2 && nearF(st.c[0].deg, 5.0f));
    KT_CHECK(nearF(st.a, 0.0f) && nearF(st.b, 5.0f));
    st = hse::weaponGesture(hse::WPM_ROLLTURN, 0, -10, 1.0f, false, false, acc);
    KT_CHECK(st.n == 1 && st.c[0].axis == 1 && nearF(st.c[0].deg, 5.0f));
    KT_CHECK(nearF(st.a, 5.0f) && nearF(st.b, 0.0f));
    st = hse::weaponGesture(hse::WPM_ROLLTURN, -4, 6, 1.0f, false, false, acc);
    KT_CHECK(st.n == 2 && st.c[0].axis == 2 && nearF(st.c[0].deg, -2.0f) && st.c[1].axis == 1 && nearF(st.c[1].deg, -3.0f));
    st = hse::weaponGesture(hse::WPM_ROLLTURN, 0, 0, 1.0f, false, false, acc);
    KT_CHECK_EQ(st.n, 0);
    st = hse::weaponGesture(99, 5, 5, 1.0f, false, false, acc);   // unknown mode: nothing
    KT_CHECK(st.n == 0 && nearF(st.a, 0.0f));
}

static void testWeaponGestureRotationSnap() {
    // Ctrl snaps rotation to 5 degrees with a carry: 1 deg per tick -> a 5 degree step every fifth tick, none lost
    hse::SnapAcc acc;
    float sum = 0.0f;
    int steps = 0;
    for (int i = 0; i < 23; ++i) {
        hse::WeaponStep st = hse::weaponGesture(hse::WPM_TILT, 0, -2, 1.0f, false, true, acc);   // 1 deg per tick
        if (st.n) {
            ++steps;
            sum += st.c[0].deg;
            KT_CHECK(nearF(fmodf(fabsf(st.c[0].deg), 5.0f), 0.0f, 1e-3f));
        }
    }
    KT_CHECK_EQ(steps, 4);                      // 23 deg of drag = four 5-degree steps, 3 deg in the carry
    KT_CHECK(nearF(sum, 20.0f, 1e-3f));
    KT_CHECK(nearF(acc.a[0], 3.0f, 1e-3f));
    // slow Shift+Ctrl drag still turns eventually (carry), where per-tick rounding never would
    hse::SnapAcc slow;
    float turned = 0.0f;
    for (int i = 0; i < 100; ++i) turned += hse::weaponGesture(hse::WPM_ROLLTURN, 1, 0, 1.0f, true, true, slow).b;   // 0.1 deg per tick
    KT_CHECK(turned >= 5.0f - 1e-3f && nearF(fmodf(turned, 5.0f), 0.0f, 1e-3f));   // whole 5 degree steps only
    KT_CHECK(nearF(turned + slow.a[0], 10.0f, 1e-3f));                              // nothing lost (the last step may still sit in the carry)
}

static void testWheelDepth() {
    hse::SnapAcc acc;
    hse::WeaponCall c = hse::wheelDepth(120, false, false, acc);
    KT_CHECK(c.kind == hse::WK_LOCAL && c.axis == 1 && nearF(c.metres, 0.02f));
    c = hse::wheelDepth(-240, false, false, acc);
    KT_CHECK(nearF(c.metres, -0.04f));
    c = hse::wheelDepth(120, true, false, acc);   // Shift
    KT_CHECK(nearF(c.metres, 0.004f));
    c = hse::wheelDepth(0, false, false, acc);
    KT_CHECK_EQ(c.kind, hse::WK_NONE);
    // Ctrl snap with a carry: Shift notches of 4 mm need three notches for one centimetre
    float total = 0.0f;
    int applied = 0;
    for (int i = 0; i < 7; ++i) {
        c = hse::wheelDepth(120, true, true, acc);
        if (c.kind == hse::WK_LOCAL) {
            total += c.metres;
            ++applied;
        }
    }
    KT_CHECK(applied == 2 && nearF(total, 0.02f, 1e-5f) && nearF(acc.a[2], 0.008f, 1e-5f));
    hse::wheelDepth(120, false, false, acc);   // releasing Ctrl clears the wheel remainder
    KT_CHECK(acc.a[2] == 0.0f);
}

static void testModeAndScopeLabels() {
    char b[64];
    hse::modeLabel(b, sizeof b, hse::WPM_MOVE, true);
    KT_CHECK_EQ(b, ">Move<");
    hse::modeLabel(b, sizeof b, hse::WPM_DEPTH, false);
    KT_CHECK_EQ(b, "Depth");
    hse::modeLabel(b, sizeof b, hse::WPM_ROLLTURN, true);
    KT_CHECK_EQ(b, ">Roll/Turn<");
    hse::modeLabel(b, sizeof b, 9, false);
    KT_CHECK_EQ(b, "?");
    char tiny[4];
    hse::modeLabel(tiny, sizeof tiny, hse::WPM_ROLLTURN, true);
    KT_CHECK(strlen(tiny) == 3 && tiny[3] == '\0');
    hse::modeLabel(nullptr, 8, 0, true);
    hse::modeLabel(b, 0, 0, true);
    KT_CHECK_EQ(hse::weaponModeName(hse::WPM_TILT), "Tilt");

    KT_CHECK_EQ(hse::categoryPlural("rifle"), "rifles");
    KT_CHECK_EQ(hse::categoryPlural("pistol"), "pistols");
    KT_CHECK_EQ(hse::categoryPlural("melee"), "melee weapons");
    KT_CHECK_EQ(hse::categoryPlural("lightsaber"), "lightsabers");
    KT_CHECK_EQ(hse::categoryPlural("qstaff"), "quarterstaffs");
    KT_CHECK_EQ(hse::categoryPlural("heavy"), "heavy weapons");
    KT_CHECK_EQ(hse::categoryPlural("unknown"), "of this type");
    KT_CHECK_EQ(hse::categoryPlural(""), "of this type");
    KT_CHECK_EQ(hse::categoryPlural(nullptr), "of this type");
    KT_CHECK_EQ(hse::categoryPlural("flamethrower"), "of this type");

    hse::scopeButtonText(b, sizeof b, 0, "rifle");
    KT_CHECK_EQ(b, "For: this weapon");
    hse::scopeButtonText(b, sizeof b, 1, "rifle");
    KT_CHECK_EQ(b, "For: all rifles");
    hse::scopeButtonText(b, sizeof b, 1, "heavy");
    KT_CHECK_EQ(b, "For: all heavy weapons");
    hse::scopeButtonText(b, sizeof b, 1, "unknown");
    KT_CHECK_EQ(b, "For: all of this type");
    hse::scopeButtonText(b, sizeof b, 1, nullptr);
    KT_CHECK_EQ(b, "For: all of this type");
    hse::scopeButtonText(b, sizeof b, 2, "rifle");
    KT_CHECK_EQ(b, "For: this body");
    hse::scopeButtonText(b, sizeof b, 2, "unknown");
    KT_CHECK_EQ(b, "For: this body");
    hse::scopeButtonText(b, sizeof b, 7, "rifle");
    KT_CHECK_EQ(b, "For: this weapon");
    hse::scopeButtonText(tiny, sizeof tiny, 1, "rifle");
    KT_CHECK(strlen(tiny) == 3);
    hse::scopeButtonText(nullptr, 5, 0, "x");
    // the scope tip fits the mod-config description buffer (192) and is plain ASCII; so is the help line
    const char* tip = hse::scopeTip();
    KT_CHECK(strlen(tip) < 190);
    for (const char* p = tip; *p; ++p) KT_CHECK(static_cast<unsigned char>(*p) >= 0x20 && static_cast<unsigned char>(*p) < 0x7F);
    for (const char* p = hse::helpLine(); *p; ++p) KT_CHECK(static_cast<unsigned char>(*p) >= 0x20 && static_cast<unsigned char>(*p) < 0x7F);
    KT_CHECK_EQ(hse::helpLine(), "LMB turn . RMB+LMB move weapon . RMB+wheel depth . Shift fine . Ctrl snap");
    KT_CHECK(strlen(hse::helpLine()) < 95);
    // the calibration hint
    KT_CHECK_EQ(hse::kCalibHint, " (wheel = zoom out)");
    hse::CalibScript cs;
    cs.round = 10;
    cs.step = 0;   // the longest target name
    char raw[96], both[160];
    hse::calibPrompt(raw, sizeof raw, &cs);
    snprintf(both, sizeof both, "%s%s", raw, hse::kCalibHint);
    KT_CHECK(strlen(both) < 95);
}

static void testHandButtonText() {
    char b[96];
    hse::handButtonText(b, sizeof b, 0, false, "Vibrosword", "w_vbrosw_001", true);
    KT_CHECK_EQ(b, "Right: Vibrosword");
    hse::handButtonText(b, sizeof b, 1, true, "Vibrosword", "w_vbrosw_001", true);
    KT_CHECK_EQ(b, "L: Vibrosword");
    hse::handButtonText(b, sizeof b, 0, false, "", "w_vbrosw_001", true);   // empty name: the resref
    KT_CHECK_EQ(b, "Right: w_vbrosw_001");
    hse::handButtonText(b, sizeof b, 0, false, nullptr, nullptr, true);
    KT_CHECK_EQ(b, "Right: Equipped");
    hse::handButtonText(b, sizeof b, 1, false, "", "", false);
    KT_CHECK_EQ(b, "Left: None");
    char tiny[6];
    hse::handButtonText(tiny, sizeof tiny, 0, false, "Vibrosword", "", true);
    KT_CHECK(strlen(tiny) == 5);
}

static void testLabelsFit() {
    // Vanilla 800x600: the strip is ~163 px, a half button 78 px (leftStrip/half above). Fitting never exceeds the width and falls back to "...".
    const textfit::TextCtx tc = {1.0f, 1.0f, 1.0f};
    const float half = 78.0f, full = 163.0f;
    char fit[64];
    const char* names[] = {"Vibrosword", "Mandalorian Heavy Repeating Blaster Rifle Mark Two Extended", "Lightsaber", "x"};
    for (const char* n : names) {
        char raw[96];
        hse::handButtonText(raw, sizeof raw, 0, true, n, "", true);
        textfit::fitText(tc, raw, half, fit, sizeof fit);
        KT_CHECK(textfit::textPx(tc, fit, static_cast<int>(strlen(fit))) <= half + 0.001f);
        KT_CHECK(strlen(fit) > 0);
    }
    char raw[96];
    hse::handButtonText(raw, sizeof raw, 0, true, "Mandalorian Heavy Repeating Blaster Rifle Mark Two Extended", "", true);
    KT_CHECK(textfit::fitText(tc, raw, half, fit, sizeof fit));      // cut
    KT_CHECK(strlen(fit) >= 3 && !strcmp(fit + strlen(fit) - 3, "..."));
    hse::handButtonText(raw, sizeof raw, 0, true, "Pistol", "", true);
    KT_CHECK(!textfit::fitText(tc, raw, half, fit, sizeof fit));     // short names are untouched
    KT_CHECK_EQ(fit, "R: Pistol");
    // the scope text and the mode bar in their slots
    const char* types[] = {"lightsaber", "melee", "qstaff", "pistol", "rifle", "heavy", "unknown"};
    for (const char* t : types) {
        char sc[64];
        hse::scopeButtonText(sc, sizeof sc, 1, t);
        textfit::fitText(tc, sc, full, fit, sizeof fit);
        KT_CHECK(textfit::textPx(tc, fit, static_cast<int>(strlen(fit))) <= full + 0.001f);
    }
    for (int m = 0; m < hse::WPM_COUNT; ++m) {
        char ml[32];
        hse::modeLabel(ml, sizeof ml, m, true);
        textfit::fitText(tc, ml, half, fit, sizeof fit);
        KT_CHECK(textfit::textPx(tc, fit, static_cast<int>(strlen(fit))) <= half + 0.001f);
    }
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
    testChordLmbOrbit();
    testChordRmbAloneDoesNothing();
    testChordRmbThenLmbIsWeapon();
    testChordRmbReleasedMidWeaponIsDead();
    testChordLmbThenRmbStaysOrbit();
    testChordSameTickWeaponWins();
    testChordPressOutsideNeverStarts();
    testChordForeground();
    testChordWheel();
    testChordMmbSpin();
    testChordModifiersAndDelta();
    testChordStaleHeldAfterReset();
    testChordNames();
    testWeaponGestureMove();
    testWeaponGestureMoveSnap();
    testWeaponGestureDepthTiltRoll();
    testWeaponGestureRotationSnap();
    testWheelDepth();
    testModeAndScopeLabels();
    testHandButtonText();
    testLabelsFit();
    return kt::Report("hse_view");
}

