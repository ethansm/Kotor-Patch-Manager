## R351 FUNCTION CResGFF::GetListCount  steam=0x00718810
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResList* list) param_bytes=4
Steam function at this address: size=124 ret_N=4
```c
// FN 00718810 FUN_00718810 size=124 ret=4 callers=00625360,005e2900,00525f90,0053fd50,00418df0,00528710,005753f0,00664cb0,005269e0,0069d1e0,00528080,006b3b70,006c67e0,005266f0,005e9370,006c53f0,005e2090,005f7cc0,006815e0,00527cb0,0058d5e0,00526f60,00655690,006295d0,00664800,0051eed0,00527330,005263c0,006029f0,00626850,00833ff0,0052ccd0,00573950,006d0db0,005e5510,0062af70,0083db30,00527580,005f9220,005409f0,00527890,00621900,00655b00,005582f0,006afed0,00601740,0055def0,00592840,00637ce0,005fbf60,005e6f80,00630f20,00626a50,005406e0,00523870,005ef570 callees=00717770,007178e0,00717880

undefined4 FUN_00718810(int param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  uint local_10 [2];
  int *local_8;
  
  if (param_1 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_1 + 4);
    local_8 = (int *)FUN_00717770(param_1,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 0xf)) {
      uVar1 = 0;
    }
    else {
      puVar2 = (undefined4 *)FUN_00717880/*CResGFF::GetDataLayoutList*/(local_8,local_10);
      if ((puVar2 == (undefined4 *)0x0) || (local_10[0] < 4)) {
        uVar1 = 0;
      }
      else {
        uVar1 = *puVar2;
      }
    }
  }
  return uVar1;
}

```

## R352 FUNCTION Scene::SetFogColor  steam=0x00464470
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, Vector* color) param_bytes=4
Steam function at this address: size=143 ret_N=4
```c
// FN 00464470 FUN_00464470 size=143 ret=4 callers= callees=0044b8a0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00464470(int param_1,undefined4 *param_2)

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

## R353 FUNCTION CGuiInGame::IsGlobalFading  steam=0x007cbc40
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=44 ret_N=0
```c
// FN 007cbc40 FUN_007cbc40 size=44 ret=0 callers=00781be0,007fe6c0,0079d550,005d75b0 callees=007bca60

undefined4 __fastcall FUN_007cbc40(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 0x6c) != 0) && (iVar1 = FUN_007bca60(), iVar1 == 0)) {
    return 1;
  }
  return 0;
}

```

## R354 FUNCTION CSWCObject::GetLoopingAnimation  steam=0x007ed980
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): WORD (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=18 ret_N=0
```c
// FN 007ed980 FUN_007ed980 size=18 ret=0 callers=007675b0,007f0300,00841440,0087b3f0,0076d200,00777e30,0080d050,00768060,008079b0 callees=

undefined2 __fastcall FUN_007ed980(int param_1)

{
  return *(undefined2 *)(param_1 + 0x58);
}

```

## R355 FUNCTION CClientExoApp::GetWorldTimer  steam=0x0073f390
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 0073f390 FUN_0073f390 size=31 ret=0 callers=0040a160,0040a750 callees=

undefined4 __fastcall FUN_0073f390(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0xe0);
}

```

## R356 FUNCTION Scene::DisableAnimations  steam=0x004651d0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=21 ret_N=0
```c
// FN 004651d0 FUN_004651d0 size=21 ret=0 callers= callees=

void __fastcall FUN_004651d0(int param_1)

{
  *(undefined1 *)(param_1 + 0xe6) = 0;
  return;
}

```

## R357 FUNCTION CClientExoApp::RunDeathSequence  steam=0x007403a0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 007403a0 FUN_007403a0 size=22 ret=0 callers=0053a7c0 callees=0079c0e0

void __fastcall FUN_007403a0(undefined4 param_1)

{
  FUN_0079c0e0(param_1);
  return;
}

```

## R358 FUNCTION CClientExoApp::GetAutoPaused  steam=0x00740960
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=26 ret_N=0
```c
// FN 00740960 FUN_00740960 size=26 ret=0 callers=00751ef0,00751750,008b9b20,005d75b0,0074ec10,007b12c0,007445d0 callees=

