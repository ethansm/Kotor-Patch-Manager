# Review cards B11a chunk 1

Answer each card with one TSV row in `steam_db_batches/B11a/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B11a-1-01

GOG function: CClientExoApp::HandleMessage  gog 0x0040ce10  callconv=__thiscall  param_size_bytes=16  gog_size=40  gog_retn=16
Candidate Steam address: 0x0073f930 (size 40, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f910:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: autolabel:DoHillClimbing
- GOG-order neighbour before: 0x0040cdf0 CClientExoApp::SetPlayerCreature -> 0x0073f910  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ce40 CClientExoApp::ContinueMessageProcessing -> 0x0073f980  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0042ddc0 CClientExoAppInternal::HandleMessage -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073f930: 16-byte aligned: YES
  0073f930  55             PUSH EBP
  0073f931  8B EC          MOV EBP,ESP
  0073f933  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f930
string literals (0):
direct callees (1):
  0x00781840  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f930 Concurrency::details::SchedulerProxy::DoHillClimbing size=40 ret=16 callers=0 callees=1
THUNK -> 0x00781840 (Ghidra may omit __thiscall on thunks; ECX is passed through)

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
  
  uVar1 = FUN_00781840(param_1,param_2,param_3,param_4);
  return uVar1;
}
```

### decoy: 0x0073f910 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f910: 16-byte aligned: YES
  0073f910  55             PUSH EBP
  0073f911  8B EC          MOV EBP,ESP
  0073f913  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f910
string literals (0):
direct callees (1):
  0x00798660  CClientExoAppInternal::SetPlayerCreature
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-02

GOG function: CClientExoApp::GetTriggerByGameObjectID  gog 0x0040ca50  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f590 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f570:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ca30 CClientExoApp::GetAreaByGameObjectID -> 0x0073f570  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ca70 CClientExoApp::GetProjectileByGameObjectID -> 0x0073f5b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004384b0 CClientExoAppInternal::GetTriggerByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073f590: 16-byte aligned: YES
  0073f590  55             PUSH EBP
  0073f591  8B EC          MOV EBP,ESP
  0073f593  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f590
string literals (0):
direct callees (1):
  0x0078BED0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f590 FUN_0073f590 size=28 ret=4 callers=2 callees=1
THUNK -> 0x0078BED0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f590(undefined4 param_1)

{
  FUN_0078bed0(param_1);
  return;
}
```

### decoy: 0x0073f570 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f570: 16-byte aligned: YES
  0073f570  55             PUSH EBP
  0073f571  8B EC          MOV EBP,ESP
  0073f573  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f570
string literals (0):
direct callees (1):
  0x0078BE60  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-03

GOG function: CClientExoApp::GetFPS  gog 0x0040c470  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x0073f410 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f3f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c3d0 CClientExoApp::Constructor -> 0x0073ef30  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c490 CClientExoApp::Destructor -> 0x0073eff0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x34}
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
prologue for 0x0073f410: 16-byte aligned: YES
  0073f410  55             PUSH EBP
  0073f411  8B EC          MOV EBP,ESP
  0073f413  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f410
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x34 [dec 52]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f410 FUN_0073f410 size=28 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_0073f410(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x34);
}
```

### decoy: 0x0073f3f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f3f0: 16-byte aligned: YES
  0073f3f0  55             PUSH EBP
  0073f3f1  8B EC          MOV EBP,ESP
  0073f3f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f3f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x30 [dec 48]  x1  via ECX
```

## Card B11a-1-04

GOG function: CClientExoApp::AttachPlayerGameObjectToArea  gog 0x0040cd30  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f850 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetServerInfo | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandDestroyObject 0x670760)
- best decoy (neighbour/other candidate) scores: 0x0073f870:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cd10 CClientExoApp::GetServerInfo -> 0x0073f830  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cd50 CClientExoApp::InitializeInGameGUI -> 0x0073f870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00438750 CClientExoAppInternal::AttachPlayerGameObjectToArea -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00785d40 Global::ExecuteCommandDestroyObject -> Steam 0x00670760  in candidate callers: yes
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
prologue for 0x0073f850: 16-byte aligned: YES
  0073f850  55             PUSH EBP
  0073f851  8B EC          MOV EBP,ESP
  0073f853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f850
string literals (0):
direct callees (1):
  0x0078C170  CClientExoAppInternal::CleanUpModuleServices
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f850 FUN_0073f850 size=22 ret=0 callers=3 callees=1
THUNK -> 0x0078C170 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f850(undefined4 param_1)

{
  FUN_0078c170(param_1);
  return;
}
```

### decoy: 0x0073f870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f870: 16-byte aligned: YES
  0073f870  55             PUSH EBP
  0073f871  8B EC          MOV EBP,ESP
  0073f873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f870
string literals (0):
direct callees (1):
  0x0078C330  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-05

GOG function: CClientExoApp::IsClientInitialized  gog 0x0040ce80  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0073f9a0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::Initialize | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073f980:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ce60 CClientExoApp::Initialize -> 0x0073f9a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cea0 CClientExoApp::MainLoop -> 0x0073f9c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x204*} | Steam {0x4*,0x204*}
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
prologue for 0x0073f9a0: 16-byte aligned: YES
  0073f9a0  55             PUSH EBP
  0073f9a1  8B EC          MOV EBP,ESP
  0073f9a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f9a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x204 [dec 516]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f9a0 FUN_0073f9a0 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0073f9a0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x204);
}
```

### decoy: 0x0073f980 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f980: 16-byte aligned: YES
  0073f980  55             PUSH EBP
  0073f981  8B EC          MOV EBP,ESP
  0073f983  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f980
string literals (0):
direct callees (1):
  0x007819E0  CClientExoAppInternal::GetModuleCamera
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-06

GOG function: CClientExoApp::GetAreaByGameObjectID  gog 0x0040ca30  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f570 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: GetClientArea 0x52db90)
- best decoy (neighbour/other candidate) scores: 0x0073f550:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> 0x0073f550  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ca50 CClientExoApp::GetTriggerByGameObjectID -> 0x0073f590  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00438440 CClientExoAppInternal::GetAreaByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00722c70 CSWSArea::GetClientArea -> Steam 0x0052db90  in candidate callers: yes
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
prologue for 0x0073f570: 16-byte aligned: YES
  0073f570  55             PUSH EBP
  0073f571  8B EC          MOV EBP,ESP
  0073f573  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f570
string literals (0):
direct callees (1):
  0x0078BE60  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f570 FUN_0073f570 size=28 ret=4 callers=1 callees=1
THUNK -> 0x0078BE60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f570(undefined4 param_1)

{
  FUN_0078be60(param_1);
  return;
}
```

### decoy: 0x0073f550 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f550: 16-byte aligned: YES
  0073f550  55             PUSH EBP
  0073f551  8B EC          MOV EBP,ESP
  0073f553  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f550
string literals (0):
direct callees (1):
  0x0078BDF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-07

GOG function: CClientExoApp::SendLoadGameRequest  gog 0x0040cbe0  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073f720 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f750:STR0/CALLEE2/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cbb0 CClientExoApp::SendSaveGameRequest -> 0x0073f6f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cc10 CClientExoApp::GetInGameGui -> 0x0073f750  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431c80 CClientExoAppInternal::SendLoadGameRequest -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x004e1650 CGuiInGame::DoQuickLoad -> Steam 0x007d0f50  in candidate callers: yes
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
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
prologue for 0x0073f720: 16-byte aligned: YES
  0073f720  55             PUSH EBP
  0073f721  8B EC          MOV EBP,ESP
  0073f723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f720
string literals (0):
direct callees (1):
  0x00785660  CClientExoAppInternal::SendLoadGameRequest
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f720 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=2 callees=1
THUNK -> 0x00785660 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
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

### decoy: 0x0073f750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f750: 16-byte aligned: YES
  0073f750  55             PUSH EBP
  0073f751  8B EC          MOV EBP,ESP
  0073f753  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

## Card B11a-1-08

GOG function: CClientExoApp::MainLoop  gog 0x0040cea0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f9c0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: MainLoop 0x781be0)
- best decoy (neighbour/other candidate) scores: 0x0073fa30:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ce80 CClientExoApp::IsClientInitialized -> 0x0073f9a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cec0 CClientExoApp::CreatureAcquireItem -> 0x0073f9e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callees: yes
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
prologue for 0x0073f9c0: 16-byte aligned: YES
  0073f9c0  55             PUSH EBP
  0073f9c1  8B EC          MOV EBP,ESP
  0073f9c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f9c0
string literals (0):
direct callees (1):
  0x00781BE0  CClientExoAppInternal::MainLoop
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f9c0 FUN_0073f9c0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00781BE0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f9c0(undefined4 param_1)

