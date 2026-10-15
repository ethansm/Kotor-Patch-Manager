// T1 host tests for HolsterTune.h (ini tuning-key parser for the holstered-weapons mod; doc 42).
// Build/run: python3 scripts/run_tests.py holster_tune
#include "HolsterTune.h"
#include "check.h"
#include <string.h>

using namespace hs;

static TuneCfg base() {
    TuneCfg c;
    c.place = true;
    c.rot[0] = 1.f; c.rot[1] = 2.f; c.rot[2] = 3.f;
    c.off[0] = 0.5f; c.off[1] = 0.5f; c.off[2] = 0.5f;
    return c;
}

static bool same(const TuneCfg& a, const TuneCfg& b) {
    return a.place == b.place && memcmp(a.rot, b.rot, sizeof a.rot) == 0 && memcmp(a.off, b.off, sizeof a.off) == 0;
}

static void testDefaultIsZero() {
    TuneCfg c;
    KT_CHECK(!c.place);
    KT_CHECK(c.rot[0] == 0.f && c.rot[1] == 0.f && c.rot[2] == 0.f);
    KT_CHECK(c.off[0] == 0.f && c.off[1] == 0.f && c.off[2] == 0.f);
}

static void testPlaceBoolWords() {
    const char* trueWords[] = {"1", "on", "true", "yes", "ON", "True", "YES"};
    for (const char* w : trueWords) {
        TuneCfg c; c.place = false;
        KT_CHECK(applyTuneKey(c, "place", w));
        KT_CHECK(c.place);
    }
    const char* falseWords[] = {"0", "off", "false", "no", "OFF", "False", "NO"};
    for (const char* w : falseWords) {
        TuneCfg c = base();
        KT_CHECK(applyTuneKey(c, "place", w));
        KT_CHECK(!c.place);
    }
}

static void testPlaceBadWordLeavesCfg() {
    const char* bad[] = {"2", "maybe", "", "onn", "y", "t", "-1"};
    for (const char* w : bad) {
        TuneCfg c = base();
        TuneCfg before = c;
        KT_CHECK(!applyTuneKey(c, "place", w));
        KT_CHECK(same(c, before));
    }
}

static void testPlaceRotCommaForms() {
    TuneCfg c;
    KT_CHECK(applyTuneKey(c, "place_rot", "-90,0,0"));
    KT_CHECK(c.rot[0] == -90.f && c.rot[1] == 0.f && c.rot[2] == 0.f);
    KT_CHECK(applyTuneKey(c, "place_rot", "-90, 0, 0"));
    KT_CHECK(c.rot[0] == -90.f && c.rot[1] == 0.f && c.rot[2] == 0.f);
    KT_CHECK(applyTuneKey(c, "place_rot", "12.5,-0.25,45"));
    KT_CHECK(c.rot[0] == 12.5f && c.rot[1] == -0.25f && c.rot[2] == 45.f);
    KT_CHECK(c.place == false);           // place_rot never touches place
}

static void testPlaceOffCommaForms() {
    TuneCfg c;
    KT_CHECK(applyTuneKey(c, "place_off", "0.12,0,-0.03"));
    KT_CHECK(c.off[0] == 0.12f && c.off[1] == 0.f && c.off[2] == -0.03f);
    KT_CHECK(c.rot[0] == 0.f && c.rot[1] == 0.f && c.rot[2] == 0.f);   // place_off never touches rot
}

static void testVectorWrongShape() {
    const char* bad[] = {"1 2 3", "1,2", "1,2,3,4", "1,2,", ",1,2", "1,2,3x", "", "   ", "1,,2", "1;2;3"};
    for (const char* v : bad) {
        TuneCfg c = base();
        TuneCfg before = c;
        KT_CHECK(!applyTuneKey(c, "place_rot", v));
        KT_CHECK(!applyTuneKey(c, "place_off", v));
        KT_CHECK(same(c, before));
    }
}

static void testVectorNonFinite() {
    const char* bad[] = {"nan,0,0", "inf,0,0", "0,-inf,0", "0,0,NaN", "0,0,INFINITY"};
    for (const char* v : bad) {
        TuneCfg c = base();
        TuneCfg before = c;
        KT_CHECK(!applyTuneKey(c, "place_rot", v));
        KT_CHECK(!applyTuneKey(c, "place_off", v));
        KT_CHECK(same(c, before));
    }
}

static void testRotRangeBoundary() {
    TuneCfg c = base();
    TuneCfg before = c;
    KT_CHECK(!applyTuneKey(c, "place_rot", "721,0,0"));
    KT_CHECK(same(c, before));
    KT_CHECK(!applyTuneKey(c, "place_rot", "0,-721,0"));
    KT_CHECK(same(c, before));
    KT_CHECK(applyTuneKey(c, "place_rot", "180,-720,720"));   // inclusive bound
    KT_CHECK(c.rot[0] == 180.f && c.rot[1] == -720.f && c.rot[2] == 720.f);
}

static void testOffRangeBoundary() {
    TuneCfg c = base();
    TuneCfg before = c;
    KT_CHECK(!applyTuneKey(c, "place_off", "3.5,0,0"));
    KT_CHECK(same(c, before));
    KT_CHECK(!applyTuneKey(c, "place_off", "0,0,-3.01"));
    KT_CHECK(same(c, before));
    KT_CHECK(applyTuneKey(c, "place_off", "3.0,-3.0,0"));     // inclusive bound
    KT_CHECK(c.off[0] == 3.f && c.off[1] == -3.f && c.off[2] == 0.f);
}

static void testRejectedVectorNotHalfApplied() {
    // First two components are valid; the third is out of range. Nothing may be written.
    TuneCfg c = base();
    TuneCfg before = c;
    KT_CHECK(!applyTuneKey(c, "place_rot", "10,20,721"));
    KT_CHECK(same(c, before));
    KT_CHECK(!applyTuneKey(c, "place_off", "0.1,0.2,3.5"));
    KT_CHECK(same(c, before));
}

static void testKeyCaseInsensitive() {
    TuneCfg c;
    KT_CHECK(applyTuneKey(c, "PLACE_ROT", "-90,0,0"));
    KT_CHECK(c.rot[0] == -90.f);
    KT_CHECK(applyTuneKey(c, "Place_Off", "0.12,0,-0.03"));
    KT_CHECK(c.off[0] == 0.12f && c.off[2] == -0.03f);
    KT_CHECK(applyTuneKey(c, "PLACE", "on"));
    KT_CHECK(c.place);
}

static void testUnknownAndNullKeys() {
    TuneCfg c = base();
    TuneCfg before = c;
    KT_CHECK(!applyTuneKey(c, "place_foo", "1"));
    KT_CHECK(!applyTuneKey(c, "placerot", "1,2,3"));
    KT_CHECK(!applyTuneKey(c, "", "1"));
    KT_CHECK(!applyTuneKey(c, "place_rot", nullptr));
    KT_CHECK(!applyTuneKey(c, nullptr, "on"));
    KT_CHECK(same(c, before));
}

int main() {
    testDefaultIsZero();
    testPlaceBoolWords();
    testPlaceBadWordLeavesCfg();
    testPlaceRotCommaForms();
    testPlaceOffCommaForms();
    testVectorWrongShape();
    testVectorNonFinite();
    testRotRangeBoundary();
    testOffRangeBoundary();
    testRejectedVectorNotHalfApplied();
    testKeyCaseInsensitive();
    testUnknownAndNullKeys();
    return kt::Report("holster_tune");
}
