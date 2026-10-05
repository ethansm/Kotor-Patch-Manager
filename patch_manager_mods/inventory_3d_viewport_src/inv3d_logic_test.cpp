// T1 host tests for Inv3dLogic.h (the pure logic of Inventory3DViewport.cpp; doc 20).
// Build/run: python3 scripts/run_tests.py -k inv3d
//
// Where a test pins down behaviour that looks questionable rather than intended it is marked "CHARACTERIZATION" (the
// current behaviour is asserted so a later change is a conscious decision, not an accident).
#include "Inv3dLogic.h"
#include "check.h"
#include <cmath>
#include <string>
#include <vector>

using namespace inv3d;

static bool approx(float a, float b, float eps = 1e-4f) { return std::fabs(a - b) <= eps; }

// ---- INI helpers -------------------------------------------------------------------------------------------------
static IniResult applyLine(Cfg& c, const char* text) {
    char line[600]; snprintf(line, sizeof line, "%s", text);
    trim(line);
    char* k = nullptr; char* v = nullptr;
    IniLine sp = splitIniLine(line, &k, &v);
    if (sp != IniLineKeyValue) return IniUnknown;
    return parseIniLine(c, k, v);
}
struct IniStats { int ok = 0, bad = 0; int clamped = 0; };
// Whole-file driver: the DLL's loadIni loop (fgets line[256], trim, split, parse, clamp) minus the logging.
static Cfg loadText(const std::string& text, IniStats* st = nullptr) {
    Cfg c; cfgDefaults(&c);
    FILE* f = tmpfile();
    fwrite(text.data(), 1, text.size(), f); rewind(f);
    char line[256]; IniStats s;
    while (fgets(line, sizeof(line), f)) {
        if (drainLongLine(line, sizeof(line), f)) { ++s.bad; continue; }   // F31: a >255-char line is rejected once
        trim(line);
        char* k = nullptr; char* v = nullptr;
        IniLine sp = splitIniLine(line, &k, &v);
        if (sp == IniLineSkip) continue;
        if (sp == IniLineNoEquals) { ++s.bad; continue; }
        IniResult r = parseIniLine(c, k, v);
        if (r == IniUnknown) { ++s.bad; continue; }
        if (r == IniBad) ++s.bad; else ++s.ok;
    }
    fclose(f);
    s.clamped = clampCfg(c);
    if (st) *st = s;
    return c;
}

// ---- trim / parseFloats ------------------------------------------------------------------------------------------
static void testTrim() {
    auto t = [](const char* in) { char b[64]; snprintf(b, sizeof b, "%s", in); trim(b); return std::string(b); };
    KT_CHECK_EQ(t("").c_str(), "");
    KT_CHECK_EQ(t("   ").c_str(), "");
    KT_CHECK_EQ(t("abc").c_str(), "abc");
    KT_CHECK_EQ(t("  abc").c_str(), "abc");
    KT_CHECK_EQ(t("abc  ").c_str(), "abc");
    KT_CHECK_EQ(t("  a b  ").c_str(), "a b");
    KT_CHECK_EQ(t("\ta\t\r\n").c_str(), "a");           // an fgets line keeps its newline
    KT_CHECK_EQ(t("\xff x \xff").c_str(), "\xff x \xff");   // high bytes are not whitespace (unsigned char cast)
}

static void testParseFloats() {
    float o[6] = {-9, -9, -9, -9, -9, -9};
    KT_CHECK(parseFloats("1.5", o, 1)); KT_CHECK(approx(o[0], 1.5f));
    KT_CHECK(parseFloats("  -2.25  ", o, 1)); KT_CHECK(approx(o[0], -2.25f));
    KT_CHECK(parseFloats("1e2", o, 1)); KT_CHECK(approx(o[0], 100.f));
    KT_CHECK(!parseFloats("", o, 1));
    KT_CHECK(!parseFloats("abc", o, 1));
    KT_CHECK(!parseFloats("1.5x", o, 1));
    KT_CHECK(!parseFloats("1,2", o, 1));
    KT_CHECK(!parseFloats("nan", o, 1));
    KT_CHECK(!parseFloats("inf", o, 1));
    KT_CHECK(!parseFloats("1e999", o, 1));
    KT_CHECK(parseFloats("1,2", o, 2)); KT_CHECK(approx(o[0], 1.f) && approx(o[1], 2.f));
    KT_CHECK(parseFloats(" 1 , 2 ", o, 2));
    KT_CHECK(!parseFloats("1", o, 2));
    KT_CHECK(!parseFloats("1 2", o, 2));
    KT_CHECK(!parseFloats("1,2,3", o, 2));
    KT_CHECK(!parseFloats("1,nan", o, 2));
    KT_CHECK(parseFloats("1,2,3", o, 3)); KT_CHECK(approx(o[2], 3.f));
    KT_CHECK(parseFloats("1,2,3,4,5,6", o, 6)); KT_CHECK(approx(o[5], 6.f));
    KT_CHECK(!parseFloats("1,2,3,4,5", o, 6));
    KT_CHECK(parseFloats("0x10", o, 1)); KT_CHECK(approx(o[0], 16.f));   // CHARACTERIZATION: strtof takes C99 hex floats
}

