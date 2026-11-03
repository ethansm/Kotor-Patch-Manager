# Changelog

Action Hotbar (id `action-hotbar`), formerly Force Power Hotbar.

## Unreleased (0.1.11-dev)

- When the bar reappears after a dialogue or menu, and the cursor is at or below the top of the bar (over a slot or on the vanilla quickbar under it), the cursor moves to the centre of the game window. Before, the cursor stayed where the dialogue left it. One move per reopen; logs `cursor: reopened at (x,y) below bar top y=N`.

Fixes from the Gemini review of TP1-TP3:

- Mode button: cycling a slot's target skips a mode another slot already holds for that item. Before, stepping through Lowest HP on the way to Self cleared the slot that held Medpac Lowest HP.
- The "Nobody needs healing" / "Not usable right now" tip stays its full 1.5 s when the slot was clicked. Before, the hover tip replaced it after one frame.
- A Lowest HP slot now dims exactly when a press would find nobody, including when self cannot take the item (full HP), and while the only hurt member has a pending heal.
- Energy shields and the Squad Recovery Kit no longer ask "use on?" or offer target modes (the game uses them directly).
- While "use on?" is showing, the arrows and the Mode button do nothing; Esc, a refused assign or another slot click also removes the choice tooltip.
- README: HealThreshold default 100 in the INI table, hook address 0x0072CB84, comments are kept on save, `spell:`/`feat:` refuse target suffixes.

## 0.1.10-dev

- The same item can now sit in several slots with different targets (Medpac, Medpac Self, Medpac Lowest HP). Before, picking it again moved it out of the other slot. Picking the same item with the same target still moves it. (TP3)
- Lowest HP now heals whoever is hurt the most: `HealThreshold` default 100 (was 50, which made the slot do nothing unless someone was below half HP). When nobody qualifies the slot shows "Nobody needs healing" and the log lists each member's HP. Existing INIs keep their written value. (TP3)

## 0.1.9-dev

- Picking a healing item, or one you can use on yourself, now asks "use on?" in the picker: Use normally / Lowest HP / Self. One click assigns the item with that target; Esc goes back to the list. Before, the item was assigned with the default target and had to be changed with the Mode button afterwards. (TP1)
- Target badges: Self is now a "Self" pill (was a small teal person icon), Lowest HP keeps the red cross, the default has none. The badge area is 23x12 at the slot's top-right (was 13x13) so the pill fits beside the key digit; the "use on?" cells use square versions of the same art. (TP2)
- Keys 1-0 are reserved for the hotbar while the HUD is up, also on an empty slot and in the editor, so they no longer fire the vanilla quick-action bar or switch party members. `[General] ReserveDigits=0` restores the old pass-through. Shift/Ctrl/Alt + digit still go to the game. (KD1)

## 0.1.8-dev

- Loading a save or starting a new game no longer does file work inside the game's load callback; the next HUD frame applies it (save-events warned 7.5-7.9 ms per load, limit 5 ms). A save made before that frame applies the load first. (PS5)
- The debug log no longer prints a slot line when only the reason changes on a widget that stays hidden (`hidden -> hidden (hud not active)`). (PS4)

## 0.1.7-dev

- Loading a save that has no hotbar copy keeps your current bar (WARN in the log) instead of emptying it. 0.1.6 emptied it, and a save whose copy was lost (the save event was missed) wiped the player's bar. Only a new game empties the bar. (PS1)
- 3 s after writing a save's copy the hotbar checks it is still in the folder and writes it once more if the game removed it. (PS3)
- Needs nothing new; save-events 0.1.1-dev additionally recovers saves whose folder name it could not read (PS2).

## 0.1.6-dev

- A save with no hotbar copy now really loads an empty bar (it used to keep the previous bar). A copy still waiting for its save folder is written before a load looks for it, and gives up after 10 s instead of about a second. (RH1)
- The bar scales correctly in 4:3 windows that are not 800x600 (1024x768, 1600x1200). (RH2)
- A failed config save no longer leaves the edited slot (or the duplicate it cleared) in memory. (RH3)
- Item slots on Lowest HP skip a member who already has a heal on the way, so two quick presses heal two members. (RH4)
- With `Invoke=0` the digits 1-3 keep switching party members; a digit pressed while the HUD is not active no longer leaves a stale "swallowed" flag. (RL23, RL29)
- An item on `@self` that the HUD shows as unusable goes through the vanilla path, and an item the game refuses shows "Not usable right now". (RL24)
- A config file that exists but cannot be read is retried every poll and editing is refused until it can be read; read errors are no longer taken for an empty file. (RL25)
- The last three load backups are kept (`.load.bak`, `.1`, `.2`). (RL26)
- Picker tooltips are hidden when the list scrolls, the tab changes or the mode button is clicked; the picker list rebuilds every 20 frames instead of every frame. (RL27, RL28)
- Modifier keys are resynced from the real keyboard state on each digit press. (RL29)
- Log wording: the stale-HUD warning says the digit passed to the game; the mouse-wheel subscribe retry is throttled to once per 60 frames; the stance read WARN can fire again after a recovery. (RL30, RL31)
- Debug log: `GATE rawtop=` names the topmost panel to diagnose digits swallowed in dialogue. (RH5, no behavior change)

