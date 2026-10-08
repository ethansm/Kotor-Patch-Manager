# Doc 22: update compatibility check (`scripts/compat_check.py`) - plan

Status: Active (U0 schema and decisions frozen; U1 graph builder next). Written 2026-10-03. Related: doc 20 (`20_TESTING_PLAN.md`, section 4.1 "Common contract tests"), doc 22 hotbar shared services (`22_hotbar_shared_services.md` section 6), doc 21 portrait slots (`21_portrait_slots_and_split.md`), doc 19b section D (Common extraction).

## Context

User request (verbatim): "we need some kind of functionality that can test if changes to a common function or shared dll would cause any failing tests indicating a breaking change with patches which consume it. Could we define a contract for a test file or tag/indicator of specific tests in the file to run as part of a update compatibility check? Maybe not a automated step but a tool to use. So if a contributor changes something in common, its easy to trigger a test run checking everything that uses what was touched. OR can that relationship be built at run/command time?"

Answer in one line: **build the relationship at command time** (include graph + manifest graph + git diff, 0.15 s for the whole repo), and add a contract only for the links a scanner cannot see (`covers` in `test.toml` / `tests/compat.toml`). The "tag the compat tests" half of the idea is not needed: a test is in the compat set because it reaches the touched file, not because someone remembered to tag it. The user's existing rule (doc 20 section 4.1: every mod that includes a Common header owns `<mod>_src/common_contract_test.cpp`) is already the consumer-side contract. The tool wires it automatically, and reports where it is missing.

Verdict on the proposed direction: **confirmed, with five refinements** (section "Design review of the proposal" below).

## Research facts

### Runner, builders, checkers (what exists today)
- `scripts/run_tests.py` is the single entry point. Items are `cpp:<stem>` (every `patch_manager_mods/**/*_test.cpp`, `cpp_item` :245), `py:<path>` / `py:<dir>(unittest)` (`**/test_*.py`, :283 `discover`), `suite:<name>` (explicit `tests/suites.toml` rows) and `wine:<smoke>[mode]` (`--wine`). Discovery prunes `PRUNE_DIRS` (:75: `.venv __pycache__ .git node_modules retired testdata fixtures`) and nested git checkouts.
- `-k SUBSTR` is the only selector (:517, :535) and it is a **single substring**: it cannot take a list of exact ids. `--list` prints ids and SKIP reasons, `--json PATH` writes `{wall_seconds, counts, results:[{id, kind, status, seconds, reason, tail}]}`. Status is PASS / FAIL / SKIP / ERROR; exit 1 on any FAIL or ERROR; a missing requirement is a SKIP, not a failure (:128 `missing_requirement`).
- `Item` (:82) carries `id, kind, skip, cwd, env, timeout, build_cmd, cmd` but **not the source path or any metadata**: the runner has no notion of "which code does this test cover".
- `test.toml` (docstring :19-34, merge at :250-252): keys `cxxflags`, `args`, `skip_reason`, top-level = directory default, `[<test_stem>]` table overrides. Unknown keys are ignored (`cfg.get`), so adding `covers` cannot break the runner. Only 3 `test.toml` files exist: `_shared_src/`, `force_power_hotbar_src/`, `portrait_slots_src/`.
- `tests/suites.toml`: 16 `[[suite]]` rows (4 level-up parity, 2 level-up scenarios incl. `--m32`, `guikit`, 8 checkers, `build-check` with `tier = "build"`). Parity suites build `research/companion_levelup/tools/parity/*.cpp` from **python scripts with hard-coded names** (`parity.py:122-127` builds `parity_main`, `apply_test`, `gate_test`, `playthrough_test`, `ui_test`, `unit_gap_test`; `scenarios.py:18` builds `scenario_test`). Their include dirs and `-include stub/m3_win_shim.h` live only inside those scripts.
- `scripts/build_check.py` (`--mod DIR` relative to `patch_manager_mods/`, repeatable; discover :357, run :398, stage_mod :193): builds each mod like KPM `create-patch.py` with `i686-w64-mingw32-g++`, stages a sibling `_shared/` copy of `_shared_src`, checks the DLL export table against the hooks toml. Common library cache is content-keyed under `$TMPDIR/kotor-tests/common-cache`, flock-serialised, so it is safe to share. It does not write the KPM tree.
- Static checkers with a per-mod filter: `check_hook_bytes.py --mod ID`, `check_hook_spans.py --mod ID` (pairs involving ID), `check_exports.py --mod ID`. Whole-repo and cheap: `check_raw_addresses.py`, `check_subclassers.py`, `check_gui_ids.py`, `check_shared_headers.py`, `check_effect_decode.py`. `check-hook-*` read the live `patch_config.toml` and the vanilla exe read-only and SKIP when absent.
- `scripts/check_shared_headers.py`: canonical = `_shared_src/*.h` + `*.inc` (:44); KPM must hold a byte-identical `Patches/Common/<h>` and a forwarder `Patches/_shared/<h>` (:26 `forwarder`); `FOREIGN` (:23) = the level-up parity stub `_shared/` (4 symlinks into `_shared_src`: `GameAddr.h GuiKit.h K2Addr.h SafeRead.h`; the stub does **not** link the other headers). It syncs only headers and `.inc`, **not tests**.
- Static registries under `tests/static/`: `span_exceptions.toml`, `gui_ids.toml` (cites C++ constants via `source_regex`), `subclassers.toml`, `raw_addresses.toml`.
- Doc 20 section 4.1 (line 134): "Common contract tests (user rule, 2026-10-03)": every mod including a `_shared_src` header owns `<mod>_src/common_contract_test.cpp`, messages start `<mod> relies on:`; proposed: a T0 checker warns on a Common include without one, and `check_shared_headers.py --sync` refuses to sync while a contract test is red. Reference: `force_power_hotbar_src/common_contract_test.cpp` (10dfc08, 16 "relies on" blocks, test.toml per-test table with `-I../_shared_src -DGUIKIT_HOST -Werror`).
- Doc 22 section 6 (lines 181-199): interface tests plan: shared `FakeKPatchApi.h` (today inline at `portrait_slots_src/ps_dispatch_test.cpp:58-80`), layout `static_assert`s at -m32, `struct_size` gating, consumer-inert-without-provider, and "`provides` string must equal the `provide()` name in the source (grep check in T0)".
- KPM side (`~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`, branch `ethansm/local-mods`): `docs/PatchRegistry.md` "Library patches" (:273-312): `kind = "library"`, `provides = ["name@major"]`, `requires`, `requires_interfaces` (informational for now; regex `^[a-z0-9_.-]+@[0-9]+$`); `DependencyValidator.ResolveWithLibraries` (`src/KPatchCore/Validators/DependencyValidator.cs:135`) derives the install set from `requires` only. The core's `src/KotorPatcher/include/kpatch_api.h` is a 6-line forwarder to the canonical `Patches/Common/kpatch_api.h` (so the ABI header is a shared file: core + every patch that includes it). KPM CI (`.github/workflows/ci.yml`) already builds **every** patch per target (`publish-patches.py --target`), runs `dotnet test` (8 test classes in `tests/KPatchCore.Tests`, 427 tests per doc 20), `make -C src/KotorPatcher test`, `tools/validate-patches.py`. `Patches/*` have no colocated tests.

