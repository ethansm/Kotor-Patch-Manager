## R270 FUNCTION CSWGuiPanel::Destructor  steam=0x0040e860
GOG build: param_bytes=0 ghidra_sig='CSWGuiPanel * this'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=276 ret_N=0
```c
// FN 0040e860 FUN_0040e860 size=276 ret=0 callers=00975670,0087fdf0,009811e0,00897e70,009766d0,0097af70,0074a730,0097f3d0,008855c0,0096a410,00969500,00978c80,00912220,009747c0,00976370,00979b20,0097ef50,009815f0,0084df70,007dcb00,0097d0a0,0097a3a0,0097c6e0,00978160,00894140,00758a70,00979d80,0097da80,00917a30,007563b0,009694c0,0096a860,00979ab0,00977cd0,00963fd0,00980ec0,00905b80,0097ab70,0097fc90,00963ed0,0096b110,00908070,00971290,00982340,00887920,00916a10,008d7af0,00969ee0,008b4f60,0097c3b0,0097fb90,009770e0,009826c0,007d8f20,00982290,0096ae50,008b2620,0097a280,009745f0,0097fda0,0097cd10,00980c20,00977640,00982080,00974570,008c84c0,008803d0,008baaa0,00974e30,00963320,00918f00,0096a090,00976db0,0097adf0,00971e80,009824b0,00969fe0,0090e440,0097ad40,00982130,008bf000,0089dc30,0097d670,0097dbc0,008b9e80,009645e0,0096afb0,00977f50,0074ad90,0096a620,0075fea0,007d2a80,0097bd10,0097a4d0,0089ca70,009798d0,008f5be0,0096be00,00760520,007db4c0,009751b0,009157d0,00980640,009801d0,0097eba0,009818d0,009828a0,009785e0,00978db0,0097a880,00903ad0,008cad50,008fd490,0097e6f0,0097f020,0074a650,00962bd0,0097b970,008b8790,00979860,0097ed70,00979d10,007da3c0,00963700,007fbad0,008c1f20,008dc620,009718e0,00972db0,009808e0,00979bf0,007d70d0,00975fc0,0088c110,009005d0,00963f50,007d52e0,00979c60,0097e2b0,00975490,008a1960,00964560,009823e0,0097a630,008fa2f0,00974d70,008925f0,0097a240,00914510,00974ff0,0075b260,009793b0,00851390,00976850,0097d8c0,0096b290,00979090,00964520,00976ca0,008a7060,008fbbf0,0096aa40,0097a310,00979d50,009640a0,008d21c0,00962910,00963c60,008c6e90,00975a40,008bde40,009774c0,008e07b0,0097aa10,0096a230,00881670,00972d30,008a3240,0097b250,00858970,007d3600,00976e60,009622d0,00977910,0096b3c0,00888ad0,0096a160,009752b0,0096ab10,00981ce0,0075d820,0097a7d0,00964250,009821e0,00982970,007bc760,008aa750,00976050,009720b0,008b94a0,0096ac90,00974540,00963a40,008e1f10,0090a9a0,00976230,00977320,0097f1e0,0097f770,0096c000,0097f640 callees=0040f5a0,00410800,0064ddb0,00410670

void __fastcall FUN_0040e860(CSize *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_009482e3;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(undefined ***)param_1 = CSWGuiPanel::vftable;
  local_8 = 1;
  if (*(undefined4 **)(param_1 + 0x60) == (undefined4 *)0x0) {
    uVar2 = 0;
  }
  else {
    uVar2 = (**(code **)**(undefined4 **)(param_1 + 0x60))(1,uVar1);
  }
  *(undefined4 *)(param_1 + 0x60) = 0;
  FUN_0040f5a0/*CSWGuiPanel::StopLoadFromLayout*/();
  if (*(undefined4 **)(param_1 + 0x30) == (undefined4 *)0x0) {
    uVar3 = 0;
  }
  else {
    uVar3 = (**(code **)**(undefined4 **)(param_1 + 0x30))(1);
  }
  *(undefined4 *)(param_1 + 0x30) = 0;
  if (*(int *)(param_1 + 0x1c) != 0) {
    iVar4 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x1c),param_1);
    if (iVar4 != 0) {
      FUN_00410670(param_1);
    }
  }
  local_8 = local_8 & 0xffffff00;
  Free(uVar1,uVar3,uVar2);
  *(undefined ***)param_1 = CSWGuiObject::vftable;
  ExceptionList = local_10;
  return;
}

```

