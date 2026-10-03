# Review cards B9 chunk 2

Answer each card with one TSV row in `steam_db_batches/B9/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B9-2-01

GOG function: CServerExoAppInternal::AddCharListRequest  gog 0x007bca50  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x00539d30 (size 37, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00539c20:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bca10 CServerExoAppInternal::GetDifficultyOption -> 0x005397c0  gate_class=AUTO
- GOG-order neighbour after:  0x007bcab0 CServerExoAppInternal::UpdateShutdownTimer -> 0x00539860  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007e44a0 CExoArrayList_int_::AddUnique -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x100ec} | Steam {0x4*,0x7f000000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00539d30: 16-byte aligned: YES
  00539d30  55             PUSH EBP
  00539d31  8B EC          MOV EBP,ESP
  00539d33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00539d30
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00539d30 FUN_00539d30 size=37 ret=4 callers=3 callees=0

uint FUN_00539d30(uint param_1)

{
  if (param_1 == 0x7f000000) {
    param_1 = 0x7f000000;
  }
  else {
    param_1 = ~DAT_0099449c & param_1;
  }
  return param_1;
}
```

### decoy: 0x00539c20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00539c20: 16-byte aligned: YES
  00539c20  55             PUSH EBP
  00539c21  8B EC          MOV EBP,ESP
  00539c23  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00539c20
string literals (0):
direct callees (2):
  0x0051A920  (unnamed)
  0x00736200  (unnamed)
struct offsets touched (top 5):
  +0x1F340 [dec 127808]  x5  via EAX,ECX,EDX
  +0x1F33C [dec 127804]  x4  via ECX,EDX
  +0x1F344 [dec 127812]  x4  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x10050 [dec 65616]  x1  via EAX
```

## Card B9-2-02

GOG function: CServerExoApp::IsInjured  gog 0x0064c110  callconv=(blank)  param_size_bytes=4  gog_size=45  gog_retn=4
Candidate Steam address: 0x0051c6f0 (size 45, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c560:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c0f0 CServerExoApp::SkipPastDialog -> 0x0051c6d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c140 CServerExoApp::GetPauseState -> 0x0051c6f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callees: yes
  0x007bbef0 CServerExoAppInternal::IsInjured -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051c6f0: 16-byte aligned: YES
  0051c6f0  55             PUSH EBP
  0051c6f1  8B EC          MOV EBP,ESP
  0051c6f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c6f0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x00538CA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c6f0 FUN_0051c6f0 size=45 ret=4 callers=3 callees=2

void FUN_0051c6f0(undefined4 param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0(param_1);
  FUN_00538ca0(uVar1);
  return;
}
```

### decoy: 0x0051c560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c560: 16-byte aligned: YES
  0051c560  55             PUSH EBP
  0051c561  8B EC          MOV EBP,ESP
  0051c563  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c560
string literals (0):
direct callees (1):
  0x005384B0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-03

GOG function: CServerExoApp::GetFactionOfObject  gog 0x0064c0b0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0051c690 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 6 (callers: OnApplyFactionModifier 0x5951a0, ExecuteCommandAdjustReputation 0x66d1b0, ExecuteCommandChangeFaction/ChangeToStandardFaction 0x66f0a0)
- best decoy (neighbour/other candidate) scores: 0x0051c620:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c090 CServerExoApp::MovePlayerToArea -> 0x0051c670  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c0d0 CServerExoApp::TogglePauseState -> 0x0051c6b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bb570 CServerExoAppInternal::GetFactionOfObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (6 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x00737a90 CSWSEffectListHandler::OnApplyFactionModifier -> Steam 0x005951a0  in candidate callers: yes
  0x00782790 Global::ExecuteCommandAdjustReputation -> Steam 0x0066d1b0  in candidate callers: yes
  0x00784680 Global::ExecuteCommandChangeFaction -> Steam 0x0066f0a0  in candidate callers: yes
  0x00792490 Global::ExecuteCommandGetFactionEqual -> Steam 0x0067ceb0  in candidate callers: yes
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
prologue for 0x0051c690: 16-byte aligned: YES
  0051c690  55             PUSH EBP
  0051c691  8B EC          MOV EBP,ESP
  0051c693  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c690
string literals (0):
direct callees (1):
  0x00538320  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c690 FUN_0051c690 size=32 ret=8 callers=7 callees=1
THUNK -> 0x00538320 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c690(undefined4 param_1,undefined4 param_2)

{
  FUN_00538320(param_1,param_2);
  return;
}
```

### decoy: 0x0051c620 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c620: 16-byte aligned: YES
  0051c620  55             PUSH EBP
  0051c621  8B EC          MOV EBP,ESP
  0051c623  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c620
string literals (0):
direct callees (1):
  0x0052F3D0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-04

GOG function: CServerExoApp::AddToExclusionList  gog 0x0064c1b0  callconv=(blank)  param_size_bytes=5  gog_size=33  gog_retn=8
Candidate Steam address: 0x0051c790 (size 33, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c740:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c180 CServerExoApp::SetPauseState -> 0x0051c760  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c1e0 CServerExoApp::RemoveFromExclusionList -> 0x0051c790  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bc600 CServerExoAppInternal::AddToExclusionList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00747660 CSWSEffectListHandler::OnApplyTimestop -> Steam 0x005a4d70  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0051c790: 16-byte aligned: YES
  0051c790  55             PUSH EBP
  0051c791  8B EC          MOV EBP,ESP
  0051c793  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c790
string literals (0):
direct callees (1):
  0x005393B0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c790 FUN_0051c790 size=33 ret=8 callers=1 callees=1
THUNK -> 0x005393B0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c790(undefined4 param_1,undefined1 param_2)

{
  FUN_005393b0(param_1,param_2);
  return;
}
```

### decoy: 0x0051c740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c740: 16-byte aligned: YES
  0051c740  55             PUSH EBP
  0051c741  8B EC          MOV EBP,ESP
  0051c743  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c740
string literals (0):
direct callees (1):
  0x00538DC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-2-05

GOG function: CServerExoApp::SetActionConversation  gog 0x0064c730  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051cd10 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: EventHandler 0x617b40, EventHandler 0x5f2fe0, ExecuteCommandBeginConversation 0x66df70)
- best decoy (neighbour/other candidate) scores: 0x0051cd60:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c6f0 CServerExoApp::GetLastConversation -> 0x0051ccd0  gate_class=AUTO
- GOG-order neighbour after:  0x0064c760 CServerExoApp::GetActionConversation -> 0x0051cd40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (3 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x00762ff0 CSWSDoor::EventHandler -> Steam 0x00617b40  in candidate callers: yes
  0x0076d3b0 CSWSPlaceable::EventHandler -> Steam 0x005f2fe0  in candidate callers: yes
  0x00783550 Global::ExecuteCommandBeginConversation -> Steam 0x0066df70  in candidate callers: yes
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

## Card B9-2-06

GOG function: CServerExoApp::CreatureAcquireItem  gog 0x0064c680  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0051cc60 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: CreatureAcquireItem 0x73f9e0)
- best decoy (neighbour/other candidate) scores: 0x0051cc80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c620 CServerExoApp::GetRoomIndex -> 0x0051cc00  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c6a0 CServerExoApp::GetFactionManager -> 0x0051cc80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bd1d0 CServerExoAppInternal::CreatureAcquireItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0040cec0 CClientExoApp::CreatureAcquireItem -> Steam 0x0073f9e0  in candidate callers: yes
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
prologue for 0x0051cc60: 16-byte aligned: YES
  0051cc60  55             PUSH EBP
  0051cc61  8B EC          MOV EBP,ESP
  0051cc63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cc60
string literals (0):
direct callees (1):
  0x00539F80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cc60 FUN_0051cc60 size=32 ret=8 callers=1 callees=1
THUNK -> 0x00539F80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051cc60(undefined4 param_1,undefined4 param_2)

{
  FUN_00539f80(param_1,param_2);
  return;
}
```

### decoy: 0x0051cc80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cc80: 16-byte aligned: YES
  0051cc80  55             PUSH EBP
  0051cc81  8B EC          MOV EBP,ESP
  0051cc83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cc80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10054 [dec 65620]  x1  via ECX
```

## Card B9-2-07

GOG function: CServerExoApp::StartServices  gog 0x0064bfe0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c5c0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c600:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bfc0 CServerExoApp::GetConnectionLib -> 0x0051c5a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c000 CServerExoApp::StopServices -> 0x0051c5e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b96e0 CServerExoAppInternal::StartServices -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c5c0: 16-byte aligned: YES
  0051c5c0  55             PUSH EBP
  0051c5c1  8B EC          MOV EBP,ESP
  0051c5c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c5c0
string literals (0):
direct callees (1):
  0x005363A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c5c0 FUN_0051c5c0 size=28 ret=4 callers=1 callees=1
THUNK -> 0x005363A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c5c0(undefined4 param_1)

