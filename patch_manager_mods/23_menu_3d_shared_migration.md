# 23. Menu 3D mods: migration to shared Common and the library framework (draft 2026-10-03)

## Status

inv3d 0.2.1-dev: INSTALLED 2026-10-03 21:00 (DLL swap, `59ed7c77`; rollback `patches/inventory-3d-viewport.dll.bak_1003_2100` = 0.2.0-dev `f0eba5be`); awaiting the user in-game check. New log starts at `inv3d_log.txt` line 10161.
- KPM commits (branch `ethansm/local-mods`): `6544ac5` step 1 SafeRead, `0beffc1` step 2 SceneView, `f1b68d3` step 3 GameAddr names, `2391b5c` 0.2.1-dev bump. Common: KPM `6312284` (synced from KOTOR-II `5652226`, joint commit with the GOG-table session).
- Step 4 (GuiKit) deliberately skipped for inv3d: no GuiKit helper has identical semantics (section e, "GuiKit not drop-in").
- Gates after step 3 + bump: check_hook_bytes / check_exports `--mod inventory-3d-viewport --kpatch` PASS (0 WARN), check_hook_spans PASS, check_gui_ids PASS, check_subclassers PASS, check_raw_addresses rc 0, run_tests.py 65/65, check_shared_headers 13 headers 0 drift.
- Imports vs the 0.2.0-dev baseline build: only `IsBadReadPtr` and `IsBadWritePtr` gone (VirtualQuery was already imported). Log format strings: identical, plus ONE new line `GUARD(%s): engine address table unknown for this exe` (never on Steam) and the load line now reads `inv3d v2 0.2.1-dev loaded (Common SafeRead + SceneView)`.
- Baseline note: a rebuild of the unchanged 0.2.0-dev source gives DLL md5 `a1e81d5f`, not the live `f0eba5be` (MinGW builds are not byte-reproducible); the baseline is the source, not the binary.
- Registries updated with the code: `raw_addresses.toml` inv3d entry down to the hook sites + `FnRender` (scene addresses now live in GameAddr.h/K2Addr.h); `subclassers.toml` inv3d hints 457/712.

| Mod | Patch id | Manifest | Step | Gate to start |
|---|---|---|---|---|
| Mod 1 Inventory 3D viewport | `inventory-3d-viewport` | 0.2.0-dev (bump to 0.2.1-dev, `manifest.toml:4`) | Step 1 (IsBad* -> saferead) committed in the KPM clone as 6544ac5 (20:28); repo mirror NOT yet synced | now |
| Mod 5 Equipment 3D character | `equipment-3d-character` | 0.1.0-probe | waiting | inv3d accepted + Equip3dLogic.h extraction shipped alone |
| Mod 14 Character effects panel | `character-effects-panel` | 0.1.0-probe | waiting | user accepts build 4c (doc 14 line 26: awaiting test) |

Out of scope: Mod 11 sort/filter, equip icon fix. Related: 19b sections A1/A2/B4/D, 19e (KpLog), 22 section 4.4, `$TOOL/docs/PatchRegistry.md:273` (library patches).

Cite key. All line numbers were checked with `sed -n` on 2026-10-03.
- **I** = `inventory_3d_viewport_src/Inventory3DViewport.cpp` at the 0.2.0-dev baseline (770 lines; `git show HEAD~1:` of the mirror). The migrated 0.2.1-dev file is 717 lines, so I-cites no longer match it.
- **E** = `equipment_3d_character_src/EquipCharPreview.cpp`, repo mirror, 1030 lines. The KPM copy is 1105 lines and has no `Equip3dLogic.h`.
- **C** = `active_effects_panel_src/CharEffectsPanel.cpp`, 1260 lines. Identical to the KPM copy, which is uncommitted (build 4c).
- Shared headers are in `_shared_src/`; `GameAddr.h`, `K2Addr.h` and `SceneView.h` are still uncommitted at the time of writing.

## (a) Duplication matrix

