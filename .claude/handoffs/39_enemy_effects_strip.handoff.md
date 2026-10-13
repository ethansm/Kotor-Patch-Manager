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
- U1 RESOLVED (static RE): name + vitality bar are controls inside CSWGuiTargetActionMenu (menu = hud+0xCC); use hud-host K2HUD_TARGET_MENU_POST (ctx.menu/target/leader). Offsets in doc 39 U1. No new hook.
- P0 probe BUILT: `patch_manager_mods/enemy_effects_strip_src/` 0.1.0-probe (build_check PASS, contract test 8/8). NOT installed: new mod id => full KPM install, hud-host listed before it, needs user GO with game closed. Then target an enemy with and without an effect; `python3 scripts/kotor2_logs.py check enemystrip_log.txt --new`.
- U2 partial: client creature +0x1198 = stats (K2Addr Creature_Stats); skill ranks int8 at stats+0x174 (server decomp 0x006b7bb0), Awareness idx 3 UNPROVEN; probe logs all 8 raw ranks to confirm against the character sheet.
- Reusable, verified in tree: `_shared_src/Effects.h` `readEffectList`, `effectRatio`; `DebuffIconTable.inc`/`EffectIconTable.inc`; `Gui2D.h` `drawVFill`.
- Buff HUD 0.2.2 (icon gaps 141-143/201/269) is LIVE but has NO play check yet; `kpm_hotswap.py` deny rule was removed from `.claude/settings.json` by user choice.
- Verify commands: `python3 scripts/query_ghidra.py symbol <name>`; `python3 scripts/kotor2_logs.py check enemystrip_log.txt --new`.
