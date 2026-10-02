# AI4 Scheduler and action queue (Steam swkotor2.exe 6a522e71, static, 2026-10-02)

## Summary (most mod-relevant first)
1. **AIQ is worse than U3 stated, and it hits level 0/1 hardest.** The per-iteration pre-pass (0x51D842..0x51D9EC) re-walks the whole level list once per loop iteration, and iterations = objects visited + 1. Every `CSWSItem` constructor (0x5FFD60 -> AddObject at 0x5FFE52, level 0) puts inventory/container items into the level-0 list, so n(L0) is plausibly hundreds to thousands. Level 0 gets "all the remaining budget" (0x51D7E3), so in dense areas UpdateState can spend most of its 10 ms every frame on `GetObjectAt` lookups (structure CONFIRMED, magnitude SPECULATIVE; perf-lab `PerfLabAiTick` 0x51D790 already measures us/tick).
2. **Budget semantics (all CONFIRMED):** 10,000 us total (0x51D79F), levels 4,3,2,1 each get 60% of what is left (0x51D7E7 `PUSH 0x3C`, 0x51D7F8 `PUSH 0x64`), level 0 gets all of it; a non-empty level always runs >= 1 AIUpdate (first-iteration elapsed = 0); an overrun is charged only up to the budget (0x51DD81), so total tick time can exceed 10 ms; the post-pass client sync (0x51DDA1..) is O(N) and unbudgeted.
3. **Action table decoded:** `JMP [EDX*4+0x5D7038]` at 0x5D5C28, 0x47 dword entries indexed by type-1; types 0xD, 0x39, 0x3B, 0x42 and > 0x47 have no handler (result stays 3 = deleted). 67 typed slots, 66 distinct handler functions, 4 slots are constant stubs (0x29/0x2B/0x36 share 0x5D1990 and return 3, 0x2A returns 2). Node = 0x74 bytes (type, 13 x (kind,value), group word, count word, flag dword). RunActions stops after MoveToPoint/JumpToPoint/JumpToObject/Drive even when they finish (`ActionTypeStopsLoop` 0x5D7160: 1, 5, 0x30, 0x33).
4. **AI levels:** 0 idle/no player in area, 1 player in area, 2 combat, 3 conversation (script/StartConversation, saved level in obj+0x7C) or party-NPC spawn, 4 party/PC/puppet. Dismissing a party member (SetPartyMemberFlag(0,1)) drops it to level **2**, not 0/1 (0x58A458). FM3 mechanism re-confirmed; reach is LOW for level 4 and LOW-MEDIUM for level 2 (each self-transition of the sentinel object spends the rest of that level's slice); every self-transition also skips the next object for one tick.
5. **Correction to U3/R1:** `[obj+0x220]` (the pre-pass gate and the client-sync gate) is NOT a "party/PC flag". Its only runtime writer is 0x63C257 (`*(int *)(local_14 + 0x220) = local_c`, local_c = 0x63DD30 distance/visibility test vs the player, forced to 1 for party members and the controlled player, and when `[+0x260] != 0`). It means "visible to / in update range of the player".

Mapper note: names marked (mapper) come from `AI0_map_*.tsv`; rows marked [C] were checked against `decomp/` (ret N, callees, quoted lines).

## (a) Function table
| addr | proposed name | conf | purpose | evidence |
|---|---|---|---|---|
| 0x51D790 | CServerAIMaster::UpdateState [C] | high | 10 ms time-sliced scheduler, 5 levels, event pump, client sync | `0051d79f MOV [EBP-0x18],0x2710`; sole caller 0x533830: `0053396e CALL 0x0051d790` with `[EAX+0x10044]` (master) |
| 0x51D1C0 | CServerAIList::GetNextObject [C] | high | cursor++ mod count, id -> object, NULL if unresolved or type byte <= 4 | `param_1[3] = param_1[3] + 1; if (param_1[1] <= param_1[3]) param_1[3] = 0;` |
| 0x51D270 | CServerAIList::GetObjectAtPosition(i) [C] | high | id array[i] -> object (hash lookup), same NULL rule | `iVar1 = FUN_0051c0a0(*(undefined4 *)(*param_1 + param_2 * 4));` |
| 0x51D150 / 0x51D170 | CServerAIList::AddObject / RemoveObject [C] | high | add id if absent (0x5210A0 -> 0x476620 count==0) / find index (0x4142E0 linear) + DelIndex 0x49B840 | `iVar1 = FUN_00476620(param_1); if (iVar1 == 0) { FUN_0083ea60(param_1);` ; `FUN_0049b840(iVar1)` |
| 0x49B840 | CExoArrayList::DelIndex [C] | high | count--, shift down, never touches the AIList cursor | `param_1[1] = param_1[1] + -1; for (; param_2 < param_1[1]; ...` |
| 0x51E040 | CServerAIMaster::AddObject(obj, level) [C] | high | level 0..4 and obj != 0; removes from old level (0x51E0C0) then appends to new list; stores obj+0x78 | `0051e0a8 MOV [EDX+0x78],EAX` |
| 0x51E0C0 | CServerAIMaster::RemoveObject(obj) [C] | high | list erase, obj+0x78 = -1 | `0051e10d MOV dword ptr [ECX + 0x78],0xffffffff` |
| 0x51E120 | CServerAIMaster::SetAILevel(obj, level) [C] | high | no-op if equal, rejects levels outside 0..4, else AddObject | `0051e135 CMP EDX,[EBP+0xc] / JNZ` ; `0051e15c CALL 0x0051e040` |
| 0x51ED10 / 0x51ED80 | CServerAIMaster::EventPending / GetPendingEvent [C] | high | head of event list (+0x54) due when head <= (day,time) | `0051ed1c ADD ECX,0x54 / CALL 0x007a1780`; `0051ed66 TEST EAX,EAX / JG` |
| 0x51E170 / 0x51E220 | AddEventDeltaTime / AddEventAbsoluteTime (mapper) | med | post event (24 bytes of args) | AI0 row; callers 0x5C87E0 etc. (FM5) |
| 0x51C280 | CServerExoApp::GetAIMaster | med | returns the AI master (`PUSH level; PUSH obj; CALL 0x51C280; MOV ECX,EAX; CALL 0x51E120` idiom) | e.g. `00569465`, `005694d3..d6` |
| 0x521E60 | CSWSArea::AIUpdate [C] | med | area tick: ambient on/off roll every 300000 ms, 6000 ms OnHeartbeat (RunScript area+0x168) | `*(undefined4 *)(param_1 + 0x238) = 300000;` ; `if (local_14 < 6000)` ; `FUN_006fd8d0(param_1 + 0x168,...)` |
| 0x5573E0 | CSWSModule::AIUpdate [C] | med | alternates per tick: area AIUpdate (0x521E60) / module heartbeat (0x557260) | `if (*(int *)(param_1 + 0x44) == 0) {... FUN_00521e60 ...} else { *(...+0x44)=0; FUN_00557260();` |
| 0x569320 | CSWSCreature::AIUpdate [C] | high | per-creature tick (map in Q6) | `0051dc7b CALL EAX` after `MOV EAX,[EDX+0x70]` |
| 0x5F4C00 / 0x600F20 / 0x62C360 | CSWSPlaceable / CSWSItem / CSWSAreaOfEffectObject::AIUpdate | med | other vtable+0x70 users; RunActions callers: 0x5F4C00, 0x617980, 0x61E360, 0x623330, 0x62C360 (cg callers 5d56b0) | `site 005f4e3e / 00617b24 / 0061e537 / 00623670 / 0062c622` |
| 0x5D56B0 | CSWSObject::RunActions(day,time,qpcLo,qpcHi) [C] | high | pop-dispatch loop (Q1) | `005d5c28 JMP dword ptr [EDX*0x4 + 0x5d7038]` |
| 0x5D7160 | ActionTypeStopsLoop(type) [C] | high | 1 for types 1, 5, 0x30, 0x33 | `005d7167 CMP [EBP+8],0x5 / 005d716d ...0x30 / 005d7173 ...0x1 / 005d7179 ...0x33` |
| 0x53F7F0 / 0x540CA0 | CSWSObject::AddAction / AddActionToFront [C] | high | `new(0x74)` node + <=13 params; tail (0x54AA30) / head (0x54AA10) insert; ret 112 | `0053f818 PUSH 0x74 / CALL 0x919723` |
| 0x53E6F0 / 0x53FA80 / 0x53F7A0 / 0x53E740 | ActionNode ctor / SetParam(i,kind,ptr) / NextGroupId / FreeParams [C] | high | node init, param store, group counter, free kind 4 strings and kind 5 script ptrs | `*(short *)(param_1 + 0x6e) = (short)param_2 + 1;` ; `*(short *)(param_1 + 0x14) = *(short *)(param_1 + 0x14) + 1;` |
| 0x53FC10 | TestActionList (no-op loop) [C] | med | counts list nodes, result unused | `local_8 = local_8 + 1; local_c = FUN_0058c370(&local_10);` |
| 0x541080 | CSWSObject::ClearAllActions(flag) [C] | high | gated by vtbl+0x10; `[+0xEC]==1`: for each node `vtbl+0x74(group,isHead)`, delete if returns 1; resets +0x11B0=1, +0x54C, pathinfo +0x254..+0x260; party slot -1 | quoted in R1 FM6; `*(undefined4 *)(iVar1 + 0x11b0) = 1;` |
| 0x5210F0 / 0x521060 | List::Count / List::HeadData [C] | high | `*(*list + 8)` / `*(*(int*)*list + 8)` | decompiles |
| 0x5233A0 / 0x523320 | CSWSArea::IncreaseAILevelPriority / IncrementPlayersInArea [C] | high | players-in-area (+0x12C) 0 -> 1: promote level-0 objects to 1 | `*(int *)(param_1 + 300) = ... + 1` ; `FUN_0051e120(piVar1,1)` |
| 0x5235A0 / 0x523360 | CSWSArea::DecreaseAILevelPriority / DecrementPlayersInArea [C] | high | count -> 0: creatures at level 1 -> 0 | `if (... (iVar2 = vtbl+0x30) != 0 && local_18[0x1e] == 1) FUN_0051e120(local_18,0)` |
| 0x66B640 | ExecuteCommandSetCreatureAILevel(obj, 0..4) | med | script: saves old level in obj+0x7C, maps 0,1,2,3,4 via jump table 0x66B758 (>4 -> 3) | `0066b6e9 MOV [EAX+0x7c],CL` ; table 0x66b758 = {0x66b702,0x66b70b,0x66b714,0x66b726,0x66b71d} |
| 0x66B800 | ExecuteCommandResetCreatureAILevel | med | script: SetAILevel(obj, byte [obj+0x7C]) | `0066b86f MOV DL,[ECX+0x7c]` ; `0066b893 CALL 0x0051e120` |
| 0x66AD10 | ExecuteCommandActionStartConversation (mapper) | med | if level < 3: saves level to +0x7C, level 3 | `0066b531 CMP ECX,0x3 / JGE`, `0066b548 MOV [EAX+0x7c],CL`, `0066b56d CALL 0x51e120` |
| 0x58A330 / 0x58A470 | SetPartyMemberFlag(flag,arg2) / SetIsPuppet | high | flag -> level 4, clear -> level 2 (only when arg2 != 0 / always for puppet) | `0058a35e MOV [EAX+0x11ac],ECX`; `0058a3f6`, `0058a458`, `0058a4b6`, `0058a4e7` |
| 0x63C1E0 | per-player object visibility update (writer of obj+0x220) | med | `+0x220 = 0x63DD30(obj, player) \|\| [+0x260]!=0` | `0063c257 MOV dword ptr [EDX + 0x220],EAX` |
| 0x63DD30 | IsObjectInPlayerUpdateRange(obj, player) | med | distance^2 vs perception range, LOS/seen/heard list; party (`+0x11AC`) and the player itself -> 1 | `if ((param_1[1] == param_2[1]) \|\| (... *(int *)(iVar1 + 0x11ac) != 0)) { local_8 = 1;` |
| 0x5C3F70 | CSWSCreature::WalkUpdateLocation_QuickWalk (mapper, 8036 B) | med | also called by the pre-pass; zeroes obj+0xC0 at 0x5C454B/0x5C5E42 | `0051d9bc CALL 0x005c3f70` |
| 0x5FFD60 | CSWSItem::Constructor | med | AddObject(item, 0): items live in the AI lists | `005ffe42 PUSH 0x0 ... 005ffe52 CALL 0x0051e040`; dtor 0x6000E0 calls RemoveObject |

### Action handler table (RunActions switch; type = node+0)
Table `0x5D7038`, entry = type-1, target in RunActions. "c" = guarded by `local_14 == 5` (object type byte [obj+8] == 5, creature), otherwise handler runs for any object class; a failed guard leaves result 3 (node silently deleted). Names are mapper/DB names, proposed names for unnamed ones are SPECULATIVE.
| type | handler | scope | name |
|---|---|---|---|
| 0x01 | 0x5BFC00 | c | AIActionMoveToPoint (stop-loop type) |
| 0x02 | 0x5BBF40 | c | AIActionCheckMoveToObject |
| 0x03 | 0x5BB5A0 | c | AIActionCheckMoveToPointRadius |
| 0x04 | 0x5BBB70 | c | AIActionChangeFacingPoint |
| 0x05 | 0x5BE950 | c | AIActionJumpToPoint (stop-loop type) |
| 0x06 | 0x5D9DA0 | any | AIActionPlayAnimation |
| 0x07 | 0x5CF8A0 | c | AIActionPickUpItem |
| 0x08 | 0x5BE0A0 | c | AIActionEquipItem |
| 0x09 | 0x5CFF50 | c | AIActionDropItem |
| 0x0A | 0x5BC700 | c | AIActionCheckMoveToPoint |
| 0x0B | 0x5D0A80 | c | AIActionUnequipItem |
| 0x0C | 0x6D7390 | c | AIActionPhysicalAttacks (RunActions sets obj+0x88 = node param0 value around the call: `005d633f MOV [EAX+0x88],ECX` ... `005d636c MOV [ECX+0x88],0x7f000000`) |
| 0x0D | none | - | default (deleted) |
| 0x0E | 0x5D9890 | any | AIActionSpeak |
| 0x0F | 0x5B96F0 (creature) / 0x5F6180 (placeable, type byte 9) | c/9 | AIActionCastSpell (creature version then `0x590180` if result 2 or 3) |
| 0x10 | 0x5D0C00 | c | unnamed (174 B, calls 0x590180) |
| 0x11 | 0x5BC1D0 | c | AIActionCheckMoveToObjectRadius |
| 0x12 | 0x5BCA60 | c | AIActionCheckMoveToPoint_2 |
| 0x13 | 0x5BB2C0 | c | AIActionChangeFacingObject |
| 0x14 / 0x15 | 0x5DB0A0 / 0x5DAD10 | any | AIActionOpenDoor / CloseDoor |
| 0x16 | 0x5CF7B0 | c | AIActionOrientCamera |
| 0x17 | 0x5D9BF0 | any | AIActionPlaySound |
| 0x18 | 0x5D75B0 | any | AIActionDialogObject |
| 0x19..0x1D | 0x5B43C0, 0x5B3380, 0x5B2C10, 0x5B5360, 0x5B5AD0 | c | DisarmMine, RecoverMine, FlagMine, ExamineMine, SetMine |
| 0x1E | 0x5D9AA0 | any | AIActionWait (R1 verified) |
| 0x1F | 0x5D9570 | any | unnamed (274 B, no callees: dead/zero-HP check, vtbl+0xC0, returns 3 or 2) |
| 0x20 | 0x5D9690 | any | unnamed (320 B, calls 0x563D20/0x563DA0) |
| 0x21 / 0x22 / 0x23 | 0x5D97E0 / 0x5DA4D0 / 0x5DAB80 | any/any/c | SpeakStrRef, GiveItem, TakeItem |
| 0x24 | 0x5BD0B0 | c | AIActionEncounterCreatureDestroySelf |
| 0x25..0x28 | 0x5D99D0, 0x5DB8F0, 0x5DDA60, 0x5DFBE0 | any | DoCommand, UnlockObject, LockObject, UseObject |
| 0x29 / 0x2B / 0x36 | 0x5D1990 | c | constant stub `return DAT_009944bc` (3) |
| 0x2A | 0x5D17D0 | c | constant stub `return DAT_009944b8` (2) |
| 0x2C | 0x5BB920 | c | AIActionCheckInterAreaPathfinding |
| 0x2D | 0x5BF000 | c | AIActionRandomWalk |
| 0x2E | 0x5B8000 | c | AIActionItemCastSpell |
| 0x2F | 0x5E0E70 | any | AIActionSetCommandable |
| 0x30 | 0x5BE1F0 | c | AIActionJumpToObject (stop-loop type) |
| 0x31 | 0x5BB470 | c | AIActionCheckMoveAwayFromLocation |
| 0x32 | 0x5D25E0 | c | AIActionCounterSpell |
| 0x33 | 0x5CCC50 | c | AIActionDrive (stop-loop type) |
| 0x34 / 0x35 | 0x5D17F0 / 0x5D18C0 | c | 2000 ms timed animation actions (anim 0x274F / 0x2711), return 1 until 2000 ms, then 2 or 3 (unnamed) |
| 0x37 / 0x3A | 0x5CEED0 / 0x5CF660 | c | AIActionForceFollowObject / CheckForceFollowObject |
| 0x38 | 0x5D19B0 | c | 3102 B, strings "Subs_Save"/"Save_DC", creates effects: mapper says AIActionHeal (low) |
| 0x39, 0x3B, 0x42 | none | - | default (deleted) |
| 0x3C | 0x5BEFC0 | c | unnamed (64 B, calls GetArea 0x5453C0) |
| 0x3D | 0x5BF7F0 | c + `[+0x11AC]!=0` | AIActionFollowLeader; if party leader id (0x73F470) == 0x7F000000 the node returns 1 (stays) |
| 0x3E | 0x5D7480 | any | AIActionBarkString |
| 0x3F | 0x6D9500 | c | AIActionCombat |
| 0x40 | 0x5BCDC0 | c + `[+0x11AC]!=0` | AIActionCheckMoveToFollowRadius |
| 0x41 | 0x5D29B0 | c | unnamed (55 B, calls 0x5D51A0) |
| 0x43 | 0x5DEC20 | any | AIActionDoorSaber |
| 0x44 | 0x5D29F0 | c | AIActionStanceTotalDefense |
| 0x45 | 0x5D2EA0 | c | unnamed (216 B, calls 0x57A990, count 0x5210F0) |
| 0x46 | 0x5BF940 | c + `[creature+0x11FC]!=0` | AIActionFollowOwner (puppet flag, written by SetIsPuppet 0x58A47F) |
| 0x47 | 0x5DF920 | any | unnamed: ClearAllActions(1), re-equip weapon slots 0x10/0x20/0x40000/0x80000, re-queue attack at round target `[round+0xB20]` if `[+0x520]!=0 && [+0x11E8]==1`; returns 2 |

## (b) Findings

### Q1. Action table and node layout. CONFIRMED
- Dispatch: `005d5c0c SUB ECX,0x1` / `005d5c15 CMP [EBP+0xfffffe4c],0x46` / `005d5c1c JA 0x005d688e` / `005d5c28 JMP dword ptr [EDX*0x4 + 0x5d7038]`. Table bytes (vanilla exe, PE read) `0x5d7038: 2f5c5d00 4d5f5d00 45605d00 9d605d00 ...` = type1 -> 0x5D5C2F, type2 -> 0x5D5F4D, type3 -> 0x5D6045, type4 -> 0x5D609D. No byte-index table: a flat dword table of 0x47 entries (type = index+1).
- Targets 0x5D688E (common tail) for types 0xD, 0x39, 0x3B, 0x42 and for `type-1 > 0x46`. The result variable is `[EBP-0x1C]`, reset every iteration to 3: `005d5bf4 MOV EDX,[0x009944bc] / 005d5bfa MOV [EBP-0x1c],EDX`. So a node without a handler (or whose creature guard fails) is deleted as "terminal 3". Constants (PE read): 0x9944B4=1, B8=2, BC=3, C0=4, 0x9944C4=1000.
- Loop (RunActions): pop head `005d5b86 CALL 0x007f2a90`; current node `[this+0x20C]` (`005d5bd3`), current type word `[this+0x84]` (`005d5bed`), action-start stamp `[this+0xC0],[this+0xC4]` set only if both zero (`005d5b94 CMP [ECX+0xc0],0 ... 005d5bb5 MOV [EAX+0xC0],ECX`); after the handler `[this+0x20C]=0` (`005d69ea`), master `[+0x7C]` = last type (`005d6a2d MOV [EAX+0x7c],ECX`), `[this+0x84]=0xFFFF` (`005d6a3b`), `[this+0xAC],[this+0xB0]` = (day,time) args (`005d6a4b`, `005d6a5a`).
- Result handling: `005d6a63 CMP EDX,[0x009944c0] / JNZ` -> 4: `CALL 0x54AA30` (list AddTail: node goes to the BACK); `005d6a88 CMP ECX,[0x009944b4]` -> 1: `CALL 0x54AA10` (AddHead: node goes back to the FRONT); anything else: if `vtbl+0xC4() != 0x7f000000 && type != 0xC` then `vtbl+0xC0(0x7f000000)`, free params (0x53E740) and the node (0x9196FD), `[+0xC0]=[+0xC4]=0` (`005d6b50`, `005d6b60`).
- Loop continuation (`005d6cee..005d6d82`): stop when result == 1; else call QPC (0x736220) and stop if `now - stamp > [0x9944C4]` (1000 us, `005d6d07 SUB EAX,[EBP+0x10] / SBB EDX,[EBP+0x14]`, `005d6d0d MOV ECX,[0x009944c4]`) or `ActionTypeStopsLoop(type) == 1` (`005d6d55 CALL 0x005d7160 / CMP EAX,1`); also stop when the queue is empty (`005d6d72 CALL 0x007a1780`). Consequence (CONFIRMED): a finished MoveToPoint/JumpToPoint/JumpToObject/Drive always ends the tick's action loop, so the next queued action runs one AIUpdate later (relevant to FM2b/E4 timing).
- Exit path (`005d6d87..`): queue length via 0x5210F0 `*(*list + 8)`; `> 0x4B (75)` logs code 0xA1; `> 0x1F4 (500)` logs 0xA4 and pops head nodes while `vtbl+0x74(node,1) == 1` (stops at the first node it cannot clear, re-adds it at the head with 0x54AA10); `> 1000` deletes every node. Effect count `this+0x14C > 500` logs 0xA2, `> 1000` calls 0x544A10 (EFFOVF). So the queue is capped at 1000 but never time-limited (no stuck detector).
- Empty-queue branch (`005d5a53 CALL 0x007a1780 / JNZ 0x005d5b57` and 0x5D6B6F): when the queue is empty the creature's `+0x550,+0x548,+0x568,+0x56C,+0x54C` are reset to 0x7F000000 and pathinfo `+0x254=0x7F000000`, `+0x258,+0x25C,+0x260 = 0` (R1 FM1 context).
- Node layout 0x74 B (field table in (e)). Creation `0053f818 PUSH 0x74 / CALL 0x919723`; `0053f82c CMP [EBP-0x18],0 / JZ 0x53f83f` then the NULL branch stores `[EBP-0x24]=0` and `0053f85f MOV [EDX],EAX` dereferences it: **no NULL check on the node allocation** (SPECULATIVE reach: operator new failure only; same in 0x540CA0).
- Params: kinds 1 (int), 2 (float), 3 (object id) copy `*param_4`; kind 4 allocates an 8-byte CExoString (`PUSH 8` in 0x53FA80) and copies; kind 5 stores the pointer, freed through the VM (0x6FDBB0) only when `DAT_00a1b4a8 != 0` (0x53E740).
- Group ids: `[obj+0x14]` (word, wraps 0xFFFF -> 0) increments per `-1` request, `[obj+0x16]` holds the last issued id (`-2` re-uses it); node `+0x6C` word. No de-dup in AddAction (R1: confirmed, no search).

### Q2. AI-level policy. CONFIRMED
Level stored in `obj+0x78` (dword, -1 = not in any list: CSWSObject ctor `0053eb61 MOV [EAX+0x78],0xffffffff`). SetAILevel 0x51E120 args = (obj, level), level must be 0..4. List order in UpdateState is 4,3,2,1,0 (`0051d7ad MOV [EBP-0x34],4`, `0051dd96 SUB EDX,1`).
| caller site | condition | new level |
|---|---|---|
| ctors via AddObject 0x51E040: creature 0x5622BB, placeable 0x5EEE6C, item 0x5FFE52, 0x616C37, 0x61DDE8, 0x622D3A, 0x62AC26, AoE 0x62C13B | always, `PUSH 0` | 0 |
| 0x5691F7 (CSWSCreature::AddToArea 0x568BD0) | level==0 (`005691c4`) and `area+0x12C > 0` | 1 |
| 0x56923E (same) | level==1 and `area+0x12C == 0` | 0 |
| 0x6286A5 / 0x6286CD (0x627FA0, object add-to-area analog) | `area+0x12C > 0` / else | 1 / 0 |
| 0x5234C1, 0x523546, 0x523589 (IncreaseAILevelPriority, on first player entering: 0x522D26 via AddObjectToArea when `0x51C600(id)` is a player, and 0x632A03) | per object with level 0 (`local_18[0x1e] == 0`), type > 4: creature/vtbl+0x50/+0x40 objects always; others only if script name (+0x290) non-empty and `[+0x410]==0` or `[+0x3F0]==0` | 1 |
| 0x523693 (DecreaseAILevelPriority, players -> 0: 0x523360 from 0x52319C / 0x632A11) | creature with level == 1 | 0 |
| 0x5694D6 (AIUpdate) | `(stats[+0x1198]+0x6C == 1 \|\| [+0x11AC] != 0) && level != 4` | 4 |
| 0x56A0B5 (AIUpdate) | `[+0x520]==1` (in combat) and level in {0,1} | 2 |
| 0x56A0FE / 0x56A111 (AIUpdate) | `[+0x520]!=1`, level==2, area != NULL: `area+0x12C == 0` / else | 0 / 1 |
| 0x56C01C (SpawnInHeartbeatPerception tail) | `area+0x12C == 0` and level != 0 (any level incl. 4 and 2; the stats read at 0x56BFF5 is dead) | 0 (FM7) |
| 0x5767C9 (PostProcess 0x576750) | `stats+0x6C == 1` | 4 |
| 0x58A3F6 / 0x58A458 (SetPartyMemberFlag) | flag!=0 and arg2!=0, level != 4 / flag==0, arg2!=0, level != 2 | 4 / **2** |
| 0x58A4B6 / 0x58A4E7 (SetIsPuppet) | flag!=0 / flag==0 | 4 / 2 |
| 0x5FAD3D (0x5FAAF0, party NPC object creation) | `PUSH 3` | 3 |
| 0x66B56D (StartConversation 0x66AD10) | old level < 3: saves level in `[obj+0x7C]` | 3 |
| 0x66B74B (script SetCreatureAILevel 0x66B640) | script arg 0,1,2,3,4 -> 0,1,2,3,4 (>4 -> 3), always saves old level in +0x7C | 0..4 |
| 0x66B893 (script ResetCreatureAILevel 0x66B800) | restores byte `[obj+0x7C]` | saved |
| 0x65D189 (script command 0x65BE70, 9928 B) | level == 0 | 1 |
Notes: level 3 is only set by scripts/conversation/NPC creation and is never demoted automatically (the combat code only promotes from 0/1 and demotes from 2). Conversation end: no engine caller of ResetCreatureAILevel was found (it is a script command only; open question). A dismissed party member lands in level 2 and is demoted to 0/1 by its own next AIUpdate (0x56A0FE/0x56A111).

### Q3. FM3 sentinel path. CONFIRMED (mechanism), reach LOW (L4) / LOW-MEDIUM (L2), SPECULATIVE (magnitude)
Quotes: sentinel set `0051dbe1 MOV ECX,[EBP-0x28]` / `0051dbe4 MOV [EBP-0x3c],ECX` (only when `[EBP-0x38]==1`, test at `0051dbdb CMP [EBP-0x38],1 / 0051dbdf JNZ 0x0051dbe7`); dispatch guard `0051dc5c CMP [EBP-0x28],0 / JZ 0x51dc7f`, `0051dc65 CMP EAX,[EBP-0x3c] / JNZ 0x51dc70`, `0051dc6a CMP [EBP-0x38],1 / JNZ 0x51dc7f`; "done" only `0051dc7f MOV [EBP-0x40],1`; level end `0051dd33` (`JNZ 0x51dd33` at 0x51DCF3/0x51DCF9, `JA` at 0x51DD22); RemoveObject `0051e0c9`.
- **Why it works in vanilla:** the object being updated is always at `idx == cursor`. A pass ends when NextObject returns S again (not first iteration) or NULL.
- **What breaks it (all CONFIRMED by the code above):** (1) S's own AIUpdate changes S's level (0x5694D6, 0x56A0B5, 0x56A0FE, 0x56A111, 0x56C01C) -> RemoveObject erases S's id; `obj == S` can never match again; (2) S destroyed during the pass (any dtor -> 0x51E0C0, e.g. 0x5634C0 creature, 0x6000E0 item); (3) S unresolved: `GetNextObject` returns 0 on the FIRST iteration -> `MOV [EBP-0x40],1` immediately, **zero AIUpdates in that level this tick** (cursor still advanced, so the next tick starts one object later; stale ids cost a partial pass each time the cursor lands on them).
- **Effect when S is lost:** the do-loop continues until `elapsed > budget` (`0051dd1c JC 0x51d842` / `0051dd2d JBE 0x51d842`), re-running AIUpdates with tiny dt and re-running the O(n) pre-pass each iteration (AIQ). Cost <= the level slice (L4 6 ms; L2 <= 0.96 ms when L4 is saturated, up to ~5.8 ms when L4/L3 are idle), once, in that tick.
- **Always-true side effect (new, CONFIRMED):** every self-transition (`SetAILevel` from inside the object's own AIUpdate) removes index == cursor via DelIndex; the next `cursor+1` lands on the element that used to be at `cursor+2`, so exactly one following object is skipped for this tick (it is picked up in the next tick's sweep, no starvation). Also: an object demoted 2 -> 0/1 inside the L2 pass is appended to the L0/L1 list that is processed LATER in the same tick, so it can receive a second AIUpdate in one tick if that pass reaches it (structure CONFIRMED, frequency SPECULATIVE).
- **Reach:** L4 holds only party/PC/puppets (<=~4). Its only self-transition is FM7 (0x56C01C, `area+0x12C==0`, every 4 s per companion per R1) and 0x5694D6 re-promotions, so FM3 in L4 needs `area+0x12C==0` while a party member is the first object of the pass: rare. L2 (combat) is small (n2 ~ 3..10): every enemy leaving combat transitions inside its own update; P(S) ~ 1/k (k = objects visited in that pass), so a few LOST events per fight end are expected (SPECULATIVE; measure with R1's `FM3 LOST`). L1/L0 passes are budget-cut in dense areas (Q4), so losing S there costs nothing extra.
- **Hook windows** (H01-H03 in (f)): bytes verified, no branch target inside, nearest sibling 137 / 166 / 1084 bytes (perf-lab 0x51DC70/0x51DC86/0x51D790).

### Q4. AIQ cost model. CONFIRMED (structure), SPECULATIVE (constants)
- Structure: per level L (n_L = `[master+8+16L]`), the do-loop body is: pre-pass `for i in 0..n_L: GetObjectAt(i)` (0x51D88D CALL 0x51D270) -> if obj && `[obj+0x220]` && vtbl+0x30 (AsCreature) && (head action type in {1,0x3D,0x46} (`0051d900 CMP [ECX],1`, `0051d90c CMP [EDX],0x3d`, `0051d918 CMP [EAX],0x46`) or `[creature+0x11B0]` in {4,5,6} (`0051d946`, `0051d974`, `0051d9a2`)) -> `0051d9bc CALL 0x005c3f70(creature,0)`; `if result == DAT_009944b8 (2) -> [creature+0x11B0]=1` (`0051d9dc..0051d9e2`); then event drain; then one NextObject + AIUpdate. Back edge `0051dd1c JC 0x0051d842` re-enters the pre-pass. Loop count I_L = AIUpdates + 1 (the wrap detection iteration) when the pass completes, = AIUpdates when budget-cut.
- Cost per tick: `T = sum_L I_L * (n_L * c_get + m_L * c_qw + c_evt) + sum_L U_L * c_ai + sum_L n_L * c_get (client sync, unbudgeted)`. c_get = GetObjectAt: 0x51D270 (41 asm lines) -> 0x51C0A0 (22) -> 0x53DFB0 chained hash (56 lines): `local_18 = *(uint **)(*param_1 + (((int)param_2 >> 0x1f) * -0x1000 + (param_2 & 0xfff)) * 4)`, i.e. 4096 buckets x 2 halves, chain walk: ~120 instructions straight-line + cache-missing chain nodes (c_get ~ 40..150 ns, SPECULATIVE). m_L = number of visible (`+0x220`) creatures with a move head action or `+0x11B0` in {4,5,6}; c_qw = the 8 KB QuickWalk body.
- **Saturation:** with budget B_L, U_L ~ B_L / (n_L*c_get + m_L*c_qw + c_ai), so a level's per-object update period is `n_L * (n_L*c_get + ...) / B_L` ticks: quadratic in n_L. Examples (c_get = 60 ns, c_ai = 5 us, SPECULATIVE): n=100 -> iteration ~11 us, whole pass ~1.1 ms, period 1 tick; n=300 -> iteration ~23 us, pass 7 ms > B_1 (~5.8 ms) -> ~250 updates/tick region, period ~1.2 ticks; n=1000 -> iteration ~65 us, B_0 = 3 ms -> 46 updates/tick, each object every ~22 ticks (~0.36 s at 60 fps) and UpdateState burns the whole slice.
- **Population:** level 0 holds every object registered at level 0 and never promoted: all items (0x5FFD60), placeables/doors/triggers/AoE ctors (8 AddObject callers, list in Q2). `GetNextObject`/`GetObjectAt` accept any type byte > 4 (`*(byte *)(iVar1 + 8) <= DAT_00994473` rejects; 4 = area), so items are accepted. Level 0 gets the leftover budget, so the pass saturates the slice whenever `(n_0+1) * T_iter > B_0`.
- **Heavy call frequency:** the QuickWalk pre-pass call runs `I_L * m_L` times per tick. For 3 following companions (head action 0x3D) in L4: I_4 = 4 iterations, so ~12 QuickWalk calls/tick for 3 creatures (4x redundant) before RunActions runs the same creatures' move actions (and 0x5BFC00 calls QuickWalk itself again). Behaviour change from reducing the frequency is SPECULATIVE (QuickWalk zeroes `[creature+0xC0]` at 0x5C454B/0x5C5E42 and may be dt-dependent; not read).
- Minimal gate (H04): replace `0051d842 MOV [EBP-0x44],0` by `[EBP-0x44] = ([EBP-0x38]==1) ? 0 : 0x7FFFFFFF` (loop head compares `0051d86d CMP ECX,[EBP-0x84]` / `JGE 0x51d9f1` signed, so INT_MAX skips the body) = pre-pass once per level per tick. The framework runs stolen bytes after the handler, so this needs `skip_original_bytes = true` and the handler doing the store. SPECULATIVE on gameplay equivalence; the counters (c) quantify the gain first.

### Q5. Budget order and overrun. CONFIRMED
- Allocation: `0051d79f MOV [EBP-0x18],0x2710` (remaining R, 64-bit pair `[EBP-0x18]/[EBP-0x14]`); per level `0051d7d5 CMP [EBP-0x34],0 / JL 0x51dda1`; `0051d7df CMP [EBP-0x34],0 / JLE 0x51d809` (level 0: budget = R, `0051d809..0051d812`); else `PUSH 0x3c; CALL 0x91fc30 (_allmul)` then `PUSH 0x64; CALL 0x91d220 (_aulldiv)` -> `[EBP-0x20/-0x1C]` = R*60/100.
- Order 4,3,2,1,0. Fully-used worst case: L4 6000 us, L3 2400, L2 960, L1 384, L0 256. Typical (party cheap, no combat): L4 ~0.3 ms used, L3/L2 empty, L1 up to ~5.8 ms (60% of ~9.7), L0 = everything left. An empty level costs ~nothing (NextObject returns 0 when count==0: `if (param_1[1] == 0) iVar1 = 0`).
- Guaranteed minimum: the first iteration compares `elapsed = [EBP-0x30]-[EBP-0x10]` where both were initialised from the same QPC read (`0051d822 MOV [EBP-0x10],EAX`, `0051d82b MOV [EBP-0x30],ECX`), so elapsed = 0 and the first object is always updated even if the budget is 0 (`0051db81 CMP [EBP-0x40],0 ... 0051dba9 CMP EAX,[EBP-0x1c] / JA`). Therefore a non-empty level never starves completely.
- Overrun accounting: `0051dd4b..0051dd61 CMP elapsed,budget / JNC 0x51dd81`: if `elapsed >= budget` -> `0051dd84 SUB EAX,[EBP-0x20]` (R -= budget, the overrun is not charged); else `0051dd63..0051dd79` R -= elapsed. A single AIUpdate is never preempted; the check `elapsed <= budget` is only evaluated between objects (`0051dcfb..0051dd2d`).
- What an expensive object does: (1) its own level ends after it (objects after the cursor wait for the next tick; the cursor persists, so it is round-robin fair, no aging); (2) lower levels still get their 60% shares of R (computed from budget-capped usage), so an overrun inflates the tick beyond 10 ms rather than starving the lower levels; (3) it starves nobody at higher levels (they ran first). A slow object (>= 75,000 us) is logged: `0051dca4 CMP [EBP-0x70],0x124f8 / 0051dcb3 PUSH 0` -> `CALL 0x549f70(0xA3, obj, us, 75000)`; master `[+0x60]` string and `[+0x68..+0x7C]` are diagnostics reset to ""/0/-1 before every AIUpdate (`0051dc06..0051dc55`) and filled by RunActions (`[+0x7C]` = type).
- **New:** a pending event at the loop bottom ends the level pass: `0051dcec CALL 0x0051ed10 / 0051dcf1 TEST EAX,EAX / 0051dcf3 JNZ 0x0051dd33`. An event posted by an AIUpdate that is already due at the tick-start snapshot (`[EBP-4]`,`[EBP-0x24]` from 0x51AD20; ms resolution, or frozen game time) cuts the level pass for this tick. SPECULATIVE reach (needs delta 0 and the same millisecond, or paused time).
- Post-pass (`0051dda1..0051df9d`, C text of 0x51D790): for all five lists, every object with `[obj+0x220]` is checked with `0x54A480(bit 0/1/2)` dirty bits and pushes position/orientation/state to the client through `vtbl+0x134/+300/+0x130`; each entry again costs one `GetObjectAt`. O(N) per tick, outside the budget. Then `if (*param_1 != 0) { module = 0x52F4C0(); 0x5573E0(module) }` = module AIUpdate (alternating area AIUpdate / module heartbeat, each 6000 ms gated).

### Q6. CSWSCreature::AIUpdate branch map. CONFIRMED (this = `[EBP+0xfffffe6c]`; offsets are creature offsets)
Order in the asm (0x569320, 3850 B):
| # | address | condition | action |
|---|---|---|---|
| 1 | 0x569334 | `DAT_00a7e098 != 0` (no code writer found, debug flag) and `[+0x11AC] != 0` | flag = 0; `0x54A610(1)` SetCurrentHitPoints(1) (`0056936a`) |
| 2 | 0x56936f | `DAT_00a7e0d4 != 0` (debug) and `0x51C100(0x51C8F0())` resolves | debug distance to the player (0x4452B0/0x5142F0) |
| 3 | 0x569440 | always | `t0 = QPC (0x736220)` (RunActions stamp); master = 0x51C280 (`0056945a`); time getter 0x51C370 |
| 4 | **0x56948E..0x5694D6** | `(stats(+0x1198)->+0x6C == 1 \|\| +0x11AC != 0) && +0x78 != 4` (`0056948e CMP [EBP+0xffffff4c],1`, `005694a9 CMP [..],0`, `005694c1 CMP [..],4`) | `0x51E120(this,4)` (`005694ca PUSH 4 ... 005694d6 CALL`). Entry of the party/PC block is 0x5694B2 (JZ target of 0x569495) |
| 5 | 0x5694E3 | always | `0x56B8A0(1)` SpawnInHeartbeatPerception (AI3 covers) |
| 6 | 0x569525..0x56955E | always | timestamps: `0x51C350(id)`, `0x51AD20` (now), `0x51AF60(...)` -> `[+0xD0] = dt ms` (`param_1[0x34] = local_30`) |
| 7 | 0x569578, 0x569583, 0x569596, 0x5695B1 | always | `0x571030` UpdateCombatRoundTimer; `0x57DD90` UpdateExcitedStateTimer; `0x544AD0(day,time)` UpdateEffectList; **`0x5D56B0(day,time,qpc)` RunActions** (`005695b1 CALL`) then `[+0xAC],[+0xB0]` = (day,time) |
| 8 | 0x5695DA | area = `0x5453C0()` (area id [+0x90] -> GetGameObject -> vtbl+0x28); `local_8 != 0` | height `0x54B130(&pos,1,&ok,this+0x11C0)` (`00569632`); if `ok==0 && +0x11AC != 0 && id != 0x51C8F0()` and `0x73F750()+0xCC==0 && +0xD8==0`: party slot `0x7E6BE0`, follow point `slot+0x24+idx*0x88 (+0x18..+0x20)`, `0x54BDC0` ComputeSafeLocation, re-height (`005697f5`, `00569815`); `+0x11B0 in {4,5}` -> z += `[+0x11B8]` (`0056982f`, `0056984a`); `0x543F50(&pos,1,0,0)` SetPosition (`00569881`) |
| 9 | 0x56988C, 0x56989D | always; area != NULL | `0x5718A0` ComputeAIState; `0x56C310` UpdateMineCheck (time-gated 3000/100 ms) |
| 10 | 0x5698B4 | `[+0x520] == 1` (in combat) | `byte [+0x11E8]==1` -> `0x563DA0(1,0,0)` SetActivity; `[+0x524] <= 0` (`005698f6..005698fd JG`) -> `0x57A560(0,[+0x11E8],0)` (`00569923`) combat exit; else if `[+0x528]==0` (`00569930`) `[+0x524] -= dt` |
| 11 | 0x569963 | `[+0x11D8] != 0` | countdown by dt, clamp 0 (`0056996a JBE`, `0056998c MOV [ECX+0x11d8],0`) |
| 12 | **0x5699CE..0x569A94** | `[+0x520] == 1` (`005699ce CMP [EBP+0xfffffee8],1 / JNZ 0x569aa0`) and `[+0x570] <= 0` (`005699e1 CMP [EDX+0x570],0 / JG 0x569a7c`) | if `vtbl+0x94() == 0` (alive): `RunScript(this+0x270+6*8 = +0x2A0, id, 1)` (`00569a67 CALL 0x6FD8D0`, E2 hook at 0x569A08); then `[+0x570] = 0xBB8` (`00569a72`); then `[+0x570] -= dt` (`00569a94`) |
| 13 | 0x569AA0 | alive (`vtbl+0x94()==0`) and party (`[+0x11AC]!=0`) and `vtbl+0x9C(0) > 0` (`SETLE`) | `0x56A230` HandleHPRegeneration, `0x56A800` HandleFPRegeneration (`00569b3a`, `00569b45`): **HP/FP regeneration runs only for party members** |
| 14 | 0x569B4A..0x569D5B | debug flags `DAT_00a7e088`, `a7e08c/a7e090` (no code writers) | debug text (0x734010, 0x73F430); leader id via `0x7E5DA0(0)` + `0x51C8B0` |
| 15 | 0x569D72 | `[+0x578] == 1` | `[+0x57C] <= 0` -> `0x5799E0(0)` SetAmbientAnimationState, `[+0x57C]=0`; else `-= dt` |
| 16 | 0x569DDF | `[+0xD8]` in {0x2714, 0x2767, 0x2766} | `[+0x574] <= 0`: if `[+0x578]==0` `0x40E770(0x7F000000)`; reload `[+0x574] = 0x5DC`; else `-= dt` |
| 17 | 0x569EA9 | `[+0x10E8] == 1` | every >= 300 ms (`CMP [EBP-0x78],0x12c`) `0x588ED0` UpdateActionQueue, stamps `[+0x3E0],[+0x3E4]` |
| 18 | 0x569F3F | always | 8 countdown timers `[+0x10BC + 4*i]`, `-= dt` clamp 0 |
| 19 | 0x569FBD | `DAT_00A0F5A0 == 1` (written at 0x6580DC) | `[+0x3D8] -= dt`, else reload 8000 |
| 20 | 0x56A028 | `[+0x384] == 1` | `0x6A7190()`; `[+0x384]=0` |
| 21 | **0x56A064..0x56A116** | `[+0x520]==1`: level in {1,0} -> `0x51E120(this,2)` (`0056a0b5`); else level==2 and area != NULL: `area+0x12C==0` -> 0 (`0056a0fe`) else 1 (`0056a111`) | combat-level maintenance (Q2) |
| 22 | 0x56A116 | area != NULL | `area+0x2F4==0` -> `[+0x11CC]=0`; else timer `[+0x11CC]` (-1 = frozen): when due `0x52DE40(max(area+0x2EC - area+0x2F0, 0))` = `[area+0x2EC] = min(arg, [area+0x2E8])` (stealth-XP style award, SPECULATIVE name) |
Notes: the A1 gate fields (+0x520/+0x524/+0x528/+0x570) are in blocks 10 and 12 and the order is block 10 (exit) before block 12 (fallback), as in R1. Blocks 1, 2, 14 are dead in retail unless a config writes the debug globals (no instruction writes them; they are read-only references).

### Extra findings (ranked)
1. **AIQ at level 0 with items (perf, CONFIRMED structure / SPECULATIVE magnitude):** see Q4. Verify with the live perf-lab AI line: mean us/tick close to 10,000 and `prepass_iter >> obj_updates` in item-rich areas.
2. **`+0x220` identity (CONFIRMED):** only writers are the ctor (`0053edc3 MOV [ECX+0x220],0`) and `0063c257 MOV [EDX+0x220],EAX` in 0x63C1E0 (callers 0x63C1B0/0x63C420 <- 0x63CAA0 <- 0x63CEC0 <- 0x537590, a time-boxed (2000) per-player object update). Contradicts U3 "party/PC-controlled flag"; party members are always 1 (0x63DD30), but other creatures in range are 1 too, so the pre-pass QuickWalk reaches visible enemies as well (head action type 1 or `+0x11B0` in {4,5,6}).
3. **Dismissed party members drop to level 2 (0x58A458), not 0/1; they are demoted by their own AIUpdate** (CONFIRMED).
4. **No stuck/timeout logic in the scheduler or RunActions (CONFIRMED):** RunActions re-queues a result-1 node at the head every tick; the 75/500/1000 queue guards are the only limits. `[+0xC0],[+0xC4]` (action start day/time) is set at pop and read by Wait/Dialog/Door/Use/Cast handlers (0x5D9BB1, 0x5D7615, 0x5DB77F, 0x5E00DD, 0x5BA6CC), NOT by AIActionMoveToPoint (0x5BFC00 has no `+0xC0` access in the census); QuickWalk zeroes it (0x5C454B, 0x5C5E42). Useful for E4: a script/engine stuck check can key on this stamp only if QuickWalk's reset is accounted for.
5. **ClearAllActions semantics (0x541080):** only runs if `vtbl+0x10()!=0`; with `[+0xEC]==1` it deletes nodes the object agrees to clear (`vtbl+0x74(group, group==head)==1`), always resets `[creature+0x11B0]=1`, `[+0x54C]=0x7F000000`, pathinfo `+0x254..0x260`, and the party follow slot `[slot+0x28] = -1` for party members (`0054137d`), the FM1 and FM6 context.
6. **Item AIUpdate exists (0x600F20):** items are ticked like everything else (395 B, timers); with n(L0) large each gets an update every n/U ticks, harmless for gameplay, expensive in the pre-pass.

## (c) Runtime probes
| probe | counter / hook | expected signal |
|---|---|---|
| P1 tick cost and level sizes | existing perf-lab `PerfLabAiTick` (0x51D790, 9 B `55 8B EC 81 EC F0 00 00 00`) plus H05 (0x51D815, once per level) logging `n_L = [master+8+16*[ebp-0x34]]`, budget `[ebp-0x20]`, level `[ebp-0x34]` | per-level n_L histogram; us/tick near 10,000 and n_0 in the hundreds confirms Extra 1 |
| P2 pre-pass iterations vs updates | H04 (0x51D842): `iter[L]++`; H02 (0x51DD33): `upd[L] = iter[L] - wrap`; ratio | `iter ~ upd + 1` when completing, `upd << n_L` in saturated levels |
| P3 QuickWalk redundancy | U3 H4 (0x51D9BC `E8 AF 65 0A 00`, relative CALL: needs a relocating stolen call): count calls per tick vs distinct creatures | ~4x for 3 followers in L4 |
| P4 FM3 | H01/H02/H03 as R1 (`FM3 LOST/REMOVE`); add `first==NULL` count at H01 (`[ebp-0x28]==0 && [ebp-0x38]==1`) | LOST count per fight end; NULL-first count = stale ids |
| P5 level transitions | H07 (0x51E151): log `[ebp+8]` obj, `[ebp-4]` old, `[ebp+0xC]` new, return address `[ebp+4]` | caller histogram vs the Q2 table; FM7 4->0 spikes |
| P6 action type distribution | H08 (0x5D5C22, EDX = type-1) and H09 (0x5D6A60, `[EBP-0x1C]` result, `[EBP-0x18]` type, `[EBP-0x40]` node): histogram type x result | long-lived result 1 per type (stuck candidates, E4); share of types with no handler |
| P7 RunActions loop-stop reason | H10 (0x5D6D07, EDX:EAX = now): `now - [ebp+0x10]` per action; count stops by (result==1, time, StopsLoop) | share of MoveToPoint ending the loop (FM2b) |
| P8 queue length | H11 (0x5D6D87): `[this+0x100]->hdr+8` histogram | > 75 events per creature; the A1 spin shows here |
| P9 client-sync cost | H06 (0x51DDA1) + level-4 end: QPC delta of the post-pass | O(N) us per tick, uncharged |

## (d) Open questions
1. Real n(L0)/n(L1) in play (items in inventories: are all party items real CSWSItem objects in the lists?). Needs P1.
2. QuickWalk 0x5C3F70: dt-dependence/idempotence, so the pre-pass gate (H04) can be judged. Not read (8 KB).
3. Handlers 0x10, 0x1F, 0x20, 0x34, 0x35, 0x38 (mapper says Heal), 0x3C, 0x41, 0x45, 0x47 are unnamed; the mapper rows for 0x5D17F0/0x5D18C0 are absent.
4. `vtbl+0x74` of CSWSObject (used by ClearAllActions and the 500-entry guard: `(**(code **)(*param_1 + 0x74))(node,1)`) and `vtbl+0xC0/+0xC4` (target getter/setter around type 0xC) are unidentified.
5. Which engine path ends a conversation's level 3 (no ResetCreatureAILevel caller found in code).
6. The class of the objects registered by 0x616A70, 0x61DCF0, 0x622C90, 0x62AB10 (door/trigger/waypoint/store/sound?) and which of them implement a non-trivial AIUpdate; name_map gives CSWSItem 0x600F20, CSWSPlaceable 0x5F4C00, AoE 0x62C360 (medium).
7. Debug globals DAT_00A7E098/E088/E08C/E090/E0D4 have no instruction writers (likely config-table pointers); confirm they are 0 in retail.
8. `0x5573E0`'s alternating tick: the module heartbeat 0x557260 timing was not traced.
9. Whether game time (0x51AD20) is QPC-based or frame-quantised decides the reach of the "pending event ends the level" cut (Q5 New).
10. Missing in decomp: none needed beyond the above (all seed functions present).

## (e) Struct field tables
| class | offset | size | type | proposed name | readers (addr) | writers (addr) | conf | evidence |
|---|---|---|---|---|---|---|---|---|
| CServerAIMaster | 0x00 | 4 | ptr | serverInternal | 0x51D7BA, 0x51DA48, 0x51ED3E | ctor (0x51D2F0, mapper; not read) | high | `0051d7bc MOV EDX,[ECX+0x10048]` |
| CServerAIMaster | 0x04+0x10*L | 4 | int* | list[L].ids | 0x51D1C0, 0x51D270, 0x51D170 | 0x83EA60 (add), 0x49B840 (del) | high | `LEA ECX,[ECX+EAX*1+4]` at 0x51D889 |
| CServerAIMaster | 0x08+0x10*L | 4 | int | list[L].count | 0x51D860, 0x51D1C0 | 0x49B840 `param_1[1]--`, add | high | `0051d860 MOV EAX,[EDX+ECX*1+0x8]` |
| CServerAIMaster | 0x0C+0x10*L | 4 | int | list[L].cursor | 0x51D1C0 | 0x51D1C0 (`param_1[3] + 1`, wrap to 0) | high | `0051d1dc MOV EDX,[ECX+0xC]` (R1 quote) |
| CServerAIMaster | 0x54 | 4 | list | eventQueue | 0x51ED1C, 0x51ED33 | 0x51E220 insert, 0x51ED80 pop | high | `0051ed1c ADD ECX,0x54 / CALL 0x7a1780` |
| CServerAIMaster | 0x58 | 4 | ptr | effectHandler | 0x51EFD0 (U3) | ctor | med | U3 quote |
| CServerAIMaster | 0x60 | 8 | CExoString | diagCurrentScript | 0x549F70 (log) | 0x51DC09 (reset to ""), RunActions | low | `0051dc06 ADD ECX,0x60 / CALL 0x7338D0` |
| CServerAIMaster | 0x68..0x78 | 5x4 | int | diag fields | log | 0x51DC14..0x51DC48 (=0) | low | `0051dc14 MOV [EAX+0x68],0` .. `0051dc48 MOV [ECX+0x78],0` |
| CServerAIMaster | 0x7C | 4 | int | lastActionType | 0x549F70 | 0x51DC55 (=-1), 0x5D6A2D (=type) | med | `0051dc55 MOV [EDX+0x7c],0xffffffff`, `005d6a2d MOV [EAX+0x7c],ECX` |
| CSWSObject | 0x04 | 4 | id | objectId | 0x51E080 | ctor | high | `0051e07d MOV ECX,[EAX+0x4]` |
| CSWSObject | 0x08 | 1 | byte | objectType (4 area, 3 module, 5 creature, 9 placeable) | 0x51D1C0, 0x51DA82, 0x5D56FB | ctor | high | `0051da82 MOVZX ECX,byte ptr [EAX+0x8]`, `005d56fb MOVZX EDX,byte ptr [ECX+0x8]` |
| CSWSObject | 0x14/0x16 | 2/2 | word | actionGroupCounter / currentGroup | 0x53F7A0, 0x53F7F0 | 0x53F7A0 | high | `*(short *)(param_1 + 0x14) = *(short *)(param_1 + 0x14) + 1;` |
| CSWSObject | 0x78 | 4 | int | aiLevel (-1 none) | 0x51E062, 0x51E0CC, 0x51E12C, 0x5233xx, 0x5694B8, 0x56A07F, 0x58A3CE | 0x51E0A8, 0x51E10D, ctor 0x53EB61 | high | `0051e0a8 MOV [EDX+0x78],EAX` |
| CSWSObject | 0x7C | 1 | byte | savedAiLevel | 0x66B86F | 0x66B6E9, 0x66B548 | med | `0066b6e9 MOV [EAX+0x7c],CL` |
| CSWSObject | 0x84 | 2 | word | currentActionType (0xFFFF none) | 0x5D6A17 | 0x5D5BED, 0x5D6A3B | high | `005d5bed MOV word ptr [EDX+0x84],CX` |
| CSWSObject | 0xAC/0xB0 | 4/4 | int | lastRunActions day/time | AIUpdate | 0x5D6A4B, 0x5D6A5A | med | `005d6a4b MOV [ECX+0xac],EDX` |
| CSWSObject | 0xC0/0xC4 | 4/4 | int | actionStart day/time | 0x5D9BB1 (Wait) etc. | 0x5D5BB5/0x5D5BC4 (set), 0x5D6B50/0x5D6B60, 0x56EEA6, 0x5C454B, 0x5C5E42 (clear) | high | `005d5bb5 MOV [EAX+0xC0],ECX` |
| CSWSObject | 0x100 | 4 | list | actionQueue | 0x5D5748+, 0x5210F0, 0x7A1780 | 0x54AA10/0x54AA30 | high | `ADD ECX,0x100 / CALL 0x007a1780` |
| CSWSObject | 0x14C | 4 | int | effectCount | 0x5D6D87 area | 0x544AD0 | med | `param_1[0x53]` > 500/1000 guard |
| CSWSObject | 0x20C | 4 | ptr | currentActionNode | 0x5D5BD3 | 0x5D5BD3, 0x5D69EA | high | `005d5bd3 MOV [ECX+0x20c],EDX` |
| CSWSObject | 0x220 | 4 | int | visibleToPlayer (update range) | 0x51D8A2, 0x51DE12, 0x5BFCBD, 0x5C3FA4 | 0x63C257 (value), 0x53EDC3 (0) | med | `0063c257 MOV [EDX+0x220],EAX` |
| CSWSObject | 0x224.. | 3x4 | bitset | dirtyFlags (bit i: word [+0x224+(i>>5)*4]) | 0x54A480 | 0x54A4D0 | med | `1 << (param_2 & 0x1f) & *(uint *)(param_1 + 0x224 + (param_2 >> 5) * 4)` |
| ActionNode | 0x00 | 4 | int | type | 0x5D5BDC, 0x51D900 | 0x53F85F, 0x540CA0, 0x581070 | high | `0053f85f MOV [EDX],EAX` |
| ActionNode | 0x04 + 4*i (i<13) | 4 | int | param[i].kind (1 int,2 float,3 object id,4 CExoString*,5 ptr) | 0x53E740 | 0x53E6F0 (0), 0x53FA80 | high | `*(int *)(param_1 + 4 + param_2 * 4) = param_3;` |
| ActionNode | 0x38 + 4*i | 4 | int | param[i].value | handlers | 0x53FA80 | high | `*(undefined4 *)(param_1 + 0x38 + param_2 * 4) = *param_4;` |
| ActionNode | 0x6C | 2 | word | groupId | 0x53F42E, 0x588F9A, 0x5D59AD | 0x53F882, 0x53F8A6 | high | `0053f882 MOV word ptr [ECX+0x6c],AX` |
| ActionNode | 0x6E | 2 | word | paramCount (= last index+1) | 0x53E740 | 0x53E6F0 (0), 0x53FA80 | high | `*(short *)(param_1 + 0x6e) = (short)param_2 + 1;` |
| ActionNode | 0x70 | 4 | int | flag (=1 at creation, no reader found) | none found | 0x53F864, 0x540D14, 0x5810ED | low | `0053f864 MOV [ECX+0x70],0x1` |
| CSWSCreature | 0x11AC | 4 | int | partyMemberFlag | 0x569497, 0x569AC4, 0x5D5xxx | 0x58A35E | high | `0058a35e MOV [EAX+0x11ac],ECX` |
| CSWSCreature | 0x11B0 | 4 | int | moveState (pre-pass reads 4,5,6; written 1) | 0x51D93A, 0x51D968, 0x51D996 | 0x51D9E2, 0x541xxx (ClearAllActions) | med | `0051d9e2 MOV [ECX+0x11b0],1` |
| CSWSCreature | 0x520/0x524/0x528/0x570 | 4 | int | inCombat / combatExitTimer / timerFrozen / fallbackTimer | 0x5698B4, 0x5698F6, 0x569930, 0x5699E1 | 0x57A560, 0x569A72 | high | R1 + `00569a72 MOV [ECX+0x570],0xbb8` |
| CSWSCreature | 0x10BC.. | 8x4 | int | countdown timers[8] | 0x569F4E | 0x569F6E.. | med | `CMP [ECX+EAX*4+0x10bc],0` |
| CSWSArea | 0x12C | 4 | int | playersInArea | 0x569D.. , 0x5691D0, 0x56A0DD, 0x523360 | 0x523320 (++), 0x523360 (--) | high | `*(int *)(param_1 + 300) = *(int *)(param_1 + 300) + 1;` |
| CSWSArea | 0x194/0x198 | 4/4 | id*/int | objectIds / count | 0x5233A0, 0x5235A0 | add/remove object | med | `for (; local_14 < *(int *)(param_1 + 0x198); ...)` |

## (f) Hook-site catalogue
Rule check done for every row: bytes copied from the asm, all offsets inside the window grepped with `grep -rEh "(J[A-Z]+|CALL|JMP) +0x<addr>\b" decomp/*/*.asm` (no hits for any interior address; start addresses that are branch targets are noted), no relative CALL/JMP/Jcc inside, nearest sibling = perf-lab / companion-ai / ranged-ai / self-patch list (distance in bytes between window edges).
| id | address | length | stolen bytes (hex) | convention / live regs | consumed exit? | EAX/stack notes | hot/cold | nearest sibling |
|---|---|---|---|---|---|---|---|---|
| AI4-H01 FM3 sentinel set | 0x51DBE1 | 6 | 8B 4D D8 89 4D C4 | ebp frame; obj = [ebp-0x28], level [ebp-0x34], first [ebp-0x38]==1 here | no | EAX=NextObject result, ECX overwritten; resume 0x51DBE7 (JNZ target) | cold (1/level/tick) | 137 (perf-lab 0x51DC70) |
| AI4-H02 FM3 level end | 0x51DD33 | 6 | 8B 4D D0 2B 4D F0 | ebp; sentinel [ebp-0x3C], done [ebp-0x40], now [ebp-0x30], start [ebp-0x10], budget [ebp-0x20] | no | stolen SUB sets CF used by `SBB EDX,..` at 0x51DD3C: stolen bytes must run last; 0x51DD33 is a branch target (3 sites) | cold (<=5/tick) | 166 (perf-lab 0x51DC86) |
| AI4-H03 FM3 remove | 0x51E0C9 | 6 | 8B 45 08 8B 48 78 | ebp; obj [ebp+8], level list base ECX=this later | no | EAX/ECX dead after window | cold | 1084 |
| AI4-H04 pre-pass gate / counter | 0x51D842 | 7 | C7 45 BC 00 00 00 00 | ebp; first [ebp-0x38], level [ebp-0x34]; target of JC 0x51DD1C and JBE 0x51DD2D, falls into `JMP 0x51D854` | counter: no; gate: yes (`skip_original_bytes`, handler stores [ebp-0x44]) | no register live; flags dead | HOT (n_L+1 per level per tick) | 169 (0x51D790) |
| AI4-H05 level start | 0x51D815 | 5 | A1 88 B4 A1 00 | ebp; level [ebp-0x34], R [ebp-0x18], budget [ebp-0x20]; list count = [master+8+16*level], master [ebp+0xffffff34] | no | EAX written by stolen MOV; 0x51D815 is a JMP target (0x51D807) | cold (5/tick) | 124 (0x51D790) |
| AI4-H06 client-sync start | 0x51DDA1 | 7 | C7 45 CC 04 00 00 00 | ebp; master [ebp+0xffffff34]; JL 0x51DDA1 target (0x51D7D9) | no | none live | cold (1/tick) | 276 |
| AI4-H07 level really changes | 0x51E151 | 7 | 8B 45 0C 50 8B 4D 08 | ebp; obj [ebp+8], new [ebp+0xC], old [ebp-4], this [ebp-8], ret [ebp+4]; JGE target (0x51E14B) | no | stolen PUSH EAX runs after the handler (stack balanced by the framework) | cold | 1220 |
| AI4-H08 RunActions dispatch | 0x5D5C22 | 6 | 8B 95 4C FE FF FF | ebp; type-1 = [ebp+0xfffffe4c] (EDX is loaded by the stolen MOV, not yet at handler time); this [ebp+0xfffffe58] | no | next instruction is the indirect JMP at 0x5D5C28 (not stolen) | HOT (per action) | 322,773 (companion 0x586F43) |
| AI4-H09 RunActions result | 0x5D6A60 | 9 | 8B 55 E4 3B 15 C0 44 99 00 | ebp; result [ebp-0x1C], type [ebp-0x18], node [ebp-0x40], this [ebp+0xfffffe58] | no | stolen CMP feeds `JNZ 0x5D6A85` (flags regenerated: stolen bytes last) | HOT (per action) | 326,419 |
| AI4-H10 RunActions time | 0x5D6D07 | 6 | 2B 45 10 1B 55 14 | EDX:EAX = QPC now (from `CALL 0x736220` at 0x5D6D02); stamp [ebp+0x10]/[ebp+0x14] | no | handler may read EAX/EDX; SUB/SBB recompute flags | HOT (per non-pending action) | 327,098 |
| AI4-H11 RunActions queue length | 0x5D6D87 | 12 | 8B 8D 58 FE FF FF 81 C1 00 01 00 00 | ebp; this [ebp+0xfffffe58]; JNZ target (0x5D5B5B) | no | ECX = queue list ptr after stolen bytes; next is `CALL 0x5210F0` | HOT (per creature per tick) | 327,226 |
| AI4-H12 force level 4 | 0x5694CA | 8 | 6A 04 8B 95 6C FE FF FF | ebp; this [ebp+0xfffffe6c]; master [ebp-0x10] | no | stolen PUSH 4 + MOV EDX; EAX/ECX/EDX dead before the CALL at 0x5694D6 | cold (transition) | 1334 (companion 0x569A08) |
| AI4-H13 fallback gate | 0x5699CE | 7 | 83 BD E8 FE FF FF 01 | ebp; [+0x520] copy at [ebp+0xfffffee8] | no | stolen CMP feeds `JNZ 0x569AA0` (0F 85 rel32 at 0x5699D5, not stolen): stolen bytes last | HOT (per creature tick) | 51 (companion E2 0x569A08) |
| AI4-H14 fallback reset | 0x569A72 | 10 | C7 81 70 05 00 00 B8 0B 00 00 | ebp; ECX = this (loaded at 0x569A6C) | no | ECX live (the MOV uses it); resume 0x569A7C (JG target) | cold (3 s) | 100 |
| AI4-H15 combat -> level 2 | 0x56A0A9 | 8 | 6A 02 8B 85 6C FE FF FF | ebp; this [ebp+0xfffffe6c]; JZ target (0x56A08F) | no | stolen PUSH 2 + MOV EAX | cold | 1691 |
Existing (informational, do not duplicate): 0x51D790 (9 B), 0x51DC70 (5 B, `8B 4D D8 8B 11`), 0x51DC86 (7 B, `C7 45 C8 00 00 00 00`) = perf-lab; 0x51DBE7 `8B 15 88 B4 A1 00` is the retired hot probe (never ship it together with H01). H01/H02 are 0x8F/0xAD from perf-lab 0x51DC70/0x51DC86. Sibling self-patch list (0x40D69E, 0x8ABA8C...) is > 322 KB away from every row.

## (g) Mod relevance
- Q1 action table / StopsLoop: **E4** (stuck move: no engine timeout, `+0xC0` stamp not read by MoveToPoint), **FM2b** (finished move ends the tick's loop), **A1** (loop continues for 2/3 within 1000 us), **party-flag (+0x11AC)** (types 0x3D/0x40 are creature+flag gated, regen only for party).
- Q2 level policy: **FM7** (0x56C01C drops any level to 0 when `area+0x12C==0`), **FM3** (self-transitions), **enemy-AI** (enemies run in L2 only while in combat), **perf**.
- Q3 FM3 / Q5 budget / Q4 AIQ: **FM3**, **AIQ**, **perf**.
- Pre-pass gate and `+0x220` correction: **AIQ**, **perf**, **FM1** (ClearAllActions resets `+0x11B0`).
- 75/500/1000 queue guards (Q1): **A1**, **EFFOVF** (effect guard at 0x5D6F5E..).
- Mod 8 (kite), E3, E7, E9, M5, M8, P1, P2, PFLK, medpac, C1: none from this batch.
