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

## C037
kind: function
address: 0x0065afd0
candidate name: `CSWSMessage::HandlePlayerToServerGuiContainerMessage`
facts:
- size 289 B, RET 8
- callers 1: 0x0065f8a0
- callees: 0x00632ef0 (CALL), 0x005e36a0 (CALL), 0x005e3de0 (CALL), 0x005e3e40 (CALL), 0x0051c080 CServerExoApp::GetObjectArray (CALL), 0x0053dfb0 CGameObjectArray::GetGameObject (CALL), ...
- indirect: 0x0065b08d slot 0x48, 0x0065b0d6 slot 0x38
- globals: 0x00a1b4a4 READ
- twins: none
- decomp: undefined4 __thiscall this_fn(undefined4 param_1,undefined4 param_2,char param_3)
    if (param_3 == '\x02') {
    uVar2 = FUN_00632ef0(param_1);
    uVar3 = FUN_005e36a0();
    iVar4 = FUN_005e3de0(1);
    if ((iVar4 != 0) || (iVar4 = FUN_005e3e40(1), iVar4 != 0)) {
    CServerExoApp::GetObjectArray(uVar2,ppiVar5);
    cVar1 = CGameObjectArray::GetGameObject(uVar2,ppiVar5);
    CSWSPlaceable::CloseInventory(uVar2,uVar3);
    CSWSItem::CloseInventory(uVar2,uVar3);

## C038
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

## C039
kind: function
address: 0x0075bf60
candidate name: `CExoPtrList::SetAllocated`
facts:
- size 278 B, RET 0
- callers 1: 0x0075c080
- callees: 0x00919f10 _strlen (CALL), 0x004178b0 (CALL), 0x0041fd90 (CALL), 0x0041e9a0 (CALL)
- indirect: 0x0075bfbe slot 0x4, 0x0075c00d slot 0x8, 0x0075c038 slot 0x4
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    local_18 = *(undefined4 *)(param_1 + 0x7c);
    local_14 = *(undefined4 *)(param_1 + 0x80);
    local_c = *(undefined4 *)(param_1 + 0x88);
    local_10 = *(int *)(param_1 + 0x94c) + (uint)*(byte *)(param_1 + 0x978) * -2;
    (**(code **)(*(int *)(param_1 + 0x78) + 4))(&local_18);
    if (*(int *)(param_1 + 0x168) == 0) {
    local_24 = _strlen(*(char **)(param_1 + 0x168));
    if ((int)local_24 < 1) {
    local_c = FUN_004178b0();

## C040
kind: function
address: 0x008b2f70
candidate name: `CSWGuiContainer::SetupGiveMode`
facts:
- size 1410 B, RET 4
- callers 2: 0x008b2850, 0x008b35d0
- callees: 0x0084a640 (CALL), 0x0051c8d0 (CALL), 0x005f7a30 CSWPartyTable::GetPartyItems (CALL), 0x00606160 CItemRepository::ItemListGetItemObjectID (CALL), 0x0073fec0 CClientExoApp::ServerToClientObj
- indirect: 0x008b3177 slot 0xa0, 0x008b3398 slot 0x88, 0x008b344c slot 0x8, 0x008b346e slot 0x8
- twins: none
- decomp: void __thiscall this_fn(int *param_1,int param_2)
    local_20 = CSWPartyTable::GetPartyItems();
    uVar2 = CItemRepository::ItemListGetItemObjectID(local_58);
    local_28 = CClientExoApp::ServerToClientObjectId(uVar2);
    local_18 = CItemRepository::ItemListGetItem(local_58);
    local_cc = CSWGuiInGameItemEntry::Constructor();
    CSWGuiInGameItemEntry::SetItem(local_28,0,0);
    CSWGuiControl::AddEvent(0x27,param_1,FUN_008b3500);
    CSWGuiControl::AddEvent(0x2d,param_1,FUN_008b3500);
    CExoString::CStrConstructor(&DAT_00986bfc);

## C041
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

## C042
kind: function
address: 0x0075bde0
candidate name: `CSWGuiMessageBox::SetMessage_2`
facts:
- size 55 B, RET 4
- callees: 0x0071dee0 (CALL)
- indirect: 0x0075be0f slot 0x74
- globals: 0x00a1b4b0 READ
- twins: none
- decomp: void this_fn(undefined4 param_1)
    local_8 = auStack_18;
    local_10 = FUN_0071dee0(auStack_18,param_1);
    (**(code **)(*local_c + 0x74))();

## C043
kind: function
address: 0x0041f7f0
candidate name: `CSWGuiListBox::OrganizeOversized`
facts:
- size 398 B, RET 0
- callers 3: 0x0041d940, 0x0041ea50, 0x00420c00
- callees: 0x0041e800 (CALL), 0x0041c7c0 (CALL), 0x0091c860 (CALL)
- indirect: 0x0041f978 slot 0x4
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    local_c = *(undefined4 *)(param_1 + 0x2c4);
    local_10 = *(int *)(param_1 + 0x2a4) + (uint)*(byte *)(param_1 + 0x2d0) * -2;
    local_18 = (uint)*(byte *)(param_1 + 0x2d0);
    local_8 = FUN_0041e800();
    if (*(short *)(param_1 + 0x2d2) == local_8) {
    local_14 = (*(int *)(param_1 + 0x2a8) - *(int *)(param_1 + 0x2c4)) -
    (uint)*(byte *)(param_1 + 0x2d0);
    FUN_0041c7c0(*(undefined4 *)(param_1 + 0x2c4));
    local_14 = FUN_0091c860();

## C044
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

## C045
kind: function
address: 0x0071b0b0
candidate name: `C2DA::GetInt`
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

## C046
kind: function
address: 0x0041f170
candidate name: `CSWGuiListBox::OrganizeUnequal`
facts:
- size 1659 B, RET 0
- callers 1: 0x0041ea50
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x00733780 CExoString::Destructor (CALL), 0x0041c760 (CALL), 0x0041c810 (CALL), 0x0041c7c0 (CALL)
- indirect: 0x0041f1ed slot 0x34, 0x0041f241 slot 0x34, 0x0041f4a5 slot 0x4, 0x0041f51a slot 0x4, 0x0041f5c9 slot 0x4
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    local_1c = *(int *)(param_1 + 0x2b0);
    CExoString::DefaultConstructor(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if ((*(uint *)(param_1 + 0x2cc) >> 3 & 1) != 0) {
    if (local_1c == 1) {
    uVar2 = (**(code **)(*(int *)**(undefined4 **)(param_1 + 0x2ac) + 0x34))();
    *(undefined4 *)(param_1 + 0x2c4) = uVar2;
    *(undefined4 *)(param_1 + 0x2c4) = 0;
    CExoString::Destructor();
    CExoString::Destructor();

## C047
kind: function
address: 0x0041fc30
candidate name: `CSWGuiListBox::AddControls`
facts:
- size 345 B, RET 16
- callers 38: 0x00758e30, 0x007592f0, 0x00759820, 0x0075a7c0, 0x007d3750, ...
- callees: 0x0041d290 (CALL), 0x004142e0 (CALL), 0x0083ea60 CExoArrayList<uint32>::Add (CALL), 0x00421720 (CALL), 0x0041ea50 (CALL)
- twins: none
- decomp: void __thiscall this_fn(int param_1,int *param_2,uint param_3,uint param_4,uint param_5)
    FUN_0041d290();
    *(uint *)(param_1 + 0x2cc) = *(uint *)(param_1 + 0x2cc) & 0xfffffffd | (param_3 & 1) << 1;
    *(uint *)(param_1 + 0x2cc) = *(uint *)(param_1 + 0x2cc) & 0xfffffffb | (param_4 & 1) << 2;
    for (local_8 = 0; local_8 < iVar1; local_8 = local_8 + 1) {
    iVar2 = *(int *)(*param_2 + local_8 * 4);
    if ((iVar2 != 0) && (iVar3 = FUN_004142e0(iVar2), iVar3 == -1)) {
    *(int *)(iVar2 + 0x38) = param_1;
    *(int *)(iVar2 + 0x54) = local_c;
    CExoArrayList<uint32>::Add(iVar2);

## C048
kind: function
address: 0x0047a600
candidate name: `SetString`
facts:
- size 126 B, RET 4
- callees: 0x00919f10 _strlen (CALL), 0x009196fd operator delete (CALL), 0x00919723 operator new (CALL), 0x0091cff0 (CALL)
- twins: none
- decomp: void __thiscall this_fn(int param_1,char *param_2)
    sVar1 = _strlen(param_2);
    sVar2 = _strlen(*(char **)(param_1 + 0x14));
    if (sVar2 < sVar1) {
    operator delete(*(undefined4 *)(param_1 + 0x14));
    sVar1 = _strlen(param_2);
    uVar3 = operator new(sVar1 + 1);
    *(undefined4 *)(param_1 + 0x14) = uVar3;
    FUN_0091cff0(*(undefined4 *)(param_1 + 0x14),param_2);
