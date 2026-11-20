// SortOrderPage.h -- Options -> Mod Configuration page of Inventory Sort & Filter (WI-155, doc 46 declarative layer). Header-only, no Win32.
// Nine rows, in page order: sort_order, tint, cycle, cyc_new / cyc_usable / cyc_upgrade (the filter-cycle states), strip, banner_filter, log_level (kplog.ini level).
// Sort order, bound to [Inventory] sort_order of inventory_sort_filter.ini: the stored value is the choice index (0 Category, 1 Base item,
// 2 Name, 3 Vanilla) = the digits isf::parseSortOrder accepts (SortLogic.h, WI-154); [Inventory] sort (0 = Vanilla) is written with it. ModConfigDecl.h reads the start value from the file when
// the page is built and writes every accepted change back through IniWrite.h (section-aware, keeps the other keys and comments). The start
// value goes through isf::readSortOrder, so a hand-edited word (sort_order=name) or an out-of-range digit shows what loadIni really uses.
// Host-tested by common_contract_test.cpp, which parses this very string.
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SortLogic.h"

namespace isf {

constexpr const char* kPageId = "inventory-sort-filter";

constexpr const char* kPageSchema = R"TOML(
[page]
name = "Inventory Sort & Filter"
title = "Inventory Sort & Filter"
desc = "Inventory list behaviour. Changes apply when the list next refreshes."

[[row]]
id = "sort_order"
kind = "list"
label = "Sort order"
desc = "Category, Base item, Name, or Vanilla (the game's own order)."
default = 0
choices = ["Category", "Base item", "Name", "Vanilla"]
ini = "inventory_sort_filter.ini"
section = "Inventory"
key = "sort_order"

[[row]]
id = "tint"
kind = "toggle"
label = "Mark unusable items"
desc = "Items the shown character cannot equip get the red-orange colours."
default = 1
ini = "inventory_sort_filter.ini"
section = "Inventory"
key = "tint"

[[row]]
id = "cycle"
kind = "toggle"
label = "Filter cycle"
desc = "Re-click the All tab or use the bumpers to cycle the filters."
default = 1
ini = "inventory_sort_filter.ini"
section = "Inventory"
key = "cycle"

[[row]]
id = "cyc_new"
kind = "toggle"
label = "  New"
desc = "Include the New state in the filter cycle."
default = 1
ini = "inventory_sort_filter.ini"
section = "Inventory"
key = "cycle_states.new"

[[row]]
id = "cyc_usable"
kind = "toggle"
label = "  Can Equip"
desc = "Include the Can Equip state in the filter cycle."
default = 1
ini = "inventory_sort_filter.ini"
section = "Inventory"
key = "cycle_states.usable"

[[row]]
id = "cyc_upgrade"
kind = "toggle"
label = "  Upgradeable"
desc = "Include the Upgradeable state in the filter cycle."
default = 1
ini = "inventory_sort_filter.ini"
section = "Inventory"
key = "cycle_states.upgrade"

[[row]]
id = "strip"
kind = "toggle"
label = "Status strip above the list"
desc = "Filter, sort and count line. Applies the next time the inventory opens."
default = 1
ini = "inventory_sort_filter.ini"
section = "Inventory"
key = "strip"

[[row]]
id = "banner_filter"
kind = "toggle"
label = "Top banner names the filter"
desc = "The Filter line shows the active filter, e.g. Filter: New. Needs Filter cycle."
default = 1
ini = "inventory_sort_filter.ini"
section = "Inventory"
key = "banner_shows_filter"

[[row]]
id = "compare"
kind = "toggle"
label = "Equip compare under the item"
desc = "Under the description, this item against the equipped one. Applies when the inventory reopens."
default = 1
ini = "inventory_sort_filter.ini"
section = "Inventory"
key = "compare"

[[row]]
id = "log_level"
kind = "list"
label = "Debug logging"
desc = "Off: startup, errors, clicks. On: also each refresh. Verbose: every item too."
default = 0
choices = ["Off", "On", "Verbose"]
ini = "kplog.ini"
section = "inventory-sort-filter"
key = "level"
)TOML";

// Debug logging row (KpLog core): bound to kplog.ini [inventory-sort-filter] level, which the core re-reads within a second. The row stores
// the choice index; the file holds the level word the core parses (info / debug / trace). Off = info: startup, config, errors; On = debug:
// per-populate and tab-click lines; Verbose = trace: per-item lines. A hand-edited err / warn / 0..2 shows Off, 3 On, 4+ Verbose.
constexpr const char* kLogLevelKey = "level";
inline int logLevelIndex(const char* v) {
    while (*v == ' ' || *v == '\t') ++v;
    char w[8] = {};
    for (int k = 0; k < 7 && v[k] && v[k] != ' ' && v[k] != '\t' && v[k] != '\r' && v[k] != '\n'; ++k)
        w[k] = (v[k] >= 'A' && v[k] <= 'Z') ? static_cast<char>(v[k] - 'A' + 'a') : v[k];
    if (!strcmp(w, "debug") || !strcmp(w, "3")) return 1;
    if (!strcmp(w, "trace") || (w[0] >= '4' && w[0] <= '9' && !w[1])) return 2;
    return 0;
}
inline const char* logLevelWord(int index) { return index >= 2 ? "trace" : index == 1 ? "debug" : "info"; }

