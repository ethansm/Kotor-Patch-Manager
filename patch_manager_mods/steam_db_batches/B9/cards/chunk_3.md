# Review cards B9 chunk 3

Answer each card with one TSV row in `steam_db_batches/B9/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B9-3-01

GOG function: CServerExoApp::SetIgnoreConversationStartRange  gog 0x0064c780  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051ce00 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CServerExoApp::SetLoadFromSaveGame | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0051cde0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c760 CServerExoApp::GetActionConversation -> 0x0051cd40  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c7b0 CServerExoApp::TestSafeLocation -> 0x0051cd90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x00762ff0 CSWSDoor::EventHandler -> Steam 0x00617b40  in candidate callers: no
  0x0076d3b0 CSWSPlaceable::EventHandler -> Steam 0x005f2fe0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051ce00: 16-byte aligned: YES
  0051ce00  55             PUSH EBP
  0051ce01  8B EC          MOV EBP,ESP
  0051ce03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051ce00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100C4 [dec 65732]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051ce00 FUN_0051ce00 size=36 ret=4 callers=3 callees=0

void __thiscall FUN_0051ce00(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x100c4) = param_2;
  return;
}
```

### decoy: 0x0051cde0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cde0: 16-byte aligned: YES
  0051cde0  55             PUSH EBP
  0051cde1  8B EC          MOV EBP,ESP
  0051cde3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cde0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100C4 [dec 65732]  x1  via ECX
```

## Card B9-3-02

GOG function: CSWSModule::PlotInterAreaPath  gog 0x0072d0b0  callconv=(blank)  param_size_bytes=8  gog_size=555  gog_retn=8
Candidate Steam address: 0x0055a900 (size 555, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=pathfind | G7 mac: pass(0.89) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0055ab30:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0072cf90 CSWSModule::PackModuleIntoMessage -> 0x0055a7e0  gate_class=AUTO
- GOG-order neighbour after:  0x0072d2e0 CSWSModule::PlotPath -> 0x0055ab30  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00647bc0 CPathfindInformation::ResetInterAreaPathSearchData -> Steam ?  in candidate callees: n/a
  0x00648270 CPathfindInformation::FlipTempEndPoints -> Steam ?  in candidate callees: n/a
  0x00729900 CSWSModule::ComputeInterAreaPath -> Steam ?  in candidate callees: n/a
  0x0072d430 CSWSModule::PlotPathInArea -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x70*,0x80*,0xa0*,0xa8*,0xac*,0xb0*,0xb4*,0xb8*} | Steam {0x70*,0x80*,0xa0*,0xa8*,0xac*,0xb0*,0xb4*,0xb8*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x4c*} | Steam {0x4*,0x8*,0x4c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0055a900: 16-byte aligned: YES
  0055a900  55             PUSH EBP
  0055a901  8B EC          MOV EBP,ESP
  0055a903  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0055a900
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x00557150  (unnamed)
  0x006FA480  (unnamed)
  0x0055AC80  CSWSArea::PlotPathInArea (chooses grid vs path-point)
  0x009196FD  operator delete
  0x006F9DD0  (unnamed)
struct offsets touched (top 8):
  +0xB8 [dec 184]  x4  via EAX,EDX
  +0xA8 [dec 168]  x3  via EAX,EDX
  +0xAC [dec 172]  x3  via EDX
  +0x70 [dec 112]  x2  via EAX,ECX
  +0xB0 [dec 176]  x2  via ECX,EDX
  +0xB4 [dec 180]  x2  via ECX,EDX
  +0x80 [dec 128]  x1  via EDX
  +0xA0 [dec 160]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0055a900 FUN_0055a900 size=555 ret=8 callers=1 callees=6

int FUN_0055a900(int param_1,undefined4 param_2)

{
  longlong lVar1;
  undefined4 *puVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int local_10;
  
  if (*(int *)(param_1 + 0x70) == *(int *)(param_1 + 0x80)) {
    if (*(int *)(param_1 + 0xac) == 0) {
      *(undefined4 *)(param_1 + 0xa0) = 1;
      *(undefined4 *)(param_1 + 0xa8) = 1;
      puVar2 = (undefined4 *)FUN_00919723(4);
      *puVar2 = *(undefined4 *)(param_1 + 0x70);
      *(undefined4 **)(param_1 + 0xac) = puVar2;
      *(undefined4 *)(param_1 + 0xb0) = 0;
    }
  }
  else if (*(int *)(param_1 + 0xac) == 0) {
    iVar3 = FUN_00557150(param_1);
    if (iVar3 == 0) {
      return DAT_009944bc;
    }
  }
  else {
    FUN_006fa480();
  }
  iVar3 = FUN_0055ac80(param_1,param_2);
  if (iVar3 == DAT_009944bc) {
    if (1 < *(int *)(param_1 + 0xa8)) {
      iVar3 = *(int *)(param_1 + 0xb4);
      lVar1 = (ulonglong)(iVar3 + 1) * 4;
      iVar4 = FUN_00919723(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
      for (local_10 = 0; local_10 < iVar3; local_10 = local_10 + 1) {
        *(undefined4 *)(iVar4 + local_10 * 4) =
             *(undefined4 *)(*(int *)(param_1 + 0xb8) + local_10 * 4);
      }
      *(undefined4 *)(iVar4 + iVar3 * 4) = **(undefined4 **)(param_1 + 0xb0);
      *(int *)(param_1 + 0xb4) = iVar3 + 1;
      if (*(int *)(param_1 + 0xb8) != 0) {
        FUN_009196fd(*(undefined4 *)(param_1 + 0xb8));
      }
      *(int *)(param_1 + 0xb8) = iVar4;
      FUN_006f9dd0();
      FUN_006fa480();
      iVar5 = FUN_00557150(param_1);
      iVar4 = DAT_009944b4;
      iVar3 = DAT_009944bc;
      if (iVar5 != 0) {
        FUN_006fa480();
        iVar3 = iVar4;
      }
    }
  }
  else if (1 < *(int *)(param_1 + 0xa8)) {
    FUN_006fa480();
  }
  return iVar3;
}
```

### decoy: 0x0055ab30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055ab30: 16-byte aligned: YES
  0055ab30  55             PUSH EBP
  0055ab31  8B EC          MOV EBP,ESP
  0055ab33  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0055ab30
string literals (0):
direct callees (7):
  0x006FA080  (unnamed)
  0x006F9E90  (unnamed)
  0x006F9D40  CPathfindInformation::ResetAtEndOfPath
  0x0051C280  (unnamed)
  0x0055A900  pathfind inter-area driver
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005C3270  CSWSCreature::ComputeTotalWalkDistance
struct offsets touched (top 8):
  +0x58 [dec 88]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x84 [dec 132]  x2  via EAX,EDX
  +0x88 [dec 136]  x2  via EDX
  +0xA4 [dec 164]  x2  via EAX,EDX
  +0x2C [dec 44]  x1  via EAX
  +0x3C [dec 60]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
```

## Card B9-3-03

GOG function: CServerExoAppInternal::CopyModuleToCurrentGame  gog 0x007b4f60  callconv=(blank)  param_size_bytes=8  gog_size=106  gog_retn=12
Candidate Steam address: 0x00531be0 (size 106, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00531c50:STR0/CALLEE0/SIG0.014/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007b4960 CServerExoAppInternal::UnloadModule -> 0x005315e0  gate_class=AUTO
- GOG-order neighbour after:  0x007b4fd0 CServerExoAppInternal::LoadModule -> 0x00531c50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0x80*} | Steam {0x8*,0xc*,0x80*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x00531be0: 16-byte aligned: YES
  00531be0  55             PUSH EBP
  00531be1  8B EC          MOV EBP,ESP
  00531be3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00531be0
string literals (0):
direct callees (1):
  0x00734010  CExoString::CStr
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00531be0 FUN_00531be0 size=106 ret=12 callers=1 callees=5

undefined4 __fastcall FUN_00531be0(undefined4 param_1)

{
  LPCSTR pCVar1;
  LPCSTR lpExistingFileName;
  undefined4 uVar2;
  DWORD DVar3;
  BOOL BVar4;
  
  BVar4 = 0;
  pCVar1 = (LPCSTR)FUN_00734010(0,param_1);
  lpExistingFileName = (LPCSTR)FUN_00734010();
  BVar4 = CopyFileA(lpExistingFileName,pCVar1,BVar4);
  if (BVar4 == 0) {
    GetLastError();
    uVar2 = 0;
  }
  else {
    pCVar1 = (LPCSTR)FUN_00734010();
    DVar3 = GetFileAttributesA(pCVar1);
    if ((DVar3 & 1) != 0) {
      DVar3 = 0x80;
      pCVar1 = (LPCSTR)FUN_00734010();
      SetFileAttributesA(pCVar1,DVar3);
    }
    uVar2 = 1;
  }
  return uVar2;
}
```

### decoy: 0x00531c50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00531c50: 16-byte aligned: YES
  00531c50  55             PUSH EBP
  00531c51  8B EC          MOV EBP,ESP
  00531c53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00531c50
string literals (17):
  0x00993F0C  at 00531d57  'CURRENTGAME:'
  0x00993F0C  at 00531da4  'CURRENTGAME:'
  0x00993F0C  at 00531ddd  'CURRENTGAME:'
  0x00993E84  at 00531fa7  'GAMEINPROGRESS:'
  0x00993E84  at 00532107  'GAMEINPROGRESS:'
  0x00993F40  at 00532258  'NWMFILES:'
  0x00993F40  at 005322bd  'NWMFILES:'
  0x00993F40  at 005323dd  'NWMFILES:'
  0x00993F38  at 00532454  'LIVE%d'
  0x00993F2C  at 005324bd  ':MODULES'
  0x00993E50  at 005328e6  'MODULES:'
  0x00993F1C  at 005329a7  '%s\\\\modules\\\\%s'
direct callees (43):
  0x00401000  (unnamed)
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00733540  CExoString::Constructor
  0x0073A590  (unnamed)
  0x00734010  CExoString::CStr
  0x005315E0  CServerExoAppInternal::UnloadModule
  0x00733570  CExoString::CExoString_char
  0x00714F30  CExoResMan::CreateDirectory2
  0x00733780  CExoString::~CExoString
  0x007156E0  CExoResMan::CleanDirectory
  0x007338D0  CExoString::operator=_2
  0x00734030  CExoString::Find
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00919F10  _strlen
  0x00734870  CExoString::Right
  0x0053AB60  (unnamed)
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FDCA0  CSWPartyTable::AddGameInProgress
  0x00710810  (unnamed)
struct offsets touched (top 9):
  +0xC [dec 12]  x16  via EAX,ECX,EDX
  +0x14 [dec 20]  x9  via EAX
  +0x8 [dec 8]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX
  +0x10 [dec 16]  x2  via ECX
  +0x38 [dec 56]  x2  via EDX
  +0x10010 [dec 65552]  x2  via EAX,EDX
  +0x100B4 [dec 65716]  x2  via EAX,EDX
  +0x30 [dec 48]  x1  via EAX
```

## Card B9-3-04

GOG function: CServerExoAppInternal::UpdateClientGameObjectsForPlayer  gog 0x007ba7e0  callconv=__thiscall  param_size_bytes=16  gog_size=546  gog_retn=16
Candidate Steam address: 0x00537590 (size 546, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005377c0:STR0/CALLEE0/SIG0.226/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ba760 CServerExoAppInternal::UpdateClientGameObjects -> 0x00537510  gate_class=REVIEW
- GOG-order neighbour after:  0x007baa10 CServerExoAppInternal::ValidatePlayerLogin -> 0x005377c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00608350 CExoTimers::GetHighResolutionTimer -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x0075ca90 CSWSPlayer::GetGameObject -> Steam ?  in candidate callees: n/a
  0x007d35e0 CSWSMessage::SendServerToPlayerGameObjUpdate_2 -> Steam ?  in candidate callees: n/a
  0x007dc930 CSWSMessage::SendServerToPlayerGameObjUpdate_ObjControl -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x24*,0x2c*,0x30*,0x34*,0x7c*,0x350*} | Steam {0x4*,0x24*,0x2c*,0x30*,0x34*,0x7c*,0x350*}
imm sets (* = in both, 5 common): GOG {0x2*,0x10*,0x54*,0x30d40*,0x7f000000*} | Steam {0x2*,0x10*,0x54*,0x30d40*,0x7f000000*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x00537590: 16-byte aligned: YES
  00537590  55             PUSH EBP
  00537591  8B EC          MOV EBP,ESP
  00537593  83 EC 54       SUB ESP,0x54
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00537590
string literals (0):
direct callees (5):
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x00632A20  (unnamed)
  0x0063CEC0  (unnamed)
  0x005453C0  CSWSObject::GetArea
  0x00646210  (unnamed)
struct offsets touched (top 8):
  +0x30 [dec 48]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x2C [dec 44]  x4  via EAX,ECX,EDX
  +0x34 [dec 52]  x4  via EAX,ECX,EDX
  +0x10010 [dec 65552]  x3  via EAX,ECX
  +0x24 [dec 36]  x2  via ECX
  +0x7C [dec 124]  x1  via ECX
  +0x350 [dec 848]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00537590 FUN_00537590 size=546 ret=16 callers=1 callees=5

void FUN_00537590(int param_1,int param_2,uint param_3,int param_4)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  undefined8 uVar5;
  
  uVar5 = CONCAT44(param_4,param_3);
  if ((param_2 == 1) && (uVar5 = CONCAT44(param_4,param_3), param_3 == 0 && param_4 == 0)) {
    uVar5 = FUN_00736220();
  }
  bVar1 = false;
  if (*(char *)(param_1 + 0x24) == '\x02') {
    bVar1 = true;
  }
  else if (((*(char *)(param_1 + 0x24) == '\x01') && (*(int *)(param_1 + 0x7c) == 1)) &&
          (iVar2 = FUN_00632a20(), iVar2 != 0)) {
    piVar4 = (int *)FUN_00632a20();
    iVar2 = (**(code **)(*piVar4 + 0x30))();
    if ((iVar2 != 0) && (*(int *)(iVar2 + 0x350) != 0x7f000000)) {
      bVar1 = true;
    }
  }
  if (bVar1) {
    if (((param_2 == 1) ||
        (param_4 - *(int *)(param_1 + 0x30) != (uint)(param_3 < *(uint *)(param_1 + 0x2c)))) ||
       (199999 < param_3 - *(uint *)(param_1 + 0x2c))) {
      FUN_0063cec0(param_1);
      iVar2 = FUN_00632a20();
      if (iVar2 != 0) {
        iVar2 = FUN_00632a20();
        iVar2 = *(int *)(iVar2 + 4);
        if (*(int *)(param_1 + 0x34) != iVar2) {
          FUN_00632a20();
          iVar3 = FUN_005453c0();
          if (iVar3 != 0) {
            FUN_00646210(*(undefined4 *)(param_1 + 4),iVar2);
            *(int *)(param_1 + 0x34) = iVar2;
            *(undefined8 *)(param_1 + 0x2c) = uVar5;
          }
        }
      }
      *(undefined8 *)(param_1 + 0x2c) = uVar5;
    }
    iVar2 = FUN_00632a20();
    if (iVar2 != 0) {
      piVar4 = (int *)FUN_00632a20();
      (**(code **)(*piVar4 + 0x30))();
      iVar2 = FUN_005453c0();
      if ((iVar2 == 0) && (*(int *)(param_1 + 0x34) != 0x7f000000)) {
        *(undefined4 *)(param_1 + 0x34) = 0x7f000000;
        FUN_00646210(*(undefined4 *)(param_1 + 4),0x7f000000);
        *(undefined8 *)(param_1 + 0x2c) = uVar5;
      }
    }
    return;
  }
  return;
}
```

### decoy: 0x005377c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005377c0: 16-byte aligned: YES
  005377c0  55             PUSH EBP
  005377c1  8B EC          MOV EBP,ESP
  005377c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005377c0
string literals (0):
direct callees (2):
  0x00647020  (unnamed)
  0x00647110  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x24 [dec 36]  x2  via EAX
  +0x10010 [dec 65552]  x2  via ECX,EDX
```

## Card B9-3-05

GOG function: CSWSModule::non_virtual_thunk_to_AsSWSModule  gog 0x00729480  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00556cd0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00556670:STR0/CALLEE0/SIG0.011/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00728e20 CSWSModule::Constructor -> 0x00556670  gate_class=AUTO
- GOG-order neighbour after:  0x007294a0 CSWSModule::Destructor -> 0x00556cf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00556cd0: 16-byte aligned: YES
  00556cd0  55             PUSH EBP
  00556cd1  8B EC          MOV EBP,ESP
  00556cd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00556cd0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00556cd0 FUN_00556cd0 size=17 ret=0 callers=0 callees=0

int __fastcall FUN_00556cd0(int param_1)

{
  return param_1 + -0x1c;
}
```

### decoy: 0x00556670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00556670: 16-byte aligned: YES
  00556670  55             PUSH EBP
  00556671  8B EC          MOV EBP,ESP
  00556673  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00556670
string literals (2):
  0x00993F0C  at 005568fa  'CURRENTGAME:'
  0x0099492C  at 00556bd1  'MODULE'
direct callees (26):
  0x00561140  (unnamed)
  0x0053D660  (unnamed)
  0x00722870  (unnamed)
  0x007107F0  (unnamed)
  0x00735240  (unnamed)
  0x00733540  CExoString::Constructor
  0x0052E4A0  (unnamed)
  0x005E91B0  CSWVarTable::Constructor
  0x0091D67E  (unnamed)
  0x00561450  (unnamed)
  0x0084A640  (unnamed)
  0x00561580  (unnamed)
  0x00919723  operator new
  0x005F6AB0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x0055FA90  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x44 [dec 68]  x1  via EAX
  +0x54 [dec 84]  x1  via ECX
  +0x58 [dec 88]  x1  via EDX
  +0x148 [dec 328]  x1  via ECX
  +0x14C [dec 332]  x1  via EDX
  +0x168 [dec 360]  x1  via ECX
```

## Card B9-3-06

GOG function: CServerExoApp::MainLoop  gog 0x0064be90  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c4c0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c490:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064be70 CServerExoApp::Initialize -> 0x0051c450  gate_class=REVIEW
- GOG-order neighbour after:  0x0064beb0 CServerExoApp::PlayerListChange -> 0x0051c490  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b6bb0 CServerExoAppInternal::MainLoop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c4c0: 16-byte aligned: YES
  0051c4c0  55             PUSH EBP
  0051c4c1  8B EC          MOV EBP,ESP
  0051c4c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c4c0
string literals (0):
direct callees (1):
  0x00537830  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c4c0 FUN_0051c4c0 size=22 ret=0 callers=0 callees=1
THUNK -> 0x00537830 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c4c0(undefined4 param_1)

{
  FUN_00537830(param_1);
  return;
}
```

### decoy: 0x0051c490 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c490: 16-byte aligned: YES
  0051c490  55             PUSH EBP
  0051c491  8B EC          MOV EBP,ESP
  0051c493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c490
string literals (0):
direct callees (1):
  0x00535C30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-3-07

