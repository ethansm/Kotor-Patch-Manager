# Gate report B15

Rows: 656. Source order for tie-breaks: server > trusted name_map > mapper:vote > branch > seeded > other.

## Counts per class and gate class

| class | AUTO | REVIEW | REJECT | NOCAND | total |
|---|---|---|---|---|---|
| CSWVirtualMachineCommands | 13 | 5 | 0 | 0 | 18 |
| Global | 461 | 177 | 0 | 0 | 638 |
| **all** | 474 | 182 | 0 | 0 | 656 |

AUTO by route: c2=159 sig=315 (the sig route is non-propagating, see DESIGN 4.2)

## REJECT rows

(none)

## Duplicates and conflicts (G6)

- Global::ExecuteCommandAddMultiClass gog 0x00781d80 -> 0x0066c7a0: review:held_by=Global::ExecuteCommandLevelUpHenchman
- Global::ExecuteCommandChangeFaction gog 0x00784680 -> 0x0066f330: review:held_by=Global::ExecuteCommandClearAllActions
- Global::ExecuteCommandDistanceConversions gog 0x00786270 -> 0x00670c90: review:held_by=CSWSCreature::CanEquipWeapon2
- Global::ExecuteCommandGetAttemptedSpellTarget gog 0x0078f920 -> 0x0067a340: review:held_by=Global::ExecuteCommandGetAttemptedMovementTarget
- Global::ExecuteCommandGetAttemptedMovementTarget gog 0x0078f990 -> 0x0067a3b0: review:held_by=Global::ExecuteCommandGetSpellTarget
- Global::ExecuteCommandGetCommandable gog 0x00790200 -> 0x0067ac20: review:held_by=CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_FindPath
- Global::ExecuteCommandGetCreatureTalent gog 0x00790420 -> 0x0067ac20: review:held_by=CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_FindPath
- Global::ExecuteCommandGetEmotions gog 0x00791a10 -> 0x0067c250: review:held_by=Global::ExecuteCommandGetEffectSpellId
- Global::ExecuteCommandGetGoldPieceValue gog 0x00793530 -> 0x0067df50: review:held_by=CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_WaitingForPath
- Global::ExecuteCommandGetIsDay gog 0x00794f90 -> 0x0067f9b0: review:held_by=Global::ExecuteCommandGetIsDoorActionPossible
- Global::ExecuteCommandGetLastAttackAction gog 0x00796f90 -> 0x006819c0: review:held_by=Global::ExecuteCommandGetLastAttackMode
- Global::ExecuteCommandGetWasForcePowerSuccessful gog 0x007971d0 -> 0x00681c00: review:held_by=Global::ExecuteCommandGetLastAttackType
- Global::ExecuteCommandGetNearestObject gog 0x00799310 -> 0x006839a0: review:held_by=Global::ExecuteCommandGetModuleName
- Global::ExecuteCommandGetPartyLeader gog 0x0079cce0 -> 0x0069b7c0: review:held_by=Global::ExecuteCommandRemoveNPCFromPartyToBase
- Global::ExecuteCommandGetPlaceableIllumination gog 0x0079cd60 -> 0x00687710: review:held_by=Global::ExecuteCommandGetPartyLeader
- Global::ExecuteCommandIsAIState gog 0x007a0b00 -> 0x0068b3c0: review:held_by=Global::ExecuteCommandIntToString
- Global::ExecuteCommandMoveToPoint gog 0x007a2530 -> 0x0068cb50: review:held_by=Global::ExecuteCommandMoveToObject
- Global::ExecuteCommandSetBonusForcePoints gog 0x007af710 -> 0x0069a0c0: review:held_by=Global::NodeHasProjection
- Global::ExecuteCommandSavePUPByObject gog 0x007b1910 -> 0x0069c340: review:held_by=Global::ExecuteCommandGetIsPlayerMadeCharacter

## G4 metadata flags

