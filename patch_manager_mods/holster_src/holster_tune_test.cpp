// T1 host tests for HolsterTune.h (ini tuning-key parser for the holstered-weapons mod; doc 42).
// Build/run: python3 scripts/run_tests.py holster_tune
#include "HolsterTune.h"
#include "HolsterQuat.h"
#include "check.h"
#include <cmath>
#include <stdio.h>
#include <string.h>
#include <string>

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

static bool closeF(float a, float b) { return std::fabs(a - b) <= 1e-5f; }
static bool closeV3(const float* v, float a, float b, float c) { return closeF(v[0], a) && closeF(v[1], b) && closeF(v[2], c); }

// Global rot (10,20,30), global off (0.1,0.2,0.3); pistol type rot (1,2,3) off (0.4,0.5,0.6) bone pelvis_g;
// left pistol rot (11,12,13); model w_Blaster rot (7,8,9) bone rthigh_g; left model w_Blaster off (0.9,0.8,0.7);
// body Juhani off (0.05,0.06,0.07).
// left_extra_rot is 0 here: these fixtures pin the plain mirror (the extra has its own tests below).
static TuneCfg fixture() {
    TuneCfg c;
    c.leftExtra = 0.f;
    KT_CHECK(applyTuneKey(c, "place_rot", "10,20,30"));
    KT_CHECK(applyTuneKey(c, "place_off", "0.1,0.2,0.3"));
    KT_CHECK(applyTuneKey(c, "place_rot_pistol", "1,2,3"));
    KT_CHECK(applyTuneKey(c, "place_off_pistol", "0.4,0.5,0.6"));
    KT_CHECK(applyTuneKey(c, "place_bone_pistol", "pelvis_g"));
    KT_CHECK(applyTuneKey(c, "place_rot_l_pistol", "11,12,13"));
    KT_CHECK(applyTuneKey(c, "place_rot_w_Blaster", "7,8,9"));
    KT_CHECK(applyTuneKey(c, "place_bone_w_Blaster", "rthigh_g"));
    KT_CHECK(applyTuneKey(c, "place_off_l_w_Blaster", "0.9,0.8,0.7"));
    KT_CHECK(applyTuneKey(c, "place_off_body_Juhani", "0.05,0.06,0.07"));
    return c;
}

static void testDefaultIsZero() {
    TuneCfg c;
    KT_CHECK(!c.place);
    KT_CHECK(c.rot[0] == 0.f && c.rot[1] == 0.f && c.rot[2] == 0.f);
    KT_CHECK(c.off[0] == 0.f && c.off[1] == 0.f && c.off[2] == 0.f);
    KT_CHECK(c.nMdl == 0 && c.nBody == 0 && c.bootBoneIdx == 0);
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
    KT_CHECK(!applyTuneKey(c, "place_bone_melee", "a234567890123456789012345678901b"));   // 32 chars, not a table bone
    KT_CHECK(!applyTuneKey(c, "place_bone_melee", "a23456789012345678901234567890b"));     // 31 chars, not a table bone (rejected)
    KT_CHECK(!applyTuneKey(c, "place_bone_rifle", "bad-name"));
    KT_CHECK(strcmp(resolveBone(c, CatRifle), "torsoUpr_g") == 0);   // rejected value leaves the old bone
    KT_CHECK(resolveBone(c, CatMelee) == nullptr);
    KT_CHECK(applyTuneKey(c, "PLACE_BONE_RIFLE", "TORSO_G"));         // case-insensitive, stored as the table spelling
    KT_CHECK(strcmp(resolveBone(c, CatRifle), "torso_g") == 0);
    KT_CHECK(isTuneKey("PLACE_BONE_Heavy") && !isTuneKey("place_bone_") && !isTuneKey("place_bonex_rifle"));
    clearCatTune(c);
    KT_CHECK(resolveBone(c, CatRifle) == nullptr);
}

static void testBoneTable() {
    KT_CHECK(boneIndexOf("torsoUpr_g") == 1 && boneIndexOf("torso_g") == 2 && boneIndexOf("pelvis_g") == 3);
    KT_CHECK(boneIndexOf("rthigh_g") == 4 && boneIndexOf("lthigh_g") == 5);
    KT_CHECK(boneIndexOf("RTHIGH_G") == 4);
    KT_CHECK(boneIndexOf("torso") == 0 && boneIndexOf("torsoUpr_g2") == 0 && boneIndexOf("") == 0 && boneIndexOf(nullptr) == 0);
    KT_CHECK(boneNameOf(0) == nullptr && boneNameOf(6) == nullptr);
    KT_CHECK(strcmp(boneNameOf(4), "rthigh_g") == 0);
    TuneCfg c;
    KT_CHECK(defaultBone(c) == 0);
    c.bootBoneIdx = 2;
    KT_CHECK(defaultBone(c) == 2);
    KT_CHECK(sizeof(TuneCfg) <= 4096);
}

static void testModelEntryParsing() {
    TuneCfg c;
    KT_CHECK(applyTuneKey(c, "place_rot_w_Blaster", "1,2,3"));
    KT_CHECK(applyTuneKey(c, "PLACE_OFF_W_BLASTER", "0.1,0.2,0.3"));   // same entry (case-insensitive name)
    KT_CHECK(c.nMdl == 1);
    KT_CHECK(strcmp(c.mdl[0].model, "w_Blaster") == 0 && !c.mdl[0].left);   // stored as written
    KT_CHECK(c.mdl[0].t.hasRot && c.mdl[0].t.hasOff && !c.mdl[0].t.boneIdx);
    KT_CHECK(applyTuneKey(c, "place_rot_w_blaster", "5,6,7"));         // re-set updates in place
    KT_CHECK(c.nMdl == 1 && closeV3(c.mdl[0].t.rot, 5, 6, 7));
    KT_CHECK(applyTuneKey(c, "place_rot_l_w_Blaster", "1,1,1"));       // left entry is a second entry
    KT_CHECK(c.nMdl == 2 && c.mdl[1].left && strcmp(c.mdl[1].model, "w_Blaster") == 0);
    KT_CHECK(applyTuneKey(c, "place_bone_w_blaster", "pelvis_g"));
    KT_CHECK(c.nMdl == 2 && c.mdl[0].t.boneIdx == 3 && c.mdl[1].t.boneIdx == 0);
    KT_CHECK(applyTuneKey(c, "place_off_body_Juhani", "0.05,0.06,0.07"));
    KT_CHECK(applyTuneKey(c, "place_off_body_JUHANI", "0.5,0.6,0.7"));  // same body entry
    KT_CHECK(c.nBody == 1 && strcmp(c.body[0].body, "Juhani") == 0 && closeF(c.body[0].off[0], 0.5f));
}

