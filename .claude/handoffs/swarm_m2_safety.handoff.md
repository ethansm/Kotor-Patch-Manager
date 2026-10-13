# Swarm M2 safety: current state (M2 CLOSED incl. real canary PASS, `claude_state` default now `ro`, 2026-10-11)

Resume next: `swarm M5 recipes or swarm M4 measurement` (M4 DONE 2026-10-11, see the next section). Older line: `swarm M4, bug-investigate paid canary and pilot` (M3b and the bug-investigate $0 slice LANDED 2026-10-11, see the next section; the `prep.py` patch landed earlier, see M3a). M2 is finished: every item is landed and the real canary passed. M3a (harden the map runner in place) landed 2026-10-11, all four patches. What is left: M4 measurement (paid, includes the bug-investigate pilot) and M5 recipes. Nothing paid ran or is pending.

## M4: real engine, canary, pilot (2026-10-11 evening): REAL ENGINE LIVE, canary PASS, pilot scored. Paid total about $0.0143
**Commits.** LIB (`~/.claude/swarm`): `e697b86` bwrap `--new-session`; `651047e` real-engine gate stack (`REAL_ENABLED=True`, `REAL_MAX_TOTAL_USD=1.00`, `--yes-spend`, `swarm_run.py probe`, model-free `claude auth status` pre-flight, `canary/` role). Project: `0baa955` orchestrator `--claude-bin/--yes-spend` (probe first); `b618b4e` playtest bundle no overlapping excerpts (12 -> 9 excerpts on the hotbar log); `7f83a08` hotbar case drops the hex sentinels from `blind_forbidden_terms`; `02022a9` drop `$schema` from the 3 role schemas + regression row. Bundle `5a8be20` untouched.
**Gate stack for `--engine real`** (all exit 2 before lock/budget/model): explicit `--claude-bin` (not the stub), `--yes-spend >= --cap-total`, `--cap-total <= 1.00`, `--cap-call <= --cap-total`, `O/work` and tree exist (made by `probe`), fresh passing `--home-probe` whose `argv_sha256` equals the prefix built in the SAME env (`probe` and `map` both `setdefault SWARM_PROBE_TOKEN`), `claude auth status` loggedIn true inside the prefix. Mutations (all red): no `--yes-spend` needed -> `test_real_needs_yes_spend`; sha compare skipped -> `test_forged_stale_or_wrong_sha_probes_are_refused_before_any_call`; pre-flight skipped -> `test_logged_out_preflight_exits_2_with_zero_reservations`; token not set -> `test_probe_subcommand_then_dry_run_accepts_and_a_new_secret_env_var_refuses`.
**Canary (G1 yes, ceiling $0.05):** 1 Haiku call, status ok, schema_valid, `ok:true`, `echo:"swarm-canary"`, model `claude-haiku-5-5`, cost **$0.000743**, 2 turns (the structured-output tool turn counts; plan said 1), run `argv_sha256` = probe `argv_sha256`. `$HOME` count in raw output 0 (`$USER` appears once, inside the scratchpad cwd path). `.credentials.json` mtime unchanged; `~/.claude.json` mtime moved 4 s AFTER the call ended (the parent Claude Code session writes it); a write attempt from inside the prefix gets `Read-only file system` on both files.
**First pilot attempt = $0 defect.** The CLI rejected all role schemas offline in 0.7 s (`--json-schema is not a valid JSON Schema: no schema with key or ref "https://json-schema.org/draft/2020-12/schema"`) because of the `$schema` line. No API call; records say cost unknown and the budget counted the unknown at the cap (2 x $0.10), so a retry needs a fresh out dir. Fixed (`02022a9`), proven at $0 by pointing `ANTHROPIC_BASE_URL` at a closed port (schema accepted -> hangs on connect), then one re-run on a fresh run dir after a fresh yes (G2 retry).
**Pilot (run `~/.cache/bug_investigate/20261012T002705Z-logic`, blind x3 + null x1, Haiku only, no verifier).** Cost $0.013517 (advocate $0.003791 / 18.2 s, evidence $0.004044 / 20.8 s, skeptic $0.002695 / 15.9 s, null $0.002987 / 12.4 s). Quote-valid: 7 of 10 hypotheses (evidence 4/4, skeptic 3/3, advocate 0/3). The advocate cited truncated evidence ids (`ev:be358bef` instead of the full `ev:<log>:L<a>-<b>:<sha8>`) in all 3: the quote check rejected them (fail closed, correct; prompt-compliance defect of the role, not changed without a go). Null verdict `insufficient`, 3 reasons all valid; its reasons say one tick shows the id invalid with only one party member and that no bind attempt/companion profile is logged: matches the evidence limits.
| hypothesis | status | score vs `true_root_cause` | note |
|---|---|---|---|
| blind-evidence:h1 | supported, med | PARTIAL | names `ffffffff` for the only party member; "not identified"; no key |
| blind-evidence:h2 | supported, med | HIT (by rubric) | names the invalid id and the key `'' -> objfffffffd`; mechanism wrong (says lookup misses because of the obj key) |
| blind-skeptic:h2 | supported, med | PARTIAL | invalid id, "transient" framing |
| blind-evidence:h3 | supported, low | PARTIAL | invalid id after the 7995 ms gap, stale reference |
| blind-skeptic:h3 | supported, low | HIT (by rubric) | profile keyed to `objfffffffd` |
| blind-evidence:h4 | supported, low | MISS | out-of-stock display as downstream effect |
| blind-skeptic:h1 | supported, low | MISS | empty slots are normal display |
| blind-advocate:h1-h3 | rejected | n/a | truncated evidence ids |
Result: 2 HIT, 3 PARTIAL, 2 MISS among 7 supported; nobody found the multi-member condition (impossible: the legacy log has one party member). Distinct evidence ids per supported hypothesis: 1 each (A4 merge removed the near-copy excerpts). Ranking put the two best-grounded id hypotheses first (`blind-evidence:h1`, `h2`). Verdict: pipeline works end to end with the real engine; Haiku quality on this case is "finds the sentinel id, misses the cause", which fits the evidence.
**Residual risks / next.** (1) Advocate role id format: add an explicit "copy the full id inside the brackets" example or a short-id resolver in `evidence_quote_check` (needs a go before any paid re-run). (2) `num_turns` is 2 for structured output; do not gate on 1. (3) Budget ledger counts unknown cost at the cap: after an all-fail wave use a fresh out dir. (4) Not measured: 2 vs 3 layers, Sonnet verifier, comparator, a case with multi-member evidence (needs a post-fix-excluded log, none exists). Next: `swarm M5 recipes` or `swarm M4 measurement` (advocate fix + a second case first).

