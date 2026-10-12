# WI-05c Shared providers: SceneView, PanelSlots, Scene3D (B4, B8)
area: shared-src
size: S
ready: hold
items: C028,C029
files: patch_manager_mods/_shared_src/SceneView.h; NEW:patch_manager_mods/_shared_src/Scene3D.h; patch_manager_mods/_shared_src/PanelSlots.h
out-of-repo: none
blockers: all rows gated by owner GO (doc 01:640 rollout). C028 is size L and creates NEW `Scene3D.h` (missing today, checked 10-12). WI-04 C130 also edits `SceneView.h`: one writer at a time, and this WI runs first. Doc 01 and doc 10 line numbers for B4 and B8 are from 10-02 and are stale.
docs: patch_manager_mods/01_inventory_3d_viewport.md:645 (C028), :646 (C029); patch_manager_mods/10_equipment_3d_character.md:184-185 (B4, B8)
deferred: C028,C029

## Goal
Hold. The shared scene-view (B4) and panel-slot (B8) providers wait on the owner's GO.

## Steps
1. None runnable now (all rows deferred). Gate notes for when the GO lands:
   - C028 (B4): merge `BindInventoryView` (`Inventory3DViewport.cpp:425` today; doc 01 says :590-654) with the constexpr block and `EquipCharPreview.cpp` `BindEquipView` (`:845` today) into one `scene3d` helper in NEW `Scene3D.h`. Drop `modelCount`, `getModel` and `dumpMem`.
   - C029 (B8): the slot table. `findSlot` is at `Inventory3DViewport.cpp:180` (doc 01 says `claimSlot`/`findSlot` at :282; `claimSlot` was not found). `PanelSlots<Slot, 4>` already drives the equipment slot table (`EquipCharPreview.cpp:218`).

## Verify
1. When unlocked: `python3 scripts/compat_check.py <changed headers>`, `python3 scripts/build_check.py`, and `python3 scripts/check_shared_headers.py`.

## Docs
1. Mark-done phase ids are unknown. Use `python3 scripts/plan_manager.py mark-done <phase> --state <STATE> --commit <sha>` only after the GO.
2. Never sed the master plan. CHANGELOG: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C028 patch_manager_mods/01_inventory_3d_viewport.md:645 id=B4 parked
- C029 patch_manager_mods/01_inventory_3d_viewport.md:646 id=B8 parked
