## R135 FUNCTION CClientExoApp::GetLastTarget  steam=0x00740660
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 00740660 FUN_00740660 size=31 ret=0 callers=00865830,008e3e60,00597750,008e4340 callees=

undefined4 __fastcall FUN_00740660(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x2b4);
}

```

## R136 FUNCTION Gob::TurnOnShadows  steam=0x00459870
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=26 ret_N=0
```c
// FN 00459870 FUN_00459870 size=26 ret=0 callers= callees=

void __fastcall FUN_00459870(int param_1)

{
  *(undefined2 *)(param_1 + 0xee) = 1;
  return;
}

```

## R137 OFFSET CSWGuiBorderParams.color = 0x14
Claim: Load stores COLOR (3 floats) at border+0x28..0x30 = params+0x14..0x1c; Border::Draw passes border+0x28 as the rgb pointer

## R138 OFFSET CRes.vtable = 0x0
Claim: ctor 0x7110d0 stores CRes::vftable (RTTI symbol) at *this
Decompile 0x007110d0:
```c
// FN 007110d0 FUN_007110d0/*CRes::CRes*/ size=151 ret=0 callers=006f3db0,00703b40,00941f10,00716e60,00945ce0,009461c0,0055db00,006f3cf0,008f8430,00729ed0,0072a0a0,00941e70,00716fd0,00943ec0,00941900,009452c0,00941a90,00941fb0,007228f0,00942050 callees=

undefined4 * __fastcall FUN_007110d0/*CRes::CRes*/(undefined4 *param_1)

{
  *param_1 = CRes::vftable;
  *(undefined2 *)(param_1 + 1) = 0;
  param_1[2] = 0xffffffff;
  *(undefined2 *)((int)param_1 + 6) = 0;
  param_1[6] = 0;
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[5] = 0;
  param_1[7] = 0;
  param_1[8] = 0;
  param_1[9] = 0;
  param_1[3] = param_1[3] & 0xfffffffb;
  param_1[3] = param_1[3] & 0xfffffff7;
  return param_1;
}

```

## R139 FUNCTION CSWSObject::GetAcceptableAction  steam=0x005d7190
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD action) param_bytes=4
Steam function at this address: size=62 ret_N=4
```c
// FN 005d7190 FUN_005d7190 size=62 ret=4 callers=0067b100,0053f3c0 callees=

undefined4 FUN_005d7190(undefined4 param_1)

{
  undefined4 uVar1;
  
  switch(param_1) {
  case 1:
  case 7:
  case 9:
  case 0xc:
  case 0xf:
  case 0x14:
  case 0x15:
  case 0x18:
  case 0x19:
  case 0x1a:
  case 0x1b:
  case 0x1c:
  case 0x1d:
  case 0x1e:
  case 0x26:
  case 0x27:
  case 0x28:
  case 0x29:
  case 0x2a:
  case 0x2b:
  case 0x2e:
  case 0x32:
  case 0x36:
  case 0x37:
  case 0x38:
  case 0x3d:
  case 0x3f:
  case 0x43:
  case 0x44:
  case 0x45:
  case 0x46:
  case 0x47:
    uVar1 = 1;
    break;
  default:
    uVar1 = 0;
  }
  return uVar1;
}

```

## R140 FUNCTION Scene::LoadVisibility  steam=0x0046fbd0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, char* visibility) param_bytes=4
Steam function at this address: size=330 ret_N=4
```c
// FN 0046fbd0 FUN_0046fbd0 size=330 ret=4 callers= callees=0046fb80,0046feb0,0091c7ad,00475ea0,00475f70,009196ee,00475ab0

void __thiscall FUN_0046fbd0(int *param_1,undefined4 param_2)

{
  undefined1 local_a4 [68];
  int local_60;
  undefined1 local_5c [64];
  uint local_1c;
  int local_18;
  int local_14;
  char *local_10;
  int local_c;
  int local_8;
  
  local_1c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  FUN_0046fb80();
  local_8 = FUN_00475ab0(param_2,&DAT_0098bda8,0,1);
  if (local_8 != 0) {
    while (local_10 = (char *)FUN_00475f70(), local_10 != (char *)0x0) {
      local_5c[0] = 0;
      local_18 = 0;
      FID_conflict__sscanf(local_10,"%s%d",local_5c,&local_18);
      local_14 = (**(code **)(*param_1 + 0x74))(local_5c);
      if ((local_14 != 0) && (local_18 != 0)) {
        for (local_c = 0; local_c < local_18; local_c = local_c + 1) {
          local_a4[0] = 0;
          local_10 = (char *)FUN_00475f70();
          FID_conflict__sscanf(local_10,"%s",local_a4);
          local_60 = (**(code **)(*param_1 + 0x74))(local_a4);
          if (local_60 != 0) {
            FUN_0046feb0(local_14,local_60,1);
          }
        }
      }
    }
    FUN_00475ea0(local_8,0);
  }
  __security_check_cookie(local_1c ^ (uint)&stack0xfffffffc);
  return;
}

