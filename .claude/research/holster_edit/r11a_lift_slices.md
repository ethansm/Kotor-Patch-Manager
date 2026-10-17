# r11a LIFT slices: HolsterEditor.cpp -> mod_config_src/ModConfig.cpp

Source: `patch_manager_mods/holster_editor_src/HolsterEditor.cpp` (716 lines).
Target: `patch_manager_mods/mod_config_src/ModConfig.cpp` (not created yet). Already present: `mod_config_src/exports.def` (KPatchInit, McBind, McTick, McDestroy) and `kotor2-steam-aspyr.hooks.toml` (McBind 0x8A16FE, McTick 0x40EC90, McDestroy 0x8A1930).

## 0. Range check
- All brief ranges match the file. No drift.
- Lines the brief leaves out, plus holster-only lines inside LIFT ranges, are listed in section 1. Apply section 1 before pasting.

## 1. Fixes to the brief (apply before pasting)
1. Add, unlisted and required: 53 `namespace {`, 55 `using menulayout::Rect;`, 88-89 `g_enabled` / `g_debug` (read by LIFT code), closing braces 414 (firstTick), 555 (`}  // namespace`), 604 (McBind), 659 (McTick), 700 (McDestroy).
2. KEEP 582-586 splits a KP_LOG statement (586/587) and leaves the `if (h.obj && p3) {` at 581 and `}` at 588 unbalanced. Correct KEEP range: **581-588**.
3. Holster-only lines inside LIFT ranges that the KEEP list does not cover (drop them):
   - firstTick: 407-408 (status/view setVisible), 412-413 (logRect status/view).
   - enterEditor: 433-434 (status/view visible), 436-438 (log fields `view=` / `token=`).
   - leaveEditor: 459 (view), 461 (status).
   - McBind: 575-576 (`g_p3.connect`, `g_hol.connect`, CTOR providers log), inside LIFT 558-577.
   - McDestroy: 668-669 (log fields `token=`, `free_view=`), 671 (`|| g_cfg.freeView`), 677 (`freeCtl(e.status)`).
   - Config 85 `freeView`, readConfig 230 `free_view`, constant 60 `kSceneViewVtable`, constant 73 `kDescText`.
4. KEEP 400-401 (`setCtlText` for btn and back) is generic logic with holster literals. The generic FirstTick must still set item labels and "Back" through the item table. The brief's KEEP only removes the literals from LIFT.
5. KPatchInit (703-713) is not in the brief. Write it fresh. Drop 707-708 (`g_p3.init`, `g_hol.init`). Keep line 709 (`embedgui::g_log`) only if the embed path stays (see OPEN). Rewrite the banner at 710-712 (holster text and ini keys). Keep it literal in the source for the create-patch export scan.

## 2. Slice table (source line ranges)

### 2.1 Includes, constants, config, state
| Range | Keep / drop | Notes |
|---|---|---|
| 25-31 | keep | std + windows includes |
| 32-38 | keep | SafeRead.h, KpLog.h, K2Addr.h, GameAddr.h, GuiKit.h, GuiButton.h, MenuLayout.h. KpLog.h is not in the repo (KPM SDK include path). |
| 39 | keep only if embed kept | EmbedGui.h (OPEN) |
| 40-43 | DROP | HolsterApi.h, Preview3dApi.h, HseLayout.h, HseViewGui.inc |
| 45 | edit | `KP_LOG_PATCH("holster-editor")` -> `"mod-config"` |
| 47-49 | keep | chMain / chEmbed / chInput |
| 51 | edit | `"holster-editor_log.txt"` -> `"mod-config_log.txt"` |
| 53, 55 | ADD | namespace open, `using menulayout::Rect;` |
| 57-59 | keep | kOptionsVtable, kLabelVtable, kButtonVtable |
| 60 | DROP | kSceneViewVtable (used only in KEEP 688-690) |
| 62 | keep Quit/Title/Exit; drop `kIdxDesc` | kIdxDesc feeds only 340 and KEEP block 388 |
| 63 | DROP | kBarFirst/kBarLast feed only statusFloor (KEEP 392) |
| 64-66 | keep, reshape | kColumnIdx = vanilla column order. kInsertAfter = 2 ("after GAMEPLAY") becomes a per-item field. kColumnN is fixed at 8 and must grow with item count. |
| 67-71 | keep | kMaxVanilla, kMaxTargets (see section 4), kMaxReapply, kTickGapMs, kLeaveDelayMs |
| 72 | keep | kLabelTemplate "LBL_TITLE" |
| 73 | DROP | kDescText (holster string, moves to item table) |
| 75-78 | keep | panel / vtable / holder offsets |
| 80-87 | keep 82-84, DROP 85 | Config: buttonClone, reflow, desc, freeLabels kept; freeView dropped |
| 88-89 | ADD | g_enabled, g_debug |
| 90 | optional | g_api, only KPatchInit uses it |
| 91-92 | DROP | g_p3, g_hol (holster providers) |
| 94-99 | keep | struct Target |
| 101-124 | keep, edit | Editor. Drop line 104 `view`, line 105 `token`, and `status` on line 106 (keep `btn, back, title`). `btn` becomes an item array. |

