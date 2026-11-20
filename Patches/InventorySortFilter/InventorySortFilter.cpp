// Inventory Sort & Filter (Steam Aspyr build 6A522E71...).
// Sorts the backpack rows of the inventory screen, tints items the shown character cannot equip and adds
// New / Usable / Upgradeable filters by re-clicking the active All tab. Research trail and every address:
// project design doc 11 (local-setup/KOTOR-II worktree).
// Build marker: title row "Equipped vs Viewed" on top of the compare box, hide on hidden description list.
//
// Hooks (cdecl detours):
//   H6a 0x008a8cf0 SortFilterTabClick   tab-click handler prologue; returns 1 = click consumed (consumed_exit_address)
//   H6b 0x008a7e22 SortFilterBufferRow  PopulateItemListBox backpack loop; replaces "reselect + CALL CreateItemEntry"
//   H6c 0x008a7e76 SortFilterFlush      PopulateItemListBox loop exit; sorts the buffer and creates the rows
//   H6d 0x008a8478 SortFilterRowSelect  row-select handler 0x8a83d0 epilogue; re-applies the tint that SetItem
//                                       (via 0x8b1cc0, only for New items) just reset
//   H6e 008a8997  SortFilterBumperPrev  controller bumper prev (input 0x2F): PUSH/MOV/CALL SetNextFilter replaced (9 bytes, skipped);
//   H6f 008a89f7  SortFilterBumperNext  bumper next (input 0x30): with the cycle on, New / Usable / Upgradeable join the bumper ring
//
// Stages are controlled from inventory_sort_filter.ini (re-read at most once a second, no restart needed):
//   all flags 0 = stage 0 (logging only; rows are still created through the buffer, in vanilla order).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define INIW_FILE_API 1   // ModConfigDecl.h ini-bound rows write through IniWrite.h's file API (doc 46); before the first include of it
#define MCDECL_FILE_API 1

#include "../Common/SafeRead.h"   // bare, like GuiKit.h / GameAddr.h: one copy (Common) for every header
#include "SortLogic.h"
#include "../Common/GameAddr.h"
#include "../Common/KpLog.h"
#include "../Common/ModConfigApi.h"
#include "../Common/ModConfigDecl.h"     // Options -> Mod Configuration page (isf::kPageSchema)
#include "../Common/ProviderClient.h"
#include "SortOrderPage.h"
#include "../Common/HudHost.h"           // K2HUD_PANEL_BUILD: the strip labels are cloned while the inventory layout is open (doc 11 VU4)
#include "../Common/GuiKit.h"
#include "../Common/PreviewCanvas.h"     // options-page preview list (VU7a): labels listed MUI_NOFREE, freed in pvDestroy
#include "IsfPreview.h"       // pure preview model (isfprev::build / buildPrims)
#include "../Common/ItemCompare.h"      // equip-compare band A (S10): itemcmp::compare -> two band lines
#include "EquipPanel.h"
#include "EquipBoxLogic.h"
#include "../Common/Party.h"             // EQ-P1 B party fingerprint (probe only): table / member / serverCreature
#include "../Common/ItemCompareRead.h"  // engine memory -> CompareInput (viewedFromRow / readFacts / fillSlots)
#include <new>               // placement new: in-place reset of the compare input
#include "../Common/TextFit.h"          // textfit::fitText: pixel-width "..." cut of each band line
#include "CompareBand.h"      // pure show/hide + dedupe-key helpers
#include "BoxGeometry.h"      // compare box sizing (pure)
#include "StripLayout.h"      // VU4 strip: four state slot rects and the sort rect (pure)
#include "StripClick.h"       // VU4 strip: hover / click region decisions (pure)

KP_LOG_PATCH("inventory-sort-filter");