{
  FUN_005363a0(param_1);
  return;
}
```

### decoy: 0x0051c600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c600: 16-byte aligned: YES
  0051c600  55             PUSH EBP
  0051c601  8B EC          MOV EBP,ESP
  0051c603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c600
string literals (0):
direct callees (1):
  0x0052F330  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-08

GOG function: CServerExoApp::SetDoingStartNewModule  gog 0x0064c8a0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051ce80 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cf50:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c870 CServerExoApp::SetButtonMashCheck -> 0x0051ce50  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c8d0 CServerExoApp::GetNormalOfPoint -> 0x0051ceb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051ce80: 16-byte aligned: YES
  0051ce80  55             PUSH EBP
  0051ce81  8B EC          MOV EBP,ESP
  0051ce83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051ce80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1007C [dec 65660]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051ce80 FUN_0051ce80 size=36 ret=4 callers=1 callees=0

void __thiscall FUN_0051ce80(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x1007c) = param_2;
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

## Card B9-2-09

GOG function: CServerExoApp::GetPlayerListString  gog 0x0064bf80  callconv=(blank)  param_size_bytes=4  gog_size=49  gog_retn=4
Candidate Steam address: 0x0051c560 (size 49, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c3d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bf60 CServerExoApp::RunModule -> 0x0051c540  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bfc0 CServerExoApp::GetConnectionLib -> 0x0051c5a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bb700 CServerExoAppInternal::GetPlayerListString -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051c560: 16-byte aligned: YES
  0051c560  55             PUSH EBP
  0051c561  8B EC          MOV EBP,ESP
  0051c563  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c560
string literals (0):
direct callees (1):
  0x005384B0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c560 FUN_0051c560 size=49 ret=4 callers=1 callees=1

undefined4 FUN_0051c560(undefined4 param_1)

{
  FUN_005384b0(param_1);
  return param_1;
}
```

### decoy: 0x0051c3d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c3d0: 16-byte aligned: YES
  0051c3d0  55             PUSH EBP
  0051c3d1  8B EC          MOV EBP,ESP
  0051c3d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c3d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1006C [dec 65644]  x1  via EDX
```

## Card B9-2-10

GOG function: CServerExoApp::SetMoveToPausedCalendarDay  gog 0x0064c970  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051cf50 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cfa0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c940 CServerExoApp::ShrinkLineByCreatures -> 0x0051cc00  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c9a0 CServerExoApp::GetMoveToPausedCalendarDay -> 0x0051cf80  gate_class=REVIEW

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
prologue for 0x0051cf50: 16-byte aligned: YES
  0051cf50  55             PUSH EBP
  0051cf51  8B EC          MOV EBP,ESP
  0051cf53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cf50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B0 [dec 65712]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cf50 FUN_0051cf50 size=36 ret=4 callers=2 callees=0

void __thiscall FUN_0051cf50(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x100b0) = param_2;
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

## Card B9-2-11

GOG function: CServerExoApp::ClientToServerObjectId  gog 0x0064c2d0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c8b0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 17 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 17 (callers: CreatureAcquireItem 0x73f9e0, MainLoop 0x781be0, GetServerArea 0x7aae00)
- best decoy (neighbour/other candidate) scores: 0x0051c8d0:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c2b0 CServerExoApp::GetGlobalVariableTable -> 0x0051c890  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bcf80 CServerExoAppInternal::ClientToServerObjectId -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (17 of 85): the card's A-CALLEE credit may come from these, not from callees
  0x0040cec0 CClientExoApp::CreatureAcquireItem -> Steam 0x0073f9e0  in candidate callers: yes
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x00457800 CSWCArea::GetServerArea -> Steam 0x007aae00  in candidate callers: yes
  0x00477660 CSWCObject::ClearAllActions -> Steam 0x007f24c0  in candidate callers: yes
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
prologue for 0x0051c8b0: 16-byte aligned: YES
  0051c8b0  55             PUSH EBP
  0051c8b1  8B EC          MOV EBP,ESP
  0051c8b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c8b0
string literals (0):
direct callees (1):
  0x00539D30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c8b0 FUN_0051c8b0 size=28 ret=4 callers=84 callees=1
THUNK -> 0x00539D30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c8b0(undefined4 param_1)

{
  FUN_00539d30(param_1);
  return;
}
```

### decoy: 0x0051c8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c8d0: 16-byte aligned: YES
  0051c8d0  55             PUSH EBP
  0051c8d1  8B EC          MOV EBP,ESP
  0051c8d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c8d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-2-12

GOG function: CServerExoApp::SetLastConversation  gog 0x0064c6c0  callconv=(blank)  param_size_bytes=4  gog_size=42  gog_retn=4
Candidate Steam address: 0x0051cca0 (size 42, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0051cd10:STR0/CALLEE2/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c6a0 CServerExoApp::GetFactionManager -> 0x0051cc80  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c6f0 CServerExoApp::GetLastConversation -> 0x0051ccd0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
GOG callers mapped through trusted rows (2 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00762ff0 CSWSDoor::EventHandler -> Steam 0x00617b40  in candidate callers: yes
  0x0076d3b0 CSWSPlaceable::EventHandler -> Steam 0x005f2fe0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x1f314*} | Steam {0x4*,0x8*,0x1f314*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051cca0: 16-byte aligned: YES
  0051cca0  55             PUSH EBP
  0051cca1  8B EC          MOV EBP,ESP
  0051cca3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cca0
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cca0 FUN_0051cca0 size=42 ret=4 callers=3 callees=1
THUNK -> 0x007337C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051cca0(undefined4 param_1)

{
  FUN_007337c0(param_1);
  return;
}
```

### decoy: 0x0051cd10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cd10: 16-byte aligned: YES
  0051cd10  55             PUSH EBP
  0051cd11  8B EC          MOV EBP,ESP
  0051cd13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cd10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1F31C [dec 127772]  x1  via EDX
```

## Card B9-2-13

GOG function: CServerExoApp::GetLoadFromSaveGame  gog 0x0064c800  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051cde0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 4 (callers: CreateParty 0x5f9220, EventHandler 0x5221b0, AddObjectToArea 0x522bf0)
- best decoy (neighbour/other candidate) scores: 0x0051ce00:STR0/CALLEE1/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c7d0 CServerExoApp::ComputeSafeLocation -> 0x0051cdb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c820 CServerExoApp::SetLoadFromSaveGame -> 0x0051ce00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (4 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x006fed90 CSWPartyTable::CreateParty -> Steam 0x005f9220  in candidate callers: yes
  0x00717360 CSWSArea::EventHandler -> Steam 0x005221b0  in candidate callers: yes
  0x00717da0 CSWSArea::AddObjectToArea -> Steam 0x00522bf0  in candidate callers: yes
  0x00798390 Global::ExecuteCommandGetLoadFromSaveGame -> Steam 0x00682dc0  in candidate callers: yes
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
prologue for 0x0051cde0: 16-byte aligned: YES
  0051cde0  55             PUSH EBP
  0051cde1  8B EC          MOV EBP,ESP
  0051cde3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cde0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100C4 [dec 65732]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cde0 FUN_0051cde0 size=31 ret=0 callers=5 callees=0

undefined4 __fastcall FUN_0051cde0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x100c4);
}
```

### decoy: 0x0051ce00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051ce00: 16-byte aligned: YES
  0051ce00  55             PUSH EBP
  0051ce01  8B EC          MOV EBP,ESP
  0051ce03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051ce00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100C4 [dec 65732]  x1  via EDX
```

## Card B9-2-14

GOG function: CServerExoApp::GetActivePauseState  gog 0x0064c160  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c740 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c790:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c140 CServerExoApp::GetPauseState -> 0x0051c6f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c180 CServerExoApp::SetPauseState -> 0x0051c760  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bc010 CServerExoAppInternal::GetActivePauseState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00747660 CSWSEffectListHandler::OnApplyTimestop -> Steam 0x005a4d70  in candidate callers: yes
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

### decoy: 0x0051c790 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c790: 16-byte aligned: YES
  0051c790  55             PUSH EBP
  0051c791  8B EC          MOV EBP,ESP
  0051c793  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c790
string literals (0):
direct callees (1):
  0x005393B0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-15

GOG function: CServerExoApp::SetPauseState  gog 0x0064c180  callconv=(blank)  param_size_bytes=8  gog_size=33  gog_retn=8
Candidate Steam address: 0x0051c760 (size 33, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c600:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c160 CServerExoApp::GetActivePauseState -> 0x0051c740  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c1b0 CServerExoApp::AddToExclusionList -> 0x0051c790  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bc120 CServerExoAppInternal::SetPauseState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x0075e400 CSWSTrigger::EventHandler -> Steam 0x0061f170  in candidate callers: yes
  0x00762ff0 CSWSDoor::EventHandler -> Steam 0x00617b40  in candidate callers: yes
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
prologue for 0x0051c760: 16-byte aligned: YES
  0051c760  55             PUSH EBP
  0051c761  8B EC          MOV EBP,ESP
  0051c763  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c760
string literals (0):
direct callees (1):
  0x00538ED0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c760 FUN_0051c760 size=33 ret=8 callers=5 callees=1
THUNK -> 0x00538ED0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c760(undefined1 param_1,undefined4 param_2)

{
  FUN_00538ed0(param_1,param_2);
  return;
}
```

### decoy: 0x0051c600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c600: 16-byte aligned: YES
  0051c600  55             PUSH EBP
  0051c601  8B EC          MOV EBP,ESP
  0051c603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c600
string literals (0):
direct callees (1):
  0x0052F330  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-16

GOG function: CServerExoApp::SkipPastDialog  gog 0x0064c0f0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c6d0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c6b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c0d0 CServerExoApp::TogglePauseState -> 0x0051c6b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c110 CServerExoApp::IsInjured -> 0x0051c6f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bd010 CServerExoAppInternal::SkipPastDialog -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c6d0: 16-byte aligned: YES
  0051c6d0  55             PUSH EBP
  0051c6d1  8B EC          MOV EBP,ESP
  0051c6d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c6d0
string literals (0):
direct callees (1):
  0x00539DC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c6d0 FUN_0051c6d0 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00539DC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c6d0(undefined4 param_1)

{
  FUN_00539dc0(param_1);
  return;
}
```

### decoy: 0x0051c6b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c6b0: 16-byte aligned: YES
  0051c6b0  55             PUSH EBP
  0051c6b1  8B EC          MOV EBP,ESP
  0051c6b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c6b0
string literals (0):
direct callees (1):
  0x00538C20  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-17

GOG function: CServerExoApp::TestSafeLocation  gog 0x0064c7b0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0051cd90 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: OnApplyForceJumpDelayed 0x5a7820)
- best decoy (neighbour/other candidate) scores: 0x0051ceb0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c780 CServerExoApp::SetIgnoreConversationStartRange -> 0x0051cd60  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c7d0 CServerExoApp::ComputeSafeLocation -> 0x0051cdb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bda30 CServerExoAppInternal::TestSafeLocation -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0074a0d0 CSWSEffectListHandler::OnApplyForceJumpDelayed -> Steam 0x005a7820  in candidate callers: yes
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

### decoy: 0x0051ceb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051ceb0: 16-byte aligned: YES
  0051ceb0  55             PUSH EBP
  0051ceb1  8B EC          MOV EBP,ESP
  0051ceb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051ceb0
string literals (0):
direct callees (1):
  0x0053AAE0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-18

GOG function: CServerExoApp::ComputeSafeLocation  gog 0x0064c7d0  callconv=(blank)  param_size_bytes=20  gog_size=47  gog_retn=20
Candidate Steam address: 0x0051cdb0 (size 47, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: PositionParty 0x5f9a20)
- best decoy (neighbour/other candidate) scores: 0x0051cd90:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c7b0 CServerExoApp::TestSafeLocation -> 0x0051cd90  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c800 CServerExoApp::GetLoadFromSaveGame -> 0x0051cde0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bda90 CServerExoAppInternal::ComputeSafeLocation -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x006ff590 CSWPartyTable::PositionParty -> Steam 0x005f9a20  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x0051cdb0: 16-byte aligned: YES
  0051cdb0  55             PUSH EBP
  0051cdb1  8B EC          MOV EBP,ESP
  0051cdb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cdb0
string literals (0):
direct callees (1):
  0x0053A840  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cdb0 FUN_0051cdb0 size=47 ret=20 callers=5 callees=1
THUNK -> 0x0053A840 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051cdb0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5)