### 2.2 Helpers
| Range | Keep / drop | Notes |
|---|---|---|
| 126-133 | keep only with embed | embedLog, tag "EMBED" |
| 135-136 | keep verbatim | `rd<T>` uses SafeRead site literal `0xE70100u`. Do not change it. |
| 138, 141-145 | keep | ctlFlagsOff, Call1Fn / Call2Fn / TextPushFn / AssignFn / FixFn |
| 148-161 | keep | listCtl, ctlId |
| 163-169 | keep | cloneLabel |
| 172-195 | keep | cloneEntry (button clone, canary, label fallback) |
| 197-202 | keep | setCtlText |
| 204 | keep | iniBool |
| 206-233 | edit | readConfig. Line 207 `"holster_editor.ini"` -> `"mod_config.ini"` (file exists with only `enabled` and `debug`). Keep keys 224-225. Drop 230 `free_view`. Keys 226-229 only if their Config fields survive. |
| 235-240 | keep | logRect |
| 242-247 | keep | ctlRect |
| 249-255 | keep | addTarget |
| 258-280 | keep | applyTargets |
| 283-288 | keep | clearFocus |
| 294-318 | keep | describe(P, text, &why): text is a parameter, so it is generic. Log "DESC ok via=8A2320 strref=-1" stays. |

### 2.3 firstTick (321-414)
- LIFT: 321-387, 397-399, 404-406, 409-411, 414 (add).
- DROP: 388-396 (KEEP, view and status geometry), 400-403 (KEEP strings), 407-408, 412-413.
- Edits:
  - 324-331: five-slot array (section 3). Loop over items plus back and title.
  - 339-340: desc rect feeds only the KEEP block and log 351. Drop.
  - 343-350: barY, nBars, and BAR log feed only statusFloor (KEEP 392). Drop, or the variables go unused.
  - 351-352: TEMPLATE log. Remove the desc / dOk fields.
  - 355-361: `menulayout::columnReflow` (MenuLayout.h:29) inserts ONE row and returns `outMoved[n - insertAfter - 1]`. For N items, call it N times or write a multi-insert helper.
  - 362-369: `menulayout::slotBelow` (MenuLayout.h:52) off-branch. Keep.
  - 370-384: fallback. Line 374 has holster literals `(422 - 206) * gp.h / 32` (D3a placement). Replace with an item-relative y or delete the fallback. Lines 363-364 and 379 use the 36/32 pitch ratio, which is generic.
  - 386-387: back and title targets. Keep.
  - 404: `setVisible(btn, true)` -> loop over items.
  - 405-406: back and title hidden. Keep.
  - 409-411: logRect for btn, back, title. Keep.
- Text (replaces KEEP 400-402): `setCtlText(item.ctl, item.isButton, item.label)` per item; `"Back"` for back; `guikit::setText(title, <page title>)`.

### 2.4 enterEditor / leaveEditor (416-466)
- enterEditor (416-439):
  - LIFT: 416-432, 435, 439.
  - DROP: 433-434.
  - Edit 436-438: remove the `view` and `token` args.
  - Per-slot hide and show (430-432) becomes an item loop.
- leaveEditor (441-466):
  - LIFT: 441-458, 460, 462-466.
  - DROP: 459, 461.
  - Restore loop (448-457) uses `guikit::FlagVisible`, `FlagClickThrough` (GuiKit.h:50) and `guikit::detail::wr` (GuiKit.h:125). Keep.

