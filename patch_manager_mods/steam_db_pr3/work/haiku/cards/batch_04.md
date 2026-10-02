## R108 FUNCTION CClientExoApp::SendLoadGameRequest  steam=0x0073f720
GOG build: param_bytes=12 ghidra_sig='CClientExoApp * this, uint * param_1, uint * param_2, uint * param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD slot, void* saveName, void* module) param_bytes=12
Steam function at this address: size=36 ret_N=12
```c
// FN 0073f720 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=00853a00,007d0f50 callees=00785660

/* Library Function - Single Match
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00785660(param_1,param_2,param_3);
  return;
}

```

## R109 FUNCTION CSWSDoor::MoveToNextOpenState  steam=0x0061d7b0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=167 ret_N=0
```c
// FN 0061d7b0 FUN_0061d7b0 size=167 ret=0 callers=0087af40 callees=0052ca10,005453c0

void __fastcall FUN_0061d7b0(int param_1)

{
  char cVar1;
  int iVar2;
  
  cVar1 = *(char *)(param_1 + 0x31c);
  *(undefined1 *)(param_1 + 0x31c) = *(undefined1 *)(param_1 + 0x31d);
  if ((*(char *)(param_1 + 0x31c) != cVar1) && (*(char *)(param_1 + 0x31c) != '\x03')) {
    iVar2 = FUN_005453c0();
    if (iVar2 != 0) {
      FUN_0052ca10(param_1 + 0x94,param_1 + 0x388,param_1 + 0x394,*(undefined4 *)(param_1 + 4),
                   *(char *)(param_1 + 0x31c) == '\0');
    }
  }
  return;
}

```

## R110 FUNCTION CClientExoApp::GetObjectName  steam=0x0073f0b0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD clientId, void* outName) param_bytes=8
Steam function at this address: size=36 ret_N=12
```c
// FN 0073f0b0 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=0082c840 callees=00785330

/* Library Function - Single Match
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00785330(param_1,param_2,param_3);
  return;
}

```

## R111 OFFSET CSWGuiBorderParams.fill_image_resref = 0x44
Claim: SetFillImage 0x414840 uses params+0x44; operator= copies src+0x44; Load tag FILL
Decompile 0x00414840:
```c
// FN 00414840 FUN_00414840/*CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force*/ size=219 ret=8 callers=0074d520,0075b370,008850b0,00414920,0088d700,00885770,0088aac0,007425f0,00854a00,00747210,008aadc0,0088a0b0,00906570,00852380,00745e60,00752760,0088c7d0,0040f390,0075bb50,008cd8e0,0075cc60,008b1250,00415080,00753850,0075dd10,00910490,008f4af0,008d0780,00753780,008973d0,0074fb20,0074ee90,0074e7f0,008dddd0,00904a80,0040ee40,00756b70,00889dd0,0089e000,008b3c00,008ad930,008857a0,00760f40,00753d60,00918420,007418a0,00885fe0,00750c10,008a5dd0,0075ca60 callees=00710ca0,00710a70,00710a10,0047eb60,00710980,009196ee

void __thiscall FUN_00414840/*CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force*/(int param_1,undefined4 param_2,int param_3)

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

## R112 FUNCTION CSWGuiPanel::HandleInputEvent  steam=0x0040eaa0
GOG build: param_bytes=8 ghidra_sig='CSWGuiPanel * this, int event, int param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int event, int inputPhase) param_bytes=8
Steam function at this address: size=51 ret_N=8
```c
// FN 0040eaa0 FUN_0040eaa0 size=51 ret=8 callers=007d8ff0,00919190,008b6010,00894e90,009158d0,008b2850,00917c80,008fbe00,0084f900,008bf0e0,0089cc50,007d2bc0,007d5580,00903ed0,00908920,008df470,008ba700,00914a90,008b9bb0,008c16a0,008c9380,00887b10,008e0a80,007d3bb0,007db680,008cfc10,0090e850,008aed10,008f6220,008517f0,007da660,00916b30,0075d950,008898a0,00898010,008bfce0,008c7330,008a3660,008e2460,007d7290,0090b200,00912640,008c1fc0,0089de70,008d9350,0088d5d0,008fd7c0,00759e20,008fabf0,007dccd0,007fbd90,008839c0,009007a0,008a2070,00757bd0,008d4260,008a8880,0075b8e0 callees=

