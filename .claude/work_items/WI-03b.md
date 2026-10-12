# WI-03b Inventory 3D viewport GUI, logging and build rows
area: inventory-3d-viewport
size: M
ready: hold
after: WI-03
items: C011,C012,C014,C015,C016,C020,C021
files: patch_manager_mods/inventory_3d_viewport_src/Inventory3DViewport.cpp; patch_manager_mods/inventory_3d_viewport_src/ItemModelTable.inc; scripts/add_inventory_3dview_control.py; scripts/generate_ultrawide_gui_overrides.py
out-of-repo: none
blockers: after WI-03 (same file: Inventory3DViewport.cpp). C011 needs user GO. C012 needs the user's fix-or-accept call on pitch 55. C014, C015, C016, C021 need manual extraction of inventory.gui and upgrade_p.gui, and C016 and C021 need an in-game check. C020 needs the hash fix (doc 01:576-577). No create-patch script or inventory .kpatch is tracked in the repo (checked 10-12).
docs: patch_manager_mods/01_inventory_3d_viewport.md:lines 5, 22, 505, 506, 509, 585, 596 (no ids)
deferred: C011,C012,C014,C015,C016,C020,C021

## Goal
Hold. Every row here is gated by user action: GO, a decision, manual extraction, an in-game check, or a hash fix. Nothing runs until those land.

## Steps
1. None runnable now (all rows deferred). Gate notes for when each gate clears:
   - C011: trim the verbose `inv3d_debug.txt` logging in `Inventory3DViewport.cpp` (about 4000 lines per session; doc 01 open item 2). After user GO. No install in this step.
   - C012: ask the user to fix or accept the flat-item pitch 55 in `ItemModelTable.inc`. Edit only after the answer.
   - C014, C015, C016, C021: after manual extraction, run `scripts/add_inventory_3dview_control.py` on the extracted `inventory.gui` and `upgrade_p.gui`. C015: the script already copies the upgrade_p 3D_MODEL; verify, then close. C016: control ID 21 is already in source; ultrawide placement needs an in-game check. C021: confirm `scripts/generate_ultrawide_gui_overrides.py` reaches the 3D view.
   - C020: doc 01 Phase 3 step 5 (line 585). Build only after the hash fix at doc 01:576-577.

## Verify
1. `python3 scripts/build_check.py` (after any C011 or C020 edit)
2. `python3 scripts/run_tests.py <filter>` (suite name unknown)
3. In-game ultrawide check: user play test, no command.

## Docs
1. Rows have no ids: the `mark-done` phase id is unknown. Do not dedupe `-` rows.
2. Once ids exist: `python3 scripts/plan_manager.py mark-done <phase> --state <STATE> --commit <sha>`.
3. Never sed the master plan. CHANGELOG: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C011 patch_manager_mods/01_inventory_3d_viewport.md:5 id=- open
- C012 patch_manager_mods/01_inventory_3d_viewport.md:22 id=- open
- C014 patch_manager_mods/01_inventory_3d_viewport.md:505 id=- open
- C015 patch_manager_mods/01_inventory_3d_viewport.md:506 id=- open
- C016 patch_manager_mods/01_inventory_3d_viewport.md:509 id=- open
- C020 patch_manager_mods/01_inventory_3d_viewport.md:585 id=- open
- C021 patch_manager_mods/01_inventory_3d_viewport.md:596 id=- open
