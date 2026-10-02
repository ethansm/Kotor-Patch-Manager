## R324 FUNCTION Gob::EnableForceAnimations  steam=0x00459d60
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=18 ret_N=0
```c
// FN 00459d60 FUN_00459d60 size=18 ret=0 callers= callees=

void __fastcall FUN_00459d60(int param_1)

{
  *(undefined1 *)(param_1 + 0x10) = 1;
  return;
}

```

## R325 FUNCTION Gob::SetColorShifting  steam=0x0045c750
GOG build: param_bytes=20 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector color, float alpha, int propagate) param_bytes=12
Steam function at this address: size=95 ret_N=20
```c
// FN 0045c750 FUN_0045c750 size=95 ret=20 callers= callees=

void __thiscall
FUN_0045c750(int *param_1,int param_2,int param_3,int param_4,int param_5,int param_6)

{
  *(undefined2 *)(param_1 + 0x3c) = 1;
  param_1[0x3e] = param_2;
  param_1[0x3f] = param_3;
  param_1[0x40] = param_4;
  param_1[0x41] = param_5;
  if (param_6 != 0) {
    (**(code **)(*param_1 + 0x1b4))("propagatecolorshift",0);
  }
  return;
}

```

## R326 FUNCTION CSWSObject::StopSoundPlayingInDialog  steam=0x00546a70
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=46 ret_N=0
```c
// FN 00546a70 FUN_00546a70 size=46 ret=0 callers=006c49d0,00545520 callees=007402b0

void __fastcall FUN_00546a70(int param_1)

{
  *(undefined4 *)(param_1 + 0x25c) = 1;
  FUN_007402b0(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  return;
}

```

## R327 FUNCTION CGuiInGame::IsFadingInDialog  steam=0x007cb720
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=44 ret_N=0
```c
// FN 007cb720 FUN_007cb720 size=44 ret=0 callers=00546ad0 callees=008bb200

undefined4 __fastcall FUN_007cb720(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 0x68) != 0) && (iVar1 = FUN_008bb200(), iVar1 == 0)) {
    return 1;
  }
  return 0;
}

```

## R328 FUNCTION CSWCObject::AttachToScene  steam=0x007ed7f0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* scene) param_bytes=4
Steam function at this address: size=61 ret_N=4
```c
// FN 007ed7f0 FUN_007ed7f0 size=61 ret=4 callers=0077d840 callees=00623320

void __thiscall FUN_007ed7f0(int param_1,int param_2)

{
  if (param_2 == 0) {
    FUN_00623320();
  }
  if (*(int *)(param_1 + 0x68) != 0) {
    (**(code **)(**(int **)(param_1 + 0x68) + 4))(param_2);
  }
  return;
}

```

## R329 FUNCTION CSWSObject::SetAnimation  steam=0x00541390
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int animation) param_bytes=4
Steam function at this address: size=35 ret_N=4
```c
// FN 00541390 FUN_00541390 size=35 ret=4 callers=0061dcf0 callees=0054a430

void __thiscall FUN_00541390(int param_1,undefined4 param_2)

{
  FUN_0054a430(0);
  *(undefined4 *)(param_1 + 0xd8) = param_2;
  return;
}

```

## R330 FUNCTION CRes::Request  steam=0x007112d0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=26 ret_N=0
```c
// FN 007112d0 FUN_007112d0 size=26 ret=0 callers=0071d360,009450d0,0070a8d0,00941190,00940550,00940d90,0093fc20,00720970,0093f530,00941710,00940990,00943cd0,0085f5f0,0093d2b0,0093d7c0,00703950,0052e640,00561240 callees=00713310

void __fastcall FUN_007112d0(undefined4 param_1)

{
  FUN_00713310(param_1);
  return;
}

```

