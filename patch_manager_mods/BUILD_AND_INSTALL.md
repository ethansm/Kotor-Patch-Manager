# Build and install: KPM mods into the live KOTOR II

Reference for building a repo mod, packing its kpatch, and getting it into the live Steam game. Tools: `scripts/kpm_status.py` (read-only state), `scripts/pre_install_gate.sh` (build + tests), `scripts/kpm_hotswap.py` (mirror, rebuild, swap, rollback). Skill router: `.claude/skills/kotor2-kpm-install/SKILL.md`.

## 1. Paths, environment, prerequisites

| Name | Value | Notes |
|---|---|---|
| `G` (game dir) | `~/.steam/steam/steamapps/common/Knights of the Old Republic II` | Override with `KOTOR2_ROOT` (kpm_hotswap.py, pre_install_gate.py) or `--game-dir` (kpm_hotswap.py). |
| `K` (KPM checkout) | `~/Games/KOTOR2_Mod_Workspace/tools/kotor-patch-manager` | Branch `ethansm/local-mods`. Override with `KPM_DIR` or `--kpm-dir`. Mod sources are mirrored to `$K/Patches/<X>/`; the built kpatch lives there too. |
| Live DLL path | `patch_config.toml` entry `dll = "patches/<id>.dll"`, relative to `$G` | Never guess it. kpm_hotswap.py reads it from the config. |
| Backups | `$G/backups/<tag>_swap_<ts>/` | Written by `--apply`. |
| Toolchain | `i686-w64-mingw32-g++-posix` or `i686-w64-mingw32-g++` (or `CXX_WIN`) | Used by build_check.py inside the gate. Without it the gate skips the build. |
| Address DBs | `KPATCH_ADDRESS_DATABASES=<dir of symlinks to the non-empty $K/AddressDatabases/*.db>` | create-patch.py fails on the 0-byte placeholder DBs. `kpm_hotswap.py --prepare` builds this symlink dir itself; set it by hand only when running create-patch.py directly. |
| Steam hooks file | `kotor2-steam-aspyr.hooks.toml` (selected by `kpm_status.steam_hooks_name`) | The live game is Steam/Windows. Never use the GOG, linux or macos hooks file of a multi-target kpatch. |

Python for the gate is `.venv/bin/python` (the .sh wrapper picks it). kpm_status.py needs only the system python3.

## 2. State model

`python3 scripts/kpm_status.py` prints one line per repo mod (`patch_manager_mods/*_src` with a manifest): verdict, id, version, live md5 / kpatch md5, detail. It is read-only and takes under a second. Options: `--game DIR --kpm DIR --json --only id,id`. It ends with summary lines (`SWAP: ...`, `DIRTY: ...`) naming the ids per verdict, and `game_running=yes|no`. First matching verdict wins.

| Verdict | Meaning | Action |
|---|---|---|
| LIVE | Live DLL md5 equals the kpatch DLL md5. | Nothing. |
| NOT-LIVE | Id is not in the live `patch_config.toml` (library, retired probe, PR-only fix). | Skip unless the user names it; installing it is a full reinstall. |
| DIRTY | Uncommitted production source in the repo mod dir (tests and docs ignored). | Commit the mod first: `git commit -- <paths>`. |
| MIRROR | Repo production files differ from `$K/Patches/<X>/`. | `kpm_hotswap.py --mod <id> --prepare` copies them and rebuilds. |
| REBUILD | Kpatch is older than its KPM code, or its manifest/hooks differ from the KPM files. | `kpm_hotswap.py --mod <id> --prepare` (runs create-patch.py). |
| SWAP | Kpatch is fresh, its hooks match live, DLL differs. | Ready for `--apply` after the user's GO. |
| REINSTALL | Kpatch hooks differ from the live `patch_config.toml` hooks for the id. | Hot-swap refused. Full reinstall (section 4), after asking the user. |
| LIVE-OTHER | Live DLL differs but is newer than the kpatch (a gate or stage build of the same source). | Leave alone. Swap only on purpose, after rebuilding the kpatch. |

Intentional states today (do not "fix"): NOT-LIVE for `k2-wrap-measure-fix`, `load-hang-probe`, `save-events`; LIVE-OTHER for `mouse-probe`, `portrait-slots`. `kpm_hotswap.py` refuses a batch containing NOT-LIVE, DIRTY, REINSTALL or LIVE-OTHER mods before writing anything; LIVE mods are skipped; MIRROR and REBUILD need `--prepare`.

## 2b. Versioning (pre-1.0)

