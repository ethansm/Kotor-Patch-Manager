# H3: pykotor 2.3.12 K2 MDL patch coverage (10-18, read-only recon)

Sources: pykotor_k2_mdl.patch (19 hunks), setup_venv.sh, doc 38 sec 2 and 12-22, k1_port_model.py, k1_port_validate.py, test_k1_port_model.py. Extra read-only runs: h3_supernode.py, h3_nodeid_check.py (outputs h3_*.out/err).
Verification key: G = game play (I = indirect only), B = byte/size compare, U = unit test, N = none.

## 1. Hunk table (patch order; names from the patched venv io_mdl.py / mdl_data.py)

| # | Function | Fixes | Verified by |
|---|---|---|---|
| 1 | _write_skin_extra (comment) | none | N/A |
| 2 | _write_skin_extra (unknown0 write) | writes skin unknown0 u32 array (K1 dropped it) | B: T6h, K2 = K1 + 8 per header; K2v corpus. U: none |
| 3 | skin_extra_size (comment) | none | N/A |
| 4 | _TrimeshHeader.read (K2) | stops reading two K2 tail uint32s (K2 trimesh = K1 + 8) | U: test_one_mesh_rename_and_k2_size; B: B2a, B2b; G: I |
| 5 | _TrimeshHeader.write (K2) | stops writing tail uint32s; 10-byte dirt block kept | U: same test; G: I |
| 6 | _SkinmeshHeader.__init__ | adds unknown0 list | prerequisite |
| 7 | _SkinmeshHeader.read_extra | reads unknown0 array, bounds-checked | B: T6h size only. U: none |
| 8 | _load_node (tangent_on) | reads 36-byte per-vertex MDX tangent block | U: test_zero_tangent_block_cleared_end_to_end (byte count only). G: N |
| 9 | _load_node (per-vertex) | stores tangent rows, short rows zero-filled | U: as 8. G: N |
| 10 | _load_node (skin copy) | copies unknown0 to output skin | B/U: as 7 |
| 11 | MDLBinaryWriter.write | honours force_game (set by k1_port_model) | U: all CLI K2 size tests. G: I |
| 12 | _update_node (skin counts) | unknown0 count = count2 = len(list) | B: T6h. U: none |
| 13 | _update_mdx (has_texture1) | synth_zero_uv opt-out; UV-less meshes stay UV-less | U: test_textured_mesh_without_uv1..., test_synth_zero_uv_flag... G: I |
| 14 | _update_mdx (tangent flag) | sets flag and offset after UVs; drops flag and block when all rows zero (deliberate, no vanilla precedent) | U: test_zero_tangent_block_cleared_end_to_end (+192 MDX bytes). G: N |
| 15 | _update_mdx (tangent write) | writes 36 bytes per vertex | U: as 14 (size only). G: N |
| 16 | _update_mdx (skin rows) | MDX skin row: weights first, then bone indices | Validator SKIN_ROW_ORDER (order only); B: K2v 1414 skins; G: I (fixed before 10-12 A/B) |
| 17 | _calc_skin_offsets | skin payload starts at children_offsets_offset minus skin size (was 1532 B off on ArmL) | Validator BAD_POINTER and SKIN_ARRAY_ORDER: 0 findings on staged k1p models; B: T6d; G: I |
| 18 | mdl_data MDLMesh.__init__ | tangent_block attribute | prerequisite |
| 19 | mdl_data MDLSkin.__init__ | unknown0 attribute | prerequisite |

Caveat: the compare in k1_port_model reads skin rows and pointers back through the header offsets, so it is self-consistent and cannot catch a row-order or pointer error. The skin round-trip test (test_k1_port_model.py:403) uses the K1 writer, not K2.

## 2. (a) K2 requirements in the docs NOT in the patch or k1_port_model post-processing

