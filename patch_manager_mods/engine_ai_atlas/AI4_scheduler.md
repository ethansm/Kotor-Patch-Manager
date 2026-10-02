# AI4 Scheduler and action queue (KOTOR II Steam swkotor2.exe 6a522e71)

Pipeline position: server tick -> `CServerExoApp` (`0x533830`, `0053396e CALL 0x0051d790`) -> **`CServerAIMaster::UpdateState 0x51D790`** (10 ms time-sliced scheduler over 5 AI-level lists, event pump, client sync) -> per object `AIUpdate` (vtable `+0x70`; creature `0x569320`) -> perception -> scripts -> **action queue** `CSWSObject::RunActions 0x5D56B0` (pop, dispatch through the table `0x5D7038`, requeue by result) -> AIAction handlers (movement = AI1, combat = AI2) -> events back to scripts via the AI event queue (`0x51E170`, drained at the top of each scheduler iteration). This page covers the scheduler budget and level policy, the per-iteration pre-pass (AIQ), the FM3 sentinel path, the RunActions loop and its guards, the full action-type table, and the CSWSCreature::AIUpdate branch map.

## Status
- Sources: batch `findings/AI4.md` (Sonnet) + `GATE_wave1.md` (section 1 AI4 verdicts, 2 reconciliation, 3 exceptions, 4 names, 5 windows, 6 lessons 8/9/10/14) + `LEAD_SPOTCHECKS.md` (AI4: items join level 0, dispatch table bytes, `+0x220` writer; all three CONFIRMED).
- Date: 2026-10-02. Static only. Gate-verified: `+0x220` meaning, items at level 0 (+ cost), the 7 decoded action-table entries, dismissed party member -> level 2, level-pass cut by a pending event (code high, reach SPECULATIVE). Action-table rows not spot-checked (60 of 67) are med (table bytes high; handler identities rest on DB/mapper names). Magnitudes (AIQ cost, FM3 reach) low.
- Full struct set in DB table `struct_fields`.

