# Opus gate B12b (DESIGN 4.6)

Decision: **OK: Opus reads the 100% classes plus a 20% sample (min 5) of the other reviewer matches**

- decoy cards: 10 (per chunk {'1': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: pending (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (14 rows)

- CSWSCreature::UpdateAppearanceForEquippedItems_2 gog 0x00698df0 -> 0x005802c0: label_suspect
- CSWSCreature::BroadcastAttackOfOpportunity gog 0x0069df00 -> 0x00585420: label_suspect
- CSWSCreature::UpdatePureGoodEvilPowers gog 0x006a2600 -> 0x00589b20: match on a mac_contra row
- CSWSCreature::ComputeArmourClass gog 0x006a26d0 -> 0x00589bf0: label_suspect
- CSWSCreature::SetInParty gog 0x006a2e10 -> 0x0058a610: reviewer corrected_addr
- CSWSCreature::HasEffect gog 0x006a3e00 -> 0x0058b320: label_suspect
- CSWSCreature::UseLoreOnItem gog 0x006a4d30 -> 0x0058c290: label_suspect
- CSWSCreature::LearnScroll gog 0x006a4da0 -> 0x0058c300: label_suspect
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest gog 0x006d5a90 -> 0x005cacd0: reviewer corrected_addr
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_PathWaitDest gog 0x006d5bc0 -> 0x005cae00: reviewer corrected_addr
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToOffsetWaitDest gog 0x006d5cf0 -> 0x005caf30: reviewer corrected_addr
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToPathWaitDest gog 0x006d5e20 -> 0x005cb060: reviewer corrected_addr
- CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_TeleportForward gog 0x006d5f50 -> 0x005cb190: reviewer corrected_addr
- CSWSCreature::GetCreatureReputation_2 gog 0x006dc300 -> 0x005d16e0: label_suspect

## Sample of the other reviewer matches (5 of 19)

- CSWSCreature::HasAction gog 0x006dfcc0 -> 0x005d50a0 (C0)
- CSWSCreature::AIActionCheckInterAreaPathfinding gog 0x006c66e0 -> 0x005bb920 (C1)
- CSWSCreature::GetVisibleListElement gog 0x006dc370 -> 0x005d1750 (C1)
- CSWSCreature::AdjustReputation gog 0x006dba10 -> 0x005d0df0 (C2)
- CSWSCreature::CalculateDamagePower gog 0x00698350 -> 0x0057f820 (C1)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CSWSCreature::AIActionUnequipItem gog 0x006db6a0 -> 0x005d0a80
- sig: CSWSCreature::UpdatePersonalSpace gog 0x00698810 -> 0x0057fce0
- sig: CSWSCreature::CutWaypointPath gog 0x0069db00 -> 0x00585020
- sig: CSWSCreature::CheckItemAlignmentRestrictions gog 0x006de0a0 -> 0x005d3480
- sig: CSWSCreature::CleanInvitationLists gog 0x006df450 -> 0x005d4830
- c2: CSWSCreature::UnsummonMyself gog 0x006e0170 -> 0x005d5550
- c2: CSWSCreature::CanEquipItem gog 0x006c8460 -> 0x005bd6a0
- c2: CSWSCreature::IsPartyVisible gog 0x006a3420 -> 0x0058a940
