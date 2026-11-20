# Area Hook Probe (area-hook-probe)

> **STATUS** Track: DIAGNOSTIC. Version 0.2.1 (matches manifest.toml). Retired from live use, kept for research.
> **What it does / explores:** Log-only probe of which engine paths run when a saved game loads into an already-visited area (H3 `CSWSModule::LoadModuleStart`) versus a fresh area load (H2 `CSWSModule::LoadModuleInProgress`). It also logs the area-ready latch at `CServerExoAppInternal+0x14`, a population-stable marker (POP-STABLE / POP-TIMEOUT), and the area pointer, object id and resref seen by `CSWSArea::AddObjectToArea` (H4).
> **Requires:** Steam/Aspyr exe with LAA (supported_versions `kotor2_steam_aspyr`, sha 6A522E71...BBFFEF), KotorPatcher with the patch registry (KPatchInit export), and Kotor Patch Manager (KPM). Logs through `KpLog.h` (`_shared_src`).
> **Known limits:** Steam/Aspyr only. No test suite exists (see Future iterations). The latch meaning is INFERRED, not a verified ready marker. Shares `0x00533830` (MainLoop) and `0x0055A460` (LoadModuleInProgress) with `npc-world`; the two conflict with each other in both manifests. **Play-test status:** none pending. The probe was removed from the live set in T26 (see `../40_npc_behaviour_variety.md`, section 11).
> **Install:** Do not install for normal play. It is a research diagnostic with detours that clash with `npc-world`, and only one owner per address may be live. Install only to reproduce the saved-vs-fresh load study, and never alongside `npc-world`.

## Features
- Four detours, all thiscall with ECX = object (Steam/Aspyr addresses in `kotor2-steam-aspyr.hooks.toml`):
  - H1 `0x00533830` CServerExoAppInternal::MainLoop (every tick): latch read, POP watch.
  - H2 `0x0055A460` CSWSModule::LoadModuleInProgress (fresh area load): arms the POP watch.
  - H3 `0x005582F0` CSWSModule::LoadModuleStart (saved-game module load, does not call LoadArea): arms the POP watch.
  - H4 `0x00522BF0` CSWSArea::AddObjectToArea: logs when the area pointer changes; object id at area+0x124, resref at area+0x110.
- Spawns nothing and changes no gameplay.

## Configuration
- No ini keys. Output file: `area_hook_probe_log.txt` in the game dir (channel `main`; falls back to the same file name at INFO on an old core).

## Logging
- Lines start `t<ticks> `. Each 0 -> nonzero latch transition dumps 0x40 dwords of the server object (capped at 8 dumps).
- `H1 POP-STABLE`: 30 consecutive ticks with no AddObjectToArea call after at least one add.
- `H1 POP-TIMEOUT`: no stable window within 1800 ticks after arming.
- Long lines are split under the 232-byte core cut.

## Hooks
- Addresses are the Steam/Aspyr entries in `kotor2-steam-aspyr.hooks.toml` (4 `[[hooks]]` entries). They are not in `raw_addresses.toml`.
- `npc-world` owns `0x00533830` and `0x0055A460` and declares `conflicts = ["area-hook-probe"]`. This manifest declares `conflicts = ["npc-world"]`.

## What was learned
- The latch at CServerExoAppInternal+0x14 and the POP-STABLE marker gave a usable "population settled" signal on 7 of 7 loads (about 33 ticks after entry), recorded in `../40_npc_behaviour_variety.md` (NB1 row).
- AddToArea in `261tel` has a branch that `204tel` does not; it is unexplained (doc 40, detour conflicts note).

## Build and test
- No build gate or test suite. `python3 scripts/fork_lint.py area_hook_probe` checks the STATUS banner and manifest only.

## Future iterations
- Tests (none exist): `fmtResref` and `fmtLatch` branches, POP-STABLE and POP-TIMEOUT counters, H4 same-area dedupe, and log split and caps. These need a fake log sink harness.

## Research links
- `../40_npc_behaviour_variety.md`: detour conflicts (sections 11 and 12).
- `../npc_world_src/README.md` and `../npc_world_src/CHANGELOG.md`: the shared provider that replaced this probe's detours.

## Changelog
See `CHANGELOG.md`.
