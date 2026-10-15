# PL-T05
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4c-02 | REFUTED | patch_manager_mods/perf_lab_src/harness/smoke_probe11.cpp:37-40 | n/a | No fixed sleep: spin is wall-clock; left about 7 s; clamp at 39; fail path live |

- smoke_probe11.cpp:19 stuckSpin loops on GetTickCount until >= ms elapsed; wall-clock bounded, no Sleep().
- smoke_probe11.cpp:37 t0 taken before the spin; line 38 calls stuckSpin(18000) with no hook() frames.
- smoke_probe11.cpp:39 left = 25000 - elapsed, clamped to 0 only if elapsed >= 25000; normal elapsed ~18000 gives left ~7000 ms.
- smoke_probe11.cpp:40 waitForLine(hangprobe_report.txt, MAIN_STALL, left) inside KT_CHECK: a missing header times out and fails the check.
- smoke_probe11.cpp:22 deletes hangprobe_report.txt at start, so a stale MAIN_STALL header from an earlier run cannot pass the check.
- smoke_probe11.cpp:32 last hook() frame precedes t0 (line 37), so the 15 s no-frame window starts before t0; spin ends at t0+18 s, giving >=3 s margin.
- smoke_probe11.cpp:29,32 Sleep(16) appears only in frame loops outside the stall window; no fixed sleep inside it.
- smoke_probe11.cpp:11 comment says a flaky MAIN_STALL is "downgraded to a printed WARN, see below"; no WARN code exists in lines 1-43.
- smoke_probe11.cpp:12 comment: no MAIN_STALL flakiness seen in 3 consecutive runs under Proton Experimental Wine.
- Residual risk: left reaches 0 only if the spin overruns by ~7 s (scheduler starvation); waitForLine(..., 0) semantics live in wait_line.h (not read).
