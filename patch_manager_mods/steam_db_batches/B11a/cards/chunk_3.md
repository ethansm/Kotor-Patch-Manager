# Review cards B11a chunk 3

Answer each card with one TSV row in `steam_db_batches/B11a/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B11a-3-01

GOG function: CClientExoApp::FadeAndStopStreamingSoundSource  gog 0x0040e4d0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x00741010 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: FadeAndStopDialogAmbientTrack 0x7d0ec0)
- best decoy (neighbour/other candidate) scores: 0x00741030:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e4a0 CClientExoApp::SetCharacterChangeInProgress -> 0x00740fe0  gate_class=AUTO
- GOG-order neighbour after:  0x0040e4f0 CClientExoApp::StartCreditSequence -> 0x00741030  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043ae40 CClientExoAppInternal::FadeAndStopStreamingSoundSource -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004e15c0 CGuiInGame::FadeAndStopDialogAmbientTrack -> Steam 0x007d0ec0  in candidate callers: yes
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
prologue for 0x00741010: 16-byte aligned: YES
  00741010  55             PUSH EBP
  00741011  8B EC          MOV EBP,ESP
  00741013  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00741010
string literals (0):
direct callees (1):
  0x0078E870  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00741010 FUN_00741010 size=32 ret=8 callers=1 callees=1
THUNK -> 0x0078E870 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00741010(undefined4 param_1,undefined4 param_2)

{
  FUN_0078e870(param_1,param_2);
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

## Card B11a-3-02

GOG function: CClientExoApp::GetClientMeleeAnimation  gog 0x0040d9c0  callconv=(blank)  param_size_bytes=24  gog_size=49  gog_retn=24
Candidate Steam address: 0x00740500 (size 49, retn 24)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007404d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d990 CClientExoApp::GetClientRangedAnimation -> 0x00740930  gate_class=REJECT
- GOG-order neighbour after:  0x0040da00 CClientExoApp::SetLoadScreenByModuleName -> 0x00740540  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448fc0 CClientExoAppInternal::GetClientMeleeAnimation -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x18*} | Steam {0x18*}

### abi
```
GOG param bytes: 24; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(24/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=24 ; asm RET = 24
Stack Slot Match: TRUE (GOG 24 bytes -> x86 slots 24; ASM RET 24)
```

### prologue
```
prologue for 0x00740500: 16-byte aligned: YES
  00740500  55             PUSH EBP
  00740501  8B EC          MOV EBP,ESP
  00740503  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740500
string literals (0):
direct callees (1):
  0x0079C9C0  CClientExoAppInternal::GetClientMeleeAnimation
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740500 FUN_00740500 size=49 ret=24 callers=1 callees=1
THUNK -> 0x0079C9C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740500(undefined4 param_1,undefined4 param_2,undefined2 param_3,undefined4 param_4,
                 undefined4 param_5,undefined4 param_6)

{
  FUN_0079c9c0(param_1,param_2,param_3,param_4,param_5,param_6);
  return;
}
```

### decoy: 0x007404d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007404d0: 16-byte aligned: YES
  007404d0  55             PUSH EBP
  007404d1  8B EC          MOV EBP,ESP
  007404d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007404d0
string literals (0):
direct callees (1):
  0x0079CA90  CClientExoAppInternal::GetClientRangedAnimation
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-3-03

GOG function: CClientExoApp::SetPlayerCreature  gog 0x0040cdf0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f8d0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::AddObjectToHitcheckIgnoreList+name_held_elsewhere=0x0073f910 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f8b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cdd0 CClientExoApp::EmptyHitcheckIgnoreList -> 0x0073f8f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ce10 CClientExoApp::HandleMessage -> 0x0073f930  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444c80 CClientExoAppInternal::SetPlayerCreature -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00785d40 Global::ExecuteCommandDestroyObject -> Steam 0x00670760  in candidate callers: no
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

## Card B11a-3-04

GOG function: CClientExoAppInternal::StopMovie  gog 0x004452f0  callconv=(blank)  param_size_bytes=0  gog_size=158  gog_retn=0
Candidate Steam address: 0x0079b6e0 (size 18, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0079a4c0:STR0/CALLEE0/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00445250 CClientExoAppInternal::InitializeMoviePlayer -> 0x00798c30  gate_class=REVIEW
- GOG-order neighbour after:  0x00445390 CClientExoAppInternal::ShowObject -> 0x007983e0  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00921140 CExoMoviePlayer::Destructor -> Steam ?  in candidate callees: n/a
  0x009211a0 CExoMoviePlayer::CancelMovie -> Steam ?  in candidate callees: n/a
  0x009211c0 CExoMoviePlayer::IsMoviePlaying -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x130} | Steam {}
imm sets (* = in both, 0 common): GOG {0x4,0x5,0x10} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0079b6e0: 16-byte aligned: YES
  0079b6e0  55             PUSH EBP
  0079b6e1  8B EC          MOV EBP,ESP
  0079b6e3  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0079b6e0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0079b6e0 FUN_0079b6e0 size=18 ret=0 callers=0 callees=0

bool FUN_0079b6e0(int param_1,int param_2)

{
  return param_1 == param_2;
}
```

### decoy: 0x0079a4c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0079a4c0: 16-byte aligned: YES
  0079a4c0  55             PUSH EBP
  0079a4c1  8B EC          MOV EBP,ESP
  0079a4c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0079a4c0
string literals (0):
direct callees (17):
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0084A640  (unnamed)
  0x0057BE40  CSWSCreature::GetNearestObjects
  0x008A6110  (unnamed)
  0x00476620  (unnamed)
  0x00798160  CClientExoAppInternal::ServerToClientObjectId
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00778510  CSWCCreature::CanSee
  0x0079BF00  (unnamed)
  0x0079E420  (unnamed)
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x004142E0  (unnamed)
  0x00798D70  OnObjectSelected
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 10):
  +0x2A8 [dec 680]  x25  via EAX,ECX,EDX
  +0xC [dec 12]  x18  via EAX,EDX
  +0x2B4 [dec 692]  x13  via EAX,ECX,EDX
  +0x4 [dec 4]  x9  via ECX,EDX
  +0x378 [dec 888]  x7  via EAX,ECX,EDX
  +0x14 [dec 20]  x6  via EAX,EDX
  +0x3C [dec 60]  x6  via EAX,EDX
  +0x138 [dec 312]  x5  via EDX
  +0x368 [dec 872]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
