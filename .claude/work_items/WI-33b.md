# WI-33b Hotbar portrait target and guiinput wheel fallback
area: hotbar-shared-services
size: M
ready: hold
after: WI-33
items: C197,C198,C200
files: patch_manager_mods/action_hotbar_src/ActionHotbar.cpp; patch_manager_mods/_shared_src/PortraitSlots.h; patch_manager_mods/_shared_src/GuiInput.h
out-of-repo: none
blockers: after: WI-33 (both edit ActionHotbar.cpp). C197: user answer on portrait click-to-target (doc 22:284). C198: portrait target UI not built (doc 22:220). C200: waits on A1 `k2.guiinput` (doc 22:221).
docs: patch_manager_mods/22_hotbar_shared_services.md rows 3 (:57, :220), 4.4 (header :199)

## Goal
Close rows C197, C198 (row 3, deduped) and C200 (row 4.4) as their gates clear. Every row is gated, so start no code now.

## Steps
1. C197 + C198 (row 3): after the user answer (doc 22:284) and the portrait target UI (doc 22:220), in `ActionHotbar.cpp` consume `k2.portraitslots@1` from `_shared_src/PortraitSlots.h`, polled from the HUD tick. Draw the target badge via HUD_DRAW on the Gui2D path (doc 22:57). `badgeFor` (`ActionHotbar.cpp:1124`) covers hotbar buttons only.
2. C200 (row 4.4): after A1 `k2.guiinput` ships, remove the window-subclass wheel fallback in `ActionHotbar.cpp` (`WheelTag`/`WheelHook` at :1167, `ensureWheelHook` at :1194). Keep the existing `k2.guiinput` client (`_shared_src/GuiInput.h`).
3. Run Verify. Close only the rows whose step landed.

## Verify
1. `python3 scripts/run_tests.py <hotbar filter; suite name unknown>`
2. `python3 scripts/build_check.py`
3. `python3 scripts/compat_check.py <touched _shared_src files>` (only if PortraitSlots.h or GuiInput.h is edited)

## Docs
- After each landed row: `python3 scripts/plan_manager.py mark-done <id> --commit <sha>` for ids 3 and 4.4 in patch_manager_mods/22_hotbar_shared_services.md. Whether mark-done resolves doc 22 ids is unknown.
- CHANGELOG: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C197 patch_manager_mods/22_hotbar_shared_services.md:220 id=3 blocked
- C198 patch_manager_mods/22_hotbar_shared_services.md:57 id=3 parked
- C200 patch_manager_mods/22_hotbar_shared_services.md:221 id=4.4 parked
