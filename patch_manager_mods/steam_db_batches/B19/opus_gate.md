# Opus gate B19 (DESIGN 4.6)

Decision: **INSUFFICIENT DECOYS (chunks 1; need >= 10 per chunk)**

- decoy cards: 4 (per chunk {'1': 4}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 0 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 0 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (0 rows)

None.

## Sample of the other reviewer matches (1 of 1)

- PartEmitter::RenderStandard gog 0x008b1f40 -> 0x004f5f50 (C1)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CExoKeyTable::AddEncapsulatedContents gog 0x0062d270 -> 0x00723820
- sig: RequestPoolManagerVBO::Constructor gog 0x0081be20 -> 0x004296f0
- sig: CExoKeyTable::AddResourceImageContents gog 0x0062e1d0 -> 0x00724780
- c2: CSWSpellArray::Load gog 0x004ed490 -> 0x006c1490
- c2: CSWCMessage::HandleServerToPlayerSafeProjectileSpell gog 0x004bdea0 -> 0x00829850
- c2: CSWGuiText::Load gog 0x00915850 -> 0x00417480