{
  FUN_00781be0(param_1);
  return;
}
```

### decoy: 0x0073fa30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fa30: 16-byte aligned: YES
  0073fa30  55             PUSH EBP
  0073fa31  8B EC          MOV EBP,ESP
  0073fa33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fa30
string literals (0):
direct callees (1):
  0x007856C0  CClientExoAppInternal::Uninitialize
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-09

GOG function: CClientExoApp::GetInGameGui  gog 0x0040cc10  callconv=(blank)  param_size_bytes=0  gog_size=28  gog_retn=0
Candidate Steam address: 0x0073f750 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=ClientApp::GetGuiInGame | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 97 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 97 (callers: MainLoop 0x781be0, LoadArea 0x7a2fb0, UnloadArea 0x7a5fa0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073f810:STR0/CALLEE5/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cbe0 CClientExoApp::SendLoadGameRequest -> 0x0073f720  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cc30 CClientExoApp::GetPlayModulePanel -> 0x0073f770  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (97 of 361): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callers: yes
  0x00453160 CSWCArea::Update -> Steam 0x007a6760  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x40*} | Steam {0x4*,0x40*}
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
prologue for 0x0073f750: 16-byte aligned: YES
  0073f750  55             PUSH EBP
  0073f751  8B EC          MOV EBP,ESP
  0073f753  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f750 FUN_0073f750 size=28 ret=0 callers=361 callees=0

undefined4 __fastcall FUN_0073f750(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x40);
}
```

### decoy: 0x0073f810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f810: 16-byte aligned: YES
  0073f810  55             PUSH EBP
  0073f811  8B EC          MOV EBP,ESP
  0073f813  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f810
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x150 [dec 336]  x1  via ECX
```

## Card B11a-1-10

GOG function: CClientExoApp::StopRumblePattern  gog 0x0040dce0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740800 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::PlayRumblePattern | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740820:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dcc0 CClientExoApp::PlayRumblePattern -> 0x00740800  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dd00 CClientExoApp::TogglePartyFollow -> 0x00740840  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449ae0 CClientExoAppInternal::StopRumblePattern -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007a97c0 Global::ExecuteCommandStopRumblePattern -> Steam 0x00694170  in candidate callers: no
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
prologue for 0x00740800: 16-byte aligned: YES
  00740800  55             PUSH EBP
  00740801  8B EC          MOV EBP,ESP
  00740803  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740800
string literals (0):
direct callees (1):
  0x0079D3F0  CClientExoAppInternal::PlayRumblePattern
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740800 FUN_00740800 size=28 ret=4 callers=2 callees=1
THUNK -> 0x0079D3F0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740800(undefined4 param_1)

{
  FUN_0079d3f0(param_1);
  return;
}
```

### decoy: 0x00740820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740820: 16-byte aligned: YES
  00740820  55             PUSH EBP
  00740821  8B EC          MOV EBP,ESP
  00740823  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740820
string literals (0):
direct callees (1):
  0x0079D4E0  CClientExoAppInternal::StopRumblePattern
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-11

GOG function: CClientExoApp::BeginIntro  gog 0x0040c4f0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f490 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f050:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c490 CClientExoApp::Destructor -> 0x0073eff0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c510 CClientExoApp::EndLoadBar -> 0x0073f430  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0042d920 CClientExoAppInternal::BeginIntro -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f490: 16-byte aligned: YES
  0073f490  55             PUSH EBP
  0073f491  8B EC          MOV EBP,ESP
  0073f493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f490
string literals (0):
direct callees (1):
  0x00781810  CClientExoAppInternal::DisplayMainMenu
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f490 FUN_0073f490 size=22 ret=0 callers=19 callees=1
THUNK -> 0x00781810 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f490(undefined4 param_1)

{
  FUN_00781810(param_1);
  return;
}
```

### decoy: 0x0073f050 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f050: 16-byte aligned: YES
  0073f050  55             PUSH EBP
  0073f051  8B EC          MOV EBP,ESP
  0073f053  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f050
string literals (0):
direct callees (1):
  0x007813A0  CClientExoAppInternal::BeginIntro
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-12

GOG function: CClientExoApp::StopRumblePattern  gog 0x0040dce0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740880 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::SetCombatMode | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007408a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dcc0 CClientExoApp::PlayRumblePattern -> 0x00740800  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dd00 CClientExoApp::TogglePartyFollow -> 0x00740840  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449ae0 CClientExoAppInternal::StopRumblePattern -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007a97c0 Global::ExecuteCommandStopRumblePattern -> Steam 0x00694170  in candidate callers: no
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
prologue for 0x00740880: 16-byte aligned: YES
  00740880  55             PUSH EBP
  00740881  8B EC          MOV EBP,ESP
  00740883  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740880
string literals (0):
direct callees (1):
  0x0079D6E0  CClientExoAppInternal::SetCombatMode
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740880 FUN_00740880 size=28 ret=4 callers=3 callees=1
THUNK -> 0x0079D6E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740880(undefined4 param_1)

{
  FUN_0079d6e0(param_1);
  return;
}
```

### decoy: 0x007408a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007408a0: 16-byte aligned: YES
  007408a0  55             PUSH EBP
  007408a1  8B EC          MOV EBP,ESP
  007408a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007408a0
string literals (0):
direct callees (1):
  0x0079F430  CClientExoAppInternal::PlayerFlourishWeapons
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-13

GOG function: CClientExoApp::AddObjectToHitcheckIgnoreList  gog 0x0040cd90  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f8d0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f8b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cd70 CClientExoApp::ServerModeChange -> 0x0073f890  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cdb0 CClientExoApp::RemoveObjectFromHitcheckIgnoreList -> 0x0073f8b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004396d0 CClientExoAppInternal::AddObjectToHitcheckIgnoreList -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f8d0: 16-byte aligned: YES
  0073f8d0  55             PUSH EBP
  0073f8d1  8B EC          MOV EBP,ESP
  0073f8d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f8d0
string literals (0):
direct callees (1):
  0x0078D190  CClientExoAppInternal::RemoveObjectFromHitcheckIgnoreList
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f8d0 FUN_0073f8d0 size=28 ret=4 callers=4 callees=1
THUNK -> 0x0078D190 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f8d0(undefined4 param_1)

{
  FUN_0078d190(param_1);
  return;
}
```

### decoy: 0x0073f8b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f8b0: 16-byte aligned: YES
  0073f8b0  55             PUSH EBP
  0073f8b1  8B EC          MOV EBP,ESP
  0073f8b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f8b0
string literals (0):
direct callees (1):
  0x0078D100  CClientExoAppInternal::AddObjectToHitcheckIgnoreList
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-14

GOG function: CClientExoApp::GetObjectName  gog 0x0040c5a0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073f0e0 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f0b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c570 CClientExoApp::JournalUpdated -> 0x0073f0b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c5c0 CClientExoApp::AddFeedbackMessage -> 0x0073f100  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043b150 CClientExoAppInternal::GetObjectName -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f0e0: 16-byte aligned: YES
  0073f0e0  55             PUSH EBP
  0073f0e1  8B EC          MOV EBP,ESP
  0073f0e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f0e0
string literals (0):
direct callees (1):
  0x0078EBA0  CClientExoAppInternal::FadeAndStopStreamingSoundSource
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f0e0 FUN_0073f0e0 size=32 ret=8 callers=1 callees=1
THUNK -> 0x0078EBA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f0e0(undefined4 param_1,undefined4 param_2)

{
  FUN_0078eba0(param_1,param_2);
  return;
}
```

### decoy: 0x0073f0b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f0b0: 16-byte aligned: YES
  0073f0b0  55             PUSH EBP
  0073f0b1  8B EC          MOV EBP,ESP
  0073f0b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f0b0
string literals (0):
direct callees (1):
  0x00785330  CClientExoAppInternal::JournalUpdated
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-15

GOG function: CClientExoApp::GetCreatureByGameObjectID  gog 0x0040ca10  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f550 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 16 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 16 (callers: LoadArea 0x7a2fb0, UnloadArea 0x7a5fa0, AddCorpse 0x7aaec0)
- best decoy (neighbour/other candidate) scores: 0x0073f750:STR0/CALLEE3/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> 0x0073f530  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ca30 CClientExoApp::GetAreaByGameObjectID -> 0x0073f570  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004383d0 CClientExoAppInternal::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (16 of 73): the card's A-CALLEE credit may come from these, not from callees
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callers: yes
  0x004578c0 CSWCArea::AddCorpse -> Steam 0x007aaec0  in candidate callers: yes
  0x00457a40 CSWCArea::ParentCorpseExists -> Steam 0x007ab040  in candidate callers: yes
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
prologue for 0x0073f550: 16-byte aligned: YES
  0073f550  55             PUSH EBP
  0073f551  8B EC          MOV EBP,ESP
  0073f553  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f550
string literals (0):
direct callees (1):
  0x0078BDF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f550 FUN_0073f550 size=28 ret=4 callers=72 callees=1
THUNK -> 0x0078BDF0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f550(undefined4 param_1)

