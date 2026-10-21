# 42c — Smooth holstering: master plan (draw/stow arm reach, HW24-HW34)

Parent: `patch_manager_mods/42_holstered_weapons_master_plan.md` (rows HW24-HW34 point here). Facts doc: `42_holster_mounts_and_weapon_types.md`. Source: `patch_manager_mods/holster_src/` (manifest id `holster`, 0.x.y until the KPM upstream PR). Live: holster 0.11.0-dev (HW23 hinge).
Inputs: two external design reviews, `42_holster_mounts_and_weapon_types_Gemini_Argon.md` (Gemini) and `42_holster_mounts_and_weapon_types_Claude.md` (claude.ai), checked against the code by 17 Haiku recon agents and 3 Sonnet reviewers (static only, no game). Evidence: `.claude/research/holster_smooth/` (A, A2, B, B2, C, C2, D, D2, E, E2, H1-H4, H6, H8 + `REVIEW_S1_topology_shape.md`, `REVIEW_S2_wrist_twohand.md`, `REVIEW_S3_timing_snaps.md`). H5 (saber blade) reported inline: no evidence, play check needed.
Resume: `python3 scripts/plan_manager.py resume patch_manager_mods/42c_smooth_holstering_master_plan.md` (or `42:HW24`).

Anchors below: `H` = `Holster.cpp`, `R` = `HolsterReach.h`, `S` = `HolsterSlide.h`, `L` = `HolsterLogic.h` (all in `holster_src/`). Line numbers are from 916c54b1 and drift; search the symbol.

## 0. Goal and definition of smooth

Smooth = in every draw, stow and reversal, no weapon or arm bone moves more than it would at the slide's own speed in one frame. Measured, not judged by eye:
- hand path position step per frame < 0.05 of the arc (no jump), arm weight step < 0.1;
- weapon orientation step per frame < 25 deg at 60 fps, no hemisphere flip;
- no slide cut short or restarted without a `SLIDE cut` log line;
- grab and landing residual (carried weapon pose vs mount pose) logged and below 20 deg / 3 cm after HW26 + HW29.

## 1. Verdict summary (both reviews vs the code)

