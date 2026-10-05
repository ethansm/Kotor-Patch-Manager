// T1 host tests for Equip3dLogic.h (the pure logic of EquipCharPreview.cpp; doc 20).
// Build/run: python3 scripts/run_tests.py -k equip3d
//
// Where a test pins down behaviour that looks questionable rather than intended it is marked "CHARACTERIZATION" (the
// current behaviour is asserted so a later change is a conscious decision, not an accident); see the findings in the
// extraction report.
#include "Equip3dLogic.h"
#include "check.h"
#include <climits>
#include <cmath>
#include <string>
#include <vector>

using namespace eq3d;

static bool approx(float a, float b, float eps = 1e-4f) { return std::fabs(a - b) <= eps; }

// ---- INI helpers -------------------------------------------------------------------------------------------------
// One "key = value" line through the same split + parse the DLL's loadIni uses.
static IniResult applyLine(Cfg& c, const char* text) {
    char line[600]; snprintf(line, sizeof line, "%s", text);
    trim(line);
    char* k = nullptr; char* v = nullptr;
    IniLine sp = splitIniLine(line, &k, &v);
    if (sp != IniLineKeyValue) return IniUnknown;
    return parseIniLine(c, k, v);
}
struct IniStats { int ok = 0, bad = 0, keep = 0; };
// Whole-file driver: the DLL's loadIni loop (fgets line[600], trim, split, parse, clamp) minus the logging.
static Cfg loadText(const std::string& text, IniStats* st = nullptr) {
    Cfg c; IniStats s;
    FILE* f = tmpfile();
    fwrite(text.data(), 1, text.size(), f); rewind(f);
    char line[600];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        char* k = nullptr; char* v = nullptr;
        IniLine sp = splitIniLine(line, &k, &v);
        if (sp == IniLineSkip) continue;
        if (sp == IniLineNoEquals) { ++s.bad; continue; }
        IniResult r = parseIniLine(c, k, v);
        if (r == IniUnknown) { ++s.bad; continue; }
        if (r == IniOkKeepObj10) ++s.keep;
        if (r == IniBad) ++s.bad; else ++s.ok;
    }
    fclose(f);
    clampCfg(c);
    if (st) *st = s;
    return c;
}

static void testTrim() {
    auto t = [](const char* in) { char b[64]; snprintf(b, sizeof b, "%s", in); trim(b); return std::string(b); };
    KT_CHECK_EQ(t("").c_str(), "");
    KT_CHECK_EQ(t("   ").c_str(), "");
    KT_CHECK_EQ(t("\t\r\n ").c_str(), "");
    KT_CHECK_EQ(t("a").c_str(), "a");
    KT_CHECK_EQ(t("  a").c_str(), "a");
    KT_CHECK_EQ(t("a  ").c_str(), "a");
    KT_CHECK_EQ(t("  a b  ").c_str(), "a b");          // inner whitespace kept
    KT_CHECK_EQ(t("\ta\t\r\n").c_str(), "a");           // an fgets line keeps its newline: trim removes it
    KT_CHECK_EQ(t("\xff x \xff").c_str(), "\xff x \xff");   // high bytes are not whitespace (unsigned char cast, no UB)
    char b[8] = "  ab  ";                               // result is moved to the start of the buffer, NUL-terminated
    trim(b); KT_CHECK(b[0] == 'a' && b[1] == 'b' && b[2] == 0);
}