{
  FUN_0078bdf0(param_1);
  return;
}
```

### decoy: 0x0073f750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f750: 16-byte aligned: YES
  0073f750  55             PUSH EBP
  0073f751  8B EC          MOV EBP,ESP
  0073f753  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

## Card B11a-1-16

GOG function: CClientExoApp::CancelMovie  gog 0x0040e420  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740fc0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::GetCharacterChangeInProgress | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740fa0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e400 CClientExoApp::SetMouseMode -> 0x00740f40  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e440 CClientExoApp::HideMouse -> 0x00740f80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044cb00 CClientExoAppInternal::CancelMovie -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x4ac}
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
prologue for 0x00740fc0: 16-byte aligned: YES
  00740fc0  55             PUSH EBP
  00740fc1  8B EC          MOV EBP,ESP
  00740fc3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740fc0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x4AC [dec 1196]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740fc0 FUN_00740fc0 size=31 ret=0 callers=3 callees=0

undefined4 __fastcall FUN_00740fc0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x4ac);
}
```

### decoy: 0x00740fa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740fa0: 16-byte aligned: YES
  00740fa0  55             PUSH EBP
  00740fa1  8B EC          MOV EBP,ESP
  00740fa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740fa0
string literals (0):
direct callees (1):
  0x007B3930  CClientExoAppInternal::ShowMouse
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-17

GOG function: CClientExoApp::GetModuleCamera  gog 0x0040c950  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f490 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::BeginIntro | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: Update 0x7a6760, UpdateAreaTransitionDisplay 0x87c9f0, GetReticlePoint 0x843b80)
- best decoy (neighbour/other candidate) scores: 0x0073f750:STR0/CALLEE2/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c930 CClientExoApp::GetPlayerCreatureId -> 0x0073f470  gate_class=AUTO
- GOG-order neighbour after:  0x0040c970 CClientExoApp::GetGameObjectArray -> 0x0073f4b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0042dd90 CClientExoAppInternal::GetModuleCamera -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 19): the card's A-CALLEE credit may come from these, not from callees
  0x00453160 CSWCArea::Update -> Steam 0x007a6760  in candidate callers: yes
  0x005082d0 CSWCDoor::UpdateAreaTransitionDisplay -> Steam 0x0087c9f0  in candidate callers: yes
  0x00537400 CSWCPlaceable::GetReticlePoint -> Steam 0x00843b80  in candidate callers: yes
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
prologue for 0x0073f490: 16-byte aligned: YES
  0073f490  55             PUSH EBP
  0073f491  8B EC          MOV EBP,ESP
  0073f493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f490
string literals (0):
direct callees (1):
  0x00781810  CClientExoAppInternal::DisplayMainMenu
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f490 FUN_0073f490 size=22 ret=0 callers=19 callees=1
THUNK -> 0x00781810 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f490(undefined4 param_1)

{
  FUN_00781810(param_1);
  return;
}
```

### decoy: 0x0073f750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f750: 16-byte aligned: YES
  0073f750  55             PUSH EBP
  0073f751  8B EC          MOV EBP,ESP
  0073f753  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

## Card B11a-1-18

GOG function: CClientExoApp::GetAreaOfEffectByGameObjectID  gog 0x0040cad0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f610 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f5f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cab0 CClientExoApp::GetDoorByGameObjectID -> 0x0073f5f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040caf0 CClientExoApp::GetSoundObjectByGameObjectID -> 0x0073f630  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00438670 CClientExoAppInternal::GetAreaOfEffectByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
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

## Card B11a-1-19

GOG function: CClientExoApp::GetSWParty  gog 0x0040d070  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0073fc10 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetSoundSetList | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.67 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fc30:STR0/CALLEE0/SIG0.35/SRV0/VT0
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
disp sets (* = in both, 1 common): GOG {0x4*,0x270} | Steam {0x4*,0x1c4}
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
prologue for 0x0073fc10: 16-byte aligned: YES
  0073fc10  55             PUSH EBP
  0073fc11  8B EC          MOV EBP,ESP
  0073fc13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fc10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1C4 [dec 452]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fc10 FUN_0073fc10 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0073fc10(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x1c4);
}
```

### decoy: 0x0073fc30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fc30: 16-byte aligned: YES
  0073fc30  55             PUSH EBP
  0073fc31  8B EC          MOV EBP,ESP
  0073fc33  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fc30
string literals (0):
direct callees (1):
  0x00710840  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x1C8 [dec 456]  x2  via ECX,EDX
```

## Card B11a-1-20

GOG function: CClientExoAppInternal::ShutDownToMainMenu  gog 0x0043a6c0  callconv=(blank)  param_size_bytes=0  gog_size=339  gog_retn=0
Candidate Steam address: 0x0078e250 (size 109, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::RestartMoviesIfNecessary | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078e0f0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043a610 CClientExoAppInternal::SetCharacterFile -> 0x0078e040  gate_class=REVIEW
- GOG-order neighbour after:  0x0043a820 CClientExoAppInternal::RestartMoviesIfNecessary -> 0x0078e250  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00401c30 CAppManager::DestroyServer -> Steam ?  in candidate callees: n/a
  0x0042db10 CClientExoAppInternal::DisplayMainMenu -> Steam ?  in candidate callees: n/a
  0x00431a30 CClientExoAppInternal::UnloadModule -> Steam ?  in candidate callees: n/a
  0x0043abf0 CClientExoAppInternal::StartMenuMusic -> Steam ?  in candidate callees: n/a
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callees: no
  0x00469b40 CGameObjectArray::Delete_2 -> Steam ?  in candidate callees: n/a
  0x004f0520 CSWPlayerControlCamRelative::GetPlayerWalking -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0040d020 CClientExoApp::ShutDownToMainMenu -> Steam 0x0073fb40  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x14,0x40,0x48,0x108,0x18c,0x1ec,0x250,0x254} | Steam {0x130}
imm sets (* = in both, 0 common): GOG {0x2,0x2c,0x7f} | Steam {0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078e250: 16-byte aligned: YES
  0078e250  55             PUSH EBP
  0078e251  8B EC          MOV EBP,ESP
  0078e253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078e250
string literals (0):
direct callees (8):
  0x0093B600  (unnamed)
  0x007B38D0  CClientExoAppInternal::HideMouse
  0x0040BE50  Movie::PrepareWindowForMovie
  0x0093B630  (unnamed)
  0x00798820  CClientExoAppInternal::WaitForMovie
  0x0040BF40  Movie::RestoreAfterMovie (handler of WM_USER+10 0x40A and sync call)
  0x007B3930  CClientExoAppInternal::ShowMouse
  0x0093B5E0  (unnamed)
struct offsets touched (top 1):
  +0x130 [dec 304]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078e250 FUN_0078e250 size=109 ret=0 callers=1 callees=8

void __fastcall FUN_0078e250(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x130) != 0) {
    iVar1 = FUN_0093b600();
    if (iVar1 != 0) {
      FUN_007b38d0(8);
      FUN_0040be50();
      FUN_0093b630();
      FUN_00798820(1);
      FUN_0040bf40();
      FUN_007b3930(8);
      FUN_0093b5e0();
    }
  }
  return;
}
```

### decoy: 0x0078e0f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e0f0: 16-byte aligned: YES
  0078e0f0  55             PUSH EBP
  0078e0f1  8B EC          MOV EBP,ESP
  0078e0f3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078e0f0
string literals (0):
direct callees (9):
  0x00401C70  (unnamed)
  0x00726F80  (unnamed)
  0x0053DED0  (unnamed)
  0x007A5FA0  CSWCArea::UnloadArea
  0x00785410  CClientExoAppInternal::UnloadModule
  0x00781590  (unnamed)
  0x0070BC60  (unnamed)
  0x0078E620  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 9):
  +0x14 [dec 20]  x4  via EAX,EDX
  +0x18C [dec 396]  x2  via ECX,EDX
  +0x254 [dec 596]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via EDX
  +0x48 [dec 72]  x1  via EAX
  +0x108 [dec 264]  x1  via ECX
  +0x1EC [dec 492]  x1  via ECX
  +0x250 [dec 592]  x1  via EDX
```

## Card B11a-1-21

GOG function: CClientExoApp::GetSoundObjectByGameObjectID  gog 0x0040caf0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f630 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: UnloadArea 0x7a5fa0, UpdateSoundObjectsTimeOfDay 0x7a9780)
- best decoy (neighbour/other candidate) scores: 0x0073f750:STR0/CALLEE1/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cad0 CClientExoApp::GetAreaOfEffectByGameObjectID -> 0x0073f610  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cb10 CClientExoApp::SetInStartNewModule -> 0x0073f650  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004386e0 CClientExoAppInternal::GetSoundObjectByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callers: yes
  0x00456180 CSWCArea::UpdateSoundObjectsTimeOfDay -> Steam 0x007a9780  in candidate callers: yes
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
prologue for 0x0073f630: 16-byte aligned: YES
  0073f630  55             PUSH EBP
  0073f631  8B EC          MOV EBP,ESP
  0073f633  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f630
string literals (0):
direct callees (1):
  0x0078C100  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f630 FUN_0073f630 size=28 ret=4 callers=6 callees=1
THUNK -> 0x0078C100 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f630(undefined4 param_1)

{
  FUN_0078c100(param_1);
  return;
}
```

### decoy: 0x0073f750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f750: 16-byte aligned: YES
  0073f750  55             PUSH EBP
  0073f751  8B EC          MOV EBP,ESP
  0073f753  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

