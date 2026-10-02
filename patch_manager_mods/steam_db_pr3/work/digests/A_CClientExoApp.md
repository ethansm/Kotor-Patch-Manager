# Digest A_CClientExoApp

### CClientExoApp::GetObjectName  gog=0x40c5a0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x0073f0e0(mapper) delta=0x332b40 align=6/6 size=32 retn=8 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD clientId, void* outName) [param_bytes=8]
name_map: CClientExoApp::GetObjectName [medium] gog 0x40c5a0; vote; retn=8
callers=1 callees=0078eba0=CClientExoAppInternal::FadeAndStopStreamingSoundSo
```
void FUN_0073f0e0(undefined4 param_1,undefined4 param_2)
{
  FUN_0078eba0(param_1,param_2);
  return;
}
```

### CClientExoApp::DisplayMainMenu  gog=0x40c6f0 (param_bytes=4, notes='CClientExoApp * this, int param_1')
steam: cand=0x0073f230(mapper) delta=0x332b40 align=6/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::DisplayMainMenu [medium] gog 0x40c6f0; vote; retn=4
callers=3 callees=00781590
```
void FUN_0073f230(undefined4 param_1)
{
  FUN_00781590(param_1);
  return;
}
```

### CClientExoApp::ExitProgram  gog=0x40c750 (param_bytes=0, notes='void')
steam: cand=0x0073f290(mapper) delta=0x332b40 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__stdcall) () [param_bytes=0]
name_map: CClientExoApp::ExitProgram [medium] gog 0x40c750; vote; retn=0
callers=2 callees=00785360=CClientExoAppInternal::ExitProgram
```
void __fastcall FUN_0073f290(undefined4 param_1)
{
  FUN_00785360(param_1);
  return;
}
```

### CClientExoApp::GetClientLanguage  gog=0x40c810 (param_bytes=0, notes='void')
steam: cand=0x0073f350(mapper) delta=0x332b40 align=6/6 size=31 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetClientLanguage [medium] gog 0x40c810; vote; retn=0
callers=34 callees=
```
undefined4 __fastcall FUN_0073f350(int param_1)
{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x148);
}
```

### CClientExoApp::GetWorldTimer  gog=0x40c870 (param_bytes=0, notes='void')
steam: cand=0x0073f3b0(mapper) delta=0x332b40 align=6/6 size=28 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetWorldTimer [medium] gog 0x40c870; vote; retn=0
callers=46 callees=
```
undefined4 __fastcall FUN_0073f3b0(int param_1)
{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x24);
}
```

### CClientExoApp::GetPlayerCreature  gog=0x40c910 (param_bytes=0, notes='void')
steam: cand=0x0073f450(mapper) delta=0x332b40 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetPlayerCreature [medium] gog 0x40c910; vote; retn=0
callers=57 callees=0078cee0
```
void __fastcall FUN_0073f450(undefined4 param_1)
{
  FUN_0078cee0(param_1);
  return;
}
```

### CClientExoApp::GetGameObject  gog=0x40c990 (param_bytes=4, notes='CClientExoApp * this, ulong param_1')
steam: cand=0x0073f4d0(mapper) delta=0x332b40 align=6/6 size=59 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, DWORD clientId) [param_bytes=4]
name_map: ClientApp::GetClientObject [medium] decompile+asm from full dump (dump_excerpts/), Opus read 2026-09-30
callers=96 callees=0053dfb0=CGameObjectArray::GetGameObject
```
undefined4 FUN_0073f4d0(undefined4 param_1)
{
  local_8 = 0;
  FUN_0053dfb0(param_1,&local_8);
  return local_8;
}
```

### CClientExoApp::GetCreatureByGameObjectID  gog=0x40ca10 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0073f550(mapper) delta=0x332b40 align=6/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, DWORD objectId) [param_bytes=4]
name_map: CClientExoApp::GetCreatureByGameObjectID [medium] gog 0x40ca10; vote; retn=4
callers=72 callees=0078bdf0
```
void FUN_0073f550(undefined4 param_1)
{
  FUN_0078bdf0(param_1);
  return;
}
```

