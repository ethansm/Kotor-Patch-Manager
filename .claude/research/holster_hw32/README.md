# HW32 offline draw sweep (W6b, doc 42)

Host-only research, nothing shipped. Source: `patch_manager_mods/holster_src/holster_sweep_test.cpp`. Asserts: `python3 scripts/run_tests.py holster_sweep` (5 checks).
Build: `g++ -std=c++17 -Wall -Wextra -Werror -I patch_manager_mods/holster_src -I patch_manager_mods/_shared_src -I tests/include patch_manager_mods/holster_src/holster_sweep_test.cpp -o "$(mktemp -d)/sweep"`
Table: `"$SWEEP" --table`. TSV: `"$SWEEP" --tsv .claude/research/holster_hw32/sweep.tsv` (469 lines = header + 468 rows; 39 families, 0 skipped, 0 NaN).
TSV columns: family mount hand via status; head and torso x tip, butt, shaft min and step (0..199); static_head, static_torso (metres).

## Pose model (runtime carry, Holster.cpp:1637-1662)
- Weapon sits on the mount while carryW is 0. Then `blendPoseLocked2(s, mount, hand, carryW, carryW > 0 ? oriF : 0, via)`. Arm solved by `reachHinge` at pathF.
- Draw 520 ms, 200 ticks. 9 shaft samples: tip = +len/2, butt = -len/2, shaft = all 9.
- Static = the mount pose itself (carryW 0).

## Proxies (metres; up +z, fwd +y, right +x)
| Item | Value | Source |
|---|---|---|
| Shoulder R / L | (0.176,-0.022,1.468) / (-0.164,-0.017,1.462) | MEASURED r3_blend_proxies.md:50 |
| Arm L1 / L2 | 0.298 / 0.332 | derived, r3:55 |
| Head sphere | (0,0,1.50), r 0.105 | GUESS (r3:61) |
| Torso capsule | (0,0,1.13)-(0,0,1.44), r 0.13 | GUESS (r3:62); back surface y=-0.14 MEASURED (r3:64) |
| Hand rest R / L, restPole | (+-0.25,0.35,1.10), (0,-0.5,-1) | GUESS |
| Back mount | (-0.106,-0.082,1.028), both hands | GUESS placement (r3:68) |
| Hip mount | (+-0.15,-0.05,1.08) | z MEASURED (r3:67); x, y GUESS |
| Thigh mount | (+-0.104,0,0.982) | x MEASURED (r3:59); y, z GUESS; not a zone (HolsterLogic.h:118) |
| Extract share | back 0.20, hip 0.10, thigh 0.10 | plan W6; thigh GUESS |

GUESS orientations: back long axis 45 deg between +z and +x (`--back-deg`); hip and thigh identity; hand long axis to +y; via long axis to +z (weapon outside the shoulder NOT modelled).

## Findings
1. Static back mount clips. Every back number depends on the GUESS 45 deg quat. At 45 deg, 76/78 back cases clip at rest; qtrstaff static torso -0.048. Static back clips of 78 by `--back-deg`: 0: 4, 15: 48, 30: 70, 45: 76, 60: 72, 75: 50, 90: 30. Re-measure the back quat before using any back number.
2. Motion matters at back only in two places: melee draw min -0.106 vs static -0.095, and the qtrstaff late-carry torso dip (tick 160) at -0.011 via off, +0.071 via on. Elsewhere draw min equals static min.
3. Hip: all pistols clear, draw and static (min +0.016). Heavy `w_hvrptbltr_` L clips at rest (static torso -0.020, identity GUESS).
4. Thigh: static 31/78 clip, draw 78/78 (mount x 0.104 inside r 0.13). Uninformative; not a decision input.
5. Via: changes the pose only where oriF > 0 (held 0 in the carry extract window). 206/234 pairs differ in draw minima. The 28 identical pairs are all back, and their minima fall in oriF = 0 steps (`w_fraggren_` min at tick 75, oriF 0; via changes its pose at 111 steps). Not a no-op. Head: this blend adds no head clips (1/78 back both states). The 34/78 head-clip figure came from a no-hold variant (fRot = oriF during reach) and does not apply.

## Per-category table (`--table`, metres; draw = all draw columns, via off/on; static = mount pose)
```
category   back_off back_on back_static hip_off hip_on hip_static thigh_off thigh_on thigh_static
lightsaber -0.0460 -0.0460 -0.0460 +0.0281 +0.0281 +0.0281 -0.0260 -0.0260 -0.0260
melee      -0.1060 -0.1006 -0.0949 +0.0190 +0.0190 +0.0190 -0.0359 -0.0345 -0.0296
qstaff     -0.0480 -0.0480 -0.0480 +0.0281 +0.0281 +0.0281 -0.0260 -0.0260 -0.0260
pistol     -0.0250 -0.0250 -0.0250 +0.0156 +0.0156 +0.0156 -0.0582 -0.0570 +0.0135
rifle      -0.0150 -0.0150 -0.0150 +0.0187 +0.0187 +0.0187 -0.0556 -0.0540 +0.0063
heavy      -0.0145 -0.0145 -0.0145 -0.0201 -0.0201 -0.0198 -0.0835 -0.0862 -0.0481
unknown    -0.0118 -0.0118 -0.0029 +0.0281 +0.0281 +0.0281 -0.0214 -0.0212 +0.0129
```
"unknown" = grenades and detonators (not in HolsterLogic.h:57-70). Melee is back when two-handed; the table does not split it.

## Asserts (test mode)
- qtrstaff/back/R via off, draw or static min < 0: holds (-0.048). Clips under the GUESS quat (finding 1), reworded.
- Via changes the draw for that case: holds. Sign not asserted (melee back on is +5 mm; qtrstaff identical).
- Pistols on hip (CatPistol, 24 cases), draw and static min > 0: holds. skipped == 0: holds.

## Recommended defaults (W7 decisions from review)
- `reach_via_<cat>` = 0 for every category. Play-test row: rifle and qstaff via on. Their back worst is the static pose, which via cannot change.
- `reach_extract_pct_back` = 0.20 and `reach_extract_pct_hip` = 0.10 (plan W6, unchanged). The sweep cannot discriminate shares: 29 of 39 families have exitSign 0, and only the 10 melee families (all exitSign -1) move.
- Apply the exit offset only when exitSign != 0. For exitSign 0 (guns, sabers, staffs) skip it; no mount-out direction until the shape survey gives signs.

## Limits
Draw only (no stow). Extract offset applied after the arm solve (W7 applies it before). Per-category outT/outPole not modelled. Outside-shoulder via placement not modelled.
