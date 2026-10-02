## R378 FUNCTION Gob::DisableRenderBumpedOut  steam=0x0045c490
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=234 ret_N=0
```c
// FN 0045c490 FUN_0045c490 size=234 ret=0 callers= callees=00463b20,004741e0,00463ed0,004276a0

void __fastcall FUN_0045c490(int *param_1)

{
  uint uVar1;
  int local_24;
  int local_20;
  int local_1c;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094d3d8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(char *)((int)param_1 + 0x15) != '\0') {
    *(undefined1 *)((int)param_1 + 0x15) = 0;
    FUN_00463ed0(0);
    local_8 = 0;
    (**(code **)(*param_1 + 0x110))(FUN_0045bf30,&local_20,uVar1);
    for (local_24 = 0; local_24 < local_1c; local_24 = local_24 + 1) {
      local_14 = *(undefined4 *)(*(int *)(*(int *)(local_20 + local_24 * 4) + 0x44) + 0xc);
      FUN_004741e0(0);
      FUN_004276a0/*ReleaseTextureProxy (delete if back-ref count 0)*/(local_14);
    }
    param_1[0x52] = 0;
    local_8 = 0xffffffff;
    FUN_00463b20();
  }
  ExceptionList = local_10;
  return;
}

```

## R379 FUNCTION CSWGuiPanel::OnPanelAdded  steam=0x0040f3c0
GOG build: param_bytes=0 ghidra_sig='CSWGuiPanel * this'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=245 ret_N=0
```c
// FN 0040f3c0 FUN_0040f3c0 size=245 ret=0 callers=0075a1f0,00912600,009158a0,007dcea0,00885770,00903ba0,008c70b0,008f61d0,0088c650,008dd520,008b3630,008abb30,008bec20,00858a70,00894320,008b98f0,0089e000,008a74c0,008d2d70,00908460,007fc290,008984d0,00919040,008928f0,008cbde0,0084e6c0,00888d80,009074b0,008a24f0,008d8350,0090ad90,008a3bc0,008bd5c0,008fa550,008b53e0,0090e810,0075dd10,007605f0,0075ca60,008c8880 callees=00413c50

void __fastcall FUN_0040f3c0(int *param_1)

{
  int *piVar1;
  int iVar2;
  int local_8;
  
  (**(code **)(*param_1 + 0x6c))();
  FUN_00413c50();
  piVar1 = *(int **)(param_1[7] + 8);
  if (piVar1 != (int *)0x0) {
    for (local_8 = 0; local_8 < param_1[10]; local_8 = local_8 + 1) {
      if (*(int **)(param_1[9] + local_8 * 4) == piVar1) {
        if (((uint)piVar1[0x12] >> 3 & 1) == 0) {
          return;
        }
        *(int **)(param_1[7] + 8) = piVar1;
        (**(code **)(*piVar1 + 0xc))(1);
        if (param_1[8] == 0) {
          return;
        }
        if (piVar1 == (int *)param_1[8]) {
          return;
        }
        iVar2 = (**(code **)(*piVar1 + 0x58))();
        if (iVar2 == 0) {
          return;
        }
        if (((uint)piVar1[0x12] >> 2 & 1) == 0) {
          return;
        }
        (**(code **)(*(int *)param_1[8] + 0x3c))(1,1);
        return;
      }
    }
  }
  return;
}

```

## R380 FUNCTION CGuiInGame::ChangeCharacter  steam=0x007d02b0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=25 ret_N=0
```c
// FN 007d02b0 FUN_007d02b0 size=25 ret=0 callers=007b12c0 callees=00750e60

void __fastcall FUN_007d02b0(undefined4 param_1)

{
  FUN_00750e60(param_1);
  return;
}

```

## R381 FUNCTION CSWCObject::SetPortrait  steam=0x007f1b30
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* portrait) param_bytes=4
Steam function at this address: size=68 ret_N=4
```c
// FN 007f1b30 FUN_007f1b30 size=68 ret=4 callers= callees=0040e770,00710ca0

void __thiscall FUN_007f1b30(int *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined1 local_24 [16];
  undefined1 local_14 [16];
  
  FUN_00710ca0(local_24,param_2);
  uVar1 = (**(code **)(*param_1 + 0xf0))(local_14,1);
  FUN_0040e770(uVar1);
  return;
}

```