uint __fastcall FUN_00740960(int param_1)

{
  return *(uint *)(*(int *)(param_1 + 4) + 900) & 1;
}

```

## R359 FUNCTION Gob::Amputate  steam=0x00462170
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, char* partName) param_bytes=4
Steam function at this address: size=421 ret_N=4
```c
// FN 00462170 FUN_00462170 size=421 ret=4 callers= callees=0049b840,00463b20,00449170,00452cf0,00461ed0,00463ed0,00456b00,00473f40,00476620,004142e0

undefined4 __thiscall FUN_00462170(int param_1,undefined4 param_2)

{
  int iVar1;
  uint uVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  int iVar5;
  int local_28;
  int local_20;
  int local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094a128;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  puVar3 = (undefined4 *)FUN_00456b00(*(undefined4 *)(param_1 + 0x98),param_2,uVar2);
  if (puVar3 == (undefined4 *)0x0) {
    uVar4 = 0;
  }
  else {
    FUN_00463ed0(0);
    local_8 = 0;
    FUN_00473f40(puVar3);
    while (local_1c != 0) {
      iVar1 = *(int *)(local_20 + -4 + local_1c * 4);
      local_1c = local_1c + -1;
      for (local_28 = 0; local_28 < *(int *)(iVar1 + 0x34); local_28 = local_28 + 1) {
        FUN_00473f40(*(undefined4 *)(*(int *)(iVar1 + 0x30) + local_28 * 4));
      }
      iVar5 = FUN_00476620(iVar1);
      if (iVar5 != 0) {
        uVar4 = FUN_004142e0(iVar1);
        FUN_00452cf0(param_1);
        FUN_0049b840(uVar4);
        FUN_0049b840(uVar4);
      }
    }
    FUN_00449170(puVar3);
    puVar3[0xf] = 0;
    if (puVar3 == (undefined4 *)0x0) {
      uVar4 = 0;
    }
    else {
      uVar4 = (**(code **)*puVar3)(1);
    }
    FUN_00461ed0(uVar2,uVar4);
    local_8 = 0xffffffff;
    FUN_00463b20();
    uVar4 = 1;
  }
  ExceptionList = local_10;
  return uVar4;
}

```

## R360 FUNCTION CSWCObject::AttachmentFromServerObject  steam=0x007f2610
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* serverObject) param_bytes=4
Steam function at this address: size=25 ret_N=4
```c
// FN 007f2610 FUN_007f2610 size=25 ret=4 callers=0054a840 callees=

void __thiscall FUN_007f2610(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xf8) = param_2;
  return;
}

```

## R361 FUNCTION CGuiInGame::AddFloatyText  steam=0x007cee80
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, DWORD clientId, void* text, Vector* color, float duration) param_bytes=16
Steam function at this address: size=58 ret_N=16
```c
// FN 007cee80 FUN_007cee80 size=58 ret=16 callers=0079de60 callees=00750f30

void __thiscall
FUN_007cee80(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            )

{
  if (*(int *)(param_1 + 0x98) != 0) {
    FUN_00750f30(param_2,param_3,param_4,param_5);
  }
  return;
}

```

## R362 FUNCTION CSWSObject::SetKeepCorpse  steam=0x0054a7a0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int keepCorpse) param_bytes=4
Steam function at this address: size=153 ret_N=4
```c
// FN 0054a7a0 FUN_0054a7a0 size=153 ret=4 callers=0056efd0 callees=0058c370,0051c600,0051c8f0,007a1720,00561430

void __thiscall FUN_0054a7a0(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_8;
  
  uVar1 = FUN_0051c8f0();
  iVar2 = FUN_0051c600(uVar1);
  if (iVar2 != 0) {
    local_8 = FUN_007a1720();
    while (local_8 != 0) {
      iVar2 = FUN_00561430(local_8);
      if (*(int *)(iVar2 + 0x2c) == *(int *)(param_1 + 4)) {
        *(undefined4 *)(iVar2 + 0x158) = param_2;
        return;
      }
      FUN_0058c370(&local_8);
    }
  }
  *(undefined4 *)(param_1 + 0x234) = param_2;
  return;
}