static void testSplitIniLine() {
    auto sp = [](const char* in, std::string* k, std::string* v) {
        char line[600]; snprintf(line, sizeof line, "%s", in);
        char *kk = nullptr, *vv = nullptr;
        IniLine r = splitIniLine(line, &kk, &vv);
        if (r == IniLineKeyValue) { *k = kk; *v = vv; }
        return r;
    };
    std::string k, v;
    KT_CHECK_EQ(sp("", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp("# c", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp("; c", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp("novalue", &k, &v), IniLineNoEquals);
    KT_CHECK_EQ(sp("[Section]", &k, &v), IniLineNoEquals);
    KT_CHECK_EQ(sp("enabled=1", &k, &v), IniLineKeyValue);
    KT_CHECK_EQ(k.c_str(), "enabled"); KT_CHECK_EQ(v.c_str(), "1");
    sp("  Enabled  =  1  ", &k, &v); KT_CHECK_EQ(k.c_str(), "enabled"); KT_CHECK_EQ(v.c_str(), "1");
    sp("RIG=Inv3d_Light", &k, &v); KT_CHECK_EQ(k.c_str(), "rig"); KT_CHECK_EQ(v.c_str(), "Inv3d_Light");   // key lower-cased, value case kept
    sp("enabled=1 ; on", &k, &v); KT_CHECK_EQ(v.c_str(), "1");
    sp("enabled=1 # on", &k, &v); KT_CHECK_EQ(v.c_str(), "1");
    sp("rig=a#b", &k, &v); KT_CHECK_EQ(v.c_str(), "a");   // CHARACTERIZATION: a value can never contain '#' or ';'
    sp("a=b=c", &k, &v); KT_CHECK_EQ(k.c_str(), "a"); KT_CHECK_EQ(v.c_str(), "b=c");
    sp("=v", &k, &v); KT_CHECK_EQ(k.c_str(), ""); KT_CHECK_EQ(v.c_str(), "v");
    char line[] = "no equals here"; char *kk = nullptr, *vv = nullptr;
    splitIniLine(line, &kk, &vv); KT_CHECK_EQ(line, "no equals here");   // rejected line left untouched for the caller's log
}

// ---- defaults ----------------------------------------------------------------------------------------------------
static void testCfgDefaults() {
    Cfg c; cfgDefaults(&c);
    KT_CHECK_EQ(c.enabled, 1); KT_CHECK_EQ(c.rig, "inv3d_light"); KT_CHECK_EQ(c.onlyWeaponsArmor, 0);
    KT_CHECK(approx(c.spin, 70) && approx(c.dragDegPx, 0.5f) && approx(c.pitchClamp, 80) && approx(c.idleResume, 1.5f));
    KT_CHECK_EQ(c.fitScale, 0);
    KT_CHECK(approx(c.margin, 1.15f) && approx(c.distMin, 0.25f) && approx(c.distMax, 4.0f));
    KT_CHECK(approx(c.fov, 22.726f)); KT_CHECK_EQ(c.fovAxis, 'v');
    for (int i = 0; i < 3; ++i) {
        KT_CHECK(c.camPos[i] == kRigCamPos[i]); KT_CHECK(c.camFwd[i] == kRigCamFwd[i]); KT_CHECK(c.camUp[i] == kRigCamUp[i]);
    }
    KT_CHECK(approx(c.camPos[0], 0.983975f) && approx(c.camFwd[0], -1.f) && approx(c.camUp[2], 1.f));
    KT_CHECK(approx(c.armorBox[2], 0.95f) && approx(c.armorBox[5], 1.9f) && c.armorBox[0] == 0);
    KT_CHECK_EQ(c.armorSpin, 1); KT_CHECK_EQ(c.hitFlipY, 0);
    KT_CHECK(c.screenW == 3840 && c.screenH == 1600 && !c.screenExplicit);
    KT_CHECK_EQ(c.bevels, 1); KT_CHECK_EQ(c.wheelZoom, 1);
    KT_CHECK(approx(c.wheelStep, 0.9f) && approx(c.zoomDrag, 0.004f) && approx(c.rollDegPx, 0.5f) && approx(c.zoomMin, 0.3f) && approx(c.zoomMax, 3.0f));
    KT_CHECK(approx(c.fillRef, 0.6f) && approx(c.fillExp, 0.35f) && approx(c.fillMin, 0.4f));
    bool anySet = false; for (int i = 0; i < 256; ++i) anySet |= c.itype[i].set || c.row[i].set;
    KT_CHECK(!anySet);
    // defaults survive an empty / missing-key file and are idempotent through clampCfg
    IniStats st; Cfg d = loadText("", &st);
    KT_CHECK_EQ(st.ok, 0); KT_CHECK_EQ(st.bad, 0); KT_CHECK_EQ(st.clamped, 0);
    KT_CHECK(memcmp(&d.fov, &c.fov, sizeof(float)) == 0 && approx(d.margin, c.margin) && approx(d.zoomMin, 0.3f) && approx(d.distMax, 4.f));
    // a reset of a dirtied struct really restores everything (including the override tables)
    Cfg e; cfgDefaults(&e); e.itype[5].set = true; e.row[7].set = true; e.enabled = 0; e.camPos[0] = 9;
    cfgDefaults(&e); KT_CHECK(!e.itype[5].set && !e.row[7].set && e.enabled == 1 && e.camPos[0] == kRigCamPos[0]);
}

// ---- parseIniLine ------------------------------------------------------------------------------------------------
static void testParseIniLine() {
    Cfg c; cfgDefaults(&c);
    KT_CHECK_EQ(applyLine(c, "enabled = 0"), IniOk); KT_CHECK_EQ(c.enabled, 0);
    KT_CHECK_EQ(applyLine(c, "enabled = 2.9"), IniOk); KT_CHECK_EQ(c.enabled, 2);   // (int) truncation
    KT_CHECK_EQ(applyLine(c, "enabled = -1.9"), IniOk); KT_CHECK_EQ(c.enabled, -1);
    KT_CHECK_EQ(applyLine(c, "enabled = x"), IniBad); KT_CHECK_EQ(c.enabled, -1);   // unchanged on a bad value
    KT_CHECK_EQ(applyLine(c, "rig = my_rig"), IniOk); KT_CHECK_EQ(c.rig, "my_rig");
    KT_CHECK_EQ(applyLine(c, "rig ="), IniBad); KT_CHECK_EQ(c.rig, "my_rig");        // empty rig rejected
    std::string r63(63, 'a'), r64(64, 'b');
    KT_CHECK_EQ(applyLine(c, ("rig = " + r63).c_str()), IniOk); KT_CHECK_EQ(c.rig, r63.c_str());   // 63 chars fits the 64-byte buffer
    KT_CHECK_EQ(applyLine(c, ("rig = " + r64).c_str()), IniBad); KT_CHECK_EQ(c.rig, r63.c_str());
    KT_CHECK_EQ(applyLine(c, "only_weapons_armor = 1"), IniOk); KT_CHECK_EQ(c.onlyWeaponsArmor, 1);
    KT_CHECK_EQ(applyLine(c, "spin_deg_s = 12.5"), IniOk); KT_CHECK(approx(c.spin, 12.5f));
    KT_CHECK_EQ(applyLine(c, "drag_deg_per_px = 0.25"), IniOk); KT_CHECK(approx(c.dragDegPx, 0.25f));
    KT_CHECK_EQ(applyLine(c, "pitch_clamp = 45"), IniOk); KT_CHECK(approx(c.pitchClamp, 45));
    KT_CHECK_EQ(applyLine(c, "idle_resume_s = 3"), IniOk); KT_CHECK(approx(c.idleResume, 3));
    // enumerations
    KT_CHECK_EQ(applyLine(c, "fit_mode = scale"), IniOk); KT_CHECK_EQ(c.fitScale, 1);
    KT_CHECK_EQ(applyLine(c, "fit_mode = DISTANCE"), IniOk); KT_CHECK_EQ(c.fitScale, 0);   // case-insensitive
    KT_CHECK_EQ(applyLine(c, "fit_mode = Scale"), IniOk); KT_CHECK_EQ(c.fitScale, 1);
    KT_CHECK_EQ(applyLine(c, "fit_mode = bogus"), IniBad); KT_CHECK_EQ(c.fitScale, 1);
    KT_CHECK_EQ(applyLine(c, "fit_mode ="), IniBad);
    KT_CHECK_EQ(applyLine(c, "fov_axis = H"), IniOk); KT_CHECK_EQ(c.fovAxis, 'h');
    KT_CHECK_EQ(applyLine(c, "fov_axis = v"), IniOk); KT_CHECK_EQ(c.fovAxis, 'v');
    KT_CHECK_EQ(applyLine(c, "fov_axis = x"), IniBad); KT_CHECK_EQ(c.fovAxis, 'v');
    KT_CHECK_EQ(applyLine(c, "fov_axis = vv"), IniBad);
    KT_CHECK_EQ(applyLine(c, "fit_margin = 1.5"), IniOk); KT_CHECK(approx(c.margin, 1.5f));
    KT_CHECK_EQ(applyLine(c, "dist_min = 0.5"), IniOk); KT_CHECK(approx(c.distMin, 0.5f));
    KT_CHECK_EQ(applyLine(c, "dist_max = 2"), IniOk); KT_CHECK(approx(c.distMax, 2));
    KT_CHECK_EQ(applyLine(c, "fov_deg = 30"), IniOk); KT_CHECK(approx(c.fov, 30));
    // vectors
    KT_CHECK_EQ(applyLine(c, "cam_pos = 1, 2, 3"), IniOk); KT_CHECK(c.camPos[0] == 1 && c.camPos[1] == 2 && c.camPos[2] == 3);
    KT_CHECK_EQ(applyLine(c, "cam_pos = 1, 2"), IniBad); KT_CHECK(c.camPos[2] == 3);
    KT_CHECK_EQ(applyLine(c, "cam_pos = 1, 2, 3, 4"), IniBad);
    KT_CHECK_EQ(applyLine(c, "cam_fwd = 0,1,0"), IniOk); KT_CHECK(c.camFwd[0] == 0 && c.camFwd[1] == 1);
    KT_CHECK_EQ(applyLine(c, "cam_up = 0,0,-1"), IniOk); KT_CHECK(c.camUp[2] == -1);
    KT_CHECK_EQ(applyLine(c, "armor_box = 1,2,3,4,5,6"), IniOk); KT_CHECK(c.armorBox[0] == 1 && c.armorBox[5] == 6);
    KT_CHECK_EQ(applyLine(c, "armor_box = 1,2,3,4,5"), IniBad); KT_CHECK(c.armorBox[5] == 6);
    KT_CHECK_EQ(applyLine(c, "armor_spin = 0"), IniOk); KT_CHECK_EQ(c.armorSpin, 0);
    KT_CHECK_EQ(applyLine(c, "hit_flip_y = 1"), IniOk); KT_CHECK_EQ(c.hitFlipY, 1);
    KT_CHECK_EQ(applyLine(c, "bevels = 0"), IniOk); KT_CHECK_EQ(c.bevels, 0);
    KT_CHECK_EQ(applyLine(c, "wheel_zoom = 0"), IniOk); KT_CHECK_EQ(c.wheelZoom, 0);
    KT_CHECK_EQ(applyLine(c, "wheel_step = 0.8"), IniOk); KT_CHECK(approx(c.wheelStep, 0.8f));
    KT_CHECK_EQ(applyLine(c, "zoom_drag = 0.01"), IniOk); KT_CHECK(approx(c.zoomDrag, 0.01f));
    KT_CHECK_EQ(applyLine(c, "roll_deg_per_px = 2"), IniOk); KT_CHECK(approx(c.rollDegPx, 2));
    KT_CHECK_EQ(applyLine(c, "zoom_min = 0.5"), IniOk); KT_CHECK(approx(c.zoomMin, 0.5f));
    KT_CHECK_EQ(applyLine(c, "zoom_max = 2"), IniOk); KT_CHECK(approx(c.zoomMax, 2));
    KT_CHECK_EQ(applyLine(c, "fill_ref = 0.7"), IniOk); KT_CHECK(approx(c.fillRef, 0.7f));
    KT_CHECK_EQ(applyLine(c, "fill_exp = 0.5"), IniOk); KT_CHECK(approx(c.fillExp, 0.5f));
    KT_CHECK_EQ(applyLine(c, "fill_min = 0.3"), IniOk); KT_CHECK(approx(c.fillMin, 0.3f));
    // screen_w / screen_h set screenExplicit only when accepted
    Cfg s; cfgDefaults(&s);
    KT_CHECK_EQ(applyLine(s, "screen_w = abc"), IniBad); KT_CHECK(!s.screenExplicit);
    KT_CHECK_EQ(applyLine(s, "screen_w = 2560"), IniOk); KT_CHECK(s.screenW == 2560 && s.screenExplicit);
    Cfg s2; cfgDefaults(&s2);
    KT_CHECK_EQ(applyLine(s2, "screen_h = 1080"), IniOk); KT_CHECK(s2.screenH == 1080 && s2.screenExplicit);
    // unknown keys (including keys of the sibling Equip3D mod, and a case-sensitive miss)
    KT_CHECK_EQ(applyLine(c, "stage = 5"), IniUnknown);
    KT_CHECK_EQ(applyLine(c, "frobnicate = 1"), IniUnknown);
    KT_CHECK_EQ(applyLine(c, "=1"), IniUnknown);
}

static void testOverrides() {
    Cfg c; cfgDefaults(&c);
    // itype.N / row.N = rx,ry,rz,pitch[,zoom]; zoom defaults to 1 when only 4 values are given
    KT_CHECK_EQ(applyLine(c, "itype.3 = 10, 20, 30, 40, 0.5"), IniOk);
    KT_CHECK(c.itype[3].set && approx(c.itype[3].v[0], 10) && approx(c.itype[3].v[3], 40) && approx(c.itype[3].v[4], 0.5f));
    KT_CHECK_EQ(applyLine(c, "row.255 = 1,2,3,4"), IniOk);
    KT_CHECK(c.row[255].set && approx(c.row[255].v[3], 4) && approx(c.row[255].v[4], 1.0f));
    KT_CHECK_EQ(applyLine(c, "row.0 = 0,0,0,0,2"), IniOk); KT_CHECK(c.row[0].set && approx(c.row[0].v[4], 2));
    KT_CHECK(!c.itype[4].set && !c.row[3].set);
    // rejects: index out of range / non-numeric / empty / negative, wrong value count
    KT_CHECK_EQ(applyLine(c, "row.256 = 1,2,3,4"), IniBad);
    KT_CHECK_EQ(applyLine(c, "row.-1 = 1,2,3,4"), IniBad);
    KT_CHECK_EQ(applyLine(c, "row. = 1,2,3,4"), IniBad);
    KT_CHECK_EQ(applyLine(c, "row.1x = 1,2,3,4"), IniBad);
    KT_CHECK_EQ(applyLine(c, "itype.abc = 1,2,3,4"), IniBad);
    KT_CHECK_EQ(applyLine(c, "itype.9 = 1,2,3"), IniBad); KT_CHECK(!c.itype[9].set);
    KT_CHECK_EQ(applyLine(c, "itype.9 = 1,2,3,4,5,6"), IniBad); KT_CHECK(!c.itype[9].set);
    KT_CHECK_EQ(applyLine(c, "itype.9 = 1,2,x,4"), IniBad); KT_CHECK(!c.itype[9].set);
    // CHARACTERIZATION: strtol accepts a leading '+' and surrounding-free decimal only; leading zeros are decimal
    KT_CHECK_EQ(applyLine(c, "row.+7 = 1,2,3,4"), IniOk); KT_CHECK(c.row[7].set);
    KT_CHECK_EQ(applyLine(c, "row.012 = 1,2,3,4"), IniOk); KT_CHECK(c.row[12].set);
    // keys that merely start with the prefixes: "rowx.1" is unknown, "row" alone is unknown
    KT_CHECK_EQ(applyLine(c, "rowx.1 = 1,2,3,4"), IniUnknown);
    KT_CHECK_EQ(applyLine(c, "row = 1,2,3,4"), IniUnknown);
    KT_CHECK_EQ(applyLine(c, "itype = 1,2,3,4"), IniUnknown);
    // a failed 5-value parse then a successful 4-value parse leaves a[4] = 1 (not stale data)
    Cfg d; cfgDefaults(&d);
    KT_CHECK_EQ(applyLine(d, "row.5 = 7,8,9,10"), IniOk); KT_CHECK(approx(d.row[5].v[4], 1.f));
}

// ---- clampCfg ----------------------------------------------------------------------------------------------------
static void testClamp() {
    Cfg c; cfgDefaults(&c);
    KT_CHECK_EQ(clampCfg(c), 0);                                   // defaults need no clamping
    KT_CHECK(approx(c.zoomMin, 0.3f) && approx(c.zoomMax, 3.f) && approx(c.fillMin, 0.4f));
    // fit_margin: [1,3] inclusive else reset to 1.15 (logged)
    c.margin = 0.99f; KT_CHECK_EQ(clampCfg(c) & ClampMargin, ClampMargin); KT_CHECK(approx(c.margin, 1.15f));
    c.margin = 3.01f; KT_CHECK_EQ(clampCfg(c) & ClampMargin, ClampMargin); KT_CHECK(approx(c.margin, 1.15f));
    c.margin = 1.0f; KT_CHECK_EQ(clampCfg(c) & ClampMargin, 0); KT_CHECK(c.margin == 1.0f);
    c.margin = 3.0f; KT_CHECK_EQ(clampCfg(c) & ClampMargin, 0); KT_CHECK(c.margin == 3.0f);
    // wheel_step: [0.5, 0.99]
    c.wheelStep = 0.49f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, ClampWheelStep); KT_CHECK(approx(c.wheelStep, 0.9f));
    c.wheelStep = 1.0f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, ClampWheelStep); KT_CHECK(approx(c.wheelStep, 0.9f));
    c.wheelStep = 0.5f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, 0); KT_CHECK(c.wheelStep == 0.5f);
    c.wheelStep = 0.99f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, 0);
    // screen: either dimension < 100 resets BOTH
    c.screenW = 99; c.screenH = 1234; KT_CHECK_EQ(clampCfg(c) & ClampScreen, ClampScreen); KT_CHECK(c.screenW == 3840 && c.screenH == 1600);
    c.screenW = 1234; c.screenH = 99; KT_CHECK_EQ(clampCfg(c) & ClampScreen, ClampScreen); KT_CHECK(c.screenW == 3840 && c.screenH == 1600);
    c.screenW = 100; c.screenH = 100; KT_CHECK_EQ(clampCfg(c) & ClampScreen, 0); KT_CHECK(c.screenW == 100 && c.screenH == 100);
    // fov: [5, 120]
    c.fov = 4.99f; KT_CHECK_EQ(clampCfg(c) & ClampFov, ClampFov); KT_CHECK(approx(c.fov, 22.726f));
    c.fov = 120.01f; KT_CHECK_EQ(clampCfg(c) & ClampFov, ClampFov);
    c.fov = 5; KT_CHECK_EQ(clampCfg(c) & ClampFov, 0); KT_CHECK(c.fov == 5);
    c.fov = 120; KT_CHECK_EQ(clampCfg(c) & ClampFov, 0); KT_CHECK(c.fov == 120);
    // silent clamps
    Cfg d; cfgDefaults(&d);
    d.zoomMin = 0.01f; d.zoomMax = 0.02f; KT_CHECK_EQ(clampCfg(d), 0); KT_CHECK(d.zoomMin == 0.05f && d.zoomMax == 0.05f);   // max follows min
    d.zoomMin = 1; d.zoomMax = 0.5f; clampCfg(d); KT_CHECK(d.zoomMin == 1 && d.zoomMax == 1);
    d.fillRef = 0; clampCfg(d); KT_CHECK(d.fillRef == 0.05f);
    d.fillMin = 0.09f; clampCfg(d); KT_CHECK(approx(d.fillMin, 0.4f));      // out of [0.1,1] -> default 0.4 (not the bound)
    d.fillMin = 1.5f; clampCfg(d); KT_CHECK(approx(d.fillMin, 0.4f));
    d.fillMin = 1.0f; clampCfg(d); KT_CHECK(d.fillMin == 1.0f);
    d.fillMin = 0.1f; clampCfg(d); KT_CHECK(d.fillMin == 0.1f);
    d.pitchClamp = -5; clampCfg(d); KT_CHECK(d.pitchClamp == 0);
    d.idleResume = -1; clampCfg(d); KT_CHECK(d.idleResume == 0);
    d.distMin = 0.01f; d.distMax = 0.02f; clampCfg(d); KT_CHECK(d.distMin == 0.05f && d.distMax == 0.05f);
    d.distMin = 2; d.distMax = 1; clampCfg(d); KT_CHECK(d.distMin == 2 && d.distMax == 2);
    // all bits at once, in log order
    Cfg e; cfgDefaults(&e); e.margin = 9; e.wheelStep = 9; e.screenW = 1; e.fov = 1;
    KT_CHECK_EQ(clampCfg(e), ClampMargin | ClampWheelStep | ClampScreen | ClampFov);
}

