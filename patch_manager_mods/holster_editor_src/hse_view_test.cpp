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

// ---- U3 inspector rows ----

static void testInspRowBasics() {
    KT_CHECK_EQ(static_cast<int>(hse::IR_COUNT), 6);
    KT_CHECK(!strcmp(hse::inspRowName(hse::IR_SLIDE), "Slide"));
    KT_CHECK(!strcmp(hse::inspRowName(hse::IR_DEPTH), "Depth"));
    KT_CHECK(!strcmp(hse::inspRowName(hse::IR_HEIGHT), "Height"));
    KT_CHECK(!strcmp(hse::inspRowName(hse::IR_TILT), "Tilt"));
    KT_CHECK(!strcmp(hse::inspRowName(hse::IR_ROLL), "Roll"));
    KT_CHECK(!strcmp(hse::inspRowName(hse::IR_TURN), "Turn"));
    KT_CHECK(!strcmp(hse::inspRowName(-1), "?") && !strcmp(hse::inspRowName(6), "?"));
    // Slide x, Depth y, Height z of the offset; Tilt x, Roll y, Turn z of the rotation (H6: in the bone frame y is depth, z is height)
    const float rot[3] = {10.0f, 20.0f, 30.0f}, off[3] = {0.1f, 0.2f, 0.3f};
    KT_CHECK(closeF(hse::inspGet(hse::IR_SLIDE, rot, off), 0.1f) && closeF(hse::inspGet(hse::IR_DEPTH, rot, off), 0.2f) &&
             closeF(hse::inspGet(hse::IR_HEIGHT, rot, off), 0.3f));
    KT_CHECK(closeF(hse::inspGet(hse::IR_TILT, rot, off), 10.0f) && closeF(hse::inspGet(hse::IR_ROLL, rot, off), 20.0f) &&
             closeF(hse::inspGet(hse::IR_TURN, rot, off), 30.0f));
    KT_CHECK(std::isnan(hse::inspGet(-1, rot, off)) && std::isnan(hse::inspGet(6, rot, off)) && std::isnan(hse::inspGet(0, nullptr, off)));
    for (int r = 0; r < hse::IR_COUNT; ++r) {
        float rr[3] = {1, 2, 3}, oo[3] = {4, 5, 6};
        hse::inspSet(r, rr, oo, 99.0f);
        int changed = 0;
        for (int i = 0; i < 3; ++i) changed += (rr[i] == 99.0f) + (oo[i] == 99.0f);
        KT_CHECK_EQ(changed, 1);   // exactly one of the six values moves
        KT_CHECK(closeF(hse::inspGet(r, rr, oo), 99.0f));
        KT_CHECK_EQ(hse::inspIsRot(r) ? 1 : 0, r >= 3 ? 1 : 0);
    }
    float rr[3] = {1, 2, 3}, oo[3] = {4, 5, 6};
    hse::inspSet(-1, rr, oo, 99.0f);
    hse::inspSet(6, rr, oo, 99.0f);
    KT_CHECK(rr[0] == 1 && rr[1] == 2 && rr[2] == 3 && oo[0] == 4 && oo[1] == 5 && oo[2] == 6);
    // body scope has no rotation
    for (int r = 0; r < hse::IR_COUNT; ++r) {
        KT_CHECK(hse::inspRowEnabled(r, 0) && hse::inspRowEnabled(r, 1));
        KT_CHECK_EQ(hse::inspRowEnabled(r, 2) ? 1 : 0, hse::inspIsRot(r) ? 0 : 1);
    }
    for (int r = 0; r < hse::IR_COUNT; ++r) {
        KT_CHECK(strlen(hse::inspTip(r)) > 20 && strlen(hse::inspTip(r)) < 190);
        for (const char* c = hse::inspTip(r); *c; ++c) KT_CHECK(static_cast<unsigned char>(*c) >= 0x20 && static_cast<unsigned char>(*c) < 0x7F);
    }
    KT_CHECK(strlen(hse::inspResetTip()) < 190);
}

static void testInspItemBudget() {
    // 26 items before the inspector + 12 = 38 of mod-config's 40 (kMaxItems 40 in mod-config 0.1.1+)
    KT_CHECK_EQ(hse::kInspItems, 12);
    KT_CHECK_EQ(hse::kPageItemsBase + hse::kInspItems, 38);
    KT_CHECK(hse::kPageItemsBase + hse::kInspItems <= hse::kPageItemCap);
    KT_CHECK_EQ(hse::kPageItemCap, 40);
    KT_CHECK_EQ(hse::kStripRowsInsp, 15);
}