## R271 FUNCTION CSWCObject::SetPortraitId  steam=0x007f1fb0
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, WORD id) param_bytes=4
Steam function at this address: size=162 ret_N=4
```c
// FN 007f1fb0 FUN_007f1fb0 size=162 ret=4 callers= callees=0085e890,00623320,0085e8c0,0085ead0,009196ee

void __thiscall FUN_007f1fb0(int *param_1,undefined2 param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_34 [32];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096ba48;
  local_10 = ExceptionList;
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0085e890(local_14);
  local_8 = 0;
  iVar1 = FUN_0085ead0(param_2);
  if (iVar1 != 0) {
    uVar2 = FUN_0085e8c0(local_34,0);
    (**(code **)(*param_1 + 0xec))(uVar2);
  }
  *(undefined2 *)(param_1 + 0x2e) = param_2;
  local_8 = 0xffffffff;
  FUN_00623320();
  ExceptionList = local_10;
  __security_check_cookie(local_14 ^ (uint)&stack0xfffffffc);
  return;
}

```

## R272 FUNCTION CClientExoApp::SetInputClass  steam=0x0073fee0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int inputClass, int setGuiStatus) param_bytes=8
Steam function at this address: size=32 ret_N=8
```c
// FN 0073fee0 FUN_0073fee0 size=32 ret=8 callers=00853a00,007cab00,008aed10,007cb7c0,0089ed90,008faca0,00746670,0075d950,00883560,008a2070,005d56b0,007caa60,007cd7e0,007cdaa0,007d1e90,00894e90,007cacb0,0084f900,007cdda0,008079b0,007cad30,007cb920,007cdf10,007ca920,007cd760,007d2030,007cb540,00753680,007bfb10,00796ba0,0075cc60,007ccf30,008a3660,007467c0,00751580,008925f0,007cf270,007c6a20,0075ca60,008928f0,007cab60,007caa10,007cb4a0,008a8880,0069bcf0,007605f0,007cac10,007ca730,008f82c0,007cdcc0,00545760,0089fbd0,008a07d0,0074c320,008f8070,007672f0,007cdbe0,007fbd90,007ccd40,007d1d10,007cd510,00754950 callees=007b3050

void FUN_0073fee0(undefined4 param_1,undefined4 param_2)

{
  FUN_007b3050(param_1,param_2);
  return;
}

```

## R273 FUNCTION CSWCDoor::SetIsAreaTransition  steam=0x0087b980
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int isTransition) param_bytes=4
Steam function at this address: size=86 ret_N=4
```c
// FN 0087b980 FUN_0087b980 size=86 ret=4 callers=0080c6b0 callees=

void __thiscall FUN_0087b980(int *param_1,int param_2)

{
  int *piVar1;
  
  if (param_2 == 0) {
    piVar1 = (int *)(**(code **)(*param_1 + 0x98))(0xff,1);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0xb0))("trans");
    }
  }
  param_1[0x4c] = param_2;
  return;
}

```

## R274 FUNCTION CSWGuiControl::AddChildControl  steam=0x00419330
GOG build: param_bytes=4 ghidra_sig='CSWGuiControl * this, CSWGuiControl * child'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* child) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 00419330 FUN_00419330 size=28 ret=4 callers=00418f20,009017b0,00741940 callees=0083ea60

void FUN_00419330(undefined4 param_1)

{
  FUN_0083ea60/*CExoArrayList<uint32>::Add*/(param_1);
  return;
}

```

## R275 FUNCTION CGuiInGame::ShowUpgradeScreen  steam=0x007cd7e0
GOG build: param_bytes=20 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD itemId) param_bytes=4
Steam function at this address: size=701 ret_N=24
```c
// FN 007cd7e0 FUN_007cd7e0 size=701 ret=24 callers=00692e50 callees=00733570,00410530,0073f810,007335f0,008c6650,0087a730,0073fee0,00919723,007337c0,0051c0e0,0070bc60,00733780

