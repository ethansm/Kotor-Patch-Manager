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

## C13
kind: function
address: 0x00464d10
candidate name: `Scene::DoGobShadows`
facts:
- size 33 B, RET 4
- indirect: 0x00464d29 slot 0xb0
- twins: none
- decomp: void __thiscall this_fn(int *param_1,undefined4 param_2)
    (**(code **)(*param_1 + 0xb0))(param_2);

## C14
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

## C15
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

## C16
kind: function
address: 0x0090bbb0
candidate name: `IODispatcher::ReadSync_4`
facts:
- size 244 B, RET 4
- callees: 0x00416e30 CSWGuiTextParams::SetText (CALL), 0x00417010 CSWGuiTextParams::SetColor (CALL), 0x00760ea0 (CALL), 0x0040ec90 CSWGuiPanel::Draw (CALL)
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 param_2)
    bVar1 = *(byte *)(param_1 + 0x1c04);
    if (1 < bVar1) {
    if (bVar1 < 4) {
    CSWGuiTextParams::SetText(param_1 + 0x1bec);
    CSWGuiTextParams::SetColor(&DAT_00a7fde8);
    *(undefined4 *)(param_1 + 0x1200) = DAT_00a7fd70;
    CSWGuiTextParams::SetText(param_1 + 0x1be4);
    CSWGuiTextParams::SetColor(&DAT_00a7fde8);
    CSWGuiPanel::Draw(param_2);

## C17
kind: function
address: 0x00414ba0
candidate name: `CSWGuiFeatsCharGen::Draw`
facts:
- size 17 B, RET 0
- callers 3: 0x005f9220 CSWPartyTable::CreateParty, 0x00721bb0 CERFFile::WriteResource_2, 0x0085ee80
- twins: none
- decomp: undefined4 __fastcall this_fn(int param_1)
    return *(undefined4 *)(param_1 + 0x10);

## C18
kind: function
address: 0x0071b8b0
candidate name: `Scene::DoGobShadows`
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

## C19
kind: function
address: 0x0047a0c0
candidate name: `GetFontTexture`
facts:
- size 71 B, RET 0
- indirect: 0x0047a0f2 slot 0x40
- twins: none
- decomp: undefined4 __fastcall this_fn(int param_1)
    if (*(int *)(param_1 + 0x18) == 0) {
    local_18 = 0;
    else {
    local_18 = (**(code **)(**(int **)(param_1 + 0x18) + 0x40))();

## C20
kind: function
address: 0x0040e480
candidate name: `GetFontTexture`
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

## C21
kind: function
address: 0x00652810
candidate name: `C2DA::Free`
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

## C22
kind: function
address: 0x00750790
candidate name: `CClientExoApp::GetCharacterChangeInProgress`
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

## C23
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

## C24
kind: function
address: 0x009091e0
candidate name: `CSWGuiPowersLevelUp::Draw`
facts:
- size 240 B, RET 4
- callees: 0x00416e30 CSWGuiTextParams::SetText (CALL), 0x00417010 CSWGuiTextParams::SetColor (CALL), 0x00760ea0 (CALL), 0x0040ec90 CSWGuiPanel::Draw (CALL)
- twins: none
- decomp: void __thiscall this_fn(int param_1,undefined4 param_2)
    cVar1 = *(char *)(param_1 + 0x1c08);
    if (cVar1 != '\x02') {
    if (cVar1 == '\x03') {
    CSWGuiTextParams::SetText(param_1 + 0x1bb8);
    CSWGuiTextParams::SetColor(&DAT_00a7fde8);
    *(undefined4 *)(param_1 + 0x1440) = DAT_00a7fd70;
    CSWGuiTextParams::SetText(param_1 + 0x1bc0);
    CSWGuiTextParams::SetColor(&DAT_00a7fde8);
    CSWGuiPanel::Draw(param_2);
