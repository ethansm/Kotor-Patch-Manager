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
