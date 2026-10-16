# Play-test queue

Live-game checks that agents cannot run. Fixes ship with host tests; the live check waits here for the next batched
play session, so it is deferred, never dropped (user rule, 10-15).

**Who adds rows:** whoever closes an item whose verification needs the live game (`live = 1` in
`.claude/work_items/sizing.tsv`, V = 3 in `.claude/work_items/SIZING_RUBRIC.md`): `/next-dev-work-item`, the
variant-fix-wave `close` step, or a manual fix. Add the row in the same commit as the fix.

**Before a play session:** install what the `install` column names (`kotor2-kpm-install` skill), play the `pending`
rows, then check each pass signal with `python3 scripts/kotor2_logs.py check <log> --new`.
**After:** set `status` to `pass` or `fail <date> <one-line why>`; a fail opens a new WI.

| id | source | mod@version | install | steps | pass signal | status | added |
|---|---|---|---|---|---|---|---|
| WI-24 | vw1-A | active_effects_panel 0.2.0-probe | rebuild + install active_effects_panel_src | add `[character-effects-panel]` `level=debug` to kplog.ini in the game dir, start the game, open the character screen with active effects | `kotor2_logs.py check character-effects-panel_log.txt` shows the INI loaded line with debug=1 | pending | 2026-10-15 |
| WI-21 | vw1-A | inventory_sort_filter 0.2.0 | rebuild + install inventory_sort_filter_src (Common GameAddr.h synced 10-15) | with the sort filter on, hold 2+ items of one base type with different names, open the backpack, close and reopen it | same-type items list A-Z, case-insensitive, by name; `kotor2_logs.py check inventory_sort_filter_log.txt` shows a sort timing line per flush and no crash (first live call of GetString 0x7356B0, REA verdict UNSURE) | pending | 2026-10-15 |
| PRV | provider rename 10-15 | portrait-slots 0.1.0, gui-input 0.1.0-dev, hud-host 0.5.1-dev, save-events 0.1.0-dev, npc-world 0.1.1-dev | none: hot-swapped LIVE 10-15 22:12 (rollback `kpm_hotswap.py --rollback batch_swap_20261015_221230`) | start the game, load a save, show the party HUD (portraits + buff/shield bars), open and close a menu with mouse clicks, quick-save once | portraits, buff HUD, shield bar, combat hotbar and menu clicks behave as before; quick-save works; `kotor2_logs.py check` on portrait-slots, gui-input, hud-host, save-events, npc-world logs shows provider registration and no new errors | pending | 2026-10-15 |
| HW-C6 | doc 42 C6 stance probe | holster 0.1.0-dev (kpatch d25a2e25) | none: hot-swapped LIVE 10-16 01:48 (rollback `kpm_hotswap.py --rollback holster_swap_20261016_014846`); game-dir holster.ini ch_anim=1 | load a save, walk with the party; unequip one member's weapon if possible; look at Atton's heavy blaster on the back (grip left, raised) | `kotor2_logs.py check holster_log.txt` ANIMRES line shows `style per creature` values per party member (expect pistol 4, melee 1/2, unarmed 0) | pending | 2026-10-16 |
