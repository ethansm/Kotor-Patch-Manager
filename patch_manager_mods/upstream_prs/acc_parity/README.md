# AdditionalConsoleCommands: Steam Windows parity (research R4, read-only)

Date 2026-10-03. Inputs: upstream/master export (`8b33840`, `Patches/Common/GameAPI`, `AdditionalConsoleCommands`, both K2 DBs),
branch Steam DB `addressdb/k2-steam-acc` @4324098 (1,683 functions), vanilla Steam exe (SHA 6A522E71) and the Ghidra text dump
`~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71`. Nothing here was written outside this directory.

Files: `simulate_init.py` (simulator), `simulate_init_output.txt` (raw output, all three DBs), `validate_output.txt`
(`tools/validate-patches.py` on a scratch copy with the draft applied: 0 errors, 0 warnings), `draft/` (hooks TOML + manifest diff).
Rerun: `python3 simulate_init.py <GameAPI dir> branch=<db> up48=<db> gog=<db>`.

## Bottom line

1. No C++ change is needed for Steam. `ConsoleCommands.cpp` has no GOG-only logic (section 1). The PR is a hooks file plus a
   manifest line plus a minor version bump.
2. With the **branch** DB: `teleport`, `freecam`, `addfeat` and all six render toggles resolve every lookup they need and
   should work. `runscript` silently does nothing, exactly as it already does on GOG (section 3, cause is a `classes` table
   bug in both K2 DBs, not a Steam gap).