// ---- whole-file behaviour ----------------------------------------------------------------------------------------
static void testLoadText() {
    IniStats st;
    Cfg c = loadText("# inv3d.ini\n\nenabled = 1\nspin_deg_s = 30 ; fast\nfit_mode = scale\n[sec]\nbogus = 1\nfov_deg = nope\nwheel_step = 5\nrow.1 = 0,0,0,0\n", &st);
    KT_CHECK_EQ(st.ok, 5); KT_CHECK_EQ(st.bad, 3);                    // [sec] no '=', bogus unknown, fov_deg bad value (wheel_step=5 parses fine, clamped later)
    KT_CHECK(approx(c.spin, 30)); KT_CHECK_EQ(c.fitScale, 1); KT_CHECK(c.row[1].set);
    KT_CHECK(approx(c.fov, 22.726f));                                  // rejected value keeps the default
    KT_CHECK(approx(c.wheelStep, 0.9f)); KT_CHECK_EQ(st.clamped, ClampWheelStep);
    // CRLF line endings
    Cfg d = loadText("spin_deg_s = 11\r\nenabled = 0\r\n", &st);
    KT_CHECK(approx(d.spin, 11)); KT_CHECK_EQ(d.enabled, 0); KT_CHECK_EQ(st.bad, 0);
    // last line without newline
    Cfg e = loadText("spin_deg_s = 12", &st); KT_CHECK(approx(e.spin, 12));
    // later keys override earlier ones
    Cfg f = loadText("spin_deg_s = 1\nspin_deg_s = 2\n"); KT_CHECK(approx(f.spin, 2));
    // F31: the DLL reads with fgets(line[256]) + drainLongLine; a longer line is rejected ONCE (its tail is not parsed as a
    // line of its own). A 300-char rig value: the default rig survives and ONE line counts as rejected.
    std::string longRig = "rig = " + std::string(300, 'q') + "\n";
    Cfg g = loadText(longRig, &st);
    KT_CHECK_EQ(st.ok, 0); KT_CHECK_EQ(st.bad, 1); KT_CHECK_EQ(g.rig, "inv3d_light");
    // a valid line after the long one is still read intact
    Cfg h = loadText(longRig + "spin_deg_s = 13\n", &st);
    KT_CHECK_EQ(st.ok, 1); KT_CHECK_EQ(st.bad, 1); KT_CHECK(approx(h.spin, 13));
}