## Function table
| addr | name | conf | purpose |
|---|---|---|---|
| 0x51D790 | CServerAIMaster::UpdateState | high | scheduler; `0051d79f MOV [EBP-0x18],0x2710`, levels 4..0, sole caller `0053396e`. DB said ServerEvents::Update (med); gate renamed |
| 0x51D1C0 | CServerAIList::GetNextObject | med | cursor++ mod count; NULL if unresolved or type byte <= 4 (gate: DB name CServerAIMaster::NextObject, `this` = list) |
| 0x51D270 | CServerAIList::GetObjectAtPosition | med | id array[i] -> object (gate rename of GetObjectAt) |
| 0x51D150 / 0x51D170 | CServerAIList::AddObject / RemoveObject | high | add if absent / find + `DelIndex 0x49B840` |
| 0x49B840 | CExoArrayList::DelIndex | high | never touches the AIList cursor |
| 0x51E040 | CServerAIMaster::AddObject(obj, level) | high | removes from old level, appends, stores `obj+0x78` (`0051e0a8`) |
| 0x51E0C0 | CServerAIMaster::RemoveObject | high | `0051e10d MOV [ECX+0x78],0xffffffff` (gate new row) |
| 0x51E120 | CServerAIMaster::SetAILevel(obj, level) | high | no-op if equal, rejects outside 0..4 (`0051e15c CALL 0x0051e040`) |
| 0x51ED10 / 0x51ED80 | CServerAIMaster::EventPending / GetPendingEvent | high / med | head of `+0x54` list due when head <= (day,time) |
| 0x51E170 / 0x51E220 | CServerAIMaster::AddEventDeltaTime / AddEventAbsoluteTime | high / med | node `new(0x18)`, stores `[2..5] = p3..p6` |
| 0x51C280 | CServerExoApp::GetAIMaster | med | idiom `PUSH level; PUSH obj; CALL 0x51C280; ... CALL 0x51E120` |
| 0x521E60 | CSWSArea::AIUpdate | med | ambient roll 300000 ms, 6000 ms OnHeartbeat |
| 0x5573E0 | CSWSModule::AIUpdate | med | alternates area AIUpdate / module heartbeat `0x557260` |
| 0x569320 | CSWSCreature::AIUpdate | high | per-creature tick (branch map below) |
| 0x5F4C00 / 0x600F20 / 0x62C360 | CSWSPlaceable / CSWSItem / CSWSAreaOfEffectObject::AIUpdate | med | other vtable `+0x70` users (item `0x600F20` gate-verified, see item row) |
| 0x5FFD60 | CSWSItem::Constructor | med | vptr `005ffdc0 MOV [EAX+0x10],0x9972cc`; `005ffe52 CALL 0x0051e040` level 0 |
| 0x5D56B0 | CSWSObject::RunActions(day,time,qpcLo,qpcHi) | high | pop-dispatch loop; `005d5c28 JMP [EDX*4+0x5D7038]` |
| 0x5D7160 | ActionTypeStopsLoop | high | 1 for types 1, 5, 0x30, 0x33 |
| 0x53F7F0 / 0x540CA0 | CSWSObject::AddAction / AddActionToFront | high | `new(0x74)` node (`0053f818`), tail `0x54AA30` / head `0x54AA10` |
| 0x53E6F0 / 0x53FA80 / 0x53F7A0 / 0x53E740 | ActionNode ctor / SetParam / NextGroupId / FreeParams | high | node init, param store, group counter, free kind 4/5 |
| 0x541080 | CSWSObject::ClearAllActions(flag) | high | `vtbl+0x10` gate; resets `+0x11B0=1`, `+0x54C`, PI `+0x254..+0x260`, party follow slot `-1` |
| 0x5233A0 / 0x523320 | IncreaseAILevelPriority / IncrementPlayersInArea | high | `area+0x12C` 0 -> 1 promotes level-0 objects to 1 |
| 0x5235A0 / 0x523360 | DecreaseAILevelPriority / DecrementPlayersInArea | high | count -> 0 demotes level-1 creatures to 0 |
| 0x66B640 | Global::ExecuteCommandSetCreatureAILevel | high | saves old level in `obj+0x7C` (`0066b6e9`), jump table `0x66B758`; gate: DB name was WalkUpdateLocation_QuickWalk_... |
| 0x66B800 | ExecuteCommandResetCreatureAILevel | med | restores byte `[obj+0x7C]` (`0066b893`) |
| 0x66AD10 | ExecuteCommandActionStartConversation | med | level < 3: saves level to `+0x7C`, sets 3 (`0066b56d`) |
| 0x58A330 / 0x58A470 | SetPartyMemberFlag / SetIsPuppet | high | flag -> level 4; clear -> level **2** (`0058a458`) |
| 0x63C1E0 | UpdateObjectInPlayerUpdateSet | med (write high) | `+0x220 = 0x63DD30(obj, player) \|\| [+0x260]` (`0063c257`) |
| 0x63DD30 | IsObjectInPlayerUpdateRange(obj, player) | med | self/party -> 1; range^2 vs max(spot,listen) + `0x585480` not invisible; seen/heard bit |
| 0x5C3F70 | CSWSCreature::WalkUpdateLocation_QuickWalk | med | pre-pass call `0051d9bc`; zeroes obj `+0xC0` at `0x5C454B/0x5C5E42` |
| 0x5DF920 | CSWSCreature::AIActionType47_ReequipResumeAttack | low | action 0x47 (`005d682a` -> `CALL 0x005df920`); name from gate, body per batch |

