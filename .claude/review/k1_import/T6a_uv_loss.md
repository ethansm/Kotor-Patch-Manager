# T6a: n_commm "UV loss" (recon, read-only)

Scratch: /tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/4b302fe0-554c-4823-af59-025f7d920a16/scratchpad/T6a
(dump_k1.py, raw_parse.py, pk_probe.py, mdl_sections.py, run_fixed.py, uv_fix.diff, manifest.json, manifest_fix.json)

## Answer
- The 25 meshes are genuinely UV-less in the K1 source. Not a pykotor reader gap.
- The K2 output has no source UVs for them either. The writer zero-fills UV1 on 11 of them.
- The uv_checksum failure is 11 meshes, not 25. The real data loss is the skin unknown0 array (side finding).

## Per-class (K1 source, 44 trimesh-type nodes)
| Class | n | K1 MDX has UV1? | bitmap / stride / tex0 off | pykotor vertex_uv1 | K2 output |
|---|---|---|---|---|---|
| TRIMESH, texture NULL, no UV | 14 | no | 0x21 / 24 / -1 | [] | still no UV |
| TRIMESH, texture N_CommM04, no UV | 11 | no | 0x21 / 24 / -1 | [] | TEX0 zero-fill (nz=0), stride 32 |
| SKIN (ArmL, torso, ArmR) | 3 | yes | 0x23 / 64 / 24 | 228 / 587 / 228 | UVs kept |

- All 25 UV-less nodes are TRIMESH (node_type 2). Skins are SKIN (9).
- Stride 24 = pos 12 + normal 12 exactly, so no UV can sit at another offset.

## Evidence
- raw_parse.py on K1 (T6a/k1_raw.txt): lfootT_g bm=0x21 stride=24 t0=-1; ArmL bm=0x23 stride=64 t0=24.
- pk_probe.py: pykotor K1 no_uv1=25 (matches raw count); K2 output no_uv1=14.
- io_mdl.py:2828-2835: reader fills vertex_uv1 only when TEX0 (0x2) is set. mdl_data.py:3008: default is [].
- io_mdl.py:3830-3845: writer zero-fills UV1 when texture_1 is non-NULL and no uv1 exists (comment: "MDLOps requires texture vertex data").
- Per-mesh diff via k1_port_model.summarize: exactly 11 uv_checksum mismatches (rcollar_g + 10 R-finger/thumb _g). manifest mismatch_detail index 11 = rcollar_g.
- Warning text k1_port_model.py:227-234 says "25 meshes ... zero UVs". Only 11 get zeros.

## Attributes
- MDLMesh instance: vertex_uv1 (list, default []), vertex_uv2, vertex_uvs (alias of uv1), tangent_space, uv_jitter, uv_direction_x/y.
- Not exposed on MDLMesh: mdx_data_bitmap, mdx_data_size (stride), texture offsets. These live on the internal _TrimeshHeader and are dropped after read.

## Side finding: K2 MDL is 708 bytes short
- 708 = 3 skins x 59 entries x 4 bytes, i.e. the skin unknown0 array.
- Reader never stores it (io_mdl.py:809 "currently unused"). Writer sets unknown0_count=0 (io_mdl.py:3489).
- Skin MDX rows: writer puts bone indices at +0x20 and weights at +0x30 (io_mdl.py:3872-3876). K1 has weights +0x20, bones +0x30. Header and data agree with each other; engine acceptance untested.

## Scratch fix trial (not applied to venv or repo)
- Patch: io_mdl.py:3830 `elif (` becomes `elif False and (`, in T6a/fixpkg. Diff: T6a/uv_fix.diff (11 lines).
- Run: T6a/run_fixed.py (scratch pykotor first on sys.path) -> manifest_fix.json: match=true, uv_checksum=true, k2_mdx_bytes=140368 (= K1).
- Caveat: K1 shipped n_commm itself has textured meshes with no TEX0 and loads in K1, so the zero-fill is not an engine requirement for K1. K2 unverified.

## Fix ideas
1. Byte-faithful UVs: drop zero-UV synthesis at io_mdl.py:3830 (verified in scratch above).
2. Or keep zero-fill and make the k1_port_model.py compare treat empty uv1 on a textured mesh as zeros. Fix the 25 -> 11 count in bare_uv_warning.
3. Close the -708: store skin unknown0 in _SkinmeshHeader.read_extra, emit it at io_mdl.py:3489.

## Open
- Does the K2 engine accept textured meshes with no TEX0? Does it accept swapped skin weight/bone offsets? Does it need skin unknown0? Not tested in game.
- The supermodel S_Female02 "unreadable" warning was not investigated.
