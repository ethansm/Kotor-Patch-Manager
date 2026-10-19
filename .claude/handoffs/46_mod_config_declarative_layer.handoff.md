# Session Handoff & Plan State: patch_manager_mods/46_mod_config_declarative_layer.md

## 0. Next-Phase Direction
MCF5

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/46_mod_config_declarative_layer.md`
- **Waves:** .claude/waves/WI-46-a1/
- **Waves:** .claude/waves/WI-46-a2/
- **Waves:** .claude/waves/WI-46-a3/
- **Branch:** `KOTOR-II` (657016d6 doc 46: section 17 build log (MCF1-MCF4, MCF2b))
- **Generated:** 2026-10-18 23:32:58 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
657016d6 doc 46: section 17 build log (MCF1-MCF4, MCF2b)
 .../46_mod_config_declarative_layer.md             | 32 ++++++++++++++++++++++
 1 file changed, 32 insertions(+)
```
- **Dirty Files (176):**
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
  - ... +166 more (run `git status --short`)

## 2. Immediate Objective (MCF5)
**[Mod-config] Look-alike slider row (needs Holster-session SliderArt extraction)**

Status: PLANNED
Task: [Mod-config] Look-alike slider row (needs Holster-session SliderArt extraction)

## 2b. Upcoming Batches in Pipeline
- **MCF1** [Meta / Planning Setup] : [Mod-config] Pure logic in MenuUiLogic.h: row kinds (toggle/stepper), ScrollWindow (offset clamp, wheel step, ensure-visible, thumb px), `mui::tickResetDue(gap, flags48, lmbDown)` + tests
- **MCF6** [Meta / Planning Setup] : [Mod-config] Native slider embed probe (gated by MCF0)
- **MCF7** [Meta / Planning Setup] : [Enemy strip] Migrate its page to the declarative layer

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/46_mod_config_declarative_layer.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate MCF5` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_

## Session note 10-18 night (read first after /clear)
Full build log, deviations, review findings and open items: `patch_manager_mods/46_mod_config_declarative_layer.md` section 17. State: MCF1-MCF4 + MCF2b BUILT and INSTALLED (LIVE 10-18 23:32, rollback `python3 scripts/kpm_hotswap.py --rollback batch_swap_20261018_233209`); nothing played yet (PLAY_TEST_QUEUE rows MCR1, MCR2, SH-PG1 pending). Opus review 2 (MCF2b/MCF3/MCF4) was still running: check `git log --oneline -10` for a commit starting `mod-config/selfheal review fixes (MCF2b-MCF4):`; if present, re-gate, `--prepare` (must be SWAP), and re-swap with user GO. Next phases MCF5-MCF7 are later rows; play-test results decide the scroll bar look and wheel delivery first. Read logs only through `kotor2_logs.py check mod-config_log.txt --new` and `selfheal_log_check.py`.
