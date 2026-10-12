# Q-A: companion queue noise, Down window (session 2026-10-12 00:16:23)

Source: queue-hud_log.txt, read only via kplog_parse.py (intermediates in scratchpad). T in ms.
W_down = T 712000..1030395 (318.4 s). W_up = T 600000..686000 (86 s).

## A1 QJ events with ne>=1 (entry appeared)

| member | W_down count | per min | W_up count | per min |
|---|---|---|---|---|
| m=0 | 165 | 31.1 | 0 | 0.0 |
| m=1 | 10 | 1.9 | 28 | 19.5 |
| m=2 | 11 | 2.1 | 27 | 18.8 |

- m=2 has no ne>=1 QJ in T 706355..712000.
- QJ logs only on queue-signature change, sampled every 100 ms (QueueHud.cpp kJournalSampleMs). It undercounts entries shorter than that (see A3).

## A2 m=1 / m=2 inside W_down

- Types: m=1 = 10 x [3f]; m=2 = 11 x [3f]. No other type seen.
- Lifetime (ne=1 QJ to next ne=0 QJ or ROUND n=0):
  - m=1: median 103.8 ms, max 118.7 ms (n=10)
  - m=2: median 107.8 ms, max 123.0 ms (n=11)
- Every end was a QJ ne=0. Lifetimes are about one sample period.

## A3 draw strip k=1 / k=2 inside W_down

- Lines: k=1 = 82 (41 shown, 41 hidden); k=2 = 135 (67 shown, 68 hidden).
- Visibility flips: k=1 = 81; k=2 = 134; total 215 in 318 s = 40.5/min.
- W_up flips for comparison: k=1 = 6; k=2 = 10; total 16 in 86 s = 11.2/min.
- Rects: shown k=1 = (3456,1304,58,58); shown k=2 = (3456,1200,58,58); hidden = (0,0,0,0).
- Shown dwell median ~409 ms (max 435). Hidden dwell median ~2.6 s.
- Shown-to-shown period median 3032 ms (k=1) and 3034 ms (k=2): a steady ~3 s cycle.
- Shown pulses with a QJ m=k ne>=1 event within +/-300 ms: k=1 = 10/41; k=2 = 11/67. Most pulses have no QJ entry.
- Mechanism (source): stripNote logs only on state change (QueueHud.cpp:617-622). The strip repaints the last good draw for kPanelHoldMs = 400 when the engine queue reads empty (QueueHud.cpp:629, 665-667; QueueHudLogic.h:131). That explains the ~400 ms shown dwell.

## A4 VANISH / FLAGS for m=1 / m=2 inside W_down

- VANISH in W_down: 12 total, all m=0 (executed 11, eaten_wait 1). For m=1 / m=2: 0.
- FLAGS from `kotor2_logs.py check`: 7 total. Six involve m=1 / m=2, all before W_down (T 183356, 550649, 550649, 672959, 679790, 680716). For m=1 / m=2 inside W_down: 0.
- Check output saved in scratchpad only (outside write scope). Its L-numbers equal parse-line index + 9; verified on HANDOFF and DROP lines.

## A5 ours= on entries

- W_down m=1: 10/10 external. m=2: 11/11 external. Non-external for m=1 / m=2: 0.
- W_down m=0: 118 external, 47 non-external (dispatch 19, handoff 13, hook 12, cancel 1, reinject 1, resolve 1).
- ours= is a 2 s temporal stamp of queue-hud's own actions (QueueHud.cpp:1482, 1539). It is not proof of origin.

## Why VERDICT is PARTIAL: origin of the 3f entries is not established

- Type 3f is also the CompanionSelfHeal Medpac item use (route A; CompanionSelfHeal.cpp:9). queue-hud does not stamp selfheal actions, so they would read as external.
- Selfheal log has exactly one CALL routeA this session: member=1 at T=710450.694. DONE at 711241 reported queued=0.
- Member 2 dedupe at T=685516..704081 saw queue=1 types=[3f], but the log has no member-2 routeA. Source of that item is unknown.
- queue-hud has 0 CMD lines for m=1 / m=2 (all 54 are m=0).
- Selfheal logged no AUTO or USE in W_down. member=1 went AutoOn->Hidden at 712390 and Hidden->Down at 712488.

## Anchors

- queue-hud_log: `T=712308.293 f=- queue-hud/trace QJ m=1 ne=1 types=[3f] ... ours=external`
- queue-hud_log: `T=718653.291 f=- queue-hud/draw strip k=1 shown n=1 types=[0x3F/0] rect=(3456,1304,58,58)`
- companion-selfheal_log: `T=710450.694 f=- companion-selfheal/call CALL routeA returned member=1`
- QueueHudLogic.h:131 `kPanelHoldMs = 400`
