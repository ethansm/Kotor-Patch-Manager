# PL-T01
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4a-01 | PARTIAL | patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1349 | yes | Unguarded c+0x18 read is real; non-null-but-invalid premise is engine-dependent, not shown here. |
| PL-4a-02 | PARTIAL | patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1378 | yes | Unguarded c+0x10 and c+8 reads are real; validity premise engine-dependent, as in PL-4a-01. |
- PerfLab.cpp:1346 Malloc guard is `!cres` (null only); no validity check on the CRes pointer.
- PerfLab.cpp:1349 is the first raw deref of cres+0x18 in PerfLabResMalloc; it runs before any saferead use.
- PerfLab.cpp:389 restypeOf reads cres+0x14 with a plain cast before its only saferead (392), so Malloc's reads are unguarded there too.
- PerfLab.cpp:391-392 the only saferead in range guards the key-entry read (ke+0x1A), not the CRes fields.
- PerfLab.cpp:1373 comment says the FreeData hook "mirrors the engine's own test for a real free", so it reads the fields the engine reads.
- PerfLab.cpp:1374-1378 FreeData guard is `!g_m || !cres`; c+0x10 and c+8 are plain casts with no exception guard.
- PerfLab.cpp:1369 (Evict) and 1391 (Release) read CRes fields the same way with only a `!cres` check.
- Fix direction: wrapping the reads in the file's own saferead pattern (as restypeOf does at 391-392) is correct and safe; failed read returns early, losing only telemetry.
- Hooks are engine-called (comments 1344, 1363, 1373, 1384), so CRes validity is an engine-side fact. Outside ranges, so OPEN.
