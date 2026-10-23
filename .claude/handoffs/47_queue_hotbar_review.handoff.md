# Session Handoff & Plan State: patch_manager_mods/47_queue_hotbar_review.md

## 0. Next-Phase Direction
RV7: install. Prereq: user OK to run check_shared_headers.py --sync (3 drifted: ActQueue.h, GameAddr.h, SelfHealUi.h; additive) + commit KPM Common separately; then pre_install_gate.sh --mod queue_hud_src --mod companion_selfheal_src --mod action_hotbar_src, kpm_hotswap.py --mod queue-hud companion-selfheal action-hotbar --prepare, fresh gate, --apply (game closed). RV1-RV6 commits 9fc34ef6 2ee85867 8422f729 be162666 24f127ba e9edc3d7, rows 55c240cf.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/47_queue_hotbar_review.md`
- **Waves:** .claude/waves/WI-47-a1/
- **Waves:** .claude/waves/WI-47-a2/
- **Branch:** `KOTOR-II` (55c240cf doc 47: RV1-RV6 DONE in plan table; PLAY rows QV-*, RV3-*, QH-RL*, SH-RQ13a/b, AHR1-6 (QH21-24 retargeted to 0.4.9-dev))
- **Generated:** 2026-10-23 17:13:48 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
55c240cf doc 47: RV1-RV6 DONE in plan table; PLAY rows QV-*, RV3-*, QH-RL*, SH-RQ13a/b, AHR1-6 (QH21-24 retargeted to 0.4.9-dev)
 patch_manager_mods/47_queue_hotbar_review.md | 12 +++++------
 patch_manager_mods/PLAY_TEST_QUEUE.md        | 32 ++++++++++++++++++++++++----
 2 files changed, 34 insertions(+), 10 deletions(-)
```
- **Dirty Files (184):**
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
  - ... +174 more (run `git status --short`)

## 2. Immediate Objective (RV7)
**install (hot-swap queue-hud, action-hotbar, companion-selfheal) + PLAY_TEST_QUEUE rows**

Status: TODO
Task: install (hot-swap queue-hud, action-hotbar, companion-selfheal) + PLAY_TEST_QUEUE rows

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/47_queue_hotbar_review.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate RV7` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
- Session 10-23 (review): QL20 LIVE 15:44 (queue-hud 0.4.5-dev, dll 99daa99e, rollback `queue-hud_swap_20261023_154447`, play QH20). Review commit 2ef99fa1.
- Versions at start: queue-hud 0.4.5-dev, companion-selfheal 0.9.3-dev, action-hotbar 0.1.5-dev. Spec bumps: queue-hud 0.4.6/0.4.7/0.4.8/0.4.9-dev (RV1/RV2/RV3/RV6), selfheal 0.9.4-dev, hotbar 0.1.6-dev.
- Specs are executor-ready and reviewed (review_RV12/RV36/RV45.md folded in; each spec ends "Review corrections applied"). Anchors are by text; RV2/RV3/RV6 Before-snippets assume the earlier phase landed.
- Dead ends: RQ12 pop/re-add of a companion's queue (no per-node remove in GameAddr.h; skips ClearAllActions bookkeeping) -> measure-first only. RQ6 hook half is dead code (hooks return pass_combat in combat). Do not "swallow digits whenever g_hudGateOk" (RL30/V4-13). Do not pass NULL as the cast pos pointer (builder 0x87A5C0 derefs it).
- Verify per phase: `python3 scripts/run_tests.py queue_hud` (scoped), `python3 -m pytest -q scripts/test_queue_hud_log_check.py`, selfheal: `python3 scripts/run_tests.py selfheal` + `scripts/test_selfheal_log_check.py`, hotbar: `python3 scripts/run_tests.py hotbar`; `_shared_src` touched -> `python3 scripts/compat_check.py <files>`; before install `scripts/pre_install_gate.sh --mod queue_hud_src --mod companion_selfheal_src --mod action_hotbar_src`.
- Other session's dirty files in tree (MASTER_KEY.md, CLAUDE.md, ...): never stage them; commit by explicit paths only.