static void testParseFloats() {
    float o[3] = {-9, -9, -9};
    KT_CHECK(parseFloats("1.5", o, 1)); KT_CHECK(approx(o[0], 1.5f));
    KT_CHECK(parseFloats("  -2.25  ", o, 1)); KT_CHECK(approx(o[0], -2.25f));
    KT_CHECK(parseFloats("+3", o, 1)); KT_CHECK(approx(o[0], 3.f));
    KT_CHECK(parseFloats(".5", o, 1)); KT_CHECK(approx(o[0], 0.5f));
    KT_CHECK(parseFloats("5.", o, 1)); KT_CHECK(approx(o[0], 5.f));
    KT_CHECK(parseFloats("1e2", o, 1)); KT_CHECK(approx(o[0], 100.f));
    KT_CHECK(!parseFloats("", o, 1));
    KT_CHECK(!parseFloats("   ", o, 1));
    KT_CHECK(!parseFloats("abc", o, 1));
    KT_CHECK(!parseFloats("1.5x", o, 1));               // trailing garbage
    KT_CHECK(!parseFloats("1 2", o, 1));
    KT_CHECK(!parseFloats("1,", o, 1));
    KT_CHECK(!parseFloats("1,2", o, 1));                // too many values for n=1
    KT_CHECK(!parseFloats("nan", o, 1));                // non-finite rejected
    KT_CHECK(!parseFloats("inf", o, 1));
    KT_CHECK(!parseFloats("-inf", o, 1));
    KT_CHECK(!parseFloats("1e999", o, 1));              // overflows to inf
    // n = 2 / 3
    KT_CHECK(parseFloats("1,2", o, 2)); KT_CHECK(approx(o[0], 1.f) && approx(o[1], 2.f));
    KT_CHECK(parseFloats(" 1 , 2 ", o, 2)); KT_CHECK(approx(o[0], 1.f) && approx(o[1], 2.f));
    KT_CHECK(parseFloats("-1.5,2.5e1", o, 2)); KT_CHECK(approx(o[0], -1.5f) && approx(o[1], 25.f));
    KT_CHECK(!parseFloats("1", o, 2));                  // too few
    KT_CHECK(!parseFloats("1 2", o, 2));                // space is not a separator
    KT_CHECK(!parseFloats("1;2", o, 2));
    KT_CHECK(!parseFloats("1,", o, 2));
    KT_CHECK(!parseFloats("1,2,3", o, 2));              // too many
    KT_CHECK(!parseFloats(",2", o, 2));
    KT_CHECK(!parseFloats("1,nan", o, 2));
    KT_CHECK(parseFloats("1,2,3", o, 3)); KT_CHECK(approx(o[2], 3.f));
    KT_CHECK(!parseFloats("1,2", o, 3));
    KT_CHECK(parseFloats("", o, 0)); KT_CHECK(parseFloats("  ", o, 0)); KT_CHECK(!parseFloats("0", o, 0));   // n=0: only whitespace is a valid "empty list"
    // CHARACTERIZATION: strtof accepts C99 hex floats, so "stage=0x3" is a valid 3.
    KT_CHECK(parseFloats("0x10", o, 1)); KT_CHECK(approx(o[0], 16.f));
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
    KT_CHECK_EQ(sp("# comment", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp("; comment", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp("#a=b", &k, &v), IniLineSkip);
    KT_CHECK_EQ(sp("novalue", &k, &v), IniLineNoEquals);
    KT_CHECK_EQ(sp("[Section]", &k, &v), IniLineNoEquals);   // sections are not understood: counted as a rejected line
    KT_CHECK_EQ(sp("stage=3", &k, &v), IniLineKeyValue);
    KT_CHECK_EQ(k.c_str(), "stage"); KT_CHECK_EQ(v.c_str(), "3");
    sp("  Stage  =  3  ", &k, &v);                           // (caller trims the whole line first; split trims both halves)
    KT_CHECK_EQ(k.c_str(), "stage"); KT_CHECK_EQ(v.c_str(), "3");
    sp("STAGE=3", &k, &v); KT_CHECK_EQ(k.c_str(), "stage");  // key lower-cased
    sp("rig=Charrec_Light", &k, &v); KT_CHECK_EQ(v.c_str(), "Charrec_Light");   // value case preserved
    sp("stage=3 ; five", &k, &v); KT_CHECK_EQ(v.c_str(), "3");                  // ';' trailing comment
    sp("stage=3 # five", &k, &v); KT_CHECK_EQ(v.c_str(), "3");                  // '#' trailing comment
    sp("rig=a#b", &k, &v); KT_CHECK_EQ(v.c_str(), "a");      // CHARACTERIZATION: a value can never contain '#' or ';'
    sp("stage=;3", &k, &v); KT_CHECK_EQ(v.c_str(), "");
    sp("a=b=c", &k, &v); KT_CHECK_EQ(k.c_str(), "a"); KT_CHECK_EQ(v.c_str(), "b=c");   // first '=' splits
    sp("=v", &k, &v); KT_CHECK_EQ(k.c_str(), ""); KT_CHECK_EQ(v.c_str(), "v");        // empty key -> later "unknown key ''"
    sp("=", &k, &v); KT_CHECK_EQ(k.c_str(), ""); KT_CHECK_EQ(v.c_str(), "");
    // a rejected (no '=') line is left untouched so the caller can log it
    char line[] = "no equals here"; char *kk = nullptr, *vv = nullptr;
    splitIniLine(line, &kk, &vv); KT_CHECK_EQ(line, "no equals here");
    // a ';' before the '=' makes the line a comment only at column 0; elsewhere it falls into the key
    sp("a;b=c", &k, &v); KT_CHECK_EQ(k.c_str(), "a;b"); KT_CHECK_EQ(v.c_str(), "c");
}

static void testCfgDefaults() {
    Cfg c;
    KT_CHECK_EQ(c.enabled, 1); KT_CHECK_EQ(c.stage, 5); KT_CHECK_EQ(c.rig, "charrec_light");
    KT_CHECK(c.fov == 22.726f); KT_CHECK_EQ(c.bevels, 1);
    KT_CHECK(c.dragDegPx == 0.5f); KT_CHECK_EQ(c.wheelZoom, 1); KT_CHECK(c.wheelStep == 0.9f);
    KT_CHECK(c.zoomMin == 0.6f); KT_CHECK(c.zoomMax == 2.5f);
    KT_CHECK_EQ(c.hitFlipY, 0); KT_CHECK(c.screenW == 3840.f && c.screenH == 1600.f); KT_CHECK(!c.screenExplicit);
    KT_CHECK_EQ(c.flourish, 1); KT_CHECK(c.flMin == 12.f && c.flMax == 25.f && c.flFirst == 6.f && c.flQuiet == 3.f);
    KT_CHECK_EQ(c.flOnEquip, 1); KT_CHECK_EQ(c.flStance, "");
    KT_CHECK_EQ(c.flList, "shrug,nodyes,salute,bow,greeting,touchheart,point,rolleyes,scanning,victory,pausesh");
    KT_CHECK_EQ(c.equipAnim, "itemequip");
    KT_CHECK_EQ(c.pollMs, 100); KT_CHECK_EQ(c.rebuildMinMs, 250); KT_CHECK_EQ(c.keepObj10, 0);
    KT_CHECK_EQ(c.helmet, 1); KT_CHECK_EQ(c.buttons, 1); KT_CHECK(c.spinToggleDegS == 20.f); KT_CHECK(c.spinDegS == 0.f);
    // the defaults must survive the clamps untouched (an absent INI file is "defaults, no clamping surprises")
    Cfg d; KT_CHECK_EQ(clampCfg(d), 0);
    KT_CHECK(d.stage == c.stage && d.fov == c.fov && d.wheelStep == c.wheelStep && d.zoomMin == c.zoomMin &&
             d.zoomMax == c.zoomMax && d.flMin == c.flMin && d.flMax == c.flMax && d.pollMs == c.pollMs &&
             d.screenW == c.screenW && d.screenH == c.screenH);
}

static void testParseIniLineKeys() {
    Cfg c;
    // every key, one accepted value each -> the field it must land in
    KT_CHECK_EQ(applyLine(c, "enabled=0"), IniOk); KT_CHECK_EQ(c.enabled, 0);
    KT_CHECK_EQ(applyLine(c, "stage=3"), IniOk); KT_CHECK_EQ(c.stage, 3);
    KT_CHECK_EQ(applyLine(c, "rig=my_rig"), IniOk); KT_CHECK_EQ(c.rig, "my_rig");
    KT_CHECK_EQ(applyLine(c, "fov_deg=30.5"), IniOk); KT_CHECK(approx(c.fov, 30.5f));
    KT_CHECK_EQ(applyLine(c, "bevels=0"), IniOk); KT_CHECK_EQ(c.bevels, 0);
    KT_CHECK_EQ(applyLine(c, "drag_deg_per_px=0.25"), IniOk); KT_CHECK(approx(c.dragDegPx, 0.25f));
    KT_CHECK_EQ(applyLine(c, "wheel_zoom=0"), IniOk); KT_CHECK_EQ(c.wheelZoom, 0);
    KT_CHECK_EQ(applyLine(c, "wheel_step=0.8"), IniOk); KT_CHECK(approx(c.wheelStep, 0.8f));
    KT_CHECK_EQ(applyLine(c, "zoom_min=0.4"), IniOk); KT_CHECK(approx(c.zoomMin, 0.4f));
    KT_CHECK_EQ(applyLine(c, "zoom_max=3"), IniOk); KT_CHECK(approx(c.zoomMax, 3.f));
    KT_CHECK_EQ(applyLine(c, "hit_flip_y=1"), IniOk); KT_CHECK_EQ(c.hitFlipY, 1);
    KT_CHECK(!c.screenExplicit);
    KT_CHECK_EQ(applyLine(c, "screen_w=2560"), IniOk); KT_CHECK(c.screenW == 2560.f); KT_CHECK(c.screenExplicit);
    Cfg c2; KT_CHECK_EQ(applyLine(c2, "screen_h=1080"), IniOk); KT_CHECK(c2.screenH == 1080.f); KT_CHECK(c2.screenExplicit);
    KT_CHECK_EQ(applyLine(c, "flourish=0"), IniOk); KT_CHECK_EQ(c.flourish, 0);
    KT_CHECK_EQ(applyLine(c, "flourish_min_s=4"), IniOk); KT_CHECK(c.flMin == 4.f);
    KT_CHECK_EQ(applyLine(c, "flourish_max_s=9"), IniOk); KT_CHECK(c.flMax == 9.f);
    KT_CHECK_EQ(applyLine(c, "flourish_first_s=1.5"), IniOk); KT_CHECK(c.flFirst == 1.5f);
    KT_CHECK_EQ(applyLine(c, "flourish_quiet_s=7"), IniOk); KT_CHECK(c.flQuiet == 7.f);
    KT_CHECK_EQ(applyLine(c, "flourish_on_equip=0"), IniOk); KT_CHECK_EQ(c.flOnEquip, 0);
    KT_CHECK_EQ(applyLine(c, "flourish_stance_list=bow,salute"), IniOk); KT_CHECK_EQ(c.flStance, "bow,salute");
    KT_CHECK_EQ(applyLine(c, "flourish_list=bow"), IniOk); KT_CHECK_EQ(c.flList, "bow");
    KT_CHECK_EQ(applyLine(c, "equip_anim=equip"), IniOk); KT_CHECK_EQ(c.equipAnim, "equip");
    KT_CHECK_EQ(applyLine(c, "poll_ms=250"), IniOk); KT_CHECK_EQ(c.pollMs, 250);
    KT_CHECK_EQ(applyLine(c, "buttons=0"), IniOk); KT_CHECK_EQ(c.buttons, 0);
    KT_CHECK_EQ(applyLine(c, "spin_toggle_deg_s=-15"), IniOk); KT_CHECK(c.spinToggleDegS == -15.f);
    KT_CHECK_EQ(applyLine(c, "helmet=0"), IniOk); KT_CHECK_EQ(c.helmet, 0);
    KT_CHECK_EQ(applyLine(c, "rebuild_min_ms=500"), IniOk); KT_CHECK_EQ(c.rebuildMinMs, 500);
    KT_CHECK_EQ(applyLine(c, "spin_deg_s=3.5"), IniOk); KT_CHECK(c.spinDegS == 3.5f);
    // keep_obj10: deprecated; non-zero -> special result (the DLL logs the warning), zero -> plain ok
    Cfg k;
    KT_CHECK_EQ(applyLine(k, "keep_obj10=1"), IniOkKeepObj10); KT_CHECK_EQ(k.keepObj10, 1);
    KT_CHECK_EQ(applyLine(k, "keep_obj10=0"), IniOk); KT_CHECK_EQ(k.keepObj10, 0);
    KT_CHECK_EQ(applyLine(k, "keep_obj10=0.9"), IniOk);          // (int)0.9 == 0: truncation, no warning
    KT_CHECK_EQ(applyLine(k, "keep_obj10=abc"), IniBad);
    // unknown keys
    KT_CHECK_EQ(parseIniLine(c, "nope", "1"), IniUnknown);
    KT_CHECK_EQ(parseIniLine(c, "", "1"), IniUnknown);
    KT_CHECK_EQ(parseIniLine(c, "Stage", "1"), IniUnknown);       // the key must already be lower-case
    KT_CHECK_EQ(parseIniLine(c, "stage ", "1"), IniUnknown);
    KT_CHECK_EQ(parseIniLine(c, "stag", "1"), IniUnknown);
    KT_CHECK_EQ(parseIniLine(c, "stages", "1"), IniUnknown);
}

static void testParseIniLineValues() {
    Cfg c;
    // int fields truncate toward zero
    applyLine(c, "stage=2.9"); KT_CHECK_EQ(c.stage, 2);
    applyLine(c, "stage=-2.9"); KT_CHECK_EQ(c.stage, -2);
    applyLine(c, "stage=-0.5"); KT_CHECK_EQ(c.stage, 0);
    applyLine(c, "poll_ms=1e3"); KT_CHECK_EQ(c.pollMs, 1000);
    // rejected values leave the field alone
    Cfg d; KT_CHECK_EQ(applyLine(d, "stage=abc"), IniBad); KT_CHECK_EQ(d.stage, 5);
    KT_CHECK_EQ(applyLine(d, "stage="), IniBad); KT_CHECK_EQ(d.stage, 5);
    KT_CHECK_EQ(applyLine(d, "stage=1 2"), IniBad); KT_CHECK_EQ(d.stage, 5);
    KT_CHECK_EQ(applyLine(d, "stage=nan"), IniBad); KT_CHECK_EQ(d.stage, 5);
    KT_CHECK_EQ(applyLine(d, "fov_deg=inf"), IniBad); KT_CHECK(d.fov == 22.726f);
    KT_CHECK_EQ(applyLine(d, "screen_w=oops"), IniBad); KT_CHECK(!d.screenExplicit);   // a rejected screen_* does not mark explicit
    // string fields: length limits (strlen < sizeof) and emptiness
    std::string s63(63, 'r'), s64(64, 'r');
    KT_CHECK_EQ(applyLine(d, ("rig=" + s63).c_str()), IniOk); KT_CHECK_EQ(d.rig, s63.c_str());
    KT_CHECK_EQ(applyLine(d, ("rig=" + s64).c_str()), IniBad); KT_CHECK_EQ(d.rig, s63.c_str());   // 64 does not fit char[64]
    KT_CHECK_EQ(applyLine(d, "rig="), IniBad); KT_CHECK_EQ(d.rig, s63.c_str());                  // empty rig rejected
    std::string e63(63, 'a'), e64(64, 'a');
    KT_CHECK_EQ(applyLine(d, ("equip_anim=" + e63).c_str()), IniOk);
    KT_CHECK_EQ(applyLine(d, ("equip_anim=" + e64).c_str()), IniBad); KT_CHECK_EQ(d.equipAnim, e63.c_str());
    KT_CHECK_EQ(applyLine(d, "equip_anim="), IniBad);
    std::string l511(511, 'x'), l512(512, 'x');
    KT_CHECK_EQ(applyLine(d, ("flourish_list=" + l511).c_str()), IniOk); KT_CHECK_EQ(strlen(d.flList), 511);
    KT_CHECK_EQ(applyLine(d, ("flourish_list=" + l512).c_str()), IniBad); KT_CHECK_EQ(strlen(d.flList), 511);
    KT_CHECK_EQ(applyLine(d, ("flourish_stance_list=" + l511).c_str()), IniOk);
    KT_CHECK_EQ(applyLine(d, ("flourish_stance_list=" + l512).c_str()), IniBad); KT_CHECK_EQ(strlen(d.flStance), 511);
    KT_CHECK_EQ(applyLine(d, "flourish_stance_list="), IniOk); KT_CHECK_EQ(d.flStance, "");   // empty = "same list as flourish_list"
    KT_CHECK_EQ(applyLine(d, "flourish_list="), IniOk); KT_CHECK_EQ(d.flList, "");            // empty allowed (pickFlourish retries in 10 s)
}

static void testClamps() {
    // stage 0..5
    { Cfg c; c.stage = -1; clampCfg(c); KT_CHECK_EQ(c.stage, 0); }
    { Cfg c; c.stage = 0; clampCfg(c); KT_CHECK_EQ(c.stage, 0); }
    { Cfg c; c.stage = 1; clampCfg(c); KT_CHECK_EQ(c.stage, 1); }
    { Cfg c; c.stage = 5; clampCfg(c); KT_CHECK_EQ(c.stage, 5); }
    { Cfg c; c.stage = 6; clampCfg(c); KT_CHECK_EQ(c.stage, 5); }
    { Cfg c; c.stage = INT_MIN; clampCfg(c); KT_CHECK_EQ(c.stage, 0); }
    { Cfg c; c.stage = INT_MAX; clampCfg(c); KT_CHECK_EQ(c.stage, 5); }
    // fov: [5, 90] inclusive, else reset to 22.726 and reported
    { Cfg c; c.fov = 5.f; KT_CHECK_EQ(clampCfg(c) & ClampFov, 0); KT_CHECK(c.fov == 5.f); }
    { Cfg c; c.fov = 4.99f; KT_CHECK_EQ(clampCfg(c) & ClampFov, ClampFov); KT_CHECK(c.fov == 22.726f); }
    { Cfg c; c.fov = 90.f; KT_CHECK_EQ(clampCfg(c) & ClampFov, 0); KT_CHECK(c.fov == 90.f); }
    { Cfg c; c.fov = 90.01f; KT_CHECK_EQ(clampCfg(c) & ClampFov, ClampFov); KT_CHECK(c.fov == 22.726f); }
    { Cfg c; c.fov = 0.f; KT_CHECK_EQ(clampCfg(c) & ClampFov, ClampFov); KT_CHECK(c.fov == 22.726f); }
    { Cfg c; c.fov = -30.f; KT_CHECK_EQ(clampCfg(c) & ClampFov, ClampFov); KT_CHECK(c.fov == 22.726f); }
    { Cfg c; c.fov = 1000.f; KT_CHECK_EQ(clampCfg(c) & ClampFov, ClampFov); KT_CHECK(c.fov == 22.726f); }
    // wheelStep: [0.5, 0.99] inclusive, else reset to 0.9 (an in-range-but-wrong 1.0 would freeze zoom, 0 would zero it)
    { Cfg c; c.wheelStep = 0.5f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, 0); KT_CHECK(c.wheelStep == 0.5f); }
    { Cfg c; c.wheelStep = 0.49f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, ClampWheelStep); KT_CHECK(c.wheelStep == 0.9f); }
    { Cfg c; c.wheelStep = 0.99f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, 0); KT_CHECK(c.wheelStep == 0.99f); }
    { Cfg c; c.wheelStep = 0.991f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, ClampWheelStep); KT_CHECK(c.wheelStep == 0.9f); }
    { Cfg c; c.wheelStep = 1.0f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, ClampWheelStep); KT_CHECK(c.wheelStep == 0.9f); }
    { Cfg c; c.wheelStep = 0.f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, ClampWheelStep); KT_CHECK(c.wheelStep == 0.9f); }
    { Cfg c; c.wheelStep = -0.9f; KT_CHECK_EQ(clampCfg(c) & ClampWheelStep, ClampWheelStep); KT_CHECK(c.wheelStep == 0.9f); }
    { Cfg c; c.fov = 1; c.wheelStep = 2; KT_CHECK_EQ(clampCfg(c), ClampFov | ClampWheelStep); }   // both reported together
    // zoomMin >= 0.2
    { Cfg c; c.zoomMin = 0.2f; clampCfg(c); KT_CHECK(c.zoomMin == 0.2f); }
    { Cfg c; c.zoomMin = 0.19f; clampCfg(c); KT_CHECK(c.zoomMin == 0.2f); }
    { Cfg c; c.zoomMin = 0.f; clampCfg(c); KT_CHECK(c.zoomMin == 0.2f); }
    { Cfg c; c.zoomMin = -5.f; clampCfg(c); KT_CHECK(c.zoomMin == 0.2f); }
    { Cfg c; c.zoomMin = 0.21f; clampCfg(c); KT_CHECK(c.zoomMin == 0.21f); }
    // zoomMax >= zoomMin (applied AFTER the zoomMin floor)
    { Cfg c; c.zoomMin = 1.f; c.zoomMax = 1.f; clampCfg(c); KT_CHECK(c.zoomMax == 1.f); }
    { Cfg c; c.zoomMin = 1.f; c.zoomMax = 0.99f; clampCfg(c); KT_CHECK(c.zoomMax == 1.f); }
    { Cfg c; c.zoomMin = 1.f; c.zoomMax = 1.01f; clampCfg(c); KT_CHECK(c.zoomMax == 1.01f); }
    { Cfg c; c.zoomMax = 0.1f; clampCfg(c); KT_CHECK(c.zoomMax == 0.6f); }                   // default zoomMin 0.6
    { Cfg c; c.zoomMin = 0.1f; c.zoomMax = 0.15f; clampCfg(c); KT_CHECK(c.zoomMin == 0.2f && c.zoomMax == 0.2f); }
    { Cfg c; c.zoomMax = -1.f; clampCfg(c); KT_CHECK(c.zoomMax == c.zoomMin); }
    // F15: zoomMin is raised so fov / zoomMin never exceeds kMaxSceneFov (default fov keeps zoomMin 0.2)
    { Cfg c; c.fov = 90.f; c.zoomMin = 0.2f; clampCfg(c); KT_CHECK(approx(c.zoomMin, 0.6f, 1e-4f)); KT_CHECK(c.zoomMax >= c.zoomMin); }
    { Cfg c; c.fov = 90.f; c.zoomMin = 0.2f; c.zoomMax = 0.3f; clampCfg(c); KT_CHECK(c.zoomMax == c.zoomMin); }
    { Cfg c; c.zoomMin = 0.2f; clampCfg(c); KT_CHECK(c.zoomMin == 0.2f); }
    // flMin >= 2, flMax >= flMin
    { Cfg c; c.flMin = 2.f; clampCfg(c); KT_CHECK(c.flMin == 2.f); }
    { Cfg c; c.flMin = 1.99f; clampCfg(c); KT_CHECK(c.flMin == 2.f); }
    { Cfg c; c.flMin = 0.f; clampCfg(c); KT_CHECK(c.flMin == 2.f); }
    { Cfg c; c.flMin = -3.f; clampCfg(c); KT_CHECK(c.flMin == 2.f); }
    { Cfg c; c.flMin = 20.f; c.flMax = 20.f; clampCfg(c); KT_CHECK(c.flMax == 20.f); }
    { Cfg c; c.flMin = 20.f; c.flMax = 19.99f; clampCfg(c); KT_CHECK(c.flMax == 20.f); }
    { Cfg c; c.flMin = 20.f; c.flMax = 20.01f; clampCfg(c); KT_CHECK(c.flMax == 20.01f); }
    { Cfg c; c.flMin = 1.f; c.flMax = 1.f; clampCfg(c); KT_CHECK(c.flMin == 2.f && c.flMax == 2.f); }
    { Cfg c; c.flMax = 0.f; clampCfg(c); KT_CHECK(c.flMax == 12.f); }                       // default flMin 12
    // pollMs >= 16
    { Cfg c; c.pollMs = 16; clampCfg(c); KT_CHECK_EQ(c.pollMs, 16); }
    { Cfg c; c.pollMs = 15; clampCfg(c); KT_CHECK_EQ(c.pollMs, 16); }
    { Cfg c; c.pollMs = 17; clampCfg(c); KT_CHECK_EQ(c.pollMs, 17); }
    { Cfg c; c.pollMs = 0; clampCfg(c); KT_CHECK_EQ(c.pollMs, 16); }
    { Cfg c; c.pollMs = -1; clampCfg(c); KT_CHECK_EQ(c.pollMs, 16); }
    { Cfg c; c.pollMs = INT_MAX; clampCfg(c); KT_CHECK_EQ(c.pollMs, INT_MAX); }   // no upper bound
    // screen: BOTH reset to 3840x1600 when EITHER is < 100 (so exactly 100 is kept)
    { Cfg c; c.screenW = 100; c.screenH = 100; clampCfg(c); KT_CHECK(c.screenW == 100.f && c.screenH == 100.f); }
    { Cfg c; c.screenW = 99.99f; c.screenH = 1080; clampCfg(c); KT_CHECK(c.screenW == 3840.f && c.screenH == 1600.f); }
    { Cfg c; c.screenW = 1920; c.screenH = 99.99f; clampCfg(c); KT_CHECK(c.screenW == 3840.f && c.screenH == 1600.f); }
    { Cfg c; c.screenW = 0; c.screenH = 0; clampCfg(c); KT_CHECK(c.screenW == 3840.f && c.screenH == 1600.f); }
    { Cfg c; c.screenW = -1920; c.screenH = 1080; clampCfg(c); KT_CHECK(c.screenW == 3840.f && c.screenH == 1600.f); }
    { Cfg c; c.screenW = 1920; c.screenH = 1080; clampCfg(c); KT_CHECK(c.screenW == 1920.f && c.screenH == 1080.f); }
    { Cfg c; c.screenW = 1e7f; c.screenH = 1e7f; clampCfg(c); KT_CHECK(c.screenW == 1e7f); }   // no upper bound
    // F14 fixed: a reset or a lone screen_w/screen_h clears screenExplicit so syncScreen() keeps tracking the real framebuffer;
    // only a valid screen_w AND screen_h pair stays explicit.
    { Cfg c; applyLine(c, "screen_w=50"); clampCfg(c); KT_CHECK(!c.screenExplicit && c.screenW == 3840.f && c.screenH == 1600.f); }
    { Cfg c; applyLine(c, "screen_w=1920"); clampCfg(c); KT_CHECK(!c.screenExplicit && c.screenW == 3840.f && c.screenH == 1600.f); }
    { Cfg c; applyLine(c, "screen_h=1080"); clampCfg(c); KT_CHECK(!c.screenExplicit && c.screenW == 3840.f && c.screenH == 1600.f); }
    { Cfg c; applyLine(c, "screen_w=1920"); applyLine(c, "screen_h=50"); clampCfg(c); KT_CHECK(!c.screenExplicit && c.screenW == 3840.f && c.screenH == 1600.f); }
    { Cfg c; applyLine(c, "screen_w=1920"); applyLine(c, "screen_h=1080"); clampCfg(c); KT_CHECK(c.screenExplicit && c.screenW == 1920.f && c.screenH == 1080.f); }
    // clamps never touch unrelated fields
    { Cfg c; c.stage = 9; Cfg d; clampCfg(c); KT_CHECK(c.fov == d.fov && c.helmet == d.helmet && c.screenW == d.screenW); }
}

