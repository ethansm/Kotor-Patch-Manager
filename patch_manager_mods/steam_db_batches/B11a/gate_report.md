# Gate report B11a

Rows: 429. Source order for tie-breaks: server > trusted name_map > mapper:vote > branch > seeded > other.

## Counts per class and gate class

| class | AUTO | REVIEW | REJECT | NOCAND | total |
|---|---|---|---|---|---|
| CClientExoApp | 72 | 152 | 1 | 0 | 225 |
| CClientExoAppInternal | 95 | 93 | 13 | 3 | 204 |
| **all** | 167 | 245 | 14 | 3 | 429 |

AUTO by route: c2=66 sig=101 (the sig route is non-propagating, see DESIGN 4.2)

## REJECT rows

- CClientExoApp::GetClientRangedAnimation gog 0x0040d990 -> 0x00740930: G4 steam retn 8 vs GOG retn 16 (no thunk explanation)
- CClientExoAppInternal::DisplayMainMenu gog 0x0042db10 -> 0x00781810: G4 steam retn 0 vs GOG retn 4 (no thunk explanation)
- CClientExoAppInternal::HandleMessage gog 0x0042ddc0 -> 0x00781590: G4 steam retn 4 vs GOG retn 16 (no thunk explanation)
- CClientExoAppInternal::ServerModeChange gog 0x00431cc0 -> 0x00785030: G4 steam retn 0 vs GOG retn 4 (no thunk explanation)
- CClientExoAppInternal::SetTexturePack gog 0x00439c70 -> 0x0078cf50: G4 steam retn 4 vs GOG retn 8 (no thunk explanation)
- CClientExoAppInternal::StartMenuMusic gog 0x0043abf0 -> 0x0078e7e0: G4 steam retn 0 vs GOG retn 12 (no thunk explanation)
- CClientExoAppInternal::AquireLoadMusic gog 0x0043b0a0 -> 0x0078e350: G4 steam retn 8 vs GOG retn 4 (no thunk explanation)
- CClientExoAppInternal::HideLoadScreen gog 0x00444a00 -> 0x00797d90: G4 steam retn 8 vs GOG retn 12 (no thunk explanation)
- CClientExoAppInternal::PlayMovie gog 0x004451c0 -> 0x00798580: G4 steam retn 4 vs GOG retn 20 (no thunk explanation)
- CClientExoAppInternal::IsMoviePlaying gog 0x00445220 -> 0x00798c00: G6 address held by trusted IsMoviePlaying
- CClientExoAppInternal::ShowObject gog 0x00445390 -> 0x007983e0: G4 steam retn 12 vs GOG retn 16 (no thunk explanation)
- CClientExoAppInternal::GetNextLoadScreenHintSTRREF gog 0x0044c240 -> 0x0079f770: G4 steam retn 12 vs GOG retn 8 (no thunk explanation)
- CClientExoAppInternal::SetMouseMode gog 0x0044ca90 -> 0x0079fc40: G4 steam retn 8 vs GOG retn 4 (no thunk explanation)
- CClientExoAppInternal::ActionIconToMouseCursor gog 0x00460b10 -> 0x007b3ca0: G4 steam retn 0 vs GOG retn 8 (no thunk explanation)

## Duplicates and conflicts (G6)

