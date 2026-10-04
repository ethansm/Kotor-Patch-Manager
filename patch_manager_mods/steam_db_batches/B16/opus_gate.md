# Opus gate B16 (DESIGN 4.6)

Decision: **OK: Opus reads the 100% classes plus a 20% sample (min 5) of the other reviewer matches**

- decoy cards: 20 (per chunk {'1': 10, '2': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 0 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 1 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (26 rows)

- CSWGuiInGameCharacter::Constructor gog 0x00595eb0 -> 0x0084c3a0: reviewer corrected_addr
- CSWGuiInGameCharacter::Destructor gog 0x00597770 -> 0x0084def0: reviewer corrected_addr
- CSWGuiInGameCharacter::Draw gog 0x005994b0 -> 0x0084fbb0: match on a mac_contra row
- CSWGuiInGameGameplay::Constructor gog 0x005e43c0 -> 0x008df930: match cites a layout/offset difference
- CSWGuiInGameGameplay::OnKeyMap gog 0x005e5a70 -> 0x008e0fc0: match cites a layout/offset difference, match on a mac_rotation row
- CSWGuiInGameGameplay::OnMouse gog 0x005e5b30 -> 0x008e1080: match cites a layout/offset difference, match on a mac_rotation row
- CSWGuiInGameGameplay::SetDifficultyText gog 0x005e5d30 -> 0x008e1280: match on a mac_rotation row
- CSWGuiInGameGameplay::OnEnterDiff gog 0x005e6020 -> 0x008e1570: match on a mac_contra row
- CSWGuiInGameGameplay::OnExitDiff gog 0x005e6080 -> 0x008e15d0: match on a mac_contra row
- CServerAIMaster::AddEventAbsoluteTime gog 0x0064dc40 -> 0x0051e220: match cites a layout/offset difference
- CServerAIMaster::SaveEventQueue gog 0x0064e840 -> 0x0051ee20: match on a mac_rotation row
- CSWGlobalVariableTable::FindIdentifierIndex gog 0x00651470 -> 0x006545b0: reviewer corrected_addr
- CSWGlobalVariableTable::GetValueBoolean gog 0x00651600 -> 0x00654740: reviewer corrected_addr
- CSWGlobalVariableTable::GetValueNumber gog 0x00651720 -> 0x00654860: reviewer corrected_addr
- CSWGlobalVariableTable::SetValueBoolean gog 0x00651a70 -> 0x00654bb0: reviewer corrected_addr
- CSWGlobalVariableTable::ReadTableWithoutCatalogue gog 0x00652550 -> 0x00655690: match cites a layout/offset difference
- CSWGlobalVariableTable::Save gog 0x00653500 -> 0x00656640: reviewer corrected_addr
- CSWGlobalVariableTable::Load gog 0x006535d0 -> 0x00656710: reviewer corrected_addr
- CServerAIMaster::AppendToScriptsRun gog 0x0077e650 -> 0x00669070: reviewer corrected_addr
- CAurTextureBasic::GetController gog 0x00815aa0 -> 0x004233a0: match cites a layout/offset difference
- CAurTextureBasic::Destructor gog 0x00815b80 -> 0x00423480: reviewer corrected_addr
- CAurTextureBasic::RemoveFromOrderedLists gog 0x00816030 -> 0x00423930: reviewer corrected_addr
- CExoMoviePlayerInternal::Destructor gog 0x0091fbe0 -> 0x0040c9f0: match on a mac_rotation row
- CExoMoviePlayerInternal::StartMovie gog 0x0091fe60 -> 0x0040cc70: match on a mac_rotation row
- CExoMoviePlayerInternal::StopMovie gog 0x00920c10 -> 0x0040da20: reviewer corrected_addr
- CExoMoviePlayerInternal::PauseMovie gog 0x00921000 -> 0x0040de10: reviewer corrected_addr

## Sample of the other reviewer matches (11 of 53)

- CSWGuiTextParams::SetBaseFont gog 0x009151b0 -> 0x00416de0 (C0)
- CSWGuiTextParams::ReSetFont gog 0x009151e0 -> 0x00416e10 (C0)
- CSWGuiTextParams::SetDefaultColor_2 gog 0x00915550 -> 0x00417180 (C0)
- CExoMoviePlayerInternal::ResumeMovie gog 0x00921040 -> 0x0040de50 (C0)
- CSWGuiInGameCharacter::SetLabelBonusPenaltyColour gog 0x00599990 -> 0x00850090 (C0)
- CSWGuiDialogLetterbox::`scalar gog 0x005af1f0 -> 0x008baaa0 (C0)
- CSWGuiInGameGameplay::Destructor gog 0x005e5210 -> 0x008e0780 (C0)
- CSWGuiInGameGameplay::OnEnterSlot gog 0x005e59f0 -> 0x008e0f40 (C1)
- CServerAIMaster::DeleteEventData gog 0x00650700 -> 0x00520ce0 (C2)
- CSWGuiInGameCharacter::SetModText gog 0x00599070 -> 0x0084f770 (C1)
- CSWGlobalVariableTable::ClearValues gog 0x00650c90 -> 0x00653dd0 (C1)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CAurTextureBasic::SetNewDownSampleLevel gog 0x00819060 -> 0x00426960
- sig: CSWGuiTextParams::SetAlignment gog 0x00915370 -> 0x00416fa0
- sig: CAurTextureBasic::LoadImagePLT gog 0x00817f00 -> 0x00425800
- sig: CSWGuiDialogLetterbox::SetBottom gog 0x005af4f0 -> 0x008bada0
- sig: CAurTextureBasic::GetMemoryUsage gog 0x008184b0 -> 0x00425db0
- c2: CAurTextureBasic::ConstructImage gog 0x00818970 -> 0x00426270
- c2: CServerAIMaster::AddObject gog 0x0064da60 -> 0x0051e040
- c2: CServerAIMaster::RemoveObject gog 0x0064dae0 -> 0x0051e0c0
