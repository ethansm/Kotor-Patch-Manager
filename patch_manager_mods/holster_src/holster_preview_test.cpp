// T-preview host tests for HolsterPreview.h (preview registry and edit overlay of the holstered-weapons editor; doc 42, HE2).
// Build/run: python3 scripts/run_tests.py holster_preview
#include "HolsterPreview.h"
#include "check.h"
#include <cmath>
#include <stdint.h>
#include <string.h>

using namespace hs;

static bool closeTo(float a, float b, float tol = 1e-4f) { return std::fabs(a - b) <= tol; }
static bool closeAng(float a, float b, float tol = 1e-3f) { return std::fabs(wrap180(a - b)) <= tol; }
static bool closeV3(const float* v, float a, float b, float c) {
    return closeTo(v[0], a) && closeTo(v[1], b) && closeTo(v[2], c);
}
static float absDot(const Q& a, const Q& b) {
    return std::fabs(a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z);
}

// Config with a global placement and one body delta ("BodyA", right-hand frame).
static TuneCfg baseCfg() {
    TuneCfg c;
    c.leftExtra = 0.f;   // plain mirror; the left_extra_rot save contract has its own tests (extraCfg)
    c.place = true;
    c.bootBoneIdx = 1;
    c.rot[0] = 10.f; c.rot[1] = 20.f; c.rot[2] = 30.f;
    c.off[0] = 0.1f; c.off[1] = 0.2f; c.off[2] = 0.3f;
    BodyTune& b = c.body[0];
    tuneCopyName(b.body, "BodyA");
    b.off[0] = 0.05f; b.off[1] = 0.1f; b.off[2] = -0.02f;
    c.nBody = 1;
    return c;
}

static const char* kModel = "w_Blaster_01";
static const char* kBody = "BodyA";

static int gGob[12];
static int gP0, gP1, gN0, gN1, gN2, gN3;

// ---- Registry ------------------------------------------------------------------------------------------------------------

static void testRegistryRegisterUnregister() {
    PreviewRegistry r;
    KT_CHECK(!previewRegister(r, nullptr, &gP0, &gN0, false));
    for (int i = 0; i < 8; ++i) KT_CHECK(previewRegister(r, &gGob[i], &gP0, &gN0, i & 1));
    KT_CHECK_EQ(r.n, 8);
    KT_CHECK(!previewRegister(r, &gGob[8], &gP0, &gN0, false));   // full
    KT_CHECK(!previewRegister(r, &gGob[3], &gP0, &gN0, false));   // duplicate (also full)
    KT_CHECK(previewUnregister(r, &gGob[3]));
    KT_CHECK_EQ(r.n, 7);
    KT_CHECK_EQ(previewFind(r, &gGob[4]), 3);                      // shifted down
    KT_CHECK_EQ(previewFind(r, &gGob[3]), -1);
    KT_CHECK(!previewUnregister(r, &gGob[3]));
    KT_CHECK(previewRegister(r, &gGob[8], &gP1, &gN2, true));      // freed slot
    KT_CHECK_EQ(r.n, 8);
    KT_CHECK(r.e[7].gob == &gGob[8]);
    KT_CHECK(r.e[7].parent == &gP1 && r.e[7].node == &gN2 && r.e[7].left);
    KT_CHECK(!previewRegister(r, &gGob[9], &gP1, &gN2, false));    // still full
}

