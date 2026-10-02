# AI5: Follow, party and formation (Steam swkotor2.exe, static, read-only)

## Summary (5 lines)
1. **`+0x11AC` has exactly two writers** (ctor `0x56214C` = 0, `SetPartyMemberFlag 0x58A330` at `0x58A35E`), and **a companion standing on the Ebon Hawk that is not one of the 2 selected followers has 0**: none of the 15 call sites flags a `SpawnAvailableNPC` / `GetNPCObject` creature (`0x5FD480`/`0x5FAAF0` never call `0x58A330`). Flag = 1 only for the PC and for NPCs added by `AddMember 0x5F8D80` / `CreateParty 0x5F9220` / party-select `0x89FBD0` (+ PC loaders and an effect-89 creature). Correct party test = `CSWPartyTable::GetIsMember_2 0x5FB070(objectId)` (server table) or `CSWParty::IsPartyMember 0x7E5C40(objectId)` (client party); `+0x11AC` is the same fact cached per creature.
2. **FollowInfo (`creature+0x500`, 0x3C bytes) is almost vestigial**: its walk/run-rate readers are dead code (`00575e04 XOR EAX,EAX / JZ`, `00576111 XOR EAX,EAX / JZ`); the only live field is `+0x38` InSafetyRange (5 m edge in `CSWParty::Update 0x7E6CE0`). The real follow state lives in the **client `CSWParty` slot elements** (stride 0x88, state at `E+4`, 11-state machine driven by `0x5C6340`) and a 100-entry leader trail ring (`CSWPartyFollowDataArray`, `P+0x2DC`).
3. **FM6 consumer found**: the per-slot flag set at `0x7E6546` (`E+0x70`) is read only by the state-5 (hold) handler `0x5CA6A0` (`005ca7c6`/`005ca7e8`/`005ca802`): while it is 1 and the follower is < 7 m (49.0) from the new leader it does NOT re-plan, beyond 7 m it clears the flag and re-plans. Script/deferred/party-select leader changes never call `ResetAllFollowData 0x7E8350` (UI path only), so non-idle slots keep their stale state (FM6 partly refuted, residual low).
4. **FM4 extension**: `SaveCreature` runs `SetPartyMemberFlag(flag,1)` for EVERY creature, so a non-party creature is also forced to AI level 2 on every save (`0058a448 PUSH 2`); the known one-byte fix at `0x574529` does not cover this (second call at `0x57516B`), a second byte at `0x575160` does.
5. Follow only runs when action `0x3D` (AIActionFollowLeader `0x5BF7F0`) sets `+0x11B0=3` and `QuickWalk 0x5C3F70` calls `0x5C6340` (`005c4142 CMP [EBP-0x238],3 / 005c4151 CALL 0x5C6340`); the engine queues `0x3D` at only two sites (`AddCharacter 0x7E5A6E`, script `ActionFollowLeader 0x66A04E`), so `ClearAllActions` on a follower (`+0x11B0:=1`, slot state `:=-1`) leaves it idle until a script re-queues it (FM1/E7).

Method: `.asm`/`.c` from `ROOT/decomp`, `field_census.py --all` (offsets `0x11AC`, `0x500`, `0x11B0`, `0x70`, `0x94`), `cg.py`, constants from `backups/vanilla_clean/swkotor2.exe` (PE section reader; `.rdata` file offset = VA-0x986000+0x584C00). Command table `0x665F50`: `cmd index = store offset / 4`, handler = immediate (verified: 574 AddPartyMember `[+0x8f8]=0x66C990`, 576 `[+0x900]=0x68BBE0`, 834 SetCreatureAILevel `[+0xd08]=0x66B640`, matching `nwscript.nss` order).

## (a) Function table
Legend: C = confirmed against decompile/asm in this batch; M = mapper/DB only.

