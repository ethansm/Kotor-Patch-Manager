# Opus gate B12a (DESIGN 4.6)

Decision: **OK: Opus reads the 100% classes plus a 20% sample (min 5) of the other reviewer matches**

- decoy cards: 10 (per chunk {'1': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: pending (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (18 rows)

- CSWSCreature::IsInTotalDefense_2 gog 0x00657710 -> 0x006db460: label_suspect
- CSWSCreature::GetWeaponDischargeTime gog 0x00658710 -> 0x006dc460: match on a mac_contra row
- CSWSCreature::ResolveMeleeSpecialAttack gog 0x0065b5a0 -> 0x006df2a0: reviewer corrected_addr
- CSWSCreature::ResolveRangedMiss gog 0x0065c680 -> 0x00543ae0: reviewer corrected_addr, label_suspect
- CSWSCreature::ResolveRangedAnimations gog 0x0065c900 -> 0x006e0600: reviewer corrected_addr
- CSWSCreature::ResolveRangedMissToParry gog 0x0065ef50 -> 0x006e2c50: match on a mac_contra row
- CSWSCreature::DoCombatStep gog 0x0065fd40 -> 0x006e3a40: reviewer corrected_addr
- CSWSCreature::Destructor gog 0x0067c0c0 -> 0x00563390: reviewer corrected_addr
- CSWSCreature::AddDropItemActions gog 0x00680a70 -> 0x00567e40: label_suspect
- CSWSCreature::AddHealActions gog 0x00680b30 -> 0x00567f00: label_suspect
- CSWSCreature::ResolveInitiative gog 0x0068a4b0 -> 0x00571980: label_suspect
- CSWSCreature::RemoveMindTrickEffects gog 0x0068ddf0 -> 0x00575340: reviewer corrected_addr, label_suspect
- CSWSCreature::GetBlind gog 0x0068f7d0 -> 0x00576ca0: label_suspect
- CSWSCreature::SetAmbientAnimationState gog 0x00692510 -> 0x005799e0: match on a mac_contra row
- CSWSCreature::BroadcastStealthDetectionData gog 0x00692f53 -> 0x0057a420: label_suspect
- CSWSCreature::GetDetectMode gog 0x00693490 -> 0x0057a960: label_suspect
- CSWSCreature::AddPickUpItemActions gog 0x00693ac0 -> 0x0057af90: label_suspect
- CSWSCreature::SetLastInstantEquip gog 0x00696db0 -> 0x0057e280: reviewer corrected_addr

## Sample of the other reviewer matches (5 of 20)

- CSWSCreature::GetLockOrientationToObject gog 0x0067be30 -> 0x00563200 (C0)
- CSWSCreature::GetEffectSpellId gog 0x0067bf80 -> 0x00563350 (C0)
- CSWSCreature::DoStealthDetection gog 0x00690a00 -> 0x00577ed0 (C0)
- CSWSCreature::GetWeaponHitShots gog 0x0065aa20 -> 0x006de720 (C0)
- CSWSCreature::SaveFollowInfo gog 0x0068dce0 -> 0x005751b0 (C0)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CSWSCreature::ResetUpdateTimes gog 0x00681ef0 -> 0x005692c0
- sig: CSWSCreature::GetWeaponImpactTime gog 0x0065a5b0 -> 0x006de2b0
- sig: CSWSCreature::BroadcastDeflectBreakdown gog 0x00692f00 -> 0x0057a3d0
- sig: CSWSCreature::BroadcastMineDetectionData gog 0x00692420 -> 0x005798f0
- sig: CSWSCreature::BroadcastThreatBreakdown gog 0x00692dc0 -> 0x0057a290
- c2: CSWSCreature::SignalMeleeDamage gog 0x0065c950 -> 0x006e0650
- c2: CSWSCreature::UpdateCombatRoundTimer gog 0x00689b60 -> 0x00571030
- c2: CSWSCreature::AddPathfindingWaitActionToFront gog 0x0067d2f0 -> 0x005646c0
