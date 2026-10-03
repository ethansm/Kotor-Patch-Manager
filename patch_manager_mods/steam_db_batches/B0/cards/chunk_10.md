# Review cards B0 chunk 10

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-10-01

GOG function: CSWSItemPropertyHandler::ApplyRapidShotBonus  gog 0x007c9f50  callconv=__thiscall  param_size_bytes=4  gog_size=34  gog_retn=20
Candidate Steam address: 0x005b2b70 (size 68, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplyMaxDexBonus
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2b40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9f20 CSWSItemPropertyHandler::ApplySniperBonus -> 0x005b2b10  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9f80 CSWSItemPropertyHandler::ApplyMaxDexBonus -> 0x005b2b70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2e4} | Steam {0x6,0x2ec}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0xc,0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005b2b70: 16-byte aligned: YES
  005b2b70  55             PUSH EBP
  005b2b71  8B EC          MOV EBP,ESP
  005b2b73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2b70
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2EC [dec 748]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2b70 FUN_005b2b70 size=68 ret=20 callers=0 callees=0

undefined4 FUN_005b2b70(int param_1,int param_2)

{
  if (param_1 != 0) {
    *(uint *)(param_1 + 0x2ec) = (uint)*(ushort *)(param_2 + 6) + *(int *)(param_1 + 0x2ec);
  }
  return 0;
}
```

### decoy: 0x005b2b40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2b40: 16-byte aligned: YES
  005b2b40  55             PUSH EBP
  005b2b41  8B EC          MOV EBP,ESP
  005b2b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2b40
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E4 [dec 740]  x1  via EAX
```

## Card B0-10-02

GOG function: CSWVirtualMachineCommands::ExecuteCommandEffectTemporaryForcePoints  gog 0x0078c440  callconv=__thiscall  param_size_bytes=8  gog_size=418  gog_retn=8
Candidate Steam address: 0x00676e60 (size 418, retn 8)
Candidate sources: branch+mapper:seeded+name_map:high:batch6_reverse_engineering+near    mapper tag: seeded

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00677010:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078c300 Global::ExecuteCommandEffectForceResistanceIncrease  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0078c5f0 Global::ExecuteCommandEffectTemporaryHP  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam ?  in candidate callees: n/a
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x30*} | Steam {0x8*,0xa*,0x30*}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0x38*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x8*,0x38*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00676e60: 16-byte aligned: YES
  00676e60  55             PUSH EBP
  00676e61  8B EC          MOV EBP,ESP
  00676e63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00676e60
string literals (0):
direct callees (10):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x30 [dec 48]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00676e60 FUN_00676e60 size=418 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00676e60(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int **ppiVar5;
  int local_44;
  undefined2 local_38;
  undefined2 local_34;
  int local_20;
  int *local_1c;
  int local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954f5b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_20);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_00919723(0x8c,uVar1);
    local_8 = 0;
    if (iVar2 == 0) {
      local_44 = 0;
    }
    else {
      local_44 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_18 = local_44;
    *(ushort *)(local_44 + 0xa [dec 10]) = *(ushort *)(local_44 + 0xa [dec 10]) & 0xffe7 | 8;
    if (local_20 < 1) {
      local_38 = (undefined2)DAT_00995f1c;
      *(undefined2 *)(local_44 + 8) = local_38;
    }
    else {
      local_34 = (undefined2)DAT_00996084;
      *(undefined2 *)(local_44 + 8) = local_34;
      FUN_005e4e80(0,local_20);
    }
    ppiVar5 = &local_1c;
    uVar3 = *(undefined4 *)(param_1 + 8);
    FUN_0051c080(uVar3,ppiVar5);
    local_11 = FUN_0053dfb0(uVar3,ppiVar5);
    if ((local_11 == DAT_0099447f) && (iVar2 = (**(code **)(*local_1c + 0x30))(), iVar2 != 0)) {
      FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
    }
    iVar4 = FUN_006fdad0(0,local_18);
    iVar2 = local_18;
    if (iVar4 == 0) {
      uVar3 = 0xfffff830;
    }
    else {
      if (local_18 != 0) {
        FUN_005e4ca0();
        FUN_009196fd(iVar2);
      }
      uVar3 = 0;
    }
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x00677010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00677010: 16-byte aligned: YES
  00677010  55             PUSH EBP
  00677011  8B EC          MOV EBP,ESP
  00677013  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00677010
string literals (0):
direct callees (10):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x30 [dec 48]  x1  via EAX
```

## Card B0-10-03

GOG function: CSWSDoor::GetFirstName  gog 0x007623f0  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x00616f40 (size 19, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00616f90:STR0/CALLEE0/SIG0.028/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007623b0 CSWSDoor::GetDialogResref -> 0x00616f00  gate_class=AUTO
- GOG-order neighbour after:  0x00762410 CSWSDoor::Destructor -> 0x00616f60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x3ec*} | Steam {0x3ec*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00616f40: 16-byte aligned: YES
  00616f40  55             PUSH EBP
  00616f41  8B EC          MOV EBP,ESP
  00616f43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00616f40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00616f40 FUN_00616f40 size=19 ret=0 callers=0 callees=0

int __fastcall FUN_00616f40(int param_1)

{
  return param_1 + 0x3ec;
}
```

### decoy: 0x00616f90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00616f90: 16-byte aligned: YES
  00616f90  55             PUSH EBP
  00616f91  8B EC          MOV EBP,ESP
  00616f93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00616f90
string literals (0):
direct callees (13):
  0x005453C0  CSWSObject::GetArea
  0x0061D040  CSWSDoor::RemoveFromArea
  0x0051C280  (unnamed)
  0x0051E0C0  CServerAIMaster::RemoveObject
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x0055EA80  (unnamed)
  0x006F3F10  (unnamed)
  0x009196FD  operator delete
  0x00735360  CExoLocString::Destructor
  0x00733780  CExoString::~CExoString
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0053F050  CSWSObject::Destructor_2
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via EAX
  +0x3D0 [dec 976]  x1  via EAX
```

## Card B0-10-04

GOG function: CSWVirtualMachineCommands::CreateGameDefinedStructure  gog 0x0077ed70  callconv=(blank)  param_size_bytes=4  gog_size=374  gog_retn=4
Candidate Steam address: 0x00669790 (size 374, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00669650:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0077ec30 CSWVirtualMachineCommands::DestroyGameDefinedStructure -> 0x00669650  gate_class=REVIEW
- GOG-order neighbour after:  0x0077eef0 Global::ExecuteCommandActionBarkString  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006a4e10 CScriptLocation::Constructor -> Steam ?  in candidate callees: n/a
  0x006a51b0 CScriptTalent::Constructor -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x0077a9b0 CSWCCMessageData::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x18*,0x34*,0x44*,0x8c*} | Steam {0x2*,0x3*,0x4*,0x18*,0x34*,0x44*,0x8c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00669790: 16-byte aligned: YES
  00669790  55             PUSH EBP
  00669791  8B EC          MOV EBP,ESP
  00669793  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00669790
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E1A60  (unnamed)
  0x0053E060  (unnamed)
  0x0053E400  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00669790 FUN_00669790 size=374 ret=4 callers=0 callees=5

undefined4 FUN_00669790(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959bdc;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (param_1 == 0) {
    iVar2 = FUN_00919723(0x8c,uVar1);
    local_8 = 0;
    if (iVar2 == 0) {
      local_48 = 0;
      local_54 = local_48;
    }
    else {
      local_54 = FUN_005e4920(0);
    }
  }
  else if (param_1 == 1) {
    iVar2 = FUN_00919723(0x34,uVar1);
    local_8 = 1;
    if (iVar2 == 0) {
      local_4c = 0;
      local_54 = local_4c;
    }
    else {
      local_54 = FUN_005e1a60();
    }
  }
  else if (param_1 == 2) {
    iVar2 = FUN_00919723(0x18,uVar1);
    local_8 = 2;
    if (iVar2 == 0) {
      local_50 = 0;
      local_54 = local_50;
    }
    else {
      local_54 = FUN_0053e060();
    }
  }
  else if (param_1 == 3) {
    iVar2 = FUN_00919723(0x18,uVar1);
    local_8 = 3;
    if (iVar2 == 0) {
      local_54 = 0;
    }
    else {
      local_54 = FUN_0053e400();
    }
  }
  else {
    local_54 = 0;
  }
  ExceptionList = local_10;
  return local_54;
}
```

### decoy: 0x00669650 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00669650: 16-byte aligned: YES
  00669650  55             PUSH EBP
  00669651  8B EC          MOV EBP,ESP
  00669653  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00669650
string literals (0):
direct callees (4):
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
  0x005E1AF0  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 0):
```

## Card B0-10-05

GOG function: CSWVirtualMachineCommands::ExecuteCommandDelayCommand  gog 0x00785950  callconv=__thiscall  param_size_bytes=8  gog_size=309  gog_retn=8
Candidate Steam address: 0x00670370 (size 309, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00670580:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00784f70 Global::ExecuteCommandCreateObject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00785a90 Global::ExecuteCommandDeleteJournalWorldAllEntries  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam ?  in candidate callees: n/a
  0x00640d50 CVirtualMachine::StackPopCommand -> Steam ?  in candidate callees: n/a
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x28*,0xfffff82f*} | Steam {0x4*,0x8*,0x28*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00670370: 16-byte aligned: YES
  00670370  55             PUSH EBP
  00670371  8B EC          MOV EBP,ESP
  00670373  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00670370
string literals (0):
direct callees (8):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x006FDB30  CVirtualMachine::StackPopCommand
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0091C860  _ftol2_sse
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x5  via EAX,EDX
  +0x4 [dec 4]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00670370 FUN_00670370 size=309 ret=8 callers=0 callees=8

undefined4 FUN_00670370(void)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined1 *puVar8;
  int extraout_var;
  undefined4 local_18;
  int local_10;
  undefined1 local_c [4];
  undefined1 local_8 [4];
  
  local_10 = 0;
  iVar2 = FUN_006fd9e0(local_8);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = extraout_var;
    iVar4 = FUN_006fdb30(&local_10);
    if (iVar4 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      if (*(int *)(iVar2 + 4) == 1) {
        uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
        puVar8 = local_c;
        uVar6 = *(undefined4 *)(iVar2 + 8);
        FUN_0051c080(uVar6,puVar8);
        cVar1 = FUN_0053dfb0(uVar6,puVar8);
        if (cVar1 == DAT_0099447f) {
          local_18 = DAT_00994484;
          if (*(int *)(iVar2 + 4) == 1) {
            local_18 = *(undefined4 *)(iVar2 + 8);
          }
          uVar6 = *(undefined4 *)(iVar2 + 8);
          uVar7 = DAT_009923f8;
          iVar4 = local_10;
          uVar3 = FUN_0091c860(local_18,uVar6,DAT_009923f8,local_10,iVar2,
                               *(undefined4 *)(DAT_00a1b4a4 + 8),uVar3);
          uVar5 = 0;
          FUN_0051c280(0,uVar3);
          FUN_0051e170(uVar5,uVar3,local_18,uVar6,uVar7,iVar4);
        }
        else if (local_10 != 0) {
          FUN_009196fd(local_10);
        }
      }
      else if (local_10 != 0) {
        FUN_009196fd(local_10);
      }
      uVar3 = 0;
    }
  }
  return uVar3;
}
```

### decoy: 0x00670580 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00670580: 16-byte aligned: YES
  00670580  55             PUSH EBP
  00670581  8B EC          MOV EBP,ESP
  00670583  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00670580
string literals (0):
direct callees (10):
  0x0051C2E0  (unnamed)
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0055F1D0  (unnamed)
  0x0051C2A0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00632A20  (unnamed)
  0x0064BB30  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,EDX
```

## Card B0-10-06

GOG function: CSWSEffectListHandler::OnRemoveLightsaberThrow  gog 0x0074bdc0  callconv=(blank)  param_size_bytes=4  gog_size=55  gog_retn=8
Candidate Steam address: 0x005a94e0 (size 55, retn 8)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a8860:STR0/CALLEE0/SIG0.077/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074b140 CSWSEffectListHandler::OnApplyLightsaberThrow -> 0x005a8860  gate_class=REVIEW
- GOG-order neighbour after:  0x0074be00 CSWSEffectListHandler::OnApplyForceResisted -> 0x005a9520  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x1128*} | Steam {0x30*,0x1128*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x005a94e0: 16-byte aligned: YES
  005a94e0  55             PUSH EBP
  005a94e1  8B EC          MOV EBP,ESP
  005a94e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a94e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x1  via EDX
  +0x1128 [dec 4392]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a94e0 FUN_005a94e0 size=55 ret=8 callers=0 callees=0

undefined4 __thiscall FUN_005a94e0(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    *(undefined4 *)(iVar1 + 0x1128) = 0;
  }
  return 1;
}
```

### decoy: 0x005a8860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a8860: 16-byte aligned: YES
  005a8860  55             PUSH EBP
  005a8861  8B EC          MOV EBP,ESP
  005a8863  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a8860
string literals (0):
direct callees (18):
  0x006BD9C0  CSWSCreatureStats::HasSpell
  0x006D0620  (unnamed)
  0x005E4EE0  CGameEffect::GetObjectID
  0x005E4E60  CGameEffect::GetInteger
  0x006C1450  (unnamed)
  0x0051C0A0  CServerExoApp::GetGameObject
  0x005142F0  (unnamed)
  0x0051C280  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E4F00  CGameEffect::SetObjectID
  0x0091C860  _ftol2_sse
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4DD0  CGameEffect::SetNumIntegers
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x006A2000  CSWRules::RollDice(n,sides)
struct offsets touched (top 10):
  +0x8 [dec 8]  x20  via EAX,ECX,EDX
  +0x4 [dec 4]  x15  via EAX,EDX
  +0x10 [dec 16]  x14  via EDX
  +0xA [dec 10]  x6  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EDX
  +0x1198 [dec 4504]  x3  via EAX,ECX,EDX
  +0x104 [dec 260]  x2  via EAX,EDX
  +0xA8 [dec 168]  x1  via EAX
  +0xAC [dec 172]  x1  via EDX
  +0xB4 [dec 180]  x1  via EAX
```

## Card B0-10-07

GOG function: CSWSPlaceable::GetEffectSpellId  gog 0x00769550  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x005ef160 (size 20, retn 0)
Candidate sources: branch+mapper:drift+0x0+name_map:high:batch5_reverse_engineering+near    mapper tag: drift

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ef180:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00769510 CSWSPlaceable::GetDialogResref -> 0x005ef120  gate_class=AUTO
- GOG-order neighbour after:  0x00769570 CSWSPlaceable::Destructor -> 0x005ef1a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3e0*} | Steam {0x3e0*}
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
prologue for 0x005ef160: 16-byte aligned: YES
  005ef160  55             PUSH EBP
  005ef161  8B EC          MOV EBP,ESP
  005ef163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005ef160
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3E0 [dec 992]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005ef160 FUN_005ef160 size=20 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_005ef160(int param_1)

{
  return *(undefined4 *)(param_1 + 0x3e0);
}
```

### decoy: 0x005ef180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ef180: 16-byte aligned: YES
  005ef180  55             PUSH EBP
  005ef181  8B EC          MOV EBP,ESP
  005ef183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005ef180
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3E0 [dec 992]  x1  via EAX
```

## Card B0-10-08

GOG function: CSWSCombatRound::ClearAllAttacks  gog 0x00774fa0  callconv=(blank)  param_size_bytes=0  gog_size=58  gog_retn=0
Candidate Steam address: 0x005908f0 (size 58, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswscombatround_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00590a50:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00774f70 CSWSCombatRound::GetAttack -> 0x005908c0  gate_class=AUTO
- GOG-order neighbour after:  0x00774fe0 CSWSCombatRound::AddCutsceneAttackAction -> 0x00590930  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00771430 CSWSCombatAttackData::ClearAttackData -> Steam ?  in candidate callees: n/a
  0x00774f70 CSWSCombatRound::GetAttack -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x5*,0x8*} | Steam {0x5*,0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005908f0: 16-byte aligned: YES
  005908f0  55             PUSH EBP
  005908f1  8B EC          MOV EBP,ESP
  005908f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005908f0
string literals (0):
direct callees (2):
  0x005908C0  CSWSCombatRound::GetAttack
  0x0058CD80  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005908f0 FUN_005908f0 size=58 ret=0 callers=5 callees=2

void FUN_005908f0(void)

{
  undefined4 extraout_var;
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 5; local_8 = local_8 + 1) {
    FUN_005908c0(local_8);
    FUN_0058cd80(extraout_var);
  }
  return;
}
```

### decoy: 0x00590a50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00590a50: 16-byte aligned: YES
  00590a50  55             PUSH EBP
  00590a51  8B EC          MOV EBP,ESP
  00590a53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00590a50
string literals (1):
  0x00995D20  at 00590cbb  'k_pend_commash'