static void testIniFile() {
    IniStats st;
    Cfg c = loadText("", &st);
    KT_CHECK(st.ok == 0 && st.bad == 0); KT_CHECK_EQ(c.stage, 5);
    c = loadText("# only comments\n; and more\n\n   \n\t\n", &st);
    KT_CHECK(st.ok == 0 && st.bad == 0);
    c = loadText("stage=3\nhelmet = 0 ; off\nFOV_DEG=30\nbogus=1\nnoequals\nstage=zzz\n", &st);
    KT_CHECK_EQ(st.ok, 3); KT_CHECK_EQ(st.bad, 3);
    KT_CHECK_EQ(c.stage, 3); KT_CHECK_EQ(c.helmet, 0); KT_CHECK(c.fov == 30.f);
    c = loadText("stage=2\r\nhelmet=0\r\n", &st);                       // CRLF files (trim removes the \r)
    KT_CHECK_EQ(st.ok, 2); KT_CHECK_EQ(c.stage, 2); KT_CHECK_EQ(c.helmet, 0);
    c = loadText("stage=2\nstage=4\n", &st); KT_CHECK_EQ(c.stage, 4);   // last one wins
    c = loadText("stage=99\nfov_deg=500\nzoom_min=0\npoll_ms=1\n");     // clamps run after the file
    KT_CHECK_EQ(c.stage, 5); KT_CHECK(c.fov == 22.726f); KT_CHECK(c.zoomMin == 0.2f); KT_CHECK_EQ(c.pollMs, 16);
    c = loadText("keep_obj10=1\nkeep_obj10=0\n", &st); KT_CHECK_EQ(st.keep, 1); KT_CHECK_EQ(st.ok, 2);
    c = loadText("stage=4");                                            // no trailing newline
    KT_CHECK_EQ(c.stage, 4);
    // CHARACTERIZATION: loadIni reads with fgets(line[600]); a longer line is split and the tail is parsed as its own line.
    std::string longRig = "rig=" + std::string(700, 'a') + "\n";
    c = loadText(longRig, &st); KT_CHECK_EQ(st.ok, 0); KT_CHECK_EQ(st.bad, 2); KT_CHECK_EQ(c.rig, "charrec_light");
    // 598-char key=value fits (599 chars + NUL); flourish_list tops out at 511 chars regardless
    std::string fits = "flourish_list=" + std::string(511, 'x') + "\n";
    c = loadText(fits, &st); KT_CHECK_EQ(st.ok, 1); KT_CHECK_EQ(strlen(c.flList), 511);
}

