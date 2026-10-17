# Review: mod-config 0.1.0-dev (card c18) and holster-editor 0.2.0-dev port

Scope read in full: `patch_manager_mods/mod_config_src/ModConfig.cpp` (1255 lines), `MenuUiLogic.h`, `mod_config_src/kotor2-steam-aspyr.hooks.toml`, `exports.def`, `manifest.toml`, `mod_config.ini`, `_shared_src/ModConfigApi.h`, `holster_editor_src/HolsterEditor.cpp` (272 lines), its hooks.toml / exports.def / manifest / ini. Reference: `git show HEAD:.../HolsterEditor.cpp`.
Not done: no build, no live run. `run_tests.py mod_config` passes (common_contract_test only). `check_raw_addresses.py` passes (FAIL=0).
Hook metadata (addresses, original bytes, parameter sources) is byte-identical to the old holster hooks apart from the three handler names.

Verdict: CHANGES. 1 BLOCKING (design break, not a crash), 9 FINDINGs.

## BLOCKING

### B1. hideVanilla also hides LB_DESC, so the LIST and ENTRY description pane never shows
- `ModConfig.cpp:606-619`: `hideVanilla()` hides every vanilla control index `0..vanillaCount-1` (`:616`). That includes index 15, which is `LB_DESC` (`kIdxDesc`, `:59`; `d2c_options_gui.md:32`).
- `kIdxDesc` is used only for geometry (`:429`, `:435`). Nothing exempts or re-shows it.
- `applyEvent` ENTER(LIST) (`:665-672`) shows only list buttons, Back and the title, then calls `writeDesc(kListBlurb)`.
- The `describe()` path (`:323-347`) pushes text into the Options description listbox through 8A2320, which clears the list and relays out. If that listbox is hidden, the text is not drawn.
- Effect: the design's "desc on hover" on the LIST page (entry description, "Choose a mod..." blurb) and the per-button `desc` on ENTRY pages cannot be seen.
- The old holster code could hide LB_DESC safely because its editor page never used the description pane. The only live-proven hover description was on MAIN, where LB_DESC is visible.
- Fix: skip index `kIdxDesc` in `hideVanilla` (and therefore in `restoreVanilla`). The VIEW/STATUS panes sit left of and below it, so there is no overlap. At ENTER(LIST/ENTRY) write the page text so stale vanilla text is not shown.
- Confidence: high from the code. Not run live. The MC1 play row should confirm it.

## Checklist

### 1. McTick first statement is the vtable compare: PASS
- `ModConfig.cpp:1130` is the first statement. It null-checks `panelArg`, then compares `*(uint32_t*)panelArg` with `kOptionsVtable`, and returns on mismatch.
- `g_enabled` (`:1131`), `GetTickCount` and everything else come after.
- The raw deref is acceptable because ECX is Draw's `this`. This is the same as the old OptTick, and the `g_enabled` test now sits after the compare, which is better.

### 2. No raw 0x00[4-B]xxxxx literals: PASS (with one nit)
- `rg 0x00[4-B]xxxxx` over `mod_config_src/*.cpp|h` and `HolsterEditor.cpp` finds nothing. `gameaddr::` names are used (`OptionsInGameVtable`, `LabelVtable`, `ButtonVtable`, `ExoStringAssign`, `GuiTextObjRefresh`, `OptionsSetDescription`, `ResManGlobal`, `SceneViewVtable`).
- `0xE70100u` (`:155`) is a saferead site id, not an address.
- Nit, F8: comments carry bare unprefixed engine addresses. They are invisible to the regex but are still raw addresses: `ModConfig.cpp:14,16,17,311,319-322,345,817,1064,1128,1208` (8A16FE, 40EC90, 8A1930, 40EC00, 8A2320, 416EA0, 7337C0, 417800, 885AB0).
- The 0x008Axxxx values in `kotor2-steam-aspyr.hooks.toml` are hook metadata, which is expected.

