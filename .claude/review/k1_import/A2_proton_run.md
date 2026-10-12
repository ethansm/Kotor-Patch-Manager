# A2: Headless Proton run pattern (Proton - Experimental)

## Precedent
- scripts/upscale_legacy_movies.py:65-67 sets STEAM_COMPAT_CLIENT_INSTALL_PATH and STEAM_COMPAT_DATA_PATH, then runs `proton run <args>`
- scripts/upscale_legacy_movies.py:70-71 converts paths to Wine form `Z:\...`
- patch_manager_mods/05_build_plan.md:94 ran `proton run <exe>` on the REAL compatdata (do not repeat for A2)

## Proton dir and verbs
- Exists. Top-level: contrib, dist.lock, filelock.py, files, LICENSE, LICENSE.OFL, PATENTS.AV1, proton, proton_3.7_tracked_files, __pycache__, steampipe_fixups.json, steampipe_fixups.py, toolmanifest.vdf, user_settings.sample.py, version
- Verbs (proton:2209-2232): run, waitforexitandrun, runinprefix, destroyprefix, getcompatpath, getnativepath. No usage text. No verb prints "Need a verb." and exits 1.
- Missing STEAM_COMPAT_DATA_PATH prints "No compat data path?" and exits 1 (proton:2187).

## Prefix init
- Prefix is $STEAM_COMPAT_DATA_PATH/pfx/ (proton:635). copy_pfx fills it from files/share/default_pfx/ (proton:868). No manual wineboot needed. The data dir must be new or empty.

## Command template
```bash
PROTON="/home/ethansm/.local/share/Steam/steamapps/common/Proton - Experimental/proton"
PFX="/tmp/claude-1000/-home-ethansm-orca-workspaces-local-setup-KOTOR-II/2c3f74c1-88fa-49a4-8101-3e63d340c7c0/scratchpad/A2/prefix"
mkdir -p "$PFX"   # must be new or empty
STEAM_COMPAT_CLIENT_INSTALL_PATH=/home/ethansm/.local/share/Steam \
STEAM_COMPAT_DATA_PATH="$PFX" STEAM_COMPAT_APP_ID=208580 \
timeout 600 "$PROTON" run 'Z:\abs\path\app.exe' [args]
```
- STEAM_COMPAT_CLIENT_INSTALL_PATH must be set: proton:1022 and :1279 read it with os.environ[] (KeyError if unset).
- STEAM_COMPAT_APP_ID is optional (proton:1466).

## Traps
1. Out-of-place exe launches for AppID 208580 hit a Steamworks DRM check (05_build_plan.md:106-118). A fresh prefix with no Steam running will likely crash before the menu.
2. Never point STEAM_COMPAT_DATA_PATH at compatdata/208580. That holds live saves.
3. `proton run` blocks until the process exits. Keep the `timeout`.
