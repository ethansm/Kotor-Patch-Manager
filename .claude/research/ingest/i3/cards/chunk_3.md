# Name check cards (blind)

Each card gives static facts about one address in swkotor2.exe and ONE candidate name for it.
Judge only from the facts on the card. Do not read any other file.

- consistent: the facts fit the name (what it calls, reads, compares, strings, how it is used).
- inconsistent: the facts contradict the name (e.g. "Vtable" but the address is a float divisor,
  "Iat" but it is never CALLed through, a GUI name on an audio routine).
- unsure: the facts say too little either way.

Naming noise to ignore: prefixes Addr/k/G_/g_, suffixes Global/Val/Flag. Iat = import address table
slot (CALL dword ptr [addr]); Vtable/Vtbl = vtable pointer stored into an object (MOV [reg],addr).

Write one TSV line per card: `card<TAB>verdict<TAB>quote` where quote is the one fact line
(copied verbatim, may be shortened) that decided it. Header line: `card	verdict	quote`.

## C025
kind: function
address: 0x00435720
candidate name: `CSWGuiMainInterface::Constructor`
facts:
- size 1885 B, RET 0
- callers 1: 0x00478300
- callees: 0x004848b0 (CALL), 0x00000023 (CALL), 0x00000045 (CALL), 0x00000044 (CALL), 0x00484970 (CALL), 0x00000121 (CALL)
- indirect: 0x0043573f slot 0x28, 0x0043576f unresolved, 0x00435796 unresolved, 0x004357aa unresolved, 0x004357b6 unresolved, ...
- globals: 0x00a1b484 READ
- imports: ReleaseDC (USER32.DLL), glDeleteTextures (OPENGL32.DLL), wglDeleteContext (OPENGL32.DLL), ...
- twins: none

## C026
kind: function
address: 0x00769f60
candidate name: `CSWCCreature::LoadModel`
facts:
- size 2654 B, RET 12
- callees: 0x00919723 operator new (CALL), 0x0085f7e0 (CALL), 0x0086bfd0 (CALL), 0x0086aea0 (CALL), 0x008695d0 (CALL), 0x00867f00 (CALL), ...
- indirect: 0x00769fe6 slot 0x0, 0x0076a0c8 slot 0x0, 0x0076a472 slot 0xc, 0x0076a4eb slot 0x8, 0x0076a502 slot 0x74, ...
- strings: 0x0099efe4 "CSWCCreature::LoadModel(): Failed to load creature model '%s'."
- twins: none

## C027
kind: function
address: 0x004687e0
candidate name: `Scene::DoGobShadows`
facts:
- size 876 B, RET 0
- callees: 0x00463ed0 (CALL), 0x0050a630 (CALL), 0x00473fb0 (CALL), 0x00463b20 (CALL), 0x005142f0 (CALL), 0x00465c90 (CALL)
- indirect: 0x004688c3 slot 0x14, 0x00468a12 slot 0x14
- twins: none

## C028
kind: function
address: 0x005f7350
candidate name: `CSWMapInfo::GetMarkerPos`
facts:
- size 93 B, RET 12
- callers 2: 0x00750790, 0x00895cc0 CSWGuiMapHider::Draw
- twins: none

## C029
kind: function
address: 0x00747210
candidate name: `CSWGuiMainInterface::Constructor`
facts:
- size 13027 B, RET 4
- callers 2: 0x007be4c0, 0x007cfca0 CGuiInGame::ResetInterfaceForSize
- callees: 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x0091d67e (CALL), 0x00742150 (CALL), 0x0074a910 (CALL), 0x00419740 (CALL), 0x0084a640 (CALL), ...
- indirect: 0x00748008 slot 0x98, 0x007487d3 slot 0x4, 0x0074889b slot 0x4, 0x007496ac slot 0x4, 0x00749951 slot 0x4, ...
- strings: 0x0099d9b4 "uibit_fill_2bt", 0x0099d9c4 "uibit_brdr_16bet", 0x0099d9d8 "uibit_brdr_16bct", ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ, 0x00a1b4a4 READ, ...
- twins: none