### 2.5 hoverTick (469-493)
- Lift whole.
- Edit 485: `describe(P, kDescText, &why)` -> `describe(P, item.desc, &why)`.
- 471 and 474 use single `e.btn`. Needs a hovered-item index.
- Helpers: `guikit::hit` (GuiKit.h:423), `guibutton::buttonHighlighted` (GuiButton.h:116).

### 2.6 Support for McDestroy (496-553)
- isOwn (496-499): five-slot, see section 3. Drop the view and status terms.
- dropRefs (502-518): keep. Uses kPanelFocus, kMgrHover, kMgrCapture, kMgrCaptureFlag, kMgrLastHit, kVtHover; `guikit::detail::wr`.
- freeCtl (521-526): keep. Uses `guikit::g_gkCallLabelDtor` (GuiKit.h:154-156), kLabelVtable, kButtonVtable.
- releaseOwn (529-553): 535-537 `own[5]` loop is five-slot. 541 `C > V + 5` is the same 5. Replace with the item count plus back plus title.

### 2.7 McBind, was OptBindView (557-604)
- Lift: 558-574, 577, 600-602, and add 604.
- Drop: 575-576 (providers).
- OPEN (embed): 579-580 and 589-597 wrap only the HSE_VIEW layout (`kHseViewGui`). With KEEP 581-588 (the view block) removed, Open and Close have nothing to serve. Recommendation: drop 579-597 and include 39 only if embed stays.
  - If embed is dropped, the gate at 572 loses the `!slot` check (`slot` is read at 565 from `embedgui::PanelLayoutSlot`).
  - If embed is kept, keep 579-580 and 589-592, and KEEP 593-596 with 594 removed.
- Edits: 600 `cloneEntry(P, "BTN_GAMEPLAY", "btn_holster", ...)` becomes the per-item template tag from the item table. 601 `"BTN_EXIT"` / `"back"` and 602 `"title"` stay as generic Back and title.
- 603 `g_ed.status = cloneLabel(P, "status")` is KEEP.

### 2.8 McTick, was OptTick (607-659)
- Lift: 606-629 (drop 630-631: `g_p3.poll()`, `g_hol.poll()`), 632-656, 658, and add 659.
- Edits:
  - 639-641 (`overBtn` / `overBack`): single `e.btn` and `e.back` -> per-item hit test.
  - 653-654 (`pressed == 1` / `pressed == 2`): codes become item index plus Back code.
  - 628 `leaveEditor("tick gap")`, 629 `firstTick()`, 658 `leaveEditor("back")`: keep.

### 2.9 McDestroy, was OptDestroy (661-700)
- Lift: 661-667, 670, 672-679 (minus 677), 699, and add 700.
- Edits:
  - 668-669: log without `token` and `free_view`.
  - 671: `if (g_cfg.freeLabels || g_cfg.freeView)` -> `if (g_cfg.freeLabels)`.
  - 673-676: loop over items plus back and title.
  - 677: DROP (status).
- KEEP 680-698: `view`, `token` destroy, freeView block.

### 2.10 KPatchInit and DllMain
- KPatchInit (703-713): rewrite per section 1, item 5.
- DllMain (716): verbatim.

## 3. Five-slot hard codes (all sites)
Replace with a fixed item table: `items[kMaxItems]` (ctl, isButton, label, desc, insertAfter, saved state) plus `back`, `title`.

| Line(s) | What it does |
|---|---|
| 324-325 | `mine[]` / `names[]` = {view, btn, back, title, status} |
| 326 | `i < 5` append loop (327-331 use it) |
| 400-402 | KEEP text calls (btn, back, title) |
| 407-413 | visibility and logRect for status and view (DROP) |
| 426-434 | enterEditor show / hide per slot |
| 459-463 | leaveEditor show / hide per slot |
| 497-499 | isOwn |
| 535-537 | releaseOwn `own[5]` and loop |
| 541 | `V + 5` bound |
| 673-677 | freeCtl per slot |

