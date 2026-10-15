# CO-T10
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| CO-2d-02 | PARTIAL | patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1091,1103-1105,1231 | yes | Leader tooltip, show and hover not gated by popover overlap; clamp and z-order unverified. |
| CO-2d-01 | REFUTED | patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1229-1232 | n/a | Max writes 4 companion + 2 leader + 1 popover = 7, below 8. Cross-thread read unchecked. |

- CompanionSelfHeal.cpp:1091 - leader Hidden branch hides b0/ring0/disc0/frm0 and clears g_popMi; equivalent of CO-2d-02 anchor (anchor :1056 is outside range).
- CompanionSelfHeal.cpp:1087 - popover cleared for leader only in Down state; no overlap check.
- CompanionSelfHeal.cpp:1095 - leader disc visibility driven by b0.hover; not gated on popover.
- CompanionSelfHeal.cpp:1103-1105 - leader wTip, wShow(true) and wInert (Down only) run every frame when not Hidden; no popover check.
- CompanionSelfHeal.cpp:1186-1191 - popover opens when g_popMi>=0; layout from popLayoutFor (body outside range, left clamp unverified).
- CompanionSelfHeal.cpp:1194 - popover row hover from cursor; no effect on leader button in range.
- CompanionSelfHeal.cpp:1229 - companion loop mi=1..2 adds at most 2 rects each (max 4).
- CompanionSelfHeal.cpp:1230 - leader adds 2 rects (btn, ring); running max 6.
- CompanionSelfHeal.cpp:1231-1232 - popover adds 1 rect; g_nShown = ns, max 7 < 8.
- CompanionSelfHeal.cpp:720 anchor not read (outside range); g_shown capacity and cross-thread readers unverified.
