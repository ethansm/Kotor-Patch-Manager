# Combat Action Hotbar (`combat-hotbar`)

> **STATUS** Track: SHIPPABLE. Version 0.11.5-dev (matches manifest.toml).
> **What it does / explores:** Replaces the vanilla target-menu action groups with clickable icon buttons in vanilla-style pills, adds Ctrl+Shift+F9..F12 invoke keys, and per-group pin/hide/sort set in `combat_hotbar.ini`.
> **Requires:** `hud-host` (k2.hud@1, listed before this mod), Steam/Aspyr KOTOR II (manifest `kotor2_steam_aspyr` hash). Optional: `mod-config` + `menuui` for the Mod Configuration options page (the mod runs without them; only the page is missing).
> **Known limits:** A group of 10 items shows 8 buttons at the default `Cols=2`, `MaxRows=4` (the other 2 are cut, not scrolled; see `combat_page_test.cpp`). No paging or scrolling yet. Standalone: no, needs hud-host. **Play-test status:** MC-TT1, CH-PG1, CH-PV1 and MC-PF1 are pending in the project play-test queue.
> **Install:** Install through Kotor Patch Manager as `combat-hotbar`, with `hud-host` enabled.

## Features

- **Clickable group buttons:** each target-menu action group (three groups) gets a grid of icon buttons inside its pill. Empty slots are hidden by default (`HideEmpty=1`).
- **Invoke keys:** by default Ctrl+Shift+F9..F12 (`Key0..Key3`) select the last usable entry of a group and invoke it, the same path as a button click.
- **Arrow hide:** `[Arrows] Hide=1` hides the carousel arrows on the target menu.
- **Profile:** `[Profile]` pins (`Pin0..Pin3`), hides (`Hide0..Hide3`) and sorts (`Sort0..Sort3`) entries per group. `Sort` values are `count` or `engine`.
- **Options page (optional):** button size, columns and spacing, with live preview (needs mod-config + menuui).

## Configuration (`combat_hotbar.ini`)

Hot-reloads within 1 second (FileWatch).

| Section | Keys (default) |
|---|---|
| `[Invoke]` | `Enabled` (1), `Mod` (3: bit0 Ctrl, bit1 Shift), `Key0..Key3` (VK_F9..VK_F12), `KeepSelection` (0) |
| `[Arrows]` | `Hide` (0) |
| `[Buttons]` | `Enabled` (1), `Replace` (1: hide the vanilla group controls), `Cols` (2, range 1..4), `MaxRows` (4, range 1..8), `BtnPx` (0 = auto, else 8..256), `Pad` (default in code, 0..32), `GroupGap` (0..64), `Inset` (3, 0..16), `ClickSound` (0, -1..18), `CapPad`, `SidePad`, `HideEmpty` (1), `ColsTotal` (6, 1..12) |
| `[Profile]` | `Pin0..Pin3`, `Hide0..Hide3`, `Sort0..Sort3` (comma lists; a token matches an entry name exactly, a `" "`-suffix when no entry has that exact name, or a `0x` engine key) |
| `[General]` | `Verbose` (0; 1 adds per-frame and probe log lines) |

## Logging

Uses the shared KpLog (`KpLog.h`). Set `[General] Verbose=1` for the per-frame and probe lines. Read mod logs with the `kotor2-log-analysis` skill, not raw.

## Hooks

No hooks of its own. `hud-host` owns the target-menu `PopulateMenus` hook (0x00743540) and calls this mod through `k2.hud@1` (`K2HUD_TARGET_MENU_PRE`, `K2HUD_TARGET_MENU_POST`, `K2HUD_BUILD`). The engine invoke wrapper is 0x00751710 (`steam_functions.tsv`).

## Build / test

```
python3 scripts/fork_lint.py combat_hotbar
python3 scripts/run_tests.py combat_hotbar
```

Test suites: `common_contract_test`, `combat_page_test`, `combat_preview_test`, `combat_logic_test` (see `test.toml`). `KPM_COMMON_DIR` defaults to `<KPM_DIR>/Patches/Common`.

## What was learned

- The target menu has three vanilla group boxes; button layout is row-major, two columns per group by default.
- A group larger than `Cols x MaxRows` is cut, not scrolled. Raise `MaxRows` or `Cols` in `[Buttons]` to see more.

## Changelog

No CHANGELOG file (same as `quick_loot_src`). History is in the `git log` for the project workspace and the header comments in `CombatHotbar.cpp`.
