## R405 FUNCTION CResGFF::InitializeForWriting  steam=0x00717aa0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
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

## R406 FUNCTION CGuiInGame::StopGlobalFade  steam=0x007cbbf0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=69 ret_N=0
```c
// FN 007cbbf0 FUN_007cbbf0 size=69 ret=0 callers=0061f170,00617b40 callees=00410670,007bc7f0

void __fastcall FUN_007cbbf0(int param_1)

{
  if ((*(int *)(param_1 + 0x6c) != 0) && (*(int *)(param_1 + 0x1024) == 0)) {
    FUN_007bc7f0();
    FUN_00410670(*(undefined4 *)(param_1 + 0x6c));
  }
  return;
}

```

## R407 FUNCTION CWorldTimer::ConvertFromTimeOfDay  steam=0x0051ab40
GOG build: param_bytes=20 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD timeInMS, DWORD* outHours, DWORD* outMinutes, DWORD* outSeconds, DWORD* outMS) param_bytes=20
Steam function at this address: size=129 ret_N=20
```c
// FN 0051ab40 FUN_0051ab40 size=129 ret=20 callers=0055bd10,0051b1c0,0051b100,0053af20,0051b140,00533830,0055f520,0051b180 callees=

void __thiscall
FUN_0051ab40(int param_1,uint param_2,uint *param_3,uint *param_4,uint *param_5,uint *param_6)

{
  uint uVar1;
  
  *param_6 = param_2 % 1000;
  *param_5 = (param_2 / 1000) % 0x3c;
  uVar1 = (param_2 / 1000) / 0x3c;
  *param_4 = uVar1 % (uint)*(byte *)(param_1 + 0x38);
  *param_3 = uVar1 / *(byte *)(param_1 + 0x38);
  return;
}

```

## R408 FUNCTION CSWGuiPanel::ResetFont  steam=0x0040ffb0
GOG build: param_bytes=0 ghidra_sig='CSWGuiPanel * this'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=99 ret_N=0
```c
// FN 0040ffb0 FUN_0040ffb0 size=99 ret=0 callers=00753010 callees=

void __fastcall FUN_0040ffb0(int param_1)

{
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x18) != 0) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x28); local_8 = local_8 + 1) {
      if (*(int *)(*(int *)(param_1 + 0x24) + local_8 * 4) != 0) {
        (**(code **)(**(int **)(*(int *)(param_1 + 0x24) + local_8 * 4) + 0x8c))();
      }
    }
  }
  return;
}

```

## R409 FUNCTION CClientExoApp::DisableVideoEffect  steam=0x007409c0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 007409c0 FUN_007409c0 size=22 ret=0 callers=007e6ce0,007c6200,007c6530,007e2a20,007c6a20,007c41f0,00698750 callees=0079e960

void __fastcall FUN_007409c0(undefined4 param_1)

{
  FUN_0079e960(param_1);
  return;
}

```

## R410 FUNCTION CResGFF::Destructor_2  steam=0x007171b0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, byte shouldFree) param_bytes=4
Steam function at this address: size=329 ret_N=0
```c
// FN 007171b0 FUN_007171b0 size=329 ret=0 callers=00716fa0,0094e580,005302e0 callees=009196fd,00717630,007111a0

void __fastcall FUN_007171b0(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0095ff38;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CResGFF::vftable;
  local_8 = 0;
  if (param_1[0x26] == 0) {
    FUN_009196fd/*operator delete*/(param_1[0x1c]);
    param_1[0x1c] = 0;
    FUN_009196fd/*operator delete*/(param_1[0x19]);
    param_1[0x19] = 0;
    FUN_009196fd/*operator delete*/(param_1[0x17]);
    param_1[0x17] = 0;
    FUN_009196fd/*operator delete*/(param_1[0x15]);
    param_1[0x15] = 0;
    FUN_009196fd/*operator delete*/(param_1[0x13]);
    param_1[0x13] = 0;
    FUN_009196fd/*operator delete*/(param_1[0x11]);
    param_1[0x11] = 0;
    FUN_009196fd/*operator delete*/(param_1[0x10]);
    param_1[0x10] = 0;
  }
  else {
    FUN_00717630/*CResGFF::ReleaseResource*/(uVar1);
  }
  local_8 = 0xffffffff;
  FUN_007111a0();
  ExceptionList = local_10;
  return;
}

