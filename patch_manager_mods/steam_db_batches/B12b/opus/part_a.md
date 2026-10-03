# Opus gate B12b part a (1 of 1)

- batch: B12b, part: a, rows: 27 (rows 1-27 of 27 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 1 | 100% | CSWSCreature::UpdateAppearanceForEquippedItems_2 | 0x00698df0 | 0x005802c0 | label_suspect |
| 2 | 100% | CSWSCreature::BroadcastAttackOfOpportunity | 0x0069df00 | 0x00585420 | label_suspect |
| 3 | 100% | CSWSCreature::UpdatePureGoodEvilPowers | 0x006a2600 | 0x00589b20 | match on a mac_contra row |
| 4 | 100% | CSWSCreature::ComputeArmourClass | 0x006a26d0 | 0x00589bf0 | label_suspect |
| 5 | 100% | CSWSCreature::SetInParty | 0x006a2e10 | 0x0058a610 | reviewer corrected_addr |
| 6 | 100% | CSWSCreature::HasEffect | 0x006a3e00 | 0x0058b320 | label_suspect |
| 7 | 100% | CSWSCreature::UseLoreOnItem | 0x006a4d30 | 0x0058c290 | label_suspect |
| 8 | 100% | CSWSCreature::LearnScroll | 0x006a4da0 | 0x0058c300 | label_suspect |
| 9 | 100% | CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest | 0x006d5a90 | 0x005cacd0 | reviewer corrected_addr |
| 10 | 100% | CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_PathWaitDest | 0x006d5bc0 | 0x005cae00 | reviewer corrected_addr |
| 11 | 100% | CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToOffsetWaitDest | 0x006d5cf0 | 0x005caf30 | reviewer corrected_addr |
| 12 | 100% | CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToPathWaitDest | 0x006d5e20 | 0x005cb060 | reviewer corrected_addr |
| 13 | 100% | CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_TeleportForward | 0x006d5f50 | 0x005cb190 | reviewer corrected_addr |
| 14 | 100% | CSWSCreature::GetCreatureReputation_2 | 0x006dc300 | 0x005d16e0 | label_suspect |
| 15 | sample | CSWSCreature::HasAction | 0x006dfcc0 | 0x005d50a0 | C0 |
| 16 | sample | CSWSCreature::AIActionCheckInterAreaPathfinding | 0x006c66e0 | 0x005bb920 | C1 |
| 17 | sample | CSWSCreature::GetVisibleListElement | 0x006dc370 | 0x005d1750 | C1 |
| 18 | sample | CSWSCreature::AdjustReputation | 0x006dba10 | 0x005d0df0 | C2 |
| 19 | sample | CSWSCreature::CalculateDamagePower | 0x00698350 | 0x0057f820 | C1 |
| 20 | AUTO sig | CSWSCreature::AIActionUnequipItem | 0x006db6a0 | 0x005d0a80 |  |
| 21 | AUTO sig | CSWSCreature::UpdatePersonalSpace | 0x00698810 | 0x0057fce0 |  |
| 22 | AUTO sig | CSWSCreature::CutWaypointPath | 0x0069db00 | 0x00585020 |  |
| 23 | AUTO sig | CSWSCreature::CheckItemAlignmentRestrictions | 0x006de0a0 | 0x005d3480 |  |
| 24 | AUTO sig | CSWSCreature::CleanInvitationLists | 0x006df450 | 0x005d4830 |  |
| 25 | AUTO c2 | CSWSCreature::UnsummonMyself | 0x006e0170 | 0x005d5550 |  |
| 26 | AUTO c2 | CSWSCreature::CanEquipItem | 0x006c8460 | 0x005bd6a0 |  |
| 27 | AUTO c2 | CSWSCreature::IsPartyVisible | 0x006a3420 | 0x0058a940 |  |
