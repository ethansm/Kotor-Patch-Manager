# Session Handoff & Plan State: patch_manager_mods/44_in_memory_asset_vfs_and_character_library.md

## 0. Next-Phase Direction (updated 10-18 by session fix-asset-vfs-review-findings)
**State.** Live: asset-vfs 0.1.2-dev (H5 Release hook, afbc163f), juhani-char 0.3.0-dev (Juhani + Carth + Juhani twin `k1p_juhanv` bundles, 1fd5c08 + peer 42793e41), k1-spawn-probe 0.3.5-probe. Game-dir `asset_vfs.ini` is `mode=serve` (set 10-18 ~16:05, backup `asset_vfs.ini.bak_20261018_h5`); kplog.ini has `[asset-vfs]`/`[juhani-char]` sections; `npc_world.ini` spawn_cap=6.
**History.** VFS-T0a (observe) PASSED 10-18 (registration 31/31 + 10/10 assets, A10768=1, OBSERVE 2DA rows, no WARN/ERR). First serve (peer test, 10-18) CRASHED in CExoResMan::Release at the `>>30==1` archive-source release (FUN_00712d70 returns NULL for VFS ids). Fix = hook H5 at 0x007130D3 (skip that block for isVfsPackedId); independent Opus review APPROVE (no other hook needed for UTC/GFF; findings: packed-id prefix 0x40FC0000 maps to key-table id 0x30 so a 48th concurrent RIM table could collide, partial reads of VFS resources return 0, async stall if a record is unregistered after binding). Rollback of the reinstall: `kpm_reinstall.py --rollback backups/asset-vfs_reinstall_20261018_155550`.
**Next.** (1) User plays VFS-CT2 (row in PLAY_TEST_QUEUE): enter 204tel, expect rig Juhani + Carth + Juhani DLL twin; then `kotor2_logs.py check asset-vfs_log.txt --new`, `k1-spawn-probe_log.txt`, `juhani-char_log.txt`. On a crash set `mode=observe` first, then read the log. (2) Then VFS-T0/T1 (rig removal, 0 disk files). (3) Implementation plan for the K1->K2 import pipeline = doc 38 section 26 (S1-S6, gated on CT2 passing); user asked to move to plan mode for it.
**Commits this session.** 1fd5c08 (Carth bundle, k1-spawn-probe 0.3.4, packer --symbol-prefix), 9e352af (docs), afbc163f (H5), b48b6869 (doc 38 sec 26). KPM: 12dcf0f, 08b6e02, and the AssetVfs 0.1.2-dev commit.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/44_in_memory_asset_vfs_and_character_library.md`
- **Waves:** .claude/waves/WI-44-a1/
- **Waves:** .claude/waves/WI-44-b1/
- **Branch:** `KOTOR-II` (9ae4631 feat(enemy-effects-strip): 0.2.0-probe reads the server creature, relogs on effect-list change, dumps icon cache (doc 39 P0b))
- **Generated:** 2026-10-17 02:17:55 in `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`
- **Last Landed Commit:**
```
9ae4631 feat(enemy-effects-strip): 0.2.0-probe reads the server creature, relogs on effect-list change, dumps icon cache (doc 39 P0b)
 .../enemy_effects_strip_src/EnemyStrip.cpp         | 324 +++++++++++++++++----
 .../common_contract_test.cpp                       |  38 ++-
 .../enemy_effects_strip_src/manifest.toml          |   2 +-
 3 files changed, 310 insertions(+), 54 deletions(-)
```
- **Dirty Files (169):**
  - ` M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/34_npc_diversity_dispatcher.handoff.md`
  - ` M .claude/handoffs/40_npc_behaviour_variety.handoff.md`
  - ` M .claude/handoffs/DECOMP_PLAN.handoff.md`
  - ` M .claude/handoffs/PHASE7_COMMON_MASTER.handoff.md`
  - ` M .claude/handoffs/deep_haiku_review_skill.handoff.md`
  - ` M .claude/handoffs/variant_fix_wave_skill.handoff.md`
  - ` M .claude/plans/swarm_library_architecture.md`
  - ` M .claude/reviews/gemini_20261014/tools/fix3_items.tsv`
  - ` M CLAUDE.md`
  - ... +159 more (run `git status --short`)

## 2. Immediate Objective (**VFS-B1**)
**`juhani_char_src` (`juhani-char` 0.1.0-dev): registers the private bundle (31 assets, 107 cells; DLL 9.96 MB)**

Status: BUILT
Task: `juhani_char_src` (`juhani-char` 0.1.0-dev): registers the private bundle (31 assets, 107 cells; DLL 9.96 MB)

## 2b. Upcoming Batches in Pipeline
- ****VFS-T0a**** : Live: asset-vfs alone, `mode=observe`, Override rig in place
- ****VFS-T0**** : Live: Override rig removed, asset-vfs observe + juhani-char
- ****VFS-T1**** : In-game verification of Juhani spawned in Telos `204tel` with 0 disk files (`mode=serve`)

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume patch_manager_mods/44_in_memory_asset_vfs_and_character_library.md`.

## 4. Subagent Delegation Direction
- **Claude Code**: parallel `model: haiku` Agent subagents; brief = `Contract` + `Write Scope` + `python3 scripts/agent_contract.py print worker|recon`.
- **Other backends** (only when the user asks): `python3 scripts/plan_manager.py delegate **VFS-B1**` or `oc-subagent <scout|code|ghidra|verify|gate|doc>`.

## 5. Dead Ends & Verification (manual)
_(none recorded; append Dead Ends & Anti-Patterns and Verification Commands here)_
