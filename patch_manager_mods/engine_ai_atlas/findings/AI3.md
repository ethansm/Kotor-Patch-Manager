# AI3 Perception, LOS and stealth (swkotor2.exe Steam 6a522e71, static, 2026-10-02)

## Summary (5 lines)
1. P1 CONFIRMED and sharper: the mode-1 stamp `+0x3A0/+0x3A4` has no creature-side writer except the ctor (`0x562366/0x562376`), and the stale (0,0) stamp makes `elapsed` = ms-of-game-day (day is ignored by `0x51AF60`), so `>=4000` fails only in the first 4 s of each game day; every non-party creature runs `UpdatePerception(1)` once per `AIUpdate` (once per scheduler pass). The shipped companion-ai hooks `0x56BDB4/0x56BDE3` supply exactly the missing store.
2. NEW: DoSpotDetection adds the distance modifier with the wrong sign (`0x579009 NEG EAX; 0x57900F SUB ECX,[EBP-0x10]`; the combat-log copy is `-n`, the applied value is `+n`): beyond 6 m stealth is EASIER to see through; listen subtracts (`0x5787B9 SUB EDX,EAX`). Also `ModSpotCheck` (area `+0x18C`) is loaded but never read by the spot check.
3. E3 CONFIRMED (static): `ClearLineOfSight 0x52BB60` tests only door AABBs (+`BlockSight` 2DA) and room-mesh rays; no creature is ever enumerated, so allies/enemies never block LOS. Doors and the two endpoint objects are the only exclusion mechanism.
4. Stealth is a cached-dice roll, not a per-tick roll: target stealth dice `+0x1024/+0x1025` (11..20) and perceiver spot/listen dice `+0x1026/+0x1027` (1..20 in detect mode, else 1..10) are re-rolled only when `>=20000` ms passed (`0x576ED7 CMP [EBP-0x84],0x4E20`); an already-SEEN entry is never re-rolled while LOS holds, so a failed roll stays failed for up to 20 s.
5. R1 sharpened: the `0x522AF0` abort only skips the new-object scan of modes 0/2 (companion refresh, enemy-vs-non-party); mode 1 (enemy notices party, the aggro path) never calls it. In combat every non-controlled creature also re-reads 2DA row 0x12 `PrimaryRange/SecondaryRange` (CExoString + `_atof`/`sscanf`) at least ~6 times per (enemy, party member) pair per tick (`0x58B140/0x58B210`).

Conventions: `ROOT/decomp/<bb>/<addr8>.asm|.c`; "this" = ECX; `[EBP+0xffffff2c]` is the Ghidra spelling of `[EBP-0xD4]`. Constants decoded from `backups/vanilla_clean/swkotor2.exe` (sha256 prefix 6a522e71, same build; NOT the game folder) with a PE-section reader.

---

## (a) Function table
CONF = confirmed against decompile+asm (ret N, callees, strings) in this batch; MAP = taken from a name_map/mapper row only.

