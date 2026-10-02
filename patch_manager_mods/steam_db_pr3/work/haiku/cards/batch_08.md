## R216 FUNCTION CSWSObject::ClearAllActions  steam=0x00541080
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, int includeAttacks) param_bytes=4
Steam function at this address: size=779 ret_N=4
```c
// FN 00541080 FUN_00541080 size=779 ret=4 callers=00566410,005a60b0,007445d0,005f8520,00696e10,0066df70,00656af0,0066ad10,0065b490,007469f0,005951a0,005f8130,0059de30,0066f330,00599050,005d5550,005302e0,005fd810,00568540,0057da90,008b05e0,0065be70,007511b0,005df920,005953e0,0065b270,008b0320,005959e0,00751750,0051cb10,0089fbd0,005a6570,00547bb0,0068db00,005fd480,00597d90,005f7f30,00586750,00586b70,0059b520,0077cf70,005a6280,00563c90,006cf5d0,0065e670,0057de70,005f8320,005d75b0,005a64c0 callees=007e6be0,0073fec0,0064dde0,0053e740,00734010,00561430,005908f0,0073fb90,0058c370,006f9c90,009196fd,00521060,007a1720

void __thiscall FUN_00541080(int *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int local_8;
  
  iVar1 = (**(code **)(*param_1 + 0x10))();
  if (iVar1 != 0) {
    if (((DAT_00a7e0c8 != 0) && (iVar1 = (**(code **)(*param_1 + 0x30))(), iVar1 != 0)) &&
       (iVar1 = (**(code **)(*param_1 + 0x30))(), *(int *)(iVar1 + 0x11ac) != 0)) {
      FUN_00734010();
    }
    if (param_1[0x3b] == 1) {
      local_8 = FUN_007a1720();
      while (local_8 != 0) {
        iVar2 = FUN_00561430(local_8);
        iVar1 = local_8;
        FUN_0058c370(&local_8);
        iVar3 = FUN_00521060();
        iVar2 = (**(code **)(*param_1 + 0x74))(iVar2,iVar2 == iVar3);
        if ((iVar2 == 1) && (iVar1 = FUN_0064dde0(iVar1), iVar1 != 0)) {
          FUN_0053e740();
          FUN_009196fd/*operator delete*/(iVar1);
        }
      }
      iVar1 = (**(code **)(*param_1 + 0x30))();
      if (iVar1 != 0) {
        (**(code **)(*param_1 + 0x30))();
        FUN_006f9c90/*CPathfindInformation::ResetRequest*/();
        iVar1 = (**(code **)(*param_1 + 0x30))();
        *(undefined4 *)(iVar1 + 0x11b0) = 1;
        iVar1 = (**(code **)(*param_1 + 0x30))();
        *(undefined4 *)(iVar1 + 0x54c) = 0x7f000000;
        iVar1 = (**(code **)(*param_1 + 0x30))();
        iVar1 = *(int *)(iVar1 + 0x380);
        *(undefined4 *)(iVar1 + 0x254) = 0x7f000000;
        *(undefined4 *)(iVar1 + 600) = 0;
        *(undefined4 *)(iVar1 + 0x25c) = 0;
        *(undefined4 *)(iVar1 + 0x260) = 0;
        if (param_2 != 0) {
          (**(code **)(*param_1 + 0x30))();
          FUN_005908f0();
        }
        (**(code **)(*param_1 + 0x30))();
        FUN_00734010();
        iVar1 = (**(code **)(*param_1 + 0x30))();
        if (*(int *)(iVar1 + 0x11ac) != 0) {
          iVar1 = (**(code **)(*param_1 + 0x30))();
          uVar4 = FUN_0073fec0(*(undefined4 *)(iVar1 + 4));
          FUN_0073fb90/*ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)*/(uVar4);
          iVar1 = FUN_007e6be0(uVar4);
          if (iVar1 != -1) {
            iVar2 = FUN_0073fb90/*ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)*/();
            *(undefined4 *)(iVar2 + 0x28 + iVar1 * 0x88) = 0xffffffff;
          }
        }
      }
    }
  }
  return;
}

```

## R217 FUNCTION CResGFF::AddStructToStruct  steam=0x00719800
GOG build: param_bytes=16 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* parentStruct, CResStruct* childStruct, char* labelText, DWORD structId) param_bytes=16
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

## R218 FUNCTION CServerExoApp::GetWorldTimer  steam=0x0051c370
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 0051c370 FUN_0051c370 size=31 ret=0 callers=0059fd20,006896d0,005d4830,0066c660,00556670,0056c310,007986a0,00781be0,005582f0,005d4700,00561f30,0079c3f0,00623330,00545520,00530170,005d18c0,00689770,00624b70,006070a0,00569320,0053af20,006c9900,00883560,00853a00,005d56b0,005d9da0,005a06f0,005d17f0,005a4d70,00692a00,006d9500,00533830,005c3f70,00576dc0,00662060,0059fcb0,00589ee0,005bfc00,0056b8a0,00545470,005d4370,005b96f0,005d4460,005d29f0,00557260,00521360,006897c0,00695140,005d4610,0062c360,00617980,0066c160,00544210,005a4bb0,00600f20,005f4c00,005b8000,00521e60,0055f520,005d9aa0,0061e360,0066c430,0055bd10,0053e830,00689720,007801a0,0062bfc0,00622c90,006c9c00,00798820 callees=

