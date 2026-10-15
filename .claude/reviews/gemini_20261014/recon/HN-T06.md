# HN-T06
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HN-6n-01 | PARTIAL | patch_manager_mods/npc_world_src/NpcWorldProvider.cpp:159-160 | yes | Unprobed vtbl read and vtbl[0] dtor call are real. Object-table dangling part is UNSURE. |

- NpcWorldProvider.cpp:147-148: p from gameaddr::New, null-checked at 148 (returns 0). Anchor at 159 matches exactly.
- NpcWorldProvider.cpp:150: creature ctor called on p with kPlaceholderId; return value not captured.
- NpcWorldProvider.cpp:156-157: LoadTemplate result masked to low byte; zero enters failure branch.
- NpcWorldProvider.cpp:159: vtbl read directly from p with a plain dereference, no saferead, no null check on vtbl.
- NpcWorldProvider.cpp:160: DtorFn called through vtbl[0] with flag 1, no check that vtbl[0] is non-null or valid.
- NpcWorldProvider.cpp:161: returns 0 right after the dtor; p is not touched again in range (no use-after-free here).
- NpcWorldProvider.cpp:168: AddToArea runs only after a successful load, so the failure path has no area registration to undo.
- NpcWorldProvider.cpp:170: same function uses saferead::readAt for the object id, so the helper is in scope but not used at 159.
- Outside range (UNSURE): whether kAddrCreatureCtor adds an object-table entry and whether DtorFn(flag 1) removes it.
- Fix direction: probe vtbl and vtbl[0] via saferead, skip the dtor on failure (leak is safer than a bad call). Does not cover table cleanup.
