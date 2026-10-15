# SH-T17

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3c-04 | PARTIAL | patch_manager_mods/_shared_src/actqueue_test.cpp:90,101; patch_manager_mods/_shared_src/ActQueue.h:98,120 | yes | Gaps real in-range (rn spellId, truncation); playerPaused needs fixed-address stub; QueueHud unverified |
- actqueue_test.cpp:101 comment says playerPaused reads fixed AppGlobal, "not host-testable", defers to QueueHud tests. Anchor matches exactly.
- ActQueue.h:68 comment gives the fixed pause-read addresses 0x51C720 / 0x538CD0.
- ActQueue.h:69-76 playerPaused walks the AppGlobal chain; any unreadable link returns false (not paused).
- actqueue_test.cpp:60-103 no call to playerPaused anywhere in the read range; only read() cases at 84-100.
- actqueue_test.cpp:90 asserts rn[0] subType, itemId, featId, ptr; no rn[0].spellId assert.
- actqueue_test.cpp:79 fixture sets rd1 RnSpell=3, so the value is built but never checked.
- actqueue_test.cpp:91 cur.spellId is asserted (21); rn spellId is the only unasserted spell read in range.
- actqueue_test.cpp:70-80 fixture holds 2 queue nodes and 1 round node; no list longer than kMaxEntries or kMaxRound.
- ActQueue.h:98 and :120 loops stop at kMaxEntries / kMaxRound, the only bound; constant values are outside the read range.
- ActQueue.h:82-86 readRNode writes out only after the sub-type read succeeds; a failed read leaves the slot unchanged.
