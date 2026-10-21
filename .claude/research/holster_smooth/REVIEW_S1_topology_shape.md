# REVIEW S1: mount topology + weapon shape (Sonnet adjudication)
Paths under patch_manager_mods/holster_src/ (H=Holster.cpp, R=HolsterReach.h, L=HolsterLogic.h). dz/arm = (mount z - shoulder z)/0.630 m (male). Bone-only: torsoUpr -0.36, torso_g -0.52, pelvis -0.54, thigh -0.77 (A2, A).

## 1. Verdicts
| id | src | claim | verdict | anchor | build impact |
|---|---|---|---|---|---|
| T1 | both | torso_g is lumbar (1.139 vs pelvis 1.126), yet gets back treatment | TRUE | H:1202 reachIsBackBone, H:1310 u=1; A1 | no live category uses torso_g (ini); editor-only today |
| T2 | Gemini | torso_g fails hinge and gets over-shoulder pole | TRUE (R6) | H:1321 gate, H:1329 reachPole after !hinge, R:127-134 `over`, R:239-240 | bone-only fails (-0.52); with a +0.20 z offset it PASSES (-0.21, margin 0.03) if bone-local z is world up (A2). Orientation unknown, so outcome is not predictable |
| T3 | Gemini | over pole twists bicep through ribcage | UNKNOWN | none (visual) | needs live look (play check P1) |
| T4 | Gemini | pelvis gets pure hip pole | FALSE | R:139-142 band -0.75..-0.45; live log cat=4 u=0.34-0.55 (this log), 0.34-0.85 (A) | pelvis is a 35-85% blend toward `over`; R:137 comment (hip ~0.8 below) is stale (real -0.54) |
| T5 | Claude | hip-height bow is zero, straight path cuts abdomen | FALSE at pelvis, TRUE at thigh | same as T4; thigh u=0 (A4) | H3 "CONFIRMED" is wrong for pelvis_g; right only for rthigh_g (nothing uses it) |
| T6 | Gemini | thigh needs drop-and-piston path | PARTIAL | A4: no thigh code, straight line, u=0 | gap real, but no live item mounts on a thigh: low priority |
| T7 | Claude | pelvis +0.20 x is a right-hip mount | TRUE (probable) | live ini:10-14 | mlt[0] sign unverified (H3b); log it |
| T8 | Claude | chooseMount unwired, so cross-draw (P2) does not exist | TRUE | L:135 no non-test caller (rg); H3c | cross-draw reachable only via negative-x ini offset; defer P2 |
| T9 | Claude | creature weapons sit on the upper back via melee key | TRUE in code (R5) | L:65 CatMelee; ini:22 place_bone_melee=torsoUpr_g; H:1609 classify(mname,0); H:1731-1749 resolveFull->boneIdx->mountPose | H1 error (see 5). Runtime needs entry to be a target node (nodeIsTarget) and no party filter exists (ownerIsParty only at H:1671): UNKNOWN whether claws ever reach it |
| T10 | Claude | CatUnknown should stay in hand | TRUE as gap | H:1605 unknown gets global place_rot/off, bone_name pelvis_g | Unknown stems are placed at the pelvis today |
| T11 | both | hinge gate input = placed weapon pos, not bone | TRUE (R7) | H:1498 (mountPos=out.pos), H:1756 (hipPose.pos), H:1303-1306 mlt=inv(body.q)*(mountPos-pS) | place_off/rot change the gate; tune by the placed point |
| T12 | Gemini | heavy on torsoUpr_g uses sagittal hinge, not its own style | TRUE | H:1321-1325 reachHinge has no style/cat arg; heavy +0.25 z passes (+0.04, A2) | reach_pole_*/reach_bulge_* do NOT act on hinged back mounts; the wide-sweep claim needs a per-cat hinge param |
| T13 | Gemini | X_clear 0.10 -> 0.12/0.25 per archetype | PARTIAL | R:kHingeOutT=0.10, kHingeOutPole=0.25, global constants | easy: make them per-category keys |
| T14 | Gemini | reach aims at mount bone, so dblsbr "teleports" 0.21 m | FALSE | H:1745 applyPlacement, H:1756 mountPos=hipPose.pos (placed origin); C5 | target already = weapon origin; no axial step from the target |
| T15 | Gemini | add O_grip to T_mount | FALSE (harmful) | same | would aim the hand 0.21 m off the origin the weapon is then pinned to (continuity break at hand-off); grip offset belongs in placement if wanted |
| T16 | both | origin = grip except dblsbr | PARTIAL | C2: saber median -0.009; dblsbr +0.207 (R17:50); G7 open | L:161 gripOffsetFromOrigin has no caller; unverified how engine holds dblsbr |
| T17 | Claude | extract main axis/length from bbox at load | PARTIAL | H8: Gob AABB offsets known (0x14c/0x158) but Gob-at-load, space, node inclusion UNKNOWN | use offline table |
| T18 | Claude | far-end/exit/support points from bbox | FALSE | H8: AABB corners sit ~0.10 m off the blade axis; no exit/grip in bbox | author per class |
| T19 | Gemini | rifle grip -Z, receiver above hand; heavy held low at hip | UNKNOWN | C2: grip direction not measurable from bbox; drawn pose unmeasured | live/render check |
| T20 | doc | doc 42 shape numbers stale | TRUE | C2: single 1.31-1.33, short 0.96-0.98, dbl 2.46, pistol 0.081x0.405x0.220 are K2; doc uses K1; qstaff/repeater data exists | refresh doc 42 sec 4 |
| T21 | Claude | thigh mount moves with leg (P3) | PARTIAL | placement re-reads mount bone each tick (H:1133-1168 mountPose) | per-frame tracking already exists; mount-rest low-pass not needed for placement |

