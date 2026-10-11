# L2 Haiku ladder replay, config A (2026-10-10)
Scratch run `rl1` (8 batches, 87 items, 4 control ids). Styles skeptic / advocate / evidence (L2), 2 comparators (L3), Sonnet L4 (4 items/batch, no sub-agents). Config A = no `--effort` flag. Profile `phase3b/profile_rl1.toml`, scorer `phase3b/replay/score_replay.py`.

## Cost: $0.949 of the $1.00 L2 ceiling (L2 $0.513, L3 $0.136, L4 $0.300)
- L4 stopped at the profile cap ($0.25, +1 call overshoot): 2 of 7 escalation batches ran, **17 of 87 items stayed unresolved** (counted as unresolved, not scored).
- Per batch actual $0.119 avg (01=$0.249 ... 10=$0.060) for 70/87 resolved items. Extrapolated to resolve all 25 L4 items: about **$1.55 (~$0.19/batch)** vs Sonnet single pass $0.239/batch ($1.912) and single Haiku $0.022/batch ($0.178).
- Config B (low effort per style) NOT run: no budget left after A (L2 total ceiling $1.00 shared).

## Quality
| | ladder (70 resolved) | single Haiku (old) | Sonnet single pass |
|---|---|---|---|
| agrees with Sonnet | 54/70 (77%) | 65/87 (75%) | - |
| control ids (truth.json) | **3/4** | **3/4** | **1/4** |
- Controls: decoy refuted OK, K2 confirmed OK (resolved at L3), rejected refuted OK, K3a refuted = MISS (all three styles + old Haiku agree; expected confirmed or needs-human).
- L2 style self-agreement (advocate vs evidence) 77% (single-Haiku run-to-run was 73%). Items needing L3: 33/87 (38%); reaching L4: 25/87 (29%). Resolved by: l2=54, l3=8, l4=2, human=6.
- Quote checks: 0 fabricated across all L2 batches.
- "Agrees with Sonnet" is a weak yardstick: stored Sonnet is only 1/4 on the controls.

## Decision (plan rule: ladder must beat single Haiku on ground truth AND cost < $0.12/batch): **NO**
Ground truth ties (3/4 vs 3/4, n=4: tiny sample) and cost is 5x single Haiku (~8x if fully escalated). Leave `adjudicate_styles = []` (ladder off) in defaults; no `EFFORT_COST_MULT` change (no L2B data). The refute-ladder build (Track R) is NOT justified: SKIPPED.
Caveats: n=4 controls, one replay, config A only; Sonnet 4-item batches cost ~$0.15 (about $0.04/item vs $0.022/item at 11 items), so the escalation shape matters more than the style count.
Option if cost matters: 2 styles + comparator, no L4 (leave residue to needs-human): about $0.40 for 8 batches (est.), untested.
