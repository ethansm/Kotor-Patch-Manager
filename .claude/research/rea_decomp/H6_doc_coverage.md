# H6 doc coverage: Steam swkotor2.exe (sha256 6a522e71)

Measured 2026-10-15, read-only (sqlite `mode=ro`, no writes to any DB). Base = Steam function count = 26,961 data rows in `patch_manager_mods/ghidra_knowledge/steam_functions.tsv`.

## 1. Where Steam knowledge lives today

| Store | Path | Size | Key | Holds |
|---|---|---|---|---|
| Steam address DB (ships via PR) | `~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db/AddressDatabases/kotor2_steam_aspyr.db` | functions 3,420; offsets 58 (19 classes); global_pointers 14; classes 0 | integer address | name, class, calling conv, param bytes, notes |
| GOG address DB (frozen; label edits need approval) | same folder, `kotor2_gog_aspyr.db` | functions 10,425 (617 classes); offsets 778 (125); global_pointers 21 | integer address | same shape; calling conv set on 1,216 rows |
| Knowledge DB (staging ledger) | `patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db` | 16 data tables (section 2) | mixed (section 4) | names, validation, struct layouts, candidates, upstream server export |
| Ghidra dump (regenerable) | `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` | decomp/ 24,710 files; callgraph.tsv 137,651 edges; strings.tsv 6,401; globals.tsv 9,678; imports.tsv 6,558; alloc_sites.tsv 2,455 | address | raw Ghidra output |
| Full function list | `patch_manager_mods/ghidra_knowledge/steam_functions.tsv` | 26,961 rows | `addr` (`0x%08x`) | Ghidra name, size, source (DEFAULT / ANALYSIS / IMPORTED) |
| REA ledger | `patch_manager_mods/ghidra_knowledge/rea_findings/` | 11 TSV runs + README | addr | agent observations, not validated (README: promotion goes through batch pipeline) |
| Engine AI atlas | `patch_manager_mods/engine_ai_atlas/` | 101 files | subsystem page | prose plus addresses: AI1-AI8 pages, GATE_*, hook_catalogue (101 windows), bug_candidates, 42 class-map TSVs, 11 tools |
| Engine findings | `patch_manager_mods/engine_research_findings/` | 108 files (84 top level) | topic (V1-V7, R, T, U series) | per-topic addresses and arithmetic, not per-function records |
| "Doc 10" | `patch_manager_mods/10_engine_optimization_research.md` | 1 doc | n/a | perf/memory/error-handling program. Not function docs. No per-function "doc 10 engine findings" exists. |
| GOG label suspects | `patch_manager_mods/steam_db_batches/GOG_LABEL_SUSPECTS.md` | 431 lines | GOG addr | GOG names contradicted by body; GOG DB edits only with user approval |

## 2. Knowledge DB: Steam vs GOG per table

| Table | Rows | Steam 6a522e71 | Other |
|---|---|---|---|
| functions | 774 | 705 (purpose on 680) | LAA 4ab72: 61; 8 rows keyed by label `kotor2_steam_aspyr` (bad key) |
| class_methods | 127 | 127 (44 classes, Steam vtables) | none |
| classes | 44 | 44 | none |
| struct_fields | 418 | keyed by class name, no hash (44 classes; high 303, medium 91, low 24) | none |
| stack_offsets | 17 | keyed by hash | none |
| name_map | 4,627 | keyed by Steam addr only | none (GOG not a key here) |
| steam_validated | 3,453 | verified 3,352, likely 95, parity 6 | gog_addr column links to GOG |
| server_functions | 15,806 | Steam program only 25 rows | GOG 10,423 (all named, all with prototype); Linux ELF 5,358 |
| server_symbols / server_types | 6,948 / 6,414 | not split per program here | upstream OpenKotOR export |
| candidates | 66 | bug candidates, subsystem tags | n/a |
| lessons / hooks_ported / strings_of_interest / ghidra_scripts / binaries | 441 / 16 / 5 / 79 / 3 | strings_of_interest has 5 rows only | GOG binary has no local Ghidra project |

## 3. Coverage (base 26,961 Steam functions)

| Measure | Count | % of base |
|---|---|---|
| Steam DB functions, named (shipped) | 3,420 | 12.7 |
| Steam DB rows with calling convention set (3,406 thiscall, 9 cdecl, 5 stdcall) | 3,420 | 12.7 |
| Steam DB rows with verdict `verified` in steam_validated | 3,352 (+95 likely, +6 parity) | 12.4 |
| name_map candidates, distinct steam_addr (high 3,586 rows, medium 926, low 71) | 4,627 | 17.2 |
| name_map addresses NOT in Steam DB (staged, unshipped) | 1,209 | 4.5 |
| Steam DB union name_map addresses (4,623 inside TSV) | 4,629 | 17.2 |
| KDB functions Steam rows (purpose on 680) | 705 | 2.6 |
| TSV non-placeholder names (ANALYSIS 15,295) | 15,308 | 56.8 (not game names, see below) |
| TSV placeholder names (FUN_/DAT_/...) | 11,653 | 43.2 |
| Steam DB addresses whose TSV name is still a placeholder | 3,392 of 3,420 | 99.2 of DB rows |
| GOG DB named functions (GOG total NOT measured locally) | 10,425 | n/a |
| GOG DB rows with calling convention set | 1,216 | 11.7 of GOG named |

