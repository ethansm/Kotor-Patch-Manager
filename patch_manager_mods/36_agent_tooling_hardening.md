# 36 — Agent Harness, Tooling Ergonomics, and Gate Hardening Plan

**Status**: PROPOSED / ACTIVE  
**Tracking**: [24_master_plan_status.md](24_master_plan_status.md) (Track H: H1–H5)  
**Root Cause Reference**: `scratch/opencode_export_solo_install.json` & `ses_ee28042a5ffebPgxcExrV5Y6w5.txt` audit  

---

## 1. Executive Summary & Goals

During the multi-batch solo installation run (2026-10-08), OpenCode / Nemotron 3 Ultra encountered 7 systematic failure modes across tool lookup, gate deadlocks, casing collisions, and verification bypasses:
1. `kpm_hotswap.py --prepare` crashed on nested manifests (`perf_lab_src/perflab/manifest.toml`).
2. `kpm_hotswap.py --prepare` exited with code 1 (`FAILED`) when hook changes resulted in `REINSTALL`, falsely signaling a build failure.
3. `pre_install_gate.sh` failed at Step 1 (`check-exports`) because the existing KPM `.kpatch` was stale, forcing the model to rationalize gate failures.
4. Capitalization mismatch (`save-events` vs `SaveEvents`) caused duplicate patch ID errors in KPM schema validation.
5. Goal-urgency rationalization: the agent dumped 8 raw addresses into `raw_addresses.toml` and bypassed final gate verification before live deployment.
6. Subagent routing cascade: 5 failed subagent calls to offline local backends before falling back to cloud.
7. Terminal token waste: 46 redundant `cd $PWD &&` invocations across 53 shell commands.

This plan specifies the implementation for hardening scripts, test runners, and agent directives against all 7 failure modes.

---

## 2. Track H: Task Specifications

```
  H1: Recursive Manifest Discovery & Fuzzy Suggestions
  H2: Decouple --prepare Build Success from --apply Constraints
  H3: Decouple check-exports from Stale KPM Trees (Scratch Gate)
  H4: Canonical KPM Directory Creation & Case Collision Guard
  H5: Agent Verify-After-Fix Invariant & Harness Middleware
```

---

### Phase H1: Recursive Manifest Discovery & Fuzzy Stderr Suggestions

