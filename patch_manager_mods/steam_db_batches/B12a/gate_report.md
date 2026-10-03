# Gate report B12a

Rows: 201. Source order for tie-breaks: server > trusted name_map > mapper:vote > branch > seeded > other.

## Counts per class and gate class

| class | AUTO | REVIEW | REJECT | NOCAND | total |
|---|---|---|---|---|---|
| CSWSCreature | 150 | 35 | 14 | 2 | 201 |
| **all** | 150 | 35 | 14 | 2 | 201 |

AUTO by route: c2=110 sig=40 (the sig route is non-propagating, see DESIGN 4.2)

## REJECT rows

- CSWSCreature::ResolveRangedAttack gog 0x006571b0 -> 0x0053e450: G4 steam retn 4 vs GOG retn 28 (no thunk explanation)
- CSWSCreature::ResolveMeleeAnimations gog 0x0065c710 -> 0x00543ae0: G4 steam retn 4 vs GOG retn 20 (no thunk explanation)
- CSWSCreature::CalculateMaxElementalDamage gog 0x0065da20 -> 0x005452c0: G4 steam retn 4 vs GOG retn 8 (no thunk explanation)
- CSWSCreature::ResolveOnHitEffect gog 0x0065f2a0 -> 0x00546540: G4 steam retn 4 vs GOG retn 12 (no thunk explanation)
- CSWSCreature::SavingThrowRoll gog 0x00660930 -> 0x00547d60: G4 steam retn 8 vs GOG retn 28 (no thunk explanation)
- CSWSCreature::ApplyOnHitConfusion gog 0x00661610 -> 0x00548b10: G4 steam retn 4 vs GOG retn 8 (no thunk explanation)
- CSWSCreature::ApplyOnHitFear gog 0x006617b0 -> 0x00548b10: G4 steam retn 4 vs GOG retn 8 (no thunk explanation)
- CSWSCreature::ApplyOnHitSlow gog 0x00661950 -> 0x00548b10: G4 steam retn 4 vs GOG retn 8 (no thunk explanation)
- CSWSCreature::ApplyOnHitPoison gog 0x00661b60 -> 0x00548db0: G4 steam retn 12 vs GOG retn 8 (no thunk explanation)
- CSWSCreature::ApplyOnHitInstantDeath gog 0x006625d0 -> 0x006e6880: G4 steam retn 0 vs GOG retn 8 (no thunk explanation)
- CSWSCreature::ApplyOnHitKnockdown gog 0x00662900 -> 0x006e6ac0: G4 steam retn 0 vs GOG retn 8 (no thunk explanation)
- CSWSCreature::SetAttackSummaryFeedbackEffectData gog 0x00663170 -> 0x006e71d0: G4 steam retn 0 vs GOG retn 8 (no thunk explanation)
- CSWSCreature::GetNearestEnemy gog 0x006954c0 -> 0x0057c990: G6 address held by trusted CSWSCreature::GetNearestVisibleEnemyInRange
- CSWSCreature::SetLastInstantEquip gog 0x00696db0 -> 0x0057e280: G6 address held by trusted CSWSCreature::RemoveMindTrickAndScrambleEffects

## Duplicates and conflicts (G6)

