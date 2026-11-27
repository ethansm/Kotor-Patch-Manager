# K2 Aspyr Map Aspect Fix

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Fixes horizontally stretched map and minimap.

## Details

- Patch id: `k2-aspyr-map-aspect-fix`, version 1.2.0
- Author: J
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)

## Hooks

- `kotor2-gog-aspyr.hooks.toml`: 3 hook(s) (3 detour), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr.hooks.toml`: 3 hook(s) (3 detour), for KotOR 2 Steam Aspyr (Windows)

## Files

- `K2AspyrMapAspectFix.cpp`: source
- `exports.def`: DLL export list
- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