- Format: `0.MINOR.PATCH`, plus an existing suffix while in flight (`-dev`, `-probe`, `-rc`).
- Stay below `1.0.0` until the PR into `LaneDibello/Kotor-Patch-Manager` is opened. `1.0.0` means upstream-ready.
- Bump MINOR (`0.x.0`) for big jumps or new features. Bump PATCH (`0.x.y`) for fixes and small changes.
- One-time migration (2026-10-11): every pre-1.0 `X.Y.Z` was mapped to `0.X.Y` (drop Z), e.g. `1.4.1` → `0.1.4`, `2.0.1` → `0.2.0`. Suffixes kept. The dialogue-letterbox probe folder is `0.1.0-probe2`, so it does not clash with the shipping probe build's `0.1.0-probe`. `kpm_status` picks one dir per id: the one matching the KPM copy, else the newest non-`-probe` dir, and `-probe` dirs only when no release dir exists.
- Source of truth: `version` in `patch_manager_mods/<mod>_src/manifest.toml`. `kpm_status.py` shows it per id.
- Before install, the version must match the expected version string in the mod's log (section 7).

## 3. Build

| Step | Command | Notes |
|---|---|---|
| Gate | `scripts/pre_install_gate.sh --mod <a>_src --mod <b>_src ...` | One run for all ids. |
| Pack | `python3 $K/Patches/create-patch.py` from `$K/Patches/<X>` | Normally done by `kpm_hotswap.py --prepare`. |

The gate stops at the first failure and ends with `GATE PASS mods=<ids> smokes=<rows> head=<sha12>` plus the md5 of every rebuilt DLL, or `GATE FAIL <n> <step>: <reason>`. Steps: (1) `run_tests.py` default tier, so do not run the tests separately; (2) build_check.py builds a fresh DLL from source; (3) the fresh DLL is swapped into a scratch copy of the live kpatch; (4) the smoke rows for that mod run; (5) report. Read-only towards `$K` and `$G`; everything is written under `$TMPDIR/kotor-tests/gate/`. Step 4 runs only in a scratch tree, never against the live game. A mod with no `smokes.toml` row has no Wine smoke at all: the gate prints NOTE and still passes, so GATE PASS proves build plus tests, not in-game behaviour.

`create-patch.py` packs all `*hooks.toml` files, all target binaries and all extras into the kpatch, so multi-target kpatches stay complete. The gate-built DLL md5 and the kpatch DLL md5 differ (different build, different timestamps); the kpatch is the shipped authority. Never install a gate-built DLL; `kpm_hotswap.py` always swaps the DLL extracted from the kpatch (`binaries/windows_x86.dll`).

## 4. Hot-swap vs full reinstall

| Change | Path |
|---|---|
| Only DLL code changed (same hooks, same exports, no new imports, same `patch_config.toml`) | Hot-swap (`kpm_hotswap.py`). |
| Hooks added, removed or changed (verdict REINSTALL) | Full reinstall. |
| DLL exports changed or new DLL imports | Full reinstall (`--apply` refuses these on PE checks). |
| New patch id not in the live config (NOT-LIVE) | Full reinstall. |
| `patch_config.toml` must be regenerated | Full reinstall. |

Full reinstall, game closed:

```
KPATCH_NO_LAUNCH=1 "$K/bin/NoLaunch/KPatchLauncher" "$G/swkotor2.exe" --patches <dir> <all live ids>
```

Risks: it uninstalls and reinstalls every patch, so it reverts peer DLLs that were hot-swapped but not committed to a kpatch, and it rewrites the hand-edited `patch_config.toml`. Pass every live id, not only the changed one. Take a backup of the DLLs, `patch_config.toml` and `kpm_install_state.json` first (a manual `$G/backups/<tag>_full_<ts>/`), and run `kpm_status.py` afterwards.
Automated alternative (doc 36 Phase H10): `python3 scripts/kpm_reinstall.py --mod <id>` automates this workflow while preserving peer DLLs.

## 5. Step-by-step flow (hot-swap)

Prepare:

