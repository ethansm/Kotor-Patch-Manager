# Session Handoff & Plan State: patch_manager_mods/39_enemy_effects_strip.md

## 0. Next-Phase Direction
start p0: static RE of where the target name/vitality bar is drawn, then the logging probe

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/39_enemy_effects_strip.md`
- **Branch:** `KOTOR-II` (6c0e0f9 docs: record user decisions on texture overlay plan)
- **Generated:** 2026-10-12 23:10:42 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
6c0e0f9 docs: record user decisions on texture overlay plan
 .claude/handoffs/texture_diversity_research.handoff.md |  2 +-
 .claude/research/texture_diversity/PLAN.md             | 12 ++++++------
 2 files changed, 7 insertions(+), 7 deletions(-)
```
- **Dirty Files (119):**
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/27_companion_selfheal.handoff.md`
  - ` M .claude/handoffs/33_ui_queue_hotbar_fixes.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/38_k1_to_k2_asset_import.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/34_npc_diversity_dispatcher.md`
  - ` M patch_manager_mods/38_k1_to_k2_asset_import.md`
  - ... +109 more (run `git status --short`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/39_enemy_effects_strip.md`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_

### Session notes (10-12)
- The hook at 0x00743850 (doc 31a) is the ACTION MENU (radial popup), not the name/vitality bar. P0 must locate the real bar code (U1).
- Reusable, verified in tree: `_shared_src/Effects.h` `readEffectList`, `effectRatio`; `DebuffIconTable.inc`/`EffectIconTable.inc`; `Gui2D.h` `drawVFill`.
- Buff HUD 0.2.2 (icon gaps 141-143/201/269) is LIVE but has NO play check yet; `kpm_hotswap.py` deny rule was removed from `.claude/settings.json` by user choice.
- Verify commands: `python3 scripts/query_ghidra.py symbol <name>`; `python3 scripts/kotor2_logs.py check enemystrip_log.txt --new`.
