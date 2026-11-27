# Party Roster Limit 32 K2 Beta

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Expands the Party roster to 32 slots. The included .gui file must be installed in override to function

## Details

- Patch id: `party-roster-limit-32-K2`, version 0.0.2b
- Author: Vriff
- Requires: none
- Conflicts (declared): `full-party-select`

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)

## Hooks

- `kotor2-gog-aspyr.hooks.toml`: 74 hook(s) (48 detour, 26 simple), for KotOR 2 GOG Aspyr (Windows)

Other patches in this repo hook the same address on the same build, so check before enabling them together:

- `FullPartySelect` (declared in `conflicts`): `0x0058BE29` (KotOR 2 GOG Aspyr (Windows))

## Files

- `PartyRosterLimit32.cpp`: source
- `additional/partyselect_p.gui`
- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
