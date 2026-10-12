# WI-01b Bug-board triage (C010)
area: bug-board
size: S
ready: hold
items: C010
files: none
out-of-repo: none
blockers: C010 needs a user decision. The doc 24:100 split (human-only vs design-gated) is stale: doc 37 reads FIXED for BG01-BG16 and BG18-BG25 and WONTFIX for BG17 (checked 10-12). Re-triage before the user decides.
docs: patch_manager_mods/24_master_plan_status.md:100 (Track BG); patch_manager_mods/37_bug_board.md (rows BG01-BG25)
deferred: C010

## Goal
Hold. Give the user the current status of the 25 BG rows so they can decide the open-track question. No code edits. As of 10-12, only BG17 (WONTFIX) is not FIXED.

## Steps
1. None runnable now. C010 (after user decision, deferred): run Verify 1 and report the rows that are not FIXED. No code edits.

## Verify
1. `awk -F'|' '/^\| BG[0-9]+ /{print $2, $6}' patch_manager_mods/37_bug_board.md` (lists id and status for 25 rows)

## Docs
1. Close rows only with `python3 scripts/plan_manager.py mark-done <phase> --state <STATE> --commit <sha>`. Phase and STATE are unknown.
2. Never sed the master plan.
3. CHANGELOG: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C010 patch_manager_mods/24_master_plan_status.md:100 id=BG01-BG25 open