```

## Card B11a-3-05

GOG function: CClientExoApp::ClearCachedCreatures  gog 0x0040df10  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740a50 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740980:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dee0 CClientExoApp::ShowTutorialWindow -> 0x00740a20  gate_class=REVIEW
- GOG-order neighbour after:  0x0040df30 CClientExoApp::LookUpAndPerformRumbleWithCutOff -> 0x00740a70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00439530 CClientExoAppInternal::ClearCachedCreatures -> Steam ?  in candidate callees: n/a
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

## Card B11a-3-06

GOG function: CClientExoApp::EnableMouseAtEndOfLoadModuleMovie  gog 0x0040e550  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00741090 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740f00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e530 CClientExoApp::GetCreditSequenceInProgress -> 0x00741070  gate_class=AUTO
- GOG-order neighbour after:  0x0040e570 CClientExoApp::SetLastClickedOnTarget -> 0x007410b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044d940 CClientExoAppInternal::EnableMouseAtEndOfLoadModuleMovie -> Steam ?  in candidate callees: n/a
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
prologue for 0x00741090: 16-byte aligned: YES
  00741090  55             PUSH EBP
  00741091  8B EC          MOV EBP,ESP
  00741093  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00741090
string literals (0):
direct callees (1):
  0x007A1340  CClientExoAppInternal::EnableMouseAtEndOfLoadModuleMovie
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00741090 FUN_00741090 size=22 ret=0 callers=1 callees=1
THUNK -> 0x007A1340 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00741090(undefined4 param_1)

{
  FUN_007a1340(param_1);
  return;
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

## Card B11a-3-07

GOG function: CClientExoApp::BusyWaitMenuMusic  gog 0x0040e3a0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740ee0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740f00:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e380 CClientExoApp::GetTogglePauseAtEndOfFrame -> 0x00740ec0  gate_class=AUTO
- GOG-order neighbour after:  0x0040e3c0 CClientExoApp::BusyWaitMuteSound -> 0x00740f00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044cb80 CClientExoAppInternal::BusyWaitMenuMusic -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00740ee0: 16-byte aligned: YES
  00740ee0  55             PUSH EBP
  00740ee1  8B EC          MOV EBP,ESP
  00740ee3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740ee0
string literals (0):
direct callees (1):
  0x007A0580  CClientExoAppInternal::BusyWaitMenuMusic
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740ee0 FUN_00740ee0 size=22 ret=0 callers=4 callees=1
THUNK -> 0x007A0580 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740ee0(undefined4 param_1)

{
  FUN_007a0580(param_1);
  return;
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

## Card B11a-3-08

GOG function: CClientExoApp::GetAutoPaused  gog 0x0040de20  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x00740960 (size 26, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740930:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ddf0 CClientExoApp::SetAutoPaused -> 0x00740930  gate_class=REVIEW
- GOG-order neighbour after:  0x0040de40 CClientExoApp::SetLoadScreenHint -> 0x00740980  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 7): the card's A-CALLEE credit may come from these, not from callees
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

## Card B11a-3-09

GOG function: CClientExoAppInternal::AddToPauseExclusionList_2  gog 0x004317f0  callconv=(blank)  param_size_bytes=0  gog_size=86  gog_retn=0
Candidate Steam address: 0x00785230 (size 132, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::CreateWorldJournal | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007851d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00431750 CClientExoAppInternal::DestroyQuestJournal_2 -> 0x00785130  gate_class=REVIEW
- GOG-order neighbour after:  0x00431850 CClientExoAppInternal::CreateWorldJournal -> 0x00785230  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0044de70 CClientExoAppInternal::IsMoviePlaying_2 -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x3c}
imm sets (* = in both, 0 common): GOG {0xc} | Steam {0x4,0x10,0x20}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00785230: 16-byte aligned: YES
  00785230  55             PUSH EBP
  00785231  8B EC          MOV EBP,ESP
  00785233  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00785230
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0084B7F0  (unnamed)
struct offsets touched (top 1):
  +0x3C [dec 60]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00785230 FUN_00785230 size=132 ret=0 callers=1 callees=2

void __fastcall FUN_00785230(int param_1)

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
  if (*(int *)(param_1 + 0x3c) == 0) {
    iVar1 = FUN_00919723(0x20,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_20 = 0;
    }
    else {
      local_20 = FUN_0084b7f0();
    }
    *(undefined4 *)(param_1 + 0x3c) = local_20;
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007851d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007851d0: 16-byte aligned: YES
  007851d0  55             PUSH EBP
  007851d1  8B EC          MOV EBP,ESP
  007851d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007851d0
string literals (0):
direct callees (2):
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x006D2CB0  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-3-10

GOG function: CClientExoApp::SetLoadScreenHint  gog 0x0040de40  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740980 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 5 (callers: DoQuickLoad 0x7d0f50, LoadGame 0x853a00, StartNewModule 0x52fda0)
- best decoy (neighbour/other candidate) scores: 0x00740ce0:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040de20 CClientExoApp::GetAutoPaused -> 0x00740960  gate_class=REVIEW
- GOG-order neighbour after:  0x0040de60 CClientExoApp::EnableVideoEffect -> 0x007409a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449540 CClientExoAppInternal::SetLoadScreenHint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (5 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x004e1650 CGuiInGame::DoQuickLoad -> Steam 0x007d0f50  in candidate callers: yes
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
  0x007b3120 CServerExoAppInternal::StartNewModule -> Steam 0x0052fda0  in candidate callers: yes
  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> Steam 0x00534600  in candidate callers: yes
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

### decoy: 0x00740ce0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740ce0: 16-byte aligned: YES
  00740ce0  55             PUSH EBP
  00740ce1  8B EC          MOV EBP,ESP
  00740ce3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740ce0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x488 [dec 1160]  x1  via EDX
```

## Card B11a-3-11

GOG function: CClientExoApp::EnableInput  gog 0x0040d7b0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740310 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::DismissInGameGUI | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007402f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
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

## Card B11a-3-12

GOG function: CClientExoApp::StartPazaakGame  gog 0x0040db90  callconv=(blank)  param_size_bytes=20  gog_size=44  gog_retn=20
Candidate Steam address: 0x007406d0 (size 44, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandPlayPazaak 0x68e880)
- best decoy (neighbour/other candidate) scores: 0x007406b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040db70 CClientExoApp::UpdateRoom -> 0x007406b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dbc0 CClientExoApp::EndPazaakGame -> 0x007404d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449670 CClientExoAppInternal::StartPazaakGame -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007a3ed0 Global::ExecuteCommandPlayPazaak -> Steam 0x0068e880  in candidate callers: yes
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
prologue for 0x007406d0: 16-byte aligned: YES
  007406d0  55             PUSH EBP
  007406d1  8B EC          MOV EBP,ESP
  007406d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007406d0
string literals (0):
direct callees (1):
  0x0079D070  CClientExoAppInternal::StartPazaakGame
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007406d0 FUN_007406d0 size=44 ret=20 callers=1 callees=1
THUNK -> 0x0079D070 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007406d0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5)