## C030
kind: function
address: 0x00433cd0
candidate name: `GLRender::CreateTexture`
facts:
- size 1493 B, RET 0
- callers 1: 0x004265e0 CAurTextureBasic::glImage
- callees: 0x00484100 (CALL), 0x00484400 (CALL), 0x004288d0 (CALL), 0x009196fd operator delete (CALL), 0x00484a60 (CALL), 0x00000021 (CALL), ...
- indirect: 0x00433ce1 slot 0x4c, 0x00433cf1 slot 0x54, 0x00433d07 slot 0x3c, 0x00433d17 slot 0x5c, 0x00433d27 slot 0x60, ...
- imports: glTexImage2D (OPENGL32.DLL), glTexParameteri (OPENGL32.DLL), glTexSubImage2D (OPENGL32.DLL), ...
- twins: none

## C031
kind: function
address: 0x00511fb0
candidate name: `CModelLoader::LoadModelByName`
facts:
- size 368 B, RET 4
- callers 1: 0x00511f50
- callees: 0x00475ab0 (CALL), 0x004762f0 (CALL), 0x00476370 (CALL), 0x005122e0 (CALL), 0x00452830 (CALL), 0x00475ea0 (CALL), ...
- twins: none

## C032
kind: function
address: 0x008b0c30
candidate name: `CSWGuiInGameItemEntry::SetExtent`
facts:
- size 654 B, RET 4
- callees: 0x00479ab0 (CALL), 0x0091c860 (CALL), 0x00919f10 _strlen (CALL), 0x00416fa0 CSWGuiTextParams::SetAlignment (CALL), 0x00414d80 CSWGuiBorder::GetInnerExtent (CALL), 0x0040e360 (CALL)
- indirect: 0x008b0ccf slot 0x4, 0x008b0cea slot 0x4, 0x008b0d05 slot 0x4, 0x008b0e14 slot 0x4, 0x008b0e56 slot 0x4, ...
- twins: none

## C033
kind: function
address: 0x00745770
candidate name: `CSWGuiMainInterfaceChar::DrawEffects`
facts:
- size 99 B, RET 8
- callers 2: 0x0074b3f0, 0x00756b70
- callees: 0x007457e0 (CALL)
- twins: none

## C034
kind: function
address: 0x008b29e0
candidate name: `CSWGuiContainer::SetContainer`
facts:
- size 1410 B, RET 8
- callers 2: 0x007cacb0, 0x008b2850
- callees: 0x0084a640 (CALL), 0x0051c8b0 CServerExoApp::ClientToServerObjectId (CALL), 0x0051c160 CServerExoApp::GetPlaceableByGameObjectID (CALL), 0x00840ef0 (CALL), 0x00606160 CItemRepository::ItemL
- indirect: 0x008b2bef slot 0xa0, 0x008b2ece slot 0x88, 0x008b2ee5 slot 0x8, 0x008b2f07 slot 0x8
- strings: 0x0099d780 "UseSearch"
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ, 0x00a1b4d0 READ
- twins: none

## C035
kind: function
address: 0x00411f30
candidate name: `CSWGuiManager::GetDialogMargins`
facts:
- size 31 B, RET 8
- callers 6: 0x00747210, 0x008bb5d0, 0x008bb8d0, 0x008bba80, 0x008bc3e0, ...
- twins: none

## C036
kind: function
address: 0x00453860
candidate name: `MdlNode::ControlDispatch`
facts:
- size 679 B, RET 16
- callers 4: 0x00456be0, 0x00463900, 0x004dfed0, 0x004fbbd0
- callees: 0x004def00 (CALL), 0x004544e0 (CALL), 0x004df010 (CALL), 0x004544b0 (CALL), 0x004df050 (CALL), 0x00454420 (CALL), ...
- twins: none
