# Inventory Sort & Filter

> **STATUS** Track: SHIPPABLE. Version 0.9.0-dev (matches manifest.toml).
> **What it does / explores:** Sorts the backpack (category, base item, name, or vanilla order), tints items the shown character cannot equip, adds New / Can Equip / Upgradeable filters, a clickable filter strip, a mod-config page and an equip-compare band under the item description.
> **Requires:** Steam/Aspyr kotor2 (supported hash in manifest.toml), KPM, `hud-host` providing `k2.hud@1`. Optional: mod-config 0.2.0+ for the config page.
> **Known limits:** Equip compare band is on by default (`compare=1`); its coverage verdict is still open (doc 11). Category divider rows and v2/v3 UX are not built. **Play-test status:** 12 ISF rows pending in the project play-test queue (ISF-T2..T16, ISF-B1, WI-21); ISF-T1 and ISF-T5 PASS.
> **Install:** Install through KPM like any other patch (manifest id `inventory-sort-filter`). Install `hud-host` first.

## Features
- Backpack sort: `sort_order` = category, base, name, or vanilla.
- Tint: items the shown character cannot equip get the red-orange "cannot use" colours.
- Filter cycle: re-click the active All tab to cycle All -> New -> Can Equip -> Upgradeable -> All. Controller bumpers cycle the same way.
- Strip: own status line above the item list (filter state, sort order, "N of M shown"), clickable. Turned off with `strip=0`.
- Equip compare band under the item description.
- Config page: Options > Mod Configuration > Inventory Sort & Filter.

## Configuration (inventory_sort_filter.ini)
| Key | Default | Meaning |
|---|---|---|
| `sort` | `1` | 0 = vanilla order, 1 = sorted |
| `sort_order` | `category` | `category`, `base`, `name`, `vanilla` (or 0..3) |
| `tint` | `1` | 1 = tint items the shown character cannot equip |
| `cycle` | `1` | 1 = re-clicking All cycles the filter |
| `cycle_states` | `new,usable,upgrade` | Filter states in the cycle |
| `strip` | `1` | 1 = own strip above the list; 0 = list untouched |
| `banner_shows_filter` | `1` | 1 = top line reads "Filter: New" etc. (needs `cycle=1`) |
| `compare` | `1` | 1 = equip compare band; 0 = hidden |

Rollback: set a flag to 0 (applies at the next inventory refresh), or remove the patch id. The ini is re-read at most once a second; no restart needed.

## Logging
- Shared KpLog logger: `inventory-sort-filter_log.txt` in the game folder.
- Level set in `kplog.ini` section `[inventory-sort-filter]` (`info`, `debug`, `trace`), or in game under Options > Mod Configuration > Inventory Sort & Filter > Debug logging.

## Hooks
- `kotor2-steam-aspyr.hooks.toml`: inventory tab handler, list build, and strip/click windows (H6a-H6f), validated against the Steam/Aspyr executable by the project workspace.
- Design and evidence: project design doc 11.

## Build/test
- Scoped tests: `python3 scripts/run_tests.py inventory_sort_filter`
- Fork readiness lint: `python3 scripts/fork_lint.py inventory_sort_filter`

## Changelog pointer
Version history is the `description` field in `manifest.toml`.
