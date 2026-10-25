# Action Hotbar 1.2.1

## Target

KOTOR II Steam (Aspyr) build, sha256 `6A522E71...`, with Kotor Patch Manager. Steam only.
Patch id `action-hotbar`, version 1.2.1. It was called Force Power Hotbar before 1.2.0.

## What it does

Adds ten slots above the action bar, bound to the keys `1` to `0`. A slot holds a Force power, a feat,
an item, or a stance/form. Pressing the key (or clicking the slot) uses it through the HUD's own
quick-action path, so the normal combat rules apply.

- A wrench button opens an in-game editor to fill the slots. Each character has its own hotbar.
- Item slots can target the party member with the lowest HP, or always the controlled character.
- The vanilla action bar is refreshed after an AI stance or form change.
- Each save keeps its own hotbar.

The DLL creates its own controls, art and INI. No `.gui` or texture files are installed.

## Install

1. Install `ActionHotbar.kpatch` (id `action-hotbar`) with Kotor Patch Manager.
2. Start the game. `action_hotbar.ini` is created in the game folder (next to `swkotor2.exe`) on first run.
   If you have an old `force_power_hotbar.ini`, it is copied first (see Migration).
3. For the item target modes, the patch needs `spells.2da` to decide which items can be targeted. It looks in
   `override/spells.2da` first, then in the base game's `chitin.key` -> `data/2da.bif`. No file needs to be copied.
   The log shows which one was used (see Logs).

## Keys

| Input | Action |
|---|---|
| `1` ... `9`, `0` | Use the action in slot 1 to 10. |
| Left click on a slot | Same as the key. |
| Esc | In the editor, back out one level (see Editor). |

