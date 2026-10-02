# AI7 Script <-> engine interface (Steam swkotor2.exe 6a522e71, static, 2026-10-02)

## Summary (5 lines)
1. **GetCreatureTalentBest / Random never use `nCRMax` as a filter**: the only test on it is `== -1` (random mode). Candidates are ranked by the talent's own spells.2da / feat.2da `MaxCR` column, the engine flips a coin between the spell group and the feat group first, and every KNOWN spell goes through `HasTalent` (which can scan the whole shared inventory) before the category test. Item-granted spells (medpacs) count as a talent only for a creature that has at least one NON-Force class slot (`0x6F2A30` gate), and a Force power is only a talent when the creature can pay its Force-point cost. This refines R4 row 4.
2. **`GetCurrentAction` returns the LAST reportable node of the head action group**, not the head: a companion walking up to attack reports `ACTION_ATTACKOBJECT` (3), never `ACTION_MOVETOPOINT` (0), because the engine re-queues the approach nodes in the 0xC node's group. `ACTION_QUEUEEMPTY` (0xFFFE) only when the list is empty; `ACTION_INVALID` (0xFFFF) when the head group has no reportable type. Join table in Q3.
3. **Party facts at the script boundary**: `IsObjectPartyMember` returns the raw creature `+0x11AC`; `ActionFollowLeader` is a silent no-op unless `+0x11AC != 0` and `+0xEC == 1`; `GetIsDead` = `IsDead` (never true for `+0x11AC != 0`) OR (`+0x11AC != 0` and HP <= 0). The engine's own "is in the active party" test is `CSWParty::IsPartyMember 0x7E5C40` (ECX = `[[[0xA1B4A4]+4]+4]+0x270]`, i.e. `0x73FB90(client)` with client = `[[0xA1B4A4]+4]`; arg = client object id from `0x73FEC0(client, serverId)`), used by the creature EventHandler for the notice event.
4. **Event table (0x56EFD0) decoded**: 14 script slots at creature `+0x270 + 8*n`; every slot's engine run site and skip condition is in Q2 (slot 3 skipped for dead / HP<=0 party members, slot 12 skipped for player-controlled creatures, slot 9 has no engine run site, slot 6 has three). `SignalEvent`/`EventUserDefined` = event 10 with payload type 0xB (slot 11), delta 0.
5. **Command costs**: the command table is `malloc(0xDB4)` indexed by nwscript.nss order (774 of 877 slots filled, the 103 empty ones are `SWMG_*`/`AurPostString`: -2002). `GetNearestCreature` is an x-sorted sweep over EVERY object of the area (not only creatures) with a dedupe scan per candidate, dead creatures are skipped, `CREATURE_TYPE_IS_ALIVE` can never match. `GetFirstObjectInShape` and `GetFirstEffect` keep their cursor in the target (`area+0x1A0`, `obj+0x210`), so nested loops on one target clash.

Method: `.asm`/`.c` from `ROOT/decomp`, command table decoded from the `MOV [reg+off],imm` stores of `0x665F50` (index = off/4) and joined to `vanilla/nwscript.nss` order (877 prototypes = 0x36D slots; anchors: idx 6/7/294 = Assign/Delay/ActionDoCommand per U1, idx 729 SetFormation -> `0x5D4B60`, idx 834 SetCreatureAILevel = `0x66B640` per GATE_wave1, handler self-checks `param_1 == 0x26/0xE2..0xE5`, `0xEB..0xED`, `0x133/0x134`, `0x135/0x136`, `0xDD`). Vtables and tables read from `backups/vanilla_clean/swkotor2.exe` with a PE section reader (CSWSCreature vtable `0x994F5C`: `+0x30`=AsCreature `0x458940`, `+0x70`=AIUpdate `0x569320`, `+0x94`=IsDead `0x563460`, `+0x9C`=GetHP `0x5413C0`). Event ids = dwords at `0x9923F4..0x992460` (0,1,2,...,27 consecutive). Window checker: boundary + relative-branch + whole-dump inbound-target scan.

## (a) Function table
Rows marked [C] were confirmed against the decompile/asm in this batch; handler names follow the GATE convention `Global::ExecuteCommand<Name>`; the index->name join is high (table order == nwscript order, 4 independent anchors above).

