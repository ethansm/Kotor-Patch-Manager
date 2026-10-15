# Session Handoff & Plan State: .claude/plans/variant_fix_wave_skill.md

## 0. Next-Phase Direction
All phases complete. Run master tests and prepare PR.

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/variant_fix_wave_skill.md`
- **Branch:** `KOTOR-II` (821937c CLAUDE.md: advertise fix_wave.py select|draft-collect (next route))
- **Generated:** 2026-10-15 19:21:41 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
821937c CLAUDE.md: advertise fix_wave.py select|draft-collect (next route)
 CLAUDE.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```
- **Dirty Files (161):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/42_holstered_weapons_master_plan.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/plans/variant_fix_wave_skill.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M patch_manager_mods/34_npc_diversity_dispatcher.md`
  - ... +151 more (run `git status --short`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/variant_fix_wave_skill.md`.

## 5. Dead Ends & Verification (manual)
- VW landed 08d1156 (skill + fix_wave.py), 288e0ae/74ea956 plan rows. FX3 plan table: A 10/44/$13.88, B 11/35/$14.49, C 2/6/$2.61.
- Parallel Haiku workers: the repo-wide `agent_contract.py check --scope` FAILs on sibling + peer dirt; verify by `git status --short` that only the unit's paths are new.
- A peer commit (6650190) swept the SUBAGENT_ROLES.md exception line in; it is in HEAD. Never re-add it.
- The legacy fix3 wave dir reads WI-102/WI-91a as `revise` (hand-named review2/review_rev2 files): expected, no legacy support.
- `wi_driver close` requires a live claim (`need_claim`), and claims go stale after 7200 s (`wi_state.STALE_SECONDS`): VS must claim and heartbeat.
- `/home/ethansm/tmp` deletion still awaits the user's answer.
- Verify: `python3 scripts/run_tests.py py:scripts/test_fix_wave_plan.py py:scripts/test_fix_wave_brief.py py:scripts/test_fix_wave_collect.py py:scripts/test_fix_wave_apply.py py:scripts/test_fix_wave.py py:scripts/test_agent_contract.py py:scripts/test_wave_tools.py` (7 PASS at 08d1156).
