# QL3 T3: Cancel button colour vs lbl_miqueue_teal holder (recon)

## 1. TGA parse: patch_manager_mods/queue_hud_src/override/lbl_miqueue_teal.tga

- Header: 512 x 256, bpp 32, type 2 (uncompressed truecolor), no colour map, no image ID, descriptor 0x08 (bottom-left origin, 8 alpha bits). TGA 2.0 footer present. File size 524332 = 18 + 524288 + 26.
- Non-transparent px (alpha > 0): 71222 of 131072 (54.3%). Alpha 255 px: 4723. Alpha 0 px: 59850.

Mean RGBA, all non-transparent pixels ("measured mean"):
- 0-255: (8.36, 59.49, 46.48, 132.18)
- 0-1: (0.0328, 0.2333, 0.1823, 0.5183)

Border ring (outermost opaque px, 4-adjacent to alpha 0 or image edge), n = 4590:
- 0-255: (17.07, 131.42, 102.80, 21.64)
- 0-1: (0.0670, 0.5154, 0.4031, 0.0848)
- The ring is a feathered edge. Its colour is sea green (matches doc 33 border {0.067, 0.514, 0.404}). Its alpha is low.

Radial profile (distance from the transparent edge):
- Depth 1-4: colour about (17, 132, 103), alpha about 0.32.
- Depth 6-11: colour fades toward black, alpha rises.
- Depth 8 and deeper (interior, n = 43293): straight mean (2.95, 17.29, 13.45, alpha 172.5). Most common colour (0,0,0) at alpha about 180-183 (0.71).

Conclusion: the holder body interior is black at alpha about 0.70. The teal or green appears only in the soft rim. The measured mean is pulled toward green by the rim. Doc 33 (black fill, alpha 0.70) matches the interior.

## 2. Commit 2c071d8, cwLookLeader (QueueHud.cpp:404-421)

- Old: border tealBorder {0.10, 0.58, 0.62} (hover {0.15, 0.75, 0.80}); fill darkTealFill {0.05, 0.25, 0.28}; alpha 0.90.
- New: border seaGreenBorder {0.067, 0.514, 0.404} (hover {0.10, 0.72, 0.57}); fill blackFill {0, 0, 0}; alpha 0.70.

## 3. Shared tint field (root cause of the black look)

- setFill(img, tint) writes the same border tint field as setBorderTint (_shared_src/GuiKit.h:312 vs :328). Border and fill share one RGB.
- setAlpha (GuiKit.h:331) is one alpha for the whole frame.
- cwLookLeader calls setBorderTint(seaGreen) at QueueHud.cpp:415, then setFill(blackFill) at QueueHud.cpp:418. The fill tint overwrites the green border, so the frame renders black whenever uibit_fill_2wt loads.
- Before 2c071d8 the same overwrite made the frame darkTealFill (dark teal, alpha 0.9). The hover tint is overwritten in both versions, so hover has no visible effect when the fill loads.

## 4. Candidate comparison

Method: source-over composite onto a grey backdrop, Euclidean RGB distance (0-255). Two references: whole-panel composite and interior (depth 8+) composite. Scratch scripts are in the scratchpad ql3/t3/ directory.

| Candidate | RGB (0-1) | alpha | Hue (deg) | dRGB whole, bd black / grey | dRGB interior, bd black / grey |
|---|---|---|---|---|---|
| Measured panel mean | 0.033, 0.233, 0.182 | 0.52 | 165 | 17 / 17 | 29 / 62 |
| qh::kTealBg | 0.05, 0.25, 0.28 | 0.45 | 188 | 22 / 36 | 33 / 81 |
| darkTealFill (pre-2c071d8) | 0.05, 0.25, 0.28 | 0.90 | 188 | 65 / 41 | 76 / 40 |
| Current blackFill | 0, 0, 0 | 0.70 | n/a | 22 / 61 | 11 / 16 |

Holder reference hue: interior 163, rim 165.

Reading:
- Best match to the whole panel (the teal or green look): measured panel mean.
- Best match to the black interior: current black at 0.70.
- kTealBg is cyan-leaning (188 deg vs 165 deg holder hue) and too blue.
- darkTealFill at 0.9 is too bright and too opaque.

## 5. Recommendation

- Frame (fill and border share one tint): rgba(0.033, 0.233, 0.182, 0.52). This is the measured panel mean.
- Border rgba: cannot be set separately with the current API. Optional hover: (0.067, 0.514, 0.404), alpha 0.52, which is the rim colour and gives a visible hover step.
- Code fix: pass the same tint to setBorderTint and setFill (or remove the setBorderTint call), and set alpha to 0.52. Otherwise the last setFill tint wins again.
- Alternative if the user wants the exact holder look (black body, green rim): needs a second control or a dedicated fill texture. Neither is in the repo.

## Open

- uibit_fill_2wt is not in the repo. The recommendation assumes it is near-white so the tint equals the rendered colour.
- It is also unknown whether setAlpha 0.52 fades the cream CANCEL QUEUE text. One in-game look settles both.