## R331 FUNCTION CSWGuiBorder::FillTile  steam=0x00415d20
GOG build: param_bytes=24 ghidra_sig='CSWGuiBorder * this, int height, int width, int x, int y, float alpha, Vector * color'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int height, int width, int x, int y, float alpha, Vector* color) param_bytes=24
Steam function at this address: size=384 ret_N=24
```c
// FN 00415d20 FUN_00415d20 size=384 ret=24 callers=00415300 callees=

void __thiscall
FUN_00415d20(int param_1,int param_2,int param_3,int param_4,int param_5,undefined4 param_6,
            undefined4 param_7)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = (**(code **)(**(int **)(param_1 + 0x74) + 0x10))();
  iVar3 = (**(code **)(**(int **)(param_1 + 0x74) + 0xc))();
  uVar1 = *(undefined4 *)(param_1 + 0x20);
  if ((iVar2 < param_3) && (iVar3 < param_2)) {
    (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
              ((param_3 - iVar2) / 2 + param_4,(param_2 - iVar3) / 2 + param_5,iVar2,iVar3,0,uVar1,
               param_7,param_6);
  }
  else if (iVar2 < param_3) {
    (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
              ((param_3 - iVar2) / 2 + param_4,param_5,iVar2,param_2,0,uVar1,param_7,param_6);
  }
  else if (iVar3 < param_2) {
    (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
              (param_4,(param_2 - iVar3) / 2 + param_5,param_3,iVar3,0,uVar1,param_7,param_6);
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x74) + 0x18))
              (param_4,param_5,param_3,param_2,0,uVar1,param_7,param_6);
  }
  return;
}

```

## R332 FUNCTION CSWSObject::GetDead  steam=0x005477f0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=202 ret_N=0
```c
// FN 005477f0 FUN_005477f0 size=202 ret=0 callers=00619e30,00563460 callees=

bool __fastcall FUN_005477f0(int *param_1)

{
  short sVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if ((iVar2 != 0) && (iVar2 = (**(code **)(*param_1 + 0x30))(), *(int *)(iVar2 + 0x10e8) != 0)) {
    piVar3 = (int *)(**(code **)(*param_1 + 0x30))();
    sVar1 = (**(code **)(*piVar3 + 0x9c))(0);
    return sVar1 < -9;
  }
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 == 0) {
    sVar1 = (**(code **)(*param_1 + 0x9c))(0);
  }
  else {
    piVar3 = (int *)(**(code **)(*param_1 + 0x30))();
    sVar1 = (**(code **)(*piVar3 + 0x9c))(0);
  }
  return sVar1 < 1;
}

```

## R333 FUNCTION CGuiInGame::ResetDialogAnimations  steam=0x007c93e0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=373 ret_N=0
```c
// FN 007c93e0 FUN_007c93e0 size=373 ret=0 callers=007c6a20 callees=00842d00,0073f4d0,007cf4e0,007ed7d0

void __fastcall FUN_007c93e0(int param_1)

{
  short sVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  uint local_8;
  
  for (local_8 = 0; local_8 < *(uint *)(param_1 + 0x1bc); local_8 = local_8 + 1) {
    piVar2 = (int *)FUN_0073f4d0(*(undefined4 *)(*(int *)(param_1 + 0x1c8) + local_8 * 4));
    if (piVar2 != (int *)0x0) {
      iVar3 = (**(code **)(*piVar2 + 0x2c))();
      if (iVar3 != 0) {
        piVar4 = (int *)(**(code **)(*piVar2 + 0x2c))();
        iVar3 = FUN_007cf4e0(piVar4[1]);
        if ((iVar3 != 0) && (piVar4 != (int *)0x0)) {
          piVar4[0x15] = 0;
          sVar1 = FUN_007ed7d0/*CSWCObject::GetCurrentAnimation*/();
          if (sVar1 != 10000) {
            (**(code **)(*piVar4 + 0x9c))(10000,0,1);
          }
          (**(code **)(*(int *)piVar4[0x1a] + 0x70))(0);
        }
      }
      iVar3 = (**(code **)(*piVar2 + 0x4c))();
      if (iVar3 != 0) {
        piVar2 = (int *)(**(code **)(*piVar2 + 0x4c))();
        iVar3 = FUN_00842d00/*CSWCPlaceable::GetServerPlaceable*/();
        if (piVar2 != (int *)0x0) {
          piVar2[0x15] = 0;
          (**(code **)(*piVar2 + 0x9c))(*(uint *)(iVar3 + 0xd8) & 0xffff,0,0);
          (**(code **)(*(int *)piVar2[0x1a] + 0x70))(0);
        }
      }
    }
  }
  return;
}

```

