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

## C085
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

## C086
kind: function
address: 0x009454b0
candidate name: `CResTGA::OnServiced`
facts:
- size 905 B, RET 0
- callees: 0x00945860 (CALL), 0x00945990 (CALL)
- indirect: 0x0094579a slot 0xc
- twins: none
- decomp: undefined4 __fastcall this_fn(int *param_1)
    if (param_1[4] == 0) {
    else if (param_1[0xd] == 0) {
    if (param_1[0x12] == 0) {
    if (param_1[0x11] == 0) {
    if (param_1[0x10] == 0) {
    param_1[0x10] = param_1[4] + 0x12 + (uint)*(byte *)param_1[0x12];
    uVar3 = *(ushort *)(param_1[0x12] + 5);
    bVar1 = *(byte *)(param_1[0x12] + 2);
    switch(bVar1) {

## C087
kind: function
address: 0x0041d290
candidate name: `CSWGuiListBox::ClearItems`
facts:
- size 408 B, RET 0
- callers 68: 0x0041d190, 0x0041fc30, 0x0041fd90, 0x00758a70, 0x0075a7c0, ...
- indirect: 0x0041d2d2 slot 0x6c, 0x0041d335 slot 0x0, 0x0041d3c1 slot 0x4
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    if ((*(uint *)(param_1 + 0x2cc) >> 1 & 1) == 0) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x2b0); local_8 = local_8 + 1) {
    if ((*(uint *)(param_1 + 0x2cc) >> 2 & 1) != 0) {
    (**(code **)(**(int **)(*(int *)(param_1 + 0x2ac) + local_8 * 4) + 4))
    (local_8 * 0x10 + *(int *)(param_1 + 0x2b8));
    *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x2ac) + local_8 * 4) + 0x38) = 0;
    if ((*(int *)(param_1 + 0x2ec) != 0) &&
    (iVar2 = (**(code **)(**(int **)(param_1 + 0x2ec) + 0x6c))(), iVar2 == 0)) {
    *(undefined4 *)(param_1 + 0x2ec) = 0;

## C088
kind: function
address: 0x00897250
candidate name: `CExoPtrList::SetAllocated`
facts:
- size 97 B, RET 4
- callers 1: 0x00895260
- callees: 0x007a1720 (CALL), 0x00561430 (CALL), 0x0058c370 (CALL)
- twins: none
- decomp: void __thiscall this_fn(int param_1,int param_2)
    local_8 = FUN_007a1720();
    while( true ) {
    if (local_8 == 0) {
    piVar1 = (int *)FUN_00561430(local_8);
    if (param_2 == *piVar1) break;
    FUN_0058c370(&local_8);
    *(int *)(param_1 + 0x24c) = local_8;

## C089
kind: function
address: 0x0041f7f0
candidate name: `CSWMessage::ReadDWORD`
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

## C090
kind: function
address: 0x0075bf60
candidate name: `CSWGuiMessageBox::AddMessageLabel_2`
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

## C091
kind: function
address: 0x0041bb20
candidate name: `CSWGuiProgressBar::Draw`
facts:
- size 67 B, RET 4
- indirect: 0x0041bb3d slot 0xc, 0x0041bb5b slot 0xc
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 param_2)
    (**(code **)(*(int *)(param_1 + 0x6c) + 0xc))(param_2);
    (**(code **)(*(int *)(param_1 + 0xe4) + 0xc))(param_2);

## C092
kind: function
address: 0x0046d230
candidate name: `CAurScene::Animate`
facts:
- size 331 B, RET 4
- indirect: 0x0046d2ad slot 0x14, 0x0046d31a slot 0x14, 0x0046d367 slot 0x14
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 param_2)
    if ((DAT_009f5d40 == 0) || (DAT_00a2004c == '\0')) {
    local_8 = *(int *)(param_1 + 0x58);
    while (iVar1 = local_8 + -1, local_8 != 0) {
    if (iVar1 < *(int *)(param_1 + 0x58)) {
    (**(code **)(**(int **)(*(int *)(param_1 + 0x54) + iVar1 * 4) + 0x14))(param_2);
    local_8 = *(int *)(param_1 + 0x110);
    while (iVar1 = local_8 + -1, local_8 != 0) {
    if (iVar1 < *(int *)(param_1 + 0x110)) {
    (**(code **)(**(int **)(*(int *)(param_1 + 0x10c) + iVar1 * 4) + 0x14))(param_2);

## C093
kind: function
address: 0x00449320
candidate name: `CExoPtrList::SetAllocated`
facts:
- size 151 B, RET 4
- callers 5: 0x004491c0, 0x00449230, 0x00474030, 0x00474370, 0x00507fa0
- callees: 0x0091d288 operator new[] (CALL), 0x0091c6aa operator delete[] (CALL)
- twins: none
- decomp: void __thiscall this_fn(int *param_1,int param_2)
    iVar1 = *param_1;
    param_1[2] = param_2;
    iVar2 = operator new[](-(uint)((int)((ulonglong)(uint)param_1[2] * 4 >> 0x20) != 0) |
    (uint)((ulonglong)(uint)param_1[2] * 4));
    *param_1 = iVar2;
    for (local_c = 0; local_c < param_1[1]; local_c = local_c + 1) {
    *(undefined4 *)(*param_1 + local_c * 4) = *(undefined4 *)(iVar1 + local_c * 4);
    if (iVar1 != 0) {
    operator delete[](iVar1);

## C094
kind: global
address: 0x0099d39c
candidate name: `CExoLinkedListInternal::Remove`
facts:
- referenced from 1 function(s)
- in 0x0073a130: 0073a13f 689cd39900 PUSH 0x99d39c

## C095
kind: function
address: 0x006fa530
candidate name: `CPathfindInformation::ResetAtEndOfPath`
facts:
- size 72 B, RET 0
- callers 5: 0x0055e610 CSWSModule::SetIntraAreaGoal, 0x0057fce0 CSWSCreature::UpdatePersonalSpace, 0x005bfc00 CSWSCreature::AIActionMoveToPoint, 0x00656af0, 0x0065a170
- callees: 0x0091c860 (CALL)
- twins: none
- decomp: /* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
    void __fastcall this_fn(int param_1)
    if (*(float *)(param_1 + 4) == (float)_DAT_00986b38) {
    uVar1 = FUN_0091c860();
    *(undefined4 *)(param_1 + 0x6c) = uVar1;
    else {
    uVar1 = FUN_0091c860();
    *(undefined4 *)(param_1 + 0x6c) = uVar1;

## C096
kind: function
address: 0x00432fa0
candidate name: `GLRender::CreateCubeTexture`
facts:
- size 883 B, RET 0
- callers 1: 0x004265e0 CAurTextureBasic::glImage
- callees: 0x00484100 (CALL), 0x004264b0 (CALL), 0x00000042 (CALL), 0x00000041 (CALL), 0x009196fd operator delete (CALL), 0x00000021 (CALL)
- indirect: 0x00432fcd slot 0x48, 0x00432fdd slot 0x4c, 0x00432fed slot 0x54, 0x0043301d slot 0x3c, 0x0043303e slot 0x5c, ...
- imports: glTexImage2D (OPENGL32.DLL), glTexParameteri (OPENGL32.DLL), glTexSubImage2D (OPENGL32.DLL), ...
- twins: none
- decomp: void this_fn(int *param_1,char param_2,int param_3)
    for (local_8 = 0; local_8 < 6; local_8 = local_8 + 1) {
    iVar2 = (**(code **)(*param_1 + 0x48))();
    iVar3 = (**(code **)(*param_1 + 0x4c))();
    iVar4 = (**(code **)(*param_1 + 0x54))();
    iVar5 = *(int *)(&DAT_009f50c0 + local_8 * 4) + 0x8515;
    iVar6 = (**(code **)(*param_1 + 0x3c))();
    iVar7 = (**(code **)(*param_1 + 0x5c))();
    iVar8 = (**(code **)(*param_1 + 0x60))();
    iVar9 = (**(code **)(*param_1 + 0x5c))();