```

## R363 OFFSET CSWGuiBorderParams.edge_image_resref = 0x34
Claim: SetEdgeImage 0x414760 uses params+0x34; operator= copies src+0x34; Load tag EDGE
Decompile 0x00414760:
```c
// FN 00414760 FUN_00414760/*CSWGuiBorderParams::SetEdgeImage*/ size=219 ret=8 callers=00415080,00414920,00747210 callees=00710ca0,00710a70,00710a10,0047eb60,00710980,009196ee

void __thiscall FUN_00414760/*CSWGuiBorderParams::SetEdgeImage*/(int param_1,undefined4 param_2,int param_3)

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

## R364 FUNCTION CSWSTrigger::InTrigger  steam=0x0061fc60
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, Vector* point) param_bytes=4
Steam function at this address: size=1209 ret_N=4
```c
// FN 0061fc60 FUN_0061fc60 size=1209 ret=4 callers=0067eb50,0054d180,0061e5f0,00585cc0,00656af0 callees=0044b8a0,0091ebad,004452b0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool __thiscall FUN_0061fc60(int param_1,float *param_2)

{
  float fVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  bool bVar5;
  float10 fVar6;
  double dVar7;
  double dVar8;
  int local_58;
  int local_18;
  uint local_14;
  undefined1 local_10 [12];
  
  if (*(int *)(param_1 + 0x2fc) == 0) {
    local_14 = 0;
    for (local_18 = 0; local_18 < *(int *)(param_1 + 0x2d4); local_18 = local_18 + 1) {
      iVar3 = *(int *)(*(int *)(param_1 + 0x2d8) + local_18 * 4);
      if (local_18 == *(int *)(param_1 + 0x2d4) + -1) {
        local_58 = 0;
      }
      else {
        local_58 = local_18 + 1;
      }
      iVar4 = *(int *)(*(int *)(param_1 + 0x2d8) + local_58 * 4);
      if (*(float *)(*(int *)(param_1 + 0x2c8) + iVar3 * 0xc) !=
          *(float *)(*(int *)(param_1 + 0x2c8) + iVar4 * 0xc)) {
        fVar2 = *(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar3 * 0xc) -
                (*(float *)(*(int *)(param_1 + 0x2c8) + iVar3 * 0xc) - *param_2) *
                ((*(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar4 * 0xc) -
                 *(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar3 * 0xc)) /
                (*(float *)(*(int *)(param_1 + 0x2c8) + iVar4 * 0xc) -
                *(float *)(*(int *)(param_1 + 0x2c8) + iVar3 * 0xc)));
        fVar1 = *param_2;
        if (param_2[1] < fVar2) {
          dVar7 = _fabs((double)(*(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar4 * 0xc) -
                                *(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar3 * 0xc)));
          dVar8 = _fabs((double)(*(float *)(*(int *)(param_1 + 0x2c8) + iVar4 * 0xc) -
                                *(float *)(*(int *)(param_1 + 0x2c8) + iVar3 * 0xc)));
          if ((float)dVar7 <= (float)dVar8) {
            if (*(float *)(*(int *)(param_1 + 0x2c8) + iVar4 * 0xc) <=
                *(float *)(*(int *)(param_1 + 0x2c8) + iVar3 * 0xc)) {
              if (((*(float *)(*(int *)(param_1 + 0x2c8) + iVar4 * 0xc) <
                    *(float *)(*(int *)(param_1 + 0x2c8) + iVar3 * 0xc)) &&
                  (*(float *)(*(int *)(param_1 + 0x2c8) + iVar4 * 0xc) < fVar1)) &&
                 (fVar1 <= *(float *)(*(int *)(param_1 + 0x2c8) + iVar3 * 0xc))) {
                local_14 = local_14 + 1;
              }
            }
            else {
              fVar2 = *(float *)(*(int *)(param_1 + 0x2c8) + iVar3 * 0xc);
              if ((fVar2 < fVar1 != (fVar2 == fVar1)) &&
                 (fVar1 < *(float *)(*(int *)(param_1 + 0x2c8) + iVar4 * 0xc))) {
                local_14 = local_14 + 1;
              }
            }
          }
          else if (*(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar4 * 0xc) <=
                   *(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar3 * 0xc)) {
            if (((*(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar4 * 0xc) <
                  *(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar3 * 0xc)) &&
                (*(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar4 * 0xc) < fVar2)) &&
               (fVar2 <= *(float *)(*(int *)(param_1 + 0x2c8) + 4 + iVar3 * 0xc))) {
              local_14 = local_14 + 1;
... (33 more lines)
```

