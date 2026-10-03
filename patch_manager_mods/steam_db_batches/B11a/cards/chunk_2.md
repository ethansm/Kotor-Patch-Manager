# Review cards B11a chunk 2

Answer each card with one TSV row in `steam_db_batches/B11a/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B11a-2-01

GOG function: CClientExoApp::HideLoadScreen  gog 0x0040d430  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073ff50 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: UnpackQuickSaveGame 0x7d1530, ExecuteCommandPlayMovie 0x68e720, StallEventSaveGame 0x534600)
- best decoy (neighbour/other candidate) scores: 0x0073ff20:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x0040d400 CClientExoApp::ShowLoadScreen -> 0x0073ff20  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d460 CClientExoApp::HandleInputEvent -> 0x0073ff80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444a00 CClientExoAppInternal::HideLoadScreen -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x004e1c30 CGuiInGame::UnpackQuickSaveGame -> Steam 0x007d1530  in candidate callers: yes
  0x007a3d70 Global::ExecuteCommandPlayMovie -> Steam 0x0068e720  in candidate callers: yes
  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> Steam 0x00534600  in candidate callers: yes
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
prologue for 0x0073ff50: 16-byte aligned: YES
  0073ff50  55             PUSH EBP
  0073ff51  8B EC          MOV EBP,ESP
  0073ff53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073ff50
string literals (0):
direct callees (1):
  0x007983E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073ff50 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=5 callees=1
THUNK -> 0x007983E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_007983e0(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x0073ff20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ff20: 16-byte aligned: YES
  0073ff20  55             PUSH EBP
  0073ff21  8B EC          MOV EBP,ESP
  0073ff23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ff20
string literals (0):
direct callees (1):
  0x00798190  CClientExoAppInternal::ShowLoadScreen
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-02

GOG function: CClientExoApp::SetEventDescriptions  gog 0x0040cfe0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fb00 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fad0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cfb0 CClientExoApp::StartServices -> 0x0073fad0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d000 CClientExoApp::StopServices -> 0x0073fb20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00432410 CClientExoAppInternal::SetEventDescriptions -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073fb00: 16-byte aligned: YES
  0073fb00  55             PUSH EBP
  0073fb01  8B EC          MOV EBP,ESP
  0073fb03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fb00
string literals (0):
direct callees (1):
  0x00785DF0  CClientExoAppInternal::SetEventDescriptions
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fb00 FUN_0073fb00 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00785DF0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fb00(undefined4 param_1)

{
  FUN_00785df0(param_1);
  return;
}
```

### decoy: 0x0073fad0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fad0: 16-byte aligned: YES
  0073fad0  55             PUSH EBP
  0073fad1  8B EC          MOV EBP,ESP
  0073fad3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fad0
string literals (0):
direct callees (1):
  0x00788D90  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-2-03

