## R189 FUNCTION Scene::SetCurrentRoom_2  steam=0x0046ffb0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int index) param_bytes=4
Steam function at this address: size=72 ret_N=4
```c
// FN 0046ffb0 FUN_0046ffb0 size=72 ret=4 callers= callees=

void __thiscall FUN_0046ffb0(int param_1,int param_2)

{
  if ((param_2 < 0) || (*(int *)(param_1 + 0xd8) <= param_2)) {
    *(undefined4 *)(param_1 + 0xe0) = 0;
  }
  else {
    *(undefined4 *)(param_1 + 0xe0) = *(undefined4 *)(*(int *)(param_1 + 0xd4) + param_2 * 4);
  }
  return;
}

```

## R190 FUNCTION Scene::SetGeomFadeProperties  steam=0x004725e0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): unsigned char (__stdcall)(int property) param_bytes=4
Steam function at this address: size=45 ret_N=4
```c
// FN 004725e0 FUN_004725e0 size=45 ret=4 callers= callees=

undefined4 FUN_004725e0(int param_1)

{
  undefined4 uVar1;
  
  if (((param_1 == 1) || (param_1 == 2)) || (param_1 == 4)) {
    DAT_009f5d28 = param_1;
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

```

## R191 FUNCTION CSWGuiBorderParams::SetBorder  steam=0x00414920
GOG build: param_bytes=4 ghidra_sig='CSWGuiBorderParams * this, CSWGuiBorder * param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* border) param_bytes=4
Steam function at this address: size=82 ret_N=4
```c
// FN 00414920 FUN_00414920 size=82 ret=4 callers=004144f0,00414a50 callees=00414760,00414680,00414840

void __thiscall FUN_00414920(int param_1,undefined4 param_2)

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

## R192 FUNCTION Gob::TurnOffShadows  steam=0x00459850
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=23 ret_N=0
```c
// FN 00459850 FUN_00459850 size=23 ret=0 callers= callees=

void __fastcall FUN_00459850(int param_1)

{
  *(undefined2 *)(param_1 + 0xee) = 0;
  return;
}

```

## R193 FUNCTION CSWGuiManager::AddPanel  steam=0x00410530
GOG build: param_bytes=12 ghidra_sig='CSWGuiManager * this, CSWGuiPanel * panel, int flags, int playSound'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* panel, int flags, int playSound) param_bytes=12
Steam function at this address: size=317 ret_N=12
```c
// FN 00410530 FUN_00410530 size=317 ret=12 callers=00760050,007cbb40,00842bd0,0088d700,007cbdb0,007a1130,0084fe20,00915010,008c70b0,007d2df0,007ca060,0069bea0,00904510,00906310,007ce780,008a87f0,00912840,00908e30,007cdda0,008d4ac0,007dabf0,007cd510,00882930,007d2eb0,00888d80,00887b10,00914bf0,007d2030,00853a00,00912d40,007b12c0,00908b50,007cf270,00781590,00914da0,008d98f0,007d5b20,008e1080,007ca3c0,008a8640,009135c0,00898010,008528b0,0040dac0,008a1df0,007cfbf0,005f5770,00407540,007cacb0,008a8760,008ceb00,007caa10,00753680,007cb920,0090b6b0,0089de70,007ca6e0,007ca550,0090b890,0088d5d0,007c9df0,0084fcd0,008a8480,008a1eb0,008abe70,00904600,007cab00,0090fa70,008c73f0,008a1c50,00908cc0,007d5d50,00914e70,007cdaa0,007d2f70,008a86d0,008b61c0,0090ae20,0074d520,00916520,00760920,008c7f60,00903ba0,007be4c0,007c6530,007c9c40,0088a420,0089ed90,008b6930,007ca230,008f61d0,0090ede0,008ce300,008d4680,00410900,007ccd40,007d9280,00882230,00914f40,007813a0,008be630,008c9460,00904420,00915f60,008e0fc0,00745e60,007ca730,0089eb50,0079d070,008827a0,0069c020,007bfb10,00854440,00854810,00889c00,00886a70,007d1d10,00798190,009152f0,008898a0,008954f0,0088f890,008545c0,008519f0,008578b0,00857f20,007c6200,00796ba0,00760aa0,00903df0,008a1d20,00915e90,00882b10,00919430,008a1f70,00881dd0,007fbd90,009074b0,009046c0,0090b430,007cd7e0,00883e00,009084f0,008fa6a0,008bc230,007d0f50,008d95e0,007d2d30,00882a30,007cfb10,008c8880 callees=004122a0,00410890,00411170,00410900,0083ea60

