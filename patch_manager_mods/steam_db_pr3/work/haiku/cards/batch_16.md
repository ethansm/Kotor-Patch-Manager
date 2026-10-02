## R432 FUNCTION CGuiInGame::SetLevelUpMode  steam=0x007bfa20
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int mode) param_bytes=4
Steam function at this address: size=41 ret_N=4
```c
// FN 007bfa20 FUN_007bfa20 size=41 ret=4 callers=0082c980,00813200 callees=

void __thiscall FUN_007bfa20(int param_1,int param_2)

{
  if (*(int *)(param_1 + 300) != param_2) {
    *(int *)(param_1 + 300) = param_2;
  }
  return;
}

```

## R433 FUNCTION CClientExoApp::SetEventDescriptions  steam=0x0073fb00
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 0073fb00 FUN_0073fb00 size=22 ret=0 callers=00409750 callees=00785df0

void __fastcall FUN_0073fb00(undefined4 param_1)

{
  FUN_00785df0(param_1);
  return;
}

```

## R434 FUNCTION CSWSDoor::GetDialogResref  steam=0x00616f00
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, void* outResRef) param_bytes=4
Steam function at this address: size=49 ret_N=4
```c
// FN 00616f00 FUN_00616f00 size=49 ret=4 callers= callees=

undefined4 * __thiscall FUN_00616f00(int param_1,undefined4 *param_2)

{
  *param_2 = *(undefined4 *)(param_1 + 0x2e2);
  param_2[1] = *(undefined4 *)(param_1 + 0x2e6);
  param_2[2] = *(undefined4 *)(param_1 + 0x2ea);
  param_2[3] = *(undefined4 *)(param_1 + 0x2ee);
  return param_2;
}

```

## R435 FUNCTION CSWCObject::SetLoopingAnimation  steam=0x007ed950
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, WORD animation) param_bytes=4
Steam function at this address: size=42 ret_N=4
```c
// FN 007ed950 FUN_007ed950 size=42 ret=4 callers=0087b9e0,00804f10,0076d200,007ed9a0,0080d050,008079b0 callees=007ef500

void __thiscall FUN_007ed950(int param_1,undefined2 param_2)

{
  int iVar1;
  
  iVar1 = FUN_007ef500/*CSWCObject::AnimationLooping*/(param_2);
  if (iVar1 == 1) {
    *(undefined2 *)(param_1 + 0x58) = param_2;
  }
  return;
}

```

## R436 FUNCTION CSWCObject::ShowShadowBlob  steam=0x00764f80
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=44 ret_N=0
```c
// FN 00764f80 FUN_00764f80 size=44 ret=0 callers= callees=

void __fastcall FUN_00764f80(int param_1)

{
  if (*(int *)(param_1 + 0x68) != 0) {
    (**(code **)(**(int **)(param_1 + 0x68) + 0xac))(0);
  }
  return;
}

```

## R437 FUNCTION CResGFF::GetFieldType  steam=0x00718710
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): GFFFieldTypes (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label, DWORD fieldIndex) param_bytes=12
Steam function at this address: size=75 ret_N=12
```c
// FN 00718710 FUN_00718710 size=75 ret=12 callers=00523870 callees=00717770,007178e0

undefined4 FUN_00718710(undefined4 param_1,int param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  
  if (param_2 != 0) {
    param_3 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_2);
  }
  puVar1 = (undefined4 *)FUN_00717770(param_1,param_3);
  if (puVar1 == (undefined4 *)0x0) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = *puVar1;
  }
  return uVar2;
}

```

## R438 FUNCTION Scene::GetIgnoreVisibilityGraph  steam=0x0046fb60
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=21 ret_N=0
```c
// FN 0046fb60 FUN_0046fb60 size=21 ret=0 callers= callees=

int __fastcall FUN_0046fb60(int param_1)

{
  return (int)*(short *)(param_1 + 0xe4);
}

```

## R439 FUNCTION CGuiInGame::HideGalaxyMapGui  steam=0x007ca920
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=239 ret_N=0
```c
// FN 007ca920 FUN_007ca920 size=239 ret=0 callers=00898010 callees=00740330,00410670,004122a0,00740350,0073fee0,007c9c40,0070bc60

