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

int main() {
    testParseMask();
    testInWeaponRange();
    testDrift();
    return kt::Report("rai_logic_test");
}
