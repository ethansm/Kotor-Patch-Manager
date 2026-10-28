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
kind: function
address: 0x0071cf80
candidate name: `IODispatcher::ReadSync_4`
facts:
- size 273 B, RET 8
- callers 1: 0x0071c3d0
- twins: none
- decomp: bool this_fn(int *param_1,uint *param_2)
    local_c = 0;
    local_10 = 0;
    local_8 = (char *)*param_1;
    do {
    if ((*param_2 <= local_c) ||
    ((((*local_8 != '\n' && (*local_8 != '\r')) && (*local_8 != ' ')) && (*local_8 != '\t'))))
    for (; (local_c < *param_2 && ((*local_8 == '\n' || (*local_8 == '\r')))); local_8 = local_8 + 1
    for (; (local_10 < *param_2 && ((*local_8 == ' ' || (*local_8 == '\t')))); local_8 = local_8 + 1
    } while (local_10 != *param_2);

## C26
kind: function
address: 0x0044ef20
candidate name: `MdlNodeAnimMesh::InternalGenVertices`
facts:
- size 1665 B, RET 0
- callers 1: 0x004549a0
- callees: 0x00428f80 (CALL), 0x00429330 (CALL), 0x00919723 operator new (CALL), 0x004cd940 (CALL), 0x004cd6c0 (CALL), 0x004cda60 (CALL), ...
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    *(int *)(param_1 + 0x1d4) = *(int *)(param_1 + 0x1ac) / *(int *)(*(int *)(param_1 + 0x134) + 4);
    *(int *)(param_1 + 0x1d8) = *(int *)(param_1 + 0x1b8) / *(int *)(*(int *)(param_1 + 0x134) + 0x10)
    if (*(int *)(param_1 + 0x5c) <= local_18) {
    for (local_2c = 0; local_2c < 3; local_2c = local_2c + 1) {
    FUN_00428f80(0);
    FUN_00428f80(0);
    FUN_00428f80(0);
    local_8 = CONCAT31(local_8._1_3_,2);
    local_50 = *(int *)(*(int *)(*(int *)(*(int *)(param_1 + 0x134) + 0x48) + local_18 * 0x14) +

## C27
kind: function
address: 0x00783d80
candidate name: `CSWGuiMainInterface::DrawMap`
facts:
- size 49 B, RET 4
- callers 4: 0x00781be0 CClientExoAppInternal::MainLoop, 0x007ac470 CClientExoAppInternal::ProcessInput, 0x00839ad0, 0x0083a5b0
- callees: 0x007335f0 CExoString::CExoString_copy (CALL)
- twins: none
- decomp: undefined4 __thiscall this_fn(int param_1,undefined4 param_2)
    CExoString::CExoString_copy(param_1 + 0x10);

## C28
kind: function
address: 0x0069fee0
candidate name: `CSWBaseItemArray::Load`
facts:
- size 3948 B, RET 0
- callers 1: 0x0069dea0
- callees: 0x00919723 operator new (CALL), 0x00710840 (CALL), 0x0071ad40 (CALL), 0x0071c3d0 (CALL), 0x0091d67e (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), ...
- indirect: 0x006a0e29 slot 0x0
- strings: 0x00999080 "PrimaryAbil", 0x0099908c "AlignRstrctType", 0x0099909c "AlignRestrict", ...
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    FUN_00710840("CLASSES");
    CExoString::DefaultConstructor();
    CExoString::CStrConstructor();
    CExoString::Destructor();
    CExoString::CStrConstructor();
    CExoString::Destructor();
    CExoString::CStrConstructor();
    CExoString::Destructor();
    CExoString::CStrConstructor();

## C29
kind: function
address: 0x0070b9b0
candidate name: `CExoSound::ReInitialize`
facts:
- size 129 B, RET 8
- callers 1: 0x007db680
- callees: 0x0070b8c0 (CALL), 0x0070b890 (CALL), 0x007051c0 (CALL)
- twins: none
- decomp: void __thiscall this_fn(int *param_1,undefined4 param_2,undefined4 param_3)
    if (*param_1 != 0) {
    bVar1 = FUN_0070b8c0();
    if (bVar1 < 0x10) {
    uVar2 = 0x10;
    uVar2 = FUN_0070b8c0();
    bVar1 = FUN_0070b890();
    if (bVar1 < 0x18) {
    uVar3 = FUN_0070b890();
    FUN_007051c0(uVar3,uVar2,param_2,param_3);

## C30
kind: function
address: 0x0071cf80
candidate name: `C2DA::Free`
facts:
- size 273 B, RET 8
- callers 1: 0x0071c3d0
- twins: none
- decomp: bool this_fn(int *param_1,uint *param_2)
    local_c = 0;
    local_10 = 0;
    local_8 = (char *)*param_1;
    do {
    if ((*param_2 <= local_c) ||
    ((((*local_8 != '\n' && (*local_8 != '\r')) && (*local_8 != ' ')) && (*local_8 != '\t'))))
    for (; (local_c < *param_2 && ((*local_8 == '\n' || (*local_8 == '\r')))); local_8 = local_8 + 1
    for (; (local_10 < *param_2 && ((*local_8 == ' ' || (*local_8 == '\t')))); local_8 = local_8 + 1
    } while (local_10 != *param_2);

## C31
kind: function
address: 0x00750790
candidate name: `CSWGuiMainInterface::DrawMap`
facts:
- size 1144 B, RET 4
- callers 1: 0x0074b3f0
- callees: 0x0072fb00 CClientExoApp::GetClientOptions (CALL), 0x0051c8d0 (CALL), 0x0051bd90 CServerExoApp::GetModule (CALL), 0x00710c20 (CALL), 0x00750c10 (CALL), 0x005f7350 (CALL), ...
- indirect: 0x007509eb slot 0x4, 0x00750a1f slot 0x24, 0x00750b2c slot 0x38, 0x00750b5f slot 0x38
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 param_2)
    local_c = CClientExoApp::GetClientOptions();
    if ((local_c == 0) || ((*(uint *)(local_c + 0x1c) >> 3 & 1) != 0)) {
    iVar1 = FUN_0051c8d0();
    if (*(int *)(iVar1 + 600) == 0) {
    iVar1 = CServerExoApp::GetModule();
    local_8 = *(int *)(iVar1 + 0x238);
    if ((local_8 == 0) || (*(int *)(local_8 + 4) == 0)) {
    *(uint *)(param_1 + 0x5da4) = *(uint *)(param_1 + 0x5da4) & 0xfffffffd;
    piVar3 = (int *)CClientExoApp::GetModuleCamera();

## C32
kind: function
address: 0x0085ef00
candidate name: `CSoundSetList::CSoundSetList`
facts:
- size 1217 B, RET 0
- callers 1: 0x00788d90 CClientExoAppInternal::StartServices
- callees: 0x00919723 operator new (CALL), 0x00710840 (CALL), 0x0071ad40 (CALL), 0x0071c3d0 (CALL), 0x007107f0 (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), ...
- indirect: 0x0085f05c slot 0x0, 0x0085f0a6 slot 0x0, 0x0085f340 slot 0x0, 0x0085f391 slot 0x0
- strings: 0x009a451c "RESREF", 0x009a4524 "SoundSetType", 0x009a4534 "SoundSet"
- twins: none
- decomp: ushort * __fastcall this_fn(ushort *param_1)
    FUN_00710840("SoundSet");
    FUN_00710840("SoundSetType");
    CExoString::DefaultConstructor();
    pcStack_d0 = "RESREF";
    CExoString::CStrConstructor();
    CExoString::Destructor();
    pcStack_d0 = "STRREF";
    CExoString::CStrConstructor();
    CExoString::Destructor();

## C33
kind: global
address: 0x009960c8
candidate name: `EffMasteryLight`
facts:
- referenced from 3 function(s)
- in 0x0058b880: local_20 = (undefined2)DAT_009960c8;
- in 0x0058baa0: if (*(ushort *)(*(int *)(*(int *)(param_1 + 0x148) + local_8 * 4) + 8) == DAT_009960c8) break;
- in 0x0084e6f0 (CSWGuiInGameCharacter::SetStats): iVar6 = FUN_0058bbe0(DAT_009960c8);

## C34
kind: function
address: 0x00512130
candidate name: `IODispatcher::ReadSync_4`
facts:
- size 419 B, RET 8
- callers 1: 0x00511f80
- callees: 0x00475d50 (CALL), 0x004762f0 (CALL), 0x00476370 (CALL), 0x005122e0 (CALL), 0x00452830 (CALL), 0x00475f70 (CALL), ...
- twins: none
- decomp: int __thiscall this_fn(undefined4 param_1,undefined4 param_2,undefined4 param_3)
    iVar2 = FUN_00475d50(param_2,param_1);
    if (iVar2 == 0) {
    pcVar3 = (char *)FUN_004762f0(4,iVar2);
    if (pcVar3 == (char *)0x0) {
    else if (*pcVar3 == '\0') {
    FUN_00476370(iVar2,pcVar3);
    uVar4 = FUN_00475d50(param_3,param_1);
    FUN_005122e0(iVar2,uVar4,0);
    iVar2 = FUN_00452830();

## C35
kind: function
address: 0x0047a510
candidate name: `CAurGUIStringInternal::GetFontPixelHeight`
facts:
- size 84 B, RET 0
- callees: 0x00479ab0 (CALL), 0x0091c860 (CALL)
- indirect: 0x0047a533 slot 0x38
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    undefined4 __fastcall this_fn(int param_1)
    (**(code **)(**(int **)(param_1 + 0x18) + 0x38))();
    FUN_00479ab0();
    uVar1 = FUN_0091c860();

## C36
kind: function
address: 0x00414ba0
candidate name: `CSWGuiControl::GetHeight`
facts:
- size 17 B, RET 0
- callers 3: 0x005f9220 CSWPartyTable::CreateParty, 0x00721bb0 CERFFile::WriteResource_2, 0x0085ee80
- twins: none
- decomp: undefined4 __fastcall this_fn(int param_1)
    return *(undefined4 *)(param_1 + 0x10);
