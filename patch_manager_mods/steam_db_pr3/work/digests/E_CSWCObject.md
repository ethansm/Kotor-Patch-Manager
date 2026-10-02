# Digest E_CSWCObject

### CSWCDoor::SetIsAreaTransition  gog=0x507260 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0087b980(mapper) delta=0x374720 align=2/6 size=86 retn=4 gapG/S=96/96 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int isTransition) [param_bytes=4]
callers=1 callees=
strings: trans
```
void __thiscall FUN_0087b980(int *param_1,int param_2)
{
  if (param_2 == 0) {
    piVar1 = (int *)(**(code **)(*param_1 + 0x98))(0xff,1);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0xb0))("trans");
    }
  }
  param_1[0x4c] = param_2;
  return;
}
```

### CSWCDoor::SetState  gog=0x507320 (param_bytes=1, notes='undefined1 param_1')
NO CANDIDATE (bracketing failed) -- see manual list

### CSWCDoor::GetServerDoor  gog=0x507760 (param_bytes=0, notes='void')
steam: cand=0x0087be80(mapper) delta=0x374720 align=4/6 size=53 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
callers=6 callees=007f2540=CSWCObject::ClearAllQueuedCombatActions
```
undefined4 FUN_0087be80(void)
{
  iVar1 = FUN_007f2540();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_007f2540();
    uVar3 = (**(code **)(*piVar2 + 0x18))(piVar2,uVar3);
  }
  return uVar3;
}
```

