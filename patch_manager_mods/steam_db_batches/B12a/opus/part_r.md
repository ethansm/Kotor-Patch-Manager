# Opus gate B12a part r (REJECT/NOCAND rows, lead addition)

- These rows have no shippable gate pick. For each, search the Steam binary for the true counterpart of the GOG body
  (whole-binary fp compare: size, retn, n_insn, callee set mapped via known names, disp/imm sets; neighbour runs).
- Ship (`verified`/`likely`) only a counterpart whose Steam body you checked against the GOG fp yourself (quote an asm line).
- If the GOG label contradicts the body (label rotation is common in this class), use `label_suspect` and name the Mac symbol.
- Otherwise `rejected` (gate pick wrong, none found) or `unresolved`. sample_errors does not apply to this part (write 0).

| # | gate | Class::Name | GOG addr | gate pick | gog size/retn |
|---|---|---|---|---|---|
| 1 | NOCAND | CSWSCreature::CanUseForcePowerOrSpell | 0x00494d00 | - | 96/8 |
| 2 | REJECT | CSWSCreature::ResolveRangedAttack | 0x006571b0 | 0x0053e450 | 1289/28 |
| 3 | REJECT | CSWSCreature::ResolveMeleeAnimations | 0x0065c710 | 0x00543ae0 | 483/20 |
| 4 | REJECT | CSWSCreature::CalculateMaxElementalDamage | 0x0065da20 | 0x005452c0 | 96/8 |
| 5 | REJECT | CSWSCreature::ResolveOnHitEffect | 0x0065f2a0 | 0x00546540 | 1657/12 |
| 6 | REJECT | CSWSCreature::SavingThrowRoll | 0x00660930 | 0x00547d60 | 2418/28 |
| 7 | REJECT | CSWSCreature::ApplyOnHitConfusion | 0x00661610 | 0x00548b10 | 408/8 |
| 8 | REJECT | CSWSCreature::ApplyOnHitFear | 0x006617b0 | 0x00548b10 | 408/8 |
| 9 | REJECT | CSWSCreature::ApplyOnHitSlow | 0x00661950 | 0x00548b10 | 513/8 |
| 10 | REJECT | CSWSCreature::ApplyOnHitPoison | 0x00661b60 | 0x00548db0 | 680/8 |
| 11 | NOCAND | CSWSCreature::ApplyOnHitSlayRacialGroup | 0x00662060 | - | 408/8 |
| 12 | REJECT | CSWSCreature::ApplyOnHitInstantDeath | 0x006625d0 | 0x006e6880 | 183/8 |
| 13 | REJECT | CSWSCreature::ApplyOnHitKnockdown | 0x00662900 | 0x006e6ac0 | 302/8 |
| 14 | REJECT | CSWSCreature::SetAttackSummaryFeedbackEffectData | 0x00663170 | 0x006e71d0 | 800/8 |
| 15 | REJECT | CSWSCreature::GetNearestEnemy | 0x006954c0 | 0x0057c990 | 2524/12 |
| 16 | REJECT | CSWSCreature::SetLastInstantEquip | 0x00696db0 | 0x0057e280 | 202/0 |
