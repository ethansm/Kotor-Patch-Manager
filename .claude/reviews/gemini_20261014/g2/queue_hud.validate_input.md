# G2.3 skeptic input: queue_hud (13 fids)

## QH-T01 (P2): In tickItemQueues, is actionInQueue set true from raw s.haveCur/s.cur.subType 9|10 (QueueHud.cpp ~1347) before/without gating on qh::curReal, so a stale flagless node (>=1500 ms, aa8==0&&ab8==0, unpaused) still yields ev.actionPresent=true and AV_PRESENT forever?
- **QH-2a-01** v2=PARTIAL (G1 PARTIAL) | claimed high logic | actionInQueue uses raw haveCur before curReal; stale flagless node deadlocks buffer | file patch_manager_mods/queue_hud_src/QueueHud.cpp fn tickItemQueues
  - G1 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:1347, :1358, :1355; patch_manager_mods/queue_hud_src/QueueHudLogic.h:1288 | G1 note: Raw gate real; stall needs out-of-range classifier. Fix would loosen QH4 busy guard.
  - v2 evidence: patch_manager_mods/queue_hud_src/QueueHudLogic.h:1251,1233,1256,1258; patch_manager_mods/30_queue_hud.md:110 | v2 notes: [gem2 PARTIAL] u033: Rule 1 ungated; no in-range release path; raw haveCur source and reInjectWhy unverified

## QH-T02 (P2): Does queueCountOf (QueueHud.cpp 775-795) and the dispatchWhy call (~1475) count a companion's resting 0x3D / bare 0x3F entry in engineNe (while makeEngineView ignores them), so decideAction buffers the first command and dispatch blocks forever at DS_ENGINE_BUSY?
- **QH-2a-02** v2=PARTIAL (G1 PARTIAL) | claimed high logic | engineNe counts resting 0x3D/bare 0x3F, forcing buffering and permanent engine_busy | file patch_manager_mods/queue_hud_src/QueueHud.cpp fn queueCountOf/tickItemQueues
  - G1 evidence: patch_manager_mods/_shared_src/ActQueue.h:110; patch_manager_mods/queue_hud_src/QueueHud.cpp:782; patch_manager_mods/queue_hud_src/QueueHud.cpp:1475 | G1 note: Count half confirmed; DS_ENGINE_BUSY blocking and makeEngineView are out of range, unverified
  - v2 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:1866, :1869, :1871 | v2 notes: [gem2 PARTIAL] u034: Count reaches dispatch gate in range; 0x3D inclusion and busy block are out of range

## QH-T03 (P2): With buf.active && !actionInQueue && hasFollowLeader && reInjectCount<2, does reInjectWhy/shouldReInject (QueueHudLogic.h 729-800) return RI_OK without consulting paused/now or kVanishDebounceMs/kReInjectSpacingMs, and does tickItemQueues act on RI_OK before the s.round!=0 debounce branch without calling buf.markDispatched?
- **QH-2a-03** v2=UNSURE (G1 PARTIAL) | claimed high logic | reInject ignores paused/now, bypasses debounce and exhausts reInjectCount in two frames | file patch_manager_mods/queue_hud_src/QueueHud.cpp,patch_manager_mods/queue_hud_src/QueueHudLogic.h fn tickItemQueues; shouldReInject/reInjectWhy
  - G1 evidence: patch_manager_mods/queue_hud_src/QueueHudLogic.h:776 (paused/now void-cast); patch_manager_mods/queue_hud_src/QueueHud.cpp:1377,1388,1400,1411 | G1 note: Paused/now ignored and debounce skipped confirmed; two-frame exhaustion needs engine-latency runtime proof.
  - v2 evidence: patch_manager_mods/queue_hud_src/QueueHudLogic.h:672,675-677,685-693; patch_manager_mods/queue_hud_src/QueueHud.cpp:914-943 | v2 notes: [gem2 UNSURE] u035: No in-range refire gate; reInjectWhy and RI_OK branch outside ranges; literal markDispatched fix clears eaten latch.

