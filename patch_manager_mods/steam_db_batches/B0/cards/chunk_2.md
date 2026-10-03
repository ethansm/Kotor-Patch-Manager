# Review cards B0 chunk 2

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-2-01

GOG function: CSWSArea::PlotSoundPath  gog 0x0070cd90  callconv=(blank)  param_size_bytes=16  gog_size=273  gog_retn=16
Candidate Steam address: 0x00552c90 (size 252, retn 16)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSArea::PathPointDFSTransTableInitialize
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00552b70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0070cd60 CSWSArea::GridDFSTransTableGet -> 0x00552b40  gate_class=AUTO
- GOG-order neighbour after:  0x0070ceb0 CSWSArea::PathPointDFSTransTableInitialize -> 0x00552c90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0070cd60 CSWSArea::GridDFSTransTableGet -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x1b8*} | Steam {0x4*,0x8*,0xc*,0x1b8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x10*,0x100*} | Steam {0x4*,0x8*,0x10*,0x100*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x00552c90: 16-byte aligned: YES
  00552c90  55             PUSH EBP
  00552c91  8B EC          MOV EBP,ESP
  00552c93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00552c90
string literals (0):
direct callees (1):
  0x00552B40  CSWSArea::GridDFSTransTableGet
struct offsets touched (top 1):
  +0x1B8 [dec 440]  x9  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00552c90 FUN_00552c90 size=252 ret=16 callers=1 callees=1

void __thiscall FUN_00552c90(int param_1,int param_2,int param_3,int param_4,int param_5)

{
  undefined4 local_8;
  
  local_8 = FUN_00552b40(param_2,param_3);
  if ((((*(int *)(local_8 * 0x10 + *(int *)(param_1 + 0x1b8)) != param_2) ||
       (*(int *)(*(int *)(param_1 + 0x1b8) + 4 + local_8 * 0x10) != param_3)) &&
      (*(int *)(*(int *)(param_1 + 0x1b8) + 8 + local_8 * 0x10) <= param_4)) &&
     ((*(int *)(*(int *)(param_1 + 0x1b8) + 8 + local_8 * 0x10) != param_4 ||
      (*(int *)(*(int *)(param_1 + 0x1b8) + 0xc + local_8 * 0x10) <= param_5)))) {
    local_8 = local_8 + 0x100;
  }
  *(int *)(local_8 * 0x10 + *(int *)(param_1 + 0x1b8)) = param_2;
  *(int *)(*(int *)(param_1 + 0x1b8) + 4 + local_8 * 0x10) = param_3;
  *(int *)(*(int *)(param_1 + 0x1b8) + 8 + local_8 * 0x10) = param_4;
  *(int *)(*(int *)(param_1 + 0x1b8) + 0xc + local_8 * 0x10) = param_5;
  return;
}
```

### decoy: 0x00552b70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00552b70: 16-byte aligned: YES
  00552b70  55             PUSH EBP
  00552b71  8B EC          MOV EBP,ESP
  00552b73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00552b70
string literals (0):
direct callees (1):
  0x00552B40  CSWSArea::GridDFSTransTableGet
struct offsets touched (top 1):
  +0x1B8 [dec 440]  x8  via EAX,ECX,EDX
```

## Card B0-2-02

GOG function: CSWCObject::AnimationAttack  gog 0x004749e0  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef880 (size 129, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef7f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474950 CSWCObject::AnimationParry -> 0x007ef7f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00474a70 CSWCObject::AnimationRunning -> 0x007ef470  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xcc*,0xe0*,0x130*} | Steam {0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef880: 16-byte aligned: YES
  007ef880  55             PUSH EBP
  007ef881  8B EC          MOV EBP,ESP
  007ef883  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef880
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef880 FUN_007ef880 size=129 ret=4 callers=1 callees=1

bool __thiscall FUN_007ef880(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe4c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef7f0: 16-byte aligned: YES
  007ef7f0  55             PUSH EBP
  007ef7f1  8B EC          MOV EBP,ESP
  007ef7f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef7f0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-2-03

GOG function: CGuiInGame::AppendToMsgBuffer  gog 0x004ce720  callconv=(blank)  param_size_bytes=9  gog_size=282  gog_retn=12
Candidate Steam address: 0x007be090 (size 282, retn 12)
Candidate sources: branch+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiInGame::AddFeedbackLine
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007be1b0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ce6e0 CSWGuiNavigable::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ce840 CGuiInGame::AppendToCombatBuffer  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605c70 CExoString::operator!=_2 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0xc*,0x110*,0x11c*} | Steam {0x8*,0xc*,0x110*,0x11c*}
imm sets (* = in both, 5 common): GOG {0x4*,0xc*,0x10*,0x3f*,0x40*} | Steam {0x4*,0xc*,0x10*,0x3f*,0x40*}

### abi
```
GOG param bytes: 9; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(9/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 9 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x007be090: 16-byte aligned: YES
  007be090  55             PUSH EBP
  007be091  8B EC          MOV EBP,ESP
  007be093  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007be090
string literals (0):
direct callees (2):
  0x00733B60  (unnamed)
  0x007337C0  CExoString::operator=
struct offsets touched (top 4):
  +0x11C [dec 284]  x8  via EAX,ECX,EDX
  +0x110 [dec 272]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007be090 FUN_007be090 size=282 ret=12 callers=9 callees=2

void __thiscall FUN_007be090(int param_1,undefined4 param_2,undefined4 param_3,undefined1 param_4)

{
  int iVar1;
  int iVar2;
  ushort local_8;
  
  iVar1 = FUN_00733b60(&DAT_00986925);
  if (iVar1 != 0) {
    if (0x3f < *(uint *)(param_1 + 0x11c)) {
      *(undefined4 *)(param_1 + 0x11c) = 0x3f;
      for (local_8 = 0; (uint)local_8 < *(uint *)(param_1 + 0x11c); local_8 = local_8 + 1) {
        iVar1 = (local_8 + 1) * 0x10 + *(int *)(param_1 + 0x110);
        iVar2 = (uint)local_8 * 0x10 + *(int *)(param_1 + 0x110);
        FUN_007337c0(iVar1);
        *(undefined4 *)(iVar2 + 8) = *(undefined4 *)(iVar1 + 8);
        *(undefined1 *)(iVar2 + 0xc) = *(undefined1 *)(iVar1 + 0xc);
      }
    }
    FUN_007337c0(param_2);
    *(undefined4 *)(*(int *)(param_1 + 0x110) + 8 + *(int *)(param_1 + 0x11c) * 0x10) = param_3;
    *(undefined1 *)(*(int *)(param_1 + 0x110) + 0xc + *(int *)(param_1 + 0x11c) * 0x10) = param_4;
    *(int *)(param_1 + 0x11c) = *(int *)(param_1 + 0x11c) + 1;
  }
  return;
}
```

### decoy: 0x007be1b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007be1b0: 16-byte aligned: YES
  007be1b0  55             PUSH EBP
  007be1b1  8B EC          MOV EBP,ESP
  007be1b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007be1b0
string literals (0):
direct callees (2):
  0x00733B60  (unnamed)
  0x007337C0  CExoString::operator=
struct offsets touched (top 4):
  +0x124 [dec 292]  x8  via EAX,ECX,EDX
  +0x118 [dec 280]  x5  via EAX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x2  via ECX,EDX
```

## Card B0-2-04

GOG function: CSWCLevelUpStats::GetClass  gog 0x0048f9b0  callconv=(blank)  param_size_bytes=1  gog_size=51  gog_retn=4
Candidate Steam address: 0x00844e40 (size 51, retn 4)
Candidate sources: branch    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreatureStats::GetClass,CSWSCreatureStats::GetClassByPosition
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 6 (AddGrantedPowers 0x847a70,CalcNumberFeats 0x847e50,ComputeNumberKnownSpellsLeft 0x847f90); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00844e80:STR0/CALLEE4/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0048f9b0 CSWSCreatureStats::GetClass -> 0x00844e40  gate_class=REVIEW
- GOG-order neighbour after:  0x0048f9f0 CSWCLevelUpStats::GetClassLevel  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xf8*} | Steam {0xf8*}
imm sets (* = in both, 4 common): GOG {0x2*,0x4*,0x5*,0x8*} | Steam {0x2*,0x4*,0x5*,0x8*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00844e40: 16-byte aligned: YES
  00844e40  55             PUSH EBP
  00844e41  8B EC          MOV EBP,ESP
  00844e43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00844e40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00844e40 FUN_00844e40 size=51 ret=4 callers=43 callees=0

undefined1 __thiscall FUN_00844e40(int param_1,byte param_2)

{
  undefined1 uVar1;
  
  if (param_2 < 2) {
    uVar1 = *(undefined1 *)(param_1 + 0xf8 + (uint)param_2 * 0x20);
  }
  else {
    uVar1 = 0xff;
  }
  return uVar1;
}
```

### decoy: 0x00844e80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00844e80: 16-byte aligned: YES
  00844e80  55             PUSH EBP
  00844e81  8B EC          MOV EBP,ESP
  00844e83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00844e80
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-2-05

GOG function: CGuiInGame::NextSWInGameGui  gog 0x004da930  callconv=(blank)  param_size_bytes=0  gog_size=389  gog_retn=0
Candidate Steam address: 0x007ca230 (size 389, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007ca3c0:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004da760 CGuiInGame::HideSWInGameGui  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004daac0 CGuiInGame::PrevSWInGameGui -> 0x007ca3c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam ?  in candidate callees: n/a
  0x004e0e60 CGuiInGame::UpdateCreatedInGameGUI -> Steam ?  in candidate callees: n/a
  0x00527f60 CSWGuiInGameMenu::SetActiveControlID -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00640af0 CVirtualMachine::RunScript -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'k_pend_screenchg'
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x2c*,0x30*,0x128*} | Steam {0x4*,0x8*,0xc*,0x2c*,0x30*,0x128*}
imm sets (* = in both, 4 common): GOG {0x2*,0x8*,0x1c*,0x7f000000*} | Steam {0x2*,0x8*,0x1c*,0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ca230: 16-byte aligned: YES
  007ca230  55             PUSH EBP
  007ca231  8B EC          MOV EBP,ESP
  007ca233  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007ca230
string literals (1):
  0x009A175C  at 007ca36d  'k_pend_screenchg'
direct callees (9):
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410670  (unnamed)
  0x007D0760  CGuiInGame::UpdateCreatedInGameGUI
  0x00410530  CSWGuiManager::AddPanel
  0x00410780  (unnamed)
  0x00756860  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x006FD8D0  CVirtualMachine::RunScript
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x2C [dec 44]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x128 [dec 296]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ca230 FUN_007ca230 size=389 ret=0 callers=1 callees=9

void __fastcall FUN_007ca230(int param_1)

{
  undefined4 local_2c;
  undefined1 local_20 [8];
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
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

### decoy: 0x007ca3c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ca3c0: 16-byte aligned: YES
  007ca3c0  55             PUSH EBP
  007ca3c1  8B EC          MOV EBP,ESP
  007ca3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007ca3c0
string literals (1):
  0x009A175C  at 007ca4fd  'k_pend_screenchg'
direct callees (9):
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410670  (unnamed)
  0x007D0760  CGuiInGame::UpdateCreatedInGameGUI
  0x00410530  CSWGuiManager::AddPanel
  0x00410780  (unnamed)
  0x00756860  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x006FD8D0  CVirtualMachine::RunScript
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x2C [dec 44]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x128 [dec 296]  x1  via EAX
```

## Card B0-2-06

GOG function: CSWSEffectListHandler::OnRemoveSetStateInternal  gog 0x00741150  callconv=(blank)  param_size_bytes=8  gog_size=234  gog_retn=8
Candidate Steam address: 0x0059db50 (size 733, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnRemoveSetState
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.14 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059de30:STR0/CALLEE0/SIG0.131/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00740720 CSWSEffectListHandler::OnApplySetStateInternal -> 0x0059de30  gate_class=REVIEW
- GOG-order neighbour after:  0x00741260 CSWSEffectListHandler::OnApplyMovementSpeedIncrease -> 0x0059e970  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640af0 CVirtualMachine::RunScript -> Steam ?  in candidate callees: n/a
  0x006934c0 CSWSCreature::RecomputeAmbientAnimationState -> Steam ?  in candidate callees: n/a
  0x006dbb50 CSWSCreature::UpdateVisibleList -> Steam ?  in candidate callees: n/a
  0x006dc130 CSWSCreature::ClearVisibleList -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x30*,0x270,0x10e8} | Steam {0x4*,0x8,0xa,0x18,0x20,0x30*,0xec,0xff5}
imm sets (* = in both, 1 common): GOG {0x3,0x6,0x8*,0xe,0x12,0x18} | Steam {0x4,0x8*,0x9,0x6c,0x8c,0x148,0x7f000000,0xfffffff8}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059db50: 16-byte aligned: YES
  0059db50  55             PUSH EBP
  0059db51  8B EC          MOV EBP,ESP
  0059db53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059db50
string literals (0):
direct callees (7):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 8):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via ECX,EDX
  +0xA [dec 10]  x2  via EDX
  +0xEC [dec 236]  x2  via EAX,ECX
  +0xFF5 [dec 4085]  x2  via ECX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x20 [dec 32]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059db50 FUN_0059db50 size=733 ret=8 callers=0 callees=7

undefined4 FUN_0059db50(int *param_1,int param_2)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int local_7c;
  undefined4 local_28;
  int local_24;
  int local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095225b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar2 != 0) {
    param_1[0x3b = byte +0xec] = 1;
    *(undefined1 *)(iVar2 + 0xff5) = 0;
    piVar3 = param_1 + 0x52;
    if (0 < param_1[0x53 = byte +0x14c]) {
      local_1c = 0;
      local_24 = 0;
      local_28 = 0x7f000000;
      while ((local_1c < param_1[0x53 = byte +0x14c] && (*(ushort *)(*(int *)(*piVar3 + local_1c * 4) + 8) < 9)))
      {
        if ((*(short *)(*(int *)(*piVar3 + local_1c * 4) + 8) == 8) &&
           ((*(int *)(*piVar3 + local_1c * 4) != param_2 &&
            (iVar4 = FUN_005e4e60(0), local_24 < iVar4)))) {
          local_24 = FUN_005e4e60(0);
          local_28 = *(undefined4 *)(*(int *)(*piVar3 + local_1c * 4) + 0x18);
        }
        local_1c = local_1c + 1;
      }
      while ((local_1c < param_1[0x53 = byte +0x14c] && (*(ushort *)(*(int *)(*piVar3 + local_1c * 4) + 8) < 10)))
      {
        if (*(short *)(*(int *)(*piVar3 + local_1c * 4) + 8) == 9) {
          puVar1 = *(undefined4 **)(*piVar3 + local_1c * 4);
          FUN_00544790(*puVar1,puVar1[1 = byte +0x4]);
        }
        else {
          local_1c = local_1c + 1;
        }
      }
      if (local_24 != 0) {
        *(undefined1 *)(iVar2 + 0xff5) = (undefined1)local_24;
        param_1[0x3b = byte +0xec] = 0;
        iVar2 = FUN_00919723(0x8c);
        local_8 = 0;
        if (iVar2 == 0) {
          local_7c = 0;
        }
        else {
          local_7c = FUN_005e4920(1);
        }
        local_8 = 0xffffffff;
        FUN_005e5ea0(local_28);
        *(ushort *)(local_7c + 0xa [dec 10]) = *(ushort *)(local_7c + 0xa [dec 10]) & 0xfff8 | DAT_009967b8;
        *(undefined4 *)(local_7c + 0x20) = 0;
        *(undefined2 *)(local_7c + 8) = 9;
        FUN_005e4e80(0,local_24);
        FUN_00544210(local_7c,0,0);
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x0059de30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059de30: 16-byte aligned: YES
  0059de30  55             PUSH EBP
  0059de31  8B EC          MOV EBP,ESP
  0059de33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0059de30
string literals (0):
direct callees (13):
  0x005E4E60  CGameEffect::GetInteger
  0x00541080  CSWSObject::ClearAllActions
  0x00590430  CSWSCombatRound::RemoveAllActions
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x005E4FA0  CGameEffect::GetExpiryTime
  0x005E4F80  CGameEffect::SetExpiryTime
  0x00544210  CSWSObject::ApplyEffect
  0x0057A990  CSWSCreature::RecomputeAmbientAnimationState
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005908C0  CSWSCombatRound::GetAttack
  0x006E6E70  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0x10DC [dec 4316]  x3  via EAX,ECX,EDX
  +0xEC [dec 236]  x2  via EAX
  +0x122 [dec 290]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x145 [dec 325]  x1  via ECX
  +0x154 [dec 340]  x1  via EAX
  +0x390 [dec 912]  x1  via ECX
```

## Card B0-2-07

GOG function: CSWCObject::GetCurrentAnimation  gog 0x00472930  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x007ed7d0 (size 18, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (ResetDialogAnimations 0x7c93e0,ResetTalkAnimations 0x7c9560)
- best decoy (neighbour/other candidate) scores: 0x007ed5f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004727b0 CSWCObject::Destructor_3  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00472950 CSWCObject::AttachToScene -> 0x007ed7f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x48*} | Steam {0x48*}
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
prologue for 0x007ed7d0: 16-byte aligned: YES
  007ed7d0  55             PUSH EBP
  007ed7d1  8B EC          MOV EBP,ESP
  007ed7d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed7d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x48 [dec 72]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed7d0 FUN_007ed7d0 size=18 ret=0 callers=22 callees=0

undefined2 __fastcall FUN_007ed7d0(int param_1)

{
  return *(undefined2 *)(param_1 + 0x48);
}
```

### decoy: 0x007ed5f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed5f0: 16-byte aligned: YES
  007ed5f0  55             PUSH EBP
  007ed5f1  8B EC          MOV EBP,ESP
  007ed5f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007ed5f0
string literals (0):
direct callees (11):
  0x0073F8D0  CClientExoApp::AddObjectToHitcheckIgnoreList
  0x007F0A50  CSWCObject::KillAllVisualEffects
  0x0073F790  CClientExoApp::GetClientAIMaster
  0x007A2540  (unnamed)
  0x0070B080  StreamingSource scalar delete
  0x009196FD  operator delete
  0x0054A900  CSWSObject::DetachFromClientObject
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0072F5F0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x5C [dec 92]  x3  via ECX,EDX
  +0xC8 [dec 200]  x3  via EAX,ECX
  +0xCC [dec 204]  x3  via EAX,EDX
  +0xF8 [dec 248]  x3  via EAX,ECX,EDX
```

## Card B0-2-08

GOG function: CGuiInGame::IsCameraDialog  gog 0x004dbda0  callconv=(blank)  param_size_bytes=16  gog_size=55  gog_retn=16
Candidate Steam address: 0x007cb6a0 (size 55, retn 16)
Candidate sources: mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb720:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbce0 CGuiInGame::UnlockDialogParticipantOrientations  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004dbde0 CGuiInGame::IsAnimationPlayingInDialog -> 0x007cb6e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005af830 CSWGuiDialogLetterbox::SetFade -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68*} | Steam {0x68*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x007cb6a0: 16-byte aligned: YES
  007cb6a0  55             PUSH EBP
  007cb6a1  8B EC          MOV EBP,ESP
  007cb6a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb6a0
string literals (0):
direct callees (1):
  0x008BB0E0  (unnamed)
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007cb6a0 FUN_007cb6a0 size=55 ret=16 callers=2 callees=1

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

### decoy: 0x007cb720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb720: 16-byte aligned: YES
  007cb720  55             PUSH EBP
  007cb721  8B EC          MOV EBP,ESP
  007cb723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb720
string literals (0):
direct callees (1):
  0x008BB200  (unnamed)
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX,ECX
```

## Card B0-2-09

GOG function: CClientExoApp::EnableVideoEffect  gog 0x0040de60  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740980 (size 28, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::SetLoadScreenHint+name_held_elsewhere=0x007409a0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007409a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040de40 CClientExoApp::SetLoadScreenHint  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040de80 CClientExoApp::DisableVideoEffect -> 0x007409c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ab50 CClientExoAppInternal::EnableVideoEffect -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740980: 16-byte aligned: YES
  00740980  55             PUSH EBP
  00740981  8B EC          MOV EBP,ESP
  00740983  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740980
string literals (0):
direct callees (1):
  0x0079CF40  CClientExoAppInternal::SetLoadScreenHint
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740980 FUN_00740980 size=28 ret=4 callers=8 callees=1
THUNK -> 0x0079CF40 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740980(undefined4 param_1)

{
  FUN_0079cf40(param_1);
  return;
}
```

### decoy: 0x007409a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007409a0: 16-byte aligned: YES
  007409a0  55             PUSH EBP
  007409a1  8B EC          MOV EBP,ESP
  007409a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007409a0
string literals (0):
direct callees (1):
  0x0079E550  CClientExoAppInternal::EnableVideoEffect
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-2-10

GOG function: CSWCArea::AttachmentFromServerArea  gog 0x00457880  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x007aae80 (size 25, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007aaea0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00457800 CSWCArea::GetServerArea -> 0x007aae00  gate_class=AUTO
- GOG-order neighbour after:  0x004578a0 CSWCArea::DetachFromServerArea -> 0x007aaea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x280*} | Steam {0x280*}
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
prologue for 0x007aae80: 16-byte aligned: YES
  007aae80  55             PUSH EBP
  007aae81  8B EC          MOV EBP,ESP
  007aae83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007aae80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x280 [dec 640]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007aae80 FUN_007aae80 size=25 ret=4 callers=1 callees=0

void __thiscall FUN_007aae80(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x280) = param_2;
  return;
}
```

### decoy: 0x007aaea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007aaea0: 16-byte aligned: YES
  007aaea0  55             PUSH EBP
  007aaea1  8B EC          MOV EBP,ESP
  007aaea3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007aaea0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x280 [dec 640]  x1  via EAX
```

## Card B0-2-11

GOG function: CSWCObject::SetLoopingAnimation  gog 0x00472ab0  callconv=(blank)  param_size_bytes=2  gog_size=42  gog_retn=4
Candidate Steam address: 0x007ed950 (size 42, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed9a0:STR0/CALLEE1/SIG0.021/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472a70 CSWCObject::GetAnimationLength  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00472ae0 CSWCObject::GetLoopingAnimation -> 0x007ed980  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00474660 CSWCObject::AnimationLooping -> Steam 0x007ef500  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x58*} | Steam {0x58*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ed950: 16-byte aligned: YES
  007ed950  55             PUSH EBP
  007ed951  8B EC          MOV EBP,ESP
  007ed953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed950
string literals (0):
direct callees (1):
  0x007EF500  CSWCObject::AnimationLooping
struct offsets touched (top 1):
  +0x58 [dec 88]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed950 FUN_007ed950 size=42 ret=4 callers=6 callees=1

void __thiscall FUN_007ed950(int param_1,undefined2 param_2)

{
  int iVar1;
  
  iVar1 = FUN_007ef500(param_2);
  if (iVar1 == 1) {
    *(undefined2 *)(param_1 + 0x58) = param_2;
  }
  return;
}
```

### decoy: 0x007ed9a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed9a0: 16-byte aligned: YES
  007ed9a0  55             PUSH EBP
  007ed9a1  8B EC          MOV EBP,ESP
  007ed9a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007ed9a0
string literals (3):
  0x009A2384  at 007ee5b8  'DeathVFX'
  0x009A2374  at 007ee68f  'DeathVFXNode'
  0x009914B8  at 007ee736  'impact'
direct callees (27):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0073F750  ClientApp::GetGuiInGame
  0x007EF730  CSWCObject::AnimationPlayOutOfPlace
  0x00774B40  CSWCCreature::StopAmbientPauseAnim
  0x007EF910  CSWCObject::AnimationHideEquippedItems
  0x007107F0  (unnamed)
  0x00773A10  CSWCCreature::HideEquippedItems
  0x007EEEE0  (unnamed)
  0x007759B0  CSWCCreature::GetCameraFollowPart
  0x007EF500  CSWCObject::AnimationLooping
  0x007ED950  CSWCObject::SetLoopingAnimation
  0x007F00B0  CSWCObject::RemoveTopFireForgetAnimation
  0x007F0300  CSWCObject::AbortFireForgetAnimation
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::CExoString(empty)
  0x0071B1D0  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00734010  CExoString::CStr
struct offsets touched (top 10):
  +0x68 [dec 104]  x47  via EAX,ECX,EDX
  +0x58 [dec 88]  x37  via EAX,ECX,EDX
  +0x48 [dec 72]  x17  via EAX,ECX,EDX
  +0x2C [dec 44]  x13  via EAX,EDX
  +0x44 [dec 68]  x9  via EAX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x50 [dec 80]  x4  via EAX,ECX,EDX
  +0x54 [dec 84]  x4  via ECX,EDX
  +0xC6 [dec 198]  x4  via EAX
```

## Card B0-2-12

GOG function: CSWCObject::AnimationParry  gog 0x00474950  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef7f0 (size 129, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef880:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004748e0 CSWCObject::AnimationDialog -> 0x007ef780  gate_class=AUTO
- GOG-order neighbour after:  0x004749e0 CSWCObject::AnimationAttack -> 0x007ef880  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xcc*,0xe0*,0x130*} | Steam {0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef7f0: 16-byte aligned: YES
  007ef7f0  55             PUSH EBP
  007ef7f1  8B EC          MOV EBP,ESP
  007ef7f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef7f0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef7f0 FUN_007ef7f0 size=129 ret=4 callers=2 callees=1

bool __thiscall FUN_007ef7f0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe3c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef880: 16-byte aligned: YES
  007ef880  55             PUSH EBP
  007ef881  8B EC          MOV EBP,ESP
  007ef883  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef880
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-2-13

GOG function: CGuiInGame::HideGalaxyMapGui  gog 0x004db020  callconv=(blank)  param_size_bytes=0  gog_size=224  gog_retn=None
Candidate Steam address: 0x007ca920 (size 239, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cab00:STR0/CALLEE0/SIG0.4/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dae30 CGuiInGame::ShowGalaxyMapGui -> 0x007ca730  gate_class=AUTO
- GOG-order neighbour after:  0x004db100 CGuiInGame::ShowDebugMenu  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d7f0 CClientExoApp::GetPausedByCombat -> Steam ?  in candidate callees: n/a
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam ?  in candidate callees: n/a
  0x004da340 CGuiInGame::SetSWGuiStatus -> Steam ?  in candidate callees: n/a
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x38*,0x80*,0x128*} | Steam {0x4*,0x38*,0x80*,0x128*}
imm sets (* = in both, 3 common): GOG {0x4*,0x5*,0x1c*} | Steam {0x4*,0x5*,0x1c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ca920: 16-byte aligned: YES
  007ca920  55             PUSH EBP
  007ca921  8B EC          MOV EBP,ESP
  007ca923  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ca920
string literals (0):
direct callees (7):
  0x00740330  CClientExoApp::GetPausedByCombat
  0x00740350  CClientExoApp::SetPausedByCombat
  0x007C9C40  CGuiInGame::SetSWGuiStatus
  0x0073FEE0  CClientExoApp::SetInputClass
  0x00410670  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0070BC60  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x80 [dec 128]  x3  via ECX
  +0x38 [dec 56]  x2  via EAX,EDX
  +0x128 [dec 296]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ca920 FUN_007ca920 size=239 ret=0 callers=1 callees=7

void __fastcall FUN_007ca920(int param_1)

{
  int iVar1;
  
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

### decoy: 0x007cab00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cab00: 16-byte aligned: YES
  007cab00  55             PUSH EBP
  007cab01  8B EC          MOV EBP,ESP
  007cab03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cab00
string literals (0):
direct callees (3):
  0x008BF280  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x0073FEE0  CClientExoApp::SetInputClass
struct offsets touched (top 4):
  +0x5C [dec 92]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x38 [dec 56]  x1  via EDX
  +0x128 [dec 296]  x1  via EAX
```

## Card B0-2-14

GOG function: CSWSEffectListHandler::OnRemoveSetStateInternal  gog 0x00741150  callconv=(blank)  param_size_bytes=8  gog_size=234  gog_retn=8
Candidate Steam address: 0x0059ea10 (size 353, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnRemoveMovementSpeedIncrease
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059e970:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00740720 CSWSEffectListHandler::OnApplySetStateInternal -> 0x0059de30  gate_class=REVIEW
- GOG-order neighbour after:  0x00741260 CSWSEffectListHandler::OnApplyMovementSpeedIncrease -> 0x0059e970  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640af0 CVirtualMachine::RunScript -> Steam ?  in candidate callees: n/a
  0x006934c0 CSWSCreature::RecomputeAmbientAnimationState -> Steam ?  in candidate callees: n/a
  0x006dbb50 CSWSCreature::UpdateVisibleList -> Steam ?  in candidate callees: n/a
  0x006dc130 CSWSCreature::ClearVisibleList -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x30*,0x270,0x10e8} | Steam {0x4*,0x8,0x30*}
imm sets (* = in both, 1 common): GOG {0x3,0x6,0x8*,0xe,0x12,0x18} | Steam {0x8*,0x1c,0x1d,0x44,0x148}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059ea10: 16-byte aligned: YES
  0059ea10  55             PUSH EBP
  0059ea11  8B EC          MOV EBP,ESP
  0059ea13  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059ea10
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EDX
  +0x8 [dec 8]  x3  via ECX
  +0x30 [dec 48]  x3  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059ea10 FUN_0059ea10 size=353 ret=8 callers=0 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0059ea10(int *param_1,int *param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int local_1c;
  float local_10;
  
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 != 0) {
    (**(code **)(*param_1 + 0x30))();
    if (param_2 != (int *)0x0) {
      local_10 = 1.0;
      iVar2 = (**(code **)(*param_1 + 0x30))();
      local_1c = 0;
      while( true ) {
        if (*(int *)(iVar2 + 0x14c) <= local_1c) break;
        if (0x1d < *(ushort *)(*(int *)(*(int *)(iVar2 + 0x148) + local_1c * 4) + 8)) break;
        piVar1 = *(int **)(*(int *)(iVar2 + 0x148) + local_1c * 4);
        if ((*piVar1 != *param_2) || (piVar1[1 = byte +0x4] != param_2[1 = byte +0x4])) {
          if ((short)piVar1[2 = byte +0x8] == 0x1c) {
            iVar3 = FUN_005e4e60(0);
            local_10 = (float)iVar3 / (float)_DAT_0098ba38 + local_10;
          }
          else if ((short)piVar1[2 = byte +0x8] == 0x1d) {
            iVar3 = FUN_005e4e60(0);
            local_10 = local_10 - (float)iVar3 / (float)_DAT_0098ba38;
          }
        }
        local_1c = local_1c + 1;
      }
      FUN_005766a0(local_10);
    }
  }
  return 1;
}
```

### decoy: 0x0059e970 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059e970: 16-byte aligned: YES
  0059e970  55             PUSH EBP
  0059e971  8B EC          MOV EBP,ESP
  0059e973  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059e970
string literals (0):
direct callees (4):
  0x005E4E60  CGameEffect::GetInteger
  0x005E4E80  CGameEffect::SetInteger
  0x00576610  CSWSCreature::GetMovementRateFactor
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 1):
  +0x30 [dec 48]  x2  via EDX
```

## Card B0-2-15

GOG function: CSWCObject::AnimationRunning  gog 0x00474a70  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef470 (size 129, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef3e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004749e0 CSWCObject::AnimationAttack -> 0x007ef880  gate_class=REVIEW
- GOG-order neighbour after:  0x00474b00 CSWCObject::AddFireForgetAnimation  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xcc*,0xe0*,0x130*} | Steam {0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef470: 16-byte aligned: YES
  007ef470  55             PUSH EBP
  007ef471  8B EC          MOV EBP,ESP
  007ef473  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef470
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef470 FUN_007ef470 size=129 ret=4 callers=1 callees=1

bool __thiscall FUN_007ef470(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe34,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef3e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef3e0: 16-byte aligned: YES
  007ef3e0  55             PUSH EBP
  007ef3e1  8B EC          MOV EBP,ESP
  007ef3e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef3e0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-2-16

GOG function: CGuiInGame::AddFloatyText  gog 0x004df580  callconv=(blank)  param_size_bytes=16  gog_size=58  gog_retn=16
Candidate Steam address: 0x007cee80 (size 58, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ceec0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004df550 CGuiInGame::GetGuiInGameScreenPanel  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004df5c0 CGuiInGame::InitializeMap  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005226b0 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x98*} | Steam {0x98*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x007cee80: 16-byte aligned: YES
  007cee80  55             PUSH EBP
  007cee81  8B EC          MOV EBP,ESP
  007cee83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cee80
string literals (0):
direct callees (1):
  0x00750F30  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007cee80 FUN_007cee80 size=58 ret=16 callers=1 callees=1

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

### decoy: 0x007ceec0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ceec0: 16-byte aligned: YES
  007ceec0  55             PUSH EBP
  007ceec1  8B EC          MOV EBP,ESP
  007ceec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ceec0
string literals (0):
direct callees (1):
  0x00750C10  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x2  via EAX,ECX
```

## Card B0-2-17

GOG function: CSWCObject::AnimationWalking  gog 0x004745d0  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef3e0 (size 129, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef350:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474540 CSWCObject::AnimationHideEquippedItems -> 0x007ef910  gate_class=REVIEW
- GOG-order neighbour after:  0x00474660 CSWCObject::AnimationLooping -> 0x007ef500  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xcc*,0xe0*,0x130*} | Steam {0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef3e0: 16-byte aligned: YES
  007ef3e0  55             PUSH EBP
  007ef3e1  8B EC          MOV EBP,ESP
  007ef3e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef3e0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef3e0 FUN_007ef3e0 size=129 ret=4 callers=1 callees=1

bool __thiscall FUN_007ef3e0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe30,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef350: 16-byte aligned: YES
  007ef350  55             PUSH EBP
  007ef351  8B EC          MOV EBP,ESP
  007ef353  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef350
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-2-18

GOG function: CSWCObject::GetLoopingAnimation  gog 0x00472ae0  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x007ed980 (size 18, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed950:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472ab0 CSWCObject::SetLoopingAnimation -> 0x007ed950  gate_class=REVIEW
- GOG-order neighbour after:  0x00472b00 CSWCObject::SetAnimation  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x58*} | Steam {0x58*}
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
prologue for 0x007ed980: 16-byte aligned: YES
  007ed980  55             PUSH EBP
  007ed981  8B EC          MOV EBP,ESP
  007ed983  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed980
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x58 [dec 88]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed980 FUN_007ed980 size=18 ret=0 callers=9 callees=0

undefined2 __fastcall FUN_007ed980(int param_1)

{
  return *(undefined2 *)(param_1 + 0x58);
}
```

### decoy: 0x007ed950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed950: 16-byte aligned: YES
  007ed950  55             PUSH EBP
  007ed951  8B EC          MOV EBP,ESP
  007ed953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed950
string literals (0):
direct callees (1):
  0x007EF500  CSWCObject::AnimationLooping
struct offsets touched (top 1):
  +0x58 [dec 88]  x1  via ECX
```

## Card B0-2-19

GOG function: CSWCObject::DetachFromServerObject  gog 0x004777d0  callconv=(blank)  param_size_bytes=0  gog_size=24  gog_retn=0
Candidate Steam address: 0x007f2630 (size 24, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f2610:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004777b0 CSWCObject::AttachmentFromServerObject -> 0x007f2610  gate_class=REVIEW
- GOG-order neighbour after:  0x004777f0 CSWCObject::EnableDistortion  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xf8*} | Steam {0xf8*}
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
prologue for 0x007f2630: 16-byte aligned: YES
  007f2630  55             PUSH EBP
  007f2631  8B EC          MOV EBP,ESP
  007f2633  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2630
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF8 [dec 248]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f2630 FUN_007f2630 size=24 ret=0 callers=1 callees=0

void __fastcall FUN_007f2630(int param_1)

{
  *(undefined4 *)(param_1 + 0xf8) = 0;
  return;
}
```

### decoy: 0x007f2610 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f2610: 16-byte aligned: YES
  007f2610  55             PUSH EBP
  007f2611  8B EC          MOV EBP,ESP
  007f2613  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f2610
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF8 [dec 248]  x1  via EAX
```

## Card B0-2-20

GOG function: CGuiInGame::ShutDownGlobal  gog 0x004cdda0  callconv=(blank)  param_size_bytes=4  gog_size=173  gog_retn=4
Candidate Steam address: 0x007bdb10 (size 173, retn 4)
Candidate sources: branch+name_map:medium:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0073ffc0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004cd090 CGuiInGame::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004cde50 CGuiInGame::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x009217f0 Global::`eh_vector_destructor_iterator' -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x2*,0x4*,0x8*,0x10*} | Steam {0x2*,0x4*,0x8*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007bdb10: 16-byte aligned: YES
  007bdb10  55             PUSH EBP
  007bdb11  8B EC          MOV EBP,ESP
  007bdb13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007bdb10
string literals (0):
direct callees (3):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007bdb10 FUN_007bdb10 size=173 ret=4 callers=1 callees=3

void * __thiscall FUN_007bdb10(void *param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00951828;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((param_2 & 2) == 0) {
    local_8 = 0;
    FUN_00733780(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0xffffffff;
    FUN_00733780();
    if ((param_2 & 1) != 0) {
      FUN_009196fd(param_1);
    }
  }
  else {
    _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
              (param_1,0x10,*(int *)((int)param_1 + -4),FID_conflict__CChevronOwnerDrawMenu [GHIDRA-AUTO-LABEL: unreliable]);
    if ((param_2 & 1) != 0) {
      FUN_009196fd((int)param_1 + -4);
    }
    param_1 = (void *)((int)param_1 + -4);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0073ffc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ffc0: 16-byte aligned: YES
  0073ffc0  55             PUSH EBP
  0073ffc1  8B EC          MOV EBP,ESP
  0073ffc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ffc0
string literals (0):
direct callees (1):
  0x00798580  CClientExoAppInternal::SetPlayerCharacterName
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-2-21

GOG function: CSWCObject::SetId  gog 0x00410c30  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00765110 (size 61, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x00764f60
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007650c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c50 CSWCObject::ShowShadowBlob -> 0x00764f80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {0x310}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0xc,0x14}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00765110: 16-byte aligned: YES
  00765110  55             PUSH EBP
  00765111  8B EC          MOV EBP,ESP
  00765113  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00765110
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 1):
  +0x310 [dec 784]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00765110 FUN_00765110 size=61 ret=4 callers=0 callees=1

undefined4 __thiscall FUN_00765110(int param_1,undefined4 param_2)

{
  FUN_007335f0(*(int *)(param_1 + 0x310) + 0x14);
  return param_2;
}
```

### decoy: 0x007650c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007650c0: 16-byte aligned: YES
  007650c0  55             PUSH EBP
  007650c1  8B EC          MOV EBP,ESP
  007650c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007650c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x310 [dec 784]  x2  via EAX,EDX
  +0x124 [dec 292]  x1  via ECX
```

## Card B0-2-22

GOG function: CSWSItemPropertyHandler::RemoveEnhancementBonus  gog 0x007c2600  callconv=__thiscall  param_size_bytes=16  gog_size=1917  gog_retn=16
Candidate Steam address: 0x005aaa70 (size 88, retn 16)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::OnItemPropertyRemoved
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.11 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005aaad0:STR0/CALLEE0/SIG0.586/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c1ee0 CSWSItemPropertyHandler::ApplyEnhancementBonus -> 0x005aaad0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c2d90 CSWSItemPropertyHandler::ApplyEnhancementPenalty -> 0x005ab980  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x0066a400 CSWSItem::GetDamageFlags -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 1 common): GOG {0x2,0x4,0x6,0x8*,0xa,0x14,0x18,0x38,0x122,0x130,0x13c,0x1198} | Steam {0x8*}
imm sets (* = in both, 2 common): GOG {0x2,0x3,0x4,0x5,0x6,0x7,0x8*,0x10*,0x20,0x94,0x148,0x4000,0x8000,0x10000} | Steam {0x8*,0x10*,0x46}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005aaa70: 16-byte aligned: YES
  005aaa70  55             PUSH EBP
  005aaa71  8B EC          MOV EBP,ESP
  005aaa73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005aaa70
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005aaa70 FUN_005aaa70 size=88 ret=16 callers=0 callees=0

undefined4 __thiscall
FUN_005aaa70(int param_1,undefined4 param_2,ushort *param_3,undefined4 param_4,undefined4 param_5)

{
  ushort uVar1;
  undefined4 uVar2;
  
  uVar1 = *param_3;
  if ((uVar1 < 0x46) && (*(int *)(*(int *)(param_1 + 8) + (uint)uVar1 * 4) != 0)) {
    uVar2 = (**(code **)(*(int *)(param_1 + 8) + (uint)uVar1 * 4))(param_2,param_3,param_4,param_5);
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x005aaad0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aaad0: 16-byte aligned: YES
  005aaad0  55             PUSH EBP
  005aaad1  8B EC          MOV EBP,ESP
  005aaad3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005aaad0
string literals (1):
  0x00992E8C  at 005aabd9  'Value'
direct callees (11):
  0x006D5200  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 10):
  +0x2 [dec 2]  x8  via EAX,EDX
  +0x8 [dec 8]  x4  via ECX,EDX
  +0xA [dec 10]  x4  via ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x122 [dec 290]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x38 [dec 56]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-2-23

GOG function: CClientExoApp::EnableVideoEffect  gog 0x0040de60  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740a50 (size 28, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::ClearCachedCreatures+name_held_elsewhere=0x007409a0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740a20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040de40 CClientExoApp::SetLoadScreenHint  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040de80 CClientExoApp::DisableVideoEffect -> 0x007409c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ab50 CClientExoAppInternal::EnableVideoEffect -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740a50: 16-byte aligned: YES
  00740a50  55             PUSH EBP
  00740a51  8B EC          MOV EBP,ESP
  00740a53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740a50
string literals (0):
direct callees (1):
  0x0078CF50  CClientExoAppInternal::ClearCachedCreatures
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740a50 FUN_00740a50 size=28 ret=4 callers=1 callees=1
THUNK -> 0x0078CF50 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740a50(undefined4 param_1)

{
  FUN_0078cf50(param_1);
  return;
}
```

### decoy: 0x00740a20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740a20: 16-byte aligned: YES
  00740a20  55             PUSH EBP
  00740a21  8B EC          MOV EBP,ESP
  00740a23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740a20
string literals (0):
direct callees (1):
  0x0079E9D0  CClientExoAppInternal::ShowTutorialWindow
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-2-24

GOG function: CGuiInGame::RePopulateMainInterface  gog 0x004df250  callconv=(blank)  param_size_bytes=0  gog_size=39  gog_retn=0
Candidate Steam address: 0x007ceb50 (size 39, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ceec0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004df1f0 CGuiInGame::GetCanOrientInDialog -> 0x007ceaf0  gate_class=REVIEW
- GOG-order neighbour after:  0x004df280 CGuiInGame::SetGlobalDialogState  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0051e760 CSWGuiMainInterface::PopulateMenus -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x98*} | Steam {0x98*}
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
prologue for 0x007ceb50: 16-byte aligned: YES
  007ceb50  55             PUSH EBP
  007ceb51  8B EC          MOV EBP,ESP
  007ceb53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ceb50
string literals (0):
direct callees (1):
  0x0074CFE0  CSWGuiMainInterface::UpdateTargetMenu
struct offsets touched (top 1):
  +0x98 [dec 152]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ceb50 FUN_007ceb50 size=39 ret=0 callers=2 callees=1

void __fastcall FUN_007ceb50(int param_1)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    FUN_0074cfe0(1);
  }
  return;
}
```

### decoy: 0x007ceec0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ceec0: 16-byte aligned: YES
  007ceec0  55             PUSH EBP
  007ceec1  8B EC          MOV EBP,ESP
  007ceec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ceec0
string literals (0):
direct callees (1):
  0x00750C10  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x2  via EAX,ECX
```

## Card B0-2-25

GOG function: CSWCObject::AnimationPlayOutOfPlace  gog 0x004744b0  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef730 (size 68, retn 4)
Candidate sources: branch+name_map:medium:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef6a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474420 CSWCObject::AnimationStationary -> 0x007ef2c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00474540 CSWCObject::AnimationHideEquippedItems -> 0x007ef910  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x68,0xcc,0xe0,0x130} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x10} | Steam {0x4*,0x4b0,0x52f,0x640,0x6bf}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef730: 16-byte aligned: YES
  007ef730  55             PUSH EBP
  007ef731  8B EC          MOV EBP,ESP
  007ef733  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef730
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007ef730 FUN_007ef730 size=68 ret=4 callers=3 callees=0

undefined4 FUN_007ef730(ushort param_1)

{
  undefined4 uVar1;
  
  if (((param_1 < 0x4b0) || (0x52f < param_1)) && ((param_1 < 0x640 || (0x6bf < param_1)))) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x007ef6a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef6a0: 16-byte aligned: YES
  007ef6a0  55             PUSH EBP
  007ef6a1  8B EC          MOV EBP,ESP
  007ef6a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef6a0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-2-26

GOG function: CGuiInGame::GetCanOrientInDialog  gog 0x004df1f0  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=4
Candidate Steam address: 0x007ceaf0 (size 90, retn 4)
Candidate sources: mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x007cf710
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007ce740:STR0/CALLEE1/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004df190 CGuiInGame::GetBarkBubbleVisible -> 0x007cea90  gate_class=AUTO
- GOG-order neighbour after:  0x004df250 CGuiInGame::RePopulateMainInterface -> 0x007ceb50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x9c*} | Steam {0x4*,0x8*,0x9c*}
imm sets (* = in both, 2 common): GOG {0x4*,0x440*} | Steam {0x4*,0x440*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ceaf0: 16-byte aligned: YES
  007ceaf0  55             PUSH EBP
  007ceaf1  8B EC          MOV EBP,ESP
  007ceaf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ceaf0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x9C [dec 156]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ceaf0 FUN_007ceaf0 size=90 ret=4 callers=2 callees=0

undefined4 * __thiscall FUN_007ceaf0(int param_1,undefined4 *param_2)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x9c) == 0) {
    *param_2 = 0;
    param_2[1 = byte +0x4] = 0;
    param_2[2 = byte +0x8] = 0;
  }
  else {
    iVar1 = *(int *)(param_1 + 0x9c);
    *param_2 = *(undefined4 *)(iVar1 + 0x440);
    param_2[1 = byte +0x4] = *(undefined4 *)(iVar1 + 0x444);
    param_2[2 = byte +0x8] = *(undefined4 *)(iVar1 + 0x448);
  }
  return param_2;
}
```

### decoy: 0x007ce740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ce740: 16-byte aligned: YES
  007ce740  55             PUSH EBP
  007ce741  8B EC          MOV EBP,ESP
  007ce743  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ce740
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x1018 [dec 4120]  x1  via ECX
```

## Card B0-2-27

GOG function: CGuiInGame::GetMessageBufferSize  gog 0x004d8430  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x007c7d40 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007c7d20:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004d8410 CGuiInGame::GetMessageBuffer  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004d8450 CGuiInGame::HandleDialogSelection  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x11c*} | Steam {0x11c*}
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
prologue for 0x007c7d40: 16-byte aligned: YES
  007c7d40  55             PUSH EBP
  007c7d41  8B EC          MOV EBP,ESP
  007c7d43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007c7d40
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x11C [dec 284]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007c7d40 FUN_007c7d40 size=20 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_007c7d40(int param_1)