```

## R411 FUNCTION Gob::GetMaximumLightRadius  steam=0x0045c690
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): double (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=180 ret_N=0
```c
// FN 0045c690 FUN_0045c690 size=180 ret=0 callers= callees=00463b20,004631c0,00463ed0

float10 FUN_0045c690(void)

{
  float fVar1;
  uint uVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  int local_24;
  int local_20;
  int local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00949f18;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = 0.0;
  FUN_00463ed0(0);
  local_8 = 0;
  uVar3 = extraout_var;
  FUN_004631c0(&local_24);
  for (local_18 = 0; fVar1 = local_14, local_18 < local_20; local_18 = local_18 + 1) {
    if (local_14 < *(float *)(*(int *)(local_24 + local_18 * 4) + 0x58)) {
      local_14 = *(float *)(*(int *)(local_24 + local_18 * 4) + 0x58);
    }
  }
  local_8 = 0xffffffff;
  FUN_00463b20(uVar2,uVar3);
  ExceptionList = local_10;
  return (float10)fVar1;
}

```

## R412 OFFSET CSWGuiControl.custom_value = 0x5c
Claim: ctor 0x418990 zeroes param_1[0x17] (+0x5c); entry-control handler 0x8a8100 treats control+0x5c as a flags word; no reader/writer pair naming it custom value (init + consumer only)
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
Decompile 0x008a8100:
```c
// FN 008a8100 FUN_008a8100 size=703 ret=4 callers= callees=007feec0,0073fb90,00416e30,00919f10,0051c0e0,0071ebd0,0041d290,0041e9a0,00607790,00733780,00733570,007e5d80,00734190,005210a0,004178b0,0041fd90

void __thiscall FUN_008a8100(int param_1,int param_2)

