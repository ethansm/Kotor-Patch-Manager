# WI-16c BuffDurationHUD slot age guard (C098 buff half)
area: load-hang-investigation
size: S
ready: hold
items: C098
files: patch_manager_mods/buff_duration_hud_src/BuffDurationHUD.cpp
out-of-repo: none
blockers: C098 letterbox half is in WI-16b; no slot-age code exists, so the guard needs design; CHANGELOG unknown.
docs: patch_manager_mods/09_load_hang_investigation.md (open at line 146)
deferred: C098

## Goal
Hold: GO-gated (quiet sources and user GO). Add a slot age guard to `BuffDurationHUD.cpp`. Split from WI-16. `rg -i '\bage\b|stale'` finds no slot-age code; the nearest match is the gate-reopen block at :411 ("close the stale state here").

## Steps
1. None now (deferred). When the GO lands: add the slot age guard in `BuffDurationHUD.cpp`. No existing anchor; design it in this file and use :411 as the nearest reference.

## Verify
1. `python3 scripts/build_check.py` after the GO-gated edit.
2. `python3 scripts/run_tests.py <filter>` (suite name unknown).
3. Hang-rate checks are user play tests, not agent work.

## Docs
- Nothing to close now. The row stays open at `patch_manager_mods/09_load_hang_investigation.md` line 146.
- Close via `python3 scripts/plan_manager.py mark-done` only after the gated step verifies. Arg form unknown.
- CHANGELOG bump: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C098 patch_manager_mods/09_load_hang_investigation.md:146 id=- parked
