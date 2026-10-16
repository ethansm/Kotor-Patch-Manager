# UX-B recon: holster editor 3D viewport (own scene vs equip3d)

Method: static dumps only (`~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<bb>/<addr>.c|.asm`, bucket = 2 hex digits after `00`), repo source, docs 01/10/42, `_shared_src/HolsterApi.h`. No live REA. Steam sha 6A522E71. `equip3d` = `patch_manager_mods/equipment_3d_character_src/`. Line numbers are in `EquipCharPreview.cpp` unless a path is given.

## 1. How the equip 3D view is built today

Control and scene (engine objects, one set per panel bind):
- Hook H-E1 `0x008AA16A` = `BindEquipView` (:845), inside CSWGuiInGameEquip ctor `0x8A92D0`, right before `CALL 0x40F5A0` (StopLoadFromLayout) at `0x8AA170` (asm `8a/008a92d0.asm:851-852`).
- `sceneview::createView(panel,"3D_MODEL")` (:854; `_shared_src/SceneView.h:120-137`): alloc `0x400` block, GuiControlCtor `0x418990`, vtable CSWGui3DSceneView `0x9A3D34`, CSWGuiScene ctor `0x417F40` at view+0x60, `InitControl 0x40F620(panel, view, "3D_MODEL", addToList=1, scale=1)`.
- CSWGuiScene ctor `0x417F40` (`41/00417f40.c:28-40`) creates the Scene* (view+0x74, `FID_conflict_Create("scene")`) and a CAurCamera (view+0x78, `"camera"`, ctor `0x47ECD0`) per view. No global scene or camera found.
- `initScene(view,"gui3D_room")` (:877; SceneView.h:142-157): Scene vt+0x70.
- Rig: `addModel(view,"charrec_light",-1)` (:883; SceneView.h:170-178) = `FnCreateModel 0x462320` + list insert `0x4183B0` (`41/00418360.c`).
- Camera: `cameraAttach(view,rig,"camerahook",1)` (:891; SceneView.h:199-205) = cam vt+0x74 `0x4819E0` into `FUN_00461670` (node via Gob vt+0x10C `FindPart 0x4587F0`, see `r1_camera_keys.md`). `cameraFov(view,22.726)` (:892; SceneView.h:207-213) = cam vt+0x44 `0x47F190`.
- Lights: no light API. The rig MDL `charrec_light` carries its own light nodes. Global render-time light state: UNKNOWN.

Body and weapons (per slot; EquipTick H-E3 `0x8ABA50` calls buildPreview):
- allocCreature (:537-553): `FnNew 0x919723` (0x470 bytes) + CSWCCreature ctor `0x7645F0`; vtable check `0x99EE14`.
- buildPreview (:706-751): readApp copies appearance +0x224 (60 bytes, :560-567); `FnBuildApp 0x76D200(creature,copy,3,1,0)` (:730); `FnSetAlign 0x85D430` (:733); creature vt[0x94] SetScene, vt[0x8c] SetPos, vt[0x88] SetFacing; `FnSetupScene 0x77EC20(creature,view,real,0,1)` (:739) = camera hook + idle pick.
- attachWeapons (:637-667): `FnCreateModel(name,"0x9a4ab0",0,0)` (`0x462320`), then Gob vt[0x50] attach(body,"rhand"|"lhand",0) (:658). Model name from `eq3d::weaponModelName` (`Equip3dLogic.h:172`, `EquipWeaponTable.inc`).
- detachWeapons (:465-478): vt[0x50](0,0,0), vt[0x4C](0), vt[0](1). Helmet: attachHelmet (:498-523, node "GoggleHook" at :518), detachHelmet (:483-494).
- Item lookup: `getItemObj` (:479-482) = `0x73F530` with mgr global `0xA1B4A4`.
- Input: tickInput (:795-826) polls `GetAsyncKeyState(VK_LBUTTON/VK_MBUTTON)` (:797); cursor via `cursorEngine` (:268-275, GuiKit.h:412); wheel `guikit::g_wheel` (GuiKit.h:568).
- Lifetime: EquipDestroy H-E4 `0x8AA6F0` (:990-999) to destroyCreature (:524-535): frees weapons, helmet, then creature (vt[0](1)). Scene dtor `0x418130` (`41/00418130.c`) frees the scene model list (count at scene+0x20), camera (param_1[6]) and Scene* (param_1[5]). The view block dtor address: UNKNOWN.

## 2. Can a 3D control be created at runtime in another panel?

Answer: not outside a panel ctor. Only a ctor-time clone works.
- InitControl `0x40F620` reads the layout handle at panel+0x30 (`40/0040f620.c`, calls `FUN_00418df0(panel,*(panel+0x30),...)`).
- StopLoadFromLayout `0x40F5A0` frees panel+0x30 and clears bit 0x2 of panel+0x48 (`40/0040f5a0.c`). So tag lookup works only before that call.
- `createView` (SceneView.h:117-119) and `createLabelFromTemplate` (GuiKit.h:169-172, IdSentinel check) both say "valid only inside the panel ctor". The post-layout path was not executed (UNKNOWN failure mode, but no handle to read).
- Vanilla 3D_MODEL templates: `upgrade_p.gui`, `classsel_p.gui`, `swoopup_p.gui` (grep of `~/Games/KOTOR2_Mod_Workspace/ultrawide_gui/originals/`). Character page uses `LBL_3DCHAR` (`character_p.gui`).