void __fastcall FUN_007ca920(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = FUN_00740330();
    if (iVar1 != 0) {
      FUN_00740350(0,0,0);
    }
    FUN_007c9c40(4,1);
    FUN_0073fee0/*CClientExoApp::SetInputClass*/(0,1);
    FUN_00410670(*(undefined4 *)(param_1 + 0x80));
    if (DAT_00a10760 == 0) {
      if (*(undefined4 **)(param_1 + 0x80) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x80))(1);
      }
      *(undefined4 *)(param_1 + 0x80) = 0;
    }
    FUN_004122a0/*CSWGuiManager::PlayGuiSound*/(5);
    FUN_0070bc60(DAT_0099c2a8,0);
  }
  return;
}

```

## R440 FUNCTION CResGFF::ReadFieldQuaternion  steam=0x007193d0
GOG build: param_bytes=20 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5'
Wrapper typedef (what calling code expects): Quaternion* (__thiscall)(void* thisPtr, Quaternion* out, CResStruct* structPtr, char* label, int* success, Quaternion* defaultValue) param_bytes=20
Steam function at this address: size=326 ret_N=20
```c
// FN 007193d0 FUN_007193d0 size=326 ret=20 callers=00528710 callees=00717770,00717820,007178e0

undefined4 *
FUN_007193d0(undefined4 *param_1,int param_2,int param_3,undefined4 *param_4,undefined4 *param_5)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  uint local_20 [2];
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  int *local_8;
  
  local_c = 0;
  local_10 = 0;
  local_14 = 0;
  local_18 = 0x3f800000;
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    *param_1 = *param_5;
    param_1[1] = param_5[1];
    param_1[2] = param_5[2];
    param_1[3] = param_5[3];
  }
  else {
    uVar3 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_2,param_3);
    local_8 = (int *)FUN_00717770(param_2,uVar3);
    if ((local_8 == (int *)0x0) || (*local_8 != 0x10)) {
      *param_4 = 0;
      *param_1 = *param_5;
      param_1[1] = param_5[1];
      param_1[2] = param_5[2];
      param_1[3] = param_5[3];
    }
    else {
      puVar4 = (undefined4 *)FUN_00717820/*CResGFF::GetDataField*/(local_8,local_20);
      if ((puVar4 == (undefined4 *)0x0) || (local_20[0] < 0x10)) {
        *param_4 = 0;
        *param_1 = *param_5;
        param_1[1] = param_5[1];
        param_1[2] = param_5[2];
        param_1[3] = param_5[3];
      }
      else {
        *param_4 = 1;
        uVar3 = puVar4[1];
        uVar1 = puVar4[2];
        uVar2 = puVar4[3];
        *param_1 = *puVar4;
        param_1[1] = uVar3;
        param_1[2] = uVar1;
        param_1[3] = uVar2;
      }
    }
  }
  return param_1;
}

```

## R441 FUNCTION Scene::SetSceneFocus  steam=0x00464ca0
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector focus) param_bytes=4
Steam function at this address: size=38 ret_N=12
```c
// FN 00464ca0 FUN_00464ca0 size=38 ret=12 callers= callees=

void __thiscall FUN_00464ca0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  *(undefined4 *)(param_1 + 0xb4) = param_2;
  *(undefined4 *)(param_1 + 0xb8) = param_3;
  *(undefined4 *)(param_1 + 0xbc) = param_4;
  return;
}

```

## R442 FUNCTION CResGFF::AddList  steam=0x00719740
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResList* list, CResStruct* structPtr, char* labelText) param_bytes=12
Steam function at this address: size=183 ret_N=12
```c
// FN 00719740 FUN_00719740 size=183 ret=12 callers=0052ab70,005f7b80,00540410,0052a600,005e2700,005f18c0,0051ee20,00574500,0052ad90,0052a840,0062b5f0,00655010,00603390,0052afc0,0052aeb0,0055cc70,0052a730,0052aa60,00629eb0,00540900,005fb1a0,00603740,0055ddf0,005e5b40,005e9510,005e1e60,006fed80,0055bd10,0052ac80,005f8710,0052a950,00592310,0055d080,0052b380,005e72d0,00621ba0,0076aed0,00540860,006b4ea0,005302e0,0058d2f0,006272c0,0069cdb0 callees=00717aa0,0091d0f0,00718030,00718580

