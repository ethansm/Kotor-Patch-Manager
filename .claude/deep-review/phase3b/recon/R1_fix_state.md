# R1 fix-session state (F0), 2026-10-10, read-only

## 1. Inventory
- `.claude/review/fix/`: 27 files, about 9,000 lines. Largest: fix_ladder.py 1018, fix_workspace.py 990, fix_checks.py 951.
- Subdirs: checks/ integ/ judge/ patches/ raw/ results/ reviews/ specialty_notes/ (results/ holds 209 files).
- `.claude/fixes/batch_01`, `batch_02`: directories exist; contents not enumerated.

## 2. Open bugs (numbering from deep_haiku_review_skill.handoff.md)
- Bug 13 (fixgate reset deletes _patches/): FIXED in code. fixgate.sh:95 `git clean -fdxq -e _patches -e _resolved`.
- Bug 12 (KPM hidden from nested gate): no fix note found. fix_settings.py:116-119 ro-binds KPM_DIR in gate profile, so probably fixed. Unverified. phase3b_impl.md:25 still lists it open.
- Bug 9 (settings dir hidden in bwrap_argv): FIXED. fix_settings.py:122-125 ro-binds WORK/settings in claude profile; self-test at :194-195.
- fix_pipeline_plan.handoff.md:33-60 numbers only bugs 1-8 (all fixed); it has no bugs 9/12/13.

## 3. Pilot and calibration
- Fix pilot DONE: fix_pipeline_plan.handoff.md:103. Canary variant A pass; calibration catch 5/5; false-reject 0/3; 0 fabricated quotes.
- Cost about $0.59 (phase3b_impl.md:25). Canary data: .claude/review/fix/canary.json.
- `.claude/review/fix/fix_canary_probe.md` (6 lines) is a probe note, not a result.
- `.claude/deep-review/p1/PILOT_REPORT.md` is the review pilot, not the fix pilot.

## 4. Code checks
- bwrap_argv settings handling: matches bug 9 request. Gate profile (fix_settings.py:110-119) does not expose settings; claude profile does (:122-125).
- fix_canary.sh (at `.claude/review/tools/fix_canary.sh`, not fix/): lines 174-176 are deliberate. Line 174 is a guard, line 176 appends the `--` separator before claude. Not a leftover.
- bwrap_argv ends with `--die-with-parent` (fix_settings.py:133), so the guard passes.

## 5. Stage to file
- fix-intake: fix_intake.py
- workspace: fix_workspace.py
- canary: .claude/review/tools/fix_canary.sh (+ canary.json)
- fixer: run_fix.sh, fix_ladder.py, fixer_prompt.md
- check: fix_checks.py, fixgate.sh
- review: run_fix_review.sh, reviewer_prompt.md, review_schema.json
- integrate: run_integrate.sh, build_integration_batches.py, integrator_prompt.md
- merge: fix_merge.py
- Also: fix_settings.py (settings and sandbox), fix_calibrate.py, run_fix_judge.sh, record_fix.py, fcommon.py