### CClientExoApp::SendSaveGameRequest  gog=0x40cbb0 (param_bytes=12, notes='CClientExoApp * this, uint * param_1, uint * param_2, uint * param_3')
steam: cand=0x0073f6f0(mapper) delta=0x332b40 align=5/6 size=36 retn=12 gapG/S=48/48 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD slot, void* saveName, void* module) [param_bytes=12]
name_map: CClientExoApp::SendSaveGameRequest [medium] gog 0x40cbb0; vote; retn=12
callers=3 callees=00785620=CClientExoAppInternal::SendSaveGameRequest
```
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

### CClientExoApp::SendLoadGameRequest  gog=0x40cbe0 (param_bytes=12, notes='CClientExoApp * this, uint * param_1, uint * param_2, uint * param_3')
steam: cand=0x0073f720(mapper) delta=0x332b40 align=6/6 size=36 retn=12 gapG/S=48/48 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD slot, void* saveName, void* module) [param_bytes=12]
name_map: CClientExoApp::SendLoadGameRequest [medium] gog 0x40cbe0; vote; retn=12
callers=2 callees=00785660=CClientExoAppInternal::SendLoadGameRequest
```
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

### CClientExoApp::GetInGameGui  gog=0x40cc10 (param_bytes=0, notes='void')
steam: cand=0x0073f750(mapper) delta=0x332b40 align=6/6 size=28 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: ClientApp::GetGuiInGame [medium] decompile+asm from full dump (dump_excerpts/), Opus read 2026-09-30
callers=361 callees=
```
undefined4 __fastcall FUN_0073f750(int param_1)
{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x40);
}
```

### CClientExoApp::SetPlayerCreature  gog=0x40cdf0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0073f910(mapper) delta=0x332b20 align=6/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, DWORD creatureId) [param_bytes=4]
name_map: CClientExoApp::SetPlayerCreature [medium] gog 0x40cdf0; vote; retn=4
callers=3 callees=00798660=CClientExoAppInternal::SetPlayerCreature
```
void FUN_0073f910(undefined4 param_1)
{
  FUN_00798660(param_1);
  return;
}
```

### CClientExoApp::CreatureAcquireItem  gog=0x40cec0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x0073f9e0(mapper) delta=0x332b20 align=6/6 size=68 retn=8 gapG/S=80/80 cc_guess=thiscall
wrapper: void (__stdcall) (DWORD creatureId, void* item) [param_bytes=8]
name_map: CClientExoApp::CreatureAcquireItem [medium] gog 0x40cec0; vote; retn=8
callers=7 callees=0051c8b0=gog:CServerExoApp::ClientToServerObjectId, 0051cc60=gog:CServerExoApp::CreatureAcquireItem
```
void FUN_0073f9e0(undefined4 param_1,undefined4 param_2)
{
  uVar1 = FUN_0051c8b0(param_1);
  FUN_0051cc60(uVar1,param_2);
  return;
}
```

### CClientExoApp::SetEventDescriptions  gog=0x40cfe0 (param_bytes=0, notes='void')
steam: cand=0x0073fb20(mapper) delta=0x332b40 align=2/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::SetEventDescriptions [medium] gog 0x40cfe0; vote; retn=0
callers=1 callees=0078ac40=CClientExoAppInternal::StopServices
```
void __fastcall FUN_0073fb20(undefined4 param_1)
{
  FUN_0078ac40(param_1);
  return;
}
```

### CClientExoApp::ShutDownToMainMenu  gog=0x40d020 (param_bytes=0, notes='void')
steam: cand=0x0073fb40(mapper) delta=0x332b20 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::StopServices [medium] gog 0x40d000; vote; retn=0
callers=0 callees=0078e0f0=CClientExoAppInternal::ShutDownToMainMenu
```
void __fastcall FUN_0073fb40(undefined4 param_1)
{
  FUN_0078e0f0(param_1);
  return;
}
```

### CClientExoApp::SetVideoMode  gog=0x40d0b0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0073fbf0(mapper) delta=0x332b40 align=1/6 size=31 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr, DWORD modeNum) [param_bytes=4]
name_map: CClientExoApp::SetVideoMode [medium] gog 0x40d0b0; vote; retn=4
callers=1 callees=0078d6a0
```
void FUN_0073fbf0(undefined1 param_1)
{
  FUN_0078d6a0(param_1,1);
  return;
}
```

