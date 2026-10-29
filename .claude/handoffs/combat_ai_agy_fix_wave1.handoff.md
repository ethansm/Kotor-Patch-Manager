# Session Handoff & Plan State: .claude/plans/combat_ai_agy_fix_wave1.md

## 0. Next-Phase Direction
Execute CAX.4 (hold items WI-176/177/178: owner decisions first); CAX.3 install awaits user GO

## 1. Active State & Provenance
- **Master Plan:** `.claude/plans/combat_ai_agy_fix_wave1.md`
- **Branch:** `KOTOR-II` (d2b9b187 play queue: CA-DR1/SK1/O3S/BT1 pending (CAX.3, not installed))
- **Generated:** 2026-10-29 02:37:43 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
d2b9b187 play queue: CA-DR1/SK1/O3S/BT1 pending (CAX.3, not installed)
 patch_manager_mods/PLAY_TEST_QUEUE.md | 4 ++++
 1 file changed, 4 insertions(+)
```
- **Dirty Files (200):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/50_k2_render_performance.handoff.md`
  - ` M .claude/handoffs/51_combat_stats_page.handoff.md`
  - ` M .claude/handoffs/52_quick_loot.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/combat_ai_agy_fix_wave1.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ... +190 more (run `git status --short`)

## 2. Immediate Objective (CAX.4)
**Hold items: WI-176 (O3 reject fallback), WI-177 (combo 29), WI-178 (intruder-first)**

Status: TODO
Task: Hold items: WI-176 (O3 reject fallback), WI-177 (combo 29), WI-178 (intruder-first)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume .claude/plans/combat_ai_agy_fix_wave1.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate CAX.4` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
- Agy review spell IDs are wrong in several places; trust validated.tsv fix_note (Enlightenment 180, Rage 271-273, Crush 177).
- +0x10E8 is NOT "controlled": it stays on the Exile (RE_AGENT_GUIDE 3.8). Controlled leader id = CServerExoApp::GetPlayerCreatureId 0x0051C8F0 (verified B9), as CompanionAI.cpp:973/1034 use.
- Recon facts for batch 1: .claude/waves/w1-CAX-2/u01.md (WI-169), u02.md (WI-170), u03.md (WI-171).
- Tests: `python3 scripts/run_tests.py rai_logic_test`, `run_tests.py companion_ai_logic_test`, `run_tests.py -k test_party_ai_patches`; build: `python3 scripts/build_check.py --mod patch_manager_mods/<x>_src`.
- CAX.3 trap: review/validated.tsv k_inc_generic.nss line numbers run ~180 lines ahead of `~/Games/KOTOR2_Mod_Workspace/mod8_kite_script/vanilla_all/k_inc_generic.nss` (3257 -> 3077, 754 -> ~690, 2678 -> 2520). Anchor by text, never by the review's line numbers. w1-CAX-3 u01/u02/u04 recon went stale for this reason; the CAX.3 plan holds Opus-verified anchors (each counted once in the CRLF->LF vanilla text).
- Spell 44 = FORCE_POWER_SLEEP (in-game "Mass Stasis"); series per k_inc_gensupport.nss:1665-1680: Fear 16/Horror 30/Insanity 31, Hold 29/Sleep 44/Stun 46.
- build_check takes `--mod <x>_src` (relative to patch_manager_mods/), not the repo path. build_party_ai_scripts.py needs `./.venv/bin/python` (pykotor).
- Install gate 10-29: default tier failed on other sessions' work (21 raw addresses in active_effects_panel_src; enemy-effects-strip contract test vs a peer's dirty _shared_src/EffectIcons.h). Ran `--skip-default` after checking ours were clean; new raw hex in a mod needs a raw_addresses.toml row (query_ghidra first).
