# WI-51b deep-haiku-review ladder wiring: refute-audit and fix (C296)
area: deep-haiku-review-ladder
size: M
ready: ready
items: C296
files: none
out-of-repo: ~/.claude/skills/deep-haiku-review/bundle/dr.py; ~/.claude/skills/deep-haiku-review/bundle/engine/verify/refute_audit.py; ~/.claude/skills/deep-haiku-review/bundle/engine/verify/run_refute_audit.sh; ~/.claude/skills/deep-haiku-review/bundle/engine/fix/run_fix_review.sh; ~/.claude/skills/deep-haiku-review/bundle/engine/fix/run_integrate.sh
blockers: after: WI-51 (split from WI-51; ladder verify must be green first; no shared file edit). Running the Sonnet residue (L4) spends money: needs user spend GO (C295). The refute-audit refusal point in `dr.py`: unknown (only the stage entry at dr.py:72, `gate_after: findings-skim`, was located).
docs: .claude/plans/deep_haiku_review_next_phases.md:78
deferred: none
## Goal
Wire the L1-L4 ladder into refute-audit, then extend the same wiring to fix review and integrate. Sonnet only for residue. No repo files change.
## Steps
1. Locate the refute-audit refusal in `dr.py` (unknown; stage entry at dr.py:72). Record the location before editing.
2. Wire the L1-L4 ladder into `engine/verify/refute_audit.py` and `engine/verify/run_refute_audit.sh` (both exist). Sonnet only for residue (`ladder.py escalate-build`).
3. Extend the same wiring to `engine/fix/run_fix_review.sh` and `engine/fix/run_integrate.sh` (both exist).
4. Keep it to one patch set; create no new files.
## Verify
1. `rg -n -e ladder -e escalate` over the four engine files above and `dr.py`.
2. `python3 ~/.claude/skills/deep-haiku-review/bundle/dr.py selftest --engine mock` (offline, no spend).
3. `python3 scripts/run_tests.py <filter>`: not applicable, no repo files change.
## Docs
1. C296 has id "-": no mark-done; report only.
2. CHANGELOG: none named in TSV.
## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C296 .claude/plans/deep_haiku_review_next_phases.md:78 id=- open
