# Opus gate B20 (DESIGN 4.6)

Decision: **OK: Opus reads the 100% classes plus a 20% sample (min 5) of the other reviewer matches**

- decoy cards: 60 (per chunk {'1': 10, '2': 10, '3': 10, '4': 10, '5': 10, '6': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 0 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (0 rows)

None.

## Sample of the other reviewer matches (5 of 16)

- CSWGuiLegalScreen::Destructor gog 0x0053a810 -> 0x0087fdf0 (C0)
- CExoMoviePlayer::PlayMoviesAsync gog 0x009211e0 -> 0x0093b590 (C1)
- PartLightsaber::Draw gog 0x00890b90 -> 0x004a9ea0 (C2)
- CSWGuiOptionsMouse::HandleInputEvent gog 0x005725e0 -> 0x007dccd0 (C1)
- CSWGuiInGameJournal::PopulateItemListBox gog 0x00591bd0 -> 0x007fc880 (C2)

## AUTO meaning checks (5 sig + 3 c2)

- c2: CExoKeyTable::RebuildTable gog 0x006304a0 -> 0x00726a50
- c2: CSWSMessage::SendServerToPlayerDebugInfo_Creature gog 0x007e2e00 -> 0x0064c6e0
- c2: CSWCCreatureAppearance::GetBTypeModelAndTextureResRef gog 0x0048e3b0 -> 0x0085e320
