# B0 Opus gate, part e2 (102 rows, reviewer `match` escalation)

## Method
For every row I compared:
- the GOG fp row with the Steam fp row: size, retn, n_insn, disp, imm, string count, and callee count plus the sorted callee sizes;
- the GOG callee names with the callee names of the Mac x86 symbol of the same name;
- the GOG neighbour labels (two on each side).

For the effect handlers, I also parsed the Steam handler table in `CSWSEffectListHandler::InitializeEffects` (0x593a30): table +4 holds the apply handlers and table +8 the remove handlers. Each row's slot was checked two ways:
- against the type constant the body itself tests (for example, OnRemoveACDecrease rescans type 0x31 and sits in remove[0x31]);
- against the NWN true-type order where it applies. That order matches every type seen in this part: 8/9 SetState, 0xa-0xe Attack/Damage, 0xf TempHP, 0x10/0x11 DamageImmunity, 0x12 Entangle, 0x14 Knockdown, 0x15 Deaf, 0x17 SetAIState, 0x18 EnemyAttackBonus, 0x19 ArcaneSpellFailure, 0x1a/0x1b SavingThrow, 0x1c/0x1d MovementSpeed, 0x1e VisualEffect, 0x1f AoE, 0x20 Beam.

All 194 Steam asm addresses quoted in `overrides_e2.tsv` were re-checked on disk in `steam_6a522e71/decomp` and are present as quoted. The string addresses were checked in `strings.tsv` or in the exe bytes.

"fp=" means the GOG and Steam fingerprints are identical. 93 of the 102 rows are fp=. The other 9 differ only in these ways:
- **String count:** Steam always has one fewer (LoadPCAutoSave, LoadJournal, SaveMember/PuppetByObject, LoadTableInfo, LoadItems, OnApplyAreaOfEffect).
- **Size:** 1893 vs 1894 B (CSWSCreatureStats ctor).
- **Ghidra split:** the GOG function is split into chunks (ClearLineOfSight, see below).

No row has a GOG/Steam layout difference.

**Verdicts: verified 98, likely 2, label_suspect 2, rejected 0, unresolved 0.** No Steam address changes.

