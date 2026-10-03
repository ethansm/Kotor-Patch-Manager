# Opus gate B16 (DESIGN 4.6)

Decision: **OK: Opus reads the 100% classes plus a 20% sample (min 5) of the other reviewer matches**

- decoy cards: 20 (per chunk {'1': 10, '2': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: pending (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 1 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (22 rows)

- CSWGuiInGameCharacter::Constructor gog 0x00595eb0 -> 0x0073b4d0: reviewer corrected_addr
- CSWGuiInGameCharacter::Destructor gog 0x00597770 -> 0x0084de70: reviewer corrected_addr
- CSWGuiInGameCharacter::Draw gog 0x005994b0 -> 0x0084fbb0: match on a mac_contra row
- CSWGuiInGameGameplay::Constructor gog 0x005e43c0 -> 0x008df930: match cites a layout/offset difference
- CSWGuiInGameGameplay::OnKeyMap gog 0x005e5a70 -> 0x008e0fc0: match cites a layout/offset difference
- CSWGuiInGameGameplay::OnMouse gog 0x005e5b30 -> 0x008e1080: match cites a layout/offset difference
- CSWGuiInGameGameplay::OnEnterDiff gog 0x005e6020 -> 0x008e1570: match on a mac_contra row
- CSWGuiInGameGameplay::OnExitDiff gog 0x005e6080 -> 0x008e15d0: match on a mac_contra row
- CServerAIMaster::AddEventAbsoluteTime gog 0x0064dc40 -> 0x0051e220: match cites a layout/offset difference
- CSWGlobalVariableTable::FindIdentifierIndex gog 0x00651470 -> 0x006544f0: reviewer corrected_addr
- CSWGlobalVariableTable::GetValueBoolean gog 0x00651600 -> 0x00654670: reviewer corrected_addr
- CSWGlobalVariableTable::GetValueNumber gog 0x00651720 -> 0x00654670: reviewer corrected_addr
- CSWGlobalVariableTable::SetValueBoolean gog 0x00651a70 -> 0x00654aa0: reviewer corrected_addr
- CSWGlobalVariableTable::ReadTableWithoutCatalogue gog 0x00652550 -> 0x00655690: match cites a layout/offset difference
- CSWGlobalVariableTable::Save gog 0x00653500 -> 0x006564c0: reviewer corrected_addr
- CSWGlobalVariableTable::Load gog 0x006535d0 -> 0x00656780: reviewer corrected_addr
- CServerAIMaster::AppendToScriptsRun gog 0x0077e650 -> 0x00669070: reviewer corrected_addr
- CAurTextureBasic::GetController gog 0x00815aa0 -> 0x004233a0: match cites a layout/offset difference
- CAurTextureBasic::Destructor gog 0x00815b80 -> 0x004233a0: reviewer corrected_addr
- CAurTextureBasic::RemoveFromOrderedLists gog 0x00816030 -> 0x00423820: reviewer corrected_addr
- CExoMoviePlayerInternal::StopMovie gog 0x00920c10 -> 0x0040dac0: reviewer corrected_addr
- CExoMoviePlayerInternal::PauseMovie gog 0x00921000 -> 0x0040de50: reviewer corrected_addr

## Sample of the other reviewer matches (12 of 57)

- CSWGuiTextParams::SetBaseFont gog 0x009151b0 -> 0x00416de0 (C0)
- CExoMoviePlayerInternal::ResumeMovie gog 0x00921040 -> 0x0040de50 (C0)
- CSWGuiTextParams::ReSetFont gog 0x009151e0 -> 0x00416e10 (C0)
- CSWGuiTextParams::SetDefaultColor_2 gog 0x00915550 -> 0x00417180 (C0)
- CSWGuiInGameCharacter::SetLabelBonusPenaltyColour gog 0x00599990 -> 0x00850090 (C0)
- CSWGuiDialogLetterbox::`scalar gog 0x005af1f0 -> 0x008baaa0 (C0)
- CSWGuiInGameGameplay::Destructor gog 0x005e5210 -> 0x008e0780 (C0)
- CSWGuiInGameGameplay::OnEnterSlot gog 0x005e59f0 -> 0x008e0f40 (C1)
- CServerAIMaster::SaveEventQueue gog 0x0064e840 -> 0x0051ee20 (C2)
- CSWGuiInGameCharacter::SetModText gog 0x00599070 -> 0x0084f770 (C1)
- CServerAIMaster::AdjustTargetAndWitnessReputations gog 0x0064eac0 -> 0x0051f0a0 (C1)
- CSWGlobalVariableTable::ClearValues gog 0x00650c90 -> 0x00653dd0 (C1)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CServerAIMaster::OnEffectRemoved gog 0x0064ea20 -> 0x0051f000
- sig: CSWGuiTextParams::SetTextObject gog 0x00915440 -> 0x00417070
- sig: CServerAIMaster::OnEffectApplied gog 0x0064e9f0 -> 0x0051efd0
- sig: CAurTextureBasic::glImage gog 0x00818ce0 -> 0x004265e0
- sig: CSWGlobalVariableTable::AddIdentifierLocation gog 0x006513b0 -> 0x006544f0
- c2: CSWGlobalVariableTable::ReadTable gog 0x00653380 -> 0x006564c0
- c2: CServerAIMaster::AddObject gog 0x0064da60 -> 0x0051e040
- c2: CServerAIMaster::RemoveObject gog 0x0064dae0 -> 0x0051e0c0