## Action-type table (RunActions `JMP [EDX*4+0x5D7038]`, index = type-1, 0x47 dword entries)
`005d5c15 CMP [EBP+0xfffffe4c],0x46` / `005d5c1c JA 0x005d688e`. [G] = gate-verified from the vanilla exe table decode (7 entries: 1, 0xC, 0x3F, 0x3D, 0x1E, 0x11, 0x47). All other rows: table bytes high, handler identity med (DB/mapper names; unnamed ones SPECULATIVE). Scope c = guarded by object type byte `[obj+8] == 5` (creature), else any object class; a failed guard leaves result 3 (node silently deleted). Result var `[EBP-0x1C]` reset to 3 each iteration (`005d5bf4`).
| type | handler | scope | name | gate |
|---|---|---|---|---|
| 0x01 | 0x5BFC00 | c | AIActionMoveToPoint (stop-loop type); `005d5c4e CALL 0x005bfc00` | [G] |
| 0x02 | 0x5BBF40 | c | AIActionCheckMoveToObject | |
| 0x03 | 0x5BB5A0 | c | AIActionCheckMoveToPointRadius | |
| 0x04 | 0x5BBB70 | c | AIActionChangeFacingPoint | |
| 0x05 | 0x5BE950 | c | AIActionJumpToPoint (stop-loop type) | |
| 0x06 | 0x5D9DA0 | any | AIActionPlayAnimation | |
| 0x07 | 0x5CF8A0 | c | AIActionPickUpItem | |
| 0x08 | 0x5BE0A0 | c | AIActionEquipItem | |
| 0x09 | 0x5CFF50 | c | AIActionDropItem | |
| 0x0A | 0x5BC700 | c | AIActionCheckMoveToPoint | |
| 0x0B | 0x5D0A80 | c | AIActionUnequipItem | |
| 0x0C | 0x6D7390 | c | AIActionPhysicalAttacks; obj `+0x88` = node param0 around the call (`005d633f`, `005d636c`); `005d635e CALL 0x006d7390`, entry `0x5D631D` | [G] |
| 0x0D | none (`0x5D688E`) | - | default, deleted | decode also shows no handler |
| 0x0E | 0x5D9890 | any | AIActionSpeak | |
| 0x0F | 0x5B96F0 (creature) / 0x5F6180 (placeable, type byte 9) | c / 9 | AIActionCastSpell (then `0x590180` if result 2 or 3) | |
| 0x10 | 0x5D0C00 | c | unnamed (174 B, calls `0x590180`) | |
| 0x11 | 0x5BC1D0 | c | AIActionCheckMoveToObjectRadius; entry `0x5D5F79`, `005d5f98 CALL 0x005bc1d0` | [G] |
| 0x12 | 0x5BCA60 | c | AIActionCheckMoveToPoint_2 | |
| 0x13 | 0x5BB2C0 | c | AIActionChangeFacingObject | |
| 0x14 / 0x15 | 0x5DB0A0 / 0x5DAD10 | any | AIActionOpenDoor / CloseDoor | |
| 0x16 | 0x5CF7B0 | c | AIActionOrientCamera | |
| 0x17 | 0x5D9BF0 | any | AIActionPlaySound | |
| 0x18 | 0x5D75B0 | any | AIActionDialogObject | |
| 0x19..0x1D | 0x5B43C0, 0x5B3380, 0x5B2C10, 0x5B5360, 0x5B5AD0 | c | DisarmMine, RecoverMine, FlagMine, ExamineMine, SetMine | |
| 0x1E | 0x5D9AA0 | any | AIActionWait; entry `0x5D656E`, `005d6578 CALL 0x005d9aa0` | [G] |
| 0x1F | 0x5D9570 | any | unnamed (274 B, no callees: dead/zero-HP check, `vtbl+0xC0`, returns 3 or 2) | |
| 0x20 | 0x5D9690 | any | unnamed (320 B, calls `0x563D20/0x563DA0`) | |
| 0x21 / 0x22 / 0x23 | 0x5D97E0 / 0x5DA4D0 / 0x5DAB80 | any/any/c | SpeakStrRef, GiveItem, TakeItem | |
| 0x24 | 0x5BD0B0 | c | AIActionEncounterCreatureDestroySelf | |
| 0x25..0x28 | 0x5D99D0, 0x5DB8F0, 0x5DDA60, 0x5DFBE0 | any | DoCommand, UnlockObject, LockObject, UseObject | |
| 0x29 / 0x2B / 0x36 | 0x5D1990 | c | constant stub `return 3` | |
| 0x2A | 0x5D17D0 | c | constant stub `return 2` | |
| 0x2C | 0x5BB920 | c | AIActionCheckInterAreaPathfinding | |
| 0x2D | 0x5BF000 | c | AIActionRandomWalk | |
| 0x2E | 0x5B8000 | c | AIActionItemCastSpell | |
| 0x2F | 0x5E0E70 | any | AIActionSetCommandable | |
| 0x30 | 0x5BE1F0 | c | AIActionJumpToObject (stop-loop type) | |
| 0x31 | 0x5BB470 | c | AIActionCheckMoveAwayFromLocation | |
| 0x32 | 0x5D25E0 | c | AIActionCounterSpell | |
| 0x33 | 0x5CCC50 | c | AIActionDrive (stop-loop type) | |
| 0x34 / 0x35 | 0x5D17F0 / 0x5D18C0 | c | 2000 ms timed animation actions (anim 0x274F / 0x2711), unnamed | |
| 0x37 / 0x3A | 0x5CEED0 / 0x5CF660 | c | AIActionForceFollowObject / CheckForceFollowObject | |
| 0x38 | 0x5D19B0 | c | 3102 B, strings "Subs_Save"/"Save_DC"; mapper says AIActionHeal (low) | |
| 0x39 / 0x3B / 0x42 | none (`0x5D688E`) | - | default, deleted | decode also shows no handler |
| 0x3C | 0x5BEFC0 | c | unnamed (64 B, calls GetArea `0x5453C0`) | |
| 0x3D | 0x5BF7F0 | c + `[+0x11AC] != 0` | AIActionFollowLeader; leader id (`0x73F470`) == INVALID returns 1; entry `0x5D5C5B`, `005d5d35 CALL 0x005bf7f0` | [G] |
| 0x3E | 0x5D7480 | any | AIActionBarkString | |
| 0x3F | 0x6D9500 | c | AIActionCombat; entry `0x5D63AE`, `005d63cd CALL 0x006d9500` | [G] |
| 0x40 | 0x5BCDC0 | c + `[+0x11AC] != 0` | AIActionCheckMoveToFollowRadius | |
| 0x41 | 0x5D29B0 | c | unnamed (55 B, calls `0x5D51A0`) | |
| 0x43 | 0x5DEC20 | any | AIActionDoorSaber | |
| 0x44 | 0x5D29F0 | c | AIActionStanceTotalDefense | |
| 0x45 | 0x5D2EA0 | c | unnamed (216 B, calls `0x57A990`, count `0x5210F0`) | |
| 0x46 | 0x5BF940 | c + `[creature+0x11FC] != 0` | AIActionFollowOwner (puppet flag, `SetIsPuppet 0x58A47F`) | |
| 0x47 | 0x5DF920 | any | AIActionType47_ReequipResumeAttack (low): ClearAllActions(1), re-equip slots, re-queue attack at `[round+0xB20]` if `[+0x520]!=0 && [+0x11E8]==1`; returns 2; entry `0x5D682A` | [G] |
Type > 0x47: no handler. 67 typed slots, 66 distinct handlers, 4 constant stubs.