## Per-row evidence (short)
| GOG | name (GOG DB) | Steam | verdict | evidence |
|---|---|---|---|---|
| 0x5eeb30 | CSWGuiMainCharGen::BeginPlayGame | 0x907340 | verified | calls RemoveSubPanels 0x906110 (510 B) + ClassSelection::BeginPlayGame 0x8faca0 (1151 B) on [+0x6c]; fp twin 0x8f6270 = GOG twin 0x5e8d50 LevelUpCharGen::HandleCancelButton |
| 0x53ccd0 | CSWGuiMainMenu::OnNewGamePicked | 0x882230 | verified | fp= unique; 'HD0:effects', 'MODULES:', AddResourceDirectory |
| 0x90cb30 | CSWGuiPanel::Constructor | 0x40e5a0 | verified | real ctor, not a dtor: stores vtable 0x98739c (also set by dtor 0x40e860 = GOG ~CSWGuiPanel 276 B), new(0x78) + CSWGuiBorder ctor; 77 callers |
| 0x5d1d20 | CSWGuiSaveLoad::LoadPCAutoSave | 0x8529e0 | verified | 'AUTOSAVE', '%06d - %s'; 45 callees size-equal |
| 0x5bb0e0 | CSWGuiUpgrade::Destructor | 0x8cad20 | verified | deleting dtor -> body 0x8cad50 (1248 B = GOG 0x5bb110) + delete |
| 0x4ecc00 | CSWItem::GetBaseItem | 0x6d6e30 | verified | fp= unique; [+0xc] id, [g+0x34] array, CSWBaseItemArray::GetBaseItem 0x6d5200 |
| 0x6fd830 | CSWPartyTable::LoadJournal | 0x5f7cc0 | verified | 'JNL_Entries'; Mac callee names 12/13 |
| 0x6fda30 | CSWPartyTable::UpdateMembers | 0x5f7ec0 | verified | 12x SaveMember + 3x SavePuppet; Mac same callees |
| 0x6fdca0 | CSWPartyTable::SaveMemberByObject | 0x5f8130 | verified | bound 0xc, GetFilename |
| 0x6fe090 | CSWPartyTable::SavePuppetByObject | 0x5f8520 | verified | bound 3, GetPupFilename |
| 0x6fec70 | CSWPartyTable::GetFilename | 0x5f9100 | verified | 'AVAILNPC%d' (fp twin 0x5f9190 uses AVAILPUP%d) |
| 0x6fed00 | CSWPartyTable::GetPupFilename | 0x5f9190 | verified | 'AVAILPUP%d' |
| 0x700170 | CSWPartyTable::AddNPC_2 | 0x5fa600 | verified | GetIsNPCAvailable + AddNPC(960 B); Mac template overload of AddNPC |
| 0x7002b0 | CSWPartyTable::AddPUP_2 | 0x5fa740 | verified | GetIsPUPAvailable + AddPUP(586 B) |
| 0x7005f0 | CSWPartyTable::SetNPCObject | 0x5faa80 | verified | GetIsNPCAvailable, store [+0x1c+i*4] |
| 0x701d10 | CSWPartyTable::LoadTableInfo | 0x5fbf60 | verified | 'PARTYTABLE', calls LoadJournal 0x5f7cc0 |
| 0x7035a0 | CSWPartyTable::GetNPCID | 0x5fdbf0 | verified | 12-slot search at +0x1c; Mac disp 0x1c |
| 0x7035f0 | CSWPartyTable::GetPUPID | 0x5fdc40 | verified | 3-slot search at +0x14c; Mac disp 0x14c |
| 0x703770 | CSWPartyTable::ClearMembers | 0x5fddc0 | verified | fills 12 x +0x1c with 0x7f000000 |
| 0x7037b0 | CSWPartyTable::ClearPuppets | 0x5fde00 | verified | fills 3 x +0x14c |
| 0x7037f0 | CSWPartyTable::ClearPuppet | 0x5fde40 | verified | one +0x14c slot, bound 3, RET 4; Mac 0x14c |
| 0x703820 | CSWPartyTable::ClearMember | 0x5fde70 | verified | one +0x1c slot, bound 0xc; Mac 0x1c |
| 0x703960 | CSWPartyTable::GivePlotXP_2 | 0x5fdfb0 | verified | calls C2DA::GetINTEntry 0x71bcf0 (= GOG 0x621790, the callee of 0x703960); twin 0x5fdea0 calls 0x71bad0 (= GOG 0x621570) |
| 0x703d60 | CSWPartyTable::GetIsLeader | 0x5fe3b0 | verified | reads +0x114 only |
| 0x703dc0 | CSWPartyTable::SetLeader | 0x5fe410 | verified | writes +0x114 |
| 0x7049b0 | CSWPartyTable::ResetCurrentSessionStartTime | 0x5ff000 | verified | GetSystemTimeAsFileTime import (Mac same) |
| 0x7049f0 | CSWPartyTable::GetPCFirstName | 0x5ff040 | verified | decompile: SubString(0, Find(' ')) of [+0x240] |
| 0x704b00 | CSWPartyTable::GetPCLastName | 0x5ff150 | verified | decompile: SubString(idx+1, -1) |
| 0x704df0 | CSWSArea::ComputeAwayVector | 0x54aa50 | verified | +0x1b0, TestDirectLine, frame 0x194 |
| 0x705280 | CSWSArea::ComputeBestCorner | 0x54aee0 | verified | TestDirectLine + sqrt, RET 0x30 |
| 0x709fd0 | CSWSArea::SmoothSelectNodes | 0x54fdb0 | **label_suspect** | body = SmoothSelection (see finding 1) |
| 0x716560 | CSWSArea::Constructor | 0x521360 | verified | CSWArea ctor + InitializeStaticObjects, RET 0xc |
| 0x716b50 | CSWSArea::Destructor | 0x5219a0 | verified | deleting dtor -> body 0x5219d0 (1167 B = GOG 0x716b80) |
| 0x718510 | CSWSArea::DecrementPlayersInArea | 0x523360 | verified | [+0x12c]-1, DecreaseAILevelPriority; fp twin 0x523320 is Increment (part b) |
| 0x71ab60 | CSWSArea::LoadRooms | 0x5259b0 | verified | stride 0x4c, CSWSRoom::LoadWalkMesh |
| 0x71b570 | CSWSArea::LoadItems | 0x5263c0 | verified | new(0x30c) + CSWSItem ctor |
| 0x720d10 | CSWSArea::ClearLineOfSight | 0x52bb60 | verified | GOG fp is split (see finding 3); union = Steam; NoNonWalkPolysDetailed, +0x250/+0x254 (Mac) |
| 0x7221c0 | CSWSArea::NoNonWalkPolys | 0x52d0e0 | verified | sole callee NoNonWalkPolysDetailed, RET 0x20 |
| 0x722d10 | CSWSArea::DetachFromClientArea | 0x52dc30 | verified | [+0x334]=0; Mac 0x334 |
| 0x723250 | CSWSArea::SetPlayerRestrictMode | 0x52e170 | verified | UnstealthParty, +0x2e4 (GOG callee 0x64c2f0 is the known GetPartyTable mislabel from part a) |
| 0x7232a0 | CSWSArea::GetNumMinesSetByPlayerParty | 0x52e1c0 | verified | trigger loop, 0x2fc/0x354 |
| 0x723350 | CSWSArea::GetPlayerCanSetMines | 0x52e270 | verified | count < 0xf |
| 0x7e6390 | CSWSAreaOfEffectObject::Destructor | 0x62c1c0 | verified | deleting dtor -> 0x62c1f0 (357 B = GOG 0x7e63c0) |
| 0x774f50 | CSWSCombatRound::SetCurrentAction | 0x5908a0 | verified | byte store +0xb24; Mac 0xb24 |
| 0x774fa0 | CSWSCombatRound::ClearAllAttacks | 0x5908f0 | verified | 5x GetAttack + ClearAttackData |
| 0x776910 | CSWSCombatRound::GetTotalAttacks | 0x592260 | verified | +0xae0 + +0xae4; Mac same |
| 0x776930 | CSWSCombatRound::SetBonusEffectAttacks | 0x592280 | verified | store +0xaf4; Mac same |
| 0x6a3970 | CSWSCreature::GetClientCreature | 0x58ae90 | verified | GetClientObject + vcall +0x2c |
| 0x6e06e0 | CSWSCreatureStats::Constructor | 0x6a4260 | verified | ctor (not a dtor): 16 callees size-equal, all 110+ disps equal; SetClassLevel |
| 0x6e10b0 | CSWSCreatureStats::GetFullName | 0x6a4c30 | verified | GetLocStringServer on +0x34/+0x3c |
| 0x6e35f0 | CSWSCreatureStats::GetAttacksPerRound | 0x6a7170 | likely | 23 B `return 1`, positional only |
| 0x6e5460 | CSWSCreatureStats::GetUseMonkAttackTables | 0x6a8fe0 | **label_suspect** | die-size ladder, see finding 2 |
| 0x6f2f70 | CSWSCreatureStats::CalcStatModifier | 0x6b6af0 | verified | (x-10)/2 with floor |
| 0x6f2fb0 | CSWSCreatureStats::GetTag | 0x6b6b30 | likely | 20 B getter [+0x24]+0x18, no Mac GetTag symbol |
| 0x6f3760 | CSWSCreatureStats::GetExpNeededForLevelUp | 0x6b72e0 | verified | GetLevel + table +0x38 |
| 0x6f3820 | CSWSCreatureStats::GetBaseFortSavingThrow | 0x6b73a0 | verified | callee 0x6f0920 reads class +0x56 = Mac CSWClass::GetFortSaveBonus (0x56); GOG callee 0x50efd0 same disp |
| 0x6f3c60 | CSWSCreatureStats::GetBaseReflexSavingThrow | 0x6b77e0 | verified | callee 0x6f0950 reads +0xce = Mac GetRefSaveBonus (Will = +0x92) |
| 0x6f47f0 | CSWSCreatureStats::ClearFeats | 0x6b8370 | verified | DelIndex + delete loop |
| 0x6f5030 | CSWSCreatureStats::GetCreatureTalentRandom | 0x6b8bb0 | verified | rand & 0x80000001; its 4 callees match Mac's 4 talent callees by fp (see finding 4: two GOG callee labels are wrong, the row itself is right) |
| 0x6f53c0 | CSWSCreatureStats::GetSpellUsesLeft | 0x6b8f40 | verified | Mac callee names 5/5 |
| 0x6f6ec0..0x6f7000 | GetTotal DEX/CON/INT/WIS/CHA Bonus | 0x6baa40, 0x6baa90, 0x6baae0, 0x6bab30, 0x6bab80 | verified x5 | PUSH 1/2/3/4/5 before GetTotalEffectBonus; Mac imm index 2..5 for CON..CHA |
| 0x6f8850 | CSWSCreatureStats::SetForceResistance | 0x6bc3d0 | verified | +0x12b + UpdateCombatInformation |
| 0x6f8880 | CSWSCreatureStats::SetForceResistancePenalty | 0x6bc400 | verified | +0x12c + UpdateCombatInformation |
| 0x762410 | CSWSDoor::Destructor | 0x616f60 | verified | deleting dtor -> 0x616f90 (500 B = GOG 0x762440) |
| 0x736240 | CSWSEffectListHandler::Destructor_2 | 0x593960 | verified | non-deleting dtor of the class whose vtable 0x992490 has [1]=InitializeEffects 0x593a30, [3]=OnEffectRemoved 0x5947a0 (vtable bytes read from the exe; built in 0x51d2f0); deleting dtor 0x51d4b0 calls it. Its fp twin 0x5aa3c0 writes the item-property handler vtable 0x992468 (not this class). |
| 0x737080 | CSWSEffectListHandler::OnEffectRemoved | 0x5947a0 | verified | vtable slot 3; remove table +8, limit 0x76 |
| 0x7370f0 .. 0x741e90 | 32 effect handlers | see TSV | verified x32 | handler-table slot anchor per row (InitializeEffects store line quoted), self-type constant or NWN order, Mac callee names |

