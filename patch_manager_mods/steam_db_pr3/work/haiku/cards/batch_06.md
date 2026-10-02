## R162 FUNCTION CSWCDoor::SetState  steam=0x0087ba40
GOG build: param_bytes=1 ghidra_sig='undefined1 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, BYTE state) param_bytes=4
Steam function at this address: size=129 ret_N=4
```c
// FN 0087ba40 FUN_0087ba40 size=129 ret=4 callers=0082adc0,0087b3f0,0080c6b0 callees=

void __thiscall FUN_0087ba40(int *param_1,undefined1 param_2)

{
  byte bVar1;
  int *piVar2;
  
  piVar2 = (int *)(**(code **)(*param_1 + 0x98))(0xff,1);
  *(undefined1 *)(param_1 + 0x48) = param_2;
  if (piVar2 != (int *)0x0) {
    bVar1 = *(byte *)(param_1 + 0x48);
    if (bVar1 == 0) {
      (**(code **)(*piVar2 + 0x90))(0);
    }
    else if ((bVar1 != 0) && (bVar1 < 4)) {
      (**(code **)(*piVar2 + 0x90))(1);
    }
  }
  return;
}

```

## R163 FUNCTION CWorldTimer::SetMinutesPerHour  steam=0x0051a800
GOG build: param_bytes=1 ghidra_sig='undefined1 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, BYTE minutesPerHour) param_bytes=4
Steam function at this address: size=81 ret_N=4
```c
// FN 0051a800 FUN_0051a800 size=81 ret=4 callers=005582f0,007f3040,0051a770 callees=

void __thiscall FUN_0051a800(int param_1,char param_2)

{
  *(char *)(param_1 + 0x38) = param_2;
  if (param_2 == '\0') {
    *(undefined1 *)(param_1 + 0x38) = 5;
  }
  *(uint *)(param_1 + 0x40) = (uint)*(byte *)(param_1 + 0x38) * 0x5a0;
  *(uint *)(param_1 + 0x3c) = (uint)*(byte *)(param_1 + 0x38) * 0x15f900;
  return;
}

```

