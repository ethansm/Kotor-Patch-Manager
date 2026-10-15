# Heavy blaster (w_hvyblstr_001) back-mount recon

Rest pose only (T-pose arms, no hand-rot). Bone frame: x right, y forward, z up. R = Rz@Ry@Rx.
Renders: /tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/acdd1f4e-477a-4cae-858e-ad9b4a652b23/scratchpad/r1_heavy/s/ (each sheet: torsoUpr_g left, torso_g right).
Side view reading: left = character's back, right = front. Scale about 480 px/m (gun length 0.46 m). Body top 1.58 m (doc 42).
Trial cap: 4 of 4 used.

## Trials
1. T1 torsoUpr_g + torso_g, rot -60 0 20, offset 0 -0.15 0. PNGs: n_commm_back_t1_diag.png, n_commm_side_t1_diag.png. REJECT: barrel angles forward-down, reads as a front/hip carry.
2. T2 rot 0 0 0, offset 0 -0.18 -0.21. PNG: n_commm_side_t2_vert.png. REJECT: barrel horizontal at belt height, receiver floats about 0.2 m off the back.
3. T3 rot 90 0 0, offset 0 -0.17 -0.25. PNGs: n_commm_back_t3_up.png, n_commm_side_t3_up.png. PARTIAL: centred spine rod, muzzle z about 1.38, but grip bottom z about 0.92 (below belt).
4. T4 rot 110 0 -25, offset 0.03 -0.17 -0.22. PNGs: n_commm_back_t4_slung.png, n_commm_side_t4_slung.png, n_commf_back_t4_slung.png, n_commf_side_t4_slung.png. BEST: barrel up and back, muzzle centre-left over the spine, grip on back surface.
- torso_g renders about 0.06 m lower than torsoUpr_g (T3, T4 back views). Use torsoUpr_g.

## Best pick (torsoUpr_g, T4), N_CommM
- rot 110 0 -25 | offset 0.03 -0.17 -0.22 (m)
- Muzzle top z about 1.33 to 1.39: under body top 1.58, no head clip.
- Grip bottom z about 0.96 to 0.98: about 0.10 m below belt (belt z about 1.07). Too low.
- Receiver stands about 0.14 m off the back in side view (grip on the surface).

## Untested z correction (not rendered, trial cap reached)
- N_CommM: rot 110 0 -25 | offset 0.03 -0.17 -0.12. Expect grip at belt (z about 1.06), muzzle z about 1.49 (under 1.58).
- N_CommF: same rot | offset 0.03 -0.17 -0.06. Based on F grip sitting about 0.16 m below its own belt (M: 0.10 m).

## N_CommF differences (T4 renders)
- Narrower coat: receiver stands about 0.2 m off the back (M: 0.14 m) at the same y.
- Gun bottom sits lower against the belt (0.16 m vs 0.10 m), so F needs about +0.05 m more z than M.
- Barrel top stops at mid-neck, no clipping.
- Y choice: keep -0.17 (grip flush, receiver floats) or move to about -0.08 (receiver flush, grip sinks about 0.1 m into the coat). Not rendered.

## Caveats
- Measurements read by eye from 460 to 920 px sheets; treat as +-0.03 m.
- T-pose arms and rest pose only; a real idle or walk pose may shift the coat.
- T1 and T2 verdicts are from small images (back view partly unreadable).
