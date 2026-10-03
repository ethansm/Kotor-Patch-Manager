# Opus gate B16 part a (1 of 1)

- batch: B16, part: a, rows: 42 (rows 1-42 of 42 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 1 | 100% | CSWGuiInGameCharacter::Constructor | 0x00595eb0 | 0x0073b4d0 | reviewer corrected_addr |
| 2 | 100% | CSWGuiInGameCharacter::Destructor | 0x00597770 | 0x0084de70 | reviewer corrected_addr |
| 3 | 100% | CSWGuiInGameCharacter::Draw | 0x005994b0 | 0x0084fbb0 | match on a mac_contra row |
| 4 | 100% | CSWGuiInGameGameplay::Constructor | 0x005e43c0 | 0x008df930 | match cites a layout/offset difference |
| 5 | 100% | CSWGuiInGameGameplay::OnKeyMap | 0x005e5a70 | 0x008e0fc0 | match cites a layout/offset difference |
| 6 | 100% | CSWGuiInGameGameplay::OnMouse | 0x005e5b30 | 0x008e1080 | match cites a layout/offset difference |
| 7 | 100% | CSWGuiInGameGameplay::OnEnterDiff | 0x005e6020 | 0x008e1570 | match on a mac_contra row |
| 8 | 100% | CSWGuiInGameGameplay::OnExitDiff | 0x005e6080 | 0x008e15d0 | match on a mac_contra row |
| 9 | 100% | CServerAIMaster::AddEventAbsoluteTime | 0x0064dc40 | 0x0051e220 | match cites a layout/offset difference |
| 10 | 100% | CSWGlobalVariableTable::FindIdentifierIndex | 0x00651470 | 0x006544f0 | reviewer corrected_addr |
| 11 | 100% | CSWGlobalVariableTable::GetValueBoolean | 0x00651600 | 0x00654670 | reviewer corrected_addr |
| 12 | 100% | CSWGlobalVariableTable::GetValueNumber | 0x00651720 | 0x00654670 | reviewer corrected_addr |
| 13 | 100% | CSWGlobalVariableTable::SetValueBoolean | 0x00651a70 | 0x00654aa0 | reviewer corrected_addr |
| 14 | 100% | CSWGlobalVariableTable::ReadTableWithoutCatalogue | 0x00652550 | 0x00655690 | match cites a layout/offset difference |
| 15 | 100% | CSWGlobalVariableTable::Save | 0x00653500 | 0x006564c0 | reviewer corrected_addr |
| 16 | 100% | CSWGlobalVariableTable::Load | 0x006535d0 | 0x00656780 | reviewer corrected_addr |
| 17 | 100% | CServerAIMaster::AppendToScriptsRun | 0x0077e650 | 0x00669070 | reviewer corrected_addr |
| 18 | 100% | CAurTextureBasic::GetController | 0x00815aa0 | 0x004233a0 | match cites a layout/offset difference |
| 19 | 100% | CAurTextureBasic::Destructor | 0x00815b80 | 0x004233a0 | reviewer corrected_addr |
| 20 | 100% | CAurTextureBasic::RemoveFromOrderedLists | 0x00816030 | 0x00423820 | reviewer corrected_addr |
| 21 | 100% | CExoMoviePlayerInternal::StopMovie | 0x00920c10 | 0x0040dac0 | reviewer corrected_addr |
| 22 | 100% | CExoMoviePlayerInternal::PauseMovie | 0x00921000 | 0x0040de50 | reviewer corrected_addr |
| 23 | sample | CSWGuiTextParams::SetBaseFont | 0x009151b0 | 0x00416de0 | C0 |
| 24 | sample | CExoMoviePlayerInternal::ResumeMovie | 0x00921040 | 0x0040de50 | C0 |
| 25 | sample | CSWGuiTextParams::ReSetFont | 0x009151e0 | 0x00416e10 | C0 |
| 26 | sample | CSWGuiTextParams::SetDefaultColor_2 | 0x00915550 | 0x00417180 | C0 |
| 27 | sample | CSWGuiInGameCharacter::SetLabelBonusPenaltyColour | 0x00599990 | 0x00850090 | C0 |
| 28 | sample | CSWGuiDialogLetterbox::`scalar | 0x005af1f0 | 0x008baaa0 | C0 |
| 29 | sample | CSWGuiInGameGameplay::Destructor | 0x005e5210 | 0x008e0780 | C0 |
| 30 | sample | CSWGuiInGameGameplay::OnEnterSlot | 0x005e59f0 | 0x008e0f40 | C1 |
| 31 | sample | CServerAIMaster::SaveEventQueue | 0x0064e840 | 0x0051ee20 | C2 |
| 32 | sample | CSWGuiInGameCharacter::SetModText | 0x00599070 | 0x0084f770 | C1 |
| 33 | sample | CServerAIMaster::AdjustTargetAndWitnessReputations | 0x0064eac0 | 0x0051f0a0 | C1 |
| 34 | sample | CSWGlobalVariableTable::ClearValues | 0x00650c90 | 0x00653dd0 | C1 |
| 35 | AUTO sig | CServerAIMaster::OnEffectRemoved | 0x0064ea20 | 0x0051f000 |  |
| 36 | AUTO sig | CSWGuiTextParams::SetTextObject | 0x00915440 | 0x00417070 |  |
| 37 | AUTO sig | CServerAIMaster::OnEffectApplied | 0x0064e9f0 | 0x0051efd0 |  |
| 38 | AUTO sig | CAurTextureBasic::glImage | 0x00818ce0 | 0x004265e0 |  |
| 39 | AUTO sig | CSWGlobalVariableTable::AddIdentifierLocation | 0x006513b0 | 0x006544f0 |  |
| 40 | AUTO c2 | CSWGlobalVariableTable::ReadTable | 0x00653380 | 0x006564c0 |  |
| 41 | AUTO c2 | CServerAIMaster::AddObject | 0x0064da60 | 0x0051e040 |  |
| 42 | AUTO c2 | CServerAIMaster::RemoveObject | 0x0064dae0 | 0x0051e0c0 |  |
