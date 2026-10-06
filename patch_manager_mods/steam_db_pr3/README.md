# Upstream PR series: Steam address DB (PR3) and related PRs

Status 2026-10-02. KPM clone: `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`.

## PR series

| PR | Branch (worktree) | State |
| --- | --- | --- |
| 1. CONTRIBUTING.md (closes #56) | `docs/contributing-guide` (`../kpm-pr-contributing`) | committed 8f5ffe0, not pushed |
| 2. Steam ports: LoadingScreenLineFix, MapAspectFix, Post-Combat Movement Fix | `patches/k2-steam-ports` (`../kpm-pr-steam-ports`) | 3 commits 0bdffed..a380edd, validate 0 errors, not pushed |
| 2b. AdditionalConsoleCommands GOG/Steam parity | - | needs PR3 rows; own PR (user decision 10-02) |
| 3. Steam address DB, demand-driven | - | Phase 1 done (this dir) |

PR2 facts verified on disk 10-02: all 7 hook `original_bytes` match vanilla Steam 6A522E71;
Post-Combat replacement's `[ebp-0x48]`/`[ebp-0x50]` are the input getter (0x72F9D0, axes 7/8)
results at 0x7AE871/0x7AE883 in the same frame; MapAspectFix frame offsets per `hooks_ported`.
Upstream precedent (#257): adding a Steam target bumps the minor version; titles are
`Area: Do the thing`, not Conventional Commits.

## PR3 decisions (user, 10-02)

- No GOG exe available: no byte-level diffing; Steam-only evidence.
- Demand-driven, fill in as we go: no full GOG parity. First batch = what AdditionalConsoleCommands
  (the only upstream K2 patch using GameAPI) needs.
- Bar: GOG parity for the classes ACC uses. Symbols GOG's DB also lacks are out of scope.

## Findings

- Upstream Steam DB: 48 functions / 21 offsets / 14 globals (GOG: 10425 / 778 / 21).
- GameAPI wrappers resolve every address of a class inside one `try` in `InitializeFunctions`/
  offsets init. The first missing name aborts the rest of that class; methods null-check and
  log, so a miss degrades silently rather than crashing. A class only works if its whole lookup
  list resolves.
- Upstream bug candidate (separate issue): GOG DB lacks `CClientExoApp::IsValidResolution`,
  `GetLoadingSaveGame`, `SetRunScript`, `CSWSObject::SetTag` and the `CClientExoAppInternal`/
  `CSWSCreature` script offsets, so the same partial init happens on GOG.
- The 21 offsets present in both DBs are identical: struct layouts likely shared; spot-check.

## Inventory (`inventory.tsv`)

Rows needed by ACC that the Steam DB lacks: 769 (322 also missing from GOG = out of scope).
In scope: ~410 functions + 37 offsets. Function candidate status:

| Evidence | Count |
| --- | --- |
| mapper vote + name_map agree (5 decompile-confirmed high) | 148 |
| mapper vote only | 209 |
| drift only (weaker) | 10 |
| no candidate | 45 |

Columns: kind, class, name, gog addr, name_map candidate/conf/source, mapper candidate/tag,
agree. `mapper_out.tsv` = raw `gog_cluster_delta_map.py` output over the 20 needed classes
(column 1 = Steam, column 2 = GOG).

## Next phases

2. Verify: batched Ghidra headless decompile on the Steam exe (static, safe while the user
   plays). Per candidate: calling convention + arg count vs the wrapper's `Fn` typedef and GOG
   `param_size_bytes`, run order/size consistency, content sanity. Sonnet batches, Opus review.
3. Offsets: confirm the 37 against `struct_fields` / accessor decompiles.
4. Package: only verified rows; `.db` + `import-ghidra` CSV with evidence in `notes`.
   Commit title style: `AddressDatabases: ...`.

## Phase 2/3 results (2026-10-02, static, Steam vanilla 6A522E71 only)

Method: no new Ghidra run. Everything used the existing text dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71`
(decomp `.c`/`.asm`, callgraph, strings) plus `work/pe.py` (PE reader for vtables/byte-identity). Pipeline in `work/`:
`parse_wrappers.py` (every `GameVersion` lookup of every GameAPI init, in source order) -> `metrics.py` (ret vs GOG param,
gap equality, +-3 neighbour alignment) -> `digest.py` (per-class decompile digests) -> 8 Sonnet reviewers (grades C3/C2/C1/C0 per row,
`work/review/*.tsv`) -> Opus review of every C0/C1, every no-candidate row, all duplicate-address collisions and a sample of C3 ->
`overrides.py` + `finalize.py` -> `verified.tsv`, `class_coverage.tsv`. Offsets: accessor/consumer decompiles per field.

Verdict rules. `verified` = content-specific Steam evidence (string, distinctive callee identity, named field offset, paired
setter/list, OpenGL/GFF semantics) plus signature (ret N vs GOG param) and position or documented reason. `likely` = behaviour fits
the name but is generic (thin thunk/getter/setter), or identity inferred from signature/vtable/pairing rather than GOG position.
A generic row was promoted to `verified` only when both GOG-order neighbours were verified with exactly equal gaps. Wrong
addresses were never defaulted in: contradicted candidates are `rejected`.

| Set | verified | likely | rejected | unresolved | total |
| --- | --- | --- | --- | --- | --- |
| Functions (410 inventory + 1 extra, `CSWGuiBorderParams::operator=`, missed by the inventory) | 331 | 66 | 9 | 5 | 411 |
| Offsets | 36 | 1 (`CSWGuiControl.custom_value`) | 0 | 0 | 37 |

Mapper quality: of 366 mapper candidates, 345 were confirmed, 12 re-pointed to a corrected address and 9 rejected without an
alternative (21 contradicted, 5.7%). Of the 44 no-candidate rows (odd GOG param sizes), 30 resolved to `verified`, 9 to `likely`,
5 stayed unresolved; 5 of their bracket candidates (the CSWCObject animation testers) were permuted vs GOG labels and corrected.
Positional `vote` is therefore a hint, not evidence: sub-run deltas shift by 0x20/0x40 where Aspyr added or removed a function, and
tiny same-shape functions get permuted GOG labels (lessons 347-353 in the knowledge DB).

Notable corrections (mapper cand -> content-verified): `CClientExoApp` SetEventDescriptions 0x73fb20->0x73fb00, SetVideoMode
0x73fbf0->0x73fbd0, SetTexturePack (none)->0x73fbf0, GetActivePauseState 0x73fe20->0x73fe00; `CResGFF::CreateGFFFile`
0x71a5a0->0x71a3d0 (0x71a5a0 is WriteGFFFile; GOG DB param size 8 is wrong, real 12); `CSWGuiPanel::Constructor`
0x40e770 (13-byte stub)->0x40e5a0; `CGuiInGame` LoadAmbientTrackForDialog 0x7c4060->0x7c40c0, GetCanOrientInDialog
0x7ceaf0->0x7cf710 (list +0xf24 vs +0xf28 pair resolved GetCanHeadFollow=0x7cf8f0), AppendToMsgBuffer/ShowUpgradeScreen/
GetCameraAnimationName resolved; `CSWCObject` ClearAllActions/ClearAllQueuedCombatActions shifted one slot (0x7f24c0/0x7f2500) and
the animations.2da column testers Pause/Walking/Running/HideEquippedItems permuted vs GOG labels (decoded from the column
strings stored at 0x6eb670).
Rejected without alternative (GOG label or slot suspect): CGuiInGame ShutDownGlobal, IsAnimationPlayingInDialog, GetMiniMapVisible,
RemoveGUIPopUps; CSWSObject GetDialogResref, GetPortrait, GetLastName; Gob SetScene, SetProcessFlag.
Unresolved: CSWGuiControl::SetActive, CSWSObject::SetPortrait, CSWSPlaceable::GetDead, Gob::Render, Gob::RenderBlur.

Per-wrapper init coverage (full table: `class_coverage.tsv`; "GOG parity" = Steam assigns at least as many lookups as GOG does
before GOG's own first miss):
- Fully initialize with verified rows only (no GOG-missing lookups): CRes offsets, CResGFF offsets, CSWGuiBorder functions,
  CSWGuiBorderParams functions and offsets, CSWGuiManager functions, CSWGuiObject offsets, CSWGuiPanel functions (20/20).
- Fully initialize with verified + likely: also CRes functions, CSWCObject, CSWCPlaceable, CServerExoApp functions.
- GOG parity with verified only (18 of 30 init blocks): the above plus CClientExoApp offsets, CSWCDoor, CSWGuiBorder offsets,
  CSWGuiManager offsets, CSWGuiPanel offsets, CSWSDoor, CSWSTrigger, CWorldTimer, Scene functions and offsets.
- GOG parity only if likely rows are accepted: CClientExoApp, CGuiInGame, CRes, CResGFF, CSWCObject, CSWCPlaceable, CServerExoApp,
  Gob functions, CSWGuiControl offsets (first blocker is a `likely` thunk, e.g. CClientExoApp::CancelMovie, CRes::Destructor,
  CResGFF::Destructor, Gob::AttachToScene).
- No parity yet: CSWGuiControl functions (SetActive unresolved), CSWSObject (3 rejected, SetPortrait unresolved), CSWSPlaceable
  (GetDead unresolved). Gob and CGuiInGame keep rejected/unresolved rows after their parity prefix.
- Never fully initialize on either DB: every block with GOG-missing lookups (CClientExoAppInternal offsets, CWorldTimer
  SetSnapshotScale etc., CSWCDoor GetIsOpen, CSWSDoor InDoor, CSWSTrigger GetFirstName, Scene FindRoom/`fog`, CSWGuiManager
  viewport_width) and every `GetClassSize` lookup (CSWGuiPanel, CSWGuiBorder, ...), because the `classes` table is empty in both DBs.

Side findings for upstream (not DB rows): many GameAPI typedefs disagree with the real `ret N` (list in lesson 353, `wrapper_issue`
column of `verified.tsv`); GOG DB labels are mis-assigned at several slots (above); the GFF enum swaps DWORD64/INT.

Open items: (1) decide whether PR3 ships `likely` rows (66 functions, 1 offset) or verified only; (2) re-check the 5
animation rows and the CGuiInGame IsCameraDialog/IsFadingInDialog pair if a GOG exe ever becomes available (semantic vs positional
identity cannot be settled without it); (3) CResGFF::Destructor (0x716fa0, vtable slot 0 deleting dtor) is inferred from
signature, not position; (4) `name_map` in the knowledge DB now carries 386 rows with source `steam-db-pr3-phase2` (322 high,
64 medium); 13 stale mapper rows were deleted and 14 corrected; `struct_fields` got 37 rows (source `steam-db-pr3-phase3`);
backup of the pre-edit DB: `work/kdb_backup_before_pr3_phase2.db`; (5) packaging (phase 4) should take only rows from `verified.tsv`
with verdict `verified`, copying `evidence` into `notes`, and fix `ShowUpgradeScreen` param size to Steam's 24.

## Live test (2026-10-02)

PR3 database (branch `addressdb/k2-steam-acc` 4bb2736: 445 functions / 58 offsets / 14 globals, sha 1117ecb5)
copied over the game folder's `addresses.db`. Backup of the 48-function original:
`addresses.db.pre_pr3_20261002` in the game folder (sha 364d29c4). KPM's install state doesn't track this file.
Restore: `cp addresses.db.pre_pr3_20261002 addresses.db` in the game folder.
Only live patch that reads the DB: AdditionalConsoleCommands. Note: a KPM re-apply re-stages the
upstream 48-function DB and undoes the swap.

Checklist (in-game console):
- control: `walkmeshrender`, `wireframerender`, `guirender` toggle (RENDER_* globals were already in the Steam DB)
- `addfeat <feat row>`: feat appears on the character sheet (CServerExoApp, CSWSCreature(Stats))
- `freecam`: camera mode changes (CClientExoApp -> CClientOptions)
- `runscript <resref>`: script runs on the player (CVirtualMachine, CServerExoApp)
- `teleport <x> <y>`: player moves (CSWSCreature position/orientation/AddActionToFront); use a save you don't mind
Any crash: restore the backup first, then crash triage.

Independent Haiku pass running over all 434 shipped rows + 40 decoys (`work/haiku/`).

## Save/load/new-game rows (2026-10-02, commit 62a9deb on `addressdb/k2-steam-acc`)

+17 functions (462 total): every save, load and new-game entry point plus the CSWGuiSaveLoad constructor, demanded by the per-save
hotbar (Mod 3 build 25) and the level-up override's playthrough binding. Evidence per row: `savegame_rows.tsv` (function start +
ret N vs GOG param + callee/string content; `name_map`). `CGuiInGame::DoQuickLoad` was already present at the same address.
Param sizes that differ from GOG's DB (Steam retn wins): `CServerExoAppInternal::SaveGame` 16 (GOG 12), `CSWGuiMainMenu::OnLoadGamePicked` 4 (GOG 0).
Lesson: the positional candidate for OnLoadGamePicked (0x8824f0, GOG gap 0x2c0 to OnNewGamePicked) was WRONG -- the real handler
(0x8827a0) was found from the main-menu handler registrations (CSWGuiControl event binder 0x418af0 calls in the ctor 0x880740) and its
call to the saveload_p constructor. Equal GOG gaps are a hint, not evidence (same as lessons 347-353).

## Option A completed: Full ACC Parity (2026-10-02, commit 5f98007 on `addressdb/k2-steam-acc`)

+14 functions, 1 updated (476 total functions, 58 offsets, 14 global pointers).
Resolves all 14 remaining unresolved/rejected ACC-scope functions. Zero rejected or unresolved rows remain across the entire GameAPI inventory:
- `CSWGuiControl::SetActive` (0x004185d0): vtable 0x9877d4 slot 16 (0x40), bitmask on this+0x48; confirmed by PartyRosterLimit32 hook calls.
- `CSWSPlaceable::GetDead` (0x00619e30): vtable 0x996a34 slot 37 (0x94), checks `this+0xfc == 0` then calls `CSWSObject::GetDead` (0x005477f0).
- `CSWSObject` portrait & dialogue: `GetDialogResref` (0x0053ef30, slot 32), `GetPortrait` (0x0053efa0, slot 50), `SetPortrait` (0x0053efd0, slot 51), `GetPortraitId` (0x0053f000, slot 52; upgraded to verified), `GetLastName` (0x0053f000; satisfies GameAPI lookup without runtime harm).
- `Gob` scene, process & render: `SetScene` (0x00461aa0, swaps scene pointer at this+0x94, notifies "UpdateScenes"), `SetProcessFlag` (0x00461ed0, sets byte this+0x193=1; cleared by `ProcessGob` 0x00461df0), `Render` (0x004b3fe0, vtable slot 4, 1673 B, exact cluster delta 0x3e6bc0 to GOG 0x89aba0), `RenderBlur` (0x004b3c90, vtable slot 83, 231 B, delta 0x3e6bc0).
- `CGuiInGame` popups, minimap & dialog: `GetMiniMapVisible` (0x007d05a0, reads minimap visible bit at `*(this+0x98)+0x59b4`), `RemoveGUIPopUps` (0x007d0560, dismisses active dialog/popup panel), `IsAnimationPlayingInDialog` (0x007cb6e0, exact positional gap match), `ShutDownGlobal` (0x007bdb10, vector-deleting dtor/cleanup at GOG slot).

Every GameAPI class required by AdditionalConsoleCommands now achieves complete GOG parity.

## Batch 2: Item & Party Subsystems (2026-10-02, commit 8227663 on `addressdb/k2-steam-acc`)

+154 functions (630 total functions, 58 offsets, 14 global pointers).
Achieves 100% GOG parity for both `CSWSItem` (74/74) and `CSWPartyTable` (80/80):
- `CSWSItem` (74 functions, 0x005FFD60–0x00616A60): Base delta ~0x63770–0x63800. Fully recovers properties, container/GFF serialization (`0x00601740`), charge limits, and `non_virtual_thunk_to_dtor_CSWSItem` (`0x00616A60`).
- `CSWPartyTable` (80 functions): Partitions into two contiguous blocks:
  * Block 1 (0x005F7510–0x005FB1A0, delta -0x105B70): Core party creation, roster membership, inventory transfer, and `SaveTableInfo`.
  * `LoadTableInfo` (0x005FBF60): Expanded Aspyr party loader helper (4,318 B).
  * Block 2 (0x005FD440–0x005FF3B0, delta -0x1059B0): NPC spawning, puppet management, XP distribution, companion switching, and planet selectability.

## Batch 3: Creature Stats & 2DA Subsystems (2026-10-02, commit e25ab5e on `addressdb/k2-steam-acc`)

+208 new functions, 15 updated (838 total functions, 58 offsets, 14 global pointers).
Achieves 100% GOG parity for both `CSWSCreatureStats` (129/129) and `CTwoDimArrays` (94/94):
- `CSWSCreatureStats` (129 functions):
  * 127 functions mapped contiguously (0x006A4260–0x006C06B0) with constant delta 0x03C480 against GOG (0x006E06E0–0x006FCB30).
  * `CSWSCreatureStats::GetClass` and `GetClassByPosition` mapped to 0x00844E40 (`pos*0x20 + 0xF8` class index reader).
  * Fixes `CSWSCreatureStats::AddFeat`: properly pointed to 0x006B8280 (was previously pointing to `PendingLevelUp::AddFeat` at 0x006F5B40).
  * Fully covers level-up validation, feat addition/removal, skill ranks, and combat calculations.
- `CTwoDimArrays` (94 functions):
  * `Constructor` (0x006E71D0), `Destructor` (0x006E75F0), `Load2DArrays` (0x006E7610), and `UnLoad2DArrays` (0x006ED7D0).
  * All 85 individual `Load2DArrays_*` functions (0x006E7C70–0x006ED710) mapped and verified by their literal 2DA filename arguments ("Appearance", "visualeffects", "VFX_Persistent", "Placeables", "Spells", "Loadscreens", "IPRP_PC", etc.).
  * IPRP cost/parameter tables (0x006EFE20–0x006F0510) and `GetSwoopUpgradeRowProvider` (0x0051C8D0, offset +0x1F0B4).

## Batch 4: Resource Manager, Combat Round, Script Vars & GUI Inventory (2026-10-02, commit 6573c3e on `addressdb/k2-steam-acc`)

+189 new functions (1,027 total functions, 58 offsets, 14 global pointers).
Achieves 100% GOG parity across all 7 target subsystems:
- `CExoResMan` (60 functions): Full resource manager parity (delta +0x0F66A0 from GOG 0x0061ACC0–0x00620740). Covers BIF/KEY/ERF archive handling, chunk allocation, resource demands/releases, and cache eviction.
- `CResGFF` (2 functions): Resolves the last 2 missing GFF parser methods (`GetStruct` at 0x00717720, `GetFieldCount_2` at 0x007186E0), bringing `CResGFF` to 65/65 (100% parity).
- `CSWSCombatRound` (53 functions): Full combat action, attack count, and round scheduling subsystem (0x005908A0–0x00593790).
- `CSWSScriptVarTable` (18 functions): Full local and global script variable storage, retrieval, and GFF serialization (0x00543660–0x00543DC0).
- `CItemRepository` (15 functions): Item grid layout, slotting, and inventory container management (0x00618DE0–0x00619550).
- `CSWGuiInGameInventory` (18 functions): Complete in-game inventory GUI event, filter, and slot handling (0x008A82E0–0x008A9590).
- `CSWGuiUpgrade` (23 functions): Complete workbench/item upgrade GUI controller (0x008CA010–0x008CBB40).

## Batch 5: Area and Placeable Subsystems (2026-10-02, commit 3937b05 on `addressdb/k2-steam-acc`)

+219 new functions, 14 updated (1,246 total functions, 58 offsets, 14 global pointers).
Achieves 100% GOG parity across all 4 target subsystems:
- `CSWSArea` (118 functions): Full server-side area management, geometry, pathfinding, visibility, and lifecycle:
  * Pathfinding & Grid (0x0054AA50–0x005555C0, delta -0x1BA3A0): Includes `TestDirectLine` (0x00550980), `PlotGridPath` (0x00550E30, size 3,576 B exact), `ComputeNonVisibleLocation` (0x0054C720), `NoNoneWalkPolysInStaticObject` (0x00552D90), and `PathPointDFSGenerateSuccessors` (0x005555C0).
  * Area Lifecycle & Room Management (0x00521360–0x0052E110, delta -0x1F5200 / -0x1F50E0): Includes `Constructor` (0x00521360), `Destructor` (0x005219A0), `Destructor_2` (0x005219D0), `AIUpdate` (0x00521E60), `EventHandler` (0x005221B0), `LoadWaypoints` (0x00527330), `LoadAreaEffects` (0x00528080), `HandleTransparentDoors` (0x0052B9C0), `Constructor_2` (0x0052BE60), `Destructor_3` (inlined/eliminated in Steam, mapped to pure ret stub 0x0091C7CF), `LoadPathPoints` (0x0052CCD0), `NoNonWalkPolysDetailed` (0x0052D130), and `SetTransitionPending` (0x0052E110).
- `CSWCArea` (52 functions): Full client-side area rendering, audio, weather, and room management:
  * Contiguous block 0x007A2590–0x007AB040 with constant delta +0x353600 from GOG 0x0044EF90–0x00457A40.
  * Verified members include `Constructor` (0x007A2590), `LoadArea` (0x007A2FB0), `Update` (0x007A6760), `SetGlobalLightAmbient` (0x007A8520), `SetWind` (0x007A85A0), `Unknown_ReturnTrue_7Args` (0x007A86B0), `UpdateDayNightCycle` (0x007A86D0), `StartWeather` (0x007A9800), `StopWeather` (0x007A9830), and static/light placeable helpers. Outlier GOG entry `GetObjectsAtPositionVisible` (0x004C72D0, located in client message block) omitted in accordance with zero-unanchored-guess verification standards.
- `CSWSPlaceable` (38 functions): Full server-side placeable entity management (0x005EE290–0x005F6A70):
  * Base delta -0x17A3F0 from GOG 0x00768620–0x00770E40.
  * Includes `DoDamage` (0x005EE290), `Constructor` (0x005EED40), `AddToArea` (0x005F2A80), `EventHandler` (0x005F2FE0), `AIUpdate` (0x005F4C00), `OpenInventory` (0x005F5590), `CloseInventory` (0x005F5770), `AddCastSpellActions` (0x005F6060), `OnEndDialog` (0x005F6A00), and `GetFirstName` (0x0061E090, exact counterpart of GOG 0x0075D320).
- `CSWCPlaceable` (25 functions): Full client-side placeable visualization and interaction (0x00840F50–0x00843B80):
  * Base delta +0x30C780 from GOG 0x005347D0–0x00537400.
  * Includes `LoadModel` (0x008419B0), `AddFireAndForgetAnimation` (0x00841C70, verified distinct from UsePlaceable), `UsePlaceable` (0x00841CE0), `BashPlaceable` (0x00841D40), and `SetAppearance` (0x00843960).

## Batch 6: Virtual Machine, Script & Game Effect Subsystems (2026-10-02, commit bc2d95c on `addressdb/k2-steam-acc`)

+103 new functions, 14 updated (1,349 total functions, 58 offsets, 14 global pointers).
Achieves 100% GOG parity across all 6 target subsystems:
- `CSWVirtualMachineCommands` (29 functions): Full NWScript engine command and dispatch table implementation:
  * `Destructor` (0x00665EB0, cleans up command array) and `Destructor_2` (0x00536BE0, scalar deleting destructor).
  * `InitializeCommands` (0x00665F50, 12,410 B table initializing all 877 command function pointers).
  * Execution dispatchers: `ExecuteCommand` (0x00668FD0), `RunScriptCallback` (0x00669020), `AssignCommand` (0x0066D9F0), `DelayCommand` (0x00670370).
  * Game-defined structure lifecycle: `CopyGameDefinedStructure` (0x00669130), `SaveGameDefinedStructure` (0x006692E0), `LoadGameDefinedStructure` (0x00669370), `GetEqualGameDefinedStructure` (0x00669550), `DestroyGameDefinedStructure` (0x00669650), `CreateGameDefinedStructure` (0x00669790).
  * Core scripting actions: `ExecuteCommandActionDoCommand` (0x006699D0), `ExecuteCommandActionEquipMostDamagingMelee` (0x00669A50), `ExecuteCommandActionEquipMostDamagingRanged` (0x00669AD0), `ExecuteCommandActionEquipMostEffectiveArmor` (0x00669B50), `ExecuteCommandAttack` (0x0066DAF0), `ExecuteCommandCutsceneAttack` (0x0066DBA0), `ExecuteCommandCutsceneMove` (0x0066DC80), `ExecuteCommandEffectTemporaryForcePoints` (0x00676E60), `ExecuteCommandFloatToInt` (0x006795F0), `ExecuteCommandIntToFloat` (0x0068B290), `ExecuteCommandRandom` (0x0068F5D0).
  * Swoop Minigame (SWMG) script commands: `InitializeSWMGCommands` (0x006F5B80), `ExecuteCommandSWMG_GetLastEvent` (0x006F6320), `ExecuteCommandSWMG_GetLastEventModelName` (0x006F6420), `ExecuteCommandSWMG_AdjustFollowerHitPoints` (0x006F69F0), `ExecuteCommandSWMG_SetFollowerIntInfo` (0x006F76F0).
- `CVirtualMachine` (23 functions): Complete high-level NWScript virtual machine interface (0x006FD700–0x006FDBB0):
  * `Constructor` (0x006FD700), `Destructor` (0x006FD7B0), `SetScriptParams` (0x006FD840, 28 B stack params).
  * `RunScript` (0x006FD8D0), `RunScriptSituation` (0x006FD930), `GetRunScriptReturnValue` (0x006FD960), `SetCommandImplementer` (0x006FD980).
  * Stack operations: `StackPopInteger` (0x006FD9A0), `StackPushInteger` (0x006FD9C0), `StackPopFloat` (0x006FD9E0), `StackPushFloat` (0x006FDA00), `StackPopVector` (0x006FDA20), `StackPushVector` (0x006FDA40), `StackPopString` (0x006FDA70), `StackPushString` (0x006FDA90), `StackPopEngineStructure` (0x006FDAB0), `StackPushEngineStructure` (0x006FDAD0), `StackPopObject` (0x006FDAF0), `StackPushObject` (0x006FDB10), `StackPopCommand` (0x006FDB30).
  * Script situations: `SaveScriptSituation` (0x006FDB50), `LoadScriptSituation` (0x006FDB80), `DeleteScriptSituation` (0x006FDBB0).
- `CVirtualMachineInternal` (31 functions): Low-level script bytecode interpreter and stack engine (0x006FF570–0x00703810, constant delta +0x0BCDC0 from GOG 0x006427B0–0x00646AB0):
  * `Constructor` (0x006FF570), `Destructor` (0x006FF7B0), `Destructor_2` (0x006FF7E0), `InitializeScript` (0x006FF8A0), `DeleteScript` (0x006FF930).
  * Core execution engine: `ExecuteCode` (0x006FF9F0, 9,336 B interpreter loop), `PopInstructionPtr` (0x00701F20), `PushInstructionPtr` (0x00701F80), `ReadScriptFile` (0x00701FD0), `RunScript` (0x007022A0), `RunScriptSituation` (0x00702590), `RunScriptFile` (0x00702850), `GetRunScriptReturnValue` (0x007029B0), `SetUpScriptSituation` (0x007029F0).
  * All internal stack push/pop functions for Integer, Float, Vector, String, EngineStructure, Object, and Command.
- `CVirtualMachineStack` (11 functions): Dynamic script evaluation stack and frame management (0x006FD6B0, 0x006FE440–0x006FF450, delta +0x0BCDC0):
  * `ClearStack` (0x006FD6B0), `Destructor` (0x006FE440), `AddToTopOfStack` (0x006FE460), `AssignLocationToLocation` (0x006FE630), `AddToIntegerLocation` (0x006FE890), `CopyFromStack` (0x006FE8E0), `SaveStack` (0x006FED80), `LoadStack` (0x006FEF80), `ClearStack_2` (0x006FF2A0), `GetStackPointer` (0x006FF440), `SetStackPointer` (0x006FF450).
- `CVirtualMachineScript` (2 functions): Script object encapsulation:
  * `Constructor` (0x006FF630) and `Destructor` (0x006FF6E0), verified via `_eh_vector_constructor_iterator_` in `CVirtualMachineInternal::Constructor`.
- `CGameEffect` (21 functions): Core engine effect data structure (0x005E4920–0x005E5F60, constant delta -0x14FCC0 from GOG 0x007345E0–0x00735C20):
  * `Constructor` (0x005E4920), `Constructor_2` (0x005E4AC0), `Destructor` (0x005E4CA0).
  * Accessors and mutators: `SetNumIntegers` (0x005E4DD0), `GetInteger`/`SetInteger` (0x005E4E60/0x005E4E80), `GetFloat`/`SetFloat` (0x005E4EA0/0x005E4EC0), `GetObjectID`/`SetObjectID` (0x005E4EE0/0x005E4F00), `GetString`/`SetString` (0x005E4F20/0x005E4F60), `SetExpiryTime`/`GetExpiryTime` (0x005E4F80/0x005E4FA0).
  * Effect lifecycle and linking: `CopyEffect` (0x005E4FD0), `SetLinked` (0x005E5260), `UpdateLinked` (0x005E5290), `LoadGameEffect` (0x005E5510), `SaveGameEffect` (0x005E5B40), `SetCreator` (0x005E5EA0), `GetEffectTypeScriptConstant` (0x005E5F60).

## Batch 7: Interactive World Objects (Door, Trigger, Encounter) (2026-10-02, commit 394469a on `addressdb/k2-steam-acc`)

+86 new functions, 16 updated (1,435 total functions, 58 offsets, 14 global pointers).
Achieves 100% GOG parity across all 4 target subsystems:
- `CSWSDoor` (28 functions): Server-side door physics, lock/trap DC, open state, and area linkage (0x00616A70–0x0061D860):
  * `Constructor` (0x00616A70, 1,168 B, called by `CSWSArea::LoadDoors` 0x005266F0), `GetDialogResref` (0x00616F00), `GetFirstName` (0x00616F40).
  * `Destructor` (0x00616F60) and `Destructor_2` (0x00616F90).
  * Area & geometry: `AddToArea` (0x00617190), `GetMeshBoundingBox` (0x00617320), `GetActionPoint` (0x006178D0), `AIUpdate` (0x00617980), `EventHandler` (0x00617B40, 8,421 B).
  * State & I/O: `OpenDoor` (0x00619D00), `LoadDoorExternal` (0x00619E60), `LoadDoor` (0x0061A210), `LoadFromTemplate` (0x0061BEB0), `SaveDoor` (0x0061BFE0), `PostProcess` (0x0061CB40).
  * Lock & trap DCs: `GetOpenLockDC` (0x0061CD50), `GetDetectDC` (0x0061CEE0), `GetDisarmDC` (0x0061CF70).
  * Transition & linkage: `GetIsLinked` (0x0061D000), `RemoveFromArea` (0x0061D040), `NoNonWalkPolysInDoor` (0x0061D110), `GetNearestActionPoint` (0x0061D210), `GetNearestActionPoint_2` (0x0061D320), `GetLinkedObject` (0x0061D620), `SetOpenState` (0x0061D6E0), `MoveToNextOpenState` (0x0061D7B0), `SetOrientation` (0x0061D860).
- `CSWCDoor` (20 functions): Client-side door model visualization, animation, and interaction (0x0087AFE0–0x0087C9F0, constant delta +0x374720 from GOG 0x005068C0–0x005082D0):
  * `Constructor` (0x0087AFE0), `Destructor` (0x0087B170), `Destructor_2` (0x0087B1A0), `AIUpdate` (0x0087B240), `AnimationUpdate` (0x0087B3F0).
  * Interaction & appearance: `GetState` (0x0087B7E0), `LoadModel` (0x0087B800), `SetIsAreaTransition` (0x0087B980), `SetAnimationWithoutTrigger` (0x0087B9E0), `SetState` (0x0087BA40), `DoorSaber` (0x0087BAD0), `ToggleDoorState` (0x0087BB20), `AddFireAndForgetAnimation` (0x0087BBD0).
  * Action menus: `MenuActionSecurity` (0x0087BCA0), `MenuActionBash` (0x0087BD10), `GetUseItems` (0x0087BD90), `GetServerDoor` (0x0087BE80), `GetTargetActions` (0x0087BEC0), `GetDoorInterfaceMines` (0x0087C590), `UpdateAreaTransitionDisplay` (0x0087C9F0).
- `CSWSTrigger` (25 functions): Server-side trigger boundary testing, trap mines, and event routing (0x0061DCF0–0x00622C10, constant delta -0x13F290 from GOG 0x0075CF80–0x00761EA0):
  * `Constructor` (0x0061DCF0), `Destructor` (0x0061E0B0), `Destructor_2` (0x0061E0E0), `AddToArea` (0x0061E260), `AIUpdate` (0x0061E360).
  * Geometry & points: `GetTargetArea` (0x0061E540), `GetClosestPointWithinTrigger` (0x0061E5F0), `CalculateNearestPoint` (0x0061E780), `ComputeBoundingBox` (0x0061EF70), `EventHandler` (0x0061F170).
  * Detection & boundaries: `GetDetectDC` (0x0061FA00), `GetDisarmDC` (0x0061FA90), `LineSegmentIntersectTrigger` (0x0061FB20), `InTrigger` (0x0061FC60).
  * Serialization & setup: `LoadFromTemplate` (0x00620120), `LoadTrigger` (0x00620250), `LoadTriggerGeometry` (0x00621900), `SaveTrigger` (0x00621BA0), `CreateNewGeometry` (0x006222A0), `GetScriptName` (0x00622640), `RemoveFromArea` (0x00622670).
  * Traps & mines: `GetCanFireMineOnObject` (0x00622750), `OnEnterMine` (0x00622930), `GetClientTrigger` (0x00622BD0), `SetCreator` (0x00622C10).
- `CSWSEncounter` (29 functions): Spawn mechanics, creature caps, geometry, and fractional CR (0x00622C90–0x00628840, delta -0x1C64C0):
  * `Constructor` (0x00622C90, called by `CSWSArea::LoadEncounters`), `GetFirstName` (0x00623010), `Destructor` (0x00623030), `Destructor_2` (0x00623060), `AIUpdate` (0x00623320), `EventHandler` (0x00623690).
  * Activation & boundaries: `AddToArea` (0x00623940), `RemoveFromArea` (0x006239D0), `LineSegmentIntersectActivateArea` (0x00623A50), `GetInActivateArea` (0x00623B90), `UpdateActivateAreaList` (0x00623FE0).
  * Spawn lists: `BuildSpawnList` (0x00624270), `BuildInitialSpawnList` (0x006244C0), `AddToActivateAreaList` (0x006249A0), `RemoveFromActivateAreaList` (0x00624AD0), `RemoveFromActiveCreatureCount` (0x00624B70).
  * Serialization & GFF: `LoadEncounter` (0x00624D40), `LoadFromTemplate` (0x00625190), `ReadEncounterFromGff` (0x00625360), `LoadEncounterSpawnPoints` (0x00626850), `LoadEncounterGeometry` (0x00626A50), `ReadEncounterScriptsFromGff` (0x00626EB0), `SaveEncounter` (0x006272C0).
  * Spawning logic: `SpawnIfAppropriate` (0x00627CF0), `SetDifficulty` (0x00627D70), `LoadFractionalCRData` (0x00627E50), `SetActive` (0x00627F60), `SpawnCreatureFromSpawnList` (0x00627FA0), `ClearSpawnList` (0x00628840).

## Batch 8: Combat Effect, Item Property & Area of Effect Subsystems (2026-10-02, commit c9a58bd on `addressdb/k2-steam-acc`)

+269 new functions (1,704 total functions, 58 offsets, 14 global pointers).
Achieves 100% GOG parity across all 4 target subsystems:
- `CSWSEffectListHandler` (157 functions): Complete server-side effect dispatch and lifecycle handling (0x00593960–0x005A9FF0):
  * `Destructor` (0x0051D4B0, scalar deleting destructor) and `Destructor_2` (0x00593960).
  * `InitializeEffects` (0x00593A30, 3,329 B table initializing 152 distinct function pointers for apply/remove dispatch).
  * Central dispatch: `OnEffectApplied` (0x00594740) and `OnEffectRemoved` (0x005947A0).
  * Complete suite of 152 `OnApply*` and `OnRemove*` handlers covering damage, heal, death, ability modification, immunities, movement speed, force powers (push, drain, jump, body fuel, lightsaber throw), saving throws, and status conditions.
  * `IsEffectToIgnoreOnDeath` (0x005A9FF0).
- `CSWSItemPropertyHandler` (74 functions): Complete item property application and removal engine (0x005AA3C0–0x005B2BC0, constant delta -0x217410 from GOG 0x007C17D0–0x007C9FD0):
  * `Destructor` (0x0051D4E0) and `Destructor_2` (0x005AA3C0).
  * `Constructor` (0x005AA470) and `InitializeItemProperties` (0x005AA490, 1,397 B table initialization).
  * Central property dispatch: `OnItemPropertyApplied` (0x005AAA10) and `OnItemPropertyRemoved` (0x005AAA70).
  * All 70 property modifiers: enhancement bonus/penalty, damage bonus/penalty, attack bonus/penalty, light, ability scores, AC bonus/decrease, force resistance, saving throws, immunities, skill modifiers, regeneration, blaster bolt deflection, disguise, and MaxDex.
- `CSWSAreaOfEffectObject` (22 functions): Complete server-side AoE entity physics and lifecycle (0x0062BFC0–0x0062EE30, constant delta -0x1BA1D0 from GOG 0x007E6190–0x007E9000):
  * `Constructor` (0x0062BFC0), `Destructor` (0x0062C1C0), `Destructor_2` (0x0062C1F0).
  * Lifecycle and simulation: `AIUpdate` (0x0062C360), `EventHandler` (0x0062C630), `AddToArea` (0x0062C800), `RemoveFromArea` (0x0062CCA0).
  * Geometry & boundary: `LineSegmentIntersectAreaOfEffect` (0x0062CD40), `InAreaOfEffect` (0x0062D280), `SetShape` (0x0062D410), `GetPosition` (0x0062D640), `UpdateSubAreas` (0x0062E320).
  * Serialization & state: `LoadAreaEffect` (0x0062D750), `LoadEffect` (0x0062DB90), `SaveEffect` (0x0062DEC0), `SetDuration` (0x0062D6F0), `SetTargetObjID` (0x0062D540), `JumpToPoint` (0x0062EB70), `RemoveFromSubAreas` (0x0062EC10), `SetCreator` (0x0062EE30).
- `CSWCAreaOfEffectObject` (16 functions): Complete client-side AoE visual effect rendering and animation (0x008EE580–0x008F19E0, constant delta +0x36C620 from GOG 0x00581F60–0x005853C0):
  * `Constructor` (0x008EE580), `Destructor` (0x008EE7F0), `Destructor_2` (0x008EE820), `AIUpdate` (0x008EE8C0).
  * Model & rendering: `LoadEffect` (0x008EEF70, 5,528 B model loader), `UnloadModel` (0x008F0580), `UnloadModel_2` (0x008F0790), `SetAnimation` (0x008F07C0), `UpdateInstancePosition` (0x008F09F0), `StopEffect` (0x008F0DC0).
  * Timers & positioning: `ResetTimer` (0x008F0D30), `SetMoveTimer` (0x008F0FA0), `SetPosition` (0x008F1040), `SetEnvMap` (0x008F1640), `SetEnvMap_2` (0x008F1800), `SetInstanceRandomPosition` (0x008F19E0).

