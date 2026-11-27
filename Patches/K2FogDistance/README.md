# K2 Fog Distance

> **STATUS** Track: SHIPPABLE. Version 0.1.1-dev (matches manifest.toml).
> **What it does / explores:** Scales each KOTOR II area's distance fog start and end (default 200%, up to 400%, or 0% = No Fog) so distant scenery stays visible. Each area keeps its own fog colour and shape.
> **Requires:** Steam/Aspyr build 6A522E71 (see `manifest.toml` `[patch.supported_versions]`); Patch Manager (KPM). No other mod required. Optional: mod-config (k2.modconfig@1) and menuui (k2.menuui@1) for the in-game page.
> **Known limits:** Only scales the scene fog fields; shadow distance is not touched. Fog-less areas are left to KPM Shader Fixes (see Notes). **Testing:** live-tested on Steam; the fog-less-area interaction still needs an in-game check.
> **Install:** Install through KPM like any other patch. Keep the mod enabled for normal play.

## Features
- Scales area fog start (`fog_start_pct`) and end (`fog_end_pct`) from the values the engine set for that area.
- Records the engine-set range when the area loads or a script sets it, so a new area is never mistaken for the mod's own last write.
- Reloads `k2_fog_distance.ini` while the game runs.
- In-game page "K2 Fog Distance" under Options -> Mod Configuration, when mod-config is installed.

## Configuration
File: `k2_fog_distance.ini` in the game directory (created with defaults on first run). Section `[Fog]`.

| Key | Default | Meaning |
|---|---|---|
| `enabled` | `1` | `0` turns the scaling off (vanilla fog). |
| `fog_start_pct` | `200` | Percent applied to the area's fog start. |
| `fog_end_pct` | `200` | Percent applied to the area's fog end. Max 400. `0` = No Fog. |
| `verbose` | `0` | `1` logs every area fog change, not only new origins. |

## Logging
Log file: `k2-fog-distance_log.txt` (KpLog). Load line: `k2-fog-distance <version> loaded: ...`. INI reloads log as `INI reload #N`.

## Hooks
| Address | Function | Purpose |
|---|---|---|
| `0x0046A989` | `Scene::DoFog` (`K2FogOnDoFog`) | Rescales Scene +0x64 / +0x68 when fog is on. |
| `0x00464C8A` | `Scene::SetFogRange` (`K2FogOnSetFogRange`) | Records the engine-set area fog as the origin (added in 0.1.1). |

Both addresses are listed in the hooks file `kotor2-steam-aspyr.hooks.toml`. Neither overlaps another mod's hook.

## Build
From this folder run `python3 ../create-patch.py`.

## Notes
- Fog-less areas: the engine path for them is owned by KPM Shader Fixes (`0x0046AADD`). If meshes in fog-less areas look fogged, that interaction is tracked as a known interaction, not as a fault in this mod.

## Changelog
- 0.1.1-dev: `Scene::SetFogRange` hook records engine-set area fog as the origin (fixes a collision with the R12 1.5a range).
- 0.1.0-dev: area fog scaling and the Mod Configuration page.
