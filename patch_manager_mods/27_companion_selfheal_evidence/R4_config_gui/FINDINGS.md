# R4: in-game config GUI for companion-selfheal (static research, 2026-10-04)

Paths relative to patch_manager_mods/ unless absolute. Nothing run in game, no source edited. Engine addresses are Steam Aspyr. "dump" = ~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/<xx>/<addr>.asm|.c.

## 0. Recommendation (short)

v1 = **own hook at StopLoadFromLayout entry 0x0040F5A0, filtered on the HUD vtable 0x0099DCC4** (not the hotbar's 0x00748502), build runtime labels with guikit + GuiTip, per-frame work in `K2PS_FRAME_BEGIN`. Text = engine label text (`guikit::setText` / `setFont("dialogfont10x10")` / `setAlign`), the same mechanism the hotbar already ships. This also retires R2's "click-through" risk (a visible label with bit5 clear absorbs the click) and gives native hover tooltips. Later upgrade: move the label build into a `k2.hud@1` host if/when a 2nd HUD consumer justifies it (L4), and per-save persistence through `k2.saveevents@1` (L3). Details in section 5.

## 1. How the Action Hotbar editor works (Q1)

Mechanism (all runtime, no .gui edit): doc `RuntimeGuiControls.md:37-60` (recipe), `:96-101` (clicks).
- **Hook**: HUD ctor CSWGuiMainInterface 0x00747210; hook 0x00748502 (`8B 8D 0C FA FF FF`, `mov ecx,[ebp-0x5F4]`, panel), `action_hotbar_src/kotor2-steam-aspyr.hooks.toml:35-48`. Window = after the last InitControl (call at 0x007484EE) and before `call 0x0040F5A0` at 0x00748508 (dump 74/00747210.asm). Per-frame hook 0x0074B3F0 `HotbarHudTick(hud)` (toml :19-32).
- **Controls**: `Layout[]` ActionHotbar.cpp:1387-1476 = 86 CtlSpec {name, template, x,y,w,h (800x600 units), fill resref, text, font, align}. Two templates only: `TemplateTags[] = {"LBL_QUEUE1","LBL_MOULDING3"}` (:1332; blank label / native framed panel, present in every mipc28x6_p.gui). `createLabel` :1338-1383 = `guikit::createLabelFromTemplate` (GuiKit.h:148-178) + `setRect` + `setFont/setText/setAlign` (:1356-1359) + `setFill(imageFor(resref))` + `setVisible(false)` + `guikit::appendToPanel` (GuiKit.h:182-194). Called in a loop from `HotbarBuildButtons` :1731-1782 (clears projection flag 0x00A1B9F0 around the binds :1741-1750, `aspectFix()`, `ensureTextures()`).
- **Text**: label text object at ctl+0xD8 (+0x18 = CExoString sub-object, renderer +0x14); set with `guikit::setText` (ActionHotbar.cpp:825, :995), colour `setTextColor` gated by `getTextColor` (:1003). Editor title / tab captions ("Powers","Items","Modes","Clear", Layout :1409-1416) are plain label text with font `dialogfont10x10`, align 18 centre.
- **Editor flow**: `updateEditor(hud, gate)` :1148-1260 (per tick): `EditMode {M_Normal, M_Edit, M_Pick}` :645; polled `guikit::clickEdge` (:1170) + cursor (`cursorInGame`) hit-tested against label extents (`wHit`); show/hide by `wVisible` (flag bit1 + bit5, GuiKit.h:230); hover = swap fill resref / alpha (:1187-1190); tabs/prev/next/clear/mode buttons are labels with `fph_slot` frame textures (embedded TGAs, `HotbarTextures.inc`, `ensureTextures` :1537, RuntimeGuiControls.md section 6). Wheel via window subclass (:1018-1040). Esc via key hook 0x0072CB63.
- **Mode button / thresholds**: `LBL_HOTBARMODE` (Layout :1473) cycles `hb::nextTargetMode` (`modeClick` :1134-1147) and persists through `assignSlot` -> `Config::Save`. **There is NO numeric entry in the hotbar.** Thresholds (`@lowhp:<pct>`) are INI-only: `hb::TargetSpec.thresholdPct`, `effectiveThreshold` HotbarLogic.h:243, text parse HotbarConfig.h:117-142. So steppers (-/+) are new work, but they are just two labels + the same click edge.
- **Tooltip**: hotbar-private pair `LBL_HOTBARTIPBG` (TP_Panel) + `LBL_HOTBARTIP` (Layout :1475-1476), sized by `hb::tipLayout`, text via `setText` (ActionHotbar.cpp:826-840), 450 ms delay (`TipDelayMs`). Shared alternative: `_shared_src/GuiTip.h` writes the CExoString at ctl+0x2C (strref ctl+0x28 := 0, :9-12, :46-53) so the ENGINE draws the box; custom two-label fallback inside; used by level_up_override only, and its header says "nothing observed live for a GuiKit control yet" (:17), so plan a probe.

### Which pieces need the ctor window (template still open)?
| Piece | Needs open layout (0x748502-style hook)? | Needs only |
|---|---|---|
| Create a NEW label (`createLabelFromTemplate` -> `InitControl` reads the template from the layout GFF at panel+0x30; GuiKit.h:145 "ONLY valid while the panel's layout GFF is open") | **YES** | panel ptr + template tag |
| `appendToPanel` (list at panel+0x24/+0x28) | no (but must happen once the label exists) | panel ptr |
| setRect / setVisible / setClickable / setText / setFont / setAlign / setTextColor / setFill / setAlpha | no | control ptr (any time, game thread) |
| Hover / click polling, wheel, tooltips (`GuiTip.setNative` = 2 writes at ctl+0x28/+0x2C) | no | control ptr, cursor, `clickEdge` |
| Persistence, INI, FileWatch | no | - |
So the ONLY hard hook requirement is the one-time creation. Panel pointer alone is not enough to create a control; a control pointer alone is enough to drive it.

### Panel pointer without the hotbar (new finding)
The HUD embeds its 3 portrait slots: vector-ctor at `this+0x1F34`, 3 elements, stride 0xDE4, element ctor 0x0074A910 (dump 74/00747210.asm lines 48-50: `PUSH 0xde4 / PUSH 3 / ADD ECX,0x1f34 / CALL 0x0091d67e`; per-slot init `LEA ECX,[ECX+EAX*1+0x1f34]` x 0xDE4 at 0x00747A89-0x00747A95; also 08_shield_bar.md:233). So `hud = slot - 0x1F34 - i*0xDE4` for i in {0,1,2}; validate `*(uint*)hud == 0x0099DCC4` (CSWGuiMainInterface vtable, upstream_prs/ekc_steam/gog_check.md:99; ctor sets it at 74/00747210.c:146). This is statically anchored but NOT live-confirmed (R2 said "not derivable"; it is, with a vtable check). It gives a panel pointer for recycling/driving controls but NOT for creating them.

## 2. Options (Q2) ranked

| # | Option | Needs | Anchors | Cost | Risk | Text? |
|---|---|---|---|---|---|---|
| **b2** | **Own hook at StopLoadFromLayout entry 0x0040F5A0, `if (*(uint*)ecx == 0x0099DCC4)` build labels** (ECX = panel, fastcall; layout still open because the function is what closes it) | 1 hook, original_bytes `55 8B EC 83 EC 10` (push ebp; mov ebp,esp; sub esp,0x10: no rel operands; dump 40/0040f5a0.asm), param `ecx`; guikit + GuiTip; FRAME_BEGIN for per-frame | Function 0x0040F5A0 = StopLoadFromLayout (RuntimeGuiControls.md:29; body: if flag bit1 of panel+0x48 -> free GFF at +0x30, clear bit1; dump 40/0040f5a0.c). ~70 callers (all panel ctors) incl. HUD ctor call at 0x00748508. No mod hooks it or any byte within 8 of it (rg over all *.hooks.toml; nearest 0x0040ED9F dialogue_letterbox_fix_probe). Runs AFTER the hotbar's 0x748502 hook, so its labels are already appended; we append after (fresh count read, RuntimeGuiControls.md:60-61) | ~500-800 LOC (hotbar's editor part = ~370 lines :964-1330 + create :1338-1383 + build :1731-1782). Mostly copy-adapt from GuiKit; 16-22 controls | Low-medium. Hook fires for every panel ctor (cheap filter). Must check layout really open (`[panel+0x30]!=0` and `[panel+0x48]&2`) and clear/restore projection flag 0x00A1B9F0 exactly like :1741-1750 (flag is restored by hotbar before we run, but our own InitControl calls see it set). HUD leaks controls per rebuild (known, ~25 KB per HUD in hotbar; we add ~5 KB). Template tags must exist (same two as hotbar) | **Yes**, native label text |
| b1 | Own hook inside the HUD ctor window, other address | 1 hook | After the last InitControl and before StopLoad: 0x007484F7 (`8D 8D DC FC FF FF` lea ecx,[ebp-0x324], 6 bytes, relocatable; before the dtor call 0x007484FD; span 0x7484F7-0x7484FD) or 0x007484F3 (`C6 45 FC 3A`, 4 bytes, too short alone). Panel = `[ebp-0x5F4]` as in the hotbar. Must stay >= 0x007484EE+5 so no InitControl follows (a later `InitControl(addToList=1)` would overwrite our slot, RuntimeGuiControls.md:195-197) | Same as b2 | Low-medium; only HUD, but ctor-internal stack offset and a 5-byte gap to the hotbar span (0x748502): `check_hook_spans.py` WARNs (<= 8 bytes apart), not FAIL | Yes |
| a | Hotbar exports `k2.hud@1` (doc 22 s4.3, 22_hotbar_shared_services.md:119-125; L4 DEFERRED, 24_master_plan_status.md:59) | Hotbar change: `KPatchInit` + `Provide` (hotbar uses NO registry API today, doc 22 s1 :9), `on_build(panel, reserveIds(n))` call inside `HotbarBuildButtons`, `on_tick` in `HotbarHudTick`, `active()`; hotbar version bump + contract test + hostile/absent-provider tests; selfheal side ~ same widget code as b2 | ActionHotbar.cpp:1731 (build), toml :19 (tick) | Hotbar ~150-250 LOC + tests, but collides with HP4 refactor of `updateEditor`/`Bar` and HP2-HP7 queue (24_master_plan_status.md:63-69); owner-gated | Medium: couples release cycles, install order, registry version rules | Yes (same labels) |
| c | Engine text draw from HUD_DRAW | New RE | Text goes through CSWGuiText (ctor 0x004171A0 -> vtable CSWGuiText; renderer object at text+0x14 built by 0x0047EC30 -> 0x00479CA0, size 0x60; SetFont 0x00416DE0, Set 0x00416E30, dump 41/). GOG/Linux names exist only as `CAurFont__TextOutA` (ELF 0x08416f30, 1269 b) and GOG `AurRenderStrings` 0x00866AF0 (a per-frame queue flush, not an immediate call). No Steam named immediate text-draw in the DB; no mod (shield bar, buff HUD, perf-lab, hotbar) draws text outside a label (rg: setText only in hotbar + CharEffectsPanel) | High: RE to find a standalone CSWGuiText Draw vtable slot + whether it needs a parent control rect; render-state (GL caps) issues like doc 08 flicker | High | Maybe, UNVERIFIED |
| d | Digit / glyph atlas TGA via `gui2d::drawImage` | Embedded or override TGA; image wrapper per resref (`imageFor`, ShieldBar.cpp:237 uses override `uibit_bar_sh_p`, manifest.toml:12) | Gui2D.h:101 `drawImage(img,x,y,w,h,rot,rgb,alpha)`; flat quads only; DLL-memory textures need the hotbar recipe (RuntimeGuiControls.md section 6, ActionHotbar.cpp:1517-1565) which is hotbar-private | Medium: glyph sheet (0-9, %, +, -, A, ON/OFF), UV sub-rect problem (drawImage has no source rect; one image per glyph = ~16 images) | Medium-high (texture lifetime/GL-reset self-heal, flicker with textured fx doc 08:301-303); localisation impossible; no click absorption | Digits + few words only |
| e1 | Menu overlay (K2PS_MENU_OVERLAY): config page in the party/character menu | `ctx.panel` (CSWGuiInGameMenu = slot - 0x2350, PortraitSlots.h:61, K2Addr Menu_PortraitSlot), `ctx.companionCtrl[2]` = LBL_CHAR2/LBL_CHAR3 (panel+0x3134/+0x36DC, :65), draw in panel-local coords (:17) | Flat `gui2d` only = no text; so text means recycling native menu labels via setText/setRect/setClickable | Medium | Medium-high: which native labels are idle is unknown (needs a dump pass), they belong to menu logic; menu only (not on HUD) | Only via recycled native labels |
| e2 | HUD native-label recycling (panel via the slot formula above) | HUD ptr = `slot-0x1F34-i*0xDE4`, `[hud]==0x99DCC4`; control ptrs by index (needs native tag->index map from the ctor) | 74/00747210.asm:48-50 | Medium | High: native HUD labels are driven by the HUD tick (action queue LBL_QUEUE*, etc.); which are idle is unknown; they get overwritten each frame | Via recycled labels |
| f | INI-only + hotkeys, hot-reload via FileWatch.h | `filewatch::changed/atomicWrite` (FileWatch.h:1-12), key capture = hotbar-owned 0x0072CB63 (cannot add a 2nd patch); polled `GetAsyncKeyState` hotkeys need no hook | FileWatch.h; hotbar INI reload pattern ActionHotbar.cpp:396-410 | Low (~150 LOC) | Low | No in-game text (feedback only via the button colour / log) |

