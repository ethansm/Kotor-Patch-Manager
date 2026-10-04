# R2: companion "heal self" portrait button (static research, 2026-10-04)

Paths are relative to patch_manager_mods/. Nothing was run in game; every pixel figure is from probe logs in 08_shield_bar.md.

## 1. Coordinate spaces

- Slot rects are read from the portrait slot object: `Slot_CardRect` +0x54C, `Slot_VitRect` +0x694, `Slot_ForceRect` +0x800, each 4 ints (x, y, w, h). Source: `_shared_src/K2Addr.h:107-112`, values at :250-255 (Steam) and :405-410. Read pattern: `shield_bar_src/ShieldBar.cpp:481` and :583.
- These rects are already SCREEN PIXELS of the game client area. Evidence: 08_shield_bar.md:297-299 ("full-HUD pixels 3840x1600", small vit `(3671,1285,47,96)`, card x 3711 w 85). The HUD frame has an empty clip stack, so draw origin = (0,0) and the same numbers feed `gui2d::drawImage` / `fillRect` directly (ShieldBar.cpp:564-585 passes `v[0..3]` straight in).
- Control extents (`ctl+4/+8/+0xC/+0x10`, `guikit::getExtent`, `_shared_src/GuiKit.h:197`) are also screen px and are compared with `GetCursorPos` + `ScreenToClient` (RuntimeGuiControls.md:37-40, :80-82). So rect space = hit-test space = draw space: no conversion needed for a button anchored to a slot rect.
- Do NOT use the hotbar's aspectFix scaling (`action_hotbar_src/ActionHotbar.cpp:1318`, `hb::makeScale`/`unitRect`, `HotbarLogic.h:187-199`). That is for the hotbar, which sits left-anchored: x_px = x_u*(H/600) via `sx*c`. The portrait cards come from the ultrawide .gui override and are RIGHT-anchored: 737 u -> 3671 px at 3840x1600 = W + (737-800)*H/600. A GuiRefW/H (800x600) conversion would put the button on the wrong side of the screen.
- Pixel-per-unit for sizing: derive it from the slot, not from `GuiRefW/H` or `gui2d::hudScale()` (hudScale logged 2.0 at 1600p while real px/unit = 2.667): `pxu = vit.h / 36.0` (small card PB_VIT is 18x36 u; 96 px / 36 = 2.667). At 4:3 600p this gives 1.0.

## 2. Free space and proposed rect

Layout in units (08_shield_bar.md:186-205): small cards 2/3 are `PB_VIT` x 737 (18x36) | back 750 (36x36) | `PB_FORCE` 781, at y 482 / 443. Shield bar draws in `[vx, vx+0.445*vw]` (width 0.55*vw, ShieldBar.cpp:162, :247-254), i.e. ON TOP of the vit rect's transparent left padding, so it does not extend left of `vit.x`. Buff icons are drawn INSIDE the card rect (buffs on the left edge, debuffs on the right edge; BuffDurationHUD.cpp:2-4), i.e. right of the vit rect. So everything is right of `vit.x`; the strip left of `vit.x` (x < 737 u, y 443-518 u) is free; the only native thing there is the invisible `LBL_ARROW_MARGIN` (3,129 794x333, 08_shield_bar.md:205).

Proposal (identical formula for both companions, SlotAt entries that are not the leader):

| slot | rect source | proposed button rect (px, same space) | scaling |
|---|---|---|---|
| leader (excluded) | `Slot_CardRect` w >= 150 px or `c->isLarge` | none | n/a |
| companion 2 (small card, y ~1285 at 1600p) | `Slot_VitRect` via `SlotAt(i,&slot,&cr)` then `readAt(slot, T->Slot_VitRect+4k)` (ShieldBar.cpp:583) | `s = clamp(round(vit.h*0.25), 16, 24)` ; `x = vit.x - gap - s` with `gap = max(2, round(2*pxu))` ; `y = vit.y + (vit.h - s)/2` | none for position (live px); size from `pxu = vit.h/36` |
| companion 3 (y ~1181 at 1600p) | same | same | same |

Numbers: at 3840x1600, vit.h = 96 -> s = 24, gap 5 -> x = 3671-5-24 = 3642, y = vit.y + 36. At 1080p (pxu 1.8, vit.h 65) -> s = 16. In 800x600 units the button is about 9x9 u, well inside the free strip (strip is >= 36 u tall, 737 - ~700 wide). Cap `s` at 24 so it never reaches the neighbouring card (cards 2/3 are 104 u apart vertically).
Guards: skip when `vit.w`/`vit.h` outside 1..400 (ShieldBar.cpp:584), when `creature == 0` (4th slot has zero rects), and when slot is large.
Menu frames: do not draw (HUD_DRAW fires only on the HUD frame).

## 3. Input

Recommended: `K2PS_FRAME_BEGIN` callback (leader call, once per frame, `PortraitSlots.h:15-16`) polls `guikit::clickEdge` (GuiKit.h:370-378: `GetAsyncKeyState(VK_LBUTTON)` edge AND game window foreground) with the cursor from `GetCursorPos`+`ScreenToClient` (hotbar `cursorInGame`, ActionHotbar.cpp:735). Hit-test against the rects computed from the same frame's slot table (compute rects once in FRAME_BEGIN, cache for HUD_DRAW). Gate it like the hotbar (`g_hudGateOk`, HUD is the active panel, no dialog/menu/pause): FRAME_BEGIN also fires in menu frames, so check `ctx->inMenu == 0` plus the hotbar-style gate.
Hotbar-style runtime controls are the alternative, but they require `createLabelFromTemplate` while the HUD layout template is still open (ActionHotbar.cpp:1338, 1731), i.e. inside hook 0x748502. Not available to us.

