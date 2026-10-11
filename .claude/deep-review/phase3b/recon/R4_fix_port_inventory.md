# R4 recon: porting the fix half (S9-S14) into deep-haiku-review

Intended output path (not written, plan mode): .claude/deep-review/phase3b/recon/R4_fix_port_inventory.md (dir already created)

## (1) Target stages
Plan Phase 5 names 8 stage names but labels them S9-S14 (6 slots). Open question below.

| Stage | Input | Output |
|---|---|---|
| fix-intake (S9) | verified_findings.json incl. control-labelled entries | clustered batches, class: auto / needs-review / needs-design / human-only |
| fix-workspace | base copy outside repo, binary overlay, secrets excluded | isolated per-task copies |
| fix-canary | workspace | canary pass/fail (must pass) |
| fix (S10) | batch + allowed-file list | patches/*.patch (Haiku; Sonnet for needs-design), bwrap, Edit/Write only |
| fix-check (S11) | patches | apply/scope/assertion-delta/skip-scan/build+tests, new failures only |
| fix-review (S12) | patches | blind Haiku review + seeded-bad calibration, one revise loop |
| fix-integrate (S13) | <= 8 patches per batch | Sonnet integration, allow-listed wrapper, script re-gates |
| fix-merge (S14) | all batch results | fix_report.json/md, exit 1 on gaps; per-batch human apply |

## (2) Wiring points (bundle = ~/.claude/skills/deep-haiku-review/bundle)
- dr.py:54-70 STEPS list: append fix-* entries (name, paid, detach, label, needs, gate_after)
- dr.py:73 GATES: add fix-go, fix-pilot, fix-apply (human) and spend-fix-* if paid
- dr.py:18 usage text "fix-* stages: phase 2, not built" -> update
- dr.py:911 cmd_next done message "fix stages are phase 2, not built" -> update
- dr.py:1040-1041 cmd_run hard-rejects names starting "fix-" -> remove/route
- dr.py:1234 selftest parser (--dirty); dr.py:1190 cmd_selftest -> dr_lib/selftest.py main
- dr_lib/selftest.py:52 engine_dir mock fallback; :135 make_fixture call; :138 error text
- references/30-fix.md: 6-line placeholder, replace with real reference
- engine/ has no fix/ dir yet (plan said engine/fix/* phase 2); templates/ has no fixer/reviewer/integrator prompts

## (3) [fix] profile fields (PROFILE_SPEC.md:132-145)
- never_edit = [] (globs)
- human_only = [] (globs)
- design_specialties = [] (specialties needing Sonnet fixer)
- sandbox_tests_only = [] (globs)
- gate_semantics = [] (globs needing human sign-off)
- max_lines = { auto = 40, review = 60, design = 120 }
- unit_regex = "" (default: first path component)
- apply_order = [] (regexes)
- [fix.gates]: build = [], test = [], compat = []
- Related budget keys: fix_call_haiku = 0.15, fix_call_sonnet = 0.60, fix_total = 25 (PROFILE_SPEC.md:105-107)

## (4) Proposed split (7 tasks, each <= 3 files)
| Task | Scope | Model | Deliverable | Verify |
|---|---|---|---|---|
| F-a | engine/fix_intake.py, engine/fcommon.py | Haiku | S9 intake + classes | python3 -I -m py_compile on both |
| F-b | engine/fix_workspace.py, engine/fixgate.sh | Sonnet | sandbox + gate design | fixture workspace builds, secrets absent |
| F-c | engine/fix_checks.py, engine/run_fix.sh | Haiku | S10/S11 fixer + checks | py_compile + selftest --engine mock |
| F-d | engine/run_fix_review.sh, templates/reviewer_prompt.md, templates/review_schema.json | Haiku | S12 review + calibration | seeded-bad calibration catches it |
| F-e | engine/build_integration_batches.py, engine/run_integrate.sh, templates/integrator_prompt.md | Haiku | S13 batches <= 8 | batch count and size asserts |
| F-f | engine/fix_merge.py, references/30-fix.md | Haiku | S14 report + exit codes | fix_report.json schema check |
| F-g | dr.py, dr_lib/selftest.py, fixture/make_fixture.py | Sonnet | driver wiring, gates, seeded fixable bugs, stubs | python3 -I dr.py selftest --engine mock (in bundle dir) |

Note: the task brief allowed a Sonnet role only for sandbox/gate design and driver wiring; F-b and F-g match that.

## (5) Fix-session sources to copy (names only, under .claude/review/fix/)
- Scripts: fix_intake.py, fix_checks.py, fix_workspace.py, fix_merge.py, fix_ladder.py, fix_calibrate.py, fix_settings.py, fcommon.py, record_fix.py, build_integration_batches.py, fixgate.sh, run_fix.sh, run_fix_review.sh, run_integrate.sh, run_fix_judge.sh, fix_prompts.py
- Prompts/schemas: fixer_prompt.md, reviewer_prompt.md, integrator_prompt.md, judge_prompt.md, fix_canary_probe.md, fix_schema.json, review_schema.json, integrate_schema.json, judge_schema.json, canary.json
- Data dirs (fixtures/evidence, not code): checks/, patches/, reviews/, results/, integ/, judge/, raw/
- Listing was truncated at 40 entries; a full name list is still needed.

## Open questions
1. Stage count: 8 names vs S9-S14 (6 slots). Which map to which?
2. Has the fix session's Phase B pilot report landed? Plan Phase 5 is blocked on it.

## Findings
- Bundle has no references/ dir in the skill root except references/30-fix.md; SKILL.md exists.
- selftest fixture is the mock_engine + make_fixture.py only (no seeded fixable bugs yet).
