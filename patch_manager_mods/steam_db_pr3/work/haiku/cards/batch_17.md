## R459 FUNCTION CServerExoApp::GetActiveTimer  steam=0x0051c350
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, int objectId) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 0051c350 FUN_0051c350 size=28 ret=4 callers=00587290,0058be80,00569320,005bfc00,0056b8a0,005a4d70,005b8000,00544ad0,0056c310,005b96f0,005c3f70,00576dc0 callees=00538e00

void FUN_0051c350(undefined4 param_1)

{
  FUN_00538e00(param_1);
  return;
}

```

## R460 FUNCTION Gob::EnableRenderBumpedOut  steam=0x0045bf50
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=1343 ret_N=8
```c
// FN 0045bf50 FUN_0045bf50 size=1343 ret=8 callers= callees=00463b20,00484d10,004741e0,00475f70,009196ee,00475ab0,004274e0,00484dc0,004762f0,009370c7,00463ba0,00475ea0,0091e050,00919f10,00463ed0,004276a0,0091d780

/* WARNING: Function: __alloca_probe replaced with injection: alloca_probe */

void __thiscall FUN_0045bf50(int *param_1,undefined4 param_2,int param_3)

{
  char cVar1;
  uint uVar2;
  undefined4 uVar3;
  char *pcVar4;
  int iVar5;
  size_t sVar6;
  int iVar7;
  int local_2074;
  char *local_2070;
  char local_2068 [8196];
  int local_64;
  int local_60;
  undefined4 local_5c;
  char local_58 [32];
  uint local_38;
  int local_34;
  undefined4 local_30;
  int local_2c;
  int *local_28;
  char *local_24;
  int local_20;
  int local_1c;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00949ee8;
  local_10 = ExceptionList;
  local_14 = 0x45bf6b;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(undefined1 *)((int)param_1 + 0x15) = 1;
  local_38 = uVar2;
  FUN_00463ed0(0);
  local_8 = 0;
  (**(code **)(*param_1 + 0x110))(FUN_0045bf30,&local_20,uVar2);
  local_2c = 0;
  local_34 = 0;
  local_58[0] = '\0';
  local_28 = (int *)FUN_004274e0(param_2,0);
  local_5c = 0;
  local_30 = 1;
  cVar1 = (**(code **)(*local_28 + 0x34))();
  if (cVar1 == '\0') {
    local_60 = FUN_00475ab0(param_2,&DAT_009883e0,&local_2c,1);
    if (local_60 == 0) {
      (**(code **)(*local_28 + 0xa8))(1);
      local_8 = 0xffffffff;
      FUN_00463b20();
      goto LAB_0045c474;
    }
    if (local_2c < 0) {
      local_34 = -1 - local_2c;
    }
    else {
      local_34 = local_2c;
    }
    if (0 < local_34) {
      uVar3 = FUN_004762f0(0,local_60,local_34);
      (**(code **)(*local_28 + 0xa4))(uVar3);
      while (local_24 = (char *)FUN_00475f70(), local_24 != (char *)0x0) {
... (105 more lines)
```

## R461 FUNCTION CResGFF::WriteFieldDWORD  steam=0x00719b90
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, DWORD value, char* label) param_bytes=12
Steam function at this address: size=96 ret_N=12
```c
// FN 00719b90 FUN_00719b90 size=96 ret=12 callers=00602dd0,005859f0,0053c680,0053e4c0,0052a600,00664d80,00520190,0062b5f0,0052afc0,0052aeb0,00629eb0,0052b270,0055ddf0,005e5b40,005e9510,006fed80,0061bfe0,0053af20,0055bd10,0052ac80,00592310,00621ba0,0076aed0,0055d7f0,0069cdb0,005e1990,0058c960,0052ab70,005f7b80,00703370,00540410,00664e40,005e2700,005f18c0,00574500,0052ad90,0052a840,00520f40,0052a730,0052aa60,00561c90,005fb1a0,0062dec0,0052b0d0,005e1e60,00534600,0052a950,0055d080,005e72d0,0058d2f0,006b3d10,006272c0 callees=00717aa0,00718030

undefined4 __thiscall FUN_00719b90(int param_1,int param_2,undefined4 param_3,int param_4)

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
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,4);
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

## R462 FUNCTION Gob::SetEnvironmentMap  steam=0x00461300
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, char* envMapName) param_bytes=4
Steam function at this address: size=389 ret_N=4
```c
// FN 00461300 FUN_00461300 size=389 ret=4 callers= callees=00463b20,004274e0,004741e0,00422e20,0091cff0,00463ed0,004276a0,00460a80,009196ee

void __thiscall FUN_00461300(int param_1,char *param_2)

{
  char cVar1;
  undefined1 local_70 [4];
  int local_6c;
  int local_64;
  int *local_60;
  undefined4 local_5c;
  undefined1 local_58 [64];
  uint local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094a058;
  local_10 = ExceptionList;
  local_18 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if ((param_2 == (char *)0x0) || (*param_2 == '\0')) {
    FUN_0091cff0(local_58,&DAT_009884c8,local_18);
  }
  else {
    FUN_0091cff0(local_58,param_2,local_18);
  }
  local_5c = 0;
  local_60 = (int *)FUN_004274e0(local_58,0);
  cVar1 = (**(code **)(*local_60 + 0x10))();
  if (cVar1 == '\0') {
    local_14 = *(undefined4 *)(param_1 + 0x1d4);
    FUN_004741e0(local_60);
    FUN_004276a0/*ReleaseTextureProxy (delete if back-ref count 0)*/(local_14);
    if (*(int *)(param_1 + 0x1d4) == 0) {
      local_5c = 0;
    }
    else {
      local_5c = 1;
      FUN_00463ed0(0);
      local_8 = 0;
      FUN_00460a80(*(undefined4 *)(param_1 + 0x98),local_70);
      for (local_64 = 0; local_64 < local_6c; local_64 = local_64 + 1) {
        FUN_00422e20/*CAurTexture::SetIsEnvironmentMapped (SetFlagDE)*/(1);
      }
      local_8 = 0xffffffff;
      FUN_00463b20();
    }
  }
  else {
    FUN_004276a0/*ReleaseTextureProxy (delete if back-ref count 0)*/(local_60);
  }
  ExceptionList = local_10;
  __security_check_cookie(local_18 ^ (uint)&stack0xfffffffc);
  return;
}

```

## R463 FUNCTION Scene::DisableVisibilityGraph  steam=0x0046fb40
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=26 ret_N=0
```c
// FN 0046fb40 FUN_0046fb40 size=26 ret=0 callers= callees=

void __fastcall FUN_0046fb40(int param_1)

{
  *(undefined2 *)(param_1 + 0xe4) = 1;
  return;
}

```

## R464 FUNCTION CClientExoApp::GetWorldTimer  steam=0x0073f3b0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=28 ret_N=0
```c
// FN 0073f3b0 FUN_0073f3b0 size=28 ret=0 callers=00798820,0040a460,00840f50,00853a00,0087afe0,007a2590,007ac470,0084a900,0079c3f0,00883560,00785da0,007f12d0,008412a0,00781840,0087d6f0,007d0b90,00801a70,007efc40,008ee580,008e59c0,007801a0,007f3040,007f1650,008ee8c0,00530170,0084bee0,007efac0,007645f0,00785b50,0086d770,0086da30,008025d0,0078c350,00774bc0,007fd370,00781be0,007d0b40,0040a160,007a6760,007b0e90,0087b240,008e54e0,0087d990,007664b0,00801ab0,007986a0 callees=

undefined4 __fastcall FUN_0073f3b0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x24);
}

```

## R465 FUNCTION CClientExoApp::PlayerFlourishWeapons  steam=0x007408a0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 007408a0 FUN_007408a0 size=22 ret=0 callers=007b12c0,008993f0 callees=0079f430

void __fastcall FUN_007408a0(undefined4 param_1)

{
  FUN_0079f430(param_1);
  return;
}

```

## R466 FUNCTION Gob::GetModelName  steam=0x004615d0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): CResRef* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=39 ret_N=0
```c
// FN 004615d0 FUN_004615d0 size=39 ret=0 callers= callees=

int __fastcall FUN_004615d0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x84) == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(param_1 + 0x84) + 8;
  }
  return iVar1;
}