```

## R141 OFFSET CSWGuiBorder.border_params = 0x14
Claim: CSWGuiBorder::Load 0x415080 stores DIMENSION at this+0x14, INNEROFFSET +0x18, INNEROFFSETY +0x1c, COLOR +0x28.. = embedded CSWGuiBorderParams starting at +0x14 (params ctor 0x4143d0 layout identical)
Decompile 0x004143d0:
```c
// FN 004143d0 FUN_004143d0 size=286 ret=36 callers=008bb5d0,008b0fb0,008d0470,00895cc0,00411b20,00895620,008b38e0,00414a50,008a5ba0 callees=00710ca0,007107f0

undefined4 * __thiscall
FUN_004143d0(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            uint param_5,undefined4 param_6,undefined4 param_7,undefined4 *param_8,
            undefined4 param_9,undefined4 param_10)

{
  undefined1 local_34 [16];
  undefined1 local_24 [16];
  undefined1 local_14 [16];
  
  param_1[5] = 0;
  param_1[6] = 0;
  param_1[7] = 0;
  FUN_007107f0();
  FUN_007107f0();
  FUN_007107f0();
  FUN_00710ca0(local_14,param_2);
  FUN_00710ca0(local_24,param_3);
  FUN_00710ca0(local_34,param_4);
  param_1[8] = param_1[8] & 0xfffffffc | param_5 & 3;
  *param_1 = param_6;
  param_1[4] = param_7;
  param_1[8] = param_1[8] & 0xfffffffb;
  param_1[5] = *param_8;
  param_1[6] = param_8[1];
  param_1[7] = param_8[2];
  param_1[8] = param_1[8] & 0xfffffff7;
  param_1[8] = param_1[8] & 0xffffffef;
  param_1[0x15] = 0;
  param_1[3] = 0;
  param_1[1] = param_9;
  param_1[2] = param_10;
  return param_1;
}

```
Decompile 0x00415080:
```c
// FN 00415080 FUN_00415080/*CSWGuiBorder::Load*/ size=639 ret=12 callers= callees=00414760,00734010,00718c80,00719520,00718960,00414680,00710840,00718a00,004149f0,00718e40,00414840

