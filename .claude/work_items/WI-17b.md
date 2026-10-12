# WI-17b Common code-patch and INI helpers (C102, C106, C123)
area: common-codepatch; common-inikit; perf-lab
size: M
ready: hold
after: WI-17
items: C102,C106,C123
files: patch_manager_mods/_shared_src/CodePatch.h; patch_manager_mods/_shared_src/IniKit.h; patch_manager_mods/perf_lab_src/perflab/PerfLab.cpp
out-of-repo: none
blockers: after: WI-17. Every row needs owner GO (see deferred). Facts from source: no `writeCode(` function body remains in repo .cpp or .h (CodePatch.h:4-16 comments only); `writeChecked` (CodePatch.h:89) is the shared form. The enabled=0 parser is absent from PerfLab.cpp; IniKit.h already has enabledFlag/enabledFile (:8). The TX cave builder is `applyTx()` (PerfLab.cpp:792) and already calls codepatch::writeChecked (:803). `patchIat` is at CodePatch.h:134 and used by PerfLab.cpp:1875-1879 and LoadHangProbe.cpp:724-726. Owner must confirm whether C106 and C123 close as verify-only.
docs: patch_manager_mods/09_load_hang_investigation.md:C102@L306,C106@L311; patch_manager_mods/10_engine_optimization_research.md:C123@L422
deferred: C102,C106,C123

## Goal
Consolidate B-id Common helpers in `_shared_src` and the perf-lab TX cave builder onto them. Keep `patchIat` names unchanged. Do not start before owner GO.

## Steps
None ungated. All three rows are deferred.

## Verify
After GO only:
1. `python3 scripts/compat_check.py patch_manager_mods/_shared_src/CodePatch.h patch_manager_mods/_shared_src/IniKit.h`
2. `python3 scripts/build_check.py`
3. `python3 scripts/run_tests.py <filter>`: suite name unknown.
4. `python3 scripts/check_shared_headers.py --sync` only if canonical headers change (KPM write, GO needed).

## Docs
- After GO and build, close with `python3 scripts/plan_manager.py mark-done <phase>`. Phase shorthand: unknown.
- Doc 09 line 311 quotes patchIat at `:345` (perf-lab). The live definition is CodePatch.h:134. Correct when closing.
- CHANGELOG bump: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C102 patch_manager_mods/09_load_hang_investigation.md:306 id=- open
- C106 patch_manager_mods/09_load_hang_investigation.md:311 id=B5 open
- C123 patch_manager_mods/10_engine_optimization_research.md:422 id=- open
