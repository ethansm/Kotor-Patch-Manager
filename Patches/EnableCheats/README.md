# Enable Cheats

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Test patch. Enables the KOTOR II cheat console without editing swkotor2.ini. It was authored to validate KPM's install-time (STATIC) byte-patching pipeline end to end. Note: the console accepts commands and the cheats take effect, but its on-screen prompt is invisible. That is a separate, known KOTOR II Aspyr rendering issue, not a fault in this patch.

## Details

- Patch id: `enable-cheats`, version 1.0.0
- Author: Synchro
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)

## Hooks

- `kotor2-steam-aspyr-linux.hooks.toml`: 2 hook(s) (2 static), for KotOR 2 Steam Aspyr (Linux native)

## Files

- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
