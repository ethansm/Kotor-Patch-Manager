// T1 host tests for HolsterLogic.h (the pure logic of Holster.cpp; doc 42).
// Build/run: python3 scripts/run_tests.py holster
#include "HolsterLogic.h"
#include "check.h"
#include <string.h>

using namespace hs;

// Every row of the R6a table (research/holstered_weapons/wave2_3.md): label, itemclass, weaponwield, expected category.
struct Row { int row; const char* stem; int ww; Category cat; };
static const Row kRows[] = {
    {0, "w_Qtrstaff", 3, CatQStaff}, {1, "w_Stunbaton", 1, CatMelee}, {2, "w_Lngswrd", 2, CatMelee}, {3, "w_Vbroswrd", 2, CatMelee},
    {4, "w_Shortswrd", 2, CatMelee}, {5, "w_Vbroshort", 2, CatMelee}, {6, "w_Dblswrd", 3, CatMelee}, {7, "w_VbrDblswd", 3, CatMelee},
    {8, "w_Lghtsbr", 2, CatLightsaber}, {9, "w_Dblsbr", 3, CatLightsaber}, {10, "w_ShortSbr", 2, CatLightsaber},
    {12, "w_BlstrPstl", 4, CatPistol}, {13, "w_HvyBlstr", 4, CatPistol}, {14, "w_HldOBlst", 4, CatPistol}, {15, "w_IonBlstr", 4, CatPistol},
    {16, "w_DsrptPstl", 4, CatPistol}, {17, "w_SonicPstl", 4, CatPistol},
    {18, "w_IonRfl", 5, CatRifle}, {19, "w_BowCstr", 5, CatRifle}, {20, "w_BlstrCrbn", 5, CatRifle}, {21, "w_DsrptRfl", 5, CatRifle},
    {22, "w_SonicRfl", 5, CatRifle}, {23, "w_RptnBlstr", 6, CatHeavy}, {24, "w_HvRptBltr", 6, CatHeavy},
    {77, "w_Blstrrfl", 5, CatRifle}, {78, "w_gaffi", 3, CatMelee}, {79, "w_warblade", 3, CatMelee}, {80, "w_waraxe", 2, CatMelee},
    {93, "w_ForcePike", 3, CatMelee}, {104, "w_BlstrPstl", 4, CatPistol}, {105, "w_Blstrrfl", 5, CatRifle},
    {106, "w_Lghtsbr", 2, CatLightsaber}, {107, "w_Qtrstaff", 3, CatQStaff},
};

static void testClassifyTable() {
    for (const Row& r : kRows) {
        // plain stem and a model-variation suffix both classify, whatever the weaponwield says
        KT_CHECK_EQ((int)classify(r.stem, r.ww), (int)r.cat);
        char buf[64]; snprintf(buf, sizeof buf, "%s_001", r.stem);
        KT_CHECK_EQ((int)classify(buf, 0), (int)r.cat);
    }
}

static void testClassifyCaseAndBoundary() {
    KT_CHECK_EQ((int)classify("W_LGHTSBR_003", 9), (int)CatLightsaber);
    KT_CHECK_EQ((int)classify("w_dblsbr", 0), (int)CatLightsaber);
    // a longer stem that merely starts with a known one is NOT that class (falls to weaponwield)
    KT_CHECK_EQ((int)classify("w_Dblsbrx", 4), (int)CatPistol);
    KT_CHECK_EQ((int)classify("w_Lghtsbrmod_001", 0), (int)CatUnknown);
    // vibroblades are not lightsabers
    KT_CHECK((int)classify("w_Vbroshort", 2) != (int)CatLightsaber);
    KT_CHECK((int)classify("w_Dblswrd_001", 3) != (int)CatLightsaber);
}

static void testClassifyFallback() {
    KT_CHECK_EQ((int)classify("w_newsword", 1), (int)CatMelee);
    KT_CHECK_EQ((int)classify("w_newsword", 3), (int)CatMelee);
    KT_CHECK_EQ((int)classify("w_newgun", 4), (int)CatPistol);
    KT_CHECK_EQ((int)classify("w_newrifle", 5), (int)CatRifle);
    KT_CHECK_EQ((int)classify("w_newheavy", 6), (int)CatHeavy);
    KT_CHECK_EQ((int)classify("w_what", 0), (int)CatUnknown);
    KT_CHECK_EQ((int)classify(nullptr, 5), (int)CatRifle);
    KT_CHECK_EQ((int)classify("", 99), (int)CatUnknown);
}

