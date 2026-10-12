# Code Review & Remediation Feedback: Mod 11 (Inventory Sort & Filter)

> **Target**: [`patch_manager_mods/inventory_sort_filter_src/InventorySortFilter.cpp`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_sort_filter_src/InventorySortFilter.cpp)  
> **Logic Header**: [`patch_manager_mods/inventory_sort_filter_src/SortLogic.h`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_sort_filter_src/SortLogic.h)  
> **Hook Configuration**: [`patch_manager_mods/inventory_sort_filter_src/kotor2-steam-aspyr.hooks.toml`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_sort_filter_src/kotor2-steam-aspyr.hooks.toml)  
> **Documentation**: [`patch_manager_mods/11_inventory_sort_filter.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/11_inventory_sort_filter.md)  
> **Review Date**: September 30, 2026  
> **Status**: Comprehensive Engineering Audit & Actionable Remediation Plan

---

## 1. Architectural Highlights & Strengths

This mod is an exemplary implementation of an engine-level quality-of-life feature:
- **Decoupled Architecture**: Pure algorithms (sorting comparator, cycle state machine, open upgrade slot detection, string parsing) are fully isolated in [`SortLogic.h`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_sort_filter_src/SortLogic.h) and verified with host unit tests ([`sort_logic_test.cpp`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_sort_filter_src/sort_logic_test.cpp)).
- **Strict Weak Ordering**: The comparator `isf::before()` satisfies all strict weak ordering invariants (irreflexivity, asymmetry, and transitivity) with guaranteed stability via `Entry::orig`.
- **Surgical Detour Hooks**:
  - **H6a (`0x008A8CF0`)**: Leverages the framework's `consumed_exit_address = 0x008A8DE2` to intercept re-clicks on the "All" tab. Advances through synthetic sub-filters (New → Usable → Upgradeable → All) and updates `LBL_FILTER` text without requiring any GFF UI alterations.
  - **H6b (`0x008A7E22`)**: Buffers incoming backpack items in `g_buf`, performing pre-filtering for upgradeable slots while preserving upstream engine filtering.
  - **H6c (`0x008A7E76`)**: Sorts the buffer and flushes row creation directly into the native row array prior to list finalization (`CALL 0x0041FC30`).
- **Reselection Preservation**: Accurately tracks `PanelSelectedIdOff` across sort permutation, updating `[EBP - 0x20]` so the active selection cursor remains anchored to the correct item.
- **Unusable Item Tinting**: Correctly invokes `CSWGuiInGameItemEntry::SetCanUse(row, 2)` (red-orange) on unusable items while preserving teal and New item highlight colors.

---

## 2. Technical Findings & Latent Issues

### 🚨 Issue 1: Stale `g_upgrade` Filter State on Panel Reopen
* **Location**: [`InventorySortFilter.cpp#L188-L197`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_sort_filter_src/InventorySortFilter.cpp#L188-L197)
* **The Problem**:
  `g_upgrade` is a global flag tracking the synthetic 4th state (engine mode 0 + upgrade filter).
  In `normaliseState(int panel)`:
  ```cpp
  int mode = filterMode();
  if (g_upgrade && (mode != 0 || !g_cfg.cycle)) g_upgrade = false;
  ```
  When the player closes the inventory panel while in the "Upgradeable" filter state and subsequently reopens it:
  1. The engine reinitializes `GuiInGame + 0xFC1` to `0` (mode "All").
  2. Because `mode == 0`, the guard `mode != 0` evaluates to `false`.
  3. If the panel memory was reused at the same heap address (`panel == g_panel`), `g_upgrade` remains `true`.
  4. The newly reopened inventory panel immediately filters for upgradeable items rather than showing the default "All" view.
* **Remediation**:
  Hook `CSWGuiInGameInventory::OnPanelRemoved` (`0x008A7520`) to reset `g_upgrade = false; strcpy(g_caption, "Filter");`, or detect a freshly opened panel by checking `panel + 0x24D0` bit 0.

---

### ⚠️ Issue 2: Stale Selection Pointer when Selected Item is Filtered Out by Upgrade Filter
* **Location**: [`InventorySortFilter.cpp#L258-L275`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_sort_filter_src/InventorySortFilter.cpp#L258-L275)
* **The Problem**:
  In `SortFilterBufferRow`:
  ```cpp
  if (g_upgrade && g_cfg.cycle && filterMode() == 0 && !itemHasOpenSlot(item)) {
      ++g_dropped;
      return;
  }
  ```
  If an item currently selected in the UI is dropped because it lacks open upgrade slots, the function returns before reaching the reselect check (`rd<int>(panel, PanelSelectedIdOff) == id`).
  As a result:
  - `g_selSet` remains `false`.
  - `PanelSelectedIdOff` retains the dropped item's ID.
  - At flush time, `[EBP - 0x20]` remains `-1`, and the vanilla engine fallback at `0x008A7F50` defaults to selecting row 0.
  While selecting row 0 prevents a crash, `PanelSelectedIdOff` temporarily retains a mismatched ID until the user clicks a row.
* **Remediation**:
  If the dropped item matches `PanelSelectedIdOff`, explicitly clear it to `NoSelection`:
  ```cpp
  if (g_upgrade && g_cfg.cycle && filterMode() == 0 && !itemHasOpenSlot(item)) {
      ++g_dropped;
      if (rd<int>(panel, PanelSelectedIdOff) == id) {
          wr32(panel + PanelSelectedIdOff, NoSelection);
      }
      return;
  }
  ```

---

### ⚠️ Issue 3: Open Slot Predicate Fallback Sentinel
* **Location**: [`InventorySortFilter.cpp#L164-L169`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/inventory_sort_filter_src/InventorySortFilter.cpp#L164-L169)
* **The Problem**:
  In `itemHasOpenSlot(int item)`:
  ```cpp
  for (int i = 0; i < n; ++i) slots[i] = rd<int>(item, ItemUpgradeSlotsOff + 4 * i, 0);
  ```
  The fallback default passed to `rd<int>` is `0`. In KotOR, an empty upgrade slot is represented by `-1` (`0xFFFFFFFF`). If `rd<int>` fails a memory probe on a corrupted item struct, returning `0` implies that slot 0 contains an installed item rather than being empty.
* **Remediation**:
  Pass `-1` as the default fallback value to match empty slot representation:
  ```cpp
  for (int i = 0; i < n; ++i) slots[i] = rd<int>(item, ItemUpgradeSlotsOff + 4 * i, -1);
  ```

---

## 3. Recommended Remediation Code Changes

### Patch A: Reset Stale Upgrade State & Clear Dropped Selection (`InventorySortFilter.cpp`)
```diff
--- a/inventory_sort_filter_src/InventorySortFilter.cpp
+++ b/inventory_sort_filter_src/InventorySortFilter.cpp
@@ -167,3 +167,3 @@ bool itemHasOpenSlot(int item) {
     int n = isf::upgradeSlotCount(static_cast<int>(type));
-    for (int i = 0; i < n; ++i) slots[i] = rd<int>(item, ItemUpgradeSlotsOff + 4 * i, 0);
+    for (int i = 0; i < n; ++i) slots[i] = rd<int>(item, ItemUpgradeSlotsOff + 4 * i, -1);
     return isf::hasOpenSlot(static_cast<int>(type), slots);
@@ -260,3 +260,6 @@ extern "C" void __cdecl SortFilterBufferRow(int ebp) {
         if (g_cfg.log && g_cfg.logItems) logf("  drop (no open upgrade slot): item=%08x id=%08x base=%d", item, id, rd<int>(item, ItemBaseOff));
+        if (rd<int>(panel, PanelSelectedIdOff) == id) {
+            wr32(panel + PanelSelectedIdOff, NoSelection);
+        }
         return;
     }
```

---

## 4. Verification & Testing Matrix

| Scenario | Target Verification | Expected Result |
| :--- | :--- | :--- |
| **All Tab Cycling** | Click "All" tab repeatedly: All → New → Usable → Upgradeable → All. | Filter caption updates cleanly (`Filter: New`, `Filter: Usable`, etc.); rows filter dynamically. |
| **Panel Reopen after Upgrade Filter** | Set filter to "Filter: Upgradeable", close inventory with ESC, reopen inventory. | Filter resets cleanly to "All" (caption "Filter"); all items are displayed. |
| **Unusable Item Tinting** | View inventory containing weapons/armor the current character cannot equip. | Unusable rows display red-orange tint; usable rows retain default teal styling. |
| **Companion Switching** | Switch active character in inventory. | Unusable tints dynamically update to reflect the newly active companion's proficiencies. |
| **Sort Stability** | Populate inventory with 20+ items of identical base types. | Stable tie-break retains original acquisition order; new items appear at the top. |
