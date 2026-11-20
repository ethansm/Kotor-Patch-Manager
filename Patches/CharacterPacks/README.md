# Character Packs (K1 port .k2pack loader)

> **STATUS** Track: EXPERIMENTAL. Version 0.1.1-dev (matches manifest.toml).
> **What it does / explores:** Asset-free, DLL-only loader that reads user-built `.k2pack` files from `<game dir>/k1_imports/` at KPatchInit and registers them with asset-vfs (`k2.assetvfs@1`) and the character library (`k2.characterlib@1`), so a Knights of the Old Republic (K1) character can be served in KOTOR II without Override files.
> **Requires:** asset-vfs (manifest `requires`), the Steam/Aspyr exe with LAA (supported_versions `kotor2_steam_aspyr`, sha 6A522E71...BBFFEF), KotorPatcher with the patch registry (KPatchInit export), and Kotor Patch Manager (KPM). Packs come from the local importer, which needs the user's own K1 install.
> **Known limits:** Win32-only loader, not yet exercised on a clean machine. Play rows VFS-B5 and MD-RB1 are pending in the project play-test queue, so the packs are unverified in play. Overlaps `juhani_char_src`: both register the `k1p_carth*` / `k1p_juhani*` keys, `conflicts` is empty, and the later registrant wins. Pools are shared with every asset-vfs client (448 assets, 1024 2DA cells, 64 characters).
> **Install:** Experimental. Do not install for normal play. Install only if you are developing the K1 character import. Build packs with the importer, then install through KPM.

## Features
- Validates each `.k2pack` (format version 1) before registering it. A bad or oversized pack is skipped alone and logged; other packs still load.
- Checks the shared asset-vfs pools up front (asset count, 2DA cell count, character count).
- Contains no game assets and no hooks (`kotor2-steam-aspyr.hooks.toml` is metadata only).

## Workflow (.k2pack)
1. Build packs on your own machine from your own K1 install with the project workspace build` (see `k1_imports.example.toml` for the private config).
2. Install the packs with `k1_port_import.py install`, which writes `<game dir>/k1_imports/*.k2pack`.
3. Install this mod and asset-vfs through KPM, then start the game.
4. Pack format: the project workspace. Inspect a pack with `python3 the project workspace inspect <file>`.

## Configuration
None. Without a `k1_imports` directory the mod logs one line and does nothing.

## Logging
Log file: `character-packs_log.txt` (KpLog.h, channel main). Lines to look for:
- `no k1_imports dir: nothing to load`
- `REGISTER ok assets=<n>/<max> cells=<n>/<max> character=<n>`
- `LOAD done packs=<n> ok=<n> skipped=<n> elapsed_ms=<ms>`

## Hooks
None. The DLL owns no detour and has no addresses to verify.

## Build and test
- Host tests: `python3 scripts/run_tests.py character_packs` (scope the filter; a bare `run_tests.py` runs all suites).
- `test.toml` uses `-I${KPM_COMMON_DIR}` (default `<KPM_DIR>/Patches/Common`).

## What was learned (EXPERIMENTAL)
- Pool limits: the shared static asset pool was 192 entries until asset-vfs 0.1.3-dev; it is now 448 (`kMaxAssets` in `PackFormat.h`). Pool sizes are shared with all asset-vfs clients, so limits belong in the shared headers.
- Load order: packs must load synchronously in KPatchInit, before the engine reads `appearance/heads.2da`, or the 2DA cells are missing.
- Duplicate (flags, res_type, resref) entries and blob overlap are rejected by the parser.

## Research links
- the project workspace (format spec)
- the project workspace (builder and installer)
- the project workspace (serving layer, asset-vfs)
- the project workspace (overlapping character provider)

## Licensing hygiene
This repository contains no game assets and no `.k2pack` files. Packs are built locally from the user's own K1 install and stay out of the repo.

## Changelog
See `PR_READINESS.md` for review status and open items. A CHANGELOG is not kept for this experimental track.