static void testRegistrySweep() {
    PreviewRegistry r;
    KT_CHECK(previewRegister(r, &gGob[0], &gP0, &gN0, false));
    KT_CHECK(previewRegister(r, &gGob[1], &gP0, &gN1, true));
    KT_CHECK(previewRegister(r, &gGob[2], &gP1, &gN2, false));
    KT_CHECK(previewRegister(r, &gGob[3], &gP1, &gN2, false));

    // No change: nothing dropped.
    const int none = previewSweep(r, [](void* gob, void*& parent, void*& node) {
        if (gob == &gGob[0]) { parent = &gP0; node = &gN0; }
        else if (gob == &gGob[1]) { parent = &gP0; node = &gN1; }
        else { parent = &gP1; node = &gN2; }
        return true;
    });
    KT_CHECK_EQ(none, 0);
    KT_CHECK_EQ(r.n, 4);

    // gob 0: node changed. gob 1: unchanged. gob 2: gone. gob 3: parent changed (registered under gP1).
    const int dropped = previewSweep(r, [](void* gob, void*& parent, void*& node) {
        if (gob == &gGob[2]) return false;
        parent = &gP0;
        if (gob == &gGob[0]) node = &gN3;
        else if (gob == &gGob[1]) node = &gN1;
        else node = &gN2;
        return true;
    });
    KT_CHECK_EQ(dropped, 3);
    KT_CHECK_EQ(r.n, 1);
    KT_CHECK(r.e[0].gob == &gGob[1]);
    KT_CHECK(r.e[0].left);
}

// ---- Overlay -------------------------------------------------------------------------------------------------------------

static void testBeginSeedsFromResolve() {
    const TuneCfg c = baseCfg();
    Resolved full;
    resolveFull(c, kModel, CatPistol, kBody, false, false, full);
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, false);
    KT_CHECK(o.active);
    KT_CHECK(!o.dirty);
    KT_CHECK(o.scope == ScopeWeapon);
    KT_CHECK(closeV3(o.cur.rot, 10, 20, 30));
    KT_CHECK(closeV3(o.cur.off, 0.15f, 0.3f, 0.28f));              // level + body delta
    KT_CHECK(closeV3(o.baseOff, 0.1f, 0.2f, 0.3f));                // level only
    KT_CHECK(closeV3(o.cur.off, full.off[0], full.off[1], full.off[2]));
    KT_CHECK(o.cur.src[0] == full.src[0] && o.cur.src[1] == full.src[1] && o.cur.src[2] == full.src[2]);
    KT_CHECK(o.cur.src[1] & 4);                                    // body delta was added
}

static void testBeginLeftMirrorsGlobal() {
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, true);
    // Left resolve mirrors the global level (rot y, z and off x negated) and the body delta (off x negated).
    KT_CHECK(closeV3(o.cur.rot, 10, -20, -30));
    KT_CHECK(closeV3(o.baseOff, -0.1f, 0.2f, 0.3f));
    KT_CHECK(closeV3(o.cur.off, -0.15f, 0.3f, 0.28f));
    KT_CHECK(o.left);
}

static void testNudgeLocalAndRevert() {
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, false);
    const float seedRot[3] = {o.cur.rot[0], o.cur.rot[1], o.cur.rot[2]};
    const float seedOff[3] = {o.cur.off[0], o.cur.off[1], o.cur.off[2]};
    KT_CHECK(!overlayNudgeLocal(o, 3, 10.f));                      // invalid axis
    KT_CHECK(!o.dirty);
    KT_CHECK(overlayNudgeLocal(o, 2, 30.f));
    KT_CHECK(o.dirty);
    const float zAxis[3] = {0.f, 0.f, 1.f};
    const Q want = qmul(qaxis(zAxis, 30.f), qbase(seedRot[0], seedRot[1], seedRot[2]));
    KT_CHECK(absDot(qbase(o.cur.rot[0], o.cur.rot[1], o.cur.rot[2]), want) > 1.f - 1e-4f);
    overlayRevert(o);
    KT_CHECK(!o.dirty);
    KT_CHECK(closeV3(o.cur.rot, seedRot[0], seedRot[1], seedRot[2]));
    KT_CHECK(closeV3(o.cur.off, seedOff[0], seedOff[1], seedOff[2]));
}

