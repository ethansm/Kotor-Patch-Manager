# Holstered weapons: mounts, weapon types, aggregate metadata

Snapshot 2026-10-21, holster 0.11.0-dev (repo `916c54b1`), holster-editor 0.6.2-dev. Compiled from four parallel Haiku recon reports, with the
slide clamp and the live `holster.ini` re-checked by hand. Companion to `42_holstered_weapons.md` (design) and `42_holstered_weapons_master_plan.md` (phases).
Paths below are relative to `patch_manager_mods/holster_src/` unless noted.

## 1. Mount types

A mount is a skeleton bone of the character's body model. The weapon is placed relative to that bone (rotation + offset); the engine hand attach is the
"drawn" state. There are exactly **5 selectable mount bones** (`HolsterTune.h:46,56-58`, `kBones`), in editor order:

| # | Editor label | Bone | Physical place (male body, height above feet) | Used by default |
|---|---|---|---|---|
| 1 | Upper back | `torsoUpr_g` | between the shoulder blades, z 1.244 m | none (set per type or model) |
| 2 | Back | `torso_g` | about pelvis height at the back, z 1.139 m | none |
| 3 | Pelvis | `pelvis_g` | centre of the waist, z 1.126 m | every category (via `bone_name`) |
| 4 | R hip | `rthigh_g` | right thigh / hip, x +0.104 m, z 0.982 m | none |
| 5 | L hip | `lthigh_g` | left thigh / hip | none |

Heights are the male body only; the female body is unverified. There is no "lower back" bone: **Back** (`torso_g`) is the nearest.

Not mounts: the attach node (weapon stays in the hand); any other `bone_name` value (rejected, the weapon stays in the hand).

### Mount resolution (what the DLL does today)

Per weapon and per hand, the first level that sets a bone wins (`HolsterTune.h:344-364`, `Holster.cpp:1454-1456`):

1. `place_bone_w_<model>` (exact model resref, variants need their own key)
2. `place_bone_<category>` (lightsaber, melee, qstaff, pistol, rifle, heavy)
3. `bone_name` (default `pelvis_g`)

The left hand mirrors the right-hand levels: `rthigh_g` <-> `lthigh_g` swap, pelvis and torso bones map to themselves, offset x and rotation y,z are negated,
and `left_extra_rot` (default -8 deg) is applied on top.

For the arm reach, any mount on `torsoUpr_g` or `torso_g` counts as a **back mount** (elbow up and over the shoulder; hinged path when the mount is at about
shoulder height or higher). Everything else is a hip mount (bowed path, elbow back and out).

### Designed but NOT wired: the four-mount model

`HolsterLogic.h` defines `MountHipR/HipL/BackR/BackL` (Zone x Side), `preferredZone`, `isLong` and `chooseMount` (two weapon sets fill hip R/L + back R/L;
lightsabers never leave the hip; spill order: preferred zone same side, other zone same side, preferred zone other side, other zone other side, else stay in
hand). It is tested (`holster_logic_test.cpp`) but has no caller in `Holster.cpp` (comment at `Holster.cpp:13`: "not used by the probe yet"). Back-R and back-L both
map to `torsoUpr_g`. `classify()` is always called with `weaponWield=0`, so the weaponwield-3 fallback for long melee never runs.

Decision open: wire `chooseMount` into the DLL (weapon sets, spill, saber-hip lock) or drop that design from `HolsterLogic.h`.

## 2. Weapon types

Six categories (`enum Category`, `HolsterLogic.h:14`), 32 model stems in the table (`HolsterLogic.h:59-71`); anything else is `CatUnknown` and uses the global
placement. Counts verified by rg.

