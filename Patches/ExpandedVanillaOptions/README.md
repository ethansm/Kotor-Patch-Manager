# Expanded Vanilla Options

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Utilizes the Mod Options patch to include additional menus for various vanilla settings that are typically hidden from the user.

## Details

- Patch id: `expanded-vanilla-options`, version 0.1.0
- Author: Lane
- Requires: `mod-options-gui`
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `hooks.toml`: 5 hook(s) (4 detour, 1 simple)

## Files

- `additional/additional-debug.toml`
- `additional/additional-game.toml`
- `additional/additional-graphics.toml`
- `exports.def`: DLL export list
- `handlers.cpp`: source
- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes
- `notes.md`: doc

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