## R382 FUNCTION CSWCObject::TurnOffShadows  steam=0x007ed500
GOG build: param_bytes=0 ghidra_sig='CSWCObject * this'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=42 ret_N=0
```c
// FN 007ed500 CPaneContainerManager::NotifyPaneDivider size=42 ret=0 callers= callees=

/* Library Function - Single Match
    public: void __thiscall CPaneContainerManager::NotifyPaneDivider(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CPaneContainerManager::NotifyPaneDivider(CPaneContainerManager *this)

{
  if (*(int *)(this + 0x68) != 0) {
    (**(code **)(**(int **)(this + 0x68) + 0xbc))();
  }
  return;
}

```

## R383 FUNCTION CSWCObject::SetPortraitId  steam=0x007f1f10
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, WORD id) param_bytes=4
Steam function at this address: size=152 ret_N=4
```c
// FN 007f1f10 FUN_007f1f10 size=152 ret=4 callers= callees=005160c0

void __thiscall FUN_007f1f10(int param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  undefined1 local_20 [16];
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  *(undefined4 *)(param_1 + 0xd8) = *param_2;
  *(undefined4 *)(param_1 + 0xdc) = param_2[1];
  *(undefined4 *)(param_1 + 0xe0) = param_2[2];
  if (*(int *)(param_1 + 0x68) != 0) {
    local_10 = 0;
    local_c = 0;
    local_8 = 0x3f800000;
    uVar1 = FUN_005160c0(local_20,0,0,0x3f800000,*param_2,param_2[1],param_2[2]);
    (**(code **)(**(int **)(param_1 + 0x68) + 0xb4))(uVar1);
  }
  return;
}

```

## R384 FUNCTION CGuiInGame::LoadAmbientTrackForDialog  steam=0x007c41f0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* track) param_bytes=4
Steam function at this address: size=8198 ret_N=24
```c
// FN 007c41f0 FUN_007c41f0 size=8198 ret=24 callers=007c6670,007cbf60,007c7d60 callees=0051c8b0,00515990,0057d370,008c6340,00773bf0,007741a0,007409c0,0073f750,008c5740,007409a0,005143c0,00543f10,0071b670,007cf710,00922a3d,0073f550,00733570,0051c0a0,005195a0,0077fc10,0073f430,007cae60,00733780,005158b0,0073f4d0,00515a70,00407380,00515620,007cf8f0,007cb010,0091e620

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __thiscall
FUN_007c41f0(int param_1,int param_2,int param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6,int param_7)

{
  float *pfVar1;
  ushort uVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  undefined4 *puVar6;
  int *piVar7;
  float10 fVar8;
  undefined8 uVar9;
  float local_43c;
  float local_434;
  float local_42c;
  float local_40c;
  float local_404;
  float local_3fc;
  undefined1 local_37c [12];
  float local_370;
  float local_36c;
  float local_368;
  float local_364;
  float local_360;
  float local_35c;
  int local_358;
  undefined1 local_354 [12];
  int local_348;
  undefined4 local_344;
  undefined4 local_340;
  int local_33c;
  float *local_338;
  float local_334;
  float local_330;
  float local_32c;
  float local_328;
  float local_324;
  float local_320;
  float local_31c;
  float local_318;
  float local_314;
  float local_310;
  float local_30c;
  float local_308;
  float local_304;
  float local_300;
  float local_2fc;
  float *local_2f8;
  float *local_2f4;
  float local_2f0;
  float local_2ec;
  float local_2e8;
  int local_2e4;
  float local_2e0;
  int local_2dc;
  int local_2d8;
  int local_2d4;
  undefined1 local_2d0 [12];
  int local_2c4;
  undefined1 local_2c0 [12];
  int local_2b4;
  undefined4 local_2b0;
  undefined4 local_2ac;
... (724 more lines)
```

## R385 FUNCTION CSWSPlaceable::SetLightIsOn  steam=0x005f6470
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int lightState) param_bytes=4
Steam function at this address: size=251 ret_N=4
```c
// FN 005f6470 FUN_005f6470 size=251 ret=4 callers=00692640,005ef570 callees=0071bcf0,00733570,00733780

void __thiscall FUN_005f6470(int param_1,int param_2)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  char local_38;
  undefined1 local_20 [11];
  char local_15;
  undefined1 local_14 [4];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954699;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  bVar1 = false;
  if (*(int *)(param_1 + 0x3e4) != param_2) {
    FUN_00733570/*CExoString::CExoString_char*/("LightColor");
    local_8 = 0;
    bVar1 = true;
    iVar3 = FUN_0071bcf0(*(undefined4 *)(param_1 + 0x270),local_20,local_14);
    if (iVar3 != 0) {
      local_38 = '\0';
      goto LAB_005f6516;
    }
  }
  local_38 = '\x01';
LAB_005f6516:
  local_15 = local_38;
  local_8 = 0xffffffff;
  if (bVar1) {
    FUN_00733780/*CExoString::~CExoString*/(uVar2);
  }
  if (local_15 == '\0') {
    *(undefined4 *)(param_1 + 1000) = 1;
    *(int *)(param_1 + 0x3e4) = param_2;
  }
  ExceptionList = local_10;
  return;
}

```

