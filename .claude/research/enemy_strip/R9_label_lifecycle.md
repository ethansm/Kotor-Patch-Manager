# R9: HUD label lifecycle (K2HUD_BUILD / TARGET_MENU_POST), static recon

Static only. Read-only except this file. Steam exe sha 6A522E71, vanilla dump.
Abbrev: `GD` = `/home/ethansm/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp` (outside the repo).
`R` = repo root `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`. Repo paths are relative to `R`.
Menu = hud + 0xCC. "1BAC" = menu+0x1BAC. Engine px = display px.

## Q1. Does TARGET_MENU_POST fire every frame, incl. no target and hidden menu? How do labels hide?

Dispatch
- PRE: `HH_TargetMenuPopulate` (`R/patch_manager_mods/hud_host_src/HudHostProvider.cpp:149-173`); runs FRAME then PRE (`hud_host_src/HhDispatch.h:78-81`).
- POST: `HH_TargetMenuTail` (`HudHostProvider.cpp:175-195`) -> `hhd::runPost` (`HhDispatch.h:124-136`).
- `runPost` gates only on "PRE seen and pending" and "menu pointer equal" (`HhDispatch.h:125-127`). It has NO target check. Its ctx.target is the POST call's `[ebp+0x10]`, which may be null (`R/patch_manager_mods/hud_host_src/kotor2-steam-aspyr.hooks.toml:30-34`).
- POST hook sits at 0x743847, the return address after `CALL 0x743850` (`hooks.toml:35-41`, `GD/74/00743540.asm` around 0x743842).

Null target still reaches POST
- `GD/74/00743850.asm` 0x74385F `CMP [EBP+0xC],0`; 0x743863 `JZ 0x744239`.
- Null path (0x744239-0x744257): clears 1BAC bit0 (`AND EDX,0xFFFFFFFE`, 0x74424E), then `RET 8` to 0x743847. POST runs.
- Non-null path sets 1BAC bit0 (`OR ECX,1`, 0x743876-0x743885). So after 0x743850, bit0 == (target != 0). See also `R/patch_manager_mods/31a_target_menu_post_re.md` Q2/Q4.

Hidden menu does not stop POST
- No gate on 1BAC inside PopulateMenus call path or hud-host. The only gate is leader presence (below).
- 1BAC bit0 gates only the menu DRAW: `GD/74/00743280.c:9`.

When PopulateMenus (and so PRE/POST) runs
- Caller HUD update `GD/74/0074b3f0.c:83` -> `FUN_0074cfe0(1)` -> asm `0x74B545 CALL 0x74CFE0`.
- Inside 0x74CFE0, PopulateMenus call is inside `if (iVar3 != 0)` where iVar3 = leader object `FUN_007E5D80()` (`GD/74/0074cfe0.c:20-30`, call at :58, `asm 0x74D19F`).
- HUD update returns early before that, so no PRE/POST and no draw, when:
  - hud+0x48 bit6 clear (`GD/74/0074b3f0.c:56-58`), or
  - leader null / leader id == 0x7F000000 (`0074b3f0.c:61-71`).
- Other callers of 0x74CFE0: `GD/7c/007ceb50.c:6-7` (gated on `*(this+0x98) != 0`, callers 0x781BE0, 0x79D6E0) and `GD/74/0074da40.c:97` (arg 0). These can run POST with no HUD draw.
- So "every frame" is wrong literally. Correct: every HUD update that has a valid leader, plus some non-draw callers.

Draw coupling (the key for stale icons)
- In `0074b3f0.c`, after line 83 there is no return until the end (grep: returns only at :57, :71, :311). The draw block is at :287-290, gated by `DAT_00A106DC != 0`.
- So every drawn HUD frame has run PRE then POST earlier in the same call. Nothing is drawn on the early-return frames.

Stale-label rule
- In POST: `shown = ctx->target != 0 && ctx->targetType in 0..3 && (menu+0x1BAC & 1)`. If not shown, hide every pool label: `setVisible(false)` + `setClickable(false)`. Reset caches.
- Because POST precedes the draw in the same update, a POST-time hide is never a frame late.
- combat-hotbar: `onTargetMenuPost` (`R/combat_hotbar_src/CombatHotbar.cpp:750-778`) -> `updateButtons` (:485) -> `if (!ctx->target || type<0 || type>3) hideAllButtons(...)` (:497) -> `hideAllButtons` (:439-452) -> `widHide` = `setVisible(false)`+`setClickable(false)` (:334-338) on containers and every bg/icon. Also :585 on rect failure. It does not read 1BAC bit0.

