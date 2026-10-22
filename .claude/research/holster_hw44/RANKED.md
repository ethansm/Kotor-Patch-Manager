# HW44 edge recon: ranked merge (8 Haiku reports, read-only, verified by anchor only)

Tier 1 (real flaws, fix candidates)
1. D1  wrist continuity pick (my HW44c) can hold a conj/conj candidate 131-150 deg from the target past ~233 deg raw turn; test checks step size only. HolsterReach.h:538-557, holster_twist_test.cpp:234.
2. L2/P1/M4/L1  topology cliff: Hip->OverBack at kOverDz -0.45 steps u 0.76->1.0 and flips extract share 0.10->0.20; HingeBack/OverBack at -0.25 has no hysteresis. HolsterReach.h:290-298, Holster.cpp:1701-1703.
3. X1/X2  editor preview: no dual stagger (world scales 60 ms by slowx), one shared slide+category for both hands.
4. R1/M6  repo holster.ini lacks the live via keys (reach_via_rifle, out_m_rifle, melee) -> fresh install loses the rifle fix.
5. F1/F2  qstaff has no default bone/rot/off: lands on pelvis_g with global placement (live ini has none either).

Tier 2 (cheap hardening)
D5 clear topoSet at 3 clear sites; D6 clear combOn/tgtOn when wrist block skipped; M1 via bump jumps ~0.11 m on mid-carry reversal; X3 100 ms reversal gate not slow-scaled; X6 keep previous slowx on loadTune failure; H5 clamp place_off; X4 preview arms reach in repoint mode; X8 slider text.

Tier 3 (opt/unverified): per-category wrist caps (L3/M7), reach_extract_pct per category (L5), via gated by topology for rifle/heavy on hip/thigh (R3/H1), M2 melee exitSign in via, sweep gaps R5/R6.
