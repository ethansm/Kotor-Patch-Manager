# R6: icon row above LBL_NAME (target action menu), placement recon

Static code + decomp (steam_6a522e71) + parsed enemy-effects-strip probe log. Read-only except this file.
Decomp: `decomp/<2 hex>/<addr>.c`. "px" = engine display px (int32), origin top-left, y down.

## 1. GuiKit label API (`patch_manager_mods/_shared_src/GuiKit.h`)
- `createLabelFromTemplate(panel, "LBL_QUEUE1", &why, &badVt)` :178. BUILD-only (layout open). Returns hidden + click-through label.
- `appendToPanel(panel, ctl)` :212. Panel list at +0x24 / count +0x28; append order = draw order.
- `setRect(ctl, x, y, w, h)` :241. px, via vtable slot 1. `getExtent(ctl, &x,&y,&w,&h)` :227 reads back control-local px.
- `setVisible(ctl,bool)` :260, `isVisible` :267, `setClickable(ctl,bool)` :251. Flag bit 0x2 visible, bit 0x20 click-through.
- `setFill(ctl, img, const float* tint=0)` :345. Image at border+0x74, tint rgb at border+0x28.
- `imageFor(resref, bool* loaded=0, int* pool=0)` :334 (`imageForRetry` :340). Lower-cased, 16 chars, static pool of 256.
- `setAlpha(ctl,float)` :370 / `getAlpha` :375 (border+0x24). `setBorderTint(ctl, rgb[3])` :363 / `getBorderTint` :354.
- `setFillStyle(ctl, 0..3)` :381 (tile / centre / stretch). `setText` :273, `setFont` :282, `setAlign` :289, `setTextColor` :302.
- `scaleFromRef` :392, `unitRect` :404: layout-unit helpers. Not needed if sizing from live px.
- Tint "recolours the whole frame" (GuiKit.h :358-362), so it also tints the fill image. Neutral tint = white is INFERRED.
- No destroy helper (QueueHud.cpp:468 comment). Create the full pool once at BUILD.
- Per-frame cost: GuiKit caches nothing. Consumers gate: `widRect/widFill/widAlpha/widTint`
  (`combat_hotbar_src/CombatHotbar.cpp:338-352`). `imageFor` is a linear strcmp scan (GuiKit.h:317-332): cache the pointer.
  Engine cost of SetRect per call: UNPROVEN.

## 2. Clipping: is a label at negative menu-local y drawn?
- Target menu draw `FUN_00743280` (`decomp/74/00743280.c`): gate `menu+0x1BAC & 1`, then
  PushClipRect(menu+0x1648, +0x164C, +0x1650, +0x1654, clip-only colour), draws +0x18F8, +0x17B0,
  +0x1668 (LBL_NAME), +0x1A40 (PB_HEALTH), 3x `FUN_007414D0`, PopClip.
  It walks no child list and does not call base draw 0x40EC90.
  => Any control at menu-local y < 0 is CLIPPED. A label appended to the menu object is never drawn.
- Base `CSWGuiPanel::Draw` 0x40EC90 (`engine_research_findings/V8b.md:81-88`): skips the whole panel
  when flag bit6 is clear, x<0, y<0, w<=0, h<=0, x+w>W or y+h>H (W,H = GUI manager +0x6C/+0x6E).
  Fill = 0x479460 = PushClipRect, which calls `glViewport(x, vpH-y-h, w, h)` (V8b.md fill block).
  Children draw inside that viewport. Fill rejected => children skipped (`0040EDA4`).
- HUD draw `FUN_0074B3F0`: `this` saved at `0074B418`. Base draw `0040EC90` at `0074C1B2` with ECX = hud,
  walks hud+0x24 list (mod labels). THEN the menu draw at `0074C1CA` with ECX = hud+0xCC (`ADD ECX,0xCC`).
- HUD rect, measured: BUILD48 dump `panel=0905f8a0` +0x04..+0x10 = (0, 0, 0xF00, 0x640) = (0,0,3840,1600)
  (`combat-hotbar_log.txt`, parsed). HUD clip = full screen. Count at +0x28 = 111 (0x6F) at BUILD.
- Precedents for absolute px above a card: `queue_hud_src/QueueHudLogic.h:275-276` (strip y = card.y - 2 - s);
  CombatHotbar.cpp:568-570 (`boxes = menu origin + menu-local extent`, absolute, then setRect on HUD labels).
