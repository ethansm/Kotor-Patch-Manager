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

## C49
kind: function
address: 0x00418360
candidate name: `SceneAddModel`
facts:
- size 69 B, RET 8
- callers 13: 0x00413650, 0x0084c3a0 CSWGuiInGameCharacter::Constructor, 0x00880740, 0x008973d0, 0x008988b0, ...
- callees: 0x00734010 (CALL), 0x00462320 (CALL), 0x004183b0 (CALL)
- twins: none

## C50
kind: global
address: 0x009f4394
candidate name: `GuiRefH`
facts:
- referenced from 8 function(s)
- in 0x0040ee40 (CSWGuiPanel::StartLoadFromLayout): local_18 = (float)DAT_009f42a8 / (float)DAT_009f4394;
- in 0x0040f620 (CSWGuiPanel::InitControl): local_c = (float)(int)*(short *)(iVar6 + 0x6e) / (float)DAT_009f4394;
- in 0x00747210: local_178 = (float)(int)*(short *)(param_2 + 0x6e) / (float)DAT_009f4394;

## C51
kind: global
address: 0x0099dcc4
candidate name: `HudVtable`
facts:
- referenced from 2 function(s)
- in 0x00747210: 0074725d c701c4dc9900 MOV dword ptr [ECX],0x99dcc4
- in 0x0074ad90: 0074adbb c700c4dc9900 MOV dword ptr [EAX],0x99dcc4

## C52
kind: function
address: 0x0046a980
candidate name: `Scene::DoFog`
facts:
- size 671 B, RET 0
- callees: 0x0000000e (CALL), 0x00000060 (CALL), 0x0000005a (CALL), 0x00000059 (CALL), 0x0000001c (CALL)
- imports: glDisable (OPENGL32.DLL), glEnable (OPENGL32.DLL), glFogf (OPENGL32.DLL), ...
- twins: none

## C53
kind: global
address: 0x00996088
candidate name: `EffDeflectInc`
facts:
- referenced from 4 function(s)
- in 0x005b2430 (CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionIncrease): local_28 = (undefined2)DAT_00996088;
- in 0x005b2560 (CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionIncrease): if ((((*(ushort *)(puVar2 + 2) == DAT_00996088) && (uVar3 = FUN_005e4e60(0), uVar3 == uVar1))
- in 0x00677b50 (Global::ExecuteCommandEffectBlasterDeflectionIncrease): local_2c = (undefined2)DAT_00996088;

## C54
kind: global
address: 0x0099a8b8
candidate name: `GenderReq2Val`
facts:
- referenced from 15 function(s)
- in 0x00568bd0 (CSWSCreature::AddToArea): if (*(short *)(*(int *)(param_1 + 0x1198) + 0xe0) != DAT_0099a8b8) {
- in 0x0056a230 (CSWSCreature::HandleHPRegeneration): if (*(short *)(param_1[0x466] + 0xe0) == DAT_0099a8b8) {
- in 0x0057eca0 (CSWSCreature::GetMaxForcePoints): if (*(short *)(*(int *)(param_1 + 0x1198) + 0xe0) == DAT_0099a8b8) {

## C55
kind: function
address: 0x00466700
candidate name: `Scene::DoFadeBuckets`
facts:
- size 2471 B, RET 0
- callees: 0x00463ed0 (CALL), 0x00427970 (CALL), 0x0049b760 (CALL), 0x0000000e (CALL), 0x0000005c (CALL), 0x00000039 (CALL), ...
- indirect: 0x00466770 slot 0x8, 0x00466998 slot 0x60, 0x00466b48 slot 0x64, 0x00466c23 slot 0x64, 0x00466d22 slot 0x60, ...
- imports: glColorMaterial (OPENGL32.DLL), glDisable (OPENGL32.DLL), glDisableClientState (OPENGL32.DLL), ...
- twins: none

## C56
kind: function
address: 0x0041a510
candidate name: `ButtonCtor`
facts:
- size 134 B, RET 0
- callers 80: 0x0041a330, 0x00420180, 0x00741420, 0x00747210, 0x0074a500, ...
- callees: 0x00418570 (CALL), 0x00414a50 CSWGuiBorder::Constructor (CALL), 0x004171a0 (CALL)
- twins: none

## C57
kind: global
address: 0x009860d4
candidate name: `SleepIat`
facts:
- referenced from 15 function(s)
- in 0x00407920: 00408479 ff15d4609800 CALL dword ptr [0x009860d4]
- in 0x0040cc70 (CExoMoviePlayerInternal::StartMovie): 0040d3b0 ff15d4609800 CALL dword ptr [0x009860d4]
- in 0x0040d430 (CExoMoviePlayer thread proc): 0040d577 ff15d4609800 CALL dword ptr [0x009860d4]

## C58
kind: global
address: 0x009f42b4
candidate name: `BorderVtable`
facts:
- referenced from 21 function(s)
- in 0x0084c3a0 (CSWGuiInGameCharacter::Constructor): if (DAT_009f42b4 != 0) {
- in 0x0084e6f0 (CSWGuiInGameCharacter::SetStats): if (DAT_009f42b4 != 0) {
- in 0x0084fc00 (CSWGuiInGameCharacter::AttachModel): if (DAT_009f42b4 != 0) {

## C59
kind: global
address: 0x00986268
candidate name: `AddrPathFlag62DC`
facts:
- referenced from 132 function(s)
- in 0x00409e10: 00409e40 ff1568629800 CALL dword ptr [0x00986268]
- in 0x004113a0: 00411487 ff1568629800 CALL dword ptr [0x00986268]
- in 0x00421cf0: 00421d24 ff1568629800 CALL dword ptr [0x00986268]

## C60
kind: global
address: 0x009f60f0
candidate name: `AddrPathN`
facts:
- referenced from 2 function(s)
- in 0x00484480: if ((iVar1 == 0) || (DAT_009f60f0 != 1)) {
- in 0x004844c0: if (DAT_009f60f0 < 2) {