## M3b + bug-investigate $0 slice (2026-10-11): shared runner, bundle builder, quote checker, orchestrator. STUB ONLY, nothing paid
- **Landed**. LIB (fast-forward from a scratch clone, LIB was clean at `1af7738`): `1716fe8` `lib/run_record.py` + byte-identical `lib/budget.py` and `lib/schema_min.py` (parity pairs now 5, no skips) and `74ca9cb` `lib/swarm_run.py` + `tests/stub_claude.py` + 70 tests; LIB HEAD `74ca9cb`. Project: `0d138ec` (`playtest_bundle.py`, `evidence_quote_check.py`, roles `blind|null|verify`), `d2a30ac` (`bug_investigate.py`, `SKILL.md`, case `hotbar_charkey.json`, e2e test), `605f82c` (`tests/suites.toml` +`swarm-lib-run-record`, `swarm-lib-swarm-run`; `swarm_argv_probe.py` fix). Bundle untouched: HEAD `5a8be20`, same 8 dirty files, p2c `canary.json` still `aa05ed73bdea`, bundle `__pycache__` dirs all predate the session; floors (mock 163, quick 251, real 259, `fix_settings` 218, `fix_workspace` 110) not re-run because no bundle file changed.
- **Counts**: `test_run_record` 68, `test_swarm_run` 70 (40 s), `test_playtest_bundle` 70, `test_evidence_quote_check` 68, `test_bug_investigate` 83, `test_bug_investigate_e2e` 6 (real orchestrator, real builder on a synthetic game dir, real landed runner on the stub). `run_tests.py swarm-lib` 9 PASS (7 + 2 new), `swarm-bundle` 5, `swarm-fix` 5, `check_skills.py` PASS skills=8, `swarm_argv_probe.py` green.
- **Mutations (5 of 5 red on a scratch copy, originals untouched)**: (1) `rec.get(k, cur[k])` in `is_complete` → 19 rows incl. `test_rec_missing_any_one_key_field_is_incomplete`; (2) `engine` dropped from the key → `test_different_engine_is_incomplete` (+12); (3) retries stop reserving → `test_429_then_ok_is_two_reserves_two_settles_and_no_ghost` (+2); (4) fall back to `DR_CLAUDE_BIN` → `test_env_never_selects_the_binary_and_is_scrubbed`; (5) quote checker ignores a bad citation → `test_mixed_valid_and_fabricated_rejects_whole_hypothesis` (+43).
- **The four approved deviations held**: LIB-owned `tests/stub_claude.py`; LIB is the source of truth for `budget.py`/`schema_min.py` (bundle copies unchanged); `swarm_run.py --engine real` coded but `REAL_ENABLED = False` (exit 2 before anything is touched); blind/null roles run with `tools: []` and an empty cwd.
- **Other deviations and findings**: (a) `scripts/swarm_argv_probe.py` was TRACKED (not untracked) and had been failing since M3a (fake `budget.py` missing); fixed with a fake `budget.py`, three env vars and the `budget_settle` function. (b) `swarm_run spent` counts a record with `cost_known` absent as unknown (stricter than `budget.py spent-lens`, which treats it as known). (c) `swarm_run` calls the private `schema_min._check_keywords` (`validate()` mixes schema and instance errors). (d) `sandbox.bwrap_argv` has no `--new-session`; add it before the real engine is enabled. (e) the 429/529 regex can match a bare `429` in the last stdout line (same as `run_map.sh`). (f) `playtest_bundle` signature excerpts overlap heavily (`L97-105`, `L98-106`, ...), which wastes `items_per_log`; a merge pass would fix it. (g) `log_triage --launch 0` means "unset"; `playtest_bundle` refuses `@0`. (h) `bug_investigate.py --swarm-run PATH` goes BEFORE the subcommand.
- **Pilot feasibility (case `hotbar_charkey.json`)**: the plan's candidate fixes were wrong. `17dd709` is a rename and `25d4e42` closes a residual path; the symptom ("Character not identified yet" for the Exile with companions) is fixed by `e360569`, anchor `94c9c55a`. Doc 00 is true at `94c9c55`, doc 33 only at `4519b96` (they describe different code states). **Pre-fix evidence is partial**: all current-name hotbar logs are 18 h to 5 days newer than the fix. The legacy `force_power_hotbar_log.txt` (mtime 2026-10-04, one launch) has the alert `character: creature 0cd83410 has invalid object id ffffffff (party members=1)` at line 3504 (2 hits), i.e. the id mechanism but with ONE party member, not the multi-member condition. A real bundle from it builds fine (14.9 KB, 12 excerpts, residual check pass, bookmark file untouched). Score the pilot on the mechanism (invalid id, empty key); `insufficient` is a correct answer for the multi-member part. `ffffffff` was dropped from `blind_forbidden_terms` because it is evidence, not a leak.
- **Required next step (needs a human go, paid)**: a canary batch for the real engine: a `--home-probe` of the exact `swarm_run` sandbox argv, then flip `REAL_ENABLED`; then one paid pilot (blind x3 + null x1 on the legacy-log bundle, capped by `--cap-total`). Cost is not estimated here; the cap decides. Until then the Haiku hypothesis quality is unmeasured: the stub proves mechanics only.