static void testInspRowText() {
    char t[64];
    hse::inspRowText(t, sizeof t, hse::IR_DEPTH, -0.11f, false, true);
    KT_CHECK(!strcmp(t, "Depth  -11.0 cm"));
    hse::inspRowText(t, sizeof t, hse::IR_TILT, 30.5f, false, true);
    KT_CHECK(!strcmp(t, "Tilt  30.5 deg"));
    hse::inspRowText(t, sizeof t, hse::IR_HEIGHT, 0.25f, true, true);
    KT_CHECK(!strcmp(t, "* Height  25.0 cm"));
    hse::inspRowText(t, sizeof t, hse::IR_SLIDE, 0.0f, false, true);
    KT_CHECK(!strcmp(t, "Slide  0.0 cm"));
    hse::inspRowText(t, sizeof t, hse::IR_SLIDE, -0.00001f, false, true);   // rounds to zero: never "-0.0"
    KT_CHECK(!strcmp(t, "Slide  0.0 cm"));
    hse::inspRowText(t, sizeof t, hse::IR_ROLL, -0.01f, false, true);
    KT_CHECK(!strcmp(t, "Roll  0.0 deg"));
    hse::inspRowText(t, sizeof t, hse::IR_TURN, -90.0f, false, true);
    KT_CHECK(!strcmp(t, "Turn  -90.0 deg"));
    hse::inspRowText(t, sizeof t, hse::IR_TURN, 720.0f, true, true);
    KT_CHECK(!strcmp(t, "* Turn  720.0 deg"));
    hse::inspRowText(t, sizeof t, hse::IR_DEPTH, 3.0f, false, true);
    KT_CHECK(!strcmp(t, "Depth  300.0 cm"));
    hse::inspRowText(t, sizeof t, hse::IR_TILT, std::nanf(""), false, true);
    KT_CHECK(!strcmp(t, "Tilt  n/a"));
    hse::inspRowText(t, sizeof t, hse::IR_TILT, INFINITY, true, true);
    KT_CHECK(!strcmp(t, "Tilt  n/a"));
    hse::inspRowText(t, sizeof t, hse::IR_TILT, 12.0f, true, false);   // disabled (body scope) beats the value and the marker
    KT_CHECK(!strcmp(t, "Tilt  (body: n/a)"));
    char small[8];
    hse::inspRowText(small, sizeof small, hse::IR_HEIGHT, 0.25f, true, true);
    KT_CHECK(small[7] == '\0' && strlen(small) == 7);
    hse::inspRowText(nullptr, 8, 0, 0.0f, false, true);   // no-ops
    hse::inspRowText(t, 0, 0, 0.0f, false, true);
    // every row text of a realistic value fits the scrub button (the 139 px of the 163 px strip minus the reset button) after fitText
    const textfit::TextCtx tc = {1.0f, 1.0f, 1.0f};
    char fit[64];
    for (int r = 0; r < hse::IR_COUNT; ++r) {
        hse::inspRowText(t, sizeof t, r, hse::inspIsRot(r) ? -150.0f : -0.11f, true, true);
        textfit::fitText(tc, t, 139.0f, fit, sizeof fit);
        KT_CHECK(textfit::textPx(tc, fit, static_cast<int>(strlen(fit))) <= 139.0f + 0.001f);
    }
}

