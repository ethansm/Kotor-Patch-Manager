# Mod Configuration

> **STATUS** Track: SHIPPABLE. Version 0.4.9-dev (matches manifest.toml).
> **What it does / explores:** Adds a "Mod Configuration" button under Gameplay in the Options screen. Other mods register config pages into it through `k2.modconfig@1` and build them with `k2.menuui@1`, so every mod's settings look like vanilla menus.
> **Requires:** Steam/Aspyr KOTOR II (target hash in manifest.toml) and KPM. No other patch is required (`requires = []`). Optional `k2.guiinput` provider for mouse wheel scrolling.
> **Known limits:** `arrow_icons=1` is reserved (logged only, text arrows are used). The ESC hook is not unhooked on DLL detach yet (open safety item). Latched-click and fail-fast root clone optimisations are not done. **Testing:** live-tested on Steam; the menu page and the ESC handling still need a further in-game check.
> **Install:** KPM install of `mod-config` (provider, install before its consumers: holster (its settings page), enemy-effects-strip, inventory-sort-filter).

## Features
- One "Mod Configuration" button after GAMEPLAY (or below QUIT when `reflow=0`).
- List page: one button per registered entry, hover shows the entry description.
- Entry page: the entry's own controls (toggles, steppers, sliders, labels, custom panes), built by its `build()` callback.
- Esc closes the menu from any page; reopening starts at the Options root.

## Configuration (`mod_config.ini`, section `[mod-config]`)

| Key | Default | Meaning |
|---|---|---|
| `enabled` | 1 | Master switch. 0 makes every hook return at once. |
| `debug` | 0 | Extra log lines (hook entry, button rects, saved control flags). |
| `button_clone` | 1 | 1 = real button clones (BTN_GAMEPLAY, BTN_EXIT). 0 = label clones. A failed clone falls back to a label. |
| `reflow` | 1 | 1 = Mod Configuration sits after GAMEPLAY and later rows move down one pitch. 0 = below QUIT. |
| `desc` | 1 | 1 = hovering a button writes a description into the description pane. |
| `free_labels` | 1 | 1 = free our buttons and labels when the Options panel is destroyed. 0 = leak them. |
| `arrow_icons` | 0 | Reserved for chevron icons. Not built. |
| `selftest` | 0 | 1 or 2 registers a built-in test entry for play checks. Never leave on in a shipped config. |

## Logging
Add a `[mod-config]` section to the game-dir `kplog.ini`. Lines go to `mod-config_log.txt` (fallback level INFO).

## Interface summary for mod authors
Declared in Patches/Common/ModConfigApi.h`. Provided by this mod; consumers use the struct sizes from that header.
- `k2.modconfig@1` (`K2ModConfigApi`): `Register(entry)` returns a token (0 = refused, all slots used); `Unregister(token)`. An entry carries `id`, `name`, `desc` and the callbacks `build`, `enter`, `leave`, `tick`, `destroy`.
- `k2.menuui@1` (`K2MenuUiApi`): `NewPage`, `AddButton`, `AddLabel`, `AddCustom`, `Clicked`, `SetText`, `SetRect`, `SetDesc`, `SetDescSplit`, `PaneRect`, `Control`. Only valid inside the entry's own `build()`, for that entry's page.
- Page changes fire on the left mouse button release edge.

## Hooks
Three Options-menu sites in `kotor2-steam-aspyr.hooks.toml` (plain detours, whole stolen instructions). Addresses are raw; lookup evidence for the PR text is still to be added.

## Build
From this folder run `python3 ../create-patch.py`. Host unit tests (common contract, menu UI logic, slider row logic) live in the author's development workspace and are not shipped here.

## Known gaps
- Host-glue tests for `ModConfig.cpp` (readConfig, pane status floor, slider give-up paths) are not written. The logic headers are covered.
- Declarative layer (ModConfigDecl, PreviewCanvas) lives outside this directory.
