# HW45 wrist split: verification and resolution (recon, Sonnet verifier)

Scope: patch_manager_mods/holster_src/HolsterReach.h reachSplitWrist / reachPickContinuous (caps twist 80 / swing 60), doc 42c rows HW45 + 4.16. No repo source edited.
Probe reports (copied unchanged): A_probe_a/b/c.md (wave 2), P1..P4 (wave 3). Verifier scratch: `scratch/` (chain.cpp, w.cpp, f.cpp, bind.py).
Build: `g++ -std=c++17 -O1 -I patch_manager_mods/holster_src -I patch_manager_mods/_shared_src -I tests/include scratch/chain.cpp` (run `./a.out 120 260`).

## 1. Verdict
P3's key claim is REFUTED as stated. HW45 is NOT a sweep-only artifact: the production chain feeds the split the same inputs as `runWristSweep`. The sweep's
"raw 233 deg" is a rotation of 127 deg about the opposite axis (the sweep flips dq to w >= 0), and production produces exactly that after a turn passes 180 deg.
P3's second claim (a 180 deg clamp truncates continuous turns, error 20/120 deg) is also REFUTED: in float32 there is no truncation (0.00 deg error over a 0..360 turn).
What stays true: no mechanism in 7 probes passes both gates (step < 5 deg and E <= E0+1); the gates look jointly unsatisfiable inside the 80/60 caps.
Whether real draws cross the 180 deg flip is still unmeasured (no log field carries the angle of dq). Resolution: measure first (R2), pin the chain with a test, decide caps/gates only on data.

## 2. Reachability (verified on disk and by running)
- Only caller of reachSplitWrist in the DLL: Holster.cpp:1859. Preview path does not call it (rg over patch_manager_mods: only Holster.cpp, HolsterReach.h, 2 tests). Unclamped feeds: none.
- Holster.cpp:1855 calls reachWristDelta(..., maxDeg 180, weight w*pct, hint = e->wristDq). e->wristDq = dqTot = that return value (Holster.cpp:1882), which blendQ makes w >= 0.
- The split therefore always gets angle <= 180 (w >= 0), same as the sweep. The question is not "can it exceed 180" but "does the input pass through 180 and flip axis".
- Why the 180 clamp does not truncate (HolsterReach.h:528-540 + blendQ 478): a hint-aligned dq of 190 deg has w < 0, clamp returns (cos(h), sin(h) a) with h = 180*pi/360 in float
  = 1.5707964 > pi/2, so w = -4.37e-8. blendQ(identity, that, 1) sees dot < 0 and flips to (+4.4e-8, -a). The hint axis flips at exactly 180, the next frame follows the shorter arc.
  scratch/f.cpp prints: clamp w=-4.37113883e-08, blend w=+4.37113883e-08 x=-1.
- scratch/chain.cpp (reachWristDelta(180, hint chain) -> reachSplitWrist, turn 120->260 about the 3 test axes, weight 1) is identical to the sweep, number for number:
  axis0 maxE 149.61 / E0 119.96 / 266 bad steps / first bad 233.5; axis1 144.69 / 119.94 / 236 / 236.5; axis2 131.30 / 119.96 / 123 / 247.8. Input axis flips at 180.0.
  maxDeltaTruncVsTrue = 0.00 (received dq equals the true turn as a rotation). Turn 0..360: maxE 179.97 / 175.47 / 137.48, still no truncation.
- For turns <= 179.5 deg the chain has zero bad steps (E == E0), so any fidelity loss starts only after the input crosses 180 (needs a turn that passes 180 deg).
- Fragility: the no-truncation result rests on cos(float(pi/2)) being negative in float32. A different libm/rounding would change it. No test pins it.
- Side finding (new): with weight < 1 the nlerp in blendQ spans the flip. scratch/w.cpp, turn 100->260: weight 1.0 max input step 0.10 deg; 0.9 gives 25.5 deg; 0.6 gives 134.8 deg, both at 180.
  Weight is the arm weight w * reach_wrist_pct, so a crossing during a weight ramp would make a large one-frame jump in what the split receives. Not probed in play.
