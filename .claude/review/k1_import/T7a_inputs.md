# T7a: K1 model inputs for mdlops_t7

## K1 install
- Path used: `/home/ethansm/.local/share/Steam/steamapps/common/swkotor` (Steam appid 32370, "STAR WARS Knights of the Old Republic"; `swkotor.exe` present)
- Note: the requested pattern `Star Wars*Knights*` matched nothing; the folder is named `swkotor`.
- Read-only: pykotor 2.3.12 `Installation(<path>)` in `~/Games/KOTOR2_Mod_Workspace/venvs/k1port`, run with `-I`. Nothing written to the game dir.

## Exported files
Output dir: `~/Games/KOTOR2_Mod_Workspace/mdlops_t7/inputs/` (new, created by this task)

| File | Bytes | SHA-256 | Supermodel (next in chain) | K1 source |
|---|---|---|---|---|
| n_commm.mdl | 165323 | bc7836e1d43daedf95e699ee53753c9599226b136ef2bd73ca072b4207f6f8e8 | S_Female02 | data/models.bif |
| n_commm.mdx | 140368 | eed7ff8a8dae136b003d629ae66ffeae6daa008e203854d022e32ae86d800b9c | n/a | data/models.bif |
| s_female02.mdl | 906680 | 607819480b40297092a70aff50a7e204d5166d233d62ee32bfbdca054226dc3f | S_Female01 | data/models.bif |
| s_female02.mdx | 44152 | d79dffb0dc35bf0364251be3142c431fb1eb2c80a4955b0c7c94d1d2082a1a3f | n/a | data/models.bif |
| s_female01.mdl | 1042696 | f33abb1db46fc5189e95a038c3111b9f8b4885b2101bd992056768967b67c96b | S_Male02 | data/models.bif |
| s_female01.mdx | 44152 | e2efd51a22610f238390d5492a27226568649e5330bb29fe85fde6bf2a00ba0d | n/a | data/models.bif |
| s_male02.mdl | 2718844 | 720583afa6df88856510f57f00d021c9072f607065cf7c233a839f5169bbf17a | S_Male01 | data/models.bif |
| s_male02.mdx | 42584 | de1d28f719c877c2785442406fd4f1fcfb7baa879fbb02fb9523c65c38568f10 | n/a | data/models.bif |
| s_male01.mdl | 325037 | 80c8dbcf5cdd7f1259f3bf164a95a60199b44ecd09be5b37ddb2df261a2565df | NULL (chain end) | data/models.bif |
| s_male01.mdx | 42424 | a219744e0443efccb82974675deb340719e3679d92eba5b1567c18627e89d8d1 | n/a | data/models.bif |

Chain: n_commm -> S_Female02 -> S_Female01 -> S_Male02 -> S_Male01 -> NULL.

## Verification
- Each file's on-disk size equals the byte count pykotor returned, and each SHA-256 matches the in-memory data.
- Each resref has exactly one K1 location (`data/models.bif`). Override/ is empty. `patch.erf` has no MDL/MDX entries for these names.
- The `supermodel` field was read from the MDL model header, not file offset 0x28. Offset 0x28 holds float data. The name sits at file offset 0x94 (model header +0x38). The chain ends cleanly at NULL on s_male01, which supports this offset.
- A full pykotor MDL parse fails on these files (stream bounds error in `_get_node_order`). The supermodel was read raw instead.
- Directory listing re-read at end: 10 files, sizes as above.