## QH-T04 (P2): Does handoffTick's midFlight test (QueueHud.cpp ~1117) look only at s.e[] types 0x2E/0x0F and not at s.rn/s.cur subType 9|10 (cf. engineHoldsAction), allowing clearBeforeHandoff to call ServerClearAllActions on an in-flight active command?
- **QH-2a-04** v2=UNSURE (G1 PARTIAL) | claimed high logic | handoffTick midFlight ignores round-node subType 9/10; can ServerClearAllActions an in-flight command | file patch_manager_mods/queue_hud_src/QueueHud.cpp fn handoffTick
  - G1 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:1116-1118 (midFlight); :1131-1140 (clear) | G1 note: Gap real: midFlight reads only e[]. In-flight kill unproven; headHeld wait covers held head. Fix adds unbounded wait.
  - v2 evidence: patch_manager_mods/30_queue_hud_evidence/R7_cancel/dump/0051cb10.c:13-19; patch_manager_mods/30_queue_hud_evidence/R7_cancel/dump/00590430.c:8-15 | v2 notes: [gem2 UNSURE] u036: In-range clear never resets cur (+0xB18) or aa8/ab8; kill hinges on out-of-range callees.

## QH-T06 (P2): Is the IDENT_SWAP identity reconciliation in tickItemQueues run only when !inMenu && g_combat==0, so that PerformUseItem/PerformCastSpell index g_itemQueue[memberK] before swap and the in-combat early-return path calls handoffTick/dispatchAction without checking a.creature against the live party member?
- **QH-2a-06** v2=PARTIAL (G1 PARTIAL) | claimed high logic | Party rotation not reconciled before buffer indexing or in-combat handoff; wrong creature dispatched | file patch_manager_mods/queue_hud_src/QueueHud.cpp fn QueueHudPerformUseItem/PerformCastSpell/handoffTick/bufOwner
  - G1 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:1256-1259, :1272-1294, :1869-1885 | G1 note: Gate and unchecked index are real; wrong-creature outcome needs handoffTick and rotation facts.
  - v2 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:1091-1093,1144; patch_manager_mods/30_queue_hud.md:114 | v2 notes: [gem2 PARTIAL] u037: Clear/gating use live slot creature, dispatch uses buffered creature; no compare; combat rotation unproven

## QH-T07 (P2): Given g_combat is forced to 1 when g_combatUnreadable (QueueHud.cpp 86, 1808), does tickItemQueues line 1253-1255 set g_prevCombat[k]=false on an unreadable frame so the next readable in-combat frame produces a false rising edge into handoffTick/endHandoff?
- **QH-2a-07** v2=CONFIRMED (G1 CONFIRMED) | claimed medium logic | Transient g_combatUnreadable overwrites g_prevCombat, firing false combat rising edge | file patch_manager_mods/queue_hud_src/QueueHud.cpp fn tickItemQueues
  - G1 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:1253,1255,1257 | G1 note: Unreadable frame writes g_prevCombat=false (1255, unguarded); next readable in-combat frame gets spurious rising edge into handoffTick (1257).
  - v2 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:1253,1255,1257 | v2 notes: 

## QH-T08 (P3): Does cwLookCompanion latch w.fill==2 with no retry (QueueHud.cpp 428-440), is drawChevron (351) never called anywhere in queue_hud_src, and does Config::kit default true (QueueHudLogic.h 184) contradict queue_hud.ini's documented kit=0 default?
- **QH-2a-08** v2=PARTIAL (G1 PARTIAL) | claimed low logic | cwLookCompanion latches text-fallback fill=2 permanently, never retries texture | file patch_manager_mods/queue_hud_src/QueueHud.cpp fn cwLookCompanion
  - G1 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:432,437 | G1 note: Latch real (432/437); intentional per comment 427 \\\"(once)\\\"; lifetime needs out-of-range reset check
  - v2 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:402 | v2 notes: [det PARTIAL] d042: Fill cleared on hide (402) and HUD rebuild (498); not permanent; no-retry-per-episode stands.
