# h6c: vanilla K2 MDL tree quirks and raw offsets (recon, read-only)

## Verdict
PARTIAL. Every offset in the brief is confirmed against the pykotor reader on 8 vanilla files (0 mismatches). Four points need correcting or caveating:
1. Node +6 is NOT a name index. It is 0 in all geometry nodes of all 3237 files. The name index is +4.
2. Anim header +44 is NOT the anim-tree size. It equals the walked size in only 424 of 7393 anim headers.
3. Anim node supernode by geometry name works only for unique names. 93 disagreements occur in 6 models with duplicate names.
4. Supermodel-root pointer (geometry+168) is not always the root. It points at `neck_g` in 177 head models.

## Inputs and method
- Corpus: 3237 vanilla `.mdl` files in `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/885aae19-5eea-47ec-b2fb-cd9b10afd222/scratchpad/k2mdl/`. All parse (0 fatal).
- Parser: stdlib `struct` only. Scripts and outputs live in `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/885aae19-5eea-47ec-b2fb-cd9b10afd222/scratchpad/h6c/`:
  - `h6c_scan.py` (walk plus the four categories) writes `scan.jsonl` (one record per model).
  - `h6c_final.py` and `h6c_summarize.py` (sorted offender lists).
  - `h6c_xcheck.py` (pykotor cross-check), `h6c_anim.py`, `h6c_animcheck.py`, `h6c_key.py`, `h6c_idx_nodup.py`, `h6c_extra.py`, `h6c_superroot.py`.
  - `h6c_snippet.py` (the walk outline below, tested).
- Pointer rule: every raw pointer P is relative to file byte 12. Absolute file offset = P + 12. pykotor does the same (`MDLBinaryReader.__init__`, `set_offset(+12)`, `io_mdl.py` ~2315).
- Walk order: pre-order DFS, children in file order. Name lookup: `names[u16 at node+4]`. Name table entries are u32 raw pointers (rel 12) to NUL-terminated strings.

## Task A: counts (3237 geometry trees)

| # | Check | Models | First 10 (sorted by model) |
|---|---|---|---|
| 1 | Duplicate sibling names (lowercase, same parent) | 64 | 001ebo10 (cg2_lights2), 002ebo (lmfilllight03), 105perf (object137 x2), 105perg (object141), 105perh (object141), 105perj (object143 x6), 105perm (object144), 105pero (object145), 107perg (auroralight31), 151har07 (har07_floor_02) |
| 2 | Supermodel root name differs from supermodel root name (case-insensitive) | 400 of 400 resolvable (402 have a supermodel) | ad_saul (ad_saul vs N_AdmrlSaulKar), baodur_bc, baodur_bd, baodur_be, baodur_bf (vs P_BaoDurBB), c_bomabeastv (vs C_Bomabeast), c_bosdrexl (vs C_Drexl), c_drdastro (vs P_T3M4), c_gammoreanp (vs C_Gammorean), c_hadmnoffh (vs S_Female02) |
| 3 | Node +4 index != pre-order position | 111 | 001ebo15 (9 of 62 nodes), 003eboq (11/18), 101per2a (51/57), 101perbl (2/14), 101peri (10/140), 101perzc (18/34), 106per11 (2/36), 107perg (8/83), 151har01 (18/137), 151har07 (20/31) |
| 4 | Cyclic, revisited, out-of-file, null or bad-name-index pointers | 0 | none (also 0 in 7393 anim headers) |

Notes:
- Check 2 is trivially true. The root node name equals the model's own name in 399 of 400 (the exception is `pmbnm`, root `PMBMN`). Supermodel roots are likewise named after their own file. Do not use root-name equality as a link check. The supermodel link is the root at index 0 with supernode 0, which holds in 3237 of 3237 files.
- Two supermodel targets are not in the extract: `h_m_hi01` wants `CSpar2`, `n_djedi_h` wants `S_Male02H`.
- Check 3: +4 is a name index, not a pre-order index. It coincides with pre-order in 3126 models, where the name table happens to follow tree order. In the 111 others it does not. A writer must not renumber +4 to pre-order unless it also rewrites the name table to match.

