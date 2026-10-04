# 24 — Master plan status: Common integration, library DLLs, test coverage

Plan source: session 81aa7cfa (2026-10-03), "Master Plan: Patches/Common integration, shared library DLLs, test coverage".
One row per batch. Each batch = one plan-mode session: Opus reads only its row + the named doc section, dispatches 1-3 Sonnet
workers, gates, commits explicit paths, then updates its row here (status, commit, next) and the memory index line.

## Batch protocol
1. `git status` of the batch's paths in both repos (KOTOR-II + KPM `ethansm/local-mods`); SendMessage the owner session if listed; stop if mid-edit.
2. Sonnet worker(s): target files, objective, guardrails (no game, no live dir, no commit, no files outside list), verify command; report <= 40 lines.
3. Opus gate: re-run verify, `check_raw_addresses / check_hook_bytes / check_hook_spans / check_shared_headers`, review risky hunks only.
4. Commit explicit paths, `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
5. Update this table + memory; live install only with user GO (splice pattern, or KPM after L2).

## Traps
- Shared index with sibling sessions: commit with `git commit -- <paths>` only (6f9363e swept staged renames; fixed 456a3b0). Docs 25/26 are taken by the level-up session.
- `scripts/run_tests.py` + `test_run_tests_coverage.py` owned by session test-coverage-dll-integration (T0: coverage roots + common_contract_test exe collision).
- LevelUpOverride (src, KPM Patches/LevelUpOverride, luo_log_check, docs 08/25/26) owned by level-up session (research/companion_levelup/PHASE7_COMMON_MASTER.md).
- KPM reinstall over the hand-edited live config reverts the split (kpm_install_state.json stale) until L2. Splice one `[[patches]]` block instead.
- `check_raw_addresses.py --write-baseline` while siblings' GameAddr.h/K2Addr.h are dirty: restore HEAD toml, add own lines only.
- `pgrep -f 'swkotor2\.exe'` matches itself in compound commands: use `pgrep -ax swkotor2.exe`.
- Harness stubs must honour a thiscall callee's ECX.
- Owned areas (ask first): hotbar (doc 07/22), level-up (doc 08), buff/shield split (doc 21), equip/inv3d (doc 23), Mod 14 (doc 14).
- Never put Common on the KPM core `-I` path (19d:40-44); never overwrite KPM `_shared/*.h` forwarders with repo copies.

## Batch table

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| M0 | Create doc 24; refresh 19b status column | - | - | DONE | (this commit) | T1 |
| T0 | `run_tests.py --coverage` roots = every `patch_manager_mods/*_src` with test.toml; exe name `<dir>__<stem>` (same-stem `common_contract_test` collided: Text file busy) | - | - | DONE | (T0 commit) | B8-style: GuiKit 78.1 / K2Addr 9.6 to >=80% |
| T1 | `tests/include/FakeKPatchApi.h` from ps_dispatch_test.cpp:58-80; port ps_dispatch_test | M0 | split (doc 21) read-only | DONE | (T1 commit) | ps_dispatch 64/64 host + m32 + mingw build; Wine not on PATH |
| T2 | `scripts/check_library_manifests.py` (provides/require vs source, consumer requires, no dup hook addrs) + selftest, static tier | M0 | - | TODO | | |
| T3 | `portrait_slots_src/ps_registry_test.cpp` (inert w/o provider, late provider via poll, require after close, short struct_size, k2ps m32 layout asserts) | T1 | split | TODO | | |
| T4 | contract tests buff / shield / portrait | T1 | split | TODO | | `-k common_contract` |
| T5a | contract tests companion_ai + ranged_ai | T1 | - | TODO | | |
| T5b | contract tests inventory_sort_filter + perf_lab | T1 | - | TODO | | |
| T5c | contract tests load_hang_probe + mouse_probe | T1 | - | TODO | | |
| T5d | contract test level_up_override | T1 | OWNED level-up session (PHASE7 C2) | TODO | | |
| T5e | contract tests char_effects + equip3d | C3, C4 | doc 23 / 14 | TODO | | |
| T6 | Wine smoke portrait-slots + buff + shield (`smoke_portraitslots.cpp`, extra_dlls) | T2 | split | TODO | | `--wine -k portraitslots` x3 |
| T7 | Wine smoke K2DialogueLetterboxFix | M0 | - | TODO | | `--wine -k letterbox` x3 |
| T8 | `pre_install_gate.sh` (doc 20 Phase 6) | T2 | - | TODO | | PASS perf-lab, FAIL broken tmp kpatch |
| T9 | check.h into TestHarness.h; VTableOverride/MemberFunctionThunk m32 tests (doc 20 Phase 4) | - | - | TODO | | default tier x2 |
| S1a | mirror sync buff + shield + portrait (K2Addr) | - | split | TODO | | `diff -q` clean + check_shared_headers |
| S1b | mirror sync ForcePowerHotbar FileWatch | - | hotbar | TODO | | |
| S1c | mirror sync LevelUpOverride | - | OWNED level-up session | TODO | | |
| S1d | mirror sync EquipCharPreview | - | doc 23 | TODO | | |
| C1 | `CodePatch.h` from RobustnessFixes; adopt ActivationFix, EquipIconFix, TexMem, then PerfLab/LoadHangProbe | - | - | TODO | | each Wine smoke x3 unchanged |
| C2 | ExoString helpers onto `saferead::readExo` + `readResRef16` | - | level_up part OWNED level-up session (proposals via 19b) | TODO | | saferead_test + perflab5/companion tests |
| C3 | EquipCharPreview Common adoption (doc 23:133 step 1) | S1d | doc 23 | TODO | | |
| C4 | CharEffectsPanel: EffectIcons, GuiKit, `TextFit.h` | build 4c accepted | doc 14 | TODO | | |
| C5 | KpLog adoption (ranged_ai, equip_icon_fix, activation_fix, texmem, then perf-lab/robustness diagnostics) | - | - | TODO | | |
| C6 | K2Addr intake of hotbar raw addresses (Steam col; GOG zero + callout) | - | hotbar | TODO | | |
| L1 | Optional dependency decision (`optional_requires` vs runtime lazy require) — ask user | - | user | TODO | | `dotnet test tests/KPatchCore.Tests` |
| L2 | Install the split through KPM (dry-run byte-identical blocks 2-4, then GO) | S1a, T6, T8 | user GO | TODO | | refreshed kpm_install_state.json |
| L3 | `k2.saveevents@1` library (perf-lab EVT sites 0x52F610/0x52FB80/0x53AF20/0x7A06B0/0x7D0F50) | L1, T3, T6 | hotbar (consumer) | TODO | | |
| L4 | Design-only: hotkeys, hud-host, k2.guiinput | 2nd consumer | - | DEFERRED | | |

Track P (engine research, separate): perf-lab 0.6.0 from W9 (doc 10 "Status log 20:49"), robustness 1.3.0 Delay/AssignCommand leak fix (shim 0x7037C0).

## Suggested order
M0 -> T1 -> T2 -> T3 -> T4 -> T6 -> T7 -> T8 -> S1a -> L2 -> C1 -> T5a/b/c -> C2 -> L1 -> L3 -> S1b-d/C3/C4/C6 (owner-gated) -> C5 -> T9 -> T5d/e.

## Final verification (end of each track)
- `python3 scripts/run_tests.py` and `--wine` all PASS x3.
- `check_shared_headers / check_raw_addresses / check_hook_bytes / check_hook_spans / check_library_manifests` FAIL=0.
- KPM `dotnet test tests/KPatchCore.Tests`; `scripts/kpm_registry_gate.sh` GATE PASS.
- Every Common consumer has `common_contract_test.cpp`.
- Live installs post-check: DLL md5s, exe `7751f7d8`, AR_ERROR `a2d88789`.

## Status log
- 2026-10-03 M0 DONE: doc 24 created; 19b ranked-list status column refreshed (KpLog pilot done, items 2/3/7/8 adoption counts).
- 2026-10-03 T1 DONE: tests/include/FakeKPatchApi.h (registry.cpp semantics: dup/closed/empty rejected, struct_size override, fake kpatch.log sink); ps_dispatch_test ported, 64 checks host/m32, mingw build clean.

- 2026-10-03 T0 DONE: baseline default run 66 items 29-41s, 66 PASS after fix (was 65 + 1 ERROR exe collision). `--coverage` now 18 headers / 6 roots: _shared_src EffectIcons 100, Effects 93.5, FileWatch 100, Gui2D 98.4, GuiKit 78.1, K2Addr 9.6, Party 97.3, PortraitSlots 89.3, SafeRead 100, SceneView 90.9; hotbar HotbarConfig 99.2, HotbarLogic 98.7, Tlk 97.8, TwoDA 100; BuffHudLogic 100; Inv3dQuat 100; PsDispatch 100; ShieldLogic 100. Below 80%: GuiKit, K2Addr. companion_levelup parity is suite-only (not a cov root). Mods with no unit tests (13): active_effects_panel (py decoder only), activation_fix, equip_icon_fix, texture_memory_release, robustness_fixes, perf_lab, load_hang_probe, dialogue_letterbox_fix (smoke not in smokes.toml), ranged_ai, mouse_probe, wrap_measure_fix, dialogue_letterbox_fix_probe. 20e_mutation_triage_hotbar_common.md referenced but missing; SafeRead mutation 121/175.