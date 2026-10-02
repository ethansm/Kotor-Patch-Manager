# Digest D_Gob

### Gob::SetScene  gog=0x84b0b0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00458b70(mapper) delta=-0x3f2540 align=0/6 size=2452 retn=4 gapG/S=144/2464 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* scene) [param_bytes=4]
name_map: Gob ctor [medium] `DAT_00A1FF68++` | asm 0x458f99 `MOV EDX,[0x00a1ff68] / ADD EDX,0x1`
callers=4 callees=00478c60, 0050c380, 00919723=operator new, 004741e0=SafePointer_T_::operator=, 00919671, 0091cff0, 00458ae0, 004595f0, 00463ed0, 00458a80, 009196ee=__security_check_cookie, 00465230=CAurScene FindByName
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
void __thiscall FUN_00458b70(undefined4 *param_1,int param_2)
{
  char local_58 [64];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00949ddf;
  local_10 = ExceptionList;
  local_18 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00463ed0(0);
  *param_1 = CAurObject::vftable;
  local_8 = 0;
  *param_1 = Gob::vftable;
  FUN_00463ed0(0);
  local_8._0_1_ = 1;
  param_1[0x29] = 0;
  param_1[0x2a] = 0;
  param_1[0x2b] = 0;
  param_1[0x2f] = 0;
  param_1[0x2e] = 0;
  param_1[0x2d] = 0;
  param_1[0x2c] = 0x3f800000;
  param_1[0x30] = 0;
  param_1[0x31] = 0;
  param_1[0x32] = 0;
  param_1[0x36] = 0;
  param_1[0x35] = 0;
  param_1[0x34] = 0;
  param_1[0x33] = 0x3f800000;
  FUN_00463ed0(0);
  local_8._0_1_ = 2;
  param_1[0x3e] = 0;
  param_1[0x3f] = 0;
  param_1[0x40] = 0;
  param_1[0x43] = 0;
  param_1[0x44] = 0;
  param_1[0x45] = 0;
  FUN_00463ed0(0);
  local_8._0_1_ = 3;
  FUN_00463ed0(0);
  local_8._0_1_ = 4;
  FUN_00478c60();
  local_8._0_1_ = 5;
  param_1[0x53] = 0;
... (146 more lines)
```

### Gob::SetProcessFlag  gog=0x84bb30 (param_bytes=0, notes='void')
steam: cand=0x00459560(mapper) delta=-0x3f25d0 align=3/6 size=31 retn=0 gapG/S=80/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=2 callees=00463a90
```
void __fastcall FUN_00459560(undefined4 *param_1)
{
  *param_1 = CAurObject::vftable;
  FUN_00463a90();
  return;
}
```

### Gob::GetName  gog=0x84bcd0 (param_bytes=0, notes='void')
steam: cand=0x004596f0(mapper) delta=-0x3f25e0 align=4/6 size=17 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: CResRef* (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
int __fastcall FUN_004596f0(int param_1)
{
  return param_1 + 100;
}
```

### Gob::GetPosition  gog=0x84bcf0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00459710(mapper) delta=-0x3f25e0 align=5/6 size=43 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector* outPosition) [param_bytes=4]
callers=0 callees=
```
undefined4 * __thiscall FUN_00459710(int param_1,undefined4 *param_2)
{
  *param_2 = *(undefined4 *)(param_1 + 0xa4);
  param_2[1] = *(undefined4 *)(param_1 + 0xa8);
  param_2[2] = *(undefined4 *)(param_1 + 0xac);
  return param_2;
}
```

### Gob::GetOrientation  gog=0x84bd20 (param_bytes=4, notes='Gob * this, undefined4 param_1')
steam: cand=0x00459740(mapper) delta=-0x3f25e0 align=6/6 size=49 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Quaternion* orientation) [param_bytes=4]
callers=0 callees=
```
/* Library Function - Single Match
    public: class CRect __thiscall CRichEditView::GetMargins(void)const
   Library: Visual Studio 2010 Debug */
undefined4 * __thiscall CRichEditView::GetMargins(CRichEditView *this)
{
  *in_stack_00000004 = *(undefined4 *)(this + 0xb0);
  in_stack_00000004[1] = *(undefined4 *)(this + 0xb4);
  in_stack_00000004[2] = *(undefined4 *)(this + 0xb8);
  in_stack_00000004[3] = *(undefined4 *)(this + 0xbc);
  return in_stack_00000004;
}
```

### Gob::GetPreviousPosition  gog=0x84bd60 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00459780(mapper) delta=-0x3f25e0 align=6/6 size=43 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector* outPosition) [param_bytes=4]
callers=0 callees=
```
undefined4 * __thiscall FUN_00459780(int param_1,undefined4 *param_2)
{
  *param_2 = *(undefined4 *)(param_1 + 0xc0);
  param_2[1] = *(undefined4 *)(param_1 + 0xc4);
  param_2[2] = *(undefined4 *)(param_1 + 200);
  return param_2;
}
```

