# H8: heavy-weapon ini-only variants (HW50 recon). Read-only; live game ini not read; shipped reference patch_manager_mods/holster_src/holster.ini.
## 1. Live vs boot (verified in code)
- LIVE (500 ms poll, only when the running file has probe=1): place, place_rot, place_off, place_bone_<cat>, place_*_body_*, place_*_w_*, left_extra_rot*, slow_holster_x. Anchors: Holster.cpp:926-945 (maybeReload, gated by g_probe), Holster.cpp:272-316 (parseTuneText), HolsterTune.h:215-220 (isTuneKey).
- BOOT ONLY (readConfig, Holster.cpp:358+): draw_slide_ms*, stow_slide_ms*, reach_grab_pct*, reach_via_*, reach_via_out_m*, reach_hinge_out_*, reach_bulge_*, reach_pole_*, reach_resid_ms, reach_hinge, reach_wrist_*, reach_twist_deg, reach_stow_*, dual_stagger_ms. Doc claim "reach_via read at load only": CONFIRMED.
- Shipped file has probe=0, bone_test=0, reach_test=0 (ini:4, :26, :143). Slides and reach keys need probe=1, bone_test=1, place_mode=drawgate, reach_test=1, reach_carry=1 running. Live file state unverified.

## 2. Heavy keys (range from clamp site; built-in default)
| key | range | default | live |
|---|---|---|---|
| slow_holster_x (global, ALL categories) | 1..8 | 1 | yes |
| draw_slide_ms_heavy | 0..570 (HolsterTune.h:34) | 570 | no |
| stow_slide_ms_heavy | 0..1000 (ini:99) | 700 | no |
| reach_grab_pct_heavy | 0..60 | 45 | no |
| reach_via_heavy | 0 or 1 | code 0, shipped 1 (ini:188) | no |
| reach_via_out_m_heavy | 0..0.30 (HolsterReach.h:664) | unset, uses global 0.12 | no |
| reach_hinge_out_heavy = outT,outPole | outT 0..1, outPole 0..2 | 0.10,0.45 (HolsterReach.h:259-260) | no |
| reach_bulge_heavy = up,fwd,out | each -2..2 | 0.60,0.10,0.25 | no |

- Draw cannot exceed 570 ms by ini. A longer heavy draw needs slow_holster_x, which also slows every category.
- No key sets an ease curve (slide progress is linear, HolsterSlide.h frac). Motion shape is in code.
- When the hinge runs, it replaces the bulge path (Holster.cpp:1737), so reach_bulge_heavy is inert on that topology. Hinge needs mount height dzArm >= -0.25 (HolsterReach.h:280).

## 3. Variants (flat lines, no [section] headers; edit an existing line, never duplicate a key)
V1 Slow heavy (hypothesis: longer slide). Step 1 is live, step 2 needs a restart:
```
slow_holster_x=1.5
stow_slide_ms_heavy=1000
```
- Heavy draw 855 ms, stow 1500 ms. Every category is 1.5x too.
- See: the weapon reaches the hand later and lands without a snap. Watch that it does not hang in the air after the arm finishes (engine wield/unwield clips stay real time, ini:114).
- Check: `holster_log_check.py <log> --row SM-L17` (PASS needs a slowed slide and no SNAP at slowx>1) and `--row SM-L15`.

V2 Via off (hypothesis: the long-axis-up roll reads as a swing). One key, by design:
```
reach_via_heavy=0
```
- See: no roll through "long axis up"; the weapon takes the bowed arc.
- Check: SM-L15 c1 PASS, c2 NO-DATA, overall NO-DATA (exit 3). Not a FAIL. Eye check on head and elbow.

V3 Smaller outward via (hypothesis: the 0.12 m outward push reads as a swing):
```
reach_via_out_m_heavy=0.05
reach_bulge_heavy=0.60,0.10,0.10
```
- The bulge line only acts when topo is over or the hinge fails.
- See: the weapon stays nearer the shoulder and back through the carry.
- Check: SM-L15 c2 PASS (vo peak 0.05 > 0.005). Eye check on the head: highest clip risk of the four. If the head is touched, revert V3 first.

V4 Tighter hinge (hypothesis: outPole 0.45 widens the arc). One key, two values:
```
reach_hinge_out_heavy=0.05,0.25
```
- outT 0.10 -> 0.05, outPole 0.45 -> 0.25. Acts only when REACH topo=hinge.
- See: hand rises nearer the shoulder; elbow stays nearer the body.
- Check: SM-L15 cannot judge this (its pb clause is pole bias, HW29b). Eye check on head and elbow. Confirm topo=hinge in the log first, or V4 is a no-op.

## 4. Corrections to doc 42c HW50 text
- Hypothesis (c) says "outT 0.45". Wrong: 0.45 is outPole. outT is 0.10 (Holster.cpp:1523).
- The head-clip check is not SM-L15. scripts/holster_log_check.py:876-914 checks vo= at reach/release and mid-carry, and pb=. Head clip is an eye check only.
- The offline sweep does not model per-category via or hinge (.claude/research/holster_hw32/README.md:106; doc 42c:188 R5). Do not use it to clear V2-V4.

## 5. Run order and cost
- Restarts: V1 step 1 needs none. V2, V3, V4 each need one restart (boot keys). Run BASE (shipped values) first.
- Log: log_verbose=1 (play row SM-L15 setup). Read verdicts with holster_log_check.py only, never raw log greps.
- Back up the live holster.ini before each edit (it holds your tuning).

## 6. Open
- Heavy on torsoUpr_g: topo=hinge or topo=over? Unmeasured (doc 42c:197). Read the REACH topo= field in the first log. It decides whether V3's bulge line and V4 do anything.