namespace {

// ---- engine addresses (doc 11 section 2) ----
constexpr int GuiGlobal = 0x00A1B4A4;         // *(GuiGlobal)+4 = CClientExoAppInternal-ish holder; ECX for FnGetGuiInGame
constexpr int FnGetGuiInGame = 0x0073F750;    // thiscall(holder) -> GuiInGame*
constexpr int FilterModeOff = 0xFC1;          // byte on GuiInGame
constexpr int FnCreateItemEntry = 0x008A75F0; // thiscall(panel; cobj, int* counter, list*, item, slot, equipped) RET 0x18
constexpr int FnIsItemUsable = 0x0077B670;    // thiscall(cobj; item, slot, mask) RET 0xC -- "has an activatable property", NOT an equip test
constexpr int FnGetBaseItem = (int)gameaddr::GetBaseItem;   // thiscall(item), no stack args -> baseitems row (slot mask at +4, gender req byte at +0xB4)
constexpr int FnCanEquipInSlot = 0x005BD6A0;  // thiscall(creature; item, u32* slotBit, 0, 0, 0) RET 0x14 -> AL; params 4/6 = 0 => no feedback
constexpr int GenderReq1Val = 0x0099A8BC, GenderReq2Val = 0x0099A8B8;   // u16 constants 0x8ac330 compares stats+0xE0 with
constexpr int FnSetCanUse = 0x008B1D00;       // thiscall(row; byte mode) RET 4
constexpr int FnSetNextFilter = 0x008A8B30;   // thiscall(panel; int mode) RET 4
constexpr int FnGetUpgradeType = 0x006076D0;  // thiscall(item; int unused) RET 4, returns byte in AL
constexpr int FnExoCtor = 0x00733570, FnExoDtor = 0x00733780, FnTextSet = 0x00416E30;

constexpr int TextVtable = 0x009876B4;
constexpr int LabelTextOff = 0xD8, TextRendererOff = 0x14, TextStringSubOff = 0x18;

constexpr int PanelFilterLabelOff = 0x16B8;   // LBL_FILTER (static caption "Filter")
constexpr int PanelItemsListOff = 0x588;      // LB_ITEMS (inline list box)
constexpr uint32_t InventoryVtable = 0x009A72AC;   // CSWGuiInGameInventory vftable, set by the ctor 0x008A6170 (008a61bd MOV [ECX],0x9a72ac)
constexpr int PanelRowArrayOff = 0x24B0, PanelRowCountOff = 0x24B4, PanelSelectedIdOff = 0x24C8;
constexpr int ItemIdOff = 0x14, ItemFlagsOff = 0x2C8, ItemBaseOff = 0x0C, ItemUpgradeSlotsOff = 0x2F4, ItemNameOff = 0x2C0;
constexpr int BaseSlotMaskOff = 0x04, BaseGenderReqOff = 0xB4, CrStatsOff = 0x1198, StatsGenderOff = 0xE0;
constexpr int TabClickedOff = 0x5C;
constexpr int NoSelection = 0x7F000000;

// PopulateItemListBox 0x8a7ba0 frame (EBP-relative)
constexpr int F_Panel = -0x84, F_Item = -0x18, F_Cobj = -0x38, F_Counter = -0x3C, F_List = -0x2C,
              F_Loop = -0x14, F_SelRow = -0x20, F_Creature = -0x40;
// row-select handler 0x8a83d0 frame: [ebp+8] = row
constexpr int F_SelectRowArg = 8, F_SelectPanel = -0x18;   // [ebp-0x18] = handler ECX (0x8a83d6)

using CExoCtor = void*(__thiscall*)(void*, const char*);
using CExoDtor = void(__thiscall*)(void*);
using TextSetFn = void(__thiscall*)(void*, void*);
using GetGuiFn = int(__thiscall*)(int);
using CreateEntryFn = void(__thiscall*)(int, int, int*, void*, int, int, int);
using IsUsableFn = int(__thiscall*)(int, int, int, int);
using SetCanUseFn = void(__thiscall*)(int, int);
using SetNextFilterFn = void(__thiscall*)(int, int);
using GetUpgradeTypeFn = unsigned(__thiscall*)(int, int);
using GetBaseItemFn = int(__thiscall*)(int);
using GetLocStringFn = bool(__thiscall*)(void*, int, void*, unsigned char);   // CExoLocString::GetString(this, language, CExoString* out, gender) RET 0xC
using GetClientLanguageFn = int(__fastcall*)(int);                            // ECX = *(*(GuiGlobal)+4)
using CanEquipFn = unsigned char(__thiscall*)(int, int, unsigned*, int, int, int);

// ---- log ----
// KpLog core (KPatchInit): inventory-sort-filter_log.txt, level from kplog.ini. Before KPatchInit and on old cores: the fallback file below,
// appended; sessions split on the "inventory-sort-filter <ver> loaded" line. At attach the previous session moves to inventory_sort_filter_log_<stamp>.txt (rotateFallbackLog; newest 5 kept). logf forwards to the KpLog fallback sink (own line cap).
KP_CHANNEL(chMain, "main");

// Only stores the path and the level (no I/O): safe from a static initialiser. Same file name as before (tooling reads it).
const bool g_fallbackSet = (KpLog_SetFallbackFile("inventory_sort_filter_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

void logAt(int lvl, const char* fmt, va_list ap) {
    if (!KP_ENABLED(chMain, lvl)) return;   // kplog.ini [inventory-sort-filter] level (Debug logging row) decides; no format cost when off
    char buf[480];
    vsnprintf(buf, sizeof buf, fmt, ap);
    KP_LOGV(chMain, lvl, "%s", buf);
}
// INFO (logged even at Debug logging Off): startup, config, page, errors, STRIP click, row counter anomalies, "tab click: cycle ->" lines. DEBUG (Debug logging On): per-populate / tab-click / flush lines. TRACE (Verbose): per-item lines.
void logf(const char* fmt, ...) { va_list ap; va_start(ap, fmt); logAt(KPLOG_INFO, fmt, ap); va_end(ap); }
void logd(const char* fmt, ...) { va_list ap; va_start(ap, fmt); logAt(KPLOG_DEBUG, fmt, ap); va_end(ap); }
void logt(const char* fmt, ...) { va_list ap; va_start(ap, fmt); logAt(KPLOG_TRACE, fmt, ap); va_end(ap); }

template <typename T> T rd(int base, int off, T def = T()) {
    T v = def;
    saferead::readAt<T>(1, static_cast<uintptr_t>(base), off, &v);
    return v;
}
bool wr32(int addr, int v) {
    if (!saferead::probeRead(2, reinterpret_cast<const void*>(addr), 4)) return false;
    *reinterpret_cast<int*>(addr) = v;
    return true;
}

// ---- config ----
struct Config {
    bool sort = false, tint = false, cycle = false;
    unsigned mask = 0;
    isf::SortOrder order = isf::SO_Category;   // [Inventory] sort_order (WI-154); vanilla also forces sort off
    bool compare = false;                      // [Inventory] compare (S10): equip-compare band; labels built at PANEL_BUILD, live gate at each select
    bool bannerFilter = true;                  // [Inventory] banner_shows_filter (P-A): the top caption names the active filter even while the strip is live
    bool strip = true;                         // [Inventory] strip (VU4): own strip above LB_ITEMS; read at PANEL_BUILD, applies at the next open
};
Config g_cfg;
DWORD g_lastIni = 0;
FILETIME g_iniWrite = {};   // INI last-write time at the last read; zero = missing or never read
char IniName[MAX_PATH + 40] = "inventory_sort_filter.ini";   // made absolute in DllMain: a bare name is searched in the Windows dir

// ---- optional Mod Configuration page (WI-155): one list row, sort_order, bound to [Inventory] sort_order of the INI ----
// Same adopter shape as companion-selfheal / companion-ai: lazy connect, no manifest `requires`, so without mod-config 0.2.0+ nothing is
// registered and the mod behaves as before. The row reads its start value from the INI when the page is built and writes each accepted change
// back (IniWrite.h); the mtime re-read in loadIni then picks it up at the next populate, no restart. ISF has no frame hook: KPatchInit tries
// once, then loadIni (called at the top of every hook) retries until registered; after that it is one flag check.
// LOAD ORDER: list mod-config before inventory-sort-filter. The retry only runs when the inventory is used, and the provider binds its
// entry list when Options is first built; if Options is opened before any inventory use and mod-config loads after this mod, the page
// misses that session and shows up the next time the game starts.
k2mc::ConfigClient g_mcCfg;
k2mc::MenuClient g_mcMenu;
k2pc::RegisterOnce g_mcReg;
mcdecl::Page g_page;
K2ModConfigEntry g_mcEntry{};
bool g_pageLoaded = false;

void mcLog(int, const char* msg) { logf("modconfig: %s", msg); }

// Start value of the row: the INI's sort_order as loadIni parses it (a hand-edited word shows too), not a digit-only read.
// Debug logging row: kplog.ini [inventory-sort-filter] level word <-> choice index (isf::readPageValue / writePageValue, SortOrderPage.h).
bool mcRead(const char* path, const char* section, const char* key, char* out, int cap) {
    return isf::readPageValue(mcdecl::fileRead, path, section, key, out, cap);
}
bool mcWrite(const char* path, const char* section, const char* key, const char* val) {
    return isf::writePageValue(mcdecl::fileWrite, path, section, key, val, mcdecl::fileRead);   // the reader merges a cycle_states bit into the list
}

int32_t mcOnChange(void*, int, const char* id, int32_t requested, int) {
    logf("PAGE row=%s requested=%d", id, static_cast<int>(requested));
    return requested;
}

// ---- live preview list on the page (doc 11 VU7a; combat-hotbar CH6 precedent) ----
// A mock backpack (IsfPreview.h kDemo) drawn from the page's CURRENT row values (g_page.get, not the INI), so a change redraws at once. Needs
// mod-config with the description split; an older provider shows the rows only. The labels are ours (AddCustom MUI_NOFREE): pvDestroy frees them
// through pvc::Canvas before the declarative page's own teardown (fills restored per label, PreviewCanvas.h).
pvc::Canvas g_cv;
void* g_pagePanel = nullptr;
bool g_pvOn = false, g_pvLogged = false;
unsigned g_pvFrame = 0;
DWORD g_pvStart = 0;
constexpr int kPvSplitPermille = 650;
constexpr const char* kPvSolid = "uibit_fill_2wt";   // solid white: the tint is the colour (= CombatHotbar kPvSolid)
constexpr float kPvBg[3] = {0.04f, 0.07f, 0.09f}, kPvStrip[3] = {0.08f, 0.16f, 0.18f}, kPvRow[3] = {0.07f, 0.12f, 0.14f}, kPvName[3] = {0.05f, 0.09f, 0.11f};
constexpr float kPvTintText[3] = {1.00f, 0.45f, 0.20f};   // red-orange "cannot use" (the live tint colour family)
float g_pvText[3] = {1.f, 1.f, 1.f};                      // the cloned label's own text colour, read at build
bool g_pvTextOk = false;                                  // getTextColor gate (GuiKit.h setTextColor)
signed char g_pvTinted[isfprev::kPoolSize];               // last text colour set per label: -1 unknown, 0 normal, 1 tinted
char g_pvTextCache[isfprev::kPoolSize][64];               // last text per label (setText only on change)

void* g_pvSolidImg = nullptr;
int g_pvSolidTries = 0;
void* pvSolid() {   // resolved lazily, a miss retried every 30th frame at most 20 times (CombatHotbar pvImage)
    if (!g_pvSolidImg && g_pvSolidTries < 20 && (g_pvFrame % 30) == 0) { ++g_pvSolidTries; g_pvSolidImg = guikit::imageForRetry(kPvSolid); }
    return g_pvSolidImg;
}

void pvText(int label, const char* t, bool tinted) {
    if (label < 0 || label >= isfprev::kPoolSize) return;
    if (strncmp(g_pvTextCache[label], t ? t : "", sizeof g_pvTextCache[label] - 1)) {
        snprintf(g_pvTextCache[label], sizeof g_pvTextCache[label], "%s", t ? t : "");
        g_cv.text(label, g_pvTextCache[label]);
    }
    const signed char want = tinted ? 1 : 0;
    if (g_pvTextOk && g_pvTinted[label] != want) {
        const float* c = tinted ? kPvTintText : g_pvText;
        if (guikit::setTextColor(g_cv.w[label].c, c[0], c[1], c[2])) g_pvTinted[label] = want;
    }
}

isfprev::Settings pvSettings() {
    isfprev::Settings s;
    const int so = g_page.get("sort_order");
    s.sortOn = so != isf::SO_Vanilla;
    s.order = so >= 0 && so <= 3 ? static_cast<isf::SortOrder>(so) : isf::SO_Category;
    s.tint = g_page.get("tint") != 0;
    s.cycle = g_page.get("cycle") != 0;
    s.strip = g_page.get("strip") != 0;
    s.bannerFilter = g_page.get("banner_filter") != 0;
    s.mask = (g_page.get("cyc_new") ? 1u << isf::S_New : 0u) | (g_page.get("cyc_usable") ? 1u << isf::S_Usable : 0u) |
             (g_page.get("cyc_upgrade") ? 1u << isf::S_Upgrade : 0u);
    s.tick = static_cast<int>((GetTickCount() - g_pvStart) / 33);   // ~30 ticks a second: the model's 90-tick step = 3 s
    return s;
}

void pvRefresh() {
    if (!g_mcMenu.ok() || !g_pagePanel) return;
    if (!g_pvOn) { g_cv.resync(); g_cv.hideAll(); return; }   // listed labels must stay hidden (mod-config shows every page item on entry)
    ++g_pvFrame;
    isfprev::Model m;
    isfprev::build(pvSettings(), &m);
    if (!g_cv.beginView(g_mcMenu.p, g_pagePanel, 0, 0, isfprev::kScreenW, isfprev::kScreenH)) { g_cv.hideAll(); return; }
    isfprev::Prim pr[isfprev::kPoolSize];
    const int n = isfprev::buildPrims(m, pr, isfprev::kPoolSize);
    void* solid = pvSolid();
    for (int i = 0; i < n; ++i) {
        const isfprev::Prim& p = pr[i];
        const pvc::Rect r = {p.x, p.y, p.w, p.h};
        switch (p.kind) {
            case isfprev::PK_BG: g_cv.put(p.label, r, solid, kPvBg, 1.f); break;
            case isfprev::PK_STRIP: g_cv.put(p.label, r, solid, kPvStrip, 1.f); break;
            case isfprev::PK_ROW: g_cv.put(p.label, r, solid, kPvRow, 1.f); break;
            case isfprev::PK_ICON: {
                float c[3];
                isfprev::categoryColor(p.row >= 0 ? m.rows[p.row].category : 6, c);
                g_cv.put(p.label, r, solid, c, 1.f);
                break;
            }
            default:   // text labels carry a fill like the combat-hotbar caption (a label's text sits on its own border)
                g_cv.put(p.label, r, solid, p.kind == isfprev::PK_NAME ? kPvName : p.kind == isfprev::PK_CAPTION ? kPvBg : kPvStrip, 1.f);
                pvText(p.label, p.text, p.tinted);
                break;
        }
    }
    g_cv.end();
    if (!g_pvLogged && g_pvFrame > 60) {
        g_pvLogged = true;
        logf("PREVIEW frame=%u prims=%d solid=%s shown=%d of %d line1=\"%s\"", g_pvFrame, n, solid ? "ok" : "missing", m.shown, m.offered, m.line1);
    }
}

// DeclHooks::onBuilt (Options ctor window, after the rows): the preview labels, then the description split.
void pvOnBuilt(void*, uint32_t page, void* panel) {
    g_pvOn = false; g_pagePanel = panel; g_pvFrame = 0; g_pvLogged = false; g_pvStart = GetTickCount();
    if (!g_pvSolidImg) g_pvSolidTries = 0;
    for (int i = 0; i < isfprev::kPoolSize; ++i) { g_pvTinted[i] = -1; g_pvTextCache[i][0] = 0; }
    if (!g_mcMenu.hasModal() || !g_mcMenu.hasDescSplit()) { logf("PREVIEW skipped: provider has no description split (older mod-config); the rows work as before"); return; }
    const K2MenuUiApi* ui = g_mcMenu.p;
    if (!g_cv.build(ui, panel, page, isfprev::kPoolSize)) { logf("PREVIEW skipped: label pool not complete (%d of %d listed)", g_cv.n, isfprev::kPoolSize); return; }
    g_pvTextOk = guikit::getTextColor(g_cv.w[isfprev::kLblCaption].c, g_pvText);
    for (int i : {isfprev::kLblStripText1, isfprev::kLblStripText2}) guikit::setAlign(g_cv.w[i].c, 9);
    for (int r = 0; r < isfprev::kMaxRows; ++r) guikit::setAlign(g_cv.w[isfprev::kLblRow0 + 3 * r + 2].c, 9);
    guikit::setAlign(g_cv.w[isfprev::kLblCaption].c, 18);
    if (!ui->SetDescSplit(page, kPvSplitPermille)) { logf("PREVIEW skipped: SetDescSplit(%u, %d) refused (listed labels stay hidden)", page, kPvSplitPermille); return; }
    g_pvOn = true;
    logf("PREVIEW BUILD ok: %d labels, split %d permille, textColor=%s", isfprev::kPoolSize, kPvSplitPermille, g_pvTextOk ? "ok" : "unreadable");
}

void pvOnTick(void*) { pvRefresh(); }

// Options dtor entry (mod-config has unlinked its controls by now): free the preview labels it listed MUI_NOFREE, then the page's own teardown.
void KPATCH_CALL pvDestroy(void* panel, void* user) {
    if (g_mcMenu.ok()) {
        const int freed = g_cv.destroy(g_mcMenu.p, g_page.page);
        if (freed) logf("PREVIEW DESTROY freed=%d", freed);
    } else {
        g_cv.reset();
    }
    g_pvOn = false; g_pagePanel = nullptr;
    mcdecl::Page::cbDestroy(panel, user);
}

void mcPageInit(const KPatchApi* api) {
    g_mcCfg.init(api);
    g_mcMenu.init(api);
    g_mcCfg.onConnect = [](bool ok, const char* why) { logf("PAGE k2.modconfig %s: %s", ok ? "ok" : "unavailable", why); };
    g_mcMenu.onConnect = [](bool ok, const char* why) { logf("PAGE k2.menuui %s: %s", ok ? "ok" : "unavailable", why); };
    char err[96] = {};
    g_pageLoaded = g_page.load(isf::kPageSchema, strlen(isf::kPageSchema), err, sizeof err);
    if (!g_pageLoaded) { logf("PAGE schema rejected: %s", err); return; }
    g_page.menu = &g_mcMenu;
    g_page.io = mcdecl::fileIo();
    g_page.io.read = mcRead;
    g_page.io.write = mcWrite;
    g_page.hooks.onChange = mcOnChange;
    g_page.hooks.onBuilt = pvOnBuilt;
    g_page.hooks.onTick = pvOnTick;
    g_mcEntry = g_page.entry(isf::kPageId);
    g_mcEntry.destroy = pvDestroy;   // frees the preview labels before the declarative layer's own teardown
}

// Connect lazily (each client retries on every 256th call), register once, only when the provider has option rows.
void mcPagePoll() {
    if (!g_pageLoaded || g_mcReg.registered) return;
    g_mcCfg.poll();
    g_mcMenu.poll();
    if (!g_mcCfg.ok() || !g_mcMenu.ok() || !g_mcMenu.hasRows()) return;
    k2pc::registerOnce(g_mcReg, g_mcCfg, g_mcEntry, mcLog);
}

void loadIni(bool force) {
    mcPagePoll();
    DWORD now = GetTickCount();
    if (!force && g_lastIni && now - g_lastIni < 1000) return;
    g_lastIni = now ? now : 1;
    // Each GetPrivateProfile* call opens and parses the file (slow under Wine), so skip the re-read when the
    // last-write time is unchanged. A missing file is read once (defaults), then skipped until it reappears.
    WIN32_FILE_ATTRIBUTE_DATA fa = {};
    bool present = GetFileAttributesExA(IniName, GetFileExInfoStandard, &fa) != 0;
    FILETIME zero = {};
    if (!force) {
        if (present && CompareFileTime(&fa.ftLastWriteTime, &g_iniWrite) == 0) return;
        if (!present && CompareFileTime(&g_iniWrite, &zero) == 0) return;
    }
    g_iniWrite = present ? fa.ftLastWriteTime : zero;   // stored before the reads so a mid-read edit is picked up next time
    Config c;
    c.sort = GetPrivateProfileIntA("Inventory", "sort", 0, IniName) != 0;
    char so[32] = {};
    GetPrivateProfileStringA("Inventory", "sort_order", "category", so, sizeof so, IniName);
    c.order = isf::parseSortOrder(so);
    if (c.order == isf::SO_Vanilla) c.sort = false;   // vanilla order = no sort, same as sort=0 (the buffer is then not even filled with names)
    c.tint = GetPrivateProfileIntA("Inventory", "tint", 0, IniName) != 0;
    c.cycle = GetPrivateProfileIntA("Inventory", "cycle", 0, IniName) != 0;
    char cs[128] = {};
    GetPrivateProfileStringA("Inventory", "cycle_states", "new,usable,upgrade", cs, sizeof cs, IniName);
    c.mask = isf::parseCycleStates(cs);
    c.strip = GetPrivateProfileIntA("Inventory", "strip", 1, IniName) != 0;
    c.bannerFilter = GetPrivateProfileIntA("Inventory", "banner_shows_filter", 1, IniName) != 0;
    c.compare = GetPrivateProfileIntA("Inventory", "compare", 1, IniName) != 0;
    if (c.sort != g_cfg.sort || c.tint != g_cfg.tint || c.cycle != g_cfg.cycle || c.mask != g_cfg.mask || c.order != g_cfg.order ||
        c.strip != g_cfg.strip || c.bannerFilter != g_cfg.bannerFilter || c.compare != g_cfg.compare || force)
        logf("config: sort=%d tint=%d cycle=%d cycle_mask=%x sort_order=%d strip=%d banner=%d compare=%d", c.sort, c.tint, c.cycle, c.mask,
             (int)c.order, c.strip, c.bannerFilter, c.compare);
    g_cfg = c;
}

// ---- engine wrappers ----
int guiInGame() {
    int holder = rd<int>(GuiGlobal, 0), h4 = rd<int>(holder, 4);
    return h4 ? reinterpret_cast<GetGuiFn>(FnGetGuiInGame)(h4) : 0;
}
int filterMode() {
    int g = guiInGame();
    return g ? rd<unsigned char>(g, FilterModeOff, 0xFF) : -1;   // 0xFF/-1 = unknown, never equals a cycle mode
}

// Client language for GetString, fetched once per populate by SortFilterFlush (WI-146). -1 when the app chain is unreadable.
int clientLanguage() {
    const int app = rd<int>(rd<int>(GuiGlobal, 0), 4);
    if (!app || rd<int>(app, 4) == 0) return -1;    // GetClientLanguage reads *(*(app+4)+0x148)
    return reinterpret_cast<GetClientLanguageFn>(gameaddr::GetClientLanguage)(app);
}

// Localised item name for the sort tie-break: CExoLocString at item+0x2C0 read through GetString (REA 20261015). Leaves out[0] = 0
// (the "unknown" name, so before() falls back to orig) for a null/unreadable handle, lang < 0, a false return or a null text pointer.
// Copies at most cap-1 chars. The engine out string is always freed with the CExoString dtor.
void readItemName(int item, int lang, char* out, int cap) {
    out[0] = 0;
    const int handle = item + ItemNameOff;
    if (!item || lang < 0 || rd<int>(handle, 0) == 0) return;   // GetString dereferences [handle+0] first, with no null check
    int exo[2] = {0, 0};                            // CExoString {char* p; uint32 len}
    const bool ok = reinterpret_cast<GetLocStringFn>(gameaddr::GetLocString)(reinterpret_cast<void*>(handle), lang, exo, 0);
    if (ok && exo[0]) {
        char tmp[256];
        if (saferead::readExo(4, reinterpret_cast<uintptr_t>(exo), tmp, sizeof tmp, 255, false)) {
            int k = 0;
            while (tmp[k] && k < cap - 1) { out[k] = tmp[k]; ++k; }
            out[k] = 0;
        }
    }
    if (exo[0]) reinterpret_cast<CExoDtor>(gameaddr::ExoDtor)(exo);
}

void setLabelText(int panel, const char* text) {
    int c = panel + PanelFilterLabelOff;
    int tx = c + LabelTextOff;
    int tvt = rd<int>(tx, 0), rend = rd<int>(tx, TextRendererOff);
    if (tvt != TextVtable || !rend) { logf("caption: text object bad (vt=%08x renderer=%08x), cannot set \"%s\"", tvt, rend, text); return; }
    char exo[16] = {0};
    reinterpret_cast<CExoCtor>(FnExoCtor)(exo, text);
    reinterpret_cast<TextSetFn>(FnTextSet)(reinterpret_cast<void*>(tx + TextStringSubOff), exo);
    reinterpret_cast<CExoDtor>(FnExoDtor)(exo);
}

// ---- state ----
isf::Extra g_extra = isf::X_None;  // our filter on top of engine mode 0: Usable or Upgradeable
int g_panel = 0;                   // panel instance the state belongs to
char g_caption[isf::kCaptionBytes] = "Filter";     // caption last written by us

// VU4 strip: labels cloned at K2HUD_PANEL_BUILD for one inventory panel instance. No destroy API: the panel list owns them, so a
// new inventory panel just replaces the handles. idx1/idx2 = their panel-list index; stripLive re-checks the list before every write.
// slot[0..3] = the clickable filter states (All, New, Can Equip, Upgradeable); line1 stays hidden. sortR = the clickable sort text on line2.
struct Strip { int panel; uintptr_t line1, line2, frame; int idx1, idx2; bool placed, warned; int replaced, lbX, lbY, lbW, lbH;
               uintptr_t slot[4]; int idxSlot[4]; striplay::Rect slotR[4]; striplay::Rect sortR; };
Strip g_strip = {};
int g_hoverPrev = -1;   // StripClick: region under the cursor on the previous frame (-1 none)
k2hud::Client g_hud;   // hud-host k2.hud@1; connect() in KPatchInit, poll() from the populate / tab hooks retries
bool g_stripStaleLogged = false;

bool stripLive(int panel) {
    if (!panel || g_strip.panel != panel || !g_strip.line1 || !g_strip.line2) return false;
    for (uintptr_t c : g_strip.slot) if (!c) return false;
    const int arr = rd<int>(panel, guikit::Panel_CtlArray), cnt = rd<int>(panel, guikit::Panel_CtlCount);
    if (!(arr && g_strip.idx2 < cnt && rd<int>(arr, 4 * g_strip.idx1) == static_cast<int>(g_strip.line1) &&
          rd<int>(arr, 4 * g_strip.idx2) == static_cast<int>(g_strip.line2))) return false;
    for (int i = 0; i < 4; ++i)
        if (g_strip.idxSlot[i] >= cnt || rd<int>(arr, 4 * g_strip.idxSlot[i]) != static_cast<int>(g_strip.slot[i])) return false;
    return true;
}

void captionTo(int panel, const char* text) {
    char banner[isf::kCaptionBytes];
    if (stripLive(panel)) {
        // v4b kept the top bar vanilla while the strip is live; the owner reversed it (11-03): the bar names the active filter. banner_shows_filter=0 restores v4b.
        isf::State st;
        if (g_cfg.bannerFilter && g_cfg.cycle && g_cfg.mask && isf::stateOf(filterMode(), g_extra, &st)) { isf::bannerOf(st, banner, sizeof banner); text = banner; }
        else text = "Filter";
    }
    if (!strcmp(g_caption, text)) return;
    setLabelText(panel, text);
    strncpy(g_caption, text, sizeof g_caption - 1); g_caption[sizeof g_caption - 1] = 0;
}

constexpr int BufCap = 1024;
isf::Entry g_buf[BufCap];
int g_n = 0;
bool g_inPop = false;
int g_lastIdx = -1;
// Populate identity (WI-148): the frame that owns the buffer, the engine row counter at populate start, and the counter
// seen by the last buffered row. A populate that never reached SortFilterFlush can leave rows behind; these catch it.
int g_popEbp = 0;
int g_popCounter0 = 0;
int g_lastCounter = 0;
bool g_selSet = false;
int g_selId = 0;
int g_seen = 0, g_dropped = 0, g_direct = 0;
LONGLONG g_bufTicks = 0, g_nameTicks = 0;   // QueryPerformanceCounter ticks: whole buffer phase (WI-145) / tied-row name reads at flush (WI-146)

// Adds construction-to-destruction ticks to g_bufTicks; one object covers every early return of SortFilterBufferRow.
struct BufTimer {
    LARGE_INTEGER t0;
    BufTimer() { QueryPerformanceCounter(&t0); }
    ~BufTimer() { LARGE_INTEGER t1; QueryPerformanceCounter(&t1); g_bufTicks += t1.QuadPart - t0.QuadPart; }
};

bool itemHasOpenSlot(int item) {
    unsigned type = reinterpret_cast<GetUpgradeTypeFn>(FnGetUpgradeType)(item, 0) & 0xFF;
    int slots[6] = {0, 0, 0, 0, 0, 0};
    int n = isf::upgradeSlotCount(static_cast<int>(type));
    for (int i = 0; i < n; ++i) slots[i] = rd<int>(item, ItemUpgradeSlotsOff + 4 * i, -1);   // unreadable = open (fail-open: show the item)
    return isf::hasOpenSlot(static_cast<int>(type), slots);
}

// Equip test of the equipment-slot list 0x8ac330 for the character the inventory shows: gender restriction, then
// 0x5BD6A0 for each slot bit of the baseitem mask (it wants one bit). *equippable = the item has any slot at all.
// *slotMaskOut (optional) = the baseitem slot mask, read even for an unknown creature (WI-154 category), 0 when there is no item.
bool canEquip(int creature, int item, bool* equippable, unsigned* slotMaskOut = nullptr) {
    *equippable = false;
    if (slotMaskOut) *slotMaskOut = 0;
    if (!item) return true;   // unknown: never tint, never hide
    int base = reinterpret_cast<GetBaseItemFn>(FnGetBaseItem)(item);
    unsigned mask = base ? rd<unsigned>(base, BaseSlotMaskOff) : 0;
    if (slotMaskOut) *slotMaskOut = mask;
    if (!creature) return true;
    if (!mask) return false;
    *equippable = true;
    int stats = rd<int>(creature, CrStatsOff);
    int req = rd<unsigned char>(base, BaseGenderReqOff);
    if (req && stats &&
        !isf::genderAllows(req, rd<unsigned short>(stats, StatsGenderOff), rd<unsigned short>(GenderReq1Val, 0), rd<unsigned short>(GenderReq2Val, 0)))
        return false;
    return isf::anySlotBit(mask, [&](unsigned bit) {
        unsigned slot = bit;   // the callee may rewrite it (0x20 -> 0x10 etc.); a fresh copy per bit
        return reinterpret_cast<CanEquipFn>(FnCanEquipInSlot)(creature, item, &slot, 0, 0, 0) != 0;
    });
}

// Rows tinted by the last populate, so the row-select hook can re-tint them after SetItem resets the colours.
// Pointers are only compared, never dereferenced; cleared whenever the backpack rows are rebuilt.
constexpr int TintCap = 1024;
int g_tinted[TintCap];
int g_nTinted = 0;
int g_tintPanel = 0;   // panel whose rows g_tinted refers to
bool isTinted(int row) {
    for (int i = 0; i < g_nTinted; ++i) if (g_tinted[i] == row) return true;
    return false;
}

// ---- equip-compare probe (EQ-P1; .claude/research/equip_compare/PLAN.md sec 9). READ-ONLY, DEBUG level only. ----
// Answers: is row+0x1D0 the item id, does the equip screen reach H6d, which panel dword holds the selection / shown creature,
// and what do GetItemInSlot(0x10) / (0x20) return. Never passes a multi-bit mask; never writes; capped at MaxLines.
namespace eqprobe {
constexpr int MapCap = 512, MaxLines = 150, MaxOther = 50, MaxOrder = 40, ItemIdAt = 0x14, RowItemIdOff = 0x1D0, InvOff = 0x1150, ScanEnd = 0x2000;
constexpr uintptr_t FnInvGetItemInSlot = gameaddr::InvGetItemInSlot;   // thiscall(inv, u32 oneSlotBit) RET 4 -> item or 0
struct RowItem { int row, item; };
RowItem g_map[MapCap];
int g_nMap = 0, g_cre = 0, g_lines = 0, g_linesOther = 0, g_checked = 0, g_mismatch = 0;   // separate budgets so inventory use cannot starve equip-screen lines
int g_h6dSeq = 0, g_orderLines = 0;   // EQP-ORDER: H6d call counter (every call, monotonic) and its own line budget (MaxOrder)

// createRow: remember row -> item and compare row+0x1D0 with the creation-path id (item+0x14). Pass signal: mismatch 0.
void note(int row, int item) {
    if (!KP_ENABLED(chMain, KPLOG_DEBUG) || !row || !item) return;
    if (g_nMap < MapCap) g_map[g_nMap++] = {row, item};
    int rid = rd<int>(row, RowItemIdOff), cid = rd<int>(item, ItemIdAt);
    ++g_checked;
    if (rid != cid && ++g_mismatch <= 3) logd("equip-probe createRow MISMATCH row=%08x row+1D0=%08x item=%08x item+14=%08x", row, rid, item, cid);
}
int itemOf(int row) { for (int i = 0; i < g_nMap; ++i) if (g_map[i].row == row) return g_map[i].item; return 0; }

// -1 = unreadable (creature/inventory), 0 = empty, else the item pointer.
int equippedIn(int cre, unsigned oneBit) {
    if (!cre) return -1;
    int inv = rd<int>(cre, InvOff);
    if (!inv || !saferead::probeRead(6, reinterpret_cast<const void*>(inv), 0x20)) return -1;
    return (int)reinterpret_cast<uintptr_t(__thiscall*)(int, unsigned)>(FnInvGetItemInSlot)(inv, oneBit);
}
void fmtSlot(int v, char* out, int cap) { if (v == -1) snprintf(out, cap, "unr"); else snprintf(out, cap, "%08x", v); }

// ---- 0.6.1-dev probe additions (logging only). Slot bits read: right 0x10, left 0x20, armor 0x2, head 0x1, gauntlet 0x8, implant 0x200, belt 0x400.
constexpr unsigned PmBits[7] = {0x10, 0x20, 0x2, 0x1, 0x8, 0x200, 0x400};
constexpr int PfCap = 4, PfLogMax = 6;
unsigned g_pfSeen[PfLogMax][2]; int g_nPf = 0;
// Appends "<prefix><hexbits>" for every equipped slot of `cre` whose pointer == ip; counts non-null reads in *nonNull.
void pmScan(int cre, int ip, const char* prefix, char* out, int cap, int* pos, int* nonNull, int* hits) {
    for (unsigned b : PmBits) {
        int v = equippedIn(cre, b);
        if (v == -1 || v == 0) continue;
        ++*nonNull;
        if (ip && v == ip && *pos < cap - 1) { *pos += snprintf(out + *pos, cap - *pos, "%s%s%x", *hits ? "," : "", prefix, b); ++*hits; }
        if (*pos >= cap) *pos = cap - 1;
    }
}
// One EQ-P1 T entry: "<tag>=<ptr>/<readFacts ok>/<baseitems idx>/<name 12 chars> ". Null ptr: 00000000/0/-1/-.
void tProbe(char* out, int cap, int* pos, const char* tag, uintptr_t p) {
    static itemcmp::ItemFacts f;   // large: static, never on the hook stack
    const bool ok = p && itemcmpread::readFacts(p, &f);
    const int bi = p ? rd<int>(static_cast<int>(p), itemcmpread::Item_BaseIdx, -1) : -1;
    if (*pos >= cap - 1) return;
    *pos += snprintf(out + *pos, cap - *pos, "%s=%08x/%d/%d/%.12s ", tag, static_cast<unsigned>(p), ok ? 1 : 0, bi, ok ? f.name : "-");
    if (*pos >= cap) *pos = cap - 1;
}
// EQ-P1 B: row->item via the engine object table (as the compare band does), pointer compare against equipped slots, screen tag, vtable, party fingerprint (ctx=other).
void probeB(int panel, int row, bool isf) {
    if (!KP_ENABLED(chMain, KPLOG_DEBUG)) return;
    const unsigned rawId = rd<unsigned>(row, RowItemIdOff);
    int ip = (int)itemcmpread::viewedFromRow(isf ? rawId : (rawId & 0x7FFFFFFFu));   // equip-screen ids carry bit 31 (ISF-CMPBOX-1 play log): mask it or ip is always 0 there
    char pm[64]; int pos = 0, nonNull = 0, hits = 0; pm[0] = 0;
    const unsigned vt = rd<unsigned>(panel, 0);
    uintptr_t tbl = isf ? 0 : party::table();
    const int pc = tbl ? party::partyCount(tbl) : 0;
    const int pn = pc < PfCap ? pc : PfCap;
    uintptr_t srv[PfCap] = {0, 0, 0, 0};
    for (int i = 0; i < pn; ++i) { uintptr_t m = party::member(tbl, i); srv[i] = m ? party::serverCreature(m) : 0; }
    if (isf) pmScan(g_cre, ip, "", pm, sizeof pm, &pos, &nonNull, &hits);
    else for (int i = 0; i < pn; ++i) if (srv[i]) { char pre[8]; snprintf(pre, sizeof pre, "%d:", i); pmScan((int)srv[i], ip, pre, pm, sizeof pm, &pos, &nonNull, &hits); }
    // Worst case: 8+15+13+13+12+3+63+6+10+12 = 155 bytes (< 232).
    logd("EQ-P1 B panel=%08x row=%08x sel=%08x ip=%08x pm=%s pmN=%d scr=%s vt=%08x", panel, row, (unsigned)rd<int>(panel, PanelSelectedIdOff), ip,
         hits ? pm : "none", nonNull, isf ? "inv" : "other", vt);
    // EQ-P1 T (one line per select): bit-31 row-id transforms -> viewedFromRow -> readFacts -> base idx + name, plus the creation-path map (itemOf).
    // t3 equals t1 whenever bit 31 is set (the equip-screen case); it differs only for a row without bit 31, kept as a control.
    // Budget: 4 entries <= 4*40 = 160 bytes + "EQ-P1 T raw=XXXXXXXX " 21 = 181 (< 232).
    char tl[200]; int tp = 0; tl[0] = 0;
    tProbe(tl, sizeof tl, &tp, "t1", itemcmpread::viewedFromRow(rawId & 0x7FFFFFFFu));
    tProbe(tl, sizeof tl, &tp, "t2", itemcmpread::viewedFromRow(rawId & 0xFFFFu));
    tProbe(tl, sizeof tl, &tp, "t3", itemcmpread::viewedFromRow(rawId ^ 0x80000000u));
    tProbe(tl, sizeof tl, &tp, "t4", static_cast<uintptr_t>(eqprobe::itemOf(row)));
    logd("EQ-P1 T raw=%08x %s", rawId, tl);
    if (isf || pn <= 0 || g_nPf >= PfLogMax) return;
    unsigned h = 0; for (int i = 0; i < pn; ++i) h = h * 31u + (unsigned)srv[i];
    for (int i = 0; i < g_nPf; ++i) if (g_pfSeen[i][0] == (unsigned)panel && g_pfSeen[i][1] == h) return;
    g_pfSeen[g_nPf][0] = (unsigned)panel; g_pfSeen[g_nPf][1] = h; ++g_nPf;
    char pf[200]; int pp = 0; pf[0] = 0;   // 4 full entries = 151 bytes + NUL; line stays < 232
    for (int i = 0; i < pn && pp < (int)sizeof pf - 40; ++i) {
        char a[16], b[16], c[16];
        if (srv[i]) { fmtSlot(equippedIn((int)srv[i], 0x10), a, sizeof a); fmtSlot(equippedIn((int)srv[i], 0x20), b, sizeof b); fmtSlot(equippedIn((int)srv[i], 0x2), c, sizeof c); }
        else { snprintf(a, sizeof a, "unr"); snprintf(b, sizeof b, "unr"); snprintf(c, sizeof c, "unr"); }
        pp += snprintf(pf + pp, sizeof pf - pp, "%s%d:%08x:%s/%s/%s", i ? " " : "", i, (unsigned)srv[i], a, b, c);
    }
    // Worst case: "EQ-P1 PF panel=XXXXXXXX n=4 pf=" 31 + 4*38+3 = 186 bytes (< 232).
    logd("EQ-P1 PF panel=%08x n=%d pf=%s", panel, pn, pf);
}

// EQ-P2 (equip screen only, logging only): the panel's creature field and what changed in the panel object since the previous equip select.
// Click a slot, then an item, then another slot: the offsets whose value follows the slot are the "edited slot" field (EQBOX_PLAN B3/B4).
// Budget: 10 diff entries x <= 19 bytes + "EQ-P2 diff n=10" 15 = 205 (< 232).
void probeP2(int panel) {
    static unsigned snap[ScanEnd / 4]; static bool have = false; static int nLines = 0;
    if (nLines >= 60) return;
    ++nLines;
    int c68i = 0, c2d0i = 0;
    saferead::readAt<int>(5, static_cast<uintptr_t>(panel), 0x68, &c68i); saferead::readAt<int>(5, static_cast<uintptr_t>(panel), 0x2D0, &c2d0i);
    const unsigned c68 = static_cast<unsigned>(c68i), c2d0 = static_cast<unsigned>(c2d0i);
    unsigned v68 = 0, v2d0 = 0;
    if (c68 > 0x10000 && saferead::probeRead(7, reinterpret_cast<const void*>(c68), 8)) v68 = rd<unsigned>(static_cast<int>(c68), 0);
    if (c2d0 > 0x10000 && saferead::probeRead(7, reinterpret_cast<const void*>(c2d0), 8)) v2d0 = rd<unsigned>(static_cast<int>(c2d0), 0);
    logd("EQ-P2 panel=%08x +68=%08x(vt %08x) +2d0=%08x(vt %08x) g_cre=%08x", panel, c68, v68, c2d0, v2d0, g_cre);
    {   // ISF-EQBOX-2 u03: edited-slot mask at +0x5098 and per-slot item ids at +0x509c + 4*i (i<11), beyond ScanEnd. Budget: 40 + 11*9 = 139 bytes.
        int sm = 0; char sl[110]; int sp = 0; sl[0] = 0;
        const bool ok = saferead::readAt<int>(5, static_cast<uintptr_t>(panel), 0x5098, &sm);
        for (int i = 0; ok && i < 11; ++i) {
            int v = 0;
            if (!saferead::readAt<int>(5, static_cast<uintptr_t>(panel), 0x509c + 4 * i, &v)) break;
            sp += snprintf(sl + sp, sizeof sl - sp, " %x", static_cast<unsigned>(v));
        }
        logd("EQ-P2 slot ok=%d mask=%x items[11]=%s", ok ? 1 : 0, static_cast<unsigned>(sm), sl);
    }
    {   // EQ-P3: is panel+0x68 the client creature (vtable 0x0099EE14) and which party member (by pointer) is it? Budget: ~40 + 4*34 = 176 bytes.
        const uintptr_t tbl = party::table();
        const int pcnt = tbl ? party::partyCount(tbl) : 0;
        char pl[150]; int pp = 0; pl[0] = 0;
        int hit = -1;
        for (int i = 0; i < pcnt && i < PfCap; ++i) {
            const uintptr_t m = party::member(tbl, i);
            const uintptr_t sc = m ? party::serverCreature(m) : 0;
            if (m == c68 || sc == c68) hit = i;
            pp += snprintf(pl + pp, sizeof pl - pp, " %d:m%x/s%x", i, static_cast<unsigned>(m), static_cast<unsigned>(sc));
        }
        logd("EQ-P3 c68vt=%08x isClient=%d partyN=%d match=%d%s", v68, v68 == static_cast<unsigned>(gameaddr::ClientCreatureVtable) ? 1 : 0, pcnt, hit, pl);
    }
    char line[236]; int p = 0, n = 0; line[0] = 0;
    for (int off = 0; off < ScanEnd; off += 4) {
        int v = 0;
        if (!saferead::readAt<int>(5, static_cast<uintptr_t>(panel), off, &v)) continue;
        unsigned& o = snap[off / 4];
        if (have && o != static_cast<unsigned>(v) && n < 10) { p += snprintf(line + p, sizeof line - p, " %x:%x>%x", off, o, static_cast<unsigned>(v)); ++n; }
        o = static_cast<unsigned>(v);
    }
    if (have) logd("EQ-P2 diff n=%d%s", n, line);
    have = true;
}
void onSelect(int ebp) {
    ++g_h6dSeq;   // counts every H6d call, logging on or off, so seq stays monotonic
    if (!KP_ENABLED(chMain, KPLOG_DEBUG)) return;
    if (g_orderLines < MaxOrder) { ++g_orderLines; logf("EQP-ORDER seq=%d h6d", g_h6dSeq); }
    int row = rd<int>(ebp, F_SelectRowArg), panel = rd<int>(ebp, F_SelectPanel);
    if (!row || !panel) return;
    int rowid = rd<int>(row, RowItemIdOff);
    if (!rowid || rowid == NoSelection) return;
    const bool isf = rd<unsigned>(panel, 0) == InventoryVtable;
    if (isf ? g_lines >= MaxLines : g_linesOther >= MaxOther) return;
    ++(isf ? g_lines : g_linesOther);
    // g_map / g_cre belong to the last inventory populate; on the equip screen they may be stale (freed items), so only compare values there.
    int item = isf ? itemOf(row) : 0, createid = item ? rd<int>(item, ItemIdAt) : 0;
    char name[96] = "-"; unsigned slotmask = 0; int base = 0;
    if (item && saferead::probeRead(6, reinterpret_cast<const void*>(item), 0x300)) {
        char nm[96]; readItemName(item, clientLanguage(), nm, sizeof nm);
        if (nm[0]) snprintf(name, sizeof name, "%s", nm);
        base = reinterpret_cast<GetBaseItemFn>(FnGetBaseItem)(item);
        slotmask = base ? rd<unsigned>(base, BaseSlotMaskOff) : 0;
    }
    char r10[16], r20[16];
    if (isf) { fmtSlot(equippedIn(g_cre, 0x10), r10, sizeof r10); fmtSlot(equippedIn(g_cre, 0x20), r20, sizeof r20); }
    else { snprintf(r10, sizeof r10, "skip"); snprintf(r20, sizeof r20, "skip"); }
    // Panel dword scan (equip screen: where is the selection id / the shown creature). Bounded, read-only.
    char scan[96] = "none", credoff[48] = "-"; int ns = 0, sp = 0, nc = 0, cp = 0;
    scan[0] = 0; credoff[0] = 0;
    for (int off = 0; off < ScanEnd; off += 4) {
        int v = 0;
        if (!saferead::readAt<int>(5, static_cast<uintptr_t>(panel), off, &v)) continue;
        if (v == rowid && ns < 8) { sp += snprintf(scan + sp, sizeof scan - sp, "%s%x", ns ? ":" : "", off); ++ns; }
        if (g_cre && v == g_cre && nc < 8) { cp += snprintf(credoff + cp, sizeof credoff - cp, "%s%x", nc ? ":" : "", off); ++nc; }
    }
    // Line A is split in two so each stays well under the 232-byte KpLog cap. A1 keeps mm= (pass signal); A2 is logged right
    // after A1 in this same call, so it always follows it. A2 name is cut at 32 chars (worst A2 ~196 bytes).
    logd("equip-probe ctx=%s panel=%08x sel=%08x rowid=%08x createid=%08x base=%d slotmask=%x mm=%d/%d",
         isf ? "isf" : "other", panel, rd<int>(panel, PanelSelectedIdOff), rowid, createid, base ? rd<int>(base, 0) : -1, slotmask,
         g_mismatch, g_checked);
    logd("equip-probe A2 name=\"%.32s\" equippedCre=%08x r10=%s r20=%s scan=%s credoff=%s",
         name, g_cre, r10, r20, ns ? scan : "none", nc ? credoff : "none");
    probeB(panel, row, isf);
    if (!isf) probeP2(panel);
}
}  // namespace eqprobe

// Creates one backpack row exactly like the vanilla call site, then tints it when the shown character cannot equip it.
void createRow(int panel, int cobj, int ebp, int item, bool tint) {
    int* counter = reinterpret_cast<int*>(ebp + F_Counter);
    int before = *counter;
    reinterpret_cast<CreateEntryFn>(FnCreateItemEntry)(panel, cobj, counter, reinterpret_cast<void*>(ebp + F_List), item, 0, 0);
    int after = *counter;
    if (after != before + 1) logf("row: counter %d -> %d (expected +1) for item %08x", before, after, item);
    if (after == before + 1 && KP_ENABLED(chMain, KPLOG_DEBUG)) {   // equip-compare probe: row -> item map + row+0x1D0 self-check
        int a2 = rd<int>(panel, PanelRowArrayOff), c2 = rd<int>(panel, PanelRowCountOff);
        eqprobe::note((a2 && before >= 0 && before < c2) ? rd<int>(a2, 4 * before) : 0, item);
    }
    if (tint && g_cfg.tint && after == before + 1) {
        int arr = rd<int>(panel, PanelRowArrayOff), cnt = rd<int>(panel, PanelRowCountOff);
        int row = (arr && before >= 0 && before < cnt) ? rd<int>(arr, 4 * before) : 0;
        if (row) {
            reinterpret_cast<SetCanUseFn>(FnSetCanUse)(row, 2);
            if (g_nTinted < TintCap) g_tinted[g_nTinted++] = row;
            g_tintPanel = panel;
        } else logf("row: tint skipped, no row at index %d (arr=%08x cnt=%d)", before, arr, cnt);
    }
}

// Every write to the engine filter mode goes through here (ISF 0.7.1 sec 7). why = bumper | strip-sort | strip-state | tab-cycle.
// The only SetNextFilter cast in this file.
void setFilterMode(int panel, int mode, const char* why) {
    logf("mode set: %s panel=%08x mode %d -> %d extra=%d", why, panel, filterMode(), mode, g_extra);
    reinterpret_cast<SetNextFilterFn>(FnSetNextFilter)(panel, mode);
}

// ISF 0.7.1 sec 4: strip slot i (0 All, 1 New, 2 Usable, 3 Upgradeable) is enabled. One test for the click gate (onStripFrame)
// and the grey paint (stripRefresh). Cycle off disables every slot.
bool stripSlotEnabled(int i, int mode) {
    if (!g_cfg.cycle) return false;
    const unsigned m = g_cfg.mask;
    switch (i) {
    case 0: return true;
    case 1: return (m & (1u << isf::S_New)) != 0;
    case 2: return (m & (1u << isf::S_Usable)) != 0 && isf::subFilterAllowed(mode, isf::S_Usable);
    case 3: return (m & (1u << isf::S_Upgrade)) != 0 && isf::subFilterAllowed(mode, isf::S_Upgrade);
    default: return false;
    }
}

// Rebuilds the extra filter/caption from the engine mode at every populate, so changes that bypass our tab handler
// (controller bumpers -> SetNextFilter, cycle switched off in the INI) cannot leave stale state. The engine keeps
// the filter mode across panel close/reopen and party switches (S0 run 2), and so do we: while the mode is not New
// (All or a category tab) the extra filter is kept and its caption re-applied.
void normaliseState(int panel) {
    int mode = filterMode();
    if (g_extra == isf::X_New && ((mode < 6 || mode > 11) || !g_cfg.cycle)) g_extra = isf::X_None;   // X_New is the New sub-filter of a category tab (modes 6..11 only)
    if ((g_extra == isf::X_Usable || g_extra == isf::X_Upgrade) && (mode == isf::kEngineModeNew || !g_cfg.cycle)) g_extra = isf::X_None;   // New is engine-native; category tabs keep the extra filter (stacking)
    if (g_extra != isf::X_None) {   // ISF 0.7.1 sec 6: a stacked filter the current category does not allow is dropped
        isf::State gs;
        if (isf::stateOf(mode, g_extra, &gs) && !isf::subFilterAllowed(mode, gs)) {
            logf("extra cleared: %s not allowed on mode %d", isf::stripNameOf(gs), mode);
            g_extra = isf::X_None;
        }
    }
    if (g_cfg.cycle) {
        isf::State st;
        char text[isf::kCaptionBytes] = "Filter";
        if (isf::stateOf(mode, g_extra, &st)) isf::captionWithCount(st, g_cfg.mask, -1, text, sizeof text);
        captionTo(panel, text);
    } else if (strcmp(g_caption, "Filter")) {
        captionTo(panel, "Filter");
    }
}

// Creates the buffered rows in their current order and empties the buffer.
void emitBuffered(int panel, int cobj, int ebp) {
    for (int i = 0; i < g_n; ++i) {
        const isf::Entry& e = g_buf[i];
        if (g_selSet && e.id == g_selId) wr32(ebp + F_SelRow, rd<int>(ebp, F_Counter));
        createRow(panel, cobj, ebp, e.item, e.tint);
    }
    g_n = 0;
}

// WI-151: a filtered state shows its visible-row count in the caption. Call it only where g_n is final: before emitBuffered
// (which empties the buffer) in SortFilterFlush, and on the !g_inPop path. g_n is 0 there, which is the empty-filter case:
// no row reached the hook, so no populate started. normaliseState wrote the plain caption just before; this replaces it in
// the same frame, so there is no visible flicker.
void captionCountTo(int panel) {
    if (!g_cfg.cycle) return;
    isf::State st;
    if (!isf::stateOf(filterMode(), g_extra, &st) || st == isf::S_All) return;
    char text[isf::kCaptionBytes];
    isf::captionWithCount(st, g_cfg.mask, g_n, text, sizeof text);
    captionTo(panel, text);
}

// Places LB_ITEMS one strip lower and the strip labels in the freed band. The strip height is the 3840x1600 layout's 133 of 877 px,
// scaled with the list height. Runs at PANEL_BUILD and again at each refresh until the list has a usable extent; re-applies (logged
// once) when something else moved the list back.
void stripPlace(int panel) {
    const uintptr_t lb = static_cast<uintptr_t>(panel + PanelItemsListOff);
    int x = 0, y = 0, w = 0, h = 0;
    if (!guikit::getExtent(lb, &x, &y, &w, &h)) { if (!g_strip.warned) logf("STRIP: WARN LB_ITEMS extent unreadable"); g_strip.warned = true; return; }
    if (g_strip.placed) {
        const int sh = g_strip.lbH * 133 / 877;
        if (x == g_strip.lbX && y == g_strip.lbY + sh && w == g_strip.lbW && h == g_strip.lbH - sh) return;
        if (++g_strip.replaced > 3) return;   // something keeps resizing the list: stop fighting it (logged 3 times)
        logf("STRIP: WARN LB_ITEMS moved to %d,%d %dx%d after placement, re-applying (%d/3)", x, y, w, h, g_strip.replaced);
        g_strip.placed = false;
    }
    g_strip.lbX = x; g_strip.lbY = y; g_strip.lbW = w; g_strip.lbH = h;
    if (w < 200 || h < 300) {   // not laid out yet (retried at each refresh) or an unexpected layout
        if (!g_strip.warned) logf("STRIP: WARN LB_ITEMS extent %d,%d %dx%d too small, list left as is", x, y, w, h);
        g_strip.warned = true;
        return;
    }
    const int sh = h * 133 / 877, in = h * 16 / 877, lh = h * 48 / 877;
    if (!guikit::setRect(lb, x, y + sh, w, h - sh)) { logf("STRIP: WARN LB_ITEMS setRect failed"); return; }
    int ax = 0, ay = 0, aw = 0, ah = 0;
    guikit::getExtent(lb, &ax, &ay, &aw, &ah);
    logd("STRIP: LB_ITEMS %d,%d %dx%d -> %d,%d %dx%d (strip %d px)", x, y, w, h, ax, ay, aw, ah, sh);
    if (g_strip.frame) { guikit::setRect(g_strip.frame, x, y, w, sh); guikit::setVisible(g_strip.frame, true); guikit::setClickable(g_strip.frame, false); }
    guikit::setRect(g_strip.line1, x + in, y + h * 14 / 877, w - 2 * in, lh);
    guikit::setRect(g_strip.line2, x + in, y + h * 70 / 877, w - 2 * in, lh);
    const striplay::Rect l1{x + in, y + h * 14 / 877, w - 2 * in, lh};
    striplay::slotRects(l1, g_strip.slotR);
    g_strip.sortR = striplay::sortRect(striplay::Rect{x + in, y + h * 70 / 877, w - 2 * in, lh});
    for (int i = 0; i < 4; ++i) {
        guikit::setRect(g_strip.slot[i], g_strip.slotR[i].x, g_strip.slotR[i].y, g_strip.slotR[i].w, g_strip.slotR[i].h);
        guikit::setVisible(g_strip.slot[i], true); guikit::setClickable(g_strip.slot[i], false);
    }
    for (uintptr_t c : {g_strip.line1, g_strip.line2}) { guikit::setVisible(c, c == g_strip.line2); guikit::setClickable(c, false); }
    g_strip.placed = true;
}

// Selected state is bright, the rest dim (no brackets). Disabled (not allowed here, ISF 0.7.1 sec 5) is grey and wins over selected.
// getTextColor readback gates setTextColor (GuiKit.h:297).
void paintSlot(uintptr_t c, bool selected, bool disabled) {
    float rgb[3] = {};
    if (!c || !guikit::getTextColor(c, rgb)) return;
    if (disabled) guikit::setTextColor(c, 0.32f, 0.32f, 0.32f);
    else if (selected) guikit::setTextColor(c, 0.55f, 1.0f, 0.80f);
    else guikit::setTextColor(c, 0.20f, 0.50f, 0.38f);
}

// Strip text for this populate: shown = rows that reached the list, offered = backpack rows the engine handed us.
void stripRefresh(int panel, int shown, int offered) {
    if (!stripLive(panel)) {
        if (g_strip.panel && g_strip.panel != panel && !g_stripStaleLogged) {
            g_stripStaleLogged = true;
            logf("STRIP: populate on panel %08x, strip belongs to %08x: not touched", panel, g_strip.panel);
        }
        return;
    }
    stripPlace(panel);
    const int mode = filterMode();
    isf::State st;
    if (!isf::stateOf(mode, g_extra, &st)) st = isf::S_Count;   // unreadable mode: no slot selected (category tabs select their stacked state)
    char l1[isf::kStripBytes], l2[isf::kStripBytes];
    isf::stripLine1(st, g_cfg.cycle ? g_cfg.mask : 0u, l1, sizeof l1);
    isf::stripLine2(g_cfg.order, g_cfg.sort, shown, offered, l2, sizeof l2);
    guikit::setText(g_strip.line1, l1);   // hidden: the four slots below carry the states
    for (int i = 0; i < 4; ++i) {
        guikit::setText(g_strip.slot[i], striplay::slotLabel(i));
        paintSlot(g_strip.slot[i], static_cast<int>(st) == i, i != 0 && !stripSlotEnabled(i, mode));   // All stays bright (user choice, ISF-CK5)
    }
    guikit::setText(g_strip.line2, l2);
    logd("STRIP: \"%s\" | \"%s\"", l1, l2);
}

// Hidden, click-through clone of a layout label, appended to the panel list; *idx = its list index. 0 on failure (why logged at WARN).
uintptr_t stripLabel(uintptr_t panel, const char* tag, const char* what, int* idx, bool required) {
    int why = guikit::CF_None;
    uint32_t bv = 0;
    const uintptr_t c = guikit::createLabelFromTemplate(panel, tag, &why, &bv);
    if (!c) { logf("STRIP: %s %s label (%s) create failed why=%d vtable=%08x", required ? "WARN" : "no", what, tag, why, (unsigned)bv); return 0; }
    guikit::setVisible(c, false);
    guikit::setClickable(c, false);
    const int before = rd<int>(static_cast<int>(panel), guikit::Panel_CtlCount, -1);
    if (!guikit::appendToPanel(panel, c)) { logf("STRIP: WARN %s label append failed (control %08x left hidden)", what, (unsigned)c); return 0; }
    if (idx) *idx = before;
    return c;
}

// ---- equip-compare box (S10 box; replaces band A; .claude/waves/ISF-CMPBOX-1/) ----
// A framed box carved out of the bottom 35% of LB_DESCRIPTION: header + up to 5 rows (name | value labels, colour by kind), built from
// label clones at PANEL_BUILD. While shown, the description list is shrunk with guikit::setRect (readback logged, re-applied if something
// moves it back, restored when the box hides). Creature = the populate-time creature (eqprobe::g_cre), cleared on every new panel;
// unknown creature hides the box. compare=0: nothing is created or touched. Reads only (itemcmpread); the writes are our own labels
// and the description list's rect.
constexpr int CmpRows = 6;                 // block lines incl. the header (itemcmp::kBlockMaxLines)
constexpr int CmpDescIdx = 20, PanelDescListOff = 0x878, ListClientW = 0x2A4, ListClientH = 0x2A8;   // inventory_p.gui LB_DESCRIPTION = control #20 (= panel list index); 01_inventory_3d_viewport.md member +0x878
struct CmpBox {
    int panel;
    uintptr_t lb; int lbIdx;               // LB_DESCRIPTION (0 until found); lbIdx = its panel-list index, -1 when found via +0x878
    uintptr_t frame; int frameIdx;
    uintptr_t hdr; int hdrIdx;               // equipped-item name, right end of row 1 (optional: without it the header keeps its own line)
    uintptr_t credCap; int credCapIdx;       // 'Credits' caption clone (the vanilla label becomes the frame around caption + number)
    uintptr_t name[CmpRows], val[CmpRows]; int nameIdx[CmpRows], valIdx[CmpRows];
    int lbX, lbY, lbW, lbH;                // original description extent (engine px), valid while have
    int boxX, boxY, boxW, boxH;            // computed box rect
    bool have, shrunk; int reapplied;
    int n, rowP, listH;                    // rows drawn, row pitch (engine px), shrunk list height for the current box
    bool bgFill;                           // equip: hdr label is a full-size background (the frame art leaves an inset gap)
    bool credMoved;                        // credits pair moved into the gap right of Use Item: box bottom can sit at y=470 gui units (mode B)
};
CmpBox g_cmp = {};
CmpBox g_cmpOther = {};   // the other panel's box (equip + inventory each build one; panels persist, so a single global lost the first one: play 2026-11-09 19:36, equip select said cmpLive false)
eqbox::State g_eqState = eqbox::initialState();
cband::Key g_cmpKey;
bool g_cmpKeyValid = false, g_cmpShown = false;
// Per-category log budgets so chatty sel/rects/hide lines cannot starve the one-per-populate creature lines.
enum CmpLogCat { LogRects, LogSel, LogCreature, LogHide, LogMismatch, LogEmpty, LogSame, LogFacts, LogHit, LogProps, LogProbe, LogCats };   // LogEmpty: sel lines with nBand==0, so a full LogSel budget cannot hide the failing select
int g_cmpLogLines[LogCats] = {};
constexpr int CmpLogCap = 400, PanelCreatureOff = 0x2D0;   // panel+0x2D0 = creature (EQ-P1 credoff): only compared and logged, never trusted

bool cmpLogOk(CmpLogCat c) { return g_cmpLogLines[c] < CmpLogCap && (++g_cmpLogLines[c], true); }

bool cmpIdxOk(int arr, int cnt, int idx, uintptr_t c) { return c && idx >= 0 && idx < cnt && rd<int>(arr, 4 * idx) == static_cast<int>(c); }

bool cmpLive(int panel) {
    if (!panel || g_cmp.panel != panel || !g_cmp.name[0] || !g_cmp.val[0]) return false;
    const int arr = rd<int>(panel, guikit::Panel_CtlArray), cnt = rd<int>(panel, guikit::Panel_CtlCount);
    if (!arr) return false;
    if (g_cmp.frame && !cmpIdxOk(arr, cnt, g_cmp.frameIdx, g_cmp.frame)) return false;
    if (g_cmp.hdr && !cmpIdxOk(arr, cnt, g_cmp.hdrIdx, g_cmp.hdr)) return false;
    for (int i = 0; i < CmpRows; ++i)
        if (!cmpIdxOk(arr, cnt, g_cmp.nameIdx[i], g_cmp.name[i]) || !cmpIdxOk(arr, cnt, g_cmp.valIdx[i], g_cmp.val[i])) return false;
    if (g_cmp.lb) {   // review r01 F2: the cached description list must still be the panel's (never write a freed control)
        const bool inArr = g_cmp.lbIdx >= 0 ? cmpIdxOk(arr, cnt, g_cmp.lbIdx, g_cmp.lb) : rd<int>(panel, PanelDescListOff) == static_cast<int>(g_cmp.lb);
        if (!inArr) return false;
    }
    return true;
}

bool cmpScreen(int* w, int* h) {
    *w = *h = 0;
    if (guikit::g_gk.screenWAddr) guikit::detail::rdInt(guikit::g_gk.screenWAddr, 0, w);
    if (guikit::g_gk.screenHAddr) guikit::detail::rdInt(guikit::g_gk.screenHAddr, 0, h);
    return *w >= 320 && *w <= 16384 && *h >= 200 && *h <= 16384;
}

// The description list: control #20, else panel+0x878; never LB_ITEMS. A shape gate (wide and tall) catches a wrong control.
bool cmpDescShape(int panel, uintptr_t c) {
    int x, y, w, h;
    return c && c != static_cast<uintptr_t>(panel + PanelItemsListOff) && guikit::getExtent(c, &x, &y, &w, &h) && w >= 200 && h >= 250;
}
uintptr_t cmpFindDesc(int panel, int* idx) {
    const int arr = rd<int>(panel, guikit::Panel_CtlArray), cnt = rd<int>(panel, guikit::Panel_CtlCount);
    const uint32_t vt = static_cast<uint32_t>(rd<int>(panel, 0));
    if (vt == static_cast<uint32_t>(gameaddr::EquipPanelVtable)) {
        if (arr && cnt > 0) {
            uintptr_t c0 = static_cast<uintptr_t>(rd<int>(arr, 0));
            int x = 0, y = 0, w = 0, h = 0;
            if (cmpDescShape(panel, c0) && guikit::getExtent(c0, &x, &y, &w, &h)) {
                int sw = 0, sh = 0;
                cmpScreen(&sw, &sh);
                const int gx250 = (sw > 0) ? (250 * sw / 800) : 250;
                if (x >= gx250 || (sh > 0 && x * 600 / sh >= 250)) {
                    *idx = 0;
                    return c0;
                }
            }
        }
    }
    uintptr_t c = (arr && CmpDescIdx < cnt) ? static_cast<uintptr_t>(rd<int>(arr, 4 * CmpDescIdx)) : 0;
    *idx = CmpDescIdx;
    if (!cmpDescShape(panel, c)) { c = static_cast<uintptr_t>(rd<int>(panel, PanelDescListOff)); *idx = -1; }
    return cmpDescShape(panel, c) ? c : 0;
}

void cmpLogList(const char* what, int panel) {
    int x = 0, y = 0, w = 0, h = 0;
    guikit::getExtent(g_cmp.lb, &x, &y, &w, &h);
    logd("ISF-CMP box %s list=%08x ext=%d,%d %dx%d clientW=%d clientH=%d (panel %08x)", what, (unsigned)g_cmp.lb, x, y, w, h, rd<int>(static_cast<int>(g_cmp.lb), ListClientW, -1), rd<int>(static_cast<int>(g_cmp.lb), ListClientH, -1), (unsigned)panel);
}

// Shrinks the description list (first time, or when something moved it back) and computes the box rect for `n` rows. False = box cannot be placed.
bool cmpShrink(int panel, int n) {
    if (!g_cmp.lb) g_cmp.lb = cmpFindDesc(panel, &g_cmp.lbIdx);
    if (!g_cmp.lb) { if (cmpLogOk(LogHide)) logd("ISF-CMP box: LB_DESCRIPTION not found (panel %08x), box off", (unsigned)panel); return false; }
    int x, y, w, h;
    if (!guikit::getExtent(g_cmp.lb, &x, &y, &w, &h)) return false;
    if (!g_cmp.have) {
        if (w < 200 || h < 250) return false;   // not laid out yet
        g_cmp.lbX = x; g_cmp.lbY = y; g_cmp.lbW = w; g_cmp.lbH = h; g_cmp.have = true;
        if (cmpLogOk(LogRects)) cmpLogList("before", panel);
    }
    int sw = 0, sh = 0;
    if (!cmpScreen(&sw, &sh)) return false;

    const uint32_t vt = static_cast<uint32_t>(rd<int>(panel, 0));
    const bool isEquip = (vt == static_cast<uint32_t>(gameaddr::EquipPanelVtable));
    bool credMoved = g_cmp.credMoved;
    int bottomY = 464 * sh / 600;   // 6 gui units of padding above the credits row (y470)
    if (isEquip) {
        const int arr = rd<int>(panel, guikit::Panel_CtlArray), cnt = rd<int>(panel, guikit::Panel_CtlCount);
        bool swapVis = false;
        if (arr && cnt > 43) {
            uintptr_t btnSwap = static_cast<uintptr_t>(rd<int>(arr, 4 * 43));
            if (btnSwap && guikit::isVisible(btnSwap)) swapVis = true;
        }
        if (swapVis) {
            credMoved = false;
        } else {
            credMoved = true;
            bottomY = 470 * sh / 600;
        }
    }

    boxgeo::In gi = {g_cmp.lbX, g_cmp.lbY, g_cmp.lbW, g_cmp.lbH, sh, n, credMoved, bottomY};

    const boxgeo::Out go = boxgeo::compute(gi);
    if (!go.ok) { if (cmpLogOk(LogHide)) logd("ISF-CMP box: geometry rejected n=%d lb=%dx%d sh=%d", n, g_cmp.lbW, g_cmp.lbH, sh); return false; }
    // The engine shows the scrollbar when the last text row's bottom passes the list's client height (REA 20261111_listbox_scrollbar), so a
    // description that exactly fills the shortened list gets a bar (play 11-11). Hand back 3/4 of the gap above the box as slack; a long text still scrolls.
    const int listH = boxgeo::shrinkListH(go.listH, g_cmp.lbH);   // mode B with few rows: never grow past the original list
    g_cmp.n = n; g_cmp.rowP = go.rowP; g_cmp.listH = listH;
    g_cmp.boxX = go.boxX; g_cmp.boxW = go.boxW; g_cmp.boxY = go.boxY; g_cmp.boxH = go.boxH;
    if (x == g_cmp.lbX && y == g_cmp.lbY && w == g_cmp.lbW && h == listH) { g_cmp.shrunk = true; return true; }
    if (g_cmp.shrunk && ++g_cmp.reapplied > 3) {   // something keeps resizing the list: stop fighting it, and hide the box rather than overlay it (review r02 B1)
        if (cmpLogOk(LogHide)) logd("ISF-CMP box: description list moved %d times in one selection, box hidden", g_cmp.reapplied);
        return false;
    }
    guikit::setRect(g_cmp.lb, g_cmp.lbX, g_cmp.lbY, g_cmp.lbW, listH);
    g_cmp.shrunk = true;
    if (cmpLogOk(LogRects)) cmpLogList("after", panel);
    return true;
}

void cmpRestoreList() {
    if (!g_cmp.shrunk || !g_cmp.lb || !g_cmp.have) return;
    guikit::setRect(g_cmp.lb, g_cmp.lbX, g_cmp.lbY, g_cmp.lbW, g_cmp.lbH);
    g_cmp.shrunk = false;
}

void cmpHide(int panel) {
    g_cmpKeyValid = false;
    if (!g_cmpShown && !g_cmp.shrunk) return;
    g_cmpShown = false;
    if (!cmpLive(panel)) return;
    if (g_cmp.frame) guikit::setVisible(g_cmp.frame, false);
    if (g_cmp.hdr) guikit::setVisible(g_cmp.hdr, false);
    for (int i = 0; i < CmpRows; ++i) { guikit::setVisible(g_cmp.name[i], false); guikit::setVisible(g_cmp.val[i], false); }
    cmpRestoreList();
}

void cmpColor(uintptr_t c, float r, float g, float b) { float cur[3]; if (guikit::getTextColor(c, cur)) guikit::setTextColor(c, r, g, b); }
void cmpKindColor(uintptr_t c, cband::RowKind k, bool valueCol) {
    switch (k) {
        case cband::RK_Up: cmpColor(c, 0.95f, 0.66f, 0.30f); break;
        case cband::RK_Down: cmpColor(c, 0.98f, 0.38f, 0.31f); break;
        case cband::RK_Dim: cmpColor(c, 0.0663f, 0.4537f, 0.3569f); break;
        case cband::RK_Header: cmpColor(c, 0.80f, 0.80f, 0.698f); break;
        default: if (valueCol) cmpColor(c, 1.0f, 1.0f, 1.0f); else cmpColor(c, 0.80f, 0.80f, 0.698f); break;   // plain value = white, plain whole row = cream
    }
}

// Writes `lines` rows into the box (header = row 0). Rows past nLines are hidden.
// guikit alignment 9 = top-left (GuiKit.h:291). Logged once per failure kind so a lost alignment is visible in the log, not guessed.
void cmpLeftAlign(uintptr_t c) {
    static bool warned = false;
    if (!guikit::setAlign(c, 9) && !warned && cmpLogOk(LogHide)) { warned = true; logd("ISF-CMP setAlign(9) failed for label %08x (text object unreadable?)", static_cast<unsigned>(c)); }
}

void cmpDraw(const char (*lines)[64], unsigned nLines, const char* hdrText) {
    int sw = 0, sh = 0;
    if (!cmpScreen(&sw, &sh)) return;
    const int pad = 6 * sh / 600, rowH = g_cmp.rowP, innerW = g_cmp.boxW - 2 * pad;
    const textfit::TextCtx tc = { sh / 600.0f, 1.03f, 1.0f };
    // name column = widest split name + a gap (value column hugs the labels), capped at 40% of the box
    int nameWs[CmpRows] = {0}; unsigned nNameW = 0;
    for (unsigned i = 1; i < nLines && i < static_cast<unsigned>(CmpRows); ++i) {
        const unsigned sp = lines[i][0] ? cband::splitName(lines[i]) : 0;
        if (!sp) continue;
        size_t nl = sp > 63 ? 63 : sp; while (nl && lines[i][nl - 1] == ' ') --nl;
        const int w = static_cast<int>(textfit::textPx(tc, lines[i], static_cast<int>(nl), textfit::F16)) + 14 * sh / 600;
        nameWs[nNameW++] = w;
    }
    const int nameW = cband::nameColumnW(nameWs, nNameW, innerW * 40 / 100);
    if (g_cmp.frame) { guikit::setRect(g_cmp.frame, g_cmp.boxX, g_cmp.boxY, g_cmp.boxW, g_cmp.boxH); guikit::setVisible(g_cmp.frame, true); }
    for (int i = 0; i < CmpRows; ++i) {
        const bool used = static_cast<unsigned>(i) < nLines && lines[i][0];
        if (!used) { guikit::setVisible(g_cmp.name[i], false); guikit::setVisible(g_cmp.val[i], false); continue; }
        const char* t = lines[i];
        const bool hasHdr = hdrText && hdrText[0] && g_cmp.hdr;
        const cband::RowKind k = cband::rowKind(t, i == 0 && !hasHdr);   // merged: row 0 is a stat row, the header is the right-hand label
        const unsigned sp = i == 0 ? 0 : cband::splitName(t);
        const int y = g_cmp.boxY + pad + i * rowH;
        char nm[64] = {0}, vl[64] = {0};
        if (sp) {
            memcpy(nm, t, sp > 63 ? 63 : sp); nm[sp > 63 ? 63 : sp] = 0;
            size_t nl = strlen(nm); while (nl && nm[nl - 1] == ' ') nm[--nl] = 0;
            { char fn[64]; textfit::fitText(tc, nm, static_cast<float>(nameW - 4), fn, sizeof fn, textfit::F16); strcpy(nm, fn); }   // r03 F4: name column pixel-fit
            char tmp[64]; strncpy(tmp, t + sp, 63); tmp[63] = 0;
            textfit::fitText(tc, tmp, static_cast<float>(innerW - nameW - 8), vl, sizeof vl, textfit::F16);
            guikit::setRect(g_cmp.name[i], g_cmp.boxX + pad, y, nameW, rowH);
            guikit::setRect(g_cmp.val[i], g_cmp.boxX + pad + nameW, y, innerW - nameW, rowH);
            guikit::setText(g_cmp.name[i], nm); cmpColor(g_cmp.name[i], 0.102f, 0.698f, 0.549f);
            guikit::setText(g_cmp.val[i], vl); cmpKindColor(g_cmp.val[i], k, true);
            cmpLeftAlign(g_cmp.name[i]); cmpLeftAlign(g_cmp.val[i]);   // re-assert top-left after setText: the box rendered centred on the equip screen
            guikit::setVisible(g_cmp.name[i], true); guikit::setVisible(g_cmp.val[i], true);
        } else {
            textfit::fitText(tc, t, static_cast<float>(innerW - 8), nm, sizeof nm, textfit::F16);
            guikit::setRect(g_cmp.name[i], g_cmp.boxX + pad, y, innerW, rowH);
            guikit::setText(g_cmp.name[i], nm); cmpKindColor(g_cmp.name[i], k, false);
            cmpLeftAlign(g_cmp.name[i]);
            guikit::setVisible(g_cmp.name[i], true); guikit::setVisible(g_cmp.val[i], false);
        }
    }
    // equip: the hdr clone is a full-size background with the row-label fill, inset 3 px so the frame's edge line stays visible (the frame's own fill
    // is inset by the edge art and left a black band). Inventory keeps the frame fill and hides it.
    if (g_cmp.hdr) {
        if (g_cmp.bgFill) {
            guikit::setRect(g_cmp.hdr, g_cmp.boxX + 3, g_cmp.boxY + 3, g_cmp.boxW - 6, g_cmp.boxH - 6);
            guikit::setText(g_cmp.hdr, "");
            guikit::setVisible(g_cmp.hdr, true);
        } else guikit::setVisible(g_cmp.hdr, false);
    }
}

// Readback of what the controls hold RIGHT NOW (tag: pre = start of a select, post = after our write, hide = after a hide). Comparing the
// post line of one select with the pre line of the next shows whether the engine changed the box between our write and the next click.
void cmpReadback(const char* tag, unsigned sel) {
    if (!g_cmp.lb || !cmpLogOk(LogFacts)) return;
    int lx = 0, ly = 0, lw = 0, lh = 0; guikit::getExtent(g_cmp.lb, &lx, &ly, &lw, &lh);
    int vis = 0; for (int i = 0; i < CmpRows; ++i) vis |= (guikit::isVisible(g_cmp.name[i]) ? 1 : 0) << i;
    logd("ISF-CMP rb-%s sel=%08x shown=%d shrunk=%d keyValid=%d list=%dx%d clientH=%d frameVis=%d rowVisMask=%x n=%d",
         tag, sel, g_cmpShown ? 1 : 0, g_cmp.shrunk ? 1 : 0, g_cmpKeyValid ? 1 : 0, lw, lh, rd<int>(static_cast<int>(g_cmp.lb), ListClientH, -1),
         g_cmp.frame && guikit::isVisible(g_cmp.frame) ? 1 : 0, vis, g_cmp.n);
}

// Per row select. Slots are re-read every time (cheap); the compare itself runs only when the key (panel, row id, creature, 7 slot items) changed.
// Catch-up (ISF 0.9.0-dev, speculative): the screen showed the box for the PREVIOUS select. There is no per-frame hook on the inventory
// screen except K2HUD_FRAME, so after every select the wanted state is stored and re-asserted for a few frames from onStripFrame
// (same thread as the engine, so no race). Logs "ISF-CMP catchup drift" when the engine changed something between our write and the re-assert.
struct CmpWant { bool show = false; int geoRows = 4; unsigned n = 0; char block[6][64] = {}; char hdr[64] = {}; int panel = 0; unsigned sel = 0; int redo = 0; } g_want;
// Equip select with the cursor off the list = programmatic (the engine auto-selects the equipped row on open / Switch Weapons). It must NOT be retried:
// an earlier retry adopted whatever row the cursor reached next, so hovering after a programmatic select showed that row's comparison (play 2026-11-11 01:20).
void cmpOnSelect(int ebp);
int cmpHitRowEquip(int panel, int argRow);
void cmpCatchUp() {
    // The equip screen swaps the description list for the slot grid without a row select: the box would stay over the grid and cover
    // its buttons (play 2026-11-09 23:37, Switch Weapons blocked). A shown box with a hidden description list is hidden now.
    if (g_cmpShown && g_cmp.have && g_cmp.lb && g_cmp.panel && cmpLive(g_cmp.panel) && !guikit::isVisible(g_cmp.lb)) {
        if (cmpLogOk(LogHide)) logd("ISF-CMP hide: description list not visible (screen mode changed), box hidden");
        cmpHide(g_cmp.panel);
        g_want.show = false; g_want.redo = 0;
        return;
    }
    if (g_want.redo <= 0 || !g_want.panel || !g_cmp.have || !g_cmp.lb) return;
    --g_want.redo;
    const int panel = g_want.panel;
    if (!cmpLive(panel)) { g_want.redo = 0; return; }
    int lx = 0, ly = 0, lw = 0, lh = 0; guikit::getExtent(g_cmp.lb, &lx, &ly, &lw, &lh);
    const bool fv = g_cmp.frame && guikit::isVisible(g_cmp.frame);
    const bool drift = g_want.show ? (!fv || (g_cmp.shrunk && lh != g_cmp.listH)) : (fv || lh != g_cmp.lbH);
    if (drift && cmpLogOk(LogHide))
        logd("ISF-CMP catchup drift sel=%08x want=%d frameVis=%d list=%dx%d (wantH=%d origH=%d) redo=%d", g_want.sel, g_want.show ? 1 : 0, fv ? 1 : 0, lw, lh, g_cmp.listH, g_cmp.lbH, g_want.redo);
    if (!drift) return;
    g_cmp.reapplied = 0;
    if (g_want.show) { if (cmpShrink(panel, g_want.geoRows)) { cmpDraw(g_want.block, g_want.n, g_want.hdr); g_cmpShown = true; } }
    else if (g_cmpShown || g_cmp.shrunk) cmpHide(panel);
}

// The row argument of the row-select epilogue is the row the engine LEFT (play 2026-11-08: the box always described the previously clicked
// item; the first hook call after open carried the default first row; no hook call at all followed the last click). The newly clicked row is
// the one under the cursor, so find it by hit-testing the panel's row controls. Exactly one hit -> that row; none/several -> 0 (caller keeps
// the row argument). Logs both so the next play shows whether the extents are usable.
int cmpHitRow(int panel, int argRow) {
    int cx = 0, cy = 0;
    if (!panel) { if (cmpLogOk(LogHit)) logd("ISF-CMP hit skip: panel null"); return 0; }
    if (!guikit::cursorEngine(&cx, &cy)) { if (cmpLogOk(LogHit)) logd("ISF-CMP hit skip: cursor unreadable (no client size or window)"); return 0; }
    const int arr = rd<int>(panel, PanelRowArrayOff), cnt = rd<int>(panel, PanelRowCountOff);
    if (!arr || cnt <= 0 || cnt > 4096) { if (cmpLogOk(LogHit)) logd("ISF-CMP hit skip: row array arr=%08x cnt=%d", static_cast<unsigned>(arr), cnt); return 0; }
    int found = 0, hits = 0, argX = 0, argY = 0, argW = 0, argH = 0;
    guikit::getExtent(static_cast<uintptr_t>(argRow), &argX, &argY, &argW, &argH);
    // Row extents are relative to LB_ITEMS (first row y=0, pitch = row height; play 2026-11-08 21:06), the cursor is screen px: shift the cursor
    // into list space. Scroll offset is not known (assumed 0); the log shows the local cursor so a scrolled list can be checked.
    int lbX = 0, lbY = 0, lbW = 0, lbH = 0;
    const int lb = panel + PanelItemsListOff;   // LB_ITEMS is an inline control (not a pointer): rd<int> returned its vtable, so every hit-test said "outside list"
    if (!lb || !guikit::getExtent(static_cast<uintptr_t>(lb), &lbX, &lbY, &lbW, &lbH) || lbW <= 0 || lbH <= 0) {
        if (cmpLogOk(LogHit)) logd("ISF-CMP hit skip: LB_ITEMS unreadable lb=%08x ext=%d,%d %dx%d", static_cast<unsigned>(lb), lbX, lbY, lbW, lbH);
        return 0;
    }
    const int rawX = cx, rawY = cy;
    const long long lx = (long long)cx - lbX, ly = (long long)cy - lbY;   // 64-bit: garbage extents must not overflow
    if (lx < 0 || ly < 0 || lx >= lbW || ly >= lbH) {   // cursor not over the list (keyboard/controller): keep the row argument
        if (cmpLogOk(LogHit)) logd("ISF-CMP hit skip: cursor outside list raw=%d,%d lb=%d,%d %dx%d (row argument used)", rawX, rawY, lbX, lbY, lbW, lbH);
        return 0;
    }
    cx = (int)lx; cy = (int)ly;
    for (int i = 0; i < cnt && i < 4096; ++i) {
        const int r = rd<int>(arr, 4 * i);
        int x = 0, y = 0, w = 0, h = 0;
        if (!r || !guikit::isVisible(static_cast<uintptr_t>(r)) || !guikit::getExtent(static_cast<uintptr_t>(r), &x, &y, &w, &h) || w <= 0 || h <= 0) continue;   // hidden (filtered-out) rows keep stale extents
        if (cx >= x && (long long)cx < (long long)x + w && cy >= y && (long long)cy < (long long)y + h) { found = r; ++hits; }
    }
    if (cmpLogOk(LogHit))
        logd("ISF-CMP hit lb=%d,%d %dx%d raw=%d,%d local=%d,%d argRow=%08x arg=%08x ext=%d,%d %dx%d | hits=%d hitRow=%08x hit=%08x rows=%d", lbX, lbY, lbW, lbH, rawX, rawY, cx, cy, static_cast<unsigned>(argRow),
             argRow ? static_cast<unsigned>(rd<int>(argRow, eqprobe::RowItemIdOff)) : 0u, argX, argY, argW, argH, hits, static_cast<unsigned>(found),
             found ? static_cast<unsigned>(rd<int>(found, eqprobe::RowItemIdOff)) : 0u, cnt);
    if (hits != 1 && cmpLogOk(LogHit)) logd("ISF-CMP hit skip: %d rows under the cursor (row argument used)", hits);
    return hits == 1 ? found : 0;
}

// Equip panel row hit-test (play 2026-11-10 probe): the equip panel keeps its item rows in two arrays, panel+0x22A0 (count +0x22A4, 28 rows) and
// panel+0x39D8 (count +0x39DC, 18 rows). LB_ITEMS is the panel's OTHER list control (same vtable as the description list g_cmp.lb); row extents are
// relative to it (same convention as the inventory list). The row argument is the row the engine LEFT, so the clicked row is the one under the cursor.
// Returns the single row under the cursor, else 0 (caller keeps the row argument). Every outcome is logged under the LogHit budget.
int cmpHitRowEquip(int panel, int argRow) {
    int cx = 0, cy = 0;
    if (!panel) return 0;
    if (!g_cmp.lb) g_cmp.lb = cmpFindDesc(panel, &g_cmp.lbIdx);   // found lazily by cmpShrink, which has not run yet on the first select (play 2026-11-11: first click did nothing)
    if (!g_cmp.lb) return 0;
    if (!guikit::cursorEngine(&cx, &cy)) { if (cmpLogOk(LogHit)) logd("ISF-CMP eq-hit skip: cursor unreadable"); return 0; }
    uintptr_t lb = 0;
    const int carr = rd<int>(panel, guikit::Panel_CtlArray), ccnt = rd<int>(panel, guikit::Panel_CtlCount);
    const int descVt = rd<int>(static_cast<int>(g_cmp.lb), 0);
    if (carr && ccnt > 0 && ccnt < 512 && descVt)
        for (int i = 0; i < ccnt; ++i) {
            const int c = rd<int>(carr, 4 * i);
            int x = 0, y = 0, w = 0, h = 0;
            if (!c || static_cast<uintptr_t>(c) == g_cmp.lb || rd<int>(c, 0) != descVt) continue;
            if (guikit::getExtent(static_cast<uintptr_t>(c), &x, &y, &w, &h) && w >= 150 && h >= 150) { lb = static_cast<uintptr_t>(c); break; }
        }
    int lbX = 0, lbY = 0, lbW = 0, lbH = 0;
    if (!lb || !guikit::getExtent(lb, &lbX, &lbY, &lbW, &lbH) || lbW <= 0 || lbH <= 0) { if (cmpLogOk(LogHit)) logd("ISF-CMP eq-hit skip: LB_ITEMS not found (ctl=%d)", ccnt); return 0; }
    const long long lx = (long long)cx - lbX, ly = (long long)cy - lbY;
    if (lx < 0 || ly < 0 || lx >= lbW || ly >= lbH) { if (cmpLogOk(LogHit)) logd("ISF-CMP eq-hit skip: cursor outside list raw=%d,%d lb=%d,%d %dx%d", cx, cy, lbX, lbY, lbW, lbH); return 0; }
    static const int kArr[2][2] = { {0x22A0, 0x22A4}, {0x39D8, 0x39DC} };
    int found = 0, distinct = 0, hitsTotal = 0;
    for (const auto& ac : kArr) {
        const int arr = rd<int>(panel, ac[0]), cnt = rd<int>(panel, ac[1]);
        if (!arr || cnt <= 0 || cnt > 512) continue;
        for (int i = 0; i < cnt; ++i) {
            const int r = rd<int>(arr, 4 * i);
            int x = 0, y = 0, w = 0, h = 0;
            if (!r || !guikit::isVisible(static_cast<uintptr_t>(r)) || !guikit::getExtent(static_cast<uintptr_t>(r), &x, &y, &w, &h) || w <= 0 || h <= 0) continue;
            if (lx >= x && lx < (long long)x + w && ly >= y && ly < (long long)y + h) { ++hitsTotal; if (r != found) { found = r; ++distinct; } }
        }
    }
    if (cmpLogOk(LogHit))
        logd("ISF-CMP eq-hit lb=%d,%d %dx%d raw=%d,%d local=%lld,%lld argRow=%08x hits=%d distinct=%d hit=%08x id=%08x", lbX, lbY, lbW, lbH, cx, cy, lx, ly, static_cast<unsigned>(argRow), hitsTotal,
             distinct, static_cast<unsigned>(found), found ? static_cast<unsigned>(rd<int>(found, eqprobe::RowItemIdOff)) : 0u);
    return distinct == 1 ? found : 0;
}

// PROBE (item prop aggregation design): when an item loses rows to the 16-row cap, log every raw property row once per item so the next play
// shows what the extra rows are (type, subtype, cost table/value, param1). Capped by the LogProps budget.
void cmpDumpProps(uintptr_t item, const char* name, unsigned dropped) {
    static uintptr_t lastItem = 0; static int lastMask = 0;
    if (item == lastItem || !cmpLogOk(LogProps)) return;
    if (lastItem) {   // the engine may set item+0x2D4 only while the description is built: re-read the previous item's mask now (it has been shown since)
        int now = 0; itemcmpread::rd(lastItem, 0x2D4, &now);
        logd("ISF-CMP propdump late item=%08x item+2d4 then=%08x now=%08x", static_cast<unsigned>(lastItem), static_cast<unsigned>(lastMask), static_cast<unsigned>(now));
    }
    lastItem = item;
    int32_t cnt[2] = {0, 0}; uint32_t arr[2] = {0, 0};
    itemcmpread::rd(item, itemcmpread::Item_PassivePropCount, &cnt[0]); itemcmpread::rd(item, itemcmpread::Item_PassivePropArray, &arr[0]);
    itemcmpread::rd(item, itemcmpread::Item_ActivePropCount, &cnt[1]); itemcmpread::rd(item, itemcmpread::Item_ActivePropArray, &arr[1]);
    int mask2d4 = 0; itemcmpread::rd(item, 0x2D4, &mask2d4);    // REA 2026-11-10 (0x60b6b0): a prop counts when its slot byte (+0x11) is 0xFF or that bit is set here
    lastMask = mask2d4;
    logd("ISF-CMP propdump \"%s\" dropped=%u passive=%d active=%d item+2d4=%08x", name, dropped, cnt[0], cnt[1], static_cast<unsigned>(mask2d4));
    for (int l = 0; l < 2; ++l)
        for (int i = 0; i < cnt[l] && i < 90; ++i) {
            itemcmpread::RawProp r;
            if (!itemcmpread::readRaw(static_cast<uintptr_t>(arr[l]), i, &r)) { logd("ISF-CMP propdump %c[%d] unreadable", l ? 'A' : 'P', i); break; }
            uint8_t slotB = 0xEE, b10 = 0xEE; itemcmpread::rd(static_cast<uintptr_t>(arr[l]), i * 0x1C + 0x11, &slotB); itemcmpread::rd(static_cast<uintptr_t>(arr[l]), i * 0x1C + 0x10, &b10);
            logd("ISF-CMP propdump %c[%d] type=%u sub=%u tbl=%u val=%u p1=%u p1v=%u slot+11=%02x b+10=%02x", l ? 'A' : 'P', i, r.type, r.subtype, r.costTable, r.costValue, r.param1, r.param1Value, slotB, b10);
            if (r.costTable == 4 || r.costTable == 7 || r.costTable == 20 || r.costTable == 21) {   // PROBE Q3: is the stored value the 2da row index or the value column? the engine text says
                itemcmpread::PropStrings ps; memset(&ps, 0, sizeof ps);
                itemcmpread::g_propText(r.type, r.subtype, r.costValue, r.param1 == itemcmpread::Param1None ? itemcmpread::Param1None : r.param1Value, &ps);
                ps.name[sizeof ps.name - 1] = ps.sub[sizeof ps.sub - 1] = ps.val[sizeof ps.val - 1] = 0;
                if (cmpLogOk(LogProbe))
                    logd("ISF-PRB cost item=%08x %c[%d] type=%u sub=%u tbl=%u val=%u txt=\"%.24s|%.24s|%.24s\"", static_cast<unsigned>(item), l ? 'A' : 'P', i, r.type, r.subtype, r.costTable, r.costValue, ps.name, ps.sub, ps.val);
            }
        }
}

// PROBE Q1: item+0x2D4 next to the six dwords at +0x2F4 (installed-slot template ids per the workbench doc) and the prop counts, so one sample says whether the
// mask is "installed" (changes with a crystal) or "template" (constant, +0x2F4 / the prop array change). src = view (selected item) or snap (equipped snapshot).
void cmpProbeMask(uintptr_t item, const char* name, const char* src, int slot) {
    if (!item || !cmpLogOk(LogProbe)) return;
    int m = 0, pas = 0, act = 0; uint32_t f[6] = {0, 0, 0, 0, 0, 0};
    itemcmpread::rd(item, 0x2D4, &m);
    itemcmpread::rd(item, itemcmpread::Item_PassivePropCount, &pas);
    itemcmpread::rd(item, itemcmpread::Item_ActivePropCount, &act);
    for (int k = 0; k < 6; ++k) itemcmpread::rd(item, 0x2F4 + 4 * k, &f[k]);
    logd("ISF-PRB mask src=%s slot=%d item=%08x n=\"%.20s\" 2d4=%08x f2f4=%08x %08x %08x %08x %08x %08x pas=%d act=%d", src, slot, static_cast<unsigned>(item), name ? name : "",
         static_cast<unsigned>(m), f[0], f[1], f[2], f[3], f[4], f[5], pas, act);
}

// PROBE Q2: the engine's Damage line (REA ledger 20261111_item-desc-damage-fold) = base + sum(type 5) - sum(type 0xF) on both ends, min clamped to 1, prop folds gated by the slot byte (+0x11).
// pred is what that formula gives with and without the gate; the tester writes the on-screen range next to it.
void cmpProbeDmg(uintptr_t item, const char* name, int nd, int die) {
    if (!item || nd <= 0 || die <= 0 || nd > 9 || !cmpLogOk(LogProbe)) return;
    int mask = 0; itemcmpread::rd(item, 0x2D4, &mask);
    int s5 = 0, s15 = 0, s5a = 0, s15a = 0, oth = 0;
    for (int l = 0; l < 2; ++l) {
        uint32_t arr = 0; int32_t cnt = 0;
        itemcmpread::rd(item, l ? itemcmpread::Item_ActivePropCount : itemcmpread::Item_PassivePropCount, &cnt);
        itemcmpread::rd(item, l ? itemcmpread::Item_ActivePropArray : itemcmpread::Item_PassivePropArray, &arr);
        for (int i = 0; i < cnt && i < 90; ++i) {
            itemcmpread::RawProp r;
            if (!itemcmpread::readRaw(static_cast<uintptr_t>(arr), i, &r)) break;
            uint8_t sb = 0xFF; itemcmpread::rd(static_cast<uintptr_t>(arr), i * 0x1C + 0x11, &sb);
            const bool pass = sb == 0xFF || ((static_cast<uint32_t>(mask) >> (sb & 0x1F)) & 1u);
            if (r.type == 5) { s5a += r.costValue; if (pass) s5 += r.costValue; }
            else if (r.type == 0xF) { s15a += r.costValue; if (pass) s15 += r.costValue; }
            else if (pass && (r.type == 0xB || r.type == 0xC || r.type == 0xD || r.type == 0x10)) ++oth;
        }
    }
    const int lo0 = nd + s5 - s15, lo = lo0 < 1 ? 1 : lo0, hi = nd * die + s5 - s15;
    const int lo1 = nd + s5a - s15a, loA = lo1 < 1 ? 1 : lo1, hiA = nd * die + s5a - s15a;
    logd("ISF-PRB dmg n=\"%.20s\" base=%dd%d s5=%d s15=%d s5all=%d s15all=%d oth=%d pred=%d-%d predAll=%d-%d", name ? name : "", nd, die, s5, s15, s5a, s15a, oth, lo, hi, loA, hiA);
}

// PROBE Q4: both weapon sets' slots next to the snapshot's right hand. After Switch Weapons R1 must equal the old R2 and snapR must follow it.
void cmpProbeWset(uintptr_t creature, uintptr_t snapR) {
    uint32_t inv = 0;
    if (!creature || !cmpLogOk(LogProbe) || !itemcmpread::rd(creature, itemcmpread::Creature_Inv, &inv) || !inv ||
        !saferead::probeRead(itemcmpread::SiteCmp, reinterpret_cast<const void*>(static_cast<uintptr_t>(inv)), 0x20)) return;   // g_invSlot derefs inv unguarded
    const uintptr_t i = static_cast<uintptr_t>(inv);
    logd("ISF-PRB wset R1=%08x L1=%08x R2=%08x L2=%08x snapR=%08x", static_cast<unsigned>(itemcmpread::g_invSlot(i, 0x10)), static_cast<unsigned>(itemcmpread::g_invSlot(i, 0x20)),
         static_cast<unsigned>(itemcmpread::g_invSlot(i, 0x40000)), static_cast<unsigned>(itemcmpread::g_invSlot(i, 0x80000)), static_cast<unsigned>(snapR));
}

// sl[] = the slot facts READ AT SNAPSHOT TIME: after Switch Weapons the snapshot's right hand is a transient object the engine frees on the next click
// (play 11-11: the pointer then read as another item, "Disguise"), so later selects reuse these copies instead of re-reading the pointers.
struct TrueEq { uintptr_t panel = 0, creature = 0, lastViewed = 0; uintptr_t it[7] = {0,0,0,0,0,0,0}; bool valid = false; itemcmp::SlotRead sl[7]; unsigned wset = 0;
    void reset() { panel = creature = lastViewed = 0; for (auto& p : it) p = 0; valid = false; wset = 0; for (auto& x : sl) x = itemcmp::SlotRead(); } };   // no whole-struct temporary (~9 KB) on the hook stack
static TrueEq g_trueEq;   // items equipped when the equip panel opened (the screen previews clicked items on the creature)

void cmpOnSelect(int ebp) {
    const int argRow = rd<int>(ebp, F_SelectRowArg), panel = rd<int>(ebp, F_SelectPanel);
    if (panel && g_cmp.panel != panel && g_cmpOther.panel == panel) {   // select on the other panel: bring its box in (never draw into a box of a different panel)
        if (g_cmpShown || g_cmp.shrunk) cmpHide(g_cmp.panel);   // review ISF-REV: the outgoing box must not keep its frame/shrunk list
        const CmpBox t = g_cmp; g_cmp = g_cmpOther; g_cmpOther = t;
        g_cmpShown = false; g_cmpKeyValid = false; g_want.redo = 0;
        if (cmpLogOk(LogHide)) logd("ISF-CMP box swapped to panel %08x", static_cast<unsigned>(panel));
    }
    if (!g_cfg.compare) { if (g_cmpShown || g_cmp.shrunk) cmpHide(panel); return; }
    if (!panel || !cmpLive(panel)) {   // equip screen / other panel: not ours
        if (g_cmpShown || g_cmp.shrunk) cmpHide(panel);
        if (!panel) eqbox::onClose(g_eqState, g_eqState.panelState.current);
        if (panel && cmpLogOk(LogHide)) logd("ISF-CMP select ignored: cmpLive false sel=%08x panel=%08x (box hidden)", (unsigned)rd<int>(ebp, F_SelectRowArg), (unsigned)panel);
        return;
    }
    const uint32_t vt = static_cast<uint32_t>(rd<int>(panel, 0));
    const bool isEquip = (vt == static_cast<uint32_t>(gameaddr::EquipPanelVtable));

    const int hitRow = isEquip ? cmpHitRowEquip(panel, argRow) : cmpHitRow(panel, argRow);
    // Play 01:20 (hover adoption) and 01:01 (old lightsaber): cursor off the list = programmatic select (panel open, Switch Weapons, slot button) OR an
    // unreadable/ambiguous hit-test (keyboard nav); the retake below keeps the previewed slots either way. A plain local: an early return cannot leave it set.
    const bool prog = isEquip && !hitRow;
    const int row = hitRow ? hitRow : argRow;   // inventory hit-test uses CSWGuiInventory offsets; equip panel uses argRow directly

    static itemcmp::CompareInput in;   // ~10 KB each: statics, never on the hook stack
    static itemcmp::CompareOut out;
    in.~CompareInput(); new (&in) itemcmp::CompareInput();   // reset in place (no ~10 KB temporary on the hook stack); every slot starts Unreadable, so a skipped fill never leaves stale state

    const itemcmp::SlotRead* sl[7] = {&in.right, &in.left, &in.armor, &in.head, &in.gauntlet, &in.implant, &in.belt};
    unsigned rawId = 0;
    unsigned creature = 0;
    uintptr_t viewed = 0;
    int sm = 0;
    bool smOk = false;
    cband::Key key;
    bool idOk = false;

    if (isEquip) {
        rawId = static_cast<unsigned>(row ? rd<int>(row, eqprobe::RowItemIdOff) : 0);   // row = hit-tested clicked row, else the row argument
        const auto r = eqpanel::resolveItemId(rawId, true);
        const uint32_t itemId = r.valid ? r.id : 0u;

        smOk = saferead::readAt<int>(5, static_cast<uintptr_t>(panel), 0x5098, &sm);

        int c68i = 0;
        saferead::readAt<int>(5, static_cast<uintptr_t>(panel), 0x68, &c68i);
        const uintptr_t c68 = static_cast<uintptr_t>(static_cast<uint32_t>(c68i));
        uintptr_t cre = 0;
        uint32_t c68vt = 0;
        if (c68 > 0x10000 && saferead::readAt<uint32_t>(5, c68, 0, &c68vt) && c68vt == static_cast<uint32_t>(gameaddr::ClientCreatureVtable)) {
            uintptr_t tbl = party::table();
            if (tbl) {
                const int pcnt = party::partyCount(tbl);
                for (int i = 0; i < pcnt; ++i) {
                    uintptr_t m = party::member(tbl, i);
                    if (m == c68) {
                        cre = party::serverCreature(m);
                        break;
                    }
                }
            }
        }

        eqbox::EquippedSlots eqSlots{};
        auto getSlotFromPanel = [&](int slotIdx) -> uint32_t {
            int v = 0;
            if (saferead::readAt<int>(5, static_cast<uintptr_t>(panel), 0x509c + 4 * slotIdx, &v)) {
                const auto ri = eqpanel::resolveItemId(static_cast<uint32_t>(v), true);
                return ri.valid ? ri.id : 0u;
            }
            return 0u;
        };
        eqSlots.leftHand  = getSlotFromPanel(0);
        eqSlots.rightHand = getSlotFromPanel(1);
        eqSlots.head      = getSlotFromPanel(2);
        eqSlots.armor     = getSlotFromPanel(5);
        eqSlots.gloves    = getSlotFromPanel(6);
        eqSlots.implant   = getSlotFromPanel(7);
        eqSlots.belt      = getSlotFromPanel(8);

        eqbox::SelectInput sin{};
        sin.panelVtable = vt;
        sin.selectedRawId = rawId;
        sin.slotMask = smOk ? static_cast<uint32_t>(sm) : 0u;
        sin.creatureReadable = (cre != 0);
        sin.equipped = eqSlots;
        sin.creature = c68;
        const eqbox::Decision d = eqbox::decide(g_eqState, sin);

        if (!d.show) {
            cmpHide(panel);
            return;
        }
        if (d.deduped) return;

        creature = static_cast<unsigned>(cre);
        idOk = true;
        viewed = itemcmpread::viewedFromRow(d.viewedItemId);
        if (creature) {
            // The equip screen PREVIEWS the clicked item on the creature, so the live slot holds the previously clicked item (log: R= follows the last
            // click). Compare against the items equipped when the panel opened: snapshot once, refresh only when a slot changed to something that is
            // neither the snapshot nor the item we last showed (a real equip/unequip done elsewhere).
            uintptr_t live[7]; itemcmpread::slotItemPtrs(creature, live);
            const bool firstTake = g_trueEq.creature != creature || g_trueEq.panel != static_cast<uintptr_t>(panel);
            const unsigned wsetNow = smOk ? eqbox::weaponSet(static_cast<uint32_t>(sm)) : g_trueEq.wset;
            uintptr_t src[7]; for (int i = 0; i < 7; ++i) src[i] = live[i];
            if (wsetNow == 1) {   // editing the second weapon set: its hands are the compare targets
                uint32_t inv = 0;
                if (itemcmpread::rd(creature, itemcmpread::Creature_Inv, &inv) && inv &&
                    saferead::probeRead(itemcmpread::SiteCmp, reinterpret_cast<const void*>(static_cast<uintptr_t>(inv)), 0x20)) {
                    src[0] = itemcmpread::g_invSlot(static_cast<uintptr_t>(inv), 0x40000);
                    src[1] = itemcmpread::g_invSlot(static_cast<uintptr_t>(inv), 0x80000);
                }
            }
            unsigned refill = 0;
            if (firstTake) {
                g_trueEq.reset(); g_trueEq.creature = creature; g_trueEq.panel = static_cast<uintptr_t>(panel); g_trueEq.wset = wsetNow;
                for (int i = 0; i < 7; ++i) g_trueEq.it[i] = src[i];
                refill = 0x7F; g_trueEq.valid = true;
                for (int i = 0; i < 7; ++i) {   // PROBE: upgrade-slot mask of each equipped item (does the equipped lightsaber carry a non-zero item+0x2D4?)
                    int m = 0, pc = 0;
                    if (live[i] && itemcmpread::rd(live[i], 0x2D4, &m) && itemcmpread::rd(live[i], itemcmpread::Item_PassivePropCount, &pc) && cmpLogOk(LogProps))
                        logd("ISF-CMP eqmask slot=%d item=%08x item+2d4=%08x passive=%d", i, static_cast<unsigned>(live[i]), static_cast<unsigned>(m), pc);
                }
            } else if (prog || wsetNow != g_trueEq.wset) {   // F1/F4: retake, but slots holding the last preview keep the snapshot
                uintptr_t nit[7]; refill = eqbox::snapRetake(src, g_trueEq.it, g_trueEq.lastViewed, nit);
                for (int i = 0; i < 7; ++i) g_trueEq.it[i] = nit[i];
                g_trueEq.wset = wsetNow;
                if (cmpLogOk(LogHide)) logd("ISF-CMP equip snapshot retaken refill=%02x prog=%d wset=%u", refill, prog ? 1 : 0, wsetNow);
            }
            if (refill) {
                itemcmpread::fillSlotsFromPtrs(g_trueEq.it, &in);   // `in` (static) is rewritten from sl[] below
                const itemcmp::SlotRead* fr[7] = { &in.right, &in.left, &in.armor, &in.head, &in.gauntlet, &in.implant, &in.belt };
                for (int i = 0; i < 7; ++i) if (refill & (1u << i)) g_trueEq.sl[i] = *fr[i];
            }   // no live adoption: pointer equality cannot tell a preview from a real equip (review: seq 2/5/7/8). A real equip repopulates the list, which resets the snapshot (startPopulate).
            for (int i = 0; i < 7; ++i) cmpProbeMask(g_trueEq.it[i], "", "snap", i);   // PROBE Q1: the truly equipped items, every select
            cmpProbeWset(creature, g_trueEq.it[0]);                                    // PROBE Q4
            g_trueEq.lastViewed = viewed;
            for (int i = 0; i < 7; ++i)
                if (viewed && g_trueEq.it[i] == viewed) {   // the screen auto-selects the equipped row on open: an item against itself is noise
                    if (cmpLogOk(LogHide)) logd("ISF-CMP equip select is the equipped item itself (slot %d): box hidden", i);
                    cmpHide(panel);
                    return;
                }
            in.right = g_trueEq.sl[0]; in.left = g_trueEq.sl[1]; in.armor = g_trueEq.sl[2]; in.head = g_trueEq.sl[3];
            in.gauntlet = g_trueEq.sl[4]; in.implant = g_trueEq.sl[5]; in.belt = g_trueEq.sl[6];
            if (cmpLogOk(LogFacts)) logd("ISF-CMP equip truth live R=%08x L=%08x A=%08x | snap R=%08x L=%08x A=%08x | viewed=%08x", (unsigned)live[0], (unsigned)live[1], (unsigned)live[2], (unsigned)g_trueEq.it[0], (unsigned)g_trueEq.it[1], (unsigned)g_trueEq.it[2], (unsigned)viewed);
        }
        cmpReadback("pre", rawId);
        if (g_cmpShown && !cmpShrink(panel, g_cmp.n) && cmpLogOk(LogHide))
            logd("ISF-CMP pre-select re-shrink failed (n=%d reapplied=%d): box may overlay the list until the next draw", g_cmp.n, g_cmp.reapplied);
    } else {
        cmpReadback("pre", static_cast<unsigned>(row ? rd<int>(row, eqprobe::RowItemIdOff) : 0));
        if (g_cmpShown && !cmpShrink(panel, g_cmp.n) && cmpLogOk(LogHide))   // keep the list shrunk: re-applied when the engine moved it back (bounded)
            logd("ISF-CMP pre-select re-shrink failed (n=%d reapplied=%d): box may overlay the list until the next draw", g_cmp.n, g_cmp.reapplied);

        rawId = row ? rd<unsigned>(row, eqprobe::RowItemIdOff) : 0;
        creature = static_cast<unsigned>(eqprobe::g_cre);
        idOk = rawId != 0 && rawId != static_cast<unsigned>(NoSelection);
        if (creature && idOk) {
            viewed = itemcmpread::viewedFromRow(rawId);
            itemcmpread::fillSlots(creature, &in);
        }
        if ((!creature || !idOk || !viewed) && cmpLogOk(LogHide))
            logd("ISF-CMP select: nothing to compare creature=%08x rawId=%08x idOk=%d viewed=%08x (box hidden)", creature, rawId, idOk ? 1 : 0, static_cast<unsigned>(viewed));
        key.panel = static_cast<unsigned>(panel); key.rawId = rawId; key.creature = creature;
        for (int i = 0; i < 7; ++i)
            key.slot[i] = sl[i]->state == itemcmp::SlotState::Unreadable ? 0xFFFFFFFFu : (sl[i]->state == itemcmp::SlotState::Empty ? 0u : sl[i]->item.id);
        if (!cband::changed(g_cmpKey, g_cmpKeyValid, key)) {
            if (cmpLogOk(LogSame)) logd("ISF-CMP same-key skip sel=%08x shown=%d (box not redrawn)", rawId, g_cmpShown ? 1 : 0);
            return;
        }
        g_cmpKey = key; g_cmpKeyValid = true;
    }
    g_cmp.reapplied = 0;   // review r01 F4: the re-apply cap is per selection, not per panel lifetime

    bool factsOk = false;
    out = itemcmp::CompareOut();
    if (viewed && !saferead::probeRead(2, reinterpret_cast<const void*>(viewed), 4)) {
        if (cmpLogOk(LogHide)) logd("ISF-CMP select: viewed item %08x unreadable, no comparison", static_cast<unsigned>(viewed));
    } else if (viewed && itemcmpread::readFacts(viewed, &in.viewed)) {
        factsOk = true;
        cmpDumpProps(viewed, in.viewed.name, in.viewed.dropped);
        cmpProbeMask(viewed, in.viewed.name, "view", -1);
        cmpProbeDmg(viewed, in.viewed.name, in.viewed.numDice, in.viewed.die);
        if (isEquip && smOk && sm != 0) {
            in.viewed.slotMask = eqbox::viewedHandMask(static_cast<uint32_t>(sm));   // second weapon set edits the same hands
        }
        bool eq = false; unsigned mask = 0;
        in.canEquip = canEquip(static_cast<int>(creature), static_cast<int>(viewed), &eq, &mask);   // unknown => true
        if (isEquip && creature && cmpLogOk(LogFacts)) {   // discriminator (review): is the live slot written by the engine preview, by canEquip above, or is the panel array the truth?
            uintptr_t post[7]; itemcmpread::slotItemPtrs(creature, post);
            int pr = 0; saferead::readAt<int>(5, static_cast<uintptr_t>(g_cmp.panel), 0x509c + 4, &pr);
            logd("ISF-CMP equip post R=%08x snapR=%08x panelRaw=%08x viewed=%08x", (unsigned)post[0], (unsigned)g_trueEq.it[0], (unsigned)pr, (unsigned)viewed);
        }
        itemcmp::compare(in, &out);   // hides only when a NEEDED slot is Unreadable
    }
    const bool show = cband::shouldShow(true, creature, isEquip ? (rawId & ~0x80000000u) : rawId, factsOk, out.nBand) && out.nBlock > 0;
    g_cmpShown = false;
    // Fixed box height (>= 4 rows) so the description list is NOT resized on every select: a row-count change used to resize the list
    // after the engine laid the description out (stale scroll/clip, "a click behind"). More than 4 rows (armor) is rare and still fits.
    static char disp[6][64]; static char dispHdr[64];   // display lines: the equipped item merged onto the right end of row 1 (cband::displayLines)
    const unsigned nDisp = cband::displayLines(out.block, out.nBlock < 6 ? out.nBlock : 6, disp, 6, dispHdr, out.viewed);
    const int geoRows = nDisp < 4 ? 4 : static_cast<int>(nDisp);
    if (show && cmpShrink(panel, geoRows)) {
        cmpDraw(disp, nDisp, dispHdr);
        g_cmpShown = true;
        if (cmpLogOk(LogFacts)) {   // post-draw readback: what the controls actually hold now (proves or clears "stale at paint")
            int lx = 0, ly = 0, lw = 0, lh = 0; guikit::getExtent(g_cmp.lb, &lx, &ly, &lw, &lh);
            int vis = 0; for (int i = 0; i < CmpRows; ++i) vis |= (guikit::isVisible(g_cmp.name[i]) ? 1 : 0) << i;
            logd("ISF-CMP painted sel=%08x rows=%u geoRows=%d list=%dx%d clientH=%d box y=%d h=%d frameVis=%d rowVisMask=%x shrunk=%d reapplied=%d",
                 rawId, out.nBlock, geoRows, lw, lh, rd<int>(static_cast<int>(g_cmp.lb), ListClientH, -1), g_cmp.boxY, g_cmp.boxH, g_cmp.frame && guikit::isVisible(g_cmp.frame) ? 1 : 0, vis, g_cmp.shrunk ? 1 : 0, g_cmp.reapplied);
        }
        cmpReadback("post", rawId);
    } else {
        if (show && cmpLogOk(LogHide)) logd("ISF-CMP hide: description list or screen unreadable");
        cmpHide(panel);
        cmpReadback("hide", rawId);
        if (!isEquip) {
            g_cmpKey = key; g_cmpKeyValid = !show;   // cmpHide cleared validity: keep the dedupe when nothing to show; a failed shrink is retried next select (r02 F1)
        }
    }
    g_want.show = g_cmpShown; g_want.geoRows = geoRows; g_want.n = nDisp; g_want.panel = panel; g_want.sel = rawId; g_want.redo = 4;
    for (unsigned i = 0; i < g_want.n; ++i) { strncpy(g_want.block[i], disp[i], 63); g_want.block[i][63] = 0; }
    strncpy(g_want.hdr, dispHdr, 63); g_want.hdr[63] = 0;
    if (cmpLogOk(LogFacts)) {   // full picture per select: what was viewed, what is equipped, every box line (diagnoses stale/mismatched boxes)
        char blk[420]; blk[0] = 0; size_t bp = 0;
        for (unsigned i = 0; i < out.nBlock && bp + 80 < sizeof blk; ++i) bp += static_cast<size_t>(snprintf(blk + bp, sizeof blk - bp, "%s[%s]", i ? " " : "", out.block[i]));
        logd("ISF-CMP facts sel=%08x view=%08x name=\"%s\" id=%u mask=%x dice=%dd%d crit=%d/%d | R=\"%s\"(%d) L=\"%s\"(%d) A=\"%s\" | box=%d blk=%s",
             rawId, static_cast<unsigned>(viewed), in.viewed.name, in.viewed.id, in.viewed.slotMask, in.viewed.numDice, in.viewed.die, in.viewed.critThreat, in.viewed.critMult,
             in.right.item.name, static_cast<int>(in.right.state), in.left.item.name, static_cast<int>(in.left.state), in.armor.item.name, g_cmpShown ? 1 : 0, blk);
    }
    if (cmpLogOk(LogFacts)) {   // detail line: every viewed stat + all 7 equipped slots, so weapon/armor boxes can be audited offline (misc/datapads show why=notWeaponOrArmor and no box)
        const bool viewedLive = viewed && saferead::probeRead(2, reinterpret_cast<const void*>(viewed), 4);   // log-only call: never on a dead pointer
        if (viewed && !viewedLive) logd("ISF-CMP detail: viewed %08x unreadable, GetUpgradeType skipped", static_cast<unsigned>(viewed));
        const unsigned up = viewedLive ? (reinterpret_cast<GetUpgradeTypeFn>(FnGetUpgradeType)(static_cast<int>(viewed), 0) & 0xFF) : 0xFFu;
        char sl7[330]; sl7[0] = 0; size_t sp7 = 0;
        static const char sn[7] = {'R', 'L', 'A', 'H', 'G', 'I', 'B'};
        const itemcmp::SlotRead* sr[7] = {&in.right, &in.left, &in.armor, &in.head, &in.gauntlet, &in.implant, &in.belt};
        for (int i = 0; i < 7 && sp7 + 60 < sizeof sl7; ++i)
            sp7 += static_cast<size_t>(snprintf(sl7 + sp7, sizeof sl7 - sp7, "%c%d:%.14s/%dd%d/ac%d ", sn[i], static_cast<int>(sr[i]->state), sr[i]->item.name, sr[i]->item.numDice, sr[i]->item.die, sr[i]->item.baseAC));
        logd("ISF-CMP detail sel=%08x why=%s up=%u canEquip=%d viewOk=%d ac=%d maxDex=%d keen=%d critBonus=%d props=%u dropped=%u nBlk=%u nBand=%u | %s",
             rawId, itemcmp::whyName(out.why), up, in.canEquip ? 1 : 0, in.viewed.ok ? 1 : 0, in.viewed.baseAC, in.viewed.maxDex, in.viewed.keen ? 1 : 0, in.viewed.critBonus,
             in.viewed.nProps, in.viewed.dropped, out.nBlock, out.nBand, sl7);
    }
    if (factsOk) {   // property probe (gloves/implants/belts show no rows): every property row read for the viewed item and each equipped slot, one short line per row
        static const char who[8] = {'V', 'R', 'L', 'A', 'H', 'G', 'I', 'B'};
        const itemcmp::ItemFacts* pf[8] = {&in.viewed, &in.right.item, &in.left.item, &in.armor.item, &in.head.item, &in.gauntlet.item, &in.implant.item, &in.belt.item};
        const itemcmp::SlotState ps[8] = {itemcmp::SlotState::Item, in.right.state, in.left.state, in.armor.state, in.head.state, in.gauntlet.state, in.implant.state, in.belt.state};
        for (int w = 0; w < 8; ++w) {
            if (ps[w] != itemcmp::SlotState::Item) continue;
            const itemcmp::ItemFacts& f = *pf[w];
            if (!cmpLogOk(LogProps)) break;
            logd("ISF-CMP props %c n=%u dropped=%u ac=%d maxDex=%d name=\"%.30s\"", who[w], f.nProps, f.dropped, f.baseAC, f.maxDex, f.name);
            for (unsigned i = 0; i < f.nProps && i < 10; ++i) {
                if (!cmpLogOk(LogProps)) break;
                const itemcmp::PropRow& r = f.props[i];
                logd("ISF-CMP prop %c%u t=%u s=%u k=%d cv=%u ct=%u p1=%u \"%.56s\"", who[w], i, (unsigned)r.type, (unsigned)r.subtype, (int)r.known, (unsigned)r.costValue, (unsigned)r.costTable, (unsigned)r.param1, r.text);
            }
        }
    }
    if (cmpLogOk(out.nBand ? LogSel : LogEmpty)) {
        // slots in compare() order R L A H G I B: E empty, I item, U unreadable; "skipped" when no fill ran
        char slotTxt[8] = "skipped";
        if (creature && idOk) { for (int i = 0; i < 7; ++i) slotTxt[i] = sl[i]->state == itemcmp::SlotState::Unreadable ? 'U' : (sl[i]->state == itemcmp::SlotState::Empty ? 'E' : 'I'); slotTxt[7] = 0; }
        const uint32_t invPtr = creature ? rd<uint32_t>(static_cast<int>(creature), itemcmpread::Creature_Inv) : 0u;   // 0 = null/unreadable inventory (inv=0 with slots=UUUUUUU: inventory-level failure)
        logd("ISF-CMP sel=%08x creature=%08x facts=%d nBand=%u why=%s slots=%s vok=%d vm=%x inv=%08x box=%d nBlk=%u l1=\"%s\" l2=\"%s\"", rawId, creature, factsOk ? 1 : 0, out.nBand, factsOk ? itemcmp::whyName(out.why) : "nofacts", slotTxt, in.viewed.ok ? 1 : 0, in.viewed.slotMask, invPtr, g_cmpShown ? 1 : 0, out.nBlock, out.band[0], out.band[1]);
    }
}

// Once per populate: which creature the band will use, and whether panel+0x2D0 agrees (feeds the ISF-CMP1 verdict).
void cmpLogCreature(int panel) {
    if (!g_cfg.compare) return;
    if (cmpLogOk(LogCreature)) logd("ISF-CMP creature=%08x src=populate", (unsigned)eqprobe::g_cre);
    const int pc = rd<int>(panel, PanelCreatureOff, -1);   // own budget: a MISMATCH must never be dropped because populate lines used up the cap
    if (pc != -1 && pc != eqprobe::g_cre && cmpLogOk(LogMismatch)) {
        logd("ISF-CMP creature MISMATCH populate=%08x panel+2D0=%08x", (unsigned)eqprobe::g_cre, (unsigned)pc);
    }
}

// Credits layout (user): Use Item (list #4, gui [78,470,306,28]) is NOT modified; the credits label (#3) and value (#11) move into
// the gap right of it (gui x390-545) on its row. Done once at PANEL_BUILD from the live extents (ultrawide changes x/width only: scale = Use Item width / 306).
// Shape-gated: any control that is not where the vanilla layout puts it leaves everything untouched (false => box mode A).
bool cmpMoveCredits(uintptr_t panel, uintptr_t cap) {
    int sw = 0, sh = 0;
    if (!cmpScreen(&sw, &sh)) return false;
    const int arr = rd<int>(static_cast<int>(panel), guikit::Panel_CtlArray), cnt = rd<int>(static_cast<int>(panel), guikit::Panel_CtlCount);
    if (!arr || cnt <= 11) return false;
    const uintptr_t lab = static_cast<uintptr_t>(rd<int>(arr, 4 * 3)), btn = static_cast<uintptr_t>(rd<int>(arr, 4 * 4)), val = static_cast<uintptr_t>(rd<int>(arr, 4 * 11));
    int bx, by, bw, bh, lx, ly, lw, lh, vx, vy, vw, vh;
    if (!lab || !btn || !val || !guikit::getExtent(btn, &bx, &by, &bw, &bh) || !guikit::getExtent(lab, &lx, &ly, &lw, &lh) || !guikit::getExtent(val, &vx, &vy, &vw, &vh)) return false;
    auto gy = [&](int v) { return v * 600 / sh; };   // engine px -> gui units (y is never ultrawide-scaled)
    if (gy(by) < 466 || gy(by) > 474 || gy(bh) < 26 || gy(bh) > 30 || gy(ly) < 438 || gy(ly) > 446 || gy(vy) < 438 || gy(vy) > 446) {
        logd("ISF-CMP credits not moved: layout differs (btn y=%d h=%d, label y=%d, value y=%d, sh=%d)", by, bh, ly, vy, sh);
        return false;
    }
    auto gx = [&](int g) { return bx + static_cast<int>(static_cast<int64_t>(g - 78) * bw / 306); };   // gui x (800-wide layout) -> engine px
    // Use Item runs to the list's right edge (gui x396); the credits unit sits one column gap (14) to its right, same row.
    guikit::setRect(btn, bx, by, gx(396) - bx, bh);
    guikit::setRect(lab, gx(410), by, gx(553) - gx(410), bh);   // one teal frame around caption AND number
    guikit::setRect(val, gx(475), by, gx(541) - gx(475), bh);   // right-aligned by the engine; 12 gui units of right padding (frame ends x553: it overlapped the neighbouring frame at x565) (caption has 6 on the left; the engine adds none)
    if (cap) {
        guikit::setText(lab, "");                               // frame only; the caption below carries the word
        guikit::setRect(cap, gx(416), by, gx(475) - gx(416), bh);
        guikit::setFont(cap, "dialogfont16x16"); guikit::setAlign(cap, 17); guikit::setClickable(cap, false);
        guikit::setText(cap, "Credits"); guikit::setVisible(cap, true);
    }
    logf("ISF-CMP credits moved: use item w=%d, frame x=%d w=%d, number x=%d y=%d (panel %08x)", gx(396) - bx, gx(410), gx(553) - gx(410), gx(475), by, (unsigned)panel);
    return true;
}

// K2HUD_PANEL_BUILD (hud-host 0.6.0+): every panel's layout load, layout still open. Only the inventory panel gets a strip.
void KPATCH_CALL onPanelBuild(const K2HudContext* ctx, void*) {
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, panelVtbl) + sizeof(ctx->panelVtbl) || !ctx->panel) return;
    const uintptr_t vtbl = ctx->panelVtbl;
    const bool isInv = (vtbl == InventoryVtable);
    const bool isEquip = (vtbl == static_cast<uintptr_t>(gameaddr::EquipPanelVtable));
    if (!isInv && !isEquip) return;
    saferead::beginScope();
    loadIni(false);
    eqbox::onPanelChange(g_eqState, static_cast<uint32_t>(ctx->panelVtbl));
    g_strip = {};
    g_caption[0] = 0;   // a rebuilt panel has a fresh top label (vanilla text): forget what we wrote so captionTo's dedupe cannot skip the next write (banner, same panel address)
    g_stripStaleLogged = false;
    if (g_cmp.panel && g_cmp.panel != ctx->panel) g_cmpOther = g_cmp;   // keep the other panel's box (it is swapped back in on that panel's select)
    else if (g_cmpOther.panel == ctx->panel) g_cmpOther = CmpBox{};      // same panel rebuilt: its old box is stale
    g_want.redo = 0; g_want.show = false;   // review ISF-REV: a pending catch-up must not draw into the new box
    g_cmp = {}; g_cmpKeyValid = false; g_cmpShown = false; eqprobe::g_cre = 0; g_trueEq.reset();   // new panel: the populate-time creature belongs to the old one
    for (auto& n : g_cmpLogLines) n = 0;   // each new compare panel gets its own compare log budget (CmpLogCap unchanged)
    if (g_cfg.compare) {
        const uintptr_t cp = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
        CmpBox b = {};
        b.panel = ctx->panel;
        // equip: LBL_BAR5 is the portrait-notch bar (wrong shape); LBL_TITLE has an EMPTY border (equip_p.gui: no edge/corner/fill, so no frame is drawn).
        // LBL_CANTEQUIP carries the full rounded frame (16bet/16bct/fill_2bt, red); it is retinted below to the vanilla description-box teal.
        const char* frameTag = isInv ? "LBL_CREDITS" : "LBL_CANTEQUIP";
        const char* rowTag = isInv ? "LBL_FILTER" : "LBL_SLOTNAME";
        b.frame = stripLabel(cp, frameTag, "compare frame", &b.frameIdx, false);   // optional: the rows are readable without it
        if (isInv) {
            b.credCap = stripLabel(cp, "LBL_FILTER", "credits caption", &b.credCapIdx, false);   // optional: without it the vanilla caption stays
        } else {
            b.credCap = 0;
            b.credCapIdx = -1;
            b.credMoved = false;
        }
        b.hdr = stripLabel(cp, rowTag, "compare header", &b.hdrIdx, false);   // optional: without it the header keeps its own line
        bool ok = true;
        for (int i = 0; i < CmpRows && ok; ++i) {
            b.name[i] = stripLabel(cp, rowTag, "compare name", &b.nameIdx[i], false);
            b.val[i] = b.name[i] ? stripLabel(cp, rowTag, "compare value", &b.valIdx[i], false) : 0;
            ok = b.name[i] && b.val[i];
        }
        if (ok) {
            if (b.frame) guikit::setText(b.frame, "");   // frame only, no caption
            if (b.frame && !isInv) {   // LB_DESC / LBL_CREDITS colour (0.051, 0.349, 0.271) and their fill image
                const float teal[3] = {0.05098039f, 0.34901961f, 0.27058825f};
                // Fill: the description box fill (uibit_fill_2wt). NEVER the pooled imageFor() handle (the label dtor deletes its fill: review ISF-REV B1);
                // the raw engine loader returns a fresh wrapper this label owns, exactly like a template-built label's own fill. The clone's original
                // fill (fill_2bt) is dropped from the label (its wrapper leaks once per panel build, a few bytes).
                const bool okTint = guikit::setBorderTint(b.frame, teal);
                b.bgFill = true;
                bool okFill = false;
                if (guikit::g_gk.loadImage) {
                    if (void* own = guikit::g_gk.loadImage("uibit_fill_2wt")) okFill = guikit::setFill(b.frame, own, teal);
                }
                int dimOld = -1; guikit::getBorderDimension(b.frame, &dimOld);
                // LBL_CANTEQUIP draws 16 px black-edged art (16bet/16bct) with the fill inset by it; LB_DESC uses the thin white-line art (16wet/16wct) at
                // about 2 px. DIMENSION 0 means "use the corner image size" (0x415300), so the earlier 0 changed nothing: set the real 2 px and the LB_DESC art.
                bool okArt = false;
                if (guikit::g_gk.loadImage) {
                    void* cor = guikit::g_gk.loadImage("uibit_brdr_16wct"); void* edg = guikit::g_gk.loadImage("uibit_brdr_16wet");
                    okArt = cor && edg && guikit::setBorderArt(b.frame, cor, edg);
                }
                const bool okDim = okArt;   // DIMENSION stays 16: squeezed to 2 px the thin edge tile vanished (play 2026-11-11)
                logf("ISF-CMP equip frame recolour: tint=%d ownedFill=%d dimension=%d->2 art=%d ok=%d (LBL_CANTEQUIP clone -> teal)", okTint ? 1 : 0, okFill ? 1 : 0, dimOld, okArt ? 1 : 0, okDim ? 1 : 0);
            }
            if (isInv) {
                b.credMoved = cmpMoveCredits(cp, b.credCap);
                if (!b.credMoved && b.credCap) { guikit::setText(b.credCap, ""); guikit::setVisible(b.credCap, false); }   // layout differs: vanilla credits row stays untouched
            }
            for (int i = 0; i < CmpRows; ++i)
                for (uintptr_t c : {b.name[i], b.val[i]}) { guikit::setAlpha(c, 0.0f); guikit::setFont(c, "fnt_d16x16"); guikit::setAlign(c, 9); guikit::setClickable(c, false); guikit::setVisible(c, false); }
            if (b.hdr) { guikit::setFont(b.hdr, "fnt_d16x16"); guikit::setAlign(b.hdr, 9); guikit::setClickable(b.hdr, false); guikit::setVisible(b.hdr, false); }
            g_cmp = b;
            logf("ISF-CMP box built for panel %08x (frame #%d, rows #%d..#%d)", (unsigned)ctx->panel, b.frame ? b.frameIdx : -1, b.nameIdx[0], b.valIdx[CmpRows - 1]);
        } else logf("ISF-CMP WARN box labels not built for panel %08x, compare box off", (unsigned)ctx->panel);
    }
    if (ctx->panelVtbl != InventoryVtable) return;
    if (!g_cfg.strip) { logf("STRIP: off (strip=0), panel %08x untouched", (unsigned)ctx->panel); return; }
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    Strip s = {};
    s.panel = ctx->panel;
    s.frame = stripLabel(panel, "LBL_MOULDING3", "frame", nullptr, false);   // optional: the strip is readable without it
    s.line1 = stripLabel(panel, "LBL_FILTER", "line 1", &s.idx1, true);
    s.line2 = s.line1 ? stripLabel(panel, "LBL_FILTER", "line 2", &s.idx2, true) : 0;
    for (int i = 0; i < 4 && s.line2; ++i) s.slot[i] = stripLabel(panel, "LBL_FILTER", "state slot", &s.idxSlot[i], true);
    if (!s.line1 || !s.line2 || !s.slot[0] || !s.slot[1] || !s.slot[2] || !s.slot[3]) {
        logf("STRIP: WARN not built for panel %08x, old caption behaviour", (unsigned)ctx->panel);
        return;
    }
    g_strip = s;
    g_hoverPrev = -1;
    logf("STRIP: built for panel %08x (frame=%d line1 #%d line2 #%d)", (unsigned)ctx->panel, s.frame ? 1 : 0, s.idx1, s.idx2);
    stripPlace(ctx->panel);
}

void startPopulate(int panel, int ebp) {
    if (panel != g_panel) {
        logd("panel instance %08x (was %08x): reset extra filter/caption", panel, g_panel);
        g_panel = panel; g_extra = isf::X_None; strcpy(g_caption, "Filter");
        eqprobe::g_cre = 0; g_cmpKeyValid = false;   // S10-a: a new panel never inherits the previous creature (rewritten below)
    }
    normaliseState(panel);
    static int s_lastOpenLogged = 0;   // C3: one INFO line per panel instance, not per populate
    if (panel != s_lastOpenLogged) {
        s_lastOpenLogged = panel;
        logd("panel open: panel=%08x mode=%d extra=%d cycle=%d mask=%x", panel, filterMode(), g_extra, g_cfg.cycle, g_cfg.mask);
    }
    g_inPop = true; g_n = 0; g_lastIdx = -1; g_selSet = false; g_seen = g_dropped = g_direct = 0; g_nTinted = 0;
    g_bufTicks = g_nameTicks = 0;
    g_popEbp = ebp; g_popCounter0 = rd<int>(ebp, F_Counter);
    eqprobe::g_nMap = 0; eqprobe::g_cre = rd<int>(ebp, F_Creature);   // probe: rows are rebuilt; remember the shown creature
    cmpHide(panel); cmpLogCreature(panel);   // S10: the old band belongs to the previous row set
    logd("populate: panel=%08x mode=%d extra=%d creature=%08x ebp=%08x rows-so-far=%d", panel, filterMode(), g_extra,
                        rd<int>(ebp, F_Creature), ebp, g_popCounter0);
}

} // namespace

