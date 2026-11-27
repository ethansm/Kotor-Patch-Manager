# Max Planets Limit Increase

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

(BETA) Increases the maximum allowed planets in this game from 16 to 127.
NOTE: saves created with this patch applied will misbehave without it, and vice versa!

## Details

- Patch id: `planets-limit`, version 1.0.0
- Author: Lane
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `hooks.toml`: 49 hook(s) (8 detour, 17 replace, 24 simple)

## Files

- `export.cpp`: source
- `exports.def`: DLL export list
- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
