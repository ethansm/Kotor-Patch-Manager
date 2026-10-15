# HN-T08
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HN-6n-03 | UNSURE | patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:247; patch_manager_mods/npc_world_src/NwCore.h:224,231,236-240 | n/a | In-range: engine tag never reaches seen_; reload seeding in onCreatureAdded unread |

- NpcWorldProvider.cpp:227 is NW_OnAddToArea (H3 0x00568BD0 per :226); the anchor text matches :247 exactly.
- NpcWorldProvider.cpp:244-249: tagStr is zeroed, and filled only when both readAt steps (0x24 then 0x18) succeed; otherwise it stays empty.
- NpcWorldProvider.cpp:249 stores tagStr in ci.tag; :255 logs it in the ADD line. No in-range code compares it with any spawn tag.
- NpcWorldProvider.cpp:258 passes ci to g_world.onCreatureAdded; that body is outside the ranges, so it is the only possible consumer.
- NwCore.h:224: spawn() returns TagPresent when the row tag is non-empty and tagPresent(row tag) is true; checked before the cap at :225.
- NwCore.h:231: after a successful engine spawn, rememberTag() records the spawn-argument tag. The engine-read tag is never passed to it (rememberTag body is outside range).
- NwCore.h:236-240: tagPresent scans seen_ only; not found returns seenOverflow_, so overflow yields fewer spawns, not duplicates.
- K1SpawnProbe.cpp:328: dumpTag falls back to libTag when the engine tag is empty; used for dump labels only.
- K1SpawnProbe.cpp:329-336: harvest and TAGSCAN triggers drive dumps and logs; no spawn or dedupe call in range.
- OPEN: does onCreatureAdded (or a load reset, outside ranges) seed seen_ from ci.tag on reload? If yes, an empty engine tag could allow duplicates.