undefined4 __fastcall FUN_0051c370(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10048);
}

```

## R219 FUNCTION CSWCPlaceable::IsHostile  steam=0x00841180
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=66 ret_N=0
```c
// FN 00841180 FUN_00841180 size=66 ret=0 callers= callees=00842d00

undefined4 FUN_00841180(void)

{
  int iVar1;
  undefined4 local_14;
  
  iVar1 = FUN_00842d00/*CSWCPlaceable::GetServerPlaceable*/();
  if (iVar1 == 0) {
    local_14 = 0;
  }
  else {
    iVar1 = FUN_00842d00/*CSWCPlaceable::GetServerPlaceable*/();
    local_14 = *(undefined4 *)(iVar1 + 0x398);
  }
  return local_14;
}

```

## R220 FUNCTION CResGFF::ReadFieldVOID  steam=0x00719640
GOG build: param_bytes=24 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5, undefined4 param_6'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, CResStruct* structPtr, void* buffer, DWORD size, char* label, int* success, void* defaultValue) param_bytes=24
Steam function at this address: size=254 ret_N=24
```c
// FN 00719640 FUN_00719640 size=254 ret=24 callers=005285b0,005fbf60,00703530,00655b00,00655690,005582f0 callees=0091dba0,00717770,00717820,007178e0

void * FUN_00719640(int param_1,void *param_2,uint param_3,int param_4,undefined4 *param_5,
                   void *param_6)

{
  undefined4 uVar1;
  uint local_20;
  uint local_14;
  uint *local_10;
  int *local_c;
  uint local_8;
  
  if (((param_1 == 0) || (param_4 == 0)) || (param_2 == (void *)0x0)) {
    *param_5 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_1,param_4);
    local_c = (int *)FUN_00717770(param_1,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 0xd)) {
      *param_5 = 0;
    }
    else {
      local_10 = (uint *)FUN_00717820/*CResGFF::GetDataField*/(local_c,&local_14);
      if ((local_10 == (uint *)0x0) || (local_14 < 4)) {
        *param_5 = 0;
      }
      else {
        local_8 = *local_10;
        if (local_14 < local_8 + 4) {
          *param_5 = 0;
        }
        else {
          *param_5 = 1;
          local_20 = local_8;
          if (param_3 <= local_8) {
            local_20 = param_3;
          }
          _memcpy(param_2,local_10 + 1,local_20);
          param_6 = param_2;
        }
      }
    }
  }
  return param_6;
}

```

## R221 FUNCTION CSWCDoor::GetServerDoor  steam=0x0087be80
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=53 ret_N=0
```c
// FN 0087be80 FUN_0087be80 size=53 ret=0 callers=0087bec0,0079d840,0080c6b0,00778510,007b40f0,0087af40 callees=007f2540

undefined4 FUN_0087be80(void)

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
    uVar3 = (**(code **)(*piVar2 + 0x18))(piVar2,uVar3);
  }
  return uVar3;
}

```

## R222 FUNCTION CResGFF::CreateGFFFile  steam=0x0071a3d0
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResStruct* structPtr, CExoString* fileType, CExoString* version) param_bytes=12
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

## R223 OFFSET CSWGuiBorderParams.fill_angle = 0xc
Claim: CSWGuiBorder::Draw 0x415300 passes *(border+0x20) (params+0xc) as the rotation argument of the quad draw; operator= 0x4144f0 copies [3]; ctor zeroes it
Decompile 0x004144f0:
```c
// FN 004144f0 FUN_004144f0/*CSWGuiBorderParams::operator=*/ size=390 ret=4 callers=008b0fb0,008d0470,008b38e0,008a5ba0,00414c20 callees=00710ca0,00414920,00414980

undefined4 * __thiscall FUN_004144f0/*CSWGuiBorderParams::operator=*/(undefined4 *param_1,int param_2)

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
Decompile 0x00415300:
```c
// FN 00415300 FUN_00415300/*CSWGuiBorder::Draw*/ size=1886 ret=4 callers= callees=00414980,00415a60,00415d20,0047ea60

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00415300/*CSWGuiBorder::Draw*/(int param_1)

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
... (174 more lines)
```

