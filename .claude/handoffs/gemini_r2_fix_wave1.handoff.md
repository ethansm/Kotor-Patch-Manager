# Session Handoff & Plan State: .claude/plans/gemini_r2_fix_wave1.md

## 0. Next-Phase Direction
Execute R2.2: Items TSV + `fix_wave.py plan` -> batch A

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/gemini_r2_fix_wave1.md`
- **Branch:** `KOTOR-II` (36dd958 work items: ingest Gemini round-2 findings as WI-115..133 (+WI-113 ND-01 note); plan R2FX1 batch A)
- **Generated:** 2026-10-16 14:16:44 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
36dd958 work items: ingest Gemini round-2 findings as WI-115..133 (+WI-113 ND-01 note); plan R2FX1 batch A
 .claude/handoffs/gemini_r2_fix_wave1.handoff.md | 55 +++++++++++++++++++++++++
 .claude/plans/gemini_r2_fix_wave1.md            | 43 +++++++++++++++++++
 .claude/work_items/WI-113.md                    |  3 ++
 .claude/work_items/WI-115.md                    | 15 ++++---
 .claude/work_items/WI-116.md                    | 12 ++++--
 .claude/work_items/WI-117.md                    | 14 +++++--
 .claude/work_items/WI-118.md                    | 13 ++++--
 .claude/work_items/WI-119.md                    | 18 +++++---
 .claude/work_items/WI-120.md                    | 10 +++--
 .claude/work_items/WI-121.md                    | 20 ++++++---
 .claude/work_items/WI-122.md                    | 14 +++++--
 .claude/work_items/WI-123.md                    | 13 ++++--
 .claude/work_items/WI-124.md                    | 16 +++++--
 .claude/work_items/WI-125.md                    | 16 ++++---
 .claude/work_items/WI-126.md                    | 15 +++++--
 .claude/work_items/WI-127.md                    | 11 +++--
 .claude/work_items/WI-128.md                    | 27 +++++++++---
 .claude/work_items/WI-129.md                    | 20 +++++++--
 .claude/work_items/WI-130.md                    | 20 +++++++--
 .claude/work_items/WI-131.md                    | 19 +++++++--
 .claude/work_items/WI-132.md                    | 13 ++++--
 .claude/work_items/WI-133.md                    | 29 +++++++++----
 22 files changed, 331 insertions(+), 85 deletions(-)
```
- **Dirty Files (167):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/fix_pipeline_plan.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/gemini_r2_fix_wave1.md`
  - ... +157 more (run `git status --short`)

## 2. Immediate Objective (R2.2)
**Items TSV + `fix_wave.py plan` -> batch A**

Status: OPEN
Task: Items TSV + `fix_wave.py plan` -> batch A

## 2b. Upcoming Batches in Pipeline
- **R2.3** [Research & Probes] : Run batch A (Haiku variants -> Sonnet cons -> Sonnet rev)
- **R2.4** [Research & Probes] : Apply in conflict order, close WIs, play-test rows
- **R2.5** [Research & Probes] : Close: cost, memory yield line, handoff

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/gemini_r2_fix_wave1.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate R2.2` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- **Do not trust Gemini fix diffs.** BH-06 doesn't compile (float[3] assignment); QH-01 needs a forward declaration of `endHandoff`; QH-04 needs a `!headHeld && !sawExec` gate; QH-08's fix never clears "X" (setFill never clears text); CA-03's reorder does not fix the pause-driven ORPHAN; BH-15 bumps to 2.x, against the 0.X.Y rule. Use the `fix` column of validated.tsv.
- **R2-BH-08 is refuted.** Its brightPx fix breaks buff_hud_logic_test.cpp:326. Do not ingest it.
- **SIDE-QH-TEST:** queue_hud_logic_test memcmp-compares default `Config` structs; padding is uninitialised, so it fails at -O0/-O1 but passes under run_tests.py's sanitizer build (7350/0). Fix: compare field by field.
- **Haiku units ran 77k-154k tokens** each (over the 60k target) with free rg/sed reads; still cheaper than round 1's windowed layer.
- **Play-test only (queue them, never block):** QH-02 (can a stale flagless 9/10 `cur` outlive a finished buffered command), QH-05 (Tab in combat reorders the party table), BH-13 (droid shield rows 112+115 together).
- Verify: `python3 scripts/run_tests.py <suite>` scoped; `python3 scripts/wi_driver.py lint` after ingest; `python3 scripts/fix_wave.py plan <tsv> --wave <W>`.
