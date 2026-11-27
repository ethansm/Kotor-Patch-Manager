# Lane's KotOR Script Extender

> This README was generated from `manifest.toml` and the hooks files in this folder. It records what those files say;
> it is not the author's own documentation and has no test results.

## What it does

Adds additional NWScript commands.

## Details

- Patch id: `script-extender`, version 1.2.3
- Author: Lane
- Requires: none
- Conflicts (declared): none

## Supported builds

- KotOR 1 GOG 1.03 (`kotor1_gog_103`)
- KotOR 1 CD-crack 1.03 (`kotor1_cdcrack_103`)

## Hooks

- `hooks.toml`: 4 hook(s) (1 detour, 3 simple)

## Files

- `Extensions/clientCreatures.cpp`: source
- `Extensions/clientCreatures.h`: header
- `Extensions/consoleCommand.cpp`: source
- `Extensions/consoleCommand.h`: header
- `Extensions/creatureStats.cpp`: source
- `Extensions/creatureStats.h`: header
- `Extensions/fileIO.cpp`: source
- `Extensions/fileIO.h`: header
- `Extensions/globalModifiers.cpp`: source
- `Extensions/globalModifiers.h`: header
- `Extensions/heartbeat.cpp`: source
- `Extensions/heartbeat.h`: header
- `Extensions/ini.cpp`: source
- `Extensions/ini.h`: header
- `Extensions/lastTarget.cpp`: source
- `Extensions/lastTarget.h`: header
- `Extensions/trig.cpp`: source
- `Extensions/trig.h`: header
- `Extensions/twoDA.cpp`: source
- `Extensions/twoDA.h`: header
- `ScriptExtender.cpp`: source
- `additional/nwscript.nss`
- `exports.def`: DLL export list
- `hooks.toml`: hook definitions
- `manifest.toml`: patch identity and supported executable hashes

## Install and build

- Put the built `.kpatch` in the folder set as the Patches path in KPatchLauncher, then enable it there.
- Build the `.kpatch` from this folder with `python3 ../create-patch.py` (it compiles the C++ DLL, see the repo README for the toolchain).
