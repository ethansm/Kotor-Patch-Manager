## R027 FUNCTION CClientExoApp::GetInFreeLook  steam=0x00741290
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=108 ret_N=0
```c
// FN 00741290 FUN_00741290 size=108 ret=0 callers=004113a0 callees=0073f430

bool FUN_00741290(void)

{
  int iVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  
  iVar1 = FUN_0073f430();
  if ((iVar1 != 0) && (uVar2 = extraout_var, iVar1 = FUN_0073f430(), *(int *)(iVar1 + 0x40) != 0)) {
    iVar1 = FUN_0073f430();
    iVar1 = (**(code **)(**(int **)(iVar1 + 0x40) + 0x80))(0x106e,*(int **)(iVar1 + 0x40),uVar2);
    return iVar1 != 0;
  }
  return false;
}

```

## R028 FUNCTION CSWGuiPanel::SetVisible  steam=0x0040fe60
GOG build: param_bytes=4 ghidra_sig='CSWGuiPanel * this, int isVisible'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int isVisible) param_bytes=4
Steam function at this address: size=59 ret_N=4
```c
// FN 0040fe60 FUN_0040fe60 size=59 ret=4 callers=0090b360,007cbb40,0090e990,0090fb50,00913650,007be4c0,00908ae0,0090b430,009047c0,00908b50,00411ee0,00904780,00912790,009047f0,00919230 callees=00411170

void __thiscall FUN_0040fe60(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffffb | (param_2 & 1) << 2;
  if (*(int *)(param_1 + 0x1c) != 0) {
    FUN_00411170();
  }
  return;
}

```

## R029 FUNCTION Gob::EnableInViewVolume  steam=0x0045a000
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=41 ret_N=0
```c
// FN 0045a000 FUN_0045a000 size=41 ret=0 callers= callees=

void __fastcall FUN_0045a000(int *param_1)

{
  *(undefined1 *)((int)param_1 + 0x11) = 1;
  (**(code **)(*param_1 + 0x1b4))("propagateviewvolume",0);
  return;
}

```

## R030 FUNCTION CSWGuiBorder::GetInnerExtent  steam=0x00414d80
GOG build: param_bytes=4 ghidra_sig='CSWGuiBorder * this, CSWGuiExtent * param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* outExtent) param_bytes=4
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

## R031 FUNCTION CGuiInGame::GetCanOrientInDialog  steam=0x007cf710
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD clientId) param_bytes=4
Steam function at this address: size=112 ret_N=4
```c
// FN 007cf710 FUN_007cf710 size=112 ret=4 callers=007c41f0 callees=00521060,007a1720,0058c370

undefined4 __thiscall FUN_007cf710(undefined4 param_1,int param_2)

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

## R032 FUNCTION CSWCObject::GetServerObject  steam=0x007f2540
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=208 ret_N=0
```c
// FN 007f2540 FUN_007f2540 size=208 ret=0 callers=0087f360,0077cf70,00742a00,00752760,00842d00,0077d800,0077e370,0087be80,007fee70,0074fb20,00798d70 callees=0051c8b0,0051c0a0,0054a8e0

undefined4 __fastcall FUN_007f2540(int param_1)

{
  undefined4 uVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  if (*(int *)(param_1 + 0xf8) == 0) {
    if (*(int *)(param_1 + 0xe4) != 0) {
      return 0;
    }
    if (*(int *)(DAT_00a1b4a4 + 8) == 0) {
      return 0;
    }
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar1 = FUN_0051c8b0/*CServerExoApp::ClientToServerObjectId*/(*(undefined4 *)(param_1 + 4));
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    piVar2 = (int *)FUN_0051c0a0/*CServerExoApp::GetGameObject*/(uVar1);
    if (piVar2 != (int *)0x0) {
      uVar3 = (**(code **)(*piVar2 + 0x10))(param_1,uVar4,uVar3);
      *(undefined4 *)(param_1 + 0xf8) = uVar3;
      if (*(int *)(param_1 + 0xf8) != 0) {
        FUN_0054a8e0(param_1);
      }
    }
  }
  return *(undefined4 *)(param_1 + 0xf8);
}

