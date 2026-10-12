# WI-26b Dialogue reply-panel stretch (C157)
area: dialogue-letterbox-and-wrap-fixes
size: S
ready: hold
items: C157
files: patch_manager_mods/dialogue_letterbox_fix_src/K2DialogueLetterboxFix.cpp
out-of-repo: none
blockers: User GO needed; doc 17 marks C157 out of scope. 5120+ visual check is user-only. Split from WI-26 (different patch). after: WI-30 (same file, K2DialogueLetterboxFix.cpp). Function anchor unknown; reply Y placement already exists at K2DialogueLetterboxFix.cpp lines 211-213 (replyY, replyMax).
docs: patch_manager_mods/17_dialogue_letterbox_and_wrap_fixes.md: C157 (line 152, row id -)
deferred: C157
## Goal
Add the K1-style reply-panel stretch (C157) only after the user decides. Write no code until then.
## Steps
1. Hold. Wait for user GO on C157. Question: "Proceed with C157? Doc 17 marks it out of scope." Report BLOCKED until then.
2. After GO: in patch_manager_mods/dialogue_letterbox_fix_src/K2DialogueLetterboxFix.cpp, extend the reply Y placement (lines 211-213) with the K1-style stretch. Function anchor: unknown.
3. Run Verify 1.
## Verify
1. `python3 scripts/build_check.py` → passes (after GO and step 2).
2. Test suite: suite name unknown; `python3 scripts/run_tests.py <filter>` (filter unknown).
3. Play test at 5120+ width: user only (needs_user=yes).
## Docs
- Mark C157 with `python3 scripts/plan_manager.py mark-done <phase>` only after user GO and Verify 1 passes. Phase id unknown (row id is '-' in TSV).
- CHANGELOG: none named in TSV.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C157 patch_manager_mods/17_dialogue_letterbox_and_wrap_fixes.md:152 id=- parked
