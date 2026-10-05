# Session Handoff & Plan State: patch_manager_mods/29_paused_command_queue.md

## 0. Next-Phase Direction
could we display our own version of the queue with green coloring instead of red for out of combat action queues?

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/29_paused_command_queue.md`
- **Branch:** `KOTOR-II` (28e5c8e doc 29: paused-command delay RE (PQ1/PQ2) + cmdq-probe design)
- **Last Landed Commit:**
```
28e5c8e doc 29: paused-command delay RE (PQ1/PQ2) + cmdq-probe design
 .../29_paused_command_evidence/PQ1.md              | 195 +++++++++++++++++++++
 .../29_paused_command_evidence/PQ2.md              | 191 ++++++++++++++++++++
 patch_manager_mods/29_paused_command_queue.md      |  88 ++++++++++
 3 files changed, 474 insertions(+)
```
- **Dirty Files (116):**
  - `M .claude/skills/plan-handoff/SKILL.md`
  - ` M patch_manager_mods/README.md`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/atlas_coverage.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/cg.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/export_dump_inputs.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/field_census.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/helpers.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_cg.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_export_and_coverage.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_field_census.cpython-312.pyc`

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## Dead Ends & Anti-Patterns (Do NOT Attempt)
- Do NOT hook 0x5D6A60 (RunActions result): companion-ai RAR probe owns it.
- Do NOT hook the relative CALLs 0x565436 / 0x5649ED / 0x6D9584 / 0x571069; use the preceding PUSH/MOV windows (PQ1 f).
- Do NOT hook HUD Update entry 0x74B3F0: action_hotbar owns it (H8 0x74C168 is a separate, checked window).
- The 0x3F placeholder is never in the server mirror UpdateActionQueue 0x588ED0; read the queue +0x100 and round list +0xB00 directly.

## Verification Commands
- Hook windows: python3 patch_manager_mods/engine_ai_atlas/tools/check_atlas_windows.py
- Logs: python3 scripts/kotor2_logs.py status