// H6e (input 0x2F, prev) and H6f (input 0x30, next): CSWGuiInGameInventory::HandleInputEvent 0x8a8880 steps the engine mode inline
// and calls SetNextFilter. The 9-byte windows (PUSH reg / MOV ECX,[EBP-0x2C] / CALL SetNextFilter) are skipped, so the hook makes
// that call itself with the mode from isf::bumperNext (our New / Usable / Upgradeable states join the bumper ring). Which physical
// bumper is 0x2F / 0x30 is unverified: every press logs the input code, so the play test settles it.
namespace {
void bumperStep(int panel, int computedMode, int dir) {
    saferead::beginScope();
    loadIni(false);
    if (panel != g_panel) {   // same reset as SortFilterTabClick (WI-148)
        g_panel = panel; g_extra = isf::X_None; strcpy(g_caption, "Filter");
        g_n = 0; g_inPop = false;
    }
    const int mode = filterMode();
    const isf::Bump next = isf::bumperNext(g_cfg.cycle, mode, g_extra, computedMode, g_cfg.mask, dir);
    logd("bumper: input=0x%02X %s panel=%08x cycle=%d mask=%x mode=%d extra=%d computed=%d -> mode=%d extra=%d", dir > 0 ? 0x30 : 0x2F,
         dir > 0 ? "next" : "prev", panel, g_cfg.cycle, g_cfg.mask, mode, g_extra, computedMode, next.mode, next.extra);
    if (!panel) return;     // unreadable frame: SetNextFilter would dereference ECX = 0, so skip the whole step
    g_extra = next.extra;   // SetNextFilter only stores the mode and flags a deferred repopulate
    logd("bumper set: cur=%d computed=%d result=%d extra=%d pass-through=%d", mode, computedMode, next.mode, next.extra, next.mode == computedMode ? 1 : 0);
    setFilterMode(panel, next.mode, "bumper");
    char text[isf::kCaptionBytes] = "Filter";
    isf::State st;
    if (g_cfg.cycle && isf::stateOf(next.mode, next.extra, &st)) isf::captionWithCount(st, g_cfg.mask, -1, text, sizeof text);
    captionTo(panel, text);
}
} // namespace

