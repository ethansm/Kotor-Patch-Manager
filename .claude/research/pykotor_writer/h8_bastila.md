# h8: K1 Bastila Shan models and recipe facts (recon, read-only, 10-18)

## Verdict
Recipe `patch_manager_mods/k1_port/recipes/bastilla.toml` written; loader PASS, loads with carth/juhani/mission/zaalbar without k2-name collisions. Two K1 models have dangly nodes (body B, head), so `converter = "pykotor"` carries a comment: hair/front-flap/leg-flap sway needs `--keep-dangly`; the recipe has no key for it yet, so the oracle routes to MDLOps until then.

## Method
- Facts: `k1_port_scan.gather_facts/derive` via pykotor (k1port venv) on the K1 install: template `p_bastilla` from chitin:templates.bif; models from models.bif.
- Parse: stdlib `struct` (h6c offsets, same script as h7): node header 80 B, children +44/+48, pointers relative to byte 12, root ptr at file 52, supermodel at file 148, dangly = `type & 0x100`, skin = `type & 0x40`.
- Vanilla K2 comparison: lowercase dev extraction in the scratchpad `k2mdl/` (p_bastilabb, p_bastilah, s_female03 present; no p_bastilaba).

## K1 creature facts (p_bastilla)
| Field | Value |
|---|---|
| appearance.2da | row 4, Party_NPC_Bastila, modeltype B, modela P_BastilaBA, modelb P_BastilaBB |
| heads.2da | row 1 -> p_bastilah |
| soundset.2da | row 75 -> `p_bastila`, SSF exists in K1 (ssf_found true) |
| conversation | `k_hbas_dialog`, DLG exists in K1 (dlg_found true) |
| gender_id / tag / name | 1 / Bastila / Bastila |
| Derived K2 names | utc/tag k1p_bastilla, ssf k1p_bastillassf, dlg k1p_bastilladlg |

## K1 models
| K1 file | Own nodes | Chain total (file 56) | Node types (flag: count) | Dangly (0x0100) | Skin (0x0040) | Anims | Supermodel (file 148) |
|---|---|---|---|---|---|---|---|
| p_bastilaba | 64 | 576 | 0x0001: 17, 0x0021: 44, 0x0061: 3 | 0 | 3 | 0 | `P_BastilaBB` |
| p_bastilabb | 94 | 511 | 0x0001: 21, 0x0021: 65, 0x0061: 4, 0x0121: 4 | 4 | 4 | 0 | `S_Female03` |
| p_bastilah | 36 | 548 | 0x0001: 9, 0x0021: 23, 0x0061: 2, 0x0121: 2 | 2 | 2 | 5 | `P_BastilaBB` |

Dangly nodes (name, vertices):
| Model | Dangly nodes |
|---|---|
| p_bastilabb | Object06 57, Frntflap 51, RLegFlap01 26, LLegFlap 26 |
| p_bastilah | hairbangs 120, HAIR 362 |

Skin nodes (name, vertices):
| Model | Skin nodes |
|---|---|
| p_bastilaba | RArm 238, LArm 238, torso 802 |
| p_bastilabb | ArmL 268, ArmR 268, backflaps 112, torso 991 |
| p_bastilah | tongue 22, head 554 |

p_bastilah animations: pause1, talk, pause2, tlknorm, listen.

## Supermodels
| Name | K1 | Vanilla K2 | Notes |
|---|---|---|---|
| `P_BastilaBB` (header of ba and h) | yes (models.bif) | yes: K2 has its own p_bastilabb, 94 own nodes, same dangly set, chain total 620 (K1: 511) | same resref as K1 body B, differs only by chain numbering (K2 S_Female03 total 525 vs K1 416) |
| `S_Female03` (header of bb) | yes | yes (models.bif, 1666902 B) | resolves by name, no map needed |

Both ba and h reference `P_BastilaBB`, as expected. The importer applies `sm = [body_b.k1 = body_b.k2]` to body A and the head (`k1_port_import.py` ~1378), so the recipe needs no explicit supermodel key: body A and head get `k1p_bastilabb`. Vanilla K2 also has `p_bastilah` (head, 36 own nodes, total 657, same two dangly nodes) but no `p_bastilaba`; our `k1p_*` names do not collide with any of them.

## Textures (derive order: body A, body B, head)
| K1 | K2 | Mesh | txi |
|---|---|---|---|
| p_bastilaba01 | k1p_bastilaba01 | body A | none |
| p_bastilabb01 | k1p_bastilabb01 | body B | none |
| p_bastilah04 | k1p_bastilah04 | head and body B | clamp 1 |

All three exist in the K1 install; no missing textures. All `force_opaque = true`.

## Guessed values
None. The converter is `pykotor` as instructed; the dangly note is a comment only, no invented key.
