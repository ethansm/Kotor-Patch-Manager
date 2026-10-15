# QH-T07
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| QH-2a-07 | CONFIRMED | patch_manager_mods/queue_hud_src/QueueHud.cpp:1253,1255,1257 | yes | Unreadable frame writes g_prevCombat=false (1255, unguarded); next readable in-combat frame gets spurious rising edge into handoffTick (1257). |

- QueueHud.cpp:1253 `combatNow = g_combat != 0 && !g_combatUnreadable` is false on every frame where the HUD flags read failed.
- QueueHud.cpp:1255 computes edgeFlags[k] = combatNow && !g_prevCombat[k], then unconditionally sets g_prevCombat[k] = combatNow, so unreadable frames store false.
- QueueHud.cpp:1256-1257 on an unreadable frame g_combat is forced to 1, so the branch is entered, but combatNow is false, so handoffTick and combatEdgeTrace are skipped; the comment says no hand-off on unreadable frames.
- QueueHud.cpp:1807-1809 rd() runs every capture; unreadable sets g_combat=1 and g_combatUnreadable=true; a fail-then-succeed sequence is the normal path.
- Defect sequence: readable in-combat (prev=true) -> unreadable (prev forced false at 1255) -> readable in-combat (edgeFlags[k]=true at 1255) -> handoffTick(now, edgeFlags) at 1257.
- No spurious edge when the last readable frame before the gap was out-of-combat, because prev is already false.
- Anchor `const bool combatNow = g_combat != 0 && !g_combatUnreadable;` matches QueueHud.cpp:1253 exactly.
- Fix sketch (skip the g_prevCombat update while g_combatUnreadable): removes the false edge; trade-off is that a combat end and restart entirely inside a gap goes unseen.
- Not read (outside ranges): declaration of g_combatUnreadable (~line 86), handoffTick/endHandoff/combatEdgeTrace bodies, tickItemQueues call frequency, other g_prevCombat writers.
- Impact of the spurious edge in handoffTick/endHandoff is unverified (UNSURE part).
