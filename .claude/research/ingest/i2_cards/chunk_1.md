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

## C01
kind: function
address: 0x00751750
candidate name: `HudInvokeGroup`
facts:
- size 833 B, RET 8
- callers 4: 0x007469f0, 0x0074d930, 0x007516b0, 0x007b12c0 CClientExoAppInternal::HandleInputEvent
- callees: 0x007523e0 (CALL), 0x00754e20 (CALL), 0x0073fb90 CClientExoApp::GetSWParty (CALL), 0x007e5d80 (CALL), 0x0073f4d0 CClientExoApp::GetGameObject (CALL), 0x007f2500 CSWCObject::ClearAllQueuedCo
- indirect: 0x007518a9 slot 0xc, 0x007519a5 slot 0xc
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## C02
kind: global
address: 0x0098c45c
candidate name: `SceneCameraVtable`
facts:
- referenced from 2 function(s)
- in 0x0047ecd0: 0047ed1e c7025cc49800 MOV dword ptr [EDX],0x98c45c
- in 0x0047f0a0: 0047f0c9 c7005cc49800 MOV dword ptr [EAX],0x98c45c

## C03
kind: function
address: 0x008faca0
candidate name: `CSWGuiMainCharGen::BeginPlayGame`
facts:
- size 1151 B, RET 0
- callers 1: 0x00907340 CSWGuiMainCharGen::BeginPlayGame
- callees: 0x00740ee0 CClientExoApp::BusyWaitMenuMusic (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x007d0af0 (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), 0x00401bc0 (CALL), 0x
- indirect: 0x008faf49 slot 0x10, 0x008faff4 slot 0x10
- strings: 0x00992da0 "Module", 0x0099d6ac "GBL_GAME_COMPLETE", 0x009a5adc "%c%s.%s %s"
- twins: none

## C04
kind: global
address: 0x00987a1c
candidate name: `AddrSlotGlGetIntegerv`
facts:
- referenced from 2 function(s)
- in 0x0041a3d0 (CSWGuiButton::Destructor): 0041a3f9 c7001c7a9800 MOV dword ptr [EAX],0x987a1c
- in 0x0041a510: 0041a548 c7001c7a9800 MOV dword ptr [EAX],0x987a1c

## C05
kind: global
address: 0x009a3d34
candidate name: `SceneViewVtable`
facts:
- referenced from 22 function(s)
- in 0x0084c3a0 (CSWGuiInGameCharacter::Constructor): 0084c80c c700343d9a00 MOV dword ptr [EAX],0x9a3d34
- in 0x0084dde0: 0084de09 c700343d9a00 MOV dword ptr [EAX],0x9a3d34
- in 0x0084de70: 0084de99 c700343d9a00 MOV dword ptr [EAX],0x9a3d34

## C06
kind: function
address: 0x0051c8d0
candidate name: `FnPartyTable`
facts:
- size 22 B, RET 0
- callers 175: 0x00523870 CSWSArea::LoadAreaHeader, 0x0052a600 CSWSArea::SaveCreatures, 0x0052dd00 CSWSArea::AwardStealthXP, 0x0052e170 CSWSArea::SetPlayerRestrictMode, 0x00531c50 CServerExoAppInterna
- twins: 0x0051c890 (distinguishing: 0x0051c8dd: ADD EAX,0x1f0b4)

## C07
kind: global
address: 0x0098657c
candidate name: `AilQuickShutdownIat`
facts:
- referenced from 2 function(s)
- in 0x007051c0: 00705562 ff157c659800 CALL dword ptr [0x0098657c]
- in 0x00705ae0: 00705d4d ff157c659800 CALL dword ptr [0x0098657c]

## C08
kind: global
address: 0x009f62dc
candidate name: `GenderReq1Val`
facts:
- referenced from 35 function(s)
- in 0x004317d0 (GLRender::InitExtensions): if (DAT_009f62dc != 0) {
- in 0x00434d50: if (DAT_009f62dc != 0) {
- in 0x0046c5c0: if (DAT_009f62dc != 0) {

## C09
kind: global
address: 0x009a044c
candidate name: `FadePanelVtable`
facts:
- referenced from 1 function(s)
- in 0x007bc600: 007bc63e c7014c049a00 MOV dword ptr [ECX],0x9a044c

## C10
kind: global
address: 0x009a7d30
candidate name: `LoadScreenVtable`
facts:
- referenced from 7 function(s)
- in 0x008baca0 (CSWGuiDialogLetterbox::SetTop): 008bace9 dc35307d9a00 FDIV double ptr [0x009a7d30]
- in 0x008bada0 (CSWGuiDialogLetterbox::SetBottom): 008bade9 dc35307d9a00 FDIV double ptr [0x009a7d30]
- in 0x008baf10 (CSWGuiDialogLetterbox::SetMiddle): 008baf6e dc35307d9a00 FDIV double ptr [0x009a7d30]

## C11
kind: function
address: 0x0050dac0
candidate name: `CAurTriangleBin::RenderGrassPolys`
facts:
- size 3103 B, RET 12
- callers 1: 0x004664e0
- callees: 0x00484480 (CALL), 0x00484710 (CALL), 0x0000005b (CALL), 0x00510f70 (CALL), 0x00000039 (CALL), 0x00484550 (CALL), ...
- indirect: 0x0050dd3d slot 0xb8, 0x0050df1b unresolved, 0x0050dfad unresolved, 0x0050dfe4 unresolved, 0x0050e023 unresolved, ...
- strings: 0x009f7b80 "!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\nPARAM mvpMatrix[4] = { state.matrix.mvp };\nTEMP vTemp;\nTEMP vReg0, vReg1, vReg2, vReg3;\nTEMP vReg10;\nTEMP vNorm
- imports: glColor4f (OPENGL32.DLL), glDisable (OPENGL32.DLL), glDisableClientState (OPENGL32.DLL), ...
- twins: none

## C12
kind: global
address: 0x009f60fc
candidate name: `AddrPathC60FC`
facts:
- referenced from 1 function(s)
- in 0x00484550: if (DAT_009f60fc == -1) {
