# Review cards B0 chunk 7

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-7-01

GOG function: CSWPartyTable::AddGameInProgress  gog 0x00703650  callconv=(blank)  param_size_bytes=0  gog_size=138  gog_retn=0
Candidate Steam address: 0x005fdca0 (size 138, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fdd30:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007035f0 CSWPartyTable::GetPUPID -> 0x005fdc40  gate_class=REVIEW
- GOG-order neighbour after:  0x007036e0 CSWPartyTable::RemoveGameInProgress -> 0x005fdd30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x0061aff0 CExoResMan::AddResourceDirectory -> Steam ?  in candidate callees: n/a
GOG strings (1): 'GAMEINPROGRESS:'
disp sets (* = in both, 1 common): GOG {0x168*} | Steam {0x168*}
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
prologue for 0x005fdca0: 16-byte aligned: YES
  005fdca0  55             PUSH EBP
  005fdca1  8B EC          MOV EBP,ESP
  005fdca3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005fdca0
string literals (1):
  0x00993E84  at 005fdcd4  'GAMEINPROGRESS:'
direct callees (3):
  0x00733570  CExoString::CExoString_char
  0x00711690  CExoResMan::AddResourceDirectory
  0x00733780  CExoString::~CExoString
struct offsets touched (top 1):
  +0x168 [dec 360]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005fdca0 FUN_005fdca0 size=138 ret=0 callers=8 callees=3

void __fastcall FUN_005fdca0(int param_1)

{
  uint uVar1;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954bd8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x168) == 0) {
    FUN_00733570("GAMEINPROGRESS:");
    local_8 = 0;
    FUN_00711690(local_18);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1);
  }
  *(int *)(param_1 + 0x168) = *(int *)(param_1 + 0x168) + 1;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x005fdd30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fdd30: 16-byte aligned: YES
  005fdd30  55             PUSH EBP
  005fdd31  8B EC          MOV EBP,ESP
  005fdd33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005fdd30
string literals (1):
  0x00993E84  at 005fdd79  'GAMEINPROGRESS:'
direct callees (3):
  0x00733570  CExoString::CExoString_char
  0x00711710  CExoResMan::RemoveResourceDirectory
  0x00733780  CExoString::~CExoString
struct offsets touched (top 1):
  +0x168 [dec 360]  x3  via EAX,EDX
```

## Card B0-7-02

GOG function: CSWPartyTable::ClearMembers  gog 0x00703770  callconv=(blank)  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x005fddc0 (size 53, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005fdbf0:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007036e0 CSWPartyTable::RemoveGameInProgress -> 0x005fdd30  gate_class=REVIEW
- GOG-order neighbour after:  0x007037b0 CSWPartyTable::ClearPuppets -> 0x005fde00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0x7f000000*} | Steam {0x8*,0xc*,0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005fddc0: 16-byte aligned: YES
  005fddc0  55             PUSH EBP
  005fddc1  8B EC          MOV EBP,ESP
  005fddc3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fddc0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fddc0 FUN_005fddc0 size=53 ret=0 callers=1 callees=0

void __fastcall FUN_005fddc0(int param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 0xc; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1 + 0x1c + local_8 * 4) = 0x7f000000;
  }
  return;
}
```

### decoy: 0x005fdbf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fdbf0: 16-byte aligned: YES
  005fdbf0  55             PUSH EBP
  005fdbf1  8B EC          MOV EBP,ESP
  005fdbf3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fdbf0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-7-03

GOG function: CSWSArea::SmoothSelectNodes  gog 0x00709fd0  callconv=(blank)  param_size_bytes=16  gog_size=135  gog_retn=16
Candidate Steam address: 0x0054fdb0 (size 135, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0054fb10:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00709d30 CSWSArea::SmoothPointsOnPathRestartable -> 0x0054fb10  gate_class=AUTO
- GOG-order neighbour after:  0x0070a060 CSWSArea::SmoothSelectionRestartable -> 0x0054fe40  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0070a060 CSWSArea::SmoothSelectionRestartable -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x1c*} | Steam {0x4*,0x10*,0x1c*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0054fdb0: 16-byte aligned: YES
  0054fdb0  55             PUSH EBP
  0054fdb1  8B EC          MOV EBP,ESP
  0054fdb3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054fdb0
string literals (0):
direct callees (2):
  0x0054FE40  CSWSArea::SmoothSelectionRestartable
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0054fdb0 FUN_0054fdb0 size=135 ret=16 callers=1 callees=2

void FUN_0054fdb0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 extraout_var;
  undefined4 uVar1;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  FUN_0054fe40(param_1,param_2,&local_8,&local_c,1,0);
  uVar1 = extraout_var;
  FUN_0054fe40(local_8,local_c,&local_10,&local_14,1,1);
  FUN_0054fe40(local_10,local_14,param_3,param_4,0,1);
  FUN_009196fd(local_c,uVar1);
  FUN_009196fd(local_14);
  return;
}
```

### decoy: 0x0054fb10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054fb10: 16-byte aligned: YES
  0054fb10  55             PUSH EBP
  0054fb11  8B EC          MOV EBP,ESP
  0054fb13  83 EC 7C       SUB ESP,0x7c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054fb10
string literals (0):
direct callees (3):
  0x005502D0  CSWSArea::SmoothSelectNodesRestartable
  0x0054F990  (unnamed)
  0x0054FA10  (unnamed)
struct offsets touched (top 8):
  +0x1B0 [dec 432]  x19  via EAX,ECX,EDX
  +0x214 [dec 532]  x5  via EAX,ECX,EDX
  +0x218 [dec 536]  x2  via ECX,EDX
  +0x220 [dec 544]  x2  via EAX,ECX
  +0x230 [dec 560]  x2  via EAX,EDX
  +0x234 [dec 564]  x2  via EAX,EDX
  +0x21C [dec 540]  x1  via EDX
  +0x228 [dec 552]  x1  via ECX
```

## Card B0-7-04

GOG function: CSWSCreatureStats::ComputeNumberKnownSpellsLeft  gog 0x006f88f0  callconv=(blank)  param_size_bytes=2  gog_size=148  gog_retn=4
Candidate Steam address: 0x006bc470 (size 148, retn 4)
Candidate sources: branch+name_map:high:gog_creaturestats_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006bc430:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f88b0 CSWSCreatureStats::GetFeat_2 -> 0x006bc430  gate_class=AUTO
- GOG-order neighbour after:  0x006f89b0 CSWSCreatureStats::GetBonusFeat -> 0x006bc530  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006f4870 CSWSCreatureStats::HasFeat -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 11 common): GOG {0x4*,0x7*,0xc*,0xb7*,0xb8*,0xb9*,0xba*,0xbb*,0xbc*,0xbd*,0xffff*} | Steam {0x4*,0x7*,0xc*,0xb7*,0xb8*,0xb9*,0xba*,0xbb*,0xbc*,0xbd*,0xffff*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006bc470: 16-byte aligned: YES
  006bc470  55             PUSH EBP
  006bc471  8B EC          MOV EBP,ESP
  006bc473  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006bc470
string literals (0):
direct callees (1):
  0x006B83F0  CSWSCreatureStats::HasFeat
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006bc470 FUN_006bc470 size=148 ret=4 callers=1 callees=1

undefined4 FUN_006bc470(undefined2 param_1)

{
  undefined4 uVar1;
  ushort local_8;
  
  local_8 = 0xffff;
  switch(param_1) {
  case 0:
    local_8 = 0xb8;
    break;
  case 1:
    local_8 = 0xb9;
    break;
  case 2:
    local_8 = 0xbc;
    break;
  case 3:
    local_8 = 0xb7;
    break;
  case 5:
    local_8 = 0xba;
    break;
  case 6:
    local_8 = 0xbb;
    break;
  case 7:
    local_8 = 0xbd;
  }
  if (local_8 == 0xffffffff) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_006b83f0(local_8);
  }
  return uVar1;
}
```

### decoy: 0x006bc430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006bc430: 16-byte aligned: YES
  006bc430  55             PUSH EBP
  006bc431  8B EC          MOV EBP,ESP
  006bc433  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006bc430
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-7-05

GOG function: CSWPartyTable::RemoveGameInProgress  gog 0x007036e0  callconv=(blank)  param_size_bytes=0  gog_size=138  gog_retn=0
Candidate Steam address: 0x005fdd30 (size 138, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fdca0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00703650 CSWPartyTable::AddGameInProgress -> 0x005fdca0  gate_class=REVIEW
- GOG-order neighbour after:  0x00703770 CSWPartyTable::ClearMembers -> 0x005fddc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x0061b070 CExoResMan::RemoveResourceDirectory -> Steam ?  in candidate callees: n/a
GOG strings (1): 'GAMEINPROGRESS:'
disp sets (* = in both, 1 common): GOG {0x168*} | Steam {0x168*}
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
prologue for 0x005fdd30: 16-byte aligned: YES
  005fdd30  55             PUSH EBP
  005fdd31  8B EC          MOV EBP,ESP
  005fdd33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005fdd30
string literals (1):
  0x00993E84  at 005fdd79  'GAMEINPROGRESS:'
direct callees (3):
  0x00733570  CExoString::CExoString_char
  0x00711710  CExoResMan::RemoveResourceDirectory
  0x00733780  CExoString::~CExoString
struct offsets touched (top 1):
  +0x168 [dec 360]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005fdd30 FUN_005fdd30 size=138 ret=0 callers=8 callees=3

void __fastcall FUN_005fdd30(int param_1)

{
  uint uVar1;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954bd8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(int *)(param_1 + 0x168) = *(int *)(param_1 + 0x168) + -1;
  if (*(int *)(param_1 + 0x168) == 0) {
    FUN_00733570("GAMEINPROGRESS:");
    local_8 = 0;
    FUN_00711710(local_18);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x005fdca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fdca0: 16-byte aligned: YES
  005fdca0  55             PUSH EBP
  005fdca1  8B EC          MOV EBP,ESP
  005fdca3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005fdca0
string literals (1):
  0x00993E84  at 005fdcd4  'GAMEINPROGRESS:'
direct callees (3):
  0x00733570  CExoString::CExoString_char
  0x00711690  CExoResMan::AddResourceDirectory
  0x00733780  CExoString::~CExoString
struct offsets touched (top 1):
  +0x168 [dec 360]  x3  via EAX,ECX,EDX
```

## Card B0-7-06

GOG function: CSWPartyTable::SetSoloMode  gog 0x007048a0  callconv=(blank)  param_size_bytes=8  gog_size=47  gog_retn=8
Candidate Steam address: 0x005feef0 (size 47, retn 8)
Candidate sources: branch+name_map:high:gog_partytable_block+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fed30:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00704810 CSWPartyTable::GetTotalPlayedSeconds -> 0x005fee60  gate_class=AUTO
- GOG-order neighbour after:  0x007048d0 CSWPartyTable::UnstealthParty -> 0x005fef20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007048d0 CSWPartyTable::UnstealthParty -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x238*} | Steam {0x238*}
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
prologue for 0x005feef0: 16-byte aligned: YES
  005feef0  55             PUSH EBP
  005feef1  8B EC          MOV EBP,ESP
  005feef3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005feef0
string literals (0):
direct callees (1):
  0x005FEF20  CSWPartyTable::UnstealthParty
struct offsets touched (top 1):
  +0x238 [dec 568]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005feef0 FUN_005feef0 size=47 ret=8 callers=6 callees=1

void __thiscall FUN_005feef0(int param_1,int param_2,int param_3)

{
  if ((param_2 == 0) && (param_3 != 0)) {
    FUN_005fef20(0);
  }
  *(int *)(param_1 + 0x238) = param_2;
  return;
}
```

### decoy: 0x005fed30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fed30: 16-byte aligned: YES
  005fed30  55             PUSH EBP
  005fed31  8B EC          MOV EBP,ESP
  005fed33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fed30
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-7-07

GOG function: CSWSArea::DecrementPlayersInArea  gog 0x00718510  callconv=(blank)  param_size_bytes=0  gog_size=52  gog_retn=0
Candidate Steam address: 0x00523360 (size 52, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00523320:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007184d0 CSWSArea::SetMaxStealthXP -> 0x00523320  gate_class=REVIEW
- GOG-order neighbour after:  0x00718550 CSWSArea::IncreaseAILevelPriority -> 0x005233a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00718750 CSWSArea::DecreaseAILevelPriority -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x12c*} | Steam {0x12c*}
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
prologue for 0x00523360: 16-byte aligned: YES
  00523360  55             PUSH EBP
  00523361  8B EC          MOV EBP,ESP
  00523363  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00523360
string literals (0):
direct callees (1):
  0x005235A0  CSWSArea::DecreaseAILevelPriority
struct offsets touched (top 1):
  +0x12C [dec 300]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00523360 FUN_00523360 size=52 ret=0 callers=2 callees=1

void __fastcall FUN_00523360(int param_1)

{
  *(int *)(param_1 + 0x12c [dec 300]) = *(int *)(param_1 + 0x12c [dec 300]) + -1;
  if (*(int *)(param_1 + 0x12c [dec 300]) == 0) {
    FUN_005235a0();
  }
  return;
}
```

### decoy: 0x00523320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00523320: 16-byte aligned: YES
  00523320  55             PUSH EBP
  00523321  8B EC          MOV EBP,ESP
  00523323  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00523320
string literals (0):
direct callees (1):
  0x005233A0  CSWSArea::IncreaseAILevelPriority
struct offsets touched (top 1):
  +0x12C [dec 300]  x3  via EAX,ECX
```

## Card B0-7-08

GOG function: CClientExoApp::DisableVideoEffect  gog 0x0040de80  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007409e0 (size 31, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetVideoEffectType+name_held_elsewhere=0x007409c0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007409c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040de60 CClientExoApp::EnableVideoEffect -> 0x007409a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dea0 CClientExoApp::GetVideoEffectType  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044af60 CClientExoAppInternal::DisableVideoEffect -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x3a8}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007409e0: 16-byte aligned: YES
  007409e0  55             PUSH EBP
  007409e1  8B EC          MOV EBP,ESP
  007409e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007409e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x3A8 [dec 936]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007409e0 FUN_007409e0 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_007409e0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x3a8);
}
```

### decoy: 0x007409c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007409c0: 16-byte aligned: YES
  007409c0  55             PUSH EBP
  007409c1  8B EC          MOV EBP,ESP
  007409c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007409c0
string literals (0):
direct callees (1):
  0x0079E960  CClientExoAppInternal::DisableVideoEffect
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-7-09

GOG function: CSWPartyTable::ResetCurrentSessionStartTime  gog 0x007049b0  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x005ff000 (size 54, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (DoQuickLoad 0x7d0f50,LoadGame 0x853a00)
- best decoy (neighbour/other candidate) scores: 0x005fee60:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007048d0 CSWPartyTable::UnstealthParty -> 0x005fef20  gate_class=AUTO
- GOG-order neighbour after:  0x007049f0 CSWPartyTable::GetPCFirstName -> 0x005ff040  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00927aa0 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0x20*} | Steam {0x8*,0x20*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005ff000: 16-byte aligned: YES
  005ff000  55             PUSH EBP
  005ff001  8B EC          MOV EBP,ESP
  005ff003  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005ff000
string literals (0):
direct callees (1):
  0x00920060  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005ff000 FUN_005ff000 size=54 ret=0 callers=5 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_005ff000(void)

{
  undefined8 uVar1;
  _FILETIME [GHIDRA-AUTO-LABEL: unreliable] local_c;
  
  GetSystemTimeAsFileTime(&local_c);
  uVar1 = __allshl [GHIDRA-AUTO-LABEL: unreliable]();
  _DAT_00a1b4c8 = (uint)uVar1 | local_c.dwLowDateTime;
  _DAT_00a1b4cc = (int)((ulonglong)uVar1 >> 0x20);
  return;
}
```

### decoy: 0x005fee60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fee60: 16-byte aligned: YES
  005fee60  55             PUSH EBP
  005fee61  8B EC          MOV EBP,ESP
  005fee63  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fee60
string literals (0):
direct callees (2):
  0x00920060  (unnamed)
  0x0091D220  (unnamed)
struct offsets touched (top 1):
  +0x1A8 [dec 424]  x1  via ECX
```

## Card B0-7-10

GOG function: CSWSArea::GetNextObjectInArea  gog 0x00718400  callconv=(blank)  param_size_bytes=4  gog_size=81  gog_retn=4
Candidate Steam address: 0x00523250 (size 81, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005232b0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00718240 CSWSArea::RemoveObjectFromArea -> 0x00523090  gate_class=AUTO
- GOG-order neighbour after:  0x00718460 CSWSArea::GetFirstObjectInArea -> 0x005232b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x194*,0x198*,0x1a0*} | Steam {0x194*,0x198*,0x1a0*}
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
prologue for 0x00523250: 16-byte aligned: YES
  00523250  55             PUSH EBP
  00523251  8B EC          MOV EBP,ESP
  00523253  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00523250
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1A0 [dec 416]  x2  via EAX
  +0x194 [dec 404]  x1  via EDX
  +0x198 [dec 408]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00523250 FUN_00523250 size=81 ret=4 callers=8 callees=0

bool __thiscall FUN_00523250(int param_1,undefined4 *param_2)

{
  bool bVar1;
  
  *(undefined4 *)(param_1 + 0x1a0) = 0;
  bVar1 = *(int *)(param_1 + 0x198) != 0;
  if (bVar1) {
    *param_2 = *(undefined4 *)(*(int *)(param_1 + 0x194) + *(int *)(param_1 + 0x1a0) * 4);
  }
  return bVar1;
}
```

### decoy: 0x005232b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005232b0: 16-byte aligned: YES
  005232b0  55             PUSH EBP
  005232b1  8B EC          MOV EBP,ESP
  005232b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005232b0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1A0 [dec 416]  x4  via EAX,ECX,EDX
  +0x194 [dec 404]  x1  via EAX
  +0x198 [dec 408]  x1  via EAX
```

## Card B0-7-11

GOG function: CSWSCombatRound::SetOffHandAttack  gog 0x00774c10  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x005905f0 (size 270, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCombatRound::CalculateOnHandAttacks
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005905a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00774b70 CSWSCombatRound::RemoveLastAction -> 0x005904c0  gate_class=AUTO
- GOG-order neighbour after:  0x00774c30 CSWSCombatRound::GetOffHandAttack -> 0x00590580  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xac0} | Steam {0xae0,0xb04,0x1110,0x1198}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0xb,0x18,0x1a,0x1e,0x35,0x5b,0x5c,0x108}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005905f0: 16-byte aligned: YES
  005905f0  55             PUSH EBP
  005905f1  8B EC          MOV EBP,ESP
  005905f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005905f0
string literals (0):
direct callees (3):
  0x006A7170  CSWSCreatureStats::GetAttacksPerRound
  0x005922C0  CSWSCombatRound::GetActualBonusEffectAttacksLimited
  0x005763E0  CSWSCreature::IsWieldingLightsaber
struct offsets touched (top 4):
  +0xAE0 [dec 2784]  x10  via EAX,ECX,EDX
  +0xB04 [dec 2820]  x3  via EAX,EDX
  +0x1110 [dec 4368]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005905f0 FUN_005905f0 size=270 ret=4 callers=1 callees=3

undefined4 __thiscall FUN_005905f0(int param_1,short param_2)

