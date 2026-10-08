# Level-Up Override Phase 7 Chunk C1: Integration Audit Baseline

**Date:** 2026-10-08  
**Scope:** Read-only integration audit of `level_up_override_src` against `_shared_src` (Common) and KPM library interfaces.  
**Source Commit Head:** `4369884` (scripts(plan): support plan stems, research plans, and active-plan phase resolution in plan_manager)

---

## 1. Tooling & Checker Baseline Run

All commands executed from workspace root `/home/ethansm/orca/workspaces/local-setup/KOTOR-II`:

| Tool / Check Command | Verdict | Details / Evidence |
|---|---|---|
| `python3 scripts/check_shared_headers.py` | **PASS** | 31 shared headers checked against KPM fork `$T/Patches/Common`; 0 drifted. |
| `python3 scripts/check_hook_bytes.py --quiet` | **PASS** | `level-up-override` detours valid. Repo: `FAIL=0 WARN=7 INFO=25 PASS=242 SKIP=3`. |
| `python3 scripts/check_hook_spans.py --quiet` | **PASS** | `level-up-override` has 0 hook drift. Repo: `FAIL=0 WARN=2 INFO=11 PASS=1 SKIP=0`. |
| `python3 scripts/check_exports.py` | **PASS** | `PASS exports: level-up-override: 32 detour function(s) exported`. |
| `.venv/bin/python scripts/check_gui_ids.py` | **PASS** | `character_p.gui` level-up matches `LevelUpUi.h`, appends runtime controls 151..176 without collision. |
| `python3 scripts/check_raw_addresses.py` | **PASS** | `level-up-override`: 0 illegal address errors. 7 candidate addresses identified for GameAddr migration. |
| `python3 scripts/run_tests.py -k levelup` | **PASS** | 5 suites passed (30s): `wizard-parity`, `options-parity`, `scenarios`, `scenarios-m32`, `parity`. |
| `python3 scripts/run_tests.py -k common_contract` | **PASS** | `patch_manager_mods/level_up_override_src/common_contract_test.cpp` PASS (1.4s). |
| `python3 scripts/compat_check.py --lint` | **PASS** | `level-up-override` has 0 contract GAPs and 0 interface GAPs. |
| `python3 scripts/build_check.py --mod level_up_override_src` | **PASS** | Rebuilt cleanly: 32 exports, 32 detours present, 0 warnings/errors. |

---

## 2. Integration Rules Compliance Matrix

Rules derived from `19b_cross_patch_sharing_survey.md`, `20_TESTING_PLAN.md`, `22_compat_check_plan.md`, and `23_menu_3d_shared_migration.md`:

| Rule ID | Integration Requirement | Status | Evidence & Baseline Notes |
|---|---|---|---|
| **R1** | **One-way sync from `_shared_src`**: Never edit `$T/Patches/Common/*.h` directly; always edit `_shared_src` and sync via `check_shared_headers.py --sync`. | **PASS** | `check_shared_headers.py` verifies 31 headers with 0 drift. |
| **R2** | **Additive & defaulted parameters**: Shared header changes must not break existing sibling consumers. | **PASS** | Level-up override consumes `SafeRead.h`, `GuiKit.h`, `GuiTip.h`, `EmbedTex.h` cleanly. |
| **R3** | **Common Contract Test ownership**: Every mod consuming `_shared_src` must own `<mod>_src/common_contract_test.cpp` verifying API assumptions. | **PASS** | `common_contract_test.cpp` + `test.toml` implemented in chunk C2 (`d89b511`), verifies SafeRead and GuiKit contracts. |
| **R4** | **Standard Include Style**: Mod code should include `"Header.h"` with `-I Common` rather than relative paths `"../_shared/Header.h"`. | **FAIL (Expected)** | `LevelUpOverride.cpp:79-82` currently uses `#include "../_shared/SafeRead.h"` etc. Scheduled for Chunk **C3**. |
| **R5** | **Centralized Engine Addresses (`GameAddr.h`)**: No duplicated global or engine addresses in mod source. | **FAIL (Expected)** | `LevelUpOverride.cpp:1052-1064` defines local `struct Engine` with 7 addresses present in `GameAddr.h`. Scheduled for Chunk **C4**. |
| **R6** | **Shared Party Chain (`Party.h`)**: Use `party::` helpers instead of custom creature lookup loops. | **FAIL (Expected)** | `LevelUpOverride.cpp:2038-2046` implements local `leaderCreature()`. Scheduled for Chunk **C5**. |
| **R7** | **Shared File Watching & Atomic IO (`FileWatch.h`)**: Use `filewatch::Stamp`, `changed()`, and `atomicWrite()`. | **FAIL (Expected)** | `LevelUpOverride.cpp:220-232, 1430-1438` implements local `fileStamp`, `reloadDue`, and `ptWriteText`. Scheduled for Chunk **C6**. |
| **R8** | **Unified Logging (`KpLog.h`)**: Migrate local `logf`/`logLine` to shared logger while preserving exact log format byte-identity. | **FAIL (Expected)** | `LevelUpOverride.cpp:142-171, 1160` implements local file logger. Scheduled for Chunk **C7**. |
| **R9** | **Library DLL Decoupling**: Isolate save event hooks and HUD interfaces to library DLL consumers. | **N/A (Design)** | Level-up override is a prospective consumer of `k2.saveevents@1`. Scheduled for Chunk **C8**. |

