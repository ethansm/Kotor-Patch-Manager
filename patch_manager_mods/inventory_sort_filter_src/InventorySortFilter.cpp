// Inventory Sort & Filter (Steam Aspyr build 6A522E71.../LAA 4AB72FC1...).
// Sorts the backpack rows of the inventory screen, tints unusable items and adds New / Usable / Upgradeable
// filters by re-clicking the active All tab. Research trail and every address:
// patch_manager_mods/11_inventory_sort_filter.md (local-setup/KOTOR-II worktree).
//
// Hooks (cdecl detours):
//   H6a 0x008a8cf0 SortFilterTabClick   tab-click handler prologue; returns 1 = click consumed (consumed_exit_address)
//   H6b 0x008a7e22 SortFilterBufferRow  PopulateItemListBox backpack loop; replaces "reselect + CALL CreateItemEntry"
//   H6c 0x008a7e76 SortFilterFlush      PopulateItemListBox loop exit; sorts the buffer and creates the rows
//
// Stages are controlled from inventory_sort_filter.ini (re-read at most once a second, no restart needed):
//   all flags 0 = stage 0 (logging only; rows are still created through the buffer, in vanilla order).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "../_shared/SafeRead.h"
#include "SortLogic.h"

namespace {

// ---- engine addresses (doc 11 section 2) ----
constexpr int GuiGlobal = 0x00A1B4A4;         // *(GuiGlobal)+4 = CClientExoAppInternal-ish holder; ECX for FnGetGuiInGame
constexpr int FnGetGuiInGame = 0x0073F750;    // thiscall(holder) -> GuiInGame*
constexpr int FilterModeOff = 0xFC1;          // byte on GuiInGame
constexpr int FnCreateItemEntry = 0x008A75F0; // thiscall(panel; cobj, int* counter, list*, item, slot, equipped) RET 0x18
constexpr int FnIsItemUsable = 0x0077B670;    // thiscall(cobj; item, slot, mask) RET 0xC
constexpr int FnSetCanUse = 0x008B1D00;       // thiscall(row; byte mode) RET 4
constexpr int FnSetNextFilter = 0x008A8B30;   // thiscall(panel; int mode) RET 4
constexpr int FnGetUpgradeType = 0x006076D0;  // thiscall(item; int unused) RET 4, returns byte in AL
constexpr int FnExoCtor = 0x00733570, FnExoDtor = 0x00733780, FnTextSet = 0x00416E30;

constexpr int TextVtable = 0x009876B4;
constexpr int LabelTextOff = 0xD8, TextRendererOff = 0x14, TextStringSubOff = 0x18;

constexpr int PanelFilterLabelOff = 0x16B8;   // LBL_FILTER (static caption "Filter")
constexpr int PanelRowArrayOff = 0x24B0, PanelRowCountOff = 0x24B4, PanelSelectedIdOff = 0x24C8;
constexpr int ItemIdOff = 0x14, ItemFlagsOff = 0x2C8, ItemBaseOff = 0x0C, ItemUpgradeSlotsOff = 0x2F4;
constexpr int TabClickedOff = 0x5C;
constexpr int NoSelection = 0x7F000000;

// PopulateItemListBox 0x8a7ba0 frame (EBP-relative)
constexpr int F_Panel = -0x84, F_Item = -0x18, F_Cobj = -0x38, F_Counter = -0x3C, F_List = -0x2C,
              F_Loop = -0x14, F_SelRow = -0x20;

using CExoCtor = void*(__thiscall*)(void*, const char*);
using CExoDtor = void(__thiscall*)(void*);
using TextSetFn = void(__thiscall*)(void*, void*);
using GetGuiFn = int(__thiscall*)(int);
using CreateEntryFn = void(__thiscall*)(int, int, int*, void*, int, int, int);
using IsUsableFn = int(__thiscall*)(int, int, int, int);
using SetCanUseFn = void(__thiscall*)(int, int);
using SetNextFilterFn = void(__thiscall*)(int, int);
using GetUpgradeTypeFn = unsigned(__thiscall*)(int, int);

// ---- log ----
int g_logLines = 0;
constexpr int MaxLogLines = 8000;
void logf(const char* fmt, ...) {
    if (g_logLines >= MaxLogLines) return;
    FILE* f = fopen("inventory_sort_filter_log.txt", "a");
    if (!f) return;
    ++g_logLines;
    va_list ap; va_start(ap, fmt); vfprintf(f, fmt, ap); va_end(ap);
    fputc('\n', f); fclose(f);
}

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
        logf("  dump %s +%03x:%s", tag, i, line);
    }
}