1. `python3 scripts/kpm_status.py`. Note the verdict per id you want to ship.
2. Commit the repo mod with explicit paths: `git commit -m "<msg>" -- patch_manager_mods/<x>_src/<files>`. DIRTY must clear.
3. Pre-mirror build check: `scripts/pre_install_gate.sh --skip-default --mod <x>_src [--mod <y>_src ...]`. Verifies fresh compilation and unit tests without tripping on stale KPM kpatch exports.
4. `python3 scripts/kpm_hotswap.py --mod <id> [<id> ...] --prepare`. Mirrors production files, auto-creates canonical KPM folders, backs up old kpatches, and rebuilds. If hooks changed, its post-prepare verdict is `REINSTALL` (requires launcher reinstall); pure DLL changes become `SWAP`.
5. Commit KPM with explicit paths: `git -C "$K" commit -m "<msg>" -- Patches/<X>/<files>` (use the exact commit command that `--prepare` printed; kpatch and `.bak` files are not committed).

Gate & Verify:

1. **Verify After Fix Gate (MANDATORY)**: `scripts/pre_install_gate.sh --mod <x>_src [--mod <y>_src ...]`. MUST report `GATE PASS` across all suites (including check-exports and Wine smokes) against the freshly prepared kpatches before proceeding. Never dismiss a failure as "pre-existing" or "expected" without explicit waiver.
2. `python3 scripts/kpm_status.py`. Each id targeted for hot-swap must read `SWAP`.
3. If the gate fails, fix the cause and re-run the failing step right away (stale smoke pins such as a version banner are real failures).

Install:

1. Ask the user for GO. The game must be closed (`game_running=no`).
2. `python3 scripts/kpm_hotswap.py --mod <id> [<id> ...] --apply`. It refuses unless every verdict allows the swap, writes one backup dir, swaps atomically (a failure restores the DLLs already swapped), and prints the old and new md5 per mod and the rollback command.
3. `python3 scripts/kpm_status.py`. Each swapped id must read `LIVE`, and `patch_config.toml` is unchanged.

## 5b. Automated single-command flow (`scripts/kpm_ship.py` — doc 36 Phase H9)

To eliminate the 7-step manual dance and nondeterministic agent round-trips:
```bash
python3 scripts/kpm_ship.py [--mod <id> ...] [--apply]
```
Orchestrates deterministically:
1. Production dirty auto-committer (`kpm_commit.py` — Phase H7) with scoped unit tests.
2. Pre-install gate (`pre_install_gate.sh`) across all target mods.
3. Mirror and KPM auto-commit (`kpm_hotswap.py --prepare --commit-kpm` — Phase H6).
4. Atomic hot-swap (`--apply`) with rollback snapshot if game is closed.


Verify: section 7.

## 6. Backup, verify, rollback

Backup dir `$G/backups/<tag>_swap_<ts>/` (tag is the mod id for one mod, `batch` for several) holds:

| File | Purpose |
|---|---|
| `<id>.dll` | The old live DLL of each swapped mod. |
| `patch_config.toml`, `kpm_install_state.json` | Snapshot of the config files at swap time. |
| `swap_manifest.json` | Timestamps, repo and KPM heads, config md5s, per mod: live DLL path, old md5, new md5. |

Verify: `kpm_status.py` shows LIVE (live md5 equals kpatch md5). Compare the printed old and new md5 against the backup's `swap_manifest.json` if in doubt.

| Command | Effect |
|---|---|
| `python3 scripts/kpm_hotswap.py --list-backups` | Lists `*_swap_*` backups, newest first. |
| `python3 scripts/kpm_hotswap.py --rollback latest` | Restores the DLLs of the newest backup. |
| `python3 scripts/kpm_hotswap.py --rollback <backup dir>` | Same for a named backup. |

Rollback restores DLLs only. It restores `patch_config.toml` and `kpm_install_state.json` only when the current config md5 equals the md5 recorded in the snapshot; otherwise it exits 0 with a WARNING and leaves the config alone (for example after a full reinstall regenerated it). Read the WARNING; do not copy the snapshot config over by hand. Old backups made before the manifest existed show `(no manifest: old-style)` in `--list-backups` and `--rollback` refuses them: copy the `.dll` files from that directory back by hand, to the live path in `patch_config.toml`.

## 7. Post-install verification

1. `python3 scripts/kpm_status.py`: ids read LIVE.
2. The user launches the game and plays or loads a save.
3. `python3 scripts/kotor2_logs.py status`: each swapped mod has a fresh `<mod>_log.txt` (new size and age, a new `# kplog session <date>` header, launches counted).
4. `python3 scripts/kotor2_logs.py check "$G/<mod>_log.txt" --new`: only lines since the last check; no errors, expected version string in the first lines.
5. Never `cat`, `tail` or `grep` the raw `.txt` logs (see the `kotor2-log-analysis` skill).

## 8. Concurrency rules