## R164 FUNCTION CSWGuiControl::GetIsSelectable  steam=0x004190a0
GOG build: param_bytes=0 ghidra_sig='CSWGuiControl * this'
Wrapper typedef (what calling code expects): bool (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=135 ret_N=0
```c
// FN 004190a0 FUN_004190a0 size=135 ret=0 callers= callees=

bool __fastcall FUN_004190a0(int *param_1)

{
  int *piVar1;
  int iVar2;
  
  piVar1 = (int *)param_1[0xe];
  if ((piVar1 == (int *)0x0) || (iVar2 = (**(code **)(*piVar1 + 0x2c))(), iVar2 == 0)) {
    if (((uint)param_1[0x12] >> 3 & 1) != 0) {
      iVar2 = (**(code **)(*param_1 + 0x4c))();
      return iVar2 != 0;
    }
  }
  else {
    iVar2 = (**(code **)(*piVar1 + 0x2c))();
    if ((*(uint *)(iVar2 + 0x48) >> 3 & 1) != 0) {
      iVar2 = (**(code **)(*param_1 + 0x4c))();
      return iVar2 != 0;
    }
  }
  return false;
}

```

## R165 FUNCTION CSWGuiBorder::Constructor  steam=0x00414a50
GOG build: param_bytes=0 ghidra_sig='CSWGuiBorder * this'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=254 ret_N=0
```c
// FN 00414a50 FUN_00414a50 size=254 ret=0 callers=008b3750,008cff80,0041ce90,00420fa0,0089c900,0041ab60,00754ed0,0041a330,00419740,0041b7f0,0041bfd0,00747210,008b0a60,0041a510,0040e5a0,0075a7c0,00420180,00758960 callees=00414920,004143d0,007107f0

undefined4 * __fastcall FUN_00414a50(undefined4 *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 *puVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_009485d8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiDrawable::vftable;
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[2] = 0;
  param_1[1] = 0;
  local_8 = 0;
  *param_1 = CSWGuiBorder::vftable;
  local_1c = 0x3f800000;
  local_18 = 0x3f800000;
  local_14 = 0x3f800000;
  uVar10 = 0;
  uVar9 = 0;
  puVar8 = &local_1c;
  uVar7 = 0x3f800000;
  uVar6 = 0;
  uVar5 = 0;
  uVar2 = FUN_007107f0(0,0,0x3f800000,puVar8,0,0,uVar1);
  uVar3 = FUN_007107f0(uVar2);
  uVar4 = FUN_007107f0(uVar3);
  FUN_004143d0(uVar4,uVar3,uVar2,uVar5,uVar6,uVar7,puVar8,uVar9,uVar10);
  param_1[0x1b] = 0;
  param_1[0x1c] = 0;
  param_1[0x1d] = 0;
  FUN_00414920/*CSWGuiBorderParams::SetBorder*/(param_1);
  ExceptionList = local_10;
  return param_1;
}

```

## R166 FUNCTION CServerExoApp::ClientToServerObjectId  steam=0x0051c8b0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr, DWORD clientId) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 0051c8b0 FUN_0051c8b0 size=28 ret=4 callers=00670760,005be1f0,007f2540,007e8a70,005c7370,008b7910,0051c9f0,005c3460,007c6a20,0056c310,007c7d60,008b6930,005d0f30,006cae20,00781be0,0069b7c0,008dae10,0066ad10,008d4680,007f2500,008b7660,008d6280,007e5730,00865830,008dab80,008d4ac0,008d50e0,00776e40,007f24c0,0058e990,0055d080,007e6ce0,006a93a0,00569320,0077fc10,0069b720,008af2d0,008ba700,005fd9a0,006874f0,008b05e0,007e7ea0,0079d070,008b1250,008d9ee0,00886fc0,008d98f0,0051caa0,0051c6f0,005c3f70,005be950,008b0320,008b29e0,007e9cc0,007c41f0,008b6e90,007e8350,008b08e0,008d5ff0,0079a4c0,00894320,008b09f0,0073f9e0,007e5df0,008ac330,007aae00,0080d050,005bf7f0,0051c910,0074b3f0,007eb460,0079d230,008ad930,007cf4e0,0079ed60,008abe70,005c6340,007eb360,008d95e0,0077c780,005d75b0,006c9c00,007cd0f0,008b61c0 callees=00539d30

void FUN_0051c8b0(undefined4 param_1)

{
  FUN_00539d30(param_1);
  return;
}

```

## R167 FUNCTION CWorldTimer::GetWorldTimeHour  steam=0x0051b100
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=49 ret_N=0
```c
// FN 0051b100 FUN_0051b100 size=49 ret=0 callers=006896d0 callees=0051ab40,0051b0e0

undefined4 __fastcall FUN_0051b100(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined1 *puVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  undefined4 local_14;
  undefined1 local_10 [4];
  undefined1 local_c [4];
  undefined1 local_8 [4];
  
  puVar5 = local_8;
  puVar4 = local_10;
  puVar3 = local_c;
  puVar2 = &local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40/*CWorldTimer::ConvertFromTimeOfDay*/(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_14;
}

```

## R168 OFFSET CSWGuiObject.extent = 0x4
Claim: CSWGuiPanel::Draw 0x40ec90 reads x,y,w,h at this+4..+0x10 and passes them to clip-viewport push 0x479460; live label dump (lesson) shows rect x@+4,y@+8,w@+0xc,h@+0x10
Decompile 0x0040ec90:
```c
// FN 0040ec90 FUN_0040ec90/*CSWGuiPanel::Draw*/ size=432 ret=4 callers=008fb130,007bcab0,008859a0,008a7350,008fe2b0,00760720,0090bbb0,0084fbb0,008b8ee0,008bb260,00892870,008b98d0,008be740,00756b70,008b82a0,0088e510,00918730,0074b3f0,00894cd0,00882080,008b8ae0,00898c20,009091e0,008aba50,008cce70,007fbce0,00907360 callees=004799c0,00479460,004793f0,00410020,004792c0

void __thiscall FUN_0040ec90/*CSWGuiPanel::Draw*/(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (((((*(uint *)(param_1 + 0x48) >> 6 & 1) != 0) && (-1 < *(int *)(param_1 + 4))) &&
      (-1 < *(int *)(param_1 + 8))) &&
     ((0 < *(int *)(param_1 + 0x10) && (0 < *(int *)(param_1 + 0xc))))) {
    if (*(int *)(param_1 + 4) + *(int *)(param_1 + 0xc) <=
        (int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6c)) {
      if (*(int *)(param_1 + 8) + *(int *)(param_1 + 0x10) <=
          (int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6e)) {
        iVar1 = *(int *)(param_1 + 0x28);
        local_14 = *(undefined4 *)(param_1 + 4);
        local_10 = *(undefined4 *)(param_1 + 8);
        local_c = *(undefined4 *)(param_1 + 0xc);
        local_8 = *(undefined4 *)(param_1 + 0x10);
        FUN_00410020/*CSWGuiPanel::GetExtentAccountingForPanelOffset*/(&local_14);
        FUN_004792c0();
        iVar2 = FUN_00479460(local_14,local_10,local_c,local_8,param_1 + 0x54,0,
                             *(undefined4 *)(param_1 + 0x50));
        if (iVar2 != 0) {
          (**(code **)(**(int **)(param_1 + 0x60) + 0xc))(param_2);
          for (local_18 = 0; local_18 < iVar1; local_18 = local_18 + 1) {
            if ((*(int *)(*(int *)(param_1 + 0x24) + local_18 * 4) != 0) &&
               ((*(uint *)(*(int *)(*(int *)(param_1 + 0x24) + local_18 * 4) + 0x48) >> 1 & 1) != 0)
               ) {
              (**(code **)(**(int **)(*(int *)(param_1 + 0x24) + local_18 * 4) + 0x38))(param_2);
            }
          }
          FUN_004799c0();
        }
        FUN_004793f0();
      }
    }
  }
  return;
}

```
Decompile 0x00479460:
```c
// FN 00479460 FUN_00479460 size=845 ret=0 callers=0040ec90,0074b3f0,008859a0,00743280,00895cc0,00894cd0,0041f980,004184f0,00750790,0041bf40,004113a0,007457e0 callees=0000002d,0000004f,0000000e,00000029,0000002c,0000001c,00408740,00517980

/* WARNING: Switch with 1 destination removed at 0x00479701 */
/* WARNING: Switch with 1 destination removed at 0x0047978a */
/* WARNING: Switch with 1 destination removed at 0x00479813 */
/* WARNING: Switch with 1 destination removed at 0x0047989c */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_00479460(int param_1,int param_2,int param_3,int param_4,float *param_5,byte param_6,
            float param_7)

{
  undefined4 uVar1;
  int iVar2;
  uint local_60;
  float local_5c;
  int *local_58;
  float local_54;
  undefined1 *local_50;
  float local_4c;
  undefined1 *local_48;
  float local_44;
  undefined1 *local_40;
  int local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined1 local_20 [4];
  uint local_1c;
  undefined1 local_18 [4];
  short local_14;
  short local_10;
  undefined1 local_c [4];
  int local_8;
  
  if (DAT_00a32a30 == 0) {
    if (((((param_1 + param_3 < 0) || (param_2 + param_4 < 0)) || (param_3 < 1)) ||
        ((param_4 < 1 || (DAT_009f42a4 <= param_1)))) || (DAT_009f42a8 <= param_2)) {
      uVar1 = 0;
    }
    else {
      local_14 = *(short *)(&DAT_00a309c8 + DAT_00a309c0 * 10) + (short)param_1;
      local_10 = *(short *)(&DAT_00a309ca + DAT_00a309c0 * 10) + (short)param_2;
      DAT_00a309c0 = DAT_00a309c0 + 1;
      *(short *)(&DAT_00a309c8 + DAT_00a309c0 * 10) = local_14;
      *(short *)(&DAT_00a309ca + DAT_00a309c0 * 10) = local_10;
... (55 more lines)
```

## R169 FUNCTION CResGFF::GetStructFromStruct  steam=0x00718960
GOG build: param_bytes=12 ghidra_sig='CResStruct * param_1, CResStruct * param_2, char * param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* outStruct, CResStruct* inStruct, char* label) param_bytes=12
Steam function at this address: size=109 ret_N=12
```c
// FN 00718960 FUN_00718960 size=109 ret=12 callers=00420180,005285b0,00853550,005205f0,00703530,004188e0,00418da0,0053fd50,0069d1e0,00833ff0,00836730,00575200,008529e0,00838d00,00528380,0083db30,005e9370,0040ee40,006afed0,00669370,005fbf60,00415080,008384a0,005e6f80,00417480,0040e2c0,00523870,00416720 callees=00717770,007178e0

undefined4 FUN_00718960(int *param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  int *piVar2;
  
  if (((param_1 == (int *)0x0) || (param_2 == 0)) || (param_3 == 0)) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_2,param_3);
    piVar2 = (int *)FUN_00717770(param_2,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 0xe)) {
      uVar1 = 0;
    }
    else {
      *param_1 = piVar2[2];
      uVar1 = 1;
    }
  }
  return uVar1;
}

```

## R170 FUNCTION CSWSObject::HasSpellEffectApplied  steam=0x005476c0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, int spellId) param_bytes=4
Steam function at this address: size=101 ret_N=4
```c
// FN 005476c0 FUN_005476c0 size=101 ret=4 callers=0067e670,00683d40 callees=

undefined4 __thiscall FUN_005476c0(int param_1,int param_2)

{
  int local_8;
  
  if (param_2 != -1) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x14c); local_8 = local_8 + 1) {
      if (*(int *)(*(int *)(*(int *)(param_1 + 0x148) + local_8 * 4) + 0x1c) == param_2) {
        return 1;
      }
    }
  }
  return 0;
}

