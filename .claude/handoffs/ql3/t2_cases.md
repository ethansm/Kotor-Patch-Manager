# T2 recon: combat hand-off, trigger coverage per action source

Static read only. Scope: QueueHud.cpp 926-1025, 1505-1565, 1587-1640; QueueHudLogic.h 140, 663-668. Hooks: `patch_manager_mods/queue_hud_src/kotor2-steam-aspyr.hooks.toml:6-12` (H9 PerformUseItem 0x775603), `:25-31` (H10 PerformCastSpell 0x775703). No live logs read.

## Table

| Row | Hooked | In g_itemQueue | Engine entry at combat edge | Re-sent by hand-off | Killed by ServerClearAllActions (QueueHud.cpp:1021) |
|---|---|---|---|---|---|
| Attack click | No. Only H9/H10 are hooked (hooks.toml:6, :25) | No. Only H9/H10 push (QueueHud.cpp:1568, :1656) | 0x3F per comment QueueHud.cpp:925 (not measured here) | No. Never buffered, so no headValid and no buf.pop | Yes, if the same member has a busy buffer at the edge (gate :958, clear :1012-1021). GameAddr.h:84 says the clear empties cr+0x100 and the round list. Otherwise no mod-side kill |
| Force power (0x0F) | Yes, H10 (hooks.toml:25-31; QueueHud.cpp:1587) | Yes. Buffered copy pushed at :1656. Dispatch-now sets active at :1644 | Buffered: none queued yet. Dispatch-now: 0x0F (QueueHudLogic.h:44) | Buffered: yes (pop :1002, after clear). Dispatch-now: yes only if engine does not hold it at the edge (:965-970). If engine holds it: NO, silently (endHandoff traces only headValid, :939) | Buffered copy: no (mod memory). Engine copy: clear is deferred while 0x0F/0x2E is queued (midFlight :996-999) |
| Item / medpack (0x2E) | Yes, H9 (hooks.toml:6-12; QueueHud.cpp:1505) | Yes. Push :1568. Dispatch-now sets active :1556 | Buffered: none. Dispatch-now: 0x2E (QueueHudLogic.h:45), preceded by a 0x3F round entry (patch_manager_mods/27_companion_selfheal_evidence/R1_actor_route/FINDINGS.md:18-19) | Same as force power | Same as force power |
| Action-hotbar click | Item slots: yes. ActionHotbar.cpp:614 calls PerformUseItem at 0x775600, which runs into the H9 site at +3. Spell slots: go through HudInvokeGroup (ActionHotbar.cpp:510). H10 coverage UNVERIFIED | Same as the underlying row | Same as the underlying row. A quickbar press wipes the engine queue (QueueHud.cpp:259 comment) | Same as the underlying row | Same as the underlying row |
| Companion command | No H9/H10 call site in companion_ai_src (grep finds none). Companion item use goes through engine UseItem 0x5648D0 (FINDINGS.md:18) | No | 0x3F, then 0x2E built by AIActionCombat (FINDINGS.md:19). Follow-leader 0x3D (QueueHud.cpp:1163) | No | Yes for that member if it has a busy buffer. Clear deferred while 0x2E/0x0F is queued (:996-999) |

## Lost rows and why

1. Attack click. It is never hooked or buffered, so no hand-off path can re-send it. Its engine 0x3F is cleared at :1021 when the same member has a busy buffer. Combat start also wipes the engine queue (comment at :1011).
2. Companion command. It is never hooked or buffered. Its engine entries are cleared at :1021 for that member, and nothing re-sends them.
3. Dispatched-now force power or item. If the engine holds 0x0F/0x2E or round subtype 9/10 at the edge (:965-969), the head is not re-sent and no DROP line is written (:939). It is lost only if the engine drops it at combat start. Timing NOT verified.
4. Action-hotbar spell slot. Lost only if HudInvokeGroup does not reach H10. UNVERIFIED.

Not lost: buffered (not yet dispatched) force power and item. They are popped after the clear (:1002, :1012-1021), and ServerClearAllActions does not touch the mod buffer.

## Summary

1. Attack click is never hooked or buffered, so the hand-off cannot re-send it.
2. Companion commands are not hooked, and their engine entries are cleared at QueueHud.cpp:1021.
3. A dispatched-now item or spell is silently dropped if the engine holds it at the edge (QueueHud.cpp:965-969, :939).
4. Buffered item and spell actions are re-sent after the clear (QueueHud.cpp:1002, :1012-1021).
5. Open: does the engine drop an executing 0x0F/0x2E at combat start; and does the action-hotbar spell slot reach H10 (ActionHotbar.cpp:510)?
