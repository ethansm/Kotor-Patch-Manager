# 47 — Review: non-combat action bar, action queue, companion queue (2026-10-23)

Scope: queue-hud 0.4.5-dev (QL20 LIVE 10-23 15:44, dll 99daa99e) and action-hotbar 0.1.4-dev, plus their interplay with
companion-selfheal. Method: 12 parallel Haiku reviewers (overlapping windows, 113 raw findings) -> 4 Sonnet validators
(code-checked, merged, 23 rejected) -> Opus adjudication (spot-checked the top items in code, re-graded).
Raw and validated material: `.claude/reviews/rv_qac_20261023/` (`haiku/h01-h12.md`, `v01-v04.md`; each issue below names
its V-id so the full Where/Scenario/Test text can be looked up there).

Status legend: CONFIRMED = code provably does it and the path is reachable; PLAUSIBLE = code does it, reachability rests on
an unverified engine behaviour (named). Severity: high = wrong game action (double use, lost/wrong cast, stuck queue needing
Cancel); med = visible misbehaviour with a workaround or a rare path; low = logging, docs, cosmetic.

Field names: in queue-hud `QueuedAction`, `arg2` = target client id, `targetId` = the pos POINTER captured at click,
`posPtr`/`hasPos` = derived from the 7th (int flag) stack arg. PerformCastSpell 0x775700 real stack order:
(spellId, targetClientId, float* pos, multiclass, meta, domain, int flag).

## Summary table

| ID | Sev | Verdict | Area | One line | V-ids |
|---|---|---|---|---|---|
| RQ1 | high | CONFIRMED | queue capture/replay | Replayed spell passes the click-time stack address as `pos`; the builder dereferences it on every cast | V3-1, V3-3 |
| RQ2 | high | PLAUSIBLE | combat hand-off | Edge frame re-sends a non-held head that already ran (double use) | V2-1 |
| RQ3 | high | PLAUSIBLE | combat hand-off | `handoff_detach_held` can detach the held head's own round node -> head runs twice | V2-2 |
| RQ4 | high | PLAUSIBLE | dispatch verdict | Orphan round node (9/10) keeps the active action PRESENT forever; queue wedged until Cancel | V1-1 |
| RQ5 | med | CONFIRMED | dispatch / QL20 | RI_OK re-inject ignores pause/settle/spacing; refused serverCast re-sent twice then lost with no DROP | V1-3, V2-3, V1-10 |
| RQ6 | med | PLAUSIBLE | dispatch | In-combat pass-through clicks stay tracked as `active`; the last one can be re-sent after combat | V1-2 |
| RQ7 | med | PLAUSIBLE | verdict | `executing` evidence is creature-wide; a foreign flag latches sawExecuting -> eaten action resolves EXECUTED (lost) | V1-4 |
| RQ8 | med | CONFIRMED | hand-off | Unsent hand-off head is dropped at combat end (DR_COMBAT_CLEAR) | V2-6 |
| RQ9 | med | CONFIRMED | hand-off | Detached round nodes stranded when combat ends (detachTick runs only in combat) | V2-7 |
| RQ10 | med | PLAUSIBLE | hand-off | Detached nodes keyed by slot: leader switch pushes them into another creature | V1-5, V2-9 |
| RQ11 | med | PLAUSIBLE | hand-off | Any left click within 600 ms counts as "Cancel" and wipes the member's buffer | V2-5 |
| RQ12 | med | PLAUSIBLE | Cancel | Companion strip Cancel clears its resting 0x3D FollowLeader; nothing re-adds it | V3-2 |
| RQ13 | med | CONFIRMED | cross-mod | selfheal dedupe blind to queued spells; Cancel leaves selfheal "pending" 6 s | V2-4, V2-8 |
| RH1 | med | CONFIRMED | hotbar persave | Save with no snapshot keeps the previous save's bar (docs promise empty) | V4-1 |
| RH2 | med | CONFIRMED | hotbar layout | Exact 4:3 windows other than 800x600 get the identity scale; bar misplaced/small | V4-2 |
| RH3 | med | CONFIRMED | hotbar editor | Failed Save in assignSlot leaves g_cfg mutated; refused edit lands on the next save | V4-4 |
| RH4 | med | PLAUSIBLE | hotbar x queue | @lowhp target chosen at press; two quick presses heal the same member | V4-5 |
| RH5 | med | PLAUSIBLE | hotbar input | No conversation gate for digits 1-3 (only indirect panel/tick gates); never play-tested | V4-3 |
| RL* | low | mixed | both | 31 low items: bookkeeping resets, logging blind spots, docs/ini drift, UX | see "Low" |

