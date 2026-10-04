# Opus gate B13a (DESIGN 4.6)

Decision: **OK: Opus reads the 100% classes plus a 20% sample (min 5) of the other reviewer matches**

- decoy cards: 20 (per chunk {'1': 10, '2': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: pending (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (26 rows)

- CSWCCreature::`scalar gog 0x00410e50 -> 0x00765110: reviewer corrected_addr
- CSWCCreature::StripEquippedItems gog 0x00413620 -> 0x007678f0: label_suspect
- CSWCCreature::ClearVisibilityList gog 0x004137c0 -> 0x007679f0: match on a mac_contra row
- CSWCCreature::UpdateRollingFootstepSound gog 0x00413e70 -> 0x007680a0: label_suspect
- CSWCCreature::UpdateRangedAttackData gog 0x0041ac10 -> 0x0076eda0: label_suspect
- CSWCCreature::BarkBubble gog 0x0041ae50 -> 0x0076efe0: label_suspect
- CSWCCreature::SetDesiredOrientationForLock gog 0x0041f5a0 -> 0x007736d0: label_suspect
- CSWCCreature::UnloadMonkVisuals gog 0x0041f6b0 -> 0x007737e0: label_suspect
- CSWCCreature::ResetAnimationState gog 0x0041f730 -> 0x007737e0: reviewer corrected_addr, label_suspect
- CSWCCreature::GetHeadTurnHorizontal gog 0x00420070 -> 0x00774200: reviewer corrected_addr
- CSWCCreature::GetHeadTurnVertical gog 0x004200d0 -> 0x00774260: reviewer corrected_addr
- CSWCCreature::SetThrownLightsaber gog 0x004213c0 -> 0x007754d0: label_suspect
- CSWCCreature::CreateNewSpellLikeAbilityList gog 0x00421650 -> 0x00775760: match cites a layout/offset difference
- CSWCCreature::SetInParty gog 0x00421750 -> 0x00775860: match on a mac_rotation row
- CSWCCreature::GetCameraFollowPart gog 0x00421820 -> 0x007759b0: reviewer corrected_addr
- CSWCCreature::GetFollowingBody gog 0x00421840 -> 0x00775970: reviewer corrected_addr
- CSWCCreature::GetHeadHeightOffset gog 0x00421860 -> 0x00775990: reviewer corrected_addr
- CSWCCreature::GetCameraOffset gog 0x00421880 -> 0x00775950: reviewer corrected_addr
- CSWCCreature::IsDriveAnimationBlocked gog 0x00422dd0 -> 0x00776e10: label_suspect
- CSWCCreature::InCreatureCombat gog 0x00422e00 -> 0x00776e40: match on a mac_contra row
- CSWCCreature::GetEquipmentForcePowers gog 0x004248c0 -> 0x00778900: label_suspect
- CSWCCreature::ActionMenuSwapImplantRegen gog 0x00426430 -> 0x0077a470: reviewer corrected_addr
- CSWCCreature::ActionMenuSwapImplantStr gog 0x00426450 -> 0x0077a490: reviewer corrected_addr
- CSWCCreature::ActionMenuSwapImplantEnd gog 0x00426490 -> 0x0077a4b0: reviewer corrected_addr
- CSWCCreature::IsInjured gog 0x0042bae0 -> 0x0077fac0: label_suspect
- CSWCCreature::SetSWAnimationSpeed gog 0x0042bd00 -> 0x0077fce0: label_suspect

## Sample of the other reviewer matches (11 of 52)

- CSWCCreature::SetEngagedState gog 0x00422e80 -> 0x00776ec0 (C0)
- CSWCCreature::ResetDesiredPositionTimer gog 0x0041f7b0 -> 0x007738e0 (C0)
- CSWCCreature::GetCurrentWayPoint gog 0x00415cf0 -> 0x00769ed0 (C0)
- CSWCCreature::Hilite gog 0x004208e0 -> 0x00774a10 (C0)
- CSWCCreature::ActionMenuSwapImplantAgi gog 0x00426470 -> 0x0077a4b0 (C0)
- CSWCCreature::GetHostileItems gog 0x00428620 -> 0x0077c6a0 (C0)
- CSWCCreature::GetHostileItems_2 gog 0x00428690 -> 0x0077c710 (C0)
- CSWCCreature::UnHilite gog 0x00420920 -> 0x00774a50 (C0)
- CSWCCreature::GetArmourClass gog 0x0041ce10 -> 0x00770fa0 (C0)
- CSWCCreature::DeleteLoopingCreatureSound gog 0x0041cdb0 -> 0x00770f40 (C0)
- CSWCCreature::GetTargetInterfaceTargetType gog 0x00429f10 -> 0x0077df90 (C0)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CSWCCreature::TurnOffShadows gog 0x00420410 -> 0x00774540
- sig: CSWCCreature::GetWayPoints gog 0x00416c70 -> 0x0076ae50
- sig: CSWCCreature::FollowAnimationUpdate gog 0x00423970 -> 0x007779b0
- sig: CSWCCreature::PartyMenuUseItem gog 0x00428c00 -> 0x0077cc80
- sig: CSWCCreature::CheckFeat gog 0x004247a0 -> 0x007787e0
- c2: CSWCCreature::CanSee gog 0x004244d0 -> 0x00778510
- c2: CSWCCreature::PlaySwingSound gog 0x0041dbb0 -> 0x00771d40
- c2: CSWCCreature::GetEquippedItemID gog 0x0041a480 -> 0x0076e610