{
  return *(undefined4 *)(param_1 + 0x11c);
}
```

### decoy: 0x007c7d20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007c7d20: 16-byte aligned: YES
  007c7d20  55             PUSH EBP
  007c7d21  8B EC          MOV EBP,ESP
  007c7d23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007c7d20
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x110 [dec 272]  x1  via EAX
```

## Card B0-2-28

GOG function: CSWCArea::DetachFromServerArea  gog 0x004578a0  callconv=(blank)  param_size_bytes=0  gog_size=24  gog_retn=0
Candidate Steam address: 0x007aaea0 (size 24, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007aae80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00457880 CSWCArea::AttachmentFromServerArea -> 0x007aae80  gate_class=REVIEW
- GOG-order neighbour after:  0x004578c0 CSWCArea::AddCorpse -> 0x007aaec0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x280*} | Steam {0x280*}
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
prologue for 0x007aaea0: 16-byte aligned: YES
  007aaea0  55             PUSH EBP
  007aaea1  8B EC          MOV EBP,ESP
  007aaea3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007aaea0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x280 [dec 640]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007aaea0 FUN_007aaea0 size=24 ret=0 callers=1 callees=0

void __fastcall FUN_007aaea0(int param_1)

{
  *(undefined4 *)(param_1 + 0x280) = 0;
  return;
}
```

### decoy: 0x007aae80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007aae80: 16-byte aligned: YES
  007aae80  55             PUSH EBP
  007aae81  8B EC          MOV EBP,ESP
  007aae83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007aae80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x280 [dec 640]  x1  via EAX
```