## M3a (2026-10-11): map runner hardened in place; `prep.py` landed the same day (bundle `5a8be20`)
- **Decision (narrows proposal D2)**: no `swarm_run.py` port now. The fork is an ancestor, 19 engine files call `claude -p`, and the M2 safety layer lives in `runlib.sh` plus vendored libs, so a Python runner would have to re-prove every M2 gate. M3a fixes the open runner defects inside the bundle. **M3b** (extract `~/.claude/swarm/swarm_run.py`) waits for `bug-investigate` as the second caller; that consumer decides the API.
- **Landed** (each through the index, committed without a pathspec; landed tree `0bbaa01e87f5…` == the verified scratch tree; patches `~/.claude/swarm/backups/swarm_m3a_<task>.patch`): bundle `944c3a2` `schema_min.py` (new), `db240cb` `record_run.py`, `7e949c6` `run_map.sh` + `budget.py` (new) + one `dr_lib/selftest.py` fixture value. LIB `1af7738` `tests/test_runner_hardening.py` (22 tests, stub only). Project `8702b3b` `tests/suites.toml` (+`swarm-bundle-record-run`, `-schema-min`, `-budget`, `swarm-lib-runner-hardening`).
- **prep.py LANDED** (2026-10-11, same day): bundle `5a8be20` (patch `~/.claude/swarm/backups/swarm_m3a_prep.patch`; committed content = the verified scratch tree `3149235fc22d…`, which is `0bbaa01e87f5…` plus this one patch). Packet+schema sha completion key, unreadable record = incomplete, unknown cost counts one per-call cap, `stamp-runs`. The user asked to land it before the other session committed, so the WORKTREE copy of `engine/prep.py` was 3-way merged by hand (`git merge-file`, 2 conflicts, both in `run_complete`): their uncommitted `_complete(r, pid)` + `[coverage] accept_truncated` hunk is kept, my `run_ok` is dropped, and `run_complete`, `cmd_stamp_runs` and `lens_run_complete` call `_complete`. So `git diff HEAD -- engine/prep.py` is now exactly their hunk (10 insertions, 7 deletions) and nothing of mine; the committed `5a8be20` has `run_ok` and no `_complete`, so it does not depend on their uncommitted `dr_profile.py` `coverage` default. Their dirty file before the merge: `~/.claude/swarm/backups/m3a_prep_worktree_before_merge.py`. When they commit, their commit will carry the `run_ok` → `_complete` swap. `lens_run_complete` and `run_map.sh lens_done` keep the old key (lens runs are not sha-keyed; follow-up).
- **Upgrade step DONE (2026-10-11)**: `prep.py stamp-runs --apply` on both real classic run dirs, after a dry-run and a tarball backup (`~/.claude/swarm/backups/m3a_runs_before_stamp.tar.gz`, restore with `tar -xzf -C ~/.cache/deep_haiku_review`): `p2c` 52 stamped, 0 left unstamped, 3 not complete; `p1` 4 stamped, 0 left, 1 not complete. `pending` before = after (0 and 0, identical lists); with the new `prep.py` and no stamping it was 52 and 4 (fail closed); a second `--apply` stamps 0 (idempotent); `spent` p2c 1.7291, p1 0.0332, no unknown-cost line. **Not stamped**: `p1-lens` (its profile `pilot/profile_lens.toml` points at a deleted scratch repo, `profile: not a git repo`, so no `prep.py` command loads it; lens-mode completion is not sha-keyed anyway). Any NEW run dir created by the new `record_run` carries the shas, so it needs no stamping. If the bundle is re-cloned, stamping is already done in the records, so mtimes no longer matter for these two.
- **Defects (proposal 4.3)**: (a) done: atomic `runs/` and `findings/` writes (findings first, the run record last as the completion marker) + `prep.py pending`, `dr_profile.py lenses` and `all_packets` failures now exit 2 loud. (b) done (`5a8be20`): a rebuilt packet or changed schema re-queues. (c) done: every attempt, retries included, reserves `CALL_USD` through `budget.py` before calling `claude`. (d) done in `record_run` (cost stays `null`, `cost_known`), lens `spent` (`budget.py spent-lens`) and classic `prep.py spent`. (e) done: flock. (f) done: `schema_min.py` fails closed on unknown keywords (all 10 bundle `templates/*schema*.json` covered). (g) already fixed in M2.
- **Floors, tree `3149235` (= the tree of `5a8be20`, all 4 patches)**: mock **163**, quick **251**, real **259**, `fix_settings` **218**, `fix_workspace` **110**, hardening 22 OK, all seven tool self-tests OK. Also: mock 163 on a copy of the live dirty tree with the merged `prep.py`; live `run_tests.py swarm-bundle` 5 PASS and `swarm-lib-runner-hardening` PASS. Mutations on `prep.py` (3 of 3 red, restored): ignore the packet sha, ignore the schema sha, unknown cost counts 0 (the `spent` row).
- **Floors for the first 3 patches** (exact landing tree, clean scratch): mock **163**, quick **251**, real **259**, `fix_settings` **218**, `fix_workspace` **110** (all unchanged); self-tests `record_run` OK, `schema_min` 15 rows, `budget` 30 rows, `record_verify` OK, `prep` OK. Live tree after landing: `run_tests.py swarm-bundle` 5 PASS, `swarm-lib` 7 PASS (hardening 70.7 s), `swarm-fix` 5 PASS.
- **Mutations (4 of 4 red, then restored)**: `run_complete` ignores the packet sha → `prep` self-test (missing-sha assertion); `os.replace` replaced by a copy → `record_run` self-test (atomic-write row); `_check_keywords` skips unknown keywords → `schema_min` rows `unsupported_top_level` and `unsupported_under_absent_property`; `run_one` reserves against a huge cap → 3 hardening tests (`test_cap_below_one_call…`, `test_reservations_bound…`, `test_lock_file_is_empty_after_a_failed_run`).
- **Deviations from the plan**: (1) the plan's `_complete` is the other session's uncommitted function; at HEAD the symbol is `run_complete`. (2) The flock is on `locks/map.pid`, not `map.lock`: `dr.py status` reads `locks/*.pid` and `dr.py` is not editable (dirty). The file holds the bare pid while held and is emptied (never removed) on exit. (3) `budget.py` locks a stable `budget.lock`, not `budget.json` (the state file is replaced atomically). (4) The worker added a deferred-unit requeue (`locks/map.deferred`): parallel reservations are conservative, so units refused at attempt 1 wait for the next wave; a wave where every unit is refused stops the run ("no room left"). Without it the 10-unit lens fixture (cap 2.0) aborted. (5) One existing selftest row's fixture changed, no count change: `dr_lib/selftest.py` "lens total cap" `DR_MAP_TOTAL_USD` `0.01` → `0.31`. A cap below one per-call cap (0.30) now runs nothing, which is the defect (c) fix; `test_cap_below_one_call_leaves_everything_pending_without_calling_claude` pins that. (6) The first landing was 3 of 4 patches (verified without `prep.py`: the old `prep.py` ignores the new record fields); `prep.py` followed as `5a8be20` (see above). (7) The plan said stop rather than hand-merge a conflict with the other session's hunk; the user's resume line said "Land the held prep.py patch first", so the worktree file was merged (committed content unaffected).
- **Behaviour changes to expect**: a total cap below one call runs nothing (before: one whole wave ran); near the cap fewer calls run in parallel; a timed-out or crashed call (no `total_cost_usd`) now counts a full `CALL_USD` in `prep.py spent` (before: 0); a packet rebuilt or a schema edited after a run makes that packet pending again.
- **Known holes (not closed)**: `dr.py` writes `locks/map.pid` with `os.replace`, which swaps the inode under the flock, so a direct `run_map.sh` racing a `dr.py`-started one is not excluded (the old `kill -0` scheme failed the same way; needs a `dr.py` edit). SIGKILL of `run_map.sh` while `claude` children live: the children keep the lock fd, so a new run is refused until they exit (intended). `--first-only` with a refused reservation reports "first packet did not complete".

