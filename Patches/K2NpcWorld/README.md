# NPC World (npc-world)

> **STATUS** Track: SHIPPABLE. Version 0.1.2-dev (matches manifest.toml).
> **What it does / explores:** Infrastructure provider. It owns the three engine detours that the NPC mods share, tracks one area visit, and exposes the `k2.npcworld@1` interface (area-load, creature-added, area-stable and tick events, plus a native Spawn() with a per-visit cap). It has no gameplay feature of its own.
> **Requires:** Steam/Aspyr exe with LAA (supported_versions `kotor2_steam_aspyr`, sha 6A522E71...BBFFEF), KotorPatcher with the patch registry (KPatchInit export), and Kotor Patch Manager (KPM).
> **Known limits:** Only useful with a client mod (k1-spawn-probe, npc-world-probe, voice-probe). Do not install alongside area-hook-probe, or k1-spawn-probe 0.2.x (both own the same addresses). Conflict list is one-sided until area-hook-probe lists npc-world. **Play-test status:** PRV row passed 10-20; VFS-CT2 is still pending (it relies on game-dir `npc_world.ini` `spawn_cap=6`). Both rows are in the project play-test queue.
> **Install:** Install `npc-world` through KPM as a provider (no feature toggle). Opt out per install with `enabled=0` in `npc_world.ini` (see Configuration).

## Features
- Hooks three engine functions (Steam/Aspyr addresses in `kotor2-steam-aspyr.hooks.toml`):
  - `0x0055A460` LoadModuleInProgress (area load)
  - `0x00568BD0` CSWSCreature AddToArea (creature added to area)
  - `0x00533830` MainLoop (tick)
- Tracks one area visit: a per-visit salt, a 30-tick POP-STABLE latch, and the seen creature tags.
- Provides `k2.npcworld@1` events: `area-load`, `creature-added`, `area-stable`, `tick`.
- Provides a native `Spawn()` service (engine CreateObject path) limited by a per-visit spawn cap.

## Configuration
File: `npc_world.ini` in the game directory (`.\npc_world.ini`). Every key is optional.

```ini
[world]
enabled=1
spawn_cap=4
verbose=0
```

- `enabled`: default `1`. Set `0` to opt out. Hooks stay passive (no events, no salt, no spawns).
- `spawn_cap`: default `4` (`DefaultSpawnCap`, NwCore.h:20). Values below 0 become 0. Values above `32` are clamped to 32 (`HardSpawnCap`, NwCore.h:21).
- `verbose`: default `0`. Only changes the log level when the KpLog core is absent: `1` means DEBUG, `0` means INFO.

## Logging
Log file: `npc-world_log.txt` in the game directory (KpLog fallback file, NpcWorldProvider.cpp:40). The mod logs through `KpLog.h`.

## Interface and dependencies
- `requires = []` (no provider dependency). Uses `KpLog.h` from `Patches/Common`.
- Clients: `k1-spawn-probe` 0.3.x (DLL-only client; 0.3.0+ is fine), `npc-world-probe` 0.1.0-probe (requires npc-world), `voice-probe` (requires `k2.npcworld@1`).

## Conflicts
- `area-hook-probe`: both mods hook 0x0055A460 and 0x00533830. The manifest lists the conflict on this side only. Area-hook-probe 0.2.1 has `conflicts = []`, so the pair is asymmetric until that manifest is changed.
- `k1-spawn-probe` 0.2.x: forbidden (same three addresses). The manifest comment says this, but the conflict is not in `conflicts`. Do not install both.
- `save-events` also hooks 0x00533830. It is a provider with no conflict entry, and both are in the live install. Whether KPM accepts two MainLoop detours is not verified in code.

## Build and test
- Run host tests with `python3 scripts/run_tests.py npc_world` (scope to this mod; a bare run takes over 60 s).
- `test.toml` uses `-I${KPM_COMMON_DIR}`. This points to the KPM `Patches/Common` headers (default `<KPM_DIR>/Patches/Common`).
- `common_contract_test` also uses `-I../../research/companion_levelup/tools/parity/stub`. That path is outside this mod dir and exists only in the development workspace. Outside that workspace, that test cannot build until the include is removed or the stub is copied in.

## What was learned
- The three prologues are `55 8B EC 6A FF` (PUSH EBP / MOV EBP,ESP / PUSH -1). Each detour overwrites whole instructions.
- Reading the engine tag at c+0x18 (0.1.1) fixed duplicate spawns after a save reload.

## Changelog
See `CHANGELOG.md`.