void __thiscall FUN_00410530(int param_1,int *param_2,uint param_3,int param_4)

{
  int iVar1;
  
  if (param_2 != (int *)0x0) {
    if (param_4 != 0) {
      FUN_004122a0/*CSWGuiManager::PlayGuiSound*/((char)param_2[0x19]);
    }
    iVar1 = FUN_00410890(param_2);
    if (iVar1 == -1) {
      param_2[0x12] = param_2[0x12] & 0xfffffff7U | (uint)((param_3 & 2) != 0) << 3;
      if (((uint)param_2[0x12] >> 3 & 1) != 0) {
        *(char *)(param_1 + 0x70) = *(char *)(param_1 + 0x70) + '\x01';
      }
      FUN_0083ea60/*CExoArrayList<uint32>::Add*/(param_2);
      if ((param_3 & 1) != 0) {
        FUN_00410900(param_2);
      }
      (**(code **)(*param_2 + 0x48))();
      FUN_00411170();
    }
    else if ((((uint)param_2[0x12] >> 8 & 3) == 2) || (((uint)param_2[0x12] >> 8 & 3) != 0)) {
      param_2[0x12] = param_2[0x12] & 0xffffff7f;
      param_2[0x12] = param_2[0x12] & 0xfffffcff;
      if ((param_3 & 1) != 0) {
        FUN_00410900(param_2);
      }
      (**(code **)(*param_2 + 0x48))();
      FUN_00411170();
    }
  }
  return;
}

```

## R194 FUNCTION Scene::ChildRemove  steam=0x00465c20
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* child) param_bytes=4
Steam function at this address: size=112 ret_N=4
```c
// FN 00465c20 FUN_00465c20 size=112 ret=4 callers= callees=00449170,00495fb0,00465ae0

void __thiscall FUN_00465c20(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  if (param_2[0x21] != 0) {
    FUN_00495fb0(param_1,param_2);
    FUN_00465ae0(param_2[0x26]);
  }
  FUN_00449170(param_2);
  iVar1 = (**(code **)(*param_2 + 0x1a8))();
  if (iVar1 != 0) {
    FUN_00449170(param_2);
  }
  return;
}

```

## R195 OFFSET CSWGuiPanel.active_control = 0x20
Claim: SetActiveControl 0x40ec00 compares/stores this+0x20 and calls control vt+0x3c(1,1)/(0,1); HandleInputEvent reads it
Decompile 0x0040ec00:
```c
// FN 0040ec00 FUN_0040ec00/*CSWGuiPanel::SetActiveControl*/ size=143 ret=8 callers=008f92b0,00910490,008fc8e0,00880740,00916640,009074e0,007d6170,008fff20,008c19f0,008b4270,008df930,008c0cb0,008d1020,007d8540,007fae60,007d9800,008c80f0,00754ed0,008c6650,008c9e10,00850770,007d4620,007dc410,008fb7a0,008d6b90,007dacb0,007d31d0,0090c6a0,00757c40,008e1810,008a6170,00909e00 callees=004122a0

void __thiscall FUN_0040ec00/*CSWGuiPanel::SetActiveControl*/(int param_1,int param_2,int param_3)

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

## R196 FUNCTION CServerExoApp::GetWorldTimer  steam=0x0051c390
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 0051c390 FUN_0051c390 size=31 ret=0 callers=005582f0 callees=

undefined4 __fastcall FUN_0051c390(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x1004c);
}

```

## R197 FUNCTION CWorldTimer::GetWorldTimeMillisecond  steam=0x0051b1c0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=49 ret_N=0
```c
// FN 0051b1c0 FUN_0051b1c0 size=49 ret=0 callers=00689720 callees=0051ab40,0051b0e0

undefined4 __fastcall FUN_0051b1c0(undefined4 param_1)

