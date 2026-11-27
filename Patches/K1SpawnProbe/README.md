# K1 Spawn Probe

> **STATUS** Track: EXPERIMENTAL. Version 0.3.9-probe (matches manifest.toml).
> **What it does / explores:** A DLL-only client of npc-world (k2.npcworld@1). Once per visit to the 204tel area, when npc-world reports the population stable, it spawns a small roster (a mouse droid and K1 NPC rows) through the library's Spawn(), then logs every creature AddToArea and dumps creature memory for the K1 probe tags. It is a research probe: can K1 character content be spawned natively into K2.
> **Requires:** npc-world (k2.npcworld@1, loads first via `requires` in manifest.toml); Steam/Aspyr build 6A522E71631DCEE93467BD2010F3B23D9145326E1E2E89305F13AB104DBBFFEF; the Patch Manager (KPM) install flow.
> **Known limits:** (1) Roster rows k1p_carth, k1p_mission, k1p_zaalbar and k1p_bastilla are optional: they need private K1 character packs that never enter the repo. Without them the row's Spawn fails and logs `SPAWN fail resref=<resref>`; the rest of the roster still runs. (2) Spawn points are hand-placed on the 204tele walkmesh (0.8 m clearance). (3) The roster is compiled into ClientLogic.h. (4) Only the pure logic is host-tested; K1SpawnProbe.cpp glue is not. **Testing:** not yet checked in game.
> **Install:** Do not install for normal play. It is a research probe that spawns extra creatures in 204tel. Install only for an in-game check, through the standard KPM flow.

## Features
- Per visit of the target area (204tel), after npc-world reports the population stable, spawns the roster rows for that area, skipping any row whose tag is already present (no duplicates on reload).
- Logs every creature AddToArea (id, area, position, tag, origin). Long ADDTOAREA lines are split into `+ ` pieces under the 232-byte core cut.
- Dumps creature memory for the K1 probe NPC tags (`k1p_*`, `MouseDroid`) through SafeRead.h, skipping unreadable blocks.
- Owns no hooks. The three world hooks (0x0055A460, 0x00568BD0, 0x00533830) belong to npc-world; KPM rejects a second owner.

## Configuration
- `k1_spawn_probe.ini` (game dir), section `[spawn]`, key `enabled`: `enabled=0` stops this client's own spawns. Observation (logging and memory dumps) stays on. Default is 1 when the file or key is missing.
- To silence everything, use npc-world's `npc_world.ini` `[world] enabled=0` instead.

## Logging
- Log file: `k1-spawn-probe_log.txt` in the game dir, written through KpLog.h (channel main), so it carries the `# kplog session` header.
- kplog.ini section name: `[k1-spawn-probe]`.
- Expected startup line: `k1-spawn-probe 0.3.9-probe loaded (client of npc-world, spawn enabled|disabled by ini)`.
- Spawn results: `SPAWN ok resref=...` or `SPAWN fail resref=...`.

## Hooks
- None owned by this mod. `kotor2-steam-aspyr.hooks.toml` holds only `[metadata]` and comments.
- The world hooks it depends on are owned by npc-world (see its README).

## Build
- From this folder run `python3 ../create-patch.py`.
- Host unit tests live in the author's development workspace and are not shipped here.

## Roster
| Resref | Area | Notes |
|---|---|---|
| c_drdmouse01 | 204tel | Pet: vanilla mouse droid at WP_com_ith_01 (tag MouseDroid) |
| k1p_juhani | 204tel | K1 Juhani (appearance 818) |
| k1p_carth | 204tel | Optional: character pack (appearance 819) |
| k1p_mission | 204tel | Optional: character pack (appearance 823) |
| k1p_zaalbaru | 204tel | Optional: character pack (appearance 824, tag k1p_zaalbar) |
| k1p_bastilla | 204tel | Optional: character pack (appearance 825) |

## What was learned (EXPERIMENTAL)
- 0.3.1 added a tag18 field to every ADDTOAREA line to chase a duplicate-spawn finding; the roster skips tags already present.
- Spawn points must sit on the walkmesh. The 0.3.6 point at -33.0 was outside 204tele.wok (user play 10-18); the 0.3.7 points at -40.5, -42.5 and -44.5 were checked against 204tele.wok with 0.8 m clearance.
- K1 Juhani sits 1.5 m from the pet's known-good point. A twin Juhani row (k1p_juhanv) was dropped in 0.3.9 after the character pack was uninstalled (10-20).

## Asset hygiene
- No game assets, no `.k2pack` contents and no k1p_* pack data are in this directory.

## Changelog
No CHANGELOG.md.
