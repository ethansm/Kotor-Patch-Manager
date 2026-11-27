# Grass Rendering Fix

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Fixes grass rendering as stretched geometry across the sky. RenderGrassPolys has three draw paths; the two taken when aurATIFragmentShadersBumpMapAvailable() returns false pass &field_0x40 -- a field only ever assigned 0 -- to glVertexPointer instead of the allocated blade buffer at field_0x38, so OpenGL reads object fields as vertex coordinates. Also restores GL state the same function leaks: one path returns with three client-state arrays still enabled, and material/texture state is never restored, which breaks lighting on later draws and across module reloads.

## Details

- Patch id: `grass-render-fix`, version 1.0.0
- Author: VexFlint
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)
- KotOR 1 GOG 1.03 (`kotor1_gog_103`)

## Hooks

- `kotor1_103.hooks.toml`: 5 hook(s) (3 replace, 2 simple), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03

## Files

- `kotor1_103.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
