# Gate report B11b

Rows: 238. Source order for tie-breaks: server > trusted name_map > mapper:vote > branch > seeded > other.

## Counts per class and gate class

| class | AUTO | REVIEW | REJECT | NOCAND | total |
|---|---|---|---|---|---|
| CGuiInGame | 111 | 59 | 2 | 0 | 172 |
| CSWGuiMainInterface | 0 | 65 | 0 | 1 | 66 |
| **all** | 111 | 124 | 2 | 1 | 238 |

AUTO by route: c2=66 sig=45 (the sig route is non-propagating, see DESIGN 4.2)

## REJECT rows

- CGuiInGame::ShutDownGlobal gog 0x004cdda0 -> 0x007bdb10: G6 name_map marks this name at this address as wrong
- CGuiInGame::Destructor gog 0x004cde50 -> 0x00740070: G6 address held by trusted CClientExoApp::GetMGOArray

## Duplicates and conflicts (G6)

- CGuiInGame::ShutDownGlobal gog 0x004cdda0 -> 0x007bdb10: fail:known_wrong
- CGuiInGame::Destructor gog 0x004cde50 -> 0x00740070: fail:trusted_holder=CClientExoApp::GetMGOArray
- CGuiInGame::AppendToCombatBuffer gog 0x004ce840 -> 0x007be1b0: review:held_by=FeedbackList::Add
- CGuiInGame::SetConversationType gog 0x004d00e0 -> 0x007bfa50: review:held_by=CGuiInGame::ShowDialogEntry
- CGuiInGame::ShowDialogEntry gog 0x004d01a0 -> 0x007bfb10: review:held_by=CGuiInGame::ShowDialogPanels
- CGuiInGame::CleanUpEntryNode gog 0x004d6920 -> 0x007c6200: review:held_by=CGuiInGame::SetComputerCameraShot
- CGuiInGame::ShowDebugMenu gog 0x004db100 -> 0x007cac10: review:held_by=CGuiInGame::HideExamineBox
- CGuiInGame::DestroyDeathMenuGUI gog 0x004db4b0 -> 0x007cadb0: review:held_by=CGuiInGame::IsAnimationPlayingInDialog
- CGuiInGame::IsLoopingDialogStuntAnimation_2 gog 0x004dbe20 -> 0x007cb5e0: review:held_by=CGuiInGame::UnlockDialogParticipantOrientations
- CGuiInGame::GetCanOrientInDialog gog 0x004df1f0 -> 0x007ceaf0: review:name_held_elsewhere=0x007cf710
- CGuiInGame::GetMiniMapVisible gog 0x004e0c60 -> 0x007d0560: review:held_by=CGuiInGame::RemoveGUIPopUps+name_held_elsewhere=0x007d05a0
- CGuiInGame::ResetInterfaceFont gog 0x004e0d20 -> 0x007d0620: review:held_by=CGuiInGame::RemoveGUIPopUps
- CSWGuiMainInterface::PopulateMenus gog 0x0051e760 -> 0x0074cfe0: review:held_by=CSWGuiMainInterface::UpdateTargetMenu
- CSWGuiMainInterface::RevertActionBarStates gog 0x0051f1c0 -> 0x0074da40: review:held_by=CSWGuiMainInterface::UpdateStanceAction
- CSWGuiMainInterface::UpdatePortraits gog 0x00520610 -> 0x0074ee90: review:held_by=CSWGuiInGameGameplay::UpdatePartyPortraits

## G4 metadata flags

