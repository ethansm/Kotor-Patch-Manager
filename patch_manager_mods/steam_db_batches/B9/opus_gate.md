# Opus gate B9 (DESIGN 4.6)

Decision: **ESCALATE: decoy cards accepted -> send 100% of this batch's reviewer matches to Opus**

- decoy cards: 40 (per chunk {'1': 10, '2': 10, '3': 10, '4': 10}), accepted by the reviewer: 1, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 0 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

- B9-3-29 0x0064c780 -> 0x0051cd10 (decoy_prev)

## Opus reads 100% (24 rows)

- CServerExoApp::Constructor gog 0x0064b640 -> 0x0051bc20: reviewer corrected_addr
- CServerExoApp::GetObjectArray gog 0x0064baa0 -> 0x0051c080: match cites a layout/offset difference
- CServerExoApp::GetNetLayer gog 0x0064bce0 -> 0x0051c2c0: match cites a layout/offset difference
- CServerExoApp::GetSWSMessage gog 0x0064bd00 -> 0x0051c2e0: match cites a layout/offset difference
- CServerExoApp::Initialize gog 0x0064be70 -> 0x0051c450: match cites a layout/offset difference
- CServerExoApp::ContinueMessageProcessing gog 0x0064bee0 -> 0x0051c4c0: match cites a layout/offset difference
- CServerExoApp::InitiateModuleForPlayer gog 0x0064c070 -> 0x0051c650: match cites a layout/offset difference
- CServerExoApp::GetPauseState gog 0x0064c140 -> 0x0051c720: reviewer corrected_addr
- CServerExoApp::RemoveFromExclusionList gog 0x0064c1e0 -> 0x0051c7c0: reviewer corrected_addr
- CServerExoApp::UpdateSubareasOnMoveTo gog 0x0064c500 -> 0x0051cae0: reviewer corrected_addr
- CServerExoApp::CreatureAcquireItem gog 0x0064c680 -> 0x0051cc60: match cites a layout/offset difference
- CServerExoApp::ShrinkLineByCreatures gog 0x0064c940 -> 0x0051cf20: reviewer corrected_addr
- CServerExoApp::GetMoveToPausedCalendarDay gog 0x0064c9a0 -> 0x0051cf80: match cites a layout/offset difference
- CServerExoApp::GetMoveToPausedTimeOfDay gog 0x0064c9f0 -> 0x0051cfd0: match cites a layout/offset difference
- CServerExoApp::GetModulePreviouslyVisited gog 0x0064ca90 -> 0x0051d070: match cites a layout/offset difference
- CServerExoAppInternal::DebugValidatePath gog 0x00662b80 -> 0x006e6880: label_suspect
- CSWSModule::non_virtual_thunk_to_dtor_CSWSModule gog 0x00733ef0 -> 0x00561c80: reviewer corrected_addr
- CServerExoAppInternal::InitiateModuleForPlayer gog 0x007b8a10 -> 0x005356d0: match cites a layout/offset difference
- CServerExoAppInternal::PlayerListChange gog 0x007b8f70 -> 0x00535c30: match cites a layout/offset difference
- CServerExoAppInternal::IsInjured gog 0x007bbef0 -> 0x00538ca0: match cites a layout/offset difference
- CServerExoAppInternal::GetPauseState gog 0x007bbf20 -> 0x00538cd0: reviewer corrected_addr
- CServerExoAppInternal::RemoveFromExclusionList gog 0x007bc710 -> 0x005394c0: reviewer corrected_addr
- CServerExoAppInternal::Uninitialize gog 0x007bda10 -> 0x0053a7c0: label_suspect
- CServerExoAppInternal::GetMoveToWorldTime gog 0x007bfa90 -> 0x0053ccf0: reviewer corrected_addr

## Sample of the other reviewer matches (143 of 143)

