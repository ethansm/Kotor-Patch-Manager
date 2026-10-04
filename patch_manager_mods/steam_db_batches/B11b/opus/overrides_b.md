# B11b Opus gate, part b (rows 61-120, all section `sample`)

Gate: Opus. Inputs: evidence.tsv, review/sonnet_c1..c3.tsv, cards/key.tsv, the GOG/Steam/Mac fp.tsv files, and the Steam asm dumps.

Method, per row:
1. Compare the GOG fp with the Steam fp: size, retn, n_insn, disp, imm.
2. Compare the callee multiset by (size, retn), and count deep2 hits in Steam (own size+retn plus the sorted callee sigs).
3. Count loose-sig hits in Steam (retn + callee sigs + disp + imm).
4. Compare the caller multiset by (size, retn), then quote asm anchors from the Steam dump.
5. Check name fit against the Mac x86 symbol of the same name (shape and constants only).

For forwarders, the anchor is the identity of the forwarded callee. For symmetric pairs, both sides get a distinguishing anchor.

Counts: verified 60, likely 0, label_suspect 0, rejected 0, unresolved 0, corrections 0.

## Results

- 43 rows have a unique deep2 hit in Steam, and that hit is the pick.
- 12 rows have no unique deep2 hit only because of no-callee or sibling twins. Their loose sig (disp/imm) is unique and is the pick: 67, 68, 76, 80, 81, 83, 95, 100, 105, 109, 117, 120.
- 5 rows have 0 deep2 hits. Each was explained by hand:
  - 98: one small helper differs in size.
  - 102: +2 B, same n_insn.
  - 112 and 119: they call 102.
  - 116: GOG fragment, explained below.

### Symmetric pairs (both sides anchored)

| Pair | Distinguishing anchor |
|---|---|
| HideUpgrade / HideChemical | 0x88 vs 0x8c, plus distinct callers. Mac agrees. |
| HidePFSDebug / HideLoadModuleDebug | 0x74 vs 0x70, plus distinct debug-menu callers. Mac agrees. |
| Prev / NextSWInGameGui | Prev writes 7, Next compares with 8. Callers are OnShoulderLeft and OnShoulderRight. |
| IsLoopingDialogStunt / Range1000_1727 | Ranges start at 0x578 vs 0x3e8. |
| GetCanHeadFollowInDialog vs twin 0x7cf710 | 0xf28 vs 0xf24. The twin is GOG 0x4dfe10. |
| TutorialReasonHasShown / SetTutorialReasonTracked | RET 4 test vs RET 8 set. |

### Explained differences

- **116 HideGalaxyMapGui.** The GOG body (224 B, no RET) falls into the 15 B fragment 0x4db100, labelled "ShowDebugMenu", which calls SetSoundMode. Steam 0x7ca920 is 239 B, which is 224 + 15. The two functions share the same start, so this is not a mid-function address. The fragment 0x4db100 must never ship on its own.
- **78 AppendToCombatBuffer.** The GOG caller AddFeedbackMessage (30173 B) maps to Steam 0x78f160. Ghidra sized that function at 496 B. Its start is region-consistent (+0x353a50), and its caller is the 0x73f100 forwarder.
- **119 SetCombatMessage.** The GOG caller "SetLeader" at 0x478eee is unaligned. It is a fragment inside Steam 0x7e5df0 (2168 B).

### Label rotations seen in forwarders (rows outside part b)

Mac forwarders show that two GOG labels are on the wrong functions:
- CGuiInGame::AddFloatyText forwards to CSWGuiMainInterface::AddFloatyText, which is GOG FUN_005226b0 (Steam 0x750f30). The GOG label "AddFloatyText" at 0x524620 is on the wrong function.
- CGuiInGame::ChangeCharacter forwards to MainInterface::StartRotation, which is GOG FUN_005225e0 (Steam 0x750e60). The GOG label "StartRotation" at 0x517fd0 is on the wrong function.

## Per-row evidence

