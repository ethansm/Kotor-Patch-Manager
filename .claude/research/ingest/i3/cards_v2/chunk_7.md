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

## C073
kind: function
address: 0x0074b3a0
candidate name: `GLRender::FrameBufferModifications`
facts:
- size 68 B, RET 8
- callees: 0x007433b0 (CALL), 0x0040eae0 CSWGuiPanel::HitCheckMouse (CALL)
- twins: none
- decomp: int this_fn(undefined4 param_1,undefined4 param_2)
    local_8 = FUN_007433b0(param_1,param_2);
    if (local_8 == 0) {
    local_8 = CSWGuiPanel::HitCheckMouse(param_1,param_2);

## C074
kind: function
address: 0x008b29e0
candidate name: `CSWGuiContainer::SetContainer`
facts:
- size 1410 B, RET 8
- callers 2: 0x007cacb0, 0x008b2850
- callees: 0x0084a640 (CALL), 0x0051c8b0 CServerExoApp::ClientToServerObjectId (CALL), 0x0051c160 CServerExoApp::GetPlaceableByGameObjectID (CALL), 0x00840ef0 (CALL), 0x00606160 CItemRepository::ItemL
- indirect: 0x008b2bef slot 0xa0, 0x008b2ece slot 0x88, 0x008b2ee5 slot 0x8, 0x008b2f07 slot 0x8
- strings: 0x0099d780 "UseSearch"
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ, 0x00a1b4d0 READ
- twins: none
- decomp: void __thiscall this_fn(int *param_1,int param_2,undefined1 param_3)
    uVar2 = CServerExoApp::ClientToServerObjectId(param_1[0x1a]);
    local_24 = CServerExoApp::GetPlaceableByGameObjectID(uVar2);
    uVar2 = CItemRepository::ItemListGetItemObjectID(local_60);
    local_28 = CClientExoApp::ServerToClientObjectId(uVar2);
    local_d0 = CSWGuiInGameItemEntry::Constructor();
    CSWGuiInGameItemEntry::SetItem(local_28,0,0);
    CExoString::CStrConstructor("UseSearch");
    CExoString::Destructor();
    CExoString::CStrConstructor(&DAT_00986bfc);

## C075
kind: function
address: 0x005f7350
candidate name: `CSWGuiListBox::OrganizeControls`
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

## C076
kind: function
address: 0x00453860
candidate name: `MdlNode::ControlDispatch`
facts:
- size 679 B, RET 16
- callers 4: 0x00456be0, 0x00463900, 0x004dfed0, 0x004fbbd0
- callees: 0x004def00 (CALL), 0x004544e0 (CALL), 0x004df010 (CALL), 0x004544b0 (CALL), 0x004df050 (CALL), 0x00454420 (CALL), ...
- twins: none
- decomp: void __thiscall
    this_fn(short *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
    iVar1 = (int)*param_1;
    if (iVar1 < 0x62) {
    if (iVar1 == 0x61) {
    FUN_00454540(param_2,param_3,param_4,param_5);
    FUN_004df120(param_2,param_3,param_4,param_5);
    switch(iVar1) {
    FUN_004def00(param_2,param_3,param_4,param_5);
    FUN_004544e0(param_2,param_3,param_4,param_5);

## C077
kind: function
address: 0x00744c20
candidate name: `CSWGuiCharButton::UpdateToolTipText`
facts:
- size 731 B, RET 0
- callers 2: 0x00744b40, 0x00744b90
- callees: 0x0071dee0 (CALL), 0x00734010 (CALL), 0x00734270 CExoString::Format (CALL), 0x00733780 CExoString::Destructor (CALL)
- strings: 0x0099d7c0 "%s %s\n%s: %d/%d\n%s: %d/%d\n%s: %d\n%s: %d"
- globals: 0x00a1b4b0 READ
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    CExoString::Format(param_1 + 0x1d4,"%s %s\n%s: %d/%d\n%s: %d\n%s: %d",uVar2);
    CExoString::Destructor();
    CExoString::Destructor();
    CExoString::Destructor();
    CExoString::Format(param_1 + 0x1d4,"%s %s\n%s: %d/%d\n%s: %d/%d\n%s: %d\n%s: %d",uVar2);
    CExoString::Destructor();
    CExoString::Destructor();
    CExoString::Destructor();
    CExoString::Destructor();

## C078
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

## C079
kind: function
address: 0x008bc100
candidate name: `GLRender::CreateTexture`
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

## C080
kind: function
address: 0x00420180
candidate name: `CSWGuiListBox::LoadProtoItem`
facts:
- size 691 B, RET 8
- callers 1: 0x0041ffd0 CSWGuiListBox::Load
- callees: 0x00718960 CResGFF::GetStructFromStruct (CALL), 0x00718c80 CResGFF::ReadFieldINT (CALL), 0x00919723 operator new (CALL), 0x00419740 (CALL), 0x00414a50 CSWGuiBorder::Constructor (CALL), 0x00
- strings: 0x00987ed4 "CONTROLTYPE", 0x00987ee0 "PROTOITEM"
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 param_2,undefined4 param_3)
    uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
    iVar2 = CResGFF::GetStructFromStruct(local_14,param_3,"PROTOITEM");
    if (iVar2 != 0) {
    uVar3 = CResGFF::ReadFieldINT(local_14,"CONTROLTYPE",local_18,0);
    *(uint *)(param_1 + 0x2cc) = *(uint *)(param_1 + 0x2cc) & 0xfffe1fff | (uVar3 & 0xf) << 0xd;
    switch(*(uint *)(param_1 + 0x2cc) >> 0xd & 0xf) {
    CSWGuiBorder::Constructor();
    CExoString::CStrConstructor("PROTOITEM");
    CExoString::Destructor();

## C081
kind: function
address: 0x00479b10
candidate name: `CSWGuiProgressBar::Draw`
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

## C082
kind: function
address: 0x008d4f60
candidate name: `PowersLevelUp::OnInputEvent`
facts:
- size 108 B, RET 4
- callers 1: 0x008d4260
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x008d65d0 (CALL), 0x008d6850 (CALL)
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    if (*(int *)(param_1 + 0x3ed8) == 0) {
    *(undefined4 *)(param_1 + 0x3ed8) = 1;
    else {
    *(undefined4 *)(param_1 + 0x3ed8) = 0;
    uVar1 = *(undefined4 *)(param_1 + 0x1c);
    CSWGuiManager::PlayGuiSound(0);
    if ((*(uint *)(param_1 + 0x3634) >> 1 & 1) == 0) {
    FUN_008d6850(param_1,uVar1);
    FUN_008d65d0();

## C083
kind: function
address: 0x0075bd50
candidate name: `CSWGuiMessageBox::SetMessage`
facts:
- size 132 B, RET 8
- callers 1: 0x00898dc0
- callees: 0x0071ebd0 (CALL), 0x00416e30 CSWGuiTextParams::SetText (CALL), 0x00416e10 CSWGuiTextParams::ReSetFont (CALL), 0x0075c080 (CALL), 0x00733780 CExoString::Destructor (CALL)
- globals: 0x00a1b4b0 READ
- twins: none
- decomp: void this_fn(void)
    puStack_c = &LAB_00964078;
    local_10 = ExceptionList;
    uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
    ExceptionList = &local_10;
    FUN_0071ebd0(&stack0x00000004);
    CSWGuiTextParams::SetText(&stack0x00000004);
    CSWGuiTextParams::ReSetFont(uVar1,uVar2);
    FUN_0075c080();
    CExoString::Destructor();

## C084
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
