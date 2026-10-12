# WI-66b Bug-investigate per-stage reliability and playtest bundle check
area: bug-investigate
size: S
ready: ready
items: C398,C396
files: scripts/bug_investigate_score.py
out-of-repo: none
blockers: C396 is verify-only: if a rerun shows unstable ids, leave the row open. Run dir for the rerun: unknown. Suite name: unknown. C398 and C396 ids are '-', so mark-done phase ids are unknown. STATE values unknown. Split from WI-66 because it edits a different file.
docs: .claude/plans/swarm_library_architecture.md:C398,C396

## Goal
Add per-stage reliability tracking to the bug-investigate score (C398), and verify the playtest bundle's ids, scrubbing and size cap (C396).

## Steps
1. C398 in `scripts/bug_investigate_score.py`: track reliability per stage. `STAGES` is ("blind", "null", "verify") (line 28). Reuse `proportion(k, n)` (line 82), which already returns a Wilson 95% interval via `wilson()` (line 71). Never report a bare point estimate. Baseline from the plan (line 179): 4 invalid quotes in about 45 (ladder_experiment.md:34).
2. C396 VERIFY ONLY in `scripts/playtest_bundle.py`. Verified: evidence ids are content-derived (`ev:<log>:L<a>-<b>:<sha8>`, and `bundle_sha256`, lines 32-33). `scrub_text` is applied (line 19). The size cap defaults to 49152 bytes (line 13). Confirm stability by rerunning on one run dir. If ids differ between runs, note it in NOTES and leave C396 open.

## Verify
1. `python3 scripts/run_tests.py <filter>`: suite name unknown.
2. `python3 scripts/build_check.py`
3. Rerun the playtest bundle on one run dir (dir unknown) and compare ids.

## Docs
- After each row passes: `python3 scripts/plan_manager.py mark-done <phase> --state <STATE> --notes "<evidence>"`. C398 and C396 have id '-', so the phase id is unknown: report instead of guessing. Never sed the plan doc.
- CHANGELOG: none named in TSV.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C396 .claude/plans/swarm_library_architecture.md:127 id=- open
- C398 .claude/plans/swarm_library_architecture.md:179 id=- open
