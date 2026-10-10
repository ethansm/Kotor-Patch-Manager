# 19b. Cross-patch sharing survey (2026-10-02)

**Single source for the Common extraction (since 2026-10-03).** `COMMON_EXTRACTION_PLAN.md` (4e2fcef) was folded into section D below and is now a pointer. Corrections from that plan: the shared logger is doc 19's `KpLog.h` (not `Log.h`); the window/wheel helper is registry interface A1 `k2.guiinput` (not a `WindowHook.h` header).

Companion to doc 19. Question (user): can the RFC #180 registry simplify our patches with shared functions? Survey of all `*_src` patches by a Sonnet agent. Main checked the key claims on disk: 4 WndProc subclasses; identical icon tables (md5); 0x84D82C/0x84D871 69 bytes apart; GuiKit used only by level-up. Line numbers are as of 10-02 and drift as sibling sessions edit.

**Answer:** two different tools.
- **Shared runtime state or hooks** call for the registry (bucket A).
- **Duplicated helper code** calls for headers in `Patches/Common` (bucket B, as Phase 0 did with SafeRead/GuiKit).

**Upstream constraint:** none of the surveyed patches exist upstream (all are local-only, checked against upstream/master). A real patch-to-patch example therefore cannot ship in the #180 PR unless its patches go upstream too.

## A. Registry candidates (runtime, patch-to-patch)
| # | Candidate | Patches | Benefit | Interface sketch |
|---|---|---|---|---|
| A1 | **Game window + wheel/mouse input** | inv3d `Inventory3DViewport.cpp:473/498/764`, equip3d `EquipCharPreview.cpp:337/361/1099`, effects `CharEffectsPanel.cpp:228/252/1229`, hotbar `ForcePowerHotbar.cpp:738-750/1302` | 4 stacked `SetWindowLongPtrA` subclasses become 1. Detach is only safe "if still on top" (hotbar:1302 comment), so stacking hazards go away. One EnumWindows scan, foreground check and cursor-to-engine transform. ~25-30 LOC less per consumer. | `k2.guiinput` 1: `game_hwnd`, `cursor_engine(x*,y*)`, `wheel_subscribe(cb,ud)`/`unsubscribe`, `foreground` |
| A2 | Character-screen co-tenancy | level-up (ctor 0x84D82C, tick 0x84F221; `LevelUpUi.h:7-14`) + effects (ctor 0x84D871; `FirstId=69` `CharEffectsPanel.cpp:65`) | Replaces coordination by comments and magic numbers (id ranges, x-ranges, append order) with one ctor hook and one tick, plus id and rect reservation | `k2.charscreen` 1: `on_bind/on_tick/on_destroy`, `reserve_ids(n)`, `claim_rect` |
| A3 | Engine state | perf_lab `PerfLab.cpp:59-61,723-753` + load_hang_probe `LoadHangProbe.cpp:44-52,150-170` (same AppGlobal/LoadScreen/GuiMgr/Modals constants; both poll); hotbar modal stack `:185-200` | One poller; load_hang already owns the MainLoop hook 0x781BE0 | `k2.enginestate`: `loading`, `frame`, `main_tid`, `modal_top` |
| A4 | Active-effect snapshot | buff HUD `BuffDurationHUD.cpp:141-262` + effects `CharEffectsPanel.cpp:658-690` (same creature+0x148/0x14C scan, same tables; both scan the leader each frame on the char screen) | One scan per creature per frame | `k2.effects`: `snapshot`, `world_time`, `classify` |
| A5 | Kite stance | ranged_ai E-H3 0x6D850F writes `[ebp-0x174]`; companion_ai A1 0x6D853E reads it (`CompanionAI.cpp:870-876`); stance 16 hard-coded in both | Fixes a silent miss when ranged_ai is absent or `ranged_rules=0` | `k2.rangedai`: `kite_stance`, `maps_to_ranged` |
| A6 | Image wrapper pools (low) | effects `:288`, hotbar `:423`, buff `:124` | Small (8-byte wrappers) | `gfx.image_for` |

