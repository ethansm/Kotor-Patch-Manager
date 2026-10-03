# Opus gate B12b part r (REJECT/NOCAND rows, lead addition)

- These rows have no shippable gate pick. For each, search the Steam binary for the true counterpart of the GOG body
  (whole-binary fp compare: size, retn, n_insn, callee set mapped via known names, disp/imm sets; neighbour runs).
- Ship (`verified`/`likely`) only a counterpart whose Steam body you checked against the GOG fp yourself (quote an asm line).
- If the GOG label contradicts the body (label rotation is common in this class), use `label_suspect` and name the Mac symbol.
- Otherwise `rejected` (gate pick wrong, none found) or `unresolved`. sample_errors does not apply to this part (write 0).

| # | gate | Class::Name | GOG addr | gate pick | gog size/retn |
|---|---|---|---|---|---|
| 1 | NOCAND | CSWSCreature::SetDetectMode | 0x006c2820 | - | 51/4 |
| 2 | REJECT | CSWSCreature::CanEquipWeapon | 0x006c7f90 | 0x00405100 | 519/16 |
| 3 | REJECT | CSWSCreature::CanEquipWeapon2 | 0x006c81a0 | 0x00670c90 | 537/16 |
| 4 | REJECT | CSWSCreature::CheckProficiencies | 0x006c8800 | 0x00405970 | 278/8 |
| 5 | NOCAND | CSWSCreature::AIActionCheckMoveAwayFromObject | 0x006c9d80 | - | 64/4 |
| 6 | REJECT | CSWSCreature::AIActionPartyFollowLeader | 0x006ca5b0 | 0x00971dc0 | 328/4 |
| 7 | REJECT | CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_FindPath | 0x006d2130 | 0x0067ac20 | 1884/16 |
| 8 | REJECT | CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_Moving_To_Path | 0x006d35a0 | 0x00669020 | 1061/16 |
| 9 | REJECT | CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_Moving_To_Offset | 0x006d4dc0 | 0x005bbf40 | 1676/16 |
| 10 | REJECT | CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_WaitingForPath | 0x006d5460 | 0x0067df50 | 1573/16 |
| 11 | REJECT | CSWSCreature::AIActionWaitForEndOfRound | 0x006db820 | 0x00418990 | 174/4 |
| 12 | REJECT | CSWSCreature::UpdateVisibleList | 0x006dbb50 | 0x005c3270 | 1218/4 |
| 13 | REJECT | CSWSCreature::AIActionRest | 0x006dc3f0 | 0x00983c00 | 18/4 |
| 14 | REJECT | CSWSCreature::AIActionTaunt | 0x006dc410 | 0x00983c20 | 18/4 |
| 15 | REJECT | CSWSCreature::AIActionAppear | 0x006dc430 | 0x00983c40 | 208/4 |
| 16 | REJECT | CSWSCreature::AIActionDisappear | 0x006dc500 | 0x00983d10 | 208/4 |
| 17 | REJECT | CSWSCreature::AIActionSurrenderToEnemies | 0x006dd5d0 | 0x00984de0 | 55/4 |
| 18 | REJECT | CSWSCreature::AIActionStanceMeditative | 0x006ddac0 | 0x009852d0 | 216/4 |
| 19 | REJECT | CSWSCreature::AIActionAnimalEmpathy | 0x006dee60 | 0x0041bfd0 | 21/8 |
| 20 | NOCAND | CSWSCreature::GetRelativeWeaponSize | 0x006f5300 | - | 83/4 |
