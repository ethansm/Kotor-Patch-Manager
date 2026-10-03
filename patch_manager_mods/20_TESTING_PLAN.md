# Doc 20: Testing plan for the KOTOR II mod repository

Written 2026-10-03. Inputs:
- the Gemini testing advice and its critique, re-checked on disk this session;
- an infrastructure survey of this repo and the KPM fork (`ethansm/local-mods`);
- a Sonnet audit of all 18 of our patches, at **20a_patch_test_audit_2026-10-03.md**. Its overlap scripts are in `20a_audit_tools/`.

Related plans: `COMMON_EXTRACTION_PLAN.md` (row 11, `TestHarness.h`) and doc 19 (shared logging).

## 1. Where we are

**Live set.** The Steam `patch_config.toml` holds 31 ids and 198 hooks (verified 10-03). The older "28 ids/153 hooks" figure is stale.

**What already exists.** It is better than it looks, but it is scattered across five places and nothing runs it as a whole:

| Kind | Where | Runs with | Status 10-03 |
|---|---|---|---|
| Header unit tests (host g++) | `patch_manager_mods/*_src/*_test.cpp` (5 files) | `g++ -std=c++17 -Wall -Wextra x_test.cpp && ./a.out` | pass; ASan/UBSan clean |
| Fake-engine parity suite | `research/companion_levelup/tools/parity/` (stub `windows.h`, fake engine structs, C++ vs Python twins) | `python3 tools/parity/parity.py` | about 19k cases, 0 failures |
| Wine DLL smoke tests | `perf_lab_src/harness/smoke_*.cpp` (11) | MinGW `-O1`, Proton wine, `.bss` trick for fixed engine addresses | all exit 0. **4 only print and never assert** |
| Static kpatch checkers | `scripts/check_{inv3d,equip3d,chareffects}_kpatch.py`, `check_effect_decode.py`, `check_shared_headers.py` | python3 | pass |
| Python tool tests | `mods678_research/test_analyze_play_logs.py` (53), `engine_ai_atlas/tools/tests/` | unittest | pass |
| KPM framework | `tests/KPatchCore.Tests` (427), `src/KotorPatcher/tests` (`make test`, i386 + x86_64 rows), `tools/validate-patches.py` | dotnet, make, python | pass. KPM's own CI runs them |
| Mutation gate | `mutate_round2.py` (workspace-only, ultrawide GUI generators) | ~9 min at `--jobs 4` | used before deploys; **no C++ mod has one** |

**Toolchains present on this host:** `g++-multilib` (`-m32` works), `i686-w64-mingw32-g++`, `dotnet`, Proton 9 and Experimental wine. Nothing blocks any tier below.

## 2. Gaps, ranked

1. **No runner and no gate.** Nobody runs the existing tests together, so a regression in one of them goes unseen until a play test.
2. **Three high-churn, high-risk mods have zero or near-zero unit tests:**
   - BuffDurationHUD: 46 commits, 0 tests.
   - ForcePowerHotbar: 36 commits, 2 weak tests.
   - EquipCharPreview: 18 commits, only a static checker.
3. **Some tests pass without checking anything:**
   - Four smoke tests only print.
   - `twoda_test` fails with no arguments and otherwise only asserts a row count.
   - `config_roundtrip_test` uses `assert`, so `-DNDEBUG` turns it off.
4. **Static checks missing from `validate-patches.py`:**
   - **Byte ranges.** KPM's `HookValidator.ValidateMultiPatchHooks` only catches two hooks at the *same* address; ranges that partly overlap get through.
   - **Code-patched sites.** Nothing checks the sites that `DllMain` byte-patches directly. The one real collision, 0x4261CF between TextureMemoryRelease and PerfLab `tx=1`, is of this kind.
   - **Exports.** Nothing checks that a detour's `function` is actually exported.
