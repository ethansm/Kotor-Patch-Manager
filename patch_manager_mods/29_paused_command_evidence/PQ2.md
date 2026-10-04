# PQ2: why a queued cast / use-item may not run promptly once the world is running (KOTOR II Steam swkotor2.exe 6a522e71)

Static only (ROOT dump + exe bytes for constants). Batch PQ2 of the paused-command bug; the pause gate and the client->server path belong to the other analyst. Constants decoded from the exe: `0x9944B4`=1, `0x9944B8`=2, `0x9944BC`=3, `0x9944C0`=4, `0x9944C4`=1000 (return codes 1 in progress / 2,3 terminal / 4 special; RunActions budget).

## Summary (ranked hypotheses for the delay)
1. H1 (structure CONFIRMED, fit to the user's case MEDIUM): `CSWSCreature::ActionManager 0x563C90` (the only queue clear on the player-command path) is a NO-OP unless the actor is a connected player's current game object; for any other actor the cast/use is queued as round action + one `0x3F` appended with AddAction (tail), and a follower's persistent FollowLeader `0x3D` (returns only 1 or 3, never 2/4) stays at the head and starves it until something else clears the queue.
2. H2 (CONFIRMED, HIGH): same-round lockout: `AIActionCombat 0x6D9500` returns 1 (holds the `0x3F` at head, AddHead) while `round+0xAB8 >= 1`; AB8 is set by the first cast/use and cleared only by EndCombatRound. Round = 3000 ms of unpaused time (cast: pause 1500 + remaining 1500; item default: pause 1331 + remaining 1669), so a 2nd command inside that window waits up to ~3 s from the first cast start.
3. H3 (CONFIRMED mechanism, MEDIUM reach): `CSWSObject::ClearAllActions` never touches the round action list (+0xB00); a command whose `0x3F` was wiped (BumpFriends, script clear, a later move/command) leaves an orphan round action that the NEXT `0x3F` runs first (new command then waits for H2); and during the 8000 ms combat-exit window (`+0x520==1`) the handler skips ActionManager and the Add*Actions clears entirely.
4. Handler timings are NOT the cause (REFUTED as multi-second): spells fire at conj 170 ms and finish at 1500 ms, item default fires at 1 ms and finishes 1331 ms, facing is an instant orientation set, AIUpdate for party (level 4) runs RunActions every tick; the engine-only latency for an unblocked controlled-leader command is about 1-4 ticks plus 170 ms.
5. No "queued" indicator exists: the server action-queue mirror `UpdateActionQueue 0x588ED0` (>= 300 ms throttle, `0x569EFB`) lists only real queue nodes (0xF, 0x2E, 1, 0xC ...), never the `0x3F` placeholder or the round list, so nothing shows until the `0x3F` expands; probes and hook catalogue below (checked windows, `RA-H09` already hooked by companion-ai).

## (a) Function table
| addr | proposed name | conf | purpose | evidence (quoted line) |
|---|---|---|---|---|
| 0x5D56B0 | CSWSObject::RunActions | high (AI4) | pop/dispatch; result 4 -> AddTail, 1 -> AddHead; stamp `+0xC0/+0xC4` | `005d6a63 CMP EDX,[0x009944c0]` / `005d6a7b CALL 0x0054aa30`; `005d6a88 CMP ECX,[0x009944b4]` / `005d6aa0 CALL 0x0054aa10` CONFIRMED |
| 0x5BF7F0 | AIActionFollowLeader (0x3D) | high | returns 1 or 3 only | `005bf881 MOV EAX,[0x009944bc]`, `005bf8c5/005bf92f MOV EAX,[0x009944b4]`, `005bf925 MOV [EAX+0x11b0],0x3` CONFIRMED |
| 0x5BCDC0 | AIActionCheckMoveToFollowRadius (0x40) | high | returns 2 or 3 only | `005bce36/005bcf08 MOV EAX,[0x009944bc]`, `005bd0a5 MOV EAX,[0x009944b8]` CONFIRMED |
| 0x5B96F0 | CSWSCreature::AIActionCastSpell (0xF) | high | cast timeline conj/cast/catch | `005b97f3 MOV [ECX+0x570],0xbb8`; `005ba788 CMP EAX,[EBP-0x80]` / `005ba78b JNC` CONFIRMED |
| 0x5B8000 | CSWSCreature::AIActionItemCastSpell (0x2E) | high | item use timeline (node-local stamp) | `005b82a4 MOV [EDX+0x570],0xbb8`; `005b8f86 CMP ECX,[EBP-0x74]`; `005b9607 CMP ECX,[EBP-0x7c]` CONFIRMED |
| 0x6D9500 | CSWSCreature::AIActionCombat (0x3F) | high (AI2) | pops ONE round action, expands it into real actions | `006d9554 MOV EAX,[EDX+0xaa8]` / `006d9567 JNZ 0x6da49a`; `006d96cc CMP [EBP-0xfc],1` / `006d96d3 JGE 0x6da425` CONFIRMED |
| 0x5D0C00 | type 0x10 handler (round-sync wait) | med | 2 if no round or spell action pending, else 1 | C: `if (A84 != 1 \|\| (GetSpellActionPending, uVar4 = 1, ret != 0)) uVar4 = 2`; creator not found (open) |
| 0x590180 | CSWSCombatRound::GetSpellActionPending | high (DB) | any round action type 2 with time <= timer | C: `*local_c <= [round+0xa94] && (char)local_c[4] == 2` CONFIRMED |
| 0x590110 / 0x590220 | CSWSCombatRound::GetActionPending / GetAction | high (DB) | round list non-empty and head time <= timer (`+0xA94`) | C: `*piVar2 <= *(param_1 + 0xa94)` CONFIRMED |
| 0x590270 | CSWSCombatRound::AddAction | high (DB) | insert sorted by time (field +0), cap 4 entries | C: `if (uVar1 < 4)`; `*param_2 < *piVar2` CONFIRMED |
| 0x591890 / 0x591670 | AddSWItemSpellAction / AddSWSpellAction (round node type 0xA / 9) | high (DB) | queue-time round node; node time = 0 (ctor `0058c810`) | C: `*(local_30+0x10) = 10` / `= 9`; `+8 = 500` (spell) CONFIRMED |
| 0x565310 | CSWSCreature::AddCastSpellActions | high | `param_19==0`: round node + `0x3F`; `!=0`: real chain | `005653c5 CALL 0x590110` / `005653cc JNZ 0x5653db` / `005653ce PUSH 4` / `005653d6 CALL 0x563c90`; `00565436 CALL 0x53f7f0` (PUSH 0x3f) CONFIRMED |
| 0x5648D0 | CSWSCreature::AddItemCastSpellActions | high | same split, opposite clear polarity | `0056497c CALL 0x590110` / `00564983 JZ 0x564992` / `00564985 PUSH 4` / `0056498d CALL 0x563c90`; `005649ed CALL 0x53f7f0` CONFIRMED |
| 0x57D8B0 / 0x57DA90 | AddUseTalentOnObjectActions / AtLocationActions | high (DB) | on-object: NO ClearAllActions; at-location+item: explicit clear | `0057db0c`, `0057db7b CALL 0x541080` (AtLocation only) CONFIRMED |
| 0x57B4C0 | CSWSCreature::UseItem | high (DB) | calls `0x5648D0(..., front=local_a0, 0)` | `0057b701 CALL 0x5648d0`, last arg 0 CONFIRMED |
| 0x563C90 | CSWSCreature::ActionManager(mode) | high (DB) | player-object gate, recompute action class, maybe ClearAllActions(1) | `00563cae CALL 0x51c600` / `00563cb6 CMP [EBP-4],0` / `00563cbc JMP 0x563d10`; `00563cf5 CALL 0x541080` CONFIRMED |
| 0x51C600 / 0x52F330 | GetClientObjectByObjectId | high (DB) | non-null only if a player's `+0x28` game object id == arg | `0052f382 CALL 0x632a20` ... `0052f3a2 CMP EDX,[EBP+8]` CONFIRMED |
| 0x632A20 | CSWSPlayer::GetGameObject | med | `0x51C0A0([this+0x28])`, type byte > `[0x994473]` | C: `FUN_0051c0a0(*(param_1 + 0x28))` |
| 0x5718A0 / 0x5719A0 / 0x571980 | ComputeActionClass / per-type classifier / `class < 10` | med | `+0x1116` byte: 0 none, 1 move, 2 attack, 0xA..0x21 others; `<10` means interruptible | C of `0x571980`: `return param_1 < 10`; `0x5719A0`: type 0xF/0x2E -> 0xE; 0x3D/0x3F/0x40 not mapped |
| 0x541080 | CSWSObject::ClearAllActions | high (AI4) | gate `vt+0x10`, `+0xEC == 1`; deletes queue, `+0x11B0=1` | `005410e9 CMP [EAX+0xec],0x1` / `005410f0 JNZ 0x541385`; `00541210 MOV [ECX+0x11b0],1` CONFIRMED |
| 0x5908F0 / 0x590430 | round: clear attack slots / RemoveAllActions (list) | high (DB) | ClearAllActions only calls the former | C `0x5908F0`: loop 5 attack slots; callers of `0x590430` listed in 4.4 CONFIRMED |
| 0x586B70 | CSWSCreature::BumpFriends | high | ClearAllActions(1) of the bumped friend unless its queue holds type 1 or 0x11 | `00586f13 CMP [EBP-0x78],1` / `00586f19 CMP ...,0x11` / `00586f48 CALL 0x541080` CONFIRMED |
| 0x66F330 | Global::ExecuteCommandClearAllActions | high (DB) | script clear of any object | `0066f38d CALL 0x00541080` CONFIRMED |
| 0x58E730 / 0x58E6D0 / 0x58E990 | StartCombatRound / SetSpellCastRound / EndCombatRound | high | round arm (`+0xA9C=3000`), spell flag (`+0xA88=1`, creature `+0x584=4`), end (`+0xAB8=0`) | `0058e8da MOV [EAX+0xa9c],0xbb8`; `0058e6dc MOV [EAX+0xa88],1`; `0058ea53 MOV [ECX+0xab8],0` CONFIRMED |
| 0x58FC60 / 0x58FCD0 / 0x58FD50 | SetRoundPaused / SetPauseTimer / DecrementRoundLength | high | cast pause and budget shrink | callers `005ba2ac/005ba2d5/005ba2fe` (cast), `005b8e66/005b8e89/005b8eac` (item) |
| 0x571030 | UpdateCombatRoundTimer | high (AI2) | returns while pause bit 2 set; `IncrementTimer(dt)` when `+0xAA8==0` | C: `if (FUN_0051c720(2) != 0) return;` CONFIRMED |
| 0x51C720 / 0x538CD0 | CServerExoApp::GetPauseState(mask) | high (DB) | `[internal+0x10078] & mask`; UI pause toggles mask 2 | `0065df6d PUSH 2` / `0065df75 CALL 0x51c6b0` (TogglePauseState) |
| 0x588ED0 | CSWSCreature::UpdateActionQueue | high | builds the 10-entry queue mirror at `+0x3E8 + i*0x1C` | AIUpdate gate `00569efb CMP [EBP-0x78],0x12c` / `00569f0a CALL 0x588ed0` CONFIRMED |
| 0x7E5120 | party slot element SetState (`E+0x84`) | low | called at cast/use start for party (`0x1B2C7`) and far-queue (`0xA5ED`) | `005b97e8`, `005b8299`, `00565eab`, `0056528c` |
| 0x5BB2C0 / 0x57D370 | AIActionChangeFacingObject / SetLockOrientationToObject | med | instant orientation set, returns 2 | C of `0x5BB2C0`: returns `DAT_009944b8` after `FUN_00543f10`; no turn loop |

## (b) Findings
### Q1 Queue position (FollowLeader / CheckMoveToFollowRadius / MoveToPoint / Wait)
- 1.1 CONFIRMED typical resting queue of a non-controlled follower is `[0x3D]`; while moving the follow driver inserts at the FRONT: `[MoveToPoint(1), 0x40, 0x3D]` (`005c786f CALL 0x00540ca0` = AddActionToFront with `PUSH 0x40`, move via `0x5644B0` = AddMoveToPointToFront at `005c78d8`, group id read from the `0x3D` node `005c77ce PUSH 0x3d` / `005c77d6 CALL 0x5d5120`). Wait (0x1E) only comes from scripts.
- 1.2 CONFIRMED the `0x3D` handler never returns 2 or 4. Returns: `005bf881 / 005bf8b2 / 005bf91b MOV EAX,[0x009944bc]` (=3: flag/FollowInfo/dead/legs-bit gates, leader==self), `005bf8c5 MOV EAX,[0x009944b4]` (=1, area null `0x5453C0`), `005bf925 MOV dword ptr [EAX+0x11b0],0x3` then `005bf92f MOV EAX,[0x009944b4]` (=1, normal). RunActions: `005d6a88 CMP ECX,[0x009944b4]` -> `005d6aa0 CALL 0x0054aa10` (AddHead) and the loop exits on result 1 (`LAB_005d6d5f`, `local_18 = 1`). So an idle follower's `0x3D` stays at the head forever and ends every RunActions call.
- 1.3 CONFIRMED `0x40` ends: 3 at `005bce36`/`005bcf08` (dead, legs, area mismatch), else 2 at `005bd0a5 MOV EAX,[0x009944b8]` (AI5: dist^2 > (r+0.01)^2 sets slot state 5 `005bd09e`, still returns 2). It never requeues.
- 1.4 CONFIRMED MoveToPoint (type 1) is a stop-loop type (`0x5D7160`): its completion ends the tick's loop (AI4 key 10).
- 1.5 CONFIRMED (code derivation) a cast/use appended to the TAIL behind a `0x3D` never runs: pop order is head-first, `0x3D` returns 1 -> AddHead -> loop exit. Release only when the `0x3D` node disappears: `0x3D` returns 3 (leader id == self `005bf91b`, i.e. the actor becomes the controlled leader; flag/FollowInfo/HP/legs gates), or ClearAllActions / BumpFriends / script clear / combat entry (4.1). Reach SPECULATIVE: it applies only when the command actor is NOT the player's game object (1.6).
- 1.6 CONFIRMED the message handler acts on the sender player's creature: `0065beb6 CALL 0x00632a20` then `local_14 = (**(code **)(*local_1c + 0x30))()` (C line 280); the three ActionManager calls: talent `0065d89b` (always), cast `0065d546..0065d560` (only `+0x520 != 1`), use-item `0065c5b5..0065c5cf` (only `+0x520 == 0`). `ActionManager` does nothing unless `0x51C600(self id)` finds a player whose game object is that id (`00563cb6 CMP [EBP-4],0`; `0052f3a2 CMP EDX,[EBP+8]`). So: controlled leader -> queue cleared (when `+0x1116 < 10`, `00563ceb CMP EAX,1` / `00563cf5 CALL 0x541080`); any other creature (a follower commanded through the mod, a script) -> no clear, and `ClearAllActions` itself is a no-op if `+0xEC != 1` (`005410e9`).
- 1.7 CONFIRMED the clear depends on `+0x1116`: `0x5719A0` maps types 7,8,9,0xB,0xF/0x2E,0x14,0x15,0x19..0x1D,0x22,0x23,0x26..0x2A,0x36,0x38,0x43 to classes >= 10 and keeps them; a command issued while a previous cast/use/equip is still in the queue is NOT cleared and is appended behind it (then waits for H2).

### Q2 Cast / item handler timing (what gates the fire)
| gate | site | value | verdict |
|---|---|---|---|
| `0x3F` dispatcher paused | `006d9554 MOV EAX,[EDX+0xaa8]`, `006d9567 JNZ 0x6da49a` | `round+0xAA8` set by SetRoundPaused during a cast; returns 1 | CONFIRMED |
| player pause bit | `006d957c PUSH 2`, `006d9584 CALL 0x51c720`, `006d958b JNZ 0x6da49a` | `[internal+0x10078] & 2`; returns 1 (re-queued each tick, nothing lost) | CONFIRMED (pause semantics: other analyst) |
| nothing pending or action already started this round | `006d96a2 JZ 0x6da425`; `006d96cc CMP [EBP-0xfc],1` / `006d96d3 JGE 0x6da425`; tail `006da48a CMP [EBP+0xfffffe58],0` / `006da491 JNZ 0x6da49a` (1) else `006da493 MOV EAX,[0x009944b8]` (2) | AB8 `>= 1`: return 1 if `+0xA84` (round started) else 2 (node dropped) | CONFIRMED |
| conj wait (spell) | `005ba788 CMP EAX,[EBP-0x80]` / `005ba78b JNC 0x5ba8b9` ([EBP-0x80]=spell+0x4C ConjTime, `005ba17a`) | elapsed < conj: anim, return 1 (`005ba88b`) | CONFIRMED |
| fire (spell) | `005bb037 MOV [EDX+0x1f0],1`; AB8++ `005bb076`; `005bb0a8 CALL 0x5482a0` | at elapsed >= conj (170 ms typical) once | CONFIRMED |
| done (spell) | `005bb167 CMP [EBP-0x64],EDX` (EDX = conj+cast+catch) / `005bb16a JNC`; return 2 `005bb24b` | 170+1330+0 = 1500 ms typical | CONFIRMED |
| start delay (item default) | `005b8d0a MOV [EBP-0x74],1`; `005b8f86 CMP ECX,[EBP-0x74]` / `005b8f89 JNC` | 1 ms (cases 6/0x31: 700/800/950; 0xC: 300; 0x14: 600; 0x19,0x1A,0x2D,0x2F: 750) | CONFIRMED values, case-for-medpac SPECULATIVE |
| done (item) | `005b8d29 MOV [EBP-0x7c],EAX` (= 1 + spell+0xB4 CastTime); `005b9607 CMP ECX,[EBP-0x7c]` / `005b960a JNC` | default 1331 ms (cases above use 1500 / 1000) | CONFIRMED |
| round arm | spell `005ba252 CALL 0x58e730`, `005ba2d5 CALL 0x58fcd0`, `005ba2fe CALL 0x58fd50`; item `005b8e10`, `005b8e89`, `005b8eac` | pause = conj+cast+catch (1500) resp. item duration; budget shrinks by the same | CONFIRMED |
| reject (return 3) paths | spell, `MOV ECX,[EBP+0xFFFFFD48]` before `CALL 0x589BF0`: `0x5B986B` (dead/hp), `0x5B9B41` (spell row null), `0x5B9BAB` (`spell+0x198 & mask`), `0x5B9C1E` (`+0x19C` req mask), `0x5B9C8B` (`0x6B8440` feat uses), `0x5B9D44` (target gone), `0x5B9E2E` (target dead), `0x5BA945` (area null), `0x5BA9DF` (`0x5818A0` ready count) | immediate, each runs `0x589BF0` + `0x589C30` first | CONFIRMED sites, gate names from callee DB names |
- Constants (spells.2da, replay snapshot `replay/pre_checkpoint_2da/spells.2da`, 286 rows; live override identical per TARGETABLE.md histogram): conjtime/casttime/catchtime = 170/1330/0 for 200 rows incl. Force powers (rows 8,10,17,24,28,134,173-175) and medpacs (64-66); 560/1940/500 for 2 rows. Spell record offsets CONFIRMED from the loader `0x6C1490`: `+0x4C` ConjTime (`006c281d MOV [ECX+0x4c],EDX`), `+0xB4` CastTime (`006c2b7e`), `+0xF8` CatchTime (`006c3027`).
- 2.1 REFUTED waits on: facing turn (`0x5BB2C0` returns 2 in one call after an instant `0x543F10`; cast uses `0x57D370` lock only), combat round boundary (the first command starts its own round: `StartCombatRound` is called by the handler itself when `+0xA84 == 0`), "already casting" flag (none; `+0x1F0` is a fired-once flag), animation length (anim ids are cosmetic; timing comes from the 2DA ms values).
- 2.2 CONFIRMED the move-to-range sub-actions come BEFORE the cast: out of range `0x5644B0`/`0x564770` (MoveToPoint, stop-loop type) then `0x11`/`0x12`; each completed node costs at most one tick (RunActions exits after type 1 and when `>1000 us`).
- 2.3 CONFIRMED stamp semantics: `0x5B96F0` measures elapsed from the CREATURE stamp `[+0xC0/+0xC4]` (`005ba6bf MOV EAX,[EDX+0xc4]`, `005ba6cc MOV EDX,[ECX+0xc0]`; init block only when elapsed == 0 `005ba6ed CMP [EBP-0x64],0`), set at pop only if both are 0 (`005d5b94 CMP [ECX+0xc0],0`, `005d5ba3 CMP [EDX+0xc4],0`, `005d5bb5 MOV [EAX+0xc0],ECX`) and cleared only when a node completes (result 2/3). `0x5B8000` uses a node-local stamp (`node+0x54/+0x58`, `node+0x6E`). A `0x3F` that waited (H2) leaves a stale creature stamp; a cast with no completing node before it then skips the conj phase (fires at once). SPECULATIVE impact (needs a chain without 0x13/0x31/0x12 nodes).
- 2.4 type 0x10 (`0x5D0C00`): 2 when the round is not started or a spell round action is pending, otherwise 1. Who queues it: not found (open).

### Q3 Combat round out of combat
- 3.1 CONFIRMED a cast/use out of combat STARTS a round: spell `005ba252 CALL 0x58e730` (guard `005ba1b6 MOV ECX,[EAX+0xa84]` / `005ba1c9 JNZ 0x5ba46b`), item `005b8e10`. First action is not deferred to a boundary; it runs in the next AIUpdate (no `+0xAB8` yet).
- 3.2 CONFIRMED fields: `round+0xA84` started, `+0xA88` spell round flag, `+0xA94` timer ms, `+0xA9C` remaining length (3000 at start `0058e8da`), `+0xAA8/+0xAB0/+0xAB4` pause flag/timer/infinite, `+0xAB8` actions started (reset `0058ea53`, ctor `0058e359`). Timer advance uses creature `+0xD0` dt (`0056956c MOV [ECX+0xd0],EDX`) and stops while pause bit 2 is set (`0x571030`).
- 3.3 CONFIRMED wait for a SECOND command: AB8 stays >= 1 from the first fire (`005bb076`, item `005b9447`) until `EndCombatRound`; round ends at timer >= `+0xA9C` after the cast pause. Spell: pause 1500 then remaining 1500 -> ~3000 ms total; item default: 1331 + 1669 -> ~3000 ms; `+0xA9C` after `DecrementRoundLength` per `0058fd50` (`+0xA9C -= ms`). No 6000 ms constant in `0x58E730/0x58E6D0/0x58E990/0x58F7A0/0x58FD50` (grep).
- 3.4 CONFIRMED worst-case wait for a 2nd command = round remaining, <= 3000 ms (unpaused time); SPECULATIVE that the round ends exactly there when `+0xA88 != 0` (`0x58F7A0` end branch checks `GetSpellActionPending`/queue head type 0xF).
- 3.5 CONFIRMED lingering combat state: `SetCombatState(1)` arms `+0x524 = 8000`; while `+0x520 == 1` the handler skips `ActionManager` (cast `0065d552 CMP ...,1`, item `0065c5c1 CMP ...,0`) and `0x565310/0x5648D0` skip their clears (`+0x520==0` guard), so up to 8 s after the last fight a command is queued behind whatever the creature is doing.

### Q4 ClearAllActions callers, AIUpdate scheduling
- 4.1 Callers of `0x541080` (cg.py, 58 sites): player input `0x65BE70/0x656AF0/0x65B270/0x65B490/0x65E670`; Add*Actions (`AddAttackActions 0x56858C/0x5687BA`, `AddUseTalentAtLocation 0x57DB0C/0x57DB7B`, `AddStance 0x566547`, `AddDoorSaber 0x547CB4`); `CancelCombat 0x58675E`; `BumpFriends 0x586F48`; effects (`OnApplyDamage 0x59770C`, Death, Entangled, SetStateInternal, Disappear/Appear, Faction mod, Resurrection); `AIActionDialogObject`, `StartConversation`, `BeginConversation`, `AIActionType47`; party table (`SaveMember/SavePuppet/SpawnNPC/SpawnPUP 0x5F7FA2...0x5FD870`); party-select Accept `0x8A04E3`; script cmd `0x66F38D`; `UnsummonMyself`. NONE on unpause (`TogglePauseState 0x538C20/0x51C6B0` callers: no ClearAllActions), none in heartbeat/AIUpdate, none in `SetLeaderBySlot`/leader switch (CONFIRMED by caller list). Script-level clears from companion AI scripts (k_hen_*, k_ai_master) are not visible statically (SPECULATIVE; hook `0x66F388`).
- 4.2 CONFIRMED wipes that can drop a queued command: BumpFriends (queue lacking types 1/0x11: `[0x3D,0x3F]` or `[0x3F]` qualify; companion-ai FM1 hook `0x586F43` already logs `queue=[...]`), `AddAttackActions` (enemy appears: party scripts attack), SaveMember on autosave, effects, script clear, a later player move/command (ActionManager). The round list survives all of them (4.4).
- 4.3 CONFIRMED reorder: new round node is inserted by time (`0x590270`, equal time -> after existing) and `AIActionCombat` takes `0x590220` = head with time <= timer, so an orphan (4.4) runs before a newer command.
- 4.4 CONFIRMED `0x541080` calls only `0x5908F0` (5 attack-data slots, `0058cd80`), never `0x590430` (RemoveAllActions of the round list). Callers of `0x590430`: `CServerExoApp::ClearAllActions 0x51CB47` (client side `CSWCObject::ClearAllActions 0x7F24ED`), `0x51CBCD`, `AddAttackActions 0x5685A3`, `SetCombatState 0x57A737` (state 0 only), `OnApplySetStateInternal 0x59DEF0`, `AIActionType47 0x5DF9D0`, `0x65E8E8/0x65F283`. Orphan lifetime: until one of those or until a `0x3F` consumes it. SPECULATIVE whether a single player pause/clear sequence makes this common.
- 4.5 AIUpdate: CONFIRMED no throttle for party: `005695b1 CALL 0x005d56b0` every AIUpdate (QPC stamp `00569440`, dt `00569563..0056956c`); party forced to level 4 (`005694c1 CMP ...,4` / `005694d6 CALL 0x51e120`), level 4 gets 60% of 10 ms (AI4 key 1). RunActions budget `005d6d07 SUB EAX,[EBP+0x10]` / `005d6d0d MOV ECX,[0x009944c4]` (=1000): loop also exits on result 1 and after types 1,5,0x30,0x33 (`0x5D7160`). So a chain `[0x13,0x31,0xF]` costs 1-2 ticks, a chain with a MoveToPoint one tick per node.

### Q5 Client feedback
- 5.1 CONFIRMED the only queue mirror is `UpdateActionQueue 0x588ED0` (server), run from AIUpdate at most every 300 ms of object time (`00569efb CMP [EBP-0x78],0x12c` / `00569f02 JC 0x569f2d` / `00569f0a CALL 0x588ed0`; stamp `+0x3E0/+0x3E4`), and again on death (`0x59830F`). It walks the queue by group id (`0x53F4A0`, `0x53F3C0`) and fills 10 entries `[+0x3E8+i*0x1C]` (group id word, icon id `0x5D7220`, spell/item `+0x3F0`, target pos `+0x3F4`, target obj `+0x400`) for types 1,7,9,0xC,0xF,0x14,0x15,0x18..0x1D,0x26..0x2A,0x2B,0x2E,0x32,0x36..0x38,0x3E,0x43 (switch at `00588ff0`). Type `0x3F` has no case and the round list (+0xB00) is not read: a command shows only after the `0x3F` expanded it into a `0xF`/`0x2E` node (and an AIUpdate >= 300 ms later). If AIUpdate does not run while paused (SPECULATIVE) nothing refreshes during the pause.
- 5.2 CONFIRMED server-side client pings only at expansion/start: `0x7E5120(slot, 0x1B2C7)` at the top of `AIActionCastSpell` (`005b97e8`) and `AIActionItemCastSpell` (`005b8299`) for party members (`+0x11AC`), `0xA5ED` in the real-chain branch when out of range (`00565eab`, `0056528c`). Nothing at queue time (`param_19 == 0` branch has no such call). Client consumption of `+0x3E8` (packing) not traced (open).

### Extra findings and bug candidates (report to KPM contributors)
| id | finding | verdict | impact |
|---|---|---|---|
| X1 | `ActionManager` clears only for the player's own game object; vanilla UI cannot command non-controlled members so the gap never showed | CONFIRMED | any mod/script that queues cast/use on a follower leaves the `0x3F` behind `0x3D` |
| X2 | pending-test polarity differs: AddCastSpellActions clears when NOT pending (`005653cc JNZ`), AddItemCastSpellActions clears when pending (`00564983 JZ`) | CONFIRMED asm, intent SPECULATIVE | casts and item uses clear in opposite situations (only matters off the player path and in combat state 0) |
| X3 | `ClearAllActions` leaves the round list; orphan round nodes run first on the next `0x3F` | CONFIRMED | new command appears delayed / wrong order after a wipe |
| X4 | `0x3F` that returns 1 (AB8 gate) sits at head and blocks everything behind it incl. tail-added moves | CONFIRMED | up to ~1.5 s after a finished cast the actor's tail queue is held |
| X5 | AIActionCombat returns 2 and drops the `0x3F` when AB8 >= 1 but the round is not started (`006da491 JNZ` / `006da493 MOV EAX,[0x9944b8]`) | CONFIRMED asm, reach SPECULATIVE | possible lost placeholder with the round list non-empty |
| X6 | stale creature stamp `+0xC0/+0xC4` shortens/skips the spell conj phase | CONFIRMED mechanism, reach SPECULATIVE | cast without wind-up animation |

## (c) Runtime probes (observation only; all windows in (f) pass the checker)
| id | what | hook | log | expected signal |
|---|---|---|---|---|
| PQ2-P1 actor identity | is the commanded creature a player's game object | `AM-ENTRY 0x563C90` | actor id, mode `[ESP+4]`, `+0x520`, `+0x11AC`, `+0x1116`, queue types, is-controlled (`0x77D800`/leader id) | non-controlled actor -> H1 live; controlled -> H1 irrelevant |
| PQ2-P2 queue shape at command | head type and length when the command is queued | `ACS-ENTRY 0x565310`, `AICS-ENTRY 0x5648D0` (`param_19 / param_12 == 0` only) | actor, queue types before, round list count, AB8, A84 | `[0x3D]` before and `[0x3D,0x3F]` after = starvation |
| PQ2-P3 3F fate | every `0x3F` dispatch result and which gate | `A-ENTRY 0x6D9500`, `A-AA8`, `A-PAUSE`, `A-AB8`, `A-EXIT 0x6DA49F` | actor, AA8, pause byte `[[0xA1B4A4]+8+0x10078]&2`, AB8, A84, pending head time/timer, EAX at exit | EAX 1 with AB8>=1 = H2; EAX 2 = dropped; EAX 4 = expanded |
| PQ2-P4 cast timeline | per call return and elapsed | `C-ENTRY`, `C-CONJ` (EAX = elapsed `[EBP-0x64]`, `[EBP-0x80]` conj), `C-FIRE`, `C-EXIT` (EAX = 1/2/3) | spell id (`node+0x38`), target, stamp `+0xC0/+0xC4`, round `+0xA84/+0xAA8/+0xAB8` | fire at 170 ms; return 3 rows give reject reason via the site list in Q2 |
| PQ2-P5 item timeline | same for items | `I-ENTRY`, `I-START`, `I-FIRE`, `I-DUR`, `I-EXIT` | item id (`node+0x38`), spell id, elapsed `[EBP-0x5C]`, delay `[EBP-0x74]`, duration `[EBP-0x7C]` | default case 1 ms / 1331 ms |
| PQ2-P6 who clears | every ClearAllActions with caller | `CLR-ENTRY 0x541080` (log only when `+0x11AC` party, or `+0xEC==1` and queue contains 0x3F/0xF/0x2E), `SCRIPT-CLR 0x66F388` | return address `[EBP+4]`, actor, queue types, round list count, tag | maps wipes to BumpFriends / script / AddAttack / SaveMember |
| PQ2-P7 queue head at pop | head type distribution | reuse installed `RA-H09 0x5D6A60` (companion-ai `CompanionAiProbeRunActions`, `probe_rar`): extend it with a per-creature log of types {0x3D,0x3F,0xF,0x2E,0x10} and result, no new window | type x result runs; the existing `PR RAR REQ` fires after 200 consecutive result-1 ticks of one type | `0x3D` res 1 run while a `0x3F` sits behind = H1 |
| PQ2-P8 queue mirror | when the GUI array updates | `AIU-QSYNC 0x569EFB` | actor id, elapsed ms since last sync (`[EBP-0x78]`), queue types | confirms >= 300 ms cadence and no update while paused |
| PQ2-P9 round end | when AB8 clears | `ENDRND-AB8 0x58EA53` (ECX = round, `[ECX+0xB04]` owner) | owner id, `+0xA94`, `+0xA9C`, time since cast fire | round length ~3000 ms from cast start |

## (d) Open questions
- Which creature the user's command executes on (controlled leader vs follower)? P1 settles H1 vs H2/H3.
- Creator of action type 0x10 (`0x5D0C00`) and what `FUN_00591A70 RemoveSpellAction` leaves in the round list when a handler returns early (`0x589BF0` path).
- Medpac `baseitems`-record byte `[+0xAC]` (switch key of `0x5B8000`, `0x6D6E30` lookup) to know which timing case it takes (default 1/1331 assumed).
- Does AIUpdate run for the controlled leader while pause bit 2 is set, and which timer (`0x538E00`, exclusion list `0x5395A0`) it sees (other analyst).
- Client consumer of creature `+0x3E8..` (queue mirror) and of party slot `E+0x84` states `0x1B2C7/0xA5ED`.
- Delay arguments of the spell-cast event `0x5482A0` (posts via `0x51E170`): effect application may lag the fire by projectile time.
- Exact intent of the opposite clear polarity (X2) in `0x565310` vs `0x5648D0`.

## (e) Struct field table
| class | offset | size | type | proposed name | readers (addr) | writers (addr) | confidence | evidence |
|---|---|---|---|---|---|---|---|---|
| CSWSObject | 0x100 | 4 | CExoLinkedList* | actionQueue | `0x5D56B0`, `0x586B70` | `0x54AA10/0x54AA30` | high | `00586ed1 ADD ECX,0x100` / `00586ef1 CALL 0x521060` |
| CSWSObject | 0xC0/0xC4 | 4+4 | int | actionStart day/time stamp | `005ba6bf`, `005ba6cc` | `005d5bb5`, `005d5bc4` (only if both 0) | high | `005d5b94 CMP [ECX+0xc0],0` |
| CSWSObject | 0xEC | 4 | int | commandable | `005410e9` | `005c77c4` (forced 1 around follow) | high | `005410e9 CMP [EAX+0xec],0x1` |
| CSWSCreature | 0x11AC | 4 | int | partyMemberFlag | `005bf7fc` | `0058a35e` (AI5) | high | `005bf7fc MOV ECX,[EAX+0x11ac]` |
| CSWSCreature | 0x11B0 | 4 | int | walk state | `0x5C3F70` | `005bf925` (=3), `00541210` (=1) | med | `005bf925`, `00541210` |
| CSWSCreature | 0x1116 / 0x1118 | 1 / 4 | byte / id | actionClass / classTarget | `00563cd5 MOV DL,[ECX+0x1116]` | `0x5718A0`, `0x5719A0` | high | `0x5719A0` writes 1,2,0xA..0x21 |
| CSWSCreature | 0x520 / 0x524 | 4 / 4 | int | inCombat / combatExitMs (8000) | `0065d546`, `0065c5b5` | `0x57A560` | high | `0065d546 MOV EDX,[ECX+0x520]` |
| CSWSCreature | 0x570 | 4 | int | slot-6 fallback ms | AI4 | `005b97f3`, `005b82a4` (=3000) | high | `005b97f3 MOV [ECX+0x570],0xbb8` |
| CSWSCreature | 0x1F0 | 4 | int | spell fired flag | `005bac90`, `005baef3` | `005bb037` (=1), `005ba13d/005ba76b/005ba0e9` (=0) | high | `005bb037 MOV [EDX+0x1f0],1` |
| CSWSCreature | 0x3E0/0x3E4 | 4+4 | int | queue-mirror last update stamp | `00569ed7`, `00569ee4` | AIUpdate | med | `00569ed7 MOV EDX,[ECX+0x3e4]` |
| CSWSCreature | 0x3E8 + i*0x1C | 10x0x1C | struct | actionQueue mirror (id word +0, icon word +2, spell +8, pos +0xC, target +0x18) | none found | `00588f1f LEA ECX,[EAX+EDX+0x3e8]`, `005899b2` | med | `005899b2 MOV word ptr [EDX+ECX+0x3ea],AX` |
| CSWSCreature | 0x10DC | 4 | CSWSCombatRound* | combatRound | all | ctor | high | AI2 |
| CSWSCombatRound | 0xA84 / 0xA88 / 0xA8C | 4 | int | started / spellCastRound / spell id | `005ba1b6`, `0058f7a0` | `0058e8b4`, `0058e6dc` | high | `0058e6dc MOV [EAX+0xa88],1` |
| CSWSCombatRound | 0xA94 / 0xA9C | 4 | int ms | timer / remaining length | `00590220`, `0058f7a0` | `0058e8da` (3000), `0058fd50` | high | `0058e8da MOV [EAX+0xa9c],0xbb8` |
| CSWSCombatRound | 0xAA8 / 0xAB0 / 0xAB4 | 4 | int | paused / pause ms / infinite | `006d9554`, `00571030` | `0x58FC60`, `0x58FCD0` | high | `006d9554 MOV EAX,[EDX+0xaa8]` |
| CSWSCombatRound | 0xAB8 | 4 | int | actions started this round | `006d96c0`, `006da45a` | `005bb076`, `005b9447` (++), `0058ea53`, `0058e359` (=0) | high | `005bb076 MOV [EDX+0xab8],ECX` |
| CSWSCombatRound | 0xB00 | 4 | list* | round action list | `0x590110/0x590180/0x590220` | `0x590270`, `0x590430` | high | `0058e350` |
| CombatRound action node | +0 / +0x10 / +0x14 / +0x24.. | int / byte / id / ints | time / type (9 spell, 0xA item, 6/7 ...) / target / args | `0x590220` | `0x591670`, `0x591890` | high | `0058c810` zeroes time |
| ActionNode | +0 word / +0x6C word | | type / group id | `005d5be1` | AddAction | high | AI4 |
| CServerExoAppInternal | 0x10078 | 1 | byte | pauseState bits (2 = UI pause) | `00538cda MOVZX ECX,[EAX+0x10078]` | `00538f27` | high | `00538cda` |
| CServerExoAppInternal | 0x10048/0x1004C/0x10050 | 4 | timers | world / pause-excluded timers | `0x538E00` | - | med | C of `0x538E00` |
| CServerExoAppInternal | 0x10064 | 4 | list | client/player list | `0052f350 MOV ECX,[EAX+0x10064]` | - | med | `0x52F330` |

## (f) Hook-site catalogue (checked with a scratch copy of `check_atlas_windows.py`: boundary OK, length >= 5, no relative branch inside, no inbound target inside, no overlap with the 09-30g live TSV or any repo `*.hooks.toml`; `RA-H09` overlaps by identity with companion-ai). "plain" = non-consumed detour, EAX/regs restored by the wrapper, stolen bytes run after the handler. All distances to the sibling self-patch list (`0x40D69E`, `0x8Axxxx`) are > 1.2 MB.
| id | address | len | stolen bytes (.asm) | convention / live regs | consumed exit? | EAX / stack notes | hot / cold | nearest installed hook |
|---|---|---|---|---|---|---|---|---|
| C-ENTRY | 0x5B96F0 | 5 | `55 8B EC 6A FF` | thiscall: ECX = actor creature, `[ESP+4]` = action node (spell `+0x38`, target `+0x44`..) | no | before prologue; stolen PUSH/MOV run after handler | warm (per tick while casting) | `0x5BC5F4` (companion-ai) 12031 B |
| C-CONJ | 0x5BA785 | 6 | `8B 45 9C 3B 45 80` | EBP frame: elapsed `[EBP-0x64]`, ConjTime `[EBP-0x80]`, CastTime `[EBP-0x60]`, CatchTime `[EBP-0x7C]`, this `[EBP+0xFFFFFD48]`, node `[EBP+8]` | no (JNC `005ba78b` not stolen) | stolen CMP feeds JNC: handler must not touch flags (wrapper restores) | warm | `0x5BC5F4` 7785 B |
| C-FIRE | 0x5BB07C | 6 | `8B 85 7C FF FF FF` | after AB8++ (`005bb076`) and before `CALL 0x5482a0`; `[EBP-0x2C4..]` fire args | no | runs once per cast | cold | `0x5BC5F4` 5490 B |
| C-EXIT | 0x5BB2AA | 10 | `8B 4D F4 64 89 0D 00 00 00 00` | common epilogue of all 15 `JMP 0x5bb2aa`; EBP = frame | no | EAX = return code (1/2/3), ECX dead; `RET 0x4` after | warm | `0x5BC5F4` 4928 B |
| I-ENTRY | 0x5B8000 | 5 | `55 8B EC 6A FF` | ECX = actor, `[ESP+4]` = node (item `+0x38`, prop `+0x3C`, target `+0x44`) | no | | warm | `0x5BC5F4` 17903 B |
| I-START | 0x5B8F83 | 6 | `8B 4D A4 3B 4D 8C` | elapsed `[EBP-0x5C]`, start delay `[EBP-0x74]`, this `[EBP+0xFFFFFDA0]` | no (JNC `005b8f89`) | stolen CMP feeds JNC | warm | `0x5BC5F4` 13931 B |
| I-FIRE | 0x5B9414 | 6 | `8B 85 A0 FD FF FF` | just before AB8++ (`005b9447`) and `CALL 0x5482a0` | no | | cold | `0x5BC5F4` 12762 B |
| I-DUR | 0x5B9604 | 6 | `8B 4D A4 3B 4D 84` | elapsed `[EBP-0x5C]`, duration `[EBP-0x7C]` (CMP uses `[EBP-0x7C]`) | no (JNC `005b960a`) | branch target of `JNZ 0x5b9604` (start only) | warm | `0x5BC5F4` 12266 B |
| I-EXIT | 0x5B9695 | 10 | `8B 4D F4 64 89 0D 00 00 00 00` | common epilogue (11 `JMP 0x5b9695`) | no | EAX = return code | warm | `0x5BC5F4` 12117 B |
| A-ENTRY | 0x6D9500 | 5 | `55 8B EC 6A FF` | ECX = actor, `[ESP+4]` = `0x3F` node | no | | warm (per tick while a `0x3F` is queued) | `0x6D8BE2` 2329 B |
| A-AA8 | 0x6D9560 | 7 | `83 BD 24 FF FF FF 00` | `[EBP-0xDC]` = round `+0xAA8` value | no (JNZ `006d9567`) | stolen CMP feeds JNZ | warm | `0x6D9E2A` 2243 B |
| A-PAUSE | 0x6D957C | 8 | `6A 02 8B 8D 20 FF FF FF` | before `CALL 0x51c720`; read `[[0xA1B4A4]+8]+0x10078` & 2 yourself | no | stolen PUSH 2 pairs with the following CALL (not stolen): plain detour only | warm | `0x6D9E2A` 2214 B |
| A-AB8 | 0x6D96CC | 7 | `83 BD 04 FF FF FF 01` | `[EBP-0xFC]` = AB8 | no (JGE `006d96d3`) | stolen CMP feeds JGE | warm | `0x6D9E2A` 1879 B |
| A-EXIT | 0x6DA49F | 10 | `8B 4D F4 64 89 0D 00 00 00 00` | single epilogue (only `RET` at `006da4ad`) | no | EAX = 1/2/3/4 | warm | `0x6D9E2A` 1638 B |
| AM-ENTRY | 0x563C90 | 6 | `55 8B EC 83 EC 10` | ECX = actor, `[ESP+4]` = mode (1/2/4/8) | no | cold (once per player command / Add*Actions) | cold | `0x564770` (mouse-probe) 2778 B |
| AM-CLR1 / AM-CLR2 | 0x563CF0 / 0x563D06 | 5 / 5 | `6A 01 8B 4D F0` | stolen PUSH 1 + MOV ECX before `CALL 0x541080` | no | reached only for the player's object; plain detour only | cold | `0x564770` 2683 / 2661 B |
| ACS-ENTRY | 0x565310 | 9 | `55 8B EC 81 EC 20 01 00 00` | ECX = actor, args: spell `[ESP+4]`, class, target `param_10`, `param_19` (= 0 queue-into-round, 1 build) at `[ESP+0x4C]` | no | cold | cold | `0x564770` 2970 B |
| AICS-ENTRY | 0x5648D0 | 9 | `55 8B EC 81 EC 10 01 00 00` | ECX = actor, item `[ESP+4]`, last arg `param_12` (= 0 queue) | no | cold | cold | `0x564770` 346 B |
| UDT-ENTRY | 0x57D8B0 | 6 | `55 8B EC 83 EC 34` | ECX = actor, `[ESP+4]` type, `[ESP+8]` spell/item, `[ESP+0xC]` target | no | cold | cold | `0x575183` 34598 B |
| UI-ENTRY | 0x57B4C0 | 5 | `55 8B EC 6A FF` | ECX = actor, item id `[ESP+4]`, prop `[ESP+8]`, target `[ESP+0x14]` | no | cold | cold | `0x575183` 25398 B |
| CLR-ENTRY | 0x541080 | 6 | `55 8B EC 83 EC 7C` | ECX = victim object, `[ESP+4]` = flag, return address `[ESP]` = caller | no | filter in the handler (party / queue contains 0x3F,0xF,0x2E) | warm (58 call sites) | `0x544210` (perf-lab) 12682 B |
| SCRIPT-CLR | 0x66F388 | 5 | `6A 01 8B 4D F8` | `[EBP-8]` = victim creature, `[EBP-0x14]` = command struct | no | plain detour (stolen PUSH 1) | cold | `0x670370` 4067 B |
| ADD-ENTRY / ADDF-ENTRY | 0x53F7F0 / 0x540CA0 | 5 / 5 | `55 8B EC 6A FF` | ECX = actor, `[ESP+4]` = type, `[ESP+8]` = group | no | log only types 0x3F/0xF/0x2E/0x3D | warm | `0x53C5AA` (hotbar) 12860 B / `0x544210` 13675 B |
| AIU-QSYNC | 0x569EFB | 7 | `81 7D 88 2C 01 00 00` | `[EBP-0x78]` = elapsed ms since last mirror update, this `[EBP+0xFFFFFE6C]` | no (JC `00569f02`) | stolen CMP feeds JC | hot (per creature per tick) | `0x569A08` 1261 B |
| ENDRND-AB8 | 0x58EA53 | 10 | `C7 81 B8 0A 00 00 00 00 00 00` | ECX = round | no | stolen MOV zeroes AB8 | cold | `0x58A35E` 18159 B |
| STARTRND | 0x58E730 | 6 | `55 8B EC 83 EC 40` | ECX = round, target `[ESP+4]` | no | cold | cold | `0x58A35E` 17356 B |
| RA-H08 (AI4) | 0x5D5C22 | 6 | `8B 95 4C FE FF FF` | type-1 in `[EBP-0x1B4]`; actor `[EBP-0x1A8]` | no | HOT (per popped node); gate-checked in AI4; reuse only to count head types | hot | `0x5D6A60` 3640 B |
| RA-H09 (AI4) | 0x5D6A60 | 9 | `8B 55 E4 3B 15 C0 44 99 00` | actor `[EBP-0x1A8]`, type `[EBP-0x18]`, result `[EBP-0x1C]` | no (stolen CMP feeds the following JNZ) | ALREADY INSTALLED by companion-ai (`CompanionAiProbeRunActions`, `probe_rar=1` in `companion_ai.ini`): extend that handler instead of a second hook | hot | overlap = identity |

Reject-site list for `0x5B96F0` (optional, one probe per gate; 6-byte windows `8B 8D 48 FD FF FF` = `MOV ECX,[EBP+0xFFFFFD48]` before `CALL 0x589BF0`; all pass the checker, entry `0x5B96F0`): `0x5B986B`, `0x5B9B41`, `0x5B9BAB`, `0x5B9C1E`, `0x5B9C8B`, `0x5B9D44`, `0x5B9E2E`, `0x5BA945`, `0x5BA9DF`; C-EXIT already gives EAX=3 and these name the gate.

## (g) Mod relevance
- medpac / action hotbar paused-command bug: H1/H2/H3, probes P1-P7, X1-X4, Q5 feedback gap (`UpdateActionQueue` types).
- party-flag (`+0x11AC`): follower vs controlled actor decides which path applies (`ActionManager` gate, `0x3D/0x40` gates).
- FM1: BumpFriends clear (`0x586F43`) already logs the bumpee queue; a starved `[0x3D,0x3F]` queue qualifies for the vanilla clear (companion-ai `onlyFollowTypes`).
- A1 / FM2b: AB8 once-per-round gate (`006d96cc`) and the stale `+0x570` re-arm (`005b97f3`, `005b82a4`) share the same round state.
- E4: stamp `[+0xC0/+0xC4]` semantics (`005d5b94..5bb5`) and `0x3D` never completing.
- AIQ, enemy-AI, EFFOVF: none.
