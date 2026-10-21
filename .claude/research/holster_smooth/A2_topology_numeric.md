# A2 numeric: back-mount hinge gate (reachHingeFits)

Inputs: arm_bones.tsv (rest, model space), doc 42 section 1 (bone z, male), live holster.ini. Arithmetic by python, no live run.

## Gate inputs
- Arm chain (Holster.cpp:1259): pS=rbicep_g, pE=Rforearm_g, pH=rhand (hook). arm = |pE-pS| + |pH-pE| (Holster.cpp:1308).
- Male: 0.298 + 0.332 = 0.630 m. Female: 0.257 + 0.311 = 0.568 m.
- Gate dz >= -0.25 * arm: male -0.158 m, female -0.142 m (HolsterReach.h:239-240).
- mountPos = placed weapon origin, bone + R(bone.q) * place_off (HolsterQuat.h:47). Passed at Holster.cpp:1498 and 1756. Not the bone position.
- mlt = inv(body.q) * (mountPos - pS); dz = mlt[2] (Holster.cpp:1303-1306). Body = parent Gob +0xA4. If upright, dz = world z difference.
- Shoulder z: male rbicep_g 1.468, female 1.348.

## Table (male unless marked; identity bone rotation assumed)
| bone / category | bone z | dz bone-only | live off z | dz | dz/arm | gate | rotated off z needed |
|---|---|---|---|---|---|---|---|
| torsoUpr_g / rifle | 1.244 | -0.224 | +0.20 | -0.024 | -0.04 | PASS (margin 0.13) | >= +0.067 |
| torsoUpr_g / heavy | 1.244 | -0.224 | +0.25 | +0.026 | +0.04 | PASS (0.18) | >= +0.067 |
| torsoUpr_g / melee, Lngswrd_001 | 1.244 | -0.224 | +0.22 | -0.004 | -0.01 | PASS (0.15) | >= +0.067 |
| torsoUpr_g / no z offset | 1.244 | -0.224 | 0 | -0.224 | -0.36 | FAIL (-0.07) | n/a |
| torso_g / rifle | 1.139 | -0.329 | +0.20 | -0.129 | -0.21 | PASS (thin, 0.03) | >= +0.172 |
| torso_g / heavy | 1.139 | -0.329 | +0.25 | -0.079 | -0.13 | PASS (0.08) | >= +0.172 |
| torso_g / melee | 1.139 | -0.329 | +0.22 | -0.109 | -0.17 | PASS (0.05) | >= +0.172 |
| pelvis_g / lightsaber, pistol | 1.126 | -0.342 | 0 (x only) | -0.342 | -0.54 | not evaluated (backMount=false) | n/a |
| rthigh_g | 0.982 | -0.486 | 0 | -0.486 | -0.77 | not evaluated | n/a |
| torsoUpr_g / rifle, FEMALE est | ~1.178 | -0.170 | +0.20 | +0.030 | +0.05 | PASS (0.17) | >= -0.030 |

Female estimate: torsoUpr_g z = rootdummy F 1.06 + male offset (1.244 - 1.126). Not measured.

## What the gate does for back mounts
- Back mounts force u = 1 (Holster.cpp:1310). The gate only picks hinge path vs bowed line (Holster.cpp:1321).
- reachOverWeight ramps u 0->1 over dz/arm -0.75..-0.45 (HolsterReach.h:139-142). The -0.25 gate sits in the saturated zone.

## Notes
1. Offsets are bone-local (applyPlacement rotates by node.q). torsoUpr_g and torso_g world quaternions are NOT in arm_bones.tsv. Rotated offset z is only bounded by |off| (rifle 0.20, heavy 0.25, melee and Lngswrd_001 0.22). Table assumes identity.
2. Identity check: back_rifle.md:17 (rot 45,0,-90 maps +y to (0.71,0,0.71)) and back_dblsaber.md:9 (rot -60,0,30 maps +z to (-0.43,0.75,0.50)) match qbase exactly with identity bone. Suggestive only; may be analytic.
3. Sign conflict: live place_off_rifle z=+0.20 vs research best rifle z=-0.21 (back_rifle.md:17,24). With -0.21 the rifle dz is -0.43 and the gate fails. Live ini decides; confirm which is intended.
4. Overrides: place_off_w_Lngswrd_001 (0.12,-0.15,0.22) beats the category key. place_off_body_P_AttonBB z -0.018 gives Atton rifle dz -0.042, still PASS.
5. TSV is rest pose. Animated shoulder and elbow move dz by an unmeasured amount.
6. Frame check: TSV rootdummy z 1.13 is close to doc pelvis 1.126, so doc z values share the model frame. Female torso bones unmeasured.