void __thiscall FUN_0040eaa0(int param_1,undefined4 param_2,undefined4 param_3)

{
  if (*(int *)(param_1 + 0x20) != 0) {
    (**(code **)(**(int **)(param_1 + 0x20) + 0x3c))(param_2,param_3);
  }
  return;
}

```

## R113 FUNCTION CResGFF::Pack  steam=0x0071a8b0
GOG build: param_bytes=8 ghidra_sig='undefined1 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, byte alwaysZero1, DWORD alwaysZero2) param_bytes=8
Steam function at this address: size=1010 ret_N=8
```c
// FN 0071a8b0 FUN_0071a8b0 size=1010 ret=8 callers=00721bb0,0071a5a0 callees=0091dba0,009196fd,0071acb0,00919723

void __thiscall FUN_0071a8b0(int param_1,byte param_2,uint param_3)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  uint local_c;
  int local_8;
  
  if (((((*(int *)(param_1 + 0x6c) != 0) || (*(int *)(param_1 + 0x78) != 0)) &&
       (*(int *)(param_1 + 0x40) != 0)) &&
      (((*(int *)(param_1 + 100) != 0 && (*(int *)(param_1 + 0x70) != 0)) &&
       ((*(int *)(param_1 + 0x44) != 0 &&
        ((*(int *)(param_1 + 0x4c) != 0 &&
         (uVar1 = FUN_0071acb0/*CResGFF::GetTotalSize*/(),
         param_3 < (uint)(*(int *)(param_1 + 0x6c) + *(int *)(param_1 + 0x78)))))))))) &&
     ((uint)param_2 < (uint)((*(int *)(param_1 + 0x6c) + *(int *)(param_1 + 0x78)) * 100) / uVar1))
  {
    if (*(int *)(param_1 + 0x6c) != 0) {
      if (*(int *)(*(int *)(param_1 + 0x40) + 0x2c) == *(int *)(param_1 + 0x6c)) {
        *(undefined4 *)(*(int *)(param_1 + 0x40) + 0x2c) = 0;
      }
      else {
        iVar2 = FUN_00919723/*operator new*/(*(int *)(*(int *)(param_1 + 0x40) + 0x2c) - *(int *)(param_1 + 0x6c));
        local_8 = 0;
        for (local_c = 0; local_c < *(uint *)(*(int *)(param_1 + 0x40) + 0xc); local_c = local_c + 1
            ) {
          if (1 < *(uint *)(*(int *)(param_1 + 0x44) + 8 + local_c * 0xc)) {
            if ((*(uint *)(*(int *)(param_1 + 0x44) + 4 + local_c * 0xc) <
                 *(uint *)(*(int *)(param_1 + 0x40) + 0x2c)) &&
               ((uint)(*(int *)(*(int *)(param_1 + 0x44) + 8 + local_c * 0xc) << 2) <=
                (uint)(*(int *)(*(int *)(param_1 + 0x40) + 0x2c) -
                      *(int *)(*(int *)(param_1 + 0x44) + 4 + local_c * 0xc)))) {
              _memcpy((void *)(iVar2 + local_8),
                      (void *)(*(int *)(param_1 + 100) +
                              *(int *)(*(int *)(param_1 + 0x44) + 4 + local_c * 0xc)),
                      *(int *)(*(int *)(param_1 + 0x44) + 8 + local_c * 0xc) << 2);
              *(int *)(*(int *)(param_1 + 0x44) + 4 + local_c * 0xc) = local_8;
              local_8 = local_8 + *(int *)(*(int *)(param_1 + 0x44) + 8 + local_c * 0xc) * 4;
            }
            else {
              *(undefined4 *)(*(int *)(param_1 + 0x44) + 4 + local_c * 0xc) = 0xffffffff;
              *(undefined4 *)(*(int *)(param_1 + 0x44) + 8 + local_c * 0xc) = 0;
            }
          }
        }
        FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 100));
        *(int *)(param_1 + 100) = iVar2;
        *(int *)(*(int *)(param_1 + 0x40) + 0x2c) = local_8;
      }
      *(undefined4 *)(param_1 + 0x6c) = 0;
    }
    if (*(int *)(param_1 + 0x78) != 0) {
      if (*(int *)(*(int *)(param_1 + 0x40) + 0x34) == *(int *)(param_1 + 0x78)) {
        *(undefined4 *)(*(int *)(param_1 + 0x40) + 0x34) = 0;
      }
      else {
        iVar2 = FUN_00919723/*operator new*/(*(int *)(*(int *)(param_1 + 0x40) + 0x34) - *(int *)(param_1 + 0x78));
        local_8 = 0;
        for (local_c = 0; local_c < *(uint *)(*(int *)(param_1 + 0x40) + 0x14);
            local_c = local_c + 1) {
          if (*(int *)(local_c * 0xc + *(int *)(param_1 + 0x4c)) == 0xf) {
            if (((*(uint *)(*(int *)(param_1 + 0x4c) + 8 + local_c * 0xc) <
                  *(uint *)(*(int *)(param_1 + 0x40) + 0x34)) &&
                (3 < (uint)(*(int *)(*(int *)(param_1 + 0x40) + 0x34) -
                           *(int *)(*(int *)(param_1 + 0x4c) + 8 + local_c * 0xc)))) &&
               (iVar3 = *(int *)(*(int *)(param_1 + 0x70) +
                                *(int *)(*(int *)(param_1 + 0x4c) + 8 + local_c * 0xc)) * 4,
... (24 more lines)
```

## R114 FUNCTION CWorldTimer::AddWorldTimes  steam=0x0051aef0
GOG build: param_bytes=24 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5, undefined4 param_6'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD lhsDays, DWORD lhsMS, DWORD rhsDays, DWORD rhsMS, DWORD* outDays, DWORD* outMS) param_bytes=24
Steam function at this address: size=108 ret_N=24
```c
// FN 0051aef0 FUN_0051aef0 size=108 ret=24 callers=00587290,005a06f0,00544210,005a4bb0,005bfc00,0051e170,005a4d70,00545470,00545520,007efc40 callees=

undefined4 __thiscall
FUN_0051aef0(int param_1,int param_2,uint param_3,int param_4,uint param_5,int *param_6,
            uint *param_7)

{
  undefined4 uVar1;
  
  if ((param_3 < *(uint *)(param_1 + 0x3c)) && (param_5 < *(uint *)(param_1 + 0x3c))) {
    *param_6 = param_2 + param_4;
    *param_7 = param_3 + param_5;
    if (*(uint *)(param_1 + 0x3c) <= *param_7) {
      *param_6 = *param_6 + 1;
      *param_7 = *param_7 - *(int *)(param_1 + 0x3c);
    }
    uVar1 = 0;
  }
  else {
    uVar1 = 0xfffffffe;
  }
  return uVar1;
}

```

## R115 FUNCTION CResGFF::WriteFieldVOID  steam=0x0071a1b0
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, void* data, DWORD size, char* label) param_bytes=16
Steam function at this address: size=174 ret_N=16
```c
// FN 0071a1b0 FUN_0071a1b0 size=174 ret=16 callers=0052b270,005fb1a0,00655010,00703370,0055bd10 callees=00717aa0,0091dba0,00718030,00718370

undefined4 __thiscall FUN_0071a1b0(int param_1,int param_2,void *param_3,size_t param_4,int param_5)

{
  undefined4 uVar1;
  int iVar2;
  size_t *psVar3;
  
  if ((param_2 == 0) || (param_5 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0/*CResGFF::InitializeForWriting*/();
    }
    iVar2 = FUN_00718030/*CResGFF::AddField*/(param_2,param_5,0xd);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      psVar3 = (size_t *)FUN_00718370/*CResGFF::AddDataField*/(0xffffffff,0,param_4 + 4);
      if (psVar3 == (size_t *)0x0) {
        uVar1 = 0;
      }
      else {
        *(int *)(iVar2 + 8) = (int)psVar3 - *(int *)(param_1 + 0x5c);
        *psVar3 = param_4;
        _memcpy(psVar3 + 1,param_3,param_4);
        uVar1 = 1;
      }
    }
  }
  return uVar1;
}

```

## R116 FUNCTION CGuiInGame::GetCanClick  steam=0x007d0b90
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=153 ret_N=0
```c
// FN 007d0b90 FUN_007d0b90 size=153 ret=0 callers=0077cf70,00841ce0 callees=0073f3b0,0051af60,0051ad20

undefined4 FUN_007d0b90(void)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int *piVar5;
  undefined4 *puVar6;
  uint *puVar7;
  int extraout_var;
  int iVar8;
  undefined4 local_14;
  uint local_10;
  undefined4 local_c;
  int local_8;
  
  local_10 = 0;
  uVar1 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  puVar6 = &local_c;
  puVar4 = &local_14;
  FUN_0073f3b0(puVar4,puVar6);
  iVar8 = extraout_var;
  FUN_0051ad20/*CWorldTimer::GetWorldTime*/(puVar4,puVar6);
  puVar7 = &local_10;
  piVar5 = &local_8;
  uVar3 = *(undefined4 *)(iVar8 + 0xfd0);
  uVar2 = *(undefined4 *)(iVar8 + 0xfcc);
  FUN_0073f3b0(local_14,local_c,uVar2,uVar3,piVar5,puVar7,iVar8,*(undefined4 *)(DAT_00a1b4a4 + 4),
               uVar1);
  FUN_0051af60/*CWorldTimer::SubtractWorldTimes*/(local_14,local_c,uVar2,uVar3,piVar5,puVar7);
  if (local_8 == 0) {
    if (local_10 < *(uint *)(iVar8 + 0xfd4)) {
      uVar1 = 0;
    }
    else {
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}

```

## R117 FUNCTION CResGFF::WriteGFFData  steam=0x0071a670
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): bool (__thiscall)(void* thisPtr, CExoFile* file, DWORD* totalBytes) param_bytes=8
Steam function at this address: size=562 ret_N=8
```c
// FN 0071a670 FUN_0071a670 size=562 ret=8 callers=00721bb0,0071a5a0 callees=00739a30

undefined4 __thiscall FUN_0071a670(int param_1,undefined4 param_2,int *param_3)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  
  if (*(int *)(param_1 + 0x40) == 0) {
    uVar1 = 0;
  }
  else {
    *param_3 = 0x38;
    *(int *)(*(int *)(param_1 + 0x40) + 8) = *param_3;
    *param_3 = *(int *)(*(int *)(param_1 + 0x40) + 0xc) * 0xc + *param_3;
    *(int *)(*(int *)(param_1 + 0x40) + 0x10) = *param_3;
    *param_3 = *(int *)(*(int *)(param_1 + 0x40) + 0x14) * 0xc + *param_3;
    *(int *)(*(int *)(param_1 + 0x40) + 0x18) = *param_3;
    *param_3 = *(int *)(*(int *)(param_1 + 0x40) + 0x1c) * 0x10 + *param_3;
    *(int *)(*(int *)(param_1 + 0x40) + 0x20) = *param_3;
    *param_3 = *param_3 + *(int *)(*(int *)(param_1 + 0x40) + 0x24);
    *(int *)(*(int *)(param_1 + 0x40) + 0x28) = *param_3;
    *param_3 = *param_3 + *(int *)(*(int *)(param_1 + 0x40) + 0x2c);
    *(int *)(*(int *)(param_1 + 0x40) + 0x30) = *param_3;
    *param_3 = *param_3 + *(int *)(*(int *)(param_1 + 0x40) + 0x34);
    iVar2 = FUN_00739a30(*(undefined4 *)(param_1 + 0x40),0x38,1);
    if (iVar2 == 0x38) {
      iVar3 = *(int *)(*(int *)(param_1 + 0x40) + 0xc) * 0xc;
      iVar2 = FUN_00739a30(*(undefined4 *)(param_1 + 0x44),iVar3,1);
      if (iVar2 == iVar3) {
        iVar3 = *(int *)(*(int *)(param_1 + 0x40) + 0x14) * 0xc;
        iVar2 = FUN_00739a30(*(undefined4 *)(param_1 + 0x4c),iVar3,1);
        if (iVar2 == iVar3) {
          iVar3 = *(int *)(*(int *)(param_1 + 0x40) + 0x1c) << 4;
          iVar2 = FUN_00739a30(*(undefined4 *)(param_1 + 0x54),iVar3,1);
          if (iVar2 == iVar3) {
            iVar2 = *(int *)(*(int *)(param_1 + 0x40) + 0x24);
            iVar3 = FUN_00739a30(*(undefined4 *)(param_1 + 0x5c),iVar2,1);
            if (iVar3 == iVar2) {
              iVar2 = *(int *)(*(int *)(param_1 + 0x40) + 0x2c);
              iVar3 = FUN_00739a30(*(undefined4 *)(param_1 + 100),iVar2,1);
              if (iVar3 == iVar2) {
                iVar2 = *(int *)(*(int *)(param_1 + 0x40) + 0x34);
                iVar3 = FUN_00739a30(*(undefined4 *)(param_1 + 0x70),iVar2,1);
                if (iVar3 == iVar2) {
                  uVar1 = 1;
                }
                else {
                  uVar1 = 0;
                }
              }
              else {
                uVar1 = 0;
              }
            }
            else {
              uVar1 = 0;
            }
          }
          else {
            uVar1 = 0;
          }
        }
        else {
          uVar1 = 0;
        }
      }
      else {
        uVar1 = 0;
... (9 more lines)
```

## R118 FUNCTION CSWSObject::GetScriptLocation  steam=0x00547600
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, void* outLocation) param_bytes=4
Steam function at this address: size=188 ret_N=4
```c
// FN 00547600 FUN_00547600 size=188 ret=4 callers=00682f50 callees=00623320,0053e060

undefined4 * __thiscall FUN_00547600(int param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  uint uVar6;
  uint uVar7;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094fae9;
  local_10 = ExceptionList;
  uVar6 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  uVar7 = 0;
  FUN_0053e060(uVar6,param_1,0);
  uVar1 = *(undefined4 *)(param_1 + 0x98);
  uVar2 = *(undefined4 *)(param_1 + 0x9c);
  uVar3 = *(undefined4 *)(param_1 + 0xa0);
  uVar4 = *(undefined4 *)(param_1 + 0xa4);
  uVar5 = *(undefined4 *)(param_1 + 0xa8);
  *param_2 = *(undefined4 *)(param_1 + 0x94);
  param_2[1] = uVar1;
  param_2[2] = uVar2;
  param_2[3] = uVar3;
  param_2[4] = uVar4;
  param_2[5] = uVar5;
  local_8 = 0;
  FUN_00623320(uVar6,param_1,uVar7 | 1);
  ExceptionList = local_10;
  return param_2;
}

```

## R119 OFFSET CResGFF.header = 0x40
Claim: GetStruct 0x717720 bounds struct id vs header+0xC (StructCount), GetField 0x717770 vs header+0x14 (FieldCount), GetFieldByLabel 0x7178e0 vs header+0x1c (LabelCount): GFF V3.2 header layout; this+0x40 = header ptr
Decompile 0x00717720:
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
Decompile 0x00717770:
```c
// FN 00717770 FUN_00717770 size=171 ret=8 callers=00719800,00718b80,007190e0,00718810,00718a00,00718c00,00718a80,00718710,00718890,00718d80,00718e40,00718b00,00718d00,007193d0,00718960,00718f90,00719640,00718760,00719520,00718c80 callees=00717720

int __thiscall FUN_00717770(int param_1,undefined4 param_2,uint param_3)

{
  int iVar1;
  
  iVar1 = FUN_00717720(param_2);
  if (((iVar1 == 0) || (*(int *)(param_1 + 0x4c) == 0)) || (*(uint *)(iVar1 + 8) <= param_3)) {
    iVar1 = 0;
  }
  else if (*(int *)(iVar1 + 8) == 1) {
    if (*(uint *)(iVar1 + 4) < *(uint *)(*(int *)(param_1 + 0x40) + 0x14)) {
      iVar1 = *(int *)(iVar1 + 4) * 0xc + *(int *)(param_1 + 0x4c);
    }
    else {
      iVar1 = 0;
    }
  }
  else if ((*(int *)(param_1 + 100) == 0) ||
          (*(uint *)(*(int *)(param_1 + 0x40) + 0x2c) < *(int *)(iVar1 + 4) + 4 + param_3 * 4)) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(*(int *)(param_1 + 100) + *(int *)(iVar1 + 4) + param_3 * 4) * 0xc +
            *(int *)(param_1 + 0x4c);
  }
  return iVar1;
}

```
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

## R120 FUNCTION CSWCPlaceable::ActionMenuSecurity  steam=0x00842750
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* creature) param_bytes=4
Steam function at this address: size=69 ret_N=8
```c
// FN 00842750 FUN_00842750 size=69 ret=8 callers= callees=00879d90,0073f810

void __fastcall FUN_00842750(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_0073f810();
  if (iVar1 != 0) {
    FUN_00879d90(*(undefined4 *)(param_1 + 4),0x7f000000);
  }
  return;
}

```

## R121 FUNCTION CGuiInGame::ShutDown  steam=0x007c7030
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=2831 ret_N=0
```c
// FN 007c7030 FUN_007c7030 size=2831 ret=0 callers=00781be0,0078ac40,0078bc50,007bdbc0 callees=007c0090,009196fd,007c0290,007c0210,008aa720

void __fastcall FUN_007c7030(int param_1)

{
  undefined4 *puVar1;
  int local_8;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    *(undefined4 *)(param_1 + 0x38) = 0;
    if (*(int *)(param_1 + 0x4c) != 0) {
      if (*(undefined4 **)(param_1 + 0x4c) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x4c))(1);
      }
      *(undefined4 *)(param_1 + 0x4c) = 0;
    }
    if (*(int *)(param_1 + 0x50) != 0) {
      if (*(undefined4 **)(param_1 + 0x50) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x50))(1);
      }
      *(undefined4 *)(param_1 + 0x50) = 0;
    }
    if (*(int *)(param_1 + 0x54) != 0) {
      if (*(undefined4 **)(param_1 + 0x54) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x54))(1);
      }
      *(undefined4 *)(param_1 + 0x54) = 0;
    }
    if (*(int *)(param_1 + 0x58) != 0) {
      if (*(undefined4 **)(param_1 + 0x58) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x58))(1);
      }
      *(undefined4 *)(param_1 + 0x58) = 0;
    }
    if (*(int *)(param_1 + 0x70) != 0) {
      if (*(undefined4 **)(param_1 + 0x70) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x70))(1);
      }
      *(undefined4 *)(param_1 + 0x70) = 0;
    }
    if (*(int *)(param_1 + 0x74) != 0) {
      if (*(undefined4 **)(param_1 + 0x74) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x74))(1);
      }
      *(undefined4 *)(param_1 + 0x74) = 0;
    }
    if (*(int *)(param_1 + 0x5c) != 0) {
      if (*(undefined4 **)(param_1 + 0x5c) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x5c))(1);
      }
      *(undefined4 *)(param_1 + 0x5c) = 0;
    }
    if (*(int *)(param_1 + 8) != 0) {
      if (*(undefined4 **)(param_1 + 8) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 8))(1);
      }
      *(undefined4 *)(param_1 + 8) = 0;
    }
    if (*(int *)(param_1 + 0x60) != 0) {
      if (*(undefined4 **)(param_1 + 0x60) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x60))(1);
      }
      *(undefined4 *)(param_1 + 0x60) = 0;
    }
    if (*(int *)(param_1 + 100) != 0) {
      if (*(undefined4 **)(param_1 + 100) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 100))(1);
      }
      *(undefined4 *)(param_1 + 100) = 0;
    }