## R334 FUNCTION CResGFF::GetField  steam=0x00717720
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): GFFFieldData* (__thiscall)(void* thisPtr, CResGFFStruct* structData, DWORD index) param_bytes=8
Steam function at this address: size=71 ret_N=4
```c
// FN 00717720 FUN_00717720 size=71 ret=4 callers=00718690,00718030,00717770,007186e0,007178e0 callees=

int __thiscall FUN_00717720(int param_1,uint *param_2)

{
  int iVar1;
  
  if ((((param_2 == (uint *)0x0) || (*(int *)(param_1 + 0x40) == 0)) ||
      (*(int *)(param_1 + 0x44) == 0)) || (*(uint *)(*(int *)(param_1 + 0x40) + 0xc) <= *param_2)) {
    iVar1 = 0;
  }
  else {
    iVar1 = *param_2 * 0xc + *(int *)(param_1 + 0x44);
  }
  return iVar1;
}

```

## R335 FUNCTION CResGFF::OnResourceFreed  steam=0x007175b0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=124 ret_N=0
```c
// FN 007175b0 FUN_007175b0 size=124 ret=0 callers= callees=

undefined4 __fastcall FUN_007175b0(int param_1)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    *(undefined4 *)(param_1 + 0x70) = 0;
    *(undefined4 *)(param_1 + 100) = 0;
    *(undefined4 *)(param_1 + 0x5c) = 0;
    *(undefined4 *)(param_1 + 0x54) = 0;
    *(undefined4 *)(param_1 + 0x4c) = 0;
    *(undefined4 *)(param_1 + 0x44) = 0;
    *(undefined4 *)(param_1 + 0x40) = 0;
    *(undefined4 *)(param_1 + 0x98) = 0;
  }
  *(undefined4 *)(param_1 + 0x94) = 0;
  return 1;
}

```

## R336 FUNCTION CSWCObject::AnimationFireAndForget  steam=0x007ef5d0
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=207 ret_N=4
```c
// FN 007ef5d0 FUN_007ef5d0 size=207 ret=4 callers=00769620,00841440,0087f410,0087b3f0,0056d350,005d9da0,007f2380,00777e30,007efc40,008e59c0,008079b0 callees=0071b8b0

bool __thiscall FUN_007ef5d0(int param_1,ushort param_2)

{
  int iVar1;
  bool bVar2;
  int local_8;
  
  if (((*(char *)(param_1 + 8) == '\x05') || (*(char *)(param_1 + 8) == '\t')) &&
     (((999 < param_2 && (param_2 < 0x468)) || ((0x4af < param_2 && (param_2 < 0x530)))))) {
    bVar2 = true;
  }
  else {
    local_8 = 0;
    if (*(int *)(param_1 + 0x68) != 0) {
      param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
    }
    iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe1c,&local_8);
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

## R337 FUNCTION CSWSPlaceable::GetEffectSpellId  steam=0x005ef120
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=49 ret_N=4
```c
// FN 005ef120 FUN_005ef120 size=49 ret=4 callers= callees=

undefined4 * __thiscall FUN_005ef120(int param_1,undefined4 *param_2)

{
  *param_2 = *(undefined4 *)(param_1 + 0x280);
  param_2[1] = *(undefined4 *)(param_1 + 0x284);
  param_2[2] = *(undefined4 *)(param_1 + 0x288);
  param_2[3] = *(undefined4 *)(param_1 + 0x28c);
  return param_2;
}

```

## R338 FUNCTION CGuiInGame::HideUpgradeScreen  steam=0x007cdbe0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=217 ret_N=0
```c
// FN 007cdbe0 FUN_007cdbe0 size=217 ret=0 callers=008c7330,008c7300 callees=0073f810,0087a730,0073fee0,0070bc60

void __fastcall FUN_007cdbe0(int param_1)

{
  int iVar1;
  
  if (((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x38) != 0)) &&
     (*(int *)(param_1 + 0x88) != 0)) {
    if (*(int *)(param_1 + 0xf18) == 0) {
      FUN_0073f810();
      FUN_0087a730();
    }
    FUN_0073fee0/*CClientExoApp::SetInputClass*/(0,1);
    FUN_0070bc60(DAT_0099c2a8,0);
    iVar1 = *(int *)(param_1 + 0x88);
    *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xffffff7f;
    *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xfffffcff | 0x200;
    *(undefined4 *)(param_1 + 0x88) = 0;
  }
  return;
}

```

## R339 FUNCTION CClientExoApp::SetGameOver  steam=0x007403c0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int gameOver) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 007403c0 FUN_007403c0 size=28 ret=4 callers=00760aa0,007608a0,00760920 callees=0079c230

void FUN_007403c0(undefined4 param_1)

{
  FUN_0079c230(param_1);
  return;
}

```

