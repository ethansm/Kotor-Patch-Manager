# R2: Buff HUD drain, reuse for a target-effects strip (recon, read-only)

Steam 6A522E71. Paths relative to the worktree. UNPROVEN = not seen in code or docs.

## 1. Icon size, tile art, debuff vs buff
- Size: `int(cardW * 0.22 | 0.19)` (large | small card), clamped to `[5, 24] * hudScale` (BuffHudLogic.h:95-96, 105-112). Scale 1 = 5-24 px, scale 2 = 10-48 px (comment BuffHudLogic.h:96). Card width at runtime UNPROVEN (engine [ebp-8]).
- Shrinks to fit card height, else adds a column (max 3). Both columns share one size (BuffHudLogic.h:113-126; BuffDurationHUD.cpp:250-251).
- Frame: none. Backing disc `buffhud_circle` (white, tinted BgColor 0.035/0.415/0.34, alpha 0.80 x 0.35), item abilities only (BuffDurationHUD.cpp:50-52, 293-295). No HudKit frame/disc use in buff HUD.
- Column: `debuff = whichRow == 0` (BuffDurationHUD.cpp:238). Debuffs = right edge, buffs = left edge (BuffHudLogic.h:133-137).
- Icon table: debuff uses `kDebuffIcons[spell]`, plus `EffectIconRowTable` for script type-0x43 effects; buff uses `kEffectIcons[spell]` (`iconForSpell` EffectIcons.h:17, `collectBuffs` :40).
- Colour: no buff/debuff colour in code. Tint and alpha come from the engine arrow widget (`Border_Tint`, `Border_Alpha`, BuffDurationHUD.cpp:274-278), white/1.0 when iconWidget == 0 (menu). Row tint values UNPROVEN.

## 2. How the drain is drawn (dim base + clipped bright slice, not an overlay)
- Per icon (BuffDurationHUD.cpp:293-310):
  a. Full-square base at alpha `DimAlpha` 0.35 (line 296; constant at line 49).
  b. `bright = brightPx(size, ratio) = int(size*ratio + 0.5f)` (BuffHudLogic.h:142-146).
  c. `pushClip(ix, y0 = iy+size-bright, size, bright)` (line 299), redraw image at `(0, -(size-bright))` alpha 1.0 (line 302), `popClip` (line 303).
  Net: bottom `bright` px at full alpha, rest at 0.35.
- `Gui2D.h` `drawVFill` (164-171; int form 173-180) is the same geometry. The buff HUD does NOT call it. It is used by shield bar `drawGauge` (ShieldBar.cpp:413), called from `onMenuOverlay` (ShieldBar.cpp:535, 578-579).
- Engine draw site: row-exit hook `0x00745AEC` = `DrawBuffColumn` (kotor2-steam-aspyr.hooks.toml, buff HUD). It sits in the portrait row function `FUN_00745770` = `CSWGuiMainInterfaceChar::DrawEffects` (21b_gog_address_map.md:89).
- Draw primitives (Gui2D.h): image vtable slot 6 (Engine.draw, line 71); PushClipRect `0x479460` with the no-fill colour (pure clip, line 93); Begin/End batch `0x4792C0`/`0x4793F0` (header 11-12); FlushGuiBatch `0x47EA60` (line 91).
- GL state: `CapGuard` snapshots and restores 9 caps: 0x0C11 scissor, 0x0B90 stencil, 0x0B71 depth, 0x0B50 lighting, 0x0BE2 blend, 0x0B60 fog, 0x0DE1 tex2d, 0x0B44 cull, 0x0BC0 alpha (Gui2D.h:222-223). Draw blocks run under it (BuffDurationHUD.cpp:273).
- Needs an engine draw hook: YES for this exact look. Gui2D draws need an open 2D batch and an empty clip stack (HUD frame; Gui2D.h:11-13, 106-113 `clipDepthRaw`/`inMenuFrame`; PortraitSlots.h:19).

## 3. Call sequence and inputs
1. Engine row draw reaches 0x745AEC: `DrawBuffColumn(slotPtr, iconWidget, isLarge, whichRow, cardW, cardH)` (BuffDurationHUD.cpp:316-326). cardW/cardH = viewport locals [ebp-8]/[ebp-4]; origin = card top-left (BuffDurationHUD.cpp:233-235).
2. `drawColumnCore` (237): `currentTime` (Effects.h:73) -> `collectBuffs` for both columns (EffectIcons.h:40) -> `effectRatio` per effect (Effects.h:95).
   `effectRatio` = remaining/(duration*1000), clamped to [minRatio, 1] (Effects.h:87-92). Returns 1.0 when there is no duration or no time.
