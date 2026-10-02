# Digest C_CResGFF

### CResGFF::Destructor  gog=0x561f80 (param_bytes=4, notes='undefined4 param_1')
NO CANDIDATE (bracketing failed) -- see manual list

### CResGFF::Constructor  gog=0x622ff0 (param_bytes=0, notes='void')
steam: cand=0x00716e60(mapper) delta=0xf3e70 align=0/6 size=312 retn=0 gapG/S=320/320 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=26 callees=0091c630, 007110d0=CRes::CRes
```
undefined4 * __fastcall FUN_00716e60(undefined4 *param_1)
{
  FUN_007110d0();
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
  return param_1;
}
```

### CResGFF::Constructor_2  gog=0x623130 (param_bytes=12, notes='CResGFF * this, undefined2 param_1, undefined4 param_2, undefined4 par')
steam: cand=0x00716fd0(mapper) delta=0xf3ea0 align=3/6 size=465 retn=12 gapG/S=480/480 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, ResourceType resourceType, char* GFFtype, void* templateResRef) [param_bytes=12]
callers=25 callees=007111e0=CRes::Demand, 00713010=CExoResMan::ReleaseResObject, 0091d0f0, 0091c630, 007110d0=CRes::CRes, 00712b60=CExoResMan::SetResObject
```
undefined4 * __thiscall
FUN_00716fd0(undefined4 *param_1,undefined2 param_2,char *param_3,undefined4 param_4)
{
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ff08;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_007110d0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
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
  FUN_007111e0();
  if (param_1[4] == 0) {
    FUN_00713010(param_1);
  }
  else {
    param_1[0x27] = 1;
... (4 more lines)
```

### CResGFF::Destructor_2  gog=0x623310 (param_bytes=0, notes='void')
steam: cand=0x007171b0(mapper) delta=0xf3ea0 align=4/6 size=329 retn=0 gapG/S=336/336 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, byte shouldFree) [param_bytes=4]
callers=3 callees=009196fd=operator delete, 00717630=gog:CResGFF::ReleaseResource, 007111a0=gog:CRes::Destructor_2
```
void __fastcall FUN_007171b0(undefined4 *param_1)
{
  void *local_10;
  puStack_c = &LAB_0095ff38;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CResGFF::vftable;
  local_8 = 0;
  if (param_1[0x26] == 0) {
    FUN_009196fd(param_1[0x1c]);
    param_1[0x1c] = 0;
    FUN_009196fd(param_1[0x19]);
    param_1[0x19] = 0;
    FUN_009196fd(param_1[0x17]);
    param_1[0x17] = 0;
    FUN_009196fd(param_1[0x15]);
    param_1[0x15] = 0;
    FUN_009196fd(param_1[0x13]);
    param_1[0x13] = 0;
    FUN_009196fd(param_1[0x11]);
    param_1[0x11] = 0;
    FUN_009196fd(param_1[0x10]);
    param_1[0x10] = 0;
  }
  else {
    FUN_00717630(uVar1);
  }
  local_8 = 0xffffffff;
  FUN_007111a0();
  ExceptionList = local_10;
  return;
}
```

### CResGFF::OnResourceServiced  gog=0x623460 (param_bytes=0, notes='void')
steam: cand=0x00717300(mapper) delta=0xf3ea0 align=5/6 size=680 retn=0 gapG/S=688/688 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=009196fd=operator delete
```
undefined4 __fastcall FUN_00717300(int param_1)
{
  if (*(int *)(param_1 + 0x10) == 0) {
    uVar1 = 0;
  }
  else {
    if ((*(int *)(param_1 + 0x40) != 0) && (*(int *)(param_1 + 0x98) == 0)) {
      FUN_009196fd(*(undefined4 *)(param_1 + 0x70));
      *(undefined4 *)(param_1 + 0x70) = 0;
      FUN_009196fd(*(undefined4 *)(param_1 + 100));
      *(undefined4 *)(param_1 + 100) = 0;
      FUN_009196fd(*(undefined4 *)(param_1 + 0x5c));
      *(undefined4 *)(param_1 + 0x5c) = 0;
      FUN_009196fd(*(undefined4 *)(param_1 + 0x54));
      *(undefined4 *)(param_1 + 0x54) = 0;
      FUN_009196fd(*(undefined4 *)(param_1 + 0x4c));
      *(undefined4 *)(param_1 + 0x4c) = 0;
      FUN_009196fd(*(undefined4 *)(param_1 + 0x44));
      *(undefined4 *)(param_1 + 0x44) = 0;
      FUN_009196fd(*(undefined4 *)(param_1 + 0x40));
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
... (20 more lines)
```

### CResGFF::OnResourceFreed  gog=0x623710 (param_bytes=0, notes='void')
steam: cand=0x007175b0(mapper) delta=0xf3ea0 align=6/6 size=124 retn=0 gapG/S=128/128 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
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

### CResGFF::ReleaseResource  gog=0x623790 (param_bytes=0, notes='void')
steam: cand=0x00717630(mapper) delta=0xf3ea0 align=6/6 size=234 retn=0 gapG/S=240/240 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=3 callees=007112b0=CRes::Release, 00713010=CExoResMan::ReleaseResObject
```
void __fastcall FUN_00717630(int param_1)
{
  if (*(int *)(param_1 + 0x98) != 0) {
    if (*(int *)(param_1 + 0x9c) != 0) {
      FUN_007112b0();
      FUN_00713010(param_1);
      *(undefined4 *)(param_1 + 0x9c) = 0;
    }
    *(undefined4 *)(param_1 + 0x98) = 0;
  }
  *(undefined4 *)(param_1 + 0x40) = 0;
  *(undefined4 *)(param_1 + 0x44) = 0;
  *(undefined4 *)(param_1 + 0x48) = 0;
  *(undefined4 *)(param_1 + 0x4c) = 0;
  *(undefined4 *)(param_1 + 0x50) = 0;
  *(undefined4 *)(param_1 + 0x54) = 0;
  *(undefined4 *)(param_1 + 0x58) = 0;
  *(undefined4 *)(param_1 + 0x5c) = 0;
  *(undefined4 *)(param_1 + 0x60) = 0;
  *(undefined4 *)(param_1 + 100) = 0;
  *(undefined4 *)(param_1 + 0x68) = 0;
  *(undefined4 *)(param_1 + 0x6c) = 0;
  *(undefined4 *)(param_1 + 0x70) = 0;
  *(undefined4 *)(param_1 + 0x74) = 0;
  *(undefined4 *)(param_1 + 0x78) = 0;
  return;
}
```