## R340 FUNCTION CResGFF::WriteFieldDWORD64  steam=0x00719c50
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, DWORD64 value, char* label) param_bytes=12
Steam function at this address: size=143 ret_N=16
```c
// FN 00719c50 FUN_00719c50 size=143 ret=16 callers=005e5b40,0053af20,0055bd10,00534600 callees=00717aa0,00718030,00718370

undefined4 __thiscall
FUN_00719c50(int param_1,int param_2,undefined4 param_3,undefined4 param_4,int param_5)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  
  if ((param_2 == 0) || (param_5 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_2,param_5,6);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      puVar3 = (undefined4 *)FUN_00718370/*CResGFF::AddDataField*/(0xffffffff,0,8);
      if (puVar3 == (undefined4 *)0x0) {
        uVar1 = 0;
      }
      else {
        *(int *)(iVar2 + 8) = (int)puVar3 - *(int *)(param_1 + 0x5c);
        *puVar3 = param_3;
        puVar3[1] = param_4;
        uVar1 = 1;
      }
    }
  }
  return uVar1;
}

```

## R341 FUNCTION CSWGuiPanel::OnPanelRemoved  steam=0x0040f4c0
GOG build: param_bytes=0 ghidra_sig='CSWGuiPanel * this'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=212 ret_N=0
```c
// FN 0040f4c0 FUN_0040f4c0 size=212 ret=0 callers=0089ea20,0084f8e0,008abd30,00753720,008857a0,008b3730,00858b60,008cca70,0075cc60,00894ca0,008d8490,008dd9e0,008d2f30,008b54f0,008a2590,008bd5f0,00898620 callees=00413f00

void __fastcall FUN_0040f4c0(int *param_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int local_8;
  
  FUN_00413f00();
  if ((*(int *)(DAT_00a1b49c + 0x18) != 0) &&
     (iVar1 = *(int *)(DAT_00a1b49c + 0x18), iVar3 = (**(code **)(*param_1 + 0x44))(),
     iVar1 == iVar3)) {
    (**(code **)(**(int **)(DAT_00a1b49c + 0x18) + 0x7c))(0);
  }
  piVar2 = *(int **)(param_1[7] + 8);
  if (piVar2 != (int *)0x0) {
    for (local_8 = 0; local_8 < param_1[10]; local_8 = local_8 + 1) {
      if (*(int **)(param_1[9] + local_8 * 4) == piVar2) {
        *(undefined4 *)(param_1[7] + 8) = 0;
        (**(code **)(*piVar2 + 0xc))(0);
        return;
      }
    }
  }
  return;
}

```