static void testInspDiffersAndMatches() {
    KT_CHECK(!hse::inspDiffers(hse::IR_DEPTH, -0.11f, -0.11f));
    KT_CHECK(!hse::inspDiffers(hse::IR_DEPTH, -0.11f, -0.11f + 0.0001f));   // under half a display step (0.05 cm)
    KT_CHECK(hse::inspDiffers(hse::IR_DEPTH, -0.11f, -0.10f));              // 1 cm
    KT_CHECK(hse::inspDiffers(hse::IR_DEPTH, -0.11f, -0.1094f));            // 0.06 cm
    KT_CHECK(!hse::inspDiffers(hse::IR_TILT, 30.0f, 30.04f));
    KT_CHECK(hse::inspDiffers(hse::IR_TILT, 30.0f, 30.1f));
    KT_CHECK(hse::inspDiffers(hse::IR_TURN, -90.0f, 90.0f));
    KT_CHECK(!hse::inspDiffers(hse::IR_TILT, std::nanf(""), 0.0f) && !hse::inspDiffers(hse::IR_TILT, 0.0f, INFINITY));

    KT_CHECK(hse::inspMatches(hse::IR_DEPTH, 0.2f, 0.2f) && !hse::inspMatches(hse::IR_DEPTH, 0.2f, 0.21f));
    KT_CHECK(hse::inspMatches(hse::IR_TILT, 90.0f, 450.0f));     // SetValues(450) reads back as 90
    KT_CHECK(hse::inspMatches(hse::IR_TILT, -170.0f, 190.0f));
    KT_CHECK(hse::inspMatches(hse::IR_TILT, 180.0f, -180.0f));
    KT_CHECK(!hse::inspMatches(hse::IR_TILT, 90.0f, 91.0f));
    KT_CHECK(!hse::inspMatches(hse::IR_TILT, std::nanf(""), 1.0f) && !hse::inspMatches(hse::IR_DEPTH, 1.0f, INFINITY));
}

static void testInspRevert() {
    const float openRot[3] = {-150.0f, 0.0f, 94.0f}, openOff[3] = {0.0f, -0.11f, 0.22f};
    const float curRot[3] = {-120.0f, 5.0f, 100.0f}, curOff[3] = {0.02f, -0.15f, 0.30f};
    for (int r = 0; r < hse::IR_COUNT; ++r) {
        float oRot[3], oOff[3];
        KT_CHECK(hse::inspRevert(r, curRot, curOff, openRot, openOff, oRot, oOff));
        for (int i = 0; i < 3; ++i) {
            const bool thisRot = hse::inspIsRot(r) && hse::inspAxis(r) == i;
            const bool thisOff = !hse::inspIsRot(r) && hse::inspAxis(r) == i;
            KT_CHECK(oRot[i] == (thisRot ? openRot[i] : curRot[i]));   // only the one value goes back; the other five are untouched
            KT_CHECK(oOff[i] == (thisOff ? openOff[i] : curOff[i]));
        }
    }
    // already at the open value: no change reported, values copied through
    float oRot[3], oOff[3];
    KT_CHECK(!hse::inspRevert(hse::IR_DEPTH, openRot, openOff, openRot, openOff, oRot, oOff));
    KT_CHECK(oRot[0] == openRot[0] && oOff[1] == openOff[1]);
    KT_CHECK(!hse::inspRevert(-1, curRot, curOff, openRot, openOff, oRot, oOff));
    KT_CHECK(oRot[0] == curRot[0] && oOff[2] == curOff[2]);
    KT_CHECK(!hse::inspRevert(6, curRot, curOff, openRot, openOff, oRot, oOff));
    KT_CHECK(!hse::inspRevert(0, nullptr, curOff, openRot, openOff, oRot, oOff));
}

static void testTextCache() {
    hse::TextCache c;
    KT_CHECK(c.update(0, "Slide  0.0 cm"));
    KT_CHECK(!c.update(0, "Slide  0.0 cm"));     // same text: no SetText
    KT_CHECK(c.update(0, "Slide  0.1 cm"));
    KT_CHECK(c.update(1, "Slide  0.1 cm"));      // a different row has its own cache
    KT_CHECK(!c.update(1, "Slide  0.1 cm"));
    KT_CHECK(!c.update(-1, "x") && !c.update(6, "x") && !c.update(0, nullptr));
    c.clear();
    KT_CHECK(c.update(0, "Slide  0.1 cm"));      // cleared: the next text is pushed again
    char longText[100];
    memset(longText, 'a', sizeof longText);
    longText[99] = '\0';
    KT_CHECK(c.update(2, longText));
    KT_CHECK(!c.update(2, longText));            // an over-long text is compared as stored (cut): no endless SetText
}

static hse::ScrubStep runPx(hse::Scrub& s, int dx, bool shift = false, bool ctrl = false) {
    return s.step(s.lastX + dx, shift, ctrl);
}

