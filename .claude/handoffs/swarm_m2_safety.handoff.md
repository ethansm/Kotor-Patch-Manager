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