### CSWCDoor::UpdateAreaTransitionDisplay  gog=0x5082d0 (param_bytes=0, notes='void')
steam: cand=0x0087c9f0(mapper) delta=0x374720 align=4/6 size=3313 retn=0 gapG/S=3328/3328 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
callers=1 callees=00473e80, 0073f750=ClientApp::GetGuiInGame, 007aae00=CSWCArea::GetServerArea, 0044c8a0=Vector::operator+ (Vector3 add), 0073f450=CClientExoApp::GetPlayerCreature, 0087b7e0=gog:CSWCDoor::GetState, 0052d0e0=CSWSArea::NoNonWalkPolys, 004452b0=Vector::operator- (Vector3 subtract), 00465560, 0046ce40, 005160c0, 0073f490=CClientExoApp::BeginIntro, 005143c0, 005188c0 ...
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */
void __fastcall FUN_0087c9f0(int *param_1)
{
  undefined1 local_1f0 [12];
  undefined1 local_1e4 [12];
  undefined1 local_1d8 [12];
  undefined1 local_190 [16];
  undefined1 local_168 [12];
  undefined1 local_15c [12];
  undefined1 local_150 [16];
  undefined4 local_100 [27];
  undefined1 local_64 [8];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_009743f6;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_18 = FUN_0073f750(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_28 = (int *)FUN_0073f450();
  if ((((local_18 != 0) && (local_28 != (int *)0x0)) && (param_1[0x4c] != 0)) &&
     ((cVar1 = FUN_0087b7e0(), cVar1 == '\x01' || (cVar1 = FUN_0087b7e0(), cVar1 == '\x02')))) {
    local_24 = 0.0;
    local_20 = 0.0;
    local_1c = 0.0;
    if (*(int *)(local_18 + 0x101c) == 0) {
      local_24 = (float)local_28[0xc];
      local_20 = (float)local_28[0xd];
      local_1c = (float)local_28[0xe];
    }
    else {
      piVar2 = (int *)FUN_0073f490();
      puVar6 = local_15c;
      (**(code **)(*piVar2 + 0x24))(local_150,puVar6);
      pfVar3 = (float *)FUN_0046ce40(puVar6);
      local_24 = *pfVar3;
      local_20 = pfVar3[1];
      local_1c = 0.0;
      pfVar3 = (float *)FUN_005143c0(local_168,&local_24);
      local_24 = *pfVar3;
      local_20 = pfVar3[1];
      local_1c = pfVar3[2];
    }
    local_174 = (float)param_1[9] - (float)local_28[9];
    local_170 = (float)param_1[10] - (float)local_28[10];
... (165 more lines)
```

### CSWCObject::SetId  gog=0x410c30 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00764f60(mapper) delta=0x354330 align=3/6 size=22 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, DWORD id) [param_bytes=4]
name_map: CSWCObject::SetId [medium] gog 0x410c30; vote; retn=4
callers=0 callees=
```
void __thiscall FUN_00764f60(int param_1,undefined4 param_2)
{
  *(undefined4 *)(param_1 + 4) = param_2;
  return;
}
```

### CSWCObject::ShowShadowBlob  gog=0x410c50 (param_bytes=0, notes='void')
steam: cand=0x00764f80(mapper) delta=0x354330 align=4/6 size=44 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::ShowShadowBlob [medium] gog 0x410c50; vote; retn=0
callers=0 callees=
```
void __fastcall FUN_00764f80(int param_1)
{
  if (*(int *)(param_1 + 0x68) != 0) {
    (**(code **)(**(int **)(param_1 + 0x68) + 0xac))(0);
  }
  return;
}
```

### CSWCObject::HideShadowBlob  gog=0x410c80 (param_bytes=0, notes='CSWCObject * this')
steam: cand=0x00764fb0(mapper) delta=0x354330 align=4/6 size=42 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::HideShadowBlob [medium] gog 0x410c80; vote; retn=0
callers=0 callees=
```
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

### CSWCObject::GetName  gog=0x472580 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007ed420(mapper) delta=0x37aea0 align=3/6 size=47 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, void* outString) [param_bytes=4]
name_map: CSWCObject::GetName [medium] gog 0x472580; vote; retn=4
callers=0 callees=00733570=CExoString::CExoString_char
```
undefined4 FUN_007ed420(undefined4 param_1)
{
  FUN_00733570(&DAT_00986925);
  return param_1;
}
```

### CSWCObject::GetPortrait  gog=0x4725e0 (param_bytes=5, notes='undefined4 param_1, undefined1 param_2')
steam: cand=0x007ed480(bracket) delta=0x37aea0 align=/ size=39 retn=8 gapG/S=48/ cc_guess=
wrapper: void* (__thiscall) (void* thisPtr, void* outResRef) [param_bytes=4]
callers=0 callees=0085e8c0
```
undefined4 FUN_007ed480(undefined4 param_1,undefined1 param_2)
{
  FUN_0085e8c0(param_1,param_2);
  return param_1;
}
```

### CSWCObject::GetPortraitId  gog=0x472610 (param_bytes=0, notes='void')
steam: cand=0x007ed4b0(mapper) delta=0x37aea0 align=5/6 size=21 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: WORD (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::GetPortraitId [medium] gog 0x472610; vote; retn=0
callers=0 callees=
```
undefined2 __fastcall FUN_007ed4b0(int param_1)
{
  return *(undefined2 *)(param_1 + 0xb8);
}
```

### CSWCObject::TurnOnShadows  gog=0x472630 (param_bytes=0, notes='CSWCObject * this')
steam: cand=0x007ed4d0(mapper) delta=0x37aea0 align=4/6 size=42 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::TurnOnShadows [medium] gog 0x472630; vote; retn=0
callers=0 callees=
```
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

### CSWCObject::TurnOffShadows  gog=0x472660 (param_bytes=0, notes='CSWCObject * this')
steam: cand=0x007ed500(mapper) delta=0x37aea0 align=4/6 size=42 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::TurnOffShadows [medium] gog 0x472660; vote; retn=0
callers=0 callees=
```
/* Library Function - Single Match
    public: void __thiscall CPaneContainerManager::NotifyPaneDivider(void)
   Library: Visual Studio 2010 Debug */
void __thiscall CPaneContainerManager::NotifyPaneDivider(CPaneContainerManager *this)
{
  if (*(int *)(this + 0x68) != 0) {
    (**(code **)(**(int **)(this + 0x68) + 0xbc))();
  }
  return;
}
```

### CSWCObject::GetCurrentAnimation  gog=0x472930 (param_bytes=0, notes='void')
steam: cand=0x007ed7d0(mapper) delta=0x37aea0 align=4/6 size=18 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: WORD (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::GetCurrentAnimation [medium] gog 0x472930; vote; retn=0
callers=22 callees=
```
undefined2 __fastcall FUN_007ed7d0(int param_1)
{
  return *(undefined2 *)(param_1 + 0x48);
}
```

### CSWCObject::AttachToScene  gog=0x472950 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007ed7f0(mapper) delta=0x37aea0 align=4/6 size=61 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* scene) [param_bytes=4]
name_map: CSWCObject::AttachToScene [medium] gog 0x472950; vote; retn=4
callers=1 callees=00623320
```
void __thiscall FUN_007ed7f0(int param_1,int param_2)
{
  if (param_2 == 0) {
    FUN_00623320();
  }
  if (*(int *)(param_1 + 0x68) != 0) {
    (**(code **)(**(int **)(param_1 + 0x68) + 4))(param_2);
  }
  return;
}
```

### CSWCObject::SetLoopingAnimation  gog=0x472ab0 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ed950(bracket) delta=0x37aea0 align=/ size=42 retn=4 gapG/S=48/ cc_guess=
wrapper: void (__thiscall) (void* thisPtr, WORD animation) [param_bytes=4]
callers=6 callees=007ef500
```
void __thiscall FUN_007ed950(int param_1,undefined2 param_2)
{
  iVar1 = FUN_007ef500(param_2);
  if (iVar1 == 1) {
    *(undefined2 *)(param_1 + 0x58) = param_2;
  }
  return;
}
```

### CSWCObject::GetLoopingAnimation  gog=0x472ae0 (param_bytes=0, notes='void')
steam: cand=0x007ed980(mapper) delta=0x37aea0 align=2/6 size=18 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: WORD (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::GetLoopingAnimation [medium] gog 0x472ae0; vote; retn=0
callers=9 callees=
```
undefined2 __fastcall FUN_007ed980(int param_1)
{
  return *(undefined2 *)(param_1 + 0x58);
}
```

### CSWCObject::AnimationStationary  gog=0x474420 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef2c0(bracket) delta=0x37aea0 align=/ size=132 retn=4 gapG/S=144/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=6 callees=0071b8b0
```
bool __thiscall FUN_007ef2c0(int param_1,undefined2 param_2)
{
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe28,&local_8);
  return iVar1 == 0 || local_8 != 0;
}
```

### CSWCObject::AnimationPlayOutOfPlace  gog=0x4744b0 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef350(bracket) delta=0x37aea0 align=/ size=129 retn=4 gapG/S=144/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=2 callees=0071b8b0
```
bool __thiscall FUN_007ef350(int param_1,undefined2 param_2)
{
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe2c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### CSWCObject::AnimationHideEquippedItems  gog=0x474540 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef3e0(bracket) delta=0x37aea0 align=/ size=129 retn=4 gapG/S=144/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=1 callees=0071b8b0
```
bool __thiscall FUN_007ef3e0(int param_1,undefined2 param_2)
{
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe30,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### CSWCObject::AnimationWalking  gog=0x4745d0 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef470(bracket) delta=0x37aea0 align=/ size=129 retn=4 gapG/S=144/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=1 callees=0071b8b0
```
bool __thiscall FUN_007ef470(int param_1,undefined2 param_2)
{
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe34,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### CSWCObject::AnimationLooping  gog=0x474660 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef500(bracket) delta=0x37aea0 align=/ size=197 retn=4 gapG/S=208/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=12 callees=0071b8b0
```
bool __thiscall FUN_007ef500(int param_1,ushort param_2)
{
  if ((*(char *)(param_1 + 8) == '\x05') &&
     (((0x577 < param_2 && (param_2 < 0x5f8)) || ((0x63f < param_2 && (param_2 < 0x6c0)))))) {
    bVar2 = true;
  }
  else {
    local_8 = 0;
    if (*(int *)(param_1 + 0x68) != 0) {
      param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
    }
    iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe24,&local_8);
    if (iVar1 == 0) {
      bVar2 = false;
    }
    else {
      bVar2 = local_8 != 0;
    }
  }
  return bVar2;
}
```

### CSWCObject::AnimationFireAndForget  gog=0x474730 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef5d0(bracket) delta=0x37aea0 align=/ size=207 retn=4 gapG/S=208/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=11 callees=0071b8b0
```
bool __thiscall FUN_007ef5d0(int param_1,ushort param_2)
{
  if (((*(char *)(param_1 + 8) == '\x05') || (*(char *)(param_1 + 8) == '\t')) &&
     (((999 < param_2 && (param_2 < 0x468)) || ((0x4af < param_2 && (param_2 < 0x530)))))) {
    bVar2 = true;
  }
  else {
    local_8 = 0;
    if (*(int *)(param_1 + 0x68) != 0) {
      param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
    }
    iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe1c,&local_8);
    if (iVar1 == 0) {
      bVar2 = false;
    }
    else {
      bVar2 = local_8 != 0;
    }
  }
  return bVar2;
}
```

### CSWCObject::AnimationOverlay  gog=0x474800 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef6a0(bracket) delta=0x37aea0 align=/ size=131 retn=4 gapG/S=144/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=1 callees=0071b8b0
```
bool __thiscall FUN_007ef6a0(int param_1,undefined2 param_2)
{
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe20,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### CSWCObject::AnimationPause  gog=0x474890 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef730(bracket) delta=0x37aea0 align=/ size=68 retn=4 gapG/S=80/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=3 callees=
```
undefined4 FUN_007ef730(ushort param_1)
{
  if (((param_1 < 0x4b0) || (0x52f < param_1)) && ((param_1 < 0x640 || (0x6bf < param_1)))) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### CSWCObject::AnimationDialog  gog=0x4748e0 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef780(bracket) delta=0x37aea0 align=/ size=104 retn=4 gapG/S=112/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, DWORD animationRowIndex) [param_bytes=4]
callers=4 callees=007cb010, 007cae60, 0073f750=ClientApp::GetGuiInGame
```
undefined4 FUN_007ef780(ushort param_1)
{
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

### CSWCObject::AnimationParry  gog=0x474950 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef7f0(bracket) delta=0x37aea0 align=/ size=129 retn=4 gapG/S=144/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=2 callees=0071b8b0
```
bool __thiscall FUN_007ef7f0(int param_1,undefined2 param_2)
{
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe3c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### CSWCObject::AnimationAttack  gog=0x4749e0 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef880(bracket) delta=0x37aea0 align=/ size=129 retn=4 gapG/S=144/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=1 callees=0071b8b0
```
bool __thiscall FUN_007ef880(int param_1,undefined2 param_2)
{
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe4c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### CSWCObject::AnimationRunning  gog=0x474a70 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007ef910(bracket) delta=0x37aea0 align=/ size=129 retn=4 gapG/S=144/ cc_guess=
wrapper: int (__thiscall) (void* thisPtr, WORD animationRowIndex) [param_bytes=4]
callers=3 callees=0071b8b0
```
bool __thiscall FUN_007ef910(int param_1,undefined2 param_2)
{
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe50,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### CSWCObject::FadeObject  gog=0x476430 (param_bytes=0, notes='void')
steam: cand=0x007f12d0(mapper) delta=0x37aea0 align=5/6 size=896 retn=0 gapG/S=896/896 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::FadeObject [medium] gog 0x476430; vote; retn=0
callers=4 callees=0073f3b0=CClientExoApp::GetWorldTimer, 0051af60=gog:CWorldTimer::SubtractWorldTimes, 0091d220, 0051ad20=gog:CWorldTimer::GetWorldTime, 0051a940=gog:CWorldTimer::GetSnapshotTimeDifference, 007f1170=CSWCObject::GetFinalFadeAlpha, 00775860, 007f03b0, 007ed830=CSWCObject::GetAnimationBase
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
undefined4 __fastcall FUN_007f12d0(int *param_1)
{
  float10 fVar2;
  undefined1 local_24 [4];
  undefined1 local_10 [4];
  if ((char)param_1[0x24] == *(char *)((int)param_1 + 0x91)) {
    if (param_1[0x25] != 0) {
      puVar9 = &local_28;
      puVar7 = &local_2c;
      FUN_0073f3b0(puVar7,puVar9);
      FUN_0051ad20(puVar7,puVar9);
      puVar10 = &local_30;
      puVar8 = local_24;
      iVar5 = param_1[0x21];
      iVar4 = param_1[0x20];
      FUN_0073f3b0(local_2c,local_28,iVar4,iVar5,puVar8,puVar10);
      FUN_0051af60(local_2c,local_28,iVar4,iVar5,puVar8,puVar10);
      if ((uint)(param_1[0x22] + param_1[0x23]) <= local_30) {
        return 1;
      }
    }
  }
  else {
    FUN_0073f3b0();
    uVar3 = FUN_0051a940();
    local_8 = __aulldiv(uVar3,1000,0);
    if (*(char *)((int)param_1 + 0x91) == '\x01') {
      fVar2 = (float10)FUN_007f1170();
      if ((float10)(float)param_1[0x26] < fVar2) {
        param_1[0x26] =
             (int)(float)((float10)local_8 / (float10)_DAT_009963e0 + (float10)(float)param_1[0x26])
        ;
        *(undefined1 *)(param_1 + 0x24) = 2;
      }
      else {
        fVar2 = (float10)FUN_007f1170();
        param_1[0x26] = (int)(float)fVar2;
        *(undefined1 *)(param_1 + 0x24) = 1;
      }
    }
    else if (*(char *)((int)param_1 + 0x91) == '\0') {
      if (param_1[0x23] != 0) {
        puVar9 = &local_14;
        puVar7 = &local_18;
... (53 more lines)
```

### CSWCObject::SetPortrait  gog=0x476c90 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007f1b30(mapper) delta=0x37aea0 align=5/6 size=68 retn=4 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* portrait) [param_bytes=4]
name_map: CSWCObject::SetPortrait [medium] gog 0x476c90; vote; retn=4
callers=0 callees=0040e770=gog:CSWGuiPanel::Constructor, 00710ca0
```
void __thiscall FUN_007f1b30(int *param_1,undefined4 param_2)
{
  undefined1 local_24 [16];
  undefined1 local_14 [16];
  FUN_00710ca0(local_24,param_2);
  uVar1 = (**(code **)(*param_1 + 0xf0))(local_14,1);
  FUN_0040e770(uVar1);
  return;
}
```

### CSWCObject::SetPortraitId  gog=0x477110 (param_bytes=2, notes='undefined2 param_1')
steam: cand=0x007f1fb0(bracket) delta=0x37aea0 align=/ size=162 retn=4 gapG/S=176/ cc_guess=
wrapper: void (__thiscall) (void* thisPtr, WORD id) [param_bytes=4]
callers=0 callees=0085e890, 00623320, 0085e8c0, 0085ead0, 009196ee=__security_check_cookie
```
void __thiscall FUN_007f1fb0(int *param_1,undefined2 param_2)
{
  undefined1 local_34 [32];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096ba48;
  local_10 = ExceptionList;
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0085e890(local_14);
  local_8 = 0;
  iVar1 = FUN_0085ead0(param_2);
  if (iVar1 != 0) {
    uVar2 = FUN_0085e8c0(local_34,0);
    (**(code **)(*param_1 + 0xec))(uVar2);
  }
  *(undefined2 *)(param_1 + 0x2e) = param_2;
  local_8 = 0xffffffff;
  FUN_00623320();
  ExceptionList = local_10;
  __security_check_cookie(local_14 ^ (uint)&stack0xfffffffc);
  return;
}
```

### CSWCObject::SetFeedbackInfo  gog=0x477360 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007f2200(mapper) delta=0x37aea0 align=3/6 size=44 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* info) [param_bytes=4]
name_map: CSWCObject::SetFeedbackInfo [medium] gog 0x477360; vote; retn=4
callers=2 callees=007337c0=CExoString::operator=
```
void __thiscall FUN_007f2200(int param_1,undefined4 param_2)
{
  FUN_007337c0(param_2);
  *(undefined4 *)(param_1 + 0xf0) = 5000;
  return;
}
```

### CSWCObject::ClearAllActions  gog=0x477660 (param_bytes=0, notes='void')
steam: cand=0x007f2500(mapper) delta=0x37aea0 align=1/6 size=54 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::ClearAllActions [medium] gog 0x477660; vote; retn=0
callers=2 callees=0051cba0=gog:CServerExoApp::ClearAllQueuedCombatActions, 0051c8b0=gog:CServerExoApp::ClientToServerObjectId
```
void __fastcall FUN_007f2500(int param_1)
{
  uVar1 = FUN_0051c8b0(*(undefined4 *)(param_1 + 4));
  FUN_0051cba0(uVar1);
  return;
}
```

### CSWCObject::ClearAllQueuedCombatActions  gog=0x4776a0 (param_bytes=0, notes='void')
steam: cand=0x007f2540(mapper) delta=0x37aea0 align=1/6 size=208 retn=0 gapG/S=64/208 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::ClearAllQueuedCombatActions [medium] gog 0x4776a0; vote; retn=0
callers=11 callees=0051c8b0=gog:CServerExoApp::ClientToServerObjectId, 0051c0a0=CServerExoApp::GetGameObject, 0054a8e0=CSWSObject::AttachmentFromClientObject
```
undefined4 __fastcall FUN_007f2540(int param_1)
{
  if (*(int *)(param_1 + 0xf8) == 0) {
    if (*(int *)(param_1 + 0xe4) != 0) {
      return 0;
    }
    if (*(int *)(DAT_00a1b4a4 + 8) == 0) {
      return 0;
    }
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar1 = FUN_0051c8b0(*(undefined4 *)(param_1 + 4));
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    piVar2 = (int *)FUN_0051c0a0(uVar1);
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

### CSWCObject::GetServerObject  gog=0x4776e0 (param_bytes=0, notes='void')
steam: cand=0x007f2540(mapper) delta=0x37ae60 align=6/6 size=208 retn=0 gapG/S=208/208 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::ClearAllQueuedCombatActions [medium] gog 0x4776a0; vote; retn=0
callers=11 callees=0051c8b0=gog:CServerExoApp::ClientToServerObjectId, 0051c0a0=CServerExoApp::GetGameObject, 0054a8e0=CSWSObject::AttachmentFromClientObject
```
undefined4 __fastcall FUN_007f2540(int param_1)
{
  if (*(int *)(param_1 + 0xf8) == 0) {
    if (*(int *)(param_1 + 0xe4) != 0) {
      return 0;
    }
    if (*(int *)(DAT_00a1b4a4 + 8) == 0) {
      return 0;
    }
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar1 = FUN_0051c8b0(*(undefined4 *)(param_1 + 4));
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    piVar2 = (int *)FUN_0051c0a0(uVar1);
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

### CSWCObject::AttachmentFromServerObject  gog=0x4777b0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007f2610(mapper) delta=0x37ae60 align=6/6 size=25 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* serverObject) [param_bytes=4]
name_map: CSWCObject::AttachmentFromServerObject [medium] gog 0x4777b0; vote; retn=4
callers=1 callees=
```
void __thiscall FUN_007f2610(int param_1,undefined4 param_2)
{
  *(undefined4 *)(param_1 + 0xf8) = param_2;
  return;
}
```

### CSWCObject::DetachFromServerObject  gog=0x4777d0 (param_bytes=0, notes='void')
steam: cand=0x007f2630(mapper) delta=0x37ae60 align=6/6 size=24 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCObject::DetachFromServerObject [medium] gog 0x4777d0; vote; retn=0
callers=1 callees=
```
void __fastcall FUN_007f2630(int param_1)
{
  *(undefined4 *)(param_1 + 0xf8) = 0;
  return;
}
```

### CSWCPlaceable::IsHostile  gog=0x534a00 (param_bytes=0, notes='void')
steam: cand=0x00841180(mapper) delta=0x30c780 align=5/6 size=66 retn=0 gapG/S=80/80 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCPlaceable::IsHostile [medium] gog 0x534a00; vote; retn=0
callers=0 callees=00842d00=CSWCPlaceable::GetServerPlaceable
```
undefined4 FUN_00841180(void)
{
  iVar1 = FUN_00842d00();
  if (iVar1 == 0) {
    local_14 = 0;
  }
  else {
    iVar1 = FUN_00842d00();
    local_14 = *(undefined4 *)(iVar1 + 0x398);
  }
  return local_14;
}
```

### CSWCPlaceable::ActionMenuSecurity  gog=0x535fd0 (param_bytes=0, notes='void')
NO CANDIDATE (bracketing failed) -- see manual list

### CSWCPlaceable::GetServerPlaceable  gog=0x536580 (param_bytes=0, notes='void')
steam: cand=0x00842d00(mapper) delta=0x30c780 align=6/6 size=53 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CSWCPlaceable::GetServerPlaceable [high] decompile+asm from full dump (dump_excerpts/), Opus read 2026-09-30
callers=20 callees=007f2540=CSWCObject::ClearAllQueuedCombatActions
```
undefined4 FUN_00842d00(void)
{
  iVar1 = FUN_007f2540();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_007f2540();
    uVar3 = (**(code **)(*piVar2 + 0x48))(piVar2,uVar3);
  }
  return uVar3;
}
```

### CSWCPlaceable::SetAppearance  gog=0x5371e0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00843960(mapper) delta=0x30c780 align=5/6 size=327 retn=4 gapG/S=336/336 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, BYTE appearance) [param_bytes=4]
name_map: CSWCPlaceable::SetAppearance [medium] gog 0x5371e0; vote; retn=4
callers=2 callees=00733570=CExoString::CExoString_char, 0071bcf0=C2DA::GetINTEntry_2, 00733780=CExoString::~CExoString
strings: NonCull | HitCheck
```
void __thiscall FUN_00843960(int *param_1,int param_2)
{
  undefined1 local_30 [11];
  undefined1 local_24 [8];
  undefined1 local_1c [4];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00970a30;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  param_1[0x43] = param_2;
  local_18 = 0;
  FUN_00733570("HitCheck");
  local_8 = 0;
  FUN_0071bcf0(param_2,local_24,&local_18);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  local_14 = (int *)(**(code **)(*param_1 + 0x98))(0xff,1);
  if (local_14 != (int *)0x0) {
    (**(code **)(*local_14 + 0x90))(local_18 == 0);
  }
  if (param_1[0x1a] != 0) {
    FUN_00733570("NonCull");
    local_8 = 1;
    iVar2 = FUN_0071bcf0(param_2,local_30,local_1c);
    local_25 = '\x01' - (iVar2 != 0);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

