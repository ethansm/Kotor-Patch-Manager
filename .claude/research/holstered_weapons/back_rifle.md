# Back-mount rifle placement: w_blstrcrbn_001 (recon R1)

Offline preview, REST pose, N_CommM + N_CommF. Bone frame: x = character right, y = forward, z = up; R = Rz@Ry@Rx.
Back shot: screen-left = character's left (`patch_manager_mods/k1_port/k1_port_tex_preview.py:393`).
The mesh export has no head, so head clearance is inferred from muzzle height, not seen.
Heights are estimated from image scale (about 290 px/m on M), not measured.
Geometry (HW3 notes): grip at origin, barrel along +y from -0.05 to +0.58 m. Bone torsoUpr_g z = 1.244 (M).
Screenshots: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/acdd1f4e-477a-4cae-858e-ad9b4a652b23/scratchpad/r1_rifle/s/`

## Trials (each run renders torsoUpr_g and torso_g)
1. `--rot -60 0 20 --offset 0 -0.15 0`, normal size. Barrel dir (-0.17, 0.47, -0.87): points down-forward. REFUTED. Sits at the belt and reaches the front-lower side.
   PNG: `s/n_commm_backt1_diag.png`
2. `--rot 90 0 0 --offset 0 -0.15 -0.25`, big. Barrel straight up. PARTIAL. Reads as carried on the spine. Stands about 0.2 m off the back (side view). M muzzle at the collar (z about 1.58). torso_g: muzzle under the collar, stock at the belt.
   PNG: `s/n_commm_backt2_vert.png`; `s/n_commm_sidet2_vert.png`
3. `--rot 90 45 0 --offset -0.20 -0.14 -0.24` on torsoUpr_g. Barrel dir (0.71, 0, 0.71): diagonal across the back, correct. PARTIAL. Receiver faces outward, so it stands about 0.25 m off the back.
   PNG: `s/n_commm_backt3_diag45.png`; `s/n_commm_sidet3_diag45.png`
4. `--rot 45 0 -90 --offset -0.20 -0.13 -0.21` on torsoUpr_g. Barrel dir (0.71, 0, 0.71). BEST. Rifle lies flat on the back with its width facing out. Stock low on the character's left at the belt. Muzzle over the character's right shoulder blade, below the collar on M. Side standoff about 0.1 m.
   PNG: `s/n_commm_backt4_flat.png`; `s/n_commm_sidet4_flat.png`
   Same numbers on torso_g: about 0.08 m lower on M, grip about 0.2 m under the belt. REJECT.

Not run: the suggested `--rot 0 0 0 --offset 0 -0.15 -0.21` strap. With identity rotation the barrel runs along +y, so it passes through the torso to the front (y -0.15 to +0.43). Expected from the formula, untested.

## Best pick (N_CommM, torsoUpr_g)
`--bones torsoUpr_g --rot 45 0 -90 --offset -0.20 -0.13 -0.21`
Absolute, estimated:
- Grip origin about (-0.20, -0.13, 1.03)
- Stock end about (-0.24, -0.13, 1.00)
- Muzzle about (+0.21, -0.13, 1.44)
- Lowest point (grip tip) about z 0.98, about 0.14 m under the belt line (belt about 1.12).

## N_CommF differences
- Same numbers on torsoUpr_g: muzzle reaches the collar line (about 1.45), grip about 0.15 m under the belt.
- Cause: the belt-to-collar band (about 0.38 m on F) is shorter than the rifle's vertical span at 45 degrees (about 0.45 m plus grip).
- torso_g on F: grip about 0.2 m under the belt. REJECT.

## Open / next (untested, one trial)
- Flatter tilt `--rot 30 0 -90` shortens the vertical span to about 0.32 m, so the grip should clear the belt on both bodies. Cost: the stock end sticks out about 0.1 to 0.15 m past the body's side at the hip. Origin needs x about -0.27.
- Head clearance is unverified. Check in-game (P0 probe) or with a head-included mesh.
