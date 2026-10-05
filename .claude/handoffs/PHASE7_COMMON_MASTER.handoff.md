# Session Handoff & Plan State: research/companion_levelup/PHASE7_COMMON_MASTER.md

## 0. Next-Phase Direction
Execute C1: Integration audit baseline (read-only)

## 1. Active State & Provenance
- **Master Plan:** `research/companion_levelup/PHASE7_COMMON_MASTER.md`
- **Branch:** `KOTOR-II` (5eb7be3 scripts: track plan_manager.py, query_ghidra.py (+test); skills and CLAUDE.md call them)
- **Last Landed Commit:**
```
5eb7be3 scripts: track plan_manager.py, query_ghidra.py (+test); skills and CLAUDE.md call them
 scripts/plan_manager.py      | 426 +++++++++++++++++++++++++++++++++++++++++++
 scripts/query_ghidra.py      | 231 +++++++++++++++++++++++
 scripts/test_query_ghidra.py |  69 +++++++
 3 files changed, 726 insertions(+)
```
- **Dirty Files (111):**
  - `D patch_manager_mods/engine_ai_atlas/tools/__pycache__/atlas_coverage.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/cg.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/export_dump_inputs.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/__pycache__/field_census.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/helpers.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_cg.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_export_and_coverage.cpython-312.pyc`
  - ` D patch_manager_mods/engine_ai_atlas/tools/tests/__pycache__/test_field_census.cpython-312.pyc`
  - ` M patch_manager_mods/ghidra_knowledge/kotor2_ghidra_knowledge.db`
  - ` M patch_manager_mods/steam_db_pr3/README.md`

## 2. Immediate Objective (C1)
**Integration audit baseline (read-only)**

Status: TODO
Task: Integration audit baseline (read-only)

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## Session 2026-10-04 (R10 batch 1) - Dead Ends & Anti-Patterns
- KPM `LevelUpOverride.kpatch` is untracked build output: commit only sources in $T (`git commit -- <paths>` failed on the .kpatch pathspec).
- Background Sonnet agents that start Monitor waits re-notify repeatedly after finishing: TaskStop them once their result is committed.
- Tag table: role-narrowing powers gave hundreds of preset conflicts -> powers carry all 5 roles; only alignment narrows (doc 25 s12 known limit).
- check_exports FAILs on a stale kpatch until the rebuild (`python3 ../create-patch.py` in $T/Patches/LevelUpOverride).
## Verification Commands
- `python3 scripts/run_tests.py` (97/97 at 2026-10-04), `python3 scripts/run_tests.py -k r10`, `cd research/companion_levelup && python3 tools/parity/parity.py && python3 tools/parity/mutate.py --jobs 6` (162/162)
- `.venv/bin/python3 research/companion_levelup/r10/gen_r10_tags.py --check` (run from research/companion_levelup with ../../.venv/bin/python3)
