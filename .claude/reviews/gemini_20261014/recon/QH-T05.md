# QH-T05
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| QH-2a-05 | PARTIAL | patch_manager_mods/queue_hud_src/QueueHud.cpp:755 | no | Three of four gates (538,739,1626) already soft-aware; only 755 raw, effect needs shouldShow |

- QueueHud.cpp:538 paintLeader passes `g_combat != 0 && !g_softCombat` to qh::cancelVisible: soft combat is not treated as combat. Refutes the raw-g_combat claim for this gate.
- QueueHud.cpp:575 defines hardCombat = g_combat != 0 && !g_softCombat. Anchor text is present (spacing differs); the claim's "partial" note is a whitespace mismatch only.
- QueueHud.cpp:576-577 use hardCombat for qh::shouldShow (5-arg) and qh::panelHold, so the leader panel path is soft-aware.
- QueueHud.cpp:739 drawStrips gate `g_combat == 0 || g_softCombat` is equivalent to !hardCombat, so strip Cancel shows in soft combat. Refutes the claim for this gate.
- QueueHud.cpp:1626-1628 onFrameBegin defines hardCombat identically; cwHideAll runs only on hardCombat, so cancel windows stay up in soft combat. Refutes the claim for this gate.
- QueueHud.cpp:755 drawAll top gate uses raw `g_combat != 0` in a 6-arg qh::shouldShow with g_cfg.showInCombat. This is the only raw use in range and is literally true.
- QueueHud.cpp:755-758: if shouldShow returns false for combat with showInCombat off, drawAll returns early and hides leader, strips and Cancel in soft combat. Contradicts the 575 comment. Depends on shouldShow (out of range).
- QueueHud.cpp:576 (5-arg, hardCombat) and 755 (6-arg, raw g_combat) use different combat criteria for the same HUD. Inconsistent in code, impact unverified.
- Gemini's blanket swap to hardCombat is a no-op at 538, 739 and 1626 and unproven at 755. Not applied as-is.
- OPEN: qh::shouldShow body, default of cfg.showInCombat, and softBannerTick (outside read ranges) are unchecked.