// ---- weapons -----------------------------------------------------------------------------------------------------
static void testWeaponTable() {
    // table integrity (catches a bad regeneration)
    KT_CHECK_EQ((int)(sizeof(kWeapon) / sizeof(kWeapon[0])), kWeaponRows);
    KT_CHECK(kWeaponRows > 100);
    int wieldable = 0;
    for (int i = 0; i < kWeaponRows; ++i) {
        const WeaponRow& r = kWeapon[i];
        char what[64]; snprintf(what, sizeof what, "row %d", i);
        kt::CheckEq(what, (int)r.wield <= 6 ? 1 : 0, 1);                     // weaponwield 0..6
        kt::CheckEq(what, r.cls[0] == 0 ? 1 : 0, r.wield == 0 ? 1 : 0);                         // class empty <=> not wieldable
        bool chars = true; for (const char* p = r.cls; *p; ++p) if (!isalnum((unsigned char)*p) && *p != '_') chars = false;
        kt::CheckEq(what, chars ? 1 : 0, 1);                                                      // ResRef characters only
        if (r.wield) {
            ++wieldable;
            char n[28]; int w = -1;
            KT_CHECK(weaponModelName(i, 255, n, sizeof n, &w));
            kt::CheckEq(what, w, (int)r.wield);
            kt::CheckEq(what, strlen(n) <= 16 ? 1 : 0, 1);                                       // ResRefs are max 16 chars
            kt::CheckEq(what, strncmp(n, "w_", 2) == 0 ? 1 : 0, 1);
            KT_CHECK(kWeapon[i].wield >= 1);
            int style = styleFor(w, 0, true, false);                                              // every weapon alone in the right hand maps to a stance
            char what2[64]; snprintf(what2, sizeof what2, "row %d stance", i);
            kt::CheckEq(what2, style > 0 ? 1 : 0, 1);
        }
    }
    KT_CHECK(wieldable > 20);

    char n[28]; int w = -1;
    KT_CHECK(weaponModelName(0, 5, n, sizeof n, &w)); KT_CHECK_EQ(n, "w_Qtrstaff_005"); KT_CHECK_EQ(w, 3);   // Quarter_Staff
    KT_CHECK(weaponModelName(1, 0, n, sizeof n, &w)); KT_CHECK_EQ(n, "w_Stunbaton_000"); KT_CHECK_EQ(w, 1);
    KT_CHECK(weaponModelName(8, 12, n, sizeof n, &w)); KT_CHECK_EQ(n, "w_Lghtsbr_012"); KT_CHECK_EQ(w, 2);
    KT_CHECK(weaponModelName(12, 1, n, sizeof n, &w)); KT_CHECK_EQ(n, "w_BlstrPstl_001"); KT_CHECK_EQ(w, 4);
    KT_CHECK(weaponModelName(18, 255, n, sizeof n, &w)); KT_CHECK_EQ(n, "w_IonRfl_255"); KT_CHECK_EQ(w, 5);
    KT_CHECK(weaponModelName(23, 99, n, sizeof n, &w)); KT_CHECK_EQ(n, "w_RptnBlstr_099"); KT_CHECK_EQ(w, 6);
    KT_CHECK(weaponModelName(107, 3, n, sizeof n, &w)); KT_CHECK_EQ(n, "w_Qtrstaff_003"); KT_CHECK_EQ(w, 3);  // last real weapon
    // var formatting: %03d pads to 3 digits, never truncates wider values; negatives are not clamped (unreachable: unsigned char source)
    KT_CHECK(weaponModelName(0, 1000, n, sizeof n, &w)); KT_CHECK_EQ(n, "w_Qtrstaff_1000");
    KT_CHECK(weaponModelName(0, -5, n, sizeof n, &w)); KT_CHECK_EQ(n, "w_Qtrstaff_-05");
    // not wieldable / out of range -> false and nothing written
    char keep[28] = "sentinel"; w = 77;
    KT_CHECK(!weaponModelName(11, 0, keep, sizeof keep, &w));          // Lightsaber_Crystals (class empty, wield 0)
    KT_CHECK(!weaponModelName(25, 0, keep, sizeof keep, &w));          // Fragmentation_Grenades
    KT_CHECK(!weaponModelName(108, 0, keep, sizeof keep, &w));         // last row, not a weapon
    KT_CHECK(!weaponModelName(kWeaponRows, 0, keep, sizeof keep, &w)); // first out-of-range
    KT_CHECK(!weaponModelName(kWeaponRows - 1 + 1000, 0, keep, sizeof keep, &w));
    KT_CHECK(!weaponModelName(-1, 0, keep, sizeof keep, &w));
    KT_CHECK(!weaponModelName(INT_MIN, 0, keep, sizeof keep, &w));
    KT_CHECK(!weaponModelName(INT_MAX, 0, keep, sizeof keep, &w));
    KT_CHECK_EQ(keep, "sentinel"); KT_CHECK_EQ(w, 77);
    // small buffers: snprintf truncates and terminates (result is still "true"); sizing is the caller's job (name[28])
    char tiny[8]; KT_CHECK(weaponModelName(0, 5, tiny, sizeof tiny, &w)); KT_CHECK_EQ(tiny, "w_Qtrst"); KT_CHECK_EQ(w, 3);
}

