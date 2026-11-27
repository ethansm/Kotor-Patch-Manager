# K2 Class Menu Clothes

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Changes the class selection menu clothes to match the screenshot from Prima Guide. Can be changed in new classmenu.2da.

## Details

- Patch id: `k2-class-menu-clothes`, version 1.0.0
- Author: J
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)

## Hooks

- `hooks.toml`: 4 hook(s) (4 detour), for KotOR 2 GOG Aspyr (Windows)

Other patches in this repo hook the same address on the same build, so check before enabling them together:

- `NpcDiversity`: `0x0048E691` (KotOR 2 GOG Aspyr (Windows))

## Files

- `ClassMenuModels.cpp`: source
- `additional/classmenu.2da`
- `exports.def`: DLL export list
- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
