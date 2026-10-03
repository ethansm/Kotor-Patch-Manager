# Opus gate B11a part b (2 of 5)

- batch: B11a, part: b, rows: 60 (rows 61-120 of 248 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 61 | 100% | CClientExoAppInternal::GetObjectName | 0x0043b150 | 0x0078eba0 | reviewer corrected_addr |
| 62 | 100% | CClientExoAppInternal::IndicateObject | 0x00444b60 | 0x00798540 | label_suspect |
| 63 | 100% | CClientExoAppInternal::SetAutoPaused | 0x0044aa20 | 0x0079e420 | reviewer corrected_addr |
| 64 | 100% | CClientExoAppInternal::ChangeCharacterToNextLivingPartyMember | 0x0044bd70 | 0x0079f770 | reviewer corrected_addr |
| 65 | 100% | CClientExoAppInternal::GetPausedByCombat | 0x0044deb0 | 0x007a1b90 | reviewer corrected_addr, label_suspect |
| 66 | 100% | CClientExoAppInternal::InitializeMoviePlayer_2 | 0x0044ded0 | 0x007a1130 | reviewer corrected_addr, label_suspect |
| 67 | sample | CClientExoApp::InitializeInGameGUI | 0x0040cd50 | 0x0073f870 | C0 |
| 68 | sample | CClientExoApp::MovieWindowActivated | 0x0040e730 | 0x00741270 | C0 |
| 69 | sample | CClientExoAppInternal::PerformRButtonDownAction | 0x00460420 | 0x007b3810 | C0 |
| 70 | sample | CClientExoApp::ClearCachedCreatures | 0x0040df10 | 0x00740a50 | C0 |
| 71 | sample | CClientExoApp::SendLoadGameRequest | 0x0040cbe0 | 0x0073f720 | C0 |
| 72 | sample | CClientExoAppInternal::HurryDeathSequence | 0x004489b0 | 0x0079c3b0 | C0 |
| 73 | sample | CClientExoApp::HideMouse | 0x0040e440 | 0x00740f80 | C0 |
| 74 | sample | CClientExoApp::GetLoadMusicByModuleName | 0x0040da20 | 0x00740560 | C0 |
| 75 | sample | CClientExoApp::AddObjectModelDirectlyToIgnoreList | 0x0040d200 | 0x0073fd20 | C0 |
| 76 | sample | CClientExoAppInternal::JournalUpdated | 0x00431950 | 0x00785330 | C0 |
| 77 | sample | CClientExoAppInternal::SetLoadScreenHint | 0x00449540 | 0x0079cf40 | C0 |
| 78 | sample | CClientExoApp::CancelMovie | 0x0040e420 | 0x00740f60 | C0 |
| 79 | sample | CClientExoApp::IsPlayerCreature | 0x0040d360 | 0x0073fe80 | C0 |
| 80 | sample | CClientExoApp::UpdateRoom | 0x0040db70 | 0x007406b0 | C0 |
| 81 | sample | CClientExoApp::GetPlayerByPlayerID | 0x0040cc90 | 0x0073f7d0 | C0 |
| 82 | sample | CClientExoApp::GetTriggerByGameObjectID | 0x0040ca50 | 0x0073f590 | C0 |
| 83 | sample | CClientExoApp::SaveOptions | 0x0040e000 | 0x00740b40 | C0 |
| 84 | sample | CClientExoApp::StartMenuMusic | 0x0040d220 | 0x0073fd40 | C0 |
| 85 | sample | CClientExoApp::DisplayMainMenu | 0x0040c6f0 | 0x0073f230 | C0 |
| 86 | sample | CClientExoApp::EnableMouseAtEndOfLoadModuleMovie | 0x0040e550 | 0x00741090 | C0 |
| 87 | sample | CClientExoAppInternal::RestartMoviesIfNecessary | 0x0043a820 | 0x0078e250 | C0 |
| 88 | sample | CClientExoApp::SetCharacterFile | 0x0040c6c0 | 0x0073f200 | C0 |
| 89 | sample | CClientExoApp::StartServices | 0x0040cfb0 | 0x0073fad0 | C0 |
| 90 | sample | CClientExoAppInternal::RemoveObjectFromHitcheckIgnoreList | 0x00439760 | 0x0078d190 | C0 |
| 91 | sample | CClientExoApp::GetActiveTimer | 0x0040cb90 | 0x0073f6d0 | C0 |
| 92 | sample | CClientExoAppInternal::AddObjectToHitcheckIgnoreList | 0x004396d0 | 0x0078d100 | C0 |
| 93 | sample | CClientExoApp::SetEventDescriptions | 0x0040cfe0 | 0x0073fb00 | C0 |
| 94 | sample | CClientExoApp::SetVideoMode | 0x0040d0b0 | 0x0073fbd0 | C0 |
| 95 | sample | CClientExoApp::SetGameOver | 0x0040d880 | 0x007403c0 | C0 |
| 96 | sample | CClientExoApp::Uninitialize | 0x0040cf10 | 0x0073fa30 | C0 |
| 97 | sample | CClientExoAppInternal::SetInputActive | 0x004443b0 | 0x00797d90 | C0 |
| 98 | sample | CClientExoApp::EmptyHitcheckIgnoreList | 0x0040cdd0 | 0x0073f8f0 | C0 |
| 99 | sample | CClientExoApp::QueryExitProgram | 0x0040d300 | 0x0073fe20 | C0 |
| 100 | sample | CClientExoApp::SetPauseState | 0x0040cb60 | 0x0073f6a0 | C0 |
| 101 | sample | CClientExoApp::HandleMessage | 0x0040ce10 | 0x0073f930 | C0 |
| 102 | sample | CClientExoApp::GetLoadScreenVisible | 0x0040d480 | 0x0073ffa0 | C0 |
| 103 | sample | CClientExoApp::SetCombatMode | 0x0040dd40 | 0x00740880 | C0 |
| 104 | sample | CClientExoApp::JournalUpdated | 0x0040c570 | 0x0073f0b0 | C0 |
| 105 | sample | CClientExoApp::GetPauseState | 0x0040cb40 | 0x0073f680 | C0 |
| 106 | sample | CClientExoAppInternal::MovieWindowDeactivated | 0x0044d6f0 | 0x007a10f0 | C0 |
| 107 | sample | CClientExoAppInternal::EnableMouseAtEndOfLoadModuleMovie | 0x0044d940 | 0x007a1340 | C0 |
| 108 | sample | CClientExoApp::ExitProgram | 0x0040c750 | 0x0073f290 | C0 |
| 109 | sample | CClientExoApp::GetProjectileByGameObjectID | 0x0040ca70 | 0x0073f5b0 | C0 |
| 110 | sample | CClientExoApp::LookUpAndPerformRumbleWithCutOff | 0x0040df30 | 0x00740a70 | C0 |
| 111 | sample | CClientExoApp::GetAreaOfEffectByGameObjectID | 0x0040cad0 | 0x0073f610 | C0 |
| 112 | sample | CClientExoApp::LoadOptions | 0x0040e020 | 0x00740b60 | C0 |
| 113 | sample | CClientExoAppInternal::PerformZMouseMoveAction | 0x00460460 | 0x007b3850 | C0 |
| 114 | sample | CClientExoAppInternal::SaveOptions | 0x0044bcc0 | 0x0079f6c0 | C0 |
| 115 | sample | CClientExoApp::GetActivePauseState | 0x0040d2e0 | 0x0073fe00 | C0 |
| 116 | sample | CClientExoAppInternal::PerformRButtonUpAction | 0x00460440 | 0x007b3830 | C0 |
| 117 | sample | CClientExoApp::GetPlayerList | 0x0040cc70 | 0x0073f7b0 | C0 |
| 118 | sample | CClientExoApp::DismissInGameGUI | 0x0040d7d0 | 0x00740310 | C0 |
| 119 | sample | CClientExoApp::StopCreditSequence | 0x0040e510 | 0x00741050 | C0 |
| 120 | sample | CClientExoApp::GetObjectName | 0x0040c5a0 | 0x0073f0e0 | C0 |