{
  int iVar1;
  undefined1 uVar2;
  uint uVar3;
  size_t local_68;
  undefined1 local_4c [8];
  undefined4 local_44;
  int local_40;
  int local_3c;
  undefined1 local_38 [8];
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00978140;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(int *)(param_2 + 0x50) != 0) {
    local_28 = param_2;
    local_24 = *(int *)(param_2 + 0x1d0);
    if (local_24 == 0x7f000000) {
      *(uint *)(param_1 + 0x24d0) = *(uint *)(param_1 + 0x24d0) & 0xfffffffd;
      FUN_00733570/*CExoString::CExoString_char*/("Error: Invalid item");
      local_8 = 1;
      FUN_00416e30(local_4c);
      local_8 = 0xffffffff;
      FUN_00733780/*CExoString::~CExoString*/();
    }
    else {
      local_30 = FUN_0051c0e0(local_24);
      FUN_00607790(local_38);
      local_8 = 0;
      local_40 = FUN_00734190(0x3c,0);
      local_3c = FUN_00734190(0x7b,0);
      if ((local_40 != -1) || (local_3c != -1)) {
        FUN_0073fb90/*ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)*/(uVar3);
... (43 more lines)
```

## R413 OFFSET CSWGuiBorderParams.inner_offset = 0x8
Claim: params ctor 0x4143d0 param_10 -> params[2] (+8); Load stores INNEROFFSETY at border+0x1c (params+8). NB +4 is INNEROFFSET(X); GOG name inner_offset denotes the +8 slot
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

## R414 FUNCTION CGuiInGame::ShowStatusSummary  steam=0x007cf270
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
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

## R415 FUNCTION CClientExoApp::SetFutureLeader  steam=0x00740770
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD leader) param_bytes=4
Steam function at this address: size=36 ret_N=4
```c
// FN 00740770 FUN_00740770 size=36 ret=4 callers=005fd9a0,007e5730 callees=

void __thiscall FUN_00740770(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x338) = param_2;
  return;
}

```

## R416 FUNCTION CGuiInGame::LoadAmbientTrackForDialog  steam=0x007c40c0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* track) param_bytes=4
Steam function at this address: size=304 ret_N=4
```c
// FN 007c40c0 FUN_007c40c0 size=304 ret=4 callers=00798820,007cc530 callees=0070b080,009196fd,00710a10,00919723,0070b170,0070afd0,0070b350

void __thiscall FUN_007c40c0(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 local_30;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097d64b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0xfbc) != 0) {
    iVar1 = *(int *)(param_1 + 0xfbc);
    if (iVar1 != 0) {
      FUN_0070b080(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
      FUN_009196fd/*operator delete*/(iVar1);
    }
    *(undefined4 *)(param_1 + 0xfbc) = 0;
  }
  iVar1 = FUN_00710a10();
  if (iVar1 != 0) {
    iVar1 = FUN_00919723/*operator new*/(4);
    local_8 = 0;
    if (iVar1 == 0) {
      local_30 = 0;
    }
    else {
      local_30 = FUN_0070afd0(1);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0xfbc) = local_30;
    if (*(int *)(param_1 + 0xfbc) != 0) {
      FUN_0070b350(4);
      FUN_0070b170(param_2,8,0,1,0);
    }
  }
  ExceptionList = local_10;
  return;
}

```

## R417 FUNCTION CResGFF::Destructor  steam=0x00716fa0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=44 ret_N=4
```c
// FN 00716fa0 FUN_00716fa0 size=44 ret=4 callers= callees=009196fd,007171b0

undefined4 __thiscall FUN_00716fa0(undefined4 param_1,uint param_2)

{
  FUN_007171b0/*CResGFF::Destructor_2*/();
  if ((param_2 & 1) != 0) {
    FUN_009196fd/*operator delete*/(param_1);
  }
  return param_1;
}

```

## R418 FUNCTION CSWCObject::GetPortrait  steam=0x007ed480
GOG build: param_bytes=5 ghidra_sig='undefined4 param_1, undefined1 param_2'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, void* outResRef) param_bytes=4
Steam function at this address: size=39 ret_N=8
```c
// FN 007ed480 FUN_007ed480 size=39 ret=8 callers= callees=0085e8c0

undefined4 FUN_007ed480(undefined4 param_1,undefined1 param_2)

{
  FUN_0085e8c0(param_1,param_2);
  return param_1;
}

```

## R419 FUNCTION Gob::IsForceAnimated  steam=0x00459da0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): unsigned char (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=17 ret_N=0
```c
// FN 00459da0 FUN_00459da0 size=17 ret=0 callers= callees=

undefined1 __fastcall FUN_00459da0(int param_1)

{
  return *(undefined1 *)(param_1 + 0x10);
}

```

## R420 FUNCTION CSWCObject::AnimationStationary  steam=0x007ef2c0
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=132 ret_N=4
```c
// FN 007ef2c0 FUN_007ef2c0 size=132 ret=4 callers=007675b0,007696e0,00769780,007ed9a0,008079b0,00776fe0 callees=0071b8b0

bool __thiscall FUN_007ef2c0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe28,&local_8);
  return iVar1 == 0 || local_8 != 0;
}

```

## R421 FUNCTION CSWGuiBorder::Draw  steam=0x00415300
GOG build: param_bytes=4 ghidra_sig='CSWGuiBorder * this, float param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, float alpha) param_bytes=4
Steam function at this address: size=1886 ret_N=4
```c
// FN 00415300 FUN_00415300 size=1886 ret=4 callers= callees=00414980,00415a60,00415d20,0047ea60

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00415300(int param_1)

