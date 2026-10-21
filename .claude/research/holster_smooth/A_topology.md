# A_topology: holster arm-reach claims (Gemini review A1-A5)

Snapshot: holster 0.11.0-dev source, live `holster.ini` (game dir), live `holster_log.txt` (mtime 2026-10-21 01:00). Recon only.

Shared numbers (from `.claude/research/holster_reach/arm_bones.tsv`, N_CommM male): shoulder `rbicep_g` z 1.468, elbow `Rforearm_g` z 1.224, hand `rhand` z 0.940.
Reach arm = |elbow - shoulder| + |hand - elbow| = 0.298 + 0.332 = 0.630 m. Ratio used below: dz/arm = (bone z - 1.468) / 0.630.

| Claim | Verdict | Anchor | What the code does |
|---|---|---|---|
| A1 bone heights; torso_g = lumbar | PARTIAL | `.claude/research/holstered_weapons/back_dblsaber.md:5`; doc `patch_manager_mods/42_holster_mounts_and_weapon_types.md:14-16` | Numbers match the bone origins in back_dblsaber.md:5. `arm_bones.tsv` has no torso or pelvis rows (arm bones only), so it does not verify them. torso_g sits 13 mm above pelvis_g: waist level, which fits "lumbar" by position. Code has no anatomy label. |
| A2 both torso bones are back mounts | TRUE | `patch_manager_mods/holster_src/Holster.cpp:1202` | `reachIsBackBone` is true for `torsoUpr_g` and `torso_g` (case-insensitive). Used at Holster.cpp:1756 (live path) and :1498 (editor preview). |
| A3 hinge fails: fallback uses over-shoulder pole | TRUE | Holster.cpp:1310, :1329; HolsterReach.h:240, :127-134, :199-205 | Hinge fails when dz < -0.25 arm (HolsterReach.h:240). Any back bone forces `u = 1` (Holster.cpp:1310), so `reachPole` returns `st.over`, mirrored for left and rotated by body. Bow is full: `k = arm * sin(pi f)` (HolsterReach.h:202). torso_g: dz/arm = -0.52, fails, gets the over pole. |
| A4 hip / thigh pole, path, height dependence | PARTIAL | Holster.cpp:1309, :1318, :1321; HolsterReach.h:139-142, :199-205 | Pelvis is NOT pure hip: dz/arm = -0.54, so u = easeSmooth(0.69) = 0.77 (pole leans over the shoulder). Thigh (`rthigh_g` z 0.982): dz/arm = -0.77, u clamps to 0, so pure hip pole and zero bulge: straight mount-to-hand line. No thigh-specific code (no "thigh" in Holster.cpp or HolsterReach.h). Height dependence besides hinge: u sets pole blend and bulge size. Hinge is back-only (Holster.cpp:1321). |
| A5 any category on torso_g or thigh today | TRUE (none is) | live `holster.ini:10,16,19,22,29` | torsoUpr_g: melee (cat 2, line 22), rifle (cat 5, line 16), heavy (cat 6, line 19), model `w_Lngswrd_001` (line 29). pelvis_g via `bone_name` (line 10): lightsaber, qstaff, pistol, unknown. |

## Pole defaults (body frame, right arm; x right, y forward, z up)
- Hip: lightsaber/melee/qstaff {0.60,-0.40,-0.40}; pistol {0.50,-0.70,-0.20}; rifle/heavy/unknown {0.60,-0.50,-0.30}.
- Over: lightsaber/melee {0.50,0.00,0.85}; qstaff {0.60,-0.20,0.70}; pistol {0.50,0.00,0.80}; rifle/heavy/unknown {0.70,-0.10,0.65}.
- Bulge (up,fwd,out): {0.20,0.40,0.10}; qstaff {0.40,0.20,0.20}; pistol {0.20,0.40,0.10}; rifle/heavy/unknown {0.60,0.10,0.25}.
- A back bone never uses the hip vector.

## Live log check (`holster_log.txt`, counts only via `rg -o`)
- 44 REACH lines. `cat=2 u=1.00` x38: melee on torsoUpr_g, forced to 1 as coded.
- `cat=4` (pistol, on pelvis_g since no `place_bone_pistol`) x6: u = 0.34, 0.52, 0.53, 0.54, 0.58, 0.85. Confirms the pelvis sits in the blend band, not u = 0.
- Pelvis static ratio 0.77 vs observed 0.34-0.85: the arm animates the shoulder-to-mount distance.
- The HolsterReach.h:137 comment assumes a hip mount sits about 0.8 arm lengths below the shoulder. Actual pelvis is about 0.54.

## Open / caveats
- Live hinge state unknown: REACH lines have no `hg=` field (written at Holster.cpp:1374 in this source), so the live DLL may predate HW23 (0.11.0-dev).
- Upper-back hinge fit depends on the mount point, not the bone origin. Bone origin gives dz/arm = -0.36 (fails). Live z offsets (+0.20 to +0.25, bone-local, `holster.ini:16,19,22`) give about -0.04 to +0.04 (fits) only if torsoUpr_g's local z is world up. Orientation is not in the repo.
- Live ini omits `reach_carry` and `reach_hinge`; code defaults both to 1 (Holster.cpp:83, :86). Hinge runs only in carry mode (Holster.cpp:1312).

## Gaps to build (only if per-topology paths are wanted)
- Lumbar under-reach: give torso_g its own style row in `reachDefaultStyle` (HolsterReach.h:127) or a torso_g-only `kHingeMinDz` near -0.55 arm; drop the blanket `u = 1` at Holster.cpp:1310 for torso_g.
- Thigh drop-and-pull: add a branch in `reachCarryTarget` (HolsterReach.h:199) for dz/arm < -0.75: drop to the mount, then up and in to the hand. Today u = 0 gives a straight line.
- Hip lateral drop: add an outward leg to the pelvis path; recentre the `reachOverWeight` band (HolsterReach.h:139-142) on the measured -0.54 so pelvis pistol and lightsaber get the hip pole.
- Per-bone bands: key the height weight on the bone name instead of one global band, so pelvis and thigh stop sharing the same ramp.
- Log first: add dz/arm and the hinge flag to the REACH line (Holster.cpp:1374), run one live draw, and read the topology from the log before tuning.
