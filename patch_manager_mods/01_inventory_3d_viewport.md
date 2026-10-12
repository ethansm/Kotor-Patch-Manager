# Mod Idea 1: Inventory 3D Viewport

**RESUME HERE (2026-10-03): 0.2.1-dev = behaviour-neutral migration onto Common `SafeRead.h` + `SceneView.h` + GameAddr names (doc 23). KPM `6544ac5 0beffc1 f1b68d3 2391b5c`; all gates PASS; STAGED in `$TOOL/inv3d_stage_1003` (STAGE_INFO head=2391b5c), NOT installed. Live is still 0.2.0-dev `f0eba5be`. On the user's OK: DLL swap with .bak (doc 23 "Next action"), then the in-game regression check and an `inv3d_log.txt` read (copy it first) against the old lines. Only expected log change: the load line `inv3d v2 0.2.1-dev loaded (Common SafeRead + SceneView)`.**

**Status (2026-09-30): Mod 1 v2 (0.2.0-dev) is COMPLETE and ACCEPTED by the user ("this initial phase of the mod is excellent"). Live, 17-id install. Final commits `ff2870f 3f0b561 b1b2d0e c3740e6 de64e60 40839b2 bec307d`. Read "v2 live test — READ THIS FIRST" and "v2 build" below. Verbose logging (`inv3d_debug.txt`) is still ON; Phase 6 (trim logging) was NOT done. NEXT (separate mod effort): equipment-page live 3D character, see "Future: equipment-page 3D character" and the research plan referenced there.**

## v2 live test — READ THIS FIRST (2026-09-29/30)

**Result: accepted.** The user tested over several game launches and reported: item previews look great, spacing between panels is good, wheel zoom and drag rotation work. Not yet seen by the user or in the log at the time of writing: **Shift+LMB roll, MMB reset, RMB-drag zoom** (no log lines for them: 0 starts) and **the LBL_BEVEL/LBL_BEVEL2 arc covers** (deployed in `bec307d`; the first attempt, GUI-only in `40839b2`, showed nothing because the engine only creates constructor-bound controls; the DLL now binds them, look for `H1 cover LBL_BEVEL: ... id=22` and `LBL_BEVEL2 ... id=23` in `inv3d_log.txt`). If the arcs are wrong or hidden, check that log line first; `bevels=0` in the INI turns them off.

**Log evidence (game logs are in `$GAME/inv3d_log*.txt`, mirrored evidence not committed):**
- H1: control id 21 OK, camera vtable `0x0098c45c` OK (matches the R13 expectation), scene model count 1, extent px `[2673,357,1003,896]` for layout U/V-era views (`[3096,357,556,800]` in v1). The view origin is top-left (`hit_flip_y=0` works; drag hit-test uses cursor scaled to 3840x1600).
- 49 items rendered in one session; **no Gob vtable GUARD failures; no "still no model"**. Missing variations were handled by the retry path (`substituting var 12/9/6/10 -> 1 (no model after render)`), i.e. the model count check works. The tick-gap lines (2.6 s, 7 s, 34 s) are inventory reopen gaps, which confirms H3 runs only while the inventory is open.
- Wheel: `WHEEL: subclassed window ..., old proc 0040a750`, then `WHEEL: delta=120 ... inside=1` / `applied 120 -> zoom=...`: **the game window receives WM_MOUSEWHEEL under Proton and the subclass works** (119 applications in one session). Zoom hit the 0.30 floor within ~14 notches at `wheel_step=0.9`.
- One later launch ran an older DLL build that rejected the new zoom/fill INI keys as unknown (harmless; the first launch parsed 25 keys, 0 rejected).
- Fit numbers seen: armour D clamps to `dist_max` 4.0 with scale 0.736 (needs 5.4); long sword D=3.59; grenade fit D=0.372 originally (filled the whole view, hence the size-aware fill rule below).

**Final INI (`$GAME/inv3d.ini`):** enabled=1 rig=inv3d_light only_weapons_armor=0 spin_deg_s=70 drag_deg_per_px=0.5 pitch_clamp=80 idle_resume_s=1.5 fit_mode=distance fit_margin=1.15 dist_min=0.25 dist_max=4.0 fov_deg=22.726 fov_axis=v armor_box=0,0,0.95,0.6,0.4,1.9 armor_spin=1 hit_flip_y=0 wheel_zoom=1 wheel_step=0.9 zoom_drag=0.004 roll_deg_per_px=0.5 zoom_min=0.3 zoom_max=3.0 fill_ref=0.6 fill_exp=0.35 fill_min=0.4 (+ `bevels`, default 1). No per-row/itype overrides were needed so far (optional 5th value = default zoom multiplier, e.g. `row.25=0,0,0,15,1.3`). `fov_axis=v` and the default fit were correct; `fit_mode=scale` was never needed.

**Controls over the 3D view:** LMB drag = yaw+pitch (pitch clamped +-80); Shift+LMB drag = roll about the view axis; mouse wheel or RMB drag (up = closer) = zoom (0.3x..3x); MMB click = reset that item's view; auto-spin (70 deg/s, engine sign) pauses while dragging and resumes 1.5 s after release; nothing grabs the mouse (the game's own "mouse look" setting is unrelated, see DB lesson 98).

