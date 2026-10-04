# Opus gate B13a part a (1 of 1)

- batch: B13a, part: a, rows: 45 (rows 1-45 of 45 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 1 | 100% | CSWCCreature::`scalar | 0x00410e50 | 0x00765110 | reviewer corrected_addr |
| 2 | 100% | CSWCCreature::StripEquippedItems | 0x00413620 | 0x007678f0 | label_suspect |
| 3 | 100% | CSWCCreature::ClearVisibilityList | 0x004137c0 | 0x007679f0 | match on a mac_contra row |
| 4 | 100% | CSWCCreature::UpdateRollingFootstepSound | 0x00413e70 | 0x007680a0 | label_suspect |
| 5 | 100% | CSWCCreature::UpdateRangedAttackData | 0x0041ac10 | 0x0076eda0 | label_suspect |
| 6 | 100% | CSWCCreature::BarkBubble | 0x0041ae50 | 0x0076efe0 | label_suspect |
| 7 | 100% | CSWCCreature::SetDesiredOrientationForLock | 0x0041f5a0 | 0x007736d0 | label_suspect |
| 8 | 100% | CSWCCreature::UnloadMonkVisuals | 0x0041f6b0 | 0x007737e0 | label_suspect |
| 9 | 100% | CSWCCreature::ResetAnimationState | 0x0041f730 | 0x007737e0 | reviewer corrected_addr, label_suspect |
| 10 | 100% | CSWCCreature::GetHeadTurnHorizontal | 0x00420070 | 0x00774200 | reviewer corrected_addr |
| 11 | 100% | CSWCCreature::GetHeadTurnVertical | 0x004200d0 | 0x00774260 | reviewer corrected_addr |
| 12 | 100% | CSWCCreature::SetThrownLightsaber | 0x004213c0 | 0x007754d0 | label_suspect |
| 13 | 100% | CSWCCreature::CreateNewSpellLikeAbilityList | 0x00421650 | 0x00775760 | match cites a layout/offset difference |
| 14 | 100% | CSWCCreature::SetInParty | 0x00421750 | 0x00775860 | match on a mac_rotation row |
| 15 | 100% | CSWCCreature::GetCameraFollowPart | 0x00421820 | 0x007759b0 | reviewer corrected_addr |
| 16 | 100% | CSWCCreature::GetFollowingBody | 0x00421840 | 0x00775970 | reviewer corrected_addr |
| 17 | 100% | CSWCCreature::GetHeadHeightOffset | 0x00421860 | 0x00775990 | reviewer corrected_addr |
| 18 | 100% | CSWCCreature::GetCameraOffset | 0x00421880 | 0x00775950 | reviewer corrected_addr |
| 19 | 100% | CSWCCreature::IsDriveAnimationBlocked | 0x00422dd0 | 0x00776e10 | label_suspect |
| 20 | 100% | CSWCCreature::InCreatureCombat | 0x00422e00 | 0x00776e40 | match on a mac_contra row |
| 21 | 100% | CSWCCreature::GetEquipmentForcePowers | 0x004248c0 | 0x00778900 | label_suspect |
| 22 | 100% | CSWCCreature::ActionMenuSwapImplantRegen | 0x00426430 | 0x0077a470 | reviewer corrected_addr |
| 23 | 100% | CSWCCreature::ActionMenuSwapImplantStr | 0x00426450 | 0x0077a490 | reviewer corrected_addr |
| 24 | 100% | CSWCCreature::ActionMenuSwapImplantEnd | 0x00426490 | 0x0077a4b0 | reviewer corrected_addr |
| 25 | 100% | CSWCCreature::IsInjured | 0x0042bae0 | 0x0077fac0 | label_suspect |
| 26 | 100% | CSWCCreature::SetSWAnimationSpeed | 0x0042bd00 | 0x0077fce0 | label_suspect |
| 27 | sample | CSWCCreature::SetEngagedState | 0x00422e80 | 0x00776ec0 | C0 |
| 28 | sample | CSWCCreature::ResetDesiredPositionTimer | 0x0041f7b0 | 0x007738e0 | C0 |
| 29 | sample | CSWCCreature::GetCurrentWayPoint | 0x00415cf0 | 0x00769ed0 | C0 |
| 30 | sample | CSWCCreature::Hilite | 0x004208e0 | 0x00774a10 | C0 |
| 31 | sample | CSWCCreature::ActionMenuSwapImplantAgi | 0x00426470 | 0x0077a4b0 | C0 |
| 32 | sample | CSWCCreature::GetHostileItems | 0x00428620 | 0x0077c6a0 | C0 |
| 33 | sample | CSWCCreature::GetHostileItems_2 | 0x00428690 | 0x0077c710 | C0 |
| 34 | sample | CSWCCreature::UnHilite | 0x00420920 | 0x00774a50 | C0 |
| 35 | sample | CSWCCreature::GetArmourClass | 0x0041ce10 | 0x00770fa0 | C0 |
| 36 | sample | CSWCCreature::DeleteLoopingCreatureSound | 0x0041cdb0 | 0x00770f40 | C0 |
| 37 | sample | CSWCCreature::GetTargetInterfaceTargetType | 0x00429f10 | 0x0077df90 | C0 |
| 38 | AUTO sig | CSWCCreature::TurnOffShadows | 0x00420410 | 0x00774540 |  |
| 39 | AUTO sig | CSWCCreature::GetWayPoints | 0x00416c70 | 0x0076ae50 |  |
| 40 | AUTO sig | CSWCCreature::FollowAnimationUpdate | 0x00423970 | 0x007779b0 |  |
| 41 | AUTO sig | CSWCCreature::PartyMenuUseItem | 0x00428c00 | 0x0077cc80 |  |
| 42 | AUTO sig | CSWCCreature::CheckFeat | 0x004247a0 | 0x007787e0 |  |
| 43 | AUTO c2 | CSWCCreature::CanSee | 0x004244d0 | 0x00778510 |  |
| 44 | AUTO c2 | CSWCCreature::PlaySwingSound | 0x0041dbb0 | 0x00771d40 |  |
| 45 | AUTO c2 | CSWCCreature::GetEquippedItemID | 0x0041a480 | 0x0076e610 |  |
