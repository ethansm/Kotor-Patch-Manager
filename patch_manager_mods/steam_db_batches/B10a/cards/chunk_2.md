# Review cards B10a chunk 2

Answer each card with one TSV row in `steam_db_batches/B10a/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B10a-2-01

GOG function: CSWSObject::IsDirtyFlagSet  gog 0x006b1160  callconv=(blank)  param_size_bytes=4  gog_size=67  gog_retn=4
Candidate Steam address: 0x0054a480 (size 67, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0054a4d0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b1110 CSWSObject::SetDirtyFlag -> 0x0054a430  gate_class=REVIEW
- GOG-order neighbour after:  0x006b11b0 CSWSObject::ClearDirtyFlag -> 0x0054a4d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0064d1b0 CServerAIMaster::UpdateState -> Steam 0x0051d790  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x224*} | Steam {0x224*}
imm sets (* = in both, 3 common): GOG {0x4*,0x5*,0xc*} | Steam {0x4*,0x5*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0054a480: 16-byte aligned: YES
  0054a480  55             PUSH EBP
  0054a481  8B EC          MOV EBP,ESP
  0054a483  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054a480
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0054a480 FUN_0054a480 size=67 ret=4 callers=1 callees=0

bool __thiscall FUN_0054a480(int param_1,uint param_2)

{
  return (1 << ((byte)param_2 & 0x1f) & *(uint *)(param_1 + 0x224 + (param_2 >> 5) * 4)) != 0;
}
```

### decoy: 0x0054a4d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054a4d0: 16-byte aligned: YES
  0054a4d0  55             PUSH EBP
  0054a4d1  8B EC          MOV EBP,ESP
  0054a4d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054a4d0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B10a-2-02

GOG function: CSWCObject::TurnOffShadows  gog 0x00472660  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x007ed5f0 (size 475, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCObject::Destructor_2+name_held_elsewhere=0x007ed500 | G7 mac: pass(0.11) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed5c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472630 CSWCObject::TurnOnShadows -> 0x007ed4d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00472690 CSWCObject::BarkString -> 0x007ed530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x68,0xbc} | Steam {0x4,0x5c,0xc8,0xcc,0xf8}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x3,0x4,0x30,0x6c,0x70,0xa8,0xe8}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ed5f0: 16-byte aligned: YES
  007ed5f0  55             PUSH EBP
  007ed5f1  8B EC          MOV EBP,ESP
  007ed5f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007ed5f0 FUN_007ed5f0 size=475 ret=0 callers=30 callees=11

void __fastcall FUN_007ed5f0(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0096b9ca;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWCObject::vftable;
  local_8 = 4;
  FUN_0073f8d0(param_1[1 = byte +0x4]);
  FUN_007f0a50(uVar2);
  FUN_0073f790();
  FUN_007a2540(param_1);
  if (param_1[0x32 = byte +0xc8] != 0) {
    if ((undefined4 *)param_1[0x32 = byte +0xc8] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x32 = byte +0xc8])(1);
    }
    param_1[0x32 = byte +0xc8] = 0;
  }
  if (param_1[0x33 = byte +0xcc] != 0) {
    iVar1 = param_1[0x33 = byte +0xcc];
    if (iVar1 != 0) {
      FUN_0070b080();
      FUN_009196fd(iVar1);
    }
    param_1[0x33 = byte +0xcc] = 0;
  }
  if (param_1[0x17 = byte +0x5c] != 0) {
    FUN_009196fd(param_1[0x17 = byte +0x5c]);
    param_1[0x17 = byte +0x5c] = 0;
  }
  if (param_1[0x3e = byte +0xf8] != 0) {
    FUN_0054a900();
    param_1[0x3e = byte +0xf8] = 0;
  }
  local_8._0_1_ = 3;
  FUN_00733780();
  local_8._0_1_ = 2;
  FUN_00623320();
  local_8._0_1_ = 1;
  Free();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_0072f5f0();
  *param_1 = CGameObject::vftable;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007ed5c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed5c0: 16-byte aligned: YES
  007ed5c0  55             PUSH EBP
  007ed5c1  8B EC          MOV EBP,ESP
  007ed5c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed5c0
string literals (0):
direct callees (2):
  0x007ED5F0  CSWCObject::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B10a-2-03

GOG function: CSWCObject::AnimationRunning  gog 0x00474a70  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef500 (size 197, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::AnimationLooping | G7 mac: pass(0.56) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.54 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef470:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004749e0 CSWCObject::AnimationAttack -> 0x007ef880  gate_class=REVIEW
- GOG-order neighbour after:  0x00474b00 CSWCObject::AddFireForgetAnimation -> 0x007ef9a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xcc*,0xe0*,0x130*} | Steam {0x8,0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x5,0x10*,0x578,0x5f7,0x640,0x6bf}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef500: 16-byte aligned: YES
  007ef500  55             PUSH EBP
  007ef501  8B EC          MOV EBP,ESP
  007ef503  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef500
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 5):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef500 FUN_007ef500 size=197 ret=4 callers=12 callees=1

bool __thiscall FUN_007ef500(int param_1,ushort param_2)

