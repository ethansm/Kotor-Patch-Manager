// Host tests for HolsterDrawGate.h (draw-gate decision of the holstered-weapons mod, HW-UX3 Wave C1; doc 42).
// Build/run: python3 scripts/run_tests.py holster_drawgate
#include "HolsterDrawGate.h"
#include "check.h"
#include <stdint.h>
#include <math.h>

using namespace hs;

static void testTruthTable() {
    // Repoint mode: always write (old behaviour), whatever the owner / combat / window say.
    for (int r = 0; r < 2; ++r)
        for (uint32_t c = 0; c < 2; ++c)
            for (int w = 0; w < 2; ++w) KT_CHECK(shouldWriteHip(false, r != 0, c, w != 0));
    // Drawgate: hip only with owner readable AND combat 0 AND window closed.
    for (int r = 0; r < 2; ++r)
        for (uint32_t c = 0; c < 2; ++c)
            for (int w = 0; w < 2; ++w) {
                const bool want = r == 1 && c == 0 && w == 0;
                KT_CHECK(shouldWriteHip(true, r != 0, c, w != 0) == want);
            }
    // Spot checks by name.
    KT_CHECK(shouldWriteHip(true, true, 0, false));     // out of combat, no flourish: hip
    KT_CHECK(!shouldWriteHip(true, true, 1, false));    // in combat: drawn
    KT_CHECK(!shouldWriteHip(true, true, 0, true));     // flourish window open: drawn
    KT_CHECK(!shouldWriteHip(true, true, 1, true));
    KT_CHECK(!shouldWriteHip(true, false, 0, false));   // owner unreadable: hand (never stuck on the hip)
    KT_CHECK(!shouldWriteHip(true, false, 1, true));
    // Any non-zero combat flag counts as in-combat, not only 1.
    KT_CHECK(!shouldWriteHip(true, true, 2, false));
    KT_CHECK(!shouldWriteHip(true, true, 0xFFFFFFFFu, false));
}

static void testPlaceMode() {
    PlaceMode m = PlaceRepoint;
    KT_CHECK(placeModeParse("drawgate", m) && m == PlaceDrawgate);
    KT_CHECK(placeModeParse("REPOINT", m) && m == PlaceRepoint);
    KT_CHECK(placeModeParse("DrawGate", m) && m == PlaceDrawgate);
    m = PlaceDrawgate;
    KT_CHECK(!placeModeParse("", m) && m == PlaceDrawgate);          // junk keeps the previous value
    KT_CHECK(!placeModeParse("draw", m) && m == PlaceDrawgate);
    KT_CHECK(!placeModeParse("drawgates", m) && m == PlaceDrawgate);
    KT_CHECK(!placeModeParse(nullptr, m) && m == PlaceDrawgate);
    KT_CHECK(ieq(placeModeName(PlaceDrawgate), "drawgate") && ieq(placeModeName(PlaceRepoint), "repoint"));
}

static void testNodeAccepted() {
    // Repoint: only the configured bone_name.
    KT_CHECK(nodeAccepted(false, "pelvis_g", "pelvis_g", true));
    KT_CHECK(nodeAccepted(false, "PELVIS_G", "pelvis_g", true));
    KT_CHECK(!nodeAccepted(false, "rhand", "pelvis_g", true));
    KT_CHECK(!nodeAccepted(false, "lhand", "pelvis_g", true));
    // Drawgate: rhand always, lhand only when enabled; the hip bone name is NOT a target.
    KT_CHECK(nodeAccepted(true, "rhand", "pelvis_g", false));
    KT_CHECK(nodeAccepted(true, "RHand", "pelvis_g", true));
    KT_CHECK(nodeAccepted(true, "lhand", "pelvis_g", true));
    KT_CHECK(!nodeAccepted(true, "lhand", "pelvis_g", false));
    KT_CHECK(!nodeAccepted(true, "pelvis_g", "pelvis_g", true));
    KT_CHECK(!nodeAccepted(true, "rhandx", "pelvis_g", true));
    KT_CHECK(!nodeAccepted(true, "", "pelvis_g", true));
    KT_CHECK(!nodeAccepted(true, nullptr, "pelvis_g", true));
    KT_CHECK(!nodeAccepted(false, nullptr, "pelvis_g", true));
}

static void testGateKeyInvalidation() {
    GateState s = gateInit();
    KT_CHECK(gateUpdate(s, 0xAB11A68, true));     // first evaluation is a change (resolve + log)
    KT_CHECK(!gateUpdate(s, 0xAB11A68, true));    // steady state: no change
    KT_CHECK(!gateUpdate(s, 0xAB11A68, true));
    KT_CHECK(gateUpdate(s, 0xAB11A68, false));    // hip -> drawn flips
    KT_CHECK(!gateUpdate(s, 0xAB11A68, false));
    KT_CHECK(gateUpdate(s, 0xAB11A68, true));     // drawn -> hip flips back
    KT_CHECK(gateUpdate(s, 0xAD5F938, true));     // same state, new owner (gob reused by another creature): invalidates
    KT_CHECK(!gateUpdate(s, 0xAD5F938, true));
    KT_CHECK(gateUpdate(s, 0, false));            // owner lost (unreadable) and drawn: changes both
    KT_CHECK(!gateUpdate(s, 0, false));
    KT_CHECK(gateUpdate(s, 0xAD5F938, false));    // owner back, still drawn: owner change alone invalidates
    // A fresh state starts "not evaluated": a drawn first evaluation is also a change.
    GateState t = gateInit();
    KT_CHECK(gateUpdate(t, 0x1000, false));
    KT_CHECK(!gateUpdate(t, 0x1000, false));
}

