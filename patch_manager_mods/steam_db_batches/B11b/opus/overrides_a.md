# B11b Opus gate, part a (60 rows: 48 "100%" + 12 "sample")

Verdicts: verified 20, likely 4, label_suspect 36, rejected 0, unresolved 0.
Address corrections vs the part's steam_addr: 6 (rows 14, 15, 19, 20, 23 and 55). Row 48 keeps the reviewer's corrected address.
Every correction was checked against the GOG fp and the Steam asm.

## Method
- **Alignment.** I sequence-aligned the GOG and Steam fp tables (size, ret, n_insn, disp, imm) over two ranges:
  - CGuiInGame: GOG 0x4cd090-0x4e2800 vs Steam 0x7bce00-0x7d2100.
  - CSWGuiMainInterface: GOG 0x517490-0x5265d0 vs Steam 0x745e60-0x754ed0.
  - Both align 1:1 apart from template/library inserts (CSWGuiPanel/CSWGuiControl dtors at GOG 0x4ce320-0x4ce6e0, and so on).
  - Every part row sits on an `=` line. The U4-mapper rows were rechecked this way plus callee/caller topology, so none rests on U4.
- **Fragments.**
  - GOG 0x4d6890 (r=-1) + 0x4d6920 (0 callers) = Steam 0x7c6200 (803 B, `007c6520 RET 0xc`). The row ships the entry; 0x4d6920 must never ship.
  - GOG 0x4db020 + ShowDebugMenu 0x4db100 = Steam 0x7ca920 (already B0). Not in this part.
- **Layout.** No layout shift: the CSWGuiMainInterface ctor is 13027 B on both sides, and disps such as 0x1c7c, 0x15230 and 0x732c are identical.
- **Labels.** Mac x86 real symbols decide the label calls (Mac vtable = Win + 4; MainInterface 0x1c3x Mac = 0x1c7x Win, from HandleButtonAction).
  - B0 already shipped six of these Steam addresses under the Mac-proven names. Each one agrees with my label_suspect "fits" name:
    - IsAnimationPlayingInDialog 0x7cadb0
    - IsFadingInDialog 0x7cb720
    - IsCameraDialog 0x7cb750
    - GetCanOrientInDialog 0x7cf710
    - GetMiniMapVisible 0x7d05a0
    - RemoveGUIPopUps 0x7d0620
  - These rows must NOT ship under their GOG names: they would collide on UNIQUE(class, function).

## Accepted decoys: both are key errors
- **B11b-2-54, 0x4ce110 -> 0x7bde80 (decoy_next).** The real card carried 0x7bdbc0, which is ~CGuiInGame (GOG 0x4cde50, sz702).
  - 0x7bde80 is the true InitializeGlobal: sz515, `007bdecb PUSH 0x9c8` + `007bded0 CALL 0x919723`, caller Initialize 0x7819e0.
  - The reviewer was right.
- **B11b-3-24, 0x4e00e0 -> 0x7cf9e0 (decoy_next).** The real card carried 0x7cf960, which is TutorialReasonWillShowWindow (GOG 0x4e0060, sz117).
  - 0x7cf9e0 is the true SetTutorialReason: sz48, `007cfa05 CALL 0x898f60` (TutorialBox::SetTutorialReason).
  - The reviewer was right.
- Same mechanism as B11a: decoy_prev/next were built around a mis-slotted pick. Effective wrong-decoy acceptance in this batch: 0.

## Label rotations found (GOG DB, read-only)
- **CGuiInGame 0x4dbda0-0x4dbe50.**
  - IsFadingInDialog = 0x4dbe20.
  - IsCameraDialog = 0x4dbe50.
  - 0x4dbda0 is a letterbox SetFade forwarder.
  - 0x4dbde0 = DialogGuiDoneScroll.
  - IsAnimationPlayingInDialog = 0x4db4b0.
- **CGuiInGame 0x4dba50/0x4dbaf0** = CleanUpEntryNode / CleanUpReplyNode.
  - ResetComputerCameraShot = 0x4d6bc0 (GOG calls it CleanUpReplyNode).
  - GOG CleanUpEntryNode 0x4d6920 is the fragment described above.
