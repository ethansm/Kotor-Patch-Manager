# G2.3 skeptic input: perf_lab (16 fids)

## PL-T01 (P1): In PerfLabResMalloc (line 1349) and PerfLabResFreeData (line 1378), is cres dereferenced at c+0x18/c+0x10/c+8 with a plain pointer cast (no SafeRead/exception guard) on a pointer that can be non-null yet invalid (not just null)?
- **PL-4a-01** v2=UNSURE (G1 PARTIAL) | claimed high memory-safety | Raw unchecked deref of CRes fields in PerfLabResMalloc hook, no SafeRead | file patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp fn PerfLabResMalloc
  - G1 evidence: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1349 | G1 note: Unguarded c+0x18 read is real; non-null-but-invalid premise is engine-dependent, not shown here.
  - v2 evidence: 0x712F3F; 0x7120C9 | v2 notes: [re UNSURE] re.md PL-T01: Malloc derefs CRes+0x18 unguarded (game itself faults on NULL); FreeResourceData NULL-tests first; stale non-null undecidable statically
- **PL-4a-02** v2=UNSURE (G1 PARTIAL) | claimed high memory-safety | Raw unchecked deref of CRes fields in PerfLabResFreeData hook | file patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp fn PerfLabResFreeData
  - G1 evidence: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1378 | G1 note: Unguarded c+0x10 and c+8 reads are real; validity premise engine-dependent, as in PL-4a-01.
  - v2 evidence: 0x712F3F; 0x7120C9 | v2 notes: [re UNSURE] re.md PL-T01: as PL-4a-01; hook reading CRes+0x10 before 0x7120C9 test faults on NULL where game would not

## PL-T02 (P2): Can topInsert hold topLock across up to TopProbeMax=1024 probes while other threads spin on it, such that a real latency spike is possible beyond the documented bounded hold (lines 436-480)?
- **PL-4a-03** v2=UNSURE (G1 PARTIAL) | claimed medium logic | topInsert spinlock with linear probing may cause spin contention and latency spikes | file patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp fn topInsert
  - G1 evidence: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:457; :450-451; :478-480 | G1 note: No fix stated; probe cap exists at 457; contention real; unbounded spike refuted
  - v2 evidence: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:488-493; :1181-1187 | v2 notes: [gem2 UNSURE] u028: Snapshot lock spans full scan; hook blocking and call cadence unverified in range; no fix stated.

## PL-T03 (P3): In smoke_perflab3.cpp, is the M5 frame-histogram check (h[5]==4 after Sleep(20) loop) timing-dependent so it can fail spuriously, and does the 7 MB static pad[] stay below the 0x420000 .bss ceiling the README warns about?
- **PL-4b-01** v2=UNSURE (G1 PARTIAL) | claimed medium test-gap | Frame-histogram check in smoke_perflab3 is timing-dependent (Sleep(20)) and flaky | file patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp fn main
  - G1 evidence: patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:172,210-211; patch_manager_mods/perf_lab_src/README.md:45 | G1 note: Timing dependence real in code; flake rate and fresh-prefix claim are runtime-only. Anchor comment absent from ranges.
  - v2 evidence: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:766-768; check at smoke_perflab3.cpp:210 unread | v2 notes: [gem2 UNSURE] u029: Timing-sensitive per code; :210 check and anchor unread. Range tolerance preferred over retry.
- **PL-4b-02** v2=PARTIAL (G1 UNSURE) | claimed medium test-gap | 7 MB static pad in smoke_perflab3 may push .bss past 0x420000; use VirtualAlloc | file patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp fn main
  - G1 evidence: patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:20,181; patch_manager_mods/perf_lab_src/README.md:45 | G1 note: 7 MiB size confirmed; .bss address needs link map. VirtualAlloc drops the fixed-address coverage at line 181.
  - v2 evidence: patch_manager_mods/perf_lab_src/harness/smoke_perflab3.cpp:20 | v2 notes: [det PARTIAL] d037: Single pad at :20 confirmed; exit-2 guard at :92 not in output.

## PL-T04 (P3): In smoke_activationfix.cpp run mode, when GetForegroundWindow()!=w does the smoke only print SKIP for the real-foreground check while the other (setFg-driven) checks still run and can fail?
- **PL-4c-01** v2=CONFIRMED (G1 CONFIRMED) | claimed medium test-gap | activationfix smoke skips real foreground check when harness window is not foreground | file patch_manager_mods/perf_lab_src/harness/smoke_activationfix.cpp fn main
  - G1 evidence: patch_manager_mods/perf_lab_src/harness/smoke_activationfix.cpp:71-73 | G1 note: No fix stated; unconditional real check would fail off-foreground since post is forwarded.
  - v2 evidence: patch_manager_mods/perf_lab_src/harness/smoke_activationfix.cpp:71-73 | v2 notes: 

