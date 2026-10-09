# Phase 7 Chunk C4 Evidence: Engine Addresses to GameAddr.h Migration & Proposals

**Date:** 2026-10-08  
**Scope:** `patch_manager_mods/level_up_override_src/LevelUpOverride.cpp`, `patch_manager_mods/level_up_override_src/common_contract_test.cpp`, `patch_manager_mods/19b_cross_patch_sharing_survey.md`  
**Status:** COMPLETE (Behaviour-Neutral)

---

## 1. Summary of Changes

1. **Adoption of `GameAddr.h` in `LevelUpOverride.cpp`**:
   - Added `#include "GameAddr.h"` in standard includes.
   - Migrated 7 local engine addresses in `struct Engine` and `g_eng` to canonical `gameaddr::` constants:
     - `gameaddr::PtrListAdd` (`0x0083EA60`) — `CExoArrayList<void*>::Add`, push helper.
     - `gameaddr::AppGlobal` (`0x00A1B4A4`) — Global application pointer.
     - `gameaddr::MemberServerCreature` (`0x0077D800`) — Client creature member -> server creature pointer.
     - `gameaddr::PartyGetAt` (`0x007E5DA0`) — Party table member accessor.
     - `gameaddr::ClientPartyTable` (`0x0073FB90`) — Party table accessor from client app.
     - `gameaddr::ExoCtor` (`0x00733570`) — `CExoString` constructor.
     - `gameaddr::ExoDtor` (`0x00733780`) — `CExoString` destructor.
   - Cleans all 7 migration warnings in `python3 scripts/check_raw_addresses.py`.

2. **Expanded Contract Pinning in `common_contract_test.cpp`**:
   - Included `"GameAddr.h"` and added 7 LEQ assertions verifying that the pinned `gameaddr::` addresses match the engine literals Level-Up Override relies upon.

3. **Semantics Resolution: `0x73F450` vs `0x7E5D80`**:
   - **`0x73F450`** (`CClientExoApp::GetPlayerCreature`): `thiscall(clientApp)`. Returns `CGameObject*`/`CSWCObject*` representing the controlled player object, where offset `+4` is the controlled object ID. Used in `CSWGuiInGame::UpdatePortraits` (0x74F423) and `leaderInstant` to detect whether the portrait being refreshed is the active leader.
   - **`0x7E5D80`** (`GetControlledCreature`): `thiscall(partyTable)`. Returns `CSWCCreature*` directly from the party table.
   - **Verdict**: These functions take different `this` pointers (`CClientExoApp` vs party table) and have distinct return types. `LevelUpOverride.cpp` correctly keeps `0x73F450` for portrait detour compatibility.

4. **Additions Proposed for `GameAddr.h` in doc 19b**:
   - Drafted proposal notes in `patch_manager_mods/19b_cross_patch_sharing_survey.md` covering `RulesGlobal` (`0x00A1B4D0`), `GetPlayerCreature` (`0x0073F450`), `GetCharacterChangeInProgress` (`0x00740FC0`), `GetClientOptions` (`0x0072FB00`), `CanLevelUp` (`0x006B9790`), `AutoLevelup` (`0x006BEDF0`), `AppendToMsgBuffer` (`0x007BE090`), `CSWGlobalVariableTable` methods (`0x00654xxx`), and `CServerExoApp::Initialize` (`0x0051C450`).

---

## 2. Verification Matrix

| Gate | Command | Result |
|---|---|---|
| Mod MinGW Build Check | `python3 scripts/build_check.py --mod level_up_override_src` | **PASS** (32 exports, 32 detours, 0 errors) |
| Common Contract Test | `python3 scripts/run_tests.py patch_manager_mods/level_up_override_src/common_contract_test.cpp` | **PASS** (1/1 passed in 0.8s) |
| Raw Address Checker | `python3 scripts/check_raw_addresses.py` | **PASS** (0 warnings/candidates for LevelUpOverride.cpp) |
| Parity Test Suite | `python3 research/companion_levelup/tools/parity/parity.py` | **PASS** (0 mismatches across 18k cases, 299k pause cases) |
| Parity Scenarios | `python3 research/companion_levelup/tools/parity/scenarios.py [--m32]` | **PASS** (22 scenarios run, 0 failed) |
| Options & Wizard Parity | `python3 research/companion_levelup/tools/parity/options_parity.py` / `wizard_parity.py` | **PASS** (0 mismatches) |
| GuiKit Host Test | `python3 research/companion_levelup/tools/parity/guikit_test.py` | **PASS** (49 checks, 0 failed) |
| Path Verification | `../../.venv/bin/python3 check_paths.py paths/*.toml paths/presets/*.toml` | **PASS** (all presets valid) |
| Mutation Killing | `python3 research/companion_levelup/tools/parity/mutate.py --jobs 6` | **PASS** (194/194 killed, 100.0%) |
| Full LUO Scripts Verification | `test_luo_log_check.py`, `test_luo_scenario.py`, `test_kotor_save_forge.py` | **PASS** (all passed) |
| Shared Headers Integrity | `python3 scripts/check_shared_headers.py` | **PASS** (33 shared headers, 0 drifted) |
| Scoped Test Suites | `python3 scripts/run_tests.py -k levelup` | **PASS** (5 passed in 29.2s) |