```

## R171 FUNCTION CSWSDoor::RemoveFromArea  steam=0x0061d040
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=193 ret_N=0
```c
// FN 0061d040 FUN_0061d040 size=193 ret=0 callers=00617190,00616f90,00617b40 callees=00543f50,00476620,0052e4e0,005453c0,00545420,00523090,005e8320

void __fastcall FUN_0061d040(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_005453c0();
  if (iVar1 != 0) {
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_00523090(uVar2);
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0();
    iVar1 = FUN_00476620(uVar2);
    if (iVar1 == 1) {
      uVar2 = *(undefined4 *)(param_1 + 4);
      FUN_005453c0();
      FUN_0052e4e0(uVar2);
    }
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_005e8320(uVar2);
  }
  FUN_00545420(0);
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  FUN_00543f50(&local_10,1,1,0);
  return;
}

```

## R172 FUNCTION CResGFF::WriteFieldCHAR  steam=0x00719a40
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined1 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, char value, char* label) param_bytes=12
Steam function at this address: size=97 ret_N=12
```c
// FN 00719a40 FUN_00719a40 size=97 ret=12 callers=0076aed0,006fed80,0061bfe0,006b3d10,005f18c0,0069cdb0 callees=00717aa0,00718030

undefined4 __thiscall FUN_00719a40(int param_1,int param_2,char param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,1);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(int *)(iVar2 + 8) = (int)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}

