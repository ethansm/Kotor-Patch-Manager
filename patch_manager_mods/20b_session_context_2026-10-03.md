# Doc 20b: session context, 2026-10-03 (testing plan session)

A hand-off for the next session. Read doc 20 (the plan) and 20a (the audit) first. This file records what is *not* in them.

## What happened
1. **Review of the Gemini advice and its critique.** The user pasted Gemini's testing advice together with a critique of it. I checked both against the KPM fork (`~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`, branch `ethansm/local-mods`, HEAD aad0a14).
   - **Gemini's errors.** There are no `K_HOOK` macros, no trampoline chaining, and no `VirtualProtect` in VTableOverride; it said otherwise.
   - **The critique's own errors:**
     - **Same-address collisions are already caught.** `HookValidator.ValidateMultiPatchHooks` (`src/KPatchCore/Validators/HookValidator.cs:140`) refuses them at install time. Only partly overlapping byte ranges get through, and the validator has no xunit test.
     - **CI already exists.** KPM's `ci.yml` runs `validate-patches.py`, `dotnet test` and `make -C src/KotorPatcher test`. The last one runs the i386 tests `tests/t32_*.cpp`.
     - **The `VTableOverride_test` sketch is wrong.** With a virtual destructor, g++ uses slots 0 and 1 for destructors, so the first method is slot 2. A 32-bit detour must also be `__fastcall(self, edx, ...)`.
     - **The collision-checker sketch is wrong.** There is no `target` key in any hooks.toml (all use `address`), and it ignored `target_versions`.
     - **SafeRead uses Windows APIs.** It calls `VirtualQuery` and its API is `readAt`, not `rd<T>`/`wr<T>`, so it cannot be tested with plain host g++.
   - **What held up.** `create-patch.py:444` globs every `.cpp`. Today that is harmless, because our `*_test.cpp` files stay in `patch_manager_mods/*_src`, which is not synced to `Patches/`.
2. **Sonnet audit of all 18 of our mods.** The full report is at 20a, and its overlap scripts are in `20a_audit_tools/`. I spot-checked these on disk:
   - the live count is 31 ids;
   - `slotDigit` has no bound (ForcePowerHotbar.cpp:441);
   - the BuffDurationHUD ICONCACHE `snprintf` loop (`ent[256]`, worst case about 276 bytes);
   - PerfLab `g_stop` is never set (PerfLab.cpp:735);
   - 0x4261CF is patched by both TextureMemoryRelease (in code) and PerfLab `tx=1`.
3. **Plan and checks.** Doc 20 was written and committed as 80fec48. Its section 6 GUI-id note was checked:
   - EquipCharPreview ids 49.. go into `equip_p.gui`, and CharEffectsPanel 69.. into `character_p.gui`, so there is no conflict;
   - level-up shares `character_p.gui` *screen space* with Mod 14, with x 54..456 against 536..752.

## Facts the next session needs
- **Toolchains:**
  - Native: `g++-multilib`, where `-m32` works (`/usr/lib/gcc/x86_64-linux-gnu/13/32`).
  - Windows targets: `i686-w64-mingw32-g++` and `dotnet`. There is no system `wine`; use `~/.local/share/Steam/steamapps/common/Proton - Experimental/files/bin/wine` with a throwaway `WINEPREFIX` under `/tmp/claude-1000/`.
- **Live config:** `~/.local/share/Steam/steamapps/common/Knights of the Old Republic II/patch_config.toml` holds 31 patches and 198 hooks. It is read-only for tests and never written.
- **Existing tests and how they run** (all passed in the 10-03 audit):
  - **Header tests,** each built as `g++ -std=c++17 -Wall -Wextra x_test.cpp -o t && ./t`:
    - `inventory_sort_filter_src/sort_logic_test.cpp`
    - `inventory_3d_viewport_src/Inv3dQuat_test.cpp`
    - `companion_ai_src/companion_ai_logic_test.cpp`
    - `force_power_hotbar_src/config_roundtrip_test.cpp`, which uses `assert`, so `-DNDEBUG` disables it
    - `force_power_hotbar_src/twoda_test.cpp`, which needs a `spells.2da` path argument and fails without one
  - **Level-up suite:** `cd research/companion_levelup && python3 tools/parity/parity.py`. It also runs apply/gate/playthrough/ui tests and writes `*_bin` files into the repo. `wizard_parity.py`, `options_parity.py` and `guikit_test.py` sit beside it. The stub is `tools/parity/stub/{windows.h,m3_win_shim.h,_shared/}`.
  - **Wine smoke tests:** `patch_manager_mods/perf_lab_src/harness/smoke_*.cpp`. The build recipe is in `perf_lab_src/README.md`. These four only print and never fail: `smoke.cpp`, `smoke_probe12.cpp`, `smoke_probe13.cpp`, `smoke_perflab.cpp`. The ActivationFix smoke needs `DISPLAY`.
  - **Python checkers:** `scripts/check_{inv3d,equip3d,chareffects}_kpatch.py`, `scripts/check_effect_decode.py`, `scripts/check_shared_headers.py`.
  - **Python unit tests:** `patch_manager_mods/mods678_research/test_analyze_play_logs.py` (53) and `patch_manager_mods/engine_ai_atlas/tools/tests/test_*.py`.
  - **KPM:** `dotnet test` (427) and `make -C src/KotorPatcher test`. Its `tests/check.h` is the model for our `check.h`.
- **Include trick:** mods include `../_shared/X.h`, but the canonical dir here is `_shared_src/`. Host-compiling a mod `.cpp` needs the parity stub's symlink layout or `-I` tricks.

## Open decisions (doc 20 section 9), still unanswered
1. Should `run_tests.py` run automatically, from a Claude Code hook or a git pre-commit hook, or only when asked?
2. Should the registries for patch sites and GUI ids be local-only or shaped for upstream KPM?
3. Is the mutation gate required for High mods, or only for Common headers?

## Next
Phase 0 of doc 20 section 7. The implementation plan is drafted in plan mode in the following session.

**Phase 0 done (10-03).** Run `python3 scripts/run_tests.py` (`--wine` for smokes, `--selftest` for the runner itself); see the doc 20 Status. Next: Phase 1.
