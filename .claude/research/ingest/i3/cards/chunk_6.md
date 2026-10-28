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

## C061
kind: function
address: 0x00479b60
candidate name: `LoadAlternateFonts`
facts:
- size 305 B, RET 4
- callers 2: 0x00479ca0, 0x0047a400
- callees: 0x0091d6f0 (CALL), 0x004274e0 (CALL), 0x004741e0 (CALL), 0x00422a70 (CALL)
- strings: 0x0098c018 "d3xfnt_d16x16b", 0x0098c028 "d2xfnt_d16x16b", 0x0098c038 "fnt_d16x16b", ...
- twins: none

## C062
kind: function
address: 0x0047a510
candidate name: `CAurGUIStringInternal::GetFontPixelHeight`
facts:
- size 84 B, RET 0
- callees: 0x00479ab0 (CALL), 0x0091c860 (CALL)
- indirect: 0x0047a533 slot 0x38
- twins: none

## C063
kind: function
address: 0x0075bd50
candidate name: `CSWGuiMessageBox::SetMessage`
facts:
- size 132 B, RET 8
- callers 1: 0x00898dc0
- callees: 0x0071ebd0 (CALL), 0x00416e30 CSWGuiTextParams::SetText (CALL), 0x00416e10 CSWGuiTextParams::ReSetFont (CALL), 0x0075c080 (CALL), 0x00733780 CExoString::Destructor (CALL)
- globals: 0x00a1b4b0 READ
- twins: none

## C064
kind: function
address: 0x00511fb0
candidate name: `CSWGuiMainInterfaceChar::DrawEffects`
facts:
- size 368 B, RET 4
- callers 1: 0x00511f50
- callees: 0x00475ab0 (CALL), 0x004762f0 (CALL), 0x00476370 (CALL), 0x005122e0 (CALL), 0x00452830 (CALL), 0x00475ea0 (CALL), ...
- twins: none

## C065
kind: function
address: 0x00411b20
candidate name: `CSWGuiPanel::StartLoadFromLayout`
facts:
- size 948 B, RET 4
- callers 1: 0x004113a0
- callees: 0x00919723 operator new (CALL), 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x00419740 (CALL), 0x0040fd80 CSWGuiPanel::AddControl (CALL), 0x00710840 (CALL), 0x00416960 CSWGuiTextParams::Con
- indirect: 0x00411d69 slot 0x9c, 0x00411dde slot 0x4, 0x00411e9b slot 0x4, 0x00411ec1 slot 0x34
- strings: 0x00987510 "border1b"
- twins: none

## C066
kind: function
address: 0x00429f00
candidate name: `CAurGLBufferPool::LockSlot`
facts:
- size 275 B, RET 16
- callees: 0x0091d288 operator new[] (CALL)
- indirect: 0x00429f4e unresolved
- twins: none

## C067
kind: function
address: 0x00783d80
candidate name: `CSWSMessage::HandlePlayerToServerGuiContainerMessage`
facts:
- size 49 B, RET 4
- callers 4: 0x00781be0 CClientExoAppInternal::MainLoop, 0x007ac470 CClientExoAppInternal::ProcessInput, 0x00839ad0, 0x0083a5b0
- callees: 0x007335f0 CExoString::CExoString_copy (CALL)
- twins: none

## C068
kind: function
address: 0x00652810
candidate name: `Global::AurGrassMaterialInUse`
facts:
- size 286 B, RET 8
- callers 2: 0x006527c0, 0x00652810
- callees: 0x0044c8a0 (CALL), 0x00652810 (CALL)
- twins: none

## C069
kind: global
address: 0x0099d39c
candidate name: `CExoLinkedListInternal::Remove`
facts:
- referenced from 1 function(s)
- in 0x0073a130: 0073a13f 689cd39900 PUSH 0x99d39c

## C070
kind: function
address: 0x007224c0
candidate name: `CERFString::Destructor`
facts:
- size 43 B, RET 0
- callers 3: 0x00720fc0, 0x007216c0 CERFFile::WriteResource, 0x00721bb0 CERFFile::WriteResource_2
- twins: none

## C071
kind: function
address: 0x00479b10
candidate name: `CSWGuiContainer::CSWGuiContainer`
facts:
- size 78 B, RET 0
- callers 3: 0x0047aca0, 0x0047af30, 0x0047b500
- twins: none

## C072
kind: function
address: 0x00729ed0
candidate name: `CRes2DA::CRes2DA`
facts:
- size 81 B, RET 0
- callers 1: 0x0071d360
- callees: 0x007110d0 CRes::Constructor (CALL)
- twins: none
