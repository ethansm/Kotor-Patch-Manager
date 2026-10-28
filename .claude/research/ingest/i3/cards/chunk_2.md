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

## C013
kind: function
address: 0x008b0c30
candidate name: `CSWGuiListBox::ClearItems`
facts:
- size 654 B, RET 4
- callees: 0x00479ab0 (CALL), 0x0091c860 (CALL), 0x00919f10 _strlen (CALL), 0x00416fa0 CSWGuiTextParams::SetAlignment (CALL), 0x00414d80 CSWGuiBorder::GetInnerExtent (CALL), 0x0040e360 (CALL)
- indirect: 0x008b0ccf slot 0x4, 0x008b0cea slot 0x4, 0x008b0d05 slot 0x4, 0x008b0e14 slot 0x4, 0x008b0e56 slot 0x4, ...
- twins: none

## C014
kind: function
address: 0x00464d10
candidate name: `Scene::AddObject`
facts:
- size 33 B, RET 4
- indirect: 0x00464d29 slot 0xb0
- twins: none

## C015
kind: function
address: 0x00479b10
candidate name: `CAurGUIStringInternal::RefreshResolutionTextScalar`
facts:
- size 78 B, RET 0
- callers 3: 0x0047aca0, 0x0047af30, 0x0047b500
- twins: none

## C016
kind: function
address: 0x00745560
candidate name: `CSWGuiProgressBar::Draw`
facts:
- size 519 B, RET 4
- callers 1: 0x0074b3f0
- callees: 0x0091c860 (CALL)
- indirect: 0x0074568c slot 0x4, 0x00745711 slot 0x4, 0x0074575f slot 0x4
- twins: none

## C017
kind: function
address: 0x0065afd0
candidate name: `CSWGuiListBox::ClearItems`
facts:
- size 289 B, RET 8
- callers 1: 0x0065f8a0
- callees: 0x00632ef0 (CALL), 0x005e36a0 (CALL), 0x005e3de0 (CALL), 0x005e3e40 (CALL), 0x0051c080 CServerExoApp::GetObjectArray (CALL), 0x0053dfb0 CGameObjectArray::GetGameObject (CALL), ...
- indirect: 0x0065b08d slot 0x48, 0x0065b0d6 slot 0x38
- globals: 0x00a1b4a4 READ
- twins: none

## C018
kind: function
address: 0x00750c10
candidate name: `CSWGuiMainInterface::InitializeMap`
facts:
- size 577 B, RET 0
- callers 2: 0x00750790, 0x007ceec0
- callees: 0x0051bd90 CServerExoApp::GetModule (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), 0x00710930 (CALL), 0x00734010 (CALL), 0x00734270 CExoString::Format (CALL), 0x00710810 (CALL), 
- indirect: 0x00750d6c slot 0x4
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## C019
kind: global
address: 0x00987bb4
candidate name: `CSWGuiProgressBar`
facts:
- referenced from 6 function(s)
- in 0x0041b7f0: 0041b828 c700b47b9800 MOV dword ptr [EAX],0x987bb4
- in 0x0041b8a0: 0041b8c9 c700b47b9800 MOV dword ptr [EAX],0x987bb4
- in 0x0074a780: 0074a7bd c701b47b9800 MOV dword ptr [ECX],0x987bb4

## C020
kind: function
address: 0x00727710
candidate name: `CExoBifFile::ReadHeader`
facts:
- size 537 B, RET 4
- callees: 0x00734010 (CALL), 0x00739970 (CALL), 0x00919723 operator new (CALL), 0x00739990 (CALL), 0x009196fd operator delete (CALL), 0x00739b20 (CALL)
- indirect: 0x00727737 slot 0x4, 0x007277d3 slot 0x14, 0x00727814 slot 0x14, 0x00727856 slot 0x14, 0x007278d6 slot 0x14
- twins: none

## C021
kind: function
address: 0x0041f170
candidate name: `CSWGuiListBox::OrganizeUnequal`
facts:
- size 1659 B, RET 0
- callers 1: 0x0041ea50
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x00733780 CExoString::Destructor (CALL), 0x0041c760 (CALL), 0x0041c810 (CALL), 0x0041c7c0 (CALL)
- indirect: 0x0041f1ed slot 0x34, 0x0041f241 slot 0x34, 0x0041f4a5 slot 0x4, 0x0041f51a slot 0x4, 0x0041f5c9 slot 0x4
- twins: none

## C022
kind: function
address: 0x0071b8b0
candidate name: `CERFString::Destructor`
facts:
- size 526 B, RET 12
- callers 37: 0x00573950 CSWSCreature::ReadItemsFromGff, 0x0057ff50 CSWSCreature::UpdateAppearanceDependantInfo, 0x00664ae0, 0x006a2170, 0x006bc9a0 CSWSCreatureStats::GetEffectLinkImmunity, ...
- callees: 0x0091cf18 (CALL), 0x0091c7ad (CALL), 0x00733a60 (CALL), 0x00919f10 _strlen (CALL), 0x00734010 (CALL)
- twins: none

## C023
kind: function
address: 0x00449320
candidate name: `CExoPtrList::SetAllocated`
facts:
- size 151 B, RET 4
- callers 5: 0x004491c0, 0x00449230, 0x00474030, 0x00474370, 0x00507fa0
- callees: 0x0091d288 operator new[] (CALL), 0x0091c6aa operator delete[] (CALL)
- twins: none

## C024
kind: function
address: 0x007457e0
candidate name: `CSWGuiListBox::LoadProtoItem`
facts:
- size 813 B, RET 12
- callers 1: 0x00745770
- callees: 0x004792c0 (CALL), 0x00479460 GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected) (CALL), 0x00479ab0 (CALL), 0x004799c0 (CALL), 0x004793f0 (CALL)
- indirect: 0x00745a7b slot 0x4, 0x00745a8f slot 0x38, 0x00745afb slot 0x4
- twins: none