## R386 FUNCTION CGuiInGame::GetPendingStatusSummary  steam=0x007cf270
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=318 ret_N=0
```c
// FN 007cf270 FUN_007cf270 size=318 ret=0 callers=00781be0 callees=0072fb00,00410530,00740350,0073fee0,0075db70,0075fbc0,0075fc30,00740ec0,007628a0

void __fastcall FUN_007cf270(int param_1)

{
  int iVar1;
  uint local_18;
  
  if (*(int *)(param_1 + 0xb8) == 0) {
    return;
  }
  if (((*(uint *)(*(int *)(param_1 + 0xb8) + 0x68) & 1) != 0) &&
     (iVar1 = FUN_0072fb00(), (*(uint *)(iVar1 + 0x1c) >> 5 & 1) != 0)) {
    if ((*(int *)(param_1 + 0xf18) == 0) && (iVar1 = FUN_00740ec0(), iVar1 == 0)) {
      local_18 = 0;
    }
    else {
      local_18 = 1;
    }
    *(uint *)(*(int *)(param_1 + 0xb8) + 0x90) =
         *(uint *)(*(int *)(param_1 + 0xb8) + 0x90) & 0xfffffffe | local_18;
    FUN_00740350(1,6,0);
    FUN_00410530/*CSWGuiManager::AddPanel*/(*(undefined4 *)(param_1 + 0xb8),1,1);
    FUN_0073fee0/*CClientExoApp::SetInputClass*/(2,0);
    return;
  }
  FUN_0075fbc0();
  FUN_0075fc30();
  FUN_007628a0(0);
  FUN_0075db70();
  return;
}

```

## R387 FUNCTION CSWSObject::SpawnBodyBag  steam=0x00548db0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=2505 ret_N=12
```c
// FN 00548db0 FUN_00548db0 size=2505 ret=12 callers=0059ec70 callees=0091ea50,004452b0,005143c0,00733570,009201d0,0091eb8b,0051c0a0,0091c860,00733780,0091e770,0071b670

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * FUN_00548db0(undefined4 *param_1)

{
  uint uVar1;
  int iVar2;
  float *pfVar3;
  int iVar4;
  float10 fVar5;
  undefined1 local_118 [12];
  float local_10c;
  float local_108;
  float local_104;
  float local_100;
  float local_fc;
  float local_f8;
  undefined1 local_f4 [12];
  undefined1 local_e8 [12];
  undefined1 local_dc [12];
  undefined1 local_d0 [12];
  float local_c4;
  float local_c0;
  float local_bc;
  undefined1 local_ac [12];
  float local_a0;
  float local_9c;
  float local_98;
  undefined1 local_94 [8];
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  undefined4 local_70;
  int *local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  int *local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  int local_40;
  float local_3c;
  float local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  float local_1c;
  float local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094fb5b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
... (144 more lines)
```

## R388 FUNCTION CClientExoApp::GetPazaakIsRunning  steam=0x00740f20
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=28 ret_N=0
```c
// FN 00740f20 FUN_00740f20 size=28 ret=0 callers=007d0d90 callees=

undefined4 __fastcall FUN_00740f20(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x70);
}

```

## R389 FUNCTION CClientExoApp::GetLastTarget  steam=0x00740640
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=32 ret_N=8
```c
// FN 00740640 FUN_00740640 size=32 ret=8 callers=007e5df0,006da4f0 callees=007a0530

void FUN_00740640(undefined4 param_1,undefined4 param_2)

{
  FUN_007a0530(param_1,param_2);
  return;
}

```

## R390 FUNCTION Gob::GetPreviousPosition  steam=0x00459780
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector* outPosition) param_bytes=4
Steam function at this address: size=43 ret_N=4
```c
// FN 00459780 FUN_00459780 size=43 ret=4 callers= callees=

undefined4 * __thiscall FUN_00459780(int param_1,undefined4 *param_2)

{
  *param_2 = *(undefined4 *)(param_1 + 0xc0);
  param_2[1] = *(undefined4 *)(param_1 + 0xc4);
  param_2[2] = *(undefined4 *)(param_1 + 200);
  return param_2;
}

```