## Card B11a-1-22

GOG function: CClientExoApp::GetItemByGameObjectID  gog 0x0040c9f0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f530 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: PopulateItemListBox 0x8a7ba0, Destructor_2 0x6000e0)
- best decoy (neighbour/other candidate) scores: 0x0073f4b0:STR0/CALLEE1/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c9d0 CClientExoApp::GetPlayerByGameObjectID -> 0x0073f510  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> 0x0073f550  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00438360 CClientExoAppInternal::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 54): the card's A-CALLEE credit may come from these, not from callees
  0x0059baa0 CSWGuiInGameInventory::PopulateItemListBox -> Steam 0x008a7ba0  in candidate callers: yes
  0x00663910 CSWSItem::Destructor_2 -> Steam 0x006000e0  in candidate callers: yes
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
prologue for 0x0073f530: 16-byte aligned: YES
  0073f530  55             PUSH EBP
  0073f531  8B EC          MOV EBP,ESP
  0073f533  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f530
string literals (0):
direct callees (1):
  0x0078BD80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f530 FUN_0073f530 size=28 ret=4 callers=54 callees=1
THUNK -> 0x0078BD80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f530(undefined4 param_1)

{
  FUN_0078bd80(param_1);
  return;
}
```

### decoy: 0x0073f4b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f4b0: 16-byte aligned: YES
  0073f4b0  55             PUSH EBP
  0073f4b1  8B EC          MOV EBP,ESP
  0073f4b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f4b0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

## Card B11a-1-23

GOG function: CClientExoApp::GetDoorByGameObjectID  gog 0x0040cab0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f5f0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f5d0:STR0/CALLEE0/SIG1/SRV0/VT0
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
prologue for 0x0073f5f0: 16-byte aligned: YES
  0073f5f0  55             PUSH EBP
  0073f5f1  8B EC          MOV EBP,ESP
  0073f5f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f5f0
string literals (0):
direct callees (1):
  0x0078C020  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f5f0 FUN_0073f5f0 size=28 ret=4 callers=4 callees=1
THUNK -> 0x0078C020 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f5f0(undefined4 param_1)

{
  FUN_0078c020(param_1);
  return;
}
```

### decoy: 0x0073f5d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f5d0: 16-byte aligned: YES
  0073f5d0  55             PUSH EBP
  0073f5d1  8B EC          MOV EBP,ESP
  0073f5d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f5d0
string literals (0):
direct callees (1):
  0x0078BFB0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-24

GOG function: CClientExoApp::CancelMovie  gog 0x0040e420  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740f20 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::GetPazaakIsRunning | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740f00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e400 CClientExoApp::SetMouseMode -> 0x00740f40  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e440 CClientExoApp::HideMouse -> 0x00740f80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044cb00 CClientExoAppInternal::CancelMovie -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x70}
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
prologue for 0x00740f20: 16-byte aligned: YES
  00740f20  55             PUSH EBP
  00740f21  8B EC          MOV EBP,ESP
  00740f23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740f20
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x70 [dec 112]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740f20 FUN_00740f20 size=28 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_00740f20(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x70);
}
```

### decoy: 0x00740f00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740f00: 16-byte aligned: YES
  00740f00  55             PUSH EBP
  00740f01  8B EC          MOV EBP,ESP
  00740f03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740f00
string literals (0):
direct callees (1):
  0x007A0630  CClientExoAppInternal::BusyWaitMuteSound
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-25

GOG function: CClientExoApp::AddFeedbackMessage  gog 0x0040c5c0  callconv=(blank)  param_size_bytes=12  gog_size=159  gog_retn=12
Candidate Steam address: 0x0073f100 (size 159, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f0b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c5a0 CClientExoApp::GetObjectName -> 0x0073f0e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c660 CClientExoApp::ResetCharacterFile -> 0x0073f1a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0043b710 CClientExoAppInternal::AddFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00605b70 CExoString::operator==_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 5): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0073f100: 16-byte aligned: YES
  0073f100  55             PUSH EBP
  0073f101  8B EC          MOV EBP,ESP
  0073f103  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0073f100 FUN_0073f100 size=159 ret=12 callers=5 callees=4

void FUN_0073f100(undefined2 param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  int iVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954bd8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_00733a60(&DAT_00986925);
  if (iVar2 == 0) {
    FUN_0078f160(param_1,param_2,param_3,uVar1,extraout_var);
  }
  else {
    uVar3 = extraout_var;
    FUN_00733570(&DAT_00986925);
    local_8 = 0;
    FUN_0078f160(param_1,param_2,local_18,uVar1,uVar3);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0073f0b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f0b0: 16-byte aligned: YES
  0073f0b0  55             PUSH EBP
  0073f0b1  8B EC          MOV EBP,ESP
  0073f0b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f0b0
string literals (0):
direct callees (1):
  0x00785330  CClientExoAppInternal::JournalUpdated
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-26

GOG function: CClientExoApp::GetProjectileByGameObjectID  gog 0x0040ca70  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f5b0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f590:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ca50 CClientExoApp::GetTriggerByGameObjectID -> 0x0073f590  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ca90 CClientExoApp::GetPlaceableByGameObjectID -> 0x0073f5d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00438520 CClientExoAppInternal::GetProjectileByGameObjectID -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f5b0: 16-byte aligned: YES
  0073f5b0  55             PUSH EBP
  0073f5b1  8B EC          MOV EBP,ESP
  0073f5b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f5b0
string literals (0):
direct callees (1):
  0x0078BF40  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f5b0 FUN_0073f5b0 size=28 ret=4 callers=1 callees=1
THUNK -> 0x0078BF40 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f5b0(undefined4 param_1)

{
  FUN_0078bf40(param_1);
  return;
}
```

### decoy: 0x0073f590 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f590: 16-byte aligned: YES
  0073f590  55             PUSH EBP
  0073f591  8B EC          MOV EBP,ESP
  0073f593  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f590
string literals (0):
direct callees (1):
  0x0078BED0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-27

GOG function: CClientExoApp::JournalUpdated  gog 0x0040c570  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073f0b0 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f100:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x0040c550 CClientExoApp::GetWorldJournal -> 0x0073f090  gate_class=AUTO
- GOG-order neighbour after:  0x0040c5a0 CClientExoApp::GetObjectName -> 0x0073f0e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431950 CClientExoAppInternal::JournalUpdated -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f0b0: 16-byte aligned: YES
  0073f0b0  55             PUSH EBP
  0073f0b1  8B EC          MOV EBP,ESP
  0073f0b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f0b0
string literals (0):
direct callees (1):
  0x00785330  CClientExoAppInternal::JournalUpdated
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f0b0 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=1 callees=1
THUNK -> 0x00785330 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00785330(param_1,param_2,param_3);
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

## Card B11a-1-28

GOG function: CClientExoApp::Initialize  gog 0x0040ce60  callconv=__thiscall  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f9a0 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f980:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ce40 CClientExoApp::ContinueMessageProcessing -> 0x0073f980  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ce80 CClientExoApp::IsClientInitialized -> 0x0073f9a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0042df60 CClientExoAppInternal::Initialize -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x204}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0073f9a0: 16-byte aligned: YES
  0073f9a0  55             PUSH EBP
  0073f9a1  8B EC          MOV EBP,ESP
  0073f9a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f9a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x204 [dec 516]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f9a0 FUN_0073f9a0 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0073f9a0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x204);
}
```

### decoy: 0x0073f980 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f980: 16-byte aligned: YES
  0073f980  55             PUSH EBP
  0073f981  8B EC          MOV EBP,ESP
  0073f983  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f980
string literals (0):
direct callees (1):
  0x007819E0  CClientExoAppInternal::GetModuleCamera
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-29

GOG function: CClientExoApp::EndLoadBar  gog 0x0040c510  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f430 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetModule | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: MainLoop 0x781be0, MainLoop 0x533830, StallEventSaveGame 0x534600)
- best decoy (neighbour/other candidate) scores: 0x0073f2f0:STR0/CALLEE1/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c4f0 CClientExoApp::BeginIntro -> 0x0073f490  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c530 CClientExoApp::GetQuestJournal -> 0x0073f070  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x007b6bb0 CServerExoAppInternal::MainLoop -> Steam 0x00533830  in candidate callers: yes
  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> Steam 0x00534600  in candidate callers: yes
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
prologue for 0x0073f430: 16-byte aligned: YES
  0073f430  55             PUSH EBP
  0073f431  8B EC          MOV EBP,ESP
  0073f433  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f430
string literals (0):
direct callees (1):
  0x00726F80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f430 FUN_0073f430 size=22 ret=0 callers=112 callees=1
THUNK -> 0x00726F80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f430(undefined4 param_1)

{
  FUN_00726f80(param_1);
  return;
}
```

### decoy: 0x0073f2f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f2f0: 16-byte aligned: YES
  0073f2f0  55             PUSH EBP
  0073f2f1  8B EC          MOV EBP,ESP
  0073f2f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f2f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x90 [dec 144]  x1  via ECX