{
  int iVar1;
  uint uVar2;
  int local_88;
  int local_84;
  int local_80;
  int local_7c;
  int local_60;
  int local_5c;
  int local_58;
  int local_54;
  int local_50;
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  int local_28;
  undefined4 local_24;
  int local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  int local_10;
  int local_c;
  int local_8;
  
  if ((*(int *)(param_1 + 0xc) != 0) && (*(int *)(param_1 + 0x10) != 0)) {
    local_24 = *(undefined4 *)(param_1 + 0x24);
    local_14 = *(undefined4 *)(param_1 + 0x24);
    if ((*(uint *)(param_1 + 0x34) >> 2 & 1) != 0) {
      if ((*(uint *)(param_1 + 0x34) >> 3 & 1) != 0) {
        local_24 = _DAT_009f4548;
      }
      if ((*(uint *)(param_1 + 0x34) >> 4 & 1) != 0) {
        local_14 = _DAT_009f4548;
      }
    }
    local_18 = *(int *)(param_1 + 4);
    local_20 = *(int *)(param_1 + 8);
    local_28 = 0;
    local_8 = 0;
    local_1c = 0;
    local_10 = 0;
    if (*(int *)(param_1 + 0x6c) != 0) {
      iVar1 = FUN_00414980();
      if (iVar1 == 0) {
        local_40 = (**(code **)(**(int **)(param_1 + 0x6c) + 0x10))();
        local_54 = (**(code **)(**(int **)(param_1 + 0x6c) + 0xc))();
        local_50 = local_40;
        local_3c = local_54;
        if (*(int *)(param_1 + 0x70) != 0) {
          local_50 = (**(code **)(**(int **)(param_1 + 0x70) + 0x10))();
          local_3c = (**(code **)(**(int **)(param_1 + 0x70) + 0xc))();
        }
      }
      else {
        local_54 = FUN_00414980();
        local_50 = local_54;
        local_40 = local_54;
        local_3c = local_54;
      }
... (154 more lines)
```

## R422 FUNCTION Gob::SetIllumination  steam=0x0045c7b0
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector color, int propagate) param_bytes=8
Steam function at this address: size=83 ret_N=16
```c
// FN 0045c7b0 FUN_0045c7b0 size=83 ret=16 callers= callees=

void __thiscall FUN_0045c7b0(int *param_1,int param_2,int param_3,int param_4,int param_5)

{
  *(undefined2 *)((int)param_1 + 0xf2) = 1;
  param_1[0x43] = param_2;
  param_1[0x44] = param_3;
  param_1[0x45] = param_4;
  if (param_5 != 0) {
    (**(code **)(*param_1 + 0x1b4))("propagateillumination",0);
  }
  return;
}

```

## R423 OFFSET CSWGuiPanel.controls = 0x24
Claim: GetControl 0x40fea0 = [this+0x24]+idx*4; Draw 0x40ec90 loops it with count [this+0x28]; InitControl 0x40f620 stores ctrl at [this+0x24][ctrl+0x54]; live-proven by inventory-3d mod
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
Decompile 0x0040fea0:
```c
// FN 0040fea0 FUN_0040fea0/*CSWGuiPanel::GetControl*/ size=48 ret=4 callers=00418f20,00412400 callees=

undefined4 __thiscall FUN_0040fea0/*CSWGuiPanel::GetControl*/(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if (param_2 < *(int *)(param_1 + 0x28)) {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0x24) + param_2 * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}

```

## R424 OFFSET Scene.fog_color = 0x74
Claim: DoFog 0x46a980 glFogfv(GL_FOG_COLOR, this+0x74); SetFogColor 0x464470 stores Vector at +0x74..+0x7c
Decompile 0x00464470:
```c
// FN 00464470 FUN_00464470/*Scene::SetFogColor*/ size=143 ret=4 callers= callees=0044b8a0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00464470/*Scene::SetFogColor*/(int param_1,undefined4 *param_2)

{
  float10 fVar1;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  
  *(undefined4 *)(param_1 + 0x74) = *param_2;
  *(undefined4 *)(param_1 + 0x78) = param_2[1];
  *(undefined4 *)(param_1 + 0x7c) = param_2[2];
  local_14 = _DAT_00986b60;
  local_10 = _DAT_0098baa0;
  local_c = _DAT_0098ba9c;
  fVar1 = (float10)FUN_0044b8a0(param_1 + 0x74,&local_14);
  *(float *)(param_1 + 0x88) = (float)fVar1 * _DAT_00a04900;
  *(undefined4 *)(param_1 + 0x84) = *(undefined4 *)(param_1 + 0x88);
  *(undefined4 *)(param_1 + 0x80) = *(undefined4 *)(param_1 + 0x84);
  return;
}

```
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

## R425 FUNCTION CSWGuiBorderParams::SetEdgeImage  steam=0x00414760
GOG build: param_bytes=8 ghidra_sig='CSWGuiBorderParams * this, CResRef * param_1, int forceUpdate'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* image, int forceUpdate) param_bytes=8
Steam function at this address: size=219 ret_N=8
```c
// FN 00414760 FUN_00414760 size=219 ret=8 callers=00415080,00414920,00747210 callees=00710ca0,00710a70,00710a10,0047eb60,00710980,009196ee

void __thiscall FUN_00414760(int param_1,undefined4 param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_2c [16];
  undefined1 local_1c [20];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  iVar2 = FUN_00710a70(param_1 + 0x34);
  if (((iVar2 != 0) || (param_3 != 0)) &&
     (FUN_00710ca0(local_2c,param_2), *(int *)(param_1 + 0x54) != 0)) {
    if (*(int *)(*(int *)(param_1 + 0x54) + 0x70) != 0) {
      piVar1 = *(int **)(*(int *)(param_1 + 0x54) + 0x70);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 4))(1);
      }
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x70) = 0;
    }
    iVar2 = FUN_00710a10();
    if (iVar2 != 0) {
      FUN_00710980(local_1c);
      uVar3 = FUN_0047eb60(local_1c);
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x70) = uVar3;
    }
  }
  __security_check_cookie(local_8 ^ (uint)&stack0xfffffffc);
  return;
}

