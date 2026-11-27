# Holstered Weapons

> **STATUS** Track: SHIPPABLE. Version 0.22.0 (matches manifest.toml).
> **What it does / explores:** Moves party weapons to hip and back mounts out of combat, with timed draw and stow slides, hand-reach arm IK, an F8 toggle, a shorter combat-exit and unarmed locomotion clips. Includes the Holstered Weapons settings page under Options -> Mod Configuration (3D placement editor, Draw / Holster toggle, Speed slider). Provides `k2.holster@1`, used by Equipment3dCharacter.
> **Requires:** Steam/Aspyr KOTOR II (target hash in manifest.toml) and KPM, plus [K2Preview3d](../K2Preview3d/) (`k2.preview3d@1`) and [K2ModConfig](../K2ModConfig/) (`k2.modconfig@1`, `k2.menuui@1`) for the settings page. A `[holster]` section with `level=info` in the game-dir `kplog.ini` is needed, or every holster log line is dropped.
> **Known limits:** Damage-reaction clip is not covered by `unarmed_anim`. Reach IK (`reach_test`) and qstaff placement still need in-game checks. The shipped `holster.ini` defaults show no placement until `probe=1` and `bone_test=1` are set. The settings page code (`HolsterEditor.cpp`) has no host unit tests for its game-linked parts.
> **Testing:** live-tested on Steam; several holster checks (reach, qstaff, hotkey, draw delay) are still pending in game. Treat them as unverified. The 0.22.0 merge of the settings page into this DLL is not yet checked in game.
> **Install:** Install through KPM after K2Preview3d and K2ModConfig. If you have the old separate Holster Editor patch (`holster-editor`) installed, uninstall it first: it is merged into this patch and the manifests conflict. Then set `probe=1` and `bone_test=1` in `holster.ini` to see placement.

## Features
- Weapons ride a hip or back mount out of combat; drawn in combat.
- Timed draw and stow slides per weapon type (`draw_slide_ms_<type>`, `stow_slide_ms_<type>`).
- Reach IK for the right and left arm (`reach_test=1`, `reach_carry=1`): hand chases the sliding weapon, with a hinged back reach and an elbow pole per mount.
- F8 toggle (`toggle_key`, default 0x77) draws or stows by a timed hold.
- Post-combat stance ends after about 1.5 s when no enemy is within 30 m.
- `unarmed_anim=1`: holstered party members walk and run with unarmed clips.
- `scene_hold=1`: a weapon a cutscene script equips during a conversation stays in hand until it ends.
- Settings page (Options -> Mod Configuration -> Holstered Weapons): 3D preview of the whole character; buttons for hand, bone, scope, stance and party member; Move / Depth / Tilt / Roll-Turn mode bar with six sliders (Slide, Depth, Height, Tilt, Roll, Turn); Draw / Holster toggle (preview weapons slide to the hand and back); Speed slider (1x to 8x) that writes `slow_holster_x` to `holster.ini`.

## Configuration (holster.ini)
- `probe=0`: master switch for the probe. Set `1` to enable placement and reach.
- `bone_test=0`: placement switch. Needs `probe=1` and `bone_test=1`.
- `place_mode=drawgate` (default): the native hand attach is kept; weapons go on the mount only while the owner is out of combat.
- `reach_test=0`: arm reach during slides. Needs `reach_carry=1`.
- `toggle_key=0x77`: F8. `0` turns the key off.
- Per-type keys: `place_bone_<type>`, `place_rot_<type>`, `place_off_<type>` with type = lightsaber, melee, qstaff, pistol, rifle, heavy.
- The file's comments document every key with its version tag.

## Configuration (holster_editor.ini, settings page)
- Read at every Options open. Keys, each documented inline: `enabled`, `view_yaw`, `view_zoom`, `margin_px`, `free_view`, `text_k`, `caption_fix`, and the `probe_*` keys.
- The page saves placements to `holster.ini`; holster polls the file, so the game follows live.

## Logging (kplog.ini section)
- Section `[holster]`, `level=info`. Lines go to `holster_log.txt` in the game dir.
- The settings page logs to the same file (`editor page loaded`, `page REGISTER ok`, channels `embed` and `view`, one SPEED line per Speed change). Before 0.22.0 it wrote `holster-editor_log.txt`.

## Hooks
- Eight hooks in `kotor2-steam-aspyr.hooks.toml` (addresses 0x7682B0, 0x7637E0, 0x5DF926, 0x863650, 0x86367C, 0x5047B0, 0x504EF5, 0x763B49 and 0x763E7B).
- Two optional code patches in `code_patches.toml` (`bone_test_rhand`, `bone_test_lhand`).
- The settings page adds no hooks: it registers a mod-config page from KPatchInit.

## Build
- From this folder run `python3 ../create-patch.py`.
- Host unit tests live in the author's development workspace and are not shipped here.

## Changelog
- See [CHANGELOG.md](CHANGELOG.md), which also holds the history of the merged Holster Editor patch.