{
  FUN_0079d070(param_1,param_2,param_3,param_4,param_5);
  return;
}
```

### decoy: 0x007406b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007406b0: 16-byte aligned: YES
  007406b0  55             PUSH EBP
  007406b1  8B EC          MOV EBP,ESP
  007406b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007406b0
string literals (0):
direct callees (1):
  0x00783DC0  CClientExoAppInternal::UpdateRoom
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-3-13

GOG function: CClientExoApp::CancelMovie  gog 0x0040e420  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740f60 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740f40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e400 CClientExoApp::SetMouseMode -> 0x00740f40  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e440 CClientExoApp::HideMouse -> 0x00740f80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044cb00 CClientExoAppInternal::CancelMovie -> Steam ?  in candidate callees: n/a
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

## Card B11a-3-14

GOG function: CClientExoApp::DismissInGameGUI  gog 0x0040d7d0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740310 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
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

## Card B11a-3-15

GOG function: CClientExoApp::TogglePartyFollow  gog 0x0040dd00  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740840 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740820:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dce0 CClientExoApp::StopRumblePattern -> 0x00740820  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dd20 CClientExoApp::GetCombatMode -> 0x00740860  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004446c0 CClientExoAppInternal::TogglePartyFollow -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740840: 16-byte aligned: YES
  00740840  55             PUSH EBP
  00740841  8B EC          MOV EBP,ESP
  00740843  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740840
string literals (0):
direct callees (1):
  0x007980A0  CClientExoAppInternal::TogglePartyFollow
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740840 FUN_00740840 size=22 ret=0 callers=2 callees=1
THUNK -> 0x007980A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740840(undefined4 param_1)

{
  FUN_007980a0(param_1);
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

## Card B11a-3-16

GOG function: CClientExoApp::SetCombatMode  gog 0x0040dd40  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740880 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740820:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dd20 CClientExoApp::GetCombatMode -> 0x00740860  gate_class=AUTO
- GOG-order neighbour after:  0x0040dd60 CClientExoApp::PlayerFlourishWeapons -> 0x007408a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449ce0 CClientExoAppInternal::SetCombatMode -> Steam ?  in candidate callees: n/a
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

## Card B11a-3-17

GOG function: CClientExoApp::ChangeCharacter  gog 0x0040d840  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x00740380 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandDestroyObject 0x670760)
- best decoy (neighbour/other candidate) scores: 0x007401f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d810 CClientExoApp::SetPausedByCombat -> 0x00740350  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d860 CClientExoApp::RunDeathSequence -> 0x007403a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444550 CClientExoAppInternal::ChangeCharacter -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x00785d40 Global::ExecuteCommandDestroyObject -> Steam 0x00670760  in candidate callers: yes
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

## Card B11a-3-18

GOG function: CClientExoApp::ChangeCharacterToNextLivingPartyMember  gog 0x0040e040  callconv=(blank)  param_size_bytes=9  gog_size=37  gog_retn=12
Candidate Steam address: 0x00740b80 (size 37, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: HandleInputEvent 0x8a8880, OnApplyDeath 0x597d90)
- best decoy (neighbour/other candidate) scores: 0x00740930:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e020 CClientExoApp::LoadOptions -> 0x00740b60  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e070 CClientExoApp::SetAttackMashTutorialTimer -> 0x00740bb0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044bd70 CClientExoAppInternal::ChangeCharacterToNextLivingPartyMember -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 11): the card's A-CALLEE credit may come from these, not from callees
  0x0059c6d0 CSWGuiInGameInventory::HandleInputEvent -> Steam 0x008a8880  in candidate callers: yes
  0x0073a680 CSWSEffectListHandler::OnApplyDeath -> Steam 0x00597d90  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 9; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(9/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 9 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00740b80: 16-byte aligned: YES
  00740b80  55             PUSH EBP
  00740b81  8B EC          MOV EBP,ESP
  00740b83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740b80
string literals (0):
direct callees (1):
  0x0079F770  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740b80 FUN_00740b80 size=37 ret=12 callers=11 callees=1
THUNK -> 0x0079F770 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740b80(undefined4 param_1,undefined4 param_2,undefined1 param_3)

{
  FUN_0079f770(param_1,param_2,param_3);
  return;
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

## Card B11a-3-19

GOG function: CClientExoApp::LoadOptions  gog 0x0040e020  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740b60 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740b40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e000 CClientExoApp::SaveOptions -> 0x00740b40  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e040 CClientExoApp::ChangeCharacterToNextLivingPartyMember -> 0x00740b80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044bcf0 CClientExoAppInternal::LoadOptions -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740b60: 16-byte aligned: YES
  00740b60  55             PUSH EBP
  00740b61  8B EC          MOV EBP,ESP
  00740b63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740b60
string literals (0):
direct callees (1):
  0x0079F6F0  CClientExoAppInternal::LoadOptions
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740b60 FUN_00740b60 size=28 ret=4 callers=1 callees=1
THUNK -> 0x0079F6F0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740b60(undefined4 param_1)

{
  FUN_0079f6f0(param_1);
  return;
}
```

### decoy: 0x00740b40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740b40: 16-byte aligned: YES
  00740b40  55             PUSH EBP
  00740b41  8B EC          MOV EBP,ESP
  00740b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740b40
string literals (0):
direct callees (1):
  0x0079F6C0  CClientExoAppInternal::SaveOptions
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-3-20

GOG function: CClientExoApp::UpdateRoom  gog 0x0040db70  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x007406b0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740680:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040db40 CClientExoApp::SetGuiGameSlow -> 0x00740680  gate_class=AUTO
- GOG-order neighbour after:  0x0040db90 CClientExoApp::StartPazaakGame -> 0x007406d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00430340 CClientExoAppInternal::UpdateRoom -> Steam ?  in candidate callees: n/a
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
prologue for 0x007406b0: 16-byte aligned: YES
  007406b0  55             PUSH EBP
  007406b1  8B EC          MOV EBP,ESP
  007406b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007406b0
string literals (0):
direct callees (1):
  0x00783DC0  CClientExoAppInternal::UpdateRoom
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007406b0 FUN_007406b0 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00783DC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007406b0(undefined4 param_1)

{
  FUN_00783dc0(param_1);
  return;
}
```

### decoy: 0x00740680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740680: 16-byte aligned: YES
  00740680  55             PUSH EBP
  00740681  8B EC          MOV EBP,ESP
  00740683  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740680
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1B0 [dec 432]  x1  via EDX
```

## Card B11a-3-21

GOG function: CClientExoApp::GetCanSendDialog  gog 0x0040d900  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x00740440 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740350:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: autolabel:Statistics
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
prologue for 0x00740440: 16-byte aligned: YES
  00740440  55             PUSH EBP
  00740441  8B EC          MOV EBP,ESP
  00740443  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740440
string literals (0):
direct callees (1):
  0x0079C5B0  CClientExoAppInternal::GetCanSendDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740440 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=1 callees=1
THUNK -> 0x0079C5B0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_0079c5b0(param_1,param_2,param_3);
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

## Card B11a-3-22

GOG function: CClientExoApp::PlayerFlourishWeapons  gog 0x0040dd60  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007408a0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740880:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dd40 CClientExoApp::SetCombatMode -> 0x00740880  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dd80 CClientExoApp::AddFloatyText -> 0x007408c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ba30 CClientExoAppInternal::PlayerFlourishWeapons -> Steam ?  in candidate callees: n/a
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

## Card B11a-3-23

GOG function: CClientExoApp::SetLoadBarProgress  gog 0x0040da90  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x007405d0 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 7 (callers: LoadArea 0x7a2fb0, LoadPCAutoSave 0x8529e0, LoadGame 0x853a00)
- best decoy (neighbour/other candidate) scores: 0x00740540:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040da60 CClientExoApp::SetLoadMusic -> 0x007405a0  gate_class=AUTO
- GOG-order neighbour after:  0x0040dab0 CClientExoApp::SetLoadFromIFO -> 0x007405f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004495b0 CClientExoAppInternal::SetLoadBarProgress -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 13): the card's A-CALLEE credit may come from these, not from callees
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: yes
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
  0x00732120 CSWSModule::AddModuleResources -> Steam 0x0055fa90  in candidate callers: yes
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
prologue for 0x007405d0: 16-byte aligned: YES
  007405d0  55             PUSH EBP
  007405d1  8B EC          MOV EBP,ESP
  007405d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007405d0
string literals (0):
direct callees (1):
  0x0079CFB0  CClientExoAppInternal::SetLoadBarProgress
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007405d0 FUN_007405d0 size=32 ret=8 callers=13 callees=1
THUNK -> 0x0079CFB0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007405d0(undefined4 param_1,undefined4 param_2)

