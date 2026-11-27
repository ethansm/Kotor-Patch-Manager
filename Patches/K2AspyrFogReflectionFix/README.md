# Fog and Reflection Fix for KOTOR2 Aspyr

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Fixes several bugs in reflection shaders and enables fog in KOTOR2 Aspyr builds.

## Details

- Patch id: `fog_reflection_fix_k2`, version 0.1.0
- Author: J
- Requires: none
- Conflicts (declared): `k2_aspyr_shader_fixes`

## Supported builds

- KotOR 2 GOG Aspyr (Windows) (`kotor2_gog_aspyr`)
- KotOR 2 Steam Aspyr (Windows) (`kotor2_steam_aspyr`)
- KotOR 2 Steam Aspyr (Linux native) (`kotor2_steam_aspyr_linux`)

## Hooks

- `kotor2-gog-aspyr.hooks.toml`: 6 hook(s) (6 simple), for KotOR 2 GOG Aspyr (Windows)
- `kotor2-steam-aspyr-linux.hooks.toml`: 6 hook(s) (6 simple), for KotOR 2 Steam Aspyr (Linux native)
- `kotor2-steam-aspyr.hooks.toml`: 6 hook(s) (6 simple), for KotOR 2 Steam Aspyr (Windows)

Other patches in this repo hook the same address on the same build, so check before enabling them together:

- `K2AspyrShaderFixes` (declared in `conflicts`): `0x009A7670` (KotOR 2 GOG Aspyr (Windows)), `0x009A7F68` (KotOR 2 GOG Aspyr (Windows)), `0x009A8518` (KotOR 2 GOG Aspyr (Windows)) and 6 more

## Files

- `kotor2-gog-aspyr.hooks.toml`: hook definitions
- `kotor2-steam-aspyr-linux.hooks.toml`: hook definitions
- `kotor2-steam-aspyr.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
