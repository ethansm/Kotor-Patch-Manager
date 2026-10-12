OUTPUT: /home/ethansm/orca/workspaces/local-setup/KOTOR-II/.claude/review/work/play1012/P-D.md
EVIDENCE: BANNER T=3612773858/3631008000/3653202658 = 10-11 13:03, 10-11 18:07, 10-12 00:17 (one continuous clock; area_hook attach T=3653161794 = 00:16:23.08); RMP oid=7f000000 T=3653779464 (tv 1>1) and T=3653966180 (tv 102>102), both why=skip:vary; handoff 34 L63 (tag route) and L70 (rule-11 bug, fix in unbuilt 0.1.5)
FINDING: N1: 3 launches; 00:16:23=3rd. FAIL16 22/23 v0.1.2-vs-v0.1.4. N2: invalid oid 7f000000 tv 1/102, no write. N3: live 0.1.4; only WARN4 documented. N4: CLEAN, no POP-STABLE.
OPEN: Should checks 11 and 16 skip oid 0x7F000000 (invalid handle) and compare only same-version launches, then re-test on 0.1.5-probe?
VERDICT: PARTIAL