static void testKeyShapes() {
    struct Case { const char* key; const char* val; };
    const Case ok[] = {
        {"place_rot_l_pistol", "1,2,3"},
        {"place_off_l_pistol", "0.1,0,0"},
        {"place_bone_l_rifle", "rthigh_g"},
        {"place_off_body_Juhani", "0.1,0,0"},
        {"place_off_body_a_1", "0,0,0"},
        {"place_off_body_0123456789abcdef", "0,0,0"},      // 16 chars
        {"place_rot_w_Blaster", "1,2,3"},
        {"place_off_w_Blaster_01", "0,0,0"},
        {"place_bone_w_Blaster", "pelvis_g"},
        {"place_rot_l_w_Blaster", "1,2,3"},
        {"place_off_l_w_Blaster", "0,0,0"},
        {"place_bone_l_w_Blaster", "torso_g"},
        {"place_rot_w_0123456789abcd", "1,2,3"},           // "w_" + 14 chars = 16
        {"PLACE_ROT_L_W_BLASTER", "1,2,3"},
        {"Place_Off_Body_JUHANI", "0,0,0"},
    };
    for (const Case& k : ok) {
        TuneCfg c;
        KT_CHECK(isTuneKey(k.key));
        KT_CHECK(applyTuneKey(c, k.key, k.val));
    }
    const char* bad[] = {
        "place_rot_body_x", "place_bone_body_x", "place_off_body_", "place_off_body_0123456789abcdefg", "place_off_body_a-b",
        "place_off_body_a.b", "place_off_l_body_x", "place_rot_l_", "place_rot_l_l_pistol", "place_rot_w_a-b",
        "place_rot_w_0123456789abcdef", "place_rot_x_w_a", "place_rot_w", "place_rot_l", "place_rot_body", "place_off_body",
        "place_bone_l_w_x-y", "place_gobvt", "place_rot_", "placerot_w_x",
    };
    TuneCfg c;
    for (const char* k : bad) {
        KT_CHECK(!isTuneKey(k));
        KT_CHECK(!applyTuneKey(c, k, "0.1,0,0"));
        KT_CHECK(!applyTuneKey(c, k, "pelvis_g"));
    }
    KT_CHECK(c.nMdl == 0 && c.nBody == 0);
}

static void testModelTableFull() {
    TuneCfg c;
    char key[32];
    for (int i = 0; i < 48; ++i) {
        snprintf(key, sizeof key, "place_rot_w_m%02d", i);
        KT_CHECK(applyTuneKey(c, key, "1,2,3"));
    }
    KT_CHECK(c.nMdl == 48);
    KT_CHECK(!applyTuneKey(c, "place_rot_w_m48", "1,2,3"));
    KT_CHECK(isTuneKey("place_rot_w_m48") && isTuneKey("place_rot_l_w_m48"));   // syntax still accepted
    KT_CHECK(c.nMdl == 48);
    KT_CHECK(!applyTuneKey(c, "place_rot_l_w_m00", "1,2,3"));                // a new left entry needs a slot too
    KT_CHECK(!applyTuneKey(c, "place_bone_w_m48", "pelvis_g"));
    KT_CHECK(applyTuneKey(c, "place_off_w_m05", "0.1,0.2,0.3"));            // existing entry still updates
    KT_CHECK(c.nMdl == 48 && c.mdl[5].t.hasOff && closeV3(c.mdl[5].t.off, 0.1f, 0.2f, 0.3f));
}

static void testBodyTableFull() {
    TuneCfg c;
    char key[40];
    for (int i = 0; i < 24; ++i) {
        snprintf(key, sizeof key, "place_off_body_b%02d", i);
        KT_CHECK(applyTuneKey(c, key, "0.1,0,0"));
    }
    KT_CHECK(c.nBody == 24);
    KT_CHECK(!applyTuneKey(c, "place_off_body_b24", "0.1,0,0"));
    KT_CHECK(isTuneKey("place_off_body_b24"));
    KT_CHECK(applyTuneKey(c, "place_off_body_B03", "0,0.5,0"));              // existing name updates in place
    KT_CHECK(c.nBody == 24 && closeF(c.body[3].off[1], 0.5f));
}

static void testRejectedValueClaimsNoSlot() {
    TuneCfg c;
    KT_CHECK(!applyTuneKey(c, "place_rot_w_New", "1,2,721"));
    KT_CHECK(c.nMdl == 0);
    KT_CHECK(!applyTuneKey(c, "place_bone_w_New", "not_a_bone"));
    KT_CHECK(c.nMdl == 0);
    KT_CHECK(!applyTuneKey(c, "place_off_body_NewBody", "4,0,0"));
    KT_CHECK(c.nBody == 0);
}

static void testResolveRightMatrix() {
    const TuneCfg c = fixture();
    Resolved r;
    // model rot, type off + body delta, model bone
    resolveFull(c, "w_Blaster", CatPistol, "Juhani", false, false, r);
    KT_CHECK(closeV3(r.rot, 7, 8, 9) && r.src[0] == 2);
    KT_CHECK(closeV3(r.off, 0.45f, 0.56f, 0.67f) && r.src[1] == (1 | 4));
    KT_CHECK(r.boneIdx == 4 && r.src[2] == 2);
    // excludeBody: no delta, no |4
    resolveFull(c, "w_Blaster", CatPistol, "Juhani", false, true, r);
    KT_CHECK(closeV3(r.off, 0.4f, 0.5f, 0.6f) && r.src[1] == 1);
    // body name is case-insensitive
    resolveFull(c, "w_Blaster", CatPistol, "juhani", false, false, r);
    KT_CHECK(closeV3(r.off, 0.45f, 0.56f, 0.67f) && r.src[1] == 5);
    // body not in the table: no delta
    resolveFull(c, "w_Blaster", CatPistol, "Nobody", false, false, r);
    KT_CHECK(closeV3(r.off, 0.4f, 0.5f, 0.6f) && r.src[1] == 1);
    // no model: type rot, type off, type bone
    resolveFull(c, nullptr, CatPistol, nullptr, false, false, r);
    KT_CHECK(closeV3(r.rot, 1, 2, 3) && r.src[0] == 1);
    KT_CHECK(closeV3(r.off, 0.4f, 0.5f, 0.6f) && r.src[1] == 1);
    KT_CHECK(r.boneIdx == 3 && r.src[2] == 1);
    // empty strings count as no model / no body
    resolveFull(c, "", CatPistol, "", false, false, r);
    KT_CHECK(closeV3(r.rot, 1, 2, 3) && r.src[0] == 1 && r.src[1] == 1);
    // unknown model, category without overrides: global, no bone
    resolveFull(c, "w_Other", CatRifle, nullptr, false, false, r);
    KT_CHECK(closeV3(r.rot, 10, 20, 30) && r.src[0] == 0);
    KT_CHECK(closeV3(r.off, 0.1f, 0.2f, 0.3f) && r.src[1] == 0);
    KT_CHECK(r.boneIdx == 0 && r.src[2] == 0);
    // model is case-insensitive
    resolveFull(c, "W_BLASTER", CatPistol, nullptr, false, false, r);
    KT_CHECK(closeV3(r.rot, 7, 8, 9) && r.src[0] == 2);
    // CatUnknown: model rot and bone, global off
    resolveFull(c, "w_Blaster", CatUnknown, nullptr, false, false, r);
    KT_CHECK(closeV3(r.rot, 7, 8, 9) && closeV3(r.off, 0.1f, 0.2f, 0.3f) && r.src[1] == 0 && r.boneIdx == 4);
    // the left-only model offset never reaches the right hand
    resolveFull(c, "w_Blaster", CatPistol, nullptr, false, false, r);
    KT_CHECK(closeV3(r.off, 0.4f, 0.5f, 0.6f));
}