## Key behaviours
1. Budget (CONFIRMED): 10,000 us total (`0051d79f`); levels 4,3,2,1 each get 60% of what is left (`PUSH 0x3C`, `PUSH 0x64`, `_allmul`/`_aulldiv`), level 0 gets all of it (`0051d809..0051d812`). Fully-used worst case L4 6000 / L3 2400 / L2 960 / L1 384 / L0 256 us. Order 4,3,2,1,0 (`0051d7ad`, `0051dd96`).
2. A non-empty level always runs >= 1 AIUpdate (first-iteration elapsed = 0: `0051d822`/`0051d82b` from the same QPC read). An overrun is charged only up to the budget (`0051dd84 SUB EAX,[EBP-0x20]`), so a tick can exceed 10 ms; the post-pass client sync (`0051dda1..0051df9d`) is O(N) and unbudgeted. A single AIUpdate is never preempted; slow objects >= 75,000 us are logged (`0051dca4`).
3. AIQ structure (CONFIRMED, magnitude SPECULATIVE): the per-iteration pre-pass (`0x51D842..0x51D9EC`) re-walks the whole level list once per loop iteration (`0x51D88D CALL 0x51D270`, per object), iterations = AIUpdates + 1 on a completed pass; back edge `0051dd1c JC 0x0051d842`. Cost per level is quadratic in n_L. Level 0 holds placeables/doors/triggers/AoE (8 AddObject callers) and **all items** (`005ffe42 PUSH 0x0` / `005ffe52 CALL 0x0051e040`); gate: the item vtable `0x9972CC` slot `+0x70` = `0x600F20` (rdata `0x99733C`) is cheap (property-list scan for type 10 unless a timed property 0xE..0x12 exists); the cost is the scheduler pre-pass lookup per iteration.
4. Pre-pass gate: object `+0x220 != 0` && creature && (head action type in {1, 0x3D, 0x46} (`0051d900/0051d90c/0051d918`) or `+0x11B0` in {4,5,6}) -> `0x5C3F70` QuickWalk (`0051d9bc`); result 2 -> `+0x11B0 = 1` (`0051d9e2`). ~4x redundant QuickWalk calls per tick for 3 followers in L4. Minimal gate H04: replace `0051d842 MOV [EBP-0x44],0` (SPECULATIVE equivalence).
5. `+0x220` is the player update-set flag (gate, CONFIRMED): only runtime writer `0063c257` in `0x63C1E0` = `0x63DD30(obj, player) || ForceAlwaysUpdate(+0x260)`. 1 for self/party (`+0x11AC`), creatures in perception range and not invisible, or seen/heard by the player. NOT a party flag (U3 was wrong); visible enemies are included. It gates the pre-pass QuickWalk and the client sync. Ctor writes 0 (`0053edc3`).
6. AI level policy (levels in `obj+0x78`, -1 none; ctor `0053eb61`): 0 idle/no player in area, 1 player in area, 2 combat, 3 conversation or party-NPC spawn, 4 party/PC/puppet. Transitions: ctors AddObject level 0 (creature `0x5622BB`, placeable `0x5EEE6C`, item `0x5FFE52`, others); AddToArea `0x5691F7` 0 -> 1 / `0x56923E` 1 -> 0 by `area+0x12C`; first player entering promotes 0 -> 1 (`0x5234C1`, `0x523546`, `0x523589`), last leaving demotes creatures 1 -> 0 (`0x523693`); AIUpdate forces party/PC to 4 (`0x5694D6`), combat 0/1 -> 2 (`0x56A0B5`), out of combat 2 -> 0/1 (`0x56A0FE/0x56A111`); SpawnInHeartbeatPerception tail drops any level to 0 when `area+0x12C == 0` (`0x56C01C`, FM7); party NPC creation 3 (`0x5FAD3D`); StartConversation 3 (`0x66B56D`); script Set/ResetCreatureAILevel (`0x66B74B`/`0x66B893`). Level 3 is never demoted automatically.
7. Dismissed party member (SetPartyMemberFlag(0, arg2 != 0)) goes to level **2**, not 0/1 (`0058a448 PUSH 0x2` / `0058a458 CALL 0x0051e120`); its own next AIUpdate demotes it to 0/1 (self-transition, FM3 skip-one-object effect). `+0x11AC` has one runtime writer (`0x58A35E`).
8. FM3 sentinel path (CONFIRMED mechanism; reach LOW at L4, LOW-MEDIUM at L2; magnitude SPECULATIVE): sentinel set `0051dbe1/0051dbe4` (only when `[EBP-0x38]==1`), dispatch guard `0051dc5c/0051dc65/0051dc6a`, "done" only `0051dc7f MOV [EBP-0x40],1`, level end `0051dd33`. It breaks when the sentinel S changes level inside its own AIUpdate (`0x5694D6`, `0x56A0B5`, `0x56A0FE`, `0x56A111`, `0x56C01C`), is destroyed (`0x51E0C0`), or is unresolved (zero AIUpdates that level that tick). Every self-transition removes index == cursor via DelIndex, so one following object is skipped for the tick.
9. A pending event at the loop bottom ends the level pass: `0051dcec CALL 0x0051ed10` / `0051dcf3 JNZ 0x0051dd33` (`0x51ED10` = head time <= tick snapshot). Gate: perception SEEN/HEARD/VANISHED posts (`0x576DC0`) and all OnBlocked posts use delta 0, stamped with `0x51AD20`, so a same-ms post cuts the pass; reach depends on ms granularity or a paused world timer (`+0x24 == 1`). Code CONFIRMED, reach SPECULATIVE.
10. RunActions loop: result 4 -> `AddTail 0x54AA30`; result 1 -> `AddHead 0x54AA10` (`005d6a88`); else `vtbl+0xC0(INVALID)` (except type 0xC) and free node. Stop when result == 1, elapsed > 1000 us (`005d6d07..005d6d0d`, `[0x9944C4]`), or `ActionTypeStopsLoop` (types 1, 5, 0x30, 0x33), or queue empty: a finished MoveToPoint/JumpTo*/Drive always ends the tick's loop. Result 2 and 3 are treated alike (settled).
11. Queue guards: > 0x4B (75) logs code 0xA1; > 0x1F4 (500) logs 0xA4 and pops head nodes while `vtbl+0x74(node,1) == 1`; > 1000 deletes every node. Effect count `this+0x14C` > 500 logs 0xA2, > 1000 calls `0x544A10` (EFFOVF, `0x5D6F5E..`). No stuck/timeout logic; `[obj+0xC0/0xC4]` (action-start stamp, set at pop `005d5bb5`) is read by Wait/Dialog/Door/Use/Cast but not by MoveToPoint, and QuickWalk zeroes it.
12. Dispatch details: `0x5D688E` is the common tail for no-handler types; current node `[this+0x20C]`, type word `[this+0x84]`, master `[+0x7C]` = last type (`005d6a2d`); empty-queue branch resets `+0x550,+0x548,+0x568,+0x56C,+0x54C` to INVALID and PI `+0x254..+0x260`. No NULL check on the 0x74-byte node allocation (`0053f82c`..`0053f85f`, SPECULATIVE reach).
13. Node = 0x74 B: type `+0`, 13 x (kind `+4+4i`, value `+0x38+4i`), group word `+0x6C`, count word `+0x6E`, flag `+0x70`. Group ids: `[obj+0x14]` increments per `-1` request, `[obj+0x16]` last issued. No de-dup in AddAction.
14. `CSWSCreature::AIUpdate 0x569320` order: (3) QPC stamp `0x569440`; (4) `0x56948E..0x5694D6` force level 4 for party/PC; (5) `0x5694E3` `0x56B8A0(1)`; (6) dt `[+0xD0]` `0x569525..0x56955E`; (7) `0x571030`, `0x57DD90`, `0x544AD0`, **RunActions `005695b1`**; (8) position/height fix for party; (9) `0x5718A0`, `0x56C310`; (10) in combat `[+0x520]==1`: `[+0x524] <= 0` -> `SetCombatState(0)` (`00569923`), else `-= dt`; (12) **slot-6 fallback `0x5699CE..0x569A94`** (`[+0x570] <= 0`, alive, `00569a67 CALL 0x6FD8D0`, reset `00569a72`); (13) HP/FP regen **party members only** (`00569b3a/00569b45`); (21) combat-level maintenance `0x56A064..0x56A116`; (22) `[+0x11CC]` timer. Blocks 1, 2, 14 are debug flags with no code writers.

