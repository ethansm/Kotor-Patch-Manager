# Review B: holster W3 helpers + provider (96f96d6, 03fc2ae, 7c94d35, 6ad23e8)

Contract: review. Decision: APPROVE WITH CHANGES (no blocking; fix B1/B2 before treating k2.preview3d@1 as stable).

## Commands run (all verified)
- `python3 scripts/run_tests.py hse_view view_drag common_contract holster_editor mod_config`: 35 passed, 0 failed (the `common_contract` filter pulls every mod's contract test; hse_view, view_drag, holster_editor and mod_config contract tests all PASS).
- `python3 scripts/check_raw_addresses.py`: FAIL=0 WARN=20 INFO=18 (all pre-existing stale-allowlist WARNs in other mods; HseParty site ids 0xE70110-0xE70116 collide with nothing, 0xE70100 is the pre-existing shared rd() site).
- `python3 scripts/check_shared_headers.py`: 47 shared headers, 0 drifted (ViewDrag.h copy + forwarder OK).
- `python3 scripts/compat_check.py` on ViewDrag.h, GuiKit.h, HseParty.h, HseView.h, Preview3dProvider.cpp, ModConfig.cpp: PASS 45 FAIL 0. GuiKit.h includers: 13 mods; only additive Engine fields (null-defaulted), no test or build break.
- `python3 scripts/build_check.py --mod preview_3d_src --mod holster_editor_src --mod mod_config_src`: PASS x3.

## Findings

### B1 MAJOR: PlayAnim contract broken in Preview3dProvider.cpp:604-615 (header doc Preview3dApi.h:82)
Header says "Null or empty = no-op". Commit 03fc2ae changed `if (!v || !anim || !anim[0] || !v->built) return;` to only check `!v || !v->built`, and line 609 now maps null, empty AND the string "idle" to the idle anim. Failure: any other k2.preview3d@1 consumer that passes null/"" as "do nothing" (or an anim literally named "idle") now restarts/cross-fades the idle on every call. Only holster-editor consumes the API today, so nothing breaks in-repo, but the table version stays 1 and the doc is now false. Fix: either keep the no-op (add a separate explicit idle, e.g. append `PlayIdle` after ViewControl via struct_size growth) or update Preview3dApi.h:82 and bump the doc to state null/empty/"idle" = idle, and pin it in preview_3d_src/common_contract_test.cpp.

### B2 MAJOR: chooseIdle hardcodes "pause1" for every rig (Preview3dProvider.cpp:212-219)
Old logic (idleFor, lines 209-211) picked pause1 for droids and evil/neutral/good by alignment for everyone else. New code ignores appearance and alignment, and additionally calls playPart on body and head inside every Rebuild (before applyView). Failure: non-droid rigs whose idle set lacks `pause1` (unverified for wookiee/other bodies) get an unresolved anim name; alignment idles are lost for all consumers, not just the editor. `idleFor`, `isDroidRow`, `appRow` (lines 207-211) are now dead code (build still passes, no -Wunused-function gate). Fix: keep `idleFor` as the default and let the editor request pause1 through PlayAnim (it already does at HolsterEditor.cpp:666), or at least fall back to idleFor when pause1 probe (`vt[0x1c]` existence check, returns -1.0 when missing) fails; delete dead code if truly dropped.

### B3 MINOR: ViewDrag.h duplicates preview_3d_src/Preview3dLogic.h in the same namespace (ViewDrag.h:1-89 vs Preview3dLogic.h:11-149)
Same `p3d::` names (kMaxSceneFov, fovForZoom, yawVec, zoomStep, yawDrag, panDrag, dragAxis, ViewState, DragCfg, Drag, dragBegin/End/Move) with different DragCfg defaults (panPerPx 0.003 / pan +-1.2 vs 0.01 / +-1). Failure: a TU including both headers fails with redefinition errors; the defaults drift silently. The ViewDrag.h:1 comment ("shared between equip3d, preview-3d and holster-editor") is false: only HolsterEditor.cpp includes it. Fix: make Preview3dLogic.h include ViewDrag.h (keep the provider's cfg values in its own DragCfg init) or drop the duplicate, and fix the comment.

### B4 MINOR: holster-editor wheel hook not cleared on leave (HolsterEditor.cpp:539-541, cbLeave ~548-565)
cbEnter sets the rect and touches the hook; cbLeave never calls `g_wheel.clearRect(0)`. The hook is "fresh" for 300 ms after the last tick, so wheel notches over the old rect in that window accumulate and the first tick after the next Enter applies them via `take()` (line 763) as a zoom jump. Fix: `guikit::g_wheel.clearRect(0)` in cbLeave and discard `take()` once in cbEnter.

### B5 MINOR: shift/ctrl read by raw GetAsyncKeyState, new GuiKit hooks unused (HolsterEditor.cpp:758-759; GuiKit.h:94-96, 706-707)
`shiftDown`/`ctrlDown` were added to guikit::Engine and installed, but nothing uses them; the editor calls GetAsyncKeyState directly and does not gate on `fg`, unlike lmb/rmb/mmb. Failure: Shift/Ctrl held in another window still scales nudges/depth while the game is not foreground (low impact). Fix: use `g_gk.shiftDown`/`ctrlDown` gated by `fg`, or delete the two unused fields.

### B6 MINOR: inf handling differs from sensible clamp (HseView.h:36-37, 54-55; test line 42)
Spec vectors all pass. Spec only defines NaN; implementation maps +/-inf to 2.2 (clampZoom) and 0 (makeView panZ), and the test pins that. Failure: an ini `view_zoom=1e999` (atof gives inf) yields 2.2 instead of 4.0. Harmless; note only. `wrapYaw(-1e-30f)` correctly returns 0 (checked by code path). `wrapIndex` can overflow for |i+delta| near INT_MAX (not reachable: member count <= 3).

### B7 MINOR: view_drag_test.cpp covers only the yaw lock path
No cases for panDrag / axis 2 (vertical lock), `dragAxis` tie (goes to yaw), inactive drag returns false, zoomStep clamp at zoomMin/zoomMax, dragMove with unchanged cursor. The behaviours the doc comments promise (ViewDrag.h:75-76, 89-91) are untested. Fix: add 5 checks.

### B8 MINOR: common_contract_test.cpp is a real pin (not filler) but misses what the mod uses
Not copy-paste (<=16 identical lines vs other mods; pins struct sizes, null-slot rejection, connect/poll for k2.modconfig/menuui/preview3d/holster; guikit::hit edges; party chain with a mmap arena; readExo cases). Gaps vs actual usage: MUI_PANE_COLUMN == 0 (only STATUS and VIEW pinned; the editor uses COLUMN for the strip), K2PREVIEW3D_HAND_*/PART_* and K2HOLSTER_SCOPE_* values, K2HolsterEditInfo layout, `g_gk.gameForeground/lbuttonDown/rbuttonDown/mbuttonDown` and `g_wheel.setRect/touch/take`, the HseParty chain inputs (`gameaddr::GetLocString/GetClientLanguage/ExoDtor/AppGlobal`, `K2AddrTable::Creature_Stats == 0x1198`), and `readExo(site, addr, buf, 256, 255, false)` (the call HseParty makes; test only exercises maxLen 80 / printableOnly). Also `k2addr::setForTest(&kSteam)` is set but nothing asserts from it. Fix: add those ~10 checks (all host-testable).

### B9 MINOR: holster_editor.ini text (holster_editor.ini:1, 8)
Header says 0.2.2-dev while manifest is 0.2.4-dev. `view_yaw` comment states "180 = looking at the character's back where holstered weapons sit", but spec section 2.7 says the facing side is NOT proven (user tunes by play test). Fix: neutral wording, current version.

## Verified OK
- HseView.h: every c20 test vector holds (hse_view_test.cpp covers all, plus scopeLabel from 7c94d35); NaN/inf paths safe; memberLabel always NUL-terminated, cap<=0 and null out are no-ops; half/leftStrip clamp negative widths to 0.
- HseParty.h (c21a, section 2.4): chain matches spec exactly ([S+8]==5 byte, stats=[S+Creature_Stats] non-zero, H=stats+0x34, [H+0] non-zero, AppGlobal -> +4 -> +4 guard, GetClientLanguage fastcall, GetLocString thiscall with exo[2]); every read is `saferead::readAt`/`readExo` with a distinct site id; `beginScope()` first; `out[0]=0` set before every false return; `ExoDtor` runs whenever `exo[0] != 0` on every path after the call (ok true or false, text read or not); no call happens with null H/stats/app. Names longer than 255 bytes are rejected (readExo maxLen), acceptable.
- ViewDrag.h logic as used: a press that starts outside the view never starts a drag (HolsterEditor.cpp:870-878 sets `pressed` on first tick, begins only if `insideView`; held press moving into view stays inactive); release and focus loss both end the drag (`lmb = fg && lbuttonDown`, else branch calls dragEnd at 888-890); wheel zoom direction/clamp correct (`zoomStep(.., 0.9, kZoomMin, kZoomMax)`, +120 zooms in).
- ModConfig.cpp:1163-1166 (6ad23e8): consistent with CTOR's `flags48 & 2` visibility test (line 1097). `resetPages` -> `pageChange` -> `applyEvent(ENTRY, leave)` fires `callEntryLeave` exactly once, then state is MAIN so later hidden ticks are no-ops; the consumer's `tick` cannot run in the same call (early return, and tick only runs in ENTRY). Skipped while hidden: firstTick/applyTargets/click handling (intended). Deferred `restorePending` completes on the next visible tick. Residual: McTick is a hook-only path with no host test for the new branch (MINOR; the page-state transitions themselves are covered by menu_ui_logic_test).
- GuiKit.h additions are additive null-defaulted members; `rebind()` does not touch them; other includers unaffected.