static void testResolveLeftMatrix() {
    const TuneCfg c = fixture();
    Resolved r;
    // No left model rot, so rot = right model mirrored (7,8,9 -> 7,-8,-9), which beats left-type 11,12,13.
    // off = left model (0.9,0.8,0.7, not mirrored) plus the body delta with x negated.
    // bone = right model rthigh_g mirrored to lthigh_g.
    resolveFull(c, "w_Blaster", CatPistol, "Juhani", true, false, r);
    KT_CHECK(closeV3(r.rot, 7, -8, -9) && r.src[0] == 2);
    KT_CHECK(closeV3(r.off, 0.85f, 0.86f, 0.77f) && r.src[1] == (2 | 4));
    KT_CHECK(r.boneIdx == 5 && r.src[2] == 2);
    // excludeBody: left-model off, no delta
    resolveFull(c, "w_Blaster", CatPistol, "Juhani", true, true, r);
    KT_CHECK(closeV3(r.off, 0.9f, 0.8f, 0.7f) && r.src[1] == 2);
    // no model: left-type rot (not mirrored); right-type off mirrored; right-type bone pelvis_g (no swap)
    resolveFull(c, nullptr, CatPistol, nullptr, true, false, r);
    KT_CHECK(closeV3(r.rot, 11, 12, 13) && r.src[0] == 1);
    KT_CHECK(closeV3(r.off, -0.4f, 0.5f, 0.6f) && r.src[1] == 1);
    KT_CHECK(r.boneIdx == 3 && r.src[2] == 1);
    // category with nothing set: global mirrored, no bone
    resolveFull(c, nullptr, CatRifle, nullptr, true, false, r);
    KT_CHECK(closeV3(r.rot, 10, -20, -30) && r.src[0] == 0);
    KT_CHECK(closeV3(r.off, -0.1f, 0.2f, 0.3f) && r.src[1] == 0);
    KT_CHECK(r.boneIdx == 0 && r.src[2] == 0);
    // body delta with no model and no category: global mirrored, x of the delta negated
    resolveFull(c, nullptr, CatRifle, "Juhani", true, false, r);
    KT_CHECK(closeV3(r.off, -0.15f, 0.26f, 0.37f) && r.src[1] == 4);
}

static void testResolveLeftOverrides() {
    TuneCfg c = fixture();
    Resolved r;
    // a left model rot is used as written (not mirrored); the right model rot is then unused on the left
    KT_CHECK(applyTuneKey(c, "place_rot_l_w_Blaster", "100,200,300"));
    KT_CHECK(applyTuneKey(c, "place_bone_l_w_Blaster", "torso_g"));
    resolveFull(c, "w_Blaster", CatPistol, nullptr, true, false, r);
    KT_CHECK(closeV3(r.rot, 100, 200, 300) && r.src[0] == 2);
    KT_CHECK(r.boneIdx == 2 && r.src[2] == 2);   // left bone not swapped
    // a left category bone is used as written; a right category bone is mirrored
    TuneCfg d = fixture();
    KT_CHECK(applyTuneKey(d, "place_bone_rifle", "rthigh_g"));
    resolveFull(d, nullptr, CatRifle, nullptr, true, false, r);
    KT_CHECK(r.boneIdx == 5 && r.src[2] == 1);
    KT_CHECK(applyTuneKey(d, "place_bone_l_rifle", "rthigh_g"));
    resolveFull(d, nullptr, CatRifle, nullptr, true, false, r);
    KT_CHECK(r.boneIdx == 4 && r.src[2] == 1);
    resolveFull(d, nullptr, CatRifle, nullptr, false, false, r);
    KT_CHECK(r.boneIdx == 4 && r.src[2] == 1);
}

static void testMirrorInvolutionAndBones() {
    float rot[3] = {10, 20, 30}, off[3] = {0.1f, 0.2f, 0.3f};
    uint8_t b = 4;
    mirrorLeft(rot, off, b);
    KT_CHECK(closeV3(rot, 10, -20, -30) && closeV3(off, -0.1f, 0.2f, 0.3f) && b == 5);
    mirrorLeft(rot, off, b);
    KT_CHECK(rot[0] == 10.f && rot[1] == 20.f && rot[2] == 30.f);   // twice = identity
    KT_CHECK(off[0] == 0.1f && off[1] == 0.2f && off[2] == 0.3f && b == 4);
    for (uint8_t i = 0; i <= kBoneCount; ++i) {
        uint8_t x = i;
        mirrorBone(x);
        const uint8_t expect = (i == 4) ? 5 : (i == 5) ? 4 : i;
        KT_CHECK(x == expect);
        mirrorBone(x);
        KT_CHECK(x == i);
    }
}