static void testGateDrivenByDecision() {
    // End-to-end of the pure part: feed shouldWriteHip into gateUpdate over a combat enter/exit and a flourish window.
    GateState s = gateInit();
    const uintptr_t owner = 0xB924540;
    int flips = 0;
    struct Step { uint32_t combat; bool window; bool expectHip; };
    const Step steps[] = {{0, false, true}, {0, false, true}, {1, false, false}, {1, false, false}, {0, false, true},
                          {0, true, false}, {0, true, false}, {0, false, true}};
    for (const Step& st : steps) {
        const bool hip = shouldWriteHip(true, true, st.combat, st.window);
        KT_CHECK(hip == st.expectHip);
        if (gateUpdate(s, owner, hip)) ++flips;
    }
    KT_CHECK(flips == 1 + 1 + 1 + 1 + 1);   // first eval, enter, exit, window open, window close
}

static void testPoseBaseEqual() {
    // mountPose(pelvis_g) base must equal the old pelvis attach-node base: same pose in either route, q and -q the same rotation.
    const float h = 0.5f;
    Pose a{{1.0f, 2.0f, 0.9f}, Q{h, h, h, h}};
    Pose b = a;
    KT_CHECK(poseBaseEqual(a, b, 1e-4f, 0.1f));
    b.q = Q{-h, -h, -h, -h};                       // same rotation, opposite sign
    KT_CHECK(poseBaseEqual(a, b, 1e-4f, 0.1f));
    b = a; b.pos[2] += 0.01f;                       // 1 cm off
    KT_CHECK(!poseBaseEqual(a, b, 1e-3f, 0.1f));
    KT_CHECK(poseBaseEqual(a, b, 2e-2f, 0.1f));
    b = a; b.q = qnormalize(Q{h, h, h, h * 1.02f}); // ~1.6 deg
    KT_CHECK(!poseBaseEqual(a, b, 1e-4f, 0.5f));
    KT_CHECK(poseBaseEqual(a, b, 1e-4f, 3.0f));
    // The hand pose is NOT the pelvis pose: a base mixed up with the hand node would fail the check.
    Pose hand{{1.3f, 2.1f, 1.0f}, Q{1, 0, 0, 0}};
    KT_CHECK(!poseBaseEqual(a, hand, 1e-2f, 1.0f));
    // NaN never compares equal.
    b = a; b.pos[0] = NAN;
    KT_CHECK(!poseBaseEqual(a, b, 1.0f, 180.0f));
    b = a; b.q = Q{NAN, 0, 0, 0};
    KT_CHECK(!poseBaseEqual(a, b, 1.0f, 180.0f));
}

static void testIsWeaponModel() {
    KT_CHECK(isWeaponModel("w_Lngswrd_001"));
    KT_CHECK(isWeaponModel("W_SonicPstl_002"));
    KT_CHECK(isWeaponModel("w_"));
    KT_CHECK(!isWeaponModel(nullptr));
    KT_CHECK(!isWeaponModel(""));
    KT_CHECK(!isWeaponModel("w"));
    KT_CHECK(!isWeaponModel("wx_blade"));
    KT_CHECK(!isWeaponModel("p_bastilabb"));
    KT_CHECK(!isWeaponModel("i_menu_preview"));
    KT_CHECK(!isWeaponModel(" w_blade"));
}

static void testHandFromNode() {
    // Drawgate: the node name wins over the ring and the previous flag (all 3 ring values x prev).
    for (int ring = HandNone; ring <= HandL; ++ring)
        for (int prev = 0; prev < 2; ++prev) {
            KT_CHECK(handFromNode(true, "lhand", ring, prev != 0));
            KT_CHECK(handFromNode(true, "LHand", ring, prev != 0));
            KT_CHECK(!handFromNode(true, "rhand", ring, prev != 0));
            KT_CHECK(!handFromNode(true, "RHAND", ring, prev != 0));
            KT_CHECK(!handFromNode(true, "headhook", ring, prev != 0));
            KT_CHECK(!handFromNode(true, "", ring, prev != 0));
            KT_CHECK(!handFromNode(true, nullptr, ring, prev != 0));
        }
    // Ring evicted + never recorded (HandNone) in drawgate: still the node decides (B2 scenarios a and b).
    KT_CHECK(handFromNode(true, "lhand", HandNone, false));
    KT_CHECK(!handFromNode(true, "rhand", HandNone, true));
    // Repoint: exactly resolveLeft (known hand wins, unknown keeps prev); the node name is ignored.
    for (int ring = HandNone; ring <= HandL; ++ring)
        for (int prev = 0; prev < 2; ++prev) {
            KT_CHECK(handFromNode(false, "pelvis_g", ring, prev != 0) == resolveLeft(prev != 0, ring));
            KT_CHECK(handFromNode(false, "lhand", ring, prev != 0) == resolveLeft(prev != 0, ring));
            KT_CHECK(handFromNode(false, nullptr, ring, prev != 0) == resolveLeft(prev != 0, ring));
        }
    KT_CHECK(handFromNode(false, "pelvis_g", HandL, false));
    KT_CHECK(!handFromNode(false, "pelvis_g", HandR, true));
    KT_CHECK(handFromNode(false, "pelvis_g", HandNone, true));    // sticky
    KT_CHECK(!handFromNode(false, "pelvis_g", HandNone, false));
}

int main() {
    testTruthTable();
    testPlaceMode();
    testNodeAccepted();
    testGateKeyInvalidation();
    testGateDrivenByDecision();
    testPoseBaseEqual();
    testIsWeaponModel();
    testHandFromNode();
    return kt::Report("holster_drawgate");
}
