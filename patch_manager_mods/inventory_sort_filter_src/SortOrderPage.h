// SortOrderPage.h -- Options -> Mod Configuration page of Inventory Sort & Filter (WI-155, doc 46 declarative layer). Header-only, no Win32.
// One list row bound to [Inventory] sort_order of inventory_sort_filter.ini: the stored value is the choice index (0 Category, 1 Base item,
// 2 Name, 3 Vanilla) = the digits isf::parseSortOrder accepts (SortLogic.h, WI-154). ModConfigDecl.h reads the start value from the file when
// the page is built and writes every accepted change back through IniWrite.h (section-aware, keeps the other keys and comments). The start
// value goes through isf::readSortOrder, so a hand-edited word (sort_order=name) or an out-of-range digit shows what loadIni really uses.
// Host-tested by common_contract_test.cpp, which parses this very string.
#pragma once

#include <stdio.h>
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
)TOML";

// Start-value reader for the page: raw INI read, then the value is replaced by the digit of isf::parseSortOrder(value), exactly the order
// loadIni applies (words, digits 0..3, case and spaces; anything else is Category), so the page never shows a row the mod does not use.
using RawIniRead = bool (*)(const char* path, const char* section, const char* key, char* out, int cap);
inline bool readSortOrder(RawIniRead raw, const char* path, const char* section, const char* key, char* out, int cap) {
    if (!raw || cap < 2 || !raw(path, section, key, out, cap)) return false;
    snprintf(out, cap, "%d", static_cast<int>(parseSortOrder(out)));
    return true;
}

}  // namespace isf
