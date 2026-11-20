# In-Memory Asset VFS & Character Library

> **STATUS** Track: EXPERIMENTAL. Version 0.1.4-dev (matches manifest.toml).
> **What it does / explores:** Resolves game resources from memory instead of Override files: a GetKeyEntry-miss and ServiceFromImage hook serve assets from an in-memory pool, load-time 2DA row splicing adds rows to existing tables, and memory-backed Miles audio streams play assets with no file on disk. Exposes `k2.assetvfs@1` and `k2.characterlib@1`. Explores whether one DLL can fan out character assets with no Override files.
> **Requires:** Steam/Aspyr KOTOR II (target hash in manifest.toml `[patch.supported_versions]`). A consumer is required: `character_packs_src` (and `juhani_char_src`, which also depends on `k2.assetvfs@1`). Installed through KPM.
> **Known limits:** 2DA splice cap is 1024 cells (`kMax2DaCells`, AssetVfsCore.h); static asset pool is 448 (Registry `kMaxAssets` 512, 64 dynamic reserve). Doc 44 does not yet record these caps. Engine hook call sites are verified in game only; host tests cover the pure helpers. `k2.npcworld@1` is optional. **Play-test status:** rows VFS-T0a, VFS-CT2, VFS-B5, VFS-T0, VFS-T1 are pending in `patch_manager_mods/PLAY_TEST_QUEUE.md`.
> **Install:** Do not install for normal play. It does nothing alone (no consumer); install only with `character-packs` for the experiment. Config defaults to `mode=observe`, which logs what it would do and changes nothing.

## Features

- **Asset resolution** (`VFS_OnGetKeyEntryMiss`, `VFS_OnServiceFromImage`): serves in-memory resources when the engine misses them.
- **2DA row splicing** (`VFS_On2DaServiced`): adds rows to a 2DA after the engine services it.
- **Memory-backed audio** (`VFS_OnOpenStream`): Miles stream callbacks read from memory.
- **Release guard** (`VFS_OnResRelease`): skips the engine's source release for served resources.

## Configuration (`asset_vfs.ini`, `[vfs]`)

- `mode = serve | observe | off`. Missing file means `observe`. `serve` makes the changes live.
- `log_prefix = ` filters logged resource names. `log_all = 0 | 1`.
- Legacy `enabled = 0` turns the mod off.

## Hooks (`kotor2-steam-aspyr.hooks.toml`, 5 hooks)

| Function | Address | Purpose |
|---|---|---|
| VFS_OnGetKeyEntryMiss | 0x0071244E | GetKeyEntry miss: serve from memory |
| VFS_OnServiceFromImage | 0x00713E83 | ServiceFromImage: serve from memory |
| VFS_On2DaServiced | 0x00729FD0 | 2DA row splice after service |
| VFS_OnOpenStream | 0x0070CB76 | Miles memory-backed audio stream |
| VFS_OnResRelease | 0x007130D3 | CExoResMan::Release source-release guard (H5) |

## Build/test

`python3 scripts/run_tests.py asset_vfs`. Shared headers come from `_shared_src` and `${KPM_COMMON_DIR}`. `common_contract_test` also uses a parity stub under `research/companion_levelup/tools/parity/stub` (tracked in this repo).

## What was learned (EXPERIMENTAL)

- **H5 Release guard:** without `VFS_OnResRelease`, a served resource (packedId 0x40FC...) takes the `>>30 == 1` branch, finds no archive source, returns NULL, and faults at 0x007130E7. Found on the first live serve (10-18).
- **2DA cap:** the cell cap went from 256 to 1024 with a WARN when cells are dropped. A third character bundle got 47 of 102 cells and lost its heads row (VFS-CT2).
- **Asset pool:** the static pool went from 192 to 448 (0098d66b). Registry `kMaxAssets` is 512 with a 64-slot dynamic reserve.

## Changelog pointer

Version history is in the commit log (`git log -- patch_manager_mods/asset_vfs_src`). Research notes: `patch_manager_mods/44_in_memory_asset_vfs_and_character_library.md` (doc 44).