### 3. No file I/O in KPatchInit: PASS
- mod-config `KPatchInit` (`:1241-1252`) does only `KpLog_Init`, a log line and two `provide` calls. The ini is read lazily by `readConfig()` on the first `McBind` (`:1066-1072`). DllMain is a no-op.
- holster-editor `KPatchInit` (`HolsterEditor.cpp:240-269`) does inits, `connect()` (registry lookups) and `Register`. `readConfig()` is called only from `cbBuild` (`:119-123`).
- `g_fallbackSet` (static init, both files) only sets the fallback log name, as before.
- `mod_config.ini` says "Read at game start", but it is read at the first Options panel. Cosmetic.

### 4. NewPage/AddButton/AddLabel/AddCustom gated on g_inBuild: PASS
- `muiNewPage` checks `!g_inBuild || g_buildSlot < 0 || !g_mc.panel` (`:924`) and then that the panel pointer matches (`:929`).
- AddButton, AddLabel and AddCustom call `buildGate` (`:952`, `:975`, `:996`), which checks `g_inBuild` (`:911`) and that the page equals `2 + g_buildSlot` (`:916`).
- `g_inBuild` is set only around `g_ent[s].build()` in `McBind` (`:1118-1121`), and cleared at McBind entry (`:1085`). Refused calls are logged and counted (capped at 8).

### 5. Clones only in McBind: PASS
- `cloneEntry` and `cloneLabel` are called from:
  - McBind (`:1097`, `:1105`, `:1109`, `:1110`);
  - `muiAddButton` / `muiAddLabel` (`:960`, `:983`), reachable only through `buildGate`, which only passes inside McBind.
- McTick and the other API calls (SetText, SetRect, SetDesc, PaneRect, Clicked, Control) never create a control.
- mod-config clones its own root, list, Back and title before any `build()` (`:1097-1110`), as `ModConfigApi.h:12` promises.
- Note: the clones are now made before the consumer's embedded layout open/close. The old code did it the other way round. This looks safe (the clone only reads the vanilla layout), but PRB1 never covered this order.

### 6. McDestroy order: PASS
`ModConfig.cpp:1209-1237`:
1. Leave the shown page: `callEntryLeave` when the state is ENTRY (`:1217`).
2. `unlinkOwn(P)` (`:1219`) covers ALL items, including MUI_NOFREE:
   - `dropRefs` on every item with a control (`:845-849`);
   - truncate gated on `mui::canTruncate` (`:866`), then `unlinked = true` for all (`:877-879`).
3. Each built, live entry's `destroy(P, user)` (`:1221-1222`). `Control()` is already null for unlinked items by then.
4. `freeCtl` (vt[0](1)) only when `!it.nofree && clean && freeLabels && it.unlinked` (`:1225-1233`). MUI_NOFREE is skipped at `:1227`. `freeCtl` re-checks the label or button vtable (`:820`).
5. DTOR log (`:1215` and `:1235`), then `resetState()`.
- Cases covered: a refused unlink gives `clean = false`, so nothing is flagged unlinked, `Control()` keeps answering the pointer, and nothing is freed. A never-listed panel gives `clean = true` (see F5).

### 7. Release-edge navigation, vanilla restore waits for LMB up: PASS (one gap, F3)
- Clicks come from `ClickTracker.tick(down, hit)` and fire on the release tick (`:1175-1176`). The page change is made on that tick (`:1182-1196`) and the trackers are reset in `pageChange` (`:710-712`).
- `down` includes the foreground gate (`:1165`).
- Back on LIST only arms `leaveAt = now + 100` (`:1195`). `pageChange(... back())`, which runs `restoreVanilla` on ENTER(MAIN), fires only when `!down` and the delay has passed (`:1198-1201`).
- Back on ENTRY goes to LIST immediately, which is fine because the vanilla BTN_EXIT is still hidden.
- Gap: the tick-gap reset (`:1153` -> `resetPages` `:717` -> ENTER(MAIN) `:654`) restores vanilla without testing LMB up. See F3.

