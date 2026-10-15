# HB-T04
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HB-1a-05 | CONFIRMED | patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1199-1208 | no | Connected branch returns unconditionally; WheelHook fallback unreachable. Naive fallthrough can double-count wheel input. |

- ActionHotbar.cpp:1199 `if (g_guiInput.connected) {` matches the brief anchor exactly (anchor check at 1199 PASS; 898 and 2147 are outside my ranges, not read).
- ActionHotbar.cpp:1201-1207: static `subscribed` gates the retry; wheel_subscribe runs every frame until tok > 0.
- ActionHotbar.cpp:1208: bare `return;` inside the connected block, so the function exits every frame on subscribe failure.
- ActionHotbar.cpp:1205: only the success path logs; a persistent tok <= 0 produces no log line at all.
- ActionHotbar.cpp:1210-1217: `WheelHook::active()` check and `WheelHook::install(g_gameWnd, &wheelProc)` are reached only when not connected, so the fallback is unreachable while connected is true.
- ActionHotbar.cpp:1220-1221: the non-connected path throttles its failure logs (first 3 attempts, then KP_LOG_EVERY 60000); the connected path has no equivalent.
- GuiInput.h:49: wheel_subscribe returns <= 0 for null cb, table full, or game window not yet available; the last two can persist.
- ActionHotbar.cpp:1202: hotbarWheelCb is passed non-null, so the null-cb failure mode is ruled out.
- GuiInput.h:114-117: poll() returns early once connected, and connect() is not re-run, so `connected` never flips back to false.
- ActionHotbar.cpp:1193: the window-hook path accumulates into g_wheelAccum; hotbarWheelCb (outside range) is not read, so the double-count risk is unverified.

OPEN: Can the provider's wheel_subscribe fail persistently (table full, or game HWND never available) in practice? Needs GuiInput provider code and hotbarWheelCb (outside range).