- One installer at a time. Peer sessions, Gemini and Antigravity (Agy) share the same game dir and the same KPM checkout. If `kpm_status.py` md5s change between two runs without your action, stop and ask.
- Run `kpm_status.py` immediately before and after every step that writes.
- Never touch the DLLs of mods you were not asked to ship. `kpm_hotswap.py` swaps only the `--mod` ids.
- Commit with explicit paths only (`git commit -- <paths>`); the worktree has many dirty paths from other sessions. The scripts never commit.
- Never run `check_shared_headers.py --sync` as part of a mirror or install step: it rewrites `Patches/Common` for all mods. Run it by hand when intended and commit it separately.

## 9. Troubleshooting (anti-patterns)

| Anti-pattern | Symptom | Fix |
|---|---|---|
| (a) Comparing hooks against `.bak` kpatches | False REINSTALL or false "unchanged"; confusion over which hooks are live. | Compare the kpatch's Steam hooks file with the mod's entry in the live `patch_config.toml`. `kpm_status.py` already does this. |
| (b) Taking the first `*hooks.toml` of a kpatch | For multi-target mods the first file is the GOG one; hooks compare wrong. | Use `kpm_status.steam_hooks_name` (the `steam` file, never gog/linux/macos). |
| (c) Hand-repacking a kpatch | Hooks or extras of the other targets disappear; the game or installer breaks. | Rebuild with `create-patch.py`. If a DLL must be replaced inside an existing kpatch, copy the zip member by member and replace only `binaries/windows_x86.dll` (reference: `swap_dll` in `scripts/pre_install_gate.py`). |
| (d) Reusing a shared gate-build DLL without a source hash | Installing a DLL built from different source than what was committed. | Always swap the DLL from the freshly built kpatch; never reuse a scratch DLL. |
| (e) Copying `.md`, `.ini` or test files into KPM | Dirty KPM, spurious MIRROR/REBUILD verdicts. | Only production files go to KPM (`kpm_status.is_production`: `.cpp .h .inc .def`, `manifest.toml`, `*.hooks.toml`; not `*_test.cpp`, `smoke_*`, `test.toml`). `--prepare` does this mirroring. |
| (f) Rollback restoring `patch_config.toml` blindly after a full reinstall | Config from before the reinstall overwrites the regenerated one. | The script guards this by md5 and prints a WARNING instead. Do not override by hand. |
| (g) KPatchLauncher for a hot-swappable change | Peer DLLs reverted, config rewritten, other sessions' work lost. | Use `kpm_hotswap.py`. KPatchLauncher only for section 4's full-reinstall cases. |
| (h) Guessing the live DLL path | Wrong file replaced or backed up. | Read `dll = "patches/<id>.dll"` from the live `patch_config.toml`; the script does this. |
| (i) Hand md5 loops, or running `run_tests.py` before the gate | Wasted minutes, stale conclusions. | `kpm_status.py` has the md5s; the gate already runs the tests. |

## 10. Time budget

A typical 4-mod batch takes about 5 to 8 minutes end to end: status and commits 1 minute, gate 2 to 4 minutes, `--prepare` under 1 minute, `--apply` and verify under 1 minute, plus the user's GO.

## 11. Prompt for Gemini / Antigravity

```
KPM install task. Read patch_manager_mods/BUILD_AND_INSTALL.md first.
1. Run: python3 scripts/kpm_status.py  (one verdict per mod; do not re-derive state by hand).
2. Build: scripts/pre_install_gate.sh --mod <x>_src ... (it already runs the tests).
3. Dry run, then prepare: python3 scripts/kpm_hotswap.py --mod <ids>  then  ... --prepare
4. Commit with explicit paths only: git commit -m "<msg>" -- <paths> (repo, and git -C $K for KPM).
5. Re-run kpm_status.py: ids must read SWAP.
6. Ask me for GO before --apply (game must be closed). Then: python3 scripts/kpm_hotswap.py --mod <ids> --apply
7. Verify with kpm_status.py (LIVE) and python3 scripts/kotor2_logs.py check <log> --new.
Hard rules:
- Never run KPatchLauncher for a change that can be hot-swapped.
- Never run two installers at once; re-run kpm_status.py before and after each write.
- Never run check_shared_headers.py --sync.
- Never commit through the script or with a bare git commit; explicit paths only.
- Never touch DLLs of mods I did not name; never hand-repack a kpatch (use create-patch.py).
- Never cat/tail/grep raw .txt logs.
- Rollback: python3 scripts/kpm_hotswap.py --rollback latest
```
