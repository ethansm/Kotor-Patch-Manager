# Session Handoff & Plan State: patch_manager_mods/31_combat_action_hotbar.md

## 0. Next-Phase Direction
Execute CH5 — Combat Action Hotbar: 31_combat_action_hotbar.md:122 (Auto-fill ranking logic & INI profile serialization in CombatLogic.h) in 31_combat_action_hotbar.md

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/31_combat_action_hotbar.md`
- **Branch:** `KOTOR-II` (9498f50 docs: T5e DONE (doc 24 row, doc 20 line))
- **Last Landed Commit:**
```
9498f50 docs: T5e DONE (doc 24 row, doc 20 line)
 patch_manager_mods/20_TESTING_PLAN.md       | 1 +
 patch_manager_mods/24_master_plan_status.md | 2 +-
 2 files changed, 2 insertions(+), 1 deletion(-)
```
- **Dirty Files (137):**
  - `M .claude/handoffs/19e_phase2_plan.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/25_pc_levelup_filter.handoff.md`
  - ` M .claude/handoffs/30_queue_hud.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M AGENTS.md`
  - ` M CLAUDE.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/README.md`

## 2. Immediate Objective (CH1c)
**Commit + install probe; user targets: enemy creature, door, placeable, mine, droid; log check confirms group 0/1/2 meanings + type values 0..3**

Status: TODO
Task: Commit + install probe; user targets: enemy creature, door, placeable, mine, droid; log check confirms group 0/1/2 meanings + type values 0..3

## 2b. Upcoming Batches in Pipeline
- **HL2** : combat-hotbar becomes consumer: drop its own 0x743540 hook, `requires = ["hud-host"]`, registers TARGET_MENU_PRE; probe output identical to CH1b; install both
- **CH2** [Common Extraction] : Invoke test (inside the hud-host TARGET_MENU_PRE callback): per-frame edge poll (`GetAsyncKeyState` gated on `guikit::winForeground`, not key hook 0x72CB63) Ctrl+Shift+1/2/3 = invoke entry[last] of group g via write-sel + `0x751710` from inside the 0x743540 detour (entries still valid). Log queue result. Add DB rows 0x7445D0 / 0x751710 (Haiku verify + decoys)
- **CH3b** [Common Extraction] : Suppression behind INI `hide_arrows=0/1` (default 0); wheel still works

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## CH4e session close (2026-10-05 11:40)
- CH4e INSTALLED by DLL hot-swap: combat-hotbar 0.7.0-dev f5a02668, src bfc0f8e + 32afa6f, doc db95a9f, KPM 286427f, backup ch4e_swap_20261005_1136. Awaiting play test, checklist v5 (doc 31).
- CH5 facts gathered (combat-hotbar_log 10-05 00:58):
  - Entry key encoding: feat = 0x10000000|featId (Critical Strike 0x08, Flurry 0x0B, Power Attack 0x1C); Attack = 0x3EB; grenade = 0x4000xxxx (0x20B/0x20C/0x20D); mine actions are 0x3F4/0x402.
  - Creature groups come from GetTargetActions 0x77E370 case 4: g0 = FUN_0077dba0 (feats), g1 = 0x77a4f0 (powers), g2 = 0x77c6a0 (grenades).
  - Open: does 0x77dba0 already collapse feat tiers? Is grenade 0x20B a baseitem/row id, and where does the stack count live?
## Dead Ends & Anti-Patterns (Do NOT Attempt)
- Full KPatchLauncher reinstall for DLL-only changes: it clobbers 8 peer hot-swapped DLLs. Use the DLL hot-swap route (kotor2-kpm-install skill).
- Changing the slot stride per = Cols*MaxRows: click, tooltip and slotEntry depend on it.
- Plain `git commit` or `git add -A`: the index is shared.
## Verification Commands
- python3 scripts/run_tests.py -k combat && python3 scripts/build_check.py --mod combat_hotbar_src && python3 scripts/check_hook_spans.py