| Topic | Gemini | claude.ai | Code truth | Plan |
|---|---|---|---|---|
| `torso_g` is lumbar but gets the over-shoulder pole | yes | yes | TRUE: `reachIsBackBone` H:1202 forces u=1 (H:1310); `torso_g` z 1.139 vs pelvis 1.126 | HW27 topology |
| Path chosen by bone name (4 topologies) vs by grip geometry | bone | geometry | Code is already a height continuum (`reachOverWeight`) + bone override. Height alone cannot split lumbar from hip (-0.52 vs -0.54 arm): needs depth and side too (neither review says so) | HW27 geometry with depth/side |
| Hinge gate input | - | - | Gate uses the PLACED weapon point, not the bone (H:1303-1306, 1498, 1756): offsets decide hinge yes/no | HW24 logs it, HW27 uses it |
| Hip bow zero at hip height | - | yes | FALSE at pelvis (u 0.34-0.85 live), TRUE only at thigh (nothing uses thigh); R:137 comment stale | HW27 recentre band |
| Thigh drop-and-piston path | yes | moving target | No thigh path; no live item on a thigh; mount is re-read each tick already | Deferred |
| Wrist 90 deg cap snaps | at grab | replace cap | No pop at the grab: the excess rotates in the first 10% of carry (~24.5 ms, `carryW`, R:328) as a fast whip. Delta already computed after the arm writes (Gemini's "compute after IK" is already built) | HW26 residual, HW29 swing/twist |
| Fix: raise the cap to 135-160 | yes | no | Feasible (ini) but a 150 deg single wrist write is the most visible seam; does not remove the whip | Interim 120 only |
| Wrist slerp is linear | yes | - | FALSE: smoothstep on pathF (R:401) | Drop |
| Extraction before the turn | per-category vector | from shape data + mouth pivot | TRUE gap: carry = reach path reversed (R:326-340) | HW31 after HW30 |
| Grip offset +0.21 m on the reach target | yes | grip = origin | Gemini FALSE and harmful: hand already aims at the placed origin, which the weapon is pinned to; an offset breaks the hand-off. Double saber grip 0.207 m off origin is real but belongs in placement after the G7 probe | Rejected; HW30 optional |
| Shapes unmeasured | partly | bbox at load | K2 fit survey (R17) + `inventory_3d_viewport_src/ItemFitTable.inc` measure most stems; doc 42 sec 4 uses K1 sizes. Runtime bbox gives axis+length only | HW30 offline table |
| Heavy held low, needs a coronal sweep | yes | heavier P6 | Heavy uses the same hinge, no category input (H:1321); held pose unmeasured | HW27 per-category hinge params, play check |
| 570 ms draw clamp | raise to 750-850 | ask first | Not engine-bound: 570 = latest `draw_weapon` anim event (12 clips); no attack delay, no attach desync. Cost: arm override fights the first swing longer | HW28 per-category inside 570; cap raise gated |
| Grab dwell / easing | 30 ms plateau | dwell + residual decay | Draw grab leaves at 1.43x speed from rest; stow arrives at full speed. A plateau costs 30 of ~105 ms | HW25 easing, no plateau |
| Reversal mid-slide | - | continue from hand | Today the hand JUMPS up to 0.97 of the arc and arm weight drops to 0 for a frame (R:326, 337; S:65-80) | HW25 (top fix) |
| Hard resets | - | snap to safe end | Unreadable owner/combat or a new Gob resets the slide (H:1676-1690): hand pop, then a hip pop or full stow | HW25 grace |
| 180 deg flip | - | yes | Mechanism TRUE: per-call sign test (S:31, R:351, R:388) flipped 46/400 jittered frames; live angle unmeasured | HW26 hemisphere lock |
| Creature weapons on the upper back | - | yes | TRUE: `w_cr*` -> CatMelee (L:65) -> `place_bone_melee=torsoUpr_g`; CatUnknown placed at the pelvis | HW27 `holsterable()` |
| Off-hand support for two-handers | build now | probe style first | No support reach; arm cache is per owner+side (H:1235) so feasible; weaponWield always 0 (H:1609); left-hand pose in the clip unknown | HW33, probe first |
| Dual-wield stagger | 35-50 ms + 8 cm | 60-120 ms | Trivial: `t0 = now + delay` on the left Slide (S:51) | HW33, 60 ms default |
| Saber blade during the slide | - | gate ignition | UNKNOWN; mod never touches ignition (engine 0x7a7620) | HW34 gated on play check |
| Anim style flip mid-slide | - | yes | Style byte engine-owned, never written; rest hand read live each frame (walk swing moves the target) | HW24 log, play check |
| Residual readout, sweep check | - | yes | Residual: gap is real (carried vs mount at the `carryW` step). Sweep: offline only | HW24 log, HW32 preview |

Both reviews and the earlier Sonnet comparison missed: the reversal jump with arm-weight collapse, hard-reset pops, the stow-arrival speed step, depth/side for the classifier, and that `reach_pole_*` / `reach_bulge_*` do not act on hinged back mounts.

## 2. Likely causes of "snapping interferes with the delayed draw/holster" (ranked)

1. Mid-slide reversal: every combat / flourish / party-hold / move-hold flip inside a slide. Delay features make these common (re-engage during a delayed stow, 250 ms party edges). Hand path jumps (0.97 at reach f=0.02, 0.64 at 0.10, 0.21 at carry p=0.5) and arm weight drops to the engine pose for one frame. Longer slides = more exposure. HW25.
2. Saber blade/hilt split (hypothesis): the engine ignites or douses at the combat flip while the hilt still travels. HW34 after play check.
3. Hard resets: transient unreadable owner/combat or Gob rebuild mid-slide. Silent, largest jump. HW25.
4. Stow arrival at full speed: a small hitch every stow. HW25.
5. Sparse attach refresh then catch-up, refusal frames showing the engine pose. HW24 measures it.

## 3. Master plan table

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| HW24 | Instrument: log checker parses `wr= hg= f=` + wrist bit; REACH adds `dzA fy fx g resid`; `SLIDE cut` on resets; flip dot (no behaviour change) | HW23 | - | DONE | 8c9c2b00 | logging only; committed hunks-only (peer HW16 hunks left in tree); RESID is its own line; not installed |
| HW25 | Continuity: reversal keeps path position + arm weight, hard-reset grace (3 frames), ease draw-carry start and stow-carry end | HW24 | - | DONE | 8933628a | B1: 3067c033 720c1dc5 8933628a; holster 0.14.0-dev gated GATE PASS, not installed; play SM-L1/SM-L2 |
| HW26 | Orientation: hemisphere lock (slide + wrist), hand-frame residual decay replaces the 10% world blend, interim `reach_wrist_max_deg=120` | HW25 | - | DONE | 9b385995 | 7c314c47 9b385995; lock + sign hint + hand-frame residual 240 ms + reversal cross-fade, wrist max 120; holster 0.15.0-dev GATE PASS, not installed; play SM-L3 |
| HW27 | Mount sense: `holsterable()` (no `w_cr*`, no CatUnknown), `reachTopology()` from placed point (height, depth, side), recentred hip band, per-category hinge out params | HW24 | - | DONE | 9b385995 | 796f3bf9 b5fa6e7f 9b385995; holsterable + reachTopology (LowBack dropped) + band -0.60..-0.38 + hinge out per cat; play SM-L4/SM-L5 |
| HW28 | Timing: `draw_slide_ms_<cat>`, `stow_slide_ms_<cat>`, `reach_grab_pct_<cat>`, clamp warnings; per-profile defaults inside 570 | HW25 | - | DONE | 9b385995 | f43605d7 9b385995; per-cat slide ms + grab pct, ini globals commented; play SM-L10 |
| HW28b | Draw cap raise (`draw_slide_cap`, default 570) | HW28 + play SM-L6 | user GO | GATED | | only if the long-draw play check passes |
| HW29 | Swing/twist split: twist on the forearm (cap 80), swing on the wrist (cap 60); optional pole bias from the target hand orientation at f<=0.30 | HW26 | - | DONE | 77ebe175 | a4bb7d86 77ebe175; forearm twist (reach_twist_deg 80) + wrist swing (reach_wrist_max_deg 60), twist 0 = 0.16 path; holster 0.17.0-dev GATE PASS, not installed; play SM-L11 |
| HW30 | Shape data: generated stem table `HolsterShape.inc` (axis, length, exit axis/len) from R17 / ItemFitTable; doc 42 sec 4 refreshed to K2 numbers | - | - | DONE | 18313018 | 18313018 b701008d; HolsterShape.inc from committed R17 (scripts/gen_holster_shape.py), shapeOf with family fallback; exitSign only known for melee sheaths; doc 42 sec 4 K2 sizes |
| HW31 | Extraction + orientation schedule: extract share, T offset along the exit axis, hold mount orientation through extract, via-orientation for rifle/staff | HW29, HW30 | - | DONE | 9222a06e | 7ed618e5 headers + 9222a06e wiring, holster 0.18.0-dev gated (GATE PASS head=c64e0c5a), not installed; play SM-L12 |
| HW32 | Offline sweep check in preview-3d: tip/butt clearance vs head/torso proxies per weapon x mount x hand | HW30 | - | DONE | 434cb1fd | offline sweep holster_sweep_test; back-mount quat is a GUESS (clips at rest), so back numbers are not tuning inputs |
| HW33 | Two hands, item 3 only: support-hand reach ONLY if play SM-L8 shows the left hand off the weapon (items 1, 2, 4 moved to HW33a) | HW33a + play SM-L8 | - | GATED | | section 4.10 item 3 |
| HW33a | Two hands, safe parts: `isTwoHandedStem`, Slide pending target (`targetDelayed` + `pump`), dual stagger `dual_stagger_ms` (60), `party_jitter_ms` (0) | HW25 | - | DONE | 724ea472 | B5 holster 0.19.0-dev gated (GATE PASS head=8bb5f3b7), not installed; play SM-L13/SM-L14 |
| HW35 | Rest-pose MOUNT log (bone, bone quat, mount quat, owner facing, body-relative `rel=conj(oq)*mq`) + `holster_log_check.py` `mounts` summary | HW32 | - | DONE | 724ea472 | B5 holster 0.19.0-dev gated (GATE PASS head=8bb5f3b7), not installed; play SM-L13/SM-L14 |
| HW36 | Ini exit overrides `reach_exit_sign_<cat>` (-1/0/+1, auto) + `reach_exit_len_<cat>` (0..0.5 m, auto), so exit sign can be tried in play without a rebuild | HW31 | - | DONE | 724ea472 | B5 holster 0.19.0-dev gated (GATE PASS head=8bb5f3b7), not installed; play SM-L13/SM-L14 |
| HW31b | Via on the default residual path: two-segment residual `reachResidualVia` + outside-shoulder lateral offset `reachViaOffset` (`reach_via_out_m` 0.12) | HW31 | - | DONE | 247b579f | 691fa25b header + 247b579f wiring; via on residual path + reach_via_out_m 0.12 shared clock; reach_via_<cat> default 0; holster 0.20.0-dev GATE PASS head=247b579f, not installed; play SM-L15; next HW32b after SM-L13 logs |
| HW29b | Pole from grip: `reachPoleBias` behind `reach_pole_from_grip` (0) + `reach_pole_bias_deg` (20), f <= 0.30 only, HW23 hinge rules kept | HW29 | - | DONE | 247b579f | 7bdf9610 header + 247b579f wiring; reach_pole_from_grip 0 default; holster 0.20.0-dev GATE PASS head=247b579f, not installed; play SM-L15 |
| HW32b | Re-run the sweep with the measured back-mount quat, re-tune hinge out + extract defaults | HW35 + play SM-L13 logs | - | GATED | | runs as its own phase once SM-L13 logs exist |
| HW34 | Saber blade gate (ignite after the swing clears, douse before the stow carry) | play SM-L9 | user GO | GATED | | only if the blade shows at the hip |

Install batches (each a hot-swap: DLL only, no hooks; `kpm_hotswap.py --mod holster`, user GO, game closed):
- B1 = HW24 + HW25 -> `0.12.0-dev` (fixes the reported snapping; first, smallest).
- B2 = HW26 + HW27 + HW28 -> `0.13.0-dev`.
- B3 = HW29 -> `0.14.0-dev`. B4 = HW30 + HW31 (+ HW32 tool) -> `0.15.0-dev`. B5 = HW33 -> `0.16.0-dev`.
- Actual versions after the peer's bumps: B3 = 0.17.0-dev, B4 = 0.18.0-dev. Next: B5 = HW35 + HW36 + HW33a -> `0.19.0-dev`; B6 = HW31b + HW29b -> `0.20.0-dev`; HW32b after SM-L13.

## 4. Batches

Common to every batch: write tests next to the code (`holster_reach_test.cpp`, `holster_slide_test.cpp`, `holster_logic_test.cpp`); gate `python3 scripts/run_tests.py holster` (scoped), `python3 scripts/compat_check.py patch_manager_mods/holster_src/*.h`, scratch `build_check` (master plan Traps), then `scripts/pre_install_gate.sh --mod holster_src --skip-default` -> `GATE PASS`; commit by explicit paths; play rows go to `PLAY_TEST_QUEUE.md` when the batch installs. Workers: Haiku for pure-header math + tests, Sonnet for `Holster.cpp` wiring; Opus reviews and commits. New ini keys get a CONFIG log field and a commented line in `holster.ini`.

### 4.1 HW24 Instrument (no behaviour change)
1. `scripts/holster_log_check.py` `_reach_events` (~line 85): parse and summarise `wr=`, `hg=`, `f=`, the wrist bit (`wrote & 4`; today masked `& 3`), and new fields below. Unit test with planted lines.
2. H REACH line (~H:1374): add `dzA=` (mlt[2]/arm), `fy=` (mlt[1]/arm, <0 = behind), `fx=` (mlt[0]/arm), `g=` (angle mountQ vs post-IK hand at the first carry frame), `cap=` (applied wrist angle), `resid=` (pos cm / angle deg between carried pose and mount pose at the `carryW` step, H:1755-1760).
3. H reset branch (~H:1684): `SLIDE cut reason=first|unreadable|gob frac=%.2f` when a reset hits an active slide; rate-limited.
4. Flip probe: log `dq=` = dot(hipPose.q, in.q) once per slide at mid-slide (S:31 site).
5. Style probe: log the style byte (+0xC5) at slide start/mid/end (answers SM-L8 and the anim-flip question).
Pass: logs only; `reach_test=0` path byte-identical.

### 4.2 HW25 Continuity (top fix for the reported snapping)
1. `S` Slide::target + `R` reachPlan (R:316-342): store `pathFrom` (path position from the OLD plan at retarget) and use it instead of the slide `from` in the stow-carry and draw-carry branches; a reversal during reach enters release from `pathFrom` with carryW 0. Test: |pathF(new, t+1) - pathF(old, t)| < 0.01 and carryW step < 0.05 for reversals at reach f 0.02/0.10/0.15 and carry p 0.5; fix `testReachPlanPartialDraw` (~test:618) expectation.
2. `R` reachWeightRamp (R:180-189) + H:1754: keep arm weight continuous across a retarget (start the new ramp from the current weight `w0`). Test near test:538.
3. H:1684-1686: split `first` from `!readable`; on `!readable` with a running slide hold the slide up to 3 frames (pure grace counter helper in `HolsterDrawGate.h`, tested) before resetting; log `SLIDE cut`.
4. reachPlan easing, total length preserved: smoothstep ease-in over the first ~15% of the draw carry, ease-out over the last ~15% of the stow carry. Tests: monotone pathF, exact endpoints, speed at the grab < 0.3 of linear.
5. On `reachArm` refusal mid-slide hold the last solved arm pose for that frame (store last rid) instead of dropping to the engine pose.

### 4.3 HW26 Orientation stability
1. Hemisphere lock: `S` Slide gains `Q lockH; uint8_t lockOn` (cleared in reset/target); `blendPoseLocked()` negates a fresh hand quat when dot(lockH, h) < 0. Use at the four blend sites (H:1494, 1500, 1747, 1758). Wrist: store the last dq per ReachEnt and pass a sign hint to `reachClampRot` (R:388) so the axis does not flip (allow w<0 when continuing). Tests: jitter 179.5..180.5 deg -> 0 flips (Haiku measured 46/400 without the lock); dq axis continuity.
2. Hand-frame residual (replaces the 10% world `carryW` window at H:1500/1758): at the first carry frame store `Rres = conj(carried.q) * mount.q` and `pres` in the hand frame (per ReachEnt); weapon = carried * slerp(Rres, I, ease(t / reach_resid_ms)), default 130 ms; mirror for stow landing. Test: max per-frame angle step < 25 deg at 60 fps for grab angles 60..180.
3. Ini interim: `reach_wrist_max_deg=120` default (still ini-tunable 0..180).

### 4.4 HW27 Mount sense
1. `L`: `holsterable(cat, stem)` false for CatUnknown and `w_cr*`; H ~1609 skips placement and reach (weapon stays in hand). Tests: 3 `w_cr*` stems + unknown.
2. `R`: `reachTopology(dzArm, fy, fx, hingeFits)` -> {HingeBack, OverBack, LowBack, Hip, Thigh}; replaces `if (backMount) u = 1` (H:1309-1310). u=1 only for HingeBack/OverBack; LowBack (back bone, no hinge fit, behind and below -0.45 arm) and Hip use the hip pole (0.60,-0.40,-0.40, close to Gemini's low-back vector). Tests from the A2 table: torsoUpr +0.22 -> HingeBack; torso_g no offset -> LowBack; pelvis -> Hip; thigh -> Thigh.
3. `R` reachOverWeight: recentre the band on the measured pelvis -0.54 (e.g. -0.75..-0.30), fix the stale R:137 comment. Test: u(-0.54) < 0.2, u(-0.36) ~ 1, monotone.
4. `R` reachHinge: per-category `outT` / `outPole` args (defaults kHingeOutT 0.10 / kHingeOutPole 0.25); keys `reach_hinge_out_<cat>`; defaults rifle 0.25, heavy 0.45 (Gemini's wide sweep, as a parameter, not a new path). Test: default == current output, finite for all categories.

### 4.5 HW28 Timing per category
1. Keys `draw_slide_ms_<cat>`, `stow_slide_ms_<cat>`, `reach_grab_pct_<cat>` (parse beside H:359/394), global as fallback, each clamped by `clampDrawSlideMs` / `clampStowSlideMs`; `cfgWarn` when a clamp bites (the silent 900 -> 570 clamp seen 10-20).
2. Defaults (draw/stow ms, grab %): pistol 300/350/30, lightsaber 350/350/30, melee 520/600/45, rifle 520/600/45, heavy 570/700/45, qstaff 570/700/45. Hip-mounted melee via per-model keys keeps 400/30.
3. Tests: config parse suite, resolution order per category.
4. HW28b (gated): `draw_slide_cap` ini (default 570) only if SM-L6 shows the first swing is not fought at 800 ms.

### 4.6 HW29 Swing/twist split
1. In `reachArm` after the forearm aim: twist about the elbow->hand axis (cap `reach_twist_deg` 80) written into the forearm quat; the wrist gets the swing remainder (cap `reach_wrist_max_deg`, default back to 60 once twist exists). Twist leaves the hand position unchanged. Test `reachSwingTwist`: recompose == input, hand position unchanged.
2. Optional, behind `reach_pole_from_grip=0`: elbow swivel from the target hand orientation at f <= 0.30 (claude.ai #8) as a bias only; must not break the HW23 hinge rules (elbow parked forward, kHingeMinOut).
Pass: SM-L1 `g - cap` < 20 deg for saber, pistol, back sword.

### 4.7 HW30 Shape data
1. Extend the ItemFitTable generator (inventory_3d_viewport_src) to emit a stem-keyed `holster_src/HolsterShape.inc`: `{stem, axis, len, centre, exitAxis, exitLen}`; `L::shapeOf(stem)`. Tests: lghtsbr 1.320 Z, blstrpstl 0.405 Y, dblsbr 2.464 Z.
2. Exit axis/length authored per class (bbox corners sit ~0.10 m off the blade axis, so they cannot give tip/exit points).
3. Doc 42 section 4: replace K1 sizes with K2 numbers (single 1.31-1.33, short 0.96-0.98, double 2.46, pistol 0.081 x 0.405 x 0.220).
4. Optional after SM-L7: `place_grip_w_<stem>` shifts PLACEMENT (never the reach target) along the model axis using `L:161 gripOffsetFromOrigin`.

### 4.8 HW31 Extraction + orientation schedule
1. reachPlan gains an extract share `reach_extract_pct` (default 20 back, 10 hip): during it T += exitAxis * exitLen * bump(q) (additive offset, hinge path unchanged), arm reach may clamp (rr<1) and that is accepted.
2. Orientation progress separate from pathF (R:401): hold the mount orientation through extract, then ease to rest; via-orientation waypoint (barrel/pole near vertical outside the shoulder) for rifle and qstaff, two-step slerp.
3. Tests: endpoints unchanged, continuity at each new phase joint, orientation held during extract.

### 4.9 HW32 Offline sweep check
`k1_port_tex_preview.py` / preview-3d: sample tip and butt points along the whole slide for each weapon x mount x hand against head sphere (`head_g`) and torso capsule proxies; report minimum clearance; use it to pick hinge out params and rotation direction. Test: one known-clipping case flagged.

### 4.10 HW33 Two hands
1. `L::isTwoHandedStem`: categories qstaff/rifle/heavy plus melee stems double sword, vibro double, force pike, gaffi, warblade. Test in `holster_logic_test.cpp`.
2. Dual stagger: `Slide::target` gains `startDelayMs` (t0 = now + delay) for the left Gob when the owner has a right Gob; `dual_stagger_ms` default 60.
3. Support reach (only if SM-L8 shows the left hand off the weapon): second `reachArm(left=true)` at H:1500 / H:1758 with a foregrip target (weapon axis offset from HW30), catch window `offhand_catch_pct` 25 at the end of draw, release at the start of stow; skipped when a left weapon Gob exists; a w=0 call at slide end hands the arm back.
4. Optional: `party_jitter_ms` (0-60) on non-leader party Gobs.
Items 1, 2 and 4 moved to HW33a (4.11); HW33 keeps item 3 only.

### 4.11 HW33a Two hands, safe parts
1. `L::isTwoHandedStem(stem)`: CatQStaff / CatRifle / CatHeavy, plus family prefixes `w_Dblsbr`, `w_Dblswrd`, `w_VbrDblswd`, `w_gaffi`, `w_warblade`, `w_ForcePike` (case-insensitive); `w_waraxe` stays one-handed (D_twohand_dual.md:15).
2. A raw `t0 = now + delay` is unsafe (`active()` false during the delay clears lockOn/cres, `touch` pushes t0, carryShownBase gets negative d). Instead `Slide::targetDelayed(hand, now, delayMs, drawMs, stowMs, holdW)` stores a pending target; `pump(now)` calls the real `target(..., pendAt, ...)` once `now >= pendAt` (wrap-safe); a newer `target()` or `reset()` cancels it; `delayMs == 0` is `target()` byte for byte.
3. Wiring: left Gob whose owner also has a right Gob (scan `s_cache`) uses `targetDelayed(dual_stagger_ms)`; `party_jitter_ms` (default 0) = deterministic per-owner delay on non-leader party Gobs. REACH/SLIDE log `dly=`.

### 4.12 HW35 Rest-pose MOUNT log
Once per (bone, stem) settle at rest and on change, capped at once per 5 s per Gob: `MOUNT gob= stem= bone= bq= mq= oq= rel= cat=`, `rel = conj(oq) * mq` (body-relative). `holster_log_check.py mounts`: per (cat, bone) median `rel` + count. Answers the back-mount GUESS (HW32 README) through SM-L13.

### 4.13 HW36 Exit overrides
`reach_exit_sign_<cat>` and `reach_exit_len_<cat>` replace `sh->exitSign` / `sh->exitLen` at the exit-offset site in `reachArm` when set; an override of +1/-1 applies even to exitSign-0 families (guns, sabers, staffs); 0 disables even melee; out of range clamps with a CONFIG warn.

### 4.14 HW31b Via on the residual path
`carryShownBase` uses the hand-frame residual for fresh draws and full stows, so B4's via (carryBlend only) is inert there. `reachResidualVia(rres, viaH, t, ms, draw)`: two segments `rres -> viaH -> I` on draw (reverse on stow), hemisphere-locked, continuous at the join; identity via = `reachResidual` byte for byte. `reachViaOffset`: lateral T offset outside the shoulder (`reach_via_out_m` 0.12) as a bump over the via phase, 0 at both ends.

## 5. Play checks (queue rows added when the batch installs; never block)

| Id | When | Do | Pass signal |
|---|---|---|---|
| SM-L0 | now (0.11 live) | draw/stow Vibrosword (upper back) and a heavy weapon, front/side screenshots | REACH shows `hg=1` for back weapons; arm does not cross the face |
| SM-L1 | B1 | `log_verbose=1`; draw then stow immediately (and reverse), 5x each | no visible jump; `SLIDE` pairs continuous; no `SLIDE cut` |
| SM-L2 | B1 | full fight with delayed stow (combat_exit, move_hold), re-engage mid-stow | no pop; zero `unreadable` lines in steady combat |
| SM-L3 | B2 | saber + pistol + back sword draws | `g - cap` < 20 deg; `resid` < 20 deg / 3 cm; no flip (`dq` stays > 0.05 or lock holds) |
| SM-L4 | B2 | editor: pistol on `torso_g` no offset, 3 draws per hand | `dzA` ~ -0.5, `hg=0`, elbow behind/out, hand not through the ribs |
| SM-L5 | B2 | meet a creature-weapon wielder | nothing placed on a back, no REACH for `w_cr*` |
| SM-L6 | B2 | back sword at 570 vs temporary 800 (HW28b build) | first swing not fought; SLIDE frames >= ms/20 |
| SM-L7 | B4 | draw `w_dblsbr` | hand on the shaft, no jump at hand-off (G7) |
| SM-L8 | B1 | rifle and staff draw, style byte log | left hand within 0.15 m of the weapon mid-slide => no support IK needed |
| SM-L9 | any | saber draw and stow at `draw_slide_ms=570`, `stow_slide_ms=1000`, screenshot mid-slide and holstered | blade state vs hilt position recorded |

## 6. Rejected or deferred (with reason)
- Raise the wrist cap to 135-160 (Gemini): does not remove the whip, worst seam. Interim 120, then HW29.
- Offset the reach target by the grip (Gemini): breaks the hand-off continuity; grip belongs in placement (HW30 optional).
- Low-back chicken-wing pole as a new path (Gemini): the existing hip pole is near-identical; LowBack topology reuses it.
- 30 ms grab plateau (Gemini): costs ~30% of the reach at defaults; easing gives the read without the cost.
- Runtime bbox at load (claude.ai): gives axis + length only; offline stem table instead.
- Thigh drop-and-piston path (Gemini): no live item mounts on a thigh. Deferred until one does.
- Cross-draw profile P2 (claude.ai): exists only if `chooseMount` is wired (open decision in doc 42 sec 7).
- Head tilt (claude.ai #10): cheap but optional; after HW31 if clipping remains.
- Hand-frame residual "attach switch" premise (claude.ai #11): in drawgate the attach never switches (native rhand); the residual is the carryW blend step, handled in HW26.

## 7. Open questions
1. Does the stance style flip inside the slide (and does the two-handed clip already place the left hand)? HW24 log + SM-L8.
2. Does the saber blade render mid-slide or on the hip? SM-L9.
3. How does the engine hold `w_dblsbr` (origin vs centre)? SM-L7 (G7).
4. torsoUpr_g / torso_g bone-local axes vs world up (affects the hinge gate with offsets): HW24 `dzA` answers it.
5. Wire `chooseMount` or drop it (doc 42 sec 7)? Owner decision; affects cross-draw only.