```

## Card B11a-1-30

GOG function: CClientExoApp::DisplayMainMenu  gog 0x0040c6f0  callconv=__thiscall  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f230 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f510:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c6c0 CClientExoApp::SetCharacterFile -> 0x0073f200  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c710 CClientExoApp::UnloadModule -> 0x0073f250  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0042db10 CClientExoAppInternal::DisplayMainMenu -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073f230: 16-byte aligned: YES
  0073f230  55             PUSH EBP
  0073f231  8B EC          MOV EBP,ESP
  0073f233  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f230
string literals (0):
direct callees (1):
  0x00781590  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f230 FUN_0073f230 size=28 ret=4 callers=3 callees=1
THUNK -> 0x00781590 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f230(undefined4 param_1)

{
  FUN_00781590(param_1);
  return;
}
```

### decoy: 0x0073f510 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f510: 16-byte aligned: YES
  0073f510  55             PUSH EBP
  0073f511  8B EC          MOV EBP,ESP
  0073f513  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f510
string literals (0):
direct callees (1):
  0x0078BD20  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-31

GOG function: CClientExoApp::GetServerInfo  gog 0x0040cd10  callconv=(blank)  param_size_bytes=0  gog_size=28  gog_retn=0
Candidate Steam address: 0x0073f830 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetClientOptions | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073f810:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ccf0 CClientExoApp::GetClientOptions -> 0x0072fb00  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cd30 CClientExoApp::AttachPlayerGameObjectToArea -> 0x0073f850  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
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
prologue for 0x0073f830: 16-byte aligned: YES
  0073f830  55             PUSH EBP
  0073f831  8B EC          MOV EBP,ESP
  0073f833  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f830
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f830 FUN_0073f830 size=28 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0073f830(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 8);
}
```

### decoy: 0x0073f810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f810: 16-byte aligned: YES
  0073f810  55             PUSH EBP
  0073f811  8B EC          MOV EBP,ESP
  0073f813  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f810
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x150 [dec 336]  x1  via ECX
```

## Card B11a-1-32

GOG function: CClientExoApp::GetPlayerList  gog 0x0040cc70  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x0073f7b0 (size 20, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f7d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cc50 CClientExoApp::GetClientAIMaster -> 0x0073f790  gate_class=AUTO
- GOG-order neighbour after:  0x0040cc90 CClientExoApp::GetPlayerByPlayerID -> 0x0073f7d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0073f7b0: 16-byte aligned: YES
  0073f7b0  55             PUSH EBP
  0073f7b1  8B EC          MOV EBP,ESP
  0073f7b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f7b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f7b0 FUN_0073f7b0 size=20 ret=0 callers=1 callees=0

int __fastcall FUN_0073f7b0(int param_1)

{
  return *(int *)(param_1 + 4) + 0x1c;
}
```

### decoy: 0x0073f7d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f7d0: 16-byte aligned: YES
  0073f7d0  55             PUSH EBP
  0073f7d1  8B EC          MOV EBP,ESP
  0073f7d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f7d0
string literals (0):
direct callees (1):
  0x0078BCC0  CClientExoAppInternal::ShutDownMoviePlayer
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-33

GOG function: CClientExoApp::GetPauseState  gog 0x0040cb40  callconv=(blank)  param_size_bytes=1  gog_size=29  gog_retn=4
Candidate Steam address: 0x0073f680 (size 29, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f630:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cb10 CClientExoApp::SetInStartNewModule -> 0x0073f650  gate_class=AUTO
- GOG-order neighbour after:  0x0040cb60 CClientExoApp::SetPauseState -> 0x0073f6a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00430a40 CClientExoAppInternal::GetPauseState -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f680: 16-byte aligned: YES
  0073f680  55             PUSH EBP
  0073f681  8B EC          MOV EBP,ESP
  0073f683  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f680
string literals (0):
direct callees (1):
  0x00784420  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f680 FUN_0073f680 size=29 ret=4 callers=1 callees=1
THUNK -> 0x00784420 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f680(undefined1 param_1)

{
  FUN_00784420(param_1);
  return;
}
```

### decoy: 0x0073f630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f630: 16-byte aligned: YES
  0073f630  55             PUSH EBP
  0073f631  8B EC          MOV EBP,ESP
  0073f633  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f630
string literals (0):
direct callees (1):
  0x0078C100  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-34

GOG function: CClientExoApp::GetPlayerByPlayerID  gog 0x0040cc90  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f7d0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f890:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cc70 CClientExoApp::GetPlayerList -> 0x0073f7b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ccb0 CClientExoApp::GetNetLayer -> 0x0073f7f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004382a0 CClientExoAppInternal::GetPlayerByPlayerID -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f7d0: 16-byte aligned: YES
  0073f7d0  55             PUSH EBP
  0073f7d1  8B EC          MOV EBP,ESP
  0073f7d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f7d0
string literals (0):
direct callees (1):
  0x0078BCC0  CClientExoAppInternal::ShutDownMoviePlayer
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f7d0 FUN_0073f7d0 size=28 ret=4 callers=1 callees=1
THUNK -> 0x0078BCC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f7d0(undefined4 param_1)

{
  FUN_0078bcc0(param_1);
  return;
}
```

### decoy: 0x0073f890 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f890: 16-byte aligned: YES
  0073f890  55             PUSH EBP
  0073f891  8B EC          MOV EBP,ESP
  0073f893  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f890
string literals (0):
direct callees (1):
  0x007856A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-35

GOG function: CClientExoApp::ContinueMessageProcessing  gog 0x0040ce40  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f980 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f960:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ce10 CClientExoApp::HandleMessage -> 0x0073f930  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ce60 CClientExoApp::Initialize -> 0x0073f9a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061aac0 CSWMGTargettingGunBank::IsTargetting -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f980: 16-byte aligned: YES
  0073f980  55             PUSH EBP
  0073f981  8B EC          MOV EBP,ESP
  0073f983  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f980
string literals (0):
direct callees (1):
  0x007819E0  CClientExoAppInternal::GetModuleCamera
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f980 FUN_0073f980 size=22 ret=0 callers=1 callees=1
THUNK -> 0x007819E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f980(undefined4 param_1)

{
  FUN_007819e0(param_1);
  return;
}
```

### decoy: 0x0073f960 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f960: 16-byte aligned: YES
  0073f960  55             PUSH EBP
  0073f961  8B EC          MOV EBP,ESP
  0073f963  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f960
string literals (0):
direct callees (1):
  0x0078CF90  CClientExoAppInternal::ContinueMessageProcessing
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-36

GOG function: CClientExoApp::GetPlayerCreature  gog 0x0040c910  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f450 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 7 (callers: LoadArea 0x7a2fb0, Update 0x7a6760, HiliteAreaObjects 0x7aa460)
- best decoy (neighbour/other candidate) scores: 0x0073f750:STR0/CALLEE4/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c8f0 CClientExoApp::GetModule -> 0x0073f430  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c930 CClientExoApp::GetPlayerCreatureId -> 0x0073f470  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 57): the card's A-CALLEE credit may come from these, not from callees
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x00453160 CSWCArea::Update -> Steam 0x007a6760  in candidate callers: yes
  0x00456e60 CSWCArea::HiliteAreaObjects -> Steam 0x007aa460  in candidate callers: yes
  0x004571a0 CSWCArea::UpdateShadowingLights -> Steam 0x007aa7a0  in candidate callers: yes
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
prologue for 0x0073f450: 16-byte aligned: YES
  0073f450  55             PUSH EBP
  0073f451  8B EC          MOV EBP,ESP
  0073f453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f450
string literals (0):
direct callees (1):
  0x0078CEE0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f450 FUN_0073f450 size=22 ret=0 callers=57 callees=1
THUNK -> 0x0078CEE0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f450(undefined4 param_1)

{
  FUN_0078cee0(param_1);
  return;
}
```

### decoy: 0x0073f750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f750: 16-byte aligned: YES
  0073f750  55             PUSH EBP
  0073f751  8B EC          MOV EBP,ESP
  0073f753  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

## Card B11a-1-37

GOG function: CClientExoApp::Destructor  gog 0x0040c490  callconv=(blank)  param_size_bytes=0  gog_size=88  gog_retn=0
Candidate Steam address: 0x0073eff0 (size 88, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073ef30:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c470 CClientExoApp::GetFPS -> 0x0073f410  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c4f0 CClientExoApp::BeginIntro -> 0x0073f490  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0042d620 CClientExoAppInternal::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0073eff0: 16-byte aligned: YES
  0073eff0  55             PUSH EBP
  0073eff1  8B EC          MOV EBP,ESP
  0073eff3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073eff0
string literals (0):
direct callees (2):
  0x007810A0  CClientExoAppInternal::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073eff0 FUN_0073eff0 size=88 ret=0 callers=1 callees=2

void __fastcall FUN_0073eff0(undefined4 *param_1)

{
  int iVar1;
  
  *param_1 = CClientExoApp::vftable;
  iVar1 = param_1[1 = byte +0x4];
  if (iVar1 != 0) {
    FUN_007810a0();
    FUN_009196fd(iVar1);
  }
  return;
}
```

