# Opus gate B11b part a (1 of 3)

- batch: B11b, part: a, rows: 60 (rows 1-60 of 129 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 1 | 100% | CGuiInGame::Constructor | 0x004cd090 | 0x007bce00 | reviewer corrected_addr |
| 2 | 100% | CGuiInGame::InitializeGlobal | 0x004ce110 | 0x007bde80 | reviewer corrected_addr |
| 3 | 100% | CGuiInGame::SetConversationType | 0x004d00e0 | 0x007bfa50 | match cites a layout/offset difference |
| 4 | 100% | CGuiInGame::InitializeDialogAnimationData | 0x004d05c0 | 0x007bff30 | match cites a layout/offset difference, match on a mac_rotation row |
| 5 | 100% | CGuiInGame::SetDialogAnimationData | 0x004d06c0 | 0x007c0030 | match on a mac_rotation row |
| 6 | 100% | CGuiInGame::SetComputerCameraShot | 0x004d6890 | 0x007c6200 | match on a mac_rotation row |
| 7 | 100% | CGuiInGame::ResetComputerCameraShot | 0x004d6c40 | 0x007c65b0 | match on a mac_contra row, match on a mac_rotation row |
| 8 | 100% | CGuiInGame::DestroyDeathMenuGUI | 0x004db4b0 | 0x007cadb0 | label_suspect |
| 9 | 100% | CGuiInGame::IsLoopingDialogAnimation | 0x004db920 | 0x007cb220 | match on a mac_contra row, match on a mac_rotation row |
| 10 | 100% | CGuiInGame::ShowSubItemCreateMenu_2 | 0x004dba50 | 0x007cb350 | label_suspect |
| 11 | 100% | CGuiInGame::HideSubItemCreateMenu_2 | 0x004dbaf0 | 0x007cb3f0 | label_suspect |
| 12 | 100% | CGuiInGame::IsCameraDialog | 0x004dbda0 | 0x007cb6a0 | label_suspect |
| 13 | 100% | CGuiInGame::IsAnimationPlayingInDialog | 0x004dbde0 | 0x007cb6e0 | label_suspect |
| 14 | 100% | CGuiInGame::IsLoopingDialogStuntAnimation_2 | 0x004dbe20 | 0x007cb5e0 | reviewer corrected_addr, label_suspect |
| 15 | 100% | CGuiInGame::IsFadingInDialog | 0x004dbe50 | 0x007cb720 | reviewer corrected_addr, label_suspect |
| 16 | 100% | CGuiInGame::InitializePlaceableCameras | 0x004dc370 | 0x007cbc70 | label_suspect |
| 17 | 100% | CGuiInGame::HideSwoopUpgradeScreen | 0x004dd7d0 | 0x007cd0d0 | label_suspect |
| 18 | 100% | CGuiInGame::GetCanOrientInDialog | 0x004df1f0 | 0x007ceaf0 | label_suspect |
| 19 | 100% | CGuiInGame::IsOverlayDialogAnimation_2 | 0x004dfe10 | 0x007cf4b0 | reviewer corrected_addr |
| 20 | 100% | CGuiInGame::SetTutorialReason | 0x004e00e0 | 0x007cf960 | reviewer corrected_addr |
| 21 | 100% | CGuiInGame::ShowDialogMessage | 0x004e0bf0 | 0x007d04f0 | label_suspect |
| 22 | 100% | CGuiInGame::GetMiniMapVisible | 0x004e0c60 | 0x007d0560 | label_suspect |
| 23 | 100% | CGuiInGame::RemoveGUIPopUps | 0x004e0ca0 | 0x007d0560 | reviewer corrected_addr, label_suspect |
| 24 | 100% | CGuiInGame::ResetInterfaceFont | 0x004e0d20 | 0x007d0620 | label_suspect |
| 25 | 100% | CGuiInGame::CopyQuickSaveGameToFutureGame | 0x004e2390 | 0x007d1c90 | label_suspect |
| 26 | 100% | CSWGuiMainInterface::OnPanelAdded | 0x00517f80 | 0x00746950 | label_suspect |
| 27 | 100% | CSWGuiMainInterface::StartRotation | 0x00517fd0 | 0x007469a0 | label_suspect |
| 28 | 100% | CSWGuiMainInterface::OnCombatYButton | 0x00518570 | 0x00746f40 | label_suspect |
| 29 | 100% | CSWGuiMainInterface::DisplayToolTipMessage | 0x005185f0 | 0x00746fc0 | label_suspect |
| 30 | 100% | CSWGuiMainInterface::QueueActiveAction | 0x00518640 | 0x00747010 | label_suspect |
| 31 | 100% | CSWGuiMainInterface::SetActionDescription | 0x0051e680 | 0x0074cf00 | match cites a layout/offset difference |
| 32 | 100% | CSWGuiMainInterface::UpdateActionQueue | 0x0051e730 | 0x0074cfb0 | label_suspect |
| 33 | 100% | CSWGuiMainInterface::HandleButtonAction | 0x0051f0b0 | 0x0074d930 | match on a mac_contra row |
| 34 | 100% | CSWGuiMainInterface::OnMenuButtonPressed | 0x00522d00 | 0x00751580 | match on a mac_rotation row |
| 35 | 100% | CSWGuiMainInterface::DoTargetAction | 0x00523230 | 0x00751ab0 | match on a mac_contra row |
| 36 | 100% | CSWGuiMainInterface::GetActiveActionIdx | 0x00523280 | 0x00751b00 | label_suspect |
| 37 | 100% | CSWGuiMainInterface::UpdateActionMenuSelected | 0x00523370 | 0x00751bf0 | match on a mac_contra row, match on a mac_rotation row |
| 38 | 100% | CSWGuiMainInterface::ShowActionQueue | 0x00523510 | 0x00751d90 | label_suspect |
| 39 | 100% | CSWGuiMainInterface::OnClearOneButtonPressed | 0x005235c0 | 0x00751e40 | label_suspect |
| 40 | 100% | CSWGuiMainInterface::OnPauseButtonPressed | 0x00523670 | 0x00751ef0 | match on a mac_rotation row |
| 41 | 100% | CSWGuiMainInterface::SetTarget | 0x00523ab0 | 0x00752330 | label_suspect |
| 42 | 100% | CSWGuiMainInterface::GetActionsFromActionIndex | 0x00523ae0 | 0x00752360 | label_suspect |
| 43 | 100% | CSWGuiMainInterface::UpdateAndDrawFogOfWar | 0x00523b30 | 0x007523b0 | label_suspect |
| 44 | 100% | CSWGuiMainInterface::ShowFlashingStatus | 0x00524590 | 0x00752e10 | match on a mac_contra row |
| 45 | 100% | CSWGuiMainInterface::AddFloatyText | 0x00524620 | 0x00752ea0 | label_suspect |
| 46 | 100% | CSWGuiMainInterface::RemoveFloatyText | 0x00524a20 | 0x007532a0 | label_suspect |
| 47 | 100% | CSWGuiMainInterface::OnPanelRemoved | 0x00524ea0 | 0x00753720 | match on a mac_contra row, match on a mac_rotation row |
| 48 | 100% | CGuiInGame::ShowGameMenuChangeButtons | 0x005a4790 | 0x008b0890 | reviewer corrected_addr |
| 49 | sample | CSWGuiMainInterface::Destructor | 0x0051bec0 | 0x0074a750 | C0 |
| 50 | sample | CGuiInGame::InitializeMap | 0x004df5c0 | 0x007ceec0 | C0 |
| 51 | sample | CGuiInGame::GetMessageBuffer | 0x004d8410 | 0x007c7d20 | C0 |
| 52 | sample | CGuiInGame::ShowFlashingStatus | 0x004dfbb0 | 0x007cf4b0 | C0 |
| 53 | sample | CGuiInGame::SetLevelUpMode | 0x004d00b0 | 0x007bfa20 | C0 |
| 54 | sample | CSWGuiMainInterface::ResetPartyMemberItemUse | 0x005265a0 | 0x00754e20 | C0 |
| 55 | sample | CGuiInGame::IsFireForgetDialogAnimation | 0x004db710 | 0x007cae60 | C0 |
| 56 | sample | CGuiInGame::GetMessageBufferSize | 0x004d8430 | 0x007c7d40 | C0 |
| 57 | sample | CGuiInGame::SetMainInterfaceTarget | 0x004dee10 | 0x007ce710 | C0 |
| 58 | sample | CSWGuiMainInterface::OnCharacterExited | 0x00523a80 | 0x00752300 | C0 |
| 59 | sample | CGuiInGame::IsGlobalFading | 0x004dc340 | 0x007cbc40 | C0 |
| 60 | sample | CSWGuiMainInterface::HandleButtonAction_3 | 0x00523460 | 0x00751ce0 | C0 |
