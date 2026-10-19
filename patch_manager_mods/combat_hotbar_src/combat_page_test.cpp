// combat_page_test.cpp -- host test of CombatPage.h: the shipped Mod Configuration schema parses, binds 1:1 to combat_hotbar.ini [Buttons]
// keys, and its defaults / ranges stay inside what CombatHotbar.cpp loadIni accepts (clamps mirrored here) and equal the mod's own defaults.
#include "CombatPage.h"
#include "CombatLogic.h"
#include "ModConfigDecl.h"
#include "check.h"
#include <string.h>

struct Clamp { const char* key; int def, lo, hi; };

// Mirrors CombatHotbar.cpp loadIni(): BtnPx is 0 (auto) or 8..256; the others are clampInt(def, lo, hi).
static const Clamp kIni[] = {
    {"BtnPx", 0, 0, 256},
    {"ColsTotal", 6, 1, 12},
    {"Pad", ch::kDefaultPad, 0, 32},
    {"GroupGap", ch::kDefaultGroupGap, 0, 64},
};

static void test_parse_and_bindings() {
    static mcdecl::PageDecl pd;
    char err[96];
    KT_CHECK(mcdecl::parseSchema(chpage::kPageSchema, strlen(chpage::kPageSchema), pd, err, sizeof err));
    KT_CHECK_EQ(pd.nRows, 4);
    KT_CHECK(strcmp(pd.name, "Combat Hotbar") == 0);
    const char* ids[4] = {chpage::kRowSize, chpage::kRowCols, chpage::kRowPad, chpage::kRowGap};
    for (int i = 0; i < 4; ++i) {
        const mcdecl::RowDecl& r = pd.rows[i];
        KT_CHECK(strcmp(r.id, ids[i]) == 0);
        KT_CHECK_EQ(r.kind, (int)mcdecl::K_STEPPER);
        KT_CHECK(strcmp(r.ini, chpage::kIniFile) == 0);
        KT_CHECK(strcmp(r.section, "Buttons") == 0);
        KT_CHECK(strcmp(r.key, kIni[i].key) == 0);
        KT_CHECK_EQ(r.def, kIni[i].def);
        KT_CHECK(r.minV >= kIni[i].lo && r.maxV <= kIni[i].hi);   // every menu value is accepted unclamped by the mod
        KT_CHECK(r.def >= r.minV && r.def <= r.maxV);
        KT_CHECK(r.step >= 1 && (r.maxV - r.minV) % r.step == 0);
    }
}

static void test_size_row_auto_and_floor() {
    static mcdecl::PageDecl pd;
    char err[96];
    KT_CHECK(mcdecl::parseSchema(chpage::kPageSchema, strlen(chpage::kPageSchema), pd, err, sizeof err));
    const mcdecl::RowDecl& r = pd.rows[pd.find(chpage::kRowSize)];
    KT_CHECK_EQ(r.def, 0);                       // 0 = auto size (CombatHotbar.cpp:142)
    KT_CHECK(strcmp(r.zero, "Auto") == 0);
    KT_CHECK_EQ(r.step, 8);                      // 0, 8, 16 ...: never a value in 1..7, which loadIni would turn into auto
}

static void test_text_budgets() {
    static mcdecl::PageDecl pd;
    char err[96];
    KT_CHECK(mcdecl::parseSchema(chpage::kPageSchema, strlen(chpage::kPageSchema), pd, err, sizeof err));
    for (int i = 0; i < pd.nRows; ++i) {
        KT_CHECK(strlen(pd.rows[i].label) <= 40);   // fits the option row caption
        KT_CHECK(strlen(pd.rows[i].desc) > 0 && strlen(pd.rows[i].desc) < 191);
    }
}

int main() {
    test_parse_and_bindings();
    test_size_row_auto_and_floor();
    test_text_budgets();
    return kt::Report("combat_page_test");
}