GOG function: CClientExoApp::GetMoveToModuleMovies  gog 0x0040d660  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007401a0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073ff20:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d640 CClientExoApp::AddMoveToModuleMovie -> 0x00740180  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d680 CClientExoApp::RemoveMoveToModuleMovies -> 0x007401c0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007be170 CServerExoAppInternal::DoPCAutosave -> Steam 0x0053af20  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x2d8*} | Steam {0x2d8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007401a0: 16-byte aligned: YES
  007401a0  55             PUSH EBP
  007401a1  8B EC          MOV EBP,ESP
  007401a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007401a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007401a0 FUN_007401a0 size=22 ret=0 callers=2 callees=0

int __fastcall FUN_007401a0(int param_1)

{
  return *(int *)(param_1 + 4) + 0x2d8;
}
```

### decoy: 0x0073ff20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ff20: 16-byte aligned: YES
  0073ff20  55             PUSH EBP
  0073ff21  8B EC          MOV EBP,ESP
  0073ff23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ff20
string literals (0):
direct callees (1):
  0x00798190  CClientExoAppInternal::ShowLoadScreen
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-04

GOG function: CClientExoApp::ServerToClientObjectId  gog 0x0040d3a0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073fec0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 25 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 25 (callers: GetTargetActions 0x842d40, AIActionPhysicalAttacks 0x6d7390, Destructor_2 0x6000e0)
- best decoy (neighbour/other candidate) scores: 0x00740260:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d380 CClientExoApp::GetSWGuiManager -> 0x0073fea0  gate_class=AUTO
- GOG-order neighbour after:  0x0040d3c0 CClientExoApp::SetInputClass -> 0x0073fee0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444780 CClientExoAppInternal::ServerToClientObjectId -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (26 of 88): the card's A-CALLEE credit may come from these, not from callees
  0x005365c0 CSWCPlaceable::GetTargetActions -> Steam 0x00842d40  in candidate callers: yes
  0x00653640 CSWSCreature::AIActionPhysicalAttacks -> Steam 0x006d7390  in candidate callers: yes
  0x00663910 CSWSItem::Destructor_2 -> Steam 0x006000e0  in candidate callers: yes
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callers: yes
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
prologue for 0x0073fec0: 16-byte aligned: YES
  0073fec0  55             PUSH EBP
  0073fec1  8B EC          MOV EBP,ESP
  0073fec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fec0
string literals (0):
direct callees (1):
  0x00798160  CClientExoAppInternal::ServerToClientObjectId
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fec0 FUN_0073fec0 size=28 ret=4 callers=88 callees=1
THUNK -> 0x00798160 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fec0(undefined4 param_1)

{
  FUN_00798160(param_1);
  return;
}
```

### decoy: 0x00740260 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740260: 16-byte aligned: YES
  00740260  55             PUSH EBP
  00740261  8B EC          MOV EBP,ESP
  00740263  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740260
string literals (0):
direct callees (1):
  0x0079BC40  CClientExoAppInternal::BarkString
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-05

GOG function: CClientExoApp::SetDebugMode  gog 0x0040cf90  callconv=__thiscall  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073fa70 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::SetCaptureEvents | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fa50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cf70 CClientExoApp::SetCaptureEventPlayBackRate -> 0x0073fab0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cfb0 CClientExoApp::StartServices -> 0x0073fad0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00439c50 CClientExoAppInternal::SetDebugMode -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0073fa70: 16-byte aligned: YES
  0073fa70  55             PUSH EBP
  0073fa71  8B EC          MOV EBP,ESP
  0073fa73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fa70
string literals (0):
direct callees (1):
  0x00785B50  CClientExoAppInternal::SetCaptureEventPlayBack
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fa70 FUN_0073fa70 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00785B50 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fa70(undefined4 param_1)

{
  FUN_00785b50(param_1);
  return;
}
```

### decoy: 0x0073fa50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fa50: 16-byte aligned: YES
  0073fa50  55             PUSH EBP
  0073fa51  8B EC          MOV EBP,ESP
  0073fa53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fa50
string literals (0):
direct callees (1):
  0x007858D0  CClientExoAppInternal::SetCaptureEvents
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-06

GOG function: CClientExoApp::IsPlayerCreature  gog 0x0040d360  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073fe80 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fec0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d340 CClientExoApp::TurnOffKeyboardModal -> 0x0073fe60  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d380 CClientExoApp::GetSWGuiManager -> 0x0073fea0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444bf0 CClientExoAppInternal::IsPlayerCreature -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073fe80: 16-byte aligned: YES
  0073fe80  55             PUSH EBP
  0073fe81  8B EC          MOV EBP,ESP
  0073fe83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fe80
string literals (0):
direct callees (1):
  0x007985D0  CClientExoAppInternal::IsPlayerCreature
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fe80 FUN_0073fe80 size=28 ret=4 callers=3 callees=1
THUNK -> 0x007985D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fe80(undefined4 param_1)

{
  FUN_007985d0(param_1);
  return;
}
```

### decoy: 0x0073fec0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fec0: 16-byte aligned: YES
  0073fec0  55             PUSH EBP
  0073fec1  8B EC          MOV EBP,ESP
  0073fec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fec0
string literals (0):
direct callees (1):
  0x00798160  CClientExoAppInternal::ServerToClientObjectId
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-07

GOG function: CClientExoApp::ShowLoadScreen  gog 0x0040d400  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073ff20 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 4 (callers: DoQuickLoad 0x7d0f50, LoadGame 0x853a00, StallEventSaveGame 0x534600)
- best decoy (neighbour/other candidate) scores: 0x0073fea0:STR0/CALLEE2/SIG0.25/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x0040d3e0 CClientExoApp::GetInputClass -> 0x0073ff00  gate_class=AUTO
- GOG-order neighbour after:  0x0040d430 CClientExoApp::HideLoadScreen -> 0x0073ff50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004447b0 CClientExoAppInternal::ShowLoadScreen -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (4 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x004e1650 CGuiInGame::DoQuickLoad -> Steam 0x007d0f50  in candidate callers: yes
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> Steam 0x00534600  in candidate callers: yes
  0x007be170 CServerExoAppInternal::DoPCAutosave -> Steam 0x0053af20  in candidate callers: yes
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
prologue for 0x0073ff20: 16-byte aligned: YES
  0073ff20  55             PUSH EBP
  0073ff21  8B EC          MOV EBP,ESP
  0073ff23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073ff20
string literals (0):
direct callees (1):
  0x00798190  CClientExoAppInternal::ShowLoadScreen
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073ff20 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=8 callees=1
THUNK -> 0x00798190 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00798190(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x0073fea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fea0: 16-byte aligned: YES
  0073fea0  55             PUSH EBP
  0073fea1  8B EC          MOV EBP,ESP
  0073fea3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fea0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x274 [dec 628]  x1  via ECX
```

## Card B11a-2-08

GOG function: CClientExoApp::DisableInput  gog 0x0040d790  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007402d0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: LoadGame 0x853a00, ExecuteLoadModule 0x530170)
- best decoy (neighbour/other candidate) scores: 0x00740240:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d770 CClientExoApp::StopSoundPlayingInDialog -> 0x007402b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d7b0 CClientExoApp::EnableInput -> 0x007402f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0045ff80 CClientExoAppInternal::DisableInput -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
  0x007b34f0 CServerExoAppInternal::ExecuteLoadModule -> Steam 0x00530170  in candidate callers: yes
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
prologue for 0x007402d0: 16-byte aligned: YES
  007402d0  55             PUSH EBP
  007402d1  8B EC          MOV EBP,ESP
  007402d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007402d0
string literals (0):
direct callees (1):
  0x007B3370  CClientExoAppInternal::DisableInput
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007402d0 FUN_007402d0 size=22 ret=0 callers=3 callees=1
THUNK -> 0x007B3370 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007402d0(undefined4 param_1)

{
  FUN_007b3370(param_1);
  return;
}
```

### decoy: 0x00740240 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740240: 16-byte aligned: YES
  00740240  55             PUSH EBP
  00740241  8B EC          MOV EBP,ESP
  00740243  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740240
string literals (0):
direct callees (1):
  0x0078C300  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-09

GOG function: CClientExoApp::SetVideoMode  gog 0x0040d0b0  callconv=(blank)  param_size_bytes=4  gog_size=30  gog_retn=4
Candidate Steam address: 0x0073fbd0 (size 30, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fbf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d090 CClientExoApp::GetHitCheckMaterialMask -> 0x0073fbb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d0d0 CClientExoApp::SetTexturePack -> 0x0073fbf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043a1c0 CClientExoAppInternal::SetVideoMode -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073fbd0: 16-byte aligned: YES
  0073fbd0  55             PUSH EBP
  0073fbd1  8B EC          MOV EBP,ESP
  0073fbd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fbd0
string literals (0):
direct callees (1):
  0x0078DBF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fbd0 FUN_0073fbd0 size=30 ret=4 callers=1 callees=1
THUNK -> 0x0078DBF0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fbd0(undefined4 param_1)

{
  FUN_0078dbf0(param_1,0);
  return;
}
```

### decoy: 0x0073fbf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fbf0: 16-byte aligned: YES
  0073fbf0  55             PUSH EBP
  0073fbf1  8B EC          MOV EBP,ESP
  0073fbf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fbf0
string literals (0):
direct callees (1):
  0x0078D6A0  CClientExoApp::SetTexturePack (texpacks.2da row: Texture, Gui, DynMemRatio, Mem)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-10

GOG function: CClientExoApp::StopMenuMusic  gog 0x0040d250  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fd90 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fd70:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d220 CClientExoApp::StartMenuMusic -> 0x0073fd40  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d270 CClientExoApp::StopLoadMusic -> 0x0073fd70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043adb0 CClientExoAppInternal::StopMenuMusic -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00454600 CSWCArea::StartSounds -> Steam 0x007a7c00  in candidate callers: yes
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
prologue for 0x0073fd90: 16-byte aligned: YES
  0073fd90  55             PUSH EBP
  0073fd91  8B EC          MOV EBP,ESP
  0073fd93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fd90
string literals (0):
direct callees (1):
  0x0078EA50  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fd90 FUN_0073fd90 size=22 ret=0 callers=3 callees=1
THUNK -> 0x0078EA50 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fd90(undefined4 param_1)

{
  FUN_0078ea50(param_1);
  return;
}
```

### decoy: 0x0073fd70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fd70: 16-byte aligned: YES
  0073fd70  55             PUSH EBP
  0073fd71  8B EC          MOV EBP,ESP
  0073fd73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fd70
string literals (0):
direct callees (1):
  0x0078E7E0  CClientExoAppInternal::StartMenuMusic
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-11

GOG function: CClientExoAppInternal::ShutDownMoviePlayer  gog 0x004381a0  callconv=(blank)  param_size_bytes=4  gog_size=129  gog_retn=4
Candidate Steam address: 0x0078bbc0 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0078bc50:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438080 CServerInfo::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00438230 CClientExoAppInternal::CleanUpModuleServices -> 0x0078c170  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0xc*,0x14*,0x30*} | Steam {0x4*,0xc*,0x14*,0x30*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bbc0: 16-byte aligned: YES
  0078bbc0  55             PUSH EBP
  0078bbc1  8B EC          MOV EBP,ESP
  0078bbc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078bbc0
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078bbc0 FUN_0078bbc0 size=129 ret=4 callers=2 callees=3

undefined4 __thiscall FUN_0078bbc0(undefined4 param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_0096cec6;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_00733780(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = local_8 & 0xffffff00;
  FUN_00623320();
  local_8 = 0xffffffff;
  FUN_00733780();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0078bc50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bc50: 16-byte aligned: YES
  0078bc50  55             PUSH EBP
  0078bc51  8B EC          MOV EBP,ESP
  0078bc53  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bc50
string literals (0):
direct callees (4):
  0x0084BDA0  (unnamed)
  0x0084B490  (unnamed)
  0x007C7030  CGuiInGame::ShutDown
  0x00865220  (unnamed)
struct offsets touched (top 4):
  +0x38 [dec 56]  x2  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x40 [dec 64]  x2  via EAX,ECX
  +0x2A0 [dec 672]  x2  via EAX,EDX
```

## Card B11a-2-12

GOG function: CClientExoApp::SetStartMenuMusicOnMovieEnd  gog 0x0040d2b0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0073fdd0 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::AquireLoadMusic | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00740040:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d290 CClientExoApp::AquireLoadMusic -> 0x0073fdd0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d2e0 CClientExoApp::GetActivePauseState -> 0x0073fe00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x1ec*} | Steam {0x4*,0x1ec*}
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
prologue for 0x0073fdd0: 16-byte aligned: YES
  0073fdd0  55             PUSH EBP
  0073fdd1  8B EC          MOV EBP,ESP
  0073fdd3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fdd0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1EC [dec 492]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fdd0 FUN_0073fdd0 size=36 ret=4 callers=3 callees=0

void __thiscall FUN_0073fdd0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x1ec) = param_2;
  return;
}
```

### decoy: 0x00740040 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740040: 16-byte aligned: YES
  00740040  55             PUSH EBP
  00740041  8B EC          MOV EBP,ESP
  00740043  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740040
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x288 [dec 648]  x1  via EDX
```

## Card B11a-2-13

GOG function: CClientExoApp::AquireLoadMusic  gog 0x0040d290  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073fdd0 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fdb0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d270 CClientExoApp::StopLoadMusic -> 0x0073fd70  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d2b0 CClientExoApp::SetStartMenuMusicOnMovieEnd -> 0x0073fdd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043b0a0 CClientExoAppInternal::AquireLoadMusic -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x1ec}
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
prologue for 0x0073fdd0: 16-byte aligned: YES
  0073fdd0  55             PUSH EBP
  0073fdd1  8B EC          MOV EBP,ESP
  0073fdd3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fdd0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1EC [dec 492]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fdd0 FUN_0073fdd0 size=36 ret=4 callers=3 callees=0

void __thiscall FUN_0073fdd0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x1ec) = param_2;
  return;
}
```

### decoy: 0x0073fdb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fdb0: 16-byte aligned: YES
  0073fdb0  55             PUSH EBP
  0073fdb1  8B EC          MOV EBP,ESP
  0073fdb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fdb0
string literals (0):
direct callees (1):
  0x0078EAD0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-14

GOG function: CClientExoAppInternal::GetLoadScreenVisible  gog 0x0044dff0  callconv=(blank)  param_size_bytes=0  gog_size=64  gog_retn=0
Candidate Steam address: 0x007a2040 (size 197, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2010:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044dfb0 CClientExoAppInternal::HandleExitGameButton -> 0x007a1ca0  gate_class=REVIEW
- GOG-order neighbour after:  0x0044e030 CExoArrayList_CSWGuiSkillStruct_::CExoArrayList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044e390 CSWRumblePattern::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0xc,0x14}
imm sets (* = in both, 0 common): GOG {0x3,0x10} | Steam {0x4,0x20}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007a2040: 16-byte aligned: YES
  007a2040  55             PUSH EBP
  007a2041  8B EC          MOV EBP,ESP
  007a2043  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007a2040
string literals (0):
direct callees (5):
  0x007A1780  (unnamed)
  0x00521060  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x007A21D0  CClientExoAppInternal::SetGameOver
  0x0072F5F0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007a2040 FUN_007a2040 size=197 ret=0 callers=1 callees=5

void FUN_007a2040(void)

{
  uint uVar1;
  int iVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 uVar4;
  int *local_24;
  int local_20;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00967e0b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8 = 0;
  while( true ) {
    iVar2 = FUN_007a1780(uVar1);
    if (iVar2 != 0) break;
    uVar3 = extraout_var;
    local_14 = FUN_00521060();
    local_1c = *(int *)(DAT_00a1b4a4 + 4);
    local_20 = *(int *)(local_1c + 4);
    local_24 = (int *)0x0;
    uVar4 = *(undefined4 *)(local_20 + 0x14);
    FUN_0053dfb0(local_14,&local_24);
    local_18 = (**(code **)(*local_24 + 0xc))(uVar1,local_24,uVar3,uVar4);
    FUN_007a21d0(local_18);
  }
  local_8 = 0xffffffff;
  FUN_0072f5f0();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007a2010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2010: 16-byte aligned: YES
  007a2010  55             PUSH EBP
  007a2011  8B EC          MOV EBP,ESP
  007a2013  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a2010
string literals (0):
direct callees (1):
  0x00722870  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B11a-2-15

GOG function: CClientExoApp::QueryExitProgram  gog 0x0040d300  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fe20 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d2e0 CClientExoApp::GetActivePauseState -> 0x0073fe00  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d320 CClientExoApp::SetInputActive -> 0x0073fe40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004431c0 CClientExoAppInternal::QueryExitProgram -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073fe20: 16-byte aligned: YES
  0073fe20  55             PUSH EBP
  0073fe21  8B EC          MOV EBP,ESP
  0073fe23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fe20
string literals (0):
direct callees (1):
  0x00796BA0  CClientExoAppInternal::QueryExitProgram
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fe20 FUN_0073fe20 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00796BA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fe20(undefined4 param_1)

{
  FUN_00796ba0(param_1);
  return;
}
```

### decoy: 0x0073fe00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fe00: 16-byte aligned: YES
  0073fe00  55             PUSH EBP
  0073fe01  8B EC          MOV EBP,ESP
  0073fe03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fe00
string literals (0):
direct callees (1):
  0x00784440  CClientExoAppInternal::GetActivePauseState
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-16

GOG function: CClientExoApp::GetHitCheckMaterialMask  gog 0x0040d090  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0073fbb0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetSWParty | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: SetPosition 0x8f1040, SetInstanceRandomPosition 0x8f19e0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073fb90:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d070 CClientExoApp::GetSWParty -> 0x0073fbb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d0b0 CClientExoApp::SetVideoMode -> 0x0073fbd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00584a20 CSWCAreaOfEffectObject::SetPosition -> Steam 0x008f1040  in candidate callers: yes
  0x005853c0 CSWCAreaOfEffectObject::SetInstanceRandomPosition -> Steam 0x008f19e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0xa4*} | Steam {0x4*,0xa4*}
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
prologue for 0x0073fbb0: 16-byte aligned: YES
  0073fbb0  55             PUSH EBP
  0073fbb1  8B EC          MOV EBP,ESP
  0073fbb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fbb0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0xA4 [dec 164]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fbb0 FUN_0073fbb0 size=31 ret=0 callers=3 callees=0

undefined4 __fastcall FUN_0073fbb0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0xa4);
}
```

### decoy: 0x0073fb90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fb90: 16-byte aligned: YES
  0073fb90  55             PUSH EBP
  0073fb91  8B EC          MOV EBP,ESP
  0073fb93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fb90
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x270 [dec 624]  x1  via ECX
```

## Card B11a-2-17

GOG function: CClientExoApp::EnableInput  gog 0x0040d7b0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007402f0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007402d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d790 CClientExoApp::DisableInput -> 0x007402d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d7d0 CClientExoApp::DismissInGameGUI -> 0x00740310  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0045ffa0 CClientExoAppInternal::EnableInput -> Steam ?  in candidate callees: n/a
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
prologue for 0x007402f0: 16-byte aligned: YES
  007402f0  55             PUSH EBP
  007402f1  8B EC          MOV EBP,ESP
  007402f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007402f0
string literals (0):
direct callees (1):
  0x007B3390  CClientExoAppInternal::EnableInput
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007402f0 FUN_007402f0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x007B3390 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007402f0(undefined4 param_1)

{
  FUN_007b3390(param_1);
  return;
}
```

### decoy: 0x007402d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007402d0: 16-byte aligned: YES
  007402d0  55             PUSH EBP
  007402d1  8B EC          MOV EBP,ESP
  007402d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007402d0
string literals (0):
direct callees (1):
  0x007B3370  CClientExoAppInternal::DisableInput
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-18

GOG function: CClientExoApp::SetCaptureEvents  gog 0x0040cf30  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073fa70 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fa50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cf10 CClientExoApp::Uninitialize -> 0x0073fa30  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cf50 CClientExoApp::SetCaptureEventPlayBack -> 0x0073fa90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431ef0 CClientExoAppInternal::SetCaptureEvents -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073fa70: 16-byte aligned: YES
  0073fa70  55             PUSH EBP
  0073fa71  8B EC          MOV EBP,ESP
  0073fa73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fa70
string literals (0):
direct callees (1):
  0x00785B50  CClientExoAppInternal::SetCaptureEventPlayBack
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fa70 FUN_0073fa70 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00785B50 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fa70(undefined4 param_1)

{
  FUN_00785b50(param_1);
  return;
}
```

### decoy: 0x0073fa50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fa50: 16-byte aligned: YES
  0073fa50  55             PUSH EBP
  0073fa51  8B EC          MOV EBP,ESP
  0073fa53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fa50
string literals (0):
direct callees (1):
  0x007858D0  CClientExoAppInternal::SetCaptureEvents
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-19

GOG function: CClientExoAppInternal::GetLoadScreenVisible  gog 0x0044dff0  callconv=(blank)  param_size_bytes=0  gog_size=64  gog_retn=0
Candidate Steam address: 0x007a1b90 (size 86, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::GetPausedByCombat | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a1ae0:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: autolabel:~CMemoryDC
- GOG-order neighbour before: 0x0044dfb0 CClientExoAppInternal::HandleExitGameButton -> 0x007a1ca0  gate_class=REVIEW
- GOG-order neighbour after:  0x0044e030 CExoArrayList_CSWGuiSkillStruct_::CExoArrayList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044e390 CSWRumblePattern::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x3,0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007a1b90: 16-byte aligned: YES
  007a1b90  55             PUSH EBP
  007a1b91  8B EC          MOV EBP,ESP
  007a1b93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007a1b90
string literals (0):
direct callees (1):
  0x0088F9A0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a1b90 AfxRibbonCollector::CMemoryDC::~CMemoryDC size=86 ret=0 callers=0 callees=1

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: __thiscall AfxRibbonCollector::CMemoryDC::~CMemoryDC(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall AfxRibbonCollector::CMemoryDC::~CMemoryDC(CMemoryDC *this)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00975d68;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 0;
  FUN_0088f9a0(DAT_00a11f20 ^ (uint)&stack0xfffffffc,this);
  local_8 = 0xffffffff;
  FUN_0088f9a0();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007a1ae0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a1ae0: 16-byte aligned: YES
  007a1ae0  55             PUSH EBP
  007a1ae1  8B EC          MOV EBP,ESP
  007a1ae3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007a1ae0
string literals (0):
direct callees (3):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
  0x0088F9A0  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-2-20

GOG function: CClientExoApp::IsMoviePlaying  gog 0x0040d620  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740160 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandIsMoviePlaying 0x69a3a0)
- best decoy (neighbour/other candidate) scores: 0x00740110:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d5d0 CClientExoApp::PlayMovie -> 0x00740110  gate_class=AUTO
- GOG-order neighbour after:  0x0040d640 CClientExoApp::AddMoveToModuleMovie -> 0x00740180  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00445220 CClientExoAppInternal::IsMoviePlaying -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x007af9f0 Global::ExecuteCommandIsMoviePlaying -> Steam 0x0069a3a0  in candidate callers: yes
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
prologue for 0x00740160: 16-byte aligned: YES
  00740160  55             PUSH EBP
  00740161  8B EC          MOV EBP,ESP
  00740163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740160
string literals (0):
direct callees (1):
  0x00798C00  IsMoviePlaying
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740160 FUN_00740160 size=22 ret=0 callers=4 callees=1
THUNK -> 0x00798C00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740160(undefined4 param_1)

{
  FUN_00798c00(param_1);
  return;
}
```

### decoy: 0x00740110 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740110: 16-byte aligned: YES
  00740110  55             PUSH EBP
  00740111  8B EC          MOV EBP,ESP
  00740113  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740110
string literals (0):
direct callees (3):
  0x00734010  CExoString::CStr
  0x00401650  (unnamed)
  0x00798BA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-2-21

GOG function: CClientExoApp::GetAreaNotReadyToDisplay  gog 0x0040d500  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00740020 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetPlayerCharacterSet | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callers: ShowSoloModeQuery 0x7cd510, MainLoop 0x533830, TempDeathUpdate 0x53a0c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007400d0:STR0/CALLEE1/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d4e0 CClientExoApp::GetPlayerCharacterSet -> 0x00740000  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d520 CClientExoApp::SetAreaNotReadyToDisplay -> 0x00740040  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (3 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x004ddc10 CGuiInGame::ShowSoloModeQuery -> Steam 0x007cd510  in candidate callers: yes
  0x007b6bb0 CServerExoAppInternal::MainLoop -> Steam 0x00533830  in candidate callers: yes
  0x007bd310 CServerExoAppInternal::TempDeathUpdate -> Steam 0x0053a0c0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x288*} | Steam {0x4*,0x288*}
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
prologue for 0x00740020: 16-byte aligned: YES
  00740020  55             PUSH EBP
  00740021  8B EC          MOV EBP,ESP
  00740023  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740020
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x288 [dec 648]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740020 FUN_00740020 size=31 ret=0 callers=8 callees=0

undefined4 __fastcall FUN_00740020(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x288);
}
```

### decoy: 0x007400d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007400d0: 16-byte aligned: YES
  007400d0  55             PUSH EBP
  007400d1  8B EC          MOV EBP,ESP
  007400d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007400d0
string literals (0):
direct callees (1):
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-22

GOG function: CClientExoApp::IndicateObject  gog 0x0040d6b0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x007401f0 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740350:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d680 CClientExoApp::RemoveMoveToModuleMovies -> 0x007401c0  gate_class=AUTO
- GOG-order neighbour after:  0x0040d6d0 CClientExoApp::ReattachCreatureToScene -> 0x00740210  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00443170 CSWCCreature::SwitchMode -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x004dae30 CGuiInGame::ShowGalaxyMapGui -> Steam 0x007ca730  in candidate callers: yes
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
prologue for 0x007401f0: 16-byte aligned: YES
  007401f0  55             PUSH EBP
  007401f1  8B EC          MOV EBP,ESP
  007401f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007401f0
string literals (0):
direct callees (1):
  0x00796B50  SetHudTarget
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007401f0 FUN_007401f0 size=32 ret=8 callers=3 callees=1
THUNK -> 0x00796B50 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007401f0(undefined4 param_1,undefined4 param_2)

{
  FUN_00796b50(param_1,param_2);
  return;
}
```

### decoy: 0x00740350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740350: 16-byte aligned: YES
  00740350  55             PUSH EBP
  00740351  8B EC          MOV EBP,ESP
  00740353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740350
string literals (0):
direct callees (1):
  0x0079BF40  CClientExoAppInternal::SetPausedByCombat
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-23

GOG function: CClientExoApp::AddMoveToModuleMovie  gog 0x0040d640  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740180 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: LoadPCAutoSave 0x8529e0, EventHandler 0x5221b0, ExecuteCommandStartNewModule 0x693c10)
- best decoy (neighbour/other candidate) scores: 0x00740210:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d620 CClientExoApp::IsMoviePlaying -> 0x00740160  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d660 CClientExoApp::GetMoveToModuleMovies -> 0x007401a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00445190 CClientExoAppInternal::AddMoveToModuleMovie -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: yes
  0x00717360 CSWSArea::EventHandler -> Steam 0x005221b0  in candidate callers: yes
  0x007a9260 Global::ExecuteCommandStartNewModule -> Steam 0x00693c10  in candidate callers: yes
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
prologue for 0x00740180: 16-byte aligned: YES
  00740180  55             PUSH EBP
  00740181  8B EC          MOV EBP,ESP
  00740183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740180
string literals (0):
direct callees (1):
  0x00798B70  CClientExoAppInternal::AddMoveToModuleMovie
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740180 FUN_00740180 size=28 ret=4 callers=3 callees=1
THUNK -> 0x00798B70 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740180(undefined4 param_1)

{
  FUN_00798b70(param_1);
  return;
}
```

### decoy: 0x00740210 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740210: 16-byte aligned: YES
  00740210  55             PUSH EBP
  00740211  8B EC          MOV EBP,ESP
  00740213  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740210
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x28C [dec 652]  x1  via ECX
```

## Card B11a-2-24

GOG function: CClientExoApp::IsSoundPlayingInDialog  gog 0x0040d750  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740290 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740260:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d720 CClientExoApp::BarkString -> 0x007404d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d770 CClientExoApp::StopSoundPlayingInDialog -> 0x007402b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004482c0 CClientExoAppInternal::IsSoundPlayingInDialog -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740290: 16-byte aligned: YES
  00740290  55             PUSH EBP
  00740291  8B EC          MOV EBP,ESP
  00740293  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740290
string literals (0):
direct callees (1):
  0x0079BCC0  CClientExoAppInternal::IsSoundPlayingInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740290 FUN_00740290 size=22 ret=0 callers=1 callees=1
THUNK -> 0x0079BCC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740290(undefined4 param_1)

{
  FUN_0079bcc0(param_1);
  return;
}
```

### decoy: 0x00740260 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740260: 16-byte aligned: YES
  00740260  55             PUSH EBP
  00740261  8B EC          MOV EBP,ESP
  00740263  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740260
string literals (0):
direct callees (1):
  0x0079BC40  CClientExoAppInternal::BarkString
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-25

GOG function: CClientExoApp::SetCaptureEventPlayBack  gog 0x0040cf50  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073fa90 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fa70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cf30 CClientExoApp::SetCaptureEvents -> 0x0073fa70  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cf70 CClientExoApp::SetCaptureEventPlayBackRate -> 0x0073fab0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00432170 CClientExoAppInternal::SetCaptureEventPlayBack -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073fa90: 16-byte aligned: YES
  0073fa90  55             PUSH EBP
  0073fa91  8B EC          MOV EBP,ESP
  0073fa93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fa90
string literals (0):
direct callees (1):
  0x00785DA0  CClientExoAppInternal::SetCaptureEventPlayBackRate
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fa90 FUN_0073fa90 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00785DA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fa90(undefined4 param_1)

{
  FUN_00785da0(param_1);
  return;
}
```

### decoy: 0x0073fa70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fa70: 16-byte aligned: YES
  0073fa70  55             PUSH EBP
  0073fa71  8B EC          MOV EBP,ESP
  0073fa73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fa70
string literals (0):
direct callees (1):
  0x00785B50  CClientExoAppInternal::SetCaptureEventPlayBack
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-26

GOG function: CClientExoApp::GetMiniGame  gog 0x0040d580  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x007400a0 (size 42, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 21 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 21 (callers: HideItemCreateMenu 0x7caa60, HideSubItemCreateMenu 0x7cab60, HideLoadModuleDebugMenu 0x7cb4a0)
- best decoy (neighbour/other candidate) scores: 0x00740070:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d550 CClientExoApp::GetMGOArray -> 0x00740070  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d5b0 CClientExoApp::PlayLoadModuleMovies -> 0x007400f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040c8f0 CClientExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x00472050 CSWCModule::GetMiniGame -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (21 of 32): the card's A-CALLEE credit may come from these, not from callees
  0x004db160 CGuiInGame::HideItemCreateMenu -> Steam 0x007caa60  in candidate callers: yes
  0x004db260 CGuiInGame::HideSubItemCreateMenu -> Steam 0x007cab60  in candidate callers: yes
  0x004dbba0 CGuiInGame::HideLoadModuleDebugMenu -> Steam 0x007cb4a0  in candidate callers: yes
  0x004dbc40 CGuiInGame::HidePowersFeatsSkillsDebugMenu -> Steam 0x007cb540  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007400a0: 16-byte aligned: YES
  007400a0  55             PUSH EBP
  007400a1  8B EC          MOV EBP,ESP
  007400a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007400a0
string literals (0):
direct callees (2):
  0x0073F430  CClientExoApp::GetModule
  0x007FAE30  GetPlayerObject-like getter `[this+0x48]->+0x268`
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007400a0 FUN_007400a0 size=42 ret=0 callers=32 callees=2

undefined4 __fastcall FUN_007400a0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_0073f430(param_1);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = FUN_007fae30();
  }
  return uVar2;
}
```

### decoy: 0x00740070 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740070: 16-byte aligned: YES
  00740070  55             PUSH EBP
  00740071  8B EC          MOV EBP,ESP
  00740073  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740070
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-27

GOG function: CClientExoApp::SetPlayerCharacterName  gog 0x0040d4a0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073ffc0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073ff20:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d480 CClientExoApp::GetLoadScreenVisible -> 0x0073ffa0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d4c0 CClientExoApp::GetPlayerCharacterName -> 0x00740000  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444ba0 CClientExoAppInternal::SetPlayerCharacterName -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
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
prologue for 0x0073ffc0: 16-byte aligned: YES
  0073ffc0  55             PUSH EBP
  0073ffc1  8B EC          MOV EBP,ESP
  0073ffc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073ffc0
string literals (0):
direct callees (1):
  0x00798580  CClientExoAppInternal::SetPlayerCharacterName
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073ffc0 FUN_0073ffc0 size=28 ret=4 callers=3 callees=1
THUNK -> 0x00798580 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073ffc0(undefined4 param_1)

{
  FUN_00798580(param_1);
  return;
}
```

### decoy: 0x0073ff20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ff20: 16-byte aligned: YES
  0073ff20  55             PUSH EBP
  0073ff21  8B EC          MOV EBP,ESP
  0073ff23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ff20
string literals (0):
direct callees (1):
  0x00798190  CClientExoAppInternal::ShowLoadScreen
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-28

GOG function: CClientExoApp::SetInputClass  gog 0x0040d3c0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073fee0 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 19 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 19 (callers: ShowGalaxyMapGui 0x7ca730, HideGalaxyMapGui 0x7ca920, ShowItemCreateMenu 0x7caa10)
- best decoy (neighbour/other candidate) scores: 0x007400a0:STR0/CALLEE4/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d3a0 CClientExoApp::ServerToClientObjectId -> 0x0073fec0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d3e0 CClientExoApp::GetInputClass -> 0x0073ff00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0045fc60 CClientExoAppInternal::SetInputClass -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (19 of 62): the card's A-CALLEE credit may come from these, not from callees
  0x004dae30 CGuiInGame::ShowGalaxyMapGui -> Steam 0x007ca730  in candidate callers: yes
  0x004db020 CGuiInGame::HideGalaxyMapGui -> Steam 0x007ca920  in candidate callers: yes
  0x004db110 CGuiInGame::ShowItemCreateMenu -> Steam 0x007caa10  in candidate callers: yes
  0x004db160 CGuiInGame::HideItemCreateMenu -> Steam 0x007caa60  in candidate callers: yes
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
prologue for 0x0073fee0: 16-byte aligned: YES
  0073fee0  55             PUSH EBP
  0073fee1  8B EC          MOV EBP,ESP
  0073fee3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fee0
string literals (0):
direct callees (1):
  0x007B3050  CClientExoAppInternal::SetInputClass
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fee0 FUN_0073fee0 size=32 ret=8 callers=62 callees=1
THUNK -> 0x007B3050 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fee0(undefined4 param_1,undefined4 param_2)

{
  FUN_007b3050(param_1,param_2);
  return;
}
```

### decoy: 0x007400a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007400a0: 16-byte aligned: YES
  007400a0  55             PUSH EBP
  007400a1  8B EC          MOV EBP,ESP
  007400a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007400a0
string literals (0):
direct callees (2):
  0x0073F430  CClientExoApp::GetModule
  0x007FAE30  GetPlayerObject-like getter `[this+0x48]->+0x268`
struct offsets touched (top 0):
```

## Card B11a-2-29

GOG function: CClientExoApp::StopSoundPlayingInDialog  gog 0x0040d770  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007402b0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: StopSoundPlayingInDialog 0x546a70)
- best decoy (neighbour/other candidate) scores: 0x00740290:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d750 CClientExoApp::IsSoundPlayingInDialog -> 0x00740290  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d790 CClientExoApp::DisableInput -> 0x007402d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448440 CClientExoAppInternal::StopSoundPlayingInDialog -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x006ad750 CSWSObject::StopSoundPlayingInDialog -> Steam 0x00546a70  in candidate callers: yes
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
prologue for 0x007402b0: 16-byte aligned: YES
  007402b0  55             PUSH EBP
  007402b1  8B EC          MOV EBP,ESP
  007402b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007402b0
string literals (0):
direct callees (1):
  0x0079BE40  CClientExoAppInternal::StopSoundPlayingInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007402b0 FUN_007402b0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x0079BE40 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007402b0(undefined4 param_1)

{
  FUN_0079be40(param_1);
  return;
}
```

### decoy: 0x00740290 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740290: 16-byte aligned: YES
  00740290  55             PUSH EBP
  00740291  8B EC          MOV EBP,ESP
  00740293  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740290
string literals (0):
direct callees (1):
  0x0079BCC0  CClientExoAppInternal::IsSoundPlayingInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-30

GOG function: CClientExoApp::SetCaptureEventPlayBackRate  gog 0x0040cf70  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073fab0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fa90:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: autolabel:SetCompletionList
- GOG-order neighbour before: 0x0040cf50 CClientExoApp::SetCaptureEventPlayBack -> 0x0073fa90  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cf90 CClientExoApp::SetDebugMode -> 0x0073fa70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004323c0 CClientExoAppInternal::SetCaptureEventPlayBackRate -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073fab0: 16-byte aligned: YES
  0073fab0  55             PUSH EBP
  0073fab1  8B EC          MOV EBP,ESP
  0073fab3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fab0
string literals (0):
direct callees (1):
  0x0078D680  CClientExoAppInternal::SetDebugMode
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fab0 Concurrency::details::SchedulerResourceManagement::SetCompletionList size=28 ret=4 callers=1 callees=1

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::SetCompletionList(struct
   Concurrency::IUMSCompletionList *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::SetCompletionList
          (SchedulerResourceManagement *this,IUMSCompletionList *param_1)

{
  UMSThreadScheduler::SetCompletionList(*(UMSThreadScheduler **)(this + 4),param_1);
  return;
}
```

### decoy: 0x0073fa90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fa90: 16-byte aligned: YES
  0073fa90  55             PUSH EBP
  0073fa91  8B EC          MOV EBP,ESP
  0073fa93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fa90
string literals (0):
direct callees (1):
  0x00785DA0  CClientExoAppInternal::SetCaptureEventPlayBackRate
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-31

GOG function: CClientExoApp::HandleInputEvent  gog 0x0040d460  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073ff80 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d430 CClientExoApp::HideLoadScreen -> 0x0073ff50  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d480 CClientExoApp::GetLoadScreenVisible -> 0x0073ffa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073ff80: 16-byte aligned: YES
  0073ff80  55             PUSH EBP
  0073ff81  8B EC          MOV EBP,ESP
  0073ff83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073ff80
string literals (0):
direct callees (1):
  0x007B12C0  CClientExoAppInternal::HandleInputEvent
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073ff80 FUN_0073ff80 size=32 ret=8 callers=1 callees=1
THUNK -> 0x007B12C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073ff80(undefined4 param_1,undefined4 param_2)

{
  FUN_007b12c0(param_1,param_2);
  return;
}
```

### decoy: 0x0073fe40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fe40: 16-byte aligned: YES
  0073fe40  55             PUSH EBP
  0073fe41  8B EC          MOV EBP,ESP
  0073fe43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fe40
string literals (0):
direct callees (1):
  0x00797D90  CClientExoAppInternal::SetInputActive
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-2-32

GOG function: CClientExoApp::ShutDownToMainMenu  gog 0x0040d020  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fb40 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fb20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d000 CClientExoApp::StopServices -> 0x0073fb20  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d040 CClientExoApp::SetApplicationIdsMatch -> 0x0073fb60  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043a6c0 CClientExoAppInternal::ShutDownToMainMenu -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073fb40: 16-byte aligned: YES
  0073fb40  55             PUSH EBP
  0073fb41  8B EC          MOV EBP,ESP
  0073fb43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fb40
string literals (0):
direct callees (1):
  0x0078E0F0  CClientExoAppInternal::ShutDownToMainMenu
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fb40 FUN_0073fb40 size=22 ret=0 callers=0 callees=1
THUNK -> 0x0078E0F0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fb40(undefined4 param_1)

{
  FUN_0078e0f0(param_1);
  return;
}
```

### decoy: 0x0073fb20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fb20: 16-byte aligned: YES
  0073fb20  55             PUSH EBP
  0073fb21  8B EC          MOV EBP,ESP
  0073fb23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fb20
string literals (0):
direct callees (1):
  0x0078AC40  CClientExoAppInternal::StopServices
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-33

GOG function: CClientExoApp::GetDisableMovies  gog 0x0040d1a0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0073fcc0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetLoadBarState | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073fca0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d180 CClientExoApp::GetLoadBarState -> 0x0073fca0  gate_class=AUTO
- GOG-order neighbour after:  0x0040d1c0 CClientExoApp::RestartMoviesIfNecessary -> 0x0073fd00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x1f8*} | Steam {0x4*,0x1f8*}
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
prologue for 0x0073fcc0: 16-byte aligned: YES
  0073fcc0  55             PUSH EBP
  0073fcc1  8B EC          MOV EBP,ESP
  0073fcc3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fcc0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1F8 [dec 504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fcc0 FUN_0073fcc0 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0073fcc0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x1f8);
}
```

### decoy: 0x0073fca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fca0: 16-byte aligned: YES
  0073fca0  55             PUSH EBP
  0073fca1  8B EC          MOV EBP,ESP
  0073fca3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fca0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x18C [dec 396]  x1  via ECX
```

## Card B11a-2-34

GOG function: CClientExoApp::GetSWParty  gog 0x0040d070  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0073fbb0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.67 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fb90:STR0/CALLEE44/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d040 CClientExoApp::SetApplicationIdsMatch -> 0x0073fb60  gate_class=AUTO
- GOG-order neighbour after:  0x0040d090 CClientExoApp::GetHitCheckMaterialMask -> 0x0073fbb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (45 of 183): the card's A-CALLEE credit may come from these, not from callees
  0x00427c70 CSWCCreature::CreateUsableItemEntry -> Steam 0x0077bcf0  in candidate callers: no
  0x004ddc10 CGuiInGame::ShowSoloModeQuery -> Steam 0x007cd510  in candidate callers: no
  0x00563290 CSWGuiInGameMap::OnPanelAdded -> Steam 0x00894320  in candidate callers: no
  0x00564c30 CSWGuiMapHider::Draw -> Steam 0x00895cc0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x270} | Steam {0x4*,0xa4}
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
prologue for 0x0073fbb0: 16-byte aligned: YES
  0073fbb0  55             PUSH EBP
  0073fbb1  8B EC          MOV EBP,ESP
  0073fbb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fbb0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0xA4 [dec 164]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fbb0 FUN_0073fbb0 size=31 ret=0 callers=3 callees=0