## R391 FUNCTION Gob::GetCutsceneDummyPosition  steam=0x00459e40
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): Vector* (__thiscall)(void* thisPtr, Vector* outPosition) param_bytes=4
Steam function at this address: size=154 ret_N=4
```c
// FN 00459e40 FUN_00459e40 size=154 ret=4 callers= callees=00456b00

undefined4 * __thiscall FUN_00459e40(int param_1,undefined4 *param_2)

{
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  int *local_8;
  
  local_8 = (int *)FUN_00456b00(*(undefined4 *)(param_1 + 0x98),"cutscenedummy");
  if (local_8 == (int *)0x0) {
    *param_2 = 0;
    param_2[1] = 0;
    param_2[2] = 0;
  }
  else {
    local_24 = 0;
    local_20 = 0;
    local_1c = 0;
    local_c = 0;
    local_10 = 0;
    local_14 = 0;
    local_18 = 0x3f800000;
    (**(code **)(*local_8 + 0xc))(&local_24,&local_18);
    *param_2 = local_24;
    param_2[1] = local_20;
    param_2[2] = local_1c;
  }
  return param_2;
}

```

## R392 FUNCTION Gob::AttachToScene  steam=0x0046d440
GOG build: param_bytes=4 ghidra_sig='Gob * this, undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, void* scene) param_bytes=4
Steam function at this address: size=38 ret_N=4
```c
// FN 0046d440 FUN_0046d440 size=38 ret=4 callers= callees=00461aa0

undefined4 FUN_0046d440(undefined4 param_1)

{
  FUN_00461aa0(param_1);
  return 1;
}

```

## R393 FUNCTION CSWSObject::GetClientObject  steam=0x0054a840
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
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

## R394 FUNCTION Gob::RemoveAddInAnimations  steam=0x0045a150
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=54 ret_N=0
```c
// FN 0045a150 FUN_0045a150 size=54 ret=0 callers= callees=00452cf0

void __fastcall FUN_0045a150(int param_1)

{
  if (*(int *)(param_1 + 0x90) != 0) {
    FUN_00452cf0(param_1);
  }
  *(undefined4 *)(param_1 + 0x90) = 0;
  return;
}

```

## R395 FUNCTION CGuiInGame::HideAreaTransition  steam=0x007ce970
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=281 ret_N=0
```c
// FN 007ce970 FUN_007ce970 size=281 ret=0 callers=007ce780 callees=008b88b0,00410670,00410780,0073f430,00410800,007aae00

void __fastcall FUN_007ce970(int param_1)

{
  int iVar1;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  local_8 = 0;
  iVar1 = FUN_0073f430();
  if (iVar1 != 0) {
    iVar1 = FUN_0073f430();
    local_c = *(int *)(iVar1 + 0x48);
    if (((local_c != 0) && (local_8 = FUN_007aae00(), local_8 != 0)) &&
       (*(int *)(local_8 + 0x2f8) != 0)) {
      iVar1 = CMFCVisualManagerBitmapCache::IsCached
                        (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                         *(CSize **)(param_1 + 0x9c));
      if (iVar1 == 0) {
        return;
      }
      FUN_00410780(*(undefined4 *)(param_1 + 0x9c));
      return;
    }
  }
  if (((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x38) != 0)) &&
     (*(int *)(param_1 + 0x9c) != 0)) {
    local_18 = 0;
    local_14 = 0;
    local_10 = 0;
    FUN_008b88b0(0x7f000000,&local_18);
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x9c));
    if (iVar1 != 0) {
      FUN_00410670(*(undefined4 *)(param_1 + 0x9c));
    }
  }
  return;
}

```

## R396 FUNCTION CResGFF::WriteFieldSHORT  steam=0x00719b20
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined2 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, short value, char* label) param_bytes=12
Steam function at this address: size=97 ret_N=12
```c
// FN 00719b20 FUN_00719b20 size=97 ret=12 callers=0076aed0,0061bfe0,006b4ea0,0058d2f0,006b3d10,005f18c0 callees=00717aa0,00718030

undefined4 __thiscall FUN_00719b20(int param_1,int param_2,short param_3,int param_4)

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
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,3);
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

## R397 FUNCTION CSWSTrigger::GetTargetArea  steam=0x0061e540
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=170 ret_N=0
```c
// FN 0061e540 FUN_0061e540 size=170 ret=0 callers=00557ff0,0054cd50 callees=00557ec0,0051c080,0051bd90,005453c0,0053dfb0