## Canary CLOSED (2026-10-11, user said "yes canary", then "yes" to the $0.13 ceiling)
- **Run**: `DR_CLAUDE_STATE=ro python3 -I <clean git-archive copy of 21a7784>/dr.py run canary --run p2c --profile .claude/deep-review/phase3b/profile_p2.toml --yes-spend 0.13 --force --wait` (copy, not the live bundle: the live `dr.py`/`prep.py`/`run_map.sh` carry the other session's uncommitted hunks, and a paid run should test exactly the verified commit). Started 15:23:37Z, 31 s.
- **Two refusals before any spend** (no model call, `canary.json` byte-identical both times): (1) "canary already done (cost $0.0257)" because the stage state was `done` from phase 3d, so `--force` was needed (the plan's overwrite step); (2) the engine's pre-flight estimate is $0.03 to $0.125, above the $0.03 cap the user had approved, so I asked again before using the plan's `--yes-spend 0.13` fallback. The user said yes. Actual cost of this run: **$0.0076** (`cost_usd` in `canary.json`; the stage's cumulative counter went $0.0257 to $0.0333).
- **Result PASS** (`canary.json` sha256 `aa05ed73bdea…`, old one backed up in the session scratchpad as `canary.p2c.before.json`, sha `906190423dec…`): variant A all 11 checks true (`authenticated`, `claude_md_not_loaded`, `control_readable`, `no_home_leak`, `no_hooks_fired`, `no_marker_leak`, `no_mcp_servers`, `probe_attempted_all_steps`, `structured_output_present`, `tools_only_read_grep_glob`, `usage_recorded`), `leaked_markers` 0, budget probe `budget_enforced` true. `home_probe`: `claude_state: "ro"`, `os_wrapper_active` true, `os_home_hidden` true, `wrapper_sha256` `8f73842231b4…` (equal to the runlib's), HOME listing `.cache .claude .claude.json .local`. So `claude -p` authenticates and runs on the read-only `~/.claude` layout; the `/run/user` risk from the Session A watch list did not show up (auth worked).
- **Landed** bundle `f0fe56e` (tree `e7d4b42d…` == verified scratch tree `a52f346`, via the §1.4 index protocol; patch `~/.claude/swarm/backups/swarm_m2_session_d_canary_ro.patch`): `[sandbox] claude_state` default `"ro"` in `dr_profile.py` DEFAULTS (plus the `sandbox_env` fallback and two docstrings) and `PROFILE_SPEC.md` (default text, the "ro is NOT the default" sentence replaced with the canary evidence, the stale "fix_settings does not read this key" sentence corrected). Selftest rows: the sandbox-env default row now expects `ro`, new row for the `rw` opt-out (profile key and env var), the stub canary row expects `claude_state=ro`, `fix_settings` self-test pins `rw` explicitly for the rw-layout rows and has a new row that the profile default is `ro`. Counts on a clean copy: mock **163** (floor 162), quick **251** (250), full **259** (258), `fix_settings` **218** (217). Mutation (`DEFAULTS` back to `rw`, confirmed by `diff`): 2 mock rows red (the sandbox-env default row and `fix_settings` default row). Live tree after landing: `fix_settings` 218, `run_tests.py swarm-lib swarm-bundle-wrapper swarm-bundle-prep` 8 PASS, argv probe rc 0, `dr_profile.py check` OK on the KOTOR composite, `sandbox-env` prints `DR_CLAUDE_STATE=ro`.
- **Run gate under the new default** (live runlib, `DR_HIDE_HOME=1`): `dr_require_home_probe` on the new p2c `canary.json` returns 0 for `DR_CLAUDE_STATE=ro` and refuses `rw` with `home_probe.claude_state is 'ro' but this run uses 'rw' (rerun the canary in that state)`. Anyone who opts out with `claude_state = "rw"` must rerun the canary in that state (`dr.py run canary --force`).
- **Caveats to know (not blockers)**:
  1. **The fix half also defaults to `ro` now**: `fix_settings.claude_state()` reads the merged profile, so a profile without the key flips with `DEFAULTS`. The canary only exercised the `runlib.sh` layout (map/verify/...), not the fix half's own `bwrap_argv`. A live fix worker on `ro` is unproven until the next fix-prep/fix run; if it fails on auth or state writes, set `claude_state = "rw"` in the fix profile or `DR_CLAUDE_STATE=rw` for that run. `PROFILE_SPEC.md` says so.
  2. The no-profile, no-env fallbacks stay `rw`: `runlib.sh` `${DR_CLAUDE_STATE:-rw}` and `fix_settings.claude_state()` when `fc.P` is empty. Every `dr.py` stage exports the profile value, so only direct runner calls without `dr.py` hit them (and the run gate then refuses against the `ro` canary).
  3. The canary ran from a clean copy under the scratchpad, so the other session's wide-gate `pgrep` (which matches the live bundle path) would not have seen it. It was clear before and after.
  4. The p2c run's stage state was `done`; `--force` re-ran it. Any other run dir that holds an `rw` canary needs a rerun before its next real stage.

## Remaining swarm work (/plan-handoff, 2026-10-11, after Session C) [ALL DONE in Session D except the canary; see the Session D CLOSED block]
- **Direction (verbatim)**: "any remaining swarm work. delete the leftover doc file. fix any issues found."
- **Done before planning**: `.claude/handoffs/misrouted_swarm_m2.handoff.md` deleted (the user asked). Root cause of the `prep.py --self-test` failure found: `prep.py` ~line 498 (in the phase 3d baseline `a42a422`) applies `[tree].strip` to dirty and untracked files, so `agent_out/log.c` never becomes a unit; the self-test assertion `byf["agent_out/log.c"][0]["domain"] == "rest"` (~line 1312) is stale. With that one line changed to `assert "agent_out/log.c" not in byf` the whole self-test passes (rc 0, checked on a scratch copy).
- **Remaining M2 work**: Session D (D1 gate pins, D2 write scope, D3 profile and close; design in the earlier plan's §6, now inlined in the Session D plan), then §7 final report and the real canary (needs the user's go, cap $0.03).
- **Issues to fix**: (I1) stale prep self-test assertion above, plus register a `tests/suites.toml` suite so something runs it; (I2) VOID: the binary-rewrite worry was wrong (`build_tree` drops binary files before the copy; see the correction under Session C residuals); (I3) no selftest covers `dr_init`'s home-probe gate (`engine/verify/runlib.sh` ~line 289); (I4) `/var/tmp/dr_hygiene.*` cache files are never removed (`runlib.sh` ~line 135); (I5) the three selftests on the LIVE bundle once the other session commits or parks its edits.
- **Out of scope (own plans)**: M3 runner, M4 measurement (paid), M5 recipes; the frozen fork `.claude/review`; a forbidden-command union in the fix half (it has no such check, and its gate tests run `git commit` in clones).
- **Dead end (new)**: `plan_manager.py handoff --plan <x>` ALSO rewrites the shared, untracked `.claude/active_plan.txt` (script ~lines 758 and 822). It pointed at the swarm plan for a few minutes; restored to `patch_manager_mods/33_ui_queue_hotbar_fixes.md` (97 bytes, no trailing newline). `.claude/handoffs/swarm_library_architecture.handoff.md` (generated) now only points here.

## Session D CLOSED (2026-10-11)
- **Bundle commits (HEAD `21a7784`; each went through the index, committed without a pathspec, landed tree hash == the verified scratch tree; patches `~/.claude/swarm/backups/swarm_m2_session_d_<task>.patch`)**: R1 `096a5b0` (stale prep self-test assertion; code unchanged), R3 `b53d440` (3 rows: `run_verify.sh` through `dr_init`'s home-probe gate), R4 `f4177f0` (hygiene args in exported `_DR_HYGIENE_ARGS` + `_DR_HYGIENE_READY`, no `dr_hygiene.*` temp file), R5 `c16a88b` (NEW issue: the selftest left a `/tmp/dr-selftest-*` tree after every real run, because the fix half's review copies are chmod a-w; cleanup now chmods first), D1a `3a0a87e` (`[fix].pinned`, `base.json["pins"]`, `fix_workspace add-pins`, `fcommon verify-pins` and `pin-closure`), D1b `0cac882` (`fixgate.sh` verifies pins before every verb except `reset`, exit 2; integrator settings deny Edit/Write per pinned path), D2 `21a7784` (`[fix].strict_scope`, `approved_scope.json`). Project: `8ba3029` (`tests/suites.toml`: `swarm-bundle-prep`). LIB unchanged (`d4f9834`).
- **Profile (untracked project data, not committed as before)**: `.claude/deep-review/phase3f/fix_section_kotor2.toml` and the composite `profile_fix_kotor2.toml` got `pinned` = 11 files (`scripts/{run_tests,build_check,compat_check,agent_diff_check,_common,_static_common,check_exports,compat_graph,master_ledger,agent_contract}.py` + `tests/suites.toml`; the 10 scripts are exactly the `pin-closure` output for the four gate entrypoints). `dr_profile.py check` OK. The composite was NOT regenerated: it is stale against `phase3b/profile_p2c.toml` (budgets, `reduce_group`, `[coverage]` changed there since), so `build_profile.sh` would drag unrelated changes in; the key was added by hand to both files.

| Stage | Floor (clean `58deafa`) | After Session D (clean copy of `21a7784`) |
|---|---|---|
| selftest mock | 160 | 162 |
| real quick | 245 | 250 |
| real full | 253 | 258 |
| `fix_workspace --self-test` | 77 | 110 |
| `fix_settings --self-test` | 206 | 217 |
| `prep.py --self-test` | FAIL (stale assertion) | OK |
| `fcommon`, `fix_intake` self-tests | PASS (no count printed) | PASS |
| `fix_checks --self-test` | 31 | 31 |

Also green: LIB loop (7 files, `test_vendor_parity` 0 skips) and `run_tests.py swarm-lib swarm-bundle-wrapper swarm-bundle-prep` (8 PASS) on the live tree, `swarm_argv_probe.py` rc 0 IDENTICAL, `dr_profile.py check` on the KOTOR composite.

| ID | Mutation (scratch copy, confirmed by `diff`) | Result |
|---|---|---|
| M-R1 | revert the prep assertion | prep self-test `KeyError: 'agent_out/log.c'`, rc 1 |
| M-R3 | delete the `dr_init` home-probe gate block | quick 3 of 248 FAIL (exactly the 3 new rows) |
| M-R4 | restore the old `mktemp` block | quick 2 of 250 FAIL (no-leak row, once-per-tree row) |
| M-R5 | old `rmtree(ignore_errors)` cleanup | one more `/tmp/dr-selftest-*` per real run (118 to 119) |
| M-D1a1 | `verify_pins` skips the sha compare (worker) | `fcommon` and `fix_workspace` self-tests red |
| M-D1a2 | drop the base.json pins union (gate-time, no profile) | `fcommon` and `fix_workspace` red (edited file passed with rc 0) |
| M-D1b1 | drop `exit 2` after `verify-pins` in `fixgate.sh` | `fix_workspace` self-test red (10 rows per the worker; mine: `fixgate status, edited pinned file: rc=0`) |
| M-D1b2 | drop the `extra_deny` pass-through (worker) | 6 `fix_settings` rows red |
| M-D2 | strict branch back to `allowed = [primary] + cited` | `fix_intake` self-test red (cited file in `allowed_files`, waves, approved rows) |

- **Choices and deviations**:
  1. D1 was split into D1a (profile, `fcommon`, `fix_workspace`) and D1b (`fixgate.sh`, `fix_settings`, gate-level tamper rows): narrow workers.
  2. **Review caught a fail-open in D1a before landing**: the installed `fcommon.py` runs at gate time with no `DR_PROFILE`, so `PINNED` was empty and `verify-pins` passed silently. Fix: with no explicit list, the pins recorded in the host-side `base.json` also define the checked set (profile list and base.json keys are unioned). Also the installed copy could not import `dr_profile` (it never read `bin/engine_dir.txt`, only `fix_settings.py` did): `fcommon.py` now reads it too. Both are pinned by tests (`fcommon` unit rows, `fix_workspace` rows with `DR_PROFILE` removed and via `fixgate.sh`).
  3. R4 treats an empty or newline-bearing hygiene arg as "unavailable" (same WARNING as a python failure, no READY marker, so a later call retries).
  4. Two workers slipped a whitespace-only hunk into existing lines (`repo =Path(`, `: >"$LEDGER_DIR`); both reverted in review. One worker used a Python heredoc for an edit.
  5. D2 `strict_scope` defaults to true (opt out `strict_scope = false`): the write scope of needs-review and needs-design tasks shrinks to the primary file; cited files are read context. `approved_scope.json` = `{task_id: [paths]}` in `$FIX_DIR/work/`. There is no real bundle queue, so there is no queue-change list.
  6. D3.2: no real bundle fix home exists (`~/.cache/deep_haiku_review/*` has none), so the pins and the new `fixgate.sh` apply at the next fix-prep (`build-base` records pins and installs bin). For an existing base: `fix_workspace.py add-pins` then `install-bin`.
  7. I5: the live bundle is STILL dirty (the other session's `PROFILE_SPEC.md`, `SELFTEST.md`, `dr.py`, `engine/{dr_profile,merge,prep}.py`, `engine/run_map.sh`, `engine/verify/verify_merge.py` hunks), so the three selftests ran on a clean `git archive HEAD` copy; LIB loop, suites, argv probe and `fix_settings` ran on the live tree. `swarm-bundle-prep` is green on the live tree too (the other session's `prep.py` hunk is compatible).
- **Workers**: 5 Sonnet (R3, R4, D1a, D1b, D2), no commits by workers; processed tokens 2.82M / 2.02M / 2.74M / 2.24M / 1.99M (11.8M total). R1 and R5 were inline (Opus).
- **Residuals**: (a) ~~`scripts/_common.py` untracked~~ DONE: committed `c245d79` (the user said "commit it"; 64 lines, imported by `run_tests.py` and 9+ other scripts; a clean `git archive` import check passed). Note for the next `build-base`: `compute_pins` reads blobs at the base commit, so the base must be a commit that contains it (HEAD or later). (b) ~~About 118 older `/tmp/dr-selftest-*` dirs and about 320 old `/var/tmp/dr_hygiene.*` files not cleaned~~ DONE 2026-10-11: swept 122 `/tmp/dr-selftest-*` dirs (`du -sm` summed 378 MB; the earlier survey said about 293 MB, the 4 newest were kept workdirs from failing selftests, fixed in `f0fe56e`) and 318 `/var/tmp/dr_hygiene.*` files (about 1.2 MB). Gate (`pgrep`) was empty first; filter was own user, maxdepth 1, `-mmin +15`; `chmod -R u+w` before `rm -rf` because the fix half's review copies are mode a-w. Counts are 0 now; new ones appear only after a failing selftest keeps its workdir (R5) or a pre-R4 process (R4). (c) `pin-closure` sees static imports only (no dynamic imports or subprocess calls): re-run it when a gate script changes. (d) Pins are checked at the start of each fixgate verb: a gate script that rewrites a pinned file during one verb is caught by the next verb, not by that verb's own result. (e) A base built before D1 has no pins: with no profile at gate time the check is off until `add-pins`; with the profile loaded, missing pins fail closed. (f) The fix half still has no forbidden-command union (own plan). (g) ~~`claude_state` stays `rw` until the canary passes~~ DONE: canary PASS, default is `ro` (see the Canary CLOSED block above).
- **Next**: ~~real canary~~ done; M3 runner (re-plan against phase 3d first), M4 measurement (paid), M5 recipes: separate plans.

## Session C CLOSED (2026-10-11)
- **Commits**: LIB `d4f9834` C1 (`lib/scrub.py`, `tests/test_scrub.py`, 17 tests). Project `cd80fb5` (`tests/suites.toml`: `swarm-lib-scrub`; `run_tests.py swarm-lib` is now 6 PASS). Bundle `cea6282` C2 (forbidden union: `engine/invariants.py` vendored, one helper `dr_profile.forbidden_hits` used by `dr.py`, `vcommon.py`, `fact_runner.sh`). Bundle `58deafa` C3 (`[protected] scrub`, default true; `engine/scrub.py` vendored; `prep.py` sites `file_diff`, whole-file read, `build_tree`).
- **Landing**: both bundle commits went through the index (patch `apply --cached`, then `apply` to the tree, commit without a pathspec). Patches: `~/.claude/swarm/backups/swarm_m2_session_c_c2.patch`, `..._c3.patch`. The verified scratch tree (`a58f78c`) equals the live `cea6282` tree.
- **Workers**: 3 Sonnet (C1, C2, C3), no commits by workers. Processed tokens 168k / 1.97M / 2.29M.

| Stage | Floor (Session B close) | After C2 | After C3 (clean copy of `58deafa`) |
|---|---|---|---|
| selftest mock | 147 | 160 | 160 |
| real quick | 230 | 242 | 245 |
| real full | 238 | 250 | 253 |

Also green at `58deafa`: LIB loop (scrub 17 tests), `test_vendor_parity` 0 skips, `run_tests.py swarm-lib` (6) and `swarm-bundle-wrapper`, `fix_settings.py --self-test` 206 checks, `swarm_argv_probe.py` rc 0 IDENTICAL.

| ID | Mutation (scratch copy, confirmed by diff) | Result |
|---|---|---|
| M-C1 | drop the `_STEAM.sub` line in `scrub.py` | 2 FAIL (`test_steam64`, mixed sample) |
| M-C2a | `forbidden_hits` ignores the built-in list | mock 4 of 84 FAIL (`git -C x commit`, `git reset`, 2 in-process) |
| M-C2b | `vcommon.forbidden_hit` legacy-only | quick 2 FAIL (`check-forbidden`, `vcommon --self-test`) |
| M-C3a | `DEFAULTS["protected"]["scrub"]` False | quick 5 FAIL |
| M-C3b | `build_tree` back to `shutil.copy2` | quick 3 FAIL |
| M-C3c | `file_diff` result not scrubbed | quick 4 FAIL |

- **Choices and deviations**:
  1. C2 pre-gate: 5 literal hits, all test/comment/docs or fix-half help text (`engine/fix/fix_series.py:190` prints `git add` as advice). No engine-built stage argv or the `vcommon` claude preview holds a deny word. The fix half (`engine/fix/`) is NOT covered by C2.
  2. `dr_profile.py` appends its own engine dir to `sys.path` before `import invariants`: `python3 -I engine/dr_profile.py` does not put the script dir on `sys.path`.
  3. `_check_protected(...)` is called BEFORE `_check_lenses` (the plan said after): the other session's `_check_coverage` call sits on the line after `_check_lenses`, so reordering avoided an overlapping hunk. Patches built with `-U1`.
  4. `fact_runner.sh` error text is now "forbidden command(s) in ..." (no check pinned the old text). The "not listed in the profile" proof uses `git reset --hard`, because the example profile already lists `git push`.
  5. C3 checks are real-only (the mock engine's prep copies the repo verbatim), so the mock count did not move in C3. `STUB_SECRET_PATTERN` is widened by alternation (old pattern kept) and is in the env-restore tuple. The placeholder check requires both `/home/USER` and `REDACTED_TOKEN`.
  6. **Gate lesson**: the plan's `pgrep` pattern did not match a live paid `dr.py ... run adjudicate --inline` (profile p2c, other session, `--yes-spend 0.44`). C2 waited until it exited. Use `pgrep -af 'run_fix|run_map|run_integrate|fix_checks|dr\.py|run_adjudicate|run_verify|run_reduce|prep\.py' | grep -v pgrep | grep -F "$B"` (filtered to the live bundle path, so scratch-copy selftests do not trip it).
- **Finding in existing code (not Session C)**: `python3 -I engine/prep.py --self-test` fails at the pristine pre-session HEAD `f15707f` with `KeyError: 'agent_out/log.c'` (`prep.py` ~line 1312, `byf["agent_out/log.c"]`). It is in no selftest or suite. Not investigated.
- **Residuals**: (a) CORRECTED 2026-10-11: no residual. `build_tree` already drops binary files (extension, size, or a NUL in the first 8 KB via `looks_binary`) before the copy loop, so only text-like files are scrubbed. (b) The live-tree selftests were NOT repeated: the other session's uncommitted edits (`dr.py`, `engine/dr_profile.py`, `engine/merge.py`, `engine/prep.py`, `engine/run_map.sh`, `engine/verify/verify_merge.py`, `PROFILE_SPEC.md`, `SELFTEST.md`) still sit on top of my commits. Run the three selftests on the live tree after it commits or parks; evidence until then is the clean copy of `58deafa`. (c) `scrub` defaults on, so packet text and fix BASE seeding change (a fix patch touching a scrubbed line fails to apply); opt-out is `scrub = false`.
- **Next**: Session D (D1 gate pins, D2 write scope, D3 profile and close), then §7 canary (needs the user's go, cap $0.03).

## Session C status (2026-10-11, before clear) — SUPERSEDED by the section above, kept for history
- **Session B CLOSED**: bundle HEAD `f15707f` (B3b), `50c05a2` (runner-order fix), `a74722a` (B2d), `b15884e` (B2c), `f353b90` (B2b), `2dcc917` (B2a), `fc8d54e` (B3a), `ad0b569` (B3c). Project handoff commits `7141a3e`, `48937ca`, `fdf5506`, `4a33d57`.
- **LIB** (`~/.claude/swarm`) HEAD `7023abd`, clean. `lib/scrub.py` does NOT exist yet.
- **C1 is unblocked**: LIB-only (`~/.claude/swarm/lib/scrub.py`, `~/.claude/swarm/tests/`, one line in project `tests/suites.toml`).
- **C2 and C3 are BLOCKED** in the live bundle by another session's uncommitted edits: `engine/dr_profile.py` (C2 and C3 both touch it), `engine/prep.py` (C3), `engine/merge.py`, `engine/run_map.sh`. Its `lens_done` change also breaks the live chain test. Do C2 and C3 in a scratch copy at HEAD until that session commits or parks its work.
- Session C resume phrase: `swarm M2 safety, Session C (C1)`. Do NOT use `plan_manager.py resume` or `handoff` for this work (see Dead Ends below).
- **Dead ends for this work**: `plan_manager.py handoff "<text>"` writes the ACTIVE plan and rotates the shared `.claude/session_handoff.md`; `mark-done` has no matching master-plan phase; bare `git commit` or `git add -A` in the project sweeps other sessions' files (use explicit paths); editing the live bundle while a `run_fix|run_map|run_integrate|fix_checks` process is live (gate: `pgrep -af 'run_fix|run_map|run_integrate|fix_checks' | grep -v pgrep` must print nothing).
- **Decided**: AGENTS.md stays without the agy backend line.

## Session B2 to B4 (2026-10-11)
- Bundle commits (`~/.claude/skills/deep-haiku-review/bundle`, master):
  - `ad0b569` B3c: fix runners export `DR_SANDBOX_HANDLED=1` inside their own bwrap shim
  - `2dcc917` B2a: `dr_wrapper_sha256`, `dr_require_home_probe`, `dr_canary_os_probe`; `DR_CLAUDE_BIN` dir bound
  - `fc8d54e` B3a: `fix_settings` hygiene args, `claude_state` rw|ro, hide-off gate mask, 7 extra deny rules
  - `f353b90` B2b: canary writes `wrapper_sha256`, `claude_state`, `env_hidden`, `run_dir_hidden`
  - `b15884e` B2c: `run_map.sh` and `run_reduce.sh` call `dr_require_home_probe`
  - `a74722a` B2d: selftests for the run gate, the canary OS probe and the bound custom binary
- `50c05a2` runner-order fix: the home-probe check in `engine/run_map.sh` and `engine/tools/run_reduce.sh` now runs after `dr_sandbox_prefix`, so `DR_ALLOW_UNSANDBOXED=1` waives it (before: both runners refused with the override set). Plus the regression check in `dr_lib/selftest.py`. Committed through the index only; the other session's `run_map.sh` hunk is still uncommitted in the working tree.
- `f15707f` B3b: `templates/worker_settings.json` denies `~/.git-credentials` and `~/.claude.json`; `dr_lib/selftest.py` static checks for the fix runners and `fix_settings --self-test`. Committed with explicit paths; the committed files match the verified scratch copies byte for byte. (Patch copy: `~/.claude/swarm/backups/swarm_m2_pending_b3b_20261011.patch`, now applied.)
- Verified on a scratch copy at HEAD plus the two uncommitted pieces (`/tmp/.../scratchpad/bcheck`): quick 230/230, full 238/238. Before B3b: quick 227, full 235. Pre-fix, the new override check fails on both runners.
- **Shared-tree conflict**: the live bundle has another session's uncommitted edits to `engine/run_map.sh`, `engine/prep.py`, `engine/merge.py`, `engine/dr_profile.py` (mtime 00:29, 2026-10-11). Its `lens_done` now calls `prep.py is-complete --lenses`, which `prep.py` does not accept, so the live chain test fails (`core-001:logic`). Not touched. The fix must be committed through the index (only my hunk) or after that session commits.

## Counts (floors 128 / 203 / 211; all met)
| Stage | Before B2 | After Wave 2 and 3 (HEAD `b15884e`) | B2d on scratch | + runner fix | + B3b |
|---|---|---|---|---|---|
| selftest mock | 128 | 128 | 147 | 147 | 147 |
| real quick | 203 | 203 | 226 | 227 | 230 |
| real full | 211 | 211 | 234 | 235 | 238 |

The mock engine skips the runner section (`if kind != "real"`), so the mock count does not move with the runner checks.

## B0 and B4 mutation checks (each must turn its test red)
| ID | Mutation | Result |
|---|---|---|
| M-a | delete `DR_SBX+=("${H[@]}")` (runlib.sh) | FAIL `test_hygiene_checks_pass` |
| M-b | old stub rule in `dr_is_stub` | FAIL `test_custom_non_stub_binary_is_wrapped` |
| M-c | delete the unprimed-parent gate | mock 6 of 128 FAIL |
| M-B2a | delete the wrong-hash `if` in `dr_require_home_probe` | quick 1 FAIL (wrong-hash row) |
| M-B2b | `env_hidden=true` unconditional in `dr_canary_os_probe` | quick 1 FAIL (bare-bwrap control) |
| M-B2c | delete `run_map`'s home-probe call | quick 1 FAIL (run_map WARNING row) |
| M-B3a | delete `a += sh.hygiene_args()` in `fix_settings` | `--self-test` FAIL (hygiene placement) |
| M-B3b | delete `~/.kube/**` from `HOME_DENY_EXTRA` | `--self-test` FAIL (deny_list) |
| M-B3c | delete one `DR_SANDBOX_HANDLED` export (`run_fix_review.sh`) | quick 1 FAIL (static check) |

- Deviation: the plan's M-B2c deletes the call in `dr_init`, but the check it names covers `run_map`'s call, so that deletion cannot turn it red. I mutated `run_map` instead. **Residual gap**: no selftest covers the `dr_init` home-probe gate.
- The first M-a attempt applied nothing (bad grep escape) and passed; re-run with `grep -F` turned it red. The first M-B3b attempt matched two occurrences; re-run with a unique anchor.

## Verification (live tree, 2026-10-11)
- LIB tests: `test_bundle_wrapper`, `test_hygiene`, `test_invariants`, `test_sandbox`, `test_home_probe` all OK; `test_vendor_parity` OK with 2 skips (expected until Session C).
- `run_tests.py swarm-lib`: 5 passed. `run_tests.py swarm-bundle-wrapper`: green.
- `scripts/swarm_argv_probe.py`: rc 0, bundle and fork default argv IDENTICAL, control PASS.
- The live real quick and full selftests are NOT green while the other session's `lens_done` edit is in place (see the shared-tree conflict above). Scratch results above are the evidence.

## Deviations and residuals
- `claude_state` lowercases its value (`RO` counts as `ro`). The `~/.claude.json` mask applies to the gate only, since the claude profile binds it on purpose.
- The B2a worker added a dict guard around `canary.json` parsing (a JSON list gives the "no home_probe" line, not a traceback).
- Residual: no selftest for `dr_init`'s home-probe gate (above).
- Residual: §7 real canary not run (needs a go, cap $0.03). `claude_state` stays `rw` until it runs.
- Residual: `/var/tmp/dr_hygiene.*` cache files are not removed at exit (unchanged from B1).
- Harness flag: the B3a worker's report matched an instruction-pattern filter ("settings-json"). The report contained no directives; reviewed by hand.

## Next
- **Session B is closed** at `f15707f` (bundle HEAD). Nothing from Session B is pending. The older combined patch `swarm_m2_pending_20261011.patch` and the B3b patch are superseded by the commits.
- **Still open, not Session B**: the other session's uncommitted edits in the live bundle (`engine/dr_profile.py`, `engine/merge.py`, `engine/prep.py`, `engine/run_map.sh`). Its `lens_done` change breaks the live chain test until it is finished or parked.
- Verified at HEAD `50c05a2` on a clean copy (other session's files reset in the copy only): real quick 227/227, real full 235/235. B3b on scratch on top of that: quick 230/230, full 238/238.
- After B3b lands: run the three selftests on the live tree once the other session's `lens_done` edit is fixed, then update this section's status.
- Decided (2026-10-11): AGENTS.md stays without the agy backend line. No re-add. The Claude Code policy line at HEAD `fdf9aff` stands. Agy policy, if wanted later, is a separate decision.
- Then Session C (C1 scrubber in LIB, C2 forbidden union, C3 scrub in bundle), then Session D, then §7 canary with the user's go.

## Session B: B1 done (bundle `~/.claude/skills/deep-haiku-review/bundle`)
- Bundle commits: `a42a422` baseline, `1926dcf` vendor hygiene, `6cb7c09` B1a hygiene args after the HOME tmpfs, `9441ced` B1b fail-closed gate and stub-by-`--version`, `12c5421` B1c `claude_state` rw|ro (default rw), `61dfbc8` B1d selftests and export fix.
- Verified after B1d: mock 128/128, real quick 203/203, real full 211/211 (A0 baselines 115 / 190 / 198). `swarm-bundle-wrapper` GREEN (7/7).
- Bugs found and fixed in B1: `dr_sandbox_gate` was missing from `export -f` (xargs children of a stub run exited 1, map stage failed). Unprimed parents (`run_map`, `run_reduce`, `canary_test`, `subagent_probe`) ran unwrapped and silent when bwrap was broken; `dr_sandbox_prefix` now runs the gate itself and exits 1.
- Deviations from plan: B1 item 5 (`DR_SANDBOX_HANDLED=1` exported by `run_fix*.sh`) moved to B3 (its scope). `DR_CLAUDE_STATE` is env-wins in `sandbox_env`, unlike `DR_HIDE_HOME` (profile-wins). The `DR_SANDBOX=none` dry-run check now passes `DR_ALLOW_UNSANDBOXED=1`, the explicit opt-out.
- B0 mutation check (2026-10-10, scratch copy of bundle + LIB, control 7/7 OK), each mutation confirmed applied by diff:

| ID | Mutation (runlib.sh) | Result |
|---|---|---|
| M-a | delete `DR_SBX+=("${H[@]}")` (line 144) | FAIL `test_hygiene_checks_pass` |
| M-b | `dr_is_stub` last line -> old `DR_CLAUDE_BIN != claude` rule (line 61) | FAIL `test_custom_non_stub_binary_is_wrapped` |
| M-c | delete the unprimed-parent gate (line 117) | mock selftest 6 of 128 FAIL, all `dr_sandbox_prefix` unprimed-caller checks |

- Note: the first M-a attempt had a bad grep escape, so it applied nothing and passed; re-run with `grep -F` turned it red. Result above is the valid run.
- Residual: `/var/tmp/dr_hygiene.*` cache files are not removed at exit (one small file per process tree).
- Residual: `fix_settings.py` bwrap does not read `claude_state` (B3 item 1).

## Next phase: B2 to B4 (then C, D, final report)
- B2 canary and run gate, B3 fix half and deny lists, B4 close. Plan §4 onward.
- Gate before any bundle edit: `pgrep -af 'run_fix|run_map|run_integrate|fix_checks' | grep -v pgrep` prints nothing.
- No paid calls. Final canary (§7) needs the user's go.

---

# Swarm M2 safety: Session A handoff (2026-10-10)

Resume (historical): `swarm M2 safety, Session B (B1)`. Plan: Swarm M2 safety re-scope (post phase 3d), §4 Session B.

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

## M4 measurement (2026-10-12, pilot scale)
- Done: resolver `ac551e2`, scorer `63ecdb6`, cases + rubric `e4a188c` `fc03108` `58b1fda`. Paid $0.704 total (hotbar $0.041, BG09 $0.058, Sonnet verify $0.604); user yes at G1/G2 ($0.45 each) and G3 ($1.00).
- Result and numbers: `.claude/reviews/bug_investigate_m4_measurement.md`. Headlines: hotbar precision_hit 0.277 [0.169, 0.418] (n=47), rank-1 HIT 5/5; BG09 0.063 [0.017, 0.201] (n=32), rank-1 HIT 0/5; null `insufficient` 10/10; advocate valid 27/28 with 0 short ids (pilot truncation did not recur; resolver never exercised live); verify layer covered only 28/79 hypotheses, rejected 2 (both MISS), abstained 22, about $0.022/unit.
- D3 stays "layer 3 off". Verify per-call cap reservation left most units pending: fix the caps before any further verify run.
- Run data in `~/.cache/bug_investigate/20261012T0400..0411Z-logic` (10 dirs, score.json each).

## M4 session review (2026-10-13, 8 Haiku recon, about $0.62 est.)
- Review note: `.claude/reviews/bug_investigate_m4_session_review.md`; raw lens outputs `.claude/reviews/bug_investigate_m4_review/u1.md`..`u8.md`.
- Verdict: mechanics fine (68/68 units, $0.704); validity weaker than the M4 note; verify layer 28/79 covered, 51 pending silently (cap math: `cap_total >= N x cap_call`).
- Fix packages R1..R6 (verify budget, stage integrity, validation gates, scorer validity, measurement protocol and docs, tooling) are tracked in `.claude/plans/swarm_library_architecture.md` section 18; implementation plan: see that section for the plan file path.
- Caution: M4 note claims "2 of 79 removed", "correct rejections", "null abstains correctly" overreach (2 of 28 verified; labels are rubric labels). Rubric agreement 8/8 was in-sample; held-out was 7/8.
- Standing for R-work: bundle `5a8be20` and LIB `651047e` untouched by this review; any LIB change (budget preflight in `swarm_run.py`) is a separate go.
