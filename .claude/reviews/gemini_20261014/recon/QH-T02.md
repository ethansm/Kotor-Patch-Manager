# QH-T02
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| QH-2a-02 | PARTIAL | patch_manager_mods/_shared_src/ActQueue.h:110; patch_manager_mods/queue_hud_src/QueueHud.cpp:782; patch_manager_mods/queue_hud_src/QueueHud.cpp:1475 | no | Count half confirmed; DS_ENGINE_BUSY blocking and makeEngineView are out of range, unverified |

- ActQueue.h:98-112: read() walks every queue node; ActQueue.h:110 `++s.ne` runs for each node regardless of type, so 0x3D and 0x3F are counted.
- ActQueue.h:27-28: 0x3D documented as a permanent resting action re-added by its handler; 0x3F is item use. Comments only, not verified in code.
- ActQueue.h:52-53: useQueued excludes a bare 0x3F with an empty round list; the raw ne count has no equivalent filter.
- QueueHud.cpp:775-782: queueCountOf returns the unfiltered s.engineNe (or -1 when gather fails).
- QueueHud.cpp:1474: comment defines the idle gate as engineNe == 0 && !haveCur && nrn == 0.
- QueueHud.cpp:1475: dispatchWhy receives raw s.engineNe; anchor text matches exactly.
- QueueHud.cpp:1485-1490: WAIT trace logs dsWhy and s.engineNe; logging only, no gating effect.
- If a follower's 0x3D always rests, engineNe >= 1 and the 1474 gate never opens; this needs dispatchWhy (out of range) to resolve.
- Not read (outside ranges): dispatchWhy, makeEngineView, decideAction, tickItemQueues bodies. makeEngineView is not defined in ActQueue.h.
- Grep-only hits outside ranges (QueueHud.cpp:237, 968-970, 1343-1358) seen incidentally; not relied on.
