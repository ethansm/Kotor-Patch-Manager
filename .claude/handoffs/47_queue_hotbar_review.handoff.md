# Session Handoff & Plan State: patch_manager_mods/47_queue_hotbar_review.md

## 0. Next-Phase Direction
47 RV1-RV6: implement the review fixes from the reviewed executor specs .claude/reviews/rv_qac_20261023/plan_RV<n>.md in order RV1 -> RV2 -> RV3 -> RV6 (queue-hud), RV4 (selfheal), RV5 (action-hotbar); RV7 install needs user GO

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/47_queue_hotbar_review.md`
- **Waves:** .claude/waves/WI-47-a1/
- **Waves:** .claude/waves/WI-47-a2/
- **Branch:** `KOTOR-II` (2ef99fa1 doc 47: queue-hud/action-hotbar/companion-queue review (RQ1-RQ13, RH1-RH5, RL1-RL31) + phases RV1-RV7 with reviewed executor specs)
- **Generated:** 2026-10-23 16:34:17 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
2ef99fa1 doc 47: queue-hud/action-hotbar/companion-queue review (RQ1-RQ13, RH1-RH5, RL1-RL31) + phases RV1-RV7 with reviewed executor specs
 .claude/reviews/rv_qac_20261023/PLAN_BRIEF.md      |  34 ++
 .../reviews/rv_qac_20261023/SPEC_REVIEW_BRIEF.md   |  22 ++
 .../reviews/rv_qac_20261023/haiku/BRIEF_COMMON.md  |  42 +++
 .../rv_qac_20261023/haiku/VALIDATE_BRIEF.md        |  34 ++
 .claude/reviews/rv_qac_20261023/haiku/h01.md       |  71 ++++
 .claude/reviews/rv_qac_20261023/haiku/h02.md       |  78 ++++
 .claude/reviews/rv_qac_20261023/haiku/h03.md       |  64 ++++
 .claude/reviews/rv_qac_20261023/haiku/h04.md       |  50 +++
 .claude/reviews/rv_qac_20261023/haiku/h05.md       |  85 +++++
 .claude/reviews/rv_qac_20261023/haiku/h06.md       |  57 +++
 .claude/reviews/rv_qac_20261023/haiku/h07.md       |  78 ++++
 .claude/reviews/rv_qac_20261023/haiku/h08.md       |  50 +++
 .claude/reviews/rv_qac_20261023/haiku/h09.md       |  57 +++
 .claude/reviews/rv_qac_20261023/haiku/h10.md       |  64 ++++
 .claude/reviews/rv_qac_20261023/haiku/h11.md       |  92 +++++
 .claude/reviews/rv_qac_20261023/haiku/h12.md       |  57 +++
 .claude/reviews/rv_qac_20261023/plan_RV1.md        | 366 +++++++++++++++++++
 .claude/reviews/rv_qac_20261023/plan_RV2.md        | 395 +++++++++++++++++++++
 .claude/reviews/rv_qac_20261023/plan_RV3.md        | 315 ++++++++++++++++
 .claude/reviews/rv_qac_20261023/plan_RV4.md        | 176 +++++++++
 .claude/reviews/rv_qac_20261023/plan_RV5.md        | 342 ++++++++++++++++++
 .claude/reviews/rv_qac_20261023/plan_RV6.md        | 284 +++++++++++++++
 .claude/reviews/rv_qac_20261023/review_RV12.md     |  71 ++++
 .claude/reviews/rv_qac_20261023/review_RV36.md     |  53 +++
 .claude/reviews/rv_qac_20261023/review_RV45.md     |  78 ++++
 .claude/reviews/rv_qac_20261023/v01.md             | 142 ++++++++
 .claude/reviews/rv_qac_20261023/v02.md             | 138 +++++++
 .claude/reviews/rv_qac_20261023/v03.md             | 147 ++++++++
 .claude/reviews/rv_qac_20261023/v04.md             | 148 ++++++++
 patch_manager_mods/47_queue_hotbar_review.md       | 287 +++++++++++++++
 30 files changed, 3877 insertions(+)
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

## 2. Immediate Objective (RV1)
**queue-hud high: RQ1 spell replay pos/flag, RQ5 RI_OK gates + serverCast bool/deferred retry (+RL12), RQ2 edge head guard, RQ3 detach excludes head**

Status: TODO
Task: queue-hud high: RQ1 spell replay pos/flag, RQ5 RI_OK gates + serverCast bool/deferred retry (+RL12), RQ2 edge head guard, RQ3 detach excludes head

## 2b. Upcoming Batches in Pipeline
- **RV2** [Research & Probes] : queue-hud verdict: RQ4 orphan round node + present ceiling, RQ6 combat-end resolve (hook half dead), RQ7 executing evidence tied to our action, RL1 counter resets, RL2 swap bookkeeping, RL3 unpaused FlagGate, RL5 re-inject total cap, RL8 kick give-up
- **RV3** [Research & Probes] : queue-hud hand-off/Cancel: RQ8 restore unsent head, RQ9 detach at combat end, RQ10 detached owner, RQ11 spatial cancel click, RQ12 companion Cancel measure-first (CANCEL had3D / CANCEL_AFTER after3D), RL6, RL11, RL17-RL19
- **RV4** [Research & Probes] : cross-mod: RQ13 actq::castQueued + selfheal dedupe, selfheal wiped-pending end

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/47_queue_hotbar_review.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate RV1` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
- Session 10-23 (review): QL20 LIVE 15:44 (queue-hud 0.4.5-dev, dll 99daa99e, rollback `queue-hud_swap_20261023_154447`, play QH20). Review commit 2ef99fa1.
- Versions at start: queue-hud 0.4.5-dev, companion-selfheal 0.9.3-dev, action-hotbar 0.1.5-dev. Spec bumps: queue-hud 0.4.6/0.4.7/0.4.8/0.4.9-dev (RV1/RV2/RV3/RV6), selfheal 0.9.4-dev, hotbar 0.1.6-dev.
- Specs are executor-ready and reviewed (review_RV12/RV36/RV45.md folded in; each spec ends "Review corrections applied"). Anchors are by text; RV2/RV3/RV6 Before-snippets assume the earlier phase landed.
- Dead ends: RQ12 pop/re-add of a companion's queue (no per-node remove in GameAddr.h; skips ClearAllActions bookkeeping) -> measure-first only. RQ6 hook half is dead code (hooks return pass_combat in combat). Do not "swallow digits whenever g_hudGateOk" (RL30/V4-13). Do not pass NULL as the cast pos pointer (builder 0x87A5C0 derefs it).
- Verify per phase: `python3 scripts/run_tests.py queue_hud` (scoped), `python3 -m pytest -q scripts/test_queue_hud_log_check.py`, selfheal: `python3 scripts/run_tests.py selfheal` + `scripts/test_selfheal_log_check.py`, hotbar: `python3 scripts/run_tests.py hotbar`; `_shared_src` touched -> `python3 scripts/compat_check.py <files>`; before install `scripts/pre_install_gate.sh --mod queue_hud_src --mod companion_selfheal_src --mod action_hotbar_src`.
- Other session's dirty files in tree (MASTER_KEY.md, CLAUDE.md, ...): never stage them; commit by explicit paths only.
