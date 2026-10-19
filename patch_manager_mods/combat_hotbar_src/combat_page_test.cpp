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

// The test-state modal table: defaults equal the starting Demo, ranges stay inside what chprev::build accepts, strings fit the provider's buffers.
static void test_modal_rows() {
    chprev::Demo d0;
    for (int r = 0; r < chpage::MR_COUNT; ++r) {
        const chpage::ModalRowDef& m = chpage::kModalRows[r];
        KT_CHECK(m.kind == 0 || m.kind == 1);
        KT_CHECK(strlen(m.label) > 0 && strlen(m.label) <= 40);
        KT_CHECK(strlen(m.desc) > 0 && strlen(m.desc) < 191);   // the provider keeps 192 bytes of description
        KT_CHECK(m.def >= m.minV && m.def <= m.maxV);
        if (m.choices) { KT_CHECK_EQ(m.nChoices, m.maxV - m.minV + 1); for (int c = 0; c < m.nChoices; ++c) KT_CHECK(m.choices[c] && strlen(m.choices[c]) > 0); }
        if (r != chpage::MR_RESET) KT_CHECK_EQ(chpage::demoValue(d0, r), m.def);   // the row starts at the Demo default
    }
    KT_CHECK_EQ(chpage::kModalRows[chpage::MR_ATTACKS].maxV, 16); KT_CHECK_EQ(chpage::kModalRows[chpage::MR_SCREEN].maxV, chprev::kNumScreens - 1);
    // apply / value round trip and clamping
    chprev::Demo d;
    chpage::demoApply(d, chpage::MR_POWERS, 0); KT_CHECK_EQ(d.count[1], 0); KT_CHECK_EQ(chpage::demoValue(d, chpage::MR_POWERS), 0);
    chpage::demoApply(d, chpage::MR_ITEMS, 99); KT_CHECK_EQ(d.count[2], 16);
    chpage::demoApply(d, chpage::MR_ATTACKS, -4); KT_CHECK_EQ(d.count[0], 0);
    chpage::demoApply(d, chpage::MR_UNUSABLE, 2); KT_CHECK_EQ(d.unusable, 2);
    chpage::demoApply(d, chpage::MR_SCREEN, 9); KT_CHECK_EQ(d.screen, chprev::kNumScreens - 1);
    chpage::demoApply(d, chpage::MR_RESET, 1); chpage::demoApply(d, 77, 1);   // not fields: nothing changes
    KT_CHECK_EQ(d.count[2], 16); KT_CHECK_EQ(d.unusable, 2);
    // the demo rows drive a model that stays on screen for every screen choice
    for (int s = 0; s < chprev::kNumScreens; ++s) {
        chprev::Demo e; chpage::demoApply(e, chpage::MR_SCREEN, s); chpage::demoApply(e, chpage::MR_ITEMS, 10);
        chprev::Model m = chprev::build(e, chprev::Settings());
        KT_CHECK_EQ(m.sw, chprev::kScreens[s].w);
        KT_CHECK_EQ(m.outN[2], 8);   // finding: with the default Cols=2 x MaxRows=4 (per 8) a 10-item group shows only 8 buttons; the rest are cut, not scrolled
    }
    KT_CHECK(chpage::kPreviewPermille >= 600 && chpage::kPreviewPermille <= 850);
    KT_CHECK(strlen(chpage::kModalTitle) < 64);
    KT_CHECK(strlen(chpage::kGearDesc) < 191);
    // item budget of the options page: pool labels + 4 stepper rows (3 items each) + the gear <= mod-config's 80 per page
    KT_CHECK(chprev::kPoolSize + 4 * 3 + 1 <= 80);
}

int main() {
    test_modal_rows();
    test_parse_and_bindings();
    test_size_row_auto_and_floor();
    test_text_budgets();
    return kt::Report("combat_page_test");
}