- **CGuiInGame 0x4e0c60/0x4e0ca0/0x4e0d20/0x4e2390** = ShowDialogMessage / GetMiniMapVisible / RemoveGUIPopUps / ResetInterfaceFont.
  - CopyQuickSaveGameToFutureGame = 0x4e2040 (GOG calls it CSWGuiSaveLoad::CopyGameToFutureGame).
- **CGuiInGame, other.**
  - GetCanOrientInDialog = 0x4dfe10.
  - 0x4df1f0 = GetAreaTranstionPosition.
  - HideSwoopUpgradeScreen = 0x4de610.
  - InitializePlaceableCameras = 0x4dc3b0.
- **CSWGuiMainInterface 0x518570/0x523280** = GetActiveActionIdx / OnPersonalActionExit (the reported permutation).
  - 0x51e730 = SetTarget.
  - 0x523ab0 = OnClearOneButtonPressed.
  - 0x523ae0 = ClearOneAction.
  - 0x523b30 = OnClearAllButtonPressed.
  - 0x523b60 = ClearAllActions.
  - 0x523230 = OnDefaultActionFocus.
  - 0x523370 = OnTargetActionFocus.
  - 0x5233b0/0x523460/0x523510/0x5235c0 = the four On{Action,Target}{Up,Down}ArrowPressed handlers. Up/Down comes from the Prev/Next callee (inference).
  - Real DoTargetAction = 0x522e90.
  - Real GetActionsFromActionIndex = 0x51ebb0.
  - Real UpdateActionQueue = 0x51f400.
  - Real ShowFlashingStatus = 0x522610.
  - Real AddFloatyText = 0x5226b0.
  - Real OnCombatYButton = 0x522930.

## Symmetric pairs (each side anchored)
| Pair | Anchor |
|---|---|
| 0x4db560/0x4db710 (sz423 twins) | 0x7cae60 calls 0x7cb1f0 (= 0x4db8f0); 0x7cb010 calls `007cb149 CALL 0x7cb220` (= 0x4db920) |
| 0x4dd7b0/0x4dd7d0 | BarkBubble Pause sz81 vs Resume sz108 callee |
| 0x4dba50/0x4dbaf0 | ClearRepliesData call `007cb448` only in 0x7cb3f0 |
| 0x523510/0x5235c0 | SelectPrevAction sz436 (0x744410) vs SelectNextAction sz420 (0x744260) |
| 0x5233b0/0x523460 | CALL 0x7525d0 vs `00751d7f CALL 0x752450` |
| 0x523ab0/0x523b30 | CALL 0x752360 vs 0x7523e0 |
| 0x524590/0x524620 | `+0x732c` = 1 vs 0, sizes 139/170 |
| 0x51bec0 vs 0x51bd90 | dtor callee sz1551 |

