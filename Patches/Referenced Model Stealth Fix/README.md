# Referenced Model Stealth Fix

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Passes stealth effect to referenced models.

## Details

- Patch id: `referenced-model-stealth`, version 1.4.0
- Author: J
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)
- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)

## Hooks

- `kotor1.hooks.toml`: 4 hook(s) (4 detour), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03
- `kotor2-gog-aspyr.hooks.toml`: 4 hook(s) (4 detour), for KotOR 2 GOG Aspyr (Windows)
- `hooks.toml`: 4 hook(s) (4 detour), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03

## Files

- `ReferencedModelStealth.cpp`: source
- `exports.def`: DLL export list
- `hooks.toml`: hook definitions
- `kotor1.hooks.toml`: hook definitions
- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