| addr | proposed name | conf | purpose | evidence |
|---|---|---|---|---|
| 0x58A330 | CSWSCreature::SetPartyMemberFlag(flag, apply) [C] | high | writes `+0x11AC`; apply!=0: flag=0 frees FollowInfo + level 2, flag!=0 allocs FollowInfo + level 4 | `0058a35e 8988ac110000 MOV [EAX+0x11ac],ECX`; `0058a448 PUSH 2`..`0058a458 CALL 0x51e120`; `0058a384 PUSH 0x3c`/`0058a3a1 CALL 0x5752c0`; `ret 8` |
| 0x5752C0 / 0x575340 | CFollowInfo ctor / default init [C] | high | `[0]=0x7f000000, [1..11]=0, [0xc]=2, [0xd]=0, [0xe]=1` | decomp 0x575340; `[0x9944b8]`=2 read from exe |
| 0x575200 / 0x561D80 | CSWSCreature::LoadFollowInfo / FollowInfo GFF loader [C] | high | loads struct "FollowInfo"; field names below; allocs `+0x500` WITHOUT touching `+0x11AC` | `0057528e MOV [ECX+0x500],EDX`; strings "FollowObject","FollowLocation",... |
| 0x5751B0 | CSWSCreature::SaveFollowInfo [C] | high | writes "FollowInfo" if `+0x500!=0` | `005751bc CMP [EAX+0x500],0` |
| 0x574500 | CSWSCreature::SaveCreature [C partial] | high | clears flag (apply=1), writes GFF, restores flag (apply=1), then SaveFollowInfo | `00574532 CALL 0x58a330` (0,1); `0057516b CALL 0x58a330` (old,1); decomp l.148-149 `FUN_0058a330(local_3c,1); FUN_005751b0(...)` |
| 0x58A470 | CSWSCreature::SetPuppetOwnerFlag (`+0x11FC`) [C] | med | PUP twin of the party flag: level 4 / 2 | `0058a470` decomp `*(param_1+0x11fc)=param_2; ...0x51e120` |
| 0x5BF7F0 | CSWSCreature::AIActionFollowLeader (type 0x3D) [C] | high | never completes: 3 on any gate failure, else sets `+0x11B0=3`, returns 1 | `005bf809 JZ 0x5bf881` (flag 0), `005bf817`..`JZ` (+0x500==0), `005bf925 MOV [EAX+0x11b0],0x3`, `005bf92f` returns `[0x9944b4]` (=1) |
| 0x5BF940 | CSWSCreature::AIActionFollowOwner (type 0x46) [C partial] | med | puppet follows owner (`+0x11FC`): AddMoveToPoint + re-queue 0x46 when farther than node float `+0x38` | decomp: `param_1[0x47f]`=+0x11FC, `FUN_00564770(0xffff..)`, `FUN_0053f7f0(0x46,..)` |
| 0x5BCDC0 | CSWSCreature::AIActionCheckMoveToFollowRadius (type 0x40) [C partial] | med | post-move check: other area or dist² > (r+0.01)² -> slot state := 5; always returns 2 | `005bd09e MOV [EDX+0x4],0x5`; `[0x987538]` f64 = 0.01 |
| 0x5C3F70 | CSWSCreature::WalkUpdateLocation_QuickWalk (follow gate) [C] | high | calls follow driver iff `+0x220!=0 && +0x11B0!=1 && vt+0x94==0 && flag` and `+0x11B0==3` | `005c3fa4`..`005c3fb7`, `005c3ffc CMP [EDX+0x11ac],0`, `005c4142 CMP [EBP-0x238],3`, `005c4151 CALL 0x5c6340` |
| 0x5C6340 | CSWSCreature::FollowLeaderDriver (QuickWalk_FollowLeader) [C] | high | per-follower state machine; slot = `GetIndex(clientId)`; slot 0 returns 2 | `005c6340` decomp `local_84 = FUN_007e6be0(); if ((local_84 != -1) && (local_60 = DAT_009944b8, local_84 != 0))`; `005c6ace JMP [ECX*4+0x5c7334]` |
| 0x5C7370 | FollowState0_PlaceBehindTrail [C] | med | target = trail point `E+0x28` + R(heading of node `E+0x34`) * `E+0x38`; queues action 0x40 + MoveToPoint (run); state := 6 (reachable) / 7 | decomp l.40-120 (`param_2+0x38` rotated by `FUN_005195a0`), `*(param_2+4)=7`/`6` |
| 0x5C7AD0 | FollowState1_WalkTrail [C partial] | low | walks trail nodes; stuck -> state 2 | decomp l.308-313 `*(param_2+4)=2` |
| 0x5C87E0 | FollowState2_Blocked [C partial] | med | posts OnBlocked (payload 0x1F, delta 0) via `0x51E170`; state := 10 | decomp l.121-131, `*(param_2+4)=10` |
| 0x5C8C20 | FollowState3_OffsetWalk [C partial] | low | timer `E+0x6C += dt`; at >=3000 ms and blocked posts OnBlocked; state 4 or 10 | decomp l.111, l.357 `2999 < *(param_2+0x6c)`, l.358/365 |
| 0x5CA000 | FollowState4_ToTrailEnd [C partial] | low | state 2 or 10 on failure | decomp l.142/148 |
| 0x5CA6A0 | FollowState5_Hold (FM6 consumer) [C] | high | `E+0x70` flag read/clear, 7 m re-plan | `005ca7c6 MOV ECX,[EAX+0x70]`; `005ca7e8 FCOMP [0x996520]` (49.0); `005ca802 MOV [ECX+0x70],0` |
| 0x5CACD0 / 0x5CAE00 / 0x5CAF30 / 0x5CB060 | FollowState6..9_WaitTrailAdvance [C] | med | wait until the leader trail passed node `E+0x34`, then state 1 / 3 / 4 / 2 | decomp l.47-52 of each (`*(param_1+4)=1/3/4/2`); only the target differs |
| 0x5CB190 | FollowState10_SkipToWalkable [C] | med | walks `E+0x24` forward to first node with `0x54B650` ok, then state 2 | decomp l.35-63 |
| 0x7E5180 / 0x7E5610 | CSWParty ctor / slot-element init [C] | high | 3 slots of 0x88 at `P+0x24`; ids = `0x7f000000`; formation presets | `007e51c2 ADD ECX,0x24`, `007e51db ADD EAX,0x88`; 7e5610 `*param_1=0x7f000000`, `param_1[0x1c]=0` |
| 0x7E5730 | CSWParty::AddCharacter(id) [C] | high | first free slot (max 3), `E+4=-1`, perception ranges, **queues action 0x3D for slot>0** | `007e5a69 PUSH 0x3d / 007e5a6b MOV ECX,[EBP-0x20] / 007e5a6e CALL 0x53f7f0` |
| 0x7E5C40 / 0x7E5DA0 / 0x7E6BE0 / 0x7E6C40 | CSWParty::IsPartyMember(id) / GetCharacter(slot) / GetIndex(id) / RemoveCharacter_2(id) [C] | high | id lookups over `P[slot*0x22+9]` | decomp 7e5c40: `param_1[local_8 * 0x22 + 9] == param_2` |
| 0x7E5DF0 | CSWParty::SetLeaderBySlot(slot) [C] | high | rotates ids, server `SetLeader`, runs `k_hen_leadchng`, sets FM6 flags | `007e64da PUSH 0x9a2178`; `007e653b CALL 0x6fd8d0`; `007e6546/6556/6566 MOV [..+0x94/0x11c/0x1a4],1` |
| 0x7E6670 | CSWParty::FlipLeaderFormationOffsets [C] | high | negates x of slot1/2 offsets (`P+0xE4`, `P+0x16C`), toggles `P+0x1C4` | decomp `-*(float*)(param_1+0xe4)` |
| 0x7E6720 | CSWParty::ApplyLeaderFormation(idx) [C] | high | copies preset `idx` (0..2) into `P+0xE4`/`P+0x16C`, spacing `P+0x1C0`; **only caller = ctor** (`FUN_007e6720(0)`) | decomp l.35-60; callers `007e6720,007e5180` |
| 0x7E6960 | CSWParty::GetWorldFormationPosition(out, leaderPos, leaderOri, slot) [C] | high | leaderPos + R(leaderOri) * `P+0x5C+slot*0x88`; **only caller 0x89FBD0** | decomp; `callers=0089fbd0` |
| 0x7E6CE0 | CSWParty::Update(dt) = "RangeMonitor" [C] | high | smoothed leader point, `TestAndResetPath`, `RecalcFollowPoint`, k_hen_enter5m/exit5m/retreat | decomp (whole); `callers=00781be0` |
| 0x7E7900 | CSWParty::TestAndResetPath [C] | med | leader turned back on itself: slots 1,2 (state not in {-1,0,5,6,7}) := state 4 and mirror formation | decomp l.54-77 |
| 0x7E7D70 | CSWParty::GetFollowRange [C] | med | ranges.2da column PrimaryRange, row 0xE (row 0xF when party-table `+0x110==1`), default 5.0 | decomp; `[0x986b68]` f32 = 5.0 |
| 0x7E7EA0 / 0x7E8350 | CSWParty::Reset / ResetAllFollowData(pos,heading,flag) [C] | high | refills the 100-entry ring, all 3 slots state := -1, targets := pos | decomp 7e8350 `*(param_1+0x28)=-1`, `+0xb0`, `+0x138` |
| 0x7E8640 / 0x7E8720 | CSWParty::FollowStateChanged / RecalcFollowPoint [C] | high | map states 1,2,3,4 -> 6,9,7,8; slot1 target = trail point at (range - spacing), slot2 at (range + spacing) | decomp 7e8640 `case 1: =6`; 7e8720 `FUN_007e7d70 - *(param_1+0x1c0)` |
| 0x7E88A0 / 0x7E8970 / 0x7E89E0 | CSWPartyFollowDataArray ctor / ResetPath / GetPathData(i) [C] | high | 100 x 0x28 ring | `007e89e0`: `if ((param_2 < 0) \|\| (99 < param_2))` "PathFollowData requesting bad data position %d"; returns `param_2*0x28+param_1` |
| 0x7E8A70 | CSWPartyFollowDataArray::RecordLeaderPoint (proposed) [C partial] | med | appends the leader's position/heading to the ring; callers = the leader's own WalkUpdateLocation / QuickWalk / ForcePushed | callers `005c3460@005c3b1e, 005c3f70@005c4e51, 005c5ef0@005c620c`; ring head `+0xFA4` write |
| 0x7E9CC0 / 0x7EAB10 / 0x7EAC20 / 0x7EAD30 / 0x7EB050 | SmoothFollowPath / FindClosest / FindFurthest / PointOnLineDistanceFromLeader / GetPointBackFromStart [M] | med | mapper vote rows, bodies not read | AI0 rows |
| 0x5FAAF0 | CSWPartyTable::GetNPCObject(idx, create, ?) [C] | high | returns `tbl+0x1C+idx*4`, or creates a creature (`new(0x1220)` + ctor) with flag 0 | `005faaf0` decomp; no `0x58A330` call |
| 0x5FAFD0 / 0x5FB070 | CSWPartyTable::GetIsMember(npcIdx) / GetIsMember_2(objectId) [C] | high | membership tests | decomp (see 2.4) |
| 0x5F8D80 / 0x5F8B00 | CSWPartyTable::AddMember(npcIdx, objId) / RemoveMember(npcIdx) [C] | high | AddMember: refuses when `*param_1 == 2` (max 2), sets `tbl[7+idx]=objId`, flag (1,1) | decomp l.8-33 |
| 0x5F8960 | CSWPartyTable::TransferInventory(creature) [C] | med | flag cleared with apply=0, personal gold/component/chemical moved to the shared repository, flag restored apply=0 | `005f89a5 CALL 0x58a330`, `005f8a0e CALL 0x58a330`, `PUSH 0x0 / PUSH 0x0` before 1st |
| 0x5F9220 | CSWPartyTable::CreateParty [C partial] | high | per member: `SpawnNPC` then (1,1); puppets: `+0x46C=1`, `0x58A470(1)` | decomp l.90-100 `FUN_0058a330(1,1)` |
| 0x5FD480 | CSWPartyTable::SpawnNPC(idx, useGiven, pos, ori, flag) [C] | high | places the NPC object; **no flag write** | decomp (no 0x58A330 call) |
| 0x5FE3B0 / 0x5FE410 / 0x5FE470 | GetIsLeader(id) / SetLeader(id) / SetPartyFollowState [C] | high | `tbl+0x114` = leader NPC idx; `tbl+0x110 := 0` | decomp |
| 0x5FE4B0 | **CSWPartyTable::SwitchPlayerCharacter(npcIdx)** (DB name MakePlayerCharacter is the wrong class) [C] | high | `this` indexes the table (`param_1[0x46]`=`+0x118`, `param_1[param_1[0x46]+7]`); saves old PC, rebuilds, (1,1) | decomp l.72-81; only caller 0x694400 = cmd 11 |
| 0x5FEEF0 | CSWPartyTable::SetSoloMode(flag, apply) [C] | high | `+0x238 = flag` (0x5FED30 is SetPlanetAvailable, mapper collision) | decomp 5feef0; cmd 753 handler `0x699550 -> 5feef0` |
| 0x696250 / 0x79ED60 / 0x797F30 | script SetPartyLeader (cmd 13) / deferred leader switch (MainLoop) / UI ChangeCharacter [C] | high | leader changes (see 3) | `00781cbf CALL 0x79ed60`; `00797fb9 CALL 0x7e8350` |
| 0x66C990 / 0x690000 / 0x69B840 / 0x68BBE0 / 0x696CC0 | cmd 574 AddPartyMember / 575 RemovePartyMember / 846 RemoveNPCFromPartyToBase / 576 IsObjectPartyMember / 729 SetFormation [C] | high | table addresses verified | InitCommands stores |
| 0x66B640 / 0x66B770 / 0x66B8A0 / 0x669020 | cmd 834 SetCreatureAILevel / cmd 850 ChangeObjectAppearance / cmd 847 CreatureFlourishWeapon / RunScriptCallback-like [C] | high | **the mapper names "WalkUpdateLocation_QuickWalk_FollowLeader_*" are false** | `[+0xd48]=0x66b770`, `[+0xd3c]=0x66b8a0`; 0x669020 per AI0 (unchecked here) |
| 0x5D4B60 | script SetFormation worker: NPC group offsets from a 2DA row (`ANGLE_%d`, `DISTANCE_%d`), AddMoveToPoint + action 0x31 [C] | med | NOT the party formation | strings `0x99665c`/`0x996668`; sole caller `0x696cc0` |
| 0x775860 | CSWCCreature::SetInParty(flag) (client twin, `+0x3BC`) [C] | high | 1 -> `CSWParty::AddCharacter`; 0 -> `RemoveCharacter_2` | decomp |
| 0x599A10 | apply-table slot 0x164 (effect type 89) handler: creates a creature and flags it (1,1) [C] | med | gate `*CSWParty < 3` | `005943a3 MOV [EDX+0x164],0x599a10`; `00599aef CALL 0x58a330` |