### CClientExoApp::SetTexturePack  gog=0x40d0d0 (param_bytes=1, notes='undefined1 param_1')
NO CANDIDATE (bracketing failed) -- see manual list

### CClientExoApp::GetActivePauseState  gog=0x40d2e0 (param_bytes=0, notes='void')
steam: cand=0x0073fe20(mapper) delta=0x332b40 align=1/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: BYTE (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetActivePauseState [medium] gog 0x40d2e0; vote; retn=0
callers=1 callees=00796ba0=CClientExoAppInternal::QueryExitProgram
```
void __fastcall FUN_0073fe20(undefined4 param_1)
{
  FUN_00796ba0(param_1);
  return;
}
```

### CClientExoApp::QueryExitProgram  gog=0x40d300 (param_bytes=0, notes='void')
steam: cand=0x0073fe20(mapper) delta=0x332b20 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetActivePauseState [medium] gog 0x40d2e0; vote; retn=0
callers=1 callees=00796ba0=CClientExoAppInternal::QueryExitProgram
```
void __fastcall FUN_0073fe20(undefined4 param_1)
{
  FUN_00796ba0(param_1);
  return;
}
```

### CClientExoApp::SetInputClass  gog=0x40d3c0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x0073fee0(mapper) delta=0x332b20 align=6/6 size=32 retn=8 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int inputClass, int setGuiStatus) [param_bytes=8]
name_map: CClientExoApp::SetInputClass [medium] gog 0x40d3c0; vote; retn=8
callers=62 callees=007b3050=CClientExoAppInternal::SetInputClass
```
void FUN_0073fee0(undefined4 param_1,undefined4 param_2)
{
  FUN_007b3050(param_1,param_2);
  return;
}
```

### CClientExoApp::GetInputClass  gog=0x40d3e0 (param_bytes=0, notes='void')
steam: cand=0x0073ff00(mapper) delta=0x332b20 align=6/6 size=31 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetInputClass [medium] gog 0x40d3e0; vote; retn=0
callers=10 callees=
```
undefined4 __fastcall FUN_0073ff00(int param_1)
{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x9c);
}
```

### CClientExoApp::AddMoveToModuleMovie  gog=0x40d640 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00740180(mapper) delta=0x332b40 align=6/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* movie) [param_bytes=4]
name_map: CClientExoApp::AddMoveToModuleMovie [medium] gog 0x40d640; vote; retn=4
callers=3 callees=00798b70=CClientExoAppInternal::AddMoveToModuleMovie
```
void FUN_00740180(undefined4 param_1)
{
  FUN_00798b70(param_1);
  return;
}
```

### CClientExoApp::RemoveMoveToModuleMovies  gog=0x40d680 (param_bytes=0, notes='void')
steam: cand=0x007401c0(mapper) delta=0x332b40 align=6/6 size=38 retn=0 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::RemoveMoveToModuleMovies [medium] gog 0x40d680; vote; retn=0
callers=1 callees=007413e0
```
void FUN_007401c0(void)
{
  FUN_007413e0(0);
  return;
}
```

### CClientExoApp::IsSoundPlayingInDialog  gog=0x40d750 (param_bytes=0, notes='void')
steam: cand=0x00740290(mapper) delta=0x332b40 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::IsSoundPlayingInDialog [medium] gog 0x40d750; vote; retn=0
callers=1 callees=0079bcc0=CClientExoAppInternal::IsSoundPlayingInDialog
```
void __fastcall FUN_00740290(undefined4 param_1)
{
  FUN_0079bcc0(param_1);
  return;
}
```

### CClientExoApp::DisableInput  gog=0x40d790 (param_bytes=0, notes='void')
steam: cand=0x007402d0(mapper) delta=0x332b40 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::DisableInput [medium] gog 0x40d790; vote; retn=0
callers=3 callees=007b3370=CClientExoAppInternal::DisableInput
```
void __fastcall FUN_007402d0(undefined4 param_1)
{
  FUN_007b3370(param_1);
  return;
}
```

### CClientExoApp::EnableInput  gog=0x40d7b0 (param_bytes=0, notes='void')
steam: cand=0x007402f0(mapper) delta=0x332b40 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::EnableInput [medium] gog 0x40d7b0; vote; retn=0
callers=1 callees=007b3390=CClientExoAppInternal::EnableInput
```
void __fastcall FUN_007402f0(undefined4 param_1)
{
  FUN_007b3390(param_1);
  return;
}
```

### CClientExoApp::DismissInGameGUI  gog=0x40d7d0 (param_bytes=0, notes='void')
steam: cand=0x00740310(mapper) delta=0x332b40 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::DismissInGameGUI [medium] gog 0x40d7d0; vote; retn=0
callers=1 callees=007b33b0=CClientExoAppInternal::DismissInGameGUI
```
void __fastcall FUN_00740310(undefined4 param_1)
{
  FUN_007b33b0(param_1);
  return;
}
```

### CClientExoApp::GetPausedByCombat  gog=0x40d7f0 (param_bytes=0, notes='void')
steam: cand=0x00740330(mapper) delta=0x332b40 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__stdcall) () [param_bytes=0]
name_map: CClientExoApp::GetPausedByCombat [medium] gog 0x40d7f0; vote; retn=0
callers=23 callees=0079bf00
```
void __fastcall FUN_00740330(undefined4 param_1)
{
  FUN_0079bf00(param_1);
  return;
}
```