## R224 OFFSET CRes.size = 0x18
Claim: loaders 0x713fb0/0x713bf0/0x713e80/0x7133a0 store resource-source size into param_2[6] (+0x18) before allocating the buffer; ctor zeroes param_1[6]
Decompile 0x007133a0:
```c
// FN 007133a0 FUN_007133a0 size=2117 ret=8 callers=00714300,00711c20 callees=00733570,00710930,009196fd,007335f0,00561430,007a1720,00733780,00739990,007398e0,0058c370,007397f0,00739970,00716d80,007399e0,00733df0,00919723,007337c0,00739880,00733540,00733a60,00712f30

int __thiscall FUN_007133a0(int param_1,int *param_2,int param_3)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  bool bVar5;
  int local_128;
  undefined1 local_a8 [8];
  int local_a0;
  int local_9c;
  int local_98;
  undefined1 local_94 [8];
  undefined1 local_8c [8];
  undefined1 local_84 [8];
  undefined1 local_7c [8];
  undefined1 local_74 [8];
  undefined1 local_6c [8];
  undefined1 local_64 [8];
  undefined1 local_5c [8];
  undefined1 local_54 [8];
  int local_4c;
  int local_48;
  int local_44;
  int local_40;
  undefined1 local_3c [8];
  int local_34;
  int local_30;
  undefined1 local_2c [8];
  int local_24;
  undefined4 local_20;
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095fa86;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1c = 0;
  local_30 = 0;
  FUN_00733540(uVar1);
  local_8 = 0;
  FUN_00733540();
... (207 more lines)
```
Decompile 0x00713bf0:
```c
// FN 00713bf0 FUN_00713bf0 size=644 ret=8 callers=00714300,00711c20 callees=00561430,007a1720,0058c370,00712f30

int __thiscall FUN_00713bf0(int param_1,int *param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  int local_10;
  int local_c;
  int local_8;
  
  local_c = 0;
  if ((param_2 != (int *)0x0) && ((param_2[3] & 4U) == 0)) {
    local_10 = FUN_007a1720();
    if (local_10 != 0) {
      local_c = FUN_00561430(local_10);
    }
    while (local_10 != 0) {
      if ((local_c != 0) &&
         ((*(uint *)(local_c + 0x28) & 0xfffffff) == (param_2[2] & 0xfc000U) >> 0xe)) {
        if (param_3 != 0) {
          (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 8))();
          local_8 = (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x24))
                              (param_2[2] & 0x3fff);
          param_2[6] = local_8;
          iVar1 = FUN_00712f30/*CExoResMan::Malloc*/(param_2);
          if (iVar1 != 0) {
            uVar2 = (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x20))();
            *(undefined4 *)(param_1 + 0x2c) = uVar2;
            (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x38))
                      (param_2[2],param_2[4],local_8,0);
            return 1;
          }
          (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x18))();
          return 0;
        }
        (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 4))();
        local_8 = (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x24))
                            (param_2[2] & 0x3fff);
        param_2[6] = local_8;
        iVar1 = FUN_00712f30/*CExoResMan::Malloc*/(param_2);
        if (iVar1 != 0) {
          iVar1 = (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x34))
                            (param_2[2],param_2[4],local_8,0);
          if (iVar1 == local_8) {
            (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x14))();
            iVar1 = (**(code **)(*param_2 + 0x10))();
            param_2[3] = param_2[3] & 0xfffffffbU | -(uint)(iVar1 != 0) & 4;
            return iVar1;
          }
... (12 more lines)
```
Decompile 0x00713e80:
```c
// FN 00713e80 FUN_00713e80/*CExoResMan::ServiceFromImage*/ size=297 ret=8 callers=00714300,00711c20 callees=00561430,007a1720,0058c370

undefined4 __thiscall FUN_00713e80/*CExoResMan::ServiceFromImage*/(undefined4 param_1,int *param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int local_10;
  int local_c;
  uint local_8;
  
  local_c = 0;
  if ((param_2 != (int *)0x0) && ((param_2[3] & 4U) == 0)) {
    local_8 = (param_2[2] & 0x3ff00000U) >> 0x14;
    local_10 = FUN_007a1720(param_1);
    if (local_10 != 0) {
      local_c = FUN_00561430(local_10);
    }
    while (local_10 != 0) {
      if ((local_c != 0) &&
         ((*(uint *)(local_c + 0x28) & 0xfffffff) == (param_2[2] & 0xfc000U) >> 0xe)) {
        uVar1 = param_2[2];
        iVar2 = (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x24))(uVar1 & 0x3fff);
        param_2[6] = iVar2;
        iVar2 = (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x44))(uVar1 & 0x3fff);
        param_2[4] = iVar2;
        param_2[3] = param_2[3] & 0xfffffffbU | 4;
        uVar3 = (**(code **)(*param_2 + 0x10))();
        return uVar3;
      }
      local_c = FUN_0058c370(&local_10);
    }
  }
  return 0;
}

```

