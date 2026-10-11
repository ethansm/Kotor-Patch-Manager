# H7 texture dimensions: commoner pool (N_CommM / N_CommF)

Source: `patch_manager_mods/npc_diversity_src/override/` (70 files: 69 TPC + `N_CommM10.tga`).
Read-only recon. No repo file changed except this output.

Method: struct parse of every file header. TPC header: u32 data_size @0x00, f32 alpha @0x04, u16 width @0x08, u16 height @0x0A, u8 pixel_type @0x0C, u8 mip count @0x0D, data from 0x80. Compressed = data_size != 0; pixel_type 4 + compressed = DXT5 (layout matches pykotor `io_tpc.py:121-160`). TGA header read directly (type 2, depth 32, 512x512, TGA 2.0 footer).
Cross-check with pykotor `read_tpc` (.venv): N_CommF01 (1024, DXT5, 11 mips), N_CommF12 (512, DXT5, 10 mips), N_CommF20 (2048, DXT5, 12 mips), N_CommM10.tga (512x512; pykotor reports RGB 3 bpp, alpha bytes all 255). All four agree with the struct parse.

## Table

| File | WxH | Format | Mips (hdr) | Bytes on disk | Notes |
|---|---|---|---|---|---|
| N_CommF01.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF02.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF03.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF04.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF05.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF06.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF07.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF08.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF09.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF10.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF11.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF12.tpc | 512x512 | DXT5 | 10 | 349680 |  |
| N_CommF13.tpc | 512x512 | DXT5 | 10 | 349680 |  |
| N_CommF14.tpc | 512x512 | DXT5 | 10 | 349680 |  |
| N_CommF15.tpc | 512x512 | DXT5 | 10 | 349680 |  |
| N_CommF16.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF17.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF18.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF19.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF20.tpc | 2048x2048 | DXT5 | 12 | 5592591 | +31 B trailing text |
| N_CommF21.tpc | 2048x2048 | DXT5 | 12 | 5592591 | +31 B trailing text |
| N_CommF22.tpc | 2048x2048 | DXT5 | 12 | 5592591 | +31 B trailing text |
| N_CommF23.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF24.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF25.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF26.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF27.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF28.tpc | 2048x2048 | DXT5 | 12 | 5592591 | +31 B trailing text |
| N_CommF29.tpc | 2048x2048 | DXT5 | 12 | 5592591 | +31 B trailing text |
| N_CommF30.tpc | 2048x2048 | DXT5 | 12 | 5592591 | +31 B trailing text |
| N_CommF31.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF32.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommF33.tpc | 2048x2048 | DXT5 | 12 | 5592581 | +21 B trailing text |
| N_CommF34.tpc | 2048x2048 | DXT5 | 12 | 5592581 | +21 B trailing text |
| N_CommF35.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM01.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM02.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM03.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM04.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM05.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM06.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM07.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM08.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM09.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM10.tga | 512x512 | TGA 32bpp uncompressed (alpha all 255) | 0 | 1048620 | only texture with this name; no .tpc |
| N_CommM11.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM12.tpc | 512x512 | DXT5 | 10 | 349680 |  |
| N_CommM13.tpc | 512x512 | DXT5 | 10 | 349680 |  |
| N_CommM14.tpc | 512x512 | DXT5 | 10 | 349680 |  |
| N_CommM15.tpc | 512x512 | DXT5 | 10 | 349680 |  |
| N_CommM16.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM17.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM18.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM19.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM20.tpc | 2048x2048 | DXT5 | 12 | 5592591 | +31 B trailing text |
| N_CommM21.tpc | 2048x2048 | DXT5 | 12 | 5592591 | +31 B trailing text |
| N_CommM22.tpc | 2048x2048 | DXT5 | 12 | 5592591 | +31 B trailing text |
| N_CommM23.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM24.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM25.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM26.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM27.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM28.tpc | 2048x2048 | DXT5 | 12 | 5592581 | +21 B trailing text |
| N_CommM29.tpc | 2048x2048 | DXT5 | 12 | 5592581 | +21 B trailing text |
| N_CommM30.tpc | 2048x2048 | DXT5 | 12 | 5592581 | +21 B trailing text |
| N_CommM31.tpc | 1024x1024 | DXT5 | 11 | 1398284 | +28 B trailing text |
| N_CommM32.tpc | 1024x1024 | DXT5 | 11 | 1398284 | +28 B trailing text |
| N_CommM33.tpc | 1024x1024 | DXT5 | 11 | 1398284 | +28 B trailing text |
| N_CommM34.tpc | 1024x1024 | DXT5 | 11 | 1398256 |  |
| N_CommM35.tpc | 2048x2048 | DXT5 | 12 | 5592581 | +21 B trailing text |

## Summary

- Count by size: 2048x2048 = 15 (F 8, M 7); 1024x1024 = 46 (F 23, M 23); 512x512 DXT5 = 8 (F 4, M 4); 512x512 TGA = 1 (M10).
- Bytes on disk: 2048 group 83,888,805; 1024 group 64,319,860; 512 group 2,797,440; TPC total 151,006,105 (144.0 MiB); TGA 1,048,620; all files 152,054,725 (145.0 MiB).
- VRAM: every DXT5 file already stores its full mip chain (header counts 12/11/10 levels; payload equals the chain sum plus any trailing text). So VRAM is about 1.0x the file bytes, not 1.33x. Applying 1.33x to on-disk bytes would double-count (191.5 MiB).
- VRAM estimate (all mips uploaded): DXT total 150,996,784 B (144.0 MiB) = 2048 group 80.0 MiB + 1024 group 61.3 MiB + 512 group 2.7 MiB. TGA raw 1,048,576 B (1.0 MiB, no mips). Pool total about 145.0 MiB.
- If the engine uploads only the base level, the DXT total drops to about 108 MiB.

## 2048x2048 files (15)

F20, F21, F22, F28, F29, F30, F33, F34, M20, M21, M22, M28, M29, M30, M35.
Each is 5,592,581 to 5,592,591 B on disk. Each carries 21 to 31 B of trailing text after the mip chain (not padding):
- "mipmap 0 / xbox_downsample 256": 9 files (F20, F21, F22, F28, F29, F30, M20, M21, M22).
- "xbox_downsample 256": 6 files (F33, F34, M28, M29, M30, M35).

## Vanilla comparison

Not possible. `chitin.key` (`~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/chitin.key`) has 0 N_Comm* .tpc or .tga entries (18,439 resources scanned with pykotor `Chitin`). Baseline used instead: the pool majority, 1024x1024.

## Recommendation

1. Downscale the 15 files at 2048x2048 to 1024x1024. They become the same size as 46 of 69 TPCs; disk falls from about 83.9 MB to about 21.0 MB and VRAM falls by 62.9 MB (60.0 MiB, about 42% of pool VRAM).
2. Check the 9 "mipmap 0" files before re-encoding. The trailing TXI text may control mip upload, and the re-encode must keep it; a pykotor re-encode is not verified to preserve it.
3. Confirm in game at normal commoner camera distance before shipping. If there is no visible loss, the downscale is worth doing.

## OPEN

- Does the engine honor "mipmap 0" in the trailing TXI of those 9 files? If yes, their VRAM is about 4.2 MB each, not 5.6 MB.
- N_CommM10 has only a .tga (no .tpc). Confirm in game that the engine loads the TGA; not verified.