1. Node header +2 "supernode" (MDLOps "node number") must follow supermodel-chain numbering. The pykotor writer sets it to the local pre-order index (io_mdl.py ~3329-3334) and drops the source value. Measured 10-18 (h3_supernode.out): pykotor K2 output has supernode == node_id on 64/64 (p_juhaniba) and 59/59 (n_commm); K2 vanilla p_bastilabb 3/94, s_female03 2/101, n_commm 3/60. Pelvis: pykotor 3, K2 chain 86, K1 source 68. Docs sec 20-22 fixed this on the MDLOps side only. No compare or validator field covers it.
   Refuted: name-index (+4) renumbering. K1 +4 ids equal pre-order position on n_commm, p_juhaniba, p_juhanibb, p_juhanih (0 mismatches; h3_nodeid_check.out).
2. Danglymesh data: constraints array plus vertex-position array (positions pointer = constraints pointer + 4 x count). pykotor writes constraints as float(type)/1e6 and no positions (doc sec 19 u02). Not in patch; k1_port_model.demote_dangly makes it static.
3. Face plane distance as float: post-process only (fix_face_planes). The stock reader still truncates through an int field.
4. bones[16] padding: post-process only (pad_skin_bones). Doc sec 14 says K2 vanilla keeps junk in 617 of 1414 skins, so the real rule is "in range where read"; the boundary is unverified.
5. Never compared: controller content and order (doc sec 6, sec 19), node transforms, mesh flags, trimesh layout token pair per game and mesh kind (io_mdl.py ~1111-1120, ~3335-3353; unknown whether the engine reads them).
6. Open from doc: bonemap length 59 vs 60 (K2 freelookhook, sec 4 item 1c). No tool checks the supermodel name match (58/64, sec 20).
Non-MDL: head TPC alpha 255 and clamp TXI (sec 24) live in k1_port_tex_tpc, not the MDL path.

## 3. (b) Validator (k1_port_validate.py, stdlib, K2 layout)

- Checks: MDL and MDX size fields; model and texture names (length, NUL, printable, lowercase under --k1p); name table; node walk (cycles, duplicates, bounds, name index range); face and index array bounds; TEX0 and TANGENT bit vs offset; block offsets, stride = sum of blocks, block range and overlap; MDX length; skin counts (bonemap vs qbones, tbones, unk0 counts and count2); skin array order; weights offset <= bone-index offset.
- Does NOT check: bones[16] content (by design), bonemap values, bone_node_number, node id or supernode against any chain, supermodel name, face vertex indices or plane distances, MDX float content, dangly extra data (type 0x100 never parsed), light/emitter/AABB/saber extras, controllers, animations, layout tokens, unknown0 content, node transforms.
- Callers: ingest (k1_port_ingest.py:317), probe (k1_port_probe.py:359), import, mdlops finish. k1_port_model.py does not call it.

## 4. Test gaps (test_k1_port_model.py)

- No test sets unknown0 content, node supernode, dangly write, or tangent content. The compare reduces tangents to a flag, a byte count and a zero/non-zero bool.
- Golden test (line 950) needs both installs and checks n_commm only: bones[16] first 14 slots and planes.

## 5. (c) Known remaining defects

1. Juhani body A shards on the pykotor path (p_juhaniba, 10-15/16): cause unknown. Leading untested candidate: supernode (2.1). Shipped Juhani uses converter mdlops, so not blocking. Caveat: pykotor k1p_commm renders with the same local numbering, so this candidate does not yet explain commm.
2. Left arm held out: MDLOps K1 supernodes (doc sec 22 root cause). Fixed by the K2-chain build in k1_port_mdlops (variant 3 played OK 10-16). Pykotor arm status on Juhani untested.
3. Danglymesh head (hair `h`, cloth `bb`), 10-15: pykotor cannot write it; demote_dangly (fa66d8d) removes sway and did not fix all head artifacts (sec 18 item 2). MDLOps keeps dangly valid (sec 19 u01); sway still unconfirmed (JUH-PT1 queued).
4. Tangent: content never compared; flag clearing on zero rows is a deliberate deviation; the nonzero tangent path was never played.
5. Face planes: fixed only in k1_port_model; not in compare.