{
  int iVar1;
  bool bVar2;
  int local_8;
  
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

### decoy: 0x007ef470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef470: 16-byte aligned: YES
  007ef470  55             PUSH EBP
  007ef471  8B EC          MOV EBP,ESP
  007ef473  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B10a-2-04

GOG function: CSWCObject::AnimationAttack  gog 0x004749e0  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef910 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::AnimationHideEquippedItems | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef880:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474950 CSWCObject::AnimationParry -> 0x007ef7f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00474a70 CSWCObject::AnimationRunning -> 0x007ef470  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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

## Card B10a-2-05

GOG function: CSWCObject::GetCanSeeHeight  gog 0x00410cb0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00764fb0 (size 42, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::HideShadowBlob | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00764f80:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: autolabel:NotifyPaneDivider
- GOG-order neighbour before: 0x00410c80 CSWCObject::HideShadowBlob -> 0x00764fb0  gate_class=REVIEW
- GOG-order neighbour after:  0x00410cd0 CSWCCreature::GetPortrait  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x68,0xb0}
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
prologue for 0x00764fb0: 16-byte aligned: YES
  00764fb0  55             PUSH EBP
  00764fb1  8B EC          MOV EBP,ESP
  00764fb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00764fb0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB0 [dec 176]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00764fb0 CPaneContainerManager::NotifyPaneDivider size=42 ret=0 callers=0 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
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

### decoy: 0x00764f80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764f80: 16-byte aligned: YES
  00764f80  55             PUSH EBP
  00764f81  8B EC          MOV EBP,ESP
  00764f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00764f80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xAC [dec 172]  x1  via EDX
```

## Card B10a-2-06

GOG function: CSWSObject::RemoveObjectFromDialog  gog 0x006ad780  callconv=(blank)  param_size_bytes=0  gog_size=39  gog_retn=0
Candidate Steam address: 0x00546aa0 (size 39, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00546ad0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006ad750 CSWSObject::StopSoundPlayingInDialog -> 0x00546a70  gate_class=AUTO
- GOG-order neighbour after:  0x006ad7e0 CSWSObject::RemoveObjectFromDialog_2 -> 0x00546b00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x004db4b0 CGuiInGame::DestroyDeathMenuGUI -> Steam ?  in candidate callees: n/a
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
prologue for 0x00546aa0: 16-byte aligned: YES
  00546aa0  55             PUSH EBP
  00546aa1  8B EC          MOV EBP,ESP
  00546aa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00546aa0
string literals (0):
direct callees (2):
  0x0073F750  ClientApp::GetGuiInGame
  0x007CADB0  CGuiInGame::DestroyDeathMenuGUI
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00546aa0 FUN_00546aa0 size=39 ret=0 callers=1 callees=2

void __fastcall FUN_00546aa0(undefined4 param_1)

{
  FUN_0073f750(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  FUN_007cadb0();
  return;
}
```

### decoy: 0x00546ad0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00546ad0: 16-byte aligned: YES
  00546ad0  55             PUSH EBP
  00546ad1  8B EC          MOV EBP,ESP
  00546ad3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00546ad0
string literals (0):
direct callees (2):
  0x0073F750  ClientApp::GetGuiInGame
  0x007CB720  CGuiInGame::IsFadingInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B10a-2-07

GOG function: CSWSObject::GetActionByGroupId  gog 0x006a60a0  callconv=(blank)  param_size_bytes=8  gog_size=209  gog_retn=8
Candidate Steam address: 0x0053f3c0 (size 209, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: rotation:0x006a6380:0.92>0.64
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: GetAcceptableAction 0x5d7190; callers: ClearHostileActionsVersus 0x5867e0, UpdateActionQueue 0x588ed0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0053f4a0:STR0/CALLEE2/SIG0.7/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a5d30 CSWSObject::Destructor_2 -> 0x0053f050  gate_class=AUTO
- GOG-order neighbour after:  0x006a6180 CSWSObject::GetPositionByGroupIndex -> 0x0053f4a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x006b3380 CSWSObject::GetAcceptableAction -> Steam 0x005d7190  in candidate callees: yes
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0069f2c0 CSWSCreature::ClearHostileActionsVersus -> Steam 0x005867e0  in candidate callers: yes
  0x006a19b0 CSWSCreature::UpdateActionQueue -> Steam 0x00588ed0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x6c*} | Steam {0x6c*}
imm sets (* = in both, 4 common): GOG {0x8*,0x14*,0x100*,0xffff*} | Steam {0x8*,0x14*,0x100*,0xffff*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0053f3c0: 16-byte aligned: YES
  0053f3c0  55             PUSH EBP
  0053f3c1  8B EC          MOV EBP,ESP
  0053f3c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053f3c0
string literals (0):
direct callees (5):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x005D7190  CSWSObject::GetAcceptableAction
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0053f3c0 FUN_0053f3c0 size=209 ret=8 callers=2 callees=5

undefined2 __thiscall FUN_0053f3c0(undefined4 param_1,short param_2,undefined4 *param_3)

{
  bool bVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined2 local_10;
  undefined4 local_8;
  
  local_10 = 0xffff;
  *param_3 = 0;
  iVar2 = FUN_007a1780(param_1);
  if (iVar2 == 1) {
    local_10 = 0xffff;
  }
  else {
    local_8 = FUN_007a1720();
    puVar3 = (undefined4 *)FUN_00561430(local_8);
    bVar1 = false;
    while (puVar3 != (undefined4 *)0x0) {
      if (*(short *)(puVar3 + 0x1b) == param_2) {
        bVar1 = true;
        iVar2 = FUN_005d7190(*puVar3);
        if (iVar2 == 1) {
          local_10 = *(undefined2 *)puVar3;
          *param_3 = puVar3;
        }
      }
      else if (bVar1) {
        return local_10;
      }
      puVar3 = (undefined4 *)FUN_0058c370(&local_8);
    }
  }
  return local_10;
}
```

### decoy: 0x0053f4a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053f4a0: 16-byte aligned: YES
  0053f4a0  55             PUSH EBP
  0053f4a1  8B EC          MOV EBP,ESP
  0053f4a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053f4a0
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x3  via EAX,ECX
```

## Card B10a-2-08

GOG function: CSWCObject::GetCanSeeHeight  gog 0x00410cb0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x007650c0 (size 68, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00765060:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c80 CSWCObject::HideShadowBlob -> 0x00764fb0  gate_class=REVIEW
- GOG-order neighbour after:  0x00410cd0 CSWCCreature::GetPortrait  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x124,0x310}
imm sets (* = in both, 0 common): GOG {} | Steam {0x10,0xffff}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007650c0: 16-byte aligned: YES
  007650c0  55             PUSH EBP
  007650c1  8B EC          MOV EBP,ESP
  007650c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007650c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x310 [dec 784]  x2  via EAX,EDX
  +0x124 [dec 292]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007650c0 FUN_007650c0 size=68 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_007650c0(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x310) == 0) {
    uVar1 = 0xffff;
  }
  else {
    uVar1 = CONCAT22((short)((uint)*(int *)(param_1 + 0x310) >> 0x10),
                     *(undefined2 *)(*(int *)(param_1 + 0x310) + 0x124));
  }
  return uVar1;
}
```

### decoy: 0x00765060 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00765060: 16-byte aligned: YES
  00765060  55             PUSH EBP
  00765061  8B EC          MOV EBP,ESP
  00765063  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00765060
string literals (0):
direct callees (2):
  0x0085E8C0  (unnamed)
  0x00710840  (unnamed)
struct offsets touched (top 1):
  +0x310 [dec 784]  x2  via EAX,EDX
```

## Card B10a-2-09

