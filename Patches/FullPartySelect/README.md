# Full Party Select

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

This patch disables blanking of paired party-select portraits (Hanharr/Mira, Handmaiden/Disciple). By enabling this patch and installing partyselect_p.gui (under 'additional') in your override folder, all 12 slots will be made available on your party-select screen.

## Details

- Patch id: `full-party-select`, version 0.0.1
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)
- KotOR 2 Steam Aspyr (macOS) (`kotor2_steam_aspyr_macos`)

## Hooks

- `kotor2-gog-aspyr.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr-linux.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr-macos.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 Steam Aspyr (macOS)
- `kotor2-steam-aspyr.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 Steam Aspyr (Windows)

Other patches in this repo hook the same address on the same build, so check before enabling them together:

- `PartyRosterLimit32K2`: `0x0058BE29` (KotOR 2 GOG Aspyr (Windows))

## Files

- `additional/partyselect_p.gui`
- `additional/readme.md`: doc
- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-macos.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
