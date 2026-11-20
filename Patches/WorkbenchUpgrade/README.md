# Workbench Upgrade

> **STATUS** Track: SHIPPABLE. Version 0.1.0-dev (matches manifest.toml).
> **What it does / explores:** Switch between the workbench Create/Breakdown and Upgrade screens without closing the workbench (BTN_SWITCH). Adds a crafting compare box, red unmet-skill rows, a crystal colour preview and a direct-open option that skips the bench dialog.
> **Requires:** KPM patch provider `hud-host` (interface `k2.hud@1`), Steam/Aspyr KOTOR II (hash pinned in manifest.toml `[patch.supported_versions]`), Patch Manager.
> **Known limits:** Live play rows are still pending. Debug `initTrace` file output is present (see PR_READINESS.md, upstream item). Version stays 0.x.y until the upstream PR.
> **Play-test status:** rows WB-1..WB-5, WB-P2, WB-CMP1, WB-UX2, WB-CS1..WB-CS3 in `patch_manager_mods/PLAY_TEST_QUEUE.md` (pending as of PR_READINESS.md).
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
Hooks are in `kotor2-steam-aspyr.hooks.toml`. Addresses are listed in `raw_addresses.toml` and the hooks file. Design notes and engine map: `patch_manager_mods/55_workbench_upgrade.md`.

## Build and test
```
python3 scripts/run_tests.py workbench
python3 scripts/fork_lint.py workbench_upgrade
```

## What was learned
The switch calls the same handler that the hub's Upgrade and Create buttons use. The hub surviving a Create Cancel is answered statically in doc 55 (Q1), with the live probe still pending. Full notes are in doc 55.

## Changelog
No CHANGELOG yet. Release notes will be added with the upstream PR.