## Struct fields
Full set in DB table `struct_fields`.
| class | offset | type | name | conf | evidence |
|---|---|---|---|---|---|
| CServerAIMaster | 0x00 | ptr | serverInternal | high | `0051d7bc` |
| CServerAIMaster | 0x04+0x10*L | int* | list[L].ids | high | `0051d889` |
| CServerAIMaster | 0x08+0x10*L | int | list[L].count | high | `0051d860` |
| CServerAIMaster | 0x0C+0x10*L | int | list[L].cursor | high | `0051d1dc` |
| CServerAIMaster | 0x54 | list | eventQueue | high | `0051ed1c` |
| CServerAIMaster | 0x58 | ptr | effectHandler | med | U3 |
| CServerAIMaster | 0x60..0x78 | diag | diagCurrentScript string and 5 ints (reset `0051dc06..0051dc48`) | low | |
| CServerAIMaster | 0x70 | int | perception-ran flag (set `0x5D0F8F`, cleared `0x51DC2E`, read `0x54A164`) | med | AI3 X5 |
| CServerAIMaster | 0x7C | int | lastActionType | med | `005d6a2d` |
| CSWSObject | 0x04 / 0x08 | id / byte | objectId / objectType (4 area, 3 module, 5 creature, 9 placeable) | high | `0051da82` |
| CSWSObject | 0x14 / 0x16 | word | actionGroupCounter / currentGroup | high | `0x53F7A0` |
| CSWSObject | 0x78 | int | aiLevel (-1 none) | high | `0051e0a8`, `0053eb61` |
| CSWSObject | 0x7C | byte | savedAiLevel | med | `0066b6e9` |
| CSWSObject | 0x84 | word | currentActionType (0xFFFF none) | high | `005d5bed` |
| CSWSObject | 0xAC / 0xB0 | int | lastRunActions day/time | med | `005d6a4b` |
| CSWSObject | 0xC0 / 0xC4 | int | actionStart day/time | high | `005d5bb5` |
| CSWSObject | 0x100 | list | actionQueue | high | `0x54AA10/0x54AA30` |
| CSWSObject | 0x14C | int | effectCount | med | EFFOVF guard |
| CSWSObject | 0x20C | ptr | currentActionNode | high | `005d5bd3` |
| CSWSObject | 0x220 | int | player update-set flag (visibleToPlayer/in update range) | high | `0063c257`; gate: up from med; not a party flag |
| CSWSObject | 0x260 | int | ForceAlwaysUpdate | high | `005744c3` |
| ActionNode | 0x00 | int | type | high | `0053f85f` |
| ActionNode | 0x04+4i / 0x38+4i | int | param[i].kind (1 int, 2 float, 3 object id, 4 CExoString*, 5 ptr) / value | high | `0x53FA80` |
| ActionNode | 0x6C / 0x6E / 0x70 | word / word / int | groupId / paramCount / flag (=1, no reader) | high / high / low | `0053f882` |
| CSWSCreature | 0x11AC | int | partyMemberFlag | high | `0058a35e` |
| CSWSCreature | 0x11B0 | int | moveState (pre-pass reads 4,5,6; written 1) | med | `0051d9e2` |
| CSWSCreature | 0x520/0x524/0x528/0x570 | int | inCombat / combatExitTimer / timerFrozen / fallbackTimer | high | `00569a72` |
| CSWSArea | 0x12C | int | playersInArea | high | `0x523320/0x523360` |
| CSWSArea | 0x194 / 0x198 | id* / int | objectIds / count | med | `0x5233A0` |

