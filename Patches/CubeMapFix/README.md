# KOTOR 1 Cube Map Fix

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Fixes missing reflections on Intel graphics under Windows.

On these systems KOTOR 1 fails to detect the graphics feature it uses for shiny surfaces, so armour, lightsabers, polished floors and other reflective materials lose their sheen and look flat. This patch corrects the detection so reflections render the way they were meant to.

It makes no difference on AMD or NVIDIA cards, or on Linux, where the game already detects the feature correctly.

## Details

- Patch id: `cube-map-fix`, version 1.0.0
- Author: Synchro
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `kotor1.hooks.toml`: 1 hook(s) (1 simple), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03, KotOR 1 Steam 1.03

## Files

- `kotor1.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