// ---- config ----
struct Config {
    bool sort = false, tint = false, cycle = false, log = true, logItems = true;
    unsigned mask = 0;
};
Config g_cfg;
DWORD g_lastIni = 0;
char IniName[MAX_PATH + 40] = "inventory_sort_filter.ini";   // made absolute in DllMain: a bare name is searched in the Windows dir

void loadIni(bool force) {
    DWORD now = GetTickCount();
    if (!force && g_lastIni && now - g_lastIni < 1000) return;
    g_lastIni = now ? now : 1;
    Config c;
    c.sort = GetPrivateProfileIntA("Inventory", "sort", 0, IniName) != 0;
    c.tint = GetPrivateProfileIntA("Inventory", "tint", 0, IniName) != 0;
    c.cycle = GetPrivateProfileIntA("Inventory", "cycle", 0, IniName) != 0;
    c.log = GetPrivateProfileIntA("Inventory", "log", 1, IniName) != 0;
    c.logItems = GetPrivateProfileIntA("Inventory", "log_items", 1, IniName) != 0;
    char cs[128] = {};
    GetPrivateProfileStringA("Inventory", "cycle_states", "new,usable,upgrade", cs, sizeof cs, IniName);
    c.mask = isf::parseCycleStates(cs);
    if (c.sort != g_cfg.sort || c.tint != g_cfg.tint || c.cycle != g_cfg.cycle || c.mask != g_cfg.mask || c.log != g_cfg.log || force)
        logf("config: sort=%d tint=%d cycle=%d cycle_mask=%x log=%d log_items=%d", c.sort, c.tint, c.cycle, c.mask, c.log, c.logItems);
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
bool g_upgrade = false;            // Upgradeable filter active (engine mode 0 + this flag)
int g_panel = 0;                   // panel instance the state belongs to
char g_caption[24] = "Filter";     // caption last written by us
bool g_labelDumped = false;

void captionTo(int panel, const char* text) {
    if (!strcmp(g_caption, text)) return;
    if (g_cfg.log && !g_labelDumped) { g_labelDumped = true; dumpMem("LBL_FILTER", panel + PanelFilterLabelOff, 0x120); }
    setLabelText(panel, text);
    strncpy(g_caption, text, sizeof g_caption - 1); g_caption[sizeof g_caption - 1] = 0;
}

constexpr int BufCap = 1024;
isf::Entry g_buf[BufCap];
int g_n = 0;
bool g_inPop = false;
int g_lastIdx = -1;
bool g_selSet = false;
int g_selId = 0;
int g_seen = 0, g_dropped = 0, g_direct = 0;

bool itemHasOpenSlot(int item) {
    unsigned type = reinterpret_cast<GetUpgradeTypeFn>(FnGetUpgradeType)(item, 0) & 0xFF;
    int slots[6] = {0, 0, 0, 0, 0, 0};
    int n = isf::upgradeSlotCount(static_cast<int>(type));
    for (int i = 0; i < n; ++i) slots[i] = rd<int>(item, ItemUpgradeSlotsOff + 4 * i, 0);
    return isf::hasOpenSlot(static_cast<int>(type), slots);
}

// Creates one backpack row exactly like the vanilla call site, then tints it when unusable.
void createRow(int panel, int cobj, int ebp, int item, bool usable) {
    int* counter = reinterpret_cast<int*>(ebp + F_Counter);
    int before = *counter;
    reinterpret_cast<CreateEntryFn>(FnCreateItemEntry)(panel, cobj, counter, reinterpret_cast<void*>(ebp + F_List), item, 0, 0);
    int after = *counter;
    if (after != before + 1) logf("row: counter %d -> %d (expected +1) for item %08x", before, after, item);
    if (!usable && g_cfg.tint && after == before + 1) {
        int arr = rd<int>(panel, PanelRowArrayOff), cnt = rd<int>(panel, PanelRowCountOff);
        int row = (arr && before >= 0 && before < cnt) ? rd<int>(arr, 4 * before) : 0;
        if (row) reinterpret_cast<SetCanUseFn>(FnSetCanUse)(row, 2);
        else logf("row: tint skipped, no row at index %d (arr=%08x cnt=%d)", before, arr, cnt);
    }
}

// Rebuilds the Upgradeable flag/caption from the engine mode, so changes that bypass our tab handler
// (controller bumpers -> SetNextFilter, panel reopen, cycle switched off in the INI) cannot leave stale state.
void normaliseState(int panel) {
    int mode = filterMode();
    if (g_upgrade && (mode != 0 || !g_cfg.cycle)) g_upgrade = false;
    if (g_cfg.cycle) {
        isf::State st;
        captionTo(panel, isf::stateOf(mode, g_upgrade, &st) ? isf::captionOf(st) : "Filter");
    } else if (strcmp(g_caption, "Filter")) {
        captionTo(panel, "Filter");
    }
}

// Creates the buffered rows in their current order and empties the buffer.
void emitBuffered(int panel, int cobj, int ebp) {
    for (int i = 0; i < g_n; ++i) {
        const isf::Entry& e = g_buf[i];
        if (g_selSet && e.id == g_selId) wr32(ebp + F_SelRow, rd<int>(ebp, F_Counter));
        createRow(panel, cobj, ebp, e.item, e.usable);
    }
    g_n = 0;
}

void startPopulate(int panel, int ebp) {
    if (panel != g_panel) {
        logf("panel instance %08x (was %08x): reset upgrade flag/caption", panel, g_panel);
        g_panel = panel; g_upgrade = false; strcpy(g_caption, "Filter");
    }
    normaliseState(panel);
    g_inPop = true; g_n = 0; g_lastIdx = -1; g_selSet = false; g_seen = g_dropped = g_direct = 0;
    if (g_cfg.log) logf("populate: panel=%08x mode=%d upgrade=%d ebp=%08x rows-so-far=%d", panel, filterMode(), g_upgrade, ebp, rd<int>(ebp, F_Counter));
}

} // namespace

