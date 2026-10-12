# P1 texture map: p_carthh (K1 NPC proof of concept)

Source: K1a/p_carthh.mdl, 26 mesh nodes. texture_1 = P_CarthH01 on 9 nodes, NULL on 17. texture_2 (lightmap) empty on all 26.
Vanilla size: K1 Installation.texture_resource_result("p_carthh01") -> swpc_tex_tpa.erf.

| texture name | vanilla K1 | upscaled match (pack / relative path) | upscaled size | file size (B) | factor |
|---|---|---|---|---|---|
| P_CarthH01 (p_carthh01) | 256x256 | mod149_character_textures_2x / Copy contents to TSL's Override folder / P_CarthH01.tpc | 512x512 | 349680 | 2.0x |
| P_CarthH01 (p_carthh01) | 256x256 | nexus_mod144_uco_redux / KOTORII - Ultimate Character Overhaul 3.0 - TPC / P_CarthH01.tpc | 1024x1024 | 1398265 | 4.0x |

Names with no pack match: NONE (0 of 1 names missing).
Packs searched: 6 folders under 01_extracted. Only 2 files matched p_carthh01.* (both .tpc, no .tga).
extra_hd_female_twileks, mod37_head_fixes, mod41_twilek_diversity, extra_fashion_line_b: no match.

VERDICT: 1 of 1 names covered; scale factors 2.0x (mod149) and 4.0x (nexus144 UCO); the two packs differ in scale, so only one should be installed per texture.
