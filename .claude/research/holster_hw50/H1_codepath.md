# H1 code-path inventory: heavy category (HW50)

Scope: `patch_manager_mods/holster_src`, read-only. Heavy = `CatHeavy` (HolsterLogic.h:14), stems `w_RptnBlstr` / `w_HvRptBltr` (HolsterLogic.h:70).
Template `holster.ini` ships `probe=0` (:4), `reach_test=0` (:143), `place=0` (:49). Live values are not in the repo. The brief in `.claude/research/holster_hw44/cat_heavy.md` assumes `place_bone_heavy=torsoUpr_g` and `reach_via_heavy=1`.

## Inventory (heavy vs other categories)

| term | file:line | heavy | other cats | effect on motion | ini key (range) |
|---|---|---|---|---|---|
| slide ms, built-in | HolsterTune.h:721 | draw 570 / stow 700 | rifle, melee 520/600; qstaff 570/700; pistol 300/350; lightsaber, unknown 350/350 | length of lift and of stow | draw_slide_ms_heavy (0..570), stow_slide_ms_heavy (0..1000) |
| ms set-time caps | HolsterSlide.h:220-230 | clamp 570 / 1000 | same | caps the ini value before slow scaling | same keys |
| slow multiplier | HolsterSlide.h:236; Holster.cpp:2558 | ms x slow_holster_x | same | stretches heavy slide | slow_holster_x (1..8) |
| grab share | HolsterTune.h:721; Holster.cpp:2543, 2558 | 45 % | melee, rifle, qstaff 45; pistol, lightsaber 30 | share of slide spent reaching mount before lift | reach_grab_pct_heavy (0..60, HolsterTune.h:682) |
| slide blend ease | HolsterSlide.h:216-218; Holster.cpp:2318, 2656 | easeSmooth | identical | no weight-specific ease exists | none |
| carry path ease | HolsterReach.h:384-392 (reachCarryF), 408-450 (reachPlan) | L = 0.15 | identical | cubic start and stop of carry path | none (constant) |
| hinge stage ease | HolsterReach.h:264-266 | half linear, half smooth | identical | soft stage joints | none (constant) |
| extract share | Holster.cpp:108, 1556-1562; HolsterReach.h:399-404 | back 0.20 (HingeBack/OverBack topo) | hip 0.10 (all others) | holds mount orientation for first share of carry, even with no lift | reach_extract_pct_back / _hip (0..0.5, Holster.cpp:463-468) |
| extract lift (positional) | HolsterShape.inc:63-65, 101-104; Holster.cpp:1801-1803 | exitSign 0 = no lift; exitLen 0.45 inert | rifle exitLen 0.25; pistol 0.12; dblswrd/gaffi sign -1 | no lift for heavy unless overridden | reach_exit_sign_heavy (-1, 0, 1, auto), reach_exit_len_heavy (0..0.5, auto) (Holster.cpp:476-492) |
| topology from mount | HolsterReach.h:280, 292-300; Holster.cpp:1722-1724 | dz >= -0.25 arm HingeBack; -0.45..-0.25 OverBack; -0.66..-0.45 Hip; below -0.66 Thigh | same rule | chooses hinge vs bulge vs hip pole | place_bone_heavy (5 bones, HolsterTune.h:62) |
| hinge outward hand offset (outT) | HolsterReach.h:259; Holster.cpp:1521-1523, 1737 | 0.10 | 0.10 all | outward hand offset at top of pitch | reach_hinge_out_heavy=outT,outPole (outT 0..1, Holster.cpp:602-616) |
| hinge outward pole share (outPole) | HolsterReach.h:260; Holster.cpp:1523 | 0.45 | rifle 0.25 (kHingeOutPoleRifle), others 0.25 | wider outward elbow share, HingeBack only | reach_hinge_out_heavy (outPole 0..2) |
| bulge | HolsterReach.h:134 (default branch), 237-245; Holster.cpp:1734 | {0.60, 0.10, 0.25} | rifle identical; sword/pistol {0.20, 0.40, 0.10}; qstaff {0.40, 0.20, 0.20} | bows path up/out; overwritten when hinge runs (Holster.cpp:1738-1740) | reach_bulge_heavy=up,fwd,out (-2..2, Holster.cpp:561-575) |
| hip / over elbow pole | HolsterReach.h:134; Holster.cpp:1727, 1760 | hip {0.60,-0.50,-0.30}; over {0.70,-0.10,0.65} | rifle identical | elbow direction; skipped when hinge runs | reach_pole_hip_heavy, reach_pole_over_heavy (x,y,z non-zero, Holster.cpp:583-594) |
| via on | Holster.cpp:109, 551, 1564-1572 | 1 in template (holster.ini:188); code default 0 | melee 1, rifle 1 (holster.ini:186-187); others 0 | turn routed through long-axis-up | reach_via_heavy (0 or 1) |
| via outward offset | HolsterReach.h:662, 665-672; Holster.cpp:114, 117, 1752-1754 | global 0.12 (heavy unset) | rifle 0.20 (holster.ini:193) | sideways bump peaks when weapon passes up | reach_via_out_m_heavy (0..0.30, Holster.cpp:523-533); reach_via_out_m (0..0.30, Holster.cpp:535-542) |
| wrist swing cap | Holster.cpp:104, 1831-1870; holster.ini:161-162 | global 60 deg, pct 100 | same | limits wrist swing | reach_wrist_max_deg (0..180), reach_wrist_pct (0..100) |
| forearm twist cap | Holster.cpp:104, 554-560 | global 80 deg (ini line commented) | same | limits forearm roll | reach_twist_deg (0..120) |
| dual stagger | Holster.cpp:132, 2404-2408; holster.ini:176-178 | global 60 ms, no category test | same | left-hand start delay | dual_stagger_ms (0..200) |
| residual, stow land, stow rot | Holster.cpp:120-135 | global 240 ms, land 0.15, rot 0.6 | same | hand-frame unwind; stow landing and turn | reach_resid_ms (0..600), reach_stow_land_f, reach_stow_rot_f |
| two-handed flags | HolsterLogic.h:95, 107, 139, 166 | isTwoHandedStem, isLong: test callers only; preferredZone, chooseMount: no caller | n/a | no support-hand logic, no zone choice in runtime code | none |
| mount offset | HolsterLogic.h:179 (placementFor returns zero, no caller); HolsterTune.h:11-16 | none built in | per category | rest pose on the mount | place_rot_heavy, place_off_heavy (commented, holster.ini:69-70) |
| holsterable | HolsterLogic.h:124-127 | yes unless w_cr prefix | same | gate only | none |
| unarmed walk clip | Holster.cpp:83-86, 891-900 | no heavy branch (comment only) | same | walk clip while holstered | unarmed_anim (holster.ini:138) |

