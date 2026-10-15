# QH-T08
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| QH-2a-08 | PARTIAL | patch_manager_mods/queue_hud_src/QueueHud.cpp:432,437 | no | Latch real (432/437); intentional per comment 427 "(once)"; lifetime needs out-of-range reset check |
| QH-2a-09 | PARTIAL | patch_manager_mods/queue_hud_src/QueueHud.cpp:351; patch_manager_mods/queue_hud_src/QueueHudLogic.h:184 | no | Line 184 documents default-on intent; fixing ini, not default, is the sound direction |

- QueueHud.cpp:432 `if (w.fill == want || w.fill == 2) return;` is an exact anchor match; once fill==2 the texture branch is unreachable.
- QueueHud.cpp:433-435 retries texture via tex::image and guikit::setFill; success sets fill=want and returns.
- QueueHud.cpp:436-437 text fallback "X" then w.fill=2; no path inside cwLookCompanion moves fill off 2.
- QueueHud.cpp:438 warning logged once via g_cancelMissLogged; no retry logic exists in range.
- QueueHud.cpp:427 comment says "(once)", so the latch is documented intent, not an accident.
- QueueHud.cpp:351 drawChevron has no `static`; -Wunused-function fires only for unused static or anonymous-namespace functions, enclosing namespace not visible in range.
- QueueHud.cpp:351-368 drawChevron body: image lookup then scanline triangle fallback; exact anchor match at 351.
- QueueHud.cpp:441-444 cwLook dispatches to cwLookLeader or cwLookCompanion; drawChevron not referenced in range 345-445.
- QueueHudLogic.h:184 `bool kit = true;` with comment "(default on)"; enclosing struct name not visible in range 180-188.
- Out of range (OPEN): queue_hud.ini kit=0 claim, any CW.fill reset (QueueHud.cpp 376+), and any drawChevron caller elsewhere in queue_hud_src.
