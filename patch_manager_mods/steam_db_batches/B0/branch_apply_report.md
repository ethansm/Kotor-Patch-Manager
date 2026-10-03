# B0 branch apply plan

branch rows 1704 -> shipped 1683 (same address 1644, moved 39, parity 6), removed 21

## Duplicates

none

## Moved

- id 460 CSWGuiMainMenu::OnLoadGamePicked 0x008827a0 -> 0x008824f0
- id 464 CGuiInGame::IsAnimationPlayingInDialog 0x007cb6e0 -> 0x007cadb0
- id 465 CGuiInGame::RemoveGUIPopUps 0x007d0560 -> 0x007d0620
- id 469 CSWSObject::GetLastName 0x0053f000 -> 0x0053ef50
- id 519 CSWSItem::GetUpgradableType 0x00746fc0 -> 0x006076d0
- id 633 CSWSCreatureStats::GetClassByPosition 0x00844e40 -> 0x0089aef0
- id 644 CSWSCreatureStats::GetForceResistance 0x006a7d40 -> 0x006bad10
- id 648 CSWSCreatureStats::GetWeaponFinesse 0x006a8ea0 -> 0x006b8da0
- id 649 CSWSCreatureStats::GetWeaponSpecialization 0x006a8f10 -> 0x006b8ee0
- id 650 CSWSCreatureStats::GetUseMonkAttackTables 0x006a8fe0 -> 0x006a8ea0
- id 711 CSWSCreatureStats::UnReadySpell 0x006b8ee0 -> 0x006b91b0
- id 713 CSWSCreatureStats::GetCreatureTalentRandomSpecialAbility 0x006b91b0 -> 0x006baec0
- id 714 CSWSCreatureStats::GetCreatureTalentRandomPower 0x006b9600 -> 0x006bae00
- id 741 CSWSCreatureStats::CheckInclusionMatch 0x006bc730 -> 0x006bdd60
- id 750 CSWSCreatureStats::GetIsInSpellLikeAbilityList 0x006be040 -> 0x006ba8f0
- id 959 CSWGuiInGameInventory::OnControlSelected 0x008a83d0 -> 0x008a8480
- id 1043 CSWSCombatRound::DEBUGDumpCombatRoundToScreen 0x00593090 -> 0x00593000
- id 1061 CSWSArea::SetHitInfoForCreature 0x0054d3d0 -> 0x00556040
- id 1065 CSWSArea::GridDFSTransTableInitialize 0x0054f3d0 -> 0x00552a90
- id 1066 CSWSArea::SmoothPointsOnPath 0x0054f5e0 -> 0x0054f3d0
- id 1067 CSWSArea::SmoothPointsOnPathRestartable 0x0054fb10 -> 0x0054f5e0
- id 1068 CSWSArea::SmoothSelectNodes 0x0054fdb0 -> 0x0054fe40
- id 1069 CSWSArea::SmoothSelectionRestartable 0x0054fe40 -> 0x0054fb10
- id 1075 CSWSArea::GridDFSTransTablePut 0x00552a90 -> 0x00552c90
- id 1076 CSWSArea::GridDFSTransTableGet 0x00552b40 -> 0x00552b70
- id 1093 CSWSArea::GetNextObjectInArea 0x00523250 -> 0x005232b0
- id 1094 CSWSArea::GetFirstObjectInArea 0x005232b0 -> 0x00523250
- id 1095 CSWSArea::SetMaxStealthXP 0x00523320 -> 0x0052de00
- id 1101 CSWSArea::LoadRoomInfo 0x005259b0 -> 0x005258a0
- id 1102 CSWSArea::LoadRooms 0x00525bf0 -> 0x005259b0
- id 1194 CSWCArea::ClearTilePathNodes 0x007a9950 -> 0x007aa910
- id 1202 CSWCArea::ClearAllMapPins 0x007aa910 -> 0x007a96f0
- id 1471 CSWSEncounter::AIUpdate 0x00623320 -> 0x00623330
- id 1522 CSWSEffectListHandler::OnApplySummonParty 0x00599d90 -> 0x00599a10
- id 1565 CSWSEffectListHandler::OnApplyForceResistanceIncrease 0x0059f6f0 -> 0x0059f710
- id 1581 CSWSEffectListHandler::OnApplyInvisibility 0x005a1c10 -> 0x005a1c50
- id 1592 CSWSEffectListHandler::OnApplyDispelAllMagic 0x005a2e90 -> 0x005a2ee0
- id 1608 CSWSEffectListHandler::OnApplyTimestop 0x005a4f20 -> 0x005a4d70
- id 1626 CSWSEffectListHandler::OnApplyNegativeLevel 0x005a65c0 -> 0x005a65d0

## Removed

