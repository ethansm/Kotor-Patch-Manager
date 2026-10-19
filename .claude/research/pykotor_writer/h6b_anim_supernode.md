# h6b: animation-node supernode vs geometry supernode, and offset-56 total (recon, read-only)

Scope: vanilla K2 MDLs in `scratchpad/k2mdl` (3237 files, 1155 with animations) plus 6 MDLOps outputs. Raw bytes, stdlib `struct` only, run with `python3 -I`. Nothing in the repo was changed.

Scripts (scratch, not committed): `scratchpad/h6b/h6b_full.py` (corpus pass), `h6b_table.py` (per-model rows), `h6b_rule.py` (supernode rule test), `h6b_seq.py` (block parse helper). Probe v1 `h6b_probe.py` and v2 `h6b_v2.py` were superseded because their block scan produced false positives.

## 1. Verdict

PARTIAL. The claim "anim-node supernode == geometry supernode of the node with the same index" holds for 178,120 of 178,513 anim-tree nodes (99.8%). The 165 exceptions are all duplicate-name nodes: the anim header carries the supernode of the first geometry node with the same name. A further 228 anim nodes have an index with no geometry node.

The offset-56 total does NOT include anim nodes (CONFIRMED on 2835/2835 NULL-supermodel models and 400/400 supermodel-linked models).

## 2. Layout (verified on the files, not only from pykotor)

Stored pointers are relative to file byte 12: absolute file offset = stored value + 12. Checked on S_Female03 (anim array, anim headers, node and children pointers all resolve to valid headers). Across the corpus, the anim root is node 0 in 7392 of 7393 anims.

Model header at file 12 (196 B):

| file offset | field |
|---|---|
| 12 | geometry header (80 B): +0 layout tokens, +8 name[32], +40 root ptr (file 52), +44 node count (file 56) |
| 92 (hdr+80) | model_type, subclass, pad, fog |
| 100 (hdr+88) | offset_to_animations ptr (abs = value+12) |
| 104 (hdr+92) | animation_count (animation_count2 at file 108) |
| 148 (hdr+136) | supermodel name [32] |
| 180 (hdr+168) | offset_to_super_root ptr |
| 196 (hdr+184) | name-table ptr (abs = value+12) |
| 200 (hdr+188) | name count |

Note on the task brief: the animation array pointer lives at file 100 (model header +0x58), not at file 100+12. The count is at file 104 (model header +0x5C). The stored pointer is relative to file 12.

Animation header (136 B, at abs = array entry + 12): geometry header (80 B, root ptr at +40, node count at +44), duration f32 +80, transition f32 +84, root name [32] +88, events ptr +120, event count +124, event count2 +128, unk0 +132. Events are 36 B each (pykotor `_EventStructure.SIZE`) and start at the events ptr. The node tree starts right after the events: root ptr = hdr + 136 + 36 x event_count (c_drdprot anim 0: 1 event, root at hdr+172).

Node header (80 B): type u16 +0, supernode u16 +2, node_id u16 +4 (the "index"), name_id u16 +6 (0 in all ~113k geometry and anim nodes checked), children ptr +44 (abs = value+12), children count +48. Names resolve through the name table indexed by node_id (pykotor `_load_node`), so name lookup uses +4, not +6.

Two further structural facts:
- Anim node type is 1 for all 32,172 nodes sampled (first 3 anims of each of the 1155 animated models), whatever the geometry type.
- Anim child lists are pruned relative to the geometry child lists. Example: S_Female03 anim 0, node 0: anim children [3, 100]; geometry children [1, 2, 3, 96, 97, 98, 99, 100].

## 3. Anim node count and the "missing" nodes

"Real anim node count" below = nodes reachable from the anim root through children pointers. The unreachable geometry nodes are not in the anim block: for S_Female03, 1,220 (anim, node) pairs are unreachable and none of them has an exact (sn, nid) header match in the anim block. The same check gives 0 matches for c_kath (84 pairs), c_drdprot (328), dor_dro02 (68). The few byte-pattern hits in p_hk47, s_male02 and n_darthnihilus (19 to 92 per model) are not headers (the type field at the hit is not 1; none match the node id).

