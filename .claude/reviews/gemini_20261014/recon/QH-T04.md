# QH-T04
| fid | verdict | evidence (path:line) | fix_sound | note (<= 20 words) |
|---|---|---|---|---|
| QH-2a-04 | PARTIAL | patch_manager_mods/queue_hud_src/QueueHud.cpp:1116-1118 (midFlight); :1131-1140 (clear) | no | Gap real: midFlight reads only e[]. In-flight kill unproven; headHeld wait covers held head. Fix adds unbounded wait. |
- QueueHud.cpp:1116-1117: midFlight loop scans only s.e[0..engineNe) for T_USE_ITEM / T_CAST_SPELL. No s.rn or s.cur test. Anchor text matches verbatim at 1117.
- QueueHud.cpp:974-976: engineHoldsAction also checks s.rn[] subType 9/10 and s.cur subType 9/10. midFlight does not, so the gap is literal.
- QueueHud.cpp:1118: midFlight `continue`s the handoff for this member before the send path.
- QueueHud.cpp:1131-1140: clearBeforeHandoff(handoffClearFirst, h.sent) gates ServerClearAllActions. Reached only when midFlight is false.
- QueueHud.cpp:1103-1106: when headValid && headHeld and heldHeadAction returns HELD_WAIT, the loop continues before midFlight. Held head's 9/10 hold is covered there only.
- QueueHud.cpp:1112-1113: HELD_RESEND / HELD_DROP clear headHeld; DROP sets headValid false. A still-running 9/10 node then reaches the unguarded midFlight path. Depends on heldHeadAction (outside range).
- QueueHud.cpp:1130 comment: clear exists so the hand-off is not queued behind engine attack entries (e[] intent). Suggests e[]-only midFlight may be deliberate.
- QueueHud.cpp:979 comment: 9/10 flags "stick in a frozen round". Adding 9/10 to midFlight with an unbounded `continue` risks a stall. Hence fix_sound no.
- Not verified (outside ranges): numeric values of qh::T_USE_ITEM / T_CAST_SPELL (Gemini says 0x2E / 0x0F), heldHeadAction, clearBeforeHandoff, ServerClearAllActions engine semantics.
OPEN: Does ServerClearAllActions cancel a running round node (subtype 9/10), or only queued e[] entries? Needs code outside ranges.
