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

## C025
kind: function
address: 0x0047a0c0
candidate name: `CERFString::Destructor`
facts:
- size 71 B, RET 0
- indirect: 0x0047a0f2 slot 0x40
- twins: none
- decomp: undefined4 __fastcall this_fn(int param_1)
    if (*(int *)(param_1 + 0x18) == 0) {
    local_18 = 0;
    else {
    local_18 = (**(code **)(**(int **)(param_1 + 0x18) + 0x40))();

## C026
kind: function
address: 0x00464d10
candidate name: `Scene::AddObject`
facts:
- size 33 B, RET 4
- indirect: 0x00464d29 slot 0xb0
- twins: none
- decomp: void __thiscall this_fn(int *param_1,undefined4 param_2)
    (**(code **)(*param_1 + 0xb0))(param_2);

## C027
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

## C028
kind: function
address: 0x0075b370
candidate name: `CSWGuiMessageBox::Constructor_2`
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

## C029
kind: function
address: 0x005e39d0
candidate name: `CSWMessage::ReadDWORD`
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

## C030
kind: function
address: 0x00769f60
candidate name: `CSWGuiTextParams::SetAlignment`
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

## C031
kind: function
address: 0x00433cd0
candidate name: `GLRender::CreateTexture`
facts:
- size 1493 B, RET 0
- callers 1: 0x004265e0 CAurTextureBasic::glImage
- callees: 0x00484100 (CALL), 0x00484400 (CALL), 0x004288d0 (CALL), 0x009196fd operator delete (CALL), 0x00484a60 (CALL), 0x00000021 (CALL), ...
- indirect: 0x00433ce1 slot 0x4c, 0x00433cf1 slot 0x54, 0x00433d07 slot 0x3c, 0x00433d17 slot 0x5c, 0x00433d27 slot 0x60, ...
- imports: glTexImage2D (OPENGL32.DLL), glTexParameteri (OPENGL32.DLL), glTexSubImage2D (OPENGL32.DLL), ...
- twins: none
- decomp: void this_fn(int *param_1,char param_2,int param_3)
    local_2c = (**(code **)(*param_1 + 0x4c))();
    local_14 = (**(code **)(*param_1 + 0x54))();
    local_38 = (**(code **)(*param_1 + 0x3c))();
    iVar3 = (**(code **)(*param_1 + 0x5c))();
    local_30 = (**(code **)(*param_1 + 0x60))();
    local_28 = (**(code **)(*param_1 + 0x48))();
    local_3c = (param_3 % iVar3) * local_24;
    local_10 = (param_3 / iVar3) * local_40;
    if ((iVar3 == 1) && (local_30 == 1)) {

## C032
kind: function
address: 0x0041fd90
candidate name: `CSWGuiListBox::AddControls`
facts:
- size 326 B, RET 20
- callers 28: 0x0075bf60, 0x007d3060, 0x007d4460, 0x007d5e90, 0x007da9c0, ...
- callees: 0x0041d290 (CALL), 0x004142e0 (CALL), 0x0083ea60 CExoArrayList<uint32>::Add (CALL), 0x00421720 (CALL), 0x0041ea50 (CALL)
- twins: none
- decomp: void __thiscall
    this_fn(int param_1,int param_2,int param_3,uint param_4,uint param_5,uint param_6)
    FUN_0041d290();
    *(uint *)(param_1 + 0x2cc) = *(uint *)(param_1 + 0x2cc) & 0xfffffffd | (param_4 & 1) << 1;
    *(uint *)(param_1 + 0x2cc) = *(uint *)(param_1 + 0x2cc) & 0xfffffffb | (param_5 & 1) << 2;
    for (local_8 = 0; local_8 < param_3; local_8 = local_8 + 1) {
    iVar1 = *(int *)(param_2 + local_8 * 4);
    iVar2 = FUN_004142e0(iVar1);
    if (iVar2 == -1) {
    *(int *)(iVar1 + 0x38) = param_1;

## C033
kind: function
address: 0x00745560
candidate name: `CSWGuiBorder::Load`
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

## C034
kind: function
address: 0x0041e790
candidate name: `CSWGuiListBox::IsOversized`
facts:
- size 97 B, RET 0
- callers 10: 0x0041d940, 0x0041ea50, 0x0041f980, 0x00420850, 0x00420c00, ...
- twins: none
- decomp: undefined4 __fastcall this_fn(int param_1)
    if (((*(int *)(param_1 + 0x2a8) <
    (int)((uint)*(byte *)(param_1 + 0x2d0) + *(int *)(param_1 + 0x2c4))) &&
    (0 < *(int *)(param_1 + 0x2b0))) && ((*(uint *)(param_1 + 0x2cc) >> 8 & 1) == 0)) {
    local_10 = 1;
    else {
    local_10 = 0;

## C035
kind: function
address: 0x0041ea50
candidate name: `CSWGuiListBox::OrganizeControls`
facts:
- size 1823 B, RET 0
- callers 8: 0x0041d710 CSWGuiListBox::SetExtent, 0x0041d940, 0x0041e870, 0x0041e9d0 CSWGuiListBox::SetPadding, 0x0041fc30, ...
- callees: 0x0041c760 (CALL), 0x0041c7c0 (CALL), 0x0041c810 (CALL), 0x0041f170 (CALL), 0x0041e790 (CALL), 0x004178b0 (CALL), ...
- indirect: 0x0041eb68 slot 0x4, 0x0041eb87 slot 0x34, 0x0041edb8 slot 0x50, 0x0041f025 slot 0x4, 0x0041f0fb slot 0x4, ...
- twins: none
- decomp: void __fastcall this_fn(int param_1)
    local_1c = *(int *)(param_1 + 0x2b0);
    if (local_1c == 0) {
    if ((*(uint *)(param_1 + 0x2cc) >> 3 & 1) != 0) {
    FUN_0041c760(1,0);
    FUN_0041c7c0(1);
    FUN_0041c810(1);
    *(uint *)(param_1 + 0x2cc) = *(uint *)(param_1 + 0x2cc) & 0xfffffff7;
    else if ((*(uint *)(param_1 + 0x2cc) >> 8 & 1) == 0) {
    local_18 = (uint)*(byte *)(param_1 + 0x2d0) + *(int *)(param_1 + 0x2c4);

## C036
kind: function
address: 0x00897250
candidate name: `CSWGuiMapHider::SetMapNote`
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