### decoy: 0x0073ef30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ef30: 16-byte aligned: YES
  0073ef30  55             PUSH EBP
  0073ef31  8B EC          MOV EBP,ESP
  0073ef33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0073ef30
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x00780460  CClientExoAppInternal::Constructor
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-38

GOG function: CClientExoApp::GetClientOptions  gog 0x0040ccf0  callconv=__thiscall  param_size_bytes=0  gog_size=28  gog_retn=0
Candidate Steam address: 0x0072fb00 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 25 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 25 (callers: MainLoop 0x781be0, LoadArea 0x7a2fb0, AddPlaceableObjectStatic 0x7a9f40)
- best decoy (neighbour/other candidate) scores: 0x0073f750:STR0/CALLEE3/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ccd0 CClientExoApp::GetSWCMessage -> 0x0073f810  gate_class=AUTO
- GOG-order neighbour after:  0x0040cd10 CClientExoApp::GetServerInfo -> 0x0073f830  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (25 of 168): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x00456940 CSWCArea::AddPlaceableObjectStatic -> Steam 0x007a9f40  in candidate callers: yes
  0x004df970 CGuiInGame::ShowStatusSummary -> Steam 0x007cf270  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0072fb00: 16-byte aligned: YES
  0072fb00  55             PUSH EBP
  0072fb01  8B EC          MOV EBP,ESP
  0072fb03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072fb00
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0072fb00 FUN_0072fb00 size=28 ret=0 callers=168 callees=0

undefined4 __fastcall FUN_0072fb00(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 4);
}
```

### decoy: 0x0073f750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f750: 16-byte aligned: YES
  0073f750  55             PUSH EBP
  0073f751  8B EC          MOV EBP,ESP
  0073f753  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

## Card B11a-1-39

GOG function: CClientExoApp::SetCharacterFile  gog 0x0040c6c0  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073f200 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f100:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x0040c660 CClientExoApp::ResetCharacterFile -> 0x0073f1a0  gate_class=AUTO
- GOG-order neighbour after:  0x0040c6f0 CClientExoApp::DisplayMainMenu -> 0x0073f230  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043a610 CClientExoAppInternal::SetCharacterFile -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f200: 16-byte aligned: YES
  0073f200  55             PUSH EBP
  0073f201  8B EC          MOV EBP,ESP
  0073f203  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f200
string literals (0):
direct callees (1):
  0x0078E040  CClientExoAppInternal::SetCharacterFile
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f200 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=1 callees=1
THUNK -> 0x0078E040 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_0078e040(param_1,param_2,param_3);
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

## Card B11a-1-40

GOG function: CClientExoApp::InitializeInGameGUI  gog 0x0040cd50  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f870 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::AttachPlayerGameObjectToArea | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f850:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cd30 CClientExoApp::AttachPlayerGameObjectToArea -> 0x0073f850  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cd70 CClientExoApp::ServerModeChange -> 0x0073f890  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00438910 CClientExoAppInternal::InitializeInGameGUI -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f870: 16-byte aligned: YES
  0073f870  55             PUSH EBP
  0073f871  8B EC          MOV EBP,ESP
  0073f873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f870
string literals (0):
direct callees (1):
  0x0078C330  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f870 FUN_0073f870 size=22 ret=0 callers=1 callees=1
THUNK -> 0x0078C330 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f870(undefined4 param_1)

{
  FUN_0078c330(param_1);
  return;
}
```

### decoy: 0x0073f850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f850: 16-byte aligned: YES
  0073f850  55             PUSH EBP
  0073f851  8B EC          MOV EBP,ESP
  0073f853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f850
string literals (0):
direct callees (1):
  0x0078C170  CClientExoAppInternal::CleanUpModuleServices
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-41

GOG function: CClientExoApp::GetActiveTimer  gog 0x0040cb90  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f6d0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f680:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cb60 CClientExoApp::SetPauseState -> 0x0073f6a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cbb0 CClientExoApp::SendSaveGameRequest -> 0x0073f6f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00430aa0 CClientExoAppInternal::GetActiveTimer -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f6d0: 16-byte aligned: YES
  0073f6d0  55             PUSH EBP
  0073f6d1  8B EC          MOV EBP,ESP
  0073f6d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f6d0
string literals (0):
direct callees (1):
  0x00784480  CClientExoAppInternal::GetActiveTimer
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f6d0 FUN_0073f6d0 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00784480 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f6d0(undefined4 param_1)

{
  FUN_00784480(param_1);
  return;
}
```

### decoy: 0x0073f680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f680: 16-byte aligned: YES
  0073f680  55             PUSH EBP
  0073f681  8B EC          MOV EBP,ESP
  0073f683  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f680
string literals (0):
direct callees (1):
  0x00784420  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-42

GOG function: CClientExoAppInternal::ShutDownToMainMenu  gog 0x0043a6c0  callconv=(blank)  param_size_bytes=0  gog_size=339  gog_retn=0
Candidate Steam address: 0x0078d220 (size 91, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::EmptyHitcheckIgnoreList | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078d280:STR0/CALLEE0/SIG0.094/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043a610 CClientExoAppInternal::SetCharacterFile -> 0x0078e040  gate_class=REVIEW
- GOG-order neighbour after:  0x0043a820 CClientExoAppInternal::RestartMoviesIfNecessary -> 0x0078e250  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00401c30 CAppManager::DestroyServer -> Steam ?  in candidate callees: n/a
  0x0042db10 CClientExoAppInternal::DisplayMainMenu -> Steam ?  in candidate callees: n/a
  0x00431a30 CClientExoAppInternal::UnloadModule -> Steam ?  in candidate callees: n/a
  0x0043abf0 CClientExoAppInternal::StartMenuMusic -> Steam ?  in candidate callees: n/a
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callees: no
  0x00469b40 CGameObjectArray::Delete_2 -> Steam ?  in candidate callees: n/a
  0x004f0520 CSWPlayerControlCamRelative::GetPlayerWalking -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0040d020 CClientExoApp::ShutDownToMainMenu -> Steam 0x0073fb40  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x14,0x40,0x48,0x108,0x18c,0x1ec,0x250,0x254} | Steam {}
imm sets (* = in both, 0 common): GOG {0x2,0x2c,0x7f} | Steam {0x4,0x10,0x190}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078d220: 16-byte aligned: YES
  0078d220  55             PUSH EBP
  0078d221  8B EC          MOV EBP,ESP
  0078d223  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078d220
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078d220 FUN_0078d220 size=91 ret=0 callers=3 callees=4

void __fastcall FUN_0078d220(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = FUN_007a1720(param_1);
  while (iVar2 = FUN_007a1780(), iVar2 == 0) {
    iVar2 = FUN_007f2a90();
    if (iVar2 != 0) {
      FUN_009196fd(iVar2,param_1,iVar2,uVar1);
    }
  }
  return;
}
```

### decoy: 0x0078d280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078d280: 16-byte aligned: YES
  0078d280  55             PUSH EBP
  0078d281  8B EC          MOV EBP,ESP
  0078d283  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078d280
string literals (0):
direct callees (8):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x004491C0  CExoPtrList::Add (doubling)
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x006D5200  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 10):
  +0x98 [dec 152]  x14  via EAX,EDX
  +0x10 [dec 16]  x11  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via ECX
  +0x18 [dec 24]  x3  via ECX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x34 [dec 52]  x2  via ECX,EDX
  +0x48 [dec 72]  x2  via EAX
  +0x14 [dec 20]  x1  via EDX
  +0x2C [dec 44]  x1  via EDX
  +0x224 [dec 548]  x1  via ECX
```

## Card B11a-1-43

GOG function: CClientExoApp::SetPlayerCreature  gog 0x0040cdf0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f910 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f850:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cdd0 CClientExoApp::EmptyHitcheckIgnoreList -> 0x0073f8f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ce10 CClientExoApp::HandleMessage -> 0x0073f930  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444c80 CClientExoAppInternal::SetPlayerCreature -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00785d40 Global::ExecuteCommandDestroyObject -> Steam 0x00670760  in candidate callers: yes
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
prologue for 0x0073f910: 16-byte aligned: YES
  0073f910  55             PUSH EBP
  0073f911  8B EC          MOV EBP,ESP
  0073f913  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f910
string literals (0):
direct callees (1):
  0x00798660  CClientExoAppInternal::SetPlayerCreature
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f910 FUN_0073f910 size=28 ret=4 callers=3 callees=1
THUNK -> 0x00798660 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f910(undefined4 param_1)

{
  FUN_00798660(param_1);
  return;
}
```

### decoy: 0x0073f850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f850: 16-byte aligned: YES
  0073f850  55             PUSH EBP
  0073f851  8B EC          MOV EBP,ESP
  0073f853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f850
string literals (0):
direct callees (1):
  0x0078C170  CClientExoAppInternal::CleanUpModuleServices
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-44