// qbase(mirrored rot) must be the reflection of qbase(rot): (w, x, -y, -z), up to the sign of the quaternion.
static bool nearQ(const Q& a, const Q& b) { return closeF(a.w, b.w) && closeF(a.x, b.x) && closeF(a.y, b.y) && closeF(a.z, b.z); }
static void testMirrorIsQuaternionReflection() {
    const float rots[5][3] = {{10, 20, 30}, {-90, 0, 0}, {33, -45, 120}, {0, 90, 0}, {170, -65, 5}};
    for (int k = 0; k < 5; ++k) {
        float m[3] = {rots[k][0], rots[k][1], rots[k][2]};
        float o[3] = {0.f, 0.f, 0.f};
        uint8_t b = 0;
        mirrorLeft(m, o, b);
        const Q q = qbase(rots[k][0], rots[k][1], rots[k][2]);
        const Q qm = qbase(m[0], m[1], m[2]);
        const Q refl = {q.w, q.x, -q.y, -q.z};
        const Q neg = {-refl.w, -refl.x, -refl.y, -refl.z};
        KT_CHECK(nearQ(qm, refl) || nearQ(qm, neg));
    }
}

static void testClearDropsNewTables() {
    TuneCfg c = fixture();
    KT_CHECK(c.nMdl > 0 && c.nBody > 0 && c.catL[CatPistol].hasRot);
    clearCatTune(c);
    KT_CHECK(c.nMdl == 0 && c.nBody == 0);
    KT_CHECK(c.leftExtra == kLeftExtraDefault && !c.hasLeftExtraCat[CatRifle]);   // the extra is an ini key: a reload re-reads it
    c.leftExtra = 0.f;
    KT_CHECK(!c.catL[CatPistol].hasRot && !c.cat[CatPistol].hasRot && !c.mdl[0].t.hasRot && !c.body[0].off[0]);
    KT_CHECK(closeV3(c.rot, 10, 20, 30) && closeV3(c.off, 0.1f, 0.2f, 0.3f));   // globals are not cleared
    Resolved r;
    resolveFull(c, "w_Blaster", CatPistol, "Juhani", true, false, r);
    KT_CHECK(closeV3(r.rot, 10, -20, -30) && r.src[0] == 0);
    KT_CHECK(closeV3(r.off, -0.1f, 0.2f, 0.3f) && r.src[1] == 0);
    KT_CHECK(r.boneIdx == 0 && r.src[2] == 0);
}


// ---- left_extra_rot ----

static float dist3(const float* a, const float* b) {
    return std::sqrt((a[0] - b[0]) * (a[0] - b[0]) + (a[1] - b[1]) * (a[1] - b[1]) + (a[2] - b[2]) * (a[2] - b[2]));
}

// Minimum distance between the right and the left rifle barrel line (the rifle barrel axis of H4 for rot -60,4,93), sampled over +-0.6 m.
static float rifleBarrelGap(const Resolved& r, const Resolved& l) {
    const float dR[3] = {-0.496f, -0.086f, -0.864f};
    float d0[3], dL[3];
    qrotate(qconj(qbase(r.rot[0], r.rot[1], r.rot[2])), dR, d0);
    qrotate(qbase(l.rot[0], l.rot[1], l.rot[2]), d0, dL);
    float best = 1e9f;
    for (float s = -0.6f; s <= 0.6f; s += 0.004f)
        for (float t = -0.6f; t <= 0.6f; t += 0.004f) {
            float a[3], b[3];
            for (int i = 0; i < 3; ++i) { a[i] = r.off[i] + s * dR[i]; b[i] = l.off[i] + t * dL[i]; }
            const float d = dist3(a, b);
            if (d < best) best = d;
        }
    return best;
}

static TuneCfg rifleCfg(float ex) {
    TuneCfg c;
    c.leftExtra = ex;
    KT_CHECK(applyTuneKey(c, "place_bone_rifle", "torsoUpr_g"));
    KT_CHECK(applyTuneKey(c, "place_rot_rifle", "-60,4,93"));
    KT_CHECK(applyTuneKey(c, "place_off_rifle", "0.12,-0.11,0.20"));
    return c;
}

static void testLeftExtraKeys() {
    TuneCfg c;
    KT_CHECK(c.leftExtra == -8.f && kLeftExtraDefault == -8.f);   // the shipped default
    KT_CHECK(isTuneKey("left_extra_rot") && isTuneKey("LEFT_EXTRA_ROT_Rifle") && isLeftExtraKey("left_extra_rot_zzz"));
    KT_CHECK(!isLeftExtraKey("left_extra") && !isLeftExtraKey(nullptr) && !isLeftExtraKey("place_rot"));
    KT_CHECK(applyTuneKey(c, "left_extra_rot", "-12.5") && c.leftExtra == -12.5f);
    KT_CHECK(applyTuneKey(c, "left_extra_rot", " 0 ") && c.leftExtra == 0.f);
    KT_CHECK(applyTuneKey(c, "left_extra_rot", "720") && c.leftExtra == 720.f);   // parser limit; the USED value is clamped
    KT_CHECK(!applyTuneKey(c, "left_extra_rot", "720.5") && c.leftExtra == 720.f);   // rejected, unchanged
    KT_CHECK(!applyTuneKey(c, "left_extra_rot", "") && !applyTuneKey(c, "left_extra_rot", "abc") && !applyTuneKey(c, "left_extra_rot", "1,2")
             && !applyTuneKey(c, "left_extra_rot", "nan") && !applyTuneKey(c, "left_extra_rot", "inf") && c.leftExtra == 720.f);
    KT_CHECK(applyTuneKey(c, "left_extra_rot_rifle", "-12") && c.hasLeftExtraCat[CatRifle] && c.leftExtraCat[CatRifle] == -12.f);
    KT_CHECK(!c.hasLeftExtraCat[CatHeavy]);
    KT_CHECK(!applyTuneKey(c, "left_extra_rot_banana", "-3") && !applyTuneKey(c, "left_extra_rot_rifle", "x") && c.leftExtraCat[CatRifle] == -12.f);
    // use-time clamp: global 720 -> +45, per-category wins over global, NaN -> 0
    KT_CHECK(leftExtraFor(c, CatHeavy) == 45.f && leftExtraFor(c, CatUnknown) == 45.f && leftExtraFor(c, CatRifle) == -12.f);
    KT_CHECK(applyTuneKey(c, "left_extra_rot", "-100") && leftExtraFor(c, CatMelee) == -45.f);
    KT_CHECK(leftExtraClamped(-100.f) && leftExtraClamped(45.5f) && !leftExtraClamped(-45.f) && !leftExtraClamped(-8.f));
    c.leftExtra = NAN;
    KT_CHECK(leftExtraFor(c, CatMelee) == 0.f);
    clearCatTune(c);
    KT_CHECK(c.leftExtra == -8.f && !c.hasLeftExtraCat[CatRifle]);
}