static void testStyleFor() {
    // single right-hand weapon: weaponwield -> style (0 stun/none, 1 g1, 2 g2, 3 g3, 4 pistol -> 5, 5 rifle -> 7, 6 heavy -> 9)
    const int want[7] = {0, 1, 2, 3, 5, 7, 9};
    for (int rw = 0; rw <= 6; ++rw) {
        char what[32]; snprintf(what, sizeof what, "right-only wield %d", rw);
        kt::CheckEq(what, styleFor(rw, 0, true, false), want[rw]);
        kt::CheckEq(what, styleFor(rw, 99, true, false), want[rw]);          // left wield ignored when no left weapon
    }
    KT_CHECK_EQ(styleFor(-1, 0, true, false), 0);
    KT_CHECK_EQ(styleFor(7, 0, true, false), 0);
    KT_CHECK_EQ(styleFor(100, 0, true, false), 0);
    KT_CHECK_EQ(styleFor(INT_MIN, 0, true, false), 0);
    KT_CHECK_EQ(styleFor(INT_MAX, 0, true, false), 0);
    // dual wield: the LEFT weapon decides (2 = one-handed melee -> 4, 4 = pistol -> 6), anything else -> 0
    for (int rw = 0; rw <= 6; ++rw) {
        char what[32]; snprintf(what, sizeof what, "dual right %d", rw);
        kt::CheckEq(what, styleFor(rw, 2, true, true), 4);
        kt::CheckEq(what, styleFor(rw, 4, true, true), 6);
        kt::CheckEq(what, styleFor(rw, 1, true, true), 0);
        kt::CheckEq(what, styleFor(rw, 3, true, true), 0);
        kt::CheckEq(what, styleFor(rw, 5, true, true), 0);
        kt::CheckEq(what, styleFor(rw, 0, true, true), 0);
    }
    // left only / nothing: no stance (unarmed style 8 deliberately not applied)
    KT_CHECK_EQ(styleFor(0, 2, false, true), 0);
    KT_CHECK_EQ(styleFor(0, 4, false, true), 0);
    KT_CHECK_EQ(styleFor(2, 2, false, false), 0);
    KT_CHECK_EQ(styleFor(0, 0, false, false), 0);
    // exhaustive range: every result indexes kStyleAnim, and every positive style has a non-empty "g<N>r1" name
    for (int rw = -2; rw <= 8; ++rw) for (int lw = -2; lw <= 8; ++lw) for (int m = 0; m < 4; ++m) {
        int s = styleFor(rw, lw, (m & 1) != 0, (m & 2) != 0);
        KT_CHECK(s >= 0 && s < 10);
        if (s > 0) KT_CHECK(kStyleAnim[s][0] == 'g' && kStyleAnim[s][strlen(kStyleAnim[s]) - 1] == '1' && strlen(kStyleAnim[s]) == 4);
    }
    KT_CHECK_EQ(kStyleAnim[0], "");
    KT_CHECK_EQ(kStyleAnim[1], "g1r1"); KT_CHECK_EQ(kStyleAnim[4], "g4r1"); KT_CHECK_EQ(kStyleAnim[5], "g5r1");
    KT_CHECK_EQ(kStyleAnim[6], "g6r1"); KT_CHECK_EQ(kStyleAnim[7], "g7r1"); KT_CHECK_EQ(kStyleAnim[8], "g8r1");
    KT_CHECK_EQ(kStyleAnim[9], "g7r1");   // heavy repeating shares the rifle stance (the table says so on purpose)
    // the stance digit drives menuBuild's attack set: pistol 5 / rifle 7 / heavy 9->g7, dual 4 and 6
    KT_CHECK_EQ(kStyleAnim[styleFor(4, 0, true, false)], "g5r1");
    KT_CHECK_EQ(kStyleAnim[styleFor(5, 0, true, false)], "g7r1");
    KT_CHECK_EQ(kStyleAnim[styleFor(6, 0, true, false)], "g7r1");
    KT_CHECK_EQ(kStyleAnim[styleFor(2, 2, true, true)], "g4r1");
    KT_CHECK_EQ(kStyleAnim[styleFor(4, 4, true, true)], "g6r1");
}

