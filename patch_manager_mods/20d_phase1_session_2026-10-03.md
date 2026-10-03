# Doc 20d: Phase 1 session context (static checks), 2026-10-03

Doc 20 is the testing plan, and doc 20c records the Phase 0 session. This doc records how Phase 1 was built, what it found and what is still open. Commits: wave 1 is `9a44ddf`, wave 2 plus the wiring is `2075f2d`. The implementation plan was `~/.claude/plans/bright-dancing-twilight.md`.

## What exists now

### Checkers
All checkers are in the default `run_tests.py` tier as suites in `tests/suites.toml`.

| Checker | What it checks | Live result 10-03 |
|---|---|---|
| `scripts/check_hook_bytes.py` | Every mod hook and `code_patches.toml` site against the pristine vanilla exe: bytes, section, instruction boundary, relative branches in stolen bytes. Also `.kpatch` packaging and stale embedded hooks. | 0 FAIL, 25 INFO (rel branches, all `skip_original_bytes=true`), 2 WARN (stale `CompanionAI-0.1.0-live.kpatch`) |
| `scripts/check_hook_spans.py` | Byte overlaps (FAIL), gaps of 8 bytes or less (WARN) and source-vs-live drift (WARN), in two scopes: source, and installed (the live `patch_config.toml` plus the code patches of installed ids). Exceptions come from `tests/static/span_exceptions.toml`. | 0 FAIL, 2 gap WARNs (see Findings) |
| `scripts/check_exports.py` | Detour `function`s against `exports.def` and against the DLL export table inside the matching KPM `.kpatch` (matched by id and version, so stale builds are ignored) | 0 FAIL, 13 PASS, 5 mods without detours |
| `scripts/check_gui_ids.py` (`.venv` python, pykotor) | `tests/static/gui_ids.toml` against the vanilla `ultrawide_gui/originals/*.gui`, the live `override/*.gui` and `source_regex` pins in the C++ | 0 FAIL; append bases level-up 151..169, hotbar 111..185 |
| `kpm-validate-patches` suite | KPM `tools/validate-patches.py`, cwd `${KPM_DIR}` (`run_tests.py` gained `$`/`~` cwd expansion) | **RED**: duplicate id, see Findings |

### Shared code and registries
- **Shared lib:** `scripts/_static_common.py`.
  - It provides `load_mods` (skips `testdata`/`retired`; same-id `-suffix` variants are excluded unless `variants=True`), `load_live_config`, `load_code_patches` and `kpatch_members`.
  - PE handling is pure Python: `PeImage` (sections, `read_va`, `exports()`, works on in-memory bytes). Disassembly is objdump `disasm` plus `branch_target`.
  - Reporting and version checks: `Reporter` (FAIL/WARN/INFO/PASS/SKIP, `--quiet`) and `require_version`.
  - The `_common` import is optional, because `scripts/_common.py` is untracked; it belongs to another session's compatch work.
- **Sidecars:** six `code_patches.toml`, one per byte-patch mod, with 22 sites in all. The mods are ActivationFix, EquipIconFix (8), RobustnessFixes (9), TextureMemoryRelease, DialogueLetterboxFix (a data double at 0x9A7D30) and PerfLab (tx, g1skip).
  - **Rule:** a new DLL-written exe site needs a sidecar entry, or the span and byte checks never see it.
  - The schema is in each file's header.
- **Exception 0x4261CF** (TMR vs PerfLab `tx=1`): `tests/static/span_exceptions.toml`. Both writers memcmp first, so the second one refuses.
  - It is proven by `wine:texmem[foreign]` and `wine:perflab3[txforeign]`, smoke modes added in `perf_lab_src/harness/`.
  - The checker fails if either side loses `verifies_original`, if the exception goes unused, or if its `tests` name a smoke mode that does not exist.
- **Colocated tests:** `scripts/test_static_checks.py` (29), `test_check_hook_spans.py` (21), `test_check_exports.py` (21; it imports `build_pe` from `test_static_checks.py`) and `test_check_gui_ids.py` (20; the pykotor test skips under system python).
- **Retired:** `scripts/retired/check_{inv3d,equip3d,chareffects}_kpatch.py`. Their USER32-import check was dropped on purpose.

### Runner numbers
- The default run has 31 items and takes 9.2 s.
- `--wine` has 59 items and takes 136 s.
- `--selftest` passes.
- Each checker was hand-broken once and went red.
- The read-only check (`find -newer` over the KPM tree, `override/` and `patch_config.toml`) was clean.