```

## R467 FUNCTION Gob::RestoreTexture  steam=0x00460ae0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=203 ret_N=0
```c
// FN 00460ae0 FUN_00460ae0 size=203 ret=0 callers= callees=00463b20,00460bb0,004db0a0,00463ed0,004545a0

undefined4 FUN_00460ae0(void)

{
  uint uVar1;
  int iVar2;
  int extraout_var;
  int iVar3;
  int local_20;
  int local_1c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094a1b8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00463ed0(0);
  local_8 = 0;
  iVar3 = extraout_var;
  FUN_00460bb0(*(undefined4 *)(extraout_var + 0x98),&local_1c,uVar1);
  for (local_20 = 0; local_20 < local_18; local_20 = local_20 + 1) {
    iVar2 = FUN_004545a0(uVar1,iVar3,local_1c + local_20 * 4);
    FUN_004db0a0(0,iVar2 + 0xa8);
  }
  local_8 = 0xffffffff;
  FUN_00463b20();
  ExceptionList = local_10;
  return 1;
}

```

## R468 FUNCTION CSWGuiPanel::GetLocalMouseCoords  steam=0x0040fed0
GOG build: param_bytes=8 ghidra_sig='CSWGuiPanel * this, int * outX, int * outY'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int* outX, int* outY) param_bytes=8
Steam function at this address: size=215 ret_N=8
```c
// FN 0040fed0 FUN_0040fed0 size=215 ret=8 callers=0041b660,0041b6f0,0041f980,00420690,00420850,00420c00 callees=00410020