// ---- idle / droids -----------------------------------------------------------------------------------------------
static void testIdle() {
    const int droids[] = {2, 3, 57, 231, 232, 453, 538, 539};
    int count = 0;
    for (int r = -5; r < 1000; ++r) if (isDroidRow(r)) ++count;
    KT_CHECK_EQ(count, 8);
    for (int r : droids) KT_CHECK(isDroidRow(r));
    KT_CHECK(!isDroidRow(-1)); KT_CHECK(!isDroidRow(0)); KT_CHECK(!isDroidRow(INT_MIN)); KT_CHECK(!isDroidRow(INT_MAX));
    KT_CHECK(!isDroidRow(56)); KT_CHECK(!isDroidRow(58)); KT_CHECK(!isDroidRow(230)); KT_CHECK(!isDroidRow(233));
    KT_CHECK(!isDroidRow(452)); KT_CHECK(!isDroidRow(454)); KT_CHECK(!isDroidRow(537)); KT_CHECK(!isDroidRow(540));
    // alignment thresholds: < 40 evil, < 60 neutral, else good
    KT_CHECK_EQ(idleFor(1, 0), "evil");
    KT_CHECK_EQ(idleFor(1, 39), "evil");
    KT_CHECK_EQ(idleFor(1, 40), "neutral");
    KT_CHECK_EQ(idleFor(1, 59), "neutral");
    KT_CHECK_EQ(idleFor(1, 60), "good");
    KT_CHECK_EQ(idleFor(1, kNeutralAlign), "neutral");   // F16: the fallback alignment when the stats block is unreadable
    KT_CHECK_EQ(idleFor(1, 100), "good");
    KT_CHECK_EQ(idleFor(1, -1), "evil");                   // Slot::align starts at -1
    KT_CHECK_EQ(idleFor(1, SHRT_MIN), "evil");
    KT_CHECK_EQ(idleFor(1, SHRT_MAX), "good");
    KT_CHECK_EQ(idleFor(0, 50), "neutral");
    KT_CHECK_EQ(idleFor(65535, 50), "neutral");            // appRow is an unsigned short
    // droids ignore alignment
    for (int r : droids) { KT_CHECK_EQ(idleFor(r, 0), "pause1"); KT_CHECK_EQ(idleFor(r, 50), "pause1"); KT_CHECK_EQ(idleFor(r, 100), "pause1"); KT_CHECK_EQ(idleFor(r, -1), "pause1"); }
}

// ---- camera / input ----------------------------------------------------------------------------------------------
static void testScreenValid() {
    KT_CHECK(screenValid(320, 200)); KT_CHECK(screenValid(1920, 1080)); KT_CHECK(screenValid(3840, 1600)); KT_CHECK(screenValid(16384, 16384));
    KT_CHECK(!screenValid(319, 200)); KT_CHECK(!screenValid(320, 199));
    KT_CHECK(!screenValid(16385, 1080)); KT_CHECK(!screenValid(1920, 16385));
    KT_CHECK(!screenValid(0, 0)); KT_CHECK(!screenValid(-1920, 1080)); KT_CHECK(!screenValid(1920, -1080));
    KT_CHECK(!screenValid(INT_MAX, 1080)); KT_CHECK(!screenValid(1920, INT_MIN));
    KT_CHECK(screenValid(320, 16384)); KT_CHECK(screenValid(16384, 200));
}

static void testClientToEngine() {
    float x = -1, y = -1;
    clientToEngine(0, 0, 1920, 1080, 3840, 1600, &x, &y); KT_CHECK(x == 0.f && y == 0.f);
    clientToEngine(960, 540, 1920, 1080, 3840, 1600, &x, &y); KT_CHECK(approx(x, 1920.f) && approx(y, 800.f));
    clientToEngine(1920, 1080, 1920, 1080, 3840, 1600, &x, &y); KT_CHECK(approx(x, 3840.f) && approx(y, 1600.f));
    clientToEngine(100, 50, 3840, 1600, 3840, 1600, &x, &y); KT_CHECK(approx(x, 100.f) && approx(y, 50.f));      // 1:1
    clientToEngine(1, 1, 3, 3, 3840, 1600, &x, &y); KT_CHECK(approx(x, 1280.f) && approx(y, 1600.f / 3.f));
    clientToEngine(-10, -20, 1920, 1080, 3840, 1600, &x, &y); KT_CHECK(approx(x, -20.f) && approx(y, -20.f * 1600.f / 1080.f, 1e-3f));   // outside the client: negative, not clamped
    clientToEngine(2000, 1100, 1920, 1080, 3840, 1600, &x, &y); KT_CHECK(x > 3840.f && y > 1600.f);          // beyond the client: > screen, not clamped
    clientToEngine(1, 1, 1, 1, 640, 480, &x, &y); KT_CHECK(approx(x, 640.f) && approx(y, 480.f));                 // 1x1 client: smallest legal divisor
    // formula identity with the double-precision expression (sweep)
    for (long px = 0; px <= 1920; px += 37) {
        clientToEngine(px, px / 2, 1920, 1080, 3840, 1600, &x, &y);
        KT_CHECK(approx(x, (float)(px * 3840.0 / 1920), 1e-2f)); KT_CHECK(approx(y, (float)((px / 2) * 1600.0 / 1080), 1e-2f));
    }
}

static void testRectContains() {
    // half-open box [rx, rx+rw) x [ry, ry+rh)
    KT_CHECK(rectContains(10, 20, 10, 20, 100, 50));          // top-left corner: inclusive
    KT_CHECK(rectContains(109.99f, 69.99f, 10, 20, 100, 50)); // just inside bottom-right
    KT_CHECK(!rectContains(110, 40, 10, 20, 100, 50));        // right edge: exclusive
    KT_CHECK(!rectContains(60, 70, 10, 20, 100, 50));         // bottom edge: exclusive
    KT_CHECK(!rectContains(110, 70, 10, 20, 100, 50));
    KT_CHECK(!rectContains(9.99f, 40, 10, 20, 100, 50));      // left of it
    KT_CHECK(!rectContains(60, 19.99f, 10, 20, 100, 50));     // above it
    KT_CHECK(rectContains(60, 40, 10, 20, 100, 50));
    KT_CHECK(rectContains(0, 0, 0, 0, 1, 1)); KT_CHECK(!rectContains(1, 0, 0, 0, 1, 1)); KT_CHECK(!rectContains(0, 1, 0, 0, 1, 1));
    // empty / inverted boxes contain nothing (ctlRect already rejects w/h <= 0, but the helper must not rely on it)
    KT_CHECK(!rectContains(10, 20, 10, 20, 0, 50)); KT_CHECK(!rectContains(10, 20, 10, 20, 100, 0));
    KT_CHECK(!rectContains(5, 5, 10, 10, -10, -10)); KT_CHECK(!rectContains(10, 10, 10, 10, -10, -10));
    // negative origin
    KT_CHECK(rectContains(-5, -5, -10, -10, 20, 20)); KT_CHECK(!rectContains(10, 0, -10, -10, 20, 20));
    // NaN never matches
    KT_CHECK(!rectContains(NAN, 40, 10, 20, 100, 50)); KT_CHECK(!rectContains(60, NAN, 10, 20, 100, 50));
    // the call sites pass ints through float: identical to the old integer arithmetic for any realistic coordinate
    for (int rx = -3; rx <= 3000; rx += 641) for (int rw = 0; rw <= 4000; rw += 799) for (int x = -5; x <= 7000; x += 97) {
        bool old = x >= rx && x < rx + rw;
        bool now = rectContains((float)x, 0, (float)rx, 0, (float)rw, 1);
        kt::CheckEq("int vs float box (x axis)", now ? 1 : 0, old ? 1 : 0);
    }
}

static void testHitTop() {
    KT_CHECK(hitTop(0, 1600, 100, 800) == 100.f);
    KT_CHECK(hitTop(1, 1600, 100, 800) == 700.f);            // flipped: screenH - y - h
    KT_CHECK(hitTop(1, 1600, 0, 1600) == 0.f);               // full-height view maps to itself
    KT_CHECK(hitTop(1, 1600, 0, 0) == 1600.f);
    KT_CHECK(hitTop(1, 1600, 900, 800) == -100.f);           // extends past the bottom: negative, not clamped
    KT_CHECK(hitTop(0, 1600, -50, 10) == -50.f);
    KT_CHECK(hitTop(2, 1080, 10, 20) == 1050.f);             // any non-zero flag flips
    KT_CHECK(hitTop(-1, 1080, 10, 20) == 1050.f);
    KT_CHECK(hitTop(1, 1080.5f, 10, 20) == 1050.5f);
}