// H6a. ECX = panel, [esp+4] = tab control. Returns 1 when the click is consumed.
extern "C" int __cdecl SortFilterTabClick(int panel, int ctrl) {
    saferead::beginScope();
    loadIni(false);
    int clicked = rd<int>(ctrl, TabClickedOff, -1);
    int mode = filterMode();
    if (g_cfg.log) logf("tab click: panel=%08x ctrl=%08x clicked=%d mode=%d upgrade=%d", panel, ctrl, clicked, mode, g_upgrade);
    if (panel != g_panel) { g_panel = panel; g_upgrade = false; strcpy(g_caption, "Filter"); }
    if (g_cfg.cycle) {
        isf::State next;
        if (isf::onTabClick(clicked, mode, g_upgrade, g_cfg.mask, &next)) {
            g_upgrade = (next == isf::S_Upgrade);
            reinterpret_cast<SetNextFilterFn>(FnSetNextFilter)(panel, isf::engineModeOf(next));
            captionTo(panel, isf::captionOf(next));
            if (g_cfg.log) logf("tab click: cycle -> state %d (engine mode %d, upgrade=%d)", static_cast<int>(next), isf::engineModeOf(next), g_upgrade);
            return 1;
        }
    }
    if (g_upgrade || strcmp(g_caption, "Filter")) { g_upgrade = false; captionTo(panel, "Filter"); }
    return 0;
}

