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
| T2 | `scripts/check_library_manifests.py` (provides/require vs source, consumer requires, no dup hook addrs) + selftest, static tier | M0 | - | DONE | (T2 commit) | real tree PASS=3; selftest 15; mutation `k2.portraitslot@1` -> FAIL; default tier 75/75 |
| T3 | `portrait_slots_src/ps_registry_test.cpp` (inert w/o provider, late provider via poll, require after close, short struct_size, k2ps m32 layout asserts) | T1 | split | DONE | (T3 commit) | 99 checks host + m32; mutation `& 127` -> 6 FAIL; default tier 76/76 |
| T4 | contract tests buff / shield / portrait | T1 | split | DONE | (T4 commit) | buff 201 / shield 215 / portrait 126 checks; host + m32 |
| T5a | contract tests companion_ai + ranged_ai | T1 | - | DONE | (T5 commit) | 52 + 32 checks; CompanionAiLogic.h cov 100 |
| T5b | contract tests inventory_sort_filter + perf_lab | T1 | - | DONE | (T5 commit) | 31 + 42 checks; SortLogic.h cov 93.8 |
| T5c | contract tests load_hang_probe + mouse_probe | T1 | - | DONE | (T5 commit) | 54 + 62 checks (mouse_probe pins KpLog.h via KPM include) |
| T5d | contract test level_up_override | T1 | OWNED level-up session (PHASE7 C2) | TODO | | |
| T5e | contract tests char_effects + equip3d | C3, C4 | doc 23 / 14 | TODO | | |
| T6 | Wine smoke portrait-slots + buff + shield (`smoke_portraitslots.cpp`, extra_dlls) | T2 | split | DONE | | `--wine -k portraitslots` x3 PASS (wire 25, late 15, inert 11, order 6 checks) |
| T7 | Wine smoke K2DialogueLetterboxFix | M0 | - | TODO | | `--wine -k letterbox` x3 |
| T8 | `pre_install_gate.sh` (doc 20 Phase 6) | T2 | - | DONE | | PASS perf-lab, FAIL broken tmp kpatch |
| T9 | check.h into TestHarness.h; VTableOverride/MemberFunctionThunk m32 tests (doc 20 Phase 4) | - | - | TODO | | default tier x2 |
| T10 | Coverage gaps from T0: K2Addr.h 9.6% -> >=80% (k2addr_test), GuiKit.h 78.1% -> >=80% | T0 | split (K2Addr) | DONE | (T10 commit) | `run_tests.py --coverage`: GuiKit 98.5, K2Addr 100 |
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
M0 -> T1 -> T2 -> T3 -> T4 (T0 done: unblocked) -> T6 -> T7 -> T8 -> S1a -> L2 -> C1 -> T5a/b/c -> C2 -> L1 -> L3 -> S1b-d/C3/C4/C6 (owner-gated) -> C5 -> T9 -> T5d/e.

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
- 2026-10-03 T2 DONE: static library-manifest gate (suite `library_manifests`): manifest entry format, provides/provide() and requires_interfaces/require() both ways (macros resolved via `_shared_src` includes + kpatch_api.h; `kpatch.log` core-exempt), consumer `requires` id, library/consumer duplicate hook addresses. portrait-slots + buff-duration-hud + shield-bar PASS.
- 2026-10-03 T10 DONE: k2addr_test calls makeSteam()/makeGog() at runtime via volatile fn ptrs and checks every field against the same literals as kSteam/kGog (runtime == constexpr parity) -> K2Addr 281/281; guikit_test adds setClickable, getBorderTint/getAlpha gates, scaleFromRef/unitX/unitY/unitRect (incl. fractional rounding), hit edges, cursorEngine scale/fallback, clickEdge -> GuiKit 198/201 (82-84 default SetRect lambda = real engine call, host-unreachable). No header edits; check_shared_headers 0 drifted. Gate: 74/75, the 1 FAIL is check-raw-addresses on uncommitted dialogue_letterbox_fix_src/K2DialogueLetterboxFix.cpp:285-288 (letterbox session, not T10).
- 2026-10-03 T3 DONE: ps_registry_test.cpp pins k2ps::Client edge cases: inert w/o provider and api==nullptr (one "no KPatchApi" log), poll cadence (2 requires per 512 unconnected polls, connects on the next 256-boundary, 0 requires once connected), require after close = NULL (late client inert; early client usable), KPatchApi struct_size < require end -> 0 requires, partial Register refusal -> connected + ok=false, add() cap 12, K2PsContext offsets/size 72 (all hosts) + K2PortraitSlotsApi offsets/size 24 (m32). 99 checks host + `g++ -m32`; cadence mutant `& 127` -> 6 FAIL. Header notes: refused registrations are never retried; failure onConnect is once-only whatever the reason. Default tier 76/76 (letterbox raw-address failure resolved upstream); check_shared_headers + check_library_manifests exit 0.
- 2026-10-03 T5a/b/c DONE: common_contract_test.cpp + test.toml for companion_ai (52), ranged_ai (32), inventory_sort_filter (31), perf_lab_src/perflab (42), load_hang_probe (54), mouse_probe (62). SafeRead pinned per call site (readAt per T the mod uses, *out untouched on failure, base 0 / <0x10000 / unreadable via local VirtualQuery wrapper, straddling ranges, beginScope reset); load_hang_probe also pins enableRing/g_ringCount/dump (base 0 is not counted; cap/RingSize bounds); mouse_probe pins KpLog fallback sink + level gate, then Init with a fake core (KpLog.h from KPM Patches/Common, as portrait_slots). Host uintptr_t is 8 bytes: pointer hops modelled as uint32_t (fixed a UBSan misaligned load). One readAt mutant per test -> FAIL with '<mod> relies on:'. Default tier 82/82; --coverage all >=80 (new: CompanionAiLogic 100, SortLogic 93.8); check_shared_headers 0 drifted. No source/header edits.
- 2026-10-04 T4 DONE: common_contract_test.cpp for buff_duration_hud (201 checks), shield_bar (215), portrait_slots (126), host ASan/UBSan + `g++ -m32 -Werror` PASS. Pins per call site: K2Addr Steam+GOG field static_asserts, SafeRead readAt/beginScope/enableRing/dump, Gui2D pushClip/popClip balance, clipDepthRaw, flushArg, drawImage slot 6, hudScale, panelRect/inMenuFrame/ClipScope, CapGuard 9 caps on a fake opengl32, imageFor pooling + failed-load cache, Effects decode, Party member/serverCreature on a fake app chain, k2ps registration shape (prio 100/200, phases). Buff + shield also pin the 2 doc 21 backlog Client gaps as-is (refused registrations still connected, failure onConnect once), as the split session asked. Each test.toml gains a [common_contract_test] table (_shared_src first, KPM Common, parity stub, -DGUIKIT_HOST, -Werror). Red proof: scratch Gui2D.h without GL_LIGHTING -> all 3 FAIL with '<mod> relies on:'. Default tier 85/85; check_shared_headers + check_library_manifests exit 0. No source/header/hooks/manifest edits.
- 2026-10-04 T8 DONE: scripts/pre_install_gate.sh (-> pre_install_gate.py; 11 tests in scripts/test_pre_install_gate.py, default-tier py item). Steps: 1 run_tests.py default tier, 2 rebuild each --mod from source into $TMPDIR (build_check), 3 copy KPM Patches into a scratch KPM_DIR with the fresh DLL in the kpatch, 4 Wine smokes for that mod against the scratch dir; prints GATE PASS/FAIL <step> + DLL md5 + head. perf_lab_src/perflab GATE PASS 1m21s (perflab,perflab2,perflab3,hwmip,perflab5; dll 4ed3781f). load_hang_probe_src: step 1 FAIL on a sibling test (py:scripts/test_kotor_save_forge.py pins live QUICKSAVE GUID gfb7bdbf7, now g15032d10 after play; not T8), --skip-default GATE PASS 1m09s (probe11,probe12,probe14; dll ba7f9eab). --selftest PASS: (a) empty exports.def -> GATE FAIL 2 build, (b) load_hang_probe dll in perflab smoke -> GATE FAIL 4 wine exit 3. KPM Patches status + PerfLab.kpatch md5 unchanged. Default tier 86/87 (only failure = save_forge above).
- 2026-10-04 T6 DONE: perf_lab_src/harness/smoke_portraitslots.cpp + smokes.toml row `portraitslots` (first extra_dlls user; kpatch = ../split_build/*.kpatch, the live split set per the split session: PortraitSlots ee5affeb, BuffDurationHUD c58f1743, ShieldBar 1.2.1 4e3c8af5). FakeKPatchApi.h registry, zeroed 7 MB pad + HudScale stub, no test exports. Modes: wire (provided + table complete, 3+5 registrations with exact phase/prio/owner, snapshot round trip, clients read provider creature, HUD_DRAW reached, pad clean) 25 checks; late (clients first, require rate-limited, connect exactly at call 513) 15; inert (unknown KOTOR_VERSION_SHA: no dispatch, one k2addr WARN, no HudScale call, pad all zero) 11; order (IAT tap on GetFileAttributesA: shield prio 100 kill-switch check before buff 200 at frames 30/60, buff registered first) 6. ~1.2 s/mode. Red proof: `late skip-provider` FAIL 3/10, shield-bar.dll removed -> LoadLibrary FAIL exit 1; worker also failed order+wire with a prio-250 patched shield-bar.dll. Full --wine 43/43 wine items PASS; default tier 88/89 (only failure = save_forge GUID pin, not T6). L2's T6 gate met.
