# T6j: llk_LenaH and llk_LenaB ingest

Pack: ~/Downloads/K1 Twi'lek Female NPC Diversity.7z (Leilukin; source leilukin_k1_twilek_f_2179, credit_required). Listed and read with py7zr 1.1.3 (k1port venv). Nothing run from the archive.

## 1. Referenced textures (pykotor read_mdl; non-NULL, non-empty)
- llk_LenaH: llk_LenaH (texture_1 on 9 meshes)
- llk_LenaB: llk_LenaB01 (4 meshes), llk_LenaH (6 meshes)
- texture_2 is empty on every mesh.

Extracted into twilek_f/KotOR 1 Twi'lek Female NPC Diversity/tslpatchdata/ (archive layout; CRC32 matches archive):
- llk_LenaB.mdl (216004 B), llk_LenaB.mdx (179656 B)
- llk_LenaB01.tpc (349708 B), llk_LenaH.tpc (349680 B)
- llk_LenaH.mdl and .mdx were already present and match archive CRC32.
Not extracted: Optional - DP's Twilek Thigh-High Boots/llk_LenaB.mdl/.mdx (alt boots variant), Optional - Upscaled Textures/*.tpc, the rest of the pack.

## 2. Ingest (stage default, one run per model, no --also-hd)
| model -> new | texture map | match | K1 mdl/mdx | K2 mdl/mdx |
|---|---|---|---|---|
| llk_LenaH -> k1p_lenah | llk_LenaH=k1p_llk_lenah | true | 97130 / 99184 | 97338 / 68128 |
| llk_LenaB -> k1p_lenab | llk_LenaB01=k1p_llk_lenab01, llk_LenaH=k1p_llk_lenah (already_ingested) | true | 216004 / 179656 | 215524 / 142016 |

- Sources: leilukin_k1_twilek_f_2179 was the only candidate for each file (vanilla_k1 had none). Not flagged; requires_author_approval false.
- Credits: Leilukin, SpaceAlex, DarthParametric (optional boots).
- Textures: k1p_llk_lenah.tpc and k1p_llk_lenab01.tpc are both 512x512 DXT5.
- Match flags, both models: model_name, node_tree, animation_count, vertex_face_counts, mesh_extents, uv/normals/faces checksums, textures, dirt_defaults, skin_bone_poses, tangent_data all true. failed_flags empty.
- Tangents: tangent_zero_cleared 9 (LenaH), 10 (LenaB). synth_zero_uv and keep_zero_tangents false.
- Counts: LenaH 26 meshes (3 skin), 1450 verts, 1566 faces. LenaB 69 meshes (4 skin), 3750 verts, 2474 faces.

## 3. Problems (reported, not fixed; tools not edited)
1. LenaB: supermodel S_Female03 (install) unreadable ("would exceed the streams boundaries"). LenaH's N_TwilekF read fine. Match is still true. Cause not investigated.
2. Missing textures: none. Every referenced texture was found.
3. Upscaled trap: the pack ships Optional - Upscaled Textures for both textures, in the same source, so --also-hd does not apply. By code reading (pack_candidates), one file per source is taken by shallowest path, then lexical order. "Optional" sorts before "tslpatchdata", so extracting the upscaled folder here would make it win. Not tested.
4. py7zr: a second extract call in one process raised CrcError on "READ ME.txt" (expected 2005627011). The extracted model and texture files all pass CRC32. Cause unknown; READ ME.txt was not used.
5. Stage ledger is shared. 4 rows come from an earlier k1p_commm run, including the flagged _hd texture (upscale_plus_v0.52, pending_author_approval). gate was not run (per brief).

## 4. Ledger (k1_port_prune.py list; basenames)
| file | kind | source_id | class | flagged | sha256[:12] |
|---|---|---|---|---|---|
| k1p_commm04.tpc | texture | vanilla_k1 | vanilla | no | 11b67dbfb954 |
| k1p_commm04.tpc (_hd) | texture | upscale_plus_v0.52 | permission_required | yes | 70d3de2e3bc9 |
| k1p_commm.mdl | model | vanilla_k1 | vanilla | no | b7cf35e00b27 |
| k1p_commm.mdx | model | vanilla_k1 | vanilla | no | 84acc62d43dc |
| k1p_llk_lenah.tpc | texture | leilukin_k1_twilek_f_2179 | credit_required | no | c8c67b90aa35 |
| k1p_lenah.mdl | model | leilukin_k1_twilek_f_2179 | credit_required | no | 6ce241704014 |
| k1p_lenah.mdx | model | leilukin_k1_twilek_f_2179 | credit_required | no | baa3c2f4c5e6 |
| k1p_llk_lenab01.tpc | texture | leilukin_k1_twilek_f_2179 | credit_required | no | 4bd0509c54ab |
| k1p_lenab.mdl | model | leilukin_k1_twilek_f_2179 | credit_required | no | 0fcec64b54a1 |
| k1p_lenab.mdx | model | leilukin_k1_twilek_f_2179 | credit_required | no | 595f3d6a2ec6 |

Credits (prune credits): BioWare 3; DarthParametric (optional boots) 6; Leilukin 6; RedRob41 1; SpaceAlex 6.