void __thiscall
FUN_007cd7e0(int param_1,int param_2,undefined4 param_3,int param_4,int param_5,char *param_6)

{
  bool bVar1;
  undefined1 uVar2;
  uint uVar3;
  undefined4 uVar4;
  undefined4 local_6c;
  undefined1 local_5c [8];
  undefined4 local_54;
  undefined1 local_50 [8];
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  undefined4 local_30;
  undefined4 local_2c;
  int local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_00969c43;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8 = 0;
  if (((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x38) != 0)) &&
     (*(int *)(param_1 + 0x88) == 0)) {
    local_14 = 1;
    if (param_2 != 0x7f000000) {
      local_2c = *(undefined4 *)(DAT_00a1b4a4 + 8);
      local_18 = FUN_0051c0e0(param_2);
      if (local_18 == 0) {
        local_14 = 0;
      }
    }
    if (local_14 != 0) {
      FUN_0070bc60(DAT_0099c2b8,0);
      if (*(int *)(param_1 + 0xf18) == 0) {
        local_30 = *(undefined4 *)(DAT_00a1b4a4 + 4);
        FUN_0073f810();
        FUN_0087a730();
      }
      local_20 = FUN_00919723/*operator new*/(0x18d8);
      local_8._0_1_ = 1;
      if (local_20 == 0) {
        local_6c = 0;
      }
      else {
        local_6c = FUN_008c6650(*(undefined4 *)(param_1 + 0x38),param_3);
      }
      local_1c = local_6c;
      local_8._0_1_ = 0;
      uVar2 = (undefined1)local_8;
      local_8._0_1_ = 0;
      *(undefined4 *)(param_1 + 0x88) = local_6c;
      local_34 = *(int *)(param_1 + 0x88);
      *(int *)(local_34 + 0x18a4) = param_2;
      local_38 = *(int *)(param_1 + 0x88);
      *(undefined4 *)(local_38 + 0x18a8) = param_3;
      if (param_4 == 0) {
... (47 more lines)
```

## R276 FUNCTION CSWGuiPanel::HitCheckMouse  steam=0x0040eae0
GOG build: param_bytes=8 ghidra_sig='CSWGuiPanel * this, int mouseX, int mouseY'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, int mouseX, int mouseY) param_bytes=8
Steam function at this address: size=287 ret_N=8
```c
// FN 0040eae0 FUN_0040eae0 size=287 ret=8 callers=0074b3a0,007bccb0,00894dc0 callees=00410020

int __thiscall FUN_0040eae0(int param_1,int param_2,int param_3)

{
  int iVar1;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_14 = *(int *)(param_1 + 4);
  local_10 = *(int *)(param_1 + 8);
  local_c = *(int *)(param_1 + 0xc);
  local_8 = *(int *)(param_1 + 0x10);
  if ((*(uint *)(param_1 + 0x48) & 1) == 0) {
    param_2 = param_2 - (DAT_009f42a4 + -0x280) / 2;
    param_3 = param_3 - (DAT_009f42a8 + -0x1e0) / 2;
  }
  else {
    FUN_00410020/*CSWGuiPanel::GetExtentAccountingForPanelOffset*/(&local_14);
  }
  if ((((param_2 < local_14) || (local_14 + local_c < param_2)) || (param_3 < local_10)) ||
     (local_10 + local_8 < param_3)) {
LAB_0040ebf7:
    iVar1 = 0;
  }
  else {
    param_2 = param_2 - local_14;
    param_3 = param_3 - local_10;
    local_18 = *(int *)(param_1 + 0x28);
    do {
      local_18 = local_18 + -1;
      if (local_18 < 0) goto LAB_0040ebf7;
    } while ((*(int *)(*(int *)(param_1 + 0x24) + local_18 * 4) == 0) ||
            (iVar1 = (**(code **)(**(int **)(*(int *)(param_1 + 0x24) + local_18 * 4) + 0x44))
                               (param_2,param_3), iVar1 == 0));
  }
  return iVar1;
}

```

## R277 FUNCTION CWorldTimer::GetWorldTimeSecond  steam=0x0051b180
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=49 ret_N=0
```c
// FN 0051b180 FUN_0051b180 size=49 ret=0 callers=006897c0 callees=0051ab40,0051b0e0

undefined4 __fastcall FUN_0051b180(undefined4 param_1)

{
  undefined4 uVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  undefined4 *puVar4;
  undefined1 *puVar5;
  undefined1 local_14 [4];
  undefined4 local_10;
  undefined1 local_c [4];
  undefined1 local_8 [4];
  
  puVar5 = local_8;
  puVar4 = &local_10;
  puVar3 = local_c;
  puVar2 = local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40/*CWorldTimer::ConvertFromTimeOfDay*/(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_10;
}

```

## R278 FUNCTION CSWCObject::SetFeedbackInfo  steam=0x007f2200
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* info) param_bytes=4
Steam function at this address: size=44 ret_N=4
```c
// FN 007f2200 FUN_007f2200 size=44 ret=4 callers=00670ae0,0078f160 callees=007337c0

void __thiscall FUN_007f2200(int param_1,undefined4 param_2)

{
  FUN_007337c0/*CExoString::operator=*/(param_2);
  *(undefined4 *)(param_1 + 0xf0) = 5000;
  return;
}

```

## R279 FUNCTION CGuiInGame::GetFadeOverridePanelVisible  steam=0x007d0d90
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=301 ret_N=0
```c
// FN 007d0d90 FUN_007d0d90 size=301 ret=0 callers=007bcab0 callees=00740f20,00410800

undefined4 __fastcall FUN_007d0d90(int param_1)

{
  int iVar1;
  undefined4 local_10;
  
  if ((*(int *)(param_1 + 0x128) == 0) || (*(int *)(param_1 + 0x38) == 0)) {
    local_10 = 0;
  }
  else if (((((*(int *)(param_1 + 0x80) == 0) ||
             (iVar1 = CMFCVisualManagerBitmapCache::IsCached
                                (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                                 *(CSize **)(param_1 + 0x80)), iVar1 == 0)) ||
            ((*(uint *)(*(int *)(param_1 + 0x80) + 0x48) >> 2 & 1) == 0)) &&
           (((*(int *)(param_1 + 0x78) == 0 ||
             (iVar1 = CMFCVisualManagerBitmapCache::IsCached
                                (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                                 *(CSize **)(param_1 + 0x78)), iVar1 == 0)) ||
            ((*(uint *)(*(int *)(param_1 + 0x78) + 0x48) >> 2 & 1) == 0)))) &&
          ((((*(int *)(param_1 + 0x84) == 0 ||
             (iVar1 = CMFCVisualManagerBitmapCache::IsCached
                                (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                                 *(CSize **)(param_1 + 0x84)), iVar1 == 0)) ||
            ((*(uint *)(*(int *)(param_1 + 0x84) + 0x48) >> 2 & 1) == 0)) &&
           ((((*(int *)(param_1 + 8) == 0 ||
              (iVar1 = CMFCVisualManagerBitmapCache::IsCached
                                 (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                                  *(CSize **)(param_1 + 8)), iVar1 == 0)) ||
             ((*(uint *)(*(int *)(param_1 + 8) + 0x48) >> 2 & 1) == 0)) &&
            (iVar1 = FUN_00740f20/*CClientExoApp::GetPazaakIsRunning*/(), iVar1 == 0)))))) {
    local_10 = 0;
  }
  else {
    local_10 = 1;
  }
  return local_10;
}

```

## R280 FUNCTION Gob::GetMinimumBoundingSphere  steam=0x004600a0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector* center, float* radius) param_bytes=8
Steam function at this address: size=71 ret_N=8
```c
// FN 004600a0 FUN_004600a0 size=71 ret=8 callers= callees=

void __thiscall FUN_004600a0(int *param_1,int *param_2,int *param_3)

{
  (**(code **)(*param_1 + 0x1d0))();
  *param_2 = param_1[0x5a];
  param_2[1] = param_1[0x5b];
  param_2[2] = param_1[0x5c];
  *param_3 = param_1[0x59];
  return;
}

```

## R281 FUNCTION CServerExoApp::GetGameObject  steam=0x0051c0a0
GOG build: param_bytes=4 ghidra_sig='CServerExoApp * this, undefined4 param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, DWORD objectId) param_bytes=4
Steam function at this address: size=62 ret_N=4
```c
// FN 0051c0a0 FUN_0051c0a0 size=62 ret=4 callers=005d53a0,0051eaa0,00680400,00681350,00687cb0,0068f440,006cae20,0068f9b0,00798d70,00698190,00669c80,0052d4b0,00548db0,0075fc30,00544790,00575df0,005d4b60,00680140,00686ff0,005dec20,0067d8a0,00686910,0067c6f0,0058f7a0,006926c0,0068fdc0,00687790,00682a20,0068b810,00815d00,0052bb60,00632a20,005555c0,0056c1e0,00679310,006dacf0,005f6180,00565310,00686c80,006dab80,0066b800,00580330,00681eb0,00682040,00699e80,005648d0,0065e670,005d75b0,005716a0,007c3f50,0066e7e0,0062af70,00686ee0,0056c310,0057b4c0,00647b80,005b43c0,0065b4f0,00781be0,005e5ea0,0063e7e0,006010b0,0066a4e0,00682b00,0068f7e0,00590de0,0069c200,005f6060,00552d90,00687180,0076d9e0,005d56b0,005b5360,0069bb60,0065be70,00682550,005c3f70,00681890,00605090,00590a50,00580c50,0052d130,0089e000,006e3a40,00571030,0068d7d0,00670d10,0056efd0,005e1150,005680d0,0068b4b0,0054a1e0,007cf4e0,00686ac0,0067bd70,006c7a30,0066b8a0,005b5ad0,0068af80,00521110,0052bf30,00685140,00692240,006900f0,006da4f0,0069a300,0069a0c0,0059ec70,00895260,005b3380,0051d270,0062ba40,00696930,0058e990,0067f840,008cbde0,00699f60,005bdb60,005b2c10,006837c0,005e0fa0,00696890,00573950,0061d620,00632830,006d7390,0067b230,005959e0,005f4ed0,0068fb20,00896d10,0079d840,005a7e70,00682470,00896fb0,00681d70,006821a0,00619d00,0069bd50,00544210,00585f10,0079c5b0,005b8000,00689810,005a8860,00545760,0080efb0,0067e390,00699490,00680830,0051d1c0,005ceed0,007cd0f0,0054cfb0,00670760,005be1f0,007f2540,00688360,0064b280,00682e10,006949f0,005c3460,007c7d60,00544640,0069a1f0,0066ad10,006cac90,006823e0,006e4630,0058fb40,0054d180,006831a0,0057be40,00895cc0,0057fa40,006995e0,005dda60,00681f40,00568540,005912b0,006c49d0,00606e20,0069c020,006d9500,0069c3c0,0068ab60,006341d0,0066b640,00580900,007c41f0,005db8f0,00694470,005b96f0,0066f0a0,0057a560,0057ac30,0066b770,006898e0,0068db00,0057c990,00585cc0,00590ff0,005452c0,0067b100,006c8de0,00698e80,00669b90,0067f8c0,005da4d0,00645ce0,005bde40,00577fa0,006c9c00 callees=0053dfb0

undefined4 FUN_0051c0a0(undefined4 param_1)

{
  undefined4 local_8;
  
  local_8 = 0;
  FUN_0053dfb0/*CGameObjectArray::GetGameObject*/(param_1,&local_8);
  return local_8;
}

```

## R282 FUNCTION Gob::GetBoundingBox  steam=0x00459fb0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector* min, Vector* max) param_bytes=8
Steam function at this address: size=68 ret_N=8
```c
// FN 00459fb0 FUN_00459fb0 size=68 ret=8 callers= callees=

void __thiscall FUN_00459fb0(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  *param_2 = *(undefined4 *)(param_1 + 0x14c);
  param_2[1] = *(undefined4 *)(param_1 + 0x150);
  param_2[2] = *(undefined4 *)(param_1 + 0x154);
  *param_3 = *(undefined4 *)(param_1 + 0x158);
  param_3[1] = *(undefined4 *)(param_1 + 0x15c);
  param_3[2] = *(undefined4 *)(param_1 + 0x160);
  return;
}

```

## R283 FUNCTION Gob::SetObjectScale  steam=0x00463340
GOG build: param_bytes=5 ghidra_sig='undefined4 param_1, undefined1 param_2'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, float scale, bool inherit) param_bytes=8
Steam function at this address: size=217 ret_N=8
```c
// FN 00463340 FUN_00463340 size=217 ret=8 callers= callees=

undefined4 __thiscall FUN_00463340(int param_1,float param_2,char param_3)

{
  undefined4 uVar1;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x84) == 0) {
    uVar1 = 0;
  }
  else {
    if (param_3 == '\x01') {
      *(float *)(param_1 + 0x1e0) = *(float *)(param_1 + 0x1e0) / *(float *)(param_1 + 0x1e4);
      *(float *)(param_1 + 0x1e4) = param_2;
      *(float *)(param_1 + 0x1e0) = *(float *)(param_1 + 0x1e0) * *(float *)(param_1 + 0x1e4);
    }
    else {
      *(float *)(param_1 + 0x1e0) = param_2 * *(float *)(param_1 + 0x1e4);
    }
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x1ec); local_8 = local_8 + 1) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0x1e8) + local_8 * 4) + 0xcc))
                (*(undefined4 *)(param_1 + 0x1e0),0);
    }
    uVar1 = 1;
  }
  return uVar1;
}