undefined4 __thiscall
FUN_00719740(int param_1,undefined4 *param_2,undefined4 *param_3,char *param_4)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  
  if (((param_2 == (undefined4 *)0x0) || (param_3 == (undefined4 *)0x0)) || (param_4 == (char *)0x0)
     ) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_3,param_4,0xf);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      puVar3 = (undefined4 *)FUN_00718580/*CResGFF::AddDataLayoutList*/(*(undefined4 *)(iVar2 + 8),0,4);
      if (puVar3 == (undefined4 *)0x0) {
        uVar1 = 0;
      }
      else {
        *(int *)(iVar2 + 8) = (int)puVar3 - *(int *)(param_1 + 0x70);
        *puVar3 = 0;
        *param_2 = *param_3;
        _strncpy((char *)(param_2 + 1),param_4,0x10);
        uVar1 = 1;
      }
    }
  }
  return uVar1;
}

```

## R443 FUNCTION CClientExoApp::GetTargetChanging  steam=0x00740e80
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=57 ret_N=0
```c
// FN 00740e80 FUN_00740e80 size=57 ret=0 callers=0077df90,0077e0c0,0074fb20 callees=

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool __fastcall FUN_00740e80(int param_1)

{
  return (float)_DAT_00986b38 < *(float *)(*(int *)(param_1 + 4) + 0x378);
}