## R225 FUNCTION CClientExoApp::GetClientLanguage  steam=0x0073f350
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=31 ret_N=0
```c
// FN 0073f350 FUN_0073f350 size=31 ret=0 callers=0040a460,00607790,0084a990,0082b990,008de2e0,00895260,008cbde0,00843500,008d0780,008ce3f0,008427a0,006cfd10,008bf280,006cae20,00605090,00896fb0,008b1250,008b3c00,00896d10,0058a610,0087c590,006c8de0,006afed0,006cc140,0077bcf0,008ddaf0,006ce710,0077c1b0,00896a80,007fc880,0077c340,0082c280,007fc390,0075dd10 callees=

undefined4 __fastcall FUN_0073f350(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x148);
}

```

## R226 FUNCTION CSWCObject::ShowShadowBlob  steam=0x00764fb0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=42 ret_N=0
```c
// FN 00764fb0 CPaneContainerManager::NotifyPaneDivider size=42 ret=0 callers= callees=

/* Library Function - Single Match
    public: void __thiscall CPaneContainerManager::NotifyPaneDivider(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CPaneContainerManager::NotifyPaneDivider(CPaneContainerManager *this)

{
  if (*(int *)(this + 0x68) != 0) {
    (**(code **)(**(int **)(this + 0x68) + 0xb0))();
  }
  return;
}

```

## R227 FUNCTION CSWGuiBorder::FillCenter  steam=0x00415d20
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

## R228 FUNCTION CResGFF::AddDataLayoutField  steam=0x00718470
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, int existingBlockOffset, DWORD sizeOfExistingBlock, DWORD dataLength) param_bytes=12
Steam function at this address: size=263 ret_N=12
```c
// FN 00718470 FUN_00718470 size=263 ret=12 callers=00718030 callees=0091dba0,009196fd,00919723

int __thiscall FUN_00718470(int param_1,int param_2,size_t param_3,int param_4)

{
  void *_Dst;
  undefined4 local_c;
  
  if ((*(int *)(param_1 + 0x40) == 0) || (*(int *)(param_1 + 100) == 0)) {
    local_c = 0;
  }
  else {
    local_c = *(int *)(*(int *)(param_1 + 0x40) + 0x2c);
    if (local_c == param_2 + param_3) {
      local_c = param_2;
    }
    if (*(uint *)(param_1 + 0x68) <= (uint)(local_c + param_4)) {
      *(int *)(param_1 + 0x68) = local_c + param_4 + *(int *)(param_1 + 0x38);
      _Dst = (void *)FUN_00919723/*operator new*/(*(undefined4 *)(param_1 + 0x68));
      _memcpy(_Dst,*(void **)(param_1 + 100),*(size_t *)(*(int *)(param_1 + 0x40) + 0x2c));
      FUN_009196fd/*operator delete*/(*(undefined4 *)(param_1 + 100));
      *(void **)(param_1 + 100) = _Dst;
    }
    if (local_c != param_2) {
      _memcpy((void *)(*(int *)(param_1 + 100) + local_c),
              (void *)(*(int *)(param_1 + 100) + param_2),param_3);
      *(size_t *)(param_1 + 0x6c) = *(int *)(param_1 + 0x6c) + param_3;
    }
    *(int *)(*(int *)(param_1 + 0x40) + 0x2c) = local_c + param_4;
    local_c = *(int *)(param_1 + 100) + local_c;
  }
  return local_c;
}

```

## R229 FUNCTION CResGFF::ReadFieldCExoLocString  steam=0x007190e0
GOG build: param_bytes=20 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr, void* out, CResStruct* structPtr, char* label, int* success, void* defaultValue) param_bytes=20
Steam function at this address: size=741 ret_N=20
```c
// FN 007190e0 FUN_007190e0 size=741 ret=20 callers=00625360,006c67e0,00628c40,0062af70,00619e60,0061a210,005582f0,006afed0,00601740,00637ce0,005269e0,00620250,008bf280,00523870,005ef570,006d0db0 callees=007352d0,00735360,00735240,00717770,00717820,007178e0,00735440,00733780,00733680

undefined4
FUN_007190e0(undefined4 param_1,int param_2,int param_3,undefined4 *param_4,undefined4 param_5)

{
  uint uVar1;
  uint uVar2;
  undefined4 uVar3;
  uint uVar4;
  uint local_30;
  undefined1 local_2c [4];
  int local_28;
  uint *local_24;
  uint local_20;
  uint local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ff89;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00735240(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 1;
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    FUN_007352d0(param_5);
    local_8 = local_8 & 0xffffff00;
    FUN_00735360/*CExoLocString::Destructor*/();
  }
  else {
    uVar3 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_2,param_3);
    local_18 = (int *)FUN_00717770(param_2,uVar3);
    if ((local_18 == (int *)0x0) || (*local_18 != 0xc)) {
      *param_4 = 0;
      FUN_007352d0(param_5);
      local_8 = local_8 & 0xffffff00;
      FUN_00735360/*CExoLocString::Destructor*/();
    }
    else {
      local_24 = (uint *)FUN_00717820/*CResGFF::GetDataField*/(local_18,&local_30);
      if ((local_24 == (uint *)0x0) || (local_30 < 4)) {
        *param_4 = 0;
        FUN_007352d0(param_5);
        local_8 = local_8 & 0xffffff00;
        FUN_00735360/*CExoLocString::Destructor*/();
      }
      else {
        local_14 = *local_24;
        if (local_30 < local_14 + 4U) {
          *param_4 = 0;
          FUN_007352d0(param_5);
          local_8 = local_8 & 0xffffff00;
          FUN_00735360/*CExoLocString::Destructor*/();
        }
        else {
          local_28 = local_24[1];
          uVar1 = local_24[2];
          local_24 = local_24 + 3;
          local_14 = local_14 + -8;
          for (local_1c = 0; local_1c < uVar1; local_1c = local_1c + 1) {
            local_20 = *local_24;
            uVar2 = local_24[1];
            local_24 = local_24 + 2;
            local_14 = local_14 + -8;
            uVar3 = FUN_00733680(local_24,uVar2);
... (31 more lines)
```