## Bugs and candidates
| item | verdict | user impact | fix idea | mod item |
|---|---|---|---|---|
| AIQ: pre-pass quadratic in n_L, items at level 0 | CONFIRMED structure, magnitude SPECULATIVE/low | UpdateState may burn most of its 10 ms in dense areas; level-0 objects updated every ~n/U ticks | H04 pre-pass once per level (`skip_original_bytes`, store `[EBP-0x44]`); measure first with perf-lab / P1-P2 | AIQ, perf |
| FM3 sentinel lost on self-transition / destroyed / unresolved | CONFIRMED mechanism, reach LOW | rest of the level slice spent re-running AIUpdates once; one following object skipped per self-transition | R1 hooks H01-H03 (log), no fix designed | FM3 |
| FM7: AI level 0 in player-less area for any level (`0x56C01C`) | CONFIRMED (AI3 agrees) | party member toggles 4 -> 0 -> 4 every ~4 s | see AI3 block 3 tail | FM7, FM3 |
| `+0x220` mis-documented as party flag | CORRECTED (gate) | pre-pass QuickWalk and client sync reach visible enemies too | docs only | AIQ, perf |
| No stuck/timeout in RunActions; `+0xC0` not read by MoveToPoint | CONFIRMED | stuck moves never time out (E4); queue capped only at 75/500/1000 | stuck check can key on `+0xC0` only if QuickWalk's reset is accounted for | E4, A1, EFFOVF |
| Finished move ends the tick's action loop | CONFIRMED | next queued action runs one AIUpdate later | note for FM2b timing | FM2b, A1 |
| Dismissed party member lands at level 2 | CONFIRMED | demoted by its own AIUpdate, self-transition | none | FM3, party-flag |
| Level-pass cut by delta-0 event | code CONFIRMED, reach SPECULATIVE | perception/OnBlocked posts may end a pass early | probe AI4-H02 with "cut by event" reason | FM3, AIQ |
| No NULL check on action-node alloc | CONFIRMED, reach SPECULATIVE | only on operator new failure | none | none |
| Conversation end level 3 | OPEN | no engine caller of ResetCreatureAILevel found | script command only | enemy-AI |

