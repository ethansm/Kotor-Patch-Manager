# Plan HW-EDIT v2: holster placement editor as its own patch on the in-game Options screen (doc 42)

Status: PIVOTED 2026-10-16. User decision: "instead of being in the equip screen, create the dedicated options screen so it's
independent from that patch. Users might not have an ultrawide monitor to run the ultrawide menus it relies on. Putting it in
the options is a more robust solution."

Supersedes the HE3/HE4/HE5 phases of plan HW-EDIT v1 (equip-screen editor; text in transcript acdd1f4e and in the handoff).
HE0, HE1, HE2 and HE6 carry over. Options study: `.claude/research/holster_edit/UX_OPTIONS.md` (O2a).

## 0. Fixed constraints
- Separate patch `holster-editor` (`patch_manager_mods/holster_editor_src/`):
  - `kind = "patch"`, `requires = ["holster"]`, `requires_interfaces = ["k2.holster@1"]`.
  - No dependency on equipment-3d-character, inventory-3d-viewport or the ultrawide GUI work.
- **Must work on the vanilla `optionsingame_p.gui` and on an ultrawide override of it.** All positions are measured at
  runtime from the panel's own control rects (by tag). No hard-coded pane coordinates.
- Holster stays `kind = "patch"`: KPM skips an unrequired provider (`DependencyValidator.cs:293`). It provides `k2.holster@1`
  from KPatchInit (ranged-ai precedent). equip3d is not touched by this plan, except the shared-code extraction in D1.
- Holster owns data, pose maths and holster.ini; the editor owns only UI.
- Installs:
  - A new patch or hook = `kpm_reinstall.py` with the game closed and the user's GO.
  - DLL-only changes = hot-swap after `pre_install_gate.sh` GATE PASS, a re-gate after `--prepare`, and the user's GO.
- No typed numbers: the game also reads the keyboard through DirectInput (r1), so keys cannot be hidden from it. Use sliders, ± buttons and drag-scrub.
- Live-only checks go to `patch_manager_mods/PLAY_TEST_QUEUE.md` rows and never block.

## 1. Carried over (done or unchanged)
- **HE0** preview probe: `PREVIEWNODE`/`CAMNODE`, kpatch 19a54e80 prepared + re-gated, `--apply` awaits GO; play row HW-C6.
  - Still useful: it proves the attach refresh 0x504D40 runs in a menu scene, and that menu-scene model and body names match the world.
- **HE1** landed `03aa85d`: `HolsterEdit.h`, `HolsterTune.h` (model/body/left tables), `HolsterIniWrite.h`,
  `_shared_src/HolsterApi.h`, `_shared_src/GearIconTex.inc`, `equipment_3d_character_src/Equip3dEdit.h` (moves in D1).
- **HE2** holster wiring, as in v1 §3 HE2, with one comment change: `HolsterApi.h:3-4` must say "the holster-editor patch" instead of equip3d.
  - provide k2.holster;
  - preview fast path first in HolsterPlace (registry ≤8, parent/node drop check, `g_nPreview` guard, editor=1 boot key);
  - `defaultBone` = boot bone (set `bootBoneIdx` in readConfig);
  - in-world full resolve cached per tune sequence;
  - per-site `loadTuneIfChanged`;
  - overlay + Save/Revert/ResetScope/SetScope;
  - `GetNodeWorld`;
  - `ch_edit` channel.
- **Camera axes** (r1): the preview camera copies the rig's `camerahook` world pose and looks along its local −Z. Screen right = node +X, screen up = node +Y.
- **Mouse design** from v1 §2 is kept:
  - left-drag rotate (trackball relative to the screen);
  - right-drag move in the screen plane;
  - Shift+wheel depth, wheel zoom;
  - middle-drag spin, middle-click reset;
  - X/Y/Z lock toggles (on screen only), Shift fine, Ctrl snap.
  - **Changed:** right-drag needs the RMB world-camera swallow at 0x7B3817 only if the world camera still reacts while Options is open (D2 check).
- **HE6** export defaults: unchanged.

## 2. Architecture (new)
1. **Entry.** Hook the CSWGuiInGameOptions ctor window before `StopLoadFromLayout` (call at 0x008A1704; site and register from D2-B).
   - Clone one button `BTN_HOLSTER` "Holster placement" from a vanilla `BTN_*` template.
   - Place it in the first free slot below the last vanilla button (measured).
   - Also clone, hidden: the editor labels and the 3D view.
2. **Editor mode (inside the Options panel).** Clicking `BTN_HOLSTER`:
   - hides every vanilla control (remember their visibility, restore on exit);
   - shows the viewport (left area) and the value pane (right area), using the space the vanilla button list + `LB_DESC` occupied (union of their rects);
   - "Back" leaves editor mode, and so do Esc and the Options EXIT button (intercepted; D2-B says where).
   - The world is paused because the in-game menu is open (D2-B confirms; play row checks).
3. **3D view.**
   - Preferred: a `3D_MODEL` control added to `optionsingame_p.gui` by a generator (copy from `upgrade_p.gui`, idempotent, sha-tracked, contiguous ids). It works on vanilla and ultrawide files and chains on an existing override instead of replacing it (D2-C).
   - Fallback if a .gui edit is unacceptable: D2-C's runtime option.
   - Scene: `sceneview::createView` → `initScene("gui3D_room")` → rig `charrec_light` → `cameraAttach("camerahook")`.