### Inventory (computed 10-03 with a throwaway scan over 106 C++ files, 172 include edges; scan time 0.15 s)

Manifests: 21 under `patch_manager_mods/` (one library `portrait-slots`; consumers `buff-duration-hud`, `shield-bar`, both with `requires = ["portrait-slots"]`, `requires_interfaces = ["k2.portraitslots@1"]`; `dialogue_letterbox_fix_probe_src` duplicates the id of `dialogue_letterbox_fix_src`).

Include fan-out of every shared file (reverse transitive closure; "mods" = mod dirs whose `.cpp` is built into a DLL, "tests" = `cpp:` items, "suites" = level-up parity suites, "smokes" = wine smokes whose `.kpatch` is built from an affected mod):

| `_shared_src` file | mods | host tests | parity suites | wine smokes (runs) |
|---|---|---|---|---|
| SafeRead.h | 12 | 7 | 4 | 7 (18) |
| K2Addr.h | 5 | 9 | 4 | 0 |
| GameAddr.h | 5 | 9 | 4 | 0 |
| GuiKit.h | 4 | 4 | 4 | 0 |
| Effects.h | 3 | 2 | 0 | 0 |
| Gui2D.h | 3 | 2 | 0 | 0 |
| PortraitSlots.h | 3 | 1 | 0 | 0 |
| Party.h | 2 | 2 | 0 | 0 |
| EffectIcons.h, EffectIconTable.inc, DebuffIconTable.inc | 1 (buff HUD) | 1 (`effects_test`) | 0 | 0 |

KPM-side (same scan over `Patches/`, resolving `../Common/x.h`, `../_shared/x.h`): the consumer sets are identical to the repo's for every shared header (SafeRead.h 12 patches, K2Addr.h 5, Gui2D.h 3 ...). The KPM `Patches/Common` also holds headers that are **not** ours: `Common.h`, `Platform.h`, `VirtualFunctionCall.h` reach 14 upstream patches each; `GameAPI/*.h` reach 13; `kpatch_api.h` reaches 3 (ours); none of our mods include `Common.h`/GameAPI.

