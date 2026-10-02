## R081 FUNCTION Scene::EnableAnimations  steam=0x004651b0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=21 ret_N=0
```c
// FN 004651b0 FUN_004651b0 size=21 ret=0 callers= callees=

void __fastcall FUN_004651b0(int param_1)

{
  *(undefined1 *)(param_1 + 0xe6) = 1;
  return;
}

```

## R082 OFFSET CSWGuiPanel.alpha = 0x50
Claim: Draw 0x40ec90 passes *(this+0x50) as the alpha argument of clip push 0x479460; ctor 0x40e5a0 sets it to 1.0
Decompile 0x0040e5a0:
```c
// FN 0040e5a0 FUN_0040e5a0/*CSWGuiPanel::Constructor*/ size=423 ret=4 callers=0075b370,008850b0,00880740,00893950,008a1170,00892090,00918b10,0088aac0,009074e0,007d6170,00747210,00880150,008c19f0,008b4270,0074a500,0084c3a0,008d1020,008b9ce0,0089c5f0,007d8540,0075ae40,007d23e0,00760210,007d9800,008c80f0,008c9e10,00850770,007d4620,007dc410,008fb7a0,0075fda0,00916ef0,007dacb0,00886fc0,008a92d0,008ba980,009153d0,007bc600,00757c40,008e1810,008586d0,008db630,008f92b0,00910490,008b91f0,008bfb60,008f4af0,008fc8e0,008b1ea0,008973d0,00916640,008bb5d0,008b82e0,00902f60,00904a80,008fff20,008bdc90,0089cf30,008df930,008c0cb0,007fae60,00754ed0,008c6650,0087fd40,00914000,008a25c0,008d6b90,00411b20,007d31d0,0090c6a0,0075cec0,008a6170,008bee10,00887da0,00909e00 callees=00919723,00414a50,0084a640

undefined4 * __thiscall FUN_0040e5a0/*CSWGuiPanel::Constructor*/(undefined4 *param_1,undefined4 param_2)

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
... (5 more lines)
```
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

## R083 FUNCTION CSWSPlaceable::SetEffectSpellId  steam=0x005ef160
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD spellId) param_bytes=4
Steam function at this address: size=20 ret_N=0
```c
// FN 005ef160 FUN_005ef160 size=20 ret=0 callers= callees=

undefined4 __fastcall FUN_005ef160(int param_1)

{
  return *(undefined4 *)(param_1 + 0x3e0);
}

```

## R084 FUNCTION CSWGuiBorderParams::operator=  steam=0x004144f0
GOG build: param_bytes=4 ghidra_sig='CSWGuiBorderParams * this, CSWGuiBorderParams * param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, void* rhs) param_bytes=4
Steam function at this address: size=390 ret_N=4
```c
// FN 004144f0 FUN_004144f0 size=390 ret=4 callers=008b0fb0,008d0470,008b38e0,008a5ba0,00414c20 callees=00710ca0,00414920,00414980

undefined4 * __thiscall FUN_004144f0(undefined4 *param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 local_44;
  undefined1 local_34 [16];
  undefined1 local_24 [16];
  undefined1 local_14 [16];
  
  FUN_00710ca0(local_14,param_2 + 0x24);
  FUN_00710ca0(local_24,param_2 + 0x34);
  FUN_00710ca0(local_34,param_2 + 0x44);
  param_1[8] = param_1[8] & 0xfffffffc | *(uint *)(param_2 + 0x20) & 3;
  param_1[3] = *(undefined4 *)(param_2 + 0xc);
  uVar1 = FUN_00414980();
  *param_1 = uVar1;
  param_1[4] = *(undefined4 *)(param_2 + 0x10);
  param_1[8] = param_1[8] & 0xfffffffb | (*(uint *)(param_2 + 0x20) >> 2 & 1) << 2;
  param_1[5] = *(undefined4 *)(param_2 + 0x14);
  param_1[6] = *(undefined4 *)(param_2 + 0x18);
  param_1[7] = *(undefined4 *)(param_2 + 0x1c);
  param_1[8] = param_1[8] & 0xfffffff7 | (*(uint *)(param_2 + 0x20) >> 3 & 1) << 3;
  param_1[8] = param_1[8] & 0xffffffef | (*(uint *)(param_2 + 0x20) >> 4 & 1) << 4;
  param_1[1] = *(undefined4 *)(param_2 + 4);
  if (*(int *)(param_2 + 8) == 0) {
    local_44 = *(undefined4 *)(param_2 + 4);
  }
  else {
    local_44 = *(undefined4 *)(param_2 + 8);
  }
  param_1[2] = local_44;
  FUN_00414920/*CSWGuiBorderParams::SetBorder*/(param_1[0x15]);
  return param_1;
}

```

