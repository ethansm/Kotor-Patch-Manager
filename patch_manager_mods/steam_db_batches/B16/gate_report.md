# Gate report B16

Rows: 149. Source order for tie-breaks: server > trusted name_map > mapper:vote > branch > seeded > other.

## Counts per class and gate class

| class | AUTO | REVIEW | REJECT | NOCAND | total |
|---|---|---|---|---|---|
| CAurTextureBasic | 14 | 7 | 0 | 0 | 21 |
| CExoMoviePlayerInternal | 0 | 14 | 0 | 1 | 15 |
| CSWGlobalVariableTable | 12 | 15 | 1 | 0 | 28 |
| CSWGuiDialogLetterbox | 9 | 2 | 0 | 0 | 11 |
| CSWGuiInGameCharacter | 7 | 10 | 0 | 0 | 17 |
| CSWGuiInGameGameplay | 0 | 21 | 1 | 0 | 22 |
| CSWGuiTextParams | 8 | 6 | 0 | 0 | 14 |
| CServerAIMaster | 15 | 6 | 0 | 0 | 21 |
| **all** | 65 | 81 | 2 | 1 | 149 |

AUTO by route: c2=38 sig=27 (the sig route is non-propagating, see DESIGN 4.2)

## REJECT rows

- CSWGuiInGameGameplay::OnPanelAdded gog 0x005e56e0 -> 0x008e0cc0: G4 steam retn 4 vs GOG retn 0 (no thunk explanation)
- CSWGlobalVariableTable::ReadTableWithCatalogue gog 0x006529c0 -> 0x00655b00: G6 address held by trusted CSWGlobalVariableTable::ReadTableWithCat

## Duplicates and conflicts (G6)

- CSWGuiDialogLetterbox::SetExtentSpecial gog 0x005af270 -> 0x008bab20: review:held_by=CSWGuiDialogLetterbox::ResizeToText
- CSWGuiInGameGameplay::Constructor gog 0x005e43c0 -> 0x008df930: review:held_by=CSWGuiOptionsGameplay
- CSWGuiInGameGameplay::OnAutoLevelUp gog 0x005e5700 -> 0x008e0c50: review:held_by=CSWGuiOptionsGameplay
- CServerAIMaster::ClearEventQueue gog 0x0064da00 -> 0x0051dfe0: review:held_by=Events::ClearAll
- CServerAIMaster::AddEventAbsoluteTime gog 0x0064dc40 -> 0x0051e220: review:held_by=EventQueue::AddEvent
- CServerAIMaster::DeleteEventData gog 0x00650700 -> 0x00520ce0: review:held_by=Events::FreePayload
- CSWGlobalVariableTable::AddIdentifierBoolean gog 0x006512f0 -> 0x00654430: review:held_by=CSWGlobalVariableTable::AddBoolean
- CSWGlobalVariableTable::AddIdentifierNumber gog 0x00651350 -> 0x00654490: review:held_by=CSWGlobalVariableTable::AddNumber
- CSWGlobalVariableTable::AddIdentifierString gog 0x00651410 -> 0x00654550: review:held_by=CSWGlobalVariableTable::AddString
- CSWGlobalVariableTable::SetValueNumber gog 0x00651b80 -> 0x00654cc0: review:held_by=CSWGlobalVariableTable::SetNumber
- CSWGlobalVariableTable::SetValueString gog 0x00651dc0 -> 0x00654f00: review:held_by=CSWGlobalVariableTable::SetString
- CSWGlobalVariableTable::ReadTableWithCatalogue gog 0x006529c0 -> 0x00655b00: fail:trusted_holder=CSWGlobalVariableTable::ReadTableWithCat
- CAurTextureBasic::ParseField gog 0x008161b0 -> 0x00423ab0: review:held_by=CAurTextureBasic::ParseTXILine
- CAurTextureBasic::Init gog 0x00817210 -> 0x00424b10: review:held_by=CAurTextureBasic::ParseTXI
- CAurTextureBasic::CanDownSampleFurther gog 0x00818ff0 -> 0x004268f0: review:held_by=CAurTextureBasic::IsDownSampleCandidate
- CExoMoviePlayerInternal::RequestRestart gog 0x0091fdf0 -> 0x0040cc00: review:held_by=Movie::NoteDeactivated
- CExoMoviePlayerInternal::CancelMovie gog 0x0091fe20 -> 0x0040cc30: review:held_by=Movie::Stop
- CExoMoviePlayerInternal::StartMovie gog 0x0091fe60 -> 0x0040cc70: review:held_by=CExoMoviePlayerInternal::OpenClip
- CExoMoviePlayerInternal::RunMovieFrameLoop gog 0x009208e0 -> 0x0040d6f0: review:held_by=Movie::PlayFrames
- CExoMoviePlayerInternal::UpdateMovieWindow gog 0x00920f00 -> 0x0040dd10: review:held_by=movie

## G4 metadata flags

- CSWGuiInGameCharacter::DoAutoLevelup gog 0x00599910: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWGuiInGameCharacter::RotateCharacterLeft gog 0x00599c30: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWGuiInGameCharacter::RotateCharacterRight gog 0x00599e50: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWGuiInGameGameplay::OnEnterDiff gog 0x005e6020: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWGuiInGameGameplay::OnExitDiff gog 0x005e6080: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CServerAIMaster::AdjustTargetAndWitnessReputations gog 0x0064eac0: ['gog_db_param_bad'] (gog_param=0 gog_retn=12 steam_retn=12)
- CExoMoviePlayerInternal::RunMovieFrameLoop gog 0x009208e0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CExoMoviePlayerInternal::UpdateMovieWindow gog 0x00920f00: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)

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
- GOG 0x00733ef0 CSWSModule::non_virtual_thunk_to_dtor_CSWSModule -> Steam 0x00561c80: size GOG 8 vs Steam 44, SIG 0.00 (intersection 0)
- GOG 0x00667710 CSWSItem::RestoreUsedActiveProperties -> Steam 0x00603dd0: size GOG 63 vs Steam 245, SIG 0.17 (intersection 3)
- GOG 0x00667600 CSWSItem::GetPropertyByTypeExists -> Steam 0x00603f60: retn GOG 4 vs Steam 8
- GOG 0x006fcf40 CSWSItem::SetNumActiveProperties -> Steam 0x00604140: size GOG 85 vs Steam 133, SIG 0.33 (intersection 2)
- GOG 0x0067a210 CSWSItem::non_virtual_thunk_to_dtor_CSWSItem -> Steam 0x00616a60: size GOG 8 vs Steam 44, SIG 0.00 (intersection 0)
- GOG 0x007ef060 CFactionManager::SetNPCFactionReputation -> Steam 0x006644d0: name is not exact (suffix on the trusted row)
- GOG 0x007ef110 CFactionManager::GetNPCFactionReputation -> Steam 0x00664580: name is not exact (suffix on the trusted row)
- GOG 0x007ef1e0 CFactionManager::GetIsNPCFaction -> Steam 0x00664650: name is not exact (suffix on the trusted row)
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