Stored anim count (anim+44) is the model's OWN geometry count in 877/877 supermodel-linked anims and 5046/6516 NULL-supermodel anims. In the other 1470 NULL-supermodel anims (233 models, mostly dor_*) it is own + 1..10, and in all 1470 it is at least the largest anim node id + 1. Not explained (see section 7).

## 4. Results

### 4.1 Corpus (all 1155 animated vanilla models, 7393 anim headers)

| quantity | count |
|---|---|
| geometry node_id is exactly 0..N-1 (unique) | 3237 / 3237 models |
| anim-tree nodes, supermodel-linked models (69 models, 877 anims) | 62,587 |
| anim-tree nodes, NULL-supermodel models (1086 models, 6516 anims) | 115,926 |
| anim-tree nodes with supernode == own geometry node with the same index | 178,120 |
| anim-tree nodes where sn != geometry[index], explained by first-same-name rule | 161 |
| anim-tree nodes where sn != geometry[index], not explained | 4 (one model, mainmenu06: three nodes named LArm_Geo / RArm_Geo) |
| anim-tree nodes whose index is not a geometry node | 228 (NULL models, mostly dor_*) |
| anim-tree nodes whose index appears twice inside one anim | 105 (all NULL models; all duplicate names) |
| supermodel-linked anims where stored count == own geometry count | 877 / 877 |
| supermodel-linked anims where real (tree) count < stored | 870 / 877 |
| NULL-supermodel anims where real (tree) count < stored | 6099 / 6516 (equal 417, greater 0) |
| models with any sn mismatch, dup or no-geo node | 33 (31 NULL, 2 supermodel-linked) |

Duplicate-name rule (161 cases: 7 in supermodel-linked models, 154 in NULL models): for a node whose name occurs more than once in the name table, the anim header's supernode = geometry supernode of the lowest-index node with that name. Examples: pfbim Rhand (node 74, sn 348) has an anim sn of 21, the sn of the first Rhand (node 38, sn 21). c_drdprot Rwr_box (node 40) has anim sn 37, the sn of node 37. c_twohead talkdummy (node 51) has anim sn 19, the sn of node 19. For names that occur once, anim sn = geometry sn of the same node.

### 4.2 Per-model table (27 vanilla + 6 MDLOps)

Columns: anims; real anim nodes = tree count (range over anims, and total); stored = anim header node count (range); own = geometry node count (walk), f56 = file offset 56; f56 check = own + 1 + f56(supermodel) when the supermodel file is present; sn!=geo = strict mismatches (task definition); nogeo = anim node index with no geometry node; dup = index repeated inside one anim.