GOG function: CClientExoApp::GetPlayerByGameObjectID  gog 0x0040c9d0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f510 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f530:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c990 CClientExoApp::GetGameObject -> 0x0073f4d0  gate_class=AUTO
- GOG-order neighbour after:  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> 0x0073f530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00438300 CClientExoAppInternal::GetPlayerByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 5): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0073f510: 16-byte aligned: YES
  0073f510  55             PUSH EBP
  0073f511  8B EC          MOV EBP,ESP
  0073f513  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f510
string literals (0):
direct callees (1):
  0x0078BD20  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f510 FUN_0073f510 size=28 ret=4 callers=5 callees=1
THUNK -> 0x0078BD20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f510(undefined4 param_1)

{
  FUN_0078bd20(param_1);
  return;
}
```

### decoy: 0x0073f530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f530: 16-byte aligned: YES
  0073f530  55             PUSH EBP
  0073f531  8B EC          MOV EBP,ESP
  0073f533  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f530
string literals (0):
direct callees (1):
  0x0078BD80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-45

GOG function: CClientExoApp::GetModule  gog 0x0040c8f0  callconv=__thiscall  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f430 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 20 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 20 (callers: GetInFreeLook 0x741290, MainLoop 0x781be0, LoadArea 0x7a2fb0)
- best decoy (neighbour/other candidate) scores: 0x0073f750:STR0/CALLEE5/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c8d0 CClientExoApp::GetConnectionLib -> 0x0073f410  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c910 CClientExoApp::GetPlayerCreature -> 0x0073f450  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004f0520 CSWPlayerControlCamRelative::GetPlayerWalking -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (20 of 112): the card's A-CALLEE credit may come from these, not from callees
  0x0040e750 CClientExoApp::GetInFreeLook -> Steam 0x00741290  in candidate callers: yes
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x004550d0 CSWCArea::UpdateDayNightCycle -> Steam 0x007a86d0  in candidate callers: yes
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
prologue for 0x0073f430: 16-byte aligned: YES
  0073f430  55             PUSH EBP
  0073f431  8B EC          MOV EBP,ESP
  0073f433  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f430
string literals (0):
direct callees (1):
  0x00726F80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f430 FUN_0073f430 size=22 ret=0 callers=112 callees=1
THUNK -> 0x00726F80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f430(undefined4 param_1)

{
  FUN_00726f80(param_1);
  return;
}
```

### decoy: 0x0073f750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f750: 16-byte aligned: YES
  0073f750  55             PUSH EBP
  0073f751  8B EC          MOV EBP,ESP
  0073f753  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

## Card B11a-1-46

GOG function: CClientExoApp::RemoveObjectFromHitcheckIgnoreList  gog 0x0040cdb0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f8b0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f8d0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cd90 CClientExoApp::AddObjectToHitcheckIgnoreList -> 0x0073f8d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cdd0 CClientExoApp::EmptyHitcheckIgnoreList -> 0x0073f8f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00439760 CClientExoAppInternal::RemoveObjectFromHitcheckIgnoreList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x00456940 CSWCArea::AddPlaceableObjectStatic -> Steam 0x007a9f40  in candidate callers: no
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
prologue for 0x0073f8b0: 16-byte aligned: YES
  0073f8b0  55             PUSH EBP
  0073f8b1  8B EC          MOV EBP,ESP
  0073f8b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f8b0
string literals (0):
direct callees (1):
  0x0078D100  CClientExoAppInternal::AddObjectToHitcheckIgnoreList
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f8b0 FUN_0073f8b0 size=28 ret=4 callers=3 callees=1
THUNK -> 0x0078D100 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f8b0(undefined4 param_1)

{
  FUN_0078d100(param_1);
  return;
}
```

### decoy: 0x0073f8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f8d0: 16-byte aligned: YES
  0073f8d0  55             PUSH EBP
  0073f8d1  8B EC          MOV EBP,ESP
  0073f8d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f8d0
string literals (0):
direct callees (1):
  0x0078D190  CClientExoAppInternal::RemoveObjectFromHitcheckIgnoreList
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-47

GOG function: CClientExoApp::ServerModeChange  gog 0x0040cd70  callconv=(blank)  param_size_bytes=1  gog_size=29  gog_retn=4
Candidate Steam address: 0x0073f890 (size 29, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f8b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cd50 CClientExoApp::InitializeInGameGUI -> 0x0073f870  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cd90 CClientExoApp::AddObjectToHitcheckIgnoreList -> 0x0073f8d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431cc0 CClientExoAppInternal::ServerModeChange -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f890: 16-byte aligned: YES
  0073f890  55             PUSH EBP
  0073f891  8B EC          MOV EBP,ESP
  0073f893  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f890
string literals (0):
direct callees (1):
  0x007856A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f890 FUN_0073f890 size=29 ret=4 callers=1 callees=1
THUNK -> 0x007856A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f890(undefined1 param_1)

{
  FUN_007856a0(param_1);
  return;
}
```

### decoy: 0x0073f8b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f8b0: 16-byte aligned: YES
  0073f8b0  55             PUSH EBP
  0073f8b1  8B EC          MOV EBP,ESP
  0073f8b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f8b0
string literals (0):
direct callees (1):
  0x0078D100  CClientExoAppInternal::AddObjectToHitcheckIgnoreList
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-48

GOG function: CClientExoApp::Constructor  gog 0x0040c3d0  callconv=__thiscall  param_size_bytes=0  gog_size=147  gog_retn=0
Candidate Steam address: 0x0073ef30 (size 147, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073eff0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c380 Global::IsJoystickConnected  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040c470 CClientExoApp::GetFPS -> 0x0073f410  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0042c9e0 CClientExoAppInternal::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x4dc*} | Steam {0x4*,0x10*,0x4dc*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0073ef30: 16-byte aligned: YES
  0073ef30  55             PUSH EBP
  0073ef31  8B EC          MOV EBP,ESP
  0073ef33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0073ef30
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x00780460  CClientExoAppInternal::Constructor
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073ef30 FUN_0073ef30 size=147 ret=0 callers=1 callees=2

undefined4 * __fastcall FUN_0073ef30(undefined4 *param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961f7b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CBaseExoApp::vftable;
  *param_1 = CClientExoApp::vftable;
  iVar2 = FUN_00919723(0x4dc,uVar1);
  local_8 = 0;
  if (iVar2 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_00780460();
  }
  param_1[1 = byte +0x4] = local_20;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0073eff0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073eff0: 16-byte aligned: YES
  0073eff0  55             PUSH EBP
  0073eff1  8B EC          MOV EBP,ESP
  0073eff3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073eff0
string literals (0):
direct callees (2):
  0x007810A0  CClientExoAppInternal::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-49

GOG function: CClientExoApp::IsClientImeUI  gog 0x0040c830  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0073f370 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::Constructor | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073f350:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c810 CClientExoApp::GetClientLanguage -> 0x0073f350  gate_class=AUTO
- GOG-order neighbour after:  0x0040c850 CClientExoApp::GetImeUI -> 0x0073f390  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0xdc*} | Steam {0x4*,0xdc*}
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
prologue for 0x0073f370: 16-byte aligned: YES
  0073f370  55             PUSH EBP
  0073f371  8B EC          MOV EBP,ESP
  0073f373  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f370
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0xDC [dec 220]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f370 FUN_0073f370 size=31 ret=0 callers=2 callees=0

undefined4 __fastcall FUN_0073f370(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0xdc);
}
```

### decoy: 0x0073f350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f350: 16-byte aligned: YES
  0073f350  55             PUSH EBP
  0073f351  8B EC          MOV EBP,ESP
  0073f353  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f350
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x148 [dec 328]  x1  via ECX
```

## Card B11a-1-50

GOG function: CClientExoApp::CreateModule  gog 0x0040c730  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f270 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f250:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c710 CClientExoApp::UnloadModule -> 0x0073f250  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c750 CClientExoApp::ExitProgram -> 0x0073f290  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004319a0 CClientExoAppInternal::CreateModule -> Steam ?  in candidate callees: n/a
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

## Card B11a-1-51

GOG function: CClientExoApp::AddObjectModelDirectlyToIgnoreList  gog 0x0040d200  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073fe40 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::SetInputActive | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe20:STR0/CALLEE0/SIG0.5/SRV0/VT0
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

## Card B11a-1-52