5. **Window procedure stacking.** Four mods subclass the game window: CharEffectsPanel :254, EquipCharPreview :361, ForcePowerHotbar :766 and Inventory3DViewport :498. Each restores only if it is still on top, nothing tests what happens when they unhook in different orders, and none re-subclasses a recreated window.
6. **Duplicated helpers:** `configEnabled`, `writeCode`, `patchIat`, the TX cave and `isItemAbility`. Each copy is untested, and the Common extraction removes them only if tests come with it.
7. **Bugs found by reading the code** (verified on disk 10-03):
   - BuffDurationHUD: the `ICONCACHE` `snprintf` loop can overrun `ent[256]`. The worst case is about 276 bytes, and `sizeof - len` then wraps to a huge size.
   - ForcePowerHotbar: `slotDigit` indexes `"1234567890"[slot]` without a bounds check (ForcePowerHotbar.cpp:441).
   - PerfLab: `g_stop` is never set (PerfLab.cpp:735).
   - ForcePowerHotbar: `strncpy(rr, sp.font, 16)` can leave no terminating NUL (:1023). This is fine only if `SetFont` reads a fixed 16 bytes; verify.

## 3. Test tiers

Each tier says what it proves, where its tests live and how fast it is. The runner (section 4) runs T0 to T3 by default. T4 and T5 run only on request.

### T0: static checks (seconds, no build)
**Built in Phase 1 (10-03).** The files are `scripts/_static_common.py` (shared lib), `scripts/check_hook_bytes.py`, `scripts/check_hook_spans.py`, `scripts/check_exports.py`, `scripts/check_gui_ids.py` (`.venv` python), the registries `tests/static/span_exceptions.toml` and `tests/static/gui_ids.toml`, and one `code_patches.toml` sidecar per byte-patch mod (6 mods, 22 sites). All are suites in `tests/suites.toml`, with colocated tests `scripts/test_static_checks.py`, `test_check_hook_spans.py`, `test_check_exports.py` and `test_check_gui_ids.py`. The bullets below are the original design.
- **0x4261CF exception (documented):** TextureMemoryRelease and PerfLab `tx=1` both write the same 7 bytes, and both memcmp the vanilla bytes first, so whichever writer runs second refuses. `span_exceptions.toml` records this. The checker accepts it only while both `code_patches.toml` entries say `verifies_original = true`, and it fails if the exception goes unused or its `tests` name a smoke mode that does not exist. Two smokes prove each writer refuses the other's jump: `wine:texmem[foreign]` and `wine:perflab3[txforeign]`.
- `validate-patches.py`, which KPM already has.
- **New `scripts/check_hook_spans.py`**, promoted from `20a_audit_tools/overlap2.py`:
  - It reads every `*.hooks.toml` of ours, plus the live `patch_config.toml`, plus a **declared list of code-patched sites**. That list is a new `[[code_patches]]` table in each byte-patch mod's manifest, or a sidecar `code_patches.toml`, so the checker never has to parse C++.
  - It respects `target_versions` and fails on any byte overlap between different patches.
  - It warns on gaps of 8 bytes or less.
  - Opt-in exceptions are allowed when both writers verify the original bytes and are mutually exclusive by config. 0x4261CF is the first one: document it and add a test that the second writer refuses.
- **New export check:** every detour `function` must appear in the patch's exports, read from `exports.def` or `objdump -p` of the built DLL.
- **Hook bytes against the exe.** Generalize the three per-mod `check_*_kpatch.py` into one `check_hook_bytes.py` that walks every mod. It checks original bytes against the vanilla exe and flags relative branches inside stolen bytes. Reuse the logic of `perf_lab_src/hookbytes.py`.
- `check_shared_headers.py` (drift).
- **Address constants.** A grep test that every raw `0x00[4-9A-B]xxxxx` constant in a mod `.cpp` is either in `GameAddr.h`, once that exists, or listed in an allowlist. This keeps the Common extraction honest.

### T1: host unit tests for pure logic (seconds)
- **What it covers:** math, parsers, INI/config, state machines, layout and coordinate scaling, sorting, bitmasks, string formatting, table lookups.
- **Pattern:**
  1. Logic moves into a header (`*Logic.h`, or a Common header) with no engine pointers.
  2. A colocated `*_test.cpp` includes it and is built with `g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined`.
- **Assertions:** a shared `Common/test/check.h` provides `CHECK(cond)` / `CHECK_EQ(a,b)` that count failures and survive `-DNDEBUG`. KPM's `src/KotorPatcher/tests/check.h` can serve as the model. No framework dependency.
- **Fixtures:** tests that need game data (2DA, TLK) use small **checked-in fixtures** under `testdata/`, never the user's install. An optional `--real-data` mode can read the install when present.

