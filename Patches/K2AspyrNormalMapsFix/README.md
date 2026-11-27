# K2 Aspyr Normal Map Fix

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Adds the Pre-Aspyr combined env+bump+tangent mode-2 gate to Aspyr GetRenderPath before the other modes

## Details

- Patch id: `k2-aspyr-normal-map-fix`, version 0.1.0
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)

## Hooks

- `kotor2-gog-aspyr.hooks.toml`: 1 hook(s) (1 replace), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr.hooks.toml`: 1 hook(s) (1 replace), for KotOR 2 Steam Aspyr (Windows)

Other patches in this repo hook the same address on the same build, so check before enabling them together:

- `K2AspyrShaderFixes`: `0x0088DD1E` (KotOR 2 GOG Aspyr (Windows)), `0x004A715E` (KotOR 2 Steam Aspyr (Windows))

## Files

- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
