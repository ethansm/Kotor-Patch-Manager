# Session Handoff & Plan State: patch_manager_mods/42c_smooth_holstering_master_plan.md

## 0. Next-Phase Direction
Execute HW28b: Draw cap raise (`draw_slide_cap`, default 570)

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/42c_smooth_holstering_master_plan.md`
- **Branch:** `KOTOR-II` (517b450c play queue: SM-L3/L4/L5/L10 for holster 0.15.0-dev (HW26 orientation, HW27 topology + holsterable, HW28 per-category timing))
- **Generated:** 2026-10-21 03:23:23 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
517b450c play queue: SM-L3/L4/L5/L10 for holster 0.15.0-dev (HW26 orientation, HW27 topology + holsterable, HW28 per-category timing)
 patch_manager_mods/PLAY_TEST_QUEUE.md | 4 ++++
 1 file changed, 4 insertions(+)
```
- **Dirty Files (185):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M .claude/settings.json`
  - ` M CLAUDE.md`
  - ... +175 more (run `git status --short`)

## 2. Immediate Objective (HW28b)
**Draw cap raise (`draw_slide_cap`, default 570)**

Status: GATED
Task: Draw cap raise (`draw_slide_cap`, default 570)

## 2b. Upcoming Batches in Pipeline
- **HW29** : Swing/twist split: twist on the forearm (cap 80), swing on the wrist (cap 60); optional pole bias from the target hand orientation at f<=0.30
- **HW30** : Shape data: generated stem table `HolsterShape.inc` (axis, length, exit axis/len) from R17 / ItemFitTable; doc 42 sec 4 refreshed to K2 numbers
- **HW31** : Extraction + orientation schedule: extract share, T offset along the exit axis, hold mount orientation through extract, via-orientation for rifle/staff

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/42c_smooth_holstering_master_plan.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate HW28b` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
