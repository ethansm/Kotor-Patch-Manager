# Opus gate B23 (DESIGN 4.6)

Decision: **INSUFFICIENT DECOYS (chunks 3; need >= 10 per chunk)**

- decoy cards: 24 (per chunk {'1': 10, '2': 10, '3': 3}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 0 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (0 rows)

None.

## Sample of the other reviewer matches (4 of 4)

- CSWGuiSaveNamePanel::OnPanelRemoved gog 0x005d7fe0 -> 0x00858b60 (C1)
- CSWGuiContainer::OnPanelRemoved gog 0x005a72d0 -> 0x008b3730 (C1)
- CExoInput::UpdateMouseDelta gog 0x00616990 -> 0x0072fd90 (C1)
- CSWGuiPowersFeatsSkillsDebugMenu::CreateOption gog 0x005b3a80 -> 0x008bfe30 (C1)

## AUTO meaning checks (5 sig + 3 c2)

- c2: CSWGuiLoadModuleDebugMenu::CreateOption gog 0x005b60c0 -> 0x008c17f0
