## R000 FUNCTION CClientExoApp::StopCreditSequence  steam=0x00741050
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 00741050 FUN_00741050 size=22 ret=0 callers=008925f0 callees=007a12c0

void __fastcall FUN_00741050(undefined4 param_1)

{
  FUN_007a12c0(param_1);
  return;
}

```

## R001 FUNCTION CSWSObject::SetPortraitId  steam=0x00540b40
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, WORD id) param_bytes=4
Steam function at this address: size=339 ret_N=4
```c
// FN 00540b40 FUN_00540b40 size=339 ret=4 callers= callees=00733df0,00734010,00733570,0071b1d0,007337c0,00710840,00733780,00733540

void __thiscall FUN_00540b40(int *param_1,ushort param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 local_50 [16];
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [16];
  undefined1 local_20 [16];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f7f0;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570/*CExoString::CExoString_char*/(&DAT_00994684);
  local_8 = 0;
  if (param_2 < 0xfffe) {
    FUN_00733540(uVar1);
    local_8._0_1_ = 1;
    FUN_00733570/*CExoString::CExoString_char*/("BaseResRef");
    local_8._0_1_ = 2;
    FUN_0071b1d0(param_2,local_38,local_20);
    local_8._0_1_ = 1;
    FUN_00733780/*CExoString::~CExoString*/();
    uVar2 = FUN_00733df0/*CExoString::operator+*/(local_40,local_20);
    local_8._0_1_ = 3;
    FUN_007337c0/*CExoString::operator=*/(uVar2);
    local_8._0_1_ = 1;
    FUN_00733780/*CExoString::~CExoString*/();
    uVar2 = FUN_00734010();
    FUN_00710840(uVar2);
    (**(code **)(*param_1 + 0xcc))(local_50);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780/*CExoString::~CExoString*/();
  }
  else {
    FUN_00710840(&DAT_00986925);
    (**(code **)(*param_1 + 0xcc))(local_30);
  }
  *(ushort *)(param_1 + 0xc) = param_2;
  local_8 = 0xffffffff;
  FUN_00733780/*CExoString::~CExoString*/();
  ExceptionList = local_10;
  return;
}

```

## R002 FUNCTION CGuiInGame::HideLoadModuleDebugMenu  steam=0x007cb540
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=149 ret_N=0
```c
// FN 007cb540 FUN_007cb540 size=149 ret=0 callers=008bff20,008c0c80 callees=00410670,0073fee0,00410800,007400a0,004109c0

void __fastcall FUN_007cb540(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x74));
    if (iVar1 != 0) {
      FUN_004109c0/*CSWGuiManager::PopModalPanel*/();
      FUN_00410670(*(undefined4 *)(param_1 + 0x74));
      iVar1 = FUN_007400a0();
      if (iVar1 == 0) {
        FUN_0073fee0/*CClientExoApp::SetInputClass*/(0,1);
      }
      else {
        FUN_0073fee0/*CClientExoApp::SetInputClass*/(1,1);
      }
    }
  }
  return;
}

```

## R003 FUNCTION CSWCPlaceable::SetAppearance  steam=0x00843960
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, BYTE appearance) param_bytes=4
Steam function at this address: size=327 ret_N=4
```c
// FN 00843960 FUN_00843960 size=327 ret=4 callers=007a9f40,0080d050 callees=00733570,0071bcf0,00733780

void __thiscall FUN_00843960(int *param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  undefined1 local_30 [11];
  char local_25;
  undefined1 local_24 [8];
  undefined1 local_1c [4];
  int local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00970a30;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  param_1[0x43] = param_2;
  local_18 = 0;
  FUN_00733570/*CExoString::CExoString_char*/("HitCheck");
  local_8 = 0;
  FUN_0071bcf0(param_2,local_24,&local_18);
  local_8 = 0xffffffff;
  FUN_00733780/*CExoString::~CExoString*/(uVar1);
  local_14 = (int *)(**(code **)(*param_1 + 0x98))(0xff,1);
  if (local_14 != (int *)0x0) {
    (**(code **)(*local_14 + 0x90))(local_18 == 0);
  }
  if (param_1[0x1a] != 0) {
    FUN_00733570/*CExoString::CExoString_char*/("NonCull");
    local_8 = 1;
    iVar2 = FUN_0071bcf0(param_2,local_30,local_1c);
    local_25 = '\x01' - (iVar2 != 0);
    local_8 = 0xffffffff;
    FUN_00733780/*CExoString::~CExoString*/();
  }
  ExceptionList = local_10;
  return;
}