```

## R033 FUNCTION CSWCObject::AnimationAttack  steam=0x007ef880
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=129 ret_N=4
```c
// FN 007ef880 FUN_007ef880 size=129 ret=4 callers=00777e30 callees=0071b8b0

bool __thiscall FUN_007ef880(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe4c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}

```

## R034 FUNCTION CSWGuiManager::PlayGuiSound  steam=0x004122a0
GOG build: param_bytes=1 ghidra_sig='undefined1 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, byte soundId) param_bytes=4
Steam function at this address: size=94 ret_N=4
```c
// FN 004122a0 FUN_004122a0 size=94 ret=4 callers=00760050,007d8ff0,008b6010,0088f5a0,009158d0,0088d700,008b2850,007d7730,008d5e20,007ccf30,008fbe00,007ca060,008ba940,008dac20,008b7700,0075f630,007d2bc0,007d7790,007469f0,00418750,008d4ac0,00889ac0,007cd510,0088f490,008d9d60,008af2d0,008ba700,00914a90,008e11c0,00901590,008c16a0,008c9380,0088f070,008e0a80,00887b10,0089c350,007b12c0,008591d0,007523e0,008d98f0,007da660,00916b30,008dea60,00898010,008e2460,008b3500,008ddf20,008ceb00,008df550,007cb920,00886e10,008c1fc0,0089de70,007d7b30,00410530,0088d5d0,007c9df0,0079d230,007dccd0,00901310,008abe70,007d7f10,007dbb10,0088f280,008c73f0,008a2070,008bed80,008b74b0,00752450,008df5b0,0041e870,00919190,00894e90,007dbaa0,007445d0,00917c80,0089ed90,008b6930,00744410,008e1140,008bf0e0,0084f900,0089cc50,008d4680,007ccd40,00903ed0,007d5580,007525d0,0041b220,00908920,00744260,008c9460,008d4420,007d3bb0,007db680,007ca730,008cfc10,0089eb50,0079d070,0090e850,008cdb00,0041a110,008aed10,008f6220,00889c00,008517f0,0041d940,00751750,0075d950,007d7e90,0088f180,008898a0,008bfce0,008c7330,008a3660,0088f890,007d7290,0089fbd0,0090b200,008d4f60,00912640,008da9b0,008fd7c0,00759e20,008cfd10,008fabf0,008d6090,007fbd90,007b3ca0,007d7a30,008839c0,00418fe0,009007a0,0040ec00,008fa6a0,008d95e0,008a8880,0075b8e0,007ca920 callees=0070ad50

void __thiscall FUN_004122a0(int param_1,char param_2)

{
  if ((((param_2 != -1) && (param_2 < *(char *)(param_1 + 0xa4))) && (*(int *)(param_1 + 0xa0) != 0)
      ) && (*(int *)(*(int *)(param_1 + 0xa0) + param_2 * 4) != 0)) {
    FUN_0070ad50();
  }
  return;
}

```

## R035 FUNCTION CResGFF::ReadFieldSHORT  steam=0x00718b80
GOG build: param_bytes=14 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined2 param_4'
Wrapper typedef (what calling code expects): short (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label, int* success, short defaultValue) param_bytes=16
Steam function at this address: size=125 ret_N=16
```c
// FN 00718b80 FUN_00718b80 size=125 ret=16 callers=00637ce0,00630f20,0058d5e0,005ef570,0061a210,006afed0 callees=00717770,007178e0

undefined2 FUN_00718b80(int param_1,int param_2,undefined4 *param_3,undefined2 param_4)