## B. Compile-time candidates (Patches/Common), by duplicated LOC
1. ~600: `EffectIconTable.inc`/`DebuffIconTable.inc` byte-identical in buff HUD and effects; also `currentTime`, `isItemAbility`.
2. ~450-550: GUI plumbing re-implemented in 6 patches although GuiKit.h exists (only level-up uses it). Covers `bindCover`, `cursorEngine` (identical x3), `syncScreen`, `ctlRect`, `setCtlVisible`, `textObjOk`, `setFill`, `imageFor`, `setLabelText`, `gameWindow`.
3. ~200: reloadable ini skeleton (`trim`, `parseFloats`, `maybeReloadIni`, loadIni loops in 8 patches). Overlaps doc 19's kplog.ini reloader.
4. ~130: 3D scene-view construction, inv3d `:590-654` vs equip3d `:942-1005`, with identical constexpr blocks.
5. ~160: code-patch helpers: `writeCode` (x5), `patchIat` (perf_lab and load_hang, identical), TX cave builder, `enabled=0` flag parser.
6. ~50: party/leader accessor chain (`[[0xA1B4A4]+4]` -> 0x73FB90/0x7E5DA0/0x77D800) in 5 patches. Upstream GameAPI `GetPlayerCreature`/`GetWorldTime` exist but are bypassed.
7. ~40: `rd`/`wr`, `parseMask`, CExoString read helpers.
8. ~45: panel-keyed slot table (`claimSlot`/`findSlot`).

## Existing ad-hoc coupling (today)
- No DLL-to-DLL GetProcAddress anywhere.
- load_hang_probe reads other patches' log files by a hard-coded, stale name list (`:504-508`).
- Stack-frame coupling ranged_ai -> companion_ai (A5).
- Id, layout and append-order coupling level-up <-> effects (A2).
- Hand-coordinated sites: robustness skips 0x726505 for perf_lab 0x72650B; texmem and perf_lab tx=1 both write 0x4261CF (second writer refused by byte verify); ActivationFix 0x40D69E vs load_hang 0x40D692.

## C. Not worth sharing
- Tooltip and input logic (low similarity).
- TwoDA/Tlk (hotbar only).
- EquipIconFix vs EquipCharPreview, and level-up vs buff HUD in UpdatePortraits: different sites, no shared state.
- companion_ai IniTable (table-driven and tested).

## Recommendation
- **#180 PR:** registry + `kpatch.log` + provider/consumer *test* patches; logging examples use patches that are upstream or come in the same PR.
- **First real patch-to-patch interface: A1 `k2.guiinput` on our fork.** Provider ~120 LOC. Consumers `require` lazily and fall back to their own subclass, so migration is near zero-risk. It goes upstream with those patches (their PULL_REQUEST.md drafts exist). A2 has the highest conflict value but the most work; A5 fixes a latent bug.
- **B1-B3 first for LOC:** shared icon tables, GuiKit adoption, ini helper (fold into the kplog.ini reloader).
- All consumers are sibling-owned: migrate with owner agreement only.

## D. 2026-10-03 survey additions (folded from COMMON_EXTRACTION_PLAN.md)

**3D-viewport menu mods (inv3d / equip / Mod 14): plan + status in `23_menu_3d_shared_migration.md`.** B4 (scene view) shipped as `SceneView.h` (5652226); inv3d 0.2.1-dev is the first consumer.

**Workflow.**
- Canonical headers: `patch_manager_mods/_shared_src/*.h`. `scripts/check_shared_headers.py --sync` copies them to the framework clone
  `Patches/Common/` (branch `ethansm/local-mods`) and rewrites the `Patches/_shared/` forwarders; a plain run reports drift (exit 1).
  Subdirectories and `*.cpp`/`*.toml` (colocated tests) are not synced.
