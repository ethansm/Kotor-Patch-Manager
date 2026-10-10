# Session Handoff & Plan State: patch_manager_mods/10_engine_optimization_research.md

## 0. Next-Phase Direction
Track P / Perf-Lab 0.6.0 live install completed (8/8 phases complete). Built DLL staged and live patch_config.toml merged with 68 hooks. Ready for gameplay soak telemetry with perflab_debug.txt.

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/10_engine_optimization_research.md`
- **Branch:** `KOTOR-II` (8395275 feat(plan): feature-scoped handoff isolation, auto-resolving mark-done, and plan list)
- **Last Landed Commit:**
```
8395275 feat(plan): feature-scoped handoff isolation, auto-resolving mark-done, and plan list
 .claude/skills/plan-handoff/SKILL.md |   6 +-
 AGENTS.md                            |  12 +--
 scripts/plan_manager.py              | 169 ++++++++++++++++++++++++++++++-----
 scripts/test_plan_manager.py         |  63 +++++++++++++
 4 files changed, 218 insertions(+), 32 deletions(-)
```
- **Dirty Files (163):**
  - `M .claude/handoffs/18_mouse_camera_investigation.handoff.md`
  - ` M .claude/handoffs/24_master_plan_status.handoff.md`
  - ` M .claude/handoffs/31_combat_action_hotbar.handoff.md`
  - ` M .claude/session_handoff.md`
  - ` M patch_manager_mods/10_engine_optimization_research.md`
  - ` M patch_manager_mods/22_compat_check_plan.md`
  - ` M patch_manager_mods/31_combat_action_hotbar.md`
  - ` M patch_manager_mods/MASTER_KEY.md`
  - ` M patch_manager_mods/action_hotbar_src/ActionHotbar.cpp`
  - ` M patch_manager_mods/action_hotbar_src/kotor2-steam-aspyr.hooks.toml`

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.
