# L3 lens test (run p1-lens, head 22b56b4, lenses logic/state/contract/security)
Written by the orchestrator from `p1-lens/master_findings.json` and `p1/verified_findings.json`. Unverified: lens findings never went through verify/adjudicate.

## Cost (live, pre-approved ceiling $0.15)
- canary $0.0072 + map-first $0.0055 + map $0.0878 + reduce $0.0028 = **$0.1034**. p1 single-lens map was $0.0296, so the 4-lens map cost about 3.0x.
- Worst case was map_total $0.12 + one per-call cap; actual stayed under the ceiling. `--yes-spend 0.31` was passed for map only because dr.py compares its high estimate ($0.304) to the ceiling; the enforced cap was `map_total` 0.12.

## Results (merged master, 18 findings vs p1's 17)
- Raw findings per lens: contract 5, state 5, logic 4, **security 0**. Severity medium 6 / low 12 (p1: 5 / 12).
- Corroborated (>=2 lenses): **3 of 18** (17%): 6d18e325826a (contract+logic, EquipCharPreview.cpp 709-714), 380b0153b1a8 (contract+logic+state, LevelUpOverride.cpp 1391-1402), d9d1f418193b (contract+state, gate_test.cpp 320-322).
- p1 CONFIRMED (6) recovered by line overlap: 4 of 6 (592-601, 709-713, 163-165, 196-199). Missed: 1412-1414 (near but not overlapping the 1391-1402 lens finding) and 1434-1435. Overlap judged by line ranges only (p1 tiers carry no file field), so treat as approximate.
- New unique, unverified: about 5 candidates (EquipCharPreview.cpp 144-165 state; LevelUpOverride.cpp 1391-1402; unit_gap_test.cpp 111-136; gate_test.cpp 320-322 plus a run of low items). 9 of 18 merged findings carry no `lenses` field (not attributed to a lens by merge).
- The two lens-only medium findings in EquipCharPreview.cpp (595-601) duplicate the p1 CONFIRMED 592-601 finding, so lenses mostly re-found known issues.

## Decision
Lenses stay OFF by default (opt-in, documented). 3x map cost bought 4/6 recall (p1 single lens was the reference), a 17% corroboration rate, and no security-lens output on this code-only diff. Corroboration is a usable ranking signal but needs verify to prove value; running verify on the new candidates would cost about $0.02-0.05 and needs a fresh go.