### 8. canTruncate before any truncate: PASS
- The only count write is at `:868`, inside the `else` of `if (!mui::canTruncate(...))` (`:866`).
- Before that it checks `C >= V`, `C <= V + nOwn` and `C <= cap` (`:859`).
- `!e.appended` skips the truncate entirely (`:851`).
- The listed-array buffer is `kMaxVanilla + kMaxOwn + 8` entries (`:857`), and the `C > cap` guard prevents overrun.

### 9. Bit-only flag restore: PASS
- `restoreVanilla` writes `(cur & ~mask) | (saved & mask)` with `mask = FlagVisible | FlagClickThrough` (`:632-633`).
- `hideVanilla` changes only those two bits (via `guikit::setVisible`, `GuiKit.h:257-262`).
- The saved sentinel is -1 (`:614`). A flags word of exactly 0xFFFFFFFF would be skipped, which is unrealistic.

### 10. Consumer: PASS
- No hooks: `holster_editor_src/kotor2-steam-aspyr.hooks.toml` has `hooks = []` (same shape as `preview_3d_src`). `exports.def` lists only `KPatchInit`. The source has no `extern "C"` handler other than `KPatchInit`.
- `Register` is called in `KPatchInit` (`HolsterEditor.cpp:266`).
- FREEVIEW runs only if `g_freeView && view` and `Control(page, viewItem) == nullptr` (`:211-214`). Otherwise it logs "skip linked".
- `Destroy(token)` runs in `cbDestroy` whenever `e.token && g_p3.ok()` (`:210`), before the FREEVIEW decision. It also runs on the build-failure paths (`:161`, `:171`).
- Caveat on "always" (F6): if mod-config's panel changes without a dtor, `cbBuild` drops the old token without `Destroy` (`:138-139`).

## Additional checks

- Null derefs on a missing or failed entry or an empty page:
  - `entLive` guards every `g_ent[...]` access: `entryClicked` (`:787-792`), `descFor` (`:757`), `callEntryLeave`, the enter/tick/destroy callbacks (`:699`, `:1204`, `:1222`), and `McBind` (`:1117`).
  - Callbacks are null-checked. A `g_reg.at()` of 0 gives slot -1, which `entLive` rejects.
  - `itemAt` and `shownItem` bound-check page and item. An empty page is refused with an "ENTRY ... empty" log (`:789`) rather than entered.
  - No null deref found.
- Out-of-bounds on the 8-entry / 24-item / 10-page limits: none found.
  - `kMaxPages = 2 + 8` pages are created at `:1092`.
  - `pageMade`, `built`, `pageTitle` and `items[k]` are indexed by `g_buildSlot` (0..7) or by `page - 2` after `buildGate`.
  - `AddButton` and `AddLabel` check the item limit before cloning (`:956`, `:979`). `AddCustom` relies on `addItem` returning -1.
  - `nList` is capped at 8 (`:1098`). `kMaxTargets = 32` against about 16 used. `kMaxVanilla` caps the `savedFlags` loops.
- State reset between Options openings: PASS.
  - `McBind` calls `resetState()` (`:1087`), so the stack is at MAIN, with fresh trackers, `appended = false` and `haveSaved = false`.
  - `McDestroy` also calls `resetState()` (`:1236`).
  - The consumer resets `g_ed` in `cbBuild` and `cbDestroy`.
  - A tick gap over 500 ms returns to MAIN (`:1153`).
- Hover and desc writes: PASS. `hoverStep` is edge-driven (`HoverTracker.update`, `:766`). `writeDesc` is called only on a hover ENTER (`:776-779`), on leave-to-none on LIST (`:781`), and once on ENTER(LIST). Per-frame work is limited to `applyTargets` drift checks (about 16 targets, re-apply capped at 8 each) and `hitItem`.
- Panel pointer use after dtor: PASS.
  - `g_mc` is reset in McDestroy, so a later Options draw takes the `P != e.panel` path and returns.
  - `muiPaneRect`, `muiNewPage` and the consumer callbacks compare the panel pointer.
  - The API calls (`itemAt` after `resetState`) get itemCount 0, so they return null, not a stale item.
