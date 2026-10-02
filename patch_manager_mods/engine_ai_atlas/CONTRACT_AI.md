# AI atlas addendum to the analysis-batch contract (2026-10-02)

This file extends `patch_manager_mods/engine_research_findings/CONTRACT.md` (also at `ROOT/CONTRACT.md`). Read that first.
Everything in it still applies: read-only, text-only over the dump, no Ghidra, no DB writes, no repo edits outside your
output, never touch the game folder. `ROOT = ~/Games/KOTOR2_Mod_Workspace/ghidra_dumps/steam_6a522e71/`.
`REPO = /home/ethansm/orca/workspaces/local-setup/KOTOR-II`.

## Output
- Exactly one file `ROOT/findings/AI<n>.md` (wave 1: AI1-AI4, wave 2: AI5-AI8), at most ~600 lines.
- Mapper TSVs, if you run the mapper, go to `ROOT/findings/AI<n>_map_<class>.tsv` (always pass `--out`).
- Start the file with a 5-line summary (most mod-relevant findings first).

## Sections (all required; write "none" if empty)
- (a) **Function table**: `| addr | proposed name | confidence (high/med/low) | one-line purpose | evidence (quoted line + address) |`.
  Mark which rows you CONFIRMED against the decompile (ret N, callees, strings) vs took from a mapper TSV.
- (b) **Findings**: answer every question in your brief, one subsection each, verdict word first.
- (c) **Runtime probes**: for each open item that only a run can settle: counter, hook address, `original_bytes`, expected signal.
- (d) **Open questions**.
- (e) **Struct field table** per struct you touched:
  `| class | offset | size | type | proposed name | readers (addr) | writers (addr) | confidence | evidence |`.
  Offsets in hex (`0x524`). A field is `high` only with a quoted asm line for at least one reader or writer.
- (f) **Hook-site catalogue** for every place a mod could hook:
  `| id | address | length | stolen bytes (hex from .asm) | convention / live regs | consumed exit? | EAX/stack notes | hot/cold | nearest sibling self-patch site distance |`.
  Rules: instruction boundary; >= 5 bytes; no relative CALL/JMP/Jcc in the stolen range; no branch target inside it
  (grep the function's .asm for jumps INTO the range); stolen bytes of a consumed-exit hook run with EAX = the handler
  result, so they must not read EAX. Hot = runs per object per tick.
- (g) **Mod relevance**: one line per finding naming the pending item it serves, from this list:
  FM1-FM7, FM2b, E3, E4, E7, E9, L1/L2/L3, M5, M8, P1, P2, A1, a1_retarget_strict, C1, T1, R1, AIQ, PFLK, EFFOVF,
  enemy-E7 (`hold_enemies`), enemy-AI, medpac, party-flag (`+0x11AC`), Mod 8 (kite), perf. Use "none" if none.

## Verdict words
CONFIRMED / REFUTED / SPECULATIVE. "Verified" means a quoted asm or decompile line with its address. A grep absence is
never proof (lesson 168): check indirect writes via `this+off`, asm `[reg + off]`, and vtable calls
(`CALL dword ptr [reg + 0xNN]`; decide the class from where the pointer comes from and quote it).

## Tools (use these BEFORE reading .c files)
Tools live in `REPO/patch_manager_mods/engine_ai_atlas/tools/` (stdlib python3; run with `python3`):
- `cg.py callers|callees|path|subtree|indirect ...` over `ROOT/callgraph.tsv` with names from the knowledge DB (read-only).
  `cg.py --help` lists the options.
- `field_census.py --offset 0x524 (--fns A,B,... | --subtree ADDR --depth N)` lists every `[reg+off]` read and write in
  the `.asm`, with a base-register provenance guess.
- GOG -> Steam name mapper: see CONTRACT.md (`gog_cluster_delta_map.py`). Rows in `ROOT/findings/AI0_map_*.tsv` and
  `ROOT/findings/AI0_mapper_summary.md` already cover the AI classes: read them instead of re-running.

## Do not re-derive (read, cite, build on)
- Knowledge DB lessons 212-292 (`ROOT/inputs/lessons_recent.tsv`, re-exported 2026-10-02): combat-ai, party-ai-scripts,
  ranged-ai scopes.
- `REPO/patch_manager_mods/mods678_research/ai_review_2026-10-01/review_ai_engine.md` sections "Map", "Stance field",
  "Checked, no bug", "Coverage" (creature and combat-round field lists live there).
- `REPO/patch_manager_mods/mods678_research/ai_research_2026-10-02/R1_engine_followups.md` (FM2 verified, A1 vs +0x524,
  the retarget metric mismatch, FM3/FM5/FM7/T1/R1/P2) and `R3_enemy_ai.md`.
- `ROOT/findings/U3.md` (AIQ, PFLK, EFFOVF) and `U1.md` (VM).
If you find something that contradicts one of these, say so explicitly with both quotes: that is a high-value result.

## Return codes (settled; do not re-open)
Action handlers: 1 in progress, 2 and 3 terminal (RunActions treats them the same), 4 special. The pathfinder's
"2 success / 3 fail" (U3) is its own convention.

## Sibling self-patching sites (the 12-byte rule)
Other installed mods patch these addresses at runtime. Any candidate window must start or end more than 12 bytes away
from each: `0x40D69E`; `0x8ABA8C 0x8ADA08 0x8ADA81 0x8ADFA2 0x8AE02D 0x8AE19A 0x8AE1CD 0x8AE793`. Installed hooks
are in `REPO/patch_manager_mods/mods678_research/live_hooks_2026-09-30g.tsv` plus the shipped companion-ai / ranged-ai
tomls (`REPO/patch_manager_mods/companion_ai_src/*.hooks.toml`, `ranged_ai_src/*.hooks.toml`): say when a candidate
overlaps or touches one. The lead re-checks every window later with `check_windows.py` (which also reads the live
config); you never open the game folder.

## Budget
Use `cg.py` and `field_census.py` first, grep second, read `.c`/`.asm` third. Quote a few precise lines, never whole
functions. Do not run Ghidra; list missing functions in (d).
