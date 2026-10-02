# Pull Request: Inventory Sort & Filter

### PR Title
`feat(inventory): backpack item sorting, usability tinting, and cycling filters (#104, #84)`

### Commit Message
```text
feat(inventory): add backpack sorting and cycling tab filters

Sorts backpack rows by base item category/type, tints unusable items
red-orange, and adds cycling filter modes to the All tab.

- Patches/InventorySortFilter/manifest.toml: patch manifest (v1.0.0)
- Patches/InventorySortFilter/kotor2-steam-aspyr.hooks.toml: detour hooks
- Patches/InventorySortFilter/InventorySortFilter.cpp: buffer sort logic,
  filter state machine, SetCanUse row coloring
- Patches/InventorySortFilter/README.md: sorting rules and filter modes
```

### PR Description Body
```markdown
### Summary of Changes
Adds backpack sorting, color tinting for unusable items, and cycling sub-filters (All $\rightarrow$ New $\rightarrow$ Usable $\rightarrow$ Upgradeable) when re-clicking the active inventory tab.

### Motivation & Background
Managing an inventory with hundreds of items in KOTOR II is cumbersome due to lack of sorting and static filter tabs. This patch implements long-requested inventory QoL improvements while demonstrating the use of `SetCanUse` row styling discussed in **#84 (Expose GUI color changes)**.

### Technical Implementation
- **Tab Cycling (`0x008A8CF0` - `SortFilterTabClick`):** Re-clicking the active "All" filter tab advances an internal mode counter cycling between `All`, `New Items` (reads `item+0x2c8` bit 7), `Usable by Current Character`, and `Has Open Upgrade Slots`. Uses `consumed_exit_address` (`0x008A8DE2`) to cleanly return.
- **Row Buffering & Sorting (`0x008A7E22` & `0x008A7E76`):** Intercepts `CSWGuiInGameInventory::PopulateItemListBox`. Rather than inserting rows immediately, items are buffered, sorted by base-item category and value, and flushed into the list control.
- **Usability Tinting (`0x008A8478` - `SortFilterRowSelect`):** Calls engine `CSWGuiInGameItemEntry::SetCanUse` (`0x008B1D00`) mode 2, tinting unusable rows with engine red-orange `(0.74, 0.11, 0.0)`.

### Testing & Verification
- [x] Tested with large inventories (200+ items) with zero frame stutter on list population.
- [x] Switching party members immediately re-evaluates usability tints for the active companion.
- [x] Closing inventory cleanly flushes "New" item flags per vanilla rules.
- [x] Passes `python3 tools/validate-patches.py`.

Relates to #104, #84
```
