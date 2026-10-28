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

## C013
kind: function
address: 0x00745560
candidate name: `CSWGuiProgressBar::Draw`
facts:
- size 519 B, RET 4
- callers 1: 0x0074b3f0
- callees: 0x0091c860 (CALL)
- indirect: 0x0074568c slot 0x4, 0x00745711 slot 0x4, 0x0074575f slot 0x4
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    local_20 = *(int *)(param_1 + 0x694);
    local_1c = *(undefined4 *)(param_1 + 0x698);
    local_18 = *(undefined4 *)(param_1 + 0x69c);
    local_14 = *(undefined4 *)(param_1 + 0x6a0);
    iVar2 = (int)*(short *)(param_1 + 0xdda) - (int)*(short *)(param_1 + 0xdd6);
    iVar3 = (int)*(short *)(param_1 + 0xdd6);
    local_c = FUN_0091c860();
    local_10 = FUN_0091c860((int)*(short *)(param_1 + 0xdd4),
    (int)*(short *)(param_1 + 0xdd8) - (int)*(short *)(param_1 + 0xdd4),iVar3,

## C014
kind: function
address: 0x00750c10
candidate name: `CSWGuiMainInterface::InitializeMap`
facts:
- size 577 B, RET 0
- callers 2: 0x00750790, 0x007ceec0
- callees: 0x0051bd90 CServerExoApp::GetModule (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), 0x00710930 (CALL), 0x00734010 (CALL), 0x00734270 CExoString::Format (CALL), 0x00710810 (CALL), 
- indirect: 0x00750d6c slot 0x4
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    local_14 = CServerExoApp::GetModule(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    CExoString::DefaultConstructor();
    CExoString::DefaultConstructor();
    local_8 = CONCAT31(local_8._1_3_,1);
    CExoString::Format(local_24,"lbl_map%s",uVar1);
    CSWGuiBorderParams::SetFillImage(local_40,0);
    local_28 = CClientExoApp::GetClientOptions();
    CExoString::Destructor();
    CExoString::Destructor();

## C015
kind: function
address: 0x00414ba0
candidate name: `CSWGuiControl::GetHeight`
facts:
- size 17 B, RET 0
- callers 3: 0x005f9220 CSWPartyTable::CreateParty, 0x00721bb0 CERFFile::WriteResource_2, 0x0085ee80
- twins: none
- decomp: undefined4 __fastcall this_fn(int param_1)
    return *(undefined4 *)(param_1 + 0x10);

## C016
kind: function
address: 0x00727710
candidate name: `CExoBifFile::ReadHeader`
facts:
- size 537 B, RET 4
- callees: 0x00734010 (CALL), 0x00739970 (CALL), 0x00919723 operator new (CALL), 0x00739990 (CALL), 0x009196fd operator delete (CALL), 0x00739b20 (CALL)
- indirect: 0x00727737 slot 0x4, 0x007277d3 slot 0x14, 0x00727814 slot 0x14, 0x00727856 slot 0x14, 0x007278d6 slot 0x14
- twins: none
- decomp: undefined4 __fastcall this_fn(int *param_1)
    if (param_1[0xb] == 0) {
    (**(code **)(*param_1 + 4))();
    if (param_1[5] == 0) {
    FUN_00734010();
    iVar3 = FUN_00739970();
    iVar3 = operator new(0x14);
    iVar3 = FUN_00739990(param_1[0xc],0x14,1);
    if (iVar3 == 0x14) {
    if (*(int *)param_1[0xc] == 0x46464942) {

## C017
kind: function
address: 0x008b2850
candidate name: `CSWGuiContainer::HandleInputEvent`
facts:
- size 357 B, RET 8
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x0073f810 CClientExoApp::GetSWCMessage (CALL), 0x0087a550 (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x007cad30 CGuiInGame::HideC
- twins: none
- decomp: void __thiscall this_fn(int param_1,int param_2,int param_3)
    CSWGuiManager::PlayGuiSound(0);
    CClientExoApp::GetSWCMessage(uVar3,0);
    CClientExoApp::GetInGameGui(iVar1,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4),uVar2);
    CGuiInGame::HideContainerGui();
    CSWGuiManager::PlayGuiSound(0);
    CSWGuiManager::PlayGuiSound(0);
    CSWGuiManager::PlayGuiSound(0);
    CClientExoApp::GetSWCMessage(uVar2,local_8);
    CClientExoApp::GetInGameGui();

## C018
kind: function
address: 0x00729ed0
candidate name: `CSWGuiListBox::OrganizeControls`
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

## C019
kind: function
address: 0x00411b20
candidate name: `CSWGuiPanel::StartLoadFromLayout`
facts:
- size 948 B, RET 4
- callers 1: 0x004113a0
- callees: 0x00919723 operator new (CALL), 0x0040e5a0 CSWGuiPanel::Constructor (CALL), 0x00419740 (CALL), 0x0040fd80 CSWGuiPanel::AddControl (CALL), 0x00710840 (CALL), 0x00416960 CSWGuiTextParams::Con
- indirect: 0x00411d69 slot 0x9c, 0x00411dde slot 0x4, 0x00411e9b slot 0x4, 0x00411ec1 slot 0x34
- strings: 0x00987510 "border1b"
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    void __thiscall this_fn(int param_1,undefined4 param_2)
    uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
    if (*(int *)(param_1 + 0x80) == 0) {
    local_170 = CSWGuiPanel::Constructor(param_1);
    CSWGuiPanel::AddControl(*(undefined4 *)(param_1 + 0x84));
    FUN_00710840("border1b");
    FUN_00710840("border2b");
    uVar3 = CSWGuiTextParams::Constructor();
    CExoString::Destructor();

## C020
kind: function
address: 0x00476b20
candidate name: `CAurInternalGL::RestoreDesktopMode`
facts:
- size 89 B, RET 0
- callees: 0x00000124 (CALL)
- indirect: 0x00476b41 slot 0x24
- imports: ChangeDisplaySettingsA (USER32.DLL)
- twins: none
- decomp: void __fastcall this_fn(int *param_1)
    (**(code **)(*param_1 + 0x24))(param_1[0x2c],1);
    if ((param_1[0x2a] != 0) &&
    (LVar1 = ChangeDisplaySettingsA((DEVMODEA *)(param_1 + 1),2), LVar1 == 0)) {
    ChangeDisplaySettingsA((DEVMODEA *)(param_1 + 1),4);

## C021
kind: function
address: 0x0065afd0
candidate name: `DialogTop::SetReply`
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

## C022
kind: function
address: 0x00656af0
candidate name: `CSWSCombatRound::EndCombatRound`
facts:
- size 13697 B, RET 8
- callers 1: 0x0065f8a0
- callees: 0x005e3de0 (CALL), 0x005e36a0 (CALL), 0x005e3e40 (CALL), 0x00704800 (CALL), 0x0051c3d0 CServerExoApp::SetDebugMode (CALL), 0x0064b210 (CALL), ...
- indirect: 0x00656b68 slot 0x10, 0x00656eeb slot 0x98, 0x00656f3e slot 0x30, 0x00656f4f slot 0x30, 0x00656f74 slot 0x30, ...
- strings: 0x0099876c "b_death", 0x00998774 "b_damage", 0x00998780 "b_attacked", ...
- globals: 0x00a1b490 READ, 0x00a1b4a4 READ, 0x00a1b4a4 READ, ...
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    CServerExoApp::SetDebugMode();
    iVar5 = CClientExoApp::GetSWParty();
    piVar6 = (int *)CClientExoApp::GetSWParty();
    CClientExoApp::GetSWParty();
    CClientExoApp::GetSWParty();
    local_698 = CGameEffect::Constructor();
    local_69c = CGameEffect::Constructor();
    CGameEffect::SetInteger();
    uVar4 = CSWSCreature::GetMaxForcePoints();

## C023
kind: function
address: 0x008b2f70
candidate name: `CSWGuiListBox::ClearItems`
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

## C024
kind: function
address: 0x0085ef00
candidate name: `C2DA::GetINTEntry`
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