Mods and the tests that reach them by include (0 = nothing reaches the mod's code):
- no reaching test at all (11 of 21): activation_fix, active_effects_panel, dialogue_letterbox_fix(+probe), equip_icon_fix, load_hang_probe, mouse_probe, perf_lab/perflab, ranged_ai, robustness_fixes, texture_memory_release, wrap_measure_fix. Six of them have wine smokes instead (load_hang_probe 3, perf_lab 4, texmem 1, robustness 1, activation_fix 1, equip_icon_fix 1).
- with tests: buff_duration_hud (`buff_hud_logic_test`, plus `shield_logic_test` which includes `../buff_duration_hud_src/BuffHudLogic.h`: a cross-mod include the scan finds for free), companion_ai (1), equipment_3d_character (1), force_power_hotbar (5 + the contract test), inventory_3d_viewport (1), inventory_sort_filter (1), portrait_slots (`ps_dispatch_test`, includes `PsDispatch.h` only, not the provider `.cpp`), shield_bar (1), level_up_override (5 parity/scenario suites, which include the real `LevelUpOverride.cpp`).
- **Contract-test gap:** 12 mods include a `_shared_src` header; **1** (force_power_hotbar) owns a `common_contract_test.cpp`. 11 gaps vs the doc 20 rule.

Tests that cover code **without including it** (need `covers`, or a naming/import rule):
1. Python checkers and their tests: `scripts/test_*.py` (11) test `scripts/*.py` by import (`_static_common.py` has 12 python dependents: 6 checkers + 6 tests). A python import graph (`ast`, 220 .py files) finds these; no `covers` needed.
2. Static checkers that read the mods as **text** (`check_raw_addresses`, `check_subclassers`, `check_gui_ids` with `source_regex` citations, `check_hook_*`, `check_exports`): covered by a file-class table (section Design), not by includes.
3. Level-up parity twins (`research/companion_levelup/tools/*_twin.py`, `levelup_rules.py`, `levelup_paths.ini`, `tests/luo_scenarios/saves.toml`): compared against the C++ by `parity.py`; needs `covers` on the suites. `scripts/test_luo_scenario.py`, `test_luo_log_check.py`, `test_kotor_save_forge.py` test scripts that read `saves.toml` (not C++).
4. Wine smokes (`harness/smokes.toml`): each row names a `.kpatch` (`kpatch = "PerfLab/PerfLab.kpatch"`), which maps to a mod via KPM `Patches/<Name>/manifest.toml` id. **They test the prebuilt `.kpatch`, not the source**, so after a header change the smoke runs the old binary and passes. This is a real hole: see Risks and unit U8.
5. `gen_*.py` generators write `.inc` tables (`gen_buff_hud_icon_table.py` -> `_shared_src/EffectIconTable.inc`, `gen_debuff_icon_table.py`, `gen_hotbar_textures.py`, `gen_equip3d_weapon_table.py`, ...): the script-to-`.inc` edge is only in docstrings.
6. Command-line includes the scanner cannot see: `-include stub/m3_win_shim.h` (parity.py:30, scenarios.py:42), `-I stub/inc` (makes `../_shared/X.h` resolve to `stub/_shared/`), `-I ../_shared_src` (test.toml). Handled by the resolver's fixed search path (below), not per-test flags.

Edge cases found in the tree (all handled in Design): one conditional include (`PortraitSlots.h:27` under its include guard), no macro-computed includes, symlinks (`stub/_shared/*.h` -> `_shared_src`; resolve with realpath so the symlink and its target are one node), `.inc` files included from headers (`EffectIcons.h` -> two `.inc`), a header that exists in both `_shared_src` and `KPM Patches/Common`, `<windows.h>` resolving to the parity **stub** for host tests but to MinGW's header for DLL builds.

### Speed (estimates from doc 20 Status; the suite was not run for this plan)
- Whole default `run_tests.py`: 53 items in about 16 s wall; `build-check` tier: cold Common library about 13 s once, then about 8 s for all 21 mods (so 1-3 s per mod warm); `--wine`: 83 items in about 172 s (59-item set: 136 s); one `parity.py` run about 8.4 s; level-up mutation gate about 10 min (never part of compat).
- Estimated compat runs: `Gui2D.h` about 15 s (3 builds, 2 host tests, 2 static); `SafeRead.h` about 35-45 s without wine (12 builds, 7 host tests, 4 parity suites), about 3 min with the 7 affected smokes; `PortraitSlotsProvider.cpp` about 10 s. All well under the 60 s target of doc 20 4.1 except the wine tier, which stays opt-in.

## Design review of the proposal

1. **Command-time derivation is right.** 0.15 s to scan, no registry to rot, and the three real gaps (11 mods without a contract test, 11 mods with no reaching test, smokes on stale binaries) are discovered by the same scan instead of being hidden by a hand-maintained list. A committed relationship file would already be stale after any session that adds one `#include` (the portrait-slots split just did).
2. **Drop `tags = ["compat"]`.** A tag has to be remembered; reachability does not. Two roles are enough and both are conventions: a test file named `*_contract_test.cpp` inside a mod dir is that mod's **consumer contract** (report line: "BREAKING for <mod>"); anything else that fails is a "self/downstream" failure. `covers` is the only contract key, used only for links the scanner cannot see.
3. **Selection must be conservative.** The danger is a false negative. Rules: include over-approximation is fine (macros and `#if 0` ignored); an unclassifiable touched file under `patch_manager_mods/` prints `UNCLASSIFIED` and (with `--strict`) fails; `--all` is a one-flag escape hatch that runs the whole `run_tests.py --build`.
4. **Host tests do not replace DLL builds.** Host tests compile with g++ + the parity stub (`-DGUIKIT_HOST`); the DLL compiles with MinGW and real `windows.h`. Both are in the affected set: `build_check --mod` for every mod reached.
5. **Interface (runtime) breaks need their own gate.** A struct layout or phase-semantics change in `PortraitSlots.h` compiles in provider and consumers alike (same header) and only breaks at run time when a live provider and consumer disagree. The include scan finds who to rebuild; it cannot find the break. Hence the ABI golden + consumer contract tests (section "Interface contract tests").

## Design

### Graph builder (`scripts/compat_graph.py`, importable, no side effects)

Nodes are repo-relative paths after `realpath`. External nodes are `kpm:Patches/Common/<file>` (read-only, from `$KPM_DIR`). Edges, all derived on each run:

| # | Edge | Source of truth | Notes |
|---|---|---|---|
| E1 | C++ `#include "x"` / `<x>` | regex `^\s*#\s*include\s*([<"])([^>"]+)` over `.cpp .h .hpp .inc` under `patch_manager_mods/`, `research/companion_levelup/`, `tests/` | resolution order below |
| E2 | Python `import` | `ast` over `.py` under `scripts/`, `tests/`, `patch_manager_mods/`, `research/` | bare module name -> unique file with that stem |
| E3 | manifest library edge | `manifest.toml`: `requires`, `requires_interfaces`, `provides`, `kind` | library touched -> every manifest whose `requires` contains its id (or whose `requires_interfaces` names a provided interface); also the reverse for `--why` output |
| E4 | test -> code, non-include | (a) colocation: a test in a mod dir covers that dir's non-test sources; (b) suite scripts: `.cpp` literals inside the python scripts named in a suite's `cmd` (`parity.py` etc.); (c) `smokes.toml` `kpatch` -> KPM manifest id -> mod dir; (d) `covers` | (a) does not apply inside `_shared_src` (E1 is precise there) |
| E5 | generator -> table | `[[generator]]` rows in `tests/compat.toml`, or docstring regex `(\w+\.inc)` in `scripts/gen_*.py` | touching a `.inc` with a generator prints INFO "regenerate, do not hand-edit" |
| E6 | file class -> static checkers | built-in table (below), overridable in `tests/compat.toml` | |

Include resolution (matches the three build profiles that exist today, in order, first hit wins): the including file's directory; `_shared_src` (also for any `../_shared/<x>`, which is how mod sources spell it for KPM); `$KPM_DIR/Patches/Common` and `.../GameAPI` (external, read-only; this is where `kpatch_api.h` lives); for roots under `research/` or `tests/` only: `stub/`, `stub/inc/`, `tests/include`. Angle includes resolve only through the stub rule and only count for host roots (so touching `stub/windows.h` does not drag in MinGW DLL builds). Unresolved quoted includes are kept per file; if a **deleted or renamed** header's basename matches an unresolved include, the includer is added with reason `includes deleted <h>` (the build will then fail, which is the correct red).

Roots and what they map to:
- `patch_manager_mods/**/*_test.cpp` -> `cpp:<stem>` (same dup rule as `run_tests.py`: `cpp:<relpath>` on a stem collision);
- `research/**/parity/*_test.cpp` and `parity_main.cpp` -> the suite whose script text names the file (E4b);
- a non-test `.cpp` in a mod dir -> `build_check --mod <relpath under patch_manager_mods>`; a mod dir = nearest ancestor with `manifest.toml` (this is how `perf_lab_src/perflab` is found).

Reverse closure is a plain BFS with one flag: a path that crossed a stub-resolved angle include is valid only for host roots.

### File-class -> checker table (E6, built in; the part scans cannot know)

| Touched file | Run |
|---|---|
| `_shared_src/*.h`, `*.inc` | `suite:check-shared-headers` (drift of Common copy + forwarder), `suite:check-raw-addresses`; **KPM-side consumer list** printed (section Upstream) |
| any `*.cpp/.h/.inc` under a mod dir | `suite:check-raw-addresses`, `suite:check-subclassers` |
| `kotor2-*.hooks.toml`, `code_patches.toml` | `build_check --mod` (exports vs hooks), `check_hook_bytes --mod <id>`, `check_hook_spans --mod <id> [--mod <consumer ids>]`, `check_exports --mod <id>`, `suite:kpm-validate-patches` |
| `exports.def` | `build_check --mod`, `check_exports --mod` |
| `manifest.toml` | `suite:kpm-validate-patches`, manifest graph re-derived, `--lint` |
| `tests/static/*.toml` | the one checker that reads it |
| `GameAddr.h`, `K2Addr.h` | as `_shared_src` + `check_hook_bytes` for **all** mods (hooks are raw addresses today, so this is cheap insurance, 120 s timeout in suites.toml, typically seconds) |
| `scripts/*.py` | python tests found by E2 |
| anything else | `UNCLASSIFIED` |

Game-version scoping (optional, unit U11): hooks files are named by version (`kotor2-steam-aspyr.hooks.toml`, no GOG file exists), and `K2Addr.h` has a `kSteam` and a `kGog` initializer (`K2Addr.h:268`, `:271`). If the diff hunks of `K2Addr.h` touch only the `kGog` initializer, print "GOG-only: Steam runtime unchanged" and narrow the run to `cpp:k2addr_test` plus a compile-only `build_check` of the 5 consumers. Default stays conservative (full fan-out).

### Contract schema (the minimum that is not derivable)

```toml
# patch_manager_mods/<dir>/test.toml, inside a per-test table (or as a dir default)
[hotbar_bugs_test]
covers = [
    "force_power_hotbar_src/ForcePowerHotbar.cpp",   # repo-relative globs (relative to patch_manager_mods/ or repo root)
    "k2.portraitslots@1",                           # interface string name@major (matched against provides / requires_interfaces)
]
```

```toml
# tests/compat.toml  (new; only for items that have no natural toml: py tests, parity suites, generators)
[[covers]]
id = "suite:levelup-parity"              # item id as printed by run_tests.py --list
paths = ["research/companion_levelup/tools/**", "research/companion_levelup/levelup_paths.ini"]

[[generator]]
script = "scripts/gen_debuff_icon_table.py"
writes = ["patch_manager_mods/_shared_src/DebuffIconTable.inc"]
```

Conventions (no key needed):
- `*_contract_test.cpp` in a mod dir = consumer contract of that mod.
- `covers` entries are verified by `--lint` (a glob that matches nothing is a WARN, same convention as the stale-entry WARNs in `check_raw_addresses`).
- Failure in a consumer contract test prints: `BREAKING for <mod>: <first "relies on:" line>`.

`run_tests.py` change that makes this possible: `Item` gains `src` (the `.cpp`/`.py` path), `covers` and `consumer`; new `--id ID` (repeatable, exact) and `--ids-file`. `-k` stays as is. `discover()` is imported by `compat_check.py`, so the ids are guaranteed identical to the runner's.

### CLI

```
python3 scripts/compat_check.py patch_manager_mods/_shared_src/Gui2D.h          # explicit files (recommended)
python3 scripts/compat_check.py --since main                                    # git diff main..worktree incl. untracked
python3 scripts/compat_check.py --since HEAD -- patch_manager_mods/_shared_src  # restrict by pathspec
python3 scripts/compat_check.py Gui2D.h --list          # print the plan, run nothing (default for --why)
python3 scripts/compat_check.py Gui2D.h --why           # print the edge chain per selected item
python3 scripts/compat_check.py Gui2D.h --json out.json # machine-readable plan + results
python3 scripts/compat_check.py Gui2D.h --wine          # add affected wine smokes (needs U8 to be meaningful)
python3 scripts/compat_check.py --lint                  # covers staleness, provides/IFACE equality, contract-test gaps
python3 scripts/compat_check.py --all                   # escape hatch: run_tests.py --build
```

Flags: `--jobs N`, `--no-sanitize`, `--strict`, `--shared-tmp` (default: private `TMPDIR=<tmp>/compat-<pid>` for host builds so a concurrent `run_tests.py` cannot collide on `$TMPDIR/kotor-tests/host/<stem>`; the flock-guarded Common cache is shared via `build_check`'s `cache_root`), `--kpm-dir`, `--repo`.

Touched-file sources: positional args; `--since REF` = `git diff --name-status -M -z REF` plus `git ls-files -o --exclude-standard`; `--staged`. Renames contribute both paths (old path = "deleted" semantics, new path = "added"); deletions are never read from disk.

Output (one line per action, grouped; reasons are the shortest edge chain):

```
touched: patch_manager_mods/_shared_src/Gui2D.h
BUILD   build_check --mod buff_duration_hud_src --mod portrait_slots_src --mod shield_bar_src   (includes Gui2D.h)
TEST    cpp:gui2d_test        (self: includes Gui2D.h)
TEST    cpp:k2addr_test       (includes Gui2D.h)
STATIC  suite:check-shared-headers, suite:check-raw-addresses
KPM     consumers after sync: BuffDurationHUD PortraitSlots ShieldBar
GAP     buff-duration-hud, shield-bar, portrait-slots include Gui2D.h but own no *_contract_test.cpp (doc 20 4.1)
...
RESULT  PASS 7  FAIL 0  SKIP 0   (unverified: 0)   wall 14.8 s
```

Exit codes:
- `0`: All selected items passed (SKIPs listed as `unverified`).
- `1`: Any FAIL or ERROR in selected items (a failing `*_contract_test` additionally prints `BREAKING for <mod>: <first "relies on:" line>`).
- `2`: Tool/usage error (bad ref, git failure, unreadable toml, invalid arguments).
- `3`: `--strict` only: UNCLASSIFIED files, GAPs or unverified items present.

`--json` schema: `{touched:[{path,status}], plan:[{kind,id,why:[...],cmd:[...]}], gaps:[...], unclassified:[...], results:[{id,status,seconds,reason,tail}], counts, wall_seconds}`; `results` reuses the exact `run_tests.py --json` record shape.

Execution: one `run_tests.py --id ... --id ... --build` call for tests/suites (so output, SKIP semantics and JSON are the runner's), then `build_check --mod ...`, then the per-mod checkers. The tool never runs the mutation gate, never touches the game dir (the `check-hook-*` suites read the live config read-only, as they do in the normal runner), and never writes to the KPM tree.

## Worked examples (computed today with a throwaway scan; the real tool must reproduce these)

### A. Touch `patch_manager_mods/_shared_src/Gui2D.h`
- Include reach: 1 intermediate file (itself); **3 mods** `buff_duration_hud_src`, `portrait_slots_src`, `shield_bar_src` (all three are in the portrait-slots library/consumer set: library -> consumers adds nothing new here).
- Host tests (**2**): `cpp:gui2d_test`, `cpp:k2addr_test` (it includes `Gui2D.h` to test fail-safe behaviour). Parity suites: 0. Wine smokes: 0.
- Static (**2**): `suite:check-shared-headers`, `suite:check-raw-addresses`.
- KPM side after `check_shared_headers.py --sync`: **3** patches (`BuffDurationHUD`, `PortraitSlots`, `ShieldBar`), identical to the repo set.
- Gaps reported: 3 consumers without a contract test.
- Total actions: 3 builds + 2 tests + 2 static = **7 items, about 15 s**.

### B. Touch `patch_manager_mods/portrait_slots_src/PortraitSlotsProvider.cpp`
- Include reach (E1): 1 mod (`portrait_slots_src` build), **0 tests** (`ps_dispatch_test` includes `PsDispatch.h` and `PortraitSlots.h`, not the provider `.cpp`: the include scan alone misses it).
- Colocation (E4a) adds `cpp:ps_dispatch_test`.
- Manifest edge (E3): `portrait-slots` is `kind = "library"` -> consumers `buff-duration-hud` and `shield-bar` -> 2 more builds and their colocated tests `cpp:buff_hud_logic_test`, `cpp:shield_logic_test` (cheap, same-dir).
- Interface edge: any test with `covers = ["k2.portraitslots@1"]` (none exist today -> **GAP**: the consumers need contract blocks, unit U6).
- Static: `check-raw-addresses`, `check-subclassers`; hooks file untouched, so no hook checkers.
- Total: **3 builds + 3 tests + 2 static = 8 items, about 10 s**, plus 1 interface GAP line.

### C. For scale: `patch_manager_mods/_shared_src/SafeRead.h`
12 builds (every mod that includes any Common header), 7 host tests (`saferead_test`, `common_contract_test`, `effects_test`, `gui2d_test`, `guikit_test`, `k2addr_test`, `party_test`), 4 parity suites (`levelup-parity`, `levelup-scenarios`, `levelup-scenarios-m32`, `guikit`), 7 wine smokes (18 runs) behind `--wine`; static 2; KPM side 12 patches; 11 contract GAPs. About 40 s without wine.

## Interface contract tests (runtime breaks the scanner cannot see)

Today's only provided interface is `k2.portraitslots@1` (`_shared_src/PortraitSlots.h`: `K2PsContext`, `K2PortraitSlotsApi`, phases 0-5, priorities, `struct_size` first, structs only grow). Provider = `portrait_slots_src/PortraitSlotsProvider.cpp`; consumers = buff HUD and shield bar via `k2ps::Client`.

1. **ABI golden file** (provider side, automatable). `tests/abi/<name>@<major>.toml` records `sizeof` and `offsetof` of every field of every interface struct (compiled at **-m32**, the DLL width), the `#define` constants (`*_IFACE`, `*_VERSION`, phase numbers, `MAX_SLOTS`) and the manifest `provides` string. `scripts/abi_check.py` (with a tiny `abi_dump.cpp` per interface) regenerates and diffs. Rules: within a major the golden must be an **append-only prefix** of the new layout (new fields at the end, `struct_size` grows, existing offsets and constants unchanged); anything else needs a new `@major` provided side by side (PatchRegistry.md "Names, versions and growth"). A red golden prints which field moved and which consumers read it (from `requires_interfaces`).
2. **Consumer contract blocks** (consumer side). Each consumer's `*_contract_test.cpp` gets a block per interface it uses: `static_assert(offsetof(K2PortraitSlotsApi, SlotAt) == N)` for every field it reads, the minimum `struct_size` it accepts, and the phase constants it registers for; messages start `<mod> relies on:` (doc 20 4.1 convention). `covers = ["k2.portraitslots@1"]` in the mod's `test.toml` makes the compat tool select it when the header, the provider or `provides` changes.
3. **Behavioural contract with a fake registry** (doc 22 section 6): `FakeKPatchApi.h` (extract from `ps_dispatch_test.cpp:58-80`), then host tests for `struct_size` gating (shorter table = absent), consumer inert without provider, init-order independence, `require` after close, re-entrancy. Already planned by the split session; this plan only asks that the tests carry `covers` so they join the compat set.
4. **Skew matrix** (cheap, high value): build the consumer against the **new** header and feed it a provider table of the **old** `sizeof` (golden's previous size); the consumer must treat it as absent (`usable()` in `PortraitSlots.h`) rather than read past the table.
5. **Lint** (`--lint`): manifest `provides` equals `<PREFIX>_IFACE@<PREFIX>_VERSION` found in the header (regex `#define (K2\w*)_IFACE "(...)"` + same-prefix `_VERSION`); every manifest `requires_interfaces` has at least one test whose `covers` names it, else GAP.

Honest limit: nothing here proves a **live** provider and consumer built from different commits agree (that needs the installed DLLs). The skew matrix and the golden together make that case unreachable without bumping the major, which is the actual rule.

## Upstream / KPM variant

Upstream contributors have no KOTOR-II repo: their relationship is `Patches/Common` -> `Patches/<Name>`, plus the .NET core and the registry ABI. CI already builds every patch dir (about 85 with sources or manifests), so a contributor gets the answer late. Proposal (a **separate, later** PR candidate; draft in `issue_drafts/`, never posted without the user's chat approval per CLAUDE.md):

`tools/affected-patches.py` in the KPM repo, stdlib only, **Python 3.9 compatible** (CI uses 3.9: `tomllib` with the same fallback `validate-patches.py` uses, or a small regex for `id/kind/requires/provides`):
- `affected-patches.py [--since upstream/master | files...] [--json]` prints: patches whose sources include a touched `Patches/Common/*` header (same resolver: patch dir, `../Common`, `../_shared`, `Common/GameAPI`; `Foo.cpp` in Common implies `Foo.h`); library -> consumers via manifest; `kpatch_api.h` -> the core too.
- `--run` executes the class-appropriate commands: `python3 Patches/create-patch.py -o <tmp>` in each affected patch dir (build only, no install), `python3 tools/validate-patches.py`, `dotnet test --filter` for `src/KPatchCore/**` (or `ShippedHooksTests` for hooks/manifest edits), `make -C src/KotorPatcher test` for `src/KotorPatcher/**` or `Patches/Common/kpatch_api.h`.
- Change classes that scans cannot resolve, listed explicitly: `AddressDatabases/*.db` rows (consumers refer to rows by name at run time; recommend `SqliteTools validate` and list GameAPI-using patches), `GameAPI/*.cpp` (linked into every Windows module: build the 13 GameAPI-including patches).
- No contract file is proposed upstream; the only convention is a `*_contract_test` note for libraries, optional. Patches there have no colocated tests, so the tool is a "build and validate the blast radius" tool, not a test selector.

Where our tool meets KPM: when a touched file is in `_shared_src`, `compat_check.py` prints the KPM-side consumer list (same scan over `$KPM_DIR/Patches`) and runs `check_shared_headers.py` read-only to say whether the sync is pending. It never syncs by itself. Proposed (doc 20 4.1, user rule): `check_shared_headers.py --sync` refuses while the selected contract tests are red; implemented as a one-line call into `compat_check.py --list --json` plus `run_tests.py --id`.

## Batch table

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| U0 | Freeze schema and decisions | - | - | DONE | | schema and 5 open decisions frozen |
| U1 | Graph builder (`scripts/compat_graph.py` + tests) | U0 | - | DONE | 40b47d2 | compat_graph.py + test suite 12/12 PASS |
| U2 | Runner hooks (`run_tests.py` Item.src/covers/consumer, `--id`, `--ids-file`) | U0 | doc 20 owner | DONE | 0b91725 | run_tests.py Item.src/covers/consumer, --id, --ids-file, --list --json; 12/12 PASS |
| U3 | `compat_check.py` CLI and file-class table | U1, U2 | - | DONE | f633a1f | U12 |
| U4 | Consumer contract gaps, ranked (`shield_bar`, `buff_hud`, `portrait_slots`, `luo`) | U3 | - | DONE | 7ae5119 | U5 |
| U5 | `covers` annotations for what scan cannot see (`tests/compat.toml`, `test.toml`) | U3 | - | DONE | 8d17773 | U7 |
| U6 | Interface contract tests and ABI golden (`abi_check.py`, golden diff) | U3, U4 | split session | DONE | 3de985d | U7 |
| U7 | `--lint` (stale covers, missing contract tests, unreached mods) | U3, U5 | - | DONE | b756482 | Implemented --lint checks (stale covers, provides/header validation, contract gaps, unreached mods, unclassified files); test_compat_check 19/19 PASS |
| U8 | Wine smokes on fresh DLL (`dll_from = "build"`, optional) | U3 | - | DONE | ea85883 | Wine smokes on fresh DLL with dll_from='build', stale kpatch detection and skip/unverified reporting; 71/71 tests PASS |
| U9 | KPM variant (`tools/affected-patches.py`, Python 3.9) | U1, U3 | user GO | DONE | f1d0a03 | KPM affected-patches.py and test_affected_patches.py on feat/affected-patches (7f05ef7); draft in patch_manager_mods/issue_drafts/pr_affected_patches.md; 7/7 tests PASS; validate-patches green |
| U10 | Docs and rules (`20_TESTING_PLAN.md`, `CLAUDE.md` / `AGENTS.md`) | U3, U7 | user GO | DONE | e5d32e1 | Documented compat_check.py in 20_TESTING_PLAN.md §4.4 + status line, CLAUDE.md, and AGENTS.md |
| U11 | Version scoping (GOG-only `K2Addr.h` narrowing, optional) | U3 | - | DONE | ecd0e33 | GOG-only K2Addr.h narrowing to cpp:k2addr_test + compile-only builds; 29/29 tests PASS |
| U12 | Proof by deliberate breakage (`SafeRead`, `Gui2D`, `PortraitSlots`, `PsDispatch`, stub) | U3, U4 | - | DONE | a734895 | Proof by deliberate breakage suite (test_compat_breakage.py 5/5 PASS; SafeRead, Gui2D, PortraitSlots, PsDispatch, stub) |

### U0 - freeze schema and decisions (Opus, 20 min)
- **Target files:** this doc only.
- **Objective:** answer the five open questions, fix the `covers`/`compat.toml` schema and exit codes.
- **Guardrails:** no code.
- **Verification:** doc 22 status line updated.

### U1 - graph builder (Sonnet, 3 h)
- **Target files:** `scripts/compat_graph.py` (new), `scripts/test_compat_graph.py` (new; auto-discovered), fixture trees built in `tempfile` inside the test (no new `testdata`).
- **Objective:** E1 + E2 + E3 + E4(a,b,c) + E5; deterministic `affected(paths) -> Plan` with edge-chain reasons; resolver per section Design; realpath for symlinks; deleted/renamed handling; per-root stub rule.
- **Guardrails:** read-only; never imports `run_tests.py` internals other than `discover`; no network; must not read `$KOTOR2_ROOT`; scan of the real repo under 1 s; unresolvable includes never raise.
- **Verification:** `python3 scripts/test_compat_graph.py` (cases: symlink node identity, `../_shared/` mapping, stub-only angle include, cross-mod include `shield_logic_test -> BuffHudLogic.h`, deleted header, rename, `.inc` chain, manifest library fan-out, `perf_lab_src/perflab` nested mod key). Plus a golden check that today's repo yields the section C numbers for `SafeRead.h` and the section A/B lists.

### U2 - runner hooks (Sonnet, 1.5 h)
- **Target files:** `scripts/run_tests.py`, `scripts/test_run_tests_select.py` (new, small).
- **Objective:** `Item.src/covers/consumer`; read `covers` from `test.toml`; `--id` (repeatable exact) and `--ids-file`; `--list --json` prints the item table including `src` and `covers`.
- **Guardrails:** default behaviour and ids byte-identical; `-k` unchanged; do not run the full suite while another session is testing (use `--list` and `-k selftest`); coordinate with the doc 20 owner before editing.
- **Verification:** `python3 scripts/run_tests.py --list` output unchanged except the new JSON option; `python3 scripts/run_tests.py --id cpp:party_test --id cpp:gui2d_test`; `python3 scripts/run_tests.py --selftest`.

### U3 - `compat_check.py` CLI and file-class table (Sonnet, 3 h)
- **Target files:** `scripts/compat_check.py` (new), `tests/compat.toml` (new, may start nearly empty), `scripts/test_compat_check.py` (new).
- **Objective:** git diff intake (`--since`, `--staged`, positional; rename/delete/untracked), plan printing, `--list/--why/--json/--strict/--all`, execution via `run_tests.py --id` + `build_check --mod` + per-mod checkers, private `TMPDIR`, exit codes 0/1/2/3, `BREAKING for <mod>` line for failed `*_contract_test`.
- **Guardrails:** no `git add/checkout/stash`, ever (read-only git: `diff`, `ls-files`, `rev-parse`); never launches the game; does not run `--write-baseline`; does not call `check_shared_headers.py --sync`; `--since` default refuses to run if more than 25 touched files unless `--yes` (the shared worktree is dirty: see Risks).
- **Verification:** `python3 scripts/compat_check.py patch_manager_mods/_shared_src/Gui2D.h --list` must print exactly the section A items; same for the `PortraitSlotsProvider.cpp` section B items; `python3 scripts/test_compat_check.py`; `python3 scripts/compat_check.py patch_manager_mods/_shared_src/Gui2D.h` runs green on a clean tree; then hand-break (below).

### U4 - consumer contract gaps, ranked (Sonnet drafts, Opus reviews, 1 mod per hour)
- **Target files:** `<mod>_src/common_contract_test.cpp` + `test.toml` per mod, in this order: `shield_bar_src`, `buff_duration_hud_src`, `portrait_slots_src`, `level_up_override_src` (already has the parity suites; contract = thin), then the rest by churn.
- **Objective:** close the doc 20 4.1 rule; each file mirrors `force_power_hotbar_src/common_contract_test.cpp` (messages `<mod> relies on:`, red-proven once per block on a scratch copy of `_shared_src`, as 10dfc08 did).
- **Guardrails:** mods whose doc says "awaiting user live test" or whose sources are dirty from another session (`buff_duration_hud_src`, `shield_bar_src`, `portrait_slots_src` are dirty today) wait for that owner; tests only, no mod edits.
- **Verification:** `python3 scripts/run_tests.py -k common_contract`; `python3 scripts/compat_check.py --lint` shows the gap count falling from 11.

### U5 - `covers` annotations for what the scan cannot see (Haiku mechanical, Opus review, 1 h)
- **Target files:** `tests/compat.toml`, `patch_manager_mods/*/test.toml`.
- **Objective:** `covers` for the four level-up parity suites and the three `luo` python tests; the 8 non-include static checkers need nothing (file-class table); generators in `[[generator]]`.
- **Guardrails:** globs must match at least one file (`--lint` enforces); do not restructure `tests/suites.toml`.
- **Verification:** `python3 scripts/compat_check.py research/companion_levelup/tools/levelup_rules.py --list` selects the parity suites; `--lint` clean.

### U6 - interface contract tests and ABI golden (Sonnet, 3 h; coordinates with the portrait-slots split session)
- **Target files:** `scripts/abi_check.py`, `scripts/test_abi_check.py`, `tests/abi/k2.portraitslots@1.toml`, `tests/abi/abi_dump_portraitslots.cpp`; consumer blocks in the U4 files; `FakeKPatchApi.h` only if the doc 22 owner has not extracted it yet (else wait).
- **Objective:** golden diff at -m32, append-only rule, `provides` vs header string lint, skew matrix.
- **Guardrails:** `PortraitSlots.h` and `portrait_slots_src/*` are owned by the split session: read-only for us; goldens are generated from committed headers only.
- **Verification:** `python3 scripts/abi_check.py` green; scratch copy with a swapped field order is red naming the field and consumers; scratch copy with an appended field is green.

### U7 - `--lint` (Sonnet, 1.5 h)
- **Target files:** `scripts/compat_check.py` (lint mode), tests in `scripts/test_compat_check.py`.
- **Objective:** stale `covers`, contract-test gaps (mods including Common without `*_contract_test.cpp`), `provides` vs header string, manifests with `requires_interfaces` but no covering test, mods with no reaching test and no smoke, `UNCLASSIFIED` file kinds. Output is GAP/WARN, never FAIL (exit 3 only with `--strict`).
- **Guardrails:** report-only; no baseline writer.
- **Verification:** `python3 scripts/compat_check.py --lint` prints today's 11 contract gaps and the 11 un-reached mods; adding a stale `covers` makes it WARN.

### U8 - wine smokes on a fresh DLL (Sonnet, 2 h; optional but closes the stale-binary hole)
- **Target files:** `patch_manager_mods/perf_lab_src/harness/smokes.toml` (new key), `scripts/run_tests.py` (`smoke_build`, `_extract_dll`).
- **Objective:** `dll_from = "build"` makes the smoke use the DLL `build_check --keep` produced from current sources instead of the `.kpatch`; default unchanged. Without it `compat_check --wine` prints `STALE: <kpatch> older than <newest affected source>` and marks the smoke `unverified`.
- **Guardrails:** never write to the KPM tree; never launch the game; wine prefix stays private under `$TMPDIR/kotor-tests/`.
- **Verification:** `python3 scripts/run_tests.py --wine -k texmem` unchanged; with the new key a deliberate edit of `TextureMemoryRelease` sources is seen by the smoke.

### U9 - KPM variant (Sonnet, 3 h; Opus review; local first)
- **Target files:** `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/tools/affected-patches.py` on a **new worktree branch cut from master** (`git -C $K worktree add ../kpm-pr-affected -b feat/affected-patches master`), plus an xunit-free python test file beside it.
- **Objective:** section Upstream. Draft the PR text in `issue_drafts/`.
- **Guardrails:** no push, no PR, no comment without explicit chat approval; Python 3.9; no dependency on the KOTOR-II repo; do not edit `ethansm/local-mods`.
- **Verification:** `python3 tools/affected-patches.py Patches/Common/Gui2D.h --list` shows 3 patches; `python3 tools/validate-patches.py` still green.

### U10 - docs and rules (Haiku, 30 min)
- **Target files:** `patch_manager_mods/20_TESTING_PLAN.md` (section 4.4 "Update compatibility check" + Status line), `CLAUDE.md` / `AGENTS.md` (one bullet: "after touching `_shared_src` or a library, run `python3 scripts/compat_check.py <files>`").
- **Guardrails:** CLAUDE.md edit only after the user approves the tool; no memory-file edits by subagents.
- **Verification:** `git diff --stat` shows only those files.

### U11 - version scoping (Sonnet, 1 h, optional)
- **Target files:** `scripts/compat_check.py`.
- **Objective:** the GOG-only `K2Addr.h` narrowing in section Design.
- **Guardrails:** default stays full fan-out.
- **Verification:** a scratch diff touching only the `kGog` initializer yields `cpp:k2addr_test` + compile-only builds.

### U12 - proof by deliberate breakage (Opus, 1 h; the doc 20 "prove it red" rule)
Run in a scratch copy (`cp -r` of `_shared_src` + one mod to a `mktemp -d`, never the live tree):
1. break `SafeRead.h` (flip a clamp): expect `cpp:common_contract_test` red with `BREAKING for force-power-hotbar`;
2. break `Gui2D.h` (rename a symbol buff HUD uses): expect `build_check` red for the 3 mods;
3. swap two fields in `PortraitSlots.h`: expect ABI golden red;
4. delete `PsDispatch.h`: expect `ps_dispatch_test` and the provider build red with reason `includes deleted`;
5. edit `stub/windows.h`: expect only host roots, no DLL build.

## Order and estimates

Critical path (MVP that answers the user's question): U0 20 min -> U1 3 h -> U2 1.5 h -> U3 3 h -> U12 (items 1, 2, 4) 40 min = **about 8.5 h of agent time**, commit per unit. Then U4 (per mod 1 h, 12 mods, only 1 done), U5 1 h, U7 1.5 h, U6 3 h, U8 2 h, U10 0.5 h. Optional last: U9 3 h, U11 1 h. Full plan about 3 working days; the MVP can be used from the first afternoon, and it already prints the gap list that drives U4.

Parallelism: U1 and U2 are independent (different files); U4, U5 and U7 are independent after U3; U6 waits on the split session; U9 is independent of everything but needs the KPM master worktree.

## Frozen decisions (resolved in U0)

1. **Dirty shared worktree:** Default invocation requires explicit file arguments (positional). If `--since` is used and touched files count exceeds 25, the tool refuses to run unless `--yes` is passed.
2. **Enforcement boundary:** Standalone CLI tool (`compat_check.py`) + sync refusal in `check_shared_headers.py --sync` if selected contract tests fail. No automated git pre-commit hook.
3. **Wine smokes:** Smokes are strictly opt-in via `--wine`. Without U8 (`dll_from = "build"`), smokes report as `unverified` (SKIP). U8 enables testing against fresh build DLLs.
4. **Contract keys and schema:** Use `covers` array in `test.toml` and `tests/compat.toml`. Use filename convention `*_contract_test.cpp` as the consumer contract marker. Drop `tags = ["compat"]`. Missing contract tests are reported as GAPs in `--lint` and plan output; U4 prioritized for high-churn mods.
5. **KPM variant:** Maintain local tool first. `tools/affected-patches.py` will be developed on isolated branch `feat/affected-patches` under KPM repo as a candidate PR draft (U9), requiring explicit user conversational chat approval before any PR creation or push. In this repo, `compat_check.py` prints KPM consumer lists when `_shared_src` headers change.

- [x] AR1 (completed)