void __thiscall FUN_0040fed0(int param_1,int *param_2,int *param_3)

{
  int local_14;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  *param_2 = *DAT_00a1b49c;
  *param_3 = DAT_00a1b49c[1];
  local_14 = *(int *)(param_1 + 4);
  local_10 = *(int *)(param_1 + 8);
  local_c = *(undefined4 *)(param_1 + 0xc);
  local_8 = *(undefined4 *)(param_1 + 0x10);
  if ((*(uint *)(param_1 + 0x48) & 1) == 0) {
    *param_2 = *param_2 - (DAT_009f42a4 + -0x280) / 2;
    *param_3 = *param_3 - (DAT_009f42a8 + -0x1e0) / 2;
  }
  else {
    FUN_00410020/*CSWGuiPanel::GetExtentAccountingForPanelOffset*/(&local_14);
  }
  *param_2 = *param_2 - local_14;
  *param_3 = *param_3 - local_10;
  return;
}

```

## R469 FUNCTION CResGFF::ReadFieldBYTE  steam=0x00718a00
GOG build: param_bytes=12 ghidra_sig='CResGFF * this, int param_1, char * param_2, undefined4 * param_3, undefined param_4'
Wrapper typedef (what calling code expects): BYTE (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label, int* success, BYTE defaultValue) param_bytes=16
Steam function at this address: size=122 ret_N=16
```c
// FN 00718a00 FUN_00718a00 size=122 ret=16 callers=00625360,005e2900,00628c40,0062db90,005269e0,0069d1e0,00631260,0041bb70,006c67e0,00630810,00619e60,005e9370,006c53f0,00838a70,00561d80,00415080,00572a70,0053e580,0058d5e0,00620250,0041c870,00855f30,006c75b0,006295d0,00853550,00525c60,005205f0,006029f0,0041a7a0,0041ffd0,0053fc70,00528380,00573950,005e5510,0062af70,0083db30,00527890,0061a210,00574180,005582f0,006afed0,005fd040,00601740,00592840,0058ca50,005fbf60,00417480,00523870,005ef570,00602be0 callees=00717770,007178e0

undefined1 FUN_00718a00(int param_1,int param_2,undefined4 *param_3,undefined1 param_4)

