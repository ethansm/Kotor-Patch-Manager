# C2 shape evidence: measured KOTOR II weapon geometry

Scope: static repo evidence only. No in-game run. Metres, model space.
Extent = max-min of composed vertex positions. Span = centre -/+ extent/2 (checked: w_lghtsbr span -0.200..1.119 matches wave2_3.md:748).
K2 = installed-model fit survey (ItemFitTable.inc, same numbers as R17). K1 = c3_compare.tsv.

| stem | long axis | K2 length (m) | origin span on long axis | origin vs grip | anchor |
|---|---|---|---|---|---|
| w_lghtsbr (single saber) | Z | 1.320 (22 var: 1.314-1.329) | z -0.200..+1.119 | origin = grip, median -0.009 (-0.07..+0.06) | ItemFitTable.inc:42; R17:39; wave2_3.md:748,752 |
| w_shortsbr | Z | 0.965 (10 var: 0.960-0.977) | z -0.126..+0.839 | origin = grip, median -0.010 | ItemFitTable.inc (shortsbr_001); R17:60; wave2_3.md:752 |
| w_dblsbr | Z | 2.464 (all 10 var) | z -1.026..+1.438 | origin 0.21 off grip centre (+0.207) | R17:50; wave2_3.md:750 |
| w_qtrstaff | Z | 1.026 (_001), 0.961 (_002), 0.522 (_003), 0.947 (_009), 0.722 (_010) | _002: z -0.328..+0.634 | UNMEASURED (rod, no grip node) | R17:2-6; ItemFitTable.inc:5-9 |
| w_stunbaton | Z | 0.428 (0.394-0.428) | z -0.063..+0.365 | UNMEASURED | R17:7 |
| w_lngswrd / w_vbroswrd | Z | 1.255 / 1.289 | z -0.211..+1.044 / -0.239..+1.050 | UNMEASURED | R17:12,17 |
| w_shortswrd / w_vbroshort | Z | 1.031 / 1.052 | z -0.140..+0.890 / -0.168..+0.884 | UNMEASURED | R17:23,27 |
| w_dblswrd / w_vbrdblswd | Z | 2.256 / 2.300 | z -0.952..+1.304 / -0.987..+1.313 | UNMEASURED | R17:31,35 |
| w_warblade / w_forcepike | Z | 2.256 / 2.117 | z -1.131..+1.125 / -0.787..+1.330 | UNMEASURED | R17:265,293 |
| w_waraxe / w_gaffi | Z | 1.029 / 1.025 | z -0.217..+0.812 / -0.205..+0.820 | UNMEASURED | R17:264,266 |
| w_blstrpstl (pistol) | Y (barrel +Y) | 0.405 (x 0.081, z 0.220) | y -0.069..+0.337, z -0.066..+0.154 | grip inferred hanging -Z at y -0.07..0.12, MEDIUM only | R17:70; wave2_3.md:760 |
| w_hvyblstr | Y | 0.462 | y -0.070..+0.392 | UNMEASURED | R17:85 |
| w_blstrrfl (rifle) | Y | 0.745 (x 0.061) | y -0.122..+0.623 | UNMEASURED | R17:258 |
| w_blstrcrbn | Y | 0.626 | y -0.047..+0.579 | UNMEASURED (grip -z asserted in doc 42 only) | R17:116 |
| w_bowcstr | Y | 0.839 (x 0.560 wide) | y -0.216..+0.623 | UNMEASURED | R17:113 |
| w_rptnblstr / w_hvrptbltr | Y | 0.723 (max var 0.828) / 0.746 (max var 0.846) | y -0.190..+0.533 / -0.109..+0.636 | UNMEASURED | R17:129,134 |
| w_ionrfl / w_sonicrfl / w_dsrptrfl | Y | 0.773 / 0.672 / 0.560 | y -0.272..+0.501 / -0.181..+0.491 / -0.067..+0.493 | UNMEASURED | R17:108,125,122 |

Conflicts with doc 42 (patch_manager_mods/42_holster_mounts_and_weapon_types.md):
- :94-95 single 1.30-1.54, short 1.01-1.07, double 2.42 (z to +1.39) are K1 values (c3_compare.tsv:39,51,13 = 1.3055, 1.0689, 2.4174). K2 is 1.31-1.33, 0.96-0.98, 2.46 (z to +1.44).
- :101 pistol 0.13 x 0.37 x 0.21 is K1 (c3_compare.tsv:5). K2 is 0.081 x 0.405 x 0.220. (42_holstered_weapons.md:67 pistol y -0.07..0.34 matches K2.)
- :99 rifle 0.07 x 0.70 x 0.26 vs K2 0.061 x 0.745 x 0.255.
- :98,:100 quarterstaff, repeaters marked "unmeasured": K2 numbers exist (rows above).

Hand attach evidence:
- Vanilla attach: body vtbl+0x7C(W, "rhand", flag 1, scale) at 0x763B4C; left "lhand" at 0x763E7E (.claude/research/holster_edit/r1_weapon_hand.md:7,21). No doc confirms the child origin lands on the node origin.
- rhand is a mesh-less dummy under Rhand_g, 0.092 m below it. Model-space rest (n_commm) 0.520,-0.032,0.940 (.claude/research/holster_reach/arm_bones.tsv:10).
- Derived, unverified: hook axes differ by quaternion convention. arm_bones header says (w,x,y,z): blade +Z = (0.31,0.92,0.25), forward-up. pykotor (x,y,z,w): blade +Z = (0.31,0.57,-0.76), forward-down.
- G7 (saber origin vs grip) is still OPEN: patch_manager_mods/42_holstered_weapons.md:59; wave2_3.md:743.
- Haiku visual reads (.claude/research/holstered_weapons/haiku_check/a.md, b.md) are offline renders that assume origin at rhand. Weak evidence.
