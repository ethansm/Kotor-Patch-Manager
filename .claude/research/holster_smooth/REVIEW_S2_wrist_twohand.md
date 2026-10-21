# REVIEW_S2: carry / wrist / orientation / two-hand / dual (Sonnet adjudication)

Anchors: `patch_manager_mods/holster_src/` (Holster.cpp = H, HolsterReach.h = R, HolsterSlide.h = S). Draw 350 ms, grab 30% => carry = 0.7*350 = 245 ms.

## 1) Verdicts
| id | source | claim | verdict | anchor | build impact |
|---|---|---|---|---|---|
| R4a | Gemini 3 | wrist cap defaults to 90 | TRUE | H:91; parse allows 0..180 H:353-356 | raising the cap is an ini edit, no code |
| R4b | Gemini 3 | the weapon "instantly snaps" across the >90 remainder | PARTIAL | H:1362 (dq clamped); H:1500/1758 blendPose(hip, carried, carryW); R:328 `carryW=easeSmooth(q/0.1)` | no value jump at the grab (carryW=0 writes the mount exactly), but the excess G-90 rotates in the first 10% of carry = 24.5 ms (1-2 frames, smoothstep peak ~1.5x mean). Stow mirrors it (R:337). Hand is capped, weapon is not => continuous but reads as a pop |
| R4c | Gemini 3 | compute the wrist delta AFTER shoulder/forearm writes, not from the cached rest hand | FALSE as a bug (already built) | H:1358-1362: gate wrote==3, `cur`=pHn re-read after both writes; `to`=pH.q is the animation hand (right target) | drop this item |
| R4d | Gemini 3 | delay the wrist slerp (hold t in [0,0.25]) | PARTIAL | R:401 uses easeSmooth(pathF) (0.156 at pathF 0.25), nlerp not slerp | cheap tweak (smootherstep / power) but does not fix the grab excess, only head clearance |
| R4e | claude 8-11 | residual readout + hand-frame decay | UNKNOWN value, TRUE gap | H:1757-1759 uses a WORLD blend with a 10% window; B_carry gap #3 | best snap remedy, see step 2-3 |
| R10a | Gemini | raise cap to 135-160 for hip mounts | PARTIAL | H:1362 | feasible now (3 writes). Does not remove the snap, only shrinks G-cap. Gemini's justification (mount rot values = "relative to drawn hand") is not a computed angle; the real G is post-IK and unmeasured (wr= is logged at H:1374 but scripts/holster_log_check.py skips it) |
| R10b | claude 9 | swing/twist split (twist on Rforearm_g, swing on wrist) | TRUE feasible | forearm write H:1344-1348, wrist write H:1360-1364 | uses the same 3 bones. The hand sits on the forearm axis (elbow->hand), so a twist about that axis does not move T. Replaces one cap with two |
| R10c | claude 8 | elbow swivel from the hand's target orientation at the grab | PARTIAL | solveTwoBone takes a pole (H:1320-1334); hinge sets its own pole (R:~242, stage 3 poleHigh) | feasible as a pole choice, no new bone. Conflicts with the tuned HW23 hinge (elbow parked forward, 6 cm inboard rule, kHingeMinOut) => only a bias at f<=0.30 |
| R11a | Gemini 2 | carry = reach path reversed, no extraction | PARTIAL | B1: geometry shared, no extraction segment (R:326-340) | needs a 3rd carry phase, see step 6 |
| R11b | claude 4/6 | piecewise path, mouth pivot, via-orientation | UNKNOWN | no weapon length/axis at runtime (kStems has category only, HolsterLogic.h:59-71) | blocked on shape data (S1 reviewer); do as additive offset on T, not a new path |
| R12 | claude 6 / H2 | per-call sign test flips near 180 deg | TRUE (mechanism), live angle UNKNOWN | S:31 `dot<0`; R:351 blendQ; R:388 `dq.w<0` forced positive | per-Slide hemisphere lock, step 1 |
| R13a | Gemini 4 / claude 14 | off-hand support reach is absent | TRUE | arm cache keyed (parent,left) H:1235; 3 reachArm call sites use the Gob's own side (D2) | feasible: a 2nd reachArm(left=true) at H:1500 and H:1758 |
| R13b | Gemini 4 | left arm "swings idle then teleports" | UNKNOWN | style byte only read (H4a); engine g3r1/g5/g6 clip may already pose the left hand | probe first (step 0 / live check L3) |
| R13c | Gemini 4 | "15 two-handed, 15 dual" | PARTIAL | D1: 15 = right-only slot 0x10; dual = 14 + 3 creature | doc wording only |
| R13d | both | runtime two-handed flag | FALSE (absent) | classify(.,0) at H:1093, 1453, 1609, 1862; isLong has no DLL caller | model-name table is enough (below) |
| R13e | claude 15 / Gemini 4B | dual stagger | TRUE trivial | each Gob has its own Slide; reset H:1685, target H:1690; S:51 `t0=now` | `t0=now+delay` on the left Slide, ~6 lines |

## 2) Gemini vs Claude conflicts and resolution
- Wrist range: Gemini = raise the cap; claude.ai = swivel + swing/twist. Resolve: both are bounded-angle fixes; neither removes the pop alone, because the attach switch is a world blend. Use hand-frame residual decay (claude 11) as the snap fix, swing/twist as the range fix, and a raised cap (120 first) only as a quick interim.
- Joint-limit risk: the engine enforces none (raw quaternion writes). Risk is visual: rigid per-bone meshes show a seam at the wrist. Twist about the forearm axis is seam-safe (cylindrical), wrist flex is not. So prefer twist cap ~80 on Rforearm_g, flex cap ~60 on the wrist, and avoid a single 150 deg wrist write. A 160 cap on the hand bone alone is the riskiest option.
- Extraction: Gemini wants a per-category hard-coded vector; claude.ai wants it derived from bbox length. Resolve: additive T offset along a per-stem exit axis from a data table until bbox runtime (H8) lands.
- Support hand: Gemini builds a foregrip solver now; claude.ai says check the style first. Claude is right (H4a/D2 section 4 UNKNOWN); probe before building.
- Stagger: Gemini 35-50 ms + 0.08 m lateral; claude.ai 60-120 ms. Take 60 ms as default, ini-tunable; lateral clearance needs the geometry slice, skip.

