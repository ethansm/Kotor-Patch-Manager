# WI-46b Agent tooling hardening: ship pipeline modules
area: agent-tooling-hardening
size: M
ready: ready
items: C264,C266,C267
files: NEW:scripts/kpm_commit.py; NEW:scripts/kpm_ship.py; NEW:scripts/kpm_reinstall.py
out-of-repo: none
blockers: after: WI-46 (H9 needs H3 and H6 from WI-46; no file overlap, order only). Reuse, no edits: `git_dirty()` (scripts/kpm_status.py:72), `is_game_running()` (scripts/kpm_hotswap.py:61), `CONFIG_FILES` (scripts/kpm_hotswap.py:45), `REINSTALL_DOC` (scripts/kpm_hotswap.py:48). Unknown: the KPatchLauncher reinstall command is not in repo (named only at scripts/kpm_status.py:14). Suite names unknown. All three files are new and do not exist yet. (after WI-46)
docs: patch_manager_mods/36_agent_tooling_hardening.md:H7,H9,H10

## Goal
Close plan rows H7, H9, H10 in patch_manager_mods/36_agent_tooling_hardening.md by creating three new scripts. H9 runs after WI-46 closes H3 and H6. No user input needed.

## Steps
1. H7 (C264): create `NEW scripts/kpm_commit.py`: production-only dirty scan reusing `git_dirty()` (`scripts/kpm_status.py:72`), per-mod tests, and topological order with `_shared_src` first.
2. H9 (C266): create `NEW scripts/kpm_ship.py` (status, commit, gate, prepare, apply; halt on gate fail; refuse while game runs via `is_game_running()`, `scripts/kpm_hotswap.py:61`). Needs step 1 and WI-46 H3+H6.
3. H10 (C267): create `NEW scripts/kpm_reinstall.py` (snapshot, KPatchLauncher reinstall, restore peer DLLs and patch_config via `CONFIG_FILES`, `scripts/kpm_hotswap.py:45`, then rollback). Reinstall guidance is `REINSTALL_DOC` (`scripts/kpm_hotswap.py:48`).

## Verify
1. `python3 scripts/kpm_commit.py --help`, `python3 scripts/kpm_ship.py --help`, `python3 scripts/kpm_reinstall.py --help` (after build: each file exists and prints usage).
2. `python3 scripts/run_tests.py <kpm filter>` (suite name unknown).
3. `python3 scripts/build_check.py`
4. `compat_check.py`: not triggered (no `_shared_src` edits).

## Docs
- After each step passes: `python3 scripts/plan_manager.py mark-done <phase> --state DONE --notes <txt>` for H7, H9, H10. The phase key form is unverified (the help takes a name or substring).
- Do not mark H8 (line 225).
- CHANGELOG: none named in TSV.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C264 patch_manager_mods/36_agent_tooling_hardening.md:197 id=H7 open
- C266 patch_manager_mods/36_agent_tooling_hardening.md:251 id=H9 blocked
- C267 patch_manager_mods/36_agent_tooling_hardening.md:279 id=H10 open
