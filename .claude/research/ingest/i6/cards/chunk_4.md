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

## C37
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

## C38
kind: global
address: 0x009960cc
candidate name: `EffMasteryDark`
facts:
- referenced from 4 function(s)
- in 0x0057eca0 (CSWSCreature::GetMaxForcePoints): iVar3 = FUN_0058bbe0(DAT_009960cc);
- in 0x0058b990: local_20 = (undefined2)DAT_009960cc;
- in 0x0058bb40: if (*(ushort *)(*(int *)(*(int *)(param_1 + 0x148) + local_8 * 4) + 8) == DAT_009960cc) break;

## C39
kind: function
address: 0x0090bbb0
candidate name: `CSWGuiFeatsCharGen::Draw`
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