## (b) Findings

### 1. FollowInfo struct: CONFIRMED (field names from the GFF loader 0x561D80, offsets from its stores)
`new(0x3C)` per party-flagged creature at `creature+0x500` (ctor `0x5752C0` + `0x575340`; load `0x575200`; save `0x5751B0`). Table in (e). Points:
- **CONFIRMED live use is one field**: `+0x38` InSafetyRange (`007e7838 MOV [EDX+0x38],0` exit, `007e78ae MOV [EDX+0x38],1` enter; read `007e76e8 MOV EDX,[ECX+0x38]`). `AIActionFollowLeader` uses `+0x500 != 0` as an extra gate (`005bf817 CMP [EBP-0xc],0 / 005bf81b JZ 0x5bf881`).
- **CONFIRMED dead**: `GetWalkRate 0x575DF0` and `GetRunRate 0x576100` read `+0x28` (MaxSpeed) and `+0x00` only behind `00575e04 XOR EAX,EAX / 00575e06 JZ 0x00575fd4` and `00576111 XOR EAX,EAX / 00576113 JZ 0x00576367` (lesson 4: `XOR r,r; JZ` is `if (0)`; Ghidra also removed the blocks). Walk/run rate is therefore never lowered by FollowInfo.
- **Never written at runtime (census over every `[reg+0x500]` creature access)**: only ctor, SetPartyMemberFlag, LoadFollowInfo, SaveFollowInfo, the two dead rate readers, FollowLeader (null test) and RangeMonitor (`+0x38`). So `FollowObject/FollowLocation/LastLeaderPos/LastFollowerPos/MaxSpeed/StickToPos/Result/TimeElapsed` keep their defaults (INVALID, 0, 0, 0, 0, 0, 2, 0) unless a save file carries other values. Absence claim over `field_census`, not proof (lesson 168); the pointer could be passed as an argument, but the only functions that receive it are the loader/saver.

### 2. `+0x11AC`: writers, readers, semantics, the right test
**2.1 Writers: CONFIRMED two.** `field_census --offset 0x11AC --all`: 291 hits / 200 functions, `{'R': 289, 'W': 2}`: `0056214c MOV [EDX+0x11ac],0` (ctor `0x561F30`) and `0058a35e MOV [EAX+0x11ac],ECX` (`0x58A330`, arg1 = new value, any non-zero; callers pass 0/1 only). No `LEA`/`RW`/`?` hits. Not serialized anywhere: `SaveCreature` never writes it (it restores it from the local snapshot `00574525 MOV [EBP-0x38],ECX`), the creature loader never sets it.

**2.2 The 15 call sites of `0x58A330` (all `thiscall`, `PUSH apply; PUSH flag; MOV ECX,creature`), 12 functions:**

| # | site | caller | args (flag, apply) | condition / meaning |
|---|---|---|---|---|
| 1 | `0x574532` | SaveCreature `0x574500` | (0, 1) | unconditional, first thing: snapshot `+0x11AC` to `[EBP-0x38]` (`0057451f`/`00574525`), then clears it (frees FollowInfo, level 2) |
| 2 | `0x57516B` | SaveCreature | (old, 1) | unconditional restore after the GFF is written, before `SaveFollowInfo` (`005751b0`): old=1 -> new FollowInfo with defaults + level 4; old=0 -> level 2 again |
| 3 | `0x599AEF` | apply-table slot 0x164 handler `0x599A10` | (1, 1) | creates a fresh `CSWSCreature` (`PUSH 0x1220`, ctor `0x561F30`); gate `*CSWParty < 3` and `param_2 != 0` (decomp l.46); runs its spawn slot. Identity of effect 89 not established |
| 4 | `0x5F89A5` | TransferInventory `0x5F8960` | (0, 0) | `if (param_2 != 0)`: clear flag WITHOUT apply to read personal gold/component/chemical |
| 5 | `0x5F8A0E` | TransferInventory | (old, 0) | restore, no FollowInfo/level side effects |
| 6 | `0x5F8EB4` | AddMember `0x5F8D80` | (1, 1) | `*tbl != 2` (max 2 members), NPC available, server creature exists (`0x51C100`), not already member, no index clash; then TransferInventory + `0x58B5A0` |
| 7 | `0x5F93AF` | CreateParty `0x5F9220` | (1, 1) | for each `tbl[2+i]`: `SpawnNPC` then flag (`if (local_50 != 0)`); puppets use `0x58A470(1)` |
| 8 | `0x5FEAC2` | SwitchPlayerCharacter `0x5FE4B0` | (1, 1) | the newly created PC creature after `ClearParty 0x7E6C80` |
| 9 | `0x6306A0` | `0x6305E0` ("TEMP_CHAR") | (1, 1) | chargen PC (string `0x998320`) |
| 10 | `0x630B12` | `0x630810` (strings "AreaId", "Mod_PlayerList") | (1, 1) | player creature loaded from a player-list GFF (name by strings only, med) |
| 11 | `0x66CA55` | cmd 574 AddPartyMember `0x66C990` | (1, 1) | only `if (AddMember(...) != 0)`: so NOT when the party already has 2 members or the NPC is unavailable |
| 12 | `0x6900BC` | cmd 575 RemovePartyMember `0x690000` | (0, 1) | after `RemoveMember`; creature object stays in the world, level 2 |
| 13 | `0x69B8FC` | cmd 846 RemoveNPCFromPartyToBase `0x69B840` | (0, 1) | same, then `KillNPCObject` (`0x5FD9A0`) |
| 14 | `0x89FE40` | party-select Accept `0x89FBD0` | (0, 1) | member de-selected in the GUI (`if (local_2c != 0)` after client `0x775860(0)`) |
| 15 | `0x8A0509` | party-select Accept | (1, 1) | newly selected NPC: `SpawnNPC`, `ClearAllActions(1)`, reset PathInfo (`0x6F9F70`), flag, `AddMember` earlier |