- Real-data context (frozen REACH logs, .claude/waves/hw44slow/holster_log_snap3.txt, joined n): melee 1067 rows: swing at cap (>=59) 881, twist at cap (>=79) 253, both 253; g >= 170: 400.
  rifle 522: swing at cap 416, both 41, g >= 170: 5. heavy 5: none both. The caps bind in most melee frames, so the capped regime is the normal case. snap3 predates the pk=/lw= tail, so no pick data.
  g is the mount-vs-hand angle (0..180), not the mid-path dq; dq crossing 180 is unobserved.

## 3. Mechanism table (spot-check = number re-run or re-read in this pass)
Baseline everywhere: raw 120->260, 0.1 deg, axes 0-2. Gate (i) step sw and comb < 5 deg; gate (ii) E <= E0+1 every step. All reproduced the baseline 149.61 / 119.96 / 266.

| probe | idea | best numbers | why it fails | spot-check |
|---|---|---|---|---|
| A a | glide the written rotation toward cand 0 (k 1/2/4 deg/frame) | sw step 160-163, E>E0+1 in 26-123 steps | roll flips 160 deg at the flip, swing takes it; E0 drops 120->33 in one frame, a rate limit cannot follow | read |
| A a2 | glide toward dqn, plus roll continuity | max E 28.38 / 24.66 / 8.14, 0 bad steps, steps = k | passes both gates literally but swing reaches ~180 vs cap 60 | re-run m_a2_k1: sw step 160.06, E 28.38 confirmed |
| A b | hysteresis band 5/10/20 deg on the pick (literal and nearest-incumbent) | literal: 96-121 deg jump; nearest: baseline unchanged | candidates are 120+ deg apart; any switch is a 120 step, never switching keeps baseline E | read (baseline re-run) |
| A c | one shared roll state, roll rate-limited 1/2/4 deg | 120 deg sw step every budget; 38 gap steps at k=4 | swing w changes sign at 180 while the roll is still on +80 side | re-run m_mode1_b4: sw 119.99, 38/38/39 gap steps confirmed |
| P1 | look-ahead twist ramp S..E | S=250: step 1.4-1.6 / 2.7-2.8, E 179.7+, 344 bad steps | swing must change side where w crosses 0; ramp only trades the jump for 180 deg error | re-run la S=250 mode 2: stepSw 1.36 stepComb 2.65, 344 confirmed |
| P2 | clamp dq to 140 once, then split (plain / hemisphere) | plain: step 25.0, hemisphere: step 0.06 but E 153.6, 986 bad steps | plain clamp sign follows shorter arc; the 140 ball is looser than the 80/60 product set | re-run m1, m2: 25.02, 153.62 / 986 confirmed |
| P3 | reachability from code and logs | see section 2 | claims refuted here | re-derived, 2 claims refuted |
| P4 | cross-fade between cand 0 and cand 2 | N=5: step 21/52, E 160; N=20: step 5.3/13, E 176.6 | fade passes zero twist where comb = swing alone (E ~ 180); looks better only at tiny d (sampling) | baseline row re-run (149.61/266) |

## 4. Structural argument
- Gate (ii) asks the written rotation to stay within ~1 deg of candidate 0's error. Candidate 0 is a nearest-point projection of dq onto the reachable set R = {twist <= 80} x {swing <= 60}. As dq crosses 180 deg
  its shorter-arc axis flips; the projection jumps to the other side (E0 falls 119.96 -> ~33 in one 0.1 deg step; comb0 step 130 deg, A c cut.cpp).
- Gate (i) forbids more than 5 deg per call, but the two projection branches are 120-135 deg apart (A b diag: 120.96 / 130 deg), and a continuous path between them inside R passes through low-fidelity
  states (cross-fade through zero twist: E 176-180, P4; ramp: E 180, P1). The only variants meeting gate (i) and gate (ii) literally (A a2 + roll continuity) drop the swing cap (~180 deg).
- So with caps 80/60 held, the gates are jointly unsatisfiable (not formally proved; supported by seven independent mechanisms and the geometry above). One of the three must give: continuity (accept a ~120 deg
  one-frame step near the crossing), fidelity near the crossing (accept E up to ~180 for a short window), or the swing cap (uncapped swing, a2). reachSplitWrist's current choice keeps continuity and
  gives up fidelity (E 150 vs E0 120, no step).
- Open prior question for the geometry: for dq = 180 the best reachable error is already ~120 deg (not 40), because the 80/60 set is not a 140 deg ball (P2 point 3). So wrist fidelity near the flip is poor even for candidate 0.

