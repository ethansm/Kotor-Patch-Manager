# Review RV12: plan_RV1.md + plan_RV2.md (queue-hud 0.4.5-dev -> 0.4.7-dev)

Reviewer: Sonnet skeptic. Checked against the working tree at 76653c96 (QueueHud.cpp 2450 lines, QueueHudLogic.h 1534 lines) plus `scripts/queue_hud_log_check.py`.

## Confirmed OK (no action)
- RQ1 replay arg order. `decomp_7791a0.txt:77` calls `FUN_00775700(spellId, target, &zero3, 0, 0, 0, 0)`. FINDINGS.md Q3 gives `(spell, targetObjId, float* pos, byte, byte, byte, int)`. The plan's new typedef and replay call match, and so does the hook's read order (+8 spell, +0xC target, +0x10 pos*, +0x14/+0x18/+0x1C bytes, +0x20 int flag).
- `pos` is never NULL at replay: `ra.pos` is `a.pos`, an inline array. At capture an unreadable or NULL pointer gives zeros. The Item hook is unchanged and its +0x10 really is `targetId`.
- No other module calls `PerformCastSpell`, `makeSpellAction` or `fmtSpellArgs`. The perl regex rewrites all `makeSpellAction` test call sites except `:2674`/`:2676`, which carry a parenthesised `(int)` arg and which the plan handles by hand.
- RV2 deviation 1 is correct: both hooks return at `g_combat != 0` (`QueueHud.cpp:2268-2271`, `:2355-2358`), so the in-combat half of RQ6 is dead code.
- `removeRoundNodes` uses only `ActListRemoveHead`/`ActListAddTail` (GameAddr.h:92-93), the `roundListHandle` guards (CrRunning == 0), the identity check and a leak without re-add. That is the same pattern as `detachRound` + `discardDetached`. The ORPHAN clock cannot fire while paused: `tick()` returns the frozen ms, which is below 1500 because the tick that reached it already fired.
- `detachRound` push-back of the head node: the list is empty after the pop loop, so pushing kept nodes in ascending index keeps their relative order. Later foreign re-adds go behind them via `detachTick`. No order problem.
- Test vectors recomputed by hand: `testReInjectGate`, `testDeferStep`, `testEdgeHeadAction`, `testOrphanRoundAge`, `testFlagGatePaused`, `testTotalReInjectCap`, the `accrueEaten` 2500 total, and the `fmtSpellArgs` goldens (1.0f = 3f800000, 2.0f = 40000000, 3.0f = 40400000). All follow from the After-code.

## R1 [BLOCKING] plan_RV1 §RQ5 tick wiring item 3 (RI_OK QLOG)
Problem: the plan extends the follower-wipe re-inject line to `"... attempt=%d) cmd=%u why=ri_ok paused=%d"`. The checker's `_REINJECT_OLD` is `...\bcmd=(\d+)(?: why=(\w+))?$`, so a trailing ` paused=0` stops the line matching. The REINJECT record is lost, and the lifecycle, flag D and flag G views lose it. Even if the regex were loosened, `why=ri_ok` is not `ok`/`eaten_paused`, so `render_default` (`queue_hud_log_check.py:628`) and `build_cmds` (`:376-384`) would count every follower-wipe re-inject as a DECLINE. The plan's "no parser change needed" is wrong for this line.
Evidence: `scripts/queue_hud_log_check.py:191,196-203,348-349,628`.
Required correction: keep the line tail exactly as today. Put the new field before `cmd`: `"re-injecting eaten %s action for member %d (id=%08x, target=%08x, attempt=%d, paused=%d) cmd=%u"` and add no `why=`. The checker then needs no change and `normalize_old` still extracts `attempt=`. Delete the "`why=ri_ok`" wording from the plan.