## 2. Conflicts and resolutions
1. Bone-keyed (Gemini 4 topologies) vs geometry-keyed (Claude #1). Code already has a geometry continuum (reachOverWeight on mlt[2]) plus a bone-name override (back => u=1). Resolve: geometry from the PLACED point in the body frame; keep bone name only as editor label. Height alone cannot split lumbar from hip (torso_g -0.52 vs pelvis -0.54), so add depth (mlt[1] < 0 = behind) and side (mlt[0]) as inputs. Neither source says this.
2. torso_g: Gemini builds a low-back "chicken-wing" pole (0.70,-0.70,-0.20), Claude P4 says consider not offering it. Resolve: no new path. When back bone, hinge does not fit, and the point is behind and below -0.45 arm, use the existing hip pole (0.60,-0.40,-0.40, close to Gemini's vector) with geometry u; else keep u=1. Gate on the live check (section 4, P1).
3. Grip offset: Gemini offsets the reach target, Claude defines grip = origin. Claude is right for the target (T14/T15). Resolve: never offset T; if dblsbr should sit centred on the mount, shift the PLACEMENT along the model axis by L:161 after a G7 probe, with a per-model key.
4. Shape source: Claude bbox at load vs H8. Resolve: stem-keyed generated table (not row,var) from the R17 survey; per-model keys override. Gemini's per-archetype constants become table defaults.
5. Creature weapons: Claude (upper back) vs H1 (unsupported). Claude is right for routing (T9); H1 is right that nothing excludes them (also T10).

## 3. Build steps (in order)
1. H:1374 REACH log: add `dzA=%.2f (mlt[2]/arm) fy=%.2f (mlt[1]/arm) fx=%.2f` and `bone`. Test: holster_reach_test (format only). Reason: no topology tuning without readings; live log lacks `hg=` (0 of 36 REACH lines) so the installed DLL predates HW23.
2. HolsterLogic.h: `holsterable(cat, stem)` false for CatUnknown and stems w_cr*; H:~1609 skip placement + reach when false. Test: holster_logic_test rows for 3 w_cr* stems + CatUnknown. Check entry is a target node first (play check P4).
3. HolsterReach.h: new `reachTopology(dzArm, fy, fx, hingeFits)` returning {HingeBack, OverBack, LowBack, Hip, Thigh}; H:1309-1310 call it, replacing `if (backMount) u = 1`. u=1 only for HingeBack/OverBack; LowBack and Hip use hip pole. Tests: holster_reach_test vectors from A2 table (torsoUpr +0.22 -> HingeBack; torso_g 0 offset -> LowBack; pelvis -> Hip; thigh -> Thigh).
4. HolsterReach.h: recentre reachOverWeight band on measured -0.54 (e.g. -0.75..-0.30) and fix R:137 comment; test: u(-0.54)<0.2, u(-0.36)~1, monotone.
5. HolsterReach.h reachHinge: add per-category `outT`, `outPole` args (kHingeOutT/kHingeOutPole defaults); HolsterTune keys `reach_hinge_out_<cat>`. Defaults rifle 0.25, others unchanged. Test: reach_test finite for all cats, default == current output.
6. gen_inventory_3dview_table.py: emit `kShape[] {stem, axis, len, c, e}` (stem-keyed) into holster_src/HolsterShape.inc; HolsterLogic.h `shapeOf(stem)`. Test: C2 rows (lghtsbr 1.320 Z, blstrpstl 0.405 Y, dblsbr 2.464 Z).
7. Only after P2 passes: per-model grip shift in applyPlacement caller (H:1745), key `place_grip_w_<stem>`; test with L:161 values.
8. Update doc 42 sec 4 with K2 numbers (T20) and R:137 comment.

## 4. Live play checks
- P1 torso_g look: in the editor set one weapon (pistol) to torso_g with 0 offset, draw/stow 3x per hand. Pass: elbow stays behind/out, hand not through ribs, log `dzA` about -0.5 and `hg=0`. Fail: arm folds across chest.
- P2 dblsbr hold: draw w_dblsbr once, watch the hand-off. Pass: no visible jump at hand-off and hand on the shaft; log HAND/ATTACH pose shows weapon origin at rhand (G7).
- P3 pelvis pistol: draw 3x. Pass: hand path bows forward/out, elbow not up; compare to log u (0.34-0.55 now).
- P4 creature weapon: meet a w_cr* wielder (or force one); Pass: no weapon placed on a back, no REACH line for cat=2 with a w_cr model name.
- P5 hinge after install of the 0.11 DLL: REACH line shows `hg=1` for melee on torsoUpr_g; heavy/rifle path does not cross the face.

## 5. Haiku errors corrected
- H1 (R5): said upper-back routing "NOT SUPPORTED" by checking isLong(); isLong/chooseMount are not on the live path. Live routing is the per-category key place_bone_melee=torsoUpr_g (ini:22), so w_cr* ARE routed to the upper back by code. The "never reach" half of H1 stands.
- H3: "bow zero at hip" is wrong for pelvis_g (u 0.34-0.85 live, static 0.78). Zero only at thigh height. H3 repeated the stale R:137 comment.
- A caveat: "live hinge unknown" is now answered: current holster_log has 0 hg= fields, so the installed DLL predates HW23.
- A2 table assumes identity bone rotation; marks torso_g PASS only under that assumption. Treat PASS as unverified, not as a margin.
- C/C2: sound; C5 note that "origin = grip" is asserted for blasters in 42_holstered_weapons.md while doc 42 says grip -z for rifles (conflict stays open, T19).
- H8 correct; one add: ItemFitTable is keyed (row,var), stems only in comments, so a stem table needs the generator (step 6).