### Gob::SetIgnoreHitCheck  gog=0x84be10 (param_bytes=1, notes='undefined1 param_1')
steam: cand=0x00459830(bracket) delta=-0x3f25e0 align=/ size=25 retn=4 gapG/S=32/ cc_guess=
wrapper: void (__thiscall) (void* thisPtr, bool ignore) [param_bytes=4]
name_map: CSWSObject::GetPortrait [medium] gog 0x663840; vote; retn=4
callers=0 callees=
```
void __thiscall FUN_00459830(int param_1,undefined1 param_2)
{
  *(undefined1 *)(param_1 + 0x1a9) = param_2;
  return;
}
```

### Gob::TurnOffShadows  gog=0x84be30 (param_bytes=0, notes='void')
steam: cand=0x00459850(mapper) delta=-0x3f25e0 align=5/6 size=23 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_00459850(int param_1)
{
  *(undefined2 *)(param_1 + 0xee) = 0;
  return;
}
```

### Gob::TurnOnShadows  gog=0x84be50 (param_bytes=0, notes='void')
steam: cand=0x00459870(mapper) delta=-0x3f25e0 align=5/6 size=26 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_00459870(int param_1)
{
  *(undefined2 *)(param_1 + 0xee) = 1;
  return;
}
```

### Gob::IsAnimating  gog=0x84be70 (param_bytes=0, notes='void')
steam: cand=0x00459890(mapper) delta=-0x3f25e0 align=5/6 size=20 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: unsigned char (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSObject::GetPortraitId [medium] gog 0x6638a0; vote; retn=0
callers=0 callees=
```
undefined1 __fastcall FUN_00459890(int param_1)
{
  return *(undefined1 *)(param_1 + 0x1aa);
}
```

### Gob::DisableInViewVolume  gog=0x84be90 (param_bytes=0, notes='void')
steam: cand=0x004598b0(mapper) delta=-0x3f25e0 align=6/6 size=18 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSObject::GetFirstName [medium] gog 0x6638c0; vote; retn=0
callers=0 callees=
```
void __fastcall FUN_004598b0(int param_1)
{
  *(undefined1 *)(param_1 + 0x11) = 0;
  return;
}
```

### Gob::IsInViewVolume  gog=0x84beb0 (param_bytes=0, notes='void')
steam: cand=0x004598d0(mapper) delta=-0x3f25e0 align=5/6 size=17 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: unsigned char (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
undefined1 __fastcall FUN_004598d0(int param_1)
{
  return *(undefined1 *)(param_1 + 0x11);
}
```

### Gob::SetAsCharacter  gog=0x84bed0 (param_bytes=0, notes='void')
steam: cand=0x004598f0(mapper) delta=-0x3f25e0 align=4/6 size=54 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_004598f0(int param_1)
{
  if (*(int *)(param_1 + 0x84) != 0) {
    *(ushort *)(*(int *)(param_1 + 0x84) + 0x50) = *(ushort *)(*(int *)(param_1 + 0x84) + 0x50) | 4;
  }
  return;
}
```

### Gob::SetRenderPersonalSpace  gog=0x84bf30 (param_bytes=1, notes='undefined1 param_1')
steam: cand=0x00459950(bracket) delta=-0x3f25e0 align=/ size=25 retn=4 gapG/S=32/ cc_guess=
wrapper: void (__thiscall) (void* thisPtr, bool render) [param_bytes=4]
callers=0 callees=
```
void __thiscall FUN_00459950(int param_1,undefined1 param_2)
{
  *(undefined1 *)(param_1 + 0x180) = param_2;
  return;
}
```

### Gob::SetMotionBlurred  gog=0x84bf50 (param_bytes=1, notes='undefined1 param_1')
steam: cand=0x00459970(bracket) delta=-0x3f25e0 align=/ size=25 retn=4 gapG/S=32/ cc_guess=
wrapper: void (__thiscall) (void* thisPtr, bool motionBlurred) [param_bytes=4]
callers=0 callees=
```
void __thiscall FUN_00459970(int param_1,undefined1 param_2)
{
  *(undefined1 *)(param_1 + 0x181) = param_2;
  return;
}
```

### Gob::GetMotionBlurred  gog=0x84bf70 (param_bytes=0, notes='void')
steam: cand=0x00459990(mapper) delta=-0x3f25e0 align=4/6 size=20 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: unsigned char (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
undefined1 __fastcall FUN_00459990(int param_1)
{
  return *(undefined1 *)(param_1 + 0x181);
}
```

### Gob::SetMaxBlurLength  gog=0x84bf90 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x004599b0(mapper) delta=-0x3f25e0 align=4/6 size=25 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, float length) [param_bytes=4]
callers=0 callees=
```
void __thiscall FUN_004599b0(int param_1,undefined4 param_2)
{
  *(undefined4 *)(param_1 + 0x1ac) = param_2;
  return;
}
```

### Gob::GetMaxBlurLength  gog=0x84bfb0 (param_bytes=0, notes='void')
steam: cand=0x004599d0(mapper) delta=-0x3f25e0 align=4/6 size=20 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: double (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
float10 __fastcall FUN_004599d0(int param_1)
{
  return (float10)*(float *)(param_1 + 0x1ac);
}
```

### Gob::EnableDistortion  gog=0x84bfd0 (param_bytes=0, notes='void')
steam: cand=0x004599f0(mapper) delta=-0x3f25e0 align=5/6 size=21 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_004599f0(int param_1)
{
  *(undefined1 *)(param_1 + 0x1a8) = 1;
  return;
}
```

### Gob::DisableDistortion  gog=0x84bff0 (param_bytes=0, notes='void')
steam: cand=0x00459a10(mapper) delta=-0x3f25e0 align=4/6 size=21 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_00459a10(int param_1)
{
  *(undefined1 *)(param_1 + 0x1a8) = 0;
  return;
}
```

### Gob::EnableForceAnimations  gog=0x84c340 (param_bytes=0, notes='void')
steam: cand=0x00459d60(mapper) delta=-0x3f25e0 align=6/6 size=18 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_00459d60(int param_1)
{
  *(undefined1 *)(param_1 + 0x10) = 1;
  return;
}
```

### Gob::DisableForceAnimations  gog=0x84c360 (param_bytes=0, notes='void')
steam: cand=0x00459d80(mapper) delta=-0x3f25e0 align=6/6 size=18 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_00459d80(int param_1)
{
  *(undefined1 *)(param_1 + 0x10) = 0;
  return;
}
```

### Gob::IsForceAnimated  gog=0x84c380 (param_bytes=0, notes='void')
steam: cand=0x00459da0(mapper) delta=-0x3f25e0 align=6/6 size=17 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: unsigned char (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
undefined1 __fastcall FUN_00459da0(int param_1)
{
  return *(undefined1 *)(param_1 + 0x10);
}
```

### Gob::EnableAlwaysRender  gog=0x84c3a0 (param_bytes=0, notes='void')
steam: cand=0x00459dc0(mapper) delta=-0x3f25e0 align=6/6 size=18 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_00459dc0(int param_1)
{
  *(undefined1 *)(param_1 + 0x12) = 1;
  return;
}
```

### Gob::DisableAlwaysRender  gog=0x84c3c0 (param_bytes=0, notes='void')
steam: cand=0x00459de0(mapper) delta=-0x3f25e0 align=6/6 size=18 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_00459de0(int param_1)
{
  *(undefined1 *)(param_1 + 0x12) = 0;
  return;
}
```

### Gob::DisableInCutscene  gog=0x84c3e0 (param_bytes=0, notes='void')
steam: cand=0x00459e00(mapper) delta=-0x3f25e0 align=6/6 size=18 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_00459e00(int param_1)
{
  *(undefined1 *)(param_1 + 0x14) = 0;
  return;
}
```

### Gob::IsInCutscene  gog=0x84c400 (param_bytes=0, notes='void')
steam: cand=0x00459e20(mapper) delta=-0x3f25e0 align=5/6 size=17 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: unsigned char (__thiscall) (void* thisPtr) [param_bytes=0]
callers=5 callees=
```
undefined1 __fastcall FUN_00459e20(int param_1)
{
  return *(undefined1 *)(param_1 + 0x14);
}
```

### Gob::GetCutsceneDummyPosition  gog=0x84c420 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00459e40(mapper) delta=-0x3f25e0 align=5/6 size=154 retn=4 gapG/S=160/160 cc_guess=thiscall
wrapper: Vector* (__thiscall) (void* thisPtr, Vector* outPosition) [param_bytes=4]
callers=0 callees=00456b00
```
undefined4 * __thiscall FUN_00459e40(int param_1,undefined4 *param_2)
{
  local_8 = (int *)FUN_00456b00(*(undefined4 *)(param_1 + 0x98),"cutscenedummy");
  if (local_8 == (int *)0x0) {
    *param_2 = 0;
    param_2[1] = 0;
    param_2[2] = 0;
  }
  else {
    local_24 = 0;
    local_20 = 0;
    local_1c = 0;
    local_c = 0;
    local_10 = 0;
    local_14 = 0;
    local_18 = 0x3f800000;
    (**(code **)(*local_8 + 0xc))(&local_24,&local_18);
    *param_2 = local_24;
    param_2[1] = local_20;
    param_2[2] = local_1c;
  }
  return param_2;
}
```

### Gob::IsAlwaysRendering  gog=0x84c4c0 (param_bytes=0, notes='void')
steam: cand=0x00459ee0(mapper) delta=-0x3f25e0 align=5/6 size=17 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: unsigned char (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
undefined1 __fastcall FUN_00459ee0(int param_1)
{
  return *(undefined1 *)(param_1 + 0x12);
}
```

### Gob::DisableFog  gog=0x84c550 (param_bytes=0, notes='void')
steam: cand=0x00459f70(mapper) delta=-0x3f25e0 align=5/6 size=31 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=00459f00
```
void __fastcall FUN_00459f70(int param_1)
{
  FUN_00459f00(*(undefined4 *)(param_1 + 0x98),0);
  return;
}
```

### Gob::EnableFog  gog=0x84c570 (param_bytes=0, notes='void')
steam: cand=0x00459f90(mapper) delta=-0x3f25e0 align=5/6 size=31 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=00459f00
```
void __fastcall FUN_00459f90(int param_1)
{
  FUN_00459f00(*(undefined4 *)(param_1 + 0x98),1);
  return;
}
```

### Gob::GetBoundingBox  gog=0x84c590 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00459fb0(mapper) delta=-0x3f25e0 align=5/6 size=68 retn=8 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector* min, Vector* max) [param_bytes=8]
callers=0 callees=
```
void __thiscall FUN_00459fb0(int param_1,undefined4 *param_2,undefined4 *param_3)
{
  *param_2 = *(undefined4 *)(param_1 + 0x14c);
  param_2[1] = *(undefined4 *)(param_1 + 0x150);
  param_2[2] = *(undefined4 *)(param_1 + 0x154);
  *param_3 = *(undefined4 *)(param_1 + 0x158);
  param_3[1] = *(undefined4 *)(param_1 + 0x15c);
  param_3[2] = *(undefined4 *)(param_1 + 0x160);
  return;
}
```

### Gob::EnableInViewVolume  gog=0x84c5e0 (param_bytes=0, notes='void')
steam: cand=0x0045a000(mapper) delta=-0x3f25e0 align=6/6 size=41 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
strings: propagateviewvolume
```
void __fastcall FUN_0045a000(int *param_1)
{
  *(undefined1 *)((int)param_1 + 0x11) = 1;
  (**(code **)(*param_1 + 0x1b4))("propagateviewvolume",0);
  return;
}
```

### Gob::LoadAddInAnimations  gog=0x84c610 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0045a030(mapper) delta=-0x3f25e0 align=6/6 size=277 retn=4 gapG/S=288/288 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, char* modelName) [param_bytes=4]
callers=0 callees=0048e160=Global::FindModel, 0044b440, 0091caeb, 00452d40=CAurModel::AddRef, 0048e2a0=Global::LoadModel_2, 0091cff0, 0091d000, 009196ee=__security_check_cookie
```
void __thiscall FUN_0045a030(int param_1,undefined4 param_2)
{
  char local_4c [64];
  FILE *local_8;
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  uVar1 = FUN_0048e160(param_2);
  *(undefined4 *)(param_1 + 0x90) = uVar1;
  if (*(int *)(param_1 + 0x90) == 0) {
    FUN_0091cff0(local_4c,param_2);
    FUN_0091d000(local_4c,&DAT_0098b9a8);
    local_8 = _fopen(local_4c,"r");
    if (local_8 == (FILE *)0x0) goto LAB_0045a135;
    uVar1 = FUN_0048e2a0(local_8,0);
    *(undefined4 *)(param_1 + 0x90) = uVar1;
    FUN_0044b440(*(undefined4 *)(param_1 + 0x84));
    for (local_50 = 0; local_50 < *(int *)(*(int *)(param_1 + 0x84) + 0x5c); local_50 = local_50 + 1
        ) {
      FUN_0044b440(*(undefined4 *)(param_1 + 0x84));
    }
  }
  FUN_00452d40(param_1);
LAB_0045a135:
  __security_check_cookie(local_c ^ (uint)&stack0xfffffffc);
  return;
}
```

### Gob::RemoveAddInAnimations  gog=0x84c730 (param_bytes=0, notes='void')
steam: cand=0x0045a150(mapper) delta=-0x3f25e0 align=6/6 size=54 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=00452cf0=CAurModel::Release (refcount `+0x54`)
```
void __fastcall FUN_0045a150(int param_1)
{
  if (*(int *)(param_1 + 0x90) != 0) {
    FUN_00452cf0(param_1);
  }
  *(undefined4 *)(param_1 + 0x90) = 0;
  return;
}
```

### Gob::EnableRenderGobBBox  gog=0x84e4b0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0045bed0(mapper) delta=-0x3f25e0 align=5/6 size=55 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector* color) [param_bytes=4]
callers=0 callees=
```
void __thiscall FUN_0045bed0(int param_1,undefined4 *param_2)
{
  *(undefined2 *)(param_1 + 0xf4) = 1;
  *(undefined4 *)(param_1 + 0x174) = *param_2;
  *(undefined4 *)(param_1 + 0x178) = param_2[1];
  *(undefined4 *)(param_1 + 0x17c) = param_2[2];
  return;
}
```

### Gob::DisableRenderGobBBox  gog=0x84e4f0 (param_bytes=0, notes='void')
steam: cand=0x0045bf10(mapper) delta=-0x3f25e0 align=5/6 size=23 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_0045bf10(int param_1)
{
  *(undefined2 *)(param_1 + 0xf4) = 0;
  return;
}
```

### Gob::EnableRenderBumpedOut  gog=0x84e530 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x0045bf50(mapper) delta=-0x3f25e0 align=4/6 size=1343 retn=8 gapG/S=1344/1344 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CAurGob::ApplyTXIToNamedTexture (Gob slot 0x150) [low] GetOrCreate texture then TXI file/memory branch by GetFlagEA | `cVar1 = (**(code **)(*local_28 + 0x34))();` ... `FUN_004
callers=0 callees=00463b20, 00484d10=Global::AurMessageGetToken, 004741e0=SafePointer_T_::operator=, 00475f70, 009196ee=__security_check_cookie, 00475ab0, 004274e0=Global::AurTextureGetReference, 00484dc0, 004762f0=ModelFile::ReadBlock(n, src), 009370c7=_stricmp, 00463ba0, 00475ea0=CloseModelStream, 0091e050, 00919f10=_strlen ...
strings: punchthrough | additive | blending
```
/* WARNING: Function: __alloca_probe replaced with injection: alloca_probe */
void __thiscall FUN_0045bf50(int *param_1,undefined4 param_2,int param_3)
{
  size_t sVar6;
  char local_2068 [8196];
  char local_58 [32];
  void *local_10;
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
        for (; (*local_24 == ' ' || (*local_24 == '\t')); local_24 = local_24 + 1) {
        }
... (102 more lines)
```

### Gob::DisableRenderBumpedOut  gog=0x84ea70 (param_bytes=0, notes='void')
steam: cand=0x0045c490(mapper) delta=-0x3f25e0 align=4/6 size=234 retn=0 gapG/S=240/240 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=00463b20, 004741e0=SafePointer_T_::operator=, 00463ed0, 004276a0=ReleaseTextureProxy (delete if back-ref count 0)
```
void __fastcall FUN_0045c490(int *param_1)
{
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094d3d8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(char *)((int)param_1 + 0x15) != '\0') {
    *(undefined1 *)((int)param_1 + 0x15) = 0;
    FUN_00463ed0(0);
    local_8 = 0;
    (**(code **)(*param_1 + 0x110))(FUN_0045bf30,&local_20,uVar1);
    for (local_24 = 0; local_24 < local_1c; local_24 = local_24 + 1) {
      local_14 = *(undefined4 *)(*(int *)(*(int *)(local_20 + local_24 * 4) + 0x44) + 0xc);
      FUN_004741e0(0);
      FUN_004276a0(local_14);
    }
    param_1[0x52] = 0;
    local_8 = 0xffffffff;
    FUN_00463b20();
  }
  ExceptionList = local_10;
  return;
}
```

### Gob::SetLightPriority  gog=0x84eb60 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0045c580(mapper) delta=-0x3f25e0 align=4/6 size=154 retn=4 gapG/S=160/160 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int priority) [param_bytes=4]
callers=0 callees=00463b20, 00463ed0, 0045c620
```
void __thiscall FUN_0045c580(int param_1,undefined4 param_2)
{
  void *local_10;
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

### Gob::GetMaximumLightRadius  gog=0x84ec70 (param_bytes=0, notes='void')
steam: cand=0x0045c690(mapper) delta=-0x3f25e0 align=5/6 size=180 retn=0 gapG/S=192/192 cc_guess=thiscall
wrapper: double (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=00463b20, 004631c0, 00463ed0
```
float10 FUN_0045c690(void)
{
  void *local_10;
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

### Gob::SetColorShifting  gog=0x84ed30 (param_bytes=20, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x0045c750(mapper) delta=-0x3f25e0 align=5/6 size=95 retn=20 gapG/S=96/96 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector color, float alpha, int propagate) [param_bytes=12]
callers=0 callees=
strings: propagatecolorshift
```
void __thiscall
FUN_0045c750(int *param_1,int param_2,int param_3,int param_4,int param_5,int param_6)
{
  *(undefined2 *)(param_1 + 0x3c) = 1;
  param_1[0x3e] = param_2;
  param_1[0x3f] = param_3;
  param_1[0x40] = param_4;
  param_1[0x41] = param_5;
  if (param_6 != 0) {
    (**(code **)(*param_1 + 0x1b4))("propagatecolorshift",0);
  }
  return;
}
```

### Gob::SetIllumination  gog=0x84ed90 (param_bytes=16, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x0045c7b0(mapper) delta=-0x3f25e0 align=4/6 size=83 retn=16 gapG/S=96/96 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector color, int propagate) [param_bytes=8]
callers=0 callees=
strings: propagateillumination
```
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

### Gob::GetMinimumBoundingBox_2  gog=0x8525d0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00460040(mapper) delta=-0x3f2590 align=5/6 size=85 retn=8 gapG/S=96/96 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector* min, Vector* max) [param_bytes=8]
callers=0 callees=
```
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

### Gob::GetMinimumBoundingSphere  gog=0x852630 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x004600a0(mapper) delta=-0x3f2590 align=5/6 size=71 retn=8 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector* center, float* radius) [param_bytes=8]
callers=0 callees=
```
void __thiscall FUN_004600a0(int *param_1,int *param_2,int *param_3)
{
  (**(code **)(*param_1 + 0x1d0))();
  *param_2 = param_1[0x5a];
  param_2[1] = param_1[0x5b];
  param_2[2] = param_1[0x5c];
  *param_3 = param_1[0x59];
  return;
}
```

### Gob::RestoreTexture  gog=0x853070 (param_bytes=0, notes='void')
steam: cand=0x00460ae0(mapper) delta=-0x3f2590 align=4/6 size=203 retn=0 gapG/S=320/208 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=00463b20, 00460bb0, 004db0a0, 00463ed0, 004545a0
```
undefined4 FUN_00460ae0(void)
{
  void *local_10;
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

### Gob::SetEnvironmentMap  gog=0x853890 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00461300(mapper) delta=-0x3f2590 align=4/6 size=389 retn=4 gapG/S=400/400 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, char* envMapName) [param_bytes=4]
callers=0 callees=00463b20, 004274e0=Global::AurTextureGetReference, 004741e0=SafePointer_T_::operator=, 00422e20=CAurTexture::SetIsEnvironmentMapped (SetFlagDE), 0091cff0, 00463ed0, 004276a0=ReleaseTextureProxy (delete if back-ref count 0), 00460a80, 009196ee=__security_check_cookie
```
void __thiscall FUN_00461300(int param_1,char *param_2)
{
  undefined1 local_70 [4];
  undefined1 local_58 [64];
  void *local_10;
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
    FUN_004276a0(local_14);
    if (*(int *)(param_1 + 0x1d4) == 0) {
      local_5c = 0;
    }
    else {
      local_5c = 1;
      FUN_00463ed0(0);
      local_8 = 0;
      FUN_00460a80(*(undefined4 *)(param_1 + 0x98),local_70);
      for (local_64 = 0; local_64 < local_6c; local_64 = local_64 + 1) {
        FUN_00422e20(1);
      }
      local_8 = 0xffffffff;
      FUN_00463b20();
    }
  }
  else {
    FUN_004276a0(local_60);
  }
  ExceptionList = local_10;
  __security_check_cookie(local_18 ^ (uint)&stack0xfffffffc);
  return;
}
```

### Gob::SetPosition  gog=0x853a20 (param_bytes=16, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x00461490(mapper) delta=-0x3f2590 align=6/6 size=69 retn=16 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, Vector* outPosition, Vector position) [param_bytes=8]
callers=0 callees=
```
undefined4 * __thiscall
FUN_00461490(int param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)
{
  *(undefined4 *)(param_1 + 0xa4) = param_3;
  *(undefined4 *)(param_1 + 0xa8) = param_4;
  *(undefined4 *)(param_1 + 0xac) = param_5;
  *param_2 = *(undefined4 *)(param_1 + 0xa4);
  param_2[1] = *(undefined4 *)(param_1 + 0xa8);
  param_2[2] = *(undefined4 *)(param_1 + 0xac);
  return param_2;
}
```

### Gob::SetOrientation  gog=0x853a70 (param_bytes=20, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x004614e0(mapper) delta=-0x3f2590 align=6/6 size=80 retn=20 gapG/S=80/80 cc_guess=thiscall
wrapper: Quaternion* (__thiscall) (void* thisPtr, Quaternion* outOrientation, Quaternion orientation) [param_bytes=8]
callers=0 callees=
```
undefined4 * __thiscall
FUN_004614e0(int param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)
{
  *(undefined4 *)(param_1 + 0xb0) = param_3;
  *(undefined4 *)(param_1 + 0xb4) = param_4;
  *(undefined4 *)(param_1 + 0xb8) = param_5;
  *(undefined4 *)(param_1 + 0xbc) = param_6;
  *param_2 = *(undefined4 *)(param_1 + 0xb0);
  param_2[1] = *(undefined4 *)(param_1 + 0xb4);
  param_2[2] = *(undefined4 *)(param_1 + 0xb8);
  param_2[3] = *(undefined4 *)(param_1 + 0xbc);
  return param_2;
}
```

### Gob::GetModelName  gog=0x853b60 (param_bytes=0, notes='void')
steam: cand=0x004615d0(mapper) delta=-0x3f2590 align=6/6 size=39 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: CResRef* (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
int __fastcall FUN_004615d0(int param_1)
{
  if (*(int *)(param_1 + 0x84) == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(param_1 + 0x84) + 8;
  }
  return iVar1;
}
```

### Gob::Append  gog=0x8545b0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00462020(mapper) delta=-0x3f2590 align=2/6 size=335 retn=8 gapG/S=336/336 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, char* modelName, char* partName) [param_bytes=8]
callers=0 callees=0048e160=Global::FindModel, 0045fde0, 00461ef0, 00452d40=CAurModel::AddRef, 00456cd0, 00461ed0, 00456b00, 00473f40=vector<T*>::push_back (`ret 4`)
```
undefined4 __thiscall FUN_00462020(int *param_1,undefined4 param_2,undefined4 param_3)
{
  iVar1 = FUN_00456b00(param_1[0x26],param_3);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    iVar3 = FUN_0048e160(param_2);
    if (iVar3 == 0) {
      uVar2 = 0;
    }
    else {
      FUN_00452d40(param_1);
      iVar4 = FUN_00456cd0(*(undefined4 *)(iVar3 + 0x28),param_1);
      *(undefined4 *)(iVar4 + 8) = 0;
      *(undefined4 *)(iVar4 + 0xc) = 0;
      *(undefined4 *)(iVar4 + 0x10) = 0;
      *(undefined4 *)(iVar4 + 0x14) = 0x3f800000;
      *(undefined4 *)(iVar4 + 0x18) = 0;
      *(undefined4 *)(iVar4 + 0x1c) = 0;
      *(undefined4 *)(iVar4 + 0x20) = 0;
      FUN_00473f40(iVar3);
      FUN_00473f40(iVar4);
      FUN_00473f40(iVar4);
      *(int *)(iVar4 + 0x3c) = iVar1;
      FUN_0045fde0(param_1,iVar4,&DAT_00986925);
      FUN_00461ed0();
      FUN_00461ef0(param_1,iVar4);
      (**(code **)(*param_1 + 0x1b4))("_EmitterTarget",0);
      uVar2 = 1;
    }
  }
  return uVar2;
}
```

### Gob::Amputate  gog=0x854700 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00462170(mapper) delta=-0x3f2590 align=3/6 size=421 retn=4 gapG/S=432/432 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, char* partName) [param_bytes=4]
callers=0 callees=0049b840, 00463b20, 00449170, 00452cf0=CAurModel::Release (refcount `+0x54`), 00461ed0, 00463ed0, 00456b00, 00473f40=vector<T*>::push_back (`ret 4`), 00476620, 004142e0
```
undefined4 __thiscall FUN_00462170(int param_1,undefined4 param_2)
{
  void *local_10;
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
... (1 more lines)
```

### Gob::GetPartLocalPosition  gog=0x855260 (param_bytes=12, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3')
steam: cand=0x00462cd0(mapper) delta=-0x3f2590 align=5/6 size=121 retn=12 gapG/S=128/128 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, Vector* outPosition, Quaternion* outOrientation) [param_bytes=8]
callers=0 callees=
```
undefined4 __thiscall
FUN_00462cd0(int *param_1,undefined4 param_2,undefined4 *param_3,undefined4 *param_4)
{
  iVar1 = (**(code **)(*param_1 + 0x10c))(param_2);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    if (param_3 != (undefined4 *)0x0) {
      *param_3 = *(undefined4 *)(iVar1 + 8);
      param_3[1] = *(undefined4 *)(iVar1 + 0xc);
      param_3[2] = *(undefined4 *)(iVar1 + 0x10);
    }
    if (param_4 != (undefined4 *)0x0) {
      *param_4 = *(undefined4 *)(iVar1 + 0x14);
      param_4[1] = *(undefined4 *)(iVar1 + 0x18);
      param_4[2] = *(undefined4 *)(iVar1 + 0x1c);
      param_4[3] = *(undefined4 *)(iVar1 + 0x20);
    }
    uVar2 = 1;
  }
  return uVar2;
}
```

### Gob::AddAttachment  gog=0x8557b0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x004632c0(mapper) delta=-0x3f24f0 align=3/6 size=55 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* attachment) [param_bytes=4]
callers=1 callees=00473f40=vector<T*>::push_back (`ret 4`), 00476620
```
void FUN_004632c0(undefined4 param_1)
{
  iVar1 = FUN_00476620(param_1);
  if (iVar1 == 0) {
    FUN_00473f40(param_1);
  }
  return;
}
```

### Gob::RemoveAttachment  gog=0x8557f0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00463300(mapper) delta=-0x3f24f0 align=4/6 size=53 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* attachment) [param_bytes=4]
callers=1 callees=00449170, 00476620
```
void FUN_00463300(undefined4 param_1)
{
  iVar1 = FUN_00476620(param_1);
  if (iVar1 != 0) {
    FUN_00449170(param_1);
  }
  return;
}
```

### Gob::SetObjectScale  gog=0x855830 (param_bytes=5, notes='undefined4 param_1, undefined1 param_2')
steam: cand=0x00463340(bracket) delta=-0x3f24f0 align=/ size=217 retn=8 gapG/S=224/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, float scale, bool inherit) [param_bytes=8]
callers=0 callees=
```
undefined4 __thiscall FUN_00463340(int param_1,float param_2,char param_3)
{
  if (*(int *)(param_1 + 0x84) == 0) {
    uVar1 = 0;
  }
  else {
    if (param_3 == '\x01') {
      *(float *)(param_1 + 0x1e0) = *(float *)(param_1 + 0x1e0) / *(float *)(param_1 + 0x1e4);
      *(float *)(param_1 + 0x1e4) = param_2;
      *(float *)(param_1 + 0x1e0) = *(float *)(param_1 + 0x1e0) * *(float *)(param_1 + 0x1e4);
    }
    else {
      *(float *)(param_1 + 0x1e0) = param_2 * *(float *)(param_1 + 0x1e4);
    }
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x1ec); local_8 = local_8 + 1) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0x1e8) + local_8 * 4) + 0xcc))
                (*(undefined4 *)(param_1 + 0x1e0),0);
    }
    uVar1 = 1;
  }
  return uVar1;
}
```

### Gob::BehaviorMessage  gog=0x855bc0 (param_bytes=4, notes='Gob * this, char * param_1')
steam: cand=0x004636d0(mapper) delta=-0x3f24f0 align=4/6 size=54 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, char* message) [param_bytes=4]
callers=1 callees=
```
/* Library Function - Single Match
    public: virtual void __thiscall CBaseTabbedPane::SerializeTabWindow(class CArchive &)
   Library: Visual Studio 2010 Debug */
void __thiscall CBaseTabbedPane::SerializeTabWindow(CBaseTabbedPane *this,CArchive *param_1)
{
  if (*(int *)(this + 0x1b4) != 0) {
    (**(code **)(**(int **)(this + 0x1b4) + 0xc))(param_1);
  }
  return;
}
```

### Gob::AttachToScene  gog=0x85f680 (param_bytes=4, notes='Gob * this, undefined4 param_1')
NO CANDIDATE (bracketing failed) -- see manual list

### Gob::RenderBlur  gog=0x89a850 (param_bytes=0, notes='void')
NO CANDIDATE (bracketing failed) -- see manual list

### Gob::Render  gog=0x89aba0 (param_bytes=1, notes='Gob * this, char param_1')
NO CANDIDATE (bracketing failed) -- see manual list

### Gob::EndLookAtAnimate  gog=0x8f6e70 (param_bytes=0, notes='void')
steam: cand=0x004dcce0(mapper) delta=-0x41a190 align=5/6 size=48 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=0 callees=
```
void __fastcall FUN_004dcce0(int param_1)
{
  if (*(int *)(param_1 + 0x1a0) != 0) {
    (**(code **)(**(int **)(param_1 + 0x1a0) + 8))();
  }
  return;
}
```

### Gob::AddAnimationFlag  gog=0x8fd240 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x004e3110(mapper) delta=-0x41a130 align=1/6 size=67 retn=4 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, unsigned long flag) [param_bytes=4]
callers=0 callees=
```
void __thiscall FUN_004e3110(int param_1,uint param_2)
{
  iVar1 = *(int *)(param_1 + 0xe0) + -1;
  if (-1 < iVar1) {
    iVar1 = *(int *)(*(int *)(param_1 + 0xdc) + iVar1 * 4);
    *(uint *)(iVar1 + 8) = *(uint *)(iVar1 + 8) | param_2;
  }
  return;
}
```

### Gob::RemoveAnimationFlag  gog=0x8fd290 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x004e3160(mapper) delta=-0x41a130 align=2/6 size=82 retn=4 gapG/S=96/96 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int flag) [param_bytes=4]
callers=0 callees=
```
void __thiscall FUN_004e3160(int param_1,int param_2)
{
  iVar1 = *(int *)(param_1 + 0xe0) + -1;
  if (((-1 < iVar1) &&
      (iVar1 = *(int *)(*(int *)(param_1 + 0xdc) + iVar1 * 4), *(int *)(iVar1 + 8) != 0)) &&
     (param_2 != 0)) {
    *(int *)(iVar1 + 8) = *(int *)(iVar1 + 8) - param_2;
  }
  return;
}
```

