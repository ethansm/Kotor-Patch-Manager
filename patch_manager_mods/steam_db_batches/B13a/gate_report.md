# Gate report B13a

Rows: 199. Source order for tie-breaks: server > trusted name_map > mapper:vote > branch > seeded > other.

## Counts per class and gate class

| class | AUTO | REVIEW | REJECT | NOCAND | total |
|---|---|---|---|---|---|
| CSWCCreature | 119 | 79 | 0 | 1 | 199 |
| **all** | 119 | 79 | 0 | 1 | 199 |

AUTO by route: c2=65 sig=54 (the sig route is non-propagating, see DESIGN 4.2)

## REJECT rows

(none)

## Duplicates and conflicts (G6)

- CSWCCreature::PlaySoundSetSound gog 0x00411770 -> 0x00765aa0: review:held_by=CSWCCreature::GetSoundSetStrres
- CSWCCreature::StripEquippedItems gog 0x00413620 -> 0x007678f0: review:held_by=CSWCCreature::ClearVisibilityList
- CSWCCreature::SetCombatState gog 0x00414080 -> 0x007682b0: review:held_by=CSWCCreature::UpdateRollingFootstepSound
- CSWCCreature::MoveCreatureToPosition gog 0x00414210 -> 0x00768440: review:held_by=CSWCCreature::SetCombatState
- CSWCCreature::SetSpecialWalkAnimation gog 0x0041f7e0 -> 0x00773910: review:held_by=CSWCCreature::ResetAnimationState
- CSWCCreature::ApplyShadowBlob gog 0x00420130 -> 0x00774260: review:held_by=CSWCCreature::GetHeadTurnVertical
- CSWCCreature::DeleteSpellLikeAbilityList gog 0x004216d0 -> 0x007757e0: review:held_by=CSWCCreature::CreateNewSpellLikeAbilityList
- CSWCCreature::GetPauseReadyAnimation gog 0x004218a0 -> 0x007759b0: review:held_by=CSWCCreature::GetCameraFollowPart
- CSWCCreature::EnableFeatForMenu gog 0x00424900 -> 0x00778940: review:held_by=Global::SceneHitCheck
- CSWCCreature::GetBehaviors gog 0x004269a0 -> 0x0077a9e0: review:held_by=CSWGuiTargetActionMenu::AddStanceActions
- CSWCCreature::IsLeaderUsableItem gog 0x004275f0 -> 0x0077b670: review:held_by=GuiInGame::IsItemUsableByCurrent
- CSWCCreature::SetAlignmentGoodEvil gog 0x004291f0 -> 0x0077d270: review:held_by=CSWCCreature::ActionMenuForm
- CSWCCreature::GetServerCreature gog 0x00429780 -> 0x0077d800: review:held_by=Party
- CSWCCreature::ActionMenuSelectBehavior gog 0x00429810 -> 0x0077d890: review:held_by=StanceActionClicked

## G4 metadata flags

- CSWCCreature::UpdateRollingFootstepSound gog 0x00413e70: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWCCreature::ComputeZValue gog 0x004151f0: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=16)
- CSWCCreature::GetSaveCreatureFilename gog 0x00418b00: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWCCreature::SetStealthState gog 0x00419850: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWCCreature::PlaySwingSound gog 0x0041dbb0: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWCCreature::UnloadMonkVisuals gog 0x0041f6b0: ['gog_db_param_bad'] (gog_param=4 gog_retn=0 steam_retn=0)
- CSWCCreature::ResetAnimationState gog 0x0041f730: ['gog_db_param_bad'] (gog_param=4 gog_retn=0 steam_retn=0)
- CSWCCreature::Hilite gog 0x004208e0: ['gog_db_param_bad'] (gog_param=5 gog_retn=16 steam_retn=16)
- CSWCCreature::ActionMenuSwapImplantRegen gog 0x00426430: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCCreature::ActionMenuSwapImplantStr gog 0x00426450: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCCreature::ActionMenuSwapImplantAgi gog 0x00426470: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCCreature::ActionMenuSwapImplantEnd gog 0x00426490: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCCreature::GetStances gog 0x00426e70: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWCCreature::PartyMenuStealth gog 0x00426fe0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCCreature::IsLeaderUsableItem gog 0x004275f0: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=12)
- CSWCCreature::ActionMenuAttack gog 0x00428d80: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCCreature::ActionInitiateDialog gog 0x00428ef0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWCCreature::ActionMenuForm gog 0x004290a0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWCCreature::ActionMenuSwitchWeapons gog 0x004299d0: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWCCreature::GetHostileInterfaceActions gog 0x0042a170: ['gog_db_param_bad'] (gog_param=0 gog_retn=12 steam_retn=12)

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