{
  uint uVar1;
  int iVar2;
  
  uVar1 = FUN_006a7170();
  *(uint *)(param_1 + 0xae0) = uVar1 & 0xff;
  iVar2 = FUN_005922c0();
  *(int *)(param_1 + 0xae0) = iVar2 + *(int *)(param_1 + 0xae0);
  if (((param_2 == 0xb) || (param_2 == 0x5b)) || (param_2 == 0x35)) {
    *(int *)(param_1 + 0xae0) = *(int *)(param_1 + 0xae0) + 1;
  }
  else if (((param_2 == 0x1e) || (param_2 == 0x5c)) || (param_2 == 0x1a)) {
    *(int *)(param_1 + 0xae0) = *(int *)(param_1 + 0xae0) + 1;
  }
  iVar2 = FUN_005763e0();
  if (iVar2 != 0) {
    if (*(int *)(*(int *)(param_1 + 0xb04) + 0x1110) == 0x108) {
      *(int *)(param_1 + 0xae0) = *(int *)(param_1 + 0xae0) + 1;
    }
  }
  return *(undefined4 *)(param_1 + 0xae0);
}
```

### decoy: 0x005905a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005905a0: 16-byte aligned: YES
  005905a0  55             PUSH EBP
  005905a1  8B EC          MOV EBP,ESP
  005905a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005905a0
string literals (0):
direct callees (1):
  0x005922C0  CSWSCombatRound::GetActualBonusEffectAttacksLimited
struct offsets touched (top 3):
  +0xABC [dec 2748]  x1  via ECX
  +0xAE0 [dec 2784]  x1  via EAX
  +0xAF0 [dec 2800]  x1  via EAX
```

## Card B0-7-12

