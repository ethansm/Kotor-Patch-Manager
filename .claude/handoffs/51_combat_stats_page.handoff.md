# Session Handoff & Plan State: patch_manager_mods/51_combat_stats_page.md

## 0. Next-Phase Direction
51 ST3 mockup

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/51_combat_stats_page.md`
- **Branch:** `KOTOR-II` (fb39ff72 doc 51 ST1+ST2: CombatStatsAddr.h read paths + pure CombatStats.h math + host test)
- **Generated:** 2026-10-27 01:23:39 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
fb39ff72 doc 51 ST1+ST2: CombatStatsAddr.h read paths + pure CombatStats.h math + host test
 .../active_effects_panel_src/CombatStats.h         | 152 ++++++++++++++++++
 .../active_effects_panel_src/CombatStatsAddr.h     |  54 +++++++
 .../active_effects_panel_src/combat_stats_test.cpp | 170 +++++++++++++++++++++
 .../active_effects_panel_src/test.toml             |   3 +
 4 files changed, 379 insertions(+)
```
- **Dirty Files (188):**
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
  - ... +178 more (run `git status --short`)

## 2. Immediate Objective (ST3)
**Mockup: `scripts/character_page_mockup.py` renders the Stats tab + one vanilla-row tooltip (Defense)**

Status: TODO
Task: Mockup: `scripts/character_page_mockup.py` renders the Stats tab + one vanilla-row tooltip (Defense)

## 2b. Upcoming Batches in Pipeline
- **ST4** [Sync / Header Mirroring] : Probe 0.3.0-probe: STATS log block per shown character (no UI) + read-only deflection check (engine total vs recompute)
- **ST5** [Sync / Header Mirroring] : Stats tab UI (header toggle, sections, rows, breakdown tooltips, `show_influence`)
- **ST6** [Sync / Header Mirroring] : Vanilla page tooltips (hover rects by tag, `vanilla_tooltips=1`, text table)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/51_combat_stats_page.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate ST3` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
- W1 Haiku claimed a Weapon Master Deflect "engine bug" (bonus never summed): REFUTED by W2 (summed at asm 006e1ef9). Never ship a bug claim from one Haiku layer.
- stats+0x178/+0x179 (ungated deflection/skill term): only CSWSCreatureStats::Constructor writes them (0) per `field_census.py --offset 0x178 --all --writers`; treat as always 0.
- GetAttacksPerRound 0x006a7170 is a stub returning 1; the real count is written by CalculateOnHandAttacks 0x005905f0 into round+0xae0.
- All ST1 functions are already `verified` in the Steam DB (`python3 scripts/query_ghidra.py address <hex>`); no new DB rows or Haiku decoy verification needed, only mod-local constants.
- Haiku RE briefs: token caps are ignored (peaks 112-200k); cap tool calls <= 12 and name line ranges.
- Verify: `python3 scripts/run_tests.py combat_stats_test` (after ST2 registers it in active_effects_panel_src/test.toml); `python3 scripts/compat_check.py <files>` only if _shared_src is touched (plan: it is not).