// ECX / EDX = the engine's computed mode (pushed as the SetNextFilter argument), [ebp-0x2C] = panel.
extern "C" void __cdecl SortFilterBumperPrev(int computedMode, int panel) { bumperStep(panel, computedMode, -1); }
extern "C" void __cdecl SortFilterBumperNext(int computedMode, int panel) { bumperStep(panel, computedMode, +1); }

// K2HUD_FRAME (hud-host): strip hover sound and clicks. Pure decisions in StripClick.h; this reads the cursor, plays the hover
// sound and calls the same state change as the tab click. K2HUD_FRAME fires on every PopulateMenus call (HudHost.h:13).
guikit::ClickEdge g_clickEdge;
void KPATCH_CALL onStripFrame(const K2HudContext*, void*) {
    const bool press = guikit::clickEdge(g_clickEdge);   // every frame, so the edge state tracks the button
    cmpCatchUp();   // before the strip early-returns: runs on every frame while a select is pending
    if (!g_strip.panel || !g_cfg.strip || !stripLive(g_strip.panel)) { g_hoverPrev = -1; return; }
    int cx = 0, cy = 0;
    if (!guikit::cursorEngine(&cx, &cy)) return;
    const int panel = g_strip.panel;
    const int cur = filterMode();   // read before en[] (ISF 0.7.1 sec 4); reused by the click paths below
    const bool en[5] = {stripSlotEnabled(0, cur), stripSlotEnabled(1, cur), stripSlotEnabled(2, cur), stripSlotEnabled(3, cur), true};
    const bool fg = guikit::g_gk.gameForeground && guikit::g_gk.gameForeground();   // no hover sound or click while another window has focus
    const stripclk::Out o = stripclk::step(g_hoverPrev, press, fg, cx, cy, g_strip.slotR, g_strip.sortR, en);
    if (o.hoverEnter) guikit::playHoverSound();
    if (o.click < 0) return;
    if (o.click == 4) {   // sort text: next order; setFilterMode with the SAME mode flags the deferred repopulate that re-sorts
        g_cfg.order = static_cast<decltype(g_cfg.order)>(isf::nextSortOrder(static_cast<int>(g_cfg.order)));
        logf("STRIP click: sort -> order %d (session only; the ini value returns on the next panel build)", static_cast<int>(g_cfg.order));
        setFilterMode(panel, cur, "strip-sort");
        return;
    }
    isf::State tmp;
    if (!isf::stateOf(cur, g_extra, &tmp)) return;   // unreadable mode: nothing to do
    isf::State next = static_cast<isf::State>(o.click);   // slot 0..3 = S_All, S_New, S_Usable, S_Upgrade
    if (next == tmp) {   // the selected slot again: cycle to the next state that applies here, as the All tab does (ISF-CK5)
        next = isf::nextSubFilter(tmp, cur, g_cfg.mask);
        if (next == tmp) return;   // no other state applies on this category
        logf("STRIP cycle: selected slot -> state %d (engine mode %d, extra=%d)", static_cast<int>(next), cur, isf::extraOf(next));
    }
    const isf::Bump w = isf::writeFor(next, cur);   // category tab stays put; New on a category is X_New
    g_extra = w.extra;
    setFilterMode(panel, w.mode, "strip-state");
    char text[isf::kCaptionBytes];
    isf::captionWithCount(next, g_cfg.mask, -1, text, sizeof text);
    captionTo(panel, text);
    logf("STRIP click: slot %d -> state %d (engine mode %d -> %d, extra=%d)", o.click, static_cast<int>(next), cur, w.mode, w.extra);
}

