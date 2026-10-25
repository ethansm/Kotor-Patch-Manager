// Host tests for RangedAiLogic.h, plus a drift guard against the companion-ai copies (cai::parseMask,
// cai::inWeaponRange). The cross-dir include is test-only; RangedAI.cpp never includes companion_ai_src.
#include "RangedAiLogic.h"
#include "../companion_ai_src/CompanionAiLogic.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

static void testParseMask() {
    kt::CheckEq("default '16'", (unsigned long long)rai::parseMask("16"), 1ull << 16);
    kt::CheckEq("list with spaces", (unsigned long long)rai::parseMask(" 11, 12 ,16"), (1ull << 11) | (1ull << 12) | (1ull << 16));
    kt::CheckEq("0 and 31 kept, 32 dropped", (unsigned long long)rai::parseMask("0,31,32,99"), 1ull | (1ull << 31));
    kt::CheckEq("empty", (unsigned long long)rai::parseMask(""), 0ull);
    kt::CheckEq("junk separators", (unsigned long long)rai::parseMask("a1;b2|3x"), 0xEull);
    // F27: the old ranged-ai copy dereferenced a null string and overflowed int on long digit runs (UB).
    kt::CheckEq("F27 null -> 0", (unsigned long long)rai::parseMask(nullptr), 0ull);
    kt::CheckEq("F27 overflowing run ignored, next value kept", (unsigned long long)rai::parseMask("4294967312,5"), 1ull << 5);
    kt::CheckEq("F27 99999999999999999999 ignored", (unsigned long long)rai::parseMask("99999999999999999999"), 0ull);
}

static void testInWeaponRange() {
    kt::Check("in range", rai::inWeaponRange(10.0f, 100.0f));
    kt::Check("edge: (10.1)^2", rai::inWeaponRange(10.0f, 10.1f * 10.1f));
    kt::Check("just outside", !rai::inWeaponRange(10.0f, 10.2f * 10.2f));
    kt::Check("zero range never", !rai::inWeaponRange(0.0f, 0.0f));
    kt::Check("range 1.0 never", !rai::inWeaponRange(1.0f, 0.0f));
    kt::Check("NaN range never", !rai::inWeaponRange(__builtin_nanf(""), 0.0f));
}

// Drift guard: rai:: and cai:: must agree everywhere.
static void testDrift() {
    const char* masks[] = {"", "16", "11,12,16", " 0 , 31 ", "32", "a1b2", ",,,", "1,,2", "007", "31,30,29,0",
                           "2147483647", "2147483648", "4294967312,5", "99999999999999999999,3", "-5", "1 2 3", nullptr};
    int bad = 0;
    for (const char* m : masks)
        if (rai::parseMask(m) != cai::parseMask(m)) { ++bad; printf("drift parseMask(\"%s\")\n", m ? m : "(null)"); }
    char buf[16];
    for (int i = 0; i < 2000; ++i) {
        snprintf(buf, sizeof buf, "%d", i);
        if (rai::parseMask(buf) != cai::parseMask(buf)) ++bad;
    }
    kt::CheckEq("parseMask rai == cai", (long long)bad, 0ll);
    bad = 0;
    long n = 0;
    for (float r = -2.0f; r <= 40.0f; r += 0.37f)
        for (float d = 0.0f; d <= 1800.0f; d += 3.3f, ++n)
            if (rai::inWeaponRange(r, d) != cai::inWeaponRange(r, d)) ++bad;
    kt::Check("inWeaponRange sweep size (~62k points)", n > 50000);
    kt::CheckEq("inWeaponRange rai == cai", (long long)bad, 0ll);
}

struct TSlot { int id = 0; int v = 0; };

static void testSlotFor() {
    TSlot a[4];
    TSlot* x = rai::slotFor(a, 10); x->v = 7;
    kt::Check("same id -> same slot", rai::slotFor(a, 10) == x && x->v == 7);
    TSlot* y = rai::slotFor(a, 11);
    kt::Check("new id -> free slot", y != x && y->id == 11);
    rai::slotFor(a, 12); rai::slotFor(a, 13);
    TSlot* z = rai::slotFor(a, 14);   // full: hash slot, reset
    kt::Check("collision slot is reset and owned", z->id == 14 && z->v == 0);
    kt::CheckEq("collision evicts the previous owner", (long long)(rai::slotFor(a, 14) == z), 1ll);
}

static void testDecisionThrottle() {
    using namespace rai;
    kt::Check("first N always log", shouldLogDecision(3, 16, 5, 5, 100, 100, 2000));
    kt::Check("same sig within gate -> no", !shouldLogDecision(16, 16, 5, 5, 2000, 100, 2000));
    kt::Check("same sig at exactly the gate -> no", !shouldLogDecision(16, 16, 5, 5, 2100, 100, 2000));
    kt::Check("same sig past the gate -> yes", shouldLogDecision(16, 16, 5, 5, 2101, 100, 2000));
    kt::Check("changed sig -> yes", shouldLogDecision(16, 16, 6, 5, 101, 100, 2000));
    kt::Check("debug gate 250 ms logs sooner", shouldLogDecision(16, 16, 5, 5, 400, 100, retargetGateMs(true)));
    kt::Check("normal gate 2 s does not", !shouldLogDecision(16, 16, 5, 5, 400, 100, retargetGateMs(false)));
    kt::Check("tick wrap safe", !shouldLogDecision(16, 16, 5, 5, 50, 0xFFFFFFF0u, 2000));
    kt::Check("sig: gate differs", retargetSig(true, 16, 1, 0) != retargetSig(false, 16, 1, 0));
    kt::Check("sig: stance differs", retargetSig(true, 16, 1, 0) != retargetSig(true, 11, 1, 0));
    kt::Check("sig: ranged differs", retargetSig(true, 16, 1, 0) != retargetSig(true, 16, 0, 0));
    kt::Check("sig: los differs", retargetSig(true, 16, 1, 0) != retargetSig(true, 16, 1, 5));
    kt::Check("sig: los value is boolean", retargetSig(true, 16, 1, 5) == retargetSig(true, 16, 1, 9));
    kt::Check("sig: stance -1 distinct and stable", retargetSig(true, -1, 1, 0) == retargetSig(true, -1, 1, 0) && retargetSig(true, -1, 1, 0) != retargetSig(true, 0, 1, 0));
}

