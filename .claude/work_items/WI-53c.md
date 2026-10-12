# WI-53c Port fix half S9-S14 and canary bwrap settings fix (P5, bug 9)
area: deep-haiku-review-fix
size: L
ready: hold
after: WI-53b
items: C325,C326
files: none
out-of-repo: /home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/fix/fix_series.py;/home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/fix/run_fix_review.sh;/home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/fix/fix_merge.py;/home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/fix/fix_canary.sh;/home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/fix/fix_settings.py;/home/ethansm/.claude/skills/deep-haiku-review/bundle/dr_lib/selftest.py
blockers: after: WI-53b (fix_merge.py and dr_lib/selftest.py overlap). Hold until the user says the fix pilot report is ready and gives GO (phase2_impl.md:131 trigger). "P5" is not a literal row id in phase2_impl.md: line 131 is the section 7 heading, so close by doc:line. C326 anchor moved: the basename guards are at fix_canary.sh:186-187, not 174-175. Whether the leftover basename `--` (phase2_impl.md:22) still exists at 186-187 is unknown; check before editing. Live pilot needs a user trigger.
docs: .claude/plans/deep_haiku_review_phase2_impl.md rows P5 (line 131), C326 (line 22)

## Goal
Port stages S9-S14 through agents F-a to F-g (P5), and fix the bwrap settings-dir bug plus its stub selftest (C326). Build only after the hold clears.

## Steps
1. C325 (P5, phase2_impl.md:131): port S9-S14 through F-a to F-g (next_phases.md:45-47) into `fix_series.py`, `run_fix_review.sh` and `fix_merge.py`. Run the fixture selftest before any live pilot.
2. C326 (phase2_impl.md:22): in `fix_canary.sh`, check the basename guards at lines 186-187 and remove any leftover `--` if it is still there. In `fix_settings.py`, expose the settings dir read-only to bwrap. In `dr_lib/selftest.py`, add a stub test that the stub loads the settings file.

## Verify
1. `sed -n '186,187p' bundle/engine/fix/fix_canary.sh` expect the basename guards, with no leftover `--`.
2. `bash -n bundle/engine/fix/fix_canary.sh` (no writes).
3. `python3 bundle/dr.py selftest --engine mock` (stub only, $0; command from dr.py:13). Expect the new settings-load check to pass.
4. Repo gates (run_tests.py, build_check.py, compat_check.py): not applicable, no repo files touched.

## Docs
- After Verify passes, close P5 (C325) at phase2_impl.md:131 and C326 (item id, since its row id is `-`) with `python3 scripts/plan_manager.py mark-done <row id> --notes "<txt>"`.
- CHANGELOG: none named in TSV.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C325 .claude/plans/deep_haiku_review_phase2_impl.md:131 id=P5 blocked
- C326 .claude/plans/deep_haiku_review_phase2_impl.md:22 id=- blocked