```

## R426 OFFSET CSWGuiControl.parent_control = 0x18
Claim: Load_3 0x418f20 reads "Obj_ParentID", resolves it with panel GetControl 0x40fea0 and stores param_1[6] (+0x18); ctor zeroes param_1[6]
Decompile 0x0040fea0:
```c
// FN 0040fea0 FUN_0040fea0/*CSWGuiPanel::GetControl*/ size=48 ret=4 callers=00418f20,00412400 callees=

undefined4 __thiscall FUN_0040fea0/*CSWGuiPanel::GetControl*/(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if (param_2 < *(int *)(param_1 + 0x28)) {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0x24) + param_2 * 4);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
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

## R427 FUNCTION CGuiInGame::HideSoloMode  steam=0x007cd760
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=127 ret_N=0
```c
// FN 007cd760 FUN_007cd760 size=127 ret=0 callers=008b9040 callees=00740350,0073fee0,0075baa0,00410800,004109c0

void __fastcall FUN_007cd760(int param_1)

{
  int iVar1;
  
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0x94))
  ;
  if (iVar1 != 0) {
    if (*(int *)(param_1 + 0xf18) == 0) {
      FUN_00740350(0,0,0);
    }
    FUN_004109c0/*CSWGuiManager::PopModalPanel*/();
    FUN_0075baa0(0);
    FUN_0073fee0/*CClientExoApp::SetInputClass*/(0,1);
  }
  return;
}

```

## R428 FUNCTION CResGFF::ReadFieldCResRef  steam=0x00718e40
GOG build: param_bytes=20 ghidra_sig='CResGFF * this, undefined4 * param_1, undefined4 param_2, char * param_3, undefined4 * param_4, undefined4 * param_5'
Wrapper typedef (what calling code expects): CResRef* (__thiscall)(void* thisPtr, CResRef* out, CResStruct* structPtr, char* label, int* success, CResRef* defaultValue) param_bytes=20
Steam function at this address: size=336 ret_N=20
```c
// FN 00718e40 FUN_00718e40 size=336 ret=20 callers=00625360,006c75b0,006295d0,00525f90,00626eb0,005263c0,00834450,00572fc0,005269e0,00836730,008bf280,00573950,006c67e0,0062af70,0083db30,00527580,00619e60,006c53f0,00527890,0040ee40,008341f0,0061a210,005582f0,006afed0,00637ce0,00415080,008384a0,00833640,00417480,00527cb0,00620250,00523870,00526f60,005ef570,00416720,00855f30,00602be0 callees=007108a0,00717770,00717820,007178e0

undefined4 *
FUN_00718e40(undefined4 *param_1,int param_2,int param_3,undefined4 *param_4,undefined4 *param_5)

{
  undefined4 uVar1;
  uint local_14;
  byte *local_10;
  int *local_c;
  byte local_5;
  
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    *param_1 = *param_5;
    param_1[1] = param_5[1];
    param_1[2] = param_5[2];
    param_1[3] = param_5[3];
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_2,param_3);
    local_c = (int *)FUN_00717770(param_2,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 0xb)) {
      *param_4 = 0;
      *param_1 = *param_5;
      param_1[1] = param_5[1];
      param_1[2] = param_5[2];
      param_1[3] = param_5[3];
    }
    else {
      local_10 = (byte *)FUN_00717820/*CResGFF::GetDataField*/(local_c,&local_14);
      if ((local_10 == (byte *)0x0) || (local_14 == 0)) {
        *param_4 = 0;
        *param_1 = *param_5;
        param_1[1] = param_5[1];
        param_1[2] = param_5[2];
        param_1[3] = param_5[3];
      }
      else {
        local_5 = *local_10;
        if (local_14 < local_5 + 1) {
          *param_4 = 0;
          *param_1 = *param_5;
          param_1[1] = param_5[1];
          param_1[2] = param_5[2];
          param_1[3] = param_5[3];
        }
        else {
          *param_4 = 1;
          FUN_007108a0(local_10 + 1,local_5);
        }
      }
    }
  }
  return param_1;
}

```

