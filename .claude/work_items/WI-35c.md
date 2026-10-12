# WI-35c Equip 21 px tooltip onto TextFit and GOG SceneView fields
area: menu-3d-shared
size: S
ready: hold
after: WI-35b
items: C205,C207
files: patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp; patch_manager_mods/_shared_src/TextFit.h; patch_manager_mods/_shared_src/K2Addr.h
out-of-repo: none
blockers: after: WI-35b (both edit EquipCharPreview.cpp). C205: needs Mod 14 acceptance (unknown). C207: doc 23:126 says `makeGog()` is uncommitted and waits on the GOG-table session (doc 21b). In this worktree `K2Addr.h` is clean (last commit dc40141), so the session state is unknown. Doc 23 cites :328 and :440-451, but `makeGog()` is at `K2Addr.h:378` now.
docs: patch_manager_mods/23_menu_3d_shared_migration.md:103 (C205), :126 (C207)

## Goal
Move the 21 px per character tooltip guess onto TextFit (C205), and fill the GOG SceneView scene fields (C207). Both rows are gated, so start no code now.

## Steps
1. Gate C205: Mod 14 acceptance (unknown). Without it, stop.
2. C205 (after the gate): `TipCharW = 21` at `EquipCharPreview.cpp:345` is commented as a px estimate. Move it onto `_shared_src/TextFit.h` (`textPx`, `fitText` exist; TextFit.h line 1).
3. Gate C207: the GOG-table session (doc 21b) commits. Check its state first, since K2Addr.h is clean in git now.
4. C207 (after the gate): in `makeGog()` (`_shared_src/K2Addr.h:378`), set the SceneView scene fields so GOG SceneView helpers stop refusing. Re-check the scene-field lines with rg before editing, because doc 23:126 line numbers are stale.

## Verify
1. `python3 scripts/run_tests.py <filter; suite name unknown>`
2. `python3 scripts/build_check.py`
3. `python3 scripts/compat_check.py patch_manager_mods/_shared_src/TextFit.h patch_manager_mods/_shared_src/K2Addr.h`

## Docs
- Per landed row: `python3 scripts/plan_manager.py mark-done <id>` for doc 23. The TSV gives these rows id "-", so resolve the id first.
- CHANGELOG: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C205 patch_manager_mods/23_menu_3d_shared_migration.md:103 id=- blocked
- C207 patch_manager_mods/23_menu_3d_shared_migration.md:126 id=- blocked
