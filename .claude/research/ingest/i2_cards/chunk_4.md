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

## C37
kind: global
address: 0x009a5c84
candidate name: `LoadScreenVtable`
facts:
- referenced from 1 function(s)
- in 0x008850b0: 008850fa c701845c9a00 MOV dword ptr [ECX],0x9a5c84

## C38
kind: global
address: 0x0099a8bc
candidate name: `GenderReq1Val`
facts:
- referenced from 7 function(s)
- in 0x005d3650 (CSWSCreature::CheckItemRaceRestrictions): if (*(short *)(*(int *)(param_1 + 0x1198) + 0xe0) != DAT_0099a8bc) {
- in 0x006a4260 (CSWSCreatureStats::Constructor): *(undefined2 *)(param_1 + 0xe0) = DAT_0099a8bc;
- in 0x0077a4f0: if (*(short *)(*(int *)(iVar3 + 0x310) + 0x24) == DAT_0099a8bc) {

## C39
kind: function
address: 0x0074b3f0
candidate name: `CSWGuiMainInterface::Draw`
facts:
- size 3874 B, RET 4
- callees: 0x0073fb90 CClientExoApp::GetSWParty (CALL), 0x007e5d80 (CALL), 0x0074ee90 (CALL), 0x0074fa50 (CALL), 0x0074fb20 (CALL), 0x007431d0 (CALL), ...
- indirect: 0x0074b596 slot 0x94, 0x0074b5b8 slot 0x9c, 0x0074b924 slot 0x70, 0x0074bb7a slot 0x70, 0x0074bd78 slot 0x88, ...
- twins: none

## C40
kind: global
address: 0x0098bfe0
candidate name: `TexMgr::Tick`
facts:
- referenced from 3 function(s)
- in 0x00478a20: _DAT_00a309ac = (float)((float10)local_24.dwAvailPhys / (float10)_PTR_0098bfe0);
- in 0x00745e60: local_2a0 = (local_274 / (float)_PTR_0098bfe0) * local_c;
- in 0x00753d60: local_18 = (float)local_24 / (float)_PTR_0098bfe0;

## C41
kind: global
address: 0x009f62dc
candidate name: `AddrPathFlag62DC`
facts:
- referenced from 35 function(s)
- in 0x004317d0 (GLRender::InitExtensions): if (DAT_009f62dc != 0) {
- in 0x00434d50: if (DAT_009f62dc != 0) {
- in 0x0046c5c0: if (DAT_009f62dc != 0) {

## C42
kind: global
address: 0x009875bc
candidate name: `BorderVtable`
facts:
- referenced from 2 function(s)
- in 0x00414a50 (CSWGuiBorder::Constructor): 00414abb c700bc759800 MOV dword ptr [EAX],0x9875bc
- in 0x00414c60 (CSWGuiBorder::Destructor_2): 00414c8b c700bc759800 MOV dword ptr [EAX],0x9875bc

## C43
kind: function
address: 0x004662e0
candidate name: `Scene::DoCapBucket`
facts:
- size 500 B, RET 0
- callees: 0x00427970 (CALL), 0x0049b760 (CALL), 0x00472630 TexBucketMap::operator[] (CALL), 0x004dae60 (CALL), 0x004db1f0 (CALL), 0x00473e50 (CALL)
- indirect: 0x00466411 slot 0x60, 0x00466420 slot 0x64, 0x004664a0 slot 0x60, 0x004664af slot 0x64
- twins: none

## C44
kind: global
address: 0x009f42b4
candidate name: `GuiGateGlobal`
facts:
- referenced from 21 function(s)
- in 0x0084c3a0 (CSWGuiInGameCharacter::Constructor): if (DAT_009f42b4 != 0) {
- in 0x0084e6f0 (CSWGuiInGameCharacter::SetStats): if (DAT_009f42b4 != 0) {
- in 0x0084fc00 (CSWGuiInGameCharacter::AttachModel): if (DAT_009f42b4 != 0) {

## C45
kind: function
address: 0x00468b50
candidate name: `Scene::DoGobShadowsForThisLight`
facts:
- size 644 B, RET 4
- callees: 0x004452b0 (CALL), 0x0044b8a0 (CALL), 0x00459e20 Gob::IsInCutscene (CALL), 0x00456b00 (CALL), 0x005142f0 (CALL), 0x00465c90 (CALL)
- indirect: 0x00468ba3 slot 0x64, 0x00468c22 slot 0x14, 0x00468d45 slot 0xc
- twins: none

## C46
kind: global
address: 0x0099e5ec
candidate name: `MessagesPanelVtable`
facts:
- referenced from 2 function(s)
- in 0x00757c40: 00757c8b c701ece59900 MOV dword ptr [ECX],0x99e5ec
- in 0x00758a70: 00758a9b c700ece59900 MOV dword ptr [EAX],0x99e5ec

## C47
kind: function
address: 0x00418430
candidate name: `SceneRemoveModel`
facts:
- size 127 B, RET 8
- callers 2: 0x008988b0, 0x008cd2c0 CSWGuiUpgrade::Update3DView
- callees: 0x0049b840 (CALL)
- indirect: 0x00418496 slot 0x0
- twins: none

## C48
kind: global
address: 0x0099e5ec
candidate name: `AilQuickShutdownIat`
facts:
- referenced from 2 function(s)
- in 0x00757c40: 00757c8b c701ece59900 MOV dword ptr [ECX],0x99e5ec
- in 0x00758a70: 00758a9b c700ece59900 MOV dword ptr [EAX],0x99e5ec