Other counts from the same scan:
- Name table longer than the geometry node count: 233 models, always longer (for example `dor_dan01`: 17 names, 9 nodes). Do not use the name count as a node count.
- Geometry node-count field (+44, file 56) equals the own walked count in 2835 of 2835 no-supermodel models. It equals own + supermodel total + 1 in 400 of 400 resolvable supermodel models. This confirms h2 rule R4.
- Children count2 (+52) equals count (+48) in every geometry node. Anim count2 equals anim count in every model.
- Root node: index 0 and supernode 0 in 3237 of 3237.

## Task B: offsets (confirmed)

### File and geometry header (G = 12, all absolute file offsets)

| Field | Abs offset | Type | Status |
|---|---|---|---|
| Zero word | 0 | u32 | 0 in all files |
| MDL content length | 4 | u32 | equals file size - 12 |
| MDX length | 8 | u32 | confirmed |
| Layout tokens | 12, 16 | u32 x2 | geometry header starts at file 12 (confirmed) |
| Model name | 20 | char[32] | confirmed |
| Root node pointer | 52 (= G+40) | u32 raw | CONFIRMED; abs node = raw + 12 |
| Geometry node count | 56 (= G+44) | i32 | CONFIRMED |
| Model type byte | 92 (= G+80) | u8 | e.g. 4 |
| Anim array pointer | 100 (= G+88) | u32 raw | CONFIRMED; array at raw + 12 |
| Anim count | 104 (= G+92) | u32 | CONFIRMED; count2 at 108 (= G+96) |
| Supermodel name | 148 (= G+136) | char[32] NUL-padded | CONFIRMED; the string `NULL` means none (2835 of 3237) |
| Super-root pointer | 180 (= G+168) | u32 raw | see correction 4 |
| Name array pointer | 196 (= G+184) | u32 raw | CONFIRMED; array starts at file 208 in the files checked |
| Name count | 200 (= G+188) | u32 | CONFIRMED; count2 at 204 (= G+192) |
| Model header end | 208 | | pykotor reads 208 bytes of prefix |

- The name array holds `count` u32 raw pointers. Each one, plus 12, is a NUL-terminated name. Example: P_BastilaBB entry 0 raw 572 gives file 584 = `P_BastilaBB`.
- Child model count is at file 96 (G+84), and is 0 in every file checked.

### Node header (80 bytes, abs A = raw + 12)

| Offset | Type | Meaning | Status |
|---|---|---|---|
| +0 | u16 | type flags (0x0001 base, 0x0021 mesh, 0x0121 dangly) | CONFIRMED |
| +2 | u16 | supernode (K2 chain number) | CONFIRMED; root = 0 |
| +4 | u16 | name index into the name table (pykotor node_id) | CONFIRMED. This is the "index" field |
| +6 | u16 | always 0 | CORRECTED. Not a name index. 0 nonzero in 3237 files |
| +8 | u32 | offset_to_root | not checked |
| +12 | u32 | offset_to_parent | not checked |
| +16 | f32 x3 | position | pykotor reads it |
| +28 | f32 x4 | orientation (w, x, y, z) | pykotor reads it |
| +44 | u32 raw | children array pointer | CONFIRMED; array at raw + 12 |
| +48 | u32 | children count | CONFIRMED |
| +52 | u32 | children count2 | equals +48 in every geometry node; no separate capacity observed |
| +56 | u32 raw | controllers pointer | pykotor reads it |
| +60, +64 | u32 | controller count, count2 | pykotor reads it |
| +68 | u32 raw | controller data pointer | pykotor reads it |
| +72, +76 | u32 | controller data length, length2 | pykotor reads it |

- Children array: `count` entries of u32 raw pointers, 4 bytes each, at (+44 value) + 12. Each child abs = entry + 12.
- Nodes checked against pykotor `_NodeHeader`: type, padding0 (= supernode), node_id (= +4), name_id (= +6), children pointer, children count, children count2. All 535 nodes on 8 files match.

### Animation header and anim tree (136 bytes, abs H = array entry + 12)