```

## R004 FUNCTION CGuiInGame::HideSubItemCreateMenu  steam=0x007cab60
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=164 ret_N=0
```c
// FN 007cab60 FUN_007cab60 size=164 ret=0 callers=008bfaa0,008bfa40,008bf0e0 callees=00410670,008bf9f0,0073fee0,00410800,007400a0,004109c0

void __fastcall FUN_007cab60(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x5c));
    if (iVar1 != 0) {
      FUN_004109c0/*CSWGuiManager::PopModalPanel*/();
      FUN_00410670(*(undefined4 *)(param_1 + 0x5c));
      iVar1 = FUN_007400a0();
      if (iVar1 == 0) {
        FUN_0073fee0/*CClientExoApp::SetInputClass*/(0,1);
      }
      else {
        FUN_0073fee0/*CClientExoApp::SetInputClass*/(1,1);
      }
      FUN_008bf9f0();
    }
  }
  return;
}

```

## R005 FUNCTION CResGFF::ReadFieldCExoString  steam=0x00718f90
GOG build: param_bytes=20 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5'
Wrapper typedef (what calling code expects): CExoString* (__thiscall)(void* thisPtr, CExoString* out, CResStruct* structPtr, char* label, int* success, CExoString* defaultValue) param_bytes=20
Steam function at this address: size=322 ret_N=20
```c
// FN 00718f90 FUN_00718f90 size=322 ret=20 callers=00625360,006c75b0,005e2900,006295d0,00628c40,00853550,0062db90,00703530,006fef80,00664800,00418df0,0053fd50,006c7400,005753f0,005269e0,0069d1e0,00585b00,006d0db0,005e5510,006c67e0,0062af70,00619e60,005409f0,006c53f0,00655b00,0061a210,005e2090,005582f0,0055d080,006afed0,00601740,005f7cc0,005fbf60,005e6f80,006815e0,00417480,0058d5e0,00620250,00523870,005ef570,00655690,00855f30 callees=007335f0,00717770,00717820,007178e0,00733680

undefined4
FUN_00718f90(undefined4 param_1,int param_2,int param_3,undefined4 *param_4,undefined4 param_5)

{
  undefined4 uVar1;
  uint local_14;
  int *local_10;
  int *local_c;
  int local_8;
  
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    FUN_007335f0/*CExoString::CExoString_copy*/(param_5);
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_2,param_3);
    local_c = (int *)FUN_00717770(param_2,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 10)) {
      *param_4 = 0;
      FUN_007335f0/*CExoString::CExoString_copy*/(param_5);
    }
    else {
      local_10 = (int *)FUN_00717820/*CResGFF::GetDataField*/(local_c,&local_14);
      if ((local_10 == (int *)0x0) || (local_14 < 4)) {
        *param_4 = 0;
        FUN_007335f0/*CExoString::CExoString_copy*/(param_5);
      }
      else {
        local_8 = *local_10;
        if (local_14 < local_8 + 4U) {
          *param_4 = 0;
          FUN_007335f0/*CExoString::CExoString_copy*/(param_5);
        }
        else {
          *param_4 = 1;
          FUN_00733680(local_10 + 1,local_8);
        }
      }
    }
  }
  return param_1;
}

```

## R006 FUNCTION CResGFF::AddDataLayoutList  steam=0x00718580
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, int existingBlockOffset, DWORD sizeOfExistingBlock, DWORD dataLength) param_bytes=12
Steam function at this address: size=263 ret_N=12
```c
// FN 00718580 FUN_00718580 size=263 ret=12 callers=00719740,00719800 callees=0091dba0,009196fd,00919723

int __thiscall FUN_00718580(int param_1,int param_2,size_t param_3,int param_4)