void __thiscall FUN_00415080/*CSWGuiBorder::Load*/(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  undefined1 local_88 [12];
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined1 local_6d [16];
  undefined1 local_5d [16];
  undefined1 local_4d [16];
  undefined1 local_3d [16];
  undefined1 local_2d [16];
  undefined1 local_1d [16];
  byte local_d;
  undefined1 local_c [4];
  undefined1 local_8 [4];
  
  uVar1 = FUN_00734010();
  iVar2 = FUN_00718960/*CResGFF::GetStructFromStruct*/(local_8,param_3,uVar1);
  if (iVar2 != 0) {
    FUN_00710840(&DAT_00986925);
    uVar1 = FUN_00718e40/*CResGFF::ReadFieldCResRef*/(local_2d,local_8,"CORNER",local_c,local_1d);
    FUN_00414680/*CSWGuiBorderParams::SetCornerImage*/(uVar1,0);
    FUN_00710840(&DAT_00986925);
    uVar1 = FUN_00718e40/*CResGFF::ReadFieldCResRef*/(local_4d,local_8,&DAT_0098762c,local_c,local_3d);
    FUN_00414760/*CSWGuiBorderParams::SetEdgeImage*/(uVar1,0);
    FUN_00710840(&DAT_00986925);
    uVar1 = FUN_00718e40/*CResGFF::ReadFieldCResRef*/(local_6d,local_8,&DAT_00987624,local_c,local_5d);
    FUN_00414840/*CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force*/(uVar1,0);
    uVar3 = FUN_00718c80/*CResGFF::ReadFieldINT*/(local_8,"FILLSTYLE",local_c,0);
    *(uint *)(param_1 + 0x34) = *(uint *)(param_1 + 0x34) & 0xfffffffc | uVar3 & 3;
    uVar1 = FUN_00718c80/*CResGFF::ReadFieldINT*/(local_8,"DIMENSION",local_c,0);
    *(undefined4 *)(param_1 + 0x14) = uVar1;
    uVar1 = FUN_00718c80/*CResGFF::ReadFieldINT*/(local_8,"INNEROFFSET",local_c,0);
    *(undefined4 *)(param_1 + 0x18) = uVar1;
    uVar1 = FUN_00718c80/*CResGFF::ReadFieldINT*/(local_8,"INNEROFFSETY",local_c,0);
    *(undefined4 *)(param_1 + 0x1c) = uVar1;
    local_7c = 0x3f800000;
    local_78 = 0x3f800000;
    local_74 = 0x3f800000;
    puVar4 = (undefined4 *)FUN_00719520/*CResGFF::ReadFieldVector*/(local_88,local_8,"COLOR",local_c,&local_7c);
    *(undefined4 *)(param_1 + 0x28) = *puVar4;
    *(undefined4 *)(param_1 + 0x2c) = puVar4[1];
    *(undefined4 *)(param_1 + 0x30) = puVar4[2];
... (6 more lines)
```

## R142 FUNCTION CClientExoApp::SetCanSendDialog  steam=0x00740420
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int canSend) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 00740420 FUN_00740420 size=28 ret=4 callers=00533830,0052fda0,00534600 callees=0079c550

void FUN_00740420(undefined4 param_1)

{
  FUN_0079c550(param_1);
  return;
}

```

## R143 FUNCTION CResGFF::WriteFieldBYTE  steam=0x007199d0
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined1 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, BYTE value, char* label) param_bytes=12
Steam function at this address: size=97 ret_N=12
```c
// FN 007199d0 FUN_007199d0 size=97 ret=12 callers=0058c960,00602dd0,0053c680,0053e4c0,005e2700,00529dd0,005f18c0,00520190,00574500,0062b5f0,00603390,00561c90,00629eb0,005fb1a0,005e5b40,0062dec0,0052b0d0,005e9510,0061bfe0,0053af20,0055bd10,00534600,00592310,0055d080,00629040,00621ba0,0053fce0,0076aed0,006b4ea0,0058d2f0,006b3d10,006272c0,0055d7f0,0069cdb0,0053c890 callees=00717aa0,00718030

undefined4 __thiscall FUN_007199d0(int param_1,int param_2,byte param_3,int param_4)

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
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,0);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(uint *)(iVar2 + 8) = (uint)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}

```

## R144 FUNCTION CClientExoApp::SetVideoMode  steam=0x0073fbb0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD modeNum) param_bytes=4
Steam function at this address: size=31 ret_N=0
```c
// FN 0073fbb0 FUN_0073fbb0 size=31 ret=0 callers=0086ea60,008f19e0,008f1040 callees=

undefined4 __fastcall FUN_0073fbb0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0xa4);
}

```

## R145 FUNCTION Gob::LoadAddInAnimations  steam=0x0045a030
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, char* modelName) param_bytes=4
Steam function at this address: size=277 ret_N=4
```c
// FN 0045a030 FUN_0045a030 size=277 ret=4 callers= callees=0048e160,0044b440,0091caeb,00452d40,0048e2a0,0091cff0,0091d000,009196ee

void __thiscall FUN_0045a030(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int local_50;
  char local_4c [64];
  uint local_c;
  FILE *local_8;
  
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  uVar1 = FUN_0048e160(param_2);
  *(undefined4 *)(param_1 + 0x90) = uVar1;
  if (*(int *)(param_1 + 0x90) == 0) {
    FUN_0091cff0(local_4c,param_2);
    FUN_0091d000(local_4c,&DAT_0098b9a8);
    local_8 = _fopen(local_4c,"r");
    if (local_8 == (FILE *)0x0) goto LAB_0045a135;
    uVar1 = FUN_0048e2a0(local_8,0);
    *(undefined4 *)(param_1 + 0x90) = uVar1;
    FUN_0044b440(*(undefined4 *)(param_1 + 0x84));
    for (local_50 = 0; local_50 < *(int *)(*(int *)(param_1 + 0x84) + 0x5c); local_50 = local_50 + 1
        ) {
      FUN_0044b440(*(undefined4 *)(param_1 + 0x84));
    }
  }
  FUN_00452d40(param_1);
LAB_0045a135:
  __security_check_cookie(local_c ^ (uint)&stack0xfffffffc);
  return;
}

```

