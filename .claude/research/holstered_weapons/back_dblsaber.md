# Back-mount double saber (w_dblsbr_001 + dblsbr_blue), recon R1

Method: `weapons` tool, REST pose, N_CommM + N_CommF, back and side shots. Convention checked: pos' = (Rz@Ry@Rx) pos + bone_t + offset. Back view confirms +x = character right.
Hilt runs +z from the weapon origin, spanning -0.065..+0.479 m (origin is 0.065 m above the lower tip).
Bone origins (model space): torsoUpr_g (0,-0.058,1.244); torso_g (0,-0.039,1.139); pelvis_g (0.004,-0.047,1.126).
Back surface y (min of mesh vertices in band): M torsoUpr -0.116, torso_g -0.138; F torsoUpr -0.117, torso_g -0.079.

## Trials (4 of 4 budget)
- T1 torsoUpr_g + torso_g | rot -60 0 30 | offset 0.15 -0.15 -0.3 | REFUTED. Local +z maps to (-0.43, 0.75, 0.50): top tip goes forward, y +0.15, through the chest. Shots: n_commm_backt1_diag.png, n_commm_sidet1_diag.png
- T2 torsoUpr_g + torso_g | rot 0 0 0 | offset 0 -0.15 -0.4 | PARTIAL. Straight spine line, but floats about 0.09 m behind the back (y -0.21 vs surface -0.12). Lower tip z 0.78, below belt (pelvis z 1.13): fails "not below waist". Shots: n_commm_backt2_vert.png, n_commm_sidet2_vert.png
- T3 torsoUpr_g | rot 0 42 0 | offset -0.106 -0.082 -0.216 | PASS on shape. Left hip to right shoulder, hugs back at y -0.14, clears head. Lower tip z 0.98 (about 0.15 m under pelvis z 1.126). Shots: n_commm_backt3_upr.png, n_commm_sidet3_upr.png, n_commf_backt3_upr.png, n_commf_sidet3_upr.png
- T4 torso_g | rot 0 42 0 | offset -0.106 -0.101 -0.111 | PASS, same look as T3 (abs origin matches). M shots only: n_commm_backt4_torso.png, n_commm_sidet4_torso.png

## Best pick: T3 (torsoUpr_g; upper torso keeps the hilt with shoulder twist)
- rot: 0 42 0 (deg)
- offset: -0.106 -0.082 -0.216 (m, torsoUpr_g frame)
- Hilt axis d = (0.669, 0, 0.743). Lower tip abs (-0.150, -0.140, 0.980); top tip abs (0.214, -0.140, 1.384). Length 0.54 m.
- Mirror (carry on the other side): rot 0 -42 0, offset +0.106 -0.082 -0.216. Predicted by symmetry, not rendered.

## Unrendered refinement (computed, not tested)
- torsoUpr_g, rot 0 47 0, offset -0.122 -0.082 -0.120: lower tip z 1.08 (belt level), top z 1.45 (shoulder), lower x -0.17 (hip edge). Needs one render to confirm.

## n_commf differences
- Same placement reads correctly. The top tip lands at the right shoulder edge; F torso is narrower (x half-width 0.26 vs 0.43 for M, arms excluded).
- F back is nearer: torso_g surface -0.079 and torsoUpr -0.117. In the F side shot the hilt sits a few cm off the back at y -0.14. Use y -0.12 for F.

## Caveats
- REST pose only (T-pose arms). In-game the arms hang at the sides and torsoUpr_g twists with animation, so the hilt will move. Not tested.
