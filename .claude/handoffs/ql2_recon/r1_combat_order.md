# r1 combat-order recon (queue-hud log L275-L335; QueueHud.cpp)
Stamps = log T (ms). Offsets = from the combat EDGE or from HANDOFF_ONE.

## (a) Timeline
cmd 4 (CMD#4 L281, dispatch_now, paused=1, combat=0, engq=0):
- L286 180041.8 QJ m=0 ne=1 [3f] round=1: item queued as 0x3F tail, 1 round node.
- L288 182174.0 HUD "hidden (empty queue)" after REINJECT_NO L287. Item gone 33 ms before edge; paused=1.
- L289 182207.2 EDGE combat 0->1.
- L290 182207.3 QJ m=2 (other member) ne=4 [1,13,11,c] combat=1: combat entries already present at the edge.
- L291 +29 ms HANDOFF m=0 active=4 (headValid). L292 +47 ms HANDOFF_ONE cmd=4 seq=1/1.
- L294 +113 ms QJ m=0 ne=1 [3f] round=2 combat=1: item still tail (not 0x2E); round 1->2.
- L295 190923.7 QJ m=0 ne=0 round=0: item left queue during combat. No m=0 QJ between L294 and L295. Outcome: none logged.
cmd 5 (CMD#5 L298, dispatch_now, paused=1, combat=0, engq=0):
- L302 192841.3 QJ m=0 ne=1 [3f] round=1 (pre-combat).
- L304 195216.6 HUD hidden (empty queue) after REINJECT_NO L303. Item gone 15 ms before edge.
- L305 195231.4 EDGE combat 0->1. L306 +11 ms HANDOFF m=0 active=5. L307 +25 ms HANDOFF_ONE cmd=5.
- L309 +39 ms QJ m=0 ne=1 [3f] round=2 combat=1: item still tail.
- L310 196908.7 EDGE combat 1->0. L312 196957.1 QJ m=0 ne=0 round=0: item left queue during combat. Outcome: none logged.
Note: HUD is hidden in combat (L293, L308), so a combat-time 0x2E cannot be drawn. The 0x2E c9 at L319 (200233.6) is cmd 6 (VANISH executed L335, age 2952), not cmd 4 or 5.

## (b) Engine order
- Assumption: m=0 got the same combat-start entries as m=2 (only m=2 logged at the edge, L290). Under that, queue order is attack, then item.
- Execution time not logged. Item still queued at +113 ms (cmd 4) and +39 ms (cmd 5); gone by combat end (+8.7 s, +1.7 s).
- Exact ms gap between first attack and item: UNKNOWN from this log.

## (c) Was the item in the engine before combat?
- Yes: 0x3F + round node pre-combat (L286, L302).
- No at the edge: removed 15-33 ms earlier, before the handoff ran. Cause open: pause-eat (precedent L80 eaten_paused) vs combat-start rebuild before g_combat flips.
- HANDOFF re-dispatch placed it behind the combat entries; it did not move a live item.

## (d) Source (QueueHud.cpp; dispatchAction not read, per brief)
- engineHoldsAction 855-863: true only for 0x2E/0x0F entries or round/cur subtype 9/10. 0x3F and subtype 1 (attack) ignored, so attacks never block re-dispatch.
- handoffTick 877: headValid = buf.active && !inEngine (898); clears buf.active/seenInQueue/eaten (908). Handed-off head untracked, so "no outcome" is structural (inference).
- Dispatch: capacity skip (924), midFlight skip only for 0x2E/0x0F (925-928), dispatchAction(a) (942). No round-node ordering check: head appends at engine tail after nodes present that frame.
- Call site tickItemQueues 959, only when combatNow. A surviving subtype 9/10 node blocks re-dispatch (item keeps its place).
- HANDOFF to HANDOFF_ONE gap of 14-18 ms inside one function: same-frame vs deferred (924/928) is OPEN.

## Open
1. cmd 4/5 execution unlogged. Add m=0 QJ on each change during combat, plus outcome for the handed-off head.
2. QJ omits round subtypes: round 1->2 cannot be attributed to attack vs item.
3. Pre-edge loss cause: one m=0 QJ at the edge frame, before handoffTick, settles pause-eat vs combat rebuild.

## Verdict: PARTIAL (re-dispatch behind combat entries supported; execution order unproven; loss predates handoff)
