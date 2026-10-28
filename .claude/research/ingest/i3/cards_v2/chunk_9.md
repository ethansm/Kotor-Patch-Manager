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
address: 0x0071b0b0
candidate name: `CSWGuiListBox::OrganizeOversized`
facts:
- size 271 B, RET 12
- callers 12: 0x0053ad00 CServerExoAppInternal::AutoSaveOnEnteringModule, 0x0053af20 CServerExoAppInternal::DoPCAutosave, 0x0059ec70 CSWSEffectListHandler::OnApplyVisualEffect, 0x006de350 CSWSCreature
- callees: 0x0071c320 (CALL), 0x0071aef0 C2DA::GetColumnIndex (CALL), 0x007337c0 CExoString::operator= (CALL), 0x007338d0 (CALL), 0x00733db0 (CALL), 0x00733a60 (CALL), ...
- twins: none
- decomp: bool __thiscall this_fn(int param_1,undefined4 param_2,undefined4 param_3,int *param_4)
    iVar2 = FUN_0071c320(param_2);
    iVar3 = C2DA::GetColumnIndex(param_3);
    if ((iVar2 == -1) || (iVar3 == -1)) {
    CExoString::operator=(param_1 + 0x1c);
    else if (*(int *)(param_1 + 0x3c) == 0) {
    iVar4 = FUN_00733a60(&DAT_00986925);
    if (iVar4 == 0) {
    CExoString::operator=(*(int *)(*(int *)(param_1 + 0x38) + iVar2 * 4) + iVar3 * 8);
    if (*param_4 == 0) {

## C098
kind: function
address: 0x00418f20
candidate name: `CSWGuiControl::LoadFromGFF`
facts:
- size 185 B, RET 8
- callers 5: 0x004188e0, 0x004198d0, 0x0041bb70, 0x0041c870, 0x004211c0
- callees: 0x0040e2c0 (CALL), 0x00718c80 CResGFF::ReadFieldINT (CALL), 0x0040fea0 CSWGuiPanel::GetControl (CALL), 0x00419330 CSWGuiControl::AddChildControl (CALL)
- indirect: 0x00418f4e slot 0x4, 0x00418fa7 slot 0x28
- strings: 0x0098788c "Obj_ParentID"
- twins: none
- decomp: void __thiscall this_fn(int *param_1,undefined4 param_2,undefined4 param_3)
    FUN_0040e2c0(param_2,param_3);
    (**(code **)(*param_1 + 4))(param_1 + 1);
    iVar1 = CResGFF::ReadFieldINT(param_3,&DAT_0098789c,local_8,0xffffffff);
    iVar1 = CResGFF::ReadFieldINT(param_3,"Obj_ParentID",local_8,0xffffffff);
    if (iVar1 != -1) {
    iVar2 = (**(code **)(*(int *)param_1[0xe] + 0x28))();
    if (iVar2 != 0) {
    iVar1 = CSWGuiPanel::GetControl(iVar1);
    CSWGuiControl::AddChildControl(param_1);

## C099
kind: function
address: 0x00429f00
candidate name: `CAurGLBufferPool::LockSlot`
facts:
- size 275 B, RET 16
- callees: 0x0091d288 operator new[] (CALL)
- indirect: 0x00429f4e unresolved
- twins: none
- decomp: int __thiscall this_fn(int param_1,int param_2,int param_3,int param_4,undefined4 param_5)
    if (param_2 == -1) {
    else if (*(int *)(*(int *)(param_1 + 0x10) + param_2 * 4) == 0) {
    (*DAT_00a32d4c)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),
    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4));
    *(undefined4 *)(*(int *)(param_1 + 0x20) + param_2 * 4) = param_5;
    *(undefined4 *)(*(int *)(param_1 + 0x10) + param_2 * 4) = 1;
    local_8 = *(int *)(*(int *)(param_1 + 0x68) + param_2 * 4);
    if (local_8 == 0) {
    local_8 = operator new[](*(undefined4 *)(*(int *)(param_1 + 0x5c) + param_2 * 4));

## C100
kind: global
address: 0x009877d4
candidate name: `CSWGuiControl`
facts:
- referenced from 2 function(s)
- in 0x00418640 (CSWGuiControl::Destructor): 00418669 c700d4779800 MOV dword ptr [EAX],0x9877d4
- in 0x00418990 (CSWGuiControl::Constructor): 004189fb c700d4779800 MOV dword ptr [EAX],0x9877d4

## C101
kind: function
address: 0x008aa6f0
candidate name: `CSWGuiInGameEquipment::Destroy`
facts:
- size 44 B, RET 4
- callees: 0x008aa750 (CALL), 0x009196fd operator delete (CALL)
- twins: none
- decomp: undefined4 __thiscall this_fn(undefined4 param_1,uint param_2)
    FUN_008aa750();
    if ((param_2 & 1) != 0) {
    operator delete(param_1);

## C102
kind: global
address: 0x0098c27c
candidate name: `Camera::vftable`
facts:
- referenced from 2 function(s)
- in 0x0047ecd0: 0047ed27 c740047cc29800 MOV dword ptr [EAX + 0x4],0x98c27c
- in 0x0047f0a0: 0047f0d2 c741047cc29800 MOV dword ptr [ECX + 0x4],0x98c27c

## C103
kind: function
address: 0x00769f60
candidate name: `CSWCCreature::LoadModel`
facts:
- size 2654 B, RET 12
- callees: 0x00919723 operator new (CALL), 0x0085f7e0 (CALL), 0x0086bfd0 (CALL), 0x0086aea0 (CALL), 0x008695d0 (CALL), 0x00867f00 (CALL), ...
- indirect: 0x00769fe6 slot 0x0, 0x0076a0c8 slot 0x0, 0x0076a472 slot 0xc, 0x0076a4eb slot 0x8, 0x0076a502 slot 0x74, ...
- strings: 0x0099efe4 "CSWCCreature::LoadModel(): Failed to load creature model '%s'."
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    void __thiscall this_fn(int param_1,undefined4 param_2,char param_3,char param_4)
    local_18 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
    if (*(int *)(param_1 + 0x370) != 0) {
    if (*(int *)(param_1 + 0x68) != 0) {
    if (*(undefined4 **)(param_1 + 0x68) != (undefined4 *)0x0) {
    (**(code **)**(undefined4 **)(param_1 + 0x68))(1,local_18);
    _sprintf(local_118,"CSWCCreature::LoadModel(): Failed to load creature model \'%s\'.",uVar4);
    iVar3 = (**(code **)(*local_14 + 0xa0))("headconjure",&local_128,&local_138);
    uVar2 = CSWSCreature::GetMaxForcePoints();

## C104
kind: function
address: 0x0040e480
candidate name: `CClientExoApp::GetCharacterChangeInProgress`
facts:
- size 285 B, RET 0
- callers 1: 0x008690a0
- twins: none
- decomp: int * this_fn(int *param_1,int *param_2,int *param_3)
    if (*param_2 < *param_3) {
    local_20 = *param_2;
    else {
    local_20 = *param_3;
    if (param_2[1] < param_3[1]) {
    local_24 = param_2[1];
    else {
    if (*param_3 + param_3[2] < *param_2 + param_2[2]) {
    if (param_3[1] + param_3[3] < param_2[1] + param_2[3]) {

## C105
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

## C106
kind: function
address: 0x00479b60
candidate name: `LoadAlternateFonts`
facts:
- size 305 B, RET 4
- callers 2: 0x00479ca0, 0x0047a400
- callees: 0x0091d6f0 (CALL), 0x004274e0 (CALL), 0x004741e0 (CALL), 0x00422a70 (CALL)
- strings: 0x0098c018 "d3xfnt_d16x16b", 0x0098c028 "d2xfnt_d16x16b", 0x0098c038 "fnt_d16x16b", ...
- twins: none
- decomp: void __thiscall this_fn(int param_1,char *param_2)
    pcVar1 = _strstr(param_2,"dialogfont16x16b");
    pcVar1 = _strstr(param_2,"dialogfont10x10b");
    uVar2 = FUN_004274e0("d2xfont16x16b",0);
    uVar2 = FUN_004274e0("d3xfont16x16b",0);
    uVar2 = FUN_004274e0("diafnt16x16b_ps",0);
    uVar2 = FUN_004274e0("d2xfont16x16b_ps",0);
    uVar2 = FUN_004274e0("d3xfont16x16b_ps",0);
    pcVar1 = _strstr(param_2,"fnt_d16x16b");
    uVar2 = FUN_004274e0("d2xfnt_d16x16b",0);