| addr | proposed name | conf | one-line purpose | evidence |
|---|---|---|---|---|
| 0x665F50 | CSWVirtualMachineCommands::InitializeCommands [C] | high | `malloc(0xDB4)`, zero 0x36D slots, fill 774 handlers | `00665f59 PUSH 0xdb4`, `00665f84 CMP [EBP-4],0x36d`, stores `MOV [ECX+0x134],0x68c4a0` ... |
| 0x668FD0 | CSWVirtualMachineCommands::ExecuteCommand(id,argc) [C] | high | `id < 0x36D && table[id] != 0` -> `handler(id,argc)` else -2002 | `00668fd9 CMP [EBP+8],0x36d`, `00669008 CALL EAX`, `00669012 MOV EAX,0xfffff82e` |
| 0x6FD8D0 | CVirtualMachine::RunScript(CExoString*,objId,flag) [C] | high | public wrapper, 63 callers = every engine script run | `006fd925 CALL 0x007022a0`; callers incl. 0x56EFD0, 0x56B8A0, 0x5959E0, 0x569320 |
| 0x679050 | Global::ExecuteCommandExecuteScript [C] | high | pops (string, object[, int]) -> `RunScript(name, target, 1)`; stores 3rd arg in `DAT_00a7e10c` | `FUN_006fd8d0(local_1c,local_14,1)`, `DAT_00a7e10c = local_20` |
| 0x56EFD0 | CSWSCreature::EventHandler(eventId,callerId,payload,day,ms) ret 20 [C] | high | per-event dispatch incl. all creature script slots | Q2 |
| 0x56B8A0 | CSWSCreature::SpawnInHeartbeatPerception [C, AI3] | high | slot 8 (spawn) and slot 0 (heartbeat) run sites | `FUN_006fd8d0(param_1 + 0xac,...)`, `FUN_006fd8d0(param_1 + 0x9c,...)` |
| 0x683D40 | Global::ExecuteCommandGetNearestCreature (+ ToLocation, GetNearestObject, ToLocation, ByTag; ids 0x26, 0xE2..0xE5) [C] | high | x-sorted area sweep | Q1 row 38 |
| 0x685960 | Global::ExecuteCommandGetFirst/NextObjectInShape (0x80/0x81) [C] | high | shape iteration with cursor in the area | `*(int *)(local_68 + 0x1a0) = local_60;` |
| 0x67ACC0 | Global::ExecuteCommandGetCreatureHasTalent [C] | high | pops talent struct (type 3) + object -> `0x6B8840(type,id)` | `0067ad56 CALL 0x006b8840` |
| 0x67AE40 | Global::ExecuteCommandGetCreatureTalentRandom/Best (ids 0x133/0x134) [C] | high | pops args -> core `0x6B8BB0` | `FUN_006b8bb0(local_20,local_44,...)` / `(local_20,0xffffffff,...)` |
| 0x66BDC0 | Global::ExecuteCommandActionUseTalentOnObject/AtLocation (0x135/0x136) [C] | high | -> `0x57D8B0` / `0x57DA90` | `FUN_0057d8b0(local_c,local_24,local_2c,...)` |
| 0x6B8840 | CSWSCreatureStats::HasTalent(type,id) ret 8 [C] | high | spell/feat/skill usable test (class loop, FP cost, item scan) | Q4 |
| 0x6B8BB0 | CSWSCreatureStats::GetTalentBestOrRandom(cat,crMax,out...) ret 44 [C] | high | coin flip between spell group and feat group | `uVar3 = uVar3 & 0x80000001;` |
| 0x6BAEC0 | CSWSCreatureStats::PickKnownSpellTalent ret 32 [C] | high | loop over known-spell vector `stats+0x134`; rank = spell `MaxCR` | `006bb09e CALL 0x006ba970`, `006bb0ca CMP [EBP+0xc],-1` |
| 0x6BA970 | CSWSCreatureStats::IsSpellTalentCandidate ret 32 [C] | high | excl id, HasTalent, area test, category match | asm Q4 |
| 0x6BAE00 / 0x6BCFA0 | class-granted power pick (Force classes) [C] | med | second spell sub-source: loops class slots, `0x6F2A30(classId)` true -> `0x6BCFA0` | `iVar1 = FUN_006f2a30(local_e); if ((iVar1 != 0) && (local_8 = FUN_006bcfa0(...), local_8 == 1)) break;` (0x6BAE00 decompile) |
| 0x6BB3A0 | CSWSCreatureStats::PickItemSpellTalent ret 36 [C] | high | equip-slot items then shared repository items with property type 10 | `FUN_006d0620(1 << local_8)`, `FUN_0058a2c0(1)` |
| 0x6BC0E0 | CSWSCreatureStats::PickFeatTalent ret 28 [C] | high | feat vector, rank = feat `MaxCR` (+0x14), category (+0x10) | `006bc23e CMP [EBP+0xc],-1` |
| 0x6BDD60 | CategoryMatches(rowCat,wantedCat) [C] | high | 4 nibbles, wanted nibble 0xF = wildcard | decompile: `(param_2 & 0xf000) != 0xf000 && != (param_1 & 0xf000)` |
| 0x6BDE30 | ExclusionColumnOk(type,id,incl) [C] | high | reads 2DA column "Exclusion"; non-zero excludes | `FUN_00733570("Exclusion")` |
| 0x6F2A30 | IsForceClass(classId) [C] | high | true for ids 3,4,5,0xB,0xC,0xD,0xE,0xF,0x10 | `006f2a34..006f2a64` compare chain; names SPECULATIVE |
| 0x6C0330 | CSWSCreatureStats::HasItemSpell(spellId) [C] | high | scans 20 equip slots then the whole shared repository for a property type 10 | `FUN_006d0620(1 << ..)`, `FUN_00606100(local_8)` |
| 0x6C06B0 | CSWSCreatureStats::FindItemProvidingSpell(spell,&item,&prop) ret 12 [C] | high | same scan, returns item id + property index | `*param_3 = *(iVar2 + 0x14); *param_4 = local_18; return 1;` |
| 0x57D8B0 | CSWSCreature::UseTalentOnObject(type,id,target,...) ret 32 [C] | high | spell: item cast or `AddCastSpellActions`; feat: `AddAttackActions`; skill: `UseSkill` | decompile |
| 0x67B100 | Global::ExecuteCommandGetCurrentAction [C] | high | last reportable node of the head group | Q3 |
| 0x5D7190 / 0x5D7220 | ActionTypeIsReportable / ActionTypeToScriptAction [C] | high | the type -> `ACTION_*` map | Q3 |
| 0x68BBE0 | Global::ExecuteCommandIsObjectPartyMember [C] | high | returns raw `creature+0x11AC` | `0068bc2e MOV EDX,[ECX+0x11ac]` |
| 0x669FC0 | Global::ExecuteCommandActionFollowLeader [C] | high | `AddAction(0x3D)` only if `+0x11AC != 0 && +0xEC == 1` | `00669fef`, `0066a001 MOV ECX,[EAX+0xec]` |
| 0x68B4B0 | Global::ExecuteCommandGetIsDead [C] | high | IsDead (vt+0x94) or (+0x11AC and HP<=0) | decompile: `iVar3 = (**(code **)(*piVar5 + 0x94))(); if (iVar3 == 0) { ... if (piVar4[0x46b] == 0) bVar1 = false; else { sVar2 = (**(code **)(*piVar4 + 0x9c))(0); bVar1 = sVar2 < 1; }` |
| 0x563460 | CSWSCreature::IsDead (vt+0x94) [C] | high | `+0x11AC == 0 ? 0x5477F0() : 0` | `0056346a CMP [EAX+0x11ac],0` |
| 0x5477F0 | CSWSObject::IsDeadHP [C] | high | HP < -9 if `+0x10E8 != 0`, else HP < 1 | `return sVar1 < -9;` / `return sVar1 < 1;` |
| 0x7E5C40 | CSWParty::IsPartyMember(clientId) ret 4 [C] | high | linear scan of the active-party array (stride 0x88, id at +0x24) | `param_1[local_8 * 0x22 + 9] == param_2`; AI0 mapper vote `CSWParty::IsPartyMember` |
| 0x73FB90 | CClientExoApp::GetParty (ECX = client `[[0xA1B4A4]+4]`, returns `[[client+4]+0x270]`) [C] | high | the CSWParty object (active party) | `return *(undefined4 *)(*(int *)(param_1 + 4) + 0x270);`; call site `005705a0 MOV EDX,[ECX+4]` ... `005705dd CALL 0x0073fb90` |
| 0x7E5DA0 | CSWParty::GetCharacter(index) [C] | high | used by GetPartyMemberByIndex / GetPartyLeader | decompile `FUN_0073f550(param_1[param_2*0x22+9])` |
| 0x66DAF0 | Global::ExecuteCommandActionAttack [C] | high | `AddAttackActions(target,0,0,passive,0,0,0,1,0x2719,0x5dc,1,0,0,4,0)` | decompile |
| 0x693020 | Global::ExecuteCommandSignalEvent [C] | high | posts event 10 (payload from EventUserDefined) delta 0 | `FUN_0051e170(..,DAT_0099241c,..)` |
| 0x678EB0 | Global::ExecuteCommandEventUserDefined [C] | high | builds payload `*local_40 = 0xb`, int0 = n | decompile |
| 0x693860 | Global::ExecuteCommandSpeakString / BarkString (0xDD) [C, delegated to AI6] | med | maps TALKVOLUME to 1/3/2/0xD/0xE, then `0x51C2E0`+`0x63A5B0` (shout) | decompile |
| 0x68F7E0 | Global::ExecuteCommandRemoveEffect [C] | high | marks effect `+0x20 = 0`, posts event 0xE delta 0 | `uVar7 = DAT_0099242c; FUN_0051e170(...)` |
| 0x67BD70 | Global::ExecuteCommandGetFirst/NextEffect [C] | high | cursor `obj+0x210` | `*(undefined2 *)(iVar4 + 0x210) = 0;` |
| 0x67E670 | Global::ExecuteCommandGetHasSpellEffect [C] | high | `0x5476C0(spellId)`: effect array scan for `effect+0x1C == spell` | `*(int *)(... + 0x1c) == param_2` |
| 0x5476C0 | CSWObject::HasSpellEffectApplied(spell) [C] | high | linear over `obj+0x148` (count `+0x14C`) | decompile |
| 0x681050 | Global::ExecuteCommandGetItemPossessedBy [C] | high | repository tag lookup `0x605CB0` then 20 equip slots | decompile |
| 0x685480 | Global::ExecuteCommandGetNumStackedItems [C] | high | `item+0x2CC` | decompile |
| 0x69AC60 | Global::ExecuteCommandHasLineOfSight [C] | high | one `CSWSArea::ClearLineOfSight 0x52BB60` ray | `FUN_0052bb60(&local_38,&local_20,...)` |
| 0x686990 | Global::ExecuteCommandGetObjectSeen/Heard (0x121/0x122) [C] | high | `0x5D1750` visible-list entry of the SOURCE; bit0 seen, bit1 heard | `*(byte *)(local_10 + 4) & 1` / `>> 1 & 1` |
| 0x6821A0 / 0x681940 / 0x67A250 / 0x67A2D0 / 0x67A340 | GetLastHostileActor / GetLastHostileTarget (+0x580) / GetAttackTarget (+0x548) / GetAttemptedAttackTarget (+0x550) / GetAttemptedSpellTarget (+0x568) [C] | high | single field reads | decompiles |
| 0x680000 | Global::ExecuteCommandGetIsInCombat [C] | high | `+0x520`; with arg2 requires `+0x11E8 == 1` | decompile |
| 0x696190 | Global::ExecuteCommandGetNPCAIStyle [C] | high | `*(u16*)(creature->stats + 0x146)` | `*(undefined2 *)(*(int *)(iVar2 + 0x1198) + 0x146)` |

DB hint: these handler rows are new; the lead may persist them as `Global::ExecuteCommand<Name>` (conf high for the index join, med where the engine callee name is only a mapper vote).

## (b) Findings

### Q1. Command table (CONFIRMED)
Dispatch: `0x668FD0` pops nothing itself; the NCS interpreter reads a signed 16-bit id (`0x700275 MOVSX EDX,word`, U1 B3.1), calls `ExecuteCommand(id, argc)`; the handler is called thiscall with ECX = implementer (`[ECX+4]` = OBJECT_SELF valid flag, `[ECX+8]` = OBJECT_SELF id, `[ECX+0xC]` = table) and stack `(id, argc)`. Return 0 = ok; `0xFFFFF82F` (-2001) = argument pop failed; `0xFFFFF830` (-2000) = result push failed; `0xFFFFF82E` (-2002) = unknown/empty slot. Arguments are popped in nwscript parameter order (first pop = first parameter, e.g. `FUN_006fd9a0(&local_6c)` = nFirstCriteriaType in 0x683D40). 774 of 877 slots are filled (about 607 distinct handlers); the 103 empty ones are idx 520/521, 582-668, 683-688, 717/718, 782, 789, 790, 792, 804, 825 (`SWMG_*`, `AurPostString`). idx 0 `Random` = `0x68F5D0` (`006685f7 MOV [ECX],0x68f5d0`, offset-0 store).

