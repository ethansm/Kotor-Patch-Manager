# Session Handoff & Plan State: .claude/plans/gemini_r2_fix_wave1.md

## 0. Next-Phase Direction
r2fx1-A done (10/10 applied, $6.83). Next: wave 2 = WI-125 (R2-CA-03, M) + WI-126 (R2-BH-03/04): items TSV + fix_wave.py plan --wave r2fx2, show batch, stop for approval. WI-133 docs -> /next-dev-work-item. Live checks queued in PLAY_TEST_QUEUE.md (WI-115/116/117/119/120).

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/gemini_r2_fix_wave1.md`
- **Branch:** `KOTOR-II` (a4e43d0 play-test queue: r2fx1-A rows WI-115/116/117/119/120 (live-only checks))
- **Generated:** 2026-10-16 15:02:33 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
a4e43d0 play-test queue: r2fx1-A rows WI-115/116/117/119/120 (live-only checks)
 patch_manager_mods/PLAY_TEST_QUEUE.md | 5 +++++
 1 file changed, 5 insertions(+)
```
- **Dirty Files (167):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/gemini_r2_fix_wave1.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ... +157 more (run `git status --short`)

## 2. Immediate Objective (WI-115)
**R2-QH-02**

Status: READY M
Task: R2-QH-02

## 2b. Upcoming Batches in Pipeline
- **WI-116** : R2-QH-03
- **WI-117** : R2-QH-06, R2-QH-07
- **WI-118** : SIDE-QH-TEST, R2-QH-10

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/gemini_r2_fix_wave1.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate WI-115` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- **Do not trust Gemini fix diffs.** BH-06 doesn't compile (float[3] assignment); QH-01 needs a forward declaration of `endHandoff`; QH-04 needs a `!headHeld && !sawExec` gate; QH-08's fix never clears "X" (setFill never clears text); CA-03's reorder does not fix the pause-driven ORPHAN; BH-15 bumps to 2.x, against the 0.X.Y rule. Use the `fix` column of validated.tsv.
- **R2-BH-08 is refuted.** Its brightPx fix breaks buff_hud_logic_test.cpp:326. Do not ingest it.
- **SIDE-QH-TEST:** queue_hud_logic_test memcmp-compares default `Config` structs; padding is uninitialised, so it fails at -O0/-O1 but passes under run_tests.py's sanitizer build (7350/0). Fix: compare field by field.
- **Haiku units ran 77k-154k tokens** each (over the 60k target) with free rg/sed reads; still cheaper than round 1's windowed layer.
- **Play-test only (queue them, never block):** QH-02 (can a stale flagless 9/10 `cur` outlive a finished buffered command), QH-05 (Tab in combat reorders the party table), BH-13 (droid shield rows 112+115 together).
- Verify: `python3 scripts/run_tests.py <suite>` scoped; `python3 scripts/wi_driver.py lint` after ingest; `python3 scripts/fix_wave.py plan <tsv> --wave <W>`.
- **r2fx1-A lessons:** `plan_manager.py mark-done` without `--plan` writes to the shared active_plan.txt target (it appended R2.x rows to holster_editor_options.md, reverted): always pass `--plan`. Reviewer worktrees can be auto-removed before review.md is copied: save the reply to `<id>/rev.reply.txt` and write review.md lines 1-2 (`DECISION`, `BLOCKING`) from it. Items in one conflict group touching the same test file need one rebase SendMessage to the consolidator (WI-118 vs WI-115).
- **Follow-ups from reviews (non-blocking, not yet WIs):** QueueHud.cpp:994 comment now wrong (engineHoldsAction vs tick actionInQueue; hand-off paths still use raw cur 9/10); NpcDiversityDispatcher stale "non-B" comment ~:184; WI-124 tables test checks only exclusivity (dropping row 619 goes unnoticed); WI-118 configEqual hand-synced with Config (no sizeof guard).
