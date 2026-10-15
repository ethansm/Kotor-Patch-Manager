# H4: custom Ghidra tooling inventory for swkotor2.exe (REA integration)

Recon, read-only, 2026-10-15. Repo: `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`.
Not run: any command that connects to ghidra.openkotor.com; `ghidra_server_api.py status`; the export shell script. Not read: `~/.config/ghidra/.server_credentials`.

## 1. Key findings

1. Static Steam dump covers 12,355 of 26,961 functions (45.8%). The 14,589 misses are one contiguous hole, `0x009478b0`-`0x009831e0`. `decomp_failed.txt` has 1 entry, so the hole is not failures.
2. `callgraph.tsv` has no callee above `0x0094784c`. No function in the hole has any recorded fan-in in `callers_count.tsv`.
3. The knowledge DB has no xref, caller, callee or pseudo-C tables. Its Steam server data is 25 function rows; the full upstream Steam list sits in TSV files.
4. REA ledger: 180 rows in 11 TSVs. Kinds `xref` (46), `data` (2) and `table` (1) are not in the README schema.
5. `query_ghidra.py` `[rea]` integration works. `scripts/test_query_ghidra.py`: 9/9 pass.

## 2. `scripts/query_ghidra.py` (last commit cc6a424, 2026-10-15; 307 lines)

- DB: `patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db`, opened `mode=ro`. Ledger dir: `<DB dir>/rea_findings` (`--ledger-dir`).
- Subcommands (argparse, L221-306):
  - `symbol <query>`: DB symbol search, then ledger substring match on `name`.
  - `address <hex>`: DB address lookup, then ledger match on the normalized int address.
  - `struct <class> [--cpp]`: `struct_fields` rows, optionally as a C++ declaration.
  - `stats [--json]`: DB table counts plus `rea_findings` = number of well-formed ledger rows.
- Globals: `--db`, `--json`, `--limit` (default 25), `--ledger-dir`.
- `load_ledger(ledger_dir)` (L26-43): reads `*.tsv` sorted, skips malformed lines, tags each row `source_table=rea_finding`, `file=<name>`.
- Text output line: `[rea] <addr> | <kind> | <name> | <confidence> | <topic> <date> (<file>)`.
- Tests: `scripts/test_query_ghidra.py`. 6 in `TestQueryGhidra` (stats, hex normalization, symbol, SQL-injection safety, struct) and 3 in `TestReaLedger` (address, symbol with `[rea]` label, malformed-row count). Run: `python3 -m unittest scripts.test_query_ghidra`. Result: 9 OK.

## 3. `scripts/ghidra_server_api.py` (last commit a055137, 2026-10-04; 258 lines)

Subcommands, from argparse only (not executed):

| subcommand | args | action |
|---|---|---|
| `list-tree` | `[folder=/]` | list remote folders and files |
| `export` | `<program> [--folder TSL] [--out DUMP_DIR]` | headless `ExportProgramSymbols`-style export |
| `export-script` | `<program> <script> --folder TSL --out DIR` (required) | run a read-only `Export*.java` (must start with `Export`) |
| `export-types` | `<program> --folder TSL --out DIR` (required) | data-type export |
| `audit` | none | local vs server comparison |
| `status` | none | credential vault check (not run) |

- Script search path: KPM clone `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/ghidra_scripts` first, then `scripts/ghidra_scripts` (`LOCAL_SCRIPT_PATH`). The KPM path has `ExportProgramSymbols.java`, `ExportFingerprints.java`, `ExportDataTypes.java`, `ExportCommandStores.java`, `ExportAllFunctions.java`.
- Test: `scripts/test_ghidra_server_api.py` (not run).

## 4. `scripts/export_server_knowledge.sh` (last commit 4bbf6dd, 2026-10-03; 71 lines)

