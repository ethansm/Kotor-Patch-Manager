# Semi-Transparent Letterbox

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Changes the dialogue letterbox alpha from 1.0 (opaque) to 0.5 (semi-transparent)

## Details

- Patch id: `semi-transparent-letterbox`, version 1.0.0
- Author: Lane (K1) & Vriff (K2)
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)
- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)
- KotOR 1 Steam Aspyr (macOS) (`kotor1_steam_aspyr_macos`)

## Hooks

- `kotor1-steam-aspyr-macos.hooks.toml`: 3 hook(s) (3 simple), for KotOR 1 Steam Aspyr (macOS)
- `kotor1.hooks.toml`: 3 hook(s) (3 simple), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03, KotOR 1 Steam 1.03
- `kotor2-gog-aspyr.hooks.toml`: 4 hook(s) (4 replace), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr-linux.hooks.toml`: 2 hook(s) (2 simple), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr.hooks.toml`: 4 hook(s) (4 replace), for KotOR 2 Steam Aspyr (Windows)

## Files

- `kotor1-steam-aspyr-macos.hooks.toml`: hook definitions
- `kotor1.hooks.toml`: hook definitions
- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
