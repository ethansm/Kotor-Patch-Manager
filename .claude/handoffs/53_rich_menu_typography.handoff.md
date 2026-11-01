# Session Handoff & Plan State: patch_manager_mods/53_rich_menu_typography.md

## 0. Next-Phase Direction
Execute **RMT1**: Engine inline multi-color hook in `CAurGUIStringInternal::Draw_2` (`0x0047C06F`, 6-byte call-site detour) with per-line span cache and INI gate `InlineColoring=1` + contract test `rich_menu_typography_contract_test.cpp`

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/53_rich_menu_typography.md`
- **Branch:** `KOTOR-II` (27739f97 doc 53 (RMT0): add shared ItemProps.h, RichTextClassifier.h, and unit tests)
- **Generated:** 2026-11-01 12:41:19 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
27739f97 doc 53 (RMT0): add shared ItemProps.h, RichTextClassifier.h, and unit tests
 patch_manager_mods/_shared_src/ItemProps.h         | 537 +++++++++++++++++++++
 .../_shared_src/RichTextClassifier.h               | 438 +++++++++++++++++
 .../tests/rich_text_classifier_test.cpp            | 285 +++++++++++
 .../rich_menu_typography_src/tests/test.toml       |   4 +
 4 files changed, 1264 insertions(+)
```
- **Dirty Files (198):**
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
  - ... +188 more (run `git status --short`)

## 2. Immediate Objective (**RMT1**)
**Engine inline multi-color hook in `CAurGUIStringInternal::Draw_2` (`0x0047C06F`, 6-byte call-site detour) with per-line span cache and INI gate `InlineColoring=1` + contract test `rich_menu_typography_contract_test.cpp`**

Status: TODO
Task: Engine inline multi-color hook in `CAurGUIStringInternal::Draw_2` (`0x0047C06F`, 6-byte call-site detour) with per-line span cache and INI gate `InlineColoring=1` + contract test `rich_menu_typography_contract_test.cpp`

## 2b. Upcoming Batches in Pipeline
- ****RMT2**** : Central multi-font description builder (`RichDescBox.h`) hooking `CSWGuiListBox::AddControls_2` (`0x0041FD90`, 6B `55 8B EC 83 EC 14`) with `bAutoDelete=0, bVariableItemHeights=1` across all 28 description listboxes (Inventory, Equip, Store, Upgrade, Crafting, Abilities, Journal, Options)
- ****RMT3**** : Static menu header & stat label hierarchy pass (`CSWGuiInGameCharacter::SetStats` `0x0084E6F0` and panel title/caption styling across primary menus)
- ****RMT4**** : Offline pixel mockup & frame-by-frame simulation (`scripts/rich_menu_mockup.py`) rendering Inventory, Abilities, Journal, and Character panes with `FontWidths.inc` metrics

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/53_rich_menu_typography.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **RMT1**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
