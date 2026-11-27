# KOTOR 1 Pazaak KOTOR 2 Speed

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Retunes KOTOR 1 GOG 1.03 pazaak GUI sequence delays to match the shorter KOTOR 2 pacing constants: 0.4s -> 0.1s for two state transitions, and 0.8s -> 0.2s for the AI action delay.

## Details

- Patch id: `k1-pazaak-k2-speed`, version 1.0.0
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `kotor1.hooks.toml`: 3 hook(s) (3 simple), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03, KotOR 1 Steam 1.03

## Files

- `kotor1.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