{
  undefined4 uVar1;
  int *piVar2;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 3)) {
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

## R036 FUNCTION Gob::SetMotionBlurred  steam=0x00459970
GOG build: param_bytes=1 ghidra_sig='undefined1 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, bool motionBlurred) param_bytes=4
Steam function at this address: size=25 ret_N=4
```c
// FN 00459970 FUN_00459970 size=25 ret=4 callers= callees=

void __thiscall FUN_00459970(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x181) = param_2;
  return;
}

```

## R037 FUNCTION CSWSPlaceable::GetLightIsOn  steam=0x005f6570
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=177 ret_N=0
```c
// FN 005f6570 FUN_005f6570 size=177 ret=0 callers=00528910,005f18c0,005ef570,00687710 callees=0071bcf0,00733570,00733780

undefined4 FUN_005f6570(void)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int extraout_var;
  int iVar4;
  undefined1 local_20 [11];
  char local_15;
  undefined1 local_14 [4];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009546c8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570/*CExoString::CExoString_char*/("LightColor");
  local_8 = 0;
  uVar3 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x30);
  iVar4 = extraout_var;
  iVar2 = FUN_0071bcf0(*(undefined4 *)(extraout_var + 0x270),local_20,local_14);
  local_15 = '\x01' - (iVar2 != 0);
  local_8 = 0xffffffff;
  FUN_00733780/*CExoString::~CExoString*/(uVar1,iVar4,uVar3);
  if (local_15 == '\0') {
    uVar3 = *(undefined4 *)(iVar4 + 0x3e4);
  }
  else {
    uVar3 = 0;
  }
  ExceptionList = local_10;
  return uVar3;
}

```

## R038 OFFSET CResGFF.labels = 0x54
Claim: GetFieldByLabel 0x7178e0 indexes [this+0x54] with stride 0x10 (16-char labels) bounded by header+0x1c
Decompile 0x007178e0:
```c
// FN 007178e0 FUN_007178e0/*CResGFF::GetFieldByLabel*/ size=269 ret=8 callers=00719800,00718b80,007190e0,00718810,00718a00,00718c00,00718a80,00718710,00718890,00718d80,00718e40,00718b00,00718d00,007193d0,00718960,00718f90,00719640,00718760,00719520,00718c80 callees=007186c0,0091d0f0,00717a00,0091c630,00717720,009196ee

void __thiscall FUN_007178e0/*CResGFF::GetFieldByLabel*/(int param_1,undefined4 param_2,char *param_3)

{
  undefined4 uVar1;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  uint local_1c;
  uint local_18;
  uint local_14;
  int *local_10;
  int local_c;
  int *local_8;
  
  local_1c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if ((param_3 != (char *)0x0) && (*(int *)(param_1 + 0x54) != 0)) {
    _memset(&local_2c,0,0x10);
    _strncpy((char *)&local_2c,param_3,0x10);
    uVar1 = FUN_00717720(param_2);
    local_18 = FUN_007186c0/*CResGFF::GetFieldCount*/(uVar1);
    local_14 = 0;
    while ((((local_14 < local_18 && (local_c = FUN_00717a00/*CResGFF::GetField_2*/(uVar1,local_14), local_c != 0)) &&
            (*(uint *)(local_c + 4) < *(uint *)(*(int *)(param_1 + 0x40) + 0x1c))) &&
           (((local_10 = (int *)(*(int *)(local_c + 4) * 0x10 + *(int *)(param_1 + 0x54)),
             local_8 = local_10, local_2c != *local_10 || (local_28 != local_10[1])) ||
            ((local_24 != local_10[2] || (local_20 != local_10[3]))))))) {
      local_14 = local_14 + 1;
    }
  }
  __security_check_cookie(local_1c ^ (uint)&stack0xfffffffc);
  return;
}

```

## R039 FUNCTION CSWGuiBorderParams::SetFillImage  steam=0x00414840
GOG build: param_bytes=8 ghidra_sig='CSWGuiBorderParams * this, CResRef * param_1, int forceUpdate'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* image, int forceUpdate) param_bytes=8
Steam function at this address: size=219 ret_N=8
```c
// FN 00414840 FUN_00414840 size=219 ret=8 callers=0074d520,0075b370,008850b0,00414920,0088d700,00885770,0088aac0,007425f0,00854a00,00747210,008aadc0,0088a0b0,00906570,00852380,00745e60,00752760,0088c7d0,0040f390,0075bb50,008cd8e0,0075cc60,008b1250,00415080,00753850,0075dd10,00910490,008f4af0,008d0780,00753780,008973d0,0074fb20,0074ee90,0074e7f0,008dddd0,00904a80,0040ee40,00756b70,00889dd0,0089e000,008b3c00,008ad930,008857a0,00760f40,00753d60,00918420,007418a0,00885fe0,00750c10,008a5dd0,0075ca60 callees=00710ca0,00710a70,00710a10,0047eb60,00710980,009196ee

void __thiscall FUN_00414840(int param_1,undefined4 param_2,int param_3)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_2c [16];
  undefined1 local_1c [20];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  iVar2 = FUN_00710a70(param_1 + 0x44);
  if (((iVar2 != 0) || (param_3 != 0)) &&
     (FUN_00710ca0(local_2c,param_2), *(int *)(param_1 + 0x54) != 0)) {
    if (*(int *)(*(int *)(param_1 + 0x54) + 0x74) != 0) {
      piVar1 = *(int **)(*(int *)(param_1 + 0x54) + 0x74);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 4))(1);
      }
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x74) = 0;
    }
    iVar2 = FUN_00710a10();
    if (iVar2 != 0) {
      FUN_00710980(local_1c);
      uVar3 = FUN_0047eb60(local_1c);
      *(undefined4 *)(*(int *)(param_1 + 0x54) + 0x74) = uVar3;
    }
  }
  __security_check_cookie(local_8 ^ (uint)&stack0xfffffffc);
  return;
}

```