{
  FUN_0079cfb0(param_1,param_2);
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

## Card B11a-3-24

GOG function: CClientExoApp::AddFloatyText  gog 0x0040dd80  callconv=(blank)  param_size_bytes=12  gog_size=37  gog_retn=12
Candidate Steam address: 0x007408c0 (size 37, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: AutoLevelup 0x6bedf0, OnApplyDamage 0x5959e0, OnApplyHeal 0x597750)
- best decoy (neighbour/other candidate) scores: 0x00740a70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dd60 CClientExoApp::PlayerFlourishWeapons -> 0x007408a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ddb0 CClientExoApp::GetGUIString -> 0x007408f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044a460 CClientExoAppInternal::AddFloatyText -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x006fb270 CSWSCreatureStats::AutoLevelup -> Steam 0x006bedf0  in candidate callers: yes
  0x007382d0 CSWSEffectListHandler::OnApplyDamage -> Steam 0x005959e0  in candidate callers: yes
  0x0073a040 CSWSEffectListHandler::OnApplyHeal -> Steam 0x00597750  in candidate callers: yes
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
prologue for 0x007408c0: 16-byte aligned: YES
  007408c0  55             PUSH EBP
  007408c1  8B EC          MOV EBP,ESP
  007408c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007408c0
string literals (0):
direct callees (1):
  0x0079DE60  CClientExoAppInternal::AddFloatyText
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007408c0 FUN_007408c0 size=37 ret=12 callers=6 callees=1
THUNK -> 0x0079DE60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007408c0(undefined4 param_1,undefined1 param_2,undefined4 param_3)

{
  FUN_0079de60(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x00740a70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740a70: 16-byte aligned: YES
  00740a70  55             PUSH EBP
  00740a71  8B EC          MOV EBP,ESP
  00740a73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740a70
string literals (0):
direct callees (1):
  0x0079F010  CClientExoAppInternal::LookUpAndPerformRumbleWithCutOff
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-3-25

GOG function: CClientExoAppInternal::GetPlayerMovementInput  gog 0x0040df80  callconv=(blank)  param_size_bytes=0  gog_size=124  gog_retn=0
Candidate Steam address: 0x00761570 (size 137, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00761600:STR0/CALLEE0/SIG0.263/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040df60 CClientExoApp::GetPlayerMovementInput -> 0x00740aa0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e000 CClientExoApp::SaveOptions -> 0x00740b40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0xc,0x2a0} | Steam {}
imm sets (* = in both, 0 common): GOG {0x8,0x44} | Steam {0x3,0x6,0x3f,0xc0}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00761570: 16-byte aligned: YES
  00761570  55             PUSH EBP
  00761571  8B EC          MOV EBP,ESP
  00761573  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00761570
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00761570 FUN_00761570 size=137 ret=0 callers=1 callees=0

void FUN_00761570(byte *param_1,byte *param_2)

{
  byte bVar1;
  
  for (; *param_1 != 0; param_1 = param_1 + 1) {
    if (*param_1 < 0xc0) {
      *param_2 = *param_1;
    }
    else {
      bVar1 = *param_1;
      param_1 = param_1 + 1;
      *param_2 = bVar1 << 6 | *param_1 & 0x3f;
    }
    param_2 = param_2 + 1;
  }
  *param_2 = 0;
  return;
}
```

### decoy: 0x00761600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00761600: 16-byte aligned: YES
  00761600  55             PUSH EBP
  00761601  8B EC          MOV EBP,ESP
  00761603  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00761600
string literals (4):
  0x0099D93C  at 00761618  'rench'
  0x0099D930  at 00761636  'talian'
  0x0099D924  at 00761654  'erman'
  0x0099D918  at 00761672  'panish'
direct callees (3):
  0x00407260  (unnamed)
  0x0091D6F0  (unnamed)
  0x0091A0F8  (unnamed)
struct offsets touched (top 4):
  +0xC [dec 12]  x3  via EAX
  +0x10 [dec 16]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
```

## Card B11a-3-26

GOG function: CClientExoApp::EnableVideoEffect  gog 0x0040de60  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x007409a0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandEnableVideoEffect 0x6986e0)
- best decoy (neighbour/other candidate) scores: 0x00740980:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040de40 CClientExoApp::SetLoadScreenHint -> 0x00740980  gate_class=REVIEW
- GOG-order neighbour after:  0x0040de80 CClientExoApp::DisableVideoEffect -> 0x007409c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ab50 CClientExoAppInternal::EnableVideoEffect -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x007add30 Global::ExecuteCommandEnableVideoEffect -> Steam 0x006986e0  in candidate callers: yes
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

## Card B11a-3-27

GOG function: CClientExoApp::SetGameOver  gog 0x0040d880  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x007403c0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
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

## Card B11a-3-28

GOG function: CClientExoApp::StopRumblePattern  gog 0x0040dce0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740820 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandStopRumblePattern 0x694170)
- best decoy (neighbour/other candidate) scores: 0x00740800:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dcc0 CClientExoApp::PlayRumblePattern -> 0x00740800  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dd00 CClientExoApp::TogglePartyFollow -> 0x00740840  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449ae0 CClientExoAppInternal::StopRumblePattern -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007a97c0 Global::ExecuteCommandStopRumblePattern -> Steam 0x00694170  in candidate callers: yes
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
prologue for 0x00740820: 16-byte aligned: YES
  00740820  55             PUSH EBP
  00740821  8B EC          MOV EBP,ESP
  00740823  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740820
string literals (0):
direct callees (1):
  0x0079D4E0  CClientExoAppInternal::StopRumblePattern
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740820 FUN_00740820 size=28 ret=4 callers=1 callees=1
THUNK -> 0x0079D4E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740820(undefined4 param_1)

{
  FUN_0079d4e0(param_1);
  return;
}
```

### decoy: 0x00740800 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740800: 16-byte aligned: YES
  00740800  55             PUSH EBP
  00740801  8B EC          MOV EBP,ESP
  00740803  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740800
string literals (0):
direct callees (1):
  0x0079D3F0  CClientExoAppInternal::PlayRumblePattern
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-3-29

GOG function: CClientExoApp::SetAutoPaused  gog 0x0040ddf0  callconv=(blank)  param_size_bytes=5  gog_size=33  gog_retn=8
Candidate Steam address: 0x00740930 (size 33, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetClientRangedAnimation | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: OnApplyDeath 0x597d90, EndCombatRound 0x58e990)
- best decoy (neighbour/other candidate) scores: 0x00740b80:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ddb0 CClientExoApp::GetGUIString -> 0x007408f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040de20 CClientExoApp::GetAutoPaused -> 0x00740960  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044aa20 CClientExoAppInternal::SetAutoPaused -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 10): the card's A-CALLEE credit may come from these, not from callees
  0x0073a680 CSWSEffectListHandler::OnApplyDeath -> Steam 0x00597d90  in candidate callers: yes
  0x00773040 CSWSCombatRound::EndCombatRound -> Steam 0x0058e990  in candidate callers: yes
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
prologue for 0x00740930: 16-byte aligned: YES
  00740930  55             PUSH EBP
  00740931  8B EC          MOV EBP,ESP
  00740933  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740930
string literals (0):
direct callees (1):
  0x0079E420  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00740930 FUN_00740930 size=33 ret=8 callers=10 callees=1
THUNK -> 0x0079E420 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740930(undefined4 param_1,undefined1 param_2)

{
  FUN_0079e420(param_1,param_2);
  return;
}
```

### decoy: 0x00740b80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740b80: 16-byte aligned: YES
  00740b80  55             PUSH EBP
  00740b81  8B EC          MOV EBP,ESP
  00740b83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740b80
string literals (0):
direct callees (1):
  0x0079F770  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-3-30

GOG function: CClientExoApp::RunDeathSequence  gog 0x0040d860  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007403a0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
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

## Card B11a-3-31

GOG function: CClientExoApp::SaveOptions  gog 0x0040e000  callconv=__thiscall  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740b40 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740b60:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040df80 CClientExoAppInternal::GetPlayerMovementInput -> 0x00761570  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e020 CClientExoApp::LoadOptions -> 0x00740b60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044bcc0 CClientExoAppInternal::SaveOptions -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 10): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00740b40: 16-byte aligned: YES
  00740b40  55             PUSH EBP
  00740b41  8B EC          MOV EBP,ESP
  00740b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740b40
string literals (0):
direct callees (1):
  0x0079F6C0  CClientExoAppInternal::SaveOptions
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740b40 FUN_00740b40 size=22 ret=0 callers=11 callees=1
THUNK -> 0x0079F6C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740b40(undefined4 param_1)

