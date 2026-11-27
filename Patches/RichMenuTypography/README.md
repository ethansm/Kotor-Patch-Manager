# Rich Menu Typography

> **STATUS** Track: SHIPPABLE. Version 0.4.5 (dev, not play-verified; matches manifest.toml).
> **What it does / explores:** Multi-colour and multi-font typography for KotOR II item descriptions and character stat menus, plus a Mod Config page with live preview.
> **Requires:** Steam/Aspyr KOTOR II exe (SHA `6A522E71...`, see manifest.toml). No hard dependencies. Mod Config (k2.modconfig@1, k2.menuui@1) is optional and connected lazily at runtime. Install through Patch Manager (KPM).
> **Known limits:** The core RichDescBox path is live since 0.3.0. The ItemNameTitle row depends on the P2a/P2a2 name probes. Preview build-failure paths and the 600-poll timeout have no unit test. **Testing:** the core path is live-tested on Steam; the item name title, stat menu typography and Mod Config preview still need an in-game check.
> **Install:** Install through KPM. Every ini key is optional; copy `rich_menu_typography.ini` next to `swkotor2.exe` only to edit defaults by hand.

## Features
- Inline per-glyph colouring in item descriptions and stat labels.
- Rich description box with semantic colours (`SemanticColors`).
- Section dividers between header, rules and body (`DividerMode`).
- Stat hierarchy styling (`StatHierarchy`).
- Item-name title row in the description box (`ItemNameTitle`).
- Mod Config page with live preview (optional).

## Configuration (ini keys, `[General]`)
| Key | Default | Meaning |
|---|---|---|
| `InlineColoring` | 1 | Per-glyph colour detour |
| `RichDescBox` | 1 | Rich description box path |
| `StatHierarchy` | 1 | Stat menu hierarchy styling |
| `SemanticColors` | 1 | Semantic colour set |
| `DividerMode` | 1 | 0 none, 1 header/rules and rules/body, 2 also title/subtitle |
| `ProbeItemName` | 0 | Log-only P2a probe of the selected item name (capped at 160 lines). Off by default. |
| `ItemNameTitle` | 1 | Description title row shows the selected item name (needs RichDescBox=1) |
| `BodyFont` / `RulesFont` | fnt_d16x16 | Description fonts |
| `Verbose` | 0 | Per-glyph and per-box diagnostics |
| `ProbeRmt` | 0 | RMTPROBE logging on every populate |

## Logging
Uses KpLog (`Common/KpLog.h`). Probe output is off by default; `Verbose` and `ProbeRmt` add more lines.

## Hooks
Hook addresses are listed in `kotor2-steam-aspyr.hooks.toml`. No other mod shares these hook addresses.

## Build
From this folder run `python3 ../create-patch.py`. Host unit tests, including vanilla-exe byte checks, live in the author's development workspace and are not shipped here.

## Changelog
- No CHANGELOG.md.