// ---- drainLongLine (F31) -----------------------------------------------------------------------------------------
static void testDrainLongLine() {
    {   // 600-char line then "key=1\n": exactly 2 logical lines, the second intact
        FILE* f = tmpfile();
        std::string text = std::string(600, 'x') + "\nkey=1\n";
        fwrite(text.data(), 1, text.size(), f); rewind(f);
        char line[256]; int n = 0; bool drained[4] = {false}; std::string second;
        while (fgets(line, sizeof(line), f) && n < 4) {
            drained[n] = drainLongLine(line, sizeof(line), f);
            if (n == 1) second = line;
            ++n;
        }
        fclose(f);
        KT_CHECK_EQ(n, 2); KT_CHECK(drained[0]); KT_CHECK(!drained[1]); KT_CHECK(second == "key=1\n");
    }
    {   // short line: false, untouched, next line still readable
        FILE* f = tmpfile(); fputs("ab=1\nnext=2\n", f); rewind(f);
        char line[256];
        KT_CHECK(fgets(line, sizeof(line), f) != nullptr);
        KT_CHECK(!drainLongLine(line, sizeof(line), f)); KT_CHECK(std::string(line) == "ab=1\n");
        KT_CHECK(fgets(line, sizeof(line), f) != nullptr); KT_CHECK(std::string(line) == "next=2\n");
        fclose(f);
    }
    {   // boundary: exactly 255 chars + newline is not "longer than 255"; 256 chars is; 255 chars at EOF is not
        char line[256];
        FILE* f = tmpfile(); fputs((std::string(255, 'a') + "\nz=1\n").c_str(), f); rewind(f);
        KT_CHECK(fgets(line, sizeof(line), f) != nullptr); KT_CHECK(!drainLongLine(line, sizeof(line), f));
        KT_CHECK(fgets(line, sizeof(line), f) != nullptr); KT_CHECK(std::string(line) == "z=1\n"); fclose(f);
        f = tmpfile(); fputs((std::string(256, 'a') + "\nz=1\n").c_str(), f); rewind(f);
        KT_CHECK(fgets(line, sizeof(line), f) != nullptr); KT_CHECK(drainLongLine(line, sizeof(line), f));
        KT_CHECK(fgets(line, sizeof(line), f) != nullptr); KT_CHECK(std::string(line) == "z=1\n"); fclose(f);
        f = tmpfile(); fputs((std::string(255, 'a') + "\r\nz=1\r\n").c_str(), f); rewind(f);   // CRLF ini: 255 chars still fit
        KT_CHECK(fgets(line, sizeof(line), f) != nullptr); KT_CHECK(!drainLongLine(line, sizeof(line), f));
        KT_CHECK(fgets(line, sizeof(line), f) != nullptr); KT_CHECK(std::string(line) == "z=1\r\n"); fclose(f);
        f = tmpfile(); fputs(std::string(255, 'a').c_str(), f); rewind(f);
        KT_CHECK(fgets(line, sizeof(line), f) != nullptr); KT_CHECK(!drainLongLine(line, sizeof(line), f)); fclose(f);
    }
}

