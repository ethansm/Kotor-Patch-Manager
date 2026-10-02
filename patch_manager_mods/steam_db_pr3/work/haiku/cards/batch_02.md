## R054 FUNCTION CClientExoApp::GetActivePauseState  steam=0x0073fe00
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): BYTE (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 0073fe00 FUN_0073fe00 size=22 ret=0 callers=007f2130 callees=00784440

void __fastcall FUN_0073fe00(undefined4 param_1)

{
  FUN_00784440(param_1);
  return;
}

```

## R055 FUNCTION Scene::SpawnRoom  steam=0x00470290
GOG build: param_bytes=12 ghidra_sig='Scene * this, char * modelName, Vector * position, Quaternion * orientation'
Wrapper typedef (what calling code expects): Gob* (__thiscall)(void* thisPtr, char* modelName, Vector* position, Quaternion* orientation) param_bytes=12
Steam function at this address: size=2598 ret_N=12
```c
// FN 00470290 FUN_00470290 size=2598 ret=12 callers= callees=00919723,0045fde0,004545a0,0046ea30,0046e9b0,00513e20,009196ee,00428f60,0046f8e0,0048e160,00449230,00463ed0,00461aa0,0050e6e0,00496080,00463b20,004491c0,00474090,00513da0,0046e390,00919671,00456cd0,0091cff0,00462320,00473e50,0046e540,00452d40,00428f50,00495860,004740d0,0091d000,00473f40

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_00470290(int *param_1,undefined4 param_2,undefined4 *param_3,undefined4 *param_4)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  int local_180;
  int local_17c;
  undefined1 local_144 [12];
  undefined1 local_138 [12];
  undefined1 local_12c [12];
  undefined1 local_120 [12];
  undefined4 local_114;
  undefined4 local_110;
  undefined4 local_10c;
  undefined4 local_108;
  undefined4 local_104;
  undefined4 local_100;
  int local_fc;
  int local_f8;
  int local_f4;
  int local_f0;
  int local_ec;
  int local_e8;
  int local_e0;
  int local_dc;
  int local_d8;
  int *local_d4;
  int local_d0;
  char local_cc [32];
  int local_ac;
  int local_a8;
  int local_a4;
  int local_9c;
  int local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined1 local_88 [68];
  uint local_44;
  int local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  int local_2c;
  int local_28;
  char local_21;
  int local_20;
  int local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094a86a;
  local_10 = ExceptionList;
  local_44 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00428f50(local_44);
  local_40 = FUN_0048e160(param_2);
  if (local_40 != 0) {
    *(ushort *)(local_40 + 0x50) = *(ushort *)(local_40 + 0x50) | 0x100;
    local_3c = 0;
... (189 more lines)
```

## R056 FUNCTION CClientExoApp::ChangeCharacter  steam=0x00740380
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD leader, int playSound) param_bytes=8
Steam function at this address: size=32 ret_N=8
```c
// FN 00740380 FUN_00740380 size=32 ret=8 callers=00752050,007c6a20,005d75b0,00754950,007469f0,00670760 callees=00797f30

void FUN_00740380(undefined4 param_1,undefined4 param_2)

{
  FUN_00797f30(param_1,param_2);
  return;
}

```

## R057 OFFSET Scene.fog_end = 0x68
Claim: DoFog 0x46a980 glFogf(GL_FOG_END, *(this+0x68)); SetFogRange/GetFogRange use +0x68
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

## R058 FUNCTION CSWGuiControl::AddChildControl  steam=0x00419350
GOG build: param_bytes=4 ghidra_sig='CSWGuiControl * this, CSWGuiControl * child'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* child) param_bytes=4
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

## R059 FUNCTION Gob::EndLookAtAnimate  steam=0x004dcce0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=48 ret_N=0
```c
// FN 004dcce0 FUN_004dcce0 size=48 ret=0 callers= callees=

void __fastcall FUN_004dcce0(int param_1)

{
  if (*(int *)(param_1 + 0x1a0) != 0) {
    (**(code **)(**(int **)(param_1 + 0x1a0) + 8))();
  }
  return;
}

```

## R060 FUNCTION Gob::SetLightPriority  steam=0x0045c580
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int priority) param_bytes=4
Steam function at this address: size=154 ret_N=4
```c
// FN 0045c580 FUN_0045c580 size=154 ret=4 callers= callees=00463b20,00463ed0,0045c620

void __thiscall FUN_0045c580(int param_1,undefined4 param_2)

{
  uint uVar1;
  int local_20;
  int local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094d378;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00463ed0(0);
  local_8 = 0;
  FUN_0045c620(*(undefined4 *)(param_1 + 0x98),&local_20,uVar1);
  for (local_14 = 0; local_14 < local_1c; local_14 = local_14 + 1) {
    *(undefined4 *)(*(int *)(local_20 + local_14 * 4) + 0x74) = param_2;
  }
  local_8 = 0xffffffff;
  FUN_00463b20();
  ExceptionList = local_10;
  return;
}

```

## R061 FUNCTION CResGFF::WriteFieldFLOAT  steam=0x00719ce0
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, float value, char* label) param_bytes=12
Steam function at this address: size=96 ret_N=12
```c
// FN 00719ce0 FUN_00719ce0 size=96 ret=12 callers=00602dd0,005859f0,00540410,005e2700,005f18c0,00574500,0062b5f0,00520f40,00561c90,00629eb0,0053e210,005fb1a0,005e5b40,0062dec0,005e1e60,006fed80,0061bfe0,0055bd10,00629040,0052b380,005e72d0,00621ba0,0058d2f0,006b3d10,006272c0,005e1990 callees=00717aa0,00718030

undefined4 __thiscall FUN_00719ce0(int param_1,int param_2,undefined4 param_3,int param_4)

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
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,8);
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

## R062 FUNCTION CGuiInGame::HideStoreGui  steam=0x007ccf30
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=258 ret_N=0
```c
// FN 007ccf30 FUN_007ccf30 size=258 ret=0 callers=008b6010 callees=00740330,00410670,004122a0,00740350,0073fee0,007c9c40,0070bc60

void __fastcall FUN_007ccf30(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = FUN_00740330();
    if (iVar1 != 0) {
      FUN_00740350(0,0,0);
    }
    FUN_007c9c40(4,1);
    FUN_0073fee0/*CClientExoApp::SetInputClass*/(0,1);
    if (DAT_00a10760 == 0) {
      iVar1 = *(int *)(param_1 + 0x84);
      *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xffffff7f | 0x80;
      *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xfffffcff | 0x200;
      *(undefined4 *)(param_1 + 0x84) = 0;
    }
    else {
      FUN_00410670(*(undefined4 *)(param_1 + 0x84));
    }
    FUN_004122a0/*CSWGuiManager::PlayGuiSound*/(5);
    FUN_0070bc60(DAT_0099c2a8,0);
  }
  return;
}

```

## R063 FUNCTION CGuiInGame::UpdateMessageGUI  steam=0x007cd040
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=103 ret_N=4
```c
// FN 007cd040 FUN_007cd040 size=103 ret=4 callers=00759e20,0075a1f0 callees=00759820,007592f0,00758e30

void __thiscall FUN_007cd040(int param_1,int param_2)

{
  if (param_2 != 0) {
    FUN_00758e30(*(undefined4 *)(param_1 + 0x110),*(undefined4 *)(param_1 + 0x11c));
    FUN_007592f0(*(undefined4 *)(param_1 + 0x114),*(undefined4 *)(param_1 + 0x120));
    FUN_00759820(*(undefined4 *)(param_1 + 0x118),*(undefined4 *)(param_1 + 0x124));
  }
  return;
}

```

## R064 FUNCTION CGuiInGame::UpdateAreaTransitionGUI  steam=0x007ce780
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=489 ret_N=0
```c
// FN 007ce780 FUN_007ce780 size=489 ret=0 callers=00781be0 callees=008b88b0,005142f0,004452b0,00410530,00410780,0073f430,007ce970,00410800,007aae00

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_007ce780(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  float10 fVar3;
  undefined1 local_28 [12];
  int local_1c;
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  local_14 = 0;
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  if (((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x38) != 0)) &&
     (*(int *)(param_1 + 0x9c) != 0)) {
    iVar1 = FUN_0073f430();
    if (iVar1 != 0) {
      iVar1 = FUN_0073f430();
      local_1c = *(int *)(iVar1 + 0x48);
      if (local_1c != 0) {
        local_8 = FUN_007aae00();
      }
    }
    if ((local_8 == 0) || (*(int *)(param_1 + 0x34) != 1)) {
      FUN_007ce970/*CGuiInGame::HideAreaTransition*/();
    }
    else {
      iVar1 = *(int *)(param_1 + 0x9c);
      local_14 = *(undefined4 *)(iVar1 + 0x440);
      local_10 = *(undefined4 *)(iVar1 + 0x444);
      local_c = *(undefined4 *)(iVar1 + 0x448);
      local_18 = *(int *)(*(int *)(param_1 + 0x9c) + 0x44c);
      if (local_18 == *(int *)(param_1 + 0x1018)) {
        uVar2 = FUN_004452b0(local_28,&local_14,param_1 + 0x100c);
        fVar3 = (float10)FUN_005142f0(uVar2);
        if (fVar3 < (float10)_DAT_0098c1e8) {
          return;
        }
      }
      if (*(int *)(param_1 + 0x1018) == 0x7f000000) {
        FUN_007ce970/*CGuiInGame::HideAreaTransition*/();
      }
      else if (*(int *)(local_8 + 0x2f8) == 0) {
        FUN_008b88b0(*(undefined4 *)(param_1 + 0x1018),param_1 + 0x100c);
        iVar1 = CMFCVisualManagerBitmapCache::IsCached
                          (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                           *(CSize **)(param_1 + 0x9c));
        if (iVar1 == 0) {
          FUN_00410530/*CSWGuiManager::AddPanel*/(*(undefined4 *)(param_1 + 0x9c),4,1);
          FUN_00410780(*(undefined4 *)(param_1 + 0x9c));
        }
      }
      else {
        iVar1 = CMFCVisualManagerBitmapCache::IsCached
                          (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                           *(CSize **)(param_1 + 0x9c));
        if (iVar1 != 0) {
          FUN_00410780(*(undefined4 *)(param_1 + 0x9c));
        }
      }
    }
... (4 more lines)
```

## R065 FUNCTION CGuiInGame::LoadStuntModelsForDialog  steam=0x007c8390
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=321 ret_N=0
```c
// FN 007c8390 FUN_007c8390 size=321 ret=0 callers=007cc530 callees=0073f4d0,00710980,009196ee

void __fastcall FUN_007c8390(int param_1)

{
  int *piVar1;
  undefined4 uVar2;
  int *local_2c;
  char local_20 [20];
  uint local_c;
  uint local_8;
  
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  for (local_8 = 0; local_8 < *(uint *)(param_1 + 0x1ac); local_8 = local_8 + 1) {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    piVar1 = (int *)FUN_0073f4d0(*(undefined4 *)(*(int *)(param_1 + 0x1b0) + local_8 * 4));
    local_2c = (int *)0x0;
    if (piVar1 != (int *)0x0) {
      local_2c = (int *)(**(code **)(*piVar1 + 0xc))(param_1,uVar2);
    }
    if ((local_2c != (int *)0x0) && (FUN_00710980(local_20), local_20[0] != '\0')) {
      piVar1 = (int *)(**(code **)(*local_2c + 0x98))(0xff,0);
      if (piVar1 == (int *)0x0) break;
      (**(code **)(*piVar1 + 0x20))(local_20);
      piVar1 = (int *)(**(code **)(*local_2c + 0x98))(0xfe,0);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 0x20))(local_20);
      }
    }
  }
  __security_check_cookie(local_c ^ (uint)&stack0xfffffffc);
  return;
}

```

## R066 FUNCTION Gob::EndLookAtAnimate  steam=0x004dcd10
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=17 ret_N=0
```c
// FN 004dcd10 FUN_004dcd10 size=17 ret=0 callers= callees=0048dd90

void FUN_004dcd10(undefined4 param_1,undefined4 param_2)

{
  FUN_0048dd90(param_2);
  return;
}

```

## R067 OFFSET CSWGuiBorderParams.dimension = 0x0
Claim: params ctor 0x4143d0 *this=param_6; GetBorderDim 0x414980 returns *params (else derives from border); Load stores DIMENSION at border+0x14 (params+0)
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
Decompile 0x00414980:
```c
// FN 00414980 FUN_00414980 size=104 ret=0 callers=007592f0,00759820,00900940,00758e30,008bdf50,00411b20,004144f0,00414d80,00415300,0075a7c0,00882c00 callees=

int __fastcall FUN_00414980(int *param_1)

{
  int local_8;
  
  local_8 = 0;
  if (*param_1 == 0) {
    if (((param_1[0x15] != 0) && (*(int *)(param_1[0x15] + 0x6c) != 0)) &&
       (*(int *)(param_1[0x15] + 0x70) != 0)) {
      local_8 = (**(code **)(**(int **)(param_1[0x15] + 0x6c) + 0xc))();
    }
  }
  else {
    local_8 = *param_1;
  }
  return local_8;
}

```

## R068 FUNCTION CResGFF::WriteFieldVector  steam=0x0071a3d0
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, Vector* value, char* label) param_bytes=12
Steam function at this address: size=446 ret_N=12
```c
// FN 0071a3d0 FUN_0071a3d0 size=446 ret=12 callers=005fa3b0,005fb1a0,0053af20,005f8710,00534600,00529dd0,005f7f30,00632220,0055b830,005f8130,005f8320,00655010,005fe4b0,00638bd0,0076aed0,0055cc70,005302e0,00631560,005f8520,0055d7f0,005f9ff0 callees=00733570,00717aa0,00919f10,00717f30,00717630,00733780,00733db0

undefined4 __thiscall FUN_0071a3d0(int param_1,int *param_2,int *param_3)

{
  uint uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  uint local_2c;
  uint local_28;
  char *local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009657d8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570/*CExoString::CExoString_char*/(PTR_DAT_00a100c8);
  local_8 = 0;
  if (param_2 != (int *)0x0) {
    if (*param_3 == 0) {
      local_28 = 0;
    }
    else {
      local_28 = _strlen((char *)*param_3);
    }
    if (3 < local_28) {
      if (local_18 == (char *)0x0) {
        local_2c = 0;
      }
      else {
        local_2c = _strlen(local_18);
      }
      if (3 < local_2c) {
        if (*(int *)(param_1 + 0x98) != 0) {
          FUN_00717630/*CResGFF::ReleaseResource*/(uVar1);
        }
        FUN_00717aa0/*CResGFF::InitializeForWriting*/();
        uVar1 = FUN_00733db0(0);
        uVar2 = FUN_00733db0(1);
        uVar3 = FUN_00733db0(2);
        iVar4 = FUN_00733db0(3);
        **(uint **)(param_1 + 0x40) =
             uVar1 & 0xff | (uVar2 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | iVar4 << 0x18;
        uVar1 = FUN_00733db0(0);
        uVar2 = FUN_00733db0(1);
        uVar3 = FUN_00733db0(2);
        iVar4 = FUN_00733db0(3);
        *(uint *)(*(int *)(param_1 + 0x40) + 4) =
             uVar1 & 0xff | (uVar2 & 0xff) << 8 | (uVar3 & 0xff) << 0x10 | iVar4 << 0x18;
        iVar4 = FUN_00717f30/*CResGFF::AddStruct*/(0xffffffff);
        *param_2 = iVar4;
        if (*param_2 == -1) {
          local_8 = 0xffffffff;
          FUN_00733780/*CExoString::~CExoString*/();
          ExceptionList = local_10;
          return 0;
        }
        local_8 = 0xffffffff;
        FUN_00733780/*CExoString::~CExoString*/();
        ExceptionList = local_10;
        return 1;
      }
    }
  }
  local_8 = 0xffffffff;
... (5 more lines)
```