Notes:
- KPM rule: only the IDENTICAL address is rejected. `HookValidator.DetectOverlappingHooks` groups by `h.Address` (src/KPatchCore/Validators/HookValidator.cs:116-132) and `ValidateMultiPatchHooks` likewise across patches (:140-175, called from PatchApplicator.cs:303-310). The project's own gate `scripts/check_hook_spans.py` is STRICTER: FAIL on any shared byte of two patches' `[address, address+len(original_bytes))` spans, WARN when <= 8 bytes apart (docstring :1-20, GAP_WARN = 8). So b2 (0x0040F5A0, 6 bytes) passes both; b1 at 0x007484F7 passes with a WARN.
- b2 also works for later panels: the same entry is the creation window of every panel (inventory ctor call at 0x008A69AD+, character 0x0084D877, etc.), so a future config page in another menu needs only a different vtable filter.
- Click absorption: with b2 the companion button is a label; a visible label with bit5 (0x20) clear is found by the label hit-test 0x00418D20 and the world never gets the press (RuntimeGuiControls.md:94-101). This replaces R2 section 3 mitigations (a)-(c).

## 3. Persistence (Q3)

- Reusable from `_shared_src` today: `FileWatch.h` (`filewatch::stamp`, `ReloadPoll`/`changed`, `accept`, `atomicWrite`; used by the hotbar at ActionHotbar.cpp:396-401, `Config::Save` HotbarConfig.h:399), `Party.h` (controlled creature, members, vitals), `SafeRead.h` (`readExo*`).
- NOT reusable as is (hotbar-private, pure C++ though): `HotbarConfig.h` (`hb::Config` is slot/profile specific; only `parseInt32` :34, `trim` :66, `lower` :72 are generic), `saveDirFor` ActionHotbar.cpp:1586-1593 (reads engine cells `0xA7EA88` cloud flag / `0xA7EABC` dir, `SaveRequestHolder`), pure helpers `hb::normalizeSaveFolder` HotbarLogic.h:202, `joinSaveDir` :210, `pickSnapshot` :468, `snapshotToSave` :1595-1606, `replaceLiveProfiles` :1608-1622.
- Per-save needs the save hooks, which are taken: hotbar 0x00534527, 0x0053C5AA, 0x0052FD1E, 0x008529E0, 0x008FACA0 (toml :50-120, doc 22 s2); perf-lab EVT sites 0x52F610/0x52FB80/0x53AF20/0x7A06B0/0x7D0F50 (L3 row, 24_master_plan_status.md:57). Selfheal cannot add its own at those addresses.
- **v1**: one global `companion_selfheal.ini` in the game dir (FileWatch reload + atomicWrite save, keys per companion = the hotbar's character key logic, ActionHotbar.cpp:307 `computeCharacterKey`, or the creature's template/tag), no per-save snapshot. Document that settings are global, not per save.
- **Upgrade**: extract `SaveDir.h` (normalize/join/pick + cloud-dir read) into `_shared_src` (hotbar owner gate, 19b section D) and subscribe to `k2.saveevents@1` (L3) for snapshot/restore; schema stays one INI so the snapshot is a CopyFile.