```

## R444 FUNCTION CSWGuiPanel::StartLoadFromLayout  steam=0x0040ee40
GOG build: param_bytes=12 ghidra_sig='CSWGuiPanel * this, CResRef * param_1, int param_2, int param_3'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* guiResref) param_bytes=4
Steam function at this address: size=1347 ret_N=12
```c
// FN 0040ee40 FUN_0040ee40 size=1347 ret=12 callers=0075b370,008850b0,00893950,008a1170,00892090,00918b10,0088aac0,009074e0,007d6170,00747210,00880150,008c19f0,008b4270,0084c3a0,008d1020,0089c5f0,007d8540,0075ae40,008bba80,007d23e0,00760210,007d9800,008c80f0,008c9e10,00850770,007d4620,007dc410,008fb7a0,0075fda0,00916ef0,007dacb0,00886fc0,008a92d0,009153d0,007bc600,00757c40,008e1810,008586d0,008db630,008f92b0,00910490,008b91f0,008bfb60,008f4af0,008fc8e0,008b1ea0,008973d0,00916640,008bd910,008b82e0,00902f60,00904a80,008fff20,008bdc90,0089cf30,008df930,008c0cb0,007fae60,00754ed0,008c6650,008bc620,00914000,008a25c0,008d6b90,00881970,007d31d0,0090c6a0,0075cec0,008a6170,008bee10,00887da0,00909e00 callees=00919723,007189d0,0040e2c0,00733570,00719520,00718e40,00718d00,0073fea0,007111e0,00718960,00718760,00710840,0091c860,00733780,00716fd0,00414840

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0040ee40(int *param_1,undefined4 param_2,int param_3,int param_4)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  float10 fVar4;
  float local_d0;
  float local_cc;
  float local_c8;
  int local_c4;
  undefined1 local_90 [16];
  undefined1 local_80 [16];
  undefined4 local_70;
  undefined4 local_6c;
  int local_68;
  int local_64;
  undefined1 local_60 [8];
  undefined1 local_58 [12];
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  int local_40;
  int local_3c;
  undefined1 local_38 [4];
  undefined1 local_34 [4];
  undefined1 local_30 [4];
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  float local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00948313;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (((uint)param_1[0x12] >> 1 & 1) == 0) {
    local_40 = FUN_00919723/*operator new*/(0xa0,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (local_40 == 0) {
      local_c4 = 0;
    }
    else {
      local_c4 = FUN_00716fd0/*CResGFF::Constructor_2*/(0x7ff,&DAT_00987478,param_2);
    }
    local_3c = local_c4;
    local_8 = 0xffffffff;
    param_1[0xc] = local_c4;
    local_1c = param_1[0xc];
    iVar1 = FUN_007111e0/*CRes::Demand*/();
    if (iVar1 != 0) {
      FUN_007189d0/*CResGFF::GetTopLevelStruct*/(local_38);
      FUN_0040e2c0(local_1c,local_38);
      local_4c = _DAT_00987474;
      local_48 = _DAT_00987474;
      local_44 = _DAT_00987474;
      piVar2 = (int *)FUN_00719520/*CResGFF::ReadFieldVector*/(local_58,local_38,"COLOR",local_30,&local_4c);
      param_1[0x15] = *piVar2;
      param_1[0x16] = piVar2[1];
      param_1[0x17] = piVar2[2];
... (70 more lines)
```

## R445 FUNCTION CGuiInGame::GetCanHeadFollowInDialog  steam=0x007cf8f0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD objectId) param_bytes=4
Steam function at this address: size=112 ret_N=4
```c
// FN 007cf8f0 FUN_007cf8f0 size=112 ret=4 callers=007c41f0 callees=00521060,007a1720,0058c370

undefined4 __thiscall FUN_007cf8f0(undefined4 param_1,int param_2)

{
  undefined4 uVar1;
  int local_c;
  int *local_8;
  
  if (param_2 == 0x7f000000) {
    uVar1 = 0;
  }
  else {
    local_c = FUN_007a1720(param_1);
    local_8 = (int *)FUN_00521060();
    while (local_c != 0) {
      if (*local_8 == param_2) {
        return 0;
      }
      local_8 = (int *)FUN_0058c370(&local_c);
    }
    uVar1 = 1;
  }
  return uVar1;
}

```

## R446 FUNCTION Gob::AddAnimationFlag  steam=0x004e3110
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, unsigned long flag) param_bytes=4
Steam function at this address: size=67 ret_N=4
```c
// FN 004e3110 FUN_004e3110 size=67 ret=4 callers= callees=

void __thiscall FUN_004e3110(int param_1,uint param_2)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0xe0) + -1;
  if (-1 < iVar1) {
    iVar1 = *(int *)(*(int *)(param_1 + 0xdc) + iVar1 * 4);
    *(uint *)(iVar1 + 8) = *(uint *)(iVar1 + 8) | param_2;
  }
  return;
}

```

## R447 FUNCTION CGuiInGame::ShowGalaxyMapGui  steam=0x007ca730
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int selectedPlanetId) param_bytes=4
Steam function at this address: size=494 ret_N=4
```c
// FN 007ca730 FUN_007ca730 size=494 ret=4 callers=00697460 callees=00733570,00740330,005fed90,0070bc60,00733780,008973d0,0073f750,0051c8d0,008bdf50,004122a0,00410530,00740350,0073fee0,007401f0,007c9c40,00919723,00710840

void __thiscall FUN_007ca730(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 local_58;
  undefined1 local_28 [8];
  undefined1 local_20 [16];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969ad3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x30) == 0)) {
    iVar1 = FUN_0073f750(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (*(int *)(iVar1 + 0xcc) == 0) {
      iVar1 = FUN_00740330();
      if (iVar1 == 0) {
        FUN_00740350(1,0,0);
      }
      FUN_007401f0(0,0);
      FUN_0070bc60(DAT_0099c2b8,0);
      FUN_007c9c40(3,1);
      FUN_00710840(&DAT_00986925);
      FUN_00733570/*CExoString::CExoString_char*/(&DAT_00986925);
      local_8 = 0;
      FUN_008bdf50(0x7f000000,local_28,local_20,0);
      local_8 = 0xffffffff;
      FUN_00733780/*CExoString::~CExoString*/();
      FUN_0073fee0/*CClientExoApp::SetInputClass*/(2,1);
      if (*(int *)(param_1 + 0x80) == 0) {
        iVar1 = FUN_00919723/*operator new*/(0x28d8);
        local_8 = 1;
        if (iVar1 == 0) {
          local_58 = 0;
        }
        else {
          local_58 = FUN_008973d0(*(undefined4 *)(param_1 + 0x38));
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x80) = local_58;
      }
      FUN_0051c8d0/*CServerExoApp::GetPartyTable (inline at *(this+4)+0x1F0B4)*/(param_2);
      FUN_005fed90(param_2);
      FUN_00410530/*CSWGuiManager::AddPanel*/(*(undefined4 *)(param_1 + 0x80),2,1);
      FUN_004122a0/*CSWGuiManager::PlayGuiSound*/(4);
    }
  }
  ExceptionList = local_10;
  return;
}