3. The dependency on the Steam DB PR is narrower than assumed. The upstream 48-row Steam DB already satisfies `teleport`,
   `freecam` and the render toggles. The **one** row that differs between it and the branch DB (of the 48 shared rows, plus
   offsets and globals) is `CSWSCreatureStats::AddFeat`: upstream has **0x006F5B40**, which is a 49-byte level-up helper (the 10-02
   notes call it `PendingLevelUp::AddFeat`; the GOG DB has the equivalent as `CSWLevelStats::AddFeat` 0x7F3B40), not the creature-stats method; the
   branch has **0x006B8280**, the real creature-stats method. So `addfeat` on the 48-row DB calls the wrong function (it never touches the creature's feat list).
4. Upstream bug candidates found (details in section 4): `runscript` has been a no-op on both K2 builds since upstream commit
   `840f515` (2026-06-25); the GOG DB lacks several names the wrappers need.
5. Hooks file name: use `kotor2-steam-aspyr.hooks.toml` (hyphens). `ethansm/local-mods` has `kotor2_steam_aspyr.hooks.toml`
   (underscores); the tooling only globs `*hooks.toml` so both load, but all 19 existing upstream Steam hook files are
   hyphenated (`kotor2-steam-aspyr.hooks.toml`), as are 24 of 25 GOG ones.

## 1. GOG-only logic in the ACC sources

`Patches/AdditionalConsoleCommands/ConsoleCommands.cpp` (316 lines), `exports.def`, `README.md`, manifest:

- No `GameVersion::GetTitle/GetPlatform` switch, no `#ifdef`, no hard-coded address or struct offset. Every game address it
  uses comes from `GameVersion` (`GetGlobalPointer("RENDER_*")`) or from the GameAPI wrappers (`addresses.db`).
- The only literals are engine-independent: `takeStraightLine = 1`, `action = 0x41a00000` (float 20.0 passed to action 5),
  `AddActionToFront(5, 0xffff, ...)` with 28 args, camera mode 7. On Steam, action id 5 is the script
  JumpToLocation command: `FUN_0066a230` (a VM command: pops engine structure type 2 via 0x6FDAB0) is the only call site of
  `AddActionToFront` (0x540CA0) my scan found pushing 5 as the first argument. 0x540CA0 has `ret 112` = 28 args, matching the wrapper typedef.
- `ConsoleFunc` layout is a local struct `{char name[80]; void* funcholder;}` (84 bytes). The Steam constructors 0x4757D0 /
  0x475870 / 0x475910 copy the name to `this+0`, store the holder at `this+0x50`: same layout.
- The `K1 Windows`-only branches in `CSWGui*.cpp` are not on any ACC path (and Steam K2 reports title KOTOR2).
- The TOML command loader (`RegisterCommandsFromToml`) is platform independent (std::filesystem + `PathCasing` +
  `GetModuleHandleA`); it is untouched by DB coverage. Not exercised under Proton yet.
- Hook sites are the only per-build data: `kotor2-gog-aspyr.hooks.toml` has two (detour at 0x925133, simple at 0x867248).

Steam hook sites verified against the vanilla exe on disk (SHA 6A522E71 confirmed, bytes read through the PE section map):

| Hook | VA | Bytes on disk | Context (Ghidra dump) |
| --- | --- | --- | --- |
| detour `InitializeAdditionalCommands` | 0x0091D52D | `68 00 00 40 00` | `PUSH 0x400000` (hInstance) at 0x91D52D immediately before `CALL 0x407920` (WinMain) in `___tmainCRTStartup` 0x91D424, after `__cinit` (0x91CCB9, called at 0x91D506), so the static `ConsoleFunc` objects (registered by the 0x9840xx initializers) exist. Same shape as GOG 0x925133. |
| simple "console history" | 0x004752D8 | `0F B6 15 28 08 A3 00` | `MOVZX EDX, byte [0xA30828]; TEST EDX,EDX; JZ` in function 0x4752A0 (skips leading spaces/tabs of the command line, then tests the flag). Replacement `BA 01 00 00 00 90 90` = `MOV EDX,1; NOP; NOP` forces the flag on, identical to GOG's edit at 0x867248. |

## 2. Wrapper init simulation

Model (from `GameVersion.cpp` / the wrapper sources, upstream/master): `GetFunctionAddress`, `GetOffset`, `GetClassSize` throw on a
miss; `ResolveFunction`, `GetClassVtable`, `GetGlobalPointer` do not; `GetOffset` behind `HasOffset` is optional. In a
`try`, the first throwing miss abandons the rest of that block (earlier statics stay assigned) and the class flag stays unset
(except `CVirtualMachine`, whose catch falls through). 130 init functions, 1,107 lookups parsed (624 function, 419 offset,
43 `ResolveFunction`, 19 class-size, 2 vtable). Upstream master differs from the 10-02 notes: several wrappers now use
`ResolveFunction` (optional), `CServerExoApp::GetPlayerCreature` is optional with a fallback, and `CClientOptions` is fully optional.

Cell = assigned/lookups; ABORTS = at least one throwing lookup missing (flag unset), first missing name shown; "full" means
nothing throwing was missing. Branch = `addressdb/k2-steam-acc` @4324098; 48 = upstream Steam DB; GOG = upstream GOG DB.

| Class / init (lookups) | Branch Steam | Upstream 48-row Steam | GOG |
| --- | --- | --- | --- |
| ConsoleFunc fns (4) | 4/4 full | 4/4 full | 4/4 full |
| ConsoleFunc offsets (2) | 0/2 ABORTS `name` (unused by the ctor) | same | same |
| CExoString fns (4) | 4/4 full | 4/4 full | 4/4 full |
| CExoString offsets (3) | 2/3 ABORTS `<classes.size>` | same | same |
| CVirtualMachine fns (14) | 14/14 full | 14/14 full | 14/14 full |
| CAppManager offsets (2) | 2/2 full | 2/2 full | 2/2 full |
| CServerExoApp fns (9) | 8/9 full (optional `GetPlayerCreature` absent, K2 has none) | 3/9 ABORTS `GetGlobalVariableTable` (6 missing) | 8/9 full |
| CClientExoApp fns (55) | 22/55 ABORTS `IsValidResolution` (3 missing) | 1/55 ABORTS `AddMoveToModuleMovie` (54 missing) | 22/55 ABORTS `IsValidResolution` (3 missing) |
| CClientExoApp offsets (12) | 1/12 ABORTS `CClientExoAppInternal::debug_mode` (11 missing) | 0/12 ABORTS `CClientExoApp::internal` | 1/12 ABORTS `debug_mode` |
| CClientOptions fns (22, all optional) | 1/22 (only `SetCameraMode`) | 1/22 | 21/22 (`GetMouseSenSetting` missing) |
| CClientOptions offsets (1, optional) | 0/1 (`tooltip_delay`) | 0/1 | 0/1 |
| CGameObject offsets (2) | 2/2 full | 2/2 full | 2/2 full |
| CSWSObject fns (25) | 12/25 ABORTS `SetTag` (1 missing) | 1/25 ABORTS `ClearAllActions` (24 missing) | 12/25 ABORTS `SetTag` |
| CSWSObject offsets (23) | 3/23 ABORTS `tag` (20 missing) | same | same |
| CSWSCreature fns (1) | 1/1 full | 1/1 full | 1/1 full |
| CSWSCreature offsets (37) | 2/37 ABORTS `joining_xp` (35 missing) | same | same |
| CSWSCreatureStats fns (15) / offsets (7) | full / full | full / full | full / full |

Read-across:

- The branch DB reproduces GOG's init outcome (same assigned count, same first miss) for every ACC-reachable class except
  `CClientOptions` fns, where the branch carries only `SetCameraMode` (ACC calls nothing else in that class).
  Over all 100 init functions that have lookups, 70 are equal to GOG, none are better, 30 are worse; the 30 are GUI/Camera/
  C2DA/CExoIni classes and `CClientOptions`/`CExoSound` optional methods, all outside ACC's path (listed in the PARITY section of
  the raw output).