## Q2. K2HUD_BUILD: when, and what to reset

- Hook: `0x0040F5A0`, `HH_LayoutClosing` (`hooks.toml:55-64`; `HudHostProvider.cpp:197-230`).
- HUD-only filter: panel vtable == HudVtable (`HudHostProvider.cpp:203-204`; `HhDispatch.h:85`). Sets `g_hud = panel` (:205). `runBuild` at :224/:229.
- Fires at the end of every panel's layout load (`GD/40/0040f5a0.c` header: ~100 callers). Layout is still open, so `createLabelFromTemplate` + `appendToPanel` are valid (`_shared_src/HudHost.h:17-20`; `GuiKit.h:174`).
- Context: panel == hud == HUD; menu == 0; targetType == -1 (`HhDispatch.h:88-98`; `HudHost.h:19-20`).
- Refire on resolution change: UNPROVEN. Static: `RescalePanels` 0x413DD0 (`GD/41/00413dd0.c` header, callers 007bc5a0, 00411a80) has no 0x40F5A0 callee. 0x892090 (also a 0x40F5A0 caller) temporarily sets 640x480 for its own panel (`GD/89/00892090.c:96-109`), not traced further.
- Reset on BUILD: yes, every consumer does it.
  - combat-hotbar: `onHudBuild` (`CombatHotbar.cpp:385-392`) resets `g_btn`, `g_cont`, and all caches before creating (:402-431).
  - queue-hud: `onHudBuild` (`R/queue_hud_src/QueueHud.cpp:494-506`) resets `g_cw`, `g_want`, `g_cpend`, `g_lmb`.
  - companion-selfheal: `buildWidgets` (`R/companion_selfheal_src/CompanionSelfHeal.cpp:1353-1354`, called :1409) resets `g_w`.
- Old labels are not destroyed. There is no destroy helper (`QueueHud.cpp:486` comment). Just drop the refs.
- New labels start hidden and click-through (`GuiKit.h:175-176, 206`), so a rebuild cannot show stale icons.

## Q3. Draw order: are our labels above or below the target menu?

- In `0074b3f0`: base panel draw `0x40EC90` with ECX = hud at `0x74C1B2` (`0074b3f0.c:289`). It walks hud+0x24 in list order, index 0 first, drawing children with flag bit1 set (`GD/40/0040ec90.c:32-38`).
- Then the menu draw `0x743280` with ECX = hud+0xCC at `0x74C1CA` (`0074b3f0.c:290`; asm :806-807 `ADD ECX,0xCC`).
- Result: labels appended to the HUD panel are drawn BEFORE the menu, so the menu draws on top of them.
- Menu draw (`GD/74/00743280.c`): gated on 1BAC bit0 (:9). `PushClip(menu+0x1648, 0x164C, 0x1650, 0x1654)` (:11-13). Then draws +0x18F8, +0x17B0 (CSWGuiLabel children), +0x1668 LBL_NAME, +0x1A40 PB_HEALTH (:15-18), then 3x `007414D0` (:19-21).
- Group children (`GD/74/007414d0.c:6-17`) draw only if flag bit1 is set: BTN +0x48, LBL +0x218, UP +0x3E8, DOWN +0x5B8.
- Consequence:
  - Inside the menu rect (about 533x51 px, R6 section 4), our labels are covered wherever opaque menu children sit (LBL_NAME, PB_HEALTH, +0x17B0/+0x18F8: rects not traced).
  - Above the menu origin (absolute y < menuY), the menu clip excludes them, so the menu cannot cover them. R6 section 2 agrees (HUD-list label at absolute y = drawn).
- Precedent:
  - Pill containers created before slot labels (`CombatHotbar.cpp:402-411`) sit under slot labels. This is order within the HUD list only, not against the menu.
  - Against the menu, combat-hotbar hides the menu's own group children under its pills: `hideVanilla` clears bit1 on BTN/LBL/UP/DOWN of all 3 groups every POST (:461-472, called :499). It does not rely on z-order.
  - The banner (LBL_NAME, PB_HEALTH) is not hidden. No precedent for HUD-list labels overlapping the banner.

## Q4. Label template defaults and init sequence (icon-only)

