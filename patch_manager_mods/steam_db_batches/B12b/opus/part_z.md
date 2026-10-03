# Opus gate B12b part z (consolidation, lead addition)

- The batch ESCALATED (part a found 1 sample error: a rotated GOG label). Decide every row below per OPUS_GATE_BRIEF.md.
- `match on a mac_rotation row`: the counterpart may be right while the GOG label is rotated; decide the name from the body vs Mac.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 1 | 100% | CSWSCreature::AIActionCheckMoveAwayFromLocation | 0x006c6230 | 0x005bb470 | match on a mac_rotation row |
| 2 | 100% | CSWSCreature::AIActionChangeFacingPoint | 0x006c6930 | 0x005bbb70 | match on a mac_rotation row |
| 3 | 100% | CSWSCreature::AIActionCheckMoveToPoint_2 | 0x006c7820 | 0x005bca60 | match on a mac_rotation row |
| 4 | 100% | CSWSCreature::AddToVisibleList | 0x006dc020 | 0x005d1400 | match on a mac_rotation row |
| 5 | 100% | CSWSCreature::CheckItemClassRestrictions | 0x006de780 | 0x005d3b60 | match on a mac_rotation row |
| 6 | escalated | CSWSCreature::GetClientCreature | 0x006a3970 | 0x0058ae90 | C1 |
| 7 | escalated | CSWSCreature::GetCurrentlyFollowingLeader | 0x006a3810 | 0x0058ad30 | C1 |
| 8 | escalated | CSWSCreature::SetAttemptedAttackTarget | 0x00696f00 | 0x0057e3d0 | C1 |
| 9 | escalated | CSWSCreature::AddChemical | 0x0069a160 | 0x00581630 | C1 |
| 10 | escalated | CSWSCreature::RemoveGold | 0x00699fd0 | 0x005814a0 | C1 |
| 11 | escalated | CSWSCreature::GetLastInstantEquip | 0x006a3dc0 | 0x0058b2e0 | C1 |
| 12 | escalated | CSWSCreature::GetListenCheckDistance | 0x006a3cf0 | 0x0058b210 | C1 |
| 13 | escalated | CSWSCreature::GetJournal | 0x00698e30 | 0x00580300 | C1 |
| 14 | escalated | CSWSCreature::AddComponent | 0x0069a240 | 0x00581710 | C1 |
