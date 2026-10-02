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
