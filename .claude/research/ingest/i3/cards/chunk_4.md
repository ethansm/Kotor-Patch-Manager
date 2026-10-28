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

## C037
kind: function
address: 0x0041fd90
candidate name: `CSWGuiListBox::AddControls`
facts:
- size 326 B, RET 20
- callers 28: 0x0075bf60, 0x007d3060, 0x007d4460, 0x007d5e90, 0x007da9c0, ...
- callees: 0x0041d290 (CALL), 0x004142e0 (CALL), 0x0083ea60 CExoArrayList<uint32>::Add (CALL), 0x00421720 (CALL), 0x0041ea50 (CALL)
- twins: none

## C038
kind: function
address: 0x009454b0
candidate name: `Scene::AddObject`
facts:
- size 905 B, RET 0
- callees: 0x00945860 (CALL), 0x00945990 (CALL)
- indirect: 0x0094579a slot 0xc
- twins: none

## C039
kind: function
address: 0x00411b20
candidate name: `GLRender::CreateCubeTexture`
facts:
- size 948 B, RET 4
- callers 1: 0x004113a0
- callees: 0x00919723 operator new (CALL), 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x00419740 (CALL), 0x0040fd80 CSWGuiPanel::AddControl (CALL), 0x00710840 (CALL), 0x00416960 CSWGuiTextParams::Con
- indirect: 0x00411d69 slot 0x9c, 0x00411dde slot 0x4, 0x00411e9b slot 0x4, 0x00411ec1 slot 0x34
- strings: 0x00987510 "border1b"
- twins: none

## C040
kind: function
address: 0x008b29e0
candidate name: `CPathfindInformation::ResetAtEndOfPath`
facts:
- size 1410 B, RET 8
- callers 2: 0x007cacb0, 0x008b2850
- callees: 0x0084a640 (CALL), 0x0051c8b0 CServerExoApp::ClientToServerObjectId (CALL), 0x0051c160 CServerExoApp::GetPlaceableByGameObjectID (CALL), 0x00840ef0 (CALL), 0x00606160 CItemRepository::ItemL
- indirect: 0x008b2bef slot 0xa0, 0x008b2ece slot 0x88, 0x008b2ee5 slot 0x8, 0x008b2f07 slot 0x8
- strings: 0x0099d780 "UseSearch"
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ, 0x00a1b4d0 READ
- twins: none

## C041
kind: function
address: 0x00897250
candidate name: `CSWGuiMapHider::SetMapNote`
facts:
- size 97 B, RET 4
- callers 1: 0x00895260
- callees: 0x007a1720 (CALL), 0x00561430 (CALL), 0x0058c370 (CALL)
- twins: none

## C042
kind: function
address: 0x00769f60
candidate name: `GetFontTexture`
facts:
- size 2654 B, RET 12
- callees: 0x00919723 operator new (CALL), 0x0085f7e0 (CALL), 0x0086bfd0 (CALL), 0x0086aea0 (CALL), 0x008695d0 (CALL), 0x00867f00 (CALL), ...
- indirect: 0x00769fe6 slot 0x0, 0x0076a0c8 slot 0x0, 0x0076a472 slot 0xc, 0x0076a4eb slot 0x8, 0x0076a502 slot 0x74, ...
- strings: 0x0099efe4 "CSWCCreature::LoadModel(): Failed to load creature model '%s'."
- twins: none

## C043
kind: function
address: 0x0071b8b0
candidate name: `C2DA::GetFloat`
facts:
- size 526 B, RET 12
- callers 37: 0x00573950 CSWSCreature::ReadItemsFromGff, 0x0057ff50 CSWSCreature::UpdateAppearanceDependantInfo, 0x00664ae0, 0x006a2170, 0x006bc9a0 CSWSCreatureStats::GetEffectLinkImmunity, ...
- callees: 0x0091cf18 (CALL), 0x0091c7ad (CALL), 0x00733a60 (CALL), 0x00919f10 _strlen (CALL), 0x00734010 (CALL)
- twins: none

## C044
kind: function
address: 0x00724780
candidate name: `CExoKeyTable::Load`
facts:
- size 967 B, RET 4
- callers 1: 0x00726240
- callees: 0x007107f0 (CALL), 0x00919723 operator new (CALL), 0x007294c0 (CALL), 0x009196fd operator delete (CALL), 0x007337c0 CExoString::operator= (CALL), 0x006f3d90 (CALL), ...
- indirect: 0x007248d0 slot 0x2c, 0x0072492b slot 0x0, 0x00724a22 slot 0x0
- twins: none

## C045
kind: function
address: 0x0041f7f0
candidate name: `CSWGuiListBox::OrganizeOversized`
facts:
- size 398 B, RET 0
- callers 3: 0x0041d940, 0x0041ea50, 0x00420c00
- callees: 0x0041e800 (CALL), 0x0041c7c0 (CALL), 0x0091c860 (CALL)
- indirect: 0x0041f978 slot 0x4
- twins: none

## C046
kind: function
address: 0x0047a600
candidate name: `SetString`
facts:
- size 126 B, RET 4
- callees: 0x00919f10 _strlen (CALL), 0x009196fd operator delete (CALL), 0x00919723 operator new (CALL), 0x0091cff0 (CALL)
- twins: none

## C047
kind: function
address: 0x0070b9b0
candidate name: `CExoSound::ReInitialize`
facts:
- size 129 B, RET 8
- callers 1: 0x007db680
- callees: 0x0070b8c0 (CALL), 0x0070b890 (CALL), 0x007051c0 (CALL)
- twins: none

## C048
kind: function
address: 0x0071cf80
candidate name: `C2DA::Free`
facts:
- size 273 B, RET 8
- callers 1: 0x0071c3d0
- twins: none