static void testLeftExtraRifleVectors() {
    Resolved r, l;
    // ex = 0 reproduces the plain mirror exactly
    TuneCfg c0 = rifleCfg(0.f);
    resolveFull(c0, nullptr, CatRifle, nullptr, true, false, l);
    KT_CHECK(closeV3(l.rot, -60, -4, -93) && closeV3(l.off, -0.12f, -0.11f, 0.20f) && l.extraDeg == 0.f && l.leftMirror == 3);
    resolveFull(c0, nullptr, CatRifle, nullptr, false, false, r);
    KT_CHECK(rifleBarrelGap(r, l) < 0.005f);   // barrels touch at x = 0 (H4)
    // ex = -8: H4 vector, 2.9 cm
    TuneCfg c8 = rifleCfg(-8.f);
    resolveFull(c8, nullptr, CatRifle, nullptr, true, false, l);
    KT_CHECK(std::fabs(l.rot[0] + 59.57f) < 0.05f && std::fabs(l.rot[1] + 11.99f) < 0.05f && std::fabs(l.rot[2] + 93.06f) < 0.05f);
    KT_CHECK(closeV3(l.off, -0.12f, -0.11f, 0.20f) && l.extraDeg == -8.f && l.src[0] == 1 && l.leftMirror == 3);
    KT_CHECK(std::fabs(rifleBarrelGap(r, l) - 0.0293f) < 0.002f);
    // the extra is a quaternion composition: qbase(result) == qaxis(X, ex) * qbase(mirrored)
    const float xa[3] = {1.f, 0.f, 0.f};
    const Q want = qmul(qaxis(xa, -8.f), qbase(-60.f, -4.f, -93.f));
    const Q got = qbase(l.rot[0], l.rot[1], l.rot[2]);
    const float dot = want.w * got.w + want.x * got.x + want.y * got.y + want.z * got.z;
    KT_CHECK(std::fabs(std::fabs(dot) - 1.f) < 1e-5f);
    // the right hand never gets the extra
    resolveFull(c8, nullptr, CatRifle, nullptr, false, false, r);
    KT_CHECK(closeV3(r.rot, -60, 4, 93) && r.extraDeg == 0.f && r.leftMirror == 0);
    // ex = -12: 4.3 cm
    TuneCfg c12 = rifleCfg(-12.f);
    resolveFull(c12, nullptr, CatRifle, nullptr, true, false, l);
    KT_CHECK(std::fabs(l.rot[0] + 59.35f) < 0.05f && std::fabs(l.rot[1] + 15.98f) < 0.05f && std::fabs(l.rot[2] + 93.11f) < 0.05f);
    KT_CHECK(std::fabs(rifleBarrelGap(r, l) - 0.0434f) < 0.002f);
}

static void testLeftExtraHeavyMeleeAndCategory() {
    Resolved l;
    // heavy mirrors onto itself (rot -90,-3,-90 -> -90,3,90, exact overlap); the extra separates the barrels
    TuneCfg c;
    c.leftExtra = 0.f;
    KT_CHECK(applyTuneKey(c, "place_rot_heavy", "-90,-3,-90"));
    KT_CHECK(applyTuneKey(c, "place_off_heavy", "0,-0.08,0.25"));
    resolveFull(c, nullptr, CatHeavy, nullptr, true, false, l);
    KT_CHECK(closeV3(l.rot, -90, 3, 90) && l.extraDeg == 0.f);
    c.leftExtra = -8.f;
    resolveFull(c, nullptr, CatHeavy, nullptr, true, false, l);
    const float xa[3] = {1.f, 0.f, 0.f};
    const Q want = qmul(qaxis(xa, -8.f), qbase(-90.f, 3.f, 90.f));
    const Q got = qbase(l.rot[0], l.rot[1], l.rot[2]);
    KT_CHECK(std::fabs(std::fabs(want.w * got.w + want.x * got.x + want.y * got.y + want.z * got.z) - 1.f) < 1e-5f);
    KT_CHECK(l.extraDeg == -8.f && !closeV3(l.rot, -90, 3, 90));
    // per-category override beats the global value; other categories keep the global one
    KT_CHECK(applyTuneKey(c, "left_extra_rot_heavy", "-12"));
    resolveFull(c, nullptr, CatHeavy, nullptr, true, false, l);
    KT_CHECK(l.extraDeg == -12.f);
    resolveFull(c, nullptr, CatMelee, nullptr, true, false, l);
    KT_CHECK(l.extraDeg == -8.f);   // melee has no rot of its own: global mirrored, extra applies
    // a per-category 0 switches the extra off for that category only
    KT_CHECK(applyTuneKey(c, "left_extra_rot_heavy", "0"));
    resolveFull(c, nullptr, CatHeavy, nullptr, true, false, l);
    KT_CHECK(closeV3(l.rot, -90, 3, 90) && l.extraDeg == 0.f);
    // out-of-range raw values are clamped to +-45 when used
    TuneCfg d = rifleCfg(-100.f);
    TuneCfg e = rifleCfg(-45.f);
    Resolved a, b;
    resolveFull(d, nullptr, CatRifle, nullptr, true, false, a);
    resolveFull(e, nullptr, CatRifle, nullptr, true, false, b);
    KT_CHECK(a.extraDeg == -45.f && closeV3(a.rot, b.rot[0], b.rot[1], b.rot[2]));
}

