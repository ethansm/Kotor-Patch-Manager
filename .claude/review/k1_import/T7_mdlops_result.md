# T7: MDLOps cross-check of `n_commm` (10-11, user GO)

Run dir (outside the repo): `~/Games/KOTOR2_Mod_Workspace/mdlops_t7/mdl_run1/` (own Proton prefix `prefix/`, `tools/mdlops.exe` extracted alone from `~/Downloads/mdlops-1.0.0.7z`, inputs copied by name from `mdlops_t7/inputs/`). No game dir, no `compatdata/208580`. Scripts: `scripts/T7_mdlops_*.{sh,py}` (paths are `$HOME`-relative).

## Result
- Pass 1 (binary -> ASCII) rc=0 under Proton Experimental, no extra dependencies needed: `n_commm-ascii.mdl` 318,204 B. Pass 2 (`-k2`, ASCII -> K2 binary) rc=0: `n_commm-ascii-k2-bin.mdl` 149,343 B + `.mdx` 101,248 B. Both ran in seconds, so the Tk/Win32::FileOp worry in the plan did not occur with the .exe.
- `k1_port_validate` on MDLOps' K2 output: 0 findings. On `k1p_commm`: 0 findings. Two independent writers both land inside the raw K2 layout the validator was calibrated on (3237 vanilla K2 models).
- Per-mesh MDX strides (`binary_mdx_strides`, 44 meshes): identical in MDLOps, ours and the K1 original (set {24, 32, 64}).
- Node tree and tbones/qbones: equal up to float noise (max diff 1e-5). Node names, model name and texture names differ only by resref (`N_CommM*` vs `k1p_commm*`).

## Differences (neither is evidence of an error in ours)
- Vertex counts: MDLOps re-welds vertices on the ASCII round trip (mesh 0: 43 vs 144; 41 of 44 meshes change). Ours keeps the K1 counts exactly (44/44), and our whole skin block (bonemap, weights, bone indices) equals the K1 original in every mesh. So per-vertex fields (`faces`, `normals`, `uv1`, `vertex_bones`) cannot be compared against MDLOps row by row.
- Three skin meshes also change vertex count in MDLOps in a non-weld way (228 -> 235, 587 -> 577, 228 -> 235).
- Bonemap: MDLOps swaps 6 entries in meshes 42 and 43 (e.g. 9<->10, 0<->1); ours equals K1. Both have 59 entries.
- MDX is smaller in MDLOps (101,248 vs 140,368 B) because of the weld, not because of a missing block (strides equal).

## Conclusion
MDLOps confirms the K2 layout class (validator clean, strides equal) and offers no reason to change `k1_port_model.py`. It is NOT a stricter reference for per-vertex data: it alters vertex counts and the bonemap, so the original K1 data remains the ground truth for those. It does not tell us whether the K2 engine accepts the file; the in-game probe (T12h) still decides that.