## Card B0-2-29

GOG function: CSWCObject::AttachmentFromServerObject  gog 0x004777b0  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x007f2610 (size 25, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (GetClientObject 0x54a840)
- best decoy (neighbour/other candidate) scores: 0x007f2630:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004776e0 CSWCObject::GetServerObject -> 0x007f2540  gate_class=AUTO
- GOG-order neighbour after:  0x004777d0 CSWCObject::DetachFromServerObject -> 0x007f2630  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xf8*} | Steam {0xf8*}
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
prologue for 0x007f2610: 16-byte aligned: YES
  007f2610  55             PUSH EBP
  007f2611  8B EC          MOV EBP,ESP
  007f2613  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2610
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF8 [dec 248]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f2610 FUN_007f2610 size=25 ret=4 callers=1 callees=0

void __thiscall FUN_007f2610(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xf8) = param_2;
  return;
}
```

### decoy: 0x007f2630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f2630: 16-byte aligned: YES
  007f2630  55             PUSH EBP
  007f2631  8B EC          MOV EBP,ESP
  007f2633  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f2630
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF8 [dec 248]  x1  via EAX
```

## Card B0-2-30

GOG function: CSWGuiControl::SetActive  gog 0x004ce5d0  callconv=__thiscall  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x004185d0 (size 36, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00418570:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ce5a0 CSWGuiObject::`scalar  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ce600 CSWGuiControl::Destructor_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x48*} | Steam {0x48*}
imm sets (* = in both, 2 common): GOG {0x4*,0xfffffffe*} | Steam {0x4*,0xfffffffe*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004185d0: 16-byte aligned: YES
  004185d0  55             PUSH EBP
  004185d1  8B EC          MOV EBP,ESP
  004185d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004185d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x48 [dec 72]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004185d0 FUN_004185d0 size=36 ret=4 callers=0 callees=0

void __thiscall FUN_004185d0(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffffe | param_2 & 1;
  return;
}
```

### decoy: 0x00418570 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00418570: 16-byte aligned: YES
  00418570  55             PUSH EBP
  00418571  8B EC          MOV EBP,ESP
  00418573  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00418570
string literals (0):
direct callees (1):
  0x00418990  CSWGuiControl::Constructor
struct offsets touched (top 1):
  +0x48 [dec 72]  x2  via EAX,ECX
```

## Card B0-2-31

GOG function: CSWCObject::SetId  gog 0x00410c30  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00764420 (size 279, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x00764f60
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.28 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00764540:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c50 CSWCObject::ShowShadowBlob -> 0x00764f80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x1,0x4*,0x8,0xa,0xc,0x10,0x14,0x18,0x1a,0x1c,0x1e,0x20,0x24,0x28,...(+4)}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x2c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00764420: 16-byte aligned: YES
  00764420  55             PUSH EBP
  00764421  8B EC          MOV EBP,ESP
  00764423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00764420
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x1 [dec 1]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x10 [dec 16]  x2  via EAX,ECX
  +0x14 [dec 20]  x2  via EAX,ECX
  +0x18 [dec 24]  x2  via EAX,ECX
  +0x1A [dec 26]  x2  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00764420 FUN_00764420 size=279 ret=4 callers=1 callees=0

void __thiscall FUN_00764420(undefined1 *param_1,undefined1 *param_2)

{
  *param_1 = *param_2;
  param_1[1] = param_2[1];
  *(undefined4 *)(param_1 + 4) = *(undefined4 *)(param_2 + 4);
  *(undefined4 *)(param_1 + 0x14) = *(undefined4 *)(param_2 + 0x14);
  *(undefined2 *)(param_1 + 8) = *(undefined2 *)(param_2 + 8);
  *(undefined2 *)(param_1 + 0xa [dec 10]) = *(undefined2 *)(param_2 + 0xa [dec 10]);
  param_1[0xc] = param_2[0xc];
  *(undefined4 *)(param_1 + 0x10) = *(undefined4 *)(param_2 + 0x10);
  *(undefined2 *)(param_1 + 0x1e) = *(undefined2 *)(param_2 + 0x1e);
  *(undefined2 *)(param_1 + 0x18) = *(undefined2 *)(param_2 + 0x18);
  *(undefined2 *)(param_1 + 0x1a) = *(undefined2 *)(param_2 + 0x1a);
  *(undefined2 *)(param_1 + 0x1c) = *(undefined2 *)(param_2 + 0x1c);
  *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_2 + 0x2c);
  *(undefined4 *)(param_1 + 0x30) = *(undefined4 *)(param_2 + 0x30);
  *(undefined4 *)(param_1 + 0x34) = *(undefined4 *)(param_2 + 0x34);
  *(undefined4 *)(param_1 + 0x38) = *(undefined4 *)(param_2 + 0x38);
  *(undefined4 *)(param_1 + 0x3c) = *(undefined4 *)(param_2 + 0x3c);
  *(undefined4 *)(param_1 + 0x44) = *(undefined4 *)(param_2 + 0x44);
  *(undefined4 *)(param_1 + 0x20) = *(undefined4 *)(param_2 + 0x20);
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_2 + 0x24);
  param_1[0x28] = param_2[0x28];
  *(undefined4 *)(param_1 + 0x40) = *(undefined4 *)(param_2 + 0x40);
  return;
}
```

### decoy: 0x00764540 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764540: 16-byte aligned: YES
  00764540  55             PUSH EBP
  00764541  8B EC          MOV EBP,ESP
  00764543  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00764540
string literals (0):
direct callees (2):
  0x00733540  CExoString::CExoString(empty)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
```

## Card B0-2-32

GOG function: CSWCArea::ClearAllMapPins  gog 0x00457310  callconv=(blank)  param_size_bytes=0  gog_size=43  gog_retn=0
Candidate Steam address: 0x007aa910 (size 43, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007aa7f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004571f0 CSWCArea::GetIsDark -> 0x007aa7f0  gate_class=AUTO
- GOG-order neighbour after:  0x00457340 CSWCArea::RestartMiniGame -> 0x007aa940  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00457b00 CExoArrayList_Vector_::Allocate -> Steam ?  in candidate callees: n/a
  0x00457b60 CExoArrayList_CSWCMapPin__::Allocate -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x23c*,0x248*} | Steam {0x23c*,0x248*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007aa910: 16-byte aligned: YES
  007aa910  55             PUSH EBP
  007aa911  8B EC          MOV EBP,ESP
  007aa913  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007aa910
string literals (0):
direct callees (2):
  0x00474030  (unnamed)
  0x004292D0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007aa910 FUN_007aa910 size=43 ret=0 callers=1 callees=2

void FUN_007aa910(void)

{
  FUN_00474030(0);
  FUN_004292d0(0);
  return;
}
```

### decoy: 0x007aa7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007aa7f0: 16-byte aligned: YES
  007aa7f0  55             PUSH EBP
  007aa7f1  8B EC          MOV EBP,ESP
  007aa7f3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007aa7f0
string literals (0):
direct callees (3):
  0x0073F450  CClientExoApp::GetControlledObject
  0x00774A80  CSWCCreature::GetCreatureLight
  0x0044B8A0  (unnamed)
struct offsets touched (top 3):
  +0x238 [dec 568]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x188 [dec 392]  x1  via EAX
```

## Card B0-2-33

GOG function: CSWCObject::AnimationPause  gog 0x00474890  callconv=(blank)  param_size_bytes=2  gog_size=68  gog_retn=4
Candidate Steam address: 0x007ef730 (size 68, retn 4)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCObject::AnimationPlayOutOfPlace+name_held_elsewhere=0x007ef350
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007ef6a0:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474800 CSWCObject::AnimationOverlay -> 0x007ef6a0  gate_class=AUTO
- GOG-order neighbour after:  0x004748e0 CSWCObject::AnimationDialog -> 0x007ef780  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x4b0*,0x52f*,0x640*,0x6bf*} | Steam {0x4*,0x4b0*,0x52f*,0x640*,0x6bf*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef730: 16-byte aligned: YES
  007ef730  55             PUSH EBP
  007ef731  8B EC          MOV EBP,ESP
  007ef733  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef730
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007ef730 FUN_007ef730 size=68 ret=4 callers=3 callees=0

undefined4 FUN_007ef730(ushort param_1)

{
  undefined4 uVar1;
  
  if (((param_1 < 0x4b0) || (0x52f < param_1)) && ((param_1 < 0x640 || (0x6bf < param_1)))) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x007ef6a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef6a0: 16-byte aligned: YES
  007ef6a0  55             PUSH EBP
  007ef6a1  8B EC          MOV EBP,ESP
  007ef6a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef6a0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-2-34

GOG function: CSWCObject::GetPortrait  gog 0x004725e0  callconv=(blank)  param_size_bytes=5  gog_size=39  gog_retn=8
Candidate Steam address: 0x007ed480 (size 39, retn 8)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed850:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004725b0 CSWCObject::GetCameraFollowPart  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00472610 CSWCObject::GetPortraitId -> 0x007ed4b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00495600 CSWPortrait::GetPortrait -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xa8*} | Steam {0x8*,0xa8*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007ed480: 16-byte aligned: YES
  007ed480  55             PUSH EBP
  007ed481  8B EC          MOV EBP,ESP
  007ed483  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed480
string literals (0):
direct callees (1):
  0x0085E8C0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007ed480 FUN_007ed480 size=39 ret=8 callers=0 callees=1

undefined4 FUN_007ed480(undefined4 param_1,undefined1 param_2)

{
  FUN_0085e8c0(param_1,param_2);
  return param_1;
}
```

### decoy: 0x007ed850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed850: 16-byte aligned: YES
  007ed850  55             PUSH EBP
  007ed851  8B EC          MOV EBP,ESP
  007ed853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed850
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-2-35

GOG function: CGuiInGame::IsFadingInDialog  gog 0x004dbe50  callconv=(blank)  param_size_bytes=0  gog_size=43  gog_retn=0
Candidate Steam address: 0x007cb720 (size 44, retn 0)
Candidate sources: branch+name_map:medium:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb750:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbe20 CGuiInGame::IsLoopingDialogStuntAnimation_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004dbe80 CGuiInGame::IsInDialog  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x3c,0x40} | Steam {0x68}
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
prologue for 0x007cb720: 16-byte aligned: YES
  007cb720  55             PUSH EBP
  007cb721  8B EC          MOV EBP,ESP
  007cb723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb720
