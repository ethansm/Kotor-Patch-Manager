# Opus gate B12a part a (1 of 1)

- batch: B12a, part: a, rows: 31 (rows 1-31 of 31 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 1 | 100% | CSWSCreature::IsInTotalDefense_2 | 0x00657710 | 0x006db460 | label_suspect |
| 2 | 100% | CSWSCreature::GetWeaponDischargeTime | 0x00658710 | 0x006dc460 | match on a mac_contra row |
| 3 | 100% | CSWSCreature::ResolveMeleeSpecialAttack | 0x0065b5a0 | 0x006df2a0 | reviewer corrected_addr |
| 4 | 100% | CSWSCreature::ResolveRangedMiss | 0x0065c680 | 0x00543ae0 | reviewer corrected_addr, label_suspect |
| 5 | 100% | CSWSCreature::ResolveRangedAnimations | 0x0065c900 | 0x006e0600 | reviewer corrected_addr |
| 6 | 100% | CSWSCreature::ResolveRangedMissToParry | 0x0065ef50 | 0x006e2c50 | match on a mac_contra row |
| 7 | 100% | CSWSCreature::DoCombatStep | 0x0065fd40 | 0x006e3a40 | reviewer corrected_addr |
| 8 | 100% | CSWSCreature::Destructor | 0x0067c0c0 | 0x00563390 | reviewer corrected_addr |
| 9 | 100% | CSWSCreature::AddDropItemActions | 0x00680a70 | 0x00567e40 | label_suspect |
| 10 | 100% | CSWSCreature::AddHealActions | 0x00680b30 | 0x00567f00 | label_suspect |
| 11 | 100% | CSWSCreature::ResolveInitiative | 0x0068a4b0 | 0x00571980 | label_suspect |
| 12 | 100% | CSWSCreature::RemoveMindTrickEffects | 0x0068ddf0 | 0x00575340 | reviewer corrected_addr, label_suspect |
| 13 | 100% | CSWSCreature::GetBlind | 0x0068f7d0 | 0x00576ca0 | label_suspect |
| 14 | 100% | CSWSCreature::SetAmbientAnimationState | 0x00692510 | 0x005799e0 | match on a mac_contra row |
| 15 | 100% | CSWSCreature::BroadcastStealthDetectionData | 0x00692f53 | 0x0057a420 | label_suspect |
| 16 | 100% | CSWSCreature::GetDetectMode | 0x00693490 | 0x0057a960 | label_suspect |
| 17 | 100% | CSWSCreature::AddPickUpItemActions | 0x00693ac0 | 0x0057af90 | label_suspect |
| 18 | 100% | CSWSCreature::SetLastInstantEquip | 0x00696db0 | 0x0057e280 | reviewer corrected_addr |
| 19 | sample | CSWSCreature::GetLockOrientationToObject | 0x0067be30 | 0x00563200 | C0 |
| 20 | sample | CSWSCreature::GetEffectSpellId | 0x0067bf80 | 0x00563350 | C0 |
| 21 | sample | CSWSCreature::DoStealthDetection | 0x00690a00 | 0x00577ed0 | C0 |
| 22 | sample | CSWSCreature::GetWeaponHitShots | 0x0065aa20 | 0x006de720 | C0 |
| 23 | sample | CSWSCreature::SaveFollowInfo | 0x0068dce0 | 0x005751b0 | C0 |
| 24 | AUTO sig | CSWSCreature::ResetUpdateTimes | 0x00681ef0 | 0x005692c0 |  |
| 25 | AUTO sig | CSWSCreature::GetWeaponImpactTime | 0x0065a5b0 | 0x006de2b0 |  |
| 26 | AUTO sig | CSWSCreature::BroadcastDeflectBreakdown | 0x00692f00 | 0x0057a3d0 |  |
| 27 | AUTO sig | CSWSCreature::BroadcastMineDetectionData | 0x00692420 | 0x005798f0 |  |
| 28 | AUTO sig | CSWSCreature::BroadcastThreatBreakdown | 0x00692dc0 | 0x0057a290 |  |
| 29 | AUTO c2 | CSWSCreature::SignalMeleeDamage | 0x0065c950 | 0x006e0650 |  |
| 30 | AUTO c2 | CSWSCreature::UpdateCombatRoundTimer | 0x00689b60 | 0x00571030 |  |
| 31 | AUTO c2 | CSWSCreature::AddPathfindingWaitActionToFront | 0x0067d2f0 | 0x005646c0 |  |
