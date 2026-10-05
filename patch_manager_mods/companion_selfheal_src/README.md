# Companion Self-Heal 0.6.0-dev

Target: KOTOR II Steam (Aspyr) build, sha256 `6A522E71...`, with Kotor Patch Manager. Steam only. Requires the
`portrait-slots` and `hud-host` patches (interfaces `k2.portraitslots@1`, `k2.hud@1`).

## What it does

Adds a small cross button beside each companion's vitality bar on the HUD portrait cards.

- Grey cross: auto mode is off for that companion.
- Rotating gold ring around the cross: auto mode is on.
- Left click: use the best healing item on that companion now.
- Right click: open a popover with **Auto** on/off, **Use below %**, and **Keep** (the reserve of items auto will not touch).
- Droids use repair kits instead of medpacs.
- The leader's card has no button.

## Rule

- The item is the smallest one that covers the companion's missing HP (rules in `HealItems.h`). If none covers it,
  the largest is used.
- The engine's `CanUseItem` check runs first, and the companion must be commandable.
- A 2 second debounce applies per companion.
- Never while the game is paused or a menu is open.
- A second use is refused while one is still queued.
- A queued heal on a follower is rotated to the head of that follower's action queue (`[queue] kick=1`). Without
  this the follower's FollowLeader action starves the heal.
- Manual clicks ignore the **Keep** reserve. Auto only uses an item while the party holds more than **Keep** of it.

## Install

1. Install `portrait-slots` and `hud-host` with Kotor Patch Manager.
2. Install the `.kpatch` (id `companion-selfheal`).
3. Start the game. `companion_selfheal.ini` in the game folder is optional; it is created/updated when you change a
   popover value.

## Quick start

1. Load a save with at least one companion in the party (the leader has no button).
2. Left click the cross beside a hurt companion's vitality bar to heal now.
3. Right click it, switch **Auto** on, and set **Use below %** and **Keep**. The ring turns gold.

## Settings

Settings live in `companion_selfheal.ini` in the game folder (next to `swkotor2.exe`). They are GLOBAL, not per save:
one set of values for every playthrough.

| Section / key | Default | Meaning |
|---|---|---|
| `[General]` `Log` | 1 | 1 = write the log file, 0 = off |
| `[General]` `ProbeKey` | 0x48 (`H`) | Key used with Ctrl+Shift for the diagnostic dump |
| `[queue]` `kick` | 1 | 1 = rotate a queued heal to the head of the follower's queue, 0 = do not rotate |
| `[probe]` `call` | 0 | 1 = enable the Ctrl+Shift+CallKey test use (diagnostic) |
| `[probe]` `CallKey` | 0x4A (`J`) | Key for the test use when `call=1` |

Per-companion rows (written by the popover; edit by hand only if you must) use `[defaults] default=` and
`[companions] <tag>=` with the form `auto:0|1,hp:NN,keep:N`. `hp` is clamped 5-95 (default 35), `keep` 0-20
(default 2), auto default off. Other lines in the file are preserved.

## Limits

- Out of combat or while paused, the use still goes through the engine's action queue, so it fires when the queue runs.
- The companion must be commandable (alive, under your control).
- Auto only works for companions whose card is shown on the HUD.
- Auto fires at most every 6 seconds per companion and is checked about every 250 ms.
- A companion's max Force Points reading 0 is a Shield Bar issue, not this patch.
- Settings are global (see Settings), not stored in the save.

## Logs

`companion-selfheal_log.txt` in the game folder. Tags: `USE`, `DONE`, `REFUSE`, `KICK`, `TIMEOUT` (plus `BUILD`,
`CLICK`, `AUTO`, `CFG`). Check it with:

```
python3 scripts/selfheal_log_check.py
```

`KICK starved` and `TIMEOUT unqueued` are warnings, not failures.