Rejected (23): listed with refuting lines at the end of each `v0N.md`. Notable: Haiku's ground-target claims (no K2 caller
passes a real ground point), action-hotbar @self id validation (would break the Exile's 0xFFFFFFFx id), kMaxRound overflow.

---

## High

### RQ1 — Spell replay passes a dead stack address as `pos` (CONFIRMED, high)
- Where: `queue_hud_src/QueueHud.cpp:81` (typedef: 3rd arg named `targetId`, 7th `pos`); `:2371-2382` (hook stores
  ebp+0x10 raw into `targetId`, dereferences ebp+0x20 (the int flag) as a pointer); `:1113-1129` (replay passes `a.targetId`
  in the pos slot and `replayPos()` in the flag slot); `QueueHudLogic.h:626-652` (makeSpellAction, replayPos);
  `_shared_src/GameAddr.h:71`; doc 30:78.
- Defect: builder 0x87A5C0 calls 0x44B8A0(pos,pos) unconditionally (`decomp_87a5c0.txt:55`); above a threshold it sets
  "has location" and serializes pos (server 0x65BE70 case 0x11 -> AddCastSpellActions with a location). Every vanilla
  caller passes a pointer to a zeroed float[3] and flag 0 (`decomp_7791a0.txt:77`, `disasm_899270.txt:69-77`). queue-hud's
  replay hands the builder the caller's old stack address, so it reads whatever lives there now. A dword that reads as a
  big float (object ids 0x7Fxxxxxx ~1.7e38) can make the cast location-targeted (wrong or no cast, nondeterministic). The flag
  slot is 0 today only because `hasPos` is always false; with `spell_replay_pos=0` it becomes a nonzero pointer = bit 4 =
  server skips the cast (that was QL9).
- Opus check: confirmed in code (I read both the hook and the replay). Not seen to misfire in logs, but it is undefined
  behaviour on every replayed cast. The server route (QL20 serverCast) does not use pos and is unaffected.
- Fix: retype `PerformCastSpellFn` as (creature, spellId, targetClientId, const float* pos, multiclass, meta, domain, int flag);
  at capture copy the 3 floats behind ebp+0x10 into `a.pos` and read ebp+0x20 as an int into a new `a.flag`; replay with
  `a.pos` (always a valid pointer, never NULL) and `a.flag`. Delete `replayPos`/`hasPos`/`spell_replay_pos` (key accepted
  and ignored with one INFO line). Rename the field `targetId` usage for spells (stop storing the pointer). Fix
  `fmtSpellArgs` to log `tgt=<arg2> pos=x,y,z flag=<n> posaddr=<raw>`.
- Test: pure `buildReplayArgs(a)` -> {pos ptr == &a.pos, flag}; vectors pos {1,2,3}/flag 0 and posZero/flag 5; golden
  fmtSpellArgs string. PLAY: pause, queue Force Heal (self) + Force Push (enemy), unpause: SPELL_ARGS hook vs replay show
  equal pos (0,0,0) and flag 0; both cast once.

### RQ2 — Combat-start edge re-sends a head that already ran (PLAUSIBLE, high)
- Where: `QueueHud.cpp:1322-1345` (`h.headValid = buf.active && (!inEngine || h.headHeld)`; `h.sawExec` computed at :1331
  but only used on the held path); `QueueHudLogic.h:1476-1479` (150 ms vanish debounce, 500 ms kMinActiveMs).
- Defect: a non-held head (`buf.active && !inEngine`) is always re-sent at the edge. `buf.sawExecuting` and `h.sawExec`
  are ignored, so a command that finished inside the not-yet-resolved window is sent again. `inEngine` is also false when
  `gatherFor` failed (`gEdgeOk=false`), which re-sends a command that IS in the engine.
- Opus check: confirmed the code shape. Reach needs combat to begin within ~150-650 ms after the command left the engine,
  or within the 1-2 frames before the server shows it (unmeasured). Cheap to guard.
- Fix: new pure `edgeHeadAction(active, inEngine, sawExecuting, sawExec, ageMs, gatherOk)` -> {DROP_EXEC, WAIT, RESEND}:
  `!gatherOk` -> WAIT (treat as held); `!inEngine && (sawExecuting || sawExec)` -> DROP_EXEC (log DROP why=executed);
  `!inEngine && ageMs < kMinActiveMs` -> WAIT through the held path; else RESEND.
- Test: vectors for each branch in `queue_hud_logic_test.cpp`. PLAY: hotbar medpac then provoke combat within 0.3 s; one
  stack decrement, one HANDOFF_ONE.

### RQ3 — Held-head detach can pop the head's own round node (PLAUSIBLE, high)
- Where: `QueueHud.cpp:1373-1376` (detachRound once when `s.nrn > 0`), `:1522-1536` (detaches all `s.nrn` nodes),
  `:1210-1217` (engineHoldsAction counts list nodes 9/10).
- Defect: assumes every pending round node at the edge is the player's new click. If the held head itself is a pending
  9/10 node (edge while paused / auto-pause), it is detached, the next frame sees the head gone -> RESEND, and the later
  reattach pushes the original node back: the head runs twice.
- Engine question: does the combat-start wipe leave round nodes (doc 29 D6 says ClearAllActions never empties the round
  list)? Either way the guard is correct.
- Fix: new pure `detachableMask(rn[], n, head)` that excludes nodes whose subType/spell/item id match `h.head`; detach only
  the rest; skip the detach when nothing remains.
- Test: mask vectors (head-only list -> 0 nodes; head + foreign -> foreign only). PLAY: auto-pause on, buffered medpac,
  combat starts paused: `ROUND_DETACH n=` counts only non-head nodes, one medpac used.

### RQ4 — Orphan round node wedges the queue (PLAUSIBLE, high)
- Where: `QueueHud.cpp:1785-1787` (any 9/10 round node sets `actionInQueue`); `QueueHudLogic.h:1476` (rule 1 PRESENT before
  any timer); `:885-893` (`nrn != 0` -> DS_NRN_BUSY); `QueueHud.cpp:1847` (verdict only when `s.round != 0`).
- Defect: ClearAllActions 0x541080 empties the queue but not the round list (doc 29 D6, PQ2 4.4). After a wipe (autosave,
  SaveMember, BumpFriends, StartConversation, script clear) an orphan 9/10 node with no 0x3F keeps the active action
  PRESENT with no age limit, and the same node blocks idle buffers via DS_NRN_BUSY. Only Cancel clears it.
- Fix: pure `orphanRoundAge(has3F0F2E, creal, nrn910, paused, since, now)`; when the creature has a 9/10 round node but no
  0x3F/0x0F/0x2E entry and no real cur for 1500 ms unpaused: log `ORPHAN m= n=`, remove the round list (same call Cancel
  uses, ServerClearAllActions is too broad for companions, see RQ12; use the round-list removal helper used by
  detachRound), then let the verdict run (eaten -> re-inject). Also an unpaused AV_PRESENT ceiling (30 s) ending in
  `DROP why=present_timeout` (new DropWhy).
- Test: pure vectors (entry present -> 0; paused -> frozen; 1499/1500 ms). PLAY: pause, queue item on leader, quicksave
  before unpause, unpause: `ORPHAN` within ~2 s and the next item dispatches without Cancel.

## Medium

### RQ5 — RI_OK re-inject bypasses all gates; refused serverCast is resent then silently lost (CONFIRMED, med)
- Where: `QueueHud.cpp:1820-1846` (RI_OK branch: no settle, debounce, spacing, pause gate); `QueueHudLogic.h:874-883`
  (`(void)paused; (void)now;`); `QueueHud.cpp:1067-1098` (serverCast returns bool), `:1095-1135` (dispatchAction is void).
- Defect: only members k>0 with a resting 0x3D reach RI_OK. A refused serverCast (`not_commandable`, `no_server_creature`,
  engine 0) leaves `buf.active` with nothing in the engine, so the next 2 frames re-send the same refused cast, then RI_MAX,
  then UNSEEN ~650 ms later with no DROP. pause_reinject=0 / pause_eaten_wait=1 do not govern this path (ini says they
  do). Transient refusals (companion stunned, scripted) permanently lose the power.
- Opus note: my own QL20 code introduced the serverCast part. A successful companion cast that vanishes without any
  executing evidence would also be re-sent by RI_OK (double cast) — not seen live (QH20 logs show VANISH:executed), but the
  settle/spacing gate closes it too.
- Fix: `dispatchAction` returns bool. On a refused server cast: transient reasons -> `buf.dispatchDeferred=true`, retry at
  250 ms up to 5 s, then `DROP why=refused` (new DR_REFUSED); permanent (`id_mask`) -> drop now. `reInjectWhy` honours
  paused (RI_PAUSED when `pauseEatenWait`), settle (now - activeStartTick > 500), spacing (250 ms) and `!pauseReinject`;
  log `REINJECT why=ri_ok paused=`. Update test at `queue_hud_logic_test.cpp:~1518`.
- Engine question: does 0x57D8B0 return nonzero on success (if not, `!r` is a false refusal)? QH20 live log shows ok=1 with
  the cast executing, so nonzero-on-success is supported by evidence.

### RQ6 — In-combat pass-through clicks stay tracked (PLAUSIBLE, med)
- Where: `QueueHud.cpp:2317-2321, 2416-2420` (DISPATCH_NOW in combat sets active and overwrites activeAction);
  `:1684-1688` (tick skipped in combat).
- Defect: the last in-combat click reaches the first non-combat tick as an active action of unknown history. Pause within
  ~150 ms of combat end -> eaten latch -> re-sent after unpause; or a member k>0 after a combat leader switch -> RI_OK
  re-cast. Overwrites leave no RESOLVE line.
- Fix: in the hook, when `g_combat != 0`, do not set `buf.active` (log `CMD ... dispatch_now combat=1 track=0`); log
  `OVERWRITE cmd=old->new` if an active is replaced. At the first non-combat tick, endAction any active whose
  activeStartTick predates combat end (`RESOLVE why=combat_end`).
- Test: pure `shouldTrackAfterCombat(activeStartTick, combatEndTick)`. PLAY: cast twice in combat, end combat, pause at once;
  no `re-injecting eaten` line.

### RQ7 — "executing" evidence not tied to our action (PLAUSIBLE, med)
- Where: `QueueHudLogic.h:1505-1518` (makeEngineView), `:776-778` (notePresent latches sawExecuting).
- Defect: any 0x2E/0x0F entry or aa8/ab8 left over from the previous cast latches sawExecuting during the first 650 ms; if
  the action is then wiped it resolves EXECUTED (lost, no re-inject). Reach: dispatch at the 2 s flag cap while ab8 is still
  set (ab8 lives to EndCombatRound, ~3 s).
- Fix: record aa8/ab8 at markDispatched; count a flag only if it rises after dispatch; count 0x0F/0x2E only when param0
  matches the active spellId/item. Keep "expanded but unfired" as eaten-eligible.
- Test: makeEngineView vectors (ab8=1 at dispatch unchanged -> false; 0->1 -> true; foreign spell id -> false).

### RQ8 — Unsent hand-off head lost at combat end (CONFIRMED, med)
- Where: `QueueHud.cpp:1333,1342` (head moved into `g_handoff[k].head`), `:1742-1743` (combat end -> endHandoff
  DR_COMBAT_CLEAR).
- Fix: on combat end, if `h.headValid && h.sent == 0`, restore it into the buffer front (`buf.pushFront` or
  `buf.active=true` with a `needsDispatch` flag) so the non-combat path sends it. Pure `restoreHead(h, buf)` with vectors.

### RQ9 — Detached nodes stranded at combat end (CONFIRMED, med)
- Where: `QueueHud.cpp:1686-1687` (detachTick only in combat), `:1742-1743`.
- Fix: before the non-combat return, while any `g_detached[k].n > 0`, run reattach (if the round list has room) or discard
  with a default-level log line `ROUND_DETACH act=combat_end_reattach|discard`.

### RQ10 — Detached nodes keyed by slot (PLAUSIBLE, med; merge of V1-5 and V2-9)
- Fix: store `owner` (server creature ptr) in `Detached`; in detachTick resolve the owner's current slot; discard with
  `why=identity_lost` if gone. Pure `detachTarget(ownerSlot, k)`.

### RQ11 — Any left click counts as Cancel (PLAUSIBLE, med)
- Where: `QueueHud.cpp:2055` (`g_lastClickTick` on every LMB edge), `:1380-1393`.
- Fix: record the click only when it hits queue-hud's Cancel rect or the vanilla QUEUE0 clear rect (`guikit::hit`), store
  x/y. Otherwise a vanished held head without exec evidence is RESEND. Pure `clickIsCancel(rects, x, y)`.

### RQ12 — Companion Cancel removes its FollowLeader 0x3D (PLAUSIBLE, med)
- Where: `QueueHud.cpp:941-967` (doCancel -> ServerClearAllActions on member k); doc 30:12 open risk; R7 FINDINGS:33
  (only 0x669FC0 script ActionFollowLeader and 0x7E5730 AddCharacter add 0x3D).
- Fix (recommended): for k >= 1 remove only non-0x3D entries plus the round list (walk the queue with the same per-node
  remove used by the kick: ActListRemove on non-0x3D nodes) instead of ServerClearAllActions; log
  `CANCEL m= n= drawn= had3D= after3D=`. Fallback: re-add FollowLeader via the 0x669FC0 route.
- PLAY: queue 2 items on a companion, X its strip, unpause: companion keeps following; `after3D=1`.

### RQ13 — selfheal interplay (CONFIRMED + PLAUSIBLE, med)
- V2-4: `_shared_src/ActQueue.h:52-56` `useQueued` is item-only; add `actq::castQueued(s)` (any 0x3F with a 9/10 node) and
  refuse in selfheal `useAllowed`. Test in the selfheal/ActQueue test.
- V2-8: after a queue-hud Cancel, selfheal keeps `pending` for 6 s then `TIMEOUT unqueued`. Fix in selfheal: end the pending
  at once when its use vanished from an emptied queue (`DONE reason=wiped`), no auto re-fire inside the cooldown.

### RH1 — Save with no snapshot keeps the previous save's bar (CONFIRMED, med)
- Where: `action_hotbar_src/ActionHotbar.cpp:1865-1869` (SS_Absent returns early); README:86, CHANGELOG:50 promise empty;
  `hotbar_logic_test.cpp:752-754` pins the keep.
- Decision (Opus default, owner may override): follow the documented contract: SS_Absent -> empty profiles (keep
  [General]); UNREADABLE and "folder not recognised" keep the live bar. Pure `snapAction(src, readable)`.

### RH2 — 4:3 windows above 800x600 get identity scale (CONFIRMED, med; latent for this user at 3840x1600)
- Where: `HotbarLogic.h:285-289` (`if (c != 1.0)` guards sx/sy); test `hotbar_logic_test.cpp:419` pins the bug.
- Fix: always set sx = w/rw, sy = h/rh when all sizes > 0; c stays 1.0 for 4:3. Tests: 1024x768 -> sx=sy=1.28,
  unitRect(4,478,20,35) == {5,611,25,44}; 1600x1200 -> {8,956,40,70}; 800x600 unchanged.

### RH3 — Failed config Save leaves g_cfg mutated (CONFIRMED, med)
- Where: `ActionHotbar.cpp:1296-1308`. Fix: copy the profile before moveDuplicates/SetSlot, restore on `!ok`.

### RH4 — @lowhp target frozen at press (PLAUSIBLE, med)
- Where: `ActionHotbar.cpp:643-655`, queue-hud replay `QueueHud.cpp:1128-1134`.
- Fix (minimal): action-hotbar counts its own pending @lowhp heals per member (cleared when queue-hud resolves or after
  5 s) and subtracts an estimated heal from that member's missing HP in `chooseTarget`. Pure vector with `pendingHeal[]`.

### RH5 — No conversation gate for digits (PLAUSIBLE, med)
- Action: PLAY row first (bind 1-3, press in a voiced dialogue reply menu); add a DEBUG gate line naming the top panel
  vtable. Code fix only if `SWALLOW` appears in dialogue. Do NOT apply "swallow whenever g_hudGateOk" (V4-13).

## Low (RL1-RL31; full text in the named V file)
Queue-hud correctness/bookkeeping:
- RL1 V1-6 eatenReInjects/eatenUnpausedMs/lastReInjectTick survive hand-off and overwrite exits -> reset in markDispatched.
- RL2 V1-7 + V2-13 identity swap leaves g_flagGate/g_kickLast/g_lastWaitTick with the slot; skip[] stale on 3-way rotation.
- RL3 V1-8 pause time counts toward FlagGate cap and settle timers -> unpaused-time accrual.
- RL4 V1-9 no reset on load/area; no dialogue gate for dispatch -> `partyGenerationChanged` drop-all; WAIT why=dialogue.
- RL5 V1-11 verdict re-injects have no lifetime total cap -> cap ~12.
- RL6 V2-10 in-combat clicks during a hand-off jump ahead of buffered commands -> grow RoundOrder foreign set.
- RL7 V2-11 foreign node keeps AV_PRESENT (BG23) -> match node id to activeAction.
- RL8 V2-12 kickStarved has no give-up -> 5 s `SRV_KICK_GIVEUP` + DROP; scan for first 0x3F up to queue cap.
- RL9 V2-15 no dispatch-time revalidation (Down member, dead target, used-up item) -> hold serverCast while memberDown.
- RL10 V3-4 unreadable engine queue at click -> treat as BUFFER or log why=unreadable.
- RL11 V3-10 `handoff_clear_first` ANDed into keepNewAtHandoff (clear_first=0 silently disables RoundOrder arming).
Queue-hud UX/logging:
- RL12 V1-10 client-route refusals invisible (merge into RQ5's bool return; log REFUSED).
- RL13 V1-12 combat=-1 logged as `why=ended`/`edge` -> ignore -1 in modeChanged/modeWhy.
- RL14 V1-13 QJ signature compares only 8 entry types.
- RL15 V1-15 + V3-14 parseIni drops rejected lines silently, no BOM skip, inline `;` comments rejected -> count + log once.
- RL16 V3-5 5th click dropped (drop_full) with no cue -> denied sound/flash.
- RL17 V3-6 hold repaint keeps cancelled icons 400 ms -> reset held state in doCancel.
- RL18 V3-7 Cancel hidden for lack of room logged as `empty` -> `no-room` reason.
- RL19 V3-8 stale Cancel stays clickable when HUD_DRAW stops -> cwHideAll on stale capture.
- RL20 V3-9 >4 slots invisible but cleared by Cancel; stale cur takes the active slot; Down companion strip hidden.
- RL21 V3-11 FlagGate cap 2 s vs ab8 ~3 s; comment says 0.7 s -> measure and fix comment.
- RL22 V3-12 detached icons draw in hard combat (document as intended).
Action-hotbar:
- RL23 V4-6 Invoke=0 still swallows bound digits -> pass(invoke off).
- RL24 V4-7 direct @self/@lowhp item routes skip entry bit0; refusal silent -> require EntryUsable, show message.
- RL25 V4-8 failed INI reload still lets the next Save overwrite; readFile ignores ferror.
- RL26 V4-9 single `.load.bak` -> rotate 3.
- RL27 V4-10 stale tooltip after scroll/mode change -> hideTip.
- RL28 V4-11 picker snapshot rebuilt every frame -> throttle.
- RL29 V4-12 modifier resync AND-only; downSwallowed not cleared on pass.
- RL30 V4-13 comments/WARN claim bound digits always swallowed (stale tick passes them) -> reword only.
- RL31 V4-14 + V4-15 wheel subscribe retries every frame unlogged; stance WARN once per process.
Docs (one batch): V1-14, V2-14, V3-13 — ini comments (16-24 px vs 48-72 px, defaults that differ from code), QueueHud.cpp:1
version, hook H8 text, doc 30 missing QL10-QL20 + key table, doc 29 status, `UseItemRouteA` naming (now shared by
queue-hud and selfheal).

## Open engine questions (play-log or REA, none block the fixes)
1. Do orphan 9/10 round nodes ever get dropped without a 0x3F (decides RQ4 wedge vs long stall)?
2. Does the follower AI re-add 0x3D after a full clear on a companion (RQ12)?
3. Can the leader be switched mid-combat while a hand-off head is held (RQ10 reach)?
4. Is the dialogue panel in the GUI panel list without ignore bits while the HUD draws (RH5)?
5. Real ab8 lifetime after an out-of-combat cast (RQ7, RL21).

---

## Fix plan (phases)
Executor specs per phase: `.claude/reviews/rv_qac_20261023/plan_<Id>.md` (exact code, tests, commands). One commit per phase
(explicit paths), queue-hud version bumps once at RV1 (0.4.6-dev) and patch bumps per later phase; install needs user GO
(game closed) and runs once after RV1-RV6 (or earlier if the user asks).

| Id | Batch | Deps | Owner gate | Status | Commit | Next |
|---|---|---|---|---|---|---|
| RV1 | queue-hud high: RQ1 spell replay pos/flag, RQ5 RI_OK gates + serverCast bool/deferred retry (+RL12), RQ2 edge head guard, RQ3 detach excludes head | - | - | TODO | | spec plan_RV1.md; unit tests queue_hud_logic_test |
| RV2 | queue-hud verdict: RQ4 orphan round node + present ceiling, RQ6 combat-end resolve (hook half dead), RQ7 executing evidence tied to our action, RL1 counter resets, RL2 swap bookkeeping, RL3 unpaused FlagGate, RL5 re-inject total cap, RL8 kick give-up | RV1 | - | TODO | | spec plan_RV2.md |
| RV3 | queue-hud hand-off/Cancel: RQ8 restore unsent head, RQ9 detach at combat end, RQ10 detached owner, RQ11 spatial cancel click, RQ12 companion Cancel measure-first (CANCEL had3D / CANCEL_AFTER after3D), RL6, RL11, RL17-RL19 | RV2 | - | TODO | | spec plan_RV3.md |
| RV4 | cross-mod: RQ13 actq::castQueued + selfheal dedupe, selfheal wiped-pending end | RV1 | - | TODO | | spec plan_RV4.md; compat_check ActQueue.h |
| RV5 | action-hotbar: RH1 persave absent -> empty, RH2 4:3 scale, RH3 assign rollback, RH4 @lowhp pending heals, RH5 gate DEBUG line + play row, RL23-RL31 | - | - | TODO | | spec plan_RV5.md |
| RV6 | low/logging/docs: RL4, RL7, RL9, RL10, RL13-RL16, RL20-RL22, docs V1-14/V2-14/V3-13 | RV3 | - | TODO | | spec plan_RV6.md |
| RV7 | install (hot-swap queue-hud, action-hotbar, companion-selfheal) + PLAY_TEST_QUEUE rows | RV1-RV6 | user GO | TODO | | gate PASS then kpm_hotswap --apply |

## Plan-stage decisions (after spec + spec review, 10-23)
Specs `plan_RV1..RV6.md` were reviewed by 3 Sonnet skeptics (`review_RV12.md`, `review_RV36.md`, `review_RV45.md`: 6 BLOCKING,
19 MINOR) and the corrections folded into the specs (each ends with "Review corrections applied"). Decisions that change the
issue text above:
- RQ6: premise wrong. Both hooks return `pass_combat` when `g_combat != 0`, so in-combat clicks are never tracked; only the
  combat-end resolve for B1-held buffers is kept (RV2).
- RQ12: MEASURE-FIRST. Cancel keeps ServerClearAllActions (no per-node remove exists; pop/re-add of live entries skips the
  engine's ClearAction/RemoveAllActions bookkeeping). New `CANCEL ... had3D=` + `CANCEL_AFTER ... after3D=` lines and a PLAY row
  decide whether a FollowLeader re-add is needed.
- RQ5: `dispatchAction` returns `SendResult {Sent, Retry, Refused}`; client-route refusals stay invisible (Perform* return void).
- RQ3: ActListRemoveHead only pops from the front, so detachRound pops all and pushes the head's own node back.
- RQ8/RQ9: restored heads and combat-end reattach are guarded against a recent Cancel (vanilla Cancel Combat included).
- RQ13: selfheal refuses only when a 0x3F sits behind 0x3D at index 1..3 with a pending 9/10 round node (never `cur`).
- RH4: no heal estimate; a member with a live pending @lowhp heal is ineligible.
- Fact fix: app-level ServerClearAllActions 0x51CB10 DOES empty the round list; only object-level ClearAllActions 0x541080
  does not (doc 29 D6 vs doc 30 wording). RQ4 orphan reach is therefore the 0x541080 callers (autosave, SaveMember, script).