Cost classes: C0 = O(1) field read + one `GetGameObject` hash lookup (`0x53DFB0`, 4096-bucket chain); the VM per-call overhead (stack pop/push, `0x6FD9A0`..`0x6FDB10`) is common. "per-obj" = scales with objects in the area, "per-eff" = effects on the object (cap 1000 per AI4), "per-inv" = inventory.

| idx | command | handler | args | engine call | side effects | cost |
|---|---|---|---|---|---|---|
| 38 | GetNearestCreature | 0x683D40 | critType1,val1,oTarget,nNth,[type2,val2,type3,val3] | x-sorted sweep of `area+0x194` from `0x522AF0` | none (allocs 2 temp arrays, frees) | per-obj, see below |
| 226-229 | GetNearestCreatureToLocation / GetNearestObject / ...ToLocation / ...ByTag | 0x683D40 | by id | same sweep, other filters (object-type mask, tag compare `0x7339D0`) | none | per-obj |
| 128/129 | GetFirstObjectInShape / GetNextObjectInShape | 0x685960 | shape, size, location, objectTypeMask, [origin] | start index `0x522AF0(minX)`, scan to `x > maxX` | cursor `area+0x1A0` | per-obj in x-slab |
| 306 | GetCreatureHasTalent | 0x67ACC0 | talent, oCreature | `0x6B8840(type,id)` | none | per known class slot; item scan per-inv (Q4) |
| 307 / 308 | GetCreatureTalentRandom / Best | 0x67AE40 | cat,[crMax],oCreature,incl,exclType,exclId | `0x6B8BB0` | `rand()` | known spells x HasTalent (Q4) |
| 309 / 310 | ActionUseTalentOnObject / AtLocation | 0x66BDC0 | talent, target | `0x57D8B0` / `0x57DA90` | queues cast/attack/skill actions; spell without item fields rescans inventory (`0x6C06B0`) | per-inv for spells |
| 37 | ActionAttack | 0x66DAF0 | oAttackee, bPassive | `0x568540 AddAttackActions(target,0,0,passive,...)` | queues one 0xC (or 0x3F) node, group new (0xFFFF) | C0 + queue |
| 9 | ClearAllActions | 0x66F330 | none | `0x541080(1)` (AI4 Q5) | clears nodes the object agrees to clear, resets `+0x11B0`, party slot | O(queue) |
| 21 | ActionMoveToLocation | 0x68CEE0 | location, bRun | `0x564770 AddMoveToPointAction(0xFFFF, ...)` | one type-1 node, new group | C0 + queue |
| 22 | ActionMoveToObject | 0x68CB50 | oMoveTo, bRun, fRange | `0x564770` (group 0xFFFF) then `AddAction(0x11,0xFFFE)`, `AddAction(0x13,0xFFFE)` | chain [1, 0x11, 0x13] in ONE group | C0 + 3 nodes |
| 23 | ActionMoveAwayFromObject | 0x68C9B0 | oFleeFrom, bRun, fDist | `AddAction(3,0xFFFE,...)` | | C0 |
| 730 | ActionFollowLeader | 0x669FC0 | none | `AddAction(0x3D,0xFFFF)` | NO-OP unless `+0x11AC != 0 && +0xEC == 1` | C0 |
| 843 | ActionFollowOwner | 0x69B660 | | `AddAction(0x46)` puppet | | C0 |
| 522 | GetCurrentAction | 0x67B100 | oObject | head-group scan (Q3) | none | O(group length) |
| 320 | GetIsInCombat | 0x680000 | oCreature, bOnlyCountReal | reads `+0x520` (`+0x11E8 == 1` if arg2) | none | C0 |
| 721 / 556 | GetLastHostileTarget / GetLastHostileActor | 0x681940 / 0x6821A0 | oAttacker | `+0x580` (set at `EndCombatRound 0x58F618`, cleared by `SetCombatState 0x57A6C7`) / `+0x17C` with validity re-checks and clears | 0x6821A0 may clear the stored id (`0x54A1E0`) | C0 |
| 316 / 361 / 375 | GetAttackTarget / GetAttemptedAttackTarget / GetAttemptedSpellTarget | 0x67A250 / 0x67A2D0 / 0x67A340 | | `+0x548` / `+0x550` / `+0x568` | none | C0 |
| 304 | GetHasSpellEffect | 0x67E670 | nSpell, oObject | `0x5476C0`: scan `obj+0x148/+0x14C` for `effect+0x1C == spell` | none | per-eff |
| 85 / 86 / 87 | GetFirstEffect / GetNextEffect / RemoveEffect | 0x67BD70 / 0x67BD70 / 0x68F7E0 | | cursor `obj+0x210`; remove = flag `effect+0x20=0` + event 0xE | event posted, removal deferred to the event pump | per-eff |
| 30 | GetItemPossessedBy | 0x681050 | oCreature, sTag | party repository `0x605CB0` (tag scan) then 20 slots | none | per-inv |
| 475 | GetNumStackedItems | 0x685480 | oItem | `item+0x2CC` | none | C0 |
| 339 / 340 | GetFirst/NextItemInInventory | 0x680830 | oTarget | repository via `0x58A2C0(1)`; cursor on the creature (`+0x1158..`, R4) | | C0 per call |
| 155 | GetItemInSlot | 0x680F20 | slot, oCreature | `0x6D0620(1<<slot)` | none | C0 |
| 576 | IsObjectPartyMember | 0x68BBE0 | oCreature | returns `creature+0x11AC` | none | C0 |
| 577 / 845 | GetPartyMemberByIndex / GetPartyLeader | 0x6874F0 / 0x69B7C0 | idx | `CSWParty::GetCharacter 0x7E5DA0` -> client id -> `0x51C8B0` | none | C0 |
| 548 | GetFirstPC | 0x6875E0 | | `0x51C8D0` | none | C0 |
| 140 | GetIsDead | 0x68B4B0 | oCreature | `vt+0x94` or (`+0x11AC` and `vt+0x9C(0)<=0`) | none | C0 |
| 163 / 162 | GetCommandable / SetCommandable | 0x67AC20 / 0x690F40 | | `obj+0xEC` | | C0 |
| 235 / 236 / 237 | GetIsEnemy / GetIsFriend / GetIsNeutral | 0x67C430 | oTarget, oSource | `rep = target->GetReputation(source) 0x5E13C0`: enemy `< 11`, friend `> 89`, neutral 11..89; if the target is an object class for which `vt+0x48` is non-NULL (placeable-like, unnamed) enemy additionally needs that object's `+0x398 != 0` | none | C0 + reputation (AI6) |
| 289 / 290 | GetObjectSeen / GetObjectHeard | 0x686990 | oTarget, oSource | `0x5D1750` entry bit 0 / bit 1 | none | O(visible list) |
| 820 | HasLineOfSight | 0x69AC60 | vFrom, vTo, oSource, oTarget | one `0x52BB60` ray | none | one LOS ray |
| 41 / 151 | GetDistanceToObject / GetDistanceBetween | 0x67BA20 / 0x67B3B0 | | `0x5142F0` 3D length | | C0 |
| 42 | GetIsObjectValid | 0x686910 | | `0x51C0A0` | | C0 |
| 131 | SignalEvent | 0x693020 | oObject, event | `0x51E170(0,0,self,target,10,payload)` (delta 0) | event 10 queued | C0 + O(n) sorted insert (U1) |
| 132 | EventUserDefined | 0x678EB0 | nEvent | `new(0x34)` payload type 0xB int0 = n | | alloc |
| 221 | SpeakString | 0x693860 | sStringToSpeak, nTalkVolume | map volume {0,1,2,3,4} -> {1,3,2,0xD,0xE}; `0x51C2E0` + `0x63A5B0` (shout dispatch, AI6) | shout delivery | AI6 |
| 671 | BarkString | 0x66DD30 | | UI string + `AddAction(0xE)` | | C0 |
| 195 / 254 | GetListenPatternNumber / GetLastSpeaker | 0x683740 / 0x682900 | | object `+0x1BC` / creature `+0x1A0` | | C0 |
| 256-261 | GetLastPerceived / ...Heard / ...Inaudible / ...Seen / ...Vanished | 0x682670 / 0x682710 | | `+0x1008` / `+0x100C` / `+0x1014` / `+0x1010` / `+0x1018` (AI3) | | C0 |
| 6 / 7 / 294 | AssignCommand / DelayCommand / ActionDoCommand | 0x66D9F0 / 0x670370 / 0x6699D0 | | U1 B1.1 / B1.5 | event 1 / action type 0x25 | O(n) insert; NCS copy |
| 8 | ExecuteScript | 0x679050 | sScript, oTarget, nScriptVar | `RunScript(name,target,1)` | `DAT_00a7e10c` = var | full script run (U1: NCS memcpy per run) |
| 54 | CancelCombat | 0x697090 | oCreature, bQuiet | `0x586750` | | C0 |
| 399 / 400 / 853 | ActionEquipMostDamagingMelee / Ranged / ActionSwitchWeapons | 0x669A50 / 0x669AD0 / 0x69B980 | | `0x580330` / `AddAction` | equip actions | per-inv |
| 705 / 704 | GetNPCAIStyle / GetPartyAIStyle | 0x696190 / 0x696140 | oCreature | `stats+0x146` / `0x51C8D0()+0x10C` | | C0 |
| 732 | GetIsDebilitated | 0x696D80 | oCreature | `0x6DAA00(0)`: true if `+0xFF5` not in {0,1,0x10} or (+0x11AC and HP<1) | | C0 |
| 810 | IsStealthed | 0x68BCF0 | | `(creature+0x1120 & 1) != 0` | | C0 |
| 514 | GetUserActionsPending | 0x68A690 | | `0x5D53A0` action-list scan | | O(queue) |
| 580 | GetGlobalNumber | 0x67DB70 | sName | globals table (R4 hook idea `0x67DC4A`) | | string compare |
| 834 | SetCreatureAILevel | 0x66B640 | | GATE row | | |

