# Workbench Upgrade

> **STATUS** Track: SHIPPABLE. Version 0.1.0-dev (matches manifest.toml).
> **What it does / explores:** Switch between the workbench Create/Breakdown and Upgrade screens without closing the workbench (BTN_SWITCH). Adds a crafting compare box, red unmet-skill rows, a crystal colour preview and a direct-open option that skips the bench dialog.
> **Requires:** KPM patch provider `hud-host` (interface `k2.hud@1`), Steam/Aspyr KOTOR II (hash pinned in manifest.toml `[patch.supported_versions]`), Patch Manager.
> **Known limits:** Live play checks are still pending. Debug `initTrace` file output is present (an upstream item to remove). Version stays 0.x.y until the upstream PR.
> **Testing:** not yet fully tested live; the switch button, compare box, red unmet-skill rows and crystal colour preview still need an in-game check.
> **Install:** Install `hud-host` first, then install `workbench-upgrade` through Patch Manager. It applies the hooks in `kotor2-steam-aspyr.hooks.toml`.

## Features
- **BTN_SWITCH:** toggles between Create/Breakdown and Upgrade on the same bench visit. It calls the same code path as the hub buttons.
- **Crafting compare box:** shows a compare panel on the Create screen (`crafting_compare`).
- **Unmet-skill rows:** rows whose skill requirement is not met are drawn in red.
- **Crystal colour preview:** previews the crystal colour on the upgrade screen.
- **Direct open:** opens the screen without the bench dialog (`skip_dialog`).

## Configuration (`workbench_upgrade.ini`, section `[workbench_upgrade]`)
| Key | Default | Meaning |
|---|---|---|
| `switch_button` | `1` | Show the BTN_SWITCH mode switch button (1 on, 0 off) |
| `crafting_compare` | `1` | Crafting compare integration on the Create screen (1 on, 0 off) |
| `verbose` | `0` | Verbose probe dumps (1) or lifecycle/switch events only (0) |
| `skip_dialog` | `1` | Open directly, skipping the bench dialog (1 skip, 0 show) |

## Logging
Uses the shared `KpLog.h` logger, configured through the `kplog.ini` section for this mod.

## Hooks
Hooks are in `kotor2-steam-aspyr.hooks.toml`. Addresses are listed in the hooks file.

## Build
From this folder run `python3 ../create-patch.py`. Host unit tests live in the author's development workspace and are not shipped here.

## What was learned
The switch calls the same handler that the hub's Upgrade and Create buttons use. The hub surviving a Create Cancel was answered by static analysis only; the live probe is still pending.

## Changelog
No CHANGELOG yet. Release notes will be added with the upstream PR.
