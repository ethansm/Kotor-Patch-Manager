// Inventory Sort & Filter (Steam Aspyr build 6A522E71...).
// Sorts the backpack rows of the inventory screen, tints items the shown character cannot equip and adds
// New / Usable / Upgradeable filters by re-clicking the active All tab. Research trail and every address:
// patch_manager_mods/11_inventory_sort_filter.md (local-setup/KOTOR-II worktree).
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

#include "SafeRead.h"   // bare, like GuiKit.h / GameAddr.h: one copy (Common) for every header
#include "SortLogic.h"
#include "GameAddr.h"
#include "KpLog.h"
#include "ModConfigApi.h"
#include "ModConfigDecl.h"     // Options -> Mod Configuration page (isf::kPageSchema)
#include "ProviderClient.h"
#include "SortOrderPage.h"
#include "HudHost.h"           // K2HUD_PANEL_BUILD: the strip labels are cloned while the inventory layout is open (doc 11 VU4)
#include "GuiKit.h"
#include "PreviewCanvas.h"     // options-page preview list (VU7a): labels listed MUI_NOFREE, freed in pvDestroy
#include "IsfPreview.h"       // pure preview model (isfprev::build / buildPrims)

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
// appended; sessions split on the "inventory-sort-filter <ver> loaded" line. At attach the previous session moves to <name>.1 (rotateFallbackLog). logf forwards to the KpLog fallback sink (own line cap).
KP_CHANNEL(chMain, "main");

// Only stores the path and the level (no I/O): safe from a static initialiser. Same file name as before (tooling reads it).
const bool g_fallbackSet = (KpLog_SetFallbackFile("inventory_sort_filter_log.txt"), KpLog_SetFallbackLevel(KPLOG_INFO), true);

void logAt(int lvl, const char* fmt, va_list ap) {
    if (!KP_ENABLED(chMain, lvl)) return;   // kplog.ini [inventory-sort-filter] level (Debug logging row) decides; no format cost when off
    char buf[480];
    vsnprintf(buf, sizeof buf, fmt, ap);
    KP_LOGV(chMain, lvl, "%s", buf);
}
// INFO: startup, config, page, errors. DEBUG (Debug logging On): per-populate / tab-click / flush lines. TRACE (Verbose): per-item lines.
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

void dumpMem(const char* tag, int addr, int bytes) {
    for (int i = 0; i < bytes; i += 32) {
        char line[160]; int p = 0;
        for (int j = i; j < i + 32 && j < bytes; j += 4) {
            int v = 0;
            if (saferead::readAt<int>(3, static_cast<uintptr_t>(addr), j, &v)) p += snprintf(line + p, sizeof line - p, " %08x", v);
            else p += snprintf(line + p, sizeof line - p, " ????????");
        }
        logd("  dump %s +%03x:%s", tag, i, line);
    }
}

