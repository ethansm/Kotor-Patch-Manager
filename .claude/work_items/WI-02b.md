# WI-02b KPM tooling H7, H10, H9 new scripts
area: kpm-tooling
size: M
ready: ready
items: C007,C009,C008
files: NEW:scripts/kpm_commit.py; NEW:scripts/kpm_reinstall.py; NEW:scripts/kpm_ship.py
out-of-repo: none
blockers: Needs WI-02 (H2, for C009) and WI-02c (H8, for C008) first. No shared files with those WIs. All three NEW paths are missing today (checked 10-12).
docs: patch_manager_mods/24_master_plan_status.md:95 (H7), :98 (H10), :97 (H9); patch_manager_mods/36_agent_tooling_hardening.md (doc 36)
deferred: none

## Goal
Create H7, H10, H9 in that order. H9 runs last because it orchestrates the other tools.

## Steps
1. H7 (C007): create `scripts/kpm_commit.py`: dirty-tree auto-committer with scoped tests and conventional commits.
2. H10 (C009): create `scripts/kpm_reinstall.py --mod <id>`: wraps KPatchLauncher with peer DLL preservation. Needs H2 from WI-02.
3. H9 (C008): create `scripts/kpm_ship.py [--apply]`: runs status, then commit (`kpm_commit.py` from step 1), gate (`pre_install_gate.py`), prepare and apply (`kpm_hotswap.py`). Build only after H8 (WI-02c) is done.

## Verify
1. `python3 scripts/build_check.py`
2. `python3 scripts/run_tests.py <filter>` (suite name unknown)
3. Dry run of `python3 scripts/kpm_ship.py` without `--apply`. Flags are unknown until built. Do not run `--apply` (live game).

## Docs
1. After each row verifies: `python3 scripts/plan_manager.py mark-done 24:<H-id> --state <STATE> --commit <sha>` for H7, H10, H9. STATE is unknown.
2. Never sed the master plan. CHANGELOG: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C007 patch_manager_mods/24_master_plan_status.md:95 id=H7 open
- C009 patch_manager_mods/24_master_plan_status.md:98 id=H10 open
- C008 patch_manager_mods/24_master_plan_status.md:97 id=H9 open
