# Review cards B11a chunk 5

Answer each card with one TSV row in `steam_db_batches/B11a/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B11a-5-01

GOG function: CClientExoAppInternal::GetPlayerMovementInput  gog 0x0040df80  callconv=(blank)  param_size_bytes=0  gog_size=124  gog_retn=0
Candidate Steam address: 0x007617d0 (size 154, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00761870:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040df60 CClientExoApp::GetPlayerMovementInput -> 0x00740aa0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e000 CClientExoApp::SaveOptions -> 0x00740b40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0xc,0x2a0} | Steam {}
imm sets (* = in both, 0 common): GOG {0x8,0x44} | Steam {0x4,0x4c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007617d0: 16-byte aligned: YES
  007617d0  55             PUSH EBP
  007617d1  8B EC          MOV EBP,ESP
  007617d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007617d0
string literals (0):
direct callees (2):
  0x00761CC0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007617d0 FUN_007617d0 size=154 ret=0 callers=2 callees=2

void __fastcall FUN_007617d0(int *param_1)

{
  undefined4 *puVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00964798;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  puVar1 = (undefined4 *)*param_1;
  if (puVar1 != (undefined4 *)0x0) {
    local_8 = 0;
    FUN_00761cc0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0xffffffff;
    FUN_009196fd(*puVar1);
    FUN_009196fd(puVar1);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00761870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00761870: 16-byte aligned: YES
  00761870  55             PUSH EBP
  00761871  8B EC          MOV EBP,ESP
  00761873  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00761870
string literals (0):
direct callees (2):
  0x004D9820  (unnamed)
  0x00761C00  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-5-02

GOG function: CClientExoAppInternal::PerformRButtonDownAction  gog 0x00460420  callconv=(blank)  param_size_bytes=0  gog_size=32  gog_retn=0
Candidate Steam address: 0x007b3810 (size 32, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007b3680:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00460290 CClientExoAppInternal::PerformLButtonUpAction -> 0x007b3680  gate_class=AUTO
- GOG-order neighbour after:  0x00460440 CClientExoAppInternal::PerformRButtonUpAction -> 0x007b3830  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009117d0 CSWGuiManager::HandleRMouseDown -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007b3810: 16-byte aligned: YES
  007b3810  55             PUSH EBP
  007b3811  8B EC          MOV EBP,ESP
  007b3813  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007b3810
string literals (0):
direct callees (1):
  0x00413350  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007b3810 FUN_007b3810 size=32 ret=0 callers=1 callees=1

void __fastcall FUN_007b3810(undefined4 param_1)

{
  DAT_00a7fcbc = 1;
  FUN_00413350(param_1);
  return;
}
```

### decoy: 0x007b3680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b3680: 16-byte aligned: YES
  007b3680  55             PUSH EBP
  007b3681  8B EC          MOV EBP,ESP
  007b3683  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b3680
string literals (0):
direct callees (6):
  0x0072FC30  (unnamed)
  0x00413A80  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007449F0  CSWGuiTargetActionMenu::IsTargetActionMenuControl
  0x007B3CA0  CClientExoAppInternal::HandleMouseClickInWorld
  0x004132E0  (unnamed)
struct offsets touched (top 8):
  +0x274 [dec 628]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX
  +0x9C [dec 156]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x38 [dec 56]  x1  via EDX
  +0x40 [dec 64]  x1  via EDX
  +0x98 [dec 152]  x1  via EAX
```

## Card B11a-5-03

GOG function: CClientExoAppInternal::EnableInput  gog 0x0045ffa0  callconv=(blank)  param_size_bytes=0  gog_size=24  gog_retn=0
Candidate Steam address: 0x007b3390 (size 24, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: EnableInput 0x7402f0, MainLoop 0x781be0)
- best decoy (neighbour/other candidate) scores: 0x007b3050:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0045ff80 CClientExoAppInternal::DisableInput -> 0x007b3370  gate_class=REVIEW
- GOG-order neighbour after:  0x0045ffc0 CClientExoAppInternal::DismissInGameGUI -> 0x007b33b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0040d7b0 CClientExoApp::EnableInput -> Steam 0x007402f0  in candidate callers: yes
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2bc*} | Steam {0x2bc*}
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
prologue for 0x007b3390: 16-byte aligned: YES
  007b3390  55             PUSH EBP
  007b3391  8B EC          MOV EBP,ESP
  007b3393  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007b3390
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2BC [dec 700]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007b3390 FUN_007b3390 size=24 ret=0 callers=2 callees=0

void __fastcall FUN_007b3390(int param_1)

{
  *(undefined4 *)(param_1 + 0x2bc [dec 700]) = 0;
  return;
}
```

### decoy: 0x007b3050 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b3050: 16-byte aligned: YES
  007b3050  55             PUSH EBP
  007b3051  8B EC          MOV EBP,ESP
  007b3053  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b3050
string literals (0):
direct callees (16):
  0x0072F9F0  (unnamed)
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x00798660  CClientExoAppInternal::SetPlayerCreature
  0x0078C170  CClientExoAppInternal::CleanUpModuleServices
  0x0078CEE0  (unnamed)
  0x00776DB0  (unnamed)
  0x00865250  (unnamed)
  0x007B3930  CClientExoAppInternal::ShowMouse
  0x007C9C40  CGuiInGame::SetSWGuiStatus
  0x00796B50  SetHudTarget
  0x007B3050  CClientExoAppInternal::SetInputClass
  0x007B38D0  CClientExoAppInternal::HideMouse
  0x0083BBB0  (unnamed)
  0x008398F0  (unnamed)
struct offsets touched (top 10):
  +0x40 [dec 64]  x12  via EAX,ECX,EDX
  +0x18 [dec 24]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x9C [dec 156]  x3  via EAX,ECX,EDX
  +0x30 [dec 48]  x2  via EAX,ECX
  +0x48 [dec 72]  x2  via EAX
  +0x28C [dec 652]  x2  via ECX,EDX
  +0x2A0 [dec 672]  x2  via ECX,EDX
  +0x34 [dec 52]  x1  via ECX
```

## Card B11a-5-04

GOG function: CClientExoAppInternal::MovieWindowDeactivated  gog 0x0044d6f0  callconv=(blank)  param_size_bytes=0  gog_size=30  gog_retn=0
Candidate Steam address: 0x007a10f0 (size 30, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a0fb0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044d5b0 CClientExoAppInternal::PlayMoviesInExoArrayList -> 0x007a0fb0  gate_class=AUTO
- GOG-order neighbour after:  0x0044d710 CClientExoAppInternal::MovieWindowActivated -> 0x007a1110  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0063ac00 CExoSound::SetSoundMode -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007a10f0: 16-byte aligned: YES
  007a10f0  55             PUSH EBP
  007a10f1  8B EC          MOV EBP,ESP
  007a10f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a10f0
string literals (0):
direct callees (1):
  0x0070BC60  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a10f0 FUN_007a10f0 size=30 ret=0 callers=1 callees=1
THUNK -> 0x0070BC60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007a10f0(void)

{
  FUN_0070bc60(DAT_0099c2bc,0);
  return;
}
```

### decoy: 0x007a0fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a0fb0: 16-byte aligned: YES
  007a0fb0  55             PUSH EBP
  007a0fb1  8B EC          MOV EBP,ESP
  007a0fb3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a0fb0
string literals (0):
direct callees (15):
  0x007D0AF0  CGuiInGame::SetGlobalUnpauseableState
  0x0072FA60  (unnamed)
  0x00797D90  CClientExoAppInternal::SetInputActive
  0x0072F9F0  (unnamed)
  0x0070BC60  (unnamed)
  0x007B38D0  CClientExoAppInternal::HideMouse
  0x0040BE50  Movie::PrepareWindowForMovie
  0x0093B5C0  (unnamed)
  0x0093B590  (unnamed)
  0x00798820  CClientExoAppInternal::WaitForMovie
  0x0040BF40  Movie::RestoreAfterMovie (handler of WM_USER+10 0x40A and sync call)
  0x007B3930  CClientExoAppInternal::ShowMouse
  0x0093B5E0  (unnamed)
  0x007413E0  (unnamed)
  0x006169E0  (unnamed)
struct offsets touched (top 3):
  +0x130 [dec 304]  x3  via ECX,EDX
  +0x40 [dec 64]  x2  via EAX,EDX
  +0x1F8 [dec 504]  x1  via EAX
```

## Card B11a-5-05

GOG function: CClientExoAppInternal::DisableInput  gog 0x0045ff80  callconv=(blank)  param_size_bytes=0  gog_size=24  gog_retn=0
Candidate Steam address: 0x007b3370 (size 24, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: DisableInput 0x7402d0)
- best decoy (neighbour/other candidate) scores: 0x007b3390:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0045fc60 CClientExoAppInternal::SetInputClass -> 0x007b3050  gate_class=AUTO
- GOG-order neighbour after:  0x0045ffa0 CClientExoAppInternal::EnableInput -> 0x007b3390  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0040d790 CClientExoApp::DisableInput -> Steam 0x007402d0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2bc*} | Steam {0x2bc*}
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
prologue for 0x007b3370: 16-byte aligned: YES
  007b3370  55             PUSH EBP
  007b3371  8B EC          MOV EBP,ESP
  007b3373  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007b3370
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2BC [dec 700]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007b3370 FUN_007b3370 size=24 ret=0 callers=1 callees=0

void __fastcall FUN_007b3370(int param_1)

{
  *(undefined4 *)(param_1 + 0x2bc [dec 700]) = 1;
  return;
}
```

### decoy: 0x007b3390 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b3390: 16-byte aligned: YES
  007b3390  55             PUSH EBP
  007b3391  8B EC          MOV EBP,ESP
  007b3393  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b3390
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2BC [dec 700]  x1  via EAX
```

## Card B11a-5-06

GOG function: CClientExoApp::ServerToClientObjectId  gog 0x0040d3a0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073ffc0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::SetPlayerCharacterName | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073ffa0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d380 CClientExoApp::GetSWGuiManager -> 0x0073fea0  gate_class=AUTO
- GOG-order neighbour after:  0x0040d3c0 CClientExoApp::SetInputClass -> 0x0073fee0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444780 CClientExoAppInternal::ServerToClientObjectId -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (26 of 88): the card's A-CALLEE credit may come from these, not from callees
  0x005365c0 CSWCPlaceable::GetTargetActions -> Steam 0x00842d40  in candidate callers: no
  0x00653640 CSWSCreature::AIActionPhysicalAttacks -> Steam 0x006d7390  in candidate callers: no
  0x00663910 CSWSItem::Destructor_2 -> Steam 0x006000e0  in candidate callers: no
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callers: no
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

### decoy: 0x0073ffa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ffa0: 16-byte aligned: YES
  0073ffa0  55             PUSH EBP
  0073ffa1  8B EC          MOV EBP,ESP
  0073ffa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ffa0
string literals (0):
direct callees (1):
  0x00798540  CClientExoAppInternal::IndicateObject
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-5-07

GOG function: CClientExoApp::HideMouse  gog 0x0040e440  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740f40 (size 29, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::SetMouseMode | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740f60:STR0/CALLEE0/SIG0.5/SRV0/VT0
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

### decoy: 0x00740f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740f60: 16-byte aligned: YES
  00740f60  55             PUSH EBP
  00740f61  8B EC          MOV EBP,ESP
  00740f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740f60
string literals (0):
direct callees (1):
  0x007A0500  CClientExoAppInternal::CancelMovie
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-5-08

GOG function: CClientExoAppInternal::ProcessInput  gog 0x00459080  callconv=(blank)  param_size_bytes=4  gog_size=15798  gog_retn=4
Candidate Steam address: 0x007ac470 (size 15798, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::Update | G7 mac: pass(0.88) | G8 mac_rotation: pass
- anchors: A-STR 6 | A-CALLEE 8 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR '%d %c %f %f %f\n'@0099f734; 'TILTDOWN'@0099ffe4; 'TILTUP'@0099fff0; CALLEE 8 (callees: GetInputClass 0x73ff00, GetIsObjectCorpse 0x7a79a0, HandleMouseOver 0x7b3a00; callers: MainLoop 0x781be0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007b0290:STR0/CALLEE0/SIG0.145/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00458fb0 Global::lockcamera  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0045cea0 CClientExoAppInternal::ReadPlayBackInputEvents -> 0x007b0290  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (82), mapped to Steam through trusted rows only:
  0x0040c380 Global::IsJoystickConnected -> Steam ?  in candidate callees: n/a
  0x0040c770 CClientExoApp::GetCaptureEventFile -> Steam ?  in candidate callees: n/a
  0x0040c790 CClientExoApp::GetCaptureEvents -> Steam ?  in candidate callees: n/a
  0x0040c7b0 CClientExoApp::GetCaptureEventPlayBack -> Steam ?  in candidate callees: n/a
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam 0x0073f3b0  in candidate callees: yes
  0x0040c8f0 CClientExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: yes
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: yes
  ... 74 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
GOG strings (7): '%d %c %f %f %f\\n'; '>@TILTSPEED'; 'TILTDOWN'; 'TILTUP' ...
disp sets (* = in both, 62 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x3c*,...(+48)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x3c*,...(+48)}
imm sets (* = in both, 57 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xc*,0xd*,0x10*,0x17*,0x18*,...(+43)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xc*,0xd*,0x10*,0x17*,0x18*,...(+43)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ac470: 16-byte aligned: YES
  007ac470  55             PUSH EBP
  007ac471  8B EC          MOV EBP,ESP
  007ac473  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007ac470
string literals (6):
  0x009A0014  at 007ac681  '107PER'
  0x0099F734  at 007ae74d  '%d %c %f %f %f\\n'
  0x009A0008  at 007aebee  'FL_LOOKUP'
  0x0099FFF0  at 007aecb8  'TILTUP'
  0x0099FFE4  at 007aecf6  'TILTDOWN'
  0x0099FFD8  at 007aed34  'TILTSPEED'
direct callees (82):
  0x007B12C0  CClientExoAppInternal::HandleInputEvent
  0x00733540  CExoString::Constructor
  0x00733780  CExoString::~CExoString
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0073F2F0  CClientExoApp::GetCaptureEventPlayBack
  0x007B0E90  CClientExoAppInternal::PlayBackInputEvents
  0x0051A940  (unnamed)
  0x0072FD90  (unnamed)
  0x0073F430  CClientExoApp::GetModule
  0x00783D80  CClientExoAppInternal::StallServerCode
  0x00734F60  CExoString::CompareNoCase(char*)
  0x0072FA60  (unnamed)
  0x0072FA80  (unnamed)
  0x007B5CC0  (unnamed)
  0x0072F540  (unnamed)
  0x007B5C10  (unnamed)
  0x0073F2D0  CClientExoApp::GetCaptureEvents
  0x0073F2B0  CClientExoApp::GetCaptureEventFile
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051A920  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x63  via EAX,ECX,EDX
  +0xC [dec 12]  x29  via EAX,ECX,EDX
  +0x8 [dec 8]  x25  via EAX,ECX,EDX
  +0x40 [dec 64]  x24  via EAX,ECX,EDX
  +0x2A0 [dec 672]  x22  via EAX,ECX,EDX
  +0x98 [dec 152]  x14  via EAX,ECX,EDX
  +0x274 [dec 628]  x13  via EAX,ECX,EDX
  +0x9C [dec 156]  x12  via EAX,ECX,EDX
  +0x5958 [dec 22872]  x10  via EAX,ECX,EDX
  +0x10 [dec 16]  x9  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007ac470 FUN_007ac470 size=15798 ret=4 callers=1 callees=82

/* WARNING: Removing unreachable block (ram,0x007af5a9) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007ac470(int param_1,float param_2)

{
  bool bVar1;
  char cVar2;
  undefined1 uVar3;
  short sVar4;
  uint uVar5;
  int iVar6;
  undefined4 *puVar7;
  uint *puVar8;
  uint *puVar9;
  int *piVar10;
  undefined4 uVar11;
  int iVar12;
  float10 fVar13;
  float10 fVar14;
  undefined8 uVar15;
  uint *puVar16;
  float local_7f8;
  float local_7f4;
  float local_7f0;
  float local_7ec;
  float local_7e8;
  float local_7e4;
  double local_7d8;
  double local_7c8;
  double local_7b8;
  double local_7a0;
  double local_790;
  double local_780;
  double local_768;
  double local_758;
  double local_748;
  float local_738;
  float local_734;
  float local_730;
  float local_714;
  float local_710;
  float local_70c;
  float local_708;
  float local_704;
  float local_700;
  uint local_6f4;
  uint local_6ec;
  uint local_6d8;
  uint local_6d0;
  char local_684;
  char local_674;
  undefined4 local_540;
  undefined4 local_528;
  float local_4f4;
  int *local_4a4;
  undefined4 *local_4a0;
  int local_49c;
  int local_498;
  int local_494;
  int local_490;
  int local_48c;
  int local_488;
  int local_484;
  int local_47c;
  int local_478;
  float local_474;
  float local_470;
  int local_448;
  int local_444;
  undefined4 local_440;
  undefined4 local_43c;
  int local_438;
  int local_434;
  int local_42c;
  undefined4 local_428;
  int local_424;
  undefined4 local_420;
  int local_41c;
/* ... 1384 more lines truncated by --max-lines */
```