undefined4 __fastcall FUN_0073fbb0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0xa4);
}
```

### decoy: 0x0073fb90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fb90: 16-byte aligned: YES
  0073fb90  55             PUSH EBP
  0073fb91  8B EC          MOV EBP,ESP
  0073fb93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fb90
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x270 [dec 624]  x1  via ECX
```

## Card B11a-2-35

GOG function: CClientExoApp::UnloadModule  gog 0x0040c710  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f270 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::CreateModule | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f250:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c6f0 CClientExoApp::DisplayMainMenu -> 0x0073f230  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c730 CClientExoApp::CreateModule -> 0x0073f270  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431a30 CClientExoAppInternal::UnloadModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: no
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
prologue for 0x0073f270: 16-byte aligned: YES
  0073f270  55             PUSH EBP
  0073f271  8B EC          MOV EBP,ESP
  0073f273  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f270
string literals (0):
direct callees (1):
  0x00785380  CClientExoAppInternal::CreateModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f270 FUN_0073f270 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00785380 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f270(undefined4 param_1)

{
  FUN_00785380(param_1);
  return;
}
```

### decoy: 0x0073f250 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f250: 16-byte aligned: YES
  0073f250  55             PUSH EBP
  0073f251  8B EC          MOV EBP,ESP
  0073f253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f250
string literals (0):
direct callees (1):
  0x00785410  CClientExoAppInternal::UnloadModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-36

GOG function: CClientExoApp::SendLoadGameRequest  gog 0x0040cbe0  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073f6f0 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::SendSaveGameRequest | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f720:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x0040cbb0 CClientExoApp::SendSaveGameRequest -> 0x0073f6f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cc10 CClientExoApp::GetInGameGui -> 0x0073f750  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431c80 CClientExoAppInternal::SendLoadGameRequest -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x004e1650 CGuiInGame::DoQuickLoad -> Steam 0x007d0f50  in candidate callers: no
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: no
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
prologue for 0x0073f6f0: 16-byte aligned: YES
  0073f6f0  55             PUSH EBP
  0073f6f1  8B EC          MOV EBP,ESP
  0073f6f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f6f0
string literals (0):
direct callees (1):
  0x00785620  CClientExoAppInternal::SendSaveGameRequest
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f6f0 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=3 callees=1
THUNK -> 0x00785620 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
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

### decoy: 0x0073f720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f720: 16-byte aligned: YES
  0073f720  55             PUSH EBP
  0073f721  8B EC          MOV EBP,ESP
  0073f723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f720
string literals (0):
direct callees (1):
  0x00785660  CClientExoAppInternal::SendLoadGameRequest
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-37

GOG function: CClientExoApp::GetDoorByGameObjectID  gog 0x0040cab0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f610 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetAreaOfEffectByGameObjectID | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f5f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ca90 CClientExoApp::GetPlaceableByGameObjectID -> 0x0073f5d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cad0 CClientExoApp::GetAreaOfEffectByGameObjectID -> 0x0073f610  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00438600 CClientExoAppInternal::GetDoorByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073f610: 16-byte aligned: YES
  0073f610  55             PUSH EBP
  0073f611  8B EC          MOV EBP,ESP
  0073f613  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f610
string literals (0):
direct callees (1):
  0x0078C090  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f610 FUN_0073f610 size=28 ret=4 callers=2 callees=1
THUNK -> 0x0078C090 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f610(undefined4 param_1)

{
  FUN_0078c090(param_1);
  return;
}
```