```

## R284 FUNCTION Gob::GetMotionBlurred  steam=0x00459990
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): unsigned char (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=20 ret_N=0
```c
// FN 00459990 FUN_00459990 size=20 ret=0 callers= callees=

undefined1 __fastcall FUN_00459990(int param_1)

{
  return *(undefined1 *)(param_1 + 0x181);
}

```

## R285 FUNCTION CRes::Release  steam=0x007112b0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=26 ret_N=0
```c
// FN 007112b0 FUN_007112b0 size=26 ret=0 callers=0093f8b0,00525c60,005236b0,0070ecb0,0055a650,0093ffb0,0085ee00,0071d0a0,0071c3d0,00638bd0,005e9ee0,0093f340,0055a460,008bf280,0083db30,00630810,00717630,005e97a0,0055db00,00944ee0,0093f970,00942330,005582f0,0055d080,0040f5a0,00556d20,00765df0,00941520,00630f20,00701fd0,00523870 callees=007130b0

void __fastcall FUN_007112b0(undefined4 param_1)

{
  FUN_007130b0(param_1);
  return;
}

```

## R286 FUNCTION CSWCDoor::UpdateAreaTransitionDisplay  steam=0x0087c9f0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=3313 ret_N=0
```c
// FN 0087c9f0 FUN_0087c9f0 size=3313 ret=0 callers=0087b240 callees=00473e80,0073f750,007aae00,0044c8a0,0073f450,0087b7e0,0052d0e0,004452b0,00465560,0046ce40,005160c0,0073f490,005143c0,005188c0,007ceaf0,007ce740,00473e50

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __fastcall FUN_0087c9f0(int *param_1)

