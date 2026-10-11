# R7 fix-source audit (read-only, 2026-10-10)

Counts: `rg -c -i -e "kotor|patch_manager|master_ledger|hotswap|ghidra|\.claude/review"`. Lines via `wc -l`.
"Imports" = fix-pipeline files referenced (py import or sh call); comment-only mentions are noted as such.

| File | Lines | Repo strings | Purpose | Imports / calls |
|---|---|---|---|---|
| fix/build_integration_batches.py | 620 | 7 | group accepted patches into integration batches | fcommon |
| fix/canary.json | 275 | 21 | FIX canary result data | none |
| fix/fcommon.py | 263 | 19 | shared paths and helpers | none |
| fix/fix_calibrate.py | 698 | 7 | reviewer seeded-bad calibration | fcommon, fix_prompts, record_fix |
| fix/fix_canary_probe.md | 6 | 0 | sandbox permission probe note | none |
| fix/fix_checks.py | 951 | 6 | deterministic patch and batch gates | fcommon, fix_workspace |
| fix/fixer_prompt.md | 38 | 1 | fixer prompt | none |
| fix/fixgate.sh | 126 | 4 | only command integrator may run | none (comments name fix_checks, fix_settings) |
| fix/fix_intake.py | 381 | 7 | verified findings to fix queue | fcommon |
| fix/fix_ladder.py | 1018 | 1 | Haiku-first variant ladder | fcommon, record_fix, fix_prompts; lazy fix_workspace, fix_settings, fix_calibrate |
| fix/fix_merge.py | 660 | 7 | merge records into fix report | fcommon, build_integration_batches |
| fix/fix_prompts.py | 503 | 2 | prompt renderers | fcommon |
| fix/fix_schema.json | 124 | 0 | fixer output schema | none |
| fix/fix_settings.py | 262 | 1 | per-run settings and bwrap argv | fcommon; lazy fix_workspace |
| fix/fix_workspace.py | 990 | 23 | isolated workspace build | fcommon |
| fix/integrate_schema.json | 101 | 0 | integrator output schema | none |
| fix/integrator_prompt.md | 30 | 0 | integrator prompt | none |
| fix/judge_prompt.md | 29 | 1 | ladder judge prompt | none |
| fix/judge_schema.json | 71 | 0 | judge output schema | none |
| fix/record_fix.py | 677 | 1 | fix recorder and task-state engine | fcommon, fix_prompts, record_run (tools/) |
| fix/reviewer_prompt.md | 28 | 1 | blind reviewer prompt | none |
| fix/review_schema.json | 138 | 0 | reviewer output schema | none |
| fix/run_fix.sh | 310 | 4 | fixer runner | fcommon, fix_checks, fix_intake, fix_prompts, fix_settings, fix_workspace, record_fix |
| fix/run_fix_review.sh | 230 | 3 | reviewer runner | fcommon, fix_calibrate, fix_intake, fix_prompts, fix_settings, fix_workspace, record_fix, run_fix |
| fix/run_fix_judge.sh | 254 | 5 | ladder judge runner | fix_ladder, fix_prompts, fix_settings, fix_workspace, record_fix, run_fix_review |
| fix/run_integrate.sh | 249 | 7 | integrator runner | fcommon, fix_checks, fix_settings, fix_workspace, record_fix |
| tools/fix_canary.sh | 328 | 4 | canary runner (outside fix/) | fix_settings, run_integrate |

Data subdirs, not code (file counts): checks 178, patches 346, raw 1518, results 217, reviews 59, specialty_notes 15, integ 8, judge 14.

R4 check: its 26-name list matches all top-level fix/ files; its "truncated at 40 entries" note is stale. It omits specialty_notes/ from the data-dir list.
R1 check: says fix/ has 27 files; actual is 26 (27 only if tools/fix_canary.sh is counted). results/ is 217 now, not 209.
