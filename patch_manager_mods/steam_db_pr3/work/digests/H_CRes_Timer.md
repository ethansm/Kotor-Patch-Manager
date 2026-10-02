# Digest H_CRes_Timer

### CRes::Constructor  gog=0x61aa20 (param_bytes=0, notes='void')
steam: cand=0x007110d0(mapper) delta=0xf66b0 align=3/6 size=151 retn=0 gapG/S=160/160 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CRes::CRes [high] base ctor called by CResTPC/TGA/PLT ctors
callers=20 callees=
```
undefined4 * __fastcall FUN_007110d0(undefined4 *param_1)
{
  *param_1 = CRes::vftable;
  *(undefined2 *)(param_1 + 1) = 0;
  param_1[2] = 0xffffffff;
  *(undefined2 *)((int)param_1 + 6) = 0;
  param_1[6] = 0;
  param_1[4] = 0;
  param_1[3] = 0;
  param_1[5] = 0;
  param_1[7] = 0;
  param_1[8] = 0;
  param_1[9] = 0;
  param_1[3] = param_1[3] & 0xfffffffb;
  param_1[3] = param_1[3] & 0xfffffff7;
  return param_1;
}
```

### CRes::Destructor  gog=0x61aad0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00711170(mapper) delta=0xf66a0 align=3/6 size=44 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=009196fd=operator delete, 007111a0=gog:CRes::Destructor_2
```
undefined4 __thiscall FUN_00711170(undefined4 param_1,uint param_2)
{
  FUN_007111a0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### CRes::CancelRequest  gog=0x61ab20 (param_bytes=0, notes='void')
steam: cand=0x007111c0(mapper) delta=0xf66a0 align=5/6 size=26 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CRes::CancelRequest [high] vote GOG 0x61ab20 p0, exact
callers=37 callees=00711a30=CExoResMan::CancelRequest
```
void __fastcall FUN_007111c0(undefined4 param_1)
{
  FUN_00711a30(param_1);
  return;
}
```

### CRes::Demand  gog=0x61ab40 (param_bytes=0, notes='CRes * this')
steam: cand=0x007111e0(mapper) delta=0xf66a0 align=6/6 size=26 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CRes::Demand [high] asm 0x7111EB push; mov ecx,[0xA1B490]; call 0x711C20
callers=28 callees=00711c20=CExoResMan::Demand
```
void __fastcall FUN_007111e0(undefined4 param_1)
{
  FUN_00711c20(param_1);
  return;
}
```

### CRes::GetResRef  gog=0x61abd0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00711270(mapper) delta=0xf66a0 align=5/6 size=50 retn=8 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* outRef, WORD* outType) [param_bytes=8]
name_map: CRes::GetResRef (reads key entry via `CRes+0x14`) [medium] no NULL guard | `FUN_00710ca0(local_14,*(undefined4 *)(param_1 + 0x14)); *param_3 = *(undefined2 *)(*(int *)(param_1 + 0
callers=2 callees=00710ca0
```
void __thiscall FUN_00711270(int param_1,undefined4 param_2,undefined2 *param_3)
{
  undefined1 local_14 [16];
  FUN_00710ca0(local_14,*(undefined4 *)(param_1 + 0x14));
  *param_3 = *(undefined2 *)(*(int *)(param_1 + 0x14) + 0x1a);
  return;
}
```

### CRes::Release  gog=0x61ac10 (param_bytes=0, notes='void')
steam: cand=0x007112b0(mapper) delta=0xf66a0 align=5/6 size=26 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CRes::Release [high] ecx=[0xA1B490] call 0x7130B0
callers=31 callees=007130b0=CExoResMan::Release
```
void __fastcall FUN_007112b0(undefined4 param_1)
{
  FUN_007130b0(param_1);
  return;
}
```

### CRes::Request  gog=0x61ac30 (param_bytes=0, notes='void')
steam: cand=0x007112d0(mapper) delta=0xf66a0 align=5/6 size=26 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CRes::Request [medium] mapper vote GOG 0x61ac30 (retn=0), Steam ret=0, sz=26
callers=18 callees=00713310=CExoResMan::Request
```
void __fastcall FUN_007112d0(undefined4 param_1)
{
  FUN_00713310(param_1);
  return;
}
```

### CWorldTimer::SetMinutesPerHour  gog=0x80ace0 (param_bytes=1, notes='undefined1 param_1')
steam: cand=0x0051a800(bracket) delta=-0x2f04e0 align=/ size=81 retn=4 gapG/S=96/ cc_guess=
wrapper: void (__thiscall) (void* thisPtr, BYTE minutesPerHour) [param_bytes=4]
callers=3 callees=
```
void __thiscall FUN_0051a800(int param_1,char param_2)
{
  *(char *)(param_1 + 0x38) = param_2;
  if (param_2 == '\0') {
    *(undefined1 *)(param_1 + 0x38) = 5;
  }
  *(uint *)(param_1 + 0x40) = (uint)*(byte *)(param_1 + 0x38) * 0x5a0;
  *(uint *)(param_1 + 0x3c) = (uint)*(byte *)(param_1 + 0x38) * 0x15f900;
  return;
}
```

### CWorldTimer::ConvertFromTimeOfDay  gog=0x80b020 (param_bytes=20, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x0051ab40(mapper) delta=-0x2f04e0 align=6/6 size=129 retn=20 gapG/S=144/144 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, DWORD timeInMS, DWORD* outHours, DWORD* outMinutes, DWORD* outSeconds, DWORD* outMS) [param_bytes=20]
callers=8 callees=
```
void __thiscall
FUN_0051ab40(int param_1,uint param_2,uint *param_3,uint *param_4,uint *param_5,uint *param_6)
{
  *param_6 = param_2 % 1000;
  *param_5 = (param_2 / 1000) % 0x3c;
  uVar1 = (param_2 / 1000) / 0x3c;
  *param_4 = uVar1 % (uint)*(byte *)(param_1 + 0x38);
  *param_3 = uVar1 / *(byte *)(param_1 + 0x38);
  return;
}
```

### CWorldTimer::GetWorldTime  gog=0x80b200 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x0051ad20(mapper) delta=-0x2f04e0 align=6/6 size=268 retn=8 gapG/S=272/272 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, DWORD* outDays, DWORD* outMS) [param_bytes=8]
callers=94 callees=0051a920=gog:CWorldTimer::GetSnapshotTime, 0091d220, 0091fc70
```
void __thiscall FUN_0051ad20(int param_1,int *param_2,uint *param_3)
{
  uVar3 = FUN_0051a920();
  if (*(int *)(param_1 + 0x24) == 1) {
    *param_2 = *(int *)(param_1 + 0x28);
    *param_3 = *(uint *)(param_1 + 0x2c);
  }
  else {
    uVar3 = __aulldiv(uVar3,1000,0);
    iVar1 = __aullrem(uVar3,*(undefined4 *)(param_1 + 0x3c),0);
    iVar2 = __aulldiv(uVar3,*(undefined4 *)(param_1 + 0x3c),0);
    *param_2 = iVar2 + *(int *)(param_1 + 0x30);
    *param_3 = iVar1 + *(int *)(param_1 + 0x34);
  }
  while (-*(int *)(param_1 + 0x3c) - 1U < *param_3) {
    *param_2 = *param_2 + -1;
    *param_3 = *param_3 + *(int *)(param_1 + 0x3c);
  }
  while (*(uint *)(param_1 + 0x3c) <= *param_3) {
    *param_2 = *param_2 + 1;
    *param_3 = *param_3 - *(int *)(param_1 + 0x3c);
  }
  return;
}
```

### CWorldTimer::ResetTimer  gog=0x80b310 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0051ae30(mapper) delta=-0x2f04e0 align=6/6 size=177 retn=4 gapG/S=192/192 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* reset) [param_bytes=4]
callers=4 callees=
```
void __thiscall FUN_0051ae30(undefined4 *param_1,undefined4 *param_2)
{
  *param_1 = *param_2;
  param_1[1] = param_2[1];
  param_1[3] = param_2[3];
  param_1[4] = param_2[4];
  param_1[5] = param_2[5];
  param_1[6] = param_2[6];
  param_1[9] = param_2[9];
  param_1[10] = param_2[10];
  param_1[0xb] = param_2[0xb];
  param_1[0xc] = param_2[0xc];
  param_1[0xd] = param_2[0xd];
  *(undefined1 *)(param_1 + 0xe) = *(undefined1 *)(param_2 + 0xe);
  param_1[0xf] = param_2[0xf];
  param_1[0x10] = param_2[0x10];
  param_1[2] = 100;
  return;
}
```

### CWorldTimer::AddWorldTimes  gog=0x80b3d0 (param_bytes=24, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x0051aef0(mapper) delta=-0x2f04e0 align=6/6 size=108 retn=24 gapG/S=112/112 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD lhsDays, DWORD lhsMS, DWORD rhsDays, DWORD rhsMS, DWORD* outDays, DWORD* outMS) [param_bytes=24]
callers=10 callees=
```
undefined4 __thiscall
FUN_0051aef0(int param_1,int param_2,uint param_3,int param_4,uint param_5,int *param_6,
            uint *param_7)
{
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

### CWorldTimer::SubtractWorldTimes  gog=0x80b440 (param_bytes=24, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x0051af60(mapper) delta=-0x2f04e0 align=6/6 size=115 retn=24 gapG/S=128/128 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD lhsDays, DWORD lhsMS, DWORD rhsDays, DWORD rhsMS, DWORD* outDays, DWORD* outMS) [param_bytes=24]
callers=39 callees=0051afe0=gog:CWorldTimer::CompareWorldTimes
```
undefined4 __thiscall
FUN_0051af60(int param_1,int param_2,int param_3,int param_4,int param_5,int *param_6,uint *param_7)
{
  iVar1 = FUN_0051afe0(param_2,param_3,param_4,param_5);
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

### CWorldTimer::CompareWorldTimes  gog=0x80b4c0 (param_bytes=16, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x0051afe0(mapper) delta=-0x2f04e0 align=6/6 size=100 retn=16 gapG/S=112/112 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD lhsDay, DWORD lhsMS, DWORD rhsDay, DWORD rhsMS) [param_bytes=16]
callers=12 callees=
```
undefined4 __thiscall FUN_0051afe0(int param_1,uint param_2,uint param_3,uint param_4,uint param_5)
{
  if ((param_3 < *(uint *)(param_1 + 0x3c)) && (param_5 < *(uint *)(param_1 + 0x3c))) {
    if (param_2 == param_4) {
      if (param_5 < param_3) {
        uVar1 = 1;
      }
      else if (param_3 < param_5) {
        uVar1 = 0xffffffff;
      }
      else {
        uVar1 = 0;
      }
    }
    else if (param_4 < param_2) {
      uVar1 = 1;
    }
    else {
      uVar1 = 0xffffffff;
    }
  }
  else {
    uVar1 = 0xfffffffe;
  }
  return uVar1;
}
```

### CWorldTimer::PauseWorldTimer  gog=0x80b530 (param_bytes=0, notes='void')
steam: cand=0x0051b050(mapper) delta=-0x2f04e0 align=6/6 size=66 retn=0 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=7 callees=0051ad20=gog:CWorldTimer::GetWorldTime
```
void __fastcall FUN_0051b050(int param_1)
{
  if (*(int *)(param_1 + 0x24) == 0) {
    FUN_0051ad20(&local_c,&local_8);
    *(undefined4 *)(param_1 + 0x28) = local_c;
    *(undefined4 *)(param_1 + 0x2c) = local_8;
    *(undefined4 *)(param_1 + 0x24) = 1;
  }
  return;
}
```

### CWorldTimer::UnpauseWorldTimer  gog=0x80b580 (param_bytes=0, notes='void')
steam: cand=0x0051b0a0(mapper) delta=-0x2f04e0 align=6/6 size=54 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=4 callees=0051ac30=gog:CWorldTimer::SetWorldTime
```
void __fastcall FUN_0051b0a0(int param_1)
{
  if (*(int *)(param_1 + 0x24) == 1) {
    FUN_0051ac30(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c),0);
    *(undefined4 *)(param_1 + 0x24) = 0;
  }
  return;
}
```

### CWorldTimer::GetWorldTimeHour  gog=0x80b5e0 (param_bytes=0, notes='void')
steam: cand=0x0051b100(mapper) delta=-0x2f04e0 align=6/6 size=49 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=1 callees=0051ab40=gog:CWorldTimer::ConvertFromTimeOfDay, 0051b0e0=Global::GetWorldTimeTimeOfDay
```
undefined4 __fastcall FUN_0051b100(undefined4 param_1)
{
  undefined1 local_10 [4];
  undefined1 local_c [4];
  undefined1 local_8 [4];
  puVar5 = local_8;
  puVar4 = local_10;
  puVar3 = local_c;
  puVar2 = &local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_14;
}
```

### CWorldTimer::GetWorldTimeMinute  gog=0x80b620 (param_bytes=0, notes='void')
steam: cand=0x0051b140(mapper) delta=-0x2f04e0 align=6/6 size=49 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=1 callees=0051ab40=gog:CWorldTimer::ConvertFromTimeOfDay, 0051b0e0=Global::GetWorldTimeTimeOfDay
```
undefined4 __fastcall FUN_0051b140(undefined4 param_1)
{
  undefined1 local_14 [4];
  undefined1 local_10 [4];
  undefined1 local_8 [4];
  puVar5 = local_8;
  puVar4 = local_10;
  puVar3 = &local_c;
  puVar2 = local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_c;
}
```

### CWorldTimer::GetWorldTimeSecond  gog=0x80b660 (param_bytes=0, notes='void')
steam: cand=0x0051b180(mapper) delta=-0x2f04e0 align=6/6 size=49 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=1 callees=0051ab40=gog:CWorldTimer::ConvertFromTimeOfDay, 0051b0e0=Global::GetWorldTimeTimeOfDay
```
undefined4 __fastcall FUN_0051b180(undefined4 param_1)
{
  undefined1 local_14 [4];
  undefined1 local_c [4];
  undefined1 local_8 [4];
  puVar5 = local_8;
  puVar4 = &local_10;
  puVar3 = local_c;
  puVar2 = local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_10;
}
```

### CWorldTimer::GetWorldTimeMillisecond  gog=0x80b6a0 (param_bytes=0, notes='void')
steam: cand=0x0051b1c0(mapper) delta=-0x2f04e0 align=6/6 size=49 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=1 callees=0051ab40=gog:CWorldTimer::ConvertFromTimeOfDay, 0051b0e0=Global::GetWorldTimeTimeOfDay
```
undefined4 __fastcall FUN_0051b1c0(undefined4 param_1)
{
  undefined1 local_14 [4];
  undefined1 local_10 [4];
  undefined1 local_c [4];
  puVar5 = &local_8;
  puVar4 = local_10;
  puVar3 = local_c;
  puVar2 = local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_8;
}
```