## 4. Widget list and text dependency (Q4)

Text relies on: `guikit::setText` + `setFont("dialogfont10x10")` + `setAlign(18)` on labels created from `LBL_QUEUE1`; frames from the native framed-panel template `LBL_MOULDING3` (no custom TGA needed: hover/on = `setAlpha` + `setFill(img, tint)` gated by `getBorderTint`, GuiKit.h:300-327). If frames look wrong, embedding 2 TGAs is the hotbar recipe (RuntimeGuiControls.md section 6) = later polish.

Scope: one POPOVER shared by both companions (selected companion index), plus one small button per companion card.

| # | Control (template) | Text / content | Behaviour |
|---|---|---|---|
| 1-2 | `SH_BTN2`, `SH_BTN3` (TP_Panel, 16-24 px square from `Slot_VitRect`, R2 formula) | "H" (heal), text colour green + frame tint when Auto ON, grey when OFF, red tint when no healing item | Left click = use best item now (R1 route); right click (needs `GetAsyncKeyState(VK_RBUTTON)` edge via `clickEdge`-style helper) = open/close popover for that companion; hover tooltip "Click: heal now. Right-click: auto-heal settings" |
| 3 | `SH_POPBG` (TP_Panel) | frame | positioned left of the card strip, clamped on screen |
| 4 | `SH_TITLE` (TP_Plain) | companion name + " auto-heal" | - |
| 5 | `SH_AUTO` (TP_Panel + text) | "Auto: ON" / "Auto: OFF" | toggle; persists |
| 6-8 | `SH_HPLBL` / `SH_HPMINUS` / `SH_HPPLUS` (+ value label `SH_HPVAL`) | "Use below" "-" "50%" "+" | step 5 %, range 5-95, hold-repeat like the hotbar's repeat timers (wheel optional via the hotbar-style subclass is NOT planned: another stacked subclass) |
| 9-12 | `SH_KEEPLBL` / `SH_KEEPMINUS` / `SH_KEEPPLUS` / `SH_KEEPVAL` | "Keep N in stock" "-" "2" "+" | step 1, range 0-20; auto-use only while party stack count > N (manual click ignores the reserve) |
| 13 | `SH_ALLTOGGLE` (optional) | "Apply to all" | copies values to every companion |
| 14-15 | `SH_TIPBG`, `SH_TIP` | tooltip (or `guitip::setNative` on 1-13 if the TIPPROBE proves the native box) | hover 450 ms |
Total 16 labels (22 with value/labels split). Always append popover/tooltip last (draw order = list index, RuntimeGuiControls.md:100-101); hide the popover when `gate` closes, a menu opens, or the companion list changes.