- Entries registered after the first McBind: handled by design. The entry list is snapshotted per McBind (`:1098-1108`), so a late registration appears on the next Options open (F4 covers the slot-reuse hole).

## FINDINGs (non-blocking)

- F1. `SetDesc` is not gated against the ctor window. `muiSetDesc` (`:1034-1037`) calls `writeDesc` -> `describe` -> 8A2320 at any time with a bound panel, including inside `build()`. That runs the vanilla description function on a half-constructed panel (before StopLoadFromLayout). The holster does not do this, but the API invites it. Suggest `if (g_inBuild || !g_mc.appended) return;`.
- F2. PaneRect(DESC/VIEW/STATUS) is measured live (`:429-436`), and 8A2320 relayouts the LB_DESC (its "rect, relayout" steps, `:322`). `cbEnter` runs after the LIST page has written descriptions, so VIEW and STATUS placement now depends on the LB_DESC rect after those writes. The old code measured once at the first tick, before any describe. Suggest caching the DESC rect in `firstTick` next to `e.col`.
- F3. Tick-gap reset does not wait for LMB up. `resetPages("tick gap")` (`:1153`) runs the ENTER(MAIN) restore at once, which un-hides and re-enables the vanilla BTN_EXIT. If LMB is held across a gap over 500 ms (alt-tab, a hitch while the 3D scene builds), the release lands on vanilla Exit. Suggest deferring the restore until `!down`, like the Back path. A hitch over 500 ms while the preview scene loads also spuriously resets ENTRY to MAIN; this is the old behaviour.
- F4. Registry slot reuse while a panel is open. `mcUnregister` followed by a `Register` into the same slot makes `entLive(slot)` true for a different entry. `built[slot]` and `listTok` are stale, so the new entry gets `enter`/`tick`/`destroy` for a page it never built. This needs an Unregister during an open Options screen, which no current client does. Suggest recording a generation counter per slot in `McState`.
- F5. The `!e.appended` shortcut (`:851-852`) marks every item unlinked, including a MUI_NOFREE control the consumer had already listed itself (the case `appendItem` allows for, `:502-508`). A consumer with `free_view=1` could then free a control that is still in the list. Only reachable if the panel dies before its first draw. Low risk, but the assumption "nothing is listed" is not true for pre-listed NOFREE items.
- F6. The holster `cbBuild` reset (`HolsterEditor.cpp:138-139`) drops a stale `g_ed.token` without `Destroy` if the previous panel never reached `cbDestroy`. `cbDestroy` also returns early when `panel != g_ed.panel` (`:205`). Both are leaks only (the preview creature stays). Same as the old code.
- F7. Dead list entries. A list button is created for every registered entry. If `build` fails or the consumer is disabled (`holster_editor.ini enabled=0`, `HolsterEditor.cpp:124`), `pageMade` is false or the page is empty, and the button logs "ENTRY ... empty" (`ModConfig.cpp:789-791`) and does nothing. Suggest not listing entries whose build made no page, or showing them as disabled. Also, after Back from LIST to MAIN the pane still shows "Choose a mod..." until a vanilla hover rewrites it (cosmetic).
- F8. Bare hex addresses in comments (see checklist item 2). Convention is `gameaddr::` names in comments too.
- F9. `AddCustom` without MUI_NOFREE sets `nofree = false` (`:1003`). mod-config then takes ownership: label and button vtable controls are freed on McDestroy (`freeCtl`), while any other vtable is silently left alone. That is a safe failure mode, but `ModConfigApi.h` should state "flags 0 = mod-config frees labels and buttons".
