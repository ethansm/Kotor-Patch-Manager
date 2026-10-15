# HN-T07

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HN-6n-02 | PARTIAL | patch_manager_mods/npc_world_src/NwCore.h:176-180; NwCore.h:161-162 | no | Seen-tag non-clear confirmed; TagPresent refusal unverified (spawn() unread); fix alters per-visit dedupe scope |
| HN-6n-04 | REFUTED | patch_manager_mods/npc_world_src/NwCore.h:174-176; nw_world_test.cpp:173-174 | n/a | Same-resref pointer change is intended no-re-arm; test asserts same area, stable() stays true |

- NwCore.h:155-168 onLoad: gateReset(), area_ cleared, areaPtr_=nullptr, seenCount_=0, seenOverflow_=false, spawnedThisVisit_=0. Only in-range clear of seen tags.
- NwCore.h:171-175 onCreatureAdded: areaPtr_ assigned whenever ci.areaPtr and non-empty ci.areaRes are present; no resref check on this path.
- NwCore.h:176-180 resref-change branch: setArea, gateReset, spawnedThisVisit_=0; no seenCount_/seenOverflow_ touch. Spawn counter resets, seen tags do not. HN-6n-02 anchor matches except spacing before the comment.
- NwCore.h:270 gateReset(): clears stableRun_, stableFired_, addsSinceTick_. Stable gate IS re-armed on resref change, so the gate half of HN-6n-02 is refuted.
- NwCore.h:197-199 onTick: stable run counts only while areaPtr_ set and !stableFired_. Pointer-only change calls no gateReset, so stableFired_ stays set.
- NwCore.h:276-284 rememberTag: dedupes with ciEqualsN, sets seenOverflow_ at MaxSeenTags; the only writer of seen_ in range.
- nw_world_test.cpp:165-171: resref change A->B asserts !stable(), spawnedCount()==0, area updated. Seen tags are not asserted.
- nw_world_test.cpp:170-174: pointer B->A with "261tel" -> "261TEL" (case-variant) asserts stable() still true. Test pins no re-arm for same resref.
- nw_world_test.cpp:172: confirms g_areaB was the prior pointer before the case-variant add.
- Unread (outside ranges): spawn() and its TagPresent path, resrefEquals definition, any seen_ reset past NwCore.h:290.