string literals (0):
direct callees (1):
  0x008BB200  (unnamed)
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007cb720 FUN_007cb720 size=44 ret=0 callers=1 callees=1

undefined4 __fastcall FUN_007cb720(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 0x68) != 0) && (iVar1 = FUN_008bb200(), iVar1 == 0)) {
    return 1;
  }
  return 0;
}
```

### decoy: 0x007cb750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb750: 16-byte aligned: YES
  007cb750  55             PUSH EBP
  007cb751  8B EC          MOV EBP,ESP
  007cb753  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x40 [dec 64]  x1  via EDX
```

## Card B0-2-36

GOG function: CClientOptions::SetCameraMode  gog 0x00462bc0  callconv=(blank)  param_size_bytes=1  gog_size=64  gog_retn=4
Candidate Steam address: 0x007b5e50 (size 64, retn 4)
Candidate sources: branch+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007b5cc0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00462aa0 CClientOptions::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00462c00 CClientOptions::SetFullScreenEnabled  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x75*,0x76*} | Steam {0x75*,0x76*}
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
prologue for 0x007b5e50: 16-byte aligned: YES
  007b5e50  55             PUSH EBP
  007b5e51  8B EC          MOV EBP,ESP
  007b5e53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007b5e50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x75 [dec 117]  x1  via EAX
  +0x76 [dec 118]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007b5e50 FUN_007b5e50 size=64 ret=4 callers=8 callees=0

