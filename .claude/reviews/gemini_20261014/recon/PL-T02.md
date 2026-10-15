# PL-T02
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4a-03 | PARTIAL | patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:457; :450-451; :478-480 | n/a | No fix stated; probe cap exists at 457; contention real; unbounded spike refuted |
- PerfLab.cpp:457 — loop condition `i < TopProbeMax && !placed` caps topInsert probing at 1024 iterations; the only loop in topInsert. Anchor exact.
- PerfLab.cpp:456 and :467 — topLock taken before the probe loop, released after the tombstone fallback; all hold time lies between them.
- PerfLab.cpp:459-463 — loop ends on match (459 sets placed) or first empty slot (463 break); tombstones are skipped, not stopped on.
- PerfLab.cpp:466 — post-loop tombstone fallback is O(1); overflow counter bumped after unlock (468).
- PerfLab.cpp:444 — TopSlots=16384, TopLiveMax=15872 (96.9% load), TopProbeMax=1024.
- PerfLab.cpp:450-451 — topLock spins 100 YieldProcessor calls, then SwitchToThread; no lock-holder preemption protection.
- PerfLab.cpp:440 — doc comment says hold is "a probe of at most TopProbeMax slots"; code matches this bound.
- PerfLab.cpp:478-480 — topRemove clears tombstones only when the next slot is empty; dense regions keep tombstones, lengthening probe runs toward the cap.
- PerfLab.cpp:445 — TopEnt is 32 bytes by layout, so 1024 probes read at most 32 KB (estimate, not measured; microseconds order).
- Not answerable from lines 436-480: how often live+tombstone density nears the 1024 cap in real sessions (needs runtime counts).