## R365 FUNCTION CClientExoApp::AddMoveToModuleMovie  steam=0x00740180
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* movie) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 00740180 FUN_00740180 size=28 ret=4 callers=005221b0,008529e0,00693c10 callees=00798b70

void FUN_00740180(undefined4 param_1)

{
  FUN_00798b70(param_1);
  return;
}

```

## R366 FUNCTION CGuiInGame::AppendToMsgBuffer  steam=0x007be090
GOG build: param_bytes=9 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined1 param_3'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* messageString, DWORD messageType, BYTE messageColor) param_bytes=12
Steam function at this address: size=282 ret_N=12
```c
// FN 007be090 FUN_007be090 size=282 ret=12 callers=005fbf60,00802f00,00617b40,0078f160,008301e0,00759e20,00830fa0,006c8de0,0082d950 callees=00733b60,007337c0

void __thiscall FUN_007be090(int param_1,undefined4 param_2,undefined4 param_3,undefined1 param_4)

{
  int iVar1;
  int iVar2;
  ushort local_8;
  
  iVar1 = FUN_00733b60(&DAT_00986925);
  if (iVar1 != 0) {
    if (0x3f < *(uint *)(param_1 + 0x11c)) {
      *(undefined4 *)(param_1 + 0x11c) = 0x3f;
      for (local_8 = 0; (uint)local_8 < *(uint *)(param_1 + 0x11c); local_8 = local_8 + 1) {
        iVar1 = (local_8 + 1) * 0x10 + *(int *)(param_1 + 0x110);
        iVar2 = (uint)local_8 * 0x10 + *(int *)(param_1 + 0x110);
        FUN_007337c0/*CExoString::operator=*/(iVar1);
        *(undefined4 *)(iVar2 + 8) = *(undefined4 *)(iVar1 + 8);
        *(undefined1 *)(iVar2 + 0xc) = *(undefined1 *)(iVar1 + 0xc);
      }
    }
    FUN_007337c0/*CExoString::operator=*/(param_2);
    *(undefined4 *)(*(int *)(param_1 + 0x110) + 8 + *(int *)(param_1 + 0x11c) * 0x10) = param_3;
    *(undefined1 *)(*(int *)(param_1 + 0x110) + 0xc + *(int *)(param_1 + 0x11c) * 0x10) = param_4;
    *(int *)(param_1 + 0x11c) = *(int *)(param_1 + 0x11c) + 1;
  }
  return;
}

```

## R367 FUNCTION CClientExoApp::GetPausedByCombat  steam=0x00740330
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__stdcall)() param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 00740330 FUN_00740330 size=22 ret=0 callers=0085cc00,007ca920,005d75b0,0074c320,008f8070,0058e990,007b12c0,00746670,0077fd90,00899120,0085b940,008775e0,00751ef0,007ccd40,007ca730,007cfbf0,007664b0,0074d520,008b9b20,0089fbd0,007ccf30,0085a9e0,00753850 callees=0079bf00

void __fastcall FUN_00740330(undefined4 param_1)

{
  FUN_0079bf00(param_1);
  return;
}

```

## R368 FUNCTION CWorldTimer::PauseWorldTimer  steam=0x0051b050
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=66 ret_N=0
```c
// FN 0051b050 FUN_0051b050 size=66 ret=0 callers=00784740,00883560,00853a00,00530170,007986a0,00538ed0,00781be0 callees=0051ad20

void __fastcall FUN_0051b050(int param_1)

{
  undefined4 local_c;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x24) == 0) {
    FUN_0051ad20/*CWorldTimer::GetWorldTime*/(&local_c,&local_8);
    *(undefined4 *)(param_1 + 0x28) = local_c;
    *(undefined4 *)(param_1 + 0x2c) = local_8;
    *(undefined4 *)(param_1 + 0x24) = 1;
  }
  return;
}

```

