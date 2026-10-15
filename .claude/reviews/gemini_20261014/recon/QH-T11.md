# QH-T11
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| QH-2b-02 | CONFIRMED | patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1064-1067 | yes | Test sets buf.active=false at 1065, then asserts it at 1067; no production call clears it. |
| QH-2b-03 | PARTIAL | patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1429,1453; patch_manager_mods/queue_hud_src/QueueHudLogic.h:771-772 | yes | Arrays are smaller (7<8, 6<7); out-of-range reachability needs body past line 775. |
- queue_hud_logic_test.cpp:1050 sets activeStartTick=1060; 1063 sets nowDone=1610, so 550 > 500 and the 1064 guard fires.
- queue_hud_logic_test.cpp:1053 and 1059 call shouldReInject; QueueHudLogic.h:775 reInjectWhy takes const ActionBuffer&, so these calls cannot clear active.
- queue_hud_logic_test.cpp:1064-1067 is the only write of buf.active=false in section C; the KT_CHECK at 1067 tests the test's own guard.
- queue_hud_logic_test.cpp:1025-1075 contains no production expiry call; the 500ms window is emulated inline by the test.
- QueueHudLogic.h:771 ReInjectWhy has 8 enumerators (RI_OK=0 .. RI_EXECUTED=7); queue_hud_logic_test.cpp:1429 reHits is int[7].
- queue_hud_logic_test.cpp:1451 coverage loop checks r<7 only, so RI_EXECUTED (index 7) is never asserted reachable.
- QueueHudLogic.h:772 DispatchWhy has 7 enumerators (DS_FLAG_BUSY=6); queue_hud_logic_test.cpp:1453 dsHits is int[6].
- queue_hud_logic_test.cpp:1454-1458 dispatch matrix varies only active, isEmpty, eng, cur, nrn; line 1467 checks r<6 only.
- Fix direction: QH-2b-02 should call the production expiry path (target not in range); QH-2b-03 resize to 8/7 and loop to 8/7 is correct, but coverage asserts may then fail.