... (129 more lines)
```

## R122 FUNCTION Gob::DisableInCutscene  steam=0x00459e20
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=17 ret_N=0
```c
// FN 00459e20 FUN_00459e20 size=17 ret=0 callers=004dfed0,00468b50,004e1d40,00467d70,004682c0 callees=

undefined1 __fastcall FUN_00459e20(int param_1)

{
  return *(undefined1 *)(param_1 + 0x14);
}

```

## R123 FUNCTION CClientExoApp::SetCombatMode  steam=0x00740880
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int mode) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 00740880 FUN_00740880 size=28 ret=4 callers=0077fd20,007e5df0,007469f0 callees=0079d6e0

void FUN_00740880(undefined4 param_1)

{
  FUN_0079d6e0(param_1);
  return;
}

```

## R124 FUNCTION CClientExoApp::SetMouseMode  steam=0x00740f40
GOG build: param_bytes=1 ghidra_sig='undefined1 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, BYTE mode) param_bytes=4
Steam function at this address: size=29 ret_N=4
```c
// FN 00740f40 FUN_00740f40 size=29 ret=4 callers=007bc1b0 callees=007a0490

void FUN_00740f40(undefined1 param_1)

{
  FUN_007a0490(param_1);
  return;
}

```

## R125 FUNCTION CSWCObject::AnimationPause  steam=0x007ef350
GOG build: param_bytes=2 ghidra_sig='undefined2 param_1'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, WORD animationRowIndex) param_bytes=4
Steam function at this address: size=129 ret_N=4
```c
// FN 007ef350 FUN_007ef350 size=129 ret=4 callers=007675b0,00776fe0 callees=0071b8b0

bool __thiscall FUN_007ef350(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe2c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}

```

