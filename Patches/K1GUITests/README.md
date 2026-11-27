# GUI TESTS

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

I wonder if we can get custom GUIs into the game. this can be used as a decent example for creating basic GUI patches

## Details

- Patch id: `gui-test-1`, version 0.0.4
- Author: Lane
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `hooks.toml`: 2 hook(s) (2 detour)

Other patches in this repo hook the same address on the same build, so check before enabling them together:

- `ModOptions`: `0x006AB648` (KotOR 1 CD-crack 1.03), `0x006AB70B` (KotOR 1 CD-crack 1.03), `0x006AB648` (KotOR 1 GOG 1.03) and 1 more

## Files

- `TestGUI.h`: header
- `additional/monke.tga`
- `additional/optionsingame.gui`
- `additional/test1.gui`
- `exports.cpp`: source
- `exports.def`: DLL export list
- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