### T2: fake-engine tests (seconds)
- **What it covers:** code that reads and writes engine structs: hook bodies, GUI builders, save binding.
- **Model:** the level-up parity suite. Generalize its `stub/windows.h` (49 lines) and `m3_win_shim.h` into **`Common/TestHarness.h`** (Common extraction row 11), which provides:
  - Windows type and function stubs: `VirtualQuery`, `IsBadReadPtr`, `GetTickCount`, `OutputDebugString`;
  - fake memory regions with readable and unreadable pages, so `SafeRead` failure paths can be exercised;
  - builders for fake engine objects at given field offsets (creature, party table, GUI control with vtable);
  - a call recorder for engine functions that tests replace through function-pointer seams.
- **Include paths.** The `../_shared/` include trick is replaced by an `-I` list that the runner owns, so a mod `.cpp` compiles on the host without symlinks.
- **Native build.** Still x86_64, so pointer-size bugs are out of scope here and belong to T3.

### T3: 32-bit tests of calling conventions and memory layout (seconds)
- **Built with:** `g++ -m32`. This works on this host, and KPM's `t32_*` tests are the precedent.
- **Targets:**
  - `MemberFunctionThunk.h` (`this` in ECX, the thunk registry lookup);
  - `VTableOverride.h`. The Gemini sketch would have crashed:
    - with a virtual destructor, g++ fills vtable slots 0 and 1 with destructors, so the first ordinary method is slot 2, not 1;
    - the detour must be `__fastcall(self, edx_unused, ...)`;
  - struct layout `static_assert`s for every engine struct a mod defines (field offsets, `sizeof`), compiled with `-m32`;
  - the window-procedure chain: a fake `SetWindowLongPtr` registry that installs A, B, C and C, A, B, unhooks in every order, and asserts that no message is lost and no restore lands on a stale procedure. This is the test that makes Common `WindowHook.h` safe to adopt.

### T4: whole-DLL smoke tests under Wine (about a minute; on request and before install)
- **Mechanism:** the existing `perf_lab_src/harness` (static MinGW harness exe, `.bss` covering 0x400000-0xB00000, the real `.kpatch` DLL).
- **Each smoke test must:**
  - exit nonzero on any failed check;
  - cover `run`, `off` (inert without a config) and `refuse` (bad original bytes mean no write and a log line).
- **Assert, don't print:** convert `smoke.cpp`, `smoke_probe12/13` and `smoke_perflab` to asserting.
- **Build first:** smoke tests rebuild the `.kpatch` from source before running. The audit found six mods whose `.cpp` is newer than their `.kpatch`.
- **Display:** ActivationFix's foreground check needs `DISPLAY`. Skip it with a warning when the variable is unset.

### T5: play verification (user-run, structured)
- **Not automated, and never launched by an agent** (standing rule).
- **Before:** each live test round gets a **test card** in the mod's doc: steps, expected log lines, what to look at, and a screenshot list.
- **After:** `analyze_play_logs.py`, which already has 53 tests, becomes the standard post-play check. Every mod's log lines get a parser rule, so a play session ends with a pass/fail report instead of manual log reading.

### Mutation gate (high-risk logic only)
- **Why:** it proves the tests would notice a change.
- **What it needs:** port `mutate_round2.py`'s selection and `--jobs` engine to C++ logic headers.
- **Mutations:** flip comparisons, swap `+`/`-`, change constants by ±1, delete early returns.
- **Scope:** run it on headers in the "High" priority set and on Common headers that many mods adopt (SafeRead, IniFile, WindowHook, CodePatch).
- **Gate:** `survived: []` before a Common header ships. This is not a per-commit gate.

## 4. Infrastructure to build

### 4.1 `scripts/run_tests.py`, the single entry point
- **Discovery is by convention:**
  - `**/*_test.cpp` → T1/T2 (host);
  - `**/t32_*_test.cpp` → T3 (`-m32`);
  - `**/test_*.py` → Python;
  - `parity.py`;
  - `smoke_*.cpp` → T4, only with `--wine`.
- **Build flags** come from an optional colocated `test.toml` per directory (extra `-I`, defines, args, fixtures). There is no per-test boilerplate in the runner.
- **Tiers and timing:**
  - default runs T0+T1+T2+T3 in parallel, target under 60 s;
  - `--wine` adds T4;
  - `--mutate <header>` runs the mutation gate.