### CClientExoApp::ChangeCharacter  gog=0x40d840 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00740380(mapper) delta=0x332b40 align=6/6 size=32 retn=8 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, DWORD leader, int playSound) [param_bytes=8]
name_map: CClientExoApp::ChangeCharacter [medium] gog 0x40d840; vote; retn=8
callers=6 callees=00797f30=CClientExoAppInternal::ChangeCharacter
```
void FUN_00740380(undefined4 param_1,undefined4 param_2)
{
  FUN_00797f30(param_1,param_2);
  return;
}
```

### CClientExoApp::RunDeathSequence  gog=0x40d860 (param_bytes=0, notes='void')
steam: cand=0x007403a0(mapper) delta=0x332b40 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::RunDeathSequence [medium] gog 0x40d860; vote; retn=0
callers=1 callees=0079c0e0=CClientExoAppInternal::RunDeathSequence
```
void __fastcall FUN_007403a0(undefined4 param_1)
{
  FUN_0079c0e0(param_1);
  return;
}
```

### CClientExoApp::SetGameOver  gog=0x40d880 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007403c0(mapper) delta=0x332b40 align=6/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int gameOver) [param_bytes=4]
name_map: CClientExoApp::SetGameOver [medium] gog 0x40d880; vote; retn=4
callers=3 callees=0079c230
```
void FUN_007403c0(undefined4 param_1)
{
  FUN_0079c230(param_1);
  return;
}
```

### CClientExoApp::GetGameOver  gog=0x40d8a0 (param_bytes=0, notes='void')
steam: cand=0x007403e0(mapper) delta=0x332b40 align=6/6 size=31 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetGameOver [medium] gog 0x40d8a0; vote; retn=0
callers=1 callees=
```
undefined4 __fastcall FUN_007403e0(int param_1)
{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x2c0);
}
```

### CClientExoApp::SetCanSendDialog  gog=0x40d8e0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00740420(mapper) delta=0x332b40 align=6/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int canSend) [param_bytes=4]
name_map: CClientExoApp::SetCanSendDialog [medium] gog 0x40d8e0; vote; retn=4
callers=3 callees=0079c550=CClientExoAppInternal::SetCanSendDialog
```
void FUN_00740420(undefined4 param_1)
{
  FUN_0079c550(param_1);
  return;
}
```