Keys 1-0 belong to the hotbar while the bar is shown, also on an empty slot, so they never trigger the vanilla quick-action bar or switch party members (`ReserveDigits=0` lets an empty slot's key do its game action again). The vanilla bar can still be clicked. Shift/Ctrl/Alt + digit always go to the game. Empty slots are hidden outside the editor.

## Editor

1. Click the wrench button at the left end of the bar. The slots show their frames.
2. Click a slot. A picker opens with three tabs: Power, Item, Mode.
3. Click a cell to put that action in the slot. The INI is saved at once. For an item that can target someone (a
   healing item, or one you can use on yourself), the cells change to a "use on?" row first: **Use normally**,
   **Lowest HP** (healing items) and **Self**. Click one to assign the item with that target mode; Esc goes back to the list.
4. Press **Clear** in the picker to empty the slot. Assigning an action that is already in another slot moves it.
5. Esc closes the picker. Esc again leaves edit mode. Clicking the wrench also leaves edit mode.

In the picker, the `<` and `>` buttons scroll one entry at a time (hold to repeat). The mouse wheel does the same
while the cursor is over the picker. Item cells show the stack count. An item you have run out of keeps its icon,
dimmed, with a count of 0, and cannot be used.

The crosshair button next to Clear changes the target mode of an item already in the slot (see Target modes). Each
slot with a mode shows a badge in its top-right corner: a red cross for Lowest HP, a "Self" pill for Self, nothing for
the default. Hover over a slot for a tooltip.

## Target modes

These apply to item slots only. Powers and modes always show "items only" on the mode button.

| Mode | Meaning |
|---|---|
| Default | The item is used as the game would use it. |
| Lowest HP (`@lowhp`) | Uses the item on the party member with the lowest HP percent, if that member is below the threshold. |
| Self (`@self`) | Always uses the item on the controlled character, whatever is targeted. |

For `@lowhp`:

- The threshold is `HealThreshold` (default 50, range 1..100). A slot can set its own, see the slot syntax.
- If nobody is below the threshold, `HealFallback` decides: `self` uses the item on the controlled character,
  `none` does nothing. Default is `self`.
- Only healing items offer Lowest HP. Which items count as healing, and which can be targeted at all, is read from the
  columns of `override/spells.2da`, not from a fixed list. An item that cannot be targeted shows "cannot target this item".
- A repair kit with no damaged droid in the party does nothing.
- A member who just received a heal from the hotbar is skipped for the next press (up to 5 seconds, or until their HP rises), so two quick presses heal two different members. Heals held in a paused queue for longer than 5 seconds are not tracked.
- `TargetProbe=2` (the default) is needed for `@lowhp` and `@self` to act. See the INI table.

`@lowhp` and `@self` are meant for items. The parser also accepts them on `spell:` and `feat:` values, but that is not a supported use.

## Per-save

The hotbar belongs to the save.

- `action_hotbar.ini` in the game folder is the working copy.
- When the game writes a save (manual save, quicksave, autosave, module-entry autosave), the patch copies the working
  copy into that save's folder as `action_hotbar.ini`.
- When a save is loaded, the profiles in the working copy are replaced by the profiles in that save's copy. `[General]`
  is never changed by a load. The old file is first backed up as `action_hotbar.ini.load.bak` (the last three are kept as
  `.load.bak`, `.load.bak.1`, `.load.bak.2`). Bar edits only persist if you save the game afterwards.
- A save with no copy (for example one made before you installed the mod) keeps the bar you have; the log WARNs `absent (live hotbar kept)`.
- Starting a new game gives an empty bar.
- A save whose copy cannot be read keeps the current bar, and the log gets a warning.
- A save made by 1.1.0 or earlier holds `force_power_hotbar.ini`. It is read when no `action_hotbar.ini` is in the folder.
  Saves written from now on use the new name.
- Edits made in the editor go to the working copy and into the next save you make. They do not change saves already written.
- The first editor change also writes `action_hotbar.ini.bak`.

The save-folder copies are part of the save, so Steam Cloud syncs them.

## Migration from force_power_hotbar.ini

Before 1.2.0 the mod was named Force Power Hotbar and used `force_power_hotbar.ini`. On the first start,
if `action_hotbar.ini` does not exist and `force_power_hotbar.ini` does, the file is copied to the new name.
The old file is left untouched. If both exist, nothing is copied. The log line is
`migrate: copied force_power_hotbar.ini -> action_hotbar.ini (old file kept)`.

## INI reference

The file is `action_hotbar.ini` in the game folder. It is rewritten by the editor, so unknown keys in `[General]` are kept
but comments are not.

### [General]

| Key | Default | Range | Meaning |
|---|---|---|---|
| `SchemaVersion` | 1 | - | Written by the mod. Ignored on load. |
| `Invoke` | 1 | integer | 1: keys and clicks use the action. 0: log only (dry run); bound digits then pass through to the game. |
| `Verbose` | 0 | integer | 1: detailed log. Only used when the shared logger core is missing (see Logs). |
| `AutoFill` | 0 | integer | 1: unbound slots show and use the character's known powers, in game order. 0: unbound slots stay empty. |
| `ReserveDigits` | 1 | 0 or 1 | 1: keys 1-0 are the hotbar's while the HUD is shown, even on an empty slot or in the editor. 0: an empty slot's key does its game action (vanilla quick bar, party switch). |
| `KeyboardDevice` | automatic | integer | DirectInput keyboard index. Leave it out unless keys do nothing. |
| `HealThreshold` | 50 | 1..100 | `@lowhp` only picks a member below this HP percent. |
| `HealFallback` | `self` | `self`, `none` | What `@lowhp` does when nobody is below the threshold. |
| `SyncActionBar` | 1 | 0 or 1 | 1: refresh the vanilla action bar after an AI stance or form change. 0: off. |
| `TargetProbe` | 2 | 0, 1, 2 | Diagnostic. 0: item target modes off. 1: log the target decision only, do not act. 2: live. |

A bad value in a key keeps the default and writes a warning to the log with the line number.

### Slots

Slots are in profile sections: `[Profile.Default]` and one section per character. A slot with no value in the
character's section uses the Default profile. Keys are `Bottom1` to `Bottom9` and `Bottom0` (slot 10).

| Value | Meaning |
|---|---|
| `spell:<n>` | Force power, `n` = row in `spells.2da`. |
| `feat:<n>` | Feat, `n` = row in `feat.2da`. |
| `item:<name>` | Item by display name. A trailing stack count like `(3)` is ignored. |
| `other:<hex>` | Raw action key, for stances and forms. |
| `none` | Explicitly empty. AutoFill will not fill it. |
| (missing or empty) | Unbound. AutoFill may fill it. |

Item target suffix, after the value:

| Suffix | Meaning |
|---|---|
| `@lowhp` | Lowest HP member. Uses `HealThreshold` and `HealFallback`. |
| `@lowhp:30` | Same, with a threshold of 30 percent. |
| `@lowhp:30:none` | Threshold 30, do nothing if nobody is below it. |
| `@lowhp::none` | Default threshold, fallback `none`. |
| `@self` | Always the controlled character. |
| `@default` | Same as no suffix. |

Example:

```ini
[General]
Invoke=1
HealThreshold=50
HealFallback=self

[Profile.Default]
Bottom1=spell:10
Bottom2=item:Medpac@lowhp:40
Bottom3=item:Stim@self
Bottom0=none
```

The editor writes the shortest form. Edit the file by hand only with the game closed, or save it and let the mod re-read it
(changes are picked up while the game runs).

## Logs

The mod uses the shared KpLog logger. Settings are in `kplog.ini`, section `[action-hotbar]`, for example
`level=debug`. The log file is `action-hotbar_log.txt` in the game folder.

Without the `kpatch.log` core, the mod writes `action_hotbar_log.txt` instead. The previous session is kept as
`action_hotbar_log.1.txt`. In that case `Verbose=1` raises the level to debug.

Useful lines: `action-hotbar 1.3.0-dev loaded` at start, `migrate:` on a first run, `persave:` on every save and load,
`actionbar:` on a bar refresh. A `WARN` line is a real fault.

The `spells.2da` lookup is logged once per session, on the `tip` channel. Order: `override/spells.2da`, then `chitin.key`
-> `data/2da.bif`. Lines:

- `tip: override/spells.2da -> ok source=override (cols=<n> rows=<n>)` : the override copy was used.
- `tip: spells.2da -> ok source=chitin (cols=<n> rows=<n>)` : no override, the base game table was used.
- `WARN tip: spells.2da missing (override + chitin: <err>); @lowhp/@self and tooltips disabled` : neither source worked.

## Troubleshooting

- **No slots shown.** Slots appear only on the main HUD, and empty slots are hidden outside the editor. Click the wrench to
  see them. If the wrench is also missing, check the log for `H3 ... template ... not in the HUD layout, control not created`.
  The patch copies two native labels (`LBL_QUEUE1` and `LBL_MOULDING3`) from `mipc28x6_p.gui`. A mod that replaces that
  file without those tags stops the controls from being created. Use a version of that GUI that keeps them.
- **Slots empty after loading a save.** Loading never empties the bar (only a new game does). Your previous bars are in `action_hotbar.ini.load.bak`, `.load.bak.1` and `.load.bak.2`; copy one over `action_hotbar.ini` while the game runs.
- **Hotbar changed after a load.** This is the per-save rule. Your old bar is in `action_hotbar.ini.load.bak`.
- **Keys do nothing.** Check `Invoke=1`. Try the editor to confirm the slot is bound. If the log shows no key events, set
  `KeyboardDevice`.
- **Item used on the wrong target.** Check the slot's suffix and `TargetProbe` (needs 2). Check that `override/spells.2da`
  is the one the game uses. Check `HealThreshold`: above the threshold, `@lowhp` falls back.
- **Medpac did nothing.** With everyone healthy and `HealFallback=none`, nothing happens by design. A repair kit with no damaged
  droid also does nothing.
- **Stance icon on the vanilla bar is stale.** Make sure `SyncActionBar=1`.
- **Old settings missing after the update.** Confirm `force_power_hotbar.ini` was in the game folder before the first start,
  and that `action_hotbar.ini` did not already exist.
- **Warnings about the INI.** The log gives the line number and key. The bad slot is left empty and the rest loads.

## Hooks

One direct detour for the Steam Aspyr build (`CaptureHotbarKey`). HUD hooks are provided by `hud-host` (`k2.hud@1`), and save/load events are provided by `save-events` (`k2.saveevents@1`). Address is in `kotor2-steam-aspyr.hooks.toml`.

| Address | Function | Purpose |
|---|---|---|
| `0x0072CB63` | `CaptureHotbarKey` | Reads the DirectInput key event, so the bound keys can be taken. |
| (hud-host) | `HotbarHudTick` | Per-frame update of the bar, editor and tooltips via `k2.hud@1`. |
| (hud-host) | `HotbarBuildButtons` | Builds the controls in the HUD constructor via `k2.hud@1`. |
| (save-events) | `onSaveEvent` | Per-save INI snapshots and restores via `k2.saveevents@1`. |

## Tests

Host tests (no game needed), in this folder and listed in `test.toml`: `config_roundtrip_test`, `hotbar_logic_test`,
`hotbar_log_test`, `textures_test`, `tlk_test`, `twoda_test`, `common_contract_test`.
Run them from the repo root, one `-k` per test name (the runner matches part of a test id):

```
python3 scripts/run_tests.py -k hotbar_logic_test
```