## Hook sites
Gate-checked (section 5: boundary OK, no relative branch inside, no inbound target inside; no overlap with live perf-lab `0x51D790/0x51DC70/0x51DC86` or shipped `0x56BDB4/BD/E3`):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| FM3 sentinel set (AI4-H01) | 0x51DBE1 | 6 | `8B 4D D8 89 4D C4` | cold | ends exactly where the unshipped hot window `0x51DBE7` starts: never ship both |
| FM3 level end (AI4-H02) | 0x51DD33 | 6 | `8B 4D D0 2B 4D F0` | cold | stolen SUB produces CF for the later SBB; safe only because KPM runs stolen bytes after the handler; branch targets `0x51DCF3/0x51DCF9/0x51DD22` |
| FM3 remove (AI4-H03) | 0x51E0C9 | 6 | `8B 45 08 8B 48 78` | cold | after list erase |
| RunActions dispatch (AI4-H08) | 0x5D5C22 | 6 | `8B 95 4C FE FF FF` | HOT (per action) | next insn is indirect `JMP [EDX*4+0x5D7038]` (not stolen); type-1 in `[EBP-0x1B4]`, EDX not yet loaded |
| RunActions result (AI4-H09) | 0x5D6A60 | 9 | `8B 55 E4 3B 15 C0 44 99 00` | HOT | stolen CMP feeds `JNZ` (stolen bytes last); gives type x result histogram |
| Party-flag census | 0x58A35E | 6 | `89 88 AC 11 00 00` | cold | only runtime writer of `+0x11AC`; handler sees OLD flag at `[EAX+0x11AC]`, new value in ECX; entry `0x58A330` has 15 call sites |