| model | class | supermodel | anims | real anim nodes (total; per anim) | stored | own | f56 | f56 check | sn!=geo | nogeo | dup |
|---|---|---|---|---|---|---|---|---|---|---|---|
| s_female03 | SUP body | S_Female02 | 68 | 5648 (17-97) | 101 | 101 | 525 | OK | 0 | 0 | 0 |
| s_female02 | SUP body | S_Female01 | 88 | 7715 (55-94) | 103 | 103 | 423 | OK | 0 | 0 | 0 |
| s_male02 | SUP body | S_Male01 | 230 | 19115 (17-102) | 109 | 109 | 212 | OK | 1 | 0 | 0 |
| n_bith | SUP body | S_Male02 | 10 | 576 (50-69) | 75 | 75 | 288 | OK | 0 | 0 | 0 |
| n_darkjhif | SUP body | S_Female03 | 5 | 289 (56-59) | 72 | 72 | 598 | OK | 0 | 0 | 0 |
| p_kreiabb | SUP body | S_Female03 | 9 | 655 (54-87) | 101 | 101 | 627 | OK | 0 | 0 | 0 |
| p_hanharr | SUP body | S_Female02 | 22 | 1296 (17-67) | 82 | 82 | 506 | OK | 0 | 0 | 0 |
| n_tusken | SUP body | S_Female02 | 9 | 485 (51-59) | 67 | 67 | 491 | OK | 0 | 0 | 0 |
| pfbim | SUP body (exception) | S_Female03 | 5 | 327 (62-68) | 80 | 80 | 606 | OK | 6 | 0 | 0 |
| h_m_hi01 | SUP head | CSpar2 (file absent) | 33 | 2008 (60-62) | 65 | 65 | 65 | no super file | 0 | 0 | 0 |
| n_jedmast2h | SUP head | S_Female03 | 1 | 28 (28) | 37 | 37 | 563 | OK | 0 | 0 | 0 |
| c_drdastro | SUP droid | P_T3M4 | 1 | 18 (18) | 31 | 31 | 63 | OK | 0 | 0 | 0 |
| c_rancors | SUP creature | c_rancor | 10 | 505 (49-52) | 57 | 57 | 115 | OK | 0 | 0 | 0 |
| c_holododonna | SUP creature | S_Female02 | 50 | 2932 (17-77) | 77 | 77 | 501 | OK | 0 | 0 | 0 |
| s_male01 | NULL body | NULL | 65 | 4737 (64-93) | 102 | 102 | 102 | n/a | 0 | 0 | 0 |
| p_g0t0 | NULL body | NULL | 76 | 1355 (3-24) | 25 | 25 | 25 | n/a | 0 | 0 | 0 |
| n_darthmalak | NULL body | NULL | 66 | 6315 (57-102) | 111 | 111 | 111 | n/a | 0 | 0 | 0 |
| n_darthnihilus | NULL body | NULL | 92 | 6710 (63-84) | 97 | 97 | 97 | n/a | 0 | 0 | 0 |
| p_hk47 | NULL droid | NULL | 79 | 3824 (10-57) | 58 | 58 | 58 | n/a | 0 | 0 | 0 |
| c_drdprot | NULL droid | NULL | 28 | 1370 (37-55) | 59 | 59 | 59 | n/a | 46 | 0 | 92 |
| c_drdassassin | NULL droid | NULL | 31 | 1330 (32-47) | 50 | 50 | 50 | n/a | 0 | 0 | 0 |
| h_m_hibody | NULL head | NULL | 6 | 276 (46) | 47 | 47 | 47 | n/a | 0 | 0 | 0 |
| c_kath | NULL creature | NULL | 28 | 980 (35) | 38 | 38 | 38 | n/a | 28 | 0 | 0 |
| c_twohead | NULL creature | NULL | 11 | 479 (9-47) | 53 | 53 | 53 | n/a | 11 | 0 | 0 |
| c_bomabeast | NULL creature | NULL | 34 | 1055 (29-32) | 35 | 35 | 35 | n/a | 0 | 0 | 0 |
| c_dewback | NULL creature | NULL | 10 | 896 (86-90) | 102 | 102 | 102 | n/a | 0 | 0 | 0 |
| dor_dro02 | NULL door | NULL | 8 | 136 (3-23) | 33 | 24 | 24 | n/a | 0 | 12 | 0 |
| k1p_carthbb (MDLOps) | MDLOps body | S_Female02 | 1 | 71 (71) | 87 | 87 | 511 | OK | 0 | 0 | 0 |
| k1p_carthh (MDLOps) | MDLOps head | S_Female02 | 1 | 29 (29) | 35 | 35 | 459 | OK | 0 | 0 | 0 |
| k1p_juhanih (MDLOps) | MDLOps head | k1p_juhanibb | 2 | 47 (19-28) | 35 | 35 | 652 | OK (652 = 35+1+616) | 0 | 0 | 0 |
| k1p_carthba (MDLOps) | MDLOps body | k1p_carthbb | 0 | none | none | 61 | 573 | OK (573 = 61+1+511) | n/a | n/a | n/a |
| k1p_juhaniba (MDLOps) | MDLOps body | k1p_juhanibb | 0 | none | none | 64 | 681 | OK (681 = 64+1+616) | n/a | n/a | n/a |
| k1p_juhanibb (MDLOps) | MDLOps body | S_Female03 | 0 | none | none | 90 | 616 | OK (616 = 90+1+525) | n/a | n/a | n/a |

