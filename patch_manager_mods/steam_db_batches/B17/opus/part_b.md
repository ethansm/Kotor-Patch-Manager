# Opus gate B17 part b (2 of 3)

- batch: B17, part: b, rows: 21 (rows 22-42 of 62 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 22 | 100% | GLRender::FrameBufferModificationsATI | 0x00829060 | 0x004369a0 | match on a mac_rotation row |
| 23 | 100% | Global::GetResource | 0x00867dc0 | 0x00475e50 | label_suspect |
| 24 | 100% | ExplosionEmitter::Update | 0x008b6ed0 | 0x004fb2d0 | match cites a layout/offset difference |
| 25 | 100% | Global::ImageRead16bit | 0x008d7860 | 0x00482ce0 | match cites a layout/offset difference |
| 26 | 100% | Gob::PlayOutOfOrderAnimation | 0x008f6ab0 | 0x004dc920 | match cites a layout/offset difference |
| 27 | 100% | Global::rand | 0x00925936 | 0x008103c0 | reviewer corrected_addr |
| 28 | sample | CClientOptions::SetShadows | 0x00468f00 | 0x007bc230 | C0 |
| 29 | sample | Material::SetBlendingMode | 0x008f5380 | 0x004db1f0 | C0 |
| 30 | sample | CSWMessage::WriteDWORD | 0x0054ba90 | 0x005e43b0 | C0 |
| 31 | sample | CAurGUIStringInternal::SetSizeBoundingRect | 0x008d0ae0 | 0x0047a1d0 | C0 |
| 32 | sample | Global::QueryTimer | 0x00713490 | 0x00650d40 | C0 |
| 33 | sample | CAurInternal::RestoreDesktopDisplayMode | 0x008689c0 | 0x00476ae0 | C0 |
| 34 | sample | CClientOptions::SetSoftShadows | 0x00468f30 | 0x007bc260 | C0 |
| 35 | sample | CClientOptions::SetVSync | 0x004691f0 | 0x007bc520 | C0 |
| 36 | sample | CSWSPlayerLUOInventory::Destructor | 0x007f2b50 | 0x006f2fb0 | C0 |
| 37 | sample | CServerAIBodyBagInfo::LoadBodyBag | 0x006509e0 | 0x00520fc0 | C0 |
| 38 | sample | Global::BinaryInsertModel | 0x008ce5e0 | 0x0048e090 | C0 |
| 39 | sample | CSWGuiMainInterface::Quicksave | 0x0044ccb0 | 0x007a06b0 | C1 |
| 40 | sample | RequestPoolManagerVBO::ResizePool | 0x0081c950 | 0x0042a220 | C1 |
| 41 | sample | CSWGuiOptionsGraphicsAdvanced::OnTexQualLeft | 0x0056d040 | 0x007d7730 | C1 |
| 42 | sample | CSWGui3DSceneView::Draw | 0x0053c050 | 0x0084de40 | C1 |
