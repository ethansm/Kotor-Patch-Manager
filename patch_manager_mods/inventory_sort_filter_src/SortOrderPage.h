// SortOrderPage.h -- Options -> Mod Configuration page of Inventory Sort & Filter (WI-155, doc 46 declarative layer). Header-only, no Win32.
// Two list rows. Debug logging (kplog.ini [inventory-sort-filter] level, below). Sort order, bound to [Inventory] sort_order of inventory_sort_filter.ini: the stored value is the choice index (0 Category, 1 Base item,
// 2 Name, 3 Vanilla) = the digits isf::parseSortOrder accepts (SortLogic.h, WI-154). ModConfigDecl.h reads the start value from the file when
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
desc = "Order of the inventory backpack. Takes effect when the inventory list next refreshes."

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
id = "log_level"
kind = "list"
label = "Debug logging"
desc = "Off: startup and errors. On: each refresh and tab click. Verbose: every item too."
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

// Page io for both rows: the level row maps the kplog.ini word to its index (read) and back (write); sort_order goes through readSortOrder.
using RawIniWrite = bool (*)(const char* path, const char* section, const char* key, const char* val);
inline bool readPageValue(RawIniRead raw, const char* path, const char* section, const char* key, char* out, int cap) {
    if (!key || strcmp(key, kLogLevelKey)) return readSortOrder(raw, path, section, key, out, cap);
    if (!raw || cap < 2 || !raw(path, section, key, out, cap)) return false;
    snprintf(out, cap, "%d", logLevelIndex(out));
    return true;
}
inline bool writePageValue(RawIniWrite raw, const char* path, const char* section, const char* key, const char* val) {
    if (!raw) return false;
    if (key && !strcmp(key, kLogLevelKey)) return raw(path, section, key, logLevelWord(atoi(val)));
    return raw(path, section, key, val);
}

}  // namespace isf