## R040 FUNCTION CGuiInGame::ShowSoloModeQuery  steam=0x007cd510
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int doStealth) param_bytes=4
Steam function at this address: size=578 ret_N=4
```c
// FN 007cd510 FUN_007cd510 size=578 ret=4 callers=00751fa0,007b12c0,0077b020 callees=0073f450,00740020,0073f430,0077d800,00410800,007aae00,0073f750,004122a0,0073fb90,00410530,0073f810,008b9120,0087a730,0073fee0

void __thiscall FUN_007cd510(int param_1,undefined4 param_2)

{
  bool bVar1;
  bool bVar2;
  short sVar3;
  int *piVar4;
  int iVar5;
  int *local_14;
  int local_10;
  
  if (*(int *)(param_1 + 0x128) == 0) {
    return;
  }
  if (*(int *)(param_1 + 0x38) == 0) {
    return;
  }
  if (*(int *)(param_1 + 0x94) == 0) {
    return;
  }
  piVar4 = (int *)FUN_0073fb90/*ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)*/();
  bVar2 = false;
  if (piVar4 != (int *)0x0) {
    bVar2 = 1 < *piVar4;
  }
  iVar5 = FUN_0073f450/*CClientExoApp::GetPlayerCreature*/();
  local_14 = (int *)0x0;
  if (iVar5 != 0) {
    local_14 = (int *)FUN_0077d800();
  }
  local_10 = 0;
  iVar5 = FUN_0073f430();
  if ((iVar5 != 0) && (iVar5 = FUN_0073f430(), *(int *)(iVar5 + 0x48) != 0)) {
    local_10 = FUN_007aae00();
  }
  iVar5 = FUN_0073f750();
  if ((*(int *)(iVar5 + 0xcc) == 0) &&
     (((local_10 == 0 || (*(int *)(local_10 + 0x2f8) == 0)) && (iVar5 = FUN_00740020(), iVar5 == 0))
     )) {
    if (local_14 == (int *)0x0) goto LAB_007cd6b3;
    iVar5 = (**(code **)(*local_14 + 0x94))();
    if (iVar5 == 0) {
      if (local_14[0x46b] == 0) {
        bVar1 = false;
      }
      else {
        sVar3 = (**(code **)(*local_14 + 0x9c))(0);
        bVar1 = sVar3 < 1;
      }
      if (!bVar1) goto LAB_007cd6b3;
    }
  }
  bVar2 = false;
LAB_007cd6b3:
  if (bVar2) {
    if (*(int *)(param_1 + 0xf18) == 0) {
      FUN_0073f810();
      FUN_0087a730();
    }
    FUN_008b9120(param_2);
    iVar5 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x94));
    if (iVar5 == 0) {
      FUN_00410530/*CSWGuiManager::AddPanel*/(*(undefined4 *)(param_1 + 0x94),1,1);
    }
    FUN_0073fee0/*CClientExoApp::SetInputClass*/(2,1);
  }
... (6 more lines)
```