// ---- config ----
struct Config {
    bool sort = false, tint = false, cycle = false;
    unsigned mask = 0;
    isf::SortOrder order = isf::SO_Category;   // [Inventory] sort_order (WI-154); vanilla also forces sort off
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
    if (c.sort != g_cfg.sort || c.tint != g_cfg.tint || c.cycle != g_cfg.cycle || c.mask != g_cfg.mask || c.order != g_cfg.order ||
        c.strip != g_cfg.strip || force)
        logf("config: sort=%d tint=%d cycle=%d cycle_mask=%x sort_order=%d strip=%d", c.sort, c.tint, c.cycle, c.mask,
             (int)c.order, c.strip);
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
bool g_labelDumped = false;

// VU4 strip: labels cloned at K2HUD_PANEL_BUILD for one inventory panel instance. No destroy API: the panel list owns them, so a
// new inventory panel just replaces the handles. idx1/idx2 = their panel-list index; stripLive re-checks the list before every write.
struct Strip { int panel; uintptr_t line1, line2, frame; int idx1, idx2; bool placed, warned; int replaced, lbX, lbY, lbW, lbH; };
Strip g_strip = {};
k2hud::Client g_hud;   // hud-host k2.hud@1; connect() in KPatchInit, poll() from the populate / tab hooks retries
bool g_stripStaleLogged = false;

bool stripLive(int panel) {
    if (!panel || g_strip.panel != panel || !g_strip.line1 || !g_strip.line2) return false;
    const int arr = rd<int>(panel, guikit::Panel_CtlArray), cnt = rd<int>(panel, guikit::Panel_CtlCount);
    return arr && g_strip.idx2 < cnt && rd<int>(arr, 4 * g_strip.idx1) == static_cast<int>(g_strip.line1) &&
           rd<int>(arr, 4 * g_strip.idx2) == static_cast<int>(g_strip.line2);
}

void captionTo(int panel, const char* text) {
    if (stripLive(panel)) text = "Filter";   // the strip shows the state; the top bar stays vanilla (owner, v4b)
    if (!strcmp(g_caption, text)) return;
    if (KP_ENABLED(chMain, KPLOG_DEBUG) && !g_labelDumped) { g_labelDumped = true; dumpMem("LBL_FILTER", panel + PanelFilterLabelOff, 0x120); }
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

// Creates one backpack row exactly like the vanilla call site, then tints it when the shown character cannot equip it.
void createRow(int panel, int cobj, int ebp, int item, bool tint) {
    int* counter = reinterpret_cast<int*>(ebp + F_Counter);
    int before = *counter;
    reinterpret_cast<CreateEntryFn>(FnCreateItemEntry)(panel, cobj, counter, reinterpret_cast<void*>(ebp + F_List), item, 0, 0);
    int after = *counter;
    if (after != before + 1) logf("row: counter %d -> %d (expected +1) for item %08x", before, after, item);
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

// Rebuilds the extra filter/caption from the engine mode at every populate, so changes that bypass our tab handler
// (controller bumpers -> SetNextFilter, cycle switched off in the INI) cannot leave stale state. The engine keeps
// the filter mode across panel close/reopen and party switches (S0 run 2), and so do we: while the mode stays 0
// the extra filter is kept and its caption re-applied.
void normaliseState(int panel) {
    int mode = filterMode();
    if (g_extra != isf::X_None && (mode != 0 || !g_cfg.cycle)) g_extra = isf::X_None;
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
    logf("STRIP: LB_ITEMS %d,%d %dx%d -> %d,%d %dx%d (strip %d px)", x, y, w, h, ax, ay, aw, ah, sh);
    if (g_strip.frame) { guikit::setRect(g_strip.frame, x, y, w, sh); guikit::setVisible(g_strip.frame, true); guikit::setClickable(g_strip.frame, false); }
    guikit::setRect(g_strip.line1, x + in, y + h * 14 / 877, w - 2 * in, lh);
    guikit::setRect(g_strip.line2, x + in, y + h * 70 / 877, w - 2 * in, lh);
    for (uintptr_t c : {g_strip.line1, g_strip.line2}) { guikit::setVisible(c, true); guikit::setClickable(c, false); }
    g_strip.placed = true;
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
    isf::State st;
    if (!isf::stateOf(filterMode(), g_extra, &st)) st = isf::S_Count;   // weapon/armour/... tab: nothing in brackets
    char l1[isf::kStripBytes], l2[isf::kStripBytes];
    isf::stripLine1(st, g_cfg.cycle ? g_cfg.mask : 0u, l1, sizeof l1);
    isf::stripLine2(g_cfg.order, g_cfg.sort, shown, offered, l2, sizeof l2);
    guikit::setText(g_strip.line1, l1);
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

// K2HUD_PANEL_BUILD (hud-host 0.6.0+): every panel's layout load, layout still open. Only the inventory panel gets a strip.
void KPATCH_CALL onPanelBuild(const K2HudContext* ctx, void*) {
    if (!ctx || ctx->struct_size < offsetof(K2HudContext, panelVtbl) + sizeof(ctx->panelVtbl) || !ctx->panel) return;
    if (ctx->panelVtbl != InventoryVtable) return;
    saferead::beginScope();
    loadIni(false);
    g_strip = {};
    g_stripStaleLogged = false;
    if (!g_cfg.strip) { logf("STRIP: off (strip=0), panel %08x untouched", (unsigned)ctx->panel); return; }
    const uintptr_t panel = static_cast<uintptr_t>(static_cast<uint32_t>(ctx->panel));
    Strip s = {};
    s.panel = ctx->panel;
    s.frame = stripLabel(panel, "LBL_MOULDING3", "frame", nullptr, false);   // optional: the strip is readable without it
    s.line1 = stripLabel(panel, "LBL_FILTER", "line 1", &s.idx1, true);
    s.line2 = s.line1 ? stripLabel(panel, "LBL_FILTER", "line 2", &s.idx2, true) : 0;
    if (!s.line1 || !s.line2) {
        logf("STRIP: WARN not built for panel %08x, old caption behaviour", (unsigned)ctx->panel);
        return;
    }
    g_strip = s;
    logf("STRIP: built for panel %08x (frame=%d line1 #%d line2 #%d)", (unsigned)ctx->panel, s.frame ? 1 : 0, s.idx1, s.idx2);
    stripPlace(ctx->panel);
}

void startPopulate(int panel, int ebp) {
    if (panel != g_panel) {
        logf("panel instance %08x (was %08x): reset extra filter/caption", panel, g_panel);
        g_panel = panel; g_extra = isf::X_None; strcpy(g_caption, "Filter");
    }
    normaliseState(panel);
    g_inPop = true; g_n = 0; g_lastIdx = -1; g_selSet = false; g_seen = g_dropped = g_direct = 0; g_nTinted = 0;
    g_bufTicks = g_nameTicks = 0;
    g_popEbp = ebp; g_popCounter0 = rd<int>(ebp, F_Counter);
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
    logf("bumper: input=0x%02X %s panel=%08x cycle=%d mask=%x mode=%d extra=%d computed=%d -> mode=%d extra=%d", dir > 0 ? 0x30 : 0x2F,
         dir > 0 ? "next" : "prev", panel, g_cfg.cycle, g_cfg.mask, mode, g_extra, computedMode, next.mode, next.extra);   // every press, ungated
    if (!panel) return;     // unreadable frame: SetNextFilter would dereference ECX = 0, so skip the whole step
    g_extra = next.extra;   // SetNextFilter only stores the mode and flags a deferred repopulate
    reinterpret_cast<SetNextFilterFn>(FnSetNextFilter)(panel, next.mode);
    char text[isf::kCaptionBytes] = "Filter";
    isf::State st;
    if (g_cfg.cycle && isf::stateOf(next.mode, next.extra, &st)) isf::captionWithCount(st, g_cfg.mask, -1, text, sizeof text);
    captionTo(panel, text);
}
} // namespace

// ECX / EDX = the engine's computed mode (pushed as the SetNextFilter argument), [ebp-0x2C] = panel.
extern "C" void __cdecl SortFilterBumperPrev(int computedMode, int panel) { bumperStep(panel, computedMode, -1); }
extern "C" void __cdecl SortFilterBumperNext(int computedMode, int panel) { bumperStep(panel, computedMode, +1); }

// H6a. ECX = panel, [esp+4] = tab control. Returns 1 when the click is consumed.
extern "C" int __cdecl SortFilterTabClick(int panel, int ctrl) {
    g_hud.poll();
    saferead::beginScope();
    loadIni(false);
    int clicked = rd<int>(ctrl, TabClickedOff, -1);
    int mode = filterMode();
    logd("tab click: panel=%08x ctrl=%08x clicked=%d mode=%d extra=%d", panel, ctrl, clicked, mode, g_extra);
    if (panel != g_panel) {   // new panel instance: drop any rows still buffered for the old one (WI-148)
        g_panel = panel; g_extra = isf::X_None; strcpy(g_caption, "Filter");
        g_n = 0; g_inPop = false;
    }
    if (g_cfg.cycle) {
        isf::State next;
        if (isf::onTabClick(clicked, mode, g_extra, g_cfg.mask, &next)) {
            g_extra = isf::extraOf(next);   // SetNextFilter only stores the mode and flags a deferred repopulate
            reinterpret_cast<SetNextFilterFn>(FnSetNextFilter)(panel, isf::engineModeOf(next));
            char text[isf::kCaptionBytes];
            isf::captionWithCount(next, g_cfg.mask, -1, text, sizeof text);
            captionTo(panel, text);
            logd("tab click: cycle -> state %d (engine mode %d, extra=%d)", static_cast<int>(next), isf::engineModeOf(next), g_extra);
            return 1;
        }
    }
    if (g_extra != isf::X_None || strcmp(g_caption, "Filter")) { g_extra = isf::X_None; captionTo(panel, "Filter"); }
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
    }
    g_lastIdx = idx;
    g_lastCounter = counter;
    ++g_seen;
    int id = rd<int>(item, ItemIdOff);

    bool extraOn = g_extra != isf::X_None && g_cfg.cycle && filterMode() == 0;
    if (extraOn && g_extra == isf::X_Upgrade && !itemHasOpenSlot(item)) {
        ++g_dropped;
        logt("  drop (no open upgrade slot): item=%08x id=%08x base=%d", item, id, rd<int>(item, ItemBaseOff));
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
    e.isNew = ((rd<unsigned>(item, ItemFlagsOff) >> 7) & 1) != 0;
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
    emitBuffered(panel, cobj, ebp);
    g_inPop = false; g_selSet = false;
}

// H6d. EBP = frame of the row-select handler 0x8a83d0 at its epilogue ([ebp+8] = row). For a New item that handler
// clears the New bit and re-runs SetItem (0x8b1cc0), which rewrites the row colours; put our tint back.
extern "C" void __cdecl SortFilterRowSelect(int ebp) {
    saferead::beginScope();
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
        logf("inventory-sort-filter 0.5.0-dev loaded (KpLog core); pid=%lu", (unsigned long)GetCurrentProcessId());
        logf("ini path: %s", IniName);
        loadIni(true);
    }
    mcPageInit(api);
    mcPagePoll();
    g_hud.init(api, "inventory-sort-filter");
    g_hud.onConnect = [](bool ok, const char* why) { logf("HUD k2.hud %s: %s (PANEL_BUILD needs hud-host 0.6.0+)", ok ? "ok" : "unavailable", why); };
    g_hud.add(K2HUD_PANEL_BUILD, 300, onPanelBuild);
    g_hud.connect();
}

// Keeps the previous session as <log>.1 (one previous, replaced each time), like the other KpLog mods. Runs at attach before the first logf.
// Rotates only when the log is older than 60 s or larger than 256 KB: a launcher+game pair attaches a few seconds apart and must stay in one file.
namespace {
void rotateFallbackLog(const char* cwd) {
    char logPath[MAX_PATH + 40], prevPath[MAX_PATH + 48];
    snprintf(logPath, sizeof logPath, "%s\\inventory_sort_filter_log.txt", cwd);   // the file the KpLog sink opens (cwd = game folder)
    snprintf(prevPath, sizeof prevPath, "%s.1", logPath);
    WIN32_FILE_ATTRIBUTE_DATA fa;
    if (!GetFileAttributesExA(logPath, GetFileExInfoStandard, &fa)) return;   // no log yet
    const unsigned long long size = (static_cast<unsigned long long>(fa.nFileSizeHigh) << 32) | fa.nFileSizeLow;
    const unsigned long long written = (static_cast<unsigned long long>(fa.ftLastWriteTime.dwHighDateTime) << 32) | fa.ftLastWriteTime.dwLowDateTime;
    FILETIME nowFt; GetSystemTimeAsFileTime(&nowFt);
    const unsigned long long now = (static_cast<unsigned long long>(nowFt.dwHighDateTime) << 32) | nowFt.dwLowDateTime;
    const unsigned long long ticksPerSec = 10000000ULL;   // FILETIME ticks are 100 ns
    const bool stale = now > written && now - written > 60 * ticksPerSec;
    const bool big = size > 256ULL * 1024;
    if (stale || big) MoveFileExA(logPath, prevPath, MOVEFILE_REPLACE_EXISTING);
}
} // namespace

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        char cwd[MAX_PATH] = "?"; GetCurrentDirectoryA(MAX_PATH, cwd);
        rotateFallbackLog(cwd);
        logf("inventory-sort-filter 0.5.0-dev loaded; cwd=%s pid=%lu", cwd, (unsigned long)GetCurrentProcessId());
        // Bare names go to the Windows directory (Wine: C:\\windows), so anchor the INI to the game folder.
        if (GetCurrentDirectoryA(MAX_PATH, cwd) > 0) snprintf(IniName, sizeof IniName, "%s\\inventory_sort_filter.ini", cwd);
        logf("ini path: %s", IniName);
        loadIni(true);
    }
    return TRUE;
}
