# r4 recon: cmd 1 eaten loop (A) and IDENT_SWAP x12 (B). Raw line = analyzer L-number; T in s.

## A. cmd 1 (item 3d1, m=0), paused=1 throughout
| # | REINJECT | in_queue back | next wipe (no_leader) | next VANISH | REINJECT->VANISH |
|---|---|---|---|---|---|
| R1 | L35 35414.926 | L36 +22ms | L53 47224.263 | L56 47378.029 | 11963 ms |
| R2 | L57 47378.032 | L58 +15ms | L66 51619.042 | L69 51769.400 | 4391 ms |
| R3 | L70 51769.403 | L72 +16ms | L76 54544.533 | L80 54705.536 | 2936 ms |
| R4 | L81 54705.540 | L82 +14ms | L90 59831.603 | L93 59981.316 | 5276 ms |
| R5 | L94 59981.320 | L95 +27ms | none while paused | executed L220 after unpause L210 | n/a |

- VANISH lines: L34, L56, L69, L80, L93. Analyzer flag "D L80" is the 4th; the 5th is L93.
- Between REINJECT and VANISH: QJ m=0 ne=1 types=[3f] round=1 paused=1 (L38, L60, L71, L84, L97), then ne=0 ours=external (L55, L68, L79, L92). No 0x2E engine node; draw-strip 0x2E entries are HUD-buffered cmd 2.
- Wipe 1 (L31, 35251.158) happens BEFORE the first reinject; its only prior own action is the dispatch CMD#1 L25 (2962 ms earlier, outside the 2000 ms ours window).
- Nearby CMD: CMD#2 L39 (buffered, 9.8 s before wipe 2), CMD#3 L99 (after R5). No CMD or EDGE within 3 s before wipes 2-5.
- Wipe-to-wipe gaps after our own PerformUseItem: 2963, 11809, 4241, 2775, 5126 ms. R5 had no wipe for 29.6 s.
- VERDICT A-wipe: not a synchronous self-wipe by REINJECT (ne=1 persists 17-112 ms after each). Cause unlogged (user or native clear). UNSURE.

Budget (A-loop):
- Reset site: ActionBuffer::notePresent (QueueHudLogic.h:625-628), called from AV_PRESENT (QueueHud.cpp:1119). Zeroes eatenReInjects and eatenUnpausedMs when actionInQueue and now - lastReInjectTick >= 2000.
- markDispatched (QueueHudLogic.h:609) does NOT reset the budget. Only notePresent does.
- The same action's round node (subType 10) keeps actionInQueue=true for 11.8 / 4.2 / 2.8 / 5.1 s; REINJECT_NO stays in_queue with no change logged between R_n and the next no_leader. So every wipe prints attempt=1.
- Per-action cap kMaxReInjectCount=2 (QueueHudLogic.h:521) is incremented only on the RI_OK path (QueueHud.cpp:1101), never on the eaten path (1150). No effective cap.
- eatenUnpausedMs accrues only while unpaused (accrueEaten), so the 20 s give-up never fires while paused. Loop is unbounded while paused.
- VERDICT A-loop: CONFIRMED (self-inflicted cap bypass).

## B. IDENT_SWAP x12 (all 12 lines; from->to)
L112 70265.180 0->2 | L128 78845.618 2->1 | L137 79647.363 1->0 | L143 81295.736 0->2
L150 81560.134 2->1 | L157 81863.093 1->0 | L168 86302.219 0->2 | L177 87082.761 2->1
L182 87494.271 1->0 | L189 88199.238 0->2 | L196 88415.428 2->1 | L203 88600.227 1->0
- One busy buffer (creature 38f8e018: active cmd 1 + queued cmd 2/3) walks 0->2->1->0 four times. Other buffers are empty. No k<->j alternation.
- Engine queues move with the creature in the same tick: L117-119, L173-176, L187-188, L194-195.
- Party.h:21: PartyGetAt position 0 = controlled creature. One creature stepping 0->2->1->0 matches a controlled-character cycle (one swap per switch).
- slotOfCreature (QueueHud.cpp:773) matches client OR serverCreature, so a pointer-type mismatch gives slot -1 (skip + identity timer), not a swap. REFUTED as cause.
- No IDENT_SWAP before L112, so the guard did not cause the cmd 1 wipes (L34-L93).
- Trigger unlogged: no EDGE or controlled-change line in L112-L203; QJ has no cr= field.
- VERDICT B: legit rotation; the guard follows the creature correctly. Not ping-pong.

## Minimal fixes
1. QueueHudLogic.h notePresent (625-628): reset eatenReInjects/eatenUnpausedMs only when ev.executing, not on any present frame. Update queue_hud_logic_test.cpp:1662-1669 (pins the reset).
2. Or in the QueueHud.cpp:1150 branch: also do buf.activeAction.reInjectCount++ so RI_MAX (cap 2) bounds the eaten loop.
3. Add trace at gameaddr::ServerClearAllActions call sites, and log the return address when a busy round node vanishes, to name the clearing source.
4. Add EDGE what=controlled when party::member(tbl,0) changes (near QueueHud.cpp:960), to tie IDENT_SWAP to input.

## Open
- What clears the round node + 0x3F at L31/L53/L66/L76/L90 (native clear vs user input).
- Whether the 12 controlled-character switches (L112-L203) were user Tab or portrait presses.
- Whether our PerformUseItem triggers a delayed clear (4 of 5 wipes followed one by 2.8-11.8 s; R5 did not).