| Piece | inv3d (I) | equip (E) | Mod 14 (C) | Existing / planned home |
|---|---|---|---|---|
| `readAt` (bad-pointer probe) | `117-122`, 6 IsBadReadPtr + 4 IsBadWritePtr at `110,119,302,304,384,627,631,691,695,715` | `132-137`, 14 IsBadReadPtr at `126,134,386,524,540,556,581,610,621,627,634,743,902,904` | `105-106`, already `saferead::readAt` (site 0); only the include `36` and `beginScope` at `1139,1188,1217` | `SafeRead.h:97` probeRead, `:107` probeWrite, `:115` readAt |
| `dumpMem` | `105-115` | `121-131` | none | none: keep per mod, rebuilt on `saferead::readAt` |
| `logf` / `dlogf` | `86-93` / `96-103`, no timestamp, `inv3d_log.txt`, cap 12000 (`:84`) | `103-112` / `113-120`, HH:MM:SS.mmm stamp, `equip3d_log.txt`, cap 15000 (`:99`) | `97-104` (`vlog`), stamp, `chareffects_log.txt`, cap 20000 (`:95`) | planned `KpLog.h` (KPM repo, 19e:31); not built |
| `trim` | `147-151` | `eq3d::trim`, `Equip3dLogic.h:35` (`using` at `E:145`) | `131-135` | `Equip3dLogic.h:35` is the only tested copy; planned `IniFile.h` (not built) |
| `parseFloats` | `152-162` | `Equip3dLogic.h:40` | `parseNum` `136` (single value) | same |
| `loadIni` | `164-238` | `146-174` + `eq3d::parseIniLine` (`Equip3dLogic.h:75`), `clampCfg` (`:113`) | `137-196` (key table `146-155`) | `IniFile.h` (not built) |
| `maybeReloadIni` | `239-248` (also watches `inv3d_debug.txt`) | `175-184` | `197-206` | `FileWatch.h:41` stamp, `:77` changed (hotbar already adopted it) |
| `enumProc` | `452-460`, no size filter | `250-258`, no size filter | `211-219`, no size filter | `GuiKit.h:391` (skips windows <= 320x240) |
| `syncScreen` | `463-467` (honours INI `screen_w/h` override via `screenExplicit`) | `260-264` (uses `eq3d::screenValid`) | `220-224` | `GuiKit.h:40` ScreenW/H, `:355` cursorEngine |
| `wheelProc` | `473-485` (float rect test) | `270-282` (`eq3d::rectContains`) | `230-242` (scrolls a list) | planned `k2.guiinput` (19b:16) |
| `gameWindow` | `487-502`, logs `DRAG: hwnd=`, installs the wheel subclass | `283-298`, logs `INPUT: hwnd=`, installs it | `243-258`, logs `INPUT: hwnd=`, installs it | `GuiKit.h:400` (no log, no subclass) |
| `cursorEngine` (float x, y) | `503-509` | `299-305` | `259-265` | `GuiKit.h:355` (int x, y) |
| DllMain restore-if-on-top | `763-767` | `1023-1027` | `1253-1257` | `k2.guiinput` provider |
| `findSlot` / `claimSlot` | `276-281` / `282-290` (4 slots, newest `seq`) | `228-231` / `233-239` (evicts oldest + `destroyCreature`) | `361` / `362-368` (`bound` flag) | none; semantics differ, keep local |
| `bindCover` / `bindLabel` | `568-586`, returns void | `309-328`, returns ctl | `bindLabel` `268-288`, log text adds `MISMATCH` | none. `GuiKit.h:148` createLabelFromTemplate is a different thing |
| Label setters | none | `setLabelText 344`, `setCtlVisible 380`, `setCtlRect 385`, `ctlRect 365` | `ctlRect 307`, `setCtlRect 313`, `textObjOk 317`, `setText 379`, `setTextColor 390` | `GuiKit.h` getExtent `:197`, setRect `:211`, setVisible `:230`, setText `:243`, setTextColor `:272` |
| `imageFor` | none | one pool (`400` caller) | `292` | `GuiKit.h:284` |
| Scene `modelCount` / `getModel` | `291-295` / `298-300` | `241` / `242-245` | none | `SceneView.h:151` / `:157` |
| `gobGuard` + `gobGetPose` | `301-317`, `318-323` (5 vtable slots `62-65`) | none (own attach-slot checks `524,540`) | none | `SceneView.h:193` gobCheck, `:223` gobGetPose |
| BindXView block recipe | `597-653` (alloc `597-609`, scene init `627-634`, rig `636-649`, camera `650`) | `874-940` (alloc `874-885`, scene init `902-907`, camera attach `917-924`) | none | `SceneView.h:112` createView, `:134` initScene, `:162` addModel, `:179` camera |
| Engine address block | `31-65` | `28-76` | `41-61` | `GameAddr.h:13-56`, `K2Addr.h:144-163` (Steam values `:286-313`) |
| Text fit / marquee / tooltip | none | fixed-width estimate: `TipCharW=21` `398`, `showTip 400-414` | `textPx 800`, `fitText 806`, `marqueeText 817`, `marqueeStep 1033`, `showTip 1086-1115`; width tables `EffectNameTable.inc:733,752,770` | planned `TextFit.h`; none today |
| Icon `.inc` tables | model/fit tables only (`22-24`, not icons) | `EquipWeaponTable.inc` via `Equip3dLogic.h:17` | `EffectIconTable.inc` / `DebuffIconTable.inc` (`33-34`) are byte-identical to `_shared_src/` | `EffectIcons.h:7-8` holds them once; Mod 14 still carries its own copies |