## R085 FUNCTION CResGFF::WriteFieldVector  steam=0x0071a320
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, Vector* value, char* label) param_bytes=12
Steam function at this address: size=172 ret_N=12
```c
// FN 0071a320 FUN_0071a320 size=172 ret=12 callers=0052b380,00561c90 callees=00717aa0,00718030,00718370

undefined4 __thiscall FUN_0071a320(int param_1,int param_2,undefined4 *param_3,int param_4)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 *puVar5;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar3 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    iVar4 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,0x11);
    if (iVar4 == 0) {
      uVar3 = 0;
    }
    else {
      puVar5 = (undefined4 *)FUN_00718370/*CResGFF::AddDataField*/(0xffffffff,0,0xc);
      if (puVar5 == (undefined4 *)0x0) {
        uVar3 = 0;
      }
      else {
        uVar3 = *param_3;
        uVar1 = param_3[1];
        uVar2 = param_3[2];
        *(int *)(iVar4 + 8) = (int)puVar5 - *(int *)(param_1 + 0x5c);
        *puVar5 = uVar3;
        puVar5[1] = uVar1;
        puVar5[2] = uVar2;
        uVar3 = 1;
      }
    }
  }
  return uVar3;
}

```

## R086 FUNCTION CResGFF::WriteFieldINT  steam=0x00719bf0
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, int value, char* label) param_bytes=12
Steam function at this address: size=96 ret_N=12
```c
// FN 00719bf0 FUN_00719bf0 size=96 ret=12 callers=0058c960,00602dd0,005859f0,0053e4c0,005f7b80,00703370,00540410,005e2700,005f18c0,00520190,00574500,005e9010,0062b5f0,00540900,0052b270,005fb1a0,005e5b40,0062dec0,005e1e60,006fed80,0061bfe0,0055bd10,00592310,0052b380,005e72d0,00621ba0,0076aed0,006b4ea0,0058d2f0,006b3d10,006272c0,0053c890 callees=00717aa0,00718030

undefined4 __thiscall FUN_00719bf0(int param_1,int param_2,undefined4 param_3,int param_4)

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
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,5);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(undefined4 *)(iVar2 + 8) = param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}

```

## R087 FUNCTION Gob::GetMinimumBoundingBox_2  steam=0x00460040
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector* min, Vector* max) param_bytes=8
Steam function at this address: size=85 ret_N=8
```c
// FN 00460040 FUN_00460040 size=85 ret=8 callers= callees=

void __thiscall FUN_00460040(int *param_1,int *param_2,int *param_3)

{
  (**(code **)(*param_1 + 0x1cc))();
  *param_2 = param_1[0x53];
  param_2[1] = param_1[0x54];
  param_2[2] = param_1[0x55];
  *param_3 = param_1[0x56];
  param_3[1] = param_1[0x57];
  param_3[2] = param_1[0x58];
  return;
}

```

## R088 FUNCTION CSWGuiBorder::Load  steam=0x00415080
GOG build: param_bytes=12 ghidra_sig='CSWGuiBorder * this, CResGFF * param_1, CResStruct * param_2, CExoString * param_3'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* gff, void* item, void* label) param_bytes=12
Steam function at this address: size=639 ret_N=12
```c
// FN 00415080 FUN_00415080 size=639 ret=12 callers= callees=00414760,00734010,00718c80,00719520,00718960,00414680,00710840,00718a00,004149f0,00718e40,00414840

void __thiscall FUN_00415080(int param_1,undefined4 param_2,undefined4 param_3)

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
    local_d = FUN_00718a00/*CResGFF::ReadFieldBYTE*/(local_8,"PULSING",local_c,0);
    FUN_004149f0(local_d != 0,(local_d & 1) != 0,(local_d & 2) != 0);
  }
  return;
}

```