| addr | proposed name | conf | purpose | evidence |
|---|---|---|---|---|
| 0x56B8A0 | CSWSCreature::SpawnInHeartbeatPerception (ret 4, arg = mode 0 spawn-in / 1 tick) | high CONF | 3 timed blocks: spawn script + heartbeat (slot 0/8), mode-1 party perception, mode 0/2 perception + AI-level drop | `0056bdc5 CALL 0x005d0f30` (push 1), `0056bf79 CALL 0x005d0f30` (push 0), `0056bf88 CALL 0x005d0f30` (push 2) |
| 0x569320 | CSWSCreature::AIUpdate | high CONF (this call only) | calls 0x56B8A0(1) once per AIUpdate | `005694e3 CALL 0x0056b8a0` (after SetAILevel(4) test) |
| 0x56EFD0 | CSWSCreature::EventHandler | high CONF (this path only) | first call -> 0x56B8A0(0); event 10 payload type 1 = ON_PERCEPTION, runs script slot 1 (`+0x278`) | `0056f01a PUSH 0 / 0056f022 CALL 0x0056b8a0`; `00570392 CMP ECX,[0x0099241c]` |
| 0x5D0F30 | CSWSObject::UpdatePerception(mode) (ret 4) | high CONF | mode 0 all, 1 party, 2 non-party; refresh list + discover by sorted-X scan | `005d1287 CALL 0x00522af0`, `005d13e2 MOV [EDX+0x11c8],0` |
| 0x576DC0 | CSWSCreature::DoPerceptionUpdateOnCreature(this=perceiver P, arg=target T) | high CONF | dice reroll, LOS, seen/heard decision, list update, event posts | `005771fd CALL 0x0052bb60`, six `CALL 0x0051e170` posts |
| 0x577ED0 | DoSeenHeardChecks (DB: DoStealthDetection; ret 20) | med CONF | seen = spot only if LOS; heard = listen regardless | `FUN_00577ed0: iVar1 = FUN_00577fa0(...); if (param_2 != 0) local_c = FUN_00578850(...)` |
| 0x577FA0 | CSWSCreature::DoListenDetection (ret 8: T, hiddenFlag) | high CONF | hearing roll | `0057883f MOV [EBP-0x30],0` result store |
| 0x578850 | CSWSCreature::DoSpotDetection (ret 8) | high CONF | seeing roll | `00579021 MOV [EBP-0x30],0` |
| 0x585420 | IsBlindOrDark (DB name BroadcastAttackOfOpportunity is wrong) | med | `+0xFF4 & 0x10` or (`&8` and not `&2`) | `FUN_00585420` body, 17 lines |
| 0x585480 | CSWSCreature::GetInvisible (this=T, arg=P) | med CONF | scans T's effect list for invisibility (type 47) / sanctuary-style (type 63, Will save, 2-bit cache) | `FUN_00585480`: `(*(byte*)(iVar6+4) >> 2 & 3)`, `FUN_006e4630(3,...)` |
| 0x586640 | DropTargetingOfThis (low) | low | if P is targeted by T: SetCombatState(0,1,0) + ClearHostileActionsVersus | `FUN_00586640`: `FUN_0057a560(0,1,0); ... FUN_005867e0(param_1)` |
| 0x5867E0 | ClearHostileActionsVersus (MAP) | med | called when T becomes hidden to P | `005773ae CALL 0x005867e0` |
| 0x5802C0 | GetDetectModeState (DB name UpdateAppearanceForEquippedItems_2 is wrong) | med | `+0x510` (detect mode) unless `+0xD8==10000` (then 2) | `if ((*(int*)(param_1+0xd8)==10000) && (*(char*)(param_1+0x510)!='\x01')) uVar1=2` |
| 0x58B140 | GetSpotCheckDistance (PrimaryRange) | high CONF | `+0x101C`, or 2DA row 0x12 when `+0x520!=0` and not the controlled creature | `0058b16b FLD [EAX+0x101c]`, `0058b1e0 CALL 0x0071b670` (push 0x12, "PrimaryRange" 0x995574) |
| 0x58B210 | GetListenCheckDistance (SecondaryRange) | high CONF | `+0x1020`, same override | `FUN_0058b210`: `"SecondaryRange"` |
| 0x5D1400 | AddPerceptionEntry(id,seen,heard,invisCache,hidden) | high CONF | `new(8)` entry, packs bits, ECX=list append `0x83EA60` | `FUN_005d1400` body |
| 0x5D15F0 | RemovePerceptionEntry(id) | high CONF | reverse scan, delete matching | body |
| 0x5D1510 | ClearPerceptionList (keeps self) | high CONF | frees all non-self entries | body |
| 0x5D1750 | FindPerceptionEntry(id) -> entry* | high CONF | linear scan of `+0x1028/+0x102C` | `005d1750` body |
| 0x5D16E0 | HasPerceptionEntry | med CONF | same scan, returns 1/0 | body |
| 0x522AF0 | CSWSArea::GetFirstObjectIndiceByX | high CONF | binary search over `area+0x194/+0x198` (sorted by X); returns 0 on unresolvable id | `00522b7b CMP EAX,ECX / 00522b7d JZ / 00522b7f XOR EAX,EAX` |
| 0x52BB60 | CSWSArea::ClearLineOfSight(srcPos,dstPos,&hitPos,&hitDoorId,srcId,dstId,doorsMode,roomsAlways) ret 0x20 | high CONF | door list test then room-mesh rays | `0052bc04 CALL 0x0084a640`, `0052bcde CALL 0x0052d130`, `0052bd9a CALL 0x005ff950`, `0052be5c RET 0x20` |
| 0x52D130 | CSWSArea::LosObjectBoxTest (door AABB + ray, ret 0x30) | high CONF | loops `area+0x48` doors | `0052d143 CMP [EAX+0x48],0`, `0052d41f CALL 0x00552d90` |
| 0x52B940 | CSWSArea::GetIsIDInExcludeList (area vt+4) | high CONF | scans the exclusion array | body `*(int*)(*(int*)(*param_2 + i*4)+4) == param_1` |
| 0x52B9C0 | CSWSArea::DoorBlockSightRetest (area vt+0x18, new) | med CONF | door 2DA `BlockSight`; if 0 and arg!=0 exclude the door and re-run 0x52D130 | `FUN_00733570("BlockSight")`, `FUN_0052d130(...,0,0,0)` |
| 0x552D90 | DoorOrPlaceableRayTest | high CONF | type 9 placeable / 10 door only, else returns 0 | `(char)piVar1[2] == '\t'`, `== '\n'`, `else uVar2 = 0` |
| 0x5FF950 | RoomMeshRayClear (returns 1 = no hit) | med CONF | ray vs room walkmesh unless `room+0x38 != 0` | `bVar2 = iVar1 == 0` |
| 0x52BE60 | InitHitInfo (zeroes 0x48 bytes, `+0x40=0x7F000000`) | med | local scratch in 0x52BB60 | body |
| 0x52C670 | CSWSArea::GetDoorCrossed (MAP) | med | uses the same door arrays `+0x48/+0x4C/+0x50` | `0052c6e0 CMP ECX,[EAX+0x48]` |
| 0x53DFB0 | GetObjectByIdHash: returns 0 if FOUND, 1 if not | high CONF | `*param_3 = local_18[1]; return 0;` ... `return 1;` | body; found constant `DAT_0099447f` = byte 0 |
| 0x51AD20 | GetCurrentTime(&day,&msOfDay) | high CONF | `/1000`, modulo/div by `+0x3C` | body |
| 0x51AF60 | TimeDiff(now,stamp,&days,&ms) ret 24 | high CONF | writes day diff and ms diff; ms only is what perception tests | body; `-1` = now<stamp -> outputs untouched |
| 0x51AFE0 | TimeCompare | high CONF | 1 / 0 / -1 / -2 (invalid ms) | body |
| 0x51A800 | SetMinutesPerHour (day length) | med | `+0x3C = byte(+0x38)*0x15F900` (default byte 5 -> 7,200,000 ms/day) | `*(uint*)(param_1+0x3c) = (uint)*(byte*)(param_1+0x38) * 0x15f900` |
| 0x51E170 | AddEventDeltaTime(dDay,dMs,callerId,targetId,eventId,payload) ret 24 | high CONF | -> 0x51E220 AddEventAbsoluteTime, node `new(0x18)` | `FUN_0051e220` stores `puVar1[2..5]=param_3..6` |
| 0x51ED80 | GetPendingEvent (6 outs) | high CONF | scheduler pump | `0051d790` calls it |
| 0x6B7BB0 | CSWSCreatureStats::GetSkillRank(skill, vsObj, flag) | high CONF | skill 2 = Stealth, 3 = Awareness (feat bonus switch cases) | `FUN_006b7bb0` `case 2:` feats 0x9E/0x9D, `case 3:` feats 0x7A/0x79 |
| 0x540FD0 | IsAnimIdOutsideSet(short) | low | 0 for 12 ids {0x2712-14,0x275E,0x275F,0x2764-67,0x276D,0x276E,0x2795} else 1 | body |
| 0x51C600 / 0x52F330 | IsPlayerOwnedObject(id) (scan of the server player list) | low | gates the blind-spot cone | `FUN_0052f330` |
| 0x51BD90 / 0x52F4C0 | GetModule (cached) | med | module `+0x15C` = array {ptr,count} of PC ids | T2 name_map row; `0057723x ADD EDX,0x15c` |
| 0x6BADA0 | stub (returns 0) | high CONF | extra score term in listen is always 0 | body: empty loop, `return 0` |
| 0x78EB80 | stub (returns 1) | high CONF | "sound path" test in listen is always 1 | `FUN_0078eb80: return 1;` |
| 0x6AFED0 | stats LoadFromGff (reads PerceptionRange default 0xB, PERCEPTIONDIST, BlindSpot) | med | fills `+0x101C/+0x1020` and stats `+0x84` | decompile lines 560-640 |
| 0x57A560 | SetCombatState (MAP) | high | writes `+0x520` | `0057a634 MOV [EDX+0x520],EAX` |

---

## (b) Findings

### Q1. Perception list struct and event posts: CONFIRMED
List: creature `+0x1028` = pointer to array of entry pointers, `+0x102C` = count (`005d1510`: `local_8 = *(int *)(param_1 + 0x102c); ... *(int *)(param_1 + 0x1028)`). Entry = `new(8)` (`005d1400: FUN_00919723(8)`):
- `+0` u32 object id (`*local_18 = param_1`).
- `+4` u8 packed: bit0 SEEN, bit1 HEARD, bits2-3 invisibility-cache (0 unknown, 1 = hidden, 2 = saw through; written only by 0x585480 via `FUN_005d1400(..,0,0,1|2,0)`), bit4 HIDDEN (= `local_8c`, "T is invisible to P"). Quotes: `*(byte *)(local_18 + 1) & 0xfe | param_2 & 1`, `& 0xfd | (param_3 & 1) << 1`, `& 0xf3 | (param_4 & 3) << 2`, `& 0xef | (param_5 & 1) << 4`.
Lookup is a linear scan (`0x5D1750`, 26 callers): cost grows with list size per (P,T) pair per tick.