static void testNudgeWorld() {
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, false);
    const float seedRot[3] = {o.cur.rot[0], o.cur.rot[1], o.cur.rot[2]};
    const Q id = {1.f, 0.f, 0.f, 0.f};
    const float zAxis[3] = {0.f, 0.f, 1.f};
    // Identity bone: a world rotation about z is a left multiplication of the placement.
    const Q dq = qaxis(zAxis, 45.f);
    KT_CHECK(overlayNudgeWorldRot(o, id, dq));
    const Q want = qmul(dq, qbase(seedRot[0], seedRot[1], seedRot[2]));
    KT_CHECK(absDot(qbase(o.cur.rot[0], o.cur.rot[1], o.cur.rot[2]), want) > 1.f - 1e-4f);
    // Identity bone: a world move adds the drag to the offset. The body part (0.05 on x) is kept.
    overlayRevert(o);
    const float dp[3] = {0.5f, 0.f, 0.f};
    KT_CHECK(overlayNudgeWorldMove(o, id, dp));
    KT_CHECK(o.dirty);
    KT_CHECK(closeTo(o.cur.off[0], 0.65f));
    KT_CHECK(closeTo(o.cur.off[0] - o.baseOff[0], 0.05f));
}

static void testSetValuesCaps() {
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, nullptr, false);       // no body: body part is zero
    const float rot[3] = {800.f, -900.f, 45.f};
    const float off[3] = {5.f, -4.f, 0.2f};
    KT_CHECK(overlaySetValues(o, rot, off));
    KT_CHECK(o.dirty);
    KT_CHECK(closeV3(o.cur.rot, 720.f, -720.f, 45.f));
    KT_CHECK(closeV3(o.cur.off, 3.f, -3.f, 0.2f));
    const float nanRot[3] = {std::nanf(""), 0.f, 0.f};
    KT_CHECK(overlaySetValues(o, nanRot, off));
    KT_CHECK(closeTo(o.cur.rot[0], 0.f));
}

static void testCapWithBodyPart() {
    // The stored level is capped at 3 m; the shown absolute is level + body part.
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, false);
    const float rot[3] = {10.f, 20.f, 30.f};
    const float off[3] = {3.5f, 0.2f, 0.3f};
    KT_CHECK(overlaySetValues(o, rot, off));
    KT_CHECK(closeTo(o.baseOff[0], 3.f));                          // 3.5 - 0.05 body part, capped
    KT_CHECK(closeTo(o.cur.off[0], 3.05f));
    TuneCfg w = c;
    KT_CHECK(overlayWrite(o, w));
    Resolved r;
    resolveFull(w, kModel, CatPistol, kBody, false, false, r);
    KT_CHECK(closeV3(r.off, 3.05f, o.cur.off[1], o.cur.off[2]));
}

static void testSetScopeAndBodyLocks() {
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, false);
    KT_CHECK(!overlaySetScope(o, 3));
    KT_CHECK(o.scope == ScopeWeapon);
    KT_CHECK(overlaySetScope(o, ScopeBody));
    KT_CHECK(!o.dirty);                                            // a scope switch is not an edit
    KT_CHECK(o.scope == ScopeBody);
    const Q id = {1.f, 0.f, 0.f, 0.f};
    const float zAxis[3] = {0.f, 0.f, 1.f};
    KT_CHECK(!overlayNudgeLocal(o, 0, 10.f));
    KT_CHECK(!overlayNudgeWorldRot(o, id, qaxis(zAxis, 10.f)));
    KT_CHECK(!overlayCycleBone(o, 1));
    const float dp[3] = {0.1f, 0.f, 0.f};
    KT_CHECK(overlayNudgeWorldMove(o, id, dp));                   // off-only edits are allowed
    KT_CHECK(closeTo(o.cur.off[0] - o.baseOff[0], 0.15f));        // body part 0.05 + 0.1
    const float rot[3] = {90.f, 90.f, 90.f};
    const float off[3] = {o.cur.off[0], o.cur.off[1], o.cur.off[2]};
    KT_CHECK(overlaySetValues(o, rot, off));
    KT_CHECK(closeV3(o.cur.rot, 10, 20, 30));                     // rot ignored under body scope
}