Folded bodies (one Steam body, several slots, no verdict change):
- 0x59aa40 is remove[0xa,0xb,0xd,0xe,0x1a,0x1b]. Mac has six identical 49 B OnRemove{SavingThrow,Attack,Damage}{Increase,Decrease} stubs. The GOG DB names the body after one of them (OnRemoveDamageDecrease), so the row ships under that name and it is correct for remove[0xe].
- 0x599d90 is remove[6] and remove[0x59] (SummonCreature/SummonParty).
- 0x59ea10 is remove[0x1c] and remove[0x1d].

The fp twins OnApplyAttackIncrease 0x59a9e0 and OnApplySavingThrowIncrease 0x599de0 differ only in instruction order. The table settles which is which: apply[0xa] = 0x59a9e0, apply[0x1a] = 0x599de0.

## GOG label errors found (for GOG_LABEL_SUSPECTS.md)
1. **CSWSArea Smooth\* block rotation.** This confirms and extends part b finding 1. Mac call graph:
   - SmoothPointsOnPath -> SmoothSelection -> SmoothSelectNodes;
   - SmoothPointsOnPathRestartable -> SmoothSelectionRestartable -> SmoothSelectNodesRestartable.

   | GOG | GOG label | body is | evidence | Steam |
   |---|---|---|---|---|
   | 0x709980 | SmoothPointsOnPath | SmoothPointsOnPathRestartable | calls SmoothCornerOptimize 0x709200 + 0x709d30; disps 0x214-0x234 (Mac Restartable 753 B, same disps) | 0x54f5e0 (fp=) |
   | 0x709d30 | SmoothPointsOnPathRestartable | SmoothSelectionRestartable | calls 0x70a4f0 only; disps 0x1b0, 0x214-0x234 (Mac 750 B) | 0x54fb10 (fp=) |
   | **0x709fd0** | **SmoothSelectNodes** | **SmoothSelection** | 135 B, calls 0x70a060 + free; caller 0x709770 = SmoothPointsOnPath (part b). Mac SmoothSelection 200 B calls SmoothSelectNodes + delete[] | 0x54fdb0 (this row) |
   | 0x70a060 | SmoothSelectionRestartable | SmoothSelectNodes | 1164 B; callees TestDirectLine 0x70aba0, magnitude 0x80c0a0, ComputePathDistance 0x705930; disp 0x14, 0x1b0. Mac SmoothSelectNodes 991 B, same 3 callees | 0x54fe40 (fp=) |
   | 0x70a4f0 | SmoothSelectNodesRestartable | correct | disps 0x230-0x238 as Mac | - |