### Shared engine-address table

All Steam Aspyr. Cites are the first line of the constant in each source.

| Address | GameAddr.h name (line) | inv3d | equip | Mod 14 |
|---|---|---|---|---|
| 0x40F620 InitControl | `InitControl` (23) | 35 | 32 | 41 |
| 0x416E30 TextSet | `TextSet` (27) | - | 42 | 46 |
| 0x417F40 SceneCtor | `GuiSceneCtor` (45) | 34 | 31 | - |
| 0x418360 AddModel | `SceneAddModel` (46) | 39 | 36 | - |
| 0x418430 RemoveModel | `SceneRemoveModel` (47) | 40 (used `679`) | - | - |
| 0x4184B0 GetModel | `SceneGetModel` (48) | 41 | 37 | - |
| 0x418990 ControlCtor | `GuiControlCtor` (44) | 32 | 29 | - |
| 0x419740 LabelCtor | `LabelCtor` (22) | 42 | 38 | 44 |
| 0x47EB60 LoadImage | `GuiLoadImage` (32) | - | 42 | 45 |
| 0x733570 / 0x733780 CExoString ctor / dtor | `ExoCtor` (25) / `ExoDtor` (26) | 36-37 | 33-34 | 42 |
| 0x919723 operator new | `New` (21) | 38 | 35 | 43 |
| vtable 0x9A3D34 SceneView | `SceneViewVtable` (49) | 33 | 30 | - |
| vtable 0x9878BC Label | `LabelVtable` (37) | 44 | 40 | 44 |
| vtable 0x98B5CC Gob | `GobVtable` (51) | 60 | 61 | - |
| vtable 0x98C45C camera | `SceneCameraVtable` (50) | 59 | 60 | - |
| vtables 0x9875BC / 0x9876B4 Border / Text | `BorderVtable` (38) / `TextVtable` (39) | - | 42 | 48 |
| 0x9F42A4 / 0x9F42A8 framebuffer | `ScreenW` (16) / `ScreenH` (17) | 465 (literal) | 262 (literal) | 222 (literal) |
| 0x9F42B4 gate global | `GuiGateGlobal` (43) | 31 | 28 | - |
| 0xA1B4A4 app global | `AppGlobal` (13) | - | 45 (`ItemMgrGlobal`) | 52 (`AppGlobal`) |
| Gob slots 0x461490 / 0x4614E0 / 0x459710 / 0x459740 / 0x461530 | `GobSetPosition` ... `GobGetParent` (52-56) | 64-65 | - | - |

Not shared, stays raw: inv3d `FnRender 0x8CD2C0` (`I:47`), equip creature/appearance functions (`E:64-72`), Mod 14 `0x51AD20` and `0x417140` (`C:61,47`). Offsets (scene `0x60`, scene obj `0x74`, camera `0x78`, count `0x80`, block `0x400`, Gob scale `0x1E0`) are already `K2Addr.h` fields (`:155-163`; Steam `:300-313`).

## (b) Target layering

