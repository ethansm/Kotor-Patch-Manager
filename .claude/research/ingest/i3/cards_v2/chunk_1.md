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

## C001
kind: global
address: 0x0098764c
candidate name: `CSWGuiImage`
facts:
- referenced from 2 function(s)
- in 0x00416290 (CSWGuiImage::Constructor): 004162fb c7004c769800 MOV dword ptr [EAX],0x98764c
- in 0x00416410: 0041643b c7004c769800 MOV dword ptr [EAX],0x98764c

## C002
kind: function
address: 0x0074b3a0
candidate name: `CSWGuiMainInterface::HitCheckMouse`
facts:
- size 68 B, RET 8
- callees: 0x007433b0 (CALL), 0x0040eae0 CSWGuiPanel::HitCheckMouse (CALL)
- twins: none
- decomp: int this_fn(undefined4 param_1,undefined4 param_2)
    local_8 = FUN_007433b0(param_1,param_2);
    if (local_8 == 0) {
    local_8 = CSWGuiPanel::HitCheckMouse(param_1,param_2);

## C003
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

## C004
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

## C005
kind: function
address: 0x00411f30
candidate name: `CSWGuiManager::GetDialogMargins`
facts:
- size 31 B, RET 8
- callers 6: 0x00747210, 0x008bb5d0, 0x008bb8d0, 0x008bba80, 0x008bc3e0, ...
- twins: none
- decomp: void this_fn(undefined4 *param_1,undefined4 *param_2)
    *param_1 = 0x30;
    *param_2 = 0x24;

## C006
kind: function
address: 0x00479ca0
candidate name: `C2DA::Free`
facts:
- size 538 B, RET 24
- callers 1: 0x0047ec30
- callees: 0x0047cd10 (CALL), 0x0091d67e (CALL), 0x00449230 (CALL), 0x004741e0 (CALL), 0x004274e0 (CALL), 0x00479b60 (CALL), ...
- twins: none
- decomp: undefined4 * __thiscall
    this_fn(undefined4 *param_1,char *param_2,int param_3,undefined4 param_4,undefined4 param_5,
    FUN_0047cd10(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    _eh_vector_constructor_iterator_(param_1 + 9,4,3,FUN_00451d00,FUN_00473e80);
    FUN_00449230(0);
    FUN_00449230(0);
    local_8 = CONCAT31(local_8._1_3_,6);
    FUN_004741e0(0);
    FUN_004741e0(0);
    if (param_3 == 0) {

## C007
kind: global
address: 0x0098c27c
candidate name: `CSWGuiControl`
facts:
- referenced from 2 function(s)
- in 0x0047ecd0: 0047ed27 c740047cc29800 MOV dword ptr [EAX + 0x4],0x98c27c
- in 0x0047f0a0: 0047f0d2 c741047cc29800 MOV dword ptr [ECX + 0x4],0x98c27c

## C008
kind: function
address: 0x0075b370
candidate name: `CSWSMessage::HandlePlayerToServerGuiContainerMessage`
facts:
- size 1389 B, RET 8
- callers 2: 0x007bde80, 0x007be4c0
- callees: 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x00419740 (CALL), 0x0041a510 (CALL), 0x0041ce90 (CALL), 0x0075ae40 (CALL), 0x0075b260 (CALL), ...
- indirect: 0x0075b705 slot 0x4, 0x0075b78c slot 0x98, 0x0075b7fe slot 0x4
- strings: 0x0099e670 "LB_MESSAGE", 0x0099e67c "BTN_CANCEL", 0x0099e688 "BTN_OK", ...
- twins: none
- decomp: undefined4 * __thiscall this_fn(undefined4 *param_1,undefined4 param_2,int param_3)
    CSWGuiPanel::Constructor(param_2);
    FUN_00710840("datapad_p");
    CSWGuiPanel::StartLoadFromLayout(local_a00,1,1);
    CSWGuiBorderParams::SetFillImage(local_a10,0);
    CExoString::CStrConstructor("BTN_OK");
    CSWGuiPanel::InitControl(param_1 + 0xc2,local_a18,1,1);
    CExoString::Destructor();
    CExoString::CStrConstructor("BTN_CANCEL");
    CSWGuiPanel::InitControl(param_1 + 0x136,local_a20,1,1);

## C009
kind: function
address: 0x005f7350
candidate name: `CSWMapInfo::GetMarkerPos`
facts:
- size 93 B, RET 12
- callers 2: 0x00750790, 0x00895cc0 CSWGuiMapHider::Draw
- twins: none
- decomp: undefined4 __thiscall this_fn(int param_1,int param_2,undefined4 *param_3,undefined4 *param_4)
    if (*(int *)(param_1 + 4) == 0) {
    uVar1 = 0;
    else if (param_2 < 3) {
    if (*(int *)(param_1 + 0x28 + param_2 * 4) == -1) {
    uVar1 = 0;
    else {
    *param_3 = *(undefined4 *)(param_1 + 0x28 + param_2 * 4);
    *param_4 = *(undefined4 *)(param_1 + 0x34 + param_2 * 4);
    uVar1 = 1;

## C010
kind: function
address: 0x007224c0
candidate name: `CSWGuiListBox::IsOversized`
facts:
- size 43 B, RET 0
- callers 3: 0x00720fc0, 0x007216c0 CERFFile::WriteResource, 0x00721bb0 CERFFile::WriteResource_2
- twins: none
- decomp: undefined4 * __fastcall this_fn(undefined4 *param_1)
    *param_1 = 0;
    param_1[1] = 0;
    param_1[2] = 0;

## C011
kind: function
address: 0x00479ca0
candidate name: `CAurGUIStringInternal::Constructor`
facts:
- size 538 B, RET 24
- callers 1: 0x0047ec30
- callees: 0x0047cd10 (CALL), 0x0091d67e (CALL), 0x00449230 (CALL), 0x004741e0 (CALL), 0x004274e0 (CALL), 0x00479b60 (CALL), ...
- twins: none
- decomp: undefined4 * __thiscall
    this_fn(undefined4 *param_1,char *param_2,int param_3,undefined4 param_4,undefined4 param_5,
    FUN_0047cd10(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    _eh_vector_constructor_iterator_(param_1 + 9,4,3,FUN_00451d00,FUN_00473e80);
    FUN_00449230(0);
    FUN_00449230(0);
    local_8 = CONCAT31(local_8._1_3_,6);
    FUN_004741e0(0);
    FUN_004741e0(0);
    if (param_3 == 0) {

## C012
kind: function
address: 0x00724780
candidate name: `CExoKeyTable::Load`
facts:
- size 967 B, RET 4
- callers 1: 0x00726240
- callees: 0x007107f0 (CALL), 0x00919723 operator new (CALL), 0x007294c0 (CALL), 0x009196fd operator delete (CALL), 0x007337c0 CExoString::operator= (CALL), 0x006f3d90 (CALL), ...
- indirect: 0x007248d0 slot 0x2c, 0x0072492b slot 0x0, 0x00724a22 slot 0x0
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    void __thiscall this_fn(undefined4 *param_1,undefined4 param_2)
    local_18 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
    FUN_007107f0();
    uVar1 = operator new();
    if (param_1[0xc] != 0) {
    this = (CMFCRibbonProgressBar *)operator new();
    if (this == (CMFCRibbonProgressBar *)0x0) {
    local_b0 = CMFCRibbonProgressBar::CMFCRibbonProgressBar(this);
    CExoString::CStrConstructor(&DAT_00986925);
