# Party Roster Limit 32 - KotOR I Beta

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Expands the party roster to 32 slots. The included .gui file must be installed to function.

## Details

- Patch id: `party-roster-limit-32-k1`, version 0.0.2b
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `kotor1.hooks.toml`: 53 hook(s) (46 detour, 7 simple), for KotOR 1 GOG 1.03

## Files

- `PartyRosterLimit32K1.cpp`: source
- `additional/partyselection.gui`
- `kotor1.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
