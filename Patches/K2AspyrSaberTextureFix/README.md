# K2 Aspyr Saber Hilt Texture Fix

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Fixes Aspyr KOTOR2 lightsaber hilt diffuse/detail loss by NOPing the PartEmitter small-particle fast-path cleanup block that disables GL_TEXTURE_COORD_ARRAY. This leaves the GL_COLOR_ARRAY cleanup intact and removes only the existing GL_TEXTURE_COORD_ARRAY disable sequence.

## Details

- Patch id: `k2-aspyr-saber-hilt-texture-fix`, version 1.0.0
- Author: Vriff
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)

## Hooks

- `kotor2-gog-aspyr.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr-linux.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr.hooks.toml`: 1 hook(s) (1 simple), for KotOR 2 Steam Aspyr (Windows)

Other patches in this repo hook the same address on the same build, so check before enabling them together:

- `K2AspyrShaderFixes`: `0x00823AC2` (KotOR 2 GOG Aspyr (Windows)), `0x0837F35F` (KotOR 2 Steam Aspyr (Linux native)), `0x004313A2` (KotOR 2 Steam Aspyr (Windows))

## Files

- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