{
  FUN_0079f6c0(param_1);
  return;
}
```

### decoy: 0x00740b60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740b60: 16-byte aligned: YES
  00740b60  55             PUSH EBP
  00740b61  8B EC          MOV EBP,ESP
  00740b63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740b60
string literals (0):
direct callees (1):
  0x0079F6F0  CClientExoAppInternal::LoadOptions
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-3-32

GOG function: CClientExoApp::SetCanSendDialog  gog 0x0040d8e0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740420 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: StartNewModule 0x52fda0, MainLoop 0x533830, StallEventSaveGame 0x534600)
- best decoy (neighbour/other candidate) scores: 0x007400d0:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d8c0 CClientExoApp::SetLastPartyMemberTempKilled -> 0x00740400  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d900 CClientExoApp::GetCanSendDialog -> 0x00740440  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448b50 CClientExoAppInternal::SetCanSendDialog -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x007b3120 CServerExoAppInternal::StartNewModule -> Steam 0x0052fda0  in candidate callers: yes
  0x007b6bb0 CServerExoAppInternal::MainLoop -> Steam 0x00533830  in candidate callers: yes
  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> Steam 0x00534600  in candidate callers: yes
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

## Card B11a-3-33

GOG function: CClientExoApp::PlayMovieQueue  gog 0x0040e6f0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00741230 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandPlayMovieQueue 0x69a4d0)
- best decoy (neighbour/other candidate) scores: 0x007413e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e680 CClientExoApp::AddMovieToMovieQueue -> 0x007411c0  gate_class=AUTO
- GOG-order neighbour after:  0x0040e710 CClientExoApp::MovieWindowDeactivated -> 0x00741250  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044d550 CClientExoAppInternal::PlayMovieQueue -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007afb20 Global::ExecuteCommandPlayMovieQueue -> Steam 0x0069a4d0  in candidate callers: yes
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
prologue for 0x00741230: 16-byte aligned: YES
  00741230  55             PUSH EBP
  00741231  8B EC          MOV EBP,ESP
  00741233  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00741230
string literals (0):
direct callees (1):
  0x007A0F50  CClientExoAppInternal::PlayMovieQueue
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00741230 FUN_00741230 size=28 ret=4 callers=1 callees=1
THUNK -> 0x007A0F50 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00741230(undefined4 param_1)

{
  FUN_007a0f50(param_1);
  return;
}
```

### decoy: 0x007413e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007413e0: 16-byte aligned: YES
  007413e0  55             PUSH EBP
  007413e1  8B EC          MOV EBP,ESP
  007413e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007413e0
string literals (0):
direct callees (1):
  0x0040DFE0  CExoArrayList<CExoString>::SetSize(n)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

## Card B11a-3-34