```

## R448 FUNCTION CSWCObject::AnimationPlayOutOfPlace  steam=0x007ef730
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=68 ret_N=4
```c
// FN 007ef730 FUN_007ef730 size=68 ret=4 callers=00841440,00777e30,007ed9a0 callees=

undefined4 FUN_007ef730(ushort param_1)

{
  undefined4 uVar1;
  
  if (((param_1 < 0x4b0) || (0x52f < param_1)) && ((param_1 < 0x640 || (0x6bf < param_1)))) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

```

## R449 FUNCTION CSWGuiPanel::Constructor  steam=0x0040e5a0
GOG build: param_bytes=4 ghidra_sig='CSWGuiPanel * this, CSWGuiManager * param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, void* manager) param_bytes=4
Steam function at this address: size=423 ret_N=4
```c
// FN 0040e5a0 FUN_0040e5a0 size=423 ret=4 callers=0075b370,008850b0,00880740,00893950,008a1170,00892090,00918b10,0088aac0,009074e0,007d6170,00747210,00880150,008c19f0,008b4270,0074a500,0084c3a0,008d1020,008b9ce0,0089c5f0,007d8540,0075ae40,007d23e0,00760210,007d9800,008c80f0,008c9e10,00850770,007d4620,007dc410,008fb7a0,0075fda0,00916ef0,007dacb0,00886fc0,008a92d0,008ba980,009153d0,007bc600,00757c40,008e1810,008586d0,008db630,008f92b0,00910490,008b91f0,008bfb60,008f4af0,008fc8e0,008b1ea0,008973d0,00916640,008bb5d0,008b82e0,00902f60,00904a80,008fff20,008bdc90,0089cf30,008df930,008c0cb0,007fae60,00754ed0,008c6650,0087fd40,00914000,008a25c0,008d6b90,00411b20,007d31d0,0090c6a0,0075cec0,008a6170,008bee10,00887da0,00909e00 callees=00919723,00414a50,0084a640

undefined4 * __thiscall FUN_0040e5a0(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_009482ae;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiObject::vftable;
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[2] = 0;
  param_1[1] = 0;
  local_8 = 0;
  *param_1 = CSWGuiPanel::vftable;
  FUN_0084a640(0);
  local_8._0_1_ = 1;
  param_1[0x15] = 0;
  param_1[0x16] = 0;
  param_1[0x17] = 0;
  iVar2 = FUN_00919723/*operator new*/(0x78,uVar1);
  local_8 = CONCAT31(local_8._1_3_,2);
  if (iVar2 == 0) {
    local_28 = 0;
  }
  else {
    local_28 = FUN_00414a50/*CSWGuiBorder::Constructor*/();
  }
  param_1[0x18] = local_28;
  param_1[0x12] = param_1[0x12] & 0xfffffffd;
  param_1[8] = 0;
  param_1[7] = param_2;
  param_1[0xc] = 0;
  param_1[0x12] = param_1[0x12] | 4;
  param_1[0x12] = param_1[0x12] | 0x40;
  param_1[0x12] = param_1[0x12] & 0xffffffcf;
  param_1[0x12] = param_1[0x12] & 0xfffffcff;
  param_1[0x12] = param_1[0x12] & 0xffffff7f;
  param_1[0x14] = 0x3f800000;
  *(undefined1 *)(param_1 + 0x13) = 0;
  param_1[0x12] = param_1[0x12] | 1;
  *(undefined1 *)(param_1 + 0x19) = 0xff;
  param_1[6] = 1;
  ExceptionList = local_10;
  return param_1;
}

```

## R450 FUNCTION CClientExoApp::ExitProgram  steam=0x0073f290
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__stdcall)() param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 0073f290 FUN_0073f290 size=22 ret=0 callers=00883cf0,00883d20 callees=00785360

void __fastcall FUN_0073f290(undefined4 param_1)

{
  FUN_00785360(param_1);
  return;
}

```

## R451 FUNCTION CSWCObject::AnimationLooping  steam=0x007ef500
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=197 ret_N=4
```c
// FN 007ef500 FUN_007ef500 size=197 ret=4 callers=00781be0,0087b9e0,00841440,0087f410,007ed950,0087b3f0,00804f10,00770310,00777e30,007ed9a0,007637e0,008079b0 callees=0071b8b0

bool __thiscall FUN_007ef500(int param_1,ushort param_2)

{
  int iVar1;
  bool bVar2;
  int local_8;
  
  if ((*(char *)(param_1 + 8) == '\x05') &&
     (((0x577 < param_2 && (param_2 < 0x5f8)) || ((0x63f < param_2 && (param_2 < 0x6c0)))))) {
    bVar2 = true;
  }
  else {
    local_8 = 0;
    if (*(int *)(param_1 + 0x68) != 0) {
      param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
    }
    iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe24,&local_8);
    if (iVar1 == 0) {
      bVar2 = false;
    }
    else {
      bVar2 = local_8 != 0;
    }
  }
  return bVar2;
}

```

## R452 FUNCTION Gob::GetName  steam=0x004596f0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): CResRef* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=17 ret_N=0
```c
// FN 004596f0 FUN_004596f0 size=17 ret=0 callers= callees=

int __fastcall FUN_004596f0(int param_1)

{
  return param_1 + 100;
}

```

## R453 FUNCTION CSWCObject::HideShadowBlob  steam=0x00764fb0
GOG build: param_bytes=0 ghidra_sig='CSWCObject * this'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=42 ret_N=0
```c
// FN 00764fb0 CPaneContainerManager::NotifyPaneDivider size=42 ret=0 callers= callees=

/* Library Function - Single Match
    public: void __thiscall CPaneContainerManager::NotifyPaneDivider(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CPaneContainerManager::NotifyPaneDivider(CPaneContainerManager *this)

{
  if (*(int *)(this + 0x68) != 0) {
    (**(code **)(**(int **)(this + 0x68) + 0xb0))();
  }
  return;
}

```

## R454 FUNCTION Gob::SetOrientation  steam=0x004614e0
GOG build: param_bytes=20 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5'
Wrapper typedef (what calling code expects): Quaternion* (__thiscall)(void* thisPtr, Quaternion* outOrientation, Quaternion orientation) param_bytes=8
Steam function at this address: size=80 ret_N=20
```c
// FN 004614e0 FUN_004614e0 size=80 ret=20 callers= callees=

undefined4 * __thiscall
FUN_004614e0(int param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  *(undefined4 *)(param_1 + 0xb0) = param_3;
  *(undefined4 *)(param_1 + 0xb4) = param_4;
  *(undefined4 *)(param_1 + 0xb8) = param_5;
  *(undefined4 *)(param_1 + 0xbc) = param_6;
  *param_2 = *(undefined4 *)(param_1 + 0xb0);
  param_2[1] = *(undefined4 *)(param_1 + 0xb4);
  param_2[2] = *(undefined4 *)(param_1 + 0xb8);
  param_2[3] = *(undefined4 *)(param_1 + 0xbc);
  return param_2;
}

```

## R455 FUNCTION CResGFF::ReadFieldFLOAT  steam=0x00718d00
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): float10 (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label, int* success, float defaultValue) param_bytes=16
Steam function at this address: size=128 ret_N=16
```c
// FN 00718d00 FUN_00718d00 size=128 ret=16 callers=00625360,005e2900,00836350,00628c40,0062db90,00525f90,006fef80,0053fd50,00528710,005269e0,00528080,006c67e0,005266f0,00630810,0040ee40,00561d80,005e2090,008384a0,00572a70,00527cb0,0058d5e0,00620250,00526f60,00416720,006295d0,00527330,005263c0,006c7400,00626850,00585b00,00833ff0,00836730,0052ccd0,006d0db0,005e5510,0083db30,00527580,00527890,00621900,0061a210,005582f0,006afed0,0055def0,005fbf60,005e6f80,00626a50,00520fc0,00523870,0053e100 callees=00717770,007178e0

float10 FUN_00718d00(int param_1,int param_2,undefined4 *param_3,float param_4)

{
  undefined4 uVar1;
  int *piVar2;
  float10 fVar3;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
    fVar3 = (float10)param_4;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 8)) {
      *param_3 = 0;
      fVar3 = (float10)param_4;
    }
    else {
      *param_3 = 1;
      fVar3 = (float10)(float)piVar2[2];
    }
  }
  return fVar3;
}

```

