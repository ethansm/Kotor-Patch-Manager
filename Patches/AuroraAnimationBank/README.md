# Aurora Animation Bank

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Establishes an animation bank for KOTOR 2. Mappings are read from stuntmodel.2da through KOTOR II's resource manager.

## Details

- Patch id: `aurora-animation-bank`, version 1.0.0
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)

## Hooks

- `hooks.toml`: 3 hook(s) (3 detour)

## Files

- `Additional/stuntmodel.2da`
- `animation_banks.cpp`: source
- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
