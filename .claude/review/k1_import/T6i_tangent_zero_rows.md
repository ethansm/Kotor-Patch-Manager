# T6i: all-zero tangent blocks (writer drops flag and block)

Scope: `patch_manager_mods/k1_port/pykotor_k2_mdl.patch` (io_mdl.py hunk; mdl_data.py unchanged), README. Trial venv `venvs/k1port_p6` only; main venv untouched; no commit.
Scratch: `scratchpad/P6/` (`scripts/raw_tangent.py` stdlib parser, `scripts/tangent_checks.py`, `lena/`, `checks/`).

## Change (`MDLBinaryWriter._update_mdx`)
- Skip flag 0x80 and the block when the source block is non-empty and every byte is zero. `writer.keep_zero_tangents` (getattr, default False) restores the old output.
- Stride drops by 36, offset stays 0xFFFFFFFF. Non-zero blocks and flag-only meshes: unchanged. Reader unchanged.
- Patch regenerated from stock: 246 -> 257 lines. Fresh stock + `patch -p1` reproduces scratch `b/` (io_mdl.py md5 b1115473..., mdl_data.py d27b6461...).

## Results (trial venv, stdlib raw parse)
| check | result |
|---|---|
| llk_LenaH source (K1) | 26 meshes, 9 flagged, 852 zero rows (matches T6f) |
| llk_LenaH converter output | 0 flagged; MDX 68128 (was 99168); MDL 97338 |
| 9 former meshes | node 17,21,22,23,28,29,30,31,32; stride 32 (6 eye/teeth), 64 (3 skins); offset -1 |
| K2 401dxna, read/write(force K2)/read-back | node 5 flag kept; 130 non-zero rows byte-identical (raw and pykotor) |
| Lena, `keep_zero_tangents=True` | 9 flags kept, stride 68/100, offset 32, MDX 99168 |

## Tool compare (`k1_port_model.py`, not edited)
- `match` false. Only `tangent_data` is false; every other field is true.
- Detail: `mesh 10 (teethlower): tangent flag True -> False, tangent bytes/vertex 36 -> 0 (mdx stride 68 -> 32)`.
- Cause: the compare sees a dropped all-zero block as a mismatch. Needs an equivalence rule for all-zero source blocks.
- Stale warning still printed: "pykotor does not preserve the skin unknown0 array". The patch keeps unknown0 (earlier hunk).

## Non-zero regression
- 401dxna output has 69 trimesh nodes: 9 zero-vertex (pre-existing, see T6f). The 60 non-empty meshes match stock on vcount, stride, flag, offset and rows.

## Tests
- `python3 scripts/run_tests.py k1_port` -> 4 passed, 0 failed. This uses the main venv, which still has the old patch, so it does not exercise this change.
- `tangent_checks.py` (trial venv, `-I -B`): RESULT PASS (A and B above).

## Rebuild hazard
- Do not re-run `setup_venv.sh` on a venv that has the old patch. Its reverse probe fails, and `patch` then reports hunks as "previously applied" and skips them (simulated in scratch). Delete `venvs/k1port` first, or use a new `K1PORT_VENV`.

## Not verified
- Engine behaviour with the flag cleared (K1 or K2). Whether the engine recomputes tangents.
- Evidence base: all 668 K2 and 4914 K1 flagged meshes have non-zero rows (K2v Check 3). Lena is the only known zero-block case.