- CClientExoApp::EndLoadBar gog 0x0040c510 -> 0x0073f430: review:held_by=CClientExoApp::GetModule
- CClientExoApp::IsClientImeUI gog 0x0040c830 -> 0x0073f370: review:held_by=CClientExoApp::Constructor
- CClientExoApp::GetConnectionLib gog 0x0040c8d0 -> 0x0073f410: review:held_by=CClientExoApp::GetFPS
- CClientExoApp::GetModuleCamera gog 0x0040c950 -> 0x0073f490: review:held_by=CClientExoApp::BeginIntro
- CClientExoApp::GetInGameGui gog 0x0040cc10 -> 0x0073f750: review:held_by=ClientApp::GetGuiInGame
- CClientExoApp::GetServerInfo gog 0x0040cd10 -> 0x0073f830: review:held_by=CClientExoApp::GetClientOptions
- CClientExoApp::AttachPlayerGameObjectToArea gog 0x0040cd30 -> 0x0073f850: review:held_by=CClientExoApp::GetServerInfo
- CClientExoApp::InitializeInGameGUI gog 0x0040cd50 -> 0x0073f870: review:held_by=CClientExoApp::AttachPlayerGameObjectToArea
- CClientExoApp::IsClientInitialized gog 0x0040ce80 -> 0x0073f9a0: review:held_by=CClientExoApp::Initialize
- CClientExoApp::SetDebugMode gog 0x0040cf90 -> 0x0073fa70: review:held_by=CClientExoApp::SetCaptureEvents
- CClientExoApp::GetHitCheckMaterialMask gog 0x0040d090 -> 0x0073fbb0: review:held_by=CClientExoApp::GetSWParty
- CClientExoApp::GetDisableMovies gog 0x0040d1a0 -> 0x0073fcc0: review:held_by=CClientExoApp::GetLoadBarState
- CClientExoApp::SetStartMenuMusicOnMovieEnd gog 0x0040d2b0 -> 0x0073fdd0: review:held_by=CClientExoApp::AquireLoadMusic
- CClientExoApp::GetPlayerCharacterSet gog 0x0040d4e0 -> 0x00740000: review:held_by=CClientExoApp::GetPlayerCharacterName
- CClientExoApp::GetAreaNotReadyToDisplay gog 0x0040d500 -> 0x00740020: review:held_by=CClientExoApp::GetPlayerCharacterSet
- CClientExoApp::SetAutoPaused gog 0x0040ddf0 -> 0x00740930: review:held_by=CClientExoApp::GetClientRangedAnimation
- CClientExoAppInternal::Initialize gog 0x0042df60 -> 0x007819e0: review:held_by=CClientExoAppInternal::GetModuleCamera
- CClientExoAppInternal::ServerModeChange gog 0x00431cc0 -> 0x00785030: review:held_by=CClientExoAppInternal::CreateQuestJournal
- CClientExoAppInternal::GetPlayerByPlayerID gog 0x004382a0 -> 0x0078bcc0: review:held_by=CClientExoAppInternal::ShutDownMoviePlayer
- CClientExoAppInternal::AttachPlayerGameObjectToArea gog 0x00438750 -> 0x0078c170: review:held_by=CClientExoAppInternal::CleanUpModuleServices
- CClientExoAppInternal::UpdateCamera gog 0x00438930 -> 0x0078c350: review:held_by=CClientExoAppInternal::GetPlayerByPlayerID
- CClientExoAppInternal::SetTexturePack gog 0x00439c70 -> 0x0078cf50: review:held_by=CClientExoAppInternal::ClearCachedCreatures
- CClientExoAppInternal::StopMenuMusic gog 0x0043adb0 -> 0x0078e7e0: review:held_by=CClientExoAppInternal::StartMenuMusic
- CClientExoAppInternal::StartLoadMusic gog 0x0043aec0 -> 0x0078e8f0: review:held_by=CClientExoAppInternal::StopMenuMusic
- CClientExoAppInternal::AquireLoadMusic gog 0x0043b0a0 -> 0x0078e350: review:held_by=CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList_2
- CClientExoAppInternal::GetObjectName gog 0x0043b150 -> 0x0078eba0: review:held_by=CClientExoAppInternal::FadeAndStopStreamingSoundSource
- CClientExoAppInternal::HideLoadScreen gog 0x00444a00 -> 0x00797d90: review:held_by=CClientExoAppInternal::SetInputActive
- CClientExoAppInternal::PlayMovie gog 0x004451c0 -> 0x00798580: review:held_by=CClientExoAppInternal::SetPlayerCharacterName
- CClientExoAppInternal::IsMoviePlaying gog 0x00445220 -> 0x00798c00: fail:trusted_holder=IsMoviePlaying
- CClientExoAppInternal::InitializeMoviePlayer_2 gog 0x0044ded0 -> 0x007a1130: review:held_by=CClientExoAppInternal::StartCreditSequence
- CClientExoAppInternal::ProcessInput gog 0x00459080 -> 0x007ac470: review:held_by=CClientExoAppInternal::Update
- CClientExoAppInternal::ActionIconToMouseCursor gog 0x00460b10 -> 0x007b3ca0: review:held_by=CClientExoAppInternal::HandleMouseClickInWorld
- CClientExoAppInternal::GetDefaultActions gog 0x00460d20 -> 0x007b40f0: review:held_by=CClientExoAppInternal::BuildHoverActions

## G4 metadata flags