## R041 FUNCTION CClientExoApp::GetInGameGui  steam=0x0073f750
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=28 ret_N=0
```c
// FN 0073f750 FUN_0073f750 size=28 ret=0 callers=00853a00,0090b6b0,008301e0,008abe70,008faca0,005d75b0,0077c780,008a2070,00908540,008abb30,00528710,0056d6f0,005676d0,00783ee0,00757ba0,008ba520,0084f900,00617b40,007fbce0,008a8640,008954f0,00919430,008079b0,00756930,008a3a80,008aaea0,0075a310,008c7330,007fc290,00696b00,009152f0,0088ed70,00413dd0,007e5120,007a6760,008a87f0,0090ede0,006c9900,0077a280,00647b80,00889c00,00798820,00757a80,0077cf70,007fe6c0,008925f0,00699e50,007568b0,005dfbe0,0075a7c0,008928f0,0089ea20,00757ae0,006b6bd0,0089b100,008117d0,008995f0,007779b0,0077b630,0082acb0,00699690,008dae10,007ca730,0066df70,00912d40,0069ac00,008d94d0,006cae20,005fb1a0,007f67f0,00796da0,0084fe20,008b98f0,0074c320,008953b0,00534600,007ca060,007b12c0,00882230,008d5ff0,007a2fb0,008bf0e0,005fbf60,00546aa0,007eeee0,00412400,00698980,007608a0,005fdfb0,0075db70,008898a0,0077d890,00751fa0,0075a630,0065e670,006cd5b0,00692e50,0084b630,00811450,007b40f0,00802f00,009135c0,00544ad0,007cb7c0,0089ed90,007d0f50,00854440,008ba420,00883560,008c2140,00898010,008805a0,00546ad0,00757a50,008ceb00,00903df0,0068d7d0,007d3f40,00831c60,006949f0,006c9770,008a5790,005cb430,00760aa0,0052dd00,0074ee90,006cac90,0066c160,008a8760,00908b50,006341d0,0063e7e0,00815d00,00908460,006cc140,0052b380,0069c020,007cfbf0,008bb8d0,007664b0,007a5fa0,006969e0,008a5850,00698750,007fc880,007ac360,00912840,00699760,00692ce0,008be740,00908cc0,00692c80,00751580,008c7300,006c49d0,009165b0,008bb260,008bff20,0090ae20,008bfaa0,00903f40,008a8880,005da4d0,00697020,007e5df0,00830fa0,0068b930,008a6170,0069aec0,0061f170,008bc100,007cbf60,007445d0,00757b70,00699720,008328f0,0066f480,0069a1f0,00697460,0089a760,0052fda0,007ef780,0084fcd0,008ab060,008aed10,0080f870,008a5680,007ac470,0079f010,00916520,008a86d0,0075d950,0056e620,00698070,008df5b0,006e1540,007fcf00,0088a5c0,008b7660,00597d90,0090ad90,00698930,008529e0,0069a0c0,007bcab0,0077b020,0082d950,00760050,006ca9f0,00698100,00832a50,005f5590,008519f0,008a2010,0053af20,006995a0,00880510,008b8ae0,008b0890,008c21a0,00813200,008a57d0,00908e30,008a5810,009084f0,00753680,008ce300,00569320,008c18e0,0090fb00,00850010,00586940,00881dd0,0066dd30,008a3660,005dab80,00899560,008d95e0,00533830,00757b40,00751750,00842bd0,007c41f0,006010b0,0089e000,008b2850,0058a2c0,008bc3e0,008b7910,00545760,0068ae60,008995a0,008c1930,007b3ca0,008a07d0,008b6930,008c0c80,0080efb0,008d6280,006986e0,008b9040,0088f890,006c8de0,0087f530,008a1f70,008bed80,00757ab0,00669910,008548a0,007fbd90,0075cdc0,0090aea0,00887b10,00854810,007cd510,0082c980,00746780,007dea10,0087c9f0,008578b0,00757b10,008858c0,0068ad70,0079c5b0,00841ce0,00746670,007ed9a0,007fae60,008dab80,007d5d50,0089eb50,008a8480,008b61c0,00894e90,0089de70,005221b0,007c9df0,0069bea0,008a8cf0,0068db00,008b6010,007ceb80,00879e80,007568f0,0079d230,006c9c00,008df550,006c53f0,008d98f0,006977f0,0075a4a0,008cfa60,00781be0,0066ad10,008d4ac0,008c1fc0,006ce710,008a8df0,00796ba0,00699df0,008bfa40,005fdea0,005f5770,008528b0,0077d120,008a8b30,0090fa70,008ba330,00783dc0,00745d90,006971f0,0054a9c0,008f82c0,008b8ee0,00759e20,008abd30,0090b890,00545520,0075a1f0,0088d5d0,0089fbd0,007e5ca0,007e6ce0,008d4680,008a53d0,00915380,00760920,008f8070,008545c0,00687230,00903ba0,00699380,007fccb0,008a3c60,00699db0,0066cb50,005b96f0,0090b430,005467b0,00693c10,0075dd10 callees=

undefined4 __fastcall FUN_0073f750(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x40);
}

```