- **Output:** one line per test; failures show the captured output; exit 1 on any failure; `--json` writes a summary for the docs.
- **Where builds go:** `$TMPDIR/kotor-tests/`, never the repo. The level-up suite currently writes `*_bin` files into the repo, so add them to `.gitignore`.

### 4.2 The gate before an install
Run in this order: `run_tests.py`, then a `.kpatch` rebuild, then `run_tests.py --wine` for touched mods, then install. This extends the "verify, commit, then launch" sequencing rule (feedback memory). Add a short `scripts/pre_install_gate.sh` that does exactly this and prints PASS or FAIL. It replaces the ad-hoc `kpm_registry_gate.sh` pattern.

### 4.3 KPM side (upstream-shaped)
- **Test-file exclusion in `create-patch.py`.** `patch_sources()` (create-patch.py:444) excludes `*_test.cpp` and `t32_*`. This is needed before any test file lives in `Patches/`. Today tests stay in `patch_manager_mods/` only, so nothing breaks yet.
- **Overlapping ranges in `HookValidator.ValidateMultiPatchHooks`:** extend it from same-address to byte overlap, with xunit cases. There are none today.
- **Tests for the Common headers:** `t32_vtable_override.cpp` and `t32_member_thunk.cpp` in `src/KotorPatcher/tests` style, or a `Patches/Common/tests/` dir wired into `make test`.
- **Posting:** each of these is a candidate upstream PR. Per AGENTS.md, nothing is posted without explicit approval in chat.

## 5. Mod backlog

Priority comes from the audit: complexity × crash risk × churn. Unit counts are the audit's upper bounds (see 20a for file:line). "Seam" means what must move out of the `.cpp` for the test to exist.

### High
1. **BuffDurationHUD** (0 tests, 46 commits, per-frame engine writes, 21 raw addresses).
   - First, fix the `ICONCACHE` `snprintf` overrun: clamp `len`, and add a test that feeds 6 maximum-width entries.
   - Extract the counter and arrow logic (raw → tracked → counters, the companion-without-icons bump from bc34464), the duration formatting and the icon-table lookup into `BuffHudLogic.h`. T1 tests cover:
     - every counter transition in the 10-02 fix;
     - duration rounding and formatting at boundaries;
     - icon-table hits and misses.
   - Shield bar, which lives in this DLL: bar fill math, plus the menu companion-bar layout scaling (T1). Portrait-group `SetRect` through vtable slot 1 belongs in T2 with a fake control.
2. **ForcePowerHotbar** (2 weak tests, 36 commits, window subclassing, save/cloud INI).
   - Harden `twoda_test`: a checked-in `spells.2da` fixture and real assertions. Make `config_roundtrip_test` survive `-DNDEBUG`.
   - New T1 tests:
     - picker counts and out-of-stock dimming (build 26);
     - slot mapping, with `slotDigit` bounded plus a test;
     - INI save/load: `.bak` and warnings, the per-save and cloud copy decision, overwrite on load;
     - layout scaling `(x*sx)*c+0.5`.
   - T2 tests: the per-save binding flow (`onSave`/`onLoad`/`onNewGame` against fake save names); HUD rebuild on area transition (the leak in RuntimeGuiControls).
   - This is the hotbar's planned "1.0.0 Phase T". Do it there, adopting Common headers as listed in the extraction plan.
3. **EquipCharPreview** (static checker only, window subclassing, GUI ids 49..78, 29 raw addresses with no byte checks).
   - T0: declare its code-patched and raw-address sites so `check_hook_spans.py` sees them.
   - T1: the weapon/appearance table lookup (`gen_equip3d_weapon_table.py` output), the camera and layout math (share `Inv3dQuat` where it is the same), and the INI.
   - T3: the window-procedure chain test (shared with the three other subclassing mods).

### Medium
4. **CharEffectsPanel.**
   - Problem: `check_effect_decode.py` tests a *Python port* of the decoder, not the C++ decoder.
   - Fix: move the C++ effect decoder into a header and run the same 63 cases against it as a T1 table test. The Python file becomes the oracle.
   - GUI id range 69..150 goes into a T0 id-reservation check (section 6).
5. **CompanionAI.** The logic header already has 2,436 checks. The 22 untested units are in the `.cpp` glue: target selection inputs and frame reads. Write T2 tests with fake creatures. T0 should also flag its 13 raw addresses that have no byte check.
6. **LevelUpOverride** (most covered).
   - Add an `onLevelUp` T2 test.
   - Add a T2 test that `LUO_G*` globals round-trip and that a save without them loads cleanly (mod-removal safety).
   - Fold the parity suite into the runner unchanged.
