# Review cards B0 chunk 1

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-1-01

GOG function: CSWCArea::Destructor  gog 0x0044f680  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x007a2c80 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2cb0:STR0/CALLEE0/SIG0.02/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044f660 CSWCArea::non_virtual_thunk_to_AsSWCArea -> 0x007a2c60  gate_class=REVIEW
- GOG-order neighbour after:  0x0044f6b0 CSWCArea::Destructor_2 -> 0x007a2cb0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0044f6b0 CSWCArea::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x007a2c80: 16-byte aligned: YES
  007a2c80  55             PUSH EBP
  007a2c81  8B EC          MOV EBP,ESP
  007a2c83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2c80
string literals (0):
direct callees (2):
  0x007A2CB0  CSWCArea::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a2c80 FUN_007a2c80 size=44 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_007a2c80(undefined4 param_1,uint param_2)

{
  FUN_007a2cb0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x007a2cb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2cb0: 16-byte aligned: YES
  007a2cb0  55             PUSH EBP
  007a2cb1  8B EC          MOV EBP,ESP
  007a2cb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007a2cb0
string literals (0):
direct callees (9):
  0x007A5FA0  CSWCArea::UnloadArea
  0x00890040  CSWCAmbientSound dtor
  0x009196FD  operator delete
  0x0052DC30  CSWSArea::DetachFromClientArea
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x0072F5F0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x005E7970  (unnamed)
struct offsets touched (top 8):
  +0x280 [dec 640]  x3  via EAX,ECX,EDX
  +0x104 [dec 260]  x2  via EAX,ECX
  +0x1E0 [dec 480]  x2  via ECX
  +0x264 [dec 612]  x2  via EAX,EDX
  +0x268 [dec 616]  x2  via ECX,EDX
  +0x288 [dec 648]  x2  via ECX,EDX
  +0x4C [dec 76]  x1  via EDX
  +0x28C [dec 652]  x1  via EAX
```

## Card B0-1-02

GOG function: CClientExoApp::GetPausedByCombat  gog 0x0040d7f0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740330 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740350:STR0/CALLEE4/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d7d0 CClientExoApp::DismissInGameGUI -> 0x00740310  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d810 CClientExoApp::SetPausedByCombat  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448500 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740330: 16-byte aligned: YES
  00740330  55             PUSH EBP
  00740331  8B EC          MOV EBP,ESP
  00740333  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740330
string literals (0):
direct callees (1):
  0x0079BF00  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740330 FUN_00740330 size=22 ret=0 callers=23 callees=1
THUNK -> 0x0079BF00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740330(undefined4 param_1)

{
  FUN_0079bf00(param_1);
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

## Card B0-1-03

GOG function: CClientExoApp::DisplayMainMenu  gog 0x0040c6f0  callconv=__thiscall  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f230 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f510:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c6c0 CClientExoApp::SetCharacterFile  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040c710 CClientExoApp::UnloadModule  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0042db10 CClientExoAppInternal::DisplayMainMenu -> Steam ?  in candidate callees: n/a
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

## Card B0-1-04

GOG function: CClientExoApp::GetObjectName  gog 0x0040c5a0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073f0e0 (size 32, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f0b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c570 CClientExoApp::JournalUpdated  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040c5c0 CClientExoApp::AddFeedbackMessage  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043b150 CClientExoAppInternal::GetObjectName -> Steam ?  in candidate callees: n/a
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

## Card B0-1-05

GOG function: CClientExoApp::IsSoundPlayingInDialog  gog 0x0040d750  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740290 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740260:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d720 CClientExoApp::BarkString  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d770 CClientExoApp::StopSoundPlayingInDialog  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004482c0 CClientExoAppInternal::IsSoundPlayingInDialog -> Steam ?  in candidate callees: n/a
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

## Card B0-1-06

GOG function: CSWCObject::TurnOnShadows  gog 0x00472630  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x007ed500 (size 42, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCObject::TurnOffShadows
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed4d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: autolabel:NotifyPaneDivider
- GOG-order neighbour before: 0x00472610 CSWCObject::GetPortraitId -> 0x007ed4b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00472660 CSWCObject::TurnOffShadows -> 0x007ed500  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68*,0xb8} | Steam {0x68*,0xbc}
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

## Card B0-1-07

GOG function: CSWCObject::AnimationStationary  gog 0x00474420  callconv=(blank)  param_size_bytes=2  gog_size=132  gog_retn=4
Candidate Steam address: 0x007ef240 (size 116, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCObject::SetOrientation+name_held_elsewhere=0x007ef2c0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.16 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef2c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004743a0 CSWCObject::SetOrientation  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004744b0 CSWCObject::AnimationPlayOutOfPlace -> 0x007ef730  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68*,0xcc,0xe0,0x130} | Steam {0x4,0x8,0x50,0x64,0x68*,0x90}
imm sets (* = in both, 1 common): GOG {0x4*,0x10} | Steam {0x4*,0x8,0x14,0x30}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef240: 16-byte aligned: YES
  007ef240  55             PUSH EBP
  007ef241  8B EC          MOV EBP,ESP
  007ef243  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef240
string literals (0):
direct callees (1):
  0x005143C0  (unnamed)
struct offsets touched (top 6):
  +0x68 [dec 104]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EDX
  +0x64 [dec 100]  x1  via EDX
  +0x90 [dec 144]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef240 FUN_007ef240 size=116 ret=4 callers=0 callees=1

void __thiscall FUN_007ef240(int *param_1,undefined4 param_2)

{
  int *piVar1;
  undefined1 local_10 [12];
  
  piVar1 = (int *)FUN_005143c0(local_10,param_2);
  param_1[0xc = byte +0x30] = *piVar1;
  param_1[0xd = byte +0x34] = piVar1[1 = byte +0x4];
  param_1[0xe = byte +0x38] = piVar1[2 = byte +0x8];
  if ((param_1[0x1a = byte +0x68] != 0) && (param_1[0x14 = byte +0x50] == 0)) {
    (**(code **)(*(int *)param_1[0x1a = byte +0x68] + 0x64 [dec 100]))(param_2);
  }
  (**(code **)(*param_1 + 0x90))();
  return;
}
```

### decoy: 0x007ef2c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef2c0: 16-byte aligned: YES
  007ef2c0  55             PUSH EBP
  007ef2c1  8B EC          MOV EBP,ESP
  007ef2c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B0-1-08

GOG function: CClientExoApp::GetInGameGui  gog 0x0040cc10  callconv=(blank)  param_size_bytes=0  gog_size=28  gog_retn=0
Candidate Steam address: 0x0073f750 (size 28, retn 0)
Candidate sources: branch+mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=ClientApp::GetGuiInGame
- anchors: A-STR 0 | A-CALLEE 14 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 14 (MainLoop 0x781be0,AnimationDialog 0x7ef780,ShowGalaxyMapGui 0x7ca730); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073f810:STR0/CALLEE3/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cbe0 CClientExoApp::SendLoadGameRequest -> 0x0073f720  gate_class=REVIEW
- GOG-order neighbour after:  0x0040cc30 CClientExoApp::GetPlayModulePanel  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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

## Card B0-1-09

GOG function: CSWCArea::Unknown_ReturnTrue_7Args  gog 0x004550b0  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=28
Candidate Steam address: 0x007a86b0 (size 18, retn 28)
Candidate sources: branch+name_map:high:batch5_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a85a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00454fa0 CSWCArea::SetWind -> 0x007a85a0  gate_class=AUTO
- GOG-order neighbour after:  0x004550d0 CSWCArea::UpdateDayNightCycle -> 0x007a86d0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=28 ; asm RET = 28
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 28)
```

### prologue
```
prologue for 0x007a86b0: 16-byte aligned: YES
  007a86b0  55             PUSH EBP
  007a86b1  8B EC          MOV EBP,ESP
  007a86b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a86b0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a86b0 FUN_007a86b0 size=18 ret=28 callers=2 callees=0