GOG function: CSWSObject::Constructor  gog 0x006a55e0  callconv=__thiscall  param_size_bytes=12  gog_size=1789  gog_retn=12
Candidate Steam address: 0x0053e830 (size 1789, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 7 (callers: Constructor 0x5ffd60, Constructor 0x561f30, Constructor 0x61dcf0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0053e740:STR0/CALLEE0/SIG0.05/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a54f0 CSWSObjectActionNode::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006a5ce0 CSWSObject::GetLastName -> 0x0053ef50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (18), mapped to Steam through trusted rows only:
  0x004692d0 CGameObject::Constructor -> Steam ?  in candidate callees: n/a
  0x00469500 CGameObjectArray::AddObjectAtPos -> Steam ?  in candidate callees: n/a
  0x00469990 CGameObjectArray::AddInternalObject -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00607350 CExoLocString::Constructor -> Steam ?  in candidate callees: n/a
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  ... 10 more GOG callees
GOG callers mapped through trusted rows (7 of 10): the card's A-CALLEE credit may come from these, not from callees
  0x006634d0 CSWSItem::Constructor -> Steam 0x005ffd60  in candidate callers: yes
  0x0067ab60 CSWSCreature::Constructor -> Steam 0x00561f30  in candidate callers: yes
  0x0075cf80 CSWSTrigger::Constructor -> Steam 0x0061dcf0  in candidate callers: yes
  0x00761f20 CSWSDoor::Constructor -> Steam 0x00616a70  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 83 common): GOG {0x4*,0x8*,0xc*,0x14*,0x16*,0x30*,0x34*,0x40*,0x44*,0x50*,0x54*,0x58*,0x5c*,0x60*,...(+69)} | Steam {0x4*,0x8*,0xc*,0x14*,0x16*,0x30*,0x34*,0x40*,0x44*,0x50*,0x54*,0x58*,0x5c*,0x60*,...(+69)}
imm sets (* = in both, 34 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xc*,0xf*,0x18*,0x20*,0x3c*,...(+20)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xc*,0xf*,0x18*,0x20*,0x3c*,...(+20)}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0053e830: 16-byte aligned: YES
  0053e830  55             PUSH EBP
  0053e831  8B EC          MOV EBP,ESP
  0053e833  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0053e830
string literals (0):
direct callees (18):
  0x0053D660  (unnamed)
  0x00735240  (unnamed)
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x00722870  (unnamed)
  0x0052E4A0  (unnamed)
  0x005E91B0  CSWVarTable::Constructor
  0x0084A640  (unnamed)
  0x00543F50  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DD20  (unnamed)
  0x0053D890  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00919723  operator new
  0x0091C630  (unnamed)
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
struct offsets touched (top 10):
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,EDX
  +0x54 [dec 84]  x2  via EAX,EDX
  +0x18C [dec 396]  x2  via ECX
  +0x1D0 [dec 464]  x2  via ECX
  +0xC [dec 12]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x16 [dec 22]  x1  via EAX
  +0x30 [dec 48]  x1  via ECX
  +0x34 [dec 52]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0053e830 FUN_0053e830 size=1789 ret=12 callers=10 callees=18

undefined4 * FUN_0053e830(undefined1 param_1,int param_2,undefined4 param_3)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 *extraout_var;
  undefined4 *puVar5;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f63e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0053d660(param_1,param_2);
  local_8 = 0;
  *extraout_var = CSWSObject::vftable;
  puVar5 = extraout_var;
  FUN_00735240(uVar1);
  local_8._0_1_ = 1;
  FUN_00733540();
  local_8._0_1_ = 2;
  FUN_007107f0();
  puVar5[0x25 = byte +0x94] = 0;
  puVar5[0x26 = byte +0x98] = 0;
  puVar5[0x27 = byte +0x9c] = 0;
  puVar5[0x28 = byte +0xa0] = 0;
  puVar5[0x29 = byte +0xa4] = 0;
  puVar5[0x2a = byte +0xa8] = 0;
  FUN_00722870();
  local_8._0_1_ = 3;
  puVar4 = puVar5;
  FUN_0052e4a0(0);
  puVar5[0x44 = byte +0x110] = 0;
  local_8._0_1_ = 4;
  FUN_005e91b0();
  FUN_0084a640(0);
  local_8._0_1_ = 5;
  FUN_0084a640(0);
  local_8._0_1_ = 6;
  FUN_0084a640(0);
  local_8._0_1_ = 7;
  FUN_00733540();
  local_8._0_1_ = 8;
  FUN_0084a640(0);
  local_8._0_1_ = 9;
  FUN_0084a640(0);
  local_8 = CONCAT31(local_8._1_3_,10);
  puVar4[0x75 = byte +0x1d4] = 0;
  puVar4[0x76 = byte +0x1d8] = 0;
  puVar4[0x77 = byte +0x1dc] = 0;
  local_20 = 0;
  local_1c = 0;
  local_18 = 0;
  FUN_00543f50(&local_20,1,1,0);
  local_2c = 0x3f800000;
  local_28 = 0;
  local_24 = 0;
  FUN_00543f10(&local_2c);
  puVar4[0x36 = byte +0xd8] = 10000;
  puVar4[0x30 = byte +0xc0] = 0;
  puVar4[0x31 = byte +0xc4] = 0;
  puVar4[0x2f = byte +0xbc] = 0x3f800000;
  puVar4[0x23 = byte +0x8c] = 0;
  puVar5 = puVar4;
  if (param_2 == 0x7f000000) {
    puVar3 = puVar4 + 0x1 [dec 1] {undefined4*: byte +0x4};
    FUN_0051c080(puVar3,puVar4,param_3);
/* ... 103 more lines truncated by --max-lines */
```

### decoy: 0x0053e740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053e740: 16-byte aligned: YES
  0053e740  55             PUSH EBP
  0053e741  8B EC          MOV EBP,ESP
  0053e743  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053e740
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
  0x006FDBB0  CVirtualMachine::DeleteScriptSituation
struct offsets touched (top 1):
  +0x6E [dec 110]  x1  via ECX
```

## Card B10a-2-10

GOG function: CSWSObject::GetAQActionIDByID  gog 0x006b3410  callconv=(blank)  param_size_bytes=8  gog_size=388  gog_retn=8
Candidate Steam address: 0x005d7220 (size 388, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: rotation:0x006b16a0:0.74>0.54
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: UpdateActionQueue 0x588ed0, ExecuteCommandGetCurrentAction 0x67b100); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005d7190:STR0/CALLEE1/SIG0.043/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b3380 CSWSObject::GetAcceptableAction -> 0x005d7190  gate_class=REVIEW
- GOG-order neighbour after:  0x006b3670 CSWSObject::AIActionBarkString -> 0x005d7480  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x006a19b0 CSWSCreature::UpdateActionQueue -> Steam 0x00588ed0  in candidate callers: yes
  0x007906e0 Global::ExecuteCommandGetCurrentAction -> Steam 0x0067b100  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 35 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+21)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+21)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005d7220: 16-byte aligned: YES
  005d7220  55             PUSH EBP
  005d7221  8B EC          MOV EBP,ESP
  005d7223  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d7220
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x5D7434 [dec 6124596]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005d7220 FUN_005d7220 size=388 ret=8 callers=3 callees=0

undefined4 FUN_005d7220(short param_1,int param_2)