## R369 FUNCTION CSWCObject::AnimationDialog  steam=0x007ef780
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD animationRowIndex) param_bytes=4
Steam function at this address: size=104 ret_N=4
```c
// FN 007ef780 FUN_007ef780 size=104 ret=4 callers=007779b0,007cadb0,00776e40,00776fe0 callees=007cb010,007cae60,0073f750

undefined4 FUN_007ef780(ushort param_1)

{
  int iVar1;
  uint uVar2;
  
  uVar2 = (uint)param_1;
  FUN_0073f750(uVar2);
  iVar1 = FUN_007cb010(uVar2);
  if (iVar1 == 0) {
    uVar2 = (uint)param_1;
    FUN_0073f750(uVar2);
    iVar1 = FUN_007cae60(uVar2);
    if (iVar1 == 0) {
      return 0;
    }
  }
  return 1;
}

```

## R370 FUNCTION CGuiInGame::GetBarkBubbleVisible  steam=0x007cea90
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, int* outBottomY) param_bytes=4
Steam function at this address: size=84 ret_N=4
```c
// FN 007cea90 FUN_007cea90 size=84 ret=4 callers=008b8ae0 callees=00410800

undefined4 __thiscall FUN_007cea90(int param_1,int *param_2)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 0x4c) != 0) &&
     (iVar1 = CMFCVisualManagerBitmapCache::IsCached
                        (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                         *(CSize **)(param_1 + 0x4c)), iVar1 != 0)) {
    *param_2 = *(int *)(*(int *)(param_1 + 0x4c) + 8) + *(int *)(*(int *)(param_1 + 0x4c) + 0x10);
    return 1;
  }
  return 0;
}

```

## R371 FUNCTION CGuiInGame::ResetInterfaceForSize  steam=0x007cfca0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=1287 ret_N=0
```c
// FN 007cfca0 FUN_007cfca0 size=1287 ret=0 callers=0078dbf0 callees=0073fea0,00747210,0040ea00,0075ae40,0089c5f0,00919723,00898ca0,008bba80

void __fastcall FUN_007cfca0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_b0;
  undefined4 local_a8;
  undefined4 local_a0;
  undefined4 local_98;
  undefined4 local_90;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969cf7;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x98) != 0) {
    if (*(undefined4 **)(param_1 + 0x98) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x98))(1,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    }
    iVar1 = FUN_00919723/*operator new*/(0x15418);
    local_8 = 0;
    if (iVar1 == 0) {
      local_90 = 0;
    }
    else {
      uVar2 = FUN_0073fea0();
      local_90 = FUN_00747210(uVar2);
    }
    *(undefined4 *)(param_1 + 0x98) = local_90;
  }
  local_8 = 0xffffffff;
  if (*(int *)(param_1 + 0x40) != 0) {
    if (*(undefined4 **)(param_1 + 0x40) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x40))(1);
    }
    iVar1 = FUN_00919723/*operator new*/(0x2bc0);
    local_8 = 1;
    if (iVar1 == 0) {
      local_98 = 0;
    }
    else {
      uVar2 = FUN_0073fea0();
      local_98 = FUN_008bba80(uVar2);
    }
    *(undefined4 *)(param_1 + 0x40) = local_98;
  }
  local_8 = 0xffffffff;
  if (*(int *)(param_1 + 0x58) != 0) {
    FUN_0040ea00/*CSWGuiPanel::CenterPanel*/();
  }
  if (*(int *)(param_1 + 0x70) != 0) {
    FUN_0040ea00/*CSWGuiPanel::CenterPanel*/();
  }
  if (*(int *)(param_1 + 0x74) != 0) {
    FUN_0040ea00/*CSWGuiPanel::CenterPanel*/();
  }
  if (*(int *)(param_1 + 0x5c) != 0) {
    FUN_0040ea00/*CSWGuiPanel::CenterPanel*/();
  }
  if (*(int *)(param_1 + 0xa0) != 0) {
    if (*(undefined4 **)(param_1 + 0xa0) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0xa0))(1);
    }
    iVar1 = FUN_00919723/*operator new*/(0x9c8);
    local_8 = 2;
... (61 more lines)
```

