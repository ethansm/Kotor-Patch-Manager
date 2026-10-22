# H2: offline sweep measurement, HW50 heavy carry

Units: f = ReachPlan.pathF (1 = hand at rest, 0 = hand at mount). Tip = sample k=8, butt = k=0 (sweep naming). Angle = weapon long axis vs back-mount axis, deg. Back mount, right hand, GUESS 45 deg mount quat. Heavy = w_rptnblstr_001 (w_hvrptbltr_001 gives the same pattern); pistol = w_blstrpstl_001; rifle = w_blstrcrbn_001. No repo edits.

## Commands run
1. `python3 scripts/run_tests.py holster_sweep` -> PASS (binary: /tmp/claude-1000/kotor-tests/host/holster_src__holster_sweep_test).
2. `<bin> --table` and `<bin> --tsv <scratch>/default.tsv --table` (937 rows; each row has via on and off).
3. Scratch copy `<scratch>/pose_dump.cpp` = holster_sweep_test.cpp plus a per-step pose dump (env POSE_DIR, POSE_STEM) and env RUNTIME_CAT=1 (runtime per-category timing, grab, via out, hinge pole). Built with the runner's g++ flags. Default mode output is byte-identical to the tool's TSV (cmp).
4. Dumps: `<scratch>/dump_default/` (tool timing) and `<scratch>/dump_rt/` (RUNTIME_CAT); analysis `analyze.py`, `tables.py` (stdlib). Scratch root: /tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/H2

## Table A: draw, runtime timing (heavy via on, pistol off, rifle on, as shipped)
Cells: tip xyz (m) / butt z (m) / angle (deg).

| f | heavy | pistol | rifle |
|---|---|---|---|
| 0.0 | +0.25 -0.12 +1.43 / 0.92 / 0 | +0.11 -0.11 +1.29 / 1.00 / 0 | +0.28 -0.12 +1.46 / 1.02 / 0 |
| 0.2 | +0.35 +0.11 +2.36 / 1.69 / 24 | +0.31 +0.19 +2.11 / 1.83 / 14 | +0.46 +0.11 +2.39 / 1.83 / 19 |
| 0.4 | +0.35 +0.49 +2.24 / 1.56 / 42 | +0.39 +0.52 +1.95 / 1.72 / 37 | +0.39 +0.35 +2.30 / 1.68 / 44 |
| 0.6 | +0.27 +0.93 +1.58 / 1.41 / 77 | +0.30 +0.70 +1.56 / 1.42 / 63 | +0.38 +0.90 +1.73 / 1.42 / 64 |
| 0.8 | +0.18 +0.84 +1.22 / 1.22 / 90 | +0.21 +0.64 +1.25 / 1.21 / 82 | +0.18 +0.89 +1.23 / 1.22 / 89 |
| 1.0 | +0.25 +0.88 +1.15 / 1.15 / 90 | +0.25 +0.69 +1.15 / 1.15 / 90 | +0.25 +0.93 +1.15 / 1.15 / 90 |

## Table B: stow, runtime timing (same via states)

| f | heavy | pistol | rifle |
|---|---|---|---|
| 0.0 | +0.25 -0.12 +1.43 / 0.92 / 0 | +0.11 -0.11 +1.29 / 1.00 / 0 | +0.28 -0.12 +1.46 / 1.02 / 0 |
| 0.2 | +0.08 -0.03 +1.87 / 1.16 / 34 | +0.14 +0.01 +1.43 / 1.15 / 15 | +0.16 -0.05 +1.83 / 1.24 / 26 |
| 0.4 | +0.28 +0.67 +1.92 / 1.51 / 59 | +0.21 +0.29 +1.55 / 1.32 / 39 | +0.28 +0.44 +2.02 / 1.47 / 44 |
| 0.6 | +0.19 +0.95 +1.45 / 1.45 / 90 | +0.20 +0.56 +1.44 / 1.32 / 65 | +0.24 +0.96 +1.51 / 1.43 / 83 |
| 0.8 | +0.18 +0.84 +1.22 / 1.22 / 90 | +0.18 +0.61 +1.23 / 1.20 / 84 | +0.18 +0.89 +1.22 / 1.22 / 90 |
| 1.0 | +0.25 +0.88 +1.15 / 1.15 / 90 | +0.25 +0.69 +1.14 / 1.14 / 90 | +0.25 +0.93 +1.15 / 1.15 / 90 |

## Table C: peaks, runtime timing (both via states)
Rates use the uniform sample spacing (lastMs/199), not the integer-ms column.

