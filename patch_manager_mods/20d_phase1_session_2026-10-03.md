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
| F1 | KPM `Patches/K2DialogueLetterboxFixProbe/manifest.toml` | Reuses the id `k2-dialogue-letterbox-fix` of `Patches/K2DialogueLetterboxFix`. KPM's validator FAILs (83 patches, 1 error). It was staged by another session (doc 17 letterbox probe 1.0.1). | hygiene, blocks the gate | ours (in the KPM tree) | **RESOLVED `abc01d8`** (user decision 10-03): kpatch moved to KPM `letterbox_probe_stage/`, probe dir removed (inputs identical to the repo source), id kept (installing it replaces 1.0.0). Validator: 82 patches, 0 errors. |
| F2 | KPM `Patches/CompanionAI/CompanionAI-0.1.0-live.kpatch` | Stale build next to the 0.2.0-probe: 18 hooks against 31 in source. A wrong install is possible. | hygiene | ours | **RESOLVED** 10-03 (user approved): deleted (untracked; sha256 4bd6e2f0...; unreferenced by `patch_config.toml`). The check_hook_bytes WARNs are gone. |
| F3 | LoadHangProbe 0x40D692-0x40D698 vs ActivationFix code patch 0x40D69E | 6-byte gap | latent risk | ours | WARN only |
| F4 | PerfLab `PerfLabNetOvfTest` 0x7040E0-0x7040E9 vs Robustness `netovf_b` 0x7040EF | 6-byte gap | latent risk | ours | WARN only |
| F5 | `RuntimeGuiControls.md` | Said the hotbar has 57 controls (N..N+56); the real count is 75. | doc drift | ours | FIXED `9a44ddf` |
| F6 | Plan / doc 20 claim "ActivationFix.kpatch stale" | False: the size difference is an extra LAA sha in `target_versions`. The checker compares parsed hooks. | wrong fact | n/a | corrected |
| F7 | BuffDurationHUD ICONCACHE `snprintf` overrun | Found by audit 20a (code reading), not by tests. **Latent:** the 6th (last) ICONCACHE entry is the one that truncates, so nothing writes past the end today; probeDump/dump never truncate. | real bug (latent) | ours | **RESOLVED** `abc01d8` (`bh::appendf` + `buff_hud_logic_test`, red on the old idiom) + call sites (7, incl. describePools) in the buff/shield split session's W2 commit |
| F8 | ForcePowerHotbar `hb::slotDigit` unbounded | audit 20a | real bug | ours | FIXED `446bb8d` (test-first, `hotbar_bugs_test`); build 27 awaits the user's smoke test |
| F9 | PerfLab `g_stop` never set | audit 20a | dead code | ours | **RESOLVED** `abc01d8`: flag and unreachable tail removed; reports are written every flushMs. No red test (dead code). |
| F10 | level-up parity `stub/windows.h` `VirtualQuery` returns `RegionSize = 1<<40` | `regionReadable` loops forever under `-m32` (it wraps). Blocks T3 for Common tests. | test-tool bug | ours (stub owner = level-up session) | **RESOLVED** `abc01d8`: 32-bit RegionSize ends at `UINTPTR_MAX` (the plan's `2^32 - a` would also wrap to 0). `-m32` saferead_test: old stub timeout 124, new 55 checks. |
| F11 | `perflab` leak check | No headroom: exactly 100000 bytes in 1000 blocks against a `>=` check (doc 20c) | test fragility | ours | **RESOLVED** `abc01d8`: deltas over a pre-leak `stats()` snapshot |
| F12 | GUI append-id semantics | The engine's base is the vanilla max id + 1, not the control count. The plan's wording was wrong; the numbers were right. | wrong fact | n/a | documented in `gui_ids.toml` header |
| F13 | KPM `Patches/create-patch.py` `find_sources` | Compiled every root `*.cpp`, including colocated `*_test.cpp` / `t32_*` (doc 20 assumed it excluded them). Copying a mod dir with tests into `Patches/<Name>/` broke the build (`check.h: No such file`). | build bug | KPM (upstream-interesting) | **FIXED on the fork** `f743ac4` (ethansm/local-mods, user approved): `is_test_source` filter; red/green on a fixture and on a scratch EquipCharPreview build (old rc=1, new packages). Upstream PR: not sent (needs approval; route via the upstream-PR session). |
| F14 | EquipCharPreview `:238` screen clamp | A bad `screen_w` resets to 3840x1600 but leaves `screenExplicit` true, so `syncScreen` stops following the framebuffer; a lone `screen_w` keeps h=1600 and marks the pair explicit | real bug (config edge) | ours | open (pinned as CHARACTERIZATION in `equip3d_logic_test`) |
| F15 | EquipCharPreview `:817`/`:933` | `fov / zoom` uncapped: fov 90 and zoom_min 0.2 give a 450-degree FOV | real bug (config edge) | ours | open |
| F16 | EquipCharPreview `:681` | Unreadable stats pointer -> alignment 0 -> "evil" idle | latent | ours | open |
| F17 | EquipCharPreview `:760` `styleFor` | Dual wield: only the left weapon decides the style (right pistol + left saber = style 4) | behaviour question | ours | open |
| F18 | EquipCharPreview spin layout | `(int)(W*0.04f)` differs by 1px under x87 (MinGW i686 default) vs SSE at widths that are multiples of 25 | cosmetic | ours | open (tests avoid those widths) |
| F19 | EquipCharPreview | INI: `strtof` takes hex floats (`stage=0x10` = 16), `(int)1e30` is UB, `;`/`#` cut values; `fgets(600)` splits long lines; `partLen`, `getModel` unused | hygiene | ours | open |
| F20 | Raw addresses across mods | Same address, different names: `0x0099EE14` (ClientCreatureVftable / CreatureVftable), `0x0051AD20`, `0x006D6E30`, `0x00A1B4E0`/`0x00A1B76C`; 8 addresses copy-pasted between EquipCharPreview and Inventory3DViewport; `FnGetArea` in CompanionAI and RangedAI; GuiKit.h holds 19 raw copies of GameAddr.h values | GameAddr.h migration candidates | ours | open (INFO lines in `check_raw_addresses`) |
| F21 | ForcePowerHotbar subclass comment | Says "Mod 1/5 do the same"; no other source subclasses the WndProc | doc drift | ours | open |
| F22 | BuffDurationHUD 2.0.0 `columnLayout` (~:180-186) | Re-derive on column change clamps to MaxIconSize but not MinIconSize: a tiny card (cardW 0 at scale 2) gets size 0; cards < ~52 px at scale 2 get 3 columns of sub-minimum icons. Real cards unaffected | latent | ours | open: in doc 21 Backlog, the split session fixes it and updates the test (live 2.0.0 f37135c8 predates ce37f3e) |
| F23 | BuffDurationHUD 2.0.0 `:188` | `(cardH-gap)/(size+gap)` divides by zero when scale < 0.2 and cardW < 0 (hudScale clamps to >= 0.1; DrawBuffColumn takes engine values unchecked) | latent crash | ours | open: in doc 21 Backlog, the split session fixes it and updates the test (live 2.0.0 f37135c8 predates ce37f3e) |
| F24 | BuffDurationHUD 2.0.0 `columnSig` (:195) | Bit fields overlap (`debuff=1,cardH=0` == `debuff=0,cardH=16`), `cardW<<20` overflows above 2047: a change can go unlogged | log only | ours | open: in doc 21 Backlog, the split session fixes it and updates the test (live 2.0.0 f37135c8 predates ce37f3e) |
| F25 | BuffDurationHUD 2.0.0 | Dead branch `:238-244` (inner `if (disc)` always true); `arrowRawFor(slot 0)` matches a free cell without claiming it (unreachable); first all-zero ARROWS sighting never logged | hygiene | ours | open |
| F26 | `bh::appendf` / `sb::appendf` | Two copies (shield_bar_src cannot include the buff dir). Drift guard: `shield_logic_test` section 8 compares both over 3000+ cases. A `_shared_src/StrFmt.h` needs a KPM `Patches/Common` sync | duplication | ours | follow-up |

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

## Phase 2-3 session (10-03, plan `~/.claude/plans` "Fix the Phase 1 findings, then expand test coverage")
- Commits: `abc01d8` (Phase 2: F1/F7/F9/F10/F11 + `build_check.py`), `1d0ccab` (EquipCharPreview T1), `670905a` (T0 raw addresses + subclassers), then docs.
- **BuffDurationHUD T1** done after the split: `ce37f3e` (64302 checks; F22-F25 from it). Also `d72dffd` (ShieldTable.inc path in the raw-address generated list).
- **Concurrent sessions.** The buff/shield split session (doc 21 W2-W4) rewrote BuffDurationHUD.cpp mid-plan and adopted `bh::appendf`; BuffHUD T1 is held until its W4 commit (it messages this session). The engine-research session's PerfLab 0.4.0 shares PerfLab.cpp; only the F9 hunk was committed here (index-staged against HEAD), and its `e5aa526` carries it.
- **Plan corrections.** "bc34464 stale" was wrong (that commit adds `ForceArrowCounters`). The F10 formula in the plan wrapped. `create-patch.py` does not exclude tests (F13).
- **Red default tier from others' WIP** at session end: `check-hook-spans` (portrait_slots_src mid-split) and `check-raw-addresses` (RobustnessFixes 1.2.0 WIP; owner will `--write-baseline` at its commit).
- **Read-only check:** this session wrote only `letterbox_probe_stage/` (and removed the probe dir) in KPM. Other KPM and `patch_config.toml` changes after the marker are other sessions' (Common headers, PortraitSlots, shield_live_patches, a 12:00 install).
- F2 (stale `CompanionAI-0.1.0-live.kpatch`) stays open: it needs separate approval.
- **F1 regression (10-03 15:29):** `Patches/K2DialogueLetterboxFixProbe/` reappeared, byte-identical; recreated by the engine-bug Phase 4 session (before it knew of F1). Its doc 15e now says never recreate it (`a2fef28`), and the vanilla session dropped the dir from KPM `.git/info/exclude` so a recreation shows in `git status` (`b2e79d0`). It was removed again (same approval) and the letterbox and vanilla sessions were told. KPM validate: 84 patches, 0 errors.
