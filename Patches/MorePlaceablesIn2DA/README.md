# Placeables 2DA Row Limit expansion

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Increases the max recognized rows in placeables.2da from 256 to 2,147,483,648. This is experimental and still in need of thorough testing.

## Details

- Patch id: `more-placeables-in-2da`, version 0.2.0
- Author: Lane
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `hooks.toml`: 62 hook(s) (62 simple)

## Files

- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