undefined4 FUN_007a86b0(void)

{
  return 1;
}
```

### decoy: 0x007a85a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a85a0: 16-byte aligned: YES
  007a85a0  55             PUSH EBP
  007a85a1  8B EC          MOV EBP,ESP
  007a85a3  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a85a0
string literals (0):
direct callees (4):
  0x005143C0  (unnamed)
  0x00436200  (unnamed)
  0x004724B0  (unnamed)
  0x00472530  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xAB [dec 171]  x1  via ECX
```

## Card B0-1-10

GOG function: CClientExoApp::SetCombatMode  gog 0x0040dd40  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740880 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740820:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dd20 CClientExoApp::GetCombatMode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040dd60 CClientExoApp::PlayerFlourishWeapons -> 0x007408a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449ce0 CClientExoAppInternal::SetCombatMode -> Steam ?  in candidate callees: n/a
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

## Card B0-1-11

GOG function: CClientExoApp::SendSaveGameRequest  gog 0x0040cbb0  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073f6f0 (size 36, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f720:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cb90 CClientExoApp::GetActiveTimer  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040cbe0 CClientExoApp::SendLoadGameRequest -> 0x0073f720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431c40 CClientExoAppInternal::SendSaveGameRequest -> Steam ?  in candidate callees: n/a
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

## Card B0-1-12

GOG function: CClientExoApp::QueryExitProgram  gog 0x0040d300  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fe20 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d2e0 CClientExoApp::GetActivePauseState -> 0x0073fe00  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d320 CClientExoApp::SetInputActive  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004431c0 CClientExoAppInternal::QueryExitProgram -> Steam ?  in candidate callees: n/a
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

## Card B0-1-13

GOG function: CClientExoApp::GetActivePauseState  gog 0x0040d2e0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fe00 (size 22, retn 0)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d2b0 CClientExoApp::SetStartMenuMusicOnMovieEnd  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d300 CClientExoApp::QueryExitProgram -> 0x0073fe20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00430a60 CClientExoAppInternal::GetActivePauseState -> Steam ?  in candidate callees: n/a
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

## Card B0-1-14

GOG function: CSWSEffectListHandler::OnApplyEnemyAttackBonus  gog 0x0073f0e0  callconv=(blank)  param_size_bytes=8  gog_size=152  gog_retn=12
Candidate Steam address: 0x0059c9b0 (size 153, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyArcaneSpellFailure
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059c890:STR0/CALLEE0/SIG0.35/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073ee40 CSWSEffectListHandler::OnApplyDeaf -> 0x0059c550  gate_class=REVIEW
- GOG-order neighbour after:  0x0073f180 CSWSEffectListHandler::OnRemoveEnemyAttackBonus -> 0x0059c890  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0xfc,0x107,0x1198*} | Steam {0x30*,0x128,0x384,0x1198*}
imm sets (* = in both, 1 common): GOG {0xc*,0x18} | Steam {0xc*,0x14}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059c9b0: 16-byte aligned: YES
  0059c9b0  55             PUSH EBP
  0059c9b1  8B EC          MOV EBP,ESP
  0059c9b3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059c9b0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x0040E770  (unnamed)
struct offsets touched (top 4):
  +0x30 [dec 48]  x4  via EDX
  +0x128 [dec 296]  x1  via EDX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059c9b0 FUN_0059c9b0 size=153 ret=12 callers=0 callees=2

undefined4 FUN_0059c9b0(int *param_1,int param_2)

{
  byte bVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 != 0) {
    iVar2 = (**(code **)(*param_1 + 0x30))();
    iVar2 = *(int *)(iVar2 + 0x1198);
    if (param_2 != 0) {
      iVar3 = (**(code **)(*param_1 + 0x30))();
      if (iVar3 != 0) {
        iVar3 = (**(code **)(*param_1 + 0x30))();
        *(undefined4 *)(iVar3 + 0x384 [dec 900]) = 1;
      }
      bVar1 = *(byte *)(iVar2 + 0x128);
      iVar2 = FUN_005e4e60(0);
      FUN_0040e770((uint)bVar1 + iVar2);
    }
  }
  return 0;
}
```

### decoy: 0x0059c890 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059c890: 16-byte aligned: YES
  0059c890  55             PUSH EBP
  0059c891  8B EC          MOV EBP,ESP
  0059c893  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059c890
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x107 [dec 263]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-1-15

GOG function: CClientExoApp::SetInputClass  gog 0x0040d3c0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073fee0 (size 32, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 17 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 17 (ShowGalaxyMapGui 0x7ca730,HideGalaxyMapGui 0x7ca920,ShowItemCreateMenu 0x7caa10)
- best decoy (neighbour/other candidate) scores: 0x007400a0:STR0/CALLEE4/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d3a0 CClientExoApp::ServerToClientObjectId  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d3e0 CClientExoApp::GetInputClass -> 0x0073ff00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0045fc60 CClientExoAppInternal::SetInputClass -> Steam ?  in candidate callees: n/a
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

## Card B0-1-16

GOG function: CSWCArea::non_virtual_thunk_to_AsSWCArea  gog 0x0044f660  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x007a2c60 (size 19, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2590:STR0/CALLEE0/SIG0.014/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044f630 CGameObject::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044f680 CSWCArea::Destructor -> 0x007a2c80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x104*} | Steam {0x104*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
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

## Card B0-1-17

GOG function: CClientExoApp::GetCreatureByGameObjectID  gog 0x0040ca10  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f550 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f750:STR0/CALLEE1/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c9f0 CClientExoApp::GetItemByGameObjectID  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040ca30 CClientExoApp::GetAreaByGameObjectID  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004383d0 CClientExoAppInternal::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
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

## Card B0-1-18

GOG function: CClientExoApp::SetCanSendDialog  gog 0x0040d8e0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740420 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740400:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d8c0 CClientExoApp::SetLastPartyMemberTempKilled  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d900 CClientExoApp::GetCanSendDialog  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448b50 CClientExoAppInternal::SetCanSendDialog -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740420: 16-byte aligned: YES
  00740420  55             PUSH EBP
  00740421  8B EC          MOV EBP,ESP
  00740423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740420
string literals (0):
direct callees (1):
  0x0079C550  CClientExoAppInternal::SetCanSendDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740420 FUN_00740420 size=28 ret=4 callers=3 callees=1
THUNK -> 0x0079C550 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740420(undefined4 param_1)

{
  FUN_0079c550(param_1);
  return;
}
```

### decoy: 0x00740400 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740400: 16-byte aligned: YES
  00740400  55             PUSH EBP
  00740401  8B EC          MOV EBP,ESP
  00740403  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740400
string literals (0):
direct callees (1):
  0x0079C4E0  CClientExoAppInternal::SetLastPartyMemberTempKilled
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-1-19

GOG function: CSWCArea::Constructor  gog 0x0044ef30  callconv=__thiscall  param_size_bytes=0  gog_size=1741  gog_retn=0
Candidate Steam address: 0x007a2590 (size 1741, retn 0)
Candidate sources: branch+name_map:high:batch5_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007a23a0:STR0/CALLEE0/SIG0.026/SRV0/VT0
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
disp sets (* = in both, 37 common): GOG {0x4*,0x8*,0xa4*,0x104*,0x188*,0x18c*,0x190*,0x1a8*,0x1b8*,0x1bc*,0x1c0*,0x1c4*,0x1e0*,0x1e4*,...(+23)} | Steam {0x4*,0x8*,0xa4*,0x104*,0x188*,0x18c*,0x190*,0x1a8*,0x1b8*,0x1bc*,0x1c0*,0x1c4*,0x1e0*,0x1e4*,...(+23)}
imm sets (* = in both, 37 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0x90*,...(+23)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0x90*,...(+23)}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007a2590: 16-byte aligned: YES
  007a2590  55             PUSH EBP
  007a2591  8B EC          MOV EBP,ESP
  007a2593  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007a2590 FUN_007a2590 size=1741 ret=0 callers=1 callees=14

