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

## C13
kind: function
address: 0x0087a550
candidate name: `SendContainerClose`
facts:
- size 106 B, RET 8
- callers 1: 0x008b2850
- callees: 0x005e3f20 (CALL), 0x00893750 (CALL), 0x005e4130 (CALL), 0x005e47e0 (CALL), 0x0087a350 (CALL)
- twins: none

## C14
kind: global
address: 0x009f60ec
candidate name: `AddrPathFlag60EC`
facts:
- referenced from 2 function(s)
- in 0x004843c0: return (DAT_009f60bc & DAT_00a32df8) == DAT_009f60bc & _DAT_009f60ec;
- in 0x004845e0: DAT_009f6104 = (uVar1 & DAT_00a32df8) == uVar1 & DAT_009f62dc & _DAT_009f60ec;

## C15
kind: global
address: 0x00987a1c
candidate name: `ButtonVtable`
facts:
- referenced from 2 function(s)
- in 0x0041a3d0 (CSWGuiButton::Destructor): 0041a3f9 c7001c7a9800 MOV dword ptr [EAX],0x987a1c
- in 0x0041a510: 0041a548 c7001c7a9800 MOV dword ptr [EAX],0x987a1c

## C16
kind: function
address: 0x00417800
candidate name: `GuiTextObjRefresh`
facts:
- size 61 B, RET 0
- callers 6: 0x00416cc0 CSWGuiTextParams::SetFont, 0x00416e30 CSWGuiTextParams::SetText, 0x00416ea0 CSWGuiTextParams::SetStrRef, 0x00416fa0 CSWGuiTextParams::SetAlignment, 0x00417340, ...
- indirect: 0x00417837 slot 0x4c
- twins: none

## C17
kind: function
address: 0x0074e0b0
candidate name: `AddrIconResolver`
facts:
- size 1667 B, RET 24
- callers 1: 0x0074dc80
- callees: 0x00710eb0 (CALL), 0x0051c0e0 CServerExoApp::GetItemByGameObjectID (CALL), 0x00615cc0 CSWSItem::GetIcon (CALL), 0x00710ca0 (CALL), 0x006c1450 (CALL), 0x006a20f0 (CALL), ...
- strings: 0x0099df14 "i_stancedefm", 0x0099df24 "i_stancedef", 0x0099df30 "i_swtchwpn", ...
- globals: 0x00a1b4d0 READ, 0x00a1b4d0 READ, 0x00a1b4d0 READ
- twins: none

## C18
kind: global
address: 0x009876b4
candidate name: `TextVtable`
facts:
- referenced from 2 function(s)
- in 0x004171a0: 0041720b c700b4769800 MOV dword ptr [EAX],0x9876b4
- in 0x00417290: 004172bb c700b4769800 MOV dword ptr [EAX],0x9876b4

## C19
kind: function
address: 0x00751710
candidate name: `HudTargetInvoke`
facts:
- size 62 B, RET 8
- callers 4: 0x007469f0, 0x0074d930, 0x007516b0, 0x007b12c0 CClientExoAppInternal::HandleInputEvent
- callees: 0x007523e0 (CALL), 0x007445d0 (CALL)
- twins: none

## C20
kind: global
address: 0x009f4394
candidate name: `AddrGrassAlphaCutoff`
facts:
- referenced from 8 function(s)
- in 0x0040ee40 (CSWGuiPanel::StartLoadFromLayout): local_18 = (float)DAT_009f42a8 / (float)DAT_009f4394;
- in 0x0040f620 (CSWGuiPanel::InitControl): local_c = (float)(int)*(short *)(iVar6 + 0x6e) / (float)DAT_009f4394;
- in 0x00747210: local_178 = (float)(int)*(short *)(param_2 + 0x6e) / (float)DAT_009f4394;

## C21
kind: function
address: 0x0073a130
candidate name: `FnListRemove`
facts:
- size 196 B, RET 4
- callers 1: 0x0064dde0
- callees: 0x0073a560 (CALL), 0x009196fd operator delete (CALL)
- strings: 0x0099d39c "CExoLinkedListInternal::Remove() Position paramter is NULL"
- globals: 0x00a1b488 READ
- twins: none

## C22
kind: global
address: 0x009a7d30
candidate name: `kConstVA`
facts:
- referenced from 7 function(s)
- in 0x008baca0 (CSWGuiDialogLetterbox::SetTop): 008bace9 dc35307d9a00 FDIV double ptr [0x009a7d30]
- in 0x008bada0 (CSWGuiDialogLetterbox::SetBottom): 008bade9 dc35307d9a00 FDIV double ptr [0x009a7d30]
- in 0x008baf10 (CSWGuiDialogLetterbox::SetMiddle): 008baf6e dc35307d9a00 FDIV double ptr [0x009a7d30]

## C23
kind: global
address: 0x009862a0
candidate name: `AddrSlotGlDisable`
facts:
- referenced from 133 function(s)
- in 0x004113a0: 00411501 ff15a0629800 CALL dword ptr [0x009862a0]
- in 0x00421cf0: 00421d3b ff15a0629800 CALL dword ptr [0x009862a0]
- in 0x004265e0 (CAurTextureBasic::glImage): 00426809 ff15a0629800 CALL dword ptr [0x009862a0]

## C24
kind: global
address: 0x00986268
candidate name: `AddrSlotGlEnable`
facts:
- referenced from 132 function(s)
- in 0x00409e10: 00409e40 ff1568629800 CALL dword ptr [0x00986268]
- in 0x004113a0: 00411487 ff1568629800 CALL dword ptr [0x00986268]
- in 0x00421cf0: 00421d24 ff1568629800 CALL dword ptr [0x00986268]