## PL-T06 (P3): In ExportImportXrefs.java, does the first loop iterate only over the snapshot new ArrayList<>(targets) so thunk entry points discovered via the IAT slot's referrers are never expanded further, causing call sites that go through a thunk to be skipped?
- **PL-4d-01** v2=UNSURE (G1 PARTIAL) | claimed high tooling | ExportImportXrefs misses import call sites through compiler thunks (IAT slot not expanded) | file patch_manager_mods/perf_lab_src/ghidra_scripts/ExportImportXrefs.java fn run
  - G1 evidence: patch_manager_mods/perf_lab_src/ghidra_scripts/ExportImportXrefs.java:26; :34; :39 | G1 note: Single-level thunk call sites are captured; only thunk-to-thunk chains are skipped.
  - v2 evidence: - | v2 notes: [re UNSURE] re.md PL-T06: thunk chains not enumerated

## PL-T07 (P3): In ExportCallGraph.java, when a non-thunk function calls an internal thunk g with a non-null non-external thunked target, is the emitted edge's callee g's entry point rather than the thunk target?
- **PL-4d-02** v2=CONFIRMED (G1 CONFIRMED) | claimed medium tooling | ExportCallGraph does not resolve internal thunk callees to their targets | file patch_manager_mods/perf_lab_src/ghidra_scripts/ExportCallGraph.java fn run
  - G1 evidence: patch_manager_mods/perf_lab_src/ghidra_scripts/ExportCallGraph.java:42-46 | G1 note: Internal thunk falls through to line 46 and emits thunk entry; target gets no credit.
  - v2 evidence: patch_manager_mods/perf_lab_src/ghidra_scripts/ExportCallGraph.java:42-46 | v2 notes: 

## PL-T08 (P3): In NewNullCheck.java, does the forward walk (k<8) neither stop at a RET/function-exit nor drop a register from held on a non-MOV instruction that overwrites it (XOR/LEA/POP), so checked=yes can be reported falsely?
- **PL-4d-06** v2=CONFIRMED (G1 CONFIRMED) | claimed high tooling | NewNullCheck forward walk crosses RET and mishandles register overwrites, false checked=yes | file patch_manager_mods/perf_lab_src/ghidra_scripts/NewNullCheck.java fn run
  - G1 evidence: patch_manager_mods/perf_lab_src/ghidra_scripts/NewNullCheck.java:63,66,67,69-73 | G1 note: Both gaps real: RET not a stop (67 continue); only MOV updates held.
  - v2 evidence: patch_manager_mods/perf_lab_src/ghidra_scripts/NewNullCheck.java:63,66,67,69-73 | v2 notes: 

## PL-T09 (P3): In parse_perflab_pair/parse_textlog_pair, does the new-file-wins selection discard all older-file attaches/blocks (no merge), and does render()'s Sources table iterate only ALL_FILES (legacy names) so alias-read logs show ABSENT?
- **PL-4d-03** v2=PARTIAL (G1 PARTIAL) | claimed high tooling | Old/new log pair selection drops older launches, tick wrap picks stale file, Sources table shows ABSENT | file patch_manager_mods/perf_lab_src/analyze_runs.py fn parse_perflab_pair/parse_textlog_pair/render
  - G1 evidence: patch_manager_mods/perf_lab_src/analyze_runs.py:415-416, 524-526, 1327 | G1 note: No-merge is real; tick compare is half-range; ALL_FILES content outside range.
  - v2 evidence: perf_lab_src/analyze_runs.py:96-98, :1379, :1382-1383, :1386-1390 | v2 notes: [gem2 PARTIAL] u030: Alias-only: legacy ReadInfo ABSENT; alias ReadInfo never in infos. Table (1327) unread.

## PL-T10 (P3): In build_launches, when a metrics launch has no matched attach, is x.start set to prev.hi+1 (instead of from its first block tick) and is there no upper bound on tdiff(first, a['tick']) in the attach-matching pass?
- **PL-4d-04** v2=UNSURE (G1 PARTIAL) | claimed high tooling | build_launches: missing attach inflates start/duration, stub hi overlap, unbounded attach stealing | file patch_manager_mods/perf_lab_src/analyze_runs.py fn build_launches
  - G1 evidence: patch_manager_mods/perf_lab_src/analyze_runs.py:775-776; :754; :772; :783-789 | G1 note: Anchor exact at :775. Start not tied to first block, no attach-age cap: real. Harm needs runtime data.
  - v2 evidence: patch_manager_mods/perf_lab_src/analyze_runs.py:1093-1095; :1098-1099 | v2 notes: [gem2 UNSURE] u031: Render prints start/duration as real, no approx flag; derivation at :776 outside my ranges.

