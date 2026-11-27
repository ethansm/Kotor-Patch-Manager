# KOTOR 1 Security and Demolitions XP Rewards

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Adds XP rewards for successful Security unlocks and Demolitions mine disarm/recover actions.

## Details

- Patch id: `k1-security-demolitions-xp`, version 1.0.1
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `hooks.toml`: 4 hook(s) (4 detour), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03, KotOR 1 Steam 1.03

## Files

- `K1SecurityDemolitionsXP.cpp`: source
- `exports.def`: DLL export list
- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