{
  char cVar1;
  int *piVar2;
  float *pfVar3;
  int iVar4;
  undefined4 uVar5;
  undefined1 *puVar6;
  undefined1 local_1f0 [12];
  undefined1 local_1e4 [12];
  undefined1 local_1d8 [12];
  float local_1cc;
  float local_1c8;
  float local_1c4;
  float local_1c0;
  float local_1bc;
  float local_1b8;
  float local_1b4;
  float local_1b0;
  float local_1ac;
  float local_1a8;
  float local_1a4;
  float local_1a0;
  float local_19c;
  float local_198;
  float local_194;
  undefined1 local_190 [16];
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178;
  float local_174;
  float local_170;
  float local_16c;
  undefined1 local_168 [12];
  undefined1 local_15c [12];
  undefined1 local_150 [16];
  float local_140;
  float local_13c;
  float local_138;
  float local_134;
  float local_130;
  float local_12c;
  float local_128;
  float local_124;
  float local_120;
  float local_11c;
  float local_118;
  float local_114;
  float local_110;
  float local_10c;
  float local_108;
  float local_104;
  undefined4 local_100 [27];
  int *local_94;
  int local_90;
  int local_8c;
  int local_88;
  int *local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  float local_70;
  float local_6c;
... (222 more lines)
```

## R287 FUNCTION CRes::Constructor  steam=0x007110d0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=151 ret_N=0
```c
// FN 007110d0 FUN_007110d0 size=151 ret=0 callers=006f3db0,00703b40,00941f10,00716e60,00945ce0,009461c0,0055db00,006f3cf0,008f8430,00729ed0,0072a0a0,00941e70,00716fd0,00943ec0,00941900,009452c0,00941a90,00941fb0,007228f0,00942050 callees=

undefined4 * __fastcall FUN_007110d0(undefined4 *param_1)

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

## R288 FUNCTION CSWGuiPanel::GetExtentAccountingForPanelOffset  steam=0x00410020
GOG build: param_bytes=4 ghidra_sig='CSWGuiPanel * this, CSWGuiExtent * outExtent'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, CSWGuiExtent* outExtent) param_bytes=4
Steam function at this address: size=156 ret_N=4
```c
// FN 00410020 FUN_00410020 size=156 ret=4 callers=0040ec90,0040eae0,0040fed0,00886cf0,00886c10 callees=

void __thiscall FUN_00410020(int param_1,int *param_2)

{
  *param_2 = *(int *)(param_1 + 4);
  param_2[1] = *(int *)(param_1 + 8);
  param_2[2] = *(int *)(param_1 + 0xc);
  param_2[3] = *(int *)(param_1 + 0x10);
  if ((*(uint *)(param_1 + 0x48) >> 3 & 1) == 0) {
    if ((*(uint *)(param_1 + 0x48) >> 4 & 1) != 0) {
      *param_2 = (*(short *)(*(int *)(param_1 + 0x1c) + 0x6c) + -0x280) / 2 + *param_2;
    }
    if ((*(uint *)(param_1 + 0x48) >> 4 & 2) != 0) {
      param_2[1] = (*(short *)(*(int *)(param_1 + 0x1c) + 0x6e) + -0x1e0) / 2 + param_2[1];
    }
  }
  return;
}

```

