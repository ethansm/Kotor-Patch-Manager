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

## C02
kind: function
address: 0x00745770
candidate name: `CSWGuiMainInterfaceChar::DrawEffects`
facts:
- size 99 B, RET 8
- callers 2: 0x0074b3f0, 0x00756b70
- callees: 0x007457e0 (CALL)
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 param_2,undefined4 param_3)
    if ((*(uint *)(param_1 + 0x590) >> 1 & 1) != 0) {
    if (*(char *)(param_1 + 0xddc) != '\0') {
    FUN_007457e0(param_2,param_3,1);
    if (*(char *)(param_1 + 0xddd) != '\0') {
    FUN_007457e0(param_2,param_3,0);

## C03
kind: function
address: 0x0070b9b0
candidate name: `C2DA::Free`
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

## C04
kind: function
address: 0x0090a970
candidate name: `CSWGuiFeatsCharGen::~CSWGuiFeatsCharGen`
facts:
- size 44 B, RET 4
- callees: 0x0090a9a0 (CALL), 0x009196fd operator delete (CALL)
- twins: none
- decomp: undefined4 __thiscall this_fn(undefined4 param_1,uint param_2)
    FUN_0090a9a0();
    if ((param_2 & 1) != 0) {
    operator delete(param_1);

## C05
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

## C06
kind: function
address: 0x00729ed0
candidate name: `CSWBaseItemArray::Load`
facts:
- size 81 B, RET 0
- callers 1: 0x0071d360
- callees: 0x007110d0 CRes::Constructor (CALL)
- twins: none
- decomp: /* Library Function - Single Match
    public: __thiscall CStatusBar::CStatusBar(void)
    CStatusBar * __thiscall CStatusBar::CStatusBar(CStatusBar *this)
    CRes::Constructor();
    *(undefined ***)this = CRes2DA::vftable;
    *(undefined4 *)(this + 0x28) = 0;
    *(undefined4 *)(this + 0x2c) = 0;
    *(undefined4 *)(this + 0x30) = 0;
    *(undefined4 *)(this + 0x34) = 0;
    *(undefined4 *)(this + 0x38) = 0;

## C07
kind: function
address: 0x0044ef20
candidate name: `GLRender::FrameBufferModifications`
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

## C08
kind: function
address: 0x00783d80
candidate name: `CClientExoAppInternal::StallServerCode`
facts:
- size 49 B, RET 4
- callers 4: 0x00781be0 CClientExoAppInternal::MainLoop, 0x007ac470 CClientExoAppInternal::ProcessInput, 0x00839ad0, 0x0083a5b0
- callees: 0x007335f0 CExoString::CExoString_copy (CALL)
- twins: none
- decomp: undefined4 __thiscall this_fn(int param_1,undefined4 param_2)
    CExoString::CExoString_copy(param_1 + 0x10);

## C09
kind: function
address: 0x0085ef00
candidate name: `MdlNodeAnimMesh::InternalGenVertices`
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

## C10
kind: function
address: 0x00435720
candidate name: `GLRender::FrameBufferModifications`
facts:
- size 1885 B, RET 0
- callers 1: 0x00478300
- callees: 0x004848b0 (CALL), 0x00000023 (CALL), 0x00000045 (CALL), 0x00000044 (CALL), 0x00484970 (CALL), 0x00000121 (CALL)
- indirect: 0x0043573f slot 0x28, 0x0043576f unresolved, 0x00435796 unresolved, 0x004357aa unresolved, 0x004357b6 unresolved, ...
- globals: 0x00a1b484 READ
- imports: ReleaseDC (USER32.DLL), glDeleteTextures (OPENGL32.DLL), wglDeleteContext (OPENGL32.DLL), ...
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    void this_fn(int param_1)
    if (DAT_00a1dd7c != (int *)0x0) {
    (**(code **)(*DAT_00a1dd7c + 0x28))();
    if (DAT_00a1ddc4 != (HDC)0x0) {
    iVar1 = FUN_004848b0();
    if (iVar1 == 0) {
    wglMakeCurrent(DAT_00a1ddc4,DAT_00a1ddc8);
    glDeleteTextures(1,&DAT_00a1ddcc);
    wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);

## C11
kind: function
address: 0x00464d10
candidate name: `Scene::AddObject`
facts:
- size 33 B, RET 4
- indirect: 0x00464d29 slot 0xb0
- twins: none
- decomp: void __thiscall this_fn(int *param_1,undefined4 param_2)
    (**(code **)(*param_1 + 0xb0))(param_2);

## C12
kind: function
address: 0x0071b8b0
candidate name: `C2DA::GetFloat`
facts:
- size 526 B, RET 12
- callers 37: 0x00573950 CSWSCreature::ReadItemsFromGff, 0x0057ff50 CSWSCreature::UpdateAppearanceDependantInfo, 0x00664ae0, 0x006a2170, 0x006bc9a0 CSWSCreatureStats::GetEffectLinkImmunity, ...
- callees: 0x0091cf18 (CALL), 0x0091c7ad (CALL), 0x00733a60 (CALL), 0x00919f10 _strlen (CALL), 0x00734010 (CALL)
- twins: none
- decomp: bool __thiscall this_fn(int param_1,int param_2,int param_3,undefined4 *param_4)
    if ((((param_2 < 0) || (*(int *)(param_1 + 0x24) <= param_2)) || (param_3 < 0)) ||
    (*(int *)(param_1 + 0x28) <= param_3)) {
    if (*(int *)(param_1 + 0x1c) == 0) {
    local_10 = FUN_0091cf18(*(int *)(param_1 + 0x1c));
    if (*(int *)(param_1 + 0x3c) != 0) {
    uVar2 = (uint)*(ushort *)
    (*(int *)(param_1 + 0x44) + (param_2 * *(int *)(param_1 + 0x28) + param_3) * 2);
    FID_conflict__sscanf((char *)(*(int *)(param_1 + 0x40) + uVar2),"%i",param_4);
    _Format = "%x";