static void testLeftExtraNeverOnExplicit() {
    Resolved l;
    // explicit left category rot: used as written, the extra is ignored
    TuneCfg c = rifleCfg(-8.f);
    KT_CHECK(applyTuneKey(c, "place_rot_l_rifle", "-61,-5,-94"));
    resolveFull(c, nullptr, CatRifle, nullptr, true, false, l);
    KT_CHECK(closeV3(l.rot, -61, -5, -94) && l.extraDeg == 0.f && (l.leftMirror & 1) == 0 && (l.leftMirror & 2) == 2);
    KT_CHECK(std::string(leftSrcName(l, true)) == "mixed");
    // explicit left model rot, too
    KT_CHECK(applyTuneKey(c, "place_rot_l_w_Blaster", "10,20,30"));
    resolveFull(c, "w_Blaster", CatRifle, nullptr, true, false, l);
    KT_CHECK(closeV3(l.rot, 10, 20, 30) && l.extraDeg == 0.f);
    // explicit left OFF only: the rot triple is still inherited as a whole, so the extra applies; off is as written
    TuneCfg d = rifleCfg(-8.f);
    KT_CHECK(applyTuneKey(d, "place_off_l_rifle", "-0.10,-0.12,0.21"));
    resolveFull(d, nullptr, CatRifle, nullptr, true, false, l);
    KT_CHECK(std::fabs(l.rot[1] + 11.99f) < 0.05f && l.extraDeg == -8.f && closeV3(l.off, -0.10f, -0.12f, 0.21f) && l.leftMirror == 1);
    KT_CHECK(std::string(leftSrcName(l, true)) == "mixed");
    // explicit left rot AND off: fully explicit
    KT_CHECK(applyTuneKey(d, "place_rot_l_rifle", "-60,-4,-93"));
    resolveFull(d, nullptr, CatRifle, nullptr, true, false, l);
    KT_CHECK(l.leftMirror == 0 && std::string(leftSrcName(l, true)) == "explicit" && l.extraDeg == 0.f);
    // fully mirrored, and the right hand
    TuneCfg m = rifleCfg(-8.f);
    resolveFull(m, nullptr, CatRifle, nullptr, true, false, l);
    KT_CHECK(std::string(leftSrcName(l, true)) == "mirror");
    resolveFull(m, nullptr, CatRifle, nullptr, false, false, l);
    KT_CHECK(std::string(leftSrcName(l, false)) == "-");
    // a right-model rot mirrored on the left is still an inherited level: the extra applies
    TuneCfg w = rifleCfg(-8.f);
    KT_CHECK(applyTuneKey(w, "place_rot_w_Blaster", "-60,4,93"));
    resolveFull(w, "w_Blaster", CatRifle, nullptr, true, false, l);
    KT_CHECK(l.src[0] == 2 && l.extraDeg == -8.f && std::fabs(l.rot[1] + 11.99f) < 0.05f);
}

static void testLeftExtraBodyDeltaUnaffected() {
    // the extra tilts the rotation only; the offset (and the negated body delta) is the plain mirror
    TuneCfg a = rifleCfg(0.f), b = rifleCfg(-8.f);
    KT_CHECK(applyTuneKey(a, "place_off_body_Juhani", "0.05,0.06,0.07"));
    KT_CHECK(applyTuneKey(b, "place_off_body_Juhani", "0.05,0.06,0.07"));
    Resolved ra, rb;
    resolveFull(a, nullptr, CatRifle, "Juhani", true, false, ra);
    resolveFull(b, nullptr, CatRifle, "Juhani", true, false, rb);
    KT_CHECK(closeV3(rb.off, ra.off[0], ra.off[1], ra.off[2]) && closeV3(ra.off, -0.17f, -0.05f, 0.27f));
}

static void testSizeBudget() {
    static_assert(sizeof(TuneCfg) <= 4096, "TuneCfg budget");
    KT_CHECK(sizeof(TuneCfg) <= 4096);
}

// ---- boneName and scopeKeys (HE2-T1) ----

// Output arrays for one scopeKeys call: 3 set and 3 unset is the most one scope can need.
struct ScopeArrays {
    KV set[3];
    const char* unset[3];
    char buf[kScopeTextMax];
    ScopeOut o;
    ScopeArrays() : set{}, unset{}, buf{}, o{set, 3, 0, unset, 3, 0, buf, sizeof buf, 0} {}
};

static const char* setVal(const ScopeOut& o, const char* key) {
    for (int i = 0; i < o.nSet; ++i)
        if (tuneEqCi(o.set[i].key, key)) return o.set[i].val;
    return nullptr;
}

static bool isUnset(const ScopeOut& o, const char* key) {
    for (int i = 0; i < o.nUnset; ++i)
        if (tuneEqCi(o.unset[i], key)) return true;
    return false;
}

// Replays the set entries through applyTuneKey into a fresh cfg; false when a key is rejected.
static bool applySet(TuneCfg& c, const ScopeOut& o) {
    for (int i = 0; i < o.nSet; ++i)
        if (!applyTuneKey(c, o.set[i].key, o.set[i].val)) return false;
    return true;
}

// Same hasRot / hasOff / bone; the values count only where the flag is set.
static bool sameLevel(const CatTune& a, const CatTune& b) {
    if (a.hasRot != b.hasRot || a.hasOff != b.hasOff || a.boneIdx != b.boneIdx) return false;
    if (a.hasRot && !closeV3(b.rot, a.rot[0], a.rot[1], a.rot[2])) return false;
    if (a.hasOff && !closeV3(b.off, a.off[0], a.off[1], a.off[2])) return false;
    return true;
}

static const float kZero3[3] = {0.f, 0.f, 0.f};   // stands in for a vector the level does not have

static CatTune level(bool hasRot, const float rot[3], bool hasOff, const float off[3], uint8_t bone) {
    CatTune t;
    t.hasRot = hasRot;
    t.hasOff = hasOff;
    t.boneIdx = bone;
    for (int i = 0; i < 3; ++i) { t.rot[i] = hasRot ? rot[i] : 0.f; t.off[i] = hasOff ? off[i] : 0.f; }
    return t;
}

static void testBoneNameForwards() {
    const char* want[kBoneCount] = {"torsoUpr_g", "torso_g", "pelvis_g", "rthigh_g", "lthigh_g"};
    KT_CHECK(boneName(0) == nullptr);
    KT_CHECK(boneName(6) == nullptr);
    KT_CHECK(boneName(255) == nullptr);
    for (uint8_t i = 1; i <= kBoneCount; ++i) {
        KT_CHECK(boneName(i) == boneNameOf(i));   // forwards: the same static string, no copy
        KT_CHECK(strcmp(boneName(i), want[i - 1]) == 0);
    }
}

