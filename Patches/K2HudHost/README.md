# HUD Host (shared in-game menu / HUD hooks)

> **STATUS** Track: SHIPPABLE. Version 0.6.0-dev (matches manifest.toml).
> **What it does / explores:** Owns three shared HUD hooks and fans them out to consumer mods through the `k2.hud` v1 registry. It draws and writes nothing by itself.
> **Requires:** KotorPatcher with the patch registry (`KPatchInit`); Steam/Aspyr (kotor2_steam_aspyr) for live play; KPM with provider support. No other patch is required (`requires = []`).
> **Known limits:** GOG hooks are static-verified only (contributor testing needed). The 0.6.0 BUILD48 probe is diagnostic. No consumer of `K2HUD_PANEL_BUILD` ships yet. **Play-test status:** HH-PB1 pending and ISF-T13 pending in `patch_manager_mods/PLAY_TEST_QUEUE.md`.
> **Install:** Install through KPM as a provider, then install the consumer mods that list it in `requires`. Not a user-facing patch on its own (no visible feature).

## Features

Three hooks, each fired into the `k2.hud` phases:

| Hook (Steam address) | Phase(s) | When it fires |
|---|---|---|
| `CSWGuiTargetActionMenu::PopulateMenus` `0x00743540` (prologue) | `K2HUD_FRAME`, `K2HUD_TARGET_MENU_PRE` | Before the stock target-menu population |
| PopulateMenus tail `0x00743847` (hud-host 0.4.0) | `K2HUD_TARGET_MENU_POST` | After the menu origin is final, before any draw |
| HUD layout-closing `0x0040F5A0` (prologue) | `K2HUD_BUILD` (HUD panel only), `K2HUD_PANEL_BUILD` (every panel, hud-host 0.6.0) | While the layout is still open, so labels can be created |

Phase ids: `K2HUD_FRAME` 0, `K2HUD_TARGET_MENU_PRE` 1, `K2HUD_TARGET_MENU_POST` 2, `K2HUD_BUILD` 3, `K2HUD_PANEL_BUILD` 4. Defined in `_shared_src/HudHost.h`.

API: consumers call `g_hud.init(api, "my-patch-id")`, `g_hud.add(phase, priority, callback)`, then `g_hud.connect()` from their `KPatchInit`. If `k2.hud` is missing (hud-host not loaded, or loaded later), `connect()` reports it through the `onConnect` callback.

## Configuration

None. No ini keys.

## Logging

Uses `KpLog.h` (`KpLog_Init`, `KP_LOG`). The BUILD48 probe output is capped by `BuildProbeMax`.

## Hooks

Addresses are for the Steam/Aspyr build (`kotor2-steam-aspyr.hooks.toml`, supported SHA256 in `manifest.toml`). GOG addresses are in `kotor2-gog-aspyr.hooks.toml` and are not yet live-tested.

## Diagnostics (not for normal play)

The BUILD48 probe (`probeRead48`, `probeDumpPanel`, and the build probe in `hudBuild`) and the per-frame HUD `+0x40..+0x4F` change-watch are research aids for the `K2HUD_PANEL_BUILD` phase. Do not rely on them for features.

## Build/test

```
python3 scripts/run_tests.py hud_host
```

`test.toml` uses `-I${KPM_COMMON_DIR}` (default `<KPM_DIR>/Patches/Common`) and a vendored Win32 host stub in `host/windows.h` for the Common contract test.

## What was learned

Shared hook ownership keeps consumers from detouring the same addresses twice. Hook addresses for `PopulateMenus` and the layout-closing function now live here, not in the consumers (enemy_effects_strip, combat_hotbar, companion_selfheal).

## Changelog

No CHANGELOG.md yet. Version history is in the `manifest.toml` description and in docs 11 and 31.