```

## R173 FUNCTION CSWCObject::GetName  steam=0x007ed420
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, void* outString) param_bytes=4
Steam function at this address: size=47 ret_N=4
```c
// FN 007ed420 FUN_007ed420 size=47 ret=4 callers= callees=00733570

undefined4 FUN_007ed420(undefined4 param_1)

{
  FUN_00733570/*CExoString::CExoString_char*/(&DAT_00986925);
  return param_1;
}

```

## R174 FUNCTION CResGFF::AddStruct  steam=0x00717f30
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr, DWORD id) param_bytes=4
Steam function at this address: size=251 ret_N=4
```c
// FN 00717f30 FUN_00717f30 size=251 ret=4 callers=00719800,00719940,0071a3d0 callees=0091dba0,009196fd,00919723

uint __thiscall FUN_00717f30(int param_1,undefined4 param_2)

{
  longlong lVar1;
  uint uVar2;
  void *_Dst;
  
  if ((*(int *)(param_1 + 0x40) == 0) || (*(int *)(param_1 + 0x44) == 0)) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = *(uint *)(*(int *)(param_1 + 0x40) + 0xc);
    if (*(uint *)(param_1 + 0x48) <= uVar2) {
      *(uint *)(param_1 + 0x48) = uVar2 + *(int *)(param_1 + 0x28);
      lVar1 = (ulonglong)*(uint *)(param_1 + 0x48) * 0xc;
      _Dst = (void *)FUN_00919723/*operator new*/(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
      _memcpy(_Dst,*(void **)(param_1 + 0x44),uVar2 * 0xc);
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x44));
      *(void **)(param_1 + 0x44) = _Dst;
    }
    *(uint *)(*(int *)(param_1 + 0x40) + 0xc) = uVar2 + 1;
    *(undefined4 *)(uVar2 * 0xc + *(int *)(param_1 + 0x44)) = param_2;
    *(undefined4 *)(*(int *)(param_1 + 0x44) + 4 + uVar2 * 0xc) = 0xffffffff;
    *(undefined4 *)(*(int *)(param_1 + 0x44) + 8 + uVar2 * 0xc) = 0;
  }
  return uVar2;
}