static void testScrubPxToValue() {
    hse::Scrub s;
    KT_CHECK(s.begin(hse::IR_DEPTH, 100, -0.11f));
    KT_CHECK(s.active && s.row == hse::IR_DEPTH && s.pressX == 100 && closeF(s.open, -0.11f));
    hse::ScrubStep st = runPx(s, 10);
    KT_CHECK(st.changed && closeF(st.value, -0.10f, 1e-5f) && st.clampSide == 0 && !st.clampedNow);   // 1 px = 1 mm
    st = runPx(s, -30);
    KT_CHECK(st.changed && closeF(st.value, -0.13f, 1e-5f));
    KT_CHECK_EQ(s.frames, 2);
    s.cancel();
    KT_CHECK(!s.active);

    hse::Scrub r;                                    // rotation: 1 px = 0.5 deg
    KT_CHECK(r.begin(hse::IR_TILT, 0, 30.0f));
    st = runPx(r, 10);
    KT_CHECK(st.changed && closeF(st.value, 35.0f, 1e-4f));
    st = runPx(r, -21);
    KT_CHECK(st.changed && closeF(st.value, 24.5f, 1e-4f));
}

static void testScrubShift() {
    hse::Scrub s;
    KT_CHECK(s.begin(hse::IR_SLIDE, 0, 0.0f));
    hse::ScrubStep st = runPx(s, 10, true);          // x0.2: 10 px = 2 mm
    KT_CHECK(st.changed && closeF(st.value, 0.002f, 1e-6f));
    hse::Scrub r;
    KT_CHECK(r.begin(hse::IR_ROLL, 0, 0.0f));
    st = runPx(r, 10, true);                         // 10 px = 1 deg
    KT_CHECK(st.changed && closeF(st.value, 1.0f, 1e-4f));
    // Shift toggled mid drag only changes the speed from that tick on: no jump
    hse::Scrub m;
    KT_CHECK(m.begin(hse::IR_HEIGHT, 0, 0.0f));
    st = runPx(m, 10);
    KT_CHECK(closeF(st.value, 0.010f, 1e-6f));
    st = runPx(m, 0, true);                          // modifier flips with no movement: nothing moves
    KT_CHECK(!st.changed && closeF(m.value, 0.010f, 1e-6f));
    st = runPx(m, 10, true);
    KT_CHECK(st.changed && closeF(st.value, 0.012f, 1e-6f));
}

static void testScrubCtrlSnap() {
    hse::Scrub s;
    KT_CHECK(s.begin(hse::IR_DEPTH, 0, -0.11f));
    hse::ScrubStep st = runPx(s, 10, false, true);   // 10 px = 1 cm exactly, in one tick
    KT_CHECK(st.changed && closeF(st.value, -0.10f, 1e-6f));
    // carry across ticks: 4 px + 4 px does nothing, the third 4 px crosses 1 cm, 2 mm stay in the carry
    hse::Scrub c;
    KT_CHECK(c.begin(hse::IR_DEPTH, 0, 0.0f));
    st = runPx(c, 4, false, true);
    KT_CHECK(!st.changed && closeF(c.value, 0.0f));
    st = runPx(c, 4, false, true);
    KT_CHECK(!st.changed);
    st = runPx(c, 4, false, true);
    KT_CHECK(st.changed && closeF(st.value, 0.01f, 1e-6f) && closeF(c.acc, 0.002f, 1e-5f));
    // ten 1 px ticks add up to one step (the float sum of 0.001 must not stall)
    hse::Scrub t;
    KT_CHECK(t.begin(hse::IR_SLIDE, 0, 0.0f));
    int moved = 0;
    for (int i = 0; i < 10; ++i) moved += runPx(t, 1, false, true).changed ? 1 : 0;
    KT_CHECK_EQ(moved, 1);
    KT_CHECK(closeF(t.value, 0.01f, 1e-6f));
    // negative direction truncates toward zero too
    hse::Scrub n;
    KT_CHECK(n.begin(hse::IR_SLIDE, 0, 0.0f));
    st = runPx(n, -25, false, true);                 // 2.5 cm -> 2 cm applied, 5 mm carried
    KT_CHECK(st.changed && closeF(st.value, -0.02f, 1e-6f) && closeF(n.acc, -0.005f, 1e-5f));
    // releasing Ctrl drops the carry; the free move that follows is exact
    st = runPx(n, 3, false, false);
    KT_CHECK(closeF(n.acc, 0.0f) && closeF(st.value, -0.017f, 1e-6f));

    hse::Scrub r;                                    // rotation: 10 px = 5 deg, 9 px = nothing (4.5 deg carried)
    KT_CHECK(r.begin(hse::IR_TURN, 0, 0.0f));
    st = runPx(r, 9, false, true);
    KT_CHECK(!st.changed && closeF(r.acc, 4.5f, 1e-4f));
    st = runPx(r, 1, false, true);
    KT_CHECK(st.changed && closeF(st.value, 5.0f, 1e-4f));
    // Ctrl + Shift: 50 px x 0.1 = 5 deg
    hse::Scrub b;
    KT_CHECK(b.begin(hse::IR_TILT, 0, 0.0f));
    st = runPx(b, 50, true, true);
    KT_CHECK(st.changed && closeF(st.value, 5.0f, 1e-4f));
}