{
  undefined4 uVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  undefined1 *puVar4;
  undefined4 *puVar5;
  undefined1 local_14 [4];
  undefined1 local_10 [4];
  undefined1 local_c [4];
  undefined4 local_8;
  
  puVar5 = &local_8;
  puVar4 = local_10;
  puVar3 = local_c;
  puVar2 = local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40/*CWorldTimer::ConvertFromTimeOfDay*/(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_8;
}

```

## R198 FUNCTION Gob::DisableForceAnimations  steam=0x00459da0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=17 ret_N=0
```c
// FN 00459da0 FUN_00459da0 size=17 ret=0 callers= callees=

undefined1 __fastcall FUN_00459da0(int param_1)

{
  return *(undefined1 *)(param_1 + 0x10);
}

```

## R199 FUNCTION CResGFF::GetDataLayoutList  steam=0x00717880
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResGFFField* field, DWORD* dataLength) param_bytes=8
Steam function at this address: size=81 ret_N=8
```c
// FN 00717880 FUN_00717880 size=81 ret=8 callers=00718890,00719800,00718810,00718760 callees=

int __thiscall FUN_00717880(int param_1,int param_2,int *param_3)

{
  int iVar1;
  
  if (((param_2 == 0) || (*(int *)(param_1 + 0x70) == 0)) ||
     (*(uint *)(*(int *)(param_1 + 0x40) + 0x34) <= *(uint *)(param_2 + 8))) {
    iVar1 = 0;
  }
  else {
    *param_3 = *(int *)(*(int *)(param_1 + 0x40) + 0x34) - *(int *)(param_2 + 8);
    iVar1 = *(int *)(param_1 + 0x70) + *(int *)(param_2 + 8);
  }
  return iVar1;
}

```

## R200 FUNCTION Gob::SetAsCharacter  steam=0x004598f0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=54 ret_N=0
```c
// FN 004598f0 FUN_004598f0 size=54 ret=0 callers= callees=

void __fastcall FUN_004598f0(int param_1)

{
  if (*(int *)(param_1 + 0x84) != 0) {
    *(ushort *)(*(int *)(param_1 + 0x84) + 0x50) = *(ushort *)(*(int *)(param_1 + 0x84) + 0x50) | 4;
  }
  return;
}

```

## R201 FUNCTION Gob::Append  steam=0x00462020
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, char* modelName, char* partName) param_bytes=8
Steam function at this address: size=335 ret_N=8
```c
// FN 00462020 FUN_00462020 size=335 ret=8 callers= callees=0048e160,0045fde0,00461ef0,00452d40,00456cd0,00461ed0,00456b00,00473f40

undefined4 __thiscall FUN_00462020(int *param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  
  iVar1 = FUN_00456b00(param_1[0x26],param_3);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    iVar3 = FUN_0048e160(param_2);
    if (iVar3 == 0) {
      uVar2 = 0;
    }
    else {
      FUN_00452d40(param_1);
      iVar4 = FUN_00456cd0(*(undefined4 *)(iVar3 + 0x28),param_1);
      *(undefined4 *)(iVar4 + 8) = 0;
      *(undefined4 *)(iVar4 + 0xc) = 0;
      *(undefined4 *)(iVar4 + 0x10) = 0;
      *(undefined4 *)(iVar4 + 0x14) = 0x3f800000;
      *(undefined4 *)(iVar4 + 0x18) = 0;
      *(undefined4 *)(iVar4 + 0x1c) = 0;
      *(undefined4 *)(iVar4 + 0x20) = 0;
      FUN_00473f40(iVar3);
      FUN_00473f40(iVar4);
      FUN_00473f40(iVar4);
      *(int *)(iVar4 + 0x3c) = iVar1;
      FUN_0045fde0(param_1,iVar4,&DAT_00986925);
      FUN_00461ed0();
      FUN_00461ef0(param_1,iVar4);
      (**(code **)(*param_1 + 0x1b4))("_EmitterTarget",0);
      uVar2 = 1;
    }
  }
  return uVar2;
}

```

## R202 OFFSET CSWGuiManager.viewport_height = 0x6e
Claim: SetSize 0x411a80 stores shorts at this+0x6c (width) and +0x6e (height); CSWGuiPanel::Draw 0x40ec90 compares panel extent against *(short*)(mgr+0x6e)
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
Decompile 0x00411a80:
```c
// FN 00411a80 FUN_00411a80 size=145 ret=8 callers=0078dbf0,008925f0,00892090,00788d90 callees=00411170,00413dd0

