# PL-T10

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4d-04 | PARTIAL | patch_manager_mods/perf_lab_src/analyze_runs.py:775-776; :754; :772; :783-789 | yes | Anchor exact at :775. Start not tied to first block, no attach-age cap: real. Harm needs runtime data. |

- analyze_runs.py:753-754: attach candidate needs unused, a.tick <= first (tdiff sign), a.tick > prev_end; no bound on first - a.tick.
- analyze_runs.py:756: picks max tick among candidates, i.e. the latest attach in the (prev_end, first] window.
- analyze_runs.py:759: prev_end = last block tick of the previous metrics launch (mlaunch order).
- analyze_runs.py:762-764: unused attaches become stub Launch(0) with attach set and no blocks.
- analyze_runs.py:772: stub end = attach tick + 60000, a fixed guess; never clamped to another launch.
- analyze_runs.py:775-776: no-attach launch start = prev.hi+1; no reference to blocks[0].tick and no clamp.
- analyze_runs.py:777-778: start = blocks[0].tick - 120000 only when there is no prev; sort key at :766-767 uses that value, not the assigned start.
- analyze_runs.py:783-789: find() returns any launch whose window covers tick, taking max lo; a stub with later lo wins blocks inside a metric window.
- analyze_runs.py:807-808: wall_s is derived from x.start, so a shifted start shifts the derived wall start.
- Fix note: clamping start to max(prev.hi+1, first-120000) is safe; an attach-age cap needs a tuned constant and is untested.
