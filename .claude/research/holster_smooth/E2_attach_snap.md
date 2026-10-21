# E2: in-world slide state machine (HolsterPlace), what cuts or restarts a slide

Scope: patch_manager_mods/holster_src/Holster.cpp (HolsterPlace, drawgate path), HolsterSlide.h, HolsterDrawGate.h, HolsterDelay.h. Read-only, no logs read.

## Core rule (drawgate, in-world)
- Per placement call, `hip` = shouldWriteHip(readable, combat, flourish|hold|partyHold) (HolsterDrawGate.h:44-48). Slide target = !hip.
- `first` (new entry, or owner changed) or `!readable` -> `slide.reset(!hip)`: hard set, len=0, no ease (Holster.cpp:1676, 1684-1686; HolsterSlide.h:47-53).
- Otherwise `touch` + `target(!hip)`: retarget keeps `from = frac(now)` (no value pop), duration = ms * remaining distance (HolsterSlide.h:65-80).
- Slide advances (1689-1715), then reads `in` (+0xA4) and may return early: slideF >= 1 returns with no write (engine hand pose shows, 1716-1718); refusals (1723, 1724, 1738, 1742, 1748, 1749) return with no write.

## Event table
| Event | Starts slide? | Restarts or cuts? | Anchor |
|---|---|---|---|
| Engine native rhand attach (drawgate, no code patch) | No. Attach is constant; only the pose write toggles. | No. Cache key (node, gob, handle) unchanged, so no reset. `in` changes under the blend. | Holster.cpp:1547-1624; 1723 |
| Combat enter/exit (owner+0x138 client flag) | Yes, at the flip frame (retarget or first start). | Retarget only when the flag flickers mid-slide. | Holster.cpp:1670; HolsterSlide.h:65-80 |
| Move hold (move_hold_ms) | No. Not a gate input; blocks the +0x524 combat cut, so combat flag stays 1 longer. | No. | Holster.cpp:837, 570 |
| Party combat hold (g_partyCombat, 250 ms) | Yes, on edges. | Retarget per 250 ms edge; ownerIsParty uses a 250 ms party cache, so an empty cache drops all party holds at once. | Holster.cpp:694-720, 723-739, 1671 |
| Toggle draw hold (draw_hold_ms, GetTickCount) | Yes, at draw press (hold sets hip=false). | Hold expiry while combat=0 retargets to stow. Stow press clears hold. | Holster.cpp:890-894; HolsterDrawGate.h:52-54 |
| Flourish window open/close | Yes, on edges (only when combat=0). | Retarget on each edge. | Holster.cpp:743-, 1670 |
| Weapon swap / new Gob | No. New entry is "first": silent placement, no slide. Pops to target. | Cancels any slide on that entry (entry rebuilt). | Holster.cpp:1613-1618, 1676, 1684 |
| GOBREUSE (freed address, model changed) | No. Same silent path. | Cancels slide. | Holster.cpp:1582-1592 |
| Cache eviction (ring of 128) | No. Re-created entry is silent. | Cancels slide. | Holster.cpp:1566, 1613-1614 |
| Hand detection flip (HANDCHK) | No. Drawgate: `left` fixed at creation from node name; the per-frame update at 1594-1599 is repoint-only. | None. Refuted as a trigger. | Holster.cpp:1594-1599, 1623 |
| Drawgate open/close (hip flip) | Yes, retarget from current frac. | Retarget only; a reversal late in the slide gets duration ms*dist (about 35 ms at 0.9 of 350 ms). | Holster.cpp:1685-1690; HolsterSlide.h:65-80 |
| Owner or combat unreadable (any frame) | Hard reset to hand (draw target). | CUTS the slide, pins to hand, then recovery resets again via `first` (owner stored as 0). | Holster.cpp:1677, 1684-1685; HolsterDrawGate.h:44-48 |
| Stall > 100 ms while placement stops | No. | touch() shifts t0 by the gap, so the slide resumes, does not finish during the pause. | HolsterSlide.h:84-88 |

## Concrete cut or restart scenarios
1. Unreadable owner, one frame (hypothesis, not yet seen in logs). Stow at fraction 0.4. Frame N: owner or combat read fails, `readable=false`, `hip=false`, reset(hand) sets slide to 1.0, slideF>=1 returns with no write: weapon shows engine hand pose. Frame N+1: readable, but gate.owner was stored as 0 at frame N, so `first` is true: silent reset(hip). Result: hand pop then hip pop, no ease, in two frames. A draw in progress is pinned to hand the same way.
2. Retarget chatter. Combat flag or party cache flips while sliding. Each flip sets t0=now and from=frac, so the value holds for one frame then moves at constant speed in the new direction. Reversal near the end is a very short slide (a 0.1 remaining distance on 350 ms is 35 ms), which reads as a snap. A longer slide gives more time for a reversal to land mid-slide.
3. Refusal frames after the slide advanced. Slide state moves at 1689-1715, then the frame can exit with no write: 1723 (readAt +0xA4 fails), 1724 (insane in), 1738/1742 (mountPose fails; mountPose returns false when node+0x40 owner reads 0, 1135, 1155), 1748, 1749. The engine pose shows for that frame: a one-frame flash to full hand mid-hip-slide. Likelihood depends on how often these refuse counters tick (PLACE n=).

## Checked and refuted
- Engine attach as a slide trigger: the drawgate attach is never changed by the mod (1547 comment, drawgate "no code patch"); attach-to-hand does not reset the key or the slide. The H6 handler (0x5047B0) writes +0xA4 only on attach events (Holster.cpp:571, 924-926), so it can overwrite our blend for one frame only if it runs after H7 in the same frame. Order unknown (see OPEN).
- Two gate flips inside one frame: holsterTick runs at 1551 before the gate read, and each call reads its inputs once, so one call sees one state. Flips need inputs to change across frames.
- Hand detection flip: refuted (see table).
- Editor preview slide: separate global `g_prevSlide` (Holster.cpp:1393, 1481-1485), does not touch in-world slides.

## Ranking (likely causes of visible snap with a longer slide)
1. Unreadable owner/combat read: hard reset cuts slide and produces hand pop then hip pop (scenario 1). Fits "engine re-parents weapon at combat transitions" only as a hypothesis; the owner pointer is beh+0x14 then +0x1D8 (1664-1665).
2. Retarget chatter from 250 ms party cache and combat/flourish edges, with short reversal slides (scenario 2).
3. Refusal frames after slide advance: one-frame engine-pose flash (scenario 3).

## Log checks that would confirm (no raw log reads done here)
- `DRAWGATE owner/combat unreadable` lines: quiet mode logs only first 3 per gob then every 50th (Holster.cpp:1514-1517); needs log_verbose=1 to count.
- `SLIDE start ... from=` followed by a new `SLIDE start` with no `SLIDE end`: retarget or silent reset. A reset clears slFrames without logging end (1686, 1711), so a cut slide leaves no end line.
- `PLACE` refuse counters `n=` for PrReadIn, PrNoMount, PrInsaneIn.

## Cheapest probe to add (if the user wants one)
Count reset reason (first vs unreadable) and log "SLIDE cut" when a reset hits a running slide (len>0 and frac<1 at reset). Anchor: Holster.cpp:1684-1686.