### CResGFF::GetField  gog=0x6238d0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00717770(mapper) delta=0xf3ea0 align=6/6 size=171 retn=8 gapG/S=176/176 cc_guess=thiscall
wrapper: GFFFieldData* (__thiscall) (void* thisPtr, CResGFFStruct* structData, DWORD index) [param_bytes=8]
callers=20 callees=00717720=gog:CResGFF::GetStruct
```
int __thiscall FUN_00717770(int param_1,undefined4 param_2,uint param_3)
{
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

### CResGFF::GetDataField  gog=0x623980 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00717820(mapper) delta=0xf3ea0 align=6/6 size=81 retn=8 gapG/S=96/96 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResGFFField* field, DWORD* dataSize) [param_bytes=8]
callers=7 callees=
```
int __thiscall FUN_00717820(int param_1,int param_2,int *param_3)
{
  if (((param_2 == 0) || (*(int *)(param_1 + 0x5c) == 0)) ||
     (*(uint *)(*(int *)(param_1 + 0x40) + 0x24) <= *(uint *)(param_2 + 8))) {
    iVar1 = 0;
  }
  else {
    *param_3 = *(int *)(*(int *)(param_1 + 0x40) + 0x24) - *(int *)(param_2 + 8);
    iVar1 = *(int *)(param_1 + 0x5c) + *(int *)(param_2 + 8);
  }
  return iVar1;
}
```

### CResGFF::GetDataLayoutList  gog=0x6239e0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00717880(mapper) delta=0xf3ea0 align=6/6 size=81 retn=8 gapG/S=96/96 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResGFFField* field, DWORD* dataLength) [param_bytes=8]
callers=4 callees=
```
int __thiscall FUN_00717880(int param_1,int param_2,int *param_3)
{
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

### CResGFF::GetFieldByLabel  gog=0x623a40 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x007178e0(mapper) delta=0xf3ea0 align=6/6 size=269 retn=8 gapG/S=288/288 cc_guess=thiscall
wrapper: DWORD (__thiscall) (void* thisPtr, CResStruct* structPtr, char* label) [param_bytes=8]
callers=20 callees=007186c0=gog:CResGFF::GetFieldCount, 0091d0f0, 00717a00=gog:CResGFF::GetField_2, 0091c630, 00717720=gog:CResGFF::GetStruct, 009196ee=__security_check_cookie
```
void __thiscall FUN_007178e0(int param_1,undefined4 param_2,char *param_3)
{
  local_1c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if ((param_3 != (char *)0x0) && (*(int *)(param_1 + 0x54) != 0)) {
    _memset(&local_2c,0,0x10);
    _strncpy((char *)&local_2c,param_3,0x10);
    uVar1 = FUN_00717720(param_2);
    local_18 = FUN_007186c0(uVar1);
    local_14 = 0;
    while ((((local_14 < local_18 && (local_c = FUN_00717a00(uVar1,local_14), local_c != 0)) &&
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

### CResGFF::GetField_2  gog=0x623b60 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00717a00(mapper) delta=0xf3ea0 align=6/6 size=156 retn=8 gapG/S=160/160 cc_guess=thiscall
wrapper: CResGFFField* (__thiscall) (void* thisPtr, CResStruct* structPtr, DWORD fieldIndex) [param_bytes=8]
callers=1 callees=
```
int __thiscall FUN_00717a00(int param_1,int param_2,uint param_3)
{
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

### CResGFF::InitializeForWriting  gog=0x623c00 (param_bytes=0, notes='void')
steam: cand=0x00717aa0(mapper) delta=0xf3ea0 align=6/6 size=1157 retn=0 gapG/S=1168/1168 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=18 callees=0091dba0=_memcpy, 009196fd=operator delete, 00717630=gog:CResGFF::ReleaseResource, 00919723=operator new
```
void __fastcall FUN_00717aa0(int param_1)
{
  void *_Dst;
  void *_Dst_00;
  void *_Dst_01;
  void *_Dst_02;
  void *_Dst_03;
  void *_Dst_04;
  puVar2 = (undefined4 *)FUN_00919723(0x38);
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
... (50 more lines)
```

### CResGFF::AddStruct  gog=0x624090 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00717f30(mapper) delta=0xf3ea0 align=6/6 size=251 retn=4 gapG/S=256/256 cc_guess=thiscall
wrapper: DWORD (__thiscall) (void* thisPtr, DWORD id) [param_bytes=4]
callers=3 callees=0091dba0=_memcpy, 009196fd=operator delete, 00919723=operator new
```
uint __thiscall FUN_00717f30(int param_1,undefined4 param_2)
{
  void *_Dst;
  if ((*(int *)(param_1 + 0x40) == 0) || (*(int *)(param_1 + 0x44) == 0)) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = *(uint *)(*(int *)(param_1 + 0x40) + 0xc);
    if (*(uint *)(param_1 + 0x48) <= uVar2) {
      *(uint *)(param_1 + 0x48) = uVar2 + *(int *)(param_1 + 0x28);
      lVar1 = (ulonglong)*(uint *)(param_1 + 0x48) * 0xc;
      _Dst = (void *)FUN_00919723(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
      _memcpy(_Dst,*(void **)(param_1 + 0x44),uVar2 * 0xc);
      FUN_009196fd(*(undefined4 *)(param_1 + 0x44));
      *(void **)(param_1 + 0x44) = _Dst;
    }
    *(uint *)(*(int *)(param_1 + 0x40) + 0xc) = uVar2 + 1;
    *(undefined4 *)(uVar2 * 0xc + *(int *)(param_1 + 0x44)) = param_2;
    *(undefined4 *)(*(int *)(param_1 + 0x44) + 4 + uVar2 * 0xc) = 0xffffffff;
    *(undefined4 *)(*(int *)(param_1 + 0x44) + 8 + uVar2 * 0xc) = 0;
  }
  return uVar2;
}
```

### CResGFF::AddField  gog=0x624190 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00718030(mapper) delta=0xf3ea0 align=6/6 size=443 retn=12 gapG/S=448/448 cc_guess=thiscall
wrapper: CResGFFField* (__thiscall) (void* thisPtr, CResStruct* structPtr, char* labelText, GFFFieldTypes fieldType) [param_bytes=12]
callers=16 callees=007181f0=gog:CResGFF::AddLabel, 0091dba0=_memcpy, 009196fd=operator delete, 00717720=gog:CResGFF::GetStruct, 00919723=operator new, 00718470=gog:CResGFF::AddDataLayoutField
```
int __thiscall FUN_00718030(int param_1,undefined4 param_2,int param_3,undefined4 param_4)
{
  void *_Dst;
  if (param_3 == 0) {
    iVar4 = 0;
  }
  else {
    iVar4 = FUN_00717720(param_2);
    if (iVar4 == 0) {
      iVar4 = 0;
    }
    else {
      uVar1 = *(uint *)(*(int *)(param_1 + 0x40) + 0x14);
      if (*(uint *)(param_1 + 0x50) <= uVar1) {
        *(uint *)(param_1 + 0x50) = uVar1 + *(int *)(param_1 + 0x2c);
        lVar3 = (ulonglong)*(uint *)(param_1 + 0x50) * 0xc;
        _Dst = (void *)FUN_00919723(-(uint)((int)((ulonglong)lVar3 >> 0x20) != 0) | (uint)lVar3);
        _memcpy(_Dst,*(void **)(param_1 + 0x4c),uVar1 * 0xc);
        FUN_009196fd(*(undefined4 *)(param_1 + 0x4c));
        *(void **)(param_1 + 0x4c) = _Dst;
      }
      *(uint *)(*(int *)(param_1 + 0x40) + 0x14) = uVar1 + 1;
      iVar2 = *(int *)(iVar4 + 8);
      if (iVar2 == 0) {
        *(uint *)(iVar4 + 4) = uVar1;
      }
      else {
        if (iVar2 == 1) {
          local_10 = (undefined4 *)FUN_00718470(0xffffffff,0,8);
          if (local_10 == (undefined4 *)0x0) {
            return 0;
          }
          *local_10 = *(undefined4 *)(iVar4 + 4);
        }
        else {
          local_10 = (undefined4 *)FUN_00718470(*(undefined4 *)(iVar4 + 4),iVar2 << 2,iVar2 * 4 + 4)
          ;
          if (local_10 == (undefined4 *)0x0) {
            return 0;
          }
        }
        local_10[iVar2] = uVar1;
        *(int *)(iVar4 + 4) = (int)local_10 - *(int *)(param_1 + 100);
      }
      *(int *)(iVar4 + 8) = iVar2 + 1;
... (9 more lines)
```

### CResGFF::AddLabel  gog=0x624350 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007181f0(mapper) delta=0xf3ea0 align=6/6 size=371 retn=4 gapG/S=384/384 cc_guess=thiscall
wrapper: DWORD (__thiscall) (void* thisPtr, char* text) [param_bytes=4]
callers=1 callees=0091dba0=_memcpy, 009196fd=operator delete, 0091d0f0, 0091c630, 00919723=operator new, 009196ee=__security_check_cookie
```
void __thiscall FUN_007181f0(int param_1,char *param_2)
{
  void *local_c;
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
      local_c = (void *)FUN_00919723(-(uint)((int)((ulonglong)lVar2 >> 0x20) != 0) | (uint)lVar2);
      _memcpy(local_c,*(void **)(param_1 + 0x54),uVar1 << 4);
      FUN_009196fd(*(undefined4 *)(param_1 + 0x54));
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

### CResGFF::AddDataField  gog=0x6244d0 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00718370(mapper) delta=0xf3ea0 align=6/6 size=248 retn=12 gapG/S=256/256 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, int existingBlockOffset, DWORD sizeOfExistingBlock, DWORD dataLength) [param_bytes=12]
callers=7 callees=0091dba0=_memcpy, 009196fd=operator delete, 00919723=operator new
```
int __thiscall FUN_00718370(int param_1,int param_2,size_t param_3,int param_4)
{
  void *_Dst;
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
      _Dst = (void *)FUN_00919723(*(undefined4 *)(param_1 + 0x60));
      _memcpy(_Dst,*(void **)(param_1 + 0x5c),*(size_t *)(*(int *)(param_1 + 0x40) + 0x24));
      FUN_009196fd(*(undefined4 *)(param_1 + 0x5c));
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

### CResGFF::AddDataLayoutField  gog=0x6245d0 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00718470(mapper) delta=0xf3ea0 align=6/6 size=263 retn=12 gapG/S=272/272 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, int existingBlockOffset, DWORD sizeOfExistingBlock, DWORD dataLength) [param_bytes=12]
callers=1 callees=0091dba0=_memcpy, 009196fd=operator delete, 00919723=operator new
```
int __thiscall FUN_00718470(int param_1,int param_2,size_t param_3,int param_4)
{
  void *_Dst;
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
      _Dst = (void *)FUN_00919723(*(undefined4 *)(param_1 + 0x68));
      _memcpy(_Dst,*(void **)(param_1 + 100),*(size_t *)(*(int *)(param_1 + 0x40) + 0x2c));
      FUN_009196fd(*(undefined4 *)(param_1 + 100));
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

### CResGFF::AddDataLayoutList  gog=0x6246e0 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00718580(mapper) delta=0xf3ea0 align=6/6 size=263 retn=12 gapG/S=272/272 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, int existingBlockOffset, DWORD sizeOfExistingBlock, DWORD dataLength) [param_bytes=12]
callers=2 callees=0091dba0=_memcpy, 009196fd=operator delete, 00919723=operator new
```
int __thiscall FUN_00718580(int param_1,int param_2,size_t param_3,int param_4)
{
  void *_Dst;
  if ((*(int *)(param_1 + 0x40) == 0) || (*(int *)(param_1 + 0x70) == 0)) {
    local_c = 0;
  }
  else {
    local_c = *(int *)(*(int *)(param_1 + 0x40) + 0x34);
    if (local_c == param_2 + param_3) {
      local_c = param_2;
    }
    if (*(uint *)(param_1 + 0x74) <= (uint)(local_c + param_4)) {
      *(int *)(param_1 + 0x74) = local_c + param_4 + *(int *)(param_1 + 0x3c);
      _Dst = (void *)FUN_00919723(*(undefined4 *)(param_1 + 0x74));
      _memcpy(_Dst,*(void **)(param_1 + 0x70),*(size_t *)(*(int *)(param_1 + 0x40) + 0x34));
      FUN_009196fd(*(undefined4 *)(param_1 + 0x70));
      *(void **)(param_1 + 0x70) = _Dst;
    }
    if (local_c != param_2) {
      _memcpy((void *)(*(int *)(param_1 + 0x70) + local_c),
              (void *)(*(int *)(param_1 + 0x70) + param_2),param_3);
      *(size_t *)(param_1 + 0x78) = *(int *)(param_1 + 0x78) + param_3;
    }
    *(int *)(*(int *)(param_1 + 0x40) + 0x34) = local_c + param_4;
    local_c = *(int *)(param_1 + 0x70) + local_c;
  }
  return local_c;
}
```

### CResGFF::GetElementType  gog=0x6247f0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00718690(mapper) delta=0xf3ea0 align=6/6 size=46 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr) [param_bytes=4]
callers=14 callees=00717720=gog:CResGFF::GetStruct
```
undefined4 FUN_00718690(undefined4 param_1)
{
  puVar1 = (undefined4 *)FUN_00717720(param_1);
  if (puVar1 == (undefined4 *)0x0) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = *puVar1;
  }
  return uVar2;
}
```

### CResGFF::GetFieldCount  gog=0x624820 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007186c0(mapper) delta=0xf3ea0 align=6/6 size=30 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: DWORD (__thiscall) (void* thisPtr, CResStruct* structPtr) [param_bytes=4]
callers=1 callees=
```
undefined4 FUN_007186c0(int param_1)
{
  if (param_1 == 0) {
    uVar1 = 0xffffffff;
  }
  else {
    uVar1 = *(undefined4 *)(param_1 + 8);
  }
  return uVar1;
}
```

### CResGFF::GetFieldType  gog=0x624870 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00718710(mapper) delta=0xf3ea0 align=6/6 size=75 retn=12 gapG/S=80/80 cc_guess=thiscall
wrapper: GFFFieldTypes (__thiscall) (void* thisPtr, CResStruct* structPtr, char* label, DWORD fieldIndex) [param_bytes=12]
callers=1 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel
```
undefined4 FUN_00718710(undefined4 param_1,int param_2,undefined4 param_3)
{
  if (param_2 != 0) {
    param_3 = FUN_007178e0(param_1,param_2);
  }
  puVar1 = (undefined4 *)FUN_00717770(param_1,param_3);
  if (puVar1 == (undefined4 *)0x0) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = *puVar1;
  }
  return uVar2;
}
```

### CResGFF::GetList  gog=0x6248c0 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00718760(mapper) delta=0xf3ea0 align=6/6 size=164 retn=12 gapG/S=176/176 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResList* outList, CResStruct* structPtr, char* label) [param_bytes=12]
callers=56 callees=0091d0f0, 00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel, 00717880=gog:CResGFF::GetDataLayoutList
```
undefined4 FUN_00718760(undefined4 *param_1,undefined4 *param_2,char *param_3)
{
  if (((param_1 == (undefined4 *)0x0) || (param_2 == (undefined4 *)0x0)) || (param_3 == (char *)0x0)
     ) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_2,param_3);
    local_8 = (int *)FUN_00717770(param_2,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 0xf)) {
      uVar1 = 0;
    }
    else {
      local_c = FUN_00717880(local_8,&local_10);
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

### CResGFF::GetListCount  gog=0x624970 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00718810(mapper) delta=0xf3ea0 align=6/6 size=124 retn=4 gapG/S=128/128 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResList* list) [param_bytes=4]
callers=56 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel, 00717880=gog:CResGFF::GetDataLayoutList
```
undefined4 FUN_00718810(int param_1)
{
  uint local_10 [2];
  if (param_1 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_1 + 4);
    local_8 = (int *)FUN_00717770(param_1,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 0xf)) {
      uVar1 = 0;
    }
    else {
      puVar2 = (undefined4 *)FUN_00717880(local_8,local_10);
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

### CResGFF::GetListElement  gog=0x6249f0 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00718890(mapper) delta=0xf3ea0 align=5/6 size=197 retn=12 gapG/S=208/208 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, CResList* list, DWORD index) [param_bytes=12]
callers=60 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel, 00717880=gog:CResGFF::GetDataLayoutList
```
undefined4 __thiscall FUN_00718890(int param_1,uint *param_2,int param_3,uint param_4)
{
  uint local_18 [3];
  if ((param_2 == (uint *)0x0) || (param_3 == 0)) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_3,param_3 + 4);
    local_c = (int *)FUN_00717770(param_3,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 0xf)) {
      uVar1 = 0;
    }
    else {
      puVar2 = (uint *)FUN_00717880(local_c,local_18);
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

### CResGFF::GetStructFromStruct  gog=0x624ac0 (param_bytes=12, notes='CResStruct * param_1, CResStruct * param_2, char * param_3')
steam: cand=0x00718960(mapper) delta=0xf3ea0 align=4/6 size=109 retn=12 gapG/S=112/112 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* outStruct, CResStruct* inStruct, char* label) [param_bytes=12]
callers=28 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel
```
undefined4 FUN_00718960(int *param_1,int param_2,int param_3)
{
  if (((param_1 == (int *)0x0) || (param_2 == 0)) || (param_3 == 0)) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_2,param_3);
    piVar2 = (int *)FUN_00717770(param_2,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 0xe)) {
      uVar1 = 0;
    }
    else {
      *param_1 = piVar2[2];
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### CResGFF::GetTopLevelStruct  gog=0x624b30 (param_bytes=4, notes='CResGFF * this, undefined4 param_1')
steam: cand=0x007189d0(mapper) delta=0xf3ea0 align=3/6 size=37 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* outStruct) [param_bytes=4]
callers=31 callees=
```
bool FUN_007189d0(undefined4 *param_1)
{
  if (param_1 != (undefined4 *)0x0) {
    *param_1 = 0;
  }
  return param_1 != (undefined4 *)0x0;
}
```

### CResGFF::ReadFieldBYTE  gog=0x624b60 (param_bytes=12, notes='CResGFF * this, int param_1, char * param_2, undefined4 * param_3, und')
steam: cand=0x00718a00(bracket) delta=0xf3ea0 align=/ size=122 retn=16 gapG/S=128/ cc_guess=
wrapper: BYTE (__thiscall) (void* thisPtr, CResStruct* structPtr, char* label, int* success, BYTE defaultValue) [param_bytes=16]
callers=50 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel
```
undefined1 FUN_00718a00(int param_1,int param_2,undefined4 *param_3,undefined1 param_4)
{
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
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

### CResGFF::ReadFieldCHAR  gog=0x624be0 (param_bytes=13, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined1')
steam: cand=0x00718a80(bracket) delta=0xf3ea0 align=/ size=122 retn=16 gapG/S=128/ cc_guess=
wrapper: char (__thiscall) (void* thisPtr, CResStruct* structPtr, char* label, int* success, char defaultValue) [param_bytes=16]
callers=5 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel
```
undefined1 FUN_00718a80(int param_1,int param_2,undefined4 *param_3,undefined1 param_4)
{
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 1)) {
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

### CResGFF::ReadFieldWORD  gog=0x624c60 (param_bytes=14, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined2')
steam: cand=0x00718b00(bracket) delta=0xf3ea0 align=/ size=125 retn=16 gapG/S=128/ cc_guess=
wrapper: WORD (__thiscall) (void* thisPtr, CResStruct* structPtr, char* label, int* success, WORD defaultValue) [param_bytes=16]
callers=24 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel
```
undefined2 FUN_00718b00(int param_1,int param_2,undefined4 *param_3,undefined2 param_4)
{
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
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

### CResGFF::ReadFieldSHORT  gog=0x624ce0 (param_bytes=14, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined2')
steam: cand=0x00718b80(bracket) delta=0xf3ea0 align=/ size=125 retn=16 gapG/S=128/ cc_guess=
wrapper: short (__thiscall) (void* thisPtr, CResStruct* structPtr, char* label, int* success, short defaultValue) [param_bytes=16]
callers=6 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel
```
undefined2 FUN_00718b80(int param_1,int param_2,undefined4 *param_3,undefined2 param_4)
{
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
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

### CResGFF::ReadFieldDWORD  gog=0x624d60 (param_bytes=16, notes='CResGFF * this, int param_1, char * param_2, undefined4 * param_3, int')
steam: cand=0x00718c00(mapper) delta=0xf3ea0 align=3/6 size=122 retn=16 gapG/S=128/128 cc_guess=thiscall
wrapper: DWORD (__thiscall) (void* thisPtr, CResStruct* structPtr, char* label, int* success, DWORD defaultValue) [param_bytes=16]
callers=67 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel
```
int FUN_00718c00(int param_1,int param_2,undefined4 *param_3,int param_4)
{
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 4)) {
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

### CResGFF::ReadFieldINT  gog=0x624de0 (param_bytes=16, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x00718c80(mapper) delta=0xf3ea0 align=4/6 size=122 retn=16 gapG/S=128/128 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, char* label, int* success, int defaultValue) [param_bytes=16]
callers=55 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel
```
int FUN_00718c80(int param_1,int param_2,undefined4 *param_3,int param_4)
{
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
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

### CResGFF::ReadFieldFLOAT  gog=0x624e60 (param_bytes=16, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x00718d00(mapper) delta=0xf3ea0 align=5/6 size=128 retn=16 gapG/S=128/128 cc_guess=thiscall
wrapper: float10 (__thiscall) (void* thisPtr, CResStruct* structPtr, char* label, int* success, float defaultValue) [param_bytes=16]
callers=49 callees=00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel
```
float10 FUN_00718d00(int param_1,int param_2,undefined4 *param_3,float param_4)
{
  float10 fVar3;
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
    fVar3 = (float10)param_4;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 8)) {
      *param_3 = 0;
      fVar3 = (float10)param_4;
    }
    else {
      *param_3 = 1;
      fVar3 = (float10)(float)piVar2[2];
    }
  }
  return fVar3;
}
```

### CResGFF::ReadFieldDWORD64  gog=0x624ee0 (param_bytes=20, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x00718d80(mapper) delta=0xf3ea0 align=6/6 size=181 retn=20 gapG/S=192/192 cc_guess=thiscall
wrapper: DWORD64 (__thiscall) (void* thisPtr, CResStruct* structPtr, char* label, int* success, DWORD64 defaultValue) [param_bytes=16]
callers=3 callees=00717770=gog:CResGFF::GetField, 00717820=gog:CResGFF::GetDataField, 007178e0=gog:CResGFF::GetFieldByLabel
```
undefined8
FUN_00718d80(int param_1,int param_2,undefined4 *param_3,undefined4 param_4,undefined4 param_5)
{
  uint local_10 [2];
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
    local_8 = (int *)FUN_00717770(param_1,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 6)) {
      *param_3 = 0;
    }
    else {
      puVar2 = (undefined4 *)FUN_00717820(local_8,local_10);
      if ((puVar2 == (undefined4 *)0x0) || (local_10[0] < 8)) {
        *param_3 = 0;
      }
      else {
        *param_3 = 1;
        param_4 = *puVar2;
        param_5 = puVar2[1];
      }
    }
  }
  return CONCAT44(param_5,param_4);
}
```

### CResGFF::ReadFieldCResRef  gog=0x624fa0 (param_bytes=20, notes='CResGFF * this, undefined4 * param_1, undefined4 param_2, char * param')
steam: cand=0x00718e40(mapper) delta=0xf3ea0 align=6/6 size=336 retn=20 gapG/S=336/336 cc_guess=thiscall
wrapper: CResRef* (__thiscall) (void* thisPtr, CResRef* out, CResStruct* structPtr, char* label, int* success, CResRef* defaultValue) [param_bytes=20]
callers=37 callees=007108a0, 00717770=gog:CResGFF::GetField, 00717820=gog:CResGFF::GetDataField, 007178e0=gog:CResGFF::GetFieldByLabel
```
undefined4 *
FUN_00718e40(undefined4 *param_1,int param_2,int param_3,undefined4 *param_4,undefined4 *param_5)
{
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    *param_1 = *param_5;
    param_1[1] = param_5[1];
    param_1[2] = param_5[2];
    param_1[3] = param_5[3];
  }
  else {
    uVar1 = FUN_007178e0(param_2,param_3);
    local_c = (int *)FUN_00717770(param_2,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 0xb)) {
      *param_4 = 0;
      *param_1 = *param_5;
      param_1[1] = param_5[1];
      param_1[2] = param_5[2];
      param_1[3] = param_5[3];
    }
    else {
      local_10 = (byte *)FUN_00717820(local_c,&local_14);
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
... (2 more lines)
```

### CResGFF::ReadFieldCExoString  gog=0x6250f0 (param_bytes=20, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x00718f90(mapper) delta=0xf3ea0 align=6/6 size=322 retn=20 gapG/S=336/336 cc_guess=thiscall
wrapper: CExoString* (__thiscall) (void* thisPtr, CExoString* out, CResStruct* structPtr, char* label, int* success, CExoString* defaultValue) [param_bytes=20]
name_map: CResGFF::ReadFieldCExoString [medium] mapper vote GOG 0x6250f0 (retn=20), Steam ret=20, sz=322
callers=42 callees=007335f0=CExoString::CExoString_copy, 00717770=gog:CResGFF::GetField, 00717820=gog:CResGFF::GetDataField, 007178e0=gog:CResGFF::GetFieldByLabel, 00733680=CExoString::Constructor_4
```
undefined4
FUN_00718f90(undefined4 param_1,int param_2,int param_3,undefined4 *param_4,undefined4 param_5)
{
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    FUN_007335f0(param_5);
  }
  else {
    uVar1 = FUN_007178e0(param_2,param_3);
    local_c = (int *)FUN_00717770(param_2,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 10)) {
      *param_4 = 0;
      FUN_007335f0(param_5);
    }
    else {
      local_10 = (int *)FUN_00717820(local_c,&local_14);
      if ((local_10 == (int *)0x0) || (local_14 < 4)) {
        *param_4 = 0;
        FUN_007335f0(param_5);
      }
      else {
        local_8 = *local_10;
        if (local_14 < local_8 + 4U) {
          *param_4 = 0;
          FUN_007335f0(param_5);
        }
        else {
          *param_4 = 1;
          FUN_00733680(local_10 + 1,local_8);
        }
      }
    }
  }
  return param_1;
}
```

### CResGFF::ReadFieldCExoLocString  gog=0x625240 (param_bytes=20, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x007190e0(mapper) delta=0xf3ea0 align=6/6 size=741 retn=20 gapG/S=752/752 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, void* out, CResStruct* structPtr, char* label, int* success, void* defaultValue) [param_bytes=20]
callers=16 callees=007352d0, 00735360=CExoLocString::Destructor, 00735240, 00717770=gog:CResGFF::GetField, 00717820=gog:CResGFF::GetDataField, 007178e0=gog:CResGFF::GetFieldByLabel, 00735440, 00733780=CExoString::~CExoString, 00733680=CExoString::Constructor_4
```
undefined4
FUN_007190e0(undefined4 param_1,int param_2,int param_3,undefined4 *param_4,undefined4 param_5)
{
  undefined1 local_2c [4];
  void *local_10;
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
    FUN_00735360();
  }
  else {
    uVar3 = FUN_007178e0(param_2,param_3);
    local_18 = (int *)FUN_00717770(param_2,uVar3);
    if ((local_18 == (int *)0x0) || (*local_18 != 0xc)) {
      *param_4 = 0;
      FUN_007352d0(param_5);
      local_8 = local_8 & 0xffffff00;
      FUN_00735360();
    }
    else {
      local_24 = (uint *)FUN_00717820(local_18,&local_30);
      if ((local_24 == (uint *)0x0) || (local_30 < 4)) {
        *param_4 = 0;
        FUN_007352d0(param_5);
        local_8 = local_8 & 0xffffff00;
        FUN_00735360();
      }
      else {
        local_14 = *local_24;
        if (local_30 < local_14 + 4U) {
          *param_4 = 0;
          FUN_007352d0(param_5);
          local_8 = local_8 & 0xffffff00;
          FUN_00735360();
        }
        else {
          local_28 = local_24[1];
          uVar1 = local_24[2];
... (38 more lines)
```

### CResGFF::ReadFieldQuaternion  gog=0x625530 (param_bytes=20, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x007193d0(mapper) delta=0xf3ea0 align=6/6 size=326 retn=20 gapG/S=336/336 cc_guess=thiscall
wrapper: Quaternion* (__thiscall) (void* thisPtr, Quaternion* out, CResStruct* structPtr, char* label, int* success, Quaternion* defaultValue) [param_bytes=20]
callers=1 callees=00717770=gog:CResGFF::GetField, 00717820=gog:CResGFF::GetDataField, 007178e0=gog:CResGFF::GetFieldByLabel
```
undefined4 *
FUN_007193d0(undefined4 *param_1,int param_2,int param_3,undefined4 *param_4,undefined4 *param_5)
{
  uint local_20 [2];
  local_c = 0;
  local_10 = 0;
  local_14 = 0;
  local_18 = 0x3f800000;
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    *param_1 = *param_5;
    param_1[1] = param_5[1];
    param_1[2] = param_5[2];
    param_1[3] = param_5[3];
  }
  else {
    uVar3 = FUN_007178e0(param_2,param_3);
    local_8 = (int *)FUN_00717770(param_2,uVar3);
    if ((local_8 == (int *)0x0) || (*local_8 != 0x10)) {
      *param_4 = 0;
      *param_1 = *param_5;
      param_1[1] = param_5[1];
      param_1[2] = param_5[2];
      param_1[3] = param_5[3];
    }
    else {
      puVar4 = (undefined4 *)FUN_00717820(local_8,local_20);
      if ((puVar4 == (undefined4 *)0x0) || (local_20[0] < 0x10)) {
        *param_4 = 0;
        *param_1 = *param_5;
        param_1[1] = param_5[1];
        param_1[2] = param_5[2];
        param_1[3] = param_5[3];
      }
      else {
        *param_4 = 1;
        uVar3 = puVar4[1];
        uVar1 = puVar4[2];
        uVar2 = puVar4[3];
        *param_1 = *puVar4;
        param_1[1] = uVar3;
        param_1[2] = uVar1;
        param_1[3] = uVar2;
      }
    }
... (3 more lines)
```

### CResGFF::ReadFieldVector  gog=0x625680 (param_bytes=20, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x00719520(mapper) delta=0xf3ea0 align=6/6 size=288 retn=20 gapG/S=288/288 cc_guess=thiscall
wrapper: Vector* (__thiscall) (void* thisPtr, Vector* out, CResStruct* structPtr, char* label, int* success, Vector* defaultValue) [param_bytes=20]
callers=8 callees=00717770=gog:CResGFF::GetField, 00717820=gog:CResGFF::GetDataField, 007178e0=gog:CResGFF::GetFieldByLabel
```
undefined4 *
FUN_00719520(undefined4 *param_1,int param_2,int param_3,undefined4 *param_4,undefined4 *param_5)
{
  uint local_10 [2];
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    *param_1 = *param_5;
    param_1[1] = param_5[1];
    param_1[2] = param_5[2];
  }
  else {
    uVar2 = FUN_007178e0(param_2,param_3);
    local_8 = (int *)FUN_00717770(param_2,uVar2);
    if ((local_8 == (int *)0x0) || (*local_8 != 0x11)) {
      *param_4 = 0;
      *param_1 = *param_5;
      param_1[1] = param_5[1];
      param_1[2] = param_5[2];
    }
    else {
      puVar3 = (undefined4 *)FUN_00717820(local_8,local_10);
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

### CResGFF::ReadFieldVOID  gog=0x6257a0 (param_bytes=24, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x00719640(mapper) delta=0xf3ea0 align=6/6 size=254 retn=24 gapG/S=256/256 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, CResStruct* structPtr, void* buffer, DWORD size, char* label, int* success, void* defaultValue) [param_bytes=24]
callers=6 callees=0091dba0=_memcpy, 00717770=gog:CResGFF::GetField, 00717820=gog:CResGFF::GetDataField, 007178e0=gog:CResGFF::GetFieldByLabel
```
void * FUN_00719640(int param_1,void *param_2,uint param_3,int param_4,undefined4 *param_5,
                   void *param_6)
{
  if (((param_1 == 0) || (param_4 == 0)) || (param_2 == (void *)0x0)) {
    *param_5 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_4);
    local_c = (int *)FUN_00717770(param_1,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 0xd)) {
      *param_5 = 0;
    }
    else {
      local_10 = (uint *)FUN_00717820(local_c,&local_14);
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

### CResGFF::AddList  gog=0x6258a0 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00719740(mapper) delta=0xf3ea0 align=6/6 size=183 retn=12 gapG/S=192/192 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResList* list, CResStruct* structPtr, char* labelText) [param_bytes=12]
callers=43 callees=00717aa0=gog:CResGFF::InitializeForWriting, 0091d0f0, 00718030=gog:CResGFF::AddField, 00718580=gog:CResGFF::AddDataLayoutList
```
undefined4 __thiscall
FUN_00719740(int param_1,undefined4 *param_2,undefined4 *param_3,char *param_4)
{
  if (((param_2 == (undefined4 *)0x0) || (param_3 == (undefined4 *)0x0)) || (param_4 == (char *)0x0)
     ) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_3,param_4,0xf);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      puVar3 = (undefined4 *)FUN_00718580(*(undefined4 *)(iVar2 + 8),0,4);
      if (puVar3 == (undefined4 *)0x0) {
        uVar1 = 0;
      }
      else {
        *(int *)(iVar2 + 8) = (int)puVar3 - *(int *)(param_1 + 0x70);
        *puVar3 = 0;
        *param_2 = *param_3;
        _strncpy((char *)(param_2 + 1),param_4,0x10);
        uVar1 = 1;
      }
    }
  }
  return uVar1;
}
```

### CResGFF::AddListElement  gog=0x625960 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00719800(mapper) delta=0xf3ea0 align=6/6 size=306 retn=12 gapG/S=320/320 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, CResList* list, DWORD structId) [param_bytes=12]
callers=44 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00717f30=gog:CResGFF::AddStruct, 00717770=gog:CResGFF::GetField, 007178e0=gog:CResGFF::GetFieldByLabel, 00718580=gog:CResGFF::AddDataLayoutList, 00717880=gog:CResGFF::GetDataLayoutList
```
undefined4 __thiscall FUN_00719800(int param_1,int *param_2,int param_3,undefined4 param_4)
{
  if ((param_3 == 0) || (param_2 == (int *)0x0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    uVar1 = FUN_007178e0(param_3,param_3 + 4);
    local_8 = (int *)FUN_00717770(param_3,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 0xf)) {
      uVar1 = 0;
    }
    else {
      local_14 = (int *)FUN_00717880(local_8,&local_18);
      if ((local_14 == (int *)0x0) || (local_18 < 4)) {
        uVar1 = 0;
      }
      else {
        local_10 = *local_14;
        if (local_18 < local_10 * 4 + 4U) {
          uVar1 = 0;
        }
        else {
          local_c = (int *)FUN_00718580(local_8[2],local_10 * 4 + 4,local_10 * 4 + 8);
          if (local_c == (int *)0x0) {
            uVar1 = 0;
          }
          else {
            local_8[2] = (int)local_c - *(int *)(param_1 + 0x70);
            iVar2 = FUN_00717f30(param_4);
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
... (3 more lines)
```

### CResGFF::AddStructToStruct  gog=0x625aa0 (param_bytes=16, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x00719940(mapper) delta=0xf3ea0 align=6/6 size=135 retn=16 gapG/S=144/144 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* parentStruct, CResStruct* childStruct, char* labelText, DWORD structId) [param_bytes=16]
callers=14 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00717f30=gog:CResGFF::AddStruct, 00718030=gog:CResGFF::AddField
```
undefined4 __thiscall
FUN_00719940(int param_1,int *param_2,int param_3,int param_4,undefined4 param_5)
{
  if (((param_2 == (int *)0x0) || (param_3 == 0)) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_3,param_4,0xe);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      iVar3 = FUN_00717f30(param_5);
      if (iVar3 == -1) {
        uVar1 = 0;
      }
      else {
        *(int *)(iVar2 + 8) = iVar3;
        *param_2 = iVar3;
        uVar1 = 1;
      }
    }
  }
  return uVar1;
}
```

### CResGFF::WriteFieldBYTE  gog=0x625b30 (param_bytes=12, notes='undefined4 param_1, undefined1 param_2, undefined4 param_3')
steam: cand=0x007199d0(mapper) delta=0xf3ea0 align=6/6 size=97 retn=12 gapG/S=112/112 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, BYTE value, char* label) [param_bytes=12]
callers=35 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00718030=gog:CResGFF::AddField
```
undefined4 __thiscall FUN_007199d0(int param_1,int param_2,byte param_3,int param_4)
{
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,0);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(uint *)(iVar2 + 8) = (uint)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### CResGFF::WriteFieldCHAR  gog=0x625ba0 (param_bytes=12, notes='undefined4 param_1, undefined1 param_2, undefined4 param_3')
steam: cand=0x00719a40(mapper) delta=0xf3ea0 align=6/6 size=97 retn=12 gapG/S=112/112 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, char value, char* label) [param_bytes=12]
callers=6 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00718030=gog:CResGFF::AddField
```
undefined4 __thiscall FUN_00719a40(int param_1,int param_2,char param_3,int param_4)
{
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,1);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(int *)(iVar2 + 8) = (int)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### CResGFF::WriteFieldWORD  gog=0x625c10 (param_bytes=12, notes='undefined4 param_1, undefined2 param_2, undefined4 param_3')
steam: cand=0x00719ab0(mapper) delta=0xf3ea0 align=6/6 size=97 retn=12 gapG/S=112/112 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, WORD value, char* label) [param_bytes=12]
callers=19 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00718030=gog:CResGFF::AddField
```
undefined4 __thiscall FUN_00719ab0(int param_1,int param_2,ushort param_3,int param_4)
{
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,2);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(uint *)(iVar2 + 8) = (uint)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### CResGFF::WriteFieldSHORT  gog=0x625c80 (param_bytes=12, notes='undefined4 param_1, undefined2 param_2, undefined4 param_3')
steam: cand=0x00719b20(mapper) delta=0xf3ea0 align=6/6 size=97 retn=12 gapG/S=112/112 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, short value, char* label) [param_bytes=12]
callers=6 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00718030=gog:CResGFF::AddField
```
undefined4 __thiscall FUN_00719b20(int param_1,int param_2,short param_3,int param_4)
{
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,3);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(int *)(iVar2 + 8) = (int)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### CResGFF::WriteFieldDWORD  gog=0x625cf0 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00719b90(mapper) delta=0xf3ea0 align=6/6 size=96 retn=12 gapG/S=96/96 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, DWORD value, char* label) [param_bytes=12]
callers=52 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00718030=gog:CResGFF::AddField
```
undefined4 __thiscall FUN_00719b90(int param_1,int param_2,undefined4 param_3,int param_4)
{
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,4);
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

### CResGFF::WriteFieldINT  gog=0x625d50 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00719bf0(mapper) delta=0xf3ea0 align=6/6 size=96 retn=12 gapG/S=96/96 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, int value, char* label) [param_bytes=12]
callers=32 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00718030=gog:CResGFF::AddField
```
undefined4 __thiscall FUN_00719bf0(int param_1,int param_2,undefined4 param_3,int param_4)
{
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,5);
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

### CResGFF::WriteFieldDWORD64  gog=0x625db0 (param_bytes=16, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x00719c50(mapper) delta=0xf3ea0 align=6/6 size=143 retn=16 gapG/S=144/144 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, DWORD64 value, char* label) [param_bytes=12]
callers=4 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00718030=gog:CResGFF::AddField, 00718370=gog:CResGFF::AddDataField
```
undefined4 __thiscall
FUN_00719c50(int param_1,int param_2,undefined4 param_3,undefined4 param_4,int param_5)
{
  if ((param_2 == 0) || (param_5 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_5,6);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      puVar3 = (undefined4 *)FUN_00718370(0xffffffff,0,8);
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

### CResGFF::WriteFieldFLOAT  gog=0x625e40 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00719ce0(mapper) delta=0xf3ea0 align=6/6 size=96 retn=12 gapG/S=96/96 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, float value, char* label) [param_bytes=12]
callers=26 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00718030=gog:CResGFF::AddField
```
undefined4 __thiscall FUN_00719ce0(int param_1,int param_2,undefined4 param_3,int param_4)
{
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,8);
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

### CResGFF::WriteFieldCResRef  gog=0x625ea0 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00719d40(mapper) delta=0xf3ea0 align=6/6 size=269 retn=12 gapG/S=272/272 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, CResRef* value, char* label) [param_bytes=12]
callers=12 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00710a30, 00718030=gog:CResGFF::AddField, 00718370=gog:CResGFF::AddDataField, 00458940=identity (basic vtable slot 0x4)
```
undefined4 __thiscall FUN_00719d40(int param_1,int param_2,undefined4 param_3,int param_4)
{
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar3 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar4 = FUN_00718030(param_2,param_4,0xb);
    if (iVar4 == 0) {
      uVar3 = 0;
    }
    else {
      bVar2 = FUN_00710a30();
      pbVar5 = (byte *)FUN_00718370(0xffffffff,0,bVar2 + 1);
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

### CResGFF::WriteFieldCExoString  gog=0x625fb0 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00719e50(mapper) delta=0xf3ea0 align=6/6 size=222 retn=12 gapG/S=224/224 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, CExoString* value, char* label) [param_bytes=12]
callers=32 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00734010=CExoString::CStr, 0091dba0=_memcpy, 00919f10=_strlen, 00718030=gog:CResGFF::AddField, 00718370=gog:CResGFF::AddDataField
```
undefined4 __thiscall FUN_00719e50(int param_1,int param_2,int *param_3,int param_4)
{
  size_t *psVar3;
  void *_Src;
  size_t local_14;
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,10);
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
      psVar3 = (size_t *)FUN_00718370(0xffffffff,0,local_14 + 4);
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

### CResGFF::WriteFieldCExoLocString  gog=0x626090 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00719f30(mapper) delta=0xf3ea0 align=6/6 size=633 retn=12 gapG/S=640/640 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, void* value, char* label) [param_bytes=12]
callers=11 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00734010=CExoString::CStr, 0091dba0=_memcpy, 00919f10=_strlen, 00718030=gog:CResGFF::AddField, 00735840, 00733780=CExoString::~CExoString, 0058c390, 00718370=gog:CResGFF::AddDataField, 00735550, 00733540=CExoString::CExoString(empty)
```
undefined4 __thiscall FUN_00719f30(int param_1,int param_2,int param_3,int param_4)
{
  void *_Src;
  size_t _Size;
  size_t local_50;
  char *local_38 [2];
  size_t local_2c;
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ffb8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_20 = 1;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if ((param_2 == 0) || (param_4 == 0)) {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar3 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    local_18 = FUN_00718030(param_2,param_4,0xc);
    if (local_18 == 0) {
      local_8 = 0xffffffff;
      FUN_00733780();
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
      local_28 = (int *)FUN_00718370(0xffffffff,0,local_14 + 4);
      if (local_28 == (int *)0x0) {
        local_8 = 0xffffffff;
        FUN_00733780();
... (48 more lines)
```

### CResGFF::WriteFieldVOID  gog=0x626310 (param_bytes=16, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x0071a1b0(mapper) delta=0xf3ea0 align=5/6 size=174 retn=16 gapG/S=176/176 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, void* data, DWORD size, char* label) [param_bytes=16]
callers=5 callees=00717aa0=gog:CResGFF::InitializeForWriting, 0091dba0=_memcpy, 00718030=gog:CResGFF::AddField, 00718370=gog:CResGFF::AddDataField
```
undefined4 __thiscall FUN_0071a1b0(int param_1,int param_2,void *param_3,size_t param_4,int param_5)
{
  size_t *psVar3;
  if ((param_2 == 0) || (param_5 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_5,0xd);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      psVar3 = (size_t *)FUN_00718370(0xffffffff,0,param_4 + 4);
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

### CResGFF::WriteFieldQuaternion  gog=0x6263c0 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x0071a260(mapper) delta=0xf3ea0 align=4/6 size=184 retn=12 gapG/S=192/192 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, Quaternion* value, char* label) [param_bytes=12]
callers=1 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00718030=gog:CResGFF::AddField, 00718370=gog:CResGFF::AddDataField
```
undefined4 __thiscall FUN_0071a260(int param_1,int param_2,undefined4 *param_3,int param_4)
{
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar4 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar5 = FUN_00718030(param_2,param_4,0x10);
    if (iVar5 == 0) {
      uVar4 = 0;
    }
    else {
      puVar6 = (undefined4 *)FUN_00718370(0xffffffff,0,0x10);
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

### CResGFF::WriteFieldVector  gog=0x626480 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x0071a320(mapper) delta=0xf3ea0 align=4/6 size=172 retn=12 gapG/S=176/176 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, Vector* value, char* label) [param_bytes=12]
callers=2 callees=00717aa0=gog:CResGFF::InitializeForWriting, 00718030=gog:CResGFF::AddField, 00718370=gog:CResGFF::AddDataField
```
undefined4 __thiscall FUN_0071a320(int param_1,int param_2,undefined4 *param_3,int param_4)
{
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar3 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar4 = FUN_00718030(param_2,param_4,0x11);
    if (iVar4 == 0) {
      uVar3 = 0;
    }
    else {
      puVar5 = (undefined4 *)FUN_00718370(0xffffffff,0,0xc);
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

### CResGFF::CreateGFFFile  gog=0x626530 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x0071a5a0(mapper) delta=0xf4070 align=0/6 size=198 retn=8 gapG/S=464/208 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, CResStruct* structPtr, CExoString* fileType, CExoString* version) [param_bytes=12]
callers=19 callees=00733570=CExoString::CExoString_char, 0071a670=CResGFF::WriteGFFData, 0071a8b0=CResGFF::Pack, 00733780=CExoString::~CExoString, 00739880, 007398e0, 007397f0
```
undefined4 FUN_0071a5a0(undefined4 param_1,undefined2 param_2)
{
  undefined1 local_20 [8];
  undefined1 local_18 [4];
  undefined1 local_14 [4];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095fff0;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570(&DAT_0098ba1c);
  local_8 = 0;
  uVar3 = extraout_var;
  FUN_007397f0(param_1,param_2,local_20);
  local_8 = CONCAT31(local_8._1_3_,2);
  FUN_00733780(uVar1,uVar3);
  iVar2 = FUN_007398e0();
  if (iVar2 == 0) {
    local_8 = 0xffffffff;
    FUN_00739880();
    uVar3 = 0;
  }
  else {
    FUN_0071a8b0(0,0);
    uVar3 = FUN_0071a670(local_14,local_18);
    local_8 = 0xffffffff;
    FUN_00739880();
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### CResGFF::WriteGFFFile  gog=0x626700 (param_bytes=6, notes='undefined4 param_1, undefined2 param_2')
NO CANDIDATE (bracketing failed) -- see manual list

### CResGFF::WriteGFFData  gog=0x6267d0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x0071a670(mapper) delta=0xf3ea0 align=3/6 size=562 retn=8 gapG/S=576/576 cc_guess=thiscall
wrapper: bool (__thiscall) (void* thisPtr, CExoFile* file, DWORD* totalBytes) [param_bytes=8]
name_map: CResGFF::WriteGFFData [medium] mapper vote GOG 0x6267d0 (retn=8), Steam ret=8, sz=562
callers=2 callees=00739a30
```
undefined4 __thiscall FUN_0071a670(int param_1,undefined4 param_2,int *param_3)
{
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
... (26 more lines)
```

### CResGFF::Pack  gog=0x626a10 (param_bytes=8, notes='undefined1 param_1, undefined4 param_2')
steam: cand=0x0071a8b0(mapper) delta=0xf3ea0 align=2/6 size=1010 retn=8 gapG/S=1024/1024 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, byte alwaysZero1, DWORD alwaysZero2) [param_bytes=8]
name_map: CResGFF::Pack [medium] mapper vote GOG 0x626a10 (retn=8), Steam ret=8, sz=1010
callers=2 callees=0091dba0=_memcpy, 009196fd=operator delete, 0071acb0=CResGFF::GetTotalSize, 00919723=operator new
```
void __thiscall FUN_0071a8b0(int param_1,byte param_2,uint param_3)
{
  if (((((*(int *)(param_1 + 0x6c) != 0) || (*(int *)(param_1 + 0x78) != 0)) &&
       (*(int *)(param_1 + 0x40) != 0)) &&
      (((*(int *)(param_1 + 100) != 0 && (*(int *)(param_1 + 0x70) != 0)) &&
       ((*(int *)(param_1 + 0x44) != 0 &&
        ((*(int *)(param_1 + 0x4c) != 0 &&
         (uVar1 = FUN_0071acb0(),
         param_3 < (uint)(*(int *)(param_1 + 0x6c) + *(int *)(param_1 + 0x78)))))))))) &&
     ((uint)param_2 < (uint)((*(int *)(param_1 + 0x6c) + *(int *)(param_1 + 0x78)) * 100) / uVar1))
  {
    if (*(int *)(param_1 + 0x6c) != 0) {
      if (*(int *)(*(int *)(param_1 + 0x40) + 0x2c) == *(int *)(param_1 + 0x6c)) {
        *(undefined4 *)(*(int *)(param_1 + 0x40) + 0x2c) = 0;
      }
      else {
        iVar2 = FUN_00919723(*(int *)(*(int *)(param_1 + 0x40) + 0x2c) - *(int *)(param_1 + 0x6c));
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
        FUN_009196fd(*(undefined4 *)(param_1 + 100));
        *(int *)(param_1 + 100) = iVar2;
        *(int *)(*(int *)(param_1 + 0x40) + 0x2c) = local_8;
      }
      *(undefined4 *)(param_1 + 0x6c) = 0;
    }
... (39 more lines)
```

### CResGFF::GetTotalSize  gog=0x626e10 (param_bytes=0, notes='void')
steam: cand=0x0071acb0(mapper) delta=0xf3ea0 align=2/6 size=135 retn=0 gapG/S=144/144 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CResGFF::GetTotalSize [medium] mapper vote GOG 0x626e10 (retn=0), Steam ret=0, sz=135
callers=2 callees=
```
int __fastcall FUN_0071acb0(int param_1)
{
  if (*(int *)(param_1 + 0x40) == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(*(int *)(param_1 + 0x40) + 0x1c) * 0x10 +
            *(int *)(*(int *)(param_1 + 0x40) + 0x14) * 0xc +
            *(int *)(*(int *)(param_1 + 0x40) + 0xc) * 0xc + 0x38 +
            *(int *)(*(int *)(param_1 + 0x40) + 0x24) + *(int *)(*(int *)(param_1 + 0x40) + 0x2c) +
            *(int *)(*(int *)(param_1 + 0x40) + 0x34);
  }
  return iVar1;
}
```