- Headless `analyzeHeadless` (`ghidra_12.1.4_PUBLIC`), `-readOnly -noanalysis`, against `ghidra://ghidra.openkotor.com:13100/Odyssey/TSL`.
- Programs: `k2_win_steam_aspyr_swkotor2.exe` and `k2_win_gog_aspyr_swkotor2.exe` with `ExportProgramSymbols.java` into `ghidra_dumps/upstream_server/` (symbols only, no decompile or fp.tsv). Then runs `scripts/audit_server_knowledge.py`.
- Credentials: sources `~/.config/ghidra/.server_credentials` (not read), else `secret-tool`, else a prompt.

## 5. Java scripts

| dir | script | writes (per header comment) |
|---|---|---|
| `scripts/ghidra_scripts/` | `ExportMemoryBlocks.java` (a055137) | raw initialized memory blocks `<block>.bin` plus `blocks.tsv` (name, start, size, perms) for offline objdump/capstone |
| `patch_manager_mods/perf_lab_src/ghidra_scripts/` (ff38b5a, 2026-09-30) | `DumpAllDecomp.java` | `decomp/<hi-byte>/<addr>.c` and `.asm`; `.c` header `// FN addr name size= ret= callers= callees=`; resumable; args `outdir [startIdx endIdx]`; failures to `decomp_failed.txt` |
| same | `ExportCallGraph.java` | `callgraph.tsv` (caller_fn, call_site, callee_fn, type, insn), `callers_count.tsv` |
| same | `ExportGlobalXrefs.java`, `ExportImportXrefs.java` | `globals.tsv`, `imports.tsv` |
| same | `ExportStrings.java`, `ExportAllFunctions.java` | `strings.tsv` (with referencing fns), function TSV |
| same | `RefsTo`, `SymSearch`, `ScalarSearch`, `FindFieldWrites`, `NewNullCheck` | query scripts, not exports |
| `patch_manager_mods/steam_db_batches/server/ghidra_scripts/` (18f0214) | `ExportFingerprints.java` | `fp.tsv` (addr, size, retn, n_insn, callconv, thunk_target, callees, str_refs, disp, imm, name, namespace, source); `strings.tsv` |
| same | `ExportDataTypes.java` | `types.tsv` |
| same | `ExportProgramSymbols.java`, `ExportCommandStores.java` | `*_symbols.tsv`, `cmdstores.tsv` |

Note: `ExportMemoryBlocks.java` is the only script in `scripts/ghidra_scripts/`. `ExportProgramSymbols.java` and `ExportFingerprints.java` are reachable from `ghidra_server_api.py` only through the KPM clone path.

## 6. `patch_manager_mods/ghidra_knowledge/` (last commit b8fec3c, 2026-10-15)

### 6a. `kotor2_ghidra_knowledge.db` (9.8 MB, git-tracked, read via `mode=ro`)

| table | rows | notes |
|---|---|---|
| `functions` | 774 | 705 for Steam `6a522e71`, 61 for `4ab72fc1` (LAA), 8 with a label-only hash |
| `server_functions` | 15,806 | GOG 10,423; Linux 5,358; Steam 25 |
| `server_symbols` | 6,948 | Steam 3,227; GOG 3,542; Linux 179 |
| `server_types` | 6,414 | Steam 886; GOG 4,476; Linux 1,052 |
| `name_map` | 4,627 | `steam_addr`, `name`, `source`, `confidence` |
| `steam_validated` | 3,453 | validated Steam rows |
| `struct_fields` | 418 | 322 with readers or writers |
| `class_methods` | 127 | all have `vtable_index`; from 44 `classes` rows |
| `stack_offsets` | 17 | |
| `strings_of_interest` | 5 | |
| others | | `binaries` 3, `candidates` 66, `ghidra_scripts` 79, `hooks_ported` 16, `lessons` 441 |

Binaries: `6a522e71` = `kotor2_steam_aspyr` (`swkotor2.exe`, project `KOTOR2_SteamAspyr`); `777bee23` = `kotor2_gog_aspyr` (no project). No views.

