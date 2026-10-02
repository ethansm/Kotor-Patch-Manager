# Digest B_CGuiInGame

### CGuiInGame::ShutDownGlobal  gog=0x4cdda0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0073ffc0(mapper) delta=0x272220 align=3/6 size=28 retn=4 gapG/S=176/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::SetPlayerCharacterName [medium] gog 0x40d4a0; vote; retn=4
callers=3 callees=00798580=CClientExoAppInternal::SetPlayerCharacterName
```
void FUN_0073ffc0(undefined4 param_1)
{
  FUN_00798580(param_1);
  return;
}
```

### CGuiInGame::AppendToMsgBuffer  gog=0x4ce720 (param_bytes=9, notes='undefined4 param_1, undefined4 param_2, undefined1 param_3')
NO CANDIDATE (bracketing failed) -- see manual list

### CGuiInGame::SetLevelUpMode  gog=0x4d00b0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007bfa20(mapper) delta=0x2ef970 align=4/6 size=41 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int mode) [param_bytes=4]
name_map: CGuiInGame::SetLevelUpMode [medium] gog 0x4d00b0; vote; retn=4
callers=2 callees=
```
void __thiscall FUN_007bfa20(int param_1,int param_2)
{
  if (*(int *)(param_1 + 300) != param_2) {
    *(int *)(param_1 + 300) = param_2;
  }
  return;
}
```

### CGuiInGame::GetCameraAnimationName  gog=0x4d1550 (param_bytes=6, notes='undefined4 param_1, undefined2 param_2')
steam: cand=0x007c0ec0(bracket) delta=0x2ef970 align=/ size=9401 retn=8 gapG/S=12320/ cc_guess=
wrapper: void* (__stdcall) (void* outName, short animationId) [param_bytes=8]
callers=1 callees=00733570=CExoString::CExoString_char, 007335f0=CExoString::CExoString_copy, 00733780=CExoString::~CExoString, 007338d0=CExoString::operator=_2
strings: cut128wl | cut127wl | cut126wl | cut125wl | cut124wl | cut123wl
```
undefined4 FUN_007c0ec0(undefined4 param_1,ushort param_2)
{
  undefined1 local_18 [8];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_009699d1;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  uVar4 = 0;
  FUN_00733570(&DAT_00987fb4);
  local_8 = 1;
  if (param_2 == 0x2772) {
    FUN_007335f0(local_18);
    local_8 = local_8 & 0xffffff00;
    FUN_00733780(uVar1);
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
... (1519 more lines)
```

### CGuiInGame::LoadAmbientTrackForDialog  gog=0x4d4750 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007c4060(mapper) delta=0x2ef910 align=0/6 size=93 retn=4 gapG/S=304/96 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* track) [param_bytes=4]
name_map: CGuiInGame::LoadCameraModelForDialog [medium] gog 0x4d46f0; vote; retn=4
callers=1 callees=007f66e0, 00710a10, 0073f430=CClientExoApp::GetModule
```
void FUN_007c4060(undefined4 param_1)
{
  iVar1 = FUN_00710a10();
  if ((iVar1 != 0) && (*(int *)(extraout_var + 0xf08) == 0)) {
    iVar1 = extraout_var;
    iVar2 = FUN_0073f430(extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 4));
    if (iVar2 != 0) {
      FUN_007f66e0(param_1);
      *(undefined4 *)(iVar1 + 0xf08) = 1;
    }
  }
  return;
}
```

### CGuiInGame::ShutDown  gog=0x4d7720 (param_bytes=0, notes='void')
steam: cand=0x007c7030(mapper) delta=0x2ef910 align=3/6 size=2831 retn=0 gapG/S=3312/2832 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::ShutDown [medium] gog 0x4d7720; vote; retn=0
callers=4 callees=007c0090=CGuiInGame::ClearDialogAnimationData, 009196fd=operator delete, 007c0290=CGuiInGame::ClearRepliesData, 007c0210=CGuiInGame::ClearLastDialogAnimationData, 008aa720
```
void __fastcall FUN_007c7030(int param_1)
{
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
... (147 more lines)
```

### CGuiInGame::GetMessageBufferSize  gog=0x4d8430 (param_bytes=0, notes='void')
steam: cand=0x007c7d40(mapper) delta=0x2ef910 align=5/6 size=20 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::GetMessageBufferSize [medium] gog 0x4d8430; vote; retn=0
callers=1 callees=
```
undefined4 __fastcall FUN_007c7d40(int param_1)
{
  return *(undefined4 *)(param_1 + 0x11c);
}
```

### CGuiInGame::LoadStuntModelsForDialog  gog=0x4d8a80 (param_bytes=0, notes='void')
steam: cand=0x007c8390(mapper) delta=0x2ef910 align=6/6 size=321 retn=0 gapG/S=336/336 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::LoadStuntModelsForDialog [medium] gog 0x4d8a80; vote; retn=0
callers=1 callees=0073f4d0=ClientApp::GetClientObject, 00710980, 009196ee=__security_check_cookie
```
void __fastcall FUN_007c8390(int param_1)
{
  char local_20 [20];
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  for (local_8 = 0; local_8 < *(uint *)(param_1 + 0x1ac); local_8 = local_8 + 1) {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    piVar1 = (int *)FUN_0073f4d0(*(undefined4 *)(*(int *)(param_1 + 0x1b0) + local_8 * 4));
    local_2c = (int *)0x0;
    if (piVar1 != (int *)0x0) {
      local_2c = (int *)(**(code **)(*piVar1 + 0xc))(param_1,uVar2);
    }
    if ((local_2c != (int *)0x0) && (FUN_00710980(local_20), local_20[0] != '\0')) {
      piVar1 = (int *)(**(code **)(*local_2c + 0x98))(0xff,0);
      if (piVar1 == (int *)0x0) break;
      (**(code **)(*piVar1 + 0x20))(local_20);
      piVar1 = (int *)(**(code **)(*local_2c + 0x98))(0xfe,0);
      if (piVar1 != (int *)0x0) {
        (**(code **)(*piVar1 + 0x20))(local_20);
      }
    }
  }
  __security_check_cookie(local_c ^ (uint)&stack0xfffffffc);
  return;
}
```

### CGuiInGame::ResetDialogAnimations  gog=0x4d9ad0 (param_bytes=0, notes='void')
steam: cand=0x007c93e0(mapper) delta=0x2ef910 align=5/6 size=373 retn=0 gapG/S=384/384 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::ResetDialogAnimations [medium] gog 0x4d9ad0; vote; retn=0
callers=1 callees=00842d00=CSWCPlaceable::GetServerPlaceable, 0073f4d0=ClientApp::GetClientObject, 007cf4e0=CGuiInGame::CanPlayDialogAnimations, 007ed7d0=CSWCObject::GetCurrentAnimation
```
void __fastcall FUN_007c93e0(int param_1)
{
  for (local_8 = 0; local_8 < *(uint *)(param_1 + 0x1bc); local_8 = local_8 + 1) {
    piVar2 = (int *)FUN_0073f4d0(*(undefined4 *)(*(int *)(param_1 + 0x1c8) + local_8 * 4));
    if (piVar2 != (int *)0x0) {
      iVar3 = (**(code **)(*piVar2 + 0x2c))();
      if (iVar3 != 0) {
        piVar4 = (int *)(**(code **)(*piVar2 + 0x2c))();
        iVar3 = FUN_007cf4e0(piVar4[1]);
        if ((iVar3 != 0) && (piVar4 != (int *)0x0)) {
          piVar4[0x15] = 0;
          sVar1 = FUN_007ed7d0();
          if (sVar1 != 10000) {
            (**(code **)(*piVar4 + 0x9c))(10000,0,1);
          }
          (**(code **)(*(int *)piVar4[0x1a] + 0x70))(0);
        }
      }
      iVar3 = (**(code **)(*piVar2 + 0x4c))();
      if (iVar3 != 0) {
        piVar2 = (int *)(**(code **)(*piVar2 + 0x4c))();
        iVar3 = FUN_00842d00();
        if (piVar2 != (int *)0x0) {
          piVar2[0x15] = 0;
          (**(code **)(*piVar2 + 0x9c))(*(uint *)(iVar3 + 0xd8) & 0xffff,0,0);
          (**(code **)(*(int *)piVar2[0x1a] + 0x70))(0);
        }
      }
    }
  }
  return;
}
```

