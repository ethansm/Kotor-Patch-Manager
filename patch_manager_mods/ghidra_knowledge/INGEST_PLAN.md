# INGEST plan: backfill names from mod code + research notes into the REA ledger / KDB

Resume: `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`.
Handoff: `.claude/handoffs/INGEST_PLAN.handoff.md`. Follow-up to `DECOMP_PLAN.md` (D0-D8 DONE 10-27).

## Why
Gap scan 2026-10-27 (scratch `gap.py`, $0): Steam-exe functions cited elsewhere but absent from `name_map`
(non-`wrong`) and every `rea_findings/*.tsv`:

| source | files | functions cited | missing |
|---|---|---|---|
| mod code (`patch_manager_mods/*_src/**/*.{toml,h,cpp,inc}`) | 578 | 380 | 79 |
| research notes (`.claude/research`, `patch_manager_mods/*research*`, `engine_research_findings`, numbered docs, `.claude/reviews`) | 836 | 1660 | 515 |

Examples: `0x0050dac0` = `CAurTriangleBin::RenderGrassPolys` (grass hooks.toml:44-48, live hook 4) and `0x007457e0`
= `CSWGuiMainInterface_PortraitIconRowDraw` (buff HUD, 33 cites in 4 docs): both unknown to `query_ghidra.py`.
D6 R2 NO-GO root cause was missing names / caller context, so coverage is the lever.

## Phases

| # | Phase | Status | Record |
|---|---|---|---|
| I0 | `tools/ingest_names.py gap|extract` + tests: mod-code candidates TSV (addr, name, kind, src path:line, fn_start, status new/agrees/conflicts) and the gap metric | TODO | spec §I0 |
| I1 | `ingest_names.py ledger`: static gate + write `probable` ledger rows for `new` mod-code candidates (topic `ingest-modcode`); conflicts report only; lint stays at baseline | TODO | spec §I1 |
| I2 | Haiku verify with decoys on I1 rows (cards from `static_facts.py`); verdict TSV; promotion list | TODO | spec §I2 |
| I3 | Research-notes extraction (same tool, `--source notes`), top-cited first, Haiku verify, ledger rows topic `ingest-notes`; stop when yield < 30% | TODO | spec §I3 |
| I4 | Promotion (user OK) of verified names into `name_map`; regen class pages for touched classes; gap re-measure | TODO | spec §I4 |
| I5 | Docs sweep (tool listed free-first in CLAUDE.md, RE_AGENT_GUIDE §2.1, README, SUBAGENT_ROLES) + memory | TODO | Decisions |

## Cost log

| phase | start (ISO) | agents | est. cost | note |
|---|---|---|---|---|

## Spec

### I0: extractor + gap metric ($0 at run time)
- New `patch_manager_mods/ghidra_knowledge/tools/ingest_names.py` (stdlib), tests
  `tools/tests/test_ingest_names.py` + fixtures `tools/tests/fixtures/ingest/`.
- `gap [--source modcode|notes|all] [--json]`: reproduces the table above (exe addresses `0x00[4-9]xxxxx` below
  `0x980000`, mapped to the containing function via `steam_functions.tsv` starts; known = `name_map` confidence !=
  `wrong` + all ledger addrs).
- `extract --source modcode --out FILE`: name/address pairs from
  1. `*.hooks.toml`: `address = 0x...` plus the nearest preceding comment naming `Class::Method` (within 6 lines);
  2. `constexpr <type> <Name> = 0x00XXXXXX;` in `.h/.cpp/.inc` (e.g. `_shared_src/GameAddr.h:15`
     `constexpr uintptr_t PlayGuiSound = 0x004122A0; // thiscall(mgr, char idx)`), name = `<Name>`, trailing comment kept;
  3. comment lines matching `(\w+::\w+)\W{1,6}(?:at|@|=)?\s*0x00[4-9][0-9A-Fa-f]{5}` or the reverse order.
- Output columns: `addr kind name src fn_start offset status kdb_name` where kind = function (addr == fn_start),
  callsite (inside a function), global (>= 0x980000); status = new | agrees | conflicts (vs `name_map`).
- Names that are not Class::Method (`PlayGuiSound`) stay as-is, flagged `bare` in a `form` column; never invent a class.

### I1: ledger rows
- `ingest_names.py ledger --in FILE --topic ingest-modcode [--apply]` (dry run default): only `status=new`,
  `kind=function|global`. Row: `kind` function/global, `confidence probable`,
  `evidence` = `<addr>: <first asm insn from the dump .asm> | src <path:line>` (passes L2), date today.
  One file `rea_findings/<YYYYMMDD>_ingest-modcode.tsv` via `rea_ledger.append_rows` (README convention 2).
- Several names for one addr: keep the most frequent; the rest go to `conflicts.tsv`. `conflicts` (vs KDB) are never written.
- Gate: `rea_ledger.py lint` issue count unchanged; `query_ghidra.py address 0x0050dac0` shows `[rea]` RenderGrassPolys.

### I2: Haiku verify (model-verify rule: every promoted row needs Haiku + decoys)
- Cards: `static_facts.py addr` output + the candidate name; 1 decoy per 4 (a real name from another card).
  Verdict per card: consistent | inconsistent | unsure + one quoted line. Chunks of <= 12 cards per Haiku, cap 10 tool calls.
- Pass: decoys rejected >= 80%; rows consistent and not decoys -> `promote.tsv`.

### I3: research notes
- `extract --source notes`: same pattern 3 plus markdown tables with an address column and a name column.
  Rank by cite count; process top 100 first; Haiku verify as I2; ledger topic `ingest-notes`.
- Stop rule: if a 100-row chunk yields < 30% consistent, stop and report.

### I4: promotion
- `promote.tsv` -> `name_map` only after user OK (lead write, source `ingest-<topic>`, confidence `medium`, integrity check).
- `fn_doc.py page <Class>` for each touched class; `ingest_names.py gap` again (report the new missing counts).

## Decisions
- Ledger first, KDB later: ledger rows are agent observations (`[rea]`), promotion needs Haiku+decoys and user OK.
- Address-DB (Steam DB worktree) is out of scope.
- New tool advertised in all agent docs in the same phase it lands (I5 closes the sweep).
