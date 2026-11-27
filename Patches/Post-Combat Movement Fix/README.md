# Post-Combat Movement Fix

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Fixes high-FPS movement locks after combat actions, Force powers, and item use.

## Details

- Patch id: `post-combat-movement-fix-v1`, version 1.10.0
- Author: J
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)
- KotOR 1 Steam Aspyr (macOS) (`kotor1_steam_aspyr_macos`)
- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)
- KotOR 2 Steam Aspyr (macOS) (`kotor2_steam_aspyr_macos`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)

## Hooks

- `kotor1-steam-aspyr-macos.hooks.toml`: 1 hook(s) (1 replace), for KotOR 1 Steam Aspyr (macOS)
- `kotor1_103.hooks.toml`: 1 hook(s) (1 replace), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03
- `kotor2-steam-aspyr-linux.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr-macos.hooks.toml`: 1 hook(s) (1 replace), for KotOR 2 Steam Aspyr (macOS)
- `kotor2-steam-aspyr.hooks.toml`: 1 hook(s) (1 replace), for KotOR 2 Steam Aspyr (Windows)
- `kotor2_gog_aspyr.hooks.toml`: 1 hook(s) (1 replace), for KotOR 2 GOG Aspyr (Windows)

## Files

- `kotor1-steam-aspyr-macos.hooks.toml`: hook definitions
- `kotor1_103.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-macos.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `kotor2_gog_aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