2. **0x6e5460 `CSWSCreatureStats::GetUseMonkAttackTables`** is a monk unarmed-damage die-size function.
   - Body: GetUseMonkAbilities, +0x8d, +0x53c, and a constant ladder 4,6,8,0xa,0xc,0x10,0x14,0x20 (`006a90cf MOV [EBP-0x10],0x14`).
   - Mac has no standalone counterpart; the logic is inlined into GetUnarmedDamageRoll / GetTotalDamageRange. By NWN naming it is probably GetUnarmedDamageDie, and 0x6e5390 `GetWeaponSpecialization` its Dice sibling. Unconfirmed.
   - The real GetUseMonkAttackTables is GOG 0x6e5320 (labelled GetWeaponFinesse) = Steam 0x6a8ea0, as part b already proposed.
3. **0x720dc0 `CSWSArea::CSWSArea` and 0x720f20 `CSWSArea::~CSWSArea`** are not functions. They are the 2nd and 3rd chunks of ClearLineOfSight 0x720d10.
   - Sizes: 176 + 352 + 239 = 767 B, contiguous.
   - The union of the three fps equals Steam 0x52bb60 exactly: disp, imm, n_insn 221, and the 8 callee sizes.
   - The middle chunk calls NoNonWalkPolysDetailed 0x722210 and GetGameObject 0x64bac0, as Mac ClearLineOfSight does.
   - The real ctor/dtor are 0x716560 / 0x716b80.