## R372 FUNCTION CSWGuiControl::Destructor  steam=0x00418640
GOG build: param_bytes=0 ghidra_sig='CSWGuiControl * this'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=126 ret_N=0
```c
// FN 00418640 FUN_00418640 size=126 ret=0 callers=00948a40,00962fb9,00975597,00963937,00948c10,00978899,00975c68,0097e94f,0097da16,008bdb10,00977580,0097b427,00971bca,009798f1,009721eb,0097bf11,0098104d,00980498,007dcb00,00982787,0097883a,009748ce,00976413,0097e9ae,00917a30,00980ab9,0097dc34,00979904,0097f094,00905b80,00978e92,0097756d,0097c8f1,0097cf07,00981ec2,0098045f,00908070,00979b54,0097b0ec,009771b9,0097960d,00948dd8,0097ae5c,009788ac,00916a10,00980a6d,00972ddf,008b4f60,0097e988,0097ac21,00948be0,0096b45e,008b2620,0097a3dd,00979a7c,00741940,009804ab,0097a893,0097555e,00978092,0097d2cc,0097d318,00420450,00982509,008c84c0,00963970,00981e9c,00975c2f,00975253,00963d6a,008bcfc0,0097c93d,00963911,009803c7,0089dc30,0097b44d,0096ab3f,00971ca3,00975c8e,009748e1,00976439,0075fea0,00962b5e,0096a52e,00979a69,007d2a80,0097b414,0096af52,00974e93,009638eb,0096b44b,0097dc47,008f5be0,0097c8b8,00963f8d,00963fa0,00980a5a,007db4c0,00974907,009803ed,00978860,009760a0,0097d305,00976edb,009721d8,00971cc9,0097d9c1,0097c917,0097ee0b,00969508,009718b0,0098175d,0097e9c1,008cad50,00971c16,009721fe,008fd490,00976ab0,00977aef,0041d190,0097ede5,009771c1,00981eaf,00971595,009829ea,00980400,00980a93,00971c31,007510b0,0097f0a7,0096460f,009768d2,00974ea6,0097d32b,0096b1b4,0096af78,0097b3f9,0097f0ba,009005d0,007d52e0,00978873,0074aa10,0096a1cc,0041a3d0,00977099,008fa2f0,009817a9,00975279,0097ee1e,00976400,0097e916,00976426,00885510,00851390,0097b108,009775b9,00975cb4,00977b02,00976540,00981e63,009642aa,00979562,0097a1f4,0097edf8,00981e50,0096c0cd,009795e7,00981e3d,00418600,00981027,0097e962,008d21c0,00974e80,00976759,008bde40,00962a1c,009768e2,008e07b0,00972211,0097b486,00881670,00962a2f,0097b473,009771e7,00976dfa,00972237,00962b71,0098242b,0097beeb,009803da,00858970,0097d2df,00979575,0097b460,00888ad0,0097e9d4,00974e6d,00975266,0041ac10,00975c7b,00978e33,0096c0ba,0097bf4a,00975ced,0097c8a5,0041c0c0,007bc760,00980413,00981770,009827b5,00975c42,0097959b,0097b3f1,0097e90e,0097d2a6,008e1f10,00421060,0096a1b9,0097d2f2,0097e929,00979a30,0097bf24,0097d29e,00977ab6,00978e59,0097befe,00971cb6,0097d296,009771d4,00897e70,009768f5,0074a780,009629f6,008855c0,00895b00,0097c8de,0089db40,0096af65,0096a51b,0096a541,009642bd,009775df,00980472,00912220,0097e99b,00975571,0097807f,0084df70,0097bf37,0097b401,0096b1a1,00894140,00758a70,007563b0,00981086,0097f4cc,0074aac0,00979588,00981ed5,0096a059,009817cf,009771a6,00974eb9,00980426,00980a47,009817e2,00975cc7,0097d33e,00887920,0097a207,0097e975,0084dde0,008d7af0,0096b471,009765c0,0097c92a,009827a2,007d8f20,0097e9e7,0041a460,00977a7d,00963d57,0096202b,009795d4,0098174a,008803d0,00975584,0098278f,0097bf5d,00981796,00918f00,00978e46,00971bf0,008b8d30,0090e440,0097c904,008bf000,00979a56,00971bdd,008b9e80,0096af3f,004186c0,0074ac30,0097e8fb,0097d2b9,0074ad90,0096af19,0096a046,00974f28,0097ac0e,0089ca70,00971c44,0097ffd9,009770ac,00978827,00979b41,0041b8a0,00980a80,00760520,00979fe8,00971c7d,00978814,00979c9d,00971c03,009795fa,0096af2c,00979ffb,0097608d,0097ae49,0097ac34,0097e4e9,009803bf,00747210,0097ed48,00971c1e,00978e6c,0097bf83,00885ee0,00981783,008b8790,009748f4,00981073,00975ca1,009810ac,009795c1,009787db,007da3c0,009638fe,00975cda,00981099,00981e89,00963d44,007fbad0,00979a43,008c1f20,008dc620,00977b15,00978801,0097bf96,0097f80b,0088c110,00978e7f,009775cc,0096395d,008a1960,0097884d,0097cf23,00980439,00979917,0097f4df,008925f0,0097c8cb,00981060,009628cc,0096394a,00977aa3,0098103a,009638d8,0075b260,00980acc,00977a90,00971c90,00962a09,0097c892,008a7060,0098044c,00899bb0,008fbbf0,009829d7,00976775,00980485,00962fa6,00963924,008c6e90,00971c57,00978886,0097806c,0097b43a,009768da,00972224,0097bf70,00981e76,0097f4b9,00977ac9,008a3240,00975240,007d3600,008fa270,009787ee,00980a34,0097e93c,0084de70,0075d820,00971c6a,009817bc,008aa750,00977adc,0097f4a6,00980aa6,0097462b,008b94a0,0090a9a0,00981014,0097a293,009775a6,009795ae,00977593,0098243e,00976a20,00963d7d,00975c55,009638c5 callees=0064ddb0,00733780

void __fastcall FUN_00418640(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0094892e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiControl::vftable;
  local_8 = 2;
  Free(uVar1);
  local_8._0_1_ = 1;
  FUN_00733780/*CExoString::~CExoString*/();
  local_8 = (uint)local_8._1_3_ << 8;
  Free();
  *param_1 = CSWGuiObject::vftable;
  ExceptionList = local_10;
  return;
}

```

