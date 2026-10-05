# Session Handoff & Plan State: patch_manager_mods/24_master_plan_status.md

## 0. Next-Phase Direction
Test direction for phase X

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/24_master_plan_status.md`
- **Branch:** `KOTOR-II` (85b2dbd doc 24 L3 DONE (built, not installed; site list corrected; hotbar migration owner-gated))
- **Last Landed Commit:**
```
85b2dbd doc 24 L3 DONE (built, not installed; site list corrected; hotbar migration owner-gated)
 patch_manager_mods/24_master_plan_status.md | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```
- **Dirty Files (128):**
  - `M .claude/skills/plan-handoff/SKILL.md`
  - ` M patch_manager_mods/24_master_plan_status.md`
  - ` M patch_manager_mods/README.md`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/atlas_coverage.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/cg.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/export_dump_inputs.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/field_census.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/helpers.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_cg.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_export_and_coverage.cpython-312.pyc`

## 2. Immediate Objective (T5e)
**contract tests char_effects + equip3d**

Status: TODO
Task: contract tests char_effects + equip3d

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.