{
  void *_Dst;
  undefined4 local_c;
  
  if ((*(int *)(param_1 + 0x40) == 0) || (*(int *)(param_1 + 0x70) == 0)) {
    local_c = 0;
  }
  else {
    local_c = *(int *)(*(int *)(param_1 + 0x40) + 0x34);
    if (local_c == param_2 + param_3) {
      local_c = param_2;
    }
    if (*(uint *)(param_1 + 0x74) <= (uint)(local_c + param_4)) {
      *(int *)(param_1 + 0x74) = local_c + param_4 + *(int *)(param_1 + 0x3c);
      _Dst = (void *)FUN_00919723/*operator new*/(*(undefined4 *)(param_1 + 0x74));
      _memcpy(_Dst,*(void **)(param_1 + 0x70),*(size_t *)(*(int *)(param_1 + 0x40) + 0x34));
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x70));
      *(void **)(param_1 + 0x70) = _Dst;
    }
    if (local_c != param_2) {
      _memcpy((void *)(*(int *)(param_1 + 0x70) + local_c),
              (void *)(*(int *)(param_1 + 0x70) + param_2),param_3);
      *(size_t *)(param_1 + 0x78) = *(int *)(param_1 + 0x78) + param_3;
    }
    *(int *)(*(int *)(param_1 + 0x40) + 0x34) = local_c + param_4;
    local_c = *(int *)(param_1 + 0x70) + local_c;
  }
  return local_c;
}

```

## R007 FUNCTION CResGFF::ReadFieldCHAR  steam=0x00718a80
GOG build: param_bytes=13 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined1 param_4'
Wrapper typedef (what calling code expects): char (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label, int* success, char defaultValue) param_bytes=16
Steam function at this address: size=122 ret_N=16
```c
// FN 00718a80 FUN_00718a80 size=122 ret=16 callers=0069d1e0,006fef80,005ef570,0061a210,006afed0 callees=00717770,007178e0

undefined1 FUN_00718a80(int param_1,int param_2,undefined4 *param_3,undefined1 param_4)

{
  undefined4 uVar1;
  int *piVar2;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 1)) {
      *param_3 = 0;
    }
    else {
      *param_3 = 1;
      param_4 = (undefined1)piVar2[2];
    }
  }
  return param_4;
}

```

## R008 FUNCTION CSWSObject::GetPortraitId  steam=0x0053f000
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): WORD (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=18 ret_N=0
```c
// FN 0053f000 FUN_0053f000 size=18 ret=0 callers= callees=

undefined2 __fastcall FUN_0053f000(int param_1)

{
  return *(undefined2 *)(param_1 + 0x30);
}

```

## R009 FUNCTION CSWSObject::SetCurrentHitPoints  steam=0x0054a610
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int currentHP) param_bytes=4
Steam function at this address: size=70 ret_N=4
```c
// FN 0054a610 FUN_0054a610 size=70 ret=4 callers=006b9d80,00569320,00599050,0056a230,005a9730,0058bce0,006995e0,006afed0,006b6850,005f0f10,006b91b0,00597750,005724d0,005a6ef0,0061a210,00541400,00631260,005a3e80,005ef570,006b9870,00597d90 callees=

void __thiscall FUN_0054a610(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xe0) = param_2;
  if ((*(int *)(param_1 + 0x22c) != 0) && (*(int *)(param_1 + 0xe0) < 1)) {
    *(undefined4 *)(param_1 + 0xe0) = 1;
  }
  return;
}

```

## R010 FUNCTION Gob::DisableAlwaysRender  steam=0x00459de0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=18 ret_N=0
```c
// FN 00459de0 FUN_00459de0 size=18 ret=0 callers= callees=

void __fastcall FUN_00459de0(int param_1)

{
  *(undefined1 *)(param_1 + 0x12) = 0;
  return;
}

```

## R011 FUNCTION CResGFF::GetFieldCount  steam=0x007186c0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr, CResStruct* structPtr) param_bytes=4
Steam function at this address: size=30 ret_N=4
```c
// FN 007186c0 FUN_007186c0 size=30 ret=4 callers=007178e0 callees=

undefined4 FUN_007186c0(int param_1)

