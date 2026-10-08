# Phase 7 Chunk C3 Evidence: Include Style + Parity Stub Alignment

**Date:** 2026-10-08  
**Scope:** `patch_manager_mods/level_up_override_src/LevelUpOverride.cpp`, `research/companion_levelup/tools/parity/`  
**Status:** COMPLETE (Behaviour-Neutral)

---

## 1. Summary of Changes

1. **Standard Include Style in Mod Source (`LevelUpOverride.cpp`)**:
   - Replaced relative parent includes with standard double-quote includes:
     - `#include "../_shared/SafeRead.h"` -> `#include "SafeRead.h"`
     - `#include "../_shared/GuiKit.h"` -> `#include "GuiKit.h"`
     - `#include "../_shared/GuiTip.h"` -> `#include "GuiTip.h"`
     - `#include "../_shared/EmbedTex.h"` -> `#include "EmbedTex.h"`
   - Compiles cleanly in MinGW via standard `-I Common` provided by `scripts/build_check.py` and KPM `create-patch.py`.

2. **Parity Harness Stub Include Path Alignment**:
   - Added `"-I", os.path.join(HERE, "stub", "_shared")` to build commands in:
     - `research/companion_levelup/tools/parity/parity.py`
     - `research/companion_levelup/tools/parity/scenarios.py`
     - `research/companion_levelup/tools/parity/guikit_test.py`
   - Updated `research/companion_levelup/tools/parity/guikit_test.cpp` to include `"GuiKit.h"` directly.

3. **Complete Shared Header Symlink Mirror in Parity Stub**:
   - Symlinked all shared headers from `patch_manager_mods/_shared_src/` into `research/companion_levelup/tools/parity/stub/_shared/` (26 headers + tables).
   - Enables immediate direct resolution for future chunk adoptions (C4 `GameAddr.h`, C5 `Party.h`, C6 `FileWatch.h`).
   - Retains full compatibility with `mutate.py` twin header copying and mutation killing.

---

## 2. Verification Matrix

| Gate | Command | Result |
|---|---|---|
| MinGW Mod Build Check | `python3 scripts/build_check.py --mod level_up_override_src` | **PASS** (32 exports, 32 detours, 0 fails) |
| Level-Up Test Suites | `python3 scripts/run_tests.py -k levelup` | **PASS** (5 passed, 0 failed in 28.5s) |
| Parity Harness (native + -m32) | `python3 research/companion_levelup/tools/parity/parity.py` | **PASS** (0 mismatches across 18k cases, 299k pause cases, 38k apply checks, 0 fails) |
| Scenarios (native + -m32) | `python3 research/companion_levelup/tools/parity/scenarios.py [--m32]` | **PASS** (22 scenarios run, 0 failed) |
| Options Parity | `python3 research/companion_levelup/tools/parity/options_parity.py` | **PASS** (752 lines, 0 mismatches) |
| Wizard Parity | `python3 research/companion_levelup/tools/parity/wizard_parity.py` | **PASS** (262 lines, 0 mismatches) |
| GuiKit Host Test | `python3 research/companion_levelup/tools/parity/guikit_test.py` | **PASS** (49 checks, 0 failed) |
| Mutation Killing | `python3 research/companion_levelup/tools/parity/mutate.py --only LN01 --only TP01` | **PASS** (100% killed by parity.py) |
| Common Contract Tests | `python3 scripts/run_tests.py -k common_contract_test` | **PASS** (24 passed, 0 failed) |
| Shared Headers Integrity | `python3 scripts/check_shared_headers.py` | **PASS** (32 shared headers, 0 drifted) |
| Agent Diff Check | `python3 scripts/agent_diff_check.py --mod level_up_override_src --test levelup` | **PASS** (scope clean, tests passing) |