## R230 FUNCTION CClientExoApp::SendLoadGameRequest  steam=0x0073f6f0
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

## R231 FUNCTION Scene::ChildAdd  steam=0x00465bb0
GOG build: param_bytes=4 ghidra_sig='Scene * this, Gob * param_1'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr, void* child) param_bytes=4
Steam function at this address: size=112 ret_N=4
```c
// FN 00465bb0 FUN_00465bb0 size=112 ret=4 callers= callees=004659f0,00495f20,00473f40

void __thiscall FUN_00465bb0(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  if (param_2[0x21] != 0) {
    FUN_00495f20(param_1,param_2);
    FUN_004659f0(param_2[0x26]);
  }
  FUN_00473f40(param_2);
  iVar1 = (**(code **)(*param_2 + 0x1a8))();
  if (iVar1 != 0) {
    FUN_00473f40(param_2);
  }
  return;
}

```

## R232 FUNCTION CResGFF::ReadFieldVector  steam=0x00719520
GOG build: param_bytes=20 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4 param_4, undefined4 param_5'
Wrapper typedef (what calling code expects): Vector* (__thiscall)(void* thisPtr, Vector* out, CResStruct* structPtr, char* label, int* success, Vector* defaultValue) param_bytes=20
Steam function at this address: size=288 ret_N=20
```c
// FN 00719520 FUN_00719520 size=288 ret=20 callers=006c67e0,00836350,00528710,00415080,00417480,0041ffd0,0040ee40,00561d80 callees=00717770,00717820,007178e0

undefined4 *
FUN_00719520(undefined4 *param_1,int param_2,int param_3,undefined4 *param_4,undefined4 *param_5)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  uint local_10 [2];
  int *local_8;
  
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    *param_1 = *param_5;
    param_1[1] = param_5[1];
    param_1[2] = param_5[2];
  }
  else {
    uVar2 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_2,param_3);
    local_8 = (int *)FUN_00717770(param_2,uVar2);
    if ((local_8 == (int *)0x0) || (*local_8 != 0x11)) {
      *param_4 = 0;
      *param_1 = *param_5;
      param_1[1] = param_5[1];
      param_1[2] = param_5[2];
    }
    else {
      puVar3 = (undefined4 *)FUN_00717820/*CResGFF::GetDataField*/(local_8,local_10);
      if ((puVar3 == (undefined4 *)0x0) || (local_10[0] < 0xc)) {
        *param_4 = 0;
        *param_1 = *param_5;
        param_1[1] = param_5[1];
        param_1[2] = param_5[2];
      }
      else {
        *param_4 = 1;
        uVar2 = puVar3[1];
        uVar1 = puVar3[2];
        *param_1 = *puVar3;
        param_1[1] = uVar2;
        param_1[2] = uVar1;
      }
    }
  }
  return param_1;
}

```