## 1.3.0-dev (2026-10-05)

- Internal cleanup (no behavior change): spell description strref is parsed with a checked integer read, request-ring push is one tested helper, dead constant and magic numbers removed.
- `spells.2da` no longer has to be in `override/`: without it the patch reads the base game's `chitin.key` -> `data/2da.bif` (HP7). The log line names the source.
- `@self`/`@lowhp` on a `spell:`/`feat:` slot is refused with a warning (it only applies to items).
- `[Profile. Name ]` section names are trimmed.
- An unbound slot never shows a quantity or out-of-stock marker.
- Numbers in `spells.2da` cells read as decimal or `0x` hex only (no octal).
- The editor wheel scroll uses the same 250 ms HUD-alive window as the keys (was 300 ms).
- Heartbeat `wfaults=` also counts ignored widget setup results.
- Migrated 5 save/load/new-game detours to the `save-events` library (`k2.saveevents@1`, doc 22 §4.1a).
- Internal: pure logic moved to `HotbarLogic.h` (HP5), named constants and cleanup (HP6), more tests.
- WI-10: Force Power Hotbar polish and config round-trip.
- WI-10b: Force Power Hotbar EKC hook move and arrow art (split from WI-10).
- WI-11: Force Power Hotbar config and statics.
- WI-12: Force Power Hotbar arrow art and setFont check.

## 1.2.2-dev (2026-10-05)

- Internal: refactor (HP4), no behavior change.
- Internal: engine addresses from the shared tables, shared healing-item check (C6, CS7).

## 1.2.1 (2026-10-04)

Polish fixes.

- A per-save snapshot that cannot be read keeps the current hotbar. The log gets a warning.
- A save load is applied on the next HUD update, so no stale slots show after a load.
- Loads back up the working INI to `action_hotbar.ini.load.bak`. The first-edit `action_hotbar.ini.bak` is kept.
- The slot tooltip no longer shows `Quantity: Type`.
- A bad `TargetProbe` value logs one warning.
- INI header, comments and manifest description corrected. Tests added for the new helpers.

## 1.2.0 (2026-10-04)

Rename and migration.

- Force Power Hotbar is now Action Hotbar: patch id `action-hotbar`, DLL, source folder, INI (`action_hotbar.ini`)
  and log (`action-hotbar_log.txt`, `kplog.ini` section `[action-hotbar]`) are renamed.
- On the first start, `force_power_hotbar.ini` is copied to `action_hotbar.ini`. The old file is left as it is.
- Saves made before 1.2.0 hold `force_power_hotbar.ini` in the save folder. It is still read when the save has no `action_hotbar.ini`.

## 1.1.0 (2026-10-04)

- One hotbar per save (1.0.0 builds 25-26, 2026-10-02/03): copied into each save folder when you save and restored when you
  load. A save with no copy loads with an empty bar.
- Picker cells show item stack counts. Items you have run out of keep a dimmed icon with a count of 0.
- The vanilla action bar is refreshed after an AI stance or form change (`SyncActionBar`, default 1).
- Item target modes: `@lowhp[:pct[:self|none]]` and `@self`, with `HealThreshold` and `HealFallback`. The rules are read from the
  columns of `override/spells.2da`. `TargetProbe` selects off, log only, or live (default live).
- Editor: a target-mode button next to Clear, a badge in each slot's corner, dimming of `@lowhp` slots, and a "Target:" line in the slot tooltip.

## 1.0.0 (2026-10-02)

- Ten slots for keys `1` to `0` above the action bar. Powers, feats, items and stances/forms, used through the HUD's quick-action path.
- Self-contained: the DLL creates its controls and art at run time, with no `.gui` file.
- In-game editor behind a wrench button: tabs for powers, items and modes, scrolling picker, Clear, hover tooltips, Esc to back out.
- One hotbar per character, saved in `force_power_hotbar.ini`.
- Release defaults `Invoke=1`, `Verbose=0`. Logging through the shared KpLog logger.

Builds before 1.0.0 were development builds and are not listed.