### 6b. `steam_functions.tsv` (git-tracked)

- Columns: `addr size retn name namespace source thunk`. 26,961 rows (+ header).
- 17 thunks; 15,309 named, 11,652 still `FUN_` placeholders.

### 6c. `rea_findings/` (README.md + 11 TSVs, 180 data rows)

- Schema (README): `addr kind name confidence evidence topic date`.
- Documented kinds: `function`, `global`, `vtable`, `string`, `field`, `callsite`.
- Observed kinds: `function` 64, `xref` 46, `callsite` 33, `global` 14, `vtable` 11, `string` 5, `field` 4, `data` 2, `table` 1.
- Off-schema: `xref`, `data`, `table` (49 rows). Confidence values are only `confirmed`, `probable`, `refuted`.

### 6d. `RE_AGENT_GUIDE.md` (202 lines)

Sections: 1 core rule (scripts compute, models judge); 2 Tier 0 pipeline; 3 traps 3.1-3.10 (stack slots, thunks, decimal offsets, auto labels, `ret N` decoys, dead code, field offsets/names, misread quotes, absence); 4 small-model prompt rules; 5 atlas process rules.

### 6e. `tools/re_card_prep.py` (last commit 4ea8b8f, 2026-10-03; 794 lines)

- Subcommands: `annotate ADDR [--max-lines]` (decimal offsets rewritten to hex), `abi ADDR (--gog-bytes N | --gog-name C::F [--gog-addr A]) [--callconv]`, `anchors ADDR`, `prologue ADDR`.
- `--root` defaults to `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`. Reads only dump files. Test: `tools/tests/test_re_card_prep.py` (not run).

### 6f. `agents/rea-recon.md` (last commit cc6a424, 2026-10-15; 39 lines)

- Haiku agent. Tools: REA MCP only (`rea-agents@6.3.0`, `GHIDRA_INSTALL_DIR` = `ghidra_12.1.4_PUBLIC`).
- Writes its ledger to `rea_findings/<YYYYMMDD>_<slug>.tsv`, ends with `LEDGER: <path> (<N> rows)`. Copy identical to `~/.claude/agents/rea-recon.md`.

## 7. Static dumps and their producers

Root: `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` (dump_start 1790754950, dump_end 1790755502; mtimes 2026-09-30).

| file | rows (data) | producer |
|---|---|---|
| `decomp/<bucket>/<addr>.c` and `.asm` | 12,355 each | `DumpAllDecomp.java` |
| `callgraph.tsv` | 137,651 (CALL 125,296; INDIRECT 12,283; IMPORT 42; TAILJMP 30) | `ExportCallGraph.java` |
| `callers_count.tsv` | 9,359 | `ExportCallGraph.java` |
| `globals.tsv` | 9,678 | `ExportGlobalXrefs.java` |
| `imports.tsv` | 6,558 | `ExportImportXrefs.java` |
| `strings.tsv` | 6,401 | `ExportStrings.java` |

Also: `alloc_sites.tsv` (2,455 rows, producer not traced) and `findings/` (139 analysis files).

Coverage of `decomp/`:
- Buckets 0x40-0x98 (89 non-thunk). 84 buckets fully covered; 0x95-0x97 at zero; 0x94 at 144/1,749; 0x98 at 286/1,104.
- Hole: `0x009478b0`-`0x009831e0`. Every missing non-thunk function (14,589) lies inside it.
- Covered in the window (430) sit at its edges: 144 below, 286 above (tail `0x009831e0`-`0x0098ffff`; first 40 sampled have empty `callers=`).
- Call graph: largest hex callee is `0x0094784c`. Zero edges point into the hole. Callers exist above it (14,852 edge rows with caller >= `0x009478b0`), so callee resolution appears to stop there (inferred, not traced to the script).