| Layer | Contents | State |
|---|---|---|
| Common, exists | `SafeRead.h` (probeRead/probeWrite/readAt/readExo/beginScope), `GuiKit.h` (setters/getters, imageFor, getExtent, `cursorEngine(int*)`, `gameWindow`, `clickEdge`), `GameAddr.h` / `K2Addr.h`, `FileWatch.h` (stamp/changed), `Effects.h` / `EffectIcons.h` | in use by Buff HUD (`BuffDurationHUD.cpp:25-30`) and Shield Bar (`ShieldBar.cpp:21-26`) |
| Common, NEW | `SceneView.h` (266 lines, test `sceneview_test.cpp` 392 lines): `createView(panel, tag)`, `initScene`, `modelCount/getModel/addModel/removeModel`, `camera`, `gobCheck/gobGetPose/gobSetPosition/gobSetOrientation/gobGetParent/gobGetScale/gobSetScale`, `gateOpen`; engine calls are replaceable function pointers in `sceneview::g_sv` | uncommitted; K2Addr table null or zero field => helper refuses |
| Common, later | `TextFit.h` from Mod 14: `textPx/fitText/marqueeText` plus the `kFontTexW/kFont10TexW` tables. Equip tooltip (`E:398-414`) can then replace its `21 px per char` guess | after Mod 14 accepted |
| Common, small additions | candidate `guikit::bindLabelByTag(panel, tag, expectId)` returning `{ctl, id, flags, extent, reason}` (inv3d/equip `bindCover`, Mod 14 `bindLabel`); the mod keeps its own log line | later, NOT this phase (do it with equip, which has 3 callers) |
| Framework, later | `KpLog.h` (logf), `IniFile.h` (trim/parse/reload on `FileWatch.h`), `k2.guiinput`, `k2.charscreen` (A2) | `KpLog.h` MERGED in KPM (`fc11fa3`, 2026-10-03) but owned by the framework session: inv3d's `logf`/`dlogf` stay local and are marked as the seam; `IniFile.h`, `k2.guiinput`, `CodePatch.h`, `FakeKPatchApi.h` not built |

Stacked wheel subclassers (`tests/static/subclassers.toml`): inv3d (`:17-22`), equip (`:24-29`), effects (`:31-36`), hotbar (`:38-43`). Each installs one proc, keeps `g_oldProc`, and restores only if still on top. `k2.guiinput` (19b:16) collapses the four into one subclass with `game_hwnd`, `cursor_engine`, `wheel_subscribe`, `foreground`. Mod 14 also joins `k2.charscreen` (19b:17), replacing the `FirstId=69` id coordination with the level-up UI (`C:65`; `gui_ids.toml:95`).

Library shape (`PatchRegistry.md:273-310`): a library owns the hook and dispatches; consumers `require` lazily and stay inert on NULL. Reference: `portrait_slots_src/` (`PortraitSlotsProvider.cpp`, `PsDispatch.h`), manifest `kind="library"`, `provides=["k2.portraitslots@1"]`. The consumer side keeps its own subclass as fallback until the library is live (19b:48).

## (c) Per-mod order and gates

| # | Mod | Concerns, one commit each | Gate |
|---|---|---|---|
| 1 | inv3d (0.2.1-dev) | DONE: 1. IsBad* -> saferead + `beginScope` (`6544ac5`). 2. `SceneView.h` for the view recipe, scene init, models, camera, gobGuard/pose/scale (`0beffc1`). 3. `gameaddr::` names (`f1b68d3`). 4. GuiKit: skipped (no identical helper). 5. Contract test `inventory_3d_viewport_src/common_contract_test.cpp` (65 checks + 45 static_asserts). LATER: `maybeReloadIni` on `FileWatch.h`/`IniFile.h`, `logf` on `KpLog.h`, wheel stack on `k2.guiinput` | user OK -> install, play test, log read |
| 2 | equip | 0. Reconcile mirror vs KPM copy (167 diff lines; mirror is ahead, KPM lacks `Equip3dLogic.h`). 1. Ship the `Equip3dLogic.h` extraction ALONE as its own build (bit-identical behaviour; commit `1d0ccab`). 2-5 as inv3d. 6. Tooltip -> `TextFit.h` only after Mod 14 | Extraction build installed and user OK before any Common change |
| 3 | Mod 14 | 1. Drop duplicate icon `.inc` copies, include `EffectIcons.h`. 2. GuiKit setters (`ctlRect/setText/setTextColor`). 3. `TextFit.h` extraction (move, do not change widths). 4. Joins `k2.guiinput` / `k2.charscreen` when they exist | User accepts build 4c; commit the dirty KPM copy first (`+53/-28`) |
| 4 | All three | `KpLog`, `IniFile`, `k2.guiinput` consumers, one mod at a time | Libraries built, live and tested (19b A1) |