static void testScopeModelKeysAndRoundTrip() {
    // Right hand, all three fields, bone rthigh_g (4).
    const float rot[3] = {7.f, 8.f, 9.f}, off[3] = {0.9f, 0.8f, 0.7f};
    TuneCfg c;
    ModelTune m;
    tuneCopyName(m.model, "w_Blaster");
    m.left = false;
    m.t = level(true, rot, true, off, 4);
    ScopeArrays a;
    KT_CHECK(scopeKeys(m, a.o));
    KT_CHECK(a.o.nSet == 3 && a.o.nUnset == 0);
    KT_CHECK(strcmp(setVal(a.o, "place_rot_w_Blaster"), "7.0,8.0,9.0") == 0);
    KT_CHECK(strcmp(setVal(a.o, "place_off_w_Blaster"), "0.900,0.800,0.700") == 0);
    KT_CHECK(setVal(a.o, "place_bone_w_Blaster") == boneNameOf(4));   // static string
    TuneCfg fresh;
    KT_CHECK(applySet(fresh, a.o));
    const ModelTune* back = findModel(fresh, "w_Blaster", false);
    KT_CHECK(back && sameLevel(back->t, m.t));

    // Left hand, off only, no bone: rot and bone become unset entries.
    ModelTune L;
    tuneCopyName(L.model, "w_Blaster");
    L.left = true;
    L.t = level(false, kZero3, true, off, 0);
    ScopeArrays b;
    KT_CHECK(scopeKeys(L, b.o));
    KT_CHECK(b.o.nSet == 1 && b.o.nUnset == 2);
    KT_CHECK(strcmp(setVal(b.o, "place_off_l_w_Blaster"), "0.900,0.800,0.700") == 0);
    KT_CHECK(isUnset(b.o, "place_rot_l_w_Blaster") && isUnset(b.o, "place_bone_l_w_Blaster"));
    TuneCfg fresh2;
    KT_CHECK(applySet(fresh2, b.o));
    const ModelTune* backL = findModel(fresh2, "w_Blaster", true);
    KT_CHECK(backL && sameLevel(backL->t, L.t));
    KT_CHECK(findModel(fresh2, "w_Blaster", false) == nullptr);   // the left entry never leaks into the right hand
}

static void testScopeTypeKeysAndRoundTrip() {
    // Fixture-style right pistol: rot, off and bone pelvis_g (3).
    TuneCfg c = fixture();
    ScopeArrays a;
    KT_CHECK(scopeKeys(CatPistol, false, c.cat[CatPistol], a.o));
    KT_CHECK(a.o.nSet == 3 && a.o.nUnset == 0);
    KT_CHECK(strcmp(setVal(a.o, "place_rot_pistol"), "1.0,2.0,3.0") == 0);
    KT_CHECK(strcmp(setVal(a.o, "place_off_pistol"), "0.400,0.500,0.600") == 0);
    KT_CHECK(setVal(a.o, "place_bone_pistol") == boneNameOf(3));
    TuneCfg fresh;
    KT_CHECK(applySet(fresh, a.o));
    KT_CHECK(sameLevel(fresh.cat[CatPistol], c.cat[CatPistol]));

    // Left pistol: rot only (catL has no off and no bone), so off and bone are unset.
    ScopeArrays b;
    KT_CHECK(scopeKeys(CatPistol, true, c.catL[CatPistol], b.o));
    KT_CHECK(b.o.nSet == 1 && b.o.nUnset == 2);
    KT_CHECK(strcmp(setVal(b.o, "place_rot_l_pistol"), "11.0,12.0,13.0") == 0);
    KT_CHECK(isUnset(b.o, "place_off_l_pistol") && isUnset(b.o, "place_bone_l_pistol"));
    TuneCfg fresh2;
    KT_CHECK(applySet(fresh2, b.o));
    KT_CHECK(sameLevel(fresh2.catL[CatPistol], c.catL[CatPistol]));
    KT_CHECK(!fresh2.cat[CatPistol].hasRot);   // the left key never touches the right table

    // Bone 0 (unset) and bone 1 / 5 keep their names through the round trip.
    const float off[3] = {0.1f, 0.2f, 0.3f};
    const uint8_t bones[3] = {0, 1, 5};
    for (uint8_t bone : bones) {
        CatTune t = level(false, kZero3, true, off, bone);
        ScopeArrays d;
        KT_CHECK(scopeKeys(CatHeavy, false, t, d.o));
        KT_CHECK(bone == 0 ? isUnset(d.o, "place_bone_heavy") : setVal(d.o, "place_bone_heavy") == boneNameOf(bone));
        TuneCfg f;
        KT_CHECK(applySet(f, d.o));
        KT_CHECK(sameLevel(f.cat[CatHeavy], t));
    }
}

static void testScopeBodyKeysAndRoundTrip() {
    TuneCfg c = fixture();
    const BodyTune* juhani = findBody(c, "Juhani");
    KT_CHECK(juhani != nullptr);
    if (!juhani) return;
    ScopeArrays a;
    KT_CHECK(scopeKeys(*juhani, a.o));
    KT_CHECK(a.o.nSet == 1 && a.o.nUnset == 0);
    KT_CHECK(strcmp(setVal(a.o, "place_off_body_Juhani"), "0.050,0.060,0.070") == 0);
    TuneCfg fresh;
    KT_CHECK(applySet(fresh, a.o));
    const BodyTune* back = findBody(fresh, "Juhani");
    KT_CHECK(back && closeV3(back->off, 0.05f, 0.06f, 0.07f));
}

static void testScopeValueFormat() {
    // Values that round to zero print 0, never -0; the rest round half-away-from-zero at the printed digit.
    BodyTune b;
    tuneCopyName(b.body, "Rnd");
    b.off[0] = -0.0001f; b.off[1] = 0.0004f; b.off[2] = -0.0006f;
    ScopeArrays a;
    KT_CHECK(scopeKeys(b, a.o));
    KT_CHECK(strcmp(setVal(a.o, "place_off_body_Rnd"), "0.000,0.000,-0.001") == 0);
    CatTune t = level(true, kZero3, false, kZero3, 0);
    t.rot[0] = 0.04f; t.rot[1] = -0.04f; t.rot[2] = 0.06f;
    ScopeArrays d;
    KT_CHECK(scopeKeys(CatRifle, false, t, d.o));
    KT_CHECK(strcmp(setVal(d.o, "place_rot_rifle"), "0.0,0.0,0.1") == 0);
}

static void testScopeRangeBoundary() {
    ScopeArrays a;
    CatTune t = level(true, kZero3, true, kZero3, 0);
    t.rot[0] = 720.f; t.rot[1] = -720.f; t.rot[2] = 0.f;
    t.off[0] = 3.f; t.off[1] = -3.f; t.off[2] = 0.f;
    KT_CHECK(scopeKeys(CatPistol, false, t, a.o));          // inclusive bounds
    KT_CHECK(strcmp(setVal(a.o, "place_rot_pistol"), "720.0,-720.0,0.0") == 0);
    KT_CHECK(strcmp(setVal(a.o, "place_off_pistol"), "3.000,-3.000,0.000") == 0);
    TuneCfg fresh;
    KT_CHECK(applySet(fresh, a.o));
    KT_CHECK(sameLevel(fresh.cat[CatPistol], t));

    t.rot[0] = 720.5f;
    KT_CHECK(!scopeKeys(CatPistol, false, t, a.o));
    KT_CHECK(a.o.nSet == 0 && a.o.nUnset == 0);
    t.rot[0] = 720.f;
    t.off[2] = -3.001f;
    KT_CHECK(!scopeKeys(CatPistol, false, t, a.o));
    KT_CHECK(a.o.nSet == 0 && a.o.nUnset == 0);

    BodyTune b;
    tuneCopyName(b.body, "Edge");
    b.off[0] = 3.001f; b.off[1] = 0.f; b.off[2] = 0.f;
    KT_CHECK(!scopeKeys(b, a.o));
    KT_CHECK(a.o.nSet == 0);
    b.off[0] = std::nanf("");
    KT_CHECK(!scopeKeys(b, a.o));
    KT_CHECK(a.o.nSet == 0);
}

