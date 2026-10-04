# Opus gate B11b (DESIGN 4.6)

Decision: **ESCALATE: decoy cards accepted -> send 100% of this batch's reviewer matches to Opus**

- decoy cards: 30 (per chunk {'1': 10, '2': 10, '3': 10}), accepted by the reviewer: 2, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: pending (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

- B11b-2-54 0x004ce110 -> 0x007bde80 (decoy_next)
- B11b-3-24 0x004e00e0 -> 0x007cf9e0 (decoy_next)

## Opus reads 100% (48 rows)

- CGuiInGame::Constructor gog 0x004cd090 -> 0x007bce00: reviewer corrected_addr
- CGuiInGame::InitializeGlobal gog 0x004ce110 -> 0x007bde80: reviewer corrected_addr
- CGuiInGame::SetConversationType gog 0x004d00e0 -> 0x007bfa50: match cites a layout/offset difference
- CGuiInGame::InitializeDialogAnimationData gog 0x004d05c0 -> 0x007bff30: match cites a layout/offset difference, match on a mac_rotation row
- CGuiInGame::SetDialogAnimationData gog 0x004d06c0 -> 0x007c0030: match on a mac_rotation row
- CGuiInGame::SetComputerCameraShot gog 0x004d6890 -> 0x007c6200: match on a mac_rotation row
- CGuiInGame::ResetComputerCameraShot gog 0x004d6c40 -> 0x007c65b0: match on a mac_contra row, match on a mac_rotation row
- CGuiInGame::DestroyDeathMenuGUI gog 0x004db4b0 -> 0x007cadb0: label_suspect
- CGuiInGame::IsLoopingDialogAnimation gog 0x004db920 -> 0x007cb220: match on a mac_contra row, match on a mac_rotation row
- CGuiInGame::ShowSubItemCreateMenu_2 gog 0x004dba50 -> 0x007cb350: label_suspect
- CGuiInGame::HideSubItemCreateMenu_2 gog 0x004dbaf0 -> 0x007cb3f0: label_suspect
- CGuiInGame::IsCameraDialog gog 0x004dbda0 -> 0x007cb6a0: label_suspect
- CGuiInGame::IsAnimationPlayingInDialog gog 0x004dbde0 -> 0x007cb6e0: label_suspect
- CGuiInGame::IsLoopingDialogStuntAnimation_2 gog 0x004dbe20 -> 0x007cb5e0: reviewer corrected_addr, label_suspect
- CGuiInGame::IsFadingInDialog gog 0x004dbe50 -> 0x007cb720: reviewer corrected_addr, label_suspect
- CGuiInGame::InitializePlaceableCameras gog 0x004dc370 -> 0x007cbc70: label_suspect
- CGuiInGame::HideSwoopUpgradeScreen gog 0x004dd7d0 -> 0x007cd0d0: label_suspect
- CGuiInGame::GetCanOrientInDialog gog 0x004df1f0 -> 0x007ceaf0: label_suspect
- CGuiInGame::IsOverlayDialogAnimation_2 gog 0x004dfe10 -> 0x007cf4b0: reviewer corrected_addr
- CGuiInGame::SetTutorialReason gog 0x004e00e0 -> 0x007cf960: reviewer corrected_addr
- CGuiInGame::ShowDialogMessage gog 0x004e0bf0 -> 0x007d04f0: label_suspect
- CGuiInGame::GetMiniMapVisible gog 0x004e0c60 -> 0x007d0560: label_suspect
- CGuiInGame::RemoveGUIPopUps gog 0x004e0ca0 -> 0x007d0560: reviewer corrected_addr, label_suspect
- CGuiInGame::ResetInterfaceFont gog 0x004e0d20 -> 0x007d0620: label_suspect
- CGuiInGame::CopyQuickSaveGameToFutureGame gog 0x004e2390 -> 0x007d1c90: label_suspect
- CSWGuiMainInterface::OnPanelAdded gog 0x00517f80 -> 0x00746950: label_suspect
- CSWGuiMainInterface::StartRotation gog 0x00517fd0 -> 0x007469a0: label_suspect
- CSWGuiMainInterface::OnCombatYButton gog 0x00518570 -> 0x00746f40: label_suspect
- CSWGuiMainInterface::DisplayToolTipMessage gog 0x005185f0 -> 0x00746fc0: label_suspect
- CSWGuiMainInterface::QueueActiveAction gog 0x00518640 -> 0x00747010: label_suspect
- CSWGuiMainInterface::SetActionDescription gog 0x0051e680 -> 0x0074cf00: match cites a layout/offset difference
- CSWGuiMainInterface::UpdateActionQueue gog 0x0051e730 -> 0x0074cfb0: label_suspect
- CSWGuiMainInterface::HandleButtonAction gog 0x0051f0b0 -> 0x0074d930: match on a mac_contra row
- CSWGuiMainInterface::OnMenuButtonPressed gog 0x00522d00 -> 0x00751580: match on a mac_rotation row
- CSWGuiMainInterface::DoTargetAction gog 0x00523230 -> 0x00751ab0: match on a mac_contra row
- CSWGuiMainInterface::GetActiveActionIdx gog 0x00523280 -> 0x00751b00: label_suspect
- CSWGuiMainInterface::UpdateActionMenuSelected gog 0x00523370 -> 0x00751bf0: match on a mac_contra row, match on a mac_rotation row
- CSWGuiMainInterface::ShowActionQueue gog 0x00523510 -> 0x00751d90: label_suspect
- CSWGuiMainInterface::OnClearOneButtonPressed gog 0x005235c0 -> 0x00751e40: label_suspect
- CSWGuiMainInterface::OnPauseButtonPressed gog 0x00523670 -> 0x00751ef0: match on a mac_rotation row
- CSWGuiMainInterface::SetTarget gog 0x00523ab0 -> 0x00752330: label_suspect
- CSWGuiMainInterface::GetActionsFromActionIndex gog 0x00523ae0 -> 0x00752360: label_suspect
- CSWGuiMainInterface::UpdateAndDrawFogOfWar gog 0x00523b30 -> 0x007523b0: label_suspect
- CSWGuiMainInterface::ShowFlashingStatus gog 0x00524590 -> 0x00752e10: match on a mac_contra row
- CSWGuiMainInterface::AddFloatyText gog 0x00524620 -> 0x00752ea0: label_suspect
- CSWGuiMainInterface::RemoveFloatyText gog 0x00524a20 -> 0x007532a0: label_suspect
- CSWGuiMainInterface::OnPanelRemoved gog 0x00524ea0 -> 0x00753720: match on a mac_contra row, match on a mac_rotation row
- CGuiInGame::ShowGameMenuChangeButtons gog 0x005a4790 -> 0x008b0890: reviewer corrected_addr

## Sample of the other reviewer matches (73 of 73)

- CSWGuiMainInterface::Destructor gog 0x0051bec0 -> 0x0074a750 (C0)
- CGuiInGame::InitializeMap gog 0x004df5c0 -> 0x007ceec0 (C0)
- CGuiInGame::GetMessageBuffer gog 0x004d8410 -> 0x007c7d20 (C0)
- CGuiInGame::ShowFlashingStatus gog 0x004dfbb0 -> 0x007cf4b0 (C0)
- CGuiInGame::SetLevelUpMode gog 0x004d00b0 -> 0x007bfa20 (C0)
- CSWGuiMainInterface::ResetPartyMemberItemUse gog 0x005265a0 -> 0x00754e20 (C0)
- CGuiInGame::IsFireForgetDialogAnimation gog 0x004db710 -> 0x007cae60 (C0)
- CGuiInGame::GetMessageBufferSize gog 0x004d8430 -> 0x007c7d40 (C0)
- CGuiInGame::SetMainInterfaceTarget gog 0x004dee10 -> 0x007ce710 (C0)
- CSWGuiMainInterface::OnCharacterExited gog 0x00523a80 -> 0x00752300 (C0)
- CGuiInGame::IsGlobalFading gog 0x004dc340 -> 0x007cbc40 (C0)
- CSWGuiMainInterface::HandleButtonAction_3 gog 0x00523460 -> 0x00751ce0 (C0)
- CGuiInGame::SetupPauseGuiExtent gog 0x004e09d0 -> 0x007d02d0 (C0)
- CSWGuiMainInterface::OnCharacterClicked gog 0x005237d0 -> 0x00752050 (C2)
- CSWGuiMainInterface::DoPersonalAction gog 0x00522ed0 -> 0x00751750 (C2)
- CSWGuiMainInterface::HandleButtonActionX gog 0x0051f070 -> 0x0074d8f0 (C1)
- CSWGuiMainInterface::UpdateActionQueue_2 gog 0x0051f400 -> 0x0074dc80 (C1)
- CSWGuiMainInterface::Constructor gog 0x00518840 -> 0x00747210 (C2)
- CGuiInGame::HideUpgradeScreen gog 0x004de2e0 -> 0x007cdbe0 (C1)
- CGuiInGame::GetCanHeadFollowInDialog gog 0x004dfff0 -> 0x007cf8f0 (C1)
- CSWGuiMainInterface::ResetFont gog 0x00524790 -> 0x00753010 (C1)
- CGuiInGame::RePopulateMainInterface gog 0x004df250 -> 0x007ceb50 (C1)
- CSWGuiMainInterface::SelectNextPersonalAction gog 0x00523bd0 -> 0x00752450 (C1)
- CSWGuiMainInterface::Destructor_2 gog 0x0051c510 -> 0x0074ad90 (C1)
- CSWGuiMainInterface::ShowActionIcon gog 0x0051ff70 -> 0x0074e7f0 (C1)
- CSWGuiMainInterface::OnCharacterRightClicked gog 0x00522e30 -> 0x007516b0 (C1)
- CGuiInGame::ShowDialogEntry gog 0x004d01a0 -> 0x007bfb10 (C2)
- CSWGuiMainInterface::OnCharacterEntered gog 0x00523a40 -> 0x007522c0 (C1)
- CGuiInGame::ClearDialogNonDependantList gog 0x004e0b70 -> 0x007d0470 (C1)
- CGuiInGame::AppendToCombatBuffer gog 0x004ce840 -> 0x007be1b0 (C1)
- CSWGuiMainInterface::PopulateMenus gog 0x0051e760 -> 0x0074cfe0 (C2)
- CGuiInGame::SetGlobalUnpauseableState gog 0x004e11f0 -> 0x007d0af0 (C1)
- CGuiInGame::HidePowersFeatsSkillsDebugMenu gog 0x004dbc40 -> 0x007cb540 (C1)
- CSWGuiMainInterface::HandleInputEvent gog 0x00518020 -> 0x007469f0 (C2)
- CGuiInGame::PrevSWInGameGui gog 0x004daac0 -> 0x007ca3c0 (C1)
- CSWGuiMainInterface::ClearOneAction gog 0x00522930 -> 0x007511b0 (C2)
- CSWGuiMainInterface::RevertActionBarStates gog 0x0051f1c0 -> 0x0074da40 (C2)
- CSWGuiMainInterface::UpdateStickyTarget gog 0x00523ee0 -> 0x00752760 (C2)
- CGuiInGame::IsOnDialogNonDependantList gog 0x004e0ae0 -> 0x007d03e0 (C1)
- CSWGuiMainInterface::OnCancelPartyItemSelect gog 0x00526570 -> 0x00754df0 (C1)
- CSWGuiMainInterface::Draw gog 0x0051cb70 -> 0x0074b3f0 (C2)
- CSWGuiMainInterface::HideHelp gog 0x00517df0 -> 0x007467c0 (C2)
- CGuiInGame::AddFloatyText gog 0x004df580 -> 0x007cee80 (C1)
- CSWGuiMainInterface::UpdateGameMenu gog 0x005211d0 -> 0x0074fa50 (C2)
- CSWGuiMainInterface::SetupPauseGuiExtent gog 0x005246d0 -> 0x00752f50 (C1)
- CSWGuiMainInterface::UpdateActionDescription gog 0x0051e410 -> 0x0074cc90 (C1)
- CGuiInGame::TutorialReasonHasShown gog 0x004e0110 -> 0x007cfa10 (C1)
- CSWGuiMainInterface::OnStealthButtonPressed gog 0x00523760 -> 0x00751fe0 (C2)
- CGuiInGame::ChangeCharacter gog 0x004e09b0 -> 0x007d02b0 (C1)
- CSWGuiMainInterface::AddHelpPanel gog 0x00517490 -> 0x00745e60 (C2)
- CSWGuiMainInterface::PopupActionMenu gog 0x0051eca0 -> 0x0074d520 (C2)
- CGuiInGame::IsLoopingDialogStuntAnimation gog 0x004db8f0 -> 0x007cb1f0 (C1)
- CSWGuiMainInterface::GetActionIcon gog 0x0051f830 -> 0x0074e0b0 (C2)
- CSWGuiMainInterface::SetCombatMessage gog 0x00520390 -> 0x0074ec10 (C2)
- CSWGuiMainInterface::OnSoloButtonPressed gog 0x00523720 -> 0x00751fa0 (C2)
- CSWGuiMainInterface::HitCheckMouse gog 0x0051cb20 -> 0x0074b3a0 (C1)
- CGuiInGame::IsDialogAnimationRange1000_1727 gog 0x004db8c0 -> 0x007cb1c0 (C1)
- CSWGuiMainInterface::PopupGamepadMenu gog 0x00524e00 -> 0x00753680 (C1)
- CSWGuiMainInterface::OnSwapWeaponsPressed gog 0x00526530 -> 0x00754db0 (C2)
- CGuiInGame::SetTutorialReasonTracked gog 0x004e0170 -> 0x007cfa70 (C1)
- CGuiInGame::NextSWInGameGui gog 0x004da930 -> 0x007ca230 (C1)
- CSWGuiMainInterface::InitializeMap gog 0x00522390 -> 0x00750c10 (C2)
- CSWGuiMainInterface::Update gog 0x0051daa0 -> 0x0074c320 (C2)
- CSWGuiMainInterface::UpdatePortraits gog 0x00520610 -> 0x0074ee90 (C2)
- CSWGuiMainInterface::UpdateIndicator gog 0x005212a0 -> 0x0074fb20 (C2)
- CSWGuiMainInterface::HandleButtonAction_2 gog 0x0051f180 -> 0x0074da00 (C1)
- CSWGuiMainInterface::DrawMap gog 0x00521f10 -> 0x00750790 (C2)
- CGuiInGame::HideGalaxyMapGui gog 0x004db020 -> 0x007ca920 (C1)
- CGuiInGame::HideChemicalUpgradeScreen gog 0x004de3c0 -> 0x007cdcc0 (C1)
- CSWGuiMainInterface::OnClearAllButtonPressed gog 0x00523b60 -> 0x007523e0 (C2)
- CGuiInGame::SetCombatMessage gog 0x004e0c30 -> 0x007d0530 (C1)
- CGuiInGame::HideLoadModuleDebugMenu gog 0x004dbba0 -> 0x007cb4a0 (C1)
- CSWGuiMainInterface::SelectPrevPersonalAction gog 0x00523d50 -> 0x007525d0 (C1)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CGuiInGame::SetDialogSelection gog 0x004ddbd0 -> 0x007cd4d0
- sig: CGuiInGame::UpdateMessageGUI gog 0x004dd740 -> 0x007cd040
- sig: CGuiInGame::LoadCameraModelForDialog gog 0x004d46f0 -> 0x007c4060
- sig: CGuiInGame::LoadStuntModelsForDialog gog 0x004d8a80 -> 0x007c8390
- sig: CGuiInGame::HandleDialogEntry gog 0x004dc660 -> 0x007cbf60
- c2: CGuiInGame::AppendToMsgBuffer gog 0x004ce720 -> 0x007be090
- c2: CGuiInGame::GetFadeOverridePanelVisible gog 0x004e1490 -> 0x007d0d90
- c2: CGuiInGame::DisplayDeathMessage gog 0x004e2730 -> 0x007d2030