### decoy: 0x0073f5f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f5f0: 16-byte aligned: YES
  0073f5f0  55             PUSH EBP
  0073f5f1  8B EC          MOV EBP,ESP
  0073f5f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f5f0
string literals (0):
direct callees (1):
  0x0078C020  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-38

GOG function: CClientExoApp::GetDoorByGameObjectID  gog 0x0040cab0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f5d0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetPlaceableByGameObjectID | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f5b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ca90 CClientExoApp::GetPlaceableByGameObjectID -> 0x0073f5d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cad0 CClientExoApp::GetAreaOfEffectByGameObjectID -> 0x0073f610  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00438600 CClientExoAppInternal::GetDoorByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073f5d0: 16-byte aligned: YES
  0073f5d0  55             PUSH EBP
  0073f5d1  8B EC          MOV EBP,ESP
  0073f5d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f5d0
string literals (0):
direct callees (1):
  0x0078BFB0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f5d0 FUN_0073f5d0 size=28 ret=4 callers=7 callees=1
THUNK -> 0x0078BFB0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f5d0(undefined4 param_1)

{
  FUN_0078bfb0(param_1);
  return;
}
```

### decoy: 0x0073f5b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f5b0: 16-byte aligned: YES
  0073f5b0  55             PUSH EBP
  0073f5b1  8B EC          MOV EBP,ESP
  0073f5b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f5b0
string literals (0):
direct callees (1):
  0x0078BF40  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-39

GOG function: CClientExoApp::GetApplicationId  gog 0x0040d1e0  callconv=(blank)  param_size_bytes=0  gog_size=27  gog_retn=0
Candidate Steam address: 0x0073fd70 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fd00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d1c0 CClientExoApp::RestartMoviesIfNecessary -> 0x0073fd00  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d200 CClientExoApp::AddObjectModelDirectlyToIgnoreList -> 0x0073fd20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x18} | Steam {0x4}
imm sets (* = in both, 0 common): GOG {0x8} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0073fd70: 16-byte aligned: YES
  0073fd70  55             PUSH EBP
  0073fd71  8B EC          MOV EBP,ESP
  0073fd73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fd70
string literals (0):
direct callees (1):
  0x0078E7E0  CClientExoAppInternal::StartMenuMusic
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fd70 FUN_0073fd70 size=22 ret=0 callers=2 callees=1
THUNK -> 0x0078E7E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fd70(undefined4 param_1)

{
  FUN_0078e7e0(param_1);
  return;
}
```