## R042 FUNCTION CClientExoApp::GetCreditSequenceInProgress  steam=0x00741070
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 00741070 FUN_00741070 size=31 ret=0 callers=0070dde0 callees=

undefined4 __fastcall FUN_00741070(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x4c0);
}

```

## R043 FUNCTION CGuiInGame::FadeAndStopDialogAmbientTrack  steam=0x007d0ec0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=142 ret_N=0
```c
// FN 007d0ec0 FUN_007d0ec0 size=142 ret=0 callers=008925f0,007a5fa0 callees=0070b080,009196fd,00741010

void __fastcall FUN_007d0ec0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0xfbc) != 0) {
    FUN_00741010(*(undefined4 *)(param_1 + 0xfbc),500);
    iVar1 = *(int *)(param_1 + 0xfbc);
    if (iVar1 != 0) {
      FUN_0070b080();
      FUN_009196fd/*operator delete*/(iVar1);
    }
    *(undefined4 *)(param_1 + 0xfbc) = 0;
  }
  return;
}

```

## R044 FUNCTION Gob::AddAttachment  steam=0x004632c0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* attachment) param_bytes=4
Steam function at this address: size=55 ret_N=4
```c
// FN 004632c0 FUN_004632c0 size=55 ret=4 callers=00504810 callees=00473f40,00476620

void FUN_004632c0(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_00476620(param_1);
  if (iVar1 == 0) {
    FUN_00473f40(param_1);
  }
  return;
}

```

## R045 FUNCTION CClientExoApp::IsSoundPlayingInDialog  steam=0x00740290
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 00740290 FUN_00740290 size=22 ret=0 callers=00546a50 callees=0079bcc0

void __fastcall FUN_00740290(undefined4 param_1)

{
  FUN_0079bcc0(param_1);
  return;
}

```

## R046 FUNCTION CSWSObject::GetNearestObjectByName  steam=0x00546bd0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr, void* name, float radius) param_bytes=8
Steam function at this address: size=468 ret_N=8
```c
// FN 00546bd0 FUN_00546bd0 size=468 ret=8 callers=006cac90,006c8de0,006c9c00,006cae20,006c7a30,006ca9f0 callees=005453c0,0051c080,004452b0,00733a60,00734d60,0091cb24,0053dfb0,005232b0,00523250,0044b8a0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00546bd0(int param_1,undefined4 param_2,float param_3)

{
  char cVar1;
  int iVar2;
  float10 fVar3;
  undefined8 uVar4;
  uint local_68;
  undefined1 local_4c [12];
  float local_40;
  uint local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  float local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_38 = 0;
  local_34 = 0;
  local_30 = 0;
  local_2c = 0x7f000000;
  local_68 = (uint)(param_3 != (float)_DAT_00986b38);
  local_3c = local_68;
  local_28 = param_3 * param_3;
  local_14 = 0;
  local_10 = 0;
  local_c = 0;
  local_24 = FUN_005453c0();
  if ((local_24 != 0) && (iVar2 = FUN_00733a60(), iVar2 == 0)) {
    local_38 = *(undefined4 *)(param_1 + 0x94);
    local_34 = *(undefined4 *)(param_1 + 0x98);
    local_30 = *(undefined4 *)(param_1 + 0x9c);
    local_1c = FUN_00523250();
    while (local_1c == 1) {
      uVar4 = CONCAT44(&local_18,local_8);
      FUN_0051c080(local_8,&local_18);
      cVar1 = FUN_0053dfb0/*CGameObjectArray::GetGameObject*/(uVar4);
      if (cVar1 == DAT_0099447f) {
        local_20 = local_18;
        local_14 = *(undefined4 *)(local_18 + 0x94);
        local_10 = *(undefined4 *)(local_18 + 0x98);
        local_c = *(undefined4 *)(local_18 + 0x9c);
        iVar2 = FUN_005453c0();
        if ((iVar2 == local_24) && (iVar2 = FUN_00734d60(), iVar2 != 0)) {
          FUN_004452b0(local_4c,&local_14);
          fVar3 = (float10)FUN_0044b8a0(local_4c);
          local_40 = (float)fVar3;
          if ((local_3c == 0) || ((local_40 <= local_28 || (*(int *)(local_20 + 0x260) != 0)))) {
            fVar3 = (float10)FUN_0091cb24((double)local_40);
            local_28 = (float)fVar3;
            local_2c = *(undefined4 *)(local_20 + 4);
            local_3c = 1;
          }
        }
        local_1c = FUN_005232b0();
      }
      else {
        local_1c = FUN_005232b0();
... (6 more lines)
```

