# Phase 7 Chunk C5 Evidence: Party.h Adoption

**Date:** 2026-10-08  
**Scope:** `patch_manager_mods/level_up_override_src/LevelUpOverride.cpp`, `patch_manager_mods/level_up_override_src/common_contract_test.cpp`, `research/companion_levelup/tools/parity/gate_test.cpp`, `research/companion_levelup/tools/parity/scenario_test.cpp`  
**Status:** COMPLETE (Behaviour-Neutral)

---

## 1. Summary of Changes

1. **Adoption of `Party.h` in `LevelUpOverride.cpp`**:
   - Added `#include "Party.h"` in standard includes.
   - Refactored `leaderCreature()` from manual 4-link pointer traversal (`appGlobal` -> `client` -> `party` via `tableGet` -> `member 0` via `partyGet` -> `serverCreature` via `getCreature`) to canonical `party::` helpers:
     ```cpp
     uintptr_t leaderCreature() {
         const uintptr_t tbl = party::table();
         const uintptr_t m = tbl ? party::member(tbl, 0) : 0;
         return m ? party::serverCreature(m) : 0;
     }
     ```
   - Retains exact semantics: `party::table()` resolves `*(*(*(appGlobal+4)+4)+0x270)` with saferead probes, `party::member(tbl, 0)` checks `partyCount(tbl) > 0` before querying member 0, and `party::serverCreature(m)` maps client creature to server creature.

2. **Expanded Contract Pinning in `common_contract_test.cpp`**:
   - Added `#include "Party.h"`.
   - Added contract assertions verifying that `party::table`, `party::member`, and `party::serverCreature` function pointers exist, traverse the `appGlobal -> client -> ci -> table` chain, and correctly pass member index 0 and client pointer.

3. **Parity Test Harness Alignment (`gate_test.cpp` & `scenario_test.cpp`)**:
   - Switched test harness hooks to `Party.h`: configured the full arena chain (`[appGlobal] = app`, `[app+4] = client`, `[client+4] = ci`, `[ci+0x270] = partyTable`, `[partyTable] = count`), `party::g_appGlobal = cell`, `party::g_member`, and `party::g_serverCreature`.
   - Updated `gate_test.cpp` line 306 to verify `party::member` received the resolved party table pointer and index 0.

---

## 2. Verification Matrix

| Gate | Command | Result |
|---|---|---|
| Mod MinGW Build Check | `python3 scripts/build_check.py --mod level_up_override_src` | **PASS** (32 exports, 32 detours, 0 errors) |
| Common Contract Test | `python3 scripts/run_tests.py patch_manager_mods/level_up_override_src/common_contract_test.cpp` | **PASS** (1/1 passed in 1.0s) |
| Parity Test Suite | `python3 research/companion_levelup/tools/parity/parity.py` | **PASS** (0 mismatches across 18k cases, 299k pause cases) |
| Parity Scenarios | `python3 research/companion_levelup/tools/parity/scenarios.py [--m32]` | **PASS** (22 scenarios run, 0 failed) |
| Options & Wizard Parity | `python3 research/companion_levelup/tools/parity/options_parity.py` / `wizard_parity.py` | **PASS** (0 mismatches) |
| GuiKit Host Test | `python3 research/companion_levelup/tools/parity/guikit_test.py` | **PASS** (49 checks, 0 failed) |
| Path Verification | `../../.venv/bin/python3 check_paths.py paths/*.toml paths/presets/*.toml` | **PASS** (all presets valid) |
| Mutation Killing | `python3 research/companion_levelup/tools/parity/mutate.py --jobs 6` | **PASS** (194/194 killed, 100.0%) |
| Full LUO Scripts Verification | `test_luo_log_check.py`, `test_luo_scenario.py`, `test_kotor_save_forge.py` | **PASS** (all passed) |
| Shared Headers Integrity | `python3 scripts/check_shared_headers.py` | **PASS** (33 shared headers, 0 drifted) |
| Scoped Test Suites | `python3 scripts/run_tests.py -k levelup` | **PASS** (5 passed in 29.5s) |