## R089 FUNCTION CGuiInGame::HideLoadModuleDebugMenu  steam=0x007cb4a0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=149 ret_N=0
```c
// FN 007cb4a0 FUN_007cb4a0 size=149 ret=0 callers=008c1930,008c18e0 callees=00410670,0073fee0,00410800,007400a0,004109c0

void __fastcall FUN_007cb4a0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x70));
    if (iVar1 != 0) {
      FUN_004109c0/*CSWGuiManager::PopModalPanel*/();
      FUN_00410670(*(undefined4 *)(param_1 + 0x70));
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

## R090 FUNCTION CClientExoApp::GetGameObject  steam=0x0073f4d0
GOG build: param_bytes=4 ghidra_sig='CClientExoApp * this, ulong param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, DWORD clientId) param_bytes=4
Steam function at this address: size=59 ret_N=4
```c
// FN 0073f4d0 FUN_0073f4d0 size=59 ret=4 callers=007f3770,007ccb00,008ee300,007cf5d0,006cff80,00877dc0,007b40f0,00802f00,00752760,007ac470,008eb7b0,0079c5b0,008e91b0,005d75b0,007cd0f0,007c9850,008ed7c0,008c5740,007cc5c0,0082adc0,00877e10,007f4cb0,00806960,00829850,00753040,007c8390,008c2960,007f51c0,008c4b50,0068db00,008ee8c0,00772020,008ea1f0,006c9c00,008ec380,008ed3a0,0076f070,007be2d0,007631d0,007d0c70,008ed620,0083a5b0,0080db70,007682b0,0066b8a0,00828b30,00877e60,0074fb20,006d0170,00670ae0,008e94f0,00773bf0,007cadb0,007f6dd0,008be740,008ed140,007cf780,00751750,007c6a20,006d0050,0079bc40,007c41f0,007c93e0,00842d40,008e9350,0079f430,008ebaf0,007aa460,0054a840,0082d870,008e98a0,006cfe50,007cc9a0,00691360,007b3ca0,007cb5e0,008b88b0,008eced0,0074cfe0,008eb950,007f67f0,0077dba0,0080efb0,007b12c0,007cbf60,006c8de0,007f4610,007445d0,008bdf50,007c9560,008e3e60,008e4340,007c84e0,007dea10,0078e5a0,00753300 callees=0053dfb0

undefined4 FUN_0073f4d0(undefined4 param_1)

{
  undefined4 local_8;
  
  local_8 = 0;
  FUN_0053dfb0/*CGameObjectArray::GetGameObject*/(param_1,&local_8);
  return local_8;
}

```

## R091 FUNCTION CResGFF::WriteFieldCResRef  steam=0x00719d40
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, CResRef* value, char* label) param_bytes=12
Steam function at this address: size=269 ret_N=12
```c
// FN 00719d40 FUN_00719d40 size=269 ret=12 callers=00621ba0,0062b5f0,0076aed0,0061bfe0,0053af20,0055bd10,006b3d10,00534600,006272c0,00629eb0,005f18c0,00574500 callees=00717aa0,00710a30,00718030,00718370,00458940

undefined4 __thiscall FUN_00719d40(int param_1,int param_2,undefined4 param_3,int param_4)

{
  byte bVar1;
  byte bVar2;
  undefined4 uVar3;
  int iVar4;
  byte *pbVar5;
  uint local_14;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar3 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    iVar4 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,0xb);
    if (iVar4 == 0) {
      uVar3 = 0;
    }
    else {
      bVar2 = FUN_00710a30();
      pbVar5 = (byte *)FUN_00718370/*CResGFF::AddDataField*/(0xffffffff,0,bVar2 + 1);
      if (pbVar5 == (byte *)0x0) {
        uVar3 = 0;
      }
      else {
        *(int *)(iVar4 + 8) = (int)pbVar5 - *(int *)(param_1 + 0x5c);
        *pbVar5 = bVar2;
        iVar4 = FUN_00458940();
        for (local_14 = 0; local_14 < bVar2; local_14 = local_14 + 1) {
          bVar1 = *(byte *)(iVar4 + local_14);
          if ((bVar1 < 0x41) || (0x5a < bVar1)) {
            pbVar5[local_14 + 1] = bVar1;
          }
          else {
            pbVar5[local_14 + 1] = bVar1 + 0x20;
          }
        }
        uVar3 = 1;
      }
    }
  }
  return uVar3;
}

```

## R092 FUNCTION CSWSPlaceable::SetEffectSpellId  steam=0x005ef180
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD spellId) param_bytes=4
Steam function at this address: size=25 ret_N=4
```c
// FN 005ef180 FUN_005ef180 size=25 ret=4 callers=00873360 callees=

void __thiscall FUN_005ef180(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x3e0) = param_2;
  return;
}