Batch-only (not gate-checked; none overlaps installed windows):
| id | addr | len | stolen bytes | hot/cold | purpose |
|---|---|---|---|---|---|
| AI4-H04 | 0x51D842 | 7 | `C7 45 BC 00 00 00 00` | HOT | pre-pass counter / gate; target of `JC 0x51DD1C`, `JBE 0x51DD2D` |
| AI4-H05 | 0x51D815 | 5 | `A1 88 B4 A1 00` | cold | level start (n_L, budget); JMP target `0x51D807` |
| AI4-H06 | 0x51DDA1 | 7 | `C7 45 CC 04 00 00 00` | cold | client-sync start; JL target `0x51D7D9` |
| AI4-H07 | 0x51E151 | 7 | `8B 45 0C 50 8B 4D 08` | cold | level really changes (obj `[ebp+8]`, new `[ebp+0xC]`, old `[ebp-4]`) |
| AI4-H10 | 0x5D6D07 | 6 | `2B 45 10 1B 55 14` | HOT | RunActions time (EDX:EAX = QPC now) |
| AI4-H11 | 0x5D6D87 | 12 | `8B 8D 58 FE FF FF 81 C1 00 01 00 00` | HOT | queue length; JNZ target `0x5D5B5B` |
| AI4-H12 | 0x5694CA | 8 | `6A 04 8B 95 6C FE FF FF` | cold | force level 4 |
| AI4-H13 | 0x5699CE | 7 | `83 BD E8 FE FF FF 01` | HOT | fallback gate; stolen CMP feeds `JNZ` (last) |
| AI4-H14 | 0x569A72 | 10 | `C7 81 70 05 00 00 B8 0B 00 00` | cold | fallback reset; ECX = this |
| AI4-H15 | 0x56A0A9 | 8 | `6A 02 8B 85 6C FE FF FF` | cold | combat -> level 2 |
Informational (installed): perf-lab `0x51D790` (9 B), `0x51DC70` (5 B `8B 4D D8 8B 11`), `0x51DC86` (7 B `C7 45 C8 00 00 00 00`).

## Open items
- Real n(L0)/n(L1) in play (are all party items real CSWSItem objects in the lists?): P1 perf-lab + H05.
- QuickWalk `0x5C3F70` dt-dependence/idempotence (8 KB, unread) before the H04 gate is judged.
- Unnamed handlers 0x10, 0x1F, 0x20, 0x34, 0x35, 0x38, 0x3C, 0x41, 0x45; `vtbl+0x74` and `vtbl+0xC0/+0xC4` of CSWSObject unidentified.
- Which engine path ends a conversation's level 3; class of objects registered by `0x616A70, 0x61DCF0, 0x622C90, 0x62AB10`.
- Debug globals `DAT_00A7E098/E088/E08C/E090/E0D4` (no instruction writers): confirm 0 in retail.
- Game time source (`0x51AD20`) QPC vs frame-quantised decides reach of the event-cut (gate: probe AI4-H02). Module heartbeat `0x557260` timing not traced.
- Gate carry-over: `0x585CC0` metric (AI1/AI2 ping-pong).

## Mod relevance
| item | served by |
|---|---|
| AIQ, perf | pre-pass cost model, level-0 items, `+0x220` correction, budget order, H04/H05/H06 |
| FM3 | sentinel path, self-transition skip, dismissed member -> level 2, delta-0 event cut |
| FM7 | level-0 drop at `0x56C01C` |
| FM1, FM6 | `ClearAllActions` semantics (`+0x11B0`, follow slot) |
| FM2b, A1, E4 | RunActions loop-stop rules, 1000 us budget, `+0xC0` stamp, queue guards |
| EFFOVF | effect-count guard 500/1000 |
| party-flag (`+0x11AC`) | creature+flag gated types 0x3D/0x40, regen party-only, census window `0x58A35E` |
| enemy-AI | enemies run in L2 only while in combat |
| Mod 8, E3, E7, E9, M5, M8, P1, P2, PFLK, medpac, C1 | none from this batch |