static void testScrubWindowAndLimits() {
    hse::Scrub s;
    KT_CHECK(s.begin(hse::IR_DEPTH, 0, 0.0f));
    KT_CHECK(closeF(s.lo, -0.30f) && closeF(s.hi, 0.30f) && !s.loHard && !s.hiHard);
    hse::ScrubStep st = runPx(s, 250);               // 25 cm: inside
    KT_CHECK(st.changed && st.clampSide == 0 && !st.clampedNow);
    st = runPx(s, 100);                              // would be 35 cm: pinned at the +30 cm window edge
    KT_CHECK(st.changed && closeF(st.value, 0.30f, 1e-6f) && st.clampSide == 1 && st.clampedNow && !st.hardLimit);
    st = runPx(s, 50);                               // still pinned: no change, and the WARN fires only once per scrub
    KT_CHECK(!st.changed && st.clampSide == 1 && !st.clampedNow);
    st = runPx(s, -100);                             // no dead zone: moving back moves the value at once
    KT_CHECK(st.changed && closeF(st.value, 0.20f, 1e-6f) && st.clampSide == 0);
    st = runPx(s, -700);                             // the low end
    KT_CHECK(st.changed && closeF(st.value, -0.30f, 1e-6f) && st.clampSide == -1 && !st.clampedNow);
    // rotation window +-180 around the value at the start
    hse::Scrub r;
    KT_CHECK(r.begin(hse::IR_TILT, 0, -150.0f));
    KT_CHECK(closeF(r.lo, -330.0f) && closeF(r.hi, 30.0f));
    st = runPx(r, 1000);
    KT_CHECK(closeF(st.value, 30.0f) && st.clampSide == 1 && st.clampedNow && !st.hardLimit);
    // hard limits: 3 m / 720 deg cut the window
    hse::Scrub h;
    KT_CHECK(h.begin(hse::IR_HEIGHT, 0, 2.9f));
    KT_CHECK(closeF(h.hi, 3.0f) && h.hiHard && closeF(h.lo, 2.6f) && !h.loHard);
    st = runPx(h, 500);
    KT_CHECK(closeF(st.value, 3.0f) && st.clampSide == 1 && st.hardLimit && st.clampedNow);
    hse::Scrub q;
    KT_CHECK(q.begin(hse::IR_ROLL, 0, -700.0f));
    KT_CHECK(closeF(q.lo, -720.0f) && q.loHard && closeF(q.hi, -520.0f));
    st = runPx(q, -1000);
    KT_CHECK(closeF(st.value, -720.0f) && st.clampSide == -1 && st.hardLimit);
    // an open value past the limit (a hand edited file): the window collapses onto the limit and the first move lands on it
    hse::Scrub o;
    KT_CHECK(o.begin(hse::IR_TURN, 0, 1000.0f));
    KT_CHECK(closeF(o.lo, 720.0f) && closeF(o.hi, 720.0f));
    st = runPx(o, 1);
    KT_CHECK(st.changed && closeF(st.value, 720.0f) && st.hardLimit);
    // the Ctrl carry is dropped when a clamp pins the value
    hse::Scrub k;
    KT_CHECK(k.begin(hse::IR_SLIDE, 0, 2.97f));
    st = runPx(k, 50, false, true);                  // +5 cm snapped, pinned at the 3 m limit
    KT_CHECK(closeF(st.value, 3.0f, 1e-6f) && st.clampSide == 1 && st.hardLimit && k.acc == 0.0f);
}