## R233 FUNCTION CSWCObject::GetPortraitId  steam=0x007ed4b0
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): WORD (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=21 ret_N=0
```c
// FN 007ed4b0 FUN_007ed4b0 size=21 ret=0 callers= callees=

undefined2 __fastcall FUN_007ed4b0(int param_1)

{
  return *(undefined2 *)(param_1 + 0xb8);
}

```

## R234 FUNCTION CSWSPlaceable::GetBodyBagAppearance  steam=0x005f6630
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): BYTE (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=384 ret_N=0
```c
// FN 005f6630 FUN_005f6630 size=384 ret=0 callers=00549780 callees=0071bcf0,00733570,00733780

uint FUN_005f6630(void)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int extraout_var;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined1 local_3c [11];
  char local_31;
  undefined1 local_30 [11];
  char local_25;
  undefined1 local_24 [11];
  char local_19;
  undefined4 local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954708;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570/*CExoString::CExoString_char*/("Appearance");
  local_8 = 0;
  iVar4 = extraout_var;
  iVar2 = FUN_0071bcf0(*(undefined1 *)(extraout_var + 0x3ec),local_24,&local_14);
  local_19 = '\x01' - (iVar2 != 0);
  local_8 = 0xffffffff;
  FUN_00733780/*CExoString::~CExoString*/(uVar1);
  if (local_19 != '\0') {
    FUN_00733570/*CExoString::CExoString_char*/("BodyBag");
    local_8 = 1;
    uVar6 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x10);
    iVar2 = FUN_0071bcf0(*(undefined4 *)(iVar4 + 0x270),local_30,&local_18);
    local_25 = '\x01' - (iVar2 != 0);
    local_8 = 0xffffffff;
    FUN_00733780/*CExoString::~CExoString*/();
    if (local_25 != '\0') {
      local_18 = 0;
    }
    FUN_00733570/*CExoString::CExoString_char*/("Appearance");
    local_8 = 2;
    iVar2 = *(int *)(DAT_00a1b4d0 + 0x130);
    uVar5 = *(undefined4 *)(iVar2 + 0xd0);
    iVar3 = FUN_0071bcf0(local_18,local_3c,&local_14);
    local_31 = '\x01' - (iVar3 != 0);
    local_8 = 0xffffffff;
    FUN_00733780/*CExoString::~CExoString*/(uVar1,iVar4,uVar5,iVar2,uVar6);
    if (local_31 != '\0') {
      local_14 = 3;
    }
  }
  ExceptionList = local_10;
  return local_14 & 0xff;
}

```

## R235 FUNCTION CResGFF::GetList  steam=0x00718760
GOG build: param_bytes=12 ghidra_sig='undefined4 param_1, undefined4 param_2, undefined4 param_3'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, CResList* outList, CResStruct* structPtr, char* label) param_bytes=12
Steam function at this address: size=164 ret_N=12
```c
// FN 00718760 FUN_00718760 size=164 ret=12 callers=00625360,005e2900,00525f90,006fef80,0053fd50,006564c0,00638bd0,00528710,005753f0,005269e0,0069d1e0,00528080,006b3b70,006c67e0,005266f0,00630810,005e9370,006c53f0,0040ee40,005e2090,005f7cc0,006815e0,00527cb0,0058d5e0,00620250,00526f60,00655690,006295d0,0051eed0,00527330,005263c0,006029f0,00833ff0,0052ccd0,00573950,006d0db0,005e5510,0062af70,0083db30,00527580,005f9220,005409f0,00527890,00655b00,005582f0,006afed0,00601740,0055def0,00592840,00637ce0,005fbf60,005e6f80,00630f20,005406e0,00523870,005ef570 callees=0091d0f0,00717770,007178e0,00717880

undefined4 FUN_00718760(undefined4 *param_1,undefined4 *param_2,char *param_3)

{
  undefined4 uVar1;
  uint local_10;
  int local_c;
  int *local_8;
  
  if (((param_1 == (undefined4 *)0x0) || (param_2 == (undefined4 *)0x0)) || (param_3 == (char *)0x0)
     ) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007178e0/*CResGFF::GetFieldByLabel*/(param_2,param_3);
    local_8 = (int *)FUN_00717770(param_2,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 0xf)) {
      uVar1 = 0;
    }
    else {
      local_c = FUN_00717880/*CResGFF::GetDataLayoutList*/(local_8,&local_10);
      if ((local_c == 0) || (local_10 < 4)) {
        uVar1 = 0;
      }
      else {
        *param_1 = *param_2;
        _strncpy((char *)(param_1 + 1),param_3,0x10);
        uVar1 = 1;
      }
    }
  }
  return uVar1;
}

```

## R236 FUNCTION CSWSObject::GetAIStateReputation  steam=0x005e1680
GOG build: param_bytes=4 ghidra_sig='undefined4 param_1'
Wrapper typedef (what calling code expects): BYTE (__thiscall)(void* thisPtr, DWORD objectId) param_bytes=4
Steam function at this address: size=159 ret_N=4
```c
// FN 005e1680 FUN_005e1680 size=159 ret=4 callers=006424b0,00634b20,00585480,0057c990,00568540,0057bb90,00647d30,0057b280,00645460,00623fe0,00642ba0,006430a0 callees=0051c080,005453c0,0053dfb0,005e13c0

undefined4 FUN_005e1680(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int **ppiVar5;
  undefined4 uVar6;
  int extraout_var;
  int iVar7;
  int iVar8;
  int *local_c;
  undefined4 local_8;
  
  local_8 = 0;
  iVar8 = 0x32;
  uVar1 = FUN_005453c0();
  uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  ppiVar5 = &local_c;
  iVar7 = extraout_var;
  FUN_0051c080(param_1,ppiVar5);
  FUN_0053dfb0/*CGameObjectArray::GetGameObject*/(param_1,ppiVar5);
  if (local_c != (int *)0x0) {
    iVar2 = (**(code **)(*local_c + 0x10))();
    if (iVar2 != 0) {
      uVar4 = *(undefined4 *)(iVar7 + 4);
      uVar6 = local_8;
      (**(code **)(*local_c + 0x10))(uVar4,local_8,iVar7,uVar4,uVar3,iVar8,uVar1);
      iVar8 = FUN_005e13c0(uVar4,uVar6);
    }
  }
  if (iVar8 < 0x5a) {
    if (iVar8 < 0xb) {
      uVar3 = 2;
    }
    else {
      uVar3 = 0;
    }
  }
  else {
    uVar3 = 1;
  }
  return uVar3;
}

```