// H6a. ECX = panel, [esp+4] = tab control. Returns 1 when the click is consumed.
extern "C" int __cdecl SortFilterTabClick(int panel, int ctrl) {
    g_hud.poll();
    saferead::beginScope();
    loadIni(false);
    int clicked = rd<int>(ctrl, TabClickedOff, -1);
    int mode = filterMode();
    logf("tab click: panel=%08x ctrl=%08x clicked=%d mode=%d extra=%d", panel, ctrl, clicked, mode, g_extra);
    if (panel != g_panel) {   // new panel instance: drop any rows still buffered for the old one (WI-148)
        g_panel = panel; g_extra = isf::X_None; strcpy(g_caption, "Filter");
        g_n = 0; g_inPop = false;
    }
    if (g_cfg.cycle) {
        isf::State next;
        if (isf::onTabClick(clicked, mode, g_extra, g_cfg.mask, &next)) {
            g_extra = isf::extraOf(next);   // SetNextFilter only stores the mode and flags a deferred repopulate
            setFilterMode(panel, isf::engineModeOf(next), "tab-cycle");
            char text[isf::kCaptionBytes];
            isf::captionWithCount(next, g_cfg.mask, -1, text, sizeof text);
            captionTo(panel, text);
            logf("tab click: cycle -> state %d (engine mode %d, extra=%d)", static_cast<int>(next), isf::engineModeOf(next), g_extra);
            return 1;
        }
        if (isf::tabChangeClearsExtra(clicked, mode)) {   // any tab change resets the stacked filter; a click on All while on All keeps it
            if (g_extra != isf::X_None) logf("tab click: tab change resets filter (extra was %d)", g_extra);
            g_extra = isf::X_None;
        }
    }
    if (!g_cfg.cycle && (g_extra != isf::X_None || strcmp(g_caption, "Filter"))) { g_extra = isf::X_None; captionTo(panel, "Filter"); }   // cycle on: the tab change above resets the filter
    return 0;
}