### decoy: 0x0073fd00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fd00: 16-byte aligned: YES
  0073fd00  55             PUSH EBP
  0073fd01  8B EC          MOV EBP,ESP
  0073fd03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fd00
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x18 [dec 24]  x1  via EAX
```

## Card B11a-2-40

GOG function: CClientExoApp::TurnOffKeyboardModal  gog 0x0040d340  callconv=__thiscall  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fe60 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d320 CClientExoApp::SetInputActive -> 0x0073fe40  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d360 CClientExoApp::IsPlayerCreature -> 0x0073fe80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444410 CClientExoAppInternal::TurnOffKeyboardModal -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0073fe60: 16-byte aligned: YES
  0073fe60  55             PUSH EBP
  0073fe61  8B EC          MOV EBP,ESP
  0073fe63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fe60
string literals (0):
direct callees (1):
  0x00797DF0  CClientExoAppInternal::TurnOffKeyboardModal
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fe60 FUN_0073fe60 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00797DF0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fe60(undefined4 param_1)

{
  FUN_00797df0(param_1);
  return;
}
```

### decoy: 0x0073fe40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fe40: 16-byte aligned: YES
  0073fe40  55             PUSH EBP
  0073fe41  8B EC          MOV EBP,ESP
  0073fe43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fe40
