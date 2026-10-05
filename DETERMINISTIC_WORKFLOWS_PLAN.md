# Deterministic Tooling & Workflow Optimization Plan

**Status:** Draft / Approved for Implementation  
**Target Repos:** `orca/workspaces/local-setup/KOTOR-II`  
**Goal:** Eliminate nondeterministic multi-turn LLM bash loops, ad-hoc heredoc scripts, and redundant path definitions across Claude Code and Antigravity sessions.

---

## 1. Problem Statement & Baseline Metrics

Analysis of the last 15 Claude Code sessions in this workspace identified massive token and turn overhead:
- **Token Consumption:** 418M cache-read tokens, 2.65M output tokens.
- **Tool Invocations:** 881 `Bash` calls, 125 `Agent` subagents, 201 multi-line `<<'EOF'` heredocs.
- **Primary Failure Modes:**
  1. **Inline File Editing via Python:** Writing 20–50 line `python3 - <<'EOF'` scripts to patch markdown or code because of complex multi-file edits.
  2. **Ad-hoc SQLite Inspection:** Writing inline Python scripts to query `kotor2_ghidra_knowledge.db` (or failing with `sqlite3: command not found`).
  3. **Path Boilerplate Repetition:** Redefining `$G` (Steam directory) 46+ times and calling `cd` 302 times.
  4. **Manual Test Compilation:** Hand-crafting `g++ -std=c++20 -m32 ...` commands across multiple directories to test shared contracts.
  5. **Live Stage Verification Guesswork:** Ad-hoc chains of `md5sum`, `tomllib`, and `diff` across the live game folder and stage directories.

---

## 2. Architectural Boundary: Skills vs. `CLAUDE.md` / `AGENTS.md` vs. Scripts

To maintain minimal token overhead and predictable agent behavior, capabilities are partitioned strictly across three tiers:

```text
┌────────────────────────────────────────────────────────────────────────┐
│ 1. CLAUDE.md / AGENTS.md (Root Rules - Always in Context)              │
│    - Global invariants, credential boundaries, upstream check policy  │
│    - One-line command cheatsheet & path reference ($K2_GAME, etc.)    │
│    - Zero script bodies, zero large example blocks                     │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ triggers
┌───────────────────────────────────▼────────────────────────────────────┐
│ 2. Claude Skills (.claude/skills/<name>/SKILL.md - Progressive)        │
│    - Multi-step procedural reasoning, branch selection, and gates     │
│    - Injected ONLY when matching prompt/intent is triggered            │
│    - Coordinates execution and interprets machine-readable output     │
└───────────────────────────────────┬────────────────────────────────────┘
                                    │ executes
┌───────────────────────────────────▼────────────────────────────────────┐
│ 3. Deterministic Shared Scripts (scripts/*.py, scripts/*.sh)           │
│    - Standalone, zero-dependency (Python 3.12 stdlib), unit-tested    │
│    - Strict exit codes: 0 = PASS, 1 = FAIL, 2 = USAGE                  │
│    - Human-readable + machine-readable (--json) output                 │
└────────────────────────────────────────────────────────────────────────┘
```

### Partitioning Matrix

| Capability / Workflow | Tier 1: `CLAUDE.md` / `AGENTS.md` | Tier 2: Skill (`.claude/skills/`) | Tier 3: Script (`scripts/`) | Rationale |
| :--- | :--- | :--- | :--- | :--- |
| **Path & Env Setup** | Reference canonical env vars | None needed | `scripts/env.sh` | Shell environment setup must run instantly without prompt tokens. |
| **Ghidra / Address DB Queries** | 1-line CLI example | `openkotor-ghidra-server` (enhanced) | `scripts/query_ghidra.py` | Eliminates ad-hoc Python sqlite scripts; skill provides semantic interpretation. |
| **Common Contract Testing** | Standing rule: run contract tests on `_shared_src` edits | `kotor2-contract-gate` (new) | `scripts/run_contract_tests.py` | Standalone script handles compilation flags (`-m32`, includes); skill gates PRs/merges. |
| **Stage & Live Install Safety** | Rule: stage check must be green before launch | `kotor2-stage-deploy` (new) | `scripts/kpm_stage_check.py` (enhanced) | Validates MD5s, config diffs, and orphan DLLs in a single pass. |
| **Session Handoff & Plan Memory** | Pointer to `/plan-handoff` | `plan-handoff` (existing) | `scripts/session_handoff.py` (new) | Replaces Python heredoc log-scraping with deterministic git/task state dumping. |

