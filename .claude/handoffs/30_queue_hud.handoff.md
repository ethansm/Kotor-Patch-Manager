# Session Handoff & Plan State: patch_manager_mods/30_queue_hud.md

## 0. Next-Phase Direction
build strips and do L3

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/30_queue_hud.md`
- **Branch:** `KOTOR-II` (6a6091e doc 30: queue-hud 0.1.0-dev INSTALLED 10-04 22:44 (dll 75defebf))
- **Last Landed Commit:**
```
6a6091e doc 30: queue-hud 0.1.0-dev INSTALLED 10-04 22:44 (dll 75defebf)
 patch_manager_mods/30_queue_hud.md | 6 ++++--
 1 file changed, 4 insertions(+), 2 deletions(-)
```
- **Dirty Files (118):**
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

## Dead Ends & Anti-Patterns (Do NOT Attempt) — added 10-04 22:50
- queue-hud 0.1.0-dev is LIVE (dll 75defebf) but NOT play-tested: do not install 0.2.0 before the user's doc 30 checklist passes.
- L3 save-events library must NOT be installed while action-hotbar 1.2.1 is live: it uses the same 5 addresses (0x534527, 0x53C5AA, 0x52FD1E, 0x8529E0, 0x8FACA0); KPM rejects identical addresses. Hotbar migration is owner-gated (hotbar session).
- Doc 24 L3 row names perf-lab EVT entries (0x52F610/0x52FB80/0x53AF20/0x7A06B0/0x7D0F50): those are perf-lab's measurement hooks, not the library's sites. Do not hook them.
- check_raw_addresses: hand-edit own entries only; never --write-baseline.
- Never `git commit` without `-- <paths>`.

## Verification Commands
- `python3 scripts/run_tests.py -k queue_hud` ; `python3 scripts/run_tests.py --wine -k queuehud`
- `bash scripts/pre_install_gate.sh --mod queue_hud_src` ; `--mod save_events_src`
- `python3 scripts/check_raw_addresses.py` ; `python3 scripts/check_hook_spans.py --help`