## R146 OFFSET CSWGuiPanel.bit_flags = 0x48
Claim: SetVisible 0x40fe60 sets/clears bit 2 of this+0x48; Draw tests bit 6; CenterPanel 0x40ea00 sets bit 0; HUD gate reads &0x600 (lesson #300)
Decompile 0x0040ea00:
```c
// FN 0040ea00 FUN_0040ea00/*CSWGuiPanel::CenterPanel*/ size=149 ret=0 callers=0089c5f0,0075ae40,0075b370,008bfb60,007d5d50,007cfca0,008b1ea0,008c19f0,008c0cb0,0075c080,008bee10,008586d0 callees=

void __fastcall FUN_0040ea00/*CSWGuiPanel::CenterPanel*/(int param_1)

{
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  *(int *)(param_1 + 4) =
       ((int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6c) - *(int *)(param_1 + 0xc)) / 2;
  *(int *)(param_1 + 8) =
       ((int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6e) - *(int *)(param_1 + 0x10)) / 2;
  local_8 = *(undefined4 *)(param_1 + 0x10);
  local_c = *(undefined4 *)(param_1 + 0xc);
  local_14 = 0;
  local_10 = 0;
  (**(code **)(**(int **)(param_1 + 0x60) + 4))(&local_14);
  *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) | 1;
  return;
}

```
Decompile 0x0040fe60:
```c
// FN 0040fe60 FUN_0040fe60/*CSWGuiPanel::SetVisible*/ size=59 ret=4 callers=0090b360,007cbb40,0090e990,0090fb50,00913650,007be4c0,00908ae0,0090b430,009047c0,00908b50,00411ee0,00904780,00912790,009047f0,00919230 callees=00411170

void __thiscall FUN_0040fe60/*CSWGuiPanel::SetVisible*/(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffffb | (param_2 & 1) << 2;
  if (*(int *)(param_1 + 0x1c) != 0) {
    FUN_00411170();
  }
  return;
}

```

## R147 FUNCTION CResGFF::AddField  steam=0x00718030
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): CResGFFField* (__thiscall)(void* thisPtr, CResStruct* structPtr, char* labelText, GFFFieldTypes fieldType) param_bytes=12
Steam function at this address: size=443 ret_N=12
```c
// FN 00718030 FUN_00718030 size=443 ret=12 callers=00719bf0,00719b90,00719940,00719d40,00719ab0,0071a260,00719ce0,00719a40,00719740,0071a320,007199d0,0071a1b0,00719c50,00719e50,00719f30,00719b20 callees=007181f0,0091dba0,009196fd,00717720,00919723,00718470

int __thiscall FUN_00718030(int param_1,undefined4 param_2,int param_3,undefined4 param_4)

{
  uint uVar1;
  int iVar2;
  longlong lVar3;
  int iVar4;
  void *_Dst;
  undefined4 uVar5;
  undefined4 *local_10;
  
  if (param_3 == 0) {
    iVar4 = 0;
  }
  else {
    iVar4 = FUN_00717720(param_2);
    if (iVar4 == 0) {
      iVar4 = 0;
    }
    else {
      uVar1 = *(uint *)(*(int *)(param_1 + 0x40) + 0x14);
      if (*(uint *)(param_1 + 0x50) <= uVar1) {
        *(uint *)(param_1 + 0x50) = uVar1 + *(int *)(param_1 + 0x2c);
        lVar3 = (ulonglong)*(uint *)(param_1 + 0x50) * 0xc;
        _Dst = (void *)FUN_00919723/*operator new*/(-(uint)((int)((ulonglong)lVar3 >> 0x20) != 0) | (uint)lVar3);
        _memcpy(_Dst,*(void **)(param_1 + 0x4c),uVar1 * 0xc);
        FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x4c));
        *(void **)(param_1 + 0x4c) = _Dst;
      }
      *(uint *)(*(int *)(param_1 + 0x40) + 0x14) = uVar1 + 1;
      iVar2 = *(int *)(iVar4 + 8);
      if (iVar2 == 0) {
        *(uint *)(iVar4 + 4) = uVar1;
      }
      else {
        if (iVar2 == 1) {
          local_10 = (undefined4 *)FUN_00718470/*CResGFF::AddDataLayoutField*/(0xffffffff,0,8);
          if (local_10 == (undefined4 *)0x0) {
            return 0;
          }
          *local_10 = *(undefined4 *)(iVar4 + 4);
        }
        else {
          local_10 = (undefined4 *)FUN_00718470/*CResGFF::AddDataLayoutField*/(*(undefined4 *)(iVar4 + 4),iVar2 << 2,iVar2 * 4 + 4)
          ;
          if (local_10 == (undefined4 *)0x0) {
            return 0;
          }
        }
        local_10[iVar2] = uVar1;
        *(int *)(iVar4 + 4) = (int)local_10 - *(int *)(param_1 + 100);
      }
      *(int *)(iVar4 + 8) = iVar2 + 1;
      uVar5 = FUN_007181f0/*CResGFF::AddLabel*/(param_3);
      *(undefined4 *)(*(int *)(param_1 + 0x4c) + 4 + uVar1 * 0xc) = uVar5;
      *(undefined4 *)(uVar1 * 0xc + *(int *)(param_1 + 0x4c)) = param_4;
      *(undefined4 *)(*(int *)(param_1 + 0x4c) + 8 + uVar1 * 0xc) = 0xffffffff;
      iVar4 = uVar1 * 0xc + *(int *)(param_1 + 0x4c);
    }
  }
  return iVar4;
}

```