7. **Inventory3DViewport.** Beyond the quaternion checks: viewport rect scaling, model-fit math (`inv3d_model_fit.py` has a Python twin, so make it a parity pair), and the INI. T2: the `CItem` mutation is restored on every exit path.
8. **PerfLab.** Fix or remove `g_stop`. Make the smoke tests assert. The 0x4261CF refuse path gets a smoke assertion (the T0 exception from section 3).
9. **RangedAI.** Delete the duplicated `inWeaponRange` and `parseMask` and use the tested `cai::` copies (behavior-neutral, verified by the existing 2,436 checks). Add T1 tests for what remains.
10. **`_shared_src` SafeRead / GuiKit.** SafeRead gets direct T2 tests with fake unreadable pages: `readAt` failure, ring buffer wrap, the region cache in `beginScope`. GuiKit's 49 checks move from the parity directory to live next to the header.

### Low
- **The byte-patch mods** (ActivationFix, EquipIconFix, RobustnessFixes, TextureMemoryRelease, K2DialogueLetterboxFix): their Wine smoke tests are adequate. Two changes:
  - when Common `CodePatch.h` lands, test its `writeCode` once (expect/replace/refuse, and partial-group behavior, since RobustnessFixes groups are not transactional) instead of each copy;
  - add each mod's code-patched sites to T0.
- **InventorySortFilter:** already well tested. Add the 79-byte stolen-byte hook (H6b) to `check_hook_bytes.py`.
- **K2WrapMeasureFix:** T0 only. Fix the stale README.
- **LoadHangProbe and MouseProbe:** research tools. Make the smoke tests assert; nothing more unless they go live again.

## 6. Checks across mods (T0 additions)
- **Hook and code-patch byte ranges:** section 3, T0.
- **GUI control-id reservations** (built in Phase 1: `tests/static/gui_ids.toml`, `scripts/check_gui_ids.py`). A `gui_ids.toml` registry that lists every injected id range per `.gui` file (EquipCharPreview 49..78, CharEffectsPanel 69..150, level-up, hotbar, ...).
  - The check fails on overlap within the same `.gui`.
  - EquipCharPreview's 49..78 and CharEffectsPanel's 69..150 overlap numerically, but they target different files: `equip_p.gui` (EquipCharPreview.cpp:59) and `character_p.gui` (CharEffectsPanel.cpp:64-65). That is not a conflict (checked 10-03). The registry has to be keyed by `.gui` file.
  - `character_p.gui` is shared on screen: Mod 14's panel uses x 536-752, and level-up stays within 54..456 (LevelUpUi.h:14). The registry records **screen rects** as well as ids, and fails on overlapping rects within the same `.gui`.
- **2DA row claims** (compatch pipeline): every row our scripts add (e.g. `build_ranged_kite_2da.py`, row 693 per the capsule-replay memory) is registered, and `verify_2da.py` fails on two claimants. Fold it into the runner.
- **Window procedure subclassers:** the T3 chain test, plus a T0 count. Adding a fifth subclasser fails until it uses Common `WindowHook.h`.

## 7. Developing the tests: phases

Model split, following the delegation memory:
- Sonnet subagents do the extraction and test-writing legwork.
- Opus designs the seams, reviews every diff, runs the gates and commits.

Rules that apply to every phase:
- Each phase ends with a commit, a doc 20 status line and a memory update.
- No phase touches live game files; installs remain a separate, user-approved step.
- A mod whose doc says "awaiting user live test" is not refactored until that test reports. Phase 3 checks this per mod.

**Phase 0: foundations (about half a day).**
- `Common/test/check.h`, `scripts/run_tests.py` (T1 discovery, Python tests, `parity.py`), and `.gitignore` for test binaries.
- Fold in every existing test unchanged.
- Fix the six do-nothing tests: four print-only smoke tests, `twoda_test`, and `NDEBUG` in `config_roundtrip_test`.
- **Exit:** `run_tests.py` is green and lists every existing test. A deliberate break of any one of them turns it red (checked by hand once).

**Phase 1: static checks (about half a day).**
- `check_hook_spans.py` (with code-patch declarations for the byte-patch mods), the exports check, `check_hook_bytes.py` (merging the three per-mod checkers) and `gui_ids.toml`.
- **Exit:** all four pass on the live set, and the 0x4261CF exception is documented. **DONE 10-03** (see Status).