- `createLabelFromTemplate` (`R/_shared_src/GuiKit.h:178-208`): allocs, ctor, id sentinel, `InitControl(addToList=0, scale=1)` (:195-197), id stamp, flags = hidden + click-through (:206). It does NOT reset size, text, or border.
- LBL_QUEUE1 defaults: blank label, no fill, text '', font dialogfont16x16 (`R/RuntimeGuiControls.md:63-64`). Layout rect: not documented anywhere in the repo (UNPROVEN). queue-hud calls setText on LBL_QUEUE1 successfully (`QueueHud.cpp:422, 441`), so the text object exists.
- Icon-only label call sequence (combat-hotbar, function names only):
  1. BUILD (`CombatHotbar.cpp:418-426`): `createLabelFromTemplate` -> `setVisible(false)` -> `setClickable(false)` -> `appendToPanel`.
  2. POST, change-gated (:706-718): `setRect` (via widRect) -> `imageFor` -> `setFill` (via widFill, only when the resref changes, :709-711) -> `setAlpha` (widAlpha, usable 1.0 / else 0.35, :717) -> `setVisible(true)` + `setClickable(false)` (widShow(icon,false), :718).
  3. Hide (`widHide`, :334-338): `setVisible(false)` + `setClickable(false)`.
- NOT called on icon labels: setText, setFont, setAlign, setTextColor, setBorderTint, setFillStyle. Border tint and text appear only in the bg/container path (:713-715, :357-363).
- Caches: `Wid.vis` starts -1, `rectSet` false (`CombatHotbar.cpp:258-262`). First POST always sets rect before the first show. So template size is never visible. Reset of text/border is not needed.
- Click: bg (clickable) takes clicks, icon is click-through (:716-718). For our labels, `setClickable(false)` always.

## Q5. Coordinates and widescreen

- `setRect(ctl,x,y,w,h)`: vtable slot 1, raw ints, no scaling (`GuiKit.h:240-249`). `RuntimeGuiControls.md:54` and :80-81 say screen px.
- combat-hotbar builds absolute px from live engine values: `boxes = fx + ex` (`CombatHotbar.cpp:577`), banner = `fx + nx` (:583), with menu origin from +0x1648/+0x164C (:508) and getExtent (:576, :582). No `scaleFromRef` / `unitRect` (`GuiKit.h:389-410`) in combat-hotbar.
- queue-hud also places from live px: `cwApply` -> `setRect` (`QueueHud.cpp:452-465`, :459), rect from the hud-host strip vit rect.
- HUD panel rect at 3840x1600 = (0,0,3840,1600) (`R/patch_manager_mods/_shared_src/` R6 section 1-2 and combat-hotbar log). HUD-local == absolute there, but this is implicit, not checked.
- Aspect projection: the flag 0x00A1B9F0 is documented on InitControl only (`RuntimeGuiControls.md:70-74`). combat-hotbar clears it only around BUILD binds (`CombatHotbar.cpp:373-381, 398`). POST setRect is unguarded, consistent with the doc, but SetRect itself was not traced (PARTIAL).
- Widescreen scaling: not needed for values read live from the engine (menu origin, extents are already display px; R6 section 3). Use `scaleFromRef` only for layout-unit constants.

## Hide and placement rules (summary)

1. BUILD: store panel; reset cache; create N labels via `createLabelFromTemplate` -> `setVisible(false)` -> `setClickable(false)` -> `appendToPanel`.
2. POST (before every drawn frame): if `!(target && type 0..3 && 1BAC bit0)` hide all; else `setRect` above the menu rect, then `setFill`/`setAlpha`, then show.
3. Guard: compare `ctx->hud` to the BUILD-time panel; if different, drop refs and skip. No consumer does this today (`CombatHotbar.cpp:750-778`, `QueueHud.cpp` tick).

## OPEN

- Resolution-change BUILD refire: static path does not show it (RescalePanels has no 0x40F5A0 callee); not proven either way.
- Lifetime of the HUD object when POST fires from a non-draw caller (0x7CEB50, 0x74DA40): not traced; the identity guard above covers it.
- LBL_QUEUE1 layout rect and border defaults: not in repo.
- SetRect itself vs aspect projection flag: doc says InitControl only; SetRect not traced.
- Rect of +0x17B0 / +0x18F8 menu children (what covers the area above the banner): not traced.
- Leader-gated POST gap: no POST while the leader is null. Harmless only because the draw is gated the same way (see Q1); not verified at runtime.

## VERDICT

PARTIAL. Confirmed: POST runs before every drawn HUD frame and with null target; HUD-list labels draw under the menu; BUILD labels start hidden and must be reset on BUILD. Not literal: POST is not every frame (leader and HUD gates), and resolution-change refire is unproven.