undefined4 * __fastcall FUN_007a2590(undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 local_a0;
  byte local_19;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00967f23;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_005e7750(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_0053d660(DAT_00994473,0x7f000000);
  local_8._0_1_ = 1;
  *param_1 = CSWCArea::vftable;
  param_1[0x41 = byte +0x104] = CSWCArea::vftable;
  FUN_007107f0();
  FUN_0084a640(0);
  local_8._0_1_ = 2;
  param_1[0x4b = byte +0x12c] = 0;
  param_1[0x4c = byte +0x130] = 0;
  param_1[0x4d = byte +0x134] = 0;
  param_1[0x4e = byte +0x138] = 0;
  param_1[0x4f = byte +0x13c] = 0;
  param_1[0x50 = byte +0x140] = 0;
  param_1[0x51 = byte +0x144] = 0;
  param_1[0x52 = byte +0x148] = 0;
  param_1[0x53 = byte +0x14c] = 0;
  param_1[0x54 = byte +0x150] = 0;
  param_1[0x55 = byte +0x154] = 0;
  param_1[0x56 = byte +0x158] = 0;
  param_1[0x57 = byte +0x15c] = 0;
  param_1[0x58 = byte +0x160] = 0;
  param_1[0x59 = byte +0x164] = 0;
  param_1[0x5a = byte +0x168] = 0;
  param_1[0x5b = byte +0x16c] = 0;
  param_1[0x5c = byte +0x170] = 0;
  param_1[0x5d = byte +0x174] = 0;
  param_1[0x5e = byte +0x178] = 0;
  param_1[0x5f = byte +0x17c] = 0;
  FUN_00733540();
  local_8._0_1_ = 3;
  FUN_0084a640(0);
  local_8._0_1_ = 4;
  FUN_0084a640(0);
  local_8._0_1_ = 5;
  FUN_00722870();
  local_8._0_1_ = 6;
  FUN_0084a640(0);
  local_8._0_1_ = 7;
  FUN_0084a640(0);
  local_8._0_1_ = 8;
  FUN_0084a640(0);
  local_8._0_1_ = 9;
  FUN_00428f80(0);
  local_8._0_1_ = 10;
  FUN_00449230(0);
  local_8._0_1_ = 0xb;
  FUN_00722870();
  local_8._0_1_ = 0xc;
  FUN_00449230(0);
  local_8._0_1_ = 0xd;
  FUN_007338d0(&DAT_00986925);
  param_1[0x62 = byte +0x188] = 0;
  param_1[99 = byte +0x18c] = 0;
  for (local_19 = 0; local_19 < 6; local_19 = local_19 + 1) {
    param_1[local_19 + 0x64 [dec 100]] = 0;
  }
  FUN_005e7a50();
  param_1[0x6a = byte +0x1a8] = 0;
  param_1[0x4b = byte +0x12c] = 0;
/* ... 66 more lines truncated by --max-lines */
```

### decoy: 0x007a23a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a23a0: 16-byte aligned: YES
  007a23a0  55             PUSH EBP
  007a23a1  8B EC          MOV EBP,ESP
  007a23a3  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a23a0
string literals (0):
direct callees (5):
  0x007A2270  (unnamed)
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x0051A940  (unnamed)
  0x0073F430  CClientExoApp::GetModule
  0x007F3640  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x8  via EAX,ECX
  +0x2C [dec 44]  x1  via EDX
  +0x74 [dec 116]  x1  via EAX
```

## Card B0-1-20

GOG function: CClientExoApp::AddMoveToModuleMovie  gog 0x0040d640  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740180 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740210:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d620 CClientExoApp::IsMoviePlaying  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d660 CClientExoApp::GetMoveToModuleMovies  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00445190 CClientExoAppInternal::AddMoveToModuleMovie -> Steam ?  in candidate callees: n/a
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

## Card B0-1-21

GOG function: CSWSEffectListHandler::OnApplyEnemyAttackBonus  gog 0x0073f0e0  callconv=(blank)  param_size_bytes=8  gog_size=152  gog_retn=12
Candidate Steam address: 0x0059c550 (size 658, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyDeaf
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059c7f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073ee40 CSWSEffectListHandler::OnApplyDeaf -> 0x0059c550  gate_class=REVIEW
- GOG-order neighbour after:  0x0073f180 CSWSEffectListHandler::OnRemoveEnemyAttackBonus -> 0x0059c890  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x30*,0xfc*,0x107,0x1198*} | Steam {0x4,0x8,0x18,0x30*,0xfc*,0x1198*}
imm sets (* = in both, 1 common): GOG {0xc*,0x18} | Steam {0x2,0x3,0x4,0x8,0xc*,0x14,0x17,0x19,0x34,0x60,0x8c,0xffffffef}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059c550: 16-byte aligned: YES
  0059c550  55             PUSH EBP
  0059c551  8B EC          MOV EBP,ESP
  0059c553  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059c550
string literals (0):
direct callees (10):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059c550 FUN_0059c550 size=658 ret=12 callers=0 callees=10

undefined4 FUN_0059c550(int *param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int local_70;
  int local_6c;
  undefined4 local_68;
  undefined4 local_64;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095210c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    iVar1 = (**(code **)(*param_1 + 0x30))();
    iVar2 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
    iVar3 = FUN_006bc570(8,iVar2);
    if (iVar3 != 0) {
      iVar1 = FUN_00919723(0x34);
      local_8 = 0;
      if (iVar1 == 0) {
        local_64 = 0;
      }
      else {
        local_64 = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      FUN_005e1c70(0,param_1[1 = byte +0x4]);
      if (iVar2 != 0) {
        iVar1 = FUN_00919723(0x34);
        local_8 = 1;
        if (iVar1 == 0) {
          local_68 = 0;
        }
        else {
          local_68 = FUN_005e2540();
        }
        local_8 = 0xffffffff;
        FUN_005e2be0(local_68);
        FUN_00584da0(0x8c,local_68);
      }
      uVar4 = 0x8c;
      (**(code **)(*param_1 + 0x30))(0x8c,local_64);
      FUN_00584da0(uVar4,local_64);
      ExceptionList = local_10;
      return 1;
    }
    if (*(int *)(iVar1 + 0xfc) != 0) {
      ExceptionList = local_10;
      return 1;
    }
    if (param_2 != 0) {
      iVar1 = FUN_00919723(0x8c);
      local_8 = 2;
      if (iVar1 == 0) {
        local_6c = 0;
      }
      else {
        local_6c = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
      iVar1 = FUN_00919723(0x8c);
      local_8 = 3;
      if (iVar1 == 0) {
        local_70 = 0;
      }
      else {
        local_70 = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
      *(undefined2 *)(local_6c + 8) = 0x17;
      FUN_005e4e80(0,0xffffffef);
/* ... 9 more lines truncated by --max-lines */
```

### decoy: 0x0059c7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059c7f0: 16-byte aligned: YES
  0059c7f0  55             PUSH EBP
  0059c7f1  8B EC          MOV EBP,ESP
  0059c7f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059c7f0
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 4):
  +0x30 [dec 48]  x3  via EDX
  +0x107 [dec 263]  x2  via EAX
  +0xFC [dec 252]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-1-22

GOG function: CClientExoApp::GetGameObject  gog 0x0040c990  callconv=__thiscall  param_size_bytes=4  gog_size=59  gog_retn=4
Candidate Steam address: 0x0073f4d0 (size 59, retn 4)
Candidate sources: branch+mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=ClientApp::GetClientObject
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 4 (ResetDialogAnimations 0x7c93e0,ResetTalkAnimations 0x7c9560,UpdateDialog 0x7cd0f0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0073f430:STR0/CALLEE1/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c970 CClientExoApp::GetGameObjectArray  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040c9d0 CClientExoApp::GetPlayerByGameObjectID  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x14*} | Steam {0x4*,0x14*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0073f4d0: 16-byte aligned: YES
  0073f4d0  55             PUSH EBP
  0073f4d1  8B EC          MOV EBP,ESP
  0073f4d3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f4d0
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f4d0 FUN_0073f4d0 size=59 ret=4 callers=96 callees=1

undefined4 FUN_0073f4d0(undefined4 param_1)

{
  undefined4 local_8;
  
  local_8 = 0;
  FUN_0053dfb0(param_1,&local_8);
  return local_8;
}
```

### decoy: 0x0073f430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f430: 16-byte aligned: YES
  0073f430  55             PUSH EBP
  0073f431  8B EC          MOV EBP,ESP
  0073f433  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f430
string literals (0):
direct callees (1):
  0x00726F80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-1-23

GOG function: CClientExoApp::ExitProgram  gog 0x0040c750  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f290 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f270:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c730 CClientExoApp::CreateModule  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040c770 CClientExoApp::GetCaptureEventFile  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431980 CClientExoAppInternal::ExitProgram -> Steam ?  in candidate callees: n/a
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

## Card B0-1-24

GOG function: CSWCObject::ShowShadowBlob  gog 0x00410c50  callconv=(blank)  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x00764f80 (size 44, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00764fb0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c30 CSWCObject::SetId -> 0x00764f60  gate_class=REVIEW
- GOG-order neighbour after:  0x00410c80 CSWCObject::HideShadowBlob -> 0x0078bc50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0xac*} | Steam {0x68*,0xac*}
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
prologue for 0x00764f80: 16-byte aligned: YES
  00764f80  55             PUSH EBP
  00764f81  8B EC          MOV EBP,ESP
  00764f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00764f80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xAC [dec 172]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00764f80 FUN_00764f80 size=44 ret=0 callers=0 callees=0

void __fastcall FUN_00764f80(int param_1)

{
  if (*(int *)(param_1 + 0x68) != 0) {
    (**(code **)(**(int **)(param_1 + 0x68) + 0xac))(0);
  }
  return;
}
```

### decoy: 0x00764fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764fb0: 16-byte aligned: YES
  00764fb0  55             PUSH EBP
  00764fb1  8B EC          MOV EBP,ESP
  00764fb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00764fb0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB0 [dec 176]  x1  via EDX
```

## Card B0-1-25

GOG function: CClientExoApp::DismissInGameGUI  gog 0x0040d7d0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740310 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007402f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d7b0 CClientExoApp::EnableInput -> 0x007402f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d7f0 CClientExoApp::GetPausedByCombat -> 0x00740330  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0045ffc0 CClientExoAppInternal::DismissInGameGUI -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740310: 16-byte aligned: YES
  00740310  55             PUSH EBP
  00740311  8B EC          MOV EBP,ESP
  00740313  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740310
string literals (0):
direct callees (1):
  0x007B33B0  CClientExoAppInternal::DismissInGameGUI
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740310 FUN_00740310 size=22 ret=0 callers=1 callees=1
THUNK -> 0x007B33B0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740310(undefined4 param_1)

{
  FUN_007b33b0(param_1);
  return;
}
```

### decoy: 0x007402f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007402f0: 16-byte aligned: YES
  007402f0  55             PUSH EBP
  007402f1  8B EC          MOV EBP,ESP
  007402f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007402f0
string literals (0):
direct callees (1):
  0x007B3390  CClientExoAppInternal::EnableInput
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-1-26

GOG function: CClientExoApp::SetPlayerCreature  gog 0x0040cdf0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f910 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f8d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cdd0 CClientExoApp::EmptyHitcheckIgnoreList  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040ce10 CClientExoApp::HandleMessage  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444c80 CClientExoAppInternal::SetPlayerCreature -> Steam ?  in candidate callees: n/a
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

## Card B0-1-27

GOG function: CClientExoApp::GetClientOptions  gog 0x0040ccf0  callconv=__thiscall  param_size_bytes=0  gog_size=28  gog_retn=0
Candidate Steam address: 0x0072fb00 (size 28, retn 0)
Candidate sources: branch    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=OptionsHolder::GetGameOptions
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (MainLoop 0x781be0,ShowStatusSummary 0x7cf270,OnPanelAdded 0x894320)
- best decoy (neighbour/other candidate) scores: 0x0073f430:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ccd0 CClientExoApp::GetSWCMessage  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040cd10 CClientExoApp::GetServerInfo  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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

### decoy: 0x0073f430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f430: 16-byte aligned: YES
  0073f430  55             PUSH EBP
  0073f431  8B EC          MOV EBP,ESP
  0073f433  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f430
string literals (0):
direct callees (1):
  0x00726F80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-1-28

GOG function: CClientExoApp::ChangeCharacter  gog 0x0040d840  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x00740380 (size 32, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007401f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d810 CClientExoApp::SetPausedByCombat  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d860 CClientExoApp::RunDeathSequence -> 0x007403a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444550 CClientExoAppInternal::ChangeCharacter -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740380: 16-byte aligned: YES
  00740380  55             PUSH EBP
  00740381  8B EC          MOV EBP,ESP
  00740383  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740380
string literals (0):
direct callees (1):
  0x00797F30  CClientExoAppInternal::ChangeCharacter
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00740380 FUN_00740380 size=32 ret=8 callers=6 callees=1
THUNK -> 0x00797F30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740380(undefined4 param_1,undefined4 param_2)

{
  FUN_00797f30(param_1,param_2);
  return;
}
```

### decoy: 0x007401f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007401f0: 16-byte aligned: YES
  007401f0  55             PUSH EBP
  007401f1  8B EC          MOV EBP,ESP
  007401f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007401f0
string literals (0):
direct callees (1):
  0x00796B50  SetHudTarget
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B0-1-29

GOG function: CSWCArea::StartWeather  gog 0x00456200  callconv=(blank)  param_size_bytes=1  gog_size=38  gog_retn=4
Candidate Steam address: 0x007a9800 (size 38, retn 4)
Candidate sources: branch+name_map:high:batch5_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a9830:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00456180 CSWCArea::UpdateSoundObjectsTimeOfDay -> 0x007a9780  gate_class=AUTO
- GOG-order neighbour after:  0x00456230 CSWCArea::StopWeather -> 0x007a9830  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1f0*,0x1f5*} | Steam {0x1f0*,0x1f5*}
imm sets (* = in both, 2 common): GOG {0x4*,0x1770*} | Steam {0x4*,0x1770*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
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

## Card B0-1-30

GOG function: CClientExoApp::SetGameOver  gog 0x0040d880  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x007403c0 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740400:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d860 CClientExoApp::RunDeathSequence -> 0x007403a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d8a0 CClientExoApp::GetGameOver -> 0x007403e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448830 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x007403c0: 16-byte aligned: YES
  007403c0  55             PUSH EBP
  007403c1  8B EC          MOV EBP,ESP
  007403c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007403c0
string literals (0):
direct callees (1):
  0x0079C230  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007403c0 FUN_007403c0 size=28 ret=4 callers=3 callees=1
THUNK -> 0x0079C230 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007403c0(undefined4 param_1)

{
  FUN_0079c230(param_1);
  return;
}
```

### decoy: 0x00740400 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740400: 16-byte aligned: YES
  00740400  55             PUSH EBP
  00740401  8B EC          MOV EBP,ESP
  00740403  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740400
string literals (0):
direct callees (1):
  0x0079C4E0  CClientExoAppInternal::SetLastPartyMemberTempKilled
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-1-31

GOG function: CClientExoApp::EnableInput  gog 0x0040d7b0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007402f0 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
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

## Card B0-1-32

GOG function: CClientExoApp::ShutDownToMainMenu  gog 0x0040d020  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fb40 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fb20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d000 CClientExoApp::StopServices  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d040 CClientExoApp::SetApplicationIdsMatch  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043a6c0 CClientExoAppInternal::ShutDownToMainMenu -> Steam ?  in candidate callees: n/a
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

## Card B0-1-33

GOG function: CSWCArea::SetAsDusk  gog 0x00455cc0  callconv=(blank)  param_size_bytes=0  gog_size=157  gog_retn=0
Candidate Steam address: 0x007a92c0 (size 157, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a9360:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004550d0 CSWCArea::UpdateDayNightCycle -> 0x007a86d0  gate_class=AUTO
- GOG-order neighbour after:  0x00455d60 CSWCArea::SetAsDawn -> 0x007a9360  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00455e00 CSWCArea::SetAsDay -> Steam ?  in candidate callees: n/a
  0x00455f20 CSWCArea::SetAsNight -> Steam ?  in candidate callees: n/a
  0x004571a0 CSWCArea::UpdateShadowingLights -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0xa0*,0xa4*,0xb4*,0x1b8*,0x1bc*,0x1c4*} | Steam {0xa0*,0xa4*,0xb4*,0x1b8*,0x1bc*,0x1c4*}
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
prologue for 0x007a92c0: 16-byte aligned: YES
  007a92c0  55             PUSH EBP
  007a92c1  8B EC          MOV EBP,ESP
  007a92c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a92c0
string literals (0):
direct callees (3):
  0x007A9520  CSWCArea::SetAsNight
  0x007A9400  CSWCArea::SetAsDay
  0x007AA7A0  CSWCArea::UpdateShadowingLights
struct offsets touched (top 6):
  +0xA4 [dec 164]  x2  via ECX
  +0xA0 [dec 160]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0x1B8 [dec 440]  x1  via EAX
  +0x1BC [dec 444]  x1  via EDX
  +0x1C4 [dec 452]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007a92c0 FUN_007a92c0 size=157 ret=0 callers=1 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_007a92c0(int param_1)

{
  if (*(int *)(param_1 + 0xa0) == 0) {
    if (*(int *)(param_1 + 0xa4) == 0) {
      FUN_007a9400();
    }
    else {
      FUN_007a9520();
    }
  }
  else {
    _DAT_009f6390 = _DAT_0098d930;
    _DAT_009f5d90 = (float)*(byte *)(param_1 + 0xb4) / (float)_DAT_0098ba38;
    *(undefined4 *)(param_1 + 0x1c4) = 1;
    *(undefined4 *)(param_1 + 0x1bc) = 1;
    *(undefined4 *)(param_1 + 0x1b8) = 0;
    *(undefined4 *)(param_1 + 0xa4) = 0;
    FUN_007aa7a0();
  }
  return;
}
```

### decoy: 0x007a9360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a9360: 16-byte aligned: YES
  007a9360  55             PUSH EBP
  007a9361  8B EC          MOV EBP,ESP
  007a9363  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a9360
string literals (0):
direct callees (3):
  0x007A9520  CSWCArea::SetAsNight
  0x007A9400  CSWCArea::SetAsDay
  0x007AA7A0  CSWCArea::UpdateShadowingLights
struct offsets touched (top 6):
  +0xA4 [dec 164]  x2  via ECX
  +0xA0 [dec 160]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0x1B8 [dec 440]  x1  via EDX
  +0x1BC [dec 444]  x1  via EAX
  +0x1C4 [dec 452]  x1  via ECX
```

## Card B0-1-34

GOG function: CClientExoApp::GetGUIString  gog 0x0040ddb0  callconv=(blank)  param_size_bytes=8  gog_size=53  gog_retn=8
Candidate Steam address: 0x007408f0 (size 53, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (CreateUsableItemEntry 0x77bcf0,UpdateNameLabel 0x742a00,SetItem 0x8b1250)
- best decoy (neighbour/other candidate) scores: 0x00740930:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dd80 CClientExoApp::AddFloatyText  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040ddf0 CClientExoApp::SetAutoPaused  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044a810 CClientExoAppInternal::GetGUIString -> Steam ?  in candidate callees: n/a
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
prologue for 0x007408f0: 16-byte aligned: YES
  007408f0  55             PUSH EBP
  007408f1  8B EC          MOV EBP,ESP
  007408f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007408f0
string literals (0):
direct callees (1):
  0x0079E210  CClientExoAppInternal::GetGUIString
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007408f0 FUN_007408f0 size=53 ret=8 callers=59 callees=1

undefined4 FUN_007408f0(undefined4 param_1,undefined4 param_2)

{
  FUN_0079e210(param_1,param_2);
  return param_1;
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

## Card B0-1-35

GOG function: CClientExoApp::SendLoadGameRequest  gog 0x0040cbe0  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073f720 (size 36, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
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

## Card B0-1-36

GOG function: CClientExoApp::CancelMovie  gog 0x0040e420  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740f60 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740f40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e400 CClientExoApp::SetMouseMode -> 0x00740f40  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e440 CClientExoApp::HideMouse  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044cb00 CClientExoAppInternal::CancelMovie -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740f60: 16-byte aligned: YES
  00740f60  55             PUSH EBP
  00740f61  8B EC          MOV EBP,ESP
  00740f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740f60
string literals (0):
direct callees (1):
  0x007A0500  CClientExoAppInternal::CancelMovie
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740f60 FUN_00740f60 size=22 ret=0 callers=1 callees=1
THUNK -> 0x007A0500 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740f60(undefined4 param_1)

{
  FUN_007a0500(param_1);
  return;
}
```

### decoy: 0x00740f40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740f40: 16-byte aligned: YES
  00740f40  55             PUSH EBP
  00740f41  8B EC          MOV EBP,ESP
  00740f43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740f40
string literals (0):
direct callees (1):
  0x007A0490  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-1-37

GOG function: CClientExoApp::SetVideoMode  gog 0x0040d0b0  callconv=(blank)  param_size_bytes=4  gog_size=30  gog_retn=4
Candidate Steam address: 0x0073fbd0 (size 30, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fbf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d090 CClientExoApp::GetHitCheckMaterialMask  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d0d0 CClientExoApp::SetTexturePack -> 0x0073fbf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043a1c0 CClientExoAppInternal::SetVideoMode -> Steam ?  in candidate callees: n/a
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

## Card B0-1-38

GOG function: CSWCObject::HideShadowBlob  gog 0x00410c80  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x0078bc50 (size 111, retn 0)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00764fb0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c50 CSWCObject::ShowShadowBlob -> 0x00764f80  gate_class=REVIEW
- GOG-order neighbour after:  0x00410cb0 CSWCObject::GetCanSeeHeight  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x68,0xb0} | Steam {0x38,0x3c,0x40,0x2a0}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078bc50: 16-byte aligned: YES
  0078bc50  55             PUSH EBP
  0078bc51  8B EC          MOV EBP,ESP
  0078bc53  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0078bc50 FUN_0078bc50 size=111 ret=0 callers=1 callees=4

void __fastcall FUN_0078bc50(int param_1)

{
  if (*(int *)(param_1 + 0x3c) != 0) {
    FUN_0084bda0();
  }
  if (*(int *)(param_1 + 0x38) != 0) {
    FUN_0084b490(param_1,*(undefined4 *)(param_1 + 0x38));
  }
  if (*(int *)(param_1 + 0x40) != 0) {
    FUN_007c7030();
  }
  if (*(int *)(param_1 + 0x2a0) != 0) {
    FUN_00865220();
  }
  return;
}
```

### decoy: 0x00764fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764fb0: 16-byte aligned: YES
  00764fb0  55             PUSH EBP
  00764fb1  8B EC          MOV EBP,ESP
  00764fb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00764fb0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB0 [dec 176]  x1  via EDX
```

## Card B0-1-39

GOG function: CSWSItemPropertyHandler::RemoveACBonus  gog 0x007c5670  callconv=__thiscall  param_size_bytes=12  gog_size=717  gog_retn=16
Candidate Steam address: 0x005ae680 (size 407, retn 16)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::RemoveImprovedForceResistance
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.44 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ae530:STR0/CALLEE0/SIG0.231/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c53c0 CSWSItemPropertyHandler::ApplyACBonus -> 0x005adfb0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5940 CSWSItemPropertyHandler::ApplyImprovedForceResistance -> 0x005ae530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 8 common): GOG {0x2,0x4*,0x6*,0x8*,0xa*,0xc,0x14*,0x18*,0x34,0x8b,0x130*,0x13a,0x1198*} | Steam {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x14e,0x1198*}
imm sets (* = in both, 3 common): GOG {0x2,0x3,0x4,0x5,0x7*,0x10*,0x70,0x148*} | Steam {0x7*,0xb,0x10*,0x48,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005ae680: 16-byte aligned: YES
  005ae680  55             PUSH EBP
  005ae681  8B EC          MOV EBP,ESP
  005ae683  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 005ae680 FUN_005ae680 size=407 ret=16 callers=0 callees=6

undefined4 FUN_005ae680(int param_1,int param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  undefined4 extraout_var;
  undefined4 uVar6;
  undefined1 local_28 [8];
  uint local_20;
  undefined4 *local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952eb8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_14;
  puVar4 = local_28;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  uVar6 = extraout_var;
  FUN_006f0190(0xb);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,uVar6);
  local_18 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x14e);
  while( true ) {
    if (local_18[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995fa0 < (int)(uint)*(ushort *)(*(int *)(*local_18 + local_20 * 4) + 8)) break;
    local_1c = *(undefined4 **)(*local_18 + local_20 * 4);
    if ((((*(ushort *)(local_1c + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995fa0) &&
         ((*(ushort *)((int)local_1c + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_1c[6 = byte +0x18] == *(int *)(param_1 + 0x14))) && (iVar3 = FUN_005e4e60(0), iVar3 == local_14))
    {
      FUN_00544790(*local_1c,local_1c[1 = byte +0x4]);
      ExceptionList = local_10;
      return 0;
    }
    local_20 = local_20 + 1;
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005ae530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ae530: 16-byte aligned: YES
  005ae530  55             PUSH EBP
  005ae531  8B EC          MOV EBP,ESP
  005ae533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ae530
string literals (1):
  0x00992E8C  at 005ae558  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-1-40

GOG function: CClientExoApp::GetCharacterChangeInProgress  gog 0x0040e480  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00740f00 (size 22, retn 0)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::BusyWaitMuteSound+name_held_elsewhere=0x00740fc0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740fc0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e460 CClientExoApp::ShowMouse  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040e4a0 CClientExoApp::SetCharacterChangeInProgress  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x4ac} | Steam {0x4*}
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
prologue for 0x00740f00: 16-byte aligned: YES
  00740f00  55             PUSH EBP
  00740f01  8B EC          MOV EBP,ESP
  00740f03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740f00
string literals (0):
direct callees (1):
  0x007A0630  CClientExoAppInternal::BusyWaitMuteSound
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740f00 FUN_00740f00 size=22 ret=0 callers=2 callees=1
THUNK -> 0x007A0630 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740f00(undefined4 param_1)

{
  FUN_007a0630(param_1);
  return;
}
```

### decoy: 0x00740fc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740fc0: 16-byte aligned: YES
  00740fc0  55             PUSH EBP
  00740fc1  8B EC          MOV EBP,ESP
  00740fc3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740fc0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x4AC [dec 1196]  x1  via ECX
```

## Card B0-1-41

GOG function: CSWCArea::SetAsDawn  gog 0x00455d60  callconv=(blank)  param_size_bytes=0  gog_size=153  gog_retn=0
Candidate Steam address: 0x007a9360 (size 153, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a92c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00455cc0 CSWCArea::SetAsDusk -> 0x007a92c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00455e00 CSWCArea::SetAsDay -> 0x007a9400  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00455e00 CSWCArea::SetAsDay -> Steam ?  in candidate callees: n/a
  0x00455f20 CSWCArea::SetAsNight -> Steam ?  in candidate callees: n/a
  0x004571a0 CSWCArea::UpdateShadowingLights -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0xa0*,0xa4*,0xb4*,0x1b8*,0x1bc*,0x1c4*} | Steam {0xa0*,0xa4*,0xb4*,0x1b8*,0x1bc*,0x1c4*}
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
prologue for 0x007a9360: 16-byte aligned: YES
  007a9360  55             PUSH EBP
  007a9361  8B EC          MOV EBP,ESP
  007a9363  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a9360
string literals (0):
direct callees (3):
  0x007A9520  CSWCArea::SetAsNight
  0x007A9400  CSWCArea::SetAsDay
  0x007AA7A0  CSWCArea::UpdateShadowingLights
struct offsets touched (top 6):
  +0xA4 [dec 164]  x2  via ECX
  +0xA0 [dec 160]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0x1B8 [dec 440]  x1  via EDX
  +0x1BC [dec 444]  x1  via EAX
  +0x1C4 [dec 452]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007a9360 FUN_007a9360 size=153 ret=0 callers=1 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_007a9360(int param_1)

{
  if (*(int *)(param_1 + 0xa0) == 0) {
    if (*(int *)(param_1 + 0xa4) == 0) {
      FUN_007a9400();
    }
    else {
      FUN_007a9520();
    }
  }
  else {
    _DAT_009f6390 = 0;
    _DAT_009f5d90 = (float)*(byte *)(param_1 + 0xb4) / (float)_DAT_0098ba38;
    *(undefined4 *)(param_1 + 0x1c4) = 1;
    *(undefined4 *)(param_1 + 0x1b8) = 1;
    *(undefined4 *)(param_1 + 0x1bc) = 0;
    *(undefined4 *)(param_1 + 0xa4) = 1;
    FUN_007aa7a0();
  }
  return;
}
```

### decoy: 0x007a92c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a92c0: 16-byte aligned: YES
  007a92c0  55             PUSH EBP
  007a92c1  8B EC          MOV EBP,ESP
  007a92c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a92c0
string literals (0):
direct callees (3):
  0x007A9520  CSWCArea::SetAsNight
  0x007A9400  CSWCArea::SetAsDay
  0x007AA7A0  CSWCArea::UpdateShadowingLights
struct offsets touched (top 6):
  +0xA4 [dec 164]  x2  via ECX
  +0xA0 [dec 160]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0x1B8 [dec 440]  x1  via EAX
  +0x1BC [dec 444]  x1  via EDX
  +0x1C4 [dec 452]  x1  via ECX
```

## Card B0-1-42

GOG function: CClientExoApp::StopCreditSequence  gog 0x0040e510  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00741050 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00741030:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e4f0 CClientExoApp::StartCreditSequence  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040e530 CClientExoApp::GetCreditSequenceInProgress -> 0x00741070  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044d8c0 CClientExoAppInternal::StopCreditSequence -> Steam ?  in candidate callees: n/a
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
prologue for 0x00741050: 16-byte aligned: YES
  00741050  55             PUSH EBP
  00741051  8B EC          MOV EBP,ESP
  00741053  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00741050
string literals (0):
direct callees (1):
  0x007A12C0  CClientExoAppInternal::StopCreditSequence
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00741050 FUN_00741050 size=22 ret=0 callers=1 callees=1
THUNK -> 0x007A12C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00741050(undefined4 param_1)

{
  FUN_007a12c0(param_1);
  return;
}
```

### decoy: 0x00741030 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00741030: 16-byte aligned: YES
  00741030  55             PUSH EBP
  00741031  8B EC          MOV EBP,ESP
  00741033  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00741030
string literals (0):
direct callees (1):
  0x007A1130  CClientExoAppInternal::StartCreditSequence
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B0-1-43

GOG function: CClientExoApp::RunDeathSequence  gog 0x0040d860  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007403a0 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740380:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d840 CClientExoApp::ChangeCharacter -> 0x00740380  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d880 CClientExoApp::SetGameOver -> 0x007403c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004486e0 CClientExoAppInternal::RunDeathSequence -> Steam ?  in candidate callees: n/a
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
prologue for 0x007403a0: 16-byte aligned: YES
  007403a0  55             PUSH EBP
  007403a1  8B EC          MOV EBP,ESP
  007403a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007403a0
string literals (0):
direct callees (1):
  0x0079C0E0  CClientExoAppInternal::RunDeathSequence
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007403a0 FUN_007403a0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x0079C0E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007403a0(undefined4 param_1)

{
  FUN_0079c0e0(param_1);
  return;
}
```

### decoy: 0x00740380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740380: 16-byte aligned: YES
  00740380  55             PUSH EBP
  00740381  8B EC          MOV EBP,ESP
  00740383  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740380
string literals (0):
direct callees (1):
  0x00797F30  CClientExoAppInternal::ChangeCharacter
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B0-1-44

GOG function: CClientExoApp::EnableVideoEffect  gog 0x0040de60  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x007409a0 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740980:STR0/CALLEE0/SIG1/SRV0/VT0
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
prologue for 0x007409a0: 16-byte aligned: YES
  007409a0  55             PUSH EBP
  007409a1  8B EC          MOV EBP,ESP
  007409a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007409a0
string literals (0):
direct callees (1):
  0x0079E550  CClientExoAppInternal::EnableVideoEffect
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007409a0 FUN_007409a0 size=28 ret=4 callers=7 callees=1
THUNK -> 0x0079E550 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007409a0(undefined4 param_1)

{
  FUN_0079e550(param_1);
  return;
}
```

### decoy: 0x00740980 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740980: 16-byte aligned: YES
  00740980  55             PUSH EBP
  00740981  8B EC          MOV EBP,ESP
  00740983  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740980
string literals (0):
direct callees (1):
  0x0079CF40  CClientExoAppInternal::SetLoadScreenHint
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-1-45

GOG function: CClientExoApp::SetEventDescriptions  gog 0x0040cfe0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fb00 (size 22, retn 0)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fad0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cfb0 CClientExoApp::StartServices  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d000 CClientExoApp::StopServices  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00432410 CClientExoAppInternal::SetEventDescriptions -> Steam ?  in candidate callees: n/a
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

## Card B0-1-46

GOG function: CClientExoApp::SetTexturePack  gog 0x0040d0d0  callconv=(blank)  param_size_bytes=1  gog_size=31  gog_retn=4
Candidate Steam address: 0x0073fbf0 (size 31, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fbd0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d0b0 CClientExoApp::SetVideoMode -> 0x0073fbd0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d0f0 CClientExoApp::GetSoundSetList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00439c70 CClientExoAppInternal::SetTexturePack -> Steam ?  in candidate callees: n/a
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

## Card B0-1-47

GOG function: CSWCObject::SetId  gog 0x00410c30  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00764f60 (size 22, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bcc0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c50 CSWCObject::ShowShadowBlob -> 0x00764f80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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
prologue for 0x00764f60: 16-byte aligned: YES
  00764f60  55             PUSH EBP
  00764f61  8B EC          MOV EBP,ESP
  00764f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00764f60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00764f60 FUN_00764f60 size=22 ret=4 callers=0 callees=0

void __thiscall FUN_00764f60(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 4) = param_2;
  return;
}
```

### decoy: 0x0078bcc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bcc0: 16-byte aligned: YES
  0078bcc0  55             PUSH EBP
  0078bcc1  8B EC          MOV EBP,ESP
  0078bcc3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bcc0
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-1-48

GOG function: CClientExoApp::SetMouseMode  gog 0x0040e400  callconv=(blank)  param_size_bytes=1  gog_size=29  gog_retn=4
Candidate Steam address: 0x00740f40 (size 29, retn 4)
Candidate sources: branch+name_map:medium:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740f80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e3e0 CClientExoApp::GetPazaakIsRunning -> 0x00740f20  gate_class=AUTO
- GOG-order neighbour after:  0x0040e420 CClientExoApp::CancelMovie -> 0x00740f60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ca90 CClientExoAppInternal::SetMouseMode -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740f40: 16-byte aligned: YES
  00740f40  55             PUSH EBP
  00740f41  8B EC          MOV EBP,ESP
  00740f43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740f40
string literals (0):
direct callees (1):
  0x007A0490  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740f40 FUN_00740f40 size=29 ret=4 callers=1 callees=1
THUNK -> 0x007A0490 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740f40(undefined1 param_1)

{
  FUN_007a0490(param_1);
  return;
}
```

### decoy: 0x00740f80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740f80: 16-byte aligned: YES
  00740f80  55             PUSH EBP
  00740f81  8B EC          MOV EBP,ESP
  00740f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740f80
string literals (0):
direct callees (1):
  0x007B38D0  CClientExoAppInternal::HideMouse
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-1-49

GOG function: CClientExoApp::DisableInput  gog 0x0040d790  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007402d0 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740540:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d770 CClientExoApp::StopSoundPlayingInDialog  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d7b0 CClientExoApp::EnableInput -> 0x007402f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0045ff80 CClientExoAppInternal::DisableInput -> Steam ?  in candidate callees: n/a
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

### decoy: 0x00740540 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740540: 16-byte aligned: YES
  00740540  55             PUSH EBP
  00740541  8B EC          MOV EBP,ESP
  00740543  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740540
string literals (0):
direct callees (1):
  0x0079CB20  CClientExoAppInternal::SetLoadScreenByModuleName
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-1-50

GOG function: CSWCArea::AddWeatherObject  gog 0x00456040  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x007a9640 (size 31, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a9800:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00455f20 CSWCArea::SetAsNight -> 0x007a9520  gate_class=AUTO
- GOG-order neighbour after:  0x00456060 CSWCArea::GetWeatherObject -> 0x007a9660  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x1c8*} | Steam {0x4*,0x1c8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007a9640: 16-byte aligned: YES
  007a9640  55             PUSH EBP
  007a9641  8B EC          MOV EBP,ESP
  007a9643  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a9640
string literals (0):
direct callees (1):
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a9640 FUN_007a9640 size=31 ret=4 callers=2 callees=1
THUNK -> 0x0083EA60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007a9640(undefined4 param_1)

{
  FUN_0083ea60(param_1);
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

## Card B0-1-51

GOG function: CSWSItemPropertyHandler::RemoveACBonus  gog 0x007c5670  callconv=__thiscall  param_size_bytes=12  gog_size=717  gog_retn=16
Candidate Steam address: 0x005adcf0 (size 669, retn 16)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::RemoveAbilityBonus
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.54 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.54
- best decoy (neighbour/other candidate) scores: 0x005adfb0:STR0/CALLEE0/SIG0.488/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c53c0 CSWSItemPropertyHandler::ApplyACBonus -> 0x005adfb0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5940 CSWSItemPropertyHandler::ApplyImprovedForceResistance -> 0x005ae530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 9 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0xc,0x14*,0x18*,0x34,0x8b,0x130*,0x13a,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x150,0x1198*}
imm sets (* = in both, 4 common): GOG {0x2,0x3,0x4,0x5*,0x7*,0x10*,0x70,0x148*} | Steam {0x5*,0x7*,0x10*,0x50,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005adcf0: 16-byte aligned: YES
  005adcf0  55             PUSH EBP
  005adcf1  8B EC          MOV EBP,ESP
  005adcf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005adcf0
string literals (1):
  0x00992E8C  at 005add18  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x150 [dec 336]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005adcf0 FUN_005adcf0 size=669 ret=16 callers=0 callees=6

undefined4 FUN_005adcf0(int param_1,int param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  undefined1 local_28 [8];
  uint local_20;
  undefined4 *local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953038;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_14;
  puVar4 = local_28;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(1);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  local_18 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x150);
LAB_005adda0:
  while( true ) {
    if (local_18[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995fac < (int)(uint)*(ushort *)(*(int *)(*local_18 + local_20 * 4) + 8)) {
      ExceptionList = local_10;
      return 0;
    }
    local_1c = *(undefined4 **)(*local_18 + local_20 * 4);
    if ((((*(ushort *)(local_1c + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995fac) &&
         ((*(ushort *)((int)local_1c + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_1c[6 = byte +0x18] == *(int *)(param_1 + 0x14))) && (iVar3 = FUN_005e4e60(1), iVar3 == local_14))
    break;
    local_20 = local_20 + 1;
  }
  switch(*(undefined2 *)(param_2 + 2)) {
  case 0:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_009960f8) {
      local_20 = local_20 + 1;
      goto LAB_005adda0;
    }
    break;
  case 1:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_009960fc) {
      local_20 = local_20 + 1;
      goto LAB_005adda0;
    }
    break;
  case 2:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_00996100) {
      local_20 = local_20 + 1;
      goto LAB_005adda0;
    }
    break;
  case 3:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_00996104) {
      local_20 = local_20 + 1;
      goto LAB_005adda0;
    }
    break;
/* ... 17 more lines truncated by --max-lines */
```

### decoy: 0x005adfb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005adfb0: 16-byte aligned: YES
  005adfb0  55             PUSH EBP
  005adfb1  8B EC          MOV EBP,ESP
  005adfb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005adfb0
string literals (1):
  0x00992E8C  at 005ae01e  'Value'
direct callees (12):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x006D5200  (unnamed)
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 10):
  +0x2 [dec 2]  x4  via ECX
  +0xA [dec 10]  x2  via ECX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x34 [dec 52]  x1  via EAX
  +0x8B [dec 139]  x1  via ECX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-1-52

GOG function: CClientExoApp::DisableVideoEffect  gog 0x0040de80  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007409c0 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007409a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040de60 CClientExoApp::EnableVideoEffect -> 0x007409a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dea0 CClientExoApp::GetVideoEffectType  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044af60 CClientExoAppInternal::DisableVideoEffect -> Steam ?  in candidate callees: n/a
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
prologue for 0x007409c0: 16-byte aligned: YES
  007409c0  55             PUSH EBP
  007409c1  8B EC          MOV EBP,ESP
  007409c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007409c0
string literals (0):
direct callees (1):
  0x0079E960  CClientExoAppInternal::DisableVideoEffect
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007409c0 FUN_007409c0 size=22 ret=0 callers=7 callees=1
THUNK -> 0x0079E960 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007409c0(undefined4 param_1)

{
  FUN_0079e960(param_1);
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

## Card B0-1-53

GOG function: CSWSPlaceable::SetEffectSpellId  gog 0x004213a0  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x005ef180 (size 25, retn 4)
Candidate sources: branch+name_map:high:batch5_reverse_engineering    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ef160:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421370 CSWCCreature::SetArea  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004213c0 CSWCCreature::SetThrownLightsaber  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3e0*} | Steam {0x3e0*}
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
prologue for 0x005ef180: 16-byte aligned: YES
  005ef180  55             PUSH EBP
  005ef181  8B EC          MOV EBP,ESP
  005ef183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005ef180
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3E0 [dec 992]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005ef180 FUN_005ef180 size=25 ret=4 callers=1 callees=0

void __thiscall FUN_005ef180(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x3e0) = param_2;
  return;
}
```

### decoy: 0x005ef160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ef160: 16-byte aligned: YES
  005ef160  55             PUSH EBP
  005ef161  8B EC          MOV EBP,ESP
  005ef163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005ef160
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3E0 [dec 992]  x1  via EAX
```

## Card B0-1-54

GOG function: CSWCObject::TurnOnShadows  gog 0x00472630  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x007ed4b0 (size 21, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::GetPortraitId
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed4d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472610 CSWCObject::GetPortraitId -> 0x007ed4b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00472660 CSWCObject::TurnOffShadows -> 0x007ed500  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68,0xb8*} | Steam {0xb8*}
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

## Card B0-1-55

GOG function: CClientExoApp::GetPlayerCreature  gog 0x0040c910  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f450 (size 22, retn 0)
Candidate sources: branch+mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetControlledObject
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f750:STR0/CALLEE2/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c8f0 CClientExoApp::GetModule  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040c930 CClientExoApp::GetPlayerCreatureId  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> Steam ?  in candidate callees: n/a
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

## Card B0-1-56

GOG function: CClientExoApp::GetAutoPaused  gog 0x0040de20  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x00740960 (size 26, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740930:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ddf0 CClientExoApp::SetAutoPaused  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040de40 CClientExoApp::SetLoadScreenHint  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x384*} | Steam {0x4*,0x384*}
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

## Card B0-1-57

GOG function: CSWCObject::AnimationStationary  gog 0x00474420  callconv=(blank)  param_size_bytes=2  gog_size=132  gog_retn=4
Candidate Steam address: 0x007ef350 (size 129, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::AnimationPause
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef2c0:STR0/CALLEE0/SIG1/SRV0/VT0
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
prologue for 0x007ef350: 16-byte aligned: YES
  007ef350  55             PUSH EBP
  007ef351  8B EC          MOV EBP,ESP
  007ef353  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007ef350 FUN_007ef350 size=129 ret=4 callers=2 callees=1

bool __thiscall FUN_007ef350(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe2c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef2c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef2c0: 16-byte aligned: YES
  007ef2c0  55             PUSH EBP
  007ef2c1  8B EC          MOV EBP,ESP
  007ef2c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B0-1-58

GOG function: Scene::RemoveFromSceneManager_2  gog 0x008646a0  callconv=(blank)  param_size_bytes=4  gog_size=71  gog_retn=0
Candidate Steam address: 0x004724b0 (size 121, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00472460:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00864620 Scene::RemoveFromSceneManager  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008646f0 Global::AurSetGlobalWind  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008730c0 List_T__::Remove -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x90}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4,0xc,0x10}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x004724b0: 16-byte aligned: YES
  004724b0  55             PUSH EBP
  004724b1  8B EC          MOV EBP,ESP
  004724b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004724b0
string literals (2):
  0x0098BD24  at 004724bd  'mainscene'
  0x00987710  at 004724d3  'scene'
direct callees (2):
  0x00465230  CAurScene FindByName
  0x00506C60  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x90 [dec 144]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004724b0 FUN_004724b0 size=121 ret=0 callers=1 callees=2

bool FUN_004724b0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  
  iVar1 = FUN_00465230("mainscene");
  if (iVar1 == 0) {
    iVar1 = FUN_00465230("scene");
  }
  if (iVar1 != 0) {
    FUN_00506c60(param_1,param_2,param_3);
  }
  return iVar1 != 0;
}
```

### decoy: 0x00472460 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00472460: 16-byte aligned: YES
  00472460  55             PUSH EBP
  00472461  8B EC          MOV EBP,ESP
  00472463  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00472460
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 0):
```

## Card B0-1-59

GOG function: CClientExoApp::PlayerFlourishWeapons  gog 0x0040dd60  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007408a0 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740880:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dd40 CClientExoApp::SetCombatMode -> 0x00740880  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dd80 CClientExoApp::AddFloatyText  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ba30 CClientExoAppInternal::PlayerFlourishWeapons -> Steam ?  in candidate callees: n/a
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
prologue for 0x007408a0: 16-byte aligned: YES
  007408a0  55             PUSH EBP
  007408a1  8B EC          MOV EBP,ESP
  007408a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007408a0
string literals (0):
direct callees (1):
  0x0079F430  CClientExoAppInternal::PlayerFlourishWeapons
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007408a0 FUN_007408a0 size=22 ret=0 callers=2 callees=1
THUNK -> 0x0079F430 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007408a0(undefined4 param_1)

{
  FUN_0079f430(param_1);
  return;
}
```

### decoy: 0x00740880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740880: 16-byte aligned: YES
  00740880  55             PUSH EBP
  00740881  8B EC          MOV EBP,ESP
  00740883  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740880
string literals (0):
direct callees (1):
  0x0079D6E0  CClientExoAppInternal::SetCombatMode
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-1-60

GOG function: CSWPartyTable::AddGameInProgress  gog 0x00703650  callconv=(blank)  param_size_bytes=0  gog_size=138  gog_retn=0
Candidate Steam address: 0x005fdd30 (size 138, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWPartyTable::RemoveGameInProgress
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fdca0:STR0/CALLEE0/SIG1/SRV0/VT0
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
