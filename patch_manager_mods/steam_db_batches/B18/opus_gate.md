# Opus gate B18 (DESIGN 4.6)

Decision: **INSUFFICIENT DECOYS (chunks 5; need >= 10 per chunk)**

- decoy cards: 43 (per chunk {'1': 10, '2': 10, '3': 10, '4': 10, '5': 3}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: 0 (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 3 -> sample rate 10%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (8 rows)

- CExoSound::StopAllOneShots gog 0x0063aaa0 -> 0x0070bb00: reviewer corrected_addr
- CSWCollisionMesh::NormalLocalToWorld gog 0x00728b10 -> 0x005eebf0: reviewer corrected_addr, match on a mac_rotation row
- GLRender::FrameBufferModifications gog 0x00827e30 -> 0x00435720: reviewer corrected_addr
- Global::AurResGetNextLine gog 0x00867ee0 -> 0x00475f70: reviewer corrected_addr
- Global::AurSetVSyncState gog 0x0086ac50 -> 0x00478d70: reviewer corrected_addr
- Global::AurEnableForceFury gog 0x0086ae30 -> 0x00478f50: reviewer corrected_addr
- Global::AurMultiTextureAvailable gog 0x0086b4a0 -> 0x00484710: reviewer corrected_addr
- Global::InsertModel gog 0x008ce680 -> 0x0048e130: reviewer corrected_addr

## Sample of the other reviewer matches (4 of 4)

- InitControl::ScaleToEdgeHelper_2 gog 0x0090deb0 -> 0x0040fa30 (C1)
- Global::AurGUICloseViewport gog 0x008d02d0 -> 0x004799c0 (C1)
- Global::AurGUIStopLayer gog 0x008cfd00 -> 0x004793f0 (C1)
- CSWGuiObject::SetExtent gog 0x00912f40 -> 0x00414b70 (C1)

## AUTO meaning checks (5 sig + 3 c2)

None.
