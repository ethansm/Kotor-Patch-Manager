# WI-59b Deep-haiku-review refute stub and checks (R2)
area: deep-haiku-review-refute
size: S
ready: ready
items: C350
files: none
out-of-repo: /home/ethansm/.claude/skills/deep-haiku-review/bundle/dr_lib/selftest.py; /home/ethansm/.claude/skills/deep-haiku-review/bundle/stub/claude
blockers: after: WI-59 (same file, selftest.py; C348 must verify first, phase3c:97). The phase3c:103 spec is "stub + 3 checks" only; the three check names are unknown, so list the chosen names in NOTES before editing. The refute-audit stage already exists in stub/claude (:5, :41) and selftest.py (:23, :26, :347-349); leave those lines alone. Live refute test needs its own go (phase3c:103). CHANGELOG: unknown. mark-done phase id: unknown.
docs: .claude/plans/deep_haiku_review_phase3c_impl.md:103 (R2 = C350)
deferred: C330, C349
## Goal
Add the R2 refute stub and its three selftest checks, stub-only, no live call. Close C350 only. Paths are relative to /home/ethansm/.claude/skills/deep-haiku-review/. C349 and C330 (refute ladder, R1) stay parked.
## Steps
1. Confirm WI-59 verified C348. If not, skip C350 and list it in NOTES.
2. C350 (R2): add the refute stub mode to `bundle/stub/claude`. Keep the existing refute-audit stage.
3. Add three checks to `bundle/dr_lib/selftest.py` next to the existing refute-audit checks (:347-349). Do not change those checks. Name the three checks in NOTES.
4. Do not start the live refute test.
## Verify
1. `python3 -m py_compile bundle/dr_lib/selftest.py`.
2. `dr.py selftest --engine mock` passes, and its count is not below the phase3d:14 mock count (93).
3. Test suite: the mock selftest above. No repo build gate applies.
## Docs
After verify, close phase3c:103 R2 (C350) with `python3 scripts/plan_manager.py mark-done <phase>`; phase format unknown, so report instead of guessing. Leave phase3c:103 R1 (C349) and phase3b:98 R1 (C330) open.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C350 .claude/plans/deep_haiku_review_phase3c_impl.md:103 id=R2 blocked
- C349 .claude/plans/deep_haiku_review_phase3c_impl.md:103 id=R1 blocked (deferred, not closed here)
- C330 .claude/plans/deep_haiku_review_phase3b_impl.md:98 id=R1 parked (deferred, not closed here)