```

## R175 FUNCTION CClientExoApp::GetCreatureByGameObjectID  steam=0x0073f550
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, DWORD objectId) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 0073f550 FUN_0073f550 size=28 ret=4 callers=007f3770,008f79a0,00575df0,008074e0,0078cee0,008ab060,00804f10,005d75b0,00899210,00899380,0069b840,00865830,008652c0,0082b1f0,00691fd0,00690000,007fecc0,00867ca0,008e59c0,0082d950,008079b0,00763110,008aaea0,009074e0,008f65d0,008f7ef0,00902f60,00873f70,00813200,008f7fe0,005f8f90,00899270,007ab040,00899420,007fd840,00828b30,00756b70,007a5fa0,008f7d10,00899300,0069b440,007e5da0,007fe6c0,007c41f0,008f78b0,008f82c0,005f8d80,007e5df0,00807730,007e5ca0,0082b0c0,00696250,007aaec0,005f9220,00770310,0056efd0,007c6670,0076dde0,008f8070,007a2fb0,008994e0,00762f00,00865250,0066c990,008f4af0,008f7e20,0076df50,00804ad0,005ef1d0,0080d050,00669c80,00867b40 callees=0078bdf0

void FUN_0073f550(undefined4 param_1)

{
  FUN_0078bdf0(param_1);
  return;
}

```

## R176 FUNCTION CSWSPlaceable::RemoveFromArea  steam=0x005f2f50
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=143 ret_N=0
```c
// FN 005f2f50 FUN_005f2f50 size=143 ret=0 callers=005ef1d0,005f2fe0,005f2a80 callees=00543f50,005453c0,00545420,005e8320,00523090

void __fastcall FUN_005f2f50(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_005453c0();
  if (iVar1 != 0) {
    if (-1 < *(int *)(param_1 + 0x39c)) {
      uVar2 = *(undefined4 *)(param_1 + 4);
      FUN_005453c0(uVar2);
      FUN_005e8320(uVar2);
      *(undefined4 *)(param_1 + 0x39c) = 0xffffffff;
    }
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_00523090(uVar2);
  }
  FUN_00545420(0);
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  FUN_00543f50(&local_10,1,1,0);
  return;
}

```

## R177 FUNCTION CSWSTrigger::SetCreator  steam=0x00622c10
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD creatorId) param_bytes=4
Steam function at this address: size=124 ret_N=4
```c
// FN 00622c10 FUN_00622c10 size=124 ret=4 callers=00620250,005b5ad0,00694470 callees=0051c8d0,0051c8f0,005fb070

void __thiscall FUN_00622c10(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  *(int *)(param_1 + 0x328) = param_2;
  if (param_2 == 0x7f000000) {
LAB_00622c79:
    *(undefined4 *)(param_1 + 0x354) = 0;
  }
  else {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    iVar1 = FUN_0051c8f0();
    if (param_2 != iVar1) {
      FUN_0051c8d0/*CServerExoApp::GetPartyTable (inline at *(this+4)+0x1F0B4)*/(param_2,param_1,*(undefined4 *)(DAT_00a1b4a4 + 8),uVar2);
      iVar1 = FUN_005fb070/*CSWPartyTable::GetIsNPCObjectMember(objId) (PC -> 0)*/(param_2);
      if (iVar1 == 0) goto LAB_00622c79;
    }
    *(undefined4 *)(param_1 + 0x354) = 1;
  }
  return;
}

```