GOG function: CClientExoApp::GetCanSendDialog  gog 0x0040d900  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x00740350 (size 37, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::SetPausedByCombat | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740330:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d8e0 CClientExoApp::SetCanSendDialog -> 0x00740420  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d930 CClientExoApp::SetDialogSent -> 0x00740470  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448bb0 CClientExoAppInternal::GetCanSendDialog -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740350: 16-byte aligned: YES
  00740350  55             PUSH EBP
  00740351  8B EC          MOV EBP,ESP
  00740353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740350
string literals (0):
direct callees (1):
  0x0079BF40  CClientExoAppInternal::SetPausedByCombat
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740350 FUN_00740350 size=37 ret=12 callers=31 callees=1
THUNK -> 0x0079BF40 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740350(undefined4 param_1,undefined1 param_2,undefined4 param_3)

{
  FUN_0079bf40(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x00740330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740330: 16-byte aligned: YES
  00740330  55             PUSH EBP
  00740331  8B EC          MOV EBP,ESP
  00740333  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740330
string literals (0):
direct callees (1):
  0x0079BF00  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-53

GOG function: CClientExoApp::SendSaveGameRequest  gog 0x0040cbb0  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073f6f0 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f720:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cb90 CClientExoApp::GetActiveTimer -> 0x0073f6d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cbe0 CClientExoApp::SendLoadGameRequest -> 0x0073f720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431c40 CClientExoAppInternal::SendSaveGameRequest -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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

## Card B11a-1-54

GOG function: CClientExoApp::GetPlaceableByGameObjectID  gog 0x0040ca90  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f5d0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: AddPlaceableObjectStatic 0x7a9f40, AddCorpse 0x7aaec0)
- best decoy (neighbour/other candidate) scores: 0x0073f8d0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ca70 CClientExoApp::GetProjectileByGameObjectID -> 0x0073f5b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cab0 CClientExoApp::GetDoorByGameObjectID -> 0x0073f5f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00438590 CClientExoAppInternal::GetPlaceableByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x00456940 CSWCArea::AddPlaceableObjectStatic -> Steam 0x007a9f40  in candidate callers: yes
  0x004578c0 CSWCArea::AddCorpse -> Steam 0x007aaec0  in candidate callers: yes
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

### decoy: 0x0073f8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f8d0: 16-byte aligned: YES
  0073f8d0  55             PUSH EBP
  0073f8d1  8B EC          MOV EBP,ESP
  0073f8d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f8d0
string literals (0):
direct callees (1):
  0x0078D190  CClientExoAppInternal::RemoveObjectFromHitcheckIgnoreList
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-55

GOG function: CClientExoApp::ExitProgram  gog 0x0040c750  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f290 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f270:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c730 CClientExoApp::CreateModule -> 0x0073f270  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c770 CClientExoApp::GetCaptureEventFile -> 0x0073f2b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431980 CClientExoAppInternal::ExitProgram -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f290: 16-byte aligned: YES
  0073f290  55             PUSH EBP
  0073f291  8B EC          MOV EBP,ESP
  0073f293  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f290
string literals (0):
direct callees (1):
  0x00785360  CClientExoAppInternal::ExitProgram
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f290 FUN_0073f290 size=22 ret=0 callers=2 callees=1
THUNK -> 0x00785360 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f290(undefined4 param_1)

{
  FUN_00785360(param_1);
  return;
}
```

### decoy: 0x0073f270 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f270: 16-byte aligned: YES
  0073f270  55             PUSH EBP
  0073f271  8B EC          MOV EBP,ESP
  0073f273  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f270
string literals (0):
direct callees (1):
  0x00785380  CClientExoAppInternal::CreateModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-56

GOG function: CClientExoApp::GetConnectionLib  gog 0x0040c8d0  callconv=(blank)  param_size_bytes=0  gog_size=28  gog_retn=0
Candidate Steam address: 0x0073f410 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetFPS | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073f3f0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c8b0 CClientExoApp::GetAnimationTimer -> 0x0073f3f0  gate_class=AUTO
- GOG-order neighbour after:  0x0040c8f0 CClientExoApp::GetModule -> 0x0073f430  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x34*} | Steam {0x4*,0x34*}
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
prologue for 0x0073f410: 16-byte aligned: YES
  0073f410  55             PUSH EBP
  0073f411  8B EC          MOV EBP,ESP
  0073f413  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f410
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x34 [dec 52]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f410 FUN_0073f410 size=28 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_0073f410(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x34);
}
```

### decoy: 0x0073f3f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f3f0: 16-byte aligned: YES
  0073f3f0  55             PUSH EBP
  0073f3f1  8B EC          MOV EBP,ESP
  0073f3f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f3f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x30 [dec 48]  x1  via ECX
```

## Card B11a-1-57

GOG function: CClientExoApp::UnloadModule  gog 0x0040c710  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f250 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f310:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c6f0 CClientExoApp::DisplayMainMenu -> 0x0073f230  gate_class=REVIEW
- GOG-order neighbour after:  0x0040c730 CClientExoApp::CreateModule -> 0x0073f270  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431a30 CClientExoAppInternal::UnloadModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
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
prologue for 0x0073f250: 16-byte aligned: YES
  0073f250  55             PUSH EBP
  0073f251  8B EC          MOV EBP,ESP
  0073f253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f250
string literals (0):
direct callees (1):
  0x00785410  CClientExoAppInternal::UnloadModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f250 FUN_0073f250 size=22 ret=0 callers=2 callees=1
THUNK -> 0x00785410 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f250(undefined4 param_1)

{
  FUN_00785410(param_1);
  return;
}
```

### decoy: 0x0073f310 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f310: 16-byte aligned: YES
  0073f310  55             PUSH EBP
  0073f311  8B EC          MOV EBP,ESP
  0073f313  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f310
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-1-58

GOG function: CClientExoApp::AddObjectModelDirectlyToIgnoreList  gog 0x0040d200  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073fc30 (size 97, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.38 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fc10:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d1e0 CClientExoApp::GetApplicationId -> 0x0073fd70  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d220 CClientExoApp::StartMenuMusic -> 0x0073fd40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043a890 CClientExoAppInternal::AddObjectModelDirectlyToIgnoreList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0xc,0x1c8}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x4,0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0073fc30: 16-byte aligned: YES
  0073fc30  55             PUSH EBP
  0073fc31  8B EC          MOV EBP,ESP
  0073fc33  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fc30
string literals (0):
direct callees (1):
  0x00710840  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x1C8 [dec 456]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fc30 FUN_0073fc30 size=97 ret=8 callers=1 callees=1

undefined4 * __thiscall FUN_0073fc30(int param_1,undefined4 *param_2,byte param_3)

{
  undefined4 *puVar1;
  
  if (*(int *)(*(int *)(param_1 + 4) + 0x1c8) == 0) {
    FUN_00710840(&DAT_00986925);
  }
  else {
    puVar1 = (undefined4 *)(*(int *)(*(int *)(param_1 + 4) + 0x1c8) + (uint)param_3 * 0x10);
    *param_2 = *puVar1;
    param_2[1 = byte +0x4] = puVar1[1 = byte +0x4];
    param_2[2 = byte +0x8] = puVar1[2 = byte +0x8];
    param_2[3 = byte +0xc] = puVar1[3 = byte +0xc];
  }
  return param_2;
}
```

### decoy: 0x0073fc10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fc10: 16-byte aligned: YES
  0073fc10  55             PUSH EBP
  0073fc11  8B EC          MOV EBP,ESP
  0073fc13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fc10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1C4 [dec 452]  x1  via ECX
```

## Card B11a-1-59

GOG function: CClientExoApp::EmptyHitcheckIgnoreList  gog 0x0040cdd0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f8f0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f8d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cdb0 CClientExoApp::RemoveObjectFromHitcheckIgnoreList -> 0x0073f8b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cdf0 CClientExoApp::SetPlayerCreature -> 0x0073f910  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004397f0 CClientExoAppInternal::EmptyHitcheckIgnoreList -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f8f0: 16-byte aligned: YES
  0073f8f0  55             PUSH EBP
  0073f8f1  8B EC          MOV EBP,ESP
  0073f8f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f8f0
string literals (0):
direct callees (1):
  0x0078D220  CClientExoAppInternal::EmptyHitcheckIgnoreList
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f8f0 FUN_0073f8f0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x0078D220 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f8f0(undefined4 param_1)

{
  FUN_0078d220(param_1);
  return;
}
```

### decoy: 0x0073f8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f8d0: 16-byte aligned: YES
  0073f8d0  55             PUSH EBP
  0073f8d1  8B EC          MOV EBP,ESP
  0073f8d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f8d0
string literals (0):
direct callees (1):
  0x0078D190  CClientExoAppInternal::RemoveObjectFromHitcheckIgnoreList
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-1-60

GOG function: CClientExoApp::SetPauseState  gog 0x0040cb60  callconv=(blank)  param_size_bytes=12  gog_size=37  gog_retn=12
Candidate Steam address: 0x0073f6a0 (size 37, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f680:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cb40 CClientExoApp::GetPauseState -> 0x0073f680  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cb90 CClientExoApp::GetActiveTimer -> 0x0073f6d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00430d60 CClientExoAppInternal::SetPauseState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0073f6a0: 16-byte aligned: YES
  0073f6a0  55             PUSH EBP
  0073f6a1  8B EC          MOV EBP,ESP
  0073f6a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f6a0
string literals (0):
direct callees (1):
  0x00784740  CClientExoAppInternal::SetPauseState
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f6a0 FUN_0073f6a0 size=37 ret=12 callers=1 callees=1
THUNK -> 0x00784740 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f6a0(undefined1 param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_00784740(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x0073f680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f680: 16-byte aligned: YES
  0073f680  55             PUSH EBP
  0073f681  8B EC          MOV EBP,ESP
  0073f683  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f680
string literals (0):
direct callees (1):
  0x00784420  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```
