# G2 resolver input: batch perf_lab (12 open fids, 10 topics)

## PL-T01 (P1) — G1 unit u005, facts file recon/PL-T01.md
Question: In PerfLabResMalloc (line 1349) and PerfLabResFreeData (line 1378), is cres dereferenced at c+0x18/c+0x10/c+8 with a plain pointer cast (no SafeRead/exception guard) on a pointer that can be non-null yet invalid (not just null)?
G1 windows (do NOT reuse): patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1340-1392;patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:386-396
G1 OPEN: Can the engine pass a non-null invalid CRes to 0x712F30 or 0x7120C0 (outside ranges)?
- PL-4a-01 [PARTIAL] Raw unchecked deref of CRes fields in PerfLabResMalloc hook, no SafeRead | file patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp | fn PerfLabResMalloc | cat memory-safety | G1 evidence patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1349 | G1 note: Unguarded c+0x18 read is real; non-null-but-invalid premise is engine-dependent, not shown here. | anchor now patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1349 drift none
- PL-4a-02 [PARTIAL] Raw unchecked deref of CRes fields in PerfLabResFreeData hook | file patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp | fn PerfLabResFreeData | cat memory-safety | G1 evidence patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1378 | G1 note: Unguarded c+0x10 and c+8 reads are real; validity premise engine-dependent, as in PL-4a-01. | anchor now patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1378 drift none

## PL-T02 (P2) — G1 unit u041, facts file recon/PL-T02.md
Question: Can topInsert hold topLock across up to TopProbeMax=1024 probes while other threads spin on it, such that a real latency spike is possible beyond the documented bounded hold (lines 436-480)?
G1 windows (do NOT reuse): patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:436-480
G1 OPEN: Do real sessions push live+tombstone density near the 1024-probe cap? Needs runtime counts.
- PL-4a-03 [PARTIAL] topInsert spinlock with linear probing may cause spin contention and latency spikes | file patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp | fn topInsert | cat logic | G1 evidence patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:457; :450-451; :478-480 | G1 note: No fix stated; probe cap exists at 457; contention real; unbounded spike refuted | anchor now patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:457 drift none

## PL-T03 (P3) — G1 unit u073, facts file recon/PL-T03.md
Question: In smoke_perflab3.cpp, is the M5 frame-histogram check (h[5]==4 after Sleep(20) loop) timing-dependent so it can fail spuriously, and does the 7 MB static pad[] stay below the 0x420000 .bss ceiling the README warns about?
G1 windows (do NOT reuse): patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:15-30;patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:165-215;patch_manager_mods/perf_lab_src/README.md:45-45
G1 OPEN: Is 0x420000 an absolute ceiling, and what are the harness image base and .bss end in the link map?
- PL-4b-01 [PARTIAL] Frame-histogram check in smoke_perflab3 is timing-dependent (Sleep(20)) and flaky | file patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp | fn main | cat test-gap | G1 evidence patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:172,210-211; patch_manager_mods/perf_lab_src/README.md:45 | G1 note: Timing dependence real in code; flake rate and fresh-prefix claim are runtime-only. Anchor comment absent from ranges. | anchor now - drift noanchor
- PL-4b-02 [UNSURE] 7 MB static pad in smoke_perflab3 may push .bss past 0x420000; use VirtualAlloc | file patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp | fn main | cat test-gap | G1 evidence patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:20,181; patch_manager_mods/perf_lab_src/README.md:45 | G1 note: 7 MiB size confirmed; .bss address needs link map. VirtualAlloc drops the fixed-address coverage at line 181. | anchor now patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:20 drift none