### CGuiInGame::ResetTalkAnimations  gog=0x4d9c50 (param_bytes=0, notes='void')
steam: cand=0x007c9560(mapper) delta=0x2ef910 align=4/6 size=750 retn=0 gapG/S=752/752 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::ResetTalkAnimations [medium] gog 0x4d9c50; vote; retn=0
callers=1 callees=0073f4d0=ClientApp::GetClientObject, 007cf4e0=CGuiInGame::CanPlayDialogAnimations, 007ed7d0=CSWCObject::GetCurrentAnimation
```
void __fastcall FUN_007c9560(int param_1)
{
  piVar2 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x198));
  if ((piVar2 != (int *)0x0) && (iVar3 = (**(code **)(*piVar2 + 0xc))(), iVar3 != 0)) {
    iVar3 = (**(code **)(*piVar2 + 0x2c))();
    if (iVar3 == 0) {
      (**(code **)(*piVar2 + 0xc))();
      sVar1 = FUN_007ed7d0();
      if ((sVar1 != 10000) && (iVar3 = (**(code **)(*piVar2 + 0x14))(), iVar3 == 0)) {
        piVar4 = (int *)(**(code **)(*piVar2 + 0xc))();
        (**(code **)(*piVar4 + 0x9c))(10000,0,1);
      }
      iVar3 = (**(code **)(*piVar2 + 0xc))();
      (**(code **)(**(int **)(iVar3 + 0x68) + 0x70))(0);
    }
    else {
      iVar3 = (**(code **)(*piVar2 + 0x2c))();
      iVar3 = FUN_007cf4e0(*(undefined4 *)(iVar3 + 4));
      if (iVar3 != 0) {
        (**(code **)(*piVar2 + 0xc))();
        sVar1 = FUN_007ed7d0();
        if (sVar1 != 10000) {
          piVar4 = (int *)(**(code **)(*piVar2 + 0xc))();
          (**(code **)(*piVar4 + 0x9c))(10000,0,1);
        }
        iVar3 = (**(code **)(*piVar2 + 0xc))();
        (**(code **)(**(int **)(iVar3 + 0x68) + 0x70))(0);
      }
    }
  }
  piVar2 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x19c));
  if ((piVar2 != (int *)0x0) && (iVar3 = (**(code **)(*piVar2 + 0xc))(), iVar3 != 0)) {
    iVar3 = (**(code **)(*piVar2 + 0x2c))();
    if (iVar3 == 0) {
      (**(code **)(*piVar2 + 0xc))();
      sVar1 = FUN_007ed7d0();
      if ((sVar1 != 10000) && (iVar3 = (**(code **)(*piVar2 + 0x14))(), iVar3 == 0)) {
        piVar4 = (int *)(**(code **)(*piVar2 + 0xc))();
        (**(code **)(*piVar4 + 0x9c))(10000,0,1);
      }
      iVar3 = (**(code **)(*piVar2 + 0xc))();
      (**(code **)(**(int **)(iVar3 + 0x68) + 0x70))(0);
    }
    else {
      iVar3 = (**(code **)(*piVar2 + 0x2c))();
... (15 more lines)
```

