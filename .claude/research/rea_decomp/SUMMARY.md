# REA + custom Ghidra tooling: research summary (2026-10-15, Opus-verified)

7 Haiku recon agents (H1-H7, $0.83 total; all 7 exceeded 100k peak context -> next waves need narrower scopes).
Lead corrections are marked **[Opus]**; they override the H-file text.

## Verified facts
1. **Static Ghidra dumps cover 100% of real code. [Opus, refutes H4's "45.8% coverage hole"]**
   `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/decomp/` has 12,355 `.asm`/`.c` pairs. Of the
   14,606 `steam_functions.tsv` rows without a dump, 14,589 are `Unwind@` SEH funclets (avg 11 B, all in
   0x9478b0-0x9831e0), 4 are `Catch_All@`, 13 are import stubs or 2-byte thunks. No real game function is missing.
2. **Static data already answers the 6 fixed REA questions** (H5): 955/958 checks agree on 145 B13a ledger rows
   (size, RET, callers, fan-in, callees, strings, start). Sources: `steam_functions.tsv`, `callgraph.tsv`,
   `callers_count.tsv`, `strings.tsv`, `globals.tsv`, asm dumps. Paying Haiku+REA for these is waste.
3. **What only REA (live Ghidra) adds**: targets of the 12,283 INDIRECT call edges (`callgraph.tsv` callee `-`; vtable
   dispatch), non-contiguous function bodies (0x7680a0 = 3 ranges, RET at 0x7682a5 beyond the contiguous size),
   live data xrefs, and named pseudo-C via in-session annotation.
4. **REA CLI cost (H1, live)**: script-drivable (`function <exe> <addr> --provider ghidra --snapshot f --format json`,
   `GHIDRA_INSTALL_DIR=~/Games/KOTOR2_Mod_Workspace/tools/ghidra/ghidra_12.1.4_PUBLIC`). But every new address or
   subcommand = full re-import: ~5 min, 3.8 GB RSS. `--snapshot` is an exact-request result cache (repeat 2.2 s).
   CLI-per-address is unusable for bulk work.
5. **REA MCP internals (H3, static)**: one JVM per MCP process; `open_binary` imports + auto-analyses once
   (`-readOnly -deleteProject`, temp copy of the exe, nothing written next to it); then a long-lived bridge serves
   requests serially (`GhidraRequestQueue`). A Python stdio MCP client (JSON-RPC: initialize ->
   notifications/initialized -> tools/call) can hold ONE session for many calls = one cold start per session.
   Pilot runs confirm: 30-85 calls in 8.5-14 min incl. ~5.5 min cold start.
6. **batch_decompile** (MCP only): `{addresses: [...]}` -> `{items:[{address,status,pseudocode|error}],...}`;
   per-item errors do not fail the batch; response capped by `REA_MCP_MAX_RESPONSE_BYTES`.
7. **annotate is ephemeral (H2)**: names/comments go to the session DB, discarded on close; only usable within one
   session (annotate known names, then decompile in the same session). Read-only sessions drop annotate.
8. **Coverage (H6)**: Steam DB 3,420 named / 26,961 functions (12.7%; ~28% of the 12,355 real ones); name_map stages
   1,209 more; KDB `functions.purpose` on 680 Steam rows. No signature store for Steam; no function->refs table.
9. **Edge cases (H7)**: no exe sha256 gate (DESIGN says "md5 6a522e71", it is a sha256 prefix); DESIGN allows 12
   parallel REA vs measured cap 2-3; process guard `pgrep analyzeHeadless` misses REA's GhidraClassLoader JVM;
   ledger has 49 off-enum kinds, 31 rows without asm quote, conflicting confirmed+refuted rows with no supersede
   flag; `load_ledger` silently truncates >7-cell rows; rea-recon writes its ledger only at the end (crash = loss);
   grep-first leaks prior (refuted) names into blind runs; 4 twin callees 0x85d260/2d0/320/3d0 have no rows.

## Open (need a probe, not more reading)
- Per-call latency and RSS inside one persistent MCP session (estimate 3-10 s from pilot transcripts).
- Does a non-read-only session (`annotate_native_function` / `set_addresses_names`) change later `batch_decompile`
  output? Cap on batch size / response bytes?
- `resolve_native_call_targets` quality on `CALL [reg+slot]` sites.
- Our local Ghidra project `KOTOR2_SteamAspyr.rep` (persistent analysis) + a headless rename-and-dump script vs a
  REA session for bulk named decompile: which is cheaper per 1,000 functions?

## Files
H1_cli_live_probe.md, H2_cli_catalog.md, H3_mcp_internals.md, H4_custom_tooling.md (section 1.1 refuted above),
H5_ledger_vs_static.md, H6_doc_coverage.md, H7_edge_cases.md.