4. **Body.**
   - `Preview3d.h` (D1) builds a preview creature from a real party member's appearance (`+0x224` blob, BuildApp 0x76D200) and attaches the equipped weapons (item ids `+0x238/+0x23C`, model create 0x462320, Gob vt[0x50] "rhand"/"lhand").
   - Picker `◀ name ▶` over the party (D2-D gives the enumeration and names).
   - The weapon model name must equal the in-world `[Gob+0x84]+0x08` name (D2-D; HE0 check 3).
5. **Holster link.**
   - The editor `require`s `k2.holster@1` and registers its preview weapon Gobs (`RegisterPreview`).
   - Editing goes through `BeginEdit` / `Nudge*` / `SetValues` / `GetEdit` / `Save`.
   - Every Gob is unregistered before it is freed, on body rebuild, picker change, editor exit and panel destroy.
6. **Value pane** (re-laid out from `Equip3dEdit.h` `modalLayout` for the measured pane):
   - header (weapon / type / body, hand ◀R/L▶, source levels);
   - override warning;
   - scope `This weapon | This type | This body`;
   - lock X/Y/Z;
   - 6 rows `[name][−][track+thumb][value][+]` with ± auto-repeat and value scrub;
   - bone row;
   - `Save | Revert | Reset | Back`;
   - a legend line under the viewport.
   - Opaque fill `e3d_fill`, renamed to the editor's own resrefs (`hse_fill`, `hse_trk`, `hse_thm`) via `embtex::ensure`.
7. **Lifetime.** Options ctor → bind. Per-frame tick (D2-B site) → input + `GetEdit` refresh. Panel destroy (D2-B site) → `Revert` + `EndEdit` + unregister + destroy creature. A tick gap > 2 s means the menu closed: leave editor mode and keep the overlay (v1 §1.3 rule).

## 3. Phases
- **HE0** (pending GO): apply holster 19a54e80, then play HW-C6.
- **HE2** (Opus, holster hot-swap): as v1 + §1 note. Gates:
  - `run_tests.py holster`, `check_hook_bytes.py --mod holster`, `check_provider_manifests.py`;
  - `pre_install_gate.sh --mod holster_src --skip-default`;
  - `kpm_hotswap.py --mod holster --prepare --commit-kpm`, re-gate;
  - `kpm_status.py` = SWAP;
  - GO → `--apply`.
- **D2** (Haiku recon, running 10-16): d2b Options hooks / Esc / pause; d2c `.gui` + ultrawide + generator; d2d party picker + `Preview3d.h` API + model name.
- **D1** shared extraction (Sonnet worker, Opus review; equip3d hot-swap, no behaviour change):
  - `_shared_src/EditKit.h`: `Box`, Repeat, slider, dragMode, midClick, fmtValue, from `Equip3dEdit.h` + tests. Also drop the duplicate `Box` in `hud_host_src/HhStrip.h` if that is trivial; otherwise leave it.
  - `_shared_src/Preview3d.h`: Viewer struct + body/weapon/view/drag functions from `EquipCharPreview.cpp` (ranges from d2d).
  - equip3d switches to it.
  - Delete `Equip3dEdit.h`'s weaponRow/tipFor (unused, never wired). Move `modalLayout`/`legendText` to `holster_editor_src/HseLayout.h`.
  - Gates: `compat_check.py` + Common sync, `run_tests.py equip3d`, equip3d `pre_install_gate`.
  - Play row: equip preview unchanged (EQ4/8/9).
- **D3a** skeleton patch (reinstall, GO):
  - `holster_editor_src/` with manifest, hooks (ctor, tick, destroy), `exports.def`, KpLog patch `holster-editor`, `holster_editor.ini` (`enabled=1`, `debug=0`);
  - `scripts/add_options_holster_controls.py` + test;
  - `BTN_HOLSTER` toggles editor mode, showing an empty opaque pane + Back.
  - Play row: button visible on vanilla-layout Options; enter/exit restores the vanilla controls; Esc behaviour; world paused.
- **D3b** viewport + picker + holstered view (hot-swap): the chosen member shows with weapons on the back, using the saved values via RegisterPreview; spin and zoom. Play row.
- **D3c** value pane (hot-swap): rows, scope, hand, bone, Save/Revert/Reset; values live in the preview and not in the world before Save. Save writes `holster.ini` under the editor marker and applies in world. Play row.
- **D3d** drag editing + legend (hot-swap): trackball rotate, screen move, depth, locks, fine/snap. Play row.
- **HE6** after the playthrough (and HW7 for players).

Order: HE0 GO → HE2 ∥ D2 → D1 → D3a → D3b → D3c → D3d. Haiku-first for workers (one retry, then one Sonnet). Opus gates and commits with explicit paths.

## 4. Verification (end to end)
- Host tests:
  - `run_tests.py holster holster_edit holster_tune holster_iniwrite equip3d edit_kit preview3d holster_editor` PASS;
  - `test_add_options_holster_controls.py` PASS;
  - `check_provider_manifests.py` FAIL=0.
- Gates: GATE PASS for holster, equip3d and holster-editor. `kpm_status.py` shows all three LIVE.
- Live:
  - the button shows on the vanilla Options layout and on the ultrawide one;
  - the editor works with equip3d uninstalled;
  - with holster uninstalled, holster-editor is not installable (hard requires);
  - values survive a restart and match the world.
