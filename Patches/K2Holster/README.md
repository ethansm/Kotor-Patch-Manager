# Holstered Weapons

> **STATUS** Track: SHIPPABLE. Version 0.21.7 (matches manifest.toml).
> **What it does / explores:** Moves party weapons to hip and back mounts out of combat, with timed draw and stow slides, hand-reach arm IK, an F8 toggle, a shorter combat-exit and unarmed locomotion clips. Provides `k2.holster@1`, used by the holster editor and equipment_3d_character.
> **Requires:** Steam/Aspyr KOTOR II (target hash in manifest.toml) and KPM. No other patch is required (`requires = []`). A `[holster]` section with `level=info` in the game-dir `kplog.ini` is needed, or every holster log line is dropped.
> **Known limits:** Damage-reaction clip is not covered by `unarmed_anim`. Reach IK (`reach_test`) and qstaff placement still need in-game checks. The shipped `holster.ini` defaults show no placement until `probe=1` and `bone_test=1` are set.
> **Testing:** live-tested on Steam; several holster checks (reach, qstaff, hotkey, draw delay) are still pending in game. Treat them as unverified.
> **Install:** Install through KPM. Then set `probe=1` and `bone_test=1` in `holster.ini` to see placement.

## Features
- Weapons ride a hip or back mount out of combat; drawn in combat.
- Timed draw and stow slides per weapon type (`draw_slide_ms_<type>`, `stow_slide_ms_<type>`).
- Reach IK for the right and left arm (`reach_test=1`, `reach_carry=1`): hand chases the sliding weapon, with a hinged back reach and an elbow pole per mount.
- F8 toggle (`toggle_key`, default 0x77) draws or stows by a timed hold.
- Post-combat stance ends after about 1.5 s when no enemy is within 30 m.
- `unarmed_anim=1`: holstered party members walk and run with unarmed clips.
- `scene_hold=1`: a weapon a cutscene script equips during a conversation stays in hand until it ends.

## Configuration (holster.ini)
- `probe=0`: master switch for the probe. Set `1` to enable placement and reach.
- `bone_test=0`: placement switch. Needs `probe=1` and `bone_test=1`.
- `place_mode=drawgate` (default): the native hand attach is kept; weapons go on the mount only while the owner is out of combat.
- `reach_test=0`: arm reach during slides. Needs `reach_carry=1`.
- `toggle_key=0x77`: F8. `0` turns the key off.
- Per-type keys: `place_bone_<type>`, `place_rot_<type>`, `place_off_<type>` with type = lightsaber, melee, qstaff, pistol, rifle, heavy.
- The file's comments document every key with its version tag.

## Logging (kplog.ini section)
- Section `[holster]`, `level=info`. Lines go to `holster_log.txt` in the game dir.

## Hooks
- Eight hooks in `kotor2-steam-aspyr.hooks.toml` (addresses 0x7682B0, 0x7637E0, 0x5DF926, 0x863650, 0x86367C, 0x5047B0, 0x504EF5, 0x763B49 and 0x763E7B).
- Two optional code patches in `code_patches.toml` (`bone_test_rhand`, `bone_test_lhand`).

## Build
- From this folder run `python3 ../create-patch.py`.
- Host unit tests live in the author's development workspace and are not shipped here.

## Version history
- The full version history is in the manifest `description`. It is to be split into CHANGELOG.md before the upstream PR.
