# Save Events (shared save / load / new-game hooks)

> **STATUS** Track: SHIPPABLE. Version 0.1.1-dev (matches manifest.toml).
> **What it does / explores:** Owns the five save, autosave, load and new-game detours for Steam Aspyr and provides the `k2.saveevents` v1 registry, so several mods can share those hooks without fighting over the same addresses.
> **Requires:** Steam/Aspyr build (6A522E71...), a KotorPatcher with the patch registry (KPatchInit). No other provider.
> **Known limits:** Provider only, no user-visible feature. Useful only through consumers (action-hotbar, level-up GUID binding). Three hook addresses (0x00534527, 0x0053C5AA, 0x0052FD1E) are not yet confirmed in the Ghidra DB (Haiku lookup, not re-run). No CHANGELOG yet. **Play-test status:** PS2 and PS3 passed live 10-27 (PLAY_TEST_QUEUE.md:100-101).
> **Install:** Install through KPM like other providers. Do not install together with an Action Hotbar build older than 1.3.0-dev, which owns the same five addresses (KPM rejects the duplicate hooks).

## Features
- Five cdecl detours covering save, autosave, load and new-game events. Original bytes still run.
- Parses the save folder (local or Steam Cloud root). If a save's folder name is unreadable, it recovers the most recent save folder with one read-only directory listing (PS2).
- Registry: consumers register a kind mask and a callback. Consumers are called per event; a slow consumer is guarded so the game never blocks.

## Configuration
- None in this provider. Consumers (action-hotbar) own their own INI keys.

## Logging
- Fallback file: `save-events_log.txt`.
- Channel `evt` in kpatch.log (patch `save-events`): one INFO line per event (kind, folder, cloud flag, consumers called).

## Hooks
- Detours: 0x00534527, 0x0053C5AA, 0x0052FD1E, 0x008529E0, 0x008FACA0. Hashes and bytes come from the shipped Action Hotbar hooks.
- 0x008529E0 and 0x008FACA0 are backed by the Ghidra DB. The other three are not yet confirmed (see Known limits).

## Build / test
- Host tests: `python3 scripts/run_tests.py save_events`
- Host test files: `se_registry_test.cpp` (registry, folder parsing, slow-consumer guard), `saveevents_client_test.cpp` (client side), `common_contract_test.cpp` (pins the Common APIs).

## API
- `k2.saveevents` v1 is defined in `_shared_src/SaveEvents.h`, not in this directory. Design and migration: `patch_manager_mods/22_hotbar_shared_services.md` sections 4.1 and 4.1a.

## What was learned
- Not an experimental mod. Its design notes are in doc 22.

## Changelog pointer
- No CHANGELOG.md yet. Version stays 0.x.y until the upstream PR.