---

## 3. Gap Analysis Table (Local Copies vs. Common Targets)

| Local Feature / Copy | Target Common Component | File Anchors in `LevelUpOverride.cpp` | Migration Effort | Risk Level | Upstream / Common Owner | Target Chunk |
|---|---|---|---|---|---|---|
| Relative include paths (`"../_shared/X.h"`) | Standard include `"X.h"` + `-I Common` | Lines 79–82 | Small (0.5h) | Low | Level-up Session | **C3** |
| Engine Addresses (`struct Engine`) | `GameAddr.h` (`namespace gameaddr`) | Lines 1052–1064 (`g_eng`) | Medium (1.5h) | Low | Common Owner (`19b`) | **C4** |
| Leader creature lookup (`leaderCreature`) | `Party.h` (`party::member`, `party::serverCreature`) | Lines 2038–2046 | Medium (2h) | Medium (resolution semantics) | Common Owner (`Party.h`) | **C5** |
| Config file polling (`fileStamp`, `reloadDue`) | `FileWatch.h` (`filewatch::Stamp`, `changed`) | Lines 220–232 | Medium (1.5h) | Low | Common Owner (`FileWatch.h`) | **C6** |
| Atomic INI writing (`ptWriteText`) | `FileWatch.h` (`filewatch::atomicWrite`) | Lines 1430–1438 | Small (1h) | Low | Common Owner (`FileWatch.h`) | **C6** |
| Logging & rotation (`logf`, `logLine`, `rotateIfLarge`) | `KpLog.h` (KPM logger) | Lines 142–171, 1160 | Large (3h) | Medium (log byte-identity) | KPM / Common Owner | **C7** |
| Playthrough GUID save binding | `k2.saveevents@1` (Library DLL) | Lines 1448–1510 | Design Only (1h) | Low (no live code change) | Library DLL Owner (`doc 22`) | **C8** |

---

## 4. Chunk Sequencing Verdict

The initial chunk roadmap defined in `PHASE7_COMMON_MASTER.md` is **validated and reaffirmed**:

1. **C1 (Current)**: Integration audit baseline established (this document).
2. **C2**: Common contract test (`common_contract_test.cpp`) — **ALREADY COMPLETE** (`d89b511`).
3. **C3 (Next)**: Include style + parity stub alignment (`"../_shared/X.h"` -> `"X.h"`).
4. **C4**: Engine addresses -> `GameAddr.h` (+ proposal for missing addresses).
5. **C5**: `Party.h` adoption.
6. **C6**: `FileWatch.h` adoption (`Stamp`, `ReloadPoll`, `atomicWrite`).
7. **C7**: `KpLog` migration (C7a design / C7b implementation).
8. **C8**: Library DLL interfaces (`k2.saveevents@1` consumer requirements).
9. **C9**: Offers to Common owners (notes only).