static void testScopeNameBoundary() {
    ScopeArrays a;
    // Model names: 16 chars is the longest, "w_" + 14.
    ModelTune m;
    tuneCopyName(m.model, "w_0123456789abcd");
    m.t = level(false, kZero3, false, kZero3, 2);
    KT_CHECK(strlen(m.model) == 16);
    KT_CHECK(scopeKeys(m, a.o));
    KT_CHECK(strcmp(setVal(a.o, "place_bone_w_0123456789abcd"), "torso_g") == 0);   // bone 2
    // Not "w_" prefixed, empty, or bad characters: rejected. (A 17-char name cannot be stored: model is char[17].)
    ModelTune bad;
    bad.t = m.t;
    tuneCopyName(bad.model, "Blaster");
    KT_CHECK(!scopeKeys(bad, a.o));
    tuneCopyName(bad.model, "w_bad-name");
    KT_CHECK(!scopeKeys(bad, a.o));
    tuneCopyName(bad.model, "");
    KT_CHECK(!scopeKeys(bad, a.o));
    KT_CHECK(a.o.nSet == 0 && a.o.nUnset == 0);

    // Body names: 16 chars ok, a name with a space rejected.
    BodyTune b;
    tuneCopyName(b.body, "Juhani_012345678");
    KT_CHECK(strlen(b.body) == 16);
    KT_CHECK(scopeKeys(b, a.o));
    KT_CHECK(setVal(a.o, "place_off_body_Juhani_012345678") != nullptr);
    tuneCopyName(b.body, "Juhani x");
    KT_CHECK(!scopeKeys(b, a.o));

    // Categories: CatUnknown and CatCount are not scopes.
    CatTune t = level(true, kZero3, false, kZero3, 0);
    KT_CHECK(!scopeKeys(CatUnknown, false, t, a.o));
    KT_CHECK(!scopeKeys(CatCount, true, t, a.o));
}

static void testScopeCountAndBufferCaps() {
    // Model with all three fields set needs 3 set slots.
    const float rot[3] = {1.f, 2.f, 3.f}, off[3] = {0.1f, 0.2f, 0.3f};
    ModelTune m;
    tuneCopyName(m.model, "w_Cap");
    m.t = level(true, rot, true, off, 1);
    ScopeArrays a;
    a.o.setCap = 2;
    KT_CHECK(!scopeKeys(m, a.o));
    KT_CHECK(a.o.nSet == 0 && a.o.nUnset == 0);
    a.o.setCap = 3;
    KT_CHECK(scopeKeys(m, a.o) && a.o.nSet == 3);

    // Model with nothing set needs 3 unset slots.
    ModelTune empty;
    tuneCopyName(empty.model, "w_Cap");
    ScopeArrays u;
    u.o.unsetCap = 2;
    KT_CHECK(!scopeKeys(empty, u.o));
    KT_CHECK(u.o.nUnset == 0);
    u.o.unsetCap = 3;
    KT_CHECK(scopeKeys(empty, u.o) && u.o.nUnset == 3 && u.o.nSet == 0);

    // Body needs one set slot; a null array is a clean false.
    BodyTune b;
    tuneCopyName(b.body, "Juhani");
    ScopeArrays z;
    z.o.setCap = 0;
    KT_CHECK(!scopeKeys(b, z.o));
    z.o.set = nullptr;
    z.o.setCap = 1;
    KT_CHECK(!scopeKeys(b, z.o));

    // Buffer: the worst case (16-char left weapon, every field at its cap) fits kScopeTextMax; a smaller buffer fails cleanly
    // and bytes past bufCap are never written.
    ModelTune worst;
    tuneCopyName(worst.model, "w_0123456789abcd");
    worst.left = true;
    const float lim[3] = {-720.f, -720.f, -720.f}, offLim[3] = {-3.f, -3.f, -3.f};
    worst.t = level(true, lim, true, offLim, 5);
    ScopeArrays w;
    KT_CHECK(scopeKeys(worst, w.o));
    KT_CHECK(w.o.bufUsed <= kScopeTextMax);
    KT_CHECK(strcmp(setVal(w.o, "place_off_l_w_0123456789abcd"), "-3.000,-3.000,-3.000") == 0);
    KT_CHECK(setVal(w.o, "place_bone_l_w_0123456789abcd") == boneNameOf(5));

    char big[200];
    memset(big, 'Z', sizeof big);
    KV set[3];
    const char* unset[3];
    ScopeOut tight = {set, 3, 0, unset, 3, 0, big, 40, 0};
    KT_CHECK(!scopeKeys(worst, tight));
    KT_CHECK(tight.nSet == 0 && tight.bufUsed <= 40);
    for (size_t i = 40; i < sizeof big; ++i)
        if (big[i] != 'Z') { KT_CHECK(false); break; }
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
    testBoneTable();
    testModelEntryParsing();
    testKeyShapes();
    testModelTableFull();
    testBodyTableFull();
    testRejectedValueClaimsNoSlot();
    testResolveRightMatrix();
    testResolveLeftMatrix();
    testResolveLeftOverrides();
    testMirrorInvolutionAndBones();
    testMirrorIsQuaternionReflection();
    testClearDropsNewTables();
    testLeftExtraKeys();
    testLeftExtraRifleVectors();
    testLeftExtraHeavyMeleeAndCategory();
    testLeftExtraNeverOnExplicit();
    testLeftExtraBodyDeltaUnaffected();
    testSizeBudget();
    testBoneNameForwards();
    testScopeModelKeysAndRoundTrip();
    testScopeTypeKeysAndRoundTrip();
    testScopeBodyKeysAndRoundTrip();
    testScopeValueFormat();
    testScopeRangeBoundary();
    testScopeNameBoundary();
    testScopeCountAndBufferCaps();
    return kt::Report("holster_tune");
}
