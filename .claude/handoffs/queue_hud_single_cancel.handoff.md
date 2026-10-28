# Handoff: queue-hud per-icon cancel on companion strips (2026-10-28)

## Ask (user, verbatim)
"click to cancel from the companion queue, single items not the X for everything"
= LMB on ONE icon of a companion strip (party k=1..2, drawn left of the portrait vit bar) removes only that queued action.
The existing Cancel X (doCancel -> ClearAllActions for the whole member) stays as is.

## Live state
- queue-hud 0.4.10-dev LIVE 10-25 09:53, dll 1d163450 (repo de4c0b6c, strip order 95b89696, KPM 0371e7a on ethansm/local-mods).
- Rollback: `python3 scripts/kpm_hotswap.py --rollback queue-hud_swap_20261025_095305`.
- Pending play rows: QH18 (strip order), QH25 (combat tail), QH26 (soft latch) in patch_manager_mods/PLAY_TEST_QUEUE.md.
- Next version for this feature: 0.4.11-dev (manifest.toml, QueueHud.cpp line 1 + KPatchInit log, queue_hud.ini line 1).

## Code anchors (patch_manager_mods/queue_hud_src)
- Strip geometry: `qh::stripRects` (QueueHudLogic.h ~325): index 0 = head at the RIGHT end next to the portrait, x decreases with i. Cancel X: `qh::stripCancelRect`.
- Strip draw: `drawStrips` (QueueHud.cpp ~765). Per member `StripDraw d[k]` has `r[j]` rects, `rr[j]` resrefs, `n`; slots come from `qh::buildSlots(s.e, s.ne, s.rn, s.nrn, cur, slots)`.
  `Slot{entry, node, key}`: entry < s.engineNe = engine queue entry; entry >= engineNe = OUR buffered entry (active buffered first if `activeNeedsSlot`, then `g_itemQueue[k].queue[0..count)`). node: NODE_CUR (-2), NODE_NONE (-1), or round-list index.
  The draw loop drops slots whose icon fails to resolve, so `sd.r[j]` index != slot index: keep a parallel slot map (entry/node per drawn j) when adding hit-test.
- Clicks: `onFrameBegin` (QueueHud.cpp ~2290) polls `guikit::clickEdge(g_lmb)`, `guikit::cursorEngine(&cx,&cy)` (engine px, same space as strip rects), hover/hit only for the Cancel controls `g_cw[k]` -> `doCancel(k)`. `g_lastClickTick` + `cancelClickAt` (RQ11) feed hand-off cancel detection: a per-icon click must be added to `cancelClickAt` rects or deliberately excluded (decide; hand-off logic treats a Cancel click as "drop whole queue").
  Strips are drawn in HUD_DRAW (drawAll); clicks are handled in FRAME_BEGIN (outside the draw batch): store last frame's strip rects + slot map in a global for the click pass, and gate on the 250 ms capture age, menu, hardCombat (strips also show in combat via show_in_combat=1: decide whether single-cancel works in combat; Cancel X is hidden in hard combat).
- Our buffer: `qh::ActionBuffer` (QueueHudLogic.h ~703): `queue[kMaxActionQueue]`, `count`, `active`, `activeAction`, many verdict fields; has `cancel()`/`clear()`. Need a pure `removeAt(buf, i)` (shift queue, keep order) + tests; removing the ACTIVE buffered one = its engine action too.
  Log a DROP per removed cmd (`traceDropAll` pattern, DropWhy DR_CANCEL or a new DR_CANCEL_ONE; the log checker scripts/queue_hud_log_check.py has strict grammar regexes, add new tokens there + scripts/test_queue_hud_log_check.py).
- Engine queue items (entry < engineNe): no per-slot engine API known. Doc 30 QL5 recon: BTN_CLEARONE (hud+0x689C) -> 0x752330 -> 0x752360 -> 0x7511B0 -> CSWSCombatRound::RemoveLastAction 0x5904C0 (round list) else 0x541080(1): removes the LAST action only, not per slot.
  Round-node removal without ClearAllActions exists: the detach code (QueueHud.cpp ~1603, "Take the round nodes selected by removeMask ... WITHOUT ServerClearAllActions") using engine list thunks; guards in the "round-list writes" helper (~1595). Reuse for round-node slots (node >= 0).
  Engine action-queue entry removal (non-round 0x0F/0x2E entries owned by the engine) is unresolved: RE needed (static_facts / query_ghidra first, rea-recon last) or scope v1 to buffered + round-node slots and make other icons ignore clicks (no hover).
- Vanilla rule to keep: never flip hud+0xA904 bit 1; menu frames never act; Cancel X behaviour unchanged.

## Suggested plan shape
1. Pure logic + tests: `removeAt`, a strip hit-test `qh::stripHit(rects, n, x, y)` (returns j or -1), slot-kind classifier (buffered / round node / engine entry / cur).
2. cpp: persist per-member drawn slot map, FRAME_BEGIN click -> classify -> remove; hover highlight (kit_frame_hi) on removable icons; hover sound `guikit::playHoverSound`.
3. Logs: `CANCEL_ONE m= cmd= kind= j=` line + checker grammar.
4. Gate `scripts/pre_install_gate.sh --mod queue_hud_src`, `kpm_hotswap.py --mod queue-hud --prepare`, commit both repos with explicit paths, ask GO, `--apply`.
5. Play row QH27 in PLAY_TEST_QUEUE.md.

## Open decisions for the user
- Should single-cancel work in combat (strips stay visible there)?
- Engine-owned icons (not buffered by us): v1 no-op, or RE a per-entry removal first?

## Status 2026-10-28 (QH27 built)
- User answers: single-cancel works in combat too; engine-owned icons ignore clicks in v1 (no RE).
- queue-hud 0.4.11-dev committed d40aa8f7 (repo), KPM 159d077; kpm_status = SWAP. --apply refused: game was running. User gave GO with gate waiver.
- Gate: step 1 fails on peer commits (level-up-override WI-164 exports/compile, character-effects-panel raw addresses); Wine smoke wire/combat/badhud fail IDENTICALLY on committed 0.4.10 (smoke_queuehud.cpp pinned to 0.4.0 strings, stale) -> smoke refresh is a follow-up.
- Removable kinds: buffered, active buffered (dropActive), pending round node (removeRoundNodes + drops matching active / hand-off head). Invisible LBL_QUEUE1 click-eater label per companion row (alpha 0) so clicks never reach the world.
- LIVE 2026-10-28 01:21 (dll 7d4bf251, rollback queue-hud_swap_20261028_012153). Next: play QH27 (+ QH18/25/26), then `python3 scripts/kotor2_logs.py check queue_hud --new`.
