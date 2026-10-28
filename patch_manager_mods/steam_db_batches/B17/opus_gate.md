# Opus gate B17 (DESIGN 4.6)

Decision: **OK: Opus reads the 100% classes plus a 20% sample (min 5) of the other reviewer matches**

- decoy cards: 150 (per chunk {'1': 10, '2': 10, '3': 10, '4': 10, '5': 10, '6': 10, '7': 10, '8': 10, '9': 10, '10': 10, '11': 10, '12': 10, '13': 10, '14': 10, '15': 10}), accepted by the reviewer: 0, unanswered: 0
- chunks not reviewed: none
- Opus-found sample errors: pending (pass `finalize --opus-sample-errors N` after the Opus pass)
- consecutive clean batches before this one: 2 -> sample rate 20%

## Decoy key errors (decoy = Opus-shipped counterpart)

None.

## Decoys accepted (0 tolerance)

None.

## Opus reads 100% (27 rows)

- CSWCItem::LoadModel gog 0x0047e830 -> 0x007fd5b0: match cites a layout/offset difference
- CSWGuiLabel::`scalar gog 0x00522830 -> 0x007510b0: match on a mac_rotation row
- CSWGuiOptionsSoundAdvanced::SetNewOptions gog 0x00571320 -> 0x007dba10: match on a mac_contra row
- CSWGuiBlackenedLabel::Destructor gog 0x005977a0 -> 0x0074ac30: label_suspect
- CSWGuiInGameEquip::Draw gog 0x0059f8a0 -> 0x008aba50: match on a mac_contra row
- CSWGuiContainer::OnPanelAdded gog 0x005a71d0 -> 0x008b3630: match on a mac_contra row
- CSWGuiCreateItem::InitializeStoreType gog 0x005c5330 -> 0x008d4f60: label_suspect
- CSWGuiCreateItem::UpdateCreateFilters gog 0x005c61f0 -> 0x008d5e20: label_suspect
- CSWGuiCreateMedicalItem::DeleteItemEntries gog 0x005ca130 -> 0x008d9d60: label_suspect
- CSWGuiSwoopUpgrade::SetDescription gog 0x005cf920 -> 0x008df550: match on a mac_contra row
- CSWGuiCustomPanel::OnSelectFeatsButton gog 0x005f42f0 -> 0x00914e70: match cites a layout/offset difference
- CSWGuiFeatsCharGen::RemoveChosenFeat gog 0x005fae20 -> 0x0090bf90: match on a mac_contra row, match on a mac_rotation row
- CSWGuiFeatsCharGen::AddChosenFeat gog 0x005fafb0 -> 0x0090c120: match on a mac_contra row
- CExoTimers::Constructor gog 0x00608260 -> 0x00736130: match cites a layout/offset difference
- CExoInputInternal::UpdateMouseDelta gog 0x00615ec0 -> 0x0072f110: match on a mac_contra row
- CResRef::GetResRefStr gog 0x0061a300 -> 0x007109b0: match on a mac_rotation row
- CExoKeyTable::GetTableIndex gog 0x00630410 -> 0x007269c0: match on a mac_rotation row
- CVirtualMachineInternal::ExecuteCode_5 gog 0x00645160 -> 0x0044b210: label_suspect
- CAvoidCreature::ComputeHexPoints gog 0x00648370 -> 0x006fa580: match on a mac_rotation row
- CSWRoomSurfaceMesh::ComputeClippedLineSegment gog 0x00714000 -> 0x006518b0: match on a mac_rotation row
- GLRender::RenderObjectsVTCQuadUP gog 0x00823860 -> 0x00431140: match on a mac_rotation row
- GLRender::FrameBufferModificationsATI gog 0x00829060 -> 0x004369a0: label_suspect, match on a mac_rotation row
- Global::GetResource gog 0x00867dc0 -> 0x00475e50: label_suspect
- ExplosionEmitter::Update gog 0x008b6ed0 -> 0x004fb2d0: match cites a layout/offset difference
- Global::ImageRead16bit gog 0x008d7860 -> 0x00482ce0: match cites a layout/offset difference
- Gob::PlayOutOfOrderAnimation gog 0x008f6ab0 -> 0x004dc920: match cites a layout/offset difference
- Global::rand gog 0x00925936 -> 0x0091eb8b: reviewer corrected_addr

## Sample of the other reviewer matches (27 of 132)

- CClientOptions::SetShadows gog 0x00468f00 -> 0x007bc230 (C0)
- Material::SetBlendingMode gog 0x008f5380 -> 0x004db1f0 (C0)
- CSWMessage::WriteDWORD gog 0x0054ba90 -> 0x005e43b0 (C0)
- CAurGUIStringInternal::SetSizeBoundingRect gog 0x008d0ae0 -> 0x0047a1d0 (C0)
- Global::QueryTimer gog 0x00713490 -> 0x00650d40 (C0)
- CAurInternal::RestoreDesktopDisplayMode gog 0x008689c0 -> 0x00476ae0 (C0)
- CClientOptions::SetSoftShadows gog 0x00468f30 -> 0x007bc260 (C0)
- CClientOptions::SetVSync gog 0x004691f0 -> 0x007bc520 (C0)
- CSWSPlayerLUOInventory::Destructor gog 0x007f2b50 -> 0x006f2fb0 (C0)
- CServerAIBodyBagInfo::LoadBodyBag gog 0x006509e0 -> 0x00520fc0 (C0)
- Global::BinaryInsertModel gog 0x008ce5e0 -> 0x0048e090 (C0)
- CSWGuiMainInterface::Quicksave gog 0x0044ccb0 -> 0x007a06b0 (C1)
- RequestPoolManagerVBO::ResizePool gog 0x0081c950 -> 0x0042a220 (C1)
- CSWGuiOptionsGraphicsAdvanced::OnTexQualLeft gog 0x0056d040 -> 0x007d7730 (C1)
- CSWGui3DSceneView::Draw gog 0x0053c050 -> 0x0084de40 (C1)
- CSWGuiExamine::HandleInputEvent gog 0x005b3580 -> 0x008bed80 (C2)
- CSWCModule::ZoomCamera gog 0x0046b6d0 -> 0x007f44b0 (C1)
- Global::AurGUIStartLayer gog 0x008cfbd0 -> 0x004792c0 (C1)
- CSWSpellArray::GetSpell gog 0x004ed450 -> 0x006c1450 (C2)
- Camera::Animate gog 0x008c1fb0 -> 0x00480e40 (C1)
- LightManager::GetShadowLights gog 0x00901f50 -> 0x0050a630 (C1)
- PartEmitter::RenderLinked gog 0x008af6e0 -> 0x004f3b40 (C1)
- Global::ImageReadTGA gog 0x008d8010 -> 0x00483490 (C2)
- CSWGuiSkillsCharGen::Destructor_2 gog 0x005fd330 -> 0x0090e440 (C1)
- Global::Transpose gog 0x0080c8b0 -> 0x00514a20 (C1)
- Global::InitOpenGLWindow gog 0x00408600 -> 0x00408af0 (C2)
- CSWBehaviorCameraDialog::GetCameraParameters gog 0x005750f0 -> 0x008c2960 (C1)

## AUTO meaning checks (5 sig + 3 c2)

- sig: CResTGA::ReadUnmappedRLETGA gog 0x00943fe0 -> 0x00945990
- sig: CSWGuiNavigable::HandleMouseOver gog 0x009169f0 -> 0x00418750
- sig: CERFFile::Read gog 0x0062adc0 -> 0x00720e30
- sig: Gob::SetPartLocalPosition gog 0x008551e0 -> 0x00462c50
- sig: CSWGuiInGameOptKeyMappings::Update gog 0x005f0340 -> 0x00901590
- c2: CSWGuiStatusSummary::Constructor gog 0x0052e4c0 -> 0x0075cec0
- c2: CSWSMessage::WriteGameObjUpdate_PlayerUpdate gog 0x007d5c70 -> 0x0063f550
- c2: CERFFile::ExportFilesFromERF gog 0x0062be80 -> 0x00721ef0
