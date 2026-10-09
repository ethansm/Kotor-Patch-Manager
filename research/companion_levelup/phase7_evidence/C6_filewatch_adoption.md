# Phase 7 Chunk C6 Evidence: FileWatch.h Adoption

**Date:** 2026-10-08  
**Scope:** `patch_manager_mods/level_up_override_src/LevelUpOverride.cpp`, `patch_manager_mods/level_up_override_src/LevelUpUi.h`, `patch_manager_mods/level_up_override_src/common_contract_test.cpp`, `research/companion_levelup/tools/parity/ui_test.cpp`, `research/companion_levelup/tools/parity/unit_gap_test.cpp`  
**Status:** COMPLETE (Behaviour-Neutral)

---

## 1. Summary of Changes

1. **Adoption of `FileWatch.h` in `LevelUpOverride.cpp`**:
   - Added `#include "FileWatch.h"` to standard includes.
   - Replaced custom `fileStamp()` and `reloadDue()` routines with canonical `filewatch::Stamp`, `filewatch::ReloadPoll`, and `filewatch::changed()`.
   - Converted config polling (`reloadConfig`) to `filewatch::changed(g_cfgPoll, "levelup_override.ini", GetTickCount(), force, &st)`.
   - Converted table polling (`reloadTable`) to `filewatch::changed(g_tablePoll, g_cfg.table, GetTickCount(), force, &st)` with `filewatch::Stamp g_tableStamp`.
   - Converted playthrough polling (`reloadPlay`) to `filewatch::ReloadPoll g_ptPoll` and `filewatch::Stamp g_ptStamp`, retaining forced reload bypass for post-write synchronization.
   - Converted `r10TagsReady()` to `const filewatch::Stamp st = filewatch::stamp(path)`.
   - Maintained `ptWriteText` using `MoveFileExA(..., MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)` to guarantee immediate physical persistence on character level-up save operations; documented additive `writeThrough` parameter recommendation for `filewatch::atomicWrite` (Chunk C9 proposal).

2. **Structural Stamp Alignment in `LevelUpUi.h`**:
   - Updated `UiPanel::pageTable` from `uint64_t` to `filewatch::Stamp` for type-safe table revision tracking (`r->pageTable != g_tableStamp`).

3. **Harness & Contract Pinning**:
   - Expanded `common_contract_test.cpp` with contract verification for `filewatch::stamp`, `filewatch::changed`, `filewatch::accept`, and `filewatch::Stamp` operator contracts.
   - Aligned `ui_test.cpp` (`g_tableStamp.mtime ^= 1`) and `unit_gap_test.cpp` (`g_tablePoll = {}; g_tableStamp = {};`) with canonical `filewatch::Stamp`/`ReloadPoll` state structures.

---

## 2. Verification Matrix

| Gate | Command | Result |
|---|---|---|
| Mod MinGW Build Check | `python3 scripts/build_check.py --mod level_up_override_src` | **PASS** (32 exports, 32 detours, 0 errors) |
| Common Contract Test | `python3 scripts/run_tests.py patch_manager_mods/level_up_override_src/common_contract_test.cpp` | **PASS** (1/1 passed in 1.1s) |
| Parity Test Suite | `python3 research/companion_levelup/tools/parity/parity.py` | **PASS** (0 mismatches across 18.4k cases, 299.6k pause cases, 0 failed) |
| Parity Scenarios | `python3 research/companion_levelup/tools/parity/scenarios.py [--m32]` | **PASS** (22 scenarios run, 0 failed) |
| Options & Wizard Parity | `python3 research/companion_levelup/tools/parity/options_parity.py` / `wizard_parity.py` | **PASS** (0 mismatches) |
| GuiKit Host Test | `python3 research/companion_levelup/tools/parity/guikit_test.py` | **PASS** (49 checks, 0 failed) |
| Path Verification | `../../.venv/bin/python3 check_paths.py paths/*.toml paths/presets/*.toml` | **PASS** (all presets valid) |
| Mutation Killing | `python3 research/companion_levelup/tools/parity/mutate.py --jobs 6` | **PASS** (194/194 killed, 100.0%) |
| Full LUO Scripts Verification | `test_luo_log_check.py`, `test_luo_scenario.py`, `test_kotor_save_forge.py` | **PASS** (all passed) |
| Scoped Test Suites | `python3 scripts/run_tests.py -k levelup` | **PASS** (5 passed in 29.0s) |