## R342 FUNCTION CSWGuiPanel::InitControl  steam=0x0040f620
GOG build: param_bytes=16 ghidra_sig='CSWGuiPanel * this, CSWGuiControl * controlToInit, CExoString * label, int activate, int param_4'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* controlToInit, void* label, int activate) param_bytes=12
Steam function at this address: size=970 ret_N=16
```c
// FN 0040f620 FUN_0040f620 size=970 ret=16 callers=0075b370,008850b0,00893950,008a1170,00892090,00918b10,0088aac0,009074e0,007d6170,00747210,00880150,008c19f0,008b4270,0084c3a0,008d1020,0089c5f0,007d8540,0075ae40,008bba80,007d23e0,00760210,007d9800,008c80f0,007423d0,008c9e10,00850770,007d4620,007dc410,008fb7a0,0075fda0,00916ef0,007dacb0,00745b30,00886fc0,008a92d0,009153d0,007bc600,00757c40,008e1810,008586d0,008db630,008f92b0,00910490,008b91f0,008bfb60,008f4af0,008fc8e0,008b1ea0,007450e0,008973d0,00916640,008bd910,008b82e0,00902f60,00904a80,008fff20,008bdc90,0089cf30,008df930,008c0cb0,007fae60,00754ed0,008c6650,008bc620,00914000,008a25c0,008d6b90,00881970,007d31d0,0090c6a0,0075cec0,00741940,008a6170,008bee10,00887da0,00909e00 callees=00418df0,0040fa80,0040f9f0,0040fb40,0073fea0,00734010,0083ea60,0091d6f0,0091c860

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0040f620(int param_1,int *param_2,undefined4 param_3,int param_4,int param_5)

{
  short sVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  int iVar6;
  char *pcVar7;
  float10 fVar8;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  float local_c;
  float local_8;
  
  FUN_00418df0/*CSWGuiControl::Load_2*/(param_1,*(undefined4 *)(param_1 + 0x30),param_1 + 0x34,param_3);
  local_1c = param_2[1];
  local_18 = param_2[2];
  local_14 = param_2[3];
  local_10 = param_2[4];
  local_8 = 1.0;
  local_c = 1.0;
  if (param_5 != 0) {
    iVar6 = FUN_0073fea0();
    if (iVar6 != 0) {
      iVar6 = FUN_0073fea0();
      local_8 = (float)(int)*(short *)(iVar6 + 0x6c) / (float)DAT_009f4390;
      iVar6 = FUN_0073fea0();
      local_c = (float)(int)*(short *)(iVar6 + 0x6e) / (float)DAT_009f4394;
    }
  }
  local_10 = FUN_0091c860();
  local_14 = FUN_0091c860();
  local_18 = FUN_0091c860();
  local_1c = FUN_0091c860();
  if ((DAT_00a1b9f0 != 0) && (param_5 != 0)) {
    iVar6 = FUN_0073fea0();
    if (iVar6 != 0) {
      fVar2 = (float)local_1c;
      fVar3 = (float)local_14;
      fVar4 = (float)local_18;
      fVar5 = (float)local_10;
      iVar6 = FUN_0073fea0();
      sVar1 = *(short *)(iVar6 + 0x6c);
      iVar6 = FUN_0073fea0();
      FUN_0040f9f0((float)(int)sVar1,(float)(int)*(short *)(iVar6 + 0x6e),
                   (float)_DAT_00987488 / ((float)(int)sVar1 / (float)(int)*(short *)(iVar6 + 0x6e))
                   ,0x3f800000);
      FUN_0040fa80(fVar2,fVar4);
      fVar8 = (float10)FUN_0040fb40(fVar4);
      FUN_0040fa80(fVar3 + fVar2,(float)fVar8);
      FUN_0040fb40(fVar5 + fVar4);
      local_1c = FUN_0091c860();
      local_14 = FUN_0091c860();
      local_18 = FUN_0091c860();
      local_10 = FUN_0091c860();
    }
  }
  (**(code **)(*param_2 + 4))(&local_1c);
  pcVar7 = (char *)FUN_00734010();
  pcVar7 = _strstr(pcVar7,"LBL_BAR");
  if (pcVar7 != (char *)0x0) {
    iVar6 = (**(code **)(*param_2 + 0x50))();
... (18 more lines)
```

## R343 FUNCTION CSWGuiPanel::Draw  steam=0x0040ec90
GOG build: param_bytes=4 ghidra_sig='CSWGuiPanel * this, float param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, float param1) param_bytes=4
Steam function at this address: size=432 ret_N=4
```c
// FN 0040ec90 FUN_0040ec90 size=432 ret=4 callers=008fb130,007bcab0,008859a0,008a7350,008fe2b0,00760720,0090bbb0,0084fbb0,008b8ee0,008bb260,00892870,008b98d0,008be740,00756b70,008b82a0,0088e510,00918730,0074b3f0,00894cd0,00882080,008b8ae0,00898c20,009091e0,008aba50,008cce70,007fbce0,00907360 callees=004799c0,00479460,004793f0,00410020,004792c0

void __thiscall FUN_0040ec90(int param_1,undefined4 param_2)

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

## R344 FUNCTION CSWGuiPanel::SetActiveControl  steam=0x0040ec00
GOG build: param_bytes=8 ghidra_sig='CSWGuiPanel * this, CSWGuiControl * controlToActivate, int playSound'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* controlToActivate, int playSound) param_bytes=8
Steam function at this address: size=143 ret_N=8
```c
// FN 0040ec00 FUN_0040ec00 size=143 ret=8 callers=008f92b0,00910490,008fc8e0,00880740,00916640,009074e0,007d6170,008fff20,008c19f0,008b4270,008df930,008c0cb0,008d1020,007d8540,007fae60,007d9800,008c80f0,00754ed0,008c6650,008c9e10,00850770,007d4620,007dc410,008fb7a0,008d6b90,007dacb0,007d31d0,0090c6a0,00757c40,008e1810,008a6170,00909e00 callees=004122a0