direct callees (9):
  0x0051C0A0  CServerExoApp::GetGameObject
  0x00919723  operator new
  0x0058C7D0  (unnamed)
  0x00590270  CSWSCombatRound::AddAction
  0x0057E3D0  CSWSCreature::SetAttemptedAttackTarget
  0x0051CE30  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x006FD8D0  CVirtualMachine::RunScript
  0x00733780  CExoString::~CExoString
struct offsets touched (top 10):
  +0x10 [dec 16]  x4  via EAX,EDX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x18 [dec 24]  x3  via EDX
  +0xB04 [dec 2820]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX
  +0x48 [dec 72]  x2  via EAX,EDX
  +0x9C [dec 156]  x2  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x5C [dec 92]  x1  via ECX
```

## Card B0-10-09

GOG function: CSWSCombatRound::GetCurrentAction  gog 0x00774f30  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00590880 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswscombatround_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005908a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00774db0 CSWSCombatRound::CalculateOffHandAttacks -> 0x00590700  gate_class=AUTO
- GOG-order neighbour after:  0x00774f50 CSWSCombatRound::SetCurrentAction -> 0x005908a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xb24*} | Steam {0xb24*}
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
prologue for 0x00590880: 16-byte aligned: YES
  00590880  55             PUSH EBP
  00590881  8B EC          MOV EBP,ESP
  00590883  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00590880
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xB24 [dec 2852]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00590880 FUN_00590880 size=20 ret=0 callers=1 callees=0

undefined1 __fastcall FUN_00590880(int param_1)

{
  return *(undefined1 *)(param_1 + 0xb24);
}
```

### decoy: 0x005908a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005908a0: 16-byte aligned: YES
  005908a0  55             PUSH EBP
  005908a1  8B EC          MOV EBP,ESP
  005908a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005908a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xB24 [dec 2852]  x1  via EAX
```

## Card B0-10-10

GOG function: CSWSEffectListHandler::OnApplyForceFizzle  gog 0x0074bf20  callconv=(blank)  param_size_bytes=4  gog_size=238  gog_retn=12
Candidate Steam address: 0x005a9640 (size 238, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a9520:STR0/CALLEE0/SIG0.708/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074be00 CSWSEffectListHandler::OnApplyForceResisted -> 0x005a9520  gate_class=REVIEW
- GOG-order neighbour after:  0x0074c010 CSWSEffectListHandler::OnApplyPureGoodPowers -> 0x005a9730  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xa*} | Steam {0x4*,0x8*,0xa*}
imm sets (* = in both, 7 common): GOG {0x4*,0xc*,0x1c*,0x1e*,0x8c*,0xfc4*,0xfffffff8*} | Steam {0x4*,0xc*,0x1c*,0x1e*,0x8c*,0xfc4*,0xfffffff8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 12)
```

### prologue
```
prologue for 0x005a9640: 16-byte aligned: YES
  005a9640  55             PUSH EBP
  005a9641  8B EC          MOV EBP,ESP
  005a9643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a9640
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 3):
  +0xA [dec 10]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005a9640 FUN_005a9640 size=238 ret=12 callers=0 callees=5

undefined4 FUN_005a9640(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  int local_2c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952aeb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_2c = 0;
  }
  else {
    local_2c = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  if (local_2c != 0) {
    *(undefined2 *)(local_2c + 8) = 0x1e;
    *(ushort *)(local_2c + 0xa [dec 10]) = *(ushort *)(local_2c + 0xa [dec 10]) & 0xfff8 | (ushort)PTR_009967ac;
    FUN_005e4e80(0,0xfc4);
    FUN_005e5ea0(*(undefined4 *)(param_1 + 4));
    FUN_00544210(local_2c,param_3,0);
  }
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x005a9520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a9520: 16-byte aligned: YES
  005a9520  55             PUSH EBP
  005a9521  8B EC          MOV EBP,ESP
  005a9523  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a9520
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x005E4EE0  CGameEffect::GetObjectID
  0x005E4F00  CGameEffect::SetObjectID
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 4):
  +0xA [dec 10]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x7C [dec 124]  x1  via EAX
```

## Card B0-10-11

