# Enemy Effects Strip

> **STATUS** Track: SHIPPABLE. Version 0.4.11-dev (matches manifest.toml).
> **What it does / explores:** Draws the current target's status-effect icons as one row above the target menu name label. Icons are gated by the party leader's Awareness tiers.
> **Requires:** hud-host (k2.hud@1) installed first or in the same set. Steam/Aspyr KOTOR II exe (SHA `6A522E71...`, see manifest.toml). Optional: mod-config (k2.modconfig@1, k2.menuui@1) for the settings page; connected at runtime, never required.
> **Known limits:** The MaxEffects and MaxIconCount reject paths have no unit test (defensive bounds only). Tier thresholds are not tested with zero values. **Testing:** the UX and config-option checks still need an in-game pass.
> **Install:** Install through Patch Manager (KPM) with hud-host. Copy `enemy_strip.ini` next to `swkotor2.exe` only if you want to edit defaults by hand; every key is optional.

## Features
- One row of small icons above the target's name label, max `MaxIcons` (default 5). Icons only, no text, no tooltip.
- Three Awareness tiers (leader's Awareness rank):
  - **Obvious** (`TierObvious`, default 5): stun, hold, sleep, fear, entangle, knockdown, slow.
  - **Own** (`TierOwn`, default 12): also debuffs the party applied.
  - **Full** (`TierFull`, default 20): every tracked debuff and buff, with a remaining-time drain overlay.
- Optional settings page under Options -> Mod Configuration when mod-config is installed. Changes apply at once and are written to `enemy_strip.ini`. A live preview shows a mock name bar with sample icons.

## Configuration (`enemy_strip.ini`, section `[Strip]`)
| Key | Default | Meaning |
|---|---|---|
| `Enabled` | 1 | Master switch. |
| `Verbose` | 0 | 1 adds the probe blocks to the log. |
| `MaxIcons` | 5 | Icons shown at once, 1..8. Hand edits need a restart. |
| `IconSize` | 0 | Icon edge in px; 0 = name label height. |
| `Gap` / `Spacing` | 4 / 4 | Px between row and label / between icons. |
| `TierObvious` / `TierOwn` / `TierFull` | 5 / 12 / 20 | Awareness thresholds, 0..127, forced to be non-decreasing. |
| `DrainAlpha` | 0 | Alpha 0..1 of the drain shading (tier Full only). |
| `Backing` / `BackingAlpha` / `BackingPad` | 0 / 0.55 / 4 | Optional pill behind the row. |
| `ForceTier` | -1 | -1 = from Awareness; 0..3 forces a tier. |

## Logging
- Log file: `enemy-effects-strip_log.txt` in the game folder.
- `Verbose=1` adds the probe blocks (menu geometry, effect list, icon cache, Awareness). `Verbose=0` writes only `strip:` change lines.

## Hooks
- No hooks of its own. `kotor2-steam-aspyr.hooks.toml` is comment-only.
- The `CSWGuiTargetActionMenu::PopulateMenus` detour (0x00743540) belongs to hud-host. This mod registers `onTargetMenuPost` for `K2HUD_TARGET_MENU_POST` in `KPatchInit`.

## Build
From this folder run `python3 ../create-patch.py`.

Host unit tests (logic, fake-engine draw checks, Common contract) live in the author's development workspace and are not shipped here.

## What was learned
- The strip's draw position depends on the target menu label height, so `IconSize=0` tracks the label rather than a fixed pixel size.
- The settings page's preview labels must restore their own fill before any free. Freeing them at Options teardown crashed in the label destructor (fixed in 0.4.9 and 0.4.10).

## Changelog pointer
- No CHANGELOG.md.