## PL-T06 (P3) — G1 unit u076, facts file recon/PL-T06.md
Question: In ExportImportXrefs.java, does the first loop iterate only over the snapshot new ArrayList<>(targets) so thunk entry points discovered via the IAT slot's referrers are never expanded further, causing call sites that go through a thunk to be skipped?
G1 windows (do NOT reuse): patch_manager_mods/perf_lab_src/ghidra_scripts/ExportImportXrefs.java
G1 OPEN: Do real swkotor2.exe imports produce depth-2 thunk chains (thunk calling thunk calling the IAT slot)?
- PL-4d-01 [PARTIAL] ExportImportXrefs misses import call sites through compiler thunks (IAT slot not expanded) | file patch_manager_mods/perf_lab_src/ghidra_scripts/ExportImportXrefs.java | fn run | cat tooling | G1 evidence patch_manager_mods/perf_lab_src/ghidra_scripts/ExportImportXrefs.java:26; :34; :39 | G1 note: Single-level thunk call sites are captured; only thunk-to-thunk chains are skipped. | anchor now patch_manager_mods/perf_lab_src/ghidra_scripts/ExportImportXrefs.java:26 drift none

## PL-T09 (P3) — G1 unit u079, facts file recon/PL-T09.md
Question: In parse_perflab_pair/parse_textlog_pair, does the new-file-wins selection discard all older-file attaches/blocks (no merge), and does render()'s Sources table iterate only ALL_FILES (legacy names) so alias-read logs show ABSENT?
G1 windows (do NOT reuse): patch_manager_mods/perf_lab_src/analyze_runs.py:405-425;patch_manager_mods/perf_lab_src/analyze_runs.py:510-531;patch_manager_mods/perf_lab_src/analyze_runs.py:1323-1335
G1 OPEN: Is ALL_FILES legacy-only, and does infos[n] mark alias-read logs ABSENT? (code outside range)
- PL-4d-03 [PARTIAL] Old/new log pair selection drops older launches, tick wrap picks stale file, Sources table shows ABSENT | file patch_manager_mods/perf_lab_src/analyze_runs.py | fn parse_perflab_pair/parse_textlog_pair/render | cat tooling | G1 evidence patch_manager_mods/perf_lab_src/analyze_runs.py:415-416, 524-526, 1327 | G1 note: No-merge is real; tick compare is half-range; ALL_FILES content outside range. | anchor now patch_manager_mods/perf_lab_src/analyze_runs.py:524 drift none

## PL-T10 (P3) — G1 unit u080, facts file recon/PL-T10.md
Question: In build_launches, when a metrics launch has no matched attach, is x.start set to prev.hi+1 (instead of from its first block tick) and is there no upper bound on tdiff(first, a['tick']) in the attach-matching pass?
G1 windows (do NOT reuse): patch_manager_mods/perf_lab_src/analyze_runs.py:741-822
G1 OPEN: Reported duration use after :822 and tdiff sign convention are outside range, not read; sign inferred from usage.
- PL-4d-04 [PARTIAL] build_launches: missing attach inflates start/duration, stub hi overlap, unbounded attach stealing | file patch_manager_mods/perf_lab_src/analyze_runs.py | fn build_launches | cat tooling | G1 evidence patch_manager_mods/perf_lab_src/analyze_runs.py:775-776; :754; :772; :783-789 | G1 note: Anchor exact at :775. Start not tied to first block, no attach-age cap: real. Harm needs runtime data. | anchor now patch_manager_mods/perf_lab_src/analyze_runs.py:775 drift none

## PL-T11 (P3) — G1 unit u081, facts file recon/PL-T11.md
Question: In resolve_sites.py mblocks, does int(x) over l.split()[1:] on an RT line containing a token like redemB=5 or 'stat' raise ValueError, and does a blank line inside a block raise IndexError at l.split()[0]?
G1 windows (do NOT reuse): patch_manager_mods/perf_lab_src/resolve_sites.py:45-141
G1 OPEN: Does the perf-lab producer ever emit non-integer RT tokens or blank lines inside #METRICS blocks? Outside the read ranges.
- PL-4d-05 [PARTIAL] resolve_sites crashes on RT redemB/stat tokens and blank lines; launches not split | file patch_manager_mods/perf_lab_src/resolve_sites.py | fn mblocks/loads/kv | cat tooling | G1 evidence patch_manager_mods/perf_lab_src/resolve_sites.py:71, :73 | G1 note: Blank line in block raises IndexError at 73 (confirmed). RT int() ValueError real; producer emission unverified. | anchor now patch_manager_mods/perf_lab_src/resolve_sites.py:71 drift none

