# Swarm M2 safety: Session A handoff (2026-10-10)

Resume next: `swarm M2 safety, Session B (B1)`. Plan: Swarm M2 safety re-scope (post phase 3d), §4 Session B.

## Commits
- Bundle baseline (own git repo, `~/.claude/skills/deep-haiku-review/bundle`): `a42a422` ("baseline before M2 re-scope (includes phase 3d)"). Tarball: `~/.claude/swarm/backups/bundle_20261010_224532.tgz`.
- LIB (`~/.claude/swarm`, base `f0afd1c`):
  - `3f89f33` hygiene module (`lib/hygiene.py`, `tests/test_hygiene.py`)
  - `981759b` sandbox hygiene args, `--hide`, `--chdir` (`lib/sandbox.py`, `tests/test_sandbox.py`)
  - `22f410c` home probe `env_clean`, `run_dir_hidden`, `--argv-file` (`bin/swarm_home_probe.py`, `tests/test_home_probe.py`)
  - `7023abd` vendor-parity and bundle-wrapper tests (RED until B1)
- Project `34927cb`: `tests/suites.toml` (3 suites) and `scripts/swarm_argv_probe.py` (stubs `dr_sandbox_prefix`).

## A0 baselines (all $0; the "spend" in selftest output is the stub's simulated DEFAULT_COST, not billed)
| Check | Plan expected | Actual |
|---|---|---|
| selftest mock | 113 | 115/115 |
| selftest real --quick | 184 | 190/190 |
| selftest real (full) | ~192 | 198/198 |
| `run_tests.py swarm-lib` (pre-A3) | 3 PASS | 3 PASS |
| argv probe | `IDENTICAL`, control PASS | rc 0, IDENTICAL, control PASS, no `command not found` (not broken; A2b is hardening) |

## Red set (by design until B1)
- `test_hygiene_checks_pass`: `env_clean` False, `run_dir_hidden` False. The current wrapper has no `--unsetenv` for `FAKE_API_TOKEN` and no `/run/user/<uid>` tmpfs.
- `test_custom_non_stub_binary_is_wrapped`: EMPTY_PREFIX. `dr_is_stub` treats any `DR_CLAUDE_BIN` other than literal `claude` as a stub.
- The other 5 wrapper tests PASS. `run_tests.py swarm-bundle-wrapper` shows 2 failures.

## Mutation table (scratch copy of LIB; each must fail its test)
| ID | Mutation | Result |
|---|---|---|
| M1 | drop the secret-env loop in `hygiene_args` | FAIL: 3 args-shape tests + live env test |
| M2 | drop the run-dir tmpfs | FAIL: args-shape with run dir + live run-dir test |
| M3 | hygiene args moved above the bwrap literal | FAIL: 3 order tests (assertions); 4 live tests ERROR (bwrap exec of `--tmpfs`) |
| M4 | `env_clean` not recorded | ERROR: `KeyError: 'env_clean'` in 4 tests |
| M5 | non-bwrap `--argv-file` accepted | FAIL: `test_main_argv_file_non_bwrap_exits_2` |

## Plan corrections found during Session A
- `ANTHROPIC_BASE_URL` does not match `SECRET_ENV_RE`; the passthrough test checks only the three names that match, plus that all four are never returned.
- New live sandbox tests went into the existing `LiveSandboxTests` class (a second class with the same name would shadow it). One extra CLI test (`test_cli_passes_hide_and_chdir`) covers `--hide`/`--chdir`.
- `_sh(script, *args)` in the probe already matched the spec; no adaptation.
- `--argv-file` validation returns rc 2 before the bwrap availability check (rc 3).

## Watch list for the §7 canary
- `CLAUDE_CODE_MESSAGING_TOKEN` is unset by the hygiene regex. A child `claude -p` might need it (plan §3.0 item 5).
- `/run/user/<uid>` tmpfs and `claude_state` (default `rw`) remain unproven until the real canary.

## Open items for the user
- Delete `.claude/handoffs/misrouted_swarm_m2.handoff.md`? (Not deleted; needs a yes.)
- Registration choice: the three suites are registered now, so a bare `run_tests.py` shows one red (`swarm-bundle-wrapper`) until B1. Say so if you'd rather register it at B1.

## Next (Session B)
B1 wrapper hardening (`engine/verify/runlib.sh`, `dr_profile.py`, `engine/sandbox_hygiene.py`, `PROFILE_SPEC.md`, `dr_lib/selftest.py`). Gate first: `pgrep -af 'run_fix|run_map|run_integrate|fix_checks' | grep -v pgrep` must print nothing. Bundle baseline `a42a422`.

---

