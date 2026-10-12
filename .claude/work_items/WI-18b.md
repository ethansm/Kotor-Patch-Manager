# WI-18b Perf-lab logging on KpLog.h channels (C122)
area: perf-lab; common-kplog
size: M
ready: hold
items: C122
files: patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp
out-of-repo: /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common/KpLog.h
blockers: after: WI-18. Owner GO (doc 10 line 421, not row 420). KpLog.h is already included at PerfLab.cpp:77; the main channel uses it. kplog.ini path: unknown (no perflab/kplog.ini in repo). Same file as WI-19 C178 (private alloc and metrics writers): serialize after WI-19 or merge the edits.
docs: patch_manager_mods/10_engine_optimization_research.md:C122@L421
deferred: C122

## Goal
Wire the remaining perf-lab channels and `kplog.ini` onto KpLog.h. Keep tag text byte-identical. Do not start before owner GO.

## Steps
None ungated. C122 is deferred.

## Verify
After GO only:
1. `python3 scripts/build_check.py`
2. `python3 scripts/run_tests.py <perf-lab filter>`: suite name unknown. Perf-lab smokes run as `wine:<n>` with --wine (scripts/run_tests.py:20).

## Docs
- After GO and build, close C122 with `python3 scripts/plan_manager.py mark-done <phase>`. Phase shorthand: unknown.
- CHANGELOG bump: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C122 patch_manager_mods/10_engine_optimization_research.md:421 id=- open
