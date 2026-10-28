# Session Handoff & Plan State: patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md

## 0. Next-Phase Direction
Execute I7b: Batch B17 (gap twins) through `steam_db_batches/tools/batch.py` candidates -> gates -> cards -> Haiku review -> finalize -> Opus gate

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`
- **Branch:** `KOTOR-II` (4b23629e INGEST I7a: ingest_names.py twins + batch.py candidates --gog-list)
- **Generated:** 2026-10-28 01:04:46 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
4b23629e INGEST I7a: ingest_names.py twins + batch.py candidates --gog-list
 .claude/research/ingest/i7/twins.tsv               | 343 +++++++++++++++++++++
 CLAUDE.md                                          |   2 +-
 patch_manager_mods/ghidra_knowledge/README.md      |   4 +-
 .../ghidra_knowledge/RE_AGENT_GUIDE.md             |   1 +
 .../ghidra_knowledge/tools/ingest_names.py         |  71 +++++
 .../tools/tests/test_ingest_names.py               |  99 ++++++
 patch_manager_mods/steam_db_batches/DESIGN.md      |   1 +
 patch_manager_mods/steam_db_batches/tools/batch.py |  26 ++
 .../tools/tests/test_batch_ledger_cards.py         |  25 ++
 9 files changed, 570 insertions(+), 2 deletions(-)
```
- **Dirty Files (189):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/51_combat_stats_page.handoff.md`
  - ` M .claude/handoffs/52_quick_loot.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ... +179 more (run `git status --short`)

## 2. Immediate Objective (I7b)
**Batch B17 (gap twins) through `steam_db_batches/tools/batch.py` candidates -> gates -> cards -> Haiku review -> finalize -> Opus gate**

Status: TODO
Task: Batch B17 (gap twins) through `steam_db_batches/tools/batch.py` candidates -> gates -> cards -> Haiku review -> finalize -> Opus gate

## 2b. Upcoming Batches in Pipeline
- **I7c** : Persist B17 to `name_map` only (user OK), class pages, gap re-measure, docs sweep, memory

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate I7b` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- Gap prototype (I0 starting point): `.claude/research/ingest/gap_prototype.py` (4368b1e4); `python3 .claude/research/ingest/gap_prototype.py` prints 79 / 515.
- `name_map` marks bad names with confidence `wrong` (38 rows incl. 0x006a8fe0, demoted 10-27); there is no `superseded` value. `static_facts.py` uses only `high` names.
- `rea_ledger.py lint` baseline is 84 issues (10-27), not 53; it grows when other sessions add ledger files, so compare before/after your own run.
- rea-recon v2: peak ctx 66k on 2 addrs after the Grep cap (115k on 4); it still Globs for the exe even when given the path. REA write tools are denied in `.claude/settings.json` (87948820).
- Haiku briefs: token caps are ignored (112-200k peaks); cap tool calls and name line ranges.
- Verify: `cd patch_manager_mods/ghidra_knowledge/tools && python3 -m unittest discover -s tests` (314 OK after I0/I1, 10-27); `python3 scripts/query_ghidra.py address 0x0050dac0` (shows `[rea]` RenderGrassPolys).
- I0/I1 landed 335c5487 (+3bf1bf2e plan rows): 69 rows in `rea_findings/20261027_ingest-modcode.tsv` (24 function, 45 global). `ledger --apply` refuses if the target file exists: never re-run it for the same date/topic.
- `agent_contract.py check` is unreliable on this tree: baseline snapshot caps at 2000 dirty paths, so new untracked files read NOT_DIRTY and other sessions' commits read OUT_OF_SCOPE. Gate with `git status --porcelain -- <scope>` instead.
- Extract quality: 45 globals are mostly bare mod constants (`AddrPathN`, `kConstVA`, IAT slots, vtables) and `0x0098bfe0 TexMgr::Tick` is a global (likely a vtable slot); weakest candidates for I2. The 116 KDB `conflicts` include comment prose captured as names (e.g. "Party entry -> creature ..."); never ledger them.
- Name conflicts kept out: `.claude/research/ingest/conflicts.tsv` (0x0046a980 DoFog/SetFogRange, 0x0085d430, 0x0098c45c).
- CLAUDE.md is dirty with another session's hunks: commit your hunk via `git hash-object -w` of HEAD+hunk, `git update-index --cacheinfo`, then mirror the edit to the working file.
- 10-28 (I2-I5 lessons): Haiku defaults to `unsure` on thin cards; static-facts-only function cards gave 38-57% decoy rejection, `cards --decomp-lines 10` gave 81%. Getter decoys (one-field return) still pass: treat getter names as weak. Promote strict = consistent in every pass.
- 10-28: name/addr regex used to span ", " in lists (mis-paired the next address); fixed, plus `.claude/research/ingest/` excluded from notes. After any extractor change, re-extract and drop ledger rows that lose source support.
- 10-28 next batch sizing (dry runs, nothing written): `extract --source notes --top 100` now yields only 25 new rows (24 ledger-able functions); `extract --source modcode` yields 2 (globals 0x009960c8 EffMasteryLight, 0x009960cc EffMasteryDark, 4 bytes apart so they cannot decoy each other). Gap 460 notes / 53 modcode is mostly addresses without any name in text: new names need another source.
- `promote_names.py --stage I<n>` labels evidence; DB copies before writes went to session scratch (`kdb_before_I4*.db`).
- Do not chain `test | tail && apply`: tail masks the test exit code (I4b applied with 1 red test; it was a test-edit bug).
- 10-28 I6 (74b64d63): 12 names, gate 11/13; text extraction exhausted (gap 452/52). Only Steam-DB classes get `fn_doc.py page` output ("no functions for class X" is normal). `steam_docs/_work/` is another session's, never commit it.
- 10-28 I7 sizing (scratch script, not committed as a tool): `.claude/research/ingest/i7/sizing_gap_gog_twins_20261028.tsv` (342 rows `steam gog name`). Full gap list = `ingest_names.gap(top_n=100000)[g]["top"]`. Key that worked: `(size, retn, n_insn, disp, imm, #callees, #str_refs)` from fp.tsv, unique in BOTH exes. Server Steam names are mostly FID noise (ANALYSIS 15295); never use them as names.
- I7 verify: `cd patch_manager_mods/steam_db_batches/tools && python3 -m unittest discover -s tests` (scope it alone, check exit); batch dir `patch_manager_mods/steam_db_batches/B17/`.
- 10-28 I7a DONE (4b23629e): `ingest_names.py twins --out .claude/research/ingest/i7/twins.tsv` -> gap=458 named=342 unnamed=20 ambiguous=58 no_twin=38 no_fp=0 (twins.tsv committed, force-added). `batch.py candidates --gog-list FILE` landed. Suites: ghidra tools 346 OK, steam_db_batches tools 331 OK. 2 Sonnet workers ~122k tokens total; `agent_usage.py --since` read 0 (workers logged under another session id). Next = I7b (B17 candidates with `--gog-list` + `--extra-candidates` on twins.tsv).
