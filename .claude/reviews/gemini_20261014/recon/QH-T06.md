# QH-T06
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| QH-2a-06 | PARTIAL | patch_manager_mods/queue_hud_src/QueueHud.cpp:1256-1259, :1272-1294, :1869-1885 | yes | Gate and unchecked index are real; wrong-creature outcome needs handoffTick and rotation facts. |
- patch_manager_mods/queue_hud_src/QueueHud.cpp:867 anchor matches exactly. bufOwner returns activeAction.creature if active, else queue[0].creature; no party-table check.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1256-1259 returns early when inMenu or g_combat!=0. Only the combatNow branch runs combatEdgeTrace, handoffTick, roundWatchTick.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1257 combatNow requires !g_combatUnreadable, so unreadable combat flags skip handoffTick too.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1272-1294 identity pre-pass sits after that return, so it never runs in combat or menu. A swap moves g_itemQueue and g_lastOurs.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1295-1306 creature in no slot: skip, then after kEatenWaitMaxMs drop with DR_IDENTITY_LOST, only when not paused. Reached only outside combat.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1310-1325 endHandoff(DR_COMBAT_CLEAR) and sequential per-slot dispatch both run only on the non-combat path.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1859-1869 item path checks isPartyCaster(memberK), then decideAction(g_itemQueue[memberK], g_combat!=0, ...). No owner check in range; memberK source is above 1855, unread.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1874-1885 DISPATCH_NOW overwrites g_itemQueue[memberK].activeAction with crPtr; BUFFER pushes crPtr action into the same slot. Neither checks buffer owner.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1257 comment says combat path has "no hand-off and no drop", so a fix must not add a drop in combat; swap-only reconcile is the safe part.
- OPEN (unread): handoffTick and decideAction bodies; dispatchAction and PerformCastSpell not named in ranges; whether party rotation can occur in combat (engine knowledge).
