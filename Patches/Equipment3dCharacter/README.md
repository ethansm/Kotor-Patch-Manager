# Equipment 3D Character

> **STATUS** Track: SHIPPABLE. Version 0.3.0-probe (matches manifest.toml).
> **What it does / explores:** Shows a live 3D render of the party member with the currently equipped gear on the Equipment screen, with flourish idles, Animate/Spin/Emotes rows, and Hide/Show + Holster/Draw rows (weapons drawn by default).
> **Requires:** `gui-input` provider (`k2.guiinput@1`, from `gui_input_src`). Optional `k2.holster@1` (holster mod) for the Holster/Draw row; absent-safe. Steam/Aspyr KOTOR II (see `manifest.toml` `supported_versions`), Kotor Patch Manager.
> **Known limits:** Hide/Show and Holster/Draw are unverified in game (see Known issues). Glue code (INI reload throttle, long-line reject, CF_Alloc) is live-only. **Play-test status:** EQ10-P1 pending in `patch_manager_mods/PLAY_TEST_QUEUE.md`.
> **Install:** install `equipment-3d-character` through KPM (it pulls in `gui-input`). Optional: copy an `equip3d.ini` into the game folder (see Configuration). Do not install for normal play until EQ10-P1 passes.

## Features
- Live 3D party-member render on the Equipment screen, updated as gear changes.
- Idle flourishes on a timer, and optional flourish on equip.
- Toolbar rows (labels as shown in game):
  1. Animate (idle), Spin (click for play/pause and Reset), Emotes (gesture or attack menu).
  2. Hide weapons / Show weapons: removes or restores the weapons in the preview.
  3. Holster / Draw (only while weapons are shown): puts weapons on hip or back, or back in hand. Needs `k2.holster`.
- Drag to pan vertically; mouse wheel zoom.

## Configuration
Settings file: `equip3d.ini` in the game folder. Missing or garbage file means defaults; every key and rejected line is logged. Keys live in `Equip3dLogic.h` (parser, roughly lines 80-120). Main keys:
- `rig`: stored and echoed in the "INI loaded" log line.
- `screen_w`, `screen_h`: screen size override (both optional; the mask follows what is set).
- `weapons`: `1` = preview opens with weapons drawn, `0` = hidden.
- `keep_obj10`: DEPRECATED and ignored. Setting it to `1` only logs a warning; use `helmet=1`.
- `pan_min`, `pan_max`, `drag_lock_px`, `poll_ms`, `hit_flip_y`.
- `equip_anim`: animation played on equip (default `itemequip`).
- `flourish`, `flourish_min_s`, `flourish_max_s`, `flourish_first_s`, `flourish_quiet_s`, `flourish_on_equip`.
- `flourish_list`: comma list of idle flourish animations (default list is built in).
- `flourish_stance_list`: flourish list used while a weapon stance is on (empty = same as `flourish_list`).

## Logging
- Core log: `equipment-3d-character_log.txt` in the game folder (KpLog). Fallback when the core file is unavailable: `equip3d_log.txt` (INFO level).
- Create an empty `equip3d_debug.txt` in the game folder to enable per-tick detail.

## Hooks
Address hooks are in `kotor2-steam-aspyr.hooks.toml` (target `6A522E71...`). Main entry points:
- `0x008AA16A` BindEquipView (equip panel ctor, after the layout load).
- `0x008ABA50` EquipTick (equip panel Update, vtable slot 0x34).
- `0x008AA6F0` panel scalar deleting dtor (frees the preview creature).

## Build and test
```
python3 scripts/run_tests.py equipment_3d   # common contract test
python3 scripts/run_tests.py equip3d        # logic tests
```
The `common_contract_test` include path is repo-relative (`../../research/companion_levelup/tools/parity/stub`) and resolves inside this repo. `${KPM_COMMON_DIR}` defaults to `<KPM_DIR>/Patches/Common`.

## Known issues
- EQ10-P1 (in-game check of Hide/Show, Holster/Draw, reopen, companion switch) is still pending.
- If Holster does not move the weapons, the holster H7 hook may not fire for this mod's own attach. Follow-up: attach through `k2.preview3d` or add a holster API.
- Untested logic arms: screen_w-only and screen_h-only masks, idle align boundaries, pan zoom fallback, drag-axis tie, poll/lock clamps, `keep_obj10=0`.

## What was learned
Research notes and Ghidra disassembly logs are in `research/` (paths scrubbed to `~/`). The `FindImm.java` and `FindStrRefs.java` files there are Ghidra scratch scripts.

## Changelog
No separate changelog file. History is in `git log -- patch_manager_mods/equipment_3d_character_src`.
