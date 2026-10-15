# QH-T10
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| QH-2b-01 | UNSURE | patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:1275,1295,1300 | yes | haveCur absent from ranges; true 2nd-arg calls exist (comments say combat); decideItemAction out of range |
| QH-2b-04 | PARTIAL | patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:334-408 | yes | waiting_dim_rgb absent from testIniLine and testIniBad; slotStyle bg and testConfigDefaultsAndStyle out of range |
| QH-2b-05 | PARTIAL | patch_manager_mods/queue_hud_src/queue_hud_logic_test.cpp:862,872 | yes | all-zero rect tested at 862/872; nonzero small-size clamp inputs absent; testStripCancel out of range |

- cpp:1275 — anchor text matches exactly; decideAction(idle,false,0) expects DECIDE_DISPATCH_NOW.
- cpp:1295,1296,1300 — decideAction called with 2nd arg true; comments read "combat beats full" / "combat beats engine queue".
- cpp:1272-1300 — no identifier haveCur in range; function body continues past 1300, unseen; no decideItemAction call seen.
- cpp:334-382 testIniLine — keys covered: enabled, waiting_dim, log, *_alpha, bg/border_rgb, border_px; no waiting_dim_rgb.
- cpp:386-396 testIniBad bad-line list — no waiting_dim_rgb entry.
- cpp:410-420 testIniText — body continues past line 420; unseen beyond it.
- cpp:853-862 testChevronRect — inputs 40x40, 80x80, 0x0 only; 0x0 expects all-zero output at 862.
- cpp:865-872 combatBgRect — inputs 40x40, 80x80, 106x106, 0x0; 0x0 expects zeros at 872.
- No nonzero input with w<8 or h<10 in 852-873; the min-size clamp is unexercised here.
- QueueHudLogic.h:303 and :463 anchors lie outside the read ranges; not verified.
