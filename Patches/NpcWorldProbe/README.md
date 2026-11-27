# NPC World Probe

> **STATUS** Track: DIAGNOSTIC. Version 0.1.0-probe (matches manifest.toml).
> **What it does / explores:** Hookless diagnostics client of npc-world (k2.npcworld@1). It spawns test NPCs through the library's Spawn() and logs evidence on hook-spawn save/load survival, capacity 1/6/18 and frame-tick windows around the spawns.
> **Requires:** npc-world loaded first (manifest.toml:8-9, requires = ["npc-world"]); interface k2.npcworld@1; Steam/Aspyr exe with LAA (manifest.toml:12); KPM with the DLL-only patch loader.
> **Known limits:** Steam/LAA only. Owns no detour: the three world hooks come from npc-world. Conflicts with area-hook-probe (manifest.toml:10; both use address 0x00533830). Testing: no live checks recorded yet.
> **Install:** Do not install for normal play. Install only when testing npc-world, with npc-world installed and loaded first, and never together with area-hook-probe.

## Features
- Stages (ProbeCore.h:76-82): `t26` (old k1-spawn-probe roster, NB0 live check), `pool` (ini rows), `capacity` (N copies of one template on a grid), `persist` (hook-spawn survival across save/load), `frame` (tick-interval windows before and after the spawns).
- `all` enables all five stages (t26, pool, capacity, persist, frame). `none` is the explicit opt-out: the probe spawns nothing.
- Without the ini file the default stages are `t26,persist`.

## Configuration (npc_world_probe.ini, [probe])
Copy `npc_world_probe.ini.sample` to the game folder as `npc_world_probe.ini`. All keys are optional.
- `stages=` comma list: t26, pool, capacity, persist, frame, all, none.
- `capacity_n`, `capacity_resref`, `capacity_realtag`, `capacity_area`, `capacity_x`, `capacity_y`, `capacity_z`, `capacity_step`, `capacity_cols`. `capacity_n` is required for `stages=capacity`; without it nothing spawns.
- `frame_window`, `frame_pre_windows`, `frame_post_windows`.
- `row1`..`row16` for `stages=pool`: `resref,tag,area,x,y,z[,facing]`. The tag must be the template's own tag.

## Logging
- Channel `npc-world-probe` in kpatch.log, with channels core, stage, persist, frame and spawn (configure in kplog.ini).
- Fallback file: `npc-world-probe_log.txt` in the game folder.

## Hooks
- None owned. hooks.toml only comments the addresses 0x0055A460, 0x00568BD0 and 0x00533830. The sole detour owner is K2NpcWorld (hooks.toml:41).

## Build
- From this folder run `python3 ../create-patch.py`. Host unit tests live in the author's development workspace and are not shipped here.

## What was learned
- V2a results (2026-10-13) were host-only.

## Changelog pointer
- No CHANGELOG.md yet; version history is in manifest.toml.
