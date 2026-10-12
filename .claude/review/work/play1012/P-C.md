OUTPUT: /home/ethansm/orca/workspaces/local-setup/KOTOR-II/.claude/review/work/play1012/P-C.md
EVIDENCE: CANCEL m=2 T=672116.115 -> QJ ne=0 +38ms -> 0x3F refill T=672436.887 (+320ms); B flags L552/L596/L606 = T=672959.453 m=2, T=679790.233 m=1, T=680716.111 m=1; VIS cap reached T=702949.355 (only; no append failed)
FINDING: WARN 1, ERR 0. A3 B3 D0; B = 0x3F entry returns after cancel, origin unlogged. No ring overlap. VANISH 21/4/0/5, eaten_paused 0 (not high).
OPEN: Is the post-cancel 0x3F entry the cancelled command itself (needs cmd id logged at cancel time) or an engine default action?
VERDICT: PARTIAL

---
Detail (checker L-numbers map to kplog_parse lines with offset 9; T= from kplog_parse)

Q1 WARN/ERR
- WARN=1 at T=649356.434: `queue-hud/icon blank icon: type=0x3F param0=00000001` (known cosmetic). ERR=0. Distinct signatures after normalization: 1.

Q2 FLAGS (checker total 7)
- I=1: L210 m=0 HANDOFF active=8 never re-sent (not an A/B/D class).
- A=3: L234 m=1, L422 m=1, L423 m=2 follower 0x3d wiped; all ours=external, so not cancel-related.
- B=3: L552 m=2 T=672959.453 (cancel L550 T=672436.887); L596 m=1 T=679790.233 (cancel L590 T=679143.103); L606 m=1 T=680716.111 (cancel L596 T=679790.233, age 2392ms, ours=external).
- D=0.
- Cancel timeline (route=ServerClearAll, each a button or strip click):

| cancel T | m | QJ ne=0 after | 0x3F refill | persisted |
|---|---|---|---|---|
| 672116.115 | 2 | +38ms | +320ms (round 1) | to next cancel, ~1.2s |
| 673602.382 | 2 | +38ms | +810ms (round 0) | >=2.3s |
| 678324.212 | 1 | +72ms | +818ms (round 1) | >=2.4s |
| 862227.012 | 0 | +82ms | +1790ms (round 1) | >=2.9s |

- The queue held 0x3F items just before each cancel (QJ ne=1 types=3f), so the refill matches the cancelled type.
- The refill shows in the strip (draw strip k=2 n=2 at T=672422.627; k=1 n=2 at T=679143.339) and the cancel button reappears (VIS cancel2 shown=1 why=queue at T=674360.427).
- No CMD#/DISPATCH/REINJECT lines in T=660000-690000, so the mod did not re-dispatch. Origin of the refill is unlogged.
- B fires because the refill's round climbs 1 to 2 while ours=cancel is still within the 2000ms window (kOursWindowMs).

Q3 Geometry
- Non-zero rects: 181 strip, 48 VIS. Strip slots are only x in {3276,3336,3396,3456} x y in {1200,1304}, all 58x58.
- cancel1 rects x 3204-3442, y 1304-1362. cancel2 rects x 3264-3442, y 1200-1258. None overlaps ring [3436,1394,58,58]; vertical gap is 32px.
- Non-58 rect: VIS cancel0 at [1700,1519,440,64] (member 0 wide button). Not a strip icon; no ring overlap.
- VIS cap reached once at T=702949.355 (64-line cap). VIS lines stop after that, so the T=862227 cancel has no VIS evidence.
- append failed: none.

Q4 VANISH (checker)
- Total 30: executed 21, unseen 4, eaten_paused 0, eaten_wait 5. Prior session: 3/2/22 (27).
- eaten_paused text appears 4x only as REINJECT why=eaten_paused (reinject reason, not a VANISH kind). D-class eaten-loop = 0.
- STALE_CUR=2 (m=0, T=952700.145 and T=993498.319).

Scope note: Q2 verdict is about timing and type match, not command identity. No cmd id is logged at cancel time.