{
  undefined4 uVar1;
  
  if (param_1 == 0) {
    uVar1 = 0xffffffff;
  }
  else {
    uVar1 = *(undefined4 *)(param_1 + 8);
  }
  return uVar1;
}

```

## R012 FUNCTION Scene::GetObjectScale  steam=0x00472610
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): double (__stdcall)(void* object) param_bytes=4
Steam function at this address: size=22 ret_N=4
```c
// FN 00472610 FUN_00472610 size=22 ret=4 callers= callees=

float10 FUN_00472610(int param_1)

{
  return (float10)*(float *)(param_1 + 0x1e0);
}

```

## R013 FUNCTION CSWCObject::FadeObject  steam=0x007f12d0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=896 ret_N=0
```c
// FN 007f12d0 FUN_007f12d0 size=896 ret=0 callers=008412a0,0087b240,007664b0,007fd370 callees=0073f3b0,0051af60,0091d220,0051ad20,0051a940,007f1170,00775860,007f03b0,007ed830

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_007f12d0(int *param_1)

{
  int *piVar1;
  float10 fVar2;
  undefined8 uVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  undefined4 *puVar7;
  undefined1 *puVar8;
  undefined4 *puVar9;
  uint *puVar10;
  uint local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined1 local_24 [4];
  int local_20;
  uint local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined1 local_10 [4];
  int local_c;
  uint local_8;
  
  if ((char)param_1[0x24] == *(char *)((int)param_1 + 0x91)) {
    if (param_1[0x25] != 0) {
      puVar9 = &local_28;
      puVar7 = &local_2c;
      FUN_0073f3b0(puVar7,puVar9);
      FUN_0051ad20/*CWorldTimer::GetWorldTime*/(puVar7,puVar9);
      puVar10 = &local_30;
      puVar8 = local_24;
      iVar5 = param_1[0x21];
      iVar4 = param_1[0x20];
      FUN_0073f3b0(local_2c,local_28,iVar4,iVar5,puVar8,puVar10);
      FUN_0051af60/*CWorldTimer::SubtractWorldTimes*/(local_2c,local_28,iVar4,iVar5,puVar8,puVar10);
      if ((uint)(param_1[0x22] + param_1[0x23]) <= local_30) {
        return 1;
      }
    }
  }
  else {
    FUN_0073f3b0();
    uVar3 = FUN_0051a940();
    local_8 = __aulldiv(uVar3,1000,0);
    if (*(char *)((int)param_1 + 0x91) == '\x01') {
      fVar2 = (float10)FUN_007f1170();
      if ((float10)(float)param_1[0x26] < fVar2) {
        param_1[0x26] =
             (int)(float)((float10)local_8 / (float10)_DAT_009963e0 + (float10)(float)param_1[0x26])
        ;
        *(undefined1 *)(param_1 + 0x24) = 2;
      }
      else {
        fVar2 = (float10)FUN_007f1170();
        param_1[0x26] = (int)(float)fVar2;
        *(undefined1 *)(param_1 + 0x24) = 1;
      }
    }
    else if (*(char *)((int)param_1 + 0x91) == '\0') {
      if (param_1[0x23] != 0) {
        puVar9 = &local_14;
        puVar7 = &local_18;
        FUN_0073f3b0(puVar7,puVar9);
        FUN_0051ad20/*CWorldTimer::GetWorldTime*/(puVar7,puVar9);
... (52 more lines)
```

## R014 FUNCTION CResGFF::GetDataField  steam=0x00717820
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResGFFField* field, DWORD* dataSize) param_bytes=8
Steam function at this address: size=81 ret_N=8
```c
// FN 00717820 FUN_00717820 size=81 ret=8 callers=00718d80,00718e40,007193d0,00718f90,007190e0,00719640,00719520 callees=

int __thiscall FUN_00717820(int param_1,int param_2,int *param_3)

{
  int iVar1;
  
  if (((param_2 == 0) || (*(int *)(param_1 + 0x5c) == 0)) ||
     (*(uint *)(*(int *)(param_1 + 0x40) + 0x24) <= *(uint *)(param_2 + 8))) {
    iVar1 = 0;
  }
  else {
    *param_3 = *(int *)(*(int *)(param_1 + 0x40) + 0x24) - *(int *)(param_2 + 8);
    iVar1 = *(int *)(param_1 + 0x5c) + *(int *)(param_2 + 8);
  }
  return iVar1;
}

```

## R015 FUNCTION CGuiInGame::HidePowersFeatsSkillsDebugMenu  steam=0x007cb540
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=149 ret_N=0
```c
// FN 007cb540 FUN_007cb540 size=149 ret=0 callers=008bff20,008c0c80 callees=00410670,0073fee0,00410800,007400a0,004109c0

void __fastcall FUN_007cb540(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x74));
    if (iVar1 != 0) {
      FUN_004109c0/*CSWGuiManager::PopModalPanel*/();
      FUN_00410670(*(undefined4 *)(param_1 + 0x74));
      iVar1 = FUN_007400a0();
      if (iVar1 == 0) {
        FUN_0073fee0/*CClientExoApp::SetInputClass*/(0,1);
      }
      else {
        FUN_0073fee0/*CClientExoApp::SetInputClass*/(1,1);
      }
    }
  }
  return;
}

```

## R016 FUNCTION Gob::GetPosition  steam=0x00459710
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector* outPosition) param_bytes=4
Steam function at this address: size=43 ret_N=4
```c
// FN 00459710 FUN_00459710 size=43 ret=4 callers= callees=

undefined4 * __thiscall FUN_00459710(int param_1,undefined4 *param_2)

{
  *param_2 = *(undefined4 *)(param_1 + 0xa4);
  param_2[1] = *(undefined4 *)(param_1 + 0xa8);
  param_2[2] = *(undefined4 *)(param_1 + 0xac);
  return param_2;
}

```

## R017 OFFSET CSWGuiBorderParams.border = 0x54
Claim: SetBorder 0x414920 stores params+0x54 = arg then re-applies the three image setters; they read *(params+0x54)+0x6c/0x70/0x74
Decompile 0x00414920:
```c
// FN 00414920 FUN_00414920/*CSWGuiBorderParams::SetBorder*/ size=82 ret=4 callers=004144f0,00414a50 callees=00414760,00414680,00414840

void __thiscall FUN_00414920/*CSWGuiBorderParams::SetBorder*/(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x54) = param_2;
  if (*(int *)(param_1 + 0x54) != 0) {
    FUN_00414680/*CSWGuiBorderParams::SetCornerImage*/(param_1 + 0x24,1);
    FUN_00414760/*CSWGuiBorderParams::SetEdgeImage*/(param_1 + 0x34,1);
    FUN_00414840/*CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force*/(param_1 + 0x44,1);
  }
  return;
}

