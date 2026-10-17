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
- Verify commands: `python3 scripts/query_ghidra.py symbol <name>`; `python3 scripts/kotor2_logs.py check enemy-effects-strip_log.txt --new` (the log is `enemy-effects-strip_log.txt`, KpLog name wins over the fallback name).

### Session notes (10-17): probe LIVE, first play results
- Probe 0.1.0-probe INSTALLED by hand (no KPatchLauncher: hook-free, config entry id+dll only, like combat-hotbar). Repo 8b5c416, KPM fac9a86, DLL md5 88212797, backup `$G/backups/enemystrip_full_20261012_233154`. Rollback = restore patch_config.toml/kplog.ini/kpm_install_state.json from it + delete patches/enemy-effects-strip.dll.
- Play result (two sessions, 8-16 target changes each): menu offsets WORK (hp max/cur at +0x1AA0/+0x1AA4 sane, bar rect +1A44=0 +1A48=35 +1A4C=533 +1A50=16). Effects read `none` for EVERY enemy, though the user Fear-cowered an enemy (visible). Leader stats/skills read junk.
- Diagnosis (UNCONFIRMED): ctx.target/ctx.leader are CLIENT objects; creature+0x148 effect list and +0x1198 stats are SERVER-object fields (Effects.h header says CSWSObject). Also the probe logs only on target CHANGE, so an effect applied to an already-targeted enemy is never seen. Engine's own client-side effect-icon cache is at client creature+0xFFC (FUN_00744f00, doc 02).
- Fear = spells.2da row 16 (6 s cower), Horror row 30 (12 s AoE), Insanity row 31. Test source with no powers: grenades via `giveitem g_w_stungren01 5` (also g_w_adhsvgren001, g_w_cryobgren001, g_w_sonicgren01, g_w_poisngren01, g_w_iongren01, g_w_firegren001; resrefs from TSL RCM k_inc_treas_k2.nss, unverified in game). `runscript` console cmd likely a no-op on Steam (FALLBACK.md).
- NO UX exists yet: only the logging probe. User design wish (10-17): REUSE the exact icons/tiles of the character-screen effects pane and the portrait Buff HUD for consistency; row above the enemy name/vitality bar, minimalist (icons only, optional grey alpha backing; no bar if empty). Research wave R1-R6 DONE 10-17: see doc 39 "Research wave 10-17" + `.claude/research/enemy_strip/`.
- NEXT: P0b probe fix (client->server map via ClientToServerObjectId, relog on effect-list signature change, log +0xFFC cache), then P1 logic/tests, P2 draw.
