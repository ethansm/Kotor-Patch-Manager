# WI-16b Letterbox IsBadReadPtr to probeRead (C098 letterbox half)
area: load-hang-investigation
size: S
ready: hold
items: C098
files: patch_manager_mods/dialogue_letterbox_fix_src/K2DialogueLetterboxFix.cpp
out-of-repo: none
blockers: C098 buff half is in WI-16c; CHANGELOG unknown.
docs: patch_manager_mods/09_load_hang_investigation.md (open at line 146)
deferred: C098

## Goal
Hold: GO-gated (quiet sources and user GO). Replace the guarded reads in the letterbox probe with `probeRead`. Split from WI-16. Only this file calls `IsBadReadPtr` in code; `CharEffectsPanel.cpp` names it in a comment only (it already includes `SafeRead.h` at :43).

## Steps
1. None now (deferred). When the GO lands:
   a. Add `#include "SafeRead.h"`. The file does not include it yet.
   b. In `struct Rd` (starts :302), replace `IsBadReadPtr` at :306 (4 bytes), :313 (2 bytes) and :319 (1 byte) with `saferead::probeRead(site, p, len)`. `probeRead(unsigned site, const void* p, size_t len)` is at `patch_manager_mods/_shared_src/SafeRead.h:97`, inside `namespace saferead`. Pick a NEW site id for each call.
   c. Update the comment at :274, which still names IsBadReadPtr.

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