```

## R093 FUNCTION CResGFF::GetField_2  steam=0x00717a00
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): CResGFFField* (__thiscall)(void* thisPtr, CResStruct* structPtr, DWORD fieldIndex) param_bytes=8
Steam function at this address: size=156 ret_N=8
```c
// FN 00717a00 FUN_00717a00 size=156 ret=8 callers=007178e0 callees=

int __thiscall FUN_00717a00(int param_1,int param_2,uint param_3)

{
  int iVar1;
  
  if (((param_2 != 0) && (*(int *)(param_1 + 0x4c) != 0)) && (param_3 < *(uint *)(param_2 + 8))) {
    if (*(int *)(param_2 + 8) == 1) {
      if (*(uint *)(param_2 + 4) < *(uint *)(*(int *)(param_1 + 0x40) + 0x14)) {
        return *(int *)(param_2 + 4) * 0xc + *(int *)(param_1 + 0x4c);
      }
    }
    else {
      iVar1 = *(int *)(param_2 + 4) + param_3 * 4;
      if ((*(int *)(param_1 + 100) != 0) &&
         (iVar1 + 4U <= *(uint *)(*(int *)(param_1 + 0x40) + 0x2c))) {
        return *(int *)(*(int *)(param_1 + 100) + iVar1) * 0xc + *(int *)(param_1 + 0x4c);
      }
    }
  }
  return 0;
}

```

## R094 FUNCTION CSWGuiBorder::Load  steam=0x00414d80
GOG build: param_bytes=12 ghidra_sig='CSWGuiBorder * this, CResGFF * param_1, CResStruct * param_2, CExoString * param_3'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* gff, void* item, void* label) param_bytes=12
Steam function at this address: size=758 ret_N=4
```c
// FN 00414d80 FUN_00414d80 size=758 ret=4 callers=004197d0,008d0110,00902990,00419dc0,00421290,0041a8b0,008b0c30,0041d430,0041c150,00419ed0,0041a670,0041b440,00420c00,004199a0,0041b9e0,00421100,008a58e0,0041d710,00419c80,0041cb50,00419a80 callees=00414980

int * __thiscall FUN_00414d80(int param_1,int *param_2)