static void testBodyScopeWriteRoundTrip() {
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, false);
    KT_CHECK(overlaySetScope(o, ScopeBody));
    const float rot[3] = {0.f, 0.f, 0.f};
    const float off[3] = {o.cur.off[0] + 0.2f, o.cur.off[1], o.cur.off[2]};
    KT_CHECK(overlaySetValues(o, rot, off));
    TuneCfg w = c;
    KT_CHECK(overlayWrite(o, w));
    KT_CHECK_EQ(w.nBody, 1);
    KT_CHECK(closeV3(w.body[0].off, 0.25f, 0.1f, -0.02f));        // absolute 0.35 - base 0.1
    KT_CHECK(findModel(w, kModel, false) == nullptr);             // body scope writes no model entry
    Resolved r;
    resolveFull(w, kModel, CatPistol, kBody, false, false, r);
    KT_CHECK(closeV3(r.off, 0.35f, 0.3f, 0.28f));
    KT_CHECK(closeV3(r.rot, 10, 20, 30));
}

static void testLeftBodyMirrorRoundTrip() {
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, true);
    KT_CHECK(overlaySetScope(o, ScopeBody));
    const float rot[3] = {0.f, 0.f, 0.f};
    const float off[3] = {o.cur.off[0] + 0.2f, o.cur.off[1], o.cur.off[2]};   // shown x: -0.15 -> 0.05
    KT_CHECK(overlaySetValues(o, rot, off));
    TuneCfg w = c;
    KT_CHECK(overlayWrite(o, w));
    // Stored body x is negated on the left: shown body part 0.15 is stored as -0.15.
    KT_CHECK(closeTo(w.body[0].off[0], -0.15f));
    KT_CHECK(closeTo(w.body[0].off[1], 0.1f));
    Resolved r;
    resolveFull(w, kModel, CatPistol, kBody, true, false, r);
    KT_CHECK(closeV3(r.off, o.cur.off[0], o.cur.off[1], o.cur.off[2]));
}

static void testWeaponScopeWriteLeft() {
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, true);
    const float rot[3] = {5.f, 15.f, 25.f};
    const float off[3] = {0.4f, 0.4f, 0.4f};
    KT_CHECK(overlaySetValues(o, rot, off));
    TuneCfg w = c;
    KT_CHECK(overlayWrite(o, w));
    const ModelTune* m = findModel(w, kModel, true);
    KT_CHECK(m != nullptr);
    if (m) {
        KT_CHECK(m->t.hasRot && m->t.hasOff);
        KT_CHECK(closeV3(m->t.rot, 5, 15, 25));                   // left frame, not mirrored
        KT_CHECK(closeV3(m->t.off, 0.45f, 0.3f, 0.42f));          // shown 0.4 - left body part (-0.05, 0.1, -0.02)
    }
    KT_CHECK(findModel(w, kModel, false) == nullptr);
    Resolved r;
    resolveFull(w, kModel, CatPistol, kBody, true, false, r);
    KT_CHECK(closeV3(r.rot, 5, 15, 25));
    KT_CHECK(closeV3(r.off, 0.4f, 0.4f, 0.4f));
    KT_CHECK_EQ(r.src[0], 2);
    KT_CHECK_EQ(r.src[1], 6);                                      // model level plus body delta
}