### CClientExoApp::GetLastTarget  gog=0x40db20 (param_bytes=0, notes='void')
steam: cand=0x00740660(mapper) delta=0x332b40 align=6/6 size=31 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: DWORD (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetLastTarget [medium] gog 0x40db20; vote; retn=0
callers=4 callees=
```
undefined4 __fastcall FUN_00740660(int param_1)
{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x2b4);
}
```

### CClientExoApp::GetPazaakResults  gog=0x40dbf0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x00740730(mapper) delta=0x332b40 align=6/6 size=49 retn=8 gapG/S=64/64 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int* won, int* wager) [param_bytes=8]
name_map: CClientExoApp::GetPazaakResults [medium] gog 0x40dbf0; vote; retn=8
callers=1 callees=
```
void __thiscall FUN_00740730(int param_1,undefined4 *param_2,undefined4 *param_3)
{
  iVar1 = *(int *)(param_1 + 4);
  *param_2 = *(undefined4 *)(iVar1 + 0x80);
  *param_3 = *(undefined4 *)(iVar1 + 0x7c);
  return;
}
```

### CClientExoApp::SetFutureLeader  gog=0x40dc30 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00740770(mapper) delta=0x332b40 align=6/6 size=36 retn=4 gapG/S=48/48 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, DWORD leader) [param_bytes=4]
name_map: CClientExoApp::SetFutureLeader [medium] gog 0x40dc30; vote; retn=4
callers=2 callees=
```
void __thiscall FUN_00740770(int param_1,undefined4 param_2)
{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x338) = param_2;
  return;
}
```

### CClientExoApp::SetCombatMode  gog=0x40dd40 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x00740880(mapper) delta=0x332b40 align=6/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int mode) [param_bytes=4]
name_map: CClientExoApp::SetCombatMode [medium] gog 0x40dd40; vote; retn=4
callers=3 callees=0079d6e0=CClientExoAppInternal::SetCombatMode
```
void FUN_00740880(undefined4 param_1)
{
  FUN_0079d6e0(param_1);
  return;
}
```

### CClientExoApp::PlayerFlourishWeapons  gog=0x40dd60 (param_bytes=0, notes='void')
steam: cand=0x007408a0(mapper) delta=0x332b40 align=5/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::PlayerFlourishWeapons [medium] gog 0x40dd60; vote; retn=0
callers=2 callees=0079f430=CClientExoAppInternal::PlayerFlourishWeapons
```
void __fastcall FUN_007408a0(undefined4 param_1)
{
  FUN_0079f430(param_1);
  return;
}
```

### CClientExoApp::GetGUIString  gog=0x40ddb0 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x007408f0(mapper) delta=0x332b40 align=5/6 size=53 retn=8 gapG/S=64/64 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, void* outString, DWORD strRef) [param_bytes=8]
name_map: CClientExoApp::GetGUIString [medium] gog 0x40ddb0; vote; retn=8
callers=59 callees=0079e210=CClientExoAppInternal::GetGUIString
```
undefined4 FUN_007408f0(undefined4 param_1,undefined4 param_2)
{
  FUN_0079e210(param_1,param_2);
  return param_1;
}
```

### CClientExoApp::GetAutoPaused  gog=0x40de20 (param_bytes=0, notes='void')
steam: cand=0x00740960(mapper) delta=0x332b40 align=5/6 size=26 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetAutoPaused [medium] gog 0x40de20; vote; retn=0
callers=7 callees=
```
uint __fastcall FUN_00740960(int param_1)
{
  return *(uint *)(*(int *)(param_1 + 4) + 900) & 1;
}
```

### CClientExoApp::EnableVideoEffect  gog=0x40de60 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x007409a0(mapper) delta=0x332b40 align=5/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, int effect) [param_bytes=4]
name_map: CClientExoApp::EnableVideoEffect [medium] gog 0x40de60; vote; retn=4
callers=7 callees=0079e550=CClientExoAppInternal::EnableVideoEffect
```
void FUN_007409a0(undefined4 param_1)
{
  FUN_0079e550(param_1);
  return;
}
```

### CClientExoApp::DisableVideoEffect  gog=0x40de80 (param_bytes=0, notes='void')
steam: cand=0x007409c0(mapper) delta=0x332b40 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::DisableVideoEffect [medium] gog 0x40de80; vote; retn=0
callers=7 callees=0079e960=CClientExoAppInternal::DisableVideoEffect
```
void __fastcall FUN_007409c0(undefined4 param_1)
{
  FUN_0079e960(param_1);
  return;
}
```

### CClientExoApp::GetTargetChanging  gog=0x40e340 (param_bytes=0, notes='void')
steam: cand=0x00740e80(mapper) delta=0x332b40 align=4/6 size=57 retn=0 gapG/S=64/64 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetTargetChanging [medium] gog 0x40e340; vote; retn=0
callers=3 callees=
```
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
bool __fastcall FUN_00740e80(int param_1)
{
  return (float)_DAT_00986b38 < *(float *)(*(int *)(param_1 + 4) + 0x378);
}
```

### CClientExoApp::GetPazaakIsRunning  gog=0x40e3e0 (param_bytes=0, notes='void')
steam: cand=0x00740f20(mapper) delta=0x332b40 align=5/6 size=28 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetPazaakIsRunning [medium] gog 0x40e3e0; vote; retn=0
callers=1 callees=
```
undefined4 __fastcall FUN_00740f20(int param_1)
{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x70);
}
```

### CClientExoApp::SetMouseMode  gog=0x40e400 (param_bytes=1, notes='undefined1 param_1')
steam: cand=0x00740f40(bracket) delta=0x332b40 align=/ size=29 retn=4 gapG/S=32/ cc_guess=
wrapper: void (__thiscall) (void* thisPtr, BYTE mode) [param_bytes=4]
callers=1 callees=007a0490
```
void FUN_00740f40(undefined1 param_1)
{
  FUN_007a0490(param_1);
  return;
}
```

### CClientExoApp::CancelMovie  gog=0x40e420 (param_bytes=0, notes='void')
steam: cand=0x00740f60(mapper) delta=0x332b40 align=5/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::CancelMovie [medium] gog 0x40e420; vote; retn=0
callers=1 callees=007a0500=CClientExoAppInternal::CancelMovie
```
void __fastcall FUN_00740f60(undefined4 param_1)
{
  FUN_007a0500(param_1);
  return;
}
```

### CClientExoApp::GetCharacterChangeInProgress  gog=0x40e480 (param_bytes=0, notes='void')
steam: cand=0x00740fc0(mapper) delta=0x332b40 align=6/6 size=31 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: GetCharacterChangeInProgress (client app, plain RET) [high] research/companion_levelup/phase4_evidence/S1_gui_guards.txt
callers=3 callees=
```
undefined4 __fastcall FUN_00740fc0(int param_1)
{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x4ac);
}
```

### CClientExoApp::StopCreditSequence  gog=0x40e510 (param_bytes=0, notes='void')
steam: cand=0x00741050(mapper) delta=0x332b40 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::StopCreditSequence [medium] gog 0x40e510; vote; retn=0
callers=1 callees=007a12c0=CClientExoAppInternal::StopCreditSequence
```
void __fastcall FUN_00741050(undefined4 param_1)
{
  FUN_007a12c0(param_1);
  return;
}
```

### CClientExoApp::GetCreditSequenceInProgress  gog=0x40e530 (param_bytes=0, notes='void')
steam: cand=0x00741070(mapper) delta=0x332b40 align=6/6 size=31 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetCreditSequenceInProgress [medium] gog 0x40e530; vote; retn=0
callers=1 callees=
```
undefined4 __fastcall FUN_00741070(int param_1)
{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x4c0);
}
```

### CClientExoApp::AddMovieToMovieQueue  gog=0x40e680 (param_bytes=8, notes='undefined4 param_1, undefined4 param_2')
steam: cand=0x007411c0(mapper) delta=0x332b40 align=6/6 size=106 retn=8 gapG/S=112/112 cc_guess=thiscall
wrapper: void (__thiscall) (void* thisPtr, void* movie, int skippable) [param_bytes=8]
name_map: CClientExoApp::AddMovieToMovieQueue [medium] gog 0x40e680; vote; retn=8
callers=1 callees=009370c7=_stricmp, 00734010=CExoString::CStr, 00401650, 007410e0, 007a0f10=CClientExoAppInternal::AddMovieToMovieQueue
strings: malmov14 | malmov12
```
void __thiscall FUN_007411c0(undefined4 param_1,undefined4 param_2,undefined4 param_3)
{
  uVar1 = FUN_00734010(param_1);
  FUN_00401650(uVar1);
  pcVar4 = "malmov12";
  pcVar2 = (char *)FUN_00734010();
  iVar3 = __stricmp(pcVar2,pcVar4);
  if (iVar3 != 0) {
    pcVar4 = "malmov14";
    pcVar2 = (char *)FUN_00734010();
    iVar3 = __stricmp(pcVar2,pcVar4);
    if (iVar3 != 0) goto LAB_00741211;
  }
  FUN_007410e0();
LAB_00741211:
  FUN_007a0f10(param_2,param_3);
  return;
}
```

### CClientExoApp::GetInFreeLook  gog=0x40e750 (param_bytes=0, notes='void')
steam: cand=0x00741290(mapper) delta=0x332b40 align=6/6 size=108 retn=0 gapG/S=112/112 cc_guess=thiscall
wrapper: int (__thiscall) (void* thisPtr) [param_bytes=0]
name_map: CClientExoApp::GetInFreeLook [medium] gog 0x40e750; vote; retn=0
callers=1 callees=0073f430=CClientExoApp::GetModule
```
bool FUN_00741290(void)
{
  iVar1 = FUN_0073f430();
  if ((iVar1 != 0) && (uVar2 = extraout_var, iVar1 = FUN_0073f430(), *(int *)(iVar1 + 0x40) != 0)) {
    iVar1 = FUN_0073f430();
    iVar1 = (**(code **)(**(int **)(iVar1 + 0x40) + 0x80))(0x106e,*(int **)(iVar1 + 0x40),uVar2);
    return iVar1 != 0;
  }
  return false;
}
```

### CServerExoApp::GetGameObject  gog=0x64bac0 (param_bytes=4, notes='CServerExoApp * this, undefined4 param_1')
steam: cand=0x0051c0a0(mapper) delta=-0x12fa20 align=6/6 size=62 retn=4 gapG/S=64/64 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, DWORD objectId) [param_bytes=4]
name_map: CServerExoApp::GetGameObject [high] ai_review_2026-10-01/review_ai_engine.md map; __thiscall(ECX=[[0xA1B4A4]+8], id) RET 4; NULL for stale ids (NOT cdecl)
callers=213 callees=0053dfb0=CGameObjectArray::GetGameObject
```
undefined4 FUN_0051c0a0(undefined4 param_1)
{
  local_8 = 0;
  FUN_0053dfb0(param_1,&local_8);
  return local_8;
}
```

