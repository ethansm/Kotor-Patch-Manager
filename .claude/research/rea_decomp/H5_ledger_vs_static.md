# H5: REA B13a ledger vs static Steam dump

Scope: 9 `*B13a*.tsv` ledger files (145 rows, 99 unique addresses) in `patch_manager_mods/ghidra_knowledge/rea_findings/`.
Static: `patch_manager_mods/ghidra_knowledge/steam_functions.tsv`, plus the dump `~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/` (`decomp/<bb>/<addr>.asm|.c`, `callgraph.tsv`, `callers_count.tsv`, `strings.tsv`, the PE image).
Read-only, $0, no REA, no Ghidra. Helper: `/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/42a6a25e-5315-402d-a861-d94a14444190/scratchpad/h5/h5_check.py` (outputs in `.../scratchpad/h5/out/`).

## Coverage
- Unique addresses: 99 = 46 function, 46 xref, 30 callsite, 14 global, 4 string, 2 data, 2 vtable, 1 table.
- Code addresses with an asm dump in their containing function: 76/76 (100%). Function 46/46, xref 46/46, callsite 30/30.
- Data addresses (global, string, data, vtable, table): 23, no dump by design (not code).
- Overall: 76/99 = 76.8% have a dump.

## Agreement by fact type
| Q | Fact | Agree / checked | % | Static source |
|---|---|---|---|---|
| 1 size/RET | size | 62/63 | 98.4 | steam_functions.tsv `size` |
| 1 size/RET | RET bytes | 79/79 | 100 | steam_functions.tsv `retn` |
| 6 start | fn start / body start | 112/112 | 100 | steam_functions.tsv `addr` |
| 2 callers | direct callers (CALL+TAILJMP) | 54/54 | 100 | callgraph.tsv |
| 2 callers | fan-in count | 39/39 | 100 | callers_count.tsv `fan_in_fns` |
| 2 callers | callsite edge | 20/20 | 100 | callgraph.tsv |
| 3 callees | callee list (subset) | 46/46 | 100 | callgraph.tsv |
| 3 callees | callgraph vs asm `CALL` targets | 92/92 | 100 | asm dump |
| 3 callees | callgraph vs `.c` header callers=/callees= | 92/92 each | 100 | `.c` line 1 |
| 4 strings | string text vs strings.tsv | 3/3 (1 empty-string row absent) | 100 | strings.tsv |
| 4 strings | string text vs exe bytes | 4/4 | 100 | PE image |
| 4 strings | "no strings" claims | 14/14 | 100 | strings.tsv `functions` |
| 4 strings | PUSH-imm quote vs static PUSH scan | 4/4 | 100 | rg over asm |
| 5 vtable/xref | vtable slot holds pointer | 12/12 | 100 | exe dword |
| 5 vtable/xref | vtable `read_bytes` | 2/2 | 100 | exe bytes |
| 5 vtable/xref | global reader functions | 6/6 | 100 | rg over asm |
| 5 vtable/xref | data-user claims | 13/15 | 86.7 | exe + asm |
| - | ledger `0xADDR: TEXT` quotes vs asm | 179/179 (5 not in dump) | 100 | asm dump |

Total: 955 of 958 checks agree. Quote normalisation: case, whitespace, `dword/byte/word/float ptr`, leading zeros in hex, `+-`.

## Disagreements (3 checks, 2 unique claims)
1. Data-user claim `0x0077c780` cites `DATA refs ... 0x77c021` (B13a-q2, listed twice). Static: `0x77c021` is `MOV dword ptr [EAX],ECX` inside `0x77bcf0`, with no reference to `0x77c780`. Not reproduced.
2. Size `0x0076e610`: ledger 484 B (body `0x76e610-0x76e7f3`, 18 body ranges, B13a-q4) vs steam size 429. Likely span-vs-contiguous definition, not a data error.
3. Quote (not scored) `0x007682a5: RET 0x8` cited for `0x007680a0` (B13a-q1). Steam size 516 ends at `0x7682a3`; the only in-body `JMP` (at `0x7680b7`) targets `0x7682a2`. Unresolved, see OPEN.

Asm quotes outside the contiguous Steam size (5): `0x7682a5`, `0x775a72` (3-range body), `0x76e7f1` x2, `0x76e7e9` (18-range body).

## Extra static findings
- Static caller sources: `callgraph.tsv` (per call site), `callers_count.tsv` (fan-in), and the `.c` header `callers=`. All three agree 92/92 per function.
- `fp.tsv` (upstream_server Steam) has a `callees` column but no in-degree column.
- Callgraph edge types: CALL 125,296; TAILJMP 30; IMPORT 42; INDIRECT 12,283. INDIRECT edges have callee `-`, so they carry no static target.
- A static PUSH scan finds a second user of `Fx_Shadow_S/M/L`: `0x842590`. The ledger cites only `0x774260`.
- Empty string `0x00986925` has 490 PUSH users, so it carries no signal.

## Which fixed REA questions are redundant
| Question | Verdict | Basis |
|---|---|---|
| size/RET | redundant | steam_functions 62/63 size, 79/79 RET; residual is the body-range definition |
| callers | redundant | callgraph 54/54, fan-in 39/39 |
| callees | redundant | callgraph 46/46, asm 92/92 |
| strings | redundant for "uses string X" | strings.tsv and PUSH scan 4/4; static scan finds more users than the ledger |
| vtable/xref users | mostly redundant | data refs and vtable slots 13/15 (1 unique claim not reproduced); indirect dispatch targets have no static callee |
| function start | redundant for listed starts | 112/112; a start missing from steam_functions.tsv is invisible statically |

Only REA can answer: what a function does (semantics), the concrete targets of `CALL [reg+slot]` vtable dispatch, non-contiguous body ranges and exact RET boundaries, and any function start absent from steam_functions.tsv.

## Caveats
- Agreement means the REA-quoted fact matches the Ghidra dump, which is one source. It shows consistency, not independent truth.
- Containing-function lookup uses the steam_functions size, so it misses non-contiguous bodies (the 5 quotes above).
- Budget: 27 tool calls against a cap of 25. The 2 extra were Edit retries.

## OPEN
- Does ledger quote `0x007682a5: RET 0x8` belong to `0x007680a0`? Check `decomp/76/007680a0.asm` for the RET address.
