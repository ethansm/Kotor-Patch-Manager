# U4 recon A: marker pool mechanics (menu-ui SetRect per frame)

Scope: read-only recon of the current tree. Anchors are `file:line` in the shared worktree. Nothing built or measured.

## Verdict
- Per-frame SetRect does NOT trip the reapply latch. Every muiSetRect call re-arms it.
- Three blockers are not latch issues: item cap (38 of 40 used), visible labels swallow clicks, pane clipping unknown.

## Facts (anchors)
1. The latch counts only engine drift. `applyOne(initial=false)` bumps `t.applied` and `e.relayouts` only when the engine extent differs from the stored target between two McTick calls. `ModConfig.cpp:283` (`++t.applied`), `:284` (`++e.relayouts`). Gives up at `:277-278` when `applied >= kMaxReapply` (`ModConfig.cpp:67`, field `:102`).
2. Our own writes reset the latch. `muiSetRect` does `it->tg = Target()` (`ModConfig.cpp:1037`, clears `applied` and `gaveUp`), then `applyOne(initial=true)` (`:1041`). Changed or identical targets both re-arm it.
3. Identical rects are a no-op. `applyOne` returns at `:275` when the extent matches, so a repeated SetRect costs 4 extent reads and no vtable call (`GuiKit.h:227-238`, `:241-250`).
4. Per-frame order. McTick runs before the panel draw (`ModConfig.cpp:13-16` header). Then `applyTargets` (`:1176`), then the ENTRY item drift loop (`:1179-1181`), then `g_ent[k].tick` = editor `cbTick` (`:1226`, ENTRY only). An editor SetRect in tick N is drift-checked in tick N+1.
5. ENTRY enter clears the latch (`:700-701`), shows all items (`:704`, `setVisible(true)` at `:479`), then calls `enter` (`:709`). Editor registers `cbEnter` at `HolsterEditor.cpp:2155`; `cbEnter` is at `:1479`.
6. Live evidence: `mod-config_log.txt` and `.1` contain 0 RELAYOUT lines and 0 "gave up" lines (checked via `kplog_parse.py --bare`). That includes two holster-editor ENTRY lines with `items=22`. PLAY_TEST_QUEUE PRB1 (10-16) also records `RELAYOUT 0`.

## Protocol (recommended)
- Call `placeRect` (muiSetRect) for every marker on every LIVE tick, inside `cbTick` (`HolsterEditor.cpp:1714`; the LIVE branch runs `SetView` each tick).
- Do NOT add a 1 px gate. `applyOne` already gates equality. A gate would skip the latch reset and let drift accumulate on a stable marker. If cost ever matters, use a keep-alive of at most every 4 ticks.
- Hide = park at `{-9999,-9999,0,0}` (`HolsterEditor.cpp:1103`). `K2MenuUiApi` has no hide call (`ModConfigApi.h` SetRect at `:97`, Control after PaneRect at `:100-103`). Zero size never hits (`GuiKit.h:428` containment test).
- Build markers with `AddLabel` after the view `AddCustom` (`HolsterEditor.cpp:429`). Later items draw on top (`GuiKit.h:8-9`, "draw order = panel list index").
- Projection space: `worldToScreen` takes `ext` = control extent (`HseView.h:161`). Live CAMHOOK `ext=(1345,357,576,896)` equals the view pane, so marker rects and projection share one space.

## Blockers and risks
1. CAPACITY (blocker). `kMaxItems = 40` per page (`MenuUiLogic.h:36`). `muiAddLabel` refuses at the cap and returns -1 (`ModConfig.cpp` muiAddLabel, "page is full"). Editor base 26 items (`HseView.h:732`) + inspector 12 (`:731`) = 38. Only 2 free slots with the inspector on. Markers need 5 (bone, mount, x, y, z). Fix: raise `mui::kMaxItems` to 48 (`MenuUiLogic.h:36`) and `kPageItemCap` (`HseView.h:734`) together. That is a mod-config DLL rebuild and install, so it needs user GO. Added McState is about 8 x 8 x ~340 B = ~22 KB (`ModConfig.cpp:131`, ItemX `:105-117`).
2. CLICK-SWALLOW. `setVisible(true)` clears bit 0x20 (`GuiKit.h:264`), so shown markers capture engine clicks over the view. Mod-config's own `hitItem` skips LABEL kind (`ModConfig.cpp` hitItem), so the mod logic is unaffected. Fix: in `cbEnter`, call `guikit::setClickable(ctl,false)` (`GuiKit.h:251`) on each marker ctl from `Control()`. `setShown` runs only on state transitions (`ModConfig.cpp:596-704`), so once per enter is enough.
3. PANE CLIPPING: UNSURE. No clipping in `muiSetRect`/`applyOne` (`ModConfig.cpp:1033-1041`, `:271-289`). No repo evidence that the engine clips panel children. Keep markers inside `ext` and park any outside it. A probe (a label 40 px past the view edge) would settle it.
4. Stale docs. `H2.md:10` and `:31` say gaveUp blocks muiSetRect silently. That is REFUTED by `ModConfig.cpp:1037`. Only the McTick correction stops. Correct it when merging. `S1.md:22` cites parking at `HolsterEditor.cpp:481`; it is now `:1103`.
5. Latch freeze. If the editor stops calling SetRect on a marker while the engine rewrites it (8 drift events), the marker freezes at the engine rect until the next SetRect or page re-enter (`:700-701`). Calling every tick prevents this.
6. Cost (NOT MEASURED, estimate). Drift loop already does 4 reads per ENTRY item every frame (22-38 items). Five markers add ~20 reads plus at most 5 vtable calls when a rect moves. `beginScope()` in `muiSetRect` (`ModConfig.cpp:1034`) clears the 4-slot caches (`SafeRead.h:21`, `:37-40`), so each call re-probes. Expect low microseconds per frame.

## Parking evidence
- `applyLayout` parks about 20 items each nav-mode build (`HolsterEditor.cpp:1103-1137`, `:1180-1187`). Live PRB1 pass (10-16) shows RELAYOUT 0. Negative and zero-size rects are accepted without engine rewrite.

## Next action
- Resolve the capacity blocker first (raise kMaxItems to 48 with user GO, or cut markers). Then build markers in `cbBuild` after the view item, with the per-tick protocol above.
