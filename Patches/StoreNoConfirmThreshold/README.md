# Stores: No Confirmation Prompts

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Removes store confirmation popups for expensive item buying and selling (K1 & K2 compatible).

## Details

- Patch id: `store-confirm-no-threshold`, version 0.1.0
- Author: Vriff
- Requires: none
- Conflicts (declared): `store-confirm-threshold-2500`

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)
- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)
- KotOR 2 Steam Aspyr (macOS) (`kotor2_steam_aspyr_macos`)
- KotOR 1 Steam Aspyr (macOS) (`kotor1_steam_aspyr_macos`)

## Hooks

- `kotor1-steam-aspyr-macos.hooks.toml`: 2 hook(s) (2 simple), for KotOR 1 Steam Aspyr (macOS)
- `kotor1.hooks.toml`: 2 hook(s) (2 simple), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03, KotOR 1 Steam 1.03
- `kotor2-gog-aspyr.hooks.toml`: 2 hook(s) (2 simple), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr-linux.hooks.toml`: 2 hook(s) (2 simple), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr-macos.hooks.toml`: 2 hook(s) (2 simple), for KotOR 2 Steam Aspyr (macOS)
- `kotor2-steam-aspyr.hooks.toml`: 2 hook(s) (2 simple), for KotOR 2 Steam Aspyr (Windows)

## Files

- `kotor1-steam-aspyr-macos.hooks.toml`: hook definitions
- `kotor1.hooks.toml`: hook definitions
- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-macos.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
