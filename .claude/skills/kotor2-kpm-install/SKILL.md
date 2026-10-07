---
name: kotor2-kpm-install
description: >-
  Commit, stage, and install a KOTOR II Patch Manager mod into the live game, plus release log tightening and
  cutting KPM PR branches. Use before any KPM install/reinstall, DLL hot-swap, staging a mod dir, preparing a mod
  for PR or final install, or creating a kpm-pr-* worktree.
---

# KPM install

## Fast path: "install what changed" (target: under 10 min, ~10 tool calls)

1. **State, one command (0.3 s, read-only):** `python3 scripts/kpm_status.py`. One verdict per repo mod plus summary lines:
   - `LIVE`: done. `NOT-LIVE`: library, retired probe or PR-only fix; skip unless the user names it.
   - `DIRTY`: commit the mod first (explicit paths). `MIRROR`: copy repo production files to `Patches/<X>/`.
   - `REBUILD`: run `create-patch.py` (step 3). `SWAP`: ready to hot-swap.
   - `REINSTALL`: kpatch hooks differ from the live `patch_config.toml`. A hot-swap is NOT allowed: stop and ask.
   - `LIVE-OTHER`: the live DLL is a different build, newer than the kpatch (gate or stage build of the same source). Leave it alone.
   Do not re-derive any of this by hand (no manual md5 loops, no transcript reading, no comparing against `.bak` kpatches).
2. **Gate once for all ids:** `scripts/pre_install_gate.sh --mod <a>_src --mod <b>_src ...` (one run; it includes `run_tests.py`, so don't run the tests separately).
3. **Build each kpatch:** with `KPATCH_ADDRESS_DATABASES=<dir of symlinks to the non-empty AddressDatabases/*.db>`, run `cd $K/Patches/<X> && python3 ../create-patch.py` (back up the old kpatch as `.<tag>_pre.bak` first). Commit KPM by explicit paths.
4. **Re-run `kpm_status.py`:** the ids must now read `SWAP`. Get the user's GO, with the game closed.
5. **Swap in one loop:** one backup dir for the batch (`$G/backups/<tag>_swap_<ts>/`, holding the old DLLs, `patch_config.toml` and `kpm_install_state.json`), then `unzip -p <kpatch> binaries/windows_x86.dll > $G/<dll>` per id.
6. **Verify:** `kpm_status.py` shows each id as `LIVE`, and the md5 of `patch_config.toml` is unchanged. Report old → new md5, the backup dir and the commits in one table.

Never run two installers at once. If another agent (Gemini/Antigravity, a peer session) is installing, `kpm_status.py` md5s change between runs: stop and ask.

- `ethansm/local-mods` in `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager` mirrors what is live. Commit the mod's source there (explicit paths) **before** staging an install.
- Stage with `python3 patch_manager_mods/mods678_research/stage_and_verify_install.py stage <dir>`; it writes `<dir>/STAGE_INFO.txt` (branch + HEAD) and exits 2 on uncommitted mod sources (`--allow-dirty` to override). Note the `head=` sha in the mod's doc.
- PR branches are cut from `master` in a separate worktree: `git -C $K worktree add ../kpm-pr-<name> -b <branch> master`.

## DLL Hot-Swap (Primary active dev path)

Eligible when hooks, exports, and manifests are identical to live (imports unchanged, `patch_config.toml` unchanged). Avoids running `KPatchLauncher`, which uninstalls all patches and would revert uncommitted peer DLLs.
Use the automated CLI helper:
```bash
python3 scripts/kpm_hotswap.py --mod <mod> --apply
```
Or check compatibility first:
```bash
python3 scripts/kpm_hotswap.py --mod <mod> --check
```
Rollback anytime:
```bash
python3 scripts/kpm_hotswap.py --rollback latest
```

## Full Reinstall (KPatchLauncher)

Use only when hooks change, new patches are added, or `patch_config.toml` must be regenerated:
```bash
KPATCH_NO_LAUNCH=1 "$HOME/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager/bin/NoLaunch/KPatchLauncher" "$G/swkotor2.exe" --patches <dir> <all live ids>
```
(game closed; doc 10 "Standing rules").

## Release log tightening (before PR or final install)

Gate per-frame or high-volume logs behind `Verbose=0` in INI, enforce deduplication (log state changes, not per-frame redraws), drop probe detours, and verify clean logs (<50 lines on startup) via `python3 scripts/kotor2_logs.py check`.
