# Gate report B9

Rows: 270. Source order for tie-breaks: server > trusted name_map > mapper:vote > branch > seeded > other.

## Counts per class and gate class

| class | AUTO | REVIEW | REJECT | NOCAND | total |
|---|---|---|---|---|---|
| CSWSModule | 38 | 16 | 0 | 0 | 54 |
| CServerExoApp | 10 | 108 | 0 | 0 | 118 |
| CServerExoAppInternal | 54 | 42 | 1 | 1 | 98 |
| **all** | 102 | 166 | 1 | 1 | 270 |

AUTO by route: c2=62 sig=40 (the sig route is non-propagating, see DESIGN 4.2)

## REJECT rows

- CServerExoAppInternal::SetMoveToWorldTime gog 0x007bfa20 -> 0x0053c5c0: G4 steam retn 0 vs GOG retn 28 (no thunk explanation)

## Duplicates and conflicts (G6)

- CServerExoApp::GetServerInfo gog 0x0064b760 -> 0x0051bd40: review:held_by=GetServerInfo/JoiningRestrictions
- CServerExoApp::GetModule gog 0x0064b7b0 -> 0x0051bd90: review:held_by=GetModule
- CServerExoApp::GetPlaceableByGameObjectID gog 0x0064bb80 -> 0x0051c160: review:held_by=ServerApp::GetPlaceableById
- CServerExoApp::GetClientObjectByObjectId gog 0x0064c020 -> 0x0051c600: review:held_by=CServerExoApp::GetPlayerByControlledObjectId
- CSWSModule::PlotInterAreaPath gog 0x0072d0b0 -> 0x0055a900: review:held_by=pathfind
- CSWSModule::PlotPathInArea gog 0x0072d430 -> 0x0055ac80: review:held_by=CSWSArea::PlotPathInArea
- CSWSModule::SaveModuleStart gog 0x0072dfe0 -> 0x0055b830: review:held_by=CSWSModule::BeginModuleSave
- CSWSModule::SaveModuleFAC gog 0x0072f420 -> 0x0055cc70: review:held_by=SaveRepute
- CSWSModule::SavePrimaryPlayerInfo gog 0x0072ffa0 -> 0x0055d7f0: review:held_by=SaveMultiplayerPlayer
- CServerExoAppInternal::StoreCurrentModule gog 0x007b39e0 -> 0x00530660: review:held_by=SaveModuleForGameInProgress
- CServerExoAppInternal::MainLoop gog 0x007b6bb0 -> 0x00533830: review:held_by=Game
- CServerExoAppInternal::StallEventSaveGame gog 0x007b7980 -> 0x00534600: review:held_by=SaveGameInProgress

## G4 metadata flags

