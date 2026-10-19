# h1 findings: pykotor vs MDLOps Carth bodies (recon, read-only)

## Inputs
- pykotor (patched writer): `k1_port_stage/models/k1p_carth{ba,bb,h}.mdl/.mdx` (10-14 01:16).
  The task path `char_carth/override/` does not exist; INSTALL_READY lists `models/` as the Carth set.
- MDLOps: `k1_port_stage/char_carth_mdlops/models/k1p_carth{ba,bb,h}.mdl/.mdx` (10-18 18:12).
- Both pass `k1_port_validate.py` with 0 findings for all three bodies.
- Scripts (stdlib, `python3 -I`): `h1_mdl_diff.py` (per-node field diff), `h1_vertex_probe.py`,
  `h1_chain_probe.py`, `h1_summary_probe.py`, `h1_arm_map_probe.py`, `h1_unk0_probe.py`, `h1_bind_diff_probe.py`.
- Outputs: `h1_diff_<body>.txt`, `h1_summary.txt`, `h1_vertex_<body>.txt`, `h1_unk0_<body>.txt`.

## Ranked differences (most likely to cause a wrong arm / shards first)
1. **Supernode field (+2) numbering.** pykotor sets +2 = positional node_id on every node.
   MDLOps writes K2 chain numbers. Differs on 50/61 (ba), 76/87 (bb), 34/35 (h) nodes.
   - Left arm, all renumbered: lcollar_dum 22>57, lbicep_g 24>59, lforearm 27>74, lhand 29>73, fingers 30-39 > 63-72.
   - Right upper arm (rbicep 7, Rforearm 9, Rhand 10) and torso are equal. Right hand/fingers and legs differ.
   - MDLOps numbers match doc 38 section 20 (pelvis_g 86, lhand_g 62, lhand 73, lforearm 74).
   - pykotor's own chain is inconsistent: body A vs its supermodel `k1p_carthbb` disagree on 19 names (pelvis_g 42 vs 67).
     MDLOps A vs B agree on 59/59 shared names.
   - Source: patched pykotor `io_mdl.py:3336` does `padding0 = node_id` unconditionally. Its comment (3329-3331) says use supernode when defined.
2. **Skin `unknown0` u32 array (skin+52/+56).** pykotor copies the K1 values; MDLOps writes all zeros.
   - pykotor non-zero counts: ba RArm 2, LArm 49, torso 32; bb Torso 87, LArm 77, RArm 48, Legs 66; h tongue 35, Head 35.
   - Junk includes float-bit words (LArm idx 2-4, 15-17), not small ints.
   - K2 vanilla populates unk0 with count = bonemap count and small ints (K2v_layout_evidence.md, Check 2).
   - Engine use of unk0 is unverified. MDLOps zeros play correctly, so zeros are safe.
3. **`total_num_nodes` (+44).** pykotor: 61 / 158 / 64. MDLOps: 573 / 511 / 459.
   MDLOps = own name count + supermodel chain total (exact for bb and h: +424).
   Low likelihood: pykotor values already cover its own ids, and MDLOps 573 exceeds max K2 supernode 351.
4. **MDX vertex duplication.** Same faces; pykotor has more rows (torso_g 134 vs 30; torso skin 710 vs 655; bb torso 915 vs 791).
   Stride, bitmap, and block offsets are identical on 44/44 (ba), 68/68 (bb), 26/26 (h) meshes.
   Per-position binding sets are identical (ba RArm 163/163, LArm 163/163, torso 487/487). Not a binding cause.
5. **bones16 / bonemap palette order** (LArm, torso) differs, e.g. LArm slot 7 is LaFngrT_g in pykotor and LaFngrB_g in MDLOps.
   Each file is self-consistent (bonemap<->bones16 inverse: 0 failures), and resolved vertex bindings are identical. Ruled out.

## Identical or ruled out
- Name table: identical on all bodies. node_id (+4): identical on all common names.
- bones[16] padding slots: zero in both writers (RArm/LArm/torso, ba).
- qbones (ba, all skins): identical within 1e-3 across writers. Both match the inverse of the header rest pose (61/61).
- tbones: identical across writers within 1e-3. My rest-pose check does not reproduce them (a uniform +0.13 Y offset in both writers); convention unverified.
- Controllers (ba): first-key values identical 61/61. Only order differs.
- Face vertex indices are within vc in both writers (all 3 bodies).
- Face material ids differ (pykotor 1-6, MDLOps 1,2,4,8,16,32). Not checked further; not skin-related.

## Open
- Does the engine bind animation tracks by the +2 supernode (K2 numbering) or by name?
  Test: pykotor body A with only +2 rewritten to the MDLOps values, then play. Also test unk0 zeroed alone to isolate item 2.
- No engine verification done. Analysis used MDLOps numbers and doc 38 section 20 as the K2 reference. The game dir was not read.
- Juhani precedent (doc 38 sections 20-22): K1 numbering in MDLOps gave arm-out; K2 numbering fixed it. Supports item 1 as mechanism.

## Verdict
PARTIAL. The numbering difference is real and matches the left-arm symptom pattern. The engine mechanism and the unk0 role are unproven.