- CSWVirtualMachineCommands::ExecuteCommandActionDoCommand gog 0x0077efb0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWVirtualMachineCommands::ExecuteCommandActionEquipMostDamagingMelee gog 0x0077f030: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandAddAvailableNPCByTemplate gog 0x00781620: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWVirtualMachineCommands::ExecuteCommandAttack gog 0x007830d0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandChangeFaction gog 0x00784680: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandCreateObject gog 0x00784f70: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandEffectForceJump gog 0x0078cbc0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandFloatToString gog 0x0078ec20: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandForceHeartbeat gog 0x0078edf0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandGetAlignment gog 0x0078f3b0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandGetArea gog 0x0078f650: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandGetClassInformation gog 0x0078ff50: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandGetHasFeat gog 0x00793790: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandGetHasSpell gog 0x00793b70: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandGetMaxForcePoints gog 0x00794030: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandAdjustCreatureAttributes gog 0x0079c250: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandAdjustCreatureSkills gog 0x0079c4b0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandModifyBaseSavingThrow gog 0x0079c5c0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandInfluence gog 0x0079c800: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandGetSkillRank gog 0x0079d630: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandGrantAbility gog 0x0079db00: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandIsStealthed gog 0x007a1340: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandPrintVector gog 0x007a4a20: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWVirtualMachineCommands::ExecuteCommandRandom gog 0x007a4c20: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandSetXP gog 0x007a81d0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandShowPartySelection gog 0x007a8330: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandStartNewModule gog 0x007a9260: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandVector gog 0x007aaac0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandGetScriptParameter gog 0x007af2f0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandSetBonusForcePoints gog 0x007af710: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- Global::ExecuteCommandIsRunning gog 0x007b06b0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)

## Trusted pair body conflicts

Trusted rows that are NOT seeds (exact name + equal retn + equal size or A-SIG >= 0.5 with intersection >= 3 required, Opus gate R1). They stay trusted as G6 holders until Opus rules on them.

