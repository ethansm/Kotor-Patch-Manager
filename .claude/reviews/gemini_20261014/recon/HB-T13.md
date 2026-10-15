# HB-T13
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| HB-1c-05 | PARTIAL | patch_manager_mods/combat_hotbar_src/CombatHotbar.cpp:740-751,753-762 | no | Gap real: no bit-1 restore, no g_arrowState reset. Visible impact unproven (stock repopulation outside range). |

- CombatHotbar.cpp:740 `if (g_cfg.hideArrows) {` matches the anchor exactly (anchor check: exact).
- CombatHotbar.cpp:743-747 clears bit 1 (`*p &= ~2u`) on 6 addresses (3 groups x 2 offsets), only while hideArrows is true.
- CombatHotbar.cpp:735 comment: bit-1 clear runs "after the stock population", so stock code sets bit 1 first on each call.
- CombatHotbar.cpp:750-751 is the only write to g_arrowState in the read ranges; it sets 1 (applied) or 0 (failed), logging on change only.
- CombatHotbar.cpp:753-762 reload block: loadIni/loadProfile, then restores btnEnabled/cols/maxRows (757) and resets g_pillMask and g_nameValid (760-761). No bit-1 write, no g_arrowState write.
- CombatHotbar.cpp:114 declares g_arrowState = -1 (initial value only).
- After 1->0, the gate at 740 is skipped, so bit 1 is never re-set by this hook; it stays as stock left it.
- Stale g_arrowState after 1->0->1: st equals 1, so the "arrows: hide applied" log line is suppressed. No functional effect in the ranges.
- hideArrows is not in the restart-only preserve list (754/757/759), so it should apply on the next call once loadIni (outside range, line 120+) sets it.
- Gemini's fix: blind `|= 2` could force visibility on controls stock hides. Resetting g_arrowState only restores one log line.
