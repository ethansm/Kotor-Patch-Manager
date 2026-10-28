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
address: 0x00417340
candidate name: `CSWGuiTextParams::SetAlignment`
facts:
- size 310 B, RET 12
- callers 5: 0x004197d0, 0x00419a80, 0x00419dc0, 0x0041a8b0, 0x00421100
- callees: 0x00710980 (CALL), 0x00734010 (CALL), 0x0047ec30 (CALL), 0x00416bd0 CSWGuiTextParams::operator= (CALL), 0x00417800 (CALL), 0x009196ee __security_check_cookie (CALL)
- indirect: 0x0041737e slot 0x4, 0x0041741c slot 0x64, 0x0041743d slot 0x74, 0x0041745c slot 0x1c
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 *param_2,int param_3,undefined4 param_4)
    local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
    if ((*(int *)(param_1 + 0x14) != 0) && (*(int **)(param_1 + 0x14) != (int *)0x0)) {
    (**(code **)(**(int **)(param_1 + 0x14) + 4))(1);
    FUN_00710980(local_1c);
    uVar1 = FUN_00734010(local_1c,param_3 + 0x1c,param_4);
    uVar1 = FUN_0047ec30(uVar1);
    *(undefined4 *)(param_1 + 0x14) = uVar1;
    CSWGuiTextParams::operator=(param_3);
    *(undefined4 *)(param_1 + 4) = *param_2;

## C062
kind: function
address: 0x00652810
candidate name: `Global::AurGrassMaterialInUse`
facts:
- size 286 B, RET 8
- callers 2: 0x006527c0, 0x00652810
- callees: 0x0044c8a0 (CALL), 0x00652810 (CALL)
- twins: none
- decomp: void __thiscall this_fn(int param_1,int param_2,undefined4 param_3)
    if (param_2 != -1) {
    iVar4 = param_2 * 0x2c + *(int *)(param_1 + 0xac);
    uVar1 = FUN_0044c8a0(local_10,param_2 * 0x2c + *(int *)(param_1 + 0xac) + 0xc,param_1 + 0x2c,
    puVar2 = (undefined4 *)FUN_0044c8a0(local_1c,uVar1);
    *(undefined4 *)(iVar4 + 0xc) = *puVar2;
    *(undefined4 *)(iVar4 + 0x10) = puVar2[1];
    *(undefined4 *)(iVar4 + 0x14) = puVar2[2];
    puVar2 = (undefined4 *)(param_2 * 0x2c + *(int *)(param_1 + 0xac));
    uVar1 = FUN_0044c8a0(local_28,param_2 * 0x2c + *(int *)(param_1 + 0xac),param_1 + 0x2c,param_3);

## C063
kind: function
address: 0x00414ba0
candidate name: `C2DA::GetINTEntry`
facts:
- size 17 B, RET 0
- callers 3: 0x005f9220 CSWPartyTable::CreateParty, 0x00721bb0 CERFFile::WriteResource_2, 0x0085ee80
- twins: none
- decomp: undefined4 __fastcall this_fn(int param_1)
    return *(undefined4 *)(param_1 + 0x10);

## C064
kind: function
address: 0x00419880
candidate name: `CSWGuiLabel::Draw`
facts:
- size 67 B, RET 4
- callers 1: 0x0084c1e0
- indirect: 0x0041989d slot 0xc, 0x004198bb slot 0xc
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 param_2)
    (**(code **)(*(int *)(param_1 + 0x60) + 0xc))(param_2);
    (**(code **)(*(int *)(param_1 + 0xd8) + 0xc))(param_2);

## C065
kind: function
address: 0x0047a0c0
candidate name: `GetFontTexture`
facts:
- size 71 B, RET 0
- indirect: 0x0047a0f2 slot 0x40
- twins: none
- decomp: undefined4 __fastcall this_fn(int param_1)
    if (*(int *)(param_1 + 0x18) == 0) {
    local_18 = 0;
    else {
    local_18 = (**(code **)(**(int **)(param_1 + 0x18) + 0x40))();

## C066
kind: function
address: 0x005f6b90
candidate name: `CSWMapInfo::Initialize`
facts:
- size 548 B, RET 48
- callers 1: 0x00523870 CSWSArea::LoadAreaHeader
- callees: 0x0091c860 (CALL), 0x00919723 operator new (CALL), 0x0091c630 (CALL)
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    this_fn(int *param_1,int param_2,int param_3,int param_4,float param_5,float param_6,
    if (param_2 != 0) {
    if ((0x58 < param_3) || (param_3 == 0)) {
    iVar3 = FUN_0091c860();
    if (param_1[3] < 1) {
    param_1[5] = (int)(1.0 / (float)param_13);
    if (*param_1 == 0) {
    iVar3 = (param_1[2] + 1) * (param_1[3] + 1);
    param_1[1] = ((int)(iVar3 + (iVar3 >> 0x1f & 0x1fU)) >> 5) + 1;

## C067
kind: function
address: 0x008bc100
candidate name: `DialogTop::SetReply`
facts:
- size 292 B, RET 0
- callees: 0x008ba420 (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x008bab20 CSWGuiDialogLetterbox::SetExtentSpecial (CALL)
- indirect: 0x008bc146 slot 0x8, 0x008bc19a slot 0x4
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    FUN_008ba420();
    local_1c = *(undefined4 *)(param_1 + 0x2a54);
    local_18 = *(undefined4 *)(param_1 + 0x2a58);
    local_14 = *(undefined4 *)(param_1 + 0x2a5c);
    local_10 = *(int *)(param_1 + 0x2a60);
    local_c = (**(code **)(*(int *)(param_1 + 0x2b28) + 8))();
    if (0 < local_8) {
    CClientExoApp::GetInGameGui();
    CSWGuiDialogLetterbox::SetExtentSpecial(local_1c,local_18,local_14,local_10);

## C068
kind: function
address: 0x0071d0a0
candidate name: `C2DA::Free`
facts:
- size 445 B, RET 0
- callers 16: 0x006ed7d0 CTwoDimArrays::UnLoad2DArrays, 0x006f00d0 CTwoDimArrays::UnLoadIPRPCostTables, 0x006f0440 CTwoDimArrays::UnLoadIPRPParamTables, 0x0071ae80, 0x0071c3d0, ...
- callees: 0x0040c170 (CALL), 0x009196fd operator delete (CALL), 0x007338d0 (CALL), 0x007112b0 CRes::Release (CALL)
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    if (*(int *)(param_1 + 0x2c) != 0) {
    if (*(int *)(param_1 + 0x38) != 0) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x24); local_8 = local_8 + 1) {
    if ((*(int *)(*(int *)(param_1 + 0x38) + local_8 * 4) != 0) &&
    (*(int *)(*(int *)(param_1 + 0x38) + local_8 * 4) != 0)) {
    FID_conflict__vector_deleting_destructor_(3);
    operator delete(*(undefined4 *)(param_1 + 0x38));
    *(undefined4 *)(param_1 + 0x38) = 0;
    CRes::Release();

## C069
kind: function
address: 0x0042a3d0
candidate name: `GLBufferPool::Resize`
facts:
- size 159 B, RET 4
- callees: 0x004491c0 (CALL), 0x0091c6aa operator delete[] (CALL)
- indirect: 0x0042a418 unresolved
- twins: none
- decomp: void __thiscall this_fn(int param_1,int param_2)
    if (param_2 != -1) {
    FUN_004491c0(param_2);
    *(undefined4 *)(*(int *)(param_1 + 0x10) + param_2 * 4) = 0;
    local_8 = *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4);
    (*DAT_00a32d54)(1,&local_8);
    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x5c) + param_2 * 4) = 0;
    operator delete[](*(undefined4 *)(*(int *)(param_1 + 0x68) + param_2 * 4));
    *(undefined4 *)(*(int *)(param_1 + 0x68) + param_2 * 4) = 0;

## C070
kind: function
address: 0x007224c0
candidate name: `CERFString::Destructor`
facts:
- size 43 B, RET 0
- callers 3: 0x00720fc0, 0x007216c0 CERFFile::WriteResource, 0x00721bb0 CERFFile::WriteResource_2
- twins: none
- decomp: undefined4 * __fastcall this_fn(undefined4 *param_1)
    *param_1 = 0;
    param_1[1] = 0;
    param_1[2] = 0;

## C071
kind: function
address: 0x00479b10
candidate name: `CAurGUIStringInternal::RefreshResolutionTextScalar`
facts:
- size 78 B, RET 0
- callers 3: 0x0047aca0, 0x0047af30, 0x0047b500
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    void __fastcall this_fn(int param_1)
    if (_DAT_009f5f64 != *(float *)(param_1 + 0x5c)) {
    fVar1 = *(float *)(param_1 + 0x5c);
    *(float *)(param_1 + 0x5c) = _DAT_009f5f64;
    *(float *)(param_1 + 0x54) = (*(float *)(param_1 + 0x5c) / fVar1) * *(float *)(param_1 + 0x54);

## C072
kind: function
address: 0x00511fb0
candidate name: `CModelLoader::LoadModelByName`
facts:
- size 368 B, RET 4
- callers 1: 0x00511f50
- callees: 0x00475ab0 (CALL), 0x004762f0 (CALL), 0x00476370 (CALL), 0x005122e0 (CALL), 0x00452830 (CALL), 0x00475ea0 (CALL), ...
- twins: none
- decomp: void __thiscall this_fn(undefined4 param_1,undefined4 param_2)
    local_24 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
    local_10 = FUN_00475ab0(param_2,&DAT_0098b9a8,0,1,param_1);
    local_8 = FUN_00475ab0(param_2,&DAT_009921dc,0,1);
    if (local_10 != 0) {
    local_c = (char *)FUN_004762f0(4,local_10);
    if (local_c == (char *)0x0) {
    local_c = (char *)0x0;
    else if (*local_c == '\0') {
    _sprintf(local_a4,"%s%s",param_2,&DAT_0098b9a8);