## R126 FUNCTION Scene::GetObjectA  steam=0x00465290
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): Gob* (__thiscall)(void* thisPtr, char* objectName) param_bytes=4
Steam function at this address: size=102 ret_N=4
```c
// FN 00465290 FUN_00465290 size=102 ret=4 callers= callees=009370c7

undefined4 __thiscall FUN_00465290(int param_1,char *param_2)

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

## R127 FUNCTION CClientExoApp::GetGameObject  steam=0x0073f510
GOG build: param_bytes=4 ghidra_sig='CClientExoApp * this, ulong param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, DWORD clientId) param_bytes=4
Steam function at this address: size=28 ret_N=4
```c
// FN 0073f510 FUN_0073f510 size=28 ret=4 callers=008074e0,00802f00,00804f10,00765aa0,008079b0 callees=0078bd20

void FUN_0073f510(undefined4 param_1)

{
  FUN_0078bd20(param_1);
  return;
}

```

## R128 FUNCTION CRes::GetResRef  steam=0x00711270
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* outRef, WORD* outType) param_bytes=8
Steam function at this address: size=50 ret_N=8
```c
// FN 00711270 FUN_00711270 size=50 ret=8 callers=006cf910,006cfa70 callees=00710ca0

void __thiscall FUN_00711270(int param_1,undefined4 param_2,undefined2 *param_3)

