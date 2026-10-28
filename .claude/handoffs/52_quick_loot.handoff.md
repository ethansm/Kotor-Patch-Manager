# Session Handoff & Plan State: patch_manager_mods/52_quick_loot.md

## 0. Next-Phase Direction
Execute **QL2**: Peek on select: list under the name tag after the first click; row click = walk + take one item

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/52_quick_loot.md`
- **Waves:** .claude/waves/w1-52-QL0/
- **Waves:** .claude/waves/w2-52-QL1b/
- **Waves:** .claude/waves/w3-52-R2/
- **Branch:** `KOTOR-II` (798bb008 quick-loot 0.3.1-dev (doc 52 PV1 UX fixes): 1200 px one-line label, '[F] Take all (N)'/'Took (N)' strings, 20-char entries, latched+clamped position, backing panel, 400 ms dwell, give-up server close)
- **Generated:** 2026-10-27 21:15:06 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
798bb008 quick-loot 0.3.1-dev (doc 52 PV1 UX fixes): 1200 px one-line label, '[F] Take all (N)'/'Took (N)' strings, 20-char entries, latched+clamped position, backing panel, 400 ms dwell, give-up server close
 patch_manager_mods/quick_loot_src/QuickLoot.cpp    | 198 ++++++++------
 patch_manager_mods/quick_loot_src/QuickLootLogic.h | 194 ++++++++++++--
 .../quick_loot_src/QuickLootMachine.h              |  95 +++++--
 .../quick_loot_src/common_contract_test.cpp        |   6 +
 patch_manager_mods/quick_loot_src/manifest.toml    |   4 +-
 .../quick_loot_src/quick_loot_logic_test.cpp       | 240 ++++++++++++++---
 .../quick_loot_src/quick_loot_sim_test.cpp         | 295 +++++++++++++++++++--
 scripts/quick_loot_mockup.py                       | 123 ++++++---
 scripts/test_quick_loot_mockup.py                  |  91 +++++--
 9 files changed, 1004 insertions(+), 242 deletions(-)
```
- **Dirty Files (187):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/51_combat_stats_page.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ... +177 more (run `git status --short`)

## 2. Immediate Objective (**QL2**)
**Peek on select: list under the name tag after the first click; row click = walk + take one item**

Status: TODO
Task: Peek on select: list under the name tag after the first click; row click = walk + take one item

## 2b. Upcoming Batches in Pipeline
- ****QL4**** : Per-item take: pick a row from the peek list -> `CSWSCreature::AcquireItem 0x5BDB60` (mod enforces range); replicate CloseInventory's first-pickup notice (item local bool 0x5E, lockertsfinv compare)
- ****QL5**** : Empty / looted container marker (K1 "Marked Empty Containers" has no TSL port): mark on hover/name tag; merge with QL3 and doc 12
- ****QL6**** : Give / quest hand-in containers: identify them (container_p "Give" view, ctrl 0x29, gui+0x70 bit0) from live logs and always route them to the vanilla menu

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/52_quick_loot.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **QL2**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_

## 5. Dead Ends & Verification (manual)
- `lockertsfinv` is NOT a stash container: per-item first-pickup notice inside CloseInventory (005f5770.c:85-130). R2/R3 guessed wrong.
- `AcquireItemsFromObject 0x5F5160` is SpawnBodyBag's corpse copy, not the GUI take path.
- Hooking the call site 0x82AD21 (E8 rel32) is avoided: KPM rel32 relocation unproven. Hook ShowContainerGui entry 0x7CACB0 (9 B, no rel branches) with consumed_exit_address 0x7CAD1F instead.
- Evidence wave: .claude/waves/w1-52-QL0/ (u01 scaffold, u02 early-exit, u03 item readers), check PASS.