static void testLongAndZone() {
    KT_CHECK(isLong(CatQStaff, 3));
    KT_CHECK(isLong(CatMelee, 3));        // double-bladed sword, pike, warblade, gaffi
    KT_CHECK(!isLong(CatMelee, 2));       // swords, axe
    KT_CHECK(!isLong(CatMelee, 1));       // stun baton
    KT_CHECK(isLong(CatRifle, 5) && isLong(CatHeavy, 6));
    KT_CHECK(!isLong(CatPistol, 4));
    KT_CHECK(!isLong(CatLightsaber, 3));  // the double saber still goes to the hip
    // locked requirement 2: sabers are hip even when "long"
    KT_CHECK_EQ((int)preferredZone(CatLightsaber, true), (int)ZoneHip);
    KT_CHECK_EQ((int)preferredZone(CatLightsaber, false), (int)ZoneHip);
    KT_CHECK_EQ((int)preferredZone(CatQStaff, true), (int)ZoneBack);
    KT_CHECK_EQ((int)preferredZone(CatMelee, false), (int)ZoneHip);
    KT_CHECK_EQ((int)preferredZone(CatPistol, false), (int)ZoneHip);
}

static void testMountGeometry() {
    for (int m = 0; m < MountCount; ++m) {
        Mount mm = static_cast<Mount>(m);
        KT_CHECK_EQ((int)mountOf(zoneOf(mm), sideOf(mm)), m);
        KT_CHECK(mountBone(mm)[0] != 0 && mountBoneAlt(mm)[0] != 0);
    }
    KT_CHECK(strcmp(mountBone(MountHipR), "rthigh_g") == 0);
    KT_CHECK(strcmp(mountBone(MountHipL), "lthigh_g") == 0);
    KT_CHECK(strcmp(mountBoneAlt(MountBackL), "torso_g") == 0);
    KT_CHECK(mountBone(MountNone)[0] == 0);
}

static unsigned bit(Mount m) { return 1u << m; }

static void testChooseMount() {
    // empty body: preferred zone, own side
    KT_CHECK_EQ((int)chooseMount(ZoneHip, SideRight, 0), (int)MountHipR);
    KT_CHECK_EQ((int)chooseMount(ZoneBack, SideLeft, 0), (int)MountBackL);
    // own-side hip taken -> same side back (set 2 lands on the other zone)
    KT_CHECK_EQ((int)chooseMount(ZoneHip, SideRight, bit(MountHipR)), (int)MountBackR);
    // both of this side taken -> other side, preferred zone
    KT_CHECK_EQ((int)chooseMount(ZoneHip, SideRight, bit(MountHipR) | bit(MountBackR)), (int)MountHipL);
    // only one free mount left
    KT_CHECK_EQ((int)chooseMount(ZoneHip, SideRight, bit(MountHipR) | bit(MountBackR) | bit(MountHipL)), (int)MountBackL);
    // all four taken
    KT_CHECK_EQ((int)chooseMount(ZoneHip, SideLeft, 0xF), (int)MountNone);
}

// Two full sets (4 weapons: set0 R/L, set1 R/L) fill the four mounts exactly once, sabers assigned first.
static void testTwoSetsFillFourMounts() {
    unsigned taken = 0;
    Mount got[4];
    const Zone zones[4] = {ZoneHip, ZoneHip, ZoneHip, ZoneHip};   // two lightsaber sets
    const Side sides[4] = {SideRight, SideLeft, SideRight, SideLeft};
    for (int i = 0; i < 4; ++i) {
        got[i] = chooseMount(zones[i], sides[i], taken);
        KT_CHECK(got[i] != MountNone);
        KT_CHECK((taken & bit(got[i])) == 0);
        taken |= bit(got[i]);
    }
    KT_CHECK_EQ((int)taken, 0xF);
    KT_CHECK_EQ((int)got[0], (int)MountHipR);
    KT_CHECK_EQ((int)got[1], (int)MountHipL);
    KT_CHECK_EQ((int)got[2], (int)MountBackR);   // second saber set spills to the back, same side
    KT_CHECK_EQ((int)got[3], (int)MountBackL);
}

static void testHilt() {
    KT_CHECK(hiltLengthMeters("w_Lghtsbr_001") > 0.33f && hiltLengthMeters("w_Lghtsbr_001") < 0.38f);
    KT_CHECK(hiltLengthMeters("w_ShortSbr") > 0.22f && hiltLengthMeters("w_ShortSbr") < 0.26f);
    KT_CHECK(hiltLengthMeters("w_Dblsbr_005") > 0.51f && hiltLengthMeters("w_Dblsbr_005") < 0.57f);
    KT_CHECK(hiltLengthMeters("w_Vbroshort") == 0.f);
    KT_CHECK(gripOffsetFromOrigin("w_Dblsbr") > 0.2f && gripOffsetFromOrigin("w_Lghtsbr") == 0.f);
}

static void testFsmDrawIsInstant() {
    Fsm f;
    KT_CHECK(!f.inHand());
    KT_CHECK(f.handFraction(0) == 0.f);
    f.onCombat(1, 1000);
    // requirement 3: the weapon is in the hand on the very edge, before any slide time passes
    KT_CHECK(f.inHand());
    KT_CHECK(f.phase == PhDrawing);
    KT_CHECK(f.handFraction(1000) == 0.f);
    KT_CHECK(f.handFraction(1000 + kDrawMs / 2) > 0.4f && f.handFraction(1000 + kDrawMs / 2) < 0.6f);
    f.tick(1000 + kDrawMs);
    KT_CHECK(f.phase == PhDrawn && f.handFraction(5000) == 1.f);
}