- Header-only, no hooks (Common is a static lib in every DLL). Changes are additive, with defaulted parameters; existing consumers
  (LevelUpOverride for GuiKit) must compile unchanged. saferead site ids are per DLL (hotbar 0x48xx, GuiKit 0x6Bxx, Party.h 0x6Cxx, SceneView.h 0x6Dxx, inv3d 0x0100, combat-hotbar 0x6Fxx, hud-host 0x70xx).
- One mod per behaviour-neutral migration: warning-clean build, host tests, log diff (+ screenshot for visual mods), one install and live check.
  The owning session migrates its mod; never edit a mod another session changed in the last hours (`ls -lt`, `git log -1 -- <dir>`).
- PRs cherry-pick the Common headers they include, at the tested version. Steam constants stay until the PR3 address-DB rows exist, then
  move to `GameAPI/GameVersion` lookups.

**Ranked list (status, refreshed 2026-10-03; live tracking in doc 24).**

| # | Item | Adopters | Status |
|---|---|---|---|
| 1 | `KpLog.h` (doc 19 Phase 2/6) | all 16 mods with a hand-copied `logf` | header merged (KPM local-mods 706b150); pilot mouse-probe 0.2.0 live 10-03; rest = doc 24 C5 |
| 2 | `GameAddr.h` (`namespace gameaddr`) | 19 addresses in 2-8 mods each | DONE hotbar only (10-03); K2Addr in buff/shield/portrait repo copies; intake = doc 24 C6 |
| 3 | SafeRead adoption + B7 `saferead::readExo` | hotbar, equip3d, inv3d (IsBadReadPtr copies); buff/effects partial | SafeRead in 13 mods; holdouts equip3d (14 IsBadReadPtr), level_up = doc 24 C2/C3 |
| 4 | `IniFile.h` (B3) | effects, equip3d, inv3d, sort_filter, ranged_ai, companion_ai, level_up | PARTIAL 10-05: `IniKit.h` (C7, 67ef3c3) covers the trio dialect (effects/equip3d/inv3d) + `.txt` toggles only; the other dialects stay per-mod (doc 32) |
| 5 | wheel/WndProc = A1 `k2.guiinput` | effects, inv3d, equip3d, hotbar | interim header 10-05: `guikit::WheelHook` (C8, a3286fe), per-DLL; `k2.guiinput` stays the long-term owner (doc 32) |
| 6 | `CodePatch.h` (`writeCode(at, expect, repl, n)` with original-byte check, keep state codes 1/0/-1/-2/-3) | activation_fix, equip_icon_fix, robustness_fixes, perf_lab | candidate = doc 24 C1 |
| 7 | B2 GuiKit adoption (+ `showTip`, `inRect`) | hotbar, effects, equip3d, sort_filter, inv3d | 4 adopters (hotbar first); effects = doc 24 C4 |
| 8 | B6 `Party.h` (`controlledCreature`, `partyCount`, `table`) | hotbar, companion_ai, buff, effects | 2 adopters (hotbar first) |
| 9 | `SaveBinding.h` | hotbar (per-save files), level_up (GUID globals) | needs framework save events; stays local |
| 10 | `EmbedTex.h` (was "Textures.h": embedded TGA -> CAurTextureBasic, `ensure`/`heal`, 20-refill give-up) | level_up (1.4.0-dev, first adopter 2026-10-04); hotbar = migration candidate (still its own copy in ActionHotbar.cpp 1503-1575 + HotbarLogic.h tga helpers) | header DONE `_shared_src/EmbedTex.h` (KOTOR-II a111b83, KPM 6015fe4); hotbar session migrates itself |
| 11 | `TestHarness.h` | perf_lab harness, pure-logic tests | doc 20 |
| - | `TwoDA.h`, `Tlk.h` | hotbar | move with #4 if level_up adopts them |
| 12 | `GuiTip.h` (`namespace guitip`: native engine tooltip string `setNative` + custom-box fallback `tick`/`layout`, `readMgr` probe) | level_up (first, 10-04 f33235f / KPM c7a2bb6); candidates Mod 14 (`showTip`), hotbar (`tipLayout`) | landed, additive; native path unverified live (spike `research/companion_levelup/phase7_evidence/UX_native_tooltip.md`); migrations in their own sessions after level_up's live TIPPROBE result |