// H6b. EBP = PopulateItemListBox frame. Buffers the item (or creates it at once when the buffer is full).
extern "C" void __cdecl SortFilterBufferRow(int ebp) {
    saferead::beginScope();
    BufTimer bufTimer;
    g_hud.poll();
    loadIni(false);
    int panel = rd<int>(ebp, F_Panel), item = rd<int>(ebp, F_Item), cobj = rd<int>(ebp, F_Cobj);
    int creature = rd<int>(ebp, F_Creature);
    int idx = rd<int>(ebp, F_Loop, -1);
    int counter = rd<int>(ebp, F_Counter);
    if (!panel || !item) { logf("buffer: bad frame panel=%08x item=%08x (row skipped)", panel, item); return; }
    // A new populate starts when the loop index does not advance, the row counter went backwards (new list), or the frame moved.
    if (!g_inPop || idx <= g_lastIdx || counter < g_lastCounter || ebp != g_popEbp) {
        if (g_inPop && g_n) {
            const char* reason = idx <= g_lastIdx ? "idx" : counter < g_lastCounter ? "counter" : "ebp";
            logf("buffer: stale buffer of %d rows discarded (reason=%s loop index %d last %d counter %d last %d ebp %08x was %08x)",
                 g_n, reason, idx, g_lastIdx, counter, g_lastCounter, ebp, g_popEbp);
        }
        startPopulate(panel, ebp);
        g_trueEq.reset();   // the equip list repopulated (real equip/unequip or slot switch): the next select re-snapshots what is equipped now
    }
    g_lastIdx = idx;
    g_lastCounter = counter;
    ++g_seen;
    int id = rd<int>(item, ItemIdOff);
    const bool isNew = ((rd<unsigned>(item, ItemFlagsOff) >> 7) & 1) != 0;

    isf::State gateSt = isf::S_All;
    const bool extraOn = g_extra != isf::X_None && g_cfg.cycle && isf::stateOf(filterMode(), g_extra, &gateSt) && gateSt != isf::S_New;   // stacks on category tabs
    if (extraOn && g_extra == isf::X_Upgrade && !itemHasOpenSlot(item)) {
        ++g_dropped;
        logt("  drop (no open upgrade slot): item=%08x id=%08x base=%d", item, id, rd<int>(item, ItemBaseOff));
        return;
    }
    if (g_extra == isf::X_New && filterMode() >= 6 && filterMode() <= 11 && !isNew) {   // category New sub-filter: only new items
        ++g_dropped;
        return;
    }

    // Equip test for the shown character (tint + sort + Usable filter); "activatable" (0x77b670) only widens Usable.
    bool needUsable = g_cfg.sort || g_cfg.tint || KP_ENABLED(chMain, KPLOG_TRACE) || (extraOn && g_extra == isf::X_Usable);
    bool equippable = false, equipOk = true, activatable = false;
    unsigned slotMask = 0;
    if (needUsable) {
        equipOk = canEquip(creature, item, &equippable, &slotMask);
        activatable = reinterpret_cast<IsUsableFn>(FnIsItemUsable)(cobj, item, 0, 0xFF) != 0;
    }
    bool usable = !needUsable || isf::usableForCycle(equipOk, activatable);
    if (extraOn && g_extra == isf::X_Usable && !usable) {
        ++g_dropped;
        logt("  drop (not usable): item=%08x id=%08x base=%d equippable=%d", item, id, rd<int>(item, ItemBaseOff), equippable);
        return;
    }
    isf::Entry e;
    e.item = item; e.id = id; e.orig = idx;
    e.isNew = isNew;
    e.usable = usable; e.baseItem = rd<int>(item, ItemBaseOff);
    e.tint = needUsable && isf::tintRow(equippable, equipOk);
    const bool catOn = g_cfg.sort && g_cfg.order == isf::SO_Category;   // one GetUpgradeType call per row, only for the category order (WI-154)
    if (catOn)
        e.category = isf::categoryOf(reinterpret_cast<GetUpgradeTypeFn>(FnGetUpgradeType)(item, 0) & 0xFF, slotMask, activatable, e.baseItem);
    // Names are not read here: SortFilterFlush reads them for tied rows only (WI-146), so e.name stays "" until then.

    // Reselect bookkeeping that the skipped vanilla code did; the row index is only known at flush time.
    bool matched = (rd<int>(panel, PanelSelectedIdOff) == id);
    if (matched) { g_selSet = true; g_selId = id; wr32(panel + PanelSelectedIdOff, NoSelection); }

    logt("  row: idx=%d item=%08x id=%08x base=%d new=%d equippable=%d canEquip=%d activatable=%d usable=%d tint=%d category=%d%s", idx, item, id,
             e.baseItem, e.isNew, equippable, equipOk, activatable, e.usable, e.tint, catOn ? (int)e.category : -1, matched ? " (selected)" : "");

    if (g_n >= BufCap) {   // overflow (never expected): emit what is buffered in arrival order, then keep going
        logf("buffer: overflow at %d rows, emitting unsorted", g_n);
        ++g_direct;
        emitBuffered(panel, cobj, ebp);
    }
    g_buf[g_n++] = e;
}