void __thiscall FUN_0040ec00(int param_1,int param_2,int param_3)

{
  if (*(int *)(param_1 + 0x20) != param_2) {
    if (*(int *)(param_1 + 0x20) != 0) {
      (**(code **)(**(int **)(param_1 + 0x20) + 0x3c))(1,1);
    }
    *(int *)(param_1 + 0x20) = param_2;
    if (((*(int *)(param_1 + 0x20) != 0) &&
        ((**(code **)(**(int **)(param_1 + 0x20) + 0x3c))(0,1), param_3 != 0)) &&
       (*(int *)(param_1 + 0x1c) != 0)) {
      FUN_004122a0/*CSWGuiManager::PlayGuiSound*/(*(undefined1 *)(param_2 + 0x59));
    }
  }
  return;
}

```

## R345 FUNCTION CSWSDoor::RemoveFromArea  steam=0x0061d000
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
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

## R346 FUNCTION CGuiInGame::GetMessageBufferSize  steam=0x007c7d40
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=20 ret_N=0
```c
// FN 007c7d40 FUN_007c7d40 size=20 ret=0 callers=005fb1a0 callees=

undefined4 __fastcall FUN_007c7d40(int param_1)

{
  return *(undefined4 *)(param_1 + 0x11c);
}

```

## R347 FUNCTION CClientExoApp::GetInputClass  steam=0x0073fee0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=32 ret_N=8
```c
// FN 0073fee0 FUN_0073fee0 size=32 ret=8 callers=00853a00,007cab00,008aed10,007cb7c0,0089ed90,008faca0,00746670,0075d950,00883560,008a2070,005d56b0,007caa60,007cd7e0,007cdaa0,007d1e90,00894e90,007cacb0,0084f900,007cdda0,008079b0,007cad30,007cb920,007cdf10,007ca920,007cd760,007d2030,007cb540,00753680,007bfb10,00796ba0,0075cc60,007ccf30,008a3660,007467c0,00751580,008925f0,007cf270,007c6a20,0075ca60,008928f0,007cab60,007caa10,007cb4a0,008a8880,0069bcf0,007605f0,007cac10,007ca730,008f82c0,007cdcc0,00545760,0089fbd0,008a07d0,0074c320,008f8070,007672f0,007cdbe0,007fbd90,007ccd40,007d1d10,007cd510,00754950 callees=007b3050

void FUN_0073fee0(undefined4 param_1,undefined4 param_2)

{
  FUN_007b3050(param_1,param_2);
  return;
}

```

## R348 FUNCTION Gob::SetIgnoreHitCheck  steam=0x00459830
GOG build: param_bytes=1 ghidra_sig='undefined1 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, bool ignore) param_bytes=4
Steam function at this address: size=25 ret_N=4
```c
// FN 00459830 FUN_00459830 size=25 ret=4 callers= callees=

void __thiscall FUN_00459830(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x1a9) = param_2;
  return;
}

```

## R349 FUNCTION CWorldTimer::ResetTimer  steam=0x0051ae30
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* reset) param_bytes=4
Steam function at this address: size=177 ret_N=4
```c
// FN 0051ae30 FUN_0051ae30 size=177 ret=4 callers=005582f0,00784740,00538ed0,00781be0 callees=

void __thiscall FUN_0051ae30(undefined4 *param_1,undefined4 *param_2)

{
  *param_1 = *param_2;
  param_1[1] = param_2[1];
  param_1[3] = param_2[3];
  param_1[4] = param_2[4];
  param_1[5] = param_2[5];
  param_1[6] = param_2[6];
  param_1[9] = param_2[9];
  param_1[10] = param_2[10];
  param_1[0xb] = param_2[0xb];
  param_1[0xc] = param_2[0xc];
  param_1[0xd] = param_2[0xd];
  *(undefined1 *)(param_1 + 0xe) = *(undefined1 *)(param_2 + 0xe);
  param_1[0xf] = param_2[0xf];
  param_1[0x10] = param_2[0x10];
  param_1[2] = 100;
  return;
}

```

## R350 FUNCTION CClientExoApp::RemoveMoveToModuleMovies  steam=0x007401a0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 007401a0 FUN_007401a0 size=22 ret=0 callers=0053af20,007986a0 callees=

int __fastcall FUN_007401a0(int param_1)

{
  return *(int *)(param_1 + 4) + 0x2d8;
}

```
