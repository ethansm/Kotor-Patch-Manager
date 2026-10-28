# Opus gate B17 part a (1 of 3)

- batch: B17, part: a, rows: 21 (rows 1-21 of 62 in the full -> sample -> AUTO order of opus_gate.md)
- Decide every row listed here; output per OPUS_GATE_BRIEF.md.

| # | section | Class::Name | GOG addr | Steam addr | why / content level |
|---|---|---|---|---|---|
| 1 | 100% | CSWCItem::LoadModel | 0x0047e830 | 0x007fd5b0 | match cites a layout/offset difference |
| 2 | 100% | CSWGuiLabel::`scalar | 0x00522830 | 0x007510b0 | match on a mac_rotation row |
| 3 | 100% | CSWGuiOptionsSoundAdvanced::SetNewOptions | 0x00571320 | 0x007dba10 | match on a mac_contra row |
| 4 | 100% | CSWGuiBlackenedLabel::Destructor | 0x005977a0 | 0x0074ac30 | label_suspect |
| 5 | 100% | CSWGuiInGameEquip::Draw | 0x0059f8a0 | 0x008aba50 | match on a mac_contra row |
| 6 | 100% | CSWGuiContainer::OnPanelAdded | 0x005a71d0 | 0x008b3630 | match on a mac_contra row |
| 7 | 100% | CSWGuiCreateItem::InitializeStoreType | 0x005c5330 | 0x008d4f60 | label_suspect |
| 8 | 100% | CSWGuiCreateItem::UpdateCreateFilters | 0x005c61f0 | 0x008d5e20 | label_suspect |
| 9 | 100% | CSWGuiCreateMedicalItem::DeleteItemEntries | 0x005ca130 | 0x008d9d60 | label_suspect |
| 10 | 100% | CSWGuiSwoopUpgrade::SetDescription | 0x005cf920 | 0x008df550 | match on a mac_contra row |
| 11 | 100% | CSWGuiCustomPanel::OnSelectFeatsButton | 0x005f42f0 | 0x00914e70 | match cites a layout/offset difference |
| 12 | 100% | CSWGuiFeatsCharGen::RemoveChosenFeat | 0x005fae20 | 0x0090bf90 | match on a mac_contra row, match on a mac_rotation row |
| 13 | 100% | CSWGuiFeatsCharGen::AddChosenFeat | 0x005fafb0 | 0x0090c120 | match on a mac_contra row |
| 14 | 100% | CExoTimers::Constructor | 0x00608260 | 0x00736130 | match cites a layout/offset difference |
| 15 | 100% | CExoInputInternal::UpdateMouseDelta | 0x00615ec0 | 0x0072f110 | match on a mac_contra row |
| 16 | 100% | CResRef::GetResRefStr | 0x0061a300 | 0x007109b0 | match on a mac_rotation row |
| 17 | 100% | CExoKeyTable::GetTableIndex | 0x00630410 | 0x007269c0 | match on a mac_rotation row |
| 18 | 100% | CVirtualMachineInternal::ExecuteCode_5 | 0x00645160 | 0x0044b210 | label_suspect |
| 19 | 100% | CAvoidCreature::ComputeHexPoints | 0x00648370 | 0x006fa580 | match on a mac_rotation row |
| 20 | 100% | CSWRoomSurfaceMesh::ComputeClippedLineSegment | 0x00714000 | 0x006518b0 | match on a mac_rotation row |
| 21 | 100% | GLRender::RenderObjectsVTCQuadUP | 0x00823860 | 0x00431140 | match on a mac_rotation row |
