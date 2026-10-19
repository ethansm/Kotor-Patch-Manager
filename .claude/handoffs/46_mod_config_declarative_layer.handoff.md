# Session Handoff & Plan State: patch_manager_mods/46_mod_config_declarative_layer.md

## 0. Next-Phase Direction
Execute MCF1: [Mod-config] Pure logic in MenuUiLogic.h: row kinds (toggle/stepper), ScrollWindow (offset clamp, wheel step, ensure-visible, thumb px), `mui::tickResetDue(gap, flags48, lmbDown)` + tests

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/46_mod_config_declarative_layer.md`
- **Waves:** .claude/waves/WI-46-a1/
- **Waves:** .claude/waves/WI-46-a2/
- **Waves:** .claude/waves/WI-46-a3/
- **Branch:** `KOTOR-II` (a1c80c88 mod-config MCF1: pure row model (toggle/stepper), row geometry + hit zones, ScrollWindow, tickResetDue + host tests)
- **Generated:** 2026-10-18 23:04:18 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
a1c80c88 mod-config MCF1: pure row model (toggle/stepper), row geometry + hit zones, ScrollWindow, tickResetDue + host tests
 patch_manager_mods/mod_config_src/MenuUiLogic.h    | 217 ++++++++++++++
 .../mod_config_src/menu_ui_logic_test.cpp          | 329 +++++++++++++++++++++
 2 files changed, 546 insertions(+)
```
- **Dirty Files (173):**
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
  - ... +163 more (run `git status --short`)

## 2. Immediate Objective (MCF1)
**[Mod-config] Pure logic in MenuUiLogic.h: row kinds (toggle/stepper), ScrollWindow (offset clamp, wheel step, ensure-visible, thumb px), `mui::tickResetDue(gap, flags48, lmbDown)` + tests**

Status: BUILT
Task: [Mod-config] Pure logic in MenuUiLogic.h: row kinds (toggle/stepper), ScrollWindow (offset clamp, wheel step, ensure-visible, thumb px), `mui::tickResetDue(gap, flags48, lmbDown)` + tests

## 2b. Upcoming Batches in Pipeline
- **MCF2** [Meta / Planning Setup] : [Mod-config] Provider: AddRow/SetChangeHandler/Get-SetValue slots (final-value handler), `tickResetDue`, stepper arrow art with click-to-cycle fallback, MCR1 logging (arrow path taken, tickResetDue gap/flags48/lmb), contract pins; NO scrolling (MCF2b) in mod_config + enemy_effects_strip (common_contract_test.cpp:233) + holster_editor (positional K2MenuUiApi initializer; Holster session owns that dir)
- **MCF2b** [Meta / Planning Setup] : [Mod-config] Scrolling: ScrollWindow on AddRow entry pages + the mod LIST page (rebind 8 column buttons, kMaxEntries raised), arrows + WheelHook + thumb drag, `scroll_test` dummy entry, MCR1/MCR2 play rows
- **MCF3** [Meta / Planning Setup] : [Mod-config] Declarative layer: `_shared_src/TomlMini.h` + `ModConfigDecl.h` + tests

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/46_mod_config_declarative_layer.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate MCF1` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
