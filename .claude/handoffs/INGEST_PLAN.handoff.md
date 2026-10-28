# Session Handoff & Plan State: patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md

## 0. Next-Phase Direction
INGEST next batch

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`
- **Branch:** `KOTOR-II` (657e2a00 backlog: add WI-167 WI-164 hook probe: log [CClientExoAppInternal+0x274] vs [pan)
- **Generated:** 2026-10-28 00:24:37 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
657e2a00 backlog: add WI-167 WI-164 hook probe: log [CClientExoAppInternal+0x274] vs [pan
 .claude/work_items/WI-167.md         | 20 ++++++++++++++++++++
 .claude/work_items/dev_work_items.md |  1 +
 2 files changed, 21 insertions(+)
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

## 2. Immediate Objective (I6)
**Next batch: the last 26 extractable candidates (24 notes, 2 mod-code globals) through cards `--decomp-lines 10 --decoy-every 2`, Haiku verify, ledger + name_map (user OK)**

Status: TODO
Task: Next batch: the last 26 extractable candidates (24 notes, 2 mod-code globals) through cards `--decomp-lines 10 --decoy-every 2`, Haiku verify, ledger + name_map (user OK)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/ghidra_knowledge/INGEST_PLAN.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate I6` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

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