Do not start two mods in parallel (rule: finish, verify, commit, then next). Rule of D: the owning session migrates its mod; check `git log -1 -- <dir>` before touching.

## (d) Behaviour-neutral migration recipe

1. **Baseline.** Build the unchanged DLL, record md5. Run (from the repo root, `.venv` python): `scripts/check_hook_bytes.py --mod <id> --kpatch <kpatch>`, `scripts/check_exports.py --mod <id> --kpatch <kpatch>`, `scripts/check_hook_spans.py --mod <id>`, `scripts/check_gui_ids.py`, `scripts/check_subclassers.py`, `scripts/check_raw_addresses.py`, `scripts/run_tests.py`. All must be green or have a known baseline before any edit.
2. **One concern per commit** (the five in section c). After each: rebuild, rerun step 1, and diff the PE imports. Expected: `IsBadReadPtr` / `IsBadWritePtr` gone, `VirtualQuery` present, nothing else new.
3. **Logs.** Every `logf` format string stays byte-identical (grep the diff for `logf(`). A helper that moves code (`bindLabelByTag`, `gobCheck`) returns the data and the mod logs with its old text (I `bindCover` text `H1 cover %s`, C `H1 %s ... MISMATCH`). Compare a game log before and after: same lines, same order, addresses and times aside.
4. **Contract test.** Each mod owns `<mod>_src/common_contract_test.cpp` pinning every Common API it uses, messages starting `"<mod> relies on:"`, built against `../_shared_src` via the mod's `test.toml`. Today only the hotbar has one (`force_power_hotbar_src/common_contract_test.cpp`); Buff HUD and Shield Bar do not, so write inv3d's from the hotbar's template.
5. **Stage, then install only on user OK.** Commit the mod source in the KPM clone first, then `python3 patch_manager_mods/mods678_research/stage_and_verify_install.py stage <dir>` (writes `STAGE_INFO.txt`). Before install: `pgrep -x swkotor2.exe` must be empty; md5 every live DLL before and after; keep a `.bak_<date>_<time>` of the DLL being replaced; `KPATCH_NO_LAUNCH=1 .../KPatchLauncher ... --patches <dir> <all live ids>`. Rollback = copy the `.bak` back and re-md5. Never launch the game.
6. **Repo hygiene.** After the KPM commit, copy the mod source back to its `_src/` mirror so the two match (`diff -q` empty), and re-sync shared headers with `scripts/check_shared_headers.py --sync`.

## (e) Risks

