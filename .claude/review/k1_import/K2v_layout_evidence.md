# K2 vanilla vs converter output: skin/mesh layout evidence

Sources (read-only): K2 vanilla = `Knights of the Old Republic II/data/Models.bif` via chitin.key (3237 MDL; loose `Override/` not used). K1 = `swkotor/data/*.bif` via chitin.key. Ours = `k1_port_stage/poc/pack/N_CommM.mdl/.mdx` (`vanilla/n_commm.*` copy: MDL identical, MDX differs in 246 bytes; all header/skin stats identical).
Method: stdlib raw parse, no pykotor. Trimesh header = node+80, K2 size 340 (K1 332). Skin header = trimesh+tm. Offsets MDL-relative. Scripts: `scratchpad/K2v/{kraw,corpus,agg,sample,final_checks}.py`.
Sample: 14 K2 models = 12 skinned (p_bastilah, p_hk47, p_kreiah, p_handmaidenh, n_commm, n_commf, n_commkidm, n_commkidf, n_twilekf, n_wookiem, n_darthmalak, n_sithassn) + 2 non-skin controls (n_admoff, n_darthrevanh). Corpus = all 3237 K2 MDLs.

## Check 1: skin MDX row order (skin hdr +12 = w_off, +16 = b_off)
| set | skin meshes | w_off < b_off | b_off < w_off | content verified (weights block / bone block) |
|---|---|---|---|---|
| K2 sample | 41 | 41 | 0 | 41 / 41 |
| K2 corpus (463 models) | 1414 | 1414 | 0 | 1414 / 1414 |
| K1 corpus (333 models) | 968 | 968 | 0 | 968 / 968 |
| OURS (2 copies, 3 skins each) | 6 | 0 | 6 | 6 / 6 |

- n_commm stride 64, base 32 (vert 12 + norm 12 + tex0 8). K2 and K1 vanilla: weights at row +32, bone indices at +48. Ours: bone indices +32, weights +48.
- Verified by content: weights = 4 floats in [0,1], sum 1.0. Bone indices = 4 integral floats, -1 = unused slot (K2 row 0: [0,1,-1,-1]).

## Check 2: skin unknown0 array (skin hdr +52 ptr, +56 count, +60 count2)
| set | count > 0 | count == count2 | count == bonemap count |
|---|---|---|---|
| K2 sample | 41/41 (34..111) | 41/41 | 41/41 |
| K2 corpus | 1414/1414 | 1414/1414 | 1414/1414 |
| K1 corpus | 968/968 | 968/968 | 968/968 |
| OURS | 0/6 (ptr 0, count 0) | 6/6 (both 0) | 0/6 (bonemap 59) |

- Payload is u32, not float. K2 n_commm first 8 = [29432920, 1, 2, 2, 3, 0, 4, 5]. K1 n_commm = [295781896, 3539320, 2, 2, 3, 0, 4, 1]. Meaning not established.

## Check 3: tangent block (bitmap 0x80, tangent offset T+288)
| set | flagged meshes | models | offset >= 0 | rows nonzero | flagged with all-zero rows |
|---|---|---|---|---|---|
| K2 corpus | 668 | 113 | 668 | 668 | 0 |
| K2 sample | 0 | 0 | - | - | - |
| K1 corpus | 4914 | 447 | 4914 | 4914 | 0 |
| OURS | 0 | - | - | - | - |

- K2 tangent rows are real: first vector length 1.000 over rows 0-19 (6 sampled meshes, e.g. 002ebo/e2_main1, 152har36/water).
- No K2 vanilla llk_LenaH-style case (flag set, zero rows) in 668 meshes.

## Check 4: UV-less mesh with texture name set (name at T+88 not NULL, tex0 offset T+272 = -1)
| set | meshes | models | skin meshes |
|---|---|---|---|
| K2 sample | n_commm 11, n_commkidm 11, n_sithassn 11, n_commkidf 4; other 10 models 0 | 4 | 0 |
| K2 corpus | 516 | 91 | 0 |
| K1 corpus | 451 | 101 | 0 |
| OURS | 11 (per copy) | 1 | 0 |

- Ours = the same 11 meshes as K2 n_commm: rcollar_g + 10 finger/thumb trimeshes, texture 'N_CommM04', tex0 offset -1.
- Name at T+120 set with tex1 offset -1: 0 in K2, K1 and ours.

## Check 5: supermodel (model header, MDL-rel +0x88, 32 B)
| model | supermodel |
|---|---|
| n_commm (K2) | S_Female02 (= K1 n_commm = ours) |
| n_commf / n_twilekf / n_commkidm | S_Female03 / S_Female03 / S_Male02 |
| n_wookiem / n_sithassn | S_Female02 / S_Female02 |
| p_hk47 / n_darthmalak / p_bastilah | NULL / NULL / P_BastilaBB |
| K2 corpus, 463 skinned models | S_Female02 193, S_Female03 127, NULL 73, S_Male02 10 |

## Extra (outside the 5 checks)
| set | node names | bonemap / unk0 count |
|---|---|---|
| K2 n_commm | 60 (extra 'freelookhook'; in only 20 of 463 K2 skinned models) | 60 |
| K1 n_commm / OURS | 59 (same set) | 59 |

## Risk reduction
1. Skin row order (ours bones-first): NOVEL. 0 of 1414 K2 skins are bones-first. Converter: write weights at +32, bone indices at +48 (w_off=32, b_off=48).
2. unknown0 count 0 (ours): NOVEL. 1414/1414 K2 skins populate it with count = bonemap count. Converter: emit count = count2 = bonemap count, valid ptr, u32 entries (match vanilla shape first).
3. 11 UV-less textured meshes: KNOWN-TOLERATED. K2 n_commm has the identical 11, and 516 K2 corpus meshes share the trait. No change.
4. Tangent flag: N/A for N_CommM (0 flagged). A flag with zero rows would be NOVEL (0 K2 precedent). Converter: emit real rows or clear bit 0x80.
5. Supermodel S_Female02 and absent 'freelookhook': KNOWN-TOLERATED (S_Female02 is K2 n_commm's own value; 443/463 K2 skinned models lack the node). UNKNOWN: our 59-entry bonemap/unk0 vs K2 n_commm 60. Converter: consider 60 if the K2 skeleton is kept.