// H6c. Runs once per populate, after the backpack loop and before the finalise call.
extern "C" void __cdecl SortFilterFlush(int ebp) {
    saferead::beginScope();
    int panel = rd<int>(ebp, F_Panel), cobj = rd<int>(ebp, F_Cobj);
    if (!g_inPop) {
        g_nTinted = 0;   // the rows at those indices may now be equipped rows
        normaliseState(panel);
        captionCountTo(panel);   // empty filter: g_n is 0 here
        stripRefresh(panel, 0, 0);
        cmpHide(panel);
        logd("flush: no backpack rows this populate (mode=%d extra=%d)", filterMode(), g_extra);
        return;
    }
    normaliseState(panel);
    if (g_cfg.sort) {
        static LONGLONG freq = 0;   // QPC frequency, read once
        if (!freq) { LARGE_INTEGER f; QueryPerformanceFrequency(&f); freq = f.QuadPart; }
        // Names only for rows tied on the sort order's key (WI-146; category: new, usable, category, base item; base: new, usable, base
        // item; name: new, usable), read here instead of per row in populate. The item
        // pointers are the ones emitBuffered turns into rows right after this, so they are still valid. Language fetched once.
        LARGE_INTEGER n0, n1;
        QueryPerformanceCounter(&n0);
        const int lang = clientLanguage();
        const int nNames = isf::fillTiedNames(g_buf, g_n, [&](isf::Entry& e) { readItemName(e.item, lang, e.name, sizeof e.name); }, g_cfg.order);
        QueryPerformanceCounter(&n1);
        g_nameTicks += n1.QuadPart - n0.QuadPart;
        LARGE_INTEGER t0, t1;
        QueryPerformanceCounter(&t0);
        isf::sortEntries(g_buf, g_n, g_cfg.order);
        QueryPerformanceCounter(&t1);
        if (freq > 0)
            logd("sort timing: sortEntries rows=%d took %u us; buffer %u us (names %u us) names=%d", g_n,
                 (unsigned)((t1.QuadPart - t0.QuadPart) * 1000000 / freq),
                 (unsigned)(g_bufTicks * 1000000 / freq), (unsigned)(g_nameTicks * 1000000 / freq), nNames);
    }
    if (KP_ENABLED(chMain, KPLOG_DEBUG)) {
        int nTint = 0, nUsable = 0;
        for (int i = 0; i < g_n; ++i) { nTint += g_buf[i].tint; nUsable += g_buf[i].usable; }
        logd("flush: panel=%08x creature=%08x rows=%d seen=%d dropped=%d direct=%d usable=%d tint-candidates=%d sort=%d tint=%d extra=%d selected=%d counter=%d",
             panel, rd<int>(ebp, F_Creature), g_n, g_seen, g_dropped, g_direct, nUsable, nTint, g_cfg.sort, g_cfg.tint, g_extra, g_selSet,
             rd<int>(ebp, F_Counter));
        if (KP_ENABLED(chMain, KPLOG_TRACE)) {
            char line[512]; int p = 0;
            for (int i = 0; i < g_n && p < (int)sizeof line - 24; ++i) p += snprintf(line + p, sizeof line - p, " %d", g_buf[i].orig);
            logt("  creation order (vanilla loop index):%s", line);
        }
    }
    captionCountTo(panel);   // g_n is final; emitBuffered below empties the buffer
    stripRefresh(panel, g_seen - g_dropped, g_seen);
    cmpHide(panel);
    emitBuffered(panel, cobj, ebp);
    g_inPop = false; g_selSet = false;
}