**Address duplication (input for `GameAddr.h`).**

| Address | Mods | Names today |
|---|---|---|
| 0x00A1B4A4 | 8 | AppGlobal, G_App, GuiGlobal, ItemMgrGlobal, SaveRequestHolder, WorldTimerSingletonGlobal |
| 0x00733780 / 0x00733570 | 7 / 6 | CExoString dtor / ctor |
| 0x00416E30, 0x009876B4 | 5 | CSWGuiText::Set, text vtable |
| 0x0040F620, 0x00919723, 0x00419740, 0x009878BC, 0x0047EB60, 0x009875BC | 5 | InitControl, engine new, label ctor/vtable, LoadImage, border vtable |
| 0x007E5DA0, 0x0077D800 | 3 | party member / controlled creature accessors |
| 0x00A1B4E0, 0x009F42B4, 0x0098C45C, 0x0098B5CC, 0x009A3D34, 0x0099EE14 | 2 | flags, GateGlobal, scene vtables, creature vtable |

Two-mod pairs also include the equip3d/inv3d scene functions, 0x73FB90, 0x5453C0, 0x51AD20, 0x9A5C84 and 0x4261CF/D6/E6. Name collision:
`FnSetAlign` is 0x416FA0 (hotbar) vs 0x85D430 (equip3d). No value mismatches found.

**Logger timestamp families (constraint for `KpLog.h`).**
1. Wall clock via `GetLocalTime` (activation_fix, equip_icon_fix, robustness_fixes, texture_memory_release, effects, equip3d).
2. Relative ms via `QueryPerformanceCounter` (inv3d, level_up, mouse_probe, companion_ai, also effects/equip3d for timings).
3. `GetTickCount` + frame: the hotbar's `[%lu f%ld] ` prefix; ranged_ai writes no prefix at all (`t=` inline).

**Log-parser constraints.** `mods678_research/analyze_play_logs.py` anchors `^H1 t=` in `ranged_ai_log.txt`, so no prefix may be added to
that log. `scripts/check_effect_decode.py` parses the effects panel output. Any `KpLog.h` adoption keeps per-mod formats or updates these parsers.

**Consumer contract tests (user rule, 2026-10-03).** Every mod that includes a Common header owns `<mod>_src/common_contract_test.cpp` pinning the Common semantics it relies on (doc 20 §4.1). A Common change that breaks a consumer fails in that consumer's test, with a `<mod> relies on:` message. Adoptions and Common edits add/extend the block in the same commit.