### Click-through / double action (main risk)
- A runtime label with bit5 (0x20) CLEARED is how the hotbar absorbs a click: the label hit-test (0x00418D20) finds it, it has no handler, and the click never reaches the world (ActionHotbar.cpp:699, 971; RuntimeGuiControls.md:94-101). Slot clicks then fire from the polled edge only.
- A polled button over EMPTY HUD space has no engine control under it, so the same press is also delivered to the engine. Expect the world click to run (move/attack/target at that screen point, or whatever the 3D pick finds); whether the strip is covered by an engine control is UNKNOWN: `LBL_ARROW_MARGIN` is invisible (probably click-through, flag unverified). It is not the portrait select: the select is on `BTN_CHAR` (card rect, `Tip_Slot` +0x1D0), which our strip does not overlap.
- Can a portrait-slots consumer swallow it? No. The registry has no input phase and no consume return value (`PortraitSlots.h` phases :16-26, callback returns void). Hotbar swallows KEYS by rewriting the input record at 0x0072CB63 (`CaptureHotbarKey`, ActionHotbar.cpp:6, 1699-1705, `dwOfs=0`); that hook is hotbar-owned, so we cannot add a second patch there.
- Mitigations (pick one, test live): (a) cheapest: native-control recycling: from FRAME_BEGIN, move an existing always-present native HUD label to the button rect with `guikit::setRect` (GuiKit.h:211, same SetRect shield-bar uses on PB_FORCE, ShieldBar.cpp:489-495) and clear its bit5 via `guikit::setClickable(ctl,true)`. Needs the HUD panel pointer, which is NOT derivable from the slot (K2Addr only has `Menu_PortraitSlot` for the menu); the only source is the hotbar's hook 0x74B3F0 (`HotbarHudTick(hud)`), so this needs a shared service. (b) proper: hotbar owns two extra `Layout[]` labels (LBL_SELFHEAL2/3, doc 22 s4.3 `k2.hud@1` "hud-host" design) that the self-heal mod positions each frame from the slot rects; absorbs the click natively and gives hit-test + tooltip for free. (c) accept the world click in v1 and document it (low blast radius if the leader merely walks; worse if it attacks).

## 4. Draw and tooltip

Recommended draw: register `K2PS_HUD_DRAW` (leader call, clip stack empty, draw ALL slots there, `PortraitSlots.h:23-25`), priority > 200 so it draws after shield bar (100) and buff HUD (200). Use `gui2d::flush(gui2d::flushArg())` then `gui2d::fillRect` (Gui2D.h: `fillRect`, `pushClip` with colour) for a flat button face/border and 1 px glyph strokes (cross); flat colours only: textured fx draws caused flicker and a black scene (08_shield_bar.md:301-303). A TGA icon via `gui2d::drawImage` + `imageFor` (ShieldBar.cpp:237, :258) is fine for plain TGAs (08_shield_bar.md:303 plan). Wrap with `CapGuard` like ShieldBar.cpp:572. Runtime labels (hotbar) are not an option (see 3). Pitfall: later slots' icon rows leak clip state (PortraitSlots.h:24), so draw from the leader call only.
Hover feedback: poll hover in FRAME_BEGIN and brighten the fill; play `guikit::playHoverSound` (GuiKit.h:380) on enter.
Tooltip: `GuiTip.h` is control-based (writes the CExoString at ctl+0x2C, strref ctl+0x28; engine draws the box, plus a custom two-label fallback). Both need a GuiKit control, so it only works with option (b). Without a control there is no drawing API for text in `Gui2D.h`. Workable fallbacks: (1) reuse the portrait's native tooltip path: FUN_00744c20 builds text into `charButton+0x1d4`, hooked at 0x00744EEC by shield-bar (ShieldBar.cpp:626-645); appending a "Click the heal icon ..." line is a shield-bar change, shows only over the portrait, not our icon; (2) no tooltip, rely on a recognisable icon + hover brighten.

## Recommended path
HUD_DRAW flat draw + FRAME_BEGIN `clickEdge` polling, gated, with rect anchored to the live `Slot_VitRect`; add click absorption via hotbar-owned label (option b) when the hud-host service exists, else ship v1 with the world-click risk flagged.

## Open risks
1. World click double action (above); first live test: click the strip with a distant walkable point and watch the leader.
2. Which slot index is a companion: `SlotAt` gives no `isLarge`; capture it in `K2PS_SLOT` (`ctx->isLarge`) or use the vit.w < 70 rule (ShieldBar.cpp:247).
3. Party of 2: 4th slot / empty slot has creature 0 and zero rects, skip; stale entries > 500 ms are dropped by SlotAt.
4. Hit-test uses screen px but the `Slot_*Rect` freshness: rects are valid only after the engine laid the slot out; read them in the same frame.
5. Gate: no existing shared "HUD active" predicate; hotbar `g_hudGateOk` is private (doc 22 s3 `GameplayActive`/`k2.enginestate`).
6. Shield bar FORCEMOVE slides only the LEADER force bar; companion vit/force unaffected, so the strip stays free. A future shield-bar change to shift companion bars would need this rect to follow `vit.x`.
7. Tooltip impossible for a pure-draw button (no text API); verify live before promising one.
8. Priority/registration: KPM allows several patches per `k2.portraitslots` phase; no new hook address needed.
