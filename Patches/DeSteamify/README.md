# KOTOR2 DeSteamify

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Removes the Steam Aspyr executable's static steam_api.dll import descriptor and neutralizes Steamworks startup/callback/UGC/stat call sites so the game can be launched by a custom launcher without Steamworks gating. Targeted only at the uploaded KOTOR2 Steam Aspyr executable.

## Details

- Patch id: `kotor2-desteamify`, version 0.1.0
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)

## Hooks

- `kotor2-steam-aspyr.hooks.toml`: 13 hook(s) (12 simple, 1 static), for KotOR 2 Steam Aspyr (Windows)

## Files

- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