{
  int iVar1;
  int local_54;
  int local_50;
  int local_4c;
  int local_48;
  int local_44;
  int local_40;
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_18;
  int local_8;
  
  iVar1 = FUN_00414980();
  if (iVar1 == 0) {
    if ((((*(int *)(param_1 + 0x6c) == 0) || (*(int *)(param_1 + 0x70) == 0)) ||
        (*(int *)(param_1 + 0x10) == 0)) || (*(int *)(param_1 + 0xc) == 0)) {
      local_20 = 0;
      local_24 = 0;
      local_18 = 0;
    }
    else {
      local_24 = (**(code **)(**(int **)(param_1 + 0x6c) + 0xc))();
      local_18 = (**(code **)(**(int **)(param_1 + 0x6c) + 0x10))();
      (**(code **)(**(int **)(param_1 + 0x70) + 0xc))();
      local_20 = (**(code **)(**(int **)(param_1 + 0x70) + 0x10))();
    }
  }
  else {
    local_24 = FUN_00414980();
    local_20 = local_24;
    local_18 = local_24;
  }
  iVar1 = *(int *)(param_1 + 0xc) + local_18 * -2;
  if (local_20 == 0) {
    local_40 = 0;
  }
  else {
    local_40 = iVar1 / local_20;
  }
  if ((iVar1 < 1) || ((local_40 == 0 && (iVar1 < local_20 / 2)))) {
    local_34 = *(int *)(param_1 + 0xc) / 2 + *(int *)(param_1 + 4);
    local_2c = 0;
  }
  else {
    local_34 = *(int *)(param_1 + 4) + local_18;
    if (*(int *)(param_1 + 0xc) + local_18 * -2 < 0) {
      local_44 = 0;
    }
    else {
      local_44 = *(int *)(param_1 + 0xc) + local_18 * -2;
    }
    local_2c = local_44;
  }
  iVar1 = *(int *)(param_1 + 0x10) + local_24 * -2;
  if (local_20 == 0) {
    local_48 = 0;
  }
  else {
    local_48 = iVar1 / local_20;
  }
  if ((iVar1 < 1) || ((local_48 == 0 && (iVar1 < local_20 / 2)))) {
... (36 more lines)
```

## R095 FUNCTION Gob::GetMaxBlurLength  steam=0x004599d0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): double (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=20 ret_N=0
```c
// FN 004599d0 FUN_004599d0 size=20 ret=0 callers= callees=

float10 __fastcall FUN_004599d0(int param_1)

{
  return (float10)*(float *)(param_1 + 0x1ac);
}

```

## R096 FUNCTION CServerExoApp::GetGlobalVariableTable  steam=0x0051c890
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 0051c890 FUN_0051c890 size=22 ret=0 callers=0067dcb0,0077a280,008495c0,0056ae40,0058bce0,0067db70,008faca0,006305e0,006ad400,00779850,00691750,00691610,00691890,00617980,008529e0,00844860,0058a9f0,008b9bb0,005f4c00,00883560,0074da40,008398f0,007410e0,0067da50,0075fc30,0068d410,007b12c0,006a4e00,0077a030,0066cb50,006b6bd0,007b5f50,007fefb0,00533830,005953e0 callees=

int __fastcall FUN_0051c890(int param_1)

{
  return *(int *)(param_1 + 4) + 0x100fc;
}

```

## R097 FUNCTION CClientExoApp::GetInputClass  steam=0x0073ff00
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 0073ff00 FUN_0073ff00 size=31 ret=0 callers=008e3e60,008f4af0,007ac470,007dea10,007c6a20,0075ca60,008928f0,00746670,004131e0,00419420 callees=

undefined4 __fastcall FUN_0073ff00(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x9c);
}

```

## R098 FUNCTION CClientExoApp::DisableInput  steam=0x007402d0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 007402d0 FUN_007402d0 size=22 ret=0 callers=00853a00,00530170,00883560 callees=007b3370

void __fastcall FUN_007402d0(undefined4 param_1)

{
  FUN_007b3370(param_1);
  return;
}

```

## R099 FUNCTION CServerExoApp::GetActiveTimer  steam=0x0051c370
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, int objectId) param_bytes=4
Steam function at this address: size=31 ret_N=0
```c
// FN 0051c370 FUN_0051c370 size=31 ret=0 callers=0059fd20,006896d0,005d4830,0066c660,00556670,0056c310,007986a0,00781be0,005582f0,005d4700,00561f30,0079c3f0,00623330,00545520,00530170,005d18c0,00689770,00624b70,006070a0,00569320,0053af20,006c9900,00883560,00853a00,005d56b0,005d9da0,005a06f0,005d17f0,005a4d70,00692a00,006d9500,00533830,005c3f70,00576dc0,00662060,0059fcb0,00589ee0,005bfc00,0056b8a0,00545470,005d4370,005b96f0,005d4460,005d29f0,00557260,00521360,006897c0,00695140,005d4610,0062c360,00617980,0066c160,00544210,005a4bb0,00600f20,005f4c00,005b8000,00521e60,0055f520,005d9aa0,0061e360,0066c430,0055bd10,0053e830,00689720,007801a0,0062bfc0,00622c90,006c9c00,00798820 callees=

undefined4 __fastcall FUN_0051c370(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10048);
}

```

## R100 OFFSET CResGFF.field_type = 0x8d
Claim: CResGFF ctor 0x716fd0 _strncpy(this+0x8d, GFFtype, 4) (4-char GFF file type)
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

## R101 FUNCTION CClientExoApp::SetInputClass  steam=0x0073fec0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int inputClass, int setGuiStatus) param_bytes=8
Steam function at this address: size=28 ret_N=4
```c
// FN 0073fec0 FUN_0073fec0 size=28 ret=4 callers=005959e0,00637350,00575df0,007b40f0,00597750,008ab060,005d29f0,0079c5b0,005648d0,005d75b0,005d56b0,0056e620,00698070,007cc5c0,0069b840,00565310,005ca000,00691fd0,00690000,0068db00,006ca9f0,00698100,005c2930,005c87e0,008aaea0,005c3f70,006cac90,006bedf0,006c9c00,006cc140,00575600,00576dc0,006ce710,0083a5b0,005f8f90,008b2f70,007e9cc0,00756b70,006da4f0,005b8000,00569320,00798d70,00586940,00670ae0,0066dd30,0069b440,006d7390,00541080,007985d0,006d0050,0052db90,006e0650,00842d40,0089e000,00586b70,0054a840,005c6340,005f8d80,006cfe50,006000e0,008b29e0,007cc9a0,0089fbd0,00691360,006cae20,00696250,005f9220,0056efd0,007c6670,0062aac0,007cbf60,006c8de0,00566650,008bdf50,007b3050,0066c990,0056d350,005bcdc0,0079c4e0,005b96f0,005c7ad0,005c8c20,0057f450,005d7480,005ef1d0,0080d050,00669c80,006cd5b0 callees=00798160

void FUN_0073fec0(undefined4 param_1)

{
  FUN_00798160(param_1);
  return;
}

```

