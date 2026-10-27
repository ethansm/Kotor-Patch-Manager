# Session Handoff & Plan State: patch_manager_mods/52_quick_loot.md

## 0. Next-Phase Direction
Execute **QL1**: Fast container 0.1.0-dev: auto take-all via Close(objId,1) after OnOpen settles, "Took: ..." list under the name tag; Shift / give containers -> vanilla menu

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/52_quick_loot.md`
- **Waves:** .claude/waves/w1-52-QL0/
- **Waves:** .claude/waves/w2-52-QL1b/
- **Branch:** `KOTOR-II` (94cfdf05 quick-loot 0.2.0-dev (doc 52 QL1b): 'Took: ...' toast under the target name for 3 s)
- **Generated:** 2026-10-27 02:21:16 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
94cfdf05 quick-loot 0.2.0-dev (doc 52 QL1b): 'Took: ...' toast under the target name for 3 s
 patch_manager_mods/quick_loot_src/QuickLoot.cpp    | 204 ++++++++++++++++++++-
 patch_manager_mods/quick_loot_src/QuickLootLogic.h |  98 +++++++++-
 .../quick_loot_src/common_contract_test.cpp        |  53 ++++++
 patch_manager_mods/quick_loot_src/manifest.toml    |   4 +-
 .../quick_loot_src/quick_loot_logic_test.cpp       |  84 +++++++++
 5 files changed, 435 insertions(+), 8 deletions(-)
```
- **Dirty Files (186):**
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
  - ... +176 more (run `git status --short`)

## 2. Immediate Objective (**QL1**)
**Fast container 0.1.0-dev: auto take-all via Close(objId,1) after OnOpen settles, "Took: ..." list under the name tag; Shift / give containers -> vanilla menu**

Status: TODO
Task: Fast container 0.1.0-dev: auto take-all via Close(objId,1) after OnOpen settles, "Took: ..." list under the name tag; Shift / give containers -> vanilla menu

## 2b. Upcoming Batches in Pipeline
- ****QL1a**** : quick-loot 0.1.0-dev: consume Show (flag 1, no Shift, hud-host connected), send Close(objId,1) from K2HUD_FRAME 2 frames after event 0x16 (timeout 1 s); logs only, no toast
- ****QL1b**** : "Took: ..." label under the target name tag (K2HUD_BUILD label, shown ~3 s)
- ****QL2**** : Peek on select: list under the name tag after the first click; row click = walk + take one item

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