void __thiscall FUN_00411a80(int param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  
  if ((*(short *)(param_1 + 0x6c) != param_2) || (*(short *)(param_1 + 0x6e) != param_3)) {
    *(undefined2 *)(param_1 + 0x6c) = (undefined2)param_2;
    *(undefined2 *)(param_1 + 0x6e) = (undefined2)param_3;
    FUN_00413dd0();
    if (*(int *)(param_1 + 0x7c) != 0) {
      if (*(undefined4 **)(param_1 + 0x7c) == (undefined4 *)0x0) {
        uVar1 = 0;
      }
      else {
        uVar1 = (**(code **)**(undefined4 **)(param_1 + 0x7c))(1);
      }
      *(undefined4 *)(param_1 + 0x7c) = 0;
      FUN_00411170(uVar1);
    }
  }
  return;
}

```

## R203 FUNCTION CSWCPlaceable::GetServerPlaceable  steam=0x00842d00
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=53 ret_N=0
```c
// FN 00842d00 FUN_00842d00 size=53 ret=0 callers=007b40f0,0077df90,007aaec0,007ac470,0077e0c0,0079d840,007c93e0,007c9850,00842d40,0077e370,00841180,00742a00,008427a0,00841c00,007c84e0,007eef90,00843ab0,00843b80,0080d050,007f1c00 callees=007f2540

undefined4 FUN_00842d00(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
  iVar1 = FUN_007f2540/*CSWCObject::GetServerObject*/();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_007f2540/*CSWCObject::GetServerObject*/();
    uVar3 = (**(code **)(*piVar2 + 0x48))(piVar2,uVar3);
  }
  return uVar3;
}

```

## R204 FUNCTION CClientExoApp::AddMovieToMovieQueue  steam=0x007411c0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* movie, int skippable) param_bytes=8
Steam function at this address: size=106 ret_N=8
```c
// FN 007411c0 FUN_007411c0 size=106 ret=8 callers=0069a3f0 callees=009370c7,00734010,00401650,007410e0,007a0f10

void __thiscall FUN_007411c0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  char *pcVar2;
  int iVar3;
  char *pcVar4;
  
  uVar1 = FUN_00734010(param_1);
  FUN_00401650(uVar1);
  pcVar4 = "malmov12";
  pcVar2 = (char *)FUN_00734010();
  iVar3 = __stricmp(pcVar2,pcVar4);
  if (iVar3 != 0) {
    pcVar4 = "malmov14";
    pcVar2 = (char *)FUN_00734010();
    iVar3 = __stricmp(pcVar2,pcVar4);
    if (iVar3 != 0) goto LAB_00741211;
  }
  FUN_007410e0();
LAB_00741211:
  FUN_007a0f10(param_2,param_3);
  return;
}

```

## R205 FUNCTION CResGFF::WriteFieldCExoString  steam=0x00719e50
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, CExoString* value, char* label) param_bytes=12
Steam function at this address: size=222 ret_N=12
```c
// FN 00719e50 FUN_00719e50 size=222 ret=12 callers=00602dd0,005859f0,0053c680,005f7b80,00703370,00540410,00664d80,005e2700,005f18c0,0062b5f0,00655010,00629eb0,00540900,005fb1a0,005e5b40,0062dec0,005e1e60,006fed80,0061bfe0,0053af20,0055bd10,00534600,0055d080,00629040,005e72d0,00621ba0,0076aed0,0058d2f0,006b3d10,006272c0,0055d7f0,0069cdb0 callees=00717aa0,00734010,0091dba0,00919f10,00718030,00718370

undefined4 __thiscall FUN_00719e50(int param_1,int param_2,int *param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  size_t *psVar3;
  void *_Src;
  size_t local_14;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,10);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      if (*param_3 == 0) {
        local_14 = 0;
      }
      else {
        local_14 = _strlen((char *)*param_3);
      }
      psVar3 = (size_t *)FUN_00718370/*CResGFF::AddDataField*/(0xffffffff,0,local_14 + 4);
      if (psVar3 == (size_t *)0x0) {
        uVar1 = 0;
      }
      else {
        *(int *)(iVar2 + 8) = (int)psVar3 - *(int *)(param_1 + 0x5c);
        *psVar3 = local_14;
        _Src = (void *)FUN_00734010();
        _memcpy(psVar3 + 1,_Src,local_14);
        uVar1 = 1;
      }
    }
  }
  return uVar1;
}

```

## R206 FUNCTION Gob::DisableInCutscene  steam=0x00459e00
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=18 ret_N=0
```c
// FN 00459e00 FUN_00459e00 size=18 ret=0 callers= callees=

void __fastcall FUN_00459e00(int param_1)

