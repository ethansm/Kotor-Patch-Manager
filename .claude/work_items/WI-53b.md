# WI-53b Build deep-haiku-review fix merge gate and gate wiring (F-f, F-g)
area: deep-haiku-review-fix
size: M
ready: hold
items: C309,C310
files: none
out-of-repo: /home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/fix/fix_merge.py;/home/ethansm/.claude/skills/deep-haiku-review/bundle/dr.py;/home/ethansm/.claude/skills/deep-haiku-review/references/30-fix.md;/home/ethansm/.claude/skills/deep-haiku-review/bundle/fixture/make_fixture.py;/home/ethansm/.claude/skills/deep-haiku-review/bundle/dr_lib/selftest.py
blockers: Hold until the user says the fix pilot report is ready and gives GO (next_phases.md:45 trigger). Apply-script path unknown: dr.py:95 names only `fix_series.py build` plus `git am --3way`, no apply script. dr.py is shared with WI-55 (C321): serialize. Unknown whether fix_merge.py main() returns 1 on gaps (gap check at fix_merge.py:647, `sys.exit(main(...))` at :674).
docs: .claude/plans/deep_haiku_review_next_phases.md rows F-f,F-g (line 47)

## Goal
Make the fix-merge gate exit 1 on gaps (F-f) and confirm the three human gates with stubs and a seeded fixture (F-g). Build only after the hold clears.

## Steps
1. C309 (F-f): in `bundle/engine/fix/fix_merge.py`, confirm main() returns 1 when gaps exist. If not, make gaps exit 1. The per-batch human apply script is NEW: path unknown. Ask the user for the path before creating it.
2. C310 (F-g): `bundle/dr.py` already defines the gates fix-go (:91), fix-pilot (:93) and fix-apply (:95), with the pipeline text at :20. Verify each gate has a stub and a seeded fixture in `bundle/dr_lib/selftest.py` and `bundle/fixture/make_fixture.py`. Add only what is missing.
3. Update `references/30-fix.md` (119 lines now; keep at most 120 lines, no placeholder text).

## Verify
1. `rg -n "fix-go|fix-pilot|fix-apply" bundle/dr.py` expect at least 3 gate definitions (lines 91, 93, 95 as of this check).
2. `rg -n "sys.exit|gap" bundle/engine/fix/fix_merge.py` expect the gap path and exit code.
3. `python3 bundle/dr.py selftest --engine mock` (stub only, $0; command from dr.py:13).
4. `wc -l references/30-fix.md` (at most 120) and `grep -ci placeholder references/30-fix.md` (expect 0).
5. Repo gates (run_tests.py, build_check.py, compat_check.py): not applicable, no repo files touched.

## Docs
- After Verify passes, close F-f (C309) and F-g (C310) with `python3 scripts/plan_manager.py mark-done <row id> --notes "<txt>"`.
- Selector for the two next_phases.md rows: unknown (see WI-53 blockers).
- CHANGELOG: none named in TSV.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C309 .claude/plans/deep_haiku_review_next_phases.md:47 id=F-f blocked
- C310 .claude/plans/deep_haiku_review_next_phases.md:47 id=F-g blocked
