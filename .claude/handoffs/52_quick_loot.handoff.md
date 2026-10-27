# Session Handoff & Plan State: patch_manager_mods/52_quick_loot.md

## 0. Next-Phase Direction
Execute **QL2p**: Proximity peek 0.3.0-dev (user choice 10-27): unlocked, untrapped container within OpenM=3 m auto-opens (OpenInventory), list under its name, click/key = take all; hides + Close(objId,0) past CloseM=5 m, on combat/dialog/area change/menu, or after take; nearest only; no re-open until you leave CloseM; ini knobs

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/52_quick_loot.md`
- **Waves:** .claude/waves/w1-52-QL0/
- **Waves:** .claude/waves/w2-52-QL1b/
- **Waves:** .claude/waves/w3-52-R2/
- **Branch:** `KOTOR-II` (aa8f0ece quick-loot 0.3.0-dev (doc 52 QL2p): proximity peek (auto-open unlocked containers within 3 m, F = take all, close past 5 m); empty-at-show containers now taken too)
- **Generated:** 2026-10-27 19:30:57 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
aa8f0ece quick-loot 0.3.0-dev (doc 52 QL2p): proximity peek (auto-open unlocked containers within 3 m, F = take all, close past 5 m); empty-at-show containers now taken too
 patch_manager_mods/52_quick_loot.md                |   2 +
 patch_manager_mods/quick_loot_src/QuickLoot.cpp    | 426 +++++++++++++++++++--
 patch_manager_mods/quick_loot_src/QuickLootLogic.h | 280 +++++++++++++-
 patch_manager_mods/quick_loot_src/manifest.toml    |   4 +-
 .../quick_loot_src/quick_loot_logic_test.cpp       | 298 ++++++++++++--
 tests/static/raw_addresses.toml                    |   2 +-
 6 files changed, 935 insertions(+), 77 deletions(-)
```
- **Dirty Files (187):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/51_combat_stats_page.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M .claude/settings.json`
  - ... +177 more (run `git status --short`)

## 2. Immediate Objective (**QL2p**)
**Proximity peek 0.3.0-dev (user choice 10-27): unlocked, untrapped container within OpenM=3 m auto-opens (OpenInventory), list under its name, click/key = take all; hides + Close(objId,0) past CloseM=5 m, on combat/dialog/area change/menu, or after take; nearest only; no re-open until you leave CloseM; ini knobs**

Status: PARTIAL
Task: Proximity peek 0.3.0-dev (user choice 10-27): unlocked, untrapped container within OpenM=3 m auto-opens (OpenInventory), list under its name, click/key = take all; hides + Close(objId,0) past CloseM=5 m, on combat/dialog/area change/menu, or after take; nearest only; no re-open until you leave CloseM; ini knobs

## 2b. Upcoming Batches in Pipeline
- ****QL2**** : Peek on select: list under the name tag after the first click; row click = walk + take one item
- ****QL3**** : True hover peek: own floating name + list on hover (app+0x4B0), merge with doc 12 empty-container cursor

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/52_quick_loot.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **QL2p**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_

## 5. Dead Ends & Verification (manual)
- `lockertsfinv` is NOT a stash container: per-item first-pickup notice inside CloseInventory (005f5770.c:85-130). R2/R3 guessed wrong.
- `AcquireItemsFromObject 0x5F5160` is SpawnBodyBag's corpse copy, not the GUI take path.
- Hooking the call site 0x82AD21 (E8 rel32) is avoided: KPM rel32 relocation unproven. Hook ShowContainerGui entry 0x7CACB0 (9 B, no rel branches) with consumed_exit_address 0x7CAD1F instead.
- Evidence wave: .claude/waves/w1-52-QL0/ (u01 scaffold, u02 early-exit, u03 item readers), check PASS.