void __thiscall FUN_007b5e50(int param_1,char param_2)

{
  *(char *)(param_1 + 0x75) = param_2;
  if ((param_2 == '\x01') || (param_2 == '\0')) {
    *(char *)(param_1 + 0x76) = param_2;
  }
  if (param_2 == '\x04') {
    DAT_00a7bac4 = 0;
  }
  return;
}
```

### decoy: 0x007b5cc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b5cc0: 16-byte aligned: YES
  007b5cc0  55             PUSH EBP
  007b5cc1  8B EC          MOV EBP,ESP
  007b5cc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b5cc0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
```

## Card B0-2-37

GOG function: Gob::SetProcessFlag  gog 0x0084bb30  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00461df0 (size 217, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00461d70:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bb00 CAurPathInfo::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084bb80 Global::return_void_0x4  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00855f80 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x98,0x190,0x192,0x193,0x194,0x1ab,0x1d0}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00461df0: 16-byte aligned: YES
  00461df0  55             PUSH EBP
  00461df1  8B EC          MOV EBP,ESP
  00461df3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00461df0
string literals (0):
direct callees (4):
  0x00461B70  (unnamed)
  0x00461CF0  (unnamed)
  0x00461D70  (unnamed)
  0x00461C60  (unnamed)
struct offsets touched (top 7):
  +0x98 [dec 152]  x4  via EAX,ECX,EDX
  +0x190 [dec 400]  x3  via EAX,ECX,EDX
  +0x194 [dec 404]  x3  via EAX,EDX
  +0x193 [dec 403]  x2  via EAX
  +0x192 [dec 402]  x1  via ECX
  +0x1AB [dec 427]  x1  via ECX
  +0x1D0 [dec 464]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00461df0 FUN_00461df0 size=217 ret=0 callers=1 callees=4

void __fastcall FUN_00461df0(int *param_1)

{
  undefined1 uVar1;
  
  if (*(char *)((int)param_1 + 0x193) != '\0') {
    param_1[0x65 = byte +0x194] = 0;
    uVar1 = FUN_00461b70(param_1[0x26 = byte +0x98]);
    *(undefined1 *)(param_1 + 0x64 [dec 100] {int*: byte +0x190}) = uVar1;
    if ((char)param_1[100 = byte +0x190] == '\0') {
      param_1[0x65 = byte +0x194] = 0;
    }
    if (param_1[0x65 = byte +0x194] == 0) {
      *(undefined1 *)(param_1 + 0x64 [dec 100] {int*: byte +0x190}) = 0;
    }
    uVar1 = FUN_00461cf0(param_1[0x26 = byte +0x98]);
    *(undefined1 *)((int)param_1 + 0x192) = uVar1;
    FUN_00461d70(param_1[0x26 = byte +0x98]);
    uVar1 = FUN_00461c60(param_1[0x26 = byte +0x98]);
    *(undefined1 *)((int)param_1 + 0x1ab) = uVar1;
    (**(code **)(*param_1 + 0x1d0))();
    *(undefined1 *)((int)param_1 + 0x193) = 0;
  }
  return;
}
```

### decoy: 0x00461d70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00461d70: 16-byte aligned: YES
  00461d70  55             PUSH EBP
  00461d71  8B EC          MOV EBP,ESP
  00461d73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00461d70
string literals (0):
direct callees (1):
  0x00461D70  (unnamed)
struct offsets touched (top 3):
  +0x2C [dec 44]  x5  via EAX,ECX,EDX
  +0x30 [dec 48]  x1  via ECX
  +0x34 [dec 52]  x1  via EDX
```

## Card B0-2-38