undefined4 __fastcall FUN_0061e540(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  int **ppiVar4;
  int *local_c;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x2f4) == 0) {
    uVar2 = 0x7f000000;
  }
  else {
    param_1 = param_1 + 0x270;
    FUN_0051bd90(param_1);
    uVar2 = FUN_00557ec0(param_1);
    ppiVar4 = &local_c;
    local_8 = uVar2;
    FUN_0051c080(uVar2,ppiVar4);
    cVar1 = FUN_0053dfb0/*CGameObjectArray::GetGameObject*/(uVar2,ppiVar4);
    if (cVar1 == DAT_0099447f) {
      (**(code **)(*local_c + 0x58))();
      iVar3 = FUN_005453c0();
      uVar2 = *(undefined4 *)(iVar3 + 0x124);
    }
    else {
      uVar2 = 0x7f000000;
    }
  }
  return uVar2;
}

```

## R398 FUNCTION CResGFF::AddField  steam=0x007181f0
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): CResGFFField* (__thiscall)(void* thisPtr, CResStruct* structPtr, char* labelText, GFFFieldTypes fieldType) param_bytes=12
Steam function at this address: size=371 ret_N=4
```c
// FN 007181f0 FUN_007181f0 size=371 ret=4 callers=00718030 callees=0091dba0,009196fd,0091d0f0,0091c630,00919723,009196ee

void __thiscall FUN_007181f0(int param_1,char *param_2)

{
  uint uVar1;
  longlong lVar2;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  uint local_14;
  uint local_10;
  void *local_c;
  int *local_8;
  
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if ((*(int *)(param_1 + 0x40) != 0) && (*(int *)(param_1 + 0x54) != 0)) {
    uVar1 = *(uint *)(*(int *)(param_1 + 0x40) + 0x1c);
    _memset(&local_24,0,0x10);
    _strncpy((char *)&local_24,param_2,0x10);
    for (local_10 = uVar1; local_10 != 0; local_10 = local_10 - 1) {
      local_8 = (int *)((local_10 - 1) * 0x10 + *(int *)(param_1 + 0x54));
      if ((((local_24 == *local_8) && (local_20 == local_8[1])) && (local_1c == local_8[2])) &&
         (local_18 == local_8[3])) goto LAB_00718355;
    }
    if (*(uint *)(param_1 + 0x58) <= uVar1) {
      *(uint *)(param_1 + 0x58) = uVar1 + *(int *)(param_1 + 0x30);
      lVar2 = (ulonglong)*(uint *)(param_1 + 0x58) * 0x10;
      local_c = (void *)FUN_00919723/*operator new*/(-(uint)((int)((ulonglong)lVar2 >> 0x20) != 0) | (uint)lVar2);
      _memcpy(local_c,*(void **)(param_1 + 0x54),uVar1 << 4);
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x54));
      *(void **)(param_1 + 0x54) = local_c;
    }
    *(uint *)(*(int *)(param_1 + 0x40) + 0x1c) = uVar1 + 1;
    _memcpy((void *)(uVar1 * 0x10 + *(int *)(param_1 + 0x54)),&local_24,0x10);
  }
LAB_00718355:
  __security_check_cookie(local_14 ^ (uint)&stack0xfffffffc);
  return;
}

```

## R399 FUNCTION CSWCObject::DetachFromServerObject  steam=0x007f2630
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=24 ret_N=0
```c
// FN 007f2630 FUN_007f2630 size=24 ret=0 callers=0053f050 callees=

void __fastcall FUN_007f2630(int param_1)

{
  *(undefined4 *)(param_1 + 0xf8) = 0;
  return;
}

```

## R400 FUNCTION CSWSObject::SetDialogOwner  steam=0x00546db0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* owner) param_bytes=4
Steam function at this address: size=22 ret_N=4
```c
// FN 00546db0 FUN_00546db0 size=22 ret=4 callers=006c4930,00545760,006c7a30 callees=

void __thiscall FUN_00546db0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x54) = param_2;
  return;
}