## PL-T11 (P3): In resolve_sites.py mblocks, does int(x) over l.split()[1:] on an RT line containing a token like redemB=5 or 'stat' raise ValueError, and does a blank line inside a block raise IndexError at l.split()[0]?
- **PL-4d-05** v2=UNSURE (G1 PARTIAL) | claimed high tooling | resolve_sites crashes on RT redemB/stat tokens and blank lines; launches not split | file patch_manager_mods/perf_lab_src/resolve_sites.py fn mblocks/loads/kv
  - G1 evidence: patch_manager_mods/perf_lab_src/resolve_sites.py:71, :73 | G1 note: Blank line in block raises IndexError at 73 (confirmed). RT int() ValueError real; producer emission unverified.
  - v2 evidence: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp:1209-1215; patch_manager_mods/perf_lab_src/analyze_runs.py:281-299 | v2 notes: [gem2 UNSURE] u032: RT tokens all integer; no bare blank line in 1205-1236; writers outside range unread; redemB= unverified

## PL-T12 (P3): In persist_findings.py, does an empty or trailing-comma argv[2] yield ver containing '' so that any(x in ev.lower() for x in ver) is True for every row?
- **PL-4d-07** v2=CONFIRMED (G1 CONFIRMED) | claimed high tooling | persist_findings empty verified-token matches every row, upgrades all to high confidence | file patch_manager_mods/perf_lab_src/persist_findings.py fn <module>
  - G1 evidence: patch_manager_mods/perf_lab_src/persist_findings.py:8,15,16 | G1 note: Empty or trailing-comma argv[2] yields '' token; every row forced high
  - v2 evidence: patch_manager_mods/perf_lab_src/persist_findings.py:8,15,16 | v2 notes: 

## PL-T13 (P3): In hookbytes.py line 21, does Python precedence make t.startswith('J') True for every JMP (including indirect ones) so REL-BRANCH-IN-STOLEN is reported falsely, and is the while loop at line 17 unbounded on k?
- **PL-4d-08** v2=CONFIRMED (G1 PARTIAL) | claimed medium tooling | hookbytes operator precedence flags all JMP as rel branch; unbounded index; thunk rows crash | file patch_manager_mods/perf_lab_src/hookbytes.py fn <module>/insns
  - G1 evidence: patch_manager_mods/perf_lab_src/hookbytes.py:21; patch_manager_mods/perf_lab_src/hookbytes.py:18-19 | G1 note: Every JMP flagged (indirect too) via startswith('J'), not precedence; loop unguarded; thunk crash needs dump data
  - v2 evidence: stdout (count=8) | v2 notes: [det CONFIRMED] d038: Eight functions under 5 bytes exist; hazard needs an ADDR inside one (not shown).

## PL-T14 (P3): Does analyze() read only b.rt_redem[TPC_RESTYPE] while parse_metrics stores standalone 'RT stat redemB=' under rt_redem['stat'], so those values never reach a['redemB']?
- **PL-4d-09** v2=CONFIRMED (G1 PARTIAL) | claimed medium tooling | analyze ignores 'RT stat redemB=' values stored under rt_redem['stat'] | file patch_manager_mods/perf_lab_src/analyze_runs.py fn analyze
  - G1 evidence: patch_manager_mods/perf_lab_src/analyze_runs.py:331; :987-988 | G1 note: Read gap real (987 reads only TPC key). Origin text, TPC_RESTYPE value, other readers out of range. Stat-as-TPC fallback unproven.
  - v2 evidence: patch_manager_mods/perf_lab_src/analyze_runs.py:331 | v2 notes: [det CONFIRMED] d039: \"stat\" key stored only at :331; no RT stat emitter in PerfLab.cpp; reads use TPC_RESTYPE.

## PL-T16 (P3): When rb.loaded contains REFUSED and rb.lines has no separate REFUSED lines, does render_launch leave ref empty so the line prints 'REFUSED none'?
- **PL-4d-11** v2=CONFIRMED (G1 CONFIRMED) | claimed low tooling | render_launch shows 'REFUSED none' when loaded line itself reports REFUSED | file patch_manager_mods/perf_lab_src/analyze_runs.py fn render_launch
  - G1 evidence: patch_manager_mods/perf_lab_src/analyze_runs.py:1192 | G1 note: Loaded REFUSED with no REFUSED lines yields empty ref and 'REFUSED none' at :1195.
  - v2 evidence: patch_manager_mods/perf_lab_src/analyze_runs.py:1192 | v2 notes: 