## R047 FUNCTION CResGFF::ReadFieldINT  steam=0x00718c80
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label, int* success, int defaultValue) param_bytes=16
Steam function at this address: size=122 ret_N=16
```c
// FN 00718c80 FUN_00718c80 size=122 ret=16 callers=00625360,00420180,005e2900,0062db90,006fef80,0053fd50,00528710,005753f0,00631260,0041bb70,006c67e0,00630810,006c53f0,005e2090,005f7cc0,00415080,00572a70,0053e580,0058d5e0,00620250,00416720,0041c870,006c75b0,005285b0,005205f0,00703530,004188e0,006c7400,005e8ee0,00585b00,0041ffd0,00833ff0,008bf280,006d0db0,005e5510,0062af70,0041b030,005409f0,00418f20,0061a210,00574180,005582f0,006afed0,005fd040,00601740,00592840,00637ce0,0058ca50,005fbf60,005e6f80,00417480,00630f20,0040e2c0,00523870,005ef570 callees=00717770,007178e0

int FUN_00718c80(int param_1,int param_2,undefined4 *param_3,int param_4)

{
  undefined4 uVar1;
  int *piVar2;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 5)) {
      *param_3 = 0;
    }
    else {
      *param_3 = 1;
      param_4 = piVar2[2];
    }
  }
  return param_4;
}

```

## R048 FUNCTION Gob::RemoveAttachment  steam=0x00463300
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* attachment) param_bytes=4
Steam function at this address: size=53 ret_N=4
```c
// FN 00463300 FUN_00463300 size=53 ret=4 callers=00504b60 callees=00449170,00476620

void FUN_00463300(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_00476620(param_1);
  if (iVar1 != 0) {
    FUN_00449170(param_1);
  }
  return;
}

```

## R049 FUNCTION CSWSTrigger::GetTargetArea  steam=0x0061e360
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=480 ret_N=0
```c
// FN 0061e360 FUN_0061e360 size=480 ret=0 callers= callees=0051c370,0051ad20,005d56b0,0051af60,0051c100,00736220,00622640,00476620,006fd8d0,0051c8f0,005e13c0

void __fastcall FUN_0061e360(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  undefined8 uVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  undefined1 *puVar8;
  int *piVar9;
  uint *puVar10;
  undefined4 uVar11;
  uint local_14;
  int local_10;
  undefined1 local_c [4];
  int local_8;
  
  uVar4 = FUN_00736220();
  piVar9 = &local_8;
  piVar7 = &local_10;
  FUN_0051c370/*CServerExoApp::GetWorldTimer*/(piVar7,piVar9);
  FUN_0051ad20/*CWorldTimer::GetWorldTime*/(piVar7,piVar9);
  puVar10 = &local_14;
  puVar8 = local_c;
  iVar2 = param_1[0xd0];
  iVar3 = param_1[0xd1];
  iVar5 = local_10;
  iVar6 = local_8;
  FUN_0051c370/*CServerExoApp::GetWorldTimer*/(local_10,local_8,iVar3,iVar2,puVar8,puVar10);
  FUN_0051af60/*CWorldTimer::SubtractWorldTimes*/(iVar5,iVar6,iVar3,iVar2,puVar8,puVar10);
  if (5999 < local_14) {
    uVar11 = 1;
    iVar2 = param_1[1];
    uVar1 = FUN_00622640(0);
    FUN_006fd8d0(uVar1,iVar2,uVar11);
    param_1[0xd1] = local_10;
    param_1[0xd0] = local_8;
  }
  if (param_1[0xbf] != 0) {
    uVar1 = FUN_0051c8f0();
    iVar2 = FUN_0051c100/*CServerExoApp::GetCreatureByGameObjectID*/(uVar1);
    if ((param_1[0xc2] == 0) &&
       ((iVar2 == 0 ||
        (((param_1[0xbe] != *(int *)(*(int *)(iVar2 + 0x1198) + 0x78) &&
          (iVar3 = FUN_005e13c0(*(undefined4 *)(iVar2 + 4),0), iVar3 < 0x5a)) &&
         (iVar2 = FUN_00476620(*(undefined4 *)(iVar2 + 4)), iVar2 == 0)))))) {
      if (param_1[0x36] != 0x279f) {
        (**(code **)(*param_1 + 0x7c))(0x279f);
      }
    }
    else if (param_1[0x36] != 0x27a0) {
      (**(code **)(*param_1 + 0x7c))(0x27a0);
    }
  }
  FUN_005d56b0(local_10,local_8,uVar4);
  return;
}

```