| Risk | Detail | Mitigation |
|---|---|---|
| WndProc restore order | Four stacked subclasses. Each restores only if its proc is on top (`I:763-767`), else leaves a dangling chain if another subclassed later. Moving install/restore text breaks the `subclassers.toml` regexes (`:19-20`, `:26-27`, `:33-34`) | Do not touch wheelProc install/restore in steps 1-5; they move only with `k2.guiinput`, with consumers keeping the fallback path |
| IsBad* -> probeRead semantics | VirtualQuery based: guard pages and NOACCESS refused, not tripped (`SafeRead.h:43,67`); addresses < 0x10000 refused (`:101`); the region cache lives until `beginScope` (`:37`) and must be dropped at every hook entry AND after engine code that allocates (`FnNew`, ctors, `AddModel`) before re-probing new memory (see `C:1217`). `probeWrite` for the 4 inv3d write sites (`384,691,695,715`) | inv3d 0.2.1-dev: `beginScope` at entry of `BindInventoryView`, `ShowItem`, `InventoryTick`, in `wheelProc`, and after `RemoveModel` and the render in `ShowItem` (engine creates/frees scene models there); per-DLL site ids (SceneView `0x6D00`, GuiKit `0x6B00`; Mod 14 uses site 0, give it its own) |
| gui_ids source_regex pins | `gui_ids.toml:73-77` pins `I:56` `ExpectedControlId = 21` and the `LBL_BEVEL`/`LBL_BEVEL2` call text (`I:651`); `:56-61` pins `E:49,50,58,59`; `:89-90` pins `C:65,68` | Do not move, rename or reformat these lines; the id constants stay in the `.cpp` |
| +-8 hook spans | `check_hook_spans.py` compares hook sites with a +-8 byte margin; hook addresses are in `*.hooks.toml`, not in code. Code moves do not change them, but any `hooks.toml` edit does | No `hooks.toml` edits in this migration; run the check `--mod` after each commit |
| GOG table empty for scene fields | `K2Addr.h:328` `makeGog()` (uncommitted, being filled from doc 21b) lists every scene field as `out of scope` (`:440-451`), so every SceneView helper refuses on GOG (returns 0/-1/false/fail code). All three mods ship `kotor2-steam-aspyr.hooks.toml` only, so they never load on GOG; test that helpers go inert, not crash | Cover in `sceneview_test.cpp` (null table) and the contract tests |
| GuiKit not drop-in for inv3d | `guikit::cursorEngine` is `int` (`GuiKit.h:355`), inv3d uses `float` with an INI `screen_w/h` override (`I:463-467`); `guikit::gameWindow` skips windows <= 320x240 (`GuiKit.h:395`), logs nothing, installs no subclass, while `I:487-502` logs `DRAG:` and installs `wheelProc` | Keep inv3d's own enumProc/syncScreen/gameWindow/cursorEngine/wheelProc until `k2.guiinput`. Equip and Mod 14 have the same shape |
| `bindCover` is not `createLabelFromTemplate` | `bindCover` (`I:568`) allocates a label, checks the label vtable (`:575`), binds an EXISTING layout tag with InitControl addToList=1, then checks the bound id and hides the control on mismatch. `GuiKit.h:148` creates from a template tag and fails differently | Add `bindLabelByTag`; do not map it onto `createLabelFromTemplate` |
| Mirror drift | inv3d: resolved (mirror == KPM `2391b5c`). Equip mirror ahead of KPM; Mod 14 KPM copy dirty | Section d step 6; no new commit until `diff -q` is clean or documented |
| Concurrent shared-header edits | 2026-10-03: the GOG-table session edited `K2Addr.h`/`Party.h` while this one added the scene fields; `check_shared_headers.py --sync` would have pushed the other session's uncommitted `Party.h` into KPM Common | Before `--sync`, `git status _shared_src/`; if another session has dirty headers, coordinate by cross-session message (one joint commit), copy only your own headers |
| Stale hints (informational) | `subclassers.toml` install/restore hints for equip (`361`/`1100`) match the KPM copy, not the mirror (`294`/`1023`); `gui_ids.toml:56` equip hint `49-50,59-60` should be `49-50,58-59`; `subclassers.toml:13-14` still says "use Common WindowHook.h" (19b says `k2.guiinput`); `raw_addresses.toml` equip entry lists scene addresses GameAddr.h now covers | Update a mod's registry entries in the same commit as the code that moves them (done for inv3d) |

## Next migrations (same recipe, section d)

1. **equip (Mod 5):** reconcile the repo mirror vs the KPM copy, ship the `Equip3dLogic.h` extraction ALONE as its own build and get the user's OK; then SafeRead (14 IsBadReadPtr sites), SceneView (view recipe `E:874-917`, `modelCount/getModel` `E:241-245`; equip also needs camera attach/FOV slots `0x74/0x44` = candidate `sceneview::cameraAttach/cameraFov`), GameAddr names, contract test.
2. **Mod 14 (after the user accepts build 4c):** commit the dirty KPM copy, drop the duplicate icon `.inc` copies for `EffectIcons.h`, GuiKit setters, then extract `TextFit.h` (move, do not change widths).

## Next action (2 minutes)

User OK -> install the staged inv3d 0.2.1-dev (see the session handoff for the stage dir + install recipe), then the in-game check: open inventory, select items (model shows and fits), drag, wheel, Shift+LMB, MMB, close/reopen, switch party member.
