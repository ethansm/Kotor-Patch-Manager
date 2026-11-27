# Script Events

> **STATUS** Track: EXPERIMENTAL. Version 0.1.1-dev (matches manifest.toml).
> **What it does / explores:** Owns three NWScript VM hook points (CVirtualMachineInternal::RunScript pre and post, CSWVirtualMachineCommands::ExecuteCommand) and provides the `k2.scriptevents@1` registry for script observation, native script overrides and custom VM command handlers.
> **Requires:** Nothing (provider, `requires = []`). Steam/Aspyr KOTOR II (LAA), KPM. Install only if another mod requires `k2.scriptevents`.
> **Known limits:** No in-tree consumer mod yet; the interface is unproven in play. Provider-path and core edge branches lack unit tests. **Testing:** not yet played live; an in-game check needs a consumer mod first.
> **Install:** Do not install for normal play. It has no player-facing behaviour; it is infrastructure R&D for consumer mods.

## Features
- Pre and post hooks on `RunScript` (pre at 0x007022E0, post at 0x007023E6).
- Hook on `ExecuteCommand` (0x00668FE8) for custom VM command handlers.
- Registry interface `k2.scriptevents@1`, declared in `Patches/Common/ScriptEvents.h`.

## Configuration
- `script_events.ini`, section `[script_events]`, key `enabled` (default 1). Set `enabled=0` to opt out (`ScriptEventsProvider.cpp:82-84`).

## Logging
- Logs through `KpLog.h` (provider channel). Enabled-state line is written at init.

## Hooks
- Addresses are mid-function patch points. The containing function `CVirtualMachineInternal::RunScript` starts at 0x007022A0 and is verified; the hook sites are not indexed in the address DB.
- Coexistence: perf-lab uses 0x007022A0 and 0x00668FD0; robustness-fixes uses 0x00668FE0 and 0x00669012. No overlap with this mod.
- Hook definitions: `kotor2-steam-aspyr.hooks.toml`.

## Build
From this folder run `python3 ../create-patch.py`. Host unit tests for `ScriptEventsCore.h` live in the author's development workspace and are not shipped here.

## What was learned (EXPERIMENTAL)
- The VM hook sites are small (7-byte pre-dispatch check) and must stay clear of the perf-lab and robustness-fixes prologue and handler hooks.

## Changelog pointer
- No CHANGELOG.md yet; see git history for this directory.