- CGuiInGame::ShowDialogEntry gog 0x004d01a0: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CGuiInGame::CleanUpEntryNode gog 0x004d6920: ['gog_db_param_bad'] (gog_param=0 gog_retn=12 steam_retn=12)
- CGuiInGame::HandleDialogEntry gog 0x004dc660: ['gog_db_param_bad'] (gog_param=0 gog_retn=92 steam_retn=92)
- CGuiInGame::HandleDialogModels gog 0x004dcc30: ['gog_db_param_bad'] (gog_param=0 gog_retn=24 steam_retn=24)
- CGuiInGame::HandleDialogReplies gog 0x004dccc0: ['gog_db_param_bad'] (gog_param=0 gog_retn=100 steam_retn=100)
- CGuiInGame::HandleDialogReplyChosen gog 0x004dd0a0: ['gog_db_param_bad'] (gog_param=0 gog_retn=28 steam_retn=28)
- CGuiInGame::ShowUpgradeScreen gog 0x004ddee0: ['gog_db_param_bad'] (gog_param=20 gog_retn=24 steam_retn=24)
- CGuiInGame::ShowDemoScreen gog 0x004e2410: ['gog_db_param_bad'] (gog_param=1 gog_retn=24 steam_retn=24)
- CSWGuiMainInterface::HideHelp gog 0x00517df0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWGuiMainInterface::HandleInputEvent gog 0x00518020: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWGuiMainInterface::UpdateActionQueue gog 0x0051e730: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWGuiMainInterface::PopulateMenus gog 0x0051e760: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWGuiMainInterface::HandleButtonActionX gog 0x0051f070: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWGuiMainInterface::GetActionIcon gog 0x0051f830: ['gog_db_param_bad'] (gog_param=0 gog_retn=24 steam_retn=24)
- CSWGuiMainInterface::DoPersonalAction gog 0x00522ed0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWGuiMainInterface::OnCharacterExited gog 0x00523a80: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWGuiMainInterface::OnSwapWeaponsPressed gog 0x00526530: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)

## Trusted pair body conflicts

Trusted rows that are NOT seeds (exact name + equal retn + equal size or A-SIG >= 0.5 with intersection >= 3 required, Opus gate R1). They stay trusted as G6 holders until Opus rules on them.

- GOG 0x00815520 CAurTexture::SetIsEnvironmentMapped -> Steam 0x00422e20: name is not exact (suffix on the trusted row)
- GOG 0x0084b0b0 Gob::SetScene -> Steam 0x00461aa0: retn GOG 0 vs Steam 4
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
- GOG 0x00692f50 CSWSCreature::BroadcastPoisoned -> Steam 0x0057a420: size GOG 3 vs Steam 312, SIG 0.00 (intersection 0)
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
- GOG 0x0043b710 CClientExoAppInternal::AddFeedbackMessage -> Steam 0x0078f160: size GOG 30173 vs Steam 496, SIG 0.05 (intersection 7)
- GOG 0x00457310 CSWCArea::ClearAllMapPins -> Steam 0x007a96f0: size GOG 43 vs Steam 137, SIG 0.00 (intersection 0)
- GOG 0x00456350 CSWCArea::ClearTilePathNodes -> Steam 0x007aa910: size GOG 64 vs Steam 43, SIG 0.00 (intersection 0)
- GOG 0x004dbe50 CGuiInGame::IsFadingInDialog -> Steam 0x007cb720: size GOG 43 vs Steam 44, SIG 0.00 (intersection 0)
- GOG 0x004df1f0 CGuiInGame::GetCanOrientInDialog -> Steam 0x007cf710: size GOG 90 vs Steam 112, SIG 0.10 (intersection 1)
- GOG 0x004e0c60 CGuiInGame::GetMiniMapVisible -> Steam 0x007d05a0: size GOG 49 vs Steam 120, SIG 0.00 (intersection 0)
- GOG 0x00478360 CSWParty::AddCharacter -> Steam 0x007e5730: name is not exact (suffix on the trusted row)
- GOG 0x00478870 CSWParty::IsPartyMember -> Steam 0x007e5c40: name is not exact (suffix on the trusted row)
- GOG 0x00474890 CSWCObject::AnimationPause -> Steam 0x007ef350: size GOG 68 vs Steam 129, SIG 0.08 (intersection 1)
- GOG 0x004744b0 CSWCObject::AnimationPlayOutOfPlace -> Steam 0x007ef730: size GOG 129 vs Steam 68, SIG 0.08 (intersection 1)

## Ambiguous rows (second candidate also reaches C2)

(none)

## Lessons

<!-- one lesson per line: `- scope | topic | note`; persist inserts each once (dedupe on scope+topic) -->