{
  FUN_0053a840(param_1,param_2,param_3,param_4,param_5);
  return;
}
```

### decoy: 0x0051cd90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cd90: 16-byte aligned: YES
  0051cd90  55             PUSH EBP
  0051cd91  8B EC          MOV EBP,ESP
  0051cd93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cd90
string literals (0):
direct callees (1):
  0x0053A7E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-19

GOG function: CServerExoAppInternal::AddCharListRequest  gog 0x007bca50  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x005397c0 (size 52, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00539800:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bca10 CServerExoAppInternal::GetDifficultyOption -> 0x005397c0  gate_class=AUTO
- GOG-order neighbour after:  0x007bcab0 CServerExoAppInternal::UpdateShutdownTimer -> 0x00539860  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007e44a0 CExoArrayList_int_::AddUnique -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x110}
imm sets (* = in both, 1 common): GOG {0x4*,0x100ec} | Steam {0x4*,0x8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005397c0: 16-byte aligned: YES
  005397c0  55             PUSH EBP
  005397c1  8B EC          MOV EBP,ESP
  005397c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005397c0
string literals (0):
direct callees (1):
  0x006A2140  (unnamed)
struct offsets touched (top 2):
  +0x110 [dec 272]  x1  via ECX
  +0x10004 [dec 65540]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005397c0 FUN_005397c0 size=52 ret=4 callers=1 callees=1
THUNK -> 0x006A2140 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __thiscall FUN_005397c0(int param_1,undefined4 param_2)

{
  FUN_006a2140(param_2,*(undefined4 *)(*(int *)(param_1 + 0x10004) + 0x110));
  return;
}
```

### decoy: 0x00539800 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00539800: 16-byte aligned: YES
  00539800  55             PUSH EBP
  00539801  8B EC          MOV EBP,ESP
  00539803  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00539800
string literals (0):
direct callees (1):
  0x005210A0  (unnamed)
struct offsets touched (top 0):
```

## Card B9-2-20

GOG function: CServerExoApp::GetClientObjectByObjectId  gog 0x0064c020  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c600 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CServerExoApp::GetPlayerByControlledObjectId | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 33 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 33 (callers: UpdateDialog 0x7cd0f0, GetTargetActions 0x842d40, UpdateUsedActiveProperties 0x603860)
- best decoy (neighbour/other candidate) scores: 0x0051c2e0:STR0/CALLEE15/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c000 CServerExoApp::StopServices -> 0x0051c5e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c040 CServerExoApp::GetClientObjectByPlayerId -> 0x0051c620  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b26b0 CServerExoAppInternal::GetClientObjectByObjectId -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (33 of 108): the card's A-CALLEE credit may come from these, not from callees
  0x004dd7f0 CGuiInGame::UpdateDialog -> Steam 0x007cd0f0  in candidate callers: yes
  0x005365c0 CSWCPlaceable::GetTargetActions -> Steam 0x00842d40  in candidate callers: yes
  0x00667090 CSWSItem::UpdateUsedActiveProperties -> Steam 0x00603860  in candidate callers: yes
  0x006688c0 CItemRepository::AddItem -> Steam 0x00605090  in candidate callers: yes
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
prologue for 0x0051c600: 16-byte aligned: YES
  0051c600  55             PUSH EBP
  0051c601  8B EC          MOV EBP,ESP
  0051c603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c600
string literals (0):
direct callees (1):
  0x0052F330  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c600 FUN_0051c600 size=28 ret=4 callers=108 callees=1
THUNK -> 0x0052F330 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c600(undefined4 param_1)

{
  FUN_0052f330(param_1);
  return;
}
```

### decoy: 0x0051c2e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c2e0: 16-byte aligned: YES
  0051c2e0  55             PUSH EBP
  0051c2e1  8B EC          MOV EBP,ESP
  0051c2e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c2e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10010 [dec 65552]  x1  via ECX
```

## Card B9-2-21

GOG function: CServerExoApp::GetDifficultyOption  gog 0x0064c250  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c830 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: GetUnarmedDamageRoll 0x6a9100, GetDamageRoll 0x6adec0)
- best decoy (neighbour/other candidate) scores: 0x0051c810:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c230 CServerExoApp::ResolvePlayerByFirstName -> 0x0051c810  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c270 CServerExoApp::AddCharListRequest -> 0x0051c850  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bca10 CServerExoAppInternal::GetDifficultyOption -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x006e5580 CSWSCreatureStats::GetUnarmedDamageRoll -> Steam 0x006a9100  in candidate callers: yes
  0x006ea340 CSWSCreatureStats::GetDamageRoll -> Steam 0x006adec0  in candidate callers: yes
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
prologue for 0x0051c830: 16-byte aligned: YES
  0051c830  55             PUSH EBP
  0051c831  8B EC          MOV EBP,ESP
  0051c833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c830
string literals (0):
direct callees (1):
  0x005397C0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c830 FUN_0051c830 size=28 ret=4 callers=4 callees=1
THUNK -> 0x005397C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c830(undefined4 param_1)

{
  FUN_005397c0(param_1);
  return;
}
```

### decoy: 0x0051c810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c810: 16-byte aligned: YES
  0051c810  55             PUSH EBP
  0051c811  8B EC          MOV EBP,ESP
  0051c813  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c810
string literals (0):
direct callees (1):
  0x00539650  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-22

GOG function: CServerExoApp::GetFirstPCObject  gog 0x0064c290  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c870 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c8d0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c270 CServerExoApp::AddCharListRequest -> 0x0051c850  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c2b0 CServerExoApp::GetGlobalVariableTable -> 0x0051c890  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bce00 CServerExoAppInternal::GetFirstPCObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x0079cbb0 Global::ExecuteCommandGetPC -> Steam 0x006875e0  in candidate callers: yes
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

### decoy: 0x0051c8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c8d0: 16-byte aligned: YES
  0051c8d0  55             PUSH EBP
  0051c8d1  8B EC          MOV EBP,ESP
  0051c8d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c8d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-2-23

