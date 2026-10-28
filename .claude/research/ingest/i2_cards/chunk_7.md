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

## C73
kind: function
address: 0x00710840
candidate name: `kAddrResrefInit`
facts:
- size 34 B, RET 4
- callers 374: 0x0040ee40 CSWGuiPanel::StartLoadFromLayout, 0x00411b20, 0x00411f50, 0x00415080 CSWGuiBorder::Load, 0x00416720, ...
- callees: 0x00710eb0 (CALL)
- twins: 0x00710810, 0x00710870 (distinguishing: 0x00710854: CALL 0x00710eb0)

## C74
kind: function
address: 0x00419740
candidate name: `LabelCtor`
facts:
- size 131 B, RET 0
- callers 83: 0x00411b20, 0x00420180, 0x00741940, 0x00742150, 0x00747210, ...
- callees: 0x00418990 CSWGuiControl::Constructor (CALL), 0x00414a50 CSWGuiBorder::Constructor (CALL), 0x004171a0 (CALL)
- twins: none

## C75
kind: global
address: 0x009878bc
candidate name: `AddrPathC60FC`
facts:
- referenced from 76 function(s)
- in 0x00419740: 00419778 c700bc789800 MOV dword ptr [EAX],0x9878bc
- in 0x00420450: 0042049e c701bc789800 MOV dword ptr [ECX],0x9878bc
- in 0x00741940: 007420c7 c78594feffffbc789800 MOV dword ptr [EBP + 0xfffffe94],0x9878bc

## C76
kind: global
address: 0x009f5d38
candidate name: `AddrGrassAlphaCutoff`
facts:
- referenced from 1 function(s)
- in 0x004664e0: glAlphaFunc(DAT_00988570,_DAT_009f5d38);

## C77
kind: global
address: 0x009a8fcc
candidate name: `AutoPausePanelVtable`
facts:
- referenced from 1 function(s)
- in 0x008e1810: 008e185a c701cc8f9a00 MOV dword ptr [ECX],0x9a8fcc

## C78
kind: global
address: 0x00986128
candidate name: `AddrGmsIat`
facts:
- referenced from 2 function(s)
- in 0x00478a20: 00478a57 ff1528619800 CALL dword ptr [0x00986128]
- in 0x00711360 (CExoResMan::Constructor): 00711427 ff1528619800 CALL dword ptr [0x00986128]

## C79
kind: global
address: 0x009f5da8
candidate name: `AddrSoftShadows`
facts:
- referenced from 6 function(s)
- in 0x0043bb50: if (((iVar2 == 0) || (DAT_009f48fa == '\0')) || (DAT_009f5da8 == 0)) {
- in 0x00468de0: if ((((DAT_009f5da8 == 0) || (cVar1 = FUN_004844c0(), cVar1 == '\0')) ||
- in 0x0046cf20: if ((((DAT_009f5da8 != 0) && (cVar2 = FUN_004844c0(), cVar2 != '\0')) &&

## C80
kind: global
address: 0x009a7cbc
candidate name: `kLetterboxVtbl`
facts:
- referenced from 1 function(s)
- in 0x008ba980 (CSWGuiDialogLetterbox::Constructor): 008ba9be c701bc7c9a00 MOV dword ptr [ECX],0x9a7cbc

## C81
kind: global
address: 0x009a4ab0
candidate name: `PreviewModelInstName`
facts:
- referenced from 1 function(s)
- in 0x0086a430: 0086a5f5 68b04a9a00 PUSH 0x9a4ab0

## C82
kind: global
address: 0x009f6118
candidate name: `AddrPathC6118`
facts:
- referenced from 1 function(s)
- in 0x004847e0: if (DAT_009f6118 == 0xffffffff) {

## C83
kind: global
address: 0x009f4390
candidate name: `GuiRefW`
facts:
- referenced from 8 function(s)
- in 0x0040ee40 (CSWGuiPanel::StartLoadFromLayout): local_14 = (float)DAT_009f42a4 / (float)DAT_009f4390;
- in 0x0040f620 (CSWGuiPanel::InitControl): local_8 = (float)(int)*(short *)(iVar6 + 0x6c) / (float)DAT_009f4390;
- in 0x00747210: local_1dc = (float)(int)*(short *)(param_2 + 0x6c) / (float)DAT_009f4390;

## C84
kind: function
address: 0x00417f40
candidate name: `GuiSceneCtor`
facts:
- size 426 B, RET 0
- callers 11: 0x00413650, 0x0084c3a0 CSWGuiInGameCharacter::Constructor, 0x00880740, 0x008973d0, 0x0089cf30, ...
- callees: 0x0084a640 (CALL), 0x0046d3c0 (CALL), 0x00481ac0 (CALL)
- indirect: 0x00418042 slot 0x20, 0x00418058 slot 0x8c, 0x0041806d slot 0x4, 0x00418094 slot 0x40, 0x004180b0 slot 0x14
- twins: none