- GOG 0x00815520 CAurTexture::SetIsEnvironmentMapped -> Steam 0x00422e20: name is not exact (suffix on the trusted row)
- GOG 0x0084b0b0 Gob::SetScene -> Steam 0x00461aa0: retn GOG 0 vs Steam 4
- GOG 0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam 0x0051e170: name is not exact (suffix on the trusted row)
- GOG 0x0064e730 CServerAIMaster::EventPending -> Steam 0x0051ed10: name is not exact (suffix on the trusted row)
- GOG 0x00720d10 CSWSArea::ClearLineOfSight -> Steam 0x0052bb60: size GOG 176 vs Steam 767, SIG 0.35 (intersection 4)
- GOG 0x007184d0 CSWSArea::SetMaxStealthXP -> Steam 0x0052de00: retn GOG 0 vs Steam 4
- GOG 0x00709980 CSWSArea::SmoothPointsOnPath -> Steam 0x0054f3d0: size GOG 938 vs Steam 518, SIG 0.44 (intersection 7)
- GOG 0x00709d30 CSWSArea::SmoothPointsOnPathRestartable -> Steam 0x0054f5e0: retn GOG 16 vs Steam 0
- GOG 0x0070a060 CSWSArea::SmoothSelectionRestartable -> Steam 0x0054fb10: retn GOG 24 vs Steam 16
- GOG 0x00709fd0 CSWSArea::SmoothSelectNodes -> Steam 0x0054fe40: retn GOG 16 vs Steam 24
- GOG 0x00709770 CSWSArea::GridDFSTransTableInitialize -> Steam 0x00552a90: size GOG 518 vs Steam 173, SIG 0.08 (intersection 1)
- GOG 0x0070cd60 CSWSArea::GridDFSTransTableGet -> Steam 0x00552b70: retn GOG 8 vs Steam 16
- GOG 0x0070ccb0 CSWSArea::GridDFSTransTablePut -> Steam 0x00552c90: retn GOG 0 vs Steam 16
- GOG 0x00707770 CSWSArea::SetHitInfoForCreature -> Steam 0x00556040: retn GOG 32 vs Steam 20
- GOG 0x00667710 CSWSItem::RestoreUsedActiveProperties -> Steam 0x00603dd0: size GOG 63 vs Steam 245, SIG 0.17 (intersection 3)
- GOG 0x00667600 CSWSItem::GetPropertyByTypeExists -> Steam 0x00603f60: retn GOG 4 vs Steam 8
- GOG 0x006fcf40 CSWSItem::SetNumActiveProperties -> Steam 0x00604140: size GOG 85 vs Steam 133, SIG 0.33 (intersection 2)
- GOG 0x0067a210 CSWSItem::non_virtual_thunk_to_dtor_CSWSItem -> Steam 0x00616a60: size GOG 8 vs Steam 44, SIG 0.00 (intersection 0)
- GOG 0x007ef060 CFactionManager::SetNPCFactionReputation -> Steam 0x006644d0: name is not exact (suffix on the trusted row)
- GOG 0x007ef110 CFactionManager::GetNPCFactionReputation -> Steam 0x00664580: name is not exact (suffix on the trusted row)
- GOG 0x007ef1e0 CFactionManager::GetIsNPCFaction -> Steam 0x00664650: name is not exact (suffix on the trusted row)
- GOG 0x00780d50 Global::ExecuteCommandChangeObjectAppearance -> Steam 0x0066b770: name is not exact (suffix on the trusted row)
- GOG 0x00780e80 Global::ExecuteCommandCreatureFlourishWeapon -> Steam 0x0066b8a0: name is not exact (suffix on the trusted row)
- GOG 0x006e5460 CSWSCreatureStats::GetUseMonkAttackTables -> Steam 0x006a8ea0: retn GOG 0 vs Steam 4
- GOG 0x006e5320 CSWSCreatureStats::GetWeaponFinesse -> Steam 0x006b8da0: size GOG 110 vs Steam 217, SIG 0.06 (intersection 1)
- GOG 0x006e5390 CSWSCreatureStats::GetWeaponSpecialization -> Steam 0x006b8ee0: retn GOG 0 vs Steam 4
- GOG 0x006f5360 CSWSCreatureStats::UnReadySpell -> Steam 0x006b91b0: size GOG 83 vs Steam 1090, SIG 0.05 (intersection 1)
- GOG 0x006fa4c0 CSWSCreatureStats::GetIsInSpellLikeAbilityList -> Steam 0x006ba8f0: retn GOG 0 vs Steam 4
- GOG 0x006e41c0 CSWSCreatureStats::GetForceResistance -> Steam 0x006bad10: size GOG 101 vs Steam 75, SIG 0.00 (intersection 0)
- GOG 0x006f5a80 CSWSCreatureStats::GetCreatureTalentRandomPower -> Steam 0x006bae00: retn GOG 8 vs Steam 28
- GOG 0x006f5630 CSWSCreatureStats::GetCreatureTalentRandomSpecialAbility -> Steam 0x006baec0: retn GOG 4 vs Steam 32
- GOG 0x006f8bb0 CSWSCreatureStats::CheckInclusionMatch -> Steam 0x006bdd60: size GOG 447 vs Steam 205, SIG 0.05 (intersection 3)
- GOG 0x007e4850 CSWSFaction::AddMember -> Steam 0x006d3330: name is not exact (suffix on the trusted row)
- GOG 0x00642c30 CVirtualMachineInternal::ExecuteCode -> Steam 0x006ff9f0: size GOG 1214 vs Steam 9336, SIG 0.46 (intersection 35)
- GOG 0x00645270 CVirtualMachineInternal::ReadScriptFile -> Steam 0x00701fd0: size GOG 64 vs Steam 716, SIG 0.11 (intersection 4)
- GOG 0x00645540 CVirtualMachineInternal::RunScript -> Steam 0x007022a0: size GOG 9 vs Steam 740, SIG 0.10 (intersection 1)
- GOG 0x00606e70 CExoString::CompareNoCase -> Steam 0x00734f60: name is not exact (suffix on the trusted row)
- GOG 0x00457310 CSWCArea::ClearAllMapPins -> Steam 0x007a96f0: size GOG 43 vs Steam 137, SIG 0.00 (intersection 0)
- GOG 0x00456350 CSWCArea::ClearTilePathNodes -> Steam 0x007aa910: size GOG 64 vs Steam 43, SIG 0.00 (intersection 0)
- GOG 0x004dbe50 CGuiInGame::IsFadingInDialog -> Steam 0x007cb720: size GOG 43 vs Steam 44, SIG 0.00 (intersection 0)
- GOG 0x004df1f0 CGuiInGame::GetCanOrientInDialog -> Steam 0x007cf710: size GOG 90 vs Steam 112, SIG 0.10 (intersection 1)
- GOG 0x004e0c60 CGuiInGame::GetMiniMapVisible -> Steam 0x007d05a0: size GOG 49 vs Steam 120, SIG 0.00 (intersection 0)
- GOG 0x00478360 CSWParty::AddCharacter -> Steam 0x007e5730: name is not exact (suffix on the trusted row)
- GOG 0x00478870 CSWParty::IsPartyMember -> Steam 0x007e5c40: name is not exact (suffix on the trusted row)
- GOG 0x00474890 CSWCObject::AnimationPause -> Steam 0x007ef350: size GOG 68 vs Steam 129, SIG 0.08 (intersection 1)
- GOG 0x004744b0 CSWCObject::AnimationPlayOutOfPlace -> Steam 0x007ef730: size GOG 129 vs Steam 68, SIG 0.08 (intersection 1)
- GOG 0x00597ff0 CSWGuiInGameCharacter::SetStats -> Steam 0x0084e6f0: name is not exact (suffix on the trusted row)
- GOG 0x005995d0 CSWGuiInGameCharacter::ShowLevelUpGUI -> Steam 0x0084fcd0: name is not exact (suffix on the trusted row)

