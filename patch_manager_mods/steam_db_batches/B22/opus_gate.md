# Opus gate B22 (DESIGN 4.6)

Decision: **INSUFFICIENT DECOYS (chunks 1, 2; need >= 10 per chunk)**

- decoy cards: 13 (per chunk {'1': 9, '2': 3}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 0 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 2 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (1 rows)

- Global::AurEnableShadows gog 0x0086ab80 -> 0x00478ca0: reviewer corrected_addr

## Sample of the other reviewer matches (3 of 3)

- Camera::MapToScreen gog 0x008c1b40 -> 0x004809d0 (C1)
- CSWCModule::Destructor gog 0x0046a080 -> 0x007f2e60 (C1)
- CExoTimersInternal::GetLowResolutionTimer gog 0x0060de70 -> 0x0073b7f0 (C1)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CSWCCreatureAppearance::GetCameraOffset gog 0x0048d460 -> 0x0085d3d0
- sig: CSWCVisualEffectOnObject::Constructor gog 0x004fa9f0 -> 0x0086f110
- sig: CSWGuiMainMenu::HandleInputEvent gog 0x0053e460 -> 0x008839c0
- sig: Gob::DeregisterCallback gog 0x008f7380 -> 0x004dd1f0
- sig: CSWCVisualEffect::AttachToScene gog 0x004f9350 -> 0x0086da00
- c2: CSWCVisualEffectOnObject::IsDone gog 0x004fd290 -> 0x008719b0