| Category | Stems | Models | Long (back-preferred in the unwired logic) | Dual-wield eligible |
|---|---|---|---|---|
| lightsaber | 3 | `w_Lghtsbr`, `w_ShortSbr`, `w_Dblsbr` | none: locked to hip (even the double saber) | single + short sabers |
| melee | 14 | `w_Stunbaton`, `w_Lngswrd`, `w_Vbroswrd`, `w_Shortswrd`, `w_Vbroshort`, `w_waraxe`, `w_Dblswrd`, `w_VbrDblswd`, `w_gaffi`, `w_warblade`, `w_ForcePike`, creature `w_crslash`, `w_crpierce`, `w_crbite` | 5: `w_Dblswrd`, `w_VbrDblswd`, `w_gaffi`, `w_warblade`, `w_ForcePike` | Stunbaton, Lngswrd, Vbroswrd, Shortswrd, Vbroshort, waraxe |
| qstaff | 1 | `w_Qtrstaff` | yes | no (two-handed) |
| pistol | 6 | `w_BlstrPstl`, `w_HvyBlstr`, `w_HldOBlst`, `w_IonBlstr`, `w_DsrptPstl`, `w_SonicPstl` | no | all 6 |
| rifle | 6 | `w_Blstrrfl`, `w_BlstrCrbn`, `w_IonRfl`, `w_SonicRfl`, `w_DsrptRfl`, `w_BowCstr` (bowcaster) | yes | no |
| heavy | 2 | `w_RptnBlstr`, `w_HvRptBltr` (repeaters) | yes | no |

Totals: 32 stems, 14 long (designed back) / 18 hip. Two-handed (right hand only): 15. Creature weapons (`w_cr*`) have no weaponwield data; their dual flag is
unverified. `w_HvyBlstr` / `w_hvyblstr_001` is a **pistol** in the DLL, not a heavy (the back_heavy research used it by mistake). No shield or off-hand item class
exists in the 36-row weapon table. Variant models (`w_dblsbr_010`, `w_qtrstaff_009/010`, `w_shortsbr_010`, `w_lghtsbr_011`, `w_hvrptbltr_003`, `w_blstrpstl_015`)
need their own per-model keys.

## 3. Where each type is mounted

**Live configuration** (game-dir `holster.ini`, play-tested 10-16, rifle/heavy "still tuning"):

| Type | Bone | Rotation (deg) | Offset (m) | Notes |
|---|---|---|---|---|
| lightsaber (global) | `pelvis_g` | 180,0,0 | 0.20,0,0 | `bone_name` default + global `place_rot/off`; hip |
| pistol | `pelvis_g` | -100,0,0 | global 0.20,0,0 | hip |
| qstaff, unknown | `pelvis_g` | global | global | no key: falls to the global hip placement |
| melee (all 14 stems) | `torsoUpr_g` | -150,7,94 | 0.12,-0.10,0.22 | **upper back**, including one-hand swords and the stun baton |
| `w_Lngswrd_001` (per model) | `torsoUpr_g` | -150,7,94 | 0.12,-0.15,0.22 | model-level override of the melee key |
| rifle (6) | `torsoUpr_g` | -60,4,93 | 0.12,-0.11,0.20 | upper back |
| heavy (2) | `torsoUpr_g` | -90,-3,-90 | 0,-0.08,0.25 | upper back |
| per body | n/a | n/a | `P_Kreia1hBB` 0,0,0; `P_AttonBB` -0.003,-0.082,-0.018 | body-specific offset deltas |

**Shipped template** (`holster_src/holster.ini`): inert (`probe=0`, `bone_test=0`, `place=0`); no per-type keys are active. Commented seeds exist
(`holster.ini:57-79`) and disagree with the live set for rifle (`45,0,-90 / -0.20,-0.09,-0.10`) and heavy (`110,0,-25`).

**Research picks** (rest pose only, not game-verified for head clearance): double saber on `torsoUpr_g` rot `0,42,0`, offset `-0.106,-0.06,-0.10` (z -0.216 on the
male body); rifle `45,0,-90` / `-0.20,-0.13,-0.21` (flatter `30,0,-90` untested); heavy `110,0,-25` / `0.03,-0.17,-0.22` (z fix -0.12 untested); pistol on thigh
`-90,0,0` / `+-0.12,0,-0.03`.

