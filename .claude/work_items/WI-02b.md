# WI-02b KPM tooling H7, H10, H9 new scripts
area: kpm-tooling
size: M
ready: ready
after: WI-02, WI-02c
items: C007,C009,C008
files: NEW:scripts/kpm_commit.py; NEW:scripts/kpm_reinstall.py; NEW:scripts/kpm_ship.py
out-of-repo: none
blockers: Needs WI-02 (H2, for C009) and WI-02c (H8, for C008) first. No shared files with those WIs. All three NEW paths are missing today (checked 10-12).
docs: patch_manager_mods/24_master_plan_status.md:95 (H7), :98 (H10), :97 (H9); patch_manager_mods/36_agent_tooling_hardening.md (doc 36)
deferred: none
verify: python3 scripts/kpm_commit.py --help
verify: python3 scripts/kpm_reinstall.py --help
verify: python3 scripts/kpm_ship.py --help

## Goal
Create H7, H10, H9 in that order. H9 runs last because it orchestrates the other tools.

## Steps
All three scripts already exist and are committed (checked 10-15): no create step remains, so the code diff is expected to be empty. Do not recreate or edit them.
1. H7 (C007): `scripts/kpm_commit.py` already satisfied, no edit (main at :68, commit 49978eb). Confirm `python3 scripts/kpm_commit.py --help` exits 0.
2. H10 (C009): `scripts/kpm_reinstall.py` already satisfied, no edit (main at :271, commits 5673d94, 6af1de1). Confirm `--help` exits 0.
3. H9 (C008): `scripts/kpm_ship.py` already satisfied, no edit (main at :85, `--apply` at :89, `--dry-run` at :91, commit 92bcb56). Confirm `--help` and `--dry-run` exit 0.
4. Only remaining work: close plan rows H7, H10, H9 in the docs step (rows are still open in patch_manager_mods/24_master_plan_status.md:95/:98/:97).

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
