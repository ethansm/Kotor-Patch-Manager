# Review cards B0 chunk 3

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-3-01

GOG function: CServerExoApp::GetGameObject  gog 0x0064bac0  callconv=__thiscall  param_size_bytes=4  gog_size=62  gog_retn=4
Candidate Steam address: 0x0051c0e0 (size 28, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x0051c0a0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c0a0:STR0/CALLEE9/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064baa0 CServerExoApp::GetObjectArray -> 0x0051c080  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bb00 CServerExoApp::GetItemByGameObjectID  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*,0x10} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c0e0: 16-byte aligned: YES
  0051c0e0  55             PUSH EBP
  0051c0e1  8B EC          MOV EBP,ESP
  0051c0e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c0e0
string literals (0):
direct callees (1):
  0x00537B00  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c0e0 FUN_0051c0e0 size=28 ret=4 callers=109 callees=1
THUNK -> 0x00537B00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c0e0(undefined4 param_1)

{
  FUN_00537b00(param_1);
  return;
}
```

### decoy: 0x0051c0a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c0a0: 16-byte aligned: YES
  0051c0a0  55             PUSH EBP
  0051c0a1  8B EC          MOV EBP,ESP
  0051c0a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c0a0
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1005C [dec 65628]  x1  via EDX
```

## Card B0-3-02

GOG function: CExoResMan::AddResourceImageFile  gog 0x0061af40  callconv=(blank)  param_size_bytes=4  gog_size=29  gog_retn=4
Candidate Steam address: 0x007115e0 (size 29, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00711670:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061af20 CExoResMan::AddEncapsulatedResourceFile -> 0x007115c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0061af60 CExoResMan::AddResourceImageFilesAsync -> 0x00711600  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061b0b0 CExoResMan::AddKeyTable -> Steam ?  in candidate callees: n/a
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
prologue for 0x007115e0: 16-byte aligned: YES
  007115e0  55             PUSH EBP
  007115e1  8B EC          MOV EBP,ESP
  007115e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007115e0
string literals (0):
direct callees (1):
  0x00711750  CExoResMan::AddKeyTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007115e0 FUN_007115e0 size=29 ret=4 callers=4 callees=1
THUNK -> 0x00711750 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007115e0(undefined4 param_1)

{
  FUN_00711750(param_1,4,0);
  return;
}
```

### decoy: 0x00711670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711670: 16-byte aligned: YES
  00711670  55             PUSH EBP
  00711671  8B EC          MOV EBP,ESP
  00711673  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711670
string literals (0):
direct callees (1):
  0x00711750  CExoResMan::AddKeyTable
struct offsets touched (top 0):
```

## Card B0-3-03

GOG function: CSWCDoor::GetState  gog 0x005070c0  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x0087b7e0 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (UpdateAreaTransitionDisplay 0x87c9f0)
- best decoy (neighbour/other candidate) scores: 0x0087b3f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00506cd0 CSWCDoor::AnimationUpdate -> 0x0087b3f0  gate_class=AUTO
- GOG-order neighbour after:  0x005070e0 CSWCDoor::LoadModel -> 0x0087b800  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x120*} | Steam {0x120*}
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
prologue for 0x0087b7e0: 16-byte aligned: YES
  0087b7e0  55             PUSH EBP
  0087b7e1  8B EC          MOV EBP,ESP
  0087b7e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0087b7e0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x120 [dec 288]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0087b7e0 FUN_0087b7e0 size=20 ret=0 callers=2 callees=0

undefined1 __fastcall FUN_0087b7e0(int param_1)

{
  return *(undefined1 *)(param_1 + 0x120);
}
```

### decoy: 0x0087b3f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0087b3f0: 16-byte aligned: YES
  0087b3f0  55             PUSH EBP
  0087b3f1  8B EC          MOV EBP,ESP
  0087b3f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0087b3f0
string literals (0):
direct callees (14):
  0x007EF5D0  CSWCObject::AnimationFireAndForget
  0x007ED980  CSWCObject::GetLoopingAnimation
  0x007EF500  CSWCObject::AnimationLooping
  0x007EFAC0  CSWCObject::GetTopFireForgetAnimation
  0x007F00B0  CSWCObject::RemoveTopFireForgetAnimation
  0x00733540  CExoString::CExoString(empty)
  0x0071B8B0  (unnamed)
  0x0071AFA0  C2DA::GetCExoStringEntry
  0x00710810  (unnamed)
  0x0070BA90  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0087BA40  CSWCDoor::SetState
  0x0087B7E0  CSWCDoor::GetState
  0x009196FD  operator delete
struct offsets touched (top 10):
  +0x48 [dec 72]  x7  via EAX,ECX,EDX
  +0x130 [dec 304]  x6  via EAX,EDX
  +0x64 [dec 100]  x5  via EAX,ECX,EDX
  +0x74 [dec 116]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x68 [dec 104]  x2  via EAX,ECX
  +0x28 [dec 40]  x1  via EDX
  +0x2C [dec 44]  x1  via ECX
  +0x44 [dec 68]  x1  via EAX
```

## Card B0-3-04