{
  undefined4 uVar1;
  
  if (param_2 == 0) {
    switch(param_1) {
    case 1:
      uVar1 = 0;
      break;
    default:
      uVar1 = 0xffff;
      break;
    case 7:
      uVar1 = 1;
      break;
    case 9:
      uVar1 = 2;
      break;
    case 0xc:
      uVar1 = 3;
      break;
    case 0xf:
      uVar1 = 4;
      break;
    case 0x14:
      uVar1 = 5;
      break;
    case 0x15:
      uVar1 = 6;
      break;
    case 0x18:
      uVar1 = 7;
      break;
    case 0x19:
      uVar1 = 8;
      break;
    case 0x1a:
      uVar1 = 9;
      break;
    case 0x1b:
      uVar1 = 10;
      break;
    case 0x1c:
      uVar1 = 0xb;
      break;
    case 0x1d:
      uVar1 = 0xc;
      break;
    case 0x1e:
      uVar1 = 0x24;
      break;
    case 0x26:
      uVar1 = 0xd;
      break;
    case 0x27:
      uVar1 = 0xe;
      break;
    case 0x28:
      uVar1 = 0xf;
      break;
    case 0x29:
      uVar1 = 0x10;
      break;
    case 0x2a:
      uVar1 = 0x11;
      break;
    case 0x2b:
      uVar1 = 0x12;
      break;
    case 0x2e:
      uVar1 = 0x13;
      break;
    case 0x32:
      uVar1 = 0x1f;
      break;
    case 0x36:
      uVar1 = 0x22;
      break;
/* ... 39 more lines truncated by --max-lines */
```

### decoy: 0x005d7190 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d7190: 16-byte aligned: YES
  005d7190  55             PUSH EBP
  005d7191  8B EC          MOV EBP,ESP
  005d7193  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d7190
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x5D71D8 [dec 6123992]  x1  via EDX
```

## Card B10a-2-11

GOG function: CSWSObject::DetachFromClientObject  gog 0x006b15e0  callconv=(blank)  param_size_bytes=0  gog_size=24  gog_retn=0
Candidate Steam address: 0x0054a900 (size 24, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0054a8e0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b15c0 CSWSObject::AttachmentFromClientObject -> 0x0054a8e0  gate_class=REVIEW
- GOG-order neighbour after:  0x006b1600 CSWSObject::DeleteDialog -> 0x0054a920  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x264*} | Steam {0x264*}
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
prologue for 0x0054a900: 16-byte aligned: YES
  0054a900  55             PUSH EBP
  0054a901  8B EC          MOV EBP,ESP
  0054a903  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054a900
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x264 [dec 612]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0054a900 FUN_0054a900 size=24 ret=0 callers=1 callees=0

void __fastcall FUN_0054a900(int param_1)

{
  *(undefined4 *)(param_1 + 0x264) = 0;
  return;
}
```

### decoy: 0x0054a8e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054a8e0: 16-byte aligned: YES
  0054a8e0  55             PUSH EBP
  0054a8e1  8B EC          MOV EBP,ESP
  0054a8e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054a8e0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x264 [dec 612]  x1  via EAX
```

## Card B10a-2-12

GOG function: CSWSObject::GetGender  gog 0x006ac140  callconv=(blank)  param_size_bytes=0  gog_size=16  gog_retn=0
Candidate Steam address: 0x00545460 (size 16, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: GetGender 0x5632b0)
- best decoy (neighbour/other candidate) scores: 0x00545420:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006ac100 CSWSObject::SetArea -> 0x00545420  gate_class=AUTO
- GOG-order neighbour after:  0x006ac150 CSWSObject::SetDialogDelay -> 0x00545470  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0067bee0 CSWSCreature::GetGender -> Steam 0x005632b0  in candidate callers: yes
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
prologue for 0x00545460: 16-byte aligned: YES
  00545460  55             PUSH EBP
  00545461  8B EC          MOV EBP,ESP
  00545463  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00545460
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00545460 FUN_00545460 size=16 ret=0 callers=1 callees=0

undefined1 FUN_00545460(void)

{
  return DAT_0099c4a2;
}
```

### decoy: 0x00545420 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00545420: 16-byte aligned: YES
  00545420  55             PUSH EBP
  00545421  8B EC          MOV EBP,ESP
  00545423  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00545420
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x124 [dec 292]  x1  via EAX
```

## Card B10a-2-13

GOG function: CSWCObject::Hilite  gog 0x004754a0  callconv=(blank)  param_size_bytes=5  gog_size=50  gog_retn=16
Candidate Steam address: 0x007f0570 (size 376, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCObject::SetEnvMap_2 | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.21 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f06f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00475460 CSWCObject::AbortFireForgetAnimation -> 0x007f0300  gate_class=AUTO
- GOG-order neighbour after:  0x004754e0 CSWCObject::UnHilite -> 0x007f0380  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004c92d0 CSWCAnimBase::Hilite -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68*} | Steam {0x4,0x8,0xc,0x48,0x68*,0x98}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x4,0x10*,0x4c,0xbc}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 5 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x007f0570: 16-byte aligned: YES
  007f0570  55             PUSH EBP
  007f0571  8B EC          MOV EBP,ESP
  007f0573  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007f0570
string literals (1):
  0x0099423C  at 007f05ec  'DEFAULT'
direct callees (7):
  0x00733540  CExoString::Constructor
  0x00733780  CExoString::~CExoString
  0x0073F430  CClientExoApp::GetModule
  0x00710C20  (unnamed)
  0x00710930  (unnamed)
  0x00919F10  _strlen
  0x00710810  (unnamed)
struct offsets touched (top 6):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x48 [dec 72]  x1  via EAX
  +0x98 [dec 152]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007f0570 FUN_007f0570 size=376 ret=16 callers=0 callees=7

undefined4 __thiscall
FUN_007f0570(int param_1,undefined1 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  size_t local_58;
  undefined1 local_50 [16];
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int local_24;
  undefined4 local_20;
  char *local_1c [2];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009639a8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) == 0) {
    local_28 = 0;
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    local_14 = FUN_0073f430();
    local_20 = 0;
    iVar1 = FUN_00710c20("DEFAULT");
    if (iVar1 == 0) {
      FUN_00710930(local_1c);
    }
    else {
      if ((local_14 != 0) && (local_24 = *(int *)(local_14 + 0x48), local_24 != 0)) {
        local_38 = *(undefined4 *)(local_24 + 0xbc);
        local_34 = *(undefined4 *)(local_24 + 0xc0);
        local_30 = *(undefined4 *)(local_24 + 0xc4);
        local_2c = *(undefined4 *)(local_24 + 0xc8 [dec 200]);
        FUN_00710930(local_1c);
      }
      if (local_1c[0] == (char *)0x0) {
        local_58 = 0;
      }
      else {
        local_58 = _strlen [GHIDRA-AUTO-LABEL: unreliable](local_1c[0]);
      }
      if (local_58 == 0) {
        local_3c = 0;
        local_8 = 0xffffffff;
        FUN_00733780();
        ExceptionList = local_10;
        return local_3c;
      }
    }
    FUN_00710810(local_1c);
    local_40 = (**(code **)(**(int **)(param_1 + 0x68) + 0x98))(param_2,param_3,param_4,local_50);
    local_8 = 0xffffffff;
    FUN_00733780();
    local_28 = local_40;
  }
  ExceptionList = local_10;
  return local_28;
}
```

### decoy: 0x007f06f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f06f0: 16-byte aligned: YES
  007f06f0  55             PUSH EBP
  007f06f1  8B EC          MOV EBP,ESP
  007f06f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f06f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xA0 [dec 160]  x1  via EDX
```

## Card B10a-2-14

GOG function: CSWCObject::AnimationRunning  gog 0x00474a70  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef3e0 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::AnimationWalking | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef350:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004749e0 CSWCObject::AnimationAttack -> 0x007ef880  gate_class=REVIEW
- GOG-order neighbour after:  0x00474b00 CSWCObject::AddFireForgetAnimation -> 0x007ef9a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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

## Card B10a-2-15

GOG function: CSWSObject::AttachmentFromClientObject  gog 0x006b15c0  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x0054a8e0 (size 25, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: GetServerObject 0x7f2540)
- best decoy (neighbour/other candidate) scores: 0x0054a900:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b1520 CSWSObject::GetClientObject -> 0x0054a840  gate_class=AUTO
- GOG-order neighbour after:  0x006b15e0 CSWSObject::DetachFromClientObject -> 0x0054a900  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004776e0 CSWCObject::GetServerObject -> Steam 0x007f2540  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x264*} | Steam {0x264*}
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
prologue for 0x0054a8e0: 16-byte aligned: YES
  0054a8e0  55             PUSH EBP
  0054a8e1  8B EC          MOV EBP,ESP
  0054a8e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054a8e0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x264 [dec 612]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0054a8e0 FUN_0054a8e0 size=25 ret=4 callers=1 callees=0

void __thiscall FUN_0054a8e0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x264) = param_2;
  return;
}
```

### decoy: 0x0054a900 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054a900: 16-byte aligned: YES
  0054a900  55             PUSH EBP
  0054a901  8B EC          MOV EBP,ESP
  0054a903  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054a900
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x264 [dec 612]  x1  via EAX
```

## Card B10a-2-16