- The 48-row DB cannot initialise `CServerExoApp`, `CClientExoApp` or `CSWSObject` fully, but ACC only needs the leading
  names (`GetPlayerCreatureId`, `GetCreatureByGameObjectID`, `GetClientOptions`, `AddActionToFront` are all among the 48 rows and
  sit before the first abort or are in an unaborted block).
- Every `GetClassSize` lookup fails on all three DBs: the K2 `classes` tables are empty (K1's has 977 rows).

## 3. Command table

Each command, the wrapper path it takes and the DB names it consumes. Status = result of the requirement check at the end of
`simulate_init_output.txt` ("ACC COMMANDS"), then my reading of the code path.

Common to every command: `InitializeAdditionalCommands` registers it via `ConsoleFunc::{NoParam,Int,String}Constructor`
(present in all three DBs). `StringConstructor` (0x475870) is `likely` in the branch DB; evidence found here that makes it
`verified`: 0x475870 registers `giveitem`/`setstrength`/`addexp`... whose handler `FUN_007AB440(int, char*)` calls
`sscanf(param_2,"%d")`, while 0x475910 registers `addlightside`/`adddarkside` with an `int`-argument handler (0x7AB8E0).
So 0x475870 = string holder, 0x475910 = int holder, as the DB has them.

| Command | Type | Wrapper path / DB names | Steam (branch DB) | Steam (48-row DB) | GOG |
| --- | --- | --- | --- | --- | --- |
| `runscript <resref>` | string | `CServerExoApp::GetInstance` (global `APP_MANAGER_PTR`, `CAppManager::Server`) -> `GetPlayerCreatureId`; `CVirtualMachine::GetInstance` (global `VIRTUAL_MACHINE_PTR`) -> `RunScript`; `CExoString(char*)` needs `CExoString` size | **silent no-op**: `CExoString::InitializeOffsets` throws on `GetClassSize("CExoString")` (empty `classes`), `classSize` stays -1, so the string object is never allocated and `RunScript` receives a null `CExoString*`. Steam `RunScript` 0x6FD8D0 and `FUN_007022A0` both start with `if (param_2 != 0)`: returns without crashing | same | **silent no-op** (identical wrapper code; GOG engine null-guard not checked, no GOG exe here) |
| `teleport "x y"` | string | `GetPlayerCreatureId`, `GetCreatureByGameObjectID` (`CServerExoApp`); `CSWSCreature` ctor; `CSWSObject` offsets `Position`,`Orientation`,`AreaId` (first 3 of 23, assigned before the abort); `AddActionToFront` (first of 25, assigned) | **works** (all lookups resolve; 0x540CA0 `ret 112`, action 5 = JumpToLocation) | works | works |
| `freecam` | none | `CAppManager::Client` offset, `CClientExoApp::GetClientOptions` (first of 55), `CClientOptions` `SetCameraMode` (`ResolveFunction`) -> `SetCameraMode(7)` | **works at the call level**: 0x7B5E50 is a plain setter (writes option bytes at +0x75/+0x76 when mode<=1; DB note); whether mode 7 is a free camera on the Steam build is the same engine code as GOG but not tested | works | works |
| `addfeat <feat>` | int | `GetPlayerCreature()` (K2 fallback: `GetPlayerCreatureId` + `GetCreatureByGameObjectID`); `CSWSCreature::CreatureStats` offset (first of 37); `CSWSCreatureStats::AddFeat` (2nd of 15, unaborted) | **works** (0x6B8280: looks up the feat row, appends to the stats feat list at +0xC/+0x10) | **wrong function**: DB has 0x6F5B40, a level-up helper (`CSWLevelStats::AddFeat`-shaped); no feat is added to the creature | works |
| `walkmeshrender` | none | global `RENDER_AABB` 0xA73284 | works if the global is right: tested as `CMP dword [0xA73284],0` in 0x499EB0/0x4A9960 (render code) | same address | works |
| `guirender` | none | global `RENDER_GUI` 0xA32A30 | works: `CMP dword [..],0` in 0x4799C0, 0x47B500, 0x47D2F0, 0x47E390 (8 refs) | same | works |
| `wireframerender` | none | global `RENDER_WIREFRAME` 0xA32A50 | works: loaded in 0x47F320 | same | works |
| `triggersrender` | none | globals `RENDER_QA_TRIGGERS` 0xA2003C and `RENDER_TRIGGERS` 0xA7E0E8 (both toggled) | works: tested in 0x46C5C0 and 0x63DD30 / 0x812650 | same | works |
| `personalspacerender` | none | global `RENDER_PERSONAL_SPACE` 0xA2006C | works: `CMP` in 0x46BD60 | same | works |
| `boundingboxesrender` | none | global `RENDER_GOB_BBS` 0xA73740 | works: tested inside `Gob::Render` 0x4B3FE0 (DB row) | same | works |
| TOML commands from `<game>\commands\*.toml` (other patches, e.g. Snake) | any | no DB lookups; `GetProcAddress` on the owning patch DLL | works (not exercised under Proton) | works | works |

Notes:

- The render toggles read only a global address; they do not depend on any wrapper. The seven `RENDER_*` rows have been in the
  upstream Steam DB since it was created and the branch keeps them unchanged; the usage contexts above are plausible
  (flag compares in render code) but a live toggle test is still outstanding (steam_db_pr3/README "Live test").
- "Partial" on GOG: no ACC command is degraded by the GOG-missing names below; they only truncate wrapper classes (CClientExoApp,
  CSWSObject, CSWSCreature offsets) that other patches use.
- `setstrength`-style built-in cheats are engine commands, not ACC.

## 4. Upstream issues found (candidates; nothing filed)

### 4a. `runscript` regressed on both K2 builds (new, high value)

Upstream commit `840f515` "Class Constructor dispatch" (2026-06-25) changed `CExoString`'s allocating constructors from
`malloc(8)` to `malloc(classSize)` with `classSize = GameVersion::GetClassSize("CExoString")`, and null-guards the constructor
call. The K2 DBs have an empty `classes` table at every revision since (GOG `840f515`, `ebdd9b0`, `fbaf709`, current: 0 rows;
Steam: 0 rows), so `classSize` is -1 and `new CExoString(char*)` yields a null object. K1's DB has `CExoString` = 8 and `ConsoleFunc` = 84.
`ConsoleFunc` already works around this (local struct). Evidence the real size is 8 on Steam: ctor 0x733540 writes two dwords
(`this[0]=0; this[1]=0`), 0x733570 stores pointer + length+1.
Fix options: add `classes` rows `CExoString = 8` (and `ConsoleFunc = 84`) to the K2 DBs, or fall back to 8 in `CExoString.cpp`.
For this PR: the Steam half is one DB row and belongs in the Steam DB PR; the GOG half is an upstream DB fix (GOG DB is
otherwise read-only per the Steam-DB-only rule: this is a confirmed bug, so it needs your call).

### 4b. GOG DB lacks names the wrappers look up (partial init on GOG)

Counts from the simulation over all wrappers: 378 lookups missing in the GOG DB (raw output "GOG DB: lookups issued..."). The ones in
ACC-reachable classes:

| Missing in GOG DB | Where | Effect | Evidence in the GOG DB |
| --- | --- | --- | --- |
| `CClientExoApp::IsValidResolution` | fns #23 (first throwing miss) | aborts #23..#55 (33 names incl. `SetVideoMode`, `GetInGameGui`, `ExitProgram`, ...). `GetClientOptions` is #1, so `freecam` is unaffected | no `CClientExoApp::IsValidResolution`; only `CAuroraInterface::IsValidResolution` 0x907050 |
| `CClientExoApp::GetLoadingSaveGame` | fns #43 | already behind the abort | no row; `CClientExoApp::SetLoadingSaveGame` 0x40E0C0 exists, no getter |
| `CClientExoApp::SetRunScript` | fns #48 | already behind the abort | no function named `*SetRunScript*` in any class |
| `CSWSObject::SetTag` | fns #13 | aborts #13..#25 (GetPortrait ... StopSoundPlayingInDialog); `AddActionToFront` is #1 so `teleport` unaffected | only `CSWSCreatureStats::SetTag` 0x6F2FD0 |
| `CClientExoAppInternal::{debug_mode, selected_texture_pack, input_disabled, mouse_x, mouse_y, game_obj_array, animation_timer, gui_manager, cached_creature, run_script, disable_movies}` (11) | offsets #2..#12 | every internal offset unavailable | offsets table has `CClientExoApp` rows `vtable`,`internal` only; no `CClientExoAppInternal` class |
| `CSWSObject` offsets from `tag` on (20: tag, dlg_*, dialog_owner, ai_level, animation, hit_points, ... reorienting) | offsets #4..#23 | only Position/Orientation/AreaId usable | `CSWSObject` has 4 offset rows: `Position` 148, `Orientation` 160, `AreaId` 144, `game_obj` 0 |
| `CSWSCreature` offsets from `joining_xp` on (35: script_*, entry_point, appearance, ...) | offsets #3..#37 | only CreatureStats/Inventory usable | `CSWSCreature` has `CreatureStats` 4504, `Inventory` 4432, `object` 0, `creature_stats` 4504 |
| every `GetClassSize` (19 lookups; `CExoString`, `CSWGuiPanel`, ...) | classes table | class sizes unavailable | `SELECT COUNT(*) FROM classes` = 0 |
| `ConsoleFunc::name`, `ConsoleFunc::funcholder` | offsets | unused (the ctor uses the local struct) | absent from `offsets` in all three DBs |
| `CClientOptions::GetMouseSenSetting` (optional), offset `tooltip_delay` (optional) | | null-guarded, harmless | absent |

The branch Steam DB mirrors every one of these (it was scoped to GOG parity), so Steam wrappers abort at the same names.
The same list is the extra work if you want the Steam DB to be strictly better than GOG for these classes.

## 5. Draft hooks file

`draft/kotor2-steam-aspyr.hooks.toml` (parses with `tomllib`; validated by `validate-patches.py`):

```toml
[metadata]
target_versions = ["6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"]

# Runs once the engine's static ConsoleFunc objects exist: PUSH hInstance (0x400000) in
# ___tmainCRTStartup, just before the WinMain call and after __cinit
[[hooks]]
address = 0x0091D52D
function = "InitializeAdditionalCommands"
original_bytes = [0x68, 0x00, 0x00, 0x40, 0x00]
type = "detour"
exclude_from_restore = []

# Set Console History to save
[[hooks]]
address = 0x004752D8
type = "simple"
original_bytes = [0x0F, 0xB6, 0x15, 0x28, 0x08, 0xA3, 0x00]
replacement_bytes = [0xBA, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90]
```

It is the `ethansm/local-mods` file (byte-identical content apart from the new comment and the file name). `git diff
upstream/master ethansm/local-mods -- Patches/AdditionalConsoleCommands` shows only that file and a manifest edit
(Steam key added at the end, version still 1.4.0); `ConsoleCommands.cpp`, `exports.def`, `README.md` are unchanged there.
`exports.def` already exports `InitializeAdditionalCommands`; no other symbol is needed for the Steam detour.

## 6. Manifest diff (`draft/manifest.diff`)

```diff
--- a/Patches/AdditionalConsoleCommands/manifest.toml
+++ b/Patches/AdditionalConsoleCommands/manifest.toml
@@ -1,7 +1,7 @@
 [patch]
 id = "additional-console-commands"
 name = "Additional Console Commands"
-version = "1.4.0"
+version = "1.5.0"
 author = "Lane"
 description = "Adds additional commands to the in-game cheat console.\nOther patches can register their own commands with TOML files in a folder titled \"commands\" in the base game directory."
 
@@ -12,4 +12,5 @@
 [patch.supported_versions]
 kotor1_gog_103 = "9C10E0450A6EECA417E036E3CDE7474FED1F0A92AAB018446D156944DEA91435"
 kotor2_gog_aspyr = "777BEE235A9E8BDD9863F6741BC3AC54BB6A113B62B1D2E4D12BBE6DB963A914"
+kotor2_steam_aspyr = "6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF"
 kotor1_cdcrack_103 = "761F9466F456A83909036BAEBB5C43167D722387BE66E54617BA20A8C49E9886"
\ No newline at end of file
```

The key goes before `kotor1_cdcrack_103` so the last line (no trailing newline upstream) is untouched. Version: minor bump
(1.4.0 -> 1.5.0), the #257 precedent and the map-aspect port (1.1.2 -> 1.2.0). The SHA is in the upstream Steam DB's
`game_version`, so `validate-patches.py` accepts it.

## 7. PR body draft

Title: `AdditionalConsoleCommands: Support the Steam Windows build`

```markdown
### Problem
Additional Console Commands has no Steam Windows target. `manifest.toml` lists the GOG K2 build only, so KPM reports a Steam
install (SHA-256 `6A522E71...BBFFEF`) as unsupported, and there is no hooks file for the Steam binary.

### Approach
Both hook sites exist in the Steam binary with the same shape as the GOG ones, and `ConsoleCommands.cpp` has no version- or
build-specific logic (every game address comes from `addresses.db` through `GameVersion`/GameAPI), so the existing
`windows_x86` module serves both builds unchanged. This adds the Steam hooks and the manifest target, nothing else.

- `Patches/AdditionalConsoleCommands/kotor2-steam-aspyr.hooks.toml` (new)
  - detour `InitializeAdditionalCommands` at `0x0091D52D`: the `PUSH 0x400000` before the WinMain call in `___tmainCRTStartup`,
    after `__cinit`, mirroring GOG `0x00925133`
  - "Set Console History to save" at `0x004752D8`: `MOVZX EDX, byte [0xA30828]` replaced by `MOV EDX,1`, mirroring GOG `0x00867248`
- `Patches/AdditionalConsoleCommands/manifest.toml`: add the `kotor2_steam_aspyr` target, bump to 1.5.0

### Dependency
Depends on the Steam address-DB PR (TBD: link when opened), which replaces the 48-row `kotor2_steam_aspyr.db`. With the current DB
`teleport`, `freecam` and the render toggles already resolve, but `addfeat` would call the wrong function: the 48-row DB maps
`CSWSCreatureStats::AddFeat` to `0x006F5B40` (a level-up helper, not the creature-stats method); the DB PR corrects it to `0x006B8280`.

### Known issue, not part of this PR
`runscript` does nothing on any K2 build (GOG included): `CExoString` allocation needs a class size from the `classes` table,
which is empty in both K2 databases since the `GetClassSize` change in 840f515. Tracked separately.

### Testing
- [x] `python3 tools/validate-patches.py` (0 errors)
- [x] Hook `original_bytes` match the vanilla Steam executable (SHA-256 `6A522E71...BBFFEF`) at both addresses
- [ ] Tested on Steam under Proton: TBD
- [x] GOG and K1 hook files left untouched

🤖 Generated with [Claude Code](https://claude.com/claude-code)
```

Before posting: replace the dependency placeholder, rerun `validate-patches.py` on the PR branch, fill in the Proton test, and
decide whether to keep the "Known issue" paragraph (it is accurate per section 4a but it is about GOG too, so it may be
better as its own issue). Never post without explicit approval in chat.

## 8. Open decisions for the main session

1. Add `classes` rows (`CExoString` 8, `ConsoleFunc` 84) to the Steam DB PR? And raise the GOG-side fix upstream (issue or tiny DB PR)?
2. Keep the known-issue paragraph in this PR body or file the issue first and link it?
3. The Steam DB PR should keep the `AddFeat` correction prominent: it is the only functional difference for ACC.
4. In-game checks still owed (never run): `teleport`, `freecam`, `addfeat 1`, the six render toggles on Steam under Proton, ideally
   `runscript` to confirm the no-op (log would show nothing). Build the live DB from the branch head, not 4bb2736.
