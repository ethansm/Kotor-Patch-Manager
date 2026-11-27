# Saber Throw Flight SFX

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Saber Throw flight audio using sbr_throw. Overrides projectile's looping flight sound.

## Details

- Patch id: `saber-throw-flight-sfx`, version 0.1.0
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)
- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)

## Hooks

- `kotor1.hooks.toml`: 1 hook(s) (1 replace), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03, KotOR 1 Steam 1.03
- `kotor2-gog-aspyr.hooks.toml`: 1 hook(s) (1 replace), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr.hooks.toml`: 1 hook(s) (1 replace), for KotOR 2 Steam Aspyr (Windows)

## Files

- `additional/sbr_throw.wav`
- `kotor1.hooks.toml`: hook definitions
- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