| Offset | Type | Meaning |
|---|---|---|
| +0, +4 | u32 x2 | layout tokens (same as geometry) |
| +8 | char[32] | animation name (for example `pauseinj`) |
| +40 | u32 raw | root node pointer of the anim node tree. Abs = raw + 12 |
| +44 | u32 | node count field. NOT the walked tree size (see correction 2) |
| +48 to +75 | 28 bytes | unknown |
| +76 | u8 | geometry type byte |
| +80 | f32 | duration |
| +84 | f32 | transition |
| +88 | char[32] | root model name (equals the model's own name) |
| +120 | u32 raw | events pointer (raw + 12) |
| +124, +128 | u32 | event count, count2 |
| +132 | u32 | unknown0 |

- Event record: 36 bytes (f32 time, then char[32] name). pykotor `_EventStructure`.
- Addressing: model geometry+0x58 (file 100) holds the raw pointer to the anim pointer array. The array has geometry+0x5C (file 104) u32 raw pointers, each to a 136-byte anim header.
- Anim node tree: starts at anim+40 (raw). Anim node headers use the same 80-byte layout and the same children arrays as geometry nodes. To edit an anim node's supernode, write u16 at (node abs + 2).
- Anim headers checked against pykotor `_AnimationHeader` on 6 files with animations: all fields match (root pointer, node count, duration, events).

### Corrections to the brief
1. Node +6 is always 0. The name index is +4. (Also confirmed by pykotor: `node_id` is read from +4 and used to index names.)
2. Anim +44 is not the walked size. It matches in 424 of 7393 anim headers. Walk by pointers and stop on invalid pointers, not on this count.
3. Anim node supernode from geometry, keyed by name: 178,023 anim nodes whose name is unique in the geometry match on both +2 and +4 (0 misses). Duplicate-name geometry names: 93 supernode disagreements across 6 models (for example `421dxn_1`, `laser01`: anim 154, geometry 153). Keying by +4 instead is worse (165 misses). 228 anim nodes in 18 models have no geometry node with that name.
4. Super-root pointer (+0xA8, file 180) is not always the root. It equals the root pointer for all 2835 no-supermodel models and 225 supermodel models. For the other 177 supermodel models (head models) it points at the node named `neck_g`. This matches pykotor's `headlink` detection.
5. h2's count of "5823 anim headers" for S_Female03 does not match. This scan finds 68 anim headers and 5648 anim-tree nodes (walked by pointers). The 5823 figure may count something else.

### Cross-check against pykotor reader
- Files: `p_bastilabb`, `s_female03`, `s_female02`, `pfha01` (head), `c_dewback` (creature), `c_tankdroid` (droid), `fxc_droid01` (droid), `c_rancor` (creature).
- Method: pykotor `_ModelHeader`, `_NodeHeader` and `_AnimationHeader` read with `BinaryReader.from_bytes(data, 0)` at the absolute offsets above. Results compared to the raw offsets.
- Result: 0 mismatches on model header fields and on 535 node headers. Anim headers match on the 6 files with animations. `MDLBinaryReader(...).load()` gives the same node names in pre-order on all 8 files.

## Walk outline (tested; `h6c_snippet.py`)
```python
import struct
def u16(b, o): return struct.unpack_from('<H', b, o)[0]
def u32(b, o): return struct.unpack_from('<I', b, o)[0]

def iter_tree(b, root_raw):
    # node header abs = raw + 12; children = count u32 raw pointers at (cptr + 12)
    seen, stack = set(), [root_raw + 12]
    while stack:
        a = stack.pop()
        if a in seen or a < 12 or a + 80 > len(b):
            raise ValueError('bad node pointer %d' % a)
        seen.add(a)
        yield a, u16(b, a + 4), u16(b, a + 2)          # (abs, index +4 = name index, supernode +2)
        cnt, cp = u32(b, a + 48), u32(b, a + 44)
        kids = [u32(b, cp + 12 + 4 * k) + 12 for k in range(cnt)] if cnt else []
        stack.extend(reversed(kids))

def iter_model(b):
    G = 12
    for a, idx, sn in iter_tree(b, u32(b, G + 40)):     # geometry root raw at file 52
        yield ('geo', a, idx, sn)
    aptr, acnt = u32(b, G + 0x58), u32(b, G + 0x5C)     # anim array raw ptr (file 100), count (file 104)
    for i in range(acnt):
        ah = u32(b, aptr + G + 4 * i) + G               # anim header abs
        for a, idx, sn in iter_tree(b, u32(b, ah + 40)):    # anim root raw at anim+40
            yield ('anim%d' % i, a, idx, sn)
```
Test result: `s_female03` gives 101 geometry and 5648 anim nodes. `p_bastilabb` gives 94 and 0.

## Open
- Which key maps anim nodes to geometry supernodes in the duplicate-name models? Name lookup misses 93 and +4 lookup misses 165.