static void testScrubGuards() {
    hse::Scrub s;
    hse::ScrubStep st = s.step(500, false, false);   // no scrub running (a press outside every row never began one): ignored
    KT_CHECK(!st.changed && st.clampSide == 0 && !s.active && s.frames == 0);
    KT_CHECK(!s.begin(-1, 0, 0.0f) && !s.active);
    KT_CHECK(!s.begin(hse::IR_COUNT, 0, 0.0f) && !s.active);
    KT_CHECK(!s.begin(hse::IR_DEPTH, 0, std::nanf("")) && !s.active);
    KT_CHECK(!s.begin(hse::IR_DEPTH, 0, INFINITY) && !s.active);
    KT_CHECK(!s.begin(hse::IR_DEPTH, 0, -INFINITY) && !s.active);
    KT_CHECK(s.begin(hse::IR_DEPTH, 40, 0.1f));
    KT_CHECK(!s.begin(hse::IR_TILT, 0, 5.0f));       // a second begin while running does nothing
    KT_CHECK(s.row == hse::IR_DEPTH && closeF(s.open, 0.1f));
    // release without a move: the value never changes (every tick at the press x)
    for (int i = 0; i < 20; ++i) {
        st = s.step(40, i & 1, i & 2);
        KT_CHECK(!st.changed);
    }
    KT_CHECK(closeF(s.value, 0.1f) && s.value == s.open && s.frames == 20);
    // a very large jump still stays inside the window
    st = s.step(40 + 2000000000, false, false);
    KT_CHECK(st.changed && closeF(st.value, 0.4f) && st.clampSide == 1);
    s.cancel();
    KT_CHECK(!s.active);
    st = s.step(0, false, false);
    KT_CHECK(!st.changed);
    // begin after cancel starts clean
    KT_CHECK(s.begin(hse::IR_SLIDE, 7, 0.0f) && s.frames == 0 && s.acc == 0.0f && !s.clampWarned);
}

static void testCompactSlot() {
    hse::HseRect strip{161, 134, 163, 284};
    const int availH = 336;                          // the view pane height
    // without the inspector the compact slot IS the vanilla stripSlot
    for (int row = 0; row < hse::kStripRowsBase; ++row) KT_CHECK(sameRect(hse::compactSlot(strip, availH, row, hse::kStripRowsBase), hse::stripSlot(strip, row)));
    // a scaled UI (ultrawide / larger window): same rule
    hse::HseRect big{200, 268, 326, 568};
    for (int row = 0; row < hse::kStripRowsBase; ++row) KT_CHECK(sameRect(hse::compactSlot(big, 672, row, hse::kStripRowsBase), hse::stripSlot(big, row)));
    // with the inspector: 15 rows, ordered, no overlap, all inside the strip's vertical room
    for (int pass = 0; pass < 2; ++pass) {
        const hse::HseRect& st = pass ? big : strip;
        const int room = pass ? 672 : availH;
        int prevBottom = st.y;
        for (int row = 0; row < hse::kStripRowsInsp; ++row) {
            const hse::HseRect r = hse::compactSlot(st, room, row, hse::kStripRowsInsp);
            KT_CHECK(r.h >= 16);                     // tall enough for one line of the 16 px font
            KT_CHECK(r.y >= prevBottom);
            KT_CHECK(r.y + r.h <= st.y + room);
            KT_CHECK(r.x == st.x && r.w == st.w);
            prevBottom = r.y + r.h;
        }
    }
    KT_CHECK_EQ(hse::compactSlot(strip, availH, 0, 15).y, 134);
    KT_CHECK_EQ(hse::compactSlot(strip, availH, 14, 15).y + hse::compactSlot(strip, availH, 14, 15).h, 134 + 14 * 22 + 19);
    // degenerate input never produces a negative or oversized slot
    const hse::HseRect z = hse::compactSlot(hse::HseRect{0, 0, 0, 0}, 0, 3, 15);
    KT_CHECK(z.h >= 1 && z.w == 0);
    const hse::HseRect d = hse::compactSlot(strip, 0, 2, 15);   // availH unknown: the vanilla pitch
    KT_CHECK(sameRect(d, hse::stripSlot(strip, 2)));
}