**Hotbar shared services (2026-10-03):** doc 22 `22_hotbar_shared_services.md`: hook map (hotbar's 8 sites uncontested), `k2.saveevents@1` = item 9's "framework save events" (recommended first library), hotbar as optional `k2.portraitslots@1` / A1 consumer, GOG address hand-off table for K2Addr.h.

**Quiet mods (safe first adopters):** EquipIconFix, ActivationFix, TextureMemoryRelease, RangedAI, MouseProbe, LoadHangProbe.

**Hotbar (Mod 3), Phase X (2026-10-03):** adopts SafeRead, B7 `readExo`, B2 GuiKit setters, `GameAddr.h`, B6 `Party.h` before its Phase D/T.
Logging, wheel/WndProc, textures and per-save hooks stay local. New headers: mutation pass pending (doc 20 mutation gate, no C++ tool yet).
**Status 10-03 01:57:** items 2, 3, 7 (setters + label creation), 8 DONE for the hotbar; build 27 `106be06e` installed, awaiting user smoke test. Common commits: repo 8e0da7c, clone 2c1bba4. Next adopters per the order above (quiet mods first; equip3d/inv3d for SafeRead).

**C2 follow-ups (2026-10-04, proposals for owning sessions; no edits made in these dirs).** C2 added `saferead::readExoLoose`
(tolerant CExoString copy: truncates, page-end fallback, `"?"` when unreadable) and `saferead::formatResRef16` / `readResRef16`
(flags `RR_LOWER`, `RR_SPACE_BAD`, `RR_EMPTY_Q`) to `SafeRead.h`; companion_ai, perf_lab and equip_icon_fix adopted them.
- Hotbar (`ActionHotbar.cpp` ~:222, inline icon resref at `e+0x20`): `readResRef16(S_Entry, e + 0x20, ib, 0)`; keep the
  `g_iconFaults` log on a `false` return. `en.icon = ib` already stops at the NUL, so the output is unchanged. Hotbar session's call.
- equip3d (`EquipCharPreview.cpp` ~:557, helmet model resref): goes with C3 (IsBadReadPtr removal). Its alnum+underscore `okName`
  check differs from the resref formatter, so it keeps a local predicate over `readResRef16(site, item + ItemModelResOff, name, 0)`.
- level_up `readTag` (`LevelUpOverride.cpp` ~:946): `readExo` (strict) or `readExoLoose` (tolerant); owner's call, level-up session.

**C4 follow-ups (2026-10-08, proposals for GameAddr.h additions from level-up-override / queue-hud).**
Level-up override migrated 7 addresses to `GameAddr.h` (`PtrListAdd`, `AppGlobal`, `MemberServerCreature`, `PartyGetAt`, `ClientPartyTable`, `ExoCtor`, `ExoDtor`). The following unmapped addresses have multi-mod utility or verified Ghidra DB symbols and are proposed for additive inclusion in `GameAddr.h`:
- `RulesGlobal = 0x00A1B4D0`: Global pointer to `CSWRules` singleton. Contains `+0x38` 2DA EXPTABLE (51 rows), `+0x104` spell table (consumed by `queue-hud` 0x60 and `level_up_override` 0x100), `+0x108` feat table rows, `+0x11C` feat count, `+0x123` skill count byte.
- `GetPlayerCreature = 0x0073F450` (GOG `0x0040C910`): `CClientExoApp::GetPlayerCreature`, `thiscall(clientApp)`. Contrasts with `GetControlledCreature = 0x007E5D80` (`thiscall(partyTable)`). Used in `CSWGuiInGame::UpdatePortraits` (0x74F423) and leader instant evaluation.
- `GetCharacterChangeInProgress = 0x00740FC0` (GOG `0x0040E480`): `CClientExoApp::GetCharacterChangeInProgress`, `thiscall(clientApp)`. Guards against GUI/portrait updates while party member swaps occur.
- `GetClientOptions = 0x0072FB00` (GOG `0x0040CCF0`): `CClientExoApp::GetClientOptions`, `thiscall(clientApp)`.
- `CanLevelUp = 0x006B9790` (GOG `0x006F5C10`): `CSWSCreatureStats::CanLevelUp`, `thiscall(creatureStats)`.
- `AutoLevelup = 0x006BEDF0` (GOG `0x006FB270`): `CSWSCreatureStats::AutoLevelup`, `thiscall(creatureStats, int notify) RET 4`.
- `AppendToMsgBuffer = 0x007BE090` (GOG `0x004CE720`): `CGuiInGame::AppendToMsgBuffer`, `thiscall(guiInGame, CExoString*, int, uint8) RET 0xC`.
- `CSWGlobalVariableTable`: `FindIdentifierIndex` (`0x006545B0` / GOG `0x00651470`), `AddIdentifierBoolean` (`0x00654430` / GOG `0x006512F0`), `GetValueBoolean` (`0x00654740` / GOG `0x00651600`), `SetValueBoolean` (`0x00654BB0` / GOG `0x00651A70`).
- `ServerInitialize = 0x0051C450` (GOG `0x0064BE70`): `CServerExoApp::Initialize`, `thiscall(serverApp)`.