## 4. Unique shapes

Measured from the model files (blade = model +z, barrel = model +y, single-axis pose solver `scripts/holster_pose_solve.py`).

Sizes: K2 R17 survey (`patch_manager_mods/inventory_3d_viewport_src/research/R17_fit_survey_2026-09-29.txt`), `.claude/research/holster_smooth/C2_shape_evidence.md`.

| Shape | Examples | Facts | Mount issue | Handling |
|---|---|---|---|---|
| Double-bladed saber | `w_dblsbr_001`, `_010` | hilt 0.544 m, origin at lower hilt end, grip centre +0.21 m; blades z -1.03..+1.44 (2.46 m total) | at identity on `torsoUpr_g` it pokes 0.48 m above the bone, over the neck; at the hip it is thigh length | hip by default (locked); back only by hand-set key |
| Single / short saber | `w_lghtsbr_001` (1.31-1.33 m), `w_shortsbr_001` (0.96-0.98 m) | hilt 0.35 / 0.24 m; origin = grip | identity hides in the hip | rot 180,0,0 + x offset 0.20 |
| Two-handed melee | `w_Dblswrd`, `w_VbrDblswd`, `w_ForcePike`, `w_warblade`, `w_gaffi` | unmeasured | long, back intended | shares the melee key (upper back) |
| One-hand melee | swords, axe, `w_Stunbaton` (0.39-0.43 m) | swords unmeasured | same key as two-handed, so one change moves both | exact per-model keys only |
| Quarterstaff | `w_Qtrstaff` | unmeasured long rod | needs the back | own `qstaff` key; no shipped seed |
| Rifles / bowcaster | `w_Blstrrfl` (0.061 x 0.745 x 0.255 for `_001`, muzzle y 0.62), `w_BlstrCrbn` (barrel -0.05..0.58) | barrel +y, grip -z | on the thigh the barrel passes the knee; head clearance on the back unverified | upper back seeds |
| Heavy repeaters | `w_RptnBlstr`, `w_HvRptBltr` | unmeasured | heavy, barrel up and back | upper back; reach path goes straight up and out |
| Pistols | six models | 0.081 x 0.405 x 0.220 (`w_blstrpstl_001`, long axis y), muzzle y 0.34 | thigh needs -90 about x and 0.2 m outward | pelvis, -100,0,0 |

Open: extents are unmeasured for the quarterstaff, long and vibro swords, pike, bowcaster, ion/sonic/disruptor rifles and repeaters. The planned 0.6 m "long"
threshold is not implemented.

## 5. Drawing and holstering: aggregate settings

Gates: the toggle, delay and reach features are inert unless `probe=1`, `bone_test=1` and `place_mode=drawgate`. Live has all three (the shipped template does not).

| Setting | Default | Range | Meaning |
|---|---|---|---|
| `toggle_key` | 0x77 (F8) | 0 = off | draw / stow toggle |
| `draw_hold_ms` | 8000 | 0 = until next press, else 1000..60000 | timed draw hold |
| `combat_exit_ms` / `flourish_exit_ms` | 1500 / 1500 | 0 = off, else 200..7800 | stance exit cut when no enemy is in range |
| `hostile_range` / `hostile_backoff_ms` | 30.0 m / 1000 | 5..60 / 250..5000 | what counts as hostile, re-check backoff |
| `draw_slide_ms` / `stow_slide_ms` | 350 / 350 | **0..570 / 0..1000** (clamped silently) | ease between mount and hand |
| `move_hold_ms` | 1500 | 0..10000 | no exit cut while walking, plus this long after |
| `party_combat_hold` | 1 | bool | any party member in combat keeps every party member drawn |
| `flourish_draw` / `delay_party` | 1 / 1 | bool | weapon stays in hand during the flourish stance / delay applies to the whole party |

