# Session Handoff & Plan State: patch_manager_mods/31_combat_action_hotbar.md

## 0. Next-Phase Direction
keep going with next phase of research and development of the combat-hotbar.  We can keep building it and when I get to playing/testing, testing everything thats ready at once

## 1. Active State & Provenance
- **Master Plan:** `patch_manager_mods/31_combat_action_hotbar.md`
- **Branch:** `KOTOR-II` (de142bd doc 25 s15 / doc 08 RESUME / doc 19b row 10: R10 batch 3c built (1.4.0-dev 978b39fd), EmbedTex.h)
- **Last Landed Commit:**
```
de142bd doc 25 s15 / doc 08 RESUME / doc 19b row 10: R10 batch 3c built (1.4.0-dev 978b39fd), EmbedTex.h
 patch_manager_mods/08_level_up_override.md           |  2 ++
 patch_manager_mods/19b_cross_patch_sharing_survey.md |  2 +-
 patch_manager_mods/25_pc_levelup_filter.md           | 17 +++++++++++++++++
 3 files changed, 20 insertions(+), 1 deletion(-)
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

## 2. Immediate Objective (CH1c)
**Commit + install probe; user targets: enemy creature, door, placeable, mine, droid; log check confirms group 0/1/2 meanings + type values 0..3**

Status: TODO
Task: Commit + install probe; user targets: enemy creature, door, placeable, mine, droid; log check confirms group 0/1/2 meanings + type values 0..3

## 3. Strict Context Budget & Read Policy
- **Self-Contained Plan**: Do NOT run broad `ls`, find commands, or `cat` entire source files.
- **No Historical Docs**: Do NOT read past markdown docs (`19*.md`, `15*.md`, etc.). All active state is captured here.
- **Targeted Reads Only**: Use `sed -n 'X,Yp'` or `rg` for specific lines if needed.
- **Fresh Session Bootstrap**: Run `python3 scripts/plan_manager.py resume`.

## Dead Ends & Anti-Patterns (session 2026-10-04 evening)
- run_tests.py default tier: the single FAIL `check-raw-addresses` at `perf_lab_src/harness/smoke_queuehud.cpp:60` belongs to the queue-hud session; do not allowlist it. Use `pre_install_gate.sh --skip-default` when that is the only failure.
- "Text file busy" ERRORs in run_tests = concurrent session building in /tmp/claude-1000/kotor-tests; rerun the item.
- raw_addresses.toml edits: never split on the first `[files.` (the header comment contains one); insert after "never by automation." marker, or use --write-baseline only if no foreign FAILs.
- plan_manager counts any Status containing "DONE" as done; use "PARTIAL (...)" for half-done rows.
- KPM install recipe that worked (10-04 23:17): copy non-test sources to `$K/Patches/<Dir>`, `python3 ../create-patch.py` in that dir, commit on local-mods with explicit paths, `stage_and_verify_install.py stage <newdir> <extra.kpatch...> --allow-dirty`, backup kpm_install_state.json+patch_config.toml+patches/(+kplog.ini), KPatchLauncher NoLaunch with IDS line, `verify`.

## Verification Commands
- `python3 scripts/build_check.py --mod hud_host_src && python3 scripts/build_check.py --mod combat_hotbar_src`
- `python3 scripts/run_tests.py -k hud_host; -k hh_; -k combat`
- `python3 scripts/check_hook_spans.py && python3 scripts/check_library_manifests.py && python3 scripts/check_shared_headers.py`
