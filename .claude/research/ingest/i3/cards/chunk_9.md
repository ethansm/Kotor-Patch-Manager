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

## C097
kind: function
address: 0x00416720
candidate name: `CSWGuiBorder::Load`
facts:
- size 372 B, RET 12
- callees: 0x00734010 (CALL), 0x00718960 CResGFF::GetStructFromStruct (CALL), 0x00710840 (CALL), 0x00718e40 CResGFF::ReadFieldCResRef (CALL), 0x00416090 (CALL), 0x00718c80 CResGFF::ReadFieldINT (CALL)
- strings: 0x00987660 "ROTATESTYLE", 0x0098766c "ROTATE", 0x00987680 "FLIPSTYLE", ...
- twins: none

## C098
kind: function
address: 0x007457e0
candidate name: `CSWGuiMainInterfaceChar::DrawStack`
facts:
- size 813 B, RET 12
- callers 1: 0x00745770
- callees: 0x004792c0 (CALL), 0x00479460 GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected) (CALL), 0x00479ab0 (CALL), 0x004799c0 (CALL), 0x004793f0 (CALL)
- indirect: 0x00745a7b slot 0x4, 0x00745a8f slot 0x38, 0x00745afb slot 0x4
- twins: none

## C099
kind: function
address: 0x0046d230
candidate name: `CAurScene::Animate`
facts:
- size 331 B, RET 4
- indirect: 0x0046d2ad slot 0x14, 0x0046d31a slot 0x14, 0x0046d367 slot 0x14
- twins: none

## C100
kind: function
address: 0x008bc100
candidate name: `DialogTop::SetReply`
facts:
- size 292 B, RET 0
- callees: 0x008ba420 (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x008bab20 CSWGuiDialogLetterbox::SetExtentSpecial (CALL)
- indirect: 0x008bc146 slot 0x8, 0x008bc19a slot 0x4
- twins: none

## C101
kind: function
address: 0x00420180
candidate name: `CSWGuiListBox::LoadProtoItem`
facts:
- size 691 B, RET 8
- callers 1: 0x0041ffd0 CSWGuiListBox::Load
- callees: 0x00718960 CResGFF::GetStructFromStruct (CALL), 0x00718c80 CResGFF::ReadFieldINT (CALL), 0x00919723 operator new (CALL), 0x00419740 (CALL), 0x00414a50 CSWGuiBorder::Constructor (CALL), 0x00
- strings: 0x00987ed4 "CONTROLTYPE", 0x00987ee0 "PROTOITEM"
- twins: none

## C102
kind: function
address: 0x0074b3a0
candidate name: `CSWGuiMainInterface::HitCheckMouse`
facts:
- size 68 B, RET 8
- callees: 0x007433b0 (CALL), 0x0040eae0 CSWGuiPanel::HitCheckMouse (CALL)
- twins: none

## C103
kind: function
address: 0x0041e790
candidate name: `CSWGuiListBox::IsOversized`
facts:
- size 97 B, RET 0
- callers 10: 0x0041d940, 0x0041ea50, 0x0041f980, 0x00420850, 0x00420c00, ...
- twins: none

## C104
kind: function
address: 0x00479ca0
candidate name: `CAurGUIStringInternal::Constructor`
facts:
- size 538 B, RET 24
- callers 1: 0x0047ec30
- callees: 0x0047cd10 (CALL), 0x0091d67e (CALL), 0x00449230 (CALL), 0x004741e0 (CALL), 0x004274e0 (CALL), 0x00479b60 (CALL), ...
- twins: none

## C105
kind: function
address: 0x008b1ea0
candidate name: `CSWGuiContainer::CSWGuiContainer`
facts:
- size 1863 B, RET 4
- callers 1: 0x007be4c0
- callees: 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x00419740 (CALL), 0x0041ce90 (CALL), 0x0041a510 (CALL), 0x00710840 (CALL), 0x0040ee40 CSWGuiPanel::StartLoadFromLayout (CALL), ...
- indirect: 0x008b22ea slot 0x4, 0x008b2358 slot 0x4, 0x008b23c9 slot 0x4, 0x008b248f slot 0x88
- strings: 0x009a7790 "BTN_GIVEITEMS", 0x009a77a0 "LBL_CANCEL", 0x009a77ac "LBL_X", ...
- twins: none

## C106
kind: function
address: 0x0041ea50
candidate name: `CSWGuiListBox::OrganizeControls`
facts:
- size 1823 B, RET 0
- callers 8: 0x0041d710 CSWGuiListBox::SetExtent, 0x0041d940, 0x0041e870, 0x0041e9d0 CSWGuiListBox::SetPadding, 0x0041fc30, ...
- callees: 0x0041c760 (CALL), 0x0041c7c0 (CALL), 0x0041c810 (CALL), 0x0041f170 (CALL), 0x0041e790 (CALL), 0x004178b0 (CALL), ...
- indirect: 0x0041eb68 slot 0x4, 0x0041eb87 slot 0x34, 0x0041edb8 slot 0x50, 0x0041f025 slot 0x4, 0x0041f0fb slot 0x4, ...
- twins: none