GOG function: CServerExoApp::RemoveFromExclusionList  gog 0x0064c1e0  callconv=(blank)  param_size_bytes=5  gog_size=33  gog_retn=8
Candidate Steam address: 0x0051c790 (size 33, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c7c0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c1b0 CServerExoApp::AddToExclusionList -> 0x0051c790  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c210 CServerExoApp::GetActiveExclusionList -> 0x0051c7f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bc710 CServerExoAppInternal::RemoveFromExclusionList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00747810 CSWSEffectListHandler::OnRemoveTimestop -> Steam 0x005a4f20  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0051c790: 16-byte aligned: YES
  0051c790  55             PUSH EBP
  0051c791  8B EC          MOV EBP,ESP
  0051c793  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c790
string literals (0):
direct callees (1):
  0x005393B0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c790 FUN_0051c790 size=33 ret=8 callers=1 callees=1
THUNK -> 0x005393B0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c790(undefined4 param_1,undefined1 param_2)

{
  FUN_005393b0(param_1,param_2);
  return;
}
```

### decoy: 0x0051c7c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c7c0: 16-byte aligned: YES
  0051c7c0  55             PUSH EBP
  0051c7c1  8B EC          MOV EBP,ESP
  0051c7c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c7c0
string literals (0):
direct callees (1):
  0x005394C0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-24

GOG function: CServerExoApp::GetConnectionLib  gog 0x0064bfc0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c5a0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c560:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bf80 CServerExoApp::GetPlayerListString -> 0x0051c560  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bfe0 CServerExoApp::StartServices -> 0x0051c5c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051c5a0: 16-byte aligned: YES
  0051c5a0  55             PUSH EBP
  0051c5a1  8B EC          MOV EBP,ESP
  0051c5a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c5a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10058 [dec 65624]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c5a0 FUN_0051c5a0 size=31 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_0051c5a0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10058);
}
```

### decoy: 0x0051c560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c560: 16-byte aligned: YES
  0051c560  55             PUSH EBP
  0051c561  8B EC          MOV EBP,ESP
  0051c563  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c560
string literals (0):
direct callees (1):
  0x005384B0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-25

GOG function: CServerExoApp::GetStoreByGameObjectID  gog 0x0064bba0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c160 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=ServerApp::GetPlaceableById | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c140:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> 0x0051c160  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bbc0 CServerExoApp::GetDoorByGameObjectID -> 0x0051c1a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bb220 CServerExoAppInternal::GetStoreByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 10): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051c160: 16-byte aligned: YES
  0051c160  55             PUSH EBP
  0051c161  8B EC          MOV EBP,ESP
  0051c163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c160
string literals (0):
direct callees (1):
  0x00537D30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c160 FUN_0051c160 size=28 ret=4 callers=18 callees=1
THUNK -> 0x00537D30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c160(undefined4 param_1)

{
  FUN_00537d30(param_1);
  return;
}
```

### decoy: 0x0051c140 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c140: 16-byte aligned: YES
  0051c140  55             PUSH EBP
  0051c141  8B EC          MOV EBP,ESP
  0051c143  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c140
string literals (0):
direct callees (1):
  0x00537CC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-26

GOG function: CServerExoAppInternal::IsInjured  gog 0x007bbef0  callconv=(blank)  param_size_bytes=4  gog_size=48  gog_retn=4
Candidate Steam address: 0x00538c20 (size 47, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00538c50:STR0/CALLEE0/SIG0.167/SRV0/VT0
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

### decoy: 0x00538c50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00538c50: 16-byte aligned: YES
  00538c50  55             PUSH EBP
  00538c51  8B EC          MOV EBP,ESP
  00538c53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00538c50
string literals (0):
direct callees (3):
  0x00537B70  (unnamed)
  0x00543F50  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
struct offsets touched (top 0):
```

## Card B9-2-27

GOG function: CServerExoApp::SetIgnoreConversationStartRange  gog 0x0064c780  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051cd60 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cd10:STR0/CALLEE2/SIG1/SRV0/VT0
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
prologue for 0x0051cd60: 16-byte aligned: YES
  0051cd60  55             PUSH EBP
  0051cd61  8B EC          MOV EBP,ESP
  0051cd63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cd60
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1F320 [dec 127776]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cd60 FUN_0051cd60 size=36 ret=4 callers=4 callees=0

void __thiscall FUN_0051cd60(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x1f320) = param_2;
  return;
}
```

### decoy: 0x0051cd10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cd10: 16-byte aligned: YES
  0051cd10  55             PUSH EBP
  0051cd11  8B EC          MOV EBP,ESP
  0051cd13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cd10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1F31C [dec 127772]  x1  via EDX
```

## Card B9-2-28