## R178 FUNCTION CResGFF::Constructor  steam=0x00716e60
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=312 ret_N=0
```c
// FN 00716e60 FUN_00716e60 size=312 ret=0 callers=005fa3b0,00529dd0,005f7f30,00632220,0055b830,005f8130,005f8320,00655010,005fe4b0,00638bd0,0052e640,0055cc70,008bf280,00561240,005f9ff0,005fb1a0,00630810,0053af20,0055db00,005f8710,00534600,0076aed0,005302e0,00631560,005f8520,0055d7f0 callees=0091c630,007110d0

undefined4 * __fastcall FUN_00716e60(undefined4 *param_1)

{
  FUN_007110d0/*CRes::CRes*/();
  *param_1 = CResGFF::vftable;
  param_1[0x10] = 0;
  param_1[0x11] = 0;
  param_1[0x12] = 0;
  param_1[0x13] = 0;
  param_1[0x14] = 0;
  param_1[0x15] = 0;
  param_1[0x16] = 0;
  param_1[0x17] = 0;
  param_1[0x18] = 0;
  param_1[0x19] = 0;
  param_1[0x1a] = 0;
  param_1[0x1b] = 0;
  param_1[0x1c] = 0;
  param_1[0x1d] = 0;
  param_1[0x1e] = 0;
  param_1[10] = 0x6e;
  param_1[0xb] = 0x286;
  param_1[0xc] = 0x62;
  param_1[0xd] = 0x72c;
  param_1[0xe] = 0xfd4;
  param_1[0xf] = 0xfd4;
  *(undefined1 *)(param_1 + 0x23) = 0;
  param_1[0x25] = 0;
  param_1[0x26] = 0;
  param_1[0x27] = 0;
  _memset((void *)((int)param_1 + 0x8d),0,4);
  return param_1;
}

```

## R179 FUNCTION Gob::IsInViewVolume  steam=0x004598d0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): unsigned char (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=17 ret_N=0
```c
// FN 004598d0 FUN_004598d0 size=17 ret=0 callers= callees=

undefined1 __fastcall FUN_004598d0(int param_1)

{
  return *(undefined1 *)(param_1 + 0x11);
}

```

## R180 FUNCTION CClientExoApp::GetObjectName  steam=0x0073f0e0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD clientId, void* outName) param_bytes=8
Steam function at this address: size=32 ret_N=8
```c
// FN 0073f0e0 FUN_0073f0e0 size=32 ret=8 callers=00802f00 callees=0078eba0

void FUN_0073f0e0(undefined4 param_1,undefined4 param_2)

{
  FUN_0078eba0(param_1,param_2);
  return;
}

```

## R181 FUNCTION CSWSDoor::GetIsLinked  steam=0x0061d000
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=50 ret_N=0
```c
// FN 0061d000 FUN_0061d000 size=50 ret=0 callers=005ce9d0,005c3f70,00557ff0,0063d0c0,0061d620,00617b40,0054cd50,005c3460,005bfc00 callees=

undefined4 __fastcall FUN_0061d000(int param_1)

{
  undefined4 uVar1;
  
  if ((*(char *)(param_1 + 0x3d4) == '\x01') || (*(char *)(param_1 + 0x3d4) == '\x02')) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

```

