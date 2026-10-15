# PL-T13
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-08 | PARTIAL | patch_manager_mods/perf_lab_src/hookbytes.py:21; patch_manager_mods/perf_lab_src/hookbytes.py:18-19 | no | Every JMP flagged (indirect too) via startswith('J'), not precedence; loop unguarded; thunk crash needs dump data |

- patch_manager_mods/perf_lab_src/hookbytes.py:21 anchor matches exactly. `and` binds tighter than `or`, so the test is (CALL/JMP and '0x' in t and '[' not in t) or t.startswith('J').
- patch_manager_mods/perf_lab_src/hookbytes.py:21 `startswith('J')` alone is True for every JMP string, so precedence is not the cause; the broad disjunct is. Direct and indirect JMP both flagged.
- patch_manager_mods/perf_lab_src/hookbytes.py:21 indirect "JMP EAX" and "JMP [mem]" are not relative branches: REL-BRANCH-IN-STOLEN is a false positive (over-flag only).
- patch_manager_mods/perf_lab_src/hookbytes.py:21 Jcc are always relative in 32-bit code, so flagging them is correct. LOOP/LOOPE/LOOPNE/LOOPZ/LOOPNZ (rel8) never match: a false negative the finding does not raise.
- patch_manager_mods/perf_lab_src/hookbytes.py:21 parenthesising the first clause is a no-op (already grouped that way). A correct fix must exclude JMP from the startswith('J') disjunct.
- patch_manager_mods/perf_lab_src/hookbytes.py:17 is initialisation only (got, txt, k=i[0]); the while loop is at :18-19, so the anchor line is off by one.
- patch_manager_mods/perf_lab_src/hookbytes.py:18-19 loop stops on byte count len(got) >= 2*max(5, len(exp)//2) only. No `k < len(ins)` guard; ins[k] at :19 raises uncaught IndexError when the function dump runs out.
- patch_manager_mods/perf_lab_src/hookbytes.py:15-17 `if not i` guards i[0], so overrun is forward-only: fewer target bytes remain in that function (e.g. a hook at its final RET).
- patch_manager_mods/perf_lab_src/hookbytes.py:7-12 insns() loads only the one function containing the address, so a short thunk dump overruns at :19; a row with no mnemonic would raise at :21. Real dump contents are outside range.
- patch_manager_mods/perf_lab_src/hookbytes.py:22 `rel` only appends a printed tag; no gate in this file. Any caller gate is outside range.
