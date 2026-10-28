# Session Handoff & Plan State: patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md

## 0. Next-Phase Direction
Execute I3: Research-notes extraction (same tool, `--source notes`), top-cited first, Haiku verify, ledger rows topic `ingest-notes`; stop when yield < 30%

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`
- **Branch:** `KOTOR-II` (53747f2f skill kotor2-persist-knowledge + AGENTS.md section: persist ad-hoc research to saved knowledge)
- **Generated:** 2026-10-27 22:47:31 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
53747f2f skill kotor2-persist-knowledge + AGENTS.md section: persist ad-hoc research to saved knowledge
 .claude/skills/kotor2-persist-knowledge/SKILL.md | 42 ++++++++++++++++++++++++
 AGENTS.md                                        |  5 +++
 2 files changed, 47 insertions(+)
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

## 2. Immediate Objective (I3)
**Research-notes extraction (same tool, `--source notes`), top-cited first, Haiku verify, ledger rows topic `ingest-notes`; stop when yield < 30%**

Status: TODO
Task: Research-notes extraction (same tool, `--source notes`), top-cited first, Haiku verify, ledger rows topic `ingest-notes`; stop when yield < 30%

## 2b. Upcoming Batches in Pipeline
- **I4** : Promotion (user OK) of verified names into `name_map`; regen class pages for touched classes; gap re-measure
- **I5** : Docs sweep (tool listed free-first in CLAUDE.md, RE_AGENT_GUIDE §2.1, README, SUBAGENT_ROLES) + memory

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate I3` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

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
