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
address: 0x008ddf20
candidate name: `CSWGuiSwoopUpgrade::OnUpgradeSelected`
facts:
- size 950 B, RET 4
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x0051c8d0 (CALL), 0x005f7a30 CSWPartyTable::GetPartyItems (CALL), 0x0051c8f0 CServerExoApp::GetPlayerCreatureId (CALL), 0x00600380 CSWSItem::
- twins: none
- decomp: void __thiscall this_fn(int param_1,int param_2)
    CSWGuiManager::PlayGuiSound(0);
    local_24 = CSWPartyTable::GetPartyItems();
    uVar2 = CServerExoApp::GetPlayerCreatureId(0,0,0x7f000000);
    CSWSItem::SetPossessor(uVar2,uVar4,uVar5,uVar6);
    local_30 = CExoArrayList::IndexOf(*(undefined4 *)(param_1 + 0x436c + local_1c * 4));
    CItemRepository::AddItem(param_1 + 0x436c + local_1c * 4,1,0,0);
    CItemRepository::AddItem(param_1 + 0x436c + local_1c * 4,1,0,0);
    local_28 = CServerExoApp::GetItemByGameObjectID(*(undefined4 *)(local_20 + 0x1d0));
    local_30 = CExoArrayList::IndexOf(local_28);

## C02
kind: function
address: 0x00499300
candidate name: `LightManager::~LightManager`
facts:
- size 327 B, RET 0
- callers 1: 0x00499610
- callees: 0x00473fb0 (CALL), 0x00473f40 (CALL), 0x00513fa0 (CALL)
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    void this_fn(int param_1,int param_2)
    if (*(int *)(param_1 + 0xe0) == 0) {
    FUN_00473fb0(param_1 + 0xd4);
    *(undefined4 *)(param_2 + 4) = 0;
    if ((*(short *)(param_1 + 0xe4) == 0) && (DAT_009f624c != 0)) {
    FUN_00473f40(*(undefined4 *)(param_1 + 0xe0));
    piVar4 = (int *)(DAT_00a32a48 + 0x22c);
    for (local_8 = 0; local_8 < *(int *)(*(int *)(param_1 + 0xe0) + 0x60); local_8 = local_8 + 1)
    iVar1 = *(int *)(*(int *)(*(int *)(param_1 + 0xe0) + 0x5c) + local_8 * 4);

## C03
kind: function
address: 0x0074ac30
candidate name: `CSWGuiSwoopUpgrade::OnUpgradeSelected`
facts:
- size 113 B, RET 0
- callers 901: 0x0074ad90, 0x007563b0, 0x007d70d0, 0x0084df70 CSWGuiInGameCharacter::Destructor_2, 0x0088c110, ...
- callees: 0x00417290 (CALL), 0x00414c60 CSWGuiBorder::Destructor_2 (CALL), 0x00418640 CSWGuiControl::Destructor (CALL)
- twins: none
- decomp: /* Library Function - Single Match
    public: virtual __thiscall CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton(void)
    Library: Visual Studio 2010 Debug */
    void __thiscall
    CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton(CMFCToolBarButtonsListButton *this)
    uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
    *(undefined ***)this = CSWGuiLabel::vftable;
    FUN_00417290(uVar1);
    CSWGuiBorder::Destructor_2();
    CSWGuiControl::Destructor();

## C04
kind: function
address: 0x00499300
candidate name: `CSWGuiBlackenedLabel::~CSWGuiBlackenedLabel`
facts:
- size 327 B, RET 0
- callers 1: 0x00499610
- callees: 0x00473fb0 (CALL), 0x00473f40 (CALL), 0x00513fa0 (CALL)
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    void this_fn(int param_1,int param_2)
    if (*(int *)(param_1 + 0xe0) == 0) {
    FUN_00473fb0(param_1 + 0xd4);
    *(undefined4 *)(param_2 + 4) = 0;
    if ((*(short *)(param_1 + 0xe4) == 0) && (DAT_009f624c != 0)) {
    FUN_00473f40(*(undefined4 *)(param_1 + 0xe0));
    piVar4 = (int *)(DAT_00a32a48 + 0x22c);
    for (local_8 = 0; local_8 < *(int *)(*(int *)(param_1 + 0xe0) + 0x60); local_8 = local_8 + 1)
    iVar1 = *(int *)(*(int *)(*(int *)(param_1 + 0xe0) + 0x5c) + local_8 * 4);

## C05
kind: function
address: 0x008ddf20
candidate name: `CSWGuiBlackenedLabel::~CSWGuiBlackenedLabel`
facts:
- size 950 B, RET 4
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x0051c8d0 (CALL), 0x005f7a30 CSWPartyTable::GetPartyItems (CALL), 0x0051c8f0 CServerExoApp::GetPlayerCreatureId (CALL), 0x00600380 CSWSItem::
- twins: none
- decomp: void __thiscall this_fn(int param_1,int param_2)
    CSWGuiManager::PlayGuiSound(0);
    local_24 = CSWPartyTable::GetPartyItems();
    uVar2 = CServerExoApp::GetPlayerCreatureId(0,0,0x7f000000);
    CSWSItem::SetPossessor(uVar2,uVar4,uVar5,uVar6);
    local_30 = CExoArrayList::IndexOf(*(undefined4 *)(param_1 + 0x436c + local_1c * 4));
    CItemRepository::AddItem(param_1 + 0x436c + local_1c * 4,1,0,0);
    CItemRepository::AddItem(param_1 + 0x436c + local_1c * 4,1,0,0);
    local_28 = CServerExoApp::GetItemByGameObjectID(*(undefined4 *)(local_20 + 0x1d0));
    local_30 = CExoArrayList::IndexOf(local_28);

## C06
kind: function
address: 0x0074ac30
candidate name: `CSWGuiBlackenedLabel::~CSWGuiBlackenedLabel`
facts:
- size 113 B, RET 0
- callers 901: 0x0074ad90, 0x007563b0, 0x007d70d0, 0x0084df70 CSWGuiInGameCharacter::Destructor_2, 0x0088c110, ...
- callees: 0x00417290 (CALL), 0x00414c60 CSWGuiBorder::Destructor_2 (CALL), 0x00418640 CSWGuiControl::Destructor (CALL)
- twins: none
- decomp: /* Library Function - Single Match
    public: virtual __thiscall CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton(void)
    Library: Visual Studio 2010 Debug */
    void __thiscall
    CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton(CMFCToolBarButtonsListButton *this)
    uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
    *(undefined ***)this = CSWGuiLabel::vftable;
    FUN_00417290(uVar1);
    CSWGuiBorder::Destructor_2();
    CSWGuiControl::Destructor();