**Open items / known limits:** (1) roll, MMB reset, RMB zoom and the arc covers not yet user-verified; (2) `inv3d_debug.txt` still present, so the log grows ~4000 lines per session (debug lines have their own 6000-line cap; main cap 12000); (3) `fit_mode=distance` puts long items far from the rig lights (reviewer finding; not visually objectionable in the user's tests); (4) default "flat -> pitch 55" ignores which axis is thin; (5) equipment page not started.

**Deploy / rollback facts learned:**
- Every `KPatchLauncher --patches ... <ids>` run uninstalls everything and reinstalls the listed ids, and **launches the game**. Other sessions reinstall too, using the newest `Patches/<Name>/*.kpatch`; always re-read `patch_config.toml` ids before any install (lesson 99).
- To ship a rebuilt Inventory3DViewport DLL **without** launching the game (hooks unchanged): with the game closed, copy `binaries/windows_x86.dll` from `Patches/Inventory3DViewport/Inventory3DViewport.kpatch` to `$GAME/patches/inventory-3d-viewport.dll` (back the old one up) and copy the kpatch to `inv3d_live_patches/`. `patch_config.toml`/`kpm_install_state.json` need no change while the hook addresses/params are unchanged (`scripts/check_inv3d_kpatch.py` checks them).
- GUI layout: `scripts/add_inventory_3dview_control.py --layout V` (current); layouts R/T/L/K/S/U also exist; `--rollback` restores the oldest backup (`backups/inventory3d_*`, pristine). The script accepts the previous layout's values as "current" (alt) so layouts chain S -> U -> V, but going back to an older layout needs `--rollback` first.
- Rig: `scripts/make_inv3d_light.py --deploy | --rollback` (override/inv3d_light.mdl/.mdx).
- Full rollback of Mod 1: reinstall the id set without `inventory-3d-viewport`; `make_inv3d_light.py --rollback`; `add_inventory_3dview_control.py --rollback`; remove `inv3d.ini`/`inv3d_debug.txt`.
- Backups: `~/Games/KOTOR2_Mod_Workspace/backups/patch_manager_inv3d_v2_pre_install_20260929_213918/` (whole game-dir state before v2), `inventory3d_*` (GUI), `inv3d_light_*` (rig), `inv3d_v2_dll_*` (DLL swaps).

## v2 build (0.2.0-dev; commits `b1b2d0e` base, `c3740e6` review fixes, `de64e60` zoom/roll/fill, `bec307d` covers)

**Files.** Source of record `kotor-patch-manager/Patches/Inventory3DViewport/`, mirrored in `inventory_3d_viewport_src/`: `Inventory3DViewport.cpp`, `Inv3dQuat.h` (+ `Inv3dQuat_test.cpp`, host `g++` test, 9 cases pass), `ItemModelTable.inc` (categories, `kItemType[]`, `kDefaultOrient[][4]`), `ItemFitTable.inc` (341 `FitEntry`, sorted by row,var), `RigCamera.inc`. Hooks unchanged from v1 (H1 0x8A69AD, H2 0x8A8172, H3 0x8A7350). Static checks: `scripts/check_inv3d_kpatch.py`.

**Rig `inv3d_light`** (`scripts/make_inv3d_light.py`): byte copy of `upgitem_light.mdl/.mdx` with names rewritten in place (61 bytes differ, all inside name strings/model-name fields): `rotatehook*`→`xotatehook*`, `camerahook3`→`camerahook`, every other `camerahook<N>`→`xamerahook<N>`, `upgitem_light`→`inv3d_light`. Every item therefore uses the fallback `camerahook` (pos 0.984, 0.104, 0.045; fwd −X, up +Z, right +Y) and nothing is attached, so the DLL owns the item transform. `RigCamera.inc` is generated from the renamed node's rest pose.

**Presentation (DLL).** `q = qAxis(right, pitch) ⊗ qAxis(up, yaw) ⊗ qBase`; target `T = camPos + fwd·D`; `pos = T − R(q)·(s·c)`; scale written at Gob+0x1e0. Fit: rotate the 8 bbox corners by qBase; spinning-cylinder sizing (h = √(ex²+ey²), v = ez) when the default pitch is 0, sphere sizing otherwise; `D = margin·max(v/(2·tanV), h/(2·tanV·aspect))`, clamped to [dist_min, dist_max], with scale below 1 when D would exceed dist_max; `fit_mode=scale` keeps the default distance and scales instead. Armour uses the INI `armor_box`. Assumes the camera up axis is world Z.
Orientation rules (generator, from the `_001` bbox): longest z → upright, pitch 0 (30 rows); weapon longest y → identity, pitch 0 (16); flat non-weapon → pitch 55 (26); otherwise pitch 15 (5); armour fixed box (17); no `_001` bbox → pitch 15 (15). INI `itype.N` / `row.N` = `rx,ry,rz,pitch` override (row wins).
**Missing/EMPTY variation (category 2):** fit-table lookup; missing or EMPTY → item byte +0x2a4 temporarily set to 1 (all 4 bytes saved and restored on every path); if the model count is still < 2 after render, one retry with var 1; else no preview. (The engine removes the previous item model on each render, so count < 2 after a render means failure.) **EMPTY count:** 6 distinct models, 8 (row,var) entries (the earlier "8 EMPTY variations" counted rows).
**Zoom/roll/reset (`de64e60`):** RMB drag and the mouse wheel change `zoom` (D_eff = D x zoom, floor 0.1); the wheel comes from a window subclass (`SetWindowLongPtrA GWLP_WNDPROC`, only consumes WM_MOUSEWHEEL over the view while the inventory ticks); Shift+LMB drag = roll about the camera forward axis (`q = qRoll(fwd) * qPitch(right) * qYaw(up) * qBase`); MMB = reset. **Size-aware default fill:** `fill = clamp((bbox diagonal / fill_ref)^fill_exp, fill_min, 1)` divides the fit distance so small items (grenades) no longer fill the whole view. **Mouse drag (original).** H3 polls `GetAsyncKeyState(VK_LBUTTON)`, `GetCursorPos`/`ScreenToClient` on the game window found via `EnumWindows` (largest visible client rect of the process), scaled to 3840×1600; the press must start inside the view extent (view+4..+0x10) and the game window must be foreground. Yaw = dx·0.5°/px about camera up, pitch = dy·0.5°/px about camera right (clamped ±80°); auto-spin is suspended while dragging and resumes after 1.5 s. Nothing grabs the mouse; only reads.
**Guards.** Before any Gob vtable call, slots 0x5C/0x60/0x64/0x68/0x108 must equal 0x461490/0x4614e0/0x459710/0x459740/0x461530; otherwise the first 0x44 slots are dumped once and presentation is skipped for that item. All reads use `IsBadReadPtr`, writes `IsBadWritePtr`. Compile switches `INV3D_PRESENTATION`, `INV3D_DRAG`.
**INI** `$GAME/inv3d.ini` (re-read at most once per second on mtime change; missing/garbage → defaults, every rejected line logged). Keys: enabled, rig, only_weapons_armor, spin_deg_s, drag_deg_per_px, pitch_clamp, idle_resume_s, fit_mode, fit_margin, dist_min, dist_max, fov_deg, fov_axis, cam_pos/fwd/up, armor_box, armor_spin, hit_flip_y, screen_w/h, itype.N, row.N. `$GAME/inv3d_debug.txt` enables per-tick logging.
**Final layout V** (`add_inventory_3dview_control.py --layout V`, applied over S/U): LB_ITEMS `[48,134,236,329]` (protoitem 60, scrollbar 268), BTN_USEITEM/BTN_EXIT at x 48, LB_DESCRIPTION `[294,134,246,300]` (protoitem `[306,150,211,42]`, scrollbar `[524,134,16,300]`), LBL_CREDITS `[294,442,246,28]`, LBL_CREDITS_VALUE `[469,442,63,28]`, **LBL_BAR5 left at its vanilla `[407,476,227,32]`** (its notch lines up with the party portraits; squeezing it broke that), `3D_MODEL [550,134,202,336]` (about 970x896 px), edge gaps 14 units both sides, middle gaps 10. **Cover frame:** `LBL_BEVEL [546,130,210,38]` and `LBL_BEVEL2 [546,436,210,38]` copied from `character_p.gui` (fills `uibit_brdr_chr1p`/`chr2p`, sized like the character page: view +-4 wide, top at view top-4, bottom at view bottom-34), IDs 22 and 23 (3D_MODEL stays 21, which the DLL expects). **Engine fact:** the panel constructor only creates controls it binds by tag; extra GUI controls are NOT instantiated (H1 log: 22 controls after binding, not 24). The DLL therefore constructs the two labels right after the view (`bindCover`: `FnNew 0x919723`, `CSWGuiLabel` ctor `0x419740`, vtable `0x9878BC`, size 0x148, `InitControl 0x40f620` by tag, flags `+0x48` bit5 = click-through), so they draw on top of the view. History: layout S was the first user-requested shift (list left).
**Superseded layout S notes:** (`add_inventory_3dview_control.py --layout S`, user request "shift the list left"): LB_ITEMS/BTN_USEITEM/BTN_EXIT −120 units; LB_DESCRIPTION 260 wide; credits/bar5 to match; `3D_MODEL [557,134,209,364]` = 1003×971 px (aspect 1.03). Other layouts R/T/L/K exist. Rollback: `--rollback` (oldest backup = pristine).

## Future: equipment-page 3D character — **now Mod 5, see `10_equipment_3d_character.md`** (0.1.0-probe live 2026-09-30; the notes below are the original pre-research sketch and contain corrections: creature-level equip hooks are NOT the refresh point, idle is not chosen by alignment on the creature)
Goal stated by the user: on the equipment screen show a LIVE render of the character wearing the currently chosen equipment (updates on equip/unequip and NPC switch), and occasionally mix a flourish animation into the default idle. Reuse the H1 technique proven here (bind a `CSWGui3DSceneView` by tag, engine only creates constructor-bound controls, cover labels by `bindCover`).
- Bind a `CSWGui3DSceneView` into `CSWGuiInGameEquip` (ctor `008a92d0`) with the same H1 technique (block + control ctor + scene ctor + InitControl by tag), and add a `3D_MODEL` control to `equip_p.gui` (it has none).
- Rig: a `charrec_light`-style rig or `inv3d_light` with a body camera hook (the engine's `0077ec20` picks `camerahook`/`camerahookt`/`h`/`z` per party member; `cam->vt[0x74](rig, hook, 1)`, `cam->vt[0x44](22.726)`).
- Build the client creature as SetStats `0084e6f0` does (helper `0076d200`) and set up the scene as `0077ec20` does (idle anim by alignment: `align%d`, `neutral`, `pause1`).
- Rebuild on SetCharacter `008aadc0` (NPC switch) and on creature equip/unequip: hook `FUN_00566130` (equip) / `FUN_00567490` (unequip), called from `EquipItem 008b0320` / `UnequipItem 008b05e0`.
- Risks: creature lifetime (must be freed with the panel), per-frame cost of an animated creature, party NPC switching mid-frame, and the unknown behaviour-param block (`cam->vt[0x80](0x3ea)`).

## Which assets won't fit, and why (R8/R17)
The MDL header bbox is a constant placeholder in every item model; pykotor's per-mesh bb/vertices are zero or empty for blasters. The reliable source is the union of MDX vertices with composed node transforms (`inv3d_model_fit.py`). Six models are EMPTY (no mesh, model count still grows): `w_blstrpstl_006` (rows 12, 104), `w_hvyblstr_006`, `w_ionrfl_004`, `w_blstrcrbn_006`, `w_rptnblstr_004`, `w_blstrrfl_006` (rows 77, 105). Missing variations (e.g. base 72 var 4, base 45 var 9) fall back to `_001`. 15 baseitems rows have no model (category 0: 11, 51, 62–65, 81–84, 86, 87, 92, 101, 108). Sabers carry light nodes and hilt reference nodes (expanded in the bbox). Extent > 1.8: double saber, force pike, war blade; < 0.12: grenades, credits.

## v2 Phase 1 research results — READ THIS FIRST (2026-09-29, commit `f2493fa`)

The raw outputs are in `inventory_3d_viewport_src/research/R12*`–`R17*`. The DB rows are: `class_methods` class `Gob` and `Camera(0098c45c)`; `functions` 008a92d0, 008aadc0, 008ad930, 008b0320, 008b05e0, 008abb30, 0077ec20, 0076d200, 00983da0 and 00515620; `lessons` 91–93.

### Live-state finding (Phase 0)
- At 19:45:07 `patch_config.toml`, `kpm_install_state.json` and `patches/` were rewritten with **14 ids**: the 13 baseline ids plus `buff-duration-hud`. `force-power-hotbar` and `inventory-3d-viewport` are gone, and `patches/` holds 6 DLLs.
- The Inventory 3D session did not do this. It was most likely the load-hang investigation (`09_load_hang_investigation.md`, A/B plan) or the Shield Bar probe session.
- The game process running at the time (started 19:37:37) still had v1 loaded from the earlier install.
- **Before any v2 install:** re-read the live id list and ask the user which set is intended, e.g. whether `force-power-hotbar` should come back and whether the load-hang A/B test is still running.

### R12 Gob vtable (Steam `Gob::vftable` = `0x0098b5cc`, RTTI symbol; 121 real slots, then `__purecall`)
| vt offset (slot) | Address | Meaning (decompile-confirmed unless noted) |
|---|---|---|
| 0x18 (6) | 004e3270 | PlayAnimation(name, speed, 0, 0). Confirmed by usage in 008cd2c0; body not read |
| 0x50 (20) | 00461670 | AttachToPart(parentGob, nodeName, type): `parent->vt[0x10c]` FindPart, builds attach behaviour type 0..3, stores via `vt[0x58]` |
| **0x5C (23)** | 00461490 | **SetPosition(float out[3], x, y, z)** → Gob+0xa4/+0xa8/+0xac, copied to out. thiscall, RET 0x10 |
| **0x60 (24)** | 004614e0 | **SetOrientation(float out[4], w, x, y, z)** → Gob+0xb0..+0xbc. thiscall, RET 0x14 |
| 0x64 (25) | 00459710 | GetPosition(out[3]) |
| **0x68 (26)** | 00459740 | **GetOrientation(out[4])**, i.e. w,x,y,z from +0xb0. Ghidra mislabels this as `CRichEditView::GetMargins` |
| 0xEC (59) | 00459fb0 | GetBoundingBox(min3*, max3*): copies the cached +0x14c (min) and +0x158 (max) |
| 0xF0 (60) | 00460040 | GetMinimumBoundingBox(min3*, max3*): calls `vt[0x1cc]` to recompute, then copies |
| 0xF4 (61) | 004600a0 | GetMinimumBoundingSphere(centre3*, radius*): `vt[0x1d0]`, then +0x168 and +0x164 |
| **0x108 (66)** | 00461530 | GetAttachedParent(-1): returns `*(Gob+0x1b4)`, or 0 if unattached. The Workbench spins only when this is 0 |
| 0x10C (67) | 004587f0 | FindPart(name): "root" → Gob+0x98, else FUN_00456b00(root, name) |
| 0x14C (83) | 004b3c90 | Render: `glPushMatrix; 0049c930(pos=+0xa4, quat=+0xb0)` (glTranslatef(pos) then glRotatef(2·acos(w)·57.3°, x, y, z)); **`glScalef(s,s,s)` with s = float at Gob+0x1e0 when s≠1.0**; draw parts; glPopMatrix |
| 0x1CC (115) | 0045d030 | RecomputeBBox: part `vt[0x74](min, max, scale=+0x1e0)` into +0x14c/+0x158 |

- **The quaternion is W-FIRST** `(w, x, y, z)`, identity `(1, 0, 0, 0)`. This corrects the earlier doc text that said x,y,z,w. The inline product in 008cce70 (and the one in 00515620) is E(a,b) = Hamilton(b,a) with index 0 = w (its cross-term signs are the opposite of Hamilton(a,b); confirmed by the Opus review, 2026-09-29). Our own `Inv3dQuat.h` uses standard Hamilton and never calls the engine helpers.
- **Object scale:** no SetObjectScale setter exists in the vtable. The scale lives in the float at **Gob+0x1e0** (1.0 = unscaled), which Render reads every frame. The DLL can write it directly (IsBadWritePtr guard).
- **Render transform order is translate(pos) · rotate(q) · scale(s).** A model-space point p ends up at `pos + R(q)·(s·p)`. To put the bbox centre c at the origin, set `pos = −R(q)·(s·c)`.

### R13 Camera (the Scene's camera = `*(void**)(view+0x78)`)
- RTTI finds three camera vtables: `Camera::vftable` 0x0098c27c (Gob-like base), **`Camera::vftable` 0x0098c45c**, and `CAurCamera` 0x0098c4f4 (pure).
  - **The vtable at 0x98c45c matches the calls in 008cd2c0 and 0077ec20:** slot 17 (0x44) = 0047f190 SetViewAngle (stores the float at cam+0x204); slot 29 (0x74) = 004819e0, a thunk to 00461670 (AttachToPart); slot 6 (0x18) = 00481080 SetPosition(out, x, y, z) at cam+0xa8..0xb0.
  - The live log must still confirm `*(int*)cam == 0x0098c45c`.
- **The camera is attached to the light-rig node `camerahook`**, and its attach behaviour almost certainly drives its pose every frame. Changing the camera position is therefore unreliable. **v2 fits by moving and scaling the item Gob instead (Gob+0xa4 position, +0x1e0 scale), not by moving the camera.**
- The view angle is 22.726° (`*(float*)0x0099f38c`). Whether it is vertical or horizontal is still unknown; determine it in the live test (see the fit maths in the plan).

### R14 hook suffix
`DAT_00986b34` = `"a"`, so the special hook names are `camerahook0a` and `camerahook4a`, as assumed.

### R15 quaternion axes
- The axis globals are zero on disk and set by static initialisers:
  - `00983d60`: `a7d870 = (1, 0, 0)` = X;
  - `00983d80`: `a7d858 = (0, 1, 0)` = Y;
  - `00983da0`: `a7d864 = (0, 0, 1)` = Z (bytes verified: `d9ee`, `d9ee`, `d9e8` = 0, 0, 1).
- `00515620(out, a, b, c)` is in degrees and ×0.0174533 (`0x98be40`): `qZ = axis(Z, a)`, `qX = axis(X, b)`, `qY = axis(Y, c)`, `out = qZ ⊗ qX ⊗ qY` in standard Hamilton order (the engine product is Hamilton(b,a)).
- `005195a0(out, axis, angleRad)` = (cos(θ/2), axis·sin(θ/2)), w-first. `0x987490` = 2.0.
- **Z is UP.** The Workbench spin is `00515620(-dt·70, 0, 0)`, i.e. about Z. Because the engine product is Hamilton(b,a), the stored result is `cur ⊗ qSpin` in standard order, a rotation about the item's own local Z axis (corrected 2026-09-29; the earlier text said world Z, pre-multiplied). For an upright item this looks the same.
- Constants: `0x9a8948` = 70.0 (double, deg/s); `0x986b50` = 1.0 (double).

### R16 Equipment page (future feature, document only; nothing built)
- The Steam `CSWGuiInGameEquip` ctor is **`008a92d0`** (the only xref of `BTN_SWAPWEAPONS` and `LBL_CANTEQUIP` in a GUI ctor; the other `BTN_SWAPWEAPONS` xref, 007484bd in 00747210, is the main HUD). GOG ctor = 0x59d120.
- **SetCharacter** `008aadc0`: `thiscall(this, charHandle)`, compares against this+0x68; on change, loops over the 11 slots, then `FUN_008ab3f0` refreshes. This is the hook point for an NPC switch.
- **EquipItem** `008b0320`: `thiscall(this, itemHandle, slotMask, flag)`: creature = `FUN_0077d800()`; checks the slot mask against baseitem+4; **`FUN_00566130(...)` = creature-level equip**. **UnequipItem** `008b05e0`: **`FUN_00567490(item, 0x7f000000, 1, 1, 0, 0, 0)`**. Hooking the two creature-level calls catches every equipment change.
- **OnPanelAdded?** `008abb30`: SetCharacter(`FUN_007e5d80()`), `FUN_008b0890(1)`, `FUN_008ac330(...)`. `008ad930` (4767 B) is probably UpdateInventory; its name is unconfirmed.
- `equip_p.gui` tags: BTN_INV_{IMPLANT, HEAD, HANDS, ARM_L, BODY?, ARM_R, WEAP_L, WEAP_R, WEAP_L2, WEAP_R2, BELT}, LBL_ATKL/ATKR/DAMAGE/TOHIT…, LB_ITEMS, BTN_SWAPWEAPONS, BTN_PREVNPC/NEXTNPC, LBL_CANTEQUIP, LBL_DEF_TEXT, LBL_BAR2–5, BTN_BACK and BTN_EQUIP. **There is no 3D control**, so one must be added, as in the v1 inventory.
- **Character-view scene setup `0077ec20`**: `thiscall(this = creature-ish, view, ?, creature, flag)`.
  - It picks the idle anim by alignment (`align%d`, `neutral`, `pause1`).
  - Model 0 is the light rig (`GetModel(0)`). The camera hook is chosen by party member: `camerahook` by default, `camerahookt`, `camerahookh`, or `camerahookz` for Zaalbar. It is checked with `vt[0x10c]` and falls back to `camerahook`.
  - Then: `cam->vt[0x74](rig, hook, 1)`; `cam->vt[0x44](22.726)`; `cam->vt[0x80](0x3ea)` returns a behaviour param block (+0x48 = 1, +0x3c = 0, +0x40 = 0, +0x44 = `this->vt[0xd8]()` height).
- `0076d200` is the creature builder helper called from SetStats `0084e6f0`; it has not been fully read.

### R17 hierarchical bbox survey (answers "what won't fit")
- **Tool:** `scripts/inv3d_model_fit.py`.
  - As a module: `load_model(inst, name)`, `parse_model(mdl, mdx)`, `model_bounds(mdl, mdx, resolver=…)`, `survey_installation(game)`.
  - As a CLI: `inv3d_model_fit.py <game> [out.txt]`, about 30 s.
- **How it works:** it parses the binary MDL itself, because pykotor's per-mesh bb and `vertex_positions` are ZERO or EMPTY for blasters (R9's "tiny blaster" numbers were this bug).
  - Offsets are relative to file+12. Node: type u16 @0, name index u16 @4, pos f3 @16, quat f4 @28 **(w, x, y, z)**, **children (ptr, count) @44/@48**, trimesh header @+80 (MDX stride u32 @+252, vertex offset in the stride @+260, vertex count u16 @+304, render u8 @+313, MDX data offset @+332; **saber meshes (flag 0x800) keep verts in the MDL @+336**). Reference node (flag 0x10) model name @+80, which is how saber hilts are expanded.
  - It composes parent→child `(q ⊗ q_child, p + R(q)·pos_child)` with the root pose ignored, and unions every rendering mesh's vertices.
- **Validated:** long sword ext z 1.255 (matches R9), pistol 0.081×0.405×0.220 (long **y**), rifle 0.745 y, force pike 2.117 z, double saber 2.464 z (blades plus hilt), grenade 0.10, mask 0.156 x.
- **Long axis by class:**
  - **z:** melee, staffs, sabers, gauntlets;
  - **y:** blasters (the barrel points +y), bands, adrenals, medpacs, implants;
  - **x:** belts, datapads, droid parts, masks (width).
- **Outliers** (the full list is at the end of R17):
  - **8 EMPTY variations:** the models exist but contain no mesh at all. They are `w_blstrpstl_006` (rows 12, 104), `w_hvyblstr_006`, `w_ionrfl_004`, `w_blstrcrbn_006`, `w_rptnblstr_004` and `w_blstrrfl_006` (rows 77, 105). The model count still grows, so the v1 "model count didn't grow" retry does NOT catch these; v2 must look them up in the fit table and treat EMPTY like missing.
  - **extent > 1.8:** double saber, force pike, war blade.
  - **extent < 0.12:** all grenades, credits, `w_blstrpstl_013`.
  - **emitter, light or dangly:** every lightsaber (lights, and possibly animated blades), `w_sonicgren_002`, `w_forcepike_001`.
- **Metadata that can make assets fit:** the header bbox is useless (a placeholder); pykotor's mesh bb is unreliable; the real vertex union from the MDX is reliable. Missing variations fall back to `_001`. Empty models count as missing. No-model classes are listed in the v1 section (R8).

## v1 live test + round-2 research — READ THIS FIRST (2026-09-29)

### What is live
- Built from `patch_manager_mods/inventory_3d_viewport_src/` (mirror of `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Inventory3DViewport/`). Hooks:
  - H1 `BindInventoryView` @`0x008A69AD`, original bytes `8B 8D 08 FD FF FF`, param `[ebp-0x2f8]`.
  - H2 `ShowItem` @`0x008A8172`, original bytes `89 45 D4 8D 55 CC`, params `eax`, `[ebp-0x68]`.
  - H3 `InventoryTick` @`0x008A7350`, original bytes `55 8B EC 83 EC 0C`, param `ecx`.
- Model table: `scripts/gen_inventory_3dview_table.py <game dir> <out.inc>` writes `ItemModelTable.inc`, taking about 30 s. Result: 17 rows category 4 (armour), 77 rows category 2, and 15 rows category 0 (11, 51, 62–65, 81–84, 86, 87, 92, 101, 108). This matches the reviewer's expected result exactly. Compile-time kill switch: `INV3D_ONLY_WEAPONS_ARMOR`.
- Static checks passed: the original bytes match the live exe, no hook is within 8 bytes of any other live hook, and the DLL exports exactly the 3 functions.
- **Live install (2026-09-29 19:08), 16 patch ids.** The CLI uninstalls everything first, so ANY later reinstall by any session must pass ALL of these:
  ```
  cd ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/bin/Debug && DISPLAY=:1 ./KPatchLauncher \
    "/home/ethansm/.local/share/Steam/steamapps/common/Knights of the Old Republic II/swkotor2.exe" \
    --patches ~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/inv3d_live_patches \
    k2-visible-console good-alignment-heads k2-aspyr-normal-map-fix k2-aspyr-saber-hilt-texture-fix \
    k2-finesse-melee-fix k2_grass_fog_fix max-lights semi-transparent-letterbox k2-mp3-playback-fix \
    loading_screen_line_fix_k2 k2-aspyr-map-aspect-fix post-combat-movement-fix-v1 \
    additional-console-commands buff-duration-hud force-power-hotbar inventory-3d-viewport
  ```
  - **Staging:** `inv3d_live_patches/` holds the 13 kpatches from `batch3_live_patches/` plus `Inventory3DViewport.kpatch`. `BuffDurationHUD.kpatch` and `ForcePowerHotbar.kpatch` there were **repacked with the DLLs that were live at install time** (the user's choice): Buff HUD `391dbad5…` and Hotbar `c3c8f6d6…`. The kpatches in `Patches/` had newer rebuilds (`8479a3e8…`, `b8c89605…`).
  - **Backup:** `~/Games/KOTOR2_Mod_Workspace/backups/patch_manager_inv3d_pre_install_20260929_190850/` (with `SHA256SUMS.txt`).
  - **Rollback:** rerun the command without `inventory-3d-viewport`.
- **Launch checks:** the Steam log shows `Applied DETOUR hook` at all 3 addresses and `SUCCESS: Patcher initialized`. The `AR_ERROR.LOG` sha is unchanged (`962039…`).

### Live test results (user, 2026-09-29; screenshots in `~/Documents/Screenshot_2026-09-29_19-2*.png`)
From `inv3d_log.txt`, H1 runs once: control id 21, controls 21→22, gate=1, light model OK. The on-screen extent is `[3096,357,556,800]` px (L,T,W,H), which is a **tall** viewport (aspect about 0.70). No crash occurred.

| Class | What the user saw | Root cause (verified, see below) |
|---|---|---|
| Blasters (itemtype 3/4/5) | Large and spinning | `upgitem_light` has `camerahook3/4/5` and `rotatehook3/4/5` under animated `Driverhook*` nodes, so the item is attached and spins. They look large because the camera hooks were tuned for the Workbench's **wide** 254×154 view. |
| Melee (itemtype 0/1/2) | Lying horizontal, not moving | `camerahook0/1/2` exist but **`rotatehook0/1/2` do NOT** (only `rotatehook0a`, which the engine uses for stun baton base 1). So the item is not attached to a spinning driver and sits at the scene origin. The Workbench spins such items itself in its Update `008cce70`; the inventory has no such code. The camera hook is rolled so the long z axis reads horizontal, which suits the Workbench's wide viewport. |
| Masks/visors (itemtype 18) and other non-weapon classes | Tiny, sideways/odd view | `camerahook18` does not exist, and the fallback `"camerahook"` does not exist either (only numbered hooks). So the camera attach fails, the camera keeps a default or previous pose, and the item is not attached. The same applies to every itemtype without hooks: 6, 12–30, 42–51. |
| Armour (itemtype 31–38) | An animated body (idle "neutral") showing only the armour mesh, not fitting | Category 4 attaches `P_MAL_A_MED_01`-style body row 137 through `008cd060`. The engine forces itemtypes 36/37/38 to `camerahook31`. That hook was framed for the Workbench's wide view, so the tall view crops it. Showing only the armour part is expected: it is the armour body model with no head. |
| Some items: droid targeting computer (base 72, var 4) and gauntlets (base 45, var 9) | No model | Log line `models 1 -> 1`, or `2 -> 1`. `<itemclass>_<var>` does not exist: for example `i_drdtrgcom` has only `_001`, and `i_drdshld` has `_001/_004/_005`. The table only checks `_001`. Upgrade items (base 101, `i_upgrade`), plot items (64/65), crystals (11) and pazaak have NO models at all, so they are category 0 by design. |

### Verified engine facts (round 2; raw output in `inventory_3d_viewport_src/research/`)
- **Base-item byte `+0xac` = baseitems.2da `ItemType`.** Raw disasm of loader `006d5240` shows `PUSH 0x99a678 "ItemType"` → `0071bcf0` (2DA get int) → `MOV [row*0xc8 + table + 0xac], CL` at `006d67a7` (file `R10_disasm_006d5240_itemtype.txt`). So `008cd2c0`'s `camerahook%d`/`rotatehook%d` = the item's itemtype.
- **Hook names in `008cd2c0`:**
  - itemtype 0 with base id 1, and itemtype 4 with base id 20 (0x14): the engine appends a suffix, giving `…0a` and `…4a`. The constant `DAT_00986b34` is the suffix, very likely "a"; confirm if needed.
  - itemtypes 36/37/38: forced to `camerahook31`/`rotatehook31`.
  - The camera attaches through `camera->vt[0x74](model0, hookName)`. FOV is set by `camera->vt[0x44](*(float*)0x0099f38c)` = **22.726°**.
  - Then `AddModel(itemResref)`. If `model0->vt[0x10c](rotatehookName)` finds the node, it calls `item->vt[0x50](model0, rotatehook)` (attach) and plays `model0->vt[0x18]("rotate", 1.0, 0, 0)`, which is the spin.
  - Otherwise the item stays unattached at the origin.
  - `powered` animation if `baseitem+0x90`; `neutral` animation if category 4.
- **`upgitem_light` node list** (the live copy is in `override/upgitem_light.mdl` + `.mdx`; also in BIF). Dump: `research/R11_upgitem_light_nodes_2026-09-29.txt`.
  - Camera hooks: `camerahook0, 0a, 1, 2, 3, 4, 4a, 5, 31–36, 39, 40, 41`.
  - `Driverhook0a/3/4/4a/5`, each with child `rotatehook*`, plus `Plane01` and 2 lights.
  - One animation, `rotate` (5.17 s), which rotates each `Driverhook` a full turn: about z for 3/4/4a/5, and about x for 0a.
  - Camera hooks 1/2 sit at (1.56, −0.025, ~0.4) with quat (0, .707, 0, .707). 3/4/5 sit at about (1.0, 0.1–0.27, 0.05) with quat (.5, .5, .5, .5). 31–36 sit at (1.336, −0.088, 1.309).
- **`gui3D_room`** = one node `Box01`. It is only the backdrop.
- **Workbench spin, `008cce70`** (Update, `thiscall(this, float dt)`):
  - `m = GetModel(1)` (`004184b0`, ECX = scene).
  - If `m->vt[0x108](-1) == 0`: `q = 00515620(out, -dt*70.0, 0, 0)` (`_DAT_009a8948` = 70.0 double, in degrees). Then `cur = m->vt[0x68](tmp)` (GetOrientation → quat*), `new = q ⊗ cur`, and `m->vt[0x60](tmp, x, y, z, w)` (SetOrientation, 5 args = GOG `Gob::SetOrientation` param size 20).
  - The meaning of `vt[0x108](-1)` is still unknown: probably "is attached / has parent / animation playing". The spin applies only when it returns 0, i.e. the unattached items.
- **`00515620(float* out, a, b, c)`**: an Euler-degrees → quaternion builder. It multiplies each angle by `_DAT_0098be40` = 0.0174533 (deg→rad) and combines three axis quaternions `FUN_005195a0(axis globals 00a7d870/00a7d864/00a7d858, angle)`. Arg a = the first axis (yaw in the Workbench spin). The mapping of a/b/c to x/y/z is not yet confirmed.
- **Gob vtable (from `008cd2c0`/`008cce70` usage plus GOG param sizes):** 0x18 PlayAnimation(name, speed, 0, 0); 0x34 set texture(?) (`&DAT_00986925`, texResref); 0x50 AttachToPart/Object(parent, nodeName); 0x60 SetOrientation(tmp, x, y, z, w); 0x68 GetOrientation(out) → float[4]; 0x108 (?) (−1); 0x10c FindPart(name) → nonzero if found. Camera (scene+0x18 = view+0x78): 0x74 AttachToPart(model, nodeName[, flag]); 0x44 SetViewAngle(float deg).
- **MDL header bbox/radius is a constant placeholder in every item model:** (−5,−5,−1)..(5,5,10), r=7. It is useless for fitting (`R8_item_model_survey_2026-09-29.txt`).
  - Per-mesh `bb_min`/`bb_max` are real (`R9_mesh_bbox_samples_2026-09-29.txt`; node rotations ignored, so treat it as approximate).
  - Sizes range from about 0.1 (grenade) through about 1.25 (long sword) to 2.15 (force pike), which is more than 20×. No fixed camera distance fits them all, so v2 needs a per-item fit.
  - The long axis is z for melee, sabers, staffs and gauntlets; y for the war-axe, belts, bands and adrenals; x for datapads and droid parts.
- **Existing model variations per itemclass** are listed in R8, e.g. `w_blstrpstl` has 15, `i_mask` 19, and `i_drdtrgcom` only 1. So `ShowItem` must fall back when `<class>_<var>` is missing.
- **Character screen (future equipment-page view reference), `CSWGuiInGameCharacter`, Steam ctor `0084c3a0`:**
  - `LBL_3DCHAR` is a `CSWGui3DSceneView` at `this+0x3c24` (`param_1[0xf09]`, vftable 0x9a3d34), bound with `InitControl`. `BTN_3DCHAR` is at `this+0x3cb8`.
  - Scene init: `gui3D_room` plus **`charrec_light`**, then `camera->vt[0x74](light, "camerahook", 1)` and `vt[0x44](22.726)`.
  - It creates a 0x470-byte client creature object `FUN_007645f0` at `this+0x4b6c`.
  - In SetStats `0084e6f0` (GOG `CSWGuiInGameCharacter::SetStats` 0x597ff0; Steam = GOG + **0x2B6700** for this class), when the creature or its appearance changes (cache `this+0x4b70`/`+0x4b74`), it does:
    - `obj->vt[0xec](serverCreature->vt[0x148](…))`, `FUN_0076d200(appearanceCopy, 3, 1, 0)`, `FUN_0085d430(…)`, `obj->vt[0x94](scene@this+0x3c98)`;
    - SetPosition `vt[0x8c]`(0,0,0), SetFacing `vt[0x88]`(1,0,0);
    - `FUN_0077ec20(view, creature, 0, 1)`.
  - Rotation: RotateCharacter `008500f0(this, dir)` (left `00850330`, right `00850550`, bound to input events `0x16256`/`0x16257` through `FUN_00418af0`; HandleInputEvent `0084f900` maps events 0x3b/0x3c). It rotates the facing vector at `obj+0x30` by the quat from `00515620(step,0,0)` and sets it with `obj->vt[0x88]`.
  - **So the character screen rotates by fixed key steps; there is no mouse-drag code to copy.** Drag in v2 = poll Win32 (`GetAsyncKeyState(VK_LBUTTON)`, `GetCursorPos`/`ScreenToClient`) in H3 against the view's pixel extent (view+4..+0x10).
- GOG address DB (`tools/kotor-patch-manager/AddressDatabases/kotor2_gog_aspyr.db`) has named `Gob`/`Camera` functions (e.g. `Gob::SetOrientation` 00853a70 p20, `Gob::GetBoundingBox` 0084c590 p8, `Camera::SetViewAngle` 008c0300, `Camera::SetPosition` 008c21f0 p16). There are no Steam addresses for those yet. The Steam DB (`kotor2_steam_aspyr.db`) has only 48 rows.

### Inventory layout facts (`override/inventory_p.gui`, 800×600 GUI units)
LB_ITEMS `[161,134,236,329]`, LB_DESCRIPTION `[407,134,232,300]`, LBL_CREDITS `[407,442,223,28]`, BTN_USEITEM `[161,470,227,28]`, bars LBL_BAR1/2 at y 105/125 across 34..766, and 3D_MODEL `[645,134,116,300]`. Workbench `upgrade_p.gui`: 3D_MODEL `[164,117,254,154]`, 3D_MODEL_LS `[228,115,125,228]`. Character `character_p.gui`: LBL_3DCHAR/BTN_3DCHAR `[166,134,230,360]`. The ultrawide generator (`scripts/generate_ultrawide_gui_overrides.py`) rewrites these files. `scripts/add_inventory_3dview_control.py` (`--replace --rect L,T,W,H`, `--rollback`) owns the 3D_MODEL control.

## Planning session findings (2026-09-29, Opus)

Four read-only Ghidra runs (raw output: `inventory_3d_viewport_src/research/ghidra_run{1..4}_2026-09-29.txt`) changed the design:

1. **Adding a `CONTROL_3DVIEW` to `inventory_p.gui` alone does nothing.** Panels bind fixed members by tag in their constructor (`StartLoadFromLayout` -> `InitControl` -> `StopLoadFromLayout`). The inventory has no scene-view member, so the DLL allocates and constructs one and binds it inside the ctor layout window.
2. **`008cd2c0` can be reused unchanged** with a fake `this` = `block - 0x3a30` (reads only `+0x3a30`, `+0x3ac4`, `+0x3d2c`, `+0x3d34`). **R1 must verify from raw disasm before any code.**
3. **Inventory item objects match** what the render function expects (`+0xc` base item id).

| What | Address | Notes |
|---|---|---|
| `CSWGuiUpgrade` ctor | `008c9e10` | 2 embedded `CSWGui3DSceneView` at `+0x3a30`, `+0x3ac4` (0x94 bytes each) |
| `CSWGuiInGameInventory` ctor | `008a6170` | binds 21 controls, no scene view |
| `CSWGuiControl` base ctor | `00418990` | fastcall(ECX=ctrl) |
| `CSWGui3DSceneView::vftable` | `009a3d34` | |
| `CSWGuiScene` ctor | `00417f40` | scene = view+0x60 (verify) |
| `StartLoadFromLayout` / `InitControl` / `StopLoadFromLayout` | `0040ee40` / `0040f620` / `0040f5a0` | InitControl stores at panel+0x24 index = ctrl+0x54 |
| `AddEvent` | `00418af0` | |
| `CExoString` ctor/dtor, operator new | `00733570` / `00733780` / `00919723` | |
| `AddModel` | `00418360` | |
| Workbench render | `008cd2c0` | fastcall(ECX=this); gated by `*(int*)0x009f42b4` |
| armor resolver | `008cd060` | called with row 0x89 = 137 = `P_MAL_A_MED_01` (fixed male medium body) |
| non-armor resref builder / base item lookup | `006d5000` / `006d6e30` | |
| Workbench Update (vt13) | `008cce70` | does NOT call the render; only spins model 1 |
| Inventory vtable `009a72ac` | slots 0/13/15/18/19 = `008a7030`/`008a7350`/`008a8880`/`008a74c0`/`008a7520` | |
| Inventory members | `LB_DESCRIPTION` +0x878, `LB_ITEMS` +0x588, entries +0x24b0 | |
| Inventory populate / CreateItemEntry | `008a7ba0` / `008a75f0` | |

**Design (v1):** GUI script adds `3D_MODEL` (ID 21, rect `[645,134,116,300]`) to `inventory_p.gui`; a DLL hooks (H1) the inventory ctor to allocate+construct+bind a view and init the scene, (H2) the item-shown/description-refresh path to call `008cd2c0` with fake this, (H3) Update for logging/poll fallback.

**Superseded:** the old "Phase 1 — copy CONTROL_3DVIEW" step is insufficient on its own (see 1 above). The "PC-own-appearance-row" Branch B is superseded: v1 uses the Workbench's fixed row 137. A PC-specific body is a possible v2, by calling `008cd060` directly.

## Phase 1 results (2026-09-29, raw disasm; evidence in `inventory_3d_viewport_src/research/R*_*.txt`)

- **R1 PASS (GATE A).** `008cd2c0` keeps `this` at `[ebp-0xac]`; it only reads `+0x3a30`, `+0x3ac4` (category==1 only), `+0x3d2c` (byte) and `+0x3d34` (item ptr, never written or freed). `008cd060` is called with `ECX=this` but only spills it (`[ebp-0x78]`, never read). Scene calls use `ECX=view+0x60`. Item fields read: `+0xc`, `+0x2a4`, `+0x2a6`. `GetModel(0)` must be `upgitem_light` (hook nodes), so scene init is mandatory. Use category 2 (never 1) so the saber view (`block+0x94`) is untouched. Fake-`this` offsets: category = `block+0x2fc`, item = `block+0x304`.
- **R2.** `00418990` ECX=`this+0x3a30`; vftable `0x9a3d34` stored at `[view]`; `00417f40` ECX=`view+0x60`; `InitControl(panel, view, &CExoString("3D_MODEL"), 1, 1)`. Scene init: `Scene*=[view+0x60+0x14]`; `(*Scene)->vt[0x70]` thiscall with 3 stack args (`"gui3D_room"` char*, float pos[3]={0,0,0}, float quat[4]={0,0,0,1}); then `AddModel(scene,&CExoString("upgitem_light"),-1)`. Gated on `*(int*)0x009f42b4`.
- **R3 (H1 site).** `0x008A69AD`, original bytes `8B 8D 08 FD FF FF`, param `[ebp-0x2f8]` = panel. Immediately before `StopLoadFromLayout` (`008a69b3`), so the GFF is still open. No branch targets. Verified against the live exe.
- **R4 (H2 site).** Entry event 0 handler `008a8100(panel=ECX, entry)`: item = `EAX` after `CALL 0051c0e0` (`008a816d`). Splice `0x008A8172`, original `89 45 D4 8D 55 CC`, params `eax` (item) and `[ebp-0x68]` (panel). Verified vs the live exe. Known gap: the "Error: Invalid item" branch (`entry+0x1d0==0x7f000000`) returns before the splice, so the preview would not clear there. The selected object id is also in `panel+0x24c8`.
- **R5 (H3 site).** `0x008A7350`, `55 8B EC 83 EC 0C`, params `ecx` and `[esp+4]` (float dt). Verified vs the live exe.
- **R6.** The panel is `0x24d4` bytes, built by factory `007d0760` / manager init `007be4c0` once per GUI init. Base dtor `0040e860` does not delete `panel+0x24` list entries, so v1 leaks the `0x400` block (log every construction) and there is no double free.
- **R7.** `CSWGui3DSceneView` vtable differs from `CSWGuiControl` at slot 0 (`0084de70`), slot 1 (`00420f50`, extent override) and slot 14 (`0084de40`, probably Draw). Informational.

## Phase 2 (GUI) DEPLOYED 2026-09-29

`scripts/add_inventory_3dview_control.py` appended `3D_MODEL` (copy of `upgrade_p.gui`'s, CONTROLTYPE 4, ID 21, EXTENT `[645,134,116,300]`, empty BORDER resrefs) to the live `override/inventory_p.gui`. New sha256 `46d84b7d5a4d…` (was `5980756fa20a…` = generator manifest `deployed_sha256`). Backup: `~/Games/KOTOR2_Mod_Workspace/backups/inventory3d_20260929_183902_373234/`. Rollback: `.venv/bin/python scripts/add_inventory_3dview_control.py --rollback` (restores the OLDEST backup). Options `--rect L,T,W,H --replace --dry-run`. Idempotent. **Generator interaction:** `generate_ultrawide_gui_overrides.py` refuses to overwrite a live file whose sha is neither the recorded original, the previous deploy nor its output ("foreign file"), so after this deploy a generator re-run for `inventory_p` fails loudly: run `--rollback` here first, re-run the generator, then re-apply this script. pykotor gotchas: `GFFStruct.get_struct()` returns a COPY (edit then `set_struct`); `get_list` may return a detached list (`set_list` after append); `ID` is Int32 (`get_int32`). Effect in game before the DLL exists: none (unbound control is ignored) — not yet visually confirmed in game.

## Phase 2 findings (2026-09-29)

**Methodology shortcut found — check this FIRST for any future mod using
this framework**: the framework's `AddressDatabases/kotor2_gog_aspyr.db`
already has fully-mapped function tables (real names, addresses, calling
conventions) for every class this mod needs — `CSWGuiUpgrade`,
`CSWGuiUpgradeSelection`, `CSWGuiUpgradeItemSelect`, `CSWGuiInGameInventory`,
`CSWUpgradeItemEntry`, `CSWItem`, `CSWCItem`, `CSWGuiScene`, `CAurScene`/
`Scene`, `Gob`, `CSWBaseItem` — none of which any *shipped* GOG-port
candidate actually uses. There's no GOG binary on this machine to
byte-diff against, but the class/method **names** and relative shape are
still a massive head start: confirm the same RTTI class exists on our
Steam binary (fast), then use GOG's named functions as a hypothesis list
for what a given Steam vtable slot does, verified by decompile — never
assumed from name/position alone (see the `lessons` table for a case where
that assumption was wrong). Query every `AddressDatabases/*.db`, not just
the versions being actively ported, before starting fresh RTTI work.

**Real class names confirmed via RTTI on our Steam binary (overturning
every SDK-style guess in this doc's earlier drafts):**
- `CSWGuiInGameInventory` (vftable `0x009a72ac`) — the real inventory
  screen class. **Not `CUIInventory`.**
- `CSWGuiUpgrade` (`0x009a88a4`), `CSWGuiUpgradeSelection` (`0x009a86cc`),
  `CSWGuiUpgradeItemSelect` (`0x009a876c`) — the three real Workbench
  screen classes (main panel / category-filter sub-screen / item-to-upgrade
  sub-screen). Per GOG, `CSWGuiUpgrade` owns `Update3DView`,
  `GetArmorModelAndTextureResRef`, `OnSlotSelected`, `OnUpgradeSelected`,
  `VerifyUpgrade`, `InsertUpgrade` as real named member functions —
  **`GetArmorModelAndTextureResRef` existing directly on the Workbench
  class is a significant simplification of the Branch A/B split below**:
  the Workbench itself already resolves armor model+texture together, not
  just standalone weapon models.
- `CSWUpgradeItemEntry` (`0x009a899c`) — per-slot item-entry widget.
- `CSWItem` (`0x0099a8a0`) — UI-side item wrapper (`GetBaseItem()` per
  GOG). `CSWCItem` (two RTTI hits, `0x009a25fc` and `0x009a2748`,
  relationship not yet resolved) — the in-world/rendered item object
  (`LoadModel`, `SpawnVisualEffect`, `PowerItem`, etc. per GOG).
- `CSWGuiScene` (`0x0098771c`) — the scene embedded in a `CONTROL_3DVIEW`.
  **Confirmed render-call target**: GOG has
  `CSWGuiScene::AddModel(CExoString* model, int insertIndex)` — takes a
  model **resref string directly**. Only a 5-slot vtable (mostly
  `CSWGuiObject` base overrides) — `AddModel` is very likely a plain
  (non-virtual) member function, so it won't show up in a vtable dump;
  needs a direct xref/string-anchor search on Steam, not attempted yet.
- `CSWGui3DSceneView` (`0x009a3d34`) — the `CONTROL_3DVIEW` control itself
  (wraps a `CSWGuiScene`, per the framework's own `CSWGui3DSceneView.h`).
- `CAurScene` (`0x0098bc7c`, mostly `__purecall` — an abstract interface)
  and `Gob` (`0x0098b5cc`, 60-slot vtable — the generic renderable-object
  class `AddModel` returns) — both confirmed present, not yet needed at
  function level.

**Vtable-shape finding**: `CSWGuiUpgrade`/`CSWGuiUpgradeSelection`/
`CSWGuiUpgradeItemSelect`/`CSWGuiInGameInventory` all share an *identical*
29-slot vtable shape (common GUI-panel base class), diverging only at
slots 13/15/18/19 — each class's own overrides. On `CSWGuiUpgrade`
specifically:
- **Slot 13** (`FUN_008cce70`, `(this, float dt)`) — confirmed per-frame
  `Update`. Gated by a flag at instance `+0x3d2c`; when set, computes
  `-dt * (a constant)` — consistent with animating the previewed item's
  spin/rotation in the 3D view every frame.
- **Slot 19** (`FUN_008cca70`) — likely `OnPanelRemoved`/teardown: zeroes
  the same `+0x3d2c` flag and frees a handle at `+0x3d38` (likely the
  preview item/model handle).
- **Slot 15** (`FUN_008cfc10`, `(this, int, int)`) — likely
  `OnControlEntered`, toggles two related UI control states.
- **Slot 18** (`FUN_008cbde0`, 3215 bytes, the largest override) — **ruled
  out as `HandleInputEvent`** despite being the biggest function (no
  switch/case shape in the decompile). Instead resolves `baseitems.2da`
  model-variant columns by literal name (`"DoubleMdlVar"`,
  `"ShortMdlVar"`, `"LongMdlVar"`, `"UpgradeType"`, `"Template"`) —
  real, useful confirmation that this class does exactly the kind of
  `baseitems.2da`-driven model resolution Branch A needs, but not yet
  matched to one specific GOG-named method with confidence.

**RESOLVED (continuation session, 2026-09-29) — the render call, the armor
data path, and the Branch A/B merge are all now decompile-confirmed.**
Per the resume plan's Step 1 ("decompile the vtable slot's own unexamined
helper calls"), decompiling the 4 functions `FUN_008cbde0` (slot 18) calls
— `FUN_008cb2f0`, `FUN_008cb230`, `FUN_008cd2c0`, `FUN_008cd8e0` — resolved
nearly everything in one batch:

- **`CSWGuiScene::AddModel` — Steam address `0x00418360`, CONFIRMED.**
  `__thiscall(this=Scene, modelResrefString, insertIndex)`: resolves the
  model reference via a registry/factory lookup, then calls
  `FUN_004183b0` (a Gob-array insert helper: null-checks the Gob,
  bounds-checks `insertIndex` against `this+0x20` — the scene's Gob
  count — appends or inserts, then calls a `SetScene`-shaped virtual on
  the new Gob at `vtbl+0x4c`), returning the created `Gob*` or null.
- **The 3D-preview resolve+render function — Steam address `0x008cd2c0`**
  (called from `CSWGuiUpgrade::Update`, vtbl[13], when the `+0x3d2c`
  flag/category byte is set). **This one function contains the entire
  Branch A/B split, decompile-confirmed**: it reads `+0x3d2c` (item
  category 1-4); category `4` (armor) branches into
  `GetArmorModelAndTextureResRef`; every other category uses a direct
  sprintf-based model-resref builder (`FUN_006d5000`, `"%s_%03d"` /
  `"%s_%c_%03d"`). **Both paths converge on the same single call** to
  `AddModel(modelResref, insertIndex=-1)` — i.e. **the Branch A/B
  shared-render-call hypothesis is CONFIRMED, not just plausible.** Both
  paths finish by setting an animation-state string on the Gob
  (`"rotate"`/`"powered"`/`"neutral"` via `vtbl+0x18`).
- **`GetArmorModelAndTextureResRef` — Steam address `0x008cd060`,
  CONFIRMED.** Resolves **both** a model resref and a texture resref:
  clamps the `bodyvar` index (param) to 1-14 and adds `'@'` to produce a
  letter A-N, referencing the string literal `"CModelA"` — this is
  exactly `appearance.2da`'s `modelA..modelN`/`texA..texN` column
  convention, independently confirming the doc's "PC-own-appearance-row
  lookup" mechanism for armor. Texture name built via `"%s%02d"` from a
  race/variant-derived index, with 2 fallback texture-existence checks
  before settling on a final index.
- **`FUN_008cbde0` (slot 18) reclassified again**: it is neither
  `HandleInputEvent` (ruled out previously) nor the render call itself —
  it's an item-category-driven **UI-slot visibility toggler** (shows/hides
  upgrade-slot controls based on the `+0xf4b` category byte) that calls
  the 4 functions above as siblings, not as its own body. Two of those
  siblings are now named: `FUN_008cb230` = `GetSlotCountForCategory`
  (returns 6/3/3/2 slots for categories 1-4 via a jump table, reading the
  same `+0x3d2c` byte); `FUN_008cd8e0` = upgrade-slot **icon** refresh
  (2D UI, unrelated to the 3D render path, ruled out).

Full addresses, decompiled bodies, and evidence trail:
`ghidra_knowledge` DB's `classes`/`functions`/`lessons` tables (scope
`Inventory 3D Viewport`, topic "Step 1 SUCCEEDED").

**Still open, low priority**: `OnSlotSelected`/`OnUpgradeSelected`'s exact
Steam addresses are not individually pinned down (the UI-visibility and
icon-refresh functions found above are closely related but not confirmed
1:1 name matches to those two specific GOG names) — not blocking, since
the render pipeline itself (data resolution → `AddModel` → animate) is
now fully mapped end-to-end, which is what Phase 3's hook needs. The two
`CSWCItem` RTTI hits' relationship (Step 4 in the resume plan) also
remains open and low-priority.

## Goal

Render a live 3D model (`.mdl`/`.mdx`) of whatever item is currently
selected in the inventory list, in the empty ultrawide space beside the 2D
inventory panel.

**Better reference than the character screen: the workbench upgrade
screen (`upgrade_p.gui`, `CUIUpgrade` or similar).** The Character/
Equipment screen's 3D view renders a *creature* (the PC/party member
wearing gear) — that's a different data path (creature model + equipped
item attachments) from what we want. The **workbench item-modification
screen renders the selected *item itself*** — you pick an item from a
list, and it shows that item's standalone `.mdl` in a 3D view, with no
creature involved. That's exactly the trigger → resolve → render sequence
the inventory viewport needs, just in a different GUI. Two things make
this doubly useful here:
- It's already present in this exact build and already ultrawide-adjusted
  — `upgrade_p` is one of the four 3D-view GUIs fixed in Phase 4 of our
  ultrawide pass (see [[kotor2-ultrawide-gui]]: "3D views
  (classsel_p/maincg_p/partyselect_p/upgrade_p)"), so its overshoot/
  positioning problem is already solved and documented — no need to
  rediscover that fix for the inventory case.
- Mod #153 (Workbench Upgrade Screen Camera Tweak, DarthParametric,
  `deadlystream.com/files/file/1743-`) is a **loose-file mod that already
  edits this screen's camera/3D-view data** — worth pulling apart as a
  known-good precedent for how the item 3D view's camera/positioning GFF
  fields work, and as a compatibility check (our new control must not
  collide with what that mod already touches on `upgrade_p`).

## Why this is a Patch Manager task, not a GUI/TSLPatcher task

`inventory.gui` has no `CONTROL_3DVIEW` control and the inventory
listbox-selection handler in `CUIInventory` only ever updates a 2D `.tga`
icon. Getting a live model on screen requires:

1. A new UI control the engine will actually render into (a GFF struct
   edit — doable today with Holocron Toolset, no C++ needed; copy the
   `CONTROL_3DVIEW` struct from `upgrade_p.gui` rather than the character
   screen, since it's already configured for a standalone item, not a
   creature).
2. **The engine to be told, at the moment of selection, to resolve the
   selected item's model and push it into that control.** This logic does
   not exist anywhere in the inventory code path today; it only exists in
   the workbench/character screens' own render calls. This part requires a
   runtime detour — there is no NWScript event or 2DA hook for "item
   selected in inventory listbox."

## Armor/robes need a different render path than weapons (confirmed on disk, 2026-09-28)

External review (Gemini) flagged that armor has no standalone 3D model in
this engine — checked against our actual installed `appearance.2da` and
`baseitems.2da`, and the claim is correct, with the exact mechanism more
precise than the original suggestion:

- `baseitems.2da` has a **`bodyvar`** column (plus `genderspecific`). An
  armor's base item type carries a *body-variation index*, not a model
  reference.
- `appearance.2da` has 14 body-slot columns per race/gender row
  (`modela`/`texa` through `modeln`/`texn`). `bodyvar` only means anything
  as an index into *one specific creature's* row — there is no
  armor-shaped `.mdl` sitting on disk waiting to be loaded standalone. The
  same armor also renders differently per race/gender since each has its
  own row.
- Net effect: an armor/robe item literally has no visual identity outside
  the context of a specific wearer. The Workbench standalone-item render
  path (Phase 1-3 below) is correct and sufficient for **weapons, shields,
  and headgear** (these use `defaultmodel`/a direct model reference, not
  `bodyvar`), but **cannot** work for armor/robes — there is nothing
  standalone to resolve and render.

**Adopted: hybrid render path.** Branch on the selected item's
`baseitems.2da` row:
- `bodyvar` unset/zero (weapon, shield, headgear, most other slots) →
  original Workbench-style standalone-item render, unchanged.
- `bodyvar` set (armor, robes) → resolve the *player character's own*
  race/gender appearance row, index `modela..n`/`texa..n` by `bodyvar`, and
  render that body mesh+texture — no separate creature object needs to be
  spawned, since the PC's own appearance row already has everything
  needed. This is simpler than "spawn a dummy `CNWSCreature`" (see below)
  and reuses data the game already resolves every time the PC's worn-armor
  slot changes in the world.

**Where the external suggestion needs Phase-2 verification, not blind
adoption:**
- *"Spawn a dummy creature, equip it, wrap it in a hologram shader"* — this
  is a bigger, riskier lift than the `appearance.2da` row-lookup approach
  above (runtime object lifecycle, must not register with
  party/faction/AI systems, must be cleaned up reliably on screen close).
  Not adopted as the primary plan; the PC's-own-appearance-row lookup
  achieves the same visual result (a body wearing the armor) without
  spawning anything. Revisit only if the direct row-lookup turns out to be
  blocked by something Ghidra finds in Phase 2.
- *`CNWSCreature` as the class name* — this is an NWN/Aurora Toolset SDK
  name. No evidence in our own `AddressDatabases`/existing patches that
  KotOR's compiled binary uses it internally; don't assume NWN symbol
  names carry over. Phase 2 must independently identify whatever struct
  layout the binary actually uses.
- *A reusable "native hologram VFX"* — partially confirmed: `Scene::` does
  have hologram-aware code (`Scene::DoHologramGobBuckets`, already named in
  Patch Manager's `Max Lights` patch,
  `Patches/Max Lights/kotor2-steam-aspyr.hooks.toml:44`), so the concept is
  real in this engine, not invented. But that specific function only tunes
  a lighting-bucket *count* — it's evidence a hologram-rendering subsystem
  exists, not proof a "make this mesh holographic" toggle is one hook
  away. Low priority since the row-lookup approach above doesn't need it,
  but worth a note if Phase 2 Ghidra work stumbles onto the real shader
  path anyway — a holographic preview would be a nice cosmetic upgrade
  later, just not on the critical path.

## Implementation pipeline (from the original spec, refined)

### Phase 1 — UI canvas prep (data layer, no C++)
1. Extract `inventory.gui` and `upgrade_p.gui` via Holocron Toolset.
2. Locate the `CONTROL_3DVIEW` struct in `upgrade_p.gui` and copy it into
   `inventory.gui`, since its camera/framing is already set up for a
   standalone item rather than a worn creature.
3. Assign an unused control ID and position it in the ultrawide margin.
   **Coordinate with `popos/docs/kotor2_ultrawide.md`** — our existing
   ultrawide pass already defines the content-vs-margin boundary for every
   menu GUI, and already solved this exact 3D-view-overshoot problem for
   `upgrade_p` itself; this control must live in the margin we've already
   reserved and reuse that fix, not re-litigate it.
4. Diff against mod #153's loose-file changes to `upgrade_p` first, so we
   know which camera/GFF fields it already touches before we copy from it.
5. This phase is independently testable/committable even before any C++
   exists — the control will just render nothing until Phase 3 wires it up.

### Phase 2 — engine mapping (Ghidra static analysis)
Now two branches, per the armor-vs-weapon split above — not one uniform
path. Community GZF/SARIF databases should be checked first for both, same
as before.

**2026-09-29 update — see "Phase 2 findings" above for the full confirmed
picture.** Real class names found for every target below (`CUIUpgrade` →
`CSWGuiUpgrade`, `CUIInventory` → `CSWGuiInGameInventory`, `CUI3DView` →
`CSWGui3DSceneView`/`CSWGuiScene`). **Branch A/B render-call question
answered: they very likely DO share one call.** `CSWGuiUpgrade` itself
owns both `Update3DView` (the render/update call) and
`GetArmorModelAndTextureResRef` (armor-specific data resolution) as real
named methods per the GOG address database — i.e. the same class already
handles both the standalone-item case (Branch A) and the
armor-model+texture case (Branch B) internally, which is stronger evidence
for "one shared render call, two data-resolution paths" than the original
spec's guess that Branch B would need the separate Character/Equipment
screen. Exact Steam addresses for these specific named methods are still
open — see "Phase 2 findings" for the recommended next search technique.

**Branch A — weapons/shields/headgear (standalone item, unchanged from
original spec).** Primary reference: the workbench item-modification flow
(`upgrade_p`/`CUIUpgrade` or equivalent) — item-selection → `.mdl`
resolution → 3D render for a standalone item:
1. **Trigger**: the workbench's item-list selection handler (structurally
   the closest analog to `CUIInventory`'s own listbox handler).
2. **Data resolution**: the routine that reads a selected item's `.uti` →
   `defaultmodel`/direct model reference (from `baseitems.2da`) — likely
   shared with the inventory 2D icon lookup.
3. **Render call**: the `CUI3DView` init/update function as called by
   `upgrade_p` for the selected item.

**Branch B — armor/robes (PC-own-appearance-row lookup; possible v2 only, not v1: a PC-specific body would call `008cd060` directly).** No
standalone model exists (see above), so this branch needs different
addresses, likely from the *Character/Equipment screen's* own render path
(the one place the engine already does a live "PC wearing X" render) rather
than the workbench:
1. **Data resolution**: the routine that, given a `baseitems.2da` `bodyvar`
   index and the PC's own `appearance.2da` row, resolves which
   `modelX`/`texX` pair to load — this almost certainly already exists,
   since it's exactly what runs every time the PC equips armor in the
   world; find it via the Character screen's equip-slot-changed handler
   rather than writing new resolution logic.
2. **Render call**: same `CUI3DView`-family call as Branch A, just fed the
   PC's body model/texture instead of a standalone item model. Confirm
   whether this is literally the same function as Branch A's Target 3 with
   different arguments, or a distinct one — if the former, Phase 3's C++
   collapses to one shared render call with two data-resolution paths
   feeding it.

### Phase 3 — hook construction (`.kpatch`)
1. New patch directory under Patch Manager's `/Patches`.
2. `manifest.toml` declaring K2 Steam/Aspyr build compatibility (confirm
   whether this needs the *exact* build hash we're on — see
   `03_framework_research.md` for whatever the framework's version-pinning
   story turns out to be; also see `05_build_plan.md`'s Phase 0 outcome —
   our live exe's LAA-patched hash currently matches no manifest entry,
   needs resolving before install regardless of this mod).
3. `hooks.toml`: detour on the inventory listbox-selection trigger.
   `skip_original_bytes = false` — the inventory must keep updating its 2D
   icon as before; we're augmenting, not replacing.
   WI-03 recon: `hooks.toml:18` (`address = 0x008A8172`) is the inventory entry event-0 handler (`008a8100`) right after `GetItemByObjectId`; the match to the listbox-selection trigger is by source comment only, not re-verified against Ghidra.
4. C++ detour: read selected item's `baseitems.2da` row → branch on
   `bodyvar` → (Branch A) resolve `.mdl` string directly, or (Branch B)
   resolve PC's own `modelX`/`texX` via `bodyvar` index → call the render
   function with our control ID either way.
   *Implementation note (WI-03 recon):* `bodyvar` is folded into `kItemCategory == 4` by `scripts/gen_inventory_3dview_table.py:40-41` at generation time (`modeltype == '1'` or `bodyvar` not in `('', '****')`). The runtime `ShowItem` (`Inventory3DViewport.cpp:477`) reads only `kItemCategory[id]` (line 493), and the `cat == 4` branch (line 269, outside `ShowItem`) tests `cat` only, so no `bodyvar` value reaches runtime and no 2DA reader exists in `inventory_3d_viewport_src/`. `FnRender` `0x008cd2c0` is `fastcall(fakeThis)` with no control-ID parameter, so "call the render with our control ID" is realised by the fake-this/view offset (`FakeThisDelta`), not an argument.
5. Build via `create-patch.py`/`.bat` → `.kpatch`.

### Phase 4 — runtime verification
Load via `KPatchLauncher.exe`, confirm `KotorPatcher.dll` injects, select
several item types across both branches — weapon (Branch A), armor
(Branch B, check it matches the PC's actual race/gender), and a stacked
consumable with neither a model nor a `bodyvar` (neither branch — confirm
the fallback, not a crash).

## Open risks specific to our build

- **Ultrawide layout collision.** Our ultrawide pass already treats 3D
  views as a special case — see [[kotor2-ultrawide-gui]]: "kept 3D views
  overshot their frames (character view covered the stats)... The engine
  renders 3D views at real pixel aspect." A new inventory 3D view will hit
  the exact same overshoot problem and needs the same fix pattern (content
  vs. margin classification), not a fresh one.
- **Model availability.** Armor/robes are not a v1 case: v1 uses the Workbench's fixed row 137, and Branch B (see above) is a possible v2 that would call `008cd060` directly. The remaining gap is items with
  neither a direct model nor a `bodyvar` — stacked consumables (medpacs,
  grenades) that may only have icon art. Need a fallback (hide the 3D
  control, or show nothing) rather than crashing on a null result from
  either branch.
- **New risk from Branch B**: rendering "PC wearing this armor" means the
  preview reflects the PC's *current* race/gender/body — correct behavior
  (that's genuinely what it'll look like worn), but confirm the render call
  doesn't require the PC to *actually* be wearing the item first, or the
  hook needs to fake that state for the render without touching the real
  equipped-item slot (which would be a gameplay-affecting bug, not just a
  UI one).
- **Texture/HD overrides.** This build has Ultimate Character Overhaul,
  HD NPC/PC Portraits, and several appearance-change mods installed (mods
  #15-16, #46-47, etc.). Whatever `.mdl`/`.mdx` the render call pulls needs
  to resolve through the same override chain those mods use, not the
  vanilla chitin path, or the 3D preview will show unmodded vanilla art.
- **Performance**: a live 3D render on every listbox selection, at 60 FPS,
  in a menu that's already being redrawn by our upscaled GUI textures —
  should be cheap (character screen already does this every frame) but
  worth a MangoHud check per [[kotor2-ultrawide-gui]]'s existing perf
  monitoring setup.

## Effort estimate (rough, pre-Ghidra)

Phase 1 (GUI struct copy): hours, once Holocron Toolset workflow is
confirmed. Phase 2 (Ghidra): the long pole — hours to days depending on
whether a community GZF database already has these three functions
labeled. Phase 3-4 (hook + C++): hours once addresses are known, assuming
the Patch Manager toolchain builds and runs under our Proton setup (open
question, see `03_framework_research.md`).


## Planned shared state and shared code (doc 19 / 19b, added 2026-10-02)
**Nothing here is built yet.** When a piece lands, use it instead of extending this patch's local copy. The ids (A1, B2, ...) refer to [`19b_cross_patch_sharing_survey.md`](19b_cross_patch_sharing_survey.md), which has the evidence. Line numbers there are from 10-02.
- **Registry interfaces** (A-ids) use the RFC #180 registry from [`19_shared_logging_design.md`](19_shared_logging_design.md). Call `require` lazily at first use, and keep the current local code as the fallback when the provider is not installed.
- **Common helpers** (B-ids) live in `Patches/Common` on the KPM fork. Edit them in `patch_manager_mods/_shared_src/` and sync with `scripts/check_shared_headers.py --sync`. New code includes `"SafeRead.h"`, not `"../_shared/SafeRead.h"`.
- **Rollout:** shared pieces are consolidated on our fork first. Each one goes upstream inside the first mod PR that uses it, so order this mod's PR after the PR that introduces anything it reuses. Migrating this patch needs its owner's go.
- **Logging:** move to `KpLog.h` channels and `kplog.ini` (doc 19 Phase 6). Keep the tag text exactly as it is, because the parsers depend on it.
- **A1 `k2.guiinput` (consumer):** replace the window subclass (`wheelProc`, install :498, conditional unlink :764), the `gameWindow()` EnumWindows scan (:487) and the `cursorEngine`/`syncScreen` transform with `game_hwnd`, `cursor_engine` and `wheel_subscribe`. This removes the detach-only-if-on-top hazard shared with equip3d, effects and hotbar.
- **B2 GuiKit:** `bindCover` label creation (:568), `cursorEngine` (:463, identical in 3 patches) and `syncScreen` (:503).
- **B3 ini helper:** `trim` (:147), `parseFloats` (:152), `maybeReloadIni` (:239). It merges with the `kplog.ini` reloader.
- **B4 3D scene view:** `BindInventoryView` (:590-654) and its constexpr block (:31-60) duplicate equip3d's `BindEquipView`; both become one `scene3d` helper. `modelCount`, `getModel` and `dumpMem` go too.
- **B8 panel slot table:** `claimSlot`/`findSlot` (:282).