**Phase 2: the three bugs (about 1 hour, small and isolated).**
- BuffDurationHUD `snprintf` clamp, hotbar `slotDigit` bound, PerfLab `g_stop`. Each comes with a test that fails before the fix.
- Ship them in each mod's next planned build, not as a separate install.

**Phase 3: High-priority mods (BuffDurationHUD, ForcePowerHotbar, EquipCharPreview).**
- One mod per step: extract a header, write tests, rebuild, check for an identical log diff.
- The hotbar step is its planned Phase T.
- **Exit per mod:** logic headers have line coverage of 80% or more (`gcov`), and the mutation gate shows `survived: []` on the new headers.

**Phase 4: `TestHarness.h`, and the 32-bit and window-procedure tests.**
- Generalize the level-up stubs; T3 tests for `VTableOverride`, `MemberFunctionThunk`, struct layouts and the window-procedure chain.
- This unblocks Common `WindowHook.h`, phase 4 of the extraction plan.
- **Exit:** the level-up suite runs on the shared harness with identical results.

**Phase 5: Medium mods,** in backlog order (CharEffectsPanel C++ decoder first, because its oracle already exists).

**Phase 6: Wine tier and the gate before an install.**
- `run_tests.py --wine` and `pre_install_gate.sh`; smoke tests rebuild `.kpatch` from source first.
- **Exit:** the gate is used for the next real install.

**Phase 7: upstream (needs explicit approval for each PR).**
- The `create-patch.py` test exclusion, overlapping ranges in `HookValidator` with xunit cases, and the Common `t32_` tests. Draft each in `issue_drafts/` first.

**Ongoing rules once Phase 0 lands:**
- Any bug fix in a mod comes with a test that fails before it, where a seam exists.
- Every new Common header ships with T1/T3 tests and a mutation pass (the Common extraction "host tests" step, made concrete).
- `run_tests.py` is green before every commit that touches mod sources.

## 8. Things Gemini proposed that we drop
- Unicorn/Qiling emulation of engine functions: too much setup for too little return. T4 Wine smoke tests with the `.bss` trick already execute real DLL code against fake engine memory.
- Catch2/GoogleTest: not needed. `check.h` plus the runner covers it with zero dependencies, which matches how KPM tests itself.
- "Auto-fix trampoline chaining": KPM does not do this. Same-address collisions are refused at install by `HookValidator`, and the rest by `VerifyBytes` at runtime.

## 9. Open questions for the user
1. Should `run_tests.py` run automatically, through a Claude Code hook on edits under `patch_manager_mods/*_src` or a git pre-commit hook, or only when asked?
2. Should the hook-span, GUI-id and 2DA registries live in this repo only, or be shaped for upstream KPM (a manifest field) from the start?
3. Should the mutation gate be required for High mods, or only for Common headers?

## Status
- 10-03: plan written; audit 20a committed. Next: Phase 0.
- 10-03: **Phase 0 done.**
  - `python3 scripts/run_tests.py` covers 17 items: 5 host cpp tests (ASan/UBSan), 3 Python tests/suites, 4 level-up parity scripts and 5 checkers. It is green in 8.5 s wall time; `parity.py` is the slowest at 8.4 s.
  - `--wine` adds 27 smoke runs (10 smokes × modes from `perf_lab_src/harness/smokes.toml`). It was green 3 times out of 3, at about 138 s each, with no flakiness.
  - `--selftest` exits 0: planted failures (C++ normal, C++ `-DNDEBUG`, Python) all report FAIL.
  - Hand breaks went red as expected: a twoda value, a `sort_logic_test` check and the perflab leak threshold.
  - `check.h` lives at `tests/include/check.h` (namespace `kt`). It moves to `Common/test/` with `TestHarness.h` in Phase 4.
  - The six do-nothing tests are fixed:
    - `config_roundtrip_test` now runs on `check.h` (51 checks).
    - `twoda_test` uses a synthetic `testdata/spells_fixture.2da` (40 checks); the real file is still available via argv[1].
    - `smoke_perflab` and `smoke_probe12` now assert.
    - `smoke.cpp` became `smoke_probe11.cpp` and now asserts. Its old manual-dump step never checked anything: it copied the report before the probe had written it.
    - `smoke_probe13` is retired to `harness/retired/`.
  - Session context: `20c_phase0_session_2026-10-03.md`. Next: Phase 1.