## Ini-tunable, heavy-keyed
- draw_slide_ms_heavy 0..570; stow_slide_ms_heavy 0..1000; reach_grab_pct_heavy 0..60
- reach_hinge_out_heavy outT 0..1, outPole 0..2
- reach_bulge_heavy each axis -2..2; reach_pole_hip_heavy / reach_pole_over_heavy non-zero x,y,z
- reach_via_heavy 0 or 1; reach_via_out_m_heavy 0..0.30
- reach_exit_sign_heavy -1, 0, 1 or auto; reach_exit_len_heavy 0..0.5 or auto
- place_bone_heavy (5 bones); place_rot_heavy; place_off_heavy

## Ini-tunable, global (heavy inherits)
reach_extract_pct_back / _hip 0..0.5; reach_wrist_pct; reach_wrist_max_deg; reach_twist_deg 0..120; dual_stagger_ms 0..200; reach_resid_ms 0..600; reach_stow_land_f; reach_stow_rot_f; slow_holster_x 1..8; reach_hinge.

## Not tunable (code constants)
kCarryEaseLen 0.15 (HolsterReach.h:384); kHingeS3 0.30 and kHingeS2 0.80 (HolsterReach.h:257); kHingeMinOut 0.35; via bump 16 s^2 (1-s)^2 (HolsterReach.h:668-672); all ease curves.

## Findings
- Heavy-only values are few: outPole 0.45 (HolsterReach.h:260) and an inert exitLen 0.45. Draw/stow timing equals qstaff, grab equals rifle/melee, bulge and pole equal rifle.
- Plan hypothesis (c) names outT. outT is 0.10 for every category; the heavy-only hinge value is outPole.
- Bulge does nothing on the HingeBack path, because the hinge overwrites T (Holster.cpp:1738-1740).
- Two-handed flags have no runtime caller. No support-hand logic exists in code.

## Open
- Live topology for heavy (`topo=` in the REACH/MOUNT log) decides whether hinge keys (outT, outPole) or bulge/pole keys act. The live holster.ini is not in the repo.
