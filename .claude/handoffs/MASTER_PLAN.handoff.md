# Session Handoff & Plan State: patch_manager_mods/steam_db_batches/MASTER_PLAN.md

## 0. Next-Phase Direction
Execute R2: REA + Sonnet re-pilot: 2 Sonnet reviewers on the 50 R1 pilot cards (`B13a/rea_pilot/cards_rea/`), scored with `score_pilot.py` vs Sonnet-orig 10/14; GO = LS > 10/14 or >= 1 of the 4 Opus-only rows, 0 decoys, < $0.08/card; else REA only for u06 fill

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/steam_db_batches/MASTER_PLAN.md`
- **Branch:** `KOTOR-II` (b8fec3c R1: REA live evidence for Steam DB cards (rea_questions.py, cards --rea/--out, DESIGN 4.9) + B13a pilot: NO-GO (Haiku+REA LS 7/14, 1 decoy; Sonnet 10/14))
- **Generated:** 2026-10-15 16:43:42 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
b8fec3c R1: REA live evidence for Steam DB cards (rea_questions.py, cards --rea/--out, DESIGN 4.9) + B13a pilot: NO-GO (Haiku+REA LS 7/14, 1 decoy; Sonnet 10/14)
 .../rea_findings/20261015_B13a-q1.tsv              |   10 +
 .../rea_findings/20261015_B13a-q1_2.tsv            |    9 +
 .../rea_findings/20261015_B13a-q2.tsv              |   28 +
 .../rea_findings/20261015_B13a-q2_2.tsv            |    9 +
 .../rea_findings/20261015_B13a-q3.tsv              |   35 +
 .../rea_findings/20261015_B13a-q3_2.tsv            |    9 +
 .../rea_findings/20261015_B13a-q4_2.tsv            |   22 +
 .../rea_findings/20261015_B13a-q5_2.tsv            |   17 +
 .../rea_findings/20261015_B13a-q6_2.tsv            |   15 +
 .../steam_db_batches/B13a/rea/q_4.md               |   39 +
 .../steam_db_batches/B13a/rea/q_5.md               |   39 +
 .../steam_db_batches/B13a/rea/q_6.md               |   37 +
 .../steam_db_batches/B13a/rea_pilot/PILOT.md       |   71 +
 .../B13a/rea_pilot/build_pilot_cards.py            |   60 +
 .../B13a/rea_pilot/cards_norea/pilot_1.md          | 2695 +++++++++++++++++
 .../B13a/rea_pilot/cards_norea/pilot_2.md          | 2989 +++++++++++++++++++
 .../B13a/rea_pilot/cards_rea/pilot_1.md            | 2795 ++++++++++++++++++
 .../B13a/rea_pilot/cards_rea/pilot_2.md            | 3089 ++++++++++++++++++++
 .../steam_db_batches/B13a/rea_pilot/key_pilot.tsv  |   51 +
 .../B13a/rea_pilot/review/norea_1.tsv              |   26 +
 .../B13a/rea_pilot/review/norea_2.tsv              |   26 +
 .../B13a/rea_pilot/review/rea_1.tsv                |   26 +
 .../B13a/rea_pilot/review/rea_2.tsv                |   26 +
 .../steam_db_batches/B13a/rea_pilot/score_pilot.py |   76 +
 patch_manager_mods/steam_db_batches/DESIGN.md      |    8 +
 patch_manager_mods/steam_db_batches/MASTER_PLAN.md |    3 +
 .../steam_db_batches/REVIEWER_BRIEF.md             |    2 +
 patch_manager_mods/steam_db_batches/tools/batch.py |   47 +-
 .../steam_db_batches/tools/rea_questions.py        |  195 ++
 .../steam_db_batches/tools/tests/test_rea_cards.py |  165 ++
 .../tools/tests/test_rea_questions.py              |  224 ++
 31 files changed, 12840 insertions(+), 3 deletions(-)
```
- **Dirty Files (152):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/42_holstered_weapons_master_plan.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/34_npc_diversity_dispatcher.md`
  - ... +142 more (run `git status --short`)

## 2. Immediate Objective (R2)
**REA + Sonnet re-pilot: 2 Sonnet reviewers on the 50 R1 pilot cards (`B13a/rea_pilot/cards_rea/`), scored with `score_pilot.py` vs Sonnet-orig 10/14; GO = LS > 10/14 or >= 1 of the 4 Opus-only rows, 0 decoys, < $0.08/card; else REA only for u06 fill**

Status: TODO
Task: REA + Sonnet re-pilot: 2 Sonnet reviewers on the 50 R1 pilot cards (`B13a/rea_pilot/cards_rea/`), scored with `score_pilot.py` vs Sonnet-orig 10/14; GO = LS > 10/14 or >= 1 of the 4 Opus-only rows, 0 decoys, < $0.08/card; else REA only for u06 fill

## 2b. Upcoming Batches in Pipeline
- **R3** [Research & Probes] : REA in B17 cards (`rea_questions.py` -> rea-recon <= 2-3 parallel -> `cards --rea`), only if R2 GO
- **R4** [Research & Probes] : Optional Opus calibration: may REA callers count as A-CALLEE (twin separation required first)
- **P6** [Parity Verification] : B13a CSWCCreature: ship (merge a/r/z -> finalize --opus-sample-errors 1 -> expect verified 177, label_suspect 19, unresolved 3; kdb_overrides.tsv 28 rows)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/steam_db_batches/MASTER_PLAN.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate R2` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
State 10-15: branch `addressdb/k2-steam-acc` db4f5cf, 3,420 rows, 0 dups, unpushed (unchanged since 10-03). B13a/B10a/B11b
gate work finished, not shipped. Live game addresses.db = stale 48-row subset; AddFeat differs (branch 0x6B8280, live
0x6F5B40 = level_up_override's addFeat) - check before any live sync. Mods: 37 (+17 since 10-03); mod_audit is from 10-03.
REA facts (recon 10-15, .claude memory rea-integration.md):
- REA is NOT wired into steam_db_batches (0 refs). rea-row-verify: 2 smoke runs failed on data/arithmetic (not REA);
  user chose to keep it off DB rows until a 3rd approved run passes. Cap 20 cards/run; no label_suspect verdict.
- rea-recon (Haiku, read-only Ghidra via `npx rea-agents@6.3.0 mcp`): first procedure call on swkotor2.exe ~356 s
  (REA_GHIDRA_STARTUP_TIMEOUT_MS=900000); no calling-convention labels; one binary per run, close_binary always.
- NEW cc6a424: rea-recon persists every run to `patch_manager_mods/ghidra_knowledge/rea_findings/<date>_<topic>.tsv`
  and greps it + steam_functions.tsv before open_binary; `query_ghidra.py address|symbol|stats` shows `[rea]` rows.
  Older prose findings (37_bug_board.md, .claude/waves/fix2/re.md, research/*) not backfilled.
Dead ends / traps:
- Do not run 10+ parallel Opus agents (usage limit hit 10-03); Opus 1-3, Haiku recon <=12.
- `opus-parts` deletes part files it did not write (prep_opus.sh orders it right).
- GOG labels often rotated with bodies right: AUTO rotation audit + Mac same-name score sweep every batch.
- Accepted decoys can be key errors (decoy_prev built from a mis-slotted pick).
- plan_manager parses Status by words: use `DONE:`/`TODO` prefixes (a6fa057).
Verification:
- `cd patch_manager_mods/steam_db_batches/tools && python3 -m unittest discover -s tests` (285)
- `python3 tools/batch.py check-validated | head -1` (3420 validated)
- `python3 -m unittest scripts/test_query_ghidra.py` (9)
- `W=~/Games/KOTOR2_Mod_Workspace/tools/kpm-pr-steam-db; $W/tools/SqliteTools/bin/Debug/net8.0/SqliteTools validate --database $W/AddressDatabases/kotor2_steam_aspyr.db`
