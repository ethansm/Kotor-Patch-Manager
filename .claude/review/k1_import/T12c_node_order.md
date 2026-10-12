# T12c: pykotor `_get_node_order` double 12-byte base (recon)

Verdict: CONFIRMED. The extra 12-byte base is in `_get_node_order`, not in the k1_port patch. It makes `load()` raise on some models. It does not change the node tree, and the converted output is not damaged.

## 1. Lines that add the extra base

Venv file: `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/lib/python3.12/site-packages/pykotor/resource/formats/mdl/io_mdl.py`

| Line | Code | Effect |
|---|---|---|
| 2315 | `self._reader.set_offset(self._reader.offset() + 12)` | The one correct base. Reader offset becomes 12, so `seek(p)` reads file offset `p + 12`. |
| 2460 | `startnode += 12` | Extra base #1. Start node is shifted again. |
| 2470 | `self._reader.seek(child_array_offset + 12)` | Extra base #2. Child array is read 12 bytes late. |

- `_load_node` (L2483, `self._reader.seek(offset)`) has no extra base. Node tree reads are correct.
- The root pointer is at file 0x34. Stored pointer P maps to file offset P + 12.
- Under the bug, the node header reads land at file P+28 (name) and P+68 (child ptr/count). The correct offsets are P+16 and P+56.
- The repo patch `patch_manager_mods/k1_port/pykotor_k2_mdl.patch` does not touch `_get_node_order`. The same two lines exist in other pykotor copies (`venvs/t3_tmp`, repo `.venv`, at L2451/L2461). This is upstream pykotor code.

## 2. Impact on the sample

Method: stdlib raw walker (`scratchpad/T12c/raw.py`, file offset = stored + 12). Compared with pykotor `MDLBinaryReader.load()` from the k1port venv (`scratchpad/T12c/pyk_check.py`, run with `-I` and `PYKOTOR_SKIP_MDL_PATCH=1`). The sample is 50 random K2 vanilla MDLs from `Models.bif` (seed 12, corpus 3237).

K2 sample (n = 50):

| Metric | Unpatched | Fixed (in-memory) |
|---|---|---|
| `_order2nameindex` equals raw pre-order | 0 / 50 | 50 / 50 |
| Load raises the bug's OSError | 0 / 50 | 0 / 50 |
| Load raises a different error (see Open) | 1 / 50 (K2:505ond_fghtr) | 1 / 50 (same) |
| Node tree equals raw (pre-order names and child counts) | 49 / 49 loaded | 49 / 49 loaded |

- Every sampled model has root children (50 / 50). Under the bug the reader sees root child count 0 and records only the root (list length 1).
- Non-supermodel (`NULL`): 43 sampled. All 43 have a wrong `_order2nameindex`. None raises the bug's OSError.
- Supermodel-bearing K2 models in the sample (S_Female02 x4, S_Female03 x1, P_BaoDurBB, N_Selkath) load fine. Supermodel status is not the trigger.

K1 controls and staged models (unpatched / fixed):

| Model | Unpatched | Fixed |
|---|---|---|
| K1 `s_female02` | OSError: exceed the streams boundaries | loads, tree == raw |
| K1 `s_female03` | OSError: exceed the streams boundaries | loads, tree == raw |
| K1 `n_commm` (and staged `k1p_commm`) | loads, order wrong, tree == raw | loads, order == raw, tree == raw |
| K1 `llk_LenaB` / staged `k1p_lenab` | loads, order wrong, tree == raw | loads, order == raw, tree == raw |
| K1 `llk_LenaH` / staged `k1p_lenah` | loads, order wrong, tree == raw | loads, order == raw, tree == raw |

- `_order2nameindex` is appended in `_get_node_order` and never read. Grep of the whole venv: only io_mdl.py L2388 and L2463 mention it. The k1_port code in the repo does not use it.
- So a `node_tree` comparison cannot see this bug, because the tree comes from `_load_node`. That is correct, and the bug has no output effect on loadable models.

## 3. Converted output damaged?

NO. Pre-order (depth, name, child count) of each staged model against its K1 source (`scratchpad/T12c/conv_cmp.py`):

- `k1p_commm` (59 nodes) matches `N_CommM` except the root name: `N_CommM` becomes `k1p_commm`. This is the intended model rename. Nothing else differs.
- `k1p_lenab` (87 nodes) matches `llk_LenaB` exactly.
- `k1p_lenah` (33 nodes) matches `llk_LenaH` exactly.

Impact is ingest-side. K1 `s_female02` and `s_female03` cannot be read unpatched, so the converter cannot ingest them until the fix is in. They are not converted wrongly.

## 4. Proposed fix (3 changed lines)

Apply to the upstream `io_mdl.py`, or as a new hunk in `patch_manager_mods/k1_port/pykotor_k2_mdl.patch`:

```diff
--- a/pykotor/resource/formats/mdl/io_mdl.py
+++ b/pykotor/resource/formats/mdl/io_mdl.py
@@ -2457,7 +2457,6 @@
         self,
         startnode: int,
     ):
-        startnode += 12
         self._reader.seek(startnode + 4)
         name_index: int = self._reader.read_uint16()
         self._order2nameindex.append(name_index)
@@ -2467,7 +2466,7 @@
         child_array_length: int = self._reader.read_uint32()
 
         if child_array_length > 0 and child_array_offset not in (0, 0xFFFFFFFF):
-            self._reader.seek(child_array_offset + 12)
+            self._reader.seek(child_array_offset)
```

Verified in memory: the same change, monkeypatched into the venv class in-process, gives order == raw 50/50 and loads both K1 supermodels. The venv file was not written. The scratch copy of the diff is `scratchpad/T12c/fixcheck/T12c_fix.diff`.

## 5. Notes and open items

- Side effect to avoid: importing `pykotor.resource.formats.mdl` runs `ensure_io_mdl_aabb_patched_on_disk()`, and `mdl_auto.read_mdl` can self-heal by writing io_mdl.py. Both are skipped with `PYKOTOR_SKIP_MDL_PATCH=1`. Hashes of io_mdl.py, mdl_auto.py and _install_mdl_io_aabb.py were unchanged after the runs.
- Open: K2 `505ond_fghtr` raises `struct.error` in `_load_node` at io_mdl.py L2671 (light flare colour read). It fails the same way before and after the fix, so it is a separate bug. Not investigated further.
- Open: corpus-wide rate of the OSError was not measured. Only the 50-model sample and the 2 K1 supermodels were run. Upstream pykotor status was not checked (offline).
- The earlier u04 statement "supermodels only fail because their root has children" is too narrow. The failure needs garbage child pointers reaching past EOF. Supermodel status alone does not trigger it (0 of 5 sampled K2 supermodel models failed).

## Scratch files

`/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/3873c768-9a9e-458f-a1ff-49e1af487bca/scratchpad/T12c/`: `raw.py`, `pyk_check.py`, `pyk_results.json`, `conv_cmp.py`, `venv_sha_before.txt`, `fixcheck/T12c_fix.diff`.