---

## 3. Component Specifications

### Component 1: Central Environment & Path Wrapper (`scripts/env.sh`)
- **Purpose:** Eliminate constant path redefinitions (`G=...`, `K=...`, `cd ...`).
- **Implementation:**
  - Exports:
    - `K2_ROOT`: Workspace root (`/home/ethansm/orca/workspaces/local-setup/KOTOR-II`).
    - `K2_GAME`: Steam game folder (`~/.local/share/Steam/.../Knights of the Old Republic II`).
    - `K2_KPM`: Kotor-Patch-Manager repo (`~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager`).
    - `K2_DB`: Steam Address DB worktree.
  - Sourced automatically or used via direct script references.

### Component 2: Ghidra Knowledge & Address DB CLI (`scripts/query_ghidra.py`)
- **Purpose:** Replace inline Python SQLite scripts with an instant CLI tool.
- **Subcommands:**
  - `symbol <name>`: Searches functions, global symbols, and offsets across GOG and Steam.
  - `address <hex>`: Looks up function name, boundary, size, and callers for an address.
  - `xref <symbol|address>`: Displays callers and callees from the callgraph table.
  - `stats`: Prints knowledge database coverage and verified count.
- **Flags:** `--json` for machine consumption, `--db <path>` override.
- **Exit Codes:** `0` (Found), `1` (Not found/error), `2` (Argument error).

### Component 3: Common Contract Test Runner (`scripts/run_contract_tests.py`)
- **Purpose:** Standardize multilib C++ compilation and test execution for `_shared_src` headers across all consumer mods.
- **Behavior:**
  1. Auto-discovers all `patch_manager_mods/*_src/common_contract_test.cpp`.
  2. Compiles each test against `patch_manager_mods/_shared_src` using `g++ -std=c++20 -m32 -O0 -g` to `$TMPDIR/kotor-tests/`.
  3. Executes binaries and checks assertion strings (`<mod> relies on:`).
  4. Returns structured summary table with pass/fail counts.
- **Flags:** `--mod <name>` to isolate a single mod, `--verbose` for full compiler diagnostics.
- **Skill Integration:** `kotor2-contract-gate` invokes this runner before permitting edits to `_shared_src` to be committed.

### Component 4: Unified Stage & Live Install Validator (`scripts/verify_live_install.py`)
- **Purpose:** Replace manual `md5sum` and `tomllib` bash loops with a single pre-flight deployment check.
- **Behavior:**
  1. Reads stage `patch_config.toml` and live `$K2_GAME/patch_config.toml`.
  2. Verifies all stage `.dll` files have matching SHA256/MD5 hashes.
  3. Detects orphan DLLs in live `patches/` that would be unmanaged.
  4. Checks if live game executable (`swkotor2.exe`) is actively running to prevent write lock collisions.
- **Exit Codes:** `0` (Safe to install), `1` (Safety violation/mismatch).

### Component 5: Deterministic Plan Handoff Helper (`scripts/session_handoff.py`)
- **Purpose:** Eliminate fragile heredoc scripts that scrape `.jsonl` transcript logs.
- **Behavior:**
  1. Captures `git status --short`, active branch, and recent commits.
  2. Reads `.claude/session_handoff.md` and rotates to `.claude/session_handoff.prev.md`.
  3. Extracts pending items and unresolved tasks from active `*_plan.md`.
  4. Emits a sanitized, token-efficient summary ready for the fresh session.