static void testFsmStowDelayAndCancel() {
    Fsm f;
    f.onCombat(1, 0);
    f.tick(kDrawMs);
    f.onCombat(0, 5000);
    f.tick(5000 + kStowDelayMs - 1);
    KT_CHECK(f.phase == PhDrawn);                       // still drawn during the exit countdown
    f.tick(5000 + kStowDelayMs);
    KT_CHECK(f.phase == PhStowing);
    KT_CHECK(f.handFraction(5000 + kStowDelayMs) == 1.f);
    f.tick(5000 + kStowDelayMs + kDrawMs);
    KT_CHECK(f.phase == PhHolstered && f.handFraction(99999) == 0.f);
    // combat re-entry during the exit countdown cancels the stow
    Fsm g;
    g.onCombat(1, 0); g.tick(kDrawMs);
    g.onCombat(0, 2000);
    g.onCombat(1, 2500);
    KT_CHECK(!g.stowPending && g.phase == PhDrawing);
    g.tick(2500 + kStowDelayMs + kDrawMs);
    KT_CHECK(g.phase == PhDrawn);
}

static void testFsmSwapAndRollover() {
    Fsm f;
    f.onCombat(1, 0); f.tick(kDrawMs);
    f.onSwap(2000);
    KT_CHECK(f.phase == PhDrawing && f.phaseLen == kSwapMs);
    f.tick(2000 + kSwapMs - 1);
    KT_CHECK(f.phase == PhDrawing);
    f.tick(2000 + kSwapMs);
    KT_CHECK(f.phase == PhDrawn);
    // tick wrap: stowAt computed across the 32-bit wrap still fires
    Fsm w;
    w.onCombat(1, 0xFFFFFF00u); w.tick(0xFFFFFF00u + kDrawMs);
    w.onCombat(0, 0xFFFFFFF0u);
    w.tick(0xFFFFFFF0u + kStowDelayMs);
    KT_CHECK(w.phase == PhStowing);
    // never-drawn creature never starts a stow
    Fsm h;
    h.onCombat(0, 100); h.tick(100 + kStowDelayMs * 2);
    KT_CHECK(h.phase == PhHolstered);
}

static void testPlacementDefaults() {
    for (int c = 0; c < CatCount; ++c)
        for (int m = 0; m < MountCount; ++m) {
            Placement p = placementFor(static_cast<Category>(c), static_cast<Mount>(m));
            KT_CHECK(p.pos[0] == 0.f && p.rotDeg[2] == 0.f);   // probe defaults: identity until P2 tunes them
        }
}

// COMB old state (Holster.cpp HolsterCombat): an unreadable old state must never print old=0 edge=1.
static void testCombFields() {
    CombFields a = combFields(true, 0u, 1);            // readable 0 -> 1: a real combat-on edge
    KT_CHECK(a.oldKnown && a.old == 0 && a.edge == 1);
    CombFields b = combFields(true, 1u, 1);            // readable 1 -> 1: no edge
    KT_CHECK(b.oldKnown && b.old == 1 && b.edge == 0);
    CombFields c = combFields(true, 5u, 0);            // readable 5 -> 0: edge
    KT_CHECK(c.oldKnown && c.edge == 1);
    CombFields d = combFields(false, 0u, 1);           // unreadable (rdU32 gives 0 today) -> 1: must print old=? edge=?
    KT_CHECK(!d.oldKnown);
    CombFields e = combFields(false, 0u, 0);           // unreadable -> 0: also unknown
    KT_CHECK(!e.oldKnown);
    KT_CHECK(d.edge == 0 && e.edge == 0);              // an unknown old state never claims an edge
    CombFields g = combFields(false, 0xFFFFFFFFu, 1);  // unreadable with the no-offset sentinel value: still unknown
    KT_CHECK(!g.oldKnown);
    CombFields h = combFields(false, 0xFFFFFFFFu, 0);  // same, new==0
    KT_CHECK(!h.oldKnown);
    CombFields i = combFields(true, 0u, 0);            // readable 0 -> 0: known, no edge
    KT_CHECK(i.oldKnown && i.old == 0 && i.edge == 0);
    CombFields j = combFields(true, 0xFFFFFFFFu, 1);   // a real readable -1 is known (prints old=-1), not mistaken for unreadable
    KT_CHECK(j.oldKnown && j.old == -1 && j.edge == 0);
    CombFields k = combFields(true, 5u, 7);               // nonzero -> nonzero: no edge
    KT_CHECK(k.oldKnown && k.old == 5 && k.edge == 0);
}

int main() {
    testClassifyTable();
    testClassifyCaseAndBoundary();
    testClassifyFallback();
    testLongAndZone();
    testMountGeometry();
    testChooseMount();
    testTwoSetsFillFourMounts();
    testHilt();
    testFsmDrawIsInstant();
    testFsmStowDelayAndCancel();
    testFsmSwapAndRollover();
    testPlacementDefaults();
    testCombFields();
    return kt::Report("holster_logic");
}
