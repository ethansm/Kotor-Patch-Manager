# KOTOR 1 Soft Shadows Fix

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Enables the Soft Shadows option on non-NVIDIA graphics.

KOTOR 1 only offers soft shadows on NVIDIA hardware and keeps the feature locked off everywhere else unless an undocumented ini setting is present. This patch removes the lock so the option works from the graphics menu.

Your driver still has to expose the two OpenGL features this shadow mode needs, GL_ATI_fragment_shader and WGL_ARB_render_texture, so the patch only makes a difference where both are available: certain AMD driver series on Windows, and Mesa on Linux under Proton/WINE, which provides them for AMD and Intel GPUs alike. Anywhere else the game keeps behaving exactly as it does now.

## Details

- Patch id: `soft-shadows-fix`, version 1.0.0
- Author: Synchro
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 Steam 1.03 (`kotor1_steam_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `kotor1.hooks.toml`: 2 hook(s) (2 simple), for KotOR 1 CD-crack 1.03, KotOR 1 GOG 1.03, KotOR 1 Steam 1.03

## Files

- `kotor1.hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py`.