- CClientExoApp::GetClientRangedAnimation gog 0x0040d990: ['gog_db_param_bad'] (gog_param=8 gog_retn=16 steam_retn=8)
- CClientExoAppInternal::DisplayMainMenu gog 0x0042db10: ['gog_db_param_bad'] (gog_param=0 gog_retn=4 steam_retn=0)
- CClientExoAppInternal::SetTexturePack gog 0x00439c70: ['gog_db_param_bad'] (gog_param=1 gog_retn=8 steam_retn=4)
- CClientExoAppInternal::StartMenuMusic gog 0x0043abf0: ['gog_db_param_bad'] (gog_param=0 gog_retn=12 steam_retn=0)
- CClientExoAppInternal::HideLoadScreen gog 0x00444a00: ['gog_db_param_bad'] (gog_param=4 gog_retn=12 steam_retn=8)
- CClientExoAppInternal::PlayMovie gog 0x004451c0: ['gog_db_param_bad'] (gog_param=8 gog_retn=20 steam_retn=4)
- CClientExoAppInternal::ShowObject gog 0x00445390: ['gog_db_param_bad'] (gog_param=12 gog_retn=16 steam_retn=12)
- CClientExoAppInternal::ActionIconToMouseCursor gog 0x00460b10: ['gog_db_param_bad'] (gog_param=4 gog_retn=8 steam_retn=0)

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
- steam-db / process | A decoy_prev built from a mis-slotted pipeline pick can be the true counterpart | B11a: both "accepted" decoys were real counterparts: B11a-2-11 (0x4381A0 -> 0x78BBC0, scalar deleting dtor) and B11a-3-52 (0x40D250 -> 0x73FD70, StopMenuMusic forwarder). The pipeline pick was one slot off (0x78BCC0 = GetPlayerByPlayerID, 0x73FD90 = StopLoadMusic), so "previous slot" was the right one. Before counting a decoy acceptance, check the decoy's fp against the GOG fp; an fp-identical decoy is a key error.
- steam-db / method | CClientExoApp forwarder block: one relocated and one out-of-block forwarder shift the slots | B11a: GOG App::EndLoadBar 0x40C510 is relinked in Steam to 0x7400D0 (between GetMiniGame 0x7400A0 and PlayLoadModuleMovies 0x7400F0), and GetClientOptions 0x40CCF0 sits at Steam 0x72FB00 in the CExoInput block. The positional pick for GOG 0x40C510-0x40D580 was one slot off (G6 held_by chain). Anchor App forwarders by the forwarded Internal callee and the caller set, never by slot.
- steam-db / method | GOG labels Internal member names on CExoArrayList<T> template instances, and Steam scatters those instances | B11a: GOG 0x44DE70-0x44E0D0 are ctor/dtor/SetSize instances (sz63/64/91) carrying Internal names (IsMoviePlaying, GetNearestVideoMode, HandleExitGameButton, GetLoadScreenVisible, AddMoveToModuleMovie, SetGameOver). Their Steam counterparts are 0x6D2CB0, 0x838F70, 0x7A18C0, 0x7A1900, 0x8A6110 and 0x7A19A0, outside the drift window. Find them with a whole-binary scan keyed on own fp + callee fp (the type-specific Allocate/vector-delete; the element size is the PUSH imm 0x24/0x3C) plus caller sets.
- steam-db / method | Place a rotated Internal name by its App forwarder's callee | B11a: GOG App::X forwards to the real Internal::X (GetLoadScreenVisible -> 0x444B60, IsMoviePlaying -> 0x445220, AddMoveToModuleMovie -> 0x445190, SetGameOver -> 0x448830 FUN_, IndicateObject -> 0x443170 labelled CSWCCreature::SwitchMode). The GOG DB also puts these names on unrelated bodies: 0x43A510 EndLoadBar = GetNearestVideoMode, 0x4452F0 StopMovie = ShutDownMoviePlayer, 0x444B60 IndicateObject = GetLoadScreenVisible. Confirm each with the Mac x86 shape.
- steam-db / process | In B11a every AUTO row inside the rotated windows was right (45/45) | B11a part z: 34 App rows in the slot-shift block and 11 Internal rows near 0x443170/0x444B60/0x4452F0/0x44DE70 were fp 1:1 and matched the Mac symbol (disp, callees, strings). The rotations hit only REVIEW rows. Mac Internal offsets above +0x3C0 are Windows - 4 (0x3C8 vs 0x3C4, 0x4D8 vs 0x4D4); that is uniform layout drift, not a contradiction.
- steam | CClientExoApp::EndLoadBar is a forwarder to an ICF-folded empty body | Steam 0x7400D0: `007400dd CALL 0x00623320`. 0x623320 is the 11 B return_void (= GOG 0x6133B0) with 162 callers, and Mac Internal::EndLoadBar is a 5 B stub. Hooking EndLoadBar only observes the call; the real load-bar teardown is elsewhere.