## R102 FUNCTION Gob::DisableRenderGobBBox  steam=0x0045bf10
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=23 ret_N=0
```c
// FN 0045bf10 FUN_0045bf10 size=23 ret=0 callers= callees=

void __fastcall FUN_0045bf10(int param_1)

{
  *(undefined2 *)(param_1 + 0xf4) = 0;
  return;
}

```

## R103 FUNCTION CClientExoApp::GetPlayerCreature  steam=0x0073f450
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 0073f450 FUN_0073f450 size=22 ret=0 callers=008074e0,00770e60,0084fcd0,00802f00,008789f0,00804f10,008767a0,007ac470,0079f010,00765aa0,0080fe80,007f3ea0,0085b940,00754db0,00751fe0,00829850,007f4220,008079b0,00774900,0074ee90,008025d0,00873f70,00876550,00815d00,007f95e0,007a6760,00753780,008b9120,007cec60,007f61b0,007aa7f0,00878050,0086ea60,008b0520,007637e0,00875d50,007aa460,008b8ee0,007f1170,00879460,0085a9e0,007f5b20,0080efb0,0074c320,007b12c0,00879310,008b9040,007a2fb0,0087f530,008775e0,007aa7a0,007cd510,007f7a30,0087c9f0,00804ad0,00754950,0082cc00 callees=0078cee0

void __fastcall FUN_0073f450(undefined4 param_1)

{
  FUN_0078cee0(param_1);
  return;
}

```

## R104 FUNCTION Gob::DisableInViewVolume  steam=0x004598b0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=18 ret_N=0
```c
// FN 004598b0 FUN_004598b0 size=18 ret=0 callers= callees=

void __fastcall FUN_004598b0(int param_1)

{
  *(undefined1 *)(param_1 + 0x11) = 0;
  return;
}

```

## R105 FUNCTION CClientExoApp::SetPlayerCreature  steam=0x0073f930
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD creatureId) param_bytes=4
Steam function at this address: size=40 ret_N=16
```c
// FN 0073f930 Concurrency::details::SchedulerProxy::DoHillClimbing size=40 ret=16 callers= callees=00781840

/* Library Function - Single Match
    public: unsigned int __thiscall Concurrency::details::SchedulerProxy::DoHillClimbing(unsigned
   int,unsigned int,unsigned int,unsigned int)
   
   Libraries: Visual Studio 2010 Debug, Visual Studio 2012 Debug, Visual Studio 2015 Debug, Visual
   Studio 2015 Release */

uint __thiscall
Concurrency::details::SchedulerProxy::DoHillClimbing
          (SchedulerProxy *this,uint param_1,uint param_2,uint param_3,uint param_4)

{
  uint uVar1;
  
  uVar1 = FUN_00781840(param_1,param_2,param_3,param_4);
  return uVar1;
}

```

## R106 FUNCTION CClientExoApp::DisplayMainMenu  steam=0x0073f230
GOG build: param_bytes=4 ghidra_sig='CClientExoApp * this, int param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=28 ret_N=4
```c
// FN 0073f230 FUN_0073f230 size=28 ret=4 callers=00407920,00851770,008fa610 callees=00781590

void FUN_0073f230(undefined4 param_1)

{
  FUN_00781590(param_1);
  return;
}

```

## R107 FUNCTION CWorldTimer::GetWorldTimeMinute  steam=0x0051b140
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=49 ret_N=0
```c
// FN 0051b140 FUN_0051b140 size=49 ret=0 callers=00689770 callees=0051ab40,0051b0e0

undefined4 __fastcall FUN_0051b140(undefined4 param_1)

{
  undefined4 uVar1;
  undefined1 *puVar2;
  undefined4 *puVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  undefined1 local_14 [4];
  undefined1 local_10 [4];
  undefined4 local_c;
  undefined1 local_8 [4];
  
  puVar5 = local_8;
  puVar4 = local_10;
  puVar3 = &local_c;
  puVar2 = local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40/*CWorldTimer::ConvertFromTimeOfDay*/(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_c;
}

```
