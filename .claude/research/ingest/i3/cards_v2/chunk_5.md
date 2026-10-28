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

## C049
kind: function
address: 0x005e39d0
candidate name: `CSWGuiMessageBox::Constructor_2`
facts:
- size 144 B, RET 4
- callers 32: 0x005e3c90, 0x0065bd50, 0x0065be70, 0x00662060, 0x006626d0, ...
- callees: 0x005e3620 (CALL)
- twins: none
- decomp: /* WARNING: Removing unreachable block (ram,0x005e3a29) */
    /* WARNING: Removing unreachable block (ram,0x005e3a4a) */
    /* WARNING: Removing unreachable block (ram,0x005e3a50) */
    undefined4 __fastcall this_fn(int param_1)
    if (*(uint *)(param_1 + 0x2c) < *(int *)(param_1 + 0x30) + 4U) {
    *(int *)(param_1 + 0x30) = *(int *)(param_1 + 0x30) + 4;
    uVar1 = 0;
    else {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0x28) + *(int *)(param_1 + 0x30));
    *(int *)(param_1 + 0x30) = *(int *)(param_1 + 0x30) + 4;

## C050
kind: function
address: 0x00729ed0
candidate name: `CRes2DA::CRes2DA`
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

## C051
kind: function
address: 0x004687e0
candidate name: `Scene::DoGobShadows`
facts:
- size 876 B, RET 0
- callees: 0x00463ed0 (CALL), 0x0050a630 (CALL), 0x00473fb0 (CALL), 0x00463b20 (CALL), 0x005142f0 (CALL), 0x00465c90 (CALL)
- indirect: 0x004688c3 slot 0x14, 0x00468a12 slot 0x14
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    void this_fn(void)
    uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
    FUN_00463ed0(0);
    uVar3 = FUN_0050a630(0);
    FUN_00473fb0(uVar3);
    if (local_18 == 0) {
    FUN_00463b20(uVar2);
    if (local_20 != 0) {
    if ((DAT_009f5d40 != 0) && (DAT_00a2004c != '\0')) goto code_r0x0046887d;

## C052
kind: function
address: 0x007457e0
candidate name: `CSWGuiMainInterfaceChar::DrawStack`
facts:
- size 813 B, RET 12
- callers 1: 0x00745770
- callees: 0x004792c0 (CALL), 0x00479460 GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected) (CALL), 0x00479ab0 (CALL), 0x004799c0 (CALL), 0x004793f0 (CALL)
- indirect: 0x00745a7b slot 0x4, 0x00745a8f slot 0x38, 0x00745afb slot 0x4
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 param_2,int param_3,int param_4)
    local_14 = *(undefined4 *)(param_1 + 0x54c);
    local_10 = *(undefined4 *)(param_1 + 0x550);
    local_c = *(undefined4 *)(param_1 + 0x554);
    local_8 = *(int *)(param_1 + 0x558);
    FUN_004792c0();
    iVar1 = GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected)(local_14,local_10,local_c,local_8,&DAT_00a1ba3c,0,0x3f800000);
    if (iVar1 != 0) {
    if (param_4 == 0) {
    local_51 = *(byte *)(param_1 + 0xddd);

## C053
kind: function
address: 0x0070b9b0
candidate name: `CClientExoAppInternal::StallServerCode`
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

## C054
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
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    CSWGuiPanel::Constructor(param_2);
    FUN_00710840("container_p");
    CSWGuiPanel::StartLoadFromLayout(local_44,0,1);
    CExoString::CStrConstructor("LBL_MESSAGE");
    CSWGuiPanel::InitControl(param_1 + 0x1d,local_4c,1,1);
    CExoString::Destructor();
    CExoString::CStrConstructor("LBL_BUTTON");
    CSWGuiPanel::InitControl(param_1 + 0x6f,local_54,1,1);
    CExoString::Destructor();

## C055
kind: function
address: 0x00416720
candidate name: `CSWGuiBorder::Load`
facts:
- size 372 B, RET 12
- callees: 0x00734010 (CALL), 0x00718960 CResGFF::GetStructFromStruct (CALL), 0x00710840 (CALL), 0x00718e40 CResGFF::ReadFieldCResRef (CALL), 0x00416090 (CALL), 0x00718c80 CResGFF::ReadFieldINT (CALL)
- strings: 0x00987660 "ROTATESTYLE", 0x0098766c "ROTATE", 0x00987680 "FLIPSTYLE", ...
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    void __thiscall this_fn(int param_1,undefined4 param_2,undefined4 param_3)
    uVar1 = FUN_00734010();
    iVar2 = CResGFF::GetStructFromStruct(local_8,param_3,uVar1);
    uVar1 = CResGFF::ReadFieldCResRef(local_34,local_8,"IMAGE",&local_10,local_24);
    uVar1 = CResGFF::ReadFieldINT(local_8,"DRAWSTYLE",&local_10,0);
    uVar3 = CResGFF::ReadFieldINT(local_8,"FLIPSTYLE",&local_10,0);
    uVar1 = CResGFF::ReadFieldINT(local_8,"ALIGNMENT",&local_10,0x12);
    fVar4 = (float10)CResGFF::ReadFieldFLOAT(local_8,"ROTATE",&local_10,0);
    uVar1 = CResGFF::ReadFieldINT(local_8,"ROTATESTYLE",&local_10,0);

## C056
kind: global
address: 0x00987bb4
candidate name: `CSWGuiProgressBar`
facts:
- referenced from 6 function(s)
- in 0x0041b7f0: 0041b828 c700b47b9800 MOV dword ptr [EAX],0x987bb4
- in 0x0041b8a0: 0041b8c9 c700b47b9800 MOV dword ptr [EAX],0x987bb4
- in 0x0074a780: 0074a7bd c701b47b9800 MOV dword ptr [ECX],0x987bb4

## C057
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

## C058
kind: function
address: 0x00880740
candidate name: `CSWGuiInGameCharacter::Constructor`
facts:
- size 3814 B, RET 4
- callers 1: 0x00781590 CClientExoAppInternal::DisplayMainMenu
- callees: 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x0041ce90 (CALL), 0x0040def0 (CALL), 0x00883f10 (CALL), 0x00418990 CSWGuiControl::Constructor (CALL), 0x00417f40 (CALL), ...
- indirect: 0x00880fa6 slot 0x70, 0x008810bd slot 0x18, 0x008810d0 slot 0x14, 0x008810e8 slot 0x74, 0x008810ff slot 0x44, ...
- strings: 0x009a5908 "HD0:CURRENTGAME", 0x009a5918 "HD0:GAMEINPROGRESS", 0x009a592c "mainmenu05", ...
- globals: 0x00a1b490 READ, 0x00a1b494 READ, 0x00a1b4a4 READ, ...
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    CSWGuiPanel::Constructor(param_2);
    CSWGuiControl::Constructor();
    FUN_00710840("MAINMENU");
    iVar2 = CExoResMan::Exists(local_7c,0xbba,0);
    CExoString::CStrConstructor("RIMS:MAINMENU");
    CExoResMan::AddResourceImageFile(local_84);
    CExoString::Destructor();
    CSWGuiControl::AddEvent(0x27,param_1,FUN_00882210);
    CSWGuiControl::AddEvent(0x27,param_1,CSWGuiMainMenu::OnNewGamePicked);

## C059
kind: function
address: 0x00747210
candidate name: `CSWGuiMainInterface::Constructor`
facts:
- size 13027 B, RET 4
- callers 2: 0x007be4c0, 0x007cfca0 CGuiInGame::ResetInterfaceForSize
- callees: 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x0091d67e (CALL), 0x00742150 (CALL), 0x0074a910 (CALL), 0x00419740 (CALL), 0x0084a640 (CALL), ...
- indirect: 0x00748008 slot 0x98, 0x007487d3 slot 0x4, 0x0074889b slot 0x4, 0x007496ac slot 0x4, 0x00749951 slot 0x4, ...
- strings: 0x0099d9b4 "uibit_fill_2bt", 0x0099d9c4 "uibit_brdr_16bet", 0x0099d9d8 "uibit_brdr_16bct", ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ, 0x00a1b4a4 READ, ...
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    CSWGuiPanel::Constructor(param_2);
    CSWGuiBorder::Constructor();
    CSWGuiPanel::Constructor(param_2);
    CSWGuiPanel::Constructor(param_2);
    CExoString::DefaultConstructor();
    FUN_00710840("mipc28x6_p");
    CSWGuiPanel::StartLoadFromLayout(local_22d,1,1);
    CExoString::Format(local_18,"LBL_QUEUE%d",local_17c);
    CSWGuiPanel::InitControl(param_1 + local_17c * 0x52 + 0x17cb,local_18,1,1);

## C060
kind: function
address: 0x008b0c30
candidate name: `CSWGuiInGameItemEntry::SetExtent`
facts:
- size 654 B, RET 4
- callees: 0x00479ab0 (CALL), 0x0091c860 (CALL), 0x00919f10 _strlen (CALL), 0x00416fa0 CSWGuiTextParams::SetAlignment (CALL), 0x00414d80 CSWGuiBorder::GetInnerExtent (CALL), 0x0040e360 (CALL)
- indirect: 0x008b0ccf slot 0x4, 0x008b0cea slot 0x4, 0x008b0d05 slot 0x4, 0x008b0e14 slot 0x4, 0x008b0e56 slot 0x4, ...
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 *param_2)
    if (0x24 < (int)param_2[2]) {
    *(undefined4 *)(param_1 + 4) = *param_2;
    *(undefined4 *)(param_1 + 8) = param_2[1];
    *(undefined4 *)(param_1 + 0xc) = param_2[2];
    *(undefined4 *)(param_1 + 0x10) = param_2[3];
    CSWGuiTextParams::SetAlignment(0x22);
    CSWGuiTextParams::SetAlignment(0x24);
    uVar1 = CSWGuiBorder::GetInnerExtent(local_40);
    uVar2 = CSWGuiBorder::GetInnerExtent(local_50);