Options, lowest risk first:
1. (proven, LIVE) Put a 3D_MODEL in a panel's own .gui by GFF copy of `upgrade_p.gui`'s control, plus a new ctor hook. Precedents: equip3d (`scripts/add_equip_3dview_control.py:161-172`, hook `0x8AA16A`) and inv3d (`inventory_p.gui`, hook `0x8A69AD` BindInventoryView). Needs a tag unique to us and a ctor hook site not already claimed.
2. Own .gui via the override folder: same as 1 (both equip_p.gui and inventory_p.gui are overrides now). A new panel resref needs a new panel class: open path, UNKNOWN, not checked. High risk.
3. Runtime template clone outside a ctor: not possible (above).
4. Zero-GFF: vanilla `LBL_3DCHAR` on the character page (CSWGuiInGameCharacter `0x84C3A0`, this+0x3c24; `01_inventory_3d_viewport.md` ~line 200). Engine owns its creature (SetStats `0x84E6F0`). It is not separate, so it does not meet the goal.

Hazards for option 1:
- equip_p.gui control IDs must be 0..N-1 (`add_equip_3dview_control.py` asserts `ids == range(new_id)`); equip3d hard-codes ExpectedControlId 49 (:49) and 79 controls total. A second tool appending controls breaks the assert on re-run.
- equip_p.gui is sha-tracked by the generator: a hand edit is a "foreign file" (script docstring).

## 3. Scene independence and lifetime

- Two views = two CSWGuiScene objects (ctor `0x417F40` makes a Scene* and a camera each). Models, camera and Scene* are per view. Shared: `FnCreateModel 0x462320` returns a new owned Gob per call (equip3d and inv3d free theirs one by one). GuiGateGlobal `0x9F42B4` is read-only in both. Draw-time GL state: UNKNOWN exact path.
- Mod state: equip3d keeps a per-panel Slot (:180-217) in `g_slots` (`PanelSlots<Slot,4>`, :218). A second DLL has its own statics. No sharing needed.
- Conflicts that are not scene conflicts:
  1. Hooks: `0x8AA16A` is taken. KPM rejects two hooks at one address (`20_TESTING_PLAN.md:40`; doc 42:12 `check_hook_spans`). A second viewer needs another ctor site before `0x8AA170` with a non-overlapping byte span, or a provider call from equip3d.
  2. Holster provider `k2.holster` (`HolsterApi.h:49-65`): one edit slot (BeginEdit / GetEdit). RegisterPreview Gobs must be UnregisterPreview'd before they are freed (HolsterApi.h:8).
  3. Wheel: each DLL has its own `WheelHook` (one per module, GuiKit.h:568) and chains the window proc. Both see every wheel event; each applies it only inside its own rect (:801, :818).
- Lifetime for our viewer (mirror of EquipDestroy): unregister holster Gobs, detach weapon Gobs (vt[0x50]/[0x4C]/[0]) BEFORE the creature dtor, dtor the creature (vt[0](1)), then the scene dtor `0x418130` frees models, camera and Scene*. The panel dtor frees the view.
- Re-bind: the equip panel is rebuilt several times per session (doc 10:84). Each bind must `createView` again and destroy the old creature (`EquipCharPreview.cpp:869-870`).

## 4. What is reusable

Candidates for `_shared_src/Preview3d.h` (or provider `k2.preview3d@1`):
- Body build: allocCreature 537-553, readApp 560-567, buildPreview 706-751 (helmet call ~750 is equip-specific), destroyCreature 524-535, realValid 555-558, partGob 569-574, playPart 576-580, constants 54-66 and FnNew/FnCreateModel/FnGetItemObj 38-43.
- Weapons: detachWeapons 465-478, getItemObj 479-482, attachWeapons 637-667, refreshWeapons 669-686. Style and table logic already sit in `Equip3dLogic.h:158-177` and `EquipWeaponTable.inc`. Helmet 483-523 is optional.
- Camera axes: applyView 687-705 (facing via `yawVec` `Equip3dLogic.h:221-224`; pan = creature z via vt+0x8c; zoom = `fovForZoom` :220 into `cameraFov`). Spin: EquipTick :982-983, applied to creature facing (no camera orbit).
- Input: tickInput 795-826 with `yawDrag` 204, `panDrag` 206-209, `dragAxis` 211-214, `zoomStep` 216-219, `hitTop` 201-203, `rectContains` 197-199, `clientToEngine` 193-195.
- Stays equip-only: toolbar/menu/tooltip 286-459, flourish 753-794, BindEquipView/EquipTick/EquipDestroy 845-999, cover labels 276-285.
- Already shared: `SceneView.h`, `GuiKit.h` (wheel, cursor, gameWindow), `WindowHook.h`.

Sharing needs a viewer struct (block, creature, wpn[2], yaw, zoom, panZ, drag state) passed to the functions above (about 300 lines to move). equip3d's tests (`equip3d_logic_test.cpp`, `common_contract_test.cpp`) would need updating.

## OPEN and UNKNOWN
- View block dtor address (control-list teardown).
- Render-time global state (GL and any light globals) for two scenes drawn in one frame.
- Behaviour of InitControl after StopLoadFromLayout (not executed).

FEASIBILITY: medium. Main blocker: the equip panel's only ctor hook (`0x8AA16A`) belongs to equip3d, and equip_p.gui IDs are contiguous and owned by one script. A second viewer needs a new pre-`0x8AA170` hook site or a provider. A new panel class is low (unverified).