## R182 OFFSET CSWGuiPanel.color = 0x54
Claim: Draw 0x40ec90 passes this+0x54 as the float* colour of clip push 0x479460 (lesson #81 signature x,y,w,h,float* color,byte,alpha)
Decompile 0x0040ec90:
```c
// FN 0040ec90 FUN_0040ec90/*CSWGuiPanel::Draw*/ size=432 ret=4 callers=008fb130,007bcab0,008859a0,008a7350,008fe2b0,00760720,0090bbb0,0084fbb0,008b8ee0,008bb260,00892870,008b98d0,008be740,00756b70,008b82a0,0088e510,00918730,0074b3f0,00894cd0,00882080,008b8ae0,00898c20,009091e0,008aba50,008cce70,007fbce0,00907360 callees=004799c0,00479460,004793f0,00410020,004792c0

void __thiscall FUN_0040ec90/*CSWGuiPanel::Draw*/(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (((((*(uint *)(param_1 + 0x48) >> 6 & 1) != 0) && (-1 < *(int *)(param_1 + 4))) &&
      (-1 < *(int *)(param_1 + 8))) &&
     ((0 < *(int *)(param_1 + 0x10) && (0 < *(int *)(param_1 + 0xc))))) {
    if (*(int *)(param_1 + 4) + *(int *)(param_1 + 0xc) <=
        (int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6c)) {
      if (*(int *)(param_1 + 8) + *(int *)(param_1 + 0x10) <=
          (int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6e)) {
        iVar1 = *(int *)(param_1 + 0x28);
        local_14 = *(undefined4 *)(param_1 + 4);
        local_10 = *(undefined4 *)(param_1 + 8);
        local_c = *(undefined4 *)(param_1 + 0xc);
        local_8 = *(undefined4 *)(param_1 + 0x10);
        FUN_00410020/*CSWGuiPanel::GetExtentAccountingForPanelOffset*/(&local_14);
        FUN_004792c0();
        iVar2 = FUN_00479460(local_14,local_10,local_c,local_8,param_1 + 0x54,0,
                             *(undefined4 *)(param_1 + 0x50));
        if (iVar2 != 0) {
          (**(code **)(**(int **)(param_1 + 0x60) + 0xc))(param_2);
          for (local_18 = 0; local_18 < iVar1; local_18 = local_18 + 1) {
            if ((*(int *)(*(int *)(param_1 + 0x24) + local_18 * 4) != 0) &&
               ((*(uint *)(*(int *)(*(int *)(param_1 + 0x24) + local_18 * 4) + 0x48) >> 1 & 1) != 0)
               ) {
              (**(code **)(**(int **)(*(int *)(param_1 + 0x24) + local_18 * 4) + 0x38))(param_2);
            }
          }
          FUN_004799c0();
        }
        FUN_004793f0();
      }
    }
  }
  return;
}

```
Decompile 0x00479460:
```c
// FN 00479460 FUN_00479460 size=845 ret=0 callers=0040ec90,0074b3f0,008859a0,00743280,00895cc0,00894cd0,0041f980,004184f0,00750790,0041bf40,004113a0,007457e0 callees=0000002d,0000004f,0000000e,00000029,0000002c,0000001c,00408740,00517980

/* WARNING: Switch with 1 destination removed at 0x00479701 */
/* WARNING: Switch with 1 destination removed at 0x0047978a */
/* WARNING: Switch with 1 destination removed at 0x00479813 */
/* WARNING: Switch with 1 destination removed at 0x0047989c */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_00479460(int param_1,int param_2,int param_3,int param_4,float *param_5,byte param_6,
            float param_7)

{
  undefined4 uVar1;
  int iVar2;
  uint local_60;
  float local_5c;
  int *local_58;
  float local_54;
  undefined1 *local_50;
  float local_4c;
  undefined1 *local_48;
  float local_44;
  undefined1 *local_40;
  int local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined1 local_20 [4];
  uint local_1c;
  undefined1 local_18 [4];
  short local_14;
  short local_10;
  undefined1 local_c [4];
  int local_8;
  
  if (DAT_00a32a30 == 0) {
    if (((((param_1 + param_3 < 0) || (param_2 + param_4 < 0)) || (param_3 < 1)) ||
        ((param_4 < 1 || (DAT_009f42a4 <= param_1)))) || (DAT_009f42a8 <= param_2)) {
      uVar1 = 0;
    }
    else {
      local_14 = *(short *)(&DAT_00a309c8 + DAT_00a309c0 * 10) + (short)param_1;
      local_10 = *(short *)(&DAT_00a309ca + DAT_00a309c0 * 10) + (short)param_2;
      DAT_00a309c0 = DAT_00a309c0 + 1;
      *(short *)(&DAT_00a309c8 + DAT_00a309c0 * 10) = local_14;
      *(short *)(&DAT_00a309ca + DAT_00a309c0 * 10) = local_10;
... (55 more lines)
```

## R183 FUNCTION CClientExoApp::SetVideoMode  steam=0x0073fbd0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD modeNum) param_bytes=4
Steam function at this address: size=30 ret_N=4
```c
// FN 0073fbd0 FUN_0073fbd0 size=30 ret=4 callers=007d9280 callees=0078dbf0

void FUN_0073fbd0(undefined4 param_1)

{
  FUN_0078dbf0(param_1,0);
  return;
}

```

## R184 FUNCTION CGuiInGame::GetPendingStatusSummary  steam=0x007cf210
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=84 ret_N=0
```c
// FN 007cf210 FUN_007cf210 size=84 ret=0 callers=00781be0 callees=

undefined4 __fastcall FUN_007cf210(int param_1)

{
  undefined4 local_c;
  
  if (*(int *)(param_1 + 0xb8) == 0) {
    local_c = 0;
  }
  else if (((*(uint *)(*(int *)(param_1 + 0xb8) + 0x68) & 1) == 0) &&
          ((*(uint *)(*(int *)(param_1 + 0xb8) + 0x68) >> 1 & 1) == 0)) {
    local_c = 0;
  }
  else {
    local_c = 1;
  }
  return local_c;
}

```

## R185 FUNCTION Gob::EnableRenderGobBBox  steam=0x0045bed0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector* color) param_bytes=4
Steam function at this address: size=55 ret_N=4
```c
// FN 0045bed0 FUN_0045bed0 size=55 ret=4 callers= callees=

void __thiscall FUN_0045bed0(int param_1,undefined4 *param_2)

{
  *(undefined2 *)(param_1 + 0xf4) = 1;
  *(undefined4 *)(param_1 + 0x174) = *param_2;
  *(undefined4 *)(param_1 + 0x178) = param_2[1];
  *(undefined4 *)(param_1 + 0x17c) = param_2[2];
  return;
}

```

## R186 FUNCTION CWorldTimer::CompareWorldTimes  steam=0x0051afe0
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD lhsDay, DWORD lhsMS, DWORD rhsDay, DWORD rhsMS) param_bytes=16
Steam function at this address: size=100 ret_N=16
```c
// FN 0051afe0 FUN_0051afe0 size=100 ret=16 callers=007efac0,0051af60,0051e220,0051b3e0,005bfc00,0084bee0,00545520,0084a900,00544ad0,0051ed10,005c3f70,007efc40 callees=

undefined4 __thiscall FUN_0051afe0(int param_1,uint param_2,uint param_3,uint param_4,uint param_5)

{
  undefined4 uVar1;
  
  if ((param_3 < *(uint *)(param_1 + 0x3c)) && (param_5 < *(uint *)(param_1 + 0x3c))) {
    if (param_2 == param_4) {
      if (param_5 < param_3) {
        uVar1 = 1;
      }
      else if (param_3 < param_5) {
        uVar1 = 0xffffffff;
      }
      else {
        uVar1 = 0;
      }
    }
    else if (param_4 < param_2) {
      uVar1 = 1;
    }
    else {
      uVar1 = 0xffffffff;
    }
  }
  else {
    uVar1 = 0xfffffffe;
  }
  return uVar1;
}

```

