# B12b Opus gate, part r (20 REJECT/NOCAND rows)

Method: a whole-binary fp scan (GOG row vs every Steam fp row: size, retn, n_insn, disp and imm multisets, str_refs, callee count). Callees were mapped through all validated GOG->Steam rows (`B*/validated.tsv`). Every top hit was then confirmed in the Steam asm dump. Mac x86 fp gave the symbol names and the neighbour order. The dispatcher `RunActions` (GOG 0x6b18a0 = Steam 0x5d56b0) has the same fp: 6534 B, ret 0x10, 1627 insns, 101 callees. Its Steam switch (`005d5c28 JMP [EDX*4+0x5d7038]`) gives a case number for each AIAction. Those numbers follow the NWN internal action enum (0x10 WAITFORENDOFROUND, 0x24 ENCOUNTERCREATUREDESTROYSELF, 0x29 ANIMALEMPATHY, 0x2a REST, 0x2b TAUNT, 0x34 APPEAR, 0x35 DISAPPEAR, 0x36 PICKPOCKET, 0x37 FORCEFOLLOWOBJECT, 0x38 HEAL). Already-validated callees in the same switch agree with that mapping.

**Systematic finding:** every gate pick was wrong because the pipeline used the batch delta -0x118b30. For GOG 0x6c2820-0x6dee60 the true local delta is -0x10adc0, and from about 0x6db820 it is -0x10ac20. Between Appear and Disappear it is -0x10ac40, because the linker moved one ICF-folded stub. GOG 0x6f5300 sits in the CSWSCreatureStats block, where the delta is -0x3c480. With these deltas, all 20 GOG bodies have an exact fp match in Steam. Every match is unique except the two identical Rest/Taunt stubs.