{
  *(undefined1 *)(param_1 + 0x14) = 0;
  return;
}

```

## R207 FUNCTION Gob::EnableAlwaysRender  steam=0x00459dc0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=18 ret_N=0
```c
// FN 00459dc0 FUN_00459dc0 size=18 ret=0 callers= callees=

void __fastcall FUN_00459dc0(int param_1)

{
  *(undefined1 *)(param_1 + 0x12) = 1;
  return;
}

```

## R208 FUNCTION CSWSTrigger::OnEnterMine  steam=0x00622930
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int skipFactionCheck) param_bytes=4
Steam function at this address: size=660 ret_N=4
```c
// FN 00622930 FUN_00622930 size=660 ret=4 callers=0061f170 callees=0051c280,00605e80,006d0620,006fd8d0,0051e170,00584da0,007339d0,0051c0e0,00622750,0051c100,00622640,0058a2c0,006d0880,00605cb0

void __thiscall FUN_00622930(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 *puVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  int local_14;
  int local_10;
  
  iVar1 = FUN_0051c100/*CServerExoApp::GetCreatureByGameObjectID*/(*(undefined4 *)(param_1 + 0x2dc));
  if ((iVar1 != 0) && (iVar1 = FUN_00622750/*CSWSTrigger::GetCanFireMineOnObject*/(*(undefined4 *)(iVar1 + 4),param_2), iVar1 != 0)) {
    iVar1 = param_1 + 700;
    FUN_0058a2c0(1);
    local_10 = FUN_00605cb0(iVar1);
    if (local_10 == 0x7f000000) {
      for (local_14 = 0; local_14 < 0x14; local_14 = local_14 + 1) {
        iVar1 = FUN_006d0620(1 << ((byte)local_14 & 0x1f));
        if ((iVar1 != 0) && (iVar2 = FUN_007339d0(param_1 + 700), iVar2 != 0)) {
          local_10 = *(int *)(iVar1 + 0x14);
        }
      }
    }
    puVar3 = (undefined4 *)FUN_0051c0e0(local_10);
    if (puVar3 == (undefined4 *)0x0) {
      FUN_00584da0(0x52,0);
      uVar9 = 1;
      uVar7 = *(undefined4 *)(param_1 + 4);
      uVar4 = FUN_00622640(4);
      FUN_006fd8d0(uVar4,uVar7,uVar9);
      uVar9 = 1;
      uVar7 = *(undefined4 *)(param_1 + 4);
      uVar4 = FUN_00622640(1);
      FUN_006fd8d0(uVar4,uVar7,uVar9);
      if (*(int *)(param_1 + 0x310) != 0) {
        uVar7 = *(undefined4 *)(param_1 + 4);
        uVar4 = *(undefined4 *)(param_1 + 4);
        uVar10 = 0;
        uVar6 = 0;
        uVar5 = 0;
        uVar9 = DAT_00992420;
        FUN_0051c280(0,0,uVar4,uVar7,DAT_00992420,0);
        FUN_0051e170/*CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)*/(uVar5,uVar6,uVar4,uVar7,uVar9,uVar10);
      }
    }
    else {
      if (*(int *)(param_1 + 0x30c) != 0) {
        puVar8 = puVar3;
        FUN_0058a2c0(1);
        FUN_00605e80/*CItemRepository::RemoveItem*/(puVar8);
        FUN_006d0880(puVar3);
        if (puVar3 != (undefined4 *)0x0) {
          (**(code **)*puVar3)(1);
        }
      }
      *(undefined4 *)(param_1 + 0x2e4) = *(undefined4 *)(param_1 + 0x2dc);
      uVar9 = 1;
      uVar7 = *(undefined4 *)(param_1 + 4);
      uVar4 = FUN_00622640(5);
      FUN_006fd8d0(uVar4,uVar7,uVar9);
      uVar7 = *(undefined4 *)(param_1 + 4);
      uVar4 = *(undefined4 *)(param_1 + 4);
... (11 more lines)
```

## R209 FUNCTION CClientExoApp::CreatureAcquireItem  steam=0x0073f9e0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__stdcall)(DWORD creatureId, void* item) param_bytes=8
Steam function at this address: size=68 ret_N=8
```c
// FN 0073f9e0 FUN_0073f9e0 size=68 ret=8 callers=007abd60,008bfaa0,007abcb0,007abc00,007abb20,007abec0,007abe10 callees=0051c8b0,0051cc60

void FUN_0073f9e0(undefined4 param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0/*CServerExoApp::ClientToServerObjectId*/(param_1);
  FUN_0051cc60(uVar1,param_2);
  return;
}

```

## R210 FUNCTION CResGFF::GetListElement  steam=0x00718890
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, CResList* list, DWORD index) param_bytes=12
Steam function at this address: size=197 ret_N=12
```c
// FN 00718890 FUN_00718890 size=197 ret=12 callers=00625360,005e2900,00525f90,006fef80,0053fd50,00418df0,00638bd0,00528710,005753f0,00664cb0,005269e0,0069d1e0,00528080,006b3b70,006c67e0,005266f0,00630810,005e9370,006c53f0,005e2090,005f7cc0,006815e0,00527cb0,0058d5e0,00526f60,00655690,006295d0,00664800,0051eed0,00527330,005263c0,006029f0,00626850,00833ff0,0052ccd0,00573950,006d0db0,005e5510,0062af70,0083db30,00527580,005f9220,005409f0,00527890,00621900,00655b00,005582f0,0055d080,006afed0,00601740,0055def0,00592840,00637ce0,005fbf60,005e6f80,00630f20,00626a50,005406e0,00523870,005ef570 callees=00717770,007178e0,00717880

undefined4 __thiscall FUN_00718890(int param_1,uint *param_2,int param_3,uint param_4)

{
  undefined4 uVar1;
  uint *puVar2;
  uint local_18 [3];
  int *local_c;
  
  if ((param_2 == (uint *)0x0) || (param_3 == 0)) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_3,param_3 + 4);
    local_c = (int *)FUN_00717770(param_3,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 0xf)) {
      uVar1 = 0;
    }
    else {
      puVar2 = (uint *)FUN_00717880/*CResGFF::GetDataLayoutList*/(local_c,local_18);
      if ((puVar2 == (uint *)0x0) || (local_18[0] < param_4 * 4 + 8)) {
        uVar1 = 0;
      }
      else if ((param_4 < *puVar2) &&
              (puVar2[param_4 + 1] < *(uint *)(*(int *)(param_1 + 0x40) + 0xc))) {
        *param_2 = puVar2[param_4 + 1];
        uVar1 = 1;
      }
      else {
        uVar1 = 0;
      }
    }
  }
  return uVar1;
}

