# Review cards B11b chunk 1

Answer each card with one TSV row in `steam_db_batches/B11b/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B11b-1-01

GOG function: CSWGuiMainInterface::ClearOneAction  gog 0x00522930  callconv=(blank)  param_size_bytes=0  gog_size=119  gog_retn=0
Candidate Steam address: 0x00750e60 (size 43, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00750c10:STR0/CALLEE0/SIG0.045/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00522830 CSWGuiLabel::`scalar  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00522d00 CSWGuiMainInterface::OnMenuButtonPressed -> 0x00751580  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callees: no
  0x00774b70 CSWSCombatRound::RemoveLastAction -> Steam 0x005904c0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x10dc} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {0x18} | Steam {0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00750e60: 16-byte aligned: YES
  00750e60  55             PUSH EBP
  00750e61  8B EC          MOV EBP,ESP
  00750e63  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00750e60
string literals (0):
direct callees (1):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00750e60 FUN_00750e60 size=43 ret=0 callers=1 callees=1
THUNK -> 0x0073FB90 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00750e60(undefined4 param_1)

{
  FUN_0073fb90(param_1);
  return;
}
```

### decoy: 0x00750c10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00750c10: 16-byte aligned: YES
  00750c10  55             PUSH EBP
  00750c11  8B EC          MOV EBP,ESP
  00750c13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00750c10
string literals (1):
  0x00993220  at 00750c88  'lbl_map%s'
direct callees (10):
  0x0051BD90  CServerExoApp::GetModule
  0x00733540  CExoString::Constructor
  0x00710930  (unnamed)
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00710810  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
  0x0091C860  _ftol2_sse
  0x0072FB00  CClientExoApp::GetClientOptions
  0x00733780  CExoString::~CExoString
struct offsets touched (top 10):
  +0x48 [dec 72]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x238 [dec 568]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
  +0x5AB4 [dec 23220]  x1  via EAX
  +0x5D54 [dec 23892]  x1  via EDX
  +0x5D58 [dec 23896]  x1  via ECX
```

## Card B11b-1-02

GOG function: CGuiInGame::ShowDebugMenu  gog 0x004db100  callconv=(blank)  param_size_bytes=0  gog_size=15  gog_retn=0
Candidate Steam address: 0x007cac10 (size 147, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::HideExamineBox | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cab60:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004db020 CGuiInGame::HideGalaxyMapGui -> 0x007ca920  gate_class=REVIEW
- GOG-order neighbour after:  0x004db110 CGuiInGame::ShowItemCreateMenu -> 0x007caa10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0063ac00 CExoSound::SetSoundMode -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x38,0x50,0x128,0x9c8}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007cac10: 16-byte aligned: YES
  007cac10  55             PUSH EBP
  007cac11  8B EC          MOV EBP,ESP
  007cac13  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cac10
string literals (0):
direct callees (4):
  0x00410800  GuiManager::Contains
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00410670  (unnamed)
  0x0073FEE0  CClientExoApp::SetInputClass
struct offsets touched (top 5):
  +0x38 [dec 56]  x3  via EAX,ECX
  +0x50 [dec 80]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x128 [dec 296]  x1  via EAX
  +0x9C8 [dec 2504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007cac10 FUN_007cac10 size=147 ret=0 callers=1 callees=4

void __fastcall FUN_007cac10(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x50));
    if (iVar1 != 0) {
      FUN_004109c0();
      FUN_00410670(*(undefined4 *)(param_1 + 0x50));
      if (*(int *)(*(int *)(param_1 + 0x50) + 0x9c8) == 1) {
        FUN_0073fee0(2,1);
      }
      else {
        FUN_0073fee0(0,1);
      }
    }
  }
  return;
}
```

### decoy: 0x007cab60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cab60: 16-byte aligned: YES
  007cab60  55             PUSH EBP
  007cab61  8B EC          MOV EBP,ESP
  007cab63  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cab60
string literals (0):
direct callees (6):
  0x00410800  GuiManager::Contains
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00410670  (unnamed)
  0x007400A0  CClientExoApp::GetMiniGame
  0x0073FEE0  CClientExoApp::SetInputClass
  0x008BF9F0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x38 [dec 56]  x3  via EAX,ECX
  +0x5C [dec 92]  x3  via ECX,EDX
  +0x128 [dec 296]  x1  via EAX
```

## Card B11b-1-03

GOG function: CGuiInGame::IsFadingInDialog  gog 0x004dbe50  callconv=(blank)  param_size_bytes=0  gog_size=43  gog_retn=0
Candidate Steam address: 0x007cb720 (size 44, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb750:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbe20 CGuiInGame::IsLoopingDialogStuntAnimation_2 -> 0x007cb5e0  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbe80 CGuiInGame::IsInDialog -> 0x007cb780  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 7): the card's A-CALLEE credit may come from these, not from callees
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
  0x008BB200  CSWGuiDialogLetterbox::IsFadeDone
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

## Card B11b-1-04

GOG function: CGuiInGame::IsAnimationPlayingInDialog  gog 0x004dbde0  callconv=(blank)  param_size_bytes=0  gog_size=55  gog_retn=0
Candidate Steam address: 0x007cb6e0 (size 55, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: Draw 0x8bb260); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007cb750:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbda0 CGuiInGame::IsCameraDialog -> 0x007cb6a0  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbe20 CGuiInGame::IsLoopingDialogStuntAnimation_2 -> 0x007cb5e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x005af9b0 CSWGuiDialogLetterbox::Draw -> Steam 0x008bb260  in candidate callers: yes
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

## Card B11b-1-05

GOG function: CGuiInGame::HideChemicalUpgradeScreen  gog 0x004de3c0  callconv=(blank)  param_size_bytes=0  gog_size=217  gog_retn=0
Candidate Steam address: 0x007cdcc0 (size 217, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cdbe0:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004de2e0 CGuiInGame::HideUpgradeScreen -> 0x007cdbe0  gate_class=REVIEW
- GOG-order neighbour after:  0x004de4a0 CGuiInGame::ShowSwoopUpgradeScreen -> 0x007cdda0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040ccd0 CClientExoApp::GetSWCMessage -> Steam 0x0073f810  in candidate callees: yes
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x00506010 CSWCMessage::SendPlayerToServerInput_TogglePauseRequest -> Steam ?  in candidate callees: n/a
  0x0063ac00 CExoSound::SetSoundMode -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x38*,0x48*,0x8c*,0x128*,0xf18*} | Steam {0x4*,0x38*,0x48*,0x8c*,0x128*,0xf18*}
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
prologue for 0x007cdcc0: 16-byte aligned: YES
  007cdcc0  55             PUSH EBP
  007cdcc1  8B EC          MOV EBP,ESP
  007cdcc3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007cdcc0 FUN_007cdcc0 size=217 ret=0 callers=1 callees=4

void __fastcall FUN_007cdcc0(int param_1)

{
  int iVar1;
  
  if (((*(int *)(param_1 + 0x128) != 0) && (*(int *)(param_1 + 0x38) != 0)) &&
     (*(int *)(param_1 + 0x8c) != 0)) {
    if (*(int *)(param_1 + 0xf18) == 0) {
      FUN_0073f810();
      FUN_0087a730();
    }
    FUN_0073fee0(0,1);
    FUN_0070bc60(DAT_0099c2a8,0);
    iVar1 = *(int *)(param_1 + 0x8c);
    *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xffffff7f;
    *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xfffffcff | 0x200;
    *(undefined4 *)(param_1 + 0x8c) = 0;
  }
  return;
}
```

### decoy: 0x007cdbe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cdbe0: 16-byte aligned: YES
  007cdbe0  55             PUSH EBP
  007cdbe1  8B EC          MOV EBP,ESP
  007cdbe3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11b-1-06

GOG function: CGuiInGame::IsLoopingDialogStuntAnimation  gog 0x004db8f0  callconv=(blank)  param_size_bytes=2  gog_size=45  gog_retn=4
Candidate Steam address: 0x007cb1f0 (size 45, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007cb1c0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004db8c0 CGuiInGame::IsDialogAnimationRange1000_1727 -> 0x007cb1c0  gate_class=REVIEW
- GOG-order neighbour after:  0x004db920 CGuiInGame::IsLoopingDialogAnimation -> 0x007cb220  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x578*,0x6bf*} | Steam {0x4*,0x578*,0x6bf*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cb1f0: 16-byte aligned: YES
  007cb1f0  55             PUSH EBP
  007cb1f1  8B EC          MOV EBP,ESP
  007cb1f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb1f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007cb1f0 FUN_007cb1f0 size=45 ret=4 callers=2 callees=0

undefined4 FUN_007cb1f0(ushort param_1)

{
  undefined4 uVar1;
  
  if ((param_1 < 0x578) || (0x6bf < param_1)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x007cb1c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb1c0: 16-byte aligned: YES
  007cb1c0  55             PUSH EBP
  007cb1c1  8B EC          MOV EBP,ESP
  007cb1c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb1c0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B11b-1-07

GOG function: CGuiInGame::ShowFlashingStatus  gog 0x004dfbb0  callconv=(blank)  param_size_bytes=1  gog_size=44  gog_retn=4
Candidate Steam address: 0x007cf4b0 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cf470:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dfb70 CGuiInGame::SuppressStatusSummary -> 0x007cf470  gate_class=AUTO
- GOG-order neighbour after:  0x004dfbe0 CGuiInGame::CanPlayDialogAnimations -> 0x007cf4e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00522610 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x98*} | Steam {0x98*}
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
prologue for 0x007cf4b0: 16-byte aligned: YES
  007cf4b0  55             PUSH EBP
  007cf4b1  8B EC          MOV EBP,ESP
  007cf4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cf4b0
string literals (0):
direct callees (1):
  0x00750E90  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007cf4b0 FUN_007cf4b0 size=44 ret=4 callers=2 callees=1

void __thiscall FUN_007cf4b0(int param_1,undefined1 param_2)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    FUN_00750e90(param_2);
  }
  return;
}
```

### decoy: 0x007cf470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cf470: 16-byte aligned: YES
  007cf470  55             PUSH EBP
  007cf471  8B EC          MOV EBP,ESP
  007cf473  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cf470
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xB8 [dec 184]  x2  via EAX,ECX
  +0x7C [dec 124]  x1  via EAX
```

## Card B11b-1-08

GOG function: CGuiInGame::ShowSubItemCreateMenu_2  gog 0x004dba50  callconv=(blank)  param_size_bytes=0  gog_size=159  gog_retn=0
Candidate Steam address: 0x007cb350 (size 159, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb3f0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004db950 CGuiInGame::IsOverlayDialogAnimation -> 0x007cb250  gate_class=AUTO
- GOG-order neighbour after:  0x004dbaf0 CGuiInGame::HideSubItemCreateMenu_2 -> 0x007cb3f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004d6bc0 CGuiInGame::CleanUpReplyNode -> Steam ?  in candidate callees: n/a
  0x004dbe50 CGuiInGame::IsFadingInDialog -> Steam ?  in candidate callees: n/a
  0x005af720 CSWGuiDialogLetterbox::ResetFade -> Steam 0x008bafd0  in candidate callees: yes
  0x006165b0 CExoInput::CoolDownEvent -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x68*,0xfc*,0x10c*,0xf0c*} | Steam {0x4*,0x8*,0x68*,0xfc*,0x10c*,0xf0c*}
imm sets (* = in both, 5 common): GOG {0x10*,0x43*,0xb5*,0x100*,0x2ee*} | Steam {0x10*,0x43*,0xb5*,0x100*,0x2ee*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007cb350: 16-byte aligned: YES
  007cb350  55             PUSH EBP
  007cb351  8B EC          MOV EBP,ESP
  007cb353  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb350
string literals (0):
direct callees (4):
  0x008BAFD0  CSWGuiDialogLetterbox::ResetFade
  0x007CB750  CGuiInGame::IsCameraDialog
  0x007C6530  CGuiInGame::CleanUpReplyNode
  0x0072F990  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x68 [dec 104]  x1  via ECX
  +0xFC [dec 252]  x1  via EAX
  +0x10C [dec 268]  x1  via EAX
  +0xF0C [dec 3852]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007cb350 FUN_007cb350 size=159 ret=0 callers=1 callees=4

void __fastcall FUN_007cb350(int param_1)

{
  int iVar1;
  
  *(undefined4 *)(param_1 + 0xfc) = 0;
  *(undefined4 *)(param_1 + 0x100) = 0;
  *(undefined4 *)(param_1 + 0x104) = 0;
  *(undefined4 *)(param_1 + 0x108) = 0;
  *(undefined4 *)(param_1 + 0x10c) = 0;
  FUN_008bafd0();
  iVar1 = FUN_007cb750();
  if (iVar1 == 0) {
    FUN_007c6530();
  }
  *(undefined4 *)(param_1 + 0xf0c) = 0;
  FUN_0072f990(0x43,0x2ee);
  FUN_0072f990(0xb5,0x2ee);
  return;
}
```

### decoy: 0x007cb3f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb3f0: 16-byte aligned: YES
  007cb3f0  55             PUSH EBP
  007cb3f1  8B EC          MOV EBP,ESP
  007cb3f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb3f0
string literals (0):
direct callees (5):
  0x008BAFD0  CSWGuiDialogLetterbox::ResetFade
  0x007C0290  CGuiInGame::ClearRepliesData
  0x007CB750  CGuiInGame::IsCameraDialog
  0x007C6530  CGuiInGame::CleanUpReplyNode
  0x0072F990  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x68 [dec 104]  x1  via ECX
  +0xFC [dec 252]  x1  via EAX
  +0x10C [dec 268]  x1  via EAX
  +0xF0C [dec 3852]  x1  via EDX
```

## Card B11b-1-09

GOG function: CGuiInGame::HideSubItemCreateMenu_2  gog 0x004dbaf0  callconv=(blank)  param_size_bytes=0  gog_size=167  gog_retn=0
Candidate Steam address: 0x007cb3f0 (size 167, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb350:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dba50 CGuiInGame::ShowSubItemCreateMenu_2 -> 0x007cb350  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbba0 CGuiInGame::HideLoadModuleDebugMenu -> 0x007cb4a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004d0920 CGuiInGame::ClearRepliesData -> Steam ?  in candidate callees: n/a
  0x004d6bc0 CGuiInGame::CleanUpReplyNode -> Steam ?  in candidate callees: n/a
  0x004dbe50 CGuiInGame::IsFadingInDialog -> Steam ?  in candidate callees: n/a
  0x005af720 CSWGuiDialogLetterbox::ResetFade -> Steam 0x008bafd0  in candidate callees: yes
  0x006165b0 CExoInput::CoolDownEvent -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x68*,0xfc*,0x10c*,0xf0c*} | Steam {0x4*,0x8*,0x68*,0xfc*,0x10c*,0xf0c*}
imm sets (* = in both, 5 common): GOG {0x10*,0x43*,0xb5*,0x100*,0x2ee*} | Steam {0x10*,0x43*,0xb5*,0x100*,0x2ee*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007cb3f0: 16-byte aligned: YES
  007cb3f0  55             PUSH EBP
  007cb3f1  8B EC          MOV EBP,ESP
  007cb3f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb3f0
string literals (0):
direct callees (5):
  0x008BAFD0  CSWGuiDialogLetterbox::ResetFade
  0x007C0290  CGuiInGame::ClearRepliesData
  0x007CB750  CGuiInGame::IsCameraDialog
  0x007C6530  CGuiInGame::CleanUpReplyNode
  0x0072F990  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x68 [dec 104]  x1  via ECX
  +0xFC [dec 252]  x1  via EAX
  +0x10C [dec 268]  x1  via EAX
  +0xF0C [dec 3852]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007cb3f0 FUN_007cb3f0 size=167 ret=0 callers=1 callees=5

void __fastcall FUN_007cb3f0(int param_1)

{
  int iVar1;
  
  *(undefined4 *)(param_1 + 0xfc) = 0;
  *(undefined4 *)(param_1 + 0x100) = 0;
  *(undefined4 *)(param_1 + 0x104) = 0;
  *(undefined4 *)(param_1 + 0x108) = 0;
  *(undefined4 *)(param_1 + 0x10c) = 0;
  FUN_008bafd0();
  FUN_007c0290();
  iVar1 = FUN_007cb750();
  if (iVar1 == 0) {
    FUN_007c6530();
  }
  *(undefined4 *)(param_1 + 0xf0c) = 0;
  FUN_0072f990(0x43,0x2ee);
  FUN_0072f990(0xb5,0x2ee);
  return;
}
```