**GetNearestCreature scan (CONFIRMED, cost law).**
- Candidate list: `area+0x194` (id array, count `+0x198`) = ALL objects of the area sorted by X (`0x522AF0 GetFirstObjectIndiceByX` bisects on `obj+0x94`; `0x522E00 UpdatePositionInObjectsArray` keeps it sorted when an object moves). Start index = the reference object's own index: `for (; (local_70 < local_44[1] && (*(int *)(*local_44 + local_70 * 4) != local_2c)); local_70 = local_70 + 1)`. If the reference object is not in the area array the result is OBJECT_INVALID (`local_38 == 0`).
- Walk order alternates outward (+1, -1, +2, -2 ...); a side ends when `*(float *)(local_a8 + -4 + local_68 * 4) < local_98 * local_98`, i.e. the squared X-distance already exceeds the current nth-best 3D squared distance, which needs `nth` matches first. With no match (no enemy in the area) the whole array is swept. Cost = O(objects in the X slab to the nth-best distance), each candidate = hash lookup `0x53DFB0`, `vt+0x30` (AsCreature), dedupe scan of the found-id array (`00684349 CALL 0x00476620`, linear in matches), criteria.
- Filter semantics (type, value): RACIAL_TYPE (0, quirk: compares against the FIRST value even for slots 2/3), PLAYER_CHAR (1: `stats+0x6C`), CLASS (2: slots `+0xAB/+0xD3/+0xFB`), REPUTATION (3: value 0 friend `> 0x59`, 1 enemy `< 0xB`, 2 neutral, computed as `candidate.GetReputation(reference)`), IS_ALIVE (4: `case 4: break;` leaves `local_c0 = 0` -> never matches), HAS_/DOES_NOT_HAVE_SPELL_EFFECT (5/6: `0x5476C0`), PERCEPTION (7: the REFERENCE creature's visible-list entry `0x5D1750(candidate)`, bit0 seen / bit1 heard; a candidate absent from the list counts as not seen and not heard).
- Always applied after the criteria: dead creatures excluded (`vt+0x94 == 1`, or `+0x11AC != 0` and HP <= 0; the "include dead" local `[EBP-0x3C]` is constant 0: `00684a6f CMP [EBP+0xffffffc4],0 / JNZ 0x684af4`). `nth` larger than the match count -> invalid (-> script gets OBJECT_INVALID, `0xFFFFF830` only if the push fails).
- Cost magnitude: SPECULATIVE; a typical K2 area has 100-600 objects (placeables, doors, waypoints, triggers, AoEs, items on the floor count: the array is not creature-only). `k_ai_master` calls it with REPUTATION_ENEMY + PERCEPTION_SEEN at about 25 sites (e.g. `k_ai_master.nss:54`, `:238`, `:452`, `:525`).

**GetFirstObjectInShape (CONFIRMED).** The scan runs in the module's current area (`local_68 = FUN_00557ef0()`), start `0x522AF0(minX)`, ends at `if (local_6c < local_20)` (x past maxX). The only iterator state is `*(int *)(local_68 + 0x1a0) = local_60;`, Next resumes at `*(int *)(local_68 + 0x1a0) + 1` and re-pops the shape arguments: two interleaved First/Next loops (e.g. a loop that calls a function containing another loop) corrupt each other. The object-type mask uses bits 1,2,4,8,0x10,0x20,0x40,0x80,0x100,0x200 = types 5,6,7,0xA,0xB,0xC,9,0xE,0xD,0x10 (`DAT_00994474..` byte table: 5 creature, 7 trigger, 9 placeable, 0xA door, 0xB AoE, 0xC waypoint).

### Q2. Event id -> script slot table of `0x56EFD0` (CONFIRMED, decoded from decompile + asm)
Signature (`ret 20`): `EventHandler(this=creature, eventId [EBP+8], callerId [EBP+0xC], payload [EBP+0x10], day [EBP+0x14], ms [EBP+0x18])`. First action of EVERY event: `0056f011 CMP [EBP-0x114],0 / 0056f018 JNZ / 0056f01a PUSH 0 / 0056f022 CALL 0x0056b8a0` = if `creature+0x38C == 0` run the spawn block (slot 8) first. Event ids (consecutive dwords `0x9923F4..0x992460`): 1 `[9923F8]`, 4 `[992404]`, 5 `[992408]`, 8 `[992414]`, 9 `[992418]`, 10 `[99241C]`, 0xB `[992420]`, 0xE `[99242C]`, 0xF `[992430]`, 0x12 `[99243C]`, 0x13 `[992440]`, 0x14, 0x15, 0x16, 0x1B.

Slot offsets (SaveCreature `0x574500`, `FUN_00710810(param_1 + 0x270..0x2D8)`): `0x270+8n`: 0 Heartbeat, 1 OnNotice, 2 SpellAt, 3 Attacked, 4 Damaged, 5 Disturbed, 6 EndRound, 7 Dialogue, 8 Spawn, 9 Rested, 10 Death, 11 UserDefined, 12 OnBlocked, 13 EndDialogue (`index < 0xE` guard at `0056fa5c CMP EAX,0xe`).

| slot | name | engine run site(s) | event / trigger | conditions that SKIP it |
|---|---|---|---|---|
| 0 | Heartbeat | `0x56B8A0` `FUN_006fd8d0(param_1 + 0x9c, ...)` (decompile) | AIUpdate every pass (`005694e3`), mode 1 | not `IsDead(vt+0x94)`; interval gate: elapsed >= `+0x398` (3000..4199 ms) AND (`+0x78` level != 0 or counter `+0x3D4 >= +0x398>>6`) per AI3 `0056ba03`; also runs the 2DA `SCRIPTNAME` script of the current SetState when `+0xFF5` in {1,0x10,2,0xB} |
| 1 | OnNotice | EventHandler `00570640` (non-party) / `0057069a` (party flag) | event 10, payload type 1 (posted by `0x576DC0`, AI3) | creatures with `+0x11AC != 0` run it only if `0x7E5C40(clientId)` (active party) : `005705e4 CALL 0x007e5c40 / 005705eb JZ 0x00570645`; no dead / combat gate |
| 2 | SpellAt | `00570d08` | event 10, payload type 2 (poster not traced) | not `IsDead (vt+0x94)` and not (`+0x11AC` and HP<1) (decompile: `iVar4 = (**(code **)(*param_1 + 0x94))(); if (iVar4 == 0) { ... if (!bVar7) FUN_006fd8d0(param_1 + 0xa0,...)`); first stores the caster in `+0x1FC` (`param_1[0x7f]`), `+0x200`, `+0x204` |
| 3 | Attacked | `0056faa1` | event 0xF (per melee/ranged hit, `0x6E0650`/`0x6DDB00`, AI2) | `0056f9fc JNZ 0x0056fab1` when `IsDead (vt+0x94)`; `0056fa08 CMP [ECX+0x11ac],0` then `0056fa2e SETLE CL / 0056fa4c JNZ 0x56fab1` skips party members with HP <= 0; then `0x57E280` |
| 4 | Damaged | `0x5959E0` `005966d0 CALL 0x6fd8d0` direct call (not an event; AI2) | effect type Damage apply | skipped when the target is the party leader (`0x5FE3B0`, AI2) |
| 5 | Disturbed | `00570f55` | event 10, payload type 0x1B | none seen; first sets `+0x188`, `+0x19C` from payload |
| 6 | EndRound | `0x58F50F` (EndCombatRound), `0x569A67` (AIUpdate 3 s fallback), `0x59E904` (SetState removal) | AI2 / GATE | AI2 Q6 conditions; third site has no gate |
| 7 | Dialogue | `005709f0` | event 10, payload type 7 (SpeakString -> listen pattern, AI6) | stores speaker `+0x1A0`, volume `+0x1BC`; an empty script name or the literal "default" is replaced by `k_hen_dialogue01` (`FUN_00733a60("default")`, `FUN_007338d0("k_hen_dialogue01")`); no dead gate |
| 8 | Spawn | `0x56B8A0` first block (`+0x38C == 0`); `0x599A10` (`FUN_006fd8d0(local_30 + 0xac,...)`, raise/resurrect) | first event / first AIUpdate | once per spawn |
| 9 | Rested | NONE found: every `FUN_006fd8d0` caller in the creature classes (0x569320, 0x56B8A0, 0x56EFD0, 0x58A500, 0x597D90, 0x599A10, 0x59E860, 0x5959E0, 0x58E990, 0x587290) uses slots 0,1,2,3,4,5,6,7,8,10,11,12,13 | - | SPECULATIVE absence (grep over callers, lesson 168) |
| 10 | Death | `0x597D90` (`FUN_006fd8d0(local_20 + 0xb0,...)`) after `0x58E990(0)` | death handling | party leader/dead conditions in 0x597D90 (not traced) |
| 11 | UserDefined | `00570a72` | event 10, payload type 0xB (`SignalEvent(o, EventUserDefined(n))`, CONFIRMED: `0x693020` + `0x678EB0`) | none; `param_1[0x5D] = GetInteger(0)` = event number for `GetUserDefinedEventNumber` |
| 12 | OnBlocked | `00570745` (`MOV EAX,0xc`) | event 10, payload type 0x1F (6 movement sites, GATE) | `005706cf CALL 0x0051c600 / 005706e1 JNZ 0x0057074a`: skipped for a player-controlled creature; sets `+0x37C` = blocker (`005706ec MOV [ECX+0x37C],EDX`) |
| 13 | EndDialogue | `0x58A500` (`FUN_006fd8d0(param_1 + 0x2d8,...)`, caller `0x6CF740` conversation end) | conversation end | none |

Events without a script slot: 1 = timed situation (`0056f1d7 CALL 0x006fd930`, RunScriptSituation: DelayCommand / AssignCommand / ActionDoCommand-less; payload null -> nothing); 4 = `RemoveFromArea 0x56E250`; 5 = `ApplyEffect 0x544210`; 8 and 0x13 = run the script NAMED IN THE PAYLOAD (`FUN_006fd8d0(local_50 + 7, param_1[1], 1)` -> payload+0x1C) after copying a "last attacker" block (`+0x1D4/0x1E0/0x1E4/0x1064/0x170`); 9 = `vt+0x7C(payload)` = `SetAnimation 0x56D350` unless dead/HP<1 party; 0xB = death finalisation (FadeDelayOnDeath, drops, `vt+0` destructor); 0xE = `RemoveEffectById 0x544790` (posted by RemoveEffect); 0x12 = `0x581070 ForceMoveToPoint`-like handler when `0x576CA0` accepts; 0x14 = `0x40E770`; 0x15 = `BroadcastSafeProjectile 0x547EC0`; 0x16 = on-hit effect application; 0x1B = `PlayRumblePattern 0x740800`. Names for 0x12-0x1B are mapper votes (med/low).

### Q3. `GetCurrentAction` -> action type mapping (CONFIRMED)
Algorithm (`0x67B100`): object -> `vt+0x10()` -> action list at `obj+0x100` (`0067b17a ADD ECX,0x100`); head node = `0x521060` (`*(*list)+8`); if NULL -> result 0xFFFE (`0067b1fd MOV [EBP-8],0xfffe`); else `group = node+0x6C` (word, `0067b1a4 MOV AX,[EDX+0x6c]`) and loop over the contiguous nodes with the same group: `0067b1bd CMP EDX,EAX / 0067b1bf JNZ 0x67b1fb` (stop at the first node of another group), `0067b1ca CALL 0x005d7190` (is the type reportable) and `0067b1df CALL 0x005d7220(type,0)`; `0067b1e7 MOV [EBP-8],EDX` overwrites the result for EACH reportable node, `0067b1f1 CALL 0x0058c370` next, `0067b1f9 JMP 0x0067b1ac`. So the result is the LAST reportable node of the head group; 0xFFFF if none is reportable.
Why attack approaches report ATTACKOBJECT: AIActionPhysicalAttacks re-queues the approach chain with the executing node's group: `006d871c MOVZX EDX,word ptr [ECX + 0x6c]` ... `006d8729 CALL 0x00540ca0` (AddActionToFront, group = node+0x6C) for 0xC (line 487), 0x11 (498), 0x13 (501), and `005644b0` (move) with `*(undefined2 *)(param_2 + 0x6c)`; likewise `0x5BC1D0` (`FUN_00540ca0(0x11,*(param_2+0x6c)...)`). The head group is therefore {move 1, 0x13, 0x11, 0xC} and the last reportable is 0xC -> 3.

| node type | AI4 handler | `GetCurrentAction` value | nwscript constant |
|---|---|---|---|
| 0x01 MoveToPoint | 0x5BFC00 | 0 | ACTION_MOVETOPOINT |
| 0x07 PickUpItem | 0x5CF8A0 | 1 | ACTION_PICKUPITEM |
| 0x09 DropItem | 0x5CFF50 | 2 | ACTION_DROPITEM |
| 0x0C PhysicalAttacks | 0x6D7390 | 3 | ACTION_ATTACKOBJECT |
| 0x0F CastSpell | 0x5B96F0 | 4 | ACTION_CASTSPELL |
| 0x14 / 0x15 Open/CloseDoor | 0x5DB0A0 / 0x5DAD10 | 5 / 6 | ACTION_OPENDOOR / CLOSEDOOR |
| 0x18 DialogObject | 0x5D75B0 | 7 | ACTION_DIALOGOBJECT |
| 0x19..0x1D mines | 0x5B43C0.. | 8, 9, 0xA, 0xB, 0xC | DISABLETRAP, RECOVERTRAP, FLAGTRAP, EXAMINETRAP, SETTRAP |
| 0x1E Wait | 0x5D9AA0 | 0x24 (36) | ACTION_WAIT |
| 0x26 / 0x27 / 0x28 Unlock/Lock/UseObject | 0x5DB8F0 / 0x5DDA60 / 0x5DFBE0 | 0xD / 0xE / 0xF | OPENLOCK, LOCK, USEOBJECT |
| 0x29 (stub) / 0x2A (stub) / 0x2B (stub) | 0x5D1990 / 0x5D17D0 / 0x5D1990 | 0x10 / 0x11 / 0x12 | ANIMALEMPATHY, REST, TAUNT |
| 0x2E ItemCastSpell | 0x5B8000 | 0x13 (19) | ACTION_ITEMCASTSPELL |
| 0x32 CounterSpell | 0x5D25E0 | 0x1F (31) | ACTION_COUNTERSPELL |
| 0x36 (stub) / 0x37 ForceFollowObject / 0x38 Heal | 0x5D1990 / 0x5CEED0 / 0x5D19B0 | 0x22 (34) / 0x23 (35) / 0x21 (33) | PICKPOCKET / FOLLOW / HEAL |
| 0x3D FollowLeader | 0x5BF7F0 | 0x26 (38) | ACTION_FOLLOWLEADER |
| 0x3F Combat | 0x6D9500 | 0x27 (39) | none (script-visible only as a number) |
| 0x43 DoorSaber / 0x44 StanceTotalDefense / 0x45 | 0x5DEC20 / 0x5D29F0 / 0x5D2EA0 | 0x28 / 0x29 / 0x2A | none |
| 0x46 FollowOwner | 0x5BF940 | 0x2B (43) | ACTION_FOLLOWOWNER |
| 0x47 (re-equip + resume attack) | 0x5DF920 | 0x2C (44) | none |

Not reportable (skipped in the loop): 0x02-0x06, 0x08, 0x0A, 0x0B, 0x0D, 0x0E, 0x10-0x13, 0x16, 0x17, 0x1F-0x25, 0x2C-0x2D, 0x2F-0x31, 0x33-0x35, 0x39-0x3C, 0x3E, 0x40-0x42. Consequences: a pure `ActionMoveToLocation` / `ActionMoveToObject` chain [1, 0x11, 0x13] reports MOVETOPOINT (type 1 is the only reportable one); an attack approach and an attack round report ATTACKOBJECT; `GetCurrentAction() == ACTION_MOVETOPOINT` therefore never holds during an engine attack approach (R2 E4 stuck-move detection does not see FM2 approach loops); a following companion reports FOLLOWLEADER (38) only while a 0x3D node is the last reportable of its group; `ACTION_QUEUEEMPTY` only for a truly empty list. Cost: O(nodes in the head group). The reverse mapper `0x5D7220(type,1)` (0xB -> 0x1B, 0x1E -> 0x18) is used by the UI queue code (`0x589B20`, `0x588ED0`), not by this command.

### Q4. Talent selection internals (CONFIRMED)
Handler `0x67AE40` pops (Best): `nCategory`, `nCRMax`, `oCreature`, `nInclusion`, `nExcludeType`, `nExcludeId`; (Random): `nCategory`, `oCreature`, `nInclusion`, then calls the core `0x6B8BB0(category, crMax, ...)` with `0xFFFFFFFF` as crMax for Random. Result talent = `{type, id, byte, itemId, propIdx, byte}` (`local_38..local_24`); type -1/id -1 = invalid talent.
1. **Group choice (`0x6B8BB0`)**: `rand() & 1`: 0 = spell group (`0x6BAEC0` known spells, else `0x6BAE00` class-granted Force powers, else `0x6BB3A0` item spells), 1 = feat group (`0x6BC0E0`); a group that finds nothing is marked done and the other is tried (`if ((bVar1) && (bVar2)) break;`). `nExcludeType == 0` with id `-1` drops the whole spell group, `1` with `-1` the feat group. So Best is NOT "the best talent of the category": it is a 50/50 pick between the best spell and the best feat when both groups match the category.
2. **`nCRMax` is dead**: the only compares are `006bb0ca CMP [EBP+0xc],-1` (0x6BAEC0), `006bc23e` (0x6BC0E0), `006bb707` (0x6BB3A0), `006bd4f7` (0x6BCFA0); the value is also forwarded as arg 4 of `0x6BA970`, whose asm reads only `[EBP+8]`, `[EBP+0xC]`, `[EBP+0x10]`, `[EBP+0x24]` (no `[EBP+0x14]`). So `GetCreatureTalentBest(cat, 20)` == `(cat, 1)` == `(cat, 99)`; `-1` turns Best into Random.
3. **Ranking**: spells by `row+0x134` = spells.2da column `MaxCR` (`006c169f PUSH 0x99902c` "MaxCR" ... `006c16fd MOV [EAX+0x134],ECX`); category = `row+0x130` (`006c163b PUSH 0x999034` "Category" ... `006c1699 MOV [ECX+0x130],EDX`); feats by `feat+0x14` MaxCR (`0069ee01`) with category `feat+0x10` (`0069ed97`). Highest MaxCR wins, ties picked by `rand() % n`; Random mode collects every match and picks one.
4. **Category match** (`0x6BDD60`): 4 nibbles; a nibble of 0xF in the requested code is a wildcard (`0xF8FF` = nibbles F,8,F,F). Spell rows also need `0x6BDE30`: the 2DA column `Exclusion` non-zero excludes the spell regardless of `nInclusion` (`(local_14 != 0 || param_3 != 0)` is true whenever `local_14 != 0`), i.e. `nInclusion` has no effect (decompile as written; SPECULATIVE intent).
5. **`nExcludeId` is not applied to spells**: `006bb09e CALL 0x006ba970` is preceded by `006bb07e PUSH -0x1` (arg 8) and `006bb080 MOV EDX,[EBP+0x24] / PUSH EDX` (arg 7); `0x6BA970` compares the spell against ARG 8 (`006ba97c CMP EAX,[EBP+0x24]` in its own frame) = the constant -1. Excluding one Force power id via `nExcludeId` therefore does nothing; excluding the whole type works. Feats do apply it (`param_8 == uVar1 -> skip`, 0x6BC0E0).
6. **Candidate test per known spell (`0x6BA970`)**: spell != excl, then `006ba98e CALL 0x006b8840` = `HasTalent(0, spell)` (BEFORE the category test), then area non-null and `0x6BDD60(row+0x130, category)`. So Best/Random cost = (#known spells) x HasTalent.
7. **`HasTalent(0,spell)` (`0x6B8840`)**: entry gate `(mask & 6) == 6 || !vt+0x94()`, and a party creature with HP <= 0 returns 0; spell row must exist (`0x6C1450`); for each class slot `i < stats+0x8D` with class id `[stats+0xAB+i*0x28]`: `006b89ce CALL 0x006f2a30 / 006b89d8 JZ 0x006b8a13`: **Force class** (ids 3,4,5,0xB,0xC,0xD,0xE,0xF,0x10) -> only `0x6BD9C0(0,spell,1)` (known and cost payable) AND `0x6B8F40(spell,i,0,0)` (class prerequisites; Force-point cost `0x6C4120` compared with `stats+0x130 + stats+0x132` (current FP); an effect of type `DAT_009960d0` with int0 0/1/2 rescales the cost to 50/40/30 %, direction not read); **non-Force class** -> innate list `0x6BA8F0` (known-spell entry with flag 1), else `0x6BD9C0 && 0x6B8F40`, else `006b8a6c CALL 0x006c0330` (item-spell scan). Final gate `006b8aa3 MOVZX EDX,word [EDX+0x1114] / AND EDX,8 / CMP EDX,ECX` = the creature ability mask `+0x1114` (init 0xFFFF at `005625a0`, ANDed by effect handlers `0x59C3B0`/`0x59C420`) must have bit 8 set.
8. **Item talents**: `0x6C0330` / `0x6BB3A0` / `0x6C06B0` scan the 20 equip slots (`0x6D0620(1 << i)`, only if `stats-owner +0x1150 != 0`) and then the shared repository `0x58A2C0(1)` (`+0x10` count, `0x606100(i)`); an item counts when it has a property of type 10 (`FUN_00603f60(10,0)`, ActivateItem), property active (`+0xC != 0` or stack/charges `+0x298 > 0`) and its usable-class bit test (`+0x11 == 0xFF` or `1 << +0x11 & item+0x2D4`); the spell is the property subtype (`+2`). The repository pass in `0x6BB3A0` applies only the category test (no `HasTalent`, no `Exclusion`).
9. **Use**: `ActionUseTalentOnObject` -> `0x57D8B0(type,id,target,byte,itemId,propIdx,...)`: type 0 with an item in the talent struct (`param_6 != INVALID && param_7 != 0xFFFFFFFF`) -> `0x57B4C0 UseItem`; type 0 without -> `0x6C06B0` (another inventory scan; found -> UseItem, else `AddCastSpellActions 0x565310`); type 1 (feat) -> `AddAttackActions(target, featId, ...)`; type 2 -> `UseSkill 0x57B280`. Requires `creature+0xEC != 0` (commandable).

**Implication for R4/medpac (contradiction)**: R4 row 4 says `GetCreatureHasTalent(tSpe11_05)` implies a stocked medpac for any creature. Correct statement: it holds only if the creature has a non-Force class slot (Soldier/Scout/Scoundrel/droid classes, or a multi-class PC with one); a creature whose class slots are all in {3,4,5,0xB..0x10} gets FALSE for an item-only spell even with medpacs in the repository (`006b89d8 JZ` skips `0x6C0330` for Force classes). Verify on a Jedi-only companion before relying on the guard.

## (c) Runtime probes
Installed perf-lab hooks already cover `0x668FD0` (PerfLabExecCmd, 6 B), `0x7022A0` (RunScript), `0x6699D0`, `0x670370`; the ExecuteCommand exit `0x66900A` is free. All new windows are > 12 B from every live hook and from the sibling self-patch list (checked against `live_hooks_2026-09-30g.tsv` + the `*.hooks.toml` addresses: nearest 585 B).

| id | counter / what | hook | original_bytes | expected signal |
|---|---|---|---|---|
| P7-RS | script runs per (creature, slot): slot = `(CExoString* - creature - 0x270)/8` when in 0..13, else "named script"; return address `[esp]` = engine site | `0x6FD8D0` (6) | `55 8B EC 83 EC 08` | per creature: heartbeat 1 per 3.0-4.2 s, slot 1 bursts, slot 6 about every 3 s in fights; any slot 9 hit refutes the "no Rested site" claim |
| P7-EV | event histogram (eventId, payload type word, creature party flag) and time in handler | `0x56EFF9` (6) | `89 8D 18 FD FF FF` | events 0xF/10/1 dominate; type-1 notice for flag creatures vs `0x7E5C40` result |
| P7-CMD-OUT | ExecuteCommand exit: command id (from entry hook) + result code, flags `-2000/-2001/-2002` | `0x66900A` (6) | `89 45 FC 8B 45 FC` | 0 non-zero results in vanilla; pairs with PerfLabExecCmd entry |
| P7-GNC | candidates visited per GetNearestCreature call (increment per scanned id) | `0x68433C` (6, hot) | `8B 95 58 FF FF FF` (`[EBP-0xA8]` = candidate id) | iterations / call (settles the cost estimate); runs only for candidates past the x-prune |
| P7-GNC-IN | GetNearestCreature entry: id, argc, reference object | `0x683D40` (10) | `55 8B EC 6A FF 68 F1 A4 95 00` | calls/s by caller object |
| P7-TB | Best/Random calls: category, crMax (settles "crMax ignored" at run time: vary it in a test script), coin result | `0x6B8BB0` (6) | `55 8B EC 83 EC 20` | same talent for crMax 1/20/99 |
| P7-HAS | HasTalent calls by type, caller return address (`0x6BA993` from 0x6BA970 vs `0x67AD5B` from the script command) | `0x6B8840` (6) | `55 8B EC 83 EC 4C` | calls per Best/Random = #known spells |
| P7-INV | item-scan calls and QPC time | `0x6C0330` (9) | `55 8B EC 81 EC 94 00 00 00` | calls per party-AI tick; time per call vs repository size |
| P7-ACT | `GetCurrentAction` result and head-group length: log `[EBP-8]` | `0x67B204` (10) | `8B 4D F8 51 8B 0D A8 B4 A1 00` | value 3 while a companion approaches an enemy; 0 for scripted moves; 0xFFFE with an empty queue |

## (d) Open questions
1. Poster of event 10 payload types 2 (SpellAt), 0x1B (Disturbed), 7 (Dialogue: `0x63A5B0`, AI6) and of events 8/0x13 (script named in the payload): not traced.
2. Class names for ids {3,4,5,0xB,0xC,0xD,0xE,0xF,0x10} (assumed Jedi base and prestige classes): classes.2da is not in the dump; the lead should confirm on disk that Kreia/Visas/Handmaiden have only Force class slots and that a medpac is then NOT a talent.
3. spells.2da `Exclusion` / `MaxCR` values: unknown without the 2DA (live override); needed to predict Best's winner.
4. `0x6B8F40` / `0x6C4120` internals (cost reduction effect `DAT_009960d0`, class prerequisites): only the flow read.
5. `creature+0x1114` writers by effect (`0x59C3B0`, `0x59C420`, `0x599050`, `0x597D90`): which bits map to which restriction (AI8).
6. Whether `0x5D53A0` (GetUserActionsPending) iterates the action list or a different queue: only header read.
7. Whether the `+0x17C` target of `GetLastHostileActor` is a CSWSObject or CSWSCreature field (the handler uses `vt+0x10`, not `vt+0x30`).
8. P7-ACT window `0x67B204` is a join target (inbound `JZ`/`JMP` from `0x67B14C`, `0x67B161`, `0x67B183`, `0x67B1FB`, all at the START of the window, allowed); nothing inside; the lead's `check_windows.py` should confirm.
9. Dump functions missing from `decomp/`: none needed; `0x6BCFA0` internals (class-granted pick) read only for the CRMax compare.

## (e) Struct field table
| class | offset | size | type | proposed name | readers (addr) | writers (addr) | conf | evidence |
|---|---|---|---|---|---|---|---|---|
| CSWVirtualMachineCommands | 0x04 | 4 | int | OBJECT_SELF valid flag | 0x679050, 0x67C430 | VM | high | `if (*(int *)(param_1 + 4) == 1)` in 0x679050 |
| CSWVirtualMachineCommands | 0x08 | 4 | objid | OBJECT_SELF | every self-default handler | VM | high | `FUN_0051c100(*(undefined4 *)(param_1 + 8))` in 0x66DAF0 |
| CSWVirtualMachineCommands | 0x0C | 4 | ptr | handler table (0x36D dwords) | `0x668FE5` | `0x665F6F` | high | `00668fe5 MOV ECX,[EAX+0xc]` |
| CSWSCreature | 0x270+8n (n=0..13) | 8 | CExoString+pad | script slot n | `0x6FD8D0` callers | LoadCreature/SetScript | high | `0057464f LEA ECX,[EAX+EDX*1+0x270]` in SaveCreature |
| CSWSCreature | 0x520 | 4 | int | combat state (in combat) | `0x680000` | `0x57A634` (SetCombatState), ctor `0x5622D3` | high | `local_10 = *(int *)(local_c + 0x520)` |
| CSWSCreature | 0x548 | 4 | objid | current attack target | `0x67A250`, 0x586577 | ctor `0x562B72`, `0x6DADB5`, cleared `0x6D7CE2`, `0x57A74C`, `0x56E7B5` | high | `006dadb5 MOV [ECX+0x548],EDX` |
| CSWSCreature | 0x550 | 4 | objid | attempted attack target | `0x67A2D0`, `0x568540 @0x5689AF` | `0x57E3DD` (SetAttemptedAttackTarget), ctor `0x562B92` | high | `0057e3dd MOV [EAX+0x550],ECX` |
| CSWSCreature | 0x568 | 4 | objid | attempted spell target | `0x67A340` | `0x565ECB`, `0x565F04`, `0x5652BA/F3`, clears | high | `00565ecb MOV [EAX+0x568],ECX` |
| CSWSCreature | 0x580 | 4 | objid | last hostile target | `0x681940 @0x681995` | `0x58F618` (EndCombatRound), clears `0x57A6C7`, ctor `0x562C0E` | high | `0058f618 MOV [ECX+0x580],EDX` |
| CSWSCreature | 0x11AC | 4 | int | party flag (raw value of IsObjectPartyMember) | `0x68BBE0`, `0x669FC0`, `0x563460`, 0x56EFD0 @0x570572, 0x56FA08 | `0x58A35E` (GATE) | high | `0068bc2e MOV EDX,[ECX+0x11ac]` |
| CSWSCreature | 0xEC | 4 | int | commandable | `0x67AC20`, `0x669FC0`, `0x57D8B0` | `0x691004` (SetCommandable handler) | high | `0066a001 MOV ECX,[EAX+0xec]`, `00691004 MOV [EAX+0xec],ECX` |
| CSWSCreature | 0x1114 | 2 | u16 | ability permission mask (init 0xFFFF) | `0x6B8840` (bit 8 for HasTalent, bits 6), AI actions | ctor `0x5625A0`, `0x59C409`, `0x5982D7`, `0x59912D`, `0x59C536` | med | `005625a0 MOV [EDX+0x1114],CX` / `006b8aa3 MOVZX EDX,word [EDX+0x1114]` |
| CSWSCreature | 0x1198 | 4 | ptr | stats | many | | high | `*(int *)(iVar2 + 0x1198)` in 0x696190 |
| CSWSCreatureStats | 0x146 | 2 | u16 | NPC AI style (AI2 "stance") | `0x696190`, `0x6D8508` (AI2) | SetNPCAIStyle | high | `0x696190: *(undefined2 *)(*(int *)(iVar2 + 0x1198) + 0x146)` |
| CSWSCreatureStats | 0x24 | 4 | ptr | owner creature | `0x6B8840` | | high | `(*(int **)(param_1 + 0x24))[0x46b]` |
| CSWSCreatureStats | 0x8D / 0xAB+i*0x28 | 1 / 1 | byte | class count / class id of slot i | `0x6B8840 @0x6B8988/0x6B89B9` | | high | `006b89b9 MOV AL,[EDX+ECX*1+0xab]` |
| CSWSCreatureStats | 0x130 / 0x132 | 2 / 2 | i16 | Force points (current / extra) | `0x6B8F40`, `0x6BD9C0` | | med | `local_64 = *(short *)(param_1 + 0x130) + *(short *)(param_1 + 0x132)` |
| CSWSCreatureStats | 0x134 | 4 | ptr | known-spell vector `{ptr data, int count}`, entries 12 B {id, flag, byte} | `0x6BAEC0`, `0x6BA8F0` | | high | `*(int *)(*(int *)(param_1 + 0x134) + 4)` |
| CSWRules | 0x104 | 4 | table | spells table (rows 0x1A8; +0x130 Category, +0x134 MaxCR) | `0x6C1450` callers | loader `0x6C1490` | high | `006c1699`, `006c16fd` |
| CSWRules | 0x108 | 4 | table | feat table (rows 0x50; +0x10 Category, +0x14 MaxCR, +0x28 bit 4 valid) | `0x6A20F0`, `0x6BC0E0` | loader `0x69E9C0` | high | `0069ed97`, `0069ee01` |
| CSWRules | 0x12C | 4 | table | skills table (rows 0x20; +8 Category, +0xC MaxCR) | `0x6B7BB0` | loader `0x6A17A0` | med | `006a1db8 MOV [ECX+0x8],EDX`, `006a1e19 MOV [EAX+0xC],ECX` |
| CSWSObject | 0x100 | - | list | action list (head node via `0x521060`) | `0x67B100` | AddAction | high | `0067b17a ADD ECX,0x100` |
| CSWSObject | 0x148 / 0x14C | 4 / 4 | ptr / int | effect array / count | `0x5476C0`, `0x67BD70`, `0x68F7E0` | apply/remove handlers | high | `*(int *)(param_1 + 0x14c)` in 0x5476C0 |
| CSWSObject | 0x210 | 2 | u16 | GetFirst/NextEffect cursor | `0x67BD70` | `0x67BD70` | high | `*(undefined2 *)(iVar4 + 0x210) = 0;` |
| CGameEffect | 0x1C | 4 | int | creator spell id | `0x5476C0`, `0x683D40` | CreateEffect | high | `*(... + 0x1c) == param_2` |
| CGameEffect | 0x20 | 4 | int | active flag (cleared by RemoveEffect, required by GetFirstEffect) | `0x67BD70` | `0x68F7E0` | high | `*(undefined4 *)(...+ 0x20) = 0` |
| CSWSArea | 0x194 / 0x198 | 4 / 4 | ptr / int | object id array sorted by X / count | `0x683D40`, `0x685960`, `0x522AF0` | `0x522E00`, AddObject | high | `FUN_00522af0` bisect on `+0x94` |
| CSWSArea | 0x1A0 | 4 | int | GetFirst/NextObjectInShape cursor | `0x685960` | `0x685960` | high | `*(int *)(local_68 + 0x1a0) = local_60;` |
| CSWSItem | 0x2CC | 4 | int | stack size | `0x685480` | | high | `local_8 = *(undefined4 *)(local_c + 0x2cc)` |
| CSWSItem | 0x288 / 0x2D4 | 4 / 4 | int | property count / usable-class mask | `0x6C0330` | | med | `*(int *)(iVar2 + 0x288)`, `& *(uint *)(iVar2 + 0x2d4)` |

## (f) Hook-site catalogue
Windows verified with the checker (boundary, relative branch, inbound targets from every `.asm`; none inside the windows, only function-entry call targets at the start).

| id | address | length | stolen bytes | convention / live regs | consumed exit? | EAX/stack notes | hot/cold | nearest sibling self-patch / live hook |
|---|---|---|---|---|---|---|---|---|
| AI7-RS | 0x6FD8D0 | 6 | `55 8B EC 83 EC 08` | thiscall: ECX = VM, `[esp]` = return address (engine site), `[esp+4]` CExoString*, `[esp+8]` objId, `[esp+0xC]` flag | no | EAX dead at entry | hot-ish (hundreds/s) | 8614 B (`0x6FFA76` perf-lab); `0x7022A0` perf-lab is the callee |
| AI7-EV | 0x56EFF9 | 6 | `89 8D 18 FD FF FF` | after SEH prologue: ECX = creature (stolen store writes `[EBP-0x2E8]`), `[EBP+8]` eventId, `[EBP+0xC]` caller, `[EBP+0x10]` payload | no | next insn reads EAX from the stored slot: stolen bytes write memory only | hot (every event) | 12822 B (`0x56BDE3`) |
| AI7-CMD-OUT | 0x66900A | 6 | `89 45 FC 8B 45 FC` | EAX = handler result; `[EBP-8]` = implementer (OBJECT_SELF at `[[EBP-8]+8]`) | no | stolen bytes copy EAX to a local and reload it: EAX preserved | hot (every command) | 58 B (`0x668FD0` perf-lab, entry) |
| AI7-GNC | 0x68433C | 6 | `8B 95 58 FF FF FF` | `[EBP-0xA8]` = candidate id, ECX = id | no | stolen load writes EDX (live afterwards: PUSH EDX) so a plain detour must restore EDX | hot (per scanned object) | 44997 B |
| AI7-GNC-IN | 0x683D40 | 10 | `55 8B EC 6A FF 68 F1 A4 95 00` | `[esp+4]` command id, `[esp+8]` argc, ECX = implementer | no | contains `PUSH imm` only (no relative) | warm | 46529 B |
| AI7-TB | 0x6B8BB0 | 6 | `55 8B EC 83 EC 20` | ECX = stats, `[esp+4]` category, `[esp+8]` crMax | no | | warm | 585 B (`0x6B8DF9` finesse fix) |
| AI7-HAS | 0x6B8840 | 6 | `55 8B EC 83 EC 4C` | ECX = stats, `[esp+4]` type, `[esp+8]` id | no | | warm | 1465 B |
| AI7-INV | 0x6C0330 | 9 | `55 8B EC 81 EC 94 00 00 00` | ECX = stats, `[esp+4]` spell id | no | | warm | 27328 B (`0x6B9870`) |

Notes: `0x668FD0` (perf-lab) and `0x7022A0` (perf-lab) are already hooked: do not stack a second detour; extend PerfLabExecCmd instead. AI7-RS is the better place for slot attribution (the `[esp]` return address identifies the engine site and the CExoString pointer identifies the slot).

## (g) Mod relevance
- crMax ignored / coin flip / per-known-spell HasTalent cost / no `nExcludeId` for spells: L2 (buff talent choice via `GetCreatureTalentBest(0x1408, 20)`), A1, enemy-AI (random talent pick), perf (script cost of the talent routines).
- Force-class gate on item spells and FP-cost-aware HasTalent: medpac (R4 contradiction, guard `GetCreatureHasTalent(tSpe11_05)`), L2.
- `GetCurrentAction` last-reportable semantic: E4 (stuck-move detection never fires on an attack approach), FM2/FM2b (approach loops report ATTACKOBJECT), E1/E3 (stand-off moves report MOVETOPOINT), exit5m / E7 (`ACTION_FOLLOWLEADER` test).
- `IsObjectPartyMember = +0x11AC`, ActionFollowLeader no-op, GetIsDead rule, `CSWParty::IsPartyMember 0x7E5C40`: party-flag (`+0x11AC`), FM1, FM4, E4.
- Event table and run sites (slots 3, 6, 11, 12, 1): P1/P2 (notice event for flag creatures), M8 (slot 6), enemy-E7 (`hold_enemies` relies on slot 1), FM5 (slot 12 skipped for the player), C1 (shout = event 10 type 7 / slot 7 -> AI6), ai_trace (AI7-RS / AI7-EV).
- GetNearestCreature sweep: perf (per-heartbeat script cost scales with area object count), enemy-AI, E9.
- Shape / effect cursors in the target: any script with nested `GetFirstObjectInShape` or `GetFirstEffect` loops (L2 buff scans `GetFirstEffect`): EFFOVF-adjacent.
- `GetHasSpellEffect` (`effect+0x1C == spell`, linear): L2 (see AI8 for aura semantics).
- Command table / ExecuteCommand error codes: perf, ai_trace.
