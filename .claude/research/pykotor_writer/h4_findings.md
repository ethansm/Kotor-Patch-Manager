# h4 findings: pykotor danglymesh (0x121) writer/reader defect

Read-only recon. Probes: h4_probe.py (raw parser), h4_trace.py (writer offset trace), h4_reader_check.py, h4_vanilla_scan.py. Pykotor output without demote_dangly: h4_pyk_out/p_carthh_pyk.mdl. Subject: K1 Carth hair node `hair` (K1 N=40; MDLOps K2 N=20).

## 1. Bytes: stock writer vs MDLOps (file offsets)
| field | K1 source | MDLOps K2 | stock pykotor K2 |
|---|---|---|---|
| dangly hdr cptr (stored) | 58426 | 57684 | 60610 (file 60622 = inside last vertex) |
| real constraint block | 58438 | 57696 (right after hdr) | 60834 (after vertindexes) |
| N / vcount | 40/40 | 20/20 | 40/40 |
| posptr | 58586 (cptr+4N) | 57764 (cptr+4N) | 0 (positions never written) |
| faces ptr | 59066 (cptr+16N) | 58004 (cptr+16N) | 59114 (no dangly term) |
| first constraints | 235,235,0,0,233,233 | same | 0,233,233,213,213,208 (shifted) |
| hdr 28 B, trimesh 340 B, mdx bitmap 0x23 / stride 32 | - | same | same |

Pykotor block order: node, trimesh hdr, dangly hdr, faces, ic_counts, vertices, io_offsets, counters, vertindexes, constraints. No positions block.

## 2. Root causes (ranked)
1. Constraint pointer, writer: io_mdl.py `_calc_dangly_offsets` line 4186 sets `after_vertices - 12`. `after_vertices` is the vertex-array end in data space, so the pointer lands 12 B inside the last vertex. The -12 is applied a second time (writer offsets are data space: trace shows dangly hdr at writer pos 59086 = file 59098). Actual constraints are written at data 60822 by `_write_dangly_extra` (io_mdl.py 781, called at line 600 after vertindexes). Pointer is 212 B off.
2. Positions never written: `danglyverts_offset` is never assigned by the writer (only class lines 1521/1536/1555), `verts_original` is unused in io_mdl.py, `dangly_extra_size` (699) and `_write_dangly_extra` (781) count and write only 4N constraint bytes. Positions pointer 0 makes the engine read the geometry header as rest positions, consistent with the 10-15 "huge planes".
3. Reader offset (corrupts the source before any writing): io_mdl.py line 2984 `constraint_loc = offset_to_contraints + 12`. Reader space is data space (set_offset(+12); flare read at line 2650 seeks the raw pointer). Reader constraints equal raw bytes at file cptr+24: first 3 constraints dropped, last 3 = positions[0]. Positions (danglyverts) are never read (block ~2971-3004).
4. Ruled out: header field sizes and order (identical to MDLOps), trimesh size (340 K2), mdx stride (32 B, bitmap 0x23 both sides; MDX size gap is vertex merging, not dangly). Value encoding int(v*1e6)/1e6 round-trips exactly. The `demote_dangly` docstring (k1_port_model.py 299-300, "placeholder, not the K1 value") is wrong about the mechanism; the value defect is the reader shift.

## 3. Vanilla and MDLOps checks
- Vanilla K2 (3237 MDLs, 98 with dangly, 716 dangly nodes): N == vcount, cptr == dangly hdr end, posptr == cptr+4N, faces == cptr+16N in 716/716. Vanilla order is header, constraints, positions, faces.
- MDLOps merges exact-duplicate positions: K1 40 verts = 20 position pairs; MDLOps keeps 20, constraints identical (20/20). This explains the u02 "234 vs 296" OPEN item (same effect).

## 4. Fix sketch
- Pointer-only (keeps current order): reader drops the +12 at line 2984 and reads positions at posptr into verts_original; `_calc_dangly_offsets` sets cptr = node_offset + vertex_indices_offset(game) + vertex_indices_size() (no -12), posptr = cptr + 4N; `dangly_extra_size` returns 16N; `_write_dangly_extra` writes constraints then positions; set positions list near line 3384.
- Vanilla order (safer): call `_write_dangly_extra` right after `dangly.write` (line 579); add dangly size to `faces_offset` (850); drop it from `children_offsets_offset` (917); cptr = node_offset + all_headers_size(game), no -12.
- Keep `demote_dangly` until an in-game sway check; only the game proves sway.

## 5. Byte-level test (no game; skip if K1 cache absent, K1 assets never in repo)
- T1 layout: raw parse of any writer output; cptr == hdr end (vanilla) or vertindexes end (pointer-only); posptr == cptr+4N; N == vcount; faces == cptr+16N.
- T2 reader value oracle: raw K1 constraints at file cptr+12 == pykotor reader values. Fails today (first 6 differ).
- T3 round trip: K1 -> writer -> raw parse; constraints and positions equal K1 index for index (N=40, no merge on the pykotor path). Fails today.
- T4 MDLOps reference: each MDLOps vertex matches a K1 vertex with equal position and constraint (passes today, 20/20).
- T5 regression: dangly mdx size and bitmap unchanged; non-dangly child and controller pointers byte-identical before and after.

## OPEN
Does the engine accept constraints and positions after vertindexes (pointer-only fix), or does it need vanilla order? Only an in-game test answers; plan for vanilla order.