string literals (0):
direct callees (1):
  0x00797D90  CClientExoAppInternal::SetInputActive
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-2-41

GOG function: CClientExoAppInternal::ShutDownMoviePlayer  gog 0x004381a0  callconv=(blank)  param_size_bytes=4  gog_size=129  gog_retn=4
Candidate Steam address: 0x0078bd20 (size 93, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bd80:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438080 CServerInfo::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00438230 CClientExoAppInternal::CleanUpModuleServices -> 0x0078c170  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x28}
imm sets (* = in both, 1 common): GOG {0x4*,0xc,0x14,0x30} | Steam {0x4*,0x10,0x1c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bd20: 16-byte aligned: YES
  0078bd20  55             PUSH EBP
  0078bd21  8B EC          MOV EBP,ESP
  0078bd23  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078bd20
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078bd20 FUN_0078bd20 size=93 ret=4 callers=1 callees=3

int __thiscall FUN_0078bd20(undefined4 param_1,int param_2)

{
  undefined4 local_c;
  int local_8;
  
  local_c = FUN_007a1720(param_1);
  local_8 = FUN_00521060();
  while( true ) {
    if (local_8 == 0) {
      return 0;
    }
    if (*(int *)(local_8 + 0x28) == param_2) break;
    local_8 = FUN_0058c370(&local_c);
  }
  return local_8;
}
```

### decoy: 0x0078bd80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bd80: 16-byte aligned: YES
  0078bd80  55             PUSH EBP
  0078bd81  8B EC          MOV EBP,ESP
  0078bd83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bd80
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

## Card B11a-2-42

GOG function: CClientExoApp::RestartMoviesIfNecessary  gog 0x0040d1c0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fd00 (size 27, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fce0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d1a0 CClientExoApp::GetDisableMovies -> 0x0073fcc0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d1e0 CClientExoApp::GetApplicationId -> 0x0073fd70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043a820 CClientExoAppInternal::RestartMoviesIfNecessary -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {0x18}
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
prologue for 0x0073fd00: 16-byte aligned: YES
  0073fd00  55             PUSH EBP
  0073fd01  8B EC          MOV EBP,ESP
  0073fd03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fd00
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fd00 FUN_0073fd00 size=27 ret=0 callers=0 callees=0

undefined4 FUN_0073fd00(void)

{
  return *(undefined4 *)(DAT_00a1b4a4 + 0x18);
}
```

### decoy: 0x0073fce0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fce0: 16-byte aligned: YES
  0073fce0  55             PUSH EBP
  0073fce1  8B EC          MOV EBP,ESP
  0073fce3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fce0
string literals (0):
direct callees (1):
  0x0078E250  CClientExoAppInternal::RestartMoviesIfNecessary
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-43

GOG function: CClientExoApp::GetPlayerCharacterSet  gog 0x0040d4e0  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00740000 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetPlayerCharacterName | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00740020:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d4c0 CClientExoApp::GetPlayerCharacterName -> 0x00740000  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d500 CClientExoApp::GetAreaNotReadyToDisplay -> 0x00740020  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x290*} | Steam {0x4*,0x290*}
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
prologue for 0x00740000: 16-byte aligned: YES
  00740000  55             PUSH EBP
  00740001  8B EC          MOV EBP,ESP
  00740003  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740000
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x290 [dec 656]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740000 FUN_00740000 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_00740000(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x290);
}
```

### decoy: 0x00740020 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740020: 16-byte aligned: YES
  00740020  55             PUSH EBP
  00740021  8B EC          MOV EBP,ESP
  00740023  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740020
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x288 [dec 648]  x1  via ECX
```

## Card B11a-2-44

GOG function: CClientExoApp::UnloadModule  gog 0x0040c710  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f1a0 (size 88, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::ResetCharacterFile | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f100:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c6f0 CClientExoApp::DisplayMainMenu -> 0x0073f230  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c730 CClientExoApp::CreateModule -> 0x0073f270  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431a30 CClientExoAppInternal::UnloadModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {0x1b4,0x1b8,0x1bc}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0073f1a0: 16-byte aligned: YES
  0073f1a0  55             PUSH EBP
  0073f1a1  8B EC          MOV EBP,ESP
  0073f1a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f1a0
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x1B4 [dec 436]  x3  via EAX,ECX
  +0x1B8 [dec 440]  x1  via EDX
  +0x1BC [dec 444]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f1a0 FUN_0073f1a0 size=88 ret=0 callers=1 callees=1

void __fastcall FUN_0073f1a0(int param_1)

{
  if (*(int *)(param_1 + 0x1b4) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x1b4));
    *(undefined4 *)(param_1 + 0x1b4) = 0;
  }
  *(undefined4 *)(param_1 + 0x1b8) = 0;
  *(undefined4 *)(param_1 + 0x1bc) = 0;
  return;
}
```

### decoy: 0x0073f100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f100: 16-byte aligned: YES
  0073f100  55             PUSH EBP
  0073f101  8B EC          MOV EBP,ESP
  0073f103  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0073f100
string literals (0):
direct callees (4):
  0x00733A60  CExoString::operator==_2
  0x00733570  CExoString::CExoString_char
  0x0078F160  CClientExoAppInternal::AddFeedbackMessage
  0x00733780  CExoString::~CExoString
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

## Card B11a-2-45

GOG function: CClientExoApp::BarkString  gog 0x0040d720  callconv=__thiscall  param_size_bytes=16  gog_size=40  gog_retn=16
Candidate Steam address: 0x007404d0 (size 36, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740260:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d700 CClientExoApp::ResetDriveAcceleration -> 0x00740240  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d750 CClientExoApp::IsSoundPlayingInDialog -> 0x00740290  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448240 CClientExoAppInternal::BarkString -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00783310 Global::ExecuteCommandBarkString -> Steam 0x0066dd30  in candidate callers: no
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
prologue for 0x007404d0: 16-byte aligned: YES
  007404d0  55             PUSH EBP
  007404d1  8B EC          MOV EBP,ESP
  007404d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007404d0
string literals (0):
direct callees (1):
  0x0079CA90  CClientExoAppInternal::GetClientRangedAnimation
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007404d0 FUN_007404d0 size=36 ret=16 callers=1 callees=1
THUNK -> 0x0079CA90 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007404d0(undefined4 param_1,undefined4 param_2)

{
  FUN_0079ca90(param_1,param_2,0,0);
  return;
}
```

### decoy: 0x00740260 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740260: 16-byte aligned: YES
  00740260  55             PUSH EBP
  00740261  8B EC          MOV EBP,ESP
  00740263  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740260
string literals (0):
direct callees (1):
  0x0079BC40  CClientExoAppInternal::BarkString
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-46

GOG function: CClientExoApp::Uninitialize  gog 0x0040cf10  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fa30 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fa50:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cec0 CClientExoApp::CreatureAcquireItem -> 0x0073f9e0  gate_class=AUTO
- GOG-order neighbour after:  0x0040cf30 CClientExoApp::SetCaptureEvents -> 0x0073fa70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431ce0 CClientExoAppInternal::Uninitialize -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073fa30: 16-byte aligned: YES
  0073fa30  55             PUSH EBP
  0073fa31  8B EC          MOV EBP,ESP
  0073fa33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fa30
string literals (0):
direct callees (1):
  0x007856C0  CClientExoAppInternal::Uninitialize
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fa30 FUN_0073fa30 size=22 ret=0 callers=1 callees=1
THUNK -> 0x007856C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fa30(undefined4 param_1)

{
  FUN_007856c0(param_1);
  return;
}
```

### decoy: 0x0073fa50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fa50: 16-byte aligned: YES
  0073fa50  55             PUSH EBP
  0073fa51  8B EC          MOV EBP,ESP
  0073fa53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fa50
string literals (0):
direct callees (1):
  0x007858D0  CClientExoAppInternal::SetCaptureEvents
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-47

GOG function: CClientExoApp::GetLoadScreenVisible  gog 0x0040d480  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073ffa0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073ff80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d460 CClientExoApp::HandleInputEvent -> 0x0073ff80  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d4a0 CClientExoApp::SetPlayerCharacterName -> 0x0073ffc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444b60 CClientExoAppInternal::IndicateObject -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073ffa0: 16-byte aligned: YES
  0073ffa0  55             PUSH EBP
  0073ffa1  8B EC          MOV EBP,ESP
  0073ffa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073ffa0
string literals (0):
direct callees (1):
  0x00798540  CClientExoAppInternal::IndicateObject
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073ffa0 FUN_0073ffa0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00798540 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073ffa0(undefined4 param_1)

{
  FUN_00798540(param_1);
  return;
}
```

### decoy: 0x0073ff80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ff80: 16-byte aligned: YES
  0073ff80  55             PUSH EBP
  0073ff81  8B EC          MOV EBP,ESP
  0073ff83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ff80
string literals (0):
direct callees (1):
  0x007B12C0  CClientExoAppInternal::HandleInputEvent
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-2-48