// ---- fitLookup ---------------------------------------------------------------------------------------------------
static void testFitLookup() {
    KT_CHECK(kFitCount > 100);
    // every table row is found exactly, and the returned pointer is that very row
    int found = 0;
    for (int i = 0; i < kFitCount; ++i) {
        const FitEntry* e = fitLookup(kFit[i].row, kFit[i].var);
        if (e == &kFit[i]) ++found;
    }
    KT_CHECK_EQ(found, kFitCount);
    // the table is sorted by (row,var) with no duplicates (the binary search requires it)
    bool sorted = true;
    for (int i = 1; i < kFitCount; ++i) {
        unsigned a = ((unsigned)kFit[i - 1].row << 8) | kFit[i - 1].var, b = ((unsigned)kFit[i].row << 8) | kFit[i].var;
        if (a >= b) sorted = false;
    }
    KT_CHECK(sorted);
    // known entries
    const FitEntry* s = fitLookup(0, 1);
    KT_CHECK(s && s->row == 0 && s->var == 1 && approx(s->e[2], 1.0255f));
    const FitEntry* last = fitLookup(kFit[kFitCount - 1].row, kFit[kFitCount - 1].var);
    KT_CHECK(last == &kFit[kFitCount - 1]);
    const FitEntry* emp = fitLookup(12, 6); KT_CHECK(emp && (emp->flags & 1));   // an EMPTY (no mesh) model
    // misses: rows/variations that are not in the table, off both ends, negative
    KT_CHECK(fitLookup(0, 0) == nullptr);
    KT_CHECK(fitLookup(0, 4) == nullptr);
    KT_CHECK(fitLookup(500, 1) == nullptr);
    KT_CHECK(fitLookup(-1, 1) == nullptr);
    KT_CHECK(fitLookup(0, -1) == nullptr);
    KT_CHECK(fitLookup(255, 255) == nullptr);
    // F32: out-of-byte row/var are rejected (var >= 256 used to spill into the row bits and alias fitLookup(r+1, v)).
    KT_CHECK(fitLookup(0, 256 + 1) == nullptr);
    KT_CHECK(fitLookup(1, 1) != nullptr);                  // the entry it used to alias still resolves directly
    KT_CHECK(fitLookup(256, 1) == nullptr);                // row > 255
    KT_CHECK(fitLookup(0, 256) == nullptr);
}