## R069 OFFSET CClientExoApp.internal = 0x4
Claim: GetClientLanguage 0x73f350 = *(*(this+4)+0x148); ~190 CClientExoApp thunks all deref [this+4] (e.g. GetWorldTimer 0x73f3b0 [internal+0x24]); existing Steam DB CAppManager.Client=4 uses same idiom
Decompile 0x0073f350:
```c
// FN 0073f350 FUN_0073f350 size=31 ret=0 callers=0040a460,00607790,0084a990,0082b990,008de2e0,00895260,008cbde0,00843500,008d0780,008ce3f0,008427a0,006cfd10,008bf280,006cae20,00605090,00896fb0,008b1250,008b3c00,00896d10,0058a610,0087c590,006c8de0,006afed0,006cc140,0077bcf0,008ddaf0,006ce710,0077c1b0,00896a80,007fc880,0077c340,0082c280,007fc390,0075dd10 callees=

undefined4 __fastcall FUN_0073f350(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x148);
}

```
Decompile 0x0073f3b0:
```c
// FN 0073f3b0 FUN_0073f3b0 size=28 ret=0 callers=00798820,0040a460,00840f50,00853a00,0087afe0,007a2590,007ac470,0084a900,0079c3f0,00883560,00785da0,007f12d0,008412a0,00781840,0087d6f0,007d0b90,00801a70,007efc40,008ee580,008e59c0,007801a0,007f3040,007f1650,008ee8c0,00530170,0084bee0,007efac0,007645f0,00785b50,0086d770,0086da30,008025d0,0078c350,00774bc0,007fd370,00781be0,007d0b40,0040a160,007a6760,007b0e90,0087b240,008e54e0,0087d990,007664b0,00801ab0,007986a0 callees=

undefined4 __fastcall FUN_0073f3b0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x24);
}

```

