# WI-20b Robustness TD and GUIID fixes, verify only (C115, C116)
area: robustness-fixes
size: S
ready: verify-only
items: C115,C116
files: patch_manager_mods/robustness_fixes_src/RobustnessFixes.cpp
out-of-repo: none
blockers: none for verify. Target file was unknown in the checker issue; now verified present. Source header is k2-robustness-fixes 1.3.0 (RobustnessFixes.cpp:1); doc 10 line 437 records 1.2.1 installed. Row anchors: doc 10 line 390 (row 10, "TD / GUIID"), not line 331. Split: WI-20.md (TX pair) and WI-20c.md (C112).
docs: patch_manager_mods/10_engine_optimization_research.md:C115@L390,C116@L390
deferred: none

## Goal
Confirm the TD (2DA tokenizer) and GUIID (GUI id padding) fixes are already in source. No code change.

## Steps
1. (C115 TD, C116 GUIID, present) Run `rg -n "const Site (Guiid|TdA|TdB|TdC) " patch_manager_mods/robustness_fixes_src/RobustnessFixes.cpp`. Expected: Guiid at :108 (0x0040F972), TdA :114 (0x0071C243), TdB :116 (0x0071C2C9), TdC :118 (0x0071C2F0). The TD and GUIID function anchors are 0x71C180 and 0x40F620 (header comments :27 and :43). Counters gui_reject and td_long are logged at :532 and :534.

## Verify
1. The step 1 rg output is the evidence for C115 and C116.
2. `python3 scripts/build_check.py`
3. `python3 scripts/run_tests.py <robustness filter>`: suite name unknown.

## Docs
- After verify, close C115 and C116 with `python3 scripts/plan_manager.py mark-done <phase>`. Phase shorthand: unknown. Never sed the master plan.
- CHANGELOG bump: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C115 patch_manager_mods/10_engine_optimization_research.md:390 id=TD open
- C116 patch_manager_mods/10_engine_optimization_research.md:390 id=GUIID open
