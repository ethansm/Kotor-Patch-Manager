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
- **Common contract tests (user rule, 2026-10-03):** every mod that includes a `_shared_src` (Patches/Common) header owns `<mod>_src/common_contract_test.cpp`, built straight against `_shared_src`, with one block per Common API the mod calls. It pins the semantics that mod relies on (not the header's own suite); every check message starts `<mod> relies on:`. Any Common change re-runs it, so a breaking edit by any session fails in the consumer. Prove each block red once with a deliberate Common break. Proposed: the T0 checker warns on a Common include without a contract test, and `check_shared_headers.py --sync` refuses to sync while any contract test is red. Reference consumer: the hotbar.

### 4.2 The gate before an install
Run in this order: `run_tests.py`, then a `.kpatch` rebuild, then `run_tests.py --wine` for touched mods, then install. This extends the "verify, commit, then launch" sequencing rule (feedback memory). Add a short `scripts/pre_install_gate.sh` that does exactly this and prints PASS or FAIL. It replaces the ad-hoc `kpm_registry_gate.sh` pattern.
- **Usage (T8, 10-04):** `scripts/pre_install_gate.sh --mod <dir under patch_manager_mods> [--mod ...]` prints `GATE PASS mods=.. smokes=.. head=..` + the rebuilt DLL md5s, or `GATE FAIL <n> <step>: <reason>`. It rebuilds into `$TMPDIR/kotor-tests/gate/` and smokes the fresh DLL through a scratch `KPM_DIR` (never the KPM tree or the game dir). `--selftest` proves a syntax error FAILs at step 2 and a wrong DLL FAILs at step 4.

### 4.3 KPM side (upstream-shaped)
- **Test-file exclusion in `create-patch.py`.** `patch_sources()` (create-patch.py:444) excludes `*_test.cpp` and `t32_*`. This is needed before any test file lives in `Patches/`. Today tests stay in `patch_manager_mods/` only, so nothing breaks yet.
- **Overlapping ranges in `HookValidator.ValidateMultiPatchHooks`:** extend it from same-address to byte overlap, with xunit cases. There are none today.
- **Tests for the Common headers:** `t32_vtable_override.cpp` and `t32_member_thunk.cpp` in `src/KotorPatcher/tests` style, or a `Patches/Common/tests/` dir wired into `make test`.
- **Posting:** each of these is a candidate upstream PR. Per AGENTS.md, nothing is posted without explicit approval in chat.

## 5. Mod backlog

Priority comes from the audit: complexity × crash risk × churn. Unit counts are the audit's upper bounds (see 20a for file:line). "Seam" means what must move out of the `.cpp` for the test to exist.

### High
1. **BuffDurationHUD** (0 tests, 46 commits, per-frame engine writes, 21 raw addresses).
   - **10-03 status:** F7 fix DONE (`bh::appendf` + `buff_hud_logic_test`, `abc01d8`; call sites in the buff/shield split session's W2 commit). T1 DONE after the split (`ce37f3e`, BuffDurationHUD 2.0.0): arrow counters + column geometry in `BuffHudLogic.h`, `buff_hud_logic_test` 64302 checks (differential 0 mismatches). Shield + gauges live in `shield_bar_src/` (`ShieldLogic.h`, `shield_logic_test`, owned by the split session; its `sb::appendf` drift-guards against `bh::appendf`); expiry is `k2fx` in `_shared_src/Effects.h`.
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
   - **10-03 status:** T1 DONE (`1d0ccab`): `Equip3dLogic.h` (`eq3d`) + `equip3d_logic_test` (13459 checks). Raw addresses are now gated by `check_raw_addresses`. Weapon-table drift test skipped (generator runs at import time, needs the game install + pykotor).
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
- **Window procedure subclassers:** the T3 chain test, plus a T0 count. Adding a fifth subclasser fails until it uses Common `WindowHook.h`. **T0 count DONE** (`check_subclassers.py`, `tests/static/subclassers.toml`, `670905a`).
- **Raw-address allowlist DONE** (`check_raw_addresses.py`, `tests/static/raw_addresses.toml`, `670905a`): 236 distinct addresses outside GameAddr.h. A new address FAILs; `--write-baseline` only by a human or Opus, committed with the change that adds the address.

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
- **Exit:** the gate is used for the next real install. 10-04: the gate exists (`scripts/pre_install_gate.sh`, doc 24 T8); it is used at the next real install.

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

- 10-03 (level-up Phase 6b, `research/companion_levelup/PHASE6B_PLAN.md`; coordination note): the level-up session adds
  `tests/luo_scenarios/saves.toml` (forced-state scenarios shared by a save forge, an offline fake-engine runner and a live log checker; doc 21
  to come), `scripts/luo_log_check.py` + `scripts/test_luo_log_check.py` (auto-discovered), and fixes **F10** in the parity stub. Proposed
  `tests/suites.toml` entries (scenario runner, forge test) will be posted here for the doc-20 owner before they are added; `run_tests.py` is not touched.
- 10-03 (level-up Phase 6b): LevelUpOverride item 6 (§5) - the onLevelUp T2 test and the LUO_G round-trip / removal-safety test are DONE in
  `research/companion_levelup/tools/parity/scenarios.py` (`scenario_test.cpp`, host + `--m32`, 0481b70). Forced-state testing (forged saves,
  ini switcher, log checker) is doc 26 `26_FORCED_STATE_TESTING.md`. Proposed suites.toml entries for the owner:
  `{name = "levelup-scenarios", cmd = ["python", "research/companion_levelup/tools/parity/scenarios.py"], requires = ["which:g++"]}` and the same
  with `"--m32"`; `scripts/test_kotor_save_forge.py` re-execs itself under `.venv` when pykotor is missing (auto-discovery works).
- 10-03: **Phase 2 done** (`abc01d8`; session record doc 20d "Phase 2-3 session").
  - F1: probe moved out of KPM `Patches/` to `letterbox_probe_stage/`; `kpm-validate-patches` green (82 patches, 0 errors).
  - F7: `bh::appendf` + `cpp:buff_hud_logic_test` (35 checks; red on the old idiom via ASan + canary). Latent, not live.
  - F9: PerfLab `g_stop` and the unreachable tail removed (no red test possible: dead code).
  - F10: parity stub 32-bit `RegionSize` ends at `UINTPTR_MAX` (`-m32` saferead_test: old hangs, new 55 checks).
  - F11: `wine:perflab` leak check asserts deltas.
  - New `scripts/build_check.py` (+14 tests): mod sources built like KPM `create-patch.py`, exports checked. Opt-in tier: `run_tests.py --build` (also run by `--wine`).
- 10-03: **Phase 3 partial.** EquipCharPreview T1 (`1d0ccab`), T0 extras raw addresses + subclassers (`670905a`). BuffDurationHUD T1 (`ce37f3e`, after the split's W4 `a193a08`). Hotbar Phase T stays with the hotbar session.
  - Level-up suites `levelup-scenarios` and `levelup-scenarios-m32` added as requested above; the forge test needed no suite after `fa6d4fc`.
  - Runner: default 53 items in ~16 s; `--wine` 83 items in 172 s; `--selftest` passes.
  - **Default tier is red at the moment, from other sessions' uncommitted work:** `check-hook-spans` (the portrait_slots_src split duplicates BuffHUD hooks mid-split) and `check-raw-addresses` (31 new addresses in RobustnessFixes.cpp WIP). Both owners have been told.
- 10-03 evening (hotbar Common/test-hardening plan): **C++ coverage + mutation tooling landed** (c6500c1).
  - `run_tests.py --coverage [-k ...]`: `--coverage -O0 -fkeep-inline-functions -fkeep-static-functions`, `gcov -t` per test build dir, line hits merged across tests, `COV <pct> <hit>/<lines> <header>`. Run it **without `-k`** for the true merged number (a `-k` subset under-counts headers other tests exercise).
  - `run_tests.py --mutate <header>` = `scripts/mutate_header.py` (comparison/logical/arith/constant/early-return/bool mutants; scratch-tree overlay mirrors the repo because quote includes beat `-I`; same flags + sanitizers as the runner; baseline must be green). ~7 min per header at `--jobs 4`; big headers cost more (CombatLogic.h ≈ 1000 mutants, 6-13 min per full run), so budget 3-4 full runs when killing survivors, or iterate with `--only K` on the survivors and do one full run at the end. A flushed `progress i/n killed= survived=` line prints every `--progress` (25) mutants: run `python3 -u ... 2>&1 | tee <scratch>/run.txt` in the background and check `tail -1`. Py tests 15 + 6 on `scripts/testdata/mutate_fixture/`.
  - Merged coverage 10-03 20:55: SafeRead 100, FileWatch 100, TwoDA 100, HotbarConfig 99.2, HotbarLogic 98.7, Tlk 97.8, Party 97.3, Gui2D 98.4, Effects 93.5, SceneView 90.9, PortraitSlots 89.3, GuiKit 78.1, K2Addr 9.6 (table data). Mutation baseline SafeRead 121/175 (54 survivors); triage of the 7 hotbar/Common headers in `20e_mutation_triage_hotbar_common.md`.
  - Hotbar consumer contract test `force_power_hotbar_src/common_contract_test.cpp` (226 checks, red-proven per area); new hotbar tests: `tlk_test` 95, `twoda_test` 142, `config_roundtrip_test` 568, `hotbar_logic_test` 391; Common `filewatch_test` 65, `saferead_test` 161, `party_test` 234. Bugs found and fixed test-first: SafeRead dump cap + INT_MIN readAt hang, Tlk offset wrap + cached I/O failure, TwoDA double Parse (ASan), HotbarConfig atoi/long wrap + slot OOB (ASan) + lone CR, TGA `w*h*4` overflow (ASan).
- 10-04 (hotbar 1.1.0, D1): full `run_tests.py --coverage`: HotbarLogic.h 97.7%, HotbarConfig.h 99.2%, FileWatch.h 100%, GuiKit.h 98.5%, Party.h 97.3%, SafeRead.h 100% (gaps = closing braces / host-uncallable __thiscall stubs). No mutation run this round. Gates: 87/89 (save_forge reads live saves; raw-address fixed cf4b097), check_gui_ids 86 controls ids 111..196, hook_spans/exports 0 FAIL, shared headers 0 drift.
- 10-05 **RESUME HERE (doc-20 owner session closed):** read `.claude/handoffs/20_TESTING_PLAN.handoff.md` sections 4-6, then doc 20d's findings table.
  - Open: #5 CompanionAI T2, T2 `TestHarness.h`, T3 struct asserts; byte-patch Low items wait for Common `CodePatch.h`.
  - F13 upstream PR: not sent; needs user approval.
- 10-05 (Medium + T3 WndProc + Low round, findings F27-F33 in doc 20d): **#4 DONE** `adf868f` (`ChareffDecode.h`, 315 checks against `check_effect_decode.py --emit-cases`), **#7 DONE** `96c2053` (`Inv3dLogic.h`, 356 checks), **#9 DONE** `7f63c52` (`RangedAiLogic.h` + drift guard vs `cai::`, F27 fixed test-first), **T3 WndProc chain DONE** `4019742` (`tests/wndproc_chain`, 611 checks, `-m32`; `run_tests.py` now also discovers `tests/**/*_test.cpp`; F28 characterised), **Low** `73652a4` (H6b pinned in `test_static_checks.py`, WrapMeasureFix README; LoadHangProbe smokes already asserted; MouseProbe has no smoke). All three mod changes are source-only, not installed (inv3d + chareffects behaviour-identical; ranged-ai differs only on null / >10-digit masks). Sweeps vs HEAD: 4.7M + 143.5M comparisons, 0 mismatches.
  - Gates 10-05: default 130/131 (42 s), `--build` 131/132, `--selftest` PASS, `--wine` 202/211. Every red item is queue-hud (F33, not this round's).
- 10-05 **BF1 DONE** (bug-fix batch 1): F29+F30 `b25d4fa` (cfx A_REGEN INT_MAX, marqueeText off<0), F31+F32 `fd34ccf` (inv3d INI long line rejected once, fitLookup range), F21 closed (finding wrong), F33 DONE `6ab426f` (theirs). Source-only, not installed. Gates: default 131/131, `--build` 132/132. Next = BF2 (EquipCharPreview F14/F15/F16/F19).
- 10-05 **BF2 DONE** (EquipCharPreview): F14+F15+F16 `47be017` (screen key clamp/explicit, fov cap 150, neutral-align fallback), F19 `14d905a` (hex floats rejected, toIntSat, long line rejected once incl. CRLF, getModel/partLen removed; `;`/`#` cut by design). Source-only, not installed. Gates: default 131/131, `--build` 132/132. Next = BF3 (BuffHUD F25).
- 10-05 **BF3 DONE** (BuffDurationHUD F25) `60dd2eb`: dead inner `if (disc)` removed, `arrowRawFor(0)` -> nullptr, `ArrowRaw.seen` so the first all-zero ARROWS tuple is logged once per slot (pins flipped test-first). Source-only, not installed. Gates: default 131/131, `--build` 132/132. Next = BF4 (F28 WindowHook design note).
- 10-05 **BF4 DONE** (F28 WindowHook design note) `e60de4a`: `20f_windowhook_design.md` recommends design A (per-DLL pass-through + module pin), rejects B (shared dispatcher); `wndproc_chain` gains the `SubPass<N>` model (0 handling after unhook over 12 orders; 10/12 leave an inert forwarding proc chained). No mod code. Gates: default 131/131, `--build` 132/132. Next = BF5: user approved WindowHook design A 10-05 (implement `WindowHook.h`); still open user decisions F17/F18/F20/F26/F3/F4/F13.
- 10-05 **BF5 DONE** (F28 Common fix): `7d41942` `_shared_src/WindowHook.h` (`kp::WindowHook<Tag>`, design A) + `windowhook_test` (12 orders, 0 handling after unhook; mutation 14/16, 2 survivors in the Win32-only `pin()` wrapper); `7aca7fd` GuiKit wheel on it + `subclassers.toml` `uses = "WindowHook"`. KPM `d46a699` (Common) + `f03c258` (`_shared` forwarder). The 4 mod subclassers are owner-gated (doc 32 MG6); the F28 CHARACTERIZATION asserts are unchanged. Gates: default 132/132, `--build` 133/133. Next: dispatch MG6, or the open user decisions F17/F18/F20/F26/F3/F4/F13. Next finding F34.
- 10-05 **BD1 DONE** (user decisions on 7 open findings): F17 `60bc42e` RE note (engine `UpdateWeaponClass` also lets only the left wield decide; `styleFor` unchanged, full table pinned) + F18 integer layout math `34cb100`; F26 `_shared_src/StrFmt.h` `84e9a7c` (KPM `d18015c`); F3/F4 `[[adjacency]]` registry in `span_exceptions.toml` `4c5aaa3` (3 pairs disassembly-verified, check_hook_spans WARN 0); F20 6 canonical `gameaddr::` names + 33 stale allowlist rows `dca7bfc` (KPM `d6aaaec`); F13 local PR draft only (`kpm-pr-test-source-filter`, not pushed). Owner-gated follow-ups: doc 32 MG2 (+`strfmt::appendf`), MG7 (GameAddr names). Gates: default 134/134, `--build` 135/135. No new findings; next finding F34.
- 10-05 **BD2 DONE**: probe edits committed (c0df1d8) + KPM sync; MG7 (ec4f6b6..35e0146, hotbar/chareffects/equip3d), MG6 (hotbar, chareffects, inv3d, equip3d on kp::WindowHook, registry c368b5e), MG2 buff (2ba0875); F28 legacy asserts relabelled (9134fe7). Gates 134/135 green. Not installed; live shield-bar/perf-lab/mouse-probe DLLs differ from KPM builds of the same source (build-path, not timestamp). Next finding F34.
  - Trap: a smoke that reads `split_build/` needs that kpatch refreshed after each Patches/ rebuild (F33 root cause).
- 10-05 **BD3 DONE** (source + KPM): MG2 shield `strfmt::appendf` df9fa9e, drift guards dropped f73d632; featureOk/fill not adopted (F34); 4 stale KPM dirs synced (272efc2, b0a5ac6, 8d46009, cb531a3, 9ddf0b1) + ShieldBar 444e26f. Gates 134/134, `--build` 135/135, static FAIL 0, 0 drifted. Install of all MG6/MG7/MG2 DLLs pending user go. Next finding F35.
- 10-05 **F34 DONE** (user: "Add int overload"): gui2d int-ratio `vfillClip`/`drawVFill` 52e6e81, ShieldBar `k2addr::featureOk` + int `drawVFill` 6d51383, BuffDurationHUD featureOk wrapper d516e1f (all output/pixel-identical); KPM 8618433, c4c02ec, ef15824. `--build` 135/135, static FAIL 0, 0 drifted. Staged `stage_f34` (head ef15824; DIFF = shield-bar, buff-duration-hud only); INSTALLED 10-05 (36 patches, verify 0 mismatches, backup `$G/backups/f34_install_20261005/`), play check pending. QueueHud/PortraitSlots rebuilds differ from live via older K2Addr.h table offsets (not F34), left on live builds. Next finding F35.
- 10-05 **T5e DONE** (test-only, no install): Common contract tests for chareffects (c042624, 32 checks: gameaddr x2, saferead readAt shape, WindowHook 1-7) and equip3d (aa63ec8, 27 checks: gameaddr, WindowHook 1-6; new test.toml keeps `-Werror`). Red-proven per block: ClientCreatureVtable+4, readAt false-on-success, install early-out dropped (ASan recursion abort), buried uninstall no passthrough. `--build` 137/137, static FAIL 0, 0 drifted. Next finding F35.
- 10-05 **T9b DONE** (test-only, no install), Phase 4 harness half: `tests/include/TestHarness.h` is canonical (`check.h` = same-dir forwarder; opt-in `-DKT_MSVC_CRT_SHIM` supplies `vsnprintf_s`/`sprintf_s`/`_TRUNCATE`/`__fastcall` so KPM `Common.h` builds for host -m32), mirrored to KPM `Patches/Common/test/TestHarness.h` (ca22462, unpushed) and drift-checked by `check_shared_headers.py` EXTRA (abbfa9b). `_shared_src/member_function_thunk_test.cpp` (0966844, 47 checks, -m32 ASan/UBSan + `_GLIBCXX_DEBUG`): registry store/lookup/null-reject/overwrite, ECX dispatch with param, unregistered no-op, inline-asm thiscall stack balance, stable thunk address. Red-proven: Register null guard dropped, Lookup end() test dropped (caught only with `_GLIBCXX_DEBUG`), thunk drops param, harness never counts failures (`--selftest` FAIL). `--build` 138/138. Level-up parity `CHECK` migration to the harness = T9c (level-up owner).