- CServerExoApp::TestDirectLine gog 0x0064c330: ['gog_db_param_bad'] (gog_param=4 gog_retn=32 steam_retn=32)
- CServerExoApp::ComputeHeight gog 0x0064c460: ['gog_db_param_bad'] (gog_param=0 gog_retn=12 steam_retn=12)
- CServerExoApp::GetRoomIndex gog 0x0064c620: ['gog_db_param_bad'] (gog_param=0 gog_retn=12 steam_retn=12)
- CSWSModule::Constructor gog 0x00728e20: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CSWSModule::LoadModuleInProgress gog 0x0072cc10: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSModule::PackModuleIntoMessage gog 0x0072cf90: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)
- CSWSModule::SaveModuleStart gog 0x0072dfe0: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CSWSModule::SaveModuleFinish gog 0x0072e330: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=8)
- CSWSModule::SavePlayers gog 0x0072f830: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=16)
- CSWSModule::PackPlayerCharacterListIntoMessage gog 0x00730b00: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CServerExoAppInternal::SaveGame gog 0x007b2990: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=16)
- CServerExoAppInternal::CopyModuleToCurrentGame gog 0x007b4f60: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CServerExoAppInternal::SetEstimatedSaveSize gog 0x007b6860: ['gog_db_param_bad'] (gog_param=0 gog_retn=8 steam_retn=8)
- CServerExoAppInternal::PlayerListChange gog 0x007b8f70: ['gog_db_param_bad'] (gog_param=8 gog_retn=12 steam_retn=12)
- CServerExoAppInternal::StartServices gog 0x007b96e0: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=4)

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
- steam-db / method | CServerExoApp is one contiguous forwarder block at Steam = GOG - 0x12FA20 | B9: all 118 CServerExoApp rows (GOG 0x64B640-0x64CAD0) sit at Steam = GOG - 0x12FA20 with fingerprints identical to GOG (size, retn, n_insn, disp, imm, callee count). Use the delta as a cross-check only; the anchor stays the forwarded Internal callee or the field reader/writer. Internal deltas are piecewise (0x283380..0x282DA0), CSWSModule 0x1D27B0/0x1D2690.
- steam-db / method | Server fp drops displacements above 0x10000, so large-field Get/Set twins share one fingerprint | CServerExoApp/Internal fields sit at +0x10004..+0x1F348 and are missing from fp disp on both sides, so Get/Set twins and adjacent-field accessors look identical (SetActionConversation +0x1F31C vs SetIgnoreConversationStartRange +0x1F320; World/Timestop/Pause timers +0x10048/4C/50). Split them by a caller that calls only one, or by the field's reader/writer in asm, never by position.
- steam-db / process | The gate's typical B9 miss is the same-shape or adjacent neighbour | B9: 10 Opus address corrections. Every wrong pick was a neighbour: a Get/Set or Add/Remove twin (Internal Set/GetMoveToWorldTime, ExoApp+Internal RemoveFromExclusionList) or the adjacent forwarder/field (GetPauseState x2, UpdateSubareasOnMoveTo, ShrinkLineByCreatures, Constructor -> GetFPS, CSWSModule dtor thunk). In 7 of 10 the right address was already a candidate but ranked below a near pick.
- steam-db / method | Mapper drift candidates can land on the twin; add an fp-exact candidate source | B9: in 3 of 10 corrections the right address was no candidate: drift+0x440 gave GetMoveToWorldTime its Set twin 0x53CC80; SetMoveToWorldTime got only near candidates without 0x53CC80, so a G4 REJECT (0x53C5C0, RET 0); drift-0x2A0 missed the dtor thunk. A candidate source of unique GOG-identical fps inside the class run window (Set 103B vs Get 117B) would have found all three.
- steam-db / method | A NOCAND row whose GOG address lies outside its class block is usually a GOG mislabel | B9: CServerExoAppInternal::DebugValidatePath GOG 0x662B80 sits in creature code. A whole-binary fp-exact search found one Steam body, 0x6E6880 (446B/127 insn), = Mac CSWSCreature::GetFirstAttacker (disp 0x194/0x198/0x548/0x11A4/0x11A8), sole caller ExecuteCommandGetAttacker: label_suspect. Run that search and a Mac shape check before leaving a NOCAND row unresolved.
- steam | 0x6E6880 = CSWSCreature::GetFirstAttacker, not DebugValidatePath or ApplyOnHitInstantDeath | Steam 0x6E6880 (446 B, sole caller ExecuteCommandGetAttacker 0x696BF0) rebuilds the attacker id list at [+0x11A4] from the area creature array and returns the first (Mac 0x378234). GOG DB name of GOG 0x662B80 (Internal::DebugValidatePath) and the KDB row (gog_cluster_delta_map/low, ApplyOnHitInstantDeath) are both wrong. GetNextAttacker probably 0x6E6A40. Evidence B9.
- steam-db / process | Shared callers do not separate twins: the decoy B9-3-29 accept | Sonnet accepted GOG 0x64C780 SetIgnoreConversationStartRange -> 0x51CD10 (the SetActionConversation twin) because Door/Placeable EventHandler call both setters. The deciding callers call only one: SetGlobalDialogState 0x7CEB80 (fp = GOG 0x4DF280) -> 0x51CD60, ExecuteCommandBeginConversation -> 0x51CD10. A genuine reviewer error, not a key error: the gate pick 0x51CD60 ships.
- steam-db / method | GOG DB param_size_bytes is wrong in 15 of 270 B9 rows | In 15 B9 rows the GOG DB param size disagrees with the RET n of the GOG function itself, and Steam RET n equals GOG RET n every time (e.g. TestDirectLine 4 vs RET 0x20, SavePlayers 8 vs 0x10, Internal::SaveGame 12 vs 0x10). Ship the Steam RET n (finalize does) and keep the GOG value in the note. List: GOG_LABEL_SUSPECTS.md B9 'param sizes'.