### CServerExoApp::GetActiveTimer  gog=0x64bd70 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0051c350(mapper) delta=-0x12fa20 align=6/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr, int objectId) [param_bytes=4]
callers=12 callees=00538e00
```
void FUN_0051c350(undefined4 param_1)
{
  FUN_00538e00(param_1);
  return;
}
```

### CServerExoApp::GetWorldTimer  gog=0x64bd90 (param_bytes=0, notes='void')
steam: cand=0x0051c370(mapper) delta=-0x12fa20 align=6/6 size=31 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
callers=70 callees=
```
undefined4 __fastcall FUN_0051c370(int param_1)
{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10048);
}
```

### CServerExoApp::GetGlobalVariableTable  gog=0x64c2b0 (param_bytes=0, notes='void')
steam: cand=0x0051c890(mapper) delta=-0x12fa20 align=6/6 size=22 retn=0 gapG/S=32/32 cc_guess=thiscall
wrapper: void* (__thiscall) (void* thisPtr) [param_bytes=0]
callers=35 callees=
```
int __fastcall FUN_0051c890(int param_1)
{
  return *(int *)(param_1 + 4) + 0x100fc;
}
```

### CServerExoApp::ClientToServerObjectId  gog=0x64c2d0 (param_bytes=4, notes='undefined4 param_1')
steam: cand=0x0051c8b0(mapper) delta=-0x12fa20 align=5/6 size=28 retn=4 gapG/S=32/32 cc_guess=thiscall
wrapper: DWORD (__thiscall) (void* thisPtr, DWORD clientId) [param_bytes=4]
callers=84 callees=00539d30
```
void FUN_0051c8b0(undefined4 param_1)
{
  FUN_00539d30(param_1);
  return;
}
```

