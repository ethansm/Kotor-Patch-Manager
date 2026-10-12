# WI-35b Equip menu-3d SafeRead, SceneView and GuiKit label migration
area: menu-3d-shared
size: M
ready: hold
items: C203,C204,C209
files: patch_manager_mods/equipment_3d_character_src/EquipCharPreview.cpp; patch_manager_mods/equipment_3d_character_src/common_contract_test.cpp; patch_manager_mods/_shared_src/GuiKit.h
out-of-repo: none
blockers: after: WI-35 (C206 verify on EquipCharPreview.cpp). C203: needs user build approval for the equip extraction (doc 23:103, :135). C204: after C203 step 2. C209: `guikit::bindLabelByTag` is not in GuiKit.h or the repo (unknown callers; doc 23:91 says equip has 3 callers, unverified). Read-only: SceneView.h and GameAddr.h.
docs: patch_manager_mods/23_menu_3d_shared_migration.md:91 (C209), :135 (C203, C204)

## Goal
Migrate the equip menu-3d shared pieces (C203), check the 0x74 offset (C204), and add the GuiKit label helper (C209). Hold: C203 needs user build approval, so no build starts yet.

## Steps
1. Gate: user build approval for the equip extraction (doc 23:135). Without it, stop.
2. C203 in `EquipCharPreview.cpp`: migrate equip steps 2-5. Replace the SafeRead calls (24 `saferead::` lines, not 14), follow the SceneView recipe (`SceneView.h`, read only), and use the GameAddr names (`GameAddr.h`, read only). Update the contract test in `common_contract_test.cpp`.
3. C209: create NEW `guikit::bindLabelByTag(panel, tag, expectId)` in `GuiKit.h` beside `bindMatches` (:530) and `bindFlags` (:531). It is not in source yet.
4. C204 (after step 2): `SceneObjOffset = 0x74` is at `EquipCharPreview.cpp:48` (comment "view+0x74 = Scene*") and is read at :726. No `0x44` is in source (rg: 0 hits). Confirm 0x74 before changing anything. Add nothing else until step 2 lands.

## Verify
1. `python3 scripts/run_tests.py <filter; suite name unknown>`
2. `python3 scripts/build_check.py`
3. `python3 scripts/compat_check.py patch_manager_mods/_shared_src/GuiKit.h`

## Docs
- Per landed row: `python3 scripts/plan_manager.py mark-done <id>` for doc 23. The TSV gives these rows id "-", so resolve the id first.
- CHANGELOG: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C203 patch_manager_mods/23_menu_3d_shared_migration.md:135 id=- blocked
- C204 patch_manager_mods/23_menu_3d_shared_migration.md:135 id=- blocked
- C209 patch_manager_mods/23_menu_3d_shared_migration.md:91 id=- parked
