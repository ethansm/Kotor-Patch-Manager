# T1 forensics: HANDOFF trigger loss

Source: `.claude/handoffs/ql3/logs/queue-hud_log.txt` (frozen copy, 453 lines). Checker: HANDOFFS 3 (clear x2), 8 commands, FLAGS 0.
ms = delta of log `T=` stamps. `cmd->HO` = trigger CMD line to HANDOFF line.

## HANDOFF events

| Event | Trigger | dec (line) | Engine just before | HANDOFF lines | ms cmd->HO / edge->HO | Class |
|---|---|---|---|---|---|---|
| H1 L180 | 4 | dispatch_now (L164) | L172 QJ m0 ne=1 [3f] r=1; L174 draw r=0, trigger slot `[0x2E/-1]` (not in round list) | L180 HANDOFF active=4 buf=0; L181 CLEAR ne=1 r=1; L182 ONE cmd=4 1/1 | 1931218 / 44 | WIPED_BEFORE_EDGE (re-sent L182) |
| H2 L236 | 6 | dispatch_now (L209) | L219 VANISH eaten_paused ne=0; L220 REINJECT#1; L223 QJ ne=1 [3f] r=1; L230 REINJECT_NO no_leader (not in queue); L231 draw r=0 `[0x2E/-1]` | L236 HANDOFF active=6 buf=1 cmds=7; L237 CLEAR ne=1 r=1; L238 ONE cmd=6 1/2; L239 ROUND ne=1 [2e]; L241 ONE cmd=7 2/2 | 3408693 / 15 | WIPED_BEFORE_EDGE (re-sent L238) |
| H3 L302 | 8 | dispatch_now (L293, engq=0) | L283 QJ m0 ne=0; L297 draw cur=1 r=0 `[0x2E/-2]` (engine current item) | L302 HANDOFF active=8 buf=0 cmds=-; no CLEAR, no ONE; L304 QJ ne=1 [3f] r=1 cur=1 (+70 ms), 2E entry gone | 20185 / 17 | OUTSIDE the 4 labels: engine-held at edge, not re-sent, dropped at combat start |

## No-outcome commands

| Cmd | dec (line) | Lifecycle lines | Re-sent | ms cmd->last line | Class |
|---|---|---|---|---|---|
| 4 | dispatch_now (L164) | L165 no_leader, L169 in_queue, L173 no_leader (not in queue) | L182 ONE (=H1) | 1913228 (L173); 1931247 (to L182) | WIPED_BEFORE_EDGE, re-sent. Round 2->1 by L190 (+17438 ms after re-send), consistent with item executing; no RESOLVE |
| 6 | dispatch_now (L209) | L210/L213/L216, L219 VANISH eaten_paused, L220 REINJECT#1, L221, L223 QJ, L230 no_leader | L238 ONE (=H2) | 3391767 (L230) | WIPED_BEFORE_EDGE, re-sent |
| 7 | buffer (L224), engq=1 | L226 WAIT active (age 1), L229 WAIT active (age 1025) | L241 ONE seq 2/2 (=H2) | 1024845 (L229); 1568948 (to L241) | TRIGGER_KEPT (buffered behind cmd 6, not the initial trigger) |
| 8 | dispatch_now (L293, engq=0) | L294 STALE_CUR sub=1; L295 REINJECT_NO no_leader | none (engine-held at L302) | 828 (L295) | OUTSIDE the 4 labels (=H3) |

Tally: LOST_BY_OUR_CLEAR 0/3. UNTRACKED 0 (all four commands are kind=item: L164, L209, L224, L293). WIPED_BEFORE_EDGE 2 (cmds 4, 6). TRIGGER_KEPT 1 (cmd 7, buffered). OUTSIDE 1 (cmd 8).

## Summary

1. Our HANDOFF_CLEAR removed no trigger: both clears (L181, L237) ran with ne=1 r=1 after the trigger had already left the round list, and the trigger was re-sent right after each clear.
2. Cmds 4 and 6 left the engine silently while paused (no VANISH; REINJECT_NO no_leader = not in queue) 17-18 s before the edge. HANDOFF re-sent both.
3. Cmd 8 is the only trigger drop after HANDOFF ran. `engineHoldsAction` said the engine held it (QueueHud.cpp:965, :969), so no clear and no re-send. Entry gone by L304 (+70 ms), no RESOLVE/VANISH. Source comment at QueueHud.cpp:1011 says combat start wipes the engine queue, so the likely cause is engine-side, not confirmed.
