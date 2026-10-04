# R10 batch 3a: level-up settings button icon (recon, 2026-10-04)

Tools: `list_icons.py` (list/render textures from TexturePacks + override, `--filter REGEX --max N --render a,b`), `check_rect.py X Y W H` (intersects rects with live character_p.gui controls from `../../phase4_evidence/S1_raw_character_p_controls.txt`).
Run with `.venv/bin/python3`. The default regex matches 1036 textures (mostly ip_* force-power art); use narrower filters such as `lbl_icn|lbl_levelup|arrow`.

## Candidates (all swpc_tex_gui.erf; no override copies)
| resref | size | preview | note |
|---|---|---|---|
| lbl_icn_opt2 | 32x32 | lbl_icn_opt2.png | grey-beige hammer, the main-menu "Options" tab icon (normal state); native GUI look, reads as settings. CHOSEN |
| lbl_icn_opt3 | 32x32 | lbl_icn_opt3.png | white hammer on black (selected/hover state of the same tab icon); good hover swap for opt2 |
| lbl_levelup | 128x128 | lbl_levelup.png | orange down-arrow "level up" glyph; reads as level-up but downscaled 4.6x and arrow points down |
| uibit_arrowpos_x | 32x32 | uibit_arrowpos_x.png | grey tag outline with white plus; generic "add/increase", looks like a stat-point button |
| uibit_uparrowo | 16x16 | uibit_uparrowo.png | gold up arrow; too small, blurry at 28 px |
Also rendered: lbl_icn_opt3p.png (128x128 outlined hammer, pressed art).

## Chosen
Default `lbl_icn_opt2` (32x32 draws crisply at 28x28 via SetRect; native style; "options" semantics matches a settings button). Hover/press: swap to `lbl_icn_opt3` (same size). Fallback if missing: `lbl_levelup`.

## Rect
Chosen: `[492,172,28,28]` (top-right of the 3D view, inside LBL_3DCHAR [295,134,230,336]; right margin 5 px to x=525).
Alternatives: bottom-right `[492,402,28,28]` free; `[492,140,28,28]` is free of controls but sits under LBL_BEVEL (cosmetic frame strip [293,130,234,38], y 130-168), so not preferred.

## Overlap check: PASS (units are the live 800x600 layout coordinates of the S1 dump)
- Expected containers (fully contain the rect, same area by design): LBL_3DCHAR [295,134,230,336], BTN_3DCHAR [295,134,230,336] (+ LBL_BEVEL [293,130,234,38] only for the y=140 alternative).
- No partial intersections with any of the 137 controls. Neighbours: BTN_AUTO [362,260,94,52] (ends x 456, y 312), BTN_LEVELUP [362,325,94,40], SLD_ALIGN [310,177,36,242] (ends x 346), LBL_LIGHT/LBL_DARK at x 304, LBL_FORCEMASTERY [294,105,232,21] (y < 126), LBL_BEVEL2 [293,436,234,38] (y >= 436, clear of the bottom alternative); x 536-752 is Mod 14's panel, x gap 525-536 untouched.
- Trap: BTN_3DCHAR (type 6) covers the whole 3D view, so the new button must be hit-tested ahead of it (create after it / higher in the list), otherwise clicks go to BTN_3DCHAR.
