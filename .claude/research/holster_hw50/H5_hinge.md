# H5: hinge arm path, heavy back mount (HW50)

Read-only recon. Numbers from a scratch Python port of reachHinge (not committed): /tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2fa8c16a-2e61-4ba5-9d13-c7405753241a/scratchpad/hinge_sim.py

## Verdicts
- Hypothesis (c) as worded is REFUTED: heavy `outT` is 0.10, not 0.45. The 0.45 is `outPole` (elbow flare). Anchors: Holster.cpp:1522 (outT = kHingeOutT for all), :1523 (heavy outPole 0.45), holster.ini:206-208.
- SM-L15 is not a clip check. `_sm_l15` reads only `vo=` and `pb=` (scripts/holster_log_check.py:876-916). Head clip is only the play clause (PLAY_TEST_QUEUE.md row SM-L15) and the offline sweep (GUESS head).
- Only topology HingeBack calls reachHinge. Heavy's live topology is not in the repo; check it first.

## 1. Topology gate (HolsterReach.h:278-298; mount height dz = mount minus shoulder, arm = L1+L2 = 0.63 m)
- Hip: dz < -0.283 m (dzArm < kOverDz -0.45)
- OverBack (bowed line, u=1): -0.283 <= dz < -0.158 m
- HingeBack: dz >= -0.158 m (kHingeMinDz -0.25 arm)
- Thigh: dz < -0.416 m (kThighDz -0.66)
- Heavy's real back-mount height is not measured in the repo. The HW32 sweep GUESS (z 1.028) is dz -0.44, outside the hinge band, yet the sweep calls reachHinge for every mount (holster_sweep_test.cpp:581; README "topology").

## 2. HingeBack hand path, heavy defaults (outT 0.10, outPole 0.45)
Stages from reachHinge (HolsterReach.h:316-369). Mount assumed at dz -0.10 (in band; not measured).

| stage | f | hand motion | hand path | peak speed* |
|---|---|---|---|---|
| 3 grab | 0-0.30 | forearm closes from vertical over shoulder onto mount; elbow parked at S + L1 fwd | 0.74 m | 8.1 m/s |
| 2 pitch | 0.30-0.80 | whole L-arm pitches 90 deg about shoulder, elbow locked at 90; hand on 0.45 m radius | 0.70 m | 3.3 m/s |
| 1 settle | 0.80-1.00 | to animation rest hand | 0.11 m | 1.3 m/s |

*Assumes f runs at reachCarryF's linear rate over a 570 ms draw (about 1.9 f/s). The ratio (grab about 2.4x pitch) holds regardless of the time mapping.

- Arc width: hand x runs from -0.28 (mount, 0.28 m medial to the shoulder) to +0.03 at the top. Lateral travel comes from the mount position. outT adds only outT*L2 = 0.033 m at f=0.30, fading to 0 at f=0.80 (about 10% of travel).
- Top of pitch: hand 0.30 m forward and 0.33 m above the shoulder.
- Elbow: pole = fwd + outPole*out, plus x*(up + 0.5 out), with an outward floor kHingeMinOut 0.35. Elbow outward max 0.14 m at outPole 0.45, 0.11 m at 0.25. The hand path does not depend on the pole.

## 3. What keeps the weapon clear of the head
- No explicit head term in reachHinge. Clearance is emergent from: (a) the stage-3 pole keeps the elbow forward and out, so the upper arm stays off the head; (b) the top of the pitch is 0.35 m from the GUESS head sphere; (c) the closest hand approach is at the grab, f about 0.09, hand near back-of-neck height.
- Sim: closest approach is about 0 (-0.002 to +0.009 m) for every in-band mount height (dz -0.158 to 0.0). outT=0 moves it +0.007 m.
- The head proxy is a GUESS (holster_sweep_test.cpp:51, center z 1.50 is only 0.03 m above the shoulder joint). The grab result is therefore not evidence for or against clip.
- Hand-only numbers ignore the weapon's tip and butt, so the weapon's clip margin is set by orientation (not this angle) and the play check.

## 4. Knobs for the arm path (ini unless noted)
| knob | heavy default | change | effect (sim) | clip risk | SM-L15 script effect |
|---|---|---|---|---|---|
| draw_slide_ms_heavy / stow_slide_ms_heavy | 570 / 700 | longer, e.g. 900 / 1000 | speed x 0.63; geometry unchanged | none expected | none (vo, pb not time-scaled) |
| reach_hinge_out_heavy outT | 0.10 | 0.0 | hand 0.033 m narrower at top; grab clearance +0.007 m | low (top margin 0.35 m, GUESS) | none |
| reach_hinge_out_heavy outPole | 0.45 | 0.25 (rifle value) | elbow flare 0.14 to 0.11 m; hand unchanged | low (elbow-head unchanged; rifle already runs 0.25) | none |
| reach_via_heavy / reach_via_out_m_heavy | 0 / 0.12 | keep 0, or out_m_heavy 0 | via bump adds up to 0.12 m outward mid-carry AFTER the hinge (Holster.cpp:1752) | outward, widening only | heavy via off does not change the verdict (vo judges rifle/qstaff) |
| kHingeS3 (code, HolsterReach.h:257) | 0.30 | 0.40 | grab peak 8.1 to 6.1 m/s; pitch 3.3 to 4.2 m/s; clearance unchanged | needs code plus gate; PB_F_MAX is separate, so the checker is unaffected | none |

Inert for HingeBack: reach_bulge_heavy and reach_pole_over_heavy. Holster.cpp:1734 computes the bulge line, 1737-1740 overwrites T and pole when hinge is true, and 1760 skips reachPole. They only matter if heavy is topo=over.

If heavy is OverBack: bowed line with bulge 0.60 up, 0.25 out, times arm times sin(pi f). Peak up 0.38 m mid-carry, path 1.1 m, speed 2.5 m/s, and the GUESS head sim dips 0.06 m into the sphere. Lever: reach_bulge_heavy up term (not simulated).

## 5. Open
- Heavy live topo: read `topo=` on cat-6 REACH lines through the kotor2-log-analysis skill (no raw grep on .txt logs). Hinge means this angle applies; over means use reach_bulge_heavy instead.
- Live holster.ini in the game dir: confirm no reach_hinge_out_heavy or reach_via_heavy overrides (repo template has them commented / 0).
- Head sphere height and back-mount height are unmeasured (HW32b).
