# Digest G_CSWS

### CSWSDoor::GetDialogResref  gog=0x7623b0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00616f00(mapper) delta=-0x14b4b0 align=4/6 size=49 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, void* outResRef) [param_bytes=4]
callers=0 callees=
```
undefined4 * __thiscall FUN_00616f00(int param_1,undefined4 *param_2)
{
  *param_2 = *(undefined4 *)(param_1 + 0x2e2);
  param_2[1] = *(undefined4 *)(param_1 + 0x2e6);
  param_2[2] = *(undefined4 *)(param_1 + 0x2ea);
  param_2[3] = *(undefined4 *)(param_1 + 0x2ee);
  return param_2;
}
```

### CSWSDoor::GetFirstName  gog=0x7623f0 (param_bytes=0, notes='void')
steam: cand=0x00616f40(mapper) delta=-0x14b4b0 align=5/6 size=19 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSItem::non_virtual_thunk_to_dtor_CSWSItem [low] gog 0x67a210; drift+0x560; retn=0
callers=0 callees=
```
int __fastcall FUN_00616f40(int param_1)
{
  return param_1 + 0x3ec;
}
```

### CSWSDoor::GetIsLinked  gog=0x768410 (param_bytes=0, notes='void')
steam: cand=0x0061d000(mapper) delta=-0x14b410 align=5/6 size=50 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
callers=9 callees=
```
undefined4 __fastcall FUN_0061d000(int param_1)
{
  if ((*(char *)(param_1 + 0x3d4) == '\x01') || (*(char *)(param_1 + 0x3d4) == '\x02')) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### CSWSDoor::RemoveFromArea  gog=0x768450 (param_bytes=0, notes='void')
steam: cand=0x0061d040(mapper) delta=-0x14b410 align=4/6 size=193 retn=0 gapG/S=208/208 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=3 callees=00543f50, 00476620, 0052e4e0, 005453c0=CSWSObject::GetArea, 00545420=CSWSObject::SetArea, 00523090=CSWSArea::RemoveObjectFromArea, 005e8320=Global::RemoveStaticBoundingBox
```
void __fastcall FUN_0061d040(int param_1)
{
  iVar1 = FUN_005453c0();
  if (iVar1 != 0) {
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_00523090(uVar2);
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0();
    iVar1 = FUN_00476620(uVar2);
    if (iVar1 == 1) {
      uVar2 = *(undefined4 *)(param_1 + 4);
      FUN_005453c0();
      FUN_0052e4e0(uVar2);
    }
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_005e8320(uVar2);
  }
  FUN_00545420(0);
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  FUN_00543f50(&local_10,1,1,0);
  return;
}
```

### CSWSDoor::GetLinkedObject  gog=0x768a60 (param_bytes=0, notes='void')
steam: cand=0x0061d620(mapper) delta=-0x14b440 align=4/6 size=179 retn=0 gapG/S=192/192 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
callers=6 callees=0061d000=gog:CSWSDoor::GetIsLinked, 0055eb90, 0051bd90=GetModule (fastcall wrapper of 0x52F4C0), 0051c1a0=gog:CServerExoApp::GetDoorByGameObjectID, 0051c0a0=CServerExoApp::GetGameObject
```
int FUN_0061d620(void)
{
  local_8 = 0;
  iVar1 = FUN_0061d000();
  if (iVar1 != 0) {
    uVar6 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    iVar1 = extraout_var;
    local_8 = FUN_0051c1a0(*(undefined4 *)(extraout_var + 1000));
    if (local_8 == 0) {
      uVar5 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      uVar4 = 0;
      iVar2 = iVar1 + 0x3d8;
      FUN_0051bd90(iVar2,0);
      uVar4 = FUN_0055eb90(iVar2,uVar4);
      *(undefined4 *)(iVar1 + 1000) = uVar4;
      uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      piVar3 = (int *)FUN_0051c0a0(*(undefined4 *)(iVar1 + 1000));
      if (piVar3 != (int *)0x0) {
        local_8 = (**(code **)(*piVar3 + 0x10))(iVar1,uVar4,uVar5,uVar6);
      }
    }
  }
  return local_8;
}
```

### CSWSDoor::MoveToNextOpenState  gog=0x768bf0 (param_bytes=0, notes='void')
steam: cand=0x0061d7b0(mapper) delta=-0x14b440 align=5/6 size=167 retn=0 gapG/S=176/176 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=1 callees=0052ca10=CSWSArea::BudgeCreatures, 005453c0=CSWSObject::GetArea
```
void __fastcall FUN_0061d7b0(int param_1)
{
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

### CSWSObject::GetDialogResref  gog=0x6637e0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x004597d0(mapper) delta=-0x20a010 align=1/6 size=25 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, void* outResRef) [param_bytes=4]
name_map: CSWSObject::GetDialogResref [medium] gog 0x6637e0; vote; retn=4
callers=0 callees=
```
void __thiscall FUN_004597d0(int param_1,undefined4 param_2)
{
  *(undefined4 *)(param_1 + 0x1d8) = param_2;
  return;
}
```

### CSWSObject::GetPortrait  gog=0x663840 (param_bytes=4, notes='CSWSObject * this, CResRef * param_1')
steam: cand=0x00459830(mapper) delta=-0x20a010 align=3/6 size=25 retn=4 gapG/S=48/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, void* outResRef) [param_bytes=4]
name_map: CSWSObject::GetPortrait [medium] gog 0x663840; vote; retn=4
callers=0 callees=
```
void __thiscall FUN_00459830(int param_1,undefined1 param_2)
{
  *(undefined1 *)(param_1 + 0x1a9) = param_2;
  return;
}
```

