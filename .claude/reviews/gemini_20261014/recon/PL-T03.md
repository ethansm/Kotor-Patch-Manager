# PL-T03
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| PL-4b-01 | PARTIAL | patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:172,210-211; patch_manager_mods/perf_lab_src/README.md:45 | no | Timing dependence real in code; flake rate and fresh-prefix claim are runtime-only. Anchor comment absent from ranges. |
| PL-4b-02 | UNSURE | patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:20,181; patch_manager_mods/perf_lab_src/README.md:45 | no | 7 MiB size confirmed; .bss address needs link map. VirtualAlloc drops the fixed-address coverage at line 181. |

- patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:20 declares static pad[0x700000] (7 MiB); the anchor text matches exactly.
- patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:20 comment says pad "covers the engine's code/data addresses used below".
- patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:181 passes fixed addresses 0x433400, 0x48FF50, 0x401234 to getimg.
- patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:175,179,183 declare other statics of 0x100-0x300 bytes (0x600 total); none is large.
- patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:172 runs 5 frames: bucket and actionbar work, SwapBuffers, then Sleep(20).
- patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:210 requires h[5]==4 exactly; the check has no tolerance or retry.
- patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:211 check text: 4 intervals in 16-31 ms; bucket bounds are defined outside the ranges.
- patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:210 other counts (frames 5, bucket_calls 60, max 14, actionbar 5) are deterministic; only h[5] is timing-based.
- patch_manager_mods/perf_lab_src/README.md:45 already documents the M5 first-run flake in a fresh Wine prefix and prescribes a rerun.
- 0x700000 exceeds the 0x20000 span between 0x400000 and 0x420000, so a ceiling check needs the image base (outside ranges).