### decoy: 0x007cb350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb350: 16-byte aligned: YES
  007cb350  55             PUSH EBP
  007cb351  8B EC          MOV EBP,ESP
  007cb353  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb350
string literals (0):
direct callees (4):
  0x008BAFD0  CSWGuiDialogLetterbox::ResetFade
  0x007CB750  CGuiInGame::IsCameraDialog
  0x007C6530  CGuiInGame::CleanUpReplyNode
  0x0072F990  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x68 [dec 104]  x1  via ECX
  +0xFC [dec 252]  x1  via EAX
  +0x10C [dec 268]  x1  via EAX
  +0xF0C [dec 3852]  x1  via EDX
```

## Card B11b-1-10

GOG function: CGuiInGame::ResetComputerCameraShot  gog 0x004d6c40  callconv=(blank)  param_size_bytes=0  gog_size=182  gog_retn=0
Candidate Steam address: 0x007c65b0 (size 182, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x004d6bc0:0.75>0.16
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007c6670:STR0/CALLEE0/SIG0.203/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004d6bc0 CGuiInGame::CleanUpReplyNode -> 0x007c6530  gate_class=AUTO
- GOG-order neighbour after:  0x004d6d00 CGuiInGame::ShowDialogReplies -> 0x007c6670  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040c8f0 CClientExoApp::GetModule -> Steam 0x0073f430  in candidate callees: yes
  0x0046e440 CSWCModule::RestoreCamera -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 12 common): GOG {0x4*,0x184*,0x190*,0x194*,0x198*,0x19c*,0x1a4*,0xf00*,0xf04*,0xf08*,0xf0c*,0xf14*} | Steam {0x4*,0x184*,0x190*,0x194*,0x198*,0x19c*,0x1a4*,0xf00*,0xf04*,0xf08*,0xf0c*,0xf14*}
imm sets (* = in both, 3 common): GOG {0x8*,0x2772*,0x7f000000*} | Steam {0x8*,0x2772*,0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007c65b0: 16-byte aligned: YES
  007c65b0  55             PUSH EBP
  007c65b1  8B EC          MOV EBP,ESP
  007c65b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007c65b0
string literals (0):
direct callees (2):
  0x0073F430  CClientExoApp::GetModule
  0x007F7220  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via EAX
  +0x184 [dec 388]  x1  via ECX
  +0x190 [dec 400]  x1  via ECX
  +0x194 [dec 404]  x1  via EDX
  +0x198 [dec 408]  x1  via EAX
  +0x19C [dec 412]  x1  via ECX
  +0x1A4 [dec 420]  x1  via EAX
  +0xF00 [dec 3840]  x1  via ECX
  +0xF04 [dec 3844]  x1  via EDX
  +0xF08 [dec 3848]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007c65b0 FUN_007c65b0 size=182 ret=0 callers=1 callees=2

void __fastcall FUN_007c65b0(int param_1)

{
  FUN_0073f430(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  FUN_007f7220();
  *(undefined4 *)(param_1 + 0xf04) = 0;
  *(undefined4 *)(param_1 + 0xf0c) = 0;
  *(undefined4 *)(param_1 + 0xf00) = 0;
  *(undefined4 *)(param_1 + 0xf08) = 0;
  *(undefined2 *)(param_1 + 0x184) = 0x2772;
  *(undefined4 *)(param_1 + 0xf14) = 0;
  *(undefined4 *)(param_1 + 0x1a4) = 0x7f000000;
  *(undefined4 *)(param_1 + 0x190 [dec 400]) = 0x7f000000;
  *(undefined4 *)(param_1 + 0x194) = 0x7f000000;
  *(undefined4 *)(param_1 + 0x198) = 0x7f000000;
  *(undefined4 *)(param_1 + 0x19c) = 0x7f000000;
  return;
}
```

### decoy: 0x007c6670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007c6670: 16-byte aligned: YES
  007c6670  55             PUSH EBP
  007c6671  8B EC          MOV EBP,ESP
  007c6673  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007c6670
string literals (0):
direct callees (15):
  0x00733B60  (unnamed)
  0x007B3930  CClientExoAppInternal::ShowMouse
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x00733A60  CExoString::operator==_2
  0x007CD4D0  CGuiInGame::SetDialogSelection
  0x007CD0F0  CGuiInGame::UpdateDialog
  0x007CB5E0  CGuiInGame::UnlockDialogParticipantOrientations
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x007CB750  CGuiInGame::IsCameraDialog
  0x007C3EE0  CGuiInGame::InitializeShotCamera
  0x007C41F0  CGuiInGame::UpdateShotCamera
struct offsets touched (top 10):
  +0x3C [dec 60]  x11  via EAX,ECX,EDX
  +0x190 [dec 400]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x138 [dec 312]  x6  via ECX,EDX
  +0x134 [dec 308]  x5  via EAX,ECX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x198 [dec 408]  x3  via EAX,EDX
  +0x68 [dec 104]  x2  via EAX
  +0x78 [dec 120]  x2  via EDX
  +0x13C [dec 316]  x2  via EAX,EDX
```

## Card B11b-1-11

GOG function: CSWGuiMainInterface::AddFloatyText  gog 0x00524620  callconv=(blank)  param_size_bytes=0  gog_size=170  gog_retn=0
Candidate Steam address: 0x00752e10 (size 139, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.17) | G8 mac_rotation: rotation:0x00524a20:0.62>0.26
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.58 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00752ea0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00524590 CSWGuiMainInterface::ShowFlashingStatus -> 0x00752e10  gate_class=REVIEW
- GOG-order neighbour after:  0x005246d0 CSWGuiMainInterface::SetupPauseGuiExtent -> 0x00752f50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0044cd50 CClientExoAppInternal::SetToolTipInstantMode -> Steam 0x007a0750  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x48*,0x6f9c,0x732c*,0xa904} | Steam {0x48*,0x732c*}
imm sets (* = in both, 4 common): GOG {0x14,0x6f54*,0x709c*,0x71e4*,0xfffffffd*} | Steam {0x10,0x6f54*,0x709c*,0x71e4*,0xfffffffd*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00752e10: 16-byte aligned: YES
  00752e10  55             PUSH EBP
  00752e11  8B EC          MOV EBP,ESP
  00752e13  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00752e10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x48 [dec 72]  x6  via EAX,ECX,EDX
  +0x732C [dec 29484]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00752e10 FUN_00752e10 size=139 ret=0 callers=1 callees=0

void __fastcall FUN_00752e10(int param_1)

{
  *(uint *)(param_1 + 0x6f9c) = *(uint *)(param_1 + 0x6f9c) & 0xfffffffd;
  *(uint *)(param_1 + 0x722c) = *(uint *)(param_1 + 0x722c) & 0xfffffffd | 2;
  *(uint *)(param_1 + 0x70e4) = *(uint *)(param_1 + 0x70e4) & 0xfffffffd | 2;
  *(undefined4 *)(param_1 + 0x732c) = 1;
  return;
}
```

### decoy: 0x00752ea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752ea0: 16-byte aligned: YES
  00752ea0  55             PUSH EBP
  00752ea1  8B EC          MOV EBP,ESP
  00752ea3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752ea0
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x48 [dec 72]  x6  via EAX,ECX,EDX
  +0x6F9C [dec 28572]  x1  via EAX
  +0x732C [dec 29484]  x1  via ECX
  +0xA904 [dec 43268]  x1  via EDX
```

## Card B11b-1-12

GOG function: CSWGuiMainInterface::OnPauseButtonPressed  gog 0x00523670  callconv=(blank)  param_size_bytes=4  gog_size=172  gog_retn=4
Candidate Steam address: 0x00751e40 (size 167, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.25) | G8 mac_rotation: rotation:0x00523760:0.89>0.62
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.42 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00751ef0:STR0/CALLEE5/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005235c0 CSWGuiMainInterface::OnClearOneButtonPressed -> 0x00751e40  gate_class=REVIEW
- GOG-order neighbour after:  0x00523720 CSWGuiMainInterface::OnSoloButtonPressed -> 0x00751fa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d7f0 CClientExoApp::GetPausedByCombat -> Steam 0x00740330  in candidate callees: no
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam 0x00740350  in candidate callees: no
  0x0040ddf0 CClientExoApp::SetAutoPaused -> Steam 0x00740930  in candidate callees: yes
  0x0040de20 CClientExoApp::GetAutoPaused -> Steam 0x00740960  in candidate callees: no
  0x0040dee0 CClientExoApp::ShowTutorialWindow -> Steam 0x00740a20  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x50*} | Steam {0x4*,0x1c,0x50*,0x5c}
imm sets (* = in both, 3 common): GOG {0x4*,0x6*,0x20,0x7f000000*} | Steam {0x4*,0x5,0x6*,0x7,0xf,0x1c,0xcc,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751e40: 16-byte aligned: YES
  00751e40  55             PUSH EBP
  00751e41  8B EC          MOV EBP,ESP
  00751e43  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751e40
string literals (0):
direct callees (5):
  0x00754E20  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x00740930  CClientExoApp::GetClientRangedAnimation
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x00744260  CSWGuiTargetActionMenu::SelectNextAction
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00751e40 FUN_00751e40 size=167 ret=4 callers=0 callees=5

void __thiscall FUN_00751e40(undefined4 param_1,int param_2)

{
  int iVar1;
  int iVar2;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    FUN_00754e20(param_1);
    iVar1 = *(int *)(param_2 + 0x5c);
    iVar2 = FUN_0072fb00();
    if ((*(uint *)(iVar2 + 0x1c) >> 0xf & 1) != 0) {
      FUN_00740930(1,7);
    }
    FUN_00740a20(5,0x7f000000,0x7f000000,0);
    FUN_00744260(iVar1 + -6);
  }
  return;
}
```

### decoy: 0x00751ef0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751ef0: 16-byte aligned: YES
  00751ef0  55             PUSH EBP
  00751ef1  8B EC          MOV EBP,ESP
  00751ef3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751ef0
string literals (0):
direct callees (5):
  0x00740330  CClientExoApp::GetPausedByCombat
  0x00740350  CClientExoApp::SetPausedByCombat
  0x00740960  CClientExoApp::GetAutoPaused
  0x00740930  CClientExoApp::GetClientRangedAnimation
  0x00740A20  CClientExoApp::ShowTutorialWindow
struct offsets touched (top 2):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B11b-1-13