GOG function: CServerExoAppInternal::ValidatePlayerLogin  gog 0x007baa10  callconv=(blank)  param_size_bytes=4  gog_size=99  gog_retn=4
Candidate Steam address: 0x005377c0 (size 99, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00537510:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ba7e0 CServerExoAppInternal::UpdateClientGameObjectsForPlayer -> 0x00537590  gate_class=REVIEW
- GOG-order neighbour after:  0x007baa80 CServerExoAppInternal::ContinueMessageProcessing -> 0x00537830  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007dd740 CSWSMessage::SendServerToPlayerLogin_Confirm -> Steam ?  in candidate callees: n/a
  0x007dd830 CSWSMessage::SendServerToPlayerModule_Info -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x24*} | Steam {0x4*,0x24*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005377c0: 16-byte aligned: YES
  005377c0  55             PUSH EBP
  005377c1  8B EC          MOV EBP,ESP
  005377c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005377c0
string literals (0):
direct callees (2):
  0x00647020  (unnamed)
  0x00647110  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x24 [dec 36]  x2  via EAX
  +0x10010 [dec 65552]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005377c0 FUN_005377c0 size=99 ret=4 callers=1 callees=2

undefined4 FUN_005377c0(int param_1)

{
  if (*(char *)(param_1 + 0x24) != '\x01') {
    *(undefined1 *)(param_1 + 0x24) = 1;
    FUN_00647020(*(undefined4 *)(param_1 + 4));
    FUN_00647110(*(undefined4 *)(param_1 + 4));
  }
  return 1;
}
```

### decoy: 0x00537510 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00537510: 16-byte aligned: YES
  00537510  55             PUSH EBP
  00537511  8B EC          MOV EBP,ESP
  00537513  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00537510
string literals (0):
direct callees (5):
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00537590  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0x10064 [dec 65636]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-3-08

GOG function: CServerExoAppInternal::StallEventSaveGame  gog 0x007b7980  callconv=(blank)  param_size_bytes=0  gog_size=4064  gog_retn=0
Candidate Steam address: 0x00534600 (size 4064, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=SaveGameInProgress | G7 mac: pass(0.84) | G8 mac_rotation: pass
- anchors: A-STR 22 | A-CALLEE 12 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'MOD V1.0'@00993fa4; 'PORTRAIT'@00993fb0; 'LIVECONTENT'@00993fbc; CALLEE 12 (callees: SetCanSendDialog 0x740420, StartSounds 0x7a7c00, Constructor 0x716e60); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005345b0:STR0/CALLEE0/SIG0.08/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b6bb0 CServerExoAppInternal::MainLoop -> 0x00533830  gate_class=REVIEW
- GOG-order neighbour after:  0x007b8970 CServerExoAppInternal::DealWithLoadGameError -> 0x00535630  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (73), mapped to Steam through trusted rows only:
  0x004072b0 Global::IsGameModded -> Steam ?  in candidate callees: n/a
  0x004099e0 Global::UpdateScreen -> Steam ?  in candidate callees: n/a
  0x0040c510 CClientExoApp::EndLoadBar -> Steam ?  in candidate callees: n/a
  0x0040c8f0 CClientExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x0040d070 CClientExoApp::GetSWParty -> Steam ?  in candidate callees: n/a
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam ?  in candidate callees: n/a
  0x0040d400 CClientExoApp::ShowLoadScreen -> Steam ?  in candidate callees: n/a
  ... 65 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (25): 'LIVE%d'; 'GAMEINPROGRESS:'; 'savenfo'; 'PORTRAIT' ...
disp sets (* = in both, 10 common): GOG {0x4*,0x8*,0xc*,0x14*,0x30*,0x48*,0xc8*,0xcc*,0x148*,0xf78*} | Steam {0x4*,0x8*,0xc*,0x14*,0x30*,0x48*,0xc8*,0xcc*,0x148*,0xf78*}
imm sets (* = in both, 33 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+19)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+19)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00534600: 16-byte aligned: YES
  00534600  55             PUSH EBP
  00534601  8B EC          MOV EBP,ESP
  00534603  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00534600
string literals (25):
  0x009940B8  at 005349cd  'savenfo'
  0x009940A4  at 00534b8c  'AREANAME'
  0x00994098  at 00534ba1  'LASTMODULE'
  0x0099408C  at 00534bb6  'TIMEPLAYED'
  0x00994080  at 00534bfe  'CHEATUSED'
  0x00994070  at 00534c14  'SAVEGAMENAME'
  0x00994064  at 00534c32  'TIMESTAMP'
  0x0099405C  at 00534c7e  'PCNAME'
  0x00994050  at 00534cb7  'SAVENUMBER'
  0x00994040  at 00534cdd  'GAMEPLAYHINT'
  0x00994034  at 00534d0c  'STORYHINT0'
  0x00994028  at 00534d3d  'STORYHINT1'
direct callees (73):
  0x00733540  CExoString::Constructor
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x007337C0  CExoString::operator=
  0x00734190  CExoString::Find_2
  0x00919F10  _strlen
  0x00734870  CExoString::Right
  0x00733780  CExoString::~CExoString
  0x00740540  CClientExoApp::SetLoadScreenByModuleName
  0x00740DC0  (unnamed)
  0x00740980  CClientExoApp::SetLoadScreenHint
  0x0073FF20  CClientExoApp::ShowLoadScreen
  0x007405D0  CClientExoApp::SetLoadBarProgress
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x004113A0  CSWGuiManager::Draw
  0x00409ED0  (unnamed)
  0x00736200  (unnamed)
  0x0051C2E0  (unnamed)
  0x006371B0  (unnamed)
  0x00530660  SaveModuleForGameInProgress
  0x005FD440  CSWPartyTable::Save
struct offsets touched (top 10):
  +0x4 [dec 4]  x38  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x14 [dec 20]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x1  via ECX
  +0x30 [dec 48]  x1  via ECX
  +0x48 [dec 72]  x1  via ECX
  +0xC8 [dec 200]  x1  via EAX
  +0xCC [dec 204]  x1  via EDX
  +0x148 [dec 328]  x1  via EAX
  +0xF78 [dec 3960]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00534600 FUN_00534600 size=4064 ret=0 callers=1 callees=73

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00534600(int param_1)

{
  char cVar1;
  undefined1 uVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int *piVar7;
  undefined8 uVar8;
  undefined4 uVar9;
  undefined1 *puVar10;
  char *pcVar11;
  undefined4 *puVar12;
  int local_230;
  undefined4 *local_220;
  size_t local_1a8;
  size_t local_12c;
  undefined4 local_124;
  undefined4 local_120;
  undefined4 local_11c;
  int local_118;
  int local_114;
  undefined1 local_110 [8];
  int local_108;
  int local_104;
  undefined4 *local_100;
  undefined4 *local_fc;
  undefined1 local_f8 [8];
  undefined1 local_f0 [16];
  undefined1 local_e0 [16];
  undefined1 local_d0 [8];
  undefined1 local_c8 [8];
  undefined1 local_c0 [8];
  int local_b8;
  undefined4 *local_b4;
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [16];
  int local_80;
  undefined4 *local_7c;
  undefined4 local_78;
  byte local_71;
  undefined1 local_70 [8];
  int *local_68;
  undefined1 local_64 [8];
  undefined4 local_5c;
  char *local_58 [2];
  undefined1 local_50 [8];
  undefined1 local_48 [8];
  int local_40;
  int *local_3c;
  undefined4 local_38;
  int local_34;
  byte local_2d;
  undefined1 local_2c [8];
  undefined4 *local_24;
  int local_20;
  undefined1 local_1c [4];
  undefined4 local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094ec35;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_38 = 0xffffffff;
  FUN_00733540(uVar3);
  local_8 = 0;
  FUN_00733540();
/* ... 308 more lines truncated by --max-lines */
```

### decoy: 0x005345b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005345b0: 16-byte aligned: YES
  005345b0  55             PUSH EBP
  005345b1  8B EC          MOV EBP,ESP
  005345b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005345b0
string literals (0):
direct callees (3):
  0x0091C860  _ftol2_sse
  0x007405D0  CClientExoApp::SetLoadBarProgress
  0x00409ED0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-3-09

GOG function: CServerExoAppInternal::GetActivePauseState  gog 0x007bc010  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x00538dc0 (size 49, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00538d00:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bbf50 CServerExoAppInternal::ReprocessExclusionListActions -> 0x00538d00  gate_class=AUTO
- GOG-order neighbour after:  0x007bc050 CServerExoAppInternal::GetActiveTimer -> 0x00538e00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bbf20 CServerExoAppInternal::GetPauseState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 5): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2*} | Steam {0x2*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00538dc0: 16-byte aligned: YES
  00538dc0  55             PUSH EBP
  00538dc1  8B EC          MOV EBP,ESP
  00538dc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00538dc0
string literals (0):
direct callees (1):
  0x00538CD0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00538dc0 FUN_00538dc0 size=49 ret=0 callers=5 callees=1

undefined4 FUN_00538dc0(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_00538cd0(2);
  if (iVar1 == 0) {
    iVar1 = FUN_00538cd0(1);
    if (iVar1 == 0) {
      uVar2 = 0;
    }
    else {
      uVar2 = 1;
    }
  }
  else {
    uVar2 = 2;
  }
  return uVar2;
}
```

### decoy: 0x00538d00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00538d00: 16-byte aligned: YES
  00538d00  55             PUSH EBP
  00538d01  8B EC          MOV EBP,ESP
  00538d03  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00538d00
string literals (0):
direct callees (6):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00538E00  (unnamed)
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0058C370  (unnamed)
struct offsets touched (top 4):
  +0x8 [dec 8]  x1  via EDX
  +0x1005C [dec 65628]  x1  via EAX
  +0x10070 [dec 65648]  x1  via ECX
  +0x10074 [dec 65652]  x1  via EAX
```

## Card B9-3-10

GOG function: CServerExoApp::GetActiveExclusionList  gog 0x0064c210  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c740 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c720:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c1e0 CServerExoApp::RemoveFromExclusionList -> 0x0051c790  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c230 CServerExoApp::ResolvePlayerByFirstName -> 0x0051c810  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bc850 CServerExoAppInternal::GetActiveExclusionList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c740: 16-byte aligned: YES
  0051c740  55             PUSH EBP
  0051c741  8B EC          MOV EBP,ESP
  0051c743  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c740
string literals (0):
direct callees (1):
  0x00538DC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c740 FUN_0051c740 size=22 ret=0 callers=3 callees=1
THUNK -> 0x00538DC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c740(undefined4 param_1)

{
  FUN_00538dc0(param_1);
  return;
}
```

### decoy: 0x0051c720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c720: 16-byte aligned: YES
  0051c720  55             PUSH EBP
  0051c721  8B EC          MOV EBP,ESP
  0051c723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c720
string literals (0):
direct callees (1):
  0x00538CD0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-3-11

GOG function: CServerExoApp::SetMoveToPausedTimeOfDay  gog 0x0064c9c0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051cfa0 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cf50:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c9a0 CServerExoApp::GetMoveToPausedCalendarDay -> 0x0051cf80  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c9f0 CServerExoApp::GetMoveToPausedTimeOfDay -> 0x0051cfd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051cfa0: 16-byte aligned: YES
  0051cfa0  55             PUSH EBP
  0051cfa1  8B EC          MOV EBP,ESP
  0051cfa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cfa0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100AC [dec 65708]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cfa0 FUN_0051cfa0 size=36 ret=4 callers=2 callees=0

void __thiscall FUN_0051cfa0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x100ac) = param_2;
  return;
}
```

### decoy: 0x0051cf50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cf50: 16-byte aligned: YES
  0051cf50  55             PUSH EBP
  0051cf51  8B EC          MOV EBP,ESP
  0051cf53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cf50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B0 [dec 65712]  x1  via EDX
```

## Card B9-3-12

GOG function: CServerExoApp::ExecuteLoadModule  gog 0x0064cab0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051d090 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cfa0:STR0/CALLEE1/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ca90 CServerExoApp::GetModulePreviouslyVisited -> 0x0051d070  gate_class=REVIEW
- GOG-order neighbour after:  0x0064cad0 CServerExoApp::SetSaveGameName -> 0x0051d0b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b34f0 CServerExoAppInternal::ExecuteLoadModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051d090: 16-byte aligned: YES
  0051d090  55             PUSH EBP
  0051d091  8B EC          MOV EBP,ESP
  0051d093  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051d090
string literals (0):
direct callees (1):
  0x00530170  CServerExoAppInternal::ExecuteLoadModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051d090 FUN_0051d090 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00530170 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051d090(undefined4 param_1)

{
  FUN_00530170(param_1);
  return;
}
```

### decoy: 0x0051cfa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cfa0: 16-byte aligned: YES
  0051cfa0  55             PUSH EBP
  0051cfa1  8B EC          MOV EBP,ESP
  0051cfa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cfa0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100AC [dec 65708]  x1  via EDX
```

## Card B9-3-13

GOG function: CSWSModule::SaveModuleIFOFinish  gog 0x0072f300  callconv=(blank)  param_size_bytes=20  gog_size=277  gog_retn=20
Candidate Steam address: 0x0055cb50 (size 277, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: rotation:0x0072ffa0:0.69>0.48
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: WriteResource_2 0x721bb0); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0055cc70:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0072e4c0 CSWSModule::SaveModuleIFOStart -> 0x0055bd10  gate_class=AUTO
- GOG-order neighbour after:  0x0072f420 CSWSModule::SaveModuleFAC -> 0x0055cc70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0062bb40 CERFFile::WriteResource_2 -> Steam 0x00721bb0  in candidate callees: yes
  0x0072f830 CSWSModule::SavePlayers -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'Module'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x4*,0x14*,0x34*,0x7de*} | Steam {0x2*,0x4*,0x14*,0x34*,0x7de*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x0055cb50: 16-byte aligned: YES
  0055cb50  55             PUSH EBP
  0055cb51  8B EC          MOV EBP,ESP
  0055cb53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0055cb50
string literals (1):
  0x00992DA0  at 0055cbcb  'Module'
direct callees (5):
  0x00733540  CExoString::Constructor
  0x0055D080  (unnamed)
  0x00721BB0  CERFFile::WriteResource_2
  0x009196FD  operator delete
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0055cb50 FUN_0055cb50 size=277 ret=20 callers=1 callees=5

undefined4
FUN_0055cb50(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

{
  uint uVar1;
  undefined4 uVar2;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00950158;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(uVar1);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  FUN_00733540();
  local_8 = CONCAT31(local_8._1_3_,2);
  FUN_0055d080(param_1,param_2,param_4,param_5);
  FUN_00721bb0("Module",0x7de,param_1,1,0xffffffff);
  if (param_1 == (undefined4 *)0x0) {
    uVar2 = 0;
  }
  else {
    uVar2 = (**(code **)*param_1)(1);
  }
  FUN_009196fd(param_2,uVar1,uVar2);
  local_8._0_1_ = 1;
  FUN_00733780();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x0055cc70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055cc70: 16-byte aligned: YES
  0055cc70  55             PUSH EBP
  0055cc71  8B EC          MOV EBP,ESP
  0055cc73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0055cc70
string literals (3):
  0x009949BC  at 0055cdc4  'FactionList'
  0x009949B4  at 0055cde9  'RepList'
  0x00994DD8  at 0055ce0e  'GAMEINPROGRESS:REPUTE'
direct callees (15):
  0x00733540  CExoString::Constructor
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x0091C630  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00719740  CResGFF::AddList
  0x00664D80  (unnamed)
  0x00664E40  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00734010  CExoString::CStr
  0x0071A5A0  CResGFF::WriteGFFFile
  0x009196FD  operator delete
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x10054 [dec 65620]  x1  via ECX
```

## Card B9-3-14

GOG function: CServerExoAppInternal::UpDatePlayerPosition  gog 0x007bbea0  callconv=(blank)  param_size_bytes=12  gog_size=66  gog_retn=12
Candidate Steam address: 0x00538c50 (size 66, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00538ca0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bbe70 CServerExoAppInternal::TogglePauseState -> 0x00538c20  gate_class=REVIEW
- GOG-order neighbour after:  0x007bbef0 CServerExoAppInternal::IsInjured -> 0x00538ca0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x006aabf0 CSWSObject::SetOrientation -> Steam ?  in candidate callees: n/a
  0x006aac30 CSWSObject::SetPosition -> Steam ?  in candidate callees: n/a
  0x007badc0 CServerExoAppInternal::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00538c50: 16-byte aligned: YES
  00538c50  55             PUSH EBP
  00538c51  8B EC          MOV EBP,ESP
  00538c53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00538c50
string literals (0):
direct callees (3):
  0x00537B70  (unnamed)
  0x00543F50  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00538c50 FUN_00538c50 size=66 ret=12 callers=1 callees=3

void FUN_00538c50(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  
  iVar1 = FUN_00537b70(param_1);
  if (iVar1 != 0) {
    FUN_00543f50(param_2,1,1,0);
    FUN_00543f10(param_3);
  }
  return;
}
```

### decoy: 0x00538ca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00538ca0: 16-byte aligned: YES
  00538ca0  55             PUSH EBP
  00538ca1  8B EC          MOV EBP,ESP
  00538ca3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00538ca0
string literals (0):
direct callees (2):
  0x00537B70  (unnamed)
  0x0058BC50  (unnamed)
struct offsets touched (top 0):
```

## Card B9-3-15

GOG function: CServerExoAppInternal::ClearLastModuleFound  gog 0x007b28f0  callconv=(blank)  param_size_bytes=0  gog_size=37  gog_retn=0
Candidate Steam address: 0x0052f570 (size 37, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0052f4c0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b2840 CServerExoAppInternal::GetModule -> 0x0052f4c0  gate_class=AUTO
- GOG-order neighbour after:  0x007b2920 CServerExoAppInternal::GetModuleName -> 0x0052f5a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x7f000000*} | Steam {0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0052f570: 16-byte aligned: YES
  0052f570  55             PUSH EBP
  0052f571  8B EC          MOV EBP,ESP
  0052f573  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052f570
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1F324 [dec 127780]  x1  via EAX
  +0x1F328 [dec 127784]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0052f570 FUN_0052f570 size=37 ret=0 callers=1 callees=0

void __fastcall FUN_0052f570(int param_1)

{
  *(undefined4 *)(param_1 + 0x1f324) = 0x7f000000;
  *(undefined4 *)(param_1 + 0x1f328) = 0;
  return;
}
```

### decoy: 0x0052f4c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052f4c0: 16-byte aligned: YES
  0052f4c0  55             PUSH EBP
  0052f4c1  8B EC          MOV EBP,ESP
  0052f4c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052f4c0
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 5):
  +0x1F328 [dec 127784]  x4  via EAX,ECX
  +0x10060 [dec 65632]  x3  via ECX
  +0x1F324 [dec 127780]  x3  via EAX,EDX
  +0x20 [dec 32]  x1  via EDX
  +0x1005C [dec 65628]  x1  via ECX
```

## Card B9-3-16

GOG function: CSWSModule::SaveModuleFAC  gog 0x0072f420  callconv=(blank)  param_size_bytes=0  gog_size=646  gog_retn=0
Candidate Steam address: 0x0055cc70 (size 646, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=SaveRepute | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 3 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'RepList'@009949b4; 'FactionList'@009949bc; 'GAMEINPROGRESS:REPUTE'@00994dd8; CALLEE 3 (callees: Constructor 0x716e60, CreateGFFFile 0x71a3d0, WriteGFFFile 0x71a5a0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0055cf00:STR0/CALLEE0/SIG0.134/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0072f300 CSWSModule::SaveModuleIFOFinish -> 0x0055cb50  gate_class=REVIEW
- GOG-order neighbour after:  0x0072f6b0 CSWSModule::SaveModuleInProgress -> 0x0055cf00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (15), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00622ff0 CResGFF::Constructor -> Steam 0x00716e60  in candidate callees: yes
  0x006258a0 CResGFF::AddList -> Steam 0x00719740  in candidate callees: yes
  0x00626530 CResGFF::CreateGFFFile -> Steam 0x0071a3d0  in candidate callees: yes
  ... 7 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (3): 'RepList'; 'FactionList'; 'GAMEINPROGRESS:REPUTE'
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0xc*,0x8c*,0xa0*,0x7f6*} | Steam {0x2*,0x3*,0x4*,0xc*,0x8c*,0xa0*,0x7f6*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0055cc70: 16-byte aligned: YES
  0055cc70  55             PUSH EBP
  0055cc71  8B EC          MOV EBP,ESP
  0055cc73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0055cc70
string literals (3):
  0x009949BC  at 0055cdc4  'FactionList'
  0x009949B4  at 0055cde9  'RepList'
  0x00994DD8  at 0055ce0e  'GAMEINPROGRESS:REPUTE'
direct callees (15):
  0x00733540  CExoString::Constructor
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x0091C630  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00719740  CResGFF::AddList
  0x00664D80  (unnamed)
  0x00664E40  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00734010  CExoString::CStr
  0x0071A5A0  CResGFF::WriteGFFFile
  0x009196FD  operator delete
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x10054 [dec 65620]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0055cc70 FUN_0055cc70 size=646 ret=0 callers=1 callees=15

void FUN_0055cc70(void)

{
  undefined4 uVar1;
  void *local_94;
  undefined4 *local_90;
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [8];
  void *local_50;
  size_t local_4c;
  int local_48;
  undefined4 *local_44;
  undefined4 local_40;
  undefined4 *local_34;
  int local_30;
  void *local_2c;
  undefined1 local_28 [20];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009501ab;
  local_10 = ExceptionList;
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(local_14);
  local_8 = 0;
  local_40 = *(undefined4 *)(*(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4) + 0x10054);
  local_48 = FUN_00919723(0xa0);
  local_8._0_1_ = 1;
  if (local_48 == 0) {
    local_90 = (undefined4 *)0x0;
  }
  else {
    local_90 = (undefined4 *)FUN_00716e60();
  }
  local_44 = local_90;
  local_8 = (uint)local_8._1_3_ << 8;
  local_34 = local_90;
  local_4c = 4;
  local_50 = (void *)FUN_00919723(4);
  if (local_50 == (void *)0x0) {
    local_94 = (void *)0x0;
  }
  else {
    _memset [GHIDRA-AUTO-LABEL: unreliable](local_50,0,local_4c);
    local_94 = local_50;
  }
  local_2c = local_94;
  FUN_00733570(&DAT_009938e4);
  local_8._0_1_ = 2;
  FUN_00733570(&DAT_009949c8);
  local_8._0_1_ = 3;
  local_30 = FUN_0071a3d0(local_2c,local_60,local_58);
  local_8._0_1_ = 2;
  FUN_00733780();
  local_8._0_1_ = 0;
  FUN_00733780();
  if (local_30 == 1) {
    FUN_00719740(local_28,local_2c,"FactionList");
    FUN_00664d80(local_34,local_28);
    FUN_00719740(local_28,local_2c,"RepList");
    FUN_00664e40(local_34,local_28);
    FUN_007338d0("GAMEINPROGRESS:REPUTE");
    uVar1 = FUN_00734010();
    FUN_00733570(uVar1);
    local_8._0_1_ = 4;
    FUN_0071a5a0(local_68,0x7f6);
    local_8._0_1_ = 0;
    FUN_00733780();
    if (local_34 != (undefined4 *)0x0) {
      (**(code **)*local_34)(1);
    }
  }
  else if (local_34 != (undefined4 *)0x0) {
    (**(code **)*local_34)(1);
/* ... 8 more lines truncated by --max-lines */
```

### decoy: 0x0055cf00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055cf00: 16-byte aligned: YES
  0055cf00  55             PUSH EBP
  0055cf01  8B EC          MOV EBP,ESP
  0055cf03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0055cf00
string literals (2):
  0x00994DF8  at 0055cf91  'tmparea'
  0x00994DF0  at 0055cf9e  'tmpgit'
direct callees (8):
  0x00733540  CExoString::Constructor
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x0051C120  (unnamed)
  0x007337C0  CExoString::operator=
  0x00529DD0  CSWSArea::SaveGIT
  0x0051C2E0  (unnamed)
  0x00637270  (unnamed)
struct offsets touched (top 6):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x14 [dec 20]  x2  via EDX
  +0x208 [dec 520]  x2  via EAX
  +0xC [dec 12]  x1  via ECX
  +0x40 [dec 64]  x1  via EAX
  +0x214 [dec 532]  x1  via EAX
```

## Card B9-3-17

GOG function: CServerExoApp::GetMoveToPausedCalendarDay  gog 0x0064c9a0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051cf80 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cfd0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c970 CServerExoApp::SetMoveToPausedCalendarDay -> 0x0051cf50  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c9c0 CServerExoApp::SetMoveToPausedTimeOfDay -> 0x0051cfa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051cf80: 16-byte aligned: YES
  0051cf80  55             PUSH EBP
  0051cf81  8B EC          MOV EBP,ESP
  0051cf83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cf80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B0 [dec 65712]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cf80 FUN_0051cf80 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0051cf80(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x100b0);
}
```

### decoy: 0x0051cfd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cfd0: 16-byte aligned: YES
  0051cfd0  55             PUSH EBP
  0051cfd1  8B EC          MOV EBP,ESP
  0051cfd3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cfd0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100AC [dec 65708]  x1  via ECX
```

## Card B9-3-18

GOG function: CSWSModule::AddObjectToLimbo  gog 0x00731ee0  callconv=(blank)  param_size_bytes=4  gog_size=101  gog_retn=4
Candidate Steam address: 0x0055f850 (size 101, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0055f8c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00731e70 CSWSModule::GetTime -> 0x0055f7e0  gate_class=AUTO
- GOG-order neighbour after:  0x00731f50 CSWSModule::RemoveObjectFromLimbo -> 0x0055f8c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1fc*,0x200*} | Steam {0x1fc*,0x200*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x1fc*} | Steam {0x4*,0x10*,0x1fc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0055f850: 16-byte aligned: YES
  0055f850  55             PUSH EBP
  0055f851  8B EC          MOV EBP,ESP
  0055f853  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0055f850
string literals (0):
direct callees (1):
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 2):
  +0x1FC [dec 508]  x1  via ECX
  +0x200 [dec 512]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0055f850 FUN_0055f850 size=101 ret=4 callers=1 callees=1

void __thiscall FUN_0055f850(int param_1,int param_2)

{
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x200) <= local_c) {
      FUN_0083ea60(param_2);
      return;
    }
    if (param_2 == *(int *)(*(int *)(param_1 + 0x1fc) + local_c * 4)) break;
    local_c = local_c + 1;
  }
  return;
}
```

### decoy: 0x0055f8c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055f8c0: 16-byte aligned: YES
  0055f8c0  55             PUSH EBP
  0055f8c1  8B EC          MOV EBP,ESP
  0055f8c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0055f8c0
string literals (0):
direct callees (1):
  0x0049B840  (unnamed)
struct offsets touched (top 2):
  +0x1FC [dec 508]  x1  via ECX
  +0x200 [dec 512]  x1  via EAX
```

## Card B9-3-19

GOG function: CServerExoAppInternal::StartServices  gog 0x007b96e0  callconv=(blank)  param_size_bytes=0  gog_size=2058  gog_retn=4
Candidate Steam address: 0x005363a0 (size 2058, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.90) | G8 mac_rotation: pass
- anchors: A-STR 5 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'swserver'@009940c0; 'HD0:dialog'@0099412c; 'HD0:TEMP'@00994138; CALLEE 7 (callees: Constructor 0x711360, AddFixedKeyTableFile 0x711670, AddResourceDirectory 0x711690); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00536bb0:STR0/CALLEE0/SIG0.014/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007b9510 CServerExoAppInternal::RunModule -> 0x005361d0  gate_class=REVIEW
- GOG-order neighbour after:  0x007b9ef0 CServerExoAppInternal::StopServices -> 0x00536c10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (30), mapped to Steam through trusted rows only:
  0x00469300 CGameObjectArray::Constructor -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00548240 CServerInfo::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00608350 CExoTimers::GetHighResolutionTimer -> Steam ?  in candidate callees: n/a
  0x0060c4a0 CExoDebug::OpenLogFiles -> Steam ?  in candidate callees: n/a
  ... 22 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (9): 'TEMP:'; 'OVERRIDE:'; 'SERVERVAULT:'; 'GAMEINPROGRESS:' ...
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 36 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+22)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+22)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x005363a0: 16-byte aligned: YES
  005363a0  55             PUSH EBP
  005363a1  8B EC          MOV EBP,ESP
  005363a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005363a0
string literals (9):
  0x0099415C  at 00536741  'OVERRIDE:'
  0x00994150  at 0053676d  'ERRORTEX:'
  0x00994144  at 00536799  'HD0:chitin'
  0x00993EF0  at 005367c5  'SERVERVAULT:'
  0x00994138  at 005367fa  'HD0:TEMP'
  0x00993E5C  at 00536831  'TEMP:'
  0x00993E84  at 00536866  'GAMEINPROGRESS:'
  0x0099412C  at 005368fe  'HD0:dialog'
  0x009940C0  at 00536a14  'swserver'
direct callees (30):
  0x00733540  CExoString::Constructor
  0x0073A590  (unnamed)
  0x00734010  CExoString::CStr
  0x00919723  operator new
  0x00664280  (unnamed)
  0x0051A770  (unnamed)
  0x0053D690  CGameObjectArray::Constructor
  0x00711360  CExoResMan::Constructor
  0x00703CA0  CNetLayer::CNetLayer (ctor, object 0x2007C B)
  0x007EB5C0  CServerInfo ctor (MinLevel +0x90=1, MaxLevel +0x94=50)
  0x00656780  (unnamed)
  0x0051D2F0  (unnamed)
  0x007A2380  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00711690  CExoResMan::AddResourceDirectory
  0x00733780  CExoString::~CExoString
  0x00711670  CExoResMan::AddFixedKeyTableFile
  0x007158E0  CExoResMan::RemoveFile
  0x00714F30  CExoResMan::CreateDirectory2
  0x0071D630  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x2  via ECX
  +0x10004 [dec 65540]  x2  via ECX,EDX
  +0x1000C [dec 65548]  x2  via EAX,ECX
  +0x10044 [dec 65604]  x2  via ECX
  +0x4 [dec 4]  x1  via EDX
  +0x10010 [dec 65552]  x1  via ECX
  +0x10038 [dec 65592]  x1  via EDX
  +0x1003C [dec 65596]  x1  via EDX
  +0x10040 [dec 65600]  x1  via EAX
  +0x10048 [dec 65608]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005363a0 FUN_005363a0 size=2058 ret=4 callers=1 callees=30

void __fastcall FUN_005363a0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined8 uVar3;
  undefined4 local_10c;
  undefined4 *local_108;
  undefined4 local_104;
  int local_100;
  undefined4 local_fc;
  undefined4 local_f8;
  undefined4 local_f4;
  undefined4 local_f0;
  undefined4 local_ec;
  undefined4 local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined1 local_c8 [8];
  undefined4 *local_c0;
  undefined4 *local_bc;
  int local_b8;
  undefined4 local_b4;
  undefined1 local_b0 [8];
  int local_a8;
  int local_a4;
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  int local_68;
  undefined4 local_64;
  int local_60;
  undefined4 local_5c;
  int local_58;
  undefined4 local_54;
  int local_50;
  undefined4 local_4c;
  int local_48;
  undefined4 local_44;
  int local_40;
  undefined4 local_3c;
  int local_38;
  undefined4 local_34;
  int local_30;
  undefined4 local_2c;
  int local_28;
  undefined4 local_24;
  int local_20;
  undefined4 local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094ee0a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_0073a590(local_18);
  FUN_00734010();
  local_20 = FUN_00919723(0x28);
  local_8._0_1_ = 1;
  if (local_20 == 0) {
    local_d8 = 0;
  }
  else {
    local_d8 = FUN_00664280();
  }
  local_1c = local_d8;
  local_8._0_1_ = 0;
  *(undefined4 *)(param_1 + 0x10054) = local_d8;
/* ... 224 more lines truncated by --max-lines */
```

### decoy: 0x00536bb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00536bb0: 16-byte aligned: YES
  00536bb0  55             PUSH EBP
  00536bb1  8B EC          MOV EBP,ESP
  00536bb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00536bb0
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B9-3-20

GOG function: CServerExoAppInternal::GetModuleName  gog 0x007b2920  callconv=(blank)  param_size_bytes=4  gog_size=103  gog_retn=4
Candidate Steam address: 0x0052f5a0 (size 103, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0052f330:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b28f0 CServerExoAppInternal::ClearLastModuleFound -> 0x0052f570  gate_class=REVIEW
- GOG-order neighbour after:  0x007b2990 CServerExoAppInternal::SaveGame -> 0x0052f610  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x007b2840 CServerExoAppInternal::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x104*} | Steam {0x4*,0x10*,0x104*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0052f5a0: 16-byte aligned: YES
  0052f5a0  55             PUSH EBP
  0052f5a1  8B EC          MOV EBP,ESP
  0052f5a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052f5a0
string literals (0):
direct callees (3):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x007335F0  CExoString::CExoString_copy
  0x00733540  CExoString::Constructor
struct offsets touched (top 1):
  +0x10004 [dec 65540]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0052f5a0 FUN_0052f5a0 size=103 ret=4 callers=1 callees=3

undefined4 __thiscall FUN_0052f5a0(int param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = FUN_0052f4c0();
  if (iVar1 == 0) {
    FUN_00733540();
  }
  else {
    FUN_007335f0(*(int *)(param_1 + 0x10004) + 0x104);
  }
  return param_2;
}
```

### decoy: 0x0052f330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052f330: 16-byte aligned: YES
  0052f330  55             PUSH EBP
  0052f331  8B EC          MOV EBP,ESP
  0052f333  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052f330
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00632A20  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0x10064 [dec 65636]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-3-21

GOG function: CServerExoApp::SetSaveGameName  gog 0x0064cad0  callconv=(blank)  param_size_bytes=4  gog_size=42  gog_retn=4
Candidate Steam address: 0x0051d0b0 (size 42, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0051cfa0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064cab0 CServerExoApp::ExecuteLoadModule -> 0x0051d090  gate_class=REVIEW
- GOG-order neighbour after:  0x0064cb00 CServerAIList::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x1f348*} | Steam {0x4*,0x8*,0x1f348*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051d0b0: 16-byte aligned: YES
  0051d0b0  55             PUSH EBP
  0051d0b1  8B EC          MOV EBP,ESP
  0051d0b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051d0b0
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051d0b0 FUN_0051d0b0 size=42 ret=4 callers=1 callees=1
THUNK -> 0x007337C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051d0b0(undefined4 param_1)

{
  FUN_007337c0(param_1);
  return;
}
```

### decoy: 0x0051cfa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cfa0: 16-byte aligned: YES
  0051cfa0  55             PUSH EBP
  0051cfa1  8B EC          MOV EBP,ESP
  0051cfa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cfa0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100AC [dec 65708]  x1  via EDX
```

## Card B9-3-22

GOG function: CServerExoAppInternal::GetItemByGameObjectID  gog 0x007bad50  callconv=(blank)  param_size_bytes=4  gog_size=105  gog_retn=4
Candidate Steam address: 0x00537b00 (size 105, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00537b70:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007baac0 CServerExoAppInternal::Shutdown -> 0x00537870  gate_class=REVIEW
- GOG-order neighbour after:  0x007badc0 CServerExoAppInternal::GetCreatureByGameObjectID -> 0x00537b70  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x38*} | Steam {0x8*,0x38*}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00537b00: 16-byte aligned: YES
  00537b00  55             PUSH EBP
  00537b01  8B EC          MOV EBP,ESP
  00537b03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00537b00
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x38 [dec 56]  x1  via EDX
  +0x1005C [dec 65628]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00537b00 FUN_00537b00 size=105 ret=4 callers=1 callees=1

undefined4 __thiscall FUN_00537b00(int param_1,undefined4 param_2)

{
  char cVar1;
  undefined4 uVar2;
  int *local_8;
  
  local_8 = (int *)0x0;
  uVar2 = *(undefined4 *)(param_1 + 0x1005c);
  cVar1 = FUN_0053dfb0(param_2,&local_8);
  if (((cVar1 == DAT_0099447f) && (local_8 != (int *)0x0)) && ((char)local_8[2 = byte +0x8] == DAT_00994475)) {
    uVar2 = (**(code **)(*local_8 + 0x38))(param_1,uVar2);
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00537b70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00537b70: 16-byte aligned: YES
  00537b70  55             PUSH EBP
  00537b71  8B EC          MOV EBP,ESP
  00537b73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00537b70
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x30 [dec 48]  x1  via EDX
  +0x1005C [dec 65628]  x1  via EAX
```

## Card B9-3-23

GOG function: CSWSModule::SavePrimaryPlayerInfo  gog 0x0072ffa0  callconv=(blank)  param_size_bytes=8  gog_size=775  gog_retn=8
Candidate Steam address: 0x0055d7f0 (size 775, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=SaveMultiplayerPlayer | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 1 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'Player'@00994e00; CALLEE 5 (callees: Constructor 0x716e60, WriteFieldBYTE 0x7199d0, WriteFieldCExoString 0x719e50); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0055d080:STR0/CALLEE2/SIG0.405/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0072f830 CSWSModule::SavePlayers -> 0x0055d080  gate_class=REVIEW
- GOG-order neighbour after:  0x007302b0 CSWSModule::SaveStatic -> 0x0055db00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (16), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x00622ff0 CResGFF::Constructor -> Steam 0x00716e60  in candidate callees: yes
  0x00625b30 CResGFF::WriteFieldBYTE -> Steam 0x007199d0  in candidate callees: yes
  0x00625cf0 CResGFF::WriteFieldDWORD -> Steam 0x00719b90  in candidate callees: yes
  ... 8 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (4): 'Player'; 'ObjectId'; 'Mod_IsPrimaryPlr'; 'Mod_CommntyName'
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x3c*} | Steam {0x4*,0x8*,0x3c*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0xc*,0xa0*,0x7df*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0xc*,0xa0*,0x7df*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0055d7f0: 16-byte aligned: YES
  0055d7f0  55             PUSH EBP
  0055d7f1  8B EC          MOV EBP,ESP
  0055d7f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0055d7f0
string literals (4):
  0x00994A3C  at 0055d96d  'Mod_CommntyName'
  0x00994A08  at 0055d99a  'Mod_IsPrimaryPlr'
  0x00992EB4  at 0055d9bc  'ObjectId'
  0x00994E00  at 0055d9e4  'Player'
direct callees (16):
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x0091C630  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::Constructor
  0x00632A90  (unnamed)
  0x00719E50  CResGFF::WriteFieldCExoString
  0x007199D0  CResGFF::WriteFieldBYTE
  0x00719B90  CResGFF::WriteFieldDWORD
  0x00574500  CSWSCreature::SaveCreature
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x0071A5A0  CResGFF::WriteGFFFile
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x3C [dec 60]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0055d7f0 FUN_0055d7f0 size=775 ret=8 callers=1 callees=16

void FUN_0055d7f0(int param_1)

{
  uint uVar1;
  undefined4 uVar2;
  void *local_94;
  undefined4 *local_90;
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [8];
  undefined1 local_50 [8];
  undefined1 local_48 [8];
  undefined1 local_40 [8];
  int local_38;
  void *local_34;
  size_t local_30;
  int local_2c;
  undefined4 *local_28;
  undefined1 local_24 [8];
  undefined4 *local_1c;
  int local_18;
  void *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009502db;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = FUN_0051c600(*(undefined4 *)(param_1 + 4));
  local_2c = FUN_00919723(0xa0,uVar1);
  local_8 = 0;
  if (local_2c == 0) {
    local_90 = (undefined4 *)0x0;
  }
  else {
    local_90 = (undefined4 *)FUN_00716e60();
  }
  local_28 = local_90;
  local_8 = 0xffffffff;
  local_1c = local_90;
  local_30 = 4;
  local_34 = (void *)FUN_00919723(4,uVar1);
  if (local_34 == (void *)0x0) {
    local_94 = (void *)0x0;
  }
  else {
    _memset [GHIDRA-AUTO-LABEL: unreliable](local_34,0,local_30);
    local_94 = local_34;
  }
  local_14 = local_94;
  FUN_00733570(&DAT_009938e4);
  local_8 = 1;
  FUN_00733570(&DAT_00993ed8);
  local_8._0_1_ = 2;
  local_38 = FUN_0071a3d0(local_14,local_48,local_40);
  local_8 = CONCAT31(local_8._1_3_,1);
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  if (local_38 != 0) {
    FUN_00733540();
    local_8 = 3;
    uVar2 = FUN_00632a90(local_50);
    local_8._0_1_ = 4;
    FUN_00719e50(local_14,uVar2,"Mod_CommntyName");
    local_8._0_1_ = 3;
    FUN_00733780();
    FUN_007199d0(local_14,*(uint *)(local_18 + 0x3c) & 0xff,"Mod_IsPrimaryPlr");
    FUN_00719b90(local_14,*(undefined4 *)(param_1 + 4),"ObjectId");
    FUN_00574500(local_1c,local_14);
    FUN_00733570("Player");
    local_8._0_1_ = 5;
    FUN_00733570(&DAT_00993e10);
    local_8._0_1_ = 6;
    FUN_00733df0(local_60,local_58);
/* ... 22 more lines truncated by --max-lines */
```

### decoy: 0x0055d080 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055d080: 16-byte aligned: YES
  0055d080  55             PUSH EBP
  0055d081  8B EC          MOV EBP,ESP
  0055d083  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0055d080
string literals (14):
  0x00993E6C  at 0055d0f1  'Mod_PlayerList'
  0x00994A3C  at 0055d283  'Mod_CommntyName'
  0x00994A08  at 0055d2b6  'Mod_IsPrimaryPlr'
  0x00994A2C  at 0055d2de  'Mod_FirstName'
  0x00994A1C  at 0055d308  'Mod_LastName'
  0x00992EB4  at 0055d32f  'ObjectId'
  0x00992DA0  at 0055d382  'Module'
  0x00992EB4  at 0055d544  'ObjectId'
  0x00994A3C  at 0055d574  'Mod_CommntyName'
  0x00994A3C  at 0055d67e  'Mod_CommntyName'
  0x00994A08  at 0055d69f  'Mod_IsPrimaryPlr'
  0x00994A2C  at 0055d6c7  'Mod_FirstName'
direct callees (41):
  0x0051C080  CServerExoApp::GetObjectArray
  0x0084A640  (unnamed)
  0x00719740  CResGFF::AddList
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x00631560  (unnamed)
  0x0055E1A0  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00719800  CResGFF::AddListElement
  0x00632A90  (unnamed)
  0x00719E50  CResGFF::WriteFieldCExoString
  0x00733780  CExoString::~CExoString
  0x007199D0  CResGFF::WriteFieldBYTE
  0x00719F30  CResGFF::WriteFieldCExoLocString
  0x00719B90  CResGFF::WriteFieldDWORD
  0x00574500  CSWSCreature::SaveCreature
  0x0058A090  CSWSCreature::DumpToLog
struct offsets touched (top 5):
  +0x4 [dec 4]  x6  via EAX,ECX
  +0x1198 [dec 4504]  x4  via EAX,ECX,EDX
  +0x3C [dec 60]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x30 [dec 48]  x1  via EDX
```

## Card B9-3-24

GOG function: CServerExoAppInternal::DealWithLoadGameError  gog 0x007b8970  callconv=(blank)  param_size_bytes=4  gog_size=159  gog_retn=4
Candidate Steam address: 0x00535630 (size 159, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005356d0:STR0/CALLEE0/SIG0.131/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b7980 CServerExoAppInternal::StallEventSaveGame -> 0x00534600  gate_class=REVIEW
- GOG-order neighbour after:  0x007b8a10 CServerExoAppInternal::InitiateModuleForPlayer -> 0x005356d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam ?  in candidate callees: n/a
  0x007b4960 CServerExoAppInternal::UnloadModule -> Steam ?  in candidate callees: n/a
  0x007cda00 CSWSMessage::SendServerToPlayerLoadBar_EndStallEvent -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x14*,0x38*} | Steam {0x4*,0x8*,0x14*,0x38*}
imm sets (* = in both, 3 common): GOG {0x2*,0x4*,0xc*} | Steam {0x2*,0x4*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00535630: 16-byte aligned: YES
  00535630  55             PUSH EBP
  00535631  8B EC          MOV EBP,ESP
  00535633  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00535630
string literals (0):
direct callees (4):
  0x005315E0  CServerExoAppInternal::UnloadModule
  0x00623320  CSWSEncounter::AIUpdate
  0x0051C2E0  (unnamed)
  0x006372E0  (unnamed)
struct offsets touched (top 6):
  +0x14 [dec 20]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x38 [dec 56]  x1  via EDX
  +0x10008 [dec 65544]  x1  via ECX
  +0x100B4 [dec 65716]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00535630 FUN_00535630 size=159 ret=4 callers=1 callees=4

void FUN_00535630(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  int extraout_var;
  int iVar3;
  
  puVar1 = *(undefined4 **)(DAT_00a1b4a4 + 0x14);
  FUN_005315e0();
  puVar1[0xe = byte +0x38] = param_1;
  puVar1[5 = byte +0x14] = 1;
  if (DAT_00a1b47d == '\x02') {
    puVar1[5 = byte +0x14] = 0;
    *puVar1 = 0;
    puVar1[1 = byte +0x4] = 0;
  }
  *(undefined2 *)(extraout_var + 0x10008) = 0;
  *(undefined4 *)(extraout_var + 0x100b4) = 0;
  iVar3 = extraout_var;
  FUN_00623320();
  uVar2 = 1;
  FUN_0051c2e0(1,param_1,iVar3,*(undefined4 *)(DAT_00a1b4a4 + 8));
  FUN_006372e0(uVar2,param_1);
  return;
}
```

### decoy: 0x005356d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005356d0: 16-byte aligned: YES
  005356d0  55             PUSH EBP
  005356d1  8B EC          MOV EBP,ESP
  005356d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005356d0
string literals (0):
direct callees (16):
  0x00632A20  (unnamed)
  0x00919723  operator new
  0x00561F30  CSWSCreature::Constructor
  0x00632830  (unnamed)
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EF0  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
  0x005453C0  CSWSObject::GetArea
  0x0051C120  (unnamed)
  0x00545420  CSWSObject::SetArea
  0x0051C2E0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x006305B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00637620  (unnamed)
  0x00639F40  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x58 [dec 88]  x5  via EAX
  +0x350 [dec 848]  x3  via EAX,ECX,EDX
  +0x124 [dec 292]  x2  via ECX,EDX
  +0x360 [dec 864]  x2  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
```

## Card B9-3-25

GOG function: CServerExoAppInternal::GetPauseState  gog 0x007bbf20  callconv=(blank)  param_size_bytes=1  gog_size=36  gog_retn=4
Candidate Steam address: 0x00538c20 (size 47, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00538cd0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bbef0 CServerExoAppInternal::IsInjured -> 0x00538ca0  gate_class=REVIEW
- GOG-order neighbour after:  0x007bbf50 CServerExoAppInternal::ReprocessExclusionListActions -> 0x00538d00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00538c20: 16-byte aligned: YES
  00538c20  55             PUSH EBP
  00538c21  8B EC          MOV EBP,ESP
  00538c23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00538c20
string literals (0):
direct callees (2):
  0x00538CD0  (unnamed)
  0x00538ED0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00538c20 FUN_00538c20 size=47 ret=4 callers=1 callees=2

void FUN_00538c20(undefined1 param_1)

{
  int iVar1;
  
  iVar1 = FUN_00538cd0(param_1);
  FUN_00538ed0(param_1,iVar1 == 0);
  return;
}
```

### decoy: 0x00538cd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00538cd0: 16-byte aligned: YES
  00538cd0  55             PUSH EBP
  00538cd1  8B EC          MOV EBP,ESP
  00538cd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00538cd0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10078 [dec 65656]  x1  via EAX
```

## Card B9-3-26

GOG function: CServerExoAppInternal::PlayerListChange  gog 0x007b8f70  callconv=(blank)  param_size_bytes=8  gog_size=1428  gog_retn=12
Candidate Steam address: 0x00535c30 (size 1428, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.57) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00535a10:STR0/CALLEE0/SIG0.133/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007b8d50 CServerExoAppInternal::MovePlayerToArea -> 0x00535a10  gate_class=REVIEW
- GOG-order neighbour after:  0x007b9510 CServerExoAppInternal::RunModule -> 0x005361d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (23), mapped to Steam through trusted rows only:
  0x00401b20 CAppManager::GetObjectTableManager -> Steam ?  in candidate callees: n/a
  0x0040c0e0 CObjectTableManager::RemovePlayer -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x0060c500 CExoDebug::GetCurrentTimestamp -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  ... 15 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x1c*,0x24*,0x3c*} | Steam {0x4*,0x8*,0x1c*,0x24*,0x3c*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x4*,0xc*,0x84*,0x98*,0x1001c*,0x100f8*} | Steam {0x2*,0x3*,0x4*,0xc*,0x84*,0x98*,0x1001c*,0x100f8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x00535c30: 16-byte aligned: YES
  00535c30  55             PUSH EBP
  00535c31  8B EC          MOV EBP,ESP
  00535c33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00535c30
string literals (0):
direct callees (23):
  0x00733540  CExoString::Constructor
  0x0073A590  (unnamed)
  0x00704800  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x00734010  CExoString::CStr
  0x00919723  operator new
  0x00665DC0  (unnamed)
  0x0054AA10  CExoLinkedList::AddHead
  0x005210F0  (unnamed)
  0x0062FA00  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
  0x00665CD0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0064DDE0  (unnamed)
  0x0058C370  (unnamed)
  0x00647A80  (unnamed)
  0x00401B60  (unnamed)
  0x0051B980  (unnamed)
struct offsets touched (top 10):
  +0x10064 [dec 65636]  x8  via EAX,ECX,EDX
  +0x10068 [dec 65640]  x7  via EAX,ECX,EDX
  +0x10040 [dec 65600]  x6  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via EDX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x24 [dec 36]  x2  via EAX,ECX
  +0x10010 [dec 65552]  x2  via EAX,ECX
  +0x3C [dec 60]  x1  via EDX
  +0x10020 [dec 65568]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00535c30 FUN_00535c30 size=1428 ret=12 callers=1 callees=23

void __thiscall FUN_00535c30(int param_1,int param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int local_a0;
  int local_9c;
  int local_3c;
  int local_34;
  undefined4 *local_30;
  int local_2c;
  int local_28;
  undefined4 local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094ecbe;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(uVar1);
  local_8 = 0;
  FUN_0073a590(local_18);
  if (param_3 == 1) {
    iVar2 = FUN_00704800(param_2);
    FUN_007335f0(iVar2 + 4);
    local_8._0_1_ = 1;
    local_1c = 0;
    FUN_00734010();
    iVar2 = FUN_00704800(param_2);
    if (*(int *)(iVar2 + 0x24) == 1) {
      iVar2 = FUN_00919723(0xc);
      local_8._0_1_ = 2;
      if (iVar2 == 0) {
        local_9c = 0;
      }
      else {
        local_9c = FUN_00665dc0(param_2);
      }
      local_8._0_1_ = 1;
      local_28 = local_9c;
      *(undefined4 *)(local_9c + 8) = local_1c;
      FUN_0054aa10(local_9c);
      FUN_00734010();
      FUN_00734010();
      FUN_005210f0();
    }
    iVar2 = FUN_00704800(param_2);
    if (*(int *)(iVar2 + 0x1c) == 1) {
      iVar2 = FUN_00919723(0x84);
      local_8._0_1_ = 3;
      if (iVar2 == 0) {
        local_a0 = 0;
      }
      else {
        local_a0 = FUN_0062fa00(param_2);
      }
      local_8._0_1_ = 1;
      local_2c = local_a0;
      *(undefined4 *)(local_a0 + 8) = local_1c;
      FUN_0054aa10(local_a0);
      FUN_00734010();
      FUN_00734010();
      FUN_005210f0();
    }
    FUN_00623320();
    FUN_00665cd0(DAT_0099b6cc);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
  }
  if (param_3 == 0) {
    iVar2 = FUN_00704800(param_2);
    FUN_007335f0(iVar2 + 4);
    local_8 = CONCAT31(local_8._1_3_,4);
    local_3c = 0;
/* ... 65 more lines truncated by --max-lines */
```

### decoy: 0x00535a10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00535a10: 16-byte aligned: YES
  00535a10  55             PUSH EBP
  00535a11  8B EC          MOV EBP,ESP
  00535a13  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00535a10
string literals (0):
direct callees (13):
  0x00632A20  (unnamed)
  0x00733DB0  CExoString::operator[]
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EC0  (unnamed)
  0x0051C1E0  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
  0x007338D0  CExoString::operator=_2
  0x00557EF0  (unnamed)
  0x0054BDC0  CSWSArea::ComputeSafeLocation
  0x005C2770  CSWSCreature::UpdateSubareasOnJumpPosition
  0x00568BD0  CSWSCreature::AddToArea
  0x005D0F30  CSWSObject::UpdatePerception
  0x005F9A20  CSWPartyTable::PositionParty
struct offsets touched (top 8):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x380 [dec 896]  x2  via ECX,EDX
  +0x100C4 [dec 65732]  x2  via EAX,EDX
  +0x2C [dec 44]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x350 [dec 848]  x1  via EDX
```

## Card B9-3-27

GOG function: CSWSModule::RemoveObjectFromLimbo  gog 0x00731f50  callconv=(blank)  param_size_bytes=4  gog_size=101  gog_retn=4
Candidate Steam address: 0x0055f8c0 (size 101, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0055f850:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00731ee0 CSWSModule::AddObjectToLimbo -> 0x0055f850  gate_class=REVIEW
- GOG-order neighbour after:  0x00731fc0 CSWSModule::IsObjectInLimbo -> 0x0055f930  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1fc*,0x200*} | Steam {0x1fc*,0x200*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x1fc*} | Steam {0x4*,0x10*,0x1fc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0055f8c0: 16-byte aligned: YES
  0055f8c0  55             PUSH EBP
  0055f8c1  8B EC          MOV EBP,ESP
  0055f8c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0055f8c0
string literals (0):
direct callees (1):
  0x0049B840  (unnamed)
struct offsets touched (top 2):
  +0x1FC [dec 508]  x1  via ECX
  +0x200 [dec 512]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0055f8c0 FUN_0055f8c0 size=101 ret=4 callers=1 callees=1

void __thiscall FUN_0055f8c0(int param_1,int param_2)

{
  int local_8;
  
  local_8 = *(int *)(param_1 + 0x200);
  do {
    local_8 = local_8 + -1;
    if (local_8 < 0) {
      return;
    }
  } while (param_2 != *(int *)(*(int *)(param_1 + 0x1fc) + local_8 * 4));
  FUN_0049b840(local_8);
  return;
}
```

### decoy: 0x0055f850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055f850: 16-byte aligned: YES
  0055f850  55             PUSH EBP
  0055f851  8B EC          MOV EBP,ESP
  0055f853  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0055f850
string literals (0):
direct callees (1):
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 2):
  +0x1FC [dec 508]  x1  via ECX
  +0x200 [dec 512]  x1  via ECX
```

## Card B9-3-28

GOG function: CServerExoApp::GetMoveToWorldTime  gog 0x0064ca50  callconv=(blank)  param_size_bytes=28  gog_size=52  gog_retn=28
Candidate Steam address: 0x0051d030 (size 52, retn 28)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cff0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ca10 CServerExoApp::SetMoveToWorldTime -> 0x0051cff0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064ca90 CServerExoApp::GetModulePreviouslyVisited -> 0x0051d070  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bfa90 CServerExoAppInternal::GetMoveToWorldTime -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}

### abi
```
GOG param bytes: 28; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(28/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=28 ; asm RET = 28
Stack Slot Match: TRUE (GOG 28 bytes -> x86 slots 28; ASM RET 28)
```

### prologue
```
prologue for 0x0051d030: 16-byte aligned: YES
  0051d030  55             PUSH EBP
  0051d031  8B EC          MOV EBP,ESP
  0051d033  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051d030
string literals (0):
direct callees (1):
  0x0053CCF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051d030 FUN_0051d030 size=52 ret=28 callers=1 callees=1
THUNK -> 0x0053CCF0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051d030(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  FUN_0053ccf0(param_1,param_2,param_3,param_4,param_5,param_6,param_7);
  return;
}
```

### decoy: 0x0051cff0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cff0: 16-byte aligned: YES
  0051cff0  55             PUSH EBP
  0051cff1  8B EC          MOV EBP,ESP
  0051cff3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cff0
string literals (0):
direct callees (1):
  0x0053CC80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-3-29

GOG function: CServerExoApp::SetIgnoreConversationStartRange  gog 0x0064c780  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051cd10 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: EventHandler 0x617b40, EventHandler 0x5f2fe0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0051cd40:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c760 CServerExoApp::GetActionConversation -> 0x0051cd40  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c7b0 CServerExoApp::TestSafeLocation -> 0x0051cd90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x00762ff0 CSWSDoor::EventHandler -> Steam 0x00617b40  in candidate callers: yes
  0x0076d3b0 CSWSPlaceable::EventHandler -> Steam 0x005f2fe0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051cd10: 16-byte aligned: YES
  0051cd10  55             PUSH EBP
  0051cd11  8B EC          MOV EBP,ESP
  0051cd13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cd10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1F31C [dec 127772]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cd10 FUN_0051cd10 size=36 ret=4 callers=4 callees=0

void __thiscall FUN_0051cd10(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x1f31c) = param_2;
  return;
}
```

### decoy: 0x0051cd40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cd40: 16-byte aligned: YES
  0051cd40  55             PUSH EBP
  0051cd41  8B EC          MOV EBP,ESP
  0051cd43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cd40
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1F31C [dec 127772]  x1  via ECX
```

## Card B9-3-30

GOG function: CServerExoAppInternal::TogglePauseState  gog 0x007bbe70  callconv=(blank)  param_size_bytes=1  gog_size=47  gog_retn=4
Candidate Steam address: 0x00538c20 (size 47, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00538cd0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bb700 CServerExoAppInternal::GetPlayerListString -> 0x005384b0  gate_class=AUTO
- GOG-order neighbour after:  0x007bbea0 CServerExoAppInternal::UpDatePlayerPosition -> 0x00538c50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007bbf20 CServerExoAppInternal::GetPauseState -> Steam ?  in candidate callees: n/a
  0x007bc120 CServerExoAppInternal::SetPauseState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00538c20: 16-byte aligned: YES
  00538c20  55             PUSH EBP
  00538c21  8B EC          MOV EBP,ESP
  00538c23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00538c20
string literals (0):
direct callees (2):
  0x00538CD0  (unnamed)
  0x00538ED0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00538c20 FUN_00538c20 size=47 ret=4 callers=1 callees=2

void FUN_00538c20(undefined1 param_1)

{
  int iVar1;
  
  iVar1 = FUN_00538cd0(param_1);
  FUN_00538ed0(param_1,iVar1 == 0);
  return;
}
```

### decoy: 0x00538cd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00538cd0: 16-byte aligned: YES
  00538cd0  55             PUSH EBP
  00538cd1  8B EC          MOV EBP,ESP
  00538cd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00538cd0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10078 [dec 65656]  x1  via EAX
```

## Card B9-3-31

GOG function: CServerExoApp::GetMoveToPausedTimeOfDay  gog 0x0064c9f0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051cfd0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cf80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c9c0 CServerExoApp::SetMoveToPausedTimeOfDay -> 0x0051cfa0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064ca10 CServerExoApp::SetMoveToWorldTime -> 0x0051cff0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051cfd0: 16-byte aligned: YES
  0051cfd0  55             PUSH EBP
  0051cfd1  8B EC          MOV EBP,ESP
  0051cfd3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cfd0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100AC [dec 65708]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cfd0 FUN_0051cfd0 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0051cfd0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x100ac);
}
```

### decoy: 0x0051cf80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cf80: 16-byte aligned: YES
  0051cf80  55             PUSH EBP
  0051cf81  8B EC          MOV EBP,ESP
  0051cf83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cf80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B0 [dec 65712]  x1  via ECX
```

## Card B9-3-32

GOG function: CServerExoApp::MainLoop  gog 0x0064be90  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c450 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c420:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064be70 CServerExoApp::Initialize -> 0x0051c450  gate_class=REVIEW
- GOG-order neighbour after:  0x0064beb0 CServerExoApp::PlayerListChange -> 0x0051c490  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b6bb0 CServerExoAppInternal::MainLoop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c450: 16-byte aligned: YES
  0051c450  55             PUSH EBP
  0051c451  8B EC          MOV EBP,ESP
  0051c453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c450
string literals (0):
direct callees (1):
  0x005315A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c450 FUN_0051c450 size=22 ret=0 callers=1 callees=1
THUNK -> 0x005315A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c450(undefined4 param_1)

{
  FUN_005315a0(param_1);
  return;
}
```

### decoy: 0x0051c420 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c420: 16-byte aligned: YES
  0051c420  55             PUSH EBP
  0051c421  8B EC          MOV EBP,ESP
  0051c423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c420
string literals (0):
direct callees (1):
  0x005314E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-3-33

GOG function: CServerExoAppInternal::ClearAreaCache  gog 0x007baee0  callconv=(blank)  param_size_bytes=0  gog_size=37  gog_retn=0
Candidate Steam address: 0x00537c90 (size 37, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00537be0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bae30 CServerExoAppInternal::GetAreaByGameObjectID -> 0x00537be0  gate_class=AUTO
- GOG-order neighbour after:  0x007baf10 CServerExoAppInternal::GetTriggerByGameObjectID -> 0x00537cc0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x7f000000*} | Steam {0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00537c90: 16-byte aligned: YES
  00537c90  55             PUSH EBP
  00537c91  8B EC          MOV EBP,ESP
  00537c93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00537c90
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1F32C [dec 127788]  x1  via ECX
  +0x1F330 [dec 127792]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00537c90 FUN_00537c90 size=37 ret=0 callers=1 callees=0

void __fastcall FUN_00537c90(int param_1)

{
  *(undefined4 *)(param_1 + 0x1f330) = 0;
  *(undefined4 *)(param_1 + 0x1f32c) = 0x7f000000;
  return;
}
```

### decoy: 0x00537be0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00537be0: 16-byte aligned: YES
  00537be0  55             PUSH EBP
  00537be1  8B EC          MOV EBP,ESP
  00537be3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00537be0
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 5):
  +0x1F32C [dec 127788]  x3  via EAX,ECX
  +0x1F330 [dec 127792]  x3  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x28 [dec 40]  x1  via EDX
  +0x1005C [dec 65628]  x1  via ECX
```

## Card B9-3-34

GOG function: CServerExoAppInternal::UpdateClientGameObjects  gog 0x007ba760  callconv=__thiscall  param_size_bytes=4  gog_size=122  gog_retn=4
Candidate Steam address: 0x00537510 (size 122, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005377c0:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b9ef0 CServerExoAppInternal::StopServices -> 0x00536c10  gate_class=REVIEW
- GOG-order neighbour after:  0x007ba7e0 CServerExoAppInternal::UpdateClientGameObjectsForPlayer -> 0x00537590  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00608350 CExoTimers::GetHighResolutionTimer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007ba7e0 CServerExoAppInternal::UpdateClientGameObjectsForPlayer -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x14*} | Steam {0x4*,0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00537510: 16-byte aligned: YES
  00537510  55             PUSH EBP
  00537511  8B EC          MOV EBP,ESP
  00537513  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00537510
string literals (0):
direct callees (5):
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00537590  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0x10064 [dec 65636]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00537510 FUN_00537510 size=122 ret=4 callers=2 callees=5

void __thiscall FUN_00537510(undefined4 param_1,undefined4 param_2)

{
  int local_14;
  undefined4 local_10;
  undefined8 local_c;
  
  local_c = FUN_00736220(param_1);
  local_14 = FUN_007a1720();
  while (local_14 != 0) {
    local_10 = FUN_00561430(local_14);
    FUN_00537590(local_10,param_2,local_c);
    FUN_0058c370(&local_14);
  }
  return;
}
```

### decoy: 0x005377c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005377c0: 16-byte aligned: YES
  005377c0  55             PUSH EBP
  005377c1  8B EC          MOV EBP,ESP
  005377c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005377c0
string literals (0):
direct callees (2):
  0x00647020  (unnamed)
  0x00647110  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x24 [dec 36]  x2  via EAX
  +0x10010 [dec 65552]  x2  via ECX,EDX
```

## Card B9-3-35

GOG function: CSWSModule::IsObjectInLimbo  gog 0x00731fc0  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=4
Candidate Steam address: 0x0055f930 (size 90, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0055f8c0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00731f50 CSWSModule::RemoveObjectFromLimbo -> 0x0055f8c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00732020 CSWSModule::TimeStopSanityCheck -> 0x0055f990  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1fc*,0x200*} | Steam {0x1fc*,0x200*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0055f930: 16-byte aligned: YES
  0055f930  55             PUSH EBP
  0055f931  8B EC          MOV EBP,ESP
  0055f933  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0055f930
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1FC [dec 508]  x1  via ECX
  +0x200 [dec 512]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0055f930 FUN_0055f930 size=90 ret=4 callers=1 callees=0

undefined4 __thiscall FUN_0055f930(int param_1,int param_2)

{
  int local_8;
  
  local_8 = *(int *)(param_1 + 0x200);
  do {
    local_8 = local_8 + -1;
    if (local_8 < 0) {
      return 0;
    }
  } while (param_2 != *(int *)(*(int *)(param_1 + 0x1fc) + local_8 * 4));
  return 1;
}
```

### decoy: 0x0055f8c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055f8c0: 16-byte aligned: YES
  0055f8c0  55             PUSH EBP
  0055f8c1  8B EC          MOV EBP,ESP
  0055f8c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0055f8c0
string literals (0):
direct callees (1):
  0x0049B840  (unnamed)
struct offsets touched (top 2):
  +0x1FC [dec 508]  x1  via ECX
  +0x200 [dec 512]  x1  via EAX
```

## Card B9-3-36

GOG function: CSWSModule::SaveModuleStart  gog 0x0072dfe0  callconv=(blank)  param_size_bytes=0  gog_size=833  gog_retn=8
Candidate Steam address: 0x0055b830 (size 833, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSModule::BeginModuleSave | G7 mac: pass(0.90) | G8 mac_rotation: pass
- anchors: A-STR 1 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'MOD V1.0'@00993fa4; CALLEE 2 (callees: Constructor 0x716e60, CreateGFFFile 0x71a3d0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0055bb80:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0072d430 CSWSModule::PlotPathInArea -> 0x0055ac80  gate_class=REVIEW
- GOG-order neighbour after:  0x0072e330 CSWSModule::SaveModuleFinish -> 0x0055bb80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (19), mapped to Steam through trusted rows only:
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x0060b840 CExoAliasList::ResolveFileName -> Steam ?  in candidate callees: n/a
  0x00622ff0 CResGFF::Constructor -> Steam 0x00716e60  in candidate callees: yes
  0x00626530 CResGFF::CreateGFFFile -> Steam 0x0071a3d0  in candidate callees: yes
  0x0062aaf0 CERFFile::Constructor -> Steam ?  in candidate callees: n/a
  ... 11 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'MOD V1.0'
disp sets (* = in both, 7 common): GOG {0xc*,0x1e8*,0x208*,0x20c*,0x210*,0x214*,0x22c*} | Steam {0xc*,0x1e8*,0x208*,0x20c*,0x210*,0x214*,0x22c*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x6c*,0xa0*,0xd0*,0xbc1*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x6c*,0xa0*,0xd0*,0xbc1*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0055b830: 16-byte aligned: YES
  0055b830  55             PUSH EBP
  0055b831  8B EC          MOV EBP,ESP
  0055b833  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0055b830
string literals (1):
  0x00993FA4  at 0055b92e  'MOD V1.0'
direct callees (19):
  0x007397B0  (unnamed)
  0x00734010  CExoString::CStr
  0x00919723  operator new
  0x00720B60  (unnamed)
  0x0084A640  (unnamed)
  0x007211E0  CERFFile::Create
  0x00720E10  (unnamed)
  0x007212D0  (unnamed)
  0x00721350  (unnamed)
  0x007214E0  (unnamed)
  0x00716E60  CResGFF::Constructor
  0x0091C630  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x0055BD10  (unnamed)
  0x0055CC70  SaveRepute (GAMEINPROGRESS:REPUTE, type 0x7F6)
  0x00720C20  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 7):
  +0x208 [dec 520]  x8  via EAX,ECX,EDX
  +0x20C [dec 524]  x5  via EAX,ECX,EDX
  +0x210 [dec 528]  x5  via EAX,EDX
  +0x22C [dec 556]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EAX
  +0x1E8 [dec 488]  x1  via EAX
  +0x214 [dec 532]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0055b830 FUN_0055b830 size=833 ret=8 callers=1 callees=20

void __thiscall FUN_0055b830(int param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  LPCSTR lpFileName;
  undefined4 uVar2;
  int iVar3;
  void *local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  int local_3c;
  void *local_38;
  size_t local_34;
  int local_30;
  undefined4 local_2c;
  int local_28;
  undefined4 local_24;
  int local_20;
  undefined4 local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009500a9;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(undefined4 *)(param_1 + 0x1e8) = 1;
  FUN_007397b0(local_18,param_3,0xbc1);
  local_8 = 0;
  lpFileName = (LPCSTR)FUN_00734010(uVar1);
  DeleteFileA(lpFileName);
  local_20 = FUN_00919723(0xd0);
  local_8._0_1_ = 1;
  if (local_20 == 0) {
    local_68 = 0;
  }
  else {
    local_68 = FUN_00720b60();
  }
  local_1c = local_68;
  local_8._0_1_ = 0;
  *(undefined4 *)(param_1 + 0x208) = local_68;
  local_28 = FUN_00919723(0xc);
  local_8._0_1_ = 2;
  if (local_28 == 0) {
    local_6c = 0;
  }
  else {
    local_6c = FUN_0084a640(0);
  }
  local_24 = local_6c;
  local_8._0_1_ = 0;
  *(undefined4 *)(param_1 + 0x214) = local_6c;
  FUN_007211e0(param_3);
  FUN_00720e10("MOD V1.0");
  FUN_007212d0();
  FUN_00721350();
  *(undefined4 *)(param_1 + 0x22c) = 3;
  FUN_007214e0(*(undefined4 *)(param_1 + 0x22c));
  local_30 = FUN_00919723(0xa0);
  local_8._0_1_ = 3;
  if (local_30 == 0) {
    local_70 = 0;
  }
  else {
    local_70 = FUN_00716e60();
  }
  local_2c = local_70;
  local_8 = (uint)local_8._1_3_ << 8;
  *(undefined4 *)(param_1 + 0x210) = local_70;
  local_34 = 4;
  local_38 = (void *)FUN_00919723(4);
  if (local_38 == (void *)0x0) {
/* ... 45 more lines truncated by --max-lines */
```

### decoy: 0x0055bb80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055bb80: 16-byte aligned: YES
  0055bb80  55             PUSH EBP
  0055bb81  8B EC          MOV EBP,ESP
  0055bb83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0055bb80
string literals (0):
direct callees (8):
  0x00733570  CExoString::CExoString_char
  0x0055DB00  CSWSModule::SaveStatic
  0x00733780  CExoString::~CExoString
  0x0055CB50  CSWSModule::SaveModuleIFOFinish
  0x00721E70  CERFFile::Finish
  0x00720C20  (unnamed)
  0x009196FD  operator delete
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 5):
  +0x208 [dec 520]  x6  via EAX,ECX
  +0x214 [dec 532]  x3  via EAX,EDX
  +0x20C [dec 524]  x1  via EDX
  +0x210 [dec 528]  x1  via ECX
  +0x220 [dec 544]  x1  via ECX
```

## Card B9-3-37

GOG function: CServerExoApp::GetActiveExclusionList  gog 0x0064c210  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c870 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c850:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c1e0 CServerExoApp::RemoveFromExclusionList -> 0x0051c790  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c230 CServerExoApp::ResolvePlayerByFirstName -> 0x0051c810  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bc850 CServerExoAppInternal::GetActiveExclusionList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c870: 16-byte aligned: YES
  0051c870  55             PUSH EBP
  0051c871  8B EC          MOV EBP,ESP
  0051c873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c870
string literals (0):
direct callees (1):
  0x00539BB0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c870 FUN_0051c870 size=22 ret=0 callers=5 callees=1
THUNK -> 0x00539BB0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c870(undefined4 param_1)

{
  FUN_00539bb0(param_1);
  return;
}
```

### decoy: 0x0051c850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c850: 16-byte aligned: YES
  0051c850  55             PUSH EBP
  0051c851  8B EC          MOV EBP,ESP
  0051c853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c850
string literals (0):
direct callees (1):
  0x00539800  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-3-38

GOG function: CSWSModule::non_virtual_thunk_to_dtor_CSWSModule  gog 0x00733ef0  callconv=(blank)  param_size_bytes=0  gog_size=8  gog_retn=None
Candidate Steam address: 0x005615c0 (size 64, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00561580:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00733d10 CExoArrayList__FILETIME_::Allocate  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00733f00 CSWSRoom::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007294a0 CSWSModule::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0x1c} | Steam {0x3,0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005615c0: 16-byte aligned: YES
  005615c0  55             PUSH EBP
  005615c1  8B EC          MOV EBP,ESP
  005615c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005615c0
string literals (0):
direct callees (1):
  0x00561870  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005615c0 FUN_005615c0 size=64 ret=0 callers=5 callees=1

void __fastcall FUN_005615c0(int *param_1)

{
  if (*param_1 != 0) {
    FUN_00561870(3);
  }
  *param_1 = 0;
  return;
}
```

### decoy: 0x00561580 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00561580: 16-byte aligned: YES
  00561580  55             PUSH EBP
  00561581  8B EC          MOV EBP,ESP
  00561583  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00561580
string literals (0):
direct callees (1):
  0x00561AD0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B9-3-39

GOG function: CSWSModule::PlotPathInArea  gog 0x0072d430  callconv=(blank)  param_size_bytes=8  gog_size=2985  gog_retn=8
Candidate Steam address: 0x0055ac80 (size 2985, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSArea::PlotPathInArea | G7 mac: pass(0.88) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: TestDirectLine 0x550980, PlotGridPath 0x550e30, PlotPathPointPath 0x5532f0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0055ab30:STR0/CALLEE0/SIG0.151/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0072d2e0 CSWSModule::PlotPath -> 0x0055ab30  gate_class=AUTO
- GOG-order neighbour after:  0x0072dfe0 CSWSModule::SaveModuleStart -> 0x0055b830  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x004143c0 unnamed -> Steam ?  in candidate callees: n/a
  0x006481e0 CPathfindInformation::FlipStartEndPoints -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0070aba0 CSWSArea::TestDirectLine -> Steam 0x00550980  in candidate callees: yes
  0x0070b050 CSWSArea::PlotGridPath -> Steam 0x00550e30  in candidate callees: yes
  ... 6 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 24 common): GOG {0x4*,0x8*,0xc*,0x14*,0x2c*,0x3c*,0x5c*,0x60*,0x68*,0x74*,0x78*,0x80*,0x84*,0x88*,...(+10)} | Steam {0x4*,0x8*,0xc*,0x14*,0x2c*,0x3c*,0x5c*,0x60*,0x68*,0x74*,0x78*,0x80*,0x84*,0x88*,...(+10)}
imm sets (* = in both, 13 common): GOG {0x2*,0x4*,0x8*,0xc*,0x10*,0x41*,0x64*,0x2b4*,0x186a0*,0xfffffff*,0xfffffffc*,0xfffffffd*,0xfffffffe*} | Steam {0x2*,0x4*,0x8*,0xc*,0x10*,0x41*,0x64*,0x2b4*,0x186a0*,0xfffffff*,0xfffffffc*,0xfffffffd*,0xfffffffe*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0055ac80: 16-byte aligned: YES
  0055ac80  55             PUSH EBP
  0055ac81  8B EC          MOV EBP,ESP
  0055ac83  81 EC B4 02 00 00 SUB ESP,0x2b4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0055ac80
string literals (0):
direct callees (14):
  0x00557EF0  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x005143C0  (unnamed)
  0x0045CFE0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x0091CB24  (unnamed)
  0x00550980  CSWSArea::TestDirectLine
  0x00919723  operator new
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00550E30  CSWSArea::PlotGridPath
  0x005532F0  CSWSArea::PlotPathPointPath
  0x009196FD  operator delete
  0x006FA3F0  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
  +0x4 [dec 4]  x9  via EAX,ECX
  +0x2C [dec 44]  x5  via EAX,ECX,EDX
  +0x3C [dec 60]  x4  via EAX,ECX,EDX
  +0x84 [dec 132]  x4  via EAX,EDX
  +0x90 [dec 144]  x4  via EAX,ECX,EDX
  +0x78 [dec 120]  x3  via ECX,EDX
  +0x8C [dec 140]  x3  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x5C [dec 92]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0055ac80 FUN_0055ac80 size=2985 ret=8 callers=1 callees=14

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int FUN_0055ac80(int *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  float *pfVar2;
  int iVar3;
  undefined4 *puVar4;
  float10 fVar5;
  float local_1c4;
  float local_1c0;
  undefined4 local_1bc;
  undefined1 local_1b8 [12];
  undefined1 local_1ac [12];
  undefined1 local_1a0 [12];
  float *local_194;
  float local_190;
  float local_18c;
  undefined4 local_188;
  float local_184;
  float local_180;
  undefined4 local_17c;
  float local_178;
  float local_174;
  undefined4 local_170;
  float local_16c;
  float local_168;
  undefined4 local_164;
  undefined1 local_160 [12];
  undefined1 local_154 [12];
  undefined1 local_148 [12];
  undefined1 local_13c [12];
  undefined1 local_130 [12];
  undefined1 local_124 [12];
  undefined1 local_118 [12];
  undefined1 local_10c [12];
  int local_100;
  int local_fc;
  int local_f8;
  int local_f4;
  int local_f0;
  float *local_ec;
  float local_e8;
  int local_e4;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  int local_ac;
  undefined4 local_a8;
  int local_a4;
  undefined4 local_a0;
  int local_9c;
  undefined4 local_98;
  int local_94;
  undefined4 local_90;
  int local_8c;
  int local_88;
  float *local_84;
  float local_80;
  float local_7c;
  float local_78;
  int local_74;
  float local_70;
  float local_6c;
  float local_68;
  undefined4 local_64;
  float local_60;
  float local_5c;
/* ... 261 more lines truncated by --max-lines */
```

### decoy: 0x0055ab30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055ab30: 16-byte aligned: YES
  0055ab30  55             PUSH EBP
  0055ab31  8B EC          MOV EBP,ESP
  0055ab33  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0055ab30
string literals (0):
direct callees (7):
  0x006FA080  (unnamed)
  0x006F9E90  (unnamed)
  0x006F9D40  CPathfindInformation::ResetAtEndOfPath
  0x0051C280  (unnamed)
  0x0055A900  pathfind inter-area driver
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005C3270  CSWSCreature::ComputeTotalWalkDistance
struct offsets touched (top 8):
  +0x58 [dec 88]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x84 [dec 132]  x2  via EAX,EDX
  +0x88 [dec 136]  x2  via EDX
  +0xA4 [dec 164]  x2  via EAX,EDX
  +0x2C [dec 44]  x1  via EAX
  +0x3C [dec 60]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
```

## Card B9-3-40

GOG function: CSWSModule::GetWaypoint  gog 0x0072a670  callconv=(blank)  param_size_bytes=4  gog_size=35  gog_retn=4
Candidate Steam address: 0x00557ec0 (size 35, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: GetTargetArea 0x61e540, ExecuteCommandGetWaypointByTag 0x68a810, ExecuteCommandGetWaypointByTrigger 0x68a950)
- best decoy (neighbour/other candidate) scores: 0x00557de0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0072a590 CSWSModule::GenerateInterAreaDFSSuccessors -> 0x00557de0  gate_class=AUTO
- GOG-order neighbour after:  0x0072a6a0 CSWSModule::GetArea -> 0x00557ef0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00731420 CSWSModule::FindObjectByTagTypeOrdinal -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x0075d7d0 CSWSTrigger::GetTargetArea -> Steam 0x0061e540  in candidate callers: yes
  0x0079fe60 Global::ExecuteCommandGetWaypointByTag -> Steam 0x0068a810  in candidate callers: yes
  0x0079ffa0 Global::ExecuteCommandGetWaypointByTrigger -> Steam 0x0068a950  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00557ec0: 16-byte aligned: YES
  00557ec0  55             PUSH EBP
  00557ec1  8B EC          MOV EBP,ESP
  00557ec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00557ec0
string literals (0):
direct callees (1):
  0x0055EC70  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00557ec0 FUN_00557ec0 size=35 ret=4 callers=4 callees=1
THUNK -> 0x0055EC70 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00557ec0(undefined4 param_1)

{
  FUN_0055ec70(param_1,DAT_0099447b,0);
  return;
}
```

### decoy: 0x00557de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00557de0: 16-byte aligned: YES
  00557de0  55             PUSH EBP
  00557de1  8B EC          MOV EBP,ESP
  00557de3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00557de0
string literals (0):
direct callees (3):
  0x00557EF0  (unnamed)
  0x0054CD50  CSWSArea::CountAreaTransitionTriggers
  0x00919723  operator new
struct offsets touched (top 3):
  +0xB4 [dec 180]  x2  via ECX
  +0xB8 [dec 184]  x2  via EDX
  +0xAC [dec 172]  x1  via EAX
```

## Card B9-3-41

GOG function: CSWSModule::SavePlayers  gog 0x0072f830  callconv=(blank)  param_size_bytes=8  gog_size=1904  gog_retn=16
Candidate Steam address: 0x0055d080 (size 1904, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: rotation:0x0072ffa0:0.84>0.59
- anchors: A-STR 2 | A-CALLEE 6 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'Mod_LastName'@00994a1c; 'Mod_FirstName'@00994a2c; CALLEE 6 (callees: Demand 0x7111e0, Release 0x7112b0, GetTopLevelStruct 0x7189d0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0055d7f0:STR0/CALLEE2/SIG0.405/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0072f6b0 CSWSModule::SaveModuleInProgress -> 0x0055cf00  gate_class=AUTO
- GOG-order neighbour after:  0x0072ffa0 CSWSModule::SavePrimaryPlayerInfo -> 0x0055d7f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (41), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam ?  in candidate callees: n/a
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00605810 CExoString::Constructor_5 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  ... 33 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (7): 'Module'; 'ObjectId'; 'Mod_IsPrimaryPlr'; 'Mod_LastName' ...
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x30*,0x3c*,0x1198*} | Steam {0x4*,0x8*,0x30*,0x3c*,0x1198*}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x4*,0x5*,0x10*,0x34*,0x3c*,0x48*,0x84*,0x134*,0x7de*,0xbead*,0x7f000000*} | Steam {0x2*,0x3*,0x4*,0x5*,0x10*,0x34*,0x3c*,0x48*,0x84*,0x134*,0x7de*,0xbead*,0x7f000000*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x0055d080: 16-byte aligned: YES
  0055d080  55             PUSH EBP
  0055d081  8B EC          MOV EBP,ESP
  0055d083  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0055d080
string literals (14):
  0x00993E6C  at 0055d0f1  'Mod_PlayerList'
  0x00994A3C  at 0055d283  'Mod_CommntyName'
  0x00994A08  at 0055d2b6  'Mod_IsPrimaryPlr'
  0x00994A2C  at 0055d2de  'Mod_FirstName'
  0x00994A1C  at 0055d308  'Mod_LastName'
  0x00992EB4  at 0055d32f  'ObjectId'
  0x00992DA0  at 0055d382  'Module'
  0x00992EB4  at 0055d544  'ObjectId'
  0x00994A3C  at 0055d574  'Mod_CommntyName'
  0x00994A3C  at 0055d67e  'Mod_CommntyName'
  0x00994A08  at 0055d69f  'Mod_IsPrimaryPlr'
  0x00994A2C  at 0055d6c7  'Mod_FirstName'
direct callees (41):
  0x0051C080  CServerExoApp::GetObjectArray
  0x0084A640  (unnamed)
  0x00719740  CResGFF::AddList
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x00631560  (unnamed)
  0x0055E1A0  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00719800  CResGFF::AddListElement
  0x00632A90  (unnamed)
  0x00719E50  CResGFF::WriteFieldCExoString
  0x00733780  CExoString::~CExoString
  0x007199D0  CResGFF::WriteFieldBYTE
  0x00719F30  CResGFF::WriteFieldCExoLocString
  0x00719B90  CResGFF::WriteFieldDWORD
  0x00574500  CSWSCreature::SaveCreature
  0x0058A090  CSWSCreature::DumpToLog
struct offsets touched (top 5):
  +0x4 [dec 4]  x6  via EAX,ECX
  +0x1198 [dec 4504]  x4  via EAX,ECX,EDX
  +0x3C [dec 60]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0055d080 FUN_0055d080 size=1904 ret=16 callers=1 callees=41

/* WARNING: Removing unreachable block (ram,0x0055d452) */
/* WARNING: Removing unreachable block (ram,0x0055d464) */
/* WARNING: Removing unreachable block (ram,0x0055d793) */
/* WARNING: Removing unreachable block (ram,0x0055d47b) */
/* WARNING: Removing unreachable block (ram,0x0055d4a0) */
/* WARNING: Removing unreachable block (ram,0x0055d4b7) */
/* WARNING: Removing unreachable block (ram,0x0055d4c8) */
/* WARNING: Removing unreachable block (ram,0x0055d491) */
/* WARNING: Removing unreachable block (ram,0x0055d4d6) */
/* WARNING: Removing unreachable block (ram,0x0055d4e4) */
/* WARNING: Removing unreachable block (ram,0x0055d4fc) */
/* WARNING: Removing unreachable block (ram,0x0055d53b) */
/* WARNING: Removing unreachable block (ram,0x0055d5e4) */
/* WARNING: Removing unreachable block (ram,0x0055d5f1) */
/* WARNING: Removing unreachable block (ram,0x0055d628) */
/* WARNING: Removing unreachable block (ram,0x0055d611) */
/* WARNING: Removing unreachable block (ram,0x0055d632) */
/* WARNING: Removing unreachable block (ram,0x0055d784) */
/* WARNING: Removing unreachable block (ram,0x0055d768) */
/* WARNING: Removing unreachable block (ram,0x0055d78e) */
/* WARNING: Removing unreachable block (ram,0x0055d5df) */
/* WARNING: Removing unreachable block (ram,0x0055d4df) */
/* WARNING: Removing unreachable block (ram,0x0055d45b) */

void FUN_0055d080(undefined4 param_1,undefined4 param_2,undefined4 param_3,int *param_4)

{
  char cVar1;
  undefined4 uVar2;
  undefined1 local_b8 [16];
  undefined1 local_a8 [16];
  int local_98;
  undefined4 local_94;
  int local_90;
  undefined1 local_8c [20];
  int local_78;
  undefined4 local_74;
  int *local_58;
  undefined1 local_54 [8];
  int local_4c;
  int local_40;
  undefined4 local_3c;
  int local_38;
  undefined1 local_30 [4];
  undefined4 local_2c;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095025f;
  local_10 = ExceptionList;
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_3c = FUN_0051c080(local_14);
  FUN_0084a640(0);
  local_8 = 0;
  local_74 = 0;
  local_2c = *(undefined4 *)(DAT_00a1b4a4 + 8);
  FUN_00719740(local_8c,param_2,"Mod_PlayerList");
  for (local_90 = 0; local_90 < param_4[1 = byte +0x4]; local_90 = local_90 + 1) {
    cVar1 = FUN_0053dfb0(*(undefined4 *)(*param_4 + local_90 * 4),&local_58);
    if (cVar1 == DAT_0099447f) {
      local_4c = (**(code **)(*local_58 + 0x30))();
      local_40 = FUN_0051c600(*(undefined4 *)(local_4c + 4));
      if (local_40 == 0) {
        uVar2 = 0;
        FUN_0073fb90(0);
        local_98 = FUN_007e5da0(uVar2);
        local_94 = FUN_0051c8b0(*(undefined4 *)(local_98 + 4));
        local_40 = FUN_0051c600(local_94);
      }
      if (local_40 == 0) {
        FUN_0058a090();
      }
      else {
        FUN_00631560();
        local_78 = FUN_0055e1a0(local_40);
/* ... 44 more lines truncated by --max-lines */
```

### decoy: 0x0055d7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055d7f0: 16-byte aligned: YES
  0055d7f0  55             PUSH EBP
  0055d7f1  8B EC          MOV EBP,ESP
  0055d7f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0055d7f0
string literals (4):
  0x00994A3C  at 0055d96d  'Mod_CommntyName'
  0x00994A08  at 0055d99a  'Mod_IsPrimaryPlr'
  0x00992EB4  at 0055d9bc  'ObjectId'
  0x00994E00  at 0055d9e4  'Player'
direct callees (16):
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x0091C630  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::Constructor
  0x00632A90  (unnamed)
  0x00719E50  CResGFF::WriteFieldCExoString
  0x007199D0  CResGFF::WriteFieldBYTE
  0x00719B90  CResGFF::WriteFieldDWORD
  0x00574500  CSWSCreature::SaveCreature
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x0071A5A0  CResGFF::WriteGFFFile
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x3C [dec 60]  x1  via ECX
```

## Card B9-3-42

GOG function: CServerExoAppInternal::IsInjured  gog 0x007bbef0  callconv=(blank)  param_size_bytes=4  gog_size=48  gog_retn=4
Candidate Steam address: 0x00538ca0 (size 48, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00538cd0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bbea0 CServerExoAppInternal::UpDatePlayerPosition -> 0x00538c50  gate_class=REVIEW
- GOG-order neighbour after:  0x007bbf20 CServerExoAppInternal::GetPauseState -> 0x00538c20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006a4730 unnamed -> Steam ?  in candidate callees: n/a
  0x007badc0 CServerExoAppInternal::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00538ca0: 16-byte aligned: YES
  00538ca0  55             PUSH EBP
  00538ca1  8B EC          MOV EBP,ESP
  00538ca3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00538ca0
string literals (0):
direct callees (2):
  0x00537B70  (unnamed)
  0x0058BC50  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00538ca0 FUN_00538ca0 size=48 ret=4 callers=1 callees=2

undefined4 FUN_00538ca0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  
  iVar1 = FUN_00537b70(param_1);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = FUN_0058bc50(extraout_var);
  }
  return uVar2;
}
```

### decoy: 0x00538cd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00538cd0: 16-byte aligned: YES
  00538cd0  55             PUSH EBP
  00538cd1  8B EC          MOV EBP,ESP
  00538cd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00538cd0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10078 [dec 65656]  x1  via EAX
```

## Card B9-3-43

GOG function: CServerExoAppInternal::InitiateModuleForPlayer  gog 0x007b8a10  callconv=(blank)  param_size_bytes=4  gog_size=819  gog_retn=4
Candidate Steam address: 0x005356d0 (size 819, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.90) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00535a10:STR0/CALLEE0/SIG0.434/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b8970 CServerExoAppInternal::DealWithLoadGameError -> 0x00535630  gate_class=REVIEW
- GOG-order neighbour after:  0x007b8d50 CServerExoAppInternal::MovePlayerToArea -> 0x00535a10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (16), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0064bb40 CServerExoApp::GetAreaByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam ?  in candidate callees: n/a
  0x0067ab60 CSWSCreature::Constructor -> Steam ?  in candidate callees: n/a
  0x006aabf0 CSWSObject::SetOrientation -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x006ac100 CSWSObject::SetArea -> Steam ?  in candidate callees: n/a
  ... 8 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 19 common): GOG {0x4*,0x8*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x2c*,0x30*,0x40*,0x58*,0x124*,0x24c*,0x350*,...(+5)} | Steam {0x4*,0x8*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x2c*,0x30*,0x40*,0x58*,0x124*,0x24c*,0x350*,...(+5)}
imm sets (* = in both, 7 common): GOG {0x4*,0x78*,0x94*,0xa0*,0x354*,0x1220*,0x7f000000*} | Steam {0x4*,0x78*,0x94*,0xa0*,0x354*,0x1220*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005356d0: 16-byte aligned: YES
  005356d0  55             PUSH EBP
  005356d1  8B EC          MOV EBP,ESP
  005356d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005356d0
string literals (0):
direct callees (16):
  0x00632A20  (unnamed)
  0x00919723  operator new
  0x00561F30  CSWSCreature::Constructor
  0x00632830  (unnamed)
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EF0  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
  0x005453C0  CSWSObject::GetArea
  0x0051C120  (unnamed)
  0x00545420  CSWSObject::SetArea
  0x0051C2E0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x006305B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00637620  (unnamed)
  0x00639F40  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x58 [dec 88]  x5  via EAX
  +0x350 [dec 848]  x3  via EAX,ECX,EDX
  +0x124 [dec 292]  x2  via ECX,EDX
  +0x360 [dec 864]  x2  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005356d0 FUN_005356d0 size=819 ret=4 callers=1 callees=16

void FUN_005356d0(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  int *piVar5;
  int local_84;
  undefined1 local_48 [8];
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  int local_34;
  int local_30;
  int local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094ec73;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar4 = FUN_00632a20(uVar3);
  if (iVar4 == 0) {
    local_34 = FUN_00919723(0x1220);
    local_8 = 0;
    if (local_34 == 0) {
      local_84 = 0;
    }
    else {
      local_84 = FUN_00561f30(0x7f000000,1);
    }
    local_30 = local_84;
    local_8 = 0xffffffff;
    local_1c = local_84;
    *(undefined4 *)(*(int *)(local_84 + 0x380) + 0x2c) = *(undefined4 *)(local_84 + 4);
    FUN_00632830(local_84);
  }
  else {
    piVar5 = (int *)FUN_00632a20(uVar3);
    local_1c = (**(code **)(*piVar5 + 0x30))();
  }
  local_14 = (uint)(*(int *)(param_1 + 0x40) == 0);
  if (local_14 == 0) {
    local_18 = FUN_005453c0();
    if (local_18 == 0) {
      local_18 = FUN_0051c120(*(undefined4 *)(local_1c + 0x350));
    }
    else {
      *(undefined4 *)(local_1c + 0x350) = *(undefined4 *)(local_18 + 0x124);
      *(undefined4 *)(local_1c + 0x354) = *(undefined4 *)(local_1c + 0x94);
      *(undefined4 *)(local_1c + 0x358) = *(undefined4 *)(local_1c + 0x98);
      *(undefined4 *)(local_1c + 0x35c) = *(undefined4 *)(local_1c + 0x9c);
      *(undefined4 *)(local_1c + 0x360) = 0;
    }
  }
  else {
    local_28 = 0;
    local_24 = 0;
    local_20 = 0;
    FUN_0052f4c0();
    local_18 = FUN_00557ef0();
    *(undefined4 *)(local_1c + 0x350) = *(undefined4 *)(local_18 + 0x124);
    iVar4 = FUN_0052f4c0();
    uVar1 = *(undefined4 *)(*(int *)(iVar4 + 0x58) + 0x10);
    iVar4 = FUN_0052f4c0();
    uVar2 = *(undefined4 *)(*(int *)(iVar4 + 0x58) + 0x14);
    iVar4 = FUN_0052f4c0();
    local_38 = *(undefined4 *)(*(int *)(iVar4 + 0x58) + 0x18);
    *(undefined4 *)(local_1c + 0x354) = uVar1;
/* ... 32 more lines truncated by --max-lines */
```

### decoy: 0x00535a10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00535a10: 16-byte aligned: YES
  00535a10  55             PUSH EBP
  00535a11  8B EC          MOV EBP,ESP
  00535a13  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00535a10
string literals (0):
direct callees (13):
  0x00632A20  (unnamed)
  0x00733DB0  CExoString::operator[]
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EC0  (unnamed)
  0x0051C1E0  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
  0x007338D0  CExoString::operator=_2
  0x00557EF0  (unnamed)
  0x0054BDC0  CSWSArea::ComputeSafeLocation
  0x005C2770  CSWSCreature::UpdateSubareasOnJumpPosition
  0x00568BD0  CSWSCreature::AddToArea
  0x005D0F30  CSWSObject::UpdatePerception
  0x005F9A20  CSWPartyTable::PositionParty
struct offsets touched (top 8):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x380 [dec 896]  x2  via ECX,EDX
  +0x100C4 [dec 65732]  x2  via EAX,EDX
  +0x2C [dec 44]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x350 [dec 848]  x1  via EDX
```

## Card B9-3-44

GOG function: CServerExoApp::SetMoveToWorldTime  gog 0x0064ca10  callconv=(blank)  param_size_bytes=26  gog_size=58  gog_retn=28
Candidate Steam address: 0x0051cff0 (size 58, retn 28)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cfa0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c9f0 CServerExoApp::GetMoveToPausedTimeOfDay -> 0x0051cfd0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064ca50 CServerExoApp::GetMoveToWorldTime -> 0x0051d030  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bfa20 CServerExoAppInternal::SetMoveToWorldTime -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}

### abi
```
GOG param bytes: 26; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(26/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=28 ; asm RET = 28
Stack Slot Match: TRUE (GOG 26 bytes -> x86 slots 28; ASM RET 28)
```

### prologue
```
prologue for 0x0051cff0: 16-byte aligned: YES
  0051cff0  55             PUSH EBP
  0051cff1  8B EC          MOV EBP,ESP
  0051cff3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cff0
string literals (0):
direct callees (1):
  0x0053CC80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cff0 FUN_0051cff0 size=58 ret=28 callers=2 callees=1
THUNK -> 0x0053CC80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051cff0(undefined4 param_1,undefined1 param_2,undefined1 param_3,undefined1 param_4,
                 undefined2 param_5,undefined2 param_6,undefined2 param_7)

{
  FUN_0053cc80(param_1,param_2,param_3,param_4,param_5,param_6,param_7);
  return;
}
```

### decoy: 0x0051cfa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cfa0: 16-byte aligned: YES
  0051cfa0  55             PUSH EBP
  0051cfa1  8B EC          MOV EBP,ESP
  0051cfa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cfa0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100AC [dec 65708]  x1  via EDX
```

## Card B9-3-45

GOG function: CServerExoAppInternal::MainLoop  gog 0x007b6bb0  callconv=__thiscall  param_size_bytes=0  gog_size=3450  gog_retn=0
Candidate Steam address: 0x00533830 (size 3450, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Game | G7 mac: pass(0.76) | G8 mac_rotation: pass
- anchors: A-STR 2 | A-CALLEE 11 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR '000_Beast_Conf_Active'@00993f74; '000_Human_Conf_Active'@00993f8c; CALLEE 11 (callees: SetCanSendDialog 0x740420, StartSounds 0x7a7c00, GetGlobalVariableTable 0x51c890); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005334e0:STR0/CALLEE0/SIG0.18/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b6860 CServerExoAppInternal::SetEstimatedSaveSize -> 0x005334e0  gate_class=AUTO
- GOG-order neighbour after:  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> 0x00534600  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (70), mapped to Steam through trusted rows only:
  0x00401c30 CAppManager::DestroyServer -> Steam ?  in candidate callees: n/a
  0x00407440 Global::ClearLogCounters -> Steam ?  in candidate callees: n/a
  0x0040c510 CClientExoApp::EndLoadBar -> Steam ?  in candidate callees: n/a
  0x0040c8f0 CClientExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x0040d070 CClientExoApp::GetSWParty -> Steam ?  in candidate callees: n/a
  0x0040d500 CClientExoApp::GetAreaNotReadyToDisplay -> Steam ?  in candidate callees: n/a
  0x0040d8e0 CClientExoApp::SetCanSendDialog -> Steam 0x00740420  in candidate callees: yes
  ... 62 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (5): '000_Droid_Conf_Active'; '000_Beast_Conf_Active'; '000_Human_Conf_Active'; 'AUTOSAVE' ...
disp sets (* = in both, 14 common): GOG {0x4*,0x8*,0xc*,0x14*,0x20*,0x30*,0x38*,0x48*,0x9c*,0xcc*,0xf4*,0x220*,0xf78*,0x1024*} | Steam {0x4*,0x8*,0xc*,0x14*,0x20*,0x30*,0x38*,0x48*,0x9c*,0xcc*,0xf4*,0x220*,0xf78*,0x1024*}
imm sets (* = in both, 18 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x18*,0x20*,0x28*,0x19c*,0x224*,0x244*,0x809*,...(+4)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x18*,0x20*,0x28*,0x19c*,0x224*,0x244*,0x809*,...(+4)}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00533830: 16-byte aligned: YES
  00533830  55             PUSH EBP
  00533831  8B EC          MOV EBP,ESP
  00533833  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00533830
string literals (4):
  0x00993F8C  at 00533a09  '000_Human_Conf_Active'
  0x00993F74  at 00533a5c  '000_Beast_Conf_Active'
  0x00993F5C  at 00533aaf  '000_Droid_Conf_Active'
  0x00993F50  at 00533dd4  'AUTOSAVE'
direct callees (70):
  0x00539C20  (unnamed)
  0x00538DC0  (unnamed)
  0x0051A860  (unnamed)
  0x00703F30  CNetLayer::ProcessReceivedFrames (mapper `vote`, CONFIRMED by body)
  0x007047B0  (unnamed)
  0x0053A0C0  (unnamed)
  0x0053A8B0  (unnamed)
  0x0051D790  CServerAIMaster::UpdateState
  0x00537510  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
  0x00733570  CExoString::CExoString_char
  0x0051C890  CServerExoApp::GetGlobalVariableTable
  0x00654CC0  CSWGlobalVariableTable::SetNumber
  0x00733780  CExoString::~CExoString
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0051AB40  CWorldTimer::ConvertFromTimeOfDay
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x0055F7E0  (unnamed)
  0x0051CFF0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x17  via EAX,ECX,EDX
  +0x8 [dec 8]  x13  via EAX,ECX,EDX
  +0x14 [dec 20]  x7  via EAX,ECX,EDX
  +0x10010 [dec 65552]  x4  via EAX,EDX
  +0x10040 [dec 65600]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,ECX
  +0x10008 [dec 65544]  x3  via ECX,EDX
  +0x10014 [dec 65556]  x2  via EAX,ECX
  +0x10080 [dec 65664]  x2  via ECX
  +0x100B8 [dec 65720]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00533830 FUN_00533830 size=3450 ret=0 callers=1 callees=70

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00533830(int param_1)

{
  char cVar1;
  short sVar2;
  int iVar3;
  undefined8 uVar4;
  undefined4 *puVar5;
  undefined1 *puVar6;
  undefined1 *puVar7;
  undefined1 *puVar8;
  undefined4 uVar9;
  undefined2 *puVar10;
  undefined1 *puVar11;
  undefined4 *puVar12;
  undefined2 *puVar13;
  undefined1 *puVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  undefined2 *puVar17;
  int local_1ac;
  undefined4 local_1a8;
  short local_174;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined1 local_ac [8];
  undefined1 local_a4 [8];
  undefined1 local_9c [8];
  int local_94;
  int local_90;
  undefined1 local_8c [8];
  undefined1 local_84 [8];
  short local_7c;
  undefined1 local_78 [16];
  uint local_68;
  int local_64;
  undefined4 *local_60;
  int local_5c;
  undefined1 local_58 [8];
  int local_50;
  int local_4c;
  int local_48;
  int *local_44;
  undefined1 local_40 [4];
  undefined2 local_3c [2];
  undefined4 local_38;
  undefined2 local_34 [2];
  undefined4 local_30;
  undefined4 local_2c;
  undefined1 local_25;
  undefined1 local_24 [4];
  undefined2 local_20 [2];
  undefined1 local_1c [4];
  undefined1 local_18 [4];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094eb4a;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00539c20(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (**(int **)(DAT_00a1b4a4 + 0x14) == 0) {
    cVar1 = FUN_00538dc0();
    if (cVar1 == '\0') {
      FUN_0051a860();
    }
    cVar1 = FUN_00538dc0();
    if (cVar1 != '\x02') {
      FUN_0051a860();
    }
    FUN_0051a860();
    FUN_00703f30(1);
    FUN_007047b0(DAT_0099b6d4);
/* ... 261 more lines truncated by --max-lines */
```

### decoy: 0x005334e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005334e0: 16-byte aligned: YES
  005334e0  55             PUSH EBP
  005334e1  8B EC          MOV EBP,ESP
  005334e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005334e0
string literals (2):
  0x00993E84  at 0053352f  'GAMEINPROGRESS:'
  0x00993E84  at 00533623  'GAMEINPROGRESS:'
direct callees (16):
  0x0040DEF0  (unnamed)
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x00736050  CExoBase::GetDirectoryList
  0x00733780  CExoString::~CExoString
  0x007337C0  CExoString::operator=
  0x00919F10  _strlen
  0x00734500  CExoString::Left
  0x00733DF0  CExoString::operator+
  0x00734870  CExoString::Right
  0x007360C0  (unnamed)
  0x007397F0  (unnamed)
  0x007398E0  (unnamed)
  0x00739970  (unnamed)
  0x00739880  (unnamed)
  0x0040DF30  (unnamed)
struct offsets touched (top 1):
  +0x100C8 [dec 65736]  x2  via EAX,EDX
```

## Card B9-3-46

GOG function: CServerExoApp::SetDebugMode  gog 0x0064bdf0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051c350 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CServerExoApp::GetActiveTimer | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c320:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bdd0 CServerExoApp::GetPauseTimer -> 0x0051c3b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064be20 CServerExoApp::GetDebugMode -> 0x0051c400  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c350: 16-byte aligned: YES
  0051c350  55             PUSH EBP
  0051c351  8B EC          MOV EBP,ESP
  0051c353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c350
string literals (0):
direct callees (1):
  0x00538E00  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c350 FUN_0051c350 size=28 ret=4 callers=12 callees=1
THUNK -> 0x00538E00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c350(undefined4 param_1)

{
  FUN_00538e00(param_1);
  return;
}
```

### decoy: 0x0051c320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c320: 16-byte aligned: YES
  0051c320  55             PUSH EBP
  0051c321  8B EC          MOV EBP,ESP
  0051c323  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c320
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10008 [dec 65544]  x1  via ECX
```

## Card B9-3-47

GOG function: CServerExoApp::ComputeHeight  gog 0x0064c460  callconv=(blank)  param_size_bytes=0  gog_size=81  gog_retn=12
Candidate Steam address: 0x0051caa0 (size 53, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051ca40:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0064c410 CServerExoApp::GetCreaturePathfindInformation -> 0x0051c9f0  gate_class=AUTO
- GOG-order neighbour after:  0x0064c4c0 CServerExoApp::UpDatePlayerPosition -> 0x0051caa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x007054d0 CSWSArea::ComputeHeight -> Steam 0x0054b130  in candidate callees: no
  0x0072a6a0 CSWSModule::GetArea -> Steam ?  in candidate callees: n/a
  0x007b2840 CServerExoAppInternal::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x0070f0d0 CSWSArea::PathPointDFSearch -> Steam 0x00554eb0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 12)
```

### prologue
```
prologue for 0x0051caa0: 16-byte aligned: YES
  0051caa0  55             PUSH EBP
  0051caa1  8B EC          MOV EBP,ESP
  0051caa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051caa0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x00538C50  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051caa0 FUN_0051caa0 size=53 ret=12 callers=1 callees=2

void FUN_0051caa0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0(param_1);
  FUN_00538c50(uVar1,param_2,param_3);
  return;
}
```

### decoy: 0x0051ca40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051ca40: 16-byte aligned: YES
  0051ca40  55             PUSH EBP
  0051ca41  8B EC          MOV EBP,ESP
  0051ca43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051ca40
string literals (0):
direct callees (3):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EF0  (unnamed)
  0x0054B130  CSWSArea::ComputeHeight
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

## Card B9-3-48

GOG function: CServerExoAppInternal::StopServices  gog 0x007b9ef0  callconv=(blank)  param_size_bytes=0  gog_size=2147  gog_retn=0
Candidate Steam address: 0x00536c10 (size 2147, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 2 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'HD0:chitin'@00994144; 'ERRORTEX:'@00994150; CALLEE 5 (callees: Destructor 0x7114d0, RemoveFixedKeyTableFile 0x7116f0, RemoveResourceDirectory 0x711710); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00537480:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b96e0 CServerExoAppInternal::StartServices -> 0x005363a0  gate_class=REVIEW
- GOG-order neighbour after:  0x007ba760 CServerExoAppInternal::UpdateClientGameObjects -> 0x00537510  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (22), mapped to Steam through trusted rows only:
  0x00438080 CServerInfo::Destructor -> Steam ?  in candidate callees: n/a
  0x004693f0 CGameObjectArray::Destructor -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x0060c500 CExoDebug::GetCurrentTimestamp -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  ... 14 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (7): 'TEMP:'; 'OVERRIDE:'; 'SERVERVAULT:'; 'GAMEINPROGRESS:' ...
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0xf8*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0xf8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00536c10: 16-byte aligned: YES
  00536c10  55             PUSH EBP
  00536c11  8B EC          MOV EBP,ESP
  00536c13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00536c10
string literals (7):
  0x00993E5C  at 00536fba  'TEMP:'
  0x00993F0C  at 00536fea  'CURRENTGAME:'
  0x00993E84  at 0053701a  'GAMEINPROGRESS:'
  0x00994144  at 005370a2  'HD0:chitin'
  0x0099415C  at 005370ce  'OVERRIDE:'
  0x00994150  at 00537103  'ERRORTEX:'
  0x00993EF0  at 00537138  'SERVERVAULT:'
direct callees (22):
  0x00733540  CExoString::Constructor
  0x0073A590  (unnamed)
  0x00734010  CExoString::CStr
  0x00537870  (unnamed)
  0x0053D780  (unnamed)
  0x009196FD  operator delete
  0x0051D510  (unnamed)
  0x00537480  (unnamed)
  0x00704580  (unnamed)
  0x00704480  CNetLayer::Unlink (mapper: EndServerMode)
  0x00703D70  CNetLayer dtor
  0x006FD7B0  CVirtualMachine::Destructor
  0x006FD6B0  CVirtualMachineStack::ClearStack
  0x00733570  CExoString::CExoString_char
  0x00715710  CExoResMan::NukeDirectory
  0x00733780  CExoString::~CExoString
  0x007116F0  CExoResMan::RemoveFixedKeyTableFile
  0x00711710  CExoResMan::RemoveResourceDirectory
  0x0071D6E0  (unnamed)
  0x007114D0  CExoResMan::Destructor
struct offsets touched (top 10):
  +0x10040 [dec 65600]  x5  via EAX,ECX,EDX
  +0x10004 [dec 65540]  x3  via EAX,EDX
  +0x1000C [dec 65548]  x3  via EAX,ECX
  +0x10010 [dec 65552]  x3  via EAX,ECX,EDX
  +0x10044 [dec 65604]  x3  via ECX,EDX
  +0x10048 [dec 65608]  x3  via EAX,ECX
  +0x1004C [dec 65612]  x3  via EAX,EDX
  +0x10050 [dec 65616]  x3  via ECX,EDX
  +0x10054 [dec 65620]  x3  via EAX,ECX
  +0x1005C [dec 65628]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00536c10 FUN_00536c10 size=2147 ret=0 callers=1 callees=22

void __fastcall FUN_00536c10(int param_1)

{
  int iVar1;
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined4 *local_78;
  undefined4 *local_74;
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  int local_58;
  int local_54;
  int local_50;
  int local_4c;
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  undefined4 *local_38;
  undefined4 *local_34;
  undefined4 *local_30;
  undefined4 *local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094ee79;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_0073a590(local_18);
  FUN_00734010();
  FUN_00537870(0,0);
  if (*(int *)(param_1 + 0x1005c) != 0) {
    local_20 = *(int *)(param_1 + 0x1005c);
    local_1c = local_20;
    if (local_20 != 0) {
      FUN_0053d780();
      FUN_009196fd(local_1c);
    }
    *(undefined4 *)(param_1 + 0x1005c) = 0;
  }
  if (*(int *)(param_1 + 0x10044) != 0) {
    local_28 = *(int *)(param_1 + 0x10044);
    local_24 = local_28;
    if (local_28 != 0) {
      FUN_0051d510();
      FUN_009196fd(local_24);
    }
    *(undefined4 *)(param_1 + 0x10044) = 0;
  }
  if (*(int *)(param_1 + 0x1000c) != 0) {
    local_30 = *(undefined4 **)(param_1 + 0x1000c);
    local_2c = local_30;
    if (local_30 != (undefined4 *)0x0) {
      (**(code **)*local_30)(1);
    }
    *(undefined4 *)(param_1 + 0x1000c) = 0;
  }
  if (*(int *)(param_1 + 0x10010) != 0) {
    local_38 = *(undefined4 **)(param_1 + 0x10010);
    local_34 = local_38;
    if (local_38 != (undefined4 *)0x0) {
      (**(code **)*local_38)(1);
    }
    *(undefined4 *)(param_1 + 0x10010) = 0;
  }
  if (*(int *)(param_1 + 0x10004) != 0) {
    local_40 = *(int *)(param_1 + 0x10004);
/* ... 134 more lines truncated by --max-lines */
```

### decoy: 0x00537480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00537480: 16-byte aligned: YES
  00537480  55             PUSH EBP
  00537481  8B EC          MOV EBP,ESP
  00537483  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00537480
string literals (0):
direct callees (1):
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B9-3-49

GOG function: CServerExoAppInternal::StoreCurrentModule  gog 0x007b39e0  callconv=(blank)  param_size_bytes=0  gog_size=475  gog_retn=0
Candidate Steam address: 0x00530660 (size 475, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=SaveModuleForGameInProgress | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005302e0:STR0/CALLEE1/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b3660 CServerExoAppInternal::StorePlayerCharacters -> 0x005302e0  gate_class=AUTO
- GOG-order neighbour after:  0x007b3bc0 CSWSMessage::SendServerToPlayerModule_StartStartNewModule  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x006062a0 CExoString::Find_2 -> Steam ?  in candidate callees: n/a
  0x00606980 CExoString::Right -> Steam ?  in candidate callees: n/a
  0x0072dfe0 CSWSModule::SaveModuleStart -> Steam ?  in candidate callees: n/a
  ... 6 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007b3120 CServerExoAppInternal::StartNewModule -> Steam 0x0052fda0  in candidate callers: yes
GOG strings (1): 'GAMEINPROGRESS:'
disp sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x5*,0x3a*,0x5c*,0x68*} | Steam {0x2*,0x3*,0x4*,0x5*,0x3a*,0x5c*,0x68*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00530660: 16-byte aligned: YES
  00530660  55             PUSH EBP
  00530661  8B EC          MOV EBP,ESP
  00530663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00530660
string literals (1):
  0x00993E84  at 005306ec  'GAMEINPROGRESS:'
direct callees (14):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00561030  (unnamed)
  0x0053AB60  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x007335F0  CExoString::CExoString_copy
  0x00734190  CExoString::Find_2
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00919F10  _strlen
  0x00734870  CExoString::Right
  0x0055B830  CSWSModule::BeginModuleSave
  0x0055CF00  (unnamed)
  0x0055BB80  CSWSModule::SaveModuleFinish
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00530660 FUN_00530660 size=475 ret=0 callers=2 callees=14

undefined4 FUN_00530660(void)

{
  undefined4 uVar1;
  size_t local_54;
  undefined1 local_50 [8];
  undefined1 local_48 [8];
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  int local_30;
  int local_2c;
  undefined1 local_28 [8];
  char *local_20 [2];
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094e630;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = FUN_0052f4c0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_18 = *(undefined4 *)(DAT_00a1b4a4 + 0x14);
  if (local_14 != 0) {
    uVar1 = FUN_00561030(local_38);
    local_8 = 0;
    local_30 = FUN_0053ab60(uVar1);
    local_8 = 0xffffffff;
    FUN_00733780();
    if (local_30 != 0) {
      FUN_00733570("GAMEINPROGRESS:");
      local_8 = 1;
      FUN_007335f0(local_14 + 0x5c);
      local_8._0_1_ = 2;
      local_2c = FUN_00734190(0x3a,0);
      if (local_2c == -1) {
        uVar1 = FUN_00733df0(local_40,local_20);
        local_8._0_1_ = 3;
        FUN_007337c0(uVar1);
        local_8 = CONCAT31(local_8._1_3_,2);
        FUN_00733780();
      }
      else {
        if (local_20[0] == (char *)0x0) {
          local_54 = 0;
        }
        else {
          local_54 = _strlen [GHIDRA-AUTO-LABEL: unreliable](local_20[0]);
        }
        uVar1 = FUN_00734870(local_48,(local_54 - local_2c) + -1);
        local_8._0_1_ = 4;
        uVar1 = FUN_00733df0(local_50,uVar1);
        local_8._0_1_ = 5;
        FUN_007337c0(uVar1);
        local_8._0_1_ = 4;
        FUN_00733780();
        local_8 = CONCAT31(local_8._1_3_,2);
        FUN_00733780();
      }
      FUN_0055b830(local_28,local_20);
      FUN_0055cf00();
      FUN_0055bb80(local_28,local_20);
      local_8 = CONCAT31(local_8._1_3_,1);
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
    }
  }
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x005302e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005302e0: 16-byte aligned: YES
  005302e0  55             PUSH EBP
  005302e1  8B EC          MOV EBP,ESP
  005302e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005302e0
string literals (2):
  0x00993E6C  at 005303e2  'Mod_PlayerList'
  0x00993E5C  at 00530532  'TEMP:'
direct callees (22):
  0x00716E60  CResGFF::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x007171B0  CResGFF::Destructor_2
  0x00719740  CResGFF::AddList
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00719800  CResGFF::AddListElement
  0x00541080  CSWSObject::ClearAllActions
  0x00574500  CSWSCreature::SaveCreature
  0x0058C370  (unnamed)
  0x00733540  CExoString::Constructor
  0x007338D0  CExoString::operator=_2
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00734010  CExoString::CStr
  0x0071A5A0  CResGFF::WriteGFFFile
  0x005F7EC0  CSWPartyTable::UpdateMembers
struct offsets touched (top 5):
  +0x80 [dec 128]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0x38 [dec 56]  x1  via EDX
  +0x10064 [dec 65636]  x1  via EAX
```

## Card B9-3-50

GOG function: CServerExoAppInternal::ContinueMessageProcessing  gog 0x007baa80  callconv=(blank)  param_size_bytes=0  gog_size=47  gog_retn=0
Candidate Steam address: 0x00537830 (size 47, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005377c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007baa10 CServerExoAppInternal::ValidatePlayerLogin -> 0x005377c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007baac0 CServerExoAppInternal::Shutdown -> 0x00537870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00537830: 16-byte aligned: YES
  00537830  55             PUSH EBP
  00537831  8B EC          MOV EBP,ESP
  00537833  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00537830
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00537830 FUN_00537830 size=47 ret=0 callers=1 callees=0

bool FUN_00537830(void)

{
  return **(int **)(DAT_00a1b4a4 + 0x14) == 1;
}
```

### decoy: 0x005377c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005377c0: 16-byte aligned: YES
  005377c0  55             PUSH EBP
  005377c1  8B EC          MOV EBP,ESP
  005377c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005377c0
string literals (0):
direct callees (2):
  0x00647020  (unnamed)
  0x00647110  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x24 [dec 36]  x2  via EAX
  +0x10010 [dec 65552]  x2  via ECX,EDX
```

## Card B9-3-51

GOG function: CSWSModule::Destructor  gog 0x007294a0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00556cf0 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005570d0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00729480 CSWSModule::non_virtual_thunk_to_AsSWSModule -> 0x00556cd0  gate_class=REVIEW
- GOG-order neighbour after:  0x007294d0 CSWSModule::Destructor_2 -> 0x00556d20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007294d0 CSWSModule::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00556cf0: 16-byte aligned: YES
  00556cf0  55             PUSH EBP
  00556cf1  8B EC          MOV EBP,ESP
  00556cf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00556cf0
string literals (0):
direct callees (2):
  0x00556D20  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00556cf0 FUN_00556cf0 size=44 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_00556cf0(undefined4 param_1,uint param_2)

{
  FUN_00556d20();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x005570d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005570d0: 16-byte aligned: YES
  005570d0  55             PUSH EBP
  005570d1  8B EC          MOV EBP,ESP
  005570d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005570d0
string literals (0):
direct callees (3):
  0x00735360  CExoLocString::Destructor
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B9-3-52

GOG function: CServerExoAppInternal::MovePlayerToArea  gog 0x007b8d50  callconv=(blank)  param_size_bytes=4  gog_size=537  gog_retn=4
Candidate Steam address: 0x00535a10 (size 537, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.73) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: PositionParty 0x5f9a20, ComputeSafeLocation 0x54bdc0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005356d0:STR0/CALLEE0/SIG0.434/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b8a10 CServerExoAppInternal::InitiateModuleForPlayer -> 0x005356d0  gate_class=REVIEW
- GOG-order neighbour after:  0x007b8f70 CServerExoAppInternal::PlayerListChange -> 0x00535c30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00605ec0 CExoString::operator[] -> Steam ?  in candidate callees: n/a
  0x0064bc00 CServerExoApp::GetWaypointByGameObjectID -> Steam ?  in candidate callees: n/a
  0x00681800 CSWSCreature::AddToArea -> Steam ?  in candidate callees: n/a
  0x006aabf0 CSWSObject::SetOrientation -> Steam ?  in candidate callees: n/a
  0x006cd530 CSWSCreature::UpdateSubareasOnJumpPosition -> Steam ?  in candidate callees: n/a
  0x006dbb50 CSWSCreature::UpdateVisibleList -> Steam ?  in candidate callees: n/a
  0x006ff590 CSWPartyTable::PositionParty -> Steam 0x005f9a20  in candidate callees: yes
  ... 5 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0x2c*,0x30*,0x124*,0x350*,0x380*} | Steam {0x4*,0x8*,0x2c*,0x30*,0x124*,0x350*,0x380*}
imm sets (* = in both, 8 common): GOG {0x4*,0x48*,0x94*,0xa0*,0x354*,0x1008c*,0x1f0b4*,0x7f000000*} | Steam {0x4*,0x48*,0x94*,0xa0*,0x354*,0x1008c*,0x1f0b4*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00535a10: 16-byte aligned: YES
  00535a10  55             PUSH EBP
  00535a11  8B EC          MOV EBP,ESP
  00535a13  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00535a10
string literals (0):
direct callees (13):
  0x00632A20  (unnamed)
  0x00733DB0  CExoString::operator[]
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EC0  (unnamed)
  0x0051C1E0  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
  0x007338D0  CExoString::operator=_2
  0x00557EF0  (unnamed)
  0x0054BDC0  CSWSArea::ComputeSafeLocation
  0x005C2770  CSWSCreature::UpdateSubareasOnJumpPosition
  0x00568BD0  CSWSCreature::AddToArea
  0x005D0F30  CSWSObject::UpdatePerception
  0x005F9A20  CSWPartyTable::PositionParty
struct offsets touched (top 8):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x380 [dec 896]  x2  via ECX,EDX
  +0x100C4 [dec 65732]  x2  via EAX,EDX
  +0x2C [dec 44]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x350 [dec 848]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00535a10 FUN_00535a10 size=537 ret=4 callers=2 callees=13

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00535a10(int param_1)

{
  char cVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_20 = 0;
  local_1c = 0;
  local_18 = 0;
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  iVar2 = FUN_00632a20();
  if (iVar2 != 0) {
    piVar3 = (int *)FUN_00632a20();
    iVar2 = (**(code **)(*piVar3 + 0x30))();
    cVar1 = FUN_00733db0(0);
    if (cVar1 != '\0') {
      iVar4 = param_1 + 0x1008c;
      FUN_0052f4c0(iVar4);
      iVar4 = FUN_00557ec0(iVar4);
      if ((iVar4 != 0x7f000000) && (iVar4 = FUN_0051c1e0(iVar4), iVar4 != 0)) {
        *(undefined4 *)(iVar2 + 0x354) = *(undefined4 *)(iVar4 + 0x94);
        *(undefined4 *)(iVar2 + 0x358) = *(undefined4 *)(iVar4 + 0x98);
        *(undefined4 *)(iVar2 + 0x35c) = *(undefined4 *)(iVar4 + 0x9c);
        FUN_00543f10(iVar4 + 0xa0);
      }
      FUN_007338d0(&DAT_00986925);
    }
    local_10 = *(undefined4 *)(iVar2 + 0x354);
    local_c = *(undefined4 *)(iVar2 + 0x358);
    local_8 = *(undefined4 *)(iVar2 + 0x35c);
    FUN_0052f4c0();
    local_14 = FUN_00557ef0();
    *(undefined4 *)(*(int *)(iVar2 + 0x380) + 0x2c) = *(undefined4 *)(iVar2 + 4);
    if ((local_14 != 0) &&
       (iVar2 = FUN_0054bdc0(&local_10,_DAT_0098bd6c,*(undefined4 *)(iVar2 + 0x380),1,&local_20),
       iVar2 == 0)) {
      local_20 = local_10;
      local_1c = local_c;
      local_18 = local_8;
    }
    if (local_14 != 0) {
      FUN_005c2770(&local_20,*(undefined4 *)(local_14 + 0x124),*(int *)(param_1 + 0x100c4) == 0);
      FUN_00568bd0(local_14,local_20,local_1c,local_18,0,0);
    }
    FUN_005d0f30(0);
    FUN_005f9a20();
    *(undefined4 *)(param_1 + 0x100c4) = 0;
  }
  return;
}
```

### decoy: 0x005356d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005356d0: 16-byte aligned: YES
  005356d0  55             PUSH EBP
  005356d1  8B EC          MOV EBP,ESP
  005356d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005356d0
string literals (0):
direct callees (16):
  0x00632A20  (unnamed)
  0x00919723  operator new
  0x00561F30  CSWSCreature::Constructor
  0x00632830  (unnamed)
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EF0  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
  0x005453C0  CSWSObject::GetArea
  0x0051C120  (unnamed)
  0x00545420  CSWSObject::SetArea
  0x0051C2E0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x006305B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00637620  (unnamed)
  0x00639F40  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x58 [dec 88]  x5  via EAX
  +0x350 [dec 848]  x3  via EAX,ECX,EDX
  +0x124 [dec 292]  x2  via ECX,EDX
  +0x360 [dec 864]  x2  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
```

## Card B9-3-53

GOG function: CSWSModule::DeleteWorldJournalEntry  gog 0x00731860  callconv=(blank)  param_size_bytes=4  gog_size=263  gog_retn=4
Candidate Steam address: 0x0055f1d0 (size 263, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandDeleteJournalWorldEntry 0x670580)
- best decoy (neighbour/other candidate) scores: 0x0055f2e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00731700 CSWSModule::AddWorldJournalEntryStrref -> 0x0055f070  gate_class=AUTO
- GOG-order neighbour after:  0x00731970 CSWSModule::DeleteWorldJournalEntryStrref -> 0x0055f2e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00547d90 CExoArrayList_CWorldJournalEntry_::DelIndex -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x0064bcc0 CServerExoApp::GetPlayerList -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam ?  in candidate callees: n/a
  0x0075ca90 CSWSPlayer::GetGameObject -> Steam ?  in candidate callees: n/a
  0x007e2250 CSWSMessage::SendServerToPlayerJournalDeleteWorld -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00785b60 Global::ExecuteCommandDeleteJournalWorldEntry -> Steam 0x00670580  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x8*,0x1c*,0x20*,0x1ec*,0x1f0*} | Steam {0x8*,0x1c*,0x20*,0x1ec*,0x1f0*}
imm sets (* = in both, 3 common): GOG {0x4*,0x24*,0x1ec*} | Steam {0x4*,0x24*,0x1ec*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0055f1d0: 16-byte aligned: YES
  0055f1d0  55             PUSH EBP
  0055f1d1  8B EC          MOV EBP,ESP
  0055f1d3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0055f1d0
string literals (0):
direct callees (8):
  0x0051C2E0  (unnamed)
  0x00561750  (unnamed)
  0x0051C2A0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00632A20  (unnamed)
  0x0064BB30  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX
  +0x1EC [dec 492]  x2  via ECX,EDX
  +0x1F0 [dec 496]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0055f1d0 FUN_0055f1d0 size=263 ret=4 callers=2 callees=8

void FUN_0055f1d0(int param_1)

{
  int iVar1;
  int extraout_var;
  int iVar2;
  undefined4 uVar3;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_10 = 0;
  uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  iVar1 = FUN_0051c2e0();
  local_c = *(int *)(extraout_var + 0x1f0);
  do {
    iVar2 = extraout_var;
    if (local_c < 0) {
LAB_0055f25f:
      if (iVar1 != 0) {
        local_8 = FUN_0051c2a0(iVar2,*(undefined4 *)(DAT_00a1b4a4 + 8),uVar3);
        if (local_8 != 0) {
          local_14 = FUN_007a1720();
          while (local_14 != 0) {
            local_10 = FUN_00561430(local_14);
            if (local_10 != 0) {
              iVar1 = FUN_00632a20();
              if (iVar1 != 0) {
                FUN_0064bb30(local_10,param_1);
              }
            }
            FUN_0058c370(&local_14);
          }
        }
      }
      return;
    }
    if (((*(uint *)(*(int *)(extraout_var + 0x1ec) + 0x20 + local_c * 0x24) & 1) == 0) &&
       (*(int *)(*(int *)(extraout_var + 0x1ec) + 0x1c + local_c * 0x24) == param_1)) {
      FUN_00561750(local_c);
      goto LAB_0055f25f;
    }
    local_c = local_c + -1;
  } while( true );
}
```

### decoy: 0x0055f2e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055f2e0: 16-byte aligned: YES
  0055f2e0  55             PUSH EBP
  0055f2e1  8B EC          MOV EBP,ESP
  0055f2e3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0055f2e0
string literals (0):
direct callees (8):
  0x0051C2E0  (unnamed)
  0x00561750  (unnamed)
  0x0051C2A0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00632A20  (unnamed)
  0x0064BBA0  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX
  +0x1EC [dec 492]  x2  via ECX,EDX
  +0x1F0 [dec 496]  x1  via EDX
```

## Card B9-3-54

GOG function: CServerExoAppInternal::RunModule  gog 0x007b9510  callconv=(blank)  param_size_bytes=0  gog_size=464  gog_retn=0
Candidate Steam address: 0x005361d0 (size 464, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: CreateParty 0x5f9220); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00535c30:STR0/CALLEE0/SIG0.133/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b8f70 CServerExoAppInternal::PlayerListChange -> 0x00535c30  gate_class=REVIEW
- GOG-order neighbour after:  0x007b96e0 CServerExoAppInternal::StartServices -> 0x005363a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x006fed90 CSWPartyTable::CreateParty -> Steam 0x005f9220  in candidate callees: yes
  0x007b2840 CServerExoAppInternal::GetModule -> Steam ?  in candidate callees: n/a
  0x007b3bf0 CServerExoAppInternal::SendServerToPlayerModule_EndStartNewModule -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x24*,0x1e8*} | Steam {0x24*,0x1e8*}
imm sets (* = in both, 3 common): GOG {0x2*,0x30*,0x1f0b4*} | Steam {0x2*,0x30*,0x1f0b4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005361d0: 16-byte aligned: YES
  005361d0  55             PUSH EBP
  005361d1  8B EC          MOV EBP,ESP
  005361d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005361d0
string literals (0):
direct callees (13):
  0x00733540  CExoString::Constructor
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x00647AF0  (unnamed)
  0x00665B30  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00535A10  (unnamed)
  0x0058C370  (unnamed)
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00531370  (unnamed)
  0x005F9220  CSWPartyTable::CreateParty
  0x00530870  (unnamed)
struct offsets touched (top 6):
  +0x10064 [dec 65636]  x5  via EAX,ECX,EDX
  +0x10008 [dec 65544]  x3  via EAX,EDX
  +0x1007C [dec 65660]  x3  via EAX,EDX
  +0x10010 [dec 65552]  x2  via EAX,ECX
  +0x24 [dec 36]  x1  via ECX
  +0x1E8 [dec 488]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005361d0 FUN_005361d0 size=464 ret=0 callers=1 callees=13

undefined4 __fastcall FUN_005361d0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094ece8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if ((*(short *)(param_1 + 0x10008) == 1) && (*(short *)(param_1 + 0x10008) != 2)) {
    *(undefined2 *)(param_1 + 0x10008) = 2;
    FUN_00623320();
    FUN_00647af0(DAT_0099b6c4);
    FUN_00665b30(DAT_0099b6cc);
    local_18 = FUN_007a1720();
    while (local_18 != 0) {
      local_14 = FUN_00561430(local_18);
      if (*(char *)(local_14 + 0x24) != '\0') {
        FUN_00535a10(local_14);
      }
      FUN_0058c370(&local_18);
    }
    iVar1 = FUN_0052f4c0();
    if ((*(int *)(iVar1 + 0x1e8) != 0) && (*(int *)(param_1 + 0x1007c) != 1)) {
      local_18 = FUN_007a1720();
      local_14 = FUN_00561430(local_18);
      FUN_00531370(local_14);
      FUN_005f9220();
    }
    if (*(int *)(param_1 + 0x1007c) == 1) {
      FUN_00530870(1);
      *(undefined4 *)(param_1 + 0x1007c) = 0;
    }
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = 1;
  }
  else {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = 0;
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00535c30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00535c30: 16-byte aligned: YES
  00535c30  55             PUSH EBP
  00535c31  8B EC          MOV EBP,ESP
  00535c33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00535c30
string literals (0):
direct callees (23):
  0x00733540  CExoString::Constructor
  0x0073A590  (unnamed)
  0x00704800  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x00734010  CExoString::CStr
  0x00919723  operator new
  0x00665DC0  (unnamed)
  0x0054AA10  CExoLinkedList::AddHead
  0x005210F0  (unnamed)
  0x0062FA00  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
  0x00665CD0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0064DDE0  (unnamed)
  0x0058C370  (unnamed)
  0x00647A80  (unnamed)
  0x00401B60  (unnamed)
  0x0051B980  (unnamed)
struct offsets touched (top 10):
  +0x10064 [dec 65636]  x8  via EAX,ECX,EDX
  +0x10068 [dec 65640]  x7  via EAX,ECX,EDX
  +0x10040 [dec 65600]  x6  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via EDX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x24 [dec 36]  x2  via EAX,ECX
  +0x10010 [dec 65552]  x2  via EAX,ECX
  +0x3C [dec 60]  x1  via EDX
  +0x10020 [dec 65568]  x1  via ECX
```

## Card B9-3-55

GOG function: CServerExoAppInternal::Shutdown  gog 0x007baac0  callconv=(blank)  param_size_bytes=8  gog_size=645  gog_retn=8
Candidate Steam address: 0x00537870 (size 645, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005377c0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007baa80 CServerExoAppInternal::ContinueMessageProcessing -> 0x00537830  gate_class=REVIEW
- GOG-order neighbour after:  0x007bad50 CServerExoAppInternal::GetItemByGameObjectID -> 0x00537b00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x00608350 CExoTimers::GetHighResolutionTimer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007bca70 unnamed -> Steam ?  in candidate callees: n/a
  0x007e3eb0 CSWSMessage::SendServerToPlayerShutDownServer -> Steam ?  in candidate callees: n/a
  0x007e4350 CExoArrayList_CAurObject__::SetSize -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x98*} | Steam {0x4*,0x98*}
imm sets (* = in both, 7 common): GOG {0x8*,0x24*,0x100*,0x1001c*,0x186a0*,0xf4240*,0xaba9500*} | Steam {0x8*,0x24*,0x100*,0x1001c*,0x186a0*,0xf4240*,0xaba9500*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00537870: 16-byte aligned: YES
  00537870  55             PUSH EBP
  00537871  8B EC          MOV EBP,ESP
  00537873  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00537870
string literals (0):
direct callees (10):
  0x00840EF0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0091D220  (unnamed)
  0x0064D790  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x0058C370  (unnamed)
  0x00476620  (unnamed)
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x00539820  (unnamed)
struct offsets touched (top 9):
  +0x10064 [dec 65636]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX
  +0x10068 [dec 65640]  x5  via EAX,ECX,EDX
  +0x98 [dec 152]  x1  via ECX
  +0x10004 [dec 65540]  x1  via EAX
  +0x10010 [dec 65552]  x1  via EAX
  +0x10020 [dec 65568]  x1  via EDX
  +0x10038 [dec 65592]  x1  via EDX
  +0x1003C [dec 65596]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00537870 FUN_00537870 size=645 ret=8 callers=4 callees=10

void __thiscall FUN_00537870(int param_1,int param_2,int param_3)

{
  undefined4 uVar1;
  int iVar2;
  undefined8 uVar3;
  undefined4 uVar4;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_8 = *(int *)(*(int *)(param_1 + 0x10004) + 0x98);
  FUN_00840ef0(0);
  local_10 = FUN_007a1720();
  if ((local_8 == 0) || (param_2 == 0)) {
    local_14 = 0x100;
    while ((local_10 != 0 && (local_14 = local_14 + -1, 0 < local_14))) {
      local_c = FUN_00561430(local_10);
      if (local_c == 0) {
        FUN_0058c370(&local_10);
      }
      else if (param_3 == 0) {
        local_10 = FUN_007a1720();
      }
      else {
        FUN_0083ea60(*(undefined4 *)(local_c + 4));
        FUN_0058c370(&local_10);
      }
    }
  }
  else {
    while (local_10 != 0) {
      local_c = FUN_00561430(local_10);
      if (local_c != 0) {
        uVar4 = *(undefined4 *)(local_c + 4);
        uVar1 = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x10038),*(undefined4 *)(param_1 + 0x1003c),
                          100000,0);
        FUN_0064d790(uVar4,uVar1);
        FUN_0083ea60(*(undefined4 *)(local_c + 4));
      }
      FUN_0058c370(&local_10);
    }
  }
  local_10 = FUN_007a1720();
  local_14 = 0x100;
  while ((local_10 != 0 && (local_14 = local_14 + -1, 0 < local_14))) {
    local_c = FUN_00561430(local_10);
    if (local_c == 0) {
      FUN_0058c370(&local_10);
    }
    else {
      iVar2 = FUN_00476620(*(undefined4 *)(local_c + 4));
      if (iVar2 == 0) {
        local_10 = FUN_007a1720();
      }
      else {
        FUN_0058c370(&local_10);
      }
    }
  }
  if (((local_8 == 0) || (param_2 == 0)) || (*(int *)(param_1 + 0x10020) == 0)) {
    uVar1 = 0;
    uVar4 = 1000000;
    uVar3 = FUN_00736220(1000000,0);
    FUN_00539820(uVar3,uVar4,uVar1);
  }
  else {
    uVar1 = 0;
    uVar4 = 180000000;
    uVar3 = FUN_00736220(180000000,0);
    FUN_00539820(uVar3,uVar4,uVar1);
  }
  return;
}
```

### decoy: 0x005377c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005377c0: 16-byte aligned: YES
  005377c0  55             PUSH EBP
  005377c1  8B EC          MOV EBP,ESP
  005377c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005377c0
string literals (0):
direct callees (2):
  0x00647020  (unnamed)
  0x00647110  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x24 [dec 36]  x2  via EAX
  +0x10010 [dec 65552]  x2  via ECX,EDX
```

## Card B9-3-56

GOG function: CServerExoAppInternal::Initialize  gog 0x007b4920  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x005315a0 (size 49, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR  (content level C1)
- evidence summary: STR 'globalcat'@00993f00
- best decoy (neighbour/other candidate) scores: 0x005314e0:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b4860 CServerExoAppInternal::HandleMessage -> 0x005314e0  gate_class=AUTO
- GOG-order neighbour after:  0x007b4960 CServerExoAppInternal::UnloadModule -> 0x005315e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
  0x00650d00 CSWGlobalVariableTable::ReadCatalogue -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'globalcat'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x14*,0x100fc*} | Steam {0x14*,0x100fc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005315a0: 16-byte aligned: YES
  005315a0  55             PUSH EBP
  005315a1  8B EC          MOV EBP,ESP
  005315a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005315a0
string literals (1):
  0x00993F00  at 005315a9  'globalcat'
direct callees (2):
  0x00710840  (unnamed)
  0x00653E40  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005315a0 FUN_005315a0 size=49 ret=0 callers=1 callees=2

undefined4 FUN_005315a0(void)

{
  undefined1 local_14 [16];
  
  FUN_00710840("globalcat");
  FUN_00653e40(local_14);
  return 1;
}
```

### decoy: 0x005314e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005314e0: 16-byte aligned: YES
  005314e0  55             PUSH EBP
  005314e1  8B EC          MOV EBP,ESP
  005314e3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005314e0
string literals (0):
direct callees (3):
  0x00704800  (unnamed)
  0x00662F60  (unnamed)
  0x0065F8A0  (unnamed)
struct offsets touched (top 4):
  +0x10010 [dec 65552]  x2  via EAX,ECX
  +0x10040 [dec 65600]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x24 [dec 36]  x1  via ECX
```

## Card B9-3-57

GOG function: CServerExoApp::LoadGame  gog 0x0064b840  callconv=__thiscall  param_size_bytes=16  gog_size=40  gog_retn=16
Candidate Steam address: 0x0051bdf0 (size 40, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CServerExoApp::SaveGame | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051be20:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: autolabel:DoHillClimbing
- GOG-order neighbour before: 0x0064b810 CServerExoApp::SaveGame -> 0x0051bdf0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b870 CServerExoApp::SetMoveToModulePending -> 0x0051be50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2f00 CServerExoAppInternal::LoadGame -> Steam 0x0052fb80  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0051bdf0: 16-byte aligned: YES
  0051bdf0  55             PUSH EBP
  0051bdf1  8B EC          MOV EBP,ESP
  0051bdf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bdf0
string literals (0):
direct callees (1):
  0x0052F610  CServerExoAppInternal::SaveGame
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bdf0 Concurrency::details::SchedulerProxy::DoHillClimbing size=40 ret=16 callers=2 callees=1
THUNK -> 0x0052F610 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: unsigned int __thiscall Concurrency::details::SchedulerProxy::DoHillClimbing(unsigned
   int,unsigned int,unsigned int,unsigned int)
   
   Libraries: Visual Studio 2010 Debug, Visual Studio 2012 Debug, Visual Studio 2015 Debug, Visual
   Studio 2015 Release */

uint __thiscall
Concurrency::details::SchedulerProxy::DoHillClimbing
          (SchedulerProxy *this,uint param_1,uint param_2,uint param_3,uint param_4)

{
  uint uVar1;
  
  uVar1 = FUN_0052f610(param_1,param_2,param_3,param_4);
  return uVar1;
}
```

### decoy: 0x0051be20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051be20: 16-byte aligned: YES
  0051be20  55             PUSH EBP
  0051be21  8B EC          MOV EBP,ESP
  0051be23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051be20
string literals (0):
direct callees (1):
  0x0052FB80  CServerExoAppInternal::LoadGame (SAVES:%06d - %s; partytable, globals 0x656710, LoadModule)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-3-58

GOG function: CServerExoApp::CreatureAcquireItem  gog 0x0064c680  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0051cd90 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cd60:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c620 CServerExoApp::GetRoomIndex -> 0x0051cc00  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c6a0 CServerExoApp::GetFactionManager -> 0x0051cc80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bd1d0 CServerExoAppInternal::CreatureAcquireItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0040cec0 CClientExoApp::CreatureAcquireItem -> Steam 0x0073f9e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0051cd90: 16-byte aligned: YES
  0051cd90  55             PUSH EBP
  0051cd91  8B EC          MOV EBP,ESP
  0051cd93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cd90
string literals (0):
direct callees (1):
  0x0053A7E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cd90 FUN_0051cd90 size=32 ret=8 callers=2 callees=1
THUNK -> 0x0053A7E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051cd90(undefined4 param_1,undefined4 param_2)

{
  FUN_0053a7e0(param_1,param_2);
  return;
}
```

### decoy: 0x0051cd60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cd60: 16-byte aligned: YES
  0051cd60  55             PUSH EBP
  0051cd61  8B EC          MOV EBP,ESP
  0051cd63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cd60
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1F320 [dec 127776]  x1  via EDX
```

## Card B9-3-59

GOG function: CSWSModule::DeleteWorldJournalEntryStrref  gog 0x00731970  callconv=(blank)  param_size_bytes=4  gog_size=263  gog_retn=4
Candidate Steam address: 0x0055f2e0 (size 263, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandDeleteJournalWorldEntryStrref 0x670670)
- best decoy (neighbour/other candidate) scores: 0x0055f1d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00731860 CSWSModule::DeleteWorldJournalEntry -> 0x0055f1d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00731a80 CSWSModule::DeleteWorldJournalAllEntries -> 0x0055f3f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00547d90 CExoArrayList_CWorldJournalEntry_::DelIndex -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x0064bcc0 CServerExoApp::GetPlayerList -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam ?  in candidate callees: n/a
  0x0075ca90 CSWSPlayer::GetGameObject -> Steam ?  in candidate callees: n/a
  0x007e22c0 CSWSMessage::SendServerToPlayerJournalDeleteWorldStrref -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00785c50 Global::ExecuteCommandDeleteJournalWorldEntryStrref -> Steam 0x00670670  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x8*,0x1c*,0x20*,0x1ec*,0x1f0*} | Steam {0x8*,0x1c*,0x20*,0x1ec*,0x1f0*}
imm sets (* = in both, 3 common): GOG {0x4*,0x24*,0x1ec*} | Steam {0x4*,0x24*,0x1ec*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0055f2e0: 16-byte aligned: YES
  0055f2e0  55             PUSH EBP
  0055f2e1  8B EC          MOV EBP,ESP
  0055f2e3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0055f2e0
string literals (0):
direct callees (8):
  0x0051C2E0  (unnamed)
  0x00561750  (unnamed)
  0x0051C2A0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00632A20  (unnamed)
  0x0064BBA0  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX
  +0x1EC [dec 492]  x2  via ECX,EDX
  +0x1F0 [dec 496]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0055f2e0 FUN_0055f2e0 size=263 ret=4 callers=2 callees=8

void FUN_0055f2e0(int param_1)

{
  int iVar1;
  int iVar2;
  int extraout_var;
  undefined4 uVar3;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_10 = 0;
  uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  iVar1 = FUN_0051c2e0();
  local_c = *(int *)(extraout_var + 0x1f0);
  do {
    iVar2 = extraout_var;
    if (local_c < 0) {
LAB_0055f36f:
      local_8 = FUN_0051c2a0(iVar2,*(undefined4 *)(DAT_00a1b4a4 + 8),uVar3);
      if (local_8 != 0) {
        local_14 = FUN_007a1720();
        while (local_14 != 0) {
          local_10 = FUN_00561430(local_14);
          if (local_10 != 0) {
            iVar2 = FUN_00632a20();
            if ((iVar2 != 0) && (iVar1 != 0)) {
              FUN_0064bba0(local_10,param_1);
            }
          }
          FUN_0058c370(&local_14);
        }
      }
      return;
    }
    if (((*(uint *)(*(int *)(extraout_var + 0x1ec) + 0x20 + local_c * 0x24) & 1) != 0) &&
       (*(int *)(*(int *)(extraout_var + 0x1ec) + 0x1c + local_c * 0x24) == param_1)) {
      FUN_00561750(local_c);
      goto LAB_0055f36f;
    }
    local_c = local_c + -1;
  } while( true );
}
```

### decoy: 0x0055f1d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055f1d0: 16-byte aligned: YES
  0055f1d0  55             PUSH EBP
  0055f1d1  8B EC          MOV EBP,ESP
  0055f1d3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0055f1d0
string literals (0):
direct callees (8):
  0x0051C2E0  (unnamed)
  0x00561750  (unnamed)
  0x0051C2A0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00632A20  (unnamed)
  0x0064BB30  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX
  +0x1EC [dec 492]  x2  via ECX,EDX
  +0x1F0 [dec 496]  x1  via EDX
```

## Card B9-3-60

GOG function: CServerExoApp::GetModulePreviouslyVisited  gog 0x0064ca90  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051d070 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051d090:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ca50 CServerExoApp::GetMoveToWorldTime -> 0x0051d030  gate_class=REVIEW
- GOG-order neighbour after:  0x0064cab0 CServerExoApp::ExecuteLoadModule -> 0x0051d090  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bfcb0 CServerExoAppInternal::GetModulePreviouslyVisited -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051d070: 16-byte aligned: YES
  0051d070  55             PUSH EBP
  0051d071  8B EC          MOV EBP,ESP
  0051d073  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051d070
string literals (0):
direct callees (1):
  0x0053CF10  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051d070 FUN_0051d070 size=28 ret=4 callers=1 callees=1
THUNK -> 0x0053CF10 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051d070(undefined4 param_1)

{
  FUN_0053cf10(param_1);
  return;
}
```

### decoy: 0x0051d090 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051d090: 16-byte aligned: YES
  0051d090  55             PUSH EBP
  0051d091  8B EC          MOV EBP,ESP
  0051d093  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051d090
string literals (0):
direct callees (1):
  0x00530170  CServerExoAppInternal::ExecuteLoadModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```