## R373 FUNCTION CResGFF::AddDataField  steam=0x00718370
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, int existingBlockOffset, DWORD sizeOfExistingBlock, DWORD dataLength) param_bytes=12
Steam function at this address: size=248 ret_N=12
```c
// FN 00718370 FUN_00718370 size=248 ret=12 callers=0071a320,0071a1b0,00719d40,00719c50,00719e50,0071a260,00719f30 callees=0091dba0,009196fd,00919723

int __thiscall FUN_00718370(int param_1,int param_2,size_t param_3,int param_4)

{
  void *_Dst;
  undefined4 local_c;
  
  if ((*(int *)(param_1 + 0x40) == 0) || (*(int *)(param_1 + 0x5c) == 0)) {
    local_c = 0;
  }
  else {
    local_c = *(int *)(*(int *)(param_1 + 0x40) + 0x24);
    if (local_c == param_2 + param_3) {
      local_c = param_2;
    }
    if (*(uint *)(param_1 + 0x60) <= (uint)(local_c + param_4)) {
      *(int *)(param_1 + 0x60) = local_c + param_4 + *(int *)(param_1 + 0x34);
      _Dst = (void *)FUN_00919723/*operator new*/(*(undefined4 *)(param_1 + 0x60));
      _memcpy(_Dst,*(void **)(param_1 + 0x5c),*(size_t *)(*(int *)(param_1 + 0x40) + 0x24));
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 0x5c));
      *(void **)(param_1 + 0x5c) = _Dst;
    }
    if (local_c != param_2) {
      _memcpy((void *)(*(int *)(param_1 + 0x5c) + local_c),
              (void *)(*(int *)(param_1 + 0x5c) + param_2),param_3);
    }
    *(int *)(*(int *)(param_1 + 0x40) + 0x24) = local_c + param_4;
    local_c = *(int *)(param_1 + 0x5c) + local_c;
  }
  return local_c;
}

```