- CSWSCreature::CancelAllCombatModes gog 0x00656dd0 -> 0x006dab20: review:held_by=CSWSCreature::EndAttackAction
- CSWSCreature::ResolveRangedMiss gog 0x0065c680 -> 0x00543ae0: review:held_by=CSWSObject::DoSpellImmunity
- CSWSCreature::ResolveMeleeAnimations gog 0x0065c710 -> 0x00543ae0: review:held_by=CSWSObject::DoSpellImmunity
- CSWSCreature::CalculateMaxElementalDamage gog 0x0065da20 -> 0x005452c0: review:held_by=CSWSObject::RemoveEffectTarget
- CSWSCreature::ResolveOnHitEffect gog 0x0065f2a0 -> 0x00546540: review:held_by=CSWSObject::SendDialogEntry
- CSWSCreature::ApplyOnHitPoison gog 0x00661b60 -> 0x00548db0: review:held_by=CSWSObject::CalculateSpellRangedMissTarget
- CSWSCreature::ApplyOnHitInstantDeath gog 0x006625d0 -> 0x006e6880: review:held_by=CSWSCreature::GetFirstAttacker
- CSWSCreature::SetAttackSummaryFeedbackEffectData gog 0x00663170 -> 0x006e71d0: fail:trusted_holder=CTwoDimArrays::Constructor
- CSWSCreature::BroadcastStealthDetectionData gog 0x00692f53 -> 0x0057a420: review:held_by=CSWSCreature::BroadcastPoisoned
- CSWSCreature::GetNearestEnemy gog 0x006954c0 -> 0x0057c990: fail:trusted_holder=CSWSCreature::GetNearestVisibleEnemyInRange
- CSWSCreature::AddUseTalentOnObjectActions gog 0x006963e0 -> 0x0057d8b0: review:held_by=CSWSCreature::UseTalentOnObject
- CSWSCreature::SetLastInstantEquip gog 0x00696db0 -> 0x0057e280: fail:trusted_holder=CSWSCreature::RemoveMindTrickAndScrambleEffects

## G4 metadata flags

- CSWSCreature::ResolveRangedAttack gog 0x006571b0: ['gog_db_param_bad'] (gog_param=4 gog_retn=28 steam_retn=4)
- CSWSCreature::ResolveMeleeSpecialAttack gog 0x0065b5a0: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSCreature::ResolveMeleeAnimations gog 0x0065c710: ['gog_db_param_bad'] (gog_param=0 gog_retn=20 steam_retn=4)
- CSWSCreature::CalculateMaxElementalDamage gog 0x0065da20: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=4)
- CSWSCreature::ResolveOnHitEffect gog 0x0065f2a0: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=4)
- CSWSCreature::DoCombatStep gog 0x0065fd40: ['gog_db_param_bad'] (gog_param=1 gog_retn=12 steam_retn=12)
- CSWSCreature::SavingThrowRoll gog 0x00660930: ['gog_db_param_bad'] (gog_param=16 gog_retn=28 steam_retn=8)
- CSWSCreature::ApplyOnHitConfusion gog 0x00661610: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=4)
- CSWSCreature::ApplyOnHitFear gog 0x006617b0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=4)
- CSWSCreature::ApplyOnHitSlow gog 0x00661950: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=4)
- CSWSCreature::ApplyOnHitPoison gog 0x00661b60: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=12)
- CSWSCreature::ApplyOnHitInstantDeath gog 0x006625d0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=0)
- CSWSCreature::ApplyOnHitKnockdown gog 0x00662900: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=0)
- CSWSCreature::SetAttackSummaryFeedbackEffectData gog 0x00663170: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=0)
- CSWSCreature::ComputeSpellRange gog 0x0067cf00: ['gog_db_param_bad'] (gog_param=1 gog_retn=12 steam_retn=12)
- CSWSCreature::ReplaceUnequipItemActions gog 0x0067fef0: ['gog_db_param_bad'] (gog_param=4 gog_retn=16 steam_retn=16)
- CSWSCreature::AddMineActions gog 0x00680d00: ['gog_db_param_bad'] (gog_param=16 gog_retn=20 steam_retn=20)
- CSWSCreature::EventHandler gog 0x00687b00: ['gog_db_param_bad'] (gog_param=12 gog_retn=20 steam_retn=20)
- CSWSCreature::GetBlind gog 0x0068f7d0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWSCreature::BroadcastSpellCast gog 0x00692540: ['gog_db_param_bad'] (gog_param=4 gog_retn=20 steam_retn=20)
- CSWSCreature::BroadcastStealthDetectionData gog 0x00692f53: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSCreature::AddPickUpItemActions gog 0x00693ac0: ['gog_db_param_bad'] (gog_param=0 gog_retn=12 steam_retn=12)
- CSWSCreature::UseFeat gog 0x00693b40: ['gog_db_param_bad'] (gog_param=2 gog_retn=16 steam_retn=16)

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