4. **CSWSCreatureStats talent block.** This extends part b finding 5.

   | GOG | GOG label | body is (Mac evidence) | Steam (fp=) |
   |---|---|---|---|
   | 0x6f7280 | SetArcaneSpellFailure | GetCreatureTalentRandomPower: 181 B, disp 0x8d,0x8d,0xab, imm 0x28 (Mac Power 161 B: 0x8d,0x8d,0xab, imm 0x28; Mac SetArcaneSpellFailure is a 5 B stub) | 0x6bae00 |
   | 0x6f7340 | GetTotalDamageRange | GetCreatureTalentRandomSpecialAbility: disp 0x104, 0x134 x8, imm 0xfe, 0xffffff9c (Mac SpecialAbility: 0x104, 0x134 x4, 0xfe, 0xffffff9c; Mac GetTotalDamageRange is a 1555 B damage function) | 0x6baec0 |
   | 0x6f5630 | GetCreatureTalentRandomSpecialAbility | GetCreatureTalentRandomKnownPower: disp 0x130/0x132/0x18a/0x1110/0x1198, imm 0x1e,0x28,0x32,0x94 (Mac KnownPower 1033 B, same values) | 0x6b91b0 |

   GetCreatureTalentRandom 0x6f5030 calls 0x6f7280 and 0x6f7340 exactly where Mac calls Power and SpecialAbility.

## mac_contra / layout
None in this part changes a verdict. The Mac differences are compiler or layout differences: the Mac vtable is +4, CGameEffect is +0x30 -> +0x34, and Mac handler tables use 8-byte entries.

## AUTO meaning checks
None: part e2 has no AUTO rows.

## Sample
Part e2 has no rows from "Sample of the other reviewer matches". All 102 rows are escalation rows. Among them, 2 reviewer `match` rows have a wrong GOG label (0x709fd0, 0x6e5460); their body counterpart is right. No reviewer `match` row has a wrong Steam address.

sample_errors: 0
