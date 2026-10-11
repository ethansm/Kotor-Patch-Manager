# Session Handoff & Plan State: patch_manager_mods/34_npc_diversity_dispatcher.md

## 0. Next-Phase Direction
Continue 34_npc_diversity_dispatcher.md

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/34_npc_diversity_dispatcher.md`
- **Branch:** `KOTOR-II` (b5b5aba docs: centralize role-based subagent catalog in SUBAGENT_ROLES.md and link across active plans)
- **Last Landed Commit:**
```
b5b5aba docs: centralize role-based subagent catalog in SUBAGENT_ROLES.md and link across active plans
 AGENTS.md                                          |  8 ++-
 patch_manager_mods/24_master_plan_status.md        |  1 +
 patch_manager_mods/31_combat_action_hotbar.md      |  1 +
 patch_manager_mods/MASTER_KEY.md                   |  4 +-
 patch_manager_mods/SUBAGENT_ROLES.md               | 78 ++++++++++++++++++++++
 research/companion_levelup/PHASE7_COMMON_MASTER.md |  1 +
 scripts/scaffold_mod.py                            |  1 +
 7 files changed, 90 insertions(+), 4 deletions(-)
```
- **Dirty Files (226):**
  - `M .claude/handoffs/18_mouse_camera_investigation.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`
  - ` M patch_manager_mods/10_engine_optimization_research.md`
  - ` M patch_manager_mods/18_mouse_camera_investigation.md`
  - ` M patch_manager_mods/33_ui_queue_hotbar_fixes.md`
  - ` M patch_manager_mods/_shared_src/GuiKit.h`
  - ` M patch_manager_mods/action_hotbar_src/ActionHotbar.cpp`

## 3. Strict Context Budget & Read Policy
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume 34_npc_diversity_dispatcher` (or `resume 34_npc_diversity_dispatcher`).
## Play-test result 10-11 (probe 0.1.1-probe, log npc-diversity_log.txt, ~77 s, 64 hook calls)
Checker: RESULT FAIL (banner pre=nosha), WARN tag empty on all 43 RMP, 0 TEX lines. Findings (VERIFIED from log):
- Both hooks FIRE. Remap calls=24, tex calls=40 in the last STAT. oid distinct 27; the same oid repeats with the same `this` ~2.8 s apart (SetAppearance is called repeatedly per creature) and again ~75 s later (new `this`, same oid) => oid is stable per creature.
- descMode14 (+0x14) is **1 for in-world commoners** (rows 610, 596, 594, 627, 452: all skip:dm) and **0 for uniformed rows** (367, 481, 484 officers). The `descMode14 != 0 -> skip` gate in HookDecision.h blocks exactly the target creatures. chg=0, swap=0 because of it.
- Texture names follow the engine rule: `cur=n_commf09` with tv=9, `n_commm06` with tv=6 (engine formats `%s%02d` from descriptor+0x01). Strategy B (write pDesc+0x01 per creature in hook 2) is confirmed viable.
- Tag read via +0x18/+0x1C is empty on every creature (offsets wrong). CRE0 first 64 B (this=0a8366b0): +0x00 vtable 0x0099ee14, +0x04 = 0x8000000a (oid OK), +0x18/+0x1C are pointers 0x0a8363d0/0x08834488 (not a CExoString), floats at +0x20=30.9, +0x24=-15.5, +0x28=10.2 (vec3 position is at +0x20..0x28, not +0x24: my xy were really (y,z)), +0x2C..0x38 quaternion-like.
- Position of the same oid differs between calls (walkers): do NOT hash position; seed from (oid, appearance).
- `KOTOR_VERSION_SHA` env is NOT visible in the game process (sha=unknown => hook address table 0 => pre=nosha). Detect the exe by probing hook bytes at the Steam and GOG addresses instead.
## Dead ends / anti-patterns
- Do NOT gate on descMode14 (commoners are 1). Do NOT seed from xy. Do NOT trust +0x18/+0x1C as tag. Do NOT read KOTOR_VERSION_SHA via getenv in the DLL.
- Hot-swap refuses export changes; a KPatchInit export needs a full reinstall (no kpm_reinstall.py exists). Keep exports {NpcDiversity_Init, OverrideNpcTexture, RemapNpcAppearance} until the final 19f migration.
- Versions are 0.X.Y until the public PR (peer session policy); never bump to 1.x.
- Never run `plan_manager.py handoff` without an explicit plan name: it resolves to the active plan and clobbers another session's file.
- `scripts/test_compat_breakage.py` fails tree-wide (not this mod; unwaived).
## Verification commands
python3 scripts/run_tests.py diversity generate_diversity_2da check_diversity_assets check_hook_sources npc_div_log_check kotor2_logs ; python3 scripts/build_check.py --mod npc_diversity_src ; python3 scripts/kpm_status.py --only npc-diversity ; python3 scripts/kotor2_logs.py check npc-diversity_log.txt --new