static void testTypeScopeWrite() {
    const TuneCfg c = baseCfg();
    const float rot[3] = {1.f, 2.f, 3.f};
    const float off[3] = {0.5f, 0.5f, 0.5f};

    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, nullptr, false);
    KT_CHECK(overlaySetScope(o, ScopeType));
    KT_CHECK(overlaySetValues(o, rot, off));
    TuneCfg w = c;
    KT_CHECK(overlayWrite(o, w));
    KT_CHECK(w.cat[CatPistol].hasRot && w.cat[CatPistol].hasOff);
    KT_CHECK(closeV3(w.cat[CatPistol].rot, 1, 2, 3));
    KT_CHECK(closeV3(w.cat[CatPistol].off, 0.5f, 0.5f, 0.5f));
    KT_CHECK(!w.catL[CatPistol].hasRot);
    Resolved r;
    resolveFull(w, "w_other", CatPistol, nullptr, false, false, r);
    KT_CHECK(closeV3(r.rot, 1, 2, 3));
    KT_CHECK_EQ(r.src[0], 1);

    EditOverlay ol;
    overlayBegin(ol, c, kModel, CatPistol, nullptr, true);
    KT_CHECK(overlaySetScope(ol, ScopeType));
    KT_CHECK(overlaySetValues(ol, rot, off));
    TuneCfg wl = c;
    KT_CHECK(overlayWrite(ol, wl));
    KT_CHECK(wl.catL[CatPistol].hasRot);
    KT_CHECK(closeV3(wl.catL[CatPistol].rot, 1, 2, 3));
    KT_CHECK(!wl.cat[CatPistol].hasRot);

    EditOverlay oc;
    overlayBegin(oc, c, kModel, CatUnknown, nullptr, false);
    KT_CHECK(overlaySetScope(oc, ScopeType));
    TuneCfg wc = c;
    KT_CHECK(!overlayWrite(oc, wc));                               // no category to write to
}

static void testWriteRejectsMissingTargets() {
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, nullptr, CatPistol, nullptr, false);       // no model name
    TuneCfg w = c;
    KT_CHECK(!overlayWrite(o, w));
    KT_CHECK(findModel(w, "", false) == nullptr);
    KT_CHECK(overlaySetScope(o, ScopeBody));
    KT_CHECK(!overlayWrite(o, w));                                 // no body name
    EditOverlay none;
    KT_CHECK(!overlayWrite(none, w));
}

static void testCycleBone() {
    KT_CHECK(boneNameOf(0) == nullptr);
    KT_CHECK(boneNameOf(6) == nullptr);
    KT_CHECK(strcmp(boneNameOf(5), "lthigh_g") == 0);
    const TuneCfg c = baseCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, false);
    KT_CHECK_EQ(o.cur.boneIdx, 0);                                 // global level has no bone
    KT_CHECK(!overlayCycleBone(o, 2));                             // invalid step
    KT_CHECK(overlayCycleBone(o, 1));
    KT_CHECK_EQ(o.cur.boneIdx, 1);
    KT_CHECK(o.dirty);
    KT_CHECK(overlayCycleBone(o, -1));                             // wraps down
    KT_CHECK_EQ(o.cur.boneIdx, 5);
    KT_CHECK(overlayCycleBone(o, 1));                              // wraps up
    KT_CHECK_EQ(o.cur.boneIdx, 1);
    const int want[5] = {2, 3, 4, 5, 1};
    for (int k = 0; k < 5; ++k) {
        KT_CHECK(overlayCycleBone(o, 1));
        KT_CHECK_EQ(o.cur.boneIdx, want[k]);
        KT_CHECK(boneNameOf(o.cur.boneIdx) != nullptr);
    }
}

static void testGetEditEulerAndSrc() {
    const TuneCfg c = baseCfg();
    Resolved expect;
    resolveFull(c, kModel, CatPistol, kBody, false, false, expect);
    EditOverlay o;
    overlayBegin(o, c, kModel, CatPistol, kBody, false);
    // Near gimbal lock with rx given as the continuity reference: the euler must come back unchanged.
    const float rot[3] = {30.f, 85.f, 10.f};
    const float off[3] = {o.cur.off[0], o.cur.off[1], o.cur.off[2]};
    KT_CHECK(overlaySetValues(o, rot, off));
    EditView v;
    overlayGetEdit(o, v);
    KT_CHECK(closeAng(v.r.rot[0], 30.f, 0.05f));
    KT_CHECK(closeAng(v.r.rot[1], 85.f, 0.05f));
    KT_CHECK(closeAng(v.r.rot[2], 10.f, 0.05f));
    KT_CHECK(v.dirty);
    KT_CHECK(v.scope == ScopeWeapon);
    KT_CHECK(!v.left);
    KT_CHECK(v.r.src[0] == expect.src[0] && v.r.src[1] == expect.src[1] && v.r.src[2] == expect.src[2]);
}

