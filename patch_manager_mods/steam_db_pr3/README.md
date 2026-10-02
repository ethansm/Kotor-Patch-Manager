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