## R148 FUNCTION CSWGuiPanel::AddControl  steam=0x0040fd80
GOG build: param_bytes=4 ghidra_sig='CSWGuiPanel * this, CSWGuiControl * param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* control) param_bytes=4
Steam function at this address: size=57 ret_N=4
```c
// FN 0040fd80 FUN_0040fd80 size=57 ret=4 callers=008bb5d0,0075ae40,0075b370,00753d60,00747210,00411b20,00893950,00760f40,00892090 callees=0083ea60,004142e0

void __thiscall FUN_0040fd80(undefined4 param_1,int param_2)

{
  int iVar1;
  
  iVar1 = FUN_004142e0(param_2);
  if (iVar1 == -1) {
    *(undefined4 *)(param_2 + 0x38) = param_1;
    FUN_0083ea60/*CExoArrayList<uint32>::Add*/(param_2);
  }
  return;
}

```

## R149 FUNCTION CClientExoApp::GetGUIString  steam=0x007408f0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, void* outString, DWORD strRef) param_bytes=8
Steam function at this address: size=53 ret_N=8
```c
// FN 007408f0 FUN_007408f0 size=53 ret=8 callers=00607790,007b40f0,0060e6b0,0060fdd0,0060ea50,007fae60,008c7cd0,00607a70,0060a990,008d0780,0087bec0,00617b40,0060da50,008f6490,008b1250,00608f30,007fefb0,0087c590,0060d8e0,0077bcf0,0077e1f0,008b2f70,00742a00,00614e10,008b0750,00756b70,00609110,0074cc90,0060a4f0,006140c0,0087e8a0,0060fee0,0060a0d0,00842d40,0060b6b0,00843500,0087ecd0,00619c90,0060e0d0,008b29e0,008ac330,0074ec10,00778940,008af2d0,00610fc0,008495c0,008b3c00,00608420,008b98f0,008b9590,008ccb80,0060b360,00609cb0,008ddaf0,0074b3f0,00779850,0077a9e0,008acdd0,0075dd10 callees=0079e210

undefined4 FUN_007408f0(undefined4 param_1,undefined4 param_2)

{
  FUN_0079e210(param_1,param_2);
  return param_1;
}

```

## R150 FUNCTION CGuiInGame::PrevSWInGameGui  steam=0x007ca3c0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=389 ret_N=0
```c
// FN 007ca3c0 FUN_007ca3c0 size=389 ret=0 callers=007568b0 callees=00733570,0073fea0,00410670,00410530,007d0760,00410780,006fd8d0,00733780,00756860

void __fastcall FUN_007ca3c0(int param_1)

{
  undefined4 local_2c;
  undefined1 local_20 [8];
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096fe28;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x128) != 0) {
    local_18 = FUN_0073fea0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (((DAT_00a10760 != 0) && (*(int *)(param_1 + 0x30) != 0)) && (local_18 != 0)) {
      FUN_00410670(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
    }
    local_14 = *(undefined4 *)(param_1 + 0x2c);
    *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + -1;
    if (*(int *)(param_1 + 0x2c) < 0) {
      *(undefined4 *)(param_1 + 0x2c) = 7;
    }
    if ((*(int *)(param_1 + 0x30) == 0) || (local_18 == 0)) {
      local_2c = 0xffffffff;
    }
    else {
      local_2c = *(undefined4 *)(param_1 + 0x2c);
    }
    FUN_007d0760(local_14,local_2c);
    if ((*(int *)(param_1 + 0x30) != 0) && (local_18 != 0)) {
      FUN_00410530/*CSWGuiManager::AddPanel*/(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4),2,1);
      FUN_00410780(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
      FUN_00756860(*(undefined4 *)(param_1 + 0x2c),1);
      if (DAT_00a1b4a8 != 0) {
        FUN_00733570/*CExoString::CExoString_char*/("k_pend_screenchg");
        local_8 = 0;
        FUN_006fd8d0(local_20,0x7f000000,0);
        local_8 = 0xffffffff;
        FUN_00733780/*CExoString::~CExoString*/();
      }
    }
  }
  ExceptionList = local_10;
  return;
}

```