Per-frame (K2PS_FRAME_BEGIN, leader call): derive rects from `SlotAt` + `Slot_VitRect` (R2 section 2), `setRect` the two buttons, update text/tint only on change (hotbar rule: write engine memory only when a value changes, RuntimeGuiControls.md "Per-frame cost"), poll clicks (`guikit::clickEdge`, `cursorInGame`), stale-HUD check (bar keyed on panel pointer, `g_bar = Bar()` on each build, ActionHotbar.cpp:1744).
HUD_DRAW is then only needed for an optional auto-ON glow; it can be dropped entirely.

## 5. Recommended path and sequencing

**v1: b2 (own hook 0x0040F5A0 + HUD vtable filter) + labels with text + GuiTip + global INI.** Why: only option that gives real text, click absorption and tooltips without touching hotbar-owned files; one extra hook address; guikit/GuiTip/FileWatch already shared and unit-tested; ~16 labels is smaller than the hotbar's 86.
**Fallback if b2 build misbehaves** (e.g. layout not open at entry): b1 at 0x007484F7 (same code, different entry parameter, WARN-level span gap).
**Fallback of the fallback**: f (INI + the R2 flat button, no text) - already specified in R2.
**Upgrade**: when a 2nd HUD consumer or the hotbar HP4 refactor lands, move the build call behind `k2.hud@1` (option a) and delete our hook; for per-save, `k2.saveevents@1` (L3) + shared SaveDir.h.

