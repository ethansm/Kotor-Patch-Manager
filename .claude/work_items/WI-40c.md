# WI-40c Queue panel shows when paused with 0x3F (held for GO)
area: queue-hud
size: S
ready: hold
items: C229
files: patch_manager_mods/queue_hud_src/QueueHud.cpp; patch_manager_mods/queue_hud_src/QueueHudLogic.h
out-of-repo: none
blockers: after: WI-41 (both files are also edited by WI-41). C229 needs user GO. Doc 29:94 names H8 hook `0x74C168` for the HUD update. Split from WI-40. The row id is `-`, so the closure key is unknown.
docs: patch_manager_mods/29_paused_command_queue.md:94 (C229)
deferred: C229

## Goal
Show the queue panel when paused and slot 0 holds 0x3F (C229). Waits for user GO.

## Steps
1. Hold. Do not edit `QueueHud.cpp` or `QueueHudLogic.h` until the user gives GO for C229 (deferred). Rule on GO: show the panel when pause bit 2 is set and slot 0's queue holds 0x3F (doc 29:94). `isWaiting` already drives the waiting style.

## Verify
1. After GO and the edit: `python3 scripts/build_check.py`

## Docs
- C229 stays open (id `-`). Closure key unknown.
- CHANGELOG: none stated in TSV.

## Source rows (close these plan rows when done: item, plan doc:line, plan row id, status)
- C229 patch_manager_mods/29_paused_command_queue.md:94 id=- open
