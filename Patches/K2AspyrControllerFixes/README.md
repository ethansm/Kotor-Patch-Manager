# K2 Aspyr Controller Fixes

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Makes controllers work on Aspyr's native Linux KOTOR2: recognises modern pads, and fixes the startup crash and stuck help-menu input that follow.

## Details

- Patch id: `k2-aspyr-controller-fixes`, version 1.0.0
- Author: Synchro
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)

## Hooks

- `kotor2-steam-aspyr-linux.hooks.toml`: 4 hook(s) (3 detour, 1 simple), for KotOR 2 Steam Aspyr (Linux native)

## Files

- `ControllerDB.cpp`: source
- `ControllerDbEmbedded.cpp`: source
- `ControllerDbEmbedded.h`: header
- `HelpMenuInputFix.cpp`: source
- `JoystickCloseCrashFix.cpp`: source
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes
- `tools/gamecontrollerdb.txt`
- `tools/generate-embedded-db.py`: script

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
