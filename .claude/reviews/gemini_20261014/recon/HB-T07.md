# HB-T07
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HB-1b-02 | PARTIAL | patch_manager_mods/action_hotbar_src/HotbarLogic.h:270 (no lower bound); patch_manager_mods/action_hotbar_src/ActionHotbar.cpp:1423 (called only when off > maxOff) | yes | Lower clamp is safe; HotbarLogic.h:274 scrollClamp already does [0,maxOff]. |
| HB-1b-03 | PARTIAL | patch_manager_mods/action_hotbar_src/HotbarLogic.h:29-30 (no cur check; negative cur passes); hotbar_logic_test.cpp not in range | yes | Test-only fix safe; a cur<=0 reject guard could drop real lowest-HP targets. |

- HotbarLogic.h:269-270: clampOffset is upper-only, and the comment says this is intentional (as the DLL did). Anchor matches exactly at :270.
- ActionHotbar.cpp:1422-1423: maxOff = pickerMaxOff, never negative (HotbarLogic.h:268); clampOffset runs only when ed.off > maxOff.
- ActionHotbar.cpp:1428-1430: wheel target goes through scrollClamp (HotbarLogic.h:274), so the wheel path never sets a negative offset.
- ActionHotbar.cpp:1440-1441: cell index is (size_t)ed.off + i; a negative offset wraps, but idx < size guards the read, so no OOB; cells shift.
- ActionHotbar.cpp:1434 and 1438: title range and prev dimming read ed.off directly; a negative value gives a wrong range and wrong prev state.
- Writers of ed.off outside the range (init, scrollPicker, prev/next handlers) were not read; negative reachability is unproven.
- HotbarLogic.h:29: skip guard tests ok, down, eligible, max<=0, but not cur; cur has no lower bound here.
- HotbarLogic.h:24-25 and :30: threshold clamped to 0..100 and max>0, so the right side is >=0; any cur<0 passes "strictly below". cur=0 is skipped only when threshold is 0.
- HotbarLogic.h:32-33: long long ratio compare ranks negative cur lowest, matching the "lowest HP% wins" contract at :21.
- HB-1b-03 anchor is NOT-FOUND in hotbar_logic_test.cpp (out of range, unread); equivalent code exists at HotbarLogic.h:32.