## R050 FUNCTION Scene::SetFogRange  steam=0x00464c80
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, float fogStart, float fogEnd) param_bytes=8
Steam function at this address: size=31 ret_N=8
```c
// FN 00464c80 FUN_00464c80 size=31 ret=8 callers= callees=

void __thiscall FUN_00464c80(int param_1,undefined4 param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 100) = param_2;
  *(undefined4 *)(param_1 + 0x68) = param_3;
  return;
}

```

## R051 FUNCTION CResGFF::WriteFieldQuaternion  steam=0x0071a260
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, Quaternion* value, char* label) param_bytes=12
Steam function at this address: size=184 ret_N=12
```c
// FN 0071a260 FUN_0071a260 size=184 ret=12 callers=0052b380 callees=00717aa0,00718030,00718370

undefined4 __thiscall FUN_0071a260(int param_1,int param_2,undefined4 *param_3,int param_4)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 *puVar6;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar4 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    iVar5 = FUN_00718030/*CResGFF::AddField*/(param_2,param_4,0x10);
    if (iVar5 == 0) {
      uVar4 = 0;
    }
    else {
      puVar6 = (undefined4 *)FUN_00718370/*CResGFF::AddDataField*/(0xffffffff,0,0x10);
      if (puVar6 == (undefined4 *)0x0) {
        uVar4 = 0;
      }
      else {
        uVar4 = *param_3;
        uVar1 = param_3[1];
        uVar2 = param_3[2];
        uVar3 = param_3[3];
        *(int *)(iVar5 + 8) = (int)puVar6 - *(int *)(param_1 + 0x5c);
        *puVar6 = uVar4;
        puVar6[1] = uVar1;
        puVar6[2] = uVar2;
        puVar6[3] = uVar3;
        uVar4 = 1;
      }
    }
  }
  return uVar4;
}

```

## R052 FUNCTION CSWCObject::ClearAllQueuedCombatActions  steam=0x007f2500
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=54 ret_N=0
```c
// FN 007f2500 FUN_007f2500 size=54 ret=0 callers=00751750,007445d0 callees=0051cba0,0051c8b0

void __fastcall FUN_007f2500(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0/*CServerExoApp::ClientToServerObjectId*/(*(undefined4 *)(param_1 + 4));
  FUN_0051cba0(uVar1);
  return;
}

```

## R053 FUNCTION Scene::RemoveFromSceneManager_2  steam=0x00472460
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__cdecl)(void* thisPtr, void* object) param_bytes=8
Steam function at this address: size=71 ret_N=0
```c
// FN 00472460 FUN_00472460 size=71 ret=0 callers=0045a190 callees=00449170,00476620

void FUN_00472460(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_00476620(param_1);
  if (iVar1 == 0) {
    iVar1 = FUN_00476620(param_1);
    if (iVar1 != 0) {
      FUN_00449170(param_1);
    }
  }
  else {
    FUN_00449170(param_1);
  }
  return;
}

```