## R289 FUNCTION Gob::SetRenderPersonalSpace  steam=0x00459950
GOG build: param_bytes=1 ghidra_sig='undefined1 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, bool render) param_bytes=4
Steam function at this address: size=25 ret_N=4
```c
// FN 00459950 FUN_00459950 size=25 ret=4 callers= callees=

void __thiscall FUN_00459950(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x180) = param_2;
  return;
}

```

## R290 OFFSET CSWGuiBorderParams.corner_image_resref = 0x24
Claim: SetCornerImage 0x414680 reads params+0x24 (FUN_00710a70/00710ca0 on this+0x24); operator= copies resref from src+0x24
Decompile 0x00414680:
```c
// FN 00414680 FUN_00414680/*CSWGuiBorderParams::SetCornerImage*/ size=219 ret=8 callers=00415080,00414920,00747210 callees=00710ca0,00710a70,00710a10,0047eb60,00710980,009196ee

void __thiscall FUN_00414680/*CSWGuiBorderParams::SetCornerImage*/(int param_1,undefined4 param_2,int param_3)

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

## R291 FUNCTION CResGFF::AddListElement  steam=0x00719800
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, CResList* list, DWORD structId) param_bytes=12
Steam function at this address: size=306 ret_N=12
```c
// FN 00719800 FUN_00719800 size=306 ret=12 callers=0052ab70,005f7b80,00540410,0052a600,00664d80,00664e40,005e2700,005f18c0,0051ee20,00574500,0052ad90,0052a840,0062b5f0,00655010,00603390,0052afc0,0052aeb0,0052a730,0052aa60,00629eb0,00540900,005fb1a0,00603740,0055ddf0,005e5b40,005e9510,005e1e60,006fed80,0055bd10,0052ac80,005f8710,0052a950,00592310,0055d080,0052b380,005e72d0,00621ba0,0076aed0,00540860,006b4ea0,005302e0,0058d2f0,006272c0,0069cdb0 callees=00717aa0,00717f30,00717770,007178e0,00718580,00717880