```

## R018 FUNCTION CSWSObject::SetKeepCorpse  steam=0x0054a840
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int keepCorpse) param_bytes=4
Steam function at this address: size=153 ret_N=0
```c
// FN 0054a840 FUN_0054a840 size=153 ret=0 callers=005f6a70,00622bd0,00698190,005d75b0,00607690,00597d90,0058a500,0051d790,005d9da0,0058ae90 callees=0073fec0,007f2610,0073f4d0

undefined4 __fastcall FUN_0054a840(int param_1)

{
  undefined4 uVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  if (*(int *)(param_1 + 0x264) == 0) {
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    uVar1 = FUN_0073fec0(*(undefined4 *)(param_1 + 4));
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    piVar2 = (int *)FUN_0073f4d0(uVar1);
    if (piVar2 != (int *)0x0) {
      uVar3 = (**(code **)(*piVar2 + 0xc))(param_1,uVar4,uVar3);
      *(undefined4 *)(param_1 + 0x264) = uVar3;
      if (*(int *)(param_1 + 0x264) != 0) {
        FUN_007f2610/*CSWCObject::AttachmentFromServerObject*/(param_1);
      }
    }
  }
  return *(undefined4 *)(param_1 + 0x264);
}

```

## R019 FUNCTION CSWGuiControl::Constructor  steam=0x00418990
GOG build: param_bytes=0 ghidra_sig='CSWGuiControl * this'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=347 ret_N=0
```c
// FN 00418990 FUN_00418990 size=347 ret=0 callers=008db630,00418570,008f4af0,00899930,00895620,00880740,008973d0,008c9e10,00419740,0041b7f0,00904a80,0041bfd0,00916ef0,008fa1f0,0089cf30,0084c3a0 callees=0084a640,008a5f70,00733540

