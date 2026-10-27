# Session Handoff & Plan State: patch_manager_mods/52_quick_loot.md

## 0. Next-Phase Direction
Execute **QL0**: Probe DLL `quick-loot` 0.0.1-probe: pass-through detour at 0x0082AD21 that logs objId, flag, resref, item list at Show and again N frames later; logs OnOpen timing; no behaviour change

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/52_quick_loot.md`
- **Waves:** .claude/waves/w1-52-QL0/
- **Branch:** `KOTOR-II` (fc102dfc quick-loot 0.0.1-probe (doc 52 QL0): log-only ShowContainerGui/EventHandler/CloseInventory hooks)
- **Generated:** 2026-10-27 01:54:05 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
fc102dfc quick-loot 0.0.1-probe (doc 52 QL0): log-only ShowContainerGui/EventHandler/CloseInventory hooks
 patch_manager_mods/52_quick_loot.md                |   2 +-
 patch_manager_mods/PLAY_TEST_QUEUE.md              |   1 +
 patch_manager_mods/quick_loot_src/QuickLoot.cpp    | 140 +++++++++++++++
 patch_manager_mods/quick_loot_src/QuickLootLogic.h |  67 ++++++++
 .../quick_loot_src/common_contract_test.cpp        | 188 +++++++++++++++++++++
 patch_manager_mods/quick_loot_src/exports.def      |   6 +
 .../quick_loot_src/kotor2-steam-aspyr.hooks.toml   |  76 +++++++++
 patch_manager_mods/quick_loot_src/manifest.toml    |  14 ++
 .../quick_loot_src/quick_loot_logic_test.cpp       |  77 +++++++++
 patch_manager_mods/quick_loot_src/test.toml        |   6 +
 tests/static/raw_addresses.toml                    |   3 +
 11 files changed, 579 insertions(+), 1 deletion(-)
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

## 2. Immediate Objective (**QL0**)
**Probe DLL `quick-loot` 0.0.1-probe: pass-through detour at 0x0082AD21 that logs objId, flag, resref, item list at Show and again N frames later; logs OnOpen timing; no behaviour change**

Status: PARTIAL
Task: Probe DLL `quick-loot` 0.0.1-probe: pass-through detour at 0x0082AD21 that logs objId, flag, resref, item list at Show and again N frames later; logs OnOpen timing; no behaviour change

## 2b. Upcoming Batches in Pipeline
- ****QL1**** : Fast container 0.1.0-dev: auto take-all via Close(objId,1) after OnOpen settles, "Took: ..." list under the name tag; Shift / give containers -> vanilla menu
- ****QL2**** : Peek on select: list under the name tag after the first click; row click = walk + take one item
- ****QL3**** : True hover peek: own floating name + list on hover (app+0x4B0), merge with doc 12 empty-container cursor

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/52_quick_loot.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **QL0**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_

## 5. Dead Ends & Verification (manual)
- `lockertsfinv` is NOT a stash container: per-item first-pickup notice inside CloseInventory (005f5770.c:85-130). R2/R3 guessed wrong.
- `AcquireItemsFromObject 0x5F5160` is SpawnBodyBag's corpse copy, not the GUI take path.
- Hooking the call site 0x82AD21 (E8 rel32) is avoided: KPM rel32 relocation unproven. Hook ShowContainerGui entry 0x7CACB0 (9 B, no rel branches) with consumed_exit_address 0x7CAD1F instead.
- Evidence wave: .claude/waves/w1-52-QL0/ (u01 scaffold, u02 early-exit, u03 item readers), check PASS.