GOG function: CServerExoAppInternal::StartServices  gog 0x007b96e0  callconv=(blank)  param_size_bytes=0  gog_size=2058  gog_retn=4
Candidate Steam address: 0x00535a10 (size 537, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.27) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.15 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00535c30:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007b9510 CServerExoAppInternal::RunModule -> 0x005361d0  gate_class=REVIEW
- GOG-order neighbour after:  0x007b9ef0 CServerExoAppInternal::StopServices -> 0x00536c10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (30), mapped to Steam through trusted rows only:
  0x00469300 CGameObjectArray::Constructor -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00548240 CServerInfo::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00608350 CExoTimers::GetHighResolutionTimer -> Steam ?  in candidate callees: n/a
  0x0060c4a0 CExoDebug::OpenLogFiles -> Steam ?  in candidate callees: n/a
  ... 22 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (9): 'TEMP:'; 'OVERRIDE:'; 'SERVERVAULT:'; 'GAMEINPROGRESS:' ...
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0x2c,0x30,0x124,0x350,0x380}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+22)} | Steam {0x4*,0x48,0x94,0xa0,0x354,0x1008c,0x1f0b4,0x7f000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
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

## Card B9-2-29

GOG function: CServerExoApp::LoadModule  gog 0x0064bf40  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0051c520 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c4e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bf20 CServerExoApp::UnloadModule -> 0x0051c500  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bf60 CServerExoApp::RunModule -> 0x0051c540  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b4fd0 CServerExoAppInternal::LoadModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051c520: 16-byte aligned: YES
  0051c520  55             PUSH EBP
  0051c521  8B EC          MOV EBP,ESP
  0051c523  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c520
string literals (0):
direct callees (1):
  0x00531C50  CServerExoAppInternal::LoadModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c520 FUN_0051c520 size=32 ret=8 callers=1 callees=1
THUNK -> 0x00531C50 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c520(undefined4 param_1,undefined4 param_2)

{
  FUN_00531c50(param_1,param_2);
  return;
}
```

### decoy: 0x0051c4e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c4e0: 16-byte aligned: YES
  0051c4e0  55             PUSH EBP
  0051c4e1  8B EC          MOV EBP,ESP
  0051c4e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c4e0
string literals (0):
direct callees (1):
  0x00537870  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-30

GOG function: CServerExoApp::GetRoomIndex  gog 0x0064c620  callconv=(blank)  param_size_bytes=0  gog_size=86  gog_retn=12
Candidate Steam address: 0x0051cc00 (size 86, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: GetRoomIndex 0x54b3b0; callers: GetRoomIndex 0x7aace0)
- best decoy (neighbour/other candidate) scores: 0x0051cbe0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0064c600 CServerExoApp::IsActionsPending -> 0x0051cbe0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c680 CServerExoApp::CreatureAcquireItem -> 0x0051cc60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00705750 CSWSArea::GetRoomIndex -> Steam 0x0054b3b0  in candidate callees: yes
  0x0072a6a0 CSWSModule::GetArea -> Steam ?  in candidate callees: n/a
  0x007b2840 CServerExoAppInternal::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004576e0 CSWCArea::GetRoomIndex -> Steam 0x007aace0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 12)
```

### prologue
```
prologue for 0x0051cc00: 16-byte aligned: YES
  0051cc00  55             PUSH EBP
  0051cc01  8B EC          MOV EBP,ESP
  0051cc03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cc00
string literals (0):
direct callees (3):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EF0  (unnamed)
  0x0054B3B0  CSWSArea::GetRoomIndex
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cc00 FUN_0051cc00 size=86 ret=12 callers=1 callees=3

undefined4 __thiscall
FUN_0051cc00(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  iVar1 = FUN_0052f4c0(param_1);
  if (iVar1 != 0) {
    FUN_0052f4c0(param_1);
    iVar1 = FUN_00557ef0();
    if (iVar1 != 0) {
      local_8 = FUN_0054b3b0(param_3,param_4);
    }
  }
  return local_8;
}
```

### decoy: 0x0051cbe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cbe0: 16-byte aligned: YES
  0051cbe0  55             PUSH EBP
  0051cbe1  8B EC          MOV EBP,ESP
  0051cbe3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cbe0
string literals (0):
direct callees (1):
  0x00539F30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-31

GOG function: CServerExoApp::StopServices  gog 0x0064c000  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c5e0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c5c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bfe0 CServerExoApp::StartServices -> 0x0051c5c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> 0x0051c600  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b9ef0 CServerExoAppInternal::StopServices -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051c5e0: 16-byte aligned: YES
  0051c5e0  55             PUSH EBP
  0051c5e1  8B EC          MOV EBP,ESP
  0051c5e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c5e0
string literals (0):
direct callees (1):
  0x00536C10  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c5e0 FUN_0051c5e0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00536C10 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c5e0(undefined4 param_1)

{
  FUN_00536c10(param_1);
  return;
}
```

### decoy: 0x0051c5c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c5c0: 16-byte aligned: YES
  0051c5c0  55             PUSH EBP
  0051c5c1  8B EC          MOV EBP,ESP
  0051c5c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c5c0
string literals (0):
direct callees (1):
  0x005363A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-32

GOG function: CServerExoApp::GetGlobalVariableTable  gog 0x0064c2b0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c890 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 14 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 14 (callers: LoadPCAutoSave 0x8529e0, ModifyAlignment 0x6ad400, SetExperience 0x6b6bd0)
- best decoy (neighbour/other candidate) scores: 0x0051c8d0:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c290 CServerExoApp::GetFirstPCObject -> 0x0051c870  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> 0x0051c8b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (15 of 35): the card's A-CALLEE credit may come from these, not from callees
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: yes
  0x006e1280 CSWSCreatureStats::GetLevel -> Steam 0x006a4e00  in candidate callers: yes
  0x006e9880 CSWSCreatureStats::ModifyAlignment -> Steam 0x006ad400  in candidate callers: yes
  0x006f3050 CSWSCreatureStats::SetExperience -> Steam 0x006b6bd0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x100fc*} | Steam {0x100fc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c890: 16-byte aligned: YES
  0051c890  55             PUSH EBP
  0051c891  8B EC          MOV EBP,ESP
  0051c893  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c890
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c890 FUN_0051c890 size=22 ret=0 callers=35 callees=0

int __fastcall FUN_0051c890(int param_1)

{
  return *(int *)(param_1 + 4) + 0x100fc;
}
```

### decoy: 0x0051c8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c8d0: 16-byte aligned: YES
  0051c8d0  55             PUSH EBP
  0051c8d1  8B EC          MOV EBP,ESP
  0051c8d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c8d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-2-33

GOG function: CServerExoApp::MovePlayerToArea  gog 0x0064c090  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c670 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c650:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c070 CServerExoApp::InitiateModuleForPlayer -> 0x0051c650  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c0b0 CServerExoApp::GetFactionOfObject -> 0x0051c690  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b8d50 CServerExoAppInternal::MovePlayerToArea -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c670: 16-byte aligned: YES
  0051c670  55             PUSH EBP
  0051c671  8B EC          MOV EBP,ESP
  0051c673  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c670
string literals (0):
direct callees (1):
  0x00535A10  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c670 FUN_0051c670 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00535A10 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c670(undefined4 param_1)

{
  FUN_00535a10(param_1);
  return;
}
```

### decoy: 0x0051c650 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c650: 16-byte aligned: YES
  0051c650  55             PUSH EBP
  0051c651  8B EC          MOV EBP,ESP
  0051c653  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c650
string literals (0):
direct callees (1):
  0x005356D0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-34

GOG function: CServerExoApp::GetActionConversation  gog 0x0064c760  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051cd40 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cd10:STR0/CALLEE1/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c730 CServerExoApp::SetActionConversation -> 0x0051cd10  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c780 CServerExoApp::SetIgnoreConversationStartRange -> 0x0051cd60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00783550 Global::ExecuteCommandBeginConversation -> Steam 0x0066df70  in candidate callers: yes
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
prologue for 0x0051cd40: 16-byte aligned: YES
  0051cd40  55             PUSH EBP
  0051cd41  8B EC          MOV EBP,ESP
  0051cd43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cd40
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1F31C [dec 127772]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cd40 FUN_0051cd40 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0051cd40(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x1f31c);
}
```

### decoy: 0x0051cd10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cd10: 16-byte aligned: YES
  0051cd10  55             PUSH EBP
  0051cd11  8B EC          MOV EBP,ESP
  0051cd13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cd10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1F31C [dec 127772]  x1  via EDX
```

## Card B9-2-35

GOG function: CServerExoApp::InitiateModuleForPlayer  gog 0x0064c070  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c650 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c670:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c040 CServerExoApp::GetClientObjectByPlayerId -> 0x0051c620  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c090 CServerExoApp::MovePlayerToArea -> 0x0051c670  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b8a10 CServerExoAppInternal::InitiateModuleForPlayer -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c650: 16-byte aligned: YES
  0051c650  55             PUSH EBP
  0051c651  8B EC          MOV EBP,ESP
  0051c653  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c650
string literals (0):
direct callees (1):
  0x005356D0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c650 FUN_0051c650 size=28 ret=4 callers=1 callees=1
THUNK -> 0x005356D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c650(undefined4 param_1)

{
  FUN_005356d0(param_1);
  return;
}
```

### decoy: 0x0051c670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c670: 16-byte aligned: YES
  0051c670  55             PUSH EBP
  0051c671  8B EC          MOV EBP,ESP
  0051c673  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c670
string literals (0):
direct callees (1):
  0x00535A10  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-36

GOG function: CServerExoApp::GetFactionManager  gog 0x0064c6a0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051cc80 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 5 (callers: AddPUP 0x5fa3b0, OnApplyFactionModifier 0x5951a0, OnRemoveFactionModifier 0x5953e0)
- best decoy (neighbour/other candidate) scores: 0x0051cc60:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c680 CServerExoApp::CreatureAcquireItem -> 0x0051cc60  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c6c0 CServerExoApp::SetLastConversation -> 0x0051cca0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (5 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x006fff20 CSWPartyTable::AddPUP -> Steam 0x005fa3b0  in candidate callers: yes
  0x00737a90 CSWSEffectListHandler::OnApplyFactionModifier -> Steam 0x005951a0  in candidate callers: yes
  0x00737cd0 CSWSEffectListHandler::OnRemoveFactionModifier -> Steam 0x005953e0  in candidate callers: yes
  0x0073a680 CSWSEffectListHandler::OnApplyDeath -> Steam 0x00597d90  in candidate callers: yes
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
prologue for 0x0051cc80: 16-byte aligned: YES
  0051cc80  55             PUSH EBP
  0051cc81  8B EC          MOV EBP,ESP
  0051cc83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cc80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10054 [dec 65620]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cc80 FUN_0051cc80 size=31 ret=0 callers=8 callees=0

undefined4 __fastcall FUN_0051cc80(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10054);
}
```

### decoy: 0x0051cc60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cc60: 16-byte aligned: YES
  0051cc60  55             PUSH EBP
  0051cc61  8B EC          MOV EBP,ESP
  0051cc63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cc60
string literals (0):
direct callees (1):
  0x00539F80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-37

GOG function: CServerExoApp::IsActionsPending  gog 0x0064c600  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051cbe0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cca0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c5c0 CServerExoApp::ClearAllQueuedCombatActions -> 0x0051cba0  gate_class=AUTO
- GOG-order neighbour after:  0x0064c620 CServerExoApp::GetRoomIndex -> 0x0051cc00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bd180 CServerExoAppInternal::IsActionsPending -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051cbe0: 16-byte aligned: YES
  0051cbe0  55             PUSH EBP
  0051cbe1  8B EC          MOV EBP,ESP
  0051cbe3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cbe0
string literals (0):
direct callees (1):
  0x00539F30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cbe0 FUN_0051cbe0 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00539F30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051cbe0(undefined4 param_1)

{
  FUN_00539f30(param_1);
  return;
}
```

### decoy: 0x0051cca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cca0: 16-byte aligned: YES
  0051cca0  55             PUSH EBP
  0051cca1  8B EC          MOV EBP,ESP
  0051cca3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cca0
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-2-38

GOG function: CServerExoApp::ShrinkLineByCreatures  gog 0x0064c940  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0051cc00 (size 86, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cf20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c920 CServerExoApp::ClearAreaCache -> 0x0051cf00  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c970 CServerExoApp::SetMoveToPausedCalendarDay -> 0x0051cf50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007be0f0 CServerExoAppInternal::ShrinkLineByCreatures -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0051cc00: 16-byte aligned: YES
  0051cc00  55             PUSH EBP
  0051cc01  8B EC          MOV EBP,ESP
  0051cc03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cc00
string literals (0):
direct callees (3):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EF0  (unnamed)
  0x0054B3B0  CSWSArea::GetRoomIndex
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cc00 FUN_0051cc00 size=86 ret=12 callers=1 callees=3

undefined4 __thiscall
FUN_0051cc00(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  iVar1 = FUN_0052f4c0(param_1);
  if (iVar1 != 0) {
    FUN_0052f4c0(param_1);
    iVar1 = FUN_00557ef0();
    if (iVar1 != 0) {
      local_8 = FUN_0054b3b0(param_3,param_4);
    }
  }
  return local_8;
}
```

### decoy: 0x0051cf20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cf20: 16-byte aligned: YES
  0051cf20  55             PUSH EBP
  0051cf21  8B EC          MOV EBP,ESP
  0051cf23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cf20
string literals (0):
direct callees (1):
  0x0053AEA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-2-39

GOG function: CServerExoAppInternal::StartServices  gog 0x007b96e0  callconv=(blank)  param_size_bytes=0  gog_size=2058  gog_retn=4
Candidate Steam address: 0x00536bb0 (size 45, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.01 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005363a0:STR5/CALLEE7/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007b9510 CServerExoAppInternal::RunModule -> 0x005361d0  gate_class=REVIEW
- GOG-order neighbour after:  0x007b9ef0 CServerExoAppInternal::StopServices -> 0x00536c10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (30), mapped to Steam through trusted rows only:
  0x00469300 CGameObjectArray::Constructor -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00548240 CServerInfo::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00608350 CExoTimers::GetHighResolutionTimer -> Steam ?  in candidate callees: n/a
  0x0060c4a0 CExoDebug::OpenLogFiles -> Steam ?  in candidate callees: n/a
  ... 22 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (9): 'TEMP:'; 'OVERRIDE:'; 'SERVERVAULT:'; 'GAMEINPROGRESS:' ...
disp sets (* = in both, 0 common): GOG {0x4,0x8} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+22)} | Steam {0x4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x00536bb0: 16-byte aligned: YES
  00536bb0  55             PUSH EBP
  00536bb1  8B EC          MOV EBP,ESP
  00536bb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00536bb0
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00536bb0 FUN_00536bb0 size=45 ret=4 callers=0 callees=1

undefined4 * __thiscall FUN_00536bb0(undefined4 *param_1,uint param_2)

{
  *param_1 = CVirtualMachineCmdImplementer::vftable;
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x005363a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005363a0: 16-byte aligned: YES
  005363a0  55             PUSH EBP
  005363a1  8B EC          MOV EBP,ESP
  005363a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B9-2-40

GOG function: CServerExoApp::GetButtonMashCheck  gog 0x0064c850  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051ce30 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: AddAttackAction 0x590a50, ExecuteCommandGetButtonMashCheck 0x697750)
- best decoy (neighbour/other candidate) scores: 0x0051ce00:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c820 CServerExoApp::SetLoadFromSaveGame -> 0x0051ce00  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c870 CServerExoApp::SetButtonMashCheck -> 0x0051ce50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00775100 CSWSCombatRound::AddAttackAction -> Steam 0x00590a50  in candidate callers: yes
  0x007acda0 Global::ExecuteCommandGetButtonMashCheck -> Steam 0x00697750  in candidate callers: yes
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
prologue for 0x0051ce30: 16-byte aligned: YES
  0051ce30  55             PUSH EBP
  0051ce31  8B EC          MOV EBP,ESP
  0051ce33  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051ce30
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100D8 [dec 65752]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051ce30 FUN_0051ce30 size=31 ret=0 callers=2 callees=0

undefined4 __fastcall FUN_0051ce30(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x100d8);
}
```

### decoy: 0x0051ce00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051ce00: 16-byte aligned: YES
  0051ce00  55             PUSH EBP
  0051ce01  8B EC          MOV EBP,ESP
  0051ce03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051ce00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100C4 [dec 65732]  x1  via EDX
```

## Card B9-2-41

GOG function: CServerExoAppInternal::IsInjured  gog 0x007bbef0  callconv=(blank)  param_size_bytes=4  gog_size=48  gog_retn=4
Candidate Steam address: 0x00538cd0 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00538ca0:STR0/CALLEE0/SIG0.5/SRV0/VT0
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
prologue for 0x00538cd0: 16-byte aligned: YES
  00538cd0  55             PUSH EBP
  00538cd1  8B EC          MOV EBP,ESP
  00538cd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00538cd0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10078 [dec 65656]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00538cd0 FUN_00538cd0 size=36 ret=4 callers=4 callees=0

bool __thiscall FUN_00538cd0(int param_1,byte param_2)

{
  return (*(byte *)(param_1 + 0x10078) & param_2) != 0;
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

## Card B9-2-42

GOG function: CServerExoApp::AddCharListRequest  gog 0x0064c270  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c850 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c830:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c250 CServerExoApp::GetDifficultyOption -> 0x0051c830  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c290 CServerExoApp::GetFirstPCObject -> 0x0051c870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bca50 CServerExoAppInternal::AddCharListRequest -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c850: 16-byte aligned: YES
  0051c850  55             PUSH EBP
  0051c851  8B EC          MOV EBP,ESP
  0051c853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c850
string literals (0):
direct callees (1):
  0x00539800  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c850 FUN_0051c850 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00539800 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c850(undefined4 param_1)

{
  FUN_00539800(param_1);
  return;
}
```

### decoy: 0x0051c830 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c830: 16-byte aligned: YES
  0051c830  55             PUSH EBP
  0051c831  8B EC          MOV EBP,ESP
  0051c833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c830
string literals (0):
direct callees (1):
  0x005397C0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-43

GOG function: CServerExoApp::GetPlayerCreatureId  gog 0x0064c310  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c8f0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 41 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 41 (callers: ShowPartySelection 0x7cb920, UpdateDialog 0x7cd0f0, MenuActionSetPlaceableMine 0x842bd0)
- best decoy (neighbour/other candidate) scores: 0x0051c600:STR0/CALLEE8/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064c330 CServerExoApp::TestDirectLine -> 0x0051c910  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bcfb0 CServerExoAppInternal::GetPlayerCreatureId -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (41 of 117): the card's A-CALLEE credit may come from these, not from callees
  0x004dc020 CGuiInGame::ShowPartySelection -> Steam 0x007cb920  in candidate callers: yes
  0x004dd7f0 CGuiInGame::UpdateDialog -> Steam 0x007cd0f0  in candidate callers: yes
  0x00536450 CSWCPlaceable::MenuActionSetPlaceableMine -> Steam 0x00842bd0  in candidate callers: yes
  0x005365c0 CSWCPlaceable::GetTargetActions -> Steam 0x00842d40  in candidate callers: yes
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
prologue for 0x0051c8f0: 16-byte aligned: YES
  0051c8f0  55             PUSH EBP
  0051c8f1  8B EC          MOV EBP,ESP
  0051c8f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c8f0
string literals (0):
direct callees (1):
  0x00539D60  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c8f0 FUN_0051c8f0 size=22 ret=0 callers=117 callees=1
THUNK -> 0x00539D60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c8f0(undefined4 param_1)

{
  FUN_00539d60(param_1);
  return;
}
```

### decoy: 0x0051c600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c600: 16-byte aligned: YES
  0051c600  55             PUSH EBP
  0051c601  8B EC          MOV EBP,ESP
  0051c603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c600
string literals (0):
direct callees (1):
  0x0052F330  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-44

GOG function: CServerExoApp::UpdateSubareasOnMoveTo  gog 0x0064c500  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0051ca40 (size 81, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cae0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c4c0 CServerExoApp::UpDatePlayerPosition -> 0x0051caa0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c530 CServerExoApp::ClearAllActions -> 0x0051cb10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bd080 CServerExoAppInternal::UpdateSubareasOnMoveTo -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0x8,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0051ca40: 16-byte aligned: YES
  0051ca40  55             PUSH EBP
  0051ca41  8B EC          MOV EBP,ESP
  0051ca43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051ca40
string literals (0):
direct callees (3):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EF0  (unnamed)
  0x0054B130  CSWSArea::ComputeHeight
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051ca40 FUN_0051ca40 size=81 ret=12 callers=8 callees=3

float10 __thiscall
FUN_0051ca40(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  float10 fVar2;
  
  iVar1 = FUN_0052f4c0(param_1);
  if (iVar1 != 0) {
    FUN_0052f4c0(param_1);
    iVar1 = FUN_00557ef0();
    if (iVar1 != 0) {
      fVar2 = (float10)FUN_0054b130(param_3,param_4,0,0);
      return fVar2;
    }
  }
  return (float10)0;
}
```

### decoy: 0x0051cae0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cae0: 16-byte aligned: YES
  0051cae0  55             PUSH EBP
  0051cae1  8B EC          MOV EBP,ESP
  0051cae3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cae0
string literals (0):
direct callees (1):
  0x00539E30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-2-45

GOG function: CServerExoApp::ResolvePlayerByFirstName  gog 0x0064c230  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c810 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c830:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c210 CServerExoApp::GetActiveExclusionList -> 0x0051c7f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c250 CServerExoApp::GetDifficultyOption -> 0x0051c830  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bc8a0 CServerExoAppInternal::ResolvePlayerByFirstName -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c810: 16-byte aligned: YES
  0051c810  55             PUSH EBP
  0051c811  8B EC          MOV EBP,ESP
  0051c813  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c810
string literals (0):
direct callees (1):
  0x00539650  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c810 FUN_0051c810 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00539650 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c810(undefined4 param_1)

{
  FUN_00539650(param_1);
  return;
}
```

### decoy: 0x0051c830 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c830: 16-byte aligned: YES
  0051c830  55             PUSH EBP
  0051c831  8B EC          MOV EBP,ESP
  0051c833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c830
string literals (0):
direct callees (1):
  0x005397C0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-46

GOG function: CServerExoApp::UpDatePlayerPosition  gog 0x0064c4c0  callconv=(blank)  param_size_bytes=12  gog_size=53  gog_retn=12
Candidate Steam address: 0x0051caa0 (size 53, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051ca40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c460 CServerExoApp::ComputeHeight -> 0x0051ca40  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c500 CServerExoApp::UpdateSubareasOnMoveTo -> 0x0051ca40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callees: yes
  0x007bbea0 CServerExoAppInternal::UpDatePlayerPosition -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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

## Card B9-2-47

GOG function: CServerExoApp::GetPauseState  gog 0x0064c140  callconv=(blank)  param_size_bytes=1  gog_size=29  gog_retn=4
Candidate Steam address: 0x0051c6f0 (size 45, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c720:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c110 CServerExoApp::IsInjured -> 0x0051c6f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c160 CServerExoApp::GetActivePauseState -> 0x0051c740  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bbf20 CServerExoAppInternal::GetPauseState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x00653640 CSWSCreature::AIActionPhysicalAttacks -> Steam 0x006d7390  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c6f0: 16-byte aligned: YES
  0051c6f0  55             PUSH EBP
  0051c6f1  8B EC          MOV EBP,ESP
  0051c6f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c6f0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x00538CA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c6f0 FUN_0051c6f0 size=45 ret=4 callers=3 callees=2

void FUN_0051c6f0(undefined4 param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0(param_1);
  FUN_00538ca0(uVar1);
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

## Card B9-2-48

GOG function: CServerExoApp::GetActiveExclusionList  gog 0x0064c210  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c7f0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c7c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
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
prologue for 0x0051c7f0: 16-byte aligned: YES
  0051c7f0  55             PUSH EBP
  0051c7f1  8B EC          MOV EBP,ESP
  0051c7f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c7f0
string literals (0):
direct callees (1):
  0x00539600  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c7f0 FUN_0051c7f0 size=22 ret=0 callers=2 callees=1
THUNK -> 0x00539600 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c7f0(undefined4 param_1)

{
  FUN_00539600(param_1);
  return;
}
```

### decoy: 0x0051c7c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c7c0: 16-byte aligned: YES
  0051c7c0  55             PUSH EBP
  0051c7c1  8B EC          MOV EBP,ESP
  0051c7c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c7c0
string literals (0):
direct callees (1):
  0x005394C0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-49

GOG function: CServerExoApp::SetButtonMashCheck  gog 0x0064c870  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051ce50 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandSetButtonMashCheck 0x6977a0)
- best decoy (neighbour/other candidate) scores: 0x0051ce80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c850 CServerExoApp::GetButtonMashCheck -> 0x0051ce30  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c8a0 CServerExoApp::SetDoingStartNewModule -> 0x0051ce80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007acdf0 Global::ExecuteCommandSetButtonMashCheck -> Steam 0x006977a0  in candidate callers: yes
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
prologue for 0x0051ce50: 16-byte aligned: YES
  0051ce50  55             PUSH EBP
  0051ce51  8B EC          MOV EBP,ESP
  0051ce53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051ce50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100D8 [dec 65752]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051ce50 FUN_0051ce50 size=36 ret=4 callers=1 callees=0

void __thiscall FUN_0051ce50(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x100d8) = param_2;
  return;
}
```

### decoy: 0x0051ce80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051ce80: 16-byte aligned: YES
  0051ce80  55             PUSH EBP
  0051ce81  8B EC          MOV EBP,ESP
  0051ce83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051ce80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1007C [dec 65660]  x1  via EDX
```