GOG function: CClientExoApp::StartMenuMusic  gog 0x0040d220  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073fd40 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073ff20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x0040d200 CClientExoApp::AddObjectModelDirectlyToIgnoreList -> 0x0073fd20  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d250 CClientExoApp::StopMenuMusic -> 0x0073fd90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043abf0 CClientExoAppInternal::StartMenuMusic -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073fd40: 16-byte aligned: YES
  0073fd40  55             PUSH EBP
  0073fd41  8B EC          MOV EBP,ESP
  0073fd43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fd40
string literals (0):
direct callees (1):
  0x0078E620  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fd40 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=2 callees=1
THUNK -> 0x0078E620 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_0078e620(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x0073ff20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ff20: 16-byte aligned: YES
  0073ff20  55             PUSH EBP
  0073ff21  8B EC          MOV EBP,ESP
  0073ff23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ff20
string literals (0):
direct callees (1):
  0x00798190  CClientExoAppInternal::ShowLoadScreen
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-49

GOG function: CClientExoApp::StopLoadMusic  gog 0x0040d270  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fd70 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fd90:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d250 CClientExoApp::StopMenuMusic -> 0x0073fd90  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d290 CClientExoApp::AquireLoadMusic -> 0x0073fdd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043b020 CClientExoAppInternal::StopLoadMusic -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00454600 CSWCArea::StartSounds -> Steam 0x007a7c00  in candidate callers: yes
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
prologue for 0x0073fd70: 16-byte aligned: YES
  0073fd70  55             PUSH EBP
  0073fd71  8B EC          MOV EBP,ESP
  0073fd73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fd70
string literals (0):
direct callees (1):
  0x0078E7E0  CClientExoAppInternal::StartMenuMusic
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fd70 FUN_0073fd70 size=22 ret=0 callers=2 callees=1
THUNK -> 0x0078E7E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fd70(undefined4 param_1)

{
  FUN_0078e7e0(param_1);
  return;
}
```

### decoy: 0x0073fd90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fd90: 16-byte aligned: YES
  0073fd90  55             PUSH EBP
  0073fd91  8B EC          MOV EBP,ESP
  0073fd93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fd90
string literals (0):
direct callees (1):
  0x0078EA50  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-50

GOG function: CClientExoApp::StopServices  gog 0x0040d000  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fb20 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fb00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cfe0 CClientExoApp::SetEventDescriptions -> 0x0073fb00  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d020 CClientExoApp::ShutDownToMainMenu -> 0x0073fb40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00437190 CClientExoAppInternal::StopServices -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073fb20: 16-byte aligned: YES
  0073fb20  55             PUSH EBP
  0073fb21  8B EC          MOV EBP,ESP
  0073fb23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fb20
string literals (0):
direct callees (1):
  0x0078AC40  CClientExoAppInternal::StopServices
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fb20 FUN_0073fb20 size=22 ret=0 callers=1 callees=1
THUNK -> 0x0078AC40 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fb20(undefined4 param_1)

{
  FUN_0078ac40(param_1);
  return;
}
```

### decoy: 0x0073fb00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fb00: 16-byte aligned: YES
  0073fb00  55             PUSH EBP
  0073fb01  8B EC          MOV EBP,ESP
  0073fb03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fb00
string literals (0):
direct callees (1):
  0x00785DF0  CClientExoAppInternal::SetEventDescriptions
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-51

GOG function: CClientExoApp::GetMGOArray  gog 0x0040d550  callconv=(blank)  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x00740070 (size 33, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 32 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 32 (callers: ExecuteCommandSWMG_GetLastEvent 0x6f6320, ExecuteCommandSWMG_GetLastEventModelName 0x6f6420, ExecuteCommandSWMG_GetObjectByName 0x6f6540)
- best decoy (neighbour/other candidate) scores: 0x007400a0:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d520 CClientExoApp::SetAreaNotReadyToDisplay -> 0x00740040  gate_class=AUTO
- GOG-order neighbour after:  0x0040d580 CClientExoApp::GetMiniGame -> 0x007400a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (32 of 46): the card's A-CALLEE credit may come from these, not from callees
  0x007f98e0 CSWVirtualMachineCommands::ExecuteCommandSWMG_GetLastEvent -> Steam 0x006f6320  in candidate callers: yes
  0x007f99e0 CSWVirtualMachineCommands::ExecuteCommandSWMG_GetLastEventModelName -> Steam 0x006f6420  in candidate callers: yes
  0x007f9b00 Global::ExecuteCommandSWMG_GetObjectByName -> Steam 0x006f6540  in candidate callers: yes
  0x007f9c00 Global::ExecuteCommandSWMG_PlayAnimation -> Steam 0x006f6640  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x00740070: 16-byte aligned: YES
  00740070  55             PUSH EBP
  00740071  8B EC          MOV EBP,ESP
  00740073  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740070
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740070 FUN_00740070 size=33 ret=0 callers=46 callees=0

undefined4 __fastcall FUN_00740070(int param_1)

{
  return **(undefined4 **)(param_1 + 4);
}
```

### decoy: 0x007400a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007400a0: 16-byte aligned: YES
  007400a0  55             PUSH EBP
  007400a1  8B EC          MOV EBP,ESP
  007400a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007400a0
string literals (0):
direct callees (2):
  0x0073F430  CClientExoApp::GetModule
  0x007FAE30  GetPlayerObject-like getter `[this+0x48]->+0x268`
struct offsets touched (top 0):
```

## Card B11a-2-52

GOG function: CClientExoApp::SetInputActive  gog 0x0040d320  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073fe40 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: SetTransitionPending 0x52e110)
- best decoy (neighbour/other candidate) scores: 0x0073ff80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d300 CClientExoApp::QueryExitProgram -> 0x0073fe20  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d340 CClientExoApp::TurnOffKeyboardModal -> 0x0073fe60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004443b0 CClientExoAppInternal::SetInputActive -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x007231f0 CSWSArea::SetTransitionPending -> Steam 0x0052e110  in candidate callers: yes
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
prologue for 0x0073fe40: 16-byte aligned: YES
  0073fe40  55             PUSH EBP
  0073fe41  8B EC          MOV EBP,ESP
  0073fe43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fe40
string literals (0):
direct callees (1):
  0x00797D90  CClientExoAppInternal::SetInputActive
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fe40 FUN_0073fe40 size=32 ret=8 callers=7 callees=1
THUNK -> 0x00797D90 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fe40(undefined4 param_1,undefined4 param_2)

{
  FUN_00797d90(param_1,param_2);
  return;
}
```

### decoy: 0x0073ff80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ff80: 16-byte aligned: YES
  0073ff80  55             PUSH EBP
  0073ff81  8B EC          MOV EBP,ESP
  0073ff83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ff80
string literals (0):
direct callees (1):
  0x007B12C0  CClientExoAppInternal::HandleInputEvent
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-2-53

GOG function: CClientExoApp::GetActivePauseState  gog 0x0040d2e0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fe00 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d2b0 CClientExoApp::SetStartMenuMusicOnMovieEnd -> 0x0073fdd0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d300 CClientExoApp::QueryExitProgram -> 0x0073fe20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00430a60 CClientExoAppInternal::GetActivePauseState -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073fe00: 16-byte aligned: YES
  0073fe00  55             PUSH EBP
  0073fe01  8B EC          MOV EBP,ESP
  0073fe03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fe00
string literals (0):
direct callees (1):
  0x00784440  CClientExoAppInternal::GetActivePauseState
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fe00 FUN_0073fe00 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00784440 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fe00(undefined4 param_1)

{
  FUN_00784440(param_1);
  return;
}
```

### decoy: 0x0073fe20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fe20: 16-byte aligned: YES
  0073fe20  55             PUSH EBP
  0073fe21  8B EC          MOV EBP,ESP
  0073fe23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fe20
string literals (0):
direct callees (1):
  0x00796BA0  CClientExoAppInternal::QueryExitProgram
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-54

GOG function: CClientExoApp::ResetDriveAcceleration  gog 0x0040d700  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740240 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: ExecuteCommandPlayMovie 0x68e720, ExecuteLoadModule 0x530170)
- best decoy (neighbour/other candidate) scores: 0x00740110:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d6d0 CClientExoApp::ReattachCreatureToScene -> 0x00740210  gate_class=AUTO
- GOG-order neighbour after:  0x0040d720 CClientExoApp::BarkString -> 0x007404d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004388e0 CClientExoAppInternal::ResetDriveAcceleration -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x007a3d70 Global::ExecuteCommandPlayMovie -> Steam 0x0068e720  in candidate callers: yes
  0x007b34f0 CServerExoAppInternal::ExecuteLoadModule -> Steam 0x00530170  in candidate callers: yes
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
prologue for 0x00740240: 16-byte aligned: YES
  00740240  55             PUSH EBP
  00740241  8B EC          MOV EBP,ESP
  00740243  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740240
string literals (0):
direct callees (1):
  0x0078C300  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740240 FUN_00740240 size=22 ret=0 callers=5 callees=1
THUNK -> 0x0078C300 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740240(undefined4 param_1)

{
  FUN_0078c300(param_1);
  return;
}
```

### decoy: 0x00740110 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740110: 16-byte aligned: YES
  00740110  55             PUSH EBP
  00740111  8B EC          MOV EBP,ESP
  00740113  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740110
string literals (0):
direct callees (3):
  0x00734010  CExoString::CStr
  0x00401650  (unnamed)
  0x00798BA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-2-55

GOG function: CClientExoApp::SetTexturePack  gog 0x0040d0d0  callconv=(blank)  param_size_bytes=1  gog_size=31  gog_retn=4
Candidate Steam address: 0x0073fbf0 (size 31, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fbd0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d0b0 CClientExoApp::SetVideoMode -> 0x0073fbd0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d0f0 CClientExoApp::GetSoundSetList -> 0x0073fc10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00439c70 CClientExoAppInternal::SetTexturePack -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073fbf0: 16-byte aligned: YES
  0073fbf0  55             PUSH EBP
  0073fbf1  8B EC          MOV EBP,ESP
  0073fbf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fbf0
string literals (0):
direct callees (1):
  0x0078D6A0  CClientExoApp::SetTexturePack (texpacks.2da row: Texture, Gui, DynMemRatio, Mem)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fbf0 FUN_0073fbf0 size=31 ret=4 callers=1 callees=1
THUNK -> 0x0078D6A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fbf0(undefined1 param_1)

{
  FUN_0078d6a0(param_1,1);
  return;
}
```

