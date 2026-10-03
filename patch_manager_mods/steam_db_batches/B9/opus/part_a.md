# Opus gate B9 part a (1 of 3)

- batch: B9, part: a, rows: 60 (rows 1-60 of 174 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 1 | 100% | CServerExoApp::Constructor | 0x0064b640 | 0x0051bcc0 | reviewer corrected_addr |
| 2 | 100% | CServerExoApp::GetObjectArray | 0x0064baa0 | 0x0051c080 | match cites a layout/offset difference |
| 3 | 100% | CServerExoApp::GetNetLayer | 0x0064bce0 | 0x0051c2c0 | match cites a layout/offset difference |
| 4 | 100% | CServerExoApp::GetSWSMessage | 0x0064bd00 | 0x0051c2e0 | match cites a layout/offset difference |
| 5 | 100% | CServerExoApp::Initialize | 0x0064be70 | 0x0051c450 | match cites a layout/offset difference |
| 6 | 100% | CServerExoApp::ContinueMessageProcessing | 0x0064bee0 | 0x0051c4c0 | match cites a layout/offset difference |
| 7 | 100% | CServerExoApp::InitiateModuleForPlayer | 0x0064c070 | 0x0051c650 | match cites a layout/offset difference |
| 8 | 100% | CServerExoApp::GetPauseState | 0x0064c140 | 0x0051c6f0 | reviewer corrected_addr |
| 9 | 100% | CServerExoApp::RemoveFromExclusionList | 0x0064c1e0 | 0x0051c790 | reviewer corrected_addr |
| 10 | 100% | CServerExoApp::UpdateSubareasOnMoveTo | 0x0064c500 | 0x0051ca40 | reviewer corrected_addr |
| 11 | 100% | CServerExoApp::CreatureAcquireItem | 0x0064c680 | 0x0051cc60 | match cites a layout/offset difference |
| 12 | 100% | CServerExoApp::ShrinkLineByCreatures | 0x0064c940 | 0x0051cc00 | reviewer corrected_addr |
| 13 | 100% | CServerExoApp::GetMoveToPausedCalendarDay | 0x0064c9a0 | 0x0051cf80 | match cites a layout/offset difference |
| 14 | 100% | CServerExoApp::GetMoveToPausedTimeOfDay | 0x0064c9f0 | 0x0051cfd0 | match cites a layout/offset difference |
| 15 | 100% | CServerExoApp::GetModulePreviouslyVisited | 0x0064ca90 | 0x0051d070 | match cites a layout/offset difference |
| 16 | 100% | CSWSModule::non_virtual_thunk_to_dtor_CSWSModule | 0x00733ef0 | 0x005615c0 | reviewer corrected_addr |
| 17 | 100% | CServerExoAppInternal::InitiateModuleForPlayer | 0x007b8a10 | 0x005356d0 | match cites a layout/offset difference |
| 18 | 100% | CServerExoAppInternal::PlayerListChange | 0x007b8f70 | 0x00535c30 | match cites a layout/offset difference |
| 19 | 100% | CServerExoAppInternal::IsInjured | 0x007bbef0 | 0x00538ca0 | match cites a layout/offset difference |
| 20 | 100% | CServerExoAppInternal::GetPauseState | 0x007bbf20 | 0x00538c20 | reviewer corrected_addr |
| 21 | 100% | CServerExoAppInternal::RemoveFromExclusionList | 0x007bc710 | 0x005393b0 | reviewer corrected_addr |
| 22 | 100% | CServerExoAppInternal::Uninitialize | 0x007bda10 | 0x0053a7c0 | label_suspect |
| 23 | 100% | CServerExoAppInternal::GetMoveToWorldTime | 0x007bfa90 | 0x0053cc80 | reviewer corrected_addr |
| 24 | sample | CSWSModule::AddObjectToLimbo | 0x00731ee0 | 0x0055f850 | C0 |
| 25 | sample | CServerExoAppInternal::UpDatePlayerPosition | 0x007bbea0 | 0x00538c50 | C0 |
| 26 | sample | CServerExoApp::GetActiveExclusionList | 0x0064c210 | 0x0051c7f0 | C0 |
| 27 | sample | CServerExoApp::GetPlayerLanguage | 0x0064bc60 | 0x0051c240 | C0 |
| 28 | sample | CServerExoApp::MovePlayerToArea | 0x0064c090 | 0x0051c670 | C0 |
| 29 | sample | CServerExoApp::GetServerMode | 0x0064bd40 | 0x0051c320 | C0 |
| 30 | sample | CServerExoAppInternal::AddCharListRequest | 0x007bca50 | 0x00539800 | C0 |
| 31 | sample | CServerExoApp::MainLoop | 0x0064be90 | 0x0051c470 | C0 |
| 32 | sample | CServerExoApp::GetClientObjectByPlayerId | 0x0064c040 | 0x0051c620 | C0 |
| 33 | sample | CServerExoApp::GetModuleDescription | 0x0064b780 | 0x0051bd60 | C0 |
| 34 | sample | CSWSModule::Destructor | 0x007294a0 | 0x00556cf0 | C0 |
| 35 | sample | CServerExoApp::IsInjured | 0x0064c110 | 0x0051c6f0 | C0 |
| 36 | sample | CServerExoApp::ResolvePlayerByFirstName | 0x0064c230 | 0x0051c810 | C0 |
| 37 | sample | CServerExoApp::ExecuteLoadModule | 0x0064cab0 | 0x0051d090 | C0 |
| 38 | sample | CServerExoApp::GetActionConversation | 0x0064c760 | 0x0051cd40 | C0 |
| 39 | sample | CServerExoAppInternal::ClientToServerObjectId | 0x007bcf80 | 0x00539d30 | C0 |
| 40 | sample | CServerExoApp::GetSysAdminList | 0x0064bd20 | 0x0051c300 | C0 |
| 41 | sample | CServerExoApp::HandleMessage | 0x0064be40 | 0x0051c420 | C0 |
| 42 | sample | CServerExoApp::ClearLastModuleFound | 0x0064c900 | 0x0051cee0 | C0 |
| 43 | sample | CSWSModule::RemoveObjectFromLimbo | 0x00731f50 | 0x0055f8c0 | C0 |
| 44 | sample | CServerExoApp::GetAreaOfEffectByGameObjectID | 0x0064bbe0 | 0x0051c1c0 | C0 |
| 45 | sample | CServerExoApp::GetActiveTimer | 0x0064bd70 | 0x0051c350 | C0 |
| 46 | sample | CServerExoAppInternal::GetActivePauseState | 0x007bc010 | 0x00538dc0 | C0 |
| 47 | sample | CServerExoApp::GetPauseTimer | 0x0064bdd0 | 0x0051c3b0 | C0 |
| 48 | sample | CServerExoApp::GetPlayerListString | 0x0064bf80 | 0x0051c560 | C0 |
| 49 | sample | CServerExoApp::GetStoreByGameObjectID | 0x0064bba0 | 0x0051c180 | C0 |
| 50 | sample | CServerExoApp::GetMoveToModulePending | 0x0064b8a0 | 0x0051be80 | C0 |
| 51 | sample | CServerExoAppInternal::ContinueMessageProcessing | 0x007baa80 | 0x00537830 | C0 |
| 52 | sample | CServerExoApp::SkipPastDialog | 0x0064c0f0 | 0x0051c6d0 | C0 |
| 53 | sample | CServerExoApp::SetIgnoreConversationStartRange | 0x0064c780 | 0x0051cd60 | C0 |
| 54 | sample | CServerExoApp::IsActionsPending | 0x0064c600 | 0x0051cbe0 | C0 |
| 55 | sample | CServerExoApp::SetMoveToPausedTimeOfDay | 0x0064c9c0 | 0x0051cfa0 | C0 |
| 56 | sample | CServerExoApp::GetMoveToWorldTime | 0x0064ca50 | 0x0051d030 | C0 |
| 57 | sample | CServerExoApp::SetDebugMode | 0x0064bdf0 | 0x0051c3d0 | C0 |
| 58 | sample | CServerExoApp::StopServices | 0x0064c000 | 0x0051c5e0 | C0 |
| 59 | sample | CSWSModule::non_virtual_thunk_to_AsSWSModule | 0x00729480 | 0x00556cd0 | C0 |
| 60 | sample | CServerExoApp::GetModuleName | 0x0064b7d0 | 0x0051bdb0 | C0 |
