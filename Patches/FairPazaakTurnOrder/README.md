# Fair Pazaak Turn Order

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Randomizes who takes the first turn of each Pazaak set. Vanilla always gives the player the first turn, but going SECOND is the real advantage (you act with full information on the opponent's board). This evens the odds with a coin-flip on the set's opening turn. Turn-order only - no 2DA changes - so it stacks with every other Pazaak/balance mod.

## Details

- Patch id: `fair-pazaak-turn-order`, version 0.1.0
- Author: Shae Condon (ShaeMyName)
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)

## Hooks

- `kotor1.hooks.toml`: 1 hook(s) (1 replace), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03, KotOR 1 Steam 1.03

Other patches in this repo hook the same address on the same build, so check before enabling them together:

- `K1AlternatePazaakTurnOrder`: `0x00680085` (KotOR 1 CD-crack 1.03), `0x00680085` (KotOR 1 GOG 1.03), `0x00680085` (KotOR 1 Steam 1.03)

## Files

- `kotor1.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
