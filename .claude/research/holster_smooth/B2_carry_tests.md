# B2: carry tests and live log summary (holster 0.11.0-dev)

Suite: `python3 scripts/run_tests.py holster_reach` gives `PASS cpp:holster_reach_test` (1 passed, 0 failed, 2.0 s).

## Tested properties
- reachPlan draw (holster_reach_test.cpp:575-596): at the grab (t=1120) pathF ~ 0, carryW < 1e-4, restMix < 1e-4. pathF is non-increasing before the grab and non-decreasing from 1126. Carry midpoint (t=1260) gives pathF 0.5 with carryW 1.
- reachPlan stow (598-616): pathF falls during the carry, rises from 1282 onward, ends near rest. Mount pathF checked at 1281.
- reachWristDelta (649-665): t=0 with weight 1 and cap 180 maps cur onto the mount orientation (|dot| ~ 1). t=1 gives identity. Cap 40 against a 90 deg need gives <= 40.01 (upper bound only). Weight 0.5 gives 30 < angle < 60. Weight 0 and NaN give identity.
- reachClampRot (634-646): 150 deg clamped to 80 deg within 0.01. A 30 deg input under cap 80 is unchanged. The negated quaternion gives the same result.
- reachAnimMix (546-552): 0 up to 0.45, 1 from 0.95, monotone in between.
- reachCarryTarget (554-572): f=0 gives the mount exactly, f=1 gives the hand exactly. The bulge is checked at f=0.5 only. Heavy weapons bulge more than swords.
- reachHinge (668-786): f=0 gives the mount, f=1 gives the rest hand and pole. Stage joints kHingeS3 and kHingeS2 are continuous at +/-1e-4 (T within 5 mm, P within 5 mm). For 6 back mounts over f in [0, 0.30] the mount is reached with rr == 1 (no clamp). The elbow stays within 6 cm inboard of the shoulder. Worst elbow step over 600 samples is < 2 cm. Left and right mirror.
- reachWeightRamp (534-544) tests only ramps 0.1 and 0.2. The production wrist ramp is 0.04 (Holster.cpp:1484), which is not tested by value.

## Untested properties that could cause a visible snap
1. 90 deg production cap: g_reachWristMax defaults to 90 (Holster.cpp:91). The cap is tested only at 40 and 80, in isolation. No test checks the consequence. If mount-to-hand exceeds the cap, the hand misses mountQ at the grab. The weapon blends from out (exact mount) to carried over the first 10% of the carry phase (Holster.cpp:1500; HolsterReach.h:328). That is a visible twist of the excess angle.
2. Grab-boundary composition: the wrist write weight is reachW, a time ramp of 4% of the slide (Holster.cpp:1362, 1484). The weapon blend uses plan.carryW, a 10% ramp of the carry (Holster.cpp:1500). No test samples the combined hand T, wrist and carryW around t=1120.
3. Per-frame skip: the wrist write is skipped when wristOk is false, parW is 0, wrote != 3, or g_reachWristPct == 0 (Holster.cpp:1358-1364). The wrist then jumps to baseW. holster_reach_test covers HolsterReach.h only, so Holster.cpp is not tested.
4. Hinge vs bowed fallback (Holster.cpp:1321, 1329): if reachHinge returns false the bowed path with reachPole mixing takes over. No test compares the two paths at mid f.
5. Loose bounds: hand T is held to 5 mm per 2e-4 of f at stage joints (line 687). The elbow is held to 2 cm per 1/600 step (line 768). A 1.9 cm elbow jump passes. No sweep covers the hand T or the wrist angle. Monotone checks skip 1120-1126 and 1278-1281.

## Log evidence (`python3 scripts/kotor2_logs.py check holster_log.txt`; raw log not read)
- Build: loaded=yes 0.11.0-dev, which matches commit 916c54b1 (HW23).
- REACH: found=7 probe lines, 3 parsed step lines. reach < 1 = 0 (no reach clamp). err min/med/max = 0.000 (hand lands on target). persist1 = 0 (engine resampled each frame). wrote_both = 3 of 3 (arm and fore). refuse = none.
- Not visible: wr= (wrist degrees), hg= and f= are not printed. The regex at scripts/holster_log_check.py:85 skips them. wrote_both masks with &3 (line 441), so the wrist bit 4 is ignored. The log gives no evidence on wrist angle against the 90 deg cap, or on whether the wrist write happened.
- Coverage: 3 step samples is too few to judge large wrist angles.
- Other: SLIDE draw=15 stow=15, frames min 25 max 53, maxdt max 32 (as printed). PLACE quat1 unit-norm ok on 173 lines. PLACEREFUSE nottarget=60000 is a placement counter, not REACH.

## Open
- Extend `_reach_events` in scripts/holster_log_check.py to print wr=, hg=, f= and the wrist bit (wrote & 4) count. This is outside this recon's write scope.
