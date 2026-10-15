# perf_lab triage (sets perf_lab_a 4a/4b/4c, perf_lab_b 4d): 18 findings, 16 topics, 0 dropped

- PL-T01 P1: PerfLabResMalloc/FreeData hooks deref CRes (c+0x18/0x10/8) raw, no SafeRead (4a-01, 4a-02)
- PL-T02 P2: topInsert spinlock + up to 1024-probe hold may spike latency (4a-03)
- PL-T03 P3: smoke_perflab3 M5 histogram timing flake; 7 MB static pad vs 0x420000 (4b-01, 4b-02)
- PL-T04 P3: smoke_activationfix skips real-foreground check when window not foreground (4c-01)
- PL-T05 P3: smoke_probe11 blind 18 s busy-wait MAIN_STALL brittleness (4c-02)
- PL-T06 P3: ExportImportXrefs misses thunk call sites (snapshot loop) (4d-01)
- PL-T07 P3: ExportCallGraph does not resolve internal thunk callees (4d-02)
- PL-T08 P3: NewNullCheck walks past RET, stale held registers (4d-06)
- PL-T09 P3: analyze_runs old/new log pair: no merge, tick wrap, Sources shows ABSENT (4d-03)
- PL-T10 P3: build_launches start/hi/attach-stealing errors without attach (4d-04)
- PL-T11 P3: resolve_sites mblocks crashes on RT redemB/stat, blank lines; no launch split (4d-05)
- PL-T12 P3: persist_findings empty verified token upgrades all rows (4d-07)
- PL-T13 P3: hookbytes precedence flags all JMP; unbounded index; thunk rows (4d-08)
- PL-T14 P3: analyze ignores rt_redem['stat'] (4d-09)
- PL-T15 P3: Launch._mk module overwrite; flags() shows tick (4d-10)
- PL-T16 P3: render_launch prints "REFUSED none" (4d-11)

## Duplicates (within batch)
None folded. PL-4a-01 and PL-4a-02 are the same root cause (raw CRes deref) and share topic PL-T01. PL-4b-02 and PL-4b-01 both from smoke_perflab3 (PL-T03). PL-4d-05 (resolve_sites RT redemB) and PL-4d-09 (analyze rt_redem stat) share the same log token but are different code.

## Findings citing files outside the bundle manifest
- PL-4b-01: anchor is a sentence from `patch_manager_mods/perf_lab_src/README.md:45` (bundle 4a), not from smoke_perflab3.cpp (4b); the quoted `// The M5 ...` comment form does not exist in the cpp. Gemini restated the README note as a finding.
- PL-4b-02: anchor line exists at smoke_perflab3.cpp:20 but the README (4a) already prescribes VirtualAlloc for NEW large buffers; the finding is hypothetical about future additions.
- PL-4c-02 / PL-4c-01: files exist in manifest 4c (smoke_probe11.cpp:38, smoke_activationfix.cpp:73). PL-4c-01 anchor (CreateWindowA) is at retired/smoke_probe13.cpp:29 and smoke_probe14.cpp:36; no match in smoke_activationfix.cpp by exact text, the SKIP line is at smoke_activationfix.cpp:73.
- PL-4d-03/09/10 analyze_runs.py depends on `scripts/kplog_parse.py` (outside bundle; exists in repo). PL-4d-02/-06 reference `operator_new_arr` at 0x91D288 (engine address, not repo).

## Cross-batch candidates
None in `_shared_src/`. PL-T01 touches `../_shared/SafeRead.h` (included by PerfLab.cpp:75); other mod batches may flag raw CRes/engine pointer derefs similarly.

## Backlog (not assessed / omitted)
- 4a: ghidra_scripts omitted per Gemini (actually reviewed under 4d); external Windows headers and KpLog.h assumed.
- 4b: "omitted external test frameworks" (generic).
- 4c: harness/retired/ smoke scripts treated as reference only, not reviewed.
- 4d: fixtures/game/*, fixtures/game050/*, fixtures/gen_fixtures.py omitted (manifest-omit); scripts/kplog_parse.py; ghidra_knowledge/kotor2_ghidra_knowledge.db and steam_functions.tsv not in bundle.
- Step 1 integrity check (bundle ends with </review_bundle>) passed for 4a/4b/4c; no truncation.