### Probes before shipping (live, user GO each; static-only here)
1. P1: hook 0x0040F5A0 log-only: log panel ptr, vtable, `[panel+0x30]`, `[panel+0x48]`, control count, for the HUD only; confirm window open and hotbar-before-us order (expected: hotbar's controls already counted).
2. P2: create one `LBL_QUEUE1` label with text "TEST" at a fixed rect, append, show; confirm text draws and a click on it does not move the leader.
3. P3: slot -> HUD formula (`slot-0x1F34-i*0xDE4`, vtable 0x99DCC4) logged next to the hook's panel ptr (they must be equal).
4. P4: `guitip::setNative` on the label, watch TIPPROBE (GuiTip.h:17 says never observed for GuiKit controls).
5. P5: right-click edge reading (`VK_RBUTTON`) and whether the engine also handles right click on empty label space.

## 6. Open questions / traps
- HUD vtable for derived HUD variants: only CSWGuiMainInterface 0x0099DCC4 verified (gog_check.md:99); confirm the live object's vtable == that in P1.
- The 3 embedded portrait slots are drawn by the panel Draw outside the +0x24 list (assumption: slots are embedded members, not list entries); a popover overlapping a card could be overdrawn by it: keep it in the free strip left of x = vit.x (R2 section 2) and test.
- Template presence: if a user's ultrawide .gui override drops `LBL_QUEUE1`/`LBL_MOULDING3` the build must degrade to "no popover" and keep the INI path (hotbar does this per control, ActionHotbar.cpp:1779-1783).
- Right-click is new input (hotbar only polls LMB, `GuiKit.h:370-378` has `clickEdge` for VK_LBUTTON only): add a `GuiKit` right-button edge or a left-click-with-modifier fallback (Shift+LMB).
- Per-save config unavailable until L3; say so in the player doc.