- Verdict on the question: menu-local negative y = CLIPPED. HUD-list label at absolute y = DRAWN.

## 3. Origin units, clamp, aspect
- `FUN_00743850` writes +0x1648 / +0x164C as int32 (`decomp/74/00743850.c:72-76, 109-111`).
  Input: `FUN_0077E970` (world to screen; returns px, scaled by GUI manager W/H, `decomp/77/0077e970.c` tail)
  rounded by `FUN_0091C860` (float to int).
- Centred case (:109-111): x = round(px) - w/2; y = round(py) - 32 - h. Panel sits 32 px above the target point.
- Clamp (x +0x1658/+0x1660, y +0x165C/+0x1664) on every path (:167-212):
  x in [cx, cx+cw-w], y in [cy, cy+ch-h]. Clamp min y = 106 means icons above the name never go above ~80 px.
- cVar3 == 2 pins x to a clamp edge (:72-76). Meaning of 1 vs 2 (from `0077E970` local_19): UNPROVEN.
- Freeze (:46-50): if (+0x1BAC bit1) or (timer +0x1BA0 > const), only y is re-based.
- GUI manager W/H (+0x6C/+0x6E) are written only by SetScreenSize 0x411A80 (`engine_research_findings/V7.md` Q1).
  RescalePanels 0x413DD0 runs after it. So panel rects are display px, not 640x480 units.
- Ultrawide: the coordinate path has no aspect transform. The aspect trap is GUI projection flag `0x00A1B9F0`
  (`GameAddr.h:20`): when nonzero, InitControl applies a non-linear aspect projection to extents
  (`07_force_power_hotbar.md:778`). CombatHotbar clears it around BUILD binds only (`CombatHotbar.cpp:365-371`).
  `CombatLogic.h:66-70` uses a 4:3 safe area (safeW = H*4/3) on wide screens. At 3840x1600: x in [853, 2986].
  Ultrawide docs (memory `kotor2-ultrawide-gui`): art upscales, 16:9 content layers, one map X-scale hook.
  No target-menu rect change found (UNPROVEN: none).

## 4. Probe values (`enemy-effects-strip_log.txt`, parsed via kplog_parse, 8 logged samples)
- menu panel: x 1384..2770 (last 1566); y 780..847 (last 780); w 533; h 51.
- clamp: x 7, y 106, w 3826, h 1392 => x in [7,3833), y in [106,1498).
- LBL_NAME local: x 0, y 2, w 533, h 32 => absolute top = menuY + 2.
- PB_HEALTH local: x 0, y 35, w 533, h 16. hp max/cur 32..46.
- flags +0x1BAC = 0x0F (bit0 = 1, bit1 = 1) in all 8.
- Float view of the same dwords = 0.00 in all 8 (denormal). Int32 is the sane reading.
- Anomaly: x and y change between samples with bit1 = 1. By :46-50 the freeze branch should not move x. OPEN.

## 5. Plan (for the strip, P2)
1. K2HUD_BUILD: pre-create N icon controls (N = MaxIcons, plus drain overlays): `createLabelFromTemplate(panel,"LBL_QUEUE1")`
   + `appendToPanel` + `setVisible(false)` + `setClickable(false)`. No later creation (no destroy).
2. Each TARGET_MENU_POST: nameRect = `getExtent(menu+0x1668)`; origin = menu+0x1648/+0x164C.
   Row y = menuY + nameY - 2 - iconH; row x centred on name, clamped to [0,W) and to the 4:3 safe area.
   Change-gate setRect. Show only when menu+0x1BAC bit0 is set and the target has effects.
3. Icon art: `setFill(ctl, cachedImageFor(resref))`. Drain: setAlpha on the overlay. setClickable(false) on all.

## OPEN
- Freeze anomaly (section 4): why x moves while bit1 = 1. Needs a PRE-hook read of +0x1BAC, or a log of +0x1BA0.
- SetRect cost per call, and whether it applies the aspect projection (ProjGuard only covers InitControl).
- Tint semantics (is white neutral?) and whether the LBL_QUEUE1 frame draws around an icon (needs a frame-less template or alpha).
- Whether a resolution change rebuilds the HUD (K2HUD_BUILD refire) or only re-lays out panels.
- Meaning of cVar3 1 vs 2 in FUN_0077E970 (local_19).
