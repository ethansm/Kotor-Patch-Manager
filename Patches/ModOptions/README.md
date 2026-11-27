# Mods Options GUI

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Allows support for custom Options GUIs for mods and patches.
Place TOML configurations in a folder titled "mod options" in the base game directory.
For examples of how to format these configurations, see the sample.toml that ships with this patch.

## Details

- Patch id: `mod-options-gui`, version 1.0.0
- Author: Lane
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `hooks.toml`: 4 hook(s) (4 detour)

Other patches in this repo hook the same address on the same build, so check before enabling them together:

- `K1GUITests`: `0x006AB648` (KotOR 1 CD-crack 1.03), `0x006AB70B` (KotOR 1 CD-crack 1.03), `0x006AB648` (KotOR 1 GOG 1.03) and 1 more

## Files

- `GuiDetours.cpp`: source
- `ModOptionFunctions.h`: header
- `ModOptionIni.h`: header
- `ModOptions.h`: header
- `ModOptionsConfig.h`: header
- `OptionsEditBox.h`: header
- `OptionsLayout.h`: header
- `OptionsListBox.h`: header
- `OptionsListSelect.h`: header
- `OptionsMenu.h`: header
- `OptionsSlider.h`: header
- `additional/modoptionmenu.gui`
- `additional/modoptions.gui`
- `additional/optionsingame.gui`
- `additional/optionsmain.gui`
- `additional/sample.toml`
- `exports.def`: DLL export list
- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