GOG function: CClientExoApp::LookUpAndPerformRumbleWithCutOff  gog 0x0040df30  callconv=(blank)  param_size_bytes=12  gog_size=37  gog_retn=12
Candidate Steam address: 0x00740a70 (size 37, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740b80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040df10 CClientExoApp::ClearCachedCreatures -> 0x00740a50  gate_class=REVIEW
- GOG-order neighbour after:  0x0040df60 CClientExoApp::GetPlayerMovementInput -> 0x00740aa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044b610 CClientExoAppInternal::LookUpAndPerformRumbleWithCutOff -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00740a70: 16-byte aligned: YES
  00740a70  55             PUSH EBP
  00740a71  8B EC          MOV EBP,ESP
  00740a73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740a70
string literals (0):
direct callees (1):
  0x0079F010  CClientExoAppInternal::LookUpAndPerformRumbleWithCutOff
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740a70 FUN_00740a70 size=37 ret=12 callers=3 callees=1
THUNK -> 0x0079F010 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740a70(undefined4 param_1,undefined1 param_2,undefined4 param_3)

{
  FUN_0079f010(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x00740b80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740b80: 16-byte aligned: YES
  00740b80  55             PUSH EBP
  00740b81  8B EC          MOV EBP,ESP
  00740b83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740b80
string literals (0):
direct callees (1):
  0x0079F770  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-3-35

GOG function: CClientExoApp::StopMenuMusic  gog 0x0040d250  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fe00 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::GetActivePauseState | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d220 CClientExoApp::StartMenuMusic -> 0x0073fd40  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d270 CClientExoApp::StopLoadMusic -> 0x0073fd70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043adb0 CClientExoAppInternal::StopMenuMusic -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00454600 CSWCArea::StartSounds -> Steam 0x007a7c00  in candidate callers: no
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

## Card B11a-3-36

GOG function: CClientExoApp::PlayRumblePattern  gog 0x0040dcc0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740800 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandPlayRumblePattern 0x68ea10)
- best decoy (neighbour/other candidate) scores: 0x00740820:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dc80 CClientExoApp::SetEndGamePending -> 0x007407c0  gate_class=AUTO
- GOG-order neighbour after:  0x0040dce0 CClientExoApp::StopRumblePattern -> 0x00740820  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004499f0 CClientExoAppInternal::PlayRumblePattern -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007a4060 Global::ExecuteCommandPlayRumblePattern -> Steam 0x0068ea10  in candidate callers: yes
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

## Card B11a-3-37

GOG function: CClientExoApp::GetLoadMusicByModuleName  gog 0x0040da20  callconv=(blank)  param_size_bytes=8  gog_size=53  gog_retn=8
Candidate Steam address: 0x00740560 (size 53, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007405d0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040da00 CClientExoApp::SetLoadScreenByModuleName -> 0x00740540  gate_class=REVIEW
- GOG-order neighbour after:  0x0040da60 CClientExoApp::SetLoadMusic -> 0x007405a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449360 CClientExoAppInternal::GetLoadMusicByModuleName -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x007be170 CServerExoAppInternal::DoPCAutosave -> Steam 0x0053af20  in candidate callers: yes
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
prologue for 0x00740560: 16-byte aligned: YES
  00740560  55             PUSH EBP
  00740561  8B EC          MOV EBP,ESP
  00740563  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740560
string literals (0):
direct callees (1):
  0x0079CD60  CClientExoAppInternal::GetLoadMusicByModuleName
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00740560 FUN_00740560 size=53 ret=8 callers=4 callees=1

undefined4 FUN_00740560(undefined4 param_1,undefined4 param_2)

{
  FUN_0079cd60(param_1,param_2);
  return param_1;
}
```

### decoy: 0x007405d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007405d0: 16-byte aligned: YES
  007405d0  55             PUSH EBP
  007405d1  8B EC          MOV EBP,ESP
  007405d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007405d0
string literals (0):
direct callees (1):
  0x0079CFB0  CClientExoAppInternal::SetLoadBarProgress
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-3-38

GOG function: CClientExoApp::StartCreditSequence  gog 0x0040e4f0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x00741030 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00741010:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e4d0 CClientExoApp::FadeAndStopStreamingSoundSource -> 0x00741010  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e510 CClientExoApp::StopCreditSequence -> 0x00741050  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044d730 CClientExoAppInternal::StartCreditSequence -> Steam ?  in candidate callees: n/a
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
prologue for 0x00741030: 16-byte aligned: YES
  00741030  55             PUSH EBP
  00741031  8B EC          MOV EBP,ESP
  00741033  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00741030
string literals (0):
direct callees (1):
  0x007A1130  CClientExoAppInternal::StartCreditSequence
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00741030 FUN_00741030 size=32 ret=8 callers=1 callees=1
THUNK -> 0x007A1130 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00741030(undefined4 param_1,undefined4 param_2)

{
  FUN_007a1130(param_1,param_2);
  return;
}
```

### decoy: 0x00741010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00741010: 16-byte aligned: YES
  00741010  55             PUSH EBP
  00741011  8B EC          MOV EBP,ESP
  00741013  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00741010
string literals (0):
direct callees (1):
  0x0078E870  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-3-39

GOG function: CClientExoApp::SetLastTarget  gog 0x0040db00  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x00740640 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007405d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dae0 CClientExoApp::GetLoadFromIFO -> 0x00740620  gate_class=AUTO
- GOG-order neighbour after:  0x0040db20 CClientExoApp::GetLastTarget -> 0x00740660  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044cb30 CClientExoAppInternal::SetLastTarget -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00740640: 16-byte aligned: YES
  00740640  55             PUSH EBP
  00740641  8B EC          MOV EBP,ESP
  00740643  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740640
string literals (0):
direct callees (1):
  0x007A0530  CClientExoAppInternal::SetLastTarget
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00740640 FUN_00740640 size=32 ret=8 callers=2 callees=1
THUNK -> 0x007A0530 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740640(undefined4 param_1,undefined4 param_2)

{
  FUN_007a0530(param_1,param_2);
  return;
}
```

### decoy: 0x007405d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007405d0: 16-byte aligned: YES
  007405d0  55             PUSH EBP
  007405d1  8B EC          MOV EBP,ESP
  007405d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007405d0
string literals (0):
direct callees (1):
  0x0079CFB0  CClientExoAppInternal::SetLoadBarProgress
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-3-40

GOG function: CClientExoApp::GetGUIString  gog 0x0040ddb0  callconv=(blank)  param_size_bytes=8  gog_size=53  gog_retn=8
Candidate Steam address: 0x007408f0 (size 53, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 30 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 30 (callers: CreateUsableItemEntry 0x77bcf0, GetTargetActions 0x87bec0, GetDoorInterfaceMines 0x87c590)
- best decoy (neighbour/other candidate) scores: 0x00740930:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dd80 CClientExoApp::AddFloatyText -> 0x007408c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ddf0 CClientExoApp::SetAutoPaused -> 0x00740930  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044a810 CClientExoAppInternal::GetGUIString -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (30 of 59): the card's A-CALLEE credit may come from these, not from callees
  0x00427c70 CSWCCreature::CreateUsableItemEntry -> Steam 0x0077bcf0  in candidate callers: yes
  0x005077a0 CSWCDoor::GetTargetActions -> Steam 0x0087bec0  in candidate callers: yes
  0x00507e70 CSWCDoor::GetDoorInterfaceMines -> Steam 0x0087c590  in candidate callers: yes
  0x00514000 CSWGuiTargetActionMenu::UpdateNameLabel -> Steam 0x00742a00  in candidate callers: yes
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

## Card B11a-3-41

GOG function: CClientExoApp::BusyWaitMuteSound  gog 0x0040e3c0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740f00 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740dc0:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e3a0 CClientExoApp::BusyWaitMenuMusic -> 0x00740ee0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e3e0 CClientExoApp::GetPazaakIsRunning -> 0x00740f20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044cc30 CClientExoAppInternal::BusyWaitMuteSound -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x004e1650 CGuiInGame::DoQuickLoad -> Steam 0x007d0f50  in candidate callers: yes
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

### decoy: 0x00740dc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740dc0: 16-byte aligned: YES
  00740dc0  55             PUSH EBP
  00740dc1  8B EC          MOV EBP,ESP
  00740dc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740dc0
string literals (0):
direct callees (1):
  0x0079FC40  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-3-42

GOG function: CClientExoApp::HideMouse  gog 0x0040e440  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740f80 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740fa0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e420 CClientExoApp::CancelMovie -> 0x00740f60  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e460 CClientExoApp::ShowMouse -> 0x00740fa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004604e0 CClientExoAppInternal::HideMouse -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740f80: 16-byte aligned: YES
  00740f80  55             PUSH EBP
  00740f81  8B EC          MOV EBP,ESP
  00740f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740f80
string literals (0):
direct callees (1):
  0x007B38D0  CClientExoAppInternal::HideMouse
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740f80 FUN_00740f80 size=28 ret=4 callers=5 callees=1
THUNK -> 0x007B38D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740f80(undefined4 param_1)

{
  FUN_007b38d0(param_1);
  return;
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

## Card B11a-3-43

GOG function: CClientExoAppInternal::StopMovie  gog 0x004452f0  callconv=(blank)  param_size_bytes=0  gog_size=158  gog_retn=0
Candidate Steam address: 0x00798c30 (size 155, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::InitializeMoviePlayer | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00798cd0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00445250 CClientExoAppInternal::InitializeMoviePlayer -> 0x00798c30  gate_class=REVIEW
- GOG-order neighbour after:  0x00445390 CClientExoAppInternal::ShowObject -> 0x007983e0  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00921140 CExoMoviePlayer::Destructor -> Steam ?  in candidate callees: n/a
  0x009211a0 CExoMoviePlayer::CancelMovie -> Steam ?  in candidate callees: n/a
  0x009211c0 CExoMoviePlayer::IsMoviePlaying -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x130*} | Steam {0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x5,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00798c30: 16-byte aligned: YES
  00798c30  55             PUSH EBP
  00798c31  8B EC          MOV EBP,ESP
  00798c33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00798c30
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x0093B470  (unnamed)
  0x0058A1B0  (unnamed)
struct offsets touched (top 1):
  +0x130 [dec 304]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00798c30 FUN_00798c30 size=155 ret=0 callers=1 callees=3

void __fastcall FUN_00798c30(int param_1)

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
  *(undefined4 *)(param_1 + 0x130) = 0;
  iVar2 = FUN_00919723(4,uVar1);
  local_8 = 0;
  if (iVar2 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_0093b470();
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x130) = local_20;
  FUN_0058a1b0(0);
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00798cd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00798cd0: 16-byte aligned: YES
  00798cd0  55             PUSH EBP
  00798cd1  8B EC          MOV EBP,ESP
  00798cd3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00798cd0
string literals (0):
direct callees (4):
  0x0093B570  (unnamed)
  0x0093B550  (unnamed)
  0x0093B4F0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x130 [dec 304]  x5  via EAX,ECX,EDX
```

## Card B11a-3-44

GOG function: CClientExoApp::ShowTutorialWindow  gog 0x0040dee0  callconv=(blank)  param_size_bytes=16  gog_size=41  gog_retn=16
Candidate Steam address: 0x00740a20 (size 41, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 6 (callers: MenuActionBash 0x87bd10, BashPlaceable 0x841d40, MenuActionSetPlaceableMine 0x842bd0)
- best decoy (neighbour/other candidate) scores: 0x00740a00:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dec0 CClientExoApp::DelayNextEnemyAutoPause -> 0x00740a00  gate_class=REVIEW
- GOG-order neighbour after:  0x0040df10 CClientExoApp::ClearCachedCreatures -> 0x00740a50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044afd0 CClientExoAppInternal::ShowTutorialWindow -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (6 of 29): the card's A-CALLEE credit may come from these, not from callees
  0x005075f0 CSWCDoor::MenuActionBash -> Steam 0x0087bd10  in candidate callers: yes
  0x005355c0 CSWCPlaceable::BashPlaceable -> Steam 0x00841d40  in candidate callers: yes
  0x00536450 CSWCPlaceable::MenuActionSetPlaceableMine -> Steam 0x00842bd0  in candidate callers: yes
  0x00563290 CSWGuiInGameMap::OnPanelAdded -> Steam 0x00894320  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x00740a20: 16-byte aligned: YES
  00740a20  55             PUSH EBP
  00740a21  8B EC          MOV EBP,ESP
  00740a23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740a20
string literals (0):
direct callees (1):
  0x0079E9D0  CClientExoAppInternal::ShowTutorialWindow
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740a20 FUN_00740a20 size=41 ret=16 callers=29 callees=1
THUNK -> 0x0079E9D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740a20(undefined1 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  FUN_0079e9d0(param_1,param_2,param_3,param_4);
  return;
}
```

### decoy: 0x00740a00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740a00: 16-byte aligned: YES
  00740a00  55             PUSH EBP
  00740a01  8B EC          MOV EBP,ESP
  00740a03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740a00
string literals (0):
direct callees (1):
  0x0079E9A0  CClientExoAppInternal::DelayNextEnemyAutoPause
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-3-45

GOG function: CClientExoApp::GetPausedByCombat  gog 0x0040d7f0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740330 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 5 (callers: ShowGalaxyMapGui 0x7ca730, HideGalaxyMapGui 0x7ca920, HideStoreGui 0x7ccf30)
- best decoy (neighbour/other candidate) scores: 0x00740350:STR0/CALLEE4/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d7d0 CClientExoApp::DismissInGameGUI -> 0x00740310  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d810 CClientExoApp::SetPausedByCombat -> 0x00740350  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448500 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (5 of 23): the card's A-CALLEE credit may come from these, not from callees
  0x004dae30 CGuiInGame::ShowGalaxyMapGui -> Steam 0x007ca730  in candidate callers: yes
  0x004db020 CGuiInGame::HideGalaxyMapGui -> Steam 0x007ca920  in candidate callers: yes
  0x004dd630 CGuiInGame::HideStoreGui -> Steam 0x007ccf30  in candidate callers: yes
  0x005eab50 CSWGuiLevelUpCharGen::BeginPlayGame -> Steam 0x008f8070  in candidate callers: yes
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

## Card B11a-3-46

GOG function: CClientExoApp::DelayNextEnemyAutoPause  gog 0x0040dec0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740a00 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callers: ToggleDoorState 0x87bb20, MenuActionSecurity 0x87bca0, MenuActionBash 0x87bd10)
- best decoy (neighbour/other candidate) scores: 0x00740a20:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dea0 CClientExoApp::GetVideoEffectType -> 0x007409e0  gate_class=AUTO
- GOG-order neighbour after:  0x0040dee0 CClientExoApp::ShowTutorialWindow -> 0x00740a20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044afa0 CClientExoAppInternal::DelayNextEnemyAutoPause -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00507400 CSWCDoor::ToggleDoorState -> Steam 0x0087bb20  in candidate callers: yes
  0x00507580 CSWCDoor::MenuActionSecurity -> Steam 0x0087bca0  in candidate callers: yes
  0x005075f0 CSWCDoor::MenuActionBash -> Steam 0x0087bd10  in candidate callers: yes
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
prologue for 0x00740a00: 16-byte aligned: YES
  00740a00  55             PUSH EBP
  00740a01  8B EC          MOV EBP,ESP
  00740a03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740a00
string literals (0):
direct callees (1):
  0x0079E9A0  CClientExoAppInternal::DelayNextEnemyAutoPause
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740a00 FUN_00740a00 size=22 ret=0 callers=3 callees=1
THUNK -> 0x0079E9A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740a00(undefined4 param_1)

{
  FUN_0079e9a0(param_1);
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

## Card B11a-3-47

GOG function: CClientExoApp::SetMouseMode  gog 0x0040e400  callconv=(blank)  param_size_bytes=1  gog_size=29  gog_retn=4
Candidate Steam address: 0x00740f40 (size 29, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
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

## Card B11a-3-48

GOG function: CClientExoAppInternal::AddToPauseExclusionList_2  gog 0x004317f0  callconv=(blank)  param_size_bytes=0  gog_size=86  gog_retn=0
Candidate Steam address: 0x00785130 (size 146, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::DestroyQuestJournal_2 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007851d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00431750 CClientExoAppInternal::DestroyQuestJournal_2 -> 0x00785130  gate_class=REVIEW
- GOG-order neighbour after:  0x00431850 CClientExoAppInternal::CreateWorldJournal -> 0x00785230  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0044de70 CClientExoAppInternal::IsMoviePlaying_2 -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0x2,0x3,0x8,0xc*,0x20,0x44}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00785130: 16-byte aligned: YES
  00785130  55             PUSH EBP
  00785131  8B EC          MOV EBP,ESP
  00785133  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00785130
string literals (0):
direct callees (3):
  0x007A1740  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x006D2CB0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00785130 FUN_00785130 size=146 ret=0 callers=1 callees=3

void __fastcall FUN_00785130(int param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_009658a3;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_007a1740(uVar1,param_1);
  local_8._1_3_ = (uint3)((uint)local_8 >> 8);
  local_8._0_1_ = 2;
  Free(uVar1,param_1,param_1 + 0x20);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_006d2cb0();
  local_8 = 3;
  Free();
  local_8 = 0xffffffff;
  FUN_006d2cb0();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007851d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007851d0: 16-byte aligned: YES
  007851d0  55             PUSH EBP
  007851d1  8B EC          MOV EBP,ESP
  007851d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007851d0
string literals (0):
direct callees (2):
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x006D2CB0  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-3-49

GOG function: CClientExoApp::SetLoadScreenByModuleName  gog 0x0040da00  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740540 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 4 (callers: DoQuickLoad 0x7d0f50, LoadGame 0x853a00, StallEventSaveGame 0x534600)
- best decoy (neighbour/other candidate) scores: 0x007405d0:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d9c0 CClientExoApp::GetClientMeleeAnimation -> 0x00740500  gate_class=REVIEW
- GOG-order neighbour after:  0x0040da20 CClientExoApp::GetLoadMusicByModuleName -> 0x00740560  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449120 CClientExoAppInternal::SetLoadScreenByModuleName -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (4 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x004e1650 CGuiInGame::DoQuickLoad -> Steam 0x007d0f50  in candidate callers: yes
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> Steam 0x00534600  in candidate callers: yes
  0x007be170 CServerExoAppInternal::DoPCAutosave -> Steam 0x0053af20  in candidate callers: yes
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
prologue for 0x00740540: 16-byte aligned: YES
  00740540  55             PUSH EBP
  00740541  8B EC          MOV EBP,ESP
  00740543  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740540
string literals (0):
direct callees (1):
  0x0079CB20  CClientExoAppInternal::SetLoadScreenByModuleName
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740540 FUN_00740540 size=28 ret=4 callers=8 callees=1
THUNK -> 0x0079CB20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740540(undefined4 param_1)

{
  FUN_0079cb20(param_1);
  return;
}
```

### decoy: 0x007405d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007405d0: 16-byte aligned: YES
  007405d0  55             PUSH EBP
  007405d1  8B EC          MOV EBP,ESP
  007405d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007405d0
string literals (0):
direct callees (1):
  0x0079CFB0  CClientExoAppInternal::SetLoadBarProgress
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-3-50

GOG function: CClientExoApp::GetNextLoadScreenHintSTRREF  gog 0x0040e280  callconv=(blank)  param_size_bytes=5  gog_size=33  gog_retn=8
Candidate Steam address: 0x00740dc0 (size 33, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 5 (callers: DoQuickLoad 0x7d0f50, LoadGame 0x853a00, StartNewModule 0x52fda0)
- best decoy (neighbour/other candidate) scores: 0x00740ce0:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e250 CClientExoApp::SetStoryHint -> 0x00740d90  gate_class=AUTO
- GOG-order neighbour after:  0x0040e2b0 CClientExoApp::SetModuleResourceLoaded -> 0x00740df0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044c240 CClientExoAppInternal::GetNextLoadScreenHintSTRREF -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (5 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x004e1650 CGuiInGame::DoQuickLoad -> Steam 0x007d0f50  in candidate callers: yes
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
  0x007b3120 CServerExoAppInternal::StartNewModule -> Steam 0x0052fda0  in candidate callers: yes
  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> Steam 0x00534600  in candidate callers: yes
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
prologue for 0x00740dc0: 16-byte aligned: YES
  00740dc0  55             PUSH EBP
  00740dc1  8B EC          MOV EBP,ESP
  00740dc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740dc0
string literals (0):
direct callees (1):
  0x0079FC40  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00740dc0 FUN_00740dc0 size=33 ret=8 callers=7 callees=1
THUNK -> 0x0079FC40 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740dc0(undefined4 param_1,undefined1 param_2)

{
  FUN_0079fc40(param_1,param_2);
  return;
}
```

### decoy: 0x00740ce0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740ce0: 16-byte aligned: YES
  00740ce0  55             PUSH EBP
  00740ce1  8B EC          MOV EBP,ESP
  00740ce3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740ce0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x488 [dec 1160]  x1  via EDX
```

## Card B11a-3-51

GOG function: CClientExoApp::GetPlayerMovementInput  gog 0x0040df60  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740aa0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandCreatureFlourishWeapon 0x66b8a0)
- best decoy (neighbour/other candidate) scores: 0x00740a70:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040df30 CClientExoApp::LookUpAndPerformRumbleWithCutOff -> 0x00740a70  gate_class=REVIEW
- GOG-order neighbour after:  0x0040df80 CClientExoAppInternal::GetPlayerMovementInput -> 0x00761570  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040df80 CClientExoAppInternal::GetPlayerMovementInput -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00780e80 Global::ExecuteCommandCreatureFlourishWeapon -> Steam 0x0066b8a0  in candidate callers: yes
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
prologue for 0x00740aa0: 16-byte aligned: YES
  00740aa0  55             PUSH EBP
  00740aa1  8B EC          MOV EBP,ESP
  00740aa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740aa0
string literals (0):
direct callees (1):
  0x00740AC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740aa0 FUN_00740aa0 size=22 ret=0 callers=3 callees=1
THUNK -> 0x00740AC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740aa0(undefined4 param_1)

{
  FUN_00740ac0(param_1);
  return;
}
```

### decoy: 0x00740a70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740a70: 16-byte aligned: YES
  00740a70  55             PUSH EBP
  00740a71  8B EC          MOV EBP,ESP
  00740a73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740a70
string literals (0):
direct callees (1):
  0x0079F010  CClientExoAppInternal::LookUpAndPerformRumbleWithCutOff
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-3-52

GOG function: CClientExoApp::StopMenuMusic  gog 0x0040d250  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fd70 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fd90:STR0/CALLEE1/SIG0.5/SRV0/VT0
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

## Card B11a-3-53

GOG function: CClientExoApp::ShowMouse  gog 0x0040e460  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740fa0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740f80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e440 CClientExoApp::HideMouse -> 0x00740f80  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e480 CClientExoApp::GetCharacterChangeInProgress -> 0x00740fc0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00460540 CClientExoAppInternal::ShowMouse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00740fa0: 16-byte aligned: YES
  00740fa0  55             PUSH EBP
  00740fa1  8B EC          MOV EBP,ESP
  00740fa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740fa0
string literals (0):
direct callees (1):
  0x007B3930  CClientExoAppInternal::ShowMouse
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740fa0 FUN_00740fa0 size=28 ret=4 callers=6 callees=1
THUNK -> 0x007B3930 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740fa0(undefined4 param_1)

{
  FUN_007b3930(param_1);
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

## Card B11a-3-54

GOG function: CClientExoApp::SetLastPartyMemberTempKilled  gog 0x0040d8c0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740400 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: OnApplyDeath 0x597d90)
- best decoy (neighbour/other candidate) scores: 0x00740420:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d8a0 CClientExoApp::GetGameOver -> 0x007403e0  gate_class=AUTO
- GOG-order neighbour after:  0x0040d8e0 CClientExoApp::SetCanSendDialog -> 0x00740420  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448ae0 CClientExoAppInternal::SetLastPartyMemberTempKilled -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0073a680 CSWSEffectListHandler::OnApplyDeath -> Steam 0x00597d90  in candidate callers: yes
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
prologue for 0x00740400: 16-byte aligned: YES
  00740400  55             PUSH EBP
  00740401  8B EC          MOV EBP,ESP
  00740403  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740400
string literals (0):
direct callees (1):
  0x0079C4E0  CClientExoAppInternal::SetLastPartyMemberTempKilled
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740400 FUN_00740400 size=28 ret=4 callers=1 callees=1
THUNK -> 0x0079C4E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740400(undefined4 param_1)

{
  FUN_0079c4e0(param_1);
  return;
}
```

### decoy: 0x00740420 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740420: 16-byte aligned: YES
  00740420  55             PUSH EBP
  00740421  8B EC          MOV EBP,ESP
  00740423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740420
string literals (0):
direct callees (1):
  0x0079C550  CClientExoAppInternal::SetCanSendDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-3-55

GOG function: CClientExoApp::EnableInput  gog 0x0040d7b0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007402d0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::DisableInput | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007402b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
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

### decoy: 0x007402b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007402b0: 16-byte aligned: YES
  007402b0  55             PUSH EBP
  007402b1  8B EC          MOV EBP,ESP
  007402b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007402b0
string literals (0):
direct callees (1):
  0x0079BE40  CClientExoAppInternal::StopSoundPlayingInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-3-56

GOG function: CClientExoApp::DisableVideoEffect  gog 0x0040de80  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007409c0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandDisableVideoEffect 0x698750)
- best decoy (neighbour/other candidate) scores: 0x007409a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040de60 CClientExoApp::EnableVideoEffect -> 0x007409a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dea0 CClientExoApp::GetVideoEffectType -> 0x007409e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044af60 CClientExoAppInternal::DisableVideoEffect -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x007adda0 Global::ExecuteCommandDisableVideoEffect -> Steam 0x00698750  in candidate callers: yes
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

## Card B11a-3-57

GOG function: CClientExoApp::StopCreditSequence  gog 0x0040e510  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00741050 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00741030:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e4f0 CClientExoApp::StartCreditSequence -> 0x00741030  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e530 CClientExoApp::GetCreditSequenceInProgress -> 0x00741070  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044d8c0 CClientExoAppInternal::StopCreditSequence -> Steam ?  in candidate callees: n/a
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

## Card B11a-3-58

GOG function: CClientExoApp::SetPlayerCreature  gog 0x0040cdf0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073fa50 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x0073f910 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fa70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cdd0 CClientExoApp::EmptyHitcheckIgnoreList -> 0x0073f8f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040ce10 CClientExoApp::HandleMessage -> 0x0073f930  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444c80 CClientExoAppInternal::SetPlayerCreature -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00785d40 Global::ExecuteCommandDestroyObject -> Steam 0x00670760  in candidate callers: no
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
prologue for 0x0073fa50: 16-byte aligned: YES
  0073fa50  55             PUSH EBP
  0073fa51  8B EC          MOV EBP,ESP
  0073fa53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fa50
string literals (0):
direct callees (1):
  0x007858D0  CClientExoAppInternal::SetCaptureEvents
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fa50 FUN_0073fa50 size=28 ret=4 callers=1 callees=1
THUNK -> 0x007858D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fa50(undefined4 param_1)

{
  FUN_007858d0(param_1);
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

## Card B11a-3-59

GOG function: CClientExoApp::EndPazaakGame  gog 0x0040dbc0  callconv=__thiscall  param_size_bytes=16  gog_size=40  gog_retn=16
Candidate Steam address: 0x007404d0 (size 36, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740700:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040db90 CClientExoApp::StartPazaakGame -> 0x007406d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040dbf0 CClientExoApp::GetPazaakResults -> 0x00740730  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449830 CClientExoAppInternal::EndPazaakGame -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
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

### decoy: 0x00740700 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740700: 16-byte aligned: YES
  00740700  55             PUSH EBP
  00740701  8B EC          MOV EBP,ESP
  00740703  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740700
string literals (0):
direct callees (1):
  0x0079D230  CClientExoAppInternal::EndPazaakGame
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-3-60

GOG function: CClientExoApp::SetPausedByCombat  gog 0x0040d810  callconv=(blank)  param_size_bytes=12  gog_size=37  gog_retn=12
Candidate Steam address: 0x00740350 (size 37, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 11 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 11 (callers: ShowGalaxyMapGui 0x7ca730, HideGalaxyMapGui 0x7ca920, HideContainerGui 0x7cad30)
- best decoy (neighbour/other candidate) scores: 0x00740330:STR0/CALLEE4/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d7f0 CClientExoApp::GetPausedByCombat -> 0x00740330  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d840 CClientExoApp::ChangeCharacter -> 0x00740380  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448540 CClientExoAppInternal::SetPausedByCombat -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (11 of 31): the card's A-CALLEE credit may come from these, not from callees
  0x004dae30 CGuiInGame::ShowGalaxyMapGui -> Steam 0x007ca730  in candidate callers: yes
  0x004db020 CGuiInGame::HideGalaxyMapGui -> Steam 0x007ca920  in candidate callers: yes
  0x004db430 CGuiInGame::HideContainerGui -> Steam 0x007cad30  in candidate callers: yes
  0x004dd630 CGuiInGame::HideStoreGui -> Steam 0x007ccf30  in candidate callers: yes
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