GOG function: ConsoleFunc::Constructor_2  gog 0x008677e0  callconv=(blank)  param_size_bytes=8  gog_size=160  gog_retn=8
Candidate Steam address: 0x00475910 (size 160, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=ConsoleFunc::IntConstructor
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00475870:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008677e0 ConsoleFunc::StringConstructor -> 0x00475870  gate_class=REVIEW
- GOG-order neighbour after:  0x00867880 ConsoleFunc::IntConstructor -> 0x00475910  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00866e80 unnamed -> Steam ?  in candidate callees: n/a
  0x00868510 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924c20 Global::strcpy -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x50*} | Steam {0x50*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x10*} | Steam {0x4*,0x8*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00475910: 16-byte aligned: YES
  00475910  55             PUSH EBP
  00475911  8B EC          MOV EBP,ESP
  00475913  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00475910
string literals (0):
direct callees (4):
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x004765F0  (unnamed)
  0x00474F10  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00475910 FUN_00475910 size=160 ret=8 callers=2 callees=4

int __thiscall FUN_00475910(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961f7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0091cff0(param_1,param_2,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  iVar1 = FUN_00919723(8);
  local_8 = 0;
  if (iVar1 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_004765f0(param_3);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x50) = local_20;
  FUN_00474f10(param_1);
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00475870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475870: 16-byte aligned: YES
  00475870  55             PUSH EBP
  00475871  8B EC          MOV EBP,ESP
  00475873  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00475870
string literals (0):
direct callees (4):
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x004765C0  (unnamed)
  0x00474F10  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

## Card B0-10-12

GOG function: CSWSCombatRound::SetDeflectArrow  gog 0x007769a0  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x005922f0 (size 25, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswscombatround_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00592280:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00776970 CSWSCombatRound::GetActualBonusEffectAttacksLimited -> 0x005922c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007769c0 CSWSCombatRound::SaveCombatRound -> 0x00592310  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xac8*} | Steam {0xac8*}
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
prologue for 0x005922f0: 16-byte aligned: YES
  005922f0  55             PUSH EBP
  005922f1  8B EC          MOV EBP,ESP
  005922f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005922f0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAC8 [dec 2760]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005922f0 FUN_005922f0 size=25 ret=4 callers=1 callees=0

void __thiscall FUN_005922f0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xac8) = param_2;
  return;
}
```

### decoy: 0x00592280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00592280: 16-byte aligned: YES
  00592280  55             PUSH EBP
  00592281  8B EC          MOV EBP,ESP
  00592283  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00592280
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAF4 [dec 2804]  x1  via EAX
```

## Card B0-10-13

GOG function: CSWSEffectListHandler::OnEffectApplied  gog 0x00737020  callconv=(blank)  param_size_bytes=12  gog_size=96  gog_retn=12
Candidate Steam address: 0x00594810 (size 1841, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyForceShield
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005947a0:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00736310 CSWSEffectListHandler::InitializeEffects -> 0x00593a30  gate_class=REVIEW
- GOG-order neighbour after:  0x00737080 CSWSEffectListHandler::OnEffectRemoved -> 0x005947a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0xa,0x30,0x110,0x130,0x1184}
imm sets (* = in both, 1 common): GOG {0xc*,0x76} | Steam {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc*,0x1e,0x6b,0x8c,...(+7)}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00594810: 16-byte aligned: YES
  00594810  55             PUSH EBP
  00594811  8B EC          MOV EBP,ESP
  00594813  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00594810
string literals (14):
  0x00996318  at 005949ad  'VisualEffectDef'
  0x00996308  at 005949be  'Appearance_01'
  0x009962F8  at 00594a47  'VisualEffect_01'
  0x009962E8  at 00594a59  'Appearance_02'
  0x009962D8  at 00594ae2  'VisualEffect_02'
  0x009962C8  at 00594af4  'Appearance_03'
  0x009962B8  at 00594b7d  'VisualEffect_03'
  0x009962A8  at 00594b8f  'Appearance_04'
  0x00996298  at 00594c27  'VisualEffect_04'
  0x0099628C  at 00594c6b  'DamageFlags'
  0x00996280  at 00594ca5  'VulnerFlags'
  0x00996274  at 00594cdf  'Resistance'
direct callees (14):
  0x0058AE90  Creature::GetServerObjectVt2c
  0x0077EB60  CSWCCreature::GetForceShield
  0x00544790  CSWSObject::RemoveEffectById
  0x005E4E60  CGameEffect::GetInteger
  0x00733700  CExoString::Constructor_5
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x110 [dec 272]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
  +0x1184 [dec 4484]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00594810 FUN_00594810 size=1841 ret=12 callers=0 callees=14

undefined4 FUN_00594810(int *param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int local_104;
  int local_100;
  char local_fc;
  char local_f8;
  char local_f4;
  char local_f0;
  undefined1 local_b8 [8];
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [11];
  char local_85;
  undefined1 local_84 [11];
  char local_79;
  undefined1 local_78 [11];
  char local_6d;
  undefined1 local_6c [11];
  char local_61;
  ushort local_60;
  int local_5c;
  undefined4 *local_58;
  int *local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined1 local_48 [8];
  undefined4 local_40;
  int local_3c;
  uint local_38;
  undefined4 local_34;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  ushort local_24;
  undefined1 local_20 [8];
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00951d12;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (((param_1 != (int *)0x0) &&
      (iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc), iVar1 != 0))
     && (param_2 != 0)) {
    local_30 = (**(code **)(*param_1 + 0x30))();
    local_24 = *(ushort *)(local_30 + 0x1184);
    iVar1 = FUN_0058ae90();
    if (iVar1 != 0) {
      FUN_0058ae90();
      iVar1 = FUN_0077eb60();
      if (iVar1 != 0) {
        local_58 = (undefined4 *)0x0;
        local_54 = (int *)(local_30 + 0x148);
        for (local_5c = 0; local_5c < *(int *)(local_30 + 0x14c); local_5c = local_5c + 1) {
          local_60 = *(ushort *)(*(int *)(*local_54 + local_5c * 4) + 8);
          if (local_60 == 0x6b) {
            local_58 = *(undefined4 **)(*local_54 + local_5c * 4);
            break;
          }
          if (0x6b < local_60) break;
        }
        FUN_00544790(*local_58,local_58[1 = byte +0x4]);
      }
    }
    uVar2 = FUN_005e4e60(0);
    FUN_00733700(uVar2);
    local_8 = 0;
    local_50 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x110);
    local_38 = 0;
    local_2c = 0;
/* ... 140 more lines truncated by --max-lines */
```

### decoy: 0x005947a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005947a0: 16-byte aligned: YES
  005947a0  55             PUSH EBP
  005947a1  8B EC          MOV EBP,ESP
  005947a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005947a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via EAX,EDX
```

## Card B0-10-14

GOG function: ConsoleFunc::Constructor_2  gog 0x008677e0  callconv=(blank)  param_size_bytes=8  gog_size=160  gog_retn=8
Candidate Steam address: 0x004757d0 (size 160, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=ConsoleFunc::NoParamConstructor
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00475870:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008677e0 ConsoleFunc::StringConstructor -> 0x00475870  gate_class=REVIEW
- GOG-order neighbour after:  0x00867880 ConsoleFunc::IntConstructor -> 0x00475910  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00866e80 unnamed -> Steam ?  in candidate callees: n/a
  0x00868510 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924c20 Global::strcpy -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x50*} | Steam {0x50*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x10*} | Steam {0x4*,0x8*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x004757d0: 16-byte aligned: YES
  004757d0  55             PUSH EBP
  004757d1  8B EC          MOV EBP,ESP
  004757d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004757d0
string literals (0):
direct callees (4):
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x00476570  (unnamed)
  0x00474F10  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004757d0 FUN_004757d0 size=160 ret=8 callers=11 callees=4

int __thiscall FUN_004757d0(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961f7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0091cff0(param_1,param_2,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  iVar1 = FUN_00919723(8);
  local_8 = 0;
  if (iVar1 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_00476570(param_3);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x50) = local_20;
  FUN_00474f10(param_1);
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00475870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475870: 16-byte aligned: YES
  00475870  55             PUSH EBP
  00475871  8B EC          MOV EBP,ESP
  00475873  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00475870
string literals (0):
direct callees (4):
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x004765C0  (unnamed)
  0x00474F10  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

## Card B0-10-15

GOG function: CSWSTrigger::GetClientTrigger  gog 0x00761e60  callconv=(blank)  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x00622bd0 (size 53, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (GetClientObject 0x54a840)
- best decoy (neighbour/other candidate) scores: 0x00622c90:STR0/CALLEE0/SIG0.036/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00761bc0 CSWSTrigger::OnEnterMine -> 0x00622930  gate_class=AUTO
- GOG-order neighbour after:  0x00761ea0 CSWSTrigger::SetCreator -> 0x00622c10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006b1520 CSWSObject::GetClientObject -> Steam 0x0054a840  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3c*} | Steam {0x3c*}
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
prologue for 0x00622bd0: 16-byte aligned: YES
  00622bd0  55             PUSH EBP
  00622bd1  8B EC          MOV EBP,ESP
  00622bd3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00622bd0
string literals (0):
direct callees (1):
  0x0054A840  CSWSObject::GetClientObject
struct offsets touched (top 1):
  +0x3C [dec 60]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00622bd0 FUN_00622bd0 size=53 ret=0 callers=1 callees=1

undefined4 FUN_00622bd0(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
  iVar1 = FUN_0054a840();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_0054a840();
    uVar3 = (**(code **)(*piVar2 + 0x3c))(piVar2,uVar3);
  }
  return uVar3;
}
```

### decoy: 0x00622c90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00622c90: 16-byte aligned: YES
  00622c90  55             PUSH EBP
  00622c91  8B EC          MOV EBP,ESP
  00622c93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00622c90
string literals (0):
direct callees (14):
  0x0053E830  CSWSObject::Constructor
  0x00735240  (unnamed)
  0x0084A640  (unnamed)
  0x0091D67E  (unnamed)
  0x0051C280  (unnamed)
  0x0051E040  CServerAIMaster::AddObject
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x00919723  operator new
  0x006288F0  (unnamed)
  0x0058C290  (unnamed)
  0x00627E50  CSWSEncounter::LoadFractionalCRData
  0x00627F60  CSWSEncounter::SetActive
  0x00543F50  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x2  via EAX
  +0x2C0 [dec 704]  x2  via EDX
  +0xAC [dec 172]  x1  via ECX
  +0xB0 [dec 176]  x1  via EAX
  +0x26C [dec 620]  x1  via ECX
  +0x27C [dec 636]  x1  via EDX
  +0x280 [dec 640]  x1  via EAX
  +0x284 [dec 644]  x1  via ECX
  +0x288 [dec 648]  x1  via EDX
  +0x290 [dec 656]  x1  via EDX
```

## Card B0-10-16

GOG function: CSWSCombatRound::SetBonusEffectAttacks  gog 0x00776930  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x00592280 (size 25, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswscombatround_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005922a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00776910 CSWSCombatRound::GetTotalAttacks -> 0x00592260  gate_class=REVIEW
- GOG-order neighbour after:  0x00776950 CSWSCombatRound::GetActualBonusEffectAttacks -> 0x005922a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xaf4*} | Steam {0xaf4*}
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
prologue for 0x00592280: 16-byte aligned: YES
  00592280  55             PUSH EBP
  00592281  8B EC          MOV EBP,ESP
  00592283  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00592280
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAF4 [dec 2804]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00592280 FUN_00592280 size=25 ret=4 callers=2 callees=0

void __thiscall FUN_00592280(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xaf4) = param_2;
  return;
}
```

### decoy: 0x005922a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005922a0: 16-byte aligned: YES
  005922a0  55             PUSH EBP
  005922a1  8B EC          MOV EBP,ESP
  005922a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005922a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAF4 [dec 2804]  x1  via EAX
```

## Card B0-10-17

GOG function: CSWSCombatRound::GetTotalAttacks  gog 0x00776910  callconv=(blank)  param_size_bytes=0  gog_size=29  gog_retn=0
Candidate Steam address: 0x00592260 (size 29, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswscombatround_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005921e0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00776890 CSWSCombatRound::UpdateAttackTargetForAllActions -> 0x005921e0  gate_class=AUTO
- GOG-order neighbour after:  0x00776930 CSWSCombatRound::SetBonusEffectAttacks -> 0x00592280  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xae0*,0xae4*} | Steam {0xae0*,0xae4*}
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
prologue for 0x00592260: 16-byte aligned: YES
  00592260  55             PUSH EBP
  00592261  8B EC          MOV EBP,ESP
  00592263  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00592260
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xAE0 [dec 2784]  x1  via EAX
  +0xAE4 [dec 2788]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00592260 FUN_00592260 size=29 ret=0 callers=5 callees=0

int __fastcall FUN_00592260(int param_1)

{
  return *(int *)(param_1 + 0xae0) + *(int *)(param_1 + 0xae4);
}
```

### decoy: 0x005921e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005921e0: 16-byte aligned: YES
  005921e0  55             PUSH EBP
  005921e1  8B EC          MOV EBP,ESP
  005921e3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005921e0
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 4):
  +0xB00 [dec 2816]  x4  via EAX,ECX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
```

## Card B0-10-18

GOG function: CSWSPlaceable::Destructor  gog 0x00769570  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x005ef1a0 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ef180:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00769550 CSWSPlaceable::GetEffectSpellId -> 0x005ef160  gate_class=REVIEW
- GOG-order neighbour after:  0x007695a0 CSWSPlaceable::Destructor_2 -> 0x005ef1d0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007695a0 CSWSPlaceable::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x005ef1a0: 16-byte aligned: YES
  005ef1a0  55             PUSH EBP
  005ef1a1  8B EC          MOV EBP,ESP
  005ef1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005ef1a0
string literals (0):
direct callees (2):
  0x005EF1D0  CSWSPlaceable::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005ef1a0 FUN_005ef1a0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_005ef1a0(undefined4 param_1,uint param_2)

{
  FUN_005ef1d0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x005ef180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ef180: 16-byte aligned: YES
  005ef180  55             PUSH EBP
  005ef181  8B EC          MOV EBP,ESP
  005ef183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005ef180
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3E0 [dec 992]  x1  via EAX
```

## Card B0-10-19

GOG function: CSWSItemPropertyHandler::ApplyRapidShotBonus  gog 0x007c9f50  callconv=__thiscall  param_size_bytes=4  gog_size=34  gog_retn=20
Candidate Steam address: 0x005b2b10 (size 34, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplySniperBonus
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2b40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9f20 CSWSItemPropertyHandler::ApplySniperBonus -> 0x005b2b10  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9f80 CSWSItemPropertyHandler::ApplyMaxDexBonus -> 0x005b2b70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2e4} | Steam {0x2e8}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005b2b10: 16-byte aligned: YES
  005b2b10  55             PUSH EBP
  005b2b11  8B EC          MOV EBP,ESP
  005b2b13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2b10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E8 [dec 744]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2b10 FUN_005b2b10 size=34 ret=20 callers=0 callees=0

undefined4 FUN_005b2b10(int param_1)

{
  if (param_1 != 0) {
    *(undefined4 *)(param_1 + 0x2e8) = 1;
  }
  return 0;
}
```

### decoy: 0x005b2b40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2b40: 16-byte aligned: YES
  005b2b40  55             PUSH EBP
  005b2b41  8B EC          MOV EBP,ESP
  005b2b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2b40
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E4 [dec 740]  x1  via EAX
```

## Card B0-10-20

GOG function: CSWSCombatRound::GetOffHandAttack  gog 0x00774c30  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00590580 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswscombatround_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00590560:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00774c10 CSWSCombatRound::SetOffHandAttack -> 0x00590560  gate_class=REVIEW
- GOG-order neighbour after:  0x00774ca0 CSWSCombatRound::CalculateOnHandAttacks -> 0x005905f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xac0*} | Steam {0xac0*}
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
prologue for 0x00590580: 16-byte aligned: YES
  00590580  55             PUSH EBP
  00590581  8B EC          MOV EBP,ESP
  00590583  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00590580
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAC0 [dec 2752]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00590580 FUN_00590580 size=20 ret=0 callers=4 callees=0

undefined4 __fastcall FUN_00590580(int param_1)

{
  return *(undefined4 *)(param_1 + 0xac0);
}
```

### decoy: 0x00590560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00590560: 16-byte aligned: YES
  00590560  55             PUSH EBP
  00590561  8B EC          MOV EBP,ESP
  00590563  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00590560
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAC0 [dec 2752]  x1  via EAX
```

## Card B0-10-21

GOG function: CSWSEffectListHandler::OnApplyDisappear  gog 0x00748db0  callconv=(blank)  param_size_bytes=8  gog_size=163  gog_retn=12
Candidate Steam address: 0x005a6570 (size 72, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyAppear
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.38 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005a64c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00748b70 CSWSEffectListHandler::OnRemoveDisappearAppear -> 0x005a6280  gate_class=REVIEW
- GOG-order neighbour after:  0x00748e60 CSWSEffectListHandler::OnApplyAppear -> 0x005a6570  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x006962c0 CSWSCreature::AddDisappearActions -> Steam ?  in candidate callees: n/a
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4,0x8*,0x18,0x30*} | Steam {0x8*,0x30*}
imm sets (* = in both, 1 common): GOG {0xc*,0x14,0x7d0} | Steam {0x8,0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a6570: 16-byte aligned: YES
  005a6570  55             PUSH EBP
  005a6571  8B EC          MOV EBP,ESP
  005a6573  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a6570
string literals (0):
direct callees (2):
  0x00541080  CSWSObject::ClearAllActions
  0x0057D700  CSWSCreature::AddAppearActions
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a6570 FUN_005a6570 size=72 ret=12 callers=0 callees=2

undefined4 __thiscall FUN_005a6570(undefined4 param_1,int *param_2)

{
  undefined4 uVar1;
  
  if ((char)param_2[2 = byte +0x8] == DAT_00994474) {
    uVar1 = (**(code **)(*param_2 + 0x30))(param_1);
    FUN_00541080(1);
    FUN_0057d700(param_1,uVar1);
  }
  return 1;
}
```

### decoy: 0x005a64c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a64c0: 16-byte aligned: YES
  005a64c0  55             PUSH EBP
  005a64c1  8B EC          MOV EBP,ESP
  005a64c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a64c0
string literals (0):
direct callees (4):
  0x00541080  CSWSObject::ClearAllActions
  0x0057D790  CSWSCreature::AddDisappearActions
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 4):
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-10-22

GOG function: CSWSPlaceable::GetFirstName  gog 0x0075d320  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x0061e090 (size 19, retn 0)
Candidate sources: branch+name_map:high:batch5_reverse_engineering    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0061dcf0:STR0/CALLEE0/SIG0.026/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0075cf80 CSWSTrigger::Constructor -> 0x0061dcf0  gate_class=AUTO
- GOG-order neighbour after:  0x0075d340 CSWSTrigger::Destructor -> 0x0061e0b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x268*} | Steam {0x268*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0061e090: 16-byte aligned: YES
  0061e090  55             PUSH EBP
  0061e091  8B EC          MOV EBP,ESP
  0061e093  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0061e090
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0061e090 FUN_0061e090 size=19 ret=0 callers=0 callees=0

int __fastcall FUN_0061e090(int param_1)

{
  return param_1 + 0x268;
}
```

### decoy: 0x0061dcf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0061dcf0: 16-byte aligned: YES
  0061dcf0  55             PUSH EBP
  0061dcf1  8B EC          MOV EBP,ESP
  0061dcf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0061dcf0
string literals (2):
  0x00995574  at 0061ddfa  'PrimaryRange'
  0x00987564  at 0061dfb8  'default'
direct callees (12):
  0x0053E830  CSWSObject::Constructor
  0x00735240  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x0091D67E  (unnamed)
  0x0084A640  (unnamed)
  0x0051C280  (unnamed)
  0x0051E040  CServerAIMaster::AddObject
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x00541390  CSWSObject::SetAnimation
struct offsets touched (top 10):
  +0x8 [dec 8]  x1  via ECX
  +0xBC [dec 188]  x1  via ECX
  +0xD8 [dec 216]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
  +0x174 [dec 372]  x1  via EDX
  +0x2C4 [dec 708]  x1  via EDX
  +0x2C8 [dec 712]  x1  via EAX
  +0x2CC [dec 716]  x1  via ECX
  +0x2D0 [dec 720]  x1  via EDX
  +0x2D4 [dec 724]  x1  via EAX
```

## Card B0-10-23

GOG function: CSWVirtualMachineCommands::ExecuteCommandCutsceneAttack  gog 0x00783180  callconv=__thiscall  param_size_bytes=8  gog_size=212  gog_retn=8
Candidate Steam address: 0x0066dba0 (size 212, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0066dc80:STR0/CALLEE0/SIG0.7/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007830d0 CSWVirtualMachineCommands::ExecuteCommandAttack -> 0x0066daf0  gate_class=REVIEW
- GOG-order neighbour after:  0x00783260 CSWVirtualMachineCommands::ExecuteCommandCutsceneMove -> 0x0066dc80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640d10 CVirtualMachine::StackPopObject -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x00681000 CSWSCreature::AddCutsceneAttackActions -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x1c*,0x7f000000*,0xfffff82f*} | Steam {0x8*,0x1c*,0x7f000000*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0066dba0: 16-byte aligned: YES
  0066dba0  55             PUSH EBP
  0066dba1  8B EC          MOV EBP,ESP
  0066dba3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0066dba0
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005683D0  CSWSCreature::AddCutsceneAttackActions
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0066dba0 FUN_0066dba0 size=212 ret=8 callers=0 callees=4

undefined4 __fastcall FUN_0066dba0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_10 = 0x7f000000;
  local_c = 0;
  local_18 = 0;
  local_8 = 0;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9a0(&local_c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_006fd9a0(&local_18);
      if (iVar1 == 0) {
        uVar2 = 0xfffff82f;
      }
      else {
        iVar1 = FUN_006fd9a0(&local_8);
        if (iVar1 == 0) {
          uVar2 = 0xfffff82f;
        }
        else {
          local_14 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
          if (local_14 != 0) {
            FUN_005683d0(local_10,local_c,local_18,local_8);
          }
          uVar2 = 0;
        }
      }
    }
  }
  return uVar2;
}
```

### decoy: 0x0066dc80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066dc80: 16-byte aligned: YES
  0066dc80  55             PUSH EBP
  0066dc81  8B EC          MOV EBP,ESP
  0066dc83  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066dc80
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDA20  CVirtualMachine::StackPopVector
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00568480  CSWSCreature::AddCutsceneMoveActions
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card B0-10-24

GOG function: CSWSEffectListHandler::OnApplyDisappear  gog 0x00748db0  callconv=(blank)  param_size_bytes=8  gog_size=163  gog_retn=12
Candidate Steam address: 0x005a60b0 (size 463, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyDisappearAppear
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.39 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005a6280:STR0/CALLEE0/SIG0.214/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00748b70 CSWSEffectListHandler::OnRemoveDisappearAppear -> 0x005a6280  gate_class=REVIEW
- GOG-order neighbour after:  0x00748e60 CSWSEffectListHandler::OnApplyAppear -> 0x005a6570  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x006962c0 CSWSCreature::AddDisappearActions -> Steam ?  in candidate callees: n/a
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x18*,0x30*} | Steam {0x4*,0x8*,0x18*,0x30*,0x90,0x350,0x360,0x10dc}
imm sets (* = in both, 2 common): GOG {0xc*,0x14,0x7d0*} | Steam {0x2,0xc*,0x50,0x94,0x354,0x7d0*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a60b0: 16-byte aligned: YES
  005a60b0  55             PUSH EBP
  005a60b1  8B EC          MOV EBP,ESP
  005a60b3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a60b0
string literals (0):
direct callees (10):
  0x005E4EA0  CGameEffect::GetFloat
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C120  (unnamed)
  0x00541080  CSWSObject::ClearAllActions
  0x005864D0  CSWSCreature::PacifyCreature
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0057D790  CSWSCreature::AddDisappearActions
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 8):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x10DC [dec 4316]  x2  via EAX,ECX
  +0x18 [dec 24]  x1  via EAX
  +0x90 [dec 144]  x1  via EAX
  +0x350 [dec 848]  x1  via ECX
  +0x360 [dec 864]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a60b0 FUN_005a60b0 size=463 ret=12 callers=0 callees=10

undefined4 FUN_005a60b0(int *param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 extraout_var;
  
  FUN_005e4ea0(0);
  uVar3 = extraout_var;
  FUN_005e4ea0(1);
  FUN_005e4ea0(2);
  uVar1 = FUN_005e4ee0(0);
  iVar2 = FUN_0051c120(uVar1);
  if (iVar2 == 0) {
    uVar3 = 1;
  }
  else {
    if ((char)param_1[2 = byte +0x8] == DAT_00994474) {
      iVar2 = (**(code **)(*param_1 + 0x30))();
      FUN_00541080(1);
      FUN_005864d0();
      *(undefined4 *)(iVar2 + 0x350) = uVar1;
      iVar4 = (**(code **)(*param_1 + 0x30))();
      *(undefined4 *)(iVar2 + 0x354) = *(undefined4 *)(iVar4 + 0x94);
      *(undefined4 *)(iVar2 + 0x358) = *(undefined4 *)(iVar4 + 0x98);
      *(undefined4 *)(iVar2 + 0x35c) = *(undefined4 *)(iVar4 + 0x9c);
      *(undefined4 *)(iVar2 + 0x360) = 0;
      FUN_0058fcd0(0,1);
      FUN_0058fc60(1,param_1[1 = byte +0x4]);
      FUN_0057d790(1);
    }
    else {
      if ((char)param_1[2 = byte +0x8] != DAT_00994478) {
        return 1;
      }
      iVar2 = param_1[1 = byte +0x4];
      uVar1 = *(undefined4 *)(param_2 + 0x18);
      uVar8 = 0;
      uVar6 = 2000;
      uVar5 = 0;
      uVar7 = DAT_00992404;
      FUN_0051c280(0,2000,uVar1,iVar2,DAT_00992404,0,uVar3,*(undefined4 *)(DAT_00a1b4a4 + 8));
      FUN_0051e170(uVar5,uVar6,uVar1,iVar2,uVar7,uVar8);
    }
    uVar3 = 0;
  }
  return uVar3;
}
```

### decoy: 0x005a6280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a6280: 16-byte aligned: YES
  005a6280  55             PUSH EBP
  005a6281  8B EC          MOV EBP,ESP
  005a6283  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a6280
string literals (0):
direct callees (12):
  0x005E4EA0  CGameEffect::GetFloat
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C120  (unnamed)
  0x00541080  CSWSObject::ClearAllActions
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
  0x0057D700  CSWSCreature::AddAppearActions
  0x0054BDC0  CSWSArea::ComputeSafeLocation
  0x0054B130  CSWSArea::ComputeHeight
  0x00568BD0  CSWSCreature::AddToArea
  0x005E4EC0  CGameEffect::SetFloat
  0x005F2A80  CSWSPlaceable::AddToArea
struct offsets touched (top 5):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x10DC [dec 4316]  x3  via EAX,ECX,EDX
  +0x30 [dec 48]  x1  via EDX
  +0x48 [dec 72]  x1  via EDX
  +0x380 [dec 896]  x1  via ECX
```

## Card B0-10-25

GOG function: CSWSDoor::GetIsLinked  gog 0x00768410  callconv=(blank)  param_size_bytes=0  gog_size=50  gog_retn=0
Candidate Steam address: 0x0061d000 (size 50, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (GetLinkedObject 0x61d620)
- best decoy (neighbour/other candidate) scores: 0x0061cf70:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00768380 CSWSDoor::GetDisarmDC -> 0x0061cf70  gate_class=AUTO
- GOG-order neighbour after:  0x00768450 CSWSDoor::RemoveFromArea -> 0x0061d040  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3d4*} | Steam {0x3d4*}
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
prologue for 0x0061d000: 16-byte aligned: YES
  0061d000  55             PUSH EBP
  0061d001  8B EC          MOV EBP,ESP
  0061d003  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0061d000
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3D4 [dec 980]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0061d000 FUN_0061d000 size=50 ret=0 callers=9 callees=0

undefined4 __fastcall FUN_0061d000(int param_1)

{
  undefined4 uVar1;
  
  if ((*(char *)(param_1 + 0x3d4) == '\x01') || (*(char *)(param_1 + 0x3d4) == '\x02')) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x0061cf70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0061cf70: 16-byte aligned: YES
  0061cf70  55             PUSH EBP
  0061cf71  8B EC          MOV EBP,ESP
  0061cf73  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0061cf70
string literals (0):
direct callees (1):
  0x0072FB00  OptionsHolder::GetGameOptions (returns *(*(this+4)+4); +8 bit0 = Auto Level Up NPCs)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x33C [dec 828]  x1  via EAX
```

## Card B0-10-26

GOG function: CSWVirtualMachineCommands::ExecuteCommandRandom  gog 0x007a4c20  callconv=(blank)  param_size_bytes=0  gog_size=109  gog_retn=8
Candidate Steam address: 0x0068f5d0 (size 109, retn 8)
Candidate sources: branch+name_map:high:batch6_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0068f560:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007a4bb0 Global::ExecuteCommandPutDownItem  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007a4c90 Global::ExecuteCommandRandomName  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam ?  in candidate callees: n/a
  0x00925936 Global::rand -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0xc*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0068f5d0: 16-byte aligned: YES
  0068f5d0  55             PUSH EBP
  0068f5d1  8B EC          MOV EBP,ESP
  0068f5d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068f5d0
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0091EB8B  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0068f5d0 FUN_0068f5d0 size=109 ret=8 callers=0 callees=3

undefined4 FUN_0068f5d0(void)

{
  int iVar1;
  undefined4 uVar2;
  int local_c;
  int local_8;
  
  local_c = 0;
  iVar1 = FUN_006fd9a0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if (local_8 != 0) {
      local_c = _rand [GHIDRA-AUTO-LABEL: unreliable]();
      local_c = local_c % local_8;
      if (local_8 < 0) {
        local_c = -local_c;
      }
    }
    iVar1 = FUN_006fd9c0(local_c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0068f560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068f560: 16-byte aligned: YES
  0068f560  55             PUSH EBP
  0068f561  8B EC          MOV EBP,ESP
  0068f563  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068f560
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00567F00  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

## Card B0-10-27

GOG function: CSWSEffectListHandler::OnApplyPureGoodPowers  gog 0x0074c010  callconv=(blank)  param_size_bytes=8  gog_size=1136  gog_retn=12
Candidate Steam address: 0x005a9730 (size 1136, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (SetCurrentHitPoints 0x54a610); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a9ba0:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074bf20 CSWSEffectListHandler::OnApplyForceFizzle -> 0x005a9640  gate_class=REVIEW
- GOG-order neighbour after:  0x0074c480 CSWSEffectListHandler::OnApplyPureEvilPowers -> 0x005a9ba0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> Steam ?  in candidate callees: n/a
  0x006977d0 CSWSCreature::GetMaxForcePoints -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x006b12f0 CSWSObject::SetCurrentHitPoints -> Steam 0x0054a610  in candidate callees: yes
  0x00734780 CGameEffect::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 11 common): GOG {0x4*,0x8*,0x30*,0x38*,0x8d*,0x98*,0x9c*,0xab*,0x130*,0x132*,0x1198*} | Steam {0x4*,0x8*,0x30*,0x38*,0x8d*,0x98*,0x9c*,0xab*,0x130*,0x132*,0x1198*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0xc*,0x24*,0x28*,0x3c*,0x43*,0x70*,0x8c*} | Steam {0x2*,0x3*,0x4*,0xc*,0x24*,0x28*,0x3c*,0x43*,0x70*,0x8c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a9730: 16-byte aligned: YES
  005a9730  55             PUSH EBP
  005a9731  8B EC          MOV EBP,ESP
  005a9733  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a9730
string literals (0):
direct callees (7):
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x0057ECA0  CSWSCreature::GetMaxForcePoints
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x0054A610  CSWSObject::SetCurrentHitPoints
struct offsets touched (top 10):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x130 [dec 304]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x98 [dec 152]  x2  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x38 [dec 56]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x132 [dec 306]  x1  via EDX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a9730 FUN_005a9730 size=1136 ret=12 callers=0 callees=7

/* WARNING: Removing unreachable block (ram,0x005a9931) */

undefined4 FUN_005a9730(int *param_1,undefined4 param_2)

{
  bool bVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  int *piVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  int local_80;
  int local_7c;
  int local_78;
  int local_70;
  char local_25;
  byte local_15;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952b3c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  piVar5 = (int *)(**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (piVar5 == (int *)0x0) {
    uVar6 = 1;
  }
  else {
    iVar8 = piVar5[0x466 = byte +0x1198];
    if (iVar8 == 0) {
      uVar6 = 1;
    }
    else {
      bVar1 = false;
      iVar7 = FUN_0051c600(piVar5[1 = byte +0x4]);
      if ((iVar7 != 0) && (*(int *)(iVar7 + 0x38) == piVar5[1 = byte +0x4])) {
        bVar1 = true;
      }
      if (bVar1) {
        for (local_15 = 0; local_15 < *(byte *)(iVar8 + 0x8d); local_15 = local_15 + 1) {
          if (local_15 < *(byte *)(iVar8 + 0x8d)) {
            local_25 = *(char *)(iVar8 + 0xab + (uint)local_15 * 0x28);
          }
          else {
            local_25 = -1;
          }
          if (((local_25 == DAT_0099b19c) || (local_25 == DAT_0099b1a4)) ||
             (local_25 == DAT_0099b1a7)) {
            sVar2 = FUN_0057eca0();
            iVar7 = FUN_00919723(0x8c);
            local_8 = 0;
            if (iVar7 == 0) {
              local_70 = 0;
            }
            else {
              local_70 = FUN_005e4ac0(param_2);
            }
            local_8 = 0xffffffff;
            *(undefined2 *)(local_70 + 8) = 0x24;
            FUN_005e4e80(0,4);
            FUN_005e4e80(1,3);
            FUN_00544210(local_70,0,0);
            sVar3 = FUN_0057eca0();
            *(short *)(iVar8 + 0x130) = *(short *)(iVar8 + 0x130) + (sVar3 - sVar2);
          }
          else if (((local_25 == DAT_0099b19b) || (local_25 == DAT_0099b1a3)) ||
                  (local_25 == DAT_0099b1a6)) {
            iVar7 = FUN_00919723(0x8c);
            local_8 = 1;
            if (iVar7 == 0) {
              local_78 = 0;
            }
            else {
              local_78 = FUN_005e4ac0(param_2);
            }
/* ... 46 more lines truncated by --max-lines */
```

### decoy: 0x005a9ba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a9ba0: 16-byte aligned: YES
  005a9ba0  55             PUSH EBP
  005a9ba1  8B EC          MOV EBP,ESP
  005a9ba3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a9ba0
string literals (0):
direct callees (5):
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 9):
  +0x8 [dec 8]  x5  via EDX
  +0x130 [dec 304]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x122 [dec 290]  x2  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x38 [dec 56]  x1  via EDX
  +0x132 [dec 306]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-10-28

GOG function: CSWVirtualMachineCommands::ExecuteCommandFloatToInt  gog 0x0078ebd0  callconv=__thiscall  param_size_bytes=8  gog_size=80  gog_retn=8
Candidate Steam address: 0x006795f0 (size 80, retn 8)
Candidate sources: branch+mapper:seeded+name_map:high:batch6_reverse_engineering+near    mapper tag: seeded

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00679640:STR0/CALLEE0/SIG0.15/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078e8f0 Global::ExecuteCommandFloatingText  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0078ec20 Global::ExecuteCommandFloatToString  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam ?  in candidate callees: n/a
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0xc*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006795f0: 16-byte aligned: YES
  006795f0  55             PUSH EBP
  006795f1  8B EC          MOV EBP,ESP
  006795f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006795f0
string literals (0):
direct callees (3):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0091C860  _ftol2_sse
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006795f0 FUN_006795f0 size=80 ret=8 callers=0 callees=3

undefined4 FUN_006795f0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined1 local_8 [4];
  
  iVar1 = FUN_006fd9e0(local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = FUN_0091c860(extraout_var);
    iVar1 = FUN_006fd9c0(uVar2);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00679640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00679640: 16-byte aligned: YES
  00679640  55             PUSH EBP
  00679641  8B EC          MOV EBP,ESP
  00679643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00679640
string literals (2):
  0x00998CA0  at 00679710  '%%%2d.%02df\\n'
  0x00998C90  at 0067972b  '%%%1d.%02df\\n'
direct callees (8):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733540  CExoString::CExoString(empty)
  0x00734270  CExoString::Format
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FDA90  CVirtualMachine::StackPushString
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B0-10-29

GOG function: CSWSItemPropertyHandler::Constructor  gog 0x007c1880  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x005aa470 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005aa3c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c17d0 CSWSItemPropertyHandler::Destructor_2 -> 0x005aa3c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c18a0 CSWSItemPropertyHandler::InitializeItemProperties -> 0x005aa490  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x005aa470: 16-byte aligned: YES
  005aa470  55             PUSH EBP
  005aa471  8B EC          MOV EBP,ESP
  005aa473  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005aa470
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005aa470 FUN_005aa470 size=20 ret=0 callers=1 callees=0

void __fastcall FUN_005aa470(undefined4 *param_1)

{
  *param_1 = CItemPropertyApplierRemover::vftable;
  return;
}
```

### decoy: 0x005aa3c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aa3c0: 16-byte aligned: YES
  005aa3c0  55             PUSH EBP
  005aa3c1  8B EC          MOV EBP,ESP
  005aa3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005aa3c0
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
```

## Card B0-10-30

GOG function: CSWSEffectListHandler::OnApplyForceResisted  gog 0x0074be00  callconv=(blank)  param_size_bytes=12  gog_size=281  gog_retn=12
Candidate Steam address: 0x005a9520 (size 281, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a9640:STR0/CALLEE0/SIG0.708/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0074bdc0 CSWSEffectListHandler::OnRemoveLightsaberThrow -> 0x005a94e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0074bf20 CSWSEffectListHandler::OnApplyForceFizzle -> 0x005a9640  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00734ba0 CGameEffect::GetObjectID -> Steam ?  in candidate callees: n/a
  0x00734bc0 CGameEffect::SetObjectID -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xa*,0x7c*} | Steam {0x4*,0x8*,0xa*,0x7c*}
imm sets (* = in both, 8 common): GOG {0x4*,0xc*,0x1c*,0x1e*,0x8c*,0xfc5*,0x27a1*,0xfffffff8*} | Steam {0x4*,0xc*,0x1c*,0x1e*,0x8c*,0xfc5*,0x27a1*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x005a9520: 16-byte aligned: YES
  005a9520  55             PUSH EBP
  005a9521  8B EC          MOV EBP,ESP
  005a9523  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a9520
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x005E4EE0  CGameEffect::GetObjectID
  0x005E4F00  CGameEffect::SetObjectID
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 4):
  +0xA [dec 10]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x7C [dec 124]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005a9520 FUN_005a9520 size=281 ret=12 callers=0 callees=7

undefined4 FUN_005a9520(int *param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  int local_2c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952aeb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_2c = 0;
  }
  else {
    local_2c = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  if (local_2c != 0) {
    *(undefined2 *)(local_2c + 8) = 0x1e;
    *(ushort *)(local_2c + 0xa [dec 10]) = *(ushort *)(local_2c + 0xa [dec 10]) & 0xfff8 | (ushort)PTR_009967ac;
    FUN_005e5ea0(param_1[1 = byte +0x4]);
    FUN_005e4e80(0,0xfc5);
    uVar2 = FUN_005e4ee0(0);
    FUN_005e4f00(0,uVar2);
    FUN_00544210(local_2c,param_3,0);
    (**(code **)(*param_1 + 0x7c))(0x27a1);
  }
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x005a9640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a9640: 16-byte aligned: YES
  005a9640  55             PUSH EBP
  005a9641  8B EC          MOV EBP,ESP
  005a9643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a9640
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 3):
  +0xA [dec 10]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
```

## Card B0-10-31

GOG function: CSWVirtualMachineCommands::ExecuteCommandAttack  gog 0x007830d0  callconv=(blank)  param_size_bytes=0  gog_size=166  gog_retn=8
Candidate Steam address: 0x0066daf0 (size 166, retn 8)
Candidate sources: branch+name_map:high:batch6_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (AddAttackActions 0x568540); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0066dc80:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00782fd0 CSWVirtualMachineCommands::ExecuteCommandAssignCommand -> 0x0066d9f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00783180 CSWVirtualMachineCommands::ExecuteCommandCutsceneAttack -> 0x0066dba0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640d10 CVirtualMachine::StackPopObject -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 7 common): GOG {0x2*,0x4*,0x8*,0x14*,0x5dc*,0x2719*,0xfffff82f*} | Steam {0x2*,0x4*,0x8*,0x14*,0x5dc*,0x2719*,0xfffff82f*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0066daf0: 16-byte aligned: YES
  0066daf0  55             PUSH EBP
  0066daf1  8B EC          MOV EBP,ESP
  0066daf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0066daf0
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00568540  CSWSCreature::AddAttackActions
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0066daf0 FUN_0066daf0 size=166 ret=8 callers=0 callees=4

undefined4 __thiscall FUN_0066daf0(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  
  local_10 = 0;
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if ((1 < param_3) && (iVar1 = FUN_006fd9a0(&local_10), iVar1 == 0)) {
      return 0xfffff82f;
    }
    local_c = FUN_0051c100(*(undefined4 *)(param_1 + 8));
    if (local_c != 0) {
      FUN_00568540(local_8,0,0,local_10,0,0,0,1,0x2719,0x5dc,1,0,0,4,0);
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0066dc80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066dc80: 16-byte aligned: YES
  0066dc80  55             PUSH EBP
  0066dc81  8B EC          MOV EBP,ESP
  0066dc83  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066dc80
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDA20  CVirtualMachine::StackPopVector
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00568480  CSWSCreature::AddCutsceneMoveActions
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card B0-10-32

GOG function: CSWSEffectListHandler::OnApplyEffectImmunity  gog 0x0074b110  callconv=(blank)  param_size_bytes=4  gog_size=33  gog_retn=12
Candidate Steam address: 0x005a87e0 (size 52, retn 12)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyBodyFuel
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.75
- best decoy (neighbour/other candidate) scores: 0x005a8820:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074b0d0 CSWSEffectListHandler::OnRemoveBodyFuel -> 0x005a8820  gate_class=REVIEW
- GOG-order neighbour after:  0x0074b140 CSWSEffectListHandler::OnApplyLightsaberThrow -> 0x005a8860  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x30*} | Steam {0x30*,0x11c4}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 12)
```

### prologue
```
prologue for 0x005a87e0: 16-byte aligned: YES
  005a87e0  55             PUSH EBP
  005a87e1  8B EC          MOV EBP,ESP
  005a87e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a87e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x1  via EDX
  +0x11C4 [dec 4548]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a87e0 FUN_005a87e0 size=52 ret=12 callers=0 callees=0

undefined4 __thiscall FUN_005a87e0(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    *(undefined4 *)(iVar1 + 0x11c4) = 1;
  }
  return 0;
}
```

### decoy: 0x005a8820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a8820: 16-byte aligned: YES
  005a8820  55             PUSH EBP
  005a8821  8B EC          MOV EBP,ESP
  005a8823  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a8820
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x1  via EDX
  +0x11C4 [dec 4548]  x1  via ECX
```

## Card B0-10-33

GOG function: CSWSDoor::Destructor  gog 0x00762410  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00616f60 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00617190:STR0/CALLEE0/SIG0.036/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007623f0 CSWSDoor::GetFirstName -> 0x00616f40  gate_class=REVIEW
- GOG-order neighbour after:  0x00762440 CSWSDoor::Destructor_2 -> 0x00616f90  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00762440 CSWSDoor::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x00616f60: 16-byte aligned: YES
  00616f60  55             PUSH EBP
  00616f61  8B EC          MOV EBP,ESP
  00616f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00616f60
string literals (0):
direct callees (2):
  0x00616F90  CSWSDoor::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00616f60 FUN_00616f60 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00616f60(undefined4 param_1,uint param_2)

{
  FUN_00616f90();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x00617190 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00617190: 16-byte aligned: YES
  00617190  55             PUSH EBP
  00617191  8B EC          MOV EBP,ESP
  00617193  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00617190
string literals (0):
direct callees (8):
  0x0061D040  CSWSDoor::RemoveFromArea
  0x00545420  CSWSObject::SetArea
  0x00543F50  (unnamed)
  0x00522BF0  CSWSArea::AddObjectToArea
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00617320  CSWSDoor::GetMeshBoundingBox
  0x005E8120  Global::AddStaticBoundingBox
  0x006178D0  CSWSDoor::GetActionPoint
struct offsets touched (top 3):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x3D0 [dec 976]  x1  via EAX
```

## Card B0-10-34

GOG function: CSWVirtualMachineCommands::ExecuteCommandActionDoCommand  gog 0x0077efb0  callconv=(blank)  param_size_bytes=0  gog_size=118  gog_retn=8
Candidate Steam address: 0x006699d0 (size 118, retn 8)
Candidate sources: branch+name_map:high:batch6_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00669a50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0077eef0 Global::ExecuteCommandActionBarkString  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0077f030 CSWVirtualMachineCommands::ExecuteCommandActionEquipMostDamagingMelee -> 0x00669a50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640d50 CVirtualMachine::StackPopCommand -> Steam ?  in candidate callees: n/a
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x006bd530 CSWSObject::AddDoCommandAction -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff82f*} | Steam {0x8*,0x14*,0xfffff82f*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x006699d0: 16-byte aligned: YES
  006699d0  55             PUSH EBP
  006699d1  8B EC          MOV EBP,ESP
  006699d3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006699d0
string literals (0):
direct callees (4):
  0x006FDB30  CVirtualMachine::StackPopCommand
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E1340  CSWSObject::AddDoCommandAction
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006699d0 FUN_006699d0 size=118 ret=8 callers=0 callees=4

undefined4 FUN_006699d0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 *puVar3;
  int extraout_var;
  undefined4 local_10;
  undefined1 local_c [7];
  char local_5;
  
  local_10 = 0;
  iVar1 = FUN_006fdb30(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    puVar3 = local_c;
    uVar2 = *(undefined4 *)(extraout_var + 8);
    FUN_0051c080(uVar2,puVar3,extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 8));
    local_5 = FUN_0053dfb0(uVar2,puVar3);
    if (local_5 == DAT_0099447f) {
      FUN_005e1340(local_10);
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00669a50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00669a50: 16-byte aligned: YES
  00669a50  55             PUSH EBP
  00669a51  8B EC          MOV EBP,ESP
  00669a53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00669a50
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00580330  CSWSCreature::EquipMostDamagingMeleeWeapon
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX,EDX
```

## Card B0-10-35

GOG function: CSWSPlaceable::GetClientPlaceable  gog 0x00770e40  callconv=(blank)  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x005f6a70 (size 53, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (GetClientObject 0x54a840)
- best decoy (neighbour/other candidate) scores: 0x005f6ab0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00770dd0 CSWSPlaceable::OnEndDialog -> 0x005f6a00  gate_class=AUTO
- GOG-order neighbour after:  0x00770e80 CSWSCombatRoundAction::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006b1520 CSWSObject::GetClientObject -> Steam 0x0054a840  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4c*} | Steam {0x4c*}
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
prologue for 0x005f6a70: 16-byte aligned: YES
  005f6a70  55             PUSH EBP
  005f6a71  8B EC          MOV EBP,ESP
  005f6a73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005f6a70
string literals (0):
direct callees (1):
  0x0054A840  CSWSObject::GetClientObject
struct offsets touched (top 1):
  +0x4C [dec 76]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005f6a70 FUN_005f6a70 size=53 ret=0 callers=1 callees=1

undefined4 FUN_005f6a70(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
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

### decoy: 0x005f6ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005f6ab0: 16-byte aligned: YES
  005f6ab0  55             PUSH EBP
  005f6ab1  8B EC          MOV EBP,ESP
  005f6ab3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005f6ab0
string literals (0):
direct callees (0):
struct offsets touched (top 7):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via ECX
  +0x18 [dec 24]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via ECX
```

## Card B0-10-36

GOG function: CSWSPlaceable::DoDamage  gog 0x00768620  callconv=(blank)  param_size_bytes=4  gog_size=47  gog_retn=4
Candidate Steam address: 0x005ee290 (size 177, retn 4)
Candidate sources: branch+mapper:drift+0x40+name_map:high:batch5_reverse_engineering+near    mapper tag: drift

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ee480:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00768520 CSWSDoor::NoNonWalkPolysInDoor -> 0x0061d110  gate_class=AUTO
- GOG-order neighbour after:  0x00768650 CSWSDoor::GetNearestActionPoint -> 0x0061d210  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006a80e0 CSWSObject::DoDamage -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xfc} | Steam {0x4,0x8,0x4c,0x50,0x54}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*,0xc,0x20,0x2c,0x38}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005ee290: 16-byte aligned: YES
  005ee290  55             PUSH EBP
  005ee291  8B EC          MOV EBP,ESP
  005ee293  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005ee290
string literals (0):
direct callees (1):
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 5):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x54 [dec 84]  x2  via EAX,EDX
  +0x4C [dec 76]  x1  via EAX
  +0x50 [dec 80]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005ee290 FUN_005ee290 size=177 ret=4 callers=1 callees=1

void __thiscall FUN_005ee290(int param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined1 local_20 [12];
  undefined1 local_14 [12];
  int local_8;
  
  if (*(int *)(param_1 + 0x4c) == 0) {
    *(undefined4 *)(param_1 + 0x20) = *param_2;
    *(undefined4 *)(param_1 + 0x24) = param_2[1 = byte +0x4];
    *(undefined4 *)(param_1 + 0x28) = param_2[2 = byte +0x8];
    *(undefined4 *)(param_1 + 4) = 1;
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x50); local_8 = local_8 + 1) {
      uVar1 = FUN_0044c8a0(local_14,local_8 * 0xc + *(int *)(param_1 + 0x54),param_1 + 0x2c,
                           param_1 + 0x20);
      puVar2 = (undefined4 *)FUN_0044c8a0(local_20,uVar1);
      puVar3 = (undefined4 *)(*(int *)(param_1 + 0x54) + local_8 * 0xc);
      *puVar3 = *puVar2;
      puVar3[1 = byte +0x4] = puVar2[1 = byte +0x4];
      puVar3[2 = byte +0x8] = puVar2[2 = byte +0x8];
    }
  }
  return;
}
```

### decoy: 0x005ee480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ee480: 16-byte aligned: YES
  005ee480  55             PUSH EBP
  005ee481  8B EC          MOV EBP,ESP
  005ee483  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005ee480
string literals (0):
direct callees (1):
  0x00919723  operator new
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x54 [dec 84]  x1  via EAX
```

## Card B0-10-37

GOG function: CSWSEffectListHandler::OnApplyPureEvilPowers  gog 0x0074c480  callconv=(blank)  param_size_bytes=8  gog_size=1090  gog_retn=12
Candidate Steam address: 0x005a9ba0 (size 1090, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a9730:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074c010 CSWSEffectListHandler::OnApplyPureGoodPowers -> 0x005a9730  gate_class=REVIEW
- GOG-order neighbour after:  0x0074c8d0 CSWSEffectListHandler::IsEffectToIgnoreOnDeath -> 0x005a9ff0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x00734780 CGameEffect::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 10 common): GOG {0x4*,0x8*,0x30*,0x38*,0x8d*,0xab*,0x122*,0x130*,0x132*,0x1198*} | Steam {0x4*,0x8*,0x30*,0x38*,0x8d*,0xab*,0x122*,0x130*,0x132*,0x1198*}
imm sets (* = in both, 14 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0xd*,0x24*,0x28*,0x32*,0x3e*,0x43*,0x68*,0x8c*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0xd*,0x24*,0x28*,0x32*,0x3e*,0x43*,0x68*,0x8c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a9ba0: 16-byte aligned: YES
  005a9ba0  55             PUSH EBP
  005a9ba1  8B EC          MOV EBP,ESP
  005a9ba3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a9ba0
string literals (0):
direct callees (5):
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 9):
  +0x8 [dec 8]  x5  via EDX
  +0x130 [dec 304]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x122 [dec 290]  x2  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x38 [dec 56]  x1  via EDX
  +0x132 [dec 306]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a9ba0 FUN_005a9ba0 size=1090 ret=12 callers=0 callees=5

/* WARNING: Removing unreachable block (ram,0x005a9d02) */

undefined4 FUN_005a9ba0(int *param_1,undefined4 param_2)

{
  int iVar1;
  bool bVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  int local_78;
  int local_74;
  int local_70;
  int local_6c;
  char local_25;
  byte local_15;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952b8c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar3 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar3 == 0) {
    uVar4 = 1;
  }
  else {
    iVar1 = *(int *)(iVar3 + 0x1198);
    if (iVar1 == 0) {
      uVar4 = 1;
    }
    else {
      bVar2 = false;
      iVar5 = FUN_0051c600(*(undefined4 *)(iVar3 + 4));
      if ((iVar5 != 0) && (*(int *)(iVar5 + 0x38) == *(int *)(iVar3 + 4))) {
        bVar2 = true;
      }
      if (bVar2) {
        for (local_15 = 0; local_15 < *(byte *)(iVar1 + 0x8d); local_15 = local_15 + 1) {
          if (local_15 < *(byte *)(iVar1 + 0x8d)) {
            local_25 = *(char *)(iVar1 + 0xab + (uint)local_15 * 0x28);
          }
          else {
            local_25 = -1;
          }
          if (((local_25 == DAT_0099b19c) || (local_25 == DAT_0099b1a7)) ||
             (local_25 == DAT_0099b1a4)) {
            *(short *)(iVar1 + 0x130) = *(short *)(iVar1 + 0x130) + 0x32;
          }
          else if (((local_25 == DAT_0099b19b) || (local_25 == DAT_0099b1a6)) ||
                  (local_25 == DAT_0099b1a3)) {
            iVar3 = FUN_00919723(0x8c);
            local_8 = 0;
            if (iVar3 == 0) {
              local_6c = 0;
            }
            else {
              local_6c = FUN_005e4ac0(param_2);
            }
            local_8 = 0xffffffff;
            *(undefined2 *)(local_6c + 8) = 0xd;
            FUN_005e4e80(0,8);
            FUN_005e4e80(1,3);
            FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
            FUN_005e4e80(5,1);
            FUN_00544210(local_6c,0,0);
            iVar3 = FUN_00919723(0x8c);
            local_8 = 1;
            if (iVar3 == 0) {
              local_70 = 0;
            }
            else {
              local_70 = FUN_005e4ac0(param_2);
            }
            local_8 = 0xffffffff;
            *(undefined2 *)(local_70 + 8) = 0xd;
            FUN_005e4e80(0,8);
/* ... 41 more lines truncated by --max-lines */
```

### decoy: 0x005a9730 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a9730: 16-byte aligned: YES
  005a9730  55             PUSH EBP
  005a9731  8B EC          MOV EBP,ESP
  005a9733  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a9730
string literals (0):
direct callees (7):
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x0057ECA0  CSWSCreature::GetMaxForcePoints
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x0054A610  CSWSObject::SetCurrentHitPoints
struct offsets touched (top 10):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x130 [dec 304]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x98 [dec 152]  x2  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x38 [dec 56]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x132 [dec 306]  x1  via EDX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-10-38

GOG function: CSWSCombatRound::SetOffHandAttack  gog 0x00774c10  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x00590560 (size 25, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswscombatround_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00590580:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00774b70 CSWSCombatRound::RemoveLastAction -> 0x005904c0  gate_class=AUTO
- GOG-order neighbour after:  0x00774c30 CSWSCombatRound::GetOffHandAttack -> 0x00590580  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xac0*} | Steam {0xac0*}
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
prologue for 0x00590560: 16-byte aligned: YES
  00590560  55             PUSH EBP
  00590561  8B EC          MOV EBP,ESP
  00590563  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00590560
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAC0 [dec 2752]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00590560 FUN_00590560 size=25 ret=4 callers=2 callees=0

void __thiscall FUN_00590560(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xac0) = param_2;
  return;
}
```

### decoy: 0x00590580 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00590580: 16-byte aligned: YES
  00590580  55             PUSH EBP
  00590581  8B EC          MOV EBP,ESP
  00590583  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00590580
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAC0 [dec 2752]  x1  via EAX
```

## Card B0-10-39

GOG function: CSWSCombatRound::GetActualBonusEffectAttacks  gog 0x00776950  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x005922a0 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswscombatround_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00592280:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00776930 CSWSCombatRound::SetBonusEffectAttacks -> 0x00592280  gate_class=REVIEW
- GOG-order neighbour after:  0x00776970 CSWSCombatRound::GetActualBonusEffectAttacksLimited -> 0x005922c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xaf4*} | Steam {0xaf4*}
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
prologue for 0x005922a0: 16-byte aligned: YES
  005922a0  55             PUSH EBP
  005922a1  8B EC          MOV EBP,ESP
  005922a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005922a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAF4 [dec 2804]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005922a0 FUN_005922a0 size=20 ret=0 callers=2 callees=0

undefined4 __fastcall FUN_005922a0(int param_1)

{
  return *(undefined4 *)(param_1 + 0xaf4);
}
```

### decoy: 0x00592280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00592280: 16-byte aligned: YES
  00592280  55             PUSH EBP
  00592281  8B EC          MOV EBP,ESP
  00592283  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00592280
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAF4 [dec 2804]  x1  via EAX
```

## Card B0-10-40

GOG function: CSWSTrigger::Destructor  gog 0x0075d340  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0061e0b0 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0061e0e0:STR0/CALLEE0/SIG0.031/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0075d320 CSWSPlaceable::GetFirstName -> 0x0061e090  gate_class=REVIEW
- GOG-order neighbour after:  0x0075d370 CSWSTrigger::Destructor_2 -> 0x0061e0e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0075d370 CSWSTrigger::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x0061e0b0: 16-byte aligned: YES
  0061e0b0  55             PUSH EBP
  0061e0b1  8B EC          MOV EBP,ESP
  0061e0b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0061e0b0
string literals (0):
direct callees (2):
  0x0061E0E0  CSWSTrigger::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0061e0b0 FUN_0061e0b0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0061e0b0(undefined4 param_1,uint param_2)

{
  FUN_0061e0e0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0061e0e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0061e0e0: 16-byte aligned: YES
  0061e0e0  55             PUSH EBP
  0061e0e1  8B EC          MOV EBP,ESP
  0061e0e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0061e0e0
string literals (0):
direct callees (12):
  0x009196FD  operator delete
  0x005453C0  CSWSObject::GetArea
  0x00622670  CSWSTrigger::RemoveFromArea
  0x0051C280  (unnamed)
  0x0051E0C0  CServerAIMaster::RemoveObject
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x0055EA80  (unnamed)
  0x00735360  CExoLocString::Destructor
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0053F050  CSWSObject::Destructor_2
struct offsets touched (top 4):
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via EAX
  +0x2C8 [dec 712]  x1  via ECX
  +0x2D8 [dec 728]  x1  via ECX
```

## Card B0-10-41

GOG function: CSWSPlaceable::GetDead  gog 0x00770da0  callconv=(blank)  param_size_bytes=0  gog_size=35  gog_retn=0
Candidate Steam address: 0x00619e30 (size 35, retn 0)
Candidate sources: branch+name_map:high:batch5_reverse_engineering    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (GetDead 0x5477f0)
- best decoy (neighbour/other candidate) scores: 0x00619d00:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00770d30 CSWSPlaceable::SetOrientation -> 0x005f6990  gate_class=AUTO
- GOG-order neighbour after:  0x00770dd0 CSWSPlaceable::OnEndDialog -> 0x005f6a00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006ae4d0 CSWSObject::GetDead -> Steam 0x005477f0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xfc*} | Steam {0xfc*}
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
prologue for 0x00619e30: 16-byte aligned: YES
  00619e30  55             PUSH EBP
  00619e31  8B EC          MOV EBP,ESP
  00619e33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00619e30
string literals (0):
direct callees (1):
  0x005477F0  CSWSObject::GetDead
struct offsets touched (top 1):
  +0xFC [dec 252]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00619e30 FUN_00619e30 size=35 ret=0 callers=0 callees=1

undefined4 __fastcall FUN_00619e30(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0xfc) == 0) {
    uVar1 = FUN_005477f0();
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00619d00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00619d00: 16-byte aligned: YES
  00619d00  55             PUSH EBP
  00619d01  8B EC          MOV EBP,ESP
  00619d03  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00619d00
string literals (0):
direct callees (6):
  0x0051C0A0  CServerExoApp::GetGameObject
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0061D6E0  CSWSDoor::SetOpenState
  0x0061D620  CSWSDoor::GetLinkedObject
  0x006FD8D0  CVirtualMachine::RunScript
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via EDX
  +0x36C [dec 876]  x1  via EAX
```

## Card B0-10-42

GOG function: CSWSItemPropertyHandler::Destructor_2  gog 0x007c17d0  callconv=(blank)  param_size_bytes=0  gog_size=167  gog_retn=0
Candidate Steam address: 0x005aa3c0 (size 167, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005aa160:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c1770 CSWSStore::GetItemInInventory  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007c1880 CSWSItemPropertyHandler::Constructor -> 0x005aa470  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005aa3c0: 16-byte aligned: YES
  005aa3c0  55             PUSH EBP
  005aa3c1  8B EC          MOV EBP,ESP
  005aa3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005aa3c0
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005aa3c0 FUN_005aa3c0 size=167 ret=0 callers=1 callees=1

void __fastcall FUN_005aa3c0(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00952bb8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWSItemPropertyHandler::vftable;
  local_8 = 0;
  if (param_1[1 = byte +0x4] != 0) {
    FUN_009196fd(param_1[1 = byte +0x4],uVar1);
    param_1[1 = byte +0x4] = 0;
  }
  if (param_1[2 = byte +0x8] != 0) {
    FUN_009196fd(param_1[2 = byte +0x8],uVar1);
    param_1[2 = byte +0x8] = 0;
  }
  *param_1 = CItemPropertyApplierRemover::vftable;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x005aa160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aa160: 16-byte aligned: YES
  005aa160  55             PUSH EBP
  005aa161  8B EC          MOV EBP,ESP
  005aa163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005aa160
string literals (0):
direct callees (1):
  0x005AA310  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B0-10-43

GOG function: CSWVirtualMachineCommands::ExecuteCommandCutsceneMove  gog 0x00783260  callconv=__thiscall  param_size_bytes=8  gog_size=165  gog_retn=8
Candidate Steam address: 0x0066dc80 (size 165, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0066dba0:STR0/CALLEE0/SIG0.7/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00783180 CSWVirtualMachineCommands::ExecuteCommandCutsceneAttack -> 0x0066dba0  gate_class=REVIEW
- GOG-order neighbour after:  0x00783310 Global::ExecuteCommandBarkString  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640c40 CVirtualMachine::StackPopVector -> Steam ?  in candidate callees: n/a
  0x00640d10 CVirtualMachine::StackPopObject -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x006810b0 CSWSCreature::AddCutsceneMoveActions -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x20*,0xfffff82f*} | Steam {0x8*,0x20*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0066dc80: 16-byte aligned: YES
  0066dc80  55             PUSH EBP
  0066dc81  8B EC          MOV EBP,ESP
  0066dc83  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0066dc80
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDA20  CVirtualMachine::StackPopVector
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00568480  CSWSCreature::AddCutsceneMoveActions
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0066dc80 FUN_0066dc80 size=165 ret=8 callers=0 callees=5

undefined4 __fastcall FUN_0066dc80(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_1c = 0;
  local_18 = 0;
  local_14 = 0;
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fda20(&local_1c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_006fd9a0(&local_c);
      if (iVar1 == 0) {
        uVar2 = 0xfffff82f;
      }
      else {
        local_10 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
        if (local_10 != 0) {
          FUN_00568480(local_8,&local_1c,local_c);
        }
        uVar2 = 0;
      }
    }
  }
  return uVar2;
}
```

### decoy: 0x0066dba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066dba0: 16-byte aligned: YES
  0066dba0  55             PUSH EBP
  0066dba1  8B EC          MOV EBP,ESP
  0066dba3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066dba0
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005683D0  CSWSCreature::AddCutsceneAttackActions
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

## Card B0-10-44

GOG function: CSWVirtualMachineCommands::Destructor_2  gog 0x0077b500  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00536be0 (size 44, retn 4)
Candidate sources: branch+name_map:high:batch6_reverse_engineering    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00536bb0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0077b430 CSWVirtualMachineCommands::Destructor -> 0x00665eb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0077b530 CSWVirtualMachineCommands::InitializeCommands -> 0x00665f50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0077b430 CSWVirtualMachineCommands::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x00536be0: 16-byte aligned: YES
  00536be0  55             PUSH EBP
  00536be1  8B EC          MOV EBP,ESP
  00536be3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00536be0
string literals (0):
direct callees (2):
  0x00665EB0  CSWVirtualMachineCommands::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00536be0 FUN_00536be0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00536be0(undefined4 param_1,uint param_2)

{
  FUN_00665eb0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
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

## Card B0-10-45

GOG function: CSWSEffectListHandler::OnRemoveBodyFuel  gog 0x0074b0d0  callconv=(blank)  param_size_bytes=4  gog_size=55  gog_retn=8
Candidate Steam address: 0x005a8820 (size 55, retn 8)
Candidate sources: branch+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyEffectImmunity
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a87e0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074b090 CSWSEffectListHandler::OnApplyBodyFuel -> 0x005a87e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0074b110 CSWSEffectListHandler::OnApplyEffectImmunity -> 0x005a87e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x11c4*} | Steam {0x30*,0x11c4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x005a8820: 16-byte aligned: YES
  005a8820  55             PUSH EBP
  005a8821  8B EC          MOV EBP,ESP
  005a8823  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a8820
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x1  via EDX
  +0x11C4 [dec 4548]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a8820 FUN_005a8820 size=55 ret=8 callers=0 callees=0

undefined4 __thiscall FUN_005a8820(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    *(undefined4 *)(iVar1 + 0x11c4) = 0;
  }
  return 1;
}
```

### decoy: 0x005a87e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a87e0: 16-byte aligned: YES
  005a87e0  55             PUSH EBP
  005a87e1  8B EC          MOV EBP,ESP
  005a87e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a87e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x1  via EDX
  +0x11C4 [dec 4548]  x1  via ECX
```

## Card B0-10-46

GOG function: CServerExoAppInternal::SaveGame  gog 0x007b2990  callconv=(blank)  param_size_bytes=12  gog_size=1386  gog_retn=16
Candidate Steam address: 0x0052f610 (size 1386, retn 16)
Candidate sources: branch+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=StartSave
- anchors: A-STR 2 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR '%s%06d - %s%sScreen'@00993df4; 'SAVES'@00993e08; CALLEE 1 (SaveGame 0x51bdf0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0052fb80:STR0/CALLEE0/SIG0.155/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007b2920 CServerExoAppInternal::GetModuleName  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007b2f00 CServerExoAppInternal::LoadGame -> 0x0052fb80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (22), mapped to Steam through trusted rows only:
  0x00401b50 CAppManager::DoSaveGameScreenShot -> Steam ?  in candidate callees: n/a
  0x004099e0 Global::UpdateScreen -> Steam ?  in candidate callees: n/a
  0x0040c8f0 CClientExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam ?  in candidate callees: n/a
  0x004e39c0 CSWCameraOnAStick::SetCameraForScreenShot -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  ... 14 more GOG callees
GOG strings (5): 'SAVES:'; 'SAVEGAME'; '>SAVES:%06d - %s'; 'SAVES' ...
disp sets (* = in both, 10 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x2c*,0x38*,0x40*,0x80*,0x2a0*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x2c*,0x38*,0x40*,0x80*,0x2a0*}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0xc*,0x10*,0x18*,0x20*,0xd8*,0x106a*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0xc*,0x10*,0x18*,0x20*,0xd8*,0x106a*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x0052f610: 16-byte aligned: YES
  0052f610  55             PUSH EBP
  0052f611  8B EC          MOV EBP,ESP
  0052f613  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0052f610
string literals (6):
  0x00993E30  at 0052f6ed  'SAVES:'
  0x00993E20  at 0052f726  'SAVES:%06d - %s'
  0x00993E30  at 0052f737  'SAVES:'
  0x00993E14  at 0052f874  'SAVEGAME'
  0x00993E08  at 0052f9ed  'SAVES'
  0x00993DF4  at 0052fabd  '%s%06d - %s%sScreen'
direct callees (22):
  0x00733540  CExoString::CExoString(empty)
  0x0070BC60  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00714F30  CExoResMan::CreateDirectory2
  0x00733780  CExoString::~CExoString
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00714DE0  CExoResMan::GetFreeDiskSpace
  0x00736080  (unnamed)
  0x006374E0  (unnamed)
  0x007156E0  CExoResMan::CleanDirectory
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x005337F0  (unnamed)
  0x00739790  (unnamed)
  0x00733B60  (unnamed)
  0x0073F430  CClientExoApp::GetModule
  0x007DE110  (unnamed)
  0x00401B90  CaptureThumbnail(name)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
struct offsets touched (top 10):
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x14 [dec 20]  x5  via EDX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x2A0 [dec 672]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
  +0x38 [dec 56]  x1  via ECX
  +0x40 [dec 64]  x1  via ECX
  +0x80 [dec 128]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0052f610 FUN_0052f610 size=1386 ret=16 callers=2 callees=22

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_0052f610(int param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  undefined1 local_7c [8];
  undefined1 local_74 [8];
  undefined1 local_6c [8];
  undefined1 local_64 [8];
  undefined1 local_5c [8];
  undefined4 local_54;
  undefined4 local_50;
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  int local_3c;
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  uint local_28;
  uint local_24;
  int local_1c;
  uint local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094e4b8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(*(int *)(*(int *)(DAT_00a1b4a4 + 4) + 4) + 0x2a0) != 0) {
    (**(code **)(**(int **)(*(int *)(*(int *)(DAT_00a1b4a4 + 4) + 4) + 0x2a0) + 0x2c))
              (DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  }
  FUN_00733540();
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  FUN_0070bc60(DAT_0099c2b4,0);
  FUN_00733570("SAVES:");
  local_8._0_1_ = 2;
  FUN_00714f30(local_44);
  local_8._0_1_ = 1;
  FUN_00733780();
  uVar2 = FUN_00734010();
  FUN_00734270(local_30,"SAVES:%06d - %s",param_2,uVar2);
  FUN_00733570("SAVES:");
  local_8._0_1_ = 3;
  local_1c = FUN_00714de0(local_4c,&local_18);
  local_8._0_1_ = 1;
  FUN_00733780();
  if (local_1c == 1) {
    local_28 = *(uint *)(param_1 + 0x100c8);
    local_24 = 0;
    uVar3 = FUN_00736080(local_30);
    uVar5 = local_14 + (uint)CARRY4(uVar3,local_18);
    if ((uVar5 <= local_24) && ((local_24 != uVar5 || (uVar3 + local_18 < local_28)))) {
      FUN_006374e0(param_4,2,1);
      local_50 = 0;
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      ExceptionList = local_10;
      return local_50;
    }
  }
  iVar4 = FUN_00714f30(local_30);
  if ((iVar4 == 0) && (iVar4 = FUN_007156e0(local_30,0,0), iVar4 == 0)) {
    local_54 = 0;
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_00733780();
/* ... 63 more lines truncated by --max-lines */
```

### decoy: 0x0052fb80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052fb80: 16-byte aligned: YES
  0052fb80  55             PUSH EBP
  0052fb81  8B EC          MOV EBP,ESP
  0052fb83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0052fb80
string literals (2):
  0x00993E38  at 0052fc7c  '%06d - %s'
  0x00993E30  at 0052fc9b  'SAVES:'
direct callees (14):
  0x00733540  CExoString::CExoString(empty)
  0x007405D0  CClientExoApp::SetLoadBarProgress
  0x00740C30  (unnamed)
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x007337C0  CExoString::operator=
  0x007338D0  CExoString::operator=_2
  0x00733DF0  CExoString::operator+
  0x00733780  CExoString::~CExoString
  0x00711690  CExoResMan::AddResourceDirectory
  0x005FBF60  CSWPartyTable::LoadTableInfo
  0x00656710  CSWGlobalVariableTable::LoadFromSave (thiscall table)
  0x00711710  CExoResMan::RemoveResourceDirectory
  0x00531C50  CServerExoAppInternal::LoadModule
struct offsets touched (top 4):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x1F254 [dec 127572]  x1  via EAX
  +0x1F33C [dec 127804]  x1  via ECX
  +0x1F344 [dec 127812]  x1  via EDX
```

## Card B0-10-47

GOG function: CSWVirtualMachineCommands::ExecuteCommandIntToFloat  gog 0x007a08e0  callconv=__thiscall  param_size_bytes=8  gog_size=78  gog_retn=8
Candidate Steam address: 0x0068b290 (size 78, retn 8)
Candidate sources: branch+name_map:high:batch6_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0068b2e0:STR0/CALLEE0/SIG0.4/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a0760 Global::ExecuteCommandInsertString  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007a0930 Global::ExecuteCommandIntToHexString  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640c20 CVirtualMachine::StackPushFloat -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0xc*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068b290: 16-byte aligned: YES
  0068b290  55             PUSH EBP
  0068b291  8B EC          MOV EBP,ESP
  0068b293  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068b290
string literals (0):
direct callees (2):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0068b290 FUN_0068b290 size=78 ret=8 callers=0 callees=2

undefined4 FUN_0068b290(void)

{
  int iVar1;
  undefined4 uVar2;
  int local_c;
  float local_8;
  
  iVar1 = FUN_006fd9a0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = (float)local_c;
    iVar1 = FUN_006fda00(local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0068b2e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068b2e0: 16-byte aligned: YES
  0068b2e0  55             PUSH EBP
  0068b2e1  8B EC          MOV EBP,ESP
  0068b2e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0068b2e0
string literals (1):
  0x00998CD8  at 0068b349  '0x%08x'
direct callees (5):
  0x00733540  CExoString::CExoString(empty)
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733780  CExoString::~CExoString
  0x00734270  CExoString::Format
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

## Card B0-10-48

GOG function: CSWVirtualMachineCommands::ExecuteCommandActionEquipMostDamagingMelee  gog 0x0077f030  callconv=(blank)  param_size_bytes=0  gog_size=123  gog_retn=8
Candidate Steam address: 0x00669a50 (size 123, retn 8)
Candidate sources: branch+name_map:high:batch6_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006699d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0077efb0 CSWVirtualMachineCommands::ExecuteCommandActionDoCommand -> 0x006699d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0077f0b0 CSWVirtualMachineCommands::ExecuteCommandActionEquipMostDamagingRanged -> 0x00669ad0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640d10 CVirtualMachine::StackPopObject -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x00698e60 CSWSCreature::EquipMostDamagingMeleeWeapon -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff82f*} | Steam {0x8*,0x14*,0xfffff82f*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x00669a50: 16-byte aligned: YES
  00669a50  55             PUSH EBP
  00669a51  8B EC          MOV EBP,ESP
  00669a53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00669a50
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00580330  CSWSCreature::EquipMostDamagingMeleeWeapon
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00669a50 FUN_00669a50 size=123 ret=8 callers=0 callees=4

undefined4 __fastcall FUN_00669a50(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9a0(&local_10);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      local_8 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
      if (local_8 != 0) {
        FUN_00580330(local_c,local_10,0);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x006699d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006699d0: 16-byte aligned: YES
  006699d0  55             PUSH EBP
  006699d1  8B EC          MOV EBP,ESP
  006699d3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006699d0
string literals (0):
direct callees (4):
  0x006FDB30  CVirtualMachine::StackPopCommand
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E1340  CSWSObject::AddDoCommandAction
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX
```

## Card B0-10-49

GOG function: CSWCArea::AddWeatherObject  gog 0x00456040  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x007a86d0 (size 3053, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCArea::UpdateDayNightCycle
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a86b0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00455f20 CSWCArea::SetAsNight -> 0x007a9520  gate_class=AUTO
- GOG-order neighbour after:  0x00456060 CSWCArea::GetWeatherObject -> 0x007a9660  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x44,0x60,0x64,0x74,0x78,0x90,0x94,0xa4,0xb4,0xb8,0xbc,...(+6)}
imm sets (* = in both, 1 common): GOG {0x4*,0x1c8} | Steam {0x4*,0x8,0xc,0x20,0x41,0x12c,0x138,0x144,0x150,0x15c,0x168,0x2c4}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007a86d0: 16-byte aligned: YES
  007a86d0  55             PUSH EBP
  007a86d1  8B EC          MOV EBP,ESP
  007a86d3  81 EC C4 02 00 00 SUB ESP,0x2c4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a86d0
string literals (3):
  0x0099FCEC  at 007a915d  'sun gidy_sun_amb color %f %f %f'
  0x0099FCC8  at 007a91a4  'sun gidy_sun_diff color %f %f %f'
  0x009884D8  at 007a9262  '%f %f %f'
direct callees (11):
  0x0073F430  CClientExoApp::GetModule
  0x007AA7A0  CSWCArea::UpdateShadowingLights
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0045CFE0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00919671  (unnamed)
  0x00463900  (unnamed)
  0x00407890  (unnamed)
  0x007A9780  CSWCArea::UpdateSoundObjectsTimeOfDay
  0x008914F0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x8 [dec 8]  x23  via EAX,ECX,EDX
  +0x4 [dec 4]  x19  via EAX,ECX,EDX
  +0xB4 [dec 180]  x5  via EAX,ECX,EDX
  +0x1B8 [dec 440]  x5  via EAX,ECX,EDX
  +0x1BC [dec 444]  x5  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EAX,ECX
  +0x90 [dec 144]  x4  via EAX,ECX,EDX
  +0x94 [dec 148]  x4  via EAX,ECX,EDX
  +0xA4 [dec 164]  x4  via ECX,EDX
  +0x188 [dec 392]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007a86d0 FUN_007a86d0 size=3053 ret=4 callers=1 callees=11

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_007a86d0(int param_1)

{
  float fVar1;
  float fVar2;
  int iVar3;
  float *pfVar4;
  int local_f4;
  int local_ec;
  int local_e8;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_9c;
  float local_98;
  float local_94;
  char local_8c [128];
  uint local_c;
  float local_8;
  
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_9c = 0.0;
  local_98 = 0.0;
  local_94 = 0.0;
  local_cc = 0.0;
  local_c8 = 0.0;
  local_c4 = 0.0;
  local_b4 = 0.0;
  local_b0 = 0.0;
  local_ac = 0.0;
  if (((*(int *)(param_1 + 0x1b8) != 0) || (*(int *)(param_1 + 0x1bc) != 0)) &&
     (iVar3 = FUN_0073f430(), iVar3 != 0)) {
    fVar1 = *(float *)(iVar3 + 0xbc);
    local_8 = *(float *)(iVar3 + 0xb8);
    if (*(int *)(param_1 + 0x1c0) == 0) {
      if (*(int *)(param_1 + 0x1b8) == 0) {
        if (*(int *)(param_1 + 0x1bc) != 0) {
          for (local_e8 = 0; local_e8 < *(int *)(param_1 + 8) * *(int *)(param_1 + 0xc);
              local_e8 = local_e8 + 1) {
          }
        }
      }
      else {
        for (local_e8 = 0; local_e8 < *(int *)(param_1 + 8) * *(int *)(param_1 + 0xc);
            local_e8 = local_e8 + 1) {
        }
      }
      *(undefined4 *)(param_1 + 0x1c0) = 1;
    }
    if (*(int *)(param_1 + 0x1b8) == 0) {
      if (*(int *)(param_1 + 0x1bc) != 0) {
        if (fVar1 < local_8 == (fVar1 == local_8)) {
          fVar2 = local_8 / fVar1;
          _DAT_009f6390 = fVar2 * (float)_DAT_0099fd10 + (float)_DAT_0098bd30;
          FUN_004452b0();
          FUN_004452b0();
          FUN_004452b0();
          FUN_0045cfe0();
          pfVar4 = (float *)FUN_0044c8a0();
          local_9c = *pfVar4;
          local_98 = pfVar4[1 = byte +0x4];
          local_94 = pfVar4[2 = byte +0x8];
          FUN_0045cfe0();
          pfVar4 = (float *)FUN_0044c8a0();
          local_cc = *pfVar4;
          local_c8 = pfVar4[1 = byte +0x4];
          local_c4 = pfVar4[2 = byte +0x8];
          FUN_0045cfe0();
          pfVar4 = (float *)FUN_0044c8a0();
          local_b4 = *pfVar4;
          local_b0 = pfVar4[1 = byte +0x4];
          local_ac = pfVar4[2 = byte +0x8];
/* ... 120 more lines truncated by --max-lines */
```

### decoy: 0x007a86b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a86b0: 16-byte aligned: YES
  007a86b0  55             PUSH EBP
  007a86b1  8B EC          MOV EBP,ESP
  007a86b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a86b0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-10-50

GOG function: CSWVirtualMachineCommands::ExecuteCommandAssignCommand  gog 0x00782fd0  callconv=__thiscall  param_size_bytes=8  gog_size=243  gog_retn=8
Candidate Steam address: 0x0066d9f0 (size 243, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0066daf0:STR0/CALLEE0/SIG0.438/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00782d60 Global::ExecuteCommandApplyEffectOnObject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007830d0 CSWVirtualMachineCommands::ExecuteCommandAttack -> 0x0066daf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam ?  in candidate callees: n/a
  0x00640d50 CVirtualMachine::StackPopCommand -> Steam ?  in candidate callees: n/a
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x24*,0xfffff82f*} | Steam {0x4*,0x8*,0x24*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0066d9f0: 16-byte aligned: YES
  0066d9f0  55             PUSH EBP
  0066d9f1  8B EC          MOV EBP,ESP
  0066d9f3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0066d9f0
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDB30  CVirtualMachine::StackPopCommand
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0066d9f0 FUN_0066d9f0 size=243 ret=8 callers=0 callees=7

undefined4 FUN_0066d9f0(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined1 *puVar7;
  int extraout_var;
  undefined4 local_18;
  int local_14;
  undefined1 local_10 [4];
  undefined4 local_c;
  char local_5;
  
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_14 = 0;
    iVar1 = extraout_var;
    iVar3 = FUN_006fdb30(&local_14);
    if (iVar3 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      puVar7 = local_10;
      uVar6 = local_c;
      FUN_0051c080(local_c,puVar7);
      local_5 = FUN_0053dfb0(uVar6,puVar7);
      if (local_5 == DAT_0099447f) {
        local_18 = DAT_00994484;
        if (*(int *)(iVar1 + 4) == 1) {
          local_18 = *(undefined4 *)(iVar1 + 8);
        }
        uVar5 = 0;
        uVar4 = 0;
        uVar6 = DAT_009923f8;
        iVar3 = local_14;
        FUN_0051c280(0,0,local_18,local_c,DAT_009923f8,local_14,iVar1,
                     *(undefined4 *)(DAT_00a1b4a4 + 8),uVar2);
        FUN_0051e170(uVar4,uVar5,local_18,local_c,uVar6,iVar3);
      }
      else if (local_14 != 0) {
        FUN_009196fd(local_14);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0066daf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066daf0: 16-byte aligned: YES
  0066daf0  55             PUSH EBP
  0066daf1  8B EC          MOV EBP,ESP
  0066daf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066daf0
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00568540  CSWSCreature::AddAttackActions
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX,EDX
```

## Card B0-10-51

GOG function: CSWVirtualMachineCommands::CopyGameDefinedStructure  gog 0x0077e710  callconv=(blank)  param_size_bytes=8  gog_size=424  gog_retn=8
Candidate Steam address: 0x00669130 (size 424, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006692e0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0077e650 CServerAIMaster::AppendToScriptsRun  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0077e8c0 CSWVirtualMachineCommands::SaveGameDefinedStructure -> 0x006692e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x006a4e10 CScriptLocation::Constructor -> Steam ?  in candidate callees: n/a
  0x006a4e60 CScriptLocation::CopyScriptLocation -> Steam ?  in candidate callees: n/a
  0x006a51b0 CScriptTalent::Constructor -> Steam ?  in candidate callees: n/a
  0x006a5200 CScriptTalent::CopyScriptTalent -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734c90 CGameEffect::CopyEffect -> Steam ?  in candidate callees: n/a
  0x0077a9b0 CSWCCMessageData::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x0077ac40 CScriptEvent::CopyTo -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x4*,0x8*,0x18*,0x34*,0x44*,0x8c*} | Steam {0x2*,0x3*,0x4*,0x8*,0x18*,0x34*,0x44*,0x8c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00669130: 16-byte aligned: YES
  00669130  55             PUSH EBP
  00669131  8B EC          MOV EBP,ESP
  00669133  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00669130
string literals (0):
direct callees (9):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4FD0  CGameEffect::CopyEffect
  0x005E1A60  (unnamed)
  0x005E1CF0  (unnamed)
  0x0053E060  (unnamed)
  0x0053E0B0  (unnamed)
  0x0053E400  (unnamed)
  0x0053E450  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00669130 FUN_00669130 size=424 ret=8 callers=0 callees=9

undefined4 FUN_00669130(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959bdc;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (param_1 == 0) {
    iVar2 = FUN_00919723(0x8c,uVar1);
    local_8 = 0;
    if (iVar2 == 0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_005e4920(0);
    }
    local_8 = 0xffffffff;
    FUN_005e4fd0(param_2,0);
  }
  else if (param_1 == 1) {
    iVar2 = FUN_00919723(0x34,uVar1);
    local_8 = 1;
    if (iVar2 == 0) {
      local_4c = 0;
    }
    else {
      local_4c = FUN_005e1a60();
    }
    local_8 = 0xffffffff;
    FUN_005e1cf0(param_2);
    local_48 = local_4c;
  }
  else if (param_1 == 2) {
    iVar2 = FUN_00919723(0x18,uVar1);
    local_8 = 2;
    if (iVar2 == 0) {
      local_50 = 0;
    }
    else {
      local_50 = FUN_0053e060();
    }
    local_8 = 0xffffffff;
    FUN_0053e0b0(param_2);
    local_48 = local_50;
  }
  else if (param_1 == 3) {
    iVar2 = FUN_00919723(0x18,uVar1);
    local_8 = 3;
    if (iVar2 == 0) {
      local_54 = 0;
    }
    else {
      local_54 = FUN_0053e400();
    }
    local_8 = 0xffffffff;
    FUN_0053e450(param_2);
    local_48 = local_54;
  }
  else {
    local_48 = 0;
  }
  ExceptionList = local_10;
  return local_48;
}
```

### decoy: 0x006692e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006692e0: 16-byte aligned: YES
  006692e0  55             PUSH EBP
  006692e1  8B EC          MOV EBP,ESP
  006692e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006692e0
string literals (1):
  0x00998C38  at 006692ed  'GameDefinedStrct'
direct callees (5):
  0x00719940  CResGFF::AddStructToStruct
  0x005E5B40  CGameEffect::SaveGameEffect
  0x005E1E60  (unnamed)
  0x0053E210  (unnamed)
  0x0053E4C0  (unnamed)
struct offsets touched (top 0):
```

## Card B0-10-52

GOG function: CSWSEffectListHandler::IsEffectToIgnoreOnDeath  gog 0x0074c8d0  callconv=(blank)  param_size_bytes=4  gog_size=269  gog_retn=4
Candidate Steam address: 0x005a9ff0 (size 269, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005aa160:STR0/CALLEE0/SIG0.225/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0074c480 CSWSEffectListHandler::OnApplyPureEvilPowers -> 0x005a9ba0  gate_class=REVIEW
- GOG-order neighbour after:  0x0074cbf0 CSWVarTable::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006a5080 CLoopingVisualEffect::GetIsBeam -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00735c20 CGameEffect::GetEffectTypeScriptConstant -> Steam ?  in candidate callees: n/a
GOG strings (1): '33#@EffectType'
disp sets (* = in both, 4 common): GOG {0x8*,0x24*,0x128*,0x130*} | Steam {0x8*,0x24*,0x128*,0x130*}
imm sets (* = in both, 4 common): GOG {0x4*,0x15*,0x1e*,0x28*} | Steam {0x4*,0x15*,0x1e*,0x28*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005a9ff0: 16-byte aligned: YES
  005a9ff0  55             PUSH EBP
  005a9ff1  8B EC          MOV EBP,ESP
  005a9ff3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a9ff0
string literals (0):
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5F60  CGameEffect::GetEffectTypeScriptConstant
  0x005E4E60  CGameEffect::GetInteger
  0x0053E2D0  (unnamed)
struct offsets touched (top 4):
  +0x8 [dec 8]  x1  via EDX
  +0x24 [dec 36]  x1  via EDX
  +0x128 [dec 296]  x1  via EDX
  +0x130 [dec 304]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005a9ff0 FUN_005a9ff0 size=269 ret=4 callers=1 callees=6

undefined4 __thiscall FUN_005a9ff0(undefined4 param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_28 [8];
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096b268;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = *(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x128);
  local_14 = 0;
  do {
    if (*(int *)(local_18 + 0x24) <= local_14) {
      ExceptionList = local_10;
      return 0;
    }
    FUN_00733570("EffectType");
    local_8 = 0;
    local_20 = FUN_0071bcf0(local_14,local_28,&local_1c);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1,param_1);
    if (local_20 != 0) {
      iVar2 = FUN_005e5f60();
      if (iVar2 == local_1c) {
        ExceptionList = local_10;
        return 1;
      }
      if ((local_1c == 0x15) && (*(short *)(param_2 + 8) == 0x1e)) {
        uVar3 = FUN_005e4e60(0);
        iVar2 = FUN_0053e2d0(uVar3);
        if (iVar2 != 0) {
          ExceptionList = local_10;
          return 1;
        }
      }
    }
    local_14 = local_14 + 1;
  } while( true );
}
```

### decoy: 0x005aa160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aa160: 16-byte aligned: YES
  005aa160  55             PUSH EBP
  005aa161  8B EC          MOV EBP,ESP
  005aa163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005aa160
string literals (0):
direct callees (1):
  0x005AA310  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B0-10-53

GOG function: CSWSItemPropertyHandler::InitializeItemProperties  gog 0x007c18a0  callconv=__thiscall  param_size_bytes=0  gog_size=1397  gog_retn=0
Candidate Steam address: 0x005aa490 (size 1397, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005aa3c0:STR0/CALLEE0/SIG0.122/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c1880 CSWSItemPropertyHandler::Constructor -> 0x005aa470  gate_class=REVIEW
- GOG-order neighbour after:  0x007c1e20 CSWSItemPropertyHandler::OnItemPropertyApplied -> 0x005aaa10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 45 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x2c*,0x30*,0x34*,0x38*,0x3c*,...(+31)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x2c*,0x30*,0x34*,0x38*,0x3c*,...(+31)}
imm sets (* = in both, 4 common): GOG {0x4*,0x10*,0x46*,0x118*} | Steam {0x4*,0x10*,0x46*,0x118*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005aa490: 16-byte aligned: YES
  005aa490  55             PUSH EBP
  005aa491  8B EC          MOV EBP,ESP
  005aa493  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005aa490
string literals (0):
direct callees (1):
  0x00919723  operator new
struct offsets touched (top 10):
  +0x4 [dec 4]  x51  via EAX,ECX,EDX
  +0x8 [dec 8]  x47  via EAX,ECX,EDX
  +0x20 [dec 32]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x10 [dec 16]  x2  via EAX,ECX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x24 [dec 36]  x2  via EAX,EDX
  +0x2C [dec 44]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005aa490 FUN_005aa490 size=1397 ret=0 callers=1 callees=1

void __fastcall FUN_005aa490(int param_1)

{
  undefined4 uVar1;
  int local_8;
  
  uVar1 = FUN_00919723(0x118);
  *(undefined4 *)(param_1 + 4) = uVar1;
  uVar1 = FUN_00919723(0x118);
  *(undefined4 *)(param_1 + 8) = uVar1;
  for (local_8 = 0; local_8 < 0x46; local_8 = local_8 + 1) {
    *(undefined4 *)(*(int *)(param_1 + 4) + local_8 * 4) = 0;
    *(undefined4 *)(*(int *)(param_1 + 8) + local_8 * 4) = 0;
  }
  **(undefined4 **)(param_1 + 4) = FUN_005adaf0;
  **(undefined4 **)(param_1 + 8) = FUN_005adcf0;
  *(code **)(*(int *)(param_1 + 4) + 4) = FUN_005adfb0;
  *(code **)(*(int *)(param_1 + 8) + 4) = FUN_005ae260;
  *(code **)(*(int *)(param_1 + 4) + 8) = FUN_005adfb0;
  *(code **)(*(int *)(param_1 + 8) + 8) = FUN_005ae260;
  *(code **)(*(int *)(param_1 + 4) + 0xc) = FUN_005adfb0;
  *(code **)(*(int *)(param_1 + 8) + 0xc) = FUN_005ae260;
  *(code **)(*(int *)(param_1 + 4) + 0x10) = FUN_005adfb0;
  *(code **)(*(int *)(param_1 + 8) + 0x10) = FUN_005ae260;
  *(code **)(*(int *)(param_1 + 4) + 0x14) = FUN_005aaad0;
  *(code **)(*(int *)(param_1 + 8) + 0x14) = FUN_005ab1f0;
  *(code **)(*(int *)(param_1 + 4) + 0x18) = FUN_005aaad0;
  *(code **)(*(int *)(param_1 + 8) + 0x18) = FUN_005ab1f0;
  *(code **)(*(int *)(param_1 + 4) + 0x1c) = FUN_005aaad0;
  *(code **)(*(int *)(param_1 + 8) + 0x1c) = FUN_005ab1f0;
  *(code **)(*(int *)(param_1 + 4) + 0x20) = FUN_005ab980;
  *(code **)(*(int *)(param_1 + 8) + 0x20) = FUN_005abef0;
  *(code **)(*(int *)(param_1 + 4) + 0x24) = FUN_005af860;
  *(code **)(*(int *)(param_1 + 8) + 0x24) = FUN_005af980;
  *(code **)(*(int *)(param_1 + 4) + 0x2c) = FUN_005ac3f0;
  *(code **)(*(int *)(param_1 + 8) + 0x2c) = FUN_005ac870;
  *(code **)(*(int *)(param_1 + 4) + 0x30) = FUN_005ac3f0;
  *(code **)(*(int *)(param_1 + 8) + 0x30) = FUN_005ac870;
  *(code **)(*(int *)(param_1 + 4) + 0x34) = FUN_005ac3f0;
  *(code **)(*(int *)(param_1 + 8) + 0x34) = FUN_005ac870;
  *(code **)(*(int *)(param_1 + 4) + 0x38) = FUN_005afa80;
  *(code **)(*(int *)(param_1 + 8) + 0x38) = FUN_005afc10;
  *(code **)(*(int *)(param_1 + 4) + 0x3c) = FUN_005afea0;
  *(code **)(*(int *)(param_1 + 8) + 0x3c) = FUN_005b01e0;
  *(code **)(*(int *)(param_1 + 4) + 0x40) = FUN_005b04f0;
  *(code **)(*(int *)(param_1 + 8) + 0x40) = FUN_005b0660;
  *(code **)(*(int *)(param_1 + 4) + 0x44) = FUN_005b0820;
  *(code **)(*(int *)(param_1 + 8) + 0x44) = FUN_005b0990;
  *(code **)(*(int *)(param_1 + 4) + 0x48) = FUN_005b0b50;
  *(code **)(*(int *)(param_1 + 8) + 0x48) = FUN_005b0cc0;
  *(code **)(*(int *)(param_1 + 4) + 0x4c) = FUN_005b0e60;
  *(code **)(*(int *)(param_1 + 8) + 0x4c) = FUN_005b1060;
  *(code **)(*(int *)(param_1 + 4) + 0x50) = FUN_005b1320;
  *(code **)(*(int *)(param_1 + 8) + 0x50) = FUN_005b14c0;
  *(code **)(*(int *)(param_1 + 4) + 0x54) = FUN_005b16b0;
  *(code **)(*(int *)(param_1 + 8) + 0x54) = FUN_005b1840;
  *(code **)(*(int *)(param_1 + 4) + 0x60) = FUN_005b1a30;
  *(code **)(*(int *)(param_1 + 8) + 0x60) = FUN_005b1d00;
  *(code **)(*(int *)(param_1 + 4) + 0x64 [dec 100]) = FUN_005ae530;
  *(code **)(*(int *)(param_1 + 8) + 0x64 [dec 100]) = FUN_005ae680;
  *(code **)(*(int *)(param_1 + 4) + 0x68) = FUN_005ae820;
  *(code **)(*(int *)(param_1 + 8) + 0x68) = FUN_005ae8e0;
  *(code **)(*(int *)(param_1 + 4) + 0x6c) = FUN_005ae820;
  *(code **)(*(int *)(param_1 + 8) + 0x6c) = FUN_005ae8e0;
  *(code **)(*(int *)(param_1 + 4) + 0x74) = FUN_005ad9b0;
  *(code **)(*(int *)(param_1 + 8) + 0x74) = FUN_005adad0;
  *(code **)(*(int *)(param_1 + 4) + 0x84) = FUN_005ae9a0;
  *(code **)(*(int *)(param_1 + 8) + 0x84) = FUN_005aea60;
  *(code **)(*(int *)(param_1 + 4) + 0x88) = FUN_005ae9a0;
  *(code **)(*(int *)(param_1 + 8) + 0x88) = FUN_005aea60;
  *(code **)(*(int *)(param_1 + 4) + 0x8c) = FUN_005b21c0;
  *(code **)(*(int *)(param_1 + 8) + 0x8c) = FUN_005b2340;
  *(code **)(*(int *)(param_1 + 4) + 0xd8) = FUN_005b21c0;
  *(code **)(*(int *)(param_1 + 8) + 0xd8) = FUN_005b2340;
  *(code **)(*(int *)(param_1 + 4) + 0x90) = FUN_005b1f30;
  *(code **)(*(int *)(param_1 + 8) + 0x90) = FUN_005b2080;
  *(code **)(*(int *)(param_1 + 4) + 0x98) = FUN_005acc60;
  *(code **)(*(int *)(param_1 + 8) + 0x98) = FUN_005acfc0;
  *(code **)(*(int *)(param_1 + 4) + 0x9c) = FUN_005acc60;
/* ... 27 more lines truncated by --max-lines */
```

### decoy: 0x005aa3c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aa3c0: 16-byte aligned: YES
  005aa3c0  55             PUSH EBP
  005aa3c1  8B EC          MOV EBP,ESP
  005aa3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005aa3c0
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
```

## Card B0-10-54

GOG function: CSWSCombatRound::GetActualBonusEffectAttacksLimited  gog 0x00776970  callconv=(blank)  param_size_bytes=0  gog_size=39  gog_retn=0
Candidate Steam address: 0x005922c0 (size 39, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswscombatround_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005922a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00776950 CSWSCombatRound::GetActualBonusEffectAttacks -> 0x005922a0  gate_class=REVIEW
- GOG-order neighbour after:  0x007769a0 CSWSCombatRound::SetDeflectArrow -> 0x005922f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xaf4*} | Steam {0xaf4*}
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
prologue for 0x005922c0: 16-byte aligned: YES
  005922c0  55             PUSH EBP
  005922c1  8B EC          MOV EBP,ESP
  005922c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005922c0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAF4 [dec 2804]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005922c0 FUN_005922c0 size=39 ret=0 callers=2 callees=0

undefined4 __fastcall FUN_005922c0(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0xaf4) < 3) {
    uVar1 = *(undefined4 *)(param_1 + 0xaf4);
  }
  else {
    uVar1 = 2;
  }
  return uVar1;
}
```

### decoy: 0x005922a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005922a0: 16-byte aligned: YES
  005922a0  55             PUSH EBP
  005922a1  8B EC          MOV EBP,ESP
  005922a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005922a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAF4 [dec 2804]  x1  via EAX
```

## Card B0-10-55

GOG function: CSWCArea::AddWeatherObject  gog 0x00456040  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x007a9800 (size 38, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCArea::StartWeather
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a9830:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00455f20 CSWCArea::SetAsNight -> 0x007a9520  gate_class=AUTO
- GOG-order neighbour after:  0x00456060 CSWCArea::GetWeatherObject -> 0x007a9660  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x1f0,0x1f5}
imm sets (* = in both, 1 common): GOG {0x4*,0x1c8} | Steam {0x4*,0x1770}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007a9800: 16-byte aligned: YES
  007a9800  55             PUSH EBP
  007a9801  8B EC          MOV EBP,ESP
  007a9803  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a9800
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1F0 [dec 496]  x1  via EDX
  +0x1F5 [dec 501]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007a9800 FUN_007a9800 size=38 ret=4 callers=2 callees=0

void __thiscall FUN_007a9800(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x1f5) = param_2;
  *(undefined4 *)(param_1 + 0x1f0) = 6000;
  return;
}
```

### decoy: 0x007a9830 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a9830: 16-byte aligned: YES
  007a9830  55             PUSH EBP
  007a9831  8B EC          MOV EBP,ESP
  007a9833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a9830
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1F0 [dec 496]  x1  via ECX
  +0x1F5 [dec 501]  x1  via EAX
```

## Card B0-10-56

GOG function: CSWSCombatRound::SetCurrentAction  gog 0x00774f50  callconv=(blank)  param_size_bytes=1  gog_size=25  gog_retn=4
Candidate Steam address: 0x005908a0 (size 25, retn 4)
Candidate sources: branch+name_map:high:gog_cswscombatround_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00590880:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00774f30 CSWSCombatRound::GetCurrentAction -> 0x00590880  gate_class=REVIEW
- GOG-order neighbour after:  0x00774f70 CSWSCombatRound::GetAttack -> 0x005908c0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xb24*} | Steam {0xb24*}
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
prologue for 0x005908a0: 16-byte aligned: YES
  005908a0  55             PUSH EBP
  005908a1  8B EC          MOV EBP,ESP
  005908a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005908a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xB24 [dec 2852]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005908a0 FUN_005908a0 size=25 ret=4 callers=2 callees=0

void __thiscall FUN_005908a0(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0xb24) = param_2;
  return;
}
```

### decoy: 0x00590880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00590880: 16-byte aligned: YES
  00590880  55             PUSH EBP
  00590881  8B EC          MOV EBP,ESP
  00590883  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00590880
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xB24 [dec 2852]  x1  via EAX
```

## Card B0-10-57

GOG function: CSWVirtualMachineCommands::Destructor  gog 0x0077b430  callconv=(blank)  param_size_bytes=0  gog_size=127  gog_retn=0
Candidate Steam address: 0x00665eb0 (size 127, retn 0)
Candidate sources: branch+name_map:high:batch6_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00665e90:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0077b2c0 CScriptEvent::operator==  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0077b500 CSWVirtualMachineCommands::Destructor_2 -> 0x00536be0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00665eb0: 16-byte aligned: YES
  00665eb0  55             PUSH EBP
  00665eb1  8B EC          MOV EBP,ESP
  00665eb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00665eb0
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0xC [dec 12]  x3  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00665eb0 FUN_00665eb0 size=127 ret=0 callers=1 callees=1

void __fastcall FUN_00665eb0(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00959b48;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWVirtualMachineCommands::vftable;
  local_8 = 0;
  if (param_1[3 = byte +0xc] != 0) {
    FUN_009196fd(param_1[3 = byte +0xc],uVar1);
    param_1[3 = byte +0xc] = 0;
  }
  *param_1 = CVirtualMachineCmdImplementer::vftable;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00665e90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00665e90: 16-byte aligned: YES
  00665e90  55             PUSH EBP
  00665e91  8B EC          MOV EBP,ESP
  00665e93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00665e90
string literals (0):
direct callees (1):
  0x0069E4E0  (unnamed)
struct offsets touched (top 0):
```

## Card B0-10-58

GOG function: CSWSItemPropertyHandler::RemoveSpecialWalk  gog 0x007c6430  callconv=__thiscall  param_size_bytes=12  gog_size=392  gog_retn=16
Candidate Steam address: 0x005aecb0 (size 402, retn 16)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::RemoveChangedSavingThrow
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.55 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.55
- best decoy (neighbour/other candidate) scores: 0x005aee50:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c6260 CSWSItemPropertyHandler::ApplySpecialWalk -> 0x005aee50  gate_class=REVIEW
- GOG-order neighbour after:  0x007c65c0 CSWSItemPropertyHandler::ApplyTrueSeeing -> 0x005af1b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*,0x15c,0x1198} | Steam {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*,0x122}
imm sets (* = in both, 3 common): GOG {0x7*,0x10*,0x4c,0x148*} | Steam {0x2,0x3,0x7*,0x10*,0x44,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005aecb0: 16-byte aligned: YES
  005aecb0  55             PUSH EBP
  005aecb1  8B EC          MOV EBP,ESP
  005aecb3  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005aecb0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 7):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via EAX
  +0x2 [dec 2]  x1  via EAX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x122 [dec 290]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005aecb0 FUN_005aecb0 size=402 ret=16 callers=2 callees=2

undefined4 FUN_005aecb0(int param_1,int param_2,int param_3,int param_4)

{
  byte bVar1;
  short sVar2;
  undefined4 *puVar3;
  int iVar4;
  uint uVar5;
  int local_18;
  
  sVar2 = *(short *)(param_3 + 2);
  local_18 = 0;
  while( true ) {
    if (*(int *)(param_4 + 0x14c) <= local_18) {
      return 0;
    }
    iVar4 = *(int *)(*(int *)(param_4 + 0x148) + local_18 * 4);
    if ((((((*(ushort *)(iVar4 + 8) == DAT_00995f84) || (*(ushort *)(iVar4 + 8) == DAT_00995f88)) &&
          ((*(ushort *)(iVar4 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
         ((*(int *)(iVar4 + 0x18) == *(int *)(param_2 + 0x14) &&
          (iVar4 = FUN_005e4e60(0), iVar4 == param_1)))) &&
        ((iVar4 = FUN_005e4e60(1), iVar4 == 0 &&
         (bVar1 = *(byte *)(DAT_00a1b4d0 + 0x122), uVar5 = FUN_005e4e60(3), uVar5 == bVar1)))) &&
       ((((iVar4 = FUN_005e4e60(1), iVar4 == 1 && (sVar2 == 1)) || ((iVar4 == 3 && (sVar2 == 3))))
        || ((iVar4 == 2 && (sVar2 == 2)))))) break;
    local_18 = local_18 + 1;
  }
  puVar3 = *(undefined4 **)(*(int *)(param_4 + 0x148) + local_18 * 4);
  FUN_00544790(*puVar3,puVar3[1 = byte +0x4]);
  return 0;
}
```

### decoy: 0x005aee50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aee50: 16-byte aligned: YES
  005aee50  55             PUSH EBP
  005aee51  8B EC          MOV EBP,ESP
  005aee53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005aee50
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 4):
  +0xA [dec 10]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EDX
```

## Card B0-10-59

GOG function: CSWSEffectListHandler::OnApplyLightsaberThrow  gog 0x0074b140  callconv=(blank)  param_size_bytes=8  gog_size=3187  gog_retn=12
Candidate Steam address: 0x005a8860 (size 3187, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a94e0:STR0/CALLEE0/SIG0.077/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074b110 CSWSEffectListHandler::OnApplyEffectImmunity -> 0x005a87e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0074bdc0 CSWSEffectListHandler::OnRemoveLightsaberThrow -> 0x005a94e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (18), mapped to Steam through trusted rows only:
  0x004ed450 CSWSpellArray::GetSpell -> Steam ?  in candidate callees: n/a
  0x0050e640 CSWRules::RollDice -> Steam ?  in candidate callees: n/a
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x0067a5b0 CSWInventory::GetItemInSlot -> Steam ?  in candidate callees: n/a
  0x006e1280 CSWSCreatureStats::GetLevel -> Steam ?  in candidate callees: n/a
  0x006f9e40 CSWSCreatureStats::HasSpell -> Steam ?  in candidate callees: n/a
  ... 10 more GOG callees
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0xa*,0x10*,0x30*,0xa8*,0xac*,0xb4*,0x104*,0x1128*,0x1150*,0x1198*,0x11dc*} | Steam {0x4*,0x8*,0xa*,0x10*,0x30*,0xa8*,0xac*,0xb4*,0x104*,0x1128*,0x1150*,0x1198*,0x11dc*}
imm sets (* = in both, 24 common): GOG {0x2*,0x3*,0x4*,0x6*,0xc*,0xe*,0xf*,0x10*,0x13*,0x14*,0x15*,0x1e*,0x26*,0x31*,...(+10)} | Steam {0x2*,0x3*,0x4*,0x6*,0xc*,0xe*,0xf*,0x10*,0x13*,0x14*,0x15*,0x1e*,0x26*,0x31*,...(+10)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a8860: 16-byte aligned: YES
  005a8860  55             PUSH EBP
  005a8861  8B EC          MOV EBP,ESP
  005a8863  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a8860
string literals (0):
direct callees (18):
  0x006BD9C0  CSWSCreatureStats::HasSpell
  0x006D0620  (unnamed)
  0x005E4EE0  CGameEffect::GetObjectID
  0x005E4E60  CGameEffect::GetInteger
  0x006C1450  (unnamed)
  0x0051C0A0  CServerExoApp::GetGameObject
  0x005142F0  (unnamed)
  0x0051C280  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E4F00  CGameEffect::SetObjectID
  0x0091C860  _ftol2_sse
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4DD0  CGameEffect::SetNumIntegers
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x006A2000  CSWRules::RollDice(n,sides)
struct offsets touched (top 10):
  +0x8 [dec 8]  x20  via EAX,ECX,EDX
  +0x4 [dec 4]  x15  via EAX,EDX
  +0x10 [dec 16]  x14  via EDX
  +0xA [dec 10]  x6  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EDX
  +0x1198 [dec 4504]  x3  via EAX,ECX,EDX
  +0x104 [dec 260]  x2  via EAX,EDX
  +0xA8 [dec 168]  x1  via EAX
  +0xAC [dec 172]  x1  via EDX
  +0xB4 [dec 180]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005a8860 FUN_005a8860 size=3187 ret=12 callers=0 callees=18

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_005a8860(int *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  float10 fVar4;
  undefined4 uVar5;
  int local_1f4;
  int local_1ec;
  int local_1e8;
  uint local_1e4;
  float local_dc;
  float local_d8;
  float local_d4;
  int local_d0;
  int local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  undefined2 local_8c;
  int *local_88;
  int local_84;
  int local_80;
  int local_7c;
  uint local_78;
  int local_74;
  int local_70;
  float local_6c;
  uint local_68;
  int *local_64;
  int *local_60;
  int local_5c;
  int local_58;
  int local_54;
  float local_50;
  int local_4c;
  int local_48[4 = byte +0x10];
  undefined4 local_38;
  uint local_34;
  int *local_30[4 = byte +0x10];
  float local_20;
  float local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952aba;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (local_14 != 0) {
    iVar1 = FUN_006bd9c0(0,0xa2,0);
    if (iVar1 == 0) {
      iVar1 = FUN_006bd9c0(0,0xa3,0);
      if (iVar1 == 0) {
        *(undefined4 *)(local_14 + 0x1128) = 1;
      }
    }
    local_18 = FUN_006d0620(0x10);
    if (local_18 != 0) {
      local_48[1 = byte +0x4] = param_1[1 = byte +0x4];
/* ... 186 more lines truncated by --max-lines */
```

### decoy: 0x005a94e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a94e0: 16-byte aligned: YES
  005a94e0  55             PUSH EBP
  005a94e1  8B EC          MOV EBP,ESP
  005a94e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a94e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x1  via EDX
  +0x1128 [dec 4392]  x1  via ECX
```

## Card B0-10-60

GOG function: CSWVirtualMachineCommands::DestroyGameDefinedStructure  gog 0x0077ec30  callconv=(blank)  param_size_bytes=8  gog_size=315  gog_retn=8
Candidate Steam address: 0x00669650 (size 315, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00669790:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0077eb30 CSWVirtualMachineCommands::GetEqualGameDefinedStructure -> 0x00669550  gate_class=AUTO
- GOG-order neighbour after:  0x0077ed70 CSWVirtualMachineCommands::CreateGameDefinedStructure -> 0x00669790  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  0x0077aa40 CSWCCMessageData::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x3*,0x4*,0x8*,0x44*} | Steam {0x2*,0x3*,0x4*,0x8*,0x44*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00669650: 16-byte aligned: YES
  00669650  55             PUSH EBP
  00669651  8B EC          MOV EBP,ESP
  00669653  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00669650
string literals (0):
direct callees (4):
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
  0x005E1AF0  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00669650 FUN_00669650 size=315 ret=8 callers=0 callees=4

void FUN_00669650(int param_1,int param_2)

{
  if ((param_1 == 0) && (param_2 != 0)) {
    FUN_005e4ca0();
    FUN_009196fd(param_2);
  }
  if ((param_1 == 1) && (param_2 != 0)) {
    FUN_005e1af0();
    FUN_009196fd(param_2);
  }
  if ((param_1 == 2) && (param_2 != 0)) {
    FUN_00623320();
    FUN_009196fd(param_2);
  }
  if ((param_1 == 3) && (param_2 != 0)) {
    FUN_00623320();
    FUN_009196fd(param_2);
  }
  return;
}
```

### decoy: 0x00669790 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00669790: 16-byte aligned: YES
  00669790  55             PUSH EBP
  00669791  8B EC          MOV EBP,ESP
  00669793  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00669790
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E1A60  (unnamed)
  0x0053E060  (unnamed)
  0x0053E400  (unnamed)
struct offsets touched (top 0):
```
