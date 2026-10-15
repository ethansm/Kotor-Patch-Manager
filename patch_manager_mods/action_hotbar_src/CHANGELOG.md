# Changelog

Action Hotbar (id `action-hotbar`), formerly Force Power Hotbar.

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