MDLOps note: the three MDLOps files with animations (carthbb, carthh, juhanih) have 0 mismatches, 0 no-geo, 0 dup. The three without animations (carthba, juhaniba, juhanibb) have nothing to check. Some vanilla player bodies also carry 0 anims (e.g. p_bastilabb, 94 geometry nodes), so the anim check does not apply to them.

## 5. Offset 56

Anim nodes are NOT included in the file offset 56 total.

- NULL-supermodel: f56 == own geometry walk count in 2835/2835 models. Example: c_kath has 28 anims and 980 anim node headers but f56 = 38.
- Supermodel-linked: f56 == own + 1 + f56(supermodel) in 400/400 cases where the supermodel file is present. The remaining 2 have no supermodel file in the corpus and f56 == own.
- Example: S_Female03: own 101 + 1 + S_Female02 (423) = 525 = f56, while its 68 anims hold 5648 reachable anim nodes.

The geometry total is therefore the own geometry count plus 1 plus the supermodel's total, exactly the MDLOps R4 rule. The anim count stored in each anim header is the own geometry count (101 for S_Female03), not f56 (525).

## 6. Exceptions, detail

- pfbim (supermodel-linked): Rhand (node 74) and Lhand (node 73) in 3 anims carry the sn of the first same-named node (Rhand node 38 sn 21; Lhand node 57 sn 73). Geometry sn 348/347 for these nodes. Strict check: 6 mismatches, all explained by the duplicate-name rule.
- s_male02 (supermodel-linked): one anim (188) has w_Longsword (node 62, sn 107) where the geometry says 109. Strict check: 1 mismatch; its name is duplicated in the name table, so the rule explains it.
- c_drdprot: Rwr_box and Lwr_box each appear twice in the name table. The anim header for the second copy carries the first copy's sn (46 mismatches, 92 dup indices across 28 anims).
- c_kath: Rear appears twice (nodes 25 and 35); the anim header for node 35 carries sn 25 (28 mismatches, 1 per anim).
- c_twohead: talkdummy appears twice (nodes 19 and 51); anim sn for node 51 is 19 (11 mismatches).
- mainmenu06: LArm_Geo appears three times (nodes 100, 262, 268). Anim header for node 268 carries sn 100 in anim 0 but sn 262 in other anims. The same-first-name rule does not fully explain this (4 cases).
- dor_dro02: 12 anim nodes whose index (24, 29, ...) has no geometry node. Geometry has 24 nodes; stored anim count is 33.

## 7. Open items

1. Does the engine bind animation tracks by supernode (index) or by name? Vanilla anim headers follow the first same-named node for duplicate names. The mainmenu06 triple-name case does not follow a single rule. A writer that emits sn = geometry sn per index would differ from vanilla only for duplicate names.
2. Stored anim count exceeds the own geometry count by 1..10 in 1470 NULL-supermodel anims (233 models, mostly dor_*). It is never below max anim node id + 1. Not explained by the anim tree or by the no-geo nodes.
3. Anim tree is pruned relative to geometry in 99% of supermodel-linked anims. Whether the engine reads the anim header count or the tree is not established here.

## 8. Pykotor writer implications (for the h2-h5 chain)

- Anim header supernode should be the geometry supernode for the same index; for duplicate names, pykotor's name-based lookup would reproduce the vanilla first-match behaviour.
- Anim header node count = own geometry count (not f56, not tree count), matching vanilla.
- f56 = own + 1 + supermodel f56 (supermodel-linked) or own (NULL), anim nodes excluded.