3. `hudScale` (Gui2D.h:105) -> `columnLayout` (BuffHudLogic.h:105) -> `iconPos` (133) -> `visColumn` (BuffDurationHUD.cpp:135).
4. `CapGuard` (222), `flush(flushArg)` (Gui2D.h:85-91), per-icon draws (section 2), `flush` again.
- Hook context: only card w/h, slot, icon widget, row, isLarge. No clip rect or viewport is passed. The clip is the engine's current card viewport, not read by code.
- Menu frames: the provider pushes the panel clip (PortraitSlots.h:16-18); buff HUD pushes its own at BuffDurationHUD.cpp:491-493.

## 4. Can it run from K2HUD_TARGET_MENU_POST?
- hud-host phases (HudHost.h:36-39): FRAME 0, TARGET_MENU_PRE 1, TARGET_MENU_POST 2, BUILD 3. None is a draw-time phase.
- FRAME/PRE/POST run inside PopulateMenus 0x743540 every frame while a leader exists (31a_target_menu_post_re.md:26). POST sits at the tail 0x743847 (HudHost.h:15; hud-host hooks.toml). BUILD = 0x40F5A0, once per HUD layout load (HudHost.h:17, 39).
- POST context: `ctx.target` = target object pointer, -1 type for none (HudHost.h:56-57). POST runs only if a PRE was seen for the same menu (HhDispatch.h:124-131).
- Reading the target's effects works from POST: `effectListNonEmpty` and `currentTime` take any creature pointer (Effects.h:63, 73).
- Labels work from POST: create in BUILD (ctx.panel valid, HudHost.h:17-19), update in POST. Pattern: CombatHotbar.cpp:377-420 (onHudBuild), 885-887 (registrations).
- Gui2D draws at POST: NO evidence. POST runs before target-menu Draw (31a:89). Clip-stack and batch state at POST UNPROVEN.
- If Gui2D is required: a new draw hook is needed. Candidate: target-menu Draw `0x743280` (31a:5, medium confidence), via hud-host. K2PS_HUD_DRAW (portrait-slots, hook 0x745774) runs with an empty clip (PortraitSlots.h:19), but K2PsContext has no target field, so the target must be read from menu+0x1644 (39_enemy_effects_strip.md U1, UNPROVEN here).
- Plan delta: 39_enemy_effects_strip.md section 2 and P2 name `drawVFill` for the drain. That needs the Gui2D route above. Use labels instead (section 5).
- Plan delta 2: 39 cites the target hook at `0x00743850`. POST has moved to the tail `0x743847` (HudHost.h:15, hud-host 0.4.0).

## 5. Label-only drain (cheapest)
- Create per icon in BUILD via `createLabelFromTemplate(panel, "LBL_QUEUE1")` + `appendToPanel` + `setVisible`/`setClickable(false)` (CombatHotbar.cpp:393-417; GuiKit.h:178, 212, 251, 260).
- Icon label I: `setRect(x, y, s, s)` (GuiKit.h:241), `setFill(icon, white tint)` (GuiKit.h:345), `setAlpha(1.0)` (GuiKit.h:370). Cache each call, as CombatHotbar.cpp:336-346 does.
- Dark overlay O, created after I. The author says creation order draws underneath (CombatHotbar.cpp:18-19). UNPROVEN live.
  O: `setBorderTint(0,0,0)` (GuiKit.h:363), `setAlpha(0.65)` = 1 - DimAlpha (BuffDurationHUD.cpp:49).
  Each POST: `setRect(x, y, s, s - bright)`, with `bright = brightPx(s, ratio)` (BuffHudLogic.h:142-146). Hide O when bright == s (`setVisible` false).
- Result: elapsed top = icon under black at 0.65 = icon x 0.35, matching the base dim. Bottom = icon at full alpha. The seam is a hard edge; there is no clip.
- Precedents: label dim `widAlpha(b.icon, usable ? 1.0 : 0.35)` (CombatHotbar.cpp:709). Stretched fill with no setFillStyle call: `lbl_miscroll_f` pill "stretched into a square" (CombatHotbar.cpp:247).
- Missing piece: a plain solid fill. None of the 5 repo TGAs is solid (`rg --files -g '*.tga'`): buffhud_circle.tga 128x128, 53 distinct pixels (antialiased disc); uibit_bar_vp_sq.tga 128x256, 140 distinct (bar art). lbl_miscroll_f is an engine texture, not in repo (content UNPROVEN). Vanilla override contents UNPROVEN.
  Options: (a) tint buffhud_circle black (rounded corners show when stretched); (b) ship a 4x4 solid TGA in override/ (new asset, no code change).