## 4. Other design notes
- `kMaxTargets` = 16 currently holds 1 entry + 5 moved rows + back + title + view + status (10). With N items and repeated `columnReflow`, raise the cap (for example 32) and dedupe moved rows.
- `g_ed` appears on 24 lines. Rename to `g_mc`. `Editor` can become `McState`.
- Log markers (EMBED, BTNCLONE, REFLOW, RELAYOUT, HOVER, DESC, FREE, INPUTCLASS, HIDDENDRAW) are in LIFT code. Keep or rename consistently with the test harness.
- Holster literals outside the LIFT ranges (not for ModConfig): 402-403 (title and status text), 582 `"HSE_VIEW"`, 710-712 banner. Keep them in holster.

## 5. Holster-specific strings and constants inside LIFT ranges
- 45 `"holster-editor"`; 51 `"holster-editor_log.txt"`
- 73 `kDescText` = "Adjust where holstered weapons sit on each party member. Changes preview here and save to holster.ini."
- 207 `"holster_editor.ini"`; 226-230 keys `button_clone`, `reflow`, `desc`, `free_labels`, `free_view`
- 324-325 `"view"`, `"status"` (the names table)
- 371 comment ("BTN_GAMEPLAY at y 206"), 374 literals `422`, `206`, `32`
- 436 log fields `view=%x token=%u`
- 600 `"BTN_GAMEPLAY"`, `"btn_holster"` (the entry template tag)
- 668-669 log fields `token=%u`, `free_view=%d`
- 575-576 holster provider connects (DROP)
- 136 `0xE70100u`: not holster. Keep identical.
- 557, 606, 661: addresses in comments only.

## 6. Helpers used in LIFT code (header paths)
- gameaddr (`patch_manager_mods/_shared_src/GameAddr.h`): OptionsInGameVtable:116, LabelVtable:38, ButtonVtable:39, OptionsSetDescription:128, ExoStringAssign:129, GuiTextObjRefresh:130. SceneViewVtable:55 is KEEP-only.
- guikit (`patch_manager_mods/_shared_src/GuiKit.h`): createLabelFromTemplate:175, appendToPanel:209, getExtent:224, setRect:238, setVisible:257, isVisible:264, setText:270, cursorEngine:412, hit:423, g_gk Engine:69-116 (exoCtor, exoDtor, gameForeground, lbuttonDown), g_gkCallLabelDtor:154-156, FlagVisible / FlagClickThrough:50, detail::wr:125.
- guibutton (`_shared_src/GuiButton.h`): steamBuild:32, releaseUnlinkedButton:45, createButtonFromTemplate:59, setButtonText:102, buttonHighlighted:116.
- menulayout (`_shared_src/MenuLayout.h`): Rect:19, columnReflow:29, slotBelow:52, statusStrip:57 (KEEP-only, line 393).
- embedgui (`_shared_src/EmbedGui.h`): PanelLayoutSlot:42, Handle:83, Open:90, Close:128, g_log:47 (only KPatchInit).
- saferead (`_shared_src/SafeRead.h`): beginScope:37, readAt:115.
- k2addr / K2AddrTable (`_shared_src/K2Addr.h`): Ctl_Flags:94, Ctl_Id:95, Panel_CtlArray:107, Panel_CtlCount:108; `k2addr::off`, `k2addr::get`.
- KpLog: KP_LOG_PATCH, KP_CHANNEL, KP_LOG, KpLog_SetFallbackFile, KpLog_SetFallbackLevel, KPLOG_INFO, KpLog_Init. Header not found in repo.
- Windows / CRT: GetTickCount (618), InterlockedIncrement, `_stricmp`.
- Holster-only, do not lift: k2p3 (`_shared_src/Preview3dApi.h:117-168`), k2holster (`_shared_src/HolsterApi.h:88-136`), hse:: viewRect (`holster_editor_src/HseLayout.h:95`), statusFloor (`:109`).

## 7. Rename map
- OptBindView -> McBind, OptTick -> McTick, OptDestroy -> McDestroy (names pinned by `mod_config_src/exports.def` and the hooks toml).
- `g_ed` -> `g_mc`; `Editor` -> `McState` (optional).
- Patch id 45 and 51 -> `mod-config`.
- Config file 207 -> `mod_config.ini`.

## 8. Not verified
- No compile or test run. Line ranges were checked by reading the file. Helper signatures were checked by grep.