// H6b. EBP = PopulateItemListBox frame. Buffers the item (or creates it at once when the buffer is full).
extern "C" void __cdecl SortFilterBufferRow(int ebp) {
    saferead::beginScope();
    loadIni(false);
    int panel = rd<int>(ebp, F_Panel), item = rd<int>(ebp, F_Item), cobj = rd<int>(ebp, F_Cobj);
    int idx = rd<int>(ebp, F_Loop, -1);
    if (!panel || !item) { logf("buffer: bad frame panel=%08x item=%08x (row skipped)", panel, item); return; }
    if (!g_inPop || idx <= g_lastIdx) {
        if (g_inPop && g_n) logf("buffer: stale buffer of %d rows discarded (loop index %d <= %d)", g_n, idx, g_lastIdx);
        startPopulate(panel, ebp);
    }
    g_lastIdx = idx;
    ++g_seen;
    int id = rd<int>(item, ItemIdOff);

    if (g_upgrade && g_cfg.cycle && filterMode() == 0 && !itemHasOpenSlot(item)) {
        ++g_dropped;
        if (g_cfg.log && g_cfg.logItems) logf("  drop (no open upgrade slot): item=%08x id=%08x base=%d", item, id, rd<int>(item, ItemBaseOff));
        return;
    }

    bool needUsable = g_cfg.sort || g_cfg.tint || g_cfg.logItems;
    bool usable = true;
    if (needUsable) usable = reinterpret_cast<IsUsableFn>(FnIsItemUsable)(cobj, item, 0, 0xFF) != 0;
    isf::Entry e;
    e.item = item; e.id = id; e.orig = idx;
    e.isNew = ((rd<unsigned>(item, ItemFlagsOff) >> 7) & 1) != 0;
    e.usable = usable; e.baseItem = rd<int>(item, ItemBaseOff);

    // Reselect bookkeeping that the skipped vanilla code did; the row index is only known at flush time.
    bool matched = (rd<int>(panel, PanelSelectedIdOff) == id);
    if (matched) { g_selSet = true; g_selId = id; wr32(panel + PanelSelectedIdOff, NoSelection); }

    if (g_cfg.log && g_cfg.logItems)
        logf("  row: idx=%d item=%08x id=%08x base=%d new=%d usable=%d%s", idx, item, id, e.baseItem, e.isNew, e.usable, matched ? " (selected)" : "");

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
    if (!g_inPop) { normaliseState(panel); if (g_cfg.log) logf("flush: no backpack rows this populate (mode=%d)", filterMode()); return; }
    normaliseState(panel);
    if (g_cfg.sort) isf::sortEntries(g_buf, g_n);
    if (g_cfg.log) {
        logf("flush: panel=%08x rows=%d seen=%d dropped=%d direct=%d sort=%d tint=%d selected=%d counter=%d",
             panel, g_n, g_seen, g_dropped, g_direct, g_cfg.sort, g_cfg.tint, g_selSet, rd<int>(ebp, F_Counter));
        if (g_cfg.logItems) {
            char line[512]; int p = 0;
            for (int i = 0; i < g_n && p < (int)sizeof line - 24; ++i) p += snprintf(line + p, sizeof line - p, " %d", g_buf[i].orig);
            logf("  creation order (vanilla loop index):%s", line);
        }
    }
    emitBuffered(panel, cobj, ebp);
    g_inPop = false; g_selSet = false;
}

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        char cwd[MAX_PATH] = "?"; GetCurrentDirectoryA(MAX_PATH, cwd);
        logf("inventory-sort-filter 0.1.0-dev loaded; cwd=%s pid=%lu", cwd, (unsigned long)GetCurrentProcessId());
        // Bare names go to the Windows directory (Wine: C:\\windows), so anchor the INI to the game folder.
        if (GetCurrentDirectoryA(MAX_PATH, cwd) > 0) snprintf(IniName, sizeof IniName, "%s\\inventory_sort_filter.ini", cwd);
        logf("ini path: %s", IniName);
        loadIni(true);
    }
    return TRUE;
}
