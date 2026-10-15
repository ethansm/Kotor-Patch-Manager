# HN-T10

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HN-6n-06 | PARTIAL | patch_manager_mods/k1_spawn_probe_src/K1SpawnProbe.cpp:217; patch_manager_mods/k1_spawn_probe_src/ClientCore.h:38; patch_manager_mods/k1_spawn_probe_src/k1_client_core_test.cpp:154-172 | yes | Primitives unit-tested; dumpEvent composition, unreadable one-liners and CAP wiring untested. Extract pure step or add seam. |

- Anchor check: `if (!g_budget.fits(kEventLines)) return false;` is exact at K1SpawnProbe.cpp:217, inside dumpEvent (215-246), outside ClientCore.h.
- K1SpawnProbe.cpp:228, :235, :244 emit `skipped (unreadable)` one-liners via logf for the creature, DUMP24 and DUMPST blocks.
- K1SpawnProbe.cpp:32 documents kEventLines=25 as header + 16 creature rows + 4 DUMP24 + 4 DUMPST.
- ClientCore.h:38 fits() is check-only (no charge); ClientCore.h:24-35 reserve() is check-and-charge.
- ClientCore.h:44-49 takeCapMarker() is once-only and formats "CAP reached at n=<seq>"; the marker is not charged.
- k1_client_core_test.cpp:154, :171-172 test fits(1) and fits(0) only; no test calls fits(25) or dumpEvent.
- k1_client_core_test.cpp:193-205 (testLogBudgetCapMarker) covers takeCapMarker formatting and once-only, not the K1SpawnProbe.cpp emission path.
- k1_client_core_test.cpp:2 includes only ClientCore.h and check.h; K1SpawnProbe.cpp is never compiled into the host test.
- Outside range (grep hit only): K1SpawnProbe.cpp:72-74 gates logf and calls writeLine(marker) on takeCapMarker; the CAP emission wiring is not in ClientCore.h.
- Outside range (grep hit only): K1SpawnProbe.cpp:119 dumpBlock returns silently when reserve(rows) is refused; safe only because fits(25) runs first.

OPEN (not verified, outside ranges): whether logf routes every line through the K1SpawnProbe.cpp:70-74 gate (so the skipped one-liners are charged); kCreatureDwords/kPtrDwords values (assumed 256/64 for the 25-line arithmetic).
