# K2 Aspyr Shader Fixes

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Fixes Aspyr KOTOR2 fog, reflections, overbright lightmaps, gamma, saber hilt textures, normalmaps and water.

## Details

- Patch id: `k2_aspyr_shader_fixes`, version 1.3.3
- Author: J, Vriff, Synchro
- Requires: none
- Conflicts (declared): `k2-aspyr-brightness-fix`, `k2-aspyr-saber-hilt-texture-fix`, `k2-aspyr-normal-map-fix`, `fog_reflection_fix_k2`

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)
- KotOR 2 Steam Aspyr (macOS) (`kotor2_steam_aspyr_macos`)

## Hooks

- `kotor2-gog-aspyr.hooks.toml`: 13 hook(s) (13 simple), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr-linux.hooks.toml`: 11 hook(s) (11 simple), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr-macos.hooks.toml`: 11 hook(s) (11 simple), for KotOR 2 Steam Aspyr (macOS)
- `kotor2-steam-aspyr.hooks.toml`: 13 hook(s) (13 simple), for KotOR 2 Steam Aspyr (Windows)

Other patches in this repo hook the same address on the same build, so check before enabling them together:

- `K2AspyrFogReflectionFix` (declared in `conflicts`): `0x009A7670` (KotOR 2 GOG Aspyr (Windows)), `0x009A7F68` (KotOR 2 GOG Aspyr (Windows)), `0x009A8518` (KotOR 2 GOG Aspyr (Windows)) and 6 more
- `K2AspyrNormalMapsFix` (declared in `conflicts`): `0x0088DD1E` (KotOR 2 GOG Aspyr (Windows)), `0x004A715E` (KotOR 2 Steam Aspyr (Windows))
- `K2AspyrSaberTextureFix` (declared in `conflicts`): `0x00823AC2` (KotOR 2 GOG Aspyr (Windows)), `0x0837F35F` (KotOR 2 Steam Aspyr (Linux native)), `0x004313A2` (KotOR 2 Steam Aspyr (Windows))

## Files

- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-macos.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
