# HN-T11
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HN-6n-07 | PARTIAL | patch_manager_mods/npc_world_src/NwCore.h:89-90; patch_manager_mods/npc_world_src/nw_registry_test.cpp:63-73 | yes | In-range tests never unregister and register in one callback. Tokens are monotonic, so no current defect. Add the test. |

- NwCore.h:90 is `return nextToken++;`; anchor is exact. Line 89 stores the same token in the new slot `e[n++]`.
- NwCore.h:80 `nextToken` starts at 1 and only increments, so a token value is never reused after removal.
- NwCore.h:92-100 `remove` compacts `e[]` and decrements `n`; the removed token is gone.
- NwCore.h:101 `live()` matches by token value, not slot index, so a removed consumer's token never reads as live.
- NwCore.h:106-113 `dispatch` iterates a copy `snap` and skips any entry whose token fails `live()`; unregister-in-callback is safe.
- nw_registry_test.cpp:63-68 registers `cbRegisters` as consumer "r"; comment says the new consumer waits for the next event.
- nw_registry_test.cpp:69 `d.remove(g_tokNew)` runs in the test body, not inside a callback.
- nw_registry_test.cpp:70-73 `cbKillsB` unregisters B; B is not called. In-range, it does not also register.
- Not read (outside ranges): bodies of `cbRegisters`/`cbKillsB`, test file lines 1-54 and 101+. Whole-file absence unverified.
- The stated "slot-based token change" is hypothetical; current code uses a monotonic counter, not slots.