## Card B9-2-50

GOG function: CServerExoApp::GetClientObjectByPlayerId  gog 0x0064c040  callconv=(blank)  param_size_bytes=5  gog_size=33  gog_retn=8
Candidate Steam address: 0x0051c620 (size 33, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c2e0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c020 CServerExoApp::GetClientObjectByObjectId -> 0x0051c600  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c070 CServerExoApp::InitiateModuleForPlayer -> 0x0051c650  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2750 CServerExoAppInternal::GetClientObjectByPlayerId -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x006688c0 CItemRepository::AddItem -> Steam 0x00605090  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0051c620: 16-byte aligned: YES
  0051c620  55             PUSH EBP
  0051c621  8B EC          MOV EBP,ESP
  0051c623  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c620
string literals (0):
direct callees (1):
  0x0052F3D0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c620 FUN_0051c620 size=33 ret=8 callers=7 callees=1
THUNK -> 0x0052F3D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c620(undefined4 param_1,undefined1 param_2)

{
  FUN_0052f3d0(param_1,param_2);
  return;
}
```

### decoy: 0x0051c2e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c2e0: 16-byte aligned: YES
  0051c2e0  55             PUSH EBP
  0051c2e1  8B EC          MOV EBP,ESP
  0051c2e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c2e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10010 [dec 65552]  x1  via ECX
```

## Card B9-2-51

GOG function: CServerExoApp::ClearAreaCache  gog 0x0064c920  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051cf00 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: Destructor_2 0x5219d0)
- best decoy (neighbour/other candidate) scores: 0x0051cee0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c900 CServerExoApp::ClearLastModuleFound -> 0x0051cee0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c940 CServerExoApp::ShrinkLineByCreatures -> 0x0051cc00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007baee0 CServerExoAppInternal::ClearAreaCache -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00716b80 CSWSArea::Destructor_2 -> Steam 0x005219d0  in candidate callers: yes
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
prologue for 0x0051cf00: 16-byte aligned: YES
  0051cf00  55             PUSH EBP
  0051cf01  8B EC          MOV EBP,ESP
  0051cf03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cf00