Events (all from 0x576DC0, `FUN_0051e170(0,0,Tid,Pid,DAT_0099241c,payload)`): delay 0, caller = T, target = P, event id `[0x99241C]` = 10, payload `new(0x34)` built by `0x5E1A60` with `*payload = 1` (script-event type 1) and int arg0 = code, object arg0 = T. Codes (posted at the quoted asm, decompile lines in 0x576DC0):
The six posts (`CALL 0x0051e170` at 0x5778A7, 0x577995, 0x577B3D, 0x577C1E, 0x577D2E, 0x577E36; payload code set by the preceding `CALL 0x005e1bd0` at 0x577862, 0x577950, 0x577AF9, 0x577BDA, 0x577CE9, 0x577DF1 in decompile order):
- code 1 INAUDIBLE (0x577862): entry bit1 was set and new `local_70 == 0`.
- code 3 VANISHED (0x577950): entry bit0 was set and new `local_74 == 0`.
- code 0 HEARD: new entry with `local_70 != 0` (0x577AF9) or existing entry whose bit1 was clear and `local_70 != 0` (0x577DF1).
- code 2 SEEN: new entry with `local_74 != 0` (0x577BDA) or existing entry whose bit0 was clear and `local_74 != 0` (0x577CE9).
Gate on every post: `param_1[0x472] == 0` (`+0x11C8`); its only writers are the ctor (`0x562fcd`) and `0x5D13E2` (reset to 0 at UpdatePerception's end), so effectively always open (the R1 early exit even skips that reset).
Consumer: `0x56EFD0 case 1` of the event-10 switch (jump table `0x570FF4`, index bytes `0x571010`, payload type `MOVZX EDX,word [EBP-0x...]`): stores T in `+0x1008`, sets exactly one of `+0x100C` (code 0), `+0x1010` (code 2), `+0x1014` (code 1), `+0x1018` (code 3) (decompile: `if (iVar4 == 1) { param_1[0x405] = 1;...} ... == 0 -> [0x403] ... == 3 -> [0x406] ... == 2 -> [0x404]`), then runs script slot 1 = ON_NOTICE (`FUN_006fd8d0(param_1 + 0x9e, param_1[1], 1)`; `0x9E*4 = 0x278 = 0x270 + 1*8`). Party members run it only if the creature is in the party table (`FUN_007e5c40(uVar5) != 0`), other creatures always. Whether the script runs this tick: event time = now, pumped by 0x51D790's `EventPending` loop (SPECULATIVE on same-tick vs next-tick).
Side effect: newly SEEN (`local_14`) sets `+0x11CC = 6000` (`00577ea6 MOV [EDX+0x11cc],0x1770`) only if T is a PC-flag stats (`stats+0x6C != 0`) or `0x5FE3B0(T)` (active leader slot). Consumer `0x569320 @0x56A142-0x56A21E` counts it down by `+0xD0` and finally calls `0x52DE40(area, ...)` (`+0x2EC` clamp to `+0x2E8`): an "excited/alert" timer (name SPECULATIVE).

DoPerception decision table (`this`=P, T=target; A = LOS, H = hidden flag `local_8c`):
- Early: stamp `+0x3BC/+0x3C0`; if `>=20000` ms re-roll dice (Q4). `0x5453c0` areas must match (`local_6c == local_90`) else A = 0.
- A: P is the controlled creature (`0x51C8F0`) -> A = 1 with no ray (`00577124 MOV [EBP+0xffffff60],1`); else if `d^2 <= max(spot,listen)^2` or `T+0x260 != 0` -> `0x52BB60` ray (`==1` -> A = 1); else A = 0.
- H: 1 if any PC id in module `+0x15C` has the same `area+0x124` as P's area, or if `0x585420(P) == 1`; refined only when P is not blind by `H &= GetInvisible(T, P)` (`00577355 CALL EDX` vt+0x10, `0057735b CALL 0x00585480`). H change vs entry bit4 and H==1 -> `ClearHostileActionsVersus(P->T)` (`005773ae CALL 0x005867e0`).
- New entry: A==0 -> only listen is evaluated (`FUN_00577ed0(T,0,..)`), entry created if heard; A==1 -> both.
- Existing entry: if A==0: seen is cleared; heard = listen; if both end 0 -> VANISHED/INAUDIBLE posted and entry removed unless H. If A==1: spot is evaluated only when the entry is NOT seen (`if ((*(byte *)(local_68 + 4) & 1) == 0)`), heard likewise only through that call; a seen entry stays seen while LOS holds unless H or silence (`T+0xFD0`) forces a drop.
- Party shortcut: both P and T in the party table -> forced `A=1, H=0, seen=heard=1` (`0x577663 CALL 0x007e5c40` twice, then `local_a4 = 1; local_8c = 0; local_74 = 1; local_70 = 1`).
- `+0xFF5 in {0x12,0x13}` creatures within `sqrt(10)` m (`[0x989EE0]` = 10.0) of a not-yet-hostile (`rep < 0xB`, not stealthed) T call `0x57E280` (purpose unresolved).

### Q2. Real cadence per creature type: CONFIRMED mechanism, SPECULATIVE on "44 = frame rate"
Callers of 0x56B8A0: `AIUpdate 0x569320 @0x5694E3` (mode 1, every AIUpdate) and `EventHandler 0x56EFD0 @0x56F022` (mode 0, only while `+0x38C == 0`, i.e. before the first AIUpdate). Scheduler: `0x51D790` runs levels 4..0; per level it round-robins `FUN_0051d1c0` (`param_1[3] = param_1[3] + 1; if (param_1[1] <= param_1[3]) param_1[3] = 0`) and calls vt+0x70 = AIUpdate on each object until the cursor wraps to the first object (`if ((local_2c == (int *)0x0) || ((local_2c == local_40 && (local_3c != 1)))) local_44 = 1`) or the budget runs out. So each object gets one AIUpdate per scheduler pass while the budget lasts (AI4 owns the budget). Per AIUpdate the cadence blocks are:

| block | gate (quoted) | party member (`+0x11AC`) | non-party creature |
|---|---|---|---|
| 1 heartbeat+spawn | `0056ba03 CMP EAX,[EDX+0x398]` (3000..4199 ms, `ADD EDX,0xbb8`, IDIV 0x4B0) or `+0x38C==0`; level 0 also needs counter `+0x3D4 >= (+0x398>>6)` (46..65 AIUpdates) | runs, stamp stored `0056bc6a/0056bc79` | same |
| 2 mode-1 party scan | `0056bd5b JNZ 0x0056bde9` skips party; `0056bd8b CMP ECX,[EAX+0x39C]` (IsPC non-party only, 300..699 ms) / `0056bdb4 CMP [EBP-0x28],0xfa0` (others, 4000 ms) | skipped (except first-run `JZ 0x56bdbd`) | **every AIUpdate** (stamp never stored, Q3), no AI-level gate |
| 3 mode 0/2 refresh | IsPC `0056be70 CMP [EBP-0x18],0xc8` (200 ms), others `0056bea2 CMP [EBP-0x18],0xfa0` (4000 ms); then counter `+0x3D0`: `0056bee8 CMP [ECX+0x3d0],0x96` -> level-0 creatures only after >=150 passing AIUpdates, then `+0x3D0 = rand()%0x32` | mode 0, 5/s for the IsPC creature (`+0x10E8`, stays on the Exile), 0.25/s for others (level always 4, no counter gate) | mode 2 every 4 s at level != 0; level 0 ~ every 100-150 AIUpdates after the 4 s mark |
Block 3 tail (both kinds, after the call and the stamp): if the area has no players (`0056bfc2 MOV EDX,[ECX+0x12c]` == 0) and `this+0x78 != 0` -> `CALL 0x0051e120` with level 0 (`0056c01c`): the FM7 drop (a party member in a player-less area toggles 4 -> 0 here and back to 4 in AIUpdate). Note the counters `+0x3D0/+0x3D4` increment per AIUpdate once the elapsed gate has opened (the stamp is only written when the block runs), so the level-0 gates are tick counts, not seconds. |

Runtime baseline (02:38, 5 creatures, 176 calls/s total, max 44/s per creature, `elapsed>=4000` 100% with p1=0): consistent with 4 non-party creatures each calling mode 1 once per AIUpdate at ~44 AIUpdates/s (4 x 44 = 176), the party member calling mode 0 from block 3 at 0.25-5/s (a counter filtered on `[esp+4]==1` does not see it). The 44/s = the scheduler pass rate (one AIUpdate per object per pass); equating it with the render frame rate is SPECULATIVE (AI4). The `elapsed>=4000` flag is true 100% because of Q3 (stamp 0): `elapsed` is the ms-of-day (`+0x3C = byte(+0x38)*0x15F900`, default 7,200,000 ms; fails only for the first 4 s after the wrap, ~0.06% of ticks).

Cost per tick (mode 1, P1 bug): for each of up to 3 party members `0x51C8B0` + `0x51C100` + vec sub + `0x5D1750` (O(list)); if already in list or `d^2 <= range^2` or `T+0x260` -> full `0x576DC0`, which (unless P is the controlled creature) fires `0x52BB60` (door list loop + `area+0x250` room rays) every tick. In combat (`+0x520 != 0`) each `0x58B140/0x58B210` call also does CExoString ctor + `0x71B670` (2DA lookup, `_atof` / `sscanf "%f"`): `0x5D0F30` 2 calls, `0x576DC0` 2 (`00577041 CALL 0x0058b140`, `0057704f CALL 0x0058b210`), spot 1 (`00578989`), listen 1 (`0057815b`) = ~6 per (enemy,party) pair per tick.

### Q3. P1 timestamp never stored: CONFIRMED (exact missing store)
Quotes: reads `0056bcf9 MOV ECX,[EAX+0x3a4]`, `0056bd06 MOV EAX,[EDX+0x3a0]` feed `CALL 0x0051af60` (`0056bd22`); result `[EBP-0x28]` is tested at `0056bd8b` and `0056bdb4 CMP [EBP-0x28],0xfa0`; after the call `0056bdc5 CALL 0x005d0f30` the only store is `0056bde3 MOV [EAX+0x39c],EDX` (the next interval for IsPC). Field census (`field_census.py --offset 0x3a0/0x3a4 --all`): creature writers = ctor only (`00562366 MOV [ECX+0x3a0],0`, `00562376 MOV [EDX+0x3a4],0`); no `LEA/ADD ...,0x3A0`; the other hits are placeable/GUI/VM-table classes. Block 3 by contrast stamps (`0056bf96 MOV [EAX+0x3c8],ECX`, `0056bfa5 MOV [EDX+0x3cc],EAX`). Missing store (what a fix must do after `0056bdc5`): `[c+0x3A0] = [EBP-0x24]` (day), `[c+0x3A4] = [EBP-0x20]` (ms), `c = [EBP+0xffffff2c]`. Note `0x51AF60` outputs only valid when `now >= stamp` (`if (iVar1 == -1) uVar2 = 0xfffffffe` leaves outputs untouched); with stamp (0,0) it never fails. The shipped companion-ai hooks already do this: `0x56BDB4` (7 bytes `81 7D D8 A0 0F 00 00`) interval and `0x56BDE3` (6 bytes `89 90 9C 03 00 00`) stamp, gated by INI `p1` (baseline p1=0).

### Q4. Stealth detection formula: CONFIRMED (constants decoded; some semantics SPECULATIVE)
Spot (`0x578850`, P = this, T = arg1, f = arg2 = hidden flag H), eye offset 1.0 (`[0x986B50]`), range check on eye positions:
1. `if (0x585420(P)) return 0;` `if (f) return 0;` `d^2 > SpotRange^2 && T+0x260 == 0 -> return 0` (`005789ba..005789d5`).
2. `if (P+0xFF4 & 4) return 1` (true-seeing-like flag, `005789e0..005789fd`).
3. `rank = GetSkillRank(T.stats, skill 2, vs P)` (`00578a29 CALL 0x006b7bb0`); `if (rank == 0 || T+0x511 == 0) return 1` (`00578a37 TEST ECX,ECX / JZ 0x00578a55`; `00578a5f`): non-hiding targets are always seen once inside the range and LOS.
4. Else `score = 0`; `if T+0x511 == 1`: `score -= rank; score -= T+0x1024` (`00578aac..00578ad6`); `score += detect(P) in {1,2} ? Awareness(P vs T) : Awareness/2` (skill 3, `0x578b29/0x578b69`, int division); `score += P+0x1026`; moving `P+0x11BC` -5 (or a current-action variant for PCs through `0x58AE90`, `-5` if its `+0x3F8 != 0`); blind-spot cone -10 (below); `P+0x520 != 0` (in combat) -10 (`00578eaf..00578eca`); `anim(T)` not in set L: -5; `anim(P)` not in L: +5 (`00578eee`, `00578f27`); distance term (below). Spotted iff `score > 0` (`0057901b CMP [EBP-0x48],0 / 0057901f JG`).
5. Blind-spot cone (stats `+0x84` = UTC `BlindSpot`, `0x6AFED0` line 562): only if P is NOT a player-list object and T is (`0x51C600`): `angle = acos(dot(norm(T-P), norm(P facing +0xA0)))`; penalty -10 iff `(180 - bs/2) deg < angle <= min(360, 180 + bs/2) deg` (constants 180.0, 360.0, 2.0, `0.01745329`), i.e. T directly behind P (`00578e29..00578ea2`).
6. **Distance term: `d3 = |P.pos - T.pos| / 3.0` (`[0x98B968]` = 3.0, foot positions `+0x94`); if `d3 > 2.0`: `0x579002 CALL 0x0091c860` (ftol of `d3 - 2.0`), `00579007 NEG EAX`, `00579009 MOV [EBP-0x10],EAX`, `0057900c MOV ECX,[EBP-0x48]`, `0057900f SUB ECX,[EBP-0x10]` => `score += ftol(d3-2)`. The log value `local_14 = -n` (`FUN_005e1bd0(7,..)`) shows a penalty, the applied value is a bonus.** Contradiction with the listen function (below) and with the log: CONFIRMED asm; "bug" intent SPECULATIVE.
7. `ModSpotCheck` (area `+0x18C`, string `0x993384` pushed at `00524654`, stored `00524674`) has NO reader in 0x578850 (census of `+0x18C`: only ctor/LoadAreaHeader/PlotGridPath/`0x5959E0 @595ed4` readers); `ModListenCheck` (`+0x190`, `005246a0`) IS read at `0x578639`. So the ARE `ModSpotCheck` is ignored by the vanilla spot check.
8. On success (`local_34 != 0`) it builds a 0x34-byte `0x5E2540` combat-log object and calls `0x579800` (feedback, only when a player is involved).

Listen (`0x577FA0`), eye offset 1.5 (`[0x989F10]` double), no LOS requirement:
1. `if (T+0xFD0 != 0) return 0` (silence effect; writers `0x5A2C70` sets 1, `0x5A2E90` sets 0).
2. `if (f || 0x585420(P))`: `MaxAttackRange(P,T)^2 < d^2 -> return 0`; entry bits2-3 == 1 -> return 0 (`00578149 CMP EDX,1`).
3. `ListenRange^2 < d^2 -> return 0` (`0057815b CALL 0x0058b210`, compare `00578190..00578199`).
4. Two rays P->T and T->P with NO excluded ids (`PUSH 0x7f000000` twice) and `p8 = 0` (any door hit blocks, BlockSight ignored): obstruction thickness `local_10` = `0.4^2` (same door both ways, `[0x9875B0]` = 0.4f), or `|hitB-hitA|^2` (room-mesh walls; zero when clear because the out positions stay `(0,0,0)`), else door-centre distance^2. If `local_10 > 0`: target not yet noticed (`entry bits0/1 == 0`) -> `score -= 5*ftol(sqrt(local_10))`; already noticed -> `0x78EB80() != 1 -> return 0` (stub, always 1) and `score -= 2`.
5. `if (rank(T Stealth) == 0 || T+0x511 == 0) return 1` (**a non-hiding target is HEARD through walls anywhere inside ListenRange**; the obstruction penalty only matters for hiders).
6. Else `score = -rank - T+0x1025` (if `T+0x511 == 1`) `+ Awareness (full in detect mode, else /2) + P+0x1027 + area+0x190 (ModListenCheck)`, moving -5, in-combat (`+0x520`) -10, anim -5/+5 as spot, `- ftol(d/3.0)` (`005787b9 SUB EDX,EAX`, **subtract**), size modifier from `T+0x53C` (1:-8, 2:-4, 4:+4, 5:+8; `00578794..00578809`, NWN size classes), `+ 0x6BADA0()` (stub 0). Heard iff `score + 0 > 0` (`00578839 CMP [EBP-0x1c],0 / JG`).

Ranges: `PrimaryRange/SecondaryRange` come from UTC `PerceptionRange` (byte, default 0xB; `0xC` = error) -> if 0xB the appearance 2DA column `PERCEPTIONDIST` selects the row, then 2DA columns `PrimaryRange/SecondaryRange` (floats stored at `0x6B15B7` (+0x101C) / `0x6B1646` (+0x1020)); in combat state any non-controlled creature uses row 0x12 instead (`0058b1de PUSH 0x12 / 0058b1e3 CALL 0x0071b670`). Numeric values are in the 2DAs (not in the dump). UpdatePerception's discovery radius is `max(spot, listen)` (`fVar4 < fVar3 == (fVar4 == fVar3)` selects `0x58B210` when it is the larger), squared compare.

Dice (CONFIRMED): in DoPerception when `>=20000` ms since `+0x3BC/+0x3C0` (`00576ed7 CMP [EBP+0xffffff7c],0x4e20` -> `JC 0x576ff3` skip): if `this+0x511 == 1`: `+0x1024 = rand%10 + 11`, `+0x1025 = rand%10 + 11` (`00576f05..00576f37`, 11..20); `+0x1026/+0x1027 = rand%20+1` if `0x5802C0(this) in {1,2}` else `rand%10+1` (`00576f63..00576fc9`); stamp rewritten `param_1[0xef] = local_7c`. Consequence: a stealthed T is evaluated against FIXED dice for 20 s per observer-refresh and only re-rolled when T's own DoPerception runs; a failed spot of an unseen T repeats identically every call until then.

### Q5. R1 scan-abort reach: CONFIRMED code path, reach SPECULATIVE (narrower than documented)
`0x522AF0` returns 0 when `FUN_0053dfb0(id)` at a bisection midpoint is not found: `00522b6c CALL 0x0053dfb0 / 00522b71 MOVZX EAX,AL / 00522b74 MOVZX ECX,[0x0099447f] / 00522b7b CMP EAX,ECX / 00522b7d JZ 0x00522b83 / 00522b7f XOR EAX,EAX / 00522b81 JMP 0x00522bde` (`0x53DFB0` returns 0 when found, `DAT_0099447f` = 0). Caller: `005d1287 CALL 0x00522af0 / 005d128c TEST EAX,EAX / 005d128e JNZ 0x005d1295 / 005d1290 JMP 0x005d13ec`: skips the whole new-object scan AND the `+0x11C8 = 0` reset at `005d13e2`.
What it skips: only the discovery of creatures NOT already in P's list, for modes 0 and 2 (party refresh via block 3; non-party-vs-non-party). The existing-list refresh loop (`local_50 = param_1[0x40b]`) runs BEFORE it, so already-known targets still update. Mode 1 (enemy notices party) takes the `param_2 == 1` branch which never calls `0x522AF0` (it walks the party table), so enemy aggro on the party is NOT affected. Other callers of `0x522AF0` (14 incl. UpdatePerception): `0x522BF0 AddObjectToArea`, `0x52B6E0`, `0x54D750`, `0x54E070`, `0x547100`, `0x683D40`, `0x685960`, `0x62E320`, `0x67EB50`, `0x5D51A0`, `0x894320`, `0x623FE0`, `0x52CA10`: the same stale id would also affect them.
The only silent removal in the list refresh is `0x5D15F0` for unresolvable listed ids (no VANISHED event). Trigger needs an id in `area+0x194` whose object is gone and which lies on a bisection path; no writer evidence of such stale ids was found. Dead-creature early exit: `005d0f65.. if (param_1[0x43a]==0 && vt+0x94()==1) return` (vt+0x94 presumably IsDead, SPECULATIVE) also skips the reset.

### Q6. LOS 0x52BB60 geometry, E3: CONFIRMED (static): creatures do NOT block LOS
Call (from perception, `005771d5..005771fd`): `this`=P's area (`0x5453C0`), `p2 = P.pos+(0,0,1.5)`, `p3 = T.pos+(0,0,1.5)` (`[0x989F10]` d=1.5 added at `005770d7`), `p4 = &hitPos`, `p5 = &hitDoorId`, `p6 = P.id`, `p7 = T.id`, `p8 = 1`, `p9 = 0`. Listen uses `0x7F000000` ids, `p8 = 0`, `p9 = 0`, eye 1.5 as well; MoveToPoint's range shortcut is other code (AI1).
Steps (asm `0x52BB60`):
1. z-band `[min(z1,z2), max(z1,z2)]` (`0052bbc4..0052bbfc`), `0x84A640` builds an exclusion array; `p6`/`p7` are resolved with `0x51C0A0` and the object pointers appended (`0052bc28..0052bc92`, `0x83EA60 Add`) when `vt+0x10() != 0`; `0x7F000000` resolves to NULL and excludes nothing. The array is only consulted by the door test (`0x52B940` compares `*(entry+4)` = object id), so the src/dst exclusion only matters when an endpoint is a door.
2. `0x52D130(src,dst,zmin,zmax,0.001,0x7F000000,&hitDoorId,1,&excl,p8,&hitInfo,0)` (`0052bcde`): loops `area+0x48` doors with AABB reject then `vt+4` = `0x52B940` (door in exclusion list?) then `0x552D90` ray test (placeable type 9 / door type 10 only, `else uVar2 = 0`). A hit with `p8 == 1` calls area `vt+0x18` = `0x52B9C0` which reads the door's `BlockSight` 2DA flag and, if 0, excludes that door and re-tests the rest; BlockSight != 0 -> blocked. With `p8 == 0` any ray-hit door blocks.
3. If the door test returned clear (or `p9 != 0`): loop `i < area+0x250` over `0x4C`-byte rooms at `area+0x254`: `0x5FF950(room, src, dst, &hit)` (skipped when `room+0x38 != 0`), nearest hit by squared distance (`FLT_MAX` init `[0x993900]`) -> blocked. Result `1` clear, `0` blocked.
Creatures: `cg.py callees` of 0x52BB60/0x52D130/0x552D90/0x5FF950 contain only `GetGameObject`, door/placeable tests and mesh rays; no creature list or `area+0x194` array is read. Both endpoints' own geometry cannot block (they are excluded by id or are not tested). So allies, enemies and the leader never block LOS; walls, closed BlockSight doors and room-mesh faces do. The two LOS users disagree on eye height/exclusions as already documented (attack eye 1.5 + exclusions; perception eye 1.5 + exclusions; listen no exclusions; spot score uses 1.0 and feet positions): the spot distance/range uses a different height than the LOS ray it depends on.

### Extra findings (not asked, high value)
- X1 Spot sign (Q4 item 6). Effect size: bonus `ftol(d/3 - 2)` for `d > 6` m, up to `+(PrimaryRange/3 - 2)` (e.g. +4 at 18 m, 2DA-dependent); makes hiders easier to see from afar. Candidate 1-byte fix `0x57900F 2B -> 03` (cold path: stealthed targets only).
- X2 ModSpotCheck ignored (Q4 item 7): ARE `ModSpotCheck` has no effect in K2 vanilla.
- X3 Seen-stays-seen: stealth is only (re)rolled for unseen entries; sneaking after being spotted does nothing until LOS breaks or H (invisibility) forces a drop.
- X4 Party-to-party always perceive (Q1) and `GetInvisible` sanctuary-style type 63: saving-throw-style check (`0x6E4630(3, DC=param(4), ...)`, a Will save is SPECULATIVE) per observer, result cached in entry bits2-3; a failed save calls `0x586640` -> `SetCombatState(0,1,0)` + `ClearHostileActionsVersus`.
- X5 `AIMaster+0x70` (`0x5D0F8F MOV [EDX+0x70],1`) is a "perception ran this AIUpdate" flag cleared by `0x51D790 @0x51DC2E` and read only by the slow-AI warning `0x549F70 @0x54A164`.
- X6 Combat-state perception range override (`+0x520`) is a behaviour change: while a creature is in combat its spot/listen range is 2DA row 0x12, not its UTC range (also for companions, not for the controlled creature).

---

## (c) Runtime probes (none needs the game now; all are counters, no behaviour change)
| id | counter | hook | original_bytes | expected signal |
|---|---|---|---|---|
| RP1 | mode-1 calls per creature/s and `elapsed` | `0x5D0F30` len 9 | `55 8B EC 81 EC C4 00 00 00` | `[esp+4]==1`: ~tick rate per non-party creature at p1=0, ~0.25/s at p1=1; ECX = creature, `[ecx+0x11AC]` party flag |
| RP2 | LOS rays per second | `0x5771D5` len 7 | `6A 00 6A 01 8B 45 A8` (no rel branch, no inbound target) | ~ (#in-range enemy x party member, controlled creature excluded) x tick rate at p1=0; at the hook `[EBP+0xfffffe54]` = P, `[EBP+8]` = T, `[EBP-0x58]` = T id |
| RP3 | spot failures + score | `0x579021` len 7 | `C7 45 D0 00 00 00 00` | `[EBP-0x48]` = score (<=0), `[EBP+8]` = T, `[EBP+0xfffffe90]` = P; fires only for hiding targets |
| RP4 | listen failures + score | `0x57883F` len 7 | `C7 45 D0 00 00 00 00` | `[EBP-0x1C]` = score, `[EBP+8]` = T, `[EBP+0xfffffe7c]` = P |
| RP5 | 2DA range re-reads / s | `0x58B140` len 5 | `55 8B EC 6A FF` (function entry; calls land on it) | in combat ~6 per pair per tick; ECX = creature, `[ecx+0x520]` combat flag |
| RP6 | R1 stale ids | none hot; at area load walk `area+0x194/+0x198` and count `0x53DFB0(id) != 0` | n/a | 0 expected; non-zero confirms R1 reach |
| RP7 | spot sign validation | RP3 + a log of `[EBP-0x10]` just after `0x579009` | n/a | `[EBP-0x10] = -n` while the applied delta is `+n` |

## (d) Open questions
1. 2DA data not in the dump: `ranges.2da`-like table at `[rules+0x130]+0xBC` rows 0xB..0x12 (PrimaryRange/SecondaryRange values, combat row 0x12), `PERCEPTIONDIST` per appearance, door `BlockSight` table (`[rules+0x130]+0x28`), IFO MinPerHour (`0x51A800` callers `0x5582F0`, `0x7F3040`).
2. Meaning of `+0xFF4` bits (effect handlers `0x5A4FD0/0x5A5600` set `|=1/|=4`; removers `0x5A5020` (effect 0x46), `0x5A57E0` (0x48), `0x5A5D60` (0x49); bits 2/8/0x10 set by `0x5A5130/0x5A53C0`), NWN-like (see-invisible 1, ultravision 2, true-seeing 4, darkness 8, blind 0x10) is SPECULATIVE.
3. `0x58AE90` / `+0x3C0 == DAT_0099ED24` / `+0x3F8` (PC "current action" -5 in spot/listen), `0x540FD0` animation-id set L, `+0xD8 == 10000`, `0x57E280` (mapper name SetLastInstantEquip is wrong), `0x5E2540`/`0x579800` (spot feedback).
4. area `vt+0x18` signature (10 args) vs `0x52B9C0` (9 stack args + recursion arg): `p8`/`p9` roles are inferred from the asm push order only.
5. Is the event delivered the same tick (`0x51D790` `EventPending(now)` loop) or the next? Needs AI4's clock details.
6. Reachability of an unresolvable id in `area+0x194` (RemoveObjectFromArea `0x523090` removes through helpers not covered here).
7. Decomp gaps: none blocking. `0x71B670` (2DA GetFloat) read from decompile only.

## (e) Struct field tables
| class | offset | size | type | proposed name | readers (addr) | writers (addr) | conf | evidence |
|---|---|---|---|---|---|---|---|---|
| CSWSObject/Creature | 0x04 | 4 | u32 | object id | everywhere (`MOV EDX,[ECX+4]`) | ctor | high | `00576e..0x57710b MOV ECX,[EAX+4]` |
| CSWSObject | 0x78 | 4 | int | AI level | `0056ba45` | `0x51E120` | high | `0056ba45 MOV ECX,[EAX+0x78]` |
| CSWSObject | 0x94/0x98/0x9C | 12 | vec3 | position | many | movement | high | `0057878a ADD EAX,0x94` |
| CSWSObject | 0xA0 | 12 | vec3 | facing | `00578d55 ADD ECX,0xa0` | | med | |
| CSWSObject | 0xD8 | 4 | u16 used | current animation id | `00578ee3`, `0x5802C0` | | med | `00578ee3 MOVZX ECX,word [EBP+0xffffff..]` |
| CSWSObject | 0x260 | 4 | int | ForceAlwaysUpdate (bypass range) | `005771a5`, `005789be`, `005d1127`, `005d1381` | `0x5744C3` (GFF "ForceAlwaysUpdate" `0x99537C`), `0x69BDC8`, ctor `0x53EF08` | high | `005744c3 MOV [EAX+0x260],ECX` |
| CSWSCreature | 0x270+8*n | 8 | script slot n | script slots (0 heartbeat, 1 notice, 8 spawn) | `0056b9e9`, `0x56EFD0` | LoadCreature | high | `lea ecx,[eax+edx+0x270]` |
| CSWSCreature | 0x38C | 4 | int | first-run done flag | `0056b98b`, `0056f005` | `0056c042` (=1), ctor `0x5626F3` | high | `0056c042 MOV [EAX+0x38c],1` |
| CSWSCreature | 0x390/0x394 | 8 | day,ms | heartbeat stamp | `0056b95a` | `0056bc6a/0056bc79`, `0x5692CD`, `0x58BEBF` | high | |
| CSWSCreature | 0x398 | 4 | ms | heartbeat interval 3000..4199 | `0056ba03` | `0056bc98` | high | `0056bc8c ADD EDX,0xbb8` |
| CSWSCreature | 0x39C | 4 | ms | mode-1 interval 300..699 (IsPC non-party) | `0056bd8b` | `0056bde3`, ctor `0x562399` | high | `0056bdd7 ADD EDX,0x12c` |
| CSWSCreature | 0x3A0/0x3A4 | 8 | day,ms | mode-1 stamp (never stored after ctor) | `0056bd06/0056bcf9` | ctor `0x562366/0x562376` only | high | census |
| CSWSCreature | 0x3BC/0x3C0 | 8 | day,ms | dice-reroll stamp | `00576eb6/00576ea9` | `00576fde/00576fed` | high | |
| CSWSCreature | 0x3C8/0x3CC | 8 | day,ms | block-3 stamp | `0056be06/0056be13` | `0056bf96/0056bfa5` | high | |
| CSWSCreature | 0x3D0 | 4 | int | block-3 counter | `0056beb5` | `0056bec4`, `0056bf26`, ctor | high | `0056bee8 CMP [ECX+0x3d0],0x96` |
| CSWSCreature | 0x3D4 | 4 | int | heartbeat counter | `0056ba2a` | `0056ba39`, `0056ba8d` | high | |
| CSWSCreature | 0x510 / 0x511 | 1/1 | u8 | detect mode / stealth mode | `0x5802C0`, `00576eed`, `00578a3e` | `0x5B7ABF` (SetStealthMode `+0x511`) | high | `005b7abf MOV [EAX+0x511],CL` |
| CSWSCreature | 0x520 | 4 | int | in-combat flag | `0058b177`, `00578eaf` | `0057a634` | high | |
| CSWSCreature | 0x53C | 4 | int | size class (1..5, NWN-like) | `005787c1` | `0x5739FD`, `0x574251`, `0x5800ED` | med | `005787c1 MOV ECX,[EAX+0x53c]` then compares 1/2/4/5 |
| CSWSCreature | 0xFD0 | 4 | int | silenced | `005774b0`, `0057800b` | `0x5A2E6B` (1), `0x5A2EBD` (0) | high | |
| CSWSCreature | 0xFF4 | 1 | u8 | sight flags | `00585429..`, `005789e0`, `0x585480` | effect handlers `0x5A4FD0 +0x5A5010` etc. | med | |
| CSWSCreature | 0xFF5 | 1 | u8 | creature class byte (script package switch 1/0x10/2/0xB; 0x12/0x13) | `0056bb65`, `00576dc0` | | low | |
| CSWSCreature | 0x1008..0x1018 | 5x4 | int | last perceived object, heard, seen, inaudible, vanished flags | `0x56EFD0` scripts (GetLastPerception*) | `0x56EFD0 @0x570413..` | high | decompile case 1 |
| CSWSCreature | 0x101C / 0x1020 | 4/4 | float | PrimaryRange (spot) / SecondaryRange (listen) | `0058b16b`, `0058b23b` | ctor `0x562E27/0x562E19`, `0x6B15B7/0x6B1646` | high | |
| CSWSCreature | 0x1024..0x1027 | 4x1 | u8 | stealth dice (11..20) x2 / spot,listen dice | `00578abe`, `0057855f`, `00578b88`, `0057861d` | `00576f1b..00576fc9` | high | |
| CSWSCreature | 0x1028 / 0x102C | 4/4 | ptr,count | perception list array / count | `005d0f..`, `005d1750` | `0x5D1400`, `0x5D15F0` | high | |
| CSWSCreature | 0x1198 | 4 | ptr | stats | `0x578a05` | | high | |
| CSWSCreature | 0x10E8 | 4 | int | IsPC cache | `0056bd67`, `0056be55` | `0x5FE4B0` | high | R1 doc |
| CSWSCreature | 0x11AC | 4 | int | party flag | `0056bd48`, `0056bf5c` | `0x58A330` | high | |
| CSWSCreature | 0x11BC | 4 | int | currently moving (MoveToPoint) | `00578c2e` | `005c209c` (1), `005c20d3` (0) | high | |
| CSWSCreature | 0x11C8 | 4 | int | suppress perception events (dead gate) | six reads in 0x576DC0 | ctor `0x562FCD`, `0x5D13E2` (0) | high | |
| CSWSCreature | 0x11CC | 4 | ms | excited timer (name low) | `0056a142..` | `00577ea6` (=6000), `0056a1e6/0056a21e` | med | |
| CSWSCreatureStats | 0x6C | 4 | int | IsPC | `00578bc7 MOV EDX,[ECX+0x6c]` | `0x6AFED0`/ctor (not checked) | med | |
| CSWSCreatureStats | 0x84 | 4 | float | BlindSpot | `00578df6 FLD float [EAX+0x84]` | `0x6AFED0` line 562 (`"BlindSpot"`) | med | |
| perception entry | +0 | 4 | u32 | object id | `0x5D1750` | `0x5D1400` | high | |
| perception entry | +4 b0/b1/b2-3/b4 | 1 | bits | seen / heard / invis cache / hidden | `00577387`, `0x585480` | `0x576DC0`, `0x5D1400` | high | |
| CSWSArea | 0x48 / 0x4C / 0x50 | 4/4/4 | count, AABB[0x18], ids | door count, door AABBs, door ids | `0052d143`, `0052c6e0` | other (no direct W) | med | `0052d2b3`, `0052d3c7` |
| CSWSArea | 0x124 | 4 | int | area id/tag compared in H | `005772d5`, `005772e7` | | low | |
| CSWSArea | 0x12C | 4 | int | players in area | `0056bfc2` | `0x52334A`, `0x523376` | high | `DecrementPlayersInArea` |
| CSWSArea | 0x18C / 0x190 | 4/4 | int | ModSpotCheck / ModListenCheck | `0x578639` (0x190 only) | `00524674`, `005246a0` | high | |
| CSWSArea | 0x194 / 0x198 | 4/4 | ptr,count | all-objects id array sorted by X | `00522b50`, `00522b12` | `0x522E00` | high | |
| CSWSArea | 0x250 / 0x254 | 4/4 | count, ptr | rooms (0x4C each) | `0052bd3e`, `0052bd94` | `0x5258A0` | high | |
| CServerAIMaster | 0x70 | 4 | int | perception-ran flag | `0054a164` | `0051dc2e` (0), `005d0f8f` (1) | med | |
| time object | 0x38 / 0x3C | 1/4 | u8,u32 | minutes-per-hour / ms per day | `0051afe0`, `0051ad20` | `0x51A800` | high | |

## (f) Hook-site catalogue
Existing sibling patches near these functions: companion-ai `0x56BDB4` (len 7), `0x56BDBD` (len 8), `0x56BDE3` (len 6). Installed list `live_hooks_2026-09-30{,b,e,g}.tsv` and `ranged_ai_src` have no entry in `0x56B8A0..0x56C060`, `0x5D0F30`, `0x576DC0..0x5792xx`, `0x52BB60`. Inbound-target scans (command: `grep -rE "(J[A-Z]+|CALL|JMP) +0x0*<addr>\b" decomp/*/*.asm` for every address inside each window): all "none" except the start of `0x56BE79` (two inbound `JZ/JNZ 0x0056be79` at `0056be68/0056be6e`, allowed: the window starts on the target). Jump tables: only `0x56BB96` (`0x56C060`) in 0x56B8A0, not near these windows.
| id | address | length | stolen bytes | convention / live regs | consumed exit? | EAX/stack notes | hot/cold | nearest sibling self-patch site |
|---|---|---|---|---|---|---|---|---|
| AI3-H1 | 0x56BE79 (P2 refresh) | 6 | `8B 85 2C FF FF FF` | `ebp`: `[ebp-0x18]` elapsed, `[ebp+8]` mode, `[ebp-0xD4]` creature | no (optionally skip-only) | stolen `MOV EAX,[EBP-0xD4]` is followed by `MOV ECX,[EAX+0x10E8]` (EAX rewritten anyway); flags of the earlier `CMP` at `0x56be70` are not read after this point (next setter `CMP` at `0x56be8b`) | hot (per AIUpdate per creature) | `0x56BDE3`+6 = 0x56BDE9: 144 bytes away (>12) |
| AI3-H2 | 0x5771D5 (LOS probe) | 7 | `6A 00 6A 01 8B 45 A8` | ebp frame: `[ebp+0xfffffe54]` P, `[ebp+8]` T, `[ebp-0x58]` T id | no | stolen `MOV EAX,[EBP-0x58]` writes EAX (EAX-free before); next insn pushes EAX | hot (per DoPerception with `d^2 <= r^2`, not controlled) | none within 12 |
| AI3-H3 | 0x579021 (spot-fail) | 7 | `C7 45 D0 00 00 00 00` | ebp: `[ebp-0x48]` score, `[ebp+8]` T, `[ebp+0xfffffe90]` P | optional replace | stolen insn is a store (EAX/ECX free); flags are not read after | cold (hiders only) | none |
| AI3-H4 | 0x57883F (listen-fail) | 7 | `C7 45 D0 00 00 00 00` | ebp: `[ebp-0x1c]` score, `[ebp+8]` T, `[ebp+0xfffffe7c]` P | optional | as H3 | cold | none |
| AI3-H5 | 0x5D0F30 (entry) | 9 | `55 8B EC 81 EC C4 00 00 00` | ECX = creature, `[esp+4]` = mode (char) | no | prologue not yet executed: preserve ECX/flags | hot | none |
| AI3-H6 | 0x576DC0 (entry) | 5 | `55 8B EC 6A FF` | ECX = P, `[esp+4]` = T | no | prologue | hot | none |
| AI3-H7 | 0x58B140 (entry) | 5 | `55 8B EC 6A FF` | ECX = creature | no | prologue; returns float in ST0 (do not touch FPU) | hot in combat | none |
| AI3-B1 | 0x57900F (spot sign byte patch) | 1 (data byte, no detour) | `2B` (`2B 4D F0 SUB ECX,[EBP-0x10]` -> `03 4D F0 ADD ECX,[EBP-0x10]`; `[EBP-0x10]` already holds `-ftol`) | n/a | n/a | result stored by `0057900f..00579012 MOV [EBP-0x48],ECX`; flags are rewritten by `CMP [EBP-0x30],1` at `0x579015` before any read | cold | none (>12 from 0x56BDE3 etc.) |
Notes: AI3-H1 duplicates the CA-P2 window of the P2 research (not yet in the shipped toml). R1 probe sites are unsuitable: `0x5D1290` is a relative `JMP E9 57 01 00 00` (5 bytes) and `0x522B7F..0x522B81` contains `EB 5B`; use RP6. Equivalent clearing-LOS skip for P1 is the shipped `0x56BDB4/0x56BDE3` pair.

## (g) Mod relevance
- Q1 events (code table, slot 1): P1/P2 (event timing), enemy-E7 (`hold_enemies` relies on OnNotice not re-engaging), companion-ai notice scripts.
- Q2 cadence table: P1 (enemy per-tick vs 4 s), P2 (companion 4 s, IsPC stays on the Exile), FM7 (block 3 AI-level drop `0x56C01C`), AIQ/perf (mode-1 per-tick cost, 2DA re-reads in combat), PFLK none.
- Q3 missing store: P1 (shipped `0x56BDE3`); designed 4 s is the vanilla cadence the fix restores.
- Q4 formula: enemy-aggro (stealth), spot sign bug candidate (new item, suggest `B1`), `ModSpotCheck` ignored (new item), E3 not affected.
- Q5 R1: R1 downgrade (companion refresh and enemy-vs-non-party only), enemy aggro safe.
- Q6 LOS: E3 settled (creatures never block), a1_retarget_strict / A1 / Mod 8 kite (LOS-blocked approach uses the same door/room geometry), M8 none.
- X6 combat-range override: Mod 8 kite (companions in combat see at the row-0x12 range), enemy-AI.
- party-flag `+0x11AC`: block 2 skip, block 3 mode 0 vs 2, ON_NOTICE party-table test (`0x56EFD0 case 1`).
- medpac, C1 (H -> `ClearHostileActionsVersus` is a C1 trigger via perception), T1, EFFOVF, FM1-FM6, L1-L3, M5, E4, E9, PFLK: none beyond the above.
