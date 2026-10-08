# Session Handoff & Plan State: research/companion_levelup/PHASE7_COMMON_MASTER.md

## 0. Next-Phase Direction
Execute C3: Include style + parity stub alignment

## 1. Active State & Provenance
- **Master Plan:** `research/companion_levelup/PHASE7_COMMON_MASTER.md`
- **Branch:** `KOTOR-II` (83cb016 docs(levelup): complete Phase 7 Chunk C1 integration audit baseline)
- **Last Landed Commit:**
```
83cb016 docs(levelup): complete Phase 7 Chunk C1 integration audit baseline
 .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md   | 61 +++++++++---------
 research/companion_levelup/PHASE7_COMMON_MASTER.md |  2 +-
 .../phase7_evidence/C1_integration_audit.md        | 72 ++++++++++++++++++++++
 3 files changed, 101 insertions(+), 34 deletions(-)
```
- **Dirty Files (200):**
  - `M .claude/handoffs/18_mouse_camera_investigation.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M AGENTS.md`
  - ` M patch_manager_mods/10_engine_optimization_research.md`
  - ` M patch_manager_mods/31_combat_action_hotbar.md`
  - ` M patch_manager_mods/33_ui_queue_hotbar_fixes.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/action_hotbar_src/ActionHotbar.cpp`

## 2. Immediate Objective (C3)
**Include style + parity stub alignment**

Status: TODO
Task: Include style + parity stub alignment

## 2b. Upcoming Batches in Pipeline
- **C4** [Common Extraction] : Engine addresses -> GameAddr.h (+ proposal)
- **C5** [Common Extraction] : Party.h adoption
- **C6** [Common Extraction] : FileWatch.h adoption

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume C3` (or `resume PHASE7_COMMON_MASTER`).

## 4. Subagent Delegation Direction
- **Delegate to Subagent**: `python3 scripts/plan_manager.py delegate C3`
- **Supported Commands**: `oc-subagent <scout|code|ghidra|verify|gate|doc>` or `oc-flow auto "<task>"`