static void testInactiveAndReBegin() {
    const TuneCfg c = baseCfg();
    EditOverlay o;
    const float rot[3] = {1.f, 2.f, 3.f};
    const float off[3] = {0.f, 0.f, 0.f};
    KT_CHECK(!overlaySetScope(o, 0));
    KT_CHECK(!overlayNudgeLocal(o, 0, 5.f));
    KT_CHECK(!overlaySetValues(o, rot, off));
    KT_CHECK(!overlayCycleBone(o, 1));
    KT_CHECK(!overlayNudgeWorldRot(o, Q{1.f, 0.f, 0.f, 0.f}, Q{1.f, 0.f, 0.f, 0.f}));
    TuneCfg w = c;
    KT_CHECK(!overlayWrite(o, w));
    // A second BeginEdit discards the dirty state and the scope of the first.
    overlayBegin(o, c, kModel, CatPistol, kBody, false);
    KT_CHECK(overlaySetScope(o, ScopeType));
    KT_CHECK(overlaySetValues(o, rot, off));
    KT_CHECK(o.dirty);
    overlayBegin(o, c, kModel, CatPistol, kBody, false);
    KT_CHECK(!o.dirty);
    KT_CHECK(o.scope == ScopeWeapon);
    KT_CHECK(closeV3(o.cur.rot, 10, 20, 30));
}

// ---- left_extra_rot save contract: the extra is never baked into an l_ key ----

static TuneCfg extraCfg() {
    TuneCfg c = baseCfg();
    c.leftExtra = -8.f;
    KT_CHECK(applyTuneKey(c, "place_rot_rifle", "-60,4,93"));
    KT_CHECK(applyTuneKey(c, "place_off_rifle", "0.12,-0.11,0.20"));
    KT_CHECK(applyTuneKey(c, "place_bone_rifle", "torsoUpr_g"));
    return c;
}

static void testExtraSeedShowsTiltedRot() {
    const TuneCfg c = extraCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatRifle, nullptr, true);
    KT_CHECK(o.seed.extraDeg == -8.f && closeAng(o.cur.rot[1], -11.99f, 0.05f));   // the editor shows the tilted left rot
    EditView v;
    overlayGetEdit(o, v);
    KT_CHECK(v.r.extraDeg == -8.f && closeAng(v.r.rot[1], -11.99f, 0.05f));
    EditOverlay r;
    overlayBegin(r, c, kModel, CatRifle, nullptr, false);
    KT_CHECK(r.seed.extraDeg == 0.f && closeV3(r.cur.rot, -60, 4, 93));
}

static void testExtraUneditedRotNotWritten() {
    const TuneCfg c = extraCfg();
    for (int scope = ScopeWeapon; scope <= ScopeType; ++scope) {
        EditOverlay o;
        overlayBegin(o, c, kModel, CatRifle, nullptr, true);
        KT_CHECK(overlaySetScope(o, scope));
        // only the offset changes (a value-pane write repeats the shown rot, within display rounding)
        const float rot[3] = {o.cur.rot[0] + 0.03f, o.cur.rot[1] - 0.03f, o.cur.rot[2]};
        const float off[3] = {-0.15f, -0.11f, 0.22f};
        KT_CHECK(overlaySetValues(o, rot, off));
        TuneCfg w = c;
        KT_CHECK(overlayWrite(o, w));
        const CatTune* t = scope == ScopeWeapon ? (findModel(w, kModel, true) ? &findModel(w, kModel, true)->t : nullptr) : &w.catL[CatRifle];
        KT_CHECK(t != nullptr);
        if (!t) continue;
        KT_CHECK(!t->hasRot && t->hasOff && closeV3(t->off, -0.15f, -0.11f, 0.22f));
        // the level stays inherited: the extra still applies after the save
        Resolved r;
        resolveFull(w, kModel, CatRifle, nullptr, true, false, r);
        KT_CHECK(r.extraDeg == -8.f && closeAng(r.rot[1], -11.99f, 0.05f) && closeV3(r.off, -0.15f, -0.11f, 0.22f));
        // and the keys the editor would write carry no rot key
        KV set[3];
        const char* unset[3];
        char buf[kScopeTextMax];
        ScopeOut so{set, 3, 0, unset, 3, 0, buf, sizeof buf, 0};
        KT_CHECK(scope == ScopeWeapon ? scopeKeys(*findModel(w, kModel, true), so) : scopeKeys(CatRifle, true, w.catL[CatRifle], so));
        for (int i = 0; i < so.nSet; ++i) KT_CHECK(strncmp(set[i].key, "place_rot_", 10) != 0);
        bool rotUnset = false;
        for (int i = 0; i < so.nUnset; ++i) rotUnset = rotUnset || strncmp(unset[i], "place_rot_", 10) == 0;
        KT_CHECK(rotUnset);
    }
}