static void testYawZoomFov() {
    KT_CHECK(approx(yawDrag(10, 110, 100, 0.5f), 15.f));
    KT_CHECK(approx(yawDrag(10, 90, 100, 0.5f), 5.f));         // dragging left turns the other way
    KT_CHECK(yawDrag(10, 100, 100, 0.5f) == 10.f);
    KT_CHECK(approx(yawDrag(0, 1, 0, -0.5f), -0.5f));          // negative drag_deg_per_px inverts
    KT_CHECK(approx(yawDrag(-720, 100, 0, 1.f), -620.f));      // yaw is not wrapped
    KT_CHECK(approx(yawDrag(0, 3840, 0, 0.5f), 1920.f));
    KT_CHECK(yawDrag(7, 55, 0, 0.f) == 7.f);

    // zoomStep: +120 = one notch up = zoom in = divide by wheelStep
    KT_CHECK(approx(zoomStep(1.f, 120, 0.9f, 0.6f, 2.5f), 1.f / 0.9f));
    KT_CHECK(approx(zoomStep(1.f, -120, 0.9f, 0.6f, 2.5f), 0.9f));
    KT_CHECK(approx(zoomStep(1.f, 240, 0.9f, 0.6f, 2.5f), 1.f / (0.9f * 0.9f)));
    KT_CHECK(approx(zoomStep(1.f, 60, 0.9f, 0.6f, 2.5f), 1.f / std::sqrt(0.9f)));          // half a notch
    KT_CHECK(approx(zoomStep(1.f, 1, 0.9f, 0.6f, 2.5f), std::pow(0.9f, -1.f / 120.f), 1e-5f));
    KT_CHECK(approx(zoomStep(1.f, 0, 0.9f, 0.6f, 2.5f), 1.f));
    KT_CHECK(approx(zoomStep(2.f, -120, 0.5f, 0.6f, 2.5f), 1.f));                          // wheelStep 0.5 halves
    KT_CHECK(approx(zoomStep(1.f, 120, 0.5f, 0.6f, 2.5f), 2.f));
    // clamps at both ends
    KT_CHECK(zoomStep(2.4f, 1200, 0.9f, 0.6f, 2.5f) == 2.5f);
    KT_CHECK(zoomStep(0.7f, -1200, 0.9f, 0.6f, 2.5f) == 0.6f);
    KT_CHECK(zoomStep(1.f, 100000, 0.9f, 0.6f, 2.5f) == 2.5f);                          // powf -> inf
    KT_CHECK(zoomStep(1.f, -100000, 0.9f, 0.6f, 2.5f) == 0.6f);                         // powf -> 0
    KT_CHECK(zoomStep(1.f, 100000, 0.99f, 0.2f, 100.f) == 100.f);
    KT_CHECK(zoomStep(1.f, 120, 0.9f, 1.f, 1.f) == 1.f);                                // min == max pins it
    // an out-of-range starting zoom is pulled into range by the first step, even a tiny one
    KT_CHECK(zoomStep(10.f, 1, 0.9f, 0.6f, 2.5f) == 2.5f);
    KT_CHECK(zoomStep(0.01f, -1, 0.9f, 0.6f, 2.5f) == 0.6f);
    // stepping up then down by the same amount returns to the start (inside the clamp range)
    { float z = 1.f; z = zoomStep(z, 360, 0.85f, 0.6f, 2.5f); z = zoomStep(z, -360, 0.85f, 0.6f, 2.5f); KT_CHECK(approx(z, 1.f, 1e-4f)); }
    // monotone in the wheel delta
    { float last = 0; for (long w = -600; w <= 600; w += 30) { float z = zoomStep(1.f, w, 0.9f, 0.2f, 10.f); KT_CHECK(z >= last); last = z; } }

    KT_CHECK(approx(fovForZoom(22.726f, 1.f), 22.726f));
    KT_CHECK(approx(fovForZoom(22.726f, 2.f), 11.363f));
    KT_CHECK(approx(fovForZoom(22.726f, 0.6f), 37.8767f, 1e-3f));
    KT_CHECK(approx(fovForZoom(22.726f, 2.5f), 9.0904f, 1e-3f));
    // F15 fixed: the scene fov is capped at kMaxSceneFov; fov_deg=90 with zoom 0.2 used to ask the camera for 450 degrees
    KT_CHECK(approx(fovForZoom(90.f, 0.2f), 150.f, 1e-2f));
    KT_CHECK(approx(fovForZoom(90.f, 0.6f), 150.f, 1e-2f));   // exactly at the cap

    float v[3];
    yawVec(0, v); KT_CHECK(approx(v[0], 1.f) && approx(v[1], 0.f) && v[2] == 0.f);
    yawVec(90, v); KT_CHECK(approx(v[0], 0.f, 1e-6f) && approx(v[1], 1.f));
    yawVec(180, v); KT_CHECK(approx(v[0], -1.f) && approx(v[1], 0.f, 1e-5f));
    yawVec(270, v); KT_CHECK(approx(v[0], 0.f, 1e-5f) && approx(v[1], -1.f));
    yawVec(360, v); KT_CHECK(approx(v[0], 1.f, 1e-5f) && approx(v[1], 0.f, 1e-5f));
    yawVec(-90, v); KT_CHECK(approx(v[0], 0.f, 1e-6f) && approx(v[1], -1.f));
    yawVec(45, v); KT_CHECK(approx(v[0], 0.70710678f) && approx(v[1], 0.70710678f));
    yawVec(720 + 30, v); KT_CHECK(approx(v[0], std::cos(30 * 3.14159265f / 180)) && approx(v[1], 0.5f, 1e-4f));
    for (int d = -1000; d <= 1000; d += 17) { yawVec((float)d, v); KT_CHECK(approx(v[0] * v[0] + v[1] * v[1], 1.f, 1e-5f)); KT_CHECK(v[2] == 0.f); }
}

// ---- toolbar / pop-up layout -------------------------------------------------------------------------------------
static void testLayoutSpin() {
    Box sp{-1, -1, -1, -1}, rs{-2, -2, -2, -2};
    // single Spin button: the whole row, Reset hidden and untouched
    KT_CHECK(!layoutSpin(Box{100, 200, 333, 40}, false, &sp, &rs));
    KT_CHECK(sp.x == 100 && sp.y == 200 && sp.w == 333 && sp.h == 40);
    KT_CHECK(rs.x == -2 && rs.w == -2);
    // split row: play/pause 30 %, 4 % gap, Reset takes the rest (W chosen off the x87/SSE rounding knife edge, see findings)
    KT_CHECK(layoutSpin(Box{100, 200, 333, 40}, true, &sp, &rs));
    KT_CHECK(sp.x == 100 && sp.y == 200 && sp.w == 99 && sp.h == 40);                     // (int)(333*0.30) = 99
    KT_CHECK(rs.x == 100 + 99 + 13 && rs.y == 200 && rs.w == 333 - 99 - 13 && rs.h == 40);   // gap (int)(333*0.04) = 13
    KT_CHECK(layoutSpin(Box{0, 0, 1000, 10}, true, &sp, &rs));
    KT_CHECK(approx((float)sp.w, 300.f, 1.f)); KT_CHECK(approx((float)(rs.x - sp.w), 40.f, 1.f));
    // tiny and degenerate rows
    KT_CHECK(layoutSpin(Box{5, 6, 1, 7}, true, &sp, &rs)); KT_CHECK(sp.w == 0 && rs.x == 5 && rs.w == 1);
    KT_CHECK(layoutSpin(Box{5, 6, 0, 7}, true, &sp, &rs)); KT_CHECK(sp.w == 0 && rs.w == 0);
    KT_CHECK(layoutSpin(Box{5, 6, -10, 7}, true, &sp, &rs)); KT_CHECK(sp.w == -3 && rs.w == -10 + 3 + 0);   // truncation toward zero: (int)-3.0, (int)-0.4
    // invariant: the two buttons plus the gap always add up to the row, whatever the rounding
    for (int W = 0; W <= 3000; ++W) {
        layoutSpin(Box{10, 20, W, 30}, true, &sp, &rs);
        int gap = rs.x - (sp.x + sp.w);
        kt::CheckEq("split row covers W", sp.w + gap + rs.w, W);
        KT_CHECK(gap >= 0 && sp.w >= 0 && rs.w >= 0 && std::abs(sp.w - 0.30 * W) <= 1.0 && std::abs(gap - 0.04 * W) <= 1.0);
        KT_CHECK(sp.y == 20 && rs.y == 20 && sp.h == 30 && rs.h == 30 && sp.x == 10);
    }
}

static void testMenuPitch() {
    KT_CHECK(approx(menuPitch(true, 100, 200, 40), 50.f));        // half the gap between buttons 0 and 2
    KT_CHECK(approx(menuPitch(true, 100, 101, 40), 0.5f));
    KT_CHECK(approx(menuPitch(true, 100, 100, 40), 48.f));        // equal Y: fallback 1.2 * h
    KT_CHECK(approx(menuPitch(true, 200, 100, 40), 48.f));        // 2 above 0: fallback
    KT_CHECK(approx(menuPitch(false, 100, 200, 40), 48.f));       // a rect missing: fallback
    KT_CHECK(approx(menuPitch(false, 0, 0, 0), 0.f));
    KT_CHECK(approx(menuPitch(true, -50, 50, 10), 50.f));
}

