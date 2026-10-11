# A1 recon: host test structure (read-only)

## 1. tests/include/TestHarness.h (159 lines, read whole)
- Idiom: no registry. Each test is a `main()` of `KT_CHECK(expr)` and `KT_CHECK_EQ(a, b)` (TestHarness.h:140-141), ending in `return kt::Report("<name>")` (TestHarness.h:14, 91-95). Exit 0 if no failures.
- Assertion style: non-aborting. `Check` counts; prints `FAIL` lines always, `PASS` only with `KT_VERBOSE` (TestHarness.h:44-54). Never uses `assert()`, so `-DNDEBUG` cannot disable it (TestHarness.h:7-8). `CheckEq` prints got/want, hex for integers, strcmp for C strings (TestHarness.h:57-89).
- Also: POSIX-only RAII `kt::TempDir` (TestHarness.h:101-133). `tests/include/check.h` is a 3-line forwarder to TestHarness.h.

## 2. patch_manager_mods/action_hotbar_src/
- `test.toml` (25 lines, read whole). Keys used: only `cxxflags`. Top-level `cxxflags = ["-Werror"]` is the directory default. Per-test tables keyed by file stem replace (not merge) the top-level key: `common_contract_test`, `config_roundtrip_test`, `hotbar_logic_test`, `textures_test`, `hotbar_log_test`, `keybif_test`. No `args`, `skip_reason` or `covers` keys.
- Suite declaration: no registration call. Any `*_test.cpp` is discovered (scripts/run_tests.py:343) and becomes item `cpp:<stem>` (run_tests.py:305). `test.toml` is optional, read from the same directory (run_tests.py:273-279).
- `hotbar_logic_test.cpp:1-60`: includes `"check.h"` (which forwards to TestHarness.h). Each case is a labeled `{ // a. ... }` block inside one `int main()` using `KT_CHECK` / `KT_CHECK_EQ`.

## 3. scripts/run_tests.py filter syntax
- Positional `filters` (run_tests.py:806, nargs="*"): an argument equal to an existing item id is an exact match. Otherwise it is a substring match on item id (run_tests.py:864-872).
- `-k SUBSTR` (run_tests.py:790): substring match on item id (run_tests.py:863).
- `--id ID` (repeatable, exact) (run_tests.py:791). `--ids-file PATH` (run_tests.py:792).
- Item id forms: `cpp:<stem>` (run_tests.py:305), `py:<path>`, `suite:<name>`, `wine:<name>[mode]`.

## 4. Ghidra consumers (rg hits, 6 files)
- YES, real data: `scripts/test_query_ghidra.py`. Runs `query_ghidra.py stats --json` against the repo knowledge DB and asserts `server_functions > 10000` (lines 13-22).
- YES, synthetic dump: `patch_manager_mods/ghidra_knowledge/tools/tests/test_re_card_prep.py` (temp-dir fixture, lines 1, 137-140).
- YES, synthetic dump: `patch_manager_mods/steam_db_batches/tools/tests/test_batch.py` (mini Ghidra dump, line 40).
- NO: `scripts/test_ghidra_server_api.py`. Subprocess and credentials are mocked, no server contact (lines 2-6).
- NO: `scripts/test_subagent_tools.py` (line 77-87) and `scripts/test_clef_matcher.py` (lines 8, 64). "ghidra" is only a prompt or role string.

## 5. Fixture convention
Colocated `testdata/` directories beside the mod or script (e.g. `patch_manager_mods/action_hotbar_src/testdata/` with `make_*_fixture.py` generators, `scripts/testdata/kplog/`), plus `fixtures/` in `patch_manager_mods/perf_lab_src/fixtures/`. Discovery skips `testdata/` and `fixtures/` (run_tests.py:21-23). No `*golden*` files exist. Synthetic inputs are built in temp dirs at test time.

## Environment check
- `$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common` exists on this machine (used by the action_hotbar `-I` flags).
- Tests were not run.