### CGuiInGame::NextSWInGameGui  gog=0x4da930 (param_bytes=0, notes='void')
steam: cand=0x007ca230(mapper) delta=0x2ef900 align=6/6 size=389 retn=0 gapG/S=400/400 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::NextSWInGameGui [medium] gog 0x4da930; vote; retn=0
callers=1 callees=00733570=CExoString::CExoString_char, 0073fea0=CClientExoApp::GetSWGuiManager, 00410670=gog:CSWGuiManager::RemovePanel, 00410530=gog:CSWGuiManager::AddPanel, 007d0760=CGuiInGame::UpdateCreatedInGameGUI, 00410780=gog:CSWGuiManager::SendPanelToBack, 006fd8d0=CVirtualMachine::RunScript(CExoString*, objid, int, 00733780=CExoString::~CExoString, 00756860
strings: k_pend_screenchg
```
void __fastcall FUN_007ca230(int param_1)
{
  undefined1 local_20 [8];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096fe28;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x128) != 0) {
    local_18 = FUN_0073fea0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (((DAT_00a10760 != 0) && (*(int *)(param_1 + 0x30) != 0)) && (local_18 != 0)) {
      FUN_00410670(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
    }
    local_14 = *(undefined4 *)(param_1 + 0x2c);
    *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + 1;
    if (7 < *(int *)(param_1 + 0x2c)) {
      *(undefined4 *)(param_1 + 0x2c) = 0;
    }
    if ((*(int *)(param_1 + 0x30) == 0) || (local_18 == 0)) {
      local_2c = 0xffffffff;
    }
    else {
      local_2c = *(undefined4 *)(param_1 + 0x2c);
    }
    FUN_007d0760(local_14,local_2c);
    if ((*(int *)(param_1 + 0x30) != 0) && (local_18 != 0)) {
      FUN_00410530(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4),2,1);
      FUN_00410780(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
      FUN_00756860(*(undefined4 *)(param_1 + 0x2c),1);
      if (DAT_00a1b4a8 != 0) {
        FUN_00733570("k_pend_screenchg");
        local_8 = 0;
        FUN_006fd8d0(local_20,0x7f000000,0);
        local_8 = 0xffffffff;
        FUN_00733780();
      }
    }
  }
  ExceptionList = local_10;
  return;
}
```

### CGuiInGame::PrevSWInGameGui  gog=0x4daac0 (param_bytes=0, notes='void')
steam: cand=0x007ca3c0(mapper) delta=0x2ef900 align=6/6 size=389 retn=0 gapG/S=400/400 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::PrevSWInGameGui [medium] gog 0x4daac0; vote; retn=0
callers=1 callees=00733570=CExoString::CExoString_char, 0073fea0=CClientExoApp::GetSWGuiManager, 00410670=gog:CSWGuiManager::RemovePanel, 00410530=gog:CSWGuiManager::AddPanel, 007d0760=CGuiInGame::UpdateCreatedInGameGUI, 00410780=gog:CSWGuiManager::SendPanelToBack, 006fd8d0=CVirtualMachine::RunScript(CExoString*, objid, int, 00733780=CExoString::~CExoString, 00756860
strings: k_pend_screenchg
```
void __fastcall FUN_007ca3c0(int param_1)
{
  undefined1 local_20 [8];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096fe28;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x128) != 0) {
    local_18 = FUN_0073fea0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (((DAT_00a10760 != 0) && (*(int *)(param_1 + 0x30) != 0)) && (local_18 != 0)) {
      FUN_00410670(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
    }
    local_14 = *(undefined4 *)(param_1 + 0x2c);
    *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + -1;
    if (*(int *)(param_1 + 0x2c) < 0) {
      *(undefined4 *)(param_1 + 0x2c) = 7;
    }
    if ((*(int *)(param_1 + 0x30) == 0) || (local_18 == 0)) {
      local_2c = 0xffffffff;
    }
    else {
      local_2c = *(undefined4 *)(param_1 + 0x2c);
    }
    FUN_007d0760(local_14,local_2c);
    if ((*(int *)(param_1 + 0x30) != 0) && (local_18 != 0)) {
      FUN_00410530(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4),2,1);
      FUN_00410780(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
      FUN_00756860(*(undefined4 *)(param_1 + 0x2c),1);
      if (DAT_00a1b4a8 != 0) {
        FUN_00733570("k_pend_screenchg");
        local_8 = 0;
        FUN_006fd8d0(local_20,0x7f000000,0);
        local_8 = 0xffffffff;
        FUN_00733780();
      }
    }
  }
  ExceptionList = local_10;
  return;
}
```

### CGuiInGame::SwitchToSWInGameGui  gog=0x4dac50 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007ca550(mapper) delta=0x2ef900 align=6/6 size=389 retn=4 gapG/S=400/400 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int guiId) [param_bytes=4]
name_map: CGuiInGame::SwitchToSWInGameGui [medium] gog 0x4dac50; vote; retn=4
callers=9 callees=00733570=CExoString::CExoString_char, 0073fea0=CClientExoApp::GetSWGuiManager, 00410670=gog:CSWGuiManager::RemovePanel, 00410530=gog:CSWGuiManager::AddPanel, 007d0760=CGuiInGame::UpdateCreatedInGameGUI, 00410780=gog:CSWGuiManager::SendPanelToBack, 006fd8d0=CVirtualMachine::RunScript(CExoString*, objid, int, 00733780=CExoString::~CExoString, 00756860
strings: k_pend_screenchg
```
void __thiscall FUN_007ca550(int param_1,int param_2)
{
  undefined1 local_1c [8];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952ee8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((((-1 < param_2) && (param_2 < 8)) && (param_2 != *(int *)(param_1 + 0x2c))) &&
     (*(int *)(param_1 + 0x128) != 0)) {
    local_14 = FUN_0073fea0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (((DAT_00a10760 != 0) && (*(int *)(param_1 + 0x30) != 0)) && (local_14 != 0)) {
      FUN_00410670(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
    }
    if ((*(int *)(param_1 + 0x30) == 0) || (local_14 == 0)) {
      local_28 = -1;
    }
    else {
      local_28 = param_2;
    }
    FUN_007d0760(*(undefined4 *)(param_1 + 0x2c),local_28);
    *(int *)(param_1 + 0x2c) = param_2;
    if ((*(int *)(param_1 + 0x30) != 0) && (local_14 != 0)) {
      FUN_00410530(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4),2,1);
      FUN_00410780(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
      FUN_00756860(*(undefined4 *)(param_1 + 0x2c),1);
      if (DAT_00a1b4a8 != 0) {
        FUN_00733570("k_pend_screenchg");
        local_8 = 0;
        FUN_006fd8d0(local_1c,0x7f000000,0);
        local_8 = 0xffffffff;
        FUN_00733780();
      }
    }
  }
  ExceptionList = local_10;
  return;
}
```

### CGuiInGame::ShowGalaxyMapGui  gog=0x4dae30 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007ca730(mapper) delta=0x2ef900 align=5/6 size=494 retn=4 gapG/S=496/496 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int selectedPlanetId) [param_bytes=4]
name_map: CGuiInGame::ShowGalaxyMapGui [medium] gog 0x4dae30; vote; retn=4
callers=1 callees=00733570=CExoString::CExoString_char, 00740330=CClientExoApp::GetPausedByCombat, 005fed90, 0070bc60, 00733780=CExoString::~CExoString, 008973d0, 0073f750=ClientApp::GetGuiInGame, 0051c8d0=CServerExoApp::GetPartyTable (inline at *(this+4)+, 008bdf50, 004122a0, 00410530=gog:CSWGuiManager::AddPanel, 00740350=CClientExoApp::SetPausedByCombat, 0073fee0=CClientExoApp::SetInputClass, 007401f0=CClientExoApp::IndicateObject ...
```
void __thiscall FUN_007ca730(int param_1,undefined4 param_2)
{
  undefined1 local_28 [8];
  undefined1 local_20 [16];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969ad3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x30) == 0)) {
    iVar1 = FUN_0073f750(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (*(int *)(iVar1 + 0xcc) == 0) {
      iVar1 = FUN_00740330();
      if (iVar1 == 0) {
        FUN_00740350(1,0,0);
      }
      FUN_007401f0(0,0);
      FUN_0070bc60(DAT_0099c2b8,0);
      FUN_007c9c40(3,1);
      FUN_00710840(&DAT_00986925);
      FUN_00733570(&DAT_00986925);
      local_8 = 0;
      FUN_008bdf50(0x7f000000,local_28,local_20,0);
      local_8 = 0xffffffff;
      FUN_00733780();
      FUN_0073fee0(2,1);
      if (*(int *)(param_1 + 0x80) == 0) {
        iVar1 = FUN_00919723(0x28d8);
        local_8 = 1;
        if (iVar1 == 0) {
          local_58 = 0;
        }
        else {
          local_58 = FUN_008973d0(*(undefined4 *)(param_1 + 0x38));
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x80) = local_58;
      }
      FUN_0051c8d0(param_2);
      FUN_005fed90(param_2);
      FUN_00410530(*(undefined4 *)(param_1 + 0x80),2,1);
      FUN_004122a0(4);
    }
  }
  ExceptionList = local_10;
... (2 more lines)
```

### CGuiInGame::HideGalaxyMapGui  gog=0x4db020 (param_bytes=0, notes='void')
steam: cand=0x007ca920(mapper) delta=0x2ef900 align=5/6 size=239 retn=0 gapG/S=224/240 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::HideGalaxyMapGui [medium] gog 0x4db020; vote; retn=0
callers=1 callees=00740330=CClientExoApp::GetPausedByCombat, 00410670=gog:CSWGuiManager::RemovePanel, 004122a0, 00740350=CClientExoApp::SetPausedByCombat, 0073fee0=CClientExoApp::SetInputClass, 007c9c40=CGuiInGame::SetSWGuiStatus, 0070bc60
```
void __fastcall FUN_007ca920(int param_1)
{
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = FUN_00740330();
    if (iVar1 != 0) {
      FUN_00740350(0,0,0);
    }
    FUN_007c9c40(4,1);
    FUN_0073fee0(0,1);
    FUN_00410670(*(undefined4 *)(param_1 + 0x80));
    if (DAT_00a10760 == 0) {
      if (*(undefined4 **)(param_1 + 0x80) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x80))(1);
      }
      *(undefined4 *)(param_1 + 0x80) = 0;
    }
    FUN_004122a0(5);
    FUN_0070bc60(DAT_0099c2a8,0);
  }
  return;
}
```

### CGuiInGame::ShowItemCreateMenu  gog=0x4db110 (param_bytes=0, notes='void')
steam: cand=0x007caa10(mapper) delta=0x2ef900 align=5/6 size=71 retn=0 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::ShowDebugMenu [medium] gog 0x4db100; vote; retn=0
callers=1 callees=00410530=gog:CSWGuiManager::AddPanel, 0073fee0=CClientExoApp::SetInputClass
```
void __fastcall FUN_007caa10(int param_1)
{
  if (*(int *)(param_1 + 0x128) != 0) {
    FUN_00410530(*(undefined4 *)(param_1 + 0x58),1,1);
    FUN_0073fee0(2,1);
  }
  return;
}
```

### CGuiInGame::HideItemCreateMenu  gog=0x4db160 (param_bytes=0, notes='void')
steam: cand=0x007caa60(mapper) delta=0x2ef900 align=5/6 size=149 retn=0 gapG/S=160/160 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::HideItemCreateMenu [medium] gog 0x4db160; vote; retn=0
callers=3 callees=00410670=gog:CSWGuiManager::RemovePanel, 0073fee0=CClientExoApp::SetInputClass, 00410800=GuiManager::Contains, 007400a0=CClientExoApp::GetMiniGame, 004109c0=gog:CSWGuiManager::PopModalPanel
```
void __fastcall FUN_007caa60(int param_1)
{
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x58));
    if (iVar1 != 0) {
      FUN_004109c0();
      FUN_00410670(*(undefined4 *)(param_1 + 0x58));
      iVar1 = FUN_007400a0();
      if (iVar1 == 0) {
        FUN_0073fee0(0,1);
      }
      else {
        FUN_0073fee0(1,1);
      }
    }
  }
  return;
}
```

### CGuiInGame::HideSubItemCreateMenu  gog=0x4db260 (param_bytes=0, notes='void')
steam: cand=0x007cab60(mapper) delta=0x2ef900 align=6/6 size=164 retn=0 gapG/S=176/176 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::HideSubItemCreateMenu [medium] gog 0x4db260; vote; retn=0
callers=3 callees=00410670=gog:CSWGuiManager::RemovePanel, 008bf9f0, 0073fee0=CClientExoApp::SetInputClass, 00410800=GuiManager::Contains, 007400a0=CClientExoApp::GetMiniGame, 004109c0=gog:CSWGuiManager::PopModalPanel
```
void __fastcall FUN_007cab60(int param_1)
{
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x5c));
    if (iVar1 != 0) {
      FUN_004109c0();
      FUN_00410670(*(undefined4 *)(param_1 + 0x5c));
      iVar1 = FUN_007400a0();
      if (iVar1 == 0) {
        FUN_0073fee0(0,1);
      }
      else {
        FUN_0073fee0(1,1);
      }
      FUN_008bf9f0();
    }
  }
  return;
}
```

### CGuiInGame::HideContainerGui  gog=0x4db430 (param_bytes=0, notes='void')
steam: cand=0x007cad30(mapper) delta=0x2ef900 align=4/6 size=126 retn=0 gapG/S=128/128 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::HideContainerGui [medium] gog 0x4db430; vote; retn=0
callers=2 callees=00410670=gog:CSWGuiManager::RemovePanel, 00740350=CClientExoApp::SetPausedByCombat, 0073fee0=CClientExoApp::SetInputClass, 00410800=GuiManager::Contains, 004109c0=gog:CSWGuiManager::PopModalPanel
```
void __fastcall FUN_007cad30(int param_1)
{
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x54));
    if (iVar1 != 0) {
      FUN_004109c0();
      FUN_00410670(*(undefined4 *)(param_1 + 0x54));
      FUN_0073fee0(0,1);
      FUN_00740350(0,0,0);
    }
  }
  return;
}
```

### CGuiInGame::HideLoadModuleDebugMenu  gog=0x4dbba0 (param_bytes=0, notes='void')
steam: cand=0x007cb4a0(mapper) delta=0x2ef900 align=5/6 size=149 retn=0 gapG/S=160/160 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::HideLoadModuleDebugMenu [medium] gog 0x4dbba0; vote; retn=0
callers=2 callees=00410670=gog:CSWGuiManager::RemovePanel, 0073fee0=CClientExoApp::SetInputClass, 00410800=GuiManager::Contains, 007400a0=CClientExoApp::GetMiniGame, 004109c0=gog:CSWGuiManager::PopModalPanel
```
void __fastcall FUN_007cb4a0(int param_1)
{
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x70));
    if (iVar1 != 0) {
      FUN_004109c0();
      FUN_00410670(*(undefined4 *)(param_1 + 0x70));
      iVar1 = FUN_007400a0();
      if (iVar1 == 0) {
        FUN_0073fee0(0,1);
      }
      else {
        FUN_0073fee0(1,1);
      }
    }
  }
  return;
}
```

### CGuiInGame::HidePowersFeatsSkillsDebugMenu  gog=0x4dbc40 (param_bytes=0, notes='void')
steam: cand=0x007cb540(mapper) delta=0x2ef900 align=6/6 size=149 retn=0 gapG/S=160/160 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::HidePowersFeatsSkillsDebugMenu [medium] gog 0x4dbc40; vote; retn=0
callers=2 callees=00410670=gog:CSWGuiManager::RemovePanel, 0073fee0=CClientExoApp::SetInputClass, 00410800=GuiManager::Contains, 007400a0=CClientExoApp::GetMiniGame, 004109c0=gog:CSWGuiManager::PopModalPanel
```
void __fastcall FUN_007cb540(int param_1)
{
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x74));
    if (iVar1 != 0) {
      FUN_004109c0();
      FUN_00410670(*(undefined4 *)(param_1 + 0x74));
      iVar1 = FUN_007400a0();
      if (iVar1 == 0) {
        FUN_0073fee0(0,1);
      }
      else {
        FUN_0073fee0(1,1);
      }
    }
  }
  return;
}
```

### CGuiInGame::IsCameraDialog  gog=0x4dbda0 (param_bytes=16, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x007cb6a0(mapper) delta=0x2ef900 align=6/6 size=55 retn=16 gapG/S=64/64 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::IsCameraDialog [medium] gog 0x4dbda0; vote; retn=16
callers=2 callees=008bb0e0
```
void __thiscall
FUN_007cb6a0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            )
{
  if (*(int *)(param_1 + 0x68) != 0) {
    FUN_008bb0e0(param_2,param_3,param_4,param_5);
  }
  return;
}
```

### CGuiInGame::IsAnimationPlayingInDialog  gog=0x4dbde0 (param_bytes=0, notes='void')
steam: cand=0x007cb6e0(mapper) delta=0x2ef900 align=6/6 size=55 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::IsAnimationPlayingInDialog [medium] gog 0x4dbde0; vote; retn=0
callers=1 callees=
```
void __fastcall FUN_007cb6e0(int param_1)
{
  if (*(int *)(param_1 + 100) != 0) {
    *(undefined4 *)(param_1 + 0xfb4) = 1;
    (**(code **)(**(int **)(param_1 + 0x3c) + 0x84))();
  }
  return;
}
```

### CGuiInGame::IsFadingInDialog  gog=0x4dbe50 (param_bytes=0, notes='void')
steam: cand=0x007cb750(mapper) delta=0x2ef900 align=6/6 size=43 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::IsFadingInDialog [medium] gog 0x4dbe50; vote; retn=0
callers=7 callees=
```
undefined4 __fastcall FUN_007cb750(int param_1)
{
  if ((*(int *)(param_1 + 0x3c) == 0) || (*(int *)(param_1 + 0x3c) != *(int *)(param_1 + 0x40))) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### CGuiInGame::ShowPartySelection  gog=0x4dc020 (param_bytes=20, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x007cb920(mapper) delta=0x2ef900 align=6/6 size=538 retn=20 gapG/S=544/544 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* exitScript, int requireAccept, int forcedNPC1, int forcedNPC2) [param_bytes=16]
name_map: CGuiInGame::ShowPartySelection [medium] gog 0x4dc020; vote; retn=20
callers=1 callees=0089fb40, 005453c0=CSWSObject::GetArea, 00410800=GuiManager::Contains, 0070bc60, 0051c8d0=CServerExoApp::GetPartyTable (inline at *(this+4)+, 0089cf30, 0051c100=CServerExoApp::GetCreatureByGameObjectID, 004122a0, 00410530=gog:CSWGuiManager::AddPanel, 0073f810=CClientExoApp::GetSWCMessage, 0087a730, 0073fee0=CClientExoApp::SetInputClass, 00919723=operator new, 007337c0=CExoString::operator= ...
```
void __thiscall
FUN_007cb920(int param_1,undefined4 param_2,int param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6)
{
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969b2b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = FUN_0051c8d0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (*(int *)(iVar1 + 0x118) == -1) {
      uVar2 = FUN_0051c8f0();
      iVar1 = FUN_0051c100(uVar2);
      if (iVar1 != 0) {
        uVar2 = FUN_0051c8f0();
        FUN_0051c100(uVar2);
        iVar1 = FUN_005453c0();
        if (iVar1 != 0) {
          uVar2 = FUN_0051c8f0();
          FUN_0051c100(uVar2);
          iVar1 = FUN_005453c0();
          if ((*(int *)(iVar1 + 0x2dc) == 0) || (param_3 != 0)) {
            iVar1 = CMFCVisualManagerBitmapCache::IsCached
                              (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                               *(CSize **)(param_1 + 0x1c));
            if (iVar1 == 0) {
              if (param_3 != 0) {
                if (*(int *)(param_1 + 0xf18) == 0) {
                  FUN_0073f810();
                  FUN_0087a730();
                }
                FUN_004122a0(4);
                FUN_0070bc60(DAT_0099c2b8,0);
              }
              if (*(int *)(param_1 + 0x1c) == 0) {
                iVar1 = FUN_00919723(0x50b4);
                local_8 = 0;
                if (iVar1 == 0) {
                  local_48 = 0;
                }
                else {
                  local_48 = FUN_0089cf30(*(undefined4 *)(param_1 + 0x38));
                }
                local_8 = 0xffffffff;
... (19 more lines)
```

### CGuiInGame::StopGlobalFade  gog=0x4dc2f0 (param_bytes=0, notes='void')
steam: cand=0x007cbbf0(mapper) delta=0x2ef900 align=6/6 size=69 retn=0 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::StopGlobalFade [medium] gog 0x4dc2f0; vote; retn=0
callers=2 callees=00410670=gog:CSWGuiManager::RemovePanel, 007bc7f0
```
void __fastcall FUN_007cbbf0(int param_1)
{
  if ((*(int *)(param_1 + 0x6c) != 0) && (*(int *)(param_1 + 0x1024) == 0)) {
    FUN_007bc7f0();
    FUN_00410670(*(undefined4 *)(param_1 + 0x6c));
  }
  return;
}
```

### CGuiInGame::IsGlobalFading  gog=0x4dc340 (param_bytes=0, notes='void')
steam: cand=0x007cbc40(mapper) delta=0x2ef900 align=5/6 size=44 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::IsGlobalFading [medium] gog 0x4dc340; vote; retn=0
callers=4 callees=007bca60
```
undefined4 __fastcall FUN_007cbc40(int param_1)
{
  if ((*(int *)(param_1 + 0x6c) != 0) && (iVar1 = FUN_007bca60(), iVar1 == 0)) {
    return 1;
  }
  return 0;
}
```

### CGuiInGame::HideStoreGui  gog=0x4dd630 (param_bytes=0, notes='void')
steam: cand=0x007ccf30(mapper) delta=0x2ef900 align=5/6 size=258 retn=0 gapG/S=272/272 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::HideStoreGui [medium] gog 0x4dd630; vote; retn=0
callers=1 callees=00740330=CClientExoApp::GetPausedByCombat, 00410670=gog:CSWGuiManager::RemovePanel, 004122a0, 00740350=CClientExoApp::SetPausedByCombat, 0073fee0=CClientExoApp::SetInputClass, 007c9c40=CGuiInGame::SetSWGuiStatus, 0070bc60
```
void __fastcall FUN_007ccf30(int param_1)
{
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = FUN_00740330();
    if (iVar1 != 0) {
      FUN_00740350(0,0,0);
    }
    FUN_007c9c40(4,1);
    FUN_0073fee0(0,1);
    if (DAT_00a10760 == 0) {
      iVar1 = *(int *)(param_1 + 0x84);
      *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xffffff7f | 0x80;
      *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xfffffcff | 0x200;
      *(undefined4 *)(param_1 + 0x84) = 0;
    }
    else {
      FUN_00410670(*(undefined4 *)(param_1 + 0x84));
    }
    FUN_004122a0(5);
    FUN_0070bc60(DAT_0099c2a8,0);
  }
  return;
}
```

### CGuiInGame::UpdateMessageGUI  gog=0x4dd740 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007cd040(mapper) delta=0x2ef900 align=6/6 size=103 retn=4 gapG/S=144/112 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::UpdateMessageGUI [medium] gog 0x4dd740; vote; retn=4
callers=2 callees=00759820, 007592f0, 00758e30
```
void __thiscall FUN_007cd040(int param_1,int param_2)
{
  if (param_2 != 0) {
    FUN_00758e30(*(undefined4 *)(param_1 + 0x110),*(undefined4 *)(param_1 + 0x11c));
    FUN_007592f0(*(undefined4 *)(param_1 + 0x114),*(undefined4 *)(param_1 + 0x120));
    FUN_00759820(*(undefined4 *)(param_1 + 0x118),*(undefined4 *)(param_1 + 0x124));
  }
  return;
}
```

### CGuiInGame::UpdateDialog  gog=0x4dd7f0 (param_bytes=0, notes='void')
steam: cand=0x007cd0f0(mapper) delta=0x2ef900 align=6/6 size=977 retn=0 gapG/S=992/992 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::UpdateDialog [medium] gog 0x4dd7f0; vote; retn=0
callers=3 callees=007bff30=CGuiInGame::InitializeDialogAnimationData, 0051c8b0=gog:CServerExoApp::ClientToServerObjectId, 0051c0a0=CServerExoApp::GetGameObject, 007c0030, 0073f430=CClientExoApp::GetModule, 008ba6c0, 0073f4d0=ClientApp::GetClientObject, 0051c600=CServerExoApp::GetPlayerByControlledObjectId, 00546710=CSWSObject::ReplyDialog, 007be2d0=CGuiInGame::AppendToDialogBuffer, 007c84e0=CGuiInGame::SetDialogAnimations, 0051c8f0=gog:CServerExoApp::GetPlayerCreatureId, 007c7d60=CGuiInGame::HandleDialogSelection
```
void __fastcall FUN_007cd0f0(int param_1)
{
  if (*(int *)(param_1 + 0xcc) != 0) {
    if (*(int *)(param_1 + 0xffc) == 1) {
      iVar2 = FUN_0073f430();
      if (*(int **)(iVar2 + 0x40) != (int *)0x0) {
        (**(code **)(**(int **)(iVar2 + 0x40) + 0x44))(*(undefined4 *)(param_1 + 0x1000));
        *(undefined4 *)(param_1 + 0xffc) = 0;
      }
    }
    if (*(int *)(param_1 + 0xe8) != 0) {
      iVar2 = *(int *)(param_1 + 0xe0);
      local_14 = *(undefined4 *)(param_1 + 400);
      if (*(int *)(param_1 + 0x140) != 0) {
        if (*(int *)(*(int *)(param_1 + 0x140) + iVar2 * 4) != 0x7f000000) {
          local_14 = *(undefined4 *)(*(int *)(param_1 + 0x140) + iVar2 * 4);
        }
        uVar5 = *(undefined4 *)(param_1 + 0x1a4);
        piVar3 = (int *)FUN_0073f4d0(local_14);
        if ((((piVar3 != (int *)0x0) && (iVar4 = (**(code **)(*piVar3 + 0xc))(), iVar4 != 0)) &&
            (piVar3 = (int *)FUN_0073f4d0(uVar5), piVar3 != (int *)0x0)) &&
           (iVar4 = (**(code **)(*piVar3 + 0xc))(), iVar4 != 0)) {
          for (local_1c = 0; local_1c < *(int *)(*(int *)(param_1 + 0x174) + iVar2 * 4);
              local_1c = local_1c + 1) {
            piVar3 = (int *)FUN_0073f4d0(*(undefined4 *)
                                          (*(int *)(*(int *)(param_1 + 0x178) + iVar2 * 4) +
                                          local_1c * 4));
            if (piVar3 == (int *)0x0) {
              return;
            }
            iVar4 = (**(code **)(*piVar3 + 0xc))();
            if (iVar4 == 0) {
              return;
            }
          }
          FUN_007be2d0(*(int *)(param_1 + 0x138) + *(int *)(param_1 + 0xe0) * 8,uVar5);
          if (*(int *)(param_1 + 0xec) == 0) {
            FUN_007c7d60(*(undefined4 *)(param_1 + 0xe0),*(undefined4 *)(param_1 + 0xe4));
          }
          else {
            if (*(int *)(param_1 + 0xe0) < 0) {
              local_7c = 0xffffffff;
            }
            else {
              local_7c = FUN_008ba6c0(*(undefined4 *)(param_1 + 0xe0));
... (33 more lines)
```

### CGuiInGame::ShowSoloModeQuery  gog=0x4ddc10 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007cd510(mapper) delta=0x2ef900 align=5/6 size=578 retn=4 gapG/S=592/592 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int doStealth) [param_bytes=4]
name_map: CGuiInGame::ShowSoloModeQuery [medium] gog 0x4ddc10; vote; retn=4
callers=3 callees=0073f450=CClientExoApp::GetPlayerCreature, 00740020=CClientExoApp::GetPlayerCharacterSet, 0073f430=CClientExoApp::GetModule, 0077d800=Party entry -> creature thiscall(entry); creature+, 00410800=GuiManager::Contains, 007aae00=CSWCArea::GetServerArea, 0073f750=ClientApp::GetGuiInGame, 004122a0, 0073fb90=ClientApp -> party (*(*(clientApp+4)+0x270)) thisc, 00410530=gog:CSWGuiManager::AddPanel, 0073f810=CClientExoApp::GetSWCMessage, 008b9120, 0087a730, 0073fee0=CClientExoApp::SetInputClass
```
void __thiscall FUN_007cd510(int param_1,undefined4 param_2)
{
  if (*(int *)(param_1 + 0x128) == 0) {
    return;
  }
  if (*(int *)(param_1 + 0x38) == 0) {
    return;
  }
  if (*(int *)(param_1 + 0x94) == 0) {
    return;
  }
  piVar4 = (int *)FUN_0073fb90();
  bVar2 = false;
  if (piVar4 != (int *)0x0) {
    bVar2 = 1 < *piVar4;
  }
  iVar5 = FUN_0073f450();
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
... (19 more lines)
```

### CGuiInGame::HideSoloMode  gog=0x4dde60 (param_bytes=0, notes='void')
steam: cand=0x007cd760(mapper) delta=0x2ef900 align=5/6 size=127 retn=0 gapG/S=128/128 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::HideSoloMode [medium] gog 0x4dde60; vote; retn=0
callers=1 callees=00740350=CClientExoApp::SetPausedByCombat, 0073fee0=CClientExoApp::SetInputClass, 0075baa0, 00410800=GuiManager::Contains, 004109c0=gog:CSWGuiManager::PopModalPanel
```
void __fastcall FUN_007cd760(int param_1)
{
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0x94))
  ;
  if (iVar1 != 0) {
    if (*(int *)(param_1 + 0xf18) == 0) {
      FUN_00740350(0,0,0);
    }
    FUN_004109c0();
    FUN_0075baa0(0);
    FUN_0073fee0(0,1);
  }
  return;
}
```

### CGuiInGame::ShowUpgradeScreen  gog=0x4ddee0 (param_bytes=20, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
NO CANDIDATE (bracketing failed) -- see manual list

### CGuiInGame::HideUpgradeScreen  gog=0x4de2e0 (param_bytes=0, notes='void')
steam: cand=0x007cdbe0(mapper) delta=0x2ef900 align=5/6 size=217 retn=0 gapG/S=224/224 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::HideUpgradeScreen [medium] gog 0x4de2e0; vote; retn=0
callers=2 callees=0073f810=CClientExoApp::GetSWCMessage, 0087a730, 0073fee0=CClientExoApp::SetInputClass, 0070bc60
```
void __fastcall FUN_007cdbe0(int param_1)
{
  if (((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x38) != 0)) &&
     (*(int *)(param_1 + 0x88) != 0)) {
    if (*(int *)(param_1 + 0xf18) == 0) {
      FUN_0073f810();
      FUN_0087a730();
    }
    FUN_0073fee0(0,1);
    FUN_0070bc60(DAT_0099c2a8,0);
    iVar1 = *(int *)(param_1 + 0x88);
    *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xffffff7f;
    *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xfffffcff | 0x200;
    *(undefined4 *)(param_1 + 0x88) = 0;
  }
  return;
}
```

### CGuiInGame::UpdateAreaTransitionGUI  gog=0x4dee80 (param_bytes=0, notes='void')
steam: cand=0x007ce780(mapper) delta=0x2ef900 align=6/6 size=489 retn=0 gapG/S=496/496 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::UpdateAreaTransitionGUI [medium] gog 0x4dee80; vote; retn=0
callers=1 callees=008b88b0, 005142f0, 004452b0=Vector::operator- (Vector3 subtract), 00410530=gog:CSWGuiManager::AddPanel, 00410780=gog:CSWGuiManager::SendPanelToBack, 0073f430=CClientExoApp::GetModule, 007ce970=CGuiInGame::HideAreaTransition, 00410800=GuiManager::Contains, 007aae00=CSWCArea::GetServerArea
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
void __fastcall FUN_007ce780(int param_1)
{
  float10 fVar3;
  undefined1 local_28 [12];
  local_14 = 0;
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  if (((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x38) != 0)) &&
     (*(int *)(param_1 + 0x9c) != 0)) {
    iVar1 = FUN_0073f430();
    if (iVar1 != 0) {
      iVar1 = FUN_0073f430();
      local_1c = *(int *)(iVar1 + 0x48);
      if (local_1c != 0) {
        local_8 = FUN_007aae00();
      }
    }
    if ((local_8 == 0) || (*(int *)(param_1 + 0x34) != 1)) {
      FUN_007ce970();
    }
    else {
      iVar1 = *(int *)(param_1 + 0x9c);
      local_14 = *(undefined4 *)(iVar1 + 0x440);
      local_10 = *(undefined4 *)(iVar1 + 0x444);
      local_c = *(undefined4 *)(iVar1 + 0x448);
      local_18 = *(int *)(*(int *)(param_1 + 0x9c) + 0x44c);
      if (local_18 == *(int *)(param_1 + 0x1018)) {
        uVar2 = FUN_004452b0(local_28,&local_14,param_1 + 0x100c);
        fVar3 = (float10)FUN_005142f0(uVar2);
        if (fVar3 < (float10)_DAT_0098c1e8) {
          return;
        }
      }
      if (*(int *)(param_1 + 0x1018) == 0x7f000000) {
        FUN_007ce970();
      }
      else if (*(int *)(local_8 + 0x2f8) == 0) {
        FUN_008b88b0(*(undefined4 *)(param_1 + 0x1018),param_1 + 0x100c);
        iVar1 = CMFCVisualManagerBitmapCache::IsCached
                          (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                           *(CSize **)(param_1 + 0x9c));
        if (iVar1 == 0) {
          FUN_00410530(*(undefined4 *)(param_1 + 0x9c),4,1);
... (15 more lines)
```

### CGuiInGame::HideAreaTransition  gog=0x4df070 (param_bytes=0, notes='void')
steam: cand=0x007ce970(mapper) delta=0x2ef900 align=6/6 size=281 retn=0 gapG/S=288/288 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::HideAreaTransition [medium] gog 0x4df070; vote; retn=0
callers=1 callees=008b88b0, 00410670=gog:CSWGuiManager::RemovePanel, 00410780=gog:CSWGuiManager::SendPanelToBack, 0073f430=CClientExoApp::GetModule, 00410800=GuiManager::Contains, 007aae00=CSWCArea::GetServerArea
```
void __fastcall FUN_007ce970(int param_1)
{
  local_8 = 0;
  iVar1 = FUN_0073f430();
  if (iVar1 != 0) {
    iVar1 = FUN_0073f430();
    local_c = *(int *)(iVar1 + 0x48);
    if (((local_c != 0) && (local_8 = FUN_007aae00(), local_8 != 0)) &&
       (*(int *)(local_8 + 0x2f8) != 0)) {
      iVar1 = CMFCVisualManagerBitmapCache::IsCached
                        (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                         *(CSize **)(param_1 + 0x9c));
      if (iVar1 == 0) {
        return;
      }
      FUN_00410780(*(undefined4 *)(param_1 + 0x9c));
      return;
    }
  }
  if (((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x38) != 0)) &&
     (*(int *)(param_1 + 0x9c) != 0)) {
    local_18 = 0;
    local_14 = 0;
    local_10 = 0;
    FUN_008b88b0(0x7f000000,&local_18);
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x9c));
    if (iVar1 != 0) {
      FUN_00410670(*(undefined4 *)(param_1 + 0x9c));
    }
  }
  return;
}
```

### CGuiInGame::GetBarkBubbleVisible  gog=0x4df190 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007cea90(mapper) delta=0x2ef900 align=6/6 size=84 retn=4 gapG/S=96/96 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, int* outBottomY) [param_bytes=4]
name_map: CGuiInGame::GetBarkBubbleVisible [medium] gog 0x4df190; vote; retn=4
callers=1 callees=00410800=GuiManager::Contains
```
undefined4 __thiscall FUN_007cea90(int param_1,int *param_2)
{
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

### CGuiInGame::GetCanOrientInDialog  gog=0x4df1f0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007ceaf0(mapper) delta=0x2ef900 align=6/6 size=90 retn=4 gapG/S=96/96 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD clientId) [param_bytes=4]
name_map: CGuiInGame::GetCanOrientInDialog [medium] gog 0x4df1f0; vote; retn=4
callers=2 callees=
```
undefined4 * __thiscall FUN_007ceaf0(int param_1,undefined4 *param_2)
{
  if (*(int *)(param_1 + 0x9c) == 0) {
    *param_2 = 0;
    param_2[1] = 0;
    param_2[2] = 0;
  }
  else {
    iVar1 = *(int *)(param_1 + 0x9c);
    *param_2 = *(undefined4 *)(iVar1 + 0x440);
    param_2[1] = *(undefined4 *)(iVar1 + 0x444);
    param_2[2] = *(undefined4 *)(iVar1 + 0x448);
  }
  return param_2;
}
```

### CGuiInGame::RePopulateMainInterface  gog=0x4df250 (param_bytes=0, notes='void')
steam: cand=0x007ceb50(mapper) delta=0x2ef900 align=5/6 size=39 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::RePopulateMainInterface [medium] gog 0x4df250; vote; retn=0
callers=2 callees=0074cfe0=CSWGuiMainInterface::UpdateTargetMenu
```
void __fastcall FUN_007ceb50(int param_1)
{
  if (*(int *)(param_1 + 0x98) != 0) {
    FUN_0074cfe0(1);
  }
  return;
}
```

### CGuiInGame::AddFloatyText  gog=0x4df580 (param_bytes=16, notes='undefined4 param_1, undefined4 param_2, undefined4 param_3, undefined4')
steam: cand=0x007cee80(mapper) delta=0x2ef900 align=5/6 size=58 retn=16 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, DWORD clientId, void* text, Vector* color, float duration) [param_bytes=16]
name_map: CGuiInGame::AddFloatyText [medium] gog 0x4df580; vote; retn=16
callers=1 callees=00750f30
```
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

### CGuiInGame::GetPendingStatusSummary  gog=0x4df910 (param_bytes=0, notes='void')
steam: cand=0x007cf210(mapper) delta=0x2ef900 align=6/6 size=84 retn=0 gapG/S=96/96 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::GetPendingStatusSummary [medium] gog 0x4df910; vote; retn=0
callers=1 callees=
```
undefined4 __fastcall FUN_007cf210(int param_1)
{
  if (*(int *)(param_1 + 0xb8) == 0) {
    local_c = 0;
  }
  else if (((*(uint *)(*(int *)(param_1 + 0xb8) + 0x68) & 1) == 0) &&
          ((*(uint *)(*(int *)(param_1 + 0xb8) + 0x68) >> 1 & 1) == 0)) {
    local_c = 0;
  }
  else {
    local_c = 1;
  }
  return local_c;
}
```

### CGuiInGame::ShowStatusSummary  gog=0x4df970 (param_bytes=0, notes='void')
steam: cand=0x007cf270(mapper) delta=0x2ef900 align=6/6 size=318 retn=0 gapG/S=320/320 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::ShowStatusSummary [medium] gog 0x4df970; vote; retn=0
callers=1 callees=0072fb00=OptionsHolder::GetGameOptions (returns *(*(this+4), 00410530=gog:CSWGuiManager::AddPanel, 00740350=CClientExoApp::SetPausedByCombat, 0073fee0=CClientExoApp::SetInputClass, 0075db70, 0075fbc0, 0075fc30=PlayerCreated one-shot k_level_dlg on CanLevelUp, 00740ec0=CClientExoApp::GetTogglePauseAtEndOfFrame, 007628a0
```
void __fastcall FUN_007cf270(int param_1)
{
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
    FUN_00410530(*(undefined4 *)(param_1 + 0xb8),1,1);
    FUN_0073fee0(2,0);
    return;
  }
  FUN_0075fbc0();
  FUN_0075fc30();
  FUN_007628a0(0);
  FUN_0075db70();
  return;
}
```

### CGuiInGame::GetCanHeadFollowInDialog  gog=0x4dfff0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007cf8f0(mapper) delta=0x2ef900 align=3/6 size=112 retn=4 gapG/S=112/112 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD objectId) [param_bytes=4]
name_map: CGuiInGame::GetCanHeadFollowInDialog [medium] gog 0x4dfff0; vote; retn=4
callers=1 callees=00521060, 007a1720, 0058c370
```
undefined4 __thiscall FUN_007cf8f0(undefined4 param_1,int param_2)
{
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

### CGuiInGame::ResetInterfaceForSize  gog=0x4e03a0 (param_bytes=0, notes='void')
steam: cand=0x007cfca0(mapper) delta=0x2ef900 align=6/6 size=1287 retn=0 gapG/S=1296/1296 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::ResetInterfaceForSize [medium] gog 0x4e03a0; vote; retn=0
callers=1 callees=0073fea0=CClientExoApp::GetSWGuiManager, 00747210, 0040ea00=gog:CSWGuiPanel::CenterPanel, 0075ae40=CSWGuiMessageBox::Constructor, 0089c5f0, 00919723=operator new, 00898ca0, 008bba80=CSWGuiDialogCinematic::Constructor
```
void __fastcall FUN_007cfca0(int param_1)
{
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969cf7;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x98) != 0) {
    if (*(undefined4 **)(param_1 + 0x98) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x98))(1,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    }
    iVar1 = FUN_00919723(0x15418);
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
    iVar1 = FUN_00919723(0x2bc0);
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
    FUN_0040ea00();
  }
  if (*(int *)(param_1 + 0x70) != 0) {
    FUN_0040ea00();
  }
... (72 more lines)
```

### CGuiInGame::ChangeCharacter  gog=0x4e09b0 (param_bytes=0, notes='void')
steam: cand=0x007d02b0(mapper) delta=0x2ef900 align=6/6 size=25 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::ChangeCharacter [medium] gog 0x4e09b0; vote; retn=0
callers=1 callees=00750e60
```
void __fastcall FUN_007d02b0(undefined4 param_1)
{
  FUN_00750e60(param_1);
  return;
}
```

### CGuiInGame::GetMiniMapVisible  gog=0x4e0c60 (param_bytes=0, notes='void')
steam: cand=0x007d0560(mapper) delta=0x2ef900 align=6/6 size=49 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::GetMiniMapVisible [medium] gog 0x4e0c60; vote; retn=0
callers=1 callees=
```
void __fastcall FUN_007d0560(int param_1)
{
  if (*(int *)(param_1 + 0x3c) == *(int *)(param_1 + 0x40)) {
    (**(code **)(**(int **)(param_1 + 0x3c) + 0x84))();
  }
  return;
}
```

### CGuiInGame::RemoveGUIPopUps  gog=0x4e0ca0 (param_bytes=0, notes='void')
steam: cand=0x007d05a0(mapper) delta=0x2ef900 align=6/6 size=120 retn=0 gapG/S=128/128 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::RemoveGUIPopUps [medium] gog 0x4e0ca0; vote; retn=0
callers=1 callees=0072fb00=OptionsHolder::GetGameOptions (returns *(*(this+4), 007400a0=CClientExoApp::GetMiniGame
```
uint FUN_007d05a0(void)
{
  uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  iVar1 = FUN_0072fb00();
  if ((*(uint *)(iVar1 + 0x1c) >> 3 & 1) == 0) {
    uVar2 = 0;
  }
  else {
    iVar3 = extraout_var;
    iVar1 = FUN_007400a0(extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 4),iVar1,uVar4);
    if (iVar1 == 0) {
      if (*(int *)(iVar3 + 0x98) == 0) {
        uVar2 = 1;
      }
      else {
        uVar2 = *(uint *)(*(int *)(iVar3 + 0x98) + 0x59b4) >> 1 & 1;
      }
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### CGuiInGame::GetCanClick  gog=0x4e1290 (param_bytes=0, notes='void')
steam: cand=0x007d0b90(mapper) delta=0x2ef900 align=6/6 size=153 retn=0 gapG/S=160/160 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::GetCanClick [medium] gog 0x4e1290; vote; retn=0
callers=2 callees=0073f3b0=CClientExoApp::GetWorldTimer, 0051af60=gog:CWorldTimer::SubtractWorldTimes, 0051ad20=gog:CWorldTimer::GetWorldTime
```
undefined4 FUN_007d0b90(void)
{
  local_10 = 0;
  uVar1 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  puVar6 = &local_c;
  puVar4 = &local_14;
  FUN_0073f3b0(puVar4,puVar6);
  iVar8 = extraout_var;
  FUN_0051ad20(puVar4,puVar6);
  puVar7 = &local_10;
  piVar5 = &local_8;
  uVar3 = *(undefined4 *)(iVar8 + 0xfd0);
  uVar2 = *(undefined4 *)(iVar8 + 0xfcc);
  FUN_0073f3b0(local_14,local_c,uVar2,uVar3,piVar5,puVar7,iVar8,*(undefined4 *)(DAT_00a1b4a4 + 4),
               uVar1);
  FUN_0051af60(local_14,local_c,uVar2,uVar3,piVar5,puVar7);
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

### CGuiInGame::UpdateDialogLipSync  gog=0x4e1370 (param_bytes=0, notes='void')
steam: cand=0x007d0c70(mapper) delta=0x2ef900 align=6/6 size=226 retn=0 gapG/S=240/240 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::UpdateDialogLipSync [medium] gog 0x4e1370; vote; retn=0
callers=1 callees=0078cf90=CClientExoAppInternal::ContinueMessageProcessing, 0070b0e0, 00710eb0, 0073f4d0=ClientApp::GetClientObject
```
void __fastcall FUN_007d0c70(int param_1)
{
  undefined1 local_18 [16];
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

### CGuiInGame::GetFadeOverridePanelVisible  gog=0x4e1490 (param_bytes=0, notes='void')
steam: cand=0x007d0d90(mapper) delta=0x2ef900 align=6/6 size=301 retn=0 gapG/S=304/304 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::GetFadeOverridePanelVisible [medium] gog 0x4e1490; vote; retn=0
callers=1 callees=00740f20=CClientExoApp::GetPazaakIsRunning, 00410800=GuiManager::Contains
```
undefined4 __fastcall FUN_007d0d90(int param_1)
{
  if ((*(int *)(param_1 + 0x128) == 0) || (*(int *)(param_1 + 0x38) == 0)) {
    local_10 = 0;
  }
  else if (((((*(int *)(param_1 + 0x80) == 0) ||
             (iVar1 = CMFCVisualManagerBitmapCache::IsCached
                                (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                                 *(CSize **)(param_1 + 0x80)), iVar1 == 0)) ||
            ((*(uint *)(*(int *)(param_1 + 0x80) + 0x48) >> 2 & 1) == 0)) &&
           (((*(int *)(param_1 + 0x78) == 0 ||
             (iVar1 = CMFCVisualManagerBitmapCache::IsCached
                                (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                                 *(CSize **)(param_1 + 0x78)), iVar1 == 0)) ||
            ((*(uint *)(*(int *)(param_1 + 0x78) + 0x48) >> 2 & 1) == 0)))) &&
          ((((*(int *)(param_1 + 0x84) == 0 ||
             (iVar1 = CMFCVisualManagerBitmapCache::IsCached
                                (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                                 *(CSize **)(param_1 + 0x84)), iVar1 == 0)) ||
            ((*(uint *)(*(int *)(param_1 + 0x84) + 0x48) >> 2 & 1) == 0)) &&
           ((((*(int *)(param_1 + 8) == 0 ||
              (iVar1 = CMFCVisualManagerBitmapCache::IsCached
                                 (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                                  *(CSize **)(param_1 + 8)), iVar1 == 0)) ||
             ((*(uint *)(*(int *)(param_1 + 8) + 0x48) >> 2 & 1) == 0)) &&
            (iVar1 = FUN_00740f20(), iVar1 == 0)))))) {
    local_10 = 0;
  }
  else {
    local_10 = 1;
  }
  return local_10;
}
```

### CGuiInGame::FadeAndStopDialogAmbientTrack  gog=0x4e15c0 (param_bytes=0, notes='void')
steam: cand=0x007d0ec0(mapper) delta=0x2ef900 align=6/6 size=142 retn=0 gapG/S=144/144 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::FadeAndStopDialogAmbientTrack [medium] gog 0x4e15c0; vote; retn=0
callers=2 callees=0070b080=StreamingSource scalar delete, 009196fd=operator delete, 00741010=CClientExoApp::FadeAndStopStreamingSoundSource
```
void __fastcall FUN_007d0ec0(int param_1)
{
  if (*(int *)(param_1 + 0xfbc) != 0) {
    FUN_00741010(*(undefined4 *)(param_1 + 0xfbc),500);
    iVar1 = *(int *)(param_1 + 0xfbc);
    if (iVar1 != 0) {
      FUN_0070b080();
      FUN_009196fd(iVar1);
    }
    *(undefined4 *)(param_1 + 0xfbc) = 0;
  }
  return;
}
```

### CGuiInGame::DoQuickLoad  gog=0x4e1650 (param_bytes=0, notes='void')
steam: cand=0x007d0f50(mapper) delta=0x2ef900 align=6/6 size=1503 retn=0 gapG/S=1504/1504 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CGuiInGame::DoQuickLoad [medium] gog 0x4e1650; vote; retn=0
callers=1 callees=00714f30=CExoResMan::CreateDirectory2, 0071e910, 007d0af0=CGuiInGame::SetGlobalUnpauseableState, 007d1530=CGuiInGame::UnpackQuickSaveGame, 00711710=CExoResMan::RemoveResourceDirectory, 00740d90, 0073f750=ClientApp::GetGuiInGame, 00740980=CClientExoApp::SetLoadScreenHint, 0071ebd0, 0071dee0, 0040df30, 007337c0=CExoString::operator=, 0040def0, 007156e0=CExoResMan::CleanDirectory ...
strings: SAVES: | TEMP:
```
void __fastcall FUN_007d0f50(int param_1)
{
  undefined1 local_5c [19];
  undefined1 local_48 [8];
  undefined1 local_3c [8];
  SchedulerResourceManagement *local_30;
  undefined1 local_1c [4];
  void *local_10;
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969df6;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0040def0();
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  FUN_00733570();
  local_8._0_1_ = 2;
  FUN_00736050(local_1c,local_48,0xffff);
  local_8._0_1_ = 1;
  FUN_00733780();
  FUN_00733570();
  local_8._0_1_ = 3;
  iVar1 = FUN_00714f30();
  local_49 = '\x01' - (iVar1 != 0);
  local_8._0_1_ = 1;
  FUN_00733780();
  if (local_49 != '\0') {
    FUN_00733570();
    local_8._0_1_ = 4;
    FUN_007156e0(local_5c);
    local_8._0_1_ = 1;
    FUN_00733780();
  }
  local_20 = 0;
  while( true ) {
    if (local_18 <= local_20) {
      FUN_00733570();
      local_8._0_1_ = 9;
      FUN_00711710();
      local_8._0_1_ = 1;
      FUN_00733780();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
... (96 more lines)
```