# Earlier handoff (M2.1 seed, 17:47): superseded where it conflicts with the Session A section above

# Swarm M2 safety handoff

Plan: "Swarm library: next phases (M2 safety now; M3-M5 as stubs)". Updated 2026-10-10 17:47 EDT.

## Status
- M2.1 library seed: DONE, committed.
- M2.0 steps 3-4 (read-only probes): DONE.
- M2.0 step 0 (live-run gate): OPEN. Runs live at 17:47: `run_map.sh` PID 65968; `run_integrate.sh` PID 21481 (fix pipeline in `.claude/review`).
- M2.0 steps 1-2 (backups, git repos, baselines): OPEN, gated.
- M2.2-M2.4 (fix edits), M2.5-M2.6 (bundle edits): OPEN, gated.

## Commits
- `~/.claude/swarm` (LIB) `f0afd1c`: M2.1 seed. `lib/invariants.py`, `lib/sandbox.py`, `bin/swarm_home_probe.py`, `tests/test_{invariants,sandbox,home_probe}.py`. `backups/` is git-ignored.
- KOTOR-II `a59cd6f`: `tests/suites.toml` adds `swarm-lib-*` (3 suites) and `swarm-fix-*` (5 suites, not run yet).

## Verified
- `python3 scripts/run_tests.py swarm-lib`: 3 passed. Unit counts: invariants 13, sandbox 11, home probe 5.
- Mutation check on a scratch copy: removing `--tmpfs HOME` fails the sentinel test. Not skipping `VALUE_OPTS` fails the `git -C x commit` tests.
- bwrap regression: `claude --version` (2.1.296) runs under tmpfs HOME, exit 0.
- Stream-json (M2.0 step 4): `wp7-018.a1` cache_read 7796, `wp7-019.a1` 7796, `wp7-016.a1` 21976. Prefix caching already occurs.
- Pin closure (M2.3): `scripts/{run_tests,build_check,compat_check,agent_diff_check,_common,_static_common,check_exports,compat_graph,master_ledger,agent_contract}.py` + `tests/suites.toml`. No dynamic imports. `_table.py` and `mutate_header.py` appear only in comments/strings.
- Gate parity (M2.2): `python3 git make i686-w64-mingw32-g++` resolve in the gate sandbox. `rg` does not (lives in `~/.cargo/bin`).

## Open decisions
- `rg` in gate: recommend ro-bind of `~/.cargo/bin/rg` alone (not the dir), or drop it from gate.
- M2.4 queue-change list needs a `fix_intake` dry run; not run (touches queue state).

## Dead Ends & Anti-Patterns (do NOT repeat)
- ❌ `python3 scripts/plan_manager.py handoff "<text>"`: ignored the text and wrote to the ACTIVE plan (`.claude/active_plan.txt` = `10_equipment_3d_character`). Also rotated the SHARED `.claude/session_handoff.md`. Repaired 17:5x: shared file restored from `.prev` (equipment session's handoff); misrouted copy at `.claude/handoffs/misrouted_swarm_m2.handoff.md` (untracked, delete once confirmed). Use this file for swarm state, not `plan_manager handoff`.
- ❌ `pgrep -af 'run_fix|run_map|run_integrate|fix_checks'` matches its own `bash -c` wrapper. Always add `| grep -v pgrep`.
- ❌ Sentinel under `/tmp`: the sandbox's own `--tmpfs /tmp` hides it, so the test passes vacuously. Use `/var/tmp`.
- ❌ Scratch files in `/tmp`: use the session scratchpad.
- ❌ Editing `BUNDLE/engine/` or `.claude/review/fix/` while any run is live.
- ❌ `git add -A` or bare `git commit`: the KOTOR-II worktree has ~220 unrelated changes. Use `git commit -- <paths>`.
- `mark-done`: not applicable. No master-plan phase covers this work.

## Verification Commands
- LIB: `cd ~/.claude/swarm && for t in tests/test_invariants.py tests/test_sandbox.py tests/test_home_probe.py; do python3 -I $t; done`
- Project: `python3 scripts/run_tests.py swarm-lib` (fix suites: `run_tests.py swarm-fix`, after the gate opens)
- Gate: `pgrep -af 'run_fix|run_map|run_integrate|fix_checks' | grep -v pgrep` (must print nothing)

## Resume
- Gate check (must print nothing): `pgrep -af 'run_fix|run_map|run_integrate|fix_checks' | grep -v pgrep`
- Once clear: M2.0 step 1 (BUNDLE + FORK git repos, tarball to `~/.claude/swarm/backups/`), then step 2 baselines.
- Do not edit `BUNDLE/engine/` or `.claude/review/fix/` while a run is live.
