// quick_loot_page_test.cpp -- host test of the Quick Loot Mod Configuration page (QuickLootPage.h, doc 52 QL7): the shipped schema parses, its rows
// are the four documented ones bound to the right ini keys, the defaults equal the mod's own, every string fits the TomlMini limit, and the pure
// row handler (applyRow) maps the page values to the live config the way QuickLoot.cpp applies them.
#include "ModConfigDecl.h"
#include "QuickLootPage.h"
#include "check.h"
#include <stdio.h>
#include <string.h>

using namespace qlpage;

static mcdecl::PageDecl g_pd;   // ~35 KB: static

int main() {
    char err[96];
    const bool ok = mcdecl::parseSchema(kPageSchema, strlen(kPageSchema), g_pd, err, sizeof err);
    if (!ok) printf("schema error: %s\n", err);
    kt::Check("the shipped schema parses", ok);
    kt::CheckEq("eight rows (0.7.1-dev: tip, sort, equip, panel)", g_pd.nRows, 8);
    kt::Check("page name", strcmp(g_pd.name, "Quick Loot") == 0 && strcmp(g_pd.title, "Quick Loot") == 0);
    const char* ids[8] = { kRowAnchor, kRowProx, kRowRows, kRowDist, kRowTip, kRowSort, kRowEquip, kRowPanel };
    for (int i = 0; i < 8 && i < g_pd.nRows; ++i) kt::Check(ids[i], strcmp(g_pd.rows[i].id, ids[i]) == 0);
    if (g_pd.nRows == 8) {
        const mcdecl::RowDecl& a = g_pd.rows[0];
        kt::Check("anchor: list of the three modes, bound to Anchor", a.kind == mcdecl::K_LIST && a.nChoices == qlogic::AnchorModeCount && a.def == 0 &&
                  strcmp(a.ini, "quick_loot.ini") == 0 && strcmp(a.section, "QuickLoot") == 0 && strcmp(a.key, "Anchor") == 0 &&
                  strcmp(a.choices[0], "Under name tag") == 0 && strcmp(a.choices[1], "Below container") == 0 && strcmp(a.choices[2], "Beside name tag") == 0);
        const mcdecl::RowDecl& p = g_pd.rows[1];
        kt::Check("prox: toggle on by default, bound to ProximityEnabled", p.kind == mcdecl::K_TOGGLE && p.def == 1 && strcmp(p.key, "ProximityEnabled") == 0 && strcmp(p.ini, "quick_loot.ini") == 0);
        const mcdecl::RowDecl& r = g_pd.rows[2];
        kt::Check("rows: stepper 4..12 step 2, default 4 (QL10), bound to ListRows", r.kind == mcdecl::K_STEPPER && r.minV == 4 && r.maxV == 12 && r.step == 2 &&
                  r.def == 4 && r.def == static_cast<int32_t>(qlogic::ListRowsDefault) && strcmp(r.key, "ListRows") == 0 && strcmp(r.fmt, "List rows: %d") == 0);
        kt::Check("rows: every stepper stop is a valid ListRows", qlogic::clampListRows(r.minV) == static_cast<unsigned>(r.minV) && qlogic::clampListRows(r.maxV) == static_cast<unsigned>(r.maxV));
        const mcdecl::RowDecl& d = g_pd.rows[3];
        kt::Check("dist: unbound list of the presets, Normal default", d.kind == mcdecl::K_LIST && d.nChoices == kNumDist && d.def == kDistDefault && d.ini[0] == 0 && d.key[0] == 0);
        const mcdecl::RowDecl& t = g_pd.rows[4];
        kt::Check("tip: Full / Stats only / Off, Full default, bound to TipDetail", t.kind == mcdecl::K_LIST && t.nChoices == 3 && t.def == TipFull &&
                  strcmp(t.choices[0], "Full") == 0 && strcmp(t.choices[1], "Stats only") == 0 && strcmp(t.choices[2], "Off") == 0 && strcmp(t.key, "TipDetail") == 0);
        const mcdecl::RowDecl& so = g_pd.rows[5];
        kt::Check("sort: toggle on, bound to SortFirst", so.kind == mcdecl::K_TOGGLE && so.def == 1 && strcmp(so.key, "SortFirst") == 0 && strcmp(so.ini, "quick_loot.ini") == 0);
        const mcdecl::RowDecl& eq = g_pd.rows[6];
        kt::Check("equip: toggle on, bound to EquippedLine", eq.kind == mcdecl::K_TOGGLE && eq.def == 1 && strcmp(eq.key, "EquippedLine") == 0);
        const mcdecl::RowDecl& pa = g_pd.rows[7];
        kt::Check("panel: Vanilla / Dark, Dark default, bound to Panel", pa.kind == mcdecl::K_LIST && pa.nChoices == 2 && pa.def == 1 &&
                  strcmp(pa.choices[0], "Vanilla") == 0 && strcmp(pa.choices[1], "Dark") == 0 && strcmp(pa.key, "Panel") == 0);
        for (int i = 0; i < 8; ++i) kt::Check("row ids are unique and short", strlen(g_pd.rows[i].id) < 31 && g_pd.find(g_pd.rows[i].id) == i);
        for (int i = 0; i < 8; ++i) kt::Check("desc fits one line of the description pane", strlen(g_pd.rows[i].desc) <= 95);
    }
    // defaults pinned to the mod's own
    kt::Check("Normal preset = the mod default OpenM, CloseM follows to the mod default", kDistOpenM[kDistDefault] == qlogic::DefaultOpenM && closeFor(qlogic::DefaultOpenM, false, 0) == qlogic::DefaultCloseM);
    // presets
    kt::CheckEq("distIndexFor 2.0", distIndexFor(2.0f), 0);
    kt::CheckEq("distIndexFor 3.0", distIndexFor(3.0f), 1);
    kt::CheckEq("distIndexFor 4.5", distIndexFor(4.5f), 2);
    kt::CheckEq("distIndexFor 3.4 -> Normal", distIndexFor(3.4f), 1);
    kt::CheckEq("distIndexFor 3.9 -> Far", distIndexFor(3.9f), 2);
    kt::CheckEq("distIndexFor 0.5 -> Near", distIndexFor(0.5f), 0);
    kt::CheckEq("distIndexFor 50 -> Far", distIndexFor(50.0f), 2);
    for (int i = 0; i < kNumDist; ++i) kt::Check("each preset keeps the hysteresis band >= 1 m", closeFor(kDistOpenM[i], false, 0) >= kDistOpenM[i] + 1.0f && qlogic::clampOpenM(kDistOpenM[i]) == kDistOpenM[i]);
    kt::Check("explicit CloseM wins", closeFor(3.0f, true, 6.5f) == 6.5f);
    kt::Check("explicit CloseM is kept >= OpenM + 1", closeFor(4.5f, true, 5.0f) == 5.5f);
    // applyRow
    Live L;
    kt::Check("Live::rows defaults to 4 (QL10)", L.rows == 4 && qlogic::ListRowsDefault == 4);
    bool persist = false;
    kt::CheckEq("anchor 2", applyRow(L, "anchor", 2, &persist), 2);
    kt::Check("anchor applied, nothing persisted by the hook", L.anchor == 2 && !persist);
    kt::CheckEq("anchor out of range -> 0", applyRow(L, "anchor", 9, &persist), 0);
    kt::CheckEq("prox off", applyRow(L, "prox", 0, &persist), 0);
    kt::Check("prox applied", !L.prox);
    applyRow(L, "prox", 1, &persist);
    kt::Check("prox on again", L.prox);
    kt::CheckEq("rows 10", applyRow(L, "rows", 10, &persist), 10);
    kt::Check("rows applied", L.rows == 10);
    kt::CheckEq("rows clamped to 12", applyRow(L, "rows", 99, &persist), 12);
    kt::CheckEq("dist Far", applyRow(L, "dist", 2, &persist), 2);
    kt::Check("dist: OpenM 4.5, CloseM 6.5, the hook persists OpenM", L.openM == 4.5f && L.closeM == 6.5f && persist);
    L.closeExplicit = true; L.closeExplicitM = 7.0f;
    applyRow(L, "dist", 0, &persist);
    kt::Check("dist Near with an explicit CloseM: CloseM stays 7", L.openM == 2.0f && L.closeM == 7.0f && persist);
    kt::CheckEq("dist out of range -> Normal", applyRow(L, "dist", 7, &persist), kDistDefault);
    kt::Check("Live defaults match the schema: tip Full, sort on, equip on, panel Dark", L.tip == TipFull && L.sort && L.equip && L.panel == 1);
    kt::CheckEq("tip Stats only", applyRow(L, "tip", TipStats, &persist), TipStats);
    kt::CheckEq("tip out of range -> Full", applyRow(L, "tip", 9, &persist), TipFull);
    kt::CheckEq("sort off", applyRow(L, "sort", 0, &persist), 0);
    kt::Check("sort applied", !L.sort);
    kt::CheckEq("equip off", applyRow(L, "equip", 0, &persist), 0);
    kt::CheckEq("panel Vanilla", applyRow(L, "panel", 0, &persist), 0);
    kt::CheckEq("panel anything else -> Dark", applyRow(L, "panel", 5, &persist), 1);
    kt::Check("the bound rows persist nothing through the hook", !persist);
    kt::CheckEq("unknown row: value passes through", applyRow(L, "nope", 5, &persist), 5);
    kt::Check("unknown row persists nothing", !persist);
    return kt::Report("quick_loot_page_test");
}
