// T1 host tests for HolsterTune.h (ini tuning-key parser for the holstered-weapons mod; doc 42).
// Build/run: python3 scripts/run_tests.py holster_tune
#include "HolsterTune.h"
#include "HolsterQuat.h"
#include "check.h"
#include <cmath>
#include <stdio.h>
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

static bool closeF(float a, float b) { return std::fabs(a - b) <= 1e-5f; }
static bool closeV3(const float* v, float a, float b, float c) { return closeF(v[0], a) && closeF(v[1], b) && closeF(v[2], c); }

// Global rot (10,20,30), global off (0.1,0.2,0.3); pistol type rot (1,2,3) off (0.4,0.5,0.6) bone pelvis_g;
// left pistol rot (11,12,13); model w_Blaster rot (7,8,9) bone rthigh_g; left model w_Blaster off (0.9,0.8,0.7);
// body Juhani off (0.05,0.06,0.07).
static TuneCfg fixture() {
    TuneCfg c;
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
    KT_CHECK(!c.catL[CatPistol].hasRot && !c.cat[CatPistol].hasRot && !c.mdl[0].t.hasRot && !c.body[0].off[0]);
    KT_CHECK(closeV3(c.rot, 10, 20, 30) && closeV3(c.off, 0.1f, 0.2f, 0.3f));   // globals are not cleared
    Resolved r;
    resolveFull(c, "w_Blaster", CatPistol, "Juhani", true, false, r);
    KT_CHECK(closeV3(r.rot, 10, -20, -30) && r.src[0] == 0);
    KT_CHECK(closeV3(r.off, -0.1f, 0.2f, 0.3f) && r.src[1] == 0);
    KT_CHECK(r.boneIdx == 0 && r.src[2] == 0);
}

static void testSizeBudget() {
    static_assert(sizeof(TuneCfg) <= 4096, "TuneCfg budget");
    KT_CHECK(sizeof(TuneCfg) <= 4096);
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
    testSizeBudget();
    return kt::Report("holster_tune");
}