// ---- camBasis / computeFit ---------------------------------------------------------------------------------------
static void testCamBasis() {
    Cfg c; cfgDefaults(&c);
    float r[3]; camBasis(c, r);
    // rig: fwd = -X, up = +Z  =>  fwd x up = +Y (the header's "right +Y")
    KT_CHECK(approx(r[0], 0, 1e-5f) && approx(r[1], 1, 1e-5f) && approx(r[2], 0, 1e-5f));
    for (int i = 0; i < 3; ++i) { KT_CHECK(approx(r[i], kRigCamRight[i], 1e-5f)); }
    c.camFwd[0] = 0; c.camFwd[1] = 1; c.camFwd[2] = 0; c.camUp[0] = 0; c.camUp[1] = 0; c.camUp[2] = 1;
    camBasis(c, r); KT_CHECK(r[0] == 1 && r[1] == 0 && r[2] == 0);       // +Y x +Z = +X exactly
    c.camUp[0] = 0; c.camUp[1] = 1; c.camUp[2] = 0;
    camBasis(c, r); KT_CHECK(r[0] == 0 && r[1] == 0 && r[2] == 0);       // parallel vectors -> zero
}

static const Q kIdent = {1, 0, 0, 0};

static void testComputeFitDistance() {
    Cfg c; cfgDefaults(&c);
    FitResult f;
    // 0.5x0.5x1 box (small enough to stay inside [distMin, distMax]), identity base, defPitch 0 -> hs = sqrt(ex^2+ey^2), vs = ez; cat 4 (armour) => fill = 1
    float e[3] = {0.5f, 0.5f, 1};
    computeFit(c, kIdent, e, 0, 4, 2.0f, &f);
    KT_CHECK(approx(f.ex, 0.5f) && approx(f.ey, 0.5f) && approx(f.ez, 1));
    KT_CHECK(approx(f.hs, std::sqrt(0.5f)) && approx(f.vs, 1));
    KT_CHECK(approx(f.aspect, 2.f));
    double t = std::tan(22.726 * 3.14159265 / 360.0);
    KT_CHECK(approx(f.tanV, (float)t, 1e-5f));                            // fov axis 'v': tanV = tan(fov/2)
    KT_CHECK(f.fill == 1.0f);
    double need = 1.15 * std::fmax(1.0 / (2 * t), std::sqrt(0.5) / (2 * t * 2.0));
    KT_CHECK(approx(f.need, (float)need, 2e-3f));
    KT_CHECK(approx(f.D, (float)need, 2e-3f) && f.s == 1.0f);            // inside [distMin, distMax]: distance mode, scale 1
    KT_CHECK(approx(f.d0, 0.983975f * 1.0f + 0.104367f * 0.000003f - 0.044566f * 0.000003f, 1e-4f));   // d0 = -(pos . fwd)
}
static void testComputeFitBranches() {
    Cfg c; cfgDefaults(&c);
    FitResult f; float e[3] = {1, 1, 2};
    // defPitch != 0 -> hs = vs = full diagonal
    computeFit(c, kIdent, e, 15, 4, 2.0f, &f);
    KT_CHECK(approx(f.hs, std::sqrt(6.f)) && f.hs == f.vs);
    // aspect < 0.1 falls back to 1 (and the result reports the fallback value)
    computeFit(c, kIdent, e, 0, 4, 0.05f, &f); KT_CHECK(f.aspect == 1.0f);
    computeFit(c, kIdent, e, 0, 4, 0.1f, &f); KT_CHECK(approx(f.aspect, 0.1f));
    computeFit(c, kIdent, e, 0, 4, -3.f, &f); KT_CHECK(f.aspect == 1.0f);
    computeFit(c, kIdent, e, 0, 4, 0.f, &f); KT_CHECK(f.aspect == 1.0f);
    // fov axis 'h': tanV = t / aspect
    Cfg h = c; h.fovAxis = 'h';
    FitResult fv, fh; computeFit(c, kIdent, e, 0, 4, 2.0f, &fv); computeFit(h, kIdent, e, 0, 4, 2.0f, &fh);
    KT_CHECK(approx(fh.tanV, fv.tanV / 2.0f, 1e-6f));
    // fill: armour (cat 4) is exempt; other categories use clamp((diag/ref)^exp, fillMin, 1)
    float small[3] = {0.1f, 0.1f, 0.1f};
    computeFit(c, kIdent, small, 0, 4, 1.f, &f); KT_CHECK(f.fill == 1.0f);
    computeFit(c, kIdent, small, 0, 2, 1.f, &f);
    KT_CHECK(approx(f.fill, std::fmax(0.4f, std::pow(std::sqrt(0.03f) / 0.6f, 0.35f)), 1e-3f)); KT_CHECK(f.fill < 1.0f);
    float tiny[3] = {0.001f, 0.001f, 0.001f};
    computeFit(c, kIdent, tiny, 0, 2, 1.f, &f); KT_CHECK(f.fill == 0.4f);   // floored at fillMin
    float huge[3] = {5, 5, 5};
    computeFit(c, kIdent, huge, 0, 2, 1.f, &f); KT_CHECK(f.fill == 1.0f);   // capped at 1
    // need is divided by fill
    FitResult a, b; computeFit(c, kIdent, tiny, 0, 4, 1.f, &a); computeFit(c, kIdent, tiny, 0, 2, 1.f, &b);
    KT_CHECK(approx(b.need, a.need / 0.4f, 1e-3f * b.need));
    // distance mode clamps: D floored at distMin
    Cfg m = c; m.distMin = 100; m.distMax = 200;
    computeFit(m, kIdent, e, 0, 4, 2.f, &f); KT_CHECK(f.D == 100 && f.s == 1.0f);
    // D above distMax: capped, and the model is scaled down instead: s = distMax / need
    Cfg n = c; n.distMax = 0.5f; n.distMin = 0.25f;
    computeFit(n, kIdent, e, 0, 4, 2.f, &f);
    KT_CHECK(f.D == 0.5f && f.need > 0.5f && approx(f.s, 0.5f / f.need, 1e-6f) && f.s < 1.0f);
    // scale mode: D = d0, s = d0 / need clamped to [0.01, 100]
    Cfg sc = c; sc.fitScale = 1;
    computeFit(sc, kIdent, e, 0, 4, 2.f, &f);
    KT_CHECK(f.D == f.d0 && approx(f.s, f.d0 / f.need, 1e-4f));
    float zero[3] = {0, 0, 0};
    computeFit(sc, kIdent, zero, 0, 4, 2.f, &f); KT_CHECK(f.s == 1.0f && f.need == 0.f);     // need <= 1e-4: scale left at 1
    float big[3] = {1000, 1000, 1000};
    computeFit(sc, kIdent, big, 0, 4, 2.f, &f); KT_CHECK(f.s == 0.01f);                       // lower clamp
    float ant[3] = {1e-3f, 1e-3f, 1e-3f};
    computeFit(sc, kIdent, ant, 0, 4, 2.f, &f); KT_CHECK(f.s <= 100.0f && f.s >= 0.01f);
    sc.camPos[0] = 1e6f; sc.camFwd[0] = -1; sc.camFwd[1] = 0; sc.camFwd[2] = 0;
    computeFit(sc, kIdent, ant, 0, 4, 2.f, &f); KT_CHECK(f.s == 100.0f);                      // upper clamp
}
static void testComputeFitRotation() {
    Cfg c; cfgDefaults(&c);
    FitResult f;
    // a 90-degree rotation about Z swaps the X and Y extents
    float e[3] = {1, 2, 3};
    const float Z[3] = {0, 0, 1};
    computeFit(c, qaxis(Z, 90), e, 0, 4, 1.f, &f);
    KT_CHECK(approx(f.ex, 2, 1e-4f) && approx(f.ey, 1, 1e-4f) && approx(f.ez, 3, 1e-4f));
    // about X by 90: Y and Z swap
    const float X[3] = {1, 0, 0};
    computeFit(c, qaxis(X, 90), e, 0, 4, 1.f, &f);
    KT_CHECK(approx(f.ex, 1, 1e-4f) && approx(f.ey, 3, 1e-4f) && approx(f.ez, 2, 1e-4f));
    // 45 degrees about Z of a 1x1 square: diagonal sqrt(2)
    float sq[3] = {1, 1, 1};
    computeFit(c, qaxis(Z, 45), sq, 0, 4, 1.f, &f); KT_CHECK(approx(f.ex, std::sqrt(2.f), 1e-4f) && approx(f.ez, 1, 1e-4f));
    // real table entry with the real default-ish orientation: result must be finite and positive
    const FitEntry* fe = fitLookup(2, 1);
    KT_CHECK(fe != nullptr);
    computeFit(c, qbase(90, 0, 90), fe->e, 15, 2, 4.0f, &f);
    KT_CHECK(std::isfinite(f.D) && f.D > 0 && std::isfinite(f.s) && f.s > 0 && f.fill >= 0.4f && f.fill <= 1);
    // all real table entries give a finite, positive D at the default config
    int bad = 0;
    for (int i = 0; i < kFitCount; ++i) {
        if (kFit[i].flags & 1) continue;
        computeFit(c, qbase(0, 0, 0), kFit[i].e, 15, 2, 2.4f, &f);
        if (!(std::isfinite(f.D) && f.D >= c.distMin && f.D <= c.distMax && std::isfinite(f.s) && f.s > 0)) ++bad;
    }
    KT_CHECK_EQ(bad, 0);
}

