# WI-59c Deep-haiku-review replay and lens profiles: verify
area: deep-haiku-review-rl-replay, deep-haiku-review-lens
size: S
ready: verify-only
items: C338,C339,C340
files: .claude/deep-review/phase3b/replay/build_rl1.py; .claude/deep-review/phase3b/replay/score_replay.py; .claude/deep-review/phase3b/profile_rl1.toml; .claude/deep-review/pilot/profile_lens.toml
out-of-repo: /home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/dr_profile.py
blockers: C339 rl2 (profile_rl2.toml) is gated on L0 (phase3c:56, :66-68). The C339 row has no id and covers rl1 and rl2, so C339 stays open. C340 (L3) run steps 2-4 need the user's first-packet gate (phase3c:72); only the profile step is verified, so L3 stays open. Lens copy path resolved: `.claude/deep-review/pilot/profile_lens.toml` (phase3c:72) exists. profile_rl1.toml sets adjudicate_total 0.55 (line 47); phase3c:56 says 0.50: record in NOTES, do not fix. CHANGELOG: unknown. mark-done phase id: unknown.
docs: .claude/plans/deep_haiku_review_phase3c_impl.md (rows :55 W2-C0b, :56 C339, :72 L3)
deferred: C339-rl2
## Goal
Verify the replay and lens artifacts (C338, C339 rl1, C340 profile step) are present and correct. No edits. Close C338 only. Steps paths are relative to /home/ethansm/.claude/skills/deep-haiku-review/ for out-of-repo files and to the repo root otherwise.
## Steps
1. C338 (W2-C0b): `replay/build_rl1.py` and `replay/score_replay.py` make no model call and no p1 write. Read-only: `rg -n -i "subprocess|claude|anthropic|p1" <both files>`, then read each hit.
2. C339 rl1 (phase3c:56): `profile_rl1.toml` has `[protected] files = []`, `adjudicate = "claude-haiku-5-5"`, `adjudicate_styles = ["skeptic", "advocate", "evidence"]`, and an empty `[effort]` (line 61). Record the 0.55 vs 0.50 mismatch.
3. C340 (L3 profile step, phase3c:72): `pilot/profile_lens.toml` has `head = "22b56b4"` (line 5), four lenses (line 54), `files = []` (line 39), `reduce = "claude-haiku-5-5"` (line 50), and map_total 0.12 + reduce_call 0.03 (lines 43-44), within $0.15. The "write the copy" step is not needed: the file exists.
4. Close only rows verified here.
## Verify
1. `python3 -m py_compile` on the two replay `.py` files.
2. `python3 /home/ethansm/.claude/skills/deep-haiku-review/bundle/engine/dr_profile.py check .claude/deep-review/phase3b/profile_rl1.toml` and the same with `.claude/deep-review/pilot/profile_lens.toml` (phase3c:56 form). Each must report OK.
3. No repo build gate applies to `.claude/deep-review` files.
## Docs
After verify, close phase3c:55 W2-C0b (C338) with `python3 scripts/plan_manager.py mark-done <phase>`; phase format unknown, so report instead of guessing. Leave phase3c:56 (C339, row id -) and phase3c:72 (L3, C340) open.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C338 .claude/plans/deep_haiku_review_phase3c_impl.md:55 id=W2-C0b blocked
- C339 .claude/plans/deep_haiku_review_phase3c_impl.md:56 id=- blocked (stays open until rl2)
- C340 .claude/plans/deep_haiku_review_phase3c_impl.md:72 id=L3 blocked (stays open)
