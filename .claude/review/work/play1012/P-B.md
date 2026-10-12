# P-B recon: play1012 action-hotbar + hud-host (kplog header 2026-10-12 00:16:23)

Scope: action-hotbar_log.txt (2452 parsed lines), hud-host_log.txt (45 parsed lines). T= is ms since process start; log span ~0.3 s to 1039.2 s. All lines are INFO per `kplog_parse.py --stats`.

## G1 HUD +0x48 per HUD build
| HUD panel | T (s) | Source | +0x48 | Class |
|---|---|---|---|---|
| 08af2448 | 36.5 | BUILD48 dump / owner lines | ff8fb44f | garbage |
| 08af2448 | 41.86 | BUILD48 frame0, HUD48 frame=1 | ff8fb44d | garbage |
| 08af2448 | 43.84 | HUD48 frame=27 | ff8fb445 | garbage |
| 0870a368 | 206.4 | BUILD48 dump | 00000047 | clean |
| 0870a368 | 210.3 | HUD48 frame=9071 | 00000045 | clean |
| 38636800 | 546.1 | BUILD48 entry (panel not named) | 00000047 | clean |
| 38636800 | 548.6 | HUD48 switch line (no value printed); gate read | 00000045 | clean (via GATE f48) |

Garbage only on the first HUD build (08af2448), T=36.5 to 173.2 s (gate kept seeing it until the HUD switch at 210.3 s).

## G2 channel gate
- `GATE hud flags implausible` lines: 7 (T=43.844, 79.813, 93.448, 103.812, 161.846, 162.594, 173.215 s). All hud=08af2448, f48=ff8fb445.
- Each is followed by `GATE open=1 why=open` within 3 ms: 7/7 recovered.
- `why=hud-ignored`: 0.
- `GATE cap reached`: yes, T=557783.002. GATE log is capped at 64 lines (ActionHotbar.cpp:295-305); no GATE lines after this. The separate `gate:` channel (`ACTIVE` / `inactive (reason)` with panel dumps) keeps logging to 1039.2 s.
- open=0 reasons (GATE lines): modal-open 6, other-panel-on-top 31. open=1 why=open: 20. Total 65 = 20+37+7+1.

## G3 open=0 stalls
- No open=0 stretch without a menu/dialog reason. Every open=0 reason is modal-open or other-panel-on-top.
- Longest open=0 stretches (both other-panel-on-top):
  - T=294.7 to 368.9 s (74.2 s; GATE channel, 31 lines).
  - T=557.8 to 633.2 s (75.4 s; `gate:` channel shows one inactive reason for the whole span). Heartbeat gate=0 at 586.25 and 617.51 s agrees.
- Other long ones: 537.1 to 550.6 s (13.5 s, other + modal); 250.0 to 263.1 s (13.1 s, other); 1017.0 to 1030.4 s (13.4 s, other).
- Caveat: the `top` panel flags on these stretches are non-clean (696c6045, 42423845, 4d0a3045). Whitelisted open panels also show such flags (656b6045), so this does not prove the reason is wrong. Not verified.

## G4 coarse-check alert
- Alert at T=550576.393: `character: creature 0c256878 has invalid object id ffffffff (party members=3 slot=0 tag='')`.
- Timing: HUD switch to 38636800 at 548.583 (2.0 s before), last BUILD48 entry at 546.05 (4.5 s before). `char` went objfffffffd -> atton at 548.583 (same tick as the HUD switch), then atton -> objfffffffd at the alert tick. Gate open=1 again at 550.639.
- objfffffffd = the PC / Exile. HotbarLogic.h:98-111 maps oid 0xFFFFFFFF with party slot 0 (or member 0) to canonical "objfffffffd". The alert has slot=0 and tag='', so it is the PC.
- Afterwards: `character:` resolves to named atton at T=712.388 s, back to objfffffffd at 1017.001 s, and stays objfffffffd to log end (last heartbeat 1033.0 s char='objfffffffd').
- Other char changes near HUD switches: 41.86 s (''->atton), 210.31 s (objfffffffd->kreia). Many char changes have no HUD switch (118.5, 183.3, 440.5, 444.1, 712.4, 1017.0 s).
- Bar content: PC profile `eff: character 'objfffffffd' autofill=0 explicit=4 auto=0` with Bottom1=spell:10, Bottom2=spell:34, Bottom3=spell:22, Bottom4=spell:42. Identical at all 6 PC switches (43.8, 183.3, 213.3, 444.1, 550.6, 1017.0 s). No change after the alert.
- atton profile: 0/10 slots bound. Heartbeat vis=0000000000 while gate=1 from 712 to 1017 s: an empty bar for atton, not a fault.

## G5 WARN / ERR / failures
- WARN or ERR lines: 0 in action-hotbar and 0 in hud-host (parser stats: all INFO).
- `append failed`: 0.
- Per channel (action-hotbar): cfg 43, edit 34, input 201, build 40. No error/fail/refused/null lines in any of them. Only hits: `warnings=0` (cfg load line) and `persave: ... absent (live hotbar kept)` at 35.8 s (INFO).
- Gate flap counter (heartbeat `gateflaps`) ends at 31.

## Gate timeline after the GATE cap (`gate:` channel)
inactive(other) 557.8 | ACTIVE 633.2 | inactive(other) 639.1 | ACTIVE 645.8 | inactive(other) 663.9 | ACTIVE 664.5 | inactive(other) 804.9 | ACTIVE 815.8 | inactive(other) 1017.0 | ACTIVE 1030.4 | inactive(other) 1037.2 | inactive(modal) 1038.3 | inactive(other) 1039.2

## OPEN
Were the 74 s (294.7-368.9) and 75 s (557.8-633.2) other-panel-on-top stalls real menus, or stale top-panel reads? Needs a play timeline to correlate.
