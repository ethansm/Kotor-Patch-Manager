# Plan-handoff shared-file review (2026-10-11)

Read-only recon. Scope: D1-D5 and the shared-file solution.

## (A) Verdict per decision
- D1 (SKILL.md:58 names plan-scoped file): KEEP. Matches SKILL.md:29; one-line template fix. Outside the user's stated list, but low risk (see D5 Q5).
- D2 (cmd_handoff writes only named file): KEEP. scripts/plan_manager.py:581-584 and :658-664 now write only .claude/handoffs/<stem>.handoff.md; no reader in code depends on the shared file (Q1).
- D3 (test rewrite): KEEP. scripts/test_plan_manager.py:98-122 pins no-shared-file and no-.prev; :186 reads the named file. Necessary consequence of D2.
- D4 (CLAUDE.md:37, AGENTS.md:63): KEEP, but the shared file is still tracked and on disk (git ls-files confirms), so "no longer written" text is accurate only until a stale copy misleads; see (E3).
- D5 (leave claude_persist_state.sh and plan_manager.py:160): KEEP as deferred. Script is unreferenced by settings/hooks (Q1/Q3). Line 160 exclusion is harmless dead filter.

## (B) Missed items
- .claude/active_plan.txt is the same shared-single-file class: written on every handoff, mark-done, explicit --plan (scripts/plan_manager.py:729, 761, 776, 823) and read by resume (:149-150). Concurrent sessions on different plans race on it. Not addressed by D1-D5.
- LIVE POLLUTION: .claude/active_plan.txt currently contains /tmp/tmpwpzgeouz_plan.md (a test temp plan). Cause: tests call run_cli("--plan", tmp, "mark-done"...) without --handoff-dir (scripts/test_plan_manager.py:138, 143), so plan_manager.py:758-761 and :775-776 write the real active_plan.txt. discover_active_plan then falls to mtime-newest "*plan*.md" glob (:149-165), which is wrong under concurrency.
- Regeneration wipes hand-appended notes: SKILL.md step 2.3 tells the agent to append dead ends to the named file, but cmd_handoff uses write_text (plan_manager.py:662), so any later handoff or mark-done auto-handoff (:786) overwrites them. The removed rotation had at least kept a .prev copy; the named file has none.
- Named handoff files are tracked in git (23 untracked tmp*_plan.handoff.md files also sit in .claude/handoffs/ from the test hazard).
- Stale docs still describe the old behavior: DETERMINISTIC_WORKFLOWS_PLAN.md:107, 162 (unbuilt helper rotating the shared file).

## (C) Risks
- Same-plan collision (Q2): two sessions on one plan_stem overwrite each other; resume reads Section 0 from that file (plan_manager.py:285-292), so the wrong direction can be shown.
- Every test run adds a tmp*_plan.handoff.md and re-points active_plan.txt at a deleted temp file; a user running `resume` with no arg may get the wrong plan.
- Agents told "do not read the shared file" lose the only place hand-written scratchpad content (claude_persist_state.sh:58-80 sections 3-5) lived, if anyone still runs that script manually.
- Stale shared file (mtime Oct 11 01:23, git-tracked) may be committed with `git add -A` and read by tools outside this repo's instructions.

## (D) Answers to questions
1. Readers of .claude/session_handoff.md in code/config: NONE. rg (excluding handoffs/ and *.jsonl) finds only: scripts/plan_manager.py:160 (exclusion filter, not a reader), scripts/claude_persist_state.sh:4,7,58,91 (a writer), scripts/test_plan_manager.py:107-108 (asserts absence). No hits in ~/.claude/settings.json, no repo .claude/settings*.json, hooks, or skills. Doc-only references (historical, not readers): patch_manager_mods/upstream_prs/NEXT_PHASE_PLAN_2026-10-04.md:12, patch_manager_mods/08_level_up_override.md:1052,1074, DETERMINISTIC_WORKFLOWS_PLAN.md:58,103-107,157-162, tests/benchmarks/transcript_prompt_eval.json:263. Memory/other worktrees not checked beyond ~/.claude settings/skills/hooks/commands.
2. Same-plan collision is a real risk: file keyed only by plan_stem (plan_manager.py:660-661), overwrite via write_text, no session or phase key. Current design does not handle it. Needed (cheap): either key by stem+phase (and make resume at :285 look up the same key) or have cmd_handoff keep a .prev copy and print a warning when the existing file's Section 0 differs. Not required for this change.
3. D5: Separate decision, do it next; recommend making claude_persist_state.sh write a named file or exit with a deprecation message, since nothing calls it automatically (no settings/hook reference) so the change cannot break a caller.
4. Spot-checks:
   (a) CONFIRMED by code: every handoff run writes the named file (plan_manager.py:658-664, unconditional). The corpus claim that scripts "report session_handoff.md" was true only before D2.
   (b) CONFIRMED: mark-done calls cmd_handoff (plan_manager.py:786), which writes the same named file; Saved State banner prints that path (:789-797). The transcript claim "mark-done never ran" cannot be checked narrowly: rg -l 'plan_manager.py mark-done' hits 562 transcript files because CLAUDE.md text is injected into them, so that count is meaningless.
   (c) PARTIAL: for the 5 session ids named in the report (0c9c5cc7, 119b7354, ddfc779e, ed54f7d8, fd6f4d01) each transcript contains at least one `cp ... session_handoff` command (rg -c 1-2 per file) and many .prev mentions (4-16). rg -l 'cp ... session_handoff.prev' across the project dir matches 8 files, so 5 is plausible but not proven; counts do not show whether each was a restore.
5. Overreach: D1 (SKILL.md:58) is the only item not in the approved list; it is a one-line, directly related fix. D2 also changed the usage docstring (plan_manager.py:13) and removed the two "Handoff written/Rotated" stdout lines, which is within scope. CLAUDE.md:37 and AGENTS.md:63 additionally add the sentence "Several sessions share this worktree, so do not read ... it is no longer written", slightly beyond a bare path swap. Note CLAUDE.md, AGENTS.md and SKILL.md were already modified before this work (git status at start), but the current diffs for those files are only the one-line changes shown. No edits to claude_persist_state.sh or plan_manager.py:160 (correct).
6. Test hygiene: track separately but soon, as its own small change. It is wider than the handoff files: it also overwrites the real .claude/active_plan.txt (live now, see B). Fix is to pass --handoff-dir to every mark-done/--plan test call (scripts/test_plan_manager.py:138,143) and to make plan_manager.py:759 and :775 skip writing the real active_plan.txt when a handoff dir is given.

## (E) Recommended next actions
1. Reset .claude/active_plan.txt to the real plan and delete the 23 untracked .claude/handoffs/tmp*_plan.handoff.md files (needs the user's OK; destructive).
2. Fix test isolation (pass --handoff-dir; guard active_plan.txt writes at plan_manager.py:759,775) in one separate commit.
3. Decide D5 together with a stub or git rm of .claude/session_handoff.md and the .prev copy, and add a .prev or warn-on-overwrite guard for the named file.