## R187 FUNCTION Scene::SetFog  steam=0x00464c30
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int fog) param_bytes=4
Steam function at this address: size=22 ret_N=4
```c
// FN 00464c30 FUN_00464c30 size=22 ret=4 callers= callees=

void __thiscall FUN_00464c30(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x60) = param_2;
  return;
}

```

## R188 FUNCTION CResGFF::GetField  steam=0x00717770
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): GFFFieldData* (__thiscall)(void* thisPtr, CResGFFStruct* structData, DWORD index) param_bytes=8
Steam function at this address: size=171 ret_N=8
```c
// FN 00717770 FUN_00717770 size=171 ret=8 callers=00719800,00718b80,007190e0,00718810,00718a00,00718c00,00718a80,00718710,00718890,00718d80,00718e40,00718b00,00718d00,007193d0,00718960,00718f90,00719640,00718760,00719520,00718c80 callees=00717720

int __thiscall FUN_00717770(int param_1,undefined4 param_2,uint param_3)

{
  int iVar1;
  
  iVar1 = FUN_00717720(param_2);
  if (((iVar1 == 0) || (*(int *)(param_1 + 0x4c) == 0)) || (*(uint *)(iVar1 + 8) <= param_3)) {
    iVar1 = 0;
  }
  else if (*(int *)(iVar1 + 8) == 1) {
    if (*(uint *)(iVar1 + 4) < *(uint *)(*(int *)(param_1 + 0x40) + 0x14)) {
      iVar1 = *(int *)(iVar1 + 4) * 0xc + *(int *)(param_1 + 0x4c);
    }
    else {
      iVar1 = 0;
    }
  }
  else if ((*(int *)(param_1 + 100) == 0) ||
          (*(uint *)(*(int *)(param_1 + 0x40) + 0x2c) < *(int *)(iVar1 + 4) + 4 + param_3 * 4)) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(*(int *)(param_1 + 100) + *(int *)(iVar1 + 4) + param_3 * 4) * 0xc +
            *(int *)(param_1 + 0x4c);
  }
  return iVar1;
}

```