### decoy: 0x007b0290 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b0290: 16-byte aligned: YES
  007b0290  55             PUSH EBP
  007b0291  8B EC          MOV EBP,ESP
  007b0293  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007b0290
string literals (5):
  0x009884EC  at 007b0495  '%d %c'
  0x009A0034  at 007b067d  '%d %d'
  0x009A0028  at 007b085f  '%d %d %d '
  0x009884D8  at 007b0bb3  '%f %f %f'
  0x009A001C  at 007b0d7b  '%x %f %f %f'
direct callees (7):
  0x0073F2B0  CClientExoApp::GetCaptureEventFile
  0x00739990  (unnamed)
  0x0091C7AD  (unnamed)
  0x00919723  operator new
  0x007B0E10  (unnamed)
  0x0054AA30  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x4 [dec 4]  x13  via EAX,ECX,EDX
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x24 [dec 36]  x2  via ECX,EDX
  +0x28 [dec 40]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
```

## Card B11a-5-09

GOG function: CClientExoAppInternal::StopMovie  gog 0x004452f0  callconv=(blank)  param_size_bytes=0  gog_size=158  gog_retn=0
Candidate Steam address: 0x00798cd0 (size 158, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00798c30:STR0/CALLEE0/SIG0.833/SRV0/VT0
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
imm sets (* = in both, 3 common): GOG {0x4*,0x5*,0x10*} | Steam {0x4*,0x5*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00798cd0: 16-byte aligned: YES
  00798cd0  55             PUSH EBP
  00798cd1  8B EC          MOV EBP,ESP
  00798cd3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00798cd0 FUN_00798cd0 size=158 ret=0 callers=1 callees=5

void __fastcall FUN_00798cd0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x130) != 0) {
    while (iVar1 = FUN_0093b570(), iVar1 == 1) {
      FUN_0093b550(1,1);
      Sleep(5);
    }
    iVar1 = *(int *)(param_1 + 0x130);
    if (iVar1 != 0) {
      FUN_0093b4f0();
      FUN_009196fd(iVar1);
    }
    *(undefined4 *)(param_1 + 0x130) = 0;
  }
  return;
}
```

### decoy: 0x00798c30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00798c30: 16-byte aligned: YES
  00798c30  55             PUSH EBP
  00798c31  8B EC          MOV EBP,ESP
  00798c33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00798c30
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x0093B470  (unnamed)
  0x0058A1B0  (unnamed)
struct offsets touched (top 1):
  +0x130 [dec 304]  x3  via EAX,ECX,EDX
```

## Card B11a-5-10

GOG function: CClientExoApp::ServerToClientObjectId  gog 0x0040d3a0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073fe80 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::IsPlayerCreature | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe60:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d380 CClientExoApp::GetSWGuiManager -> 0x0073fea0  gate_class=AUTO
- GOG-order neighbour after:  0x0040d3c0 CClientExoApp::SetInputClass -> 0x0073fee0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444780 CClientExoAppInternal::ServerToClientObjectId -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (26 of 88): the card's A-CALLEE credit may come from these, not from callees
  0x005365c0 CSWCPlaceable::GetTargetActions -> Steam 0x00842d40  in candidate callers: no
  0x00653640 CSWSCreature::AIActionPhysicalAttacks -> Steam 0x006d7390  in candidate callers: no
  0x00663910 CSWSItem::Destructor_2 -> Steam 0x006000e0  in candidate callers: no
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callers: no
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

### decoy: 0x0073fe60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fe60: 16-byte aligned: YES
  0073fe60  55             PUSH EBP
  0073fe61  8B EC          MOV EBP,ESP
  0073fe63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fe60
string literals (0):
direct callees (1):
  0x00797DF0  CClientExoAppInternal::TurnOffKeyboardModal
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-5-11

GOG function: CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList_2  gog 0x0043a920  callconv=(blank)  param_size_bytes=8  gog_size=450  gog_retn=8
Candidate Steam address: 0x0078e350 (size 450, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0078e520:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043a890 CClientExoAppInternal::AddObjectModelDirectlyToIgnoreList -> 0x0078e2c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0043aaf0 CClientExoAppInternal::AddItemModelDirectlyToIgnoreList -> 0x0078e520  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x004383d0 CClientExoAppInternal::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x10*,0x98*} | Steam {0x4*,0x10*,0x98*}
imm sets (* = in both, 6 common): GOG {0x8*,0x10*,0x20*,0xfb*,0xfe*,0xff*} | Steam {0x8*,0x10*,0x20*,0xfb*,0xfe*,0xff*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0078e350: 16-byte aligned: YES
  0078e350  55             PUSH EBP
  0078e351  8B EC          MOV EBP,ESP
  0078e353  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078e350
string literals (0):
direct callees (4):
  0x0078BDF0  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 3):
  +0x98 [dec 152]  x5  via EAX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x10 [dec 16]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078e350 FUN_0078e350 size=450 ret=8 callers=1 callees=4

void FUN_0078e350(undefined4 param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 extraout_var;
  byte local_5;
  
  piVar1 = (int *)FUN_0078bdf0(param_1);
  if (piVar1 != (int *)0x0) {
    iVar2 = (**(code **)(*piVar1 + 0x98))(0xff,1,extraout_var);
    if (iVar2 != 0) {
      FUN_004491c0(iVar2);
    }
    iVar2 = (**(code **)(*piVar1 + 0x98))(0xfe,1);
    if (iVar2 != 0) {
      FUN_004491c0(iVar2);
    }
    uVar3 = FUN_0076e610(1);
    iVar2 = FUN_0073f530(uVar3);
    if ((iVar2 != 0) && (iVar2 = (**(code **)(*(int *)(iVar2 + 0x10) + 0x98))(0xff,1), iVar2 != 0))
    {
      FUN_004491c0(iVar2);
    }
    uVar3 = FUN_0076e610(0x10);
    iVar2 = FUN_0073f530(uVar3);
    if (iVar2 != 0) {
      for (local_5 = 0xff; 0xfa < local_5; local_5 = local_5 - 1) {
        iVar4 = (**(code **)(*(int *)(iVar2 + 0x10) + 0x98))(local_5,1);
        if (iVar4 != 0) {
          FUN_004491c0(iVar4);
        }
      }
    }
    uVar3 = FUN_0076e610(0x20);
    iVar2 = FUN_0073f530(uVar3);
    if (iVar2 != 0) {
      for (local_5 = 0xff; 0xfa < local_5; local_5 = local_5 - 1) {
        iVar4 = (**(code **)(*(int *)(iVar2 + 0x10) + 0x98))(local_5,1);
        if (iVar4 != 0) {
          FUN_004491c0(iVar4);
        }
      }
    }
  }
  return;
}
```

### decoy: 0x0078e520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e520: 16-byte aligned: YES
  0078e520  55             PUSH EBP
  0078e521  8B EC          MOV EBP,ESP
  0078e523  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078e520
string literals (0):
direct callees (2):
  0x0078BD80  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
struct offsets touched (top 2):
  +0x10 [dec 16]  x1  via EDX
  +0x98 [dec 152]  x1  via EAX
```

## Card B11a-5-12

GOG function: CClientExoAppInternal::SetInputActive  gog 0x004443b0  callconv=(blank)  param_size_bytes=8  gog_size=84  gog_retn=8
Candidate Steam address: 0x00797d90 (size 84, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007971a0:STR0/CALLEE0/SIG0.01/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004437c0 CClientExoAppInternal::InitializeSoundOptions -> 0x007971a0  gate_class=AUTO
- GOG-order neighbour after:  0x00444410 CClientExoAppInternal::TurnOffKeyboardModal -> 0x00797df0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00444410 CClientExoAppInternal::TurnOffKeyboardModal -> Steam ?  in candidate callees: n/a
  0x00460540 CClientExoAppInternal::ShowMouse -> Steam ?  in candidate callees: n/a
  0x00616610 CExoInput::ClearEvents -> Steam ?  in candidate callees: n/a
  0x00616720 CExoInput::SetActive -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 5): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00797d90: 16-byte aligned: YES
  00797d90  55             PUSH EBP
  00797d91  8B EC          MOV EBP,ESP
  00797d93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00797d90
string literals (0):
direct callees (4):
  0x0072FB20  (unnamed)
  0x00797DF0  CClientExoAppInternal::TurnOffKeyboardModal
  0x0072F9F0  (unnamed)
  0x007B3930  CClientExoAppInternal::ShowMouse
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00797d90 FUN_00797d90 size=84 ret=8 callers=5 callees=4

void __thiscall FUN_00797d90(undefined4 param_1,int param_2,int param_3)

{
  if (DAT_00a1b48c != 0) {
    FUN_0072fb20(param_2);
  }
  if (param_2 == 0) {
    FUN_00797df0(param_1);
  }
  if (param_3 != 0) {
    FUN_0072f9f0();
  }
  if (param_2 != 0) {
    FUN_007b3930(0);
  }
  return;
}
```

### decoy: 0x007971a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007971a0: 16-byte aligned: YES
  007971a0  55             PUSH EBP
  007971a1  8B EC          MOV EBP,ESP
  007971a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007971a0
string literals (43):
  0x0099F8C4  at 0079720a  'Sound Init'
  0x00986BA8  at 0079721b  'Sound Options'
  0x00986B98  at 0079722c  '.\\\\swkotor2.ini'
  0x0099F8C4  at 007972b5  'Sound Init'
  0x00986BA8  at 007972c6  'Sound Options'
  0x00986B98  at 007972d7  '.\\\\swkotor2.ini'
  0x0099F8B0  at 0079732c  'Number 2D Voices'
  0x00986BA8  at 0079733d  'Sound Options'
  0x00986B98  at 0079734e  '.\\\\swkotor2.ini'
  0x0099F89C  at 007973d3  'Number 3D Voices'
  0x00986BA8  at 007973e7  'Sound Options'
  0x00986B98  at 007973fb  '.\\\\swkotor2.ini'
direct callees (20):
  0x00736240  (unnamed)
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x00736320  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091CF18  (unnamed)
  0x00734270  CExoString::Format
  0x00736350  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x00919723  operator new
  0x0070B4E0  CExoSound::Constructor (ret 16)
  0x0070BB50  (unnamed)
  0x0070BB20  (unnamed)
  0x0091EF5C  (unnamed)
  0x0070B7D0  (unnamed)
  0x0070BB80  (unnamed)
  0x0070B860  (unnamed)
  0x0070B890  (unnamed)
  0x0070B8C0  (unnamed)
  0x007362C0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x51 [dec 81]  x1  via EAX
  +0x54 [dec 84]  x1  via ECX
```

## Card B11a-5-13

GOG function: CClientExoAppInternal::StopLoadMusic  gog 0x0043b020  callconv=(blank)  param_size_bytes=0  gog_size=123  gog_retn=0
Candidate Steam address: 0x0078e0f0 (size 339, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::ShutDownToMainMenu | G7 mac: pass(0.10) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078e7e0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043aec0 CClientExoAppInternal::StartLoadMusic -> 0x0078e0f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0043b0a0 CClientExoAppInternal::AquireLoadMusic -> 0x0078e350  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0063a020 CExoStreamingSoundSource::Destructor -> Steam ?  in candidate callees: n/a
  0x0063a290 CExoStreamingSoundSource::Stop -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x23c} | Steam {0x4,0x14,0x40,0x48,0x108,0x18c,0x1ec,0x250,0x254}
imm sets (* = in both, 0 common): GOG {0x4,0x10} | Steam {0x2,0x2c,0x7f}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078e0f0: 16-byte aligned: YES
  0078e0f0  55             PUSH EBP
  0078e0f1  8B EC          MOV EBP,ESP
  0078e0f3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0078e0f0 FUN_0078e0f0 size=339 ret=0 callers=2 callees=9

void __fastcall FUN_0078e0f0(int param_1)

{
  undefined4 *puVar1;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  local_c = 0;
  local_8 = 0;
  local_10 = 0;
  if (*(int *)(param_1 + 0x254) != 0) {
    *(undefined4 *)(param_1 + 0x250) = 1;
  }
  FUN_00401c70();
  local_8 = FUN_00726f80();
  if (local_8 != 0) {
    if (*(int *)(local_8 + 0x48) != 0) {
      local_c = *(undefined4 *)(param_1 + 0x40);
      FUN_0053ded0(*(undefined4 *)(*(int *)(local_8 + 0x48) + 0x108),&local_10);
      FUN_007a5fa0();
    }
    FUN_00785410();
  }
  if (*(int *)(param_1 + 0x254) == 0) {
    FUN_00781590(0);
    FUN_0070bc60(DAT_0099c2a8,0);
    *(undefined4 *)(param_1 + 0x1ec) = 1;
    FUN_0078e620(1,0x7f,2);
    if (*(int *)(param_1 + 0x18c) != 0) {
      FUN_00623320();
      *(undefined4 *)(*(int *)(DAT_00a1b4a4 + 0x14) + 0x14) = 0;
      puVar1 = *(undefined4 **)(DAT_00a1b4a4 + 0x14);
      *puVar1 = 0;
      puVar1[1 = byte +0x4] = 0;
      *(undefined4 *)(param_1 + 0x18c) = 0;
    }
  }
  return;
}
```

### decoy: 0x0078e7e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e7e0: 16-byte aligned: YES
  0078e7e0  55             PUSH EBP
  0078e7e1  8B EC          MOV EBP,ESP
  0078e7e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078e7e0
string literals (0):
direct callees (3):
  0x0078E870  (unnamed)
  0x0070B080  StreamingSource scalar delete
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x238 [dec 568]  x4  via EAX,ECX
```

## Card B11a-5-14

GOG function: CClientExoAppInternal::UpdateCamera  gog 0x00438930  callconv=(blank)  param_size_bytes=4  gog_size=2883  gog_retn=4
Candidate Steam address: 0x0078bcc0 (size 93, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::ShutDownMoviePlayer | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bc50:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438910 CClientExoAppInternal::InitializeInGameGUI -> 0x0078bc50  gate_class=REVIEW
- GOG-order neighbour after:  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> 0x0078cea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (32), mapped to Steam through trusted rows only:
  0x0040c380 Global::IsJoystickConnected -> Steam ?  in candidate callees: n/a
  0x0040c770 CClientExoApp::GetCaptureEventFile -> Steam ?  in candidate callees: n/a
  0x0040c790 CClientExoApp::GetCaptureEvents -> Steam ?  in candidate callees: n/a
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam 0x0073f3b0  in candidate callees: no
  0x0040c890 CClientExoApp::GetPauseTimer -> Steam ?  in candidate callees: n/a
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: no
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam ?  in candidate callees: n/a
  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> Steam ?  in candidate callees: n/a
  ... 24 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: no
GOG strings (1): '%d %c %f %f %f\\n'
disp sets (* = in both, 1 common): GOG {0x4*,0x8,0x18,0x20,0x2c,0x40,0x48,0x64,0x68,0x6c,0x6e,0x75,0x80,0xac,...(+7)} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x2,0x4*,0x5,0x8,0xc,0x28,0x41,0x44,0x56,0x11c,0x17c,0x3e8} | Steam {0x4*,0x10,0x1c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bcc0: 16-byte aligned: YES
  0078bcc0  55             PUSH EBP
  0078bcc1  8B EC          MOV EBP,ESP
  0078bcc3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078bcc0
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078bcc0 FUN_0078bcc0 size=93 ret=4 callers=1 callees=3

int __thiscall FUN_0078bcc0(undefined4 param_1,int param_2)

{
  undefined4 local_c;
  int local_8;
  
  local_c = FUN_007a1720(param_1);
  local_8 = FUN_00521060();
  while( true ) {
    if (local_8 == 0) {
      return 0;
    }
    if (*(int *)(local_8 + 4) == param_2) break;
    local_8 = FUN_0058c370(&local_c);
  }
  return local_8;
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

## Card B11a-5-15

GOG function: CClientExoAppInternal::SetLoadScreenHint  gog 0x00449540  callconv=(blank)  param_size_bytes=4  gog_size=43  gog_retn=4
Candidate Steam address: 0x0079cf40 (size 43, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0079cf70:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00449360 CClientExoAppInternal::GetLoadMusicByModuleName -> 0x0079cd60  gate_class=AUTO
- GOG-order neighbour after:  0x004495b0 CClientExoAppInternal::SetLoadBarProgress -> 0x0079cfb0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005402d0 CSWGuiLoadScreen::SetHint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x278*} | Steam {0x278*}
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
prologue for 0x0079cf40: 16-byte aligned: YES
  0079cf40  55             PUSH EBP
  0079cf41  8B EC          MOV EBP,ESP
  0079cf43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0079cf40
string literals (0):
direct callees (1):
  0x008857E0  (unnamed)
struct offsets touched (top 1):
  +0x278 [dec 632]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0079cf40 FUN_0079cf40 size=43 ret=4 callers=2 callees=1

void __thiscall FUN_0079cf40(int param_1,undefined4 param_2)

{
  if (*(int *)(param_1 + 0x278) != 0) {
    FUN_008857e0(param_2);
  }
  return;
}
```

