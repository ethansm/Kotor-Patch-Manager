# Opus gate B10a part r (REJECT/NOCAND rows, lead addition)

- These rows have no shippable gate pick. For each, search the Steam binary for the true counterpart of the GOG body
  (whole-binary fp compare: size, retn, n_insn, callee set mapped via known names, disp/imm sets; neighbour runs).
- Ship (`verified`/`likely`) only a counterpart whose Steam body you checked against the GOG fp yourself (quote an asm line).
- If the GOG label contradicts the body, use `label_suspect` and name the Mac symbol.
- Otherwise `rejected` (gate pick wrong, none found) or `unresolved`. sample_errors does not apply to this part (write 0).

| # | gate | Class::Name | GOG addr | gate pick | gog size/retn |
|---|---|---|---|---|---|
| 1 | NOCAND | CSWCObject::MenuActionSetObjectMine | 0x0042b680 | - | 85/8 |
| 2 | REJECT | CSWCObject::ClientServerSyncHandler_Position | 0x004726c0 | 0x00405970 | 33/12 |
| 3 | REJECT | CSWCObject::BarkBubble | 0x004774e0 | 0x007f23f0 | 62/16 |
| 4 | REJECT | CSWSObject::GetMaxHitPoints | 0x00663800 | 0x004597f0 | 23/4 |
| 5 | NOCAND | CSWSObject::GetLockOrientationToObject | 0x00663830 | - | 16/0 |
| 6 | REJECT | CSWSObject::GetFirstName | 0x006638c0 | 0x004598b0 | 19/0 |
| 7 | NOCAND | CSWSObject::AddSwitchWeaponsAction | 0x006969a0 | - | 716/4 |
| 8 | NOCAND | CSWCObject::LipSync | 0x006a2c90 | - | 15/4 |
| 9 | NOCAND | CSWSObject::AIActionPauseDialog | 0x006b5760 | - | 274/4 |
| 10 | NOCAND | CSWSObject::AIActionResumeDialog | 0x006b5880 | - | 320/4 |
| 11 | NOCAND | CSWSObject::AIActionSwitchWeapons | 0x006bbb10 | - | 696/4 |
| 12 | REJECT | CSWSObject::AddUseObjectAction | 0x006bd120 | 0x005564b0 | 111/4 |
| 13 | REJECT | CSWSObject::AddGiveItemActions | 0x006bd190 | 0x005564b0 | 419/16 |
| 14 | REJECT | CSWSObject::AddTakeItemActions | 0x006bd340 | 0x005564b0 | 486/12 |
