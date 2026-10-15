# QH-T01

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| QH-2a-01 | PARTIAL | patch_manager_mods/queue_hud_src/QueueHud.cpp:1347, :1358, :1355; patch_manager_mods/queue_hud_src/QueueHudLogic.h:1288 | no | Raw gate real; stall needs out-of-range classifier. Fix would loosen QH4 busy guard. |

- QueueHud.cpp:1347 matches the anchor exactly: haveCur and subType 9|10 set actionInQueue with no curReal, pause, or staleCurSince check.
- QueueHud.cpp:1344 and :1349: engine entries and rn[] round nodes also set actionInQueue raw, ungated by curReal.
- QueueHud.cpp:1353-1354: staleCurSince starts only when haveCur, unpaused, aa8==0, ab8==0; any flag change resets it to 0.
- QueueHud.cpp:1355: creal = curReal(...) is computed but never feeds actionInQueue.
- QueueHud.cpp:1358-1360: makeEngineView receives actionInQueue as its actionPresent argument, plus creal inputs.
- QueueHudLogic.h:1265-1273: curReal false only if unpaused, aa8==0, ab8==0, subType 9|10, and now-staleSince >= kStaleCurMs (value defined outside range).
- QueueHudLogic.h:1288: ev.actionPresent copied straight from input with no curReal gate; :1289 executing and :1291 idle are gated, so actionPresent=true with idle=true is possible.
- QueueHud.cpp:1339-1340: busy test kept raw on purpose (QH4 ClearAllActions guard); BG23 limit covers only unrelated engine casts, not stale flagless nodes.
- QueueHud.cpp:1362-1368: STALE_CUR logs once per episode when !creal, while actionInQueue stays true.
- QueueHud.cpp:1374 and :1377: raw actionInQueue also feeds seenInQueue and reInjectWhy; the AV_PRESENT mapping lives above QueueHudLogic.h:1260 (out of range).