#### 1. Exact Code Changes
- **Target Files**:
  - [`scripts/kpm_status.py`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/kpm_status.py#L171)
  - [`scripts/kpm_hotswap.py`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/kpm_hotswap.py#L138)
- **Anchor**: Change shallow glob `MODS.glob("*_src/manifest.toml")` to recursive `MODS.glob("*_src/**/manifest.toml")`.
- **Mod Resolution**:
  In `scripts/kpm_hotswap.py:resolve()`:
  - Add relative source directory alias: `keys.add(canon(str(m.parent.relative_to(MODS))))`.
  - Add exact parent name alias: `keys.add(canon(m.parent.name))`.
  - On lookup failure (`not hit`), compute candidate suggestions using `difflib.get_close_matches(canon(q), all_keys)` and print:
    ```
    usage error: unknown mod '<query>'
    Did you mean: '<suggestion>'?
    Available mod IDs: <comma-separated list>
    ```

#### 2. Edge Cases & Test Matrix
- Nested mod paths: `--mod perf-lab`, `--mod perf_lab_src/perflab`, `--mod perflab`, `--mod PerfLab` all resolve to `patch_manager_mods/perf_lab_src/perflab/manifest.toml`.
- Nested probe paths: `dialogue_letterbox_fix_src/probe/manifest.toml`.
- Typo query: `--mod action_hotbr` outputs suggestion `Did you mean 'action-hotbar'?`.

#### 3. Verification Commands
- `python3 scripts/run_tests.py suite:test_kpm_status`
- `python3 scripts/run_tests.py suite:test_kpm_hotswap`

---

### Phase H2: Decouple `--prepare` Build Success from `--apply` Constraints

#### 1. Exact Code Changes
- **Target File**: [`scripts/kpm_hotswap.py:227-235`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/kpm_hotswap.py#L227-L235)
- **Anchor**: Post-prepare verdict check in `main()`.
- **Refactoring**:
  - Replace the blocking `if bad: return 1` check with a categorized readiness summary.
  - If all target mods mirrored and `create-patch.py` succeeded, return exit code `0`.
  - Emit clear agent-facing separation between mods ready for hot-swap (`SWAP`) and mods requiring launcher reinstall (`REINSTALL`):
    ```python
    swappable = [pid for pid, r in todo.items() if r["verdict"] == "SWAP"]
    reinstall = [pid for pid, r in todo.items() if r["verdict"] == "REINSTALL"]
    print(f"PREPARE SUCCESS: {len(todo)} mod(s) mirrored and rebuilt.")
    if swappable:
        print(f"  Ready for --apply (hot-swap): {', '.join(swappable)}")
    if reinstall:
        print(f"  Requires KPatchLauncher reinstall (hooks changed): {', '.join(reinstall)}")
    return 0
    ```

#### 2. Edge Cases & Test Matrix
- Mixed batch: preparing a batch with hook changes (`action-hotbar`) and pure DLL changes (`combat-hotbar`) succeeds with exit code 0.
- Rebuild failure: if `create-patch.py` fails or returns non-zero, exit code remains 1.

#### 3. Verification Commands
- `python3 scripts/run_tests.py suite:test_kpm_hotswap`

---

### Phase H3: Decouple `check-exports` from Stale KPM Trees (Scratch Gate)

#### 1. Exact Code Changes
- **Target Files**:
  - [`scripts/pre_install_gate.py`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/pre_install_gate.py)
  - [`scripts/check_exports.py`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/check_exports.py)
- **Anchor**: `step_default` and `step_scratch` in `pre_install_gate.py`.
- **Refactoring**:
  - In `pre_install_gate.py`: filter out `suite:check-exports` from Step 1 (`step_default`) for the mods actively specified in `--mod`.
  - In Step 3 (`step_scratch`): after packing the fresh DLL into the scratch `.kpatch`, invoke `check_exports.py --kpatch <scratch_kpatch> --quiet`.
  - This guarantees `check-exports` validates the *freshly compiled binary* rather than the un-updated binary in the KPM repository.

#### 2. Edge Cases & Test Matrix
- Mod with new exports added to source: Step 1 does not falsely abort. Step 2 compiles DLL. Step 3 scratch kpatch verifies new exports and passes.
- Missing export in source `exports.def`: Step 3 aborts with `GATE FAIL 3 scratch: detour function not in exports.def`.

#### 3. Verification Commands
- `python3 scripts/test_pre_install_gate.py`
- `scripts/pre_install_gate.sh --mod perf_lab_src/perflab --skip-default`

---

### Phase H4: Canonical KPM Directory Creation & Case Collision Guard

#### 1. Exact Code Changes
- **Target Files**:
  - [`scripts/kpm_status.py`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/kpm_status.py#L77-L84)
  - [`scripts/kpm_hotswap.py`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/kpm_hotswap.py#L164-L175)
- **Anchor**: `kpm_dirs()` and `mirror()`.
- **Refactoring**:
  - In `kpm_status.py:kpm_dirs()`: detect duplicate patch IDs across differently-cased folders. If both `Patches/save-events` and `Patches/SaveEvents` exist, raise an immediate error:
    `FATAL: Duplicate KPM patch ID '<id>' found in directories '<dir1>' and '<dir2>'`
  - In `kpm_hotswap.py:mirror()`: if target KPM folder does not exist, resolve via `canon()` against existing folders. If completely new, auto-create using PascalCase (`mod_id.replace("_", "-").title().replace("-", "")`) so the agent never calls manual `mkdir`.

#### 2. Edge Cases & Test Matrix
- New library mod (e.g. `save-events`): auto-creates `Patches/SaveEvents/` without manual agent intervention.
- Existing directory with alternative casing: correctly mirrors into existing folder without creating a dual-case twin.

#### 3. Verification Commands
- `python3 scripts/run_tests.py suite:test_kpm_status`
- `python3 scripts/run_tests.py suite:test_kpm_hotswap`

---

### Phase H5: Agent Verify-After-Fix Invariant & Harness Middleware

#### 1. Exact Code Changes & Directives
- **Target Files**:
  - [`AGENTS.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/AGENTS.md)
  - [`patch_manager_mods/BUILD_AND_INSTALL.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/BUILD_AND_INSTALL.md#L65-L85)
- **Directives Added to `AGENTS.md`**:
  ```markdown
  ## Verify-After-Fix & Gate Invariant (CRITICAL)
  - **Zero-Assumption Gate Invariant**: An agent may NEVER label a gate or test failure as "pre-existing" or "expected" unless explicitly listed in a known-issues block in the active prompt. A gate failure is a blocking error.
  - **Immediate Re-Verification Loop**: Whenever source code, manifests, allowlists (`raw_addresses.toml`), or `.kpatch` files are edited to fix a failure, the specific failing test suite MUST be re-executed immediately. Never batch verification after multiple untested edits.
  - **Final Gate Pass Before Deploy**: Rebuilding `.kpatch` files via `kpm_hotswap.py --prepare` REQUIRES re-running `scripts/pre_install_gate.sh` to obtain a fresh `GATE PASS` before calling `--apply`.
  - **Prohibited Raw Address Bypass**: Never append raw hex addresses directly to `raw_addresses.toml` without first running `python3 scripts/query_ghidra.py address <addr>` to confirm whether symbolic names or `GameAddr.h` constants exist.
  ```
- **Updates to `BUILD_AND_INSTALL.md`**:
  - Update Section 5 to define the strict two-stage gate lifecycle:
    1. Pre-Mirror Check: `pre_install_gate.sh --skip-default --mod <id>_src`
    2. Mirror & Pack: `kpm_hotswap.py --mod <id> --prepare`
    3. Full Verification: `pre_install_gate.sh --mod <id>_src` (MUST report `GATE PASS`)
    4. Live Apply: `kpm_hotswap.py --mod <id> --apply`

#### 2. OpenCode Harness Enhancements
- Strip redundant `cd $PWD &&` from subshell executions before dispatch.
- Fast fallback circuit breaker: on local backend error (`Model unloaded` / 503), immediately fail over to cloud.

---

### Phase H6: Auto-Commit KPM Mirror (`kpm_hotswap.py --commit-kpm`)

#### 1. Exact Code Changes
- **Target File**: [`scripts/kpm_hotswap.py`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/kpm_hotswap.py)
- **Anchor**: Add argument `--commit-kpm` in `main()` parser and update `print_commit_commands()`.
- **Refactoring**:
  - Add CLI option `--commit-kpm`: automatically commits mirrored changes in `$KPM` instead of only printing strings to stdout.
  - In `kpm_hotswap.py`: after `mirror()` and `create-patch.py` succeed for a mod:
    ```python
    if a.commit_kpm:
        untr, mod = git_pending(kpm, pathspec)
        if untr:
            subprocess.run(["git", "-C", str(kpm), "add", "--"] + untr, check=True)
        paths = untr + mod
        if paths:
            msg = f"Patches/{kd.name}: {pid} {row['version']} (repo {git_head(REPO)})"
            subprocess.run(["git", "-C", str(kpm), "commit", "-m", msg, "--"] + paths, check=True)
            print(f"  kpm  {pid}: committed {git_head(kpm)}")
    ```
  - When `--commit-kpm` is provided with `--apply`, `--prepare` completes and transitions directly to `--apply` with zero manual bash interactions.

#### 2. Edge Cases & Test Matrix
- Already clean KPM checkout: no-op, logs `kpm <pid>: clean at <sha>, nothing to commit`.
- Untracked files in KPM directory: explicitly staged via `git add` before committing.
- Commit failure (e.g. git lock or pre-commit hook): aborts with clear error and non-zero exit code without continuing to `--apply`.

#### 3. Verification Commands
- `python3 scripts/run_tests.py -k kpm_hotswap`
- Dry run: `python3 scripts/kpm_hotswap.py --mod companion-selfheal --prepare --commit-kpm`

---

### Phase H7: Production Dirty Auto-Committer (`scripts/kpm_commit.py`)

#### 1. Exact Code Changes
- **Target File**: `scripts/kpm_commit.py` (new executable script)
- **Dependencies**: Uses `kpm_status.git_dirty()` and `kpm_status.is_production()` to isolate true production files from test logs, docs, and scratch artifacts.
- **CLI Design**:
  ```bash
  python3 scripts/kpm_commit.py [--mod MOD [MOD ...]] [--dry-run] [--prefix feat|fix|refactor]
  ```
- **Execution Flow**:
  1. Scan all mods in `patch_manager_mods/*_src/` for dirty production files.
  2. For each dirty mod:
     - Run targeted unit test suite (`python3 scripts/run_tests.py -k <mod_id>`).
     - If tests pass, execute:
       `git commit -m "<prefix>(<mod_id>): update production sources" -- <production_files>`
     - If tests fail, print `TEST FAIL for <mod_id>; commit aborted` and exit 1.

#### 2. Edge Cases & Test Matrix
- Non-production files (markdown docs, scratch tests, `.bak`): ignored; never mistakenly committed as mod production files.
- Untracked production files (e.g. new `.cpp` or `.h` in `*_src/`): staged and committed.
- Clean working directory: prints `All mod directories clean; nothing to commit` and exits 0.

#### 3. Verification Commands
- `python3 scripts/kpm_commit.py --dry-run`
- `python3 scripts/run_tests.py -k kpm_commit`

---

### Phase H8: Smoke Test Harness Canonical Include Normalization (`run_tests.py`)

#### 1. Exact Code Changes
- **Target Files**:
  - [`scripts/run_tests.py:build_wine_smoke()`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/scripts/run_tests.py)
  - [`patch_manager_mods/perf_lab_src/harness/smokes.toml`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/patch_manager_mods/perf_lab_src/harness/smokes.toml)
- **Refactoring**:
  - In `run_tests.py`: resolve the canonical KPM Common path:
    ```python
    kpm_common = REPO / "patch_manager_mods" / "_shared_src"
    canonical_common = Path(os.environ.get("KPM_COMMON_DIR") or (Path.home() / "Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common"))
    canonical_includes = [f"-I{kpm_common}", f"-I{canonical_common}"]
    ```
  - Prepend `canonical_includes` to MinGW compiler arguments for all smoke test builds, overriding any broken scratch-tree `$KPM_DIR` paths.
  - Remove brittle hardcoded `-I$HOME/Games/...` and `-I$KPM_DIR/...` entries across `smokes.toml`.

#### 2. Edge Cases & Test Matrix
- Scratch sandbox build (`pre_install_gate.py` where `$KPM_DIR` is in `/tmp/`): smoke compiler resolves headers from canonical KPM checkout, eliminating `fatal error: kpatch_api.h: No such file or directory`.
- Standalone execution (`python3 scripts/run_tests.py --wine`): resolves identical headers.

#### 3. Verification Commands
- `python3 scripts/run_tests.py --wine -k portraitslots`
- `scripts/pre_install_gate.sh --mod portrait_slots_src`

---

### Phase H9: End-to-End Atomic Deployment Pipeline (`scripts/kpm_ship.py`)

#### 1. Exact Code Changes
- **Target File**: `scripts/kpm_ship.py` (new executable script)
- **CLI Design**:
  ```bash
  python3 scripts/kpm_ship.py [--mod MOD [MOD ...]] [--apply] [--skip-gate]
  ```
- **Pipeline Orchestration**:
  1. `kpm_status` check: identifies target mods with pending changes (`DIRTY`, `MIRROR`, `REBUILD`, `SWAP`).
  2. Auto-commit repo: invokes `kpm_commit` for dirty mods with passing unit tests.
  3. Pre-install gate: invokes `pre_install_gate.py` across target mods. Halts immediately on failure with full diagnostics.
  4. Prepare & mirror: invokes `kpm_hotswap.py --prepare --commit-kpm` to mirror code, rebuild `.kpatch`, and commit KPM.
  5. Live apply: if `--apply` is set and `game_running=no`:
     - Invokes `kpm_hotswap.py --apply`.
     - Validates and prints final `kpm_status` table (verifying all targets are `LIVE`).

#### 2. Edge Cases & Test Matrix
- Game running when `--apply` requested: refuses swap before touching files; preserves prepped kpatches.
- Gate failure: halts before mirroring; leaves KPM and live game untouched.
- Single mod vs batch: handles `--mod buff-duration-hud` or defaults to all pending mods.

#### 3. Verification Commands
- `python3 scripts/kpm_ship.py --dry-run`
- `python3 scripts/kpm_ship.py --mod companion-selfheal`

---

### Phase H10: Safe Automated Hook Reinstaller (`scripts/kpm_reinstall.py`)

#### 1. Exact Code Changes
- **Target File**: `scripts/kpm_reinstall.py` (new executable script)
- **CLI Design**:
  ```bash
  python3 scripts/kpm_reinstall.py --mod MOD [--dry-run] [--rollback <backup_dir>]
  ```
- **Execution Workflow**:
  1. Detect mods with `REINSTALL` verdict (e.g. `action-hotbar`, `queue-hud` whose hook counts changed).
  2. Snapshot live state: backups live `patches/` DLLs, `patch_config.toml`, and `kpm_install_state.json` to `$G/backups/<mod>_reinstall_<ts>/`.
  3. Scan uncommitted peer DLLs: records checksums of all live DLLs that differ from their repository `.kpatch`.
  4. Reinstall via `KPatchLauncher`:
     ```bash
     KPATCH_NO_LAUNCH=1 "$K/bin/NoLaunch/KPatchLauncher" "$G/swkotor2.exe" --patches "$K/Patches" <all_live_ids>
     ```
  5. Restore uncommitted peer DLLs: copies back any peer DLLs overwritten by the full launcher reinstall.
  6. Verify: runs `kpm_status.py` to confirm the target mod transitioned to `LIVE`.

#### 2. Edge Cases & Test Matrix
- Reinstall rollback: `python3 scripts/kpm_reinstall.py --rollback <dir>` restores the full pre-reinstall snapshot.
- Interrupted reinstall: snapshot manifest enables clean recovery.

#### 3. Verification Commands
- `python3 scripts/kpm_reinstall.py --dry-run --mod action-hotbar`

---

## 3. Plan Status & Batch Registry

| Phase | Description | Scope | Status | Next Action |
|---|---|---|---|---|
| H1 | Recursive manifest discovery & fuzzy stderr hints | `scripts/kpm_status.py`, `scripts/kpm_hotswap.py` | DONE | Apply recursive glob & fuzzy match |
| H2 | Decouple `--prepare` exit code from `--apply` constraints | `scripts/kpm_hotswap.py` | DONE | Return 0 on mirror/rebuild success |
| H3 | Decouple `check-exports` from stale KPM trees in gate | `scripts/pre_install_gate.py` | DONE | Test scratch kpatch in Step 3 |
| H4 | Canonical KPM folder auto-creation & casing collision guard | `scripts/kpm_hotswap.py`, `scripts/kpm_status.py` | DONE | Implement PascalCase auto-dir & collision check |
| H5 | Verify-After-Fix invariant & two-stage gate documentation | `AGENTS.md`, `BUILD_AND_INSTALL.md` | READY | Insert invariant directives & update lifecycle |
| H6 | Auto-commit KPM mirror in `--prepare` (`--commit-kpm`) | `scripts/kpm_hotswap.py` | DONE | Implement `--commit-kpm` flag and git commit execution |
| H7 | Production dirty auto-committer (`scripts/kpm_commit.py`) | `scripts/kpm_commit.py` | READY | Build dirty scanner, test runner, and commit wrapper |
| H8 | Smoke harness canonical include normalization | `scripts/run_tests.py`, `smokes.toml` | READY | Inject canonical Common includes in MinGW smoke builds |
| H9 | End-to-end atomic deployment pipeline (`scripts/kpm_ship.py`) | `scripts/kpm_ship.py` | READY | Implement unified single-command shipping runner |
| H10 | Safe automated hook reinstaller (`scripts/kpm_reinstall.py`) | `scripts/kpm_reinstall.py` | READY | Build KPatchLauncher wrapper with peer DLL preservation |

---

## 4. Cross-Phase Overlap & Dependency Analysis

```
  H1 (Recursive Discovery) ──► H4 (Canonical Folders)
          │                               │
          ▼                               ▼
  H2 (Decoupled --prepare) ──► H6 (--commit-kpm) ──► H9 (kpm_ship.py)
          ▲                                                 ▲
          │                                                 │
  H3 (Scratch Gate) ────────────────────────────────────────┤
                                                            │
  H7 (kpm_commit.py) ───────────────────────────────────────┘
                                                            │
  H10 (kpm_reinstall.py) ◄── [REINSTALL Verdict Handoff] ───┘
```

1. **H2 vs H6 vs H9 Pipeline Coupling**:
   - `H9` (`kpm_ship.py`) directly invokes `H7` (commit) -> `H3` (gate) -> `H6` (mirror + commit) -> apply.
   - **Critical Contract**: `H6` depends on `H2`. If `H2` is not implemented first, `kpm_hotswap.py --prepare --commit-kpm` will exit with code 1 on any mod whose hooks modified (`REINSTALL`), causing `kpm_ship.py` to abort before committing KPM changes.
2. **H4 vs H6 Untracked Directory Staging**:
   - When `H4` auto-creates a new PascalCase directory in `$KPM/Patches/` (e.g. `Patches/GuiInput`), all mirrored files are initially untracked in KPM git.
   - `H6` (`git_pending()`) must execute `git add -- <untracked>` for new directories before committing; otherwise git will ignore untracked files and commit nothing.
3. **H5 Directives vs H7/H9 Tooling Automation**:
   - `H5` establishes prompt-level rules in `AGENTS.md` ("Verify After Fix").
   - `H7` and `H9` translate these instructions into deterministic code paths. If an agent runs `kpm_ship.py`, compliance with `H5` is guaranteed automatically.

---

## 5. Critical Edge Cases & Architectural Safeguards

1. **Mixed Batch Partitioning in `kpm_ship.py` (H9)**:
   - A multi-batch run often includes mixed mod types:
     - Pure DLL modifications (`SWAP` -> hot-swappable).
     - Provider libraries (`NOT-LIVE` -> e.g. `gui-input`, `save-events` not in `patch_config.toml`).
     - Hook structure changes (`REINSTALL` -> e.g. `action-hotbar`, `queue-hud`).
   - **Safeguard**: `kpm_ship.py` must never treat `NOT-LIVE` or `REINSTALL` as fatal errors during `--prepare`. It must mirror and build all targets, apply only `SWAP` targets, and print an explicit handoff notice:
     `"Target mods [X, Y] require full hook reinstallation. Run: python3 scripts/kpm_reinstall.py --mod X --mod Y"`
2. **Topological Commit Ordering in `kpm_commit.py` (H7)**:
   - If `_shared_src` (e.g. `GuiInput.h`, `SaveEvents.h`) is dirty alongside consumer patches (`action_hotbar_src`), committing consumer patches first breaks git bisectability.
   - **Safeguard**: `kpm_commit.py` must sort commits topologically:
     1. `patch_manager_mods/_shared_src/`
     2. Provider libraries (`gui_input_src/`, `save_events_src/`)
     3. Consumer patches (`action_hotbar_src/`, `combat_hotbar_src/`, etc.)
     If `_shared_src` is dirty, `kpm_commit.py` must also run `python3 scripts/compat_check.py` before committing.
3. **Peer DLL & `patch_config.toml` Preservation in `kpm_reinstall.py` (H10)**:
   - `KPatchLauncher` uninstalls all patches and rewrites `patch_config.toml`, which reverts peer DLLs that were hot-swapped without a repo kpatch.
   - **Safeguard**: `kpm_reinstall.py` must snapshot both `$G/patches/` and `$G/patch_config.toml`. After `KPatchLauncher` completes, it must restore any overwritten peer DLLs and merge any custom configuration sections before verifying with `kpm_status.py`.
4. **Dynamic CRT Runtime Dependencies in MinGW Smokes (H8)**:
   - Smokes compiled dynamically with MinGW (e.g. `smoke_perflab2.cpp`) require `libstdc++-6.dll`, `libgcc_s_dw2-1.dll`, and `libwinpthread-1.dll` in the execution directory under Wine, or Wine exits with code 53 silently.
   - **Safeguard**: `run_tests.py` smoke runner must copy or symlink these 3 runtime DLLs into the scratch test directory alongside the test executable.
5. **Stale Raw Address Cleanup in `tests/static/raw_addresses.toml`**:
   - When engine addresses migrate to symbolic constants in `GameAddr.h`, existing allowlist entries produce warnings (`WARN raw-address: stale allowlist entry`).
   - **Safeguard**: Add `--prune` flag to `scripts/check_raw_addresses.py` so agents can prune obsolete allowlist entries cleanly without manual regex editing.
6. **Shallow Mirroring & Asset Pool Omission (`is_production()` & `mirror()`)**:
   - `kpm_status.py`'s `CODE_EXT` (`.cpp`, `.h`, `.inc`, `.def`) omits `.ini`, `.toml` (e.g. `code_patches.toml`), and asset files (`.tpc`, `.mdl`, `.mdx`, `.2da`, `.tga` in `override/`).
   - `kpm_hotswap.py`'s `mirror()` performs a shallow `src.iterdir()`, ignoring nested asset trees (such as `npc_diversity_src/override/`).
   - **Safeguard**: Expand `is_production()` to include `.ini`, `.toml`, `.tpc`, `.mdl`, `.mdx`, `.2da`, `.tga`, and make `mirror()` recursively copy nested production asset folders while skipping test harnesses.
7. **Cross-Worktree Concurrency Lock on Shared `$KPM` Repo**:
   - Multiple git worktrees share a single KPM checkout (`~/Games/.../kotor-patch-manager`). Concurrent agents running `--prepare` can clobber uncommitted mirror states.
   - **Safeguard**: `kpm_hotswap.py --prepare` and `kpm_ship.py` must acquire a lock via `scripts/master_ledger.py` (`LOCK_ACQUIRE` for `kpm_repo`), check for collision, and release on exit.
8. **Headless Git Commit Hangs (GPG Pinentry Guard)**:
   - If user git configuration enables `commit.gpgsign = true`, `git commit` invocations inside subagents or background tasks will hang indefinitely waiting for pinentry.
   - **Safeguard**: `kpm_hotswap.py --commit-kpm` and `kpm_commit.py` must detect non-interactive environments and add `--no-gpg-sign` to prevent deadlocking.

---

## 6. Required Additional Test Suites

| Test Suite | Target | Verifies |
|---|---|---|
| `scripts/test_kpm_status.py` | `kpm_status.py` (H1, H4) | Recursive manifest discovery (`perf_lab_src/perflab`), case collision detection (`SaveEvents` vs `save-events`). |
| `scripts/test_kpm_hotswap.py` | `kpm_hotswap.py` (H1, H2, H4, H6) | Stderr fuzzy suggestions, exit code 0 on `REINSTALL` in `--prepare`, `--commit-kpm` staging of untracked dirs. |
| `scripts/test_kpm_commit.py` | `kpm_commit.py` (H7) | Production vs non-production file filtering, topological commit order, unit test abort on failure. |
| `scripts/test_kpm_ship.py` | `kpm_ship.py` (H9) | End-to-end orchestration, dry-run mode, mixed-batch partitioning (`SWAP` vs `REINSTALL` vs `NOT-LIVE`), running-game abort. |
| `scripts/test_kpm_reinstall.py` | `kpm_reinstall.py` (H10) | Snapshot creation, mock `KPatchLauncher` execution, peer DLL restoration, and atomic rollback on failure. |
| `scripts/test_pre_install_gate.py` | `pre_install_gate.py` (H3) | Decoupled Step 1 default tier, Step 3 scratch `.kpatch` export verification with new detour functions. |