undefined4 * __fastcall FUN_00418990(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0094896e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiObject::vftable;
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[2] = 0;
  param_1[1] = 0;
  local_8 = 0;
  *param_1 = CSWGuiControl::vftable;
  FUN_0084a640(0);
  local_8._0_1_ = 1;
  FUN_00733540(uVar1);
  local_8 = CONCAT31(local_8._1_3_,2);
  FUN_008a5f70(0);
  param_1[0xe] = 0;
  param_1[0x12] = param_1[0x12] & 0xfffffffe;
  param_1[0x12] = param_1[0x12] | 2;
  param_1[0x17] = 0;
  param_1[0x15] = 0xffffffff;
  *(undefined1 *)(param_1 + 0x16) = 0;
  *(undefined1 *)((int)param_1 + 0x59) = 1;
  param_1[6] = 0;
  param_1[0x12] = param_1[0x12] & 0xffffffef;
  param_1[0x12] = param_1[0x12] | 8;
  param_1[0x12] = param_1[0x12] & 0xffffffdf;
  param_1[10] = 0;
  param_1[0xd] = 0;
  param_1[0x12] = param_1[0x12] & 0xffffffbf;
  ExceptionList = local_10;
  return param_1;
}

```

## R020 FUNCTION CGuiInGame::ShowPartySelection  steam=0x007cb920
GOG build: param_bytes=20 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* exitScript, int requireAccept, int forcedNPC1, int forcedNPC2) param_bytes=16
Steam function at this address: size=538 ret_N=20
```c
// FN 007cb920 FUN_007cb920 size=538 ret=20 callers=00692ce0 callees=0089fb40,005453c0,00410800,0070bc60,0051c8d0,0089cf30,0051c100,004122a0,00410530,0073f810,0087a730,0073fee0,00919723,007337c0,0051c8f0