## R2 [BLOCKING] plan_RV1 §RQ2 (new `DROP why=executed`) vs checker flag I
Problem: for `EDGE_DROP_EXEC` the edge block prints `HANDOFF ... active=<cmd>` and then only `DROP why=executed`. The head is never put in the held branch, so there is no `HANDOFF_ONE` and no `HANDOFF_HELD act=drop`. `handoff_trigger_flags` (flag I) then reports "active never re-sent (trigger lost)" for exactly the case QH23 expects to pass.
Evidence: `scripts/queue_hud_log_check.py:463-475`; `handoff_windows` at `:416-427` only collects the listed keywords.
Required correction: add to the RV1 checker section:
- `handoff_trigger_flags`: call `handoff_windows(recs, ("HANDOFF_ONE", "HANDOFF_HELD", "DROP"))` and extend `settled` with `(r["kw"] == "DROP" and r["f"].get("why") in ("executed", "refused"))`.
- `test_queue_hud_log_check.py`: add `test_I_quiet_when_active_dropped_executed` next to `:882-889`.
- Docstring line 65: add "or DROP cmd=<n> why=executed|refused".

## R3 [BLOCKING] plan_RV2 §RL8 kickStarved give-up and §RQ4 removal scope (other mods' nodes, never-sent action)
Problem 1: `kickStarved` runs whenever `k > 0 && buf.active`. With RV1's deferral, `buf.active` can be an action that was never sent. If another mod's 0x3F (companion-selfheal item use) starves behind 0x3D, `at != 0` and the new clock reaches 5 s. The give-up then drops an action the engine never saw, with a false `DROP why=eaten_timeout`. It also pops EVERY round node, including selfheal's, so selfheal's pending use times out with a dead 0x3F left in the queue.
Problem 2: `removeRoundNodes` takes all `s.nrn` nodes, whatever their subType. The ORPHAN condition only proves that the 9/10 nodes are orphans. A node of another subtype may have a consumer entry that `entryConsumesRound` does not model.
Evidence: `QueueHud.cpp:1809` (kick runs for any active k>0); `CompanionSelfHeal.cpp:~805-870` (selfheal rotates and gives up on its own 5 s clock, `actq::starved` cap k<=3).
Required correction:
1. Give `removeRoundNodes` a keep-mask like RV1's `detachRound`: `int removeRoundNodes(uintptr_t srv, const Snapshot& s, unsigned removeMask)`. Pop all `s.nrn`, push back every node whose bit is clear, and return the count removed (0 if the mask is empty, before touching the list).
2. ORPHAN: `mask` = bits of nodes with `subType == 9 || 10`.
3. Kick give-up: `mask` = nodes matching the active action via RV1's `nodeIsHead(s.rn[i], buf.activeAction)` (first match only, like `detachableMask`). If `buf.dispatchDeferred` or the mask is 0, do not give up. Reset `g_kickClk[k]` and return false, because the starved 0x3F is not ours.
4. `kickStarved(...)`: add `if (buf.dispatchDeferred || !buf.active) { g_kickClk[k].reset(); return false; }` as the first line.

## R4 [MINOR] plan_RV1 §RQ5 deferred retry x plan_RV2 `markDispatched` reset
Problem: RV1 calls `buf.markDispatched(now)` when a deferred retry succeeds. RV2 makes `markDispatched` also zero `eatenReInjects`, `eatenUnpausedMs`, `lastReInjectTick`, `totalReInjects`. A re-inject that is refused, deferred and then sent wipes the per-action budget, so the RL5 total cap and the spacing gate are bypassed on that path.
Required correction: in RV2's "RV1 overlap" step, change the deferred-retry call to `buf.markReInjected(now, qh::flagBits(s.aa8, s.ab8))`. The text "if RV1 added a markDispatched call it keeps flags=-1" is wrong for this call. The AV_EATEN_REINJECT RV1 form `markDispatched(now); afterSend(...)` becomes `markReInjected(now, ...)` as RV2 already says.

## R5 [MINOR] plan_RV2 §RL8 `g_kickClk` stale ms
Problem: `kickStarved` is only called while `k > 0 && buf.active && companionServerCast`. If the action ends (resolve, cancel, hand-off) while the clock holds e.g. 3000 ms, the value survives (a later gap only stops accrual, it does not reset). The next starved cast then gives up after 2 s instead of 5 s.
Required correction: at the call site add `else g_kickClk[k].reset();`, i.e. `if (k > 0 && buf.active && g_cfg.companionServerCast) { if (kickStarved(...)) continue; } else g_kickClk[k].reset();`.

