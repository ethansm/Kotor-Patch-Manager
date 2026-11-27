# MP3 Playback Fix

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Ensures loose .mp3 audio files can be played in KOTOR2, and in KOTOR on macOS, prioritizing those files over same-named .wav files in the event of a conflict.

## Details

- Patch id: `mp3-playback-fix`, version 1.1.1
- Author: Synchro, FTD, Vriff, Lane
- Requires: none
- Conflicts (declared): `k2-mp3-playback-fix`

## Supported builds

- KotOR 1 Steam Aspyr (macOS) (`kotor1_steam_aspyr_macos`)
- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)
- KotOR 2 Steam Aspyr (macOS) (`kotor2_steam_aspyr_macos`)

## Hooks

- `kotor1-steam-aspyr-macos.hooks.toml`: 2 hook(s) (2 detour), for KotOR 1 Steam Aspyr (macOS)
- `kotor2-gog-aspyr.hooks.toml`: 2 hook(s) (2 detour), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr-linux.hooks.toml`: 2 hook(s) (2 detour), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr-macos.hooks.toml`: 2 hook(s) (2 detour), for KotOR 2 Steam Aspyr (macOS)
- `kotor2-steam-aspyr.hooks.toml`: 2 hook(s) (2 detour), for KotOR 2 Steam Aspyr (Windows)

## Files

- `MP3PlaybackFix.cpp`: source
- `exports.def`: DLL export list
- `kotor1-steam-aspyr-macos.hooks.toml`: hook definitions
- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-macos.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