| # | GOG | Steam (final) | verdict | evidence |
|---|---|---|---|---|
| 61 | 0x004e09d0 CGuiInGame::SetupPauseGuiExtent | 0x007d02d0 | verified | 007d02e4 CALL 0x752f50 (=MainInterface::SetupPauseGuiExtent, row 93) via 007d02de MOV ECX,[ECX+0x98]; RET 4; Mac forwards same; caller OnPanelAdded = GOG |
| 62 | 0x005237d0 CSWGuiMainInterface::OnCharacterClicked | 0x00752050 | verified | 00752119 CALL 0x77c780 (=PartyMenuUseItemSelect sz1271); 007521ad CALL 0x751580 (=OnMenuButtonPressed sz294); 00752107 OR EDX,0x40000000; fp = GOG, deep2 unique; callers = GOG |
| 63 | 0x00522ed0 CSWGuiMainInterface::DoPersonalAction | 0x00751750 | verified | 007519e7 CALL 0x541080 (=ClearAllActions sz779); 00751986 CALL 0x7cbdb0 (=SetPaused sz352); 00751a64 MOV [EAX + 0x74],0xbb40; fp = GOG, deep2 unique; callers = GOG |
| 64 | 0x0051f070 CSWGuiMainInterface::HandleButtonActionX | 0x0074d8f0 | verified | 0074d8fc CALL 0x74d8c0 (=FUN_0051f040 sz38); 0074d922 CALL 0x740350 (=SetPausedByCombat sz37); fp = GOG, deep2 unique; callers = GOG |
| 65 | 0x0051f400 CSWGuiMainInterface::UpdateActionQueue_2 | 0x0074dc80 | verified | 0074de6c CALL 0x74e0b0 (=GetActionIcon sz1667); 0074de89 CALL 0x74e7f0 (=ShowActionIcon sz286); 0074e045 LEA EDX,[ECX + EAX*0x1 + 0x5f2c]; fp = GOG, deep2 unique; callers = GOG |
| 66 | 0x00518840 CSWGuiMainInterface::Constructor | 0x00747210 | verified | 007499db CALL 0x753d60 (=LayoutGamepadMenu); 007491cd MOV [EDX+0x28],0x1e581; sz13027 r4 n2707 = GOG (no layout shift); deep2 unique |
| 67 | 0x004de2e0 CGuiInGame::HideUpgradeScreen | 0x007cdbe0 | verified | 007cdc09 CMP [EDX+0x88],0 (sibling HideChemical 0x7cdcc0 uses 0x8c); 007cdc38 CALL 0x87a730 (=TogglePauseRequest); callers = CloseUpgradeScreen 0x8c7300/HandleInputEvent 0x8c7330 |
| 68 | 0x004dfff0 CGuiInGame::GetCanHeadFollowInDialog | 0x007cf8f0 | verified | 007cf905 ADD ECX,0xf28 (twin 0x7cf710 = GOG 0x4dfe10 uses 0xf24); 007cf91c CALL 0x521060 (=GetFirstPlayer); caller UpdateShotCamera 0x7c41f0; Mac 0xf28 same |
| 69 | 0x00524790 CSWGuiMainInterface::ResetFont | 0x00753010 | verified | 0075301a CALL 0x40ffb0 (=ResetFont sz99); 00753028 CALL 0x744ac0 (=~CSWGuiTargetActionMenu sz89); 00753022 ADD ECX,0xcc; fp = GOG, deep2 unique; callers = GOG |
| 70 | 0x004df250 CGuiInGame::RePopulateMainInterface | 0x007ceb50 | verified | 007ceb6e CALL 0x74cfe0 (=MainInterface::PopulateMenus, row 79); 007ceb5a CMP [EAX+0x98],0; Mac forwards same; deep2 unique |
| 71 | 0x00523bd0 CSWGuiMainInterface::SelectNextPersonalAction | 0x00752450 | verified | 007525b7 CALL 0x4122a0 (=PlayGuiSound sz94); 00752496 CALL 0x73fb90 (=GetSWParty sz31); 007524b2 CMP [EAX + EDX*0x4 + 0x1c7c],-0x1; fp = GOG, deep2 unique; callers = GOG |
| 72 | 0x0051c510 CSWGuiMainInterface::Destructor_2 | 0x0074ad90 | verified | 0074b363 CALL 0x74a780 (=~CSWGuiTargetActionMenu sz389); 0074b314 CALL 0x74aac0 (=~CSWGuiMainInterfaceChar sz334); 0074ae0e ADD ECX,0x1523c; fp = GOG, deep2 unique; callers = GOG |
| 73 | 0x0051ff70 CSWGuiMainInterface::ShowActionIcon | 0x0074e7f0 | verified | 0074e844 CALL 0x414840 (=SetFillImage sz219); 0074e807 CALL 0x710ad0 (=operator!= sz110); 0074e82e LEA EAX,[EDX + ECX*0x1 + 0x5f8c]; fp = GOG, deep2 unique; callers = GOG |
| 74 | 0x00522e30 CSWGuiMainInterface::OnCharacterRightClicked | 0x007516b0 | verified | 007516e6 CALL 0x751750 (=DoPersonalAction, row 63); 007516cb MOV EAX,[EDX+0x5c]; fp = GOG, deep2 unique; Mac OnCharacterRightClicked is an empty stub (platform diff) |
| 75 | 0x004d01a0 CGuiInGame::ShowDialogEntry | 0x007bfb10 | verified | 007bfc95 CALL 0x7c9c40 (=SetSWGuiStatus sz411); 007bfc44 CALL 0x8bada0 (=SetBottom sz367); 007bfb1c CMP [EAX + 0x134],0x1; fp = GOG, deep2 unique; callers = GOG |
| 76 | 0x00523a40 CSWGuiMainInterface::OnCharacterEntered | 0x007522c0 | verified | 007522e4 MOV EAX,[EDX+0x5c]; 007522f3 MOV [EDX+0x15234],ECX; sz63 r4 disp/imm = GOG, loose-sig unique; Mac OnCharacterEntered reads 0x5c |
| 77 | 0x004e0b70 CGuiInGame::ClearDialogNonDependantList | 0x007d0470 | verified | 007d04b1 CALL 0x733780 (=~CExoString sz60); 007d0482 CALL 0x7a1780 (=IsEmpty sz32); 007d047c ADD ECX,0xf2c; fp = GOG, deep2 unique; callers = GOG |
| 78 | 0x004ce840 CGuiInGame::AppendToCombatBuffer | 0x007be1b0 | verified | 007be240 CALL 0x7337c0 (=CExoString::op=); 007be1d1 CMP [EAX+0x124],0x40; fp=GOG, deep2 unique; caller 0x78f160 = AddFeedbackMessage start (Ghidra-sized 496 vs GOG 30173) |
| 79 | 0x0051e760 CSWGuiMainInterface::PopulateMenus | 0x0074cfe0 | verified | 0074d19f CALL 0x743540 (=PopulateMenus sz782); 0074d3f4 CALL 0x74cc90 (=UpdateActionDescription sz613); 0074d247 LEA ECX,[ECX + EAX*0x1 + 0x733c]; fp = GOG, deep2 unique; callers = GOG |
| 80 | 0x004e11f0 CGuiInGame::SetGlobalUnpauseableState | 0x007d0af0 | verified | 007d0b00 MOV ECX,[EAX+0xf0] inc/dec lock; sz75 r4 no callees; 16 callers = GOG (MainLoop/Initialize/DisplayMainMenu...), loose-sig unique |
| 81 | 0x004dbc40 CGuiInGame::HidePowersFeatsSkillsDebugMenu | 0x007cb540 | verified | 007cb558 MOV EDX,[ECX+0x74] (sibling HideLoadModule 0x7cb4a0 uses 0x70); 007cb571 CALL 0x4109c0 (=PopModalPanel); callers PFS-debug Back 0x8bff20/ReturnToMainAfterPick 0x8c0c80 |
| 82 | 0x00518020 CSWGuiMainInterface::HandleInputEvent | 0x007469f0 | verified | 00746bba CALL 0x5b7ae0 (=SetCombatMode sz1302); 00746eff CALL 0x74d520 (=PopupActionMenu sz927); 00746bd2 FSTP float ptr [EDX + 0x595c]; fp = GOG, deep2 unique; callers = GOG |
| 83 | 0x004daac0 CGuiInGame::PrevSWInGameGui | 0x007ca3c0 | verified | 007ca461 MOV [EDX+0x2c],0x7 (wrap, Next 0x7ca230 CMPs 0x8); 007ca494 CALL 0x7d0760 (=UpdateCreatedInGameGUI); caller 0x7568b0 = OnShoulderLeft; Mac imm 0x7 same |
| 84 | 0x00522930 CSWGuiMainInterface::ClearOneAction | 0x007511b0 | verified | 0075121e CALL 0x541080 (=ClearAllActions sz779); 0075120b CALL 0x5904c0 (=RemoveLastAction sz147); 007511ed MOV EAX,[EDX + 0x10dc]; fp = GOG, deep2 unique; callers = GOG |
| 85 | 0x0051f1c0 CSWGuiMainInterface::RevertActionBarStates | 0x0074da40 | verified | 0074dc43 CALL 0x74cfe0 (=PopulateMenus sz1102); 0074db22 CALL 0x654860 (=GetValueNumber sz267); 0074dc38 MOV [EAX + 0x1c90],ECX; fp = GOG, deep2 unique; callers = GOG |
| 86 | 0x00523ee0 CSWGuiMainInterface::UpdateStickyTarget | 0x00752760 | verified | 00752bca CALL 0x5e13c0 (=GetReputation sz695); 00752d85 CALL 0x414840 (=SetFillImage sz219); 00752be1 ADD ECX,0xca18; fp = GOG, deep2 unique; callers = GOG |
| 87 | 0x004e0ae0 CGuiInGame::IsOnDialogNonDependantList | 0x007d03e0 | verified | 007d03f1 CALL 0x734f60 (=CompareNoCase sz81); 007d0407 CALL 0x521060 (=GetFirstPlayer sz52); 007d0401 ADD ECX,0xf2c; fp = GOG, deep2 unique; callers = GOG |
| 88 | 0x00526570 CSWGuiMainInterface::OnCancelPartyItemSelect | 0x00754df0 | verified | 00754e11 CALL 0x754e20 (=ResetPartyMemberItemUse sz37); 00754e02 MOV ECX,[EAX + 0x50]; fp = GOG, deep2 unique; callers = GOG |
| 89 | 0x0051cb70 CSWGuiMainInterface::Draw | 0x0074b3f0 | verified | 0074b520 CALL 0x74fb20 (=UpdateIndicator sz3146); 0074b4ee CALL 0x74ee90 (=UpdatePortraits sz2732); 0074b6cf PUSH 0x1e621; fp = GOG, deep2 unique; callers = GOG |
| 90 | 0x00517df0 CSWGuiMainInterface::HideHelp | 0x007467c0 | verified | 0074685c CALL 0x745e60 (=AddHelpPanel sz1955); 007467e2 CALL 0x410670 (=RemovePanel sz266); 00746804 ADD EDX,0x15060; fp = GOG, deep2 unique; callers = GOG |
| 91 | 0x004df580 CGuiInGame::AddFloatyText | 0x007cee80 | verified | 007ceeaf CALL 0x750f30 (=GOG 0x5226b0, Mac MainInterface::AddFloatyText) via +0x98; RET 0x10 = GOG. GOG label at 0x524620 is NOT AddFloatyText (rotation) |
| 92 | 0x005211d0 CSWGuiMainInterface::UpdateGameMenu | 0x0074fa50 | verified | 0074fa6e CALL 0x5f7a30 (=GetPartyItems sz170); 0074faa4 CALL 0x4149f0 (=SetPulsingAlpha sz91); 0074fae1 MOV ECX,[EAX + 0xc400]; fp = GOG, deep2 unique; callers = GOG |
| 93 | 0x005246d0 CSWGuiMainInterface::SetupPauseGuiExtent | 0x00752f50 | verified | 00752f59 CALL 0x479ab0 (=GetResolutionTextScalar sz92); 00752f84 CALL 0x72fb00 (=GetClientOptions sz28); 00752fba MOV ECX,[EAX + 0xba5c]; fp = GOG, deep2 unique; callers = GOG |
| 94 | 0x0051e410 CSWGuiMainInterface::UpdateActionDescription | 0x0074cc90 | verified | 0074ceb6 CALL 0x734270 (=Format sz380); 0074cd41 CALL 0x7337c0 (=operator= sz267); 0074ced0 FSTP float ptr [ECX + 0xa784]; fp = GOG, deep2 unique; callers = GOG |
| 95 | 0x004e0110 CGuiInGame::TutorialReasonHasShown | 0x007cfa10 | verified | 007cfa4f MOVZX EAX,[ECX+EAX+0xf88] bit test, RET 4 (setter 0x7cfa70 is RET 8); callers TutorialReasonWillShowWindow/ShowTutorialWindow = GOG; loose-sig unique |
| 96 | 0x00523760 CSWGuiMainInterface::OnStealthButtonPressed | 0x00751fe0 | verified | 0075200e CALL 0x77fe50 (=IsStealthCapable sz501); 00752038 CALL 0x77b020 (=PartyMenuStealth sz148); 00751fec MOV ECX,[EAX + 0x50]; fp = GOG, deep2 unique; callers = GOG |
| 97 | 0x004e09b0 CGuiInGame::ChangeCharacter | 0x007d02b0 | verified | 007d02c0 CALL 0x750e60 (=GOG 0x5225e0 sz43, Mac MainInterface::StartRotation) via +0x98; deep2 unique. GOG label StartRotation@0x517fd0 suspect (real = 0x5225e0) |
| 98 | 0x00517490 CSWGuiMainInterface::AddHelpPanel | 0x00745e60 | verified | 007465ee CALL 0x410530 (=AddPanel); 0074646e CALL 0x414840 (=SetFillImage); sz1955 r8 disp/imm = GOG; only diff: callee 0x407260 sz71 vs GOG sz10 (helper); callers HideHelp = GOG |
| 99 | 0x0051eca0 CSWGuiMainInterface::PopupActionMenu | 0x0074d520 | verified | 0074d77a CALL 0x734270 (=Format sz380); 0074d5ff CALL 0x410530 (=AddPanel sz317); 0074d5d6 ADD EDX,0xffb8; fp = GOG, deep2 unique; callers = GOG |
| 100 | 0x004db8f0 CGuiInGame::IsLoopingDialogStuntAnimation | 0x007cb1f0 | verified | 007cb1fb CMP EAX,0x578 / 007cb206 CMP ECX,0x6bf (range 1400-1727; sibling 0x7cb1c0 uses 0x3e8); callers 0x7cae60 + 0x7f67f0 SetAnimatedCamera = GOG |
| 101 | 0x0051f830 CSWGuiMainInterface::GetActionIcon | 0x0074e0b0 | verified | 0074e65e CALL 0x91dba0 (=memcpy sz711); 0074e265 CALL 0x615cc0 (=GetIcon sz145); 0074e0b3 SUB ESP,0x314; fp = GOG, deep2 unique; callers = GOG |
| 102 | 0x00520390 CSWGuiMainInterface::SetCombatMessage | 0x0074ec10 | verified | 0074ecdc CALL 0x740960 (=GetAutoPaused); 0074eca7 CMP [EBP-0x30],0x1b2c7; sz627 vs GOG 625 (n_insn 150 equal), disp/imm/callees = GOG, loose-sig unique; callers = GOG |
| 103 | 0x00523720 CSWGuiMainInterface::OnSoloButtonPressed | 0x00751fa0 | verified | 00751fd0 CALL 0x7cd510 (=ShowSoloModeQuery sz578); 00751fc9 CALL 0x73f750 (=GetInGameGui sz28); 00751fac MOV ECX,[EAX + 0x50]; fp = GOG, deep2 unique; callers = GOG |
| 104 | 0x0051cb20 CSWGuiMainInterface::HitCheckMouse | 0x0074b3a0 | verified | 0074b3ba CALL 0x7433b0 (=HitCheckMouse sz388); 0074b3d3 CALL 0x40eae0 (=HitCheckMouse sz287); 0074b3b4 ADD ECX,0xcc; fp = GOG, deep2 unique; callers = GOG |
| 105 | 0x004db8c0 CGuiInGame::IsDialogAnimationRange1000_1727 | 0x007cb1c0 | verified | 007cb1cb CMP EAX,0x3e8 / 007cb1d6 CMP ECX,0x6bf (range 1000-1727, name fits; sibling 0x7cb1f0 uses 0x578); caller 0x7c3f50 = GOG SetAnimatedCamera 0x4d45e0 |
| 106 | 0x00524e00 CSWGuiMainInterface::PopupGamepadMenu | 0x00753680 | verified | 0075370e CALL 0x753850 (=InitGamepadMenu sz1007); 007536e9 CALL 0x410530 (=AddPanel sz317); 007536bf ADD ECX,0xd588; fp = GOG, deep2 unique; callers = GOG |
| 107 | 0x00526530 CSWGuiMainInterface::OnSwapWeaponsPressed | 0x00754db0 | verified | 00754ddc CALL 0x77da50 (=ActionMenuSwitchWeapons sz179); 00754dc7 CALL 0x73f450 (=GetPlayerCreature sz22); fp = GOG, deep2 unique; callers = GOG |
| 108 | 0x004e0170 CGuiInGame::SetTutorialReasonTracked | 0x007cfa70 | verified | 007cfabe MOVZX EAX,[ECX+EAX+0xf88]; 007cfab9 SHL EDX,CL bit set, RET 8 (test 0x7cfa10 RET 4); fp = GOG, deep2 unique |
| 109 | 0x004da930 CGuiInGame::NextSWInGameGui | 0x007ca230 | verified | 007ca2c8 CMP [ECX+0x2c],0x8 (Prev 0x7ca3c0 writes 0x7); 007ca304 CALL 0x7d0760 (=UpdateCreatedInGameGUI); caller 0x7568f0 = OnShoulderRight |
| 110 | 0x00522390 CSWGuiMainInterface::InitializeMap | 0x00750c10 | verified | 00750c91 CALL 0x734270 (=Format sz380); 00750ce7 CALL 0x414840 (=SetFillImage sz219); 00750d17 FIMUL [ECX + 0x5d58]; fp = GOG, deep2 unique; callers = GOG |
| 111 | 0x0051daa0 CSWGuiMainInterface::Update | 0x0074c320 | verified | 0074c69c CALL 0x7c9df0 (=ShowSWInGameGui sz622); 0074c91b CALL 0x77fe50 (=IsStealthCapable sz501); 0074c7df PUSH 0x1b2c7; fp = GOG, deep2 unique; callers = GOG |
| 112 | 0x00520610 CSWGuiMainInterface::UpdatePortraits | 0x0074ee90 | verified | 0074f1f5 CALL 0x57eca0 (=GetMaxForcePoints); 0074f47a CALL 0x6bedf0 (=AutoLevelup); sz2732 = GOG; 22/23 callees by sig, only diff SetCombatMessage 627 vs 625 (row 102) |
| 113 | 0x005212a0 CSWGuiMainInterface::UpdateIndicator | 0x0074fb20 | verified | 00750755 CALL 0x752760 (=UpdateStickyTarget sz1699); 007500b2 CALL 0x5e13c0 (=GetReputation sz695); 00750230 ADD EDX,0xca18; fp = GOG, deep2 unique; callers = GOG |
| 114 | 0x0051f180 CSWGuiMainInterface::HandleButtonAction_2 | 0x0074da00 | verified | 0074da29 CALL 0x74d930 (=HandleButtonAction sz203); 0074da18 MOV EAX,[EDX + 0x5c]; fp = GOG, deep2 unique; callers = GOG |
| 115 | 0x00521f10 CSWGuiMainInterface::DrawMap | 0x00750790 | verified | 00750afe CALL 0x479460 (=AurGUISetupViewport sz845); 00750b38 CALL 0x751230 (=FUN_005229b0 sz842); 007507cf ADD ECX,0x5d5c; fp = GOG, deep2 unique; callers = GOG |
| 116 | 0x004db020 CGuiInGame::HideGalaxyMapGui | 0x007ca920 | verified | 007ca971 CALL 0x7c9c40 (=SetSWGuiStatus); 007ca99e CALL 0x410670 (=RemovePanel); Steam 239 = GOG 224 + 15 B tail frag 0x4db100 (SetSoundMode); same start, not mid-fn |
| 117 | 0x004de3c0 CGuiInGame::HideChemicalUpgradeScreen | 0x007cdcc0 | verified | 007cdce9 CMP [EDX+0x8c],0 (sibling HideUpgrade 0x7cdbe0 uses 0x88); 007cdd18 CALL 0x87a730 (=TogglePauseRequest); caller 0x8d94d0 sz41 = GOG 0x5c98a0; Mac same 0x8c |
| 118 | 0x00523b60 CSWGuiMainInterface::OnClearAllButtonPressed | 0x007523e0 | verified | 00752411 CALL 0x77fd20 (=SetCombatMode sz110); 0075242c CALL 0x4122a0 (=PlayGuiSound sz94); 0075243a FSTP float ptr [ECX + 0x595c]; fp = GOG, deep2 unique; callers = GOG |
| 119 | 0x004e0c30 CGuiInGame::SetCombatMessage | 0x007d0530 | verified | 007d0550 CALL 0x74ec10 (=MainInterface::SetCombatMessage, row 102) via +0x98 forwarder; callers UpdateCombatMode 0x79d840, 0x5cb430 = GOG; Mac forwards same |
| 120 | 0x004dbba0 CGuiInGame::HideLoadModuleDebugMenu | 0x007cb4a0 | verified | 007cb4b8 MOV EDX,[ECX+0x70] (sibling HidePFS 0x7cb540 uses 0x74); 007cb4d1 CALL 0x4109c0 (=PopModalPanel); callers LoadModule Back 0x8c18e0/OnModulePicked 0x8c1930 |

## Sample check

All 60 sampled reviewer `match` rows were confirmed.
- The decoy cards on rows 79, 81, 84, 94 and 97 (decoy_prev/decoy_next) were correctly rejected by their reviewers, who corrected to the pick.
- No reviewer match in this part was found wrong.

sample_errors: 0

AUTO meaning checks: none in part b (they are in part c).