## R374 FUNCTION CClientExoApp::QueryExitProgram  steam=0x0073fe20
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=22 ret_N=0
```c
// FN 0073fe20 FUN_0073fe20 size=22 ret=0 callers=0040a750 callees=00796ba0

void __fastcall FUN_0073fe20(undefined4 param_1)

{
  FUN_00796ba0(param_1);
  return;
}

```

## R375 FUNCTION CGuiInGame::UpdateDialogLipSync  steam=0x007d0c70
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=226 ret_N=0
```c
// FN 007d0c70 FUN_007d0c70 size=226 ret=0 callers=005467b0 callees=0078cf90,0070b0e0,00710eb0,0073f4d0

void __fastcall FUN_007d0c70(int param_1)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined1 local_18 [16];
  int *local_8;
  
  if ((*(int *)(param_1 + 0xfdc) != 0x7f000000) && (*(int *)(param_1 + 0xf20) != 0)) {
    iVar1 = FUN_0070b0e0();
    if (iVar1 != 0) {
      iVar1 = FUN_0078cf90();
      if (iVar1 != 0) {
        uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 4);
        local_8 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0xfdc));
        if (local_8 != (int *)0x0) {
          iVar1 = (**(code **)(*local_8 + 0xc))();
          if (iVar1 != 0) {
            piVar2 = (int *)(**(code **)(*local_8 + 0xc))();
            (**(code **)(*piVar2 + 0x128))(param_1 + 0xfe0,piVar2,param_1,uVar3);
            FUN_00710eb0(local_18,&DAT_00986925);
            *(undefined4 *)(param_1 + 0xfdc) = 0x7f000000;
          }
        }
      }
    }
  }
  return;
}

```

## R376 FUNCTION CWorldTimer::AddWorldTimes  steam=0x0051af60
GOG build: param_bytes=24 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5, undefined4 param_6'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD lhsDays, DWORD lhsMS, DWORD rhsDays, DWORD rhsMS, DWORD* outDays, DWORD* outMS) param_bytes=24
Steam function at this address: size=115 ret_N=24
```c
// FN 0051af60 FUN_0051af60 size=115 ret=24 callers=00801ab0,007a6760,005d4830,007664b0,0056b8a0,0056c310,005b96f0,005d4460,008e59c0,005d29f0,00557260,005d4700,0086da30,00623330,0062c360,00544ad0,005d18c0,00617980,007f12d0,00569320,0087b240,00600f20,005f4c00,005b8000,00521e60,008412a0,005d9aa0,005d9da0,0061e360,007fd370,008ee8c0,005d17f0,007d0b90,00891d50,005c3f70,006c9c00,00576dc0,0051acd0,0087d990 callees=0051afe0

undefined4 __thiscall
FUN_0051af60(int param_1,int param_2,int param_3,int param_4,int param_5,int *param_6,uint *param_7)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_0051afe0/*CWorldTimer::CompareWorldTimes*/(param_2,param_3,param_4,param_5);
  if (iVar1 == -1) {
    uVar2 = 0xfffffffe;
  }
  else {
    *param_6 = param_2 - param_4;
    *param_7 = param_3 - param_5;
    if (*(uint *)(param_1 + 0x3c) <= *param_7) {
      *param_6 = *param_6 + -1;
      *param_7 = *param_7 + *(int *)(param_1 + 0x3c);
    }
    uVar2 = 0;
  }
  return uVar2;
}

```

## R377 FUNCTION CResGFF::ReadFieldSHORT  steam=0x00718b00
GOG build: param_bytes=14 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined2 param_4'
Wrapper typedef (what calling code expects): short (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label, int* success, short defaultValue) param_bytes=16
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