- id 476 Gob::SetProcessFlag 0x00461ed0 (unresolved) reviewer mismatch: Size and callee differ from GOG; candidate sets a +0x193 byte flag. Possibly 0x459560/0x464bd0-type w
- id 710 CSWSCreatureStats::DecrementFeatRemainingUses_2 0x006b8da0 (label_suspect) override: [merge] GOG label DecrementFeatRemainingUses_2 is wrong: body = GetWeaponFinesse (f2 side finding: callees Get
- id 731 CSWSCreatureStats::SetArcaneSpellFailure 0x006bae00 (label_suspect) override: [merge] GOG label SetArcaneSpellFailure is wrong: body = GetCreatureTalentRandomPower (e2 finding 4: 181 B, di
- id 732 CSWSCreatureStats::GetTotalDamageRange 0x006baec0 (label_suspect) override: [merge] GOG label GetTotalDamageRange is wrong: body = GetCreatureTalentRandomSpecialAbility (e2 finding 4: di
- id 738 CSWSCreatureStats::ComputeNumberKnownSpellsLeft 0x006bc470 (label_suspect) override: [b] skill idx -> feat 0xb7..0xbd then HasFeat (006bc499 MOV EAX,0xb7, 006bc4f5 CALL 0x6b83f0); Mac ComputeNumb
- id 749 CSWSCreatureStats::AddSpellLikeAbilityToList 0x006bdd60 (label_suspect) override: [b] = CheckInclusionMatch: nibble wildcard match (006bdd73 AND EAX,0xf000); callers TalentRandomPowerFromItem/
- id 854 CTwoDimArrays::GetSwoopUpgradeRowProvider 0x0051c8d0 (label_suspect) override: [a] Body = CServerExoApp::GetPartyTable (Mac 0x324362, 16 B disp 0x4): 0051c8dd ADD EAX,0x1f0b4 on [this+4]; c
- id 912 CExoResMan::FreeResourceData_2 0x00716d80 (unresolved) reviewer unsure
- id 1077 CSWSArea::PlotSoundPath 0x00552b70 (label_suspect) override: [b] fits Mac GridDFSTransTableGet (two-probe lookup, disp 0x1b8, imm 0x100; caller GridDFSearch 0x551c30): 005
- id 1078 CSWSArea::PathPointDFSTransTableInitialize 0x00552c90 (label_suspect) override: [b] fits Mac GridDFSTransTablePut (store 4 values at [+0x1b8], caller GridDFSearch 0x551c30): 00552d26 ADD ECX
- id 1137 CSWSArea::Constructor_2 0x0052be60 (rejected) override: [f2] not a function: 2nd chunk of ClearLineOfSight (GOG 0x720d10+0xb0; 176+352+239=767 B = Steam 0x52bb60, par
- id 1138 CSWSArea::Destructor_3 0x0091c7cf (rejected) override: [merge] not a function: 3rd Ghidra chunk of ClearLineOfSight 0x720d10 (176+352+239=767 B = Steam 0x52bb60; e2 
- id 1155 CSWSArea::GetClientArea_2 0x0052de00 (label_suspect) override: [b] = SetMaxStealthXP (Mac disp 0x2e8,0x2ec,0x2ec exact): 0052de27 MOV [ECX+0x2e8],EDX, clamps +0x2ec. Real Ge
- id 1186 CSWCArea::AddWeatherObject 0x007a9640 (label_suspect) override: [a] Body = Mac CSWCArea::AddMapPin (imm 0x1c8): 007a964e ADD ECX,0x1c8 / 007a9654 CALL 0x0083ea60 (array Add).
- id 1187 CSWCArea::GetWeatherObject 0x007a9660 (label_suspect) override: [f1] = GetMapPin: 007a9699 MOV EDX,[ECX+0x1c8] / 007a96b1 MOV EAX,[EDX+0x108]; Mac GetMapPin disp 0x108,0x10c,
- id 1188 CSWCArea::ClearWeatherObjects 0x007a96f0 (label_suspect) override: [f1] = ClearAllMapPins: 007a9722 MOV EDX,[ECX+0x1c8], 007a9770 CALL 0x840ef0 SetSize; caller UnloadArea 0x7a5f
- id 1216 CSWSPlaceable::DoDamage 0x005ee290 (unresolved) reviewer mismatch: Body unrelated to GOG DoDamage wrapper; GOG neighbours are CSWSDoor, no Steam slot at 0x61d1xx
- id 1360 CVirtualMachineStack::ClearStack 0x006fd6b0 (rejected) gog_db_param_bad;why:G4 steam retn 0 vs GOG retn 12 (no thunk explanation)
- id 1498 CSWSEffectListHandler::OnRemoveForceSight 0x00593a10 (unresolved) reviewer unsure
- id 1645 CSWSEffectListHandler::OnApplyEffectImmunity 0x005a8820 (unresolved) reviewer mismatch: Candidate is OnApplyBodyFuel flag setter; no Steam slot between 0x5a8820 and 0x5a8860
- id 1655 CSWSItemPropertyHandler::Constructor 0x005aa470 (label_suspect) override: [c] body = base dtor ~CItemPropertyApplierRemover: 005aa47a MOV [EAX],0x99247c (base vftable; Handler vptr is 
