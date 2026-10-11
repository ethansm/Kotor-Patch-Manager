# r2_medpacks (recon, read-only)
Log: queue-hud_log.txt, session 02:37:21, L1-L368, read via bounded windows only. Cursor not advanced.

## 7 CMD# rows (all kind=item, m=0, combat=0, paused=1)
| cmd | line | id | icon (0x2E resolve line) | tgt | cr | dec | engq | buf | outcome |
|---|---|---|---|---|---|---|---|---|---|
| 1 | L25 | 3d1 | ii_frarmbnds_005 (L28 0x3F, L211 0x2E) | fffffff6 | 38f8e018 | dispatch_now | 0 | 0 | eaten_paused x5 (L34,56,69,80,93), executed L220 |
| 2 | L39 | 476 | ii_adrnaline_003 (L42) | fffffff6 | 38f8e018 | buffer | 1 | 0 | dispatched L225, executed L237 |
| 3 | L99 | 478 | ii_cmbtshot_001 (L101) | fffffff6 | 38f8e018 | buffer | 1 | 1 | dispatched L241, executed L250 |
| 4 | L281 | c9 | ii_frarmbnds_005 (L284) | ffffffff | c3bf3b0 | dispatch_now | 0 | 0 | combat hand-off L292, lost, no VANISH |
| 5 | L298 | c9 | ii_frarmbnds_005 | ffffffff | c3bf3b0 | dispatch_now | 0 | 0 | combat hand-off L307, lost, no VANISH |
| 6 | L313 | c9 | ii_frarmbnds_005 (L319) | ffffffff | c3bf3b0 | dispatch_now | 0 | 0 | executed L335 |
| 7 | L339 | 1eb | ii_adrnaline_002 (L350) | ffffffff | c3bf3b0 | dispatch_now | 0 | 0 | executed L362 |

## Medpack identity
- Icon families (scripts/gen_buff_hud_icon_table.py:19-38): ii_adrnaline = stim rows 69-74, ii_cmbtshot = 75-77, ii_frarmbnds = 99-107, ii_medeqpmnt = 129 (Squad Recovery Stim).
- No cmd carries an ii_medeqpmnt icon. cmd 2, 7 = adrenal stims. cmd 3 = combat-shot family (class UNSURE). cmd 1, 4-6 = armband family (shield per user report, UNSURE from log).
- Medpac icon name is not in the repo. Medpacs are g_i_medeqpmnt01-03 (items.tsv row 1-3). Verdict on medpacs in log: none present (UNSURE on icon).

## Bypass check
- kind=spell (H10) rows in log: 0. Engine 0x0F rows: 0. DROPS 0. dispatch_now while busy: 0.
- Every 0x2E QJ row traces to a logged CMD#: L211 (cmd 1), L214-215 external (cmd 1, >2000 ms after L25), L321 hook (cmd 6), L330 external (cmd 6, 2.4 s), L349 and L358 external (cmd 7, 2.1-3.0 s).
- "ours=external" on these rows is the 2000 ms expiry only (QueueHud.cpp:1223 kOursWindowMs). Not a bypass signal.
- Result: no hook bypass visible, and no medpack event to judge "hooked but not buffered".

## Engine path (source and docs)
- PerformUseItem 0x775600 (thin wrapper, RET 0x10): H9 hook at 0x775603 (hooks.toml:10-14). Handler QueueHud.cpp:1387, dispatch_now :1436, buffer :1447, drop_full :1457.
- PerformCastSpell 0x775700: H10 hook at 0x775703 (hooks.toml:29-33), QueueHud.cpp:1469. Non-combat Force powers use it (33_ui_queue_hotbar_fixes.md:147-148).
- Own dispatch: dispatchAction QueueHud.cpp:815 (H10 call :818, H9 call :828). g_dispatching suppresses CMD# on re-entry (:1388, :1470).
- Item callers: PartyMenuUseItem 0x77CC80 calls 0x775600 at 0x77CCC3 (29_paused_command_evidence/PQ1.md:41). Hotbar runs CanUseItem before PerformUseItem (27_companion_selfheal_evidence/R5_canuse/FINDINGS.md:33).
- Server side: client PerformUseItem sends msg (6,9) to handler 0x65C45F (PQ1.md:82). ItemCastSpell = 0x2E, AddItemCastSpellActions 0x5648D0 (29_paused_command_queue.md:23, :55).
- Docs name no separate medpac entry. queue_hud_logic_test.cpp:973-1026 models medpacs as item actions (test assumption, not engine evidence).

## Unknowns
- A medpac refused by CanUseItem never reaches 0x775600, so no CMD# would appear. Not testable from this log.
- tgt fffffff6 vs ffffffff and cr 38f8e018 vs c3bf3b0 meanings: not documented in repo (UNSURE).
- Side finding: cmd 4 and 5 (shields) handed to combat produced no outcome.

## Next single test
- Paused, shield queued, click one medpack. Expect a CMD# kind=item line with dec=buffer. No CMD# means refusal before 0x775600 (check CanUseItem).
