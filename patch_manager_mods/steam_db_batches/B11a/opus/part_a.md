# Opus gate B11a part a (1 of 5)

- batch: B11a, part: a, rows: 60 (rows 1-60 of 248 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 1 | 100% | CClientExoApp::Constructor | 0x0040c3d0 | 0x0073ef30 | match cites a layout/offset difference |
| 2 | 100% | CClientExoApp::GetFPS | 0x0040c470 | 0x0073f410 | reviewer corrected_addr |
| 3 | 100% | CClientExoApp::BeginIntro | 0x0040c4f0 | 0x0073f490 | reviewer corrected_addr |
| 4 | 100% | CClientExoApp::CreateModule | 0x0040c730 | 0x0073f270 | match cites a layout/offset difference |
| 5 | 100% | CClientExoApp::AddObjectToHitcheckIgnoreList | 0x0040cd90 | 0x0073f8d0 | reviewer corrected_addr |
| 6 | 100% | CClientExoApp::RemoveObjectFromHitcheckIgnoreList | 0x0040cdb0 | 0x0073f8b0 | reviewer corrected_addr |
| 7 | 100% | CClientExoApp::ContinueMessageProcessing | 0x0040ce40 | 0x0073f980 | reviewer corrected_addr |
| 8 | 100% | CClientExoApp::Initialize | 0x0040ce60 | 0x0073f9a0 | reviewer corrected_addr |
| 9 | 100% | CClientExoApp::SetCaptureEvents | 0x0040cf30 | 0x0073fa70 | reviewer corrected_addr |
| 10 | 100% | CClientExoApp::SetCaptureEventPlayBack | 0x0040cf50 | 0x0073fa90 | reviewer corrected_addr |
| 11 | 100% | CClientExoApp::SetCaptureEventPlayBackRate | 0x0040cf70 | 0x0073fab0 | reviewer corrected_addr |
| 12 | 100% | CClientExoApp::SetDebugMode | 0x0040cf90 | 0x0073fa70 | reviewer corrected_addr |
| 13 | 100% | CClientExoApp::GetSWParty | 0x0040d070 | 0x0073fbb0 | reviewer corrected_addr |
| 14 | 100% | CClientExoApp::RestartMoviesIfNecessary | 0x0040d1c0 | 0x0073fd00 | reviewer corrected_addr |
| 15 | 100% | CClientExoApp::GetApplicationId | 0x0040d1e0 | 0x0073fd70 | reviewer corrected_addr |
| 16 | 100% | CClientExoApp::StopMenuMusic | 0x0040d250 | 0x0073fd90 | reviewer corrected_addr |
| 17 | 100% | CClientExoApp::StopLoadMusic | 0x0040d270 | 0x0073fd70 | reviewer corrected_addr |
| 18 | 100% | CClientExoApp::AquireLoadMusic | 0x0040d290 | 0x0073fdd0 | reviewer corrected_addr |
| 19 | 100% | CClientExoApp::GetPlayerCharacterName | 0x0040d4c0 | 0x00740000 | reviewer corrected_addr |
| 20 | 100% | CClientExoApp::BarkString | 0x0040d720 | 0x007404d0 | reviewer corrected_addr |
| 21 | 100% | CClientExoApp::RunDeathSequence | 0x0040d860 | 0x007403a0 | match cites a layout/offset difference |
| 22 | 100% | CClientExoApp::StartPazaakGame | 0x0040db90 | 0x007406d0 | match cites a layout/offset difference |
| 23 | 100% | CClientExoApp::EndPazaakGame | 0x0040dbc0 | 0x007404d0 | reviewer corrected_addr |
| 24 | 100% | CClientExoAppInternal::GetPlayerMovementInput | 0x0040df80 | 0x00740ac0 | reviewer corrected_addr |
| 25 | 100% | CClientExoApp::StartCreditSequence | 0x0040e4f0 | 0x00741030 | match cites a layout/offset difference |
| 26 | 100% | CClientExoAppInternal::GetModuleCamera | 0x0042dd90 | 0x007819e0 | reviewer corrected_addr |
| 27 | 100% | CClientExoAppInternal::Initialize | 0x0042df60 | 0x007819e0 | reviewer corrected_addr |
| 28 | 100% | CClientExoAppInternal::StallServerCode | 0x00430300 | 0x00783d80 | label_suspect |
| 29 | 100% | CClientExoAppInternal::GetPauseState | 0x00430a40 | 0x00784420 | reviewer corrected_addr |
| 30 | 100% | CClientExoAppInternal::DestroyQuestJournal_2 | 0x00431750 | 0x00785130 | label_suspect |
| 31 | 100% | CClientExoAppInternal::AddToPauseExclusionList_2 | 0x004317f0 | 0x007851d0 | label_suspect |
| 32 | 100% | CClientExoAppInternal::CreateWorldJournal | 0x00431850 | 0x00785230 | match cites a layout/offset difference |
| 33 | 100% | CClientExoAppInternal::DestroyWorldJournal | 0x004318e0 | 0x007852c0 | match cites a layout/offset difference |
| 34 | 100% | CClientExoAppInternal::LoadItemDropSoundList | 0x00436970 | 0x0078a420 | reviewer corrected_addr |
| 35 | 100% | CClientExoAppInternal::ShutDownMoviePlayer | 0x004381a0 | 0x0078bcc0 | reviewer corrected_addr, label_suspect |
| 36 | 100% | CClientExoAppInternal::CleanUpModuleServices | 0x00438230 | 0x0078bc50 | reviewer corrected_addr |
| 37 | 100% | CClientExoAppInternal::GetPlayerByPlayerID | 0x004382a0 | 0x0078bcc0 | reviewer corrected_addr |
| 38 | 100% | CClientExoAppInternal::GetItemByGameObjectID | 0x00438360 | 0x0078bd80 | reviewer corrected_addr |
| 39 | 100% | CClientExoAppInternal::GetCreatureByGameObjectID | 0x004383d0 | 0x0078bdf0 | reviewer corrected_addr |
| 40 | 100% | CClientExoAppInternal::GetAreaByGameObjectID | 0x00438440 | 0x0078be60 | reviewer corrected_addr |
| 41 | 100% | CClientExoAppInternal::GetTriggerByGameObjectID | 0x004384b0 | 0x0078bed0 | reviewer corrected_addr |
| 42 | 100% | CClientExoAppInternal::GetProjectileByGameObjectID | 0x00438520 | 0x0078bf40 | reviewer corrected_addr |
| 43 | 100% | CClientExoAppInternal::GetPlaceableByGameObjectID | 0x00438590 | 0x0078bfb0 | reviewer corrected_addr |
| 44 | 100% | CClientExoAppInternal::GetDoorByGameObjectID | 0x00438600 | 0x0078c020 | reviewer corrected_addr |
| 45 | 100% | CClientExoAppInternal::GetAreaOfEffectByGameObjectID | 0x00438670 | 0x0078c090 | reviewer corrected_addr |
| 46 | 100% | CClientExoAppInternal::GetSoundObjectByGameObjectID | 0x004386e0 | 0x0078c100 | reviewer corrected_addr |
| 47 | 100% | CClientExoAppInternal::AttachPlayerGameObjectToArea | 0x00438750 | 0x0078c170 | reviewer corrected_addr |
| 48 | 100% | CClientExoAppInternal::ResetDriveAcceleration | 0x004388e0 | 0x0078bc50 | reviewer corrected_addr |
| 49 | 100% | CClientExoAppInternal::InitializeInGameGUI | 0x00438910 | 0x0078bc50 | reviewer corrected_addr |
| 50 | 100% | CClientExoAppInternal::UpdateCamera | 0x00438930 | 0x0078c350 | reviewer corrected_addr |
| 51 | 100% | CClientExoAppInternal::GetPlayerCreature | 0x004394c0 | 0x0078cee0 | reviewer corrected_addr |
| 52 | 100% | CClientExoAppInternal::ContinueMessageProcessing | 0x00439570 | 0x0078cfa0 | reviewer corrected_addr |
| 53 | 100% | CClientExoAppInternal::SetVideoMode | 0x0043a1c0 | 0x0078dbf0 | reviewer corrected_addr |
| 54 | 100% | CClientExoAppInternal::EndLoadBar | 0x0043a510 | 0x0078df40 | label_suspect |
| 55 | 100% | CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList | 0x0043a5e0 | 0x0078e010 | label_suspect |
| 56 | 100% | CClientExoAppInternal::SetCharacterFile | 0x0043a610 | 0x0078e040 | match cites a layout/offset difference |
| 57 | 100% | CClientExoAppInternal::StopMenuMusic | 0x0043adb0 | 0x0078e7e0 | reviewer corrected_addr |
| 58 | 100% | CClientExoAppInternal::FadeAndStopStreamingSoundSource | 0x0043ae40 | 0x0078e870 | reviewer corrected_addr |
| 59 | 100% | CClientExoAppInternal::StartLoadMusic | 0x0043aec0 | 0x0078e8f0 | reviewer corrected_addr |
| 60 | 100% | CClientExoAppInternal::StopLoadMusic | 0x0043b020 | 0x0078ea50 | reviewer corrected_addr |
