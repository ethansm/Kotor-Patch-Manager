# Companion Self-Heal

> **STATUS** Track: SHIPPABLE. Version 0.9.4-dev (matches manifest.toml).
> **What it does / explores:** Adds a heal button beside each companion's vitality bar. Left click uses the best healing item now. Right click opens a popover for Auto heal, the use-below % and the Keep reserve.
> **Requires:** `portrait-slots` and `hud-host` (interfaces `k2.portraitslots@1`, `k2.hud@1`); KOTOR II Steam (Aspyr) build, sha256 `6A522E71...`; Kotor Patch Manager. Optional: `mod-config` 0.2.0+ for the options page.
> **Known limits:** Mouse Look inversion is not handled (TODO CS8, `CompanionSelfHeal.cpp`). Settings are global, not per save. **Testing:** the heal request cases and the options page still need an in-game check.
> **Install:** Install `portrait-slots` and `hud-host` with KPM, then the `companion-selfheal` patch.

## What it does

Adds a small cross button beside each companion's vitality bar on the HUD portrait cards.

- Grey cross: auto mode is off for that companion.
- Rotating gold ring around the cross: auto mode is on.
- Left click: use the best healing item on that companion now.
- Right click: open a popover with **Auto** on/off, **Use below %**, and **Keep** (the reserve of items auto will not touch).
- Droids use repair kits instead of medpacs.
- The leader's card has a button too (outside the portrait, left of it).

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
   popover value or an options page value.

## Quick start

1. Load a save with at least one companion in the party.
2. Left click the cross beside a hurt companion's vitality bar to heal now.
3. Right click it, switch **Auto** on, and set **Use below %** and **Keep**. The ring turns gold.

## Options page

When `mod-config` 0.2.0+ is installed, the same values appear as a **Companion Self-Heal** page in the Options menu. The page edits the same global defaults and per-companion rows as the popover and saves to `companion_selfheal.ini`. Without `mod-config` the page is not registered and nothing else changes.

## Settings

Settings live in `companion_selfheal.ini` in the game folder (next to `swkotor2.exe`). They are GLOBAL, not per save:
one set of values for every playthrough.

| Section / key | Default | Meaning |
|---|---|---|
| `[General]` `Log` | 1 | 1 = write the log file, 0 = off |
| `[General]` `ProbeKey` | 0x48 (`H`) | Key used with Ctrl+Shift for the diagnostic dump |
| `[queue]` `kick` | 1 | 1 = rotate a queued heal to the head of the follower's queue, 0 = do not rotate |
| `[queue]` `trace` | 1 | 1 = write the QDIAG queue trace lines, 0 = silence them |
| `[probe]` `call` | 0 | 1 = enable the Ctrl+Shift+CallKey test use (diagnostic) |
| `[probe]` `CallKey` | 0x4A (`J`) | Key for the test use when `call=1` |

Per-companion rows (written by the popover or the options page; edit by hand only if you must) use `[defaults] default=` and
`[companions] <tag>=` with the form `auto:0|1,hp:NN,keep:N`. `hp` is clamped 5-95 (default 35), `keep` 0-20
(default 2), auto default off. Other lines in the file are preserved.

## Limits

- Out of combat or while paused, the use still goes through the engine's action queue, so it fires when the queue runs.
- The companion must be commandable (alive, under your control).
- Auto only works for companions whose card is shown on the HUD.
- Auto fires at most every 6 seconds per companion and is checked about every 250 ms.
- Mouse Look inversion is not handled (TODO CS8): the flag semantics invert when Mouse Look is set.
- A companion's max Force Points reading 0 is a Shield Bar issue, not this patch.
- Settings are global (see Settings), not stored in the save.

## Build

From this folder run `python3 ../create-patch.py`. Host unit tests live in the author's development workspace and are not shipped here.

## Logs

`companion-selfheal_log.txt` in the game folder. Tags: `USE`, `DONE`, `REFUSE`, `KICK`, `TIMEOUT` (plus `BUILD`,
`CLICK`, `AUTO`, `CFG`, `QDIAG`). `DONE ... reason=wiped` means an outside Cancel emptied the follower's whole queue while the use was pending (ended after 400 ms, no `TIMEOUT`). `REFUSE cast queued` means a cast/use is queued behind follow on that creature (not "any cast"; an executing cast does not count).`KICK starved` and `TIMEOUT unqueued` are warnings, not failures.
