# Grass Buffer Double Free

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

CreateArrays stores one allocation in two fields (0x38 and 0x3c); DestroyGrassPolys and ~CAurTriangleBin both free each of them. The 0x3c field is only ever assigned 0 or the 0x38 pointer, never its own allocation, so the first free is either free(NULL) or a double free of the same block. Frees the alias only when it genuinely differs, so nothing leaks if 0x3c ever holds an independent allocation.

## Details

- Patch id: `grass-memory-safety`, version 1.0.0
- Author: VexFlint
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)
- KotOR 1 GOG 1.03 (`kotor1_gog_103`)

## Hooks

- `kotor1_103.hooks.toml`: 2 hook(s) (2 replace), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03

## Files

- `kotor1_103.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
