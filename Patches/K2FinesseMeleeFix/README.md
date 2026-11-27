# KOTOR 2 Finesse: Melee No Longer Applies to Lightsabers

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Fixes KOTOR2 Aspyr's GetWeaponFinesse fallthrough so Finesse: Melee Weapons no longer grants Dexterity attack bonus with lightsabers.

## Details

- Patch id: `k2-finesse-melee-fix`, version 1.0.0
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)

## Hooks

- `kotor2-gog-aspyr.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr-linux.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 Steam Aspyr (Windows)

## Files

- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