---

## 4. Comprehensive Testing, Hardening & Edge Case Matrix

Since these tools replace critical LLM reasoning steps in build, test, and deployment workflows, they must be rigorously hardened against edge cases, environment drift, and malformed inputs.

### 4.1. Edge Case Matrix by Component

#### 1. Environment & Path Bootstrap (`scripts/env.sh`)
| Test ID | Interaction / Edge Case | Expected Behavior | Verification Assertion |
| :--- | :--- | :--- | :--- |
| **ENV-01** | Subshell invocation (`bash scripts/env.sh`) | Detects non-sourced execution; prints warning and returns exit code 1. | Stderr contains `"Must be sourced: source scripts/env.sh"`. |
| **ENV-02** | Path with spaces (`"Knights of the Old Republic II"`) | Quotes all variable expansions; prevents word splitting in `$K2_GAME`. | `test -d "$K2_GAME"` succeeds without syntax error. |
| **ENV-03** | Invoked from nested subdirectory (e.g. `patch_manager_mods/`) | Resolves canonical root using `BASH_SOURCE[0]` realpath. | `$K2_ROOT` matches top-level repository root exactly. |
| **ENV-04** | Idempotent re-sourcing | Sourcing 5x in the same session does not prepend duplicate entries to `$PATH`. | `$PATH` contains exactly one occurrence of `$K2_ROOT/scripts`. |
| **ENV-05** | Missing optional dependency (e.g. unmounted Steam drive) | Emits clean diagnostic; exports fallback or leaves unset without crashing. | Returns code 0; warns: `"[WARN] Steam game dir not found at default path"`. |

#### 2. Ghidra Knowledge & Address DB CLI (`scripts/query_ghidra.py`)
| Test ID | Interaction / Edge Case | Expected Behavior | Verification Assertion |
| :--- | :--- | :--- | :--- |
| **GHI-01** | Hex address formatting variance | Normalizes `0x0053f000`, `0x53f000`, `53F000`, `53f000` to canonical hex integer. | All 4 inputs query address `0x0053f000` and return identical results. |
| **GHI-02** | Ambiguous/overloaded symbol name | Returns all matching symbols tagged by binary (`k2_win_gog_aspyr` vs `steam`). | Multiple entries returned; exit code 0. |
| **GHI-03** | Non-existent symbol or address | Emits clean message to stderr, does not dump python traceback. | Exit code 1; JSON mode returns `{"status": "not_found", "matches": []}`. |
| **GHI-04** | SQL injection / malformed input | Parameterized queries prevent syntax errors or DB corruption. | Input `' OR '1'='1` returns not_found (code 1), no sqlite error. |
| **GHI-05** | Missing or locked database file | Detects unreadable DB; reports clear file path error; handles lock timeout. | Exit code 2; stderr indicates `"Database unavailable or locked: <path>"`. |
| **GHI-06** | Large result set output capping | Truncates terminal output to top 25 matches with `--all` hint to protect context window. | Output does not exceed 100 lines unless `--all` is specified. |

#### 3. Common Contract Test Runner (`scripts/run_contract_tests.py`)
| Test ID | Interaction / Edge Case | Expected Behavior | Verification Assertion |
| :--- | :--- | :--- | :--- |
| **CON-01** | Missing 32-bit multilib toolchain (`g++ -m32`) | Pre-checks compiler capability before attempting build; fails fast with install instructions. | Stderr: `"Missing 32-bit multilib headers. Run: sudo apt install g++-multilib"`. |
| **CON-02** | Compiler syntax error vs runtime assert | Distinguishes build failure from runtime contract failure in summary table. | Build error tagged as `COMPILE_FAIL`; assert tagged as `CONTRACT_FAIL`. |
| **CON-03** | Hanging / infinite loop test execution | Kills hung subprocess after 15-second timeout; reports timeout failure. | Subprocess terminated via `SIGKILL`; test status reported as `TIMEOUT`. |
| **CON-04** | Intentional Mutation Verification (Red/Green) | Deliberately altering `_shared_src` header causes consumer contract test to fail loudly. | Injected breaking change produces non-zero exit code and displays failing check. |
| **CON-05** | Build artifact hygiene | Writes all test binaries to `$TMPDIR/kotor-tests/`; cleans up on exit/signal. | No `.o`, `.bin`, or executable artifacts left in `patch_manager_mods/`. |
| **CON-06** | Mod without contract test | Detects mod folder including `_shared_src` without `common_contract_test.cpp`. | Warns: `"[WARN] <mod> includes shared headers but lacks contract test"`. |