### CSWSObject::SetPortrait  gog=0x663870 (param_bytes=4, notes='undefined4 param_1')
NO CANDIDATE (bracketing failed) -- see manual list

### CSWSObject::GetPortraitId  gog=0x6638a0 (param_bytes=0, notes='void')
steam: cand=0x00459890(mapper) delta=-0x20a010 align=2/6 size=20 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: WORD (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSObject::GetPortraitId [medium] gog 0x6638a0; vote; retn=0
callers=0 callees=
```
undefined1 __fastcall FUN_00459890(int param_1)
{
  return *(undefined1 *)(param_1 + 0x1aa);
}
```

### CSWSObject::GetLastName  gog=0x6a5ce0 (param_bytes=0, notes='CSWSObject * this')
steam: cand=0x0053f000(mapper) delta=-0x166ce0 align=3/6 size=18 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSObject::GetLastName [medium] gog 0x6a5ce0; vote; retn=0
callers=0 callees=
```
undefined2 __fastcall FUN_0053f000(int param_1)
{
  return *(undefined2 *)(param_1 + 0x30);
}
```

### CSWSObject::SetPortraitId  gog=0x6a7820 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x00540b40(bracket) delta=-0x166ce0 align=/ size=339 retn=4 gapG/S=352/ cc_guess=
wrapper: void (__thiscall) (void* thisPtr, WORD id) [param_bytes=4]
callers=0 callees=00733df0=CExoString::operator+, 00734010=CExoString::CStr, 00733570=CExoString::CExoString_char, 0071b1d0, 007337c0=CExoString::operator=, 00710840, 00733780=CExoString::~CExoString, 00733540=CExoString::CExoString(empty)
strings: BaseResRef
```
void __thiscall FUN_00540b40(int *param_1,ushort param_2)
{
  undefined1 local_50 [16];
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [16];
  undefined1 local_20 [16];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f7f0;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570(&DAT_00994684);
  local_8 = 0;
  if (param_2 < 0xfffe) {
    FUN_00733540(uVar1);
    local_8._0_1_ = 1;
    FUN_00733570("BaseResRef");
    local_8._0_1_ = 2;
    FUN_0071b1d0(param_2,local_38,local_20);
    local_8._0_1_ = 1;
    FUN_00733780();
    uVar2 = FUN_00733df0(local_40,local_20);
    local_8._0_1_ = 3;
    FUN_007337c0(uVar2);
    local_8._0_1_ = 1;
    FUN_00733780();
    uVar2 = FUN_00734010();
    FUN_00710840(uVar2);
    (**(code **)(*param_1 + 0xcc))(local_50);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
  }
  else {
    FUN_00710840(&DAT_00986925);
    (**(code **)(*param_1 + 0xcc))(local_30);
  }
  *(ushort *)(param_1 + 0xc) = param_2;
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### CSWSObject::ClearAllActions  gog=0x6a7d60 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00541080(mapper) delta=-0x166ce0 align=5/6 size=779 retn=4 gapG/S=784/784 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int includeAttacks) [param_bytes=4]
name_map: CSWSObject::ClearAllActions [medium] gog 0x6a7d60; vote; retn=4
callers=49 callees=007e6be0, 0073fec0=CClientExoApp::ServerToClientObjectId, 0064dde0, 0053e740=Action-list free params, 00734010=CExoString::CStr, 00561430, 005908f0=CSWSCombatRound::ClearAllAttacks, 0073fb90=ClientApp -> party (*(*(clientApp+4)+0x270)) thisc, 0058c370, 006f9c90=CPathfindInformation::ResetRequest, 009196fd=operator delete, 00521060, 007a1720
```
void __thiscall FUN_00541080(int *param_1,int param_2)
{
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
          FUN_009196fd(iVar1);
        }
      }
      iVar1 = (**(code **)(*param_1 + 0x30))();
      if (iVar1 != 0) {
        (**(code **)(*param_1 + 0x30))();
        FUN_006f9c90();
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
... (12 more lines)
```

### CSWSObject::SetAnimation  gog=0x6a8070 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00541390(mapper) delta=-0x166ce0 align=5/6 size=35 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int animation) [param_bytes=4]
name_map: CSWSObject::SetAnimation [medium] gog 0x6a8070; vote; retn=4
callers=1 callees=0054a430=CSWSObject::SetDirtyFlag
```
void __thiscall FUN_00541390(int param_1,undefined4 param_2)
{
  FUN_0054a430(0);
  *(undefined4 *)(param_1 + 0xd8) = param_2;
  return;
}
```

### CSWSObject::SetDialogDelay  gog=0x6ac150 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00545470(mapper) delta=-0x166ce0 align=6/6 size=162 retn=4 gapG/S=176/176 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, float delay) [param_bytes=4]
name_map: CSWSObject::SetDialogDelay [medium] gog 0x6ac150; vote; retn=4
callers=3 callees=0051c370=gog:CServerExoApp::GetWorldTimer, 0051ad20=gog:CWorldTimer::GetWorldTime, 0051aef0=gog:CWorldTimer::AddWorldTimes
```
void __thiscall FUN_00545470(int param_1,float param_2)
{
  iVar2 = param_1 + 0x44;
  iVar1 = param_1 + 0x40;
  FUN_0051c370(iVar1,iVar2);
  FUN_0051ad20(iVar1,iVar2);
  iVar1 = param_1 + 0x3c;
  iVar2 = param_1 + 0x38;
  uVar6 = (undefined4)(longlong)ROUND((float)DAT_00992384 * param_2);
  uVar5 = 0;
  uVar4 = *(undefined4 *)(param_1 + 0x44);
  uVar3 = *(undefined4 *)(param_1 + 0x40);
  FUN_0051c370(uVar3,uVar4,0,uVar6,iVar2,iVar1,(longlong)ROUND((float)DAT_00992384 * param_2),
               in_FPUControlWord | 0xc00);
  FUN_0051aef0(uVar3,uVar4,uVar5,uVar6,iVar2,iVar1);
  return;
}
```

### CSWSObject::StopDialog  gog=0x6ad5e0 (param_bytes=0, notes='void')
steam: cand=0x00546900(mapper) delta=-0x166ce0 align=6/6 size=329 retn=0 gapG/S=336/336 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSObject::StopDialog [medium] gog 0x6ad5e0; vote; retn=0
callers=4 callees=006cf740, 0051c080=gog:CServerExoApp::GetObjectArray, 0057d370=CSWSCreature::SetLockOrientationToObject, 0053dfb0=CGameObjectArray::GetGameObject, 00546b00=CSWSObject::RemoveObjectFromDialog_2
```
undefined4 __fastcall FUN_00546900(int *param_1)
{
  undefined1 local_8 [4];
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 != 0) {
    uVar4 = 0;
    uVar3 = DAT_00994484;
    (**(code **)(*param_1 + 0x30))(DAT_00994484,0);
    FUN_0057d370(uVar3,uVar4);
  }
  if (param_1[0x15] == 0x7f000000) {
    uVar3 = 0;
  }
  else if (param_1[0x15] == param_1[1]) {
    if (param_1[0xd] == 0) {
      uVar3 = 0;
    }
    else {
      FUN_006cf740(param_1,0);
      param_1[0xe] = 0;
      param_1[0xf] = 0;
      param_1[0x12] = 0;
      param_1[0x14] = 0;
      if ((undefined4 *)param_1[0xd] != (undefined4 *)0x0) {
        (*(code *)**(undefined4 **)param_1[0xd])(1);
      }
      param_1[0xd] = 0;
      uVar3 = 1;
    }
  }
  else {
    puVar5 = local_8;
    iVar2 = param_1[0x15];
    FUN_0051c080(iVar2,puVar5);
    cVar1 = FUN_0053dfb0(iVar2,puVar5);
    if (cVar1 == DAT_0099447f) {
      FUN_00546b00(param_1[1]);
      param_1[0x15] = 0x7f000000;
    }
    uVar3 = 1;
  }
  return uVar3;
}
```

### CSWSObject::StopSoundPlayingInDialog  gog=0x6ad750 (param_bytes=0, notes='void')
steam: cand=0x00546a70(mapper) delta=-0x166ce0 align=6/6 size=46 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSObject::StopSoundPlayingInDialog [medium] gog 0x6ad750; vote; retn=0
callers=2 callees=007402b0=CClientExoApp::StopSoundPlayingInDialog
```
void __fastcall FUN_00546a70(int param_1)
{
  *(undefined4 *)(param_1 + 0x25c) = 1;
  FUN_007402b0(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  return;
}
```

### CSWSObject::GetNearestObjectByName  gog=0x6ad8b0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00546bd0(mapper) delta=-0x166ce0 align=6/6 size=468 retn=8 gapG/S=480/480 cc_guess=thiscall
wrapper: DWORD (__thiscall) (void* thisPtr, void* name, float radius) [param_bytes=8]
name_map: CSWSObject::GetNearestObjectByName [medium] gog 0x6ad8b0; vote; retn=8
callers=6 callees=005453c0=CSWSObject::GetArea, 0051c080=gog:CServerExoApp::GetObjectArray, 004452b0=Vector::operator- (Vector3 subtract), 00733a60=CExoString::operator==_2, 00734d60=CExoString::CompareNoCase, 0091cb24, 0053dfb0=CGameObjectArray::GetGameObject, 005232b0=CSWSArea::GetFirstObjectInArea, 00523250=CSWSArea::GetNextObjectInArea, 0044b8a0
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
undefined4 __thiscall FUN_00546bd0(int param_1,undefined4 param_2,float param_3)
{
  float10 fVar3;
  undefined1 local_4c [12];
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
      cVar1 = FUN_0053dfb0(uVar4);
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
... (6 more lines)
```

### CSWSObject::SetDialogOwner  gog=0x6ada90 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00546db0(mapper) delta=-0x166ce0 align=6/6 size=22 retn=4 gapG/S=144/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* owner) [param_bytes=4]
name_map: CSWSObject::SetDialogOwner [medium] gog 0x6ada90; vote; retn=4
callers=3 callees=
```
void __thiscall FUN_00546db0(int param_1,undefined4 param_2)
{
  *(undefined4 *)(param_1 + 0x54) = param_2;
  return;
}
```

### CSWSObject::GetScriptLocation  gog=0x6ae2e0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00547600(mapper) delta=-0x166ce0 align=6/6 size=188 retn=4 gapG/S=192/192 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, void* outLocation) [param_bytes=4]
name_map: CSWSObject::GetScriptLocation [medium] gog 0x6ae2e0; vote; retn=4
callers=1 callees=00623320, 0053e060
```
undefined4 * __thiscall FUN_00547600(int param_1,undefined4 *param_2)
{
  void *local_10;
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

### CSWSObject::HasSpellEffectApplied  gog=0x6ae3a0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x005476c0(mapper) delta=-0x166ce0 align=5/6 size=101 retn=4 gapG/S=112/112 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, int spellId) [param_bytes=4]
name_map: CSWSObject::HasSpellEffectApplied [medium] gog 0x6ae3a0; vote; retn=4
callers=2 callees=
```
undefined4 __thiscall FUN_005476c0(int param_1,int param_2)
{
  if (param_2 != -1) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x14c); local_8 = local_8 + 1) {
      if (*(int *)(*(int *)(*(int *)(param_1 + 0x148) + local_8 * 4) + 0x1c) == param_2) {
        return 1;
      }
    }
  }
  return 0;
}
```

### CSWSObject::GetDead  gog=0x6ae4d0 (param_bytes=0, notes='void')
steam: cand=0x005477f0(mapper) delta=-0x166ce0 align=3/6 size=202 retn=0 gapG/S=208/208 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSObject::GetDead [medium] gog 0x6ae4d0; vote; retn=0
callers=2 callees=
```
bool __fastcall FUN_005477f0(int *param_1)
{
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if ((iVar2 != 0) && (iVar2 = (**(code **)(*param_1 + 0x30))(), *(int *)(iVar2 + 0x10e8) != 0)) {
    piVar3 = (int *)(**(code **)(*param_1 + 0x30))();
    sVar1 = (**(code **)(*piVar3 + 0x9c))(0);
    return sVar1 < -9;
  }
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 == 0) {
    sVar1 = (**(code **)(*param_1 + 0x9c))(0);
  }
  else {
    piVar3 = (int *)(**(code **)(*param_1 + 0x30))();
    sVar1 = (**(code **)(*piVar3 + 0x9c))(0);
  }
  return sVar1 < 1;
}
```

### CSWSObject::SpawnBodyBag  gog=0x6b0460 (param_bytes=0, notes='void')
steam: cand=0x00549780(mapper) delta=-0x166ce0 align=2/6 size=1861 retn=0 gapG/S=1872/1872 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSObject::SpawnBodyBag [medium] gog 0x6b0460; vote; retn=0
callers=2 callees=00919723=operator new, 0051e170=CServerAIMaster::AddEventDeltaTime(dayD,msD,caller, 00735360=CExoLocString::Destructor, 00515620, 0071bcf0=C2DA::GetINTEntry_2, 00733570=CExoString::CExoString_char, 00515d50, 00735240, 005f5160=CSWSPlaceable::AcquireItemsFromObject, 00588d50=CSWSCreature::GetBodyBagAppearance, 005812e0=CSWSCreature::GetGold, 005f0f10=CSWSPlaceable::LoadBodyBag, 00735400, 00515dd0 ...
strings: Corpse | BODY_BAG
```
undefined4 __fastcall FUN_00549780(int *param_1)
{
  float10 fVar3;
  undefined1 local_a8 [16];
  undefined1 local_98 [8];
  undefined1 local_8c [8];
  undefined1 local_60 [8];
  undefined1 local_54 [11];
  undefined1 local_34 [4];
  void *local_10;
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
          if (local_40 != 0) {
            iVar2 = FUN_005f67b0(0);
            if (iVar2 == 0) {
              ExceptionList = local_10;
              return 0x7f000000;
            }
            local_20 = FUN_005f6630();
          }
        }
        else {
          local_20 = FUN_00588d50();
          local_44 = (uint)*(byte *)(local_24 + 0x10ee);
          if (local_44 == 0) {
            FUN_00733570("BODY_BAG");
... (143 more lines)
```

### CSWSObject::GetHasFeatEffectApplied  gog=0x6b0bb0 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x00549ed0(bracket) delta=-0x166ce0 align=/ size=151 retn=4 gapG/S=160/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD feat) [param_bytes=4]
callers=1 callees=006a20f0
```
undefined4 __thiscall FUN_00549ed0(int param_1,undefined2 param_2)
{
  iVar1 = FUN_006a20f0(param_2);
  if ((iVar1 != 0) && ((*(uint *)(iVar1 + 4) & 0xffff) != 0xffffffff)) {
    for (local_10 = 0; local_10 < *(int *)(param_1 + 0x14c); local_10 = local_10 + 1) {
      if (*(uint *)(*(int *)(*(int *)(param_1 + 0x148) + local_10 * 4) + 0x1c) ==
          (*(uint *)(iVar1 + 4) & 0xffff)) {
        return 1;
      }
    }
  }
  return 0;
}
```

### CSWSObject::SetCurrentHitPoints  gog=0x6b12f0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0054a610(mapper) delta=-0x166ce0 align=6/6 size=70 retn=4 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int currentHP) [param_bytes=4]
name_map: CSWSObject::SetCurrentHitPoints [medium] gog 0x6b12f0; vote; retn=4
callers=21 callees=
```
void __thiscall FUN_0054a610(int param_1,undefined4 param_2)
{
  *(undefined4 *)(param_1 + 0xe0) = param_2;
  if ((*(int *)(param_1 + 0x22c) != 0) && (*(int *)(param_1 + 0xe0) < 1)) {
    *(undefined4 *)(param_1 + 0xe0) = 1;
  }
  return;
}
```

### CSWSObject::SetKeepCorpse  gog=0x6b1480 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0054a7a0(mapper) delta=-0x166ce0 align=6/6 size=153 retn=4 gapG/S=160/160 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int keepCorpse) [param_bytes=4]
name_map: CSWSObject::SetKeepCorpse [medium] gog 0x6b1480; vote; retn=4
callers=1 callees=0058c370, 0051c600=CServerExoApp::GetPlayerByControlledObjectId, 0051c8f0=gog:CServerExoApp::GetPlayerCreatureId, 007a1720, 00561430
```
void __thiscall FUN_0054a7a0(int param_1,undefined4 param_2)
{
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

### CSWSObject::GetClientObject  gog=0x6b1520 (param_bytes=0, notes='void')
steam: cand=0x0054a840(mapper) delta=-0x166ce0 align=6/6 size=153 retn=0 gapG/S=160/160 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSObject::GetClientObject [medium] gog 0x6b1520; vote; retn=0
callers=10 callees=0073fec0=CClientExoApp::ServerToClientObjectId, 007f2610=CSWCObject::AttachmentFromServerObject, 0073f4d0=ClientApp::GetClientObject
```
undefined4 __fastcall FUN_0054a840(int param_1)
{
  if (*(int *)(param_1 + 0x264) == 0) {
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    uVar1 = FUN_0073fec0(*(undefined4 *)(param_1 + 4));
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    piVar2 = (int *)FUN_0073f4d0(uVar1);
    if (piVar2 != (int *)0x0) {
      uVar3 = (**(code **)(*piVar2 + 0xc))(param_1,uVar4,uVar3);
      *(undefined4 *)(param_1 + 0x264) = uVar3;
      if (*(int *)(param_1 + 0x264) != 0) {
        FUN_007f2610(param_1);
      }
    }
  }
  return *(undefined4 *)(param_1 + 0x264);
}
```

### CSWSObject::GetAcceptableAction  gog=0x6b3380 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x005d7190(mapper) delta=-0xdc1f0 align=4/6 size=62 retn=4 gapG/S=144/144 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD action) [param_bytes=4]
name_map: CSWSObject::GetAcceptableAction [medium] gog 0x6b3380; vote; retn=4
callers=2 callees=
```
undefined4 FUN_005d7190(undefined4 param_1)
{
  switch(param_1) {
  case 1:
  case 7:
  case 9:
  case 0xc:
  case 0xf:
  case 0x14:
  case 0x15:
  case 0x18:
  case 0x19:
  case 0x1a:
  case 0x1b:
  case 0x1c:
  case 0x1d:
  case 0x1e:
  case 0x26:
  case 0x27:
  case 0x28:
  case 0x29:
  case 0x2a:
  case 0x2b:
  case 0x2e:
  case 0x32:
  case 0x36:
  case 0x37:
  case 0x38:
  case 0x3d:
  case 0x3f:
  case 0x43:
  case 0x44:
  case 0x45:
  case 0x46:
  case 0x47:
    uVar1 = 1;
    break;
  default:
    uVar1 = 0;
  }
  return uVar1;
}
```

### CSWSObject::GetAIStateReputation  gog=0x6bd870 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x005e1680(mapper) delta=-0xdc1f0 align=3/6 size=159 retn=4 gapG/S=352/160 cc_guess=thiscall
wrapper: BYTE (__thiscall) (void* thisPtr, DWORD objectId) [param_bytes=4]
name_map: CSWSObject::GetAIStateReputation [medium] gog 0x6bd870; vote; retn=4
callers=12 callees=0051c080=gog:CServerExoApp::GetObjectArray, 005453c0=CSWSObject::GetArea, 0053dfb0=CGameObjectArray::GetGameObject, 005e13c0=CSWSObject::GetReputation
```
undefined4 FUN_005e1680(undefined4 param_1)
{
  local_8 = 0;
  iVar8 = 0x32;
  uVar1 = FUN_005453c0();
  uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  ppiVar5 = &local_c;
  iVar7 = extraout_var;
  FUN_0051c080(param_1,ppiVar5);
  FUN_0053dfb0(param_1,ppiVar5);
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

### CSWSPlaceable::SetEffectSpellId  gog=0x4213a0 (param_bytes=4, notes='undefined4 param_1')
NO CANDIDATE (bracketing failed) -- see manual list

### CSWSPlaceable::GetDialogResref  gog=0x769510 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x005ef120(mapper) delta=-0x17a3f0 align=3/6 size=49 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, void* outResref) [param_bytes=4]
name_map: CSWSPlaceable::GetDialogResref [low] gog 0x769510; drift+0x0; retn=4
callers=0 callees=
```
undefined4 * __thiscall FUN_005ef120(int param_1,undefined4 *param_2)
{
  *param_2 = *(undefined4 *)(param_1 + 0x280);
  param_2[1] = *(undefined4 *)(param_1 + 0x284);
  param_2[2] = *(undefined4 *)(param_1 + 0x288);
  param_2[3] = *(undefined4 *)(param_1 + 0x28c);
  return param_2;
}
```

### CSWSPlaceable::GetEffectSpellId  gog=0x769550 (param_bytes=0, notes='void')
steam: cand=0x005ef160(mapper) delta=-0x17a3f0 align=3/6 size=20 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: DWORD (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSPlaceable::GetEffectSpellId [low] gog 0x769550; drift+0x0; retn=0
callers=0 callees=
```
undefined4 __fastcall FUN_005ef160(int param_1)
{
  return *(undefined4 *)(param_1 + 0x3e0);
}
```

### CSWSPlaceable::RemoveFromArea  gog=0x76d320 (param_bytes=0, notes='void')
steam: cand=0x005f2f50(mapper) delta=-0x17a3d0 align=5/6 size=143 retn=0 gapG/S=144/144 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSPlaceable::RemoveFromArea [medium] gog 0x76d320; vote; retn=0
callers=3 callees=00543f50, 005453c0=CSWSObject::GetArea, 00545420=CSWSObject::SetArea, 005e8320=Global::RemoveStaticBoundingBox, 00523090=CSWSArea::RemoveObjectFromArea
```
void __fastcall FUN_005f2f50(int param_1)
{
  iVar1 = FUN_005453c0();
  if (iVar1 != 0) {
    if (-1 < *(int *)(param_1 + 0x39c)) {
      uVar2 = *(undefined4 *)(param_1 + 4);
      FUN_005453c0(uVar2);
      FUN_005e8320(uVar2);
      *(undefined4 *)(param_1 + 0x39c) = 0xffffffff;
    }
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_00523090(uVar2);
  }
  FUN_00545420(0);
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  FUN_00543f50(&local_10,1,1,0);
  return;
}
```

### CSWSPlaceable::SetLightIsOn  gog=0x770810 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x005f6470(mapper) delta=-0x17a3a0 align=5/6 size=251 retn=4 gapG/S=256/256 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int lightState) [param_bytes=4]
name_map: CSWSPlaceable::SetLightIsOn [medium] gog 0x770810; vote; retn=4
callers=2 callees=0071bcf0=C2DA::GetINTEntry_2, 00733570=CExoString::CExoString_char, 00733780=CExoString::~CExoString
strings: LightColor
```
void __thiscall FUN_005f6470(int param_1,int param_2)
{
  undefined1 local_20 [11];
  undefined1 local_14 [4];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954699;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  bVar1 = false;
  if (*(int *)(param_1 + 0x3e4) != param_2) {
    FUN_00733570("LightColor");
    local_8 = 0;
    bVar1 = true;
    iVar3 = FUN_0071bcf0(*(undefined4 *)(param_1 + 0x270),local_20,local_14);
    if (iVar3 != 0) {
      local_38 = '\0';
      goto LAB_005f6516;
    }
  }
  local_38 = '\x01';
LAB_005f6516:
  local_15 = local_38;
  local_8 = 0xffffffff;
  if (bVar1) {
    FUN_00733780(uVar2);
  }
  if (local_15 == '\0') {
    *(undefined4 *)(param_1 + 1000) = 1;
    *(int *)(param_1 + 0x3e4) = param_2;
  }
  ExceptionList = local_10;
  return;
}
```

### CSWSPlaceable::GetLightIsOn  gog=0x770910 (param_bytes=0, notes='void')
steam: cand=0x005f6570(mapper) delta=-0x17a3a0 align=5/6 size=177 retn=0 gapG/S=192/192 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSPlaceable::GetLightIsOn [medium] gog 0x770910; vote; retn=0
callers=4 callees=0071bcf0=C2DA::GetINTEntry_2, 00733570=CExoString::CExoString_char, 00733780=CExoString::~CExoString
strings: LightColor
```
undefined4 FUN_005f6570(void)
{
  undefined1 local_20 [11];
  undefined1 local_14 [4];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_009546c8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("LightColor");
  local_8 = 0;
  uVar3 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x30);
  iVar4 = extraout_var;
  iVar2 = FUN_0071bcf0(*(undefined4 *)(extraout_var + 0x270),local_20,local_14);
  local_15 = '\x01' - (iVar2 != 0);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,iVar4,uVar3);
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

### CSWSPlaceable::GetBodyBagAppearance  gog=0x7709d0 (param_bytes=0, notes='void')
steam: cand=0x005f6630(mapper) delta=-0x17a3a0 align=6/6 size=384 retn=0 gapG/S=384/384 cc_guess=thiscall
wrapper: BYTE (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSPlaceable::GetBodyBagAppearance [medium] gog 0x7709d0; vote; retn=0
callers=1 callees=0071bcf0=C2DA::GetINTEntry_2, 00733570=CExoString::CExoString_char, 00733780=CExoString::~CExoString
strings: Appearance | BodyBag
```
uint FUN_005f6630(void)
{
  undefined1 local_3c [11];
  undefined1 local_30 [11];
  undefined1 local_24 [11];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954708;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Appearance");
  local_8 = 0;
  iVar4 = extraout_var;
  iVar2 = FUN_0071bcf0(*(undefined1 *)(extraout_var + 0x3ec),local_24,&local_14);
  local_19 = '\x01' - (iVar2 != 0);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  if (local_19 != '\0') {
    FUN_00733570("BodyBag");
    local_8 = 1;
    uVar6 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x10);
    iVar2 = FUN_0071bcf0(*(undefined4 *)(iVar4 + 0x270),local_30,&local_18);
    local_25 = '\x01' - (iVar2 != 0);
    local_8 = 0xffffffff;
    FUN_00733780();
    if (local_25 != '\0') {
      local_18 = 0;
    }
    FUN_00733570("Appearance");
    local_8 = 2;
    iVar2 = *(int *)(DAT_00a1b4d0 + 0x130);
    uVar5 = *(undefined4 *)(iVar2 + 0xd0);
    iVar3 = FUN_0071bcf0(local_18,local_3c,&local_14);
    local_31 = '\x01' - (iVar3 != 0);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1,iVar4,uVar5,iVar2,uVar6);
    if (local_31 != '\0') {
      local_14 = 3;
    }
  }
  ExceptionList = local_10;
  return local_14 & 0xff;
}
```

### CSWSPlaceable::SetOrientation  gog=0x770d30 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x005f6990(mapper) delta=-0x17a3a0 align=3/6 size=111 retn=4 gapG/S=112/112 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* orientation) [param_bytes=4]
name_map: CSWSPlaceable::SetOrientation [medium] gog 0x770d30; vote; retn=4
callers=3 callees=005143c0, 00543f10=CSWSObject::SetOrientation, 0046ce40
```
void __thiscall FUN_005f6990(int param_1,undefined4 *param_2)
{
  undefined1 local_1c [12];
  *(undefined4 *)(param_1 + 0x480) = *param_2;
  *(undefined4 *)(param_1 + 0x484) = param_2[1];
  *(undefined4 *)(param_1 + 0x488) = param_2[2];
  *(undefined4 *)(param_1 + 0x48c) = param_2[3];
  FUN_0046ce40(&local_10);
  local_8 = 0;
  puVar1 = (undefined4 *)FUN_005143c0(local_1c,&local_10);
  local_10 = *puVar1;
  local_c = puVar1[1];
  local_8 = puVar1[2];
  FUN_00543f10(&local_10);
  return;
}
```

### CSWSPlaceable::GetDead  gog=0x770da0 (param_bytes=0, notes='void')
NO CANDIDATE (bracketing failed) -- see manual list

### CSWSPlaceable::GetClientPlaceable  gog=0x770e40 (param_bytes=0, notes='void')
steam: cand=0x005f6a70(mapper) delta=-0x17a3d0 align=1/6 size=53 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWSPlaceable::GetClientPlaceable [medium] gog 0x770e40; vote; retn=0
callers=1 callees=0054a840=CSWSObject::GetClientObject
```
undefined4 FUN_005f6a70(void)
{
  iVar1 = FUN_0054a840();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_0054a840();
    uVar3 = (**(code **)(*piVar2 + 0x4c))(piVar2,uVar3);
  }
  return uVar3;
}
```

### CSWSTrigger::GetTargetArea  gog=0x75d7d0 (param_bytes=0, notes='void')
steam: cand=0x0061e540(mapper) delta=-0x13f290 align=6/6 size=170 retn=0 gapG/S=176/176 cc_guess=thiscall
wrapper: DWORD (__thiscall) (void* thisPtr) [param_bytes=0]
callers=2 callees=00557ec0, 0051c080=gog:CServerExoApp::GetObjectArray, 0051bd90=GetModule (fastcall wrapper of 0x52F4C0), 005453c0=CSWSObject::GetArea, 0053dfb0=CGameObjectArray::GetGameObject
```
undefined4 __fastcall FUN_0061e540(int param_1)
{
  if (*(int *)(param_1 + 0x2f4) == 0) {
    uVar2 = 0x7f000000;
  }
  else {
    param_1 = param_1 + 0x270;
    FUN_0051bd90(param_1);
    uVar2 = FUN_00557ec0(param_1);
    ppiVar4 = &local_c;
    local_8 = uVar2;
    FUN_0051c080(uVar2,ppiVar4);
    cVar1 = FUN_0053dfb0(uVar2,ppiVar4);
    if (cVar1 == DAT_0099447f) {
      (**(code **)(*local_c + 0x58))();
      iVar3 = FUN_005453c0();
      uVar2 = *(undefined4 *)(iVar3 + 0x124);
    }
    else {
      uVar2 = 0x7f000000;
    }
  }
  return uVar2;
}
```

### CSWSTrigger::InTrigger  gog=0x75eef0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0061fc60(mapper) delta=-0x13f290 align=6/6 size=1209 retn=4 gapG/S=1216/1216 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, Vector* point) [param_bytes=4]
callers=5 callees=0044b8a0, 0091ebad, 004452b0=Vector::operator- (Vector3 subtract)
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
bool __thiscall FUN_0061fc60(int param_1,float *param_2)
{
  float10 fVar6;
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
... (42 more lines)
```

### CSWSTrigger::RemoveFromArea  gog=0x761900 (param_bytes=0, notes='void')
steam: cand=0x00622670(mapper) delta=-0x13f290 align=5/6 size=213 retn=0 gapG/S=224/224 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=5 callees=00543f50, 00476620, 0052e4e0, 005453c0=CSWSObject::GetArea, 00545420=CSWSObject::SetArea, 00523090=CSWSArea::RemoveObjectFromArea
```
void __fastcall FUN_00622670(int param_1)
{
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

### CSWSTrigger::GetCanFireMineOnObject  gog=0x7619e0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00622750(mapper) delta=-0x13f290 align=5/6 size=469 retn=8 gapG/S=480/480 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD objectId, int skipFactionCheck) [param_bytes=8]
callers=1 callees=006b83f0=CSWSCreatureStats::HasFeat, 0077d800=Party entry -> creature thiscall(entry); creature+, 0051c100=CServerExoApp::GetCreatureByGameObjectID, 007e5da0=Party member entry thiscall(party, int idx) RET 4 , 006bc570, 0073fb90=ClientApp -> party (*(*(clientApp+4)+0x270)) thisc, 005e13c0=CSWSObject::GetReputation, 005d0cb0=CSWSCreature::GetFaction
```
undefined4 __thiscall FUN_00622750(int param_1,undefined4 param_2,int param_3)
{
  local_c = 1;
  iVar1 = FUN_0051c100(*(undefined4 *)(param_1 + 0x328));
  iVar2 = FUN_0051c100(param_2);
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
      FUN_0073fb90(0);
      iVar1 = FUN_007e5da0(uVar4);
      if (((iVar1 != 0) && (iVar1 = FUN_0077d800(), iVar1 != 0)) &&
         (iVar1 = FUN_006b83f0(0xef), iVar1 != 0)) {
        local_c = 0;
      }
    }
  }
  return local_c;
}
```

### CSWSTrigger::OnEnterMine  gog=0x761bc0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00622930(mapper) delta=-0x13f290 align=6/6 size=660 retn=4 gapG/S=672/672 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int skipFactionCheck) [param_bytes=4]
callers=1 callees=0051c280=gog:CServerExoApp::GetServerAIMaster, 00605e80=CItemRepository::RemoveItem, 006d0620, 006fd8d0=CVirtualMachine::RunScript(CExoString*, objid, int, 0051e170=CServerAIMaster::AddEventDeltaTime(dayD,msD,caller, 00584da0=CSWSCreature::SendFeedbackMessage, 007339d0=CExoString::operator==, 0051c0e0=gog:CServerExoApp::GetItemByGameObjectID, 00622750=gog:CSWSTrigger::GetCanFireMineOnObject, 0051c100=CServerExoApp::GetCreatureByGameObjectID, 00622640=gog:CSWSTrigger::GetScriptName, 0058a2c0=CSWSCreature::GetItemRepository, 006d0880, 00605cb0=CItemRepository::FindItemWithTag
```
void __thiscall FUN_00622930(int param_1,undefined4 param_2)
{
  iVar1 = FUN_0051c100(*(undefined4 *)(param_1 + 0x2dc));
  if ((iVar1 != 0) && (iVar1 = FUN_00622750(*(undefined4 *)(iVar1 + 4),param_2), iVar1 != 0)) {
    iVar1 = param_1 + 700;
    FUN_0058a2c0(1);
    local_10 = FUN_00605cb0(iVar1);
    if (local_10 == 0x7f000000) {
      for (local_14 = 0; local_14 < 0x14; local_14 = local_14 + 1) {
        iVar1 = FUN_006d0620(1 << ((byte)local_14 & 0x1f));
        if ((iVar1 != 0) && (iVar2 = FUN_007339d0(param_1 + 700), iVar2 != 0)) {
          local_10 = *(int *)(iVar1 + 0x14);
        }
      }
    }
    puVar3 = (undefined4 *)FUN_0051c0e0(local_10);
    if (puVar3 == (undefined4 *)0x0) {
      FUN_00584da0(0x52,0);
      uVar9 = 1;
      uVar7 = *(undefined4 *)(param_1 + 4);
      uVar4 = FUN_00622640(4);
      FUN_006fd8d0(uVar4,uVar7,uVar9);
      uVar9 = 1;
      uVar7 = *(undefined4 *)(param_1 + 4);
      uVar4 = FUN_00622640(1);
      FUN_006fd8d0(uVar4,uVar7,uVar9);
      if (*(int *)(param_1 + 0x310) != 0) {
        uVar7 = *(undefined4 *)(param_1 + 4);
        uVar4 = *(undefined4 *)(param_1 + 4);
        uVar10 = 0;
        uVar6 = 0;
        uVar5 = 0;
        uVar9 = DAT_00992420;
        FUN_0051c280(0,0,uVar4,uVar7,DAT_00992420,0);
        FUN_0051e170(uVar5,uVar6,uVar4,uVar7,uVar9,uVar10);
      }
    }
    else {
      if (*(int *)(param_1 + 0x30c) != 0) {
        puVar8 = puVar3;
        FUN_0058a2c0(1);
        FUN_00605e80(puVar8);
        FUN_006d0880(puVar3);
        if (puVar3 != (undefined4 *)0x0) {
          (**(code **)*puVar3)(1);
... (19 more lines)
```

### CSWSTrigger::SetCreator  gog=0x761ea0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00622c10(mapper) delta=-0x13f290 align=4/6 size=124 retn=4 gapG/S=128/128 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, DWORD creatorId) [param_bytes=4]
callers=3 callees=0051c8d0=CServerExoApp::GetPartyTable (inline at *(this+4)+, 0051c8f0=gog:CServerExoApp::GetPlayerCreatureId, 005fb070=CSWPartyTable::GetIsNPCObjectMember(objId) (PC -> 
```
void __thiscall FUN_00622c10(int param_1,int param_2)
{
  *(int *)(param_1 + 0x328) = param_2;
  if (param_2 == 0x7f000000) {
LAB_00622c79:
    *(undefined4 *)(param_1 + 0x354) = 0;
  }
  else {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    iVar1 = FUN_0051c8f0();
    if (param_2 != iVar1) {
      FUN_0051c8d0(param_2,param_1,*(undefined4 *)(DAT_00a1b4a4 + 8),uVar2);
      iVar1 = FUN_005fb070(param_2);
      if (iVar1 == 0) goto LAB_00622c79;
    }
    *(undefined4 *)(param_1 + 0x354) = 1;
  }
  return;
}
```

