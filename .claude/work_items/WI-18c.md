# WI-18c KpLog remainder: probe, companion AI, parser, design rows (C103, C168-C173)
area: common-kplog; load-hang-probe; companion-ai; kplog-parser
size: M
ready: ready
items: C103,C168,C169,C170,C172,C173
files: patch_manager_mods/load_hang_probe_src/LoadHangProbe.cpp; patch_manager_mods/companion_ai_src/CompanionAI.cpp; scripts/kplog_parse.py
out-of-repo: /home/ethansm/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/Patches/Common/KpLog.h
blockers: after: WI-18. Mixed patches (probe, companion, parser, design rows) stay together because the three-file split cap is reached. Re-split each row when it gets GO. C103 waits on doc 19 Phase 6 (LoadHangProbe.cpp private writer hangprobe_log.txt at :91). C173 waits on owner agreement (CompanionAI.cpp private writer, g_log fopen at :1434). C172 waits on phase 3 play test; kplog_parse.py already has merged sessions (:29) and MARK (:13); F9 MARK hotkey: unknown. C168 applies only if RFC #180 is rejected; KpLog core path: unknown. C169 parked, not built.
docs: patch_manager_mods/09_load_hang_investigation.md:C103@L308; patch_manager_mods/19_shared_logging_design.md:C168@L57,C169@L66,C170@L92,C172@L119,C173@L121
deferred: C103,C168,C169,C172,C173

## Goal
Keep the gated KpLog rows in one place until each gets its GO. Ready work here is the C170 report only.

## Steps
1. (C170, no GO needed, report only) Run `rg -l debuglog patch_manager_mods scripts -g '!*.md'`. Current result: no code hits, so `debuglog=` is not implemented. Record "not implemented". Do not build debuglog parsing.

## Verify
1. `rg -l debuglog patch_manager_mods scripts -g '!*.md'` returns no output (expected).
After GO only:
2. `python3 scripts/build_check.py`
3. `python3 scripts/run_tests.py <filter>`: suite name unknown.
4. `python3 scripts/kplog_parse.py <args>` for C172: args unknown.

## Docs
- Keep C170 open after the report. C169 closes as parked, not built.
- After GO and build, close each row with `python3 scripts/plan_manager.py mark-done <phase>`. Phase shorthand: unknown.
- CHANGELOG bump: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C103 patch_manager_mods/09_load_hang_investigation.md:308 id=- blocked
- C168 patch_manager_mods/19_shared_logging_design.md:57 id=- parked
- C169 patch_manager_mods/19_shared_logging_design.md:66 id=- parked
- C170 patch_manager_mods/19_shared_logging_design.md:92 id=- open
- C172 patch_manager_mods/19_shared_logging_design.md:119 id=4 blocked
- C173 patch_manager_mods/19_shared_logging_design.md:121 id=6 parked