**2.3 When is it 0 for a companion: CONFIRMED (code) and matches the 02:38 runtime sample.** 0 for every NPC creature that was never passed through sites 6/7/11/15 (or 15 was undone by 12-14): (a) the standees on the Ebon Hawk and everywhere else: `SpawnAvailableNPC` = cmd 698 `0x693620 -> 0x5FD480 SpawnNPC` (no flag, `callers` of `0x58A330` do not include `0x5FD480`/`0x5FAAF0`/`0x693620`); (b) module-spawned companion templates (ctor `0x56214C`); (c) NPCs reloaded from a GFF (`LoadFollowInfo` can allocate `+0x500` but never sets `+0x11AC`); (d) members dismissed by 12-14; (e) transiently 0 inside SaveCreature/TransferInventory (single thread, invisible to other code). **A companion standing on the Hawk and not among the 2 selected followers has 0.** The PC has 1 (sites 8-10). A DLL that sees 0 on atton/t3m4/kreia/droid in that launch is seeing the engine's truth; if a SELECTED follower ever reads 0, log `[ESP]` (return address) of `0x58A35E` (H1) to see which site wrote it.

**2.4 Semantics vs "in the active party": CONFIRMED.** Flag != 0 <=> PC or an NPC currently in `CSWPartyTable` members (max 2, `tbl[0]`) [plus an effect-89 creature]. It is cached per creature and set together with the client twin (`CSWCCreature+0x3BC` via `0x775860`, which also adds the creature to `CSWParty`, max 3 slots). `IsObjectPartyMember` (cmd 576) is literally `return creature+0x11AC` (`0068bc2e MOV EDX,[ECX+0x11ac]`).

**2.5 Correct party test for a DLL.**
- Hook inside a server creature function (creature pointer in hand): `*(int*)(creature+0x11AC) != 0` (the engine's own test; 200 functions do exactly that). Add `creature+0x500 != 0` only if you want the AIActionFollowLeader gate.
- Server object id -> membership: `CSWPartyTable::GetIsMember_2 0x5FB070` (thiscall ECX = table, `PUSH objId`, `ret 4`): true iff the id is in `tbl[7..18]` (`tbl+0x1C+npcIdx*4`) and that npcIdx is in `tbl[2..]` (`tbl[0]` entries). Table address = `0x51C8D0(ECX = serverApp)` with `serverApp = *(*(void**)0xA1B4A4 + 8)`: `return *(serverApp+4) + 0x1F0B4` (`0051c8da MOV EAX,[EAX+0x4] / 0051c8dd ADD EAX,0x1f0b4`), i.e. the table is an INLINE object at `*(int*)(serverApp+4)+0x1F0B4`, not a pointer to dereference. Object id -> creature: `0x51C8B0(id)` then `0x51C100(obj)` (thiscall ECX = serverApp, `ret 4` each; pattern at `007e5730`). PC id: `0x51C8F0`. Leader NPC index: `tbl+0x114` (-1 = PC; `GetIsLeader 0x5FE3B0`).
- Client side / "is it one of the 3 on screen": `CSWParty::IsPartyMember 0x7E5C40` (thiscall ECX = party, `PUSH objId`, `ret 4`), party = `0x73FB90(ECX = clientApp)` with `clientApp = *(*(void**)0xA1B4A4 + 4)`: `0073fb9c MOV ECX,[EAX+0x4] / 0073fb9f MOV EDX,[ECX+0x270]`, i.e. `party = *(void**)(*(int*)(clientApp+4)+0x270)` (a heap object, `new(0x1290)` at `00780c21 PUSH 0x1290`, ctor `0x7E5180`); slot id `*(party+0x24+slot*0x88)`, count `*(int*)party`, slot index `0x7E6BE0(id)` (-1 = none), `0x7E5DA0(slot)` = client creature (`+4` = object id).
- Prefer the per-creature flag in hot paths (one load); use the table/party lookups only from code that has just an object id. **Do not use `+0x220`** (player update-set flag, GATE) or `+0x10E8` (IsPC).

**2.6 Readers (289 hits): classified by purpose, not enumerated one by one.** Full list = `field_census --offset 0x11AC --all`. Mod-relevant families:
- AI level: `AIUpdate 0x569320` promotes to 4 (`0056948e CMP [stats+0x6c],1 / 005694a9 CMP [+0x11ac],0 / 005694c1 CMP [+0x78],4 / 005694d6 CALL 0x51e120`), also `0x569343` (clears `[0xA7E098]`), `0x5693fb` (positions), `0x56964a` (off-mesh rescue: when the height query fails (`local_5c==0`) and flag != 0 and not the PC, the creature is snapped to its slot target `E+0x18`, `l.99-128`), `0x569ac4/0x569b10` (HP/KO tests near the slot-6 fallback, not read in detail).
- Follow: RunActions case 0x3D `0x5D5C9C..0x5D5D13` (leader id `0x73F470`==INVALID -> 1; flag of the leader and of `self`), QuickWalk `0x5C3FFC`, `0x5BF7F0`, `0x5BF940`, `0x5BCDC0`, `0x5C3460 WalkUpdateLocation`, ClearAllActions `0x5410CC/0x5412EE`, ClearAction `0x56EC4A`, bump rules `0x586940` (leader may bump flagged or owner-flagged creatures) / `0x586B70` (FM1).
- Combat/perception: `0x56B8A0` party skip (P1/P2), `0x5D0F30 UpdatePerception`, `0x57C990 GetNearestVisibleEnemyInRange`, `0x57A560 SetCombatState`, `0x57AC30 BroadcastCombatStateToParty`, `0x58E990 EndCombatRound`, `0x568540 AddAttackActions`, `0x6D7390 AIActionPhysicalAttacks`, `0x5D0D00 GetCreatureReputation`, `0x575B20 CalculateDeathExperience`, `0x5724D0`, `0x59xxxx` effect handlers, `0x56EFD0 EventHandler`.
- Shared inventory/economy: `GetGold/SetGold/GetComponent/SetComponent/GetChemical/SetChemical` (`0x5812E0..0x581840`), `0x58A2C0 GetItemRepository`, `RemoveItem 0x5BDE40`, `InstantUseItem`, `UseItem`.
- Script commands: IsObjectPartyMember, ActionFollowLeader (`0x669FEF`), GiveXP/Gold, GetNearestObject, SetPartyLeader (`0x696343/0x696384`), CreateItemOnObject, AdjustAlignment, ...
- Client: portraits, target menu, `ChangeCharacter 0x797F30`, `OnObjectSelected`, hover actions: all via the server object.

### 3. Leader switch (UI and script): CONFIRMED
Common worker `CSWParty::SetLeaderBySlot 0x7E5DF0(slot)`: is a no-op unless `count >= 2` and the argument is non-zero (`if ((1 < *param_1) && (param_2 != 0))`); accepts `-3` (first slot whose `stats+0x6C != 0`, i.e. the PC), `-2` (slot 1), or `1..count-1`; other values leave the target INVALID and nothing happens. It
1. rotates the 3 ids (and the per-slot dword `E+0x80`) until the target is at slot 0 (`while (param_1[9] != local_40)` in the decompile); the old leader goes to the end of the rotation;
2. calls server `CSWPartyTable::SetLeader 0x5FE410(newId)` (`tbl+0x114`) and `0x632830(creature)` (controlled creature / player update set);
3. rewrites perception ranges `+0x101C/+0x1020` of the new leader (ranges.2da row 0xC = PC) and of the old one (row 0xB) and swaps the client effect-icon lists;
4. if the leader id changed: `RunScript("k_hen_leadchng" (0x9A2178), newLeaderId, 1)` (`007e6535 MOV ECX,[0xa1b4a8]`, `007e653b CALL 0x6fd8d0`);
5. sets the **FM6 hold flag in all 3 slots** (`007e6546/007e6556/007e6566`, `E+0x70 := 1`).
It does not touch FollowInfo (`+0x500`) or any AI level.

Entry points:
- **UI** `CClientExoAppInternal::ChangeCharacter 0x797F30(slot, flag)`: `SetLeaderBySlot`; if the leader changed: **`0x7E8350 ResetAllFollowData(leaderPos, heading)`** (`00797fb9`): ring refilled with the leader position (count 0, head 0), every slot's state := -1 and targets := leader position (`+0x28/+0xB0/+0x138 := -1`, `+0x3C..` vec3 := pos), per-creature `PI+0x210 := 0`, `PI+0x34 := INVALID`, walkmesh snap (`0x51CD90/0x51CDB0`); then on the new leader `PI+0x34 := INVALID` and `ClearAction(0x3D)` (`0x58AC50`; the leader's server creature comes from `0x77D800`).
- **Script** `SetPartyLeader` (cmd 13) `0x696250`: looks up the NPC's client id, loops slots, `SetLeaderBySlot(slot)` (or `-3` for the PC when `-1`); **no ResetAllFollowData**. **Deferred** `0x79ED60` (per frame from MainLoop `00781cbf`): `SetLeaderBySlot(idx)` (or re-assert for slot 0), swaps the effect-icon lists, clears the pending id; **no reset**. **Party select** `0x89FBD0`: `SetLeaderBySlot(-3)` first (`FUN_007e5df0(0xfffffffd)`), then `RecalcFollowPoint 0x7E8720`, no ResetAllFollowData.
- **AI level**: never changed by a leader switch (all flagged creatures are already level 4: `0058a3ef`/`005694d6`). Level 2 only on removal (sites 12-14) and on every SaveCreature of non-party creatures (see 5/FM4).
- **SwitchPlayerCharacter** (cmd 11) rebuilds the party (save old PC, `ClearParty`, new creature, flag).