## 5. Remaining real risks
1. Unknown frequency of the flip. If a real draw's dq never reaches ~170 deg mid-path, HW45 never triggers in play. The melee g >= 170 share (400 of 1067 frames) says g is large at the grab/landing, not that dq crosses 180 mid-path.
2. Which mounts could cross: melee on hinge+back mounts (g max 180 at f 0.71, 47 frames >= 179.5), rifle (5 frames >= 170). Pistol hip, heavy: not near. Lightsaber, qstaff, thigh, over: not sampled.
3. Weight < 1 at a crossing (section 2 side finding): up to 135 deg one-frame jump in the split input. Only matters if the crossing lands in the arm weight ramp.
4. Float32 dependence of the no-truncation behaviour (section 2). A change to reachClampRot rounding or maxDeg would silently introduce the truncation P3 feared.

## 6. RECOMMENDED RESOLUTION (ranked)
The task's R1 as worded ("sweep the clamped input range") has no effect: the sweep input already is the clamped range. Reworded below.
- R2 (do first, log-only, ~15 lines): add the angle of dq fed to the split to the REACH diagnostic tail.
  File/function: HolsterReach.h reachDiagTail (add `dq=%.0f`, keep piece <= 200 bytes) and Holster.cpp ~1859 (pass reachQAngleDeg(dq)); also log the previous-frame dq angle jump (|dq| step > 60) as `dj=`.
  Pin: holster_reach_test.cpp reachDiagTail length cases (extend the existing <=200 byte check) plus scripts/holster_log_check.py summary line (max dq per category; frames dq >= 170; frames with dj > 60).
  Play: SM-L18 batch (holster 0.21.3-dev), copy the log before relaunch. Pass signal: max dq per category < 150 means HW45 cannot trigger in play and can close as "reachable only in synthetic sweep"; dq >= 170 or dj > 60 means go to R3.
- R1' (with R2, test only, ~40 lines): pin the production chain. holster_twist_test.cpp: add a test that runs reachWristDelta(180, hint chain) then reachSplitWrist over 0..360 and asserts (a) received dq equals the true turn
  as a rotation (no truncation, maxDelta < 0.5 deg), (b) the input axis flips at 180 deg, (c) identical E to runWristSweep. Run: `python3 scripts/run_tests.py holster_twist`. This turns the float32 dependence into a failing test instead of a silent regression,
  and documents HW45 as "the regime the sweep reaches, reachable by a turn that crosses 180". It does not change behaviour. Keep the existing pinned bad-case assertions (maxStep unguarded > 60) unchanged.
- R3 (only if R2 shows dq >= 170 or dj > 60 in play; a decision, not a fix): choose which gate to give up.
  (a) accept the continuity-over-fidelity status quo and document it in doc 42c 4.16 (0 lines, closes HW45 as WONTFIX with data);
  (b) raise reach_wrist_max_deg toward 120-180 (ini, no code; reach_twist_deg 80 stays) and re-run the sweep gates: A a2 shows the swing cap is the lever (max E 8-28 with uncapped swing); needs a live look at the wrist bend;
  (c) A a2 + roll continuity as code (reachPickContinuous / reachSplitWrist, ~20 lines), tests: new gate in holster_twist_test.cpp (step < 5, E <= E0+1, swing cap relaxed to the chosen value);
  (d) if the weight < 1 jump shows up in the log, fix the nlerp across the flip (blendQ sign choice against the hint in reachWristDelta, ~8 lines; test in holster_reach_test.cpp with weights 0.6 / 0.9).
- Do not spend more Haiku probes on the pick: all seven mechanisms are exhausted for the 80/60 caps (P1/P2/P4 open questions: projection onto the true product set, comb-space writer; untested, low expected value given section 4).

## 7. Corrections to the probe reports (README only)
- P3 section 1 and 4 (HW45 is a sweep-only artifact; split never gets the 233 regime): wrong. Same input as the sweep after any turn that crosses 180 deg (sections 2).
- P3 section 3 and clamp_chain_sim.py (error 20 / 60 / 120 deg from the 180 clamp): wrong in float32; the python replica used exact cos(pi/2) = 0 and lost the blendQ flip.
- P3 section 2 g table: numbers consistent with my re-derivation of the join (melee/rifle/heavy); "none reaches 233" is irrelevant (angles are always <= 180 by construction).
- A c note and P4: baseline numbers reproduced; no corrections.