static void testExtraEditedRotWrittenAsShown() {
    const TuneCfg c = extraCfg();
    EditOverlay o;
    overlayBegin(o, c, kModel, CatRifle, nullptr, true);
    KT_CHECK(overlayNudgeLocal(o, 0, 5.f));   // user tilts the weapon
    const float shown[3] = {o.cur.rot[0], o.cur.rot[1], o.cur.rot[2]};
    TuneCfg w = c;
    KT_CHECK(overlayWrite(o, w));
    const ModelTune* m = findModel(w, kModel, true);
    KT_CHECK(m && m->t.hasRot && closeV3(m->t.rot, shown[0], shown[1], shown[2]));
    // explicit now: the extra is not applied on top, so the saved weapon renders exactly as it was shown
    Resolved r;
    resolveFull(w, kModel, CatRifle, nullptr, true, false, r);
    KT_CHECK(r.extraDeg == 0.f && closeV3(r.rot, shown[0], shown[1], shown[2]));
}

static void testExtraRightHandAndBodyScopeUnchanged() {
    const TuneCfg c = extraCfg();
    // the right hand stores its rot as before
    EditOverlay o;
    overlayBegin(o, c, kModel, CatRifle, nullptr, false);
    TuneCfg w = c;
    KT_CHECK(overlayWrite(o, w));
    const ModelTune* m = findModel(w, kModel, false);
    KT_CHECK(m && m->t.hasRot && closeV3(m->t.rot, -60, 4, 93));
    // an unedited left weapon with ex = 0 stores the rot like before (no extra, nothing to keep)
    TuneCfg c0 = extraCfg();
    c0.leftExtra = 0.f;
    EditOverlay z;
    overlayBegin(z, c0, kModel, CatRifle, nullptr, true);
    TuneCfg w0 = c0;
    KT_CHECK(overlayWrite(z, w0));
    const ModelTune* m0 = findModel(w0, kModel, true);
    KT_CHECK(m0 && m0->t.hasRot && closeV3(m0->t.rot, -60, -4, -93));
}

int main() {
    testRegistryRegisterUnregister();
    testRegistrySweep();
    testBeginSeedsFromResolve();
    testBeginLeftMirrorsGlobal();
    testNudgeLocalAndRevert();
    testNudgeWorld();
    testSetValuesCaps();
    testCapWithBodyPart();
    testSetScopeAndBodyLocks();
    testBodyScopeWriteRoundTrip();
    testLeftBodyMirrorRoundTrip();
    testWeaponScopeWriteLeft();
    testTypeScopeWrite();
    testWriteRejectsMissingTargets();
    testCycleBone();
    testGetEditEulerAndSrc();
    testInactiveAndReBegin();
    testExtraSeedShowsTiltedRot();
    testExtraUneditedRotNotWritten();
    testExtraEditedRotWrittenAsShown();
    testExtraRightHandAndBodyScopeUnchanged();
    return kt::Report("holster_preview");
}