## R070 OFFSET CSWGuiControl.id = 0x54
Claim: Load_3 0x418f20 reads GFF "ID" into param_1[0x15] (+0x54); InitControl 0x40f620 indexes panel controls by ctrl[0x15]; ctor 0x418990 sets -1; same idiom proven live by inventory-3d mod (lesson #67)
Decompile 0x0040f620:
```c
// FN 0040f620 FUN_0040f620/*CSWGuiPanel::InitControl*/ size=970 ret=16 callers=0075b370,008850b0,00893950,008a1170,00892090,00918b10,0088aac0,009074e0,007d6170,00747210,00880150,008c19f0,008b4270,0084c3a0,008d1020,0089c5f0,007d8540,0075ae40,008bba80,007d23e0,00760210,007d9800,008c80f0,007423d0,008c9e10,00850770,007d4620,007dc410,008fb7a0,0075fda0,00916ef0,007dacb0,00745b30,00886fc0,008a92d0,009153d0,007bc600,00757c40,008e1810,008586d0,008db630,008f92b0,00910490,008b91f0,008bfb60,008f4af0,008fc8e0,008b1ea0,007450e0,008973d0,00916640,008bd910,008b82e0,00902f60,00904a80,008fff20,008bdc90,0089cf30,008df930,008c0cb0,007fae60,00754ed0,008c6650,008bc620,00914000,008a25c0,008d6b90,00881970,007d31d0,0090c6a0,0075cec0,00741940,008a6170,008bee10,00887da0,00909e00 callees=00418df0,0040fa80,0040f9f0,0040fb40,0073fea0,00734010,0083ea60,0091d6f0,0091c860

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0040f620/*CSWGuiPanel::InitControl*/(int param_1,int *param_2,undefined4 param_3,int param_4,int param_5)

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
... (38 more lines)
```
Decompile 0x00418990:
```c
// FN 00418990 FUN_00418990/*CSWGuiControl::Constructor*/ size=347 ret=0 callers=008db630,00418570,008f4af0,00899930,00895620,00880740,008973d0,008c9e10,00419740,0041b7f0,00904a80,0041bfd0,00916ef0,008fa1f0,0089cf30,0084c3a0 callees=0084a640,008a5f70,00733540

undefined4 * __fastcall FUN_00418990/*CSWGuiControl::Constructor*/(undefined4 *param_1)

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
Decompile 0x00418f20:
```c
// FN 00418f20 FUN_00418f20 size=185 ret=8 callers=004211c0,0041c870,0041bb70,004198d0,004188e0 callees=0040fea0,0040e2c0,00419330,00718c80

void __thiscall FUN_00418f20(int *param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  undefined1 local_8 [4];
  
  FUN_0040e2c0(param_2,param_3);
  (**(code **)(*param_1 + 4))(param_1 + 1);
  iVar1 = FUN_00718c80/*CResGFF::ReadFieldINT*/(param_3,&DAT_0098789c,local_8,0xffffffff);
  param_1[0x15] = iVar1;
  iVar1 = FUN_00718c80/*CResGFF::ReadFieldINT*/(param_3,"Obj_ParentID",local_8,0xffffffff);
  if (iVar1 != -1) {
    iVar2 = (**(code **)(*(int *)param_1[0xe] + 0x28))();
    if (iVar2 != 0) {
      iVar1 = FUN_0040fea0/*CSWGuiPanel::GetControl*/(iVar1);
      param_1[6] = iVar1;
      FUN_00419330/*CSWGuiControl::AddChildControl*/(param_1);
    }
  }
  return;
}

```

## R071 FUNCTION Gob::EnableDistortion  steam=0x004599f0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=21 ret_N=0
```c
// FN 004599f0 FUN_004599f0 size=21 ret=0 callers= callees=

void __fastcall FUN_004599f0(int param_1)

{
  *(undefined1 *)(param_1 + 0x1a8) = 1;
  return;
}

```

## R072 FUNCTION CResGFF::WriteFieldCExoLocString  steam=0x00719f30
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, void* value, char* label) param_bytes=12
Steam function at this address: size=633 ret_N=12
```c
// FN 00719f30 FUN_00719f30 size=633 ret=12 callers=00629040,00602dd0,00621ba0,0062b5f0,0076aed0,0061bfe0,0055bd10,006b3d10,006272c0,005f18c0,0055d080 callees=00717aa0,00734010,0091dba0,00919f10,00718030,00735840,00733780,0058c390,00718370,00735550,00733540

undefined4 __thiscall FUN_00719f30(int param_1,int param_2,int param_3,int param_4)

{
  uint uVar1;
  int iVar2;
  void *_Src;
  undefined4 uVar3;
  size_t _Size;
  size_t local_50;
  char *local_38 [2];
  int local_30;
  size_t local_2c;
  int *local_28;
  uint local_24;
  undefined4 local_20;
  byte local_19;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ffb8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_20 = 1;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if ((param_2 == 0) || (param_4 == 0)) {
    local_8 = 0xffffffff;
    FUN_00733780/*CExoString::~CExoString*/();
    uVar3 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    local_18 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,0xc);
    if (local_18 == 0) {
      local_8 = 0xffffffff;
      FUN_00733780/*CExoString::~CExoString*/();
      uVar3 = 0;
    }
    else {
      local_14 = 8;
      local_24 = 0;
      while( true ) {
        uVar1 = FUN_0058c390();
        if (uVar1 <= local_24) break;
        local_14 = local_14 + 8;
        iVar2 = FUN_00735840(local_24);
        local_14 = iVar2 + local_14;
        local_24 = local_24 + 1;
      }
      local_28 = (int *)FUN_00718370/*CResGFF::AddDataField*/(0xffffffff,0,local_14 + 4);
      if (local_28 == (int *)0x0) {
        local_8 = 0xffffffff;
        FUN_00733780/*CExoString::~CExoString*/();
        uVar3 = 0;
      }
      else {
        *(int *)(local_18 + 8) = (int)local_28 - *(int *)(param_1 + 0x5c);
        *local_28 = local_14;
        local_28[1] = *(int *)(param_3 + 4);
        local_28 = local_28 + 2;
        iVar2 = FUN_0058c390();
        *local_28 = iVar2;
... (40 more lines)
```

## R073 FUNCTION CClientExoApp::GetPazaakResults  steam=0x00740730
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int* won, int* wager) param_bytes=8
Steam function at this address: size=49 ret_N=8
```c
// FN 00740730 FUN_00740730 size=49 ret=8 callers=00682610 callees=

void __thiscall FUN_00740730(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 4);
  *param_2 = *(undefined4 *)(iVar1 + 0x80);
  *param_3 = *(undefined4 *)(iVar1 + 0x7c);
  return;
}

```

## R074 FUNCTION CSWGuiBorder::Initialize  steam=0x00414c20
GOG build: param_bytes=8 ghidra_sig='CSWGuiBorder * this, CSWGuiExtent * param_1, CSWGuiBorderParams * param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* extent, void* borderParams) param_bytes=8
Steam function at this address: size=59 ret_N=8
```c
// FN 00414c20 FUN_00414c20 size=59 ret=8 callers=0041aca0,004197d0,00419dc0,0041ca20,00421100,0041a8b0,0041d430,0041b930,00419a80 callees=004144f0

void __thiscall FUN_00414c20(int param_1,undefined4 *param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 4) = *param_2;
  *(undefined4 *)(param_1 + 8) = param_2[1];
  *(undefined4 *)(param_1 + 0xc) = param_2[2];
  *(undefined4 *)(param_1 + 0x10) = param_2[3];
  FUN_004144f0/*CSWGuiBorderParams::operator=*/(param_3);
  return;
}

```

## R075 FUNCTION CSWCObject::SetId  steam=0x00764f60
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD id) param_bytes=4
Steam function at this address: size=22 ret_N=4
```c
// FN 00764f60 FUN_00764f60 size=22 ret=4 callers= callees=

void __thiscall FUN_00764f60(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 4) = param_2;
  return;
}

```

## R076 FUNCTION CGuiInGame::ResetTalkAnimations  steam=0x007c9560
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=750 ret_N=0
```c
// FN 007c9560 FUN_007c9560 size=750 ret=0 callers=007c6a20 callees=0073f4d0,007cf4e0,007ed7d0

void __fastcall FUN_007c9560(int param_1)

{
  short sVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  
  piVar2 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x198));
  if ((piVar2 != (int *)0x0) && (iVar3 = (**(code **)(*piVar2 + 0xc))(), iVar3 != 0)) {
    iVar3 = (**(code **)(*piVar2 + 0x2c))();
    if (iVar3 == 0) {
      (**(code **)(*piVar2 + 0xc))();
      sVar1 = FUN_007ed7d0/*CSWCObject::GetCurrentAnimation*/();
      if ((sVar1 != 10000) && (iVar3 = (**(code **)(*piVar2 + 0x14))(), iVar3 == 0)) {
        piVar4 = (int *)(**(code **)(*piVar2 + 0xc))();
        (**(code **)(*piVar4 + 0x9c))(10000,0,1);
      }
      iVar3 = (**(code **)(*piVar2 + 0xc))();
      (**(code **)(**(int **)(iVar3 + 0x68) + 0x70))(0);
    }
    else {
      iVar3 = (**(code **)(*piVar2 + 0x2c))();
      iVar3 = FUN_007cf4e0(*(undefined4 *)(iVar3 + 4));
      if (iVar3 != 0) {
        (**(code **)(*piVar2 + 0xc))();
        sVar1 = FUN_007ed7d0/*CSWCObject::GetCurrentAnimation*/();
        if (sVar1 != 10000) {
          piVar4 = (int *)(**(code **)(*piVar2 + 0xc))();
          (**(code **)(*piVar4 + 0x9c))(10000,0,1);
        }
        iVar3 = (**(code **)(*piVar2 + 0xc))();
        (**(code **)(**(int **)(iVar3 + 0x68) + 0x70))(0);
      }
    }
  }
  piVar2 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x19c));
  if ((piVar2 != (int *)0x0) && (iVar3 = (**(code **)(*piVar2 + 0xc))(), iVar3 != 0)) {
    iVar3 = (**(code **)(*piVar2 + 0x2c))();
    if (iVar3 == 0) {
      (**(code **)(*piVar2 + 0xc))();
      sVar1 = FUN_007ed7d0/*CSWCObject::GetCurrentAnimation*/();
      if ((sVar1 != 10000) && (iVar3 = (**(code **)(*piVar2 + 0x14))(), iVar3 == 0)) {
        piVar4 = (int *)(**(code **)(*piVar2 + 0xc))();
        (**(code **)(*piVar4 + 0x9c))(10000,0,1);
      }
      iVar3 = (**(code **)(*piVar2 + 0xc))();
      (**(code **)(**(int **)(iVar3 + 0x68) + 0x70))(0);
    }
    else {
      iVar3 = (**(code **)(*piVar2 + 0x2c))();
      iVar3 = FUN_007cf4e0(*(undefined4 *)(iVar3 + 4));
      if (iVar3 != 0) {
        (**(code **)(*piVar2 + 0xc))();
        sVar1 = FUN_007ed7d0/*CSWCObject::GetCurrentAnimation*/();
        if (sVar1 != 10000) {
          piVar4 = (int *)(**(code **)(*piVar2 + 0xc))();
          (**(code **)(*piVar4 + 0x9c))(10000,0,1);
        }
        iVar3 = (**(code **)(*piVar2 + 0xc))();
        (**(code **)(**(int **)(iVar3 + 0x68) + 0x70))(0);
      }
    }
  }
  return;
}