## Ambiguous rows (second candidate also reaches C2)

(none)

## Lessons

<!-- one lesson per line: `- scope | topic | note`; persist inserts each once (dedupe on scope+topic) -->
- steam | 0x66C7A0 = ExecuteCommandAddMultiClass (slot 389), not LevelUpHenchman | Steam InitCommands store 006662e9 MOV [EDX+0x614],0x66c7a0 = slot 389 AddMultiClass (B15/cmdtable.tsv); 483 B, fp = GOG 0x781d80, Mac symbol same. Resolves doc 08 residual (08_level_up_override.md line 413: routine id of 0x66C7A0 = 389). LevelUpOverride.cpp:103 labels it 'LevelUpHenchman': address right, label wrong. Evidence steam_db_batches/B15.
- steam-db / method | Mac x86 NWScript command table uses 8-byte slots, Windows 4 | Mac x86 InitCommands 0x324A7C + 0x3579D4 store gcc pointer-to-member {ptr, adj} pairs: slot = disp/8. Windows MSVC (GOG 0x77B530 + 0x7F9140, Steam 0x665F50 + 0x6F5B80) stores 4-byte pointers: slot = disp/4. Read stores in instruction order; the later store to a slot wins (slot 156 is stored twice on all three). tools/cmdtable.py, DESIGN 4.8.
- steam-db / process | Decoy key error: a decoy taken next to a wrong gate pick can be the true counterpart | B15-4-38: GOG 0x784680 ChangeFaction, gate pick 0x66F330 (= ClearAllActions handler), decoy_prev 0x66F0A0 = the real slots 173/412 handler; the reviewer accepted it correctly. Fix F1: cards never use a proof-source (cmdtable) candidate as a decoy; finalize lists accepted decoys that an Opus override ships as key errors, separately, never escalating.
- steam-db / method | The NWScript command table is a label-independent proof of the handler counterpart; gate needs a TBL family | B15: 655/655 GOG handler slot sets have exactly one identical Steam set; 653/655 GOG names = Mac symbols (exceptions: slot-156 pair). The gate ranked near over cmdtable in 7 rows (6 picks G6-held by another name); Opus corrected all 7 to the table. A TBL anchor family (exact unique slot set toward C2) is deferred to an Opus calibration review before B10b.
- steam-db / method | NWScript source names are not nwscript names; the Mac symbol decides | About 152 B15 handler names have no nwscript slot match (GetEmotions = slots 235-237 GetIsEnemy/Friend/Neutral, IsAIState = slot 140 GetIsDead, GetPartyLeader = 238/845 with Mac GetPCSpeaker). They are Mac real symbols of the slot set: verified, not label_suspect. Reviewers flagged two as label_suspect; Opus overruled both.
- steam-db / method | Slot-156 pair ships at GOG parity although Mac names are swapped | Slot 156 (EffectTemporaryForcePoints) is stored twice: GOG 0x78D6B0/Steam 0x6780D0 dead, GOG 0x78C440/Steam 0x676E60 live. Mac live = ExecuteCommandEffectTemporaryFP, dead = ...ForcePoints (swapped vs GOG). User decision 2026-10-03: GOG label follows nwscript (name_slot yes) and B0 already ships it at 0x676E60, so both ship at GOG parity; not a GOG label error.
- steam | Steam 0x681880 is an ICF fold: CSWArea::GetIsIDInExcludeList + 3 command stubs | 15 B XOR EAX,EAX; RET 8 = command slots 321/327/403 (GetLastAssociateCommand, SetAssociateListenPatterns, ExploreAreaForPlayer; stores 006676b2/00668770/00666cea) and CSWArea::GetIsIDInExcludeList (direct call 0063eecd in TestPlayerUpdateDifferences). Mac has four separate 7 B return-0 stubs (0x341974, 0x34CDD6, 0x338B7C, 0x270766). GOG 0x44F600.
- steam-db / process | opus_gate 100% list counts decoy-card corrected_addr as reviewer corrections | B15: 18 of 25 rows listed as 'reviewer corrected_addr' were reviewers correctly rejecting a decoy card and naming the real address; only 7 were real corrections. Build the 100% list from real cards only (cards/key.tsv kind = real). Harmless when the batch escalates, inflates Opus load otherwise.