## PL-T13 (P3) — G1 unit u083, facts file recon/PL-T13.md
Question: In hookbytes.py line 21, does Python precedence make t.startswith('J') True for every JMP (including indirect ones) so REL-BRANCH-IN-STOLEN is reported falsely, and is the while loop at line 17 unbounded on k?
G1 windows (do NOT reuse): patch_manager_mods/perf_lab_src/hookbytes.py
G1 OPEN: Do real thunk rows in the steam function dumps hold fewer than 5 bytes, triggering the :19 IndexError?
- PL-4d-08 [PARTIAL] hookbytes operator precedence flags all JMP as rel branch; unbounded index; thunk rows crash | file patch_manager_mods/perf_lab_src/hookbytes.py | fn <module>/insns | cat tooling | G1 evidence patch_manager_mods/perf_lab_src/hookbytes.py:21; patch_manager_mods/perf_lab_src/hookbytes.py:18-19 | G1 note: Every JMP flagged (indirect too) via startswith('J'), not precedence; loop unguarded; thunk crash needs dump data | anchor now patch_manager_mods/perf_lab_src/hookbytes.py:21 drift none

## PL-T14 (P3) — G1 unit u084, facts file recon/PL-T14.md
Question: Does analyze() read only b.rt_redem[TPC_RESTYPE] while parse_metrics stores standalone 'RT stat redemB=' under rt_redem['stat'], so those values never reach a['redemB']?
G1 windows (do NOT reuse): patch_manager_mods/perf_lab_src/analyze_runs.py:318-335;patch_manager_mods/perf_lab_src/analyze_runs.py:983-992
G1 OPEN: Does any code outside analyze_runs.py:318-992 read rt_redem["stat"], and is stat redemB the same metric as TPC redemB?
- PL-4d-09 [PARTIAL] analyze ignores 'RT stat redemB=' values stored under rt_redem['stat'] | file patch_manager_mods/perf_lab_src/analyze_runs.py | fn analyze | cat tooling | G1 evidence patch_manager_mods/perf_lab_src/analyze_runs.py:331; :987-988 | G1 note: Read gap real (987 reads only TPC key). Origin text, TPC_RESTYPE value, other readers out of range. Stat-as-TPC fallback unproven. | anchor now patch_manager_mods/perf_lab_src/analyze_runs.py:987 drift none

## PL-T15 (P3) — G1 unit u085, facts file recon/PL-T15.md
Question: In Launch._mk does the outer 'for b in (down, up)' loop continue to up and overwrite L['module'] after down already set it, and does Launch.flags() exclude only exe/scan (not tick) from attach kv?
G1 windows (do NOT reuse): patch_manager_mods/perf_lab_src/analyze_runs.py:552-564;patch_manager_mods/perf_lab_src/analyze_runs.py:608-638
G1 OPEN: Does attach kv ever carry "tick", and can a block have hdr mod/area while .module is None? Parser code is outside the read ranges.
- PL-4d-10 [PARTIAL] Launch._mk overwrites module from LOAD_UP; flags() renders tick as config flag | file patch_manager_mods/perf_lab_src/analyze_runs.py | fn Launch._mk/Launch.flags | cat tooling | G1 evidence patch_manager_mods/perf_lab_src/analyze_runs.py:630-636; analyze_runs.py:555 | G1 note: Module overwrite CONFIRMED (no outer break); tick-as-flag needs attach kv to hold tick, not in range | anchor now patch_manager_mods/perf_lab_src/analyze_runs.py:630 drift none
