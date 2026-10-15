# WI-02c Smoke harness include normalization (H8)
area: kpm-tooling
size: S
ready: ready
items: H8 (no C-id in bundle)
files: scripts/run_tests.py
out-of-repo: none
blockers: none. WI-02b (H9) waits on this row.
docs: patch_manager_mods/24_master_plan_status.md:96 (H8); patch_manager_mods/36_agent_tooling_hardening.md (doc 36)
deferred: none
verify: python3 scripts/run_tests.py test_run_tests_select test_run_tests_coverage test_run_tests_includes

## Goal
Build H8 (plan row only; the checker had no C-id). H8 was undefined in the earlier bundle. It is defined at 24:96 (READY, no dependencies), so it is now a step here, and H9 depends on it.

## Steps
1. H8 is ALREADY SATISFIED in commit 65f9206: `scripts/run_tests.py:114` `canonical_include_flags()` and its use at `scripts/run_tests.py:702` (smoke_build) already inject the canonical Common includes; tests live in `scripts/test_run_tests_includes.py`. No edit to `scripts/run_tests.py` is needed. Confirm by `sed -n '114,120p;700,704p' scripts/run_tests.py`, run the Verify command, and reply STATUS: PASS with MODIFIED: None.

## Verify
1. `python3 scripts/run_tests.py <smoke filter>` (suite name unknown; always scope it, a bare run covers all 156 suites)
2. `python3 scripts/build_check.py`

## Docs
1. After verify: `python3 scripts/plan_manager.py mark-done 24:H8 --state <STATE> --commit <sha>`. STATE is unknown.
2. Never sed the master plan. CHANGELOG: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- H8 (no C-id) patch_manager_mods/24_master_plan_status.md:96 id=H8 open