GOG function: CRes::Request  gog 0x0061ac30  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x007112d0 (size 26, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007112b0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061ac10 CRes::Release -> 0x007112b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0061ac50 Global::AsyncRimLoadThreadProc  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061cc70 CExoResMan::Request -> Steam ?  in candidate callees: n/a
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
prologue for 0x007112d0: 16-byte aligned: YES
  007112d0  55             PUSH EBP
  007112d1  8B EC          MOV EBP,ESP
  007112d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007112d0
string literals (0):
direct callees (1):
  0x00713310  CExoResMan::Request
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007112d0 FUN_007112d0 size=26 ret=0 callers=18 callees=1
THUNK -> 0x00713310 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007112d0(undefined4 param_1)

{
  FUN_00713310(param_1);
  return;
}
```

### decoy: 0x007112b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007112b0: 16-byte aligned: YES
  007112b0  55             PUSH EBP
  007112b1  8B EC          MOV EBP,ESP
  007112b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007112b0
string literals (0):
direct callees (1):
  0x007130B0  CExoResMan::Release
struct offsets touched (top 0):
```

## Card B0-3-05

GOG function: CExoResMan::RemoveResourceImageFile  gog 0x0061b030  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x007116d0 (size 27, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007116f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061b010 CExoResMan::RemoveEncapsulatedResourceFile -> 0x007116b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0061b050 CExoResMan::RemoveFixedKeyTableFile -> 0x007116f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061cb30 CExoResMan::RemoveKeyTable -> Steam ?  in candidate callees: n/a
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
prologue for 0x007116d0: 16-byte aligned: YES
  007116d0  55             PUSH EBP
  007116d1  8B EC          MOV EBP,ESP
  007116d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007116d0
string literals (0):
direct callees (1):
  0x007131D0  CExoResMan::RemoveKeyTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007116d0 FUN_007116d0 size=27 ret=4 callers=4 callees=1
THUNK -> 0x007131D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007116d0(undefined4 param_1)

{
  FUN_007131d0(param_1,4);
  return;
}
```

### decoy: 0x007116f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007116f0: 16-byte aligned: YES
  007116f0  55             PUSH EBP
  007116f1  8B EC          MOV EBP,ESP
  007116f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007116f0
string literals (0):
direct callees (1):
  0x007131D0  CExoResMan::RemoveKeyTable
struct offsets touched (top 0):
```

## Card B0-3-06

GOG function: CSWGuiSaveLoad::LoadGame  gog 0x005d33b0  callconv=(blank)  param_size_bytes=4  gog_size=2611  gog_retn=4
Candidate Steam address: 0x00853a00 (size 2611, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-savegame    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 0.93 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 6 (SendLoadGameRequest 0x73f720,DisableInput 0x7402d0,SetLoadFromSaveGame 0x51ce00); SIG 0.93
- best decoy (neighbour/other candidate) scores: 0x00854440:STR0/CALLEE0/SIG0.226/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d2e00 CStatusSummary::LoadFromGFF  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d3df0 CSWGuiSaveLoad::PromptForSaveName  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (47), mapped to Steam through trusted rows only:
  0x00401b80 CAppManager::CreateServer -> Steam ?  in candidate callees: n/a
  0x00401c30 CAppManager::DestroyServer -> Steam ?  in candidate callees: n/a
  0x004099e0 Global::UpdateScreen -> Steam ?  in candidate callees: n/a
  0x0040c710 CClientExoApp::UnloadModule -> Steam ?  in candidate callees: n/a
  0x0040c7d0 CClientExoApp::GetClientName -> Steam ?  in candidate callees: n/a
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam ?  in candidate callees: n/a
  0x0040cbe0 CClientExoApp::SendLoadGameRequest -> Steam 0x0073f720  in candidate callees: yes
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  ... 39 more GOG callees
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0x10*,0x1c*,0x48*,0x68*,0x74*,0x98*,0xa0*,0x1d0*,0x1d4*,0x1e0*,0x1e1*,0x1b70} | Steam {0x4*,0x8*,0x10*,0x1c*,0x48*,0x68*,0x74*,0x98*,0xa0*,0x1d0*,0x1d4*,0x1e0*,0x1e1*,0x1e40}
imm sets (* = in both, 18 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x114*,0x1f4*,0x200*,...(+4)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x114*,0x1f4*,0x200*,...(+4)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00853a00: 16-byte aligned: YES
  00853a00  55             PUSH EBP
  00853a01  8B EC          MOV EBP,ESP
  00853a03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00853a00
string literals (0):
direct callees (47):
  0x0073F750  ClientApp::GetGuiInGame
  0x0070BC60  (unnamed)
  0x00740EE0  CClientExoApp::BusyWaitMenuMusic
  0x00740F00  CClientExoApp::BusyWaitMuteSound
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x0071DEE0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x0071E910  (unnamed)
  0x0071EBD0  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x00410530  CSWGuiManager::AddPanel
  0x005FF000  CSWPartyTable::ResetCurrentSessionStartTime
  0x007D0AF0  CGuiInGame::SetGlobalUnpauseableState
  0x004109C0  CSWGuiManager::PopModalPanel
  0x007CA060  CGuiInGame::HideSWInGameGui
  0x00740540  CClientExoApp::SetLoadScreenByModuleName
  0x00733570  CExoString::CExoString_char
struct offsets touched (top 10):
  +0x4 [dec 4]  x14  via EAX,ECX,EDX
  +0x48 [dec 72]  x12  via EAX,ECX,EDX
  +0x68 [dec 104]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via ECX,EDX
  +0x1C [dec 28]  x4  via ECX,EDX
  +0x1D4 [dec 468]  x4  via EAX,ECX,EDX
  +0x1D0 [dec 464]  x3  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EDX
  +0xA0 [dec 160]  x2  via ECX,EDX
  +0x1E0 [dec 480]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00853a00 FUN_00853a00 size=2611 ret=4 callers=2 callees=47

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00853a00(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined1 *puVar4;
  undefined1 local_78 [24];
  undefined1 local_60 [16];
  undefined1 *local_50;
  int local_44;
  SchedulerResourceManagement *local_40;
  int local_3c;
  uint local_38[2 = byte +0x8];
  uint local_30[2 = byte +0x8];
  uint *local_28;
  undefined1 local_24 [8];
  int *local_1c;
  int local_18;
  SchedulerResourceManagement *local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972546;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((((*(uint *)(param_1 + 0x68) >> 1 & 1) != 0) ||
      (iVar1 = FUN_0073f750(), (*(uint *)(*(int *)(iVar1 + 0xa0) + 0x68) & 1) != 0)) ||
     (*(int *)(param_1 + 0x1e40) != 0)) {
    FUN_0070bc60();
    if ((*(uint *)(param_1 + 0x68) >> 1 & 1) == 0) {
      FUN_00740f00();
    }
    else {
      FUN_00740ee0();
    }
    local_18 = param_2;
    local_14 = *(SchedulerResourceManagement **)(DAT_00a1b4a4 + 4);
    if (((*(uint *)(param_2 + 0x1d0) >> 5 & 1) == 0) || (DAT_00a7f098 != 0)) {
      FUN_005ff000();
      FUN_0073f750();
      FUN_007d0af0();
      if ((*(uint *)(local_18 + 0x1d0) >> 3 & 1) == 0) {
        if ((*(uint *)(local_18 + 0x1d0) >> 4 & 1) == 0) {
          if ((*(uint *)(param_1 + 0x68) >> 1 & 1) == 0) {
            local_40 = *(SchedulerResourceManagement **)(DAT_00a1b4a4 + 4);
            FUN_00740540();
            FUN_00733570();
            local_8 = 0xb;
            FUN_007405a0();
            local_8 = 0xffffffff;
            FUN_00733780();
            Concurrency::details::SchedulerResourceManagement::Statistics
                      (local_40,(uint *)0x0,(uint *)0x1,(uint *)0x0);
            FUN_00740d30();
            for (local_44 = 0; local_44 < 10; local_44 = local_44 + 1) {
              FUN_00740d90();
            }
            FUN_00740dc0();
            FUN_00740980();
            FUN_00740ce0();
            iVar1 = FUN_00857490(*(undefined4 *)(local_18 + 0x1d4));
            if (iVar1 != 0) {
              *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xffffff7f;
              *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffcff | 0x200;
              while (*(int *)(*(int *)(param_1 + 0x1c) + 0x98) != 0) {
                FUN_004109c0();
              }
              FUN_0073f750();
              FUN_007ca060();
              FUN_0051ce00();
              FUN_00740c00();
              Concurrency::details::SchedulerResourceManagement::Statistics
                        (*(SchedulerResourceManagement **)(DAT_00a1b4a4 + 4),
                         *(uint **)(local_18 + 0x1d4),(uint *)(local_18 + 0x1f4 [dec 500]),
/* ... 142 more lines truncated by --max-lines */
```

### decoy: 0x00854440 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00854440: 16-byte aligned: YES
  00854440  55             PUSH EBP
  00854441  8B EC          MOV EBP,ESP
  00854443  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00854440
string literals (0):
direct callees (9):
  0x00733540  CExoString::CExoString(empty)
  0x00733780  CExoString::~CExoString
  0x0073F750  ClientApp::GetGuiInGame
  0x007337C0  CExoString::operator=
  0x00733570  CExoString::CExoString_char
  0x00919723  operator new
  0x008586D0  (unnamed)
  0x00858A40  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 5):
  +0x4 [dec 4]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x68 [dec 104]  x1  via ECX
  +0xA0 [dec 160]  x1  via EDX
  +0x1D0 [dec 464]  x1  via ECX
```

## Card B0-3-07

GOG function: CExoString::Destructor  gog 0x00605890  callconv=(blank)  param_size_bytes=0  gog_size=60  gog_retn=0
Candidate Steam address: 0x00733540 (size 33, retn 0)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoString::CExoString,CExoString::DefaultConstructor
- anchors: A-STR 0 | A-CALLEE 16 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00733780:STR0/CALLEE41/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00605810 CExoString::Constructor_5  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006058d0 CExoString::operator= -> 0x007337c0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {0x4,0x8} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00733540: 16-byte aligned: YES
  00733540  55             PUSH EBP
  00733541  8B EC          MOV EBP,ESP
  00733543  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00733540
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00733540 FUN_00733540 size=33 ret=0 callers=854 callees=0

undefined4 * __fastcall FUN_00733540(undefined4 *param_1)

{
  *param_1 = 0;
  param_1[1 = byte +0x4] = 0;
  return param_1;
}
```

### decoy: 0x00733780 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00733780: 16-byte aligned: YES
  00733780  55             PUSH EBP
  00733781  8B EC          MOV EBP,ESP
  00733783  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00733780
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-3-08

GOG function: CSWGuiSaveLoad::LaunchMostRecentSave  gog 0x005d86f0  callconv=(blank)  param_size_bytes=0  gog_size=294  gog_retn=0
Candidate Steam address: 0x00859270 (size 294, retn 0)
Candidate sources: branch+name_map:high:steam-db-pr3-savegame+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (LoadGame 0x853a00); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00859470:STR0/CALLEE0/SIG0.045/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d86d0 CSWGuiSaveLoad::SetEndGameMode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d8840 CExoArrayList_CSWGuiControl__::Insert  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00512940 unnamed -> Steam ?  in candidate callees: n/a
  0x005d1680 CSWGuiSaveLoadEntry::GetGameDirectory -> Steam ?  in candidate callees: n/a
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00606e70 CExoString::CompareNoCase -> Steam ?  in candidate callees: n/a
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x0091df10 CSWGuiListBox::GetControl -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 2 common): GOG {0x2c*,0x970*} | Steam {0x2c*,0x970*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00859270: 16-byte aligned: YES
  00859270  55             PUSH EBP
  00859271  8B EC          MOV EBP,ESP
  00859273  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00859270
string literals (0):
direct callees (7):
  0x0041FF70  (unnamed)
  0x00852340  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005355F0  (unnamed)
  0x00734D60  CExoString::CompareNoCase
  0x00853A00  CSWGuiSaveLoad::LoadGame
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00859270 FUN_00859270 size=294 ret=0 callers=1 callees=7

undefined4 FUN_00859270(void)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined1 *puVar4;
  undefined4 extraout_var;
  undefined1 local_24 [8];
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972e90;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1c = FUN_0041ff70(0);
  if (local_1c == 0) {
    uVar2 = 0;
  }
  else {
    uVar2 = extraout_var;
    FUN_00852340(local_24);
    local_8 = 0;
    if (*(int *)(DAT_00a1b4a4 + 8) == 0) {
      local_8 = 0xffffffff;
      FUN_00733780();
      uVar2 = 0;
    }
    else {
      puVar4 = local_18;
      FUN_0051c8d0(puVar4,uVar1,uVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
      FUN_005355f0(puVar4);
      local_8._0_1_ = 1;
      iVar3 = FUN_00734d60(local_24);
      if (iVar3 == 0) {
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        local_8 = 0xffffffff;
        FUN_00733780();
        uVar2 = 0;
      }
      else {
        FUN_00853a00(local_1c);
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        local_8 = 0xffffffff;
        FUN_00733780();
        uVar2 = 1;
      }
    }
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00859470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00859470: 16-byte aligned: YES
  00859470  55             PUSH EBP
  00859471  8B EC          MOV EBP,ESP
  00859473  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00859470
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

## Card B0-3-09

GOG function: CSWCPlaceable::GetName  gog 0x005349c0  callconv=(blank)  param_size_bytes=4  gog_size=51  gog_retn=4
Candidate Steam address: 0x00841140 (size 51, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00840ef0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005347d0 CSWCPlaceable::Constructor -> 0x00840f50  gate_class=AUTO
- GOG-order neighbour after:  0x00534a00 CSWCPlaceable::IsHostile -> 0x00841180  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0xfc*} | Steam {0x4*,0x8*,0xfc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00841140: 16-byte aligned: YES
  00841140  55             PUSH EBP
  00841141  8B EC          MOV EBP,ESP
  00841143  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00841140
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00841140 FUN_00841140 size=51 ret=4 callers=0 callees=1

undefined4 __thiscall FUN_00841140(int param_1,undefined4 param_2)

{
  FUN_007335f0(param_1 + 0xfc);
  return param_2;
}
```

### decoy: 0x00840ef0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00840ef0: 16-byte aligned: YES
  00840ef0  55             PUSH EBP
  00840ef1  8B EC          MOV EBP,ESP
  00840ef3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00840ef0
string literals (0):
direct callees (2):
  0x009196FD  operator delete
  0x0084A680  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-3-10

GOG function: CSWCDoor::Destructor_2  gog 0x00506a80  callconv=(blank)  param_size_bytes=0  gog_size=146  gog_retn=0
Candidate Steam address: 0x0087b1a0 (size 146, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0087afe0:STR0/CALLEE0/SIG0.357/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00506a50 CSWCDoor::Destructor -> 0x0087b170  gate_class=REVIEW
- GOG-order neighbour after:  0x00506b20 CSWCDoor::AIUpdate -> 0x0087b240  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00472750 CSWCObject::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00537c00 CSWCTrigger::UnloadModel -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x3*,0xfc*,0x134*,0x140*} | Steam {0x2*,0x3*,0xfc*,0x134*,0x140*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0087b1a0: 16-byte aligned: YES
  0087b1a0  55             PUSH EBP
  0087b1a1  8B EC          MOV EBP,ESP
  0087b1a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0087b1a0
string literals (0):
direct callees (3):
  0x0086E3B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007ED5F0  CSWCObject::Destructor_2
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0087b1a0 FUN_0087b1a0 size=146 ret=0 callers=1 callees=3

void __fastcall FUN_0087b1a0(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_00974342;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWCDoor::vftable;
  local_8 = 3;
  FUN_0086e3b0(uVar1);
  local_8._0_1_ = 2;
  FUN_00733780();
  local_8._0_1_ = 1;
  FUN_00733780();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_007ed5f0();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0087afe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0087afe0: 16-byte aligned: YES
  0087afe0  55             PUSH EBP
  0087afe1  8B EC          MOV EBP,ESP
  0087afe3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0087afe0
string literals (0):
direct callees (5):
  0x007ED050  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via ECX
  +0x3C [dec 60]  x1  via EDX
  +0x40 [dec 64]  x1  via ECX
  +0x48 [dec 72]  x1  via EAX
  +0x104 [dec 260]  x1  via ECX
  +0x108 [dec 264]  x1  via EDX
  +0x10C [dec 268]  x1  via EAX
  +0x110 [dec 272]  x1  via ECX
  +0x114 [dec 276]  x1  via EDX
  +0x118 [dec 280]  x1  via EAX
```

## Card B0-3-11

GOG function: CSWSArea::GetClientArea_2  gog 0x00722ee0  callconv=(blank)  param_size_bytes=4  gog_size=51  gog_retn=4
Candidate Steam address: 0x0052de40 (size 51, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0052de00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00722de0 CSWSArea::AwardStealthXP -> 0x0052dd00  gate_class=AUTO
- GOG-order neighbour after:  0x00722f60 CSWSArea::EnforceNoPCHeadCloning -> 0x0052de80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x2e8*,0x2ec*} | Steam {0x2e8*,0x2ec*}
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
prologue for 0x0052de40: 16-byte aligned: YES
  0052de40  55             PUSH EBP
  0052de41  8B EC          MOV EBP,ESP
  0052de43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052de40
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2E8 [dec 744]  x2  via EAX,EDX
  +0x2EC [dec 748]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0052de40 FUN_0052de40 size=51 ret=4 callers=5 callees=0

void __thiscall FUN_0052de40(int param_1,uint param_2)

{
  if (*(uint *)(param_1 + 0x2e8) < param_2) {
    param_2 = *(uint *)(param_1 + 0x2e8);
  }
  *(uint *)(param_1 + 0x2ec) = param_2;
  return;
}
```

### decoy: 0x0052de00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052de00: 16-byte aligned: YES
  0052de00  55             PUSH EBP
  0052de01  8B EC          MOV EBP,ESP
  0052de03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052de00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2EC [dec 748]  x2  via EAX,EDX
  +0x2E8 [dec 744]  x1  via ECX
```

## Card B0-3-12

GOG function: CRes::Destructor  gog 0x0061aad0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00711170 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00711230:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061aac0 CSWMGTargettingGunBank::IsTargetting  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061ab00 CRes::Destructor_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061ab00 CRes::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x00711170: 16-byte aligned: YES
  00711170  55             PUSH EBP
  00711171  8B EC          MOV EBP,ESP
  00711173  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711170
string literals (0):
direct callees (2):
  0x007111A0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00711170 FUN_00711170 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00711170(undefined4 param_1,uint param_2)

{
  FUN_007111a0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x00711230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711230: 16-byte aligned: YES
  00711230  55             PUSH EBP
  00711231  8B EC          MOV EBP,ESP
  00711233  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711230
string literals (0):
direct callees (1):
  0x00711E60  CExoResMan::Dump
struct offsets touched (top 0):
```

## Card B0-3-13

GOG function: CGuiInGame::ChangeCharacter  gog 0x004e09b0  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x007d02b0 (size 25, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d02d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0920 CGuiInGame::EndControllerPauseState  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004e09d0 CGuiInGame::SetupPauseGuiExtent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005225e0 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x007d02b0: 16-byte aligned: YES
  007d02b0  55             PUSH EBP
  007d02b1  8B EC          MOV EBP,ESP
  007d02b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d02b0
string literals (0):
direct callees (1):
  0x00750E60  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007d02b0 FUN_007d02b0 size=25 ret=0 callers=1 callees=1
THUNK -> 0x00750E60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007d02b0(undefined4 param_1)

{
  FUN_00750e60(param_1);
  return;
}
```

### decoy: 0x007d02d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d02d0: 16-byte aligned: YES
  007d02d0  55             PUSH EBP
  007d02d1  8B EC          MOV EBP,ESP
  007d02d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d02d0
string literals (0):
direct callees (1):
  0x00752F50  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x1  via ECX
```

## Card B0-3-14

GOG function: CSWVirtualMachineCommands::ExecuteCommandFloatToInt  gog 0x0078ebd0  callconv=__thiscall  param_size_bytes=8  gog_size=80  gog_retn=8
Candidate Steam address: 0x00679640 (size 453, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.15 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006795f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
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
imm sets (* = in both, 3 common): GOG {0x8*,0xc,0xfffff82f*,0xfffff830*} | Steam {0x2,0x8*,0x9,0xa,0x10,0x12,0x34,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00679640: 16-byte aligned: YES
  00679640  55             PUSH EBP
  00679641  8B EC          MOV EBP,ESP
  00679643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00679640 FUN_00679640 size=453 ret=8 callers=0 callees=8

undefined4 FUN_00679640(void)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  int local_2c;
  undefined1 local_28 [8];
  float local_20;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a2d8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9e0();
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    uVar3 = extraout_var;
    iVar2 = FUN_006fd9a0();
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      if (0x11 < local_2c) {
        local_2c = 0x12;
      }
      if (local_2c < 0) {
        local_2c = 0;
      }
      iVar2 = FUN_006fd9a0();
      if (iVar2 == 0) {
        uVar3 = 0xfffff82f;
      }
      else {
        if (9 < local_14) {
          local_14 = 9;
        }
        if (local_14 < 0) {
          local_14 = 0;
        }
        FUN_00733540(uVar1,uVar3);
        local_8 = 0;
        if (local_2c < 10) {
          FUN_00734270(local_28,"%%%1d.%02df\n",local_2c,local_14);
        }
        else {
          FUN_00734270(local_28,"%%%2d.%02df\n",local_2c,local_14);
        }
        FUN_00733540();
        local_8._0_1_ = 1;
        uVar3 = FUN_00734010((double)local_20);
        FUN_00734270(local_1c,uVar3);
        FUN_00734010();
        FUN_00733570();
        local_8._0_1_ = 2;
        iVar2 = FUN_006fda90();
        local_8._0_1_ = 1;
        FUN_00733780();
        if (iVar2 == 0) {
          local_8 = (uint)local_8._1_3_ << 8;
          FUN_00733780();
          local_8 = 0xffffffff;
          FUN_00733780();
          uVar3 = 0xfffff830;
        }
        else {
          local_8 = (uint)local_8._1_3_ << 8;
          FUN_00733780();
          local_8 = 0xffffffff;
          FUN_00733780();
/* ... 8 more lines truncated by --max-lines */
```

### decoy: 0x006795f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006795f0: 16-byte aligned: YES
  006795f0  55             PUSH EBP
  006795f1  8B EC          MOV EBP,ESP
  006795f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006795f0
string literals (0):
direct callees (3):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0091C860  _ftol2_sse
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

## Card B0-3-15

GOG function: CServerExoApp::GetGameObject  gog 0x0064bac0  callconv=__thiscall  param_size_bytes=4  gog_size=62  gog_retn=4
Candidate Steam address: 0x0051bfd0 (size 61, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x0051c0a0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.62 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051bfa0:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064baa0 CServerExoApp::GetObjectArray -> 0x0051c080  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bb00 CServerExoApp::GetItemByGameObjectID  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*,0x10} | Steam {0x4*,0xc,0x1008c}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051bfd0: 16-byte aligned: YES
  0051bfd0  55             PUSH EBP
  0051bfd1  8B EC          MOV EBP,ESP
  0051bfd3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bfd0
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bfd0 FUN_0051bfd0 size=61 ret=4 callers=1 callees=1

undefined4 __thiscall FUN_0051bfd0(int param_1,undefined4 param_2)

{
  FUN_007335f0(*(int *)(param_1 + 4) + 0x1008c);
  return param_2;
}
```

### decoy: 0x0051bfa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bfa0: 16-byte aligned: YES
  0051bfa0  55             PUSH EBP
  0051bfa1  8B EC          MOV EBP,ESP
  0051bfa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bfa0
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-3-16

GOG function: CSWGuiMainCharGen::BeginPlayGame  gog 0x005eeb30  callconv=(blank)  param_size_bytes=0  gog_size=30  gog_retn=0
Candidate Steam address: 0x00907340 (size 30, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-savegame+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00907310:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005eeb00 CSWGuiMainCharGen::ClearFeats  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005eeb50 CSWGuiMainCharGen::Draw  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005df060 CSWGuiClassSelection::BeginPlayGame -> Steam ?  in candidate callees: n/a
  0x005ed900 CSWGuiMainCharGen::RemoveSubPanels -> Steam ?  in candidate callees: n/a
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
prologue for 0x00907340: 16-byte aligned: YES
  00907340  55             PUSH EBP
  00907341  8B EC          MOV EBP,ESP
  00907343  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00907340
string literals (0):
direct callees (2):
  0x00906110  (unnamed)
  0x008FACA0  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00907340 FUN_00907340 size=30 ret=0 callers=1 callees=2

void __fastcall FUN_00907340(undefined4 param_1)

{
  FUN_00906110(param_1);
  FUN_008faca0();
  return;
}
```

### decoy: 0x00907310 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00907310: 16-byte aligned: YES
  00907310  55             PUSH EBP
  00907311  8B EC          MOV EBP,ESP
  00907313  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00907310
string literals (0):
direct callees (1):
  0x00844560  (unnamed)
struct offsets touched (top 2):
  +0x68 [dec 104]  x1  via EAX
  +0x310 [dec 784]  x1  via ECX
```

## Card B0-3-17

GOG function: CSWVirtualMachineCommands::ExecuteCommandFloatToInt  gog 0x0078ebd0  callconv=__thiscall  param_size_bytes=8  gog_size=80  gog_retn=8
Candidate Steam address: 0x00679310 (size 724, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandFloatingText
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006795f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x10}
imm sets (* = in both, 2 common): GOG {0x8*,0xc,0xfffff82f*,0xfffff830} | Steam {0x3,0x4,0x8*,0x9,0xb,0x34,0x5d,0x5e,0x68,0x20d,0x20e,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00679310: 16-byte aligned: YES
  00679310  55             PUSH EBP
  00679311  8B EC          MOV EBP,ESP
  00679313  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00679310
string literals (0):
direct callees (16):
  0x00733540  CExoString::CExoString(empty)
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733780  CExoString::~CExoString
  0x006FDA70  CVirtualMachine::StackPopString
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0A0  CServerExoApp::GetGameObject
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E1BD0  (unnamed)
  0x005E1CB0  CSWCMessagePayload::SetString
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x0051C2E0  (unnamed)
  0x00648CC0  (unnamed)
  0x005E25D0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x10 [dec 16]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00679310 FUN_00679310 size=724 ret=8 callers=0 callees=16

undefined4 FUN_00679310(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int local_74;
  undefined1 local_2c [8];
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  int *local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a2a3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  local_1c = 1;
  if (param_1 == 0x20d) {
    iVar1 = FUN_006fd9a0(&local_20);
    if (iVar1 == 0) {
      local_8 = 0xffffffff;
      FUN_00733780();
      ExceptionList = local_10;
      return 0xfffff82f;
    }
    local_24 = local_20;
  }
  else if ((param_1 == 0x20e) && (iVar1 = FUN_006fda70(local_2c), iVar1 == 0)) {
    local_8 = 0xffffffff;
    FUN_00733780();
    ExceptionList = local_10;
    return 0xfffff82f;
  }
  iVar1 = FUN_006fdaf0(&local_14);
  if (iVar1 == 0) {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = 0xfffff82f;
  }
  else if ((param_2 < 3) || (iVar1 = FUN_006fd9a0(&local_1c), iVar1 != 0)) {
    local_18 = (int *)FUN_0051c0a0(local_14);
    if ((local_18 != (int *)0x0) &&
       (((char)local_18[2 = byte +0x8] == DAT_00994474 &&
        (iVar1 = (**(code **)(*local_18 + 0x10))(), iVar1 != 0)))) {
      iVar1 = FUN_00919723(0x34);
      local_8._0_1_ = 1;
      if (iVar1 == 0) {
        local_74 = 0;
      }
      else {
        local_74 = FUN_005e2540();
      }
      local_8 = (uint)local_8._1_3_ << 8;
      if (local_74 != 0) {
        FUN_005e1c70(0,local_14);
        if (param_1 == 0x20d) {
          FUN_005e1bd0(9,0x5d);
          FUN_005e1bd0(0,local_24);
        }
        else if (param_1 == 0x20e) {
          FUN_005e1bd0(9,0x5e);
          FUN_005e1cb0(0,local_2c);
        }
        if ((local_1c == 0) && (iVar1 = FUN_0051c600(local_14), iVar1 != 0)) {
          FUN_0051c2e0(*(undefined4 *)(iVar1 + 4),0xb,local_74,0);
          FUN_00648cc0();
        }
        if (local_74 != 0) {
          FUN_005e25d0();
          FUN_009196fd(local_74);
        }
      }
    }
/* ... 12 more lines truncated by --max-lines */
```

### decoy: 0x006795f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006795f0: 16-byte aligned: YES
  006795f0  55             PUSH EBP
  006795f1  8B EC          MOV EBP,ESP
  006795f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006795f0
string literals (0):
direct callees (3):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0091C860  _ftol2_sse
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

## Card B0-3-18

GOG function: CSWCAreaOfEffectObject::Destructor_2  gog 0x00582200  callconv=(blank)  param_size_bytes=0  gog_size=156  gog_retn=0
Candidate Steam address: 0x008ee820 (size 156, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008ee8c0:STR0/CALLEE0/SIG0.022/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005821d0 CSWCAreaOfEffectObject::Destructor -> 0x008ee7f0  gate_class=REVIEW
- GOG-order neighbour after:  0x005822a0 CSWCAreaOfEffectObject::AIUpdate -> 0x008ee8c0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00472750 CSWCObject::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00583f60 CSWCAreaOfEffectObject::UnloadModel -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x154*} | Steam {0x154*}
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
prologue for 0x008ee820: 16-byte aligned: YES
  008ee820  55             PUSH EBP
  008ee821  8B EC          MOV EBP,ESP
  008ee823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008ee820
string literals (0):
direct callees (2):
  0x008F0580  CSWCAreaOfEffectObject::UnloadModel
  0x007ED5F0  CSWCObject::Destructor_2
struct offsets touched (top 1):
  +0x154 [dec 340]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008ee820 FUN_008ee820 size=156 ret=0 callers=1 callees=2

void __fastcall FUN_008ee820(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0097df78;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWCAreaOfEffectObject::vftable;
  local_8 = 0;
  if ((param_1[0x55 = byte +0x154] != 0) && ((undefined4 *)param_1[0x55 = byte +0x154] != (undefined4 *)0x0)) {
    (*(code *)**(undefined4 **)param_1[0x55 = byte +0x154])(1,uVar1);
  }
  FUN_008f0580();
  local_8 = 0xffffffff;
  FUN_007ed5f0();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008ee8c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008ee8c0: 16-byte aligned: YES
  008ee8c0  55             PUSH EBP
  008ee8c1  8B EC          MOV EBP,ESP
  008ee8c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008ee8c0
string literals (2):
  0x0099AB38  at 008eea05  'SoundImpact'
  0x0099AB44  at 008eea82  'SoundDuration'
direct callees (23):
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0051AF60  CWorldTimer::SubtractWorldTimes
  0x0073F4D0  ClientApp::GetClientObject
  0x00733540  CExoString::CExoString(empty)
  0x00733570  CExoString::CExoString_char
  0x0071B1D0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x00710810  (unnamed)
  0x0070BA90  (unnamed)
  0x00919723  operator new
  0x0070AB90  (unnamed)
  0x0070ACF0  (unnamed)
  0x0070ACC0  (unnamed)
  0x0070AE30  (unnamed)
  0x0070AE90  (unnamed)
  0x0070AD50  (unnamed)
  0x008F07C0  CSWCAreaOfEffectObject::SetAnimation
  0x008F0FA0  CSWCAreaOfEffectObject::SetMoveTimer
struct offsets touched (top 10):
  +0x154 [dec 340]  x9  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x130 [dec 304]  x6  via ECX
  +0x44 [dec 68]  x5  via EAX,EDX
  +0x104 [dec 260]  x5  via EAX,EDX
  +0x14C [dec 332]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x20 [dec 32]  x2  via EAX
  +0x3C [dec 60]  x2  via ECX
  +0x40 [dec 64]  x2  via EAX,EDX
```

## Card B0-3-19

GOG function: CSWGuiUpgrade::OnPanelRemoved  gog 0x005bce30  callconv=(blank)  param_size_bytes=0  gog_size=258  gog_retn=0
Candidate Steam address: 0x008cca70 (size 258, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswguiupgrade_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (OnPanelRemoved 0x40f4c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008cbde0:STR0/CALLEE0/SIG0.094/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005bc1a0 CSWGuiUpgrade::OnPanelAdded -> 0x008cbde0  gate_class=AUTO
- GOG-order neighbour after:  0x005bcf40 CSWGuiUpgrade::OnControlEntered -> 0x008ccb80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x3d2c*,0x3d34*,0x3d38*,0x3d40*,0x3d4c*,0x3d54*} | Steam {0x3d2c*,0x3d34*,0x3d38*,0x3d40*,0x3d4c*,0x3d54*}
imm sets (* = in both, 2 common): GOG {0x6*,0x20*} | Steam {0x6*,0x20*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008cca70: 16-byte aligned: YES
  008cca70  55             PUSH EBP
  008cca71  8B EC          MOV EBP,ESP
  008cca73  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008cca70
string literals (0):
direct callees (1):
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
struct offsets touched (top 5):
  +0x3D38 [dec 15672]  x3  via EAX,ECX,EDX
  +0x3D2C [dec 15660]  x1  via EAX
  +0x3D34 [dec 15668]  x1  via ECX
  +0x3D40 [dec 15680]  x1  via EAX
  +0x3D4C [dec 15692]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 008cca70 FUN_008cca70 size=258 ret=0 callers=0 callees=1

void __fastcall FUN_008cca70(int param_1)

{
  undefined4 *puVar1;
  int local_8;
  
  *(undefined1 *)(param_1 + 0x3d2c) = 0;
  *(undefined4 *)(param_1 + 0x3d34) = 0;
  if (*(int *)(param_1 + 0x3d38) != 0) {
    if (*(undefined4 **)(param_1 + 0x3d38) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x3d38))(1);
    }
    *(undefined4 *)(param_1 + 0x3d38) = 0;
  }
  for (local_8 = 0; local_8 < 6; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x3d54 + local_8 * 4) != 0) {
      puVar1 = *(undefined4 **)(param_1 + 0x3d54 + local_8 * 4);
      if (puVar1 != (undefined4 *)0x0) {
        (**(code **)*puVar1)(1);
      }
      *(undefined4 *)(param_1 + 0x3d54 + local_8 * 4) = 0;
    }
  }
  *(undefined4 *)(param_1 + 0x3d40) = 0;
  *(undefined4 *)(param_1 + 0x3d4c) = 0;
  FUN_0040f4c0();
  return;
}
```

### decoy: 0x008cbde0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008cbde0: 16-byte aligned: YES
  008cbde0  55             PUSH EBP
  008cbde1  8B EC          MOV EBP,ESP
  008cbde3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008cbde0
string literals (6):
  0x009A893C  at 008cc133  'LongMdlVar'
  0x009A8930  at 008cc145  'ShortMdlVar'
  0x009A8920  at 008cc157  'DoubleMdlVar'
  0x009A8914  at 008cc27a  'Template'
  0x009973C8  at 008cc4eb  'UpgradeType'
  0x009A8914  at 008cc52e  'Template'
direct callees (33):
  0x00733540  CExoString::CExoString(empty)
  0x0073F350  CClientExoApp::GetClientLanguage
  0x007356B0  CExoLocString::GetString_3
  0x00734190  CExoString::Find_2
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x007FEEC0  CSWCItem::non_virtual_thunk_to_dtor_CSWCItem
  0x0071EBD0  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x00733570  CExoString::CExoString_char
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005FFD60  CSWSItem::Constructor
  0x00606600  CSWSItem::CopyItem
  0x006D6E30  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x0071B1D0  (unnamed)
  0x00734D60  CExoString::CompareNoCase
  0x00710810  (unnamed)
  0x00601610  CSWSItem::LoadFromTemplate
struct offsets touched (top 10):
  +0x48 [dec 72]  x15  via EAX,ECX,EDX
  +0x3D34 [dec 15668]  x8  via EAX,ECX,EDX
  +0x3D2C [dec 15660]  x5  via ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x3DB4 [dec 15796]  x3  via EAX,ECX,EDX
  +0x3DB8 [dec 15800]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x24 [dec 36]  x2  via ECX
  +0x38 [dec 56]  x2  via EDX
  +0x5C [dec 92]  x2  via EAX,ECX
```

## Card B0-3-20

GOG function: CSWGuiInGameInventory::Destructor  gog 0x0059af30  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x008a7030 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswguiingameinventory_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008a7350:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0059a070 CSWGuiInGameInventory::Constructor -> 0x008a6170  gate_class=AUTO
- GOG-order neighbour after:  0x0059af60 CSWGuiInGameInventory::Destructor_2 -> 0x008a7060  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0059af60 CSWGuiInGameInventory::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x008a7030: 16-byte aligned: YES
  008a7030  55             PUSH EBP
  008a7031  8B EC          MOV EBP,ESP
  008a7033  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008a7030
string literals (0):
direct callees (2):
  0x008A7060  CSWGuiInGameInventory::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008a7030 FUN_008a7030 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_008a7030(undefined4 param_1,uint param_2)

{
  FUN_008a7060();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x008a7350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008a7350: 16-byte aligned: YES
  008a7350  55             PUSH EBP
  008a7351  8B EC          MOV EBP,ESP
  008a7353  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008a7350
string literals (0):
direct callees (6):
  0x008A7BA0  CSWGuiInGameInventory::PopulateItemListBox
  0x00623320  CSWSEncounter::AIUpdate
  0x0051C8F0  (unnamed)
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00417010  (unnamed)
  0x0040EC90  CSWGuiPanel::Draw
struct offsets touched (top 5):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x24D0 [dec 9424]  x2  via EAX,EDX
  +0xD80 [dec 3456]  x1  via EDX
  +0x11D8 [dec 4568]  x1  via EAX
```

## Card B0-3-21

GOG function: CSWSEffectListHandler::OnRemoveAreaOfEffect  gog 0x00741de0  callconv=(blank)  param_size_bytes=8  gog_size=176  gog_retn=8
Candidate Steam address: 0x0059f120 (size 37, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnRemoveVisualEffect
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059f150:STR0/CALLEE0/SIG0.143/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00741a40 CSWSEffectListHandler::OnApplyAreaOfEffect -> 0x0059f150  gate_class=REVIEW
- GOG-order neighbour after:  0x00741e90 CSWSEffectListHandler::OnApplyBeam -> 0x0059f5a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x00734ba0 CGameEffect::GetObjectID -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x30} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0x1c} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059f120: 16-byte aligned: YES
  0059f120  55             PUSH EBP
  0059f121  8B EC          MOV EBP,ESP
  0059f123  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059f120
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544180  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0059f120 FUN_0059f120 size=37 ret=8 callers=0 callees=2

undefined4 FUN_0059f120(void)

{
  undefined4 uVar1;
  
  uVar1 = FUN_005e4e60(0);
  FUN_00544180(uVar1);
  return 1;
}
```

### decoy: 0x0059f150 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059f150: 16-byte aligned: YES
  0059f150  55             PUSH EBP
  0059f151  8B EC          MOV EBP,ESP
  0059f153  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0059f150
string literals (0):
direct callees (13):
  0x00919723  operator new
  0x0062BFC0  CSWSAreaOfEffectObject::Constructor
  0x0062EE30  CSWSAreaOfEffectObject::SetCreator
  0x005E4E60  CGameEffect::GetInteger
  0x0062D750  CSWSAreaOfEffectObject::LoadAreaEffect
  0x005E4F20  CGameEffect::GetString
  0x00733B60  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007337C0  CExoString::operator=
  0x005453C0  CSWSObject::GetArea
  0x0062D540  CSWSAreaOfEffectObject::SetTargetObjID
  0x0062C800  CSWSAreaOfEffectObject::AddToArea
  0x005E4F00  CGameEffect::SetObjectID
struct offsets touched (top 6):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0x30 [dec 48]  x1  via EDX
  +0xBC [dec 188]  x1  via EDX
```

## Card B0-3-22

GOG function: CExoResMan::RemoveFixedKeyTableFile  gog 0x0061b050  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x007116f0 (size 27, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007116d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061b030 CExoResMan::RemoveResourceImageFile -> 0x007116d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0061b070 CExoResMan::RemoveResourceDirectory -> 0x00711710  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061cb30 CExoResMan::RemoveKeyTable -> Steam ?  in candidate callees: n/a
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
prologue for 0x007116f0: 16-byte aligned: YES
  007116f0  55             PUSH EBP
  007116f1  8B EC          MOV EBP,ESP
  007116f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007116f0
string literals (0):
direct callees (1):
  0x007131D0  CExoResMan::RemoveKeyTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007116f0 FUN_007116f0 size=27 ret=4 callers=2 callees=1
THUNK -> 0x007131D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007116f0(undefined4 param_1)

{
  FUN_007131d0(param_1,1);
  return;
}
```

### decoy: 0x007116d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007116d0: 16-byte aligned: YES
  007116d0  55             PUSH EBP
  007116d1  8B EC          MOV EBP,ESP
  007116d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007116d0
string literals (0):
direct callees (1):
  0x007131D0  CExoResMan::RemoveKeyTable
struct offsets touched (top 0):
```

## Card B0-3-23

GOG function: CExoResMan::RemoveResourceDirectory  gog 0x0061b070  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x00711710 (size 27, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (DoQuickLoad 0x7d0f50)
- best decoy (neighbour/other candidate) scores: 0x00711730:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061b050 CExoResMan::RemoveFixedKeyTableFile -> 0x007116f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0061b090 CExoResMan::UpdateResourceDirectory -> 0x00711730  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061cb30 CExoResMan::RemoveKeyTable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x2*,0x4*} | Steam {0x2*,0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00711710: 16-byte aligned: YES
  00711710  55             PUSH EBP
  00711711  8B EC          MOV EBP,ESP
  00711713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711710
string literals (0):
direct callees (1):
  0x007131D0  CExoResMan::RemoveKeyTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00711710 FUN_00711710 size=27 ret=4 callers=23 callees=1
THUNK -> 0x007131D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00711710(undefined4 param_1)

{
  FUN_007131d0(param_1,2);
  return;
}
```

### decoy: 0x00711730 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711730: 16-byte aligned: YES
  00711730  55             PUSH EBP
  00711731  8B EC          MOV EBP,ESP
  00711733  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711730
string literals (0):
direct callees (1):
  0x007144A0  CExoResMan::UpdateKeyTable
struct offsets touched (top 0):
```

## Card B0-3-24

GOG function: CExoResMan::AddResourceDirectory  gog 0x0061aff0  callconv=(blank)  param_size_bytes=4  gog_size=29  gog_retn=4
Candidate Steam address: 0x00711690 (size 29, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (Constructor 0x850770)
- best decoy (neighbour/other candidate) scores: 0x00711710:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061afd0 CExoResMan::AddFixedKeyTableFile -> 0x00711670  gate_class=REVIEW
- GOG-order neighbour after:  0x0061b010 CExoResMan::RemoveEncapsulatedResourceFile -> 0x007116b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061b0b0 CExoResMan::AddKeyTable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x2*,0x4*} | Steam {0x2*,0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00711690: 16-byte aligned: YES
  00711690  55             PUSH EBP
  00711691  8B EC          MOV EBP,ESP
  00711693  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711690
string literals (0):
direct callees (1):
  0x00711750  CExoResMan::AddKeyTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00711690 FUN_00711690 size=29 ret=4 callers=23 callees=1
THUNK -> 0x00711750 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00711690(undefined4 param_1)

{
  FUN_00711750(param_1,2,0);
  return;
}
```

### decoy: 0x00711710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711710: 16-byte aligned: YES
  00711710  55             PUSH EBP
  00711711  8B EC          MOV EBP,ESP
  00711713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711710
string literals (0):
direct callees (1):
  0x007131D0  CExoResMan::RemoveKeyTable
struct offsets touched (top 0):
```

## Card B0-3-25

GOG function: CSWSEffectListHandler::OnRemoveAreaOfEffect  gog 0x00741de0  callconv=(blank)  param_size_bytes=8  gog_size=176  gog_retn=8
Candidate Steam address: 0x0059f6f0 (size 18, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyForceResistanceIncrease
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059f5a0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00741a40 CSWSEffectListHandler::OnApplyAreaOfEffect -> 0x0059f150  gate_class=REVIEW
- GOG-order neighbour after:  0x00741e90 CSWSEffectListHandler::OnApplyBeam -> 0x0059f5a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x00734ba0 CGameEffect::GetObjectID -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x30} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0x1c} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059f6f0: 16-byte aligned: YES
  0059f6f0  55             PUSH EBP
  0059f6f1  8B EC          MOV EBP,ESP
  0059f6f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059f6f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0059f6f0 FUN_0059f6f0 size=18 ret=8 callers=3 callees=0

undefined4 FUN_0059f6f0(void)

{
  return 1;
}
```

### decoy: 0x0059f5a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059f5a0: 16-byte aligned: YES
  0059f5a0  55             PUSH EBP
  0059f5a1  8B EC          MOV EBP,ESP
  0059f5a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0059f5a0
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4EE0  CGameEffect::GetObjectID
  0x005E4F00  CGameEffect::SetObjectID
  0x005E4E60  CGameEffect::GetInteger
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 1):
  +0x8 [dec 8]  x5  via ECX
```

## Card B0-3-26

GOG function: CRes::Demand  gog 0x0061ab40  callconv=__thiscall  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x007111e0 (size 26, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:R1-static+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (Constructor_2 0x716fd0,Load 0x93ffb0)
- best decoy (neighbour/other candidate) scores: 0x007111c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061ab20 CRes::CancelRequest -> 0x007111c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0061ab60 CRes::ReadRaw  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061b580 CExoResMan::Demand -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007111e0: 16-byte aligned: YES
  007111e0  55             PUSH EBP
  007111e1  8B EC          MOV EBP,ESP
  007111e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007111e0
string literals (0):
direct callees (1):
  0x00711C20  CExoResMan::Demand
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007111e0 FUN_007111e0 size=26 ret=0 callers=28 callees=1
THUNK -> 0x00711C20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007111e0(undefined4 param_1)

{
  FUN_00711c20(param_1);
  return;
}
```

### decoy: 0x007111c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007111c0: 16-byte aligned: YES
  007111c0  55             PUSH EBP
  007111c1  8B EC          MOV EBP,ESP
  007111c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007111c0
string literals (0):
direct callees (1):
  0x00711A30  CExoResMan::CancelRequest
struct offsets touched (top 0):
```

## Card B0-3-27

GOG function: CRes::CancelRequest  gog 0x0061ab20  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x007111c0 (size 26, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007111a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061ab00 CRes::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061ab40 CRes::Demand -> 0x007111e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061b390 CExoResMan::CancelRequest -> Steam ?  in candidate callees: n/a
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
prologue for 0x007111c0: 16-byte aligned: YES
  007111c0  55             PUSH EBP
  007111c1  8B EC          MOV EBP,ESP
  007111c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007111c0
string literals (0):
direct callees (1):
  0x00711A30  CExoResMan::CancelRequest
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007111c0 FUN_007111c0 size=26 ret=0 callers=37 callees=1
THUNK -> 0x00711A30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007111c0(undefined4 param_1)

{
  FUN_00711a30(param_1);
  return;
}
```

### decoy: 0x007111a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007111a0: 16-byte aligned: YES
  007111a0  55             PUSH EBP
  007111a1  8B EC          MOV EBP,ESP
  007111a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007111a0
string literals (0):
direct callees (1):
  0x00711230  (unnamed)
struct offsets touched (top 0):
```

## Card B0-3-28

GOG function: CSWGuiUpgrade::SetDescription  gog 0x005bd130  callconv=(blank)  param_size_bytes=4  gog_size=250  gog_retn=4
Candidate Steam address: 0x008ccd70 (size 250, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswguiupgrade_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008cce70:STR0/CALLEE0/SIG0.233/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005bcf40 CSWGuiUpgrade::OnControlEntered -> 0x008ccb80  gate_class=AUTO
- GOG-order neighbour after:  0x005bd230 CSWGuiUpgrade::Draw -> 0x008cce70  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00915200 CSWGuiTextParams::SetText -> Steam ?  in candidate callees: n/a
  0x00915c80 CSWGuiText::GetFontHeight -> Steam ?  in candidate callees: n/a
  0x0091b2a0 CSWGuiListBox::ClearItems -> Steam ?  in candidate callees: n/a
  0x0091c940 CSWGuiListBox::SetActiveControl -> Steam ?  in candidate callees: n/a
  0x0091dd30 CSWGuiListBox::AddControls_2 -> Steam ?  in candidate callees: n/a
  0x00921b40 Global::strlen -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x2ee0*} | Steam {0x4*,0x8*,0xc*,0x2ee0*}
imm sets (* = in both, 7 common): GOG {0x4*,0x24*,0x2670*,0x2ee0*,0x2ee4*,0x2fb8*,0x2fd0*} | Steam {0x4*,0x24*,0x2670*,0x2ee0*,0x2ee4*,0x2fb8*,0x2fd0*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008ccd70: 16-byte aligned: YES
  008ccd70  55             PUSH EBP
  008ccd71  8B EC          MOV EBP,ESP
  008ccd73  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008ccd70
string literals (0):
direct callees (6):
  0x00416E30  CSWGuiTextParams::SetText
  0x0041D290  (unnamed)
  0x00919F10  _strlen
  0x004178B0  (unnamed)
  0x0041FD90  (unnamed)
  0x0041E9A0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x2EE0 [dec 12000]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008ccd70 FUN_008ccd70 size=250 ret=4 callers=2 callees=6

void __thiscall FUN_008ccd70(int param_1,int *param_2)

{
  size_t local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  FUN_00416e30(param_2);
  local_18 = param_1 + 0x2670;
  FUN_0041d290();
  local_14 = *(undefined4 *)(param_1 + 0x2ee4);
  local_10 = *(undefined4 *)(param_1 + 0x2ee8);
  local_c = *(undefined4 *)(param_1 + 0x2eec);
  local_8 = *(undefined4 *)(param_1 + 0x2ef0);
  if (*param_2 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = _strlen [GHIDRA-AUTO-LABEL: unreliable]((char *)*param_2);
  }
  if ((int)local_20 < 1) {
    local_8 = FUN_004178b0();
  }
  else {
    local_8 = (**(code **)(*(int *)(param_1 + 0x2fb8) + 8))();
  }
  (**(code **)(*(int *)(param_1 + 0x2ee0 [dec 12000]) + 4))(&local_14);
  local_1c = param_1 + 0x2ee0 [dec 12000];
  FUN_0041fd90(&local_1c,1,0,0,0);
  FUN_0041e9a0(param_1 + 0x2ee0 [dec 12000],0);
  return;
}
```

### decoy: 0x008cce70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008cce70: 16-byte aligned: YES
  008cce70  55             PUSH EBP
  008cce71  8B EC          MOV EBP,ESP
  008cce73  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008cce70
string literals (0):
direct callees (3):
  0x004184B0  (unnamed)
  0x00515620  (unnamed)
  0x0040EC90  CSWGuiPanel::Draw
struct offsets touched (top 7):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x9  via EAX,ECX,EDX
  +0x60 [dec 96]  x1  via EDX
  +0x68 [dec 104]  x1  via EAX
  +0x108 [dec 264]  x1  via EAX
  +0x3D2C [dec 15660]  x1  via EAX
```

## Card B0-3-29

GOG function: CSWCDoor::Destructor  gog 0x00506a50  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0087b170 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0087b240:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005068c0 CSWCDoor::Constructor -> 0x0087afe0  gate_class=AUTO
- GOG-order neighbour after:  0x00506a80 CSWCDoor::Destructor_2 -> 0x0087b1a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00506a80 CSWCDoor::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x0087b170: 16-byte aligned: YES
  0087b170  55             PUSH EBP
  0087b171  8B EC          MOV EBP,ESP
  0087b173  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0087b170
string literals (0):
direct callees (2):
  0x0087B1A0  CSWCDoor::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0087b170 FUN_0087b170 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0087b170(undefined4 param_1,uint param_2)

{
  FUN_0087b1a0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0087b240 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0087b240: 16-byte aligned: YES
  0087b240  55             PUSH EBP
  0087b241  8B EC          MOV EBP,ESP
  0087b243  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0087b240
string literals (0):
direct callees (11):
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0051AF60  CWorldTimer::SubtractWorldTimes
  0x0087B3F0  CSWCDoor::AnimationUpdate
  0x00623320  CSWSEncounter::AIUpdate
  0x0087C9F0  CSWCDoor::UpdateAreaTransitionDisplay
  0x007F12D0  CSWCObject::FadeObject
  0x0073F790  CClientExoApp::GetClientAIMaster
  0x007A21D0  CClientExoAppInternal::SetGameOver
  0x0073F4B0  CClientExoApp::GetGameObjectArray
  0x0053DED0  (unnamed)
struct offsets touched (top 7):
  +0x4 [dec 4]  x5  via EAX,ECX
  +0xF0 [dec 240]  x5  via EAX,ECX,EDX
  +0x44 [dec 68]  x3  via EAX,EDX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x40 [dec 64]  x2  via EDX
  +0xC4 [dec 196]  x1  via EDX
  +0xE8 [dec 232]  x1  via EDX
```

## Card B0-3-30

GOG function: CResGFF::Destructor  gog 0x00561f80  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00716fa0 (size 44, retn 4)
Candidate sources: branch+name_map:medium:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (Destructor_2 0x7171b0)
- best decoy (neighbour/other candidate) scores: 0x00716e60:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00561d90 2012_::SetResRef  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005620a0 CSWCRoom::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623310 CResGFF::Destructor_2 -> Steam 0x007171b0  in candidate callees: yes
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
prologue for 0x00716fa0: 16-byte aligned: YES
  00716fa0  55             PUSH EBP
  00716fa1  8B EC          MOV EBP,ESP
  00716fa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00716fa0
string literals (0):
direct callees (2):
  0x007171B0  CResGFF::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00716fa0 FUN_00716fa0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00716fa0(undefined4 param_1,uint param_2)

{
  FUN_007171b0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x00716e60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00716e60: 16-byte aligned: YES
  00716e60  55             PUSH EBP
  00716e61  8B EC          MOV EBP,ESP
  00716e63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00716e60
string literals (0):
direct callees (2):
  0x007110D0  CRes::CRes
  0x0091C630  (unnamed)
struct offsets touched (top 10):
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via ECX
  +0x38 [dec 56]  x1  via EDX
  +0x3C [dec 60]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
  +0x44 [dec 68]  x1  via EDX
  +0x48 [dec 72]  x1  via EAX
  +0x4C [dec 76]  x1  via ECX
```

## Card B0-3-31

GOG function: CExoResMan::RemoveEncapsulatedResourceFile  gog 0x0061b010  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x007116b0 (size 27, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007116d0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061aff0 CExoResMan::AddResourceDirectory -> 0x00711690  gate_class=REVIEW
- GOG-order neighbour after:  0x0061b030 CExoResMan::RemoveResourceImageFile -> 0x007116d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061cb30 CExoResMan::RemoveKeyTable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x3*,0x4*} | Steam {0x3*,0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007116b0: 16-byte aligned: YES
  007116b0  55             PUSH EBP
  007116b1  8B EC          MOV EBP,ESP
  007116b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007116b0
string literals (0):
direct callees (1):
  0x007131D0  CExoResMan::RemoveKeyTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007116b0 FUN_007116b0 size=27 ret=4 callers=3 callees=1
THUNK -> 0x007131D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007116b0(undefined4 param_1)

{
  FUN_007131d0(param_1,3);
  return;
}
```

### decoy: 0x007116d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007116d0: 16-byte aligned: YES
  007116d0  55             PUSH EBP
  007116d1  8B EC          MOV EBP,ESP
  007116d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007116d0
string literals (0):
direct callees (1):
  0x007131D0  CExoResMan::RemoveKeyTable
struct offsets touched (top 0):
```

## Card B0-3-32

GOG function: CSWSCreatureStats::GetClassByPosition  gog 0x00588990  callconv=(blank)  param_size_bytes=1  gog_size=60  gog_retn=4
Candidate Steam address: 0x00844e40 (size 51, retn 4)
Candidate sources: branch+name_map:high:gog_creaturestats_cluster    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreatureStats::GetClass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.20 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00844e80:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00588540 CSWGuiSkillFlowChart::CreatePowerChart  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005889d0 CSWGuiSkillFlowChart::AddPowerSet  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8d,0xab} | Steam {0xf8}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*,0x28} | Steam {0x2,0x4*,0x5,0x8*}

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

## Card B0-3-33

GOG function: CSWCDoor::GetServerDoor  gog 0x00507760  callconv=(blank)  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x0087be80 (size 53, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (GetServerObject 0x7f2540)
- best decoy (neighbour/other candidate) scores: 0x0087bd90:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00507670 CSWCDoor::GetUseItems -> 0x0087bd90  gate_class=AUTO
- GOG-order neighbour after:  0x005077a0 CSWCDoor::GetTargetActions -> 0x0087bec0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004776e0 CSWCObject::GetServerObject -> Steam 0x007f2540  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x18*} | Steam {0x18*}
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
prologue for 0x0087be80: 16-byte aligned: YES
  0087be80  55             PUSH EBP
  0087be81  8B EC          MOV EBP,ESP
  0087be83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0087be80
string literals (0):
direct callees (1):
  0x007F2540  CSWCObject::GetServerObject
struct offsets touched (top 1):
  +0x18 [dec 24]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0087be80 FUN_0087be80 size=53 ret=0 callers=6 callees=1

undefined4 FUN_0087be80(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
  iVar1 = FUN_007f2540();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_007f2540();
    uVar3 = (**(code **)(*piVar2 + 0x18))(piVar2,uVar3);
  }
  return uVar3;
}
```

### decoy: 0x0087bd90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0087bd90: 16-byte aligned: YES
  0087bd90  55             PUSH EBP
  0087bd91  8B EC          MOV EBP,ESP
  0087bd93  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0087bd90
string literals (0):
direct callees (3):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00845730  (unnamed)
  0x0058A610  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x10C [dec 268]  x1  via EDX
  +0x310 [dec 784]  x1  via EAX
```

## Card B0-3-34

GOG function: CGuiInGame::GetMiniMapVisible  gog 0x004e0c60  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x007d0560 (size 49, retn 0)
Candidate sources: mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::RemoveGUIPopUps
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007d0530:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0c30 CGuiInGame::SetCombatMessage  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004e0ca0 CGuiInGame::RemoveGUIPopUps -> 0x007d05a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x3c*,0x40*,0x84*} | Steam {0x3c*,0x40*,0x84*}
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
prologue for 0x007d0560: 16-byte aligned: YES
  007d0560  55             PUSH EBP
  007d0561  8B EC          MOV EBP,ESP
  007d0563  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d0560
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x3C [dec 60]  x3  via EAX,EDX
  +0x40 [dec 64]  x1  via ECX
  +0x84 [dec 132]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007d0560 FUN_007d0560 size=49 ret=0 callers=1 callees=0

void __fastcall FUN_007d0560(int param_1)

{
  if (*(int *)(param_1 + 0x3c) == *(int *)(param_1 + 0x40)) {
    (**(code **)(**(int **)(param_1 + 0x3c) + 0x84))();
  }
  return;
}
```

### decoy: 0x007d0530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d0530: 16-byte aligned: YES
  007d0530  55             PUSH EBP
  007d0531  8B EC          MOV EBP,ESP
  007d0533  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d0530
string literals (0):
direct callees (1):
  0x0074EC10  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x2  via EAX,EDX
```

## Card B0-3-35

GOG function: CSWCPlaceable::GetServerPlaceable  gog 0x00536580  callconv=(blank)  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x00842d00 (size 53, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (GetServerObject 0x7f2540,ResetDialogAnimations 0x7c93e0,UpdateNameLabel 0x742a00)
- best decoy (neighbour/other candidate) scores: 0x00842bd0:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00536450 CSWCPlaceable::MenuActionSetPlaceableMine -> 0x00842bd0  gate_class=AUTO
- GOG-order neighbour after:  0x005365c0 CSWCPlaceable::GetTargetActions -> 0x00842d40  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004776e0 CSWCObject::GetServerObject -> Steam 0x007f2540  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x48*} | Steam {0x48*}
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
prologue for 0x00842d00: 16-byte aligned: YES
  00842d00  55             PUSH EBP
  00842d01  8B EC          MOV EBP,ESP
  00842d03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00842d00
string literals (0):
direct callees (1):
  0x007F2540  CSWCObject::GetServerObject
struct offsets touched (top 1):
  +0x48 [dec 72]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00842d00 FUN_00842d00 size=53 ret=0 callers=20 callees=1

undefined4 FUN_00842d00(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
  iVar1 = FUN_007f2540();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_007f2540();
    uVar3 = (**(code **)(*piVar2 + 0x48))(piVar2,uVar3);
  }
  return uVar3;
}
```

### decoy: 0x00842bd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00842bd0: 16-byte aligned: YES
  00842bd0  55             PUSH EBP
  00842bd1  8B EC          MOV EBP,ESP
  00842bd3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00842bd0
string literals (0):
direct callees (11):
  0x0051C8F0  (unnamed)
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005453C0  CSWSObject::GetArea
  0x0052E270  CSWSArea::GetPlayerCanSetMines
  0x0073F750  ClientApp::GetGuiInGame
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410530  CSWGuiManager::AddPanel
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x00775600  CSWCCreature::PerformUseItem
struct offsets touched (top 4):
  +0x4 [dec 4]  x5  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
  +0xA0 [dec 160]  x1  via ECX
```

## Card B0-3-36

GOG function: CSWGuiSaveLoad::LoadPCAutoSave  gog 0x005d1d20  callconv=(blank)  param_size_bytes=4  gog_size=2923  gog_retn=4
Candidate Steam address: 0x008529e0 (size 2923, retn 4)
Candidate sources: branch+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=LoadAutosaveSlot
- anchors: A-STR 2 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'TEMP:pifo'@00994208; 'AUTOSAVEPARAMS'@00994214; CALLEE 5 (AddMoveToModuleMovie 0x740180,Constructor_2 0x716fd0,GetStructFromStruct 0x718960); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00853550:STR0/CALLEE1/SIG0.155/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d1cd0 Global::LoadScreenUpdate  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d2890 KOTOR_AUTOSAVE_PARAMS::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (45), mapped to Steam through trusted rows only:
  0x004099e0 Global::UpdateScreen -> Steam ?  in candidate callees: n/a
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x0040d640 CClientExoApp::AddMoveToModuleMovie -> Steam 0x00740180  in candidate callees: yes
  0x0040da60 CClientExoApp::SetLoadMusic -> Steam ?  in candidate callees: n/a
  0x0040da90 CClientExoApp::SetLoadBarProgress -> Steam ?  in candidate callees: n/a
  0x0040dab0 CClientExoApp::SetLoadFromIFO -> Steam ?  in candidate callees: n/a
  0x0040e0f0 CClientExoApp::SetLoadStep -> Steam ?  in candidate callees: n/a
  0x004dfab0 CGuiInGame::SetStatusSummary -> Steam ?  in candidate callees: n/a
  ... 37 more GOG callees
GOG strings (9): 'TEMP:'; 'SAVES:'; '%06d - %s'; 'GAMEINPROGRESS:' ...
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x48*} | Steam {0x4*,0x8*,0x48*}
imm sets (* = in both, 35 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+21)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+21)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008529e0: 16-byte aligned: YES
  008529e0  55             PUSH EBP
  008529e1  8B EC          MOV EBP,ESP
  008529e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008529e0
string literals (11):
  0x00993F50  at 00852a97  'AUTOSAVE'
  0x00993E38  at 00852a9e  '%06d - %s'
  0x00993E30  at 00852aaf  'SAVES:'
  0x00993E84  at 00852b03  'GAMEINPROGRESS:'
  0x00993E84  at 00852b50  'GAMEINPROGRESS:'
  0x00993E14  at 00852bec  'SAVEGAME'
  0x00993E84  at 00852d16  'GAMEINPROGRESS:'
  0x00993E5C  at 00852d5c  'TEMP:'
  0x00994208  at 00852e62  'TEMP:pifo'
  0x009940B8  at 00852fd0  'savenfo'
  0x00994214  at 00853073  'AUTOSAVEPARAMS'
direct callees (45):
  0x00733540  CExoString::CExoString(empty)
  0x0051C500  (unnamed)
  0x0051BF20  (unnamed)
  0x007405D0  CClientExoApp::SetLoadBarProgress
  0x00734270  CExoString::Format
  0x007338D0  CExoString::operator=_2
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x00714F30  CExoResMan::CreateDirectory2
  0x007156E0  CExoResMan::CleanDirectory
  0x00919723  operator new
  0x00720B60  (unnamed)
  0x00720E30  (unnamed)
  0x00720FC0  (unnamed)
  0x00409ED0  (unnamed)
  0x00721EF0  (unnamed)
  0x00715740  CExoResMan::FileCopy
  0x007405F0  CClientExoApp::SetLoadFromIFO
struct offsets touched (top 3):
  +0x4 [dec 4]  x23  via EAX,ECX,EDX
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
  +0x48 [dec 72]  x4  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 008529e0 FUN_008529e0 size=2923 ret=4 callers=1 callees=45

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_008529e0(int param_1,undefined4 param_2)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined1 *puVar4;
  undefined4 *local_2c4;
  undefined4 local_2a0;
  undefined1 local_16c [8];
  undefined4 *local_164;
  undefined4 *local_160;
  undefined1 local_15c [16];
  int local_14c;
  undefined4 *local_148;
  undefined1 local_144 [8];
  undefined1 local_13c [8];
  undefined1 local_134 [8];
  undefined1 local_12c [8];
  undefined1 local_124 [8];
  undefined1 local_11c [8];
  undefined1 local_114 [8];
  undefined1 local_10c [8];
  undefined1 local_104 [8];
  undefined1 local_fc [8];
  undefined1 local_f4 [8];
  int local_ec;
  undefined4 local_e8;
  undefined1 local_e4 [11];
  char local_d9;
  undefined1 local_d8 [11];
  char local_cd;
  undefined1 local_cc [8];
  undefined1 local_c4 [4];
  undefined1 local_c0 [8];
  undefined1 local_b8 [8];
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined4 local_78;
  undefined1 local_74;
  undefined1 local_73;
  undefined1 local_72;
  undefined2 local_70;
  undefined2 local_6e;
  undefined2 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined1 local_60 [40];
  undefined4 *local_38;
  undefined4 local_34;
  undefined1 local_30 [8];
  undefined4 local_28;
  undefined1 local_1c [4];
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097245f;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(uVar1);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  FUN_00733540();
  local_8._0_1_ = 2;
  FUN_00733540();
  local_8._0_1_ = 3;
  FUN_0051c500();
/* ... 182 more lines truncated by --max-lines */
```

### decoy: 0x00853550 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00853550: 16-byte aligned: YES
  00853550  55             PUSH EBP
  00853551  8B EC          MOV EBP,ESP
  00853553  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00853550
string literals (18):
  0x00994330  at 00853589  'LOADMUSIC'
  0x00994320  at 008535e0  'STARTWAYPOINT'
  0x00994318  at 0085363a  'MOVIE1'
  0x00994310  at 00853694  'MOVIE2'
  0x00994308  at 008536ee  'MOVIE3'
  0x00994300  at 0085374e  'MOVIE4'
  0x009942F8  at 008537ae  'MOVIE5'
  0x009942F0  at 0085380e  'MOVIE6'
  0x009942E4  at 00853866  'TIME_YEAR'
  0x009942D8  at 00853880  'TIME_MONTH'
  0x009942CC  at 0085389a  'TIME_DAY'
  0x009942C0  at 008538b4  'TIME_HOUR'
direct callees (8):
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00718C00  CResGFF::ReadFieldDWORD
  0x00718A00  CResGFF::ReadFieldBYTE
  0x00718B00  CResGFF::ReadFieldWORD
  0x00718960  CResGFF::GetStructFromStruct
  0x005FD040  (unnamed)
struct offsets touched (top 9):
  +0x40 [dec 64]  x1  via EDX
  +0x44 [dec 68]  x1  via ECX
  +0x45 [dec 69]  x1  via EAX
  +0x46 [dec 70]  x1  via EDX
  +0x48 [dec 72]  x1  via ECX
  +0x4A [dec 74]  x1  via EAX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via ECX
  +0x54 [dec 84]  x1  via ECX
```

## Card B0-3-37

GOG function: CExoString::Constructor_2  gog 0x00605680  callconv=__thiscall  param_size_bytes=4  gog_size=126  gog_retn=4
Candidate Steam address: 0x007338d0 (size 250, retn 4)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoString::operator=_2
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00733570:STR0/CALLEE29/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00605680 CExoString::CStrConstructor -> 0x007338d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00605700 CExoString::Constructor_3  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00921b40 Global::strlen -> Steam ?  in candidate callees: n/a
  0x00924c20 Global::strcpy -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0x14}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007338d0: 16-byte aligned: YES
  007338d0  55             PUSH EBP
  007338d1  8B EC          MOV EBP,ESP
  007338d3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007338d0
string literals (0):
direct callees (4):
  0x00919F10  _strlen
  0x009196FD  operator delete
  0x00919723  operator new
  0x0091CFF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x5  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007338d0 FUN_007338d0 size=250 ret=4 callers=224 callees=4

int * __thiscall FUN_007338d0(int *param_1,char *param_2)

{
  size_t sVar1;
  int iVar2;
  
  if ((*param_1 != 0) &&
     ((param_2 == (char *)0x0 || (sVar1 = _strlen [GHIDRA-AUTO-LABEL: unreliable](param_2), (uint)param_1[1 = byte +0x4] < sVar1 + 1)))) {
    param_1[1 = byte +0x4] = 0;
    FUN_009196fd(*param_1);
    *param_1 = 0;
  }
  if ((param_2 != (char *)0x0) && (sVar1 = _strlen [GHIDRA-AUTO-LABEL: unreliable](param_2), 0 < (int)sVar1)) {
    if (*param_1 == 0) {
      sVar1 = _strlen [GHIDRA-AUTO-LABEL: unreliable](param_2);
      param_1[1 = byte +0x4] = sVar1 + 1;
      iVar2 = FUN_00919723(param_1[1 = byte +0x4]);
      *param_1 = iVar2;
    }
    FUN_0091cff0(*param_1,param_2);
    return param_1;
  }
  if (*param_1 != 0) {
    FUN_009196fd(*param_1);
    *param_1 = 0;
    param_1[1 = byte +0x4] = 0;
  }
  return param_1;
}
```

### decoy: 0x00733570 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00733570: 16-byte aligned: YES
  00733570  55             PUSH EBP
  00733571  8B EC          MOV EBP,ESP
  00733573  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00733570
string literals (0):
direct callees (3):
  0x00919F10  _strlen
  0x00919723  operator new
  0x0091CFF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via EAX,EDX
```

## Card B0-3-38

GOG function: CSWCPlaceable::Destructor_2  gog 0x00534a80  callconv=(blank)  param_size_bytes=0  gog_size=154  gog_retn=0
Candidate Steam address: 0x00841200 (size 154, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x008411d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00534a50 CSWCPlaceable::Destructor -> 0x008411d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00534b20 CSWCPlaceable::AIUpdate -> 0x008412a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00472750 CSWCObject::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0047eb10 CSWCItem::UnloadModel -> Steam ?  in candidate callees: n/a
  0x00535d20 CSWCPlaceable::UnloadLight -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x3*,0xfc*,0x104*,0x148*} | Steam {0x2*,0x3*,0xfc*,0x104*,0x148*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00841200: 16-byte aligned: YES
  00841200  55             PUSH EBP
  00841201  8B EC          MOV EBP,ESP
  00841203  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00841200
string literals (0):
direct callees (4):
  0x008424A0  CSWCPlaceable::UnloadLight
  0x007FD890  CSWCItem::AIUpdate
  0x00733780  CExoString::~CExoString
  0x007ED5F0  CSWCObject::Destructor_2
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00841200 FUN_00841200 size=154 ret=0 callers=1 callees=4

void __fastcall FUN_00841200(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_009707e2;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWCPlaceable::vftable;
  local_8 = 3;
  FUN_008424a0(uVar1);
  FUN_007fd890();
  local_8._0_1_ = 2;
  FUN_00733780();
  local_8._0_1_ = 1;
  FUN_00733780();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_007ed5f0();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008411d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008411d0: 16-byte aligned: YES
  008411d0  55             PUSH EBP
  008411d1  8B EC          MOV EBP,ESP
  008411d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008411d0
string literals (0):
direct callees (2):
  0x00841200  CSWCPlaceable::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-3-39

GOG function: CSWGuiUpgrade::Destructor  gog 0x005bb0e0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x008cad20 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswguiupgrade_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008cad50:STR0/CALLEE0/SIG0.007/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005ba1d0 CSWGuiUpgrade::Constructor -> 0x008c9e10  gate_class=AUTO
- GOG-order neighbour after:  0x005bb5f0 CSWGuiUpgrade::GetCurrentUpgradeCategory -> 0x008cb230  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005bb110 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x008cad20: 16-byte aligned: YES
  008cad20  55             PUSH EBP
  008cad21  8B EC          MOV EBP,ESP
  008cad23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008cad20
string literals (0):
direct callees (2):
  0x008CAD50  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008cad20 FUN_008cad20 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_008cad20(undefined4 param_1,uint param_2)

{
  FUN_008cad50();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x008cad50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008cad50: 16-byte aligned: YES
  008cad50  55             PUSH EBP
  008cad51  8B EC          MOV EBP,ESP
  008cad53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008cad50
string literals (0):
direct callees (10):
  0x0041D290  (unnamed)
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0041A3D0  CSWGuiButton::Destructor
  0x00418130  (unnamed)
  0x00418640  CSWGuiControl::Destructor
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x0041D190  (unnamed)
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 2):
  +0x3DB4 [dec 15796]  x3  via EAX,ECX,EDX
  +0x3DB8 [dec 15800]  x3  via EAX,ECX,EDX
```

## Card B0-3-40

GOG function: CSWSArea::GetClientArea_2  gog 0x00722ee0  callconv=(blank)  param_size_bytes=4  gog_size=51  gog_retn=4
Candidate Steam address: 0x0052dd00 (size 255, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSArea::AwardStealthXP
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.27 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0052de00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00722de0 CSWSArea::AwardStealthXP -> 0x0052dd00  gate_class=AUTO
- GOG-order neighbour after:  0x00722f60 CSWSArea::EnforceNoPCHeadCloning -> 0x0052de80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2e8,0x2ec*} | Steam {0x4,0x8,0x2ec*,0x2f4}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x3,0x4*,0x2c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0052dd00: 16-byte aligned: YES
  0052dd00  55             PUSH EBP
  0052dd01  8B EC          MOV EBP,ESP
  0052dd03  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052dd00
string literals (0):
direct callees (6):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FE0C0  CSWPartyTable::DistributeExperience
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
  0x0052DE40  (unnamed)
struct offsets touched (top 4):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x2EC [dec 748]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x2F4 [dec 756]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0052dd00 FUN_0052dd00 size=255 ret=4 callers=1 callees=6

void FUN_0052dd00(undefined4 param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int extraout_var;
  
  iVar1 = FUN_0051c100(param_1);
  if (((iVar1 != 0) && (*(int *)(extraout_var + 0x2f4) != 0)) &&
     (*(int *)(extraout_var + 0x2ec) != 0)) {
    iVar1 = extraout_var;
    iVar2 = FUN_0051c8d0();
    if (iVar2 != 0) {
      uVar3 = *(undefined4 *)(iVar1 + 0x2ec);
      uVar5 = 1;
      FUN_0051c8d0(uVar3,1);
      FUN_005fe0c0(uVar3,uVar5);
    }
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    iVar2 = FUN_0073f750();
    if (iVar2 != 0) {
      uVar5 = *(undefined4 *)(iVar1 + 0x2ec);
      uVar4 = 3;
      FUN_0073f750(3,uVar5,iVar1,*(undefined4 *)(DAT_00a1b4a4 + 4),uVar5,uVar3);
      FUN_007ceef0(uVar4,uVar5);
    }
    FUN_0052de40(0);
    *(undefined4 *)(iVar1 + 0x2f4) = 0;
  }
  return;
}
```

### decoy: 0x0052de00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052de00: 16-byte aligned: YES
  0052de00  55             PUSH EBP
  0052de01  8B EC          MOV EBP,ESP
  0052de03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052de00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2EC [dec 748]  x2  via EAX,EDX
  +0x2E8 [dec 744]  x1  via ECX
```

## Card B0-3-41

GOG function: CExoResMan::UpdateResourceDirectory  gog 0x0061b090  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x00711730 (size 27, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00711710:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061b070 CExoResMan::RemoveResourceDirectory -> 0x00711710  gate_class=REVIEW
- GOG-order neighbour after:  0x0061b0b0 CExoResMan::AddKeyTable -> 0x00711750  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061de00 CExoResMan::UpdateKeyTable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x2*,0x4*} | Steam {0x2*,0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00711730: 16-byte aligned: YES
  00711730  55             PUSH EBP
  00711731  8B EC          MOV EBP,ESP
  00711733  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711730
string literals (0):
direct callees (1):
  0x007144A0  CExoResMan::UpdateKeyTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00711730 FUN_00711730 size=27 ret=4 callers=1 callees=1
THUNK -> 0x007144A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00711730(undefined4 param_1)

{
  FUN_007144a0(param_1,2);
  return;
}
```

### decoy: 0x00711710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711710: 16-byte aligned: YES
  00711710  55             PUSH EBP
  00711711  8B EC          MOV EBP,ESP
  00711713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711710
string literals (0):
direct callees (1):
  0x007131D0  CExoResMan::RemoveKeyTable
struct offsets touched (top 0):
```

## Card B0-3-42

GOG function: CExoResMan::AddFixedKeyTableFile  gog 0x0061afd0  callconv=(blank)  param_size_bytes=4  gog_size=29  gog_retn=4
Candidate Steam address: 0x00711670 (size 29, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00711690:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061af60 CExoResMan::AddResourceImageFilesAsync -> 0x00711600  gate_class=AUTO
- GOG-order neighbour after:  0x0061aff0 CExoResMan::AddResourceDirectory -> 0x00711690  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061b0b0 CExoResMan::AddKeyTable -> Steam ?  in candidate callees: n/a
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
prologue for 0x00711670: 16-byte aligned: YES
  00711670  55             PUSH EBP
  00711671  8B EC          MOV EBP,ESP
  00711673  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711670
string literals (0):
direct callees (1):
  0x00711750  CExoResMan::AddKeyTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00711670 FUN_00711670 size=29 ret=4 callers=3 callees=1
THUNK -> 0x00711750 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00711670(undefined4 param_1)

{
  FUN_00711750(param_1,1,0);
  return;
}
```

### decoy: 0x00711690 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711690: 16-byte aligned: YES
  00711690  55             PUSH EBP
  00711691  8B EC          MOV EBP,ESP
  00711693  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711690
string literals (0):
direct callees (1):
  0x00711750  CExoResMan::AddKeyTable
struct offsets touched (top 0):
```

## Card B0-3-43

GOG function: CGuiInGame::RemoveGUIPopUps  gog 0x004e0ca0  callconv=(blank)  param_size_bytes=0  gog_size=120  gog_retn=0
Candidate Steam address: 0x007d05a0 (size 120, retn 0)
Candidate sources: mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::GetMiniMapVisible
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007d0560:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0c60 CGuiInGame::GetMiniMapVisible -> 0x007d0560  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0d20 CGuiInGame::ResetInterfaceFont  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam ?  in candidate callees: n/a
  0x0040d580 CClientExoApp::GetMiniGame -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x98*,0x59b4*} | Steam {0x4*,0x1c*,0x98*,0x59b4*}
imm sets (* = in both, 2 common): GOG {0x3*,0x10*} | Steam {0x3*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007d05a0: 16-byte aligned: YES
  007d05a0  55             PUSH EBP
  007d05a1  8B EC          MOV EBP,ESP
  007d05a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d05a0
string literals (0):
direct callees (2):
  0x0072FB00  OptionsHolder::GetGameOptions (returns *(*(this+4)+4); +8 bit0 = Auto Level Up NPCs)
  0x007400A0  CClientExoApp::GetMiniGame
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x59B4 [dec 22964]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007d05a0 FUN_007d05a0 size=120 ret=0 callers=1 callees=2

uint FUN_007d05a0(void)

{
  int iVar1;
  uint uVar2;
  int extraout_var;
  int iVar3;
  undefined4 uVar4;
  
  uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  iVar1 = FUN_0072fb00();
  if ((*(uint *)(iVar1 + 0x1c) >> 3 & 1) == 0) {
    uVar2 = 0;
  }
  else {
    iVar3 = extraout_var;
    iVar1 = FUN_007400a0(extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 4),iVar1,uVar4);
    if (iVar1 == 0) {
      if (*(int *)(iVar3 + 0x98) == 0) {
        uVar2 = 1;
      }
      else {
        uVar2 = *(uint *)(*(int *)(iVar3 + 0x98) + 0x59b4) >> 1 & 1;
      }
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x007d0560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d0560: 16-byte aligned: YES
  007d0560  55             PUSH EBP
  007d0561  8B EC          MOV EBP,ESP
  007d0563  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d0560
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x3C [dec 60]  x3  via EAX,EDX
  +0x40 [dec 64]  x1  via ECX
  +0x84 [dec 132]  x1  via EDX
```

## Card B0-3-44

GOG function: CSWGuiSaveLoad::Constructor  gog 0x005cfd00  callconv=(blank)  param_size_bytes=16  gog_size=2656  gog_retn=16
Candidate Steam address: 0x00850770 (size 3051, retn 16)
Candidate sources: branch+name_map:high:steam-db-pr3-savegame+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 14 | A-CALLEE 1 | A-SIG 0.57 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'BTN_FILTER'@009a3f04; 'BTN_DELETE'@009a3f10; 'BTN_SAVELOAD'@009a3f1c; CALLEE 1 (SetActiveControl 0x40ec00); SIG 0.57
- best decoy (neighbour/other candidate) scores: 0x00451840:STR0/CALLEE0/SIG0.021/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005cfbc0 CSWGuiSwoopUpgrade::CreateItemEntry  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d0760 CSWGuiSaveLoad::`scalar  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (22), mapped to Steam through trusted rows only:
  0x0044dd60 CExoArrayList_CExoString_::CExoArrayList_2 -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00512a40 CSWGuiButton::Constructor -> Steam ?  in candidate callees: n/a
  0x005324a0 Global::AspyrAddButtonIcon -> Steam ?  in candidate callees: n/a
  0x005340d0 CExoArrayList_CSWGuiControl__::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x005d0d50 CSWGuiSaveLoad::PopulateGameList -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  ... 14 more GOG callees
GOG strings (20): 'TEMP:'; 'LBL_BAR4'; 'LBL_BAR3'; 'LBL_BAR2' ...
disp sets (* = in both, 7 common): GOG {0x48*,0x58*,0x68*,0x6c*,0xa4*,0x970*,0xc4c*,0x1b5c,0x1b6c,0x1b70} | Steam {0x48*,0x58*,0x5c,0x68*,0x6c*,0xa4*,0x1d4,0x970*,0xc4c*,0xef0,0x1e2c,0x1e3c,0x1e40}
imm sets (* = in both, 61 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+57)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+63)}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x00850770: 16-byte aligned: YES
  00850770  55             PUSH EBP
  00850771  8B EC          MOV EBP,ESP
  00850773  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00850770
string literals (20):
  0x009A3FAC  at 0085095e  'saveload_p'
  0x009A3F9C  at 0085097e  'LBL_PANELNAME'
  0x009A3F8C  at 008509b8  'LBL_SCREENSHOT'
  0x009A3F7C  at 008509f5  'LBL_PLANETNAME'
  0x009A3F6C  at 00850a31  'LBL_AREANAME'
  0x009A3F64  at 00850a6e  'LBL_PM1'
  0x009A3F5C  at 00850aab  'LBL_PM2'
  0x009A3F54  at 00850ae7  'LBL_PM3'
  0x009A3F48  at 00850b24  'LB_GAMES'
  0x009A3F3C  at 00850b61  'LBL_PCNAME'
  0x009A3F2C  at 00850b9d  'LBL_TIMEPLAYED'
  0x009A3F1C  at 00850bda  'BTN_SAVELOAD'
direct callees (26):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x00733540  CExoString::CExoString(empty)
  0x00419740  (unnamed)
  0x0091D67E  (unnamed)
  0x0041CE90  (unnamed)
  0x0041A330  (unnamed)
  0x0041A510  (unnamed)
  0x0084A640  (unnamed)
  0x0040DEF0  (unnamed)
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x00753C40  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x00418AF0  CSWGuiControl::AddEvent
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x007338D0  CExoString::operator=_2
  0x00760EA0  (unnamed)
struct offsets touched (top 10):
  +0x48 [dec 72]  x10  via EAX,ECX,EDX
  +0x68 [dec 104]  x7  via EAX,ECX,EDX
  +0x1D4 [dec 468]  x2  via ECX,EDX
  +0x58 [dec 88]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
  +0x6C [dec 108]  x1  via EDX
  +0xA4 [dec 164]  x1  via EAX
  +0x970 [dec 2416]  x1  via EDX
  +0xC4C [dec 3148]  x1  via EAX
  +0xEF0 [dec 3824]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00850770 FUN_00850770 size=3051 ret=16 callers=5 callees=27

undefined4 * __thiscall
FUN_00850770(undefined4 *param_1,undefined4 param_2,uint param_3,uint param_4,undefined4 param_5)

{
  uint uVar1;
  int iVar2;
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
  undefined1 local_28 [16];
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972086;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiSaveLoad::vftable;
  FUN_00733540(uVar1);
  local_8._0_1_ = 1;
  FUN_00419740();
  local_8._0_1_ = 2;
  FUN_00419740();
  local_8._0_1_ = 3;
  FUN_00419740();
  local_8._0_1_ = 4;
  FUN_00419740();
  local_8._0_1_ = 5;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
            (param_1 + 0x166,0x148,3,FUN_00419740,
             CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton);
  local_8._0_1_ = 6;
  FUN_0041ce90();
  local_8._0_1_ = 7;
  FUN_00419740();
  local_8._0_1_ = 8;
  FUN_00419740();
  local_8._0_1_ = 9;
  FUN_0041a330();
  param_1[0x3bc = byte +0xef0] = CSWGuiOptionsCheckbox::vftable;
  local_8._0_1_ = 10;
  FUN_0041a510();
  local_8._0_1_ = 0xb;
  FUN_0041a510();
  local_8._0_1_ = 0xc;
  FUN_0041a510();
  local_8._0_1_ = 0xd;
  FUN_0041a510();
  local_8._0_1_ = 0xe;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
            (param_1 + 0x640,0x148,4,FUN_00419740,
             CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton);
  local_8._0_1_ = 0xf;
  FUN_0084a640(0);
  local_8._0_1_ = 0x10;
/* ... 162 more lines truncated by --max-lines */
```

### decoy: 0x00451840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00451840: 16-byte aligned: YES
  00451840  55             PUSH EBP
  00451841  8B EC          MOV EBP,ESP
  00451843  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00451840
string literals (0):
direct callees (1):
  0x00451BD0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x5  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0xC [dec 12]  x1  via ECX
```

## Card B0-3-45

GOG function: CSWGuiMainMenu::OnNewGamePicked  gog 0x0053ccd0  callconv=__thiscall  param_size_bytes=4  gog_size=702  gog_retn=4
Candidate Steam address: 0x00882230 (size 702, retn 4)
Candidate sources: branch+mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiMainMenu
- anchors: A-STR 2 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'HD0:effects'@009a5aa4; '001ebo'@009a5ab0; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008824f0:STR1/CALLEE0/SIG0.171/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0053cb40 CSWGuiMainMenu::Draw  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0053cf90 CSWGuiMainMenu::OnLoadGamePicked -> 0x008824f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (22), mapped to Steam through trusted rows only:
  0x00401b80 CAppManager::CreateServer -> Steam ?  in candidate callees: n/a
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x0040e820 CClientExoApp::ClearMainMenuPointer -> Steam ?  in candidate callees: n/a
  0x004e28e0 CGuiInGame::ResetTutorialWindowTracker -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x005dd670 CSWGuiClassSelection::Constructor -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  ... 14 more GOG callees
GOG strings (3): 'MODULES:'; 'HD0:effects'; '001ebo'
disp sets (* = in both, 5 common): GOG {0x4*,0x1c*,0x48*,0x50*,0x1c98*} | Steam {0x4*,0x1c*,0x48*,0x50*,0x1c98*}
imm sets (* = in both, 15 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x78*,0x200*,0x7db*,0xbba*,0x15f0*,0x270d*,0xfffffcff*,...(+1)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x78*,0x200*,0x7db*,0xbba*,0x15f0*,0x270d*,0xfffffcff*,...(+1)}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00882230: 16-byte aligned: YES
  00882230  55             PUSH EBP
  00882231  8B EC          MOV EBP,ESP
  00882233  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00882230
string literals (5):
  0x009A5AB0  at 008822ba  '001ebo'
  0x009A5AA4  at 008822cb  'HD0:effects'
  0x00993E50  at 00882308  'MODULES:'
  0x009A5AB0  at 00882385  '001ebo'
  0x00993E50  at 00882392  'MODULES:'
direct callees (22):
  0x005FF000  CSWPartyTable::ResetCurrentSessionStartTime
  0x00401BC0  Recreate CServerExoApp (new game / load screen)
  0x00736240  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x00733570  CExoString::CExoString_char
  0x00716DA0  CExoResMan::CacheResource
  0x00733780  CExoString::~CExoString
  0x00711690  CExoResMan::AddResourceDirectory
  0x005564B0  (unnamed)
  0x00710810  (unnamed)
  0x00711ED0  CExoResMan::Exists
  0x007338D0  CExoString::operator=_2
  0x00711710  CExoResMan::RemoveResourceDirectory
  0x00556590  (unnamed)
  0x00919723  operator new
  0x008F92B0  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x0070BC60  (unnamed)
  0x00741360  CClientExoApp::ClearMainMenuPointer
  0x007362C0  (unnamed)
struct offsets touched (top 5):
  +0x48 [dec 72]  x5  via EAX,ECX
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x50 [dec 80]  x1  via EAX
  +0x1C98 [dec 7320]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00882230 FUN_00882230 size=702 ret=4 callers=0 callees=22

void __thiscall FUN_00882230(int param_1,int param_2)

{
  int iVar1;
  undefined4 local_88;
  undefined1 local_68 [8];
  undefined1 local_60 [16];
  undefined1 local_50 [16];
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [12];
  undefined4 local_24;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00974a8b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (((*(uint *)(param_1 + 0x48) >> 8 & 3) != 2) && (*(int *)(param_1 + 0x1c98) != 0)) {
    if (*(int *)(param_2 + 0x50) != 0) {
      FUN_005ff000(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
      FUN_00401bc0(0);
      FUN_00736240();
      local_8 = 0;
      FUN_00733540();
      local_8._0_1_ = 1;
      FUN_00733570("001ebo");
      local_8._0_1_ = 2;
      FUN_00733570("HD0:effects");
      local_8._0_1_ = 3;
      FUN_00716da0(local_38,local_30,0x270d);
      FUN_00733780();
      local_8._0_1_ = 2;
      FUN_00733780();
      FUN_00733570("MODULES:");
      local_8._0_1_ = 4;
      FUN_00711690(local_40);
      local_8 = CONCAT31(local_8._1_3_,2);
      FUN_00733780();
      FUN_005564b0();
      FUN_00710810(local_18);
      iVar1 = FUN_00711ed0(local_50,0x7db,0);
      if (iVar1 == 0) {
        FUN_00710810(local_18);
        iVar1 = FUN_00711ed0(local_60,0xbba,0);
        if (iVar1 == 0) {
          FUN_007338d0("001ebo");
        }
      }
      FUN_00733570("MODULES:");
      local_8._0_1_ = 5;
      FUN_00711710(local_68);
      local_8._0_1_ = 2;
      FUN_00733780();
      FUN_00556590();
      iVar1 = FUN_00919723(0x15f0);
      local_8._0_1_ = 6;
      if (iVar1 == 0) {
        local_88 = 0;
      }
      else {
        local_88 = FUN_008f92b0(*(undefined4 *)(param_1 + 0x1c),local_18);
      }
      local_8._0_1_ = 2;
      local_24 = local_88;
      FUN_00410530(local_88,2,1);
      FUN_0070bc60(DAT_0099c2a8,0);
      *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xffffff7f;
      *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffcff | 0x200;
      FUN_00741360();
      local_8._0_1_ = 1;
      FUN_00733780();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_007362c0();
/* ... 10 more lines truncated by --max-lines */
```

### decoy: 0x008824f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008824f0: 16-byte aligned: YES
  008824f0  55             PUSH EBP
  008824f1  8B EC          MOV EBP,ESP
  008824f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008824f0
string literals (1):
  0x009A5AA4  at 0088253a  'HD0:effects'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x00716DA0  CExoResMan::CacheResource
  0x00733780  CExoString::~CExoString
  0x00882C00  (unnamed)
  0x0041FF70  (unnamed)
struct offsets touched (top 6):
  +0x48 [dec 72]  x23  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x6C [dec 108]  x1  via EDX
  +0x31C [dec 796]  x1  via EDX
  +0x368 [dec 872]  x1  via EAX
  +0x1C98 [dec 7320]  x1  via EDX
```

## Card B0-3-46

GOG function: CRes::Constructor  gog 0x0061aa20  callconv=(blank)  param_size_bytes=0  gog_size=151  gog_retn=0
Candidate Steam address: 0x007111a0 (size 30, retn 0)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x007110d0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007110d0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a890 CResRef::operator+=  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061aac0 CSWMGTargettingGunBank::IsTargetting  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x6,0x8,0xc,0x10,0x14,0x18,0x1c,0x20,0x24} | Steam {}
imm sets (* = in both, 0 common): GOG {0xfffffff7,0xfffffffb} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007111a0: 16-byte aligned: YES
  007111a0  55             PUSH EBP
  007111a1  8B EC          MOV EBP,ESP
  007111a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007111a0
string literals (0):
direct callees (1):
  0x00711230  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007111a0 FUN_007111a0 size=30 ret=0 callers=21 callees=1

void __fastcall FUN_007111a0(undefined4 *param_1)

{
  *param_1 = CRes::vftable;
  FUN_00711230(1);
  return;
}
```

### decoy: 0x007110d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007110d0: 16-byte aligned: YES
  007110d0  55             PUSH EBP
  007110d1  8B EC          MOV EBP,ESP
  007110d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007110d0
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

## Card B0-3-47

GOG function: CSWCAreaOfEffectObject::Destructor  gog 0x005821d0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x008ee7f0 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008ee8c0:STR0/CALLEE0/SIG0.05/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00581f60 CSWCAreaOfEffectObject::Constructor -> 0x008ee580  gate_class=AUTO
- GOG-order neighbour after:  0x00582200 CSWCAreaOfEffectObject::Destructor_2 -> 0x008ee820  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00582200 CSWCAreaOfEffectObject::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x008ee7f0: 16-byte aligned: YES
  008ee7f0  55             PUSH EBP
  008ee7f1  8B EC          MOV EBP,ESP
  008ee7f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008ee7f0
string literals (0):
direct callees (2):
  0x008EE820  CSWCAreaOfEffectObject::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008ee7f0 FUN_008ee7f0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_008ee7f0(undefined4 param_1,uint param_2)

{
  FUN_008ee820();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x008ee8c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008ee8c0: 16-byte aligned: YES
  008ee8c0  55             PUSH EBP
  008ee8c1  8B EC          MOV EBP,ESP
  008ee8c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008ee8c0
string literals (2):
  0x0099AB38  at 008eea05  'SoundImpact'
  0x0099AB44  at 008eea82  'SoundDuration'
direct callees (23):
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0051AF60  CWorldTimer::SubtractWorldTimes
  0x0073F4D0  ClientApp::GetClientObject
  0x00733540  CExoString::CExoString(empty)
  0x00733570  CExoString::CExoString_char
  0x0071B1D0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x00710810  (unnamed)
  0x0070BA90  (unnamed)
  0x00919723  operator new
  0x0070AB90  (unnamed)
  0x0070ACF0  (unnamed)
  0x0070ACC0  (unnamed)
  0x0070AE30  (unnamed)
  0x0070AE90  (unnamed)
  0x0070AD50  (unnamed)
  0x008F07C0  CSWCAreaOfEffectObject::SetAnimation
  0x008F0FA0  CSWCAreaOfEffectObject::SetMoveTimer
struct offsets touched (top 10):
  +0x154 [dec 340]  x9  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x130 [dec 304]  x6  via ECX
  +0x44 [dec 68]  x5  via EAX,EDX
  +0x104 [dec 260]  x5  via EAX,EDX
  +0x14C [dec 332]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x20 [dec 32]  x2  via EAX
  +0x3C [dec 60]  x2  via ECX
  +0x40 [dec 64]  x2  via EAX,EDX
```

## Card B0-3-48

GOG function: CExoResMan::AddEncapsulatedResourceFile  gog 0x0061af20  callconv=(blank)  param_size_bytes=8  gog_size=31  gog_retn=8
Candidate Steam address: 0x007115c0 (size 31, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00711200:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061ae30 CExoResMan::Destructor -> 0x007114d0  gate_class=AUTO
- GOG-order neighbour after:  0x0061af40 CExoResMan::AddResourceImageFile -> 0x007115e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061b0b0 CExoResMan::AddKeyTable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x3*,0x8*} | Steam {0x3*,0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007115c0: 16-byte aligned: YES
  007115c0  55             PUSH EBP
  007115c1  8B EC          MOV EBP,ESP
  007115c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007115c0
string literals (0):
direct callees (1):
  0x00711750  CExoResMan::AddKeyTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007115c0 FUN_007115c0 size=31 ret=8 callers=6 callees=1
THUNK -> 0x00711750 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007115c0(undefined4 param_1,undefined4 param_2)

{
  FUN_00711750(param_1,3,param_2);
  return;
}
```

### decoy: 0x00711200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711200: 16-byte aligned: YES
  00711200  55             PUSH EBP
  00711201  8B EC          MOV EBP,ESP
  00711203  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711200
string literals (0):
direct callees (1):
  0x00714580  CExoResMan::ReadRaw
struct offsets touched (top 0):
```

## Card B0-3-49

GOG function: CSWCPlaceable::Destructor  gog 0x00534a50  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x008411d0 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00840ef0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00534a00 CSWCPlaceable::IsHostile -> 0x00841180  gate_class=REVIEW
- GOG-order neighbour after:  0x00534a80 CSWCPlaceable::Destructor_2 -> 0x00841200  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00534a80 CSWCPlaceable::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x008411d0: 16-byte aligned: YES
  008411d0  55             PUSH EBP
  008411d1  8B EC          MOV EBP,ESP
  008411d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008411d0
string literals (0):
direct callees (2):
  0x00841200  CSWCPlaceable::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008411d0 FUN_008411d0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_008411d0(undefined4 param_1,uint param_2)

{
  FUN_00841200();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x00840ef0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00840ef0: 16-byte aligned: YES
  00840ef0  55             PUSH EBP
  00840ef1  8B EC          MOV EBP,ESP
  00840ef3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00840ef0
string literals (0):
direct callees (2):
  0x009196FD  operator delete
  0x0084A680  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-3-50

GOG function: CExoString::Constructor_4  gog 0x00605790  callconv=(blank)  param_size_bytes=8  gog_size=117  gog_retn=8
Candidate Steam address: 0x00733680 (size 117, retn 8)
Candidate sources: branch+mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoString::CStrLenConstructor
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (ReadFieldCExoLocString 0x7190e0)
- best decoy (neighbour/other candidate) scores: 0x007335f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00605790 CExoString::CStrLenConstructor -> 0x00733680  gate_class=REVIEW
- GOG-order neighbour after:  0x00605810 CExoString::Constructor_5  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924d20 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00733680: 16-byte aligned: YES
  00733680  55             PUSH EBP
  00733681  8B EC          MOV EBP,ESP
  00733683  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00733680
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0091D0F0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00733680 FUN_00733680 size=117 ret=8 callers=15 callees=2

int * __thiscall FUN_00733680(int *param_1,char *param_2,size_t param_3)

{
  int iVar1;
  
  if ((int)param_3 < 1) {
    *param_1 = 0;
    param_1[1 = byte +0x4] = 0;
  }
  else {
    param_1[1 = byte +0x4] = param_3 + 1;
    iVar1 = FUN_00919723(param_1[1 = byte +0x4]);
    *param_1 = iVar1;
    _strncpy [GHIDRA-AUTO-LABEL: unreliable]((char *)*param_1,param_2,param_3);
    *(undefined1 *)(*param_1 + param_3) = 0;
  }
  return param_1;
}
```

### decoy: 0x007335f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007335f0: 16-byte aligned: YES
  007335f0  55             PUSH EBP
  007335f1  8B EC          MOV EBP,ESP
  007335f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007335f0
string literals (0):
direct callees (3):
  0x00919F10  _strlen
  0x00919723  operator new
  0x0091CFF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via EAX,EDX
```

## Card B0-3-51

GOG function: CSWGuiMainMenu::OnLoadGamePicked  gog 0x0053cf90  callconv=(blank)  param_size_bytes=0  gog_size=679  gog_retn=0
Candidate Steam address: 0x008824f0 (size 679, retn 0)
Candidate sources: mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x008827a0
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008827a0:STR1/CALLEE0/SIG0.174/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0053ccd0 CSWGuiMainMenu::OnNewGamePicked -> 0x00882230  gate_class=REVIEW
- GOG-order neighbour after:  0x0053d240 CSWGuiMainMenu::OnLoadSaveGame  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0053d6a0 CSWGuiMainMenu::PopulateModules -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00620700 CExoResMan::CacheResource -> Steam ?  in candidate callees: n/a
  0x0091df10 CSWGuiListBox::GetControl -> Steam ?  in candidate callees: n/a
GOG strings (1): 'HD0:effects'
disp sets (* = in both, 7 common): GOG {0x8*,0x48*,0x6c*,0x31c*,0x368*,0x1034*,0x1c98*} | Steam {0x8*,0x48*,0x6c*,0x31c*,0x368*,0x1034*,0x1c98*}
imm sets (* = in both, 17 common): GOG {0x2*,0x3*,0x8*,0x48*,0x6c*,0x148*,0x378*,0x40c*,0x5dc*,0x7ac*,0x97c*,0xb4c*,0x1554*,0x1724*,...(+3)} | Steam {0x2*,0x3*,0x8*,0x48*,0x6c*,0x148*,0x378*,0x40c*,0x5dc*,0x7ac*,0x97c*,0xb4c*,0x1554*,0x1724*,...(+3)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008824f0: 16-byte aligned: YES
  008824f0  55             PUSH EBP
  008824f1  8B EC          MOV EBP,ESP
  008824f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008824f0
string literals (1):
  0x009A5AA4  at 0088253a  'HD0:effects'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x00716DA0  CExoResMan::CacheResource
  0x00733780  CExoString::~CExoString
  0x00882C00  (unnamed)
  0x0041FF70  (unnamed)
struct offsets touched (top 6):
  +0x48 [dec 72]  x23  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x6C [dec 108]  x1  via EDX
  +0x31C [dec 796]  x1  via EDX
  +0x368 [dec 872]  x1  via EAX
  +0x1C98 [dec 7320]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008824f0 FUN_008824f0 size=679 ret=0 callers=1 callees=5

void __fastcall FUN_008824f0(int *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00974ab8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if ((((uint)param_1[0x12 = byte +0x48] >> 8 & 3) != 2) && (param_1[0x726 = byte +0x1c98] != 0)) {
    FUN_00733570("HD0:effects");
    local_8 = 0;
    FUN_00716da0(local_24,local_1c,0x270d);
    FUN_00733780(uVar1);
    local_8 = 0xffffffff;
    FUN_00733780();
    if (param_1[199 = byte +0x31c] == 0) {
      FUN_00882c00();
    }
    param_1[0x2d = byte +0xb4] = param_1[0x2d = byte +0xb4] & 0xfffffffdU | 2;
    param_1[0xf0 = byte +0x3c0] = param_1[0xf0 = byte +0x3c0] & 0xfffffffd;
    param_1[0x189 = byte +0x624] = param_1[0x189 = byte +0x624] & 0xfffffffd;
    param_1[0x1fd = byte +0x7f4] = param_1[0x1fd = byte +0x7f4] & 0xfffffffd;
    param_1[0x271 = byte +0x9c4] = param_1[0x271 = byte +0x9c4] & 0xfffffffd;
    param_1[0x115 = byte +0x454] = param_1[0x115 = byte +0x454] & 0xfffffffd;
    param_1[0x2e5 = byte +0xb94] = param_1[0x2e5 = byte +0xb94] & 0xfffffffd;
    param_1[0x567 = byte +0x159c] = param_1[0x567 = byte +0x159c] & 0xfffffffd;
    param_1[0x5db = byte +0x176c] = param_1[0x5db = byte +0x176c] & 0xfffffffd;
    param_1[0x64f = byte +0x193c] = param_1[0x64f = byte +0x193c] & 0xfffffffd;
    for (local_14 = 0; local_14 < 2; local_14 = local_14 + 1) {
      param_1[local_14 * 0x52 + 0x41f] = param_1[local_14 * 0x52 + 0x41f] & 0xfffffffd;
    }
    (**(code **)(*param_1 + 0x8 [dec 8] {int*: byte +0x20}))(param_1 + 0x1b,0);
    uVar3 = 0;
    uVar2 = FUN_0041ff70(0);
    (**(code **)(param_1[0x1b = byte +0x6c] + 8))(uVar2,uVar3);
    param_1[0xda = byte +0x368] = 2;
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008827a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008827a0: 16-byte aligned: YES
  008827a0  55             PUSH EBP
  008827a1  8B EC          MOV EBP,ESP
  008827a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008827a0
string literals (1):
  0x009A5AA4  at 008827ea  'HD0:effects'
direct callees (7):
  0x00733570  CExoString::CExoString_char
  0x00716DA0  CExoResMan::CacheResource
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x00850770  CSWGuiSaveLoad::Constructor
  0x00410530  CSWGuiManager::AddPanel
  0x00741360  CClientExoApp::ClearMainMenuPointer
struct offsets touched (top 6):
  +0x48 [dec 72]  x6  via EAX,ECX,EDX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x50 [dec 80]  x1  via EDX
  +0x1C98 [dec 7320]  x1  via EDX
  +0x1CA4 [dec 7332]  x1  via ECX
```

## Card B0-3-52

GOG function: CExoString::Constructor  gog 0x004eaed0  callconv=__thiscall  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x00733540 (size 33, retn 0)
Candidate sources: branch    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoString::CExoString,CExoString::DefaultConstructor
- anchors: A-STR 0 | A-CALLEE 19 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 19 (CreateUsableItemEntry 0x77bcf0,MainLoop 0x781be0,GetName 0x7fdfa0)
- best decoy (neighbour/other candidate) scores: 0x00733570:STR0/CALLEE9/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004eaed0 CExoString::DefaultConstructor -> 0x00733540  gate_class=REVIEW
- GOG-order neighbour after:  0x004eaf00 CSWBaseItemArray::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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
prologue for 0x00733540: 16-byte aligned: YES
  00733540  55             PUSH EBP
  00733541  8B EC          MOV EBP,ESP
  00733543  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00733540
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00733540 FUN_00733540 size=33 ret=0 callers=854 callees=0

undefined4 * __fastcall FUN_00733540(undefined4 *param_1)

{
  *param_1 = 0;
  param_1[1 = byte +0x4] = 0;
  return param_1;
}
```

### decoy: 0x00733570 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00733570: 16-byte aligned: YES
  00733570  55             PUSH EBP
  00733571  8B EC          MOV EBP,ESP
  00733573  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00733570
string literals (0):
direct callees (3):
  0x00919F10  _strlen
  0x00919723  operator new
  0x0091CFF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via EAX,EDX
```

## Card B0-3-53

GOG function: CSWGuiSaveLoad::VerifyLoadGame  gog 0x005d1bf0  callconv=(blank)  param_size_bytes=4  gog_size=209  gog_retn=4
Candidate Steam address: 0x008528b0 (size 209, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-savegame+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.88 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (LoadGame 0x853a00); SIG 0.88
- best decoy (neighbour/other candidate) scores: 0x00852340:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d16c0 CSWGuiSaveLoad::PopulateGameLB  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d1cd0 Global::LoadScreenUpdate  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x0052d090 CSWGuiMessageBox::SetAllowCancel -> Steam ?  in candidate callees: n/a
  0x0052e030 CSWGuiMessageBox::SetCallback -> Steam ?  in candidate callees: n/a
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callees: yes
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x0091df50 CSWGuiListBox::GetSelectedControl -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x1c*,0x50*,0x68*,0x70*,0xa0*,0x1b70} | Steam {0x4*,0x1c*,0x50*,0x68*,0x70*,0xa0*,0x1e40}
imm sets (* = in both, 4 common): GOG {0x4*,0x18*,0x970*,0x7d9b*} | Steam {0x4*,0x18*,0x970*,0x7d9b*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008528b0: 16-byte aligned: YES
  008528b0  55             PUSH EBP
  008528b1  8B EC          MOV EBP,ESP
  008528b3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008528b0
string literals (0):
direct callees (6):
  0x0041FFB0  (unnamed)
  0x00853A00  CSWGuiSaveLoad::LoadGame
  0x0073F750  ClientApp::GetGuiInGame
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 7):
  +0x4 [dec 4]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
  +0x68 [dec 104]  x1  via EDX
  +0x70 [dec 112]  x1  via EDX
  +0xA0 [dec 160]  x1  via EDX
  +0x1E40 [dec 7744]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 008528b0 FUN_008528b0 size=209 ret=4 callers=0 callees=6

void __thiscall FUN_008528b0(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  
  if ((*(int *)(param_2 + 0x50) != 0) && (iVar2 = FUN_0041ffb0(), iVar2 != 0)) {
    if (((*(uint *)(param_1 + 0x68) >> 1 & 1) == 0) && (*(int *)(param_1 + 0x1e40) == 0)) {
      iVar3 = FUN_0073f750();
      piVar1 = *(int **)(iVar3 + 0xa0);
      FUN_0075bb00(1);
      (**(code **)(*piVar1 + 0x70))(0x7d9b);
      FUN_0075ca30(param_1,FUN_00853a00,iVar2);
      FUN_00410530(piVar1,1,1);
    }
    else {
      FUN_00853a00(iVar2);
    }
  }
  return;
}
```

### decoy: 0x00852340 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00852340: 16-byte aligned: YES
  00852340  55             PUSH EBP
  00852341  8B EC          MOV EBP,ESP
  00852343  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00852340
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 0):
```

## Card B0-3-54

GOG function: CSWSEffectListHandler::OnApplyDamageImmunityIncrease  gog 0x0073da20  callconv=(blank)  param_size_bytes=8  gog_size=110  gog_retn=12
Candidate Steam address: 0x0059ac50 (size 101, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyTemporaryHitpoints
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059abb0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073d5b0 CSWSEffectListHandler::OnRemoveTemporaryHitpoints -> 0x0059acc0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073da90 CSWSEffectListHandler::OnRemoveDamageImmunityIncrease -> 0x0059b1a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006a9ed0 CSWSObject::SetDamageImmunity -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xb4} | Steam {0x30,0xe8}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*,0x10}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059ac50: 16-byte aligned: YES
  0059ac50  55             PUSH EBP
  0059ac51  8B EC          MOV EBP,ESP
  0059ac53  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059ac50
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0xE8 [dec 232]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059ac50 FUN_0059ac50 size=101 ret=12 callers=0 callees=1

undefined4 __thiscall FUN_0059ac50(undefined4 param_1,int *param_2,int param_3)

{
  short sVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar2 != 0) {
    iVar2 = (**(code **)(*param_2 + 0x30))();
    if (param_3 != 0) {
      sVar1 = *(short *)(iVar2 + 0xe8);
      iVar3 = FUN_005e4e60(0);
      *(int *)(iVar2 + 0xe8) = sVar1 + iVar3;
    }
  }
  return 0;
}
```

### decoy: 0x0059abb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059abb0: 16-byte aligned: YES
  0059abb0  55             PUSH EBP
  0059abb1  8B EC          MOV EBP,ESP
  0059abb3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059abb0
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
struct offsets touched (top 5):
  +0x30 [dec 48]  x3  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-3-55

GOG function: CRes::Release  gog 0x0061ac10  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x007112b0 (size 26, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:R1-static+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (ReleaseResource 0x717630,LoadModuleFinish 0x55a650,Load 0x93ffb0)
- best decoy (neighbour/other candidate) scores: 0x00711270:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061abd0 CRes::GetResRef -> 0x00711270  gate_class=AUTO
- GOG-order neighbour after:  0x0061ac30 CRes::Request -> 0x007112d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061ca10 CExoResMan::Release -> Steam ?  in candidate callees: n/a
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
prologue for 0x007112b0: 16-byte aligned: YES
  007112b0  55             PUSH EBP
  007112b1  8B EC          MOV EBP,ESP
  007112b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007112b0
string literals (0):
direct callees (1):
  0x007130B0  CExoResMan::Release
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007112b0 FUN_007112b0 size=26 ret=0 callers=31 callees=1
THUNK -> 0x007130B0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007112b0(undefined4 param_1)

{
  FUN_007130b0(param_1);
  return;
}
```

### decoy: 0x00711270 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711270: 16-byte aligned: YES
  00711270  55             PUSH EBP
  00711271  8B EC          MOV EBP,ESP
  00711273  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711270
string literals (0):
direct callees (1):
  0x00710CA0  (unnamed)
struct offsets touched (top 2):
  +0x14 [dec 20]  x2  via EAX
  +0x1A [dec 26]  x1  via ECX
```

## Card B0-3-56

GOG function: CSWSEffectListHandler::OnApplyDamageImmunityIncrease  gog 0x0073da20  callconv=(blank)  param_size_bytes=8  gog_size=110  gog_retn=12
Candidate Steam address: 0x0059b2e0 (size 241, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyDamageImmunityDecrease
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059b1a0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073d5b0 CSWSEffectListHandler::OnRemoveTemporaryHitpoints -> 0x0059acc0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073da90 CSWSEffectListHandler::OnRemoveDamageImmunityIncrease -> 0x0059b1a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006a9ed0 CSWSObject::SetDamageImmunity -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xb4*} | Steam {0x8,0x18,0x30,0xb4*,0xfc,0x1198}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*,0x16,0x24}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059b2e0: 16-byte aligned: YES
  0059b2e0  55             PUSH EBP
  0059b2e1  8B EC          MOV EBP,ESP
  0059b2e3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059b2e0
string literals (0):
direct callees (4):
  0x005E4E60  CGameEffect::GetInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x005431F0  (unnamed)
struct offsets touched (top 6):
  +0x30 [dec 48]  x2  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0xFC [dec 252]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b2e0 FUN_0059b2e0 size=241 ret=12 callers=0 callees=4

undefined4 FUN_0059b2e0(int *param_1,int param_2)

{
  char cVar1;
  undefined2 uVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 extraout_var;
  
  if (param_2 != 0) {
    uVar2 = FUN_005e4e60(0);
    uVar5 = extraout_var;
    iVar3 = FUN_005e4e60(1);
    if ((iVar3 < 0) || (param_1[0x3f = byte +0xfc] != 0)) {
      return 1;
    }
    iVar4 = (**(code **)(*param_1 + 0x30))(uVar5);
    if (iVar4 != 0) {
      uVar5 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
      (**(code **)(*param_1 + 0x30))();
      iVar4 = FUN_006bc570(0x16,uVar5);
      if (iVar4 != 0) {
        return 1;
      }
    }
    cVar1 = (**(code **)(*param_1 + 0xb4))(uVar2);
    FUN_005431f0(uVar2,cVar1 - iVar3 & 0xff);
  }
  return 0;
}
```

### decoy: 0x0059b1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b1a0: 16-byte aligned: YES
  0059b1a0  55             PUSH EBP
  0059b1a1  8B EC          MOV EBP,ESP
  0059b1a3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059b1a0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0xB4 [dec 180]  x1  via EDX
```

## Card B0-3-57

GOG function: CSWGuiInGameInventory::OnPanelRemoved  gog 0x0059b420  callconv=(blank)  param_size_bytes=0  gog_size=195  gog_retn=0
Candidate Steam address: 0x008a7520 (size 195, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswguiingameinventory_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008a75f0:STR0/CALLEE0/SIG0.144/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0059b3c0 CSWGuiInGameInventory::OnPanelAdded -> 0x008a74c0  gate_class=AUTO
- GOG-order neighbour after:  0x0059b4f0 CSWGuiInGameInventory::CreateItemEntry -> 0x008a75f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x005ac200 unnamed -> Steam ?  in candidate callees: n/a
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x00669770 CItemRepository::ClearNewItems -> Steam ?  in candidate callees: n/a
  0x006fd5a0 CSWPartyTable::GetPartyItems -> Steam ?  in candidate callees: n/a
  0x0091b2a0 CSWGuiListBox::ClearItems -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0x24b0*,0x24b4*} | Steam {0x8*,0x24b0*,0x24b4*}
imm sets (* = in both, 4 common): GOG {0x18*,0x588*,0x24b0*,0x24bc*} | Steam {0x18*,0x588*,0x24b0*,0x24bc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008a7520: 16-byte aligned: YES
  008a7520  55             PUSH EBP
  008a7521  8B EC          MOV EBP,ESP
  008a7523  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008a7520
string literals (0):
direct callees (5):
  0x0041D290  (unnamed)
  0x006169E0  (unnamed)
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005F7A30  CSWPartyTable::GetPartyItems
  0x00605F40  CItemRepository::ClearNewItems
struct offsets touched (top 3):
  +0x24B0 [dec 9392]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x24B4 [dec 9396]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 008a7520 FUN_008a7520 size=195 ret=0 callers=0 callees=5

void __fastcall FUN_008a7520(int param_1)

{
  undefined4 *puVar1;
  int local_8;
  
  FUN_0041d290();
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0x24b4); local_8 = local_8 + 1) {
    puVar1 = *(undefined4 **)(*(int *)(param_1 + 0x24b0) + local_8 * 4);
    if (puVar1 != (undefined4 *)0x0) {
      (**(code **)*puVar1)(1);
    }
    *(undefined4 *)(*(int *)(param_1 + 0x24b0) + local_8 * 4) = 0;
  }
  FUN_006169e0(1);
  param_1 = param_1 + 0x24bc;
  FUN_0051c8d0(param_1);
  FUN_005f7a30();
  FUN_00605f40(param_1);
  return;
}
```

### decoy: 0x008a75f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008a75f0: 16-byte aligned: YES
  008a75f0  55             PUSH EBP
  008a75f1  8B EC          MOV EBP,ESP
  008a75f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008a75f0
string literals (0):
direct callees (11):
  0x00919723  operator new
  0x008B0A60  CSWGuiInGameItemEntry::Constructor
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00418AF0  CSWGuiControl::AddEvent
  0x0077B670  GuiInGame::IsItemUsableByCurrent
  0x006D6E30  (unnamed)
  0x00848CF0  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x008B1250  CSWGuiInGameItemEntry::SetItem
struct offsets touched (top 10):
  +0x310 [dec 784]  x11  via EAX,ECX,EDX
  +0x5C [dec 92]  x7  via EAX,ECX,EDX
  +0x4C [dec 76]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x24 [dec 36]  x3  via ECX,EDX
  +0x14 [dec 20]  x1  via EDX
  +0x40 [dec 64]  x1  via EDX
  +0x58 [dec 88]  x1  via EDX
  +0xA0 [dec 160]  x1  via EDX
  +0xAC [dec 172]  x1  via ECX
```

## Card B0-3-58

GOG function: CSWCPlaceable::IsHostile  gog 0x00534a00  callconv=(blank)  param_size_bytes=0  gog_size=66  gog_retn=0
Candidate Steam address: 0x00841180 (size 66, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch5_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00841140:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005349c0 CSWCPlaceable::GetName -> 0x00841140  gate_class=REVIEW
- GOG-order neighbour after:  0x00534a50 CSWCPlaceable::Destructor -> 0x008411d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00536580 CSWCPlaceable::GetServerPlaceable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x398*} | Steam {0x398*}
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
prologue for 0x00841180: 16-byte aligned: YES
  00841180  55             PUSH EBP
  00841181  8B EC          MOV EBP,ESP
  00841183  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00841180
string literals (0):
direct callees (1):
  0x00842D00  CSWCPlaceable::GetServerPlaceable
struct offsets touched (top 1):
  +0x398 [dec 920]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00841180 FUN_00841180 size=66 ret=0 callers=0 callees=1

undefined4 FUN_00841180(void)

{
  int iVar1;
  undefined4 local_14;
  
  iVar1 = FUN_00842d00();
  if (iVar1 == 0) {
    local_14 = 0;
  }
  else {
    iVar1 = FUN_00842d00();
    local_14 = *(undefined4 *)(iVar1 + 0x398);
  }
  return local_14;
}
```

### decoy: 0x00841140 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00841140: 16-byte aligned: YES
  00841140  55             PUSH EBP
  00841141  8B EC          MOV EBP,ESP
  00841143  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00841140
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 0):
```

## Card B0-3-59

GOG function: CSWItem::GetBaseItem  gog 0x004ecc00  callconv=__thiscall  param_size_bytes=0  gog_size=46  gog_retn=0
Candidate Steam address: 0x006d6e30 (size 46, retn 0)
Candidate sources: branch    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (CreateUsableItemEntry 0x77bcf0,MaxAttackRange 0x571570,CanUseItem 0x5d2f80); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006d6e60:STR0/CALLEE0/SIG0.036/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ecbe0 CSWItem::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ecc30 CSWSpell::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x34*} | Steam {0xc*,0x34*}
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
prologue for 0x006d6e30: 16-byte aligned: YES
  006d6e30  55             PUSH EBP
  006d6e31  8B EC          MOV EBP,ESP
  006d6e33  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006d6e30
string literals (0):
direct callees (1):
  0x006D5200  (unnamed)
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EAX
  +0x34 [dec 52]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006d6e30 FUN_006d6e30 size=46 ret=0 callers=80 callees=1
THUNK -> 0x006D5200 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_006d6e30(int param_1)

{
  FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
  return;
}
```

### decoy: 0x006d6e60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006d6e60: 16-byte aligned: YES
  006d6e60  55             PUSH EBP
  006d6e61  8B EC          MOV EBP,ESP
  006d6e63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006d6e60
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x19 [dec 25]  x1  via EDX
  +0x1A [dec 26]  x1  via EAX
  +0x1B [dec 27]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x1D [dec 29]  x1  via EAX
  +0x1E [dec 30]  x1  via ECX
```

## Card B0-3-60

GOG function: CSWCPlaceable::BashPlaceable  gog 0x005355c0  callconv=(blank)  param_size_bytes=0  gog_size=98  gog_retn=8
Candidate Steam address: 0x00841d40 (size 98, retn 8)
Candidate sources: branch+name_map:high:batch5_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00841ce0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00535560 CSWCPlaceable::UsePlaceable -> 0x00841ce0  gate_class=AUTO
- GOG-order neighbour after:  0x00535630 CSWCPlaceable::LoadLight -> 0x00841db0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040dee0 CClientExoApp::ShowTutorialWindow -> Steam ?  in candidate callees: n/a
  0x004215b0 CSWCCreature::PerformAttack -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0x21*} | Steam {0x8*,0x14*,0x21*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x00841d40: 16-byte aligned: YES
  00841d40  55             PUSH EBP
  00841d41  8B EC          MOV EBP,ESP
  00841d43  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00841d40
string literals (0):
direct callees (2):
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x007756C0  CSWCCreature::PerformAttack
struct offsets touched (top 1):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00841d40 FUN_00841d40 size=98 ret=8 callers=1 callees=2

void __thiscall FUN_00841d40(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  
  if ((param_3 != 0) &&
     (iVar1 = FUN_00740a20(0x21,*(undefined4 *)(param_3 + 4),*(undefined4 *)(param_1 + 4),0),
     iVar1 == 0)) {
    FUN_007756c0(*(undefined4 *)(param_1 + 4));
  }
  return;
}
```

### decoy: 0x00841ce0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00841ce0: 16-byte aligned: YES
  00841ce0  55             PUSH EBP
  00841ce1  8B EC          MOV EBP,ESP
  00841ce3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00841ce0
string literals (0):
direct callees (4):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x0073F750  ClientApp::GetGuiInGame
  0x007D0B90  CGuiInGame::GetCanClick
  0x00879CC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
```
