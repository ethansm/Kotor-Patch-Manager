# Opus gate B12a (DESIGN 4.6)

Decision: **OK: Opus reads the 100% classes plus a 20% sample (min 5) of the other reviewer matches**

- decoy cards: 10 (per chunk {'1': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 0 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (32 rows)

- CSWSCreature::CanUseForcePowerOrSpell gog 0x00494d00 -> 0x0084a170: label_suspect
- CSWSCreature::IsInTotalDefense_2 gog 0x00657710 -> 0x006db460: label_suspect
- CSWSCreature::ResolvePostRangedDamage gog 0x00658540 -> 0x006dc290: label_suspect
- CSWSCreature::GetWeaponDischargeTime gog 0x00658710 -> 0x006dc460: label_suspect, match on a mac_contra row
- CSWSCreature::IsLightsaberEquipped gog 0x006587e0 -> 0x006dc530: label_suspect
- CSWSCreature::ResolveRangedHit gog 0x0065b530 -> 0x006df230: label_suspect
- CSWSCreature::ResolveMeleeSpecialAttack gog 0x0065b5a0 -> 0x006df2a0: reviewer corrected_addr
- CSWSCreature::ResolveRangedMiss gog 0x0065c680 -> 0x006e0380: reviewer corrected_addr, label_suspect
- CSWSCreature::ResolveRangedAnimations gog 0x0065c900 -> 0x006e0600: reviewer corrected_addr, label_suspect
- CSWSCreature::ApplyOnHitSleep gog 0x0065dc30 -> 0x006e1930: label_suspect, match on a mac_rotation row
- CSWSCreature::ApplyOnHitSlayAlignmentGroup gog 0x0065e710 -> 0x006e2410: label_suspect
- CSWSCreature::ResolveRangedMissToParry gog 0x0065ef50 -> 0x006e2c50: label_suspect, match on a mac_contra row, match on a mac_rotation row
- CSWSCreature::DoCombatStep gog 0x0065fd40 -> 0x006e3a40: reviewer corrected_addr
- CSWSCreature::ApplyOnHitHold gog 0x006614d0 -> 0x006e51d0: label_suspect, match on a mac_rotation row
- CSWSCreature::ApplyOnHitSlow gog 0x00661950 -> 0x006e5650: label_suspect
- CSWSCreature::ApplyOnHitPoison gog 0x00661b60 -> 0x006e5860: label_suspect
- CSWSCreature::ApplyOnHitSlayRacialGroup gog 0x00662060 -> 0x006e5d60: label_suspect
- CSWSCreature::ApplyOnHitInstantDeath gog 0x006625d0 -> 0x006e62d0: label_suspect
- CSWSCreature::ApplyOnHitKnockdown gog 0x00662900 -> 0x006e6600: label_suspect
- CSWSCreature::Destructor gog 0x0067c0c0 -> 0x00563490: reviewer corrected_addr
- CSWSCreature::AddDropItemActions gog 0x00680a70 -> 0x00567e40: label_suspect
- CSWSCreature::AddHealActions gog 0x00680b30 -> 0x00567f00: label_suspect
- CSWSCreature::Rest gog 0x00680c50 -> 0x00568020: label_suspect
- CSWSCreature::ResolveInitiative gog 0x0068a4b0 -> 0x00571980: label_suspect
- CSWSCreature::RemoveMindTrickEffects gog 0x0068ddf0 -> 0x005752c0: reviewer corrected_addr, label_suspect
- CSWSCreature::GetBlind gog 0x0068f7d0 -> 0x00576ca0: label_suspect
- CSWSCreature::BroadcastForcePointUsageData gog 0x00692330 -> 0x00579800: label_suspect
- CSWSCreature::SetAmbientAnimationState gog 0x00692510 -> 0x005799e0: label_suspect, match on a mac_contra row
- CSWSCreature::GetDetectMode gog 0x00693490 -> 0x0057a960: label_suspect
- CSWSCreature::AddPickUpItemActions gog 0x00693ac0 -> 0x0057af90: label_suspect
- CSWSCreature::SetLastInstantEquip gog 0x00696db0 -> 0x0057e280: reviewer corrected_addr, label_suspect
- CSWSCreature::Rest_2 gog 0x00696e80 -> 0x0057e350: label_suspect

## Sample of the other reviewer matches (5 of 18)

- CSWSCreature::SetEffectSpellId gog 0x0067bfa0 -> 0x00563370 (C0)
- CSWSCreature::GetLockOrientationToObject gog 0x0067be30 -> 0x00563200 (C0)
- CSWSCreature::GetWeaponHitShots gog 0x0065aa20 -> 0x006de720 (C0)
- CSWSCreature::SaveFollowInfo gog 0x0068dce0 -> 0x005751b0 (C0)
- CSWSCreature::GetEffectSpellId gog 0x0067bf80 -> 0x00563350 (C0)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CSWSCreature::ResolvePostMeleeDamage gog 0x0065da80 -> 0x006e1780
- sig: CSWSCreature::GetNearestObjects gog 0x00694970 -> 0x0057be40
- sig: CSWSCreature::GetFirstName gog 0x0067bf20 -> 0x005632f0
- sig: CSWSCreature::GetDialogResref gog 0x0067be50 -> 0x00563220
- sig: CSWSCreature::ApplyOnHitAbilityDamage gog 0x00662200 -> 0x006e5f00
- c2: CSWSCreature::ResolveAttack gog 0x00656fa0 -> 0x006dacf0
- c2: CSWSCreature::AIUpdate gog 0x00681f50 -> 0x00569320
- c2: CSWSCreature::IsInTotalDefense gog 0x00656d70 -> 0x006daac0