| case | via | dir | tip z max (m) | peak deg/step (at f) | peak deg/s (at ms) | peak deg/f (f range) | tip path (m) |
|---|---|---|---|---|---|---|---|
| heavy | off | draw | 2.23 | 1.78 (0.33) | 623 (377) | 173 (0.33-0.34) | 2.57 |
| heavy | on | draw | 2.37 | 3.41 (0.42) | 1192 (403) | 331 (0.41-0.42) | 2.88 |
| heavy | off | stow | 1.76 | 2.37 (0.27) | 674 (259) | 213 (0.25-0.26) | 1.75 |
| heavy | on | stow | 2.02 | 4.54 (0.33) | 1294 (238) | 406 (0.32-0.33) | 2.28 |
| pistol | off | draw | 2.11 | 1.36 (0.48) | 907 (199) | 134 (0.46) | 2.33 |
| pistol | on | draw | 2.17 | 2.61 (0.57) | 1738 (217) | 257 (0.57) | 2.37 |
| pistol | off | stow | 1.55 | 1.19 (0.45) | 679 (124) | 135 (0.44-0.45) | 1.18 |
| pistol | on | stow | 1.68 | 2.27 (0.55) | 1297 (101) | 259 (0.56-0.57) | 1.41 |
| rifle | off | draw | 2.28 | 1.78 (0.37) | 683 (354) | 159 (0.38-0.39) | 2.64 |
| rifle | on | draw | 2.40 | 3.41 (0.47) | 1306 (380) | 301 (0.47-0.48) | 2.93 |
| rifle | off | stow | 1.75 | 1.78 (0.32) | 592 (207) | 182 (0.30-0.31) | 1.71 |
| rifle | on | stow | 2.03 | 3.41 (0.40) | 1133 (183) | 348 (0.39-0.40) | 2.25 |

Mean deg/s per 0.1 f bin, heavy draw: via on 212 / 468 / 530 / 931 / 1119 / 831 / 451 / 51 (f 0.0 to 0.7); via off 199 / 460 / 549 / 596 / 533 / 385 / 218 / 25.

## Observations
1. Mid-path spin is the big motion. With via on, heavy turns at 931-1119 deg/s over f 0.3-0.4, about 2x its via-off rate (530-600), and finishes 90 deg by f 0.8. Heavy is 9% slower per second than rifle (1192 vs 1306) only because its draw is 570 ms; pistol peaks at 1738. Heavy is not the outlier per second; its via arc is.
2. Height and length: via on lifts the tip to 2.2-2.4 m at f 0.2-0.4 (draw), about 0.75 m above the GUESS head top (1.61 m). Tip path is 2.88 m for heavy draw versus 2.37 m for pistol (both via on). The butt stays at 1.6-1.7 m, so the tip arcs over the head. This tall, long arc is the likely "swing" read.
3. The shipped tool cannot show a heavy-specific feel. In default timing, heavy, pistol and rifle have identical angle at each f (draw f 0.2 = 29 deg, stow f 0.2 = 39 deg). Only tip/butt offsets differ, from length (0.72 vs 0.41 vs 0.63 m). The heavy-only values (570/700 ms, grab 0.45, pole 0.45) move the angle by about 5 deg at f 0.2 only. Carry onset is not a lag: about 210 deg/s already at f 0-0.1.

## What the sweep does not model (vs runtime)
- Per-category timing: sweep uses 520/600 ms for every category (holster_sweep_test.cpp:33-34, 556, 562). Runtime is heavy 570/700, pistol 300/350 (HolsterTune.h:711-722). Only the RUNTIME_CAT scratch variant has these.
- Grab: fixed 0.30 (:574). Runtime heavy and rifle 0.45.
- Hinge pole: heavy outPole 0.45 at runtime, 0.25 in the sweep (:581). Via out: rifle 0.20 (holster.ini:193), heavy 0.12, sweep 0.12 for all (:584).
- Via: no per-category switch. Both states are always computed. Shipped reach_via_melee/rifle/heavy=1 and pistol off (holster.ini:186-188) are not read.
- Stow landing and rotation: runtime reach_stow_land_f 0.15 and reach_stow_rot_f 0.6 (Holster.cpp:120, 123). Sweep stow uses the time window kReachResidMs = 240 ms (holster_sweep_test.cpp:497).
- slow_holster_x: not a sweep option; shipped value is 1 (no effect).
- Pose: the TSV has clearance only. Tip, butt and angle came from the scratch dump.
- Clearance is negative for all categories under the GUESS mount (heavy stow back -0.12, via on). Not used for feel here.

## Open
Which weapon is "heavy blaster"? The sweep's heavy category is w_RptnBlstr and w_HvRptBltr. w_HvyBlstr (Heavy Blaster) is mapped to PISTOL (HolsterLogic.h:66), so it is not covered by these numbers.