GOG function: CSWSObject::TestActionList  gog 0x006a68f0  callconv=(blank)  param_size_bytes=0  gog_size=92  gog_retn=0
Candidate Steam address: 0x0053fc10 (size 92, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053fba0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a6880 CSWSObject::SetGroupInterruptable -> 0x0053fba0  gate_class=AUTO
- GOG-order neighbour after:  0x006a6950 CSWSObject::LoadObjectState -> 0x0053fc70  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x006a7980 CSWSObject::AddActionToFront -> Steam 0x00540ca0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x10*,0x100*} | Steam {0x10*,0x100*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053fc10: 16-byte aligned: YES
  0053fc10  55             PUSH EBP
  0053fc11  8B EC          MOV EBP,ESP
  0053fc13  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053fc10
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053fc10 FUN_0053fc10 size=92 ret=0 callers=2 callees=3

void __fastcall FUN_0053fc10(undefined4 param_1)

{
  undefined4 local_10;
  int local_c;
  int local_8;
  
  local_8 = 0;
  local_10 = FUN_007a1720(param_1);
  local_c = FUN_00521060();
  while (local_c != 0) {
    local_8 = local_8 + 1;
    local_c = FUN_0058c370(&local_10);
  }
  return;
}
```

### decoy: 0x0053fba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053fba0: 16-byte aligned: YES
  0053fba0  55             PUSH EBP
  0053fba1  8B EC          MOV EBP,ESP
  0053fba3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053fba0
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0x6C [dec 108]  x1  via EAX
  +0x70 [dec 112]  x1  via EAX
```

## Card B10a-2-17

GOG function: CSWCObject::TurnOffShadows  gog 0x00472660  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x007ed4d0 (size 42, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::TurnOnShadows | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed500:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: autolabel:NotifyPaneDivider
- GOG-order neighbour before: 0x00472630 CSWCObject::TurnOnShadows -> 0x007ed4d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00472690 CSWCObject::BarkString -> 0x007ed530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68*,0xbc} | Steam {0x68*,0xb8}
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

### decoy: 0x007ed500 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed500: 16-byte aligned: YES
  007ed500  55             PUSH EBP
  007ed501  8B EC          MOV EBP,ESP
  007ed503  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed500
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xBC [dec 188]  x1  via EDX
```

## Card B10a-2-18

GOG function: CSWSObject::DoDamage  gog 0x006a80e0  callconv=(blank)  param_size_bytes=4  gog_size=46  gog_retn=4
Candidate Steam address: 0x00541400 (size 46, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: SetCurrentHitPoints 0x54a610; callers: DoDamage 0x57f450)
- best decoy (neighbour/other candidate) scores: 0x005413c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a80a0 CSWSObject::GetCurrentHitPoints -> 0x005413c0  gate_class=AUTO
- GOG-order neighbour after:  0x006a8110 CSWSObject::DoDamageReduction -> 0x00541430  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006b12f0 CSWSObject::SetCurrentHitPoints -> Steam 0x0054a610  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00697f80 CSWSCreature::DoDamage -> Steam 0x0057f450  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x9c*} | Steam {0x9c*}
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
prologue for 0x00541400: 16-byte aligned: YES
  00541400  55             PUSH EBP
  00541401  8B EC          MOV EBP,ESP
  00541403  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00541400
string literals (0):
direct callees (1):
  0x0054A610  CSWSObject::SetCurrentHitPoints
struct offsets touched (top 1):
  +0x9C [dec 156]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00541400 FUN_00541400 size=46 ret=4 callers=2 callees=1

void __thiscall FUN_00541400(int *param_1,int param_2)

{
  short sVar1;
  
  sVar1 = (**(code **)(*param_1 + 0x9c))(1);
  FUN_0054a610(sVar1 - param_2);
  return;
}
```

### decoy: 0x005413c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005413c0: 16-byte aligned: YES
  005413c0  55             PUSH EBP
  005413c1  8B EC          MOV EBP,ESP
  005413c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005413c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xE0 [dec 224]  x2  via EAX,ECX
  +0xE8 [dec 232]  x1  via EDX
```

## Card B10a-2-19

GOG function: CSWCObject::Hilite  gog 0x004754a0  callconv=(blank)  param_size_bytes=5  gog_size=50  gog_retn=16
Candidate Steam address: 0x007efac0 (size 372, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCObject::GetTopFireForgetAnimation | G7 mac: pass(0.38) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef9a0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00475460 CSWCObject::AbortFireForgetAnimation -> 0x007f0300  gate_class=AUTO
- GOG-order neighbour after:  0x004754e0 CSWCObject::UnHilite -> 0x007f0380  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004c92d0 CSWCAnimBase::Hilite -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x68} | Steam {0x4,0x8,0xc,0x10,0x14,0x18,0x5c}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*,0x32,0x38,0x1f4,0x2710}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 5 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x007efac0: 16-byte aligned: YES
  007efac0  55             PUSH EBP
  007efac1  8B EC          MOV EBP,ESP
  007efac3  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007efac0
string literals (0):
direct callees (4):
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051ACD0  (unnamed)
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0051AFE0  CWorldTimer::CompareWorldTimes
struct offsets touched (top 7):
  +0x5C [dec 92]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007efac0 FUN_007efac0 size=372 ret=16 callers=4 callees=4

undefined4 __thiscall
FUN_007efac0(int param_1,undefined2 *param_2,float *param_3,int *param_4,undefined4 *param_5)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  undefined4 local_20;
  undefined4 local_1c[2 = byte +0x8];
  undefined4 local_14;
  int local_10;
  undefined4 local_c;
  int local_8;
  
  uVar2 = *(undefined4 *)(*(int *)(param_1 + 0x5c) + 8);
  uVar5 = *(undefined4 *)(*(int *)(param_1 + 0x5c) + 0xc);
  local_14 = uVar5;
  local_c = uVar2;
  FUN_0073f3b0(uVar2,uVar5);
  local_10 = FUN_0051acd0(uVar2,uVar5);
  if (**(int **)(param_1 + 0x5c) == 0) {
    *param_2 = 10000;
    *param_5 = 0;
    *param_3 = 1.0;
    *param_4 = 500;
    uVar2 = 0;
  }
  else {
    if (local_10 == 0) {
      puVar6 = local_1c;
      puVar4 = &local_20;
      FUN_0073f3b0(puVar4,puVar6);
      FUN_0051ad20(puVar4,puVar6);
      uVar2 = local_c;
      uVar5 = local_14;
      FUN_0073f3b0(local_20,local_1c[0],local_c,local_14);
      iVar3 = FUN_0051afe0(local_20,local_1c[0],uVar2,uVar5);
      if (iVar3 == -1) {
        *param_2 = 10000;
        *param_5 = 0;
        *param_3 = 1.0;
        *param_4 = 500;
        return 0;
      }
    }
    *param_2 = *(undefined2 *)(*(int *)(param_1 + 0x5c) + 4);
    *param_5 = *(undefined4 *)(*(int *)(param_1 + 0x5c) + 0x18);
    uVar1 = *(uint *)(*(int *)(param_1 + 0x5c) + 0x10);
    local_8 = uVar1 - local_10;
    if (local_8 < 0x32) {
      local_8 = 0x32;
    }
    *param_4 = local_8;
    *param_3 = ((float)uVar1 * *(float *)(*(int *)(param_1 + 0x5c) + 0x14)) / (float)local_8;
    uVar2 = 1;
  }
  return uVar2;
}
```

### decoy: 0x007ef9a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef9a0: 16-byte aligned: YES
  007ef9a0  55             PUSH EBP
  007ef9a1  8B EC          MOV EBP,ESP
  007ef9a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef9a0
string literals (0):
direct callees (4):
  0x0087BBD0  CSWCDoor::AddFireAndForgetAnimation
  0x00841C70  CSWCPlaceable::AddFireAndForgetAnimation
  0x0087F3A0  (unnamed)
  0x007EFC40  CSWCObject::InsertFireForgetAnimation
struct offsets touched (top 5):
  +0x14 [dec 20]  x2  via EDX
  +0x2C [dec 44]  x2  via EDX
  +0x3C [dec 60]  x2  via EDX
  +0x4C [dec 76]  x2  via EDX
  +0xA0 [dec 160]  x1  via EDX
```

## Card B10a-2-20

GOG function: CSWSObject::SetAnimation  gog 0x006a8070  callconv=(blank)  param_size_bytes=4  gog_size=35  gog_retn=4
Candidate Steam address: 0x00541390 (size 35, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: Constructor 0x61dcf0)
- best decoy (neighbour/other candidate) scores: 0x005413c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a7d60 CSWSObject::ClearAllActions -> 0x00541080  gate_class=AUTO
- GOG-order neighbour after:  0x006a80a0 CSWSObject::GetCurrentHitPoints -> 0x005413c0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006b1110 CSWSObject::SetDirtyFlag -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0075cf80 CSWSTrigger::Constructor -> Steam 0x0061dcf0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xd8*} | Steam {0xd8*}
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
prologue for 0x00541390: 16-byte aligned: YES
  00541390  55             PUSH EBP
  00541391  8B EC          MOV EBP,ESP
  00541393  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00541390
string literals (0):
direct callees (1):
  0x0054A430  CSWSObject::SetDirtyFlag
struct offsets touched (top 1):
  +0xD8 [dec 216]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00541390 FUN_00541390 size=35 ret=4 callers=1 callees=1

void __thiscall FUN_00541390(int param_1,undefined4 param_2)

{
  FUN_0054a430(0);
  *(undefined4 *)(param_1 + 0xd8) = param_2;
  return;
}
```

### decoy: 0x005413c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005413c0: 16-byte aligned: YES
  005413c0  55             PUSH EBP
  005413c1  8B EC          MOV EBP,ESP
  005413c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005413c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xE0 [dec 224]  x2  via EAX,ECX
  +0xE8 [dec 232]  x1  via EDX
```

## Card B10a-2-21

GOG function: CSWSObject::ClearDirtyFlag  gog 0x006b11b0  callconv=(blank)  param_size_bytes=4  gog_size=79  gog_retn=4
Candidate Steam address: 0x0054a4d0 (size 79, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: UpdateState 0x51d790, ExecuteCommandBeginConversation 0x66df70)
- best decoy (neighbour/other candidate) scores: 0x0054a480:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b1160 CSWSObject::IsDirtyFlagSet -> 0x0054a480  gate_class=REVIEW
- GOG-order neighbour after:  0x006b1200 CSWSObject::ClearAllEffects -> 0x0054a520  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0064d1b0 CServerAIMaster::UpdateState -> Steam 0x0051d790  in candidate callers: yes
  0x00783550 Global::ExecuteCommandBeginConversation -> Steam 0x0066df70  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x224*} | Steam {0x224*}
imm sets (* = in both, 3 common): GOG {0x4*,0x5*,0xc*} | Steam {0x4*,0x5*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0054a4d0: 16-byte aligned: YES
  0054a4d0  55             PUSH EBP
  0054a4d1  8B EC          MOV EBP,ESP
  0054a4d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054a4d0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0054a4d0 FUN_0054a4d0 size=79 ret=4 callers=2 callees=0

void __thiscall FUN_0054a4d0(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x224 + (param_2 >> 5) * 4) =
       -(1 << ((byte)param_2 & 0x1f)) - 1U & *(uint *)(param_1 + 0x224 + (param_2 >> 5) * 4);
  return;
}
```

### decoy: 0x0054a480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054a480: 16-byte aligned: YES
  0054a480  55             PUSH EBP
  0054a481  8B EC          MOV EBP,ESP
  0054a483  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054a480
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B10a-2-22

GOG function: CSWCObject::AnimationAttack  gog 0x004749e0  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef7f0 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::AnimationParry | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef880:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474950 CSWCObject::AnimationParry -> 0x007ef7f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00474a70 CSWCObject::AnimationRunning -> 0x007ef470  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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

## Card B10a-2-23

GOG function: CSWSObject::AddMatchedExpressionString  gog 0x006adc90  callconv=(blank)  param_size_bytes=4  gog_size=144  gog_retn=4
Candidate Steam address: 0x00546fb0 (size 144, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00547040:STR0/CALLEE3/SIG0.375/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006adbf0 CSWSObject::TestListenExpression -> 0x00546f10  gate_class=AUTO
- GOG-order neighbour after:  0x006add20 CSWSObject::ClearMatchedExpressionStrings -> 0x00547040  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00687b00 CSWSCreature::EventHandler -> Steam 0x0056efd0  in candidate callers: yes
  0x00762ff0 CSWSDoor::EventHandler -> Steam 0x00617b40  in candidate callers: yes
  0x0076d3b0 CSWSPlaceable::EventHandler -> Steam 0x005f2fe0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x14*,0x1c4*} | Steam {0x4*,0x8*,0x14*,0x1c4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00546fb0: 16-byte aligned: YES
  00546fb0  55             PUSH EBP
  00546fb1  8B EC          MOV EBP,ESP
  00546fb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00546fb0
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x007335F0  CExoString::CExoString_copy
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00546fb0 FUN_00546fb0 size=144 ret=4 callers=3 callees=3

void FUN_00546fb0(undefined4 param_1)

{
  int iVar1;
  undefined4 local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00949bbb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(8,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_24 = 0;
  }
  else {
    local_24 = FUN_007335f0(param_1);
  }
  local_8 = 0xffffffff;
  FUN_0083ea60(local_24);
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00547040 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00547040: 16-byte aligned: YES
  00547040  55             PUSH EBP
  00547041  8B EC          MOV EBP,ESP
  00547043  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00547040
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
  0x00840EF0  (unnamed)
struct offsets touched (top 2):
  +0x1C4 [dec 452]  x3  via EAX,EDX
  +0x1C8 [dec 456]  x1  via ECX
```

## Card B10a-2-24

GOG function: CSWSObject::GetLastName  gog 0x006a5ce0  callconv=__thiscall  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x0053ef50 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053f050:STR0/CALLEE0/SIG0.025/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a55e0 CSWSObject::Constructor -> 0x0053e830  gate_class=REVIEW
- GOG-order neighbour after:  0x006a5d00 CSWSObject::Destructor -> 0x0053f020  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053ef50: 16-byte aligned: YES
  0053ef50  55             PUSH EBP
  0053ef51  8B EC          MOV EBP,ESP
  0053ef53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053ef50
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053ef50 FUN_0053ef50 size=17 ret=0 callers=0 callees=0

int __fastcall FUN_0053ef50(int param_1)

{
  return param_1 + 0xc;
}
```

### decoy: 0x0053f050 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053f050: 16-byte aligned: YES
  0053f050  55             PUSH EBP
  0053f051  8B EC          MOV EBP,ESP
  0053f053  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0053f050
string literals (0):
direct callees (16):
  0x006A2710  (unnamed)
  0x009196FD  operator delete
  0x00547040  CSWSObject::ClearMatchedExpressionStrings
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DEA0  (unnamed)
  0x005E4CA0  CGameEffect::Destructor
  0x00840EF0  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x0053E740  Action-list free params
  0x007F2630  CSWCObject::DetachFromServerObject
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x005E6DE0  CSWSScriptVarTable::Destructor
  0x0072F5F0  (unnamed)
  0x00735360  CExoLocString::Destructor
struct offsets touched (top 8):
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x18C [dec 396]  x3  via EAX,EDX
  +0x1D0 [dec 464]  x3  via EAX,ECX
  +0x264 [dec 612]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EDX
  +0x1B0 [dec 432]  x2  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x1B4 [dec 436]  x1  via EDX
```

## Card B10a-2-25

GOG function: CSWSObject::SaveObjectState  gog 0x006a69c0  callconv=(blank)  param_size_bytes=8  gog_size=97  gog_retn=8
Candidate Steam address: 0x0053fce0 (size 97, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 11 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 11 (callees: WriteFieldBYTE 0x7199d0; callers: SaveCreature 0x574500, SaveItems 0x52a730, SaveDoors 0x52a840)
- best decoy (neighbour/other candidate) scores: 0x0053fc70:STR1/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a6950 CSWSObject::LoadObjectState -> 0x0053fc70  gate_class=AUTO
- GOG-order neighbour after:  0x006a6a30 CSWSObject::LoadActionQueue -> 0x0053fd50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00625b30 CResGFF::WriteFieldBYTE -> Steam 0x007199d0  in candidate callees: yes
  0x006a70f0 CSWSObject::SaveActionQueue -> Steam ?  in candidate callees: n/a
  0x006a7380 unnamed -> Steam ?  in candidate callees: n/a
  0x006a7540 CSWSObject::SaveEffectList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (10 of 10): the card's A-CALLEE credit may come from these, not from callees
  0x0068d030 CSWSCreature::SaveCreature -> Steam 0x00574500  in candidate callers: yes
  0x0071f8e0 CSWSArea::SaveItems -> Steam 0x0052a730  in candidate callers: yes
  0x0071f9f0 CSWSArea::SaveDoors -> Steam 0x0052a840  in candidate callers: yes
  0x0071fb00 CSWSArea::SaveTriggers -> Steam 0x0052a950  in candidate callers: yes
GOG strings (1): 'Commandable'
disp sets (* = in both, 1 common): GOG {0xec*} | Steam {0xec*}
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
prologue for 0x0053fce0: 16-byte aligned: YES
  0053fce0  55             PUSH EBP
  0053fce1  8B EC          MOV EBP,ESP
  0053fce3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053fce0
string literals (1):
  0x009945EC  at 0053fd25  'Commandable'
direct callees (4):
  0x00540860  CSWSObject::SaveEffectList
  0x005406A0  (unnamed)
  0x00540410  CSWSObject::SaveActionQueue
  0x007199D0  CResGFF::WriteFieldBYTE
struct offsets touched (top 1):
  +0xEC [dec 236]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0053fce0 FUN_0053fce0 size=97 ret=8 callers=10 callees=4

void __thiscall FUN_0053fce0(int param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_00540860(param_2,param_3);
  FUN_005406a0(param_2,param_3);
  FUN_00540410(param_2,param_3);
  FUN_007199d0(param_3,*(uint *)(param_1 + 0xec) & 0xff,"Commandable");
  return;
}
```

### decoy: 0x0053fc70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053fc70: 16-byte aligned: YES
  0053fc70  55             PUSH EBP
  0053fc71  8B EC          MOV EBP,ESP
  0053fc73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053fc70
string literals (1):
  0x009945EC  at 0053fcaf  'Commandable'
direct callees (4):
  0x005406E0  CSWSObject::LoadEffectList
  0x00540660  (unnamed)
  0x0053FD50  CSWSObject::LoadActionQueue
  0x00718A00  CResGFF::ReadFieldBYTE
struct offsets touched (top 1):
  +0xEC [dec 236]  x1  via EAX
```

## Card B10a-2-26

GOG function: CSWSObject::GetNumActionGroups  gog 0x006a62c0  callconv=(blank)  param_size_bytes=0  gog_size=177  gog_retn=0
Candidate Steam address: 0x0053f5e0 (size 177, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0053f4a0:STR0/CALLEE1/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a6240 CSWSObject::GetNodeById -> 0x0053f560  gate_class=AUTO
- GOG-order neighbour after:  0x006a6380 CSWSObject::RemoveGroup -> 0x0053f6a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0069f2c0 CSWSCreature::ClearHostileActionsVersus -> Steam 0x005867e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x6c*} | Steam {0x6c*}
imm sets (* = in both, 2 common): GOG {0x14*,0x100*} | Steam {0x14*,0x100*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053f5e0: 16-byte aligned: YES
  0053f5e0  55             PUSH EBP
  0053f5e1  8B EC          MOV EBP,ESP
  0053f5e3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053f5e0
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0053f5e0 FUN_0053f5e0 size=177 ret=0 callers=1 callees=4

uint __fastcall FUN_0053f5e0(undefined4 param_1)

{
  int iVar1;
  int local_14;
  uint local_10;
  int local_c;
  short local_8;
  
  iVar1 = FUN_007a1780(param_1);
  if (iVar1 == 1) {
    local_10 = 0;
  }
  else {
    local_c = FUN_007a1720();
    local_14 = FUN_00561430(local_c);
    local_10 = (uint)(local_14 != 0);
    local_8 = *(short *)(local_14 + 0x6c);
    while (local_c != 0) {
      if (*(short *)(local_14 + 0x6c) != local_8) {
        local_8 = *(short *)(local_14 + 0x6c);
        local_10 = local_10 + 1;
      }
      local_14 = FUN_0058c370(&local_c);
    }
  }
  return local_10;
}
```

### decoy: 0x0053f4a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053f4a0: 16-byte aligned: YES
  0053f4a0  55             PUSH EBP
  0053f4a1  8B EC          MOV EBP,ESP
  0053f4a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053f4a0
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x3  via EAX,ECX
```

## Card B10a-2-27

GOG function: CSWSObject::SetDirtyFlag  gog 0x006b1110  callconv=(blank)  param_size_bytes=4  gog_size=74  gog_retn=4
Candidate Steam address: 0x0054a430 (size 74, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: SetAnimation 0x56d350, SetAnimation 0x541390)
- best decoy (neighbour/other candidate) scores: 0x0054a480:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b10c0 CSWSObject::AnimationWalking -> 0x0054a3e0  gate_class=AUTO
- GOG-order neighbour after:  0x006b1160 CSWSObject::IsDirtyFlagSet -> 0x0054a480  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x00685f80 CSWSCreature::SetAnimation -> Steam 0x0056d350  in candidate callers: yes
  0x006a8070 CSWSObject::SetAnimation -> Steam 0x00541390  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x224*} | Steam {0x224*}
imm sets (* = in both, 3 common): GOG {0x4*,0x5*,0xc*} | Steam {0x4*,0x5*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0054a430: 16-byte aligned: YES
  0054a430  55             PUSH EBP
  0054a431  8B EC          MOV EBP,ESP
  0054a433  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054a430
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0054a430 FUN_0054a430 size=74 ret=4 callers=4 callees=0

void __thiscall FUN_0054a430(int param_1,uint param_2)

{
  *(uint *)(param_1 + 0x224 + (param_2 >> 5) * 4) =
       1 << ((byte)param_2 & 0x1f) | *(uint *)(param_1 + 0x224 + (param_2 >> 5) * 4);
  return;
}
```

### decoy: 0x0054a480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054a480: 16-byte aligned: YES
  0054a480  55             PUSH EBP
  0054a481  8B EC          MOV EBP,ESP
  0054a483  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054a480
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B10a-2-28

GOG function: CSWSObject::GetAcceptableAction  gog 0x006b3380  callconv=(blank)  param_size_bytes=4  gog_size=62  gog_retn=4
Candidate Steam address: 0x005d7190 (size 62, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005d7220:STR0/CALLEE1/SIG0.043/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b18a0 CSWSObject::RunActions -> 0x005d56b0  gate_class=AUTO
- GOG-order neighbour after:  0x006b3410 CSWSObject::GetAQActionIDByID -> 0x005d7220  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007906e0 Global::ExecuteCommandGetCurrentAction -> Steam 0x0067b100  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x46*} | Steam {0x4*,0x8*,0x46*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005d7190: 16-byte aligned: YES
  005d7190  55             PUSH EBP
  005d7191  8B EC          MOV EBP,ESP
  005d7193  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d7190
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x5D71D8 [dec 6123992]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005d7190 FUN_005d7190 size=62 ret=4 callers=2 callees=0

undefined4 FUN_005d7190(undefined4 param_1)

{
  undefined4 uVar1;
  
  switch(param_1) {
  case 1:
  case 7:
  case 9:
  case 0xc:
  case 0xf:
  case 0x14:
  case 0x15:
  case 0x18:
  case 0x19:
  case 0x1a:
  case 0x1b:
  case 0x1c:
  case 0x1d:
  case 0x1e:
  case 0x26:
  case 0x27:
  case 0x28:
  case 0x29:
  case 0x2a:
  case 0x2b:
  case 0x2e:
  case 0x32:
  case 0x36:
  case 0x37:
  case 0x38:
  case 0x3d:
  case 0x3f:
  case 0x43:
  case 0x44:
  case 0x45:
  case 0x46:
  case 0x47:
    uVar1 = 1;
    break;
  default:
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x005d7220 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d7220: 16-byte aligned: YES
  005d7220  55             PUSH EBP
  005d7221  8B EC          MOV EBP,ESP
  005d7223  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d7220
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x5D7434 [dec 6124596]  x1  via EAX
```

## Card B10a-2-29

GOG function: CSWSObject::Destructor  gog 0x006a5d00  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0053f020 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053ef70:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a5ce0 CSWSObject::GetLastName -> 0x0053ef50  gate_class=REVIEW
- GOG-order neighbour after:  0x006a5d30 CSWSObject::Destructor_2 -> 0x0053f050  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006a5d30 CSWSObject::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0053f020: 16-byte aligned: YES
  0053f020  55             PUSH EBP
  0053f021  8B EC          MOV EBP,ESP
  0053f023  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053f020
string literals (0):
direct callees (2):
  0x0053F050  CSWSObject::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053f020 FUN_0053f020 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0053f020(undefined4 param_1,uint param_2)

{
  FUN_0053f050();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0053ef70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053ef70: 16-byte aligned: YES
  0053ef70  55             PUSH EBP
  0053ef71  8B EC          MOV EBP,ESP
  0053ef73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053ef70
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE4 [dec 228]  x1  via EAX
```

## Card B10a-2-30

GOG function: CSWSObject::SetDialogOwner  gog 0x006ada90  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00546db0 (size 22, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00546dd0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006ad8b0 CSWSObject::GetNearestObjectByName -> 0x00546bd0  gate_class=AUTO
- GOG-order neighbour after:  0x006adb20 CSWSObject::SetListenExpression -> 0x00546e40  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x54*} | Steam {0x54*}
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
prologue for 0x00546db0: 16-byte aligned: YES
  00546db0  55             PUSH EBP
  00546db1  8B EC          MOV EBP,ESP
  00546db3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00546db0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x54 [dec 84]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00546db0 FUN_00546db0 size=22 ret=4 callers=3 callees=0

void __thiscall FUN_00546db0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x54) = param_2;
  return;
}
```

### decoy: 0x00546dd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00546dd0: 16-byte aligned: YES
  00546dd0  55             PUSH EBP
  00546dd1  8B EC          MOV EBP,ESP
  00546dd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00546dd0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1B0 [dec 432]  x2  via EAX
  +0x1B4 [dec 436]  x1  via ECX
```

## Card B10a-2-31

GOG function: CSWSObject::GetPositionByGroupIndex  gog 0x006a6180  callconv=(blank)  param_size_bytes=4  gog_size=187  gog_retn=4
Candidate Steam address: 0x0053f4a0 (size 187, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0053f3c0:STR0/CALLEE2/SIG0.7/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a60a0 CSWSObject::GetActionByGroupId -> 0x0053f3c0  gate_class=REVIEW
- GOG-order neighbour after:  0x006a6240 CSWSObject::GetNodeById -> 0x0053f560  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0069f2c0 CSWSCreature::ClearHostileActionsVersus -> Steam 0x005867e0  in candidate callers: yes
  0x006a19b0 CSWSCreature::UpdateActionQueue -> Steam 0x00588ed0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x6c*} | Steam {0x6c*}
imm sets (* = in both, 3 common): GOG {0x4*,0x14*,0x100*} | Steam {0x4*,0x14*,0x100*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0053f4a0: 16-byte aligned: YES
  0053f4a0  55             PUSH EBP
  0053f4a1  8B EC          MOV EBP,ESP
  0053f4a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053f4a0
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0053f4a0 FUN_0053f4a0 size=187 ret=4 callers=2 callees=4

undefined4 __thiscall FUN_0053f4a0(undefined4 param_1,int param_2)

{
  int iVar1;
  int local_14;
  int local_10;
  undefined4 local_c;
  short local_8;
  
  local_14 = 0;
  iVar1 = FUN_007a1780(param_1);
  if (iVar1 == 1) {
    local_c = 0;
  }
  else {
    local_c = FUN_007a1720();
    local_10 = FUN_00561430(local_c);
    local_8 = *(short *)(local_10 + 0x6c);
    if (param_2 != 0) {
      while (local_10 != 0) {
        if (*(short *)(local_10 + 0x6c) != local_8) {
          local_8 = *(short *)(local_10 + 0x6c);
          local_14 = local_14 + 1;
        }
        if (local_14 == param_2) {
          return local_c;
        }
        local_10 = FUN_0058c370(&local_c);
      }
      local_c = 0;
    }
  }
  return local_c;
}
```

### decoy: 0x0053f3c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053f3c0: 16-byte aligned: YES
  0053f3c0  55             PUSH EBP
  0053f3c1  8B EC          MOV EBP,ESP
  0053f3c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053f3c0
string literals (0):
direct callees (5):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x005D7190  CSWSObject::GetAcceptableAction
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x1  via EAX
```
