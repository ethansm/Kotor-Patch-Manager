# PL-T06
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-01 | PARTIAL | patch_manager_mods/perf_lab_src/ghidra_scripts/ExportImportXrefs.java:26; :34; :39 | yes | Single-level thunk call sites are captured; only thunk-to-thunk chains are skipped. |

- ExportImportXrefs.java:25 seeds targets with only the external (IAT) symbol address.
- :26 loop 1 iterates a copy (new ArrayList<>(targets)) that holds only the IAT slot, so it expands exactly one referrer level.
- :27-28 refs to the IAT slot: every from-address is added to the live targets set (thunk jmp sites and direct call sites alike).
- :29-30 if a from-address lies in a thunk, its entry point is added to the live targets set (depth-1 thunk).
- :34-35 loop 2 iterates the live set, so depth-1 thunk entries get their referrers queried; real-function callers are recorded at :43.
- :39 drops refs whose calling function is a thunk, and no loop adds depth-2 thunks; calls to a thunk-of-thunk are never queried.
- :26 copy avoids ConcurrentModificationException while adding to the set; it is a guard, not the defect.
- Claim wording "IAT slot not expanded" is inaccurate: the IAT slot's referrers are expanded at :27.
- Fix: worklist over thunk entries only (add at :30, visited-set guard). Expanding every :28 from-address is unsafe (mid-function addresses).
- Side note: loop 2 also queries refs to non-thunk instruction addresses (:28), so a branch to an IAT-call instruction may log an extra JUMP row.