## Findings in existing code (Phase 0 + Phase 1)
The user wants these called out for other contributors. See memory `feedback-report-test-findings`.

| # | Where | Issue | Kind | Ours/upstream | Status |
|---|---|---|---|---|---|
| F1 | KPM `Patches/K2DialogueLetterboxFixProbe/manifest.toml` | Reuses the id `k2-dialogue-letterbox-fix` of `Patches/K2DialogueLetterboxFix`. KPM's validator FAILs (83 patches, 1 error). It was staged by another session (doc 17 letterbox probe 1.0.1). | hygiene, blocks the gate | ours (in the KPM tree) | **open: user decides** whether to delete the probe dir or give it its own id. Default `run_tests.py` is red until then. |
| F2 | KPM `Patches/CompanionAI/CompanionAI-0.1.0-live.kpatch` | Stale build next to the 0.2.0-probe: 18 hooks against 31 in source. A wrong install is possible. | hygiene | ours | open: delete it (user approval, KPM tree) |
| F3 | LoadHangProbe 0x40D692-0x40D698 vs ActivationFix code patch 0x40D69E | 6-byte gap | latent risk | ours | WARN only |
| F4 | PerfLab `PerfLabNetOvfTest` 0x7040E0-0x7040E9 vs Robustness `netovf_b` 0x7040EF | 6-byte gap | latent risk | ours | WARN only |
| F5 | `RuntimeGuiControls.md` | Said the hotbar has 57 controls (N..N+56); the real count is 75. | doc drift | ours | FIXED `9a44ddf` |
| F6 | Plan / doc 20 claim "ActivationFix.kpatch stale" | False: the size difference is an extra LAA sha in `target_versions`. The checker compares parsed hooks. | wrong fact | n/a | corrected |
| F7 | BuffDurationHUD ICONCACHE `snprintf` overrun | Found by audit 20a (code reading), not by tests | real bug | ours | **open: Phase 2** |
| F8 | ForcePowerHotbar `hb::slotDigit` unbounded | audit 20a | real bug | ours | FIXED `446bb8d` (test-first, `hotbar_bugs_test`); build 27 awaits the user's smoke test |
| F9 | PerfLab `g_stop` never set | audit 20a | real bug | ours | **open: Phase 2** |
| F10 | level-up parity `stub/windows.h` `VirtualQuery` returns `RegionSize = 1<<40` | `regionReadable` loops forever under `-m32` (it wraps). Blocks T3 for Common tests. | test-tool bug | ours (stub owner = level-up session) | open |
| F11 | `perflab` leak check | No headroom: exactly 100000 bytes in 1000 blocks against a `>=` check (doc 20c) | test fragility | ours | open (low) |
| F12 | GUI append-id semantics | The engine's base is the vanilla max id + 1, not the control count. The plan's wording was wrong; the numbers were right. | wrong fact | n/a | documented in `gui_ids.toml` header |

Upstream-interesting candidates. Do not post without the user's approval.
- **KPM `HookValidator` only catches same-address hooks.** Partial overlaps and DLL code patches are invisible to it, and our spans checker covers both. A natural upstream RFC.
- **KPM logs "Function not found" at runtime and silently skips the hook.** An install-time exports check would catch it; ours is `check_exports.py`.

## Still open after Phase 1
- **The user decides F1 and F2.** Both live in the KPM tree, which is read-only for test sessions.
- **Doc 20 Phase 2:** F7 and F9, each with a test that fails before the fix, shipped in each mod's next planned build. F8 is already fixed.
- **Doc 20 T0 extras not built yet:**
  - the raw-address allowlist: every `0x00[4-9A-B]xxxxx` constant in a mod `.cpp` must be in `GameAddr.h` or the allowlist;
  - the WndProc subclasser count;
  - 2DA row claims (§6).
- **Doc 20 Phase 3:** the High mods (BuffDurationHUD has 0 tests; ForcePowerHotbar; EquipCharPreview), then Medium and Low (§5).
- **IAT patches** (PerfLab, LoadHangProbe `patchIat`) are not covered by any span check.
- **`check_exports` and `check_hook_bytes`** each have their own ".kpatch for this mod" selection; a lib helper would unify them (optional).

## Hard rules that carried through
- Read-only toward the game dir, the live `patch_config.toml`, the live `override/` and the KPM tree.
- Never launch the game. Wine only through `run_tests.py --wine` (the prefix is under `/tmp/claude-1000/kotor-tests`).
- Stage by path: the tree holds unrelated changes from other sessions.
- Sonnet subagents do the routine work. Opus reviews, wires, verifies and commits.