{
  undefined1 local_14 [16];
  
  FUN_00710ca0(local_14,*(undefined4 *)(param_1 + 0x14));
  *param_3 = *(undefined2 *)(*(int *)(param_1 + 0x14) + 0x1a);
  return;
}

```

## R129 FUNCTION CResGFF::AddLabel  steam=0x007181f0
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr, char* text) param_bytes=4
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

## R130 FUNCTION CSWGuiPanel::StopLoadFromLayout  steam=0x0040f5a0
GOG build: param_bytes=0 ghidra_sig='CSWGuiPanel * this'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=117 ret_N=0
```c
// FN 0040f5a0 FUN_0040f5a0 size=117 ret=0 callers=0075b370,008850b0,00893950,008a1170,00892090,00918b10,0088aac0,009074e0,007d6170,00747210,00880150,008c19f0,008b4270,0084c3a0,008d1020,0089c5f0,007d8540,0075ae40,008bba80,007d23e0,00760210,007d9800,008c80f0,008c9e10,00850770,007d4620,007dc410,008fb7a0,0075fda0,00916ef0,007dacb0,00886fc0,008a92d0,009153d0,007bc600,00757c40,008e1810,008586d0,008db630,008f92b0,00910490,0040e860,008b91f0,008bfb60,008f4af0,008fc8e0,008b1ea0,008973d0,00916640,008bd910,008b82e0,00902f60,00904a80,008fff20,008bdc90,0089cf30,008df930,008c0cb0,007fae60,00754ed0,008c6650,008bc620,00914000,008a25c0,008d6b90,00881970,007d31d0,0090c6a0,0075cec0,008a6170,008bee10,00887da0,00909e00 callees=0040fb70,007112b0

