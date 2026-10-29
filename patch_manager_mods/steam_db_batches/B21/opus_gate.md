# Opus gate B21 (DESIGN 4.6)

Decision: **INSUFFICIENT DECOYS (chunks none reviewed; need >= 10 per chunk)**

- decoy cards: 0 (per chunk {}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 0 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 1 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (0 rows)

None.

## Sample of the other reviewer matches (0 of 0)

None.

## AUTO meaning checks (5 sig + 3 c2)

- sig: CClientMsgSysAdmin::SendSysAdminToServerMessage gog 0x0053ef40 -> 0x00884450
- sig: CSWGuiImage::Destructor_2 gog 0x009147e0 -> 0x00416410
- sig: Gob::DeregisterCallback gog 0x008f7380 -> 0x004dd1f0
- sig: CSWCVisualEffectOnObject::HideVisualEffect gog 0x004fd100 -> 0x00871820
- sig: CSWGuiImage::GetImageExtent gog 0x00914880 -> 0x004164b0
- c2: CSWCVisualEffectOnObject::LoadVisualEffect gog 0x004fae10 -> 0x0086f530
- c2: CSWCMessage::HandleServerToPlayerCreatureUpdate_VisEffect gog 0x0049af50 -> 0x00806960
- c2: CSWCVisualEffect::LoadEffect gog 0x004f95d0 -> 0x0086dc80