## R237 FUNCTION CSWSTrigger::GetCanFireMineOnObject  steam=0x00622750
GOG build: param_bytes=8 ghidra_sig='undefined4 param_1, undefined4 param_2'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr, DWORD objectId, int skipFactionCheck) param_bytes=8
Steam function at this address: size=469 ret_N=8
```c
// FN 00622750 FUN_00622750 size=469 ret=8 callers=00622930 callees=006b83f0,0077d800,0051c100,007e5da0,006bc570,0073fb90,005e13c0,005d0cb0

undefined4 __thiscall FUN_00622750(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 local_c;
  
  local_c = 1;
  iVar1 = FUN_0051c100/*CServerExoApp::GetCreatureByGameObjectID*/(*(undefined4 *)(param_1 + 0x328));
  iVar2 = FUN_0051c100/*CServerExoApp::GetCreatureByGameObjectID*/(param_2);
  if (iVar2 != 0) {
    if (param_3 == 0) {
      if (iVar1 == 0) {
        iVar1 = FUN_005e13c0(*(undefined4 *)(param_1 + 4),0);
        if ((10 < iVar1) || (*(int *)(*(int *)(iVar2 + 0x1198) + 0x78) == *(int *)(param_1 + 0x2f8))
           ) {
          local_c = 0;
        }
      }
      else {
        iVar3 = FUN_005d0cb0();
        if ((iVar3 != 0) &&
           ((iVar1 = FUN_005e13c0(*(undefined4 *)(iVar1 + 4),0), 10 < iVar1 ||
            (*(int *)(param_1 + 0x2f8) == *(int *)(*(int *)(iVar2 + 0x1198) + 0x78))))) {
          local_c = 0;
        }
      }
    }
    iVar1 = FUN_006bc570(DAT_009961d0,0);
    if (iVar1 != 0) {
      local_c = 0;
    }
    iVar1 = FUN_006b83f0(0xee);
    if (iVar1 != 0) {
      local_c = 0;
    }
    if (*(int *)(iVar2 + 0x11ac) != 0) {
      uVar4 = 0;
      FUN_0073fb90/*ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)*/(0);
      iVar1 = FUN_007e5da0/*Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)*/(uVar4);
      if (((iVar1 != 0) && (iVar1 = FUN_0077d800(), iVar1 != 0)) &&
         (iVar1 = FUN_006b83f0(0xef), iVar1 != 0)) {
        local_c = 0;
      }
    }
  }
  return local_c;
}

```

## R238 FUNCTION Scene::UpdateEmitters  steam=0x0046c190
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=281 ret_N=0
```c
// FN 0046c190 FUN_0046c190 size=281 ret=0 callers=0046c2b0 callees=004ec080

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0046c190(int param_1)

{
  int iVar1;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  DAT_00a20020 = param_1;
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0xd8); local_8 = local_8 + 1) {
    iVar1 = *(int *)(*(int *)(*(int *)(param_1 + 0xd4) + local_8 * 4) + 0x58);
    for (local_10 = 0; local_10 < *(int *)(iVar1 + 0x28); local_10 = local_10 + 1) {
      for (local_c = 0; local_c < 100; local_c = local_c + 1) {
        FUN_004ec080(_DAT_009876fc);
      }
    }
  }
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0xa4); local_8 = local_8 + 1) {
    for (local_c = 0; local_c < 100; local_c = local_c + 1) {
      FUN_004ec080(_DAT_009876fc);
    }
  }
  DAT_00a20020 = 0;
  return;
}

```

## R239 FUNCTION CSWSDoor::GetFirstName  steam=0x00616f40
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=19 ret_N=0
```c
// FN 00616f40 FUN_00616f40 size=19 ret=0 callers= callees=

int __fastcall FUN_00616f40(int param_1)

{
  return param_1 + 0x3ec;
}

```