#### 4. Stage & Live Install Validator (`scripts/verify_live_install.py`)
| Test ID | Interaction / Edge Case | Expected Behavior | Verification Assertion |
| :--- | :--- | :--- | :--- |
| **STG-01** | Live game executable running | Checks `pgrep -af swkotor2.exe` before validation; blocks deployment to prevent locked DLL errors. | Exit code 1; message: `"ERROR: swkotor2.exe is currently running. Exit game first."` |
| **STG-02** | Malformed / invalid `patch_config.toml` | Catches syntax errors and missing tables; reports exact line number. | Exit code 2; stderr: `"Invalid TOML syntax in <path> at line X"`. |
| **STG-03** | Missing DLL in stage payload | TOML declares `patches/mod.dll` but file does not exist on disk. | Exit code 1; reports `"Stage payload missing file: patches/mod.dll"`. |
| **STG-04** | Unmanaged orphan DLL in live directory | Live `patches/` contains DLL not tracked in stage TOML. | Emits warning/error detailing orphan file to prevent payload collisions. |
| **STG-05** | Live hash mismatch / dirty state | Live DLL hash differs from both stage and backup baseline. | Reports diff with both SHA256 hashes displayed. |

#### 5. Deterministic Plan Handoff Helper (`scripts/session_handoff.py`)
| Test ID | Interaction / Edge Case | Expected Behavior | Verification Assertion |
| :--- | :--- | :--- | :--- |
| **HND-01** | Detached HEAD or in-progress rebase | Resolves HEAD SHA and branch status without throwing git errors. | Handoff records `"Branch: detached at <SHA>"`. |
| **HND-02** | Massive uncommitted diff (>10,000 lines) | Caps `git diff --stat` output to top 25 files to prevent blowing fresh session context. | Stat summary strictly bounded under 50 lines. |
| **HND-03** | Missing previous handoff file | First run with no `.claude/session_handoff.prev.md` initializes cleanly. | Exits code 0 without rotation error. |
| **HND-04** | Atomic write on concurrent session | Uses `NamedTemporaryFile` + `os.replace` to prevent truncated reads. | File write is atomic; permissions preserved (`0644`). |
| **HND-05** | Empty user arguments (`$ARGUMENTS`) | Auto-detects active branch and latest plan without hallucinating goals. | Records `"Next-Phase Direction: [Inferred from branch and active plan]"`. |

---

### 4.2. Failure Injection & Mutation Testing Protocol

To ensure contract tests and gates are not producing false-positive passes (tautological tests), every test runner must pass a two-step mutation check:

1. **Red Gate Check (Intentional Failure):**
   - Mutate a shared struct offset in `_shared_src/CommonStructs.h` (e.g. change offset from `0x10` to `0x14`).
   - Run `python3 scripts/run_contract_tests.py`.
   - **Verification:** The runner MUST fail with exit code 1, explicitly highlighting the assertion that failed (`<mod> relies on: struct offset 0x10`).
2. **Green Gate Check (Baseline Restore):**
   - Revert the mutation via `git checkout -- patch_manager_mods/_shared_src/`.
   - Run `python3 scripts/run_contract_tests.py`.
   - **Verification:** The runner MUST return exit code 0 (`ALL TESTS PASSED`).