void __fastcall FUN_0040f5a0(int param_1)

{
  undefined4 uVar1;
  
  if ((*(uint *)(param_1 + 0x48) >> 1 & 1) != 0) {
    FUN_007112b0/*CRes::Release*/();
    if (*(undefined4 **)(param_1 + 0x30) == (undefined4 *)0x0) {
      uVar1 = 0;
    }
    else {
      uVar1 = (**(code **)**(undefined4 **)(param_1 + 0x30))(1);
    }
    *(undefined4 *)(param_1 + 0x30) = 0;
    *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffffd;
    FUN_0040fb70(uVar1);
  }
  return;
}

```

## R131 FUNCTION CGuiInGame::GetCameraAnimationName  steam=0x007c0ec0
GOG build: param_bytes=6 ghidra_sig='undefined4 param_1, undefined2 param_2'
Wrapper typedef (what calling code expects): void* (__stdcall)(void* outName, short animationId) param_bytes=8
Steam function at this address: size=9401 ret_N=8
```c
// FN 007c0ec0 FUN_007c0ec0 size=9401 ret=8 callers=007f67f0 callees=00733570,007335f0,00733780,007338d0

undefined4 FUN_007c0ec0(undefined4 param_1,ushort param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  uint uVar4;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009699d1;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  uVar4 = 0;
  FUN_00733570/*CExoString::CExoString_char*/(&DAT_00987fb4);
  local_8 = 1;
  if (param_2 == 0x2772) {
    FUN_007335f0/*CExoString::CExoString_copy*/(local_18);
    local_8 = local_8 & 0xffffff00;
    FUN_00733780/*CExoString::~CExoString*/(uVar1);
  }
  else {
    iVar2 = param_2 - 1000;
    uVar3 = extraout_var;
    switch(iVar2) {
    case 0:
      FUN_007338d0("cut001");
      break;
    case 1:
      FUN_007338d0("cut002");
      break;
    case 2:
      FUN_007338d0("cut003");
      break;
    case 3:
      FUN_007338d0("cut004");
      break;
    case 4:
      FUN_007338d0("cut005");
      break;
    case 5:
      FUN_007338d0("cut006");
      break;
    case 6:
      FUN_007338d0("cut007");
      break;
    case 7:
      FUN_007338d0("cut008");
      break;
    case 8:
      FUN_007338d0("cut009");
      break;
    case 9:
      FUN_007338d0("cut010");
      break;
    case 10:
      FUN_007338d0("cut011");
      break;
    case 0xb:
      FUN_007338d0("cut012");
      break;
    case 0xc:
      FUN_007338d0("cut013");
... (1506 more lines)
```

## R132 FUNCTION CSWSTrigger::RemoveFromArea  steam=0x00622670
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=213 ret_N=0
```c
// FN 00622670 FUN_00622670 size=213 ret=0 callers=0061f170,005f2fe0,0061e260,0061e0e0,00617b40 callees=00543f50,00476620,0052e4e0,005453c0,00545420,00523090

void __fastcall FUN_00622670(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_005453c0();
  if (iVar1 != 0) {
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_00523090(uVar2);
    FUN_005453c0();
    FUN_0052e4e0(*(undefined4 *)(param_1 + 4));
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0();
    iVar1 = FUN_00476620(uVar2);
    if (iVar1 == 1) {
      uVar2 = *(undefined4 *)(param_1 + 4);
      FUN_005453c0();
      FUN_0052e4e0(uVar2);
    }
  }
  FUN_00545420(0);
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  FUN_00543f50(&local_10,1,1,0);
  return;
}

```

## R133 FUNCTION CResGFF::GetFieldByLabel  steam=0x007178e0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): DWORD (__thiscall)(void* thisPtr, CResStruct* structPtr, char* label) param_bytes=8
Steam function at this address: size=269 ret_N=8
```c
// FN 007178e0 FUN_007178e0 size=269 ret=8 callers=00719800,00718b80,007190e0,00718810,00718a00,00718c00,00718a80,00718710,00718890,00718d80,00718e40,00718b00,00718d00,007193d0,00718960,00718f90,00719640,00718760,00719520,00718c80 callees=007186c0,0091d0f0,00717a00,0091c630,00717720,009196ee

void __thiscall FUN_007178e0(int param_1,undefined4 param_2,char *param_3)

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

## R134 FUNCTION CRes::Demand  steam=0x007111e0
GOG build: param_bytes=0 ghidra_sig='CRes * this'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=26 ret_N=0
```c
// FN 007111e0 FUN_007111e0 size=26 ret=0 callers=00940840,0070ec50,0093f8b0,00941040,005236b0,0093ffb0,00944d80,009422e0,00716fd0,0071c3d0,00638bd0,005e9ee0,0093f340,008bf280,0071f750,0083db30,00630810,009414d0,00940c40,0055db00,0040ee40,0093d110,005582f0,0055d080,00765df0,00630f20,0093d5b0,00701fd0 callees=00711c20

void __fastcall FUN_007111e0(undefined4 param_1)

{
  FUN_00711c20(param_1);
  return;
}

```