static void testLayoutMenu() {
    KT_CHECK_EQ(MenuRows, 8); KT_CHECK_EQ(kPageToggle, "@page");
    char anim[20][12] = {};
    MenuPos out[20];
    auto fill = [&](int n, int toggleAt) {
        for (int i = 0; i < 20; ++i) anim[i][0] = 0;
        for (int i = 0; i < n; ++i) { if (i == toggleAt) strcpy(anim[i], "@page"); else { anim[i][0] = 'e'; anim[i][1] = (char)('a' + i); anim[i][2] = 0; } }
    };
    // roomy view: no vertical shift. w=200 pitch=50 view (100,50) 800 tall: left 120, gap 16, top 146
    fill(6, 5);
    MenuGeom g = layoutMenu(200.f, 50.f, 100, 50, 800, 6, anim, 20, out);
    KT_CHECK(approx(g.left, 120.f) && approx(g.gap, 16.f) && approx(g.top, 146.f));
    for (int i = 0; i < 5; ++i) { KT_CHECK_EQ(out[i].col, 0); KT_CHECK_EQ(out[i].row, i); KT_CHECK_EQ(out[i].x, 120); KT_CHECK_EQ(out[i].y, 146 + 50 * i); }
    KT_CHECK_EQ(out[5].col, 0); KT_CHECK_EQ(out[5].row, 7); KT_CHECK_EQ(out[5].x, 120); KT_CHECK_EQ(out[5].y, 146 + 350);   // toggle: bottom row of column 0
    // exactly one column of plain items: rows 0..6 in column 0, the 8th spills into column 1 row 0
    fill(8, -1);
    layoutMenu(200.f, 50.f, 100, 50, 800, 8, anim, 20, out);
    for (int i = 0; i < 7; ++i) { KT_CHECK_EQ(out[i].col, 0); KT_CHECK_EQ(out[i].row, i); }
    KT_CHECK_EQ(out[7].col, 1); KT_CHECK_EQ(out[7].row, 0); KT_CHECK_EQ(out[7].x, (int)(120 + 216.f)); KT_CHECK_EQ(out[7].y, 146);
    // gestures page of a humanoid with a stance: 14 gestures + "Attacks >" last -> col0 rows 0-6, toggle (0,7), col1 rows 0-6
    fill(15, 14);
    layoutMenu(200.f, 50.f, 100, 50, 800, 15, anim, 20, out);
    for (int i = 0; i < 7; ++i) { KT_CHECK_EQ(out[i].col, 0); KT_CHECK_EQ(out[i].row, i); }
    for (int i = 7; i < 14; ++i) { KT_CHECK_EQ(out[i].col, 1); KT_CHECK_EQ(out[i].row, i - 7); }
    KT_CHECK(out[14].col == 0 && out[14].row == 7);
    // 20 plain items: 7 in col 0, 8 in col 1, 5 in col 2 (rows restart at 0 each column)
    fill(20, -1);
    layoutMenu(200.f, 50.f, 100, 50, 800, 20, anim, 20, out);
    for (int i = 0; i < 20; ++i) {
        int col = i < 7 ? 0 : 1 + (i - 7) / 8, row = i < 7 ? i : (i - 7) % 8;
        KT_CHECK_EQ(out[i].col, col); KT_CHECK_EQ(out[i].row, row);
        KT_CHECK_EQ(out[i].x, (int)(120 + col * (200.f + 16.f))); KT_CHECK_EQ(out[i].y, (int)(146 + row * 50.f));
    }
    KT_CHECK_EQ(out[19].col, 2); KT_CHECK_EQ(out[15].col, 2); KT_CHECK_EQ(out[14].col, 1);
    // the toggle does not consume a plain slot, wherever it sits in the list
    fill(4, 0);
    layoutMenu(200.f, 50.f, 100, 50, 800, 4, anim, 20, out);
    KT_CHECK(out[0].col == 0 && out[0].row == 7);
    for (int i = 1; i < 4; ++i) { KT_CHECK_EQ(out[i].col, 0); KT_CHECK_EQ(out[i].row, i - 1); }
    // CHARACTERIZATION: two toggles would stack on the same cell (menuBuild never produces two)
    fill(3, -1); strcpy(anim[0], "@page"); strcpy(anim[2], "@page");
    layoutMenu(200.f, 50.f, 100, 50, 800, 3, anim, 20, out);
    KT_CHECK(out[0].x == out[2].x && out[0].y == out[2].y);
    // an animation merely STARTING with "@page" is a plain item (exact match only)
    fill(2, -1); strcpy(anim[0], "@pageX");
    layoutMenu(200.f, 50.f, 100, 50, 800, 2, anim, 20, out); KT_CHECK(out[0].row == 0);

    // view too short for 8 rows: top shifts up so the last row ends at "bottom" (87 % of the view height), never above the view
    fill(1, -1);
    g = layoutMenu(200.f, 70.f, 0, 0, 700, 1, anim, 20, out);          // top 84 + 560 = 644 > bottom 609 -> top = 609-560 = 49
    KT_CHECK(approx(g.top, 49.f, 0.01f)); KT_CHECK(out[0].y >= 48 && out[0].y <= 49);
    g = layoutMenu(200.f, 70.f, 0, 0, 300, 1, anim, 20, out);          // bottom 261 - 560 < 0 -> clamps to the view top
    KT_CHECK(approx(g.top, 0.f)); KT_CHECK_EQ(out[0].y, 0);
    g = layoutMenu(200.f, 70.f, 0, 25, 300, 1, anim, 20, out);         // ... which is vy, not 0
    KT_CHECK(approx(g.top, 25.f)); KT_CHECK_EQ(out[0].y, 25);
    g = layoutMenu(200.f, 50.f, 0, 0, 700, 1, anim, 20, out);          // 84 + 400 = 484 <= 609: no shift (boundary below)
    KT_CHECK(approx(g.top, 84.f));
    // boundary: top + 8*pitch == bottom exactly -> no shift (the test is strictly greater). view 1000 tall: top 120, bottom 870
    g = layoutMenu(200.f, 93.75f, 0, 0, 1000, 1, anim, 20, out); KT_CHECK(approx(g.top, 120.f, 0.01f));   // 120 + 750 = 870
    g = layoutMenu(200.f, 94.f, 0, 0, 1000, 1, anim, 20, out); KT_CHECK(approx(g.top, 870.f - 752.f, 0.01f));
    // vh <= 0 (view extent unreadable): no shift, the offsets use vh as given
    g = layoutMenu(200.f, 50.f, 100, 50, 0, 1, anim, 20, out); KT_CHECK(approx(g.top, 50.f) && approx(g.left, 120.f));
    g = layoutMenu(200.f, 5000.f, 100, 50, 0, 1, anim, 20, out); KT_CHECK(approx(g.top, 50.f));
    g = layoutMenu(200.f, 5000.f, 100, 50, -100, 1, anim, 20, out); KT_CHECK(approx(g.top, 50.f - 12.f));
    // negative view origin (view partially off-screen) is carried through
    g = layoutMenu(100.f, 40.f, -30, -10, 800, 1, anim, 20, out); KT_CHECK(approx(g.left, -20.f) && approx(g.top, 86.f));
    // count 0 writes nothing; count above the buffer is clamped to maxOut
    MenuPos sentinel[4]; for (auto& m : sentinel) m = MenuPos{-7, -7, -7, -7};
    fill(0, -1); layoutMenu(200.f, 50.f, 0, 0, 800, 0, anim, 4, sentinel);
    KT_CHECK(sentinel[0].x == -7);
    fill(8, -1); layoutMenu(200.f, 50.f, 0, 0, 800, 8, anim, 4, sentinel);
    KT_CHECK(sentinel[3].row == 3 && sentinel[0].row == 0);
    for (auto& m : sentinel) m = MenuPos{-7, -7, -7, -7};
    layoutMenu(200.f, 50.f, 0, 0, 800, 8, anim, 3, sentinel); KT_CHECK(sentinel[2].row == 2 && sentinel[3].x == -7);
    layoutMenu(200.f, 50.f, 0, 0, 800, -3, anim, 3, sentinel);   // negative count: nothing to do
    // const-qualified source array (template deduction)
    static const char konst[3][12] = {"a", "@page", "c"};
    layoutMenu(200.f, 50.f, 0, 0, 800, 3, konst, 3, sentinel); KT_CHECK(sentinel[1].row == 7 && sentinel[2].row == 1);
}

int main() {
    testTrim();
    testParseFloats();
    testSplitIniLine();
    testCfgDefaults();
    testParseIniLineKeys();
    testParseIniLineValues();
    testClamps();
    testIniFile();
    testWeaponTable();
    testStyleFor();
    testIdle();
    testScreenValid();
    testClientToEngine();
    testRectContains();
    testHitTop();
    testYawZoomFov();
    testLayoutSpin();
    testMenuPitch();
    testLayoutMenu();
    return kt::Report("equip3d_logic");
}
