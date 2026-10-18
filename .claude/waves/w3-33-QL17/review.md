# QL17 review (queue_hud_src uncommitted diff vs synthesis B1/B2/B3)
Files: Q=QueueHud.cpp L=QueueHudLogic.h T=queue_hud_logic_test.cpp. Verdict: APPROVE, 0 BLOCKING, 6 MINOR.

## B1 hold non-slot-0 owners
Call-site sweep (dispatchAction 1276/1734/1784/1847; ServerClearAllActions 950/1331; no other Perform* caller):
- 1847 sequential dispatch: gated by DS_NOT_CTRL (1805). OK.
- 1734 RI_OK re-inject: gated by RI_NOT_CTRL (1704). OK.
- 1784 AV_EATEN_REINJECT: downgraded to EATEN_WAIT (1743). OK.
- 1276 + 1331 hand-off send/clear: gated at the edge (1202) and per frame (1252). OK.
- 950 Cancel click ServerClearAll: user action, per member k. Not a dispatch. OK.
- ownerCtrl (1651) is computed after the identity pre-pass, so for a busy buffer it is effectively (k == 0). bufOwner reads queue[0] only when bufBusy (count>0 or active): safe at 1202/1651. Disabled-feature path unaffected.
- g_identLostMs (1624) only runs when the owner is in NO slot, not for "merely not leader". Not a regression.
- g_handoffHold reset: set at the edge hold (1203) but the second loop (1245, !pending) resets it the same call, so the flag only dedups the pending-path trace; edge hold traces once per edge anyway. Correct, flag is redundant on the edge path.
MINOR-1 Q:1297/1254: pending hand-off with headValid=false and an empty buffer (HELD_DROP, nothing buffered) now evaluates bufOwner = default queue[0] (creature 0) -> slot -1 -> HANDOFF_HOLD owner_slot=-1 and `continue`, so `!buf.pop -> pending=false` is never reached; the Handoff stays pending (g_handoff pending also feeds journal busy at 1458) until combat ends (1636 clears). Fix: in the pending hold block skip the check when !h.headValid && buf.count==0 (let it fall to the pop/clear path).
MINOR-2 Q:1743 + L:1438 AV_GIVE_UP: a held ACTIVE action of a demoted owner is eaten by follow-leader; with EATEN_REINJECT turned into EATEN_WAIT, eatenUnpausedMs still accrues and after kEatenWaitMaxMs (20 s unpaused) the active action is dropped (DR_EATEN_TIMEOUT). Buffered (queued) entries are NOT dropped (endAction keeps them). Acceptable (the active action was already eaten), but note: "hold" is not unlimited for the active cmd. Also AV_UNSEEN/EXECUTED resolveActive still run for non-ctrl; harmless (no dispatch).
MINOR-3 Q:1202-1203 / 1331: ownership check is by owner slot, but srv/ServerClearAllActions at 1331 use member k. In combat the identity pre-pass does not run, so a buffer at index k whose owner is slot 0 (leader switched in the last frames before combat, k != 0) passes the check and would ServerClearAll member k's (a follower's) queue while dispatching on the leader. Narrow window (1 frame after a switch), pre-existing indexing; cheap fix: require ownerSlot == k (or use the owner's server creature).
MINOR-4 Q:1805-1818 WAIT heartbeat: a buffer held for a long time on slots 1/2 logs one WAIT line per second per member (trace_queue) until the owner leads again. Bounded by existing heartbeat pattern, not per frame; consider a longer period for why=not_ctrl.

## B2 BTN_CLEARALL clone
- Every g_cw[0].c use: cwShow setClickable/setVisible (GuiKit.h:251,260: flag bits on shared control base), setRect (vtable slot via Ctl_VtSetRect, also used on ModConfig buttons), hit/getExtent (shared Ctl_X..H), guitip::setNative (Ctl_TipText on base) - valid for CSWGuiButton. cwLookLeader branches on w.btn and skips setFont/setAlign/setText/setTextColor/setBorderTint/setAlpha/setFill (label-only). cwLookCompanion is k>=1 only. OK.
- Hover: setButtonFill (GuiKit.h:420) writes image+tint to BOTH the normal (+0x70) and highlight (+0xE8) border, gated up front; hover swaps tint for both. OK. setButtonFillStyle also both.
- Rebuild: `for (CW& w : g_cw) w = CW()` (533) resets btn; the old panel owns/destroys its appended controls, same as the label path. No per-rebuild leak beyond the existing label behaviour.
- Failure paths: canary fail -> releaseUnlinkedButton (matches ModConfig cloneEntry). Append failure -> button left hidden+click-through and unreferenced (not released; label comment-consistent, release could double-free if partially linked). Fallback label path OK.
MINOR-5 Q:424-428 setButtonFill/setButtonFillStyle/setButtonText return values ignored and w.fill=want is set regardless: a refused write leaves the template look and is never retried or logged. Add one-shot WARN on first failure (chCancel).
Live-only risks (not blocking): engine now routes real button click/hover to the panel (id = list index, past the vanilla ids) and may play its own hover/click sound on top of playHoverSound (Q:1971); verify no HUD side effect on a click and no double sound.

## B3 head icon
- L:261-282 stripHeadSize/stripRowWidth/stripRects: left row right edge == vit.x - reserve (tested), head centred on vit, head <= vit.h so no card overflow; above: bottom edges aligned at card.y-gap; below: top edges aligned at card.y+card.h+gap (tested). pct=100 pinned byte-identical on 3 sides x n=1..4 (T). Clamp 100..200 in parser AND stripHeadSize.
- Change-gate sig (Q:809) folds r[0].w/x/y, so a head size change repaints; held strip copies cancelSq with StripDraw (struct copy), default 0 keeps old behaviour. hud-host stripSlot rowH uses head size, reserve recomputed consistently (Q:802-804).
- Cancel: sq = stripIconSize, centred on head (T pinned), sq=0 = old behaviour (T pinned).
- slotStyle waitingIconDim: default true keeps existing asserts; ini ships waiting_icon_dim=0 / strip_head_pct=150 uncommented (as with show_in_combat) while the ini header says "values below are the defaults": wording only.
MINOR-6 Q/L above-side: the head row now extends hs (87) above the card instead of s (58); no screen-top clamp (pre-existing for s), and left-side default is unaffected.

## General
- No new hooks/exports; no per-frame logging added (new lines: BUILD, HANDOFF_HOLD once per episode, WAIT edge/1 s existing).
- Version bump 0.4.3-dev in L/Q/ini/manifest consistent. Tests: pure parts covered (ownerMayDispatch, why enums + numbering, fmtHandoffHold incl. truncation, slotStyle, stripHeadSize, stripRects/RowWidth/CancelRect, ini keys). Caller-side post-checks (Q) and B2 button branch are untestable on host (live rows needed).
