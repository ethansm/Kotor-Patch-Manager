# WI-40b Cmdq LAT and hold-reason parser
area: paused-command-queue
size: S
ready: ready
items: C228
files: scripts/kotor2_logs.py
out-of-repo: none
blockers: Split from WI-40. `kotor2_logs.py:51` routes cmdq-probe_log.txt to `cmdq_probe_log_check.py`, so a section added to kotor2_logs.py is not reached by `check` unless it is wired in; placement is unknown (doc 29:85 names kotor2_logs.py). Whether the live log exists in the game dir is unknown. The row id is `-`, so the closure key is unknown. git status on kotor2_logs.py was clean at write time.
docs: patch_manager_mods/29_paused_command_queue.md:85 (C228)

## Goal
Add the cmdq LAT and hold-reason parser (C228), the only buildable row from WI-40.

## Steps
1. Read `scripts/cmdq_probe_log_check.py`. If it already prints LAT lines and a hold-reason histogram, close C228 as covered and skip step 2. At write time it has neither (rg -i 'LAT|histogram': 0 hits), so step 2 applies.
2. In `scripts/kotor2_logs.py`, add a cmdq section that prints LAT lines and a hold-reason histogram. Hold reasons: PAUSED, BUSY(AA8), ROUND(AB8), NOPEND, EXPANDED, DROPPED (doc 29:79). Touch only that new section.

## Verify
1. `python3 scripts/run_tests.py <cmdq filter>` (suite name unknown)
2. `python3 scripts/kotor2_logs.py check cmdq-probe_log.txt` (log name from kotor2_logs.py:51; never cat, tail, or grep raw .txt logs)
No `build_check` needed: no `.cpp` is touched.

## Docs
- After Verify 1 and 2 pass, close C228 (doc 29:85, id `-`). Closure key unknown, see blockers.
- CHANGELOG: none stated in TSV.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C228 patch_manager_mods/29_paused_command_queue.md:85 id=- open