```

## R211 FUNCTION Gob::GetPartLocalPosition  steam=0x00462cd0
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, Vector* outPosition, Quaternion* outOrientation) param_bytes=8
Steam function at this address: size=121 ret_N=12
```c
// FN 00462cd0 FUN_00462cd0 size=121 ret=12 callers= callees=

undefined4 __thiscall
FUN_00462cd0(int *param_1,undefined4 param_2,undefined4 *param_3,undefined4 *param_4)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = (**(code **)(*param_1 + 0x10c))(param_2);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    if (param_3 != (undefined4 *)0x0) {
      *param_3 = *(undefined4 *)(iVar1 + 8);
      param_3[1] = *(undefined4 *)(iVar1 + 0xc);
      param_3[2] = *(undefined4 *)(iVar1 + 0x10);
    }
    if (param_4 != (undefined4 *)0x0) {
      *param_4 = *(undefined4 *)(iVar1 + 0x14);
      param_4[1] = *(undefined4 *)(iVar1 + 0x18);
      param_4[2] = *(undefined4 *)(iVar1 + 0x1c);
      param_4[3] = *(undefined4 *)(iVar1 + 0x20);
    }
    uVar2 = 1;
  }
  return uVar2;
}

```

## R212 FUNCTION CClientExoApp::SendSaveGameRequest  steam=0x0073f6f0
GOG build: param_bytes=12 ghidra_sig='CClientExoApp * this, uint * param_1, uint * param_2, uint * param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD slot, void* saveName, void* module) param_bytes=12
Steam function at this address: size=36 ret_N=12
```c
// FN 0073f6f0 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=007a06b0,008545c0,00754950 callees=00785620