Read the TSV "named" figure with care. A 12-name random sample of ANALYSIS names is all library code (`_Decref`, `setbuf`, `xsgetn`, `Catch_All@...`, `FID_conflict:...`). A regex for game-style names (CSW/CExo/C[A-Z]/`::`) matched only 30 of 15,308. Game names live only in the Steam DB and name_map.

Steam DB vs name_map: 3,418 of 3,420 Steam DB addresses also appear in name_map. The 1,209 name_map-only addresses are staged, not shipped.

Ratio only (not one-to-one): Steam DB named 3,420 vs GOG DB named 10,425 = 32.8%. GOG coverage % against a GOG total cannot be computed: the server GOG export has 0 DEFAULT (placeholder) rows, and no GOG function list is stored locally.

## 4. Data hygiene found (affects any join)

1. Address formats differ: integer (Steam/GOG DB), `0x%08x` (name_map, steam_functions.tsv, server_functions), unprefixed `%08x` (KDB functions, class_methods). Normalize to one form before joining.
2. name_map.confidence holds 4 free-text rows (e.g. `mov [ebp-4],-1 (7 B)...`) and 37 rows `wrong` (SCHEMA.md says demoted rows are `refuted`).
3. KDB functions has 8 rows with binary_hash = `kotor2_steam_aspyr` (a label, not a hash).
4. 28 Steam DB addresses have a TSV name that disagrees with the DB (e.g. 0x0073F720: TSV `Statistics`, DB `SendLoadGameRequest`). The DB name wins for shipping; log the disagreement, do not overwrite.
5. KDB has no schema_version table; Steam DB does (4 rows).

## 5. Per-function documentation record: fields and existing homes

| Field | Type | Existing home | Gap |
|---|---|---|---|
| address | int / `0x%08x` | Steam DB (int), KDB functions (no prefix) | normalize key |
| name | text | Steam DB function_name; name_map.name | none |
| class | text | Steam DB class_name; KDB class_methods | none |
| signature (return, params) | text | GOG only: server_functions.prototype (10,423) | none for Steam |
| calling convention, param bytes | text, int | Steam DB (all 3,420); KDB none | mirror into KDB for joins |
| size | int | KDB functions.size; TSV size | none |
| callers / callees | list | callgraph.tsv, read by engine_ai_atlas/tools/cg.py | not persisted |
| strings / globals / imports referenced | list | strings.tsv, globals.tsv, imports.tsv (no function link) | link table missing |
| struct offsets touched | list | struct_fields.readers / writers (instruction addrs) | keyed by class, not function |
| purpose | text | KDB functions.purpose (680 Steam) | coverage 2.5% |
| evidence, source | text | name_map.evidence / .source; steam_validated.evidence_ref | none |
| confidence | enum | name_map.confidence | polluted (section 4) |
| validation verdict | enum | steam_validated.verdict | none |
| shipped? | bool | derived from Steam DB row presence | none |

## 6. Recommended single write target per fact type

| Fact type | Single write target | Who writes | Notes |
|---|---|---|---|
| Shipped name, calling conv, param bytes | Steam address DB `functions` | kpm-pr-steam-db PR only | kotor2-address-db rules; the only shipped path |
| Candidate name + evidence + confidence | KDB `name_map` (steam_addr `0x%08x`) | REA / batch pipeline | staging; promote to Steam DB via batch |
| Validation verdict | KDB `steam_validated` | batch pipeline | 3,453 rows, keep |
| Purpose, size, signature, callconv for Steam | KDB `functions` (best fit: PK binary_hash + address already) | record writer | ADD nullable columns (signature, callconv, param_bytes, confidence, source); do not alter existing columns; fix 8 bad keys first |
| Caller/callee counts | KDB `functions` nullable columns (callers_n, callees_n) | regen script from callgraph.tsv | edges themselves stay in the dump; read with cg.py |
| Function to string/global/import refs | NEW KDB table `function_refs` (binary_hash, fn_addr, kind, target_addr, text) | regen or REA ledger promotion | the only new table needed |
| Struct field layouts | KDB `struct_fields` | persist scripts | existing |
| Stack locals | KDB `stack_offsets` | persist scripts | existing |
| REA observations | `rea_findings/<date>_<topic>.tsv` | rea-recon | ledger; promote via batch |
| Subsystem narrative | `engine_ai_atlas/` pages (AI) or `engine_research_findings/` | authors | prose only; link to addresses, do not duplicate records |
| GOG label errors | `steam_db_batches/GOG_LABEL_SUSPECTS.md` | batch review | never edit GOG DB without approval |

Fit: KDB `functions` is the closest existing store for a unified per-function record (PK binary_hash + address, has purpose, size, notes). name_map stays the name and evidence ledger. The atlas is a prose sink, not a store. Not verified: whether `query_ghidra.py` tolerates added columns (it should, if it selects by column name).

## 7. Limits and unverified

- GOG total function count is not stored anywhere local; GOG coverage % is open.
- decomp/ has 24,710 files for 26,961 functions. A 1:1 mapping was not checked.
- Caller/callee completeness per function was not checked.
- Steam DB functions with a non-placeholder TSV name: 28 disagree (section 4); the other 3,392 are placeholders in the TSV.
- No write was made to any DB, TSV, or doc other than this file.