```

## R401 FUNCTION CSWGuiBorder::FillTile  steam=0x00415a60
GOG build: param_bytes=24 ghidra_sig='CSWGuiBorder * this, int height, int width, int x, int y, float alpha, Vector * color'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int height, int width, int x, int y, float alpha, Vector* color) param_bytes=24
Steam function at this address: size=702 ret_N=24
```c
// FN 00415a60 FUN_00415a60 size=702 ret=24 callers=00415300 callees=

void __thiscall
FUN_00415a60(int param_1,int param_2,int param_3,int param_4,int param_5,undefined4 param_6,
            undefined4 param_7)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_2c;
  int local_28;
  int local_14;
  int local_10;
  
  iVar3 = (**(code **)(**(int **)(param_1 + 0x74) + 0x10))();
  iVar4 = (**(code **)(**(int **)(param_1 + 0x74) + 0xc))();
  if ((0 < iVar3) && (0 < iVar4)) {
    iVar5 = param_3 / iVar3;
    iVar6 = param_2 / iVar4;
    uVar1 = *(undefined4 *)(param_1 + 0x20);
    param_2 = param_2 - iVar6 * iVar4;
    iVar7 = param_3 - iVar5 * iVar3;
    if (iVar6 != 0) {
      local_14 = param_2 / iVar6;
      param_2 = param_2 - local_14 * iVar6;
    }
    iVar2 = param_2;
    if (iVar5 == 0) {
      if (iVar6 == 0) {
        (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
                  (param_4,param_5,iVar7,param_2,0,uVar1,param_7,param_6);
      }
      else {
        local_40 = param_5;
        for (local_10 = 0; local_10 < iVar6; local_10 = local_10 + 1) {
          local_44 = iVar4 + local_14;
          if (param_2 != 0) {
            param_2 = param_2 + -1;
            local_44 = local_44 + 1;
          }
          (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
                    (param_4,local_40,iVar7,local_44,0,uVar1,param_7,param_6);
          local_40 = local_40 + local_44;
        }
      }
    }
    else {
      param_3 = iVar7 - (iVar7 / iVar5) * iVar5;
      local_28 = param_4;
      for (local_10 = 0; local_10 < iVar5; local_10 = local_10 + 1) {
        local_34 = iVar3 + iVar7 / iVar5;
        if (param_3 != 0) {
          param_3 = param_3 + -1;
          local_34 = local_34 + 1;
        }
        if (iVar6 == 0) {
          (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
                    (local_28,param_5,local_34,param_2,0,uVar1,param_7,param_6);
        }
        else {
... (20 more lines)
```

## R402 FUNCTION CGuiInGame::IsCameraDialog  steam=0x007cb750
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=43 ret_N=0
```c
// FN 007cb750 FUN_007cb750 size=43 ret=0 callers=006cc140,007cb350,007cb3f0,007c6670,008ba330,007cbf60,007c7d60 callees=

undefined4 __fastcall FUN_007cb750(int param_1)

{
  undefined4 uVar1;
  
  if ((*(int *)(param_1 + 0x3c) == 0) || (*(int *)(param_1 + 0x3c) != *(int *)(param_1 + 0x40))) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

```

## R403 FUNCTION CClientExoApp::CancelMovie  steam=0x00740f60
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 00740f60 FUN_00740f60 size=22 ret=0 callers=0040a750 callees=007a0500

void __fastcall FUN_00740f60(undefined4 param_1)

{
  FUN_007a0500(param_1);
  return;
}

```

## R404 FUNCTION CResGFF::ReadFieldDWORD64  steam=0x00718d80
GOG build: param_bytes=20 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5'
Wrapper typedef (what calling code expects): DWORD64 (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label, int* success, DWORD64 defaultValue) param_bytes=16
Steam function at this address: size=181 ret_N=20
```c
// FN 00718d80 FUN_00718d80 size=181 ret=20 callers=005e5510,00855f30,005582f0 callees=00717770,00717820,007178e0

undefined8
FUN_00718d80(int param_1,int param_2,undefined4 *param_3,undefined4 param_4,undefined4 param_5)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  uint local_10 [2];
  int *local_8;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_2);
    local_8 = (int *)FUN_00717770(param_1,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 6)) {
      *param_3 = 0;
    }
    else {
      puVar2 = (undefined4 *)FUN_00717820/*CResGFF::GetDataField*/(local_8,local_10);
      if ((puVar2 == (undefined4 *)0x0) || (local_10[0] < 8)) {
        *param_3 = 0;
      }
      else {
        *param_3 = 1;
        param_4 = *puVar2;
        param_5 = puVar2[1];
      }
    }
  }
  return CONCAT44(param_5,param_4);
}

```
