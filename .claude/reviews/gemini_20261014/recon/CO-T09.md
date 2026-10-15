# CO-T09

| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| CO-2d-06 | CONFIRMED | patch_manager_mods/companion_selfheal_src/CompanionSelfHeal.cpp:1166-1180 | yes | Logging a noneed-style line is sound; closing the popover on a no-op click is a UX choice. |

- CompanionSelfHeal.cpp:1166 `} else if (onBtn >= 0) {` is an exact anchor match.
- CompanionSelfHeal.cpp:1167-1175: leader arm (onBtn==0) logs "CLICK member=0 noneed" for FullHp; the companion arm has no equivalent log.
- CompanionSelfHeal.cpp:1176: `else if (Card& c = g_card[onBtn]; shui::clickAllowed(c.st))` has no else arm, so a refused companion click does nothing.
- CompanionSelfHeal.cpp:1177: the NoItem log sits inside the clickAllowed arm, so it fires only if clickAllowed(NoItem) is true.
- CompanionSelfHeal.cpp:1180: `else if (popOpen) g_popMi = -1;` chains to the onBtn<0 case only; any onBtn>=0 click leaves g_popMi open.
- CompanionSelfHeal.cpp:1134 and 1143: onBtn is set only when !inPop, so onBtn>=0 with popover open means the cursor is outside the popover.
- CompanionSelfHeal.cpp:1149-1165: the inPop branch handles clicks inside the popover and is not on this path.
- CompanionSelfHeal.cpp:1181-1182: RMB toggles g_popMi on onBtn, separate from the LMB path.
- Popover-stays-open applies to every companion LMB on a button, allowed or not, so it is not specific to FullHp/Pending.
- Not checked: shui::clickAllowed results for FullHp and Pending (defined outside range).
