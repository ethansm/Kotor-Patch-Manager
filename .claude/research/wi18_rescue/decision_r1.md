# WI-18 rescue round 1 decision
- Cause of 2 Haiku FAILs: brief omitted out-of-repo KpLog.h; Step 1 named no API. Recon: .claude/waves/WI-18-rescue/u01..u07.md.
- Route: code plan, direct edits (no worker; scope tiny, API pattern from equip_icon_fix_src).
- InventorySortFilter.cpp: private logf -> thin forwarder to KpLog fallback sink (KP_LOG_PATCH, KP_CHANNEL main, fallback file inventory_sort_filter_log.txt at INFO). 21 call sites + 1 def unchanged (22). DllMain truncate kept.
- test.toml + common_contract_test.cpp: -I Patches/Common; pins SetFallbackFile no-I/O.
- 19e_phase2_plan.md: shared row split; new row "inventory-sort-filter (C180) | WI-18".
- Verified: build_check --mod inventory_sort_filter_src PASS; run_tests -k inventory_sort_filter_src PASS; sort_logic_test PASS.
- Not installed. Log lines gain "<ms> <id>/main " prefix; reader fix in backlog (analyze_play_logs.py). Do not install until it lands.