{
  undefined4 uVar1;
  int *piVar2;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 0)) {
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

## R470 FUNCTION CClientExoApp::SetTexturePack  steam=0x0073fbf0
GOG build: param_bytes=1 ghidra_sig='undefined1 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, BYTE pack) param_bytes=4
Steam function at this address: size=31 ret_N=4
```c
// FN 0073fbf0 FUN_0073fbf0 size=31 ret=4 callers=007bc2f0 callees=0078d6a0

void FUN_0073fbf0(undefined1 param_1)

{
  FUN_0078d6a0(param_1,1);
  return;
}

```

## R471 OFFSET Scene.fog_density = 0x70
Claim: DoFog 0x46a980 glFogf(GL_FOG_DENSITY, *(this+0x70))
Decompile 0x0046a980:
```c
// FN 0046a980 FUN_0046a980 size=671 ret=0 callers= callees=00000059,0000001c,0000000e,0000005a,00000060

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0046a980(int param_1)

{
  int local_3c;
  int local_2c;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (DAT_00a20064 == 0) {
    if (*(int *)(param_1 + 0x60) == 0) {
      glDisable(DAT_009f4930);
    }
    else {
      glEnable(DAT_009f4930);
      if (*(int *)(param_1 + 0x6c) == 0) {
        local_2c = 0;
      }
      else {
        local_2c = (*(int *)(param_1 + 0x6c) != 1) + 1;
      }
      glFogi(0xb65,(&DAT_0098851c)[local_2c]);
      if (DAT_00a7be44 == 0) {
        if (DAT_00a7bac4 == '\0') {
          glFogfv(DAT_009f4970,param_1 + 0x74);
        }
        else {
          glFogfv(DAT_009f4970,param_1 + 0x80);
        }
      }
      else {
        local_10 = 0;
        local_c = 0;
        local_8 = 0;
        glFogfv(DAT_009f4970,&local_10);
      }
      glFogf(DAT_009f4964,*(undefined4 *)(param_1 + 100));
      glFogf(DAT_009f4968,*(undefined4 *)(param_1 + 0x68));
      glFogf(DAT_009f496c,*(undefined4 *)(param_1 + 0x70));
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x60) = 1;
... (25 more lines)
```

## R472 FUNCTION CClientExoApp::RemoveMoveToModuleMovies  steam=0x007401c0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=38 ret_N=0
```c
// FN 007401c0 FUN_007401c0 size=38 ret=0 callers=00798820 callees=007413e0

void FUN_007401c0(void)

{
  FUN_007413e0(0);
  return;
}

```

## R473 FUNCTION CSWGuiManager::PopModalPanel  steam=0x004109c0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=217 ret_N=0
```c
// FN 004109c0 FUN_004109c0 size=217 ret=0 callers=00917f10,007d8ff0,008c9750,008e2400,0090fb50,00858c60,0088a420,008fbe00,00410670,0089cc50,007983e0,00906310,007d2bc0,007d5580,007d9280,00917d60,007cd760,008f6290,008d4420,007cb540,008c9380,008e0a80,00887b10,007d3bb0,007db680,00853a00,007cb4a0,00908b50,007da660,007cab60,008cfd90,008f8070,00908ae0,007d7290,008545c0,007cad30,007cac10,008f82c0,00906110,0090b360,007caa60,008fd7c0,00919430,008cfd10,0090e990,007dccd0,00901310,00913650,0090b430,009007a0,00851770,00858be0,007d0f50,0075b8e0,00912790,00919230 callees=0049b840,00411170

undefined4 __fastcall FUN_004109c0(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  int *piVar3;
  
  iVar2 = *(int *)(param_1 + 0x98);
  if (iVar2 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0x94) + -4 + iVar2 * 4);
    FUN_0049b840(iVar2 + -1);
    FUN_00411170();
    if ((0 < *(int *)(param_1 + 0x98)) &&
       (iVar2 = (**(code **)(**(int **)(*(int *)(param_1 + 0x94) + -4 + *(int *)(param_1 + 0x98) * 4
                                       ) + 0x44))(), iVar2 != 0)) {
      piVar3 = (int *)(**(code **)(**(int **)(*(int *)(param_1 + 0x94) + -4 +
                                             *(int *)(param_1 + 0x98) * 4) + 0x44))();
      (**(code **)(*piVar3 + 0x7c))(1);
    }
  }
  return uVar1;
}

```
