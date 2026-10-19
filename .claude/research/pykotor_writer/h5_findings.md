# h5: no-game acceptance test for pykotor K2 MDL output (10-18, read-only; scripts pykfix/h5_*.py)
## Bottom line
1. k1_port_validate is structural only: 0 findings on all 12 Carth/Juhani files, including the pykotor Juhani body that shreds in game.
2. Strongest discriminator: node supernode (u16 at node+2). pykotor writes the positional index; MDLOps and K2 vanilla use K2 supermodel-chain numbers.
3. Open conflict: pykotor n_commm has the same violation (35) yet was fine in game. Causality is not proven.
## Inputs (brief paths corrected)
- Carth pykotor: k1_port_stage/models/k1p_carth{ba,bb,h} (char_carth/override has no MDL). MDLOps: k1_port_stage/char_carth_mdlops/models/.
- Juhani pykotor: k1_port_stage/models/k1p_juhani{ba,bb,h}. MDLOps played OK 10-16: k1_port_stage/probe_mdlops_juhani3/models/.
- K1 truth: mdlops_t7/{carth_run1,juhani_run2}/work and pass2 (p_*.mdl). mdlops_t7/juhani_run2/pass2/s_female03.mdl is K1, not K2.
## 1. What is proven, and the decidable test
- Validator: structure only. k1_port_model.compare: no pose, controller, supernode, dangly or plane check.
- Vs K1, pykotor matches pose, controller values, skin semantics, triangles, normals and planes exactly. Real defects: supernode, and dangly (constraints = float(type)/1e6, positions pointer 0).
- MDLOps: triangles equal under (pos, uv) keys; normals recomputed (median 0 deg, p95 63 deg).
Skinned-bone supernode violations vs K2 chain (S_Female03/S_Female02):
| model (writer) | violations | in game |
|---|---|---|
| Juhani A, MDLOps K2-numbered | 0 | correct (10-16) |
| Juhani A, MDLOps K1-numbered | 31 | left arm out (10-16) |
| Juhani A, pykotor | 42 | shreds (10-16) |
| Carth A, MDLOps / pykotor | 0 / 31 | OK / left arm out (10-18) |
| n_commm, MDLOps K1-numbered / pykotor | 27 / 35 | arm out (10-12) / fine (10-14, 10-16): CONFLICT |
Checks, against K1 source plus K2 chain (never against MDLOps):
- T1 topology: pre-order names, child counts, name indices equal K1 (rename allowed).
- T2 supernode: skinned node in the chain has the chain value, exact. Non-skin mismatch = WARN.
- T3 skin: per vertex (node name, weight 3dp), matched by position. Serials ignored.
- T4 bones[16] slots beyond the real bone count lie in [0, node count). Writer hygiene only: K2 vanilla itself fails this in 617 of 1414 skins (doc 38 section 11).
- T5 pose and controllers vs K1 at 1e-4 (controllers as multiset).
- T6 triangles keyed by (pos, uv0) equal K1; normals exact on the pykotor path.
- T7 plane: abs(d + n.v1) <= 1e-3 * max(1, abs d).
- T8 dangly (if K1 has it): header scalars, constraints per position key 1e-4, positions non-null.
## 2. Legitimate differences (allowed noise)
- Names: rename map only. Controller order: MDLOps reorders; K2 vanilla keeps K1 order.
- Bone serials: MDLOps permutes bonemap/bones[]; semantics equal.
- Weld: vertex counts change (Juhani LArm 245 vs 252; hair 296 vs 234). Compare by key.
- bones[16] padding: K1 junk removed by both writers; range rule only.
- MDX bytes outside blocks (116 B in n_commm): ignore. Tangent zero block: already accepted.
- unknown0 values: count only. freelookhook: WARN only. Not compared yet: animation blocks, function pointers.
## 3. Wiring into k1_port_import.py
- k1_port_recipe.py L40 CONVERTERS gains "auto"; validation L282-283.
- build_character L1438-1452: "auto" runs pykotor ingest into a candidate stage under ctx.cdir, then the oracle per model. PASS copies to ctx.stage; FAIL runs mdlops_steps and mdlops_finish, reason in manifest.
- print_plan L1349, manifest near L1519, cmd_doctor near L1862 get the oracle and chain checks.
- juhani.toml, carth.toml switch to "auto" only after a play batch passes.
## 4. Staged plan (agent hours; play is user-only)
1. k1_port_mdl_oracle.py: stdlib T1-T8, CLI, synthetic tests. 6 h.
2. Calibrate: MDLOps PASS; pykotor and K1-numbered MDLOps FAIL T2; K2 vanilla sample PASS. 3 h.
3. Writer fix in k1_port_model.py: supernode from chain after write; dangly stays demoted. 4 h.
4. Re-convert Juhani, Carth, n_commm control; run oracle; queue rows. 1 h.
5. Importer "auto" branch, doctor, manifest, tests. 5 h.
6. Flip recipes after play; update doc 38 section 26 and README. 1.5 h.
Total about 20.5 agent hours.
## Play queue rows (never blocking)
- PQ-1: pykotor Juhani A with only supernode set to chain values. PQ-2: n_commm pykotor control, same session.
- PQ-3: pykotor Carth with chain supernodes. Dangly sway only after the T8 writer fix.
Read-only: no repo, game, venv or K2 file written.