### 4. Formation slots: how a follower's target is computed: CONFIRMED (structure), med (speed constants)
Two coordinate systems exist; only the first drives walking.
1. **Trail + leader-local offset (drives follow).** The leader's own movement appends points to the ring `P+0x2DC` (`RecordLeaderPoint 0x7E8A70`, 100 entries x 0x28 B: pos vec3, heading `+0xC`, head `P+0x1280`, count `P+0x1284`). `RecalcFollowPoint 0x7E8720` (called per frame from `Update`, from `TestAndResetPath`, from state 0 and from party select) sets the slot target `E+0x28` (vec3, `E+0x34` = node id) = the trail point at distance `FollowRange - P+0x1C0` (slot 1) / `FollowRange + P+0x1C0` (slot 2) behind the leader (`0x7EB050 GetPointBackFromStart`). `FollowRange = 0x7E7D70` = ranges.2da `PrimaryRange` row 0xE (row 0xF when `tbl+0x110 == 1`; `_DAT_00986b68` = 5.0 default; the 2DA is not on disk here: open item). State 0 (`0x5C7370`) then builds the world target: `target = trailPoint(E+0x28) + R_z(heading(node E+0x34)) * (E+0x38,0x3C,0x40)` and clamps it to the walkmesh (`0x5453C0/0x54B130`), writes it to `E+0x18`, queues action 0x40 (radius check) + MoveToPoint (run) and goes to state 6/7.
2. **Formation presets (the offsets).** `P+0x1BC` = current preset (init -10000 then `ApplyLeaderFormation(0)` from the ctor; **no other caller exists**, so vanilla always uses preset 0), `P+0x1C0` = spacing, `P+0x1C4` = mirror flag, presets at `P+0x1C8 + k*0x24` (slot-1 offset, slot-2 offset, third vec unused), spacing `P+0x234 + k*4`. Ctor values (floats read from the exe): preset 0 = slot 1 `(1.5, -0.7, 0)`, slot 2 `(-1.5, 0.8, 0)`, spacing 1.5; preset 1 = all zero; preset 2 = slot 1 `(-2, 1.5, 0)`, slot 2 `(2, 4.5, 0)`, spacing 1.5. Active offsets copied to `P+0xE4` (=`E1+0x38`) and `P+0x16C` (=`E2+0x38`). `TestAndResetPath 0x7E7900` mirrors x (`FlipLeaderFormationOffsets`) when the leader's newest heading points against the closest follower's `E+0x4C` vector (dot < 0, `[0x986b38]`=0.0) and moves slots 1/2 (not in states -1/0/5/6/7) to state 4.
3. `GetWorldFormationPosition 0x7E6960` = `leaderPos + R(leaderOri) * (P+0x5C+slot*0x88)` is used only by the party-select screen to place a newly selected NPC (and only when the slot has no trail target yet, `0x89FBD0` l.143-150); otherwise the same trail + offset formula as above (`0x89FBD0` l.151-190).
4. Speed: `0x5C6340` chooses walk/run from the distance to the target (`16.0` at `[0x9946D0]`, `100.0` at `[0x98BA38]`, `225.0` at `[0x996528]`) and the leader's rate (`GetRunRate 0x576100` / `GetWalkRate 0x575DF0` of the follower, leader `+0x3C8`): low confidence on the exact thresholds.

State machine of `E+4` (slot element, `0x5C6340` switch `005c6ace JMP [ECX*4+0x5c7334]`): -1 reset (`E+0x5C := creature pos`, -> 5), 0 place (`0x5C7370`), 1 walk trail (`0x5C7AD0`), 2 blocked (`0x5C87E0`), 3 offset walk (`0x5C8C20`), 4 to trail end (`0x5CA000`), 5 hold/re-plan (`0x5CA6A0`), 6-9 wait for the trail to advance (`0x5CACD0/5CAE00/5CAF30/5CB060` -> 1/3/4/2), 10 skip to walkable node (`0x5CB190`). Loop guard: `005c6cd4 CMP [EDX+0x44],0x32 / 005c6ce2 MOV [ECX+4],-1` (more than 50 loop iterations without a wait state resets the slot to -1). `ClearAllActions` resets the creature's own slot (`0054137d MOV [EAX+EDX+0x28],-1`).

### 5. FM6 consumer of `E+0x70`: CONFIRMED consumer, FM6 PARTLY REFUTED
Consumer `0x5CA6A0` (state 5, `this`=creature, `[EBP+8]`=slot element E): `005ca7c6 MOV ECX,[EAX+0x70]` / `005ca7cf CMP [EBP-0xe0],0` / `005ca7d6 JZ 0x5ca7ff` / `005ca7e8 FCOMP double [0x00996520]` (49.0) / `005ca7f3 JP 0x5ca7ff` / `005ca7f5 MOV EAX,[0x9944b8]` (return 2, no re-plan) ... `005ca802 MOV [ECX+0x70],0` (re-plan path). So after any `SetLeaderBySlot` every idle (state 5) follower stays put while it is within 7 m (squared 49) of the new leader; farther ones clear the flag and re-plan (states 4 / 2 / 0 by the `0x550980` line tests). Initial value 0 (`0x7E5610 param_1[0x1c]=0`); zeroed only there. Only writers of the flag: `0x7E6546/6556/6566` (set), `0x5CA802` (clear), `0x7E5610` (init). Residual FM6: slots that are NOT in state 5 when the script/deferred/party-select path rotates the ids keep their `E+4` state, `E+0x18` target, `E+0x44` counter (no `ResetAllFollowData`), and the trail still holds the old leader's points (the new leader appends its own); effect is bounded (state 6-9 waits end when the trail advances; the loop guard resets to -1 after 50 iterations). No evidence of a visible bug: severity low.