## 3) Build steps in order
0. Instrument (no behavior change): extend `_reach_events` in `scripts/holster_log_check.py:85` to parse and report wr=, hg=, f= and the wrist bit (wrote&4); in `H:reachArm` add `g=%.0f` = angle(mountQ, pHn.q) at the first carry frame and the capped dq angle. Test: `scripts/holster_log_check` unit test with a planted line.
1. Hemisphere lock. `S`: add `Q lockH; uint8_t lockOn` to Slide (cleared in reset/target); new `blendPoseLocked(mount, hand, f, Slide&)` negating fresh h when dot(lockH,h)<0 and storing it; first frame picks the sign by the old test. Use at H:1494, 1500, 1747, 1758. Wrist: store last dq in ReachEnt (cleared at `!w`), pass a sign hint to `reachClampRot` (R:388) so the axis does not flip; allow w<0 when it continues the previous direction. Tests in `holster_slide_test.cpp` (jitter 179.5..180.5, expect 0 flips; Haiku measured 46 vs 0) and `holster_reach_test.cpp` (dq axis continuity).
2. Hand-frame residual (claude 11). At the first carry frame store `Rres = conj(carried.q)*hip.q` and `pres` in the hand frame (per ReachEnt); weapon = carried * slerp(Rres, I, ease(t/decay_ms)) with decay 120-150 ms (ini `reach_resid_ms`). Replaces the 10% carryW window at H:1500/1758; mirror for stow landing. Test: new reachResidual in HolsterReach.h, assert max per-frame angle step < X at 60 fps for G=60..180.
3. Reversal/fallback snaps (B list items 1, 2): carry pathF/carryW into the new slide; hold last `rid` when reachArm returns false instead of dropping to `out`. Test: reachPlan reversal continuity.
4. Swing/twist split in `reachArm`: after the forearm aim, apply twist (cap `reach_twist_deg` 80) about the elbow->hand axis into outF, wrist gets the swing remainder (cap `reach_wrist_max_deg` 60). Raise `reach_wrist_max_deg` interim to 120 via ini only. Test: reachSwingTwist in `holster_reach_test.cpp` (recompose == input, twist leaves the hand position unchanged).
5. Pole bias from target hand orientation at f<=0.30 (claude 8) behind a flag, after step 4 shows residual by the readout.
6. Extraction (only after the shape slice): reachPlan gains an extract share e; T += exitAxis*len*bump(q); orientation progress oF separate from pathF (hold the mount orientation through e) at R:401; per-stem `ext_len`, axis in kStems. Arm reach will clamp (rr<1, hand error), accept it.
7. Two-handed + dual: `isTwoHandedStem` in HolsterLogic.h (CatQStaff/Rifle/Heavy plus Dblswrd, VbrDblswd, ForcePike, gaffi, warblade; test in holster_logic_test.cpp). Dual stagger: `Slide::target` gets `startDelayMs` (`t0=now+delay`); apply when ent->left and the owner has a right Gob. Support reach only if probe L3 shows the left hand off the weapon: 2nd reachArm(left=true) with a foregrip target (weapon axis offset), skipped when a left-weapon Gob shares the ReachEnt, w=0 call undoes at slide end.

## 4) Live checks (queue in PLAY_TEST_QUEUE.md, never block)
- L1 grab excess: `holster_log_check` prints g= and wr=; pass = for saber/pistol/back sword g-cap < 20 deg after steps 2+4, residual max step < 25 deg per frame.
- L2 flip: log dot(hipPose.q, in.q) over one saber slide; pass = |dot| stays > 0.05 or lock keeps 0 sign flips in the log.
- L3 style/left hand: log style byte (+0xC5) and lhand-to-weapon-axis distance at slide start, mid, end for a rifle and a staff; pass = distance at mid < 0.15 m means the engine already poses it (no support IK).
- L4 dual stagger: two vibroswords, no clipping at the midline with 60 ms offset; pass = player sees no blade crossing.
- L5 persist: record RCH1 persist=0/1 for a left support write (D2 open).

## 5) Haiku errors corrected
- B4 said the swing is G-90 with G = angle(hip, hand): the clamp is on `target*conj(pHn)` (post-IK hand), so the true excess is angle(mountQ, pHn)-cap, not measured vs the animation hand.
- B expects `wrote=7` and RCH1 pass tolerance in the queue: correct, but the checker masks `&3`, so even the live log cannot show the wrist today.
- H2 says keep the previous wrist dq sign instead of forcing w>=0: right idea, but a lock needs `reachClampRot` to accept a sign hint and handle w<0 (ang>180); a plain flag swap is not enough.
- D5 and D2 agree on weaponWield=0; add that CatQStaff/Rifle/Heavy are already known by category without any new table, only five melee stems need a flag (Dblsbr is CatLightsaber, hip).
- D4 "no clearance" is TRUE, but stagger needs only `t0=now+delay` (S:51), not a per-hand clock rewrite; touch() gap guard stays valid.
- D1: Gemini's "15 two-handed" counts slot 0x10 right-only (incl. rifles); runtime needs the style-based truth, not that count.
