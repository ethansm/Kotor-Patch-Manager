# QH-T03
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| QH-2a-03 | PARTIAL | patch_manager_mods/queue_hud_src/QueueHudLogic.h:776 (paused/now void-cast); patch_manager_mods/queue_hud_src/QueueHud.cpp:1377,1388,1400,1411 | yes | Paused/now ignored and debounce skipped confirmed; two-frame exhaustion needs engine-latency runtime proof. |
- patch_manager_mods/queue_hud_src/QueueHudLogic.h:776 reInjectWhy does `(void)paused; (void)now;`; neither arg is read in its body.
- patch_manager_mods/queue_hud_src/QueueHudLogic.h:775-784 reInjectWhy body references no kVanishDebounceMs or kReInjectSpacingMs; RI_OK returned at 783 after earlier checks.
- patch_manager_mods/queue_hud_src/QueueHudLogic.h:729-736 shouldReInject is the bool twin: same void-casts (730), same check order (parity stated at 770).
- patch_manager_mods/queue_hud_src/QueueHudLogic.h:765 kVanishDebounceMs appears only in FlagGate::step, not in re-inject logic.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1377 anchor matches exactly; call passes s.paused and now; no paused check exists between 1377 and 1388.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1388-1410 RI_OK branch: probeRead gate only, then reInjectCount++ (1400), dispatchAction (1407), activeStartTick=now (1408).
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1411 `else if (s.round != 0)` is the only path to noteView/accrueEaten/activeVerdict (1414); RI_OK never tests s.round.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1400-1409 no buf.markDispatched call in block; QueueHud.cpp:1415 pauseReinject gates only the verdict branch, not RI_OK.
- patch_manager_mods/queue_hud_src/QueueHud.cpp:1402 QLOG "re-injecting eaten ... attempt=%d" fires per RI_OK, so two-frame exhaustion would show as attempt=1 then attempt=2.
- OPEN: actionInQueue, kMaxReInjectCount (brief assumes 2), dispatchAction internals (markDispatched?), and engine queue latency are outside the ranges; UNSURE.