## R240 FUNCTION CGuiInGame::DoQuickLoad  steam=0x007d0f50
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): void (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=1503 ret_N=0
```c
// FN 007d0f50 FUN_007d0f50 size=1503 ret=0 callers=007b12c0 callees=00714f30,0071e910,007d0af0,007d1530,00711710,00740d90,0073f750,00740980,0071ebd0,0071dee0,0040df30,007337c0,0040def0,007156e0,004109c0,00733570,007335f0,00740d30,00740540,00733780,0070bc60,007405a0,007ca060,00736050,0073ff20,00740dc0,00855410,00410530,0051ce00,005ff000,0075bb00,0073f720,00919723,00733540,0075ca30,00855f30,00740ce0,00740f00

void __fastcall FUN_007d0f50(int param_1)

{
  int iVar1;
  undefined4 *local_c8;
  undefined1 local_5c [19];
  char local_49;
  undefined1 local_48 [8];
  int local_40;
  undefined1 local_3c [8];
  int *local_34;
  SchedulerResourceManagement *local_30;
  undefined4 *local_2c;
  int local_20;
  undefined1 local_1c [4];
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969df6;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0040def0();
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  FUN_00733570/*CExoString::CExoString_char*/();
  local_8._0_1_ = 2;
  FUN_00736050/*CExoBase::GetDirectoryList*/(local_1c,local_48,0xffff);
  local_8._0_1_ = 1;
  FUN_00733780/*CExoString::~CExoString*/();
  FUN_00733570/*CExoString::CExoString_char*/();
  local_8._0_1_ = 3;
  iVar1 = FUN_00714f30();
  local_49 = '\x01' - (iVar1 != 0);
  local_8._0_1_ = 1;
  FUN_00733780/*CExoString::~CExoString*/();
  if (local_49 != '\0') {
    FUN_00733570/*CExoString::CExoString_char*/();
    local_8._0_1_ = 4;
    FUN_007156e0(local_5c);
    local_8._0_1_ = 1;
    FUN_00733780/*CExoString::~CExoString*/();
  }
  local_20 = 0;
  while( true ) {
    if (local_18 <= local_20) {
      FUN_00733570/*CExoString::CExoString_char*/();
      local_8._0_1_ = 9;
      FUN_00711710();
      local_8._0_1_ = 1;
      FUN_00733780/*CExoString::~CExoString*/();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780/*CExoString::~CExoString*/();
      local_8 = 0xffffffff;
      FUN_0040df30();
      ExceptionList = local_10;
      return;
    }
    iVar1 = FUN_00919723/*operator new*/();
    local_8._0_1_ = 5;
    if (iVar1 == 0) {
      local_c8 = (undefined4 *)0x0;
    }
    else {
      local_c8 = (undefined4 *)FUN_00855410();
... (86 more lines)
```

## R241 FUNCTION CSWSObject::SpawnBodyBag  steam=0x00549780
GOG build: param_bytes=0 ghidra_sig='void'
Wrapper typedef (what calling code expects): int (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=1861 ret_N=0
```c
// FN 00549780 FUN_00549780 size=1861 ret=0 callers=0056efd0,005f2fe0 callees=00919723,0051e170,00735360,00515620,0071bcf0,00733570,00515d50,00735240,005f5160,00588d50,005812e0,005f0f10,00735400,00515dd0,005f6990,005453c0,005f67b0,0064b6a0,005f6630,0051c280,005eed40,0057e3f0,0051c2e0,00733780

undefined4 __fastcall FUN_00549780(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  float10 fVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 *puVar7;
  undefined4 *local_120;
  undefined4 *local_118;
  undefined1 local_a8 [16];
  undefined1 local_98 [8];
  int local_90;
  undefined1 local_8c [8];
  int local_84;
  undefined4 *local_80;
  undefined4 *local_7c;
  undefined4 *local_78;
  undefined4 *local_74;
  undefined4 *local_70;
  undefined4 *local_6c;
  int local_68;
  undefined4 *local_64;
  undefined1 local_60 [8];
  int local_58;
  undefined1 local_54 [11];
  char local_49;
  int local_48;
  uint local_44;
  int local_40;
  undefined4 *local_3c;
  undefined4 local_38;
  undefined1 local_34 [4];
  undefined4 local_30;
  uint local_2c;
  undefined4 *local_28;
  int local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094fbb9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (((char)param_1[2] == DAT_00994474) || ((char)param_1[2] == DAT_00994478)) {
    local_24 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_40 = (**(code **)(*param_1 + 0x48))();
    if ((local_24 == 0) && (local_40 == 0)) {
      uVar1 = 0x7f000000;
    }
    else {
      iVar2 = param_1[1];
      FUN_0051c2e0(iVar2);
      FUN_0064b6a0(iVar2);
      iVar2 = FUN_005453c0();
      if (iVar2 == 0) {
        uVar1 = 0x7f000000;
      }
      else {
        local_2c = 0;
        if (local_24 == 0) {
... (158 more lines)
```

## R242 FUNCTION CSWGuiBorder::Destructor  steam=0x00414bf0
GOG build: param_bytes=4 ghidra_sig='CSWGuiBorder * this, uint param_1'
Wrapper typedef (what calling code expects): void* (__thiscall)(void* thisPtr) param_bytes=0
Steam function at this address: size=44 ret_N=4
```c
// FN 00414bf0 FUN_00414bf0 size=44 ret=4 callers= callees=00414c60,009196fd

undefined4 __thiscall FUN_00414bf0(undefined4 param_1,uint param_2)

{
  FUN_00414c60/*CSWGuiBorder::Destructor_2*/();
  if ((param_2 & 1) != 0) {
    FUN_009196fd/*operator delete*/(param_1);
  }
  return param_1;
}

```