// H6d. EBP = frame of the row-select handler 0x8a83d0 at its epilogue ([ebp+8] = row). For a New item that handler
// clears the New bit and re-runs SetItem (0x8b1cc0), which rewrites the row colours; put our tint back.
extern "C" void __cdecl SortFilterRowSelect(int ebp) {
    saferead::beginScope();
    eqprobe::onSelect(ebp);   // EQ-P1 probe: logs before the early returns so usable rows and the equip screen are seen
    cmpOnSelect(ebp);         // S10: compare band, BEFORE the tint early return (untinted rows and tint=0 must still refresh it)
    if (!g_cfg.tint || !g_nTinted) return;
    int row = rd<int>(ebp, F_SelectRowArg);
    if (!row || !isTinted(row)) return;
    // 0x8a83d0 is also the equipment screen's row handler, and rows freed on inventory close (0x8a7520 deletes them
    // and zeroes the +0x24B0 entries) can be reallocated there. Act only for our panel with the row still live in it.
    int panel = rd<int>(ebp, F_SelectPanel);
    if (panel != g_tintPanel) return;
    int arr = rd<int>(panel, PanelRowArrayOff), cnt = rd<int>(panel, PanelRowCountOff);
    bool live = false;
    for (int i = 0; arr && i < cnt && i < 4096 && !live; ++i) live = rd<int>(arr, 4 * i) == row;
    if (!live) return;
    reinterpret_cast<SetCanUseFn>(FnSetCanUse)(row, 2);
    logt("row select: re-tint row=%08x item id=%08x", row, rd<int>(row, 0x1D0));
}

// Called by the patcher core after load (new cores only). KpLog_Init moves logging onto the shared core: inventory-sort-filter_log.txt,
// level from kplog.ini [inventory-sort-filter] (the page's Debug logging row). DllMain ran before the core existed, so its banner went to the
// fallback file; repeat it here so the core log starts with the loaded / config lines. Old core: no call, everything stays in the fallback.
// Registers the mod-config page now when mod-config is already loaded (it binds its entry list when the Options panel is first built).
extern "C" void __cdecl KPatchInit(const KPatchApi* api) {
    KpLog_Init(api);
    if (KpLog_HasCore()) {
        logf("inventory-sort-filter 0.9.0-dev loaded (KpLog core); pid=%lu", (unsigned long)GetCurrentProcessId());
        logf("ini path: %s", IniName);
        loadIni(true);
    }
    mcPageInit(api);
    mcPagePoll();
    g_hud.init(api, "inventory-sort-filter");
    g_hud.onConnect = [](bool ok, const char* why) { logf("HUD k2.hud %s: %s (PANEL_BUILD needs hud-host 0.6.0+)", ok ? "ok" : "unavailable", why); };
    g_hud.add(K2HUD_PANEL_BUILD, 300, onPanelBuild);
    g_hud.add(K2HUD_FRAME, 310, onStripFrame);   // strip hover sound + clicks (0.7.0-dev)
    g_hud.connect();
}

// Keeps the previous session as <log>.1 (one previous, replaced each time), like the other KpLog mods. Runs at attach before the first logf.
// Rotates only when the log is older than 60 s or larger than 256 KB: a launcher+game pair attaches a few seconds apart and must stay in one file.
namespace {
// Kept session logs: the previous session is moved to a timestamped name (never overwritten), then only the newest
// kKeptSessionLogs timestamped logs stay. The stamp is yyyymmdd_hhmmss, so the largest name is the newest log.
constexpr int kKeptSessionLogs = 5;
constexpr const char* kSessionLogPrefix = "inventory_sort_filter_log_";

// Deletes every kSessionLogPrefix*.txt in dir except the kKeptSessionLogs largest names. Pass 1 finds the newest names
// into a fixed window (descending); pass 2 deletes whatever is outside the window.
void pruneSessionLogs(const char* dir) {
    char pattern[MAX_PATH + 80];
    snprintf(pattern, sizeof pattern, "%s\\%s*.txt", dir, kSessionLogPrefix);
    char newest[kKeptSessionLogs][MAX_PATH];
    int kept = 0;
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (kept == kKeptSessionLogs && strcmp(fd.cFileName, newest[kKeptSessionLogs - 1]) <= 0) continue;
        int at = kept < kKeptSessionLogs ? kept++ : kKeptSessionLogs - 1;   // a full window drops its smallest name
        while (at > 0 && strcmp(fd.cFileName, newest[at - 1]) > 0) {
            snprintf(newest[at], sizeof newest[at], "%s", newest[at - 1]);
            --at;
        }
        snprintf(newest[at], sizeof newest[at], "%s", fd.cFileName);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        bool keep = false;
        for (int i = 0; i < kept; ++i) if (strcmp(fd.cFileName, newest[i]) == 0) keep = true;
        if (keep) continue;
        char victim[MAX_PATH + 80];
        snprintf(victim, sizeof victim, "%s\\%s", dir, fd.cFileName);
        DeleteFileA(victim);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
}

// Runs once at attach, before the first logf. The KpLog fallback sink opens its file lazily on the first write, so the
// current log is not held open here. A log that is more than 60 s old or over 256 KB is moved to a timestamped name
// (no overwrite); a successful move is followed by the prune.
void rotateFallbackLog(const char* cwd) {
    char logPath[MAX_PATH + 40];
    snprintf(logPath, sizeof logPath, "%s\\inventory_sort_filter_log.txt", cwd);   // the file the KpLog sink opens (cwd = game folder)
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (!GetFileAttributesExA(logPath, GetFileExInfoStandard, &fa)) return;   // no log yet
    const unsigned long long size = (static_cast<unsigned long long>(fa.nFileSizeHigh) << 32) | fa.nFileSizeLow;
    const unsigned long long written = (static_cast<unsigned long long>(fa.ftLastWriteTime.dwHighDateTime) << 32) | fa.ftLastWriteTime.dwLowDateTime;
    FILETIME nowFt; GetSystemTimeAsFileTime(&nowFt);
    const unsigned long long now = (static_cast<unsigned long long>(nowFt.dwHighDateTime) << 32) | nowFt.dwLowDateTime;
    const unsigned long long ticksPerSec = 10000000ULL;   // FILETIME ticks are 100 ns
    const bool stale = now > written && now - written > 60 * ticksPerSec;
    const bool big = size > 256ULL * 1024;
    if (!(stale || big)) return;
    SYSTEMTIME st; GetLocalTime(&st);
    char movedPath[MAX_PATH + 80];
    snprintf(movedPath, sizeof movedPath, "%s\\%s%04u%02u%02u_%02u%02u%02u.txt", cwd, kSessionLogPrefix,
             static_cast<unsigned>(st.wYear), static_cast<unsigned>(st.wMonth), static_cast<unsigned>(st.wDay),
             static_cast<unsigned>(st.wHour), static_cast<unsigned>(st.wMinute), static_cast<unsigned>(st.wSecond));
    if (MoveFileExA(logPath, movedPath, 0)) pruneSessionLogs(cwd);   // no MOVEFILE_REPLACE_EXISTING: an existing session is never overwritten
}
} // namespace

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        char cwd[MAX_PATH] = "?"; GetCurrentDirectoryA(MAX_PATH, cwd);
        rotateFallbackLog(cwd);
        logf("inventory-sort-filter 0.9.0-dev loaded; cwd=%s pid=%lu", cwd, (unsigned long)GetCurrentProcessId());
        // Bare names go to the Windows directory (Wine: C:\\windows), so anchor the INI to the game folder.
        if (GetCurrentDirectoryA(MAX_PATH, cwd) > 0) snprintf(IniName, sizeof IniName, "%s\\inventory_sort_filter.ini", cwd);
        logf("ini path: %s", IniName);
        loadIni(true);
    }
    return TRUE;
}
