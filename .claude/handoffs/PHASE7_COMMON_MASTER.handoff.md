# Session Handoff & Plan State: research/companion_levelup/PHASE7_COMMON_MASTER.md

## 0. Next-Phase Direction
Execute C7: KpLog migration (C7a design / C7b code)

## 1. Active State & Provenance
- **Master Plan:** `research/companion_levelup/PHASE7_COMMON_MASTER.md`
- **Branch:** `KOTOR-II` (0fd1151 refactor(level-up-override): adopt FileWatch.h for config, table, and playthrough polling)
- **Last Landed Commit:**
```
0fd1151 refactor(level-up-override): adopt FileWatch.h for config, table, and playthrough polling
 .../level_up_override_src/LevelUpOverride.cpp      | 66 ++++++++++------------
 .../level_up_override_src/LevelUpUi.h              |  2 +-
 .../level_up_override_src/common_contract_test.cpp | 28 +++++++++
 .../phase6_evidence/A3_mutation_report.md          |  2 +-
 .../phase7_evidence/C6_filewatch_adoption.md       | 42 ++++++++++++++
 .../companion_levelup/tools/parity/ui_test.cpp     |  2 +-
 .../tools/parity/unit_gap_test.cpp                 |  6 +-
 7 files changed, 105 insertions(+), 43 deletions(-)
```
- **Dirty Files (188):**
  - `M .claude/handoffs/18_mouse_camera_investigation.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`
  - ` M patch_manager_mods/10_engine_optimization_research.md`
  - ` M patch_manager_mods/13_ranged_ai_spacing.md`
  - ` M patch_manager_mods/18_mouse_camera_investigation.md`
  - ` M patch_manager_mods/19b_cross_patch_sharing_survey.md`

## 2. Immediate Objective (C7)
**KpLog migration (C7a design / C7b code)**

Status: TODO
Task: KpLog migration (C7a design / C7b code)

## 2b. Upcoming Batches in Pipeline
- **C8** [Common Extraction] : Library DLL interfaces: consumer requirements (design only)
- **C9** [Common Extraction] : Offers to Common owners (notes only)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume C7` (or `resume PHASE7_COMMON_MASTER`).

## 4. Subagent Delegation Direction
- **Delegate to Subagent**: `python3 scripts/plan_manager.py delegate C7`
- **Supported Commands**: `oc-subagent <scout|code|ghidra|verify|gate|doc>` or `oc-flow auto "<task>"`
