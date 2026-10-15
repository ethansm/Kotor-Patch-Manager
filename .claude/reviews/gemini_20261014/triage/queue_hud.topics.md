# queue_hud triage (set queue_hud, bundles 2a, 2b; prefix QH)

16 findings (2a: 11, 2b: 5), 11 topics, 0 dropped. Gemini confidence stated "certain" for all; not judged here.

- QH-T01 P2: actionInQueue from raw s.haveCur before curReal (stale node deadlock) [2a-01]
- QH-T02 P2: engineNe/queueCountOf count resting 0x3D / bare 0x3F -> buffering + engine_busy forever [2a-02]
- QH-T03 P2: reInjectWhy/shouldReInject ignore paused/now, bypass debounce, burn reInjectCount [2a-03]
- QH-T04 P2: handoffTick midFlight ignores round nodes subType 9/10; may ServerClearAllActions in-flight cmd [2a-04]
- QH-T05 P2: soft-combat gates use raw g_combat in four places (Cancel button hidden) [2a-05]
- QH-T06 P2: party rotation (IDENT_SWAP) not reconciled before PerformUseItem/CastSpell or in combat handoff [2a-06]
- QH-T07 P2: transient g_combatUnreadable resets g_prevCombat -> false combat edge [2a-07]
- QH-T08 P3: cwLookCompanion fill=2 latch; drawChevron dead code; Config::kit default vs ini [2a-08, 2a-09]
- QH-T09 P2: SafeRead::readAt 32-bit wrap; embtex::heal premature "giving up" log [2a-10, 2a-11] (shared lib)
- QH-T10 P3: test gaps: haveCur=true, waiting_dim_rgb, chevron/strip/combatBg small-extent guards [2b-01, 2b-04, 2b-05]
- QH-T11 P3: vacuous section C in testFollowerWipeProtection; reHits[7]/dsHits[6] undersized vs enums [2b-02, 2b-03]

## Duplicates (within batch) and how folded
- None dropped. Related but kept distinct: 2a-01 and 2a-02 (both tickItemQueues, different root cause: curReal gating vs engineNe counting); 2a-06 and 2a-07 (both at tickItemQueues start / combat early-return, different causes). 2a-08 and 2a-09 folded into T08 as minor low-risk items; 2a-10 and 2a-11 folded into T09 (shared-lib, same reader). 2b-01/04/05 folded as same "branch not covered by tests" shape; 2b-02/03 as "test itself defective".
- Anchor check notes (transcription only): 2a-03 text says shouldReInject/reInjectWhy "ignore paused and now ((void)paused; (void)now;)", but the worktree signature at QueueHudLogic.h 729/775 does take `bool paused = false, uint32_t now = 0`; the recon should read the body. 2b-03 claims ReInjectWhy has 8 enumerators and DispatchWhy 7; QueueHudLogic.h 771-772 agree on counts (RI_OK..RI_EXECUTED = 8; DS_OK..DS_FLAG_BUSY = 7).
- 2a-07: queue_hud src line 86/1808 note g_combat is "forced to 1" when unreadable; relevant to the recon question.

## Findings citing files outside the bundle manifest
- None. All cited files are in 2a/2b manifests (SafeRead.h and EmbedTex.h in 2a deps). 2a-03 names `ActQueue.h` notes (in manifest). 2a-09 cites queue_hud.ini (in manifest). 2b-04/05 cite QueueHudLogic.h (in 2b manifest, dirty=1). Gemini also mentions test target `common_contract_test` (not in bundle; see Backlog).

## Cross-batch candidates
- patch_manager_mods/_shared_src/SafeRead.h: QH-2a-10 (readAt overflow). Likely flagged by shared_lib_a (3a/3b) and any batch that includes SafeRead.h.
- patch_manager_mods/_shared_src/EmbedTex.h: QH-2a-11 (heal give-up log). Same.
- patch_manager_mods/_shared_src/ActQueue.h: QH-2a-02 relies on its comments (FollowLeader / bare 0x3F) but is a queue_hud logic claim; companion batch (2c/2d) may flag the same engineNe semantics.
- patch_manager_mods/queue_hud_src/QueueHud.cpp and QueueHudLogic.h carry uncommitted edits from another session (dirty=1): read worktree as-is.

## Backlog (not assessed / omitted, not topics)
- Omitted from 2a bundle: _shared_src/GuiKit.h, _shared_src/K2Addr.h (manifest-omit), _shared_src/HudKitTextures.inc (over max_file_tokens), queue_hud_src/override/lbl_miqueue_teal.tga (binary).
- External headers kpatch_api.h and KpLog.h not provided in 2a.
- 2b: all files assessed; note that test.toml references a `common_contract_test` target whose source is not in the bundle.