```

## R077 FUNCTION CResGFF::ReadFieldWORD  steam=0x00718b00
GOG build: param_bytes=14 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined2 param_4'
Wrapper typedef (what calling code expects): WORD (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label, int* success, WORD defaultValue) param_bytes=16
Steam function at this address: size=125 ret_N=16
```c
// FN 00718b00 FUN_00718b00 size=125 ret=16 callers=005e5510,006c67e0,00853550,00630810,00664800,0053fd50,0061a210,005e2090,00574180,006c7400,005582f0,006afed0,00601740,00592840,00637ce0,0058ca50,00572a70,00631260,0058d5e0,006b3b70,00620250,00523870,005ef570,006d0db0 callees=00717770,007178e0

undefined2 FUN_00718b00(int param_1,int param_2,undefined4 *param_3,undefined2 param_4)

{
  undefined4 uVar1;
  int *piVar2;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 2)) {
      *param_3 = 0;
    }
    else {
      *param_3 = 1;
      param_4 = (undefined2)piVar2[2];
    }
  }
  return param_4;
}

```

## R078 FUNCTION CGuiInGame::UpdateDialog  steam=0x007cd0f0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=977 ret_N=0
```c
// FN 007cd0f0 FUN_007cd0f0 size=977 ret=0 callers=00781be0,008ba520,007c6670 callees=007bff30,0051c8b0,0051c0a0,007c0030,0073f430,008ba6c0,0073f4d0,0051c600,00546710,007be2d0,007c84e0,0051c8f0,007c7d60