GOG function: CGuiInGame::HideLoadModuleDebugMenu  gog 0x004dbba0  callconv=(blank)  param_size_bytes=0  gog_size=149  gog_retn=0
Candidate Steam address: 0x007cb4a0 (size 149, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cb540:STR0/CALLEE1/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbaf0 CGuiInGame::HideSubItemCreateMenu_2 -> 0x007cb3f0  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbc40 CGuiInGame::HidePowersFeatsSkillsDebugMenu -> 0x007cb540  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d580 CClientExoApp::GetMiniGame -> Steam 0x007400a0  in candidate callees: yes
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
  0x0090ee40 CSWGuiManager::PopModalPanel -> Steam 0x004109c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
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

## Card B11b-1-14

GOG function: CGuiInGame::PrevSWInGameGui  gog 0x004daac0  callconv=(blank)  param_size_bytes=0  gog_size=389  gog_retn=0
Candidate Steam address: 0x007ca3c0 (size 389, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
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
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam 0x0073fea0  in candidate callees: yes
  0x004e0e60 CGuiInGame::UpdateCreatedInGameGUI -> Steam ?  in candidate callees: n/a
  0x00527f60 CSWGuiInGameMenu::SetActiveControlID -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00640af0 CVirtualMachine::RunScript -> Steam 0x006fd8d0  in candidate callees: yes
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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

## Card B11b-1-15

GOG function: CGuiInGame::GetCanOrientInDialog  gog 0x004df1f0  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=4
Candidate Steam address: 0x007ceaf0 (size 90, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x007cf710 | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
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
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005082d0 CSWCDoor::UpdateAreaTransitionDisplay -> Steam 0x0087c9f0  in candidate callers: yes
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

## Card B11b-1-16

GOG function: CGuiInGame::HidePowersFeatsSkillsDebugMenu  gog 0x004dbc40  callconv=(blank)  param_size_bytes=0  gog_size=149  gog_retn=0
Candidate Steam address: 0x007cb540 (size 149, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cb4a0:STR0/CALLEE1/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbba0 CGuiInGame::HideLoadModuleDebugMenu -> 0x007cb4a0  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbce0 CGuiInGame::UnlockDialogParticipantOrientations -> 0x007cb5e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d580 CClientExoApp::GetMiniGame -> Steam 0x007400a0  in candidate callees: yes
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
  0x0090ee40 CSWGuiManager::PopModalPanel -> Steam 0x004109c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
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

## Card B11b-1-17

GOG function: CGuiInGame::SetComputerCameraShot  gog 0x004d6890  callconv=(blank)  param_size_bytes=12  gog_size=144  gog_retn=None
Candidate Steam address: 0x007c6200 (size 803, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.65) | G8 mac_rotation: rotation:0x004d6920:0.76>0.45
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.29 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007c6530:STR0/CALLEE0/SIG0.188/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004d4880 CGuiInGame::UpdateShotCamera -> 0x007c41f0  gate_class=AUTO
- GOG-order neighbour after:  0x004d6920 CGuiInGame::CleanUpEntryNode -> 0x007c6200  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005b2390 CSWGuiDialogComputerCamera::SetLiveFeedMessageVisible -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x38*,0x3c*,0x48*,0x7c*,0x1d0*,0x275c*} | Steam {0x4,0x8,0xc,0x38*,0x3c*,0x40,0x48*,0x7c*,0x80,0xfc,0x10c,0x180,0x1d0*,0x1d4,...(+7)}
imm sets (* = in both, 3 common): GOG {0xa*,0x64*,0xfffffffe*} | Steam {0x4,0x6,0xa*,0xc,0x10,0x30,0x64*,0x100,0x106d,0xfffffffe*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x007c6200: 16-byte aligned: YES
  007c6200  55             PUSH EBP
  007c6201  8B EC          MOV EBP,ESP
  007c6203  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007c6200
string literals (0):
direct callees (11):
  0x008BDC40  (unnamed)
  0x00410670  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x0073F430  CClientExoApp::GetModule
  0x00515A70  (unnamed)
  0x00515990  (unnamed)
  0x005158B0  (unnamed)
  0x00515620  (unnamed)
  0x008C6340  (unnamed)
  0x007409A0  CClientExoApp::EnableVideoEffect
  0x007409C0  CClientExoApp::DisableVideoEffect
struct offsets touched (top 10):
  +0x4 [dec 4]  x10  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x48 [dec 72]  x5  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EAX,ECX,EDX
  +0x38 [dec 56]  x2  via EDX
  +0x3C [dec 60]  x2  via EAX
  +0x1D0 [dec 464]  x2  via EAX,EDX
  +0x40 [dec 64]  x1  via ECX
  +0x7C [dec 124]  x1  via EDX
  +0x80 [dec 128]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007c6200 FUN_007c6200 size=803 ret=12 callers=3 callees=11

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007c6200(int param_1,int param_2,undefined4 param_3,int param_4)

{
  int iVar1;
  undefined4 *puVar2;
  float10 fVar3;
  undefined1 local_44 [16];
  undefined4 local_34;
  undefined4 local_30;
  float local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  if (*(int *)(param_1 + 0x1d0) != 0) {
    if ((param_4 == -2) || (param_4 == 10)) {
      FUN_008bdc40(0);
    }
    else {
      FUN_008bdc40(1);
    }
    FUN_00410670(*(undefined4 *)(param_1 + 0x3c));
    FUN_00410530(*(undefined4 *)(param_1 + 0x48),0,1);
    (**(code **)(**(int **)(param_1 + 0x48) + 0x7c))
              (*(undefined4 *)(*(int *)(param_1 + 0x3c) + 0x275c));
    iVar1 = FUN_0073f430();
    local_8 = (**(code **)(**(int **)(iVar1 + 0x40) + 0x80))(0x106d);
    if (local_8 != 0) {
      local_18 = 0;
      for (local_c = 0; local_c < *(int *)(param_1 + 0x1d0); local_c = local_c + 1) {
        if (*(int *)(param_1 + 0x1d4 + local_c * 0x30) == param_2) {
          local_18 = 1;
          break;
        }
      }
      local_34 = 0;
      local_30 = 0;
      local_2c = 0.0;
      local_1c = 0;
      local_20 = 0;
      local_24 = 0;
      local_28 = 0x3f800000;
      local_10 = _DAT_00993828;
      local_14 = 0;
      if (local_18 != 0) {
        puVar2 = (undefined4 *)(param_1 + 0x1d8 + local_c * 0x30);
        local_34 = *puVar2;
        local_30 = puVar2[1 = byte +0x4];
        local_2c = (float)puVar2[2 = byte +0x8] + *(float *)(param_1 + 0x1f8 + local_c * 0x30);
        puVar2 = (undefined4 *)(param_1 + 0x1e4 + local_c * 0x30);
        local_28 = *puVar2;
        local_24 = puVar2[1 = byte +0x4];
        local_20 = puVar2[2 = byte +0x8];
        local_1c = puVar2[3 = byte +0xc];
        fVar3 = (float10)FUN_00515a70(local_28,local_24,local_20,local_1c);
        fVar3 = (float10)FUN_00515990(&local_28,(float)fVar3);
        fVar3 = (float10)FUN_005158b0(&local_28,
                                      (float)(fVar3 + (float10)*(float *)(param_1 + 0x1f4 [dec 500] +
                                                                         local_c * 0x30)));
        puVar2 = (undefined4 *)FUN_00515620(local_44,(float)fVar3);
        local_28 = *puVar2;
        local_24 = puVar2[1 = byte +0x4];
        local_20 = puVar2[2 = byte +0x8];
        local_1c = puVar2[3 = byte +0xc];
        local_10 = *(undefined4 *)(param_1 + 0x1fc + local_c * 0x30);
        local_14 = *(undefined4 *)(param_1 + 0x200 + local_c * 0x30);
      }
      FUN_008c6340(6,local_34,local_30,local_2c,local_28,local_24,local_20,local_1c,local_10);
      *(undefined4 *)(param_1 + 0xfc) = 1;
      *(undefined4 *)(param_1 + 0x100) = local_34;
      *(undefined4 *)(param_1 + 0x104) = local_30;
/* ... 16 more lines truncated by --max-lines */
```

### decoy: 0x007c6530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007c6530: 16-byte aligned: YES
  007c6530  55             PUSH EBP
  007c6531  8B EC          MOV EBP,ESP
  007c6533  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007c6530
string literals (0):
direct callees (4):
  0x00410800  GuiManager::Contains
  0x00410670  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x007409C0  CClientExoApp::DisableVideoEffect
struct offsets touched (top 5):
  +0x38 [dec 56]  x3  via EDX
  +0x48 [dec 72]  x2  via EAX
  +0x4 [dec 4]  x1  via EDX
  +0x3C [dec 60]  x1  via EAX
  +0x1028 [dec 4136]  x1  via EAX
```

## Card B11b-1-18

GOG function: CGuiInGame::GetCanHeadFollowInDialog  gog 0x004dfff0  callconv=(blank)  param_size_bytes=4  gog_size=112  gog_retn=4
Candidate Steam address: 0x007cf8f0 (size 112, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007cf780:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dfe80 CGuiInGame::SetLockHeadFollowInDialog -> 0x007cf780  gate_class=AUTO
- GOG-order neighbour after:  0x004e0060 CGuiInGame::TutorialReasonWillShowWindow -> 0x007cf960  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
  0x0073F4D0  CClientExoApp::GetGameObject
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

## Card B11b-1-19

GOG function: CGuiInGame::TutorialReasonHasShown  gog 0x004e0110  callconv=(blank)  param_size_bytes=1  gog_size=90  gog_retn=4
Candidate Steam address: 0x007cfa10 (size 90, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cfa70:STR0/CALLEE0/SIG0.9/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e00e0 CGuiInGame::SetTutorialReason -> 0x007cf960  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0170 CGuiInGame::SetTutorialReasonTracked -> 0x007cfa70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xf88*} | Steam {0xf88*}
imm sets (* = in both, 5 common): GOG {0x3*,0x4*,0x7*,0x8*,0x100*} | Steam {0x3*,0x4*,0x7*,0x8*,0x100*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cfa10: 16-byte aligned: YES
  007cfa10  55             PUSH EBP
  007cfa11  8B EC          MOV EBP,ESP
  007cfa13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cfa10
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007cfa10 FUN_007cfa10 size=90 ret=4 callers=2 callees=0

uint __thiscall FUN_007cfa10(int param_1,byte param_2)

{
  uint uVar1;
  
  if (param_2 < 0x100) {
    uVar1 = (uint)*(byte *)(param_1 + 0xf88 + ((int)(uint)param_2 >> 3)) &
            1 << (param_2 + (char)((int)(uint)param_2 >> 3) * -8 & 0x1f);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x007cfa70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cfa70: 16-byte aligned: YES
  007cfa70  55             PUSH EBP
  007cfa71  8B EC          MOV EBP,ESP
  007cfa73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cfa70
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B11b-1-20

GOG function: CGuiInGame::InitializePlaceableCameras  gog 0x004dc370  callconv=(blank)  param_size_bytes=0  gog_size=60  gog_retn=0
Candidate Steam address: 0x007cbc70 (size 60, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x004dc630:0.59>0.12
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cbc40:STR0/CALLEE2/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dc340 CGuiInGame::IsGlobalFading -> 0x007cbc40  gate_class=REVIEW
- GOG-order neighbour after:  0x004dc3e0 CGuiInGame::SetPlaceableCamera -> 0x007cbce0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x00449b50 CClientExoAppInternal::UpdateRumble -> Steam 0x0079d550  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x50*,0x6c*} | Steam {0x50*,0x6c*}
imm sets (* = in both, 2 common): GOG {0xc*,0x41*} | Steam {0xc*,0x41*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007cbc70: 16-byte aligned: YES
  007cbc70  55             PUSH EBP
  007cbc71  8B EC          MOV EBP,ESP
  007cbc73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cbc70
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x50 [dec 80]  x1  via ECX
  +0x6C [dec 108]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007cbc70 FUN_007cbc70 size=60 ret=0 callers=2 callees=0
THUNK -> unknown (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool __fastcall FUN_007cbc70(int param_1)

{
  return (float)_DAT_00986b70 < *(float *)(*(int *)(param_1 + 0x6c) + 0x50);
}
```

### decoy: 0x007cbc40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cbc40: 16-byte aligned: YES
  007cbc40  55             PUSH EBP
  007cbc41  8B EC          MOV EBP,ESP
  007cbc43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cbc40
string literals (0):
direct callees (1):
  0x007BCA60  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x2  via EAX,ECX
```

## Card B11b-1-21

GOG function: CGuiInGame::SetConversationType  gog 0x004d00e0  callconv=(blank)  param_size_bytes=5  gog_size=192  gog_retn=8
Candidate Steam address: 0x007bfa50 (size 192, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::ShowDialogEntry | G7 mac: pass(0.83) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007bfb10:STR0/CALLEE0/SIG0.205/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004d00b0 CGuiInGame::SetLevelUpMode -> 0x007bfa20  gate_class=REVIEW
- GOG-order neighbour after:  0x004d01a0 CGuiInGame::ShowDialogEntry -> 0x007bfb10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005b0d70 CSWGuiDialogComputer::Constructor -> Steam ?  in candidate callees: n/a
  0x005b1ef0 CSWGuiDialogComputer::SetType -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x38*,0x3c*,0x40*,0x44*} | Steam {0x38*,0x3c*,0x40*,0x44*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x10*,0x45c0*} | Steam {0x4*,0x8*,0x10*,0x45c0*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007bfa50: 16-byte aligned: YES
  007bfa50  55             PUSH EBP
  007bfa51  8B EC          MOV EBP,ESP
  007bfa53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007bfa50
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x008BC620  (unnamed)
  0x008BD7A0  (unnamed)
struct offsets touched (top 4):
  +0x44 [dec 68]  x4  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x38 [dec 56]  x1  via ECX
  +0x40 [dec 64]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007bfa50 FUN_007bfa50 size=192 ret=8 callers=1 callees=3

void __thiscall FUN_007bfa50(int param_1,int param_2,undefined1 param_3)

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
  if (param_2 == 1) {
    if (*(int *)(param_1 + 0x44) == 0) {
      iVar1 = FUN_00919723(0x45c0,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
      local_8 = 0;
      if (iVar1 == 0) {
        local_20 = 0;
      }
      else {
        local_20 = FUN_008bc620(*(undefined4 *)(param_1 + 0x38));
      }
      *(undefined4 *)(param_1 + 0x44) = local_20;
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x3c) = *(undefined4 *)(param_1 + 0x44);
    FUN_008bd7a0(param_3);
  }
  else {
    *(undefined4 *)(param_1 + 0x3c) = *(undefined4 *)(param_1 + 0x40);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007bfb10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bfb10: 16-byte aligned: YES
  007bfb10  55             PUSH EBP
  007bfb11  8B EC          MOV EBP,ESP
  007bfb13  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bfb10
string literals (0):
direct callees (10):
  0x007B38D0  CClientExoAppInternal::HideMouse
  0x0073FEE0  CClientExoApp::SetInputClass
  0x00740240  CClientExoApp::ResetDriveAcceleration
  0x008BE6A0  (unnamed)
  0x00410800  GuiManager::Contains
  0x008BAF10  CSWGuiDialogLetterbox::SetMiddle
  0x00410530  CSWGuiManager::AddPanel
  0x008BACA0  CSWGuiDialogLetterbox::SetTop
  0x008BADA0  CSWGuiDialogLetterbox::SetBottom
  0x007C9C40  CGuiInGame::SetSWGuiStatus
struct offsets touched (top 10):
  +0x3C [dec 60]  x10  via EAX,ECX,EDX
  +0x38 [dec 56]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x60 [dec 96]  x2  via EAX,EDX
  +0x64 [dec 100]  x2  via EAX,ECX
  +0x68 [dec 104]  x2  via ECX,EDX
  +0xC8 [dec 200]  x2  via ECX,EDX
  +0x40 [dec 64]  x1  via EDX
  +0x44 [dec 68]  x1  via EDX
  +0x4C [dec 76]  x1  via EAX
```

## Card B11b-1-22

GOG function: CGuiInGame::ChangeCharacter  gog 0x004e09b0  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x007d02b0 (size 25, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: HandleInputEvent 0x7b12c0)
- best decoy (neighbour/other candidate) scores: 0x007d02d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0920 CGuiInGame::EndControllerPauseState -> 0x007d0220  gate_class=AUTO
- GOG-order neighbour after:  0x004e09d0 CGuiInGame::SetupPauseGuiExtent -> 0x007d02d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005225e0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: yes
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

## Card B11b-1-23

GOG function: CGuiInGame::Constructor  gog 0x004cd090  callconv=(blank)  param_size_bytes=0  gog_size=3333  gog_retn=0
Candidate Steam address: 0x007bc760 (size 140, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc7f0:STR0/CALLEE0/SIG0.045/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004cce20 CSWCAnimBase::SetGroundTilt  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004cdda0 CGuiInGame::ShutDownGlobal -> 0x007bdb10  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (18), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: no
  0x00462bc0 CClientOptions::SetCameraMode -> Steam 0x007b5e50  in candidate callees: no
  0x00495270 unnamed -> Steam ?  in candidate callees: n/a
  0x004e29e0 CSWCMessage::HandleServerToPlayerUpdate_Appearance -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  ... 10 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004353b0 CClientExoAppInternal::StartServices -> Steam 0x00788d90  in candidate callers: no
GOG strings (3): '.\\\\swkotor2.ini'; 'Display Options'; 'Compass'
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x2c,0x30,0x34,0x38,0x3c,0x40,0x44,0x48,0x4c,0x50,0x54,...(+123)} | Steam {}
imm sets (* = in both, 2 common): GOG {0x2*,0x3,0x4,0x5,0x6,0x7,0x8*,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+22)} | Steam {0x2*,0x8*,0x60,0x78,0xd8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007bc760: 16-byte aligned: YES
  007bc760  55             PUSH EBP
  007bc761  8B EC          MOV EBP,ESP
  007bc763  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007bc760
string literals (0):
direct callees (4):
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007bc760 FUN_007bc760 size=140 ret=0 callers=1 callees=4

void __fastcall FUN_007bc760(int param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0096951b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(undefined ***)(param_1 + 0x78) = CSWGuiLabel::vftable;
  local_8._0_1_ = 2;
  local_8._1_3_ = 0;
  FUN_00417290(uVar1);
  local_8._0_1_ = 1;
  FUN_00414c60();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00418640();
  local_8 = 0xffffffff;
  FUN_0040e860();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007bc7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bc7f0: 16-byte aligned: YES
  007bc7f0  55             PUSH EBP
  007bc7f1  8B EC          MOV EBP,ESP
  007bc7f3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bc7f0
string literals (0):
direct callees (3):
  0x007BCD10  (unnamed)
  0x0051A920  (unnamed)
  0x00920670  (unnamed)
struct offsets touched (top 9):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x1  via ECX
  +0x2C [dec 44]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x68 [dec 104]  x1  via EAX
  +0x6C [dec 108]  x1  via ECX
  +0x70 [dec 112]  x1  via EDX
  +0x74 [dec 116]  x1  via EAX
  +0x1C0 [dec 448]  x1  via ECX
```

## Card B11b-1-24

GOG function: CSWGuiMainInterface::UpdateActionMenuSelected  gog 0x00523370  callconv=(blank)  param_size_bytes=4  gog_size=54  gog_retn=4
Candidate Steam address: 0x00751b00 (size 228, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.12) | G8 mac_rotation: rotation:0x00523670:0.52>0.08
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00751bf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523280 CSWGuiMainInterface::GetActiveActionIdx -> 0x00751b00  gate_class=REVIEW
- GOG-order neighbour after:  0x005233b0 CSWGuiActionMenuIos::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x50*,0x5c*,0x1c77} | Steam {0x8,0x38,0x50*,0x5c*,0x78,0x7c,0x1c7c}
imm sets (* = in both, 2 common): GOG {0x4*,0x6,0xc*} | Steam {0x4*,0xc*,0x18,0x3c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751b00: 16-byte aligned: YES
  00751b00  55             PUSH EBP
  00751b01  8B EC          MOV EBP,ESP
  00751b03  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751b00
string literals (0):
direct callees (1):
  0x0074DA40  CSWGuiMainInterface::UpdateStanceAction
struct offsets touched (top 4):
  +0x15414 [dec 87060]  x2  via EAX,EDX
  +0x38 [dec 56]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00751b00 FUN_00751b00 size=228 ret=4 callers=0 callees=1

void __thiscall FUN_00751b00(int param_1,int param_2)

{
  int iVar1;
  undefined4 local_10;
  undefined4 local_8;
  
  if ((*(int *)(param_2 + 0x50) != 0) && (*(int *)(param_1 + 0x15414) != 0)) {
    iVar1 = *(int *)(param_2 + 0x5c);
    local_8 = 0;
    if (*(int *)(param_1 + 0x1c7c + iVar1 * 4) != -1) {
      for (local_10 = 0; local_10 < *(int *)(param_1 + 0x7c + iVar1 * 0xc); local_10 = local_10 + 1)
      {
        if (*(int *)(*(int *)(param_1 + 0x78 + iVar1 * 0xc) + 8 + local_10 * 0x3c) ==
            *(int *)(param_1 + 0x1c7c + iVar1 * 4)) {
          local_8 = local_10 * 0x3c + *(int *)(param_1 + 0x78 + iVar1 * 0xc);
          break;
        }
      }
    }
    if ((local_8 != 0) && (*(char *)(local_8 + 0x38) != '\0')) {
      FUN_0074da40();
    }
    *(undefined4 *)(param_1 + 0x15414) = 0;
  }
  return;
}
```

### decoy: 0x00751bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751bf0: 16-byte aligned: YES
  00751bf0  55             PUSH EBP
  00751bf1  8B EC          MOV EBP,ESP
  00751bf3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751bf0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
  +0x1C77 [dec 7287]  x1  via EDX
```

## Card B11b-1-25

GOG function: CGuiInGame::RePopulateMainInterface  gog 0x004df250  callconv=(blank)  param_size_bytes=0  gog_size=39  gog_retn=0
Candidate Steam address: 0x007ceb50 (size 39, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: MainLoop 0x781be0, SetCombatMode 0x79d6e0)
- best decoy (neighbour/other candidate) scores: 0x007ceec0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004df1f0 CGuiInGame::GetCanOrientInDialog -> 0x007ceaf0  gate_class=REVIEW
- GOG-order neighbour after:  0x004df280 CGuiInGame::SetGlobalDialogState -> 0x007ceb80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0051e760 CSWGuiMainInterface::PopulateMenus -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x00449ce0 CClientExoAppInternal::SetCombatMode -> Steam 0x0079d6e0  in candidate callers: yes
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

## Card B11b-1-26

GOG function: CGuiInGame::InitializeMap  gog 0x004df5c0  callconv=(blank)  param_size_bytes=0  gog_size=37  gog_retn=0
Candidate Steam address: 0x007ceec0 (size 37, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cec60:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004df580 CGuiInGame::AddFloatyText -> 0x007cee80  gate_class=REVIEW
- GOG-order neighbour after:  0x004df5f0 CGuiInGame::UpdateStatus -> 0x007ceef0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00522390 CSWGuiMainInterface::InitializeMap -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
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
prologue for 0x007ceec0: 16-byte aligned: YES
  007ceec0  55             PUSH EBP
  007ceec1  8B EC          MOV EBP,ESP
  007ceec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ceec0
string literals (0):
direct callees (1):
  0x00750C10  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ceec0 FUN_007ceec0 size=37 ret=0 callers=2 callees=1

void __fastcall FUN_007ceec0(int param_1)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    FUN_00750c10();
  }
  return;
}
```

### decoy: 0x007cec60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cec60: 16-byte aligned: YES
  007cec60  55             PUSH EBP
  007cec61  8B EC          MOV EBP,ESP
  007cec63  81 EC C0 00 00 00 SUB ESP,0xc0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cec60
string literals (0):
direct callees (6):
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0073F430  CClientExoApp::GetModule
  0x00463120  (unnamed)
  0x005143C0  (unnamed)
  0x0045CFE0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 7):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
  +0xF00 [dec 3840]  x1  via EAX
  +0xF04 [dec 3844]  x1  via EDX
```

## Card B11b-1-27

GOG function: CGuiInGame::NextSWInGameGui  gog 0x004da930  callconv=(blank)  param_size_bytes=0  gog_size=389  gog_retn=0
Candidate Steam address: 0x007ca230 (size 389, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007ca3c0:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004da760 CGuiInGame::HideSWInGameGui -> 0x007ca060  gate_class=AUTO
- GOG-order neighbour after:  0x004daac0 CGuiInGame::PrevSWInGameGui -> 0x007ca3c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam 0x0073fea0  in candidate callees: yes
  0x004e0e60 CGuiInGame::UpdateCreatedInGameGUI -> Steam ?  in candidate callees: n/a
  0x00527f60 CSWGuiInGameMenu::SetActiveControlID -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00640af0 CVirtualMachine::RunScript -> Steam 0x006fd8d0  in candidate callees: yes
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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

## Card B11b-1-28

GOG function: CGuiInGame::HideSwoopUpgradeScreen  gog 0x004dd7d0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007cd0d0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cd0b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dd740 CGuiInGame::UpdateMessageGUI -> 0x007cd040  gate_class=AUTO
- GOG-order neighbour after:  0x004dd7f0 CGuiInGame::UpdateDialog -> 0x007cd0f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005b2d90 CSWGuiBarkBubble::Resume -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4c*} | Steam {0x4c*}
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
prologue for 0x007cd0d0: 16-byte aligned: YES
  007cd0d0  55             PUSH EBP
  007cd0d1  8B EC          MOV EBP,ESP
  007cd0d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cd0d0
string literals (0):
direct callees (1):
  0x008BE630  (unnamed)
struct offsets touched (top 1):
  +0x4C [dec 76]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007cd0d0 FUN_007cd0d0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x008BE630 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007cd0d0(undefined4 param_1)

{
  FUN_008be630(param_1);
  return;
}
```

### decoy: 0x007cd0b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cd0b0: 16-byte aligned: YES
  007cd0b0  55             PUSH EBP
  007cd0b1  8B EC          MOV EBP,ESP
  007cd0b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cd0b0
string literals (0):
direct callees (1):
  0x008BE5D0  (unnamed)
struct offsets touched (top 1):
  +0x4C [dec 76]  x1  via EAX
```

## Card B11b-1-29

GOG function: CGuiInGame::InitializeDialogAnimationData  gog 0x004d05c0  callconv=(blank)  param_size_bytes=4  gog_size=249  gog_retn=4
Candidate Steam address: 0x007bff30 (size 249, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: rotation:0x004d0720:0.96>0.76
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007c0030:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004d0550 CGuiInGame::SetStuntModelData -> 0x007bfec0  gate_class=AUTO
- GOG-order neighbour after:  0x004d06c0 CGuiInGame::SetDialogAnimationData -> 0x007c0030  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004d0720 CGuiInGame::ClearDialogAnimationData -> Steam ?  in candidate callees: n/a
  0x004d07a0 CGuiInGame::StoreLastDialogAnimationData -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x004dd7f0 CGuiInGame::UpdateDialog -> Steam 0x007cd0f0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1b8*,0x1c0*,0x1c4*} | Steam {0x1b8*,0x1c0*,0x1c4*}
imm sets (* = in both, 3 common): GOG {0x2*,0x4*,0x18*} | Steam {0x2*,0x4*,0x18*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007bff30: 16-byte aligned: YES
  007bff30  55             PUSH EBP
  007bff31  8B EC          MOV EBP,ESP
  007bff33  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bff30
string literals (0):
direct callees (4):
  0x007C0110  CGuiInGame::StoreLastDialogAnimationData
  0x007C0090  CGuiInGame::ClearDialogAnimationData
  0x00919723  operator new
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x1B8 [dec 440]  x3  via EAX,EDX
  +0x1C0 [dec 448]  x3  via EAX,ECX,EDX
  +0x1C4 [dec 452]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007bff30 FUN_007bff30 size=249 ret=4 callers=3 callees=4

undefined4 __thiscall FUN_007bff30(int param_1,int param_2)

{
  longlong lVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  local_8 = 0;
  FUN_007c0110();
  FUN_007c0090();
  if (param_2 != 0) {
    *(int *)(param_1 + 0x1b8) = param_2;
    lVar1 = (ulonglong)*(uint *)(param_1 + 0x1b8) * 4;
    uVar2 = FUN_00919723(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
    *(undefined4 *)(param_1 + 0x1c0) = uVar2;
    lVar1 = (ulonglong)*(uint *)(param_1 + 0x1b8) * 2;
    uVar2 = FUN_00919723(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
    *(undefined4 *)(param_1 + 0x1c4) = uVar2;
    if ((*(int *)(param_1 + 0x1c0) == 0) || (*(int *)(param_1 + 0x1c4) == 0)) {
      FUN_009196fd(*(undefined4 *)(param_1 + 0x1c0));
      FUN_009196fd(*(undefined4 *)(param_1 + 0x1c4));
      local_8 = 0;
    }
    else {
      local_8 = 1;
    }
  }
  return local_8;
}
```

### decoy: 0x007c0030 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007c0030: 16-byte aligned: YES
  007c0030  55             PUSH EBP
  007c0031  8B EC          MOV EBP,ESP
  007c0033  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007c0030
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1C0 [dec 448]  x2  via ECX,EDX
  +0x1C4 [dec 452]  x2  via EAX,EDX
  +0x1B8 [dec 440]  x1  via EAX
```

## Card B11b-1-30

GOG function: CSWGuiMainInterface::AddFloatyText  gog 0x00524620  callconv=(blank)  param_size_bytes=0  gog_size=170  gog_retn=0
Candidate Steam address: 0x00753010 (size 33, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: rotation:0x00524a20:0.62>0.26
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00752f50:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00524590 CSWGuiMainInterface::ShowFlashingStatus -> 0x00752e10  gate_class=REVIEW
- GOG-order neighbour after:  0x005246d0 CSWGuiMainInterface::SetupPauseGuiExtent -> 0x00752f50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0044cd50 CClientExoAppInternal::SetToolTipInstantMode -> Steam 0x007a0750  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x48,0x6f9c,0x732c,0xa904} | Steam {}
imm sets (* = in both, 0 common): GOG {0x14,0x6f54,0x709c,0x71e4,0xfffffffd} | Steam {0xcc}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00753010: 16-byte aligned: YES
  00753010  55             PUSH EBP
  00753011  8B EC          MOV EBP,ESP
  00753013  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00753010
string literals (0):
direct callees (2):
  0x0040FFB0  CSWGuiPanel::ResetFont
  0x00744AC0  CSWGuiTargetActionMenu::Destructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00753010 FUN_00753010 size=33 ret=0 callers=0 callees=2

void __fastcall FUN_00753010(undefined4 param_1)

{
  FUN_0040ffb0(param_1);
  FUN_00744ac0();
  return;
}
```

### decoy: 0x00752f50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752f50: 16-byte aligned: YES
  00752f50  55             PUSH EBP
  00752f51  8B EC          MOV EBP,ESP
  00752f53  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752f50
string literals (0):
direct callees (3):
  0x00479AB0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 6):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0x6C [dec 108]  x1  via ECX
  +0xBA54 [dec 47700]  x1  via EDX
  +0xBA5C [dec 47708]  x1  via EAX
```

## Card B11b-1-31

GOG function: CGuiInGame::IsDialogAnimationRange1000_1727  gog 0x004db8c0  callconv=(blank)  param_size_bytes=2  gog_size=45  gog_retn=4
Candidate Steam address: 0x007cb1c0 (size 45, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007cb1f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004db710 CGuiInGame::IsFireForgetDialogAnimation -> 0x007cae60  gate_class=REVIEW
- GOG-order neighbour after:  0x004db8f0 CGuiInGame::IsLoopingDialogStuntAnimation -> 0x007cb1f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x3e8*,0x6bf*} | Steam {0x4*,0x3e8*,0x6bf*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cb1c0: 16-byte aligned: YES
  007cb1c0  55             PUSH EBP
  007cb1c1  8B EC          MOV EBP,ESP
  007cb1c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb1c0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007cb1c0 FUN_007cb1c0 size=45 ret=4 callers=1 callees=0

undefined4 FUN_007cb1c0(ushort param_1)

{
  undefined4 uVar1;
  
  if ((param_1 < 1000) || (0x6bf < param_1)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x007cb1f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb1f0: 16-byte aligned: YES
  007cb1f0  55             PUSH EBP
  007cb1f1  8B EC          MOV EBP,ESP
  007cb1f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb1f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B11b-1-32

GOG function: CGuiInGame::IsOnDialogNonDependantList  gog 0x004e0ae0  callconv=(blank)  param_size_bytes=4  gog_size=129  gog_retn=4
Candidate Steam address: 0x007d03e0 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007d0470:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e09f0 CGuiInGame::AddToDialogNonDependantList -> 0x007d02f0  gate_class=AUTO
- GOG-order neighbour after:  0x004e0b70 CGuiInGame::ClearDialogNonDependantList -> 0x007d0470  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00607070 CExoString::CompareNoCase_2 -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0xf2c*} | Steam {0x4*,0xc*,0xf2c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007d03e0: 16-byte aligned: YES
  007d03e0  55             PUSH EBP
  007d03e1  8B EC          MOV EBP,ESP
  007d03e3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d03e0
string literals (0):
direct callees (5):
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00521060  (unnamed)
  0x007A1720  (unnamed)
  0x00734010  CExoString::CStr
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007d03e0 FUN_007d03e0 size=129 ret=4 callers=3 callees=5

undefined4 FUN_007d03e0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  int local_c;
  undefined4 local_8;
  
  iVar1 = FUN_00734f60(&DAT_00986925);
  if (iVar1 == 0) {
    local_8 = FUN_00521060(extraout_var);
    local_c = FUN_007a1720();
    while (local_c != 0) {
      uVar2 = FUN_00734010();
      iVar1 = FUN_00734f60(uVar2);
      if (iVar1 != 0) {
        return 1;
      }
      local_8 = FUN_0058c370(&local_c);
    }
  }
  return 0;
}
```

### decoy: 0x007d0470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d0470: 16-byte aligned: YES
  007d0470  55             PUSH EBP
  007d0471  8B EC          MOV EBP,ESP
  007d0473  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d0470
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B11b-1-33

GOG function: CGuiInGame::ShowDialogMessage  gog 0x004e0bf0  callconv=(blank)  param_size_bytes=0  gog_size=50  gog_retn=0
Candidate Steam address: 0x007d0470 (size 113, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::ClearDialogNonDependantList | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x004e0c60:0.92>0.17
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d04f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0b70 CGuiInGame::ClearDialogNonDependantList -> 0x007d0470  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0c30 CGuiInGame::SetCombatMessage -> 0x007d0530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x98,0xa900} | Steam {}
imm sets (* = in both, 0 common): GOG {0x8} | Steam {0x4,0x14,0xf2c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007d0470: 16-byte aligned: YES
  007d0470  55             PUSH EBP
  007d0471  8B EC          MOV EBP,ESP
  007d0473  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d0470
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007d0470 FUN_007d0470 size=113 ret=0 callers=1 callees=4

void FUN_007d0470(void)

{
  int iVar1;
  
  while (iVar1 = FUN_007a1780(), iVar1 == 0) {
    iVar1 = FUN_007f2a90();
    if (iVar1 != 0) {
      FUN_00733780();
      FUN_009196fd(iVar1);
    }
  }
  return;
}
```

### decoy: 0x007d04f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d04f0: 16-byte aligned: YES
  007d04f0  55             PUSH EBP
  007d04f1  8B EC          MOV EBP,ESP
  007d04f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d04f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x98 [dec 152]  x2  via EAX,ECX
  +0xA900 [dec 43264]  x1  via EDX
```

## Card B11b-1-34

GOG function: CGuiInGame::CleanUpEntryNode  gog 0x004d6920  callconv=(blank)  param_size_bytes=0  gog_size=659  gog_retn=12
Candidate Steam address: 0x007c6200 (size 803, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::SetComputerCameraShot | G7 mac: pass(0.35) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.78 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.78
- best decoy (neighbour/other candidate) scores: 0x007c41f0:STR0/CALLEE2/SIG0.376/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004d6890 CGuiInGame::SetComputerCameraShot -> 0x007c6200  gate_class=REVIEW
- GOG-order neighbour after:  0x004d6bc0 CGuiInGame::CleanUpReplyNode -> 0x007c6530  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040c8f0 CClientExoApp::GetModule -> Steam 0x0073f430  in candidate callees: yes
  0x0040de60 CClientExoApp::EnableVideoEffect -> Steam 0x007409a0  in candidate callees: yes
  0x0040de80 CClientExoApp::DisableVideoEffect -> Steam 0x007409c0  in candidate callees: yes
  0x00578ad0 CSWBehaviorCameraDialog::SetShot_2 -> Steam ?  in candidate callees: n/a
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
  0x0080d740 Global::Pitch -> Steam ?  in candidate callees: n/a
  0x0080d820 Global::Yaw -> Steam ?  in candidate callees: n/a
  0x0080d900 Global::float_Roll_Quaternion -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 16 common): GOG {0x4*,0x8*,0xc*,0x40*,0x80*,0xfc*,0x10c*,0x180*,0x1d0*,0x1d4*,0x1d8*,0x1e4*,0x1f4*,0x1f8*,...(+2)} | Steam {0x4*,0x8*,0xc*,0x38,0x3c,0x40*,0x48,0x7c,0x80*,0xfc*,0x10c*,0x180*,0x1d0*,0x1d4*,...(+7)}
imm sets (* = in both, 8 common): GOG {0x4*,0x6*,0xc*,0x10*,0x30*,0x100*,0x106d*,0xfffffffe*} | Steam {0x4*,0x6*,0xa,0xc*,0x10*,0x30*,0x64,0x100*,0x106d*,0xfffffffe*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 12)
```

### prologue
```
prologue for 0x007c6200: 16-byte aligned: YES
  007c6200  55             PUSH EBP
  007c6201  8B EC          MOV EBP,ESP
  007c6203  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007c6200
string literals (0):
direct callees (11):
  0x008BDC40  (unnamed)
  0x00410670  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x0073F430  CClientExoApp::GetModule
  0x00515A70  (unnamed)
  0x00515990  (unnamed)
  0x005158B0  (unnamed)
  0x00515620  (unnamed)
  0x008C6340  (unnamed)
  0x007409A0  CClientExoApp::EnableVideoEffect
  0x007409C0  CClientExoApp::DisableVideoEffect
struct offsets touched (top 10):
  +0x4 [dec 4]  x10  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x48 [dec 72]  x5  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EAX,ECX,EDX
  +0x38 [dec 56]  x2  via EDX
  +0x3C [dec 60]  x2  via EAX
  +0x1D0 [dec 464]  x2  via EAX,EDX
  +0x40 [dec 64]  x1  via ECX
  +0x7C [dec 124]  x1  via EDX
  +0x80 [dec 128]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007c6200 FUN_007c6200 size=803 ret=12 callers=3 callees=11

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007c6200(int param_1,int param_2,undefined4 param_3,int param_4)

{
  int iVar1;
  undefined4 *puVar2;
  float10 fVar3;
  undefined1 local_44 [16];
  undefined4 local_34;
  undefined4 local_30;
  float local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  if (*(int *)(param_1 + 0x1d0) != 0) {
    if ((param_4 == -2) || (param_4 == 10)) {
      FUN_008bdc40(0);
    }
    else {
      FUN_008bdc40(1);
    }
    FUN_00410670(*(undefined4 *)(param_1 + 0x3c));
    FUN_00410530(*(undefined4 *)(param_1 + 0x48),0,1);
    (**(code **)(**(int **)(param_1 + 0x48) + 0x7c))
              (*(undefined4 *)(*(int *)(param_1 + 0x3c) + 0x275c));
    iVar1 = FUN_0073f430();
    local_8 = (**(code **)(**(int **)(iVar1 + 0x40) + 0x80))(0x106d);
    if (local_8 != 0) {
      local_18 = 0;
      for (local_c = 0; local_c < *(int *)(param_1 + 0x1d0); local_c = local_c + 1) {
        if (*(int *)(param_1 + 0x1d4 + local_c * 0x30) == param_2) {
          local_18 = 1;
          break;
        }
      }
      local_34 = 0;
      local_30 = 0;
      local_2c = 0.0;
      local_1c = 0;
      local_20 = 0;
      local_24 = 0;
      local_28 = 0x3f800000;
      local_10 = _DAT_00993828;
      local_14 = 0;
      if (local_18 != 0) {
        puVar2 = (undefined4 *)(param_1 + 0x1d8 + local_c * 0x30);
        local_34 = *puVar2;
        local_30 = puVar2[1 = byte +0x4];
        local_2c = (float)puVar2[2 = byte +0x8] + *(float *)(param_1 + 0x1f8 + local_c * 0x30);
        puVar2 = (undefined4 *)(param_1 + 0x1e4 + local_c * 0x30);
        local_28 = *puVar2;
        local_24 = puVar2[1 = byte +0x4];
        local_20 = puVar2[2 = byte +0x8];
        local_1c = puVar2[3 = byte +0xc];
        fVar3 = (float10)FUN_00515a70(local_28,local_24,local_20,local_1c);
        fVar3 = (float10)FUN_00515990(&local_28,(float)fVar3);
        fVar3 = (float10)FUN_005158b0(&local_28,
                                      (float)(fVar3 + (float10)*(float *)(param_1 + 0x1f4 [dec 500] +
                                                                         local_c * 0x30)));
        puVar2 = (undefined4 *)FUN_00515620(local_44,(float)fVar3);
        local_28 = *puVar2;
        local_24 = puVar2[1 = byte +0x4];
        local_20 = puVar2[2 = byte +0x8];
        local_1c = puVar2[3 = byte +0xc];
        local_10 = *(undefined4 *)(param_1 + 0x1fc + local_c * 0x30);
        local_14 = *(undefined4 *)(param_1 + 0x200 + local_c * 0x30);
      }
      FUN_008c6340(6,local_34,local_30,local_2c,local_28,local_24,local_20,local_1c,local_10);
      *(undefined4 *)(param_1 + 0xfc) = 1;
      *(undefined4 *)(param_1 + 0x100) = local_34;
      *(undefined4 *)(param_1 + 0x104) = local_30;
/* ... 16 more lines truncated by --max-lines */
```

### decoy: 0x007c41f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007c41f0: 16-byte aligned: YES
  007c41f0  55             PUSH EBP
  007c41f1  8B EC          MOV EBP,ESP
  007c41f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007c41f0
string literals (0):
direct callees (31):
  0x0073F4D0  CClientExoApp::GetGameObject
  0x00773BF0  CSWCCreature::LookAt
  0x0073F750  ClientApp::GetGuiInGame
  0x005143C0  (unnamed)
  0x007CF710  CGuiInGame::GetCanOrientInDialog
  0x007CF8F0  CGuiInGame::GetCanHeadFollowInDialog
  0x0091E620  (unnamed)
  0x007741A0  (unnamed)
  0x00922A3D  (unnamed)
  0x005195A0  (unnamed)
  0x0077FC10  CSWCCreature::SetLockOrientationToObject
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0057D370  CSWSCreature::SetLockOrientationToObject
  0x00543F10  CSWSObject::SetOrientation
  0x0073F430  CClientExoApp::GetModule
  0x007CB010  (unnamed)
  0x007CAE60  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x51  via EAX,ECX,EDX
  +0x8 [dec 8]  x33  via EAX,ECX,EDX
  +0x2C [dec 44]  x22  via EAX,EDX
  +0x30 [dec 48]  x12  via EAX,EDX
  +0xC [dec 12]  x8  via EAX,ECX,EDX
  +0x190 [dec 400]  x6  via EAX,ECX,EDX
  +0xFB8 [dec 4024]  x5  via EAX,ECX,EDX
  +0x88 [dec 136]  x4  via EAX
  +0x194 [dec 404]  x4  via EAX,ECX,EDX
  +0x18 [dec 24]  x3  via EAX,EDX
```

## Card B11b-1-35

GOG function: CGuiInGame::ResetInterfaceFont  gog 0x004e0d20  callconv=(blank)  param_size_bytes=0  gog_size=306  gog_retn=0
Candidate Steam address: 0x007d0b90 (size 153, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CGuiInGame::GetCanClick | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x004e0ca0:0.56>0.22
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d0b40:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0ca0 CGuiInGame::RemoveGUIPopUps -> 0x007d0560  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0e60 CGuiInGame::UpdateCreatedInGameGUI -> 0x007d0760  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0073a680 CSWSEffectListHandler::OnApplyDeath -> Steam 0x00597d90  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x38,0x3c,0x54,0x94,0xa0,0xa4,0xb0} | Steam {0x4,0xfcc,0xfd0,0xfd4}
imm sets (* = in both, 0 common): GOG {0x20,0x28} | Steam {0x1c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007d0b90: 16-byte aligned: YES
  007d0b90  55             PUSH EBP
  007d0b91  8B EC          MOV EBP,ESP
  007d0b93  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d0b90
string literals (0):
direct callees (3):
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0051AF60  CWorldTimer::SubtractWorldTimes
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xFCC [dec 4044]  x1  via ECX
  +0xFD0 [dec 4048]  x1  via EDX
  +0xFD4 [dec 4052]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007d0b90 FUN_007d0b90 size=153 ret=0 callers=2 callees=3

undefined4 FUN_007d0b90(void)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int *piVar5;
  undefined4 *puVar6;
  uint *puVar7;
  int extraout_var;
  int iVar8;
  undefined4 local_14;
  uint local_10;
  undefined4 local_c;
  int local_8;
  
  local_10 = 0;
  uVar1 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  puVar6 = &local_c;
  puVar4 = &local_14;
  FUN_0073f3b0(puVar4,puVar6);
  iVar8 = extraout_var;
  FUN_0051ad20(puVar4,puVar6);
  puVar7 = &local_10;
  piVar5 = &local_8;
  uVar3 = *(undefined4 *)(iVar8 + 0xfd0);
  uVar2 = *(undefined4 *)(iVar8 + 0xfcc);
  FUN_0073f3b0(local_14,local_c,uVar2,uVar3,piVar5,puVar7,iVar8,*(undefined4 *)(DAT_00a1b4a4 + 4),
               uVar1);
  FUN_0051af60(local_14,local_c,uVar2,uVar3,piVar5,puVar7);
  if (local_8 == 0) {
    if (local_10 < *(uint *)(iVar8 + 0xfd4)) {
      uVar1 = 0;
    }
    else {
      uVar1 = 1;
    }
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x007d0b40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d0b40: 16-byte aligned: YES
  007d0b40  55             PUSH EBP
  007d0b41  8B EC          MOV EBP,ESP
  007d0b43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d0b40
string literals (0):
direct callees (2):
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0xFD4 [dec 4052]  x1  via ECX
```

## Card B11b-1-36

GOG function: CSWGuiMainInterface::OnPauseButtonPressed  gog 0x00523670  callconv=(blank)  param_size_bytes=4  gog_size=172  gog_retn=4
Candidate Steam address: 0x00751fa0 (size 59, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: rotation:0x00523760:0.89>0.62
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.60 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00751ef0:STR0/CALLEE5/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005235c0 CSWGuiMainInterface::OnClearOneButtonPressed -> 0x00751e40  gate_class=REVIEW
- GOG-order neighbour after:  0x00523720 CSWGuiMainInterface::OnSoloButtonPressed -> 0x00751fa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d7f0 CClientExoApp::GetPausedByCombat -> Steam 0x00740330  in candidate callees: no
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam 0x00740350  in candidate callees: no
  0x0040ddf0 CClientExoApp::SetAutoPaused -> Steam 0x00740930  in candidate callees: no
  0x0040de20 CClientExoApp::GetAutoPaused -> Steam 0x00740960  in candidate callees: no
  0x0040dee0 CClientExoApp::ShowTutorialWindow -> Steam 0x00740a20  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x50*} | Steam {0x4*,0x50*}
imm sets (* = in both, 1 common): GOG {0x4*,0x6,0x20,0x7f000000} | Steam {0x4*,0xc}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751fa0: 16-byte aligned: YES
  00751fa0  55             PUSH EBP
  00751fa1  8B EC          MOV EBP,ESP
  00751fa3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751fa0
string literals (0):
direct callees (2):
  0x0073F750  ClientApp::GetGuiInGame
  0x007CD510  CGuiInGame::ShowSoloModeQuery
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00751fa0 FUN_00751fa0 size=59 ret=4 callers=0 callees=2

void __thiscall FUN_00751fa0(undefined4 param_1,int param_2)

{
  undefined4 uVar1;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    uVar1 = 0;
    FUN_0073f750(0,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
    FUN_007cd510(uVar1);
  }
  return;
}
```

### decoy: 0x00751ef0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751ef0: 16-byte aligned: YES
  00751ef0  55             PUSH EBP
  00751ef1  8B EC          MOV EBP,ESP
  00751ef3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751ef0
string literals (0):
direct callees (5):
  0x00740330  CClientExoApp::GetPausedByCombat
  0x00740350  CClientExoApp::SetPausedByCombat
  0x00740960  CClientExoApp::GetAutoPaused
  0x00740930  CClientExoApp::GetClientRangedAnimation
  0x00740A20  CClientExoApp::ShowTutorialWindow
struct offsets touched (top 2):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B11b-1-37

GOG function: CGuiInGame::HideGalaxyMapGui  gog 0x004db020  callconv=(blank)  param_size_bytes=0  gog_size=224  gog_retn=None
Candidate Steam address: 0x007ca920 (size 239, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007ca730:STR0/CALLEE2/SIG0.312/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dae30 CGuiInGame::ShowGalaxyMapGui -> 0x007ca730  gate_class=AUTO
- GOG-order neighbour after:  0x004db100 CGuiInGame::ShowDebugMenu -> 0x007cac10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d7f0 CClientExoApp::GetPausedByCombat -> Steam 0x00740330  in candidate callees: yes
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam 0x00740350  in candidate callees: yes
  0x004da340 CGuiInGame::SetSWGuiStatus -> Steam ?  in candidate callees: n/a
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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

### decoy: 0x007ca730 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ca730: 16-byte aligned: YES
  007ca730  55             PUSH EBP
  007ca731  8B EC          MOV EBP,ESP
  007ca733  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007ca730
string literals (0):
direct callees (17):
  0x0073F750  ClientApp::GetGuiInGame
  0x00740330  CClientExoApp::GetPausedByCombat
  0x00740350  CClientExoApp::SetPausedByCombat
  0x007401F0  CClientExoApp::IndicateObject
  0x0070BC60  (unnamed)
  0x007C9C40  CGuiInGame::SetSWGuiStatus
  0x00710840  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x008BDF50  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0073FEE0  CClientExoApp::SetInputClass
  0x00919723  operator new
  0x008973D0  (unnamed)
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FED90  CSWPartyTable::SetSelectedPlanet
  0x00410530  CSWGuiManager::AddPanel
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 8):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x38 [dec 56]  x3  via EAX,ECX,EDX
  +0x80 [dec 128]  x3  via EAX,ECX
  +0x8 [dec 8]  x1  via EAX
  +0x30 [dec 48]  x1  via ECX
  +0x4C [dec 76]  x1  via EAX
  +0xCC [dec 204]  x1  via ECX
  +0x128 [dec 296]  x1  via EAX
```

## Card B11b-1-38

GOG function: CGuiInGame::AppendToCombatBuffer  gog 0x004ce840  callconv=(blank)  param_size_bytes=9  gog_size=288  gog_retn=12
Candidate Steam address: 0x007be1b0 (size 288, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=FeedbackList::Add | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007be090:STR0/CALLEE1/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ce720 CGuiInGame::AppendToMsgBuffer -> 0x007be090  gate_class=AUTO
- GOG-order neighbour after:  0x004ce960 CGuiInGame::AppendToDialogBuffer -> 0x007be2d0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605c70 CExoString::operator!=_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00701d10 CSWPartyTable::LoadTableInfo -> Steam 0x005fbf60  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0xc*,0x118*,0x124*} | Steam {0x8*,0xc*,0x118*,0x124*}
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
prologue for 0x007be1b0: 16-byte aligned: YES
  007be1b0  55             PUSH EBP
  007be1b1  8B EC          MOV EBP,ESP
  007be1b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007be1b0 FUN_007be1b0 size=288 ret=12 callers=2 callees=2

void __thiscall FUN_007be1b0(int param_1,undefined4 param_2,undefined4 param_3,undefined1 param_4)

{
  int iVar1;
  int iVar2;
  uint local_8;
  
  iVar1 = FUN_00733b60(&DAT_00986925);
  if (iVar1 != 0) {
    if (0x3f < *(uint *)(param_1 + 0x124)) {
      *(undefined4 *)(param_1 + 0x124) = 0x3f;
      for (local_8 = 0; local_8 < *(uint *)(param_1 + 0x124); local_8 = local_8 + 1) {
        iVar2 = (local_8 + 1) * 0x10 + *(int *)(param_1 + 0x118);
        iVar1 = local_8 * 0x10 + *(int *)(param_1 + 0x118);
        FUN_007337c0(iVar2);
        *(undefined4 *)(iVar1 + 8) = *(undefined4 *)(iVar2 + 8);
        *(undefined1 *)(iVar1 + 0xc) = *(undefined1 *)(iVar2 + 0xc);
      }
    }
    FUN_007337c0(param_2);
    *(undefined4 *)(*(int *)(param_1 + 0x118) + 8 + *(int *)(param_1 + 0x124) * 0x10) = param_3;
    *(undefined1 *)(*(int *)(param_1 + 0x118) + 0xc + *(int *)(param_1 + 0x124) * 0x10) = param_4;
    *(int *)(param_1 + 0x124) = *(int *)(param_1 + 0x124) + 1;
  }
  return;
}
```

### decoy: 0x007be090 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007be090: 16-byte aligned: YES
  007be090  55             PUSH EBP
  007be091  8B EC          MOV EBP,ESP
  007be093  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11b-1-39

GOG function: CGuiInGame::GetMessageBuffer  gog 0x004d8410  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x007c7d20 (size 20, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007c7d40:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004d7720 CGuiInGame::ShutDown -> 0x007c7030  gate_class=AUTO
- GOG-order neighbour after:  0x004d8430 CGuiInGame::GetMessageBufferSize -> 0x007c7d40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00700d10 CSWPartyTable::SaveTableInfo -> Steam 0x005fb1a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x110*} | Steam {0x110*}
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
prologue for 0x007c7d20: 16-byte aligned: YES
  007c7d20  55             PUSH EBP
  007c7d21  8B EC          MOV EBP,ESP
  007c7d23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007c7d20
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x110 [dec 272]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007c7d20 FUN_007c7d20 size=20 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_007c7d20(int param_1)

{
  return *(undefined4 *)(param_1 + 0x110);
}
```

### decoy: 0x007c7d40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007c7d40: 16-byte aligned: YES
  007c7d40  55             PUSH EBP
  007c7d41  8B EC          MOV EBP,ESP
  007c7d43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007c7d40
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x11C [dec 284]  x1  via EAX
```

## Card B11b-1-40

GOG function: CGuiInGame::GetMessageBufferSize  gog 0x004d8430  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x007c7d40 (size 20, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007c7d20:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004d8410 CGuiInGame::GetMessageBuffer -> 0x007c7d20  gate_class=REVIEW
- GOG-order neighbour after:  0x004d8450 CGuiInGame::HandleDialogSelection -> 0x007c7d60  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00700d10 CSWPartyTable::SaveTableInfo -> Steam 0x005fb1a0  in candidate callers: yes
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

## Card B11b-1-41

GOG function: CGuiInGame::ClearMapDisplays  gog 0x004ce580  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x007bdbc0 (size 702, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bdb10:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ce500 CSWGuiControl::Destructor -> 0x00418640  trusted
- GOG-order neighbour after:  0x004ce5a0 CSWGuiObject::`scalar  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x110,0x114,0x118,0x11c,0xf20,0xfbc}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x4c,0xef8,0xf24,0xf28,0xf2c,0xf38,0x1030,...(+2)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007bdbc0: 16-byte aligned: YES
  007bdbc0  55             PUSH EBP
  007bdbc1  8B EC          MOV EBP,ESP
  007bdbc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007bdbc0
string literals (0):
direct callees (14):
  0x007C7030  CGuiInGame::ShutDown
  0x007C7B40  (unnamed)
  0x007BDB10  CGuiInGame::ShutDownGlobal
  0x00401D20  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x009196FD  operator delete
  0x0070B080  StreamingSource scalar delete
  0x0070B2F0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x007D2320  (unnamed)
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0072F5F0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 6):
  +0xF20 [dec 3872]  x4  via EAX,ECX,EDX
  +0xFBC [dec 4028]  x3  via EAX,EDX
  +0x110 [dec 272]  x2  via ECX,EDX
  +0x114 [dec 276]  x2  via EAX,ECX
  +0x118 [dec 280]  x2  via EAX,EDX
  +0x11C [dec 284]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007bdbc0 FUN_007bdbc0 size=702 ret=0 callers=1 callees=14

void __fastcall FUN_007bdbc0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_009696fa;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 7;
  FUN_007c7030(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  FUN_007c7b40();
  if ((*(int *)(param_1 + 0x114) != 0) && (*(int *)(param_1 + 0x114) != 0)) {
    FUN_007bdb10(3);
  }
  if (*(int *)(param_1 + 0x110) != 0) {
    for (local_14 = 0; local_14 < *(uint *)(param_1 + 0x11c); local_14 = local_14 + 1) {
    }
    if (*(int *)(param_1 + 0x110) != 0) {
      FID_conflict__vector_deleting_destructor_ [GHIDRA-AUTO-LABEL: unreliable](3);
    }
  }
  if ((*(int *)(param_1 + 0x118) != 0) && (*(int *)(param_1 + 0x118) != 0)) {
    FID_conflict__vector_deleting_destructor_ [GHIDRA-AUTO-LABEL: unreliable](3);
  }
  while (iVar1 = FUN_007a1780(), iVar1 == 0) {
    uVar2 = FUN_007f2a90();
    FUN_009196fd(uVar2);
  }
  if (*(int *)(param_1 + 0xfbc) != 0) {
    iVar1 = *(int *)(param_1 + 0xfbc);
    if (iVar1 != 0) {
      FUN_0070b080();
      FUN_009196fd(iVar1);
    }
    *(undefined4 *)(param_1 + 0xfbc) = 0;
  }
  if (*(int *)(param_1 + 0xf20) != 0) {
    FUN_0070b2f0(0);
    iVar1 = *(int *)(param_1 + 0xf20);
    if (iVar1 != 0) {
      FUN_0070b080();
      FUN_009196fd(iVar1);
    }
    *(undefined4 *)(param_1 + 0xf20) = 0;
  }
  local_8._0_1_ = 6;
  Free();
  local_8._0_1_ = 5;
  Free();
  local_8._0_1_ = 4;
  FUN_007d2320();
  local_8._0_1_ = 3;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]((void *)(param_1 + 0xf38),8,8,FUN_00733780);
  local_8._0_1_ = 2;
  FUN_0072f5f0();
  local_8._0_1_ = 1;
  FUN_0072f5f0();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_0072f5f0();
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007bdb10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bdb10: 16-byte aligned: YES
  007bdb10  55             PUSH EBP
  007bdb11  8B EC          MOV EBP,ESP
  007bdb13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007bdb10
string literals (0):
direct callees (3):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B11b-1-42

GOG function: CGuiInGame::IsLoopingDialogAnimation  gog 0x004db920  callconv=(blank)  param_size_bytes=2  gog_size=45  gog_retn=4
Candidate Steam address: 0x007cb220 (size 45, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x004db950:0.74>0.04
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007cb1f0:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004db8f0 CGuiInGame::IsLoopingDialogStuntAnimation -> 0x007cb1f0  gate_class=REVIEW
- GOG-order neighbour after:  0x004db950 CGuiInGame::IsOverlayDialogAnimation -> 0x007cb250  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x3e8*,0x52f*} | Steam {0x4*,0x3e8*,0x52f*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cb220: 16-byte aligned: YES
  007cb220  55             PUSH EBP
  007cb221  8B EC          MOV EBP,ESP
  007cb223  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb220
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007cb220 FUN_007cb220 size=45 ret=4 callers=2 callees=0

undefined4 FUN_007cb220(ushort param_1)

{
  undefined4 uVar1;
  
  if ((param_1 < 1000) || (0x52f < param_1)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x007cb1f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb1f0: 16-byte aligned: YES
  007cb1f0  55             PUSH EBP
  007cb1f1  8B EC          MOV EBP,ESP
  007cb1f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb1f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B11b-1-43

GOG function: CGuiInGame::SetTutorialReason  gog 0x004e00e0  callconv=(blank)  param_size_bytes=1  gog_size=48  gog_retn=4
Candidate Steam address: 0x007cf960 (size 117, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.38 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cf9e0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0060 CGuiInGame::TutorialReasonWillShowWindow -> 0x007cf960  gate_class=AUTO
- GOG-order neighbour after:  0x004e0110 CGuiInGame::TutorialReasonHasShown -> 0x007cfa10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00586a00 CSWGuiTutorialBox::SetTutorialReason -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0044afd0 CClientExoAppInternal::ShowTutorialWindow -> Steam 0x0079e9d0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xb0*} | Steam {0x4,0x1c,0xb0*,0xcc}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x10}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cf960: 16-byte aligned: YES
  007cf960  55             PUSH EBP
  007cf961  8B EC          MOV EBP,ESP
  007cf963  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cf960
string literals (0):
direct callees (3):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007CFA10  (unnamed)
  0x00898EB0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x1C [dec 28]  x1  via EDX
  +0xB0 [dec 176]  x1  via EDX
  +0xCC [dec 204]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007cf960 FUN_007cf960 size=117 ret=4 callers=1 callees=3

undefined4 __thiscall FUN_007cf960(int param_1,undefined1 param_2)

{
  int iVar1;
  
  iVar1 = FUN_0072fb00();
  if (((((*(uint *)(iVar1 + 0x1c) >> 1 & 1) != 0) && (iVar1 = FUN_007cfa10(param_2), iVar1 == 0)) &&
      (*(int *)(param_1 + 0xcc) == 0)) && (iVar1 = FUN_00898eb0(param_2), iVar1 != 0)) {
    return 1;
  }
  return 0;
}
```

### decoy: 0x007cf9e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cf9e0: 16-byte aligned: YES
  007cf9e0  55             PUSH EBP
  007cf9e1  8B EC          MOV EBP,ESP
  007cf9e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cf9e0
string literals (0):
direct callees (1):
  0x00898F60  (unnamed)
struct offsets touched (top 1):
  +0xB0 [dec 176]  x2  via EAX,EDX
```

## Card B11b-1-44

GOG function: CGuiInGame::IsFireForgetDialogAnimation  gog 0x004db710  callconv=(blank)  param_size_bytes=2  gog_size=423  gog_retn=4
Candidate Steam address: 0x007cae60 (size 423, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb010:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004db4b0 CGuiInGame::DestroyDeathMenuGUI -> 0x007cadb0  gate_class=REVIEW
- GOG-order neighbour after:  0x004db8c0 CGuiInGame::IsDialogAnimationRange1000_1727 -> 0x007cb1c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x004db920 CGuiInGame::IsLoopingDialogAnimation -> Steam ?  in candidate callees: n/a
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x004748e0 CSWCObject::AnimationDialog -> Steam 0x007ef780  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x24*,0xcc*,0xf0*,0x130*} | Steam {0x24*,0xcc*,0xf0*,0x130*}
imm sets (* = in both, 3 common): GOG {0x4*,0x44*,0x2710*} | Steam {0x4*,0x44*,0x2710*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cae60: 16-byte aligned: YES
  007cae60  55             PUSH EBP
  007cae61  8B EC          MOV EBP,ESP
  007cae63  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cae60
string literals (0):
direct callees (2):
  0x0071B8B0  (unnamed)
  0x007CB1F0  (unnamed)
struct offsets touched (top 4):
  +0x130 [dec 304]  x5  via EAX,ECX,EDX
  +0xF0 [dec 240]  x3  via EAX,ECX
  +0xCC [dec 204]  x2  via ECX,EDX
  +0x24 [dec 36]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007cae60 FUN_007cae60 size=423 ret=4 callers=3 callees=2

undefined4 FUN_007cae60(ushort param_1)

{
  int iVar1;
  undefined4 uVar2;
  int local_18;
  int local_14;
  int local_10;
  short local_c;
  int local_8;
  
  if ((param_1 < 10000) ||
     (*(int *)(*(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0xf0) + 0x24) + 0x2710 [dec 10000] <= (int)(uint)param_1
     )) {
    iVar1 = FUN_0071b8b0(param_1,DAT_00a0fe24,&local_14);
    if (iVar1 == 0) {
      uVar2 = FUN_007cb1f0(param_1);
    }
    else {
      iVar1 = FUN_0071b8b0(param_1,DAT_00a0fe40,&local_18);
      if (iVar1 == 0) {
        uVar2 = FUN_007cb1f0(param_1);
      }
      else if ((local_14 == 0) || (local_18 == 0)) {
        uVar2 = FUN_007cb1f0(param_1);
      }
      else {
        uVar2 = 1;
      }
    }
    return uVar2;
  }
  local_c = param_1 + 0xd8f0;
  iVar1 = FUN_0071b8b0(local_c,DAT_00a0fe64,&local_8);
  if (iVar1 == 0) {
    return 0;
  }
  iVar1 = FUN_0071b8b0(local_c,DAT_00a0fe60,&local_10);
  if (iVar1 == 0) {
    return 0;
  }
  if ((local_8 != 0) && (local_10 != 0)) {
    return 1;
  }
  return 0;
}
```

### decoy: 0x007cb010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb010: 16-byte aligned: YES
  007cb010  55             PUSH EBP
  007cb011  8B EC          MOV EBP,ESP
  007cb013  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb010
string literals (0):
direct callees (2):
  0x0071B8B0  (unnamed)
  0x007CB220  (unnamed)
struct offsets touched (top 4):
  +0x130 [dec 304]  x5  via EAX,ECX,EDX
  +0xF0 [dec 240]  x3  via EAX,ECX
  +0xCC [dec 204]  x2  via ECX,EDX
  +0x24 [dec 36]  x1  via EDX
```

## Card B11b-1-45

GOG function: CSWGuiMainInterface::UpdateActionMenuSelected  gog 0x00523370  callconv=(blank)  param_size_bytes=4  gog_size=54  gog_retn=4
Candidate Steam address: 0x00751c30 (size 170, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: rotation:0x00523670:0.52>0.08
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.26 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00751bf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523280 CSWGuiMainInterface::GetActiveActionIdx -> 0x00751b00  gate_class=REVIEW
- GOG-order neighbour after:  0x005233b0 CSWGuiActionMenuIos::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x50*,0x5c*,0x1c77} | Steam {0x4,0x1c,0x50*,0x5c*}
imm sets (* = in both, 1 common): GOG {0x4*,0x6,0xc} | Steam {0x4*,0x5,0x7,0xf,0x1c,0x7f000000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751c30: 16-byte aligned: YES
  00751c30  55             PUSH EBP
  00751c31  8B EC          MOV EBP,ESP
  00751c33  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751c30
string literals (0):
direct callees (5):
  0x00754E20  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x00740930  CClientExoApp::GetClientRangedAnimation
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x007525D0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x1C [dec 28]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EAX
  +0x15414 [dec 87060]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00751c30 FUN_00751c30 size=170 ret=4 callers=0 callees=5

void __thiscall FUN_00751c30(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    FUN_00754e20();
    *(undefined4 *)(param_1 + 0x15414) = 1;
    uVar1 = *(undefined4 *)(param_2 + 0x5c);
    iVar2 = FUN_0072fb00();
    if ((*(uint *)(iVar2 + 0x1c) >> 0xf & 1) != 0) {
      FUN_00740930(1,7);
    }
    FUN_00740a20(5,0x7f000000,0x7f000000,0);
    FUN_007525d0(uVar1);
  }
  return;
}
```

### decoy: 0x00751bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751bf0: 16-byte aligned: YES
  00751bf0  55             PUSH EBP
  00751bf1  8B EC          MOV EBP,ESP
  00751bf3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751bf0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
  +0x1C77 [dec 7287]  x1  via EDX
```

## Card B11b-1-46

GOG function: CGuiInGame::SetTutorialReasonTracked  gog 0x004e0170  callconv=(blank)  param_size_bytes=8  gog_size=154  gog_retn=8
Candidate Steam address: 0x007cfa70 (size 154, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cfa10:STR0/CALLEE0/SIG0.9/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0110 CGuiInGame::TutorialReasonHasShown -> 0x007cfa10  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0210 CGuiInGame::ShowTutorialWindow -> 0x007cfb10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xf88*} | Steam {0xf88*}
imm sets (* = in both, 4 common): GOG {0x3*,0x7*,0x8*,0x100*} | Steam {0x3*,0x7*,0x8*,0x100*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007cfa70: 16-byte aligned: YES
  007cfa70  55             PUSH EBP
  007cfa71  8B EC          MOV EBP,ESP
  007cfa73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cfa70
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007cfa70 FUN_007cfa70 size=154 ret=8 callers=1 callees=0

void __thiscall FUN_007cfa70(int param_1,byte param_2,int param_3)

{
  int iVar1;
  
  if (param_2 < 0x100) {
    iVar1 = (int)(uint)param_2 >> 3;
    param_2 = param_2 + (char)iVar1 * -8;
    if (param_3 == 0) {
      *(byte *)(param_1 + 0xf88 + iVar1) =
           *(byte *)(param_1 + 0xf88 + iVar1) & ~(byte)(1 << (param_2 & 0x1f));
    }
    else {
      *(byte *)(param_1 + 0xf88 + iVar1) =
           *(byte *)(param_1 + 0xf88 + iVar1) | (byte)(1 << (param_2 & 0x1f));
    }
  }
  return;
}
```

### decoy: 0x007cfa10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cfa10: 16-byte aligned: YES
  007cfa10  55             PUSH EBP
  007cfa11  8B EC          MOV EBP,ESP
  007cfa13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cfa10
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B11b-1-47

GOG function: CGuiInGame::ShowDialogMessage  gog 0x004e0bf0  callconv=(blank)  param_size_bytes=0  gog_size=50  gog_retn=0
Candidate Steam address: 0x007d0560 (size 49, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::RemoveGUIPopUps | G7 mac: pass(1.00) | G8 mac_rotation: rotation:0x004e0c60:0.92>0.17
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d0530:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0b70 CGuiInGame::ClearDialogNonDependantList -> 0x007d0470  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0c30 CGuiInGame::SetCombatMessage -> 0x007d0530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x98,0xa900} | Steam {0x3c,0x40,0x84}
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

## Card B11b-1-48

GOG function: CGuiInGame::HideUpgradeScreen  gog 0x004de2e0  callconv=(blank)  param_size_bytes=0  gog_size=217  gog_retn=0
Candidate Steam address: 0x007cdbe0 (size 217, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cdcc0:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004de1a0 CGuiInGame::ShowChemicalUpgradeScreen -> 0x007cdaa0  gate_class=AUTO
- GOG-order neighbour after:  0x004de3c0 CGuiInGame::HideChemicalUpgradeScreen -> 0x007cdcc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040ccd0 CClientExoApp::GetSWCMessage -> Steam 0x0073f810  in candidate callees: yes
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x00506010 CSWCMessage::SendPlayerToServerInput_TogglePauseRequest -> Steam ?  in candidate callees: n/a
  0x0063ac00 CExoSound::SetSoundMode -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
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

## Card B11b-1-49

GOG function: CGuiInGame::SetMainInterfaceTarget  gog 0x004dee10  callconv=(blank)  param_size_bytes=8  gog_size=47  gog_retn=8
Candidate Steam address: 0x007ce710 (size 47, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ce740:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004deae0 CGuiInGame::SetShotFromScript -> 0x007ce3e0  gate_class=AUTO
- GOG-order neighbour after:  0x004dee40 CGuiInGame::SetAreaTransition -> 0x007ce740  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0051e730 CSWGuiMainInterface::UpdateActionQueue -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x98*} | Steam {0x98*}
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
prologue for 0x007ce710: 16-byte aligned: YES
  007ce710  55             PUSH EBP
  007ce711  8B EC          MOV EBP,ESP
  007ce713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ce710
string literals (0):
direct callees (1):
  0x0074CFB0  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ce710 FUN_007ce710 size=47 ret=8 callers=1 callees=1

void __thiscall FUN_007ce710(int param_1,undefined4 param_2,undefined4 param_3)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    FUN_0074cfb0(param_2,param_3);
  }
  return;
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

## Card B11b-1-50

GOG function: CGuiInGame::IsCameraDialog  gog 0x004dbda0  callconv=(blank)  param_size_bytes=16  gog_size=55  gog_retn=16
Candidate Steam address: 0x007cb6a0 (size 55, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x004dbe50:0.91>0.12
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: SetFade 0x8bb0e0)
- best decoy (neighbour/other candidate) scores: 0x007cb5e0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbce0 CGuiInGame::UnlockDialogParticipantOrientations -> 0x007cb5e0  gate_class=AUTO
- GOG-order neighbour after:  0x004dbde0 CGuiInGame::IsAnimationPlayingInDialog -> 0x007cb6e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005af830 CSWGuiDialogLetterbox::SetFade -> Steam 0x008bb0e0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
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
  0x008BB0E0  CSWGuiDialogLetterbox::SetFade
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

### decoy: 0x007cb5e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb5e0: 16-byte aligned: YES
  007cb5e0  55             PUSH EBP
  007cb5e1  8B EC          MOV EBP,ESP
  007cb5e3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb5e0
string literals (0):
direct callees (2):
  0x0073F4D0  CClientExoApp::GetGameObject
  0x0077FC10  CSWCCreature::SetLockOrientationToObject
struct offsets touched (top 4):
  +0x2C [dec 44]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x190 [dec 400]  x1  via EAX
  +0x194 [dec 404]  x1  via ECX
```

## Card B11b-1-51

GOG function: CGuiInGame::IsOverlayDialogAnimation_2  gog 0x004dfe10  callconv=(blank)  param_size_bytes=4  gog_size=112  gog_retn=4
Candidate Steam address: 0x007cf4b0 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cf710:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dfcd0 CGuiInGame::SetLockObjectOrientationInDialog -> 0x007cf5d0  gate_class=AUTO
- GOG-order neighbour after:  0x004dfe80 CGuiInGame::SetLockHeadFollowInDialog -> 0x007cf780  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x98}
imm sets (* = in both, 1 common): GOG {0x4*,0xc,0xf24,0x7f000000} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cf4b0: 16-byte aligned: YES
  007cf4b0  55             PUSH EBP
  007cf4b1  8B EC          MOV EBP,ESP
  007cf4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cf4b0
string literals (0):
direct callees (1):
  0x00750E90  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007cf4b0 FUN_007cf4b0 size=44 ret=4 callers=2 callees=1

void __thiscall FUN_007cf4b0(int param_1,undefined1 param_2)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    FUN_00750e90(param_2);
  }
  return;
}
```

### decoy: 0x007cf710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cf710: 16-byte aligned: YES
  007cf710  55             PUSH EBP
  007cf711  8B EC          MOV EBP,ESP
  007cf713  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cf710
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

## Card B11b-1-52

GOG function: CGuiInGame::ShowDialogEntry  gog 0x004d01a0  callconv=(blank)  param_size_bytes=8  gog_size=428  gog_retn=12
Candidate Steam address: 0x007bfb10 (size 428, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::ShowDialogPanels | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 5 (callees: ResetDriveAcceleration 0x740240, HideMouse 0x7b38d0, SetTop 0x8baca0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007bfa50:STR0/CALLEE0/SIG0.205/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004d00e0 CGuiInGame::SetConversationType -> 0x007bfa50  gate_class=REVIEW
- GOG-order neighbour after:  0x004d0350 CGuiInGame::InitializeStuntModelData -> 0x007bfcc0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d700 CClientExoApp::ResetDriveAcceleration -> Steam 0x00740240  in candidate callees: yes
  0x004604e0 CClientExoAppInternal::HideMouse -> Steam 0x007b38d0  in candidate callees: yes
  0x004da340 CGuiInGame::SetSWGuiStatus -> Steam ?  in candidate callees: n/a
  0x005af3f0 CSWGuiDialogLetterbox::SetTop -> Steam 0x008baca0  in candidate callees: yes
  0x005af4f0 CSWGuiDialogLetterbox::SetBottom -> Steam 0x008bada0  in candidate callees: yes
  0x005af660 CSWGuiDialogLetterbox::SetMiddle -> Steam 0x008baf10  in candidate callees: yes
  0x005b2e00 CSWGuiBarkBubble::StopBark -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 14 common): GOG {0x4*,0x38*,0x3c*,0x40*,0x44*,0x4c*,0x60*,0x64*,0x68*,0x74*,0x7c*,0x80*,0xc8*,0x134*} | Steam {0x4*,0x38*,0x3c*,0x40*,0x44*,0x4c*,0x60*,0x64*,0x68*,0x74*,0x7c*,0x80*,0xc8*,0x134*}
imm sets (* = in both, 5 common): GOG {0x2*,0x3*,0xc*,0x10*,0x18*} | Steam {0x2*,0x3*,0xc*,0x10*,0x18*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x007bfb10: 16-byte aligned: YES
  007bfb10  55             PUSH EBP
  007bfb11  8B EC          MOV EBP,ESP
  007bfb13  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bfb10
string literals (0):
direct callees (10):
  0x007B38D0  CClientExoAppInternal::HideMouse
  0x0073FEE0  CClientExoApp::SetInputClass
  0x00740240  CClientExoApp::ResetDriveAcceleration
  0x008BE6A0  (unnamed)
  0x00410800  GuiManager::Contains
  0x008BAF10  CSWGuiDialogLetterbox::SetMiddle
  0x00410530  CSWGuiManager::AddPanel
  0x008BACA0  CSWGuiDialogLetterbox::SetTop
  0x008BADA0  CSWGuiDialogLetterbox::SetBottom
  0x007C9C40  CGuiInGame::SetSWGuiStatus
struct offsets touched (top 10):
  +0x3C [dec 60]  x10  via EAX,ECX,EDX
  +0x38 [dec 56]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x60 [dec 96]  x2  via EAX,EDX
  +0x64 [dec 100]  x2  via EAX,ECX
  +0x68 [dec 104]  x2  via ECX,EDX
  +0xC8 [dec 200]  x2  via ECX,EDX
  +0x40 [dec 64]  x1  via EDX
  +0x44 [dec 68]  x1  via EDX
  +0x4C [dec 76]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007bfb10 FUN_007bfb10 size=428 ret=12 callers=1 callees=10

void __thiscall FUN_007bfb10(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  if (*(uint *)(param_1 + 0x134) < 2) {
    FUN_007b38d0(0x10);
  }
  if (*(int *)(param_1 + 0xc8 [dec 200]) == 0) {
    *(undefined4 *)(param_1 + 0xc8 [dec 200]) = 1;
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    FUN_0073fee0(3,1);
    FUN_00740240(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4),uVar2);
    FUN_008be6a0(1);
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x3c) + 0x74))();
  }
  (**(code **)(**(int **)(param_1 + 0x3c) + 0x7c))(param_3);
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0x3c))
  ;
  if (iVar1 == 0) {
    local_8 = 0;
    if (*(int *)(param_1 + 0x3c) == *(int *)(param_1 + 0x40)) {
      if (DAT_00a7fd30 == 0) {
        FUN_008baf10();
        FUN_00410530(*(undefined4 *)(param_1 + 0x68),0,1);
        FUN_008baca0();
        FUN_00410530(*(undefined4 *)(param_1 + 0x60),0,1);
        FUN_008bada0();
        FUN_00410530(*(undefined4 *)(param_1 + 0x64 [dec 100]),0,1);
      }
    }
    else if (*(int *)(param_1 + 0x3c) == *(int *)(param_1 + 0x44)) {
      local_8 = 2;
    }
    FUN_00410530(*(undefined4 *)(param_1 + 0x3c),local_8,1);
    FUN_007c9c40(3,1);
  }
  (**(code **)(**(int **)(param_1 + 0x3c) + 0x80))(param_2);
  return;
}
```

### decoy: 0x007bfa50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bfa50: 16-byte aligned: YES
  007bfa50  55             PUSH EBP
  007bfa51  8B EC          MOV EBP,ESP
  007bfa53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007bfa50
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x008BC620  (unnamed)
  0x008BD7A0  (unnamed)
struct offsets touched (top 4):
  +0x44 [dec 68]  x4  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x38 [dec 56]  x1  via ECX
  +0x40 [dec 64]  x1  via EAX
```

## Card B11b-1-53

GOG function: CGuiInGame::SetupPauseGuiExtent  gog 0x004e09d0  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x007d02d0 (size 31, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d02b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e09b0 CGuiInGame::ChangeCharacter -> 0x007d02b0  gate_class=REVIEW
- GOG-order neighbour after:  0x004e09f0 CGuiInGame::AddToDialogNonDependantList -> 0x007d02f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005246d0 CSWGuiMainInterface::SetupPauseGuiExtent -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x98*} | Steam {0x98*}
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
prologue for 0x007d02d0: 16-byte aligned: YES
  007d02d0  55             PUSH EBP
  007d02d1  8B EC          MOV EBP,ESP
  007d02d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d02d0
string literals (0):
direct callees (1):
  0x00752F50  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007d02d0 FUN_007d02d0 size=31 ret=4 callers=1 callees=1
THUNK -> 0x00752F50 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007d02d0(undefined4 param_1)

{
  FUN_00752f50(param_1);
  return;
}
```

### decoy: 0x007d02b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d02b0: 16-byte aligned: YES
  007d02b0  55             PUSH EBP
  007d02b1  8B EC          MOV EBP,ESP
  007d02b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d02b0
string literals (0):
direct callees (1):
  0x00750E60  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x1  via EAX
```

## Card B11b-1-54

GOG function: CGuiInGame::AddFloatyText  gog 0x004df580  callconv=(blank)  param_size_bytes=16  gog_size=58  gog_retn=16
Candidate Steam address: 0x007cee80 (size 58, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: AddFloatyText 0x79de60)
- best decoy (neighbour/other candidate) scores: 0x007ceec0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004df550 CGuiInGame::GetGuiInGameScreenPanel -> 0x007cee50  gate_class=AUTO
- GOG-order neighbour after:  0x004df5c0 CGuiInGame::InitializeMap -> 0x007ceec0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005226b0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0044a460 CClientExoAppInternal::AddFloatyText -> Steam 0x0079de60  in candidate callers: yes
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

## Card B11b-1-55

GOG function: CGuiInGame::IsGlobalFading  gog 0x004dc340  callconv=(blank)  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x007cbc40 (size 44, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cbc70:STR0/CALLEE2/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dc2f0 CGuiInGame::StopGlobalFade -> 0x007cbbf0  gate_class=AUTO
- GOG-order neighbour after:  0x004dc370 CGuiInGame::InitializePlaceableCameras -> 0x007cbc70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005ac690 CSWGuiFade::IsFadeDone -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x00449b50 CClientExoAppInternal::UpdateRumble -> Steam 0x0079d550  in candidate callers: yes
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

## Card B11b-1-56

GOG function: CGuiInGame::InitializeGlobal  gog 0x004ce110  callconv=(blank)  param_size_bytes=0  gog_size=515  gog_retn=0
Candidate Steam address: 0x007bdbc0 (size 702, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bde80:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004cde50 CGuiInGame::Destructor -> 0x00740070  gate_class=REJECT
- GOG-order neighbour after:  0x004ce320 CSWGuiPanel::GetActiveControl  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam 0x0073fea0  in candidate callees: no
  0x0052c540 CSWGuiMessageBox::Constructor -> Steam ?  in candidate callees: n/a
  0x0052c960 CSWGuiMessageBox::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00586770 CSWGuiTutorialBox::Constructor -> Steam ?  in candidate callees: n/a
  0x0058a090 CSWGuiSkillInfoBox::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0042df60 CClientExoAppInternal::Initialize -> Steam 0x007819e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x38,0xa0,0xa4,0xac,0xb0,0xb4} | Steam {0x110,0x114,0x118,0x11c,0xf20,0xfbc}
imm sets (* = in both, 3 common): GOG {0x2*,0x3*,0x4*,0x48,0x9c8,0x9dc,0x4598} | Steam {0x2*,0x3*,0x4*,0x5,0x6,0x7,0x8,0x4c,0xef8,0xf24,0xf28,0xf2c,0xf38,0x1030,...(+2)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007bdbc0: 16-byte aligned: YES
  007bdbc0  55             PUSH EBP
  007bdbc1  8B EC          MOV EBP,ESP
  007bdbc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007bdbc0
string literals (0):
direct callees (14):
  0x007C7030  CGuiInGame::ShutDown
  0x007C7B40  (unnamed)
  0x007BDB10  CGuiInGame::ShutDownGlobal
  0x00401D20  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x009196FD  operator delete
  0x0070B080  StreamingSource scalar delete
  0x0070B2F0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x007D2320  (unnamed)
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0072F5F0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 6):
  +0xF20 [dec 3872]  x4  via EAX,ECX,EDX
  +0xFBC [dec 4028]  x3  via EAX,EDX
  +0x110 [dec 272]  x2  via ECX,EDX
  +0x114 [dec 276]  x2  via EAX,ECX
  +0x118 [dec 280]  x2  via EAX,EDX
  +0x11C [dec 284]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007bdbc0 FUN_007bdbc0 size=702 ret=0 callers=1 callees=14

void __fastcall FUN_007bdbc0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_009696fa;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 7;
  FUN_007c7030(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  FUN_007c7b40();
  if ((*(int *)(param_1 + 0x114) != 0) && (*(int *)(param_1 + 0x114) != 0)) {
    FUN_007bdb10(3);
  }
  if (*(int *)(param_1 + 0x110) != 0) {
    for (local_14 = 0; local_14 < *(uint *)(param_1 + 0x11c); local_14 = local_14 + 1) {
    }
    if (*(int *)(param_1 + 0x110) != 0) {
      FID_conflict__vector_deleting_destructor_ [GHIDRA-AUTO-LABEL: unreliable](3);
    }
  }
  if ((*(int *)(param_1 + 0x118) != 0) && (*(int *)(param_1 + 0x118) != 0)) {
    FID_conflict__vector_deleting_destructor_ [GHIDRA-AUTO-LABEL: unreliable](3);
  }
  while (iVar1 = FUN_007a1780(), iVar1 == 0) {
    uVar2 = FUN_007f2a90();
    FUN_009196fd(uVar2);
  }
  if (*(int *)(param_1 + 0xfbc) != 0) {
    iVar1 = *(int *)(param_1 + 0xfbc);
    if (iVar1 != 0) {
      FUN_0070b080();
      FUN_009196fd(iVar1);
    }
    *(undefined4 *)(param_1 + 0xfbc) = 0;
  }
  if (*(int *)(param_1 + 0xf20) != 0) {
    FUN_0070b2f0(0);
    iVar1 = *(int *)(param_1 + 0xf20);
    if (iVar1 != 0) {
      FUN_0070b080();
      FUN_009196fd(iVar1);
    }
    *(undefined4 *)(param_1 + 0xf20) = 0;
  }
  local_8._0_1_ = 6;
  Free();
  local_8._0_1_ = 5;
  Free();
  local_8._0_1_ = 4;
  FUN_007d2320();
  local_8._0_1_ = 3;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]((void *)(param_1 + 0xf38),8,8,FUN_00733780);
  local_8._0_1_ = 2;
  FUN_0072f5f0();
  local_8._0_1_ = 1;
  FUN_0072f5f0();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_0072f5f0();
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007bde80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bde80: 16-byte aligned: YES
  007bde80  55             PUSH EBP
  007bde81  8B EC          MOV EBP,ESP
  007bde83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007bde80
string literals (0):
direct callees (6):
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00919723  operator new
  0x0075AE40  CSWGuiMessageBox::Constructor
  0x0075B370  (unnamed)
  0x0089C5F0  (unnamed)
  0x00898CA0  (unnamed)
struct offsets touched (top 7):
  +0x38 [dec 56]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0xA0 [dec 160]  x1  via EDX
  +0xA4 [dec 164]  x1  via ECX
  +0xAC [dec 172]  x1  via EAX
  +0xB0 [dec 176]  x1  via EDX
  +0xB4 [dec 180]  x1  via ECX
```

## Card B11b-1-57

GOG function: CGuiInGame::DestroyDeathMenuGUI  gog 0x004db4b0  callconv=(blank)  param_size_bytes=0  gog_size=174  gog_retn=0
Candidate Steam address: 0x007cadb0 (size 174, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::IsAnimationPlayingInDialog | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: AnimationDialog 0x7ef780); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cad30:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004db430 CGuiInGame::HideContainerGui -> 0x007cad30  gate_class=AUTO
- GOG-order neighbour after:  0x004db710 CGuiInGame::IsFireForgetDialogAnimation -> 0x007cae60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: yes
  0x004748e0 CSWCObject::AnimationDialog -> Steam 0x007ef780  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x2c*,0x48*,0x1b8*,0x1c0*} | Steam {0x4*,0x2c*,0x48*,0x1b8*,0x1c0*}
imm sets (* = in both, 1 common): GOG {0x18*} | Steam {0x18*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007cadb0: 16-byte aligned: YES
  007cadb0  55             PUSH EBP
  007cadb1  8B EC          MOV EBP,ESP
  007cadb3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cadb0
string literals (0):
direct callees (2):
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007EF780  CSWCObject::AnimationDialog
struct offsets touched (top 5):
  +0x1B8 [dec 440]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
  +0x48 [dec 72]  x1  via ECX
  +0x1C0 [dec 448]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007cadb0 FUN_007cadb0 size=174 ret=0 callers=1 callees=2

int __fastcall FUN_007cadb0(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  int local_c;
  uint local_8;
  
  local_c = 0;
  local_8 = 0;
  while( true ) {
    if (*(uint *)(param_1 + 0x1b8) <= local_8) {
      if (local_8 == *(uint *)(param_1 + 0x1b8)) {
        local_c = 1;
      }
      return local_c;
    }
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    piVar1 = (int *)FUN_0073f4d0(*(undefined4 *)(*(int *)(param_1 + 0x1c0) + local_8 * 4));
    iVar2 = (**(code **)(*piVar1 + 0x2c))(piVar1,param_1,uVar3);
    if ((iVar2 != 0) && (local_c = FUN_007ef780(*(undefined2 *)(iVar2 + 0x48)), local_c == 0))
    break;
    local_8 = local_8 + 1;
  }
  return 0;
}
```

### decoy: 0x007cad30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cad30: 16-byte aligned: YES
  007cad30  55             PUSH EBP
  007cad31  8B EC          MOV EBP,ESP
  007cad33  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cad30
string literals (0):
direct callees (5):
  0x00410800  GuiManager::Contains
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00410670  (unnamed)
  0x0073FEE0  CClientExoApp::SetInputClass
  0x00740350  CClientExoApp::SetPausedByCombat
struct offsets touched (top 4):
  +0x38 [dec 56]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x54 [dec 84]  x2  via ECX,EDX
  +0x128 [dec 296]  x1  via EAX
```

## Card B11b-1-58

GOG function: CGuiInGame::IsLoopingDialogStuntAnimation_2  gog 0x004dbe20  callconv=(blank)  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x007cb5e0 (size 190, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::UnlockDialogParticipantOrientations | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb720:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbde0 CGuiInGame::IsAnimationPlayingInDialog -> 0x007cb6e0  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbe50 CGuiInGame::IsFadingInDialog -> 0x007cb720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005af950 CSWGuiDialogLetterbox::IsFadeDone -> Steam 0x008bb200  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x68} | Steam {0x4,0x2c,0x190,0x194}
imm sets (* = in both, 0 common): GOG {} | Steam {0x18,0x7f000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007cb5e0: 16-byte aligned: YES
  007cb5e0  55             PUSH EBP
  007cb5e1  8B EC          MOV EBP,ESP
  007cb5e3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb5e0
string literals (0):
direct callees (2):
  0x0073F4D0  CClientExoApp::GetGameObject
  0x0077FC10  CSWCCreature::SetLockOrientationToObject
struct offsets touched (top 4):
  +0x2C [dec 44]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x190 [dec 400]  x1  via EAX
  +0x194 [dec 404]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007cb5e0 FUN_007cb5e0 size=190 ret=0 callers=3 callees=2

void __fastcall FUN_007cb5e0(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  
  uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  piVar1 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x190 [dec 400]));
  if (piVar1 != (int *)0x0) {
    iVar2 = (**(code **)(*piVar1 + 0x2c))();
    if (iVar2 != 0) {
      uVar5 = 0;
      uVar3 = 0x7f000000;
      (**(code **)(*piVar1 + 0x2c))(0x7f000000,0);
      FUN_0077fc10(uVar3,uVar5);
    }
  }
  uVar3 = *(undefined4 *)(param_1 + 0x194);
  uVar5 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  piVar1 = (int *)FUN_0073f4d0(uVar3);
  if (piVar1 != (int *)0x0) {
    iVar2 = (**(code **)(*piVar1 + 0x2c))(param_1,uVar5,uVar3,uVar4);
    if (iVar2 != 0) {
      uVar3 = 0;
      uVar4 = 0x7f000000;
      (**(code **)(*piVar1 + 0x2c))(0x7f000000,0);
      FUN_0077fc10(uVar4,uVar3);
    }
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
  0x008BB200  CSWGuiDialogLetterbox::IsFadeDone
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX,ECX
```

## Card B11b-1-59

GOG function: CGuiInGame::SetLevelUpMode  gog 0x004d00b0  callconv=(blank)  param_size_bytes=4  gog_size=41  gog_retn=4
Candidate Steam address: 0x007bfa20 (size 41, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bfcc0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ceb50 CGuiInGame::Initialize -> 0x007be4c0  gate_class=AUTO
- GOG-order neighbour after:  0x004d00e0 CGuiInGame::SetConversationType -> 0x007bfa50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
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

## Card B11b-1-60

GOG function: CGuiInGame::SetDialogAnimationData  gog 0x004d06c0  callconv=(blank)  param_size_bytes=10  gog_size=89  gog_retn=12
Candidate Steam address: 0x007c0030 (size 89, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: rotation_in:0x004d05c0:0.96>0.76
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007bff30:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004d05c0 CGuiInGame::InitializeDialogAnimationData -> 0x007bff30  gate_class=REVIEW
- GOG-order neighbour after:  0x004d0720 CGuiInGame::ClearDialogAnimationData -> 0x007c0090  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x004dd7f0 CGuiInGame::UpdateDialog -> Steam 0x007cd0f0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1b8*,0x1c0*,0x1c4*} | Steam {0x1b8*,0x1c0*,0x1c4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 10; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(10/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 10 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x007c0030: 16-byte aligned: YES
  007c0030  55             PUSH EBP
  007c0031  8B EC          MOV EBP,ESP
  007c0033  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007c0030
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1C0 [dec 448]  x2  via ECX,EDX
  +0x1C4 [dec 452]  x2  via EAX,EDX
  +0x1B8 [dec 440]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007c0030 FUN_007c0030 size=89 ret=12 callers=3 callees=0

void __thiscall FUN_007c0030(int param_1,uint param_2,undefined4 param_3,undefined2 param_4)

{
  if (((param_2 < *(uint *)(param_1 + 0x1b8)) && (*(int *)(param_1 + 0x1c0) != 0)) &&
     (*(int *)(param_1 + 0x1c4) != 0)) {
    *(undefined4 *)(*(int *)(param_1 + 0x1c0) + param_2 * 4) = param_3;
    *(undefined2 *)(*(int *)(param_1 + 0x1c4) + param_2 * 2) = param_4;
  }
  return;
}
```

### decoy: 0x007bff30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bff30: 16-byte aligned: YES
  007bff30  55             PUSH EBP
  007bff31  8B EC          MOV EBP,ESP
  007bff33  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bff30
string literals (0):
direct callees (4):
  0x007C0110  CGuiInGame::StoreLastDialogAnimationData
  0x007C0090  CGuiInGame::ClearDialogAnimationData
  0x00919723  operator new
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x1B8 [dec 440]  x3  via EAX,EDX
  +0x1C0 [dec 448]  x3  via EAX,ECX,EDX
  +0x1C4 [dec 452]  x3  via EAX,ECX
```