## R151 FUNCTION Gob::RemoveAnimationFlag  steam=0x004e3160
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int flag) param_bytes=4
Steam function at this address: size=82 ret_N=4
```c
// FN 004e3160 FUN_004e3160 size=82 ret=4 callers= callees=

void __thiscall FUN_004e3160(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0xe0) + -1;
  if (((-1 < iVar1) &&
      (iVar1 = *(int *)(*(int *)(param_1 + 0xdc) + iVar1 * 4), *(int *)(iVar1 + 8) != 0)) &&
     (param_2 != 0)) {
    *(int *)(iVar1 + 8) = *(int *)(iVar1 + 8) - param_2;
  }
  return;
}

```

## R152 FUNCTION CSWSPlaceable::SetOrientation  steam=0x005f6990
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* orientation) param_bytes=4
Steam function at this address: size=111 ret_N=4
```c
// FN 005f6990 FUN_005f6990 size=111 ret=4 callers=00527890,0066f990,00549780 callees=005143c0,00543f10,0046ce40

void __thiscall FUN_005f6990(int param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  undefined1 local_1c [12];
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  *(undefined4 *)(param_1 + 0x480) = *param_2;
  *(undefined4 *)(param_1 + 0x484) = param_2[1];
  *(undefined4 *)(param_1 + 0x488) = param_2[2];
  *(undefined4 *)(param_1 + 0x48c) = param_2[3];
  FUN_0046ce40(&local_10);
  local_8 = 0;
  puVar1 = (undefined4 *)FUN_005143c0(local_1c,&local_10);
  local_10 = *puVar1;
  local_c = puVar1[1];
  local_8 = puVar1[2];
  FUN_00543f10(&local_10);
  return;
}

```

