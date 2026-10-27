# Session Handoff & Plan State: patch_manager_mods/52_quick_loot.md

## 0. Next-Phase Direction
Execute **QL1**: Fast container 0.1.0-dev: auto take-all via Close(objId,1) after OnOpen settles, "Took: ..." list under the name tag; Shift / give containers -> vanilla menu

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/52_quick_loot.md`
- **Waves:** .claude/waves/w1-52-QL0/
- **Branch:** `KOTOR-II` (daa7ef68 doc 52: QL0 row names the entry hooks actually built)
- **Generated:** 2026-10-27 02:01:01 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
daa7ef68 doc 52: QL0 row names the entry hooks actually built
 .claude/handoffs/52_quick_loot.handoff.md | 42 ++++++++++++++++++-------------
 patch_manager_mods/52_quick_loot.md       |  2 +-
 2 files changed, 26 insertions(+), 18 deletions(-)
```
- **Dirty Files (185):**
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
  - ... +175 more (run `git status --short`)

## 2. Immediate Objective (**QL1**)
**Fast container 0.1.0-dev: auto take-all via Close(objId,1) after OnOpen settles, "Took: ..." list under the name tag; Shift / give containers -> vanilla menu**

Status: TODO
Task: Fast container 0.1.0-dev: auto take-all via Close(objId,1) after OnOpen settles, "Took: ..." list under the name tag; Shift / give containers -> vanilla menu

## 2b. Upcoming Batches in Pipeline
- ****QL2**** : Peek on select: list under the name tag after the first click; row click = walk + take one item
- ****QL3**** : True hover peek: own floating name + list on hover (app+0x4B0), merge with doc 12 empty-container cursor

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/52_quick_loot.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **QL1**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_

## 5. Dead Ends & Verification (manual)
- `lockertsfinv` is NOT a stash container: per-item first-pickup notice inside CloseInventory (005f5770.c:85-130). R2/R3 guessed wrong.
- `AcquireItemsFromObject 0x5F5160` is SpawnBodyBag's corpse copy, not the GUI take path.
- Hooking the call site 0x82AD21 (E8 rel32) is avoided: KPM rel32 relocation unproven. Hook ShowContainerGui entry 0x7CACB0 (9 B, no rel branches) with consumed_exit_address 0x7CAD1F instead.
- Evidence wave: .claude/waves/w1-52-QL0/ (u01 scaffold, u02 early-exit, u03 item readers), check PASS.