GOG function: CSWPartyTable::LoadJournal  gog 0x006fd830  callconv=(blank)  param_size_bytes=8  gog_size=506  gog_retn=8
Candidate Steam address: 0x005f7cc0 (size 506, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 6 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005f7b80:STR6/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006fd6f0 CSWPartyTable::SaveJournal -> 0x005f7b80  gate_class=AUTO
- GOG-order neighbour after:  0x006fda30 CSWPartyTable::UpdateMembers -> 0x005f7ec0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (15), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006248c0 CResGFF::GetList -> Steam 0x00718760  in candidate callees: yes
  0x00624970 CResGFF::GetListCount -> Steam 0x00718810  in candidate callees: yes
  0x006249f0 CResGFF::GetListElement -> Steam 0x00718890  in candidate callees: yes
  0x00624d60 CResGFF::ReadFieldDWORD -> Steam 0x00718c00  in candidate callees: yes
  ... 7 more GOG callees
GOG strings (6): 'JNL_Time'; 'JNL_Date'; 'JNL_State'; 'JNL_PlotID' ...
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x2*,0x8*,0x60*} | Steam {0x2*,0x8*,0x60*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005f7cc0: 16-byte aligned: YES
  005f7cc0  55             PUSH EBP
  005f7cc1  8B EC          MOV EBP,ESP
  005f7cc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005f7cc0
string literals (6):
  0x009954C8  at 005f7d06  'JNL_Entries'
  0x009954B8  at 005f7d2d  'JNL_SortOrder'
  0x009954AC  at 005f7dac  'JNL_PlotID'
  0x009954A0  at 005f7df8  'JNL_State'
  0x00995494  at 005f7e2b  'JNL_Date'
  0x00995488  at 005f7e5e  'JNL_Time'
direct callees (15):
  0x005F7AE0  CSWPartyTable::GetPartyJournal
  0x00718760  CResGFF::GetList
  0x00718C80  CResGFF::ReadFieldINT
  0x00733540  CExoString::CExoString(empty)
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x006D0DB0  (unnamed)
  0x00718C00  CResGFF::ReadFieldDWORD
  0x006D21D0  (unnamed)
  0x006D2590  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005f7cc0 FUN_005f7cc0 size=506 ret=8 callers=1 callees=15

void FUN_005f7cc0(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 *puVar3;
  undefined4 uVar4;
  undefined1 local_64 [8];
  undefined1 local_5c [8];
  undefined1 local_54 [8];
  undefined4 local_4c;
  undefined4 local_48;
  int local_44;
  undefined4 local_40;
  int local_3c;
  undefined1 local_38 [4];
  int local_34;
  undefined1 local_30 [20];
  uint local_1c;
  int local_18;
  undefined1 local_14 [4];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00950f08;
  local_10 = ExceptionList;
  local_1c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = FUN_005f7ae0(local_1c);
  if (local_18 != 0) {
    local_34 = local_18;
    iVar1 = FUN_00718760(local_30,param_2,"JNL_Entries");
    if (iVar1 != 0) {
      DAT_00a7ffd0 = FUN_00718c80(param_2,"JNL_SortOrder",local_14,DAT_00a7ffd0);
      FUN_00733540();
      local_8 = 0;
      local_3c = FUN_00718810(local_30);
      for (local_44 = 0; local_44 < local_3c; local_44 = local_44 + 1) {
        FUN_00718890(local_38,local_30,local_44);
        FUN_00733570(&DAT_00986925);
        local_8._0_1_ = 1;
        uVar2 = FUN_00718f90(local_64,local_38,"JNL_PlotID",local_14,local_5c);
        local_8._0_1_ = 2;
        FUN_007337c0(uVar2);
        local_8._0_1_ = 1;
        FUN_00733780();
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        uVar2 = FUN_00718c80(local_38,"JNL_State",local_14,0);
        uVar4 = 1;
        puVar3 = local_54;
        local_40 = uVar2;
        FUN_005f7ae0(puVar3,uVar2,1);
        FUN_006d0db0(puVar3,uVar2,uVar4);
        uVar2 = FUN_00718c00(local_38,"JNL_Date",local_14,0);
        uVar4 = 1;
        puVar3 = local_54;
        local_48 = uVar2;
        FUN_005f7ae0(puVar3,uVar2,1);
        FUN_006d21d0(puVar3,uVar2,uVar4);
        uVar2 = FUN_00718c00(local_38,"JNL_Time",local_14,0);
        uVar4 = 1;
        puVar3 = local_54;
        local_4c = uVar2;
        FUN_005f7ae0(puVar3,uVar2,1);
        FUN_006d2590(puVar3,uVar2,uVar4);
      }
      local_8 = 0xffffffff;
      FUN_00733780();
    }
  }
  ExceptionList = local_10;
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_1c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x005f7b80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005f7b80: 16-byte aligned: YES
  005f7b80  55             PUSH EBP
  005f7b81  8B EC          MOV EBP,ESP
  005f7b83  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005f7b80
string literals (6):
  0x009954B8  at 005f7bbb  'JNL_SortOrder'
  0x009954C8  at 005f7bd3  'JNL_Entries'
  0x009954AC  at 005f7c1b  'JNL_PlotID'
  0x009954A0  at 005f7c3c  'JNL_State'
  0x00995494  at 005f7c5d  'JNL_Date'
  0x00995488  at 005f7c7e  'JNL_Time'
direct callees (7):
  0x005F7AE0  CSWPartyTable::GetPartyJournal
  0x00719BF0  CResGFF::WriteFieldINT
  0x00719740  CResGFF::AddList
  0x00719800  CResGFF::AddListElement
  0x00719E50  CResGFF::WriteFieldCExoString
  0x00719B90  CResGFF::WriteFieldDWORD
  0x009196EE  __security_check_cookie
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via ECX
```

## Card B0-7-13

GOG function: CSWSArea::Destructor  gog 0x00716b50  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x005219a0 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00521950:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00716560 CSWSArea::Constructor -> 0x00521360  gate_class=REVIEW
- GOG-order neighbour after:  0x00716b80 CSWSArea::Destructor_2 -> 0x005219d0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00716b80 CSWSArea::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x005219a0: 16-byte aligned: YES
  005219a0  55             PUSH EBP
  005219a1  8B EC          MOV EBP,ESP
  005219a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005219a0
string literals (0):
direct callees (2):
  0x005219D0  CSWSArea::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005219a0 FUN_005219a0 size=44 ret=4 callers=2 callees=2

undefined4 __thiscall FUN_005219a0(undefined4 param_1,uint param_2)

{
  FUN_005219d0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x00521950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00521950: 16-byte aligned: YES
  00521950  55             PUSH EBP
  00521951  8B EC          MOV EBP,ESP
  00521953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00521950
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-7-14

GOG function: CSWSArea::ComputeBestCorner  gog 0x00705280  callconv=(blank)  param_size_bytes=48  gog_size=578  gog_retn=48
Candidate Steam address: 0x0054aee0 (size 578, retn 48)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0054aa50:STR0/CALLEE0/SIG0.115/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00704df0 CSWSArea::ComputeAwayVector -> 0x0054aa50  gate_class=REVIEW
- GOG-order neighbour after:  0x007054d0 CSWSArea::ComputeHeight -> 0x0054b130  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0070aba0 CSWSArea::TestDirectLine -> Steam ?  in candidate callees: n/a
  0x00924754 Global::sqrt -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x5*,0x8*,0x30*,0x41*,0x60*} | Steam {0x5*,0x8*,0x30*,0x41*,0x60*}

### abi
```
GOG param bytes: 48; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(48/4)*4 = 48 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=48 ; asm RET = 48
Stack Slot Match: TRUE (GOG 48 bytes -> x86 slots 48; ASM RET 48)
```

### prologue
```
prologue for 0x0054aee0: 16-byte aligned: YES
  0054aee0  55             PUSH EBP
  0054aee1  8B EC          MOV EBP,ESP
  0054aee3  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054aee0
string literals (0):
direct callees (2):
  0x0091CB24  (unnamed)
  0x00550980  CSWSArea::TestDirectLine
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0054aee0 FUN_0054aee0 size=578 ret=48 callers=1 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_0054aee0(float param_1,float param_2,float param_3,float param_4,float param_5,float param_6,
            undefined4 param_7,undefined4 param_8,float *param_9,float *param_10,float *param_11,
            float *param_12)

{
  float10 fVar1;
  float local_50;
  float local_4c;
  undefined4 local_48;
  float local_44;
  float local_40;
  undefined4 local_3c;
  int local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  undefined4 local_24;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  int local_10;
  float local_c;
  float local_8;
  
  local_24 = 0;
  fVar1 = (float10)FUN_0091cb24((double)((param_5 - param_3) * (param_5 - param_3) +
                                        (param_6 - param_4) * (param_6 - param_4)));
  local_20 = (float)fVar1;
  fVar1 = (float10)FUN_0091cb24((double)((param_3 - param_1) * (param_3 - param_1) +
                                        (param_4 - param_2) * (param_4 - param_2)));
  local_1c = (float)fVar1;
  if ((local_1c < (float)_DAT_0098c1e8) || (local_20 < (float)_DAT_0098c1e8)) {
    local_24 = 0;
  }
  else {
    local_8 = ((param_4 - param_2) * (param_6 - param_4) + (param_3 - param_1) * (param_5 - param_3)
              ) / (local_1c * local_20);
    if (_DAT_0099686c < local_8 == (_DAT_0099686c == local_8)) {
      local_14 = local_1c;
      if (local_20 < local_1c) {
        local_14 = local_20;
      }
      local_18 = _DAT_0098c014;
      local_10 = 1;
      for (local_c = _DAT_0098be34; (local_10 == 1 || ((float)_DAT_00987df0 < local_c * local_14));
          local_c = local_c / (float)_DAT_00987490) {
        local_50 = (local_18 * local_14 * (param_1 - param_3)) / local_1c + param_3;
        local_4c = (local_18 * local_14 * (param_2 - param_4)) / local_1c + param_4;
        local_44 = (local_18 * local_14 * (param_5 - param_3)) / local_20 + param_3;
        local_40 = (local_18 * local_14 * (param_6 - param_4)) / local_20 + param_4;
        local_3c = 0;
        local_48 = 0;
        local_34 = local_40;
        local_30 = local_4c;
        local_2c = local_44;
        local_28 = local_50;
        local_38 = FUN_00550980(&local_50,&local_44,&param_7,param_8,1,0);
        if (local_38 == 1) {
          *param_9 = local_28;
          *param_11 = local_2c;
          *param_10 = local_30;
          *param_12 = local_34;
          local_24 = 1;
          local_18 = local_18 + local_c;
        }
        else {
          local_18 = local_18 - local_c;
        }
        local_10 = 0;
      }
    }
    else {
      local_24 = 0;
    }
/* ... 3 more lines truncated by --max-lines */
```

### decoy: 0x0054aa50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054aa50: 16-byte aligned: YES
  0054aa50  55             PUSH EBP
  0054aa51  8B EC          MOV EBP,ESP
  0054aa53  81 EC 94 01 00 00 SUB ESP,0x194
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054aa50
string literals (0):
direct callees (10):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x005143C0  (unnamed)
  0x0091EA50  (unnamed)
  0x0091E770  (unnamed)
  0x00436200  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x0045CFE0  (unnamed)
  0x00550980  CSWSArea::TestDirectLine
  0x005142F0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x1B0 [dec 432]  x1  via ECX
```

## Card B0-7-15

GOG function: CSWSArea::ComputeAwayVector  gog 0x00704df0  callconv=(blank)  param_size_bytes=20  gog_size=1168  gog_retn=20
Candidate Steam address: 0x0054aa50 (size 1168, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0054aee0:STR0/CALLEE0/SIG0.115/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00704d60 CSWPartyTable::GetNextPC -> 0x005ff3b0  gate_class=AUTO
- GOG-order neighbour after:  0x00705280 CSWSArea::ComputeBestCorner -> 0x0054aee0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x004143c0 unnamed -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0070aba0 CSWSArea::TestDirectLine -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x0080c170 Global::normalize -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x14*,0x1b0*} | Steam {0x4*,0x8*,0x14*,0x1b0*}
imm sets (* = in both, 11 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0xc*,0x14*,0x41*,0x194*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0xc*,0x14*,0x41*,0x194*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x0054aa50: 16-byte aligned: YES
  0054aa50  55             PUSH EBP
  0054aa51  8B EC          MOV EBP,ESP
  0054aa53  81 EC 94 01 00 00 SUB ESP,0x194
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054aa50
string literals (0):
direct callees (10):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x005143C0  (unnamed)
  0x0091EA50  (unnamed)
  0x0091E770  (unnamed)
  0x00436200  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x0045CFE0  (unnamed)
  0x00550980  CSWSArea::TestDirectLine
  0x005142F0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x1B0 [dec 432]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0054aa50 FUN_0054aa50 size=1168 ret=20 callers=3 callees=10

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_0054aa50(int param_1,undefined4 *param_2,undefined4 *param_3,undefined4 param_4,float param_5,
            int param_6)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  int iVar3;
  float10 fVar4;
  float10 fVar5;
  float10 fVar6;
  float10 fVar7;
  undefined1 local_d4 [12];
  undefined4 local_c8;
  undefined1 local_c4 [12];
  undefined1 local_b8 [12];
  undefined1 local_ac [12];
  undefined4 local_a0;
  undefined1 local_9c [12];
  undefined1 local_90 [12];
  undefined1 local_84 [12];
  float local_78;
  undefined1 local_74 [12];
  int local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  undefined4 local_54;
  undefined1 local_50 [12];
  int local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  float local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined1 local_24 [8];
  undefined4 local_1c;
  float local_18;
  float local_14;
  float local_10;
  int local_8;
  
  FUN_004452b0(local_24,param_4,param_3);
  fVar4 = (float10)FUN_0044b8a0(local_24,local_24);
  local_18 = (float)fVar4;
  local_1c = 0;
  FUN_005143c0(&local_14,local_24);
  local_34 = 0.0;
  local_30 = *param_3;
  local_2c = param_3[1 = byte +0x4];
  local_28 = param_3[2 = byte +0x8];
  for (local_8 = 0; local_8 < 8; local_8 = local_8 + 1) {
    local_60 = (float)local_8 * (float)_DAT_0098d890;
    fVar4 = (float10)FUN_0091ea50((double)local_60);
    fVar5 = (float10)FUN_0091e770((double)local_60);
    fVar6 = (float10)FUN_0091ea50((double)local_60);
    fVar7 = (float10)FUN_0091e770((double)local_60);
    local_5c = (float)fVar6 * local_14 + (float)fVar7 * local_10;
    local_58 = (float)fVar4 * local_10 - (float)fVar5 * local_14;
    local_54 = 0;
    uVar1 = FUN_00436200(local_84,&local_5c,param_5);
    FUN_0044c8a0(&local_40,param_3,uVar1);
    FUN_004452b0(local_50,param_4,&local_40);
    fVar4 = (float10)FUN_0044b8a0(local_50,local_50);
    if (fVar4 < (float10)(param_5 * param_5)) {
      uVar1 = FUN_0045cfe0(local_90,param_5,&local_5c);
      puVar2 = (undefined4 *)FUN_0044c8a0(local_9c,&local_40,uVar1);
      local_40 = *puVar2;
      local_3c = puVar2[1 = byte +0x4];
      local_38 = puVar2[2 = byte +0x8];
    }
    local_44 = 0;
    if (param_6 != 0) {
      *(int *)(param_1 + 0x1b0) = param_6;
/* ... 63 more lines truncated by --max-lines */
```

### decoy: 0x0054aee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054aee0: 16-byte aligned: YES
  0054aee0  55             PUSH EBP
  0054aee1  8B EC          MOV EBP,ESP
  0054aee3  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054aee0
string literals (0):
direct callees (2):
  0x0091CB24  (unnamed)
  0x00550980  CSWSArea::TestDirectLine
struct offsets touched (top 0):
```

## Card B0-7-16

GOG function: CSWPartyTable::ClearPuppet  gog 0x007037f0  callconv=(blank)  param_size_bytes=4  gog_size=42  gog_retn=4
Candidate Steam address: 0x005fde40 (size 42, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005fdc40:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007037b0 CSWPartyTable::ClearPuppets -> 0x005fde00  gate_class=REVIEW
- GOG-order neighbour after:  0x00703820 CSWPartyTable::ClearMember -> 0x005fde70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x14c*} | Steam {0x14c*}
imm sets (* = in both, 3 common): GOG {0x3*,0x4*,0x7f000000*} | Steam {0x3*,0x4*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005fde40: 16-byte aligned: YES
  005fde40  55             PUSH EBP
  005fde41  8B EC          MOV EBP,ESP
  005fde43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fde40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fde40 FUN_005fde40 size=42 ret=4 callers=1 callees=0

void __thiscall FUN_005fde40(int param_1,int param_2)

{
  if ((-1 < param_2) && (param_2 < 3)) {
    *(undefined4 *)(param_1 + 0x14c + param_2 * 4) = 0x7f000000;
  }
  return;
}
```

### decoy: 0x005fdc40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fdc40: 16-byte aligned: YES
  005fdc40  55             PUSH EBP
  005fdc41  8B EC          MOV EBP,ESP
  005fdc43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fdc40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-7-17

GOG function: CSWPartyTable::AddPUP_2  gog 0x007002b0  callconv=(blank)  param_size_bytes=8  gog_size=325  gog_retn=8
Candidate Steam address: 0x005fa740 (size 325, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005fa600:STR0/CALLEE0/SIG0.357/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00700170 CSWPartyTable::AddNPC_2 -> 0x005fa600  gate_class=REVIEW
- GOG-order neighbour after:  0x00700400 CSWPartyTable::AssignPUP -> 0x005fa890  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0061a160 CResRef::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x0067ab60 CSWSCreature::Constructor -> Steam ?  in candidate callees: n/a
  0x0068b5a0 CSWSCreature::LoadFromTemplate -> Steam ?  in candidate callees: n/a
  0x006fff20 CSWPartyTable::AddPUP -> Steam ?  in candidate callees: n/a
  0x00700500 CSWPartyTable::GetIsPUPAvailable -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x3*,0x4*,0x8*,0x40*,0x1220*,0x7f000000*} | Steam {0x3*,0x4*,0x8*,0x40*,0x1220*,0x7f000000*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005fa740: 16-byte aligned: YES
  005fa740  55             PUSH EBP
  005fa741  8B EC          MOV EBP,ESP
  005fa743  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005fa740
string literals (0):
direct callees (6):
  0x005FA990  CSWPartyTable::GetIsPUPAvailable
  0x00919723  operator new
  0x00561F30  CSWSCreature::Constructor
  0x00710810  (unnamed)
  0x00572A70  CSWSCreature::LoadFromTemplate
  0x005FA3B0  CSWPartyTable::AddPUP
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fa740 FUN_005fa740 size=325 ret=8 callers=1 callees=6

undefined4 FUN_005fa740(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 *local_48;
  undefined1 local_30 [16];
  int local_20;
  undefined4 *local_1c;
  undefined4 local_18;
  undefined4 *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009549bb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (((param_1 < 0) || (2 < param_1)) || (iVar2 = FUN_005fa990(param_1), iVar2 != 0)) {
    local_18 = 0;
  }
  else {
    local_20 = FUN_00919723(0x1220,uVar1);
    local_8 = 0;
    if (local_20 == 0) {
      local_48 = (undefined4 *)0x0;
    }
    else {
      local_48 = (undefined4 *)FUN_00561f30(0x7f000000,0);
    }
    local_1c = local_48;
    local_8 = 0xffffffff;
    local_14 = local_48;
    FUN_00710810(param_2);
    iVar2 = FUN_00572a70(local_30,0);
    if (iVar2 == 0) {
      if (local_14 != (undefined4 *)0x0) {
        (**(code **)*local_14)(1);
      }
      local_18 = 0;
    }
    else {
      local_18 = FUN_005fa3b0(param_1,local_14);
      if (local_14 != (undefined4 *)0x0) {
        (**(code **)*local_14)(1);
      }
    }
  }
  ExceptionList = local_10;
  return local_18;
}
```

### decoy: 0x005fa600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fa600: 16-byte aligned: YES
  005fa600  55             PUSH EBP
  005fa601  8B EC          MOV EBP,ESP
  005fa603  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005fa600
string literals (0):
direct callees (6):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
  0x00919723  operator new
  0x00561F30  CSWSCreature::Constructor
  0x00710810  (unnamed)
  0x00572A70  CSWSCreature::LoadFromTemplate
  0x005F9FF0  CSWPartyTable::AddNPC
struct offsets touched (top 0):
```

## Card B0-7-18

GOG function: CSWSArea::LoadRooms  gog 0x0071ab60  callconv=(blank)  param_size_bytes=4  gog_size=570  gog_retn=0
Candidate Steam address: 0x005259b0 (size 570, retn 0)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSArea::LoadRoomInfo
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00525bf0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0071aa50 CSWSArea::LoadRoomInfo -> 0x005258a0  gate_class=AUTO
- GOG-order neighbour after:  0x0071ada0 CSWSRoom::`scalar  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x005623b0 CSWRoom::SetRoomInfo -> Steam ?  in candidate callees: n/a
  0x006270e0 CSWCJournal::GetNumActiveEntries -> Steam ?  in candidate callees: n/a
  0x0062a660 CLYT::GetRoomName -> Steam ?  in candidate callees: n/a
  0x0062a6a0 CLYT::GetRoomPosition -> Steam ?  in candidate callees: n/a
  0x00734050 CSWSRoom::LoadWalkMesh -> Steam ?  in candidate callees: n/a
  0x00734280 CSWSRoom::TransformToWorld -> Steam ?  in candidate callees: n/a
  0x007342e0 CSWSRoom::WeldRoom -> Steam ?  in candidate callees: n/a
  0x00734360 CSWSRoom::ComputeMeshNormals -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x40*,0x4c*} | Steam {0x4*,0x40*,0x4c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x005259b0: 16-byte aligned: YES
  005259b0  55             PUSH EBP
  005259b1  8B EC          MOV EBP,ESP
  005259b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005259b0
string literals (0):
direct callees (11):
  0x0084B570  (unnamed)
  0x00919723  operator new
  0x0091D67E  (unnamed)
  0x00720730  (unnamed)
  0x007206F0  (unnamed)
  0x005FFC00  (unnamed)
  0x005FF590  (unnamed)
  0x005FF7C0  (unnamed)
  0x005FF820  (unnamed)
  0x005FF8D0  (unnamed)
  0x005FF8A0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005259b0 FUN_005259b0 size=570 ret=0 callers=1 callees=11

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint * FUN_005259b0(void)

{
  undefined4 uVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  uint *local_50;
  undefined1 local_4c [16];
  undefined1 local_3c [12];
  uint *local_30;
  uint *local_2c;
  uint local_28;
  uint local_24;
  int local_20;
  int local_1c;
  uint *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959f5b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_28 = FUN_0084b570(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (local_28 == 0) {
    local_18 = (uint *)0x0;
  }
  else {
    uVar4 = -(uint)((int)((ulonglong)local_28 * 0x4c >> 0x20) != 0) |
            (uint)((ulonglong)local_28 * 0x4c);
    local_24 = local_28;
    local_30 = (uint *)FUN_00919723(-(uint)(0xfffffffb < uVar4) | uVar4 + 4);
    local_8 = 0;
    if (local_30 == (uint *)0x0) {
      local_50 = (uint *)0x0;
    }
    else {
      *local_30 = local_28;
      _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_30 + 0x1 [dec 1] {uint*: byte +0x4},0x4c,local_28,FUN_005ff440,FUN_005ff4e0);
      local_50 = local_30 + 0x1 [dec 1] {uint*: byte +0x4};
    }
    local_2c = local_50;
    local_8 = 0xffffffff;
    local_18 = local_50;
    for (local_1c = 0; local_1c < (int)local_24; local_1c = local_1c + 1) {
      uVar1 = FUN_00720730(local_3c,local_1c);
      uVar2 = FUN_007206f0(local_4c,local_1c);
      FUN_005ffc00(uVar2,uVar1);
      FUN_005ff590(0);
      FUN_005ff7c0();
    }
    local_14 = 0;
    for (local_1c = 0; iVar3 = local_1c, local_1c < (int)local_24; local_1c = local_1c + 1) {
      while (local_20 = iVar3 + 1, local_20 < (int)local_24) {
        iVar3 = FUN_005ff820(local_18 + local_20 * 0x13,_DAT_0098c628);
        local_14 = iVar3 + local_14;
        iVar3 = local_20;
      }
    }
    local_14 = 0;
    for (local_1c = 0; iVar3 = local_1c, local_1c < (int)local_24; local_1c = local_1c + 1) {
      while (local_20 = iVar3 + 1, local_20 < (int)local_24) {
        iVar3 = FUN_005ff8d0(local_18 + local_20 * 0x13,local_1c,local_20);
        local_14 = iVar3 + local_14;
        iVar3 = local_20;
      }
    }
    for (local_1c = 0; local_1c < (int)local_24; local_1c = local_1c + 1) {
      FUN_005ff8a0();
    }
  }
  ExceptionList = local_10;
  return local_18;
}
```

### decoy: 0x00525bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00525bf0: 16-byte aligned: YES
  00525bf0  55             PUSH EBP
  00525bf1  8B EC          MOV EBP,ESP
  00525bf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00525bf0
string literals (0):
direct callees (3):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
  0x005FF4E0  (unnamed)
struct offsets touched (top 0):
```

## Card B0-7-19

GOG function: CSWPartyTable::GetNPCSelectability  gog 0x00700cb0  callconv=(blank)  param_size_bytes=4  gog_size=42  gog_retn=4
Candidate Steam address: 0x005fb140 (size 42, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fb170:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00700c60 CSWPartyTable::GetIsAvailable -> 0x005fb0f0  gate_class=AUTO
- GOG-order neighbour after:  0x00700ce0 CSWPartyTable::SetNPCSelectability -> 0x005fb170  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007004d0 CSWPartyTable::GetIsNPCAvailable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x7c*} | Steam {0x7c*}
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
prologue for 0x005fb140: 16-byte aligned: YES
  005fb140  55             PUSH EBP
  005fb141  8B EC          MOV EBP,ESP
  005fb143  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fb140
string literals (0):
direct callees (1):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
struct offsets touched (top 1):
  +0x7C [dec 124]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005fb140 FUN_005fb140 size=42 ret=4 callers=3 callees=1

undefined1 __thiscall FUN_005fb140(int param_1,int param_2)

{
  undefined1 uVar1;
  int iVar2;
  
  iVar2 = FUN_005fa960(param_2);
  if (iVar2 == 0) {
    uVar1 = 0xff;
  }
  else {
    uVar1 = *(undefined1 *)(param_1 + param_2 + 0x7c);
  }
  return uVar1;
}
```

### decoy: 0x005fb170 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fb170: 16-byte aligned: YES
  005fb170  55             PUSH EBP
  005fb171  8B EC          MOV EBP,ESP
  005fb173  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fb170
string literals (0):
direct callees (1):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
struct offsets touched (top 1):
  +0x7C [dec 124]  x1  via ECX
```

## Card B0-7-20

GOG function: CSWPartyTable::GetPCLastName  gog 0x00704b00  callconv=(blank)  param_size_bytes=4  gog_size=343  gog_retn=4
Candidate Steam address: 0x005ff150 (size 343, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005ff040:STR0/CALLEE1/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007049f0 CSWPartyTable::GetPCFirstName -> 0x005ff040  gate_class=REVIEW
- GOG-order neighbour after:  0x00704c70 CSWPartyTable::GetPUPOwner -> 0x005ff2c0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00512940 unnamed -> Steam ?  in candidate callees: n/a
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006062a0 CExoString::Find_2 -> Steam ?  in candidate callees: n/a
  0x00606b10 CExoString::SubString -> Steam 0x00734a00  in candidate callees: yes
  0x00921b40 Global::strlen -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x4*,0x20*,0x30*,0x240*} | Steam {0x2*,0x4*,0x20*,0x30*,0x240*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005ff150: 16-byte aligned: YES
  005ff150  55             PUSH EBP
  005ff151  8B EC          MOV EBP,ESP
  005ff153  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ff150
string literals (0):
direct callees (7):
  0x007335F0  CExoString::CExoString_copy
  0x00919F10  _strlen
  0x00734190  CExoString::Find_2
  0x00734A00  CExoString::SubString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x005355F0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005ff150 FUN_005ff150 size=343 ret=4 callers=1 callees=7

undefined4 __thiscall FUN_005ff150(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  size_t local_34;
  size_t local_30;
  undefined1 local_24 [8];
  int local_1c;
  char *local_18 [2];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954cb9;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_007335f0(param_1 + 0x240);
  local_8 = 1;
  if (*(int *)(param_1 + 0x240) == 0) {
    local_30 = 0;
  }
  else {
    local_30 = _strlen [GHIDRA-AUTO-LABEL: unreliable](*(char **)(param_1 + 0x240));
  }
  if (0 < (int)local_30) {
    local_1c = FUN_00734190(0x20,0);
    if (local_1c != -1) {
      uVar1 = FUN_00734a00(local_24,local_1c + 1,0xffffffff);
      local_8._0_1_ = 2;
      FUN_007337c0(uVar1);
      local_8 = CONCAT31(local_8._1_3_,1);
      FUN_00733780();
    }
  }
  if (local_18[0] == (char *)0x0) {
    local_34 = 0;
  }
  else {
    local_34 = _strlen [GHIDRA-AUTO-LABEL: unreliable](local_18[0]);
  }
  if ((int)local_34 < 1) {
    FUN_005355f0(param_2);
    local_8 = local_8 & 0xffffff00;
    FUN_00733780();
  }
  else {
    FUN_007335f0(local_18);
    local_8 = local_8 & 0xffffff00;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return param_2;
}
```

### decoy: 0x005ff040 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ff040: 16-byte aligned: YES
  005ff040  55             PUSH EBP
  005ff041  8B EC          MOV EBP,ESP
  005ff043  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ff040
string literals (0):
direct callees (6):
  0x007335F0  CExoString::CExoString_copy
  0x00919F10  _strlen
  0x00734190  CExoString::Find_2
  0x00734A00  CExoString::SubString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B0-7-21

GOG function: CExoResMan::GetResRefFromFile  gog 0x0061c540  callconv=(blank)  param_size_bytes=8  gog_size=175  gog_retn=8
Candidate Steam address: 0x00712970 (size 481, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.11 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00712920:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061c4c0 CExoResMan::SetResObject -> 0x00712b60  gate_class=AUTO
- GOG-order neighbour after:  0x0061c5f0 CExoResMan::GetResTypeFromFile -> 0x00712c90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006062a0 CExoString::Find_2 -> Steam ?  in candidate callees: n/a
  0x00606b10 CExoString::SubString -> Steam 0x00734a00  in candidate callees: no
  0x0061a710 CResRef::operator=_3 -> Steam ?  in candidate callees: n/a
  0x0061a800 CResRef::operator=_4 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x10,0x1a,0x1c}
imm sets (* = in both, 2 common): GOG {0x8*,0x10,0x2e,0x38*} | Steam {0x2,0x3,0x4,0x8*,0x1c,0x20,0x38*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00712970: 16-byte aligned: YES
  00712970  55             PUSH EBP
  00712971  8B EC          MOV EBP,ESP
  00712973  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00712970
string literals (0):
direct callees (9):
  0x00712360  CExoResMan::GetKeyEntry
  0x00733540  CExoString::CExoString(empty)
  0x007338D0  CExoString::operator=_2
  0x007122C0  CExoResMan::GetKeyEntryFromTable
  0x007360A0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00710930  (unnamed)
  0x00734010  CExoString::CStr
  0x00733780  CExoString::~CExoString
struct offsets touched (top 3):
  +0x10 [dec 16]  x6  via EAX,ECX,EDX
  +0x1C [dec 28]  x5  via EAX,ECX,EDX
  +0x1A [dec 26]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00712970 FUN_00712970 size=481 ret=8 callers=25 callees=9

undefined4 __thiscall FUN_00712970(int param_1,undefined4 param_2,undefined2 param_3)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_40 [4];
  undefined1 local_3c [4];
  undefined4 local_38;
  int local_2c;
  undefined1 local_28 [16];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095f9c8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = 0;
  local_14 = 0;
  iVar2 = FUN_00712360(param_2,param_3,&local_18,&local_14);
  if (iVar2 == 0) {
    uVar3 = 0;
  }
  else {
    if (*(int *)(local_14 + 0x10) != 0) {
      *(int *)(*(int *)(local_14 + 0x10) + 0x1c) = *(int *)(*(int *)(local_14 + 0x10) + 0x1c) + 1;
    }
    if ((*(int *)(local_18 + 0x1c) != 4) &&
       ((*(int *)(local_14 + 0x10) == 0 || (*(int *)(*(int *)(local_14 + 0x10) + 0x1c) == 1)))) {
      FUN_00733540(uVar1);
      local_8 = 0;
      FUN_00733540();
      local_8._0_1_ = 1;
      FUN_00733540();
      local_8 = CONCAT31(local_8._1_3_,2);
      local_2c = 1;
      iVar2 = *(int *)(local_18 + 0x1c);
      if (iVar2 == 1) {
        FUN_007338d0(&DAT_0099c3dc);
        local_2c = 0;
      }
      else if (iVar2 == 2) {
        local_38 = 4;
        iVar2 = FUN_007122c0(param_1 + 0x1c,param_2,param_3,local_40,local_3c,0);
        if (iVar2 == 0) {
          FUN_007338d0(&DAT_00987d30);
        }
        else {
          local_2c = 0;
        }
      }
      else if (iVar2 == 3) {
        FUN_007338d0(&DAT_0099c3d8);
      }
      if (local_2c != 0) {
        uVar3 = FUN_007360a0(*(undefined2 *)(local_14 + 0x1a));
        FUN_007337c0(uVar3);
        FUN_00710930(local_28);
        FUN_00734010();
        FUN_00734010();
        FUN_00734010();
        FUN_00734010();
      }
      local_8._0_1_ = 1;
      FUN_00733780();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    uVar3 = *(undefined4 *)(local_14 + 0x10);
  }
  ExceptionList = local_10;
  return uVar3;
/* ... 1 more lines truncated by --max-lines */
```

### decoy: 0x00712920 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00712920: 16-byte aligned: YES
  00712920  55             PUSH EBP
  00712921  8B EC          MOV EBP,ESP
  00712923  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00712920
string literals (0):
direct callees (2):
  0x00712D70  CExoResMan::GetTable
  0x00726790  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-7-22

GOG function: CSWPartyTable::RemoveNPC  gog 0x00700490  callconv=(blank)  param_size_bytes=4  gog_size=52  gog_retn=4
Candidate Steam address: 0x005fa920 (size 52, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fa960:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00700400 CSWPartyTable::AssignPUP -> 0x005fa890  gate_class=AUTO
- GOG-order neighbour after:  0x007004d0 CSWPartyTable::GetIsNPCAvailable -> 0x005fa960  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007004d0 CSWPartyTable::GetIsNPCAvailable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4c*} | Steam {0x4c*}
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
prologue for 0x005fa920: 16-byte aligned: YES
  005fa920  55             PUSH EBP
  005fa921  8B EC          MOV EBP,ESP
  005fa923  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fa920
string literals (0):
direct callees (1):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fa920 FUN_005fa920 size=52 ret=4 callers=1 callees=1

bool __thiscall FUN_005fa920(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = FUN_005fa960(param_2);
  if (iVar1 != 0) {
    *(undefined4 *)(param_1 + 0x4c + param_2 * 4) = 0;
  }
  return iVar1 != 0;
}
```

### decoy: 0x005fa960 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fa960: 16-byte aligned: YES
  005fa960  55             PUSH EBP
  005fa961  8B EC          MOV EBP,ESP
  005fa963  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fa960
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-7-23

GOG function: CSWCAreaOfEffectObject::Destructor  gog 0x005821d0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x008eef70 (size 5528, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCAreaOfEffectObject::LoadEffect
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.02 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008f0510:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00581f60 CSWCAreaOfEffectObject::Constructor -> 0x008ee580  gate_class=AUTO
- GOG-order neighbour after:  0x00582200 CSWCAreaOfEffectObject::Destructor_2 -> 0x008ee820  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00582200 CSWCAreaOfEffectObject::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0xc,0x1c,0x20,0x44,0x68,0x78,0x80,0xb0,0xfc,0x100,0x104,0x114,0x118,...(+15)}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x3,0x4*,0x5,0x6,0x7,0x8,0x43,0x52,0x64,0xfc,0xff,0x108,0x10c,...(+14)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008eef70: 16-byte aligned: YES
  008eef70  55             PUSH EBP
  008eef71  8B EC          MOV EBP,ESP
  008eef73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008eef70
string literals (1):
  0x0099423C  at 008f045d  'DEFAULT'
direct callees (17):
  0x0091D67E  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x0071B8B0  (unnamed)
  0x0071AFA0  C2DA::GetCExoStringEntry
  0x0071B2E0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00919723  operator new
  0x0086D770  (unnamed)
  0x00710810  (unnamed)
  0x0086E420  (unnamed)
  0x00734010  CExoString::CStr
  0x0071B3F0  (unnamed)
  0x0072FB00  OptionsHolder::GetGameOptions (returns *(*(this+4)+4); +8 bit0 = Auto Level Up NPCs)
  0x00710DC0  (unnamed)
  0x00861400  CSWCAnimBase::Hilite
  0x00710840  (unnamed)
struct offsets touched (top 10):
  +0x130 [dec 304]  x32  via EAX,ECX,EDX
  +0x20 [dec 32]  x26  via EAX,ECX,EDX
  +0x68 [dec 104]  x11  via EAX,ECX,EDX,ESI
  +0x120 [dec 288]  x9  via EAX,ECX,EDX
  +0x1C [dec 28]  x6  via EAX,ECX,EDX
  +0x80 [dec 128]  x6  via EDX
  +0x100 [dec 256]  x6  via EAX,ECX,EDX
  +0x13C [dec 316]  x5  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,EDX
  +0xFC [dec 252]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008eef70 FUN_008eef70 size=5528 ret=4 callers=1 callees=17

/* WARNING: Removing unreachable block (ram,0x008efa58) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_008eef70(int *param_1,int param_2)

{
  undefined1 uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  uint *puVar5;
  uint *local_280;
  int local_27c;
  undefined1 local_26c;
  int *local_25c;
  undefined1 local_148 [16];
  undefined4 local_138;
  undefined4 local_134;
  undefined4 local_130;
  undefined1 local_12c [16];
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined1 local_110 [16];
  undefined4 local_100;
  undefined4 local_fc;
  undefined4 local_f8;
  undefined1 local_f4 [16];
  undefined4 local_e4;
  int local_e0;
  undefined4 local_dc;
  int local_d8;
  undefined4 local_d4;
  int local_d0;
  undefined4 local_cc;
  int local_c8;
  undefined4 local_c4;
  int local_c0;
  undefined4 local_bc;
  uint *local_b8;
  uint *local_b4;
  uint local_b0;
  undefined4 local_ac;
  undefined1 local_a8 [16];
  int *local_98;
  int *local_94;
  undefined1 local_90 [16];
  int local_80;
  int *local_7c;
  undefined4 local_78;
  undefined1 local_74 [8];
  int local_6c;
  int *local_68;
  int local_64;
  undefined1 local_58 [8];
  float local_50;
  undefined1 local_4c [4];
  undefined1 local_48 [4];
  int local_44;
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  float local_30;
  undefined1 local_2c [8];
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097e044;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_2c,8,3,FUN_00733540,FUN_00733780);
  local_8 = 0;
  FUN_00733540(uVar2);
  local_8._0_1_ = 1;
/* ... 387 more lines truncated by --max-lines */
```

### decoy: 0x008f0510 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008f0510: 16-byte aligned: YES
  008f0510  55             PUSH EBP
  008f0511  8B EC          MOV EBP,ESP
  008f0513  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008f0510
string literals (0):
direct callees (3):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
  0x008F8540  (unnamed)
struct offsets touched (top 0):
```

## Card B0-7-24

GOG function: CSWCArea::Constructor  gog 0x0044ef30  callconv=__thiscall  param_size_bytes=0  gog_size=1741  gog_retn=0
Candidate Steam address: 0x007a2570 (size 28, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2590:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ef10 Global::PlayRoomSoundEvent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044f600 CSWArea::GetIsIDInExcludeList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam ?  in candidate callees: n/a
  0x00457ac0 CExoArrayList_Vector_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x004692d0 CGameObject::Constructor -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00556550 CSWCAmbientSound::Constructor -> Steam ?  in candidate callees: n/a
  0x00559d20 Global::CSWArea -> Steam ?  in candidate callees: n/a
  0x0055a020 Global::InitializeStaticObjects -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  ... 6 more GOG callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xa4,0x104,0x188,0x18c,0x190,0x1a8,0x1b8,0x1bc,0x1c0,0x1c4,0x1e0,0x1e4,...(+23)} | Steam {}
imm sets (* = in both, 0 common): GOG {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0x90,...(+23)} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007a2570: 16-byte aligned: YES
  007a2570  55             PUSH EBP
  007a2571  8B EC          MOV EBP,ESP
  007a2573  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2570
string literals (0):
direct callees (1):
  0x00893420  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a2570 FUN_007a2570 size=28 ret=0 callers=0 callees=1

void FUN_007a2570(undefined4 param_1,undefined4 param_2,int param_3)

{
  if (param_3 != 0) {
    FUN_00893420();
  }
  return;
}
```

### decoy: 0x007a2590 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2590: 16-byte aligned: YES
  007a2590  55             PUSH EBP
  007a2591  8B EC          MOV EBP,ESP
  007a2593  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007a2590
string literals (0):
direct callees (14):
  0x005E7750  Global::CSWArea
  0x0053D660  (unnamed)
  0x007107F0  (unnamed)
  0x0084A640  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x00722870  (unnamed)
  0x00428F80  (unnamed)
  0x00449230  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x005E7A50  Global::InitializeStaticObjects
  0x00919723  operator new
  0x0088FD20  (unnamed)
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
struct offsets touched (top 10):
  +0x4 [dec 4]  x15  via EAX,ECX,EDX
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0xA4 [dec 164]  x1  via ECX
  +0x104 [dec 260]  x1  via EDX
  +0x188 [dec 392]  x1  via ECX
  +0x18C [dec 396]  x1  via EDX
  +0x1A8 [dec 424]  x1  via ECX
  +0x1B8 [dec 440]  x1  via EDX
  +0x1BC [dec 444]  x1  via EAX
  +0x1C0 [dec 448]  x1  via ECX
```

## Card B0-7-25

GOG function: CSWPartyTable::Save  gog 0x00702df0  callconv=(blank)  param_size_bytes=8  gog_size=51  gog_retn=8
Candidate Steam address: 0x005fd440 (size 51, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fd040:STR0/CALLEE0/SIG0.024/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00701d10 CSWPartyTable::LoadTableInfo -> 0x005fbf60  gate_class=REVIEW
- GOG-order neighbour after:  0x00702e30 CSWPartyTable::SpawnNPC -> 0x005fd480  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x006fda30 CSWPartyTable::UpdateMembers -> Steam ?  in candidate callees: n/a
  0x006fe280 CSWPartyTable::UpdateInventory -> Steam ?  in candidate callees: n/a
  0x00700d10 CSWPartyTable::SaveTableInfo -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x005fd440: 16-byte aligned: YES
  005fd440  55             PUSH EBP
  005fd441  8B EC          MOV EBP,ESP
  005fd443  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fd440
string literals (0):
direct callees (3):
  0x005F7EC0  CSWPartyTable::UpdateMembers
  0x005F8710  CSWPartyTable::UpdateInventory
  0x005FB1A0  CSWPartyTable::SaveTableInfo
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fd440 FUN_005fd440 size=51 ret=8 callers=1 callees=3

void FUN_005fd440(undefined4 param_1,int param_2)

{
  if (param_2 != 0) {
    FUN_005f7ec0(0);
    FUN_005f8710(0);
  }
  FUN_005fb1a0(param_1);
  return;
}
```

### decoy: 0x005fd040 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fd040: 16-byte aligned: YES
  005fd040  55             PUSH EBP
  005fd041  8B EC          MOV EBP,ESP
  005fd043  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fd040
string literals (20):
  0x00994430  at 005fd056  'DISPLAYSPENDING'
  0x00994420  at 005fd090  'ITEMRECEIVED'
  0x00994414  at 005fd0cd  'ITEMLOST'
  0x0099440C  at 005fd10a  'JOURNAL'
  0x00994400  at 005fd143  'LIGHTSHIFT'
  0x009943F4  at 005fd166  'DARKSHIFT'
  0x009943EC  at 005fd188  'CREDITS'
  0x009943E0  at 005fd1af  'CREDITSNET'
  0x009943D0  at 005fd20c  'STEALTHXP'
  0x009943C0  at 005fd232  'SOUNDPENDING'
  0x009943B0  at 005fd26e  'LEVELUPSOUND'
  0x009943A0  at 005fd2ab  'NEWQUESTSOUND'
direct callees (2):
  0x00718A00  CResGFF::ReadFieldBYTE
  0x00718C80  CResGFF::ReadFieldINT
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x5 [dec 5]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x10 [dec 16]  x2  via EAX,ECX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x1C [dec 28]  x1  via ECX
  +0x20 [dec 32]  x1  via ECX
  +0x24 [dec 36]  x1  via ECX
```

## Card B0-7-26

GOG function: CSWPartyTable::SavePuppetByObject  gog 0x006fe090  callconv=(blank)  param_size_bytes=8  gog_size=491  gog_retn=8
Candidate Steam address: 0x005f8520 (size 491, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005f8130:STR0/CALLEE3/SIG0.95/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006fde90 CSWPartyTable::SavePuppet -> 0x005f8320  gate_class=AUTO
- GOG-order neighbour after:  0x006fe280 CSWPartyTable::UpdateInventory -> 0x005f8710  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00622ff0 CResGFF::Constructor -> Steam 0x00716e60  in candidate callees: yes
  0x00626530 CResGFF::CreateGFFFile -> Steam 0x0071a3d0  in candidate callees: yes
  0x00626700 CResGFF::WriteGFFFile -> Steam 0x0071a5a0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  ... 4 more GOG callees
GOG strings (2): 'GAMEINPROGRESS:'; 'UTC '
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 9 common): GOG {0x2*,0x3*,0x4*,0x6*,0x7*,0x8*,0x64*,0xa0*,0x7eb*} | Steam {0x2*,0x3*,0x4*,0x6*,0x7*,0x8*,0x64*,0xa0*,0x7eb*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005f8520: 16-byte aligned: YES
  005f8520  55             PUSH EBP
  005f8521  8B EC          MOV EBP,ESP
  005f8523  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005f8520
string literals (1):
  0x00993E84  at 005f8644  'GAMEINPROGRESS:'
direct callees (12):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00541080  CSWSObject::ClearAllActions
  0x005F9190  CSWPartyTable::GetPupFilename
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00574500  CSWSCreature::SaveCreature
  0x00733DF0  CExoString::operator+
  0x00734010  CExoString::CStr
  0x0071A5A0  CResGFF::WriteGFFFile
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005f8520 FUN_005f8520 size=491 ret=8 callers=1 callees=12

void FUN_005f8520(int param_1,undefined4 param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 *local_68;
  undefined1 local_54 [16];
  undefined1 local_44 [8];
  undefined1 local_3c [8];
  int local_34;
  undefined4 *local_30;
  undefined1 local_2c [8];
  undefined1 local_24 [8];
  undefined4 *local_1c;
  undefined1 local_18 [4];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009547cb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = 0;
  if ((-1 < param_1) && (param_1 < 3)) {
    local_14 = FUN_0051c100(param_2);
    if (local_14 != 0) {
      FUN_00541080(1);
      FUN_005f9190(local_24,param_1);
      local_8 = 0;
      local_34 = FUN_00919723(0xa0,uVar1);
      local_8._0_1_ = 1;
      if (local_34 == 0) {
        local_68 = (undefined4 *)0x0;
      }
      else {
        local_68 = (undefined4 *)FUN_00716e60();
      }
      local_30 = local_68;
      local_8._0_1_ = 0;
      local_1c = local_68;
      FUN_00733570(&DAT_009938e4);
      local_8._0_1_ = 2;
      FUN_00733570(&DAT_00995250);
      local_8._0_1_ = 3;
      FUN_0071a3d0(local_18,local_44,local_3c);
      local_8._0_1_ = 2;
      FUN_00733780();
      local_8._0_1_ = 0;
      FUN_00733780();
      FUN_00574500(local_1c,local_18);
      FUN_00733570("GAMEINPROGRESS:");
      local_8._0_1_ = 4;
      FUN_00733df0(local_2c,local_24);
      local_8._0_1_ = 6;
      FUN_00733780();
      uVar2 = FUN_00734010();
      FUN_00733570(uVar2);
      local_8._0_1_ = 7;
      FUN_0071a5a0(local_54,0x7eb);
      local_8 = CONCAT31(local_8._1_3_,6);
      FUN_00733780();
      if (local_1c == (undefined4 *)0x0) {
        uVar2 = 0;
      }
      else {
        uVar2 = (**(code **)*local_1c)(1);
      }
      local_1c = (undefined4 *)0x0;
      local_8 = local_8 & 0xffffff00;
      FUN_00733780(uVar1,uVar2);
      local_8 = 0xffffffff;
      FUN_00733780();
    }
  }
  ExceptionList = local_10;
  return;
/* ... 1 more lines truncated by --max-lines */
```

### decoy: 0x005f8130 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005f8130: 16-byte aligned: YES
  005f8130  55             PUSH EBP
  005f8131  8B EC          MOV EBP,ESP
  005f8133  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005f8130
string literals (1):
  0x00993E84  at 005f8254  'GAMEINPROGRESS:'
direct callees (12):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00541080  CSWSObject::ClearAllActions
  0x005F9100  CSWPartyTable::GetFilename
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00574500  CSWSCreature::SaveCreature
  0x00733DF0  CExoString::operator+
  0x00734010  CExoString::CStr
  0x0071A5A0  CResGFF::WriteGFFFile
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B0-7-27

GOG function: CSWSArea::GridDFSTransTableInitialize  gog 0x00709770  callconv=(blank)  param_size_bytes=0  gog_size=518  gog_retn=0
Candidate Steam address: 0x0054f3d0 (size 518, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0054f5e0:STR0/CALLEE0/SIG0.439/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00709200 CSWSArea::SmoothCornerOptimize -> 0x0054ee60  gate_class=AUTO
- GOG-order neighbour after:  0x00709980 CSWSArea::SmoothPointsOnPath -> 0x0054f5e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00709200 CSWSArea::SmoothCornerOptimize -> Steam ?  in candidate callees: n/a
  0x00709fd0 CSWSArea::SmoothSelectNodes -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x1c*,0x8c*,0x90*,0x94*,0x98*,0x1b0*} | Steam {0x1c*,0x8c*,0x90*,0x94*,0x98*,0x1b0*}
imm sets (* = in both, 2 common): GOG {0x4*,0x60*} | Steam {0x4*,0x60*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0054f3d0: 16-byte aligned: YES
  0054f3d0  55             PUSH EBP
  0054f3d1  8B EC          MOV EBP,ESP
  0054f3d3  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054f3d0
string literals (0):
direct callees (4):
  0x009196FD  operator delete
  0x00919723  operator new
  0x0054FDB0  CSWSArea::SmoothSelectNodes
  0x0054EE60  CSWSArea::SmoothCornerOptimize
struct offsets touched (top 6):
  +0x1B0 [dec 432]  x13  via EAX,ECX,EDX
  +0x90 [dec 144]  x6  via EAX,ECX,EDX
  +0x8C [dec 140]  x4  via EAX
  +0x1C [dec 28]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x98 [dec 152]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0054f3d0 FUN_0054f3d0 size=518 ret=0 callers=2 callees=4

undefined4 __fastcall FUN_0054f3d0(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 local_1c;
  undefined4 local_18;
  uint local_14;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  
  if (*(int *)(*(int *)(param_1 + 0x1b0) + 0x90) != 0) {
    FUN_009196fd(*(undefined4 *)(*(int *)(param_1 + 0x1b0) + 0x90));
    *(undefined4 *)(*(int *)(param_1 + 0x1b0) + 0x90) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x1b0) + 0x8c) = 0;
  }
  local_14 = *(uint *)(*(int *)(param_1 + 0x1b0) + 0x94);
  iVar1 = *(int *)(*(int *)(param_1 + 0x1b0) + 0x98);
  if (local_14 == 0) {
    *(undefined4 *)(*(int *)(param_1 + 0x1b0) + 0x8c) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x1b0) + 0x90) = 0;
  }
  else if (*(int *)(*(int *)(param_1 + 0x1b0) + 0x1c) == 1) {
    iVar2 = FUN_00919723(-(uint)((int)((ulonglong)local_14 * 4 >> 0x20) != 0) |
                         (uint)((ulonglong)local_14 * 4));
    for (local_c = 0; local_c < (int)local_14; local_c = local_c + 1) {
      *(undefined4 *)(iVar2 + local_c * 4) = *(undefined4 *)(iVar1 + local_c * 4);
    }
    *(int *)(*(int *)(param_1 + 0x1b0) + 0x90) = iVar2;
    *(uint *)(*(int *)(param_1 + 0x1b0) + 0x8c) = local_14;
  }
  else {
    FUN_0054fdb0(local_14,iVar1,&local_10,&local_8);
    FUN_0054ee60(local_10,local_8,&local_1c,&local_18);
    *(undefined4 *)(*(int *)(param_1 + 0x1b0) + 0x90) = local_18;
    *(undefined4 *)(*(int *)(param_1 + 0x1b0) + 0x8c) = local_1c;
    FUN_009196fd(local_8);
  }
  return 1;
}
```

### decoy: 0x0054f5e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054f5e0: 16-byte aligned: YES
  0054f5e0  55             PUSH EBP
  0054f5e1  8B EC          MOV EBP,ESP
  0054f5e3  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054f5e0
string literals (0):
direct callees (7):
  0x009196FD  operator delete
  0x00919723  operator new
  0x0054F990  (unnamed)
  0x0054FA10  (unnamed)
  0x0054FA90  (unnamed)
  0x0054FB10  CSWSArea::SmoothPointsOnPathRestartable
  0x0054EE60  CSWSArea::SmoothCornerOptimize
struct offsets touched (top 10):
  +0x1B0 [dec 432]  x26  via EAX,ECX,EDX
  +0x90 [dec 144]  x6  via EAX,ECX,EDX
  +0x8C [dec 140]  x4  via EAX,ECX,EDX
  +0x224 [dec 548]  x3  via EAX,ECX,EDX
  +0x214 [dec 532]  x2  via ECX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0x94 [dec 148]  x1  via ECX
  +0x98 [dec 152]  x1  via ECX
  +0x22C [dec 556]  x1  via EAX
  +0x230 [dec 560]  x1  via ECX
```

## Card B0-7-28

GOG function: CSWPartyTable::ClearPuppets  gog 0x007037b0  callconv=__thiscall  param_size_bytes=0  gog_size=63  gog_retn=0
Candidate Steam address: 0x005fde00 (size 63, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005fdc40:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00703770 CSWPartyTable::ClearMembers -> 0x005fddc0  gate_class=REVIEW
- GOG-order neighbour after:  0x007037f0 CSWPartyTable::ClearPuppet -> 0x005fde40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x14c*} | Steam {0x14c*}
imm sets (* = in both, 3 common): GOG {0x3*,0x8*,0x7f000000*} | Steam {0x3*,0x8*,0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005fde00: 16-byte aligned: YES
  005fde00  55             PUSH EBP
  005fde01  8B EC          MOV EBP,ESP
  005fde03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fde00
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fde00 FUN_005fde00 size=63 ret=0 callers=1 callees=0

void __fastcall FUN_005fde00(int param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 3; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1 + 0x14c + local_8 * 4) = 0x7f000000;
  }
  return;
}
```

### decoy: 0x005fdc40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fdc40: 16-byte aligned: YES
  005fdc40  55             PUSH EBP
  005fdc41  8B EC          MOV EBP,ESP
  005fdc43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fdc40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-7-29

GOG function: CSWPartyTable::GivePlotXP_2  gog 0x00703960  callconv=(blank)  param_size_bytes=8  gog_size=268  gog_retn=8
Candidate Steam address: 0x005fdfb0 (size 268, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fdea0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00703850 CSWPartyTable::GivePlotXP -> 0x005fdea0  gate_class=REVIEW
- GOG-order neighbour after:  0x00703a70 CSWPartyTable::DistributeExperience -> 0x005fe0c0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x004df5f0 CGuiInGame::UpdateStatus -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x00703a70 CSWPartyTable::DistributeExperience -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
  0x00929f80 Global::ceilf -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0xe8*,0x130*} | Steam {0x4*,0xe8*,0x130*}
imm sets (* = in both, 3 common): GOG {0x2*,0x8*,0x2c*} | Steam {0x2*,0x8*,0x2c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005fdfb0: 16-byte aligned: YES
  005fdfb0  55             PUSH EBP
  005fdfb1  8B EC          MOV EBP,ESP
  005fdfb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005fdfb0
string literals (0):
direct callees (8):
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x0091EC70  (unnamed)
  0x0091C860  _ftol2_sse
  0x005FE0C0  CSWPartyTable::DistributeExperience
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0xE8 [dec 232]  x1  via EDX
  +0x130 [dec 304]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005fdfb0 FUN_005fdfb0 size=268 ret=8 callers=1 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_005fdfb0(undefined4 param_1,int param_2)

{
  uint uVar1;
  float10 fVar2;
  undefined8 uVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  undefined1 local_24 [8];
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ec08;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = 0;
  local_18 = 0;
  local_1c = *(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0xe8);
  if (local_1c != 0) {
    FUN_00733570();
    local_8 = 0;
    uVar4 = extraout_var;
    FUN_0071bcf0(param_1,local_24,&local_14);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1);
    param_2 = local_14 * param_2;
    fVar2 = (float10)FUN_0091ec70((double)((float)param_2 / (float)_DAT_0098ba38));
    local_18 = FUN_0091c860(uVar1,(float)fVar2,param_2,uVar4);
    if (0 < local_18) {
      FUN_005fe0c0(local_18,1);
      uVar3 = CONCAT44(local_18,2);
      FUN_0073f750(2,local_18);
      FUN_007ceef0(uVar3);
    }
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x005fdea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fdea0: 16-byte aligned: YES
  005fdea0  55             PUSH EBP
  005fdea1  8B EC          MOV EBP,ESP
  005fdea3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005fdea0
string literals (0):
direct callees (8):
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091EC70  (unnamed)
  0x0091C860  _ftol2_sse
  0x005FE0C0  CSWPartyTable::DistributeExperience
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0xE8 [dec 232]  x1  via EDX
  +0x130 [dec 304]  x1  via EAX
```

## Card B0-7-30

GOG function: CSWSArea::PathPointDFSTransTableInitialize  gog 0x0070ceb0  callconv=(blank)  param_size_bytes=16  gog_size=252  gog_retn=16
Candidate Steam address: 0x00552c90 (size 252, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00552b70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0070cd90 CSWSArea::PlotSoundPath -> 0x00552b70  gate_class=REVIEW
- GOG-order neighbour after:  0x0070cfb0 CSWSArea::NoNoneWalkPolysInStaticObject -> 0x00552d90  gate_class=AUTO

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

## Card B0-7-31

GOG function: CSWCArea::Constructor  gog 0x0044ef30  callconv=__thiscall  param_size_bytes=0  gog_size=1741  gog_retn=0
Candidate Steam address: 0x007a2c60 (size 19, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCArea::non_virtual_thunk_to_AsSWCArea
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.01 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2590:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ef10 Global::PlayRoomSoundEvent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044f600 CSWArea::GetIsIDInExcludeList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam ?  in candidate callees: n/a
  0x00457ac0 CExoArrayList_Vector_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x004692d0 CGameObject::Constructor -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00556550 CSWCAmbientSound::Constructor -> Steam ?  in candidate callees: n/a
  0x00559d20 Global::CSWArea -> Steam ?  in candidate callees: n/a
  0x0055a020 Global::InitializeStaticObjects -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  ... 6 more GOG callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xa4,0x104,0x188,0x18c,0x190,0x1a8,0x1b8,0x1bc,0x1c0,0x1c4,0x1e0,0x1e4,...(+23)} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0x90,...(+23)} | Steam {0x104*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007a2c60: 16-byte aligned: YES
  007a2c60  55             PUSH EBP
  007a2c61  8B EC          MOV EBP,ESP
  007a2c63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2c60
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a2c60 FUN_007a2c60 size=19 ret=0 callers=0 callees=0

int __fastcall FUN_007a2c60(int param_1)

{
  return param_1 + -0x104;
}
```

### decoy: 0x007a2590 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2590: 16-byte aligned: YES
  007a2590  55             PUSH EBP
  007a2591  8B EC          MOV EBP,ESP
  007a2593  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007a2590
string literals (0):
direct callees (14):
  0x005E7750  Global::CSWArea
  0x0053D660  (unnamed)
  0x007107F0  (unnamed)
  0x0084A640  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x00722870  (unnamed)
  0x00428F80  (unnamed)
  0x00449230  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x005E7A50  Global::InitializeStaticObjects
  0x00919723  operator new
  0x0088FD20  (unnamed)
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
struct offsets touched (top 10):
  +0x4 [dec 4]  x15  via EAX,ECX,EDX
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0xA4 [dec 164]  x1  via ECX
  +0x104 [dec 260]  x1  via EDX
  +0x188 [dec 392]  x1  via ECX
  +0x18C [dec 396]  x1  via EDX
  +0x1A8 [dec 424]  x1  via ECX
  +0x1B8 [dec 440]  x1  via EDX
  +0x1BC [dec 444]  x1  via EAX
  +0x1C0 [dec 448]  x1  via ECX
```

## Card B0-7-32

GOG function: CSWPartyTable::SaveMemberByObject  gog 0x006fdca0  callconv=(blank)  param_size_bytes=8  gog_size=491  gog_retn=8
Candidate Steam address: 0x005f8130 (size 491, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005f8520:STR0/CALLEE3/SIG0.95/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006fdaa0 CSWPartyTable::SaveMember -> 0x005f7f30  gate_class=AUTO
- GOG-order neighbour after:  0x006fde90 CSWPartyTable::SavePuppet -> 0x005f8320  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00622ff0 CResGFF::Constructor -> Steam 0x00716e60  in candidate callees: yes
  0x00626530 CResGFF::CreateGFFFile -> Steam 0x0071a3d0  in candidate callees: yes
  0x00626700 CResGFF::WriteGFFFile -> Steam 0x0071a5a0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  ... 4 more GOG callees
GOG strings (2): 'GAMEINPROGRESS:'; 'UTC '
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x6*,0x7*,0x8*,0xc*,0x64*,0xa0*,0x7eb*} | Steam {0x2*,0x3*,0x4*,0x6*,0x7*,0x8*,0xc*,0x64*,0xa0*,0x7eb*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005f8130: 16-byte aligned: YES
  005f8130  55             PUSH EBP
  005f8131  8B EC          MOV EBP,ESP
  005f8133  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005f8130
string literals (1):
  0x00993E84  at 005f8254  'GAMEINPROGRESS:'
direct callees (12):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00541080  CSWSObject::ClearAllActions
  0x005F9100  CSWPartyTable::GetFilename
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00574500  CSWSCreature::SaveCreature
  0x00733DF0  CExoString::operator+
  0x00734010  CExoString::CStr
  0x0071A5A0  CResGFF::WriteGFFFile
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005f8130 FUN_005f8130 size=491 ret=8 callers=1 callees=12

void FUN_005f8130(int param_1,undefined4 param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 *local_68;
  undefined1 local_54 [16];
  undefined1 local_44 [8];
  undefined1 local_3c [8];
  int local_34;
  undefined4 *local_30;
  undefined1 local_2c [8];
  undefined1 local_24 [8];
  undefined4 *local_1c;
  undefined1 local_18 [4];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009547cb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = 0;
  if ((-1 < param_1) && (param_1 < 0xc)) {
    local_14 = FUN_0051c100(param_2);
    if (local_14 != 0) {
      FUN_00541080(1);
      FUN_005f9100(local_24,param_1);
      local_8 = 0;
      local_34 = FUN_00919723(0xa0,uVar1);
      local_8._0_1_ = 1;
      if (local_34 == 0) {
        local_68 = (undefined4 *)0x0;
      }
      else {
        local_68 = (undefined4 *)FUN_00716e60();
      }
      local_30 = local_68;
      local_8._0_1_ = 0;
      local_1c = local_68;
      FUN_00733570(&DAT_009938e4);
      local_8._0_1_ = 2;
      FUN_00733570(&DAT_00995250);
      local_8._0_1_ = 3;
      FUN_0071a3d0(local_18,local_44,local_3c);
      local_8._0_1_ = 2;
      FUN_00733780();
      local_8._0_1_ = 0;
      FUN_00733780();
      FUN_00574500(local_1c,local_18);
      FUN_00733570("GAMEINPROGRESS:");
      local_8._0_1_ = 4;
      FUN_00733df0(local_2c,local_24);
      local_8._0_1_ = 6;
      FUN_00733780();
      uVar2 = FUN_00734010();
      FUN_00733570(uVar2);
      local_8._0_1_ = 7;
      FUN_0071a5a0(local_54,0x7eb);
      local_8 = CONCAT31(local_8._1_3_,6);
      FUN_00733780();
      if (local_1c == (undefined4 *)0x0) {
        uVar2 = 0;
      }
      else {
        uVar2 = (**(code **)*local_1c)(1);
      }
      local_1c = (undefined4 *)0x0;
      local_8 = local_8 & 0xffffff00;
      FUN_00733780(uVar1,uVar2);
      local_8 = 0xffffffff;
      FUN_00733780();
    }
  }
  ExceptionList = local_10;
  return;
/* ... 1 more lines truncated by --max-lines */
```

### decoy: 0x005f8520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005f8520: 16-byte aligned: YES
  005f8520  55             PUSH EBP
  005f8521  8B EC          MOV EBP,ESP
  005f8523  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005f8520
string literals (1):
  0x00993E84  at 005f8644  'GAMEINPROGRESS:'
direct callees (12):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00541080  CSWSObject::ClearAllActions
  0x005F9190  CSWPartyTable::GetPupFilename
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00574500  CSWSCreature::SaveCreature
  0x00733DF0  CExoString::operator+
  0x00734010  CExoString::CStr
  0x0071A5A0  CResGFF::WriteGFFFile
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B0-7-33

GOG function: CSWPartyTable::SetNPCObject  gog 0x007005f0  callconv=(blank)  param_size_bytes=8  gog_size=42  gog_retn=8
Candidate Steam address: 0x005faa80 (size 42, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005faa30:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007005a0 CSWPartyTable::GetNumNPCAvailable -> 0x005faa30  gate_class=AUTO
- GOG-order neighbour after:  0x00700620 CSWPartyTable::SetPUPObject -> 0x005faab0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007004d0 CSWPartyTable::GetIsNPCAvailable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x005faa80: 16-byte aligned: YES
  005faa80  55             PUSH EBP
  005faa81  8B EC          MOV EBP,ESP
  005faa83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005faa80
string literals (0):
direct callees (1):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005faa80 FUN_005faa80 size=42 ret=8 callers=3 callees=1

void __thiscall FUN_005faa80(int param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  
  iVar1 = FUN_005fa960(param_2);
  if (iVar1 != 0) {
    *(undefined4 *)(param_1 + 0x1c + param_2 * 4) = param_3;
  }
  return;
}
```

### decoy: 0x005faa30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005faa30: 16-byte aligned: YES
  005faa30  55             PUSH EBP
  005faa31  8B EC          MOV EBP,ESP
  005faa33  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005faa30
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-7-34

GOG function: CSWPartyTable::UpdateMembers  gog 0x006fda30  callconv=(blank)  param_size_bytes=4  gog_size=99  gog_retn=4
Candidate Steam address: 0x005f7ec0 (size 99, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005f7f30:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006fd830 CSWPartyTable::LoadJournal -> 0x005f7cc0  gate_class=REVIEW
- GOG-order neighbour after:  0x006fdaa0 CSWPartyTable::SaveMember -> 0x005f7f30  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006fdaa0 CSWPartyTable::SaveMember -> Steam ?  in candidate callees: n/a
  0x006fde90 CSWPartyTable::SavePuppet -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0x8*,0xc*} | Steam {0x3*,0x4*,0x8*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005f7ec0: 16-byte aligned: YES
  005f7ec0  55             PUSH EBP
  005f7ec1  8B EC          MOV EBP,ESP
  005f7ec3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005f7ec0
string literals (0):
direct callees (2):
  0x005F7F30  CSWPartyTable::SaveMember
  0x005F8320  CSWPartyTable::SavePuppet
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005f7ec0 FUN_005f7ec0 size=99 ret=4 callers=3 callees=2

void FUN_005f7ec0(undefined4 param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 0xc; local_8 = local_8 + 1) {
    FUN_005f7f30(local_8,param_1);
  }
  for (local_8 = 0; local_8 < 3; local_8 = local_8 + 1) {
    FUN_005f8320(local_8,param_1);
  }
  return;
}
```

### decoy: 0x005f7f30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005f7f30: 16-byte aligned: YES
  005f7f30  55             PUSH EBP
  005f7f31  8B EC          MOV EBP,ESP
  005f7f33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005f7f30
string literals (1):
  0x00993E84  at 005f805a  'GAMEINPROGRESS:'
direct callees (12):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00541080  CSWSObject::ClearAllActions
  0x005F9100  CSWPartyTable::GetFilename
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00574500  CSWSCreature::SaveCreature
  0x00733DF0  CExoString::operator+
  0x00734010  CExoString::CStr
  0x0071A5A0  CResGFF::WriteGFFFile
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B0-7-35

GOG function: CSWSArea::Constructor  gog 0x00716560  callconv=__thiscall  param_size_bytes=12  gog_size=1474  gog_retn=12
Candidate Steam address: 0x00521360 (size 1474, retn 12)
Candidate sources: branch+mapper:drift-0x50+name_map:high:batch5_reverse_engineering+near    mapper tag: drift-0x50

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00521110:STR0/CALLEE0/SIG0.052/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00716310 Global::GetObstructionFactor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00716b50 CSWSArea::Destructor -> 0x005219a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (22), mapped to Steam through trusted rows only:
  0x0044dd60 CExoArrayList_CExoString_::CExoArrayList_2 -> Steam ?  in candidate callees: n/a
  0x004692d0 CGameObject::Constructor -> Steam ?  in candidate callees: n/a
  0x00469500 CGameObjectArray::AddObjectAtPos -> Steam ?  in candidate callees: n/a
  0x00469990 CGameObjectArray::AddInternalObject -> Steam ?  in candidate callees: n/a
  0x004951d0 CExoArrayList_CResRef_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00559d20 Global::CSWArea -> Steam ?  in candidate callees: n/a
  0x0055a020 Global::InitializeStaticObjects -> Steam ?  in candidate callees: n/a
  ... 14 more GOG callees
GOG strings (0): 
disp sets (* = in both, 55 common): GOG {0x4*,0x8*,0xc*,0x104*,0x120*,0x124*,0x12c*,0x13c*,0x140*,0x144*,0x164*,0x188*,0x18c*,0x190*,...(+41)} | Steam {0x4*,0x8*,0xc*,0x104*,0x120*,0x124*,0x12c*,0x13c*,0x140*,0x144*,0x164*,0x188*,0x18c*,0x190*,...(+41)}
imm sets (* = in both, 39 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+25)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+25)}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00521360: 16-byte aligned: YES
  00521360  55             PUSH EBP
  00521361  8B EC          MOV EBP,ESP
  00521363  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00521360
string literals (0):
direct callees (22):
  0x005E7750  Global::CSWArea
  0x0052E540  (unnamed)
  0x0053D660  (unnamed)
  0x0084A640  (unnamed)
  0x00722870  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x00735240  (unnamed)
  0x0091D67E  (unnamed)
  0x0052E4A0  (unnamed)
  0x005E91B0  CSWVarTable::Constructor
  0x0040DEF0  (unnamed)
  0x0052E830  (unnamed)
  0x00919723  operator new
  0x0052E640  (unnamed)
  0x006F9200  CPathfindInformation::Constructor
  0x005E7A50  Global::InitializeStaticObjects
  0x0051C080  (unnamed)
  0x0053DD20  (unnamed)
  0x0053D890  (unnamed)
  0x005E8450  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x124 [dec 292]  x2  via ECX
  +0x230 [dec 560]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x104 [dec 260]  x1  via EAX
  +0x120 [dec 288]  x1  via ECX
  +0x12C [dec 300]  x1  via ECX
  +0x13C [dec 316]  x1  via EAX
  +0x140 [dec 320]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00521360 FUN_00521360 size=1474 ret=12 callers=1 callees=22

undefined4 * __thiscall
FUN_00521360(undefined4 *param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 local_50;
  undefined4 *local_4c;
  undefined4 *local_48;
  undefined4 local_44;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094dac8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_005e7750(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_0052e540();
  local_8._0_1_ = 1;
  FUN_0053d660(DAT_00994473,param_4);
  local_8._0_1_ = 2;
  *param_1 = CSWSArea::vftable;
  param_1[0x41 = byte +0x104] = CSWSArea::vftable;
  param_1[0x48 = byte +0x120] = CSWSArea::vftable;
  FUN_0084a640(0);
  local_8._0_1_ = 3;
  FUN_00722870();
  local_8._0_1_ = 4;
  FUN_00733540();
  local_8._0_1_ = 5;
  FUN_00735240();
  local_8._0_1_ = 6;
  FUN_00733540();
  local_8._0_1_ = 7;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x5a,8,4,FUN_00733540,FUN_00733780);
  local_8._0_1_ = 8;
  FUN_0084a640(0);
  local_8._0_1_ = 9;
  FUN_0084a640(0);
  local_8._0_1_ = 10;
  FUN_0052e4a0(0);
  param_1[0x7d = byte +0x1f4] = 0;
  local_8._0_1_ = 0xb;
  FUN_005e91b0();
  FUN_0040def0(0);
  local_8._0_1_ = 0xc;
  FUN_0084a640(0);
  local_8._0_1_ = 0xd;
  FUN_0040def0(0);
  local_8._0_1_ = 0xe;
  FUN_0040def0(0);
  local_8._0_1_ = 0xf;
  FUN_0052e830(0);
  local_8._0_1_ = 0x10;
  uVar1 = FUN_00919723(0xc);
  param_1[0x59 = byte +0x164] = uVar1;
  param_1[1 = byte +0x4] = 0;
  param_1[0x62 = byte +0x188] = 0;
  param_1[3 = byte +0xc] = 0;
  param_1[2 = byte +0x8] = 0;
  *(undefined1 *)((int)param_1 + 0x247) = 0;
  param_1[0x4f = byte +0x13c] = 0;
  param_1[0x4b = byte +0x12c] = 0;
  param_1[99 = byte +0x18c] = 0;
  param_1[100 = byte +0x190] = 0;
  param_1[0x50 = byte +0x140] = 0;
  param_1[0x51 = byte +0x144] = 0;
  param_1[0x6e = byte +0x1b8] = 0;
  param_1[0x9d = byte +0x274] = 0;
  FUN_0052e640(param_2,param_3);
  iVar2 = FUN_00919723(0x278);
  local_8._0_1_ = 0x11;
/* ... 85 more lines truncated by --max-lines */
```

### decoy: 0x00521110 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00521110: 16-byte aligned: YES
  00521110  55             PUSH EBP
  00521111  8B EC          MOV EBP,ESP
  00521113  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00521110
string literals (0):
direct callees (11):
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x0091D220  (unnamed)
  0x00517980  Vec3::Equal
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x00557EF0  (unnamed)
  0x0052BB60  CSWSArea::ClearLineOfSight
  0x0051C0A0  CServerExoApp::GetGameObject
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005143C0  (unnamed)
  0x00436200  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 4):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x18 [dec 24]  x1  via EDX
  +0x48 [dec 72]  x1  via EDX
```

## Card B0-7-36

GOG function: CSWSCreatureStats::CheckInclusionMatch  gog 0x006f8bb0  callconv=(blank)  param_size_bytes=8  gog_size=447  gog_retn=8
Candidate Steam address: 0x006bc730 (size 447, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006bc570:STR0/CALLEE0/SIG0.05/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f89f0 CSWSCreatureStats::GetEffectImmunity -> 0x006bc570  gate_class=AUTO
- GOG-order neighbour after:  0x006f8e20 CSWSCreatureStats::GetEffectLinkImmunity -> 0x006bc9a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 26 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+12)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+12)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006bc730: 16-byte aligned: YES
  006bc730  55             PUSH EBP
  006bc731  8B EC          MOV EBP,ESP
  006bc733  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006bc730
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x6BC940 [dec 7063872]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006bc730 FUN_006bc730 size=447 ret=8 callers=1 callees=0

undefined4 FUN_006bc730(undefined4 param_1,int param_2)

{
  undefined4 local_8;
  
  switch(param_1) {
  case 8:
    local_8 = 0;
    if (param_2 == 8) {
      local_8 = 0xd;
    }
    else if (param_2 == 7) {
      local_8 = 0xf;
    }
    else if (param_2 == 5) {
      local_8 = 0x12;
    }
    else if (param_2 == 4) {
      local_8 = 0x13;
    }
    else if (param_2 == 3) {
      local_8 = 0x15;
    }
    else if (param_2 == 1) {
      local_8 = 0x16;
    }
    else if (param_2 == 6) {
      local_8 = 0x17;
    }
    else if (param_2 == 2) {
      local_8 = 0x18;
    }
    else if (param_2 == 0x13) {
      local_8 = 0x19;
    }
    else if (param_2 == 0x12) {
      local_8 = 0x1a;
    }
    break;
  default:
    local_8 = 0;
    break;
  case 0xb:
    local_8 = 4;
    break;
  case 0xe:
    local_8 = 5;
    break;
  case 0x11:
    local_8 = 6;
    break;
  case 0x12:
    local_8 = 1;
    break;
  case 0x13:
    local_8 = 0x14;
    break;
  case 0x1b:
    local_8 = 9;
    break;
  case 0x1d:
    local_8 = 8;
    break;
  case 0x22:
    local_8 = 10;
    break;
  case 0x23:
    local_8 = 2;
    break;
  case 0x25:
    local_8 = 3;
    break;
  case 0x31:
    local_8 = 7;
    break;
  case 0x38:
    local_8 = 0xb;
    break;
  case 0x5d:
/* ... 19 more lines truncated by --max-lines */
```

### decoy: 0x006bc570 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006bc570: 16-byte aligned: YES
  006bc570  55             PUSH EBP
  006bc571  8B EC          MOV EBP,ESP
  006bc573  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006bc570
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 8):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x122 [dec 290]  x2  via ECX,EDX
  +0x18A [dec 394]  x2  via ECX
  +0x1198 [dec 4504]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x24 [dec 36]  x1  via EDX
  +0xE0 [dec 224]  x1  via EDX
  +0x16A [dec 362]  x1  via EDX
```

## Card B0-7-37

GOG function: CSWPartyTable::AddNPC_2  gog 0x00700170  callconv=(blank)  param_size_bytes=8  gog_size=311  gog_retn=8
Candidate Steam address: 0x005fa600 (size 311, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005fa740:STR0/CALLEE0/SIG0.357/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006fff20 CSWPartyTable::AddPUP -> 0x005fa3b0  gate_class=AUTO
- GOG-order neighbour after:  0x007002b0 CSWPartyTable::AddPUP_2 -> 0x005fa740  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0061a160 CResRef::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x0067ab60 CSWSCreature::Constructor -> Steam ?  in candidate callees: n/a
  0x0068b5a0 CSWSCreature::LoadFromTemplate -> Steam ?  in candidate callees: n/a
  0x006ffb60 CSWPartyTable::AddNPC -> Steam ?  in candidate callees: n/a
  0x007004d0 CSWPartyTable::GetIsNPCAvailable -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x40*,0x1220*,0x7f000000*} | Steam {0x4*,0x8*,0xc*,0x40*,0x1220*,0x7f000000*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005fa600: 16-byte aligned: YES
  005fa600  55             PUSH EBP
  005fa601  8B EC          MOV EBP,ESP
  005fa603  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005fa600
string literals (0):
direct callees (6):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
  0x00919723  operator new
  0x00561F30  CSWSCreature::Constructor
  0x00710810  (unnamed)
  0x00572A70  CSWSCreature::LoadFromTemplate
  0x005F9FF0  CSWPartyTable::AddNPC
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fa600 FUN_005fa600 size=311 ret=8 callers=1 callees=6

undefined4 FUN_005fa600(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 *local_48;
  undefined1 local_30 [16];
  int local_20;
  undefined4 *local_1c;
  undefined4 local_18;
  undefined4 *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009549bb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (((param_1 < 0) || (0xb < param_1)) || (iVar2 = FUN_005fa960(param_1), iVar2 != 0)) {
    local_18 = 0;
  }
  else {
    local_20 = FUN_00919723(0x1220,uVar1);
    local_8 = 0;
    if (local_20 == 0) {
      local_48 = (undefined4 *)0x0;
    }
    else {
      local_48 = (undefined4 *)FUN_00561f30(0x7f000000,0);
    }
    local_1c = local_48;
    local_8 = 0xffffffff;
    local_14 = local_48;
    FUN_00710810(param_2);
    iVar2 = FUN_00572a70(local_30,0);
    if (iVar2 == 0) {
      if (local_14 != (undefined4 *)0x0) {
        (**(code **)*local_14)(1);
      }
      local_18 = 0;
    }
    else {
      local_18 = FUN_005f9ff0(param_1,local_14);
      if (local_14 != (undefined4 *)0x0) {
        (**(code **)*local_14)(1);
      }
    }
  }
  ExceptionList = local_10;
  return local_18;
}
```

### decoy: 0x005fa740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fa740: 16-byte aligned: YES
  005fa740  55             PUSH EBP
  005fa741  8B EC          MOV EBP,ESP
  005fa743  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005fa740
string literals (0):
direct callees (6):
  0x005FA990  CSWPartyTable::GetIsPUPAvailable
  0x00919723  operator new
  0x00561F30  CSWSCreature::Constructor
  0x00710810  (unnamed)
  0x00572A70  CSWSCreature::LoadFromTemplate
  0x005FA3B0  CSWPartyTable::AddPUP
struct offsets touched (top 0):
```

## Card B0-7-38

GOG function: CSWCAreaOfEffectObject::Destructor  gog 0x005821d0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x008ee260 (size 94, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008edb50:STR0/CALLEE0/SIG0.031/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00581f60 CSWCAreaOfEffectObject::Constructor -> 0x008ee580  gate_class=AUTO
- GOG-order neighbour after:  0x00582200 CSWCAreaOfEffectObject::Destructor_2 -> 0x008ee820  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00582200 CSWCAreaOfEffectObject::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x48,0x54,0x68}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008ee260: 16-byte aligned: YES
  008ee260  55             PUSH EBP
  008ee261  8B EC          MOV EBP,ESP
  008ee263  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008ee260
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x48 [dec 72]  x1  via ECX
  +0x54 [dec 84]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008ee260 FUN_008ee260 size=94 ret=4 callers=8 callees=0

undefined4 __thiscall FUN_008ee260(int param_1,undefined2 param_2)

{
  int iVar1;
  undefined4 local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    iVar1 = (**(code **)(**(int **)(param_1 + 0x68) + 0x54))(param_2,0x3f800000,0,0);
    if (iVar1 != 0) {
      *(undefined2 *)(param_1 + 0x48) = param_2;
      local_8 = 1;
    }
  }
  return local_8;
}
```

### decoy: 0x008edb50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008edb50: 16-byte aligned: YES
  008edb50  55             PUSH EBP
  008edb51  8B EC          MOV EBP,ESP
  008edb53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008edb50
string literals (2):
  0x009A9204  at 008edc5f  'SOUND'
  0x009914B8  at 008ee16d  'impact'
direct callees (24):
  0x009196FD  operator delete
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x00733570  CExoString::CExoString_char
  0x0071B1D0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00710810  (unnamed)
  0x0070BA90  (unnamed)
  0x008EE260  (unnamed)
  0x00451E60  (unnamed)
  0x0073F4B0  CClientExoApp::GetGameObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006C1450  (unnamed)
  0x00919723  operator new
  0x008E54E0  (unnamed)
  0x008EDA80  (unnamed)
  0x0073F430  CClientExoApp::GetModule
  0x008E7200  (unnamed)
  0x007ED7D0  CSWCObject::GetCurrentAnimation
struct offsets touched (top 10):
  +0x198 [dec 408]  x21  via EAX,ECX,EDX
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x10  via EAX,ECX,EDX
  +0x20 [dec 32]  x4  via EAX,ECX,EDX
  +0x18 [dec 24]  x3  via ECX,EDX
  +0x2D [dec 45]  x3  via ECX,EDX
  +0x8C [dec 140]  x3  via EAX,EDX
  +0x30 [dec 48]  x2  via ECX,EDX
  +0x98 [dec 152]  x2  via EDX
  +0x104 [dec 260]  x2  via EDX
```

## Card B0-7-39

GOG function: CSWPartyTable::GetFilename  gog 0x006fec70  callconv=(blank)  param_size_bytes=8  gog_size=136  gog_retn=8
Candidate Steam address: 0x005f9100 (size 136, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR  (content level C1)
- evidence summary: STR 'AVAILNPC%d'@00996ebc
- best decoy (neighbour/other candidate) scores: 0x005f9190:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006feb00 CSWPartyTable::AddPuppet -> 0x005f8f90  gate_class=AUTO
- GOG-order neighbour after:  0x006fed00 CSWPartyTable::GetPupFilename -> 0x005f9190  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: yes
GOG strings (1): 'AVAILNPC%d'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0x10*} | Steam {0x8*,0xc*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005f9100: 16-byte aligned: YES
  005f9100  55             PUSH EBP
  005f9101  8B EC          MOV EBP,ESP
  005f9103  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005f9100
string literals (1):
  0x00996EBC  at 005f9142  'AVAILNPC%d'
direct callees (4):
  0x00733540  CExoString::CExoString(empty)
  0x00734270  CExoString::Format
  0x007335F0  CExoString::CExoString_copy
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005f9100 FUN_005f9100 size=136 ret=8 callers=4 callees=4

undefined4 __thiscall FUN_005f9100(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  uint uVar2;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972a61;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  uVar2 = 0;
  FUN_00733540(uVar1,param_1,0);
  local_8 = 1;
  FUN_00734270(local_18,"AVAILNPC%d",param_3);
  FUN_007335f0(local_18);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780(uVar1,param_1,uVar2 | 1);
  ExceptionList = local_10;
  return param_2;
}
```

### decoy: 0x005f9190 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005f9190: 16-byte aligned: YES
  005f9190  55             PUSH EBP
  005f9191  8B EC          MOV EBP,ESP
  005f9193  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005f9190
string literals (1):
  0x00996EC8  at 005f91d2  'AVAILPUP%d'
direct callees (4):
  0x00733540  CExoString::CExoString(empty)
  0x00734270  CExoString::Format
  0x007335F0  CExoString::CExoString_copy
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B0-7-40

GOG function: CSWSArea::LoadItems  gog 0x0071b570  callconv=(blank)  param_size_bytes=16  gog_size=808  gog_retn=16
Candidate Steam address: 0x005263c0 (size 808, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00525f90:STR0/CALLEE2/SIG0.214/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0071b140 CSWSArea::LoadCreatures -> 0x00525f90  gate_class=AUTO
- GOG-order neighbour after:  0x0071b8a0 CSWSArea::LoadDoors -> 0x005266f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (16), mapped to Steam through trusted rows only:
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
  0x006247f0 CResGFF::GetElementType -> Steam 0x00718690  in candidate callees: yes
  0x006248c0 CResGFF::GetList -> Steam 0x00718760  in candidate callees: yes
  0x00624970 CResGFF::GetListCount -> Steam 0x00718810  in candidate callees: yes
  0x006249f0 CResGFF::GetListElement -> Steam 0x00718890  in candidate callees: yes
  0x00624d60 CResGFF::ReadFieldDWORD -> Steam 0x00718c00  in candidate callees: yes
  0x00624e60 CResGFF::ReadFieldFLOAT -> Steam 0x00718d00  in candidate callees: yes
  0x00624fa0 CResGFF::ReadFieldCResRef -> Steam 0x00718e40  in candidate callees: yes
  ... 8 more GOG callees
GOG strings (8): 'TemplateResRef'; 'ObjectId'; 'ZOrientation'; 'YOrientation' ...
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x10*,0x84*,0x30c*,0x7f000000*} | Steam {0x4*,0x10*,0x84*,0x30c*,0x7f000000*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005263c0: 16-byte aligned: YES
  005263c0  55             PUSH EBP
  005263c1  8B EC          MOV EBP,ESP
  005263c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005263c0
string literals (8):
  0x00992EB4  at 00526475  'ObjectId'
  0x0099364C  at 005264ff  'TemplateResRef'
  0x00993628  at 005265f8  'ZPosition'
  0x00993634  at 00526617  'YPosition'
  0x00993640  at 00526636  'XPosition'
  0x00993618  at 00526664  'XOrientation'
  0x00993608  at 00526682  'YOrientation'
  0x009935F8  at 005266a0  'ZOrientation'
direct callees (16):
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00718690  CResGFF::GetElementType
  0x00718C00  CResGFF::ReadFieldDWORD
  0x00919723  operator new
  0x005FFD60  CSWSItem::Constructor
  0x00710840  (unnamed)
  0x00718E40  CResGFF::ReadFieldCResRef
  0x00601610  CSWSItem::LoadFromTemplate
  0x00602BE0  CSWSItem::LoadItem
  0x0053FC70  CSWSObject::LoadObjectState
  0x00718D00  CResGFF::ReadFieldFLOAT
  0x00600EB0  CSWSItem::AddToArea
  0x00543F10  CSWSObject::SetOrientation
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005263c0 FUN_005263c0 size=808 ret=16 callers=1 callees=16

void __thiscall
FUN_005263c0(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4,int param_5)

{
  int iVar1;
  uint uVar2;
  float10 fVar3;
  float fVar4;
  float fVar5;
  undefined4 *local_8c;
  undefined1 local_74 [16];
  int local_64;
  undefined4 *local_60;
  float local_5c;
  float local_58;
  float local_54;
  undefined1 local_50 [16];
  int local_40;
  undefined4 *local_3c;
  uint local_38;
  undefined1 local_34 [4];
  undefined1 local_30 [4];
  undefined1 local_2c [20];
  uint local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094ddfb;
  local_10 = ExceptionList;
  local_18 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar1 = FUN_00718760(local_2c,param_3,&DAT_0099366c);
  if (iVar1 != 0) {
    local_38 = 0;
    while (uVar2 = FUN_00718810(local_2c), local_38 < uVar2) {
      iVar1 = FUN_00718890(local_34,local_2c,local_38);
      if ((iVar1 != 0) && (iVar1 = FUN_00718690(local_34), iVar1 == 0)) {
        local_3c = (undefined4 *)0x0;
        local_14 = FUN_00718c00(local_34,"ObjectId",local_30,0x7f000000);
        local_64 = FUN_00919723(0x30c);
        local_8 = 0;
        if (local_64 == 0) {
          local_8c = (undefined4 *)0x0;
        }
        else {
          local_8c = (undefined4 *)FUN_005ffd60(local_14);
        }
        local_60 = local_8c;
        local_8 = 0xffffffff;
        local_3c = local_8c;
        if (local_8c != (undefined4 *)0x0) {
          if (param_5 == 0) {
            iVar1 = FUN_00602be0(param_2,local_34);
            if (iVar1 == 0) {
              if (local_3c != (undefined4 *)0x0) {
                (**(code **)*local_3c)(1);
              }
              local_3c = (undefined4 *)0x0;
            }
          }
          else {
            FUN_00710840(&DAT_00986925);
            FUN_00718e40(local_50,local_34,"TemplateResRef",&local_40,local_74);
            if ((local_40 == 0) || (iVar1 = FUN_00601610(local_50), iVar1 == 0)) {
              if (local_3c != (undefined4 *)0x0) {
                (**(code **)*local_3c)(1);
              }
              local_3c = (undefined4 *)0x0;
            }
          }
          if (local_3c != (undefined4 *)0x0) {
            local_5c = 0.0;
            local_58 = 0.0;
            local_54 = 0.0;
            if (param_4 != 0) {
              FUN_0053fc70(param_2,local_34);
/* ... 25 more lines truncated by --max-lines */
```

### decoy: 0x00525f90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00525f90: 16-byte aligned: YES
  00525f90  55             PUSH EBP
  00525f91  8B EC          MOV EBP,ESP
  00525f93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00525f90
string literals (9):
  0x0099365C  at 00525fc1  'Creature List'
  0x0099364C  at 005260b6  'TemplateResRef'
  0x00992EB4  at 0052612a  'ObjectId'
  0x00993640  at 0052623b  'XPosition'
  0x00993634  at 00526259  'YPosition'
  0x00993628  at 00526277  'ZPosition'
  0x00993618  at 0052630a  'XOrientation'
  0x00993608  at 0052632b  'YOrientation'
  0x009935F8  at 0052634c  'ZOrientation'
direct callees (16):
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00718690  CResGFF::GetElementType
  0x00919723  operator new
  0x00561F30  CSWSCreature::Constructor
  0x00710840  (unnamed)
  0x00718E40  CResGFF::ReadFieldCResRef
  0x00572A70  CSWSCreature::LoadFromTemplate
  0x00718C00  CResGFF::ReadFieldDWORD
  0x00574180  CSWSCreature::LoadCreature
  0x00718D00  CResGFF::ReadFieldFLOAT
  0x0054BDC0  CSWSArea::ComputeSafeLocation
  0x00568BD0  CSWSCreature::AddToArea
  0x00543F10  CSWSObject::SetOrientation
  0x009196EE  __security_check_cookie
struct offsets touched (top 1):
  +0x380 [dec 896]  x1  via ECX
```

## Card B0-7-41

GOG function: CSWSItem::SetNumActiveProperties  gog 0x006fcf40  callconv=(blank)  param_size_bytes=2  gog_size=85  gog_retn=4
Candidate Steam address: 0x00604140 (size 133, retn 4)
Candidate sources: branch+name_map:high:gog_item_cluster    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00699760:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006fcb30 CSWSCreatureStats::GetItemDetailsForPower -> 0x006c06b0  gate_class=AUTO
- GOG-order neighbour after:  0x006fd080 CSWPartyTable::Constructor -> 0x005f7510  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {0x288,0x290}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*,0x1c}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00604140: 16-byte aligned: YES
  00604140  55             PUSH EBP
  00604141  8B EC          MOV EBP,ESP
  00604143  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00604140
string literals (0):
direct callees (2):
  0x009196FD  operator delete
  0x00919723  operator new
struct offsets touched (top 2):
  +0x290 [dec 656]  x4  via EAX,EDX
  +0x288 [dec 648]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00604140 FUN_00604140 size=133 ret=4 callers=1 callees=2

void __thiscall FUN_00604140(int param_1,int param_2)

{
  longlong lVar1;
  undefined4 uVar2;
  
  *(int *)(param_1 + 0x288) = param_2;
  if (*(int *)(param_1 + 0x290) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x290));
  }
  if (param_2 == 0) {
    *(undefined4 *)(param_1 + 0x290) = 0;
  }
  else {
    lVar1 = (ulonglong)*(uint *)(param_1 + 0x288) * 0x1c;
    uVar2 = FUN_00919723(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
    *(undefined4 *)(param_1 + 0x290) = uVar2;
  }
  return;
}
```

### decoy: 0x00699760 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00699760: 16-byte aligned: YES
  00699760  55             PUSH EBP
  00699761  8B EC          MOV EBP,ESP
  00699763  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00699760
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073F750  ClientApp::GetGuiInGame
  0x007CF470  CGuiInGame::SuppressStatusSummary
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-7-42

GOG function: CSWPartyTable::GetPupFilename  gog 0x006fed00  callconv=(blank)  param_size_bytes=8  gog_size=136  gog_retn=8
Candidate Steam address: 0x005f9190 (size 136, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR  (content level C1)
- evidence summary: STR 'AVAILPUP%d'@00996ec8
- best decoy (neighbour/other candidate) scores: 0x005f9100:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006fec70 CSWPartyTable::GetFilename -> 0x005f9100  gate_class=REVIEW
- GOG-order neighbour after:  0x006fed90 CSWPartyTable::CreateParty -> 0x005f9220  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: yes
GOG strings (1): 'AVAILPUP%d'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0x10*} | Steam {0x8*,0xc*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005f9190: 16-byte aligned: YES
  005f9190  55             PUSH EBP
  005f9191  8B EC          MOV EBP,ESP
  005f9193  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005f9190
string literals (1):
  0x00996EC8  at 005f91d2  'AVAILPUP%d'
direct callees (4):
  0x00733540  CExoString::CExoString(empty)
  0x00734270  CExoString::Format
  0x007335F0  CExoString::CExoString_copy
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005f9190 FUN_005f9190 size=136 ret=8 callers=4 callees=4

undefined4 __thiscall FUN_005f9190(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  uint uVar2;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972a61;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  uVar2 = 0;
  FUN_00733540(uVar1,param_1,0);
  local_8 = 1;
  FUN_00734270(local_18,"AVAILPUP%d",param_3);
  FUN_007335f0(local_18);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780(uVar1,param_1,uVar2 | 1);
  ExceptionList = local_10;
  return param_2;
}
```

### decoy: 0x005f9100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005f9100: 16-byte aligned: YES
  005f9100  55             PUSH EBP
  005f9101  8B EC          MOV EBP,ESP
  005f9103  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005f9100
string literals (1):
  0x00996EBC  at 005f9142  'AVAILNPC%d'
direct callees (4):
  0x00733540  CExoString::CExoString(empty)
  0x00734270  CExoString::Format
  0x007335F0  CExoString::CExoString_copy
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B0-7-43

GOG function: CSWSArea::PlotSoundPath  gog 0x0070cd90  callconv=(blank)  param_size_bytes=16  gog_size=273  gog_retn=16
Candidate Steam address: 0x00552b70 (size 273, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00552c90:STR0/CALLEE0/SIG1/SRV0/VT0
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
prologue for 0x00552b70: 16-byte aligned: YES
  00552b70  55             PUSH EBP
  00552b71  8B EC          MOV EBP,ESP
  00552b73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00552b70
string literals (0):
direct callees (1):
  0x00552B40  CSWSArea::GridDFSTransTableGet
struct offsets touched (top 1):
  +0x1B8 [dec 440]  x8  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00552b70 FUN_00552b70 size=273 ret=16 callers=1 callees=1

undefined4 __thiscall
FUN_00552b70(int param_1,int param_2,int param_3,undefined4 *param_4,undefined4 *param_5)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_00552b40(param_2,param_3);
  if ((*(int *)(iVar1 * 0x10 + *(int *)(param_1 + 0x1b8)) == param_2) &&
     (*(int *)(*(int *)(param_1 + 0x1b8) + 4 + iVar1 * 0x10) == param_3)) {
    *param_4 = *(undefined4 *)(*(int *)(param_1 + 0x1b8) + 8 + iVar1 * 0x10);
    *param_5 = *(undefined4 *)(*(int *)(param_1 + 0x1b8) + 0xc + iVar1 * 0x10);
    uVar2 = 1;
  }
  else {
    iVar1 = iVar1 + 0x100;
    if ((*(int *)(iVar1 * 0x10 + *(int *)(param_1 + 0x1b8)) == param_2) &&
       (*(int *)(*(int *)(param_1 + 0x1b8) + 4 + iVar1 * 0x10) == param_3)) {
      *param_4 = *(undefined4 *)(*(int *)(param_1 + 0x1b8) + 8 + iVar1 * 0x10);
      *param_5 = *(undefined4 *)(*(int *)(param_1 + 0x1b8) + 0xc + iVar1 * 0x10);
      uVar2 = 1;
    }
    else {
      *param_4 = 0;
      *param_5 = 0;
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00552c90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00552c90: 16-byte aligned: YES
  00552c90  55             PUSH EBP
  00552c91  8B EC          MOV EBP,ESP
  00552c93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00552c90
string literals (0):
direct callees (1):
  0x00552B40  CSWSArea::GridDFSTransTableGet
struct offsets touched (top 1):
  +0x1B8 [dec 440]  x9  via EAX,ECX,EDX
```

## Card B0-7-44

GOG function: CSWSArea::SetMaxStealthXP  gog 0x007184d0  callconv=(blank)  param_size_bytes=0  gog_size=52  gog_retn=0
Candidate Steam address: 0x00523320 (size 52, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00523360:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00718460 CSWSArea::GetFirstObjectInArea -> 0x005232b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00718510 CSWSArea::DecrementPlayersInArea -> 0x00523360  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00718550 CSWSArea::IncreaseAILevelPriority -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x12c*} | Steam {0x12c*}
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
prologue for 0x00523320: 16-byte aligned: YES
  00523320  55             PUSH EBP
  00523321  8B EC          MOV EBP,ESP
  00523323  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00523320
string literals (0):
direct callees (1):
  0x005233A0  CSWSArea::IncreaseAILevelPriority
struct offsets touched (top 1):
  +0x12C [dec 300]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00523320 FUN_00523320 size=52 ret=0 callers=2 callees=1

void __fastcall FUN_00523320(int param_1)

{
  if (*(int *)(param_1 + 0x12c [dec 300]) == 0) {
    FUN_005233a0();
  }
  *(int *)(param_1 + 0x12c [dec 300]) = *(int *)(param_1 + 0x12c [dec 300]) + 1;
  return;
}
```

### decoy: 0x00523360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00523360: 16-byte aligned: YES
  00523360  55             PUSH EBP
  00523361  8B EC          MOV EBP,ESP
  00523363  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00523360
string literals (0):
direct callees (1):
  0x005235A0  CSWSArea::DecreaseAILevelPriority
struct offsets touched (top 1):
  +0x12C [dec 300]  x3  via EAX,EDX
```

## Card B0-7-45

GOG function: CSWPartyTable::SetNPCSelectability  gog 0x00700ce0  callconv=(blank)  param_size_bytes=5  gog_size=41  gog_retn=8
Candidate Steam address: 0x005fb170 (size 41, retn 8)
Candidate sources: branch+name_map:high:gog_partytable_block+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fb140:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00700cb0 CSWPartyTable::GetNPCSelectability -> 0x005fb140  gate_class=REVIEW
- GOG-order neighbour after:  0x00700d10 CSWPartyTable::SaveTableInfo -> 0x005fb1a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007004d0 CSWPartyTable::GetIsNPCAvailable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x7c*} | Steam {0x7c*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005fb170: 16-byte aligned: YES
  005fb170  55             PUSH EBP
  005fb171  8B EC          MOV EBP,ESP
  005fb173  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fb170
string literals (0):
direct callees (1):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
struct offsets touched (top 1):
  +0x7C [dec 124]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005fb170 FUN_005fb170 size=41 ret=8 callers=1 callees=1

void __thiscall FUN_005fb170(int param_1,int param_2,undefined1 param_3)

{
  int iVar1;
  
  iVar1 = FUN_005fa960(param_2);
  if (iVar1 != 0) {
    *(undefined1 *)(param_1 + param_2 + 0x7c) = param_3;
  }
  return;
}
```

### decoy: 0x005fb140 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fb140: 16-byte aligned: YES
  005fb140  55             PUSH EBP
  005fb141  8B EC          MOV EBP,ESP
  005fb143  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fb140
string literals (0):
direct callees (1):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
struct offsets touched (top 1):
  +0x7C [dec 124]  x1  via ECX
```

## Card B0-7-46

GOG function: CSWSCreatureStats::AddSpellLikeAbilityToList  gog 0x006fa1e0  callconv=(blank)  param_size_bytes=8  gog_size=205  gog_retn=8
Candidate Steam address: 0x006bdd60 (size 205, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006bdcd0:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006fa150 CSWSCreatureStats::AddClass -> 0x006bdcd0  gate_class=AUTO
- GOG-order neighbour after:  0x006fa4c0 CSWSCreatureStats::GetIsInSpellLikeAbilityList -> 0x006be040  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x8*,0xf*,0x10*,0xf0*,0xf00*,0xf000*} | Steam {0x8*,0xf*,0x10*,0xf0*,0xf00*,0xf000*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006bdd60: 16-byte aligned: YES
  006bdd60  55             PUSH EBP
  006bdd61  8B EC          MOV EBP,ESP
  006bdd63  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006bdd60
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006bdd60 FUN_006bdd60 size=205 ret=8 callers=3 callees=0

undefined4 FUN_006bdd60(uint param_1,uint param_2)

{
  undefined4 local_10;
  
  local_10 = 1;
  if (((param_2 & 0xf000) != 0xf000) && ((param_2 & 0xf000) != (param_1 & 0xf000))) {
    local_10 = 0;
  }
  if (((param_2 & 0xf00) != 0xf00) && ((param_2 & 0xf00) != (param_1 & 0xf00))) {
    local_10 = 0;
  }
  if (((param_2 & 0xf0) != 0xf0) && ((param_2 & 0xf0) != (param_1 & 0xf0))) {
    local_10 = 0;
  }
  if (((param_2 & 0xf) != 0xf) && ((param_2 & 0xf) != (param_1 & 0xf))) {
    local_10 = 0;
  }
  return local_10;
}
```

### decoy: 0x006bdcd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006bdcd0: 16-byte aligned: YES
  006bdcd0  55             PUSH EBP
  006bdcd1  8B EC          MOV EBP,ESP
  006bdcd3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006bdcd0
string literals (0):
direct callees (3):
  0x006B69E0  CSWSCreatureStats::SetClass
  0x006B6A20  CSWSCreatureStats::SetClassLevel
  0x0058B7F0  (unnamed)
struct offsets touched (top 2):
  +0x8D [dec 141]  x5  via EAX,ECX,EDX
  +0x24 [dec 36]  x2  via ECX,EDX
```

## Card B0-7-47

GOG function: CSWPartyTable::SetLeader  gog 0x00703dc0  callconv=(blank)  param_size_bytes=4  gog_size=92  gog_retn=4
Candidate Steam address: 0x005fe410 (size 92, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fe3b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00703d60 CSWPartyTable::GetIsLeader -> 0x005fe3b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00703e20 CSWPartyTable::SetPartyFollowState -> 0x005fe470  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1c*,0x114*} | Steam {0x1c*,0x114*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x7f000000*} | Steam {0x4*,0x8*,0xc*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005fe410: 16-byte aligned: YES
  005fe410  55             PUSH EBP
  005fe411  8B EC          MOV EBP,ESP
  005fe413  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fe410
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x114 [dec 276]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005fe410 FUN_005fe410 size=92 ret=4 callers=2 callees=0

void __thiscall FUN_005fe410(int param_1,int param_2)

{
  int local_8;
  
  *(undefined4 *)(param_1 + 0x114) = 0xffffffff;
  if (param_2 != 0x7f000000) {
    for (local_8 = 0; local_8 < 0xc; local_8 = local_8 + 1) {
      if (param_2 == *(int *)(param_1 + 0x1c + local_8 * 4)) {
        *(int *)(param_1 + 0x114) = local_8;
        return;
      }
    }
  }
  return;
}
```

### decoy: 0x005fe3b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fe3b0: 16-byte aligned: YES
  005fe3b0  55             PUSH EBP
  005fe3b1  8B EC          MOV EBP,ESP
  005fe3b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fe3b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x114 [dec 276]  x1  via ECX
```

## Card B0-7-48

GOG function: CSWPartyTable::ClearInventory  gog 0x006fe490  callconv=(blank)  param_size_bytes=0  gog_size=50  gog_retn=0
Candidate Steam address: 0x005f8920 (size 50, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005f8710:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006fe280 CSWPartyTable::UpdateInventory -> 0x005f8710  gate_class=AUTO
- GOG-order neighbour after:  0x006fe4d0 CSWPartyTable::TransferInventory -> 0x005f8960  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00669a30 CItemRepository::ClearItems -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1a4*,0x1ac*} | Steam {0x1a4*,0x1ac*}
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
prologue for 0x005f8920: 16-byte aligned: YES
  005f8920  55             PUSH EBP
  005f8921  8B EC          MOV EBP,ESP
  005f8923  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005f8920
string literals (0):
direct callees (1):
  0x00606200  CItemRepository::ClearItems
struct offsets touched (top 2):
  +0x1AC [dec 428]  x2  via EAX,ECX
  +0x1A4 [dec 420]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005f8920 FUN_005f8920 size=50 ret=0 callers=2 callees=1

void __fastcall FUN_005f8920(int param_1)

{
  if (*(int *)(param_1 + 0x1ac) != 0) {
    FUN_00606200();
  }
  *(undefined4 *)(param_1 + 0x1a4) = 0;
  return;
}
```

### decoy: 0x005f8710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005f8710: 16-byte aligned: YES
  005f8710  55             PUSH EBP
  005f8711  8B EC          MOV EBP,ESP
  005f8713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005f8710
string literals (2):
  0x00995348  at 005f87e3  'ItemList'
  0x00996E98  at 005f8864  'GAMEINPROGRESS:INVENTORY'
direct callees (13):
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00719740  CResGFF::AddList
  0x00606100  CItemRepository::ItemListGetItem
  0x00719800  CResGFF::AddListElement
  0x00602DD0  CSWSItem::SaveItem
  0x00734010  CExoString::CStr
  0x0071A5A0  CResGFF::WriteGFFFile
  0x00606200  CItemRepository::ClearItems
  0x009196EE  __security_check_cookie
struct offsets touched (top 2):
  +0x1AC [dec 428]  x4  via EAX,EDX
  +0x10 [dec 16]  x1  via ECX
```

## Card B0-7-49

GOG function: CSWPartyTable::GetNPCID  gog 0x007035a0  callconv=(blank)  param_size_bytes=4  gog_size=78  gog_retn=4
Candidate Steam address: 0x005fdbf0 (size 78, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005fddc0:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007034a0 CSWPartyTable::KillPUPObject -> 0x005fdaf0  gate_class=AUTO
- GOG-order neighbour after:  0x007035f0 CSWPartyTable::GetPUPID -> 0x005fdc40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x7f000000*} | Steam {0x4*,0x8*,0xc*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005fdbf0: 16-byte aligned: YES
  005fdbf0  55             PUSH EBP
  005fdbf1  8B EC          MOV EBP,ESP
  005fdbf3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fdbf0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fdbf0 FUN_005fdbf0 size=78 ret=4 callers=3 callees=0

int __thiscall FUN_005fdbf0(int param_1,int param_2)

{
  int local_8;
  
  if (param_2 != 0x7f000000) {
    for (local_8 = 0; local_8 < 0xc; local_8 = local_8 + 1) {
      if (*(int *)(param_1 + 0x1c + local_8 * 4) == param_2) {
        return local_8;
      }
    }
  }
  return -1;
}
```

### decoy: 0x005fddc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fddc0: 16-byte aligned: YES
  005fddc0  55             PUSH EBP
  005fddc1  8B EC          MOV EBP,ESP
  005fddc3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fddc0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-7-50

GOG function: CClientExoApp::DisableVideoEffect  gog 0x0040de80  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740960 (size 26, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::GetAutoPaused
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740930:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040de60 CClientExoApp::EnableVideoEffect -> 0x007409a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dea0 CClientExoApp::GetVideoEffectType  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044af60 CClientExoAppInternal::DisableVideoEffect -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x384}
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
prologue for 0x00740960: 16-byte aligned: YES
  00740960  55             PUSH EBP
  00740961  8B EC          MOV EBP,ESP
  00740963  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740960
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x384 [dec 900]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740960 FUN_00740960 size=26 ret=0 callers=7 callees=0

uint __fastcall FUN_00740960(int param_1)

{
  return *(uint *)(*(int *)(param_1 + 4) + 0x384 [dec 900]) & 1;
}
```

### decoy: 0x00740930 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740930: 16-byte aligned: YES
  00740930  55             PUSH EBP
  00740931  8B EC          MOV EBP,ESP
  00740933  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740930
string literals (0):
direct callees (1):
  0x0079E420  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B0-7-51

GOG function: CSWPartyTable::GetIsLeader  gog 0x00703d60  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=4
Candidate Steam address: 0x005fe3b0 (size 90, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (AddAttackActions 0x568540)
- best decoy (neighbour/other candidate) scores: 0x005fe410:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00703a70 CSWPartyTable::DistributeExperience -> 0x005fe0c0  gate_class=AUTO
- GOG-order neighbour after:  0x00703dc0 CSWPartyTable::SetLeader -> 0x005fe410  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1c*,0x114*} | Steam {0x1c*,0x114*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x7f000000*} | Steam {0x4*,0x8*,0xc*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005fe3b0: 16-byte aligned: YES
  005fe3b0  55             PUSH EBP
  005fe3b1  8B EC          MOV EBP,ESP
  005fe3b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fe3b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x114 [dec 276]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005fe3b0 FUN_005fe3b0 size=90 ret=4 callers=8 callees=0

bool __thiscall FUN_005fe3b0(int param_1,int param_2)

{
  int local_8;
  
  if (param_2 != 0x7f000000) {
    for (local_8 = 0; local_8 < 0xc; local_8 = local_8 + 1) {
      if (param_2 == *(int *)(param_1 + 0x1c + local_8 * 4)) {
        return *(int *)(param_1 + 0x114) == local_8;
      }
    }
  }
  return false;
}
```

### decoy: 0x005fe410 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fe410: 16-byte aligned: YES
  005fe410  55             PUSH EBP
  005fe411  8B EC          MOV EBP,ESP
  005fe413  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fe410
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x114 [dec 276]  x2  via EAX,EDX
```

## Card B0-7-52

GOG function: CExoResMan::GetResRefFromFile  gog 0x0061c540  callconv=(blank)  param_size_bytes=8  gog_size=175  gog_retn=8
Candidate Steam address: 0x00712e50 (size 53, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoResMan::GetTableCount
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00712e90:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061c4c0 CExoResMan::SetResObject -> 0x00712b60  gate_class=AUTO
- GOG-order neighbour after:  0x0061c5f0 CExoResMan::GetResTypeFromFile -> 0x00712c90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006062a0 CExoString::Find_2 -> Steam ?  in candidate callees: n/a
  0x00606b10 CExoString::SubString -> Steam 0x00734a00  in candidate callees: no
  0x0061a710 CResRef::operator=_3 -> Steam ?  in candidate callees: n/a
  0x0061a800 CResRef::operator=_4 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0x10,0x2e,0x38} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00712e50: 16-byte aligned: YES
  00712e50  55             PUSH EBP
  00712e51  8B EC          MOV EBP,ESP
  00712e53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00712e50
string literals (0):
direct callees (2):
  0x00712D70  CExoResMan::GetTable
  0x00726050  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00712e50 FUN_00712e50 size=53 ret=8 callers=1 callees=2

undefined4 FUN_00712e50(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_00712d70(param_1);
  if (iVar1 == 0) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = FUN_00726050(param_2);
  }
  return uVar2;
}
```

### decoy: 0x00712e90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00712e90: 16-byte aligned: YES
  00712e90  55             PUSH EBP
  00712e91  8B EC          MOV EBP,ESP
  00712e93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00712e90
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x712EF0 [dec 7417584]  x1  via EDX
```

## Card B0-7-53

GOG function: CSWPartyTable::ClearMember  gog 0x00703820  callconv=(blank)  param_size_bytes=4  gog_size=39  gog_retn=4
Candidate Steam address: 0x005fde70 (size 39, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005fdbf0:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007037f0 CSWPartyTable::ClearPuppet -> 0x005fde40  gate_class=REVIEW
- GOG-order neighbour after:  0x00703850 CSWPartyTable::GivePlotXP -> 0x005fdea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x7f000000*} | Steam {0x4*,0xc*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005fde70: 16-byte aligned: YES
  005fde70  55             PUSH EBP
  005fde71  8B EC          MOV EBP,ESP
  005fde73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fde70
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fde70 FUN_005fde70 size=39 ret=4 callers=1 callees=0

void __thiscall FUN_005fde70(int param_1,int param_2)

{
  if ((-1 < param_2) && (param_2 < 0xc)) {
    *(undefined4 *)(param_1 + 0x1c + param_2 * 4) = 0x7f000000;
  }
  return;
}
```

### decoy: 0x005fdbf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fdbf0: 16-byte aligned: YES
  005fdbf0  55             PUSH EBP
  005fdbf1  8B EC          MOV EBP,ESP
  005fdbf3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fdbf0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-7-54

GOG function: CSWPartyTable::LoadTableInfo  gog 0x00701d10  callconv=(blank)  param_size_bytes=0  gog_size=4318  gog_retn=0
Candidate Steam address: 0x005fbf60 (size 4318, retn 0)
Candidate sources: branch+mapper:drift-0x240+name_map:high:gog_partytable_block+near    mapper tag: drift-0x240

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 58 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'PT_DISABLEREGEN'@00996f00; 'PT_DISABLEMAP'@00996f10; 'PT_COST_MULT_VALUE'@00996f20; CALLEE 5 (AddFeedbackLine 0x7be090,Constructor_2 0x716fd0,GetStructFromStruct 0x718960); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005fb1a0:STR57/CALLEE0/SIG0.766/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00701ad0 CStatusSummary::SaveToGFF  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00702df0 CSWPartyTable::Save -> 0x005fd440  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (31), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x004ce720 CGuiInGame::AppendToMsgBuffer -> Steam 0x007be090  in candidate callees: yes
  0x004ce840 CGuiInGame::AppendToCombatBuffer -> Steam ?  in candidate callees: n/a
  0x004cea50 CGuiInGame::AppendToDialogBuffer_2 -> Steam ?  in candidate callees: n/a
  0x004dfab0 CGuiInGame::SetStatusSummary -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x005d2e00 CStatusSummary::LoadFromGFF -> Steam ?  in candidate callees: n/a
  ... 23 more GOG callees
GOG strings (58): 'PT_DISABLEREGEN'; 'PT_DISABLEMAP'; 'PT_COST_MULT_VALUE'; 'PT_COST_MULT_LIST' ...
disp sets (* = in both, 35 common): GOG {0x4*,0x8*,0x10*,0x2c*,0x34*,0x4c*,0x7c*,0x88*,0x94*,0xc4*,0xc8*,0x108*,0x10c*,0x110*,...(+21)} | Steam {0x4*,0x8*,0x10*,0x2c*,0x34*,0x4c*,0x7c*,0x88*,0x94*,0xc4*,0xc8*,0x108*,0x10c*,0x110*,...(+21)}
imm sets (* = in both, 21 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x10*,0x17*,0x21*,...(+7)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x10*,0x17*,0x21*,...(+7)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005fbf60: 16-byte aligned: YES
  005fbf60  55             PUSH EBP
  005fbf61  8B EC          MOV EBP,ESP
  005fbf63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005fbf60
string literals (58):
  0x00997268  at 005fbfb4  'PARTYTABLE'
  0x0099719C  at 005fc09e  'PT_MEMBERS'
  0x0099725C  at 005fc0cf  'PT_PCNAME'
  0x00997254  at 005fc12a  'PT_GOLD'
  0x00997240  at 005fc14d  'PT_ITEM_COMPONENT'
  0x0099722C  at 005fc170  'PT_ITEM_CHEMICAL'
  0x00997220  at 005fc193  'PT_SWOOP1'
  0x00997214  at 005fc1b6  'PT_SWOOP2'
  0x00997208  at 005fc1d9  'PT_SWOOP3'
  0x009971FC  at 005fc1fc  'PT_XP_POOL'
  0x009971E8  at 005fc226  'PT_PLAYEDSECONDS'
  0x0099727C  at 005fc24f  'PT_PLAYEDMINUTES'
direct callees (31):
  0x00919723  operator new
  0x00710840  (unnamed)
  0x00716FD0  CResGFF::Constructor_2
  0x008339D0  (unnamed)
  0x005F76A0  CSWPartyTable::ClearTable
  0x007189D0  CResGFF::GetTopLevelStruct
  0x00718760  CResGFF::GetList
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00718C00  CResGFF::ReadFieldDWORD
  0x00718C80  CResGFF::ReadFieldINT
  0x00718A00  CResGFF::ReadFieldBYTE
  0x005FEEF0  CSWPartyTable::SetSoloMode
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00718960  CResGFF::GetStructFromStruct
  0x0091C630  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
struct offsets touched (top 10):
  +0x4 [dec 4]  x13  via EAX,ECX,EDX
  +0x2C [dec 44]  x2  via EAX,ECX
  +0x34 [dec 52]  x2  via ECX,EDX
  +0x1A8 [dec 424]  x2  via ECX
  +0x7C [dec 124]  x1  via EDX
  +0x94 [dec 148]  x1  via EAX
  +0xC4 [dec 196]  x1  via EDX
  +0x108 [dec 264]  x1  via ECX
  +0x10C [dec 268]  x1  via EDX
  +0x110 [dec 272]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005fbf60 FUN_005fbf60 size=4318 ret=0 callers=2 callees=31

void __fastcall FUN_005fbf60(uint *param_1)

{
  undefined1 uVar1;
  uint uVar2;
  undefined4 uVar3;
  uint uVar4;
  int iVar5;
  float10 fVar6;
  undefined1 *puVar7;
  undefined1 *puVar8;
  int local_1a4;
  int local_1a0;
  int local_19c;
  undefined4 *local_18c;
  undefined1 local_120 [8];
  undefined1 local_118 [8];
  undefined1 local_110 [8];
  undefined1 local_108 [8];
  undefined1 local_100 [8];
  undefined1 local_f8 [8];
  undefined1 local_f0 [8];
  undefined1 local_e8 [8];
  undefined1 local_e0 [8];
  undefined4 *local_d8;
  undefined4 *local_d4;
  undefined1 local_d0 [16];
  int local_c0;
  undefined4 *local_bc;
  uint local_b8;
  int local_b4;
  byte local_ad;
  undefined1 local_ac [8];
  undefined1 local_a4 [40];
  int local_7c;
  undefined4 *local_78;
  undefined1 local_74 [4];
  int local_70;
  int local_6c;
  undefined1 local_68 [4];
  undefined4 local_64;
  undefined1 local_60 [8];
  undefined1 local_58 [20];
  uint local_44;
  undefined1 local_40 [8];
  undefined1 local_38 [36];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954b6c;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = uVar2;
  local_c0 = FUN_00919723(0xa0,uVar2);
  local_8 = 0;
  if (local_c0 == 0) {
    local_18c = (undefined4 *)0x0;
  }
  else {
    FUN_00710840("PARTYTABLE");
    local_18c = (undefined4 *)FUN_00716fd0(0,&DAT_00997274,local_d0);
  }
  local_bc = local_18c;
  local_8 = 0xffffffff;
  local_78 = local_18c;
  if ((local_18c == (undefined4 *)0x0) || (local_18c[0x25 = byte +0x94] == 0)) {
    FUN_008339d0(0xbe76);
    if (local_78 != (undefined4 *)0x0) {
      local_d8 = local_78;
      local_d4 = local_78;
      if (local_78 != (undefined4 *)0x0) {
        (**(code **)*local_78)(1);
      }
    }
  }
/* ... 278 more lines truncated by --max-lines */
```

### decoy: 0x005fb1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fb1a0: 16-byte aligned: YES
  005fb1a0  55             PUSH EBP
  005fb1a1  8B EC          MOV EBP,ESP
  005fb1a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005fb1a0
string literals (59):
  0x00997268  at 005fb2b5  'PARTYTABLE'
  0x0099725C  at 005fb39f  'PT_PCNAME'
  0x00997254  at 005fb3bc  'PT_GOLD'
  0x00997240  at 005fb3da  'PT_ITEM_COMPONENT'
  0x0099722C  at 005fb3f8  'PT_ITEM_CHEMICAL'
  0x00997220  at 005fb416  'PT_SWOOP1'
  0x00997214  at 005fb434  'PT_SWOOP2'
  0x00997208  at 005fb452  'PT_SWOOP3'
  0x009971FC  at 005fb470  'PT_XP_POOL'
  0x009971E8  at 005fb48e  'PT_PLAYEDSECONDS'
  0x009971D4  at 005fb4ac  'PT_CONTROLLED_NPC'
  0x009971C8  at 005fb4dc  'PT_SOLOMODE'
direct callees (30):
  0x00733540  CExoString::CExoString(empty)
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x005FEE60  CSWPartyTable::GetTotalPlayedSeconds
  0x005FF000  CSWPartyTable::ResetCurrentSessionStartTime
  0x00719E50  CResGFF::WriteFieldCExoString
  0x00719B90  CResGFF::WriteFieldDWORD
  0x00719BF0  CResGFF::WriteFieldINT
  0x007199D0  CResGFF::WriteFieldBYTE
  0x00719740  CResGFF::AddList
  0x00719800  CResGFF::AddListElement
  0x00719940  CResGFF::AddStructToStruct
  0x0073F750  ClientApp::GetGuiInGame
  0x0071A1B0  CResGFF::WriteFieldVOID
  0x007C7D20  CGuiInGame::GetMessageBuffer
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x34 [dec 52]  x2  via ECX
  +0x114 [dec 276]  x2  via ECX,EDX
  +0x118 [dec 280]  x2  via ECX,EDX
  +0x1A8 [dec 424]  x2  via EDX
  +0x23C [dec 572]  x2  via EAX,EDX
  +0x2C [dec 44]  x1  via EAX
  +0x7C [dec 124]  x1  via EAX
  +0xC4 [dec 196]  x1  via EDX
  +0x108 [dec 264]  x1  via EAX
```

## Card B0-7-55

GOG function: CSWSCreatureStats::SetForceResistancePenalty  gog 0x006f8880  callconv=(blank)  param_size_bytes=1  gog_size=33  gog_retn=4
Candidate Steam address: 0x006bc400 (size 33, retn 4)
Candidate sources: branch+name_map:high:gog_creaturestats_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006bc3d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f8850 CSWSCreatureStats::SetForceResistance -> 0x006bc3d0  gate_class=REVIEW
- GOG-order neighbour after:  0x006f88b0 CSWSCreatureStats::GetFeat_2 -> 0x006bc430  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006e3610 CSWSCreatureStats::UpdateCombatInformation -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x12c*} | Steam {0x12c*}
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
prologue for 0x006bc400: 16-byte aligned: YES
  006bc400  55             PUSH EBP
  006bc401  8B EC          MOV EBP,ESP
  006bc403  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006bc400
string literals (0):
direct callees (1):
  0x006A7190  CSWSCreatureStats::UpdateCombatInformation
struct offsets touched (top 1):
  +0x12C [dec 300]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006bc400 FUN_006bc400 size=33 ret=4 callers=2 callees=1

void __thiscall FUN_006bc400(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x12c [dec 300]) = param_2;
  FUN_006a7190();
  return;
}
```

### decoy: 0x006bc3d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006bc3d0: 16-byte aligned: YES
  006bc3d0  55             PUSH EBP
  006bc3d1  8B EC          MOV EBP,ESP
  006bc3d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006bc3d0
string literals (0):
direct callees (1):
  0x006A7190  CSWSCreatureStats::UpdateCombatInformation
struct offsets touched (top 1):
  +0x12B [dec 299]  x1  via EAX
```

## Card B0-7-56

GOG function: CSWSArea::GetFirstObjectInArea  gog 0x00718460  callconv=(blank)  param_size_bytes=4  gog_size=97  gog_retn=4
Candidate Steam address: 0x005232b0 (size 97, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00523250:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00718400 CSWSArea::GetNextObjectInArea -> 0x00523250  gate_class=REVIEW
- GOG-order neighbour after:  0x007184d0 CSWSArea::SetMaxStealthXP -> 0x00523320  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x194*,0x198*,0x1a0*} | Steam {0x194*,0x198*,0x1a0*}
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
prologue for 0x005232b0: 16-byte aligned: YES
  005232b0  55             PUSH EBP
  005232b1  8B EC          MOV EBP,ESP
  005232b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005232b0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1A0 [dec 416]  x4  via EAX,ECX,EDX
  +0x194 [dec 404]  x1  via EAX
  +0x198 [dec 408]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005232b0 FUN_005232b0 size=97 ret=4 callers=8 callees=0

bool __thiscall FUN_005232b0(int param_1,undefined4 *param_2)

{
  bool bVar1;
  
  *(int *)(param_1 + 0x1a0) = *(int *)(param_1 + 0x1a0) + 1;
  bVar1 = *(int *)(param_1 + 0x1a0) < *(int *)(param_1 + 0x198);
  if (bVar1) {
    *param_2 = *(undefined4 *)(*(int *)(param_1 + 0x194) + *(int *)(param_1 + 0x1a0) * 4);
  }
  return bVar1;
}
```

### decoy: 0x00523250 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00523250: 16-byte aligned: YES
  00523250  55             PUSH EBP
  00523251  8B EC          MOV EBP,ESP
  00523253  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00523250
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1A0 [dec 416]  x2  via EAX
  +0x194 [dec 404]  x1  via EDX
  +0x198 [dec 408]  x1  via ECX
```

## Card B0-7-57

GOG function: CSWPartyTable::GivePlotXP  gog 0x00703850  callconv=(blank)  param_size_bytes=8  gog_size=268  gog_retn=8
Candidate Steam address: 0x005fdea0 (size 268, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fdfb0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00703820 CSWPartyTable::ClearMember -> 0x005fde70  gate_class=REVIEW
- GOG-order neighbour after:  0x00703960 CSWPartyTable::GivePlotXP_2 -> 0x005fdfb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x004df5f0 CGuiInGame::UpdateStatus -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621570 C2DA::GetINTEntry -> Steam ?  in candidate callees: n/a
  0x00703a70 CSWPartyTable::DistributeExperience -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
  0x00929f80 Global::ceilf -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0xe8*,0x130*} | Steam {0x4*,0xe8*,0x130*}
imm sets (* = in both, 3 common): GOG {0x2*,0x8*,0x2c*} | Steam {0x2*,0x8*,0x2c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005fdea0: 16-byte aligned: YES
  005fdea0  55             PUSH EBP
  005fdea1  8B EC          MOV EBP,ESP
  005fdea3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005fdea0
string literals (0):
direct callees (8):
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091EC70  (unnamed)
  0x0091C860  _ftol2_sse
  0x005FE0C0  CSWPartyTable::DistributeExperience
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0xE8 [dec 232]  x1  via EDX
  +0x130 [dec 304]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005fdea0 FUN_005fdea0 size=268 ret=8 callers=1 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_005fdea0(undefined4 param_1,int param_2)

{
  uint uVar1;
  float10 fVar2;
  undefined8 uVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  undefined1 local_24 [8];
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ec08;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = 0;
  local_18 = 0;
  local_1c = *(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0xe8);
  if (local_1c != 0) {
    FUN_00733570();
    local_8 = 0;
    uVar4 = extraout_var;
    FUN_0071bad0(param_1,local_24,&local_14);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1);
    param_2 = local_14 * param_2;
    fVar2 = (float10)FUN_0091ec70((double)((float)param_2 / (float)_DAT_0098ba38));
    local_18 = FUN_0091c860(uVar1,(float)fVar2,param_2,uVar4);
    if (0 < local_18) {
      FUN_005fe0c0(local_18,1);
      uVar3 = CONCAT44(local_18,2);
      FUN_0073f750(2,local_18);
      FUN_007ceef0(uVar3);
    }
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x005fdfb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fdfb0: 16-byte aligned: YES
  005fdfb0  55             PUSH EBP
  005fdfb1  8B EC          MOV EBP,ESP
  005fdfb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005fdfb0
string literals (0):
direct callees (8):
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x0091EC70  (unnamed)
  0x0091C860  _ftol2_sse
  0x005FE0C0  CSWPartyTable::DistributeExperience
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0xE8 [dec 232]  x1  via EDX
  +0x130 [dec 304]  x1  via EAX
```

## Card B0-7-58

GOG function: CSWPartyTable::GetPUPID  gog 0x007035f0  callconv=(blank)  param_size_bytes=4  gog_size=88  gog_retn=4
Candidate Steam address: 0x005fdc40 (size 88, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005fde40:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007035a0 CSWPartyTable::GetNPCID -> 0x005fdbf0  gate_class=REVIEW
- GOG-order neighbour after:  0x00703650 CSWPartyTable::AddGameInProgress -> 0x005fdca0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x14c*} | Steam {0x14c*}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0x8*,0x7f000000*} | Steam {0x3*,0x4*,0x8*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005fdc40: 16-byte aligned: YES
  005fdc40  55             PUSH EBP
  005fdc41  8B EC          MOV EBP,ESP
  005fdc43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fdc40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fdc40 FUN_005fdc40 size=88 ret=4 callers=3 callees=0

int __thiscall FUN_005fdc40(int param_1,int param_2)

{
  int local_8;
  
  if (param_2 != 0x7f000000) {
    for (local_8 = 0; local_8 < 3; local_8 = local_8 + 1) {
      if (*(int *)(param_1 + 0x14c + local_8 * 4) == param_2) {
        return local_8;
      }
    }
  }
  return -1;
}
```

### decoy: 0x005fde40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fde40: 16-byte aligned: YES
  005fde40  55             PUSH EBP
  005fde41  8B EC          MOV EBP,ESP
  005fde43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fde40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-7-59

GOG function: CSWPartyTable::GetPCFirstName  gog 0x007049f0  callconv=(blank)  param_size_bytes=4  gog_size=264  gog_retn=4
Candidate Steam address: 0x005ff040 (size 264, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_partytable_block+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005ff150:STR0/CALLEE1/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007049b0 CSWPartyTable::ResetCurrentSessionStartTime -> 0x005ff000  gate_class=REVIEW
- GOG-order neighbour after:  0x00704b00 CSWPartyTable::GetPCLastName -> 0x005ff150  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006062a0 CExoString::Find_2 -> Steam ?  in candidate callees: n/a
  0x00606b10 CExoString::SubString -> Steam 0x00734a00  in candidate callees: yes
  0x00921b40 Global::strlen -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x4*,0x20*,0x2c*,0x240*} | Steam {0x2*,0x4*,0x20*,0x2c*,0x240*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005ff040: 16-byte aligned: YES
  005ff040  55             PUSH EBP
  005ff041  8B EC          MOV EBP,ESP
  005ff043  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ff040
string literals (0):
direct callees (6):
  0x007335F0  CExoString::CExoString_copy
  0x00919F10  _strlen
  0x00734190  CExoString::Find_2
  0x00734A00  CExoString::SubString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005ff040 FUN_005ff040 size=264 ret=4 callers=1 callees=6

undefined4 __thiscall FUN_005ff040(int param_1,undefined4 param_2)

{
  uint uVar1;
  undefined4 uVar2;
  size_t local_30;
  undefined1 local_24 [8];
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00970f59;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_007335f0(param_1 + 0x240);
  local_8 = 1;
  if (*(int *)(param_1 + 0x240) == 0) {
    local_30 = 0;
  }
  else {
    local_30 = _strlen [GHIDRA-AUTO-LABEL: unreliable](*(char **)(param_1 + 0x240));
  }
  if (0 < (int)local_30) {
    local_1c = FUN_00734190(0x20,0);
    if (local_1c != -1) {
      uVar2 = FUN_00734a00(local_24,0,local_1c);
      local_8._0_1_ = 2;
      FUN_007337c0(uVar2);
      local_8 = CONCAT31(local_8._1_3_,1);
      FUN_00733780();
    }
  }
  FUN_007335f0(local_18);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780(uVar1);
  ExceptionList = local_10;
  return param_2;
}
```

### decoy: 0x005ff150 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ff150: 16-byte aligned: YES
  005ff150  55             PUSH EBP
  005ff151  8B EC          MOV EBP,ESP
  005ff153  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ff150
string literals (0):
direct callees (7):
  0x007335F0  CExoString::CExoString_copy
  0x00919F10  _strlen
  0x00734190  CExoString::Find_2
  0x00734A00  CExoString::SubString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x005355F0  (unnamed)
struct offsets touched (top 0):
```

## Card B0-7-60

GOG function: CSWSItemPropertyHandler::ApplyImprovedForceResistance  gog 0x007c5940  callconv=__thiscall  param_size_bytes=12  gog_size=335  gog_retn=20
Candidate Steam address: 0x005ae820 (size 184, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplyImprovedSavingThrow
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.27 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ae680:STR0/CALLEE0/SIG0.328/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c5670 CSWSItemPropertyHandler::RemoveACBonus -> 0x005ae260  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5a90 CSWSItemPropertyHandler::RemoveImprovedForceResistance -> 0x005ae680  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 2 common): GOG {0x6*,0x8,0xa,0x14,0x130*} | Steam {0x6*,0x130*}
imm sets (* = in both, 1 common): GOG {0x4,0xb,0x14*,0x34,0x8c,0xfffffff8} | Steam {0x14*,0x18}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005ae820: 16-byte aligned: YES
  005ae820  55             PUSH EBP
  005ae821  8B EC          MOV EBP,ESP
  005ae823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ae820
string literals (1):
  0x00992E8C  at 005ae848  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005AEB20  CSWSItemPropertyHandler::ApplyChangedSavingThrow
struct offsets touched (top 2):
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005ae820 FUN_005ae820 size=184 ret=20 callers=0 callees=5

undefined4
FUN_005ae820(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            )

{
  uint uVar1;
  uint uVar2;
  undefined1 *puVar3;
  int *piVar4;
  undefined4 extraout_var;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952ee8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  uVar6 = *(undefined4 *)(DAT_00a1b4d0 + 0x130);
  piVar4 = &local_14;
  puVar3 = local_1c;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  uVar5 = extraout_var;
  FUN_006f0190(1);
  FUN_0071bcf0(uVar2,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,uVar5,uVar6);
  if (local_14 != 0) {
    FUN_005aeb20(param_1,local_14,param_2,param_3,param_5);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005ae680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ae680: 16-byte aligned: YES
  005ae680  55             PUSH EBP
  005ae681  8B EC          MOV EBP,ESP
  005ae683  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ae680
string literals (1):
  0x00992E8C  at 005ae6a8  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 9):
  +0x4 [dec 4]  x2  via ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x14E [dec 334]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```
