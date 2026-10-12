# WI-33c Hotbar item-target move and FPH item table handoff
area: hotbar-shared-services
size: S
ready: hold
after: WI-35c
items: C201,C202
files: patch_manager_mods/action_hotbar_src/HotbarLogic.h; NEW:patch_manager_mods/_shared_src/Items.h; patch_manager_mods/_shared_src/K2Addr.h
out-of-repo: none
blockers: after: WI-35c (both edit K2Addr.h). C201: needs a second item-targeting consumer (doc 22:214); `Items.h` does not exist yet. C202: GameAddr.h has split-session edits (doc 22:249), and doc 22:249 lists K2Addr.h as "theirs". Intake format at doc 22:280.
docs: patch_manager_mods/22_hotbar_shared_services.md rows 4.5 (header :212, line :214), T3 (line :249, intake :280)

## Goal
Close rows C201 (row 4.5) and C202 (row T3) as their gates clear. Both rows are gated, so start no code now.

## Steps
1. C201 (row 4.5): only when a second item-targeting consumer exists (doc 22:214), create NEW `_shared_src/Items.h` and move `chooseTarget` (`action_hotbar_src/HotbarLogic.h:22`) into it. Until then, change nothing.
2. C202 (row T3): keep FPH item constants local. Hand the item table to `_shared_src/K2Addr.h` in the intake format (doc 22:280: Steam values, GOG zero). Do not edit `_shared_src/GameAddr.h` until the split-session edits land.
3. Run Verify. Close only the rows whose step landed.

## Verify
1. `rg -n 'chooseTarget' patch_manager_mods/action_hotbar_src/HotbarLogic.h` (before the move: expect :22)
2. `python3 scripts/run_tests.py <filter; suite name unknown>`
3. `python3 scripts/build_check.py`
4. `python3 scripts/compat_check.py patch_manager_mods/_shared_src/K2Addr.h` (plus `Items.h` if created)

## Docs
- After each landed row: `python3 scripts/plan_manager.py mark-done <id> --commit <sha>` for ids 4.5 and T3 in patch_manager_mods/22_hotbar_shared_services.md. Whether mark-done resolves doc 22 ids is unknown.
- CHANGELOG: unknown.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C201 patch_manager_mods/22_hotbar_shared_services.md:214 id=4.5 parked
- C202 patch_manager_mods/22_hotbar_shared_services.md:249 id=T3 parked