string literals (0):
direct callees (1):
  0x00537C90  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cf00 FUN_0051cf00 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00537C90 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051cf00(undefined4 param_1)

{
  FUN_00537c90(param_1);
  return;
}
```

### decoy: 0x0051cee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cee0: 16-byte aligned: YES
  0051cee0  55             PUSH EBP
  0051cee1  8B EC          MOV EBP,ESP
  0051cee3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cee0
string literals (0):
direct callees (1):
  0x0052F570  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B9-2-52

GOG function: CServerExoAppInternal::GetPauseState  gog 0x007bbf20  callconv=(blank)  param_size_bytes=1  gog_size=36  gog_retn=4
Candidate Steam address: 0x00538ca0 (size 48, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
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
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
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

## Card B9-2-53

GOG function: CServerExoApp::GetStoreByGameObjectID  gog 0x0064bba0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c1a0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c180:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> 0x0051c160  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bbc0 CServerExoApp::GetDoorByGameObjectID -> 0x0051c1a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bb220 CServerExoAppInternal::GetStoreByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 10): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051c1a0: 16-byte aligned: YES
  0051c1a0  55             PUSH EBP
  0051c1a1  8B EC          MOV EBP,ESP
  0051c1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c1a0
string literals (0):
direct callees (1):
  0x00537DA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c1a0 FUN_0051c1a0 size=28 ret=4 callers=9 callees=1
THUNK -> 0x00537DA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c1a0(undefined4 param_1)

{
  FUN_00537da0(param_1);
  return;
}
```

### decoy: 0x0051c180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c180: 16-byte aligned: YES
  0051c180  55             PUSH EBP
  0051c181  8B EC          MOV EBP,ESP
  0051c183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c180
string literals (0):
direct callees (1):
  0x00537FD0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-54

