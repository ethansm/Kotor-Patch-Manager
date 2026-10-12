# T6f: llk_LenaH tangent block (K1 -> K2 writer)

Scope: read-only on venv, pack, installs. Fix in scratch copy only. No repo edits, no commit.
Scratch: `scratchpad/T6f/` (scripts in `scripts/`, fix diff `tangent_fix.diff`).

## (a) Raw parse of K1 source (struct only; stride and base match pykotor for all 26 meshes)

| mesh | name | node | vcount | bitmap | stride | tangent off | all-zero rows |
|---|---|---|---|---|---|---|---|
| 10 | teethlower | 17 | 10 | 0xA3 | 68 | 32 | 10/10 |
| 14 | eyeLlid | 21 | 29 | 0xA3 | 68 | 32 | 29/29 |
| 15 | eyeRA | 22 | 17 | 0xA3 | 68 | 32 | 17/17 |
| 16 | eyeLA | 23 | 17 | 0xA3 | 68 | 32 | 17/17 |
| 21 | teethupper | 28 | 10 | 0xA3 | 68 | 32 | 10/10 |
| 22 | eyeRlid | 29 | 29 | 0xA3 | 68 | 32 | 29/29 |
| 23 | tongue (skin) | 30 | 22 | 0xA3 | 100 | 32 | 22/22 |
| 24 | head (skin) | 31 | 359 | 0xA3 | 100 | 32 | 359/359 |
| 25 | hair (skin) | 32 | 359 | 0xA3 | 100 | 32 | 359/359 |

- Row layout: pos @0, normal @12, uv1 @24, tangent block @32 (36 B), skin only: weights @68, bone indices @84.
- Totals: 852 tangent rows, 852 all-zero, 0 non-zero. 852 x 36 = 30672 B.
- Stock K1 `models.bif` (indicative): 9 models, 14 flagged meshes parsed, 0 all-zero rows. Example m41aa_01d (tangent @40 after uv2) row 1: T=(1.0,0.001,0.005), B=(1.0,-0.001,0.006), so T.B is about 1 and the (T,B,N) order is not supported.
- Stock K2 `Data/Models.bif`: 8 models, 15 flagged meshes parsed, 0 all-zero rows.
- Unusable stock pairs: 401/1202 (K1) and 63/1236 (K2). MDX shorter than row offsets. Pairing unresolved, not investigated.

## (b) Reader and writer (venv pykotor 2.3.12, `io_mdl.py` line numbers)

- Reader drops the data, keeps the flag. `io_mdl.py:1287` reads `mdx_tangent_offset` and never uses it. `io_mdl.py:2761-2763` sets `mesh.tangent_space` from the flag only. Row loop `io_mdl.py:2843-2900` reads VERTEX, NORMAL, TEX0, TEX1 only. `mdl_data.py:2996` MDLMesh has `tangent_space: bool` only.
- Writer skips the block. `io_mdl.py:3781` sets `mdx_tangent_offset = 0xFFFFFFFF`. `io_mdl.py:3866-3871` emits the flag only ("calculated, not stored"). Per-vertex emit `io_mdl.py:3887-3919` writes VERTEX, NORMAL, TEX0, TEX1, skin. `io_mdl.py:3882` sets stride = suboffset, so stride drops 68 to 32. Terminator row `io_mdl.py:3961` is sized from the stride, so it shrinks too.

## (c) Requirement evidence

- MDLOps 1.0.0.7z source (`MDLOpsM.pm`): flag 0x80 at :244; tangent row = 9 floats at :270; offset `unpacked[60]` at :2033; reader skips a row type whose offset is -1 at :2329-2330. MDLOps tolerates flag-without-block.
- pykotor comment `io_mdl.py:3867-3868` claims tangents are computed at runtime. No data here supports or refutes it.
- K2 engine requirement: UNKNOWN. Not tested, not guessed.

## (d) Fix trial (scratch, 1 attempt, 2 files, 27 added / 1 removed line)

- Reader keeps the raw 36-byte block as `mesh.tangent_block`. Writer emits it after the UV blocks (K1 order) and sets the offset. Flag-only path is unchanged.
- Tool `k1_port_model.main` (scratch copy, force K2, synth off) on each pykotor:

| | K1 source | K2 baseline | K2 fixed |
|---|---|---|---|
| MDX bytes | 99184 | 68128 | 99168 |
| tangent stride (6 eye/teeth / 3 skin) | 68 / 100 | 32 / 64 | 68 / 100 |
| tangent offset | 32 | 0xFFFFFFFF | 32 |
| tool `tangent_data` | - | false | true |

- Byte check, 26/26 meshes: pos/normal/uv bytes identical to K1. Tangent rows all zero in K1 and fixed.
- Gap accounting: K1 - baseline = 31056 = 30672 (tangent slots) + 384 (gap region, 1312 vs 928). The fix recovers 30672 + 368 (terminator rows grow 36 per tangent mesh, plus 44 alignment). Residual K1 - fixed = 16 B, all in per-mesh alignment padding (6 meshes shift 8 B). No row data.
- Regression: stock K2 `401dxna` (130 non-zero tangent rows). Read, write, read: fixed keeps those rows byte-identical; unfixed drops them. Mesh count 60 to 69 with and without the fix (pre-existing).
- Tool `match` stays false from `skin_bone_poses` (bonemap -1 to 0, mesh 23). Pre-existing, not touched.

## (e) NOT verified

- Engine behaviour in K1 or K2: zero-filled blocks, flag-without-block (baseline), 16 B padding shift. No game run.
- Whether the engine reads or recomputes tangents. Whether (T,B,N) order is right.
- Fix tested on one pack model and one stock K2 model only. Repo test suite not run.