Upstream (`upstream_server/k2_win_steam_aspyr_swkotor2.exe/`) data rows: `fp.tsv` 26,965 (4-row delta vs `steam_functions.tsv`, unreconciled); `swkotor2_aspyr.exe_functions.tsv` 26,965; `_symbols.tsv` 114,497; `strings.tsv` 16,996; `types.tsv` 886; `cmdstores.tsv` 1,180. No decompile output; `fp.tsv` has callees and `str_refs`, no callers. The `DumpAllDecomp` run is not documented in the repo (only a recipe in `perf_lab_src/README.md`).

## 8. Local Ghidra install and projects

- Install: `~/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC/support/analyzeHeadless` (exists; not run). Java: `/usr/bin/java`, OpenJDK 21.
- Project: `~/Games/KOTOR2_Mod_Workspace/tools/ghidra_projects/KOTOR2_SteamAspyr.rep/`; `.gpr` is 0 bytes (not opened); `Odyssey.lock` (243 B, 2026-10-02) present.
- Headless callers: `perf_lab_src/gh.sh`, `mods678_research/decompile.sh`. Rule (DESIGN.md L332): one `analyzeHeadless` at a time; check `pgrep -af '^java.*analyzeHeadless'` first.

## 9. Gap table

Source key: **S** = static dumps (local `steam_6a522e71` + `steam_functions.tsv`); **K** = KDB; **G** = Ghidra server (upstream dumps + `server_*` tables); **R** = REA (live MCP tools + ledger).

| fact | S | K | G | R |
|---|---|---|---|---|
| size | have (26,961 fns) | partial (244 rows total) | have (fp.tsv, functions.tsv) | have (`procedure_info`, live) |
| RET | have (`retn`; `.c` header `ret=`) | partial (`server_functions.retn`, Steam 25) | have (fp.tsv `retn`) | have (`procedure_assembly`, live) |
| callers | partial (edges only for callees <= `0x0094784c`; none for hole) | none | none (fp.tsv has callees only) | have (`procedure_callers`, live); ledger `callsite` 33 |
| callees | partial (call graph has callers to `0x985690` but no callee above `0x0094784c`) | none | have (fp.tsv `callees`) | have (`procedure_callees`, live) |
| strings | have (strings.tsv 6,401 with fn refs) | partial (`strings_of_interest` 5) | have (upstream strings.tsv 16,996; fp.tsv `str_refs`) | have (`search_strings`, live); ledger 5 |
| xrefs | partial (globals.tsv, imports.tsv; no data xrefs for hole) | none | partial (fp.tsv `disp`/`imm`/`str_refs` only) | have (`xrefs`, `procedure_references`, live); ledger `xref` 46 (off-schema) |
| pseudo-C | partial (`.c` for 45.8%; none in hole) | none | none (no decompile in upstream dirs) | have (`procedure_pseudo_code`, live, not cached) |
| vtable slot | none | partial (`class_methods` 127, 44 classes) | partial (801 vtable/vftable lines in symbols.tsv, unverified) | partial (ledger `vtable` 11; no slot tool, only `read_bytes`) |
| struct offsets | none (no types data locally) | partial (`struct_fields` 418; `stack_offsets` 17) | partial (`server_types` Steam 886; types.tsv) | partial (ledger `field` 4; otherwise live reads) |
| names | have (15,309 named; 11,652 `FUN_`) | partial (`name_map` 4,627; `steam_validated` 3,453) | have (symbols 114,497; functions.tsv names) | partial (ledger `function` 60 confirmed; `search_procedures`, live) |

## 10. Integration notes

- Schema drift: `query_ghidra.py` reads any kind, but the README lists only six. Either document `xref`, `data`, `table` or normalize them on write.
- Hole: decompile and call graph stop at `0x0094784c`; REA live is the only source for callers, callees, xrefs and pseudo-C in `0x009478b0`-`0x009831e0`. KDB has 25 Steam function rows, so Steam lookups fall back to TSV files.