## R429 FUNCTION CSWSPlaceable::GetDialogResref  steam=0x005ef120
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, void* outResref) param_bytes=4
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

## R430 FUNCTION CSWCObject::TurnOnShadows  steam=0x007ed4d0
GOG build: param_bytes=0 ghidra_sig='CSWCObject * this'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=42 ret_N=0
```c
// FN 007ed4d0 CPaneContainerManager::NotifyPaneDivider size=42 ret=0 callers= callees=

/* Library Function - Single Match
    public: void __thiscall CPaneContainerManager::NotifyPaneDivider(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CPaneContainerManager::NotifyPaneDivider(CPaneContainerManager *this)

{
  if (*(int *)(this + 0x68) != 0) {
    (**(code **)(**(int **)(this + 0x68) + 0xb8))();
  }
  return;
}

```

## R431 FUNCTION CSWGuiControl::Load_2  steam=0x00418df0
GOG build: param_bytes=16 ghidra_sig='CSWGuiControl * this, CSWGuiObject * param_1, CResGFF * param_2, CResList * param_3, CExoString * param_4'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* owner, void* gff, void* controls, void* tag) param_bytes=16
Steam function at this address: size=300 ret_N=16
```c
// FN 00418df0 FUN_00418df0 size=300 ret=16 callers=0040f620 callees=00718810,00733570,007337c0,007339d0,00718f90,00718890,00733780,00733540

void __thiscall
FUN_00418df0(int *param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  int local_28;
  undefined1 local_1c [4];
  undefined1 local_18 [4];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009489a8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  param_1[0xe] = param_2;
  local_28 = FUN_00718810/*CResGFF::GetListCount*/(param_4);
  FUN_00733540(uVar1);
  local_8 = 0;
  local_14 = 0;
  do {
    if (local_28 <= local_14) {
LAB_00418efc:
      local_8 = 0xffffffff;
      FUN_00733780/*CExoString::~CExoString*/();
      ExceptionList = local_10;
      return;
    }
    FUN_00718890/*CResGFF::GetListElement*/(local_1c,param_4,local_14);
    FUN_00733570/*CExoString::CExoString_char*/(&DAT_00986925);
    local_8._0_1_ = 1;
    uVar2 = FUN_00718f90/*CResGFF::ReadFieldCExoString*/(local_38,local_1c,&DAT_00987888,local_18,local_30);
    local_8._0_1_ = 2;
    FUN_007337c0/*CExoString::operator=*/(uVar2);
    local_8._0_1_ = 1;
    FUN_00733780/*CExoString::~CExoString*/();
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780/*CExoString::~CExoString*/();
    iVar3 = FUN_007339d0(param_5);
    if (iVar3 != 0) {
      (**(code **)(*param_1 + 0x48))(param_3,local_1c);
      goto LAB_00418efc;
    }
    local_14 = local_14 + 1;
  } while( true );
}

```
