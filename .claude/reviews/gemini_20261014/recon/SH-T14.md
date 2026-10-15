# SH-T14
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| SH-3b-05 | CONFIRMED | patch_manager_mods/_shared_src/SelfHealUi.h:382; patch_manager_mods/_shared_src/selfheal_ui_test.cpp:335-354 | yes | Gap real; the anchor sits in SelfHealUi.h:382. Test is right but fails: now==0 wraps and times out at once. |

- selfheal_ui_test.cpp:335-354: 16 pendingDone2 calls in range; smallest now is 1100 (lines 335-338); no now==0 call.
- selfheal_ui_test.cpp:337: now=1100, queued=false, u=0 returns true early via the item-gone check at SelfHealUi.h:379; never reaches :382.
- SelfHealUi.h:382: sentinel line matches the anchor text exactly; the `1u` arm of `now ? now : 1u` needs now==0.
- selfheal_ui_test.cpp:343, 347, 350: queued=false with u=0 reach :382 at now=10000, 10000, 15000; the 0 arm never runs.
- selfheal_ui_test.cpp:353-354: nullptr tracker path at now=7000/6999; does not touch :382.
- SelfHealUi.h:383: elapsed = (uint32_t)(now - *unqueuedSince); with now==0 stored as 1 this is 0xFFFFFFFF, so the return is true.
- SelfHealUi.h:375: comment states the 6 s timeout; selfheal_ui_test.cpp:345-346 (5999 false, 6000 true) pins the threshold at 6000 ms.
- Anchor check: anchor text is absent from selfheal_ui_test.cpp; it exists only at SelfHealUi.h:382.
- kPendingTimeoutMs is not in the read ranges; 6000 is inferred from the test, not read.
