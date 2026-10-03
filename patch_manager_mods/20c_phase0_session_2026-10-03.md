# Doc 20c: Phase 0 session context (2026-10-03)

Phase 0 of doc 20 is done and committed in `20c04c9`. This file records what a fresh session needs that the commit and doc 20 do not.

## How to run the tests
- `python3 scripts/run_tests.py` runs the default tier: 17 items, about 9 s wall time on 12 cores.
- `--wine` adds the Wine smokes: 44 items in all, about 138 s. Wine runs are serial.
- `--selftest` proves the runner reports a failure as FAIL, including a C++ test built with `-DNDEBUG`.
- Other flags:

| Flag | Effect |
|---|---|
| `--list` | list the items |
| `-k SUBSTR` | filter the items |
| `--jobs N` | parallel builds and host runs |
| `--json PATH` | write the results as JSON |
| `-v` | print each item's full output when it finishes |
| `--no-sanitize` | build C++ tests without ASan/UBSan |

- **Builds and the Wine prefix** live in `$TMPDIR/kotor-tests/`. With `TMPDIR` unset, that is `/tmp/claude-<uid>/kotor-tests/`. The prefix is created once with `wineboot`.

**How the runner classifies results:**

| Outcome | Status |
|---|---|
| exit 0 | PASS |
| exit greater than 0 | FAIL |
| timeout | ERROR, after killing the process group (`wineserver -k` for Wine) |
| negative return code, or 128+N for a fatal signal | ERROR |
| could not launch | ERROR |

## Where things live
**`tests/include/check.h`:**
- namespace `kt`, with `KT_CHECK`, `KT_CHECK_EQ` and `kt::Report(name)`;
- `KT_CHECK_EQ` handles integers, enums and `char*`/`const char*` (via `strcmp`); pass `.c_str()` for a `std::string`;
- set `KT_VERBOSE=1` to print PASS lines too.

It moves to `Patches/Common/test/` with `TestHarness.h` in Phase 4.

**Discovery:**
- `patch_manager_mods/**/*_test.cpp` builds with `-fsanitize=address,undefined` and runs with cwd = the test's directory.
- An optional `test.toml` next to the test sets `cxxflags`, `args` and `skip_reason`. A table named after the test stem overrides the top-level keys.
- `**/test_*.py` runs as `python3 <file>`.
- A `tests/` dir that holds a `helpers.py` runs as one item: `.venv/bin/python -m unittest discover`.
- Discovery skips `.venv`, `__pycache__`, `retired/`, `testdata/`, `fixtures/` and nested git trees.

**`tests/suites.toml`:**
- explicit entries: the 4 level-up parity scripts and 5 checkers;
- `requires` entries expand `$KPM_DIR`, `$KOTOR2_ROOT` and `$REPO`; a missing path gives SKIP;
- a leading `"python"` in `cmd` is replaced by the interpreter;
- `[discovery] exclude` is empty for now.

**`patch_manager_mods/perf_lab_src/harness/smokes.toml`:**
- 10 rows, using the keys documented in its header;
- the MinGW runtime DLLs are in `/usr/lib/gcc/i686-w64-mingw32/13-win32/` (libstdc++-6, libgcc_s_dw2-1) and `/usr/i686-w64-mingw32/lib/` (libwinpthread-1).

**`harness/wait_line.h`:** `waitForLine(file, "A ... B", ms)`, which polls every 100 ms.

**Hotbar fixture:** `force_power_hotbar_src/testdata/make_spells_fixture.py`. Run it with `.venv/bin/python`; it reads its output back and checks it. It writes `spells_fixture.2da` (binary `2DA V2.b`, 43 rows, 7 columns).

## Measured timings
**Default tier:**

| Item | Time |
|---|---|
| `parity.py` | 8.4 s (dominates) |
| engine_ai_atlas unittest | 4.3 s |
| `config_roundtrip_test` | 3.3 s |
| `twoda_test` | 2.4 s |
| `companion_ai_logic_test` | 2.3 s |
| everything else | under 1.5 s each |

**Wine smokes:**

| Smoke | Time per run |
|---|---|
| probe11 | 25 s (it waits 15 s for MAIN_STALL by design) |
| probe14 | 9 to 14 s |
| equipiconfix (run mode) | 12 s |
| robustness (run mode) | 7 s |
| perflab | 7 s |
| everything else | 1 to 6 s |

## What testing found
**Fixed in Phase 0:**
1. **The old LoadHangProbe smoke (`smoke.cpp`) never tested its manual dump.** It copied `hangprobe_report.txt` to `report_manual.txt` before the probe had written it, and it only printed, so nothing noticed. `smoke_probe11.cpp` now waits for the MANUAL header, with a 6 s deadline because the probe samples for 2.5 s after the trigger.
2. **Dynamic MinGW builds need the runtime DLLs next to the exe.** That applies to probe14, perflab, perflab2 and perflab3. Without them Wine exits 53 and prints nothing under `WINEDEBUG=-all`.
3. **The level-up parity scripts write build outputs into the repo.** All 8 `*_bin` files are now in `research/companion_levelup/tools/parity/.gitignore`.

**Corrections to doc 20b and the plan:**
- probe12 logs `ret=0078237c` in lowercase.
- perflab2/3 do not need `DISPLAY`. Only probe14 and activationfix do.

**Still open:**
- **The perflab leak check has no headroom.** The simulated leak is exactly 100000 bytes in 1000 blocks, and the check is `>=` those numbers. It is deterministic.
- **Three checkers hardcode their KPM paths.** `check_{inv3d,equip3d,chareffects}_kpatch.py` use `~/Games/...` and ignore `$KPM_DIR`. Phase 1's `check_hook_bytes.py` replaces them.
- **The three Phase 2 bugs are unfixed:** the BuffHUD ICONCACHE `snprintf` overrun, the unbounded hotbar `slotDigit`, and PerfLab's `g_stop`, which is never set.
- **No mod bugs turned up.** Every existing test passed once it was wired into the runner.

## Untouched but relevant state
- **The working tree has unrelated, uncommitted changes from other sessions.** These are the steam_db_pr3 files, `.pyc` and `debug_pykotor` logs, plus many untracked docs. Always stage by path.
- **Untracked Ghidra export scripts in the KPM tree.** Another session's Ghidra export job wrote `ExportDataTypes.java`, `ExportFingerprints.java` and `ExportProgramSymbols.java` under `tools/ghidra_scripts/`. They have nothing to do with testing.
- **A play session ran 10-03 from 01:11 to 01:24, and its logs are in the game dir, unanalysed.** I offered a read-only pass: copy the logs out, run `analyze_play_logs.py`, then report on the companion-ai probe, the level-up run 4 log (from line 387), the BuffHUD ARROWS/ICONCACHE lines and hotbar build 26. The user has not answered yet.
- **Doc 20 section 9 questions are still open:** whether the runner runs automatically, whether the registries are local or shaped for upstream, and the scope of the mutation gate.

## Next
Phase 1 (static checks): see the implementation plan written after this session.