### decoy: 0x0073fbd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fbd0: 16-byte aligned: YES
  0073fbd0  55             PUSH EBP
  0073fbd1  8B EC          MOV EBP,ESP
  0073fbd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fbd0
string literals (0):
direct callees (1):
  0x0078DBF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-2-56

GOG function: CClientExoApp::AddObjectModelDirectlyToIgnoreList  gog 0x0040d200  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073fd20 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d1e0 CClientExoApp::GetApplicationId -> 0x0073fd70  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d220 CClientExoApp::StartMenuMusic -> 0x0073fd40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043a890 CClientExoAppInternal::AddObjectModelDirectlyToIgnoreList -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073fd20: 16-byte aligned: YES
  0073fd20  55             PUSH EBP
  0073fd21  8B EC          MOV EBP,ESP
  0073fd23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fd20
string literals (0):
direct callees (1):
  0x0078E2C0  CClientExoAppInternal::AddObjectModelDirectlyToIgnoreList
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fd20 FUN_0073fd20 size=32 ret=8 callers=1 callees=1
THUNK -> 0x0078E2C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fd20(undefined4 param_1,undefined4 param_2)

{
  FUN_0078e2c0(param_1,param_2);
  return;
}
```

### decoy: 0x0073fe40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fe40: 16-byte aligned: YES
  0073fe40  55             PUSH EBP
  0073fe41  8B EC          MOV EBP,ESP
  0073fe43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fe40
string literals (0):
direct callees (1):
  0x00797D90  CClientExoAppInternal::SetInputActive
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-2-57

GOG function: CClientExoAppInternal::SaveOptions  gog 0x0044bcc0  callconv=(blank)  param_size_bytes=0  gog_size=39  gog_retn=0
Candidate Steam address: 0x0079f430 (size 642, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::PlayerFlourishWeapons | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0079f6c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ba30 CClientExoAppInternal::PlayerFlourishWeapons -> 0x0079f430  gate_class=AUTO
- GOG-order neighbour after:  0x0044bcf0 CClientExoAppInternal::LoadOptions -> 0x0079f6f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: no
  0x00462cc0 CClientOptions::SaveOptions -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x34,0x44,0x94,0x9c,0xc5,0xe0,0x2e4,0x464,0x11ac}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x2,0x8*,0x10,0x20,0x40,0x3e8,0x5dc,0x27ad,0x27ae,0x7f000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0079f430: 16-byte aligned: YES
  0079f430  55             PUSH EBP
  0079f431  8B EC          MOV EBP,ESP
  0079f433  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0079f430
string literals (0):
direct callees (12):
  0x0078CEE0  (unnamed)
  0x007680A0  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x005453C0  CSWSObject::GetArea
  0x0058AE90  CSWSCreature::GetClientCreature
  0x007ED830  CSWCObject::GetAnimationBase
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007FE6C0  CSWCItem::PowerItem
  0x0057A560  CSWSCreature::SetCombatState
  0x00740AC0  (unnamed)
  0x007EFC40  CSWCObject::InsertFireForgetAnimation
struct offsets touched (top 10):
  +0x34 [dec 52]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x464 [dec 1124]  x2  via EAX
  +0x44 [dec 68]  x1  via EAX
  +0x94 [dec 148]  x1  via EAX
  +0x9C [dec 156]  x1  via EDX
  +0xC5 [dec 197]  x1  via EDX
  +0xE0 [dec 224]  x1  via EDX
  +0x2E4 [dec 740]  x1  via EAX
  +0x11AC [dec 4524]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0079f430 FUN_0079f430 size=642 ret=0 callers=1 callees=12

void FUN_0079f430(void)

{
  bool bVar1;
  short sVar2;
  undefined2 uVar3;
  int iVar4;
  int *piVar5;
  int iVar6;
  int *piVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  
  iVar4 = FUN_0078cee0();
  if (0 < *(int *)(iVar4 + 0x464)) {
    return;
  }
  uVar10 = 1;
  uVar8 = 0;
  FUN_0078cee0(0,1);
  FUN_007680a0(uVar8,uVar10);
  FUN_0078cee0();
  piVar5 = (int *)FUN_0077d800();
  if ((piVar5 != (int *)0x0) && (iVar6 = (**(code **)(*piVar5 + 0x94))(), iVar6 == 0)) {
    if (piVar5[0x46b = byte +0x11ac] == 0) {
      bVar1 = false;
    }
    else {
      sVar2 = (**(code **)(*piVar5 + 0x9c))(0);
      bVar1 = sVar2 < 1;
    }
    if (!bVar1) {
      iVar6 = FUN_005453c0();
      if (*(int *)(iVar6 + 0x2e4) != 0) {
        return;
      }
      FUN_0058ae90();
      iVar6 = FUN_007ed830();
      if (*(char *)(iVar6 + 0xc5) == '\x01') {
        return;
      }
      if (*(char *)(iVar6 + 0xc5) == '\b') {
        return;
      }
      uVar8 = 0x10;
      FUN_0078cee0(0x10);
      iVar6 = FUN_0076e610(uVar8);
      if (((iVar6 != 0x7f000000) && (piVar5 = (int *)FUN_0073f4d0(iVar6), piVar5 != (int *)0x0)) &&
         (iVar6 = (**(code **)(*piVar5 + 0x34))(), iVar6 != 0)) {
        uVar11 = 0;
        uVar9 = 1;
        uVar10 = 0;
        uVar8 = 1;
        (**(code **)(*piVar5 + 0x34))(1,0,1,0);
        FUN_007fe6c0(uVar8,uVar10,uVar9,uVar11);
      }
      uVar8 = 0x20;
      FUN_0078cee0(0x20);
      iVar6 = FUN_0076e610(uVar8);
      if (((iVar6 != 0x7f000000) && (piVar5 = (int *)FUN_0073f4d0(iVar6), piVar5 != (int *)0x0)) &&
         (iVar6 = (**(code **)(*piVar5 + 0x34))(), iVar6 != 0)) {
        uVar11 = 0;
        uVar9 = 1;
        uVar10 = 0;
        uVar8 = 1;
        (**(code **)(*piVar5 + 0x34))(1,0,1,0);
        FUN_007fe6c0(uVar8,uVar10,uVar9,uVar11);
      }
      FUN_0057a560(1,2,0);
    }
  }
  iVar6 = FUN_00740ac0();
  if (iVar6 == 0) {
    uVar12 = 0;
    uVar11 = 0;
    uVar9 = 0x3f800000;
/* ... 16 more lines truncated by --max-lines */
```

### decoy: 0x0079f6c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0079f6c0: 16-byte aligned: YES
  0079f6c0  55             PUSH EBP
  0079f6c1  8B EC          MOV EBP,ESP
  0079f6c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0079f6c0
string literals (0):
direct callees (2):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007B5F50  ClientOptions ini writer (Auto Level Up NPCs)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-58

GOG function: CClientExoApp::PlayLoadModuleMovies  gog 0x0040d5b0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007400f0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: StartNewModule 0x52fda0)
- best decoy (neighbour/other candidate) scores: 0x007400d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d580 CClientExoApp::GetMiniGame -> 0x007400a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d5d0 CClientExoApp::PlayMovie -> 0x00740110  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444cc0 CClientExoAppInternal::PlayLoadModuleMovies -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007b3120 CServerExoAppInternal::StartNewModule -> Steam 0x0052fda0  in candidate callers: yes
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
prologue for 0x007400f0: 16-byte aligned: YES
  007400f0  55             PUSH EBP
  007400f1  8B EC          MOV EBP,ESP
  007400f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007400f0
string literals (0):
direct callees (1):
  0x007986A0  CClientExoAppInternal::PlayLoadModuleMovies
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007400f0 FUN_007400f0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x007986A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007400f0(undefined4 param_1)

{
  FUN_007986a0(param_1);
  return;
}
```

### decoy: 0x007400d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007400d0: 16-byte aligned: YES
  007400d0  55             PUSH EBP
  007400d1  8B EC          MOV EBP,ESP
  007400d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007400d0
string literals (0):
direct callees (1):
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-59

GOG function: CClientExoApp::GetPlayerCharacterName  gog 0x0040d4c0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740000 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073ffe0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d4a0 CClientExoApp::SetPlayerCharacterName -> 0x0073ffc0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d4e0 CClientExoApp::GetPlayerCharacterSet -> 0x00740000  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444bd0 CClientExoAppInternal::GetPlayerCharacterName -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x290}
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
prologue for 0x00740000: 16-byte aligned: YES
  00740000  55             PUSH EBP
  00740001  8B EC          MOV EBP,ESP
  00740003  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740000
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x290 [dec 656]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740000 FUN_00740000 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_00740000(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x290);
}
```

### decoy: 0x0073ffe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ffe0: 16-byte aligned: YES
  0073ffe0  55             PUSH EBP
  0073ffe1  8B EC          MOV EBP,ESP
  0073ffe3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ffe0
string literals (0):
direct callees (1):
  0x007985B0  CClientExoAppInternal::GetPlayerCharacterName
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-2-60

GOG function: CClientExoApp::StartServices  gog 0x0040cfb0  callconv=(blank)  param_size_bytes=20  gog_size=44  gog_retn=20
Candidate Steam address: 0x0073fad0 (size 44, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fab0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cf90 CClientExoApp::SetDebugMode -> 0x0073fa70  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cfe0 CClientExoApp::SetEventDescriptions -> 0x0073fb00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004353b0 CClientExoAppInternal::StartServices -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073fad0: 16-byte aligned: YES
  0073fad0  55             PUSH EBP
  0073fad1  8B EC          MOV EBP,ESP
  0073fad3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fad0
string literals (0):
direct callees (1):
  0x00788D90  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fad0 FUN_0073fad0 size=44 ret=20 callers=1 callees=1
THUNK -> 0x00788D90 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fad0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5)

{
  FUN_00788d90(param_1,param_2,param_3,param_4,param_5);
  return;
}
```

### decoy: 0x0073fab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fab0: 16-byte aligned: YES
  0073fab0  55             PUSH EBP
  0073fab1  8B EC          MOV EBP,ESP
  0073fab3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fab0
string literals (0):
direct callees (1):
  0x0078D680  CClientExoAppInternal::SetDebugMode
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```
