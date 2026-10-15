# CO-T08
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| CO-2d-05 | PARTIAL | patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:998 (refreshCache call) precedes :1001-1027 (seen reset and set); anchor exact at :872 | no | Ordering is true; lag is one frame, not a cache-period delay. Simple reorder breaks c.have/c.m dependency. |
- patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:998: refreshCache runs only when cache invalid or older than CacheMs, before the slot loop in the same uiFrame call.
- patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1001-1002: seen reset to false for g_card[] and g_card0 at start of slot loop, after refreshCache.
- patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:872: member auto-heal reads g_card[i].seen; anchor matches exactly; value is from the last uiFrame that set it.
- patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:896: leader auto-heal reads g_card0.seen; same ordering as members.
- patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1020: member seen = !isLeader, set only on client-or-server id match; default false.
- patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1027: leader seen = true, set only when k==0 and g_card0.have.
- patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1017-1019: match uses c.have, c.m.client, c.m.server, which refreshCache sets at :858-860; blocks a simple reorder.
- patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:997: gate-closed path returns before the slot loop, so seen is not refreshed while the gate is closed.
- patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:866: comment says auto-heal runs at cache cadence (250 ms); seen lag is one uiFrame, not one cache period.
- Not read (outside range): heal::shouldAutoUse semantics for seen, and hideAll() seen reset on gate close.
