# AurPostString Enabled

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Re-enables the debug AurPostString script function that displays on-screen debug text. This function is called throughout the game scripts but was disabled for release.

## Details

- Patch id: `enable-script-aurpoststring`, version 1.4.1
- Author: KotOR Patch Manager
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `hooks.toml`: 1 hook(s) (1 detour)

## Files

- `aurpoststring_patch.cpp`: source
- `exports.def`: DLL export list
- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
