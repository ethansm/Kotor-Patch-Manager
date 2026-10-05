# Session Handoff & Plan State: patch_manager_mods/engine_bug_research/PHASE4_PLAN.md

## 0. Next-Phase Direction
game session is done, logs need to be analyzed using kotor2-log-analysis skill

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/engine_bug_research/PHASE4_PLAN.md`
- **Branch:** `KOTOR-II` (4a92cc1 docs: level-up UX polish (pause -/+, tooltips) - README, doc 08 build ac531c4c, doc 19b GuiTip.h row)
- **Last Landed Commit:**
```
4a92cc1 docs: level-up UX polish (pause -/+, tooltips) - README, doc 08 build ac531c4c, doc 19b GuiTip.h row
 patch_manager_mods/08_level_up_override.md         | 12 ++++++++
 .../19b_cross_patch_sharing_survey.md              |  1 +
 patch_manager_mods/level_up_override_src/README.md | 36 ++++++++++++++--------
 3 files changed, 36 insertions(+), 13 deletions(-)
```
- **Dirty Files (113):**
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

## 2. Immediate Objective (Step 0)
**persist this plan**


## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## Dead Ends & Anti-Patterns (Do NOT Attempt) — engine-bug Phase 4 session, 10-03/04
- Do NOT recreate KPM `Patches/K2DialogueLetterboxFixProbe/` (duplicate id -> validate-patches red; doc 20d F1). Probe kpatches live in `KPM/letterbox_probe_stage/` (1.0.2-probe: `letterbox_probe_stage/1.0.2-probe/`).
- Do NOT read game-dir logs with cat/tail/grep/Read: use `python3 scripts/kotor2_logs.py status|check <log>` (kotor2-log-analysis skill).
- The letterbox log is now `k2-dialogue-letterbox-fix_log.txt` (KpLog via kplog.ini, L1 1.0.2 live DLL 7b3936bc); `dialogue_letterbox_fix_log.txt` is the OLD format (stale).
- Smoke-harness logs are synthetic (fake structs); judge the analyzer on real game logs only.
- After any sibling KPM install, check `$G/KotorPatcher.dll` md5 == 36f10b7f (it was found reverted to 2a706ac9 at 16:44 on 10-03).

## Verification Commands
- `python3 scripts/kotor2_logs.py status`
- `python3 scripts/kotor2_logs.py check k2-dialogue-letterbox-fix_log.txt`
- `python3 patch_manager_mods/engine_bug_research/tools/test_analyze_engfix_logs.py`
- `md5sum "$HOME/.local/share/Steam/steamapps/common/Knights of the Old Republic II/KotorPatcher.dll"` (expect 36f10b7f...)