## Per row (short)
| # | GOG | GOG name | final Steam | verdict | anchor / fits |
|---|---|---|---|---|---|
| 1 | 0x4cd090 | Constructor | 0x7bce00 | verified | 'Compass'/'Display Options' pushes, StartServices caller |
| 2 | 0x4ce110 | InitializeGlobal | 0x7bde80 | verified | PUSH 0x9c8 + new, GetSWGuiManager |
| 3 | 0x4d00e0 | SetConversationType | 0x7bfa50 | verified | PUSH 0x45c0, CALL SetType 0x8bd7a0 |
| 4 | 0x4d05c0 | InitializeDialogAnimationData | 0x7bff30 | verified | calls Store/Clear...AnimationData, +0x1b8 |
| 5 | 0x4d06c0 | SetDialogAnimationData | 0x7c0030 | verified | bound check +0x1b8, RET 0xc, 3 callers |
| 6 | 0x4d6890 | SetComputerCameraShot | 0x7c6200 | verified | merged entry; AddPanel, 3 callers |
| 7 | 0x4d6c40 | ResetComputerCameraShot | 0x7c65b0 | label_suspect | RestoreCamera + 0x2772; Mac name = 0x4d6bc0 shape |
| 8 | 0x4db4b0 | DestroyDeathMenuGUI | 0x7cadb0 | label_suspect | fits IsAnimationPlayingInDialog |
| 9 | 0x4db920 | IsLoopingDialogAnimation | 0x7cb220 | label_suspect | 0x3e8..0x52f range; Mac name is a 318-B 2DA fn |
| 10 | 0x4dba50 | ShowSubItemCreateMenu_2 | 0x7cb350 | label_suspect | fits CleanUpEntryNode |
| 11 | 0x4dbaf0 | HideSubItemCreateMenu_2 | 0x7cb3f0 | label_suspect | fits CleanUpReplyNode |
| 12 | 0x4dbda0 | IsCameraDialog | 0x7cb6a0 | label_suspect | SetFade forwarder, RET 0x10 |
| 13 | 0x4dbde0 | IsAnimationPlayingInDialog | 0x7cb6e0 | label_suspect | fits DialogGuiDoneScroll |
| 14 | 0x4dbe20 | IsLoopingDialogStuntAnimation_2 | 0x7cb720 (was 0x7cb5e0) | label_suspect | IsFadeDone via +0x68, fits IsFadingInDialog |
| 15 | 0x4dbe50 | IsFadingInDialog | 0x7cb750 (was 0x7cb720) | label_suspect | +0x3c==+0x40, fits IsCameraDialog |
| 16 | 0x4dc370 | InitializePlaceableCameras | 0x7cbc70 | label_suspect | fade float predicate (Mac IsGlobalFadeObscuring?) |
| 17 | 0x4dd7d0 | HideSwoopUpgradeScreen | 0x7cd0d0 | label_suspect | BarkBubble::Resume forwarder |
| 18 | 0x4df1f0 | GetCanOrientInDialog | 0x7ceaf0 | label_suspect | fits GetAreaTranstionPosition |
| 19 | 0x4dfe10 | IsOverlayDialogAnimation_2 | 0x7cf710 (was 0x7cf4b0) | label_suspect | fits GetCanOrientInDialog |
| 20 | 0x4e00e0 | SetTutorialReason | 0x7cf9e0 (was 0x7cf960) | verified | CALL TutorialBox::SetTutorialReason |
| 21 | 0x4e0bf0 | ShowDialogMessage | 0x7d04f0 | label_suspect | byte getter +0xa900, BarkBubble::Draw caller |
| 22 | 0x4e0c60 | GetMiniMapVisible | 0x7d0560 | label_suspect | fits ShowDialogMessage |
| 23 | 0x4e0ca0 | RemoveGUIPopUps | 0x7d05a0 (was 0x7d0560) | label_suspect | fits GetMiniMapVisible |
| 24 | 0x4e0d20 | ResetInterfaceFont | 0x7d0620 | label_suspect | fits RemoveGUIPopUps |
| 25 | 0x4e2390 | CopyQuickSaveGameToFutureGame | 0x7d1c90 | label_suspect | fits ResetInterfaceFont (UpdateAllFonts caller) |
| 26 | 0x517f80 | OnPanelAdded | 0x746950 | label_suspect | 9-slot reset, direct callers |
| 27 | 0x517fd0 | StartRotation | 0x7469a0 | label_suspect | panel ptr by index (0x750 stride) |
| 28 | 0x518570 | OnCombatYButton | 0x746f40 | label_suspect | fits GetActiveActionIdx |
| 29 | 0x5185f0 | DisplayToolTipMessage | 0x746fc0 | label_suspect | returns button index |
| 30 | 0x518640 | QueueActiveAction | 0x747010 | label_suspect | HandleMouseMove to button |
| 31 | 0x51e680 | SetActionDescription | 0x74cf00 | verified | CALL SetText 0x416e30 |
| 32 | 0x51e730 | UpdateActionQueue | 0x74cfb0 | label_suspect | fits SetTarget |
| 33 | 0x51f0b0 | HandleButtonAction | 0x74d930 | verified | CALL HandleButtonActionX + DoPersonalAction |
| 34 | 0x522d00 | OnMenuButtonPressed | 0x751580 | verified | CALL ShowSWInGameGui, SetInputClass |
| 35 | 0x523230 | DoTargetAction | 0x751ab0 | label_suspect | fits OnDefaultActionFocus |
| 36 | 0x523280 | GetActiveActionIdx | 0x751b00 | label_suspect | fits OnPersonalActionExit |
| 37 | 0x523370 | UpdateActionMenuSelected | 0x751bf0 | label_suspect | fits OnTargetActionFocus |
| 38 | 0x523510 | ShowActionQueue | 0x751d90 | label_suspect | SelectPrevAction; OnTargetUpArrowPressed |
| 39 | 0x5235c0 | OnClearOneButtonPressed | 0x751e40 | label_suspect | SelectNextAction; OnTargetDownArrowPressed |
| 40 | 0x523670 | OnPauseButtonPressed | 0x751ef0 | verified | 5 pause/tutorial callees |
| 41 | 0x523ab0 | SetTarget | 0x752330 | label_suspect | fits OnClearOneButtonPressed |
| 42 | 0x523ae0 | GetActionsFromActionIndex | 0x752360 | label_suspect | fits ClearOneAction |
| 43 | 0x523b30 | UpdateAndDrawFogOfWar | 0x7523b0 | label_suspect | fits OnClearAllButtonPressed |
| 44 | 0x524590 | ShowFlashingStatus | 0x752e10 | label_suspect | tooltip-instant on (+0x732c=1) |
| 45 | 0x524620 | AddFloatyText | 0x752ea0 | label_suspect | tooltip-instant off (+0x732c=0) |
| 46 | 0x524a20 | RemoveFloatyText | 0x7532a0 | label_suspect | floaty entry tick, removes nothing |
| 47 | 0x524ea0 | OnPanelRemoved | 0x753720 | verified | 3x RemovePanel + base OnPanelRemoved |
| 48 | 0x5a4790 | ShowGameMenuChangeButtons | 0x8b0890 | verified | GetInGameGui + ShowChangeButtons chain |
| 49 | 0x51bec0 | Destructor | 0x74a750 | verified | calls dtor sz1551 + free |
| 50 | 0x4df5c0 | InitializeMap | 0x7ceec0 | verified | CALL MainInterface::InitializeMap 0x750c10 |
| 51 | 0x4d8410 | GetMessageBuffer | 0x7c7d20 | likely | getter +0x110 |
| 52 | 0x4dfbb0 | ShowFlashingStatus | 0x7cf4b0 | verified | CALL 0x750e90 (= GOG 0x522610) |
| 53 | 0x4d00b0 | SetLevelUpMode | 0x7bfa20 | likely | setter +0x12c (B0) |
| 54 | 0x5265a0 | ResetPartyMemberItemUse | 0x754e20 | verified | +0x15234=-1/+0x15230=0, 8 shared callers |
| 55 | 0x4db710 | IsFireForgetDialogAnimation | 0x7cb010 (was 0x7cae60) | verified | CALL 0x7cb220 (= GOG callee 0x4db920) |
| 56 | 0x4d8430 | GetMessageBufferSize | 0x7c7d40 | likely | getter +0x11c (B0) |
| 57 | 0x4dee10 | SetMainInterfaceTarget | 0x7ce710 | verified | CALL 0x74cfb0 (= GOG 0x51e730) |
| 58 | 0x523a80 | OnCharacterExited | 0x752300 | likely | +0x15230/+0x15234 reset, vtable slot |
| 59 | 0x4dc340 | IsGlobalFading | 0x7cbc40 | verified | CALL Fade::IsFadeDone 0x7bca60 (B0) |
| 60 | 0x523460 | HandleButtonAction_3 | 0x751ce0 | label_suspect | SelectNextPersonalAction; OnActionDownArrowPressed |

## sample_errors: 2
- **Row 55** (0x4db710, reviewer `match` at 0x7cae60): address error.
  - 0x7cae60 is the fp-identical twin GOG FUN_004db560. It calls 0x7cb1f0, which is GOG IsLoopingDialogStuntAnimation 0x4db8f0.
  - GOG 0x4db710 calls 0x4db920. The Steam counterpart of 0x4db920 is 0x7cb220, called only by 0x7cb010.
  - The reviewer noted that callee caveat and still matched.
- **Row 60** (0x523460, reviewer `match`): the counterpart is right, but the right verdict is `label_suspect`.
  - The Mac 4-arrow-handler set and the two genuine HandleButtonAction bodies (0x51f0b0/0x51f180) contradict this label.
  - It is counted strictly, following the B11a convention.
- The other 10 sampled matches are correct (4 of them are only `likely`: thin getters, a setter, a vtable slot).

AUTO meaning checks: none in part a (rows 1-60 are "100%" + "sample"; the AUTO rows are in a later part).