### decoy: 0x0079cf70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0079cf70: 16-byte aligned: YES
  0079cf70  55             PUSH EBP
  0079cf71  8B EC          MOV EBP,ESP
  0079cf73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0079cf70
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x278 [dec 632]  x2  via EAX,ECX
  +0x59C [dec 1436]  x1  via EDX
```

## Card B11a-5-16

GOG function: CClientExoAppInternal::ChangeCharacterToNextLivingPartyMember  gog 0x0044bd70  callconv=(blank)  param_size_bytes=9  gog_size=715  gog_retn=12
Candidate Steam address: 0x0079f010 (size 1041, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::LookUpAndPerformRumbleWithCutOff | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.20 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0079f430:STR0/CALLEE0/SIG0.154/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044bcf0 CClientExoAppInternal::LoadOptions -> 0x0079f6f0  gate_class=AUTO
- GOG-order neighbour after:  0x0044c050 CClientExoAppInternal::UpdateLoadingScreenHint -> 0x0079fa50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x00444550 CClientExoAppInternal::ChangeCharacter -> Steam ?  in candidate callees: n/a
  0x0044d8e0 CClientExoAppInternal::PlayCharacterSelectSound -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x00479810 CSWParty::GetIndex -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: no
  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> Steam 0x0051c600  in candidate callees: no
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0x38,0x94,0x9c,0x270,0x11ac} | Steam {0x4*,0x8*,0x1c,0x5c,0xcc,0x130}
imm sets (* = in both, 2 common): GOG {0x3*,0xc*,0x78,0xfffffffe} | Steam {0x2,0x3*,0x4,0x5,0xc*,0x24,0xb8}

### abi
```
GOG param bytes: 9; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(9/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 9 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0079f010: 16-byte aligned: YES
  0079f010  55             PUSH EBP
  0079f011  8B EC          MOV EBP,ESP
  0079f013  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0079f010
string literals (4):
  0x0099FA80  at 0079f138  'RumblePattern'
  0x0099FA80  at 0079f191  'RumblePattern'
  0x0099FA70  at 0079f233  'RumbleCutOff'
  0x0099FA70  at 0079f281  'RumbleCutOff'
direct callees (13):
  0x00733540  CExoString::Constructor
  0x00733700  CExoString::Constructor_5
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x0071BCF0  C2DA::GetINTEntry_2
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0071B530  (unnamed)
  0x0071B670  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
  0x0070B970  (unnamed)
  0x0079D3F0  CClientExoAppInternal::PlayRumblePattern
struct offsets touched (top 6):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX
  +0x130 [dec 304]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x5C [dec 92]  x1  via EAX
  +0xCC [dec 204]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0079f010 FUN_0079f010 size=1041 ret=12 callers=1 callees=13

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0079f010(undefined4 param_1,char param_2,float *param_3)

{
  undefined4 uVar1;
  int iVar2;
  undefined1 local_80 [11];
  char local_75;
  undefined1 local_74 [11];
  char local_69;
  undefined1 local_68 [11];
  char local_5d;
  undefined1 local_5c [11];
  char local_51;
  int local_48;
  undefined4 local_44;
  float local_40;
  undefined1 local_3c [8];
  int local_34;
  int local_30;
  float local_2c;
  float local_28;
  float local_24;
  undefined4 local_20;
  float local_1c;
  float local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00967af0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  local_48 = 0;
  if (param_2 == '\0') {
    local_48 = *(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x5c);
    local_30 = 0;
    local_44 = param_1;
  }
  else {
    if (param_2 != '\x01') {
      local_8 = 0xffffffff;
      FUN_00733780();
      ExceptionList = local_10;
      return;
    }
    local_48 = *(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x1c);
    local_30 = 1;
    uVar1 = FUN_00733700(param_1);
    local_8._0_1_ = 1;
    FUN_007337c0(uVar1);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
  }
  if (local_48 == 0) {
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    if (local_30 == 0) {
      FUN_00733570("RumblePattern");
      local_8._0_1_ = 3;
      iVar2 = FUN_0071bcf0(local_44,local_68,&local_20);
      local_5d = '\x01' - (iVar2 != 0);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      if (local_5d != '\0') {
        local_8 = 0xffffffff;
        FUN_00733780();
        ExceptionList = local_10;
        return;
      }
    }
    else {
/* ... 73 more lines truncated by --max-lines */
```

### decoy: 0x0079f430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0079f430: 16-byte aligned: YES
  0079f430  55             PUSH EBP
  0079f431  8B EC          MOV EBP,ESP
  0079f433  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11a-5-17

GOG function: CClientExoAppInternal::CancelMovie  gog 0x0044cb00  callconv=(blank)  param_size_bytes=0  gog_size=41  gog_retn=0
Candidate Steam address: 0x007a0500 (size 41, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: CancelMovie 0x740f60)
- best decoy (neighbour/other candidate) scores: 0x007a0490:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ca90 CClientExoAppInternal::SetMouseMode -> 0x0079fc40  gate_class=REJECT
- GOG-order neighbour after:  0x0044cb30 CClientExoAppInternal::SetLastTarget -> 0x007a0530  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009211a0 CExoMoviePlayer::CancelMovie -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0040e420 CClientExoApp::CancelMovie -> Steam 0x00740f60  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x130*} | Steam {0x130*}
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
prologue for 0x007a0500: 16-byte aligned: YES
  007a0500  55             PUSH EBP
  007a0501  8B EC          MOV EBP,ESP
  007a0503  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a0500
string literals (0):
direct callees (1):
  0x0093B550  (unnamed)
struct offsets touched (top 1):
  +0x130 [dec 304]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007a0500 FUN_007a0500 size=41 ret=0 callers=1 callees=1

void __fastcall FUN_007a0500(int param_1)

{
  if (*(int *)(param_1 + 0x130) != 0) {
    FUN_0093b550(1,0);
  }
  return;
}
```

### decoy: 0x007a0490 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a0490: 16-byte aligned: YES
  007a0490  55             PUSH EBP
  007a0491  8B EC          MOV EBP,ESP
  007a0493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a0490
string literals (0):
direct callees (5):
  0x007B38D0  CClientExoAppInternal::HideMouse
  0x0072FBF0  (unnamed)
  0x00413650  (unnamed)
  0x004139D0  (unnamed)
  0x007B3930  CClientExoAppInternal::ShowMouse
struct offsets touched (top 1):
  +0x274 [dec 628]  x3  via EAX,ECX
```

## Card B11a-5-18

GOG function: CClientExoAppInternal::StartLoadMusic  gog 0x0043aec0  callconv=(blank)  param_size_bytes=0  gog_size=352  gog_retn=0
Candidate Steam address: 0x0078e0f0 (size 339, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::ShutDownToMainMenu | G7 mac: pass(0.10) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078e250:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043ae40 CClientExoAppInternal::FadeAndStopStreamingSoundSource -> 0x0078eba0  gate_class=REVIEW
- GOG-order neighbour after:  0x0043b020 CClientExoAppInternal::StopLoadMusic -> 0x0078e0f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0043adb0 CClientExoAppInternal::StopMenuMusic -> Steam ?  in candidate callees: n/a
  0x00605c70 CExoString::operator!=_2 -> Steam ?  in candidate callees: n/a
  0x0061a160 CResRef::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00639f70 CExoStreamingSoundSource::Constructor -> Steam ?  in candidate callees: n/a
  0x0063a080 CExoStreamingSoundSource::IsPlaying -> Steam ?  in candidate callees: n/a
  0x0063a110 CExoStreamingSoundSource::Play -> Steam ?  in candidate callees: n/a
  0x0063a170 CExoStreamingSoundSource::Resume -> Steam ?  in candidate callees: n/a
  0x0063a1e0 CExoStreamingSoundSource::SetLooping -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x238,0x23c,0x2dc} | Steam {0x4,0x14,0x40,0x48,0x108,0x18c,0x1ec,0x250,0x254}
imm sets (* = in both, 1 common): GOG {0x2*,0x4,0x8,0x20,0x304} | Steam {0x2*,0x2c,0x7f}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078e0f0: 16-byte aligned: YES
  0078e0f0  55             PUSH EBP
  0078e0f1  8B EC          MOV EBP,ESP
  0078e0f3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0078e0f0 FUN_0078e0f0 size=339 ret=0 callers=2 callees=9

void __fastcall FUN_0078e0f0(int param_1)

{
  undefined4 *puVar1;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  local_c = 0;
  local_8 = 0;
  local_10 = 0;
  if (*(int *)(param_1 + 0x254) != 0) {
    *(undefined4 *)(param_1 + 0x250) = 1;
  }
  FUN_00401c70();
  local_8 = FUN_00726f80();
  if (local_8 != 0) {
    if (*(int *)(local_8 + 0x48) != 0) {
      local_c = *(undefined4 *)(param_1 + 0x40);
      FUN_0053ded0(*(undefined4 *)(*(int *)(local_8 + 0x48) + 0x108),&local_10);
      FUN_007a5fa0();
    }
    FUN_00785410();
  }
  if (*(int *)(param_1 + 0x254) == 0) {
    FUN_00781590(0);
    FUN_0070bc60(DAT_0099c2a8,0);
    *(undefined4 *)(param_1 + 0x1ec) = 1;
    FUN_0078e620(1,0x7f,2);
    if (*(int *)(param_1 + 0x18c) != 0) {
      FUN_00623320();
      *(undefined4 *)(*(int *)(DAT_00a1b4a4 + 0x14) + 0x14) = 0;
      puVar1 = *(undefined4 **)(DAT_00a1b4a4 + 0x14);
      *puVar1 = 0;
      puVar1[1 = byte +0x4] = 0;
      *(undefined4 *)(param_1 + 0x18c) = 0;
    }
  }
  return;
}
```

### decoy: 0x0078e250 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e250: 16-byte aligned: YES
  0078e250  55             PUSH EBP
  0078e251  8B EC          MOV EBP,ESP
  0078e253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11a-5-19

GOG function: CClientExoAppInternal::SetAutoPaused  gog 0x0044aa20  callconv=(blank)  param_size_bytes=5  gog_size=296  gog_retn=8
Candidate Steam address: 0x0079e210 (size 528, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::GetGUIString | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0079e420:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044a810 CClientExoAppInternal::GetGUIString -> 0x0079e210  gate_class=AUTO
- GOG-order neighbour after:  0x0044ab50 CClientExoAppInternal::EnableVideoEffect -> 0x0079e550  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00448500 unnamed -> Steam ?  in candidate callees: n/a
  0x00448540 CClientExoAppInternal::SetPausedByCombat -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x40,0xcc,0x384,0x38c,0x390,0x398,0x39c} | Steam {0x4,0x358,0x35c}
imm sets (* = in both, 2 common): GOG {0x8*,0x9,0xc*,0x41,0xfffffffe} | Steam {0x2,0x3,0x4,0x8*,0xc*,0x50,0x358}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0079e210: 16-byte aligned: YES
  0079e210  55             PUSH EBP
  0079e211  8B EC          MOV EBP,ESP
  0079e213  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0079e210
string literals (0):
direct callees (7):
  0x00733540  CExoString::Constructor
  0x007337C0  CExoString::operator=
  0x0071DEE0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007335F0  CExoString::CExoString_copy
  0x007A19E0  (unnamed)
  0x0071EBD0  (unnamed)
struct offsets touched (top 2):
  +0x358 [dec 856]  x4  via EAX,ECX,EDX
  +0x35C [dec 860]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0079e210 FUN_0079e210 size=528 ret=8 callers=3 callees=7

undefined4 __thiscall FUN_0079e210(int param_1,undefined4 param_2,uint param_3)

{
  int iVar1;
  uint local_74[2 = byte +0x8];
  undefined1 *puStack_6c;
  undefined1 *puStack_68;
  uint uStack_64;
  undefined1 *local_60;
  undefined4 local_5c;
  undefined4 local_58;
  int local_54;
  undefined1 *local_50;
  int local_4c;
  int local_48;
  uint local_44;
  undefined1 *local_40;
  undefined1 local_3c [8];
  uint local_34;
  undefined1 local_30 [8];
  undefined1 local_28 [8];
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009679b1;
  local_10 = ExceptionList;
  uStack_64 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_44 = 0;
  local_20 = 0;
  local_48 = *(int *)(param_1 + 0x35c);
  local_14 = local_48 + -1;
  local_18 = 0;
  local_1c = 0;
  puStack_68 = (undefined1 *)0x79e271;
  local_54 = param_1;
  FUN_00733540();
  local_8 = 1;
  iVar1 = local_14;
  while ((local_14 = iVar1, local_20 < local_14 && (local_1c == 0))) {
    local_18 = (local_14 - local_20) / 2 + local_20;
    if (*(uint *)(local_18 * 0xc + *(int *)(local_54 + 0x358)) == param_3) {
      puStack_68 = (undefined1 *)(*(int *)(local_54 + 0x358) + 4 + local_18 * 0xc);
      puStack_6c = (undefined1 *)0x79e2ce;
      FUN_007337c0();
      local_1c = 1;
      iVar1 = local_14;
    }
    else {
      iVar1 = local_18;
      if (param_3 <= *(uint *)(local_18 * 0xc + *(int *)(local_54 + 0x358))) {
        local_20 = local_18 + 1;
        iVar1 = local_14;
      }
    }
  }
  if (local_1c == 0) {
    puStack_68 = (undefined1 *)0x79e316;
    FUN_00733540();
    local_8._0_1_ = 2;
    puStack_68 = (undefined1 *)param_3;
    puStack_6c = local_3c;
    local_74[1 = byte +0x4] = 0x79e32d;
    puStack_68 = (undefined1 *)FUN_0071dee0();
    local_8._0_1_ = 3;
    puStack_6c = (undefined1 *)0x79e346;
    local_5c = puStack_68;
    local_58 = puStack_68;
    FUN_007337c0();
    local_8._0_1_ = 2;
    puStack_68 = (undefined1 *)0x79e352;
    FUN_00733780();
    local_34 = param_3;
/* ... 31 more lines truncated by --max-lines */
```

### decoy: 0x0079e420 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0079e420: 16-byte aligned: YES
  0079e420  55             PUSH EBP
  0079e421  8B EC          MOV EBP,ESP
  0079e423  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0079e420
string literals (0):
direct callees (2):
  0x0079BF00  (unnamed)
  0x0079BF40  CClientExoAppInternal::SetPausedByCombat
struct offsets touched (top 7):
  +0x384 [dec 900]  x5  via EAX,ECX,EDX
  +0x38C [dec 908]  x2  via ECX,EDX
  +0x39C [dec 924]  x2  via EAX,EDX
  +0x40 [dec 64]  x1  via ECX
  +0xCC [dec 204]  x1  via EDX
  +0x390 [dec 912]  x1  via EAX
  +0x398 [dec 920]  x1  via EDX
```

## Card B11a-5-20

GOG function: CClientExoAppInternal::PerformZMouseMoveAction  gog 0x00460460  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x007b3850 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007b38d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00460440 CClientExoAppInternal::PerformRButtonUpAction -> 0x007b3830  gate_class=REVIEW
- GOG-order neighbour after:  0x00460480 CClientExoAppInternal::SetMouseCursor -> 0x007b3870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00911920 CSWGuiManager::HandleZMouse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007b3850: 16-byte aligned: YES
  007b3850  55             PUSH EBP
  007b3851  8B EC          MOV EBP,ESP
  007b3853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007b3850
string literals (0):
direct callees (1):
  0x004134A0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007b3850 FUN_007b3850 size=28 ret=4 callers=1 callees=1
THUNK -> 0x004134A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007b3850(undefined4 param_1)

{
  FUN_004134a0(param_1);
  return;
}
```

### decoy: 0x007b38d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b38d0: 16-byte aligned: YES
  007b38d0  55             PUSH EBP
  007b38d1  8B EC          MOV EBP,ESP
  007b38d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b38d0
string literals (0):
direct callees (2):
  0x0072FCB0  (unnamed)
  0x00413A40  (unnamed)
struct offsets touched (top 2):
  +0x3D8 [dec 984]  x3  via EAX,EDX
  +0x274 [dec 628]  x2  via ECX,EDX
```

## Card B11a-5-21

GOG function: CClientExoAppInternal::StopCreditSequence  gog 0x0044d8c0  callconv=(blank)  param_size_bytes=0  gog_size=24  gog_retn=0
Candidate Steam address: 0x007a12c0 (size 24, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: StopCreditSequence 0x741050)
- best decoy (neighbour/other candidate) scores: 0x007a1130:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044d730 CClientExoAppInternal::StartCreditSequence -> 0x007a1130  gate_class=AUTO
- GOG-order neighbour after:  0x0044d8e0 CClientExoAppInternal::PlayCharacterSelectSound -> 0x007a12e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0040e510 CClientExoApp::StopCreditSequence -> Steam 0x00741050  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4c0*} | Steam {0x4c0*}
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
prologue for 0x007a12c0: 16-byte aligned: YES
  007a12c0  55             PUSH EBP
  007a12c1  8B EC          MOV EBP,ESP
  007a12c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a12c0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4C0 [dec 1216]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007a12c0 FUN_007a12c0 size=24 ret=0 callers=1 callees=0

void __fastcall FUN_007a12c0(int param_1)

{
  *(undefined4 *)(param_1 + 0x4c0) = 0;
  return;
}
```

### decoy: 0x007a1130 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a1130: 16-byte aligned: YES
  007a1130  55             PUSH EBP
  007a1131  8B EC          MOV EBP,ESP
  007a1133  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007a1130
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x00892090  (unnamed)
  0x00410800  GuiManager::Contains
  0x00410530  CSWGuiManager::AddPanel
  0x00726F80  (unnamed)
  0x00890260  (unnamed)
struct offsets touched (top 6):
  +0x48 [dec 72]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x1E0 [dec 480]  x2  via EAX,ECX
  +0x4C0 [dec 1216]  x2  via EAX
  +0x4C4 [dec 1220]  x1  via EAX
```

## Card B11a-5-22

GOG function: CClientExoAppInternal::AddObjectModelDirectlyToIgnoreList  gog 0x0043a890  callconv=(blank)  param_size_bytes=8  gog_size=137  gog_retn=8
Candidate Steam address: 0x0078e2c0 (size 137, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0078e250:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043a820 CClientExoAppInternal::RestartMoviesIfNecessary -> 0x0078e250  gate_class=REVIEW
- GOG-order neighbour after:  0x0043a920 CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList_2 -> 0x0078e350  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0043a920 CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList_2 -> Steam ?  in candidate callees: n/a
  0x0043aaf0 CClientExoAppInternal::AddItemModelDirectlyToIgnoreList -> Steam ?  in candidate callees: n/a
  0x0043ab70 CClientExoAppInternal::AddSimpleObjectModelDirectlyToIgnoreList -> Steam ?  in candidate callees: n/a
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x14*} | Steam {0x8*,0x14*}
imm sets (* = in both, 4 common): GOG {0x5*,0x6*,0x8*,0x14*} | Steam {0x5*,0x6*,0x8*,0x14*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0078e2c0: 16-byte aligned: YES
  0078e2c0  55             PUSH EBP
  0078e2c1  8B EC          MOV EBP,ESP
  0078e2c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078e2c0
string literals (0):
direct callees (4):
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0078E350  CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList_2
  0x0078E520  CClientExoAppInternal::AddItemModelDirectlyToIgnoreList
  0x0078E5A0  CClientExoAppInternal::AddSimpleObjectModelDirectlyToIgnoreList
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078e2c0 FUN_0078e2c0 size=137 ret=8 callers=1 callees=4

void FUN_0078e2c0(undefined4 param_1,undefined4 param_2)

{
  int local_c;
  int local_8;
  
  local_c = 0;
  FUN_0053dfb0(param_1,&local_c);
  local_8 = local_c;
  if (local_c != 0) {
    if (*(char *)(local_c + 8) == '\x05') {
      FUN_0078e350(param_1,param_2);
    }
    else if (*(char *)(local_c + 8) == '\x06') {
      FUN_0078e520(param_1,param_2);
    }
    else {
      FUN_0078e5a0(param_1,param_2);
    }
  }
  return;
}
```

### decoy: 0x0078e250 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e250: 16-byte aligned: YES
  0078e250  55             PUSH EBP
  0078e251  8B EC          MOV EBP,ESP
  0078e253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11a-5-23

GOG function: CClientExoAppInternal::InitializeMoviePlayer_2  gog 0x0044ded0  callconv=(blank)  param_size_bytes=8  gog_size=113  gog_retn=8
Candidate Steam address: 0x007a1130 (size 397, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::StartCreditSequence | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.23 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a1380:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044deb0 CClientExoAppInternal::GetPausedByCombat -> 0x007a1b90  gate_class=REVIEW
- GOG-order neighbour after:  0x0044df50 CClientExoAppInternal::GetNearestVideoMode -> 0x007a1bf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00460630 CClientExoAppInternal::HandleMouseOver -> Steam 0x007b3a00  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0x48,0x1e0,0x4c0,0x4c4}
imm sets (* = in both, 1 common): GOG {0x8*,0xc,0x10} | Steam {0x3,0x4,0x8*,0x38,0x54,0x61c}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007a1130: 16-byte aligned: YES
  007a1130  55             PUSH EBP
  007a1131  8B EC          MOV EBP,ESP
  007a1133  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007a1130
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x00892090  (unnamed)
  0x00410800  GuiManager::Contains
  0x00410530  CSWGuiManager::AddPanel
  0x00726F80  (unnamed)
  0x00890260  (unnamed)
struct offsets touched (top 6):
  +0x48 [dec 72]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x1E0 [dec 480]  x2  via EAX,ECX
  +0x4C0 [dec 1216]  x2  via EAX
  +0x4C4 [dec 1220]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007a1130 FUN_007a1130 size=397 ret=8 callers=1 callees=6

void __thiscall FUN_007a1130(int param_1,undefined4 *param_2,undefined4 param_3)

{
  int iVar1;
  CSize *local_48;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00967d3b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x4c0) == 0) {
    iVar1 = FUN_00919723(0x61c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_48 = (CSize *)0x0;
    }
    else {
      local_48 = (CSize *)FUN_00892090(DAT_00a1b49c,param_3);
    }
    local_8 = 0xffffffff;
    if (local_48 != (CSize *)0x0) {
      iVar1 = CMFCVisualManagerBitmapCache::IsCached(DAT_00a1b49c,local_48);
      if (iVar1 == 0) {
        if (*(int *)(param_1 + 0x4c4) == 0) {
          FUN_00410530(local_48,3,1);
        }
        else {
          FUN_00410530(local_48,1,1);
        }
      }
      *(undefined4 *)(local_48 + 0x54) = *param_2;
      *(undefined4 *)(local_48 + 0x58) = param_2[1 = byte +0x4];
      *(undefined4 *)(local_48 + 0x5c) = param_2[2 = byte +0x8];
      iVar1 = FUN_00726f80();
      if (((iVar1 != 0) && (iVar1 = FUN_00726f80(), *(int *)(iVar1 + 0x48) != 0)) &&
         (iVar1 = FUN_00726f80(), *(int *)(*(int *)(iVar1 + 0x48) + 0x1e0) != 0)) {
        FUN_00726f80();
        FUN_00890260(0);
      }
      *(undefined4 *)(param_1 + 0x4c0) = 1;
    }
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007a1380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a1380: 16-byte aligned: YES
  007a1380  55             PUSH EBP
  007a1381  8B EC          MOV EBP,ESP
  007a1383  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a1380
string literals (0):
direct callees (2):
  0x0071DEE0  (unnamed)
  0x0072FEA0  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-5-24

GOG function: CClientExoApp::HideMouse  gog 0x0040e440  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740fa0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::ShowMouse | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740f80:STR0/CALLEE0/SIG1/SRV0/VT0
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

## Card B11a-5-25

GOG function: CClientExoAppInternal::GetDefaultActions  gog 0x00460d20  callconv=(blank)  param_size_bytes=0  gog_size=6909  gog_retn=0
Candidate Steam address: 0x007b40f0 (size 6909, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::BuildHoverActions | G7 mac: pass(0.90) | G8 mac_rotation: pass
- anchors: A-STR 7 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'UseSearch'@0099d780; 'i_dialog'@0099f364; 'i_disablemine'@009a0078; CALLEE 5 (callees: GetServerArea 0x7aae00, GetCurrentAnimation 0x7ed7d0, GetServerDoor 0x87be80; callers: HandleMouseOver 0x7b3a00); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007b3ee0:STR0/CALLEE1/SIG0.09/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00460b10 CClientExoAppInternal::ActionIconToMouseCursor -> 0x007b3ca0  gate_class=REJECT
- GOG-order neighbour after:  0x00462840 CExoArrayList_CExoInputEvent_::Add  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (24), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: yes
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x0040d070 CClientExoApp::GetSWParty -> Steam ?  in candidate callees: n/a
  0x0040d3a0 CClientExoApp::ServerToClientObjectId -> Steam ?  in candidate callees: n/a
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam 0x007408f0  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x0042a040 CSWCCreature::GetInterfaceTargetType -> Steam ?  in candidate callees: n/a
  0x0042c190 CExoArrayList_CSWGuiInterfaceAction_::SetSize -> Steam ?  in candidate callees: n/a
  ... 16 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00460630 CClientExoAppInternal::HandleMouseOver -> Steam 0x007b3a00  in candidate callers: yes
GOG strings (9): 'i_dialog'; 'i_attack'; '@i_recovermine'; 'i_disablemine' ...
disp sets (* = in both, 34 common): GOG {0x4*,0x8*,0xc*,0x14*,0x1c*,0x20*,0x2c*,0x30*,0x38*,0x3c*,0x4c*,0x68*,0x94*,0x98*,...(+20)} | Steam {0x4*,0x8*,0xc*,0x14*,0x1c*,0x20*,0x2c*,0x30*,0x38*,0x3c*,0x4c*,0x68*,0x94*,0x98*,...(+20)}
imm sets (* = in both, 37 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3c*,0x16d*,...(+23)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3c*,0x16d*,...(+23)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007b40f0: 16-byte aligned: YES
  007b40f0  55             PUSH EBP
  007b40f1  8B EC          MOV EBP,ESP
  007b40f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007b40f0
string literals (12):
  0x009A00AC  at 007b4427  'i_noaction'
  0x0099F364  at 007b45db  'i_dialog'
  0x0099D780  at 007b46f2  'UseSearch'
  0x009A00A0  at 007b48da  'i_openplace'
  0x0099DF88  at 007b4cbe  'i_attack'
  0x009A0094  at 007b5090  'i_useplace'
  0x0099DF88  at 007b522c  'i_attack'
  0x009A0088  at 007b54af  'i_opendoor'
  0x0099DF88  at 007b567f  'i_attack'
  0x009A0078  at 007b5863  'i_disablemine'
  0x009A0068  at 007b59ed  'i_recovermine'
  0x0099DF88  at 007b5bc4  'i_attack'
direct callees (24):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0073F750  ClientApp::GetGuiInGame
  0x0073F4D0  CClientExoApp::GetGameObject
  0x0077E0C0  CSWCCreature::GetInterfaceTargetType
  0x007ECFA0  ActionEntry::Reset
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x007408F0  CClientExoApp::GetGUIString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00710EB0  (unnamed)
  0x00842D00  CSWCPlaceable::GetServerPlaceable
  0x007AAE00  CSWCArea::GetServerArea
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
struct offsets touched (top 10):
  +0x4CC [dec 1228]  x60  via EAX,ECX,EDX
  +0x4 [dec 4]  x43  via EAX,ECX,EDX
  +0x8 [dec 8]  x15  via EAX,ECX
  +0x4C [dec 76]  x13  via EDX
  +0xC [dec 12]  x12  via EAX,ECX
  +0x2C [dec 44]  x8  via EAX,EDX
  +0x14 [dec 20]  x6  via EAX,EDX
  +0x9C [dec 156]  x3  via EAX,EDX
  +0x2E4 [dec 740]  x3  via EAX,ECX,EDX
  +0x11AC [dec 4524]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007b40f0 FUN_007b40f0 size=6909 ret=0 callers=2 callees=24

void __fastcall FUN_007b40f0(int param_1)

{
  uint *puVar1;
  undefined4 *puVar2;
  bool bVar3;
  short sVar4;
  int iVar5;
  int *piVar6;
  undefined4 uVar7;
  int *local_3dc;
  undefined1 local_258 [16];
  code *local_248;
  undefined4 local_244;
  undefined4 local_240;
  undefined4 local_23c;
  undefined1 local_238 [8];
  undefined1 local_230 [16];
  code *local_220;
  undefined4 local_21c;
  undefined4 local_218;
  undefined4 local_214;
  undefined1 local_210 [8];
  undefined1 local_208 [16];
  code *local_1f8;
  undefined4 local_1f4;
  undefined4 local_1f0;
  undefined4 local_1ec;
  undefined1 local_1e8 [8];
  undefined1 local_1e0 [16];
  code *local_1d0;
  undefined4 local_1cc;
  undefined4 local_1c8;
  undefined4 local_1c4;
  undefined1 local_1c0 [8];
  undefined1 local_1b8 [16];
  code *local_1a8;
  undefined4 local_1a4;
  undefined4 local_1a0;
  undefined4 local_19c;
  undefined1 local_198 [8];
  undefined1 local_190 [16];
  code *local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined1 local_170 [8];
  undefined1 local_168 [16];
  code *local_158;
  undefined4 local_154;
  undefined4 local_150;
  undefined4 local_14c;
  undefined1 local_148 [8];
  undefined1 local_140 [16];
  code *local_130;
  undefined4 local_12c;
  undefined4 local_128;
  undefined4 local_124;
  undefined1 local_120 [8];
  undefined1 local_118 [16];
  code *local_108;
  undefined4 local_104;
  undefined4 local_100;
  undefined4 local_fc;
  undefined1 local_f8 [8];
  undefined1 local_f0 [8];
  int local_e8;
  undefined1 local_e4 [16];
  code *local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined1 local_c4 [8];
  undefined1 local_bc [16];
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined1 local_9c [8];
/* ... 461 more lines truncated by --max-lines */
```

### decoy: 0x007b3ee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b3ee0: 16-byte aligned: YES
  007b3ee0  55             PUSH EBP
  007b3ee1  8B EC          MOV EBP,ESP
  007b3ee3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b3ee0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x7B40B4 [dec 8077492]  x1  via EAX
  +0x7B40D8 [dec 8077528]  x1  via ECX
```

## Card B11a-5-26

GOG function: CClientExoAppInternal::PlayMovieQueue  gog 0x0044d550  callconv=(blank)  param_size_bytes=4  gog_size=91  gog_retn=4
Candidate Steam address: 0x007a0f50 (size 91, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007a0f10:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044d510 CClientExoAppInternal::AddMovieToMovieQueue -> 0x007a0f10  gate_class=AUTO
- GOG-order neighbour after:  0x0044d5b0 CClientExoAppInternal::PlayMoviesInExoArrayList -> 0x007a0fb0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040e9e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0044d5b0 CClientExoAppInternal::PlayMoviesInExoArrayList -> Steam ?  in candidate callees: n/a
  0x005ac200 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0xc*,0x2e4*,0x2f0*} | Steam {0x4*,0xc*,0x2e4*,0x2f0*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007a0f50: 16-byte aligned: YES
  007a0f50  55             PUSH EBP
  007a0f51  8B EC          MOV EBP,ESP
  007a0f53  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a0f50
string literals (0):
direct callees (3):
  0x007A0FB0  CClientExoAppInternal::PlayMoviesInExoArrayList
  0x007413E0  (unnamed)
  0x006169E0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a0f50 FUN_007a0f50 size=91 ret=4 callers=2 callees=3

undefined4 __thiscall FUN_007a0f50(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined1 local_8 [4];
  
  uVar1 = FUN_007a0fb0(param_1 + 0x2e4,local_8,0,param_2,param_1 + 0x2f0,0);
  FUN_007413e0(0);
  FUN_006169e0(0);
  return uVar1;
}
```

### decoy: 0x007a0f10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a0f10: 16-byte aligned: YES
  007a0f10  55             PUSH EBP
  007a0f11  8B EC          MOV EBP,ESP
  007a0f13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a0f10
string literals (0):
direct callees (2):
  0x0083EA60  CExoArrayList<uint32>::Add
  0x007A0A40  CClientExoAppInternal::AddMovieToExoArrayList
struct offsets touched (top 0):
```

## Card B11a-5-27

GOG function: CClientExoAppInternal::AddMoveToModuleMovie  gog 0x00445190  callconv=(blank)  param_size_bytes=4  gog_size=34  gog_retn=4
Candidate Steam address: 0x00798b70 (size 34, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: AddMoveToModuleMovie 0x740180)
- best decoy (neighbour/other candidate) scores: 0x00798660:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00444e40 CClientExoAppInternal::WaitForMovie -> 0x00798820  gate_class=AUTO
- GOG-order neighbour after:  0x004451c0 CClientExoAppInternal::PlayMovie -> 0x00798580  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044d040 CClientExoAppInternal::AddMovieToExoArrayList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0040d640 CClientExoApp::AddMoveToModuleMovie -> Steam 0x00740180  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x2d8*} | Steam {0x4*,0x2d8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00798b70: 16-byte aligned: YES
  00798b70  55             PUSH EBP
  00798b71  8B EC          MOV EBP,ESP
  00798b73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00798b70
string literals (0):
direct callees (1):
  0x007A0A40  CClientExoAppInternal::AddMovieToExoArrayList
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00798b70 FUN_00798b70 size=34 ret=4 callers=2 callees=1
THUNK -> 0x007A0A40 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __thiscall FUN_00798b70(int param_1,undefined4 param_2)

{
  FUN_007a0a40(param_2,param_1 + 0x2d8);
  return;
}
```

### decoy: 0x00798660 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00798660: 16-byte aligned: YES
  00798660  55             PUSH EBP
  00798661  8B EC          MOV EBP,ESP
  00798663  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00798660
string literals (0):
direct callees (1):
  0x007E5730  CSWParty::AddCharacter(id)
struct offsets touched (top 2):
  +0x20 [dec 32]  x2  via EAX
  +0x270 [dec 624]  x2  via EDX
```

## Card B11a-5-28

GOG function: CClientExoApp::SetPauseState  gog 0x0040cb60  callconv=(blank)  param_size_bytes=12  gog_size=37  gog_retn=12
Candidate Steam address: 0x0073f6f0 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::SendSaveGameRequest | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f720:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: autolabel:Statistics
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

## Card B11a-5-29

GOG function: CClientExoAppInternal::PerformRButtonUpAction  gog 0x00460440  callconv=(blank)  param_size_bytes=0  gog_size=32  gog_retn=0
Candidate Steam address: 0x007b3830 (size 32, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007b3810:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00460420 CClientExoAppInternal::PerformRButtonDownAction -> 0x007b3810  gate_class=REVIEW
- GOG-order neighbour after:  0x00460460 CClientExoAppInternal::PerformZMouseMoveAction -> 0x007b3850  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00911840 CSWGuiManager::HandleRMouseUp -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007b3830: 16-byte aligned: YES
  007b3830  55             PUSH EBP
  007b3831  8B EC          MOV EBP,ESP
  007b3833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007b3830
string literals (0):
direct callees (1):
  0x004133C0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007b3830 FUN_007b3830 size=32 ret=0 callers=1 callees=1

void __fastcall FUN_007b3830(undefined4 param_1)

{
  DAT_00a7fcbc = 0;
  FUN_004133c0(param_1);
  return;
}
```

### decoy: 0x007b3810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b3810: 16-byte aligned: YES
  007b3810  55             PUSH EBP
  007b3811  8B EC          MOV EBP,ESP
  007b3813  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b3810
string literals (0):
direct callees (1):
  0x00413350  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-5-30

GOG function: CClientExoAppInternal::HandleMouseOver  gog 0x00460630  callconv=(blank)  param_size_bytes=12  gog_size=667  gog_retn=12
Candidate Steam address: 0x007b3a00 (size 667, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.87) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: SetMouseCursor 0x7b3870); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007b3ca0:STR0/CALLEE0/SIG0.474/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00460540 CClientExoAppInternal::ShowMouse -> 0x007b3930  gate_class=AUTO
- GOG-order neighbour after:  0x004608d0 CClientExoAppInternal::HandleMouseClickInWorld -> 0x007b3ca0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040c930 CClientExoApp::GetPlayerCreatureId -> Steam ?  in candidate callees: n/a
  0x0044ded0 CClientExoAppInternal::InitializeMoviePlayer_2 -> Steam ?  in candidate callees: n/a
  0x00460480 CClientExoAppInternal::SetMouseCursor -> Steam 0x007b3870  in candidate callees: yes
  0x00460b10 CClientExoAppInternal::ActionIconToMouseCursor -> Steam ?  in candidate callees: n/a
  0x00460d20 CClientExoAppInternal::GetDefaultActions -> Steam ?  in candidate callees: n/a
  0x00616a80 CExoInput::GetLastSetMouseDownState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 12 common): GOG {0x4*,0x8*,0x30*,0x40*,0x68*,0x75*,0x98*,0xcc*,0x2b4*,0x4b0*,0x4cc*,0x4d0*} | Steam {0x4*,0x8*,0x30*,0x40*,0x68*,0x75*,0x98*,0xcc*,0x2b4*,0x4b0*,0x4cc*,0x4d0*}
imm sets (* = in both, 8 common): GOG {0x5*,0xc*,0x2d*,0x3c*,0x70*,0x2a8*,0x4b4*,0x7f000000*} | Steam {0x5*,0xc*,0x2d*,0x3c*,0x70*,0x2a8*,0x4b4*,0x7f000000*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x007b3a00: 16-byte aligned: YES
  007b3a00  55             PUSH EBP
  007b3a01  8B EC          MOV EBP,ESP
  007b3a03  83 EC 70       SUB ESP,0x70
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007b3a00
string literals (0):
direct callees (6):
  0x007B3870  CClientExoAppInternal::SetMouseCursor
  0x0073F470  CClientExoApp::GetPlayerCreatureId
  0x007B40F0  CClientExoAppInternal::BuildHoverActions
  0x007B3EE0  ActionIconToMouseCursor
  0x007A17A0  (unnamed)
  0x0072FE80  (unnamed)
struct offsets touched (top 10):
  +0x4B0 [dec 1200]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x40 [dec 64]  x4  via EAX,EDX
  +0x2B4 [dec 692]  x2  via ECX
  +0x4D0 [dec 1232]  x2  via EAX,ECX
  +0x30 [dec 48]  x1  via ECX
  +0x68 [dec 104]  x1  via ECX
  +0x75 [dec 117]  x1  via EAX
  +0x98 [dec 152]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007b3a00 FUN_007b3a00 size=667 ret=12 callers=1 callees=6

void __thiscall FUN_007b3a00(int param_1,undefined4 param_2,undefined4 *param_3,int param_4)

{
  int iVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 local_18;
  int local_10;
  
  *(undefined4 *)(param_1 + 0x4b4) = *param_3;
  *(undefined4 *)(param_1 + 0x4b8) = param_3[1 = byte +0x4];
  *(undefined4 *)(param_1 + 0x4bc) = param_3[2 = byte +0x8];
  *(undefined4 *)(param_1 + 0x4b0) = param_2;
  if ((*(int *)(param_1 + 0x40) != 0) &&
     (((*(int *)(*(int *)(param_1 + 0x40) + 0x30) != 0 ||
       (*(int *)(*(int *)(param_1 + 0x40) + 0xcc) == 1)) ||
      (*(char *)(*(int *)(param_1 + 4) + 0x75) == '\x05')))) {
    FUN_007b3870(1,0,0);
    return;
  }
  iVar1 = *(int *)(DAT_00a1b49c + 8);
  if ((param_4 == 0) &&
     (((*(int *)(param_1 + 0x4b0) == 0x7f000000 || (*(int *)(param_1 + 0x2b4) == 0x7f000000)) ||
      ((iVar4 = *(int *)(param_1 + 0x4b0), iVar3 = FUN_0073f470(), iVar4 == iVar3 ||
       ((iVar1 != 0 || (*(int *)(param_1 + 0x2b4) != *(int *)(param_1 + 0x4b0))))))))) {
    if (((*(int *)(param_1 + 0x4b0) != 0x7f000000) &&
        (iVar4 = FUN_007a17a0(*(undefined4 *)(param_1 + 0x4b0),FUN_0079b6e0), iVar4 != -1)) &&
       (iVar1 == 0)) {
      FUN_007b3870(0x2d,0,0);
      return;
    }
    uVar5 = FUN_0072fe80();
    FUN_007b3870(1,0,uVar5);
  }
  else {
    local_18 = 1;
    local_10 = 0;
    FUN_007b40f0();
    uVar5 = DAT_00a7fcc0;
    bVar2 = false;
    if (0 < *(int *)(param_1 + 0x4d0)) {
      local_10 = *(int *)(param_1 + 0x4cc);
      bVar2 = 1 < *(int *)(param_1 + 0x4d0);
    }
    if ((*(int *)(param_1 + 0x4b0) != 0x7f000000) &&
       (*(int *)(param_1 + 0x4b0) != *(int *)(*(int *)(*(int *)(param_1 + 0x40) + 0x98) + 0x68))) {
      FUN_007b3870(0x2d,0,0);
      return;
    }
    if (local_10 != 0) {
      local_18 = FUN_007b3ee0(*(undefined4 *)(local_10 + 8),bVar2);
    }
    FUN_007b3870(local_18,0,uVar5);
  }
  return;
}
```

### decoy: 0x007b3ca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b3ca0: 16-byte aligned: YES
  007b3ca0  55             PUSH EBP
  007b3ca1  8B EC          MOV EBP,ESP
  007b3ca3  83 EC 74       SUB ESP,0x74
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b3ca0
string literals (0):
direct callees (7):
  0x0073F750  ClientApp::GetGuiInGame
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0073F4D0  CClientExoApp::GetGameObject
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x007A0530  CClientExoAppInternal::SetLastTarget
  0x0074CFE0  CSWGuiMainInterface::UpdateTargetMenu
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x2B8 [dec 696]  x3  via ECX,EDX
  +0x4B0 [dec 1200]  x3  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via EDX
  +0x98 [dec 152]  x2  via EAX,EDX
  +0x2B4 [dec 692]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0x30 [dec 48]  x1  via EAX
```

## Card B11a-5-31

GOG function: CClientExoAppInternal::HandleExitGameButton  gog 0x0044dfb0  callconv=(blank)  param_size_bytes=4  gog_size=63  gog_retn=4
Candidate Steam address: 0x007a1ca0 (size 331, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.60 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a1bf0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044df50 CClientExoAppInternal::GetNearestVideoMode -> 0x007a1bf0  gate_class=REVIEW
- GOG-order neighbour after:  0x0044dff0 CClientExoAppInternal::GetLoadScreenVisible -> 0x007a2010  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044e5b0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x3,0x4*,0x9,0x24,0x28}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007a1ca0: 16-byte aligned: YES
  007a1ca0  55             PUSH EBP
  007a1ca1  8B EC          MOV EBP,ESP
  007a1ca3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007a1ca0
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x0091D67E  (unnamed)
  0x007A1AE0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007a1ca0 FUN_007a1ca0 size=331 ret=4 callers=1 callees=3

void __thiscall FUN_007a1ca0(int *param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  uint *local_34;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097f5db;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar1 = *param_1;
  param_1[2 = byte +0x8] = param_2;
  uVar2 = param_1[2 = byte +0x8];
  uVar4 = -(uint)((int)((ulonglong)uVar2 * 0x24 >> 0x20) != 0) | (uint)((ulonglong)uVar2 * 0x24);
  local_34 = (uint *)FUN_00919723(-(uint)(0xfffffffb < uVar4) | uVar4 + 4,uVar3);
  local_8 = 0;
  if (local_34 == (uint *)0x0) {
    local_34 = (uint *)0x0;
  }
  else {
    *local_34 = uVar2;
    _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
              (local_34 + 0x1 [dec 1] {uint*: byte +0x4},0x24,uVar2,FUN_0088fbb0,AfxRibbonCollector::CMemoryDC::~CMemoryDC);
    local_34 = local_34 + 0x1 [dec 1] {uint*: byte +0x4};
  }
  local_8 = 0xffffffff;
  *param_1 = (int)local_34;
  for (local_18 = 0; local_18 < param_1[1 = byte +0x4]; local_18 = local_18 + 1) {
    puVar6 = (undefined4 *)(local_18 * 0x24 + iVar1);
    puVar7 = (undefined4 *)(*param_1 + local_18 * 0x24);
    for (iVar5 = 9; iVar5 != 0; iVar5 = iVar5 + -1) {
      *puVar7 = *puVar6;
      puVar6 = puVar6 + 0x1 [dec 1] {undefined4*: byte +0x4};
      puVar7 = puVar7 + 0x1 [dec 1] {undefined4*: byte +0x4};
    }
  }
  if ((iVar1 != 0) && (iVar1 != 0)) {
    FUN_007a1ae0(3);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007a1bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a1bf0: 16-byte aligned: YES
  007a1bf0  55             PUSH EBP
  007a1bf1  8B EC          MOV EBP,ESP
  007a1bf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a1bf0
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-5-32

GOG function: CClientExoAppInternal::EnableMouseAtEndOfLoadModuleMovie  gog 0x0044d940  callconv=(blank)  param_size_bytes=0  gog_size=58  gog_retn=0
Candidate Steam address: 0x007a1340 (size 58, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a1380:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044d8e0 CClientExoAppInternal::PlayCharacterSelectSound -> 0x007a12e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0044d980 CClientExoAppInternal::GetDisplayCharacterForKey -> 0x007a1380  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00445220 CClientExoAppInternal::IsMoviePlaying -> Steam ?  in candidate callees: n/a
  0x00460540 CClientExoAppInternal::ShowMouse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x200*} | Steam {0x200*}
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
prologue for 0x007a1340: 16-byte aligned: YES
  007a1340  55             PUSH EBP
  007a1341  8B EC          MOV EBP,ESP
  007a1343  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a1340
string literals (0):
direct callees (2):
  0x00798C00  IsMoviePlaying
  0x007B3930  CClientExoAppInternal::ShowMouse
struct offsets touched (top 1):
  +0x200 [dec 512]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007a1340 FUN_007a1340 size=58 ret=0 callers=1 callees=2

void __fastcall FUN_007a1340(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_00798c00();
  if ((iVar1 == 0) && (*(int *)(param_1 + 0x200) != 0)) {
    *(undefined4 *)(param_1 + 0x200) = 0;
    FUN_007b3930(8);
  }
  return;
}
```

### decoy: 0x007a1380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a1380: 16-byte aligned: YES
  007a1380  55             PUSH EBP
  007a1381  8B EC          MOV EBP,ESP
  007a1383  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a1380
string literals (0):
direct callees (2):
  0x0071DEE0  (unnamed)
  0x0072FEA0  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-5-33

GOG function: CClientExoAppInternal::DismissInGameGUI  gog 0x0045ffc0  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x007b33b0 (size 49, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: DismissInGameGUI 0x740310)
- best decoy (neighbour/other candidate) scores: 0x007b3390:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0045ffa0 CClientExoAppInternal::EnableInput -> 0x007b3390  gate_class=REVIEW
- GOG-order neighbour after:  0x00460000 CClientExoAppInternal::PerformXMouseMoveAction -> 0x007b33f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0045fc60 CClientExoAppInternal::SetInputClass -> Steam ?  in candidate callees: n/a
  0x004da760 CGuiInGame::HideSWInGameGui -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0040d7d0 CClientExoApp::DismissInGameGUI -> Steam 0x00740310  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x40*} | Steam {0x40*}
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
prologue for 0x007b33b0: 16-byte aligned: YES
  007b33b0  55             PUSH EBP
  007b33b1  8B EC          MOV EBP,ESP
  007b33b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007b33b0
string literals (0):
direct callees (2):
  0x007CA060  CGuiInGame::HideSWInGameGui
  0x007B3050  CClientExoAppInternal::SetInputClass
struct offsets touched (top 1):
  +0x40 [dec 64]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007b33b0 FUN_007b33b0 size=49 ret=0 callers=1 callees=2

bool FUN_007b33b0(void)

{
  int iVar1;
  
  iVar1 = FUN_007ca060(0);
  if (iVar1 != 0) {
    FUN_007b3050(0,1);
  }
  return iVar1 != 0;
}
```

### decoy: 0x007b3390 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b3390: 16-byte aligned: YES
  007b3390  55             PUSH EBP
  007b3391  8B EC          MOV EBP,ESP
  007b3393  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b3390
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2BC [dec 700]  x1  via EAX
```

## Card B11a-5-34

GOG function: CClientExoAppInternal::MovieWindowActivated  gog 0x0044d710  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007a1110 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a10f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044d6f0 CClientExoAppInternal::MovieWindowDeactivated -> 0x007a10f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0044d730 CClientExoAppInternal::StartCreditSequence -> 0x007a1130  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0063ab80 CExoSound::ResumeMovieStreamingSounds -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007a1110: 16-byte aligned: YES
  007a1110  55             PUSH EBP
  007a1111  8B EC          MOV EBP,ESP
  007a1113  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a1110
string literals (0):
direct callees (1):
  0x0070BBE0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a1110 FUN_007a1110 size=22 ret=0 callers=1 callees=1
THUNK -> 0x0070BBE0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007a1110(undefined4 param_1)

{
  FUN_0070bbe0(param_1);
  return;
}
```

### decoy: 0x007a10f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a10f0: 16-byte aligned: YES
  007a10f0  55             PUSH EBP
  007a10f1  8B EC          MOV EBP,ESP
  007a10f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a10f0
string literals (0):
direct callees (1):
  0x0070BC60  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-5-35

GOG function: CClientExoAppInternal::UpdateCamera  gog 0x00438930  callconv=(blank)  param_size_bytes=4  gog_size=2883  gog_retn=4
Candidate Steam address: 0x0078bd80 (size 102, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bdf0:STR0/CALLEE0/SIG0.129/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438910 CClientExoAppInternal::InitializeInGameGUI -> 0x0078bc50  gate_class=REVIEW
- GOG-order neighbour after:  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> 0x0078cea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (32), mapped to Steam through trusted rows only:
  0x0040c380 Global::IsJoystickConnected -> Steam ?  in candidate callees: n/a
  0x0040c770 CClientExoApp::GetCaptureEventFile -> Steam ?  in candidate callees: n/a
  0x0040c790 CClientExoApp::GetCaptureEvents -> Steam ?  in candidate callees: n/a
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam 0x0073f3b0  in candidate callees: no
  0x0040c890 CClientExoApp::GetPauseTimer -> Steam ?  in candidate callees: n/a
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: no
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam ?  in candidate callees: n/a
  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> Steam ?  in candidate callees: n/a
  ... 24 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: no
GOG strings (1): '%d %c %f %f %f\\n'
disp sets (* = in both, 1 common): GOG {0x4,0x8*,0x18,0x20,0x2c,0x40,0x48,0x64,0x68,0x6c,0x6e,0x75,0x80,0xac,...(+7)} | Steam {0x8*,0x14,0x34}
imm sets (* = in both, 2 common): GOG {0x2,0x4*,0x5,0x8,0xc*,0x28,0x41,0x44,0x56,0x11c,0x17c,0x3e8} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bd80: 16-byte aligned: YES
  0078bd80  55             PUSH EBP
  0078bd81  8B EC          MOV EBP,ESP
  0078bd83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078bd80
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078bd80 FUN_0078bd80 size=102 ret=4 callers=2 callees=1

undefined4 __thiscall FUN_0078bd80(int param_1,undefined4 param_2)

{
  char cVar1;
  undefined4 uVar2;
  int *local_8;
  
  local_8 = (int *)0x0;
  uVar2 = *(undefined4 *)(param_1 + 0x14);
  cVar1 = FUN_0053dfb0(param_2,&local_8);
  if (((cVar1 == DAT_0099447f) && (local_8 != (int *)0x0)) && ((char)local_8[2 = byte +0x8] == DAT_00994475)) {
    uVar2 = (**(code **)(*local_8 + 0x34))(param_1,uVar2);
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0078bdf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bdf0: 16-byte aligned: YES
  0078bdf0  55             PUSH EBP
  0078bdf1  8B EC          MOV EBP,ESP
  0078bdf3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bdf0
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
```

## Card B11a-5-36

GOG function: CClientExoApp::HandleInputEvent  gog 0x0040d460  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073fee0 (size 32, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::SetInputClass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073ff00:STR0/CALLEE0/SIG0.75/SRV0/VT0
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

### decoy: 0x0073ff00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ff00: 16-byte aligned: YES
  0073ff00  55             PUSH EBP
  0073ff01  8B EC          MOV EBP,ESP
  0073ff03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ff00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x9C [dec 156]  x1  via ECX
```

## Card B11a-5-37

GOG function: CClientExoAppInternal::IsMoviePlaying_2  gog 0x0044de70  callconv=(blank)  param_size_bytes=0  gog_size=64  gog_retn=0
Candidate Steam address: 0x007a1900 (size 64, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a18c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044dde0 CExoArrayList_CExoString_::AddUnique  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044deb0 CClientExoAppInternal::GetPausedByCombat -> 0x007a1b90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044e2d0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007a1900: 16-byte aligned: YES
  007a1900  55             PUSH EBP
  007a1901  8B EC          MOV EBP,ESP
  007a1903  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a1900
string literals (0):
direct callees (1):
  0x007A1AE0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a1900 FUN_007a1900 size=64 ret=0 callers=3 callees=1

void __fastcall FUN_007a1900(int *param_1)

{
  if (*param_1 != 0) {
    FUN_007a1ae0(3);
  }
  *param_1 = 0;
  return;
}
```

### decoy: 0x007a18c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a18c0: 16-byte aligned: YES
  007a18c0  55             PUSH EBP
  007a18c1  8B EC          MOV EBP,ESP
  007a18c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a18c0
string literals (0):
direct callees (1):
  0x007A1CA0  CClientExoAppInternal::HandleExitGameButton
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B11a-5-38

GOG function: CClientExoAppInternal::ServerToClientObjectId  gog 0x00444780  callconv=(blank)  param_size_bytes=4  gog_size=35  gog_retn=4
Candidate Steam address: 0x00798160 (size 35, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007980a0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004446c0 CClientExoAppInternal::TogglePartyFollow -> 0x007980a0  gate_class=AUTO
- GOG-order neighbour after:  0x004447b0 CClientExoAppInternal::ShowLoadScreen -> 0x00798190  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x7f000000*,0x80000000*} | Steam {0x4*,0x7f000000*,0x80000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00798160: 16-byte aligned: YES
  00798160  55             PUSH EBP
  00798161  8B EC          MOV EBP,ESP
  00798163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00798160
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00798160 FUN_00798160 size=35 ret=4 callers=4 callees=0

uint FUN_00798160(uint param_1)

{
  if (param_1 == 0x7f000000) {
    param_1 = 0x7f000000;
  }
  else {
    param_1 = param_1 | 0x80000000;
  }
  return param_1;
}
```

### decoy: 0x007980a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007980a0: 16-byte aligned: YES
  007980a0  55             PUSH EBP
  007980a1  8B EC          MOV EBP,ESP
  007980a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007980a0
string literals (1):
  0x0099F8D0  at 007980c8  'k_sup_solo'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FEEF0  CSWPartyTable::SetSoloMode
  0x006FD8D0  CVirtualMachine::RunScript
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x238 [dec 568]  x1  via EDX
```

## Card B11a-5-39

GOG function: CClientExoAppInternal::SetMouseCursor  gog 0x00460480  callconv=(blank)  param_size_bytes=12  gog_size=86  gog_retn=12
Candidate Steam address: 0x007b3870 (size 86, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: HandleMouseOver 0x7b3a00)
- best decoy (neighbour/other candidate) scores: 0x007b38d0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00460460 CClientExoAppInternal::PerformZMouseMoveAction -> 0x007b3850  gate_class=REVIEW
- GOG-order neighbour after:  0x004604e0 CClientExoAppInternal::HideMouse -> 0x007b38d0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00616850 CExoInput::SetMouseCursor -> Steam ?  in candidate callees: n/a
  0x00911f00 CSWGuiManager::SetSoftwareMouseCursor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00460630 CClientExoAppInternal::HandleMouseOver -> Steam 0x007b3a00  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x274*} | Steam {0x274*}
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
prologue for 0x007b3870: 16-byte aligned: YES
  007b3870  55             PUSH EBP
  007b3871  8B EC          MOV EBP,ESP
  007b3873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007b3870
string literals (0):
direct callees (2):
  0x0072FC50  (unnamed)
  0x00413A80  (unnamed)
struct offsets touched (top 1):
  +0x274 [dec 628]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007b3870 FUN_007b3870 size=86 ret=12 callers=2 callees=2

void __thiscall FUN_007b3870(int param_1,uint *param_2,uint *param_3,uint *param_4)

{
  uint *extraout_EAX;
  
  if (DAT_00a1b48c != (SchedulerResourceManagement *)0x0) {
    Concurrency::details::SchedulerResourceManagement::Statistics
              (DAT_00a1b48c,param_2,param_3,param_4);
    param_2 = extraout_EAX;
  }
  if (*(int *)(param_1 + 0x274) != 0) {
    FUN_00413a80(param_2,param_3,param_4);
  }
  return;
}
```

### decoy: 0x007b38d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b38d0: 16-byte aligned: YES
  007b38d0  55             PUSH EBP
  007b38d1  8B EC          MOV EBP,ESP
  007b38d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b38d0
string literals (0):
direct callees (2):
  0x0072FCB0  (unnamed)
  0x00413A40  (unnamed)
struct offsets touched (top 2):
  +0x3D8 [dec 984]  x3  via EAX,EDX
  +0x274 [dec 628]  x2  via ECX,EDX
```

## Card B11a-5-40

GOG function: CClientExoAppInternal::SetGameOver  gog 0x0044e0d0  callconv=(blank)  param_size_bytes=4  gog_size=63  gog_retn=4
Candidate Steam address: 0x007a21d0 (size 153, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.42 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a1820:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044e070 CClientExoAppInternal::AddMoveToModuleMovie_2 -> 0x007a1820  gate_class=REVIEW
- GOG-order neighbour after:  0x0044e110 CExoArrayList_CGuiStringStruct_::Insert  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044e7b0 CExoArrayList_CGuiStringStruct_::Allocate -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8*} | Steam {0x8*,0x1c}
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
prologue for 0x007a21d0: 16-byte aligned: YES
  007a21d0  55             PUSH EBP
  007a21d1  8B EC          MOV EBP,ESP
  007a21d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a21d0
string literals (0):
direct callees (3):
  0x0058C370  (unnamed)
  0x007A1720  (unnamed)
  0x0064DDE0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x5  via ECX,EDX
  +0x1C [dec 28]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007a21d0 FUN_007a21d0 size=153 ret=4 callers=8 callees=3

undefined4 __thiscall FUN_007a21d0(int *param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  
  iVar1 = *(int *)(param_2 + 0x1c);
  if (iVar1 == 0) {
    uVar2 = 1;
  }
  else {
    if (param_1[2 = byte +0x8] == iVar1) {
      FUN_0058c370(param_1 + 0x2 [dec 2] {int*: byte +0x8});
      if (param_1[2 = byte +0x8] == 0) {
        iVar3 = FUN_007a1720();
        param_1[2 = byte +0x8] = iVar3;
        if (param_1[2 = byte +0x8] == iVar1) {
          param_1[2 = byte +0x8] = 0;
        }
      }
    }
    *param_1 = *param_1 + -0x1 [dec -1] {int*: byte -0x4};
    FUN_0064dde0(iVar1);
    *(undefined4 *)(param_2 + 0x1c) = 0;
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x007a1820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a1820: 16-byte aligned: YES
  007a1820  55             PUSH EBP
  007a1821  8B EC          MOV EBP,ESP
  007a1823  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a1820
string literals (0):
direct callees (1):
  0x007A1BF0  CClientExoAppInternal::GetNearestVideoMode
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B11a-5-41

GOG function: CClientExoAppInternal::GetObjectName  gog 0x0043b150  callconv=(blank)  param_size_bytes=8  gog_size=1424  gog_retn=8
Candidate Steam address: 0x0078e350 (size 450, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList_2 | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078e2c0:STR0/CALLEE0/SIG0.258/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043b0a0 CClientExoAppInternal::AquireLoadMusic -> 0x0078e350  gate_class=REJECT
- GOG-order neighbour after:  0x0043b710 CClientExoAppInternal::AddFeedbackMessage -> 0x0078f160  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
  0x0047f250 CSWCItem::GetName -> Steam 0x007fdfa0  in candidate callees: no
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00605b70 CExoString::operator==_2 -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  ... 6 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0040c5a0 CClientExoApp::GetObjectName -> Steam 0x0073f0e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x14,0x2c,0x30,0x34,0x3c,0x4c,0x70,0x10c,0x130,0x310} | Steam {0x4,0x10,0x98}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4,0x5,0x6,0x7,0x8*,0x14,0x1c,0x30,0xf8,0x58d} | Steam {0x8*,0x10,0x20,0xfb,0xfe,0xff}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0078e350: 16-byte aligned: YES
  0078e350  55             PUSH EBP
  0078e351  8B EC          MOV EBP,ESP
  0078e353  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078e350
string literals (0):
direct callees (4):
  0x0078BDF0  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 3):
  +0x98 [dec 152]  x5  via EAX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x10 [dec 16]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078e350 FUN_0078e350 size=450 ret=8 callers=1 callees=4

void FUN_0078e350(undefined4 param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 extraout_var;
  byte local_5;
  
  piVar1 = (int *)FUN_0078bdf0(param_1);
  if (piVar1 != (int *)0x0) {
    iVar2 = (**(code **)(*piVar1 + 0x98))(0xff,1,extraout_var);
    if (iVar2 != 0) {
      FUN_004491c0(iVar2);
    }
    iVar2 = (**(code **)(*piVar1 + 0x98))(0xfe,1);
    if (iVar2 != 0) {
      FUN_004491c0(iVar2);
    }
    uVar3 = FUN_0076e610(1);
    iVar2 = FUN_0073f530(uVar3);
    if ((iVar2 != 0) && (iVar2 = (**(code **)(*(int *)(iVar2 + 0x10) + 0x98))(0xff,1), iVar2 != 0))
    {
      FUN_004491c0(iVar2);
    }
    uVar3 = FUN_0076e610(0x10);
    iVar2 = FUN_0073f530(uVar3);
    if (iVar2 != 0) {
      for (local_5 = 0xff; 0xfa < local_5; local_5 = local_5 - 1) {
        iVar4 = (**(code **)(*(int *)(iVar2 + 0x10) + 0x98))(local_5,1);
        if (iVar4 != 0) {
          FUN_004491c0(iVar4);
        }
      }
    }
    uVar3 = FUN_0076e610(0x20);
    iVar2 = FUN_0073f530(uVar3);
    if (iVar2 != 0) {
      for (local_5 = 0xff; 0xfa < local_5; local_5 = local_5 - 1) {
        iVar4 = (**(code **)(*(int *)(iVar2 + 0x10) + 0x98))(local_5,1);
        if (iVar4 != 0) {
          FUN_004491c0(iVar4);
        }
      }
    }
  }
  return;
}
```

### decoy: 0x0078e2c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e2c0: 16-byte aligned: YES
  0078e2c0  55             PUSH EBP
  0078e2c1  8B EC          MOV EBP,ESP
  0078e2c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078e2c0
string literals (0):
direct callees (4):
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0078E350  CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList_2
  0x0078E520  CClientExoAppInternal::AddItemModelDirectlyToIgnoreList
  0x0078E5A0  CClientExoAppInternal::AddSimpleObjectModelDirectlyToIgnoreList
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
```

## Card B11a-5-42

GOG function: CClientExoAppInternal::HurryDeathSequence  gog 0x004489b0  callconv=(blank)  param_size_bytes=0  gog_size=61  gog_retn=0
Candidate Steam address: 0x0079c3b0 (size 61, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0079be40:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004486e0 CClientExoAppInternal::RunDeathSequence -> 0x0079c0e0  gate_class=AUTO
- GOG-order neighbour after:  0x004489f0 CClientExoAppInternal::SetGameSpeed -> 0x0079c3f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004dc240 CGuiInGame::StartGlobalFade -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x40*} | Steam {0x40*}
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
prologue for 0x0079c3b0: 16-byte aligned: YES
  0079c3b0  55             PUSH EBP
  0079c3b1  8B EC          MOV EBP,ESP
  0079c3b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0079c3b0
string literals (0):
direct callees (1):
  0x007CBB40  CGuiInGame::StartGlobalFade
struct offsets touched (top 1):
  +0x40 [dec 64]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0079c3b0 FUN_0079c3b0 size=61 ret=0 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0079c3b0(void)

{
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  FUN_007cbb40(0,0,_DAT_0098c014,&local_10);
  return;
}
```

### decoy: 0x0079be40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0079be40: 16-byte aligned: YES
  0079be40  55             PUSH EBP
  0079be41  8B EC          MOV EBP,ESP
  0079be43  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0079be40
string literals (0):
direct callees (3):
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0077B600  CSWCCreature::StopLipSync
  0x0070B2F0  (unnamed)
struct offsets touched (top 6):
  +0x40 [dec 64]  x3  via EAX,ECX
  +0x2C [dec 44]  x2  via EDX
  +0xF20 [dec 3872]  x2  via EDX
  +0xC [dec 12]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x1A0 [dec 416]  x1  via ECX
```

## Card B11a-5-43

GOG function: CClientExoAppInternal::PlayCharacterSelectSound  gog 0x0044d8e0  callconv=(blank)  param_size_bytes=4  gog_size=83  gog_retn=4
Candidate Steam address: 0x007a12e0 (size 83, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007a0a40:STR0/CALLEE0/SIG0.139/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044d8c0 CClientExoAppInternal::StopCreditSequence -> 0x007a12c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0044d940 CClientExoAppInternal::EnableMouseAtEndOfLoadModuleMovie -> 0x007a1340  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00411770 CSWCCreature::PlaySoundSetSound -> Steam ?  in candidate callees: n/a
  0x00925936 Global::rand -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x3*,0x4*,0x7*,0x8*,0x9*} | Steam {0x3*,0x4*,0x7*,0x8*,0x9*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007a12e0: 16-byte aligned: YES
  007a12e0  55             PUSH EBP
  007a12e1  8B EC          MOV EBP,ESP
  007a12e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a12e0
string literals (0):
direct callees (2):
  0x0091EB8B  (unnamed)
  0x00765AA0  CSWCCreature::GetSoundSetStrres
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a12e0 FUN_007a12e0 size=83 ret=4 callers=2 callees=2

void FUN_007a12e0(void)

{
  int iVar1;
  
  iVar1 = _rand [GHIDRA-AUTO-LABEL: unreliable]();
  if (iVar1 % 3 == 0) {
    FUN_00765aa0(7,1);
  }
  else if (iVar1 % 3 == 1) {
    FUN_00765aa0(8,1);
  }
  else {
    FUN_00765aa0(9,1);
  }
  return;
}
```

### decoy: 0x007a0a40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a0a40: 16-byte aligned: YES
  007a0a40  55             PUSH EBP
  007a0a41  8B EC          MOV EBP,ESP
  007a0a43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007a0a40
string literals (2):
  0x00993F38  at 007a0ca0  'LIVE%d'
  0x0099FAFC  at 007a0d15  'LIVE%d:movies\\\\%s'
direct callees (21):
  0x00733B60  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00734030  CExoString::Find
  0x00733780  CExoString::~CExoString
  0x007335F0  CExoString::CExoString_copy
  0x007A1690  (unnamed)
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x007A0860  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00919723  operator new
  0x007397F0  (unnamed)
  0x007398E0  (unnamed)
  0x00739880  (unnamed)
  0x009196FD  operator delete
  0x00734270  CExoString::Format
  0x00739790  (unnamed)
  0x00919F10  _strlen
  0x007397B0  (unnamed)
  0x007337C0  CExoString::operator=
struct offsets touched (top 2):
  +0xC [dec 12]  x2  via EAX
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-5-44

GOG function: CClientExoAppInternal::GetLoadScreenVisible  gog 0x0044dff0  callconv=(blank)  param_size_bytes=0  gog_size=64  gog_retn=0
Candidate Steam address: 0x007a2010 (size 44, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a1f50:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044dfb0 CClientExoAppInternal::HandleExitGameButton -> 0x007a1ca0  gate_class=REVIEW
- GOG-order neighbour after:  0x0044e030 CExoArrayList_CSWGuiSkillStruct_::CExoArrayList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044e390 CSWRumblePattern::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x8}
imm sets (* = in both, 0 common): GOG {0x3,0x10} | Steam {0x4}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007a2010: 16-byte aligned: YES
  007a2010  55             PUSH EBP
  007a2011  8B EC          MOV EBP,ESP
  007a2013  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2010
string literals (0):
direct callees (1):
  0x00722870  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007a2010 FUN_007a2010 size=44 ret=0 callers=1 callees=1

undefined4 * __fastcall FUN_007a2010(undefined4 *param_1)

{
  FUN_00722870();
  param_1[2 = byte +0x8] = 0;
  *param_1 = 0;
  return param_1;
}
```

### decoy: 0x007a1f50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a1f50: 16-byte aligned: YES
  007a1f50  55             PUSH EBP
  007a1f51  8B EC          MOV EBP,ESP
  007a1f53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007a1f50
string literals (0):
direct callees (3):
  0x007A1DF0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x3  via EAX,ECX
```

## Card B11a-5-45

GOG function: CClientExoAppInternal::AddMoveToModuleMovie_2  gog 0x0044e070  callconv=(blank)  param_size_bytes=4  gog_size=91  gog_retn=4
Candidate Steam address: 0x007a1820 (size 63, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.75
- best decoy (neighbour/other candidate) scores: 0x007a17a0:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044e030 CExoArrayList_CSWGuiSkillStruct_::CExoArrayList  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044e0d0 CClientExoAppInternal::SetGameOver -> 0x007a21d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0044e700 unnamed -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
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
prologue for 0x007a1820: 16-byte aligned: YES
  007a1820  55             PUSH EBP
  007a1821  8B EC          MOV EBP,ESP
  007a1823  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a1820
string literals (0):
direct callees (1):
  0x007A1BF0  CClientExoAppInternal::GetNearestVideoMode
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007a1820 FUN_007a1820 size=63 ret=4 callers=6 callees=1

undefined4 * __thiscall FUN_007a1820(undefined4 *param_1,int param_2)

{
  param_1[1 = byte +0x4] = 0;
  param_1[2 = byte +0x8] = 0;
  *param_1 = 0;
  if (param_2 != 0) {
    FUN_007a1bf0(param_2);
  }
  return param_1;
}
```

### decoy: 0x007a17a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a17a0: 16-byte aligned: YES
  007a17a0  55             PUSH EBP
  007a17a1  8B EC          MOV EBP,ESP
  007a17a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a17a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card B11a-5-46

GOG function: CClientExoAppInternal::SaveOptions  gog 0x0044bcc0  callconv=(blank)  param_size_bytes=0  gog_size=39  gog_retn=0
Candidate Steam address: 0x0079f6c0 (size 39, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0079f6f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ba30 CClientExoAppInternal::PlayerFlourishWeapons -> 0x0079f430  gate_class=AUTO
- GOG-order neighbour after:  0x0044bcf0 CClientExoAppInternal::LoadOptions -> 0x0079f6f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x00462cc0 CClientOptions::SaveOptions -> Steam ?  in candidate callees: n/a
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
prologue for 0x0079f6c0: 16-byte aligned: YES
  0079f6c0  55             PUSH EBP
  0079f6c1  8B EC          MOV EBP,ESP
  0079f6c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0079f6c0
string literals (0):
direct callees (2):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007B5F50  ClientOptions ini writer (Auto Level Up NPCs)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0079f6c0 FUN_0079f6c0 size=39 ret=0 callers=1 callees=2

void __fastcall FUN_0079f6c0(undefined4 param_1)

{
  FUN_0072fb00(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  FUN_007b5f50();
  return;
}
```

### decoy: 0x0079f6f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0079f6f0: 16-byte aligned: YES
  0079f6f0  55             PUSH EBP
  0079f6f1  8B EC          MOV EBP,ESP
  0079f6f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0079f6f0
string literals (0):
direct callees (4):
  0x007B88B0  ClientOptions ini reader ([Game Options] Auto Level Up NPCs etc.)
  0x007BBAD0  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007BBF90  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x24 [dec 36]  x1  via EDX
```

## Card B11a-5-47

GOG function: CClientExoAppInternal::FadeAndStopStreamingSoundSource  gog 0x0043ae40  callconv=(blank)  param_size_bytes=8  gog_size=118  gog_retn=8
Candidate Steam address: 0x0078eba0 (size 1424, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.08) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078e350:STR0/CALLEE0/SIG0.229/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043adb0 CClientExoAppInternal::StopMenuMusic -> 0x0078e8f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0043aec0 CClientExoAppInternal::StartLoadMusic -> 0x0078e0f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00608350 CExoTimers::GetHighResolutionTimer -> Steam ?  in candidate callees: n/a
  0x0063a2c0 CExoStreamingSoundSource::FadeAndStop -> Steam ?  in candidate callees: n/a
  0x0063a890 CExoSound::Render -> Steam ?  in candidate callees: n/a
  0x00924e50 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {0x8,0x14,0x2c,0x30,0x34,0x3c,0x4c,0x70,0x10c,0x130,0x310}
imm sets (* = in both, 1 common): GOG {0x8*,0xc,0x3e8} | Steam {0x2,0x3,0x4,0x5,0x6,0x7,0x8*,0x14,0x1c,0x30,0xf8,0x58d}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0078eba0: 16-byte aligned: YES
  0078eba0  55             PUSH EBP
  0078eba1  8B EC          MOV EBP,ESP
  0078eba3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078eba0
string literals (0):
direct callees (14):
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x0058C370  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00733A60  CExoString::operator==_2
  0x0071B8B0  (unnamed)
  0x0071DF90  (unnamed)
  0x007FDFA0  CSWCItem::GetName
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x8 [dec 8]  x5  via EAX,ECX
  +0x70 [dec 112]  x4  via EAX,EDX
  +0x4C [dec 76]  x3  via EAX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x2C [dec 44]  x2  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x34 [dec 52]  x1  via EDX
  +0x3C [dec 60]  x1  via EDX
  +0x10C [dec 268]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078eba0 FUN_0078eba0 size=1424 ret=8 callers=2 callees=14

void FUN_0078eba0(int param_1)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined1 *puVar4;
  int *local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined1 local_a0 [8];
  undefined4 local_98;
  undefined1 local_94 [8];
  undefined4 local_8c;
  undefined1 local_88 [8];
  undefined4 local_80;
  undefined1 local_7c [8];
  undefined1 local_74 [8];
  int local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined1 local_5c [28];
  int *local_40;
  undefined1 local_3c [28];
  uint local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009666e1;
  local_10 = ExceptionList;
  local_20 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(local_20);
  local_8 = 0;
  FUN_007107f0();
  local_8 = 1;
  local_ac = (int *)0x0;
  FUN_0053dfb0(param_1,&local_ac);
  local_40 = local_ac;
  if (local_ac == (int *)0x0) {
    local_1c = FUN_007a1720();
    while (local_1c != 0) {
      local_18 = FUN_00561430(local_1c);
      if ((local_18 != 0) && (*(int *)(local_18 + 0x2c) == param_1)) {
        FUN_007337c0(local_18 + 0x30);
        local_60 = 1;
        local_8 = 0xffffffff;
        FUN_00733780();
        goto LAB_0078f13a;
      }
      FUN_0058c370(&local_1c);
    }
    FUN_007338d0(&DAT_00986925);
    local_64 = 0;
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else if ((char)local_ac[2 = byte +0x8] == DAT_00994474) {
    iVar1 = (**(code **)(*local_ac + 0x2c))();
    FUN_007337c0(*(int *)(iVar1 + 0x310) + 0x14);
    local_68 = 1;
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else if ((char)local_ac[2 = byte +0x8] == DAT_00994478) {
    piVar2 = (int *)(**(code **)(*local_ac + 0x4c))();
    (**(code **)(*piVar2 + 0x70))(local_74);
    local_8._0_1_ = 2;
    local_6c = FUN_00733a60(&DAT_00986925);
    local_8._0_1_ = 1;
    FUN_00733780();
    if (local_6c == 0) {
      piVar2 = (int *)(**(code **)(*local_40 + 0x4c))();
/* ... 73 more lines truncated by --max-lines */
```

### decoy: 0x0078e350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e350: 16-byte aligned: YES
  0078e350  55             PUSH EBP
  0078e351  8B EC          MOV EBP,ESP
  0078e353  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078e350
string literals (0):
direct callees (4):
  0x0078BDF0  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 3):
  +0x98 [dec 152]  x5  via EAX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x10 [dec 16]  x3  via EAX,EDX
```

## Card B11a-5-48

GOG function: CClientExoAppInternal::AddSimpleObjectModelDirectlyToIgnoreList  gog 0x0043ab70  callconv=(blank)  param_size_bytes=8  gog_size=117  gog_retn=8
Candidate Steam address: 0x0078e5a0 (size 117, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0078e350:STR0/CALLEE0/SIG0.393/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043aaf0 CClientExoAppInternal::AddItemModelDirectlyToIgnoreList -> 0x0078e520  gate_class=REVIEW
- GOG-order neighbour after:  0x0043abf0 CClientExoAppInternal::StartMenuMusic -> 0x0078e7e0  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: yes
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0xc*,0x98*} | Steam {0x4*,0xc*,0x98*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xff*} | Steam {0x8*,0x14*,0xff*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0078e5a0: 16-byte aligned: YES
  0078e5a0  55             PUSH EBP
  0078e5a1  8B EC          MOV EBP,ESP
  0078e5a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078e5a0
string literals (0):
direct callees (2):
  0x0073F4D0  CClientExoApp::GetGameObject
  0x004491C0  CExoPtrList::Add (doubling)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x98 [dec 152]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078e5a0 FUN_0078e5a0 size=117 ret=8 callers=1 callees=2

void FUN_0078e5a0(undefined4 param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  
  uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  piVar1 = (int *)FUN_0073f4d0(param_1);
  if (((piVar1 != (int *)0x0) &&
      (piVar1 = (int *)(**(code **)(*piVar1 + 0xc))(extraout_var,uVar3), piVar1 != (int *)0x0)) &&
     (iVar2 = (**(code **)(*piVar1 + 0x98))(0xff,1), iVar2 != 0)) {
    FUN_004491c0(iVar2);
  }
  return;
}
```

### decoy: 0x0078e350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e350: 16-byte aligned: YES
  0078e350  55             PUSH EBP
  0078e351  8B EC          MOV EBP,ESP
  0078e353  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078e350
string literals (0):
direct callees (4):
  0x0078BDF0  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 3):
  +0x98 [dec 152]  x5  via EAX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x10 [dec 16]  x3  via EAX,EDX
```

## Card B11a-5-49

GOG function: CClientExoAppInternal::AddItemModelDirectlyToIgnoreList  gog 0x0043aaf0  callconv=(blank)  param_size_bytes=8  gog_size=115  gog_retn=8
Candidate Steam address: 0x0078e520 (size 115, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0078e350:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043a920 CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList_2 -> 0x0078e350  gate_class=REVIEW
- GOG-order neighbour after:  0x0043ab70 CClientExoAppInternal::AddSimpleObjectModelDirectlyToIgnoreList -> 0x0078e5a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00438360 CClientExoAppInternal::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x10*,0x98*} | Steam {0x10*,0x98*}
imm sets (* = in both, 3 common): GOG {0x8*,0x10*,0xfb*} | Steam {0x8*,0x10*,0xfb*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0078e520: 16-byte aligned: YES
  0078e520  55             PUSH EBP
  0078e521  8B EC          MOV EBP,ESP
  0078e523  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078e520
string literals (0):
direct callees (2):
  0x0078BD80  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
struct offsets touched (top 2):
  +0x10 [dec 16]  x1  via EDX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078e520 FUN_0078e520 size=115 ret=8 callers=1 callees=2

void FUN_0078e520(undefined4 param_1)

{
  int iVar1;
  int iVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined1 local_5;
  
  iVar1 = FUN_0078bd80(param_1);
  if (iVar1 != 0) {
    uVar3 = extraout_var;
    for (local_5 = 0xff; 0xfa < local_5; local_5 = local_5 - 1) {
      iVar2 = (**(code **)(*(int *)(iVar1 + 0x10) + 0x98))(local_5,1,uVar3);
      if (iVar2 != 0) {
        FUN_004491c0(iVar2);
      }
    }
  }
  return;
}
```

### decoy: 0x0078e350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e350: 16-byte aligned: YES
  0078e350  55             PUSH EBP
  0078e351  8B EC          MOV EBP,ESP
  0078e353  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078e350
string literals (0):
direct callees (4):
  0x0078BDF0  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 3):
  +0x98 [dec 152]  x5  via EAX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x10 [dec 16]  x3  via EAX,EDX
```

## Card B11a-5-50

GOG function: CClientExoAppInternal::GetDisplayCharacterForKey  gog 0x0044d980  callconv=(blank)  param_size_bytes=8  gog_size=115  gog_retn=8
Candidate Steam address: 0x007a1380 (size 115, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007a1340:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044d940 CClientExoAppInternal::EnableMouseAtEndOfLoadModuleMovie -> 0x007a1340  gate_class=REVIEW
- GOG-order neighbour after:  0x0044da00 CClientExoAppInternal::SetEvilEndingHackInProgress -> 0x007a1400  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00616aa0 CExoInput::GetDisplayCharacterForKey -> Steam ?  in candidate callees: n/a
  0x00627e40 CTlkTable::GetSimpleString -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0xbe54*} | Steam {0x8*,0xc*,0xbe54*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007a1380: 16-byte aligned: YES
  007a1380  55             PUSH EBP
  007a1381  8B EC          MOV EBP,ESP
  007a1383  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a1380
string literals (0):
direct callees (2):
  0x0071DEE0  (unnamed)
  0x0072FEA0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a1380 FUN_007a1380 size=115 ret=8 callers=1 callees=2

undefined4 FUN_007a1380(undefined4 param_1,int param_2)

{
  if ((param_2 < DAT_0099c894) || (DAT_0099c898 <= param_2)) {
    FUN_0071dee0(param_1,0xbe54);
  }
  else {
    FUN_0072fea0(param_1,param_2);
  }
  return param_1;
}
```

### decoy: 0x007a1340 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a1340: 16-byte aligned: YES
  007a1340  55             PUSH EBP
  007a1341  8B EC          MOV EBP,ESP
  007a1343  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a1340
string literals (0):
direct callees (2):
  0x00798C00  IsMoviePlaying
  0x007B3930  CClientExoAppInternal::ShowMouse
struct offsets touched (top 1):
  +0x200 [dec 512]  x2  via EAX,ECX
```

## Card B11a-5-51

GOG function: CClientExoAppInternal::IndicateObject  gog 0x00444b60  callconv=(blank)  param_size_bytes=0  gog_size=63  gog_retn=0
Candidate Steam address: 0x00798540 (size 63, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x00444a00:0.54>0.21
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007983e0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00444a00 CClientExoAppInternal::HideLoadScreen -> 0x00797d90  gate_class=REJECT
- GOG-order neighbour after:  0x00444ba0 CClientExoAppInternal::SetPlayerCharacterName -> 0x00798580  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x274*,0x278*} | Steam {0x274*,0x278*}
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
prologue for 0x00798540: 16-byte aligned: YES
  00798540  55             PUSH EBP
  00798541  8B EC          MOV EBP,ESP
  00798543  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00798540
string literals (0):
direct callees (1):
  0x00410800  GuiManager::Contains
struct offsets touched (top 2):
  +0x274 [dec 628]  x2  via ECX
  +0x278 [dec 632]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00798540 FUN_00798540 size=63 ret=0 callers=2 callees=1

int __fastcall FUN_00798540(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 0x278) == 0) || (*(int *)(param_1 + 0x274) == 0)) {
    iVar1 = 0;
  }
  else {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x274),
                       *(CSize **)(param_1 + 0x278));
  }
  return iVar1;
}
```

### decoy: 0x007983e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007983e0: 16-byte aligned: YES
  007983e0  55             PUSH EBP
  007983e1  8B EC          MOV EBP,ESP
  007983e3  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007983e0
string literals (0):
direct callees (12):
  0x007B3930  CClientExoAppInternal::ShowMouse
  0x007CBB40  CGuiInGame::StartGlobalFade
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00410670  (unnamed)
  0x008858C0  (unnamed)
  0x0041BCF0  (unnamed)
  0x00710840  (unnamed)
  0x0040F390  CSWGuiPanel::SetBackground
  0x00797D90  CClientExoAppInternal::SetInputActive
  0x00479250  (unnamed)
  0x00479220  (unnamed)
  0x00409ED0  (unnamed)
struct offsets touched (top 3):
  +0x278 [dec 632]  x5  via EAX,ECX
  +0x274 [dec 628]  x3  via ECX,EDX
  +0x40 [dec 64]  x1  via EDX
```

## Card B11a-5-52

GOG function: CClientExoAppInternal::GetPlayerMovementInput  gog 0x0040df80  callconv=(blank)  param_size_bytes=0  gog_size=124  gog_retn=0
Candidate Steam address: 0x00760f20 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00760ea0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040df60 CClientExoApp::GetPlayerMovementInput -> 0x00740aa0  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e000 CClientExoApp::SaveOptions -> 0x00740b40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0xc,0x2a0} | Steam {}
imm sets (* = in both, 0 common): GOG {0x8,0x44} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00760f20: 16-byte aligned: YES
  00760f20  55             PUSH EBP
  00760f21  8B EC          MOV EBP,ESP
  00760f23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00760f20
string literals (0):
direct callees (1):
  0x0041A510  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00760f20 FUN_00760f20 size=31 ret=0 callers=1 callees=1

undefined4 * __fastcall FUN_00760f20(undefined4 *param_1)

{
  FUN_0041a510();
  *param_1 = AspyrFloatingButtonIcon::vftable;
  return param_1;
}
```

### decoy: 0x00760ea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00760ea0: 16-byte aligned: YES
  00760ea0  55             PUSH EBP
  00760ea1  8B EC          MOV EBP,ESP
  00760ea3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00760ea0
string literals (0):
direct callees (4):
  0x007335F0  CExoString::CExoString_copy
  0x00760CF0  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B11a-5-53

GOG function: CClientExoAppInternal::GetPlayerCharacterName  gog 0x00444bd0  callconv=(blank)  param_size_bytes=0  gog_size=32  gog_retn=0
Candidate Steam address: 0x007985b0 (size 32, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00798580:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00444ba0 CClientExoAppInternal::SetPlayerCharacterName -> 0x00798580  gate_class=AUTO
- GOG-order neighbour after:  0x00444bf0 CClientExoAppInternal::IsPlayerCreature -> 0x007985d0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x290*} | Steam {0x290*}
imm sets (* = in both, 1 common): GOG {0x294*} | Steam {0x294*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007985b0: 16-byte aligned: YES
  007985b0  55             PUSH EBP
  007985b1  8B EC          MOV EBP,ESP
  007985b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007985b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x290 [dec 656]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007985b0 FUN_007985b0 size=32 ret=0 callers=1 callees=0

int __fastcall FUN_007985b0(int param_1)

{
  *(undefined4 *)(param_1 + 0x290) = 0;
  return param_1 + 0x294;
}
```

### decoy: 0x00798580 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00798580: 16-byte aligned: YES
  00798580  55             PUSH EBP
  00798581  8B EC          MOV EBP,ESP
  00798583  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00798580
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 1):
  +0x290 [dec 656]  x1  via ECX
```

## Card B11a-5-54

GOG function: CClientExoAppInternal::GetPausedByCombat  gog 0x0044deb0  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x007a1b90 (size 86, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a1ae0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: autolabel:~CMemoryDC
- GOG-order neighbour before: 0x0044de70 CClientExoAppInternal::IsMoviePlaying_2 -> 0x007a1900  gate_class=REVIEW
- GOG-order neighbour after:  0x0044ded0 CClientExoAppInternal::InitializeMoviePlayer_2 -> 0x007a1130  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0060c210 CExoLinkedListInternal::RemoveTail -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00774b70 CSWSCombatRound::RemoveLastAction -> Steam 0x005904c0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {0x10}

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

## Card B11a-5-55

GOG function: CClientExoAppInternal::GetNearestVideoMode  gog 0x0044df50  callconv=(blank)  param_size_bytes=4  gog_size=91  gog_retn=4
Candidate Steam address: 0x007a1bf0 (size 166, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.83
- best decoy (neighbour/other candidate) scores: 0x007a1ca0:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ded0 CClientExoAppInternal::InitializeMoviePlayer_2 -> 0x007a1130  gate_class=REVIEW
- GOG-order neighbour after:  0x0044dfb0 CClientExoAppInternal::HandleExitGameButton -> 0x007a1ca0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00733d10 CExoArrayList__FILETIME_::Allocate -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0x14}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007a1bf0: 16-byte aligned: YES
  007a1bf0  55             PUSH EBP
  007a1bf1  8B EC          MOV EBP,ESP
  007a1bf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a1bf0
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007a1bf0 FUN_007a1bf0 size=166 ret=4 callers=3 callees=2

void __thiscall FUN_007a1bf0(int *param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_c;
  
  iVar1 = *param_1;
  param_1[2 = byte +0x8] = param_2;
  iVar3 = FUN_00919723(-(uint)((int)((ulonglong)(uint)param_1[2 = byte +0x8] * 8 >> 0x20) != 0) |
                       (uint)((ulonglong)(uint)param_1[2 = byte +0x8] * 8));
  *param_1 = iVar3;
  for (local_c = 0; local_c < param_1[1 = byte +0x4]; local_c = local_c + 1) {
    uVar2 = *(undefined4 *)(iVar1 + 4 + local_c * 8);
    iVar3 = *param_1;
    *(undefined4 *)(iVar3 + local_c * 8) = *(undefined4 *)(iVar1 + local_c * 8);
    *(undefined4 *)(iVar3 + 4 + local_c * 8) = uVar2;
  }
  if (iVar1 != 0) {
    FUN_009196fd(iVar1);
  }
  return;
}
```

### decoy: 0x007a1ca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a1ca0: 16-byte aligned: YES
  007a1ca0  55             PUSH EBP
  007a1ca1  8B EC          MOV EBP,ESP
  007a1ca3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007a1ca0
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x0091D67E  (unnamed)
  0x007A1AE0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-5-56

GOG function: CClientExoAppInternal::StopMenuMusic  gog 0x0043adb0  callconv=(blank)  param_size_bytes=0  gog_size=130  gog_retn=0
Candidate Steam address: 0x0078e8f0 (size 352, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.40) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.24 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078ea50:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043abf0 CClientExoAppInternal::StartMenuMusic -> 0x0078e7e0  gate_class=REJECT
- GOG-order neighbour after:  0x0043ae40 CClientExoAppInternal::FadeAndStopStreamingSoundSource -> 0x0078eba0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0043ae40 CClientExoAppInternal::FadeAndStopStreamingSoundSource -> Steam ?  in candidate callees: n/a
  0x0063a020 CExoStreamingSoundSource::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x238*} | Steam {0x238*,0x23c,0x2dc}
imm sets (* = in both, 1 common): GOG {0x4*,0x10,0x1f4} | Steam {0x2,0x4*,0x8,0x20,0x304}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078e8f0: 16-byte aligned: YES
  0078e8f0  55             PUSH EBP
  0078e8f1  8B EC          MOV EBP,ESP
  0078e8f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078e8f0
string literals (0):
direct callees (11):
  0x0078E7E0  CClientExoAppInternal::StartMenuMusic
  0x00733B60  (unnamed)
  0x00919723  operator new
  0x0070AFD0  (unnamed)
  0x0070B240  (unnamed)
  0x0070B350  (unnamed)
  0x00710810  (unnamed)
  0x0070B170  (unnamed)
  0x0070BC60  (unnamed)
  0x0070B0E0  (unnamed)
  0x0070B1D0  (unnamed)
struct offsets touched (top 3):
  +0x23C [dec 572]  x7  via EAX,ECX,EDX
  +0x238 [dec 568]  x1  via EAX
  +0x2DC [dec 732]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0078e8f0 FUN_0078e8f0 size=352 ret=0 callers=2 callees=11

void __fastcall FUN_0078e8f0(int param_1)

{
  int iVar1;
  undefined4 local_30;
  undefined1 local_28 [16];
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095e3ab;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x238) != 0) {
    FUN_0078e7e0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  }
  if ((DAT_00a1b494 != 0) && (*(int *)(param_1 + 0x2dc) == 0)) {
    if (*(int *)(param_1 + 0x23c) == 0) {
      iVar1 = FUN_00733b60(&DAT_00986925);
      if (iVar1 == 0) {
        FUN_0070bc60(DAT_0099c2b4,0);
      }
      else {
        local_18 = FUN_00919723(4);
        local_8 = 0;
        if (local_18 == 0) {
          local_30 = 0;
        }
        else {
          local_30 = FUN_0070afd0(1);
        }
        local_14 = local_30;
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x23c) = local_30;
        FUN_0070b240(1);
        FUN_0070b350(2);
        FUN_00710810(param_1 + 0x304);
        FUN_0070b170(local_28,8,0,1,0);
      }
    }
    else {
      iVar1 = FUN_0070b0e0();
      if (iVar1 == 0) {
        FUN_0070b1d0();
      }
    }
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0078ea50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078ea50: 16-byte aligned: YES
  0078ea50  55             PUSH EBP
  0078ea51  8B EC          MOV EBP,ESP
  0078ea53  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078ea50
string literals (0):
direct callees (3):
  0x0070B2F0  (unnamed)
  0x0070B080  StreamingSource scalar delete
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x23C [dec 572]  x4  via EAX,ECX,EDX
```

## Card B11a-5-57

GOG function: CClientExoAppInternal::AddFeedbackMessage  gog 0x0043b710  callconv=(blank)  param_size_bytes=12  gog_size=30173  gog_retn=12
Candidate Steam address: 0x0078f160 (size 30173, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.60) | G8 mac_rotation: pass
- anchors: A-STR 5 | A-CALLEE 2 | A-SIG 0.05 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE  (content level C2)
- evidence summary: STR '%s %s'@0098b99c; '(%d > %d) Scripts: (%s), Action: %d [Fla'@0099f788; '(%d > %d) OID: %08x, Tag: %s'@0099f7d0; CALLEE 2 (callees: SetFeedbackInfo 0x7f2200, AppendToMsgBuffer 0x7be090)
- best decoy (neighbour/other candidate) scores: 0x0078eba0:STR0/CALLEE0/SIG0.153/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043b150 CClientExoAppInternal::GetObjectName -> 0x0078e350  gate_class=REVIEW
- GOG-order neighbour after:  0x00443100 CSWTlkTable::GetObjectGender  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (33), mapped to Steam through trusted rows only:
  0x0040ca90 CClientExoApp::GetPlaceableByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0040cab0 CClientExoApp::GetDoorByGameObjectID -> Steam ?  in candidate callees: n/a
  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> Steam ?  in candidate callees: n/a
  0x0043b150 CClientExoAppInternal::GetObjectName -> Steam ?  in candidate callees: n/a
  0x00443100 CSWTlkTable::GetObjectGender -> Steam ?  in candidate callees: n/a
  0x0044a810 CClientExoAppInternal::GetGUIString -> Steam ?  in candidate callees: n/a
  0x00477360 CSWCObject::SetFeedbackInfo -> Steam 0x007f2200  in candidate callees: yes
  0x004ce720 CGuiInGame::AppendToMsgBuffer -> Steam 0x007be090  in candidate callees: yes
  ... 25 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (5): '(%d > %d) Scripts: (%s), Action: %d [Fla'; '(%d > %d) OID: %08x, Tag: %s'; 'OID: %08x, Tag: %s, %s'; '%s %s' ...
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x28,0x2c,0x30,0x40*,0x74,0x104,0x10c,0x128,0x129,0x130} | Steam {0x40*}
imm sets (* = in both, 6 common): GOG {0x2,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc*,0xd,0xe,0xf,...(+375)} | Steam {0x4*,0xc*,0x80*,0xeb*,0x138*,0x1390*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0078f160: 16-byte aligned: YES
  0078f160  55             PUSH EBP
  0078f161  8B EC          MOV EBP,ESP
  0078f163  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078f160
string literals (7):
  0x0099F808  at 0079008d  '%s %i'
  0x0099F808  at 007900fc  '%s %i'
  0x0098B99C  at 00790177  '%s %s'
  0x0099F7F0  at 00795c4b  'OID: %08x, Tag: %s, %s'
  0x0099F7D0  at 00795ddf  '(%d > %d) OID: %08x, Tag: %s'
  0x0099F7D0  at 00795fbe  '(%d > %d) OID: %08x, Tag: %s'
  0x0099F788  at 00796254  '(%d > %d) Scripts: (%s), Action: %d [Flags: %02x], OID: %08x, Tag: %s'
direct callees (33):
  0x0091E050  (unnamed)
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x0078CEE0  (unnamed)
  0x0079E210  CClientExoAppInternal::GetGUIString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x005E1BA0  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x0071E910  (unnamed)
  0x00619C90  (unnamed)
  0x005E1C40  (unnamed)
  0x0078EBA0  CClientExoAppInternal::FadeAndStopStreamingSoundSource
  0x005E2690  (unnamed)
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x0073F5F0  CClientExoApp::GetDoorByGameObjectID
  0x0073F5D0  CClientExoApp::GetPlaceableByGameObjectID
  0x0071B8B0  (unnamed)
  0x0071AFA0  C2DA::GetCExoStringEntry
struct offsets touched (top 10):
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x130 [dec 304]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x74 [dec 116]  x2  via EAX,ECX
  +0x104 [dec 260]  x2  via EAX,EDX
  +0x28 [dec 40]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x40 [dec 64]  x1  via EAX
  +0x10C [dec 268]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0078f160 FUN_0078f160 size=30173 ret=12 callers=1 callees=33

/* WARNING: Function: __alloca_probe replaced with injection: alloca_probe */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0078f160(int param_1,short param_2,int param_3)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  float10 fVar5;
  size_t local_b00;
  size_t local_aec;
  undefined1 local_abc [8];
  undefined1 local_ab4 [8];
  undefined1 local_aac [8];
  undefined1 local_aa4 [8];
  undefined1 local_a9c [8];
  undefined1 local_a94 [8];
  undefined1 local_a8c [8];
  undefined1 local_a84 [8];
  undefined1 local_a7c [8];
  undefined1 local_a74 [8];
  undefined1 local_a6c [8];
  undefined1 local_a64 [8];
  undefined1 local_a5c [8];
  undefined1 local_a54 [8];
  undefined1 local_a4c [8];
  undefined1 local_a44 [8];
  undefined1 local_a3c [8];
  undefined1 local_a34 [8];
  undefined1 local_a2c [8];
  undefined1 local_a24 [8];
  undefined1 local_a1c [8];
  undefined1 local_a14 [8];
  undefined1 local_a0c [8];
  undefined1 local_a04 [8];
  undefined1 local_9fc [8];
  undefined1 local_9f4 [8];
  undefined1 local_9ec [8];
  undefined1 local_9e4 [8];
  undefined1 local_9dc [8];
  undefined1 local_9d4 [8];
  undefined1 local_9cc [8];
  undefined1 local_9c4 [8];
  undefined1 local_9bc [8];
  undefined1 local_9b4 [8];
  undefined1 local_9ac [8];
  undefined1 local_9a4 [8];
  undefined1 local_99c [8];
  undefined1 local_994 [8];
  undefined1 local_98c [8];
  undefined1 local_984 [8];
  undefined1 local_97c [8];
  undefined1 local_974 [8];
  undefined1 local_96c [8];
  undefined1 local_964 [8];
  undefined1 local_95c [8];
  undefined1 local_954 [8];
  undefined1 local_94c [8];
  undefined1 local_944 [8];
  undefined1 local_93c [8];
  undefined1 local_934 [8];
  undefined1 local_92c [8];
  undefined1 local_924 [8];
  undefined1 local_91c [8];
  undefined1 local_914 [8];
  undefined1 local_90c [8];
  undefined1 local_904 [8];
  undefined1 local_8fc [8];
  undefined1 local_8f4 [8];
  undefined1 local_8ec [8];
  undefined1 local_8e4 [8];
  undefined1 local_8dc [8];
  undefined1 local_8d4 [8];
  undefined1 local_8cc [8];
  undefined1 local_8c4 [8];
  undefined1 local_8bc [8];
  undefined1 local_8b4 [8];
/* ... 2899 more lines truncated by --max-lines */
```

### decoy: 0x0078eba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078eba0: 16-byte aligned: YES
  0078eba0  55             PUSH EBP
  0078eba1  8B EC          MOV EBP,ESP
  0078eba3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078eba0
string literals (0):
direct callees (14):
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x0058C370  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00733A60  CExoString::operator==_2
  0x0071B8B0  (unnamed)
  0x0071DF90  (unnamed)
  0x007FDFA0  CSWCItem::GetName
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x8 [dec 8]  x5  via EAX,ECX
  +0x70 [dec 112]  x4  via EAX,EDX
  +0x4C [dec 76]  x3  via EAX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x2C [dec 44]  x2  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x34 [dec 52]  x1  via EDX
  +0x3C [dec 60]  x1  via EDX
  +0x10C [dec 268]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
```

## Card B11a-5-58

GOG function: CClientExoAppInternal::InitializeMoviePlayer  gog 0x00445250  callconv=(blank)  param_size_bytes=0  gog_size=155  gog_retn=0
Candidate Steam address: 0x00798c30 (size 155, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00798cd0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00445220 CClientExoAppInternal::IsMoviePlaying -> 0x00798c00  gate_class=REJECT
- GOG-order neighbour after:  0x004452f0 CClientExoAppInternal::StopMovie -> 0x00798cd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x006a2c90 CSWCObject::LipSync -> Steam ?  in candidate callees: n/a
  0x009210c0 CExoMoviePlayer::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x130*} | Steam {0x130*}
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
