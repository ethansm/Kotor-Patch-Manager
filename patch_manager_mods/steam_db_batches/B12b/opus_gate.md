# Opus gate B12b (DESIGN 4.6)

Decision: **ESCALATE: Opus-found error in the sample -> send 100% of this batch's reviewer matches to Opus**

- decoy cards: 10 (per chunk {'1': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 1 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (24 rows)

- CSWSCreature::UpdateAppearanceForEquippedItems_2 gog 0x00698df0 -> 0x005802c0: label_suspect
- CSWSCreature::BroadcastAttackOfOpportunity gog 0x0069df00 -> 0x00585420: label_suspect
- CSWSCreature::UpdatePureGoodEvilPowers gog 0x006a2600 -> 0x00589b20: label_suspect, match on a mac_contra row
- CSWSCreature::ComputeArmourClass gog 0x006a26d0 -> 0x00589bf0: label_suspect
- CSWSCreature::SetInParty gog 0x006a2e10 -> 0x0058a330: reviewer corrected_addr
- CSWSCreature::HasEffect gog 0x006a3e00 -> 0x0058b320: label_suspect
- CSWSCreature::UseLoreOnItem gog 0x006a4d30 -> 0x0058c290: label_suspect
- CSWSCreature::LearnScroll gog 0x006a4da0 -> 0x0058c300: label_suspect
- CSWSCreature::AIActionCheckMoveAwayFromLocation gog 0x006c6230 -> 0x005bb470: match on a mac_rotation row
- CSWSCreature::AIActionCheckInterAreaPathfinding gog 0x006c66e0 -> 0x005bb920: label_suspect, match on a mac_rotation row
- CSWSCreature::AIActionChangeFacingPoint gog 0x006c6930 -> 0x005bbb70: match on a mac_rotation row
- CSWSCreature::AIActionCheckMoveToPoint_2 gog 0x006c7820 -> 0x005bca60: match on a mac_rotation row
- CSWSCreature::AIActionCheckMoveAwayFromObject gog 0x006c9d80 -> 0x005befc0: label_suspect
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest gog 0x006d5a90 -> 0x005cacd0: reviewer corrected_addr
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_PathWaitDest gog 0x006d5bc0 -> 0x005cae00: reviewer corrected_addr
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToOffsetWaitDest gog 0x006d5cf0 -> 0x005caf30: reviewer corrected_addr
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToPathWaitDest gog 0x006d5e20 -> 0x005cb060: reviewer corrected_addr
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_TeleportForward gog 0x006d5f50 -> 0x005cb190: reviewer corrected_addr
- CSWSCreature::AdjustReputation gog 0x006dba10 -> 0x005d0df0: match on a mac_rotation row
- CSWSCreature::AddToVisibleList gog 0x006dc020 -> 0x005d1400: match on a mac_rotation row
- CSWSCreature::GetCreatureReputation_2 gog 0x006dc300 -> 0x005d16e0: label_suspect
- CSWSCreature::CheckItemClassRestrictions gog 0x006de780 -> 0x005d3b60: match on a mac_rotation row
- CSWSCreature::AIActionAnimalEmpathy gog 0x006dee60 -> 0x005d4240: label_suspect
- CSWSCreature::GetRelativeWeaponSize gog 0x006f5300 -> 0x006b8e80: label_suspect

## Sample of the other reviewer matches (12 of 12)

- CSWSCreature::HasAction gog 0x006dfcc0 -> 0x005d50a0 (C0)
- CSWSCreature::CalculateDamagePower gog 0x00698350 -> 0x0057f820 (C1)
- CSWSCreature::GetClientCreature gog 0x006a3970 -> 0x0058ae90 (C1)
- CSWSCreature::GetCurrentlyFollowingLeader gog 0x006a3810 -> 0x0058ad30 (C1)
- CSWSCreature::SetAttemptedAttackTarget gog 0x00696f00 -> 0x0057e3d0 (C1)
- CSWSCreature::AddChemical gog 0x0069a160 -> 0x00581630 (C1)
- CSWSCreature::RemoveGold gog 0x00699fd0 -> 0x005814a0 (C1)
- CSWSCreature::GetLastInstantEquip gog 0x006a3dc0 -> 0x0058b2e0 (C1)
- CSWSCreature::GetListenCheckDistance gog 0x006a3cf0 -> 0x0058b210 (C1)
- CSWSCreature::GetVisibleListElement gog 0x006dc370 -> 0x005d1750 (C1)
- CSWSCreature::GetJournal gog 0x00698e30 -> 0x00580300 (C1)
- CSWSCreature::AddComponent gog 0x0069a240 -> 0x00581710 (C1)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CSWSCreature::AIActionFlagMine gog 0x006bd9d0 -> 0x005b2c10
- sig: CSWSCreature::UpdateAppearanceForEquippedItems gog 0x00698430 -> 0x0057f900
- sig: CSWSCreature::UpdatePersonalSpace gog 0x00698810 -> 0x0057fce0
- sig: CSWSCreature::AIActionCounterSpell gog 0x006dd200 -> 0x005d25e0
- sig: CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_Following_Path gog 0x006d39e0 -> 0x005c8c20
- c2: CSWSCreature::TransferGold gog 0x00698570 -> 0x0057fa40
- c2: CSWSCreature::AIActionEncounterCreatureDestroySelf gog 0x006c7e70 -> 0x005bd0b0
- c2: CSWSCreature::IsPartyVisible gog 0x006a3420 -> 0x0058a940
