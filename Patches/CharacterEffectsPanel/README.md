# Character Active Effects Panel

> **STATUS** Track: SHIPPABLE. Version 0.6.0-dev (matches manifest.toml).
> **What it does / explores:** Adds an Effects/Stats panel to the right third of the character page. Effects lists the shown character's active effects (Harmful first, then Beneficial); Stats shows combat stats (deflection, defense, resist, attack, crit, Force DC, alignment).
> **Requires:** Patch Manager (KPM) with the `gui-input` provider (`k2.guiinput@1`). Target: Steam/Aspyr KOTOR II, supported hash in `manifest.toml` `[patch.supported_versions]`.
> **Known limits:** Stats attack value is unverified in general (play row UX-TAB-P6). Influence row is hidden by default (`show_influence=0`, a spoiler). Hover tooltips and Stats tab still have pending play rows. **Play-test status:** ST5b-P1, ST6e-P1, UX-TAB-P1, UX-TAB-P5 pending in `PLAY_TEST_QUEUE.md`.
> **Install:** Install as patch `character-effects-panel` through KPM. Copy `chareffects.ini` into the game folder to change settings (hot-reloaded about once a second).

## Features
- Effects tab: icon, name, remaining time (`m:ss`, or `h:mm:ss` at one hour or more; `Permanent`; `--` when unknown). Harmful rows come before Beneficial rows. The list scrolls when it overflows.
- Stats tab: toggled by clicking the panel header (`stats_tab`).
- Hover tooltips on rows (`tooltips`), including on the vanilla character sheet (`vanilla_tips`).
- Long names scroll (marquee) after a delay (`marquee*` keys).

## Requirements and install notes
- Needs the `gui-input` provider. It is declared in `manifest.toml` (`requires`, `requires_interfaces`).
- Only the Steam/Aspyr build is supported (hash in `manifest.toml`).
- Config file: `chareffects.ini` in the game folder. Unknown or bad keys are logged and ignored.

## Configuration (chareffects.ini keys)
| Key | Line | Default | Effect |
|---|---|---|---|
| `tooltips` | :9 | 1 | Hover tooltip on Effects rows |
| `vanilla_tips` | :46 | 1 | Hover tooltip on vanilla character sheet rows |
| `stats_tab` | :42 | 1 | Click header to switch Effects / Stats; 0 = header stays "Active Effects" |
| `show_influence` | :44 | 0 | Influence row on Stats tab for party companions (spoiler) |
| `deflect_check` | :40 | 1 | Diagnostic: logs deflection totals (see Diagnostics) |

Other keys (`stage`, `poll_ms`, `stat_column`, `marquee*`, `skip_equipped`, `fake_rows`, and more) are documented inline in `chareffects.ini`.

## Logging
- Core file: `character-effects-panel_log.txt`. Old core fallback: `chareffects_log.txt` (INFO).
- Detail: set `level=debug` for `[character-effects-panel]` in `kplog.ini`. Line caps apply (`MaxLogLines`, `MaxDebugLines`).

## Hooks
Defined in `kotor2-steam-aspyr.hooks.toml` (4 hooks, Steam/Aspyr hash):
- `0x0084D871` BindCharEffects (detour, character page constructor)
- `0x0084FBB0` CharEffectsTick (detour, per-frame update)
- `0x0084DEF0` CharEffectsDestroy (detour, panel destructor)
- `0x006E2351` CharEffectsDeflectCheck (detour, DIAGNOSTIC; logs only)

## Diagnostics
- **CharEffectsDeflectCheck (`0x006E2351`) is DIAGNOSTIC.** It logs how the engine's blaster-deflection total compares with `CombatStats.h` (first 30 attacks, then mismatches). It does not change gameplay. Set `deflect_check=0` to turn it off; the hook then returns at once.
- The raw attack-byte logger in `CharEffectsPanel.cpp` is also DIAGNOSTIC. It logs combat-info attack bytes when they change, capped per session.

## Build and test
Scoped host tests (run from repo root):

```
python3 scripts/run_tests.py --id chareff_decode_test --id combat_stats_test --id stats_rows_test --id tab_model_test --id vanilla_tips_test --id cpp:patch_manager_mods/active_effects_panel_src/common_contract_test.cpp
```

Known gap: no test compiles `CharEffectsPanel.cpp` (about 1450 lines). Logic is split into testable headers (`ChareffDecode.h`, `StatsRows.h`, `TabModel.h`, `VanillaTips.h`, `CombatStats.h`).

## Files
- `manifest.toml`, `kotor2-steam-aspyr.hooks.toml`, `chareffects.ini`
- `CharEffectsPanel.cpp` (panel, ini, logging, hooks), `ChareffDecode.h`, `EffectNameTable.inc`
- `StatsRows.h`, `TabModel.h`, `VanillaTips.h`, `CombatStats.h`, `CombatStatsAddr.h`
- Tests: `*_test.cpp`, `testdata/`, `research/`

## Changelog
See `CHANGELOG.md`.
