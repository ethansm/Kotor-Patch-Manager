# T6: K1 -> K2 proof of concept, n_commm

Tools: `patch_manager_mods/k1_port/*` (`k1_port_model.py` unmodified; the pykotor patch now has the skin pointer fix (T6e) and the tangent block hunk (T6g)).
Interpreter `~/Games/KOTOR2_Mod_Workspace/venvs/k1port/bin/python -I`. Venv rebuilt for T6g; second `setup_venv.sh` run prints "Patch already applied, skipping".
Outputs (local only): `~/Games/KOTOR2_Mod_Workspace/k1_port_stage/`. Scratch: `.../scratchpad/T6/` (textures, pre-fix manifests), `.../scratchpad/T6e/` (re-run logs: `run_pack.out`, `run_vanilla.out`, `run_lena.out`), `.../scratchpad/T6g/` (tangent re-run: `run_vanilla.out`, `run_lena.out`, `vanilla_out/`, `lena_out/`, two-file patch `new_pykotor_k2_mdl.patch`).

## (a) Texture stage

| Staged file | Source | Dims / format | Bytes | sha256[:16] |
|---|---|---|---|---|
| `override/k1p_commm04.tpc` | `N_CommM04.tpc` (upscale_plus_v0.52) | 2048x2048, DXT5 (tool) | 5592560 | 70d3de2e3bc9b5cb (= source, `cmp` identical) |

Map: `{"N_CommM04": "k1p_commm04"}`. `stage_manifest.json`: 1 file, 0 warnings.

## (b) Model manifests (`k1_port_model.py`, re-run T6e after skin fix)

| Field | pack run (`poc/pack`) | vanilla run (`poc/vanilla`) |
|---|---|---|
| source | pack dir `Copy contents...` | install `data/models.bif` |
| match | true | true |
| skin_bone_poses | true | true |
| tangent_data | true | true |
| nodes / meshes / skins | 59 / 44 / 3 | 59 / 44 / 3 |
| vertices / faces | 3603 / 2292 | 3603 / 2292 |
| k1 mdl / mdx bytes | 165323 / 140368 | 165323 / 140368 |
| k2 mdl / mdx bytes | 164967 / 140368 | 164967 / 140368 |
| texture refs renamed | 18 (`N_CommM04` -> `k1p_commm04`) | 18 |
| supermodel S_Female02 | unreadable (install) | unreadable (install) |
| tool warnings | 3 (11 UV-less, skin unknown0, supermodel) | same 3 |

The vanilla run also needs `--tex-map N_CommM04=k1p_commm04` (CLI requires it).
Manifests: `scratchpad/T6e/run_pack.out`, `run_vanilla.out`. Pre-fix T6 manifests are superseded.
T6g re-run after the tangent fix (install source, `scratchpad/T6g/run_vanilla.out`): match=true, all compare fields true, sizes unchanged (K2 164967 / 140368). N_CommM has no tangent meshes.

## (c) Independent check (`indep_check.py`, run before the skin fix)

- Node names, order, types, depth: equal (59).
- Per-mesh vertex and face counts, positions, normals, face index lists: 0 mismatches of 44.
- UV1 vectors (exact): 0 mismatching meshes of 44. The 11 textured UV-less meshes stay UV-less in the output.
- Textures: 18 refs changed, all `N_CommM04` -> `k1p_commm04`. No other change.
- MDX size: 140368 = 140368. 41 of 41 non-skin mesh blocks byte-identical. 1043 of 1043 skin rows have the weight and bone 16-byte halves swapped relative to K1.
- Size relation: K2 = K1 + 8 x 44 trimesh headers - 708 skin unknown0 = 164967 (delta -356). MDL and MDX sizes unchanged after the skin fix.
- Pack vs vanilla output: MDL identical. MDX differs only in UV1 of ArmL and ArmR (23 of 228 vertices each). This is a source difference in the pack, not a tool error.
- MDX bytes outside vertex blocks (1504, per-mesh trailers): 15 differ in 8 residual bytes after UV-less meshes. Unreferenced.
- **PASS (after pointer fix, T6e): skin bone poses.** Before the fix, the pointers to bonemap/qbones/tbones were 1532 B short on N_CommM ArmL (the faces block sat between them), so readback gave face bits. The fix (`_calc_skin_offsets` uses the writer's data order, see T6d) makes re-read bonemap, qbones and tbones match K1 for 3 of 3 skins. `vertex_bones` and `bone_node_number` matched before the fix too.

## (d) Stretch: llk_LenaH (Twi'lek NPC Diversity)

- Listed with py7zr. Extracted only `tslpatchdata/llk_LenaH.mdl` and `.mdx` to `~/Games/KOTOR2_Mod_Workspace/k1_port_extract/twilek_f/`. `Install.exe` was not run.
- Model tool, `--pack-dir` (tslpatchdata), `--tex-map llk_LenaH=k1p_lenah`, nothing staged, output to scratch (`T6g/lena_out`). Run after the tangent fix (T6g): **match=true**.
- Counts: 33 nodes, 26 meshes, 3 skins, 1450 vertices, 1566 faces. Supermodel `N_TwilekF` read OK.
- K1 mdl/mdx 97130 / 99184. K2 mdl/mdx 96942 / 99168 (was 96942 / 68128 before the tangent fix).
- Flags (T6g run): `skin_bone_poses` true, `tangent_data` true, `uv_checksum` true; every other compare field true.
- Skin bone poses: PASS (3 of 3), same as N_CommM.
- Texture ref: `llk_LenaH` (9 refs). Not staged. Upscaled variant in the archive is unused.
- Tangent fix: before it, `tangent_data` FAILED. 9 meshes (6 eye/teeth, 3 skins) lost the 36 B/vertex tangent block (stride 68 -> 32, 100 -> 64). The hunk keeps the raw block and re-emits it. The K1 block is all zero for these meshes (T6f), so the fix restores layout, not data. The 16 B gap to K1 (99168 vs 99184) is per-mesh alignment padding (T6f).

## Open for in-game test

1. Skin bone poses: tool compare PASS after the pointer fix (T6e, pack and vanilla). Engine acceptance untested.
2. Skin weight/bone offsets: writer header and MDX agree (weights at +0x30, bones at +0x20). K1 source has weights at +0x20. Engine acceptance untested.
3. Skin unknown0 dropped: 708 bytes for N_CommM, 396 for Lena. Engine acceptance untested.
4. UV-less textured meshes (11 in N_CommM): engine acceptance untested.
5. S_Female02 supermodel is unreadable in pykotor. The output carries no supermodel data.
6. Lena tangent blocks (9 meshes): restored, `tangent_data` true (T6g). Engine acceptance untested.

Tool gaps: the compare now covers `skin_bone_poses` and `tangent_data`. It still does not check skin unknown0 (dropped, warned) or the supermodel (unreadable). `match=true` is not enough for those.
