---
name: kotor2-kpm-install
description: >-
  Commit, stage, and install a KOTOR II Patch Manager mod into the live game, plus release log tightening and
  cutting KPM PR branches. Use before any KPM install/reinstall, DLL hot-swap, staging a mod dir, preparing a mod
  for PR or final install, or creating a kpm-pr-* worktree.
---

# KPM install

Full reference (paths, verdict table, build, rollback, troubleshooting, Gemini prompt): `patch_manager_mods/BUILD_AND_INSTALL.md`.

## Fast path: "install what changed" (about 5-8 min for a 4-mod batch)

1. **State (read-only, under 1 s):** `python3 scripts/kpm_status.py`. One verdict per repo mod: `LIVE` done; `DIRTY` commit first (explicit paths); `MIRROR`/`REBUILD` need `--prepare`; `SWAP` ready; `REINSTALL` hooks changed, no hot-swap, ask the user; `NOT-LIVE`/`LIVE-OTHER` leave alone unless the user names them. Never re-derive this by hand (no md5 loops, no `.bak` comparisons).
2. **Gate once for all ids:** `scripts/pre_install_gate.sh --mod <a>_src --mod <b>_src ...` (includes `run_tests.py`; wait for `GATE PASS`).
3. **Prepare:** `python3 scripts/kpm_hotswap.py --mod <id>...` (dry run), then `... --prepare` (mirror to KPM, rebuild with create-patch.py, prints the commit commands). Commit repo and KPM with explicit paths: `git commit -- <paths>`. `kpm_status.py` must now show `SWAP`.
4. **GO, then apply:** ask the user for GO with the game closed, then `python3 scripts/kpm_hotswap.py --mod <id>... --apply` (one backup dir, atomic swap).
5. **Verify:** `kpm_status.py` shows `LIVE`; `python3 scripts/kotor2_logs.py check <log> --new` after play. Rollback: `kpm_hotswap.py --rollback latest` (`--list-backups` to browse).

Rules: never run two installers at once (re-run `kpm_status.py` before and after; changing md5s mean a peer is installing, so stop and ask). Never KPatchLauncher for a hot-swappable change (it reverts peer DLLs); full reinstall only for hooks, exports/imports, new ids or a regenerated `patch_config.toml` (see the doc, section 4); automated: `python3 scripts/kpm_reinstall.py --mod <id> [--dry-run]` (needs the user's GO, game closed, rollback built in). Never run `check_shared_headers.py --sync` inside an install step.

## KPM repo, staging, PR branches

- `ethansm/local-mods` in `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager` mirrors what is live. Commit the mod's source there (explicit paths) **before** staging an install.
- Stage with `python3 patch_manager_mods/mods678_research/stage_and_verify_install.py stage <dir>`; it writes `<dir>/STAGE_INFO.txt` (branch + HEAD) and exits 2 on uncommitted mod sources (`--allow-dirty` to override). Note the `head=` sha in the mod's doc.
- PR branches are cut from `master` in a separate worktree: `git -C $K worktree add ../kpm-pr-<name> -b <branch> master`.

## Release log tightening (before PR or final install)

Gate per-frame or high-volume logs behind `Verbose=0` in INI, enforce deduplication (log state changes, not per-frame redraws), drop probe detours, and verify clean logs (<50 lines on startup) via `python3 scripts/kotor2_logs.py check`.