void __thiscall
FUN_007cb920(int param_1,undefined4 param_2,int param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_48;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969b2b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = FUN_0051c8d0/*CServerExoApp::GetPartyTable (inline at *(this+4)+0x1F0B4)*/(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (*(int *)(iVar1 + 0x118) == -1) {
      uVar2 = FUN_0051c8f0();
      iVar1 = FUN_0051c100/*CServerExoApp::GetCreatureByGameObjectID*/(uVar2);
      if (iVar1 != 0) {
        uVar2 = FUN_0051c8f0();
        FUN_0051c100/*CServerExoApp::GetCreatureByGameObjectID*/(uVar2);
        iVar1 = FUN_005453c0();
        if (iVar1 != 0) {
          uVar2 = FUN_0051c8f0();
          FUN_0051c100/*CServerExoApp::GetCreatureByGameObjectID*/(uVar2);
          iVar1 = FUN_005453c0();
          if ((*(int *)(iVar1 + 0x2dc) == 0) || (param_3 != 0)) {
            iVar1 = CMFCVisualManagerBitmapCache::IsCached
                              (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                               *(CSize **)(param_1 + 0x1c));
            if (iVar1 == 0) {
              if (param_3 != 0) {
                if (*(int *)(param_1 + 0xf18) == 0) {
                  FUN_0073f810();
                  FUN_0087a730();
                }
                FUN_004122a0/*CSWGuiManager::PlayGuiSound*/(4);
                FUN_0070bc60(DAT_0099c2b8,0);
              }
              if (*(int *)(param_1 + 0x1c) == 0) {
                iVar1 = FUN_00919723/*operator new*/(0x50b4);
                local_8 = 0;
                if (iVar1 == 0) {
                  local_48 = 0;
                }
                else {
                  local_48 = FUN_0089cf30(*(undefined4 *)(param_1 + 0x38));
                }
                local_8 = 0xffffffff;
                *(undefined4 *)(param_1 + 0x1c) = local_48;
              }
              iVar1 = *(int *)(param_1 + 0x1c);
              FUN_007337c0/*CExoString::operator=*/(param_2);
              *(int *)(iVar1 + 0x74) = param_3;
              FUN_0089fb40(param_4,param_5);
              *(undefined4 *)(iVar1 + 0x78) = param_6;
              *(undefined4 *)(iVar1 + 0x68) = 0;
              FUN_00410530/*CSWGuiManager::AddPanel*/(iVar1,3,1);
              FUN_0073fee0/*CClientExoApp::SetInputClass*/(2,1);
            }
          }
        }
      }
    }
  }
... (4 more lines)
```

## R021 FUNCTION CSWSPlaceable::GetEffectSpellId  steam=0x005ef160
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=20 ret_N=0
```c
// FN 005ef160 FUN_005ef160 size=20 ret=0 callers= callees=

undefined4 __fastcall FUN_005ef160(int param_1)

{
  return *(undefined4 *)(param_1 + 0x3e0);
}

```

## R022 FUNCTION Scene::IsGeomFadeEnabled  steam=0x00464cd0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): bool (__stdcall)() param_bytes=0
Steam function at this address: size=25 ret_N=0
```c
// FN 00464cd0 FUN_00464cd0 size=25 ret=0 callers= callees=

undefined4 FUN_00464cd0(void)

{
  return CONCAT31((int3)((uint)DAT_009f5d28 >> 8),DAT_009f5d28 == DAT_0098ba8c);
}

```

## R023 FUNCTION Gob::DisableDistortion  steam=0x00459a10
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=21 ret_N=0
```c
// FN 00459a10 FUN_00459a10 size=21 ret=0 callers= callees=

void __fastcall FUN_00459a10(int param_1)

{
  *(undefined1 *)(param_1 + 0x1a8) = 0;
  return;
}

```

## R024 FUNCTION CSWGuiBorderParams::SetCornerImage  steam=0x00414680
GOG build: param_bytes=8 ghidra_sig='CSWGuiBorderParams * this, CResRef * image, int forceUpdate'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* image, int forceUpdate) param_bytes=8
Steam function at this address: size=219 ret_N=8
```c
// FN 00414680 FUN_00414680 size=219 ret=8 callers=00415080,00414920,00747210 callees=00710ca0,00710a70,00710a10,0047eb60,00710980,009196ee

void __thiscall FUN_00414680(int param_1,undefined4 param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_2c [16];
  undefined1 local_1c [20];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  iVar2 = FUN_00710a70(param_1 + 0x24);
  if (((iVar2 != 0) || (param_3 != 0)) &&
     (FUN_00710ca0(local_2c,param_2), *(int *)(param_1 + 0x54) != 0)) {
    if (*(int *)(*(int *)(param_1 + 0x54) + 0x6c) != 0) {
      piVar1 = *(int **)(*(int *)(param_1 + 0x54) + 0x6c);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 4))(1);
      }
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x6c) = 0;
    }
    iVar2 = FUN_00710a10();
    if (iVar2 != 0) {
      FUN_00710980(local_1c);
      uVar3 = FUN_0047eb60(local_1c);
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x6c) = uVar3;
    }
  }
  __security_check_cookie(local_8 ^ (uint)&stack0xfffffffc);
  return;
}

```

## R025 FUNCTION CClientExoApp::GetGameOver  steam=0x007403e0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 007403e0 FUN_007403e0 size=31 ret=0 callers=0053a0c0 callees=

undefined4 __fastcall FUN_007403e0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x2c0);
}

```

## R026 FUNCTION CSWGuiPanel::SetBackground  steam=0x0040f390
GOG build: param_bytes=4 ghidra_sig='CSWGuiPanel * this, CResRef * image'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* image) param_bytes=4
Steam function at this address: size=41 ret_N=4
```c
// FN 0040f390 FUN_0040f390 size=41 ret=4 callers=007983e0,007467c0,00885950,00880460,0087fe20 callees=00414840

void FUN_0040f390(undefined4 param_1)

{
  FUN_00414840/*CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force*/(param_1,0);
  return;
}

```
