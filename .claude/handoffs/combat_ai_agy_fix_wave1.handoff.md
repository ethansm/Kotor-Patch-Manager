# Session Handoff & Plan State: .claude/plans/combat_ai_agy_fix_wave1.md

## 0. Next-Phase Direction
Execute CAX.3: party-script batch WI-172..175

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/combat_ai_agy_fix_wave1.md`
- **Branch:** `KOTOR-II` (0bba15ca play queue: CA-RH1 / CA-EN1 / CA-F4K (WI-169/171/170, not installed))
- **Generated:** 2026-10-29 01:27:22 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
0bba15ca play queue: CA-RH1 / CA-EN1 / CA-F4K (WI-169/171/170, not installed)
 patch_manager_mods/PLAY_TEST_QUEUE.md | 3 +++
 1 file changed, 3 insertions(+)
```
- **Dirty Files (202):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/50_k2_render_performance.handoff.md`
  - ` M .claude/handoffs/51_combat_stats_page.handoff.md`
  - ` M .claude/handoffs/52_quick_loot.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ... +192 more (run `git status --short`)

## 2. Immediate Objective (CAX.3)
**Batch 2 (party scripts): WI-172 Defensive sticky target, WI-173 buff picker, WI-174 O3 series, WI-175 Drain Force droids**

Status: TODO
Task: Batch 2 (party scripts): WI-172 Defensive sticky target, WI-173 buff picker, WI-174 O3 series, WI-175 Drain Force droids

## 2b. Upcoming Batches in Pipeline
- **CAX.4** [Common Extraction] : Hold items: WI-176 (O3 reject fallback), WI-177 (combo 29), WI-178 (intruder-first)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/combat_ai_agy_fix_wave1.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate CAX.3` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- Agy review spell IDs are wrong in several places; trust validated.tsv fix_note (Enlightenment 180, Rage 271-273, Crush 177).
- +0x10E8 is NOT "controlled": it stays on the Exile (RE_AGENT_GUIDE 3.8). Controlled leader id = CServerExoApp::GetPlayerCreatureId 0x0051C8F0 (verified B9), as CompanionAI.cpp:973/1034 use.
- Recon facts for batch 1: .claude/waves/w1-CAX-2/u01.md (WI-169), u02.md (WI-170), u03.md (WI-171).
- Tests: `python3 scripts/run_tests.py rai_logic_test`, `run_tests.py companion_ai_logic_test`, `run_tests.py -k test_party_ai_patches`; build: `python3 scripts/build_check.py --mod patch_manager_mods/<x>_src`.