Live values: `draw_slide_ms=570` (corrected 10-21: 900 was clamped to 570), `stow_slide_ms=900`.

### Arm reach (needs `reach_test=1`; live is 1, default 0)

| Setting | Default | Meaning |
|---|---|---|
| `reach_carry` | 1 | weapon rides the real hand during the slide |
| `reach_hinge` | 1 | back mounts at about shoulder height or higher use the staged hinged path (0.11.0-dev) |
| `reach_grab_pct` | 30 (0..60) | share of the slide spent reaching / grabbing (draw) and releasing (stow) |
| `reach_wrist_pct` / `reach_wrist_max_deg` | 100 / 90 | wrist turn from the weapon's mount orientation to the resting orientation |
| `reach_bulge_<cat>`, `reach_pole_<hip\|over>_<cat>` | per-category table | bow of the hand path and elbow direction |

Timing note: the reach is `reach_grab_pct` of the slide, so at `draw_slide_ms=570` it lasts about 171 ms; raise `reach_grab_pct` (up to 60) to see it longer.

Per-category defaults (`HolsterReach.h:127-134`, right-arm body frame x right, y forward, z up):

| Category | Hip pole | Over-shoulder pole | Bulge (up, forward, out) |
|---|---|---|---|
| lightsaber, melee | 0.60,-0.40,-0.40 | 0.50,0.00,0.85 | 0.20,0.40,0.10 |
| qstaff | 0.60,-0.40,-0.40 | 0.60,-0.20,0.70 | 0.40,0.20,0.20 |
| pistol | 0.50,-0.70,-0.20 | 0.50,0.00,0.80 | 0.20,0.40,0.10 |
| rifle, heavy, unknown | 0.60,-0.50,-0.30 | 0.70,-0.10,0.65 | 0.60,0.10,0.25 |

Hinge constants: stage 3 (grab) ends at path 0.30, stage 2 (pitch) at 0.80, stage 1 (settle) up to 1.0; pole outward share 0.25; raised-hand outward offset 0.10
forearm length; minimum outward share of the stage-3 elbow bend 0.35; elbow lift 1.0 up and 0.5 out; the hinge applies only for mounts no lower than 0.25 arm
lengths below the shoulder (`reachHingeFits`).

## 6. Counts and versions

| Item | Value |
|---|---|
| Model stems per category | lightsaber 3, melee 14, qstaff 1, pistol 6, rifle 6, heavy 2 (32) |
| Live `holster.ini` | 33 key lines; 17 `place_*` lines: 3 `place_off_<type>`, 4 `place_rot_<type>`, 3 `place_bone_<type>`, 3 per-model keys (1 model), 2 per-body offsets |
| holster | 0.11.0-dev, provides `k2.holster@1`, requires nothing |
| holster-editor | 0.6.2-dev, requires holster, preview-3d, mod-config |
| `k2.holster` API | version 1, 16 function slots (`RegisterPreview` .. `SetPreviewDraw`; the last slot is past the V1 base size) |
| Tests in `holster_src/` | 16 `*_test.cpp` (animstyle, delay, drawgate, edit, hand, iniwrite, logic, lograte, move, preview, quat, reach, slide, toggle, tune, common_contract) |

## 7. Findings to act on

1. The four-mount / weapon-set / saber-hip-lock logic in `HolsterLogic.h` is not used by the DLL (section 1).
2. Live melee key puts **every** melee weapon (including one-hand swords and the stun baton) on the upper back; only `w_Lngswrd_001` has its own override.
3. The quarterstaff has no key, so it sits on the hip with the saber placement.
4. Shipped `holster.ini` header says 0.4.0-dev (stale), and its seeds disagree with the live-tuned values.
5. `HolsterTune.h:11` says the attach node is used when no bone is set; the code uses `bone_name`.
6. `HolsterBonePatch.h` has no dedicated test.