// Start-value reader for the page: raw INI read, then the value is replaced by the digit of isf::parseSortOrder(value), exactly the order
// loadIni applies (words, digits 0..3, case and spaces; anything else is Category), so the page never shows a row the mod does not use.
using RawIniRead = bool (*)(const char* path, const char* section, const char* key, char* out, int cap);
inline bool readSortOrder(RawIniRead raw, const char* path, const char* section, const char* key, char* out, int cap) {
    if (!raw || cap < 2 || !raw(path, section, key, out, cap)) return false;
    snprintf(out, cap, "%d", static_cast<int>(parseSortOrder(out)));
    return true;
}

// Page io: the level row maps the kplog.ini word to its index (read) and back (write). sort_order goes through readSortOrder; [Inventory] sort = 0
// (the sort switch off, Vanilla) makes the shown order Vanilla, and its write sets sort (0 for Vanilla, else 1). cycle_states.<name> reads one bit of the
// cycle list and writes it back in canonical order; the write merges into the current list through rd (the same reader), so a cycle write without rd is refused.
// Other keys (tint, cycle, strip) pass through raw.
using RawIniWrite = bool (*)(const char* path, const char* section, const char* key, const char* val);
constexpr const char* kSortOrderKey = "sort_order";
constexpr const char* kSortKey = "sort";
constexpr const char* kCycleStatesKey = "cycle_states";
constexpr const char* kCycleStatesPrefix = "cycle_states.";
constexpr const char* kCycleStatesDefault = "new,usable,upgrade";   // the default InventorySortFilter.cpp reads when cycle_states is absent
inline bool isCycleKey(const char* key) { return key && !strncmp(key, kCycleStatesPrefix, strlen(kCycleStatesPrefix)); }
// Suffix of cycle_states.<name> -> its State bit (SortLogic.h:112); 0 for an unknown name.
inline unsigned cycleBitOf(const char* name) { return !strcmp(name, "new") ? 1u << S_New : !strcmp(name, "usable") ? 1u << S_Usable : !strcmp(name, "upgrade") ? 1u << S_Upgrade : 0u; }
// Canonical list for a mask: new, usable, upgrade in that order, comma-joined; the empty mask gives "".
inline void cycleStatesText(unsigned mask, char* out, int cap) {
    static const char* const names[3] = {"new", "usable", "upgrade"};
    static const unsigned bits[3] = {1u << S_New, 1u << S_Usable, 1u << S_Upgrade};
    if (cap < 1) return;
    out[0] = 0;
    for (int k = 0; k < 3; ++k)
        if (mask & bits[k]) snprintf(out + strlen(out), cap - strlen(out), "%s%s", out[0] ? "," : "", names[k]);
}
// Current filter-cycle mask: [Inventory] cycle_states through the raw reader, or the default list when the key is absent.
inline unsigned readCycleMask(RawIniRead raw, const char* path, const char* section) {
    char cs[64];
    if (!raw || !raw(path, section, kCycleStatesKey, cs, sizeof cs)) snprintf(cs, sizeof cs, "%s", kCycleStatesDefault);
    return parseCycleStates(cs);
}
inline bool readPageValue(RawIniRead raw, const char* path, const char* section, const char* key, char* out, int cap) {
    if (!raw || !key) return false;
    if (!strcmp(key, kSortOrderKey)) {
        const bool have = readSortOrder(raw, path, section, key, out, cap);
        char sv[16];   // sort = 0 is Vanilla: the switch is off and the order is not used (also when sort_order itself is absent)
        if (cap >= 2 && raw(path, section, kSortKey, sv, sizeof sv) && atoi(sv) == 0) { snprintf(out, cap, "3"); return true; }
        return have;
    }
    if (!strcmp(key, kLogLevelKey)) {
        if (cap < 2 || !raw(path, section, key, out, cap)) return false;
        snprintf(out, cap, "%d", logLevelIndex(out));
        return true;
    }
    if (isCycleKey(key)) {
        const unsigned bit = cycleBitOf(key + strlen(kCycleStatesPrefix));
        if (!bit || cap < 2) return false;
        snprintf(out, cap, "%d", (readCycleMask(raw, path, section) & bit) ? 1 : 0);
        return true;
    }
    return raw(path, section, key, out, cap);
}
inline bool writePageValue(RawIniWrite raw, const char* path, const char* section, const char* key, const char* val, RawIniRead rd = nullptr) {
    if (!raw || !key || !val) return false;
    if (!strcmp(key, kSortOrderKey)) {   // one change, two keys: sort_order, then the sort switch (3 = Vanilla = sort 0)
        const bool a = raw(path, section, key, val);
        const bool b = raw(path, section, kSortKey, atoi(val) == 3 ? "0" : "1");
        return a && b;
    }
    if (!strcmp(key, kLogLevelKey)) return raw(path, section, key, logLevelWord(atoi(val)));
    if (isCycleKey(key)) {
        const unsigned bit = cycleBitOf(key + strlen(kCycleStatesPrefix));
        if (!bit || !rd) return false;   // unknown state name, or no reader to merge the bit into the current list
        const unsigned mask = readCycleMask(rd, path, section);
        char text[32];
        cycleStatesText(atoi(val) != 0 ? (mask | bit) : (mask & ~bit), text, sizeof text);
        return raw(path, section, kCycleStatesKey, text);
    }
    return raw(path, section, key, val);
}

}  // namespace isf