| # | GOG | name | Steam | verdict | evidence |
|---|---|---|---|---|---|
| 1 | 0x6c2820 | SetDetectMode | 0x5b7a60 | verified | fp 1:1. `005b7a6e MOVZX EDX,[ECX+0x510]` and `005b7a7f MOV [EAX+0x510],CL`. Calls ComputeModifiedMovementRate 0x5889b0 (mapped). Mac SetDetectMode disp 0x510 x2 |
| 2 | 0x6c7f90 | CanEquipWeapon | 0x5bd1d0 | verified | fp 1:1. `005bd20c MOV EAX,[EDX+0x1150]` and `005bd21a CALL 0x6d0620` (GetItemInSlot). Mac disp set is identical |
| 3 | 0x6c81a0 | CanEquipWeapon2 | 0x5bd3e0 | verified | fp 1:1. `005bd408 CMP [EAX],0x40000` (GOG imm 0x40000 tells it apart from #2) |
| 4 | 0x6c8800 | CheckProficiencies | 0x5bda40 | verified | fp 1:1. `005bda87 MOV EDX,[ECX+0x1198]` and `005bda95 CALL 0x6b83f0` (Stats::HasFeat) |
| 5 | 0x6c9d80 | AIActionCheckMoveAwayFromObject | 0x5befc0 | label_suspect | fp 1:1. `005befcc MOV ECX,[EAX+0x10e8]`, GetArea callee, RunActions case 0x3c. Mac CheckMoveAwayFromObject is 947 B. Mac AIActionAreaWait (54 B, 0x10e8, GetArea) sits just before RandomWalk/TerminateClientSidePath/PartyFollowLeader, the same order GOG has. Name = **AIActionAreaWait**. The real CheckMoveAwayFromObject is at some other GOG address |
| 6 | 0x6ca5b0 | AIActionPartyFollowLeader | 0x5bf7f0 | verified | fp 1:1. `005bf89e MOVZX EAX,[EAX+0x1114]`, `005bf911 CALL 0x51c8b0`, `005bf925 MOV [EAX+0x11b0],3`. RunActions case 0x3d |
| 7 | 0x6d2130 | WUL_QW_FL_FindPath | 0x5c7370 | verified | fp 1:1, 15 callees. `005c78d8 CALL 0x5644b0` (AddMoveToPointActionToFront) and `005c792a CALL 0x54bdc0` (ComputeSafeLocation) |
| 8 | 0x6d35a0 | WUL_QW_FL_Moving_To_Path | 0x5c87e0 | verified | fp 1:1. `005c8a99 CALL 0x550980` (TestDirectLine) and `005c8bf7 CALL 0x51c280` (GetServerAIMaster) |
| 9 | 0x6d4dc0 | WUL_QW_FL_Moving_To_Offset | 0x5ca000 | verified | fp 1:1. `005ca532 CALL 0x550980`. The GOG neighbour order matches Mac (FindPath, Offset, Moving_To_Path, Following_Path, Moving_To_Offset, WaitingForPath, OffsetWaitDest) |
| 10 | 0x6d5460 | WUL_QW_FL_WaitingForPath | 0x5ca6a0 | verified | fp 1:1. `005cabdf CALL 0x550980`. Directly followed by 0x5cacd0, which part a gives as OffsetWaitDest |
| 11 | 0x6db820 | AIActionWaitForEndOfRound | 0x5d0c00 | verified | fp 1:1. `005d0c77 MOV EAX,[EDX+0xa84]` and `005d0c95 CALL 0x590180` (GetSpellActionPending). Case 0x10 |
| 12 | 0x6dbb50 | UpdateVisibleList | 0x5d0f30 | verified | fp 1:1, 14 callees, all mapped ones present. `005d1146 CALL 0x576dc0` (DoPerceptionUpdateOnCreature). Mac disp 0x194/0x198/0x260 |
| 13 | 0x6dc3f0 | AIActionRest | 0x5d17d0 | likely | Stub: `005d17d7 MOV EAX,[0x9944b8]` (ACTION_COMPLETE, the global Appear returns at its end). Only caller is RunActions case 0x2a (REST). The GOG fps for 0x6dc3f0 and 0x6dc410 are identical, so the GOG-side pairing comes from the case number, not the body |
| 14 | 0x6dc410 | AIActionTaunt | 0x5d1990 | likely | Stub: `005d1997 MOV EAX,[0x9944bc]`. Case 0x2b (TAUNT). **ICF-folded**: the same address also serves case 0x29 (AnimalEmpathy) and case 0x36 (PickPocket), so a hook here fires for all three |
| 15 | 0x6dc430 | AIActionAppear | 0x5d17f0 | verified | fp 1:1. `005d186e PUSH 0x274f`, matching GOG imm 0x274f. Case 0x34 |
| 16 | 0x6dc500 | AIActionDisappear | 0x5d18c0 | verified | fp 1:1. `005d193e PUSH 0x274e`, matching GOG imm 0x274e. Case 0x35 |
| 17 | 0x6dd5d0 | AIActionSurrenderToEnemies | 0x5d29b0 | verified | fp 1:1. `005d29bc MOV ECX,[EAX+0x10e8]` and `005d29d7 CALL 0x5d51a0` (SurrenderToEnemies). Case 0x41 |
| 18 | 0x6ddac0 | AIActionStanceMeditative | 0x5d2ea0 | verified | fp 1:1. `005d2f10 CALL 0x563d20` (ActivityManager) and `005d2f2d MOV byte [EDX+0x110c],1`. Case 0x45. Mac disp 0x110c x3 |
| 19 | 0x6dee60 | AIActionAnimalEmpathy | 0x5d4240 | label_suspect | fp 1:1. `005d4249 MOV byte [EBP-1],0`, `XOR EAX,EAX`, `RET 8`: a 2-argument stub that returns false. It sits between CheckItemCNPCRestrictions and ReplyToInvitation and is called by GOG 0x6ddee0. GOG labels in this block are rotated (0x6ddee0 "RemoveFromInvitationsOffered" has a CheckItem* body). Name unknown. The real AnimalEmpathy is 0x5d1990 (case 0x29) |
| 20 | 0x6f5300 | GetRelativeWeaponSize | 0x6b8e80 | label_suspect | fp 1:1, same delta as HasFeat (-0x3c480). `006b8e92 CALL 0x6d6e30` (CSWItem::GetBaseItem), `006b8ea9 MOV CX,[EAX+0xb2]`, `006b8ec4 CALL 0x6b83f0` (HasFeat on `this`, which is a Stats object). Name = **CSWSCreatureStats::GetWeaponFocus** (Mac: 0xb2, GetBaseItem+HasFeat) |

Label notes for the lead:
- GOG 0x6c9d80 = AIActionAreaWait.
- GOG 0x6f5300 = CSWSCreatureStats::GetWeaponFocus. Its neighbour Mac GetWeaponSpecialization uses disp 0xb0.
- GOG 0x6dee60 is not AnimalEmpathy.
- Steam 0x5d1990 is one folded body for AnimalEmpathy, Taunt and PickPocket.

sample_errors: 0 (this part has no sample rows)

AUTO meaning checks: none in this part.
