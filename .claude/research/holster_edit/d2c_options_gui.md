# D2c: in-game Options layout for the holster editor (static recon, 2026-10-16)

Rects are [L,T,W,H] in 800x600 layout units unless marked px. Vanilla = `ultrawide_gui/originals/` (sha-equal to the chitin copy). Override = live `override/`.

## 1. Ultrawide override of optionsingame_p.gui and optionsmain_p.gui

Both are overridden by the ultrawide GUI work (content layer + frame bars, k = 0.7407 at 3840x1600, 16:9).

- Generator: `scripts/generate_ultrawide_gui_overrides.py`. Both names in `FRAME_GUIS` (lines 164-168). Root fill `pnl_pause_pc` is in `CONTENT_FILLS` (line 92). `k = content_factor(3840,1600,16/9)` (lines 594-603); defaults at lines 1703-1704. Frame rule E_left = 34 from `ultrawide_gui/frame_edges.json`.
- Sha tracking: `ultrawide_gui/manifest.json`, keys `optionsingame_p` and `optionsmain_p`. Both have source "chitin" (no override existed before), original_sha256 f89f3ca2... / b60c4b32... (matches originals), deployed_sha256 93b63a60... / 8322c5a5.... The live files match the manifest, so they are untouched.
- Foreign-file guard: `deploy_file` refuses a live sha that is neither original, nor manifest deployed, nor new output (lines 1399-1405). A hand edit therefore makes the next generator run refuse.
- Rollback for source "chitin" deletes the override file (lines 1452-1453). Any control we add is lost on rollback.

optionsingame_p.gui, 16 controls (IDs 0-15, no 3D_MODEL), vanilla -> override:

```
ID0  BTN_SAVEGAME   T6 (78,134,318,32)   -> (161,134,236,32)
ID1  BTN_GAMEPLAY   T6 (78,206,318,32)   -> (161,206,236,32)
ID2  BTN_QUIT       T6 (78,386,318,32)   -> (161,386,236,32)
ID3  LBL_TITLE      T4 (90,57,252,24)    -> (170,57,187,24)
ID4  BTN_FEEDBACK   T6 (78,242,318,32)   -> (161,242,236,32)
ID5  BTN_AUTOPAUSE  T6 (78,278,318,32)   -> (161,278,236,32)
ID6  BTN_GRAPHICS   T6 (78,314,318,32)   -> (161,314,236,32)
ID7  BTN_SOUND      T6 (78,350,318,32)   -> (161,350,236,32)
ID8  LBL_BAR1       T4 (61,105,678,14)   -> (34,105,732,14)
ID9  LBL_BAR2       T4 (61,125,678,14)   -> (34,125,732,14)
ID10 LBL_BAR3       T4 (61,505,469,14)   -> (34,505,462,14)
ID11 LBL_BAR4       T4 (636,505,103,14)  -> (575,505,191,14)
ID12 LBL_BAR5       T4 (409,476,307,32)  -> (407,476,227,32)
ID13 BTN_EXIT       T6 (75,516,120,28)   -> (159,516,89,28)
ID14 BTN_LOADGAME   T6 (78,170,318,32)   -> (161,170,236,32)
ID15 LB_DESC        T11 (410,134,313,336) -> (407,134,232,336)
     PROTOITEM child (426,150,265,38) -> (419,150,197,38)
     SCROLLBAR child (707,134,16,336) -> (623,134,16,336)
```

Child rule: SCROLLBAR.left = LB_DESC.left + W - 16 in both files. Children are absolute coordinates.

optionsmain_p.gui, 11 controls, no 3D_MODEL. Same rule: buttons x 78 -> 161 (W 318 -> 236); BTN_GAMEPLAY 9 (134), BTN_FEEDBACK 3 (170), BTN_AUTOPAUSE 0 (206), BTN_GRAPHICS 1 (242), BTN_SOUND 2 (278) are one column; LBL_TITLE 4 (76,56,648,28) -> (160,56,480,28); BTN_BACK 8 (75,516,120,28) -> (159,516,89,28); LB_DESC 10 (410,134,313,364) -> (407,134,232,364); BAR1/2/3 full width -> (34,y,732,14) by the frame rule.

## 2. Free space for (a) one extra button and (b) editor viewport + pane

**Vanilla, 800x600**
- (a) The button column is x 78..396, y 134..418 (8 buttons, pitch 36). Free below it: y 422..490, because BAR3 starts at 505. Extra button: `BTN_HOLSTER (78,422,318,32)`. A second slot fits at (78,458,318,32). UX_OPTIONS.md says 460; 458 keeps the 36 pitch.
- (b) If the list and LB_DESC are hidden, the free block is x 78..723, y 134..470 (BAR5 at 476 stays).
  - Option V1 (no LB_DESC change): 3D_MODEL `(178,134,216,336)`, 16 px gap, then the unchanged LB_DESC `(410,134,313,336)`. Block 178..723.
  - Option V2 (pane 232 wide): LB_DESC `(410,134,232,336)`, PROTOITEM `(426,150,184,38)`, SCROLLBAR `(626,134,16,336)`, 3D_MODEL `(178,134,216,336)`. Block 178..642.

**Ultrawide override (k = 0.7407)**
- Content region is fx(61)..fx(739) = 149..651. The frame bars run 34..766.
- (a) `BTN_HOLSTER (161,422,236,32)`; second slot `(161,458,236,32)`.
- (b) The live LB_DESC is already `(407,134,232,336)`, which is the 232x336 pane. Put the 3D_MODEL to its left:
  - U1 (recommended, same as equip3d layout A, which uses an unmapped 216-unit view = 1037x896 px): 3D_MODEL `(179,134,216,336)`, gap 12. Block 179..639 fits 149..651.
  - U2 (fully content-mapped): 3D_MODEL `(236,134,160,336)` = 768x896 px, gap 11.