### 6. FollowLeader / FollowOwner / CheckMoveToFollowRadius semantics: CONFIRMED
- `0x3D` returns 3 (action removed) when: flag == 0 or `+0x500 == 0` (`005bf809`, `005bf81b`), `vt+0x94() == 1`, flagged and HP <= 0 (`vt+0x9C(0)`), `+0x380 == 0`, `(+0x1114 & 2) != 2` (meaning of the bit unknown), or the leader id (`0x73FB90 +0x24` -> `0x51C8B0`) equals self (`005bf916 CMP EAX,[EBP-0x20] / JNZ`; the new leader drops its own 0x3D). It returns 1 (stays queued) with `+0x11B0 := 3` otherwise, and 1 without setting it while `0x5453C0` (area) is null.
- RunActions gate (case 0x3D, `0x5D5C5B..0x5D5D35`): controlled/leader id (`0x73F470`) INVALID -> returns 1; the acting object must be a creature (`005d5ccb CMP [EBP-0x10],5`) with `+0x11AC != 0` (`005d5d07`/`005d5d13 CMP [EBP+0xfffffee4],0 / JZ 0x5d5d3d`) before `005d5d35 CALL 0x5BF7F0`; the first `+0x11AC` read at `005d5cbf` is stored but not used for the branch. Otherwise it jumps to `0x5D688E` with the result variable untouched (initial value not traced).
- Engine-side queueing of 0x3D: `PUSH 0x3d` scan finds two sites: `007e5a69` (AddCharacter, slot > 0) and `0066a04b` (cmd 730 ActionFollowLeader `0x669FC0`, requires flag and `+0xEC == 1`). Absence is not proof (a register-pushed type would be missed). Consequence (FM1): `ClearAllActions` / `ClearAction(0x3D)` on a follower leaves it idle (QuickWalk's `+0x11B0==3` test fails, `ClearAllActions` writes `+0x11B0 := 1` at `0x541210`) until a script re-queues it.
- FollowOwner: cmd 0x46 `0x69B660` requires `+0x11FC != 0`.

### 7. 5 m edges and the "retreat" script (`CSWParty::Update 0x7E6CE0`): CONFIRMED
Per frame, for slots 1..count-1 with a server creature and `+0x500 != 0`: `d2 = |leaderClientPos - slotClientPos|^2`; `inside = d2 < 25.0` (`007e77ab FCOMP [0x989ed0]` f64 25.0). `!inside && FollowInfo+0x38 != 0` -> run `k_hen_exit5m` on that creature and `+0x38 := 0` (`007e7838`); `inside && +0x38 == 0` -> `k_hen_enter5m`, `+0x38 := 1` (`007e78ae`). Retreat: `timer P+0x2BC += dt`; if `timer > T` and `d2 > R^2` where (R, T) = ranges.2da row 0x10 (`007e760b PUSH 0x10`; `[CSWRules+0x130]+0xBC` table) -> run `k_hen_retreat` on that creature and `timer := 0` (`local_58`). Strings: `0x9A2178 k_hen_leadchng`, `0x9A2188 k_hen_enter5m`, `0x9A2198 k_hen_exit5m`, `0x9A21A8 k_hen_retreat`. Edge flags are reset to 1 by every SaveCreature (FM4: FollowInfo recreated), so `exit5m` can re-fire after a save for a companion farther than 5 m.

### 8. FM4 (SaveCreature) extension: CONFIRMED
`SaveCreature` calls `0x58A330(0,1)` and `0x58A330(old,1)` unconditionally (`00574528 6A 01 / 0057452A 6A 00 / 00574532 CALL`, `0057515F 6A 01 / 00575164 PUSH EAX / 0057516B CALL`). For `old == 0` (every non-party creature saved, e.g. all creatures of an area in `CSWSArea::SaveCreatures 0x52A600`) both calls take the `flag==0, apply!=0` branch: `if (+0x78 != 2) SetAILevel(2)` (`0058a448 PUSH 0x2 ... 0058a458 CALL 0x51e120`). The AI0/review one-byte fix (`0x574529: 01 -> 00`) leaves the second call (`apply=1`), so non-party creatures still get level 2 and party members still go through `SetAILevel(4)` (a no-op if already 4). **Two bytes** `0x574529` and `0x575160` (`01 -> 00`) make both calls pure flag writes; the FollowInfo then also survives and the saved "FollowInfo" block carries the live values instead of defaults (a save-format change: values like `Result` stay 2/`InSafetyRange` real). Shipped companion-ai hooks sit at `0x57451F` (6 B) and `0x575183` (7 B): the byte at `0x574529` is 10 B from the first hook start (4 B after its end), `0x575160` is 35 B before the second.

### 9. Mapper / DB name decisions
- `0x5FE4B0` = `CSWPartyTable::SwitchPlayerCharacter` (mapper right, DB `CSWSCreature::MakePlayerCharacter` wrong class): `this` is the table (`param_1[0x46]`, `FUN_005f7f30(param_1[0x46],0)`).
- `0x66B770` = cmd 850 ChangeObjectAppearance, `0x66B8A0` = cmd 847 CreatureFlourishWeapon, `0x66B640` = cmd 834 SetCreatureAILevel: the "WalkUpdateLocation_QuickWalk_FollowLeader_*" names for them are false (InitCommands `006674c8`-style stores `[+0xd48]=0x66b770`, `[+0xd3c]=0x66b8a0`, `[+0xd08]=0x66b640`). The real follow handlers are `0x5C7370..0x5CB190` (states 0-10).
- `0x5FEEF0` = SetSoloMode (not `0x5FED30`/`0x5FED60`).
- `0x7E6CE0` is `CSWParty::Update` (RangeMonitor is a sub-behaviour).

## (c) Runtime probes
| id | what | hook | expected signal |
|---|---|---|---|
| P1 | who writes `+0x11AC` | H1 `0x58A35E` (6 B `89 88 AC 11 00 00`), log creature id, old `[EAX+0x11AC]`, new `ECX`, apply `[EBP+0xC]`, return address `[EBP+4]` | return addresses from the 15 sites (table 2.2); a Hawk standee must never log a 1 |
| P2 | follower state transitions | H7 `0x5C6CD4` (stuck reset) + H8 `0x5BD09E` (forced state 5): log `E+4` before, creature id | rare; confirms the 50-iteration guard / follow-radius re-plans (E4/E7) |
| P3 | FM6 flag lifetime | H2 `0x7E6546` + H3 `0x5CA7C3`: log slot ids, `E+4`, `E+0x70`, dist² to leader | after a script `SetPartyLeader`, idle followers within 7 m show `E+0x70==1` and no re-plan; beyond 7 m flag cleared |
| P4 | 5 m edges | H4a `0x7E7838`, H4b `0x7E78AE`: log creature `[EBP-0x38]`, id `[EBP-0x3C]`, `d2` | one exit per departure, one enter per return; a pair right after a save (FM4) |
| P5 | save side effects | log `+0x78` (AI level) and `+0x500` before `0x574532` and after `0x57516B` for party and non-party creatures | non-party `+0x78` becomes 2 after every save; party FollowInfo pointer changes |
| P6 | follow engaged | H5 `0x5BF925` (hot): count per creature | counts ticks where `0x3D` keeps the follower in follow mode; 0 after `ClearAllActions` until re-queued |
| P7 | follow range / 2DA | log `0x7E7D70` return and row 0xE/0xF/0x10 values | settles PrimaryRange/retreat numbers (2DA not on disk) |

## (d) Open questions
1. ranges.2da rows 0xB/0xC (perception), 0xE/0xF (follow range), 0x10 (retreat R,T): values not on disk; probe P7.
2. `E+0x80` (`P+0xA4+slot*0x88`) is written from the PC creature's `+0x548` in `SetLeaderBySlot` (l.57-62); meaning unknown. `E+0x08` = 1.0f, `E+0x44` loop counter, `E+0x48` speed, `E+0x58/+0x5C/+0x60/+0x64/+0x68/+0x6C/+0x74..+0x7C` partly identified (see e).
3. Identity of effect type 89 (apply slot `0x164`, handler `0x599A10` creates a party-flagged creature); `0x599A10` has no direct callers (vtable table entry at `0x5943A9`).
4. `+0x1114` word bit 1 (tested by AIActionFollowLeader).
5. States 1-4 in detail (`0x5C7AD0`, `0x5C87E0`, `0x5C8C20`, `0x5CA000`): transitions only; `SmoothFollowPath 0x7E9CC0` and the body of `RecordLeaderPoint 0x7E8A70` (4.7 KB) not read.
6. Leader-vs-self check `005bf916`: the exact object (`0x51C8B0` of party slot 0 id) vs `+0x4`; semantics fine, but reading id vs pointer not distinguished.
7. Whether `CClientExoAppInternal+0x338` (deferred leader id) is written only by `0x798660` (not read here).
8. `0x669020` AI0 claims RunScriptCallback; not re-verified.

## (e) Struct field tables
**CFollowInfo** (`creature+0x500`, `new(0x3C)`)
| class | offset | size | type | proposed name | readers (addr) | writers (addr) | confidence | evidence |
|---|---|---|---|---|---|---|---|---|
| CFollowInfo | 0x00 | 4 | objid | FollowObject (default 0x7f000000) | 0x575E14 (dead block) | ctor 0x575340, loader 0x561D80 | high | `FUN_00718c00(param_3,"FollowObject",...)`; `*param_1 = 0x7f000000` |
| CFollowInfo | 0x04 | 12 | vec3 | FollowLocation | save only | loader | high | `"FollowLocation"` -> `param_1[1..3]` |
| CFollowInfo | 0x10 | 12 | vec3 | LastLeaderPos | save only | loader | high | `"LastLeaderPos"` -> `param_1[4..6]` |
| CFollowInfo | 0x1C | 12 | vec3 | LastFollowerPos | save only | loader | high | `"LastFollowerPos"` -> `param_1[7..9]` |
| CFollowInfo | 0x28 | 4 | float | MaxSpeed | 0x575EC5, 0x576128, 0x576197 (all behind `XOR EAX,EAX; JZ`) | loader | high | `00576128 FLD [EAX+0x28]` dead block |
| CFollowInfo | 0x2C | 1 | byte | StickToPos | none | loader | med | `"StickToPos"` -> `param_1[0xb]&0xff` |
| CFollowInfo | 0x30 | 4 | int | Result (default 2) | none | ctor `param_1[0xc]=DAT_009944b8`, loader | high | `"Result"` |
| CFollowInfo | 0x34 | 4 | int | TimeElapsed | none | loader | med | `"TimeElapsed"` |
| CFollowInfo | 0x38 | 1 | byte | InSafetyRange (default 1) | 0x7E76E8 | 0x7E7838 (:=0), 0x7E78AE (:=1), ctor, loader | high | `007e7838 C7 42 38 00 00 00 00` / `007e78ae C7 42 38 01 00 00 00` |

**CSWSCreature (additions to the census list)**
| class | offset | size | type | proposed name | readers (addr) | writers (addr) | confidence | evidence |
|---|---|---|---|---|---|---|---|---|
| CSWSCreature | 0x11AC | 4 | int | PartyMemberFlag | 289 hits / 200 fns | 0x56214C (0), 0x58A35E | high | quoted |
| CSWSCreature | 0x500 | 4 | ptr | pFollowInfo | 0x575xxx, 0x5BF81B, 0x7E76BA/76D6/7826/789C | 0x561F30 (0), 0x58A3C5 (new), 0x58A424 (0), 0x57528E (load), dtor 0x563AC7 | high | census `--offset 0x500` |
| CSWSCreature | 0x11B0 | 4 | int | walkState (1 idle, 2 move-to-point, 3 follow-leader, -1 none, 0/4/5/6 other) | AIUpdate 0x569823, QuickWalk 0x5C3FC3/0x5C4136, ... | 0x5BF925 (:=3), 0x5644B0/0x564770 (:=2), ClearAllActions 0x541210 (:=1), ClearAction 0x56E837, ctor (-1), 0x5A7BF0 (6), 0x5A7E3C (5), 0x5A8339 (4) | med | census `--offset 0x11b0`; only 1/2/3 verified |
| CSWSCreature | 0x11FC | 4 | int | PuppetOwnerFlag | 0x5BF940, 0x586940 | 0x58A470 | med | decomp 58a470 |
| CSWSCreature | 0x1114 | 2 | word | flags (bit1 tested by 0x3D) | 0x5BF89E | ? | low | `005bf89e MOVZX EAX,[EAX+0x1114]` / `AND EAX,2` |

**CSWParty (client, `P` = `*(*(clientApp+4)+0x270)`, ctor 0x7E5180)**
| class | offset | size | type | proposed name | readers | writers | confidence | evidence |
|---|---|---|---|---|---|---|---|---|
| CSWParty | 0x00 | 4 | int | memberCount (max 3) | 7E5C40, 7E5DA0, 7E6CE0 | 7E5730 (+1), 7E4xx RemoveCharacter, ctor | high | `if (*param_1 <= local_8) return 0` |
| CSWParty | 0x18 | 4 | int | dirty flag (set by AddCharacter) | ? | 0x7E5730 `param_1[6]=1` | low | decomp |
| CSWParty | 0x24 + s*0x88 | 3 x 0x88 | slot element E | slots[3] | all | all | high | `007e51c2 ADD ECX,0x24`, `007e51db ADD EAX,0x88` |
| CSWParty | 0xE4 / 0x16C | 12 | vec3 | slot-1 / slot-2 formation offset (= E1/E2 `+0x38`) | 0x5C7370, 0x5CA6A0, 0x7E6960 | 0x7E6720, 0x7E6670 | high | `7e6720` l.35-60 |
| CSWParty | 0x1BC | 4 | int | formation preset (init -10000) | 0x7E6720 | 0x7E6720 | high | decomp |
| CSWParty | 0x1C0 | 4 | float | formation spacing | 0x7E8720 | 0x7E6720 (from `P+0x234+k*4` / `0x230-k*4`) | high | `*(float*)(param_1+0x1c0)` |
| CSWParty | 0x1C4 | 1 | byte | mirror flag | 0x7E6720 | 0x7E6670 | high | decomp |
| CSWParty | 0x1C8 | 0x6C | 3 x (3 vec3) | preset table (slot1, slot2, unused) | 0x7E6720 | ctor 0x7E5180 | high | ctor `param_1[0x72..0x89]` |
| CSWParty | 0x234 | 12 | float[3] | preset spacing (1.5, 1.5, 1.5) | 0x7E6720 | ctor | high | `param_1[0x8d..0x8f]` |
| CSWParty | 0x2BC | 4 | float | retreat timer | 0x7E6CE0 | 0x7E6CE0 (`+= dt`, 0 on retreat) | med | `param_1[0xaf]` |
| CSWParty | 0x2C0 / 0x2CC | 12 / 16 | vec3 / quat | smoothed leader point / orientation | 0x7E6CE0 | 0x7E6CE0 | low | `param_1[0xb0..0xb6]` |
| CSWParty | 0x2DC | 0xFA0 + 12 | CSWPartyFollowDataArray | 100 x 0x28 ring; `+0xFA0` back pointer to P, `+0xFA4` head (P+0x1280), `+0xFA8` count (P+0x1284) | 0x7E7900, 0x5CACD0.., 0x7EAB10 | 0x7E8A70, 0x7E8970 | high | `007e89e0`, `007e8970 +0xfa8/+0xfa4`, `param_1[0x49f]=param_1` |

**CSWParty slot element E (stride 0x88, `E_s = P+0x24+s*0x88`)**
| class | offset | size | type | proposed name | readers | writers | confidence | evidence |
|---|---|---|---|---|---|---|---|---|
| E | 0x00 | 4 | objid | client object id (INVALID 0x7f000000) | everything | 0x7E5730, 0x7E5DF0 (rotation) | high | `007e5c40` |
| E | 0x04 | 4 | int | follow state -1..10 | 0x5C6340 switch, 0x7E7900 | all state handlers, 0x7E8350 (-1), 0x7E8640, 0x541380 (-1), 0x5BD09E (5), 0x5C6CE2 (-1) | high | quoted |
| E | 0x18 | 12 | vec3 | resolved world target of the follower | 0x569xxx rescue, 0x5CA..., | 0x5C7370, 0x5CA6A0, 0x5CB190 | high | decomp 7370 `*(param_2+0x18)=local_30` |
| E | 0x24 | 4 | int | trail node of `E+0x18` | 0x5CACD0.. | 0x5C7370, 0x5CA6A0, 0x5CB190 | med | `param_2+0x24` |
| E | 0x28 | 12 (+4) | vec3 + node | trail point at follow distance, node id `+0x34` | 0x5C7370, 0x89FBD0 | 0x7E8720, 0x7E8350, 0x5CA6A0 | high | `7e8720` stores `+0xD4/+0x15C` |
| E | 0x38 | 12 | vec3 | leader-local formation offset | 0x5C7370, 0x5CA6A0, 0x7E6960, 0x89FBD0 | 0x7E6720, 0x7E6670 | high | quoted |
| E | 0x44 | 4 | int | loop counter (guard 0x32) | 0x5C6340 | 0x5C6340, 0x5C7370 (0), states 6-10 (0) | high | `005c6cd4` |
| E | 0x48 | 4 | float | current follow speed | 0x5C6340 | 0x5C6340 tail | med | `*(float*)(local_7c+0x48)=local_28c` |
| E | 0x4C | 12 | vec3 | follower direction vector (compared with the leader heading) | 0x7E7900 | 0x5C6340 tail | low | `7e7ba3 LEA [EDX+ECX+0x70]` (=P-relative `E+0x4C`) |
| E | 0x5C | 12 | vec3 | creature position snapshot (reset) | 0x5C6340 | 0x5C6340 (-1 case, `E+0x68`) | med | `005c6340` l.42-47 |
| E | 0x68 | 4 | int | "snapshot pending" | 0x5C6340 | 0x5C7370 (1), 0x5C6340 (0) | med | l.40-46 |
| E | 0x6C | 4 | int | blocked timer (ms) | 0x5C8C20 | 0x5C87E0 (0), 0x5C8C20 (+= dt) | med | `2999 < *(param_2+0x6c)` |
| E | 0x70 | 4 | int | **FM6 hold flag** (1 after leader change) | 0x5CA7C6 | 0x7E6546/6556/6566 (1), 0x5CA802 (0), 0x7E5610 (0) | high | quoted |
| E | 0x74 | 12 | vec3 | leader position at last plan | none seen | 0x5C7370 | low | `param_2+0x74..0x7C` |
| E | 0x80 | 4 | int | per-slot extra dword rotated with the ids | 0x7E5DF0 | 0x7E5DF0 | low | `param_1[local_48*0x22+0x29]` |

**CSWPartyTable (server, `*(*(serverApp+4)+0x1F0B4)`, `0x51C8D0`)**
| class | offset | size | type | proposed name | readers | writers | confidence | evidence |
|---|---|---|---|---|---|---|---|---|
| CSWPartyTable | 0x00 | 4 | int | member count (max 2) | 0x5F8D80, 0x5FAFD0, 0x5FB070 | 0x5F8D80 (+1), 0x5F8B00 (-1) | high | `if (*param_1 != 2)` |
| CSWPartyTable | 0x04 | 4 | int | puppet count | 0x5F9220 | ? | med | `param_1[1]` loop |
| CSWPartyTable | 0x08 | 8 | int[2] | member NPC indices | 0x5FAFD0 | 0x5F8D80 | high | `param_1[*param_1 + 2] = param_2` |
| CSWPartyTable | 0x10 | 8 | int[2] | puppet indices | 0x5F9220 | ? | low | `param_1[local_3c + 4]` |
| CSWPartyTable | 0x1C | 0x30 | objid[12] | NPC object id per NPC index | 0x5FAAF0, 0x5FB070, 0x5FE3B0 | 0x5F8D80, 0x5F9220 | high | `param_1[param_2 + 7] = param_3` |
| CSWPartyTable | 0x110 | 4 | int | follow state (range row selector) | 0x7E7D70 | 0x5FE470 (0) | med | decomp |
| CSWPartyTable | 0x114 / 0x118 | 4 / 4 | int | leader NPC index / controlled-PC NPC index (-1 = PC) | 0x5FE3B0 / 0x5FE4B0 | 0x5FE410 / 0x5FE4B0 | high | decomp |
| CSWPartyTable | 0x238 | 4 | int | solo mode | ? | 0x5FEEF0 | high | decomp |

## (f) Hook-site catalogue
All windows verified with a script over every `.asm` in the dump: instruction boundary, no relative branch/call in the stolen bytes, no branch target inside (none), start targets listed. None is within 12 B of the sibling self-patch list (`0x40D69E`, `0x8ABA..`) nor of the installed hooks (`0x55AB30/0x55AC26/0x51D790/0x51DC70/0x51DC86`, `0x56BDB4/BD/E3`, `0x569A08`, `0x586930/0x586B70/0x586F4D`, `0x541080`...) except the FM4 byte patches (see 8).
| id | address | length | stolen bytes (hex) | convention / live regs | consumed exit? | EAX/stack notes | hot/cold | nearest sibling self-patch site |
|---|---|---|---|---|---|---|---|---|
| AI5-H1 party-flag census | `0x58A35E` | 6 | `89 88 AC 11 00 00` | EAX=creature (`[EBP-0x48]`), ECX=new flag (`[EBP+8]`); apply `[EBP+0xC]`; caller return addr `[EBP+4]` | no | handler sees OLD `[EAX+0x11AC]`; stolen writes memory only (EAX, ECX unchanged) | cold | far |
| AI5-H2 FM6 leader-change | `0x7E6546` | 10 | `C7 80 94 00 00 00 01 00 00 00` | EAX=CSWParty (`[EBP-0x12C]`); ids `[EAX+0x24+k*0x88]`, states `[EAX+0x28+k*0x88]` | no | after `RunScript(k_hen_leadchng)`; next 2 stores are the sibling flags (+0x11C, +0x1A4) | cold | far |
| AI5-H3 FM6 consumer | `0x5CA7C3` | 6 | `8B 45 08 8B 48 70` | `[EBP+8]`=E, creature `[EBP-0x168]`; stolen loads EAX,ECX | no | handler runs before the stolen loads; do not rely on EAX | warm (state-5 handler per idle follower per tick) | far |
| AI5-H4a exit5m | `0x7E7838` | 7 | `C7 42 38 00 00 00 00` | EDX=FollowInfo, creature `[EBP-0x38]`, id `[EBP-0x3C]` | no | edge, after `RunScript(k_hen_exit5m)` | cold | far |
| AI5-H4b enter5m | `0x7E78AE` | 7 | `C7 42 38 01 00 00 00` | same | no | edge, after `RunScript(k_hen_enter5m)` | cold | far |
| AI5-H5 follow engaged | `0x5BF925` | 10 | `C7 80 B0 11 00 00 03 00 00 00` | EAX=creature (`[EBP-0x24]`) | no | each time 0x3D returns 1 | hot (per follower per AI tick) | far |
| AI5-H6 follow driver entry | `0x5C6340` | 9 | `55 8B EC 81 EC 88 02 00 00` | ECX=creature (fastcall); inbound CALL `0x5C4151` only | no | prologue; stolen restored in trampoline | hot | far |
| AI5-H7 stuck guard | `0x5C6CD4` | 7 | `83 7A 44 32 0F 9F C0` | EDX=E (`[EBP-0x78]`), creature `[EBP-0x244]` | no | stolen `CMP`+`SETG AL` write EAX/flags (the next `TEST EAX,EAX` reads them): do not clobber flags | warm (once per loop iteration of a follower) | far |
| AI5-H8 follow-radius re-plan | `0x5BD09E` | 7 | `C7 42 04 05 00 00 00` | EDX=E, creature `[EBP-0x94]` | no | after `FUN_007e6be0` slot test | cold-warm | far |
| AI5-H9 SetLeaderBySlot entry | `0x7E5DF0` | 10 | `55 8B EC 6A FF 68 21 B8 96 00` | ECX=CSWParty, `[ESP+4]`=slot arg (-3/-2/1..) | no | 9 inbound CALLs at the start are fine | cold | far |
| AI5-H10 ResetAllFollowData entry | `0x7E8350` | 9 | `55 8B EC 83 EC 3C 89 4D C4` | ECX=CSWParty, args `[ESP+4]` pos ptr, `[ESP+8]` heading float, `[ESP+0xC]` flag | no | UI path only (5 callers) | cold | far |
| AI5-H11 state dispatch (optional) | `0x5C6ACE` | 7 | `FF 24 8D 34 73 5C 00` | ECX=state (-1..10), indirect JMP absolute | **last stolen insn is an indirect jump** (fine in a trampoline) | per follower per loop iteration | hot | far |
| FM4 byte patch A | `0x574529` | 1 | `01` -> `00` | imm8 of `6A 01` at `0x574528` | n/a | KPM simple patch | cold | 10 B from shipped hook `0x57451F` |
| FM4 byte patch B | `0x575160` | 1 | `01` -> `00` | imm8 of `6A 01` at `0x57515F` | n/a | needed in addition to A (non-party level-2) | cold | 35 B before shipped hook `0x575183` |

## (g) Mod relevance
- party-flag (`+0x11AC`): section 2; H1 settles who set it; correct test = `0x5FB070`/`0x7E5C40` or the cached flag; Hawk standees are 0.
- FM4: section 8 (non-party AI level 2 on every save; second byte `0x575160`); P5.
- FM6: section 5 (consumer `0x5CA6A0` found, hold flag semantics); P3; low residual.
- FM1: section 6 (`ClearAllActions` on a follower sets `+0x11B0:=1`, slot state -1, and nothing in the engine re-queues 0x3D; `BumpFriends` rule at `0x586940`).
- E4 (stuck-move timeout): section 4 state machine (loop guard `0x5C6CD4`, OnBlocked posts in states 2/3 `0x5C87E0`/`0x5C8C20`, delta 0); H7/P2.
- E7 (5 m hooks): section 7: `exit5m`/`enter5m` are edges of `FollowInfo+0x38` at 25.0 m^2 (5 m) and fire from `CSWParty::Update`, not from the follow state machine; `k_hen_retreat` uses ranges.2da row 0x10; restoring follow in them would fight the 0x3D/QuickWalk driver (see R2).
- AIQ/FM3 (AI level churn): section 3/8 (only SetPartyMemberFlag, AIUpdate promotion, perception drop FM7 and SaveCreature change levels).
- Mod 8 / L1 / L2: party gating inside scripts = `IsObjectPartyMember` = `+0x11AC` (cmd 576 `0x68BBE0`), so selected followers only; Hawk standees excluded.
- perf: `0x5C6340` + `0x7E8A70` + `0x7E6CE0` run per follower per tick / per frame; FollowInfo is not on that path.
