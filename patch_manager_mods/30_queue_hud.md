# 30 — Queue HUD (`queue-hud` 0.1.0-dev): green out-of-combat action queue, leader

## RESUME HERE
- State: **INSTALLED 2026-10-04 22:44** (user GO). Stage `$K/queuehud_stage_1004` (STAGE_INFO head=cd0eba3; DIRTY = sibling .bak/CharEffectsPanel only;
  PROBLEMS 1 = known 4gb artifact), 34 ids applied, verify mismatches 0, live dll `75defebf`, config `56e83f2e` -> `bdadbf1e` (appended block only).
  Backup `$G/backups/queuehud010_install_20261004_2244` (rollback: restore it). No INI = defaults. Awaiting the user play test (checklist below).
- Repo commits: `e7fa879` (RE + logic/tests), `66b3702` (DLL), `e4fc966` (Wine smoke), this doc. KPM `ethansm/local-mods` `cd0eba3` (`Patches/QueueHud`).
- Gate: `pre_install_gate.sh --mod queue_hud_src` GATE PASS (113/113 default tier), smokes `queuehud` 6/6.
- Offline next (user 10-04 22:50 "build strips and do L3"): companion strips 0.2.0-dev (build + gate, NOT install) + doc 24 L3 save-events library (not installable until hotbar migrates).
- Next: user runs the checklist below, then "check queue-hud log" (KpLog file `$G/queue-hud_log.txt` or per KpLog naming).

## What it does
When NOT in combat mode, draws a green copy of the vanilla queue panel (4 slots: slot 0 big, 1-3 small) at the vanilla `LBL_QUEUE0..3`
extents, for party member 0. The vanilla red panel still handles combat. Companion strips near the portraits are a later iteration.

## Design
- Hook H8 `0x0074C168` (12 B `8B 8D 94 FE FF FF 8B 91 04 A9 00 00`), detour `QueueHudCapture([ebp-0x16C] = hud)`: stores the hud,
  combat = `([hud+0xA904]>>1)&1` (an unreadable value counts as combat), tick. Read-only.
- Draw: PortraitSlots `K2PS_HUD_DRAW` prio 400. Show rule `qh::shouldShow`: enabled, !combat, !inMenu, capture < 250 ms, >= 1 slot.
- Data: `party::serverCreature(member 0)` (same chain as vanilla `0x74DC80`, RE.md §3). Queue = inline CExoLinkedList at `cr+0x100`
  (cap 16). Round `R=[cr+0x10DC]`: list `[R+0xB00]` (pointer, cap 4), current `[R+0xB18]`.
- Slots: `qh::buildSlots` (QueueHudLogic.h). Mirrors vanilla, with these deviations:
  - 0x3F shows without vanilla's `[R+0xA84] && [R+0xB24]` gate; that gate is why nothing shows out of combat.
  - Only the first 0x3F expands the round list, and a node equal to `cur` is deduped.
  - A 0x3F with no nodes and no `cur` shows nothing; vanilla would show `i_attack`.
  - Nodes with a blank icon take no slot.
- Icons: the engine's own `0x74E0B0` (ECX ignored, 6 args, `RET 0x18`), cached per key. Type 0x0F spell id is pre-validated against
  `[[0xA1B4D0]+0x104]` {count, data}, because the engine's 0x0F path does not NULL-check the row.
- Skipped icons compact: the next icon takes the next slot position (no empty big slot).
- Waiting: `paused || [R+0xAB8]>=1 || [R+0xAA8]!=0` dims slot 0 (alpha 0.6). Pause bit = `[[[[0xA1B4A4]+8]+4]+0x10078] & 2`
  (the plan's chain was missing the `+4`; confirmed from `0x51C720` / `0x538CD0`).
- INI `queue_hud.ini` (game dir, optional): `enabled, bg_rgb, bg_alpha, border_rgb, border_alpha, border_px (1-6), waiting_dim, waiting_alpha, log`.
- Log channels (KpLog): `main`, `cap` (hud ptr changes), `icon` (resolved / blank / bad spell id, once per key), `draw` (shown/hidden edges + slot0 rect + hudScale).

## Open / to check live
- Slot rects come from `guikit::getExtent` with no hudScale applied. Check the first `draw shown` line (slot0 rect, hudScale).
- Do hidden vanilla labels still report a valid extent out of combat? If not, the log says `extent invalid`.
- The atlas tool `check_atlas_windows.py` has no QH row. H8 was checked OK (no overlap) with a scratch driver only.
- `split_build/QueueHud.kpatch` (local, git-ignored) is needed by the Wine smoke.

## In-game checklist
1. Pause out of combat, queue a medpac: a green slot appears.
2. Unpause: it clears when the action fires.
3. Enter combat: green hides, red vanilla shows.
4. Two queued commands: two slots, slot 0 dimmed while waiting.
5. Log: `python3 scripts/kotor2_logs.py check <queue-hud log> --new`.