undefined4 __thiscall FUN_00719800(int param_1,int *param_2,int param_3,undefined4 param_4)

{
  undefined4 uVar1;
  int iVar2;
  uint local_18;
  int *local_14;
  int local_10;
  int *local_c;
  int *local_8;
  
  if ((param_3 == 0) || (param_2 == (int *)0x0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_3,param_3 + 4);
    local_8 = (int *)FUN_00717770(param_3,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 0xf)) {
      uVar1 = 0;
    }
    else {
      local_14 = (int *)FUN_00717880/*CResGFF::GetDataLayoutList*/(local_8,&local_18);
      if ((local_14 == (int *)0x0) || (local_18 < 4)) {
        uVar1 = 0;
      }
      else {
        local_10 = *local_14;
        if (local_18 < local_10 * 4 + 4U) {
          uVar1 = 0;
        }
        else {
          local_c = (int *)FUN_00718580/*CResGFF::AddDataLayoutList*/(local_8[2],local_10 * 4 + 4,local_10 * 4 + 8);
          if (local_c == (int *)0x0) {
            uVar1 = 0;
          }
          else {
            local_8[2] = (int)local_c - *(int *)(param_1 + 0x70);
            iVar2 = FUN_00717f30/*CResGFF::AddStruct*/(param_4);
            *param_2 = iVar2;
            if (*param_2 == -1) {
              uVar1 = 0;
            }
            else {
              *local_c = local_10 + 1;
              local_c[local_10 + 1] = *param_2;
              uVar1 = 1;
            }
          }
        }
      }
    }
  }
  return uVar1;
}