---

### 4.3. Standard Exit Code Contract Across All Tools

Every tool in `scripts/` must strictly conform to standard POSIX process exit codes for deterministic chaining in agent workflows:

| Exit Code | Meaning | Agent Action |
| :--- | :--- | :--- |
| **`0`** | **PASS / SUCCESS** | Proceed to next step; consume output payload. |
| **`1`** | **OPERATIONAL FAILURE** | Stop workflow; diagnose reported issue (e.g. failing test, missing symbol, dirty stage). Do NOT retry without changing code/inputs. |
| **`2`** | **USAGE / CONFIG ERROR** | Command syntax invalid, missing file, or bad flag. Fix arguments. |
| **`124`** | **SUBPROCESS TIMEOUT** | Test or build process exceeded time limit (hung loop or deadlocked lock). |

---

### 4.4. CI & Master Suite Integration

All new unit tests (`scripts/test_*.py`) will be registered in `scripts/run_tests.py`:
- Executed in parallel with isolation via Python's `unittest` runner.
- Output reported in the unified test matrix:
  ```text
  [PASS] test_query_ghidra.py (6 tests, 0.08s)
  [PASS] test_run_contract_tests.py (6 tests, 1.42s)
  [PASS] test_verify_live_install.py (5 tests, 0.12s)
  [PASS] test_session_handoff.py (5 tests, 0.05s)
  ```
- Any failure in these critical tools blocks the master test gate (`python3 scripts/run_tests.py`).

---

## 5. Implementation Phases & Milestones

### Phase 1: Environment & Ghidra DB CLI
- [ ] Create `scripts/env.sh`.
- [x] Implement `scripts/query_ghidra.py`.
- [x] Add `scripts/test_query_ghidra.py` and register in `scripts/run_tests.py` (PASS).
- [x] Update `.claude/skills/openkotor-ghidra-server/SKILL.md` to reference `scripts/query_ghidra.py`.

### Phase 2: Common Contract Test Runner
- [ ] Implement `scripts/run_contract_tests.py`.
- [ ] Add `scripts/test_run_contract_tests.py`.
- [ ] Create skill `.claude/skills/kotor2-contract-gate/SKILL.md`.
- [ ] Update `patch_manager_mods/20_TESTING_PLAN.md` with the standardized invocation.

### Phase 3: Stage Verification & Live Safety
- [ ] Implement `scripts/verify_live_install.py` (or extend `scripts/kpm_stage_check.py`).
- [ ] Add unit tests in `scripts/test_verify_live_install.py`.
- [ ] Create skill `.claude/skills/kotor2-stage-deploy/SKILL.md`.

### Phase 4: Plan Handoff & Memory Automation
- [x] Implement `scripts/plan_manager.py` (resume, next, status, handoff, mark-done).
- [x] Add `scripts/test_plan_manager.py` and register in `scripts/run_tests.py` (PASS).
- [x] Update `.claude/skills/plan-handoff/SKILL.md` to call `plan_manager.py handoff`.

### Phase 5: Root Guidelines & Tooling Deployment
- [x] Implement structural repo map `scripts/generate_repo_map.py`.
- [x] Implement semantic documentation search `scripts/search_docs.py` (local LM Studio).
- [x] Install Ripgrep `rg` (v15.2.0) into user `$PATH`.
- [x] Update [`CLAUDE.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/CLAUDE.md) with fast local dev tools and context resume.
- [x] Update [`AGENTS.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/AGENTS.md) with fast local dev tools and context resume.
- [x] Update crash triage skill [`.claude/skills/kotor2-crash-triage/SKILL.md`](file:///home/ethansm/orca/workspaces/local-setup/KOTOR-II/.claude/skills/kotor2-crash-triage/SKILL.md).
- [x] Run workspace test pass: `python3 scripts/run_tests.py -k query_ghidra && python3 scripts/run_tests.py -k plan_manager` (PASS).