// ---- cursor / hit rect -------------------------------------------------------------------------------------------
static void testClientToEngine() {
    float x = -1, y = -1;
    clientToEngine(0, 0, 1920, 800, 3840, 1600, &x, &y); KT_CHECK(x == 0 && y == 0);
    clientToEngine(1920, 800, 1920, 800, 3840, 1600, &x, &y); KT_CHECK(x == 3840 && y == 1600);        // right/bottom edge maps to screenW/H
    clientToEngine(960, 400, 1920, 800, 3840, 1600, &x, &y); KT_CHECK(x == 1920 && y == 800);
    clientToEngine(1, 1, 1, 1, 3840, 1600, &x, &y); KT_CHECK(x == 3840 && y == 1600);                  // 1x1 client, smallest legal
    clientToEngine(-10, -5, 1920, 800, 3840, 1600, &x, &y); KT_CHECK(x == -20 && y == -10);            // outside the client: negative, not clamped
    clientToEngine(3840, 1600, 1920, 800, 3840, 1600, &x, &y); KT_CHECK(x == 7680 && y == 3200);       // beyond the client: > screen, not clamped
    clientToEngine(1, 1, 3, 3, 3840, 1600, &x, &y);
    KT_CHECK(x == 1.f * 3840.f / 3.f && y == 1.f * 1600.f / 3.f);                                      // float arithmetic: px*W first, then /cw
    // evaluation order: (px * screenW) / cw, not px * (screenW / cw)
    clientToEngine(7, 0, 3, 1, 3840.5f, 1600, &x, &y); KT_CHECK(x == 7 * 3840.5f / 3);
}
static void testFlipTop() {
    KT_CHECK(flipTop(0, 1600, 100, 300) == 100.f);
    KT_CHECK(flipTop(1, 1600, 100, 300) == 1200.f);          // screenH - y - h
    KT_CHECK(flipTop(1, 1600, 0, 1600) == 0.f);              // full-height view: same either way
    KT_CHECK(flipTop(0, 1600, -5, 10) == -5.f);              // no clamping
    KT_CHECK(flipTop(1, 1600, 1500, 300) == -200.f);         // extent hangs off the bottom -> negative top
    KT_CHECK(flipTop(7, 1600, 100, 300) == 1200.f);          // any non-zero flag flips
    KT_CHECK(flipTop(1, 1080.5f, 1, 2) == 1077.5f);
}
static void testRects() {
    // all-integer rect (wheel hook): left/top inclusive, right/bottom exclusive
    KT_CHECK(rectContains(100, 50, 100, 50, 200, 100));          // top-left corner
    KT_CHECK(rectContains(299.999f, 149.999f, 100, 50, 200, 100));
    KT_CHECK(!rectContains(300, 100, 100, 50, 200, 100));        // exclusive right/bottom
    KT_CHECK(!rectContains(300, 149.f, 100, 50, 200, 100));
    KT_CHECK(!rectContains(250, 150, 100, 50, 200, 100));
    KT_CHECK(!rectContains(99.999f, 100, 100, 50, 200, 100));
    KT_CHECK(!rectContains(150, 49.999f, 100, 50, 200, 100));
    KT_CHECK(!rectContains(250, 100, 100, 50, 0, 100));          // zero width: nothing is inside
    KT_CHECK(!rectContains(250, 100, 100, 50, 200, 0));
    KT_CHECK(!rectContains(250, 100, 100, 50, -200, 100));       // negative size: nothing
    KT_CHECK(rectContains(-5, -5, -10, -10, 20, 20));            // negative origin
    KT_CHECK(!rectContains(0, 0, 0, 0, 0, 0));                   // g_viewRect before the first tick (all zero)
    // float-top variant (drag tick)
    KT_CHECK(rectContainsTop(100, 50.5f, 100, 50.5f, 200, 100));
    KT_CHECK(!rectContainsTop(100, 50.49f, 100, 50.5f, 200, 100));
    KT_CHECK(rectContainsTop(299, 150.49f, 100, 50.5f, 200, 100));
    KT_CHECK(!rectContainsTop(299, 150.5f, 100, 50.5f, 200, 100));   // bottom = top + h exclusive, float sum
    KT_CHECK(!rectContainsTop(300, 100, 100, 50.5f, 200, 100));
    KT_CHECK(!rectContainsTop(99, 100, 100, 50.5f, 200, 100));
    // flipped box through the whole pipeline: hit_flip_y=1, engine 1600 high, extent y=100 h=300 -> top 1200
    float top = flipTop(1, 1600, 100, 300);
    KT_CHECK(rectContainsTop(150, 1200, 100, top, 200, 300) && rectContainsTop(150, 1499, 100, top, 200, 300));
    KT_CHECK(!rectContainsTop(150, 1500, 100, top, 200, 300) && !rectContainsTop(150, 150, 100, top, 200, 300));
}

int main() {
    testTrim();
    testParseFloats();
    testSplitIniLine();
    testCfgDefaults();
    testParseIniLine();
    testOverrides();
    testClamp();
    testLoadText();
    testDrainLongLine();
    testFitLookup();
    testCamBasis();
    testComputeFitDistance();
    testComputeFitBranches();
    testComputeFitRotation();
    testClientToEngine();
    testFlipTop();
    testRects();
    return kt::Report("inv3d_logic_test");
}