/* Library Function - Single Match
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00785620(param_1,param_2,param_3);
  return;
}

```

## R213 FUNCTION CResGFF::OnResourceServiced  steam=0x00717300
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=680 ret_N=0
```c
// FN 00717300 FUN_00717300 size=680 ret=0 callers= callees=009196fd

undefined4 __fastcall FUN_00717300(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x10) == 0) {
    uVar1 = 0;
  }
  else {
    if ((*(int *)(param_1 + 0x40) != 0) && (*(int *)(param_1 + 0x98) == 0)) {
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x70));
      *(undefined4 *)(param_1 + 0x70) = 0;
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 100));
      *(undefined4 *)(param_1 + 100) = 0;
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x5c));
      *(undefined4 *)(param_1 + 0x5c) = 0;
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x54));
      *(undefined4 *)(param_1 + 0x54) = 0;
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x4c));
      *(undefined4 *)(param_1 + 0x4c) = 0;
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x44));
      *(undefined4 *)(param_1 + 0x44) = 0;
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x40));
      *(undefined4 *)(param_1 + 0x40) = 0;
    }
    *(undefined4 *)(param_1 + 0x40) = *(undefined4 *)(param_1 + 0x10);
    if (**(int **)(param_1 + 0x40) == *(int *)(param_1 + 0x8d)) {
      if (*(int *)(*(int *)(param_1 + 0x40) + 4) == *(int *)PTR_DAT_00a100c8) {
        if (*(int *)(*(int *)(param_1 + 0x40) + 0xc) != 0) {
          *(int *)(param_1 + 0x44) =
               *(int *)(param_1 + 0x10) + *(int *)(*(int *)(param_1 + 0x40) + 8);
        }
        if (*(int *)(*(int *)(param_1 + 0x40) + 0x14) != 0) {
          *(int *)(param_1 + 0x4c) =
               *(int *)(param_1 + 0x10) + *(int *)(*(int *)(param_1 + 0x40) + 0x10);
        }
        if (*(int *)(*(int *)(param_1 + 0x40) + 0x1c) != 0) {
          *(int *)(param_1 + 0x54) =
               *(int *)(param_1 + 0x10) + *(int *)(*(int *)(param_1 + 0x40) + 0x18);
        }
        if (*(int *)(*(int *)(param_1 + 0x40) + 0x24) != 0) {
          *(int *)(param_1 + 0x5c) =
               *(int *)(param_1 + 0x10) + *(int *)(*(int *)(param_1 + 0x40) + 0x20);
        }
        if (*(int *)(*(int *)(param_1 + 0x40) + 0x2c) != 0) {
          *(int *)(param_1 + 100) =
               *(int *)(param_1 + 0x10) + *(int *)(*(int *)(param_1 + 0x40) + 0x28);
        }
        if (*(int *)(*(int *)(param_1 + 0x40) + 0x34) != 0) {
          *(int *)(param_1 + 0x70) =
               *(int *)(param_1 + 0x10) + *(int *)(*(int *)(param_1 + 0x40) + 0x30);
        }
        *(undefined4 *)(param_1 + 0x98) = 1;
        *(undefined4 *)(param_1 + 0x94) = 1;
        uVar1 = 1;
      }
      else {
        *(undefined4 *)(param_1 + 0x40) = 0;
        uVar1 = 0;
      }
    }
    else {
      *(undefined4 *)(param_1 + 0x40) = 0;
      uVar1 = 0;
    }
  }
  return uVar1;
}
... (1 more lines)
```

## R214 FUNCTION CSWGuiControl::GetIsChild  steam=0x00419350
GOG build: param_bytes=4 ghidra_sig='CSWGuiControl * this, CSWGuiControl * child'
Wrapper typedef (what calling code expects): bool (__thiscall)(void* thisPtr, void* child) param_bytes=4
Steam function at this address: size=72 ret_N=4
```c
// FN 00419350 FUN_00419350 size=72 ret=4 callers=00412c00 callees=

undefined4 __thiscall FUN_00419350(int param_1,int param_2)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x20) <= local_8) {
      return 0;
    }
    if (*(int *)(*(int *)(param_1 + 0x1c) + local_8 * 4) == param_2) break;
    local_8 = local_8 + 1;
  }
  return 1;
}

```

## R215 FUNCTION CSWGuiPanel::CenterPanel  steam=0x0040ea00
GOG build: param_bytes=0 ghidra_sig='CSWGuiPanel * this'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=149 ret_N=0
```c
// FN 0040ea00 FUN_0040ea00 size=149 ret=0 callers=0089c5f0,0075ae40,0075b370,008bfb60,007d5d50,007cfca0,008b1ea0,008c19f0,008c0cb0,0075c080,008bee10,008586d0 callees=

void __fastcall FUN_0040ea00(int param_1)

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
