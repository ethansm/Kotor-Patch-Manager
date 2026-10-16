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

static void testCategoryOverride() {
    TuneCfg c = base();
    KT_CHECK(applyTuneKey(c, "place_rot_pistol", "-90,0,0"));
    KT_CHECK(applyTuneKey(c, "PLACE_OFF_Rifle", "0.1,-0.05,0"));
    KT_CHECK(c.cat[CatPistol].hasRot && !c.cat[CatPistol].hasOff && c.cat[CatPistol].rot[0] == -90.f);
    KT_CHECK(c.cat[CatRifle].hasOff && !c.cat[CatRifle].hasRot);
    float r[3], o[3];
    resolveTune(c, CatPistol, r, o);   // rot override, global offset
    KT_CHECK(r[0] == -90.f && r[1] == 0.f && o[0] == 0.5f);
    resolveTune(c, CatRifle, r, o);    // global rot, offset override
    KT_CHECK(r[0] == 1.f && r[2] == 3.f && o[0] == 0.1f && o[1] == -0.05f);
    resolveTune(c, CatLightsaber, r, o);
    KT_CHECK(r[0] == 1.f && o[2] == 0.5f);
    resolveTune(c, CatUnknown, r, o);
    KT_CHECK(r[1] == 2.f);
    clearCatTune(c);
    resolveTune(c, CatPistol, r, o);
    KT_CHECK(r[0] == 1.f && !c.cat[CatPistol].hasRot);
}

static void testCategoryKeyRejects() {
    TuneCfg c = base();
    KT_CHECK(!applyTuneKey(c, "place_rot_unknown", "1,2,3"));
    KT_CHECK(!applyTuneKey(c, "place_rot_", "1,2,3"));
    KT_CHECK(!applyTuneKey(c, "place_rot_pistolx", "1,2,3"));
    KT_CHECK(!applyTuneKey(c, "place_rotpistol", "1,2,3"));
    KT_CHECK(!applyTuneKey(c, "place_rot_pistol", "1,2"));
    KT_CHECK(!applyTuneKey(c, "place_off_heavy", "4,0,0"));
    KT_CHECK(!c.cat[CatPistol].hasRot && !c.cat[CatHeavy].hasOff);
    KT_CHECK(isTuneKey("place_rot_heavy") && isTuneKey("place") && isTuneKey("Place_Off"));
    KT_CHECK(!isTuneKey("place_gobvt") && !isTuneKey("place_rot_x") && !isTuneKey(nullptr));
}

static void testCategoryBone() {
    TuneCfg c = base();
    KT_CHECK(resolveBone(c, CatRifle) == nullptr);
    KT_CHECK(applyTuneKey(c, "place_bone_rifle", "torsoUpr_g"));
    KT_CHECK(resolveBone(c, CatRifle) && strcmp(resolveBone(c, CatRifle), "torsoUpr_g") == 0);
    KT_CHECK(resolveBone(c, CatPistol) == nullptr && resolveBone(c, CatUnknown) == nullptr);
    KT_CHECK(!applyTuneKey(c, "place_bone_melee", ""));
    KT_CHECK(!applyTuneKey(c, "place_bone_melee", "torso g"));
    KT_CHECK(!applyTuneKey(c, "place_bone_melee", "a234567890123456789012345678901b"));   // 32 chars
    KT_CHECK(applyTuneKey(c, "place_bone_melee", "a23456789012345678901234567890b"));     // 31 chars
    KT_CHECK(!applyTuneKey(c, "place_bone_rifle", "bad-name"));
    KT_CHECK(strcmp(resolveBone(c, CatRifle), "torsoUpr_g") == 0);   // rejected value leaves the old bone
    KT_CHECK(isTuneKey("PLACE_BONE_Heavy") && !isTuneKey("place_bone_") && !isTuneKey("place_bonex_rifle"));
    clearCatTune(c);
    KT_CHECK(resolveBone(c, CatRifle) == nullptr);
}

int main() {
    testCategoryBone();
    testCategoryOverride();
    testCategoryKeyRejects();
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
