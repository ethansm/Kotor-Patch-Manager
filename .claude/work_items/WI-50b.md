# WI-50b deep-haiku-review H3/H4 verify, H6 and H9 decisions
area: deep-haiku-review-* (build_dossiers, prep resize, lens mode, dr.py estimate)
size: S
ready: verify-only
items: C288, C289
files: none
out-of-repo: ~/.claude/skills/deep-haiku-review/bundle/engine/verify/build_dossiers.py (verify only); ~/.claude/skills/deep-haiku-review/bundle/engine/prep.py (verify only); ~/.claude/skills/deep-haiku-review/bundle/templates/lens_*.md (verify only); ~/.claude/skills/deep-haiku-review/bundle/dr.py (C290, no edit)
blockers: after: WI-50. Deferred C290 needs a user decision: keep the Sonnet UNIT_COST figures (dr.py:100, reduce 0.50, adjudicate 0.21; no measured source in PILOT_REPORT.md) or re-measure. The Haiku adjudicate figure disagrees: dr.py:106 has 0.022 (A/B, 8 batches) while PILOT_REPORT.md:13 implies about 0.013 (2 batches). Deferred C291 needs a user go for the live multi-lens run (spend, same as C322). The C288 `--only` flag is dropped: it is not a build_dossiers.py flag (docstring lines 19-21). The real-engine selftest (80/80) needs a user go (spend); only the offline check runs here. H3/H4 ids also exist in phase2_impl.md, so mark-done needs a doc qualifier: unknown.
docs: .claude/plans/deep_haiku_review_next_phases.md:29, :30, :32, :69
deferred: C290 (dr.py H6 estimate constants), C291 (multi-lens live run)
## Goal
Verify C288 and C289, which are already in the bundle, and record the two deferred rows for the user. No edits in this WI.
## Steps
1. C288 (H3): verify `backup_stale` (engine/verify/build_dossiers.py:151) runs only under `if stale:` after the `--force` gate (lines 186-191). Offline check: `python3 ~/.claude/skills/deep-haiku-review/bundle/engine/verify/build_dossiers.py --self-test`. Do not add `--only`.
2. C289 (H4): verify `cmd_resize` (engine/prep.py:1185) falls back to default parameters with a WARNING on degenerate points (prep.py:1199-1201). The self-test case is at prep.py:1591-1602. Offline check: `python3 ~/.claude/skills/deep-haiku-review/bundle/engine/prep.py --self-test` (prints "prep.py self-test OK", prep.py:1612).
3. C291 (H9, deferred): verify lens mode. Four templates exist: templates/lens_{logic,contract,security,state}.md. `_check_lenses` (engine/dr_profile.py:203) checks names and templates only; it does not enforce a 2-4 count. Stub lens cases: dr_lib/selftest.py:724-839 ($0). Do not run the live lens run.
## Verify
1. `rg -n -m 3 'backup_stale\(' engine/verify/build_dossiers.py`: call at line 191.
2. `build_dossiers.py --self-test` exits 0.
3. `prep.py --self-test` exits 0 and prints "prep.py self-test OK".
4. `rg -n -m 3 'WARNING: resize points degenerate' engine/prep.py`: match at line 1200.
5. Repo gates: not applicable, no repo files change.
## Docs
1. After Steps 1-2 pass: mark C288 (H3) and C289 (H4) with plan_manager.py mark-done and notes, using the doc qualifier from blockers.
2. Deferred C290 and C291: put the two questions in blockers to the user. Do not close either row.
## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C288 .claude/plans/deep_haiku_review_next_phases.md:29 id=H3 open
- C289 .claude/plans/deep_haiku_review_next_phases.md:30 id=H4 open
- C290 .claude/plans/deep_haiku_review_next_phases.md:32 id=H6 open
- C291 .claude/plans/deep_haiku_review_next_phases.md:69 id=H9 open
