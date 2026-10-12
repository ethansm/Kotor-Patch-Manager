# T6h: converted skins match vanilla K1/K2 layout (MDX row order + unknown0)

Patch: `patch_manager_mods/k1_port/pykotor_k2_mdl.patch` regenerated from stock 2.3.12 (previous repo patch had 10 hunks, not 6; now 19). README: +2 bullets, stale unknown0 bullet replaced.
Scratch: `scratchpad/P5/` (verify_skin.py, mdx_diff.py, old_vs_new.py, out/, out_lena/, old_out/, old_out_lena/).
Venv rebuilt: first setup run applied the patch, second run printed "already applied"; venv io_mdl.py and mdl_data.py byte-equal to stock + new patch.

## Changes
- A. Skin MDX row: weights at +32, bone indices at +48 (offsets set and written in that order). Stride 64 unchanged.
- B. `unknown0` kept: `_SkinmeshHeader` and `MDLSkin` get `unknown0: list[int]`. Reader reads u32 x count (bounds-checked). Writer sets count == count2 == len(list) (0 when empty). `_write_skin_extra` emits it after tbones, zero-padded if short. `skin_extra_size` and `_calc_skin_offsets` already cover it.

## 1. Raw parse, n_commm (stdlib, K1 source = T6a/k1/k1_n_commm.mdl)
| check | ArmL | torso | ArmR | previous patch |
|---|---|---|---|---|
| w_off < b_off (32 < 48) | yes | yes | yes | no (48 / 32) |
| unknown0 count == count2 == bonemap (59) | yes | yes | yes | 0 / 0 |
| unknown0 ptr (MDL-rel) | 113803 | 150247 | 165359 | 0 |
| unknown0 u32 list == K1 list | yes | yes | yes | n/a |
| bonemap, qbones, tbones bytes == K1 | yes | yes | yes | yes (pointer hunk) |
| blocks contiguous, children ptr follows unknown0 | yes | yes | yes | n/a |
| bones[16] == K1 | yes | yes | yes | yes |

## 2. MDL size
- Out 165675 = K1 165323 + 352 (44 trimesh headers x 8 B). Exact.
- Previous patch: 164967 (708 B short = 3 x 59 x 4).

## 3. MDX
- Size 140368 = K1 140368.
- Skin rows: weights 1043/1043 and bones 1043/1043 equal K1 (ArmL 228, torso 587, ArmR 228).
- 116 bytes differ from K1, all in the 16-byte alignment padding after mesh blocks (offsets 24-31 past each block end). K1 holds leftover floats there, output has zeros. No vertex row reads them.
- Previous patch vs new: same size, all differing bytes (18130) inside skin rows, 0 outside.

## 4. Tool compare (n_commm)
- match=true. All 11 keys true, including uv_checksum, skin_bone_poses, tangent_data.

## 5. Tests
- `python3 scripts/run_tests.py k1_port` -> 4 passed, 0 failed, 0 error (6.1 s).

## 6. llk_LenaH (`--pack-dir` tslpatchdata, `--source-id leilukin_k1_twilek_f_2179`)
- match=true, all 11 compare keys true.
- MDL 97130 -> 97338 (+208 = 26 x 8, exact). Previous patch: 96942.
- Skin rows equal K1: tongue 22/22, head 359/359, hair 359/359. unknown0 3/3 equal K1.
- MDX 99168 vs K1 99184 (-16). Same as previous patch. Mesh blocks sit 8 B earlier from f_mdbrw_g on. Pre-existing, not touched.

## Notes
- Stale warning text in `k1_port_model.py` ("pykotor does not preserve the skin unknown0 array ... 4 bytes per entry") is now false. Not edited (other agent owns that file).
- Supermodel "S_Female02 unreadable" warning unchanged, not investigated.
- Engine acceptance of skin row order and unknown0 is untested in game.