- **QH-2a-09** v2=CONFIRMED (G1 PARTIAL) | claimed low logic | drawChevron unused (dead code, -Wunused-function); Config::kit default true vs ini kit=0 | file patch_manager_mods/queue_hud_src/QueueHud.cpp,patch_manager_mods/queue_hud_src/QueueHudLogic.h,patch_manager_mods/queue_hud_src/queue_hud.ini fn drawChevron; Config::kit
  - G1 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:351; patch_manager_mods/queue_hud_src/QueueHudLogic.h:184 | G1 note: Line 184 documents default-on intent; fixing ini, not default, is the sound direction
  - v2 evidence: patch_manager_mods/queue_hud_src/QueueHud.cpp:351 | v2 notes: [det CONFIRMED] d043: drawChevron has only its definition; no caller in queue_hud_src. || [det PARTIAL] d044: ini:8 documents kit=0 default; Config default (QueueHudLogic.h:184) not in output.

## QH-T10 (P3): Does queue_hud_logic_test.cpp lack any call with haveCur=true to decideAction/decideItemAction, any 'waiting_dim_rgb' ini case, and any chevronRect/stripCancelRect/combatBgRect case with q0 smaller than 8x10 or zero height?
- **QH-2b-01** v2=CONFIRMED (G1 UNSURE) | claimed medium test-gap | No test passes haveCur=true to decideAction/decideItemAction | file patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp fn testDecideBranches
  - G1 evidence: patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1275,1295,1300 | G1 note: haveCur absent from ranges; true 2nd-arg calls exist (comments say combat); decideItemAction out of range
  - v2 evidence: no-hits | v2 notes: [det CONFIRMED] d045: No 4-arg call found; regex skips args containing parens, so nested-call args could be missed.
- **QH-2b-04** v2=PARTIAL (G1 PARTIAL) | claimed low test-gap | waiting_dim_rgb key and slotStyle bg unasserted in tests | file patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp,patch_manager_mods/queue_hud_src/QueueHudLogic.h fn testIniLine/testConfigDefaultsAndStyle
  - G1 evidence: patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:334-408 | G1 note: waiting_dim_rgb absent from testIniLine and testIniBad; slotStyle bg and testConfigDefaultsAndStyle out of range
  - v2 evidence: no-hits | v2 notes: [det PARTIAL] d046: waiting_dim_rgb has no test parse; slotStyle half refuted per rule note.
- **QH-2b-05** v2=PARTIAL (G1 PARTIAL) | claimed low test-gap | chevronRect/stripCancelRect/combatBgRect minimum-size and zero-height guards untested | file patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp,patch_manager_mods/queue_hud_src/QueueHudLogic.h fn testChevronRect/testStripCancel
  - G1 evidence: patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:862,872 | G1 note: all-zero rect tested at 862/872; nonzero small-size clamp inputs absent; testStripCancel out of range
  - v2 evidence: patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:862 | v2 notes: [det PARTIAL] d047: Chevron clamp untested (3 calls only); combatBgRect/stripCancelRect tested per rule.

## QH-T11 (P3): In queue_hud_logic_test.cpp, do the section-C asserts of testFollowerWipeProtection (~1064) only test the test's own local mutation, and are reHits[7]/dsHits[6] (~1429-1467) smaller than the 8 ReInjectWhy / 7 DispatchWhy enumerators with indexing reHits[why]/dsHits[why] reachable out of range?
- **QH-2b-02** v2=CONFIRMED (G1 CONFIRMED) | claimed medium test-gap | Follower-wipe section C asserts only the test's own local mutation (vacuous) | file patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp fn testFollowerWipeProtection
  - G1 evidence: patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1064-1067 | G1 note: Test sets buf.active=false at 1065, then asserts it at 1067; no production call clears it.
  - v2 evidence: patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1064-1067 | v2 notes: 
- **QH-2b-03** v2=UNSURE (G1 PARTIAL) | claimed high test-gap | reHits[7]/dsHits[6] undersized vs 8 ReInjectWhy / 7 DispatchWhy enumerators; possible OOB | file patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp fn testWhyParityMatrix
  - G1 evidence: patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1429,1453; patch_manager_mods/queue_hud_src/QueueHudLogic.h:771-772 | G1 note: Arrays are smaller (7<8, 6<7); out-of-range reachability needs body past line 775.
  - v2 evidence: patch_manager_mods/queue_hud_src/QueueHudLogic.h:781,792; patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1399 | v2 notes: [gem2 UNSURE] u038: RI_EXECUTED needs sawExecuting, DS_FLAG_BUSY needs flagHold (in-range); enum values and array decls outside window