- 10-03 (hotbar Phase X, X1-X3): **colocated Common tests** in `patch_manager_mods/_shared_src/` (shared `test.toml`: parity stub `-I`, `-DGUIKIT_HOST`, `-Werror`):
  `saferead_test` (55 checks, `readExo`), `guikit_test` (42, the new `imageFor`/`createLabelFromTemplate` out-params), `party_test` (89, `Party.h` + `static_assert`s that
  `gameaddr::` equals GuiKit's constants). Each was hand-broken once and went red. `check_shared_headers.py` syncs only `*.h`, so tests never reach `Patches/Common`.
  - **-m32 blocked by the parity stub:** `stub/windows.h` `VirtualQuery` returns `RegionSize = 1<<40`; on 32-bit `lo + RegionSize` wraps and `regionReadable` loops
    forever. Fix belongs to the level-up stub owner (cap RegionSize, e.g. `0x80000000 - lo`). Host 64-bit runs are fine.
  - **Mutation pass pending** (no C++ mutation tool yet) for `SafeRead.h` `readExo`, `GuiKit.h` out-params, `GameAddr.h`, `Party.h`.
  - The level-up parity `guikit_test.cpp` (49 checks) stays where it is until its owner agrees to move it (§5 Medium #10).
- 10-03: **Phase 1 done** (static checks; wave 1 `9a44ddf`, wave 2 + wiring in the next commit).
  - **Live results.** All four checkers report 0 FAIL on the live set.
    - `check_hook_bytes`: 158 hooks and 22 code patches all match the vanilla exe. 25 INFO lines report relative branches in stolen bytes, all with `skip_original_bytes = true` (H6b, CompanionAI 0x586F43, 23 LevelUpOverride sites). 2 WARN lines flag the stale `Patches/CompanionAI/CompanionAI-0.1.0-live.kpatch` (18 hooks against 31 in source). ActivationFix.kpatch is **not** stale: the size difference is only an extra LAA sha in `target_versions`, so the check compares parsed hooks.
    - `check_hook_spans`: 18 mods and 180 spans in source, 31 patches and 220 spans installed. No overlaps. The 0x4261CF exception is used in both scopes. 2 gap WARNs, each 6 bytes: LoadHangProbe 0x40D692 against ActivationFix 0x40D69E, and PerfLab `PerfLabNetOvfTest` 0x7040E0 against Robustness `netovf_b` 0x7040EF. There is no source-vs-live drift.
    - `check_exports`: 13 mods PASS; 5 have no detours.
    - `check_gui_ids`: 0 FAIL. The append bases are level-up 151..169 and hotbar 111..185. The engine's base is the vanilla max id + 1, not the control count.
  - **Runner.** `run_tests.py` now has 31 items and takes 9.2 s. `--wine` has 59 items, including the new `texmem[foreign]` and `perflab3[txforeign]`, and takes 136 s. `--selftest` passes.
  - **Hand breaks.** Each of these went red: a flipped `code_patches` byte, a third-id code patch at 0x4261D0, the exceptions file removed, a name deleted from `exports.def` (temp repo copy), and a GUI range shifted into another mod's range.
  - **Read-only check.** `find -newer` over the KPM tree, `override/` and `patch_config.toml` showed nothing written.
  - **Retired.** `check_{inv3d,equip3d,chareffects}_kpatch.py` moved to `scripts/retired/`. The old ±8 check now lives in the spans checker and the exports check in `check_exports`. The USER32-import check was dropped.
  - **New suite `kpm-validate-patches`** runs KPM's `tools/validate-patches.py`, with `cwd = ${KPM_DIR}`; `run_tests.py` gained `$`/`~` cwd expansion for this. It is **red today**: another session staged `Patches/K2DialogueLetterboxFixProbe/`, which reuses the id `k2-dialogue-letterbox-fix`. That is a real duplicate-id finding (83 patches, 1 error); the user decides whether the probe dir goes or gets its own id.
  - **§9 Q2 answered:** the registries are local sidecars, and the KPM manifests are untouched.
  - **Session context and findings table (F1-F12):** `20d_phase1_session_2026-10-03.md`.
  - **Next:** Phase 2, the three bugs.