GOG function: CSWGuiControl::Destructor  gog 0x004ce500  callconv=__thiscall  param_size_bytes=0  gog_size=126  gog_retn=0
Candidate Steam address: 0x00418640 (size 126, retn 0)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00418600:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ce3f0 CSWGuiMessageBox::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ce580 CGuiInGame::ClearMapDisplays  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x2*,0x1c*,0x2c*,0x3c*} | Steam {0x2*,0x1c*,0x2c*,0x3c*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00418640: 16-byte aligned: YES
  00418640  55             PUSH EBP
  00418641  8B EC          MOV EBP,ESP
  00418643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00418640
string literals (0):
direct callees (2):
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00418640 FUN_00418640 size=126 ret=0 callers=432 callees=2

void __fastcall FUN_00418640(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0094892e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiControl::vftable;
  local_8 = 2;
  Free(uVar1);
  local_8._0_1_ = 1;
  FUN_00733780();
  local_8 = (uint)local_8._1_3_ << 8;
  Free();
  *param_1 = CSWGuiObject::vftable;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00418600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00418600: 16-byte aligned: YES
  00418600  55             PUSH EBP
  00418601  8B EC          MOV EBP,ESP
  00418603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00418600
string literals (0):
direct callees (2):
  0x00418640  CSWGuiControl::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-2-39

GOG function: CGuiInGame::HideLoadModuleDebugMenu  gog 0x004dbba0  callconv=(blank)  param_size_bytes=0  gog_size=149  gog_retn=0
Candidate Steam address: 0x007cb4a0 (size 149, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cb540:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbaf0 CGuiInGame::HideSubItemCreateMenu_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004dbc40 CGuiInGame::HidePowersFeatsSkillsDebugMenu -> 0x007cb540  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d580 CClientExoApp::GetMiniGame -> Steam ?  in candidate callees: n/a
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
  0x0090ee40 CSWGuiManager::PopModalPanel -> Steam 0x004109c0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x38*,0x70*,0x128*} | Steam {0x4*,0x38*,0x70*,0x128*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007cb4a0: 16-byte aligned: YES
  007cb4a0  55             PUSH EBP
  007cb4a1  8B EC          MOV EBP,ESP
  007cb4a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb4a0
string literals (0):
direct callees (5):
  0x00410800  GuiManager::Contains
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00410670  (unnamed)
  0x007400A0  CClientExoApp::GetMiniGame
  0x0073FEE0  CClientExoApp::SetInputClass
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x38 [dec 56]  x3  via EAX,ECX
  +0x70 [dec 112]  x2  via ECX,EDX
  +0x128 [dec 296]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007cb4a0 FUN_007cb4a0 size=149 ret=0 callers=2 callees=5

void __fastcall FUN_007cb4a0(int param_1)

{
  int iVar1;
  
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

### decoy: 0x007cb540 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb540: 16-byte aligned: YES
  007cb540  55             PUSH EBP
  007cb541  8B EC          MOV EBP,ESP
  007cb543  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb540
string literals (0):
direct callees (5):
  0x00410800  GuiManager::Contains
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00410670  (unnamed)
  0x007400A0  CClientExoApp::GetMiniGame
  0x0073FEE0  CClientExoApp::SetInputClass
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x38 [dec 56]  x3  via EAX,ECX
  +0x74 [dec 116]  x2  via ECX,EDX
  +0x128 [dec 296]  x1  via EAX
```

## Card B0-2-40

GOG function: CGuiInGame::PrevSWInGameGui  gog 0x004daac0  callconv=(blank)  param_size_bytes=0  gog_size=389  gog_retn=0
Candidate Steam address: 0x007ca3c0 (size 389, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007ca230:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004da930 CGuiInGame::NextSWInGameGui -> 0x007ca230  gate_class=REVIEW
- GOG-order neighbour after:  0x004dac50 CGuiInGame::SwitchToSWInGameGui -> 0x007ca550  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam ?  in candidate callees: n/a
  0x004e0e60 CGuiInGame::UpdateCreatedInGameGUI -> Steam ?  in candidate callees: n/a
  0x00527f60 CSWGuiInGameMenu::SetActiveControlID -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00640af0 CVirtualMachine::RunScript -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'k_pend_screenchg'
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x2c*,0x30*,0x128*} | Steam {0x4*,0x8*,0xc*,0x2c*,0x30*,0x128*}
imm sets (* = in both, 4 common): GOG {0x2*,0x7*,0x1c*,0x7f000000*} | Steam {0x2*,0x7*,0x1c*,0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ca3c0: 16-byte aligned: YES
  007ca3c0  55             PUSH EBP
  007ca3c1  8B EC          MOV EBP,ESP
  007ca3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007ca3c0
string literals (1):
  0x009A175C  at 007ca4fd  'k_pend_screenchg'
direct callees (9):
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410670  (unnamed)
  0x007D0760  CGuiInGame::UpdateCreatedInGameGUI
  0x00410530  CSWGuiManager::AddPanel
  0x00410780  (unnamed)
  0x00756860  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x006FD8D0  CVirtualMachine::RunScript
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x2C [dec 44]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x128 [dec 296]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ca3c0 FUN_007ca3c0 size=389 ret=0 callers=1 callees=9

void __fastcall FUN_007ca3c0(int param_1)

{
  undefined4 local_2c;
  undefined1 local_20 [8];
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
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

### decoy: 0x007ca230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ca230: 16-byte aligned: YES
  007ca230  55             PUSH EBP
  007ca231  8B EC          MOV EBP,ESP
  007ca233  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007ca230
string literals (1):
  0x009A175C  at 007ca36d  'k_pend_screenchg'
direct callees (9):
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410670  (unnamed)
  0x007D0760  CGuiInGame::UpdateCreatedInGameGUI
  0x00410530  CSWGuiManager::AddPanel
  0x00410780  (unnamed)
  0x00756860  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x006FD8D0  CVirtualMachine::RunScript
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x2C [dec 44]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x128 [dec 296]  x1  via EAX
```

## Card B0-2-41

GOG function: CTwoDimArrays::Load2DArrays  gog 0x004805b0  callconv=(blank)  param_size_bytes=0  gog_size=1617  gog_retn=0
Candidate Steam address: 0x006e7610 (size 1617, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_2da_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006e7c70:STR0/CALLEE0/SIG0.017/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00480590 CTwoDimArrays::Destructor -> 0x006e75f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00480c10 CTwoDimArrays::Load2DArrays_Appearance -> 0x006e7c70  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (85), mapped to Steam through trusted rows only:
  0x00480c10 CTwoDimArrays::Load2DArrays_Appearance -> Steam ?  in candidate callees: n/a
  0x00481510 CTwoDimArrays::Load2DArrays_AppearanceSounds -> Steam ?  in candidate callees: n/a
  0x00481740 CTwoDimArrays::Load2DArrays_Gender -> Steam ?  in candidate callees: n/a
  0x00481830 CTwoDimArrays::Load2DArrays_SurfaceMaterial -> Steam ?  in candidate callees: n/a
  0x00481920 CTwoDimArrays::Load2DArrays_VisualEffect -> Steam ?  in candidate callees: n/a
  0x00481d80 CTwoDimArrays::Load2DArrays_PersistentVisualEffect -> Steam ?  in candidate callees: n/a
  0x004824f0 CTwoDimArrays::Load2DArrays_CreatureSpeed -> Steam ?  in candidate callees: n/a
  0x00482620 CTwoDimArrays::Load2DArrays_DoorTypes -> Steam ?  in candidate callees: n/a
  ... 77 more GOG callees
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
prologue for 0x006e7610: 16-byte aligned: YES
  006e7610  55             PUSH EBP
  006e7611  8B EC          MOV EBP,ESP
  006e7613  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006e7610
string literals (0):
direct callees (85):
  0x006EFE20  CTwoDimArrays::LoadIPRPCostTables
  0x006F01E0  CTwoDimArrays::LoadIPRPParamTables
  0x006E7C70  CTwoDimArrays::Load2DArrays_Appearance
  0x006E87A0  CTwoDimArrays::Load2DArrays_Gender
  0x006E8890  CTwoDimArrays::Load2DArrays_SurfaceMaterial
  0x006E8980  CTwoDimArrays::Load2DArrays_VisualEffect
  0x006E8DE0  CTwoDimArrays::Load2DArrays_PersistentVisualEffect
  0x006E9550  CTwoDimArrays::Load2DArrays_CreatureSpeed
  0x006E9680  CTwoDimArrays::Load2DArrays_DoorTypes
  0x006E9770  CTwoDimArrays::Load2DArrays_GenericDoors
  0x006E9860  CTwoDimArrays::Load2DArrays_Placeables
  0x006E9BC0  CTwoDimArrays::Load2DArrays_AreaTransition
  0x006E9C70  CTwoDimArrays::Load2DArrays_IPRPMelee
  0x006E9D20  CTwoDimArrays::Load2DArrays_ItemPropDef
  0x006E9DD0  CTwoDimArrays::Load2DArrays_Portrait
  0x006E9E80  CTwoDimArrays::Load2DArrays_IPRPDamage
  0x006E9F30  CTwoDimArrays::Load2DArrays_Traps
  0x006E9FE0  CTwoDimArrays::Load2DArrays_LightColor
  0x006EA090  CTwoDimArrays::Load2DArrays_Cursor
  0x006EA140  CTwoDimArrays::Load2DArrays_AmbientMusic
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006e7610 FUN_006e7610 size=1617 ret=0 callers=1 callees=85

undefined4 __fastcall FUN_006e7610(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  FUN_006efe20(param_1);
  FUN_006f01e0();
  iVar1 = FUN_006e7c70();
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    iVar1 = FUN_006e87a0();
    if (iVar1 == 0) {
      uVar2 = 0;
    }
    else {
      iVar1 = FUN_006e8890();
      if (iVar1 == 0) {
        uVar2 = 0;
      }
      else {
        iVar1 = FUN_006e8980();
        if (iVar1 == 0) {
          uVar2 = 0;
        }
        else {
          iVar1 = FUN_006e8de0();
          if (iVar1 == 0) {
            uVar2 = 0;
          }
          else {
            iVar1 = FUN_006e9550();
            if (iVar1 == 0) {
              uVar2 = 0;
            }
            else {
              iVar1 = FUN_006e9680();
              if (iVar1 == 0) {
                uVar2 = 0;
              }
              else {
                iVar1 = FUN_006e9770();
                if (iVar1 == 0) {
                  uVar2 = 0;
                }
                else {
                  iVar1 = FUN_006e9860();
                  if (iVar1 == 0) {
                    uVar2 = 0;
                  }
                  else {
                    iVar1 = FUN_006e9bc0();
                    if (iVar1 == 0) {
                      uVar2 = 0;
                    }
                    else {
                      iVar1 = FUN_006e9c70();
                      if (iVar1 == 0) {
                        uVar2 = 0;
                      }
                      else {
                        iVar1 = FUN_006e9d20();
                        if (iVar1 == 0) {
                          uVar2 = 0;
                        }
                        else {
                          iVar1 = FUN_006e9dd0();
                          if (iVar1 == 0) {
                            uVar2 = 0;
                          }
                          else {
                            iVar1 = FUN_006e9e80();
                            if (iVar1 == 0) {
                              uVar2 = 0;
                            }
                            else {
                              iVar1 = FUN_006e9f30();
/* ... 300 more lines truncated by --max-lines */
```

### decoy: 0x006e7c70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006e7c70: 16-byte aligned: YES
  006e7c70  55             PUSH EBP
  006e7c71  8B EC          MOV EBP,ESP
  006e7c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006e7c70
string literals (26):
  0x009958C8  at 006e7cc2  'Appearance'
  0x0099AAC8  at 006e7d27  'ABORTONPARRY'
  0x00998D4C  at 006e7d6d  'BACKUPHEAD'
  0x0099AAB4  at 006e7db3  'CameraHeightOffset'
  0x0099AAA8  at 006e7df9  'CAMERASPACE'
  0x0099AA9C  at 006e7e3f  'CREPERSPACE'
  0x0099AA88  at 006e7e85  'DisableInjuredAnim'
  0x0099AA7C  at 006e7ecb  'DRIVEACCl'
  0x0099AA6C  at 006e7f11  'DriveAnimRun_PC'
  0x0099AA5C  at 006e7f57  'DriveAnimWalk'
  0x0099AA4C  at 006e7f9d  'DriveMaxSpeed'
  0x0099AA44  at 006e7fe3  'ENVMAP'
direct callees (7):
  0x00919723  operator new
  0x00710840  (unnamed)
  0x0071AD40  CRes2DA helper ctor (name, flag)
  0x0071C3D0  C2DA::LoadText
  0x00733570  CExoString::CExoString_char
  0x0071AEF0  C2DA::GetColumnIndex
  0x00733780  CExoString::~CExoString
struct offsets touched (top 1):
  +0x10 [dec 16]  x31  via EAX,ECX,EDX
```

## Card B0-2-42

GOG function: CGuiInGame::HideUpgradeScreen  gog 0x004de2e0  callconv=(blank)  param_size_bytes=0  gog_size=217  gog_retn=0
Candidate Steam address: 0x007cdbe0 (size 217, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cdcc0:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004de1a0 CGuiInGame::ShowChemicalUpgradeScreen  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004de3c0 CGuiInGame::HideChemicalUpgradeScreen  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040ccd0 CClientExoApp::GetSWCMessage -> Steam ?  in candidate callees: n/a
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x00506010 CSWCMessage::SendPlayerToServerInput_TogglePauseRequest -> Steam ?  in candidate callees: n/a
  0x0063ac00 CExoSound::SetSoundMode -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x38*,0x48*,0x88*,0x128*,0xf18*} | Steam {0x4*,0x38*,0x48*,0x88*,0x128*,0xf18*}
imm sets (* = in both, 5 common): GOG {0x7*,0x10*,0x200*,0xfffffcff*,0xffffff7f*} | Steam {0x7*,0x10*,0x200*,0xfffffcff*,0xffffff7f*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007cdbe0: 16-byte aligned: YES
  007cdbe0  55             PUSH EBP
  007cdbe1  8B EC          MOV EBP,ESP
  007cdbe3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cdbe0
string literals (0):
direct callees (4):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x0087A730  (unnamed)
  0x0073FEE0  CClientExoApp::SetInputClass
  0x0070BC60  (unnamed)
struct offsets touched (top 6):
  +0x48 [dec 72]  x4  via EAX,EDX
  +0x88 [dec 136]  x3  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x38 [dec 56]  x1  via ECX
  +0x128 [dec 296]  x1  via EAX
  +0xF18 [dec 3864]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007cdbe0 FUN_007cdbe0 size=217 ret=0 callers=2 callees=4

void __fastcall FUN_007cdbe0(int param_1)

{
  int iVar1;
  
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

### decoy: 0x007cdcc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cdcc0: 16-byte aligned: YES
  007cdcc0  55             PUSH EBP
  007cdcc1  8B EC          MOV EBP,ESP
  007cdcc3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cdcc0
string literals (0):
direct callees (4):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x0087A730  (unnamed)
  0x0073FEE0  CClientExoApp::SetInputClass
  0x0070BC60  (unnamed)
struct offsets touched (top 6):
  +0x48 [dec 72]  x4  via EAX,EDX
  +0x8C [dec 140]  x3  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x38 [dec 56]  x1  via ECX
  +0x128 [dec 296]  x1  via EAX
  +0xF18 [dec 3864]  x1  via EAX
```

## Card B0-2-43

GOG function: CSWCObject::ClearAllActions  gog 0x00477660  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x007f24c0 (size 54, retn 0)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f2500:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00477630 CSWCObject::DisableBumpedOutShell  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004776a0 CSWCObject::ClearAllQueuedCombatActions -> 0x007f2500  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callees: yes
  0x0064c530 CServerExoApp::ClearAllActions -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
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
prologue for 0x007f24c0: 16-byte aligned: YES
  007f24c0  55             PUSH EBP
  007f24c1  8B EC          MOV EBP,ESP
  007f24c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f24c0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051CB10  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f24c0 FUN_007f24c0 size=54 ret=0 callers=11 callees=2

void __fastcall FUN_007f24c0(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0(*(undefined4 *)(param_1 + 4));
  FUN_0051cb10(uVar1);
  return;
}
```

### decoy: 0x007f2500 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f2500: 16-byte aligned: YES
  007f2500  55             PUSH EBP
  007f2501  8B EC          MOV EBP,ESP
  007f2503  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f2500
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051CBA0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
```

## Card B0-2-44

GOG function: CSWSItemPropertyHandler::RemoveEnhancementBonus  gog 0x007c2600  callconv=__thiscall  param_size_bytes=16  gog_size=1917  gog_retn=16
Candidate Steam address: 0x005abef0 (size 1261, retn 16)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::RemoveEnhancementPenalty
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.73 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.73
- best decoy (neighbour/other candidate) scores: 0x005ab980:STR0/CALLEE0/SIG0.492/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c1ee0 CSWSItemPropertyHandler::ApplyEnhancementBonus -> 0x005aaad0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c2d90 CSWSItemPropertyHandler::ApplyEnhancementPenalty -> 0x005ab980  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x0066a400 CSWSItem::GetDamageFlags -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 10 common): GOG {0x2,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x38,0x122*,0x130*,0x13c*,0x1198*} | Steam {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x13c*,0x1198*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3,0x4,0x5*,0x6,0x7*,0x8*,0x10*,0x20*,0x94,0x148*,0x4000*,0x8000*,0x10000*} | Steam {0x2*,0x5*,0x7*,0x8*,0x10*,0x14,0x20*,0x68,0x148*,0x4000*,0x8000*,0x10000*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005abef0: 16-byte aligned: YES
  005abef0  55             PUSH EBP
  005abef1  8B EC          MOV EBP,ESP
  005abef3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005abef0
string literals (1):
  0x00992E8C  at 005abf19  'Value'
direct callees (7):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 10):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x122 [dec 290]  x2  via ECX,EDX
  +0x13C [dec 316]  x2  via ECX,EDX
  +0x1198 [dec 4504]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005abef0 FUN_005abef0 size=1261 ret=16 callers=0 callees=7

undefined4 FUN_005abef0(int param_1,int param_2,int param_3,int param_4)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  undefined1 *puVar5;
  int *piVar6;
  undefined1 local_28 [8];
  uint local_20;
  int local_1c;
  undefined4 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952cd8;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar6 = &local_1c;
  puVar5 = local_28;
  uVar3 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar3,puVar5,piVar6);
  local_8 = 0xffffffff;
  FUN_00733780(uVar2);
  local_1c = -local_1c;
  local_14 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x13c);
LAB_005abfa9:
  do {
    if (local_14[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995f54 < (int)(uint)*(ushort *)(*(int *)(*local_14 + local_20 * 4) + 8)) {
      ExceptionList = local_10;
      return 0;
    }
    local_18 = *(undefined4 **)(*local_14 + local_20 * 4);
    if ((((*(ushort *)(local_18 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f48) &&
         ((*(ushort *)((int)local_18 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_18[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
       ((iVar4 = FUN_005e4e60(0), iVar4 == local_1c &&
        (bVar1 = *(byte *)(DAT_00a1b4d0 + 0x122), uVar2 = FUN_005e4e60(2), uVar2 == bVar1)))) {
      if (param_4 == 0x10) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_00996188) {
          local_20 = local_20 + 1;
          goto LAB_005abfa9;
        }
      }
      else if (param_4 == 0x20) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_0099618c) {
          local_20 = local_20 + 1;
          goto LAB_005abfa9;
        }
      }
      else if (param_4 == 0x4000) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_00996190) {
          local_20 = local_20 + 1;
          goto LAB_005abfa9;
        }
      }
      else if (param_4 == 0x8000) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_00996194) {
          local_20 = local_20 + 1;
          goto LAB_005abfa9;
        }
      }
/* ... 86 more lines truncated by --max-lines */
```

### decoy: 0x005ab980 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ab980: 16-byte aligned: YES
  005ab980  55             PUSH EBP
  005ab981  8B EC          MOV EBP,ESP
  005ab983  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ab980
string literals (1):
  0x00992E8C  at 005aba70  'Value'
direct callees (12):
  0x006D5200  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 8):
  +0x8 [dec 8]  x4  via EAX,ECX
  +0xA [dec 10]  x4  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x122 [dec 290]  x2  via ECX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-2-45

GOG function: CGuiInGame::GetCanHeadFollowInDialog  gog 0x004dfff0  callconv=(blank)  param_size_bytes=4  gog_size=112  gog_retn=4
Candidate Steam address: 0x007cf8f0 (size 112, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007cf780:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dfe80 CGuiInGame::SetLockHeadFollowInDialog  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004e0060 CGuiInGame::TutorialReasonWillShowWindow  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0xc*,0xf28*,0x7f000000*} | Steam {0x4*,0xc*,0xf28*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cf8f0: 16-byte aligned: YES
  007cf8f0  55             PUSH EBP
  007cf8f1  8B EC          MOV EBP,ESP
  007cf8f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cf8f0
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007cf8f0 FUN_007cf8f0 size=112 ret=4 callers=1 callees=3

undefined4 __thiscall FUN_007cf8f0(undefined4 param_1,int param_2)

{
  undefined4 uVar1;
  int local_c;
  int *local_8;
  
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

### decoy: 0x007cf780 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cf780: 16-byte aligned: YES
  007cf780  55             PUSH EBP
  007cf781  8B EC          MOV EBP,ESP
  007cf783  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cf780
string literals (0):
direct callees (8):
  0x0073F4D0  ClientApp::GetClientObject
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
  0x00919723  operator new
  0x0054AA10  CExoLinkedList::AddHead
  0x00773BF0  CSWCCreature::LookAt
  0x0064DDE0  (unnamed)
struct offsets touched (top 3):
  +0x2C [dec 44]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
```

## Card B0-2-46

GOG function: CSWCObject::GetName  gog 0x00472580  callconv=(blank)  param_size_bytes=4  gog_size=47  gog_retn=4
Candidate Steam address: 0x007ed420 (size 47, retn 4)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed590:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004721b0 CSWCObject::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004725b0 CSWCObject::GetCameraFollowPart  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x007ed420: 16-byte aligned: YES
  007ed420  55             PUSH EBP
  007ed421  8B EC          MOV EBP,ESP
  007ed423  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed420
string literals (0):
direct callees (1):
  0x00733570  CExoString::CExoString_char
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007ed420 FUN_007ed420 size=47 ret=4 callers=0 callees=1

undefined4 FUN_007ed420(undefined4 param_1)

{
  FUN_00733570(&DAT_00986925);
  return param_1;
}
```

### decoy: 0x007ed590 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed590: 16-byte aligned: YES
  007ed590  55             PUSH EBP
  007ed591  8B EC          MOV EBP,ESP
  007ed593  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed590
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x88 [dec 136]  x1  via EDX
```

## Card B0-2-47

GOG function: CSWCObject::TurnOffShadows  gog 0x00472660  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x007ed500 (size 42, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed4d0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: autolabel:NotifyPaneDivider
- GOG-order neighbour before: 0x00472630 CSWCObject::TurnOnShadows -> 0x007ed4d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00472690 CSWCObject::BarkString  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0xbc*} | Steam {0x68*,0xbc*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ed500: 16-byte aligned: YES
  007ed500  55             PUSH EBP
  007ed501  8B EC          MOV EBP,ESP
  007ed503  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed500
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xBC [dec 188]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed500 CPaneContainerManager::NotifyPaneDivider size=42 ret=0 callers=0 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
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

### decoy: 0x007ed4d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed4d0: 16-byte aligned: YES
  007ed4d0  55             PUSH EBP
  007ed4d1  8B EC          MOV EBP,ESP
  007ed4d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed4d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB8 [dec 184]  x1  via EDX
```

## Card B0-2-48

GOG function: CTwoDimArrays::Constructor  gog 0x00480170  callconv=(blank)  param_size_bytes=0  gog_size=1042  gog_retn=0
Candidate Steam address: 0x006e71d0 (size 1042, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_2da_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006e6e70:STR0/CALLEE0/SIG0.011/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00480160 CSWCItem::non_virtual_thunk_to_dtor_CSWCItem  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00480590 CTwoDimArrays::Destructor -> 0x006e75f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 86 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x38*,...(+72)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x38*,...(+72)}
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
prologue for 0x006e71d0: 16-byte aligned: YES
  006e71d0  55             PUSH EBP
  006e71d1  8B EC          MOV EBP,ESP
  006e71d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006e71d0
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x28 [dec 40]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006e71d0 FUN_006e71d0 size=1042 ret=0 callers=1 callees=0

undefined4 * __fastcall FUN_006e71d0(undefined4 *param_1)

{
  *param_1 = 0;
  *(undefined1 *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) = 0;
  param_1[2 = byte +0x8] = 0;
  *(undefined1 *)(param_1 + 0x3 [dec 3] {undefined4*: byte +0xc}) = 0;
  param_1[4 = byte +0x10] = 0;
  param_1[5 = byte +0x14] = 0;
  param_1[6 = byte +0x18] = 0;
  param_1[7 = byte +0x1c] = 0;
  param_1[8 = byte +0x20] = 0;
  param_1[9 = byte +0x24] = 0;
  param_1[10 = byte +0x28] = 0;
  param_1[0xb = byte +0x2c] = 0;
  param_1[0xc = byte +0x30] = 0;
  param_1[0xd = byte +0x34] = 0;
  param_1[0xe = byte +0x38] = 0;
  param_1[0xf = byte +0x3c] = 0;
  param_1[0x10 = byte +0x40] = 0;
  param_1[0x11 = byte +0x44] = 0;
  param_1[0x12 = byte +0x48] = 0;
  param_1[0x13 = byte +0x4c] = 0;
  param_1[0x14 = byte +0x50] = 0;
  param_1[0x15 = byte +0x54] = 0;
  param_1[0x16 = byte +0x58] = 0;
  param_1[0x17 = byte +0x5c] = 0;
  param_1[0x18 = byte +0x60] = 0;
  param_1[0x19 = byte +0x64] = 0;
  param_1[0x1a = byte +0x68] = 0;
  param_1[0x1b = byte +0x6c] = 0;
  param_1[0x1c = byte +0x70] = 0;
  param_1[0x1d = byte +0x74] = 0;
  param_1[0x1e = byte +0x78] = 0;
  param_1[0x1f = byte +0x7c] = 0;
  param_1[0x20 = byte +0x80] = 0;
  param_1[0x21 = byte +0x84] = 0;
  param_1[0x22 = byte +0x88] = 0;
  param_1[0x23 = byte +0x8c] = 0;
  param_1[0x24 = byte +0x90] = 0;
  param_1[0x25 = byte +0x94] = 0;
  param_1[0x26 = byte +0x98] = 0;
  param_1[0x27 = byte +0x9c] = 0;
  param_1[0x28 = byte +0xa0] = 0;
  param_1[0x29 = byte +0xa4] = 0;
  param_1[0x2a = byte +0xa8] = 0;
  param_1[0x2b = byte +0xac] = 0;
  param_1[0x2c = byte +0xb0] = 0;
  param_1[0x2d = byte +0xb4] = 0;
  param_1[0x2e = byte +0xb8] = 0;
  param_1[0x2f = byte +0xbc] = 0;
  param_1[0x30 = byte +0xc0] = 0;
  param_1[0x31 = byte +0xc4] = 0;
  param_1[0x32 = byte +0xc8] = 0;
  param_1[0x33 = byte +0xcc] = 0;
  param_1[0x34 = byte +0xd0] = 0;
  param_1[0x35 = byte +0xd4] = 0;
  param_1[0x36 = byte +0xd8] = 0;
  param_1[0x37 = byte +0xdc] = 0;
  param_1[0x38 = byte +0xe0] = 0;
  param_1[0x39 = byte +0xe4] = 0;
  param_1[0x3a = byte +0xe8] = 0;
  param_1[0x3b = byte +0xec] = 0;
  param_1[0x3c = byte +0xf0] = 0;
  param_1[0x3d = byte +0xf4] = 0;
  param_1[0x3e = byte +0xf8] = 0;
  param_1[0x3f = byte +0xfc] = 0;
  param_1[0x40 = byte +0x100] = 0;
  param_1[0x41 = byte +0x104] = 0;
  param_1[0x42 = byte +0x108] = 0;
  param_1[0x43 = byte +0x10c] = 0;
  param_1[0x44 = byte +0x110] = 0;
  param_1[0x45 = byte +0x114] = 0;
  param_1[0x46 = byte +0x118] = 0;
  param_1[0x47 = byte +0x11c] = 0;
  param_1[0x48 = byte +0x120] = 0;
  param_1[0x49 = byte +0x124] = 0;
  param_1[0x4a = byte +0x128] = 0;
  param_1[0x4b = byte +0x12c] = 0;
/* ... 13 more lines truncated by --max-lines */
```

### decoy: 0x006e6e70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006e6e70: 16-byte aligned: YES
  006e6e70  55             PUSH EBP
  006e6e71  8B EC          MOV EBP,ESP
  006e6e73  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006e6e70
string literals (0):
direct callees (4):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005908C0  CSWSCombatRound::GetAttack
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x006B83F0  CSWSCreatureStats::HasFeat
struct offsets touched (top 7):
  +0x1198 [dec 4504]  x16  via EAX,ECX,EDX
  +0x150 [dec 336]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0x145 [dec 325]  x1  via EDX
  +0xABC [dec 2748]  x1  via EDX
  +0x10DC [dec 4316]  x1  via EAX
  +0x6E71B8 [dec 7238072]  x1  via EAX
```

## Card B0-2-49

GOG function: CSWCObject::TurnOnShadows  gog 0x00472630  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x007ed4d0 (size 42, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed4b0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: autolabel:NotifyPaneDivider
- GOG-order neighbour before: 0x00472610 CSWCObject::GetPortraitId -> 0x007ed4b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00472660 CSWCObject::TurnOffShadows -> 0x007ed500  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0xb8*} | Steam {0x68*,0xb8*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ed4d0: 16-byte aligned: YES
  007ed4d0  55             PUSH EBP
  007ed4d1  8B EC          MOV EBP,ESP
  007ed4d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed4d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB8 [dec 184]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed4d0 CPaneContainerManager::NotifyPaneDivider size=42 ret=0 callers=0 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
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

### decoy: 0x007ed4b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed4b0: 16-byte aligned: YES
  007ed4b0  55             PUSH EBP
  007ed4b1  8B EC          MOV EBP,ESP
  007ed4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed4b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xB8 [dec 184]  x1  via EAX
```

## Card B0-2-50

GOG function: CGuiInGame::SetLevelUpMode  gog 0x004d00b0  callconv=(blank)  param_size_bytes=4  gog_size=41  gog_retn=4
Candidate Steam address: 0x007bfa20 (size 41, retn 4)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bfcc0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ceb50 CGuiInGame::Initialize  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004d00e0 CGuiInGame::SetConversationType  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x12c*} | Steam {0x12c*}
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
prologue for 0x007bfa20: 16-byte aligned: YES
  007bfa20  55             PUSH EBP
  007bfa21  8B EC          MOV EBP,ESP
  007bfa23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bfa20
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x12C [dec 300]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007bfa20 FUN_007bfa20 size=41 ret=4 callers=2 callees=0

void __thiscall FUN_007bfa20(int param_1,int param_2)

{
  if (*(int *)(param_1 + 0x12c [dec 300]) != param_2) {
    *(int *)(param_1 + 0x12c [dec 300]) = param_2;
  }
  return;
}
```

### decoy: 0x007bfcc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bfcc0: 16-byte aligned: YES
  007bfcc0  55             PUSH EBP
  007bfcc1  8B EC          MOV EBP,ESP
  007bfcc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007bfcc0
string literals (0):
direct callees (3):
  0x009196FD  operator delete
  0x00919723  operator new
  0x007107F0  (unnamed)
struct offsets touched (top 3):
  +0x1B0 [dec 432]  x7  via EAX,ECX,EDX
  +0x1B4 [dec 436]  x7  via EAX,EDX
  +0x1AC [dec 428]  x4  via EAX,ECX,EDX
```

## Card B0-2-51

GOG function: CSWCObject::GetPortraitId  gog 0x00472610  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x007ed4b0 (size 21, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed4d0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004725e0 CSWCObject::GetPortrait -> 0x007ed480  gate_class=REVIEW
- GOG-order neighbour after:  0x00472630 CSWCObject::TurnOnShadows -> 0x007ed4d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xb8*} | Steam {0xb8*}
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
prologue for 0x007ed4b0: 16-byte aligned: YES
  007ed4b0  55             PUSH EBP
  007ed4b1  8B EC          MOV EBP,ESP
  007ed4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed4b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xB8 [dec 184]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed4b0 FUN_007ed4b0 size=21 ret=0 callers=0 callees=0

undefined2 __fastcall FUN_007ed4b0(int param_1)

{
  return *(undefined2 *)(param_1 + 0xb8);
}
```

### decoy: 0x007ed4d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed4d0: 16-byte aligned: YES
  007ed4d0  55             PUSH EBP
  007ed4d1  8B EC          MOV EBP,ESP
  007ed4d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed4d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB8 [dec 184]  x1  via EDX
```

## Card B0-2-52

GOG function: CSWCObject::AnimationHideEquippedItems  gog 0x00474540  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef910 (size 129, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef880:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004744b0 CSWCObject::AnimationPlayOutOfPlace -> 0x007ef730  gate_class=REVIEW
- GOG-order neighbour after:  0x004745d0 CSWCObject::AnimationWalking -> 0x007ef3e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xcc*,0xe0*,0x130*} | Steam {0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef910: 16-byte aligned: YES
  007ef910  55             PUSH EBP
  007ef911  8B EC          MOV EBP,ESP
  007ef913  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef910
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef910 FUN_007ef910 size=129 ret=4 callers=3 callees=1

bool __thiscall FUN_007ef910(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe50,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef880: 16-byte aligned: YES
  007ef880  55             PUSH EBP
  007ef881  8B EC          MOV EBP,ESP
  007ef883  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef880
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-2-53

GOG function: CGuiInGame::IsGlobalFading  gog 0x004dc340  callconv=(blank)  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x007cbc40 (size 44, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cbc70:STR0/CALLEE1/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dc2f0 CGuiInGame::StopGlobalFade -> 0x007cbbf0  gate_class=AUTO
- GOG-order neighbour after:  0x004dc370 CGuiInGame::InitializePlaceableCameras  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005ac690 CSWGuiFade::IsFadeDone -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x6c*} | Steam {0x6c*}
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
prologue for 0x007cbc40: 16-byte aligned: YES
  007cbc40  55             PUSH EBP
  007cbc41  8B EC          MOV EBP,ESP
  007cbc43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cbc40
string literals (0):
direct callees (1):
  0x007BCA60  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007cbc40 FUN_007cbc40 size=44 ret=0 callers=4 callees=1

undefined4 __fastcall FUN_007cbc40(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 0x6c) != 0) && (iVar1 = FUN_007bca60(), iVar1 == 0)) {
    return 1;
  }
  return 0;
}
```

### decoy: 0x007cbc70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cbc70: 16-byte aligned: YES
  007cbc70  55             PUSH EBP
  007cbc71  8B EC          MOV EBP,ESP
  007cbc73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cbc70
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x50 [dec 80]  x1  via ECX
  +0x6C [dec 108]  x1  via EAX
```

## Card B0-2-54

GOG function: CSWCArea::ClearTilePathNodes  gog 0x00456350  callconv=(blank)  param_size_bytes=0  gog_size=64  gog_retn=0
Candidate Steam address: 0x007a9950 (size 64, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a9860:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004562d0 CSWCArea::ApplyWeatherVisual -> 0x007a98d0  gate_class=AUTO
- GOG-order neighbour after:  0x00456390 CSWCArea::AddPlaceableObjectLight -> 0x007a9990  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x007c0300 CExoLinkedList_CAurObject_::RemoveHead -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0x208*} | Steam {0x8*,0x208*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007a9950: 16-byte aligned: YES
  007a9950  55             PUSH EBP
  007a9951  8B EC          MOV EBP,ESP
  007a9953  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a9950
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a9950 FUN_007a9950 size=64 ret=0 callers=1 callees=3

void __fastcall FUN_007a9950(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = FUN_007a1720(param_1);
  while( true ) {
    iVar2 = FUN_007a1780(param_1,uVar1);
    if (iVar2 != 0) break;
    FUN_007f2a90();
  }
  return;
}
```

### decoy: 0x007a9860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a9860: 16-byte aligned: YES
  007a9860  55             PUSH EBP
  007a9861  8B EC          MOV EBP,ESP
  007a9863  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a9860
string literals (0):
direct callees (1):
  0x0091EB8B  (unnamed)
struct offsets touched (top 3):
  +0x1A8 [dec 424]  x3  via EAX,ECX
  +0x4C [dec 76]  x1  via EDX
  +0x1E4 [dec 484]  x1  via EDX
```

## Card B0-2-55

GOG function: CGuiInGame::HidePowersFeatsSkillsDebugMenu  gog 0x004dbc40  callconv=(blank)  param_size_bytes=0  gog_size=149  gog_retn=0
Candidate Steam address: 0x007cb540 (size 149, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cb4a0:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbba0 CGuiInGame::HideLoadModuleDebugMenu -> 0x007cb4a0  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbce0 CGuiInGame::UnlockDialogParticipantOrientations  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d580 CClientExoApp::GetMiniGame -> Steam ?  in candidate callees: n/a
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
  0x0090ee40 CSWGuiManager::PopModalPanel -> Steam 0x004109c0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x38*,0x74*,0x128*} | Steam {0x4*,0x38*,0x74*,0x128*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007cb540: 16-byte aligned: YES
  007cb540  55             PUSH EBP
  007cb541  8B EC          MOV EBP,ESP
  007cb543  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb540
string literals (0):
direct callees (5):
  0x00410800  GuiManager::Contains
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00410670  (unnamed)
  0x007400A0  CClientExoApp::GetMiniGame
  0x0073FEE0  CClientExoApp::SetInputClass
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x38 [dec 56]  x3  via EAX,ECX
  +0x74 [dec 116]  x2  via ECX,EDX
  +0x128 [dec 296]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007cb540 FUN_007cb540 size=149 ret=0 callers=2 callees=5

void __fastcall FUN_007cb540(int param_1)

{
  int iVar1;
  
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

### decoy: 0x007cb4a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb4a0: 16-byte aligned: YES
  007cb4a0  55             PUSH EBP
  007cb4a1  8B EC          MOV EBP,ESP
  007cb4a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb4a0
string literals (0):
direct callees (5):
  0x00410800  GuiManager::Contains
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00410670  (unnamed)
  0x007400A0  CClientExoApp::GetMiniGame
  0x0073FEE0  CClientExoApp::SetInputClass
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x38 [dec 56]  x3  via EAX,ECX
  +0x70 [dec 112]  x2  via ECX,EDX
  +0x128 [dec 296]  x1  via EAX
```

## Card B0-2-56

GOG function: CTwoDimArrays::Destructor  gog 0x00480590  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x006e75f0 (size 19, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_2da_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006e71d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00480170 CTwoDimArrays::Constructor -> 0x006e71d0  gate_class=REVIEW
- GOG-order neighbour after:  0x004805b0 CTwoDimArrays::Load2DArrays -> 0x006e7610  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00486770 CTwoDimArrays::UnLoad2DArrays -> Steam ?  in candidate callees: n/a
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
prologue for 0x006e75f0: 16-byte aligned: YES
  006e75f0  55             PUSH EBP
  006e75f1  8B EC          MOV EBP,ESP
  006e75f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006e75f0
string literals (0):
direct callees (1):
  0x006ED7D0  CTwoDimArrays::UnLoad2DArrays
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006e75f0 FUN_006e75f0 size=19 ret=0 callers=1 callees=1
THUNK -> 0x006ED7D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_006e75f0(undefined4 param_1)

{
  FUN_006ed7d0(param_1);
  return;
}
```

### decoy: 0x006e71d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006e71d0: 16-byte aligned: YES
  006e71d0  55             PUSH EBP
  006e71d1  8B EC          MOV EBP,ESP
  006e71d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006e71d0
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x28 [dec 40]  x1  via ECX
```

## Card B0-2-57

GOG function: CGuiInGame::IsAnimationPlayingInDialog  gog 0x004dbde0  callconv=(blank)  param_size_bytes=0  gog_size=55  gog_retn=0
Candidate Steam address: 0x007cb6e0 (size 55, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (Draw 0x8bb260); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007cb750:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbda0 CGuiInGame::IsCameraDialog -> 0x007cb6a0  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbe20 CGuiInGame::IsLoopingDialogStuntAnimation_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x3c*,0x64*,0x84*,0xfb4*} | Steam {0x3c*,0x64*,0x84*,0xfb4*}
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
prologue for 0x007cb6e0: 16-byte aligned: YES
  007cb6e0  55             PUSH EBP
  007cb6e1  8B EC          MOV EBP,ESP
  007cb6e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb6e0
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x3C [dec 60]  x2  via ECX,EDX
  +0x64 [dec 100]  x1  via EAX
  +0x84 [dec 132]  x1  via EDX
  +0xFB4 [dec 4020]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007cb6e0 FUN_007cb6e0 size=55 ret=0 callers=1 callees=0

void __fastcall FUN_007cb6e0(int param_1)

{
  if (*(int *)(param_1 + 0x64 [dec 100]) != 0) {
    *(undefined4 *)(param_1 + 0xfb4) = 1;
    (**(code **)(**(int **)(param_1 + 0x3c) + 0x84))();
  }
  return;
}
```

### decoy: 0x007cb750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb750: 16-byte aligned: YES
  007cb750  55             PUSH EBP
  007cb751  8B EC          MOV EBP,ESP
  007cb753  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x40 [dec 64]  x1  via EDX
```

## Card B0-2-58

GOG function: CSWCObject::AnimationStationary  gog 0x00474420  callconv=(blank)  param_size_bytes=2  gog_size=132  gog_retn=4
Candidate Steam address: 0x007ef2c0 (size 132, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef350:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004743a0 CSWCObject::SetOrientation  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004744b0 CSWCObject::AnimationPlayOutOfPlace -> 0x007ef730  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xcc*,0xe0*,0x130*} | Steam {0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef2c0: 16-byte aligned: YES
  007ef2c0  55             PUSH EBP
  007ef2c1  8B EC          MOV EBP,ESP
  007ef2c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef2c0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef2c0 FUN_007ef2c0 size=132 ret=4 callers=6 callees=1

bool __thiscall FUN_007ef2c0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe28,&local_8);
  return iVar1 == 0 || local_8 != 0;
}
```

### decoy: 0x007ef350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef350: 16-byte aligned: YES
  007ef350  55             PUSH EBP
  007ef351  8B EC          MOV EBP,ESP
  007ef353  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef350
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-2-59

GOG function: CSWCObject::ClearAllQueuedCombatActions  gog 0x004776a0  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x007f2500 (size 54, retn 0)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f24c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00477660 CSWCObject::ClearAllActions -> 0x007f24c0  gate_class=REVIEW
- GOG-order neighbour after:  0x004776e0 CSWCObject::GetServerObject -> 0x007f2540  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callees: yes
  0x0064c5c0 CServerExoApp::ClearAllQueuedCombatActions -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
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
prologue for 0x007f2500: 16-byte aligned: YES
  007f2500  55             PUSH EBP
  007f2501  8B EC          MOV EBP,ESP
  007f2503  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2500
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051CBA0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f2500 FUN_007f2500 size=54 ret=0 callers=2 callees=2

void __fastcall FUN_007f2500(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0(*(undefined4 *)(param_1 + 4));
  FUN_0051cba0(uVar1);
  return;
}
```

### decoy: 0x007f24c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f24c0: 16-byte aligned: YES
  007f24c0  55             PUSH EBP
  007f24c1  8B EC          MOV EBP,ESP
  007f24c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f24c0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051CB10  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
```

## Card B0-2-60

GOG function: CSWCArea::StopWeather  gog 0x00456230  callconv=(blank)  param_size_bytes=0  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a9830 (size 36, retn 4)
Candidate sources: branch+name_map:high:batch5_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a9800:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00456200 CSWCArea::StartWeather -> 0x007a9800  gate_class=REVIEW
- GOG-order neighbour after:  0x00456260 CSWCArea::SetLightningDone -> 0x007a9860  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1f0*,0x1f5*} | Steam {0x1f0*,0x1f5*}
imm sets (* = in both, 2 common): GOG {0x4*,0x1770*} | Steam {0x4*,0x1770*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x007a9830: 16-byte aligned: YES
  007a9830  55             PUSH EBP
  007a9831  8B EC          MOV EBP,ESP
  007a9833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a9830
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1F0 [dec 496]  x1  via ECX
  +0x1F5 [dec 501]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007a9830 FUN_007a9830 size=36 ret=4 callers=2 callees=0

void __fastcall FUN_007a9830(int param_1)

{
  *(undefined1 *)(param_1 + 0x1f5) = 0;
  *(undefined4 *)(param_1 + 0x1f0) = 6000;
  return;
}
```

### decoy: 0x007a9800 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a9800: 16-byte aligned: YES
  007a9800  55             PUSH EBP
  007a9801  8B EC          MOV EBP,ESP
  007a9803  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a9800
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1F0 [dec 496]  x1  via EDX
  +0x1F5 [dec 501]  x1  via EAX
```