## R153 FUNCTION Scene::EnableVisibilityGraph  steam=0x0046fb10
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): bool (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=44 ret_N=0
```c
// FN 0046fb10 FUN_0046fb10 size=44 ret=0 callers= callees=

bool __fastcall FUN_0046fb10(int param_1)

{
  *(undefined2 *)(param_1 + 0xe4) = 0;
  return *(int *)(param_1 + 0xe0) != 0;
}

```

## R154 FUNCTION CResGFF::AddStruct  steam=0x00717aa0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr, DWORD id) param_bytes=4
Steam function at this address: size=1157 ret_N=0
```c
// FN 00717aa0 FUN_00717aa0 size=1157 ret=0 callers=00719bf0,00719800,00719b90,00719940,00719d40,00719ab0,0071a260,00719ce0,00719a40,00719740,0071a320,007199d0,0071a1b0,0071a3d0,00719c50,00719e50,00719f30,00719b20 callees=0091dba0,009196fd,00717630,00919723

void __fastcall FUN_00717aa0(int param_1)

{
  longlong lVar1;
  undefined4 *puVar2;
  void *_Dst;
  void *_Dst_00;
  void *_Dst_01;
  void *_Dst_02;
  void *_Dst_03;
  void *_Dst_04;
  
  puVar2 = (undefined4 *)FUN_00919723/*operator new*/(0x38);
  puVar2[2] = 0;
  puVar2[4] = 0;
  puVar2[6] = 0;
  puVar2[8] = 0;
  puVar2[10] = 0;
  puVar2[0xc] = 0;
  if (*(int *)(param_1 + 0x40) == 0) {
    *puVar2 = 0;
    puVar2[1] = 0;
    puVar2[3] = 0;
    *(undefined4 *)(param_1 + 0x48) = 0x6e;
    puVar2[5] = 0;
    *(undefined4 *)(param_1 + 0x50) = 0x286;
    puVar2[7] = 0;
    *(undefined4 *)(param_1 + 0x58) = 0x62;
    puVar2[9] = 0;
    *(undefined4 *)(param_1 + 0x60) = 0x72c;
    puVar2[0xb] = 0;
    *(undefined4 *)(param_1 + 0x68) = 0xfd4;
    puVar2[0xd] = 0;
    *(undefined4 *)(param_1 + 0x74) = 0xfd4;
  }
  else {
    *puVar2 = **(undefined4 **)(param_1 + 0x40);
    puVar2[1] = *(undefined4 *)(*(int *)(param_1 + 0x40) + 4);
    puVar2[3] = *(undefined4 *)(*(int *)(param_1 + 0x40) + 0xc);
    *(int *)(param_1 + 0x48) = *(int *)(*(int *)(param_1 + 0x40) + 0xc) + *(int *)(param_1 + 0x28);
    puVar2[5] = *(undefined4 *)(*(int *)(param_1 + 0x40) + 0x14);
    *(int *)(param_1 + 0x50) = *(int *)(*(int *)(param_1 + 0x40) + 0x14) + *(int *)(param_1 + 0x2c);
    puVar2[7] = *(undefined4 *)(*(int *)(param_1 + 0x40) + 0x1c);
    *(int *)(param_1 + 0x58) = *(int *)(*(int *)(param_1 + 0x40) + 0x1c) + *(int *)(param_1 + 0x30);
    puVar2[9] = *(undefined4 *)(*(int *)(param_1 + 0x40) + 0x24);
    *(int *)(param_1 + 0x60) = *(int *)(*(int *)(param_1 + 0x40) + 0x24) + *(int *)(param_1 + 0x34);
    puVar2[0xb] = *(undefined4 *)(*(int *)(param_1 + 0x40) + 0x2c);
    *(int *)(param_1 + 0x68) = *(int *)(*(int *)(param_1 + 0x40) + 0x2c) + *(int *)(param_1 + 0x38);
    puVar2[0xd] = *(undefined4 *)(*(int *)(param_1 + 0x40) + 0x34);
    *(int *)(param_1 + 0x74) = *(int *)(*(int *)(param_1 + 0x40) + 0x34) + *(int *)(param_1 + 0x3c);
  }
  lVar1 = (ulonglong)*(uint *)(param_1 + 0x48) * 0xc;
  _Dst = (void *)FUN_00919723/*operator new*/(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
  if (*(int *)(param_1 + 0x44) != 0) {
    _memcpy(_Dst,*(void **)(param_1 + 0x44),*(int *)(*(int *)(param_1 + 0x40) + 0xc) * 0xc);
  }
  lVar1 = (ulonglong)*(uint *)(param_1 + 0x50) * 0xc;
  _Dst_00 = (void *)FUN_00919723/*operator new*/(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
  if (*(int *)(param_1 + 0x4c) != 0) {
    _memcpy(_Dst_00,*(void **)(param_1 + 0x4c),*(int *)(*(int *)(param_1 + 0x40) + 0x14) * 0xc);
  }
  lVar1 = (ulonglong)*(uint *)(param_1 + 0x58) * 0x10;
  _Dst_01 = (void *)FUN_00919723/*operator new*/(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
  if (*(int *)(param_1 + 0x54) != 0) {
    _memcpy(_Dst_01,*(void **)(param_1 + 0x54),*(int *)(*(int *)(param_1 + 0x40) + 0x1c) << 4);
  }
  _Dst_02 = (void *)FUN_00919723/*operator new*/(*(undefined4 *)(param_1 + 0x60));
  if (*(int *)(param_1 + 0x5c) != 0) {
... (32 more lines)
```

## R155 FUNCTION Scene::GetNumRooms  steam=0x0046ff70
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=20 ret_N=0
```c
// FN 0046ff70 FUN_0046ff70 size=20 ret=0 callers= callees=

undefined4 __fastcall FUN_0046ff70(int param_1)

{
  return *(undefined4 *)(param_1 + 0xd8);
}

```

## R156 FUNCTION CWorldTimer::UnpauseWorldTimer  steam=0x0051b0a0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=54 ret_N=0
```c
// FN 0051b0a0 FUN_0051b0a0 size=54 ret=0 callers=00784740,00798820,00538ed0,00781be0 callees=0051ac30

void __fastcall FUN_0051b0a0(int param_1)

{
  if (*(int *)(param_1 + 0x24) == 1) {
    FUN_0051ac30(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c),0);
    *(undefined4 *)(param_1 + 0x24) = 0;
  }
  return;
}

```

## R157 OFFSET CRes.data = 0x10
Claim: Demand 0x711c20 returns [res+0x10] after loading, ctor zeroes param_1[4]; CResGFF ctor 0x716fd0 tests param_1[4]==0 after Demand 0x7111e0
Decompile 0x007111e0:
```c
// FN 007111e0 FUN_007111e0/*CRes::Demand*/ size=26 ret=0 callers=00940840,0070ec50,0093f8b0,00941040,005236b0,0093ffb0,00944d80,009422e0,00716fd0,0071c3d0,00638bd0,005e9ee0,0093f340,008bf280,0071f750,0083db30,00630810,009414d0,00940c40,0055db00,0040ee40,0093d110,005582f0,0055d080,00765df0,00630f20,0093d5b0,00701fd0 callees=00711c20

void __fastcall FUN_007111e0/*CRes::Demand*/(undefined4 param_1)

{
  FUN_00711c20(param_1);
  return;
}

```
Decompile 0x00711c20:
```c
// FN 00711c20 FUN_00711c20 size=548 ret=4 callers=007111e0 callees=00521060,0064dde0,000000b6,00713fb0,007a1720,00713e80,00713bf0,00739a10,0058c370,00715a60,007133a0

undefined4 __thiscall FUN_00711c20(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_18;
  int local_14;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  if (param_2 == 0) {
    uVar1 = 0;
  }
  else if (*(int *)(param_2 + 8) == -1) {
    uVar1 = 0;
  }
  else if (((*(uint *)(param_2 + 0xc) & 4) == 0) || (*(int *)(param_2 + 0x10) == 0)) {
    if ((*(uint *)(param_2 + 0xc) & 0x10) == 0) {
      switch(*(uint *)(param_2 + 8) >> 0x1e) {
      case 0:
        local_8 = FUN_00713fb0(param_2,0);
        break;
      case 1:
        local_8 = FUN_00713e80/*CExoResMan::ServiceFromImage*/(param_2,0);
        break;
      case 2:
        local_8 = FUN_00713bf0(param_2,0);
        break;
      case 3:
        local_8 = FUN_007133a0(param_2,0);
      }
      if (local_8 == 0) {
        uVar1 = 0;
      }
      else {
        local_18 = FUN_007a1720();
        local_14 = FUN_00521060();
        while (local_18 != 0) {
          if (local_14 == param_2) {
            FUN_0064dde0(local_18);
            break;
          }
          local_14 = FUN_0058c370(&local_18);
        }
        *(short *)(param_2 + 4) = *(short *)(param_2 + 4) + 1;
        uVar1 = *(undefined4 *)(param_2 + 0x10);
      }
... (37 more lines)
```
Decompile 0x00716fd0:
```c
// FN 00716fd0 FUN_00716fd0/*CResGFF::Constructor_2*/ size=465 ret=12 callers=00525c60,00620120,00629d80,005460f0,0061beb0,006564c0,00638bd0,00629220,00631260,00601610,008529e0,0052ccd0,00545760,006d0db0,005f9220,0062add0,0040ee40,005582f0,00637ce0,005fbf60,006815e0,00572a70,00625190,005ef3d0,00855f30 callees=007111e0,00713010,0091d0f0,0091c630,007110d0,00712b60

undefined4 * __thiscall
FUN_00716fd0/*CResGFF::Constructor_2*/(undefined4 *param_1,undefined2 param_2,char *param_3,undefined4 param_4)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ff08;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_007110d0/*CRes::CRes*/(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
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
  _strncpy((char *)((int)param_1 + 0x8d),param_3,4);
  FUN_00712b60(param_4,param_2,param_1);
  FUN_007111e0/*CRes::Demand*/();
  if (param_1[4] == 0) {
    FUN_00713010/*CExoResMan::ReleaseResObject*/(param_1);
  }
  else {
... (6 more lines)
```

## R158 FUNCTION Gob::IsInCutscene  steam=0x00459e20
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): unsigned char (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=17 ret_N=0
```c
// FN 00459e20 FUN_00459e20 size=17 ret=0 callers=004dfed0,00468b50,004e1d40,00467d70,004682c0 callees=

undefined1 __fastcall FUN_00459e20(int param_1)

{
  return *(undefined1 *)(param_1 + 0x14);
}

```

## R159 FUNCTION CSWGuiControl::SetEnabled  steam=0x004193f0
GOG build: param_bytes=4 ghidra_sig='CSWGuiControl * this, int param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, UINT enabled) param_bytes=4
Steam function at this address: size=39 ret_N=4
```c
// FN 004193f0 FUN_004193f0 size=39 ret=4 callers=0041a210,00419a10,00421300 callees=

void __thiscall FUN_004193f0(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffff7 | (param_2 & 1) << 3;
  return;
}

```

## R160 FUNCTION Gob::DisableForceAnimations  steam=0x00459d80
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=18 ret_N=0
```c
// FN 00459d80 FUN_00459d80 size=18 ret=0 callers= callees=

void __fastcall FUN_00459d80(int param_1)

{
  *(undefined1 *)(param_1 + 0x10) = 0;
  return;
}

```

## R161 FUNCTION CSWCObject::AnimationWalking  steam=0x007ef3e0
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=129 ret_N=4
```c
// FN 007ef3e0 FUN_007ef3e0 size=129 ret=4 callers=00769780 callees=0071b8b0

bool __thiscall FUN_007ef3e0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe30,&local_8);
  return iVar1 != 0 && local_8 != 0;
}

```
