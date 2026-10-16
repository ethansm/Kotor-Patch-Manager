# Session Handoff & Plan State: .claude/plans/gemini_r2_fix_wave1.md

## 0. Next-Phase Direction
we should ingest these into WorkItems to be tracked, and can pick up a wave of them with the variant-fix-wave skill. Plan the first fix wave run.

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/gemini_r2_fix_wave1.md`
- **Branch:** `KOTOR-II` (7322abb plan HW-EDIT v2: holster editor pivots to a dedicated in-game Options screen patch (independent of equip3d/ultrawide))
- **Generated:** 2026-10-16 14:08:55 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
7322abb plan HW-EDIT v2: holster editor pivots to a dedicated in-game Options screen patch (independent of equip3d/ultrawide)
 .../42_holstered_weapons_master_plan.handoff.md    |   1 +
 .claude/plans/holster_editor_options.md            | 121 +++++++++++++++++++++
 .claude/research/holster_edit/UX_OPTIONS.md        |   4 +
 3 files changed, 126 insertions(+)
```
- **Dirty Files (166):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ... +156 more (run `git status --short`)

## 2. Immediate Objective (R2.1)
**Ingest: create WIs from validated.tsv (reachable + SIDE-QH-TEST + test gaps ready; latent/doc grouped)**

Status: OPEN
Task: Ingest: create WIs from validated.tsv (reachable + SIDE-QH-TEST + test gaps ready; latent/doc grouped)

## 2b. Upcoming Batches in Pipeline
- **R2.2** [Research & Probes] : Items TSV + `fix_wave.py plan` -> batch A
- **R2.3** [Research & Probes] : Run batch A (Haiku variants -> Sonnet cons -> Sonnet rev)
- **R2.4** [Research & Probes] : Apply in conflict order, close WIs, play-test rows

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/gemini_r2_fix_wave1.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate R2.1` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- **Do not trust Gemini fix diffs.** BH-06 doesn't compile (float[3] assignment); QH-01 needs a forward declaration of `endHandoff`; QH-04 needs a `!headHeld && !sawExec` gate; QH-08's fix never clears "X" (setFill never clears text); CA-03's reorder does not fix the pause-driven ORPHAN; BH-15 bumps to 2.x, against the 0.X.Y rule. Use the `fix` column of validated.tsv.
- **R2-BH-08 is refuted.** Its brightPx fix breaks buff_hud_logic_test.cpp:326. Do not ingest it.
- **SIDE-QH-TEST:** queue_hud_logic_test memcmp-compares default `Config` structs; padding is uninitialised, so it fails at -O0/-O1 but passes under run_tests.py's sanitizer build (7350/0). Fix: compare field by field.
- **Haiku units ran 77k-154k tokens** each (over the 60k target) with free rg/sed reads; still cheaper than round 1's windowed layer.
- **Play-test only (queue them, never block):** QH-02 (can a stale flagless 9/10 `cur` outlive a finished buffered command), QH-05 (Tab in combat reorders the party table), BH-13 (droid shield rows 112+115 together).
- Verify: `python3 scripts/run_tests.py <suite>` scoped; `python3 scripts/wi_driver.py lint` after ingest; `python3 scripts/fix_wave.py plan <tsv> --wave <W>`.
