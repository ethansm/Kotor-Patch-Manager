# G: what a menu-ui label can draw (U4 viewport markers)

Recon, read-only. Static reads of repo sources, the stock Gui.bif layouts and the dialogfont16x16 atlas (via pykotor in `.venv`). No hook, no REA, no live run.

## 1. Font used by AddLabel / AddButton
- AddLabel clones template **LBL_TITLE**: `patch_manager_mods/mod_config_src/ModConfig.cpp:70` (kLabelTemplate), `:185` (cloneLabel), `:984` (muiAddLabel).
- AddButton clones template **BTN_GAMEPLAY**: `ModConfig.cpp:961` (muiAddButton), `:970` (cloneEntry). Falls back to a label when `button_clone=0` (`mod_config.ini`).
- Template values, read from `optionsingame_p` and `optionsmain_p` (Gui.bif):
  - LBL_TITLE: font `dialogfont16x16`, align 18 (centre), colour (0.800, 0.800, 0.698) cream. Border has empty FILL, EDGE and CORNER, so no frame is drawn.
  - BTN_GAMEPLAY: font `dialogfont16x16`, align 18, colour (0.102, 0.698, 0.549). Draws a frame (`uibit_brdr_16we` / `16wc`), so it is unsuitable for markers.
- Neither ModConfig.cpp nor holster_editor_src calls SetFont, SetAlign or a colour setter. Items inherit the template font and colour.

## 2. Glyph coverage of dialogfont16x16
- Atlas: 256x256 DXT5, 16x16 cells, cell index = byte. Cell boxes come from the TXI upperleft/lowerright lists (`scripts/character_page_mockup.py` `font()`). The raw decode is stored bottom-up, so I flipped it vertically before inspecting. Render kept at `scratchpad/u4g/atlas16_flipped_grid.png`.
- 0x20-0x7E is ASCII. Lowercase a-z draw CAPITAL-shaped glyphs (no lowercase set), so "x" looks like "X".
- 0x00-0x0F, 0x17-0x1F, 0x7F: blank or tofu boxes.
- 0x10-0x16: coloured controller icons (A, B, X, Y, circles, a left triangle). Not text; a colour tint would modulate them. Do not use.
- 0x80-0xFF: Windows-1252 / Latin-1 layout (euro, quotes, bullet, dashes, letters, symbols).
- Usable non-ASCII: `•` 0x95, `°` 0xB0, `±` 0xB1, `×` 0xD7, `÷` 0xF7, `·` 0xB7, `«»‹›` 0xAB/0xBB/0x8B/0x9B.
- Absent: diamond U+25CA, arrows U+2190-2193, triangles. No arrow glyph exists.
- Widths for 0x80-0xFF are real TXI boxes (`_shared_src/FontWidths.inc:6` onward), so `textfit::textPx` measures them.
- Repo policy: `scripts/gen_effect_name_table.py` lines 17 and 35 reduce names to ASCII (NFKD, ascii-ignore). The docstring says names the font "cannot draw" are reduced. Engine drawing of bytes >= 0x80 is NOT verified in game.
- Source encoding: `rg -P "[^\x00-\x7F]"` finds no non-ASCII bytes in holster_editor_src, ModConfig.cpp, GuiKit.h or TextFit.h. A literal "°" in C++ depends on the compiler source encoding, so use `\xB0` escapes if one is ever needed.

## 3. Colour per item
- K2MenuUiApi has no colour or font setter. Fields: NewPage, AddButton, AddLabel, AddCustom, Clicked, SetText, SetRect, SetDesc, PaneRect, Control (`_shared_src/ModConfigApi.h:85-103`).
- `Control(page, item)` returns the engine control pointer, or nullptr after unlink (`ModConfig.cpp:1063-1068`). HolsterEditor already uses it (`holster_editor_src/HolsterEditor.cpp:1277`, `:1931`).
- `guikit::setTextColor(ctl, r, g, b)` (`_shared_src/GuiKit.h:302`) writes the label text object colour through `g_gk.textColor` = 0x00417140 (`_shared_src/GameAddr.h:31`). Gates: `textObjOk` (`GuiKit.h:140`) and readback `getTextColor` (`GuiKit.h:295`). Floats 0..1.
- Works on AddLabel items. Sibling helpers `setFont` (`GuiKit.h:282`) and `setAlign` (`GuiKit.h:289`) use the same path.
- Static fact: 0x417140 is 50 B, RET 4, calls 0x417010 CSWGuiTextParams::SetColor (`static_facts.py addr 0x417140`). Hook-free and already in use (Mod 14).
- Buttons keep their text object at btn+0x160 (`_shared_src/GuiButton.h:24`). The label gate fails on buttons, which is safe. Not needed for markers.
- Result: red, green and blue markers are feasible via `Control()` plus `setTextColor`, with a readback check.

## 4. Minimal-risk marker design (ASCII only)
- Glyphs: bone anchor `+`, mount point `O`, axis tips `X` (x), `Y` (y), `Z` (z). All present in the atlas and all ASCII.
- Colours: X red, Y green, Z blue, anchor and mount in cream (template default) or white.
- Box size: k = H / 800 engine px per texel, where H = screen height (0.75 x H/600; `TextFit.h` textPx). Box is `kFontTexW[c] * k` wide and `16 * k` tall.
  - At H = 1600 (k = 2.0): `+` 24x32, `O` 24x32, `X` 24x32, `Y` 24x32, `Z` 22x32.
- Rect: W = max(2 x glyph width, glyph width + 8), H = 16k. Centre the rect on the point: (px - W/2, py - H/2, W, H). Align 18 keeps the glyph centred.
  - Two-glyph minimum width: labels narrower than two glyphs lose text silently (`patch_manager_mods/16_candidate_register.md:35`, L3).
- Ink nudge, from the atlas, relative to the box centre (texels, then px at H=1600). Shift the rect by this to centre the ink:
  - `+`: ink offset (-1, +1) texels, so nudge (+2, -2) px.
  - `O`, `X`, `Y`: ink offset (0, +1.5), so nudge (0, -3) px.
  - `Z`: ink offset (-0.5, +1.5), so nudge (+1, -3) px.
  - Assumes align 18 centres the 16-texel box vertically. Verify in game.
- Avoid: `° ± × •` (non-ASCII coverage and source-encoding risk), arrows and diamonds (absent), AddButton for markers (framed, teal text).
- Optional: set click-through (ctl+0x48 bit 0x20, `GuiKit.h:50`) so markers do not steal the view drag. GuiKit has no setter for this bit; it would be a direct write.

## 5. Gaps and checks
- Engine draw of bytes >= 0x80 is unverified in game. Keep markers ASCII to avoid it.
- Vertical alignment of align 18 is unverified. Nudges are derived from the atlas.
- Scale assumes H = ScreenH / 600 (`scripts/character_page_mockup.py:43`; `TextFit.h` 19:09 screenshot at sy 2.6667).
- Suggested live check: a PLAY_TEST_QUEUE row. Place `+` on a known point and X, Y, Z in red, green and blue. Pass signal: each glyph centred on its point at the current resolution.
