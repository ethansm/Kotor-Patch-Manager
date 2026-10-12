# WI-60b Phase3d effort pins in p2 profile
area: deep-haiku-review-phase3d-impl
size: S
ready: ready
after: WI-58, WI-60
items: C354
files: .claude/deep-review/phase3b/profile_p2.toml
out-of-repo: /home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/dr_profile.py
blockers: after: WI-60 (C351 verify first). after: WI-58 (same file, `[effort]`; WI-58 is on hold for the L0/L2 decision, so reconcile the two pin sets before editing). Conflict: profile_p2.toml:59-60 says "left empty on purpose; user decides pinning", but phase3d:16 (D4) pins them; this WI follows D4 and records the conflict. dr.py effort-stage map (phase3d:49 scope): no change specified; unknown whether needed. Fix-stage pin values (fix, fix_review, fix_integrate): not specified; left empty. C355 deferred: run dir ~/.cache/deep_haiku_review/kotor2-p2/p2c already exists, and `.claude/deep-review/phase3b/recon/P2_prep.md` would be overwritten. CHANGELOG: unknown. mark-done phase id: unknown.
docs: .claude/plans/deep_haiku_review_phase3d_impl.md (rows :49 A4, :53 A5)
deferred: C355
verify: python3 ~/.claude/skills/deep-haiku-review/bundle/engine/dr_profile.py check .claude/deep-review/phase3b/profile_p2.toml
verify: rg -n -A8 "^\[effort\]" .claude/deep-review/phase3b/profile_p2.toml

## Goal
Pin `[effort]` for profile_p2 per phase3d D4 (phase3d:16, :49). The fix keys already exist in code. Close A4 after the edit and verify. C355 (A5 p2c rebuild) is deferred.
## Steps
1. Confirm WI-60 verified C351. If not, stop and list it in NOTES.
2. C354 (A4): `bundle/engine/dr_profile.py:33` EFFORT_STAGES already has `fix`, `fix_review`, `fix_integrate`. Keep them. Verify only.
3. Edit `.claude/deep-review/phase3b/profile_p2.toml` `[effort]` (line 59). Set `map`, `verify`, `adjudicate`, `compare`, `lens` to `"xhigh"`, and `reduce`, `escalate`, `refute` to `"high"` (phase3d:16, :49). Leave the fix keys empty. Update the comment on line 60 to match.
4. Run `python3 /home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/dr_profile.py check .claude/deep-review/phase3b/profile_p2.toml` (phase3c:56 form).
## Verify
1. `dr_profile.py check` on profile_p2.toml reports OK.
2. Effort-empty argv is unchanged for profiles without `[effort]` (existing selftest check; phase3d:49).
3. `rg -n -A8 "^\[effort\]" .claude/deep-review/phase3b/profile_p2.toml` shows the pins.
4. Test suite: none named. No repo build gate applies.
## Docs
After edit and verify, close A4 (phase3d:49) with `python3 scripts/plan_manager.py mark-done <phase>`; phase format unknown. A5 (:53) stays open.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C354 .claude/plans/deep_haiku_review_phase3d_impl.md:49 id=A4 blocked
- C355 .claude/plans/deep_haiku_review_phase3d_impl.md:53 id=A5 blocked (deferred, not closed here)
