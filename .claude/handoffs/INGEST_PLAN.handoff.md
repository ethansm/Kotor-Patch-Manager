# Session Handoff & Plan State: patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md

## 0. Next-Phase Direction
Execute I5: Docs sweep (tool listed free-first in CLAUDE.md, RE_AGENT_GUIDE §2.1, README, SUBAGENT_ROLES) + memory

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`
- **Branch:** `KOTOR-II` (021c750f INGEST I3: richer cards PASS (81%), 56 ingest-notes ledger rows; fix comma-list name pairing)
- **Generated:** 2026-10-27 23:23:05 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
021c750f INGEST I3: richer cards PASS (81%), 56 ingest-notes ledger rows; fix comma-list name pairing
 .claude/research/ingest/i3/README.md               |  12 +
 .claude/research/ingest/i3/cards_v2/chunk_1.md     | 209 +++++
 .claude/research/ingest/i3/cards_v2/chunk_2.md     | 254 ++++++
 .claude/research/ingest/i3/cards_v2/chunk_3.md     | 241 ++++++
 .claude/research/ingest/i3/cards_v2/chunk_4.md     | 254 ++++++
 .claude/research/ingest/i3/cards_v2/chunk_5.md     | 254 ++++++
 .claude/research/ingest/i3/cards_v2/chunk_6.md     | 222 ++++++
 .claude/research/ingest/i3/cards_v2/chunk_7.md     | 244 ++++++
 .claude/research/ingest/i3/cards_v2/chunk_8.md     | 229 ++++++
 .claude/research/ingest/i3/cards_v2/chunk_9.md     | 191 +++++
 .claude/research/ingest/i3/cards_v2/key.tsv        | 107 +++
 .claude/research/ingest/i3/cards_v2/promote.tsv    |  62 ++
 .claude/research/ingest/i3/cards_v2/score.txt      |   6 +
 .claude/research/ingest/i3/cards_v2/verdicts_1.tsv |  13 +
 .claude/research/ingest/i3/cards_v2/verdicts_2.tsv |  13 +
 .claude/research/ingest/i3/cards_v2/verdicts_3.tsv |  13 +
 .claude/research/ingest/i3/cards_v2/verdicts_4.tsv |  13 +
 .claude/research/ingest/i3/cards_v2/verdicts_5.tsv |  13 +
 .claude/research/ingest/i3/cards_v2/verdicts_6.tsv |  13 +
 .claude/research/ingest/i3/cards_v2/verdicts_7.tsv |  13 +
 .claude/research/ingest/i3/cards_v2/verdicts_8.tsv |  13 +
 .claude/research/ingest/i3/cards_v2/verdicts_9.tsv |  11 +
 .claude/research/ingest/i3/conflicts.tsv           |   1 +
 .../research/ingest/i3/extract_notes_all_v2.tsv    | 888 +++++++++++++++++++++
 .../research/ingest/i3/extract_notes_verified.tsv  |  69 ++
 patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md |   1 +
 .../rea_findings/20261027_ingest-notes.tsv         |  57 ++
 .../ghidra_knowledge/tools/ingest_names.py         |  74 +-
 .../tools/tests/test_ingest_names.py               |  56 ++
 29 files changed, 3538 insertions(+), 8 deletions(-)
```
- **Dirty Files (188):**
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
  - ... +178 more (run `git status --short`)

## 2. Immediate Objective (I5)
**Docs sweep (tool listed free-first in CLAUDE.md, RE_AGENT_GUIDE §2.1, README, SUBAGENT_ROLES) + memory**

Status: TODO
Task: Docs sweep (tool listed free-first in CLAUDE.md, RE_AGENT_GUIDE §2.1, README, SUBAGENT_ROLES) + memory

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate I5` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

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
