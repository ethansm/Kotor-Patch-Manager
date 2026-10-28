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

## C25
kind: global
address: 0x009f5d50
candidate name: `AddrShadowDist`
facts:
- referenced from 2 function(s)
- in 0x004687e0: if ((float10)_DAT_009f5d50 < fVar5) goto LAB_00468855;
- in 0x00468b50: if ((float10)_DAT_009f5d50 < fVar4) goto LAB_00468bad;

## C26
kind: function
address: 0x008a2320
candidate name: `OptionsSetDescription`
facts:
- size 461 B, RET 4
- callers 1: 0x008a2210
- callees: 0x00416ea0 CSWGuiTextParams::SetStrRef (CALL), 0x007335f0 CExoString::CExoString_copy (CALL), 0x0041d290 (CALL), 0x00919f10 _strlen (CALL), 0x004178b0 (CALL), 0x0041fd90 (CALL), ...
- indirect: 0x008a23dd slot 0x8, 0x008a240e slot 0x4
- twins: none

## C27
kind: function
address: 0x004184b0
candidate name: `SceneGetModel`
facts:
- size 54 B, RET 4
- callers 3: 0x0077ec20, 0x008cce70 CSWGuiUpgrade::Draw, 0x008cd2c0 CSWGuiUpgrade::Update3DView
- twins: none

## C28
kind: global
address: 0x009f42a8
candidate name: `ScreenH`
facts:
- referenced from 76 function(s)
- in 0x00407920: FUN_00409750(DAT_009f42a4,DAT_009f42a8,DAT_009f42b0,DAT_00a1b4d4 == 2);
- in 0x0040a750 (MainWndProc): local_18 = param_4 & 0xffff | ((DAT_009f42a8 + -1) - (param_4 >> 0x10)) * 0x10000;
- in 0x0040be50: DAT_009f42c4 = DAT_009f42a8;

## C29
kind: global
address: 0x00996088
candidate name: `AddrSlotGlDisable`
facts:
- referenced from 4 function(s)
- in 0x005b2430 (CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionIncrease): local_28 = (undefined2)DAT_00996088;
- in 0x005b2560 (CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionIncrease): if ((((*(ushort *)(puVar2 + 2) == DAT_00996088) && (uVar3 = FUN_005e4e60(0), uVar3 == uVar1))
- in 0x00677b50 (Global::ExecuteCommandEffectBlasterDeflectionIncrease): local_2c = (undefined2)DAT_00996088;

## C30
kind: global
address: 0x009878bc
candidate name: `LabelVtable`
facts:
- referenced from 76 function(s)
- in 0x00419740: 00419778 c700bc789800 MOV dword ptr [EAX],0x9878bc
- in 0x00420450: 0042049e c701bc789800 MOV dword ptr [ECX],0x9878bc
- in 0x00741940: 007420c7 c78594feffffbc789800 MOV dword ptr [EBP + 0xfffffe94],0x9878bc

## C31
kind: global
address: 0x009f5da8
candidate name: `GenderReq2Val`
facts:
- referenced from 6 function(s)
- in 0x0043bb50: if (((iVar2 == 0) || (DAT_009f48fa == '\0')) || (DAT_009f5da8 == 0)) {
- in 0x00468de0: if ((((DAT_009f5da8 == 0) || (cVar1 = FUN_004844c0(), cVar1 == '\0')) ||
- in 0x0046cf20: if ((((DAT_009f5da8 != 0) && (cVar2 = FUN_004844c0(), cVar2 != '\0')) &&

## C32
kind: global
address: 0x009f42a4
candidate name: `ScreenW`
facts:
- referenced from 72 function(s)
- in 0x00407920: FUN_00409750(DAT_009f42a4,DAT_009f42a8,DAT_009f42b0,DAT_00a1b4d4 == 2);
- in 0x0040be50: DAT_009f42c0 = DAT_009f42a4;
- in 0x0040cc70 (CExoMoviePlayerInternal::StartMovie): PatBlt(local_98,0,0,DAT_009f42a4,DAT_009f42a8,0x42);

## C33
kind: function
address: 0x0046a980
candidate name: `CSWGuiMainCharGen::BeginPlayGame`
facts:
- size 671 B, RET 0
- callees: 0x0000000e (CALL), 0x00000060 (CALL), 0x0000005a (CALL), 0x00000059 (CALL), 0x0000001c (CALL)
- imports: glDisable (OPENGL32.DLL), glEnable (OPENGL32.DLL), glFogf (OPENGL32.DLL), ...
- twins: none

## C34
kind: function
address: 0x00417800
candidate name: `kAddrResrefInit`
facts:
- size 61 B, RET 0
- callers 6: 0x00416cc0 CSWGuiTextParams::SetFont, 0x00416e30 CSWGuiTextParams::SetText, 0x00416ea0 CSWGuiTextParams::SetStrRef, 0x00416fa0 CSWGuiTextParams::SetAlignment, 0x00417340, ...
- indirect: 0x00417837 slot 0x4c
- twins: none

## C35
kind: global
address: 0x0099a8b8
candidate name: `AddrPathC60FC`
facts:
- referenced from 15 function(s)
- in 0x00568bd0 (CSWSCreature::AddToArea): if (*(short *)(*(int *)(param_1 + 0x1198) + 0xe0) != DAT_0099a8b8) {
- in 0x0056a230 (CSWSCreature::HandleHPRegeneration): if (*(short *)(param_1[0x466] + 0xe0) == DAT_0099a8b8) {
- in 0x0057eca0 (CSWSCreature::GetMaxForcePoints): if (*(short *)(*(int *)(param_1 + 0x1198) + 0xe0) == DAT_0099a8b8) {

## C36
kind: global
address: 0x0099c884
candidate name: `KeyboardDeviceGlobal`
facts:
- referenced from 6 function(s)
- in 0x0072c7e0: FUN_00732c40(local_100 + DAT_0099c884 * 3);
- in 0x00732960: if (param_2 == DAT_0099c884) {
- in 0x00732aa0: if (param_1 == DAT_0099c884) {
