# Session Handoff & Plan State: research/companion_levelup/PHASE7_COMMON_MASTER.md

## 0. Next-Phase Direction
Execute C4: Engine addresses -> GameAddr.h (+ proposal)

## 1. Active State & Provenance
- **Master Plan:** `research/companion_levelup/PHASE7_COMMON_MASTER.md`
- **Branch:** `KOTOR-II` (d8438c6 refactor(levelup): complete Phase 7 Chunk C3 include style and parity stub alignment)
- **Last Landed Commit:**
```
d8438c6 refactor(levelup): complete Phase 7 Chunk C3 include style and parity stub alignment
 .../level_up_override_src/LevelUpOverride.cpp      |  8 ++--
 .../phase7_evidence/C3_include_style_alignment.md  | 47 ++++++++++++++++++++++
 .../companion_levelup/tools/parity/apply_test.cpp  | 14 +++++--
 .../companion_levelup/tools/parity/gate_test.cpp   | 12 ++++--
 .../companion_levelup/tools/parity/guikit_test.cpp | 13 +++---
 .../companion_levelup/tools/parity/guikit_test.py  |  2 +-
 research/companion_levelup/tools/parity/parity.py  |  2 +-
 .../tools/parity/playthrough_test.cpp              | 11 +++--
 .../tools/parity/scenario_test.cpp                 | 17 +++++---
 .../companion_levelup/tools/parity/scenarios.py    |  2 +-
 .../tools/parity/stub/TestHarness.h                |  1 +
 .../tools/parity/stub/_shared/ActQueue.h           |  1 +
 .../tools/parity/stub/_shared/CodePatch.h          |  1 +
 .../tools/parity/stub/_shared/DebuffIconTable.inc  |  1 +
 .../tools/parity/stub/_shared/EffectIconTable.inc  |  1 +
 .../tools/parity/stub/_shared/EffectIcons.h        |  1 +
 .../tools/parity/stub/_shared/Effects.h            |  1 +
 .../tools/parity/stub/_shared/FileWatch.h          |  1 +
 .../tools/parity/stub/_shared/FontWidths.inc       |  1 +
 .../tools/parity/stub/_shared/Gui2D.h              |  1 +
 .../tools/parity/stub/_shared/GuiInput.h           |  1 +
 .../tools/parity/stub/_shared/HealItems.h          |  1 +
 .../tools/parity/stub/_shared/HudHost.h            |  1 +
 .../tools/parity/stub/_shared/HudKit.h             |  1 +
 .../tools/parity/stub/_shared/HudKitTextures.inc   |  1 +
 .../tools/parity/stub/_shared/IniKit.h             |  1 +
 .../tools/parity/stub/_shared/ItemRead.h           |  1 +
 .../tools/parity/stub/_shared/PanelSlots.h         |  1 +
 .../tools/parity/stub/_shared/Party.h              |  1 +
 .../tools/parity/stub/_shared/PortraitSlots.h      |  1 +
 .../tools/parity/stub/_shared/SaveEvents.h         |  1 +
 .../tools/parity/stub/_shared/SceneView.h          |  1 +
 .../tools/parity/stub/_shared/SelfHealUi.h         |  1 +
 .../tools/parity/stub/_shared/StrFmt.h             |  1 +
 .../tools/parity/stub/_shared/TextFit.h            |  1 +
 .../tools/parity/stub/_shared/WindowHook.h         |  1 +
 .../tools/parity/stub/inc/TestHarness.h            |  1 +
 .../companion_levelup/tools/parity/ui_test.cpp     | 17 ++++----
 .../tools/parity/unit_gap_test.cpp                 | 11 +++--
 39 files changed, 142 insertions(+), 41 deletions(-)
```
- **Dirty Files (206):**
  - `M .claude/handoffs/18_mouse_camera_investigation.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M AGENTS.md`
  - ` M patch_manager_mods/10_engine_optimization_research.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/31_combat_action_hotbar.md`
  - ` M patch_manager_mods/33_ui_queue_hotbar_fixes.md`
  - ` M patch_manager_mods/MASTER_KEY.md`

## 2. Immediate Objective (C4)
**Engine addresses -> GameAddr.h (+ proposal)**

Status: TODO
Task: Engine addresses -> GameAddr.h (+ proposal)

## 2b. Upcoming Batches in Pipeline
- **C5** [Common Extraction] : Party.h adoption
- **C6** [Common Extraction] : FileWatch.h adoption
- **C7** [Common Extraction] : KpLog migration (C7a design / C7b code)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume C4` (or `resume PHASE7_COMMON_MASTER`).

## 4. Subagent Delegation Direction
- **Delegate to Subagent**: `python3 scripts/plan_manager.py delegate C4`
- **Role-Based Commands**:
  - Address Cross-ref: `oc-subagent ghidra "Cross-reference LevelUpOverride struct Engine addresses (0x73F450, 0x7E5D80, 0xA1B4D0) against Ghidra DB and GameAddr.h"`
  - Constant Replacement: `oc-subagent code "Replace struct Engine constants with GameAddr.h constants in LevelUpOverride.cpp and draft 19b proposal"`
  - Build/Test Verification: `oc-subagent verify "Run python3 scripts/build_check.py --mod level_up_override_src"`