static void testChangeThrottle() {
    using namespace rai;
    kt::Check("first time logs", shouldLogChange(false, 1, 0, 10, 0, 250));
    kt::Check("same sig never", !shouldLogChange(true, 1, 1, 100000, 10, 250));
    kt::Check("changed but inside gap -> held back", !shouldLogChange(true, 2, 1, 200, 10, 250));
    kt::Check("changed, gap reached", shouldLogChange(true, 2, 1, 260, 10, 250));
}

static void testBudgets() {
    using namespace rai;
    LineBudget b;
    int ok = 0, marker = 0, drop = 0;
    for (int i = 0; i < 20; ++i) { Take t = b.take(5); if (t == Take::Ok) ++ok; else if (t == Take::Marker) ++marker; else ++drop; }
    kt::CheckEq("5 ok", (long long)ok, 5ll);
    kt::CheckEq("exactly one marker", (long long)marker, 1ll);
    kt::CheckEq("rest dropped", (long long)drop, 14ll);
    LineBudget info, warn;   // independence: exhausting INFO never touches WARN
    for (int i = 0; i < MaxInfoLines + 10; ++i) info.take(MaxInfoLines);
    kt::Check("WARN budget untouched by INFO exhaustion", warn.take(MaxWarnLines) == Take::Ok);
    int w = 0;
    for (int i = 0; i < 300; ++i) if (warn.take(MaxWarnLines) == Take::Ok) ++w;
    kt::CheckEq("WARN budget: 200 total ok (1 used above)", (long long)w, 199ll);
    kt::Check("WARN marker already issued once", warn.marked);
    kt::CheckEq("menu budget normal", (long long)menuLogBudget(false), 20ll);
    kt::CheckEq("menu budget debug", (long long)menuLogBudget(true), 100ll);
    int s1 = 0, s2 = 0; bool marker2 = false; int markers = 0;
    for (int i = 0; i < 30; ++i) {
        Take t1 = menuTake(s1, 20, marker2); if (t1 == Take::Marker) ++markers;
        Take t2 = menuTake(s2, 20, marker2); if (t2 == Take::Marker) ++markers;
    }
    kt::CheckEq("S1 and S2 each take 20", (long long)(s1 + s2), 40ll);
    kt::CheckEq("S1/S2 share one marker", (long long)markers, 1ll);
}

static void testKindThrottle() {
    using namespace rai;
    KindThrottle k; unsigned sup = 99;
    kt::Check("first allowed", k.allow(1000, 5000, &sup) && sup == 0);
    kt::Check("repeat 1 suppressed", !k.allow(2000, 5000, &sup));
    kt::Check("repeat 2 suppressed", !k.allow(5999, 5000, &sup));
    kt::Check("after 5 s allowed, reports 2", k.allow(6000, 5000, &sup) && sup == 2);
    kt::Check("counter reset", !k.allow(6100, 5000, &sup) && k.allow(11000, 5000, &sup) && sup == 1);
    KindThrottle other; kt::Check("kinds independent", other.allow(6100, 5000, &sup) && sup == 0);
    char b[32];
    suppressedSuffix(b, sizeof b, 0); kt::Check("suffix empty for 0", b[0] == 0);
    suppressedSuffix(b, sizeof b, 7); kt::Check("suffix text", strcmp(b, " (+7 suppressed)") == 0);
}

static void testNoHold() {
    using namespace rai;
    kt::Check("all good -> None", noHoldReason(true, false, true, true, true, true, true, true) == NoHold::None);
    kt::Check("hold=0", noHoldReason(false, false, true, true, true, true, true, true) == NoHold::HoldOff);
    kt::Check("PC", noHoldReason(true, true, true, true, true, true, true, true) == NoHold::IsPC);
    kt::Check("stance", noHoldReason(true, false, false, true, true, true, true, true) == NoHold::StanceNotInMask);
    kt::Check("no ranged", noHoldReason(true, false, true, false, true, true, true, true) == NoHold::NoRanged);
    kt::Check("not blocked", noHoldReason(true, false, true, true, false, true, true, true) == NoHold::ShotNotBlocked);
    kt::Check("out of range", noHoldReason(true, false, true, true, true, false, true, true) == NoHold::OutOfRange);
    kt::Check("area", noHoldReason(true, false, true, true, true, true, false, true) == NoHold::AreaMismatch);
    kt::Check("target type", noHoldReason(true, false, true, true, true, true, true, false) == NoHold::TargetNotCreature);
    kt::Check("priority: stance before ranged", noHoldReason(true, false, false, false, false, false, false, false) == NoHold::StanceNotInMask);
    kt::Check("text", strcmp(noHoldText(NoHold::NoRanged), "no ranged weapon") == 0 && strcmp(noHoldText(NoHold::None), "held") == 0);
}

int main() {
    testParseMask();
    testInWeaponRange();
    testDrift();
    testSlotFor();
    testDecisionThrottle();
    testChangeThrottle();
    testBudgets();
    testKindThrottle();
    testNoHold();
    return kt::Report("rai_logic_test");
}