## R6 [MINOR] plan_RV2 §RQ7 `sawExpanded` makes a missed execution a re-send (double-use window)
Problem: an expanded 0x0F/0x2E node is no longer `executing`, and rule 3 skips UNSEEN when `sawExpanded`. If the cast or use finishes while no tick sees a matching cur node or a risen flag, the action reads as eaten and is re-injected. Candidates are a frame hitch longer than the cast (the plan itself caps gaps at 500 ms) or a hook-dispatched action whose flags rose before the first tick, so `noteFlags` baselines them in. That is a second medpac or cast. The old behaviour was fail-safe (lost, never doubled).
Required correction: keep the design, but bound the damage. In `activeVerdict` rule 6 use `buf.sawExpanded ? 1 : kMaxEatenReInject` as the re-inject limit. Also add `sawExpanded && eatenReInjects==1 -> AV_GIVE_UP` to `testActiveVerdictRules`. QV-EX1 must also record the stack count before and after.

## R7 [MINOR] plan_RV1 §RQ3 kept head node vs combat-start wipe
Problem: doc 47 and RV2's own deviation say ClearAllActions never empties the round list. If the combat-start wipe removes the head's queue entry but not its round node, the kept node (a) keeps `engineHoldsAction` true, so the hand-off waits until `h.heldUnpausedMs > kHeldStuckMs` (3 s) and then HELD_DROPs: the head is lost, but it is never doubled; (b) leaves an orphan 9/10 node. If the buffer is idle, RV2's ORPHAN (gated on `bufBusy`) never removes it. The plan's edge-case sentence "wipe that removed the node -> all detachable" ignores this branch.
Required correction: add to QH24 pass criteria: "if `HANDOFF_HELD act=drop` follows after about 3 s with no `HANDOFF_ONE`, record it (OPEN: wipe leaves the node)". Add to the plan's OPEN list: "orphan head node after wipe, tied to RV2 ORPHAN (consider running the orphan clock when `h.pending` too)". The item-id low-24-bit match is an unproven assumption but fails safe (no match = old detach-all behaviour).

## R8 [MINOR] plan_RV2 §RQ6 RESOLVE why=combat_end vs checker flag C
Problem: a held action resolved at `combat_end` can have `seen=0` and `reinj>0`, which flag C (`queue_hud_log_check.py:309-313`) reports as "unseen-reinject".
Required correction: in `qj_flags` skip flag C when `f.get("why") == "combat_end"`, and add a test.

## R9 [MINOR] Before-snippets that do not match the code literally
- RV2 `accrueEaten` Before is quoted as one line. The real code is:
  ```
  if (eaten && !paused) {
      const uint32_t d = now - lastVerdictTick;
      eatenUnpausedMs += d < 1000u ? d : 1000u;
  }
  lastVerdictTick = now;
  ```
  Tell the executor to replace exactly that block (`QueueHudLogic.h`, in `accrueEaten`).
- RV1 `serverCast` Before: the code has `const qh::QueuedAction& a`, not `const QueuedAction& a`.
- RV2 "Before `dispatchAction(buf.activeAction, k);\n buf.markDispatched(now);`" no longer exists after RV1. The text is then `const SendResult r = dispatchAction(...); buf.markDispatched(now); afterSend(...)`. Anchor on `buf.markDispatched(now);` inside `case qh::AV_EATEN_REINJECT:`.

## Verdict
BLOCKING: R1, R2, R3. Apply the corrections above (R1 is a one-line format change, R2 is checker plus test, R3 is a keep-mask on `removeRoundNodes` plus deferred and identity gates) before the executor starts; R4-R9 are one-line fixes to fold in. With those, RV1/RV2 are consistent with the code, the engine-call set and the hook stack layout.