- CSWSModule::AddObjectToLimbo gog 0x00731ee0 -> 0x0055f850 (C0)
- CServerExoAppInternal::UpDatePlayerPosition gog 0x007bbea0 -> 0x00538c50 (C0)
- CServerExoApp::GetActiveExclusionList gog 0x0064c210 -> 0x0051c7f0 (C0)
- CServerExoApp::GetPlayerLanguage gog 0x0064bc60 -> 0x0051c240 (C0)
- CServerExoApp::MovePlayerToArea gog 0x0064c090 -> 0x0051c670 (C0)
- CServerExoApp::GetServerMode gog 0x0064bd40 -> 0x0051c320 (C0)
- CServerExoAppInternal::AddCharListRequest gog 0x007bca50 -> 0x00539800 (C0)
- CServerExoApp::MainLoop gog 0x0064be90 -> 0x0051c470 (C0)
- CServerExoApp::GetClientObjectByPlayerId gog 0x0064c040 -> 0x0051c620 (C0)
- CServerExoApp::GetModuleDescription gog 0x0064b780 -> 0x0051bd60 (C0)
- CSWSModule::Destructor gog 0x007294a0 -> 0x00556cf0 (C0)
- CServerExoApp::IsInjured gog 0x0064c110 -> 0x0051c6f0 (C0)
- CServerExoApp::ResolvePlayerByFirstName gog 0x0064c230 -> 0x0051c810 (C0)
- CServerExoApp::ExecuteLoadModule gog 0x0064cab0 -> 0x0051d090 (C0)
- CServerExoApp::GetActionConversation gog 0x0064c760 -> 0x0051cd40 (C0)
- CServerExoAppInternal::ClientToServerObjectId gog 0x007bcf80 -> 0x00539d30 (C0)
- CServerExoApp::GetSysAdminList gog 0x0064bd20 -> 0x0051c300 (C0)
- CServerExoApp::HandleMessage gog 0x0064be40 -> 0x0051c420 (C0)
- CServerExoApp::ClearLastModuleFound gog 0x0064c900 -> 0x0051cee0 (C0)
- CSWSModule::RemoveObjectFromLimbo gog 0x00731f50 -> 0x0055f8c0 (C0)
- CServerExoApp::GetAreaOfEffectByGameObjectID gog 0x0064bbe0 -> 0x0051c1c0 (C0)
- CServerExoApp::GetActiveTimer gog 0x0064bd70 -> 0x0051c350 (C0)
- CServerExoAppInternal::GetActivePauseState gog 0x007bc010 -> 0x00538dc0 (C0)
- CServerExoApp::GetPauseTimer gog 0x0064bdd0 -> 0x0051c3b0 (C0)
- CServerExoApp::GetPlayerListString gog 0x0064bf80 -> 0x0051c560 (C0)
- CServerExoApp::GetStoreByGameObjectID gog 0x0064bba0 -> 0x0051c180 (C0)
- CServerExoApp::GetMoveToModulePending gog 0x0064b8a0 -> 0x0051be80 (C0)
- CServerExoAppInternal::ContinueMessageProcessing gog 0x007baa80 -> 0x00537830 (C0)
- CServerExoApp::SkipPastDialog gog 0x0064c0f0 -> 0x0051c6d0 (C0)
- CServerExoApp::SetIgnoreConversationStartRange gog 0x0064c780 -> 0x0051cd60 (C0)
- CServerExoApp::IsActionsPending gog 0x0064c600 -> 0x0051cbe0 (C0)
- CServerExoApp::SetMoveToPausedTimeOfDay gog 0x0064c9c0 -> 0x0051cfa0 (C0)
- CServerExoApp::GetMoveToWorldTime gog 0x0064ca50 -> 0x0051d030 (C0)
- CServerExoApp::SetDebugMode gog 0x0064bdf0 -> 0x0051c3d0 (C0)
- CServerExoApp::StopServices gog 0x0064c000 -> 0x0051c5e0 (C0)
- CSWSModule::non_virtual_thunk_to_AsSWSModule gog 0x00729480 -> 0x00556cd0 (C0)
- CServerExoApp::GetModuleName gog 0x0064b7d0 -> 0x0051bdb0 (C0)
- CServerExoApp::RunModule gog 0x0064bf60 -> 0x0051c540 (C0)
- CServerExoAppInternal::GetActiveExclusionList gog 0x007bc850 -> 0x00539600 (C0)
- CServerExoAppInternal::ClearLastModuleFound gog 0x007b28f0 -> 0x0052f570 (C0)
- CServerExoApp::SetMoveToWorldTime gog 0x0064ca10 -> 0x0051cff0 (C0)
- CServerExoApp::SetLoadingModule gog 0x0064b940 -> 0x0051bf20 (C0)
- CServerExoApp::AddToExclusionList gog 0x0064c1b0 -> 0x0051c790 (C0)
- CServerExoApp::GetActivePauseState gog 0x0064c160 -> 0x0051c740 (C0)
- CServerExoApp::SetPauseState gog 0x0064c180 -> 0x0051c760 (C0)
- CServerExoApp::GetAutoSavePending gog 0x0064b9a0 -> 0x0051bf80 (C0)
- CServerExoApp::GetDebugMode gog 0x0064be20 -> 0x0051c400 (C0)
- CServerExoApp::StartServices gog 0x0064bfe0 -> 0x0051c5c0 (C0)
- CServerExoAppInternal::ClearAreaCache gog 0x007baee0 -> 0x00537c90 (C0)
- CServerExoApp::GetTimestopTimer gog 0x0064bdb0 -> 0x0051c390 (C0)
- CServerExoApp::GetModuleLanguage gog 0x0064bc80 -> 0x0051c260 (C0)
- CServerExoApp::Shutdown gog 0x0064bf00 -> 0x0051c4e0 (C0)
- CServerExoAppInternal::GetFirstPCObject gog 0x007bce00 -> 0x00539bb0 (C0)
- CServerExoApp::GetLoadingModule gog 0x0064b920 -> 0x0051bf00 (C0)
- CServerExoAppInternal::IsOnActiveExclusionList gog 0x007bc7f0 -> 0x005395a0 (C0)
- CServerExoApp::GetFirstPCObject gog 0x0064c290 -> 0x0051c870 (C0)
- CServerExoApp::GetNormalOfPoint gog 0x0064c8d0 -> 0x0051ceb0 (C0)
- CServerExoApp::SetDoingStartNewModule gog 0x0064c8a0 -> 0x0051ce80 (C0)
- CServerExoApp::LoadCharacterFinish gog 0x0064ba80 -> 0x0051c060 (C0)
- CServerExoApp::SetMoveToPausedCalendarDay gog 0x0064c970 -> 0x0051cf50 (C0)
- CServerExoApp::PlayerListChange gog 0x0064beb0 -> 0x0051c490 (C0)
- CServerExoApp::GetWaypointByGameObjectID gog 0x0064bc00 -> 0x0051c1e0 (C0)
- CServerExoApp::LoadModule gog 0x0064bf40 -> 0x0051c520 (C0)
- CServerExoApp::GetFPS gog 0x0064b6e0 -> 0x0051bcc0 (C0)
- CServerExoApp::UpDatePlayerPosition gog 0x0064c4c0 -> 0x0051caa0 (C0)
- CServerExoAppInternal::TogglePauseState gog 0x007bbe70 -> 0x00538c20 (C0)
- CServerExoApp::GetConnectionLib gog 0x0064bfc0 -> 0x0051c5a0 (C0)
- CServerExoApp::AddCharListRequest gog 0x0064c270 -> 0x0051c850 (C0)
- CServerExoApp::SetActionConversation gog 0x0064c730 -> 0x0051cd10 (C1)
- CSWSModule::DeleteWorldJournalEntryStrref gog 0x00731970 -> 0x0055f2e0 (C1)
- CServerExoApp::SetSaveGameName gog 0x0064cad0 -> 0x0051d0b0 (C1)
- CServerExoAppInternal::StartServices gog 0x007b96e0 -> 0x005363a0 (C2)
- CServerExoApp::ClientToServerObjectId gog 0x0064c2d0 -> 0x0051c8b0 (C1)
- CServerExoApp::UnloadModule gog 0x0064bf20 -> 0x0051c500 (C1)
- CServerExoAppInternal::Shutdown gog 0x007baac0 -> 0x00537870 (C1)
- CServerExoAppInternal::IsActionsPending gog 0x007bd180 -> 0x00539f30 (C1)
- CServerExoApp::SetLoadFromSaveGame gog 0x0064c820 -> 0x0051ce00 (C1)
- CServerExoApp::GetAreaByGameObjectID gog 0x0064bb40 -> 0x0051c120 (C1)
- CServerExoAppInternal::ValidatePlayerLogin gog 0x007baa10 -> 0x005377c0 (C1)
- CServerExoAppInternal::UpdateShutdownTimer gog 0x007bcab0 -> 0x00539860 (C1)
- CSWSModule::SaveModuleIFOFinish gog 0x0072f300 -> 0x0055cb50 (C2)
- CServerExoApp::GetPlayerList gog 0x0064bcc0 -> 0x0051c2a0 (C1)
- CServerExoAppInternal::ShrinkLineByCreatures gog 0x007be0f0 -> 0x0053aea0 (C1)
- CServerExoAppInternal::StallEventSaveGame gog 0x007b7980 -> 0x00534600 (C2)
- CServerExoAppInternal::GetModulePreviouslyVisited gog 0x007bfcb0 -> 0x0053cf10 (C2)
- CServerExoApp::ClearAreaCache gog 0x0064c920 -> 0x0051cf00 (C1)
- CServerExoAppInternal::HasEnoughDiskSpaceForSaveGame gog 0x007c0210 -> 0x0053d470 (C2)
- CServerExoApp::GetEncounterByGameObjectID gog 0x0064bc20 -> 0x0051c200 (C1)
- CServerExoApp::TestSafeLocation gog 0x0064c7b0 -> 0x0051cd90 (C1)
- CSWSModule::SavePlayers gog 0x0072f830 -> 0x0055d080 (C2)
- CServerExoAppInternal::MainLoop gog 0x007b6bb0 -> 0x00533830 (C2)
- CServerExoAppInternal::GetSystemTimeAsDWORD64 gog 0x007be120 -> 0x0053aed0 (C1)
- CServerExoApp::GetDifficultyOption gog 0x0064c250 -> 0x0051c830 (C1)
- CServerExoApp::ComputeHeight gog 0x0064c460 -> 0x0051ca40 (C1)
- CServerExoAppInternal::AddToExclusionList gog 0x007bc600 -> 0x005393b0 (C1)
- CServerExoApp::GetServerAIMaster gog 0x0064bca0 -> 0x0051c280 (C1)
- CSWSModule::DeleteWorldJournalEntry gog 0x00731860 -> 0x0055f1d0 (C1)
- CServerExoApp::GetTriggerByGameObjectID gog 0x0064bb60 -> 0x0051c140 (C1)
- CServerExoApp::SetMoveToModulePending gog 0x0064b870 -> 0x0051be50 (C1)
- CServerExoAppInternal::Initialize gog 0x007b4920 -> 0x005315a0 (C1)
- CServerExoApp::GetWorldTimer gog 0x0064bd90 -> 0x0051c370 (C1)
- CServerExoApp::TogglePauseState gog 0x0064c0d0 -> 0x0051c6b0 (C1)
- CSWSModule::SaveModuleStart gog 0x0072dfe0 -> 0x0055b830 (C2)
- CServerExoAppInternal::CopyModuleToCurrentGame gog 0x007b4f60 -> 0x00531be0 (C1)
- CServerExoApp::LoadGame gog 0x0064b840 -> 0x0051be20 (C1)
- CSWSModule::SavePrimaryPlayerInfo gog 0x0072ffa0 -> 0x0055d7f0 (C2)
- CServerExoApp::GetModule gog 0x0064b7b0 -> 0x0051bd90 (C1)
- CServerExoAppInternal::MovePlayerToArea gog 0x007b8d50 -> 0x00535a10 (C2)
- CServerExoApp::GetFactionOfObject gog 0x0064c0b0 -> 0x0051c690 (C1)
- CServerExoAppInternal::StopServices gog 0x007b9ef0 -> 0x00536c10 (C2)
- CServerExoApp::SaveGame gog 0x0064b810 -> 0x0051bdf0 (C1)
- CSWSModule::GetWaypoint gog 0x0072a670 -> 0x00557ec0 (C1)
- CServerExoApp::SetLastConversation gog 0x0064c6c0 -> 0x0051cca0 (C1)
- CServerExoApp::SetButtonMashCheck gog 0x0064c870 -> 0x0051ce50 (C1)
- CServerExoApp::GetButtonMashCheck gog 0x0064c850 -> 0x0051ce30 (C1)
- CServerExoAppInternal::GetModuleName gog 0x007b2920 -> 0x0052f5a0 (C1)
- CServerExoApp::GetPlaceableByGameObjectID gog 0x0064bb80 -> 0x0051c160 (C1)
- CServerExoApp::GetLoadFromSaveGame gog 0x0064c800 -> 0x0051cde0 (C1)
- CServerExoApp::GetItemByGameObjectID gog 0x0064bb00 -> 0x0051c0e0 (C1)
- CServerExoApp::GetRoomIndex gog 0x0064c620 -> 0x0051cc00 (C1)
- CServerExoApp::GetSoundObjectByGameObjectID gog 0x0064bc40 -> 0x0051c220 (C1)
- CServerExoAppInternal::RunModule gog 0x007b9510 -> 0x005361d0 (C2)
- CSWSModule::IsObjectInLimbo gog 0x00731fc0 -> 0x0055f930 (C1)
- CServerExoAppInternal::StoreCurrentModule gog 0x007b39e0 -> 0x00530660 (C1)
- CSWSModule::PlotPathInArea gog 0x0072d430 -> 0x0055ac80 (C2)
- CServerExoApp::GetCreatureByGameObjectID gog 0x0064bb20 -> 0x0051c100 (C1)
- CServerExoApp::GetDoorByGameObjectID gog 0x0064bbc0 -> 0x0051c1a0 (C1)
- CServerExoAppInternal::UpdateClientGameObjects gog 0x007ba760 -> 0x00537510 (C1)
- CServerExoApp::ComputeSafeLocation gog 0x0064c7d0 -> 0x0051cdb0 (C1)
- CServerExoApp::SetMoveToModuleString gog 0x0064b8c0 -> 0x0051bea0 (C1)
- CServerExoAppInternal::DoModuleEnterSaveCleanup gog 0x007bfb10 -> 0x0053cd70 (C2)
- CServerExoApp::GetGlobalVariableTable gog 0x0064c2b0 -> 0x0051c890 (C1)
- CServerExoApp::SetExportAllCharactersPending gog 0x0064b8f0 -> 0x0051bed0 (C1)
- CServerExoApp::GetClientObjectByObjectId gog 0x0064c020 -> 0x0051c600 (C1)
- CServerExoApp::GetServerInfo gog 0x0064b760 -> 0x0051bd40 (C1)
- CServerExoAppInternal::DealWithLoadGameError gog 0x007b8970 -> 0x00535630 (C1)
- CServerExoAppInternal::UpdateClientGameObjectsForPlayer gog 0x007ba7e0 -> 0x00537590 (C1)
- CServerExoApp::SetAutoSavePending gog 0x0064b970 -> 0x0051bf50 (C1)
- CServerExoAppInternal::GetItemByGameObjectID gog 0x007bad50 -> 0x00537b00 (C1)
- CSWSModule::SaveModuleFAC gog 0x0072f420 -> 0x0055cc70 (C2)
- CServerExoApp::GetPlayerCreatureId gog 0x0064c310 -> 0x0051c8f0 (C1)
- CServerExoApp::GetFactionManager gog 0x0064c6a0 -> 0x0051cc80 (C1)
- CSWSModule::PlotInterAreaPath gog 0x0072d0b0 -> 0x0055a900 (C1)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CServerExoAppInternal::LoadPrimaryPlayer gog 0x007b46f0 -> 0x00531370
- sig: CServerExoAppInternal::UpdateMapData gog 0x007bdb00 -> 0x0053a8b0
- sig: CServerExoAppInternal::ResolvePlayerByFirstName gog 0x007bc8a0 -> 0x00539650
- sig: CSWSModule::TimeStopSanityCheck gog 0x00732020 -> 0x0055f990
- sig: CServerExoAppInternal::GetWaypointByGameObjectID gog 0x007bb0d0 -> 0x00537e80
- c2: CServerExoApp::ClearAllQueuedCombatActions gog 0x0064c5c0 -> 0x0051cba0
- c2: CSWSModule::RemoveModuleResources gog 0x00732700 -> 0x00560070
- c2: CServerExoAppInternal::SaveGame gog 0x007b2990 -> 0x0052f610