void __fastcall FUN_007cd0f0(int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  undefined4 uVar5;
  uint uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 local_7c;
  int local_3c;
  int local_1c;
  undefined4 local_14;
  
  if (*(int *)(param_1 + 0xcc) != 0) {
    if (*(int *)(param_1 + 0xffc) == 1) {
      iVar2 = FUN_0073f430();
      if (*(int **)(iVar2 + 0x40) != (int *)0x0) {
        (**(code **)(**(int **)(iVar2 + 0x40) + 0x44))(*(undefined4 *)(param_1 + 0x1000));
        *(undefined4 *)(param_1 + 0xffc) = 0;
      }
    }
    if (*(int *)(param_1 + 0xe8) != 0) {
      iVar2 = *(int *)(param_1 + 0xe0);
      local_14 = *(undefined4 *)(param_1 + 400);
      if (*(int *)(param_1 + 0x140) != 0) {
        if (*(int *)(*(int *)(param_1 + 0x140) + iVar2 * 4) != 0x7f000000) {
          local_14 = *(undefined4 *)(*(int *)(param_1 + 0x140) + iVar2 * 4);
        }
        uVar5 = *(undefined4 *)(param_1 + 0x1a4);
        piVar3 = (int *)FUN_0073f4d0(local_14);
        if ((((piVar3 != (int *)0x0) && (iVar4 = (**(code **)(*piVar3 + 0xc))(), iVar4 != 0)) &&
            (piVar3 = (int *)FUN_0073f4d0(uVar5), piVar3 != (int *)0x0)) &&
           (iVar4 = (**(code **)(*piVar3 + 0xc))(), iVar4 != 0)) {
          for (local_1c = 0; local_1c < *(int *)(*(int *)(param_1 + 0x174) + iVar2 * 4);
              local_1c = local_1c + 1) {
            piVar3 = (int *)FUN_0073f4d0(*(undefined4 *)
                                          (*(int *)(*(int *)(param_1 + 0x178) + iVar2 * 4) +
                                          local_1c * 4));
            if (piVar3 == (int *)0x0) {
              return;
            }
            iVar4 = (**(code **)(*piVar3 + 0xc))();
            if (iVar4 == 0) {
              return;
            }
          }
          FUN_007be2d0(*(int *)(param_1 + 0x138) + *(int *)(param_1 + 0xe0) * 8,uVar5);
          if (*(int *)(param_1 + 0xec) == 0) {
            FUN_007c7d60(*(undefined4 *)(param_1 + 0xe0),*(undefined4 *)(param_1 + 0xe4));
          }
          else {
            if (*(int *)(param_1 + 0xe0) < 0) {
              local_7c = 0xffffffff;
            }
            else {
              local_7c = FUN_008ba6c0(*(undefined4 *)(param_1 + 0xe0));
            }
            uVar6 = (uint)(*(int *)(param_1 + 0xe0) == -3);
            uVar5 = FUN_0051c8f0();
            iVar4 = FUN_0051c600(uVar5);
            uVar5 = FUN_0051c8b0/*CServerExoApp::ClientToServerObjectId*/(*(undefined4 *)(*(int *)(param_1 + 0x3c) + 0x275c));
            piVar3 = (int *)FUN_0051c0a0/*CServerExoApp::GetGameObject*/(uVar5);
            if (*(int *)(param_1 + 0xd8) == 0) {
              iVar1 = *(int *)(*(int *)(param_1 + 0x174) + iVar2 * 4);
              FUN_007bff30(iVar1);
... (25 more lines)
```

## R079 FUNCTION CClientExoApp::EnableInput  steam=0x007402f0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 007402f0 FUN_007402f0 size=22 ret=0 callers=00796ba0 callees=007b3390

void __fastcall FUN_007402f0(undefined4 param_1)

{
  FUN_007b3390(param_1);
  return;
}

```

## R080 FUNCTION CGuiInGame::SwitchToSWInGameGui  steam=0x007ca550
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int guiId) param_bytes=4
Steam function at this address: size=389 ret_N=4
```c
// FN 007ca550 FUN_007ca550 size=389 ret=4 callers=00757a80,00757ab0,00757a50,00757b10,00757ba0,00757b40,007b12c0,00757ae0,00757b70 callees=00733570,0073fea0,00410670,00410530,007d0760,00410780,006fd8d0,00733780,00756860

void __thiscall FUN_007ca550(int param_1,int param_2)

{
  int local_28;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952ee8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((((-1 < param_2) && (param_2 < 8)) && (param_2 != *(int *)(param_1 + 0x2c))) &&
     (*(int *)(param_1 + 0x128) != 0)) {
    local_14 = FUN_0073fea0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (((DAT_00a10760 != 0) && (*(int *)(param_1 + 0x30) != 0)) && (local_14 != 0)) {
      FUN_00410670(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
    }
    if ((*(int *)(param_1 + 0x30) == 0) || (local_14 == 0)) {
      local_28 = -1;
    }
    else {
      local_28 = param_2;
    }
    FUN_007d0760(*(undefined4 *)(param_1 + 0x2c),local_28);
    *(int *)(param_1 + 0x2c) = param_2;
    if ((*(int *)(param_1 + 0x30) != 0) && (local_14 != 0)) {
      FUN_00410530/*CSWGuiManager::AddPanel*/(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4),2,1);
      FUN_00410780(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
      FUN_00756860(*(undefined4 *)(param_1 + 0x2c),1);
      if (DAT_00a1b4a8 != 0) {
        FUN_00733570/*CExoString::CExoString_char*/("k_pend_screenchg");
        local_8 = 0;
        FUN_006fd8d0(local_1c,0x7f000000,0);
        local_8 = 0xffffffff;
        FUN_00733780/*CExoString::~CExoString*/();
      }
    }
  }
  ExceptionList = local_10;
  return;
}

```
