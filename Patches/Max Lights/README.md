# Max Lights

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Raises KOTOR2's dynamic-light limit from 3 to 7, characters included.

## Details

- Patch id: `max-lights`, version 1.2.1
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)

## Hooks

- `kotor2-gog-aspyr.hooks.toml`: 20 hook(s) (3 detour, 17 simple), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr-linux.hooks.toml`: 28 hook(s) (1 detour, 27 simple), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr.hooks.toml`: 20 hook(s) (3 detour, 17 simple), for KotOR 2 Steam Aspyr (Windows)

## Files

- `MaxLights.cpp`: source
- `exports.def`: DLL export list
- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
