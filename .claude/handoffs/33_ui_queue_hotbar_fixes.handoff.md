# Session Handoff & Plan State: patch_manager_mods/33_ui_queue_hotbar_fixes.md

## 0. Next-Phase Direction
Execute QH4: [Queue HUD] Buffer non-combat Force powers & items in unified FIFO (`haveCur` idle gate, `fakeAction` icon resolution)

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/33_ui_queue_hotbar_fixes.md`
- **Branch:** `KOTOR-II` (22b56b4 fix(equip3d): destroy creature on character change, add anim fallback, sync screen)
- **Last Landed Commit:**
```
22b56b4 fix(equip3d): destroy creature on character change, add anim fallback, sync screen
 .../EquipCharPreview.cpp                           | 16 ++++++---
 .../equipment_3d_character_src/PULL_REQUEST.md     | 42 ++++++++++++++++++++++
 2 files changed, 54 insertions(+), 4 deletions(-)
```
- **Dirty Files (190):**
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

## 2. Immediate Objective (QH4)
**[Queue HUD] Buffer non-combat Force powers & items in unified FIFO (`haveCur` idle gate, `fakeAction` icon resolution)**

Status: PLANNED
Task: [Queue HUD] Buffer non-combat Force powers & items in unified FIFO (`haveCur` idle gate, `fakeAction` icon resolution)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume QH4` (or `resume 33_ui_queue_hotbar_fixes`).

## 4. Subagent Delegation Direction
- **Delegate to Subagent**: `python3 scripts/plan_manager.py delegate QH4`
- **Supported Commands**: `oc-subagent <scout|code|ghidra|verify|gate|doc>` or `oc-flow auto "<task>"`
