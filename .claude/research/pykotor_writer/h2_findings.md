# h2 findings: K2 node numbers and the pykotor writer

## 1. What the "node number" is
- The number in doc 38 (pelvis_g 86, lhand_g 62) is the node header **2nd uint16 (supernode, padding0)**, not the 3rd (node_id = file index, 0..N-1). Evidence: P_BastilaBB has 94 nodes, so 86 cannot be a file index.
- Engine-facing consequence: animation node headers carry the same supernode as the geometry node (S_Female03: 5823/5823 match). Animation channels are therefore keyed by this number.
- bones[16] and bonemap are file indices (checked by bone names on P_BastilaBB ArmL/ArmR), not supernodes. They need no change.

## 2. MDLOps rule (MDLOpsM.pm `get_super_nodes`, lines 3814-3885; applied at 4622-4633)
- R0 root: child node 0 is matched to supermodel node 0 (K2 roots are sn 0).
- R1 matched: a child whose lowercase name equals a direct child of its matched supermodel parent takes that node's supernode.
- R2 parent matched, child not: value = `max`, then `max++`. `max` starts at (max supernode over the supermodel's nodes) + 1. Order is DFS pre-order over children in file order.
- R3 under an unmatched node (any depth): value = own file index + supermodel total + 1.
- R4 geometry totalnumnodes written = own geometry node count + supermodel total + 1 (no supermodel: own count). Anim headers are NOT counted (S_Female03: 101 + 423 + 1 = 525, while it has 5823 anim headers).
- R5 no supermodel: value = file index (current pykotor behaviour).

## 3. Verification (stdlib/pykotor read-only prototype `h2_sn_sim.py`, ported R0-R4)
| child | supermodel | nodes match | total file vs predicted |
|---|---|---|---|
| K2 P_BastilaBB | K2 S_Female03 | 94/94 | 620 vs 620 |
| K2 S_Female03 | K2 S_Female02 | 101/101 | 525 vs 525 |
| MDLOps K2 k1p_juhanibb (B) | K2 S_Female03 | 90/90 | 616 vs 616 |
| MDLOps K2 k1p_juhaniba (A, played OK) | MDLOps B | 64/64 | 681 vs 681 |
- Concrete A values: pelvis_g 86, lhand_g 62, lforearm_g 61 (matched); necklwr_g 359 (R2: 358+1); neck_g 669 (R3: 52+616+1).
- Caveat: S_Female02 to S_Female01 (1 node, total 423 vs 210) and S_Female01 to S_Male02 (9 nodes, total 106) do not fit. Juhani does not use those links; read the vanilla binaries and do not regenerate them.

## 4. Arm A/B (the only difference is the supernode)
- `probe_mdlops_juhani2` (cased, K1-numbered supermodel): arm out.
- `probe_mdlops_juhani3` (cased, K2-numbered): arm correct.
- Same type, name and file index on 64/64 nodes, identical SKIN bones16 and bonemap lines. Only supernode differs (48 nodes).
- The K1-numbered build (pelvis 68, lhand 28) is wrong for K2 because the supermodel's numbers differ.

## 5. What pykotor writes now (venv pykotor 2.3.12, io_mdl.py)
- `_update_all_data` 3299-3301: node_id_override = i (all_nodes() DFS pre-order = file index).
- `_update_node` 3327-3332: node_id = i; **padding0 (supernode) = node_id** (own index). No supermodel awareness anywhere in the writer.
- Anim headers 3784-3789: node_id_override = geometry index by name, so padding0 = geometry index (also wrong under R0-R5).
- Geometry node_count 4357-4362: geometry + anim counts. Wrong: K2 totals exclude anims, and the own+supertotal+1 term is missing. Written value for k1p_juhaniba = 64 (K2 needs 681). The comment at 4358 says MDLOps includes anims; the data says otherwise.
- Skin bones16 and bonemap (3495-3504) are copied from K1 values; they match MDLOps except 2 slot-order swaps (RArm slots 10/11, LArm 6/7). The slot permutation is consistent with the MDX (doc 38 section 19), so it is not the fault.
- Output check: `models/k1p_juhaniba.mdl` has supernode == index (max 63, total 64). Simulation predicts 61/64 mismatches against the K2 chain.

## 6. Minimal design for a pykotor-side fix (K2 only)
1. Reader (io_mdl.py ~2488 and ~263): keep `node.supernode = header.padding0` and the geometry total on the parsed MDL. mdl_data.py MDLNode: add `supernode`. About 12 lines.
2. New `assign_supernodes(child_root, super_view) -> (sn_by_node_id, total)`: direct port of R0-R4, about 60 lines (prototype: `h2_sn_sim.py` `supernodes()`).
3. Writer: compute the map once in `_update_all_data`; `_update_node` padding0 = map value (fallback node_id when no supermodel); anim headers use the geometry node's value by name; node_count = own + (S_total+1 if super). About 25 lines.
4. Supermodel view: K2 vanilla link = read the binary (sn + total) from the K2 install, read-only (as `k1_port_mdlops.k2_model_bytes` does). Built-in-run parents (B before A) = the child view computed in step 2. Build in dependency order (same as `cmd_pass2`). About 40 lines in k1_port_model.py glue.
5. Tests: skip-if-no-install golden tests for P_BastilaBB (94/94) and S_Female03 (101/101); MDLOps-fixture golden for B (90/90) and A (64/64, totals 616/681) kept local, not committed.
- Estimate: about 100 lines in pykotor (io_mdl.py and mdl_data.py), about 40 in k1_port glue, about 100 in tests. Bones16, bonemap and MDX are untouched.

## 7. Open items and caveats
- Engine mechanism: animation mapping by supernode is inferred from the A/B test and the matching anim headers. The engine code was not read.
- Counter-example: pykotor `k1p_commm` (supernode = index) has 56/59 mismatches against S_Female02 yet is reported as rendering fine. Numbering alone does not explain every body failure.
- Controllers were not compared in the A/B test (doc 38 section 20 mentions controller-order differences).
- K1 chain: not simulated. The K1 total rule matches once (v2 A total 572 = 64 + 507 + 1).

## Scripts (scratchpad/pykfix, read-only)
- `h2_probe.py` (node_id, supernode, bones16, bonemap), `h2_anim_probe.py` (anim vs geometry supernode), `h2_geo.py` (geometry total), `h2_sn_sim.py` (R0-R4 port and check).
- Source: `h2_mdlops_src/MDLOpsM.pm` (extracted from `~/Downloads/mdlops-1.0.0.7z`, read only).
