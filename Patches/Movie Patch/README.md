# Movie Patch

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Keeps movies in the configured game window, preserves aspect ratio, and applies Mitchell-Netravali bicubic scaling.

## Details

- Patch id: `better-movie-playback-v1`, version 1.5.3
- Author: J
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)
- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)

## Hooks

- `hooks.toml`: 6 hook(s) (3 detour, 3 simple), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03

## Files

- `better_movie_playback.cpp`: source
- `better_movie_playback.h`: header
- `export.cpp`: source
- `exports.def`: DLL export list
- `hooks.toml`: hook definitions
- `kotor2_hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes
- `mitchell_netravali_filter.cpp`: source
- `mitchell_netravali_filter.h`: header

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
