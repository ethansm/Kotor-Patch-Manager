# Visible Console for KOTOR2

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Restores KOTOR2's visible cheat console.

## Details

- Patch id: `k2-visible-console`, version 1.4.1
- Author: J, Synchro
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)
- KotOR 2 Steam Aspyr (macOS) (`kotor2_steam_aspyr_macos`)

## Hooks

- `kotor2-gog-aspyr.hooks.toml`: 3 hook(s) (3 detour), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr-linux.hooks.toml`: 3 hook(s) (3 detour), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr-macos.hooks.toml`: 2 hook(s) (2 detour), for KotOR 2 Steam Aspyr (macOS)
- `kotor2-steam-aspyr.hooks.toml`: 3 hook(s) (3 detour), for KotOR 2 Steam Aspyr (Windows)

## Files

- `VisibleConsole.cpp`: source
- `exports.def`: DLL export list
- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-macos.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