static void testInspRects() {
    hse::HseRect strip{161, 134, 163, 284};
    for (int i = 0; i < hse::IR_COUNT; ++i) {
        const hse::HseRect slot = hse::compactSlot(strip, 336, hse::kStripRowsBase + i, hse::kStripRowsInsp);
        hse::HseRect sc, rs;
        hse::inspRects(slot, &sc, &rs);
        KT_CHECK(sc.x == slot.x && sc.y == slot.y && sc.h == slot.h && rs.y == slot.y && rs.h == slot.h);
        KT_CHECK(sc.w > 100);                        // room for "* Height  -11.0 cm"
        KT_CHECK(rs.w >= 18 && rs.w <= slot.w / 3);
        KT_CHECK(sc.x + sc.w < rs.x);                // a gap between the two buttons
        KT_CHECK(rs.x + rs.w == slot.x + slot.w);    // the reset button ends at the strip's right edge
    }
    // no two of the 15 weapon-mode rows (12 inspector buttons + 9 one/two button rows) overlap
    hse::HseRect all[40];
    int n = 0;
    for (int row = 0; row < hse::kStripRowsBase; ++row) {
        const hse::HseRect slot = hse::compactSlot(strip, 336, row, hse::kStripRowsInsp);
        if (row == 0 || row == 2 || row == 3) all[n++] = slot;
        else {
            all[n++] = hse::half(slot, 0, 6);
            all[n++] = hse::half(slot, 1, 6);
        }
    }
    for (int i = 0; i < hse::IR_COUNT; ++i) {
        hse::HseRect sc, rs;
        hse::inspRects(hse::compactSlot(strip, 336, hse::kStripRowsBase + i, hse::kStripRowsInsp), &sc, &rs);
        all[n++] = sc;
        all[n++] = rs;
    }
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const bool overlap = all[i].x < all[j].x + all[j].w && all[j].x < all[i].x + all[i].w && all[i].y < all[j].y + all[j].h && all[j].y < all[i].y + all[i].h;
            KT_CHECK(!overlap);
        }
    }
    // a narrow strip still splits without negative widths
    hse::HseRect sc, rs;
    hse::inspRects(hse::HseRect{0, 0, 30, 20}, &sc, &rs);
    KT_CHECK(sc.w >= 0 && rs.w >= 0 && rs.w <= 10);
    hse::inspRects(hse::HseRect{0, 0, 0, 0}, &sc, &rs);
    KT_CHECK(sc.w == 0 && rs.w == 0);
    hse::inspRects(hse::HseRect{0, 0, 100, 20}, nullptr, &rs);   // null guards
}

static void testLeftMirroredStatus() {
    const uint8_t none[3] = {2, 6, 0}, rotBit[3] = {static_cast<uint8_t>(1 | hse::kSrcMirrorBit), 1, 0}, offBit[3] = {0, static_cast<uint8_t>(4 | hse::kSrcMirrorBit), 0};
    KT_CHECK(!hse::leftMirrored(true, none));
    KT_CHECK(hse::leftMirrored(true, rotBit) && hse::leftMirrored(true, offBit));
    KT_CHECK(!hse::leftMirrored(false, rotBit));     // the right hand is never "mirrored"
    KT_CHECK(!hse::leftMirrored(true, nullptr));
    KT_CHECK(hse::kSrcMirrorBit != 4 && hse::kSrcMirrorBit != 3);   // no clash with the body bit (4) or the level numbers (0..2)
    char t[160];
    hse::idleStatusText(t, sizeof t, false, false);
    KT_CHECK(!strcmp(t, hse::helpLine()));
    hse::idleStatusText(t, sizeof t, true, false);
    KT_CHECK(!strncmp(t, "* ", 2) && !strcmp(t + 2, hse::helpLine()));
    hse::idleStatusText(t, sizeof t, false, true);
    KT_CHECK(!strncmp(t, "Left: mirrored from right", 25));
    KT_CHECK(strstr(t, hse::helpLine()) != nullptr);
    hse::idleStatusText(t, sizeof t, true, true);
    KT_CHECK(!strncmp(t, "* Left: mirrored from right", 27));
    char small[10];
    hse::idleStatusText(small, sizeof small, true, true);
    KT_CHECK(strlen(small) == 9);
    hse::idleStatusText(nullptr, 10, true, true);
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
    testInspRowBasics();
    testInspItemBudget();
    testInspRowText();
    testInspDiffersAndMatches();
    testInspRevert();
    testTextCache();
    testScrubPxToValue();
    testScrubShift();
    testScrubCtrlSnap();
    testScrubWindowAndLimits();
    testScrubGuards();
    testCompactSlot();
    testInspRects();
    testLeftMirroredStatus();
    return kt::Report("hse_view");
}