GOG function: CServerExoApp::GetNormalOfPoint  gog 0x0064c8d0  callconv=(blank)  param_size_bytes=8  gog_size=35  gog_retn=8
Candidate Steam address: 0x0051ceb0 (size 35, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051cd90:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c8a0 CServerExoApp::SetDoingStartNewModule -> 0x0051ce80  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c900 CServerExoApp::ClearLastModuleFound -> 0x0051cee0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bdd30 CServerExoAppInternal::GetNormalOfPoint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051ceb0: 16-byte aligned: YES
  0051ceb0  55             PUSH EBP
  0051ceb1  8B EC          MOV EBP,ESP
  0051ceb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051ceb0
string literals (0):
direct callees (1):
  0x0053AAE0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051ceb0 FUN_0051ceb0 size=35 ret=8 callers=1 callees=1

undefined4 FUN_0051ceb0(undefined4 param_1,undefined4 param_2)

{
  FUN_0053aae0(param_1,param_2);
  return param_1;
}
```

### decoy: 0x0051cd90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cd90: 16-byte aligned: YES
  0051cd90  55             PUSH EBP
  0051cd91  8B EC          MOV EBP,ESP
  0051cd93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cd90
string literals (0):
direct callees (1):
  0x0053A7E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-55

GOG function: CServerExoApp::SetLoadFromSaveGame  gog 0x0064c820  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051ce00 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: DoQuickLoad 0x7d0f50, LoadGame 0x853a00, EventHandler 0x5221b0)
- best decoy (neighbour/other candidate) scores: 0x0051cde0:STR0/CALLEE1/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c800 CServerExoApp::GetLoadFromSaveGame -> 0x0051cde0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c850 CServerExoApp::GetButtonMashCheck -> 0x0051ce30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (3 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x004e1650 CGuiInGame::DoQuickLoad -> Steam 0x007d0f50  in candidate callers: yes
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
  0x00717360 CSWSArea::EventHandler -> Steam 0x005221b0  in candidate callers: yes
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

## Card B9-2-56

GOG function: CServerExoAppInternal::GetPauseState  gog 0x007bbf20  callconv=(blank)  param_size_bytes=1  gog_size=36  gog_retn=4
Candidate Steam address: 0x005384b0 (size 1894, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.02 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00538c20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bbef0 CServerExoAppInternal::IsInjured -> 0x00538ca0  gate_class=REVIEW
- GOG-order neighbour after:  0x007bbf50 CServerExoAppInternal::ReprocessExclusionListActions -> 0x00538d00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x10}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+14)}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005384b0: 16-byte aligned: YES
  005384b0  55             PUSH EBP
  005384b1  8B EC          MOV EBP,ESP
  005384b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005384b0
string literals (5):
  0x009941BC  at 00538530  'ServerAdmins: '
  0x009941AC  at 005386f6  ' [ServerAdmin]'
  0x009941A0  at 005387f3  'Players: '
  0x00994194  at 005389ea  ' [Player]'
  0x0099418C  at 00538b14  'Total: '
direct callees (14):
  0x00733540  CExoString::Constructor
  0x005210F0  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00704800  (unnamed)
  0x00734010  CExoString::CStr
  0x007338D0  CExoString::operator=_2
  0x0058C370  (unnamed)
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 5):
  +0x10064 [dec 65636]  x5  via EAX,ECX,EDX
  +0x10068 [dec 65640]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005384b0 FUN_005384b0 size=1894 ret=4 callers=1 callees=14

undefined4 FUN_005384b0(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  undefined1 local_100 [8];
  undefined1 local_f8 [8];
  undefined1 local_f0 [8];
  undefined1 local_e8 [8];
  undefined1 local_e0 [8];
  undefined1 local_d8 [8];
  undefined1 local_d0 [8];
  undefined1 local_c8 [8];
  undefined1 local_c0 [8];
  undefined1 local_b8 [8];
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [8];
  undefined1 local_50 [8];
  undefined1 local_48 [8];
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  undefined4 local_28;
  int local_24;
  int local_20;
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f06f;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 1;
  FUN_00733540();
  local_8._0_1_ = 2;
  local_20 = 0;
  local_1c = 0;
  uVar1 = FUN_005210f0();
  FUN_00733700(uVar1);
  local_8._0_1_ = 3;
  FUN_00733570("ServerAdmins: ");
  local_8._0_1_ = 4;
  FUN_00733df0(local_40,local_38);
  local_8._0_1_ = 5;
  uVar1 = FUN_00733df0(local_50,local_48);
  local_8._0_1_ = 6;
  FUN_007337c0(uVar1);
  local_8._0_1_ = 5;
  FUN_00733780();
  local_8._0_1_ = 4;
  FUN_00733780();
  local_8._0_1_ = 3;
  FUN_00733780();
  local_8 = CONCAT31(local_8._1_3_,2);
  FUN_00733780();
  local_24 = FUN_007a1720();
  while (local_24 != 0) {
    local_1c = FUN_00561430(local_24);
    uVar1 = *(undefined4 *)(local_1c + 4);
    (**(code **)(**(int **)(DAT_00a1b4a4 + 8) + 0x10))(uVar1);
    local_28 = FUN_00704800(uVar1);
    uVar1 = FUN_00734010();
    FUN_007338d0(uVar1);
    FUN_00733570(&DAT_00986b2c);
/* ... 118 more lines truncated by --max-lines */
```

### decoy: 0x00538c20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00538c20: 16-byte aligned: YES
  00538c20  55             PUSH EBP
  00538c21  8B EC          MOV EBP,ESP
  00538c23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00538c20
string literals (0):
direct callees (2):
  0x00538CD0  (unnamed)
  0x00538ED0  (unnamed)
struct offsets touched (top 0):
```

## Card B9-2-57

GOG function: CServerExoApp::ClearLastModuleFound  gog 0x0064c900  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051cee0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051ceb0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c8d0 CServerExoApp::GetNormalOfPoint -> 0x0051ceb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c920 CServerExoApp::ClearAreaCache -> 0x0051cf00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b28f0 CServerExoAppInternal::ClearLastModuleFound -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051cee0: 16-byte aligned: YES
  0051cee0  55             PUSH EBP
  0051cee1  8B EC          MOV EBP,ESP
  0051cee3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cee0
string literals (0):
direct callees (1):
  0x0052F570  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cee0 FUN_0051cee0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x0052F570 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051cee0(undefined4 param_1)

{
  FUN_0052f570(param_1);
  return;
}
```

### decoy: 0x0051ceb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051ceb0: 16-byte aligned: YES
  0051ceb0  55             PUSH EBP
  0051ceb1  8B EC          MOV EBP,ESP
  0051ceb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051ceb0
string literals (0):
direct callees (1):
  0x0053AAE0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-58

GOG function: CServerExoApp::ComputeHeight  gog 0x0064c460  callconv=(blank)  param_size_bytes=0  gog_size=81  gog_retn=12
Candidate Steam address: 0x0051ca40 (size 81, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: ComputeHeight 0x54b130; callers: PathPointDFSearch 0x554eb0)
- best decoy (neighbour/other candidate) scores: 0x0051caa0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0064c410 CServerExoApp::GetCreaturePathfindInformation -> 0x0051c9f0  gate_class=AUTO
- GOG-order neighbour after:  0x0064c4c0 CServerExoApp::UpDatePlayerPosition -> 0x0051caa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x007054d0 CSWSArea::ComputeHeight -> Steam 0x0054b130  in candidate callees: yes
  0x0072a6a0 CSWSModule::GetArea -> Steam ?  in candidate callees: n/a
  0x007b2840 CServerExoAppInternal::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x0070f0d0 CSWSArea::PathPointDFSearch -> Steam 0x00554eb0  in candidate callers: yes
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
prologue for 0x0051ca40: 16-byte aligned: YES
  0051ca40  55             PUSH EBP
  0051ca41  8B EC          MOV EBP,ESP
  0051ca43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051ca40
string literals (0):
direct callees (3):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EF0  (unnamed)
  0x0054B130  CSWSArea::ComputeHeight
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051ca40 FUN_0051ca40 size=81 ret=12 callers=8 callees=3

float10 __thiscall
FUN_0051ca40(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  float10 fVar2;
  
  iVar1 = FUN_0052f4c0(param_1);
  if (iVar1 != 0) {
    FUN_0052f4c0(param_1);
    iVar1 = FUN_00557ef0();
    if (iVar1 != 0) {
      fVar2 = (float10)FUN_0054b130(param_3,param_4,0,0);
      return fVar2;
    }
  }
  return (float10)0;
}
```

### decoy: 0x0051caa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051caa0: 16-byte aligned: YES
  0051caa0  55             PUSH EBP
  0051caa1  8B EC          MOV EBP,ESP
  0051caa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051caa0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x00538C50  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-2-59

GOG function: CServerExoApp::RunModule  gog 0x0064bf60  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c540 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c520:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bf40 CServerExoApp::LoadModule -> 0x0051c520  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bf80 CServerExoApp::GetPlayerListString -> 0x0051c560  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b9510 CServerExoAppInternal::RunModule -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c540: 16-byte aligned: YES
  0051c540  55             PUSH EBP
  0051c541  8B EC          MOV EBP,ESP
  0051c543  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c540
string literals (0):
direct callees (1):
  0x005361D0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c540 FUN_0051c540 size=22 ret=0 callers=2 callees=1
THUNK -> 0x005361D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c540(undefined4 param_1)

{
  FUN_005361d0(param_1);
  return;
}
```

### decoy: 0x0051c520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c520: 16-byte aligned: YES
  0051c520  55             PUSH EBP
  0051c521  8B EC          MOV EBP,ESP
  0051c523  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c520
string literals (0):
direct callees (1):
  0x00531C50  CServerExoAppInternal::LoadModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B9-2-60

GOG function: CServerExoApp::TogglePauseState  gog 0x0064c0d0  callconv=(blank)  param_size_bytes=1  gog_size=29  gog_retn=4
Candidate Steam address: 0x0051c6b0 (size 29, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: OnApplyTimestop 0x5a4d70, OnRemoveTimestop 0x5a4f20)
- best decoy (neighbour/other candidate) scores: 0x0051c740:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c0b0 CServerExoApp::GetFactionOfObject -> 0x0051c690  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c0f0 CServerExoApp::SkipPastDialog -> 0x0051c6d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bbe70 CServerExoAppInternal::TogglePauseState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x00747660 CSWSEffectListHandler::OnApplyTimestop -> Steam 0x005a4d70  in candidate callers: yes
  0x00747810 CSWSEffectListHandler::OnRemoveTimestop -> Steam 0x005a4f20  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0051c6b0: 16-byte aligned: YES
  0051c6b0  55             PUSH EBP
  0051c6b1  8B EC          MOV EBP,ESP
  0051c6b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c6b0
string literals (0):
direct callees (1):
  0x00538C20  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c6b0 FUN_0051c6b0 size=29 ret=4 callers=5 callees=1
THUNK -> 0x00538C20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c6b0(undefined1 param_1)

{
  FUN_00538c20(param_1);
  return;
}
```

### decoy: 0x0051c740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c740: 16-byte aligned: YES
  0051c740  55             PUSH EBP
  0051c741  8B EC          MOV EBP,ESP
  0051c743  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c740
string literals (0):
direct callees (1):
  0x00538DC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```