```

## R292 OFFSET Scene.objects = 0x54
Claim: GetObjectA 0x465290 scans [this+0x54] with count [this+0x58], _stricmp(name, obj+0x64): CExoArrayList<Gob*>
Decompile 0x00465290:
```c
// FN 00465290 FUN_00465290/*Scene::GetObjectA*/ size=102 ret=4 callers= callees=009370c7

undefined4 __thiscall FUN_00465290/*Scene::GetObjectA*/(int param_1,char *param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x58) <= local_8) {
      return 0;
    }
    iVar1 = __stricmp(param_2,(char *)(*(int *)(*(int *)(param_1 + 0x54) + local_8 * 4) + 100));
    if (iVar1 == 0) break;
    local_8 = local_8 + 1;
  }
  return *(undefined4 *)(*(int *)(param_1 + 0x54) + local_8 * 4);
}

```

## R293 FUNCTION Scene::SetBlindLights  steam=0x00465210
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* object) param_bytes=4
Steam function at this address: size=30 ret_N=4
```c
// FN 00465210 FUN_00465210 size=30 ret=4 callers= callees=

void FUN_00465210(undefined4 param_1)

{
  DAT_00a7be44 = param_1;
  return;
}

```

## R294 FUNCTION CSWCObject::AnimationOverlay  steam=0x007ef6a0
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=131 ret_N=4
```c
// FN 007ef6a0 FUN_007ef6a0 size=131 ret=4 callers=00777e30 callees=0071b8b0

bool __thiscall FUN_007ef6a0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe20,&local_8);
  return iVar1 != 0 && local_8 != 0;
}

```

## R295 FUNCTION CClientExoApp::GetTargetChanging  steam=0x00740e50
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=39 ret_N=4
```c
// FN 00740e50 FUN_00740e50 size=39 ret=4 callers=006cc140 callees=

bool __thiscall FUN_00740e50(int param_1,byte param_2)

{
  return (*(byte *)(*(int *)(param_1 + 4) + 0x4a0) & param_2) != 0;
}

```

## R296 FUNCTION CSWCObject::AnimationHideEquippedItems  steam=0x007ef910
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=129 ret_N=4
```c
// FN 007ef910 FUN_007ef910 size=129 ret=4 callers=007eeee0,00777e30,007ed9a0 callees=0071b8b0

bool __thiscall FUN_007ef910(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe50,&local_8);
  return iVar1 != 0 && local_8 != 0;
}

```
