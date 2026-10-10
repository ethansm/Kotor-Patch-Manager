# Session Handoff & Plan State: patch_manager_mods/31_combat_action_hotbar.md

## 0. Next-Phase Direction
CH1c — Commit + install probe; user targets: enemy creature, door, placeable, mine, droid; log check confirms group 0/1/2 meanings + type values 0..3

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/31_combat_action_hotbar.md`
- **Branch:** `KOTOR-II` (95945d4 docs(18): record mouse-probe 0.4.0 hot-swap installation)
- **Last Landed Commit:**
```
95945d4 docs(18): record mouse-probe 0.4.0 hot-swap installation
 patch_manager_mods/18_mouse_camera_investigation.md | 15 ++++++++++++++-
 1 file changed, 14 insertions(+), 1 deletion(-)
```
- **Dirty Files (159):**
  - `M .claude/handoffs/18_mouse_camera_investigation.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M patch_manager_mods/22_compat_check_plan.md`
  - ` M patch_manager_mods/31_combat_action_hotbar.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/action_hotbar_src/ActionHotbar.cpp`
  - ` M patch_manager_mods/action_hotbar_src/kotor2-steam-aspyr.hooks.toml`
  - ` M patch_manager_mods/action_hotbar_src/manifest.toml`

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