- Evidence for U1: `patch_manager_mods/10_equipment_3d_character.md:98-100` (layout A is 1037x896 px).

## 3. Generator plan: `scripts/add_options_holster_controls.py` (not written)

Mirror `scripts/add_equip_3dview_control.py` (261 lines). Its anchors: layout table 50-54, rebuild-from-vanilla 131-157, ID assert 159-161, copy 3D_MODEL from upgrade_p 162-172, backup and sha 232-236, read-back asserts 245-256.

1. Input, `--layout auto|vanilla|ultrawide`:
   - ultrawide: the live file's sha equals `manifest.json[optionsingame_p].deployed_sha256`. Measure k = live BTN_GAMEPLAY width / 318 (expect about 0.741). Refuse unless |k - 1| < 0.01 or |k - 0.7407| < 0.01.
   - vanilla: no override file. Base = `originals/optionsingame_p.gui` (k = 1).
2. Backup `BACKUP_ROOT/options3d_<stamp>/` and rebuild from vanilla each run (idempotent). Drop TAG in {3D_MODEL, BTN_HOLSTER} first.
3. Assert vanilla IDs 0..15, then add `3D_MODEL` as ID 16 and `BTN_HOLSTER` as ID 17. Export these as constants for the DLL, like equip's `ExpectedControlId = 49`.
4. Sources, all inside one run:
   - 3D_MODEL: deep copy from `override/upgrade_p.gui`, CONTROLTYPE 4. Its fields are BORDER, CONTROLTYPE, EXTENT, ID, TAG, TEXT.
   - BTN_HOLSTER: deep copy of BTN_GAMEPLAY from the same file, CONTROLTYPE 6. No cross-file copy needed.
   - Rects from section 2 (U1/V1 or V2). Add a guard: refuse any control whose rect is neither vanilla nor the mapped value.
5. Read back and check: IDs contiguous 0..17, CONTROLTYPE 4 and 6, rects as planned, sha printed.
6. Manifest: after a ultrawide-mode write, update `optionsingame_p.deployed_sha256` and add a "post_step" key. Otherwise the generator refuses on its next run (1399-1405). Alternative: `generate_ultrawide_gui_overrides.py --rollback` for this file, regenerate, then re-run this script. A regenerate removes the controls silently.
7. Modes: `--dry-run`, `--rollback` (vanilla mode: delete the file; ultrawide mode: restore the oldest backup).

Ownership risk: a KPM-shipped `optionsingame_p.gui` would overwrite the ultrawide override. Do not ship it. Ship a post-step only.

**Runtime-only alternative (no .gui edit): not viable cleanly**
- optionsingame has no 3D-capable control. Its types are T6 buttons, T4 labels and one T11 LB_DESC.
- `createLabelFromTemplate` (`patch_manager_mods/_shared_src/GuiKit.h:175-208`) clones only a label vtable from a tag in the open layout. It cannot make a scene view. Other layouts (upgrade_p) are not open during this ctor.
- `createView` (`SceneView.h:120-137`) passes the tag through `exoCtor` into `initControl(addToList=1)`. The tag must exist in the open layout; "3D_MODEL" is only the convention. The ID comes from the struct, which is why GuiKit checks `IdSentinel`.
- Re-typing LB_DESC (ID 15) would bind a second control to LB_DESC's struct. The new control would get ID 15 at list index 16, breaking the id==index rule (`GuiKit.h:209-222`). The vanilla listbox would still bind the same tag. Risk: high. Not recommended.
- Whether the engine dispatches on the struct's CONTROLTYPE during `InitControl` is unverified.

## 4. Runtime measurement, to adapt to both layouts

- In-memory control extents are engine px, not layout units (`GuiKit.h:8`, `getExtent` at 224; `K2Addr.h:96`). The engine maps units to px with the screen size, so sx = screenW / 800.
- No tag lookup exists in GuiKit or K2Addr. A control's ID is its index in the panel list (`K2Addr.h:95`; `GuiKit.h:209-222`). Vanilla IDs are the same in both files, so read by ID: BTN_GAMEPLAY = 1, LB_DESC = 15.
- Ctor-time recipe. Hook where equip's H-E1 sits: before `StopLoadFromLayout`.
  1. Read the panel control count at panel+0x28 (`GuiKit.h:49`). Expect 16 (vanilla) or 18 (generated). Else disable and log.
  2. Take the control pointer from the panel list at index 1. `getExtent` gives W_px. k_live = (W_px / screenW) / (318/800). Expect about 1.0 or 0.741.
  3. Cross-check LB_DESC (index 15) the same way: 313 or 232 units.
  4. `createView(panel, "3D_MODEL")` (`SceneView.h:120`), then check the readback ID == 16 (pattern: `EquipCharPreview.cpp:854-870`). Then `bindByTag(panel, "BTN_HOLSTER", 17)` (`GuiKit.h:583`).
- Prefer baked rects (section 3). The file already holds the right layout, so no runtime rect math is needed. Runtime writes via `setRect` (`GuiKit.h:238`) use px and would need `scaleFromRef`/`unitRect` (`GuiKit.h:389-401`) with the live-unit reference rect.
- Hiding the list: `setVisible` (`GuiKit.h:257`).

## Open

Who writes `override/optionsingame_p.gui` for users without the ultrawide mod: a post-step script or a shipped file? A shipped file clobbers the ultrawide override, so this decision comes first.