## R456 FUNCTION CSWGuiControl::GetSelectableParent  steam=0x004193a0
GOG build: param_bytes=0 ghidra_sig='CSWGuiControl * this'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=72 ret_N=0
```c
// FN 004193a0 FUN_004193a0 size=72 ret=0 callers=00419130,007ac470,0041aac0,00419240,00419190,004192a0,004134a0,00420850,00412c00 callees=

int * __fastcall FUN_004193a0(int *param_1)

{
  int iVar1;
  int *local_8;
  
  local_8 = param_1;
  do {
    if (local_8[6] == 0) {
      return (int *)0x0;
    }
    local_8 = (int *)local_8[6];
    iVar1 = (**(code **)(*local_8 + 0x80))();
  } while (iVar1 == 0);
  return local_8;
}

```

## R457 OFFSET CSWGuiManager.panels = 0x88
Claim: FindPanelIndex 0x410890 scans [this+0x88] up to count [this+0x8c]; IsOnTop 0x412300 and HandleInputEvent use the same array
Decompile 0x00410890:
```c
// FN 00410890 FUN_00410890 size=101 ret=4 callers=00410800,00410900,00410670,00410530 callees=

int __thiscall FUN_00410890(int param_1,int param_2)

{
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x8c) <= local_c) {
      return -1;
    }
    if ((*(int *)(*(int *)(param_1 + 0x88) + local_c * 4) != 0) &&
       (*(int *)(*(int *)(param_1 + 0x88) + local_c * 4) == param_2)) break;
    local_c = local_c + 1;
  }
  return local_c;
}

```
Decompile 0x00412300:
```c
// FN 00412300 FUN_00412300 size=246 ret=4 callers=0088e510 callees=

bool __thiscall FUN_00412300(int param_1,int param_2)

{
  int local_8;
  
  local_8 = *(int *)(param_1 + 0x98);
  do {
    local_8 = local_8 + -1;
    if (local_8 < 0) {
      local_8 = *(int *)(param_1 + 0x8c);
      do {
        local_8 = local_8 + -1;
        if (local_8 < 0) {
          return false;
        }
      } while ((*(uint *)(*(int *)(*(int *)(param_1 + 0x88) + local_8 * 4) + 0x48) >> 8 & 3) != 0);
      return *(int *)(*(int *)(param_1 + 0x88) + local_8 * 4) == param_2;
    }
  } while ((*(uint *)(*(int *)(*(int *)(param_1 + 0x94) + local_8 * 4) + 0x48) >> 8 & 3) != 0);
  if (*(int *)(*(int *)(param_1 + 0x94) + local_8 * 4) == param_2) {
    return true;
  }
  return false;
}

```

## R458 FUNCTION Gob::IsAnimating  steam=0x00459890
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): unsigned char (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=20 ret_N=0
```c
// FN 00459890 FUN_00459890 size=20 ret=0 callers= callees=

undefined1 __fastcall FUN_00459890(int param_1)

{
  return *(undefined1 *)(param_1 + 0x1aa);
}

```
