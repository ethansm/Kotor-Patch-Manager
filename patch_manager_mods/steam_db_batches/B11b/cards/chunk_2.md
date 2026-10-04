# Review cards B11b chunk 2

Answer each card with one TSV row in `steam_db_batches/B11b/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B11b-2-01

GOG function: CSWGuiMainInterface::SetActionDescription  gog 0x0051e680  callconv=(blank)  param_size_bytes=4  gog_size=162  gog_retn=4
Candidate Steam address: 0x0074cf00 (size 162, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.14) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074cfe0:STR0/CALLEE0/SIG0.119/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051e410 CSWGuiMainInterface::UpdateActionDescription -> 0x0074cc90  gate_class=REVIEW
- GOG-order neighbour after:  0x0051e730 CSWGuiMainInterface::UpdateActionQueue -> 0x0074cfb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00915200 CSWGuiTextParams::SetText -> Steam 0x00416e30  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0xa66c*,0xa7b4*,0xa8fc*} | Steam {0x4*,0x8*,0xc*,0xa66c*,0xa7b4*,0xa8fc*}
imm sets (* = in both, 7 common): GOG {0x4*,0x18*,0xa66c*,0xa670*,0xa744*,0xa75c*,0xa7b4*} | Steam {0x4*,0x18*,0xa66c*,0xa670*,0xa744*,0xa75c*,0xa7b4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0074cf00: 16-byte aligned: YES
  0074cf00  55             PUSH EBP
  0074cf01  8B EC          MOV EBP,ESP
  0074cf03  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074cf00
string literals (0):
direct callees (1):
  0x00416E30  CSWGuiTextParams::SetText
struct offsets touched (top 6):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via ECX
  +0xA66C [dec 42604]  x1  via EAX
  +0xA7B4 [dec 42932]  x1  via EDX
  +0xA8FC [dec 43260]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0074cf00 FUN_0074cf00 size=162 ret=4 callers=2 callees=1

void __thiscall FUN_0074cf00(int param_1,undefined4 param_2)

{
  undefined4 local_14;
  int local_10;
  undefined4 local_c;
  int local_8;
  
  FUN_00416e30(param_2);
  local_14 = *(undefined4 *)(param_1 + 0xa670);
  local_10 = *(undefined4 *)(param_1 + 0xa674);
  local_c = *(undefined4 *)(param_1 + 0xa678);
  local_8 = *(undefined4 *)(param_1 + 0xa67c);
  local_8 = (**(code **)(*(int *)(param_1 + 0xa744) + 8))();
  local_10 = *(int *)(param_1 + 0xa8fc) - local_8;
  (**(code **)(*(int *)(param_1 + 0xa66c) + 4))(&local_14);
  (**(code **)(*(int *)(param_1 + 0xa7b4) + 4))(&local_14);
  return;
}
```

### decoy: 0x0074cfe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074cfe0: 16-byte aligned: YES
  0074cfe0  55             PUSH EBP
  0074cfe1  8B EC          MOV EBP,ESP
  0074cfe3  83 EC 68       SUB ESP,0x68
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074cfe0
string literals (0):
direct callees (13):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0073F4D0  CClientExoApp::GetGameObject
  0x0074DA40  CSWGuiMainInterface::UpdateStanceAction
  0x007ECFA0  ActionEntry::Reset
  0x0077E650  CSWCCreature::GetPersonalActions
  0x00743540  CSWGuiTargetActionMenu::Update
  0x00741660  (unnamed)
  0x007418A0  action-bar button icon set (`ret 16`)
  0x0074CC90  (unnamed)
  0x007469A0  (unnamed)
  0x00746950  (unnamed)
struct offsets touched (top 8):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x6C [dec 108]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x68 [dec 104]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EAX
  +0x74D [dec 1869]  x1  via ECX
  +0x11AC [dec 4524]  x1  via ECX
```

## Card B11b-2-02

GOG function: CSWGuiMainInterface::RevertActionBarStates  gog 0x0051f1c0  callconv=(blank)  param_size_bytes=0  gog_size=535  gog_retn=0
Candidate Steam address: 0x0074da40 (size 535, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiMainInterface::UpdateStanceAction | G7 mac: pass(0.24) | G8 mac_rotation: pass
- anchors: A-STR 1 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'CANDEROUS_CUR_IMPLANT'@0099defc; CALLEE 2 (callees: GetGlobalVariableTable 0x51c890, GlobalVars 0x654860); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074dc80:STR0/CALLEE0/SIG0.092/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051f180 CSWGuiMainInterface::HandleButtonAction_2 -> 0x0074da00  gate_class=REVIEW
- GOG-order neighbour after:  0x0051f400 CSWGuiMainInterface::UpdateActionQueue_2 -> 0x0074dc80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x0051e760 CSWGuiMainInterface::PopulateMenus -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0064c2b0 CServerExoApp::GetGlobalVariableTable -> Steam 0x0051c890  in candidate callees: yes
  0x00651720 CSWGlobalVariableTable::GetValueNumber -> Steam 0x00654860  in candidate callees: yes
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'CANDEROUS_CUR_IMPLANT'
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0x146*,0x1110*,0x1198*,0x1c7c*,0x1c8c*,0x1c90*} | Steam {0x4*,0x8*,0x146*,0x1110*,0x1198*,0x1c7c*,0x1c8c*,0x1c90*}
imm sets (* = in both, 15 common): GOG {0x3*,0x4*,0x9*,0x48*,0xcb*,0x408*,0x409*,0x40a*,0x40b*,0x410*,0x411*,0x412*,0x413*,0x414*,...(+1)} | Steam {0x3*,0x4*,0x9*,0x48*,0xcb*,0x408*,0x409*,0x40a*,0x40b*,0x410*,0x411*,0x412*,0x413*,0x414*,...(+1)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0074da40: 16-byte aligned: YES
  0074da40  55             PUSH EBP
  0074da41  8B EC          MOV EBP,ESP
  0074da43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0074da40
string literals (1):
  0x0099DEFC  at 0074daf0  'CANDEROUS_CUR_IMPLANT'
direct callees (9):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x006B83F0  CSWSCreatureStats::HasFeat
  0x00733570  CExoString::CExoString_char
  0x0051C890  CServerExoApp::GetGlobalVariableTable
  0x00654860  GlobalVars GetNumber thiscall(table, CExoString* name, byte* out) RET 8
  0x00733780  CExoString::~CExoString
  0x0074CFE0  CSWGuiMainInterface::UpdateTargetMenu
struct offsets touched (top 8):
  +0x1198 [dec 4504]  x2  via EAX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x146 [dec 326]  x1  via EDX
  +0x1110 [dec 4368]  x1  via ECX
  +0x1C7C [dec 7292]  x1  via EDX
  +0x1C8C [dec 7308]  x1  via ECX
  +0x1C90 [dec 7312]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0074da40 FUN_0074da40 size=535 ret=0 callers=3 callees=9

void __fastcall FUN_0074da40(int param_1)

{
  int iVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  int local_50;
  undefined1 local_34 [8];
  uint local_2c;
  int local_28;
  undefined1 local_21;
  int local_20;
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00963178;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_1c = -1;
  local_20 = 0xffffffff;
  local_14 = 0xffffffff;
  FUN_0073fb90(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_28 = FUN_007e5d80();
  if (local_28 == 0) {
    local_50 = 0;
  }
  else {
    local_50 = FUN_0077d800();
  }
  local_18 = local_50;
  if ((local_28 != 0) && (local_50 != 0)) {
    local_21 = 0;
    iVar1 = FUN_006b83f0(0xcb);
    if (iVar1 != 0) {
      FUN_00733570("CANDEROUS_CUR_IMPLANT");
      local_8 = 0;
      puVar3 = &local_21;
      puVar2 = local_34;
      FUN_0051c890(puVar2,puVar3);
      FUN_00654860(puVar2,puVar3);
      local_8 = 0xffffffff;
      FUN_00733780();
      switch(local_21) {
      case 1:
        local_1c = 0x408;
        break;
      case 2:
        local_1c = 0x409;
        break;
      case 3:
        local_1c = 0x40a;
        break;
      case 4:
        local_1c = 0x40b;
      }
    }
    local_2c = (uint)*(ushort *)(*(int *)(local_18 + 0x1198) + 0x146);
    switch(local_2c) {
    case 9:
      local_20 = 0x410;
      break;
    case 10:
      local_20 = 0x411;
      break;
    case 0xb:
      local_20 = 0x412;
      break;
    case 0xc:
      local_20 = 0x413;
      break;
    case 0xd:
      local_20 = 0x414;
      break;
    default:
/* ... 20 more lines truncated by --max-lines */
```

### decoy: 0x0074dc80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074dc80: 16-byte aligned: YES
  0074dc80  55             PUSH EBP
  0074dc81  8B EC          MOV EBP,ESP
  0074dc83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0074dc80
string literals (0):
direct callees (14):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00590880  CSWSCombatRound::GetCurrentAction
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x007107F0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0074E0B0  (unnamed)
  0x0074E7F0  (unnamed)
  0x0058C370  (unnamed)
  0x00733780  CExoString::~CExoString
  0x009196EE  __security_check_cookie
struct offsets touched (top 7):
  +0xB00 [dec 2816]  x4  via EAX,ECX,EDX
  +0x48 [dec 72]  x2  via ECX,EDX
  +0xA84 [dec 2692]  x2  via EAX,EDX
  +0xB18 [dec 2840]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x10DC [dec 4316]  x1  via ECX
```

## Card B11b-2-03

GOG function: CGuiInGame::ChangeCharacter  gog 0x004e09b0  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x007d0470 (size 113, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::ClearDialogNonDependantList+name_held_elsewhere=0x007d02b0 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d04f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0920 CGuiInGame::EndControllerPauseState -> 0x007d0220  gate_class=AUTO
- GOG-order neighbour after:  0x004e09d0 CGuiInGame::SetupPauseGuiExtent -> 0x007d02d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005225e0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x98} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4,0x14,0xf2c}

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

## Card B11b-2-04

GOG function: CSWGuiMainInterface::GetActiveActionIdx  gog 0x00523280  callconv=(blank)  param_size_bytes=4  gog_size=228  gog_retn=4
Candidate Steam address: 0x00751b00 (size 228, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.38) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00751ab0:STR0/CALLEE0/SIG0.375/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523230 CSWGuiMainInterface::DoTargetAction -> 0x00751ab0  gate_class=REVIEW
- GOG-order neighbour after:  0x00523370 CSWGuiMainInterface::UpdateActionMenuSelected -> 0x00751bf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0051f1c0 CSWGuiMainInterface::RevertActionBarStates -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x8*,0x38*,0x50*,0x5c*,0x78*,0x7c*,0x1c7c*} | Steam {0x8*,0x38*,0x50*,0x5c*,0x78*,0x7c*,0x1c7c*}
imm sets (* = in both, 4 common): GOG {0x4*,0xc*,0x18*,0x3c*} | Steam {0x4*,0xc*,0x18*,0x3c*}

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

### decoy: 0x00751ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751ab0: 16-byte aligned: YES
  00751ab0  55             PUSH EBP
  00751ab1  8B EC          MOV EBP,ESP
  00751ab3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751ab0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1C98 [dec 7320]  x2  via EAX,ECX
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
```

## Card B11b-2-05

GOG function: CSWGuiMainInterface::StartRotation  gog 0x00517fd0  callconv=(blank)  param_size_bytes=4  gog_size=78  gog_retn=4
Candidate Steam address: 0x007469a0 (size 78, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00746950:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00517f80 CSWGuiMainInterface::OnPanelAdded -> 0x00746950  gate_class=REVIEW
- GOG-order neighbour after:  0x00518020 CSWGuiMainInterface::HandleInputEvent -> 0x007469f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x120*,0x733c*} | Steam {0x120*,0x733c*}
imm sets (* = in both, 4 common): GOG {0x4*,0x6*,0x9*,0x750*} | Steam {0x4*,0x6*,0x9*,0x750*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007469a0: 16-byte aligned: YES
  007469a0  55             PUSH EBP
  007469a1  8B EC          MOV EBP,ESP
  007469a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007469a0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007469a0 FUN_007469a0 size=78 ret=4 callers=4 callees=0

int __thiscall FUN_007469a0(int param_1,int param_2)

{
  if (-1 < param_2) {
    if (param_2 < 6) {
      return param_1 + 0x733c + param_2 * 0x750;
    }
    if (param_2 < 9) {
      return param_1 + 0x120 + (param_2 + -6) * 0x750;
    }
  }
  return 0;
}
```

### decoy: 0x00746950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00746950: 16-byte aligned: YES
  00746950  55             PUSH EBP
  00746951  8B EC          MOV EBP,ESP
  00746953  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00746950
string literals (0):
direct callees (2):
  0x007469A0  (unnamed)
  0x007468E0  (unnamed)
struct offsets touched (top 0):
```

## Card B11b-2-06

GOG function: CSWGuiMainInterface::HandleButtonAction_2  gog 0x0051f180  callconv=(blank)  param_size_bytes=4  gog_size=52  gog_retn=4
Candidate Steam address: 0x0074da00 (size 52, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074d8f0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051f0b0 CSWGuiMainInterface::HandleButtonAction -> 0x0074d930  gate_class=REVIEW
- GOG-order neighbour after:  0x0051f1c0 CSWGuiMainInterface::RevertActionBarStates -> 0x0074da40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0051f0b0 CSWGuiMainInterface::HandleButtonAction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x5c*} | Steam {0x5c*}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0074da00: 16-byte aligned: YES
  0074da00  55             PUSH EBP
  0074da01  8B EC          MOV EBP,ESP
  0074da03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074da00
string literals (0):
direct callees (1):
  0x0074D930  (unnamed)
struct offsets touched (top 2):
  +0x5C [dec 92]  x1  via EDX
  +0x10020 [dec 65568]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0074da00 FUN_0074da00 size=52 ret=4 callers=0 callees=1
THUNK -> 0x0074D930 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __thiscall FUN_0074da00(int param_1,int param_2)

{
  FUN_0074d930(*(undefined4 *)(param_1 + 0x10020),*(undefined4 *)(param_2 + 0x5c));
  return;
}
```

### decoy: 0x0074d8f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074d8f0: 16-byte aligned: YES
  0074d8f0  55             PUSH EBP
  0074d8f1  8B EC          MOV EBP,ESP
  0074d8f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074d8f0
string literals (0):
direct callees (2):
  0x0074D8C0  (unnamed)
  0x00740350  CClientExoApp::SetPausedByCombat
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x10024 [dec 65572]  x1  via EAX
```

## Card B11b-2-07

GOG function: CSWGuiMainInterface::OnPanelAdded  gog 0x00517f80  callconv=(blank)  param_size_bytes=0  gog_size=80  gog_retn=0
Candidate Steam address: 0x00746950 (size 80, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00746780:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00517f10 Global::PopHelpGui  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00517fd0 CSWGuiMainInterface::StartRotation -> 0x007469a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00517f10 Global::PopHelpGui -> Steam ?  in candidate callees: n/a
  0x00517fd0 CSWGuiMainInterface::StartRotation -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x9*,0xc*} | Steam {0x9*,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00746950: 16-byte aligned: YES
  00746950  55             PUSH EBP
  00746951  8B EC          MOV EBP,ESP
  00746953  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00746950
string literals (0):
direct callees (2):
  0x007469A0  (unnamed)
  0x007468E0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00746950 FUN_00746950 size=80 ret=0 callers=3 callees=2

void FUN_00746950(void)

{
  int iVar1;
  undefined4 local_8;
  
  DAT_00a10340 = 0xffffffff;
  for (local_8 = 0; local_8 < 9; local_8 = local_8 + 1) {
    iVar1 = FUN_007469a0(local_8);
    if (iVar1 != 0) {
      FUN_007468e0(0);
    }
  }
  return;
}
```

### decoy: 0x00746780 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00746780: 16-byte aligned: YES
  00746780  55             PUSH EBP
  00746781  8B EC          MOV EBP,ESP
  00746783  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00746780
string literals (0):
direct callees (2):
  0x0073F750  ClientApp::GetGuiInGame
  0x00745E60  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x98 [dec 152]  x1  via EDX
```

## Card B11b-2-08

GOG function: CGuiInGame::CopyQuickSaveGameToFutureGame  gog 0x004e2390  callconv=(blank)  param_size_bytes=0  gog_size=116  gog_retn=0
Candidate Steam address: 0x007d1c90 (size 116, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007d1f90:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e2040 CSWGuiSaveLoad::CopyGameToFutureGame  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004e2410 CGuiInGame::ShowDemoScreen -> 0x007d1d10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0xc*,0x6c*,0x98*} | Steam {0xc*,0x6c*,0x98*}
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
prologue for 0x007d1c90: 16-byte aligned: YES
  007d1c90  55             PUSH EBP
  007d1c91  8B EC          MOV EBP,ESP
  007d1c93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d1c90
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x98 [dec 152]  x3  via EAX,ECX
  +0x6C [dec 108]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007d1c90 FUN_007d1c90 size=116 ret=0 callers=1 callees=0

void __fastcall FUN_007d1c90(int param_1)

{
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x98) != 0) {
    (**(code **)(**(int **)(param_1 + 0x98) + 0x6c))();
  }
  for (local_8 = 0; local_8 < 8; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0xc + local_8 * 4) != 0) {
      (**(code **)(**(int **)(param_1 + 0xc + local_8 * 4) + 0x6c))();
    }
  }
  return;
}
```

### decoy: 0x007d1f90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d1f90: 16-byte aligned: YES
  007d1f90  55             PUSH EBP
  007d1f91  8B EC          MOV EBP,ESP
  007d1f93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d1f90
string literals (0):
direct callees (3):
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410800  GuiManager::Contains
  0x008A5F30  (unnamed)
struct offsets touched (top 2):
  +0x18 [dec 24]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
```

## Card B11b-2-09

GOG function: CGuiInGame::IsLoopingDialogStuntAnimation_2  gog 0x004dbe20  callconv=(blank)  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x007cb540 (size 149, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CGuiInGame::HidePowersFeatsSkillsDebugMenu | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb4a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbde0 CGuiInGame::IsAnimationPlayingInDialog -> 0x007cb6e0  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbe50 CGuiInGame::IsFadingInDialog -> 0x007cb720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005af950 CSWGuiDialogLetterbox::IsFadeDone -> Steam 0x008bb200  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x68} | Steam {0x4,0x38,0x74,0x128}
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

## Card B11b-2-10

GOG function: CSWGuiMainInterface::UpdateIndicator  gog 0x005212a0  callconv=(blank)  param_size_bytes=4  gog_size=3146  gog_retn=4
Candidate Steam address: 0x0074fb20 (size 3146, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.73) | G8 mac_rotation: pass
- anchors: A-STR 4 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'hostilereticle2'@0099dfa4; 'combatreticle'@0099dfb4; 'friendlyarrow'@0099dfc4; CALLEE 3 (callees: GetCombatMode 0x740860, GetTargetChanging 0x740e80, GetServerObject 0x7f2540); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00750790:STR0/CALLEE0/SIG0.135/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005211d0 CSWGuiMainInterface::UpdateGameMenu -> 0x0074fa50  gate_class=REVIEW
- GOG-order neighbour after:  0x00521f10 CSWGuiMainInterface::DrawMap -> 0x00750790  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (19), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x0040dd20 CClientExoApp::GetCombatMode -> Steam 0x00740860  in candidate callees: yes
  0x0040e340 CClientExoApp::GetTargetChanging -> Steam 0x00740e80  in candidate callees: yes
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004776e0 CSWCObject::GetServerObject -> Steam 0x007f2540  in candidate callees: yes
  ... 11 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (5): 'Chostilearrow'; 'hostilereticle2'; 'combatreticle'; 'friendlyarrow' ...
disp sets (* = in both, 23 common): GOG {0x4*,0x8*,0xc*,0x14*,0x1c*,0x2c*,0x3c*,0x48*,0x4c*,0x68*,0x6c*,0x6e*,0x114*,0x118*,...(+9)} | Steam {0x4*,0x8*,0xc*,0x14*,0x1c*,0x2c*,0x3c*,0x48*,0x4c*,0x68*,0x6c*,0x6e*,0x114*,0x118*,...(+9)}
imm sets (* = in both, 19 common): GOG {0x2*,0x4*,0x5*,0x8*,0xa*,0xc*,0x10*,0x20*,0x24*,0x40*,0x41*,0x44*,0xcc*,0x244*,...(+5)} | Steam {0x2*,0x4*,0x5*,0x8*,0xa*,0xc*,0x10*,0x20*,0x24*,0x40*,0x41*,0x44*,0xcc*,0x244*,...(+5)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0074fb20: 16-byte aligned: YES
  0074fb20  55             PUSH EBP
  0074fb21  8B EC          MOV EBP,ESP
  0074fb23  81 EC 44 02 00 00 SUB ESP,0x244
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074fb20
string literals (9):
  0x0099DFD4  at 0074fe10  'friendlyreticle2'
  0x0099DFC4  at 0074fe24  'friendlyarrow'
  0x0099DFB4  at 0074fe5f  'combatreticle'
  0x0099DFA4  at 0074ff35  'hostilereticle2'
  0x0099DF94  at 0074ff49  'hostilearrow'
  0x0099DFD4  at 0075019a  'friendlyreticle2'
  0x0099DFA4  at 007501a6  'hostilereticle2'
  0x0099DFC4  at 007501cc  'friendlyarrow'
  0x0099DF94  at 007501d8  'hostilearrow'
direct callees (19):
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007107F0  (unnamed)
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00740E80  CClientExoApp::GetTargetChanging
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0091CB24  (unnamed)
  0x0091C860  _ftol2_sse
  0x00710EB0  (unnamed)
  0x00740860  CClientExoApp::GetCombatMode
  0x007F2540  CSWCObject::GetServerObject
  0x005E13C0  CSWSObject::GetReputation
  0x007425F0  CSWGuiTargetActionMenu::SetFriend
  0x00710CA0  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
  0x00752760  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x595C [dec 22876]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EDX
  +0x138 [dec 312]  x3  via EAX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x48 [dec 72]  x2  via EAX,ECX
  +0x1BAC [dec 7084]  x2  via EAX,EDX
  +0x14 [dec 20]  x1  via EDX
  +0x2C [dec 44]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0074fb20 FUN_0074fb20 size=3146 ret=4 callers=1 callees=19

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0074fb20(int param_1,float param_2)

{
  int iVar1;
  float10 fVar2;
  ulonglong uVar3;
  undefined4 local_1e0;
  uint local_1dc;
  char *local_1c8;
  char *local_1c4;
  undefined1 local_174 [16];
  undefined1 local_164 [16];
  undefined1 local_154 [16];
  undefined1 local_144 [16];
  undefined1 local_134 [16];
  undefined1 local_124 [16];
  undefined1 local_114 [16];
  undefined1 local_104 [16];
  undefined1 local_f4 [16];
  undefined1 local_e4 [16];
  undefined1 local_d4 [16];
  undefined1 local_c4 [16];
  int local_b4;
  int local_b0;
  int local_ac;
  int local_a8;
  undefined4 local_a4;
  int *local_a0;
  int *local_9c;
  int local_98;
  float local_94;
  undefined1 local_90 [12];
  int local_84;
  float local_80;
  undefined1 local_7c [12];
  undefined4 local_70;
  int local_6c;
  int *local_68;
  uint local_64;
  int *local_60;
  int local_5c;
  int local_58;
  uint local_54;
  uint local_50;
  float local_4c;
  float local_48;
  undefined4 local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  int local_2c;
  undefined1 local_28 [16];
  undefined1 local_18 [16];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_60 = (int *)FUN_0073f4d0();
  local_50 = 0;
  local_54 = 0;
  local_2c = 0x40;
  local_40 = 0x20;
  local_4c = 0.0;
  local_48 = 0.0;
  local_44 = 0;
  FUN_007107f0();
  FUN_007107f0();
  local_30 = 0;
  local_34 = 0;
  local_38 = 0;
  local_3c = 0;
  local_5c = (int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6c);
  local_58 = (int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6e);
  if ((local_60 != (int *)0x0) && (iVar1 = (**(code **)(*local_60 + 0xc))(), iVar1 != 0)) {
    local_68 = (int *)(**(code **)(*local_60 + 0xc))();
    local_64 = 1;
/* ... 245 more lines truncated by --max-lines */
```

### decoy: 0x00750790 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00750790: 16-byte aligned: YES
  00750790  55             PUSH EBP
  00750791  8B EC          MOV EBP,ESP
  00750793  81 EC B4 00 00 00 SUB ESP,0xb4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00750790
string literals (0):
direct callees (15):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x0051BD90  CServerExoApp::GetModule
  0x00710C20  (unnamed)
  0x00750C10  (unnamed)
  0x005F7350  (unnamed)
  0x0091C860  _ftol2_sse
  0x0073F490  CClientExoApp::BeginIntro
  0x00463120  (unnamed)
  0x005F7170  (unnamed)
  0x004792C0  (unnamed)
  0x00479460  GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected)
  0x00751230  (unnamed)
  0x004799C0  (unnamed)
  0x004793F0  (unnamed)
struct offsets touched (top 10):
  +0x48 [dec 72]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x38 [dec 56]  x2  via EAX
  +0x5AB4 [dec 23220]  x2  via EDX
  +0x5D4C [dec 23884]  x2  via ECX
  +0x5D50 [dec 23888]  x2  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
```

## Card B11b-2-11

GOG function: CGuiInGame::GetMiniMapVisible  gog 0x004e0c60  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x007d0560 (size 49, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::RemoveGUIPopUps+name_held_elsewhere=0x007d05a0 | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x004e0ca0:0.66>0.13
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007d0530:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0c30 CGuiInGame::SetCombatMessage -> 0x007d0530  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0ca0 CGuiInGame::RemoveGUIPopUps -> 0x007d0560  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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

## Card B11b-2-12

GOG function: CSWGuiMainInterface::OnPanelRemoved  gog 0x00524ea0  callconv=(blank)  param_size_bytes=0  gog_size=81  gog_retn=0
Candidate Steam address: 0x00753780 (size 195, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x00524a20:0.55>0.23
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00753720:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00524e00 CSWGuiMainInterface::PopupGamepadMenu -> 0x00753680  gate_class=REVIEW
- GOG-order neighbour after:  0x00524f00 CSWGamepadMenuIos::UpdateStealth  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: no
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x7c}
imm sets (* = in both, 0 common): GOG {0xd588,0xffb8,0x148b4} | Steam {0x48,0x844,0x8bc}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00753780: 16-byte aligned: YES
  00753780  55             PUSH EBP
  00753781  8B EC          MOV EBP,ESP
  00753783  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00753780
string literals (3):
  0x0099E188  at 007537b9  'cus_gpad_ste3'
  0x0099E188  at 007537da  'cus_gpad_ste3'
  0x0099E178  at 007537fd  'cus_gpad_ste'
direct callees (4):
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0077FE50  CSWCCreature::IsStealthCapable
  0x00710840  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
struct offsets touched (top 2):
  +0x7C [dec 124]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00753780 FUN_00753780 size=195 ret=0 callers=3 callees=4

void __fastcall FUN_00753780(int param_1)

{
  int iVar1;
  undefined1 local_44 [16];
  undefined1 local_34 [16];
  undefined1 local_24 [16];
  undefined1 local_14 [16];
  
  FUN_0073f450(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  iVar1 = FUN_0077fe50();
  *(bool *)(param_1 + 0x7c) = iVar1 != 0;
  if (*(char *)(param_1 + 0x7c) == '\0') {
    FUN_00710840("cus_gpad_ste3");
    FUN_00414840(local_14,0);
    FUN_00710840("cus_gpad_ste3");
    FUN_00414840(local_24,0);
  }
  else {
    FUN_00710840("cus_gpad_ste");
    FUN_00414840(local_34,0);
    FUN_00710840("cus_gpad_ste2");
    FUN_00414840(local_44,0);
  }
  return;
}
```

### decoy: 0x00753720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00753720: 16-byte aligned: YES
  00753720  55             PUSH EBP
  00753721  8B EC          MOV EBP,ESP
  00753723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00753720
string literals (0):
direct callees (2):
  0x00410670  (unnamed)
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
struct offsets touched (top 0):
```

## Card B11b-2-13

GOG function: CSWGuiMainInterface::DrawMap  gog 0x00521f10  callconv=(blank)  param_size_bytes=4  gog_size=1144  gog_retn=4
Candidate Steam address: 0x00750790 (size 1144, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.36) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: BeginIntro 0x73f490); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00750c10:STR0/CALLEE0/SIG0.391/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005212a0 CSWGuiMainInterface::UpdateIndicator -> 0x0074fb20  gate_class=REVIEW
- GOG-order neighbour after:  0x00522390 CSWGuiMainInterface::InitializeMap -> 0x00750c10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (15), mapped to Steam through trusted rows only:
  0x0040c950 CClientExoApp::GetModuleCamera -> Steam 0x0073f490  in candidate callees: yes
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x00522390 CSWGuiMainInterface::InitializeMap -> Steam ?  in candidate callees: n/a
  0x005229b0 unnamed -> Steam ?  in candidate callees: n/a
  0x0061a570 CResRef::operator==_3 -> Steam ?  in candidate callees: n/a
  0x0064b7b0 CServerExoApp::GetModule -> Steam 0x0051bd90  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  ... 7 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 20 common): GOG {0x4*,0x8*,0xc*,0x14*,0x1c*,0x24*,0x38*,0x48*,0x50*,0x238*,0x258*,0x5ab4*,0x5bfc*,0x5c7c*,...(+6)} | Steam {0x4*,0x8*,0xc*,0x14*,0x1c*,0x24*,0x38*,0x48*,0x50*,0x238*,0x258*,0x5ab4*,0x5bfc*,0x5c7c*,...(+6)}
imm sets (* = in both, 12 common): GOG {0x3*,0x4*,0xc*,0x1c*,0xb4*,0x596c*,0x5ab4*,0x5ab8*,0x5b6c*,0x5bfc*,0x5d5c*,0xfffffffd*} | Steam {0x3*,0x4*,0xc*,0x1c*,0xb4*,0x596c*,0x5ab4*,0x5ab8*,0x5b6c*,0x5bfc*,0x5d5c*,0xfffffffd*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00750790: 16-byte aligned: YES
  00750790  55             PUSH EBP
  00750791  8B EC          MOV EBP,ESP
  00750793  81 EC B4 00 00 00 SUB ESP,0xb4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00750790
string literals (0):
direct callees (15):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x0051BD90  CServerExoApp::GetModule
  0x00710C20  (unnamed)
  0x00750C10  (unnamed)
  0x005F7350  (unnamed)
  0x0091C860  _ftol2_sse
  0x0073F490  CClientExoApp::BeginIntro
  0x00463120  (unnamed)
  0x005F7170  (unnamed)
  0x004792C0  (unnamed)
  0x00479460  GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected)
  0x00751230  (unnamed)
  0x004799C0  (unnamed)
  0x004793F0  (unnamed)
struct offsets touched (top 10):
  +0x48 [dec 72]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x38 [dec 56]  x2  via EAX
  +0x5AB4 [dec 23220]  x2  via EDX
  +0x5D4C [dec 23884]  x2  via ECX
  +0x5D50 [dec 23888]  x2  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00750790 FUN_00750790 size=1144 ret=4 callers=1 callees=15

void __thiscall FUN_00750790(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  float *pfVar4;
  float10 fVar5;
  undefined1 *puVar6;
  undefined4 *puVar7;
  undefined1 local_44 [12];
  undefined1 local_38 [16];
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  local_c = FUN_0072fb00();
  if ((local_c == 0) || ((*(uint *)(local_c + 0x1c) >> 3 & 1) != 0)) {
    iVar1 = FUN_0051c8d0();
    if (*(int *)(iVar1 + 0x258 [dec 600]) == 0) {
      iVar1 = FUN_0051bd90();
      local_8 = *(int *)(iVar1 + 0x238);
      if ((local_8 == 0) || (*(int *)(local_8 + 4) == 0)) {
        *(uint *)(param_1 + 0x5da4) = *(uint *)(param_1 + 0x5da4) & 0xfffffffd;
      }
      else {
        iVar1 = FUN_00710c20(&DAT_00986925);
        if (iVar1 != 0) {
          FUN_00750c10();
        }
        local_28 = *(undefined4 *)(param_1 + 0x5ab8);
        local_24 = *(undefined4 *)(param_1 + 0x5abc);
        local_20 = *(undefined4 *)(param_1 + 0x5ac0);
        local_1c = *(undefined4 *)(param_1 + 0x5ac4);
        local_18 = *(undefined4 *)(local_8 + 0x14);
        FUN_005f7350(0,&local_14,&local_10);
        local_14 = FUN_0091c860();
        local_10 = FUN_0091c860();
        iVar1 = *(int *)(param_1 + 0x5d4c) / 2;
        local_28 = FUN_0091c860();
        iVar2 = *(int *)(param_1 + 0x5d50) / 2;
        local_24 = FUN_0091c860();
        puVar7 = &local_28;
        (**(code **)(*(int *)(param_1 + 0x5ab4) + 4))(puVar7);
        piVar3 = (int *)FUN_0073f490();
        puVar6 = local_38;
        (**(code **)(*piVar3 + 0x24))(puVar6,local_44,puVar7,piVar3,iVar2,iVar1);
        pfVar4 = (float *)FUN_00463120(puVar6);
        fVar5 = (float10)FUN_005f7170(-*pfVar4,-pfVar4[1 = byte +0x4],-pfVar4[2 = byte +0x8]);
        *(float *)(param_1 + 0x5c7c) = (float)fVar5;
        FUN_004792c0();
        iVar1 = FUN_00479460(*(undefined4 *)(param_1 + 0x5d44),*(undefined4 *)(param_1 + 0x5d48),
                             *(undefined4 *)(param_1 + 0x5d4c),*(undefined4 *)(param_1 + 0x5d50),
                             &DAT_00a7fd34,0,*(undefined4 *)(param_1 + 0x50));
        if (iVar1 != 0) {
          (**(code **)(*(int *)(param_1 + 0x5ab4) + 0x38))(param_2);
          FUN_00751230(&local_28);
          (**(code **)(*(int *)(param_1 + 0x5bfc) + 0x38))(param_2);
          FUN_004799c0();
        }
        FUN_004793f0();
        *(uint *)(param_1 + 0x5da4) = *(uint *)(param_1 + 0x5da4) & 0xfffffffd | 2;
        *(uint *)(param_1 + 0x59b4) = *(uint *)(param_1 + 0x59b4) & 0xfffffffd | 2;
      }
    }
    else {
      *(uint *)(param_1 + 0x5da4) = *(uint *)(param_1 + 0x5da4) & 0xfffffffd;
      *(uint *)(param_1 + 0x5afc) = *(uint *)(param_1 + 0x5afc) & 0xfffffffd;
      *(uint *)(param_1 + 0x59b4) = *(uint *)(param_1 + 0x59b4) & 0xfffffffd;
    }
  }
  else {
    *(uint *)(param_1 + 0x5da4) = *(uint *)(param_1 + 0x5da4) & 0xfffffffd;
/* ... 3 more lines truncated by --max-lines */
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

## Card B11b-2-14

GOG function: CSWGuiMainInterface::ShowActionQueue  gog 0x00523510  callconv=(blank)  param_size_bytes=4  gog_size=167  gog_retn=4
Candidate Steam address: 0x00751d90 (size 167, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.25) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00751e40:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523460 CSWGuiMainInterface::HandleButtonAction_3 -> 0x00751ce0  gate_class=REVIEW
- GOG-order neighbour after:  0x005235c0 CSWGuiMainInterface::OnClearOneButtonPressed -> 0x00751e40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0040ddf0 CClientExoApp::SetAutoPaused -> Steam 0x00740930  in candidate callees: yes
  0x0040dee0 CClientExoApp::ShowTutorialWindow -> Steam 0x00740a20  in candidate callees: yes
  0x00515a10 CSWGuiTargetActionMenu::SelectPrevAction -> Steam ?  in candidate callees: n/a
  0x005265a0 CSWGuiMainInterface::ResetPartyMemberItemUse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x50*,0x5c*} | Steam {0x4*,0x1c*,0x50*,0x5c*}
imm sets (* = in both, 8 common): GOG {0x4*,0x5*,0x6*,0x7*,0xf*,0x1c*,0xcc*,0x7f000000*} | Steam {0x4*,0x5*,0x6*,0x7*,0xf*,0x1c*,0xcc*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751d90: 16-byte aligned: YES
  00751d90  55             PUSH EBP
  00751d91  8B EC          MOV EBP,ESP
  00751d93  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751d90
string literals (0):
direct callees (5):
  0x00754E20  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x00740930  CClientExoApp::GetClientRangedAnimation
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x00744410  CSWGuiTargetActionMenu::SelectPrevAction
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00751d90 FUN_00751d90 size=167 ret=4 callers=0 callees=5

void __thiscall FUN_00751d90(undefined4 param_1,int param_2)

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
    FUN_00744410(iVar1 + -6);
  }
  return;
}
```

### decoy: 0x00751e40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751e40: 16-byte aligned: YES
  00751e40  55             PUSH EBP
  00751e41  8B EC          MOV EBP,ESP
  00751e43  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11b-2-15

GOG function: CGuiInGame::ResetInterfaceFont  gog 0x004e0d20  callconv=(blank)  param_size_bytes=0  gog_size=306  gog_retn=0
Candidate Steam address: 0x007d0620 (size 306, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::RemoveGUIPopUps | G7 mac: pass(0.11) | G8 mac_rotation: rotation:0x004e0ca0:0.56>0.22
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: OnApplyDeath 0x597d90); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007d0760:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0ca0 CGuiInGame::RemoveGUIPopUps -> 0x007d0560  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0e60 CGuiInGame::UpdateCreatedInGameGUI -> 0x007d0760  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0073a680 CSWSEffectListHandler::OnApplyDeath -> Steam 0x00597d90  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x38*,0x3c*,0x54*,0x94*,0xa0*,0xa4*,0xb0*} | Steam {0x38*,0x3c*,0x54*,0x94*,0xa0*,0xa4*,0xb0*}
imm sets (* = in both, 2 common): GOG {0x20*,0x28*} | Steam {0x20*,0x28*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007d0620: 16-byte aligned: YES
  007d0620  55             PUSH EBP
  007d0621  8B EC          MOV EBP,ESP
  007d0623  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d0620
string literals (0):
direct callees (1):
  0x00410800  GuiManager::Contains
struct offsets touched (top 7):
  +0x38 [dec 56]  x5  via EAX,ECX,EDX
  +0x3C [dec 60]  x5  via EAX,EDX
  +0x54 [dec 84]  x3  via EAX,ECX
  +0x94 [dec 148]  x3  via EAX,EDX
  +0xA4 [dec 164]  x3  via ECX,EDX
  +0xA0 [dec 160]  x2  via EAX,EDX
  +0xB0 [dec 176]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007d0620 FUN_007d0620 size=306 ret=0 callers=1 callees=1

void __fastcall FUN_007d0620(int param_1)

{
  int iVar1;
  
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0xa0))
  ;
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0xa0) + 0x3c))(0x28,1);
  }
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0xb0))
  ;
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0xb0) + 0x3c))(0x28,1);
  }
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0x94))
  ;
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0x94) + 0x3c))(0x28,1);
  }
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0x54))
  ;
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0x54) + 0x3c))(0x28,1);
  }
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0xa4))
  ;
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0xa4) + 0x3c))(0x28,1);
  }
  return;
}
```

### decoy: 0x007d0760 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d0760: 16-byte aligned: YES
  007d0760  55             PUSH EBP
  007d0761  8B EC          MOV EBP,ESP
  007d0763  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007d0760
string literals (0):
direct callees (9):
  0x00919723  operator new
  0x008A92D0  (unnamed)
  0x008A6170  CSWGuiInGameInventory::Constructor
  0x0084C3A0  CSWGuiInGameCharacter::CSWGuiInGameCharacter (ctor; BTN_AUTO +0x34F0, BTN_LEVELUP +0x36C0)
  0x008A25C0  (unnamed)
  0x0089CF30  (unnamed)
  0x007FAE60  (unnamed)
  0x00893950  CSWGuiInGameMap::Constructor
  0x008A1170  (unnamed)
struct offsets touched (top 2):
  +0x38 [dec 56]  x8  via EAX,ECX,EDX
  +0x48 [dec 72]  x4  via EAX,ECX
```

## Card B11b-2-16

GOG function: CGuiInGame::RemoveGUIPopUps  gog 0x004e0ca0  callconv=(blank)  param_size_bytes=0  gog_size=120  gog_retn=0
Candidate Steam address: 0x007d0560 (size 49, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: rotation:0x004e0d20:0.90>0.38
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d05a0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0c60 CGuiInGame::GetMiniMapVisible -> 0x007d0560  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0d20 CGuiInGame::ResetInterfaceFont -> 0x007d0620  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: no
  0x0040d580 CClientExoApp::GetMiniGame -> Steam 0x007400a0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x1c,0x98,0x59b4} | Steam {0x3c,0x40,0x84}
imm sets (* = in both, 0 common): GOG {0x3,0x10} | Steam {}

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

### decoy: 0x007d05a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d05a0: 16-byte aligned: YES
  007d05a0  55             PUSH EBP
  007d05a1  8B EC          MOV EBP,ESP
  007d05a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d05a0
string literals (0):
direct callees (2):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007400A0  CClientExoApp::GetMiniGame
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x59B4 [dec 22964]  x1  via EDX
```

## Card B11b-2-17

GOG function: CSWGuiMainInterface::HitCheckMouse  gog 0x0051cb20  callconv=(blank)  param_size_bytes=8  gog_size=68  gog_retn=8
Candidate Steam address: 0x0074b3a0 (size 68, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: HitCheckMouse 0x40eae0)
- best decoy (neighbour/other candidate) scores: 0x0074b3f0:STR0/CALLEE0/SIG0.024/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051c510 CSWGuiMainInterface::Destructor_2 -> 0x0074ad90  gate_class=REVIEW
- GOG-order neighbour after:  0x0051cb70 CSWGuiMainInterface::Draw -> 0x0074b3f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005149b0 CSWGuiTargetActionMenu::HitCheckMouse -> Steam ?  in candidate callees: n/a
  0x0090cf60 CSWGuiPanel::HitCheckMouse -> Steam 0x0040eae0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xcc*} | Steam {0x8*,0xcc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0074b3a0: 16-byte aligned: YES
  0074b3a0  55             PUSH EBP
  0074b3a1  8B EC          MOV EBP,ESP
  0074b3a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074b3a0
string literals (0):
direct callees (2):
  0x007433B0  CSWGuiTargetActionMenu::HitCheckMouse
  0x0040EAE0  CSWGuiPanel::HitCheckMouse
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0074b3a0 FUN_0074b3a0 size=68 ret=8 callers=0 callees=2

int FUN_0074b3a0(undefined4 param_1,undefined4 param_2)

{
  undefined4 local_8;
  
  local_8 = FUN_007433b0(param_1,param_2);
  if (local_8 == 0) {
    local_8 = FUN_0040eae0(param_1,param_2);
  }
  return local_8;
}
```

### decoy: 0x0074b3f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074b3f0: 16-byte aligned: YES
  0074b3f0  55             PUSH EBP
  0074b3f1  8B EC          MOV EBP,ESP
  0074b3f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0074b3f0
string literals (0):
direct callees (36):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0074EE90  CSWGuiInGameGameplay::UpdatePartyPortraits
  0x0074FA50  (unnamed)
  0x0074FB20  (unnamed)
  0x007431D0  (unnamed)
  0x0074CFE0  CSWGuiMainInterface::UpdateTargetMenu
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x006DAA00  CSWSCreature::IsDebilitated
  0x00733540  CExoString::Constructor
  0x007408F0  CClientExoApp::GetGUIString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x00416E30  CSWGuiTextParams::SetText
  0x00417140  CSWGuiTextParams::SetDefaultColor
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
struct offsets touched (top 10):
  +0x4 [dec 4]  x13  via EAX,ECX,EDX
  +0x48 [dec 72]  x7  via EAX,ECX,EDX
  +0x70 [dec 112]  x7  via EAX,ECX,EDX
  +0x56C4 [dec 22212]  x7  via EAX,ECX,EDX
  +0x1540C [dec 87052]  x6  via EAX,ECX,EDX
  +0x1523C [dec 86588]  x4  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x38 [dec 56]  x3  via EAX,EDX
  +0x15230 [dec 86576]  x3  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
```

## Card B11b-2-18

GOG function: CSWGuiMainInterface::PopupActionMenu  gog 0x0051eca0  callconv=(blank)  param_size_bytes=4  gog_size=927  gog_retn=4
Candidate Steam address: 0x0074d520 (size 927, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: GetPausedByCombat 0x740330, SetPausedByCombat 0x740350); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074d8f0:STR0/CALLEE1/SIG0.122/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051e760 CSWGuiMainInterface::PopulateMenus -> 0x0074cfe0  gate_class=REVIEW
- GOG-order neighbour after:  0x0051f070 CSWGuiMainInterface::HandleButtonActionX -> 0x0074d8f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (16), mapped to Steam through trusted rows only:
  0x0040d7f0 CClientExoApp::GetPausedByCombat -> Steam 0x00740330  in candidate callees: yes
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam 0x00740350  in candidate callees: yes
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00517f10 Global::PopHelpGui -> Steam ?  in candidate callees: n/a
  0x00517f80 CSWGuiMainInterface::OnPanelAdded -> Steam ?  in candidate callees: n/a
  0x00517fd0 CSWGuiMainInterface::StartRotation -> Steam ?  in candidate callees: n/a
  0x0051ebb0 unnamed -> Steam ?  in candidate callees: n/a
  0x005c0410 CSWGuiObject::SetActiveControl -> Steam ?  in candidate callees: n/a
  ... 8 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '%s (%d)'
disp sets (* = in both, 6 common): GOG {0x4*,0x20*,0x34*,0x40*,0x48*,0x74d*} | Steam {0x4*,0x20*,0x34*,0x40*,0x48*,0x74d*}
imm sets (* = in both, 12 common): GOG {0x2*,0x3*,0x4*,0x6*,0x10*,0x20*,0x3c*,0x44*,0x1d0*,0xffb8*,0xfffffffc*,0xfffffffd*} | Steam {0x2*,0x3*,0x4*,0x6*,0x10*,0x20*,0x3c*,0x44*,0x1d0*,0xffb8*,0xfffffffc*,0xfffffffd*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0074d520: 16-byte aligned: YES
  0074d520  55             PUSH EBP
  0074d521  8B EC          MOV EBP,ESP
  0074d523  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0074d520
string literals (1):
  0x0099D78C  at 0074d771  '%s (%d)'
direct callees (16):
  0x0074D430  (unnamed)
  0x007469A0  (unnamed)
  0x00746950  (unnamed)
  0x007468E0  (unnamed)
  0x00410800  GuiManager::Contains
  0x00410530  CSWGuiManager::AddPanel
  0x00740330  CClientExoApp::GetPausedByCombat
  0x00740350  CClientExoApp::SetPausedByCombat
  0x0042A470  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
  0x00733540  CExoString::Constructor
  0x007337C0  CExoString::operator=
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
struct offsets touched (top 9):
  +0x48 [dec 72]  x6  via EAX,ECX,EDX
  +0x20 [dec 32]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x34 [dec 52]  x2  via EAX,ECX
  +0x10024 [dec 65572]  x2  via ECX,EDX
  +0x40 [dec 64]  x1  via EAX
  +0x74D [dec 1869]  x1  via EAX
  +0x10020 [dec 65568]  x1  via EAX
  +0x146DC [dec 83676]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0074d520 FUN_0074d520 size=927 ret=4 callers=1 callees=16

void __thiscall FUN_0074d520(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  uint local_34;
  undefined1 local_30 [8];
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00963148;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1c = FUN_0074d430(param_2,&local_18,0);
  DAT_00a7eaec = local_18;
  *(undefined4 *)(param_1 + 0x146dc) = 2;
  if (local_18 != 0) {
    if (((param_2 != DAT_00a10340) && (local_20 = FUN_007469a0(param_2), local_20 != 0)) &&
       (*(char *)(local_20 + 0x74d) == '\0')) {
      FUN_00746950(uVar1);
      DAT_00a10340 = param_2;
      FUN_007468e0(1);
    }
    *(int *)(param_1 + 0x10020) = param_2;
    iVar2 = CMFCVisualManagerBitmapCache::IsCached(DAT_00a1b49c,(CSize *)(param_1 + 0xffb8));
    if (iVar2 == 0) {
      FUN_00410530(param_1 + 0xffb8,4,1);
      uVar3 = FUN_00740330();
      *(undefined4 *)(param_1 + 0x10024) = uVar3;
      if (*(int *)(param_1 + 0x10024) == 0) {
        FUN_00740350(1,0,0);
      }
    }
    local_14 = (uint)(5 < param_2);
    FUN_0042a470(local_18,local_14 != 0);
    for (local_24 = 0; local_24 < 0x20; local_24 = local_24 + 1) {
      if (local_24 < local_18) {
        local_28 = local_24 * 0x3c + local_1c;
        FUN_00414840(local_28 + 0x20,0);
        FUN_00414840(local_28 + 0x20,0);
        iVar2 = param_1 + 0x100b0 + local_24 * 0x1d0;
        *(uint *)(iVar2 + 0x20) = *(uint *)(iVar2 + 0x20) & 0xfffffffc | 2;
        iVar2 = param_1 + 0x10128 + local_24 * 0x1d0;
        *(uint *)(iVar2 + 0x20) = *(uint *)(iVar2 + 0x20) & 0xfffffffc | 2;
        FUN_00733540();
        local_8 = 0;
        FUN_007337c0(local_28);
        if (1 < *(int *)(local_28 + 0x34)) {
          uVar3 = FUN_00734010(*(undefined4 *)(local_28 + 0x34));
          FUN_00734270(local_30,"%s (%d)",uVar3);
        }
        FUN_00416e30(local_30);
        *(int *)(param_1 + 0x10088 + local_24 * 0x1d0) = local_24;
        iVar2 = param_1 + 0x1002c + local_24 * 0x1d0;
        *(uint *)(iVar2 + 0x48) = *(uint *)(iVar2 + 0x48) & 0xfffffffd | 2;
        local_8 = 0xffffffff;
        FUN_00733780();
      }
      else {
        iVar2 = param_1 + 0x1002c + local_24 * 0x1d0;
        *(uint *)(iVar2 + 0x48) = *(uint *)(iVar2 + 0x48) & 0xfffffffd;
      }
      (**(code **)(*(int *)(param_1 + 0x1002c + local_24 * 0x1d0) + 0x40))(0);
    }
    for (local_34 = 0; (int)local_34 < 2; local_34 = local_34 + 1) {
      iVar2 = param_1 + 0x13a2c + local_34 * 0x1d0;
      *(uint *)(iVar2 + 0x48) =
           *(uint *)(iVar2 + 0x48) & 0xfffffffd | (uint)(local_34 == local_14) << 1;
    }
/* ... 5 more lines truncated by --max-lines */
```

### decoy: 0x0074d8f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074d8f0: 16-byte aligned: YES
  0074d8f0  55             PUSH EBP
  0074d8f1  8B EC          MOV EBP,ESP
  0074d8f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074d8f0
string literals (0):
direct callees (2):
  0x0074D8C0  (unnamed)
  0x00740350  CClientExoApp::SetPausedByCombat
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x10024 [dec 65572]  x1  via EAX
```

## Card B11b-2-19

GOG function: CSWGuiMainInterface::Destructor_2  gog 0x0051c510  callconv=(blank)  param_size_bytes=0  gog_size=1551  gog_retn=0
Candidate Steam address: 0x0074ad90 (size 1551, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074acb0:STR0/CALLEE0/SIG0.129/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051c490 CSWGuiCharButton::`scalar  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0051cb20 CSWGuiMainInterface::HitCheckMouse -> 0x0074b3a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: yes
  0x004ce630 CSWGuiButton::Destructor -> Steam 0x0041a3d0  in candidate callees: yes
  0x0051bb30 CSWGuiLabelHilight::Destructor -> Steam ?  in candidate callees: n/a
  0x0051bdc0 CSWGamepadMenuIos::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0051bef0 CSWGuiTargetActionMenu::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0051c2c0 CSWGuiMainInterfaceChar::Destructor -> Steam ?  in candidate callees: n/a
  0x005977a0 CSWGuiBlackenedLabel::Destructor -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0xd584*} | Steam {0x4*,0xd584*}
imm sets (* = in both, 128 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+114)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+114)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0074ad90: 16-byte aligned: YES
  0074ad90  55             PUSH EBP
  0074ad91  8B EC          MOV EBP,ESP
  0074ad93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0074ad90
string literals (0):
direct callees (12):
  0x0041A3D0  CSWGuiButton::Destructor
  0x0040E860  CSWGuiPanel::Destructor
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0074A650  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00417290  (unnamed)
  0x00418640  CSWGuiControl::Destructor
  0x0074AC30  (unnamed)
  0x00420450  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0074AAC0  (unnamed)
  0x0074A780  (unnamed)
struct offsets touched (top 2):
  +0xD584 [dec 54660]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0074ad90 FUN_0074ad90 size=1551 ret=0 callers=1 callees=12

void __fastcall FUN_0074ad90(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_00962fcc;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiMainInterface::vftable;
  local_8 = 0x37;
  if ((param_1[0x3561 = byte +0xd584] != 0) && ((int *)param_1[0x3561 = byte +0xd584] != (int *)0x0)) {
    (**(code **)(*(int *)param_1[0x3561 = byte +0xd584] + 4))(1,uVar1);
  }
  local_8._0_1_ = 0x36;
  FUN_0041a3d0();
  local_8._0_1_ = 0x35;
  FUN_0041a3d0();
  local_8._0_1_ = 0x34;
  FUN_0041a3d0();
  local_8._0_1_ = 0x33;
  FUN_0041a3d0();
  local_8._0_1_ = 0x32;
  FUN_0041a3d0();
  local_8._0_1_ = 0x31;
  FUN_0041a3d0();
  local_8._0_1_ = 0x30;
  FUN_0040e860();
  local_8._0_1_ = 0x2f;
  FUN_0041a3d0();
  local_8._0_1_ = 0x2e;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x50cf,0x1d0,2,FUN_0041a3d0);
  local_8._0_1_ = 0x2d;
  FUN_0041a3d0();
  local_8._0_1_ = 0x2c;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x4f73,0x1d0,2,FUN_0041a3d0);
  local_8._0_1_ = 0x2b;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x4e8b,0x1d0,2,FUN_0041a3d0);
  local_8._0_1_ = 0x2a;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x400b,0x1d0,0x20,FUN_0041a3d0);
  local_8._0_1_ = 0x29;
  FUN_0040e860();
  local_8._0_1_ = 0x28;
  FUN_0074a650();
  local_8._0_1_ = 0x27;
  FUN_0041a3d0();
  param_1[0x343b = byte +0xd0ec] = CSWGuiButtonToggle::vftable;
  local_8._0_1_ = 0x39;
  FUN_00414c60();
  local_8._0_1_ = 0x38;
  FUN_00414c60();
  local_8._0_1_ = 0x26;
  FUN_0041a3d0();
  param_1[0x3389 = byte +0xce24] = CSWGuiButtonToggle::vftable;
  local_8._0_1_ = 0x3b;
  FUN_00414c60();
  local_8._0_1_ = 0x3a;
  FUN_00414c60();
  local_8._0_1_ = 0x25;
  FUN_0041a3d0();
  param_1[0x32d7 = byte +0xcb5c] = CSWGuiButtonToggle::vftable;
  local_8._0_1_ = 0x3d;
  FUN_00414c60();
  local_8._0_1_ = 0x3c;
  FUN_00414c60();
  local_8._0_1_ = 0x24;
  FUN_0041a3d0();
  param_1[0x3285 = byte +0xca14] = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x3f;
  FUN_00417290();
  local_8._0_1_ = 0x3e;
  FUN_00414c60();
  local_8._0_1_ = 0x23;
  FUN_00418640();
  param_1[0x3233 = byte +0xc8cc] = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x41;
/* ... 95 more lines truncated by --max-lines */
```

### decoy: 0x0074acb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074acb0: 16-byte aligned: YES
  0074acb0  55             PUSH EBP
  0074acb1  8B EC          MOV EBP,ESP
  0074acb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074acb0
string literals (0):
direct callees (1):
  0x0041B7F0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
```

## Card B11b-2-20

GOG function: CGuiInGame::Constructor  gog 0x004cd090  callconv=(blank)  param_size_bytes=0  gog_size=3333  gog_retn=0
Candidate Steam address: 0x007bc7f0 (size 248, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc8f0:STR0/CALLEE0/SIG0.061/SRV0/VT0
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
disp sets (* = in both, 9 common): GOG {0x4*,0x8*,0xc,0x2c*,0x30,0x34,0x38,0x3c,0x40,0x44,0x48,0x4c,0x50*,0x54,...(+123)} | Steam {0x4*,0x8*,0x2c*,0x50*,0x68*,0x6c*,0x70*,0x74*,0x1c0*}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4,0x5*,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+22)} | Steam {0x5*,0x28,0x41,0x54,0x3e8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007bc7f0: 16-byte aligned: YES
  007bc7f0  55             PUSH EBP
  007bc7f1  8B EC          MOV EBP,ESP
  007bc7f3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007bc7f0 FUN_007bc7f0 size=248 ret=0 callers=2 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_007bc7f0(int param_1)

{
  undefined4 uVar1;
  undefined8 uVar2;
  undefined4 local_2c;
  undefined4 local_28;
  float local_24;
  
  if (_DAT_00986b50 <= 0.0) {
    local_24 = 1.0;
  }
  else {
    local_24 = 0.0;
  }
  if (local_24 <= (float)_DAT_00986b38) {
    local_2c = 0;
  }
  else {
    if (_DAT_00986b50 <= 0.0) {
      local_28 = 0x3f800000;
    }
    else {
      local_28 = 0;
    }
    local_2c = local_28;
  }
  *(undefined4 *)(param_1 + 0x50) = local_2c;
  *(undefined4 *)(param_1 + 0x54) = 0;
  *(undefined4 *)(param_1 + 0x58) = 0;
  *(undefined4 *)(param_1 + 0x5c) = 0;
  FUN_007bcd10(0);
  *(undefined4 *)(param_1 + 0x68) = 0;
  *(undefined4 *)(param_1 + 0x6c) = 0;
  *(undefined4 *)(param_1 + 0x70) = 0;
  *(undefined4 *)(param_1 + 0x74) = 0;
  uVar2 = FUN_0051a920();
  uVar1 = __alldiv [GHIDRA-AUTO-LABEL: unreliable](uVar2,1000,0);
  *(undefined4 *)(param_1 + 0x1c0) = uVar1;
  return;
}
```

### decoy: 0x007bc8f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bc8f0: 16-byte aligned: YES
  007bc8f0  55             PUSH EBP
  007bc8f1  8B EC          MOV EBP,ESP
  007bc8f3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bc8f0
string literals (0):
direct callees (2):
  0x0051A920  (unnamed)
  0x00920670  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x50 [dec 80]  x2  via EAX,ECX
  +0x6C [dec 108]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x2C [dec 44]  x1  via EDX
  +0x68 [dec 104]  x1  via EDX
  +0x6E [dec 110]  x1  via EAX
```

## Card B11b-2-21

GOG function: CGuiInGame::HidePowersFeatsSkillsDebugMenu  gog 0x004dbc40  callconv=(blank)  param_size_bytes=0  gog_size=149  gog_retn=0
Candidate Steam address: 0x007cb5e0 (size 190, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::UnlockDialogParticipantOrientations+name_held_elsewhere=0x007cb540 | G7 mac: pass(0.25) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb540:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbba0 CGuiInGame::HideLoadModuleDebugMenu -> 0x007cb4a0  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbce0 CGuiInGame::UnlockDialogParticipantOrientations -> 0x007cb5e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: no
  0x0040d580 CClientExoApp::GetMiniGame -> Steam 0x007400a0  in candidate callees: no
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
  0x0090ee40 CSWGuiManager::PopModalPanel -> Steam 0x004109c0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x38,0x74,0x128} | Steam {0x4*,0x2c,0x190,0x194}
imm sets (* = in both, 0 common): GOG {0x10} | Steam {0x18,0x7f000000}

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

## Card B11b-2-22

GOG function: CSWGuiMainInterface::Constructor  gog 0x00518840  callconv=(blank)  param_size_bytes=4  gog_size=13027  gog_retn=4
Candidate Steam address: 0x00747210 (size 13027, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 38 | A-CALLEE 6 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'uibit_fill_2bt'@0099d9b4; 'uibit_brdr_16bet'@0099d9c4; 'uibit_brdr_16bct'@0099d9d8; CALLEE 6 (callees: SetExtent 0x40e980, AddControl 0x40fd80, SetCornerImage 0x414680; callers: ResetInterfaceForSize 0x7cfca0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00747010:STR0/CALLEE0/SIG0.052/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00518760 CSWGuiMainInterfaceChar::Show_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0051bb30 CSWGuiLabelHilight::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (44), mapped to Steam through trusted rows only:
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam 0x0073fea0  in candidate callees: yes
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: yes
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00512a40 CSWGuiButton::Constructor -> Steam ?  in candidate callees: n/a
  0x00512f40 CSWGuiMainInterfaceAction::Initialize -> Steam ?  in candidate callees: n/a
  0x00513750 CSWGuiTargetActionMenu::Constructor -> Steam ?  in candidate callees: n/a
  0x005139d0 CSWGuiTargetActionMenu::Initialize -> Steam ?  in candidate callees: n/a
  0x005166e0 CSWGuiMainInterfaceChar::Initialize -> Steam ?  in candidate callees: n/a
  ... 36 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x004e03a0 CGuiInGame::ResetInterfaceForSize -> Steam 0x007cfca0  in candidate callers: yes
GOG strings (38): 'uibit_fill_2bt'; 'uibit_brdr_16bet'; 'uibit_brdr_16bct'; 'x@ios_action_bg' ...
disp sets (* = in both, 44 common): GOG {0x4*,0x8*,0xc*,0x1c*,0x20*,0x28*,0x34*,0x48*,0x68*,0x6c*,0x6e*,0x70*,0x98*,0x1c7c*,...(+30)} | Steam {0x4*,0x8*,0xc*,0x1c*,0x20*,0x28*,0x34*,0x48*,0x68*,0x6c*,0x6e*,0x70*,0x98*,0x1c7c*,...(+30)}
imm sets (* = in both, 225 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+211)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+211)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00747210: 16-byte aligned: YES
  00747210  55             PUSH EBP
  00747211  8B EC          MOV EBP,ESP
  00747213  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00747210
string literals (40):
  0x0099DBC4  at 0074796d  'mipc28x6_p'
  0x0099DBB8  at 00747af2  'LBL_QUEUE%d'
  0x0099DBA8  at 00747ba7  'LBL_MOULDING1'
  0x0099DB98  at 00747bed  'LBL_MOULDING3'
  0x0099DB88  at 00747c3d  'LBL_COMBATBG2'
  0x0099DB78  at 00747c82  'LBL_COMBATBG3'
  0x0099DB68  at 00747cc8  'BTN_CLEARONE'
  0x0099DB58  at 00747d0e  'BTN_CLEARONE2'
  0x0099DB48  at 00747d53  'BTN_CLEARALL'
  0x0099DB38  at 00747d99  'LBL_ACTIONDESC'
  0x0099DB24  at 00747ddf  'LBL_ACTIONDESCBG'
  0x0099DB14  at 00747e2e  'LBL_MAPBORDER'
direct callees (44):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x0091D67E  (unnamed)
  0x00742150  CSWGuiTargetActionMenu::Constructor
  0x0074A910  (unnamed)
  0x00419740  (unnamed)
  0x0084A640  (unnamed)
  0x0041A510  (unnamed)
  0x00414A50  CSWGuiBorder::Constructor
  0x0041A330  (unnamed)
  0x0074A500  (unnamed)
  0x00733540  CExoString::Constructor
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00741940  (unnamed)
  0x007468E0  (unnamed)
  0x007450E0  (unnamed)
  0x00734270  CExoString::Format
  0x0040F620  CSWGuiPanel::InitControl
  0x00745B30  (unnamed)
  0x007423D0  CSWGuiTargetActionMenu::Initialize
struct offsets touched (top 10):
  +0x48 [dec 72]  x72  via EAX,ECX,EDX
  +0x4 [dec 4]  x34  via EAX,ECX,EDX
  +0x28 [dec 40]  x16  via EDX
  +0x34 [dec 52]  x16  via EAX
  +0x20 [dec 32]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
  +0x6C [dec 108]  x10  via EAX,ECX,EDX
  +0xC [dec 12]  x8  via EAX,ECX,EDX
  +0x6E [dec 110]  x6  via EAX,ECX
  +0x68 [dec 104]  x3  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00747210 FUN_00747210 size=13027 ret=4 callers=2 callees=44

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00747210(undefined4 *param_1,int param_2)

{
  short sVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  char *local_61c;
  undefined1 local_484 [16];
  undefined1 local_474 [16];
  undefined1 local_464 [16];
  undefined1 local_454 [16];
  undefined1 local_444 [16];
  undefined1 local_434 [16];
  undefined4 local_424;
  undefined4 local_420;
  undefined4 local_41c;
  undefined1 local_418 [16];
  undefined1 local_408 [16];
  undefined1 local_3f8 [8];
  undefined1 local_3f0 [16];
  undefined1 local_3e0 [16];
  undefined1 local_3d0 [8];
  undefined1 local_3c8 [16];
  undefined1 local_3b8 [16];
  undefined1 local_3a8 [8];
  undefined4 local_3a0;
  undefined4 local_39c;
  int local_398;
  int local_394;
  undefined4 local_390;
  undefined4 local_38c;
  undefined4 local_388;
  undefined4 local_384;
  undefined4 local_380;
  int local_37c;
  int local_378;
  undefined1 local_374 [8];
  undefined1 local_36c [8];
  undefined4 local_364;
  undefined4 local_360;
  undefined4 local_35c;
  undefined4 local_358;
  undefined4 local_354;
  int local_350;
  int local_34c;
  undefined1 local_348 [8];
  undefined1 local_340 [8];
  undefined4 local_338;
  undefined4 local_334;
  undefined4 local_330;
  undefined4 local_32c;
  undefined1 local_328 [8];
  undefined1 local_320 [8];
  undefined1 local_318 [8];
  undefined1 local_310 [8];
  undefined1 local_308 [8];
  undefined1 local_300 [8];
  undefined1 local_2f8 [8];
  undefined1 local_2f0 [8];
  undefined1 local_2e8 [8];
  undefined1 local_2e0 [8];
  undefined1 local_2d8 [8];
  undefined1 local_2d0 [8];
  undefined1 local_2c8 [8];
  undefined1 local_2c0 [8];
  undefined1 local_2b8 [8];
  undefined1 local_2b0 [8];
  undefined1 local_2a8 [8];
  undefined1 local_2a0 [8];
  undefined1 local_298 [8];
  undefined1 local_290 [8];
  undefined1 local_288 [8];
  undefined1 local_280 [8];
  undefined1 local_278 [8];
  undefined1 local_270 [8];
  undefined1 local_268 [8];
/* ... 760 more lines truncated by --max-lines */
```

### decoy: 0x00747010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00747010: 16-byte aligned: YES
  00747010  55             PUSH EBP
  00747011  8B EC          MOV EBP,ESP
  00747013  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00747010
string literals (0):
direct callees (1):
  0x00412C00  CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
```

## Card B11b-2-23

GOG function: CSWGuiMainInterface::UpdateActionQueue  gog 0x0051e730  callconv=(blank)  param_size_bytes=4  gog_size=48  gog_retn=8
Candidate Steam address: 0x0074cfb0 (size 48, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074cfe0:STR0/CALLEE0/SIG0.031/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0051e680 CSWGuiMainInterface::SetActionDescription -> 0x0074cf00  gate_class=REVIEW
- GOG-order neighbour after:  0x0051e760 CSWGuiMainInterface::PopulateMenus -> 0x0074cfe0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0x595c*} | Steam {0x68*,0x595c*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x0074cfb0: 16-byte aligned: YES
  0074cfb0  55             PUSH EBP
  0074cfb1  8B EC          MOV EBP,ESP
  0074cfb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074cfb0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x2  via EAX,EDX
  +0x595C [dec 22876]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0074cfb0 FUN_0074cfb0 size=48 ret=8 callers=1 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0074cfb0(int param_1,int param_2)

{
  if (*(int *)(param_1 + 0x68) != param_2) {
    *(int *)(param_1 + 0x68) = param_2;
    *(undefined4 *)(param_1 + 0x595c) = _DAT_00987474;
  }
  return;
}
```

### decoy: 0x0074cfe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074cfe0: 16-byte aligned: YES
  0074cfe0  55             PUSH EBP
  0074cfe1  8B EC          MOV EBP,ESP
  0074cfe3  83 EC 68       SUB ESP,0x68
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074cfe0
string literals (0):
direct callees (13):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0073F4D0  CClientExoApp::GetGameObject
  0x0074DA40  CSWGuiMainInterface::UpdateStanceAction
  0x007ECFA0  ActionEntry::Reset
  0x0077E650  CSWCCreature::GetPersonalActions
  0x00743540  CSWGuiTargetActionMenu::Update
  0x00741660  (unnamed)
  0x007418A0  action-bar button icon set (`ret 16`)
  0x0074CC90  (unnamed)
  0x007469A0  (unnamed)
  0x00746950  (unnamed)
struct offsets touched (top 8):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x6C [dec 108]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x68 [dec 104]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EAX
  +0x74D [dec 1869]  x1  via ECX
  +0x11AC [dec 4524]  x1  via ECX
```

## Card B11b-2-24

GOG function: CSWGuiMainInterface::Draw  gog 0x0051cb70  callconv=(blank)  param_size_bytes=4  gog_size=3874  gog_retn=4
Candidate Steam address: 0x0074b3f0 (size 3874, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.46) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 5 (callees: GetCombatMode 0x740860, IsDebilitated 0x6daa00, CanUseItem 0x5d2f80); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074c320:STR0/CALLEE0/SIG0.217/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051cb20 CSWGuiMainInterface::HitCheckMouse -> 0x0074b3a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0051daa0 CSWGuiMainInterface::Update -> 0x0074c320  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (36), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x0040dd20 CClientExoApp::GetCombatMode -> Steam 0x00740860  in candidate callees: yes
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam 0x007408f0  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00512dd0 CSWGuiMainInterfaceAction::Update -> Steam ?  in candidate callees: n/a
  ... 28 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 21 common): GOG {0x4*,0x8*,0xc*,0x10*,0x1c*,0x38*,0x48*,0x50*,0x60*,0x70*,0x88*,0x94*,0x9c*,0x11ac*,...(+7)} | Steam {0x4*,0x8*,0xc*,0x10*,0x1c*,0x38*,0x48*,0x50*,0x60*,0x70*,0x88*,0x94*,0x9c*,0x11ac*,...(+7)}
imm sets (* = in both, 41 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+27)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+27)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0074b3f0: 16-byte aligned: YES
  0074b3f0  55             PUSH EBP
  0074b3f1  8B EC          MOV EBP,ESP
  0074b3f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0074b3f0
string literals (0):
direct callees (36):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0074EE90  CSWGuiInGameGameplay::UpdatePartyPortraits
  0x0074FA50  (unnamed)
  0x0074FB20  (unnamed)
  0x007431D0  (unnamed)
  0x0074CFE0  CSWGuiMainInterface::UpdateTargetMenu
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x006DAA00  CSWSCreature::IsDebilitated
  0x00733540  CExoString::Constructor
  0x007408F0  CClientExoApp::GetGUIString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x00416E30  CSWGuiTextParams::SetText
  0x00417140  CSWGuiTextParams::SetDefaultColor
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
struct offsets touched (top 10):
  +0x4 [dec 4]  x13  via EAX,ECX,EDX
  +0x48 [dec 72]  x7  via EAX,ECX,EDX
  +0x70 [dec 112]  x7  via EAX,ECX,EDX
  +0x56C4 [dec 22212]  x7  via EAX,ECX,EDX
  +0x1540C [dec 87052]  x6  via EAX,ECX,EDX
  +0x1523C [dec 86588]  x4  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x38 [dec 56]  x3  via EAX,EDX
  +0x15230 [dec 86576]  x3  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0074b3f0 FUN_0074b3f0 size=3874 ret=4 callers=0 callees=36

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0074b3f0(int param_1,float param_2)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  undefined4 uVar4;
  int *piVar5;
  int local_174;
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
  int local_40;
  undefined4 local_3c;
  undefined4 local_38;
  int *local_34;
  undefined4 local_30;
  int local_2c;
  undefined1 local_28 [8];
  int *local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009630e8;
  local_10 = ExceptionList;
  if ((*(uint *)(param_1 + 0x48) >> 6 & 1) == 0) {
    return;
  }
  ExceptionList = &local_10;
  FUN_0073fb90(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_18 = FUN_007e5d80();
  if (local_18 == 0) {
    local_174 = 0x7f000000;
  }
  else {
    local_174 = *(int *)(local_18 + 4);
  }
  local_1c = local_174;
  if (local_174 == 0x7f000000) {
    ExceptionList = local_10;
    return;
  }
  if ((*(float *)(param_1 + 0x70) != (float)_DAT_00986d98) &&
     (*(float *)(param_1 + 0x70) = *(float *)(param_1 + 0x70) - param_2,
     *(float *)(param_1 + 0x70) < (float)_DAT_00986b38 !=
     (*(float *)(param_1 + 0x70) == (float)_DAT_00986b38))) {
    *(undefined4 *)(param_1 + 0x70) = _DAT_00987474;
  }
  FUN_0074ee90();
  FUN_0074fa50();
  FUN_0074fb20(*(undefined4 *)(*(int *)(param_1 + 0x1c) + 0x60));
/* ... 231 more lines truncated by --max-lines */
```

### decoy: 0x0074c320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074c320: 16-byte aligned: YES
  0074c320  55             PUSH EBP
  0074c321  8B EC          MOV EBP,ESP
  0074c323  81 EC F8 00 00 00 SUB ESP,0xf8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074c320
string literals (0):
direct callees (19):
  0x00746670  (unnamed)
  0x007467C0  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
  0x00410800  GuiManager::Contains
  0x00746F40  (unnamed)
  0x00747010  (unnamed)
  0x0074D8F0  (unnamed)
  0x007C9DF0  CGuiInGame::ShowSWInGameGui
  0x0073FEE0  CClientExoApp::SetInputClass
  0x0073F470  CClientExoApp::GetPlayerCreatureId
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E6BE0  (unnamed)
  0x007E5120  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x0077FE50  CSWCCreature::IsStealthCapable
  0x0078CF90  CClientExoAppInternal::ContinueMessageProcessing
  0x00740330  CClientExoApp::GetPausedByCombat
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 10):
  +0x48 [dec 72]  x24  via EAX,ECX,EDX
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x1D4 [dec 468]  x6  via EAX,ECX,EDX
  +0x7334 [dec 29492]  x6  via EAX,ECX,EDX
  +0x146DC [dec 83676]  x3  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via ECX,EDX
  +0x5960 [dec 22880]  x2  via EDX
  +0x10 [dec 16]  x1  via ECX
```

## Card B11b-2-25

GOG function: CSWGuiMainInterface::QueueActiveAction  gog 0x00518640  callconv=(blank)  param_size_bytes=4  gog_size=116  gog_retn=4
Candidate Steam address: 0x00747010 (size 116, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00747090:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005185f0 CSWGuiMainInterface::DisplayToolTipMessage -> 0x00746fc0  gate_class=REVIEW
- GOG-order neighbour after:  0x005186c0 CSWGamepadMenuIos::Update  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00911080 CSWGuiManager::HandleMouseMove -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x1c*} | Steam {0x4*,0x8*,0xc*,0x1c*}
imm sets (* = in both, 3 common): GOG {0x4*,0x18*,0x1d0*} | Steam {0x4*,0x18*,0x1d0*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00747010: 16-byte aligned: YES
  00747010  55             PUSH EBP
  00747011  8B EC          MOV EBP,ESP
  00747013  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00747010
string literals (0):
direct callees (1):
  0x00412C00  CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00747010 FUN_00747010 size=116 ret=4 callers=2 callees=1

void __thiscall FUN_00747010(int param_1,int param_2)

{
  int *piVar1;
  
  piVar1 = (int *)(param_1 + 0x10030 + param_2 * 0x1d0);
  FUN_00412c00(*piVar1 + 0x1 [dec 1] {int*: byte +0x4},piVar1[1 = byte +0x4] + 1);
  return;
}
```

### decoy: 0x00747090 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00747090: 16-byte aligned: YES
  00747090  55             PUSH EBP
  00747091  8B EC          MOV EBP,ESP
  00747093  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00747090
string literals (0):
direct callees (2):
  0x00746FC0  CSWSItem::GetUpgradableType
  0x00747010  (unnamed)
struct offsets touched (top 1):
  +0x40 [dec 64]  x1  via EAX
```

## Card B11b-2-26

GOG function: CSWGuiMainInterface::HandleInputEvent  gog 0x00518020  callconv=(blank)  param_size_bytes=4  gog_size=1310  gog_retn=8
Candidate Steam address: 0x007469f0 (size 1310, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.55) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 5 (callees: HandleInputEvent 0x73ff80, ChangeCharacter 0x740380, SetCombatMode 0x740880); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007468e0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00517fd0 CSWGuiMainInterface::StartRotation -> 0x007469a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00518570 CSWGuiMainInterface::OnCombatYButton -> 0x00746f40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (24), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam 0x0073fea0  in candidate callees: yes
  0x0040d460 CClientExoApp::HandleInputEvent -> Steam 0x0073ff80  in candidate callees: yes
  0x0040d840 CClientExoApp::ChangeCharacter -> Steam 0x00740380  in candidate callees: yes
  0x0040dd40 CClientExoApp::SetCombatMode -> Steam 0x00740880  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x00512980 unnamed -> Steam ?  in candidate callees: n/a
  ... 16 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0x4*,0x8*,0x9*,0x70*,0x74c*,0x74d*,0xb00*,0x10dc*,0x595c*} | Steam {0x4*,0x8*,0x9*,0x70*,0x74c*,0x74d*,0xb00*,0x10dc*,0x595c*}
imm sets (* = in both, 10 common): GOG {0x6*,0x7*,0x8*,0x9*,0xb*,0x27*,0x80*,0xcc*,0xef*,0x1d0*} | Steam {0x6*,0x7*,0x8*,0x9*,0xb*,0x27*,0x80*,0xcc*,0xef*,0x1d0*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x007469f0: 16-byte aligned: YES
  007469f0  55             PUSH EBP
  007469f1  8B EC          MOV EBP,ESP
  007469f3  81 EC 80 00 00 00 SUB ESP,0x80
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007469f0
string literals (0):
direct callees (24):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00741410  (unnamed)
  0x00740380  CClientExoApp::ChangeCharacter
  0x00753680  (unnamed)
  0x005210F0  (unnamed)
  0x005904C0  CSWSCombatRound::RemoveLastAction
  0x00740880  CClientExoApp::SetCombatMode
  0x005B7AE0  CSWSCreature::SetCombatMode
  0x00541080  CSWSObject::ClearAllActions
  0x007469A0  (unnamed)
  0x00751750  (unnamed)
  0x00751710  (unnamed)
  0x00746950  (unnamed)
  0x0073FF80  CClientExoApp::HandleInputEvent
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x007468E0  (unnamed)
  0x007525D0  (unnamed)
struct offsets touched (top 8):
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x74D [dec 1869]  x4  via EAX,EDX
  +0x10DC [dec 4316]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x74C [dec 1868]  x2  via EAX,ECX
  +0x70 [dec 112]  x1  via EAX
  +0xB00 [dec 2816]  x1  via EDX
  +0x595C [dec 22876]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007469f0 FUN_007469f0 size=1310 ret=8 callers=0 callees=24

/* WARNING: Removing unreachable block (ram,0x00746d6c) */
/* WARNING: Removing unreachable block (ram,0x00746eb5) */
/* WARNING: Removing unreachable block (ram,0x00746ec2) */
/* WARNING: Removing unreachable block (ram,0x00746ee1) */
/* WARNING: Removing unreachable block (ram,0x00746eef) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_007469f0(int param_1)

{
  bool bVar1;
  bool bVar2;
  char cVar3;
  int iVar4;
  uint uVar5;
  bool bVar6;
  undefined4 uVar7;
  int *extraout_var;
  int *piVar8;
  int local_3c;
  int local_38;
  uint local_24;
  uint local_20;
  
  FUN_0073fb90();
  piVar8 = extraout_var;
  iVar4 = FUN_007e5d80();
  if (iVar4 != 0) {
    iVar4 = FUN_0077d800();
    bVar6 = false;
    if (iVar4 != 0) {
      bVar6 = *(int *)(iVar4 + 0x10dc) != 0;
    }
    bVar2 = false;
    local_24 = 0;
    local_20 = 0;
    param_1 = param_1 + -0x27;
    switch(param_1) {
    case 0:
      DAT_00a7ead4 = 1;
      cVar3 = FUN_00741410();
      if (cVar3 != '\0') {
        if (DAT_00a10340 < 0) {
          FUN_0073ff80(0xef,1);
        }
        else {
          iVar4 = FUN_007469a0(DAT_00a10340);
          if ((*(char *)(iVar4 + 0x74c) == '\0') || (*(char *)(iVar4 + 0x74d) != '\0')) {
            FUN_00746950();
            FUN_0073ff80(0xef,1);
          }
          else if ((DAT_00a10340 < 0) || (5 < DAT_00a10340)) {
            FUN_00751710(DAT_00a10340 + -6,0);
          }
          else {
            FUN_00751750(DAT_00a10340,0);
          }
        }
      }
      DAT_00a7ead4 = 0;
      break;
    case 1:
      if (DAT_00a10340 == -1) {
        if (iVar4 != 0) {
          bVar1 = true;
          if ((bVar6) && (uVar5 = FUN_005210f0(), 1 < uVar5)) {
            FUN_005904c0();
            bVar1 = false;
          }
          if (bVar1) {
            FUN_005904c0();
            FUN_00740880(0);
            FUN_005b7ae0(0,0);
            FUN_00541080(1);
            piVar8[0x1657 = byte +0x595c] = _DAT_00987474;
            piVar8[0x1c = byte +0x70] = _DAT_00987474;
          }
        }
      }
/* ... 81 more lines truncated by --max-lines */
```

### decoy: 0x007468e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007468e0: 16-byte aligned: YES
  007468e0  55             PUSH EBP
  007468e1  8B EC          MOV EBP,ESP
  007468e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007468e0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x74C [dec 1868]  x2  via EAX,EDX
```

## Card B11b-2-27

GOG function: CGuiInGame::ShowDialogMessage  gog 0x004e0bf0  callconv=(blank)  param_size_bytes=0  gog_size=50  gog_retn=0
Candidate Steam address: 0x007d04f0 (size 50, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x004e0c60:0.92>0.17
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007d0530:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0b70 CGuiInGame::ClearDialogNonDependantList -> 0x007d0470  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0c30 CGuiInGame::SetCombatMessage -> 0x007d0530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x98*,0xa900*} | Steam {0x98*,0xa900*}
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
prologue for 0x007d04f0: 16-byte aligned: YES
  007d04f0  55             PUSH EBP
  007d04f1  8B EC          MOV EBP,ESP
  007d04f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d04f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x98 [dec 152]  x2  via EAX,ECX
  +0xA900 [dec 43264]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007d04f0 FUN_007d04f0 size=50 ret=0 callers=1 callees=0

undefined1 __fastcall FUN_007d04f0(int param_1)

{
  undefined1 uVar1;
  
  if (*(int *)(param_1 + 0x98) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined1 *)(*(int *)(param_1 + 0x98) + 0xa900);
  }
  return uVar1;
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

## Card B11b-2-28

GOG function: CSWGuiMainInterface::HandleButtonActionX  gog 0x0051f070  callconv=(blank)  param_size_bytes=0  gog_size=61  gog_retn=4
Candidate Steam address: 0x0074d8f0 (size 61, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074d520:STR0/CALLEE1/SIG0.122/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0051eca0 CSWGuiMainInterface::PopupActionMenu -> 0x0074d520  gate_class=REVIEW
- GOG-order neighbour after:  0x0051f0b0 CSWGuiMainInterface::HandleButtonAction -> 0x0074d930  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam 0x00740350  in candidate callees: yes
  0x0051f040 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x0074d8f0: 16-byte aligned: YES
  0074d8f0  55             PUSH EBP
  0074d8f1  8B EC          MOV EBP,ESP
  0074d8f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074d8f0
string literals (0):
direct callees (2):
  0x0074D8C0  (unnamed)
  0x00740350  CClientExoApp::SetPausedByCombat
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x10024 [dec 65572]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0074d8f0 FUN_0074d8f0 size=61 ret=4 callers=3 callees=2

void __fastcall FUN_0074d8f0(int param_1)

{
  FUN_0074d8c0();
  if (*(int *)(param_1 + 0x10024) == 0) {
    FUN_00740350(0,0,0);
  }
  return;
}
```

### decoy: 0x0074d520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074d520: 16-byte aligned: YES
  0074d520  55             PUSH EBP
  0074d521  8B EC          MOV EBP,ESP
  0074d523  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0074d520
string literals (1):
  0x0099D78C  at 0074d771  '%s (%d)'
direct callees (16):
  0x0074D430  (unnamed)
  0x007469A0  (unnamed)
  0x00746950  (unnamed)
  0x007468E0  (unnamed)
  0x00410800  GuiManager::Contains
  0x00410530  CSWGuiManager::AddPanel
  0x00740330  CClientExoApp::GetPausedByCombat
  0x00740350  CClientExoApp::SetPausedByCombat
  0x0042A470  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
  0x00733540  CExoString::Constructor
  0x007337C0  CExoString::operator=
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
struct offsets touched (top 9):
  +0x48 [dec 72]  x6  via EAX,ECX,EDX
  +0x20 [dec 32]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x34 [dec 52]  x2  via EAX,ECX
  +0x10024 [dec 65572]  x2  via ECX,EDX
  +0x40 [dec 64]  x1  via EAX
  +0x74D [dec 1869]  x1  via EAX
  +0x10020 [dec 65568]  x1  via EAX
  +0x146DC [dec 83676]  x1  via EAX
```

## Card B11b-2-29

GOG function: CSWGuiMainInterface::UpdateActionMenuSelected  gog 0x00523370  callconv=(blank)  param_size_bytes=4  gog_size=54  gog_retn=4
Candidate Steam address: 0x00751bf0 (size 54, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x00523670:0.52>0.08
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00751ab0:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523280 CSWGuiMainInterface::GetActiveActionIdx -> 0x00751b00  gate_class=REVIEW
- GOG-order neighbour after:  0x005233b0 CSWGuiActionMenuIos::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x50*,0x5c*,0x1c77*} | Steam {0x50*,0x5c*,0x1c77*}
imm sets (* = in both, 3 common): GOG {0x4*,0x6*,0xc*} | Steam {0x4*,0x6*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751bf0: 16-byte aligned: YES
  00751bf0  55             PUSH EBP
  00751bf1  8B EC          MOV EBP,ESP
  00751bf3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751bf0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
  +0x1C77 [dec 7287]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00751bf0 FUN_00751bf0 size=54 ret=4 callers=0 callees=0

void __thiscall FUN_00751bf0(int param_1,int param_2)

{
  if (*(int *)(param_2 + 0x50) != 0) {
    *(char *)(param_1 + 0x1c77) = (char)*(undefined4 *)(param_2 + 0x5c) + -6;
  }
  return;
}
```

### decoy: 0x00751ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751ab0: 16-byte aligned: YES
  00751ab0  55             PUSH EBP
  00751ab1  8B EC          MOV EBP,ESP
  00751ab3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751ab0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1C98 [dec 7320]  x2  via EAX,ECX
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
```

## Card B11b-2-30

GOG function: CGuiInGame::SetGlobalUnpauseableState  gog 0x004e11f0  callconv=(blank)  param_size_bytes=4  gog_size=75  gog_retn=4
Candidate Steam address: 0x007d0af0 (size 75, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 10 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 10 (callers: ? 0x781590, GetModuleCamera 0x7819e0, MainLoop 0x781be0)
- best decoy (neighbour/other candidate) scores: 0x007d0d60:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0e60 CGuiInGame::UpdateCreatedInGameGUI -> 0x007d0760  gate_class=AUTO
- GOG-order neighbour after:  0x004e1240 CGuiInGame::MarkNoClickEvent -> 0x007d0b40  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (10 of 16): the card's A-CALLEE credit may come from these, not from callees
  0x0042db10 CClientExoAppInternal::DisplayMainMenu -> Steam 0x00781590  in candidate callers: yes
  0x0042df60 CClientExoAppInternal::Initialize -> Steam 0x007819e0  in candidate callers: yes
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x00444cc0 CClientExoAppInternal::PlayLoadModuleMovies -> Steam 0x007986a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xf0*} | Steam {0xf0*}
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
prologue for 0x007d0af0: 16-byte aligned: YES
  007d0af0  55             PUSH EBP
  007d0af1  8B EC          MOV EBP,ESP
  007d0af3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d0af0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF0 [dec 240]  x5  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007d0af0 FUN_007d0af0 size=75 ret=4 callers=16 callees=0

void __thiscall FUN_007d0af0(int param_1,int param_2)

{
  if (param_2 == 1) {
    *(int *)(param_1 + 0xf0) = *(int *)(param_1 + 0xf0) + 1;
  }
  else if (*(int *)(param_1 + 0xf0) != 0) {
    *(int *)(param_1 + 0xf0) = *(int *)(param_1 + 0xf0) + -1;
  }
  return;
}
```

### decoy: 0x007d0d60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d0d60: 16-byte aligned: YES
  007d0d60  55             PUSH EBP
  007d0d61  8B EC          MOV EBP,ESP
  007d0d63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d0d60
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xFFC [dec 4092]  x1  via EAX
  +0x1000 [dec 4096]  x1  via ECX
```

## Card B11b-2-31

GOG function: CGuiInGame::SetCombatMessage  gog 0x004e0c30  callconv=(blank)  param_size_bytes=4  gog_size=43  gog_retn=4
Candidate Steam address: 0x007d0530 (size 43, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: UpdateCombatMode 0x79d840, WalkUpdateLocationDistance 0x5cb430)
- best decoy (neighbour/other candidate) scores: 0x007d04f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0bf0 CGuiInGame::ShowDialogMessage -> 0x007d04f0  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0c60 CGuiInGame::GetMiniMapVisible -> 0x007d0560  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00520390 CSWGuiMainInterface::SetCombatMessage -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x00449e40 CClientExoAppInternal::UpdateCombatMode -> Steam 0x0079d840  in candidate callers: yes
  0x006d61f0 CSWSCreature::WalkUpdateLocationDistance -> Steam 0x005cb430  in candidate callers: yes
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
prologue for 0x007d0530: 16-byte aligned: YES
  007d0530  55             PUSH EBP
  007d0531  8B EC          MOV EBP,ESP
  007d0533  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d0530
string literals (0):
direct callees (1):
  0x0074EC10  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007d0530 FUN_007d0530 size=43 ret=4 callers=4 callees=1

void __thiscall FUN_007d0530(int param_1,undefined4 param_2)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    FUN_0074ec10(param_2);
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

## Card B11b-2-32

GOG function: CSWGuiMainInterface::SetCombatMessage  gog 0x00520390  callconv=(blank)  param_size_bytes=4  gog_size=625  gog_retn=4
Candidate Steam address: 0x0074ec10 (size 627, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.08) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: GetAutoPaused 0x740960); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074ebc0:STR0/CALLEE0/SIG0.113/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051ff70 CSWGuiMainInterface::ShowActionIcon -> 0x0074e7f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00520610 CSWGuiMainInterface::UpdatePortraits -> 0x0074ee90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam 0x007408f0  in candidate callees: yes
  0x0040de20 CClientExoApp::GetAutoPaused -> Steam 0x00740960  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00915200 CSWGuiTextParams::SetText -> Steam 0x00416e30  in candidate callees: yes
  0x009153e0 CSWGuiTextParams::SetColor -> Steam 0x00417010  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x706c*,0x7330*,0x7334*,0x7338*} | Steam {0x4*,0x706c*,0x7330*,0x7334*,0x7338*}
imm sets (* = in both, 10 common): GOG {0x4*,0x24*,0x44*,0x7044*,0xa5ec*,0xa5ed*,0xa5ee*,0xbaf3*,0xbb2b*,0x1b2c7*} | Steam {0x4*,0x24*,0x44*,0x7044*,0xa5ec*,0xa5ed*,0xa5ee*,0xbaf3*,0xbb2b*,0x1b2c7*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0074ec10: 16-byte aligned: YES
  0074ec10  55             PUSH EBP
  0074ec11  8B EC          MOV EBP,ESP
  0074ec13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0074ec10
string literals (0):
direct callees (5):
  0x00417010  CSWGuiTextParams::SetColor
  0x00740960  CClientExoApp::GetAutoPaused
  0x007408F0  CClientExoApp::GetGUIString
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x7334 [dec 29492]  x4  via EAX,ECX,EDX
  +0x7330 [dec 29488]  x3  via EAX,ECX
  +0x706C [dec 28780]  x1  via EAX
  +0x7338 [dec 29496]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0074ec10 FUN_0074ec10 size=627 ret=4 callers=3 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0074ec10(int param_1,uint param_2)

{
  uint uVar1;
  int iVar2;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a238;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if (*(float *)(param_1 + 0x7334) != (float)_DAT_00986d98) {
    return;
  }
  local_14 = 1;
  ExceptionList = &local_10;
  if (param_2 < 0xbaf4) {
    if (param_2 == 0xbaf3) {
      iVar2 = FUN_00740960(uVar1);
      if (iVar2 == 0) {
        FUN_00417010(&DAT_00a7fdac);
        *(undefined4 *)(param_1 + 0x7330) = _DAT_00991554;
        *(undefined4 *)(param_1 + 0x7334) = _DAT_00991554;
      }
      else {
        local_14 = 0;
      }
      goto LAB_0074ee11;
    }
    if (param_2 == 0xa5ec) {
      iVar2 = FUN_00740960(uVar1);
      if (iVar2 == 0) {
        FUN_00417010(&DAT_00a7fdac);
      }
      else {
        local_14 = 0;
      }
      goto LAB_0074ee11;
    }
    if (param_2 == 0xa5ed) {
      iVar2 = FUN_00740960(uVar1);
      if (iVar2 == 0) {
        FUN_00417010(&DAT_00a7fdac);
      }
      else {
        local_14 = 0;
      }
      goto LAB_0074ee11;
    }
    if (param_2 == 0xa5ee) {
      FUN_00417010(&DAT_00a7fdac);
      *(undefined4 *)(param_1 + 0x7330) = _DAT_00991554;
      *(undefined4 *)(param_1 + 0x7334) = _DAT_00991554;
      goto LAB_0074ee11;
    }
  }
  else {
    if (param_2 == 0xbb2b) {
      FUN_00417010(&DAT_00a7fdac);
      goto LAB_0074ee11;
    }
    if (param_2 == 0x1b2c7) {
      FUN_00417010(&DAT_00a7fdac);
      goto LAB_0074ee11;
    }
  }
  FUN_00417010(&DAT_00a7fdac);
  *(undefined4 *)(param_1 + 0x7330) = _DAT_00991554;
  *(undefined4 *)(param_1 + 0x7334) = _DAT_00991554;
LAB_0074ee11:
  if (local_14 != 0) {
    *(uint *)(param_1 + 0x7338) = param_2;
    FUN_007408f0(local_1c,param_2);
/* ... 9 more lines truncated by --max-lines */
```

### decoy: 0x0074ebc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074ebc0: 16-byte aligned: YES
  0074ebc0  55             PUSH EBP
  0074ebc1  8B EC          MOV EBP,ESP
  0074ebc3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074ebc0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
```

## Card B11b-2-33

GOG function: CGuiInGame::ChangeCharacter  gog 0x004e09b0  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x007d0220 (size 142, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::EndControllerPauseState+name_held_elsewhere=0x007d02b0 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d02b0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0920 CGuiInGame::EndControllerPauseState -> 0x007d0220  gate_class=AUTO
- GOG-order neighbour after:  0x004e09d0 CGuiInGame::SetupPauseGuiExtent -> 0x007d02d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005225e0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x98} | Steam {0x4,0x8,0xf4,0xf8,0x1008}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007d0220: 16-byte aligned: YES
  007d0220  55             PUSH EBP
  007d0221  8B EC          MOV EBP,ESP
  007d0223  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d0220
string literals (0):
direct callees (3):
  0x0072F9F0  (unnamed)
  0x0072FB20  (unnamed)
  0x00740350  CClientExoApp::SetPausedByCombat
struct offsets touched (top 5):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EDX
  +0xF4 [dec 244]  x1  via EAX
  +0xF8 [dec 248]  x1  via ECX
  +0x1008 [dec 4104]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007d0220 FUN_007d0220 size=142 ret=0 callers=1 callees=3

void __fastcall FUN_007d0220(int param_1)

{
  *(undefined4 *)(param_1 + 0xf4) = 0;
  FUN_0072f9f0();
  if (*(int *)(param_1 + 0x1008) == 1) {
    FUN_0072fb20(1);
  }
  else {
    FUN_0072fb20(0);
  }
  if (((DAT_00a1b4a4 != 0) && (*(int *)(DAT_00a1b4a4 + 8) != 0)) && (*(int *)(param_1 + 0xf8) == 0))
  {
    FUN_00740350(0,0,1);
  }
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

## Card B11b-2-34

GOG function: CSWGuiMainInterface::HandleButtonAction  gog 0x0051f0b0  callconv=(blank)  param_size_bytes=8  gog_size=203  gog_retn=8
Candidate Steam address: 0x0074d930 (size 203, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074d8f0:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051f070 CSWGuiMainInterface::HandleButtonActionX -> 0x0074d8f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0051f180 CSWGuiMainInterface::HandleButtonAction_2 -> 0x0074da00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0051ebb0 unnamed -> Steam ?  in candidate callees: n/a
  0x0051f070 CSWGuiMainInterface::HandleButtonActionX -> Steam ?  in candidate callees: n/a
  0x00522e90 unnamed -> Steam ?  in candidate callees: n/a
  0x00522ed0 CSWGuiMainInterface::DoPersonalAction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x8*,0xf0*,0x1c76*,0x1c7c*,0x1c98*} | Steam {0x8*,0xf0*,0x1c76*,0x1c7c*,0x1c98*}
imm sets (* = in both, 4 common): GOG {0x6*,0x8*,0xc*,0x3c*} | Steam {0x6*,0x8*,0xc*,0x3c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0074d930: 16-byte aligned: YES
  0074d930  55             PUSH EBP
  0074d931  8B EC          MOV EBP,ESP
  0074d933  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074d930
string literals (0):
direct callees (4):
  0x0074D8F0  (unnamed)
  0x0074D430  (unnamed)
  0x00751750  (unnamed)
  0x00751710  (unnamed)
struct offsets touched (top 2):
  +0x1C76 [dec 7286]  x1  via ECX
  +0x1C98 [dec 7320]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0074d930 FUN_0074d930 size=203 ret=8 callers=2 callees=4

void __thiscall FUN_0074d930(int param_1,int param_2,int param_3)

{
  int iVar1;
  int local_8;
  
  FUN_0074d8f0(0);
  iVar1 = FUN_0074d430(param_2,&local_8,0);
  if ((((-1 < param_2) && (-1 < param_3)) && (param_3 < local_8)) && (iVar1 != 0)) {
    *(undefined4 *)(param_1 + 0x1c98) = 0;
    if (param_2 < 6) {
      *(undefined4 *)(param_1 + 0x1c7c + param_2 * 4) = *(undefined4 *)(iVar1 + 8 + param_3 * 0x3c);
      FUN_00751750(param_2,0);
    }
    else {
      *(undefined4 *)(param_1 + *(char *)(param_1 + 0x1c76) * 0xc + 0xd8 + param_2 * 4) =
           *(undefined4 *)(iVar1 + 8 + param_3 * 0x3c);
      FUN_00751710(param_2 + -6,0);
    }
  }
  return;
}
```

### decoy: 0x0074d8f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074d8f0: 16-byte aligned: YES
  0074d8f0  55             PUSH EBP
  0074d8f1  8B EC          MOV EBP,ESP
  0074d8f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074d8f0
string literals (0):
direct callees (2):
  0x0074D8C0  (unnamed)
  0x00740350  CClientExoApp::SetPausedByCombat
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x10024 [dec 65572]  x1  via EAX
```

## Card B11b-2-35

GOG function: CGuiInGame::InitializeGlobal  gog 0x004ce110  callconv=(blank)  param_size_bytes=0  gog_size=515  gog_retn=0
Candidate Steam address: 0x007bce00 (size 3333, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.07) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bcd10:STR0/CALLEE0/SIG0.111/SRV0/VT0
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
disp sets (* = in both, 6 common): GOG {0x4*,0x38*,0xa0*,0xa4*,0xac*,0xb0,0xb4*} | Steam {0x4*,0x8,0xc,0x2c,0x30,0x34,0x38*,0x3c,0x40,0x44,0x48,0x4c,0x50,0x54,...(+123)}
imm sets (* = in both, 3 common): GOG {0x2*,0x3*,0x4*,0x48,0x9c8,0x9dc,0x4598} | Steam {0x2*,0x3*,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+22)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007bce00: 16-byte aligned: YES
  007bce00  55             PUSH EBP
  007bce01  8B EC          MOV EBP,ESP
  007bce03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007bce00
string literals (3):
  0x009A04CC  at 007bd728  'Compass'
  0x009A04BC  at 007bd739  'Display Options'
  0x00986B98  at 007bd74a  '.\\\\swkotor2.ini'
direct callees (18):
  0x00733540  CExoString::Constructor
  0x00722870  (unnamed)
  0x0091D67E  (unnamed)
  0x007107F0  (unnamed)
  0x007D22E0  (unnamed)
  0x005933B0  (unnamed)
  0x0084A640  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007B5E50  CClientOptions::SetCameraMode
  0x00919723  operator new
  0x007338D0  CExoString::operator=_2
  0x00736240  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00736320  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091CF18  (unnamed)
  0x00710EB0  (unnamed)
  0x007362C0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,EDX
  +0xFC4 [dec 4036]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x2C [dec 44]  x1  via ECX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
  +0x38 [dec 56]  x1  via EAX
  +0x3C [dec 60]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007bce00 FUN_007bce00 size=3333 ret=0 callers=1 callees=18

uint * __fastcall FUN_007bce00(uint *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 *local_a0;
  undefined4 *local_9c;
  undefined4 *local_98;
  int local_90;
  int local_84;
  uint *local_80;
  undefined1 local_64 [16];
  undefined1 local_54 [8];
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  int local_3c;
  undefined4 *local_38;
  undefined4 *local_34;
  undefined4 *local_30;
  undefined4 *local_2c;
  undefined4 *local_28;
  undefined4 *local_24;
  byte local_19;
  int local_18[2 = byte +0x8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096965b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  param_1[0x40 = byte +0x100] = 0;
  param_1[0x41 = byte +0x104] = 0;
  param_1[0x42 = byte +0x108] = 0;
  local_84 = 0x46;
  local_80 = param_1 + 0x75;
  while (local_84 = local_84 + -1, -1 < local_84) {
    local_80[1 = byte +0x4] = 0;
    local_80[2 = byte +0x8] = 0;
    local_80[3 = byte +0xc] = 0;
    local_80[7 = byte +0x1c] = 0;
    local_80[6 = byte +0x18] = 0;
    local_80[5 = byte +0x14] = 0;
    local_80[4 = byte +0x10] = 0x3f800000;
    local_80 = local_80 + 0xc;
  }
  FUN_00733540(uVar1);
  local_8 = 0;
  FUN_00722870();
  local_8._0_1_ = 1;
  FUN_00722870();
  local_8._0_1_ = 2;
  FUN_00722870();
  local_8._0_1_ = 3;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x3ce,8,8,FUN_00733540,FUN_00733780);
  local_8._0_1_ = 4;
  FUN_007107f0();
  param_1[0x403 = byte +0x100c] = 0;
  param_1[0x404 = byte +0x1010] = 0;
  param_1[0x405 = byte +0x1014] = 0;
  FUN_007d22e0(0);
  local_8._0_1_ = 5;
  FUN_005933b0(0);
  local_8._0_1_ = 6;
  FUN_0084a640(0);
  local_8._0_1_ = 7;
  uVar2 = 3;
  FUN_0072fb00(3);
  FUN_007b5e50(uVar2);
  local_28 = (undefined4 *)FUN_00919723(0x404);
  local_8._0_1_ = 8;
  if (local_28 == (undefined4 *)0x0) {
    local_98 = (undefined4 *)0x0;
  }
  else {
    *local_28 = 0x40;
    _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_28 + 0x1 [dec 1] {undefined4*: byte +0x4},0x10,0x40,FUN_008fe650,FUN_006c5370);
/* ... 212 more lines truncated by --max-lines */
```

### decoy: 0x007bcd10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bcd10: 16-byte aligned: YES
  007bcd10  55             PUSH EBP
  007bcd11  8B EC          MOV EBP,ESP
  007bcd13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007bcd10
string literals (0):
direct callees (7):
  0x00733540  CExoString::Constructor
  0x0071DEE0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x0071EBD0  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x00733570  CExoString::CExoString_char
struct offsets touched (top 0):
```

## Card B11b-2-36

GOG function: CSWGuiMainInterface::Destructor  gog 0x0051bec0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0074a750 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0074a620:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051bdc0 CSWGamepadMenuIos::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0051bef0 CSWGuiTargetActionMenu::Destructor_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0051c510 CSWGuiMainInterface::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x0074a750: 16-byte aligned: YES
  0074a750  55             PUSH EBP
  0074a751  8B EC          MOV EBP,ESP
  0074a753  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074a750
string literals (0):
direct callees (2):
  0x0074AD90  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0074a750 FUN_0074a750 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0074a750(undefined4 param_1,uint param_2)

{
  FUN_0074ad90();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0074a620 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074a620: 16-byte aligned: YES
  0074a620  55             PUSH EBP
  0074a621  8B EC          MOV EBP,ESP
  0074a623  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074a620
string literals (0):
direct callees (2):
  0x0074A650  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B11b-2-37

GOG function: CSWGuiMainInterface::OnClearOneButtonPressed  gog 0x005235c0  callconv=(blank)  param_size_bytes=4  gog_size=167  gog_retn=4
Candidate Steam address: 0x00751e40 (size 167, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00751d90:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523510 CSWGuiMainInterface::ShowActionQueue -> 0x00751d90  gate_class=REVIEW
- GOG-order neighbour after:  0x00523670 CSWGuiMainInterface::OnPauseButtonPressed -> 0x00751ef0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0040ddf0 CClientExoApp::SetAutoPaused -> Steam 0x00740930  in candidate callees: yes
  0x0040dee0 CClientExoApp::ShowTutorialWindow -> Steam 0x00740a20  in candidate callees: yes
  0x00515860 CSWGuiTargetActionMenu::SelectNextAction -> Steam ?  in candidate callees: n/a
  0x005265a0 CSWGuiMainInterface::ResetPartyMemberItemUse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x50*,0x5c*} | Steam {0x4*,0x1c*,0x50*,0x5c*}
imm sets (* = in both, 8 common): GOG {0x4*,0x5*,0x6*,0x7*,0xf*,0x1c*,0xcc*,0x7f000000*} | Steam {0x4*,0x5*,0x6*,0x7*,0xf*,0x1c*,0xcc*,0x7f000000*}

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

### decoy: 0x00751d90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751d90: 16-byte aligned: YES
  00751d90  55             PUSH EBP
  00751d91  8B EC          MOV EBP,ESP
  00751d93  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751d90
string literals (0):
direct callees (5):
  0x00754E20  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x00740930  CClientExoApp::GetClientRangedAnimation
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x00744410  CSWGuiTargetActionMenu::SelectPrevAction
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
```

## Card B11b-2-38

GOG function: CSWGuiMainInterface::DoTargetAction  gog 0x00523230  callconv=(blank)  param_size_bytes=4  gog_size=65  gog_retn=4
Candidate Steam address: 0x00751ab0 (size 65, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007516b0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00522ed0 CSWGuiMainInterface::DoPersonalAction -> 0x00751750  gate_class=REVIEW
- GOG-order neighbour after:  0x00523280 CSWGuiMainInterface::GetActiveActionIdx -> 0x00751b00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x50*,0x5c*,0x1c98*} | Steam {0x50*,0x5c*,0x1c98*}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751ab0: 16-byte aligned: YES
  00751ab0  55             PUSH EBP
  00751ab1  8B EC          MOV EBP,ESP
  00751ab3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751ab0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1C98 [dec 7320]  x2  via EAX,ECX
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00751ab0 FUN_00751ab0 size=65 ret=4 callers=0 callees=0

void __thiscall FUN_00751ab0(int param_1,int param_2)

{
  if ((*(int *)(param_2 + 0x50) != 0) && (*(int *)(param_2 + 0x5c) != *(int *)(param_1 + 0x1c98))) {
    *(int *)(param_1 + 0x1c98) = *(int *)(param_2 + 0x5c);
  }
  return;
}
```

### decoy: 0x007516b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007516b0: 16-byte aligned: YES
  007516b0  55             PUSH EBP
  007516b1  8B EC          MOV EBP,ESP
  007516b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007516b0
string literals (0):
direct callees (2):
  0x00751750  (unnamed)
  0x00751710  (unnamed)
struct offsets touched (top 2):
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
```

## Card B11b-2-39

GOG function: CSWGuiMainInterface::Update  gog 0x0051daa0  callconv=(blank)  param_size_bytes=4  gog_size=2402  gog_retn=4
Candidate Steam address: 0x0074c320 (size 2402, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.31) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: GetPlayerCreatureId 0x73f470, GetPausedByCombat 0x740330); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074b3f0:STR0/CALLEE0/SIG0.217/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051cb70 CSWGuiMainInterface::Draw -> 0x0074b3f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0051e410 CSWGuiMainInterface::UpdateActionDescription -> 0x0074cc90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (19), mapped to Steam through trusted rows only:
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: yes
  0x0040c930 CClientExoApp::GetPlayerCreatureId -> Steam 0x0073f470  in candidate callees: yes
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d7f0 CClientExoApp::GetPausedByCombat -> Steam 0x00740330  in candidate callees: yes
  0x0042be70 CSWCCreature::IsStealthCapable -> Steam ?  in candidate callees: n/a
  ... 11 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 27 common): GOG {0x4*,0x8*,0x10*,0x1c*,0x24*,0x30*,0x38*,0x48*,0x98*,0xa0*,0xcc*,0x120*,0x19c*,0x1d4*,...(+13)} | Steam {0x4*,0x8*,0x10*,0x1c*,0x24*,0x30*,0x38*,0x48*,0x98*,0xa0*,0xcc*,0x120*,0x19c*,0x1d4*,...(+13)}
imm sets (* = in both, 30 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0x41*,0x44*,0x88*,0xf8*,0x750*,0xba4c*,...(+16)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0x41*,0x44*,0x88*,0xf8*,0x750*,0xba4c*,...(+16)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0074c320: 16-byte aligned: YES
  0074c320  55             PUSH EBP
  0074c321  8B EC          MOV EBP,ESP
  0074c323  81 EC F8 00 00 00 SUB ESP,0xf8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074c320
string literals (0):
direct callees (19):
  0x00746670  (unnamed)
  0x007467C0  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
  0x00410800  GuiManager::Contains
  0x00746F40  (unnamed)
  0x00747010  (unnamed)
  0x0074D8F0  (unnamed)
  0x007C9DF0  CGuiInGame::ShowSWInGameGui
  0x0073FEE0  CClientExoApp::SetInputClass
  0x0073F470  CClientExoApp::GetPlayerCreatureId
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E6BE0  (unnamed)
  0x007E5120  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x0077FE50  CSWCCreature::IsStealthCapable
  0x0078CF90  CClientExoAppInternal::ContinueMessageProcessing
  0x00740330  CClientExoApp::GetPausedByCombat
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 10):
  +0x48 [dec 72]  x24  via EAX,ECX,EDX
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x1D4 [dec 468]  x6  via EAX,ECX,EDX
  +0x7334 [dec 29492]  x6  via EAX,ECX,EDX
  +0x146DC [dec 83676]  x3  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via ECX,EDX
  +0x5960 [dec 22880]  x2  via EDX
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0074c320 FUN_0074c320 size=2402 ret=4 callers=0 callees=19

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0074c320(int param_1,float param_2)

{
  float fVar1;
  bool bVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  int *piVar6;
  uint uVar7;
  uint local_fc;
  uint local_b8;
  uint local_b0;
  uint local_a4;
  float local_48;
  int local_38;
  int local_34;
  int local_8;
  
  if ((*(int *)(*(int *)(param_1 + 0x1c) + 0x98) == 0) && (-1 < DAT_00a1031c)) {
    if (DAT_00a7eae4 < 1) {
      FUN_00746670(DAT_00a1031c,DAT_00a7eae0 != 0);
      DAT_00a1031c = -1;
    }
    DAT_00a7eae4 = DAT_00a7eae4 + -1;
  }
  if (((float)_DAT_00986b38 < _DAT_00a10320) &&
     (_DAT_00a10320 = _DAT_00a10320 - param_2,
     _DAT_00a10320 < (float)_DAT_00986b38 != (_DAT_00a10320 == (float)_DAT_00986b38))) {
    FUN_007467c0(0);
  }
  iVar3 = FUN_0073f750();
  iVar3 = *(int *)(iVar3 + 0xcc);
  FUN_0073f750();
  iVar4 = CMFCVisualManagerBitmapCache::IsCached(DAT_00a1b49c,(CSize *)(param_1 + 0xffb8));
  if (iVar4 != 0) {
    if (0 < *(int *)(param_1 + 0x146dc)) {
      *(int *)(param_1 + 0x146dc) = *(int *)(param_1 + 0x146dc) + -1;
      uVar5 = FUN_00746f40(DAT_00a10340);
      FUN_00747010(uVar5);
    }
    bVar2 = false;
    if ((*(int **)(DAT_00a1b49c + 0x10) != (int *)0x0) &&
       (iVar4 = (**(code **)(**(int **)(DAT_00a1b49c + 0x10) + 0x30))(), iVar4 != 0)) {
      if (*(int *)(iVar4 + 0x38) == param_1 + 0xffb8) {
        bVar2 = true;
      }
      else {
        for (local_34 = 0; local_34 < 7; local_34 = local_34 + 1) {
          if ((((param_1 + 0x733c + local_34 * 0x750 == iVar4) ||
               (param_1 + 0x750c + local_34 * 0x750 == iVar4)) ||
              (param_1 + 0x76dc + local_34 * 0x750 == iVar4)) ||
             (param_1 + 0x78ac + local_34 * 0x750 == iVar4)) {
            bVar2 = true;
            break;
          }
        }
        if (!bVar2) {
          for (local_38 = 0; local_38 < 3; local_38 = local_38 + 1) {
            if (((param_1 + 0x120 + local_38 * 0x750 == iVar4) ||
                (param_1 + 0x2f0 + local_38 * 0x750 == iVar4)) ||
               ((param_1 + 0x4c0 + local_38 * 0x750 == iVar4 ||
                (param_1 + 0x690 + local_38 * 0x750 == iVar4)))) {
              bVar2 = true;
              break;
            }
          }
        }
      }
    }
    if ((DAT_00a7fcc0 == 0) || (bVar2)) {
      bVar2 = false;
    }
    else {
      bVar2 = true;
    }
    if ((iVar3 != 0) || (bVar2)) {
/* ... 87 more lines truncated by --max-lines */
```

### decoy: 0x0074b3f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074b3f0: 16-byte aligned: YES
  0074b3f0  55             PUSH EBP
  0074b3f1  8B EC          MOV EBP,ESP
  0074b3f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0074b3f0
string literals (0):
direct callees (36):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0074EE90  CSWGuiInGameGameplay::UpdatePartyPortraits
  0x0074FA50  (unnamed)
  0x0074FB20  (unnamed)
  0x007431D0  (unnamed)
  0x0074CFE0  CSWGuiMainInterface::UpdateTargetMenu
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x006DAA00  CSWSCreature::IsDebilitated
  0x00733540  CExoString::Constructor
  0x007408F0  CClientExoApp::GetGUIString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x00416E30  CSWGuiTextParams::SetText
  0x00417140  CSWGuiTextParams::SetDefaultColor
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
struct offsets touched (top 10):
  +0x4 [dec 4]  x13  via EAX,ECX,EDX
  +0x48 [dec 72]  x7  via EAX,ECX,EDX
  +0x70 [dec 112]  x7  via EAX,ECX,EDX
  +0x56C4 [dec 22212]  x7  via EAX,ECX,EDX
  +0x1540C [dec 87052]  x6  via EAX,ECX,EDX
  +0x1523C [dec 86588]  x4  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x38 [dec 56]  x3  via EAX,EDX
  +0x15230 [dec 86576]  x3  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
```

## Card B11b-2-40

GOG function: CSWGuiMainInterface::UpdateActionDescription  gog 0x0051e410  callconv=(blank)  param_size_bytes=0  gog_size=613  gog_retn=0
Candidate Steam address: 0x0074cc90 (size 613, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.28) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074c320:STR0/CALLEE0/SIG0.113/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051daa0 CSWGuiMainInterface::Update -> 0x0074c320  gate_class=REVIEW
- GOG-order neighbour after:  0x0051e680 CSWGuiMainInterface::SetActionDescription -> 0x0074cf00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam 0x007408f0  in candidate callees: yes
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x0051e680 CSWGuiMainInterface::SetActionDescription -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '%s (%d)'
disp sets (* = in both, 15 common): GOG {0x4*,0x8*,0x34*,0x48*,0x70*,0x74*,0x78*,0x7c*,0x218*,0x3e8*,0x5b8*,0x1c7c*,0x1c98*,0x733c*,...(+1)} | Steam {0x4*,0x8*,0x34*,0x48*,0x70*,0x74*,0x78*,0x7c*,0x218*,0x3e8*,0x5b8*,0x1c7c*,0x1c98*,0x733c*,...(+1)}
imm sets (* = in both, 7 common): GOG {0x5*,0xc*,0x10*,0x3c*,0x41*,0x44*,0x750*} | Steam {0x5*,0xc*,0x10*,0x3c*,0x41*,0x44*,0x750*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0074cc90: 16-byte aligned: YES
  0074cc90  55             PUSH EBP
  0074cc91  8B EC          MOV EBP,ESP
  0074cc93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0074cc90
string literals (1):
  0x0099D78C  at 0074cead  '%s (%d)'
direct callees (7):
  0x00733540  CExoString::Constructor
  0x007408F0  CClientExoApp::GetGUIString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x0074CF00  (unnamed)
struct offsets touched (top 10):
  +0x34 [dec 52]  x2  via EAX,EDX
  +0x70 [dec 112]  x2  via EAX,ECX
  +0x1C98 [dec 7320]  x2  via EAX
  +0x4 [dec 4]  x1  via EDX
  +0x48 [dec 72]  x1  via ECX
  +0x74 [dec 116]  x1  via ECX
  +0x218 [dec 536]  x1  via ECX
  +0x3E8 [dec 1000]  x1  via EAX
  +0x5B8 [dec 1464]  x1  via EDX
  +0xA784 [dec 42884]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0074cc90 FUN_0074cc90 size=613 ret=0 callers=2 callees=7

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0074cc90(int param_1)

{
  int iVar1;
  bool bVar2;
  undefined4 uVar3;
  undefined1 local_34 [8];
  int local_2c;
  int local_28;
  float local_24;
  undefined1 local_20 [8];
  int local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00963120;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_18 = -1;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  local_14 = 1.0;
  if (*(float *)(param_1 + 0x70) <= (float)_DAT_00986b38) {
    iVar1 = param_1 + 0x733c + *(int *)(param_1 + 0x1c98) * 0x750;
    if (((((*(uint *)(iVar1 + 0x218) & 1) == 0) && ((*(uint *)(iVar1 + 0x3e8 [dec 1000]) & 1) == 0)) &&
        ((*(uint *)(iVar1 + 0x5b8) & 1) == 0)) && ((*(uint *)(iVar1 + 0x48) & 1) == 0)) {
      bVar2 = false;
    }
    else {
      bVar2 = true;
    }
    if (bVar2) {
      local_18 = *(int *)(param_1 + 0x1c98);
    }
    if ((-1 < local_18) && (0 < *(int *)(param_1 + 0x7c + local_18 * 0xc))) {
      local_28 = 0;
      if (*(int *)(param_1 + 0x1c7c + local_18 * 4) != -1) {
        for (local_2c = 0; local_2c < *(int *)(param_1 + 0x7c + local_18 * 0xc);
            local_2c = local_2c + 1) {
          if (*(int *)(*(int *)(param_1 + 0x78 + local_18 * 0xc) + 8 + local_2c * 0x3c) ==
              *(int *)(param_1 + 0x1c7c + local_18 * 4)) {
            local_28 = local_2c * 0x3c + *(int *)(param_1 + 0x78 + local_18 * 0xc);
          }
        }
      }
      if (local_28 == 0) {
        local_28 = *(int *)(param_1 + 0x78 + local_18 * 0xc);
      }
      FUN_007337c0(local_28);
      if (1 < *(int *)(local_28 + 0x34)) {
        uVar3 = FUN_00734010(*(undefined4 *)(local_28 + 0x34));
        FUN_00734270(local_20,"%s (%d)",uVar3);
      }
    }
  }
  else {
    local_24 = *(float *)(param_1 + 0x70) / (float)_DAT_00991370;
    if (local_24 < (float)_DAT_00987f08) {
      local_14 = local_24 + local_24;
    }
    uVar3 = FUN_007408f0(local_34,*(undefined4 *)(param_1 + 0x74));
    local_8._0_1_ = 1;
    FUN_007337c0(uVar3);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
  }
  FUN_0074cf00(local_20);
  *(float *)(param_1 + 0xa784) = local_14;
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0074c320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074c320: 16-byte aligned: YES
  0074c320  55             PUSH EBP
  0074c321  8B EC          MOV EBP,ESP
  0074c323  81 EC F8 00 00 00 SUB ESP,0xf8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074c320
string literals (0):
direct callees (19):
  0x00746670  (unnamed)
  0x007467C0  (unnamed)
  0x0073F750  ClientApp::GetGuiInGame
  0x00410800  GuiManager::Contains
  0x00746F40  (unnamed)
  0x00747010  (unnamed)
  0x0074D8F0  (unnamed)
  0x007C9DF0  CGuiInGame::ShowSWInGameGui
  0x0073FEE0  CClientExoApp::SetInputClass
  0x0073F470  CClientExoApp::GetPlayerCreatureId
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E6BE0  (unnamed)
  0x007E5120  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x0077FE50  CSWCCreature::IsStealthCapable
  0x0078CF90  CClientExoAppInternal::ContinueMessageProcessing
  0x00740330  CClientExoApp::GetPausedByCombat
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 10):
  +0x48 [dec 72]  x24  via EAX,ECX,EDX
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x1D4 [dec 468]  x6  via EAX,ECX,EDX
  +0x7334 [dec 29492]  x6  via EAX,ECX,EDX
  +0x146DC [dec 83676]  x3  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via ECX,EDX
  +0x5960 [dec 22880]  x2  via EDX
  +0x10 [dec 16]  x1  via ECX
```

## Card B11b-2-41

GOG function: CSWGuiMainInterface::ShowActionIcon  gog 0x0051ff70  callconv=(blank)  param_size_bytes=20  gog_size=286  gog_retn=20
Candidate Steam address: 0x0074e7f0 (size 286, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.14) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074e0b0:STR0/CALLEE0/SIG0.141/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051f830 CSWGuiMainInterface::GetActionIcon -> 0x0074e0b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00520390 CSWGuiMainInterface::SetCombatMessage -> 0x0074ec10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0061a360 CResRef::IsValid -> Steam ?  in candidate callees: n/a
  0x0061a420 CResRef::operator!=_3 -> Steam ?  in candidate callees: n/a
  0x00912c10 CSWGuiBorderParams::SetFillImage -> Steam 0x00414840  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x24*,0x48*,0x5f2c*,0x5f8c*} | Steam {0x24*,0x48*,0x5f2c*,0x5f8c*}
imm sets (* = in both, 4 common): GOG {0x14*,0x18*,0x148*,0xfffffffd*} | Steam {0x14*,0x18*,0x148*,0xfffffffd*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x0074e7f0: 16-byte aligned: YES
  0074e7f0  55             PUSH EBP
  0074e7f1  8B EC          MOV EBP,ESP
  0074e7f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074e7f0
string literals (0):
direct callees (3):
  0x00710AD0  (unnamed)
  0x00710A10  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
struct offsets touched (top 2):
  +0x24 [dec 36]  x2  via EAX,EDX
  +0x48 [dec 72]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0074e7f0 FUN_0074e7f0 size=286 ret=20 callers=1 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_0074e7f0(int param_1,int *param_2,undefined4 param_3,undefined4 param_4,int param_5,int param_6)

{
  int iVar1;
  
  if ((param_6 == 0) && (iVar1 = FUN_00710ad0(&DAT_00986925), iVar1 == 0)) {
    return;
  }
  iVar1 = FUN_00710a10();
  if (iVar1 == 0) {
    FUN_00414840(param_4,0);
  }
  else {
    FUN_00414840(param_3,0);
  }
  if ((param_5 == 0) && (*param_2 == 0)) {
    *(undefined4 *)(param_1 + *param_2 * 0x148 + 0x5fb0) = _DAT_0098be34;
  }
  else {
    *(undefined4 *)(param_1 + *param_2 * 0x148 + 0x5fb0) = 0x3f800000;
  }
  iVar1 = param_1 + 0x5f2c + *param_2 * 0x148;
  *param_2 = *param_2 + 0x1 [dec 1] {int*: byte +0x4};
  *(uint *)(iVar1 + 0x48) = *(uint *)(iVar1 + 0x48) & 0xfffffffd | 2;
  return;
}
```

### decoy: 0x0074e0b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074e0b0: 16-byte aligned: YES
  0074e0b0  55             PUSH EBP
  0074e0b1  8B EC          MOV EBP,ESP
  0074e0b3  81 EC 14 03 00 00 SUB ESP,0x314
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074e0b0
string literals (30):
  0x0099DF88  at 0074e1b3  'i_attack'
  0x0099DF7C  at 0074e1c4  'i_attackm'
  0x0099DF74  at 0074e1da  'i_equip'
  0x0099DF68  at 0074e1eb  'i_equipm'
  0x0099DF5C  at 0074e204  'i_unequip'
  0x0099DF68  at 0074e218  'i_equipm'
  0x0099DF50  at 0074e27a  'i_useitemm'
  0x0099DF44  at 0074e2e2  'i_powerm'
  0x0099DF3C  at 0074e329  'i_featm'
  0x0099DF88  at 0074e382  'i_attack'
  0x0099DF7C  at 0074e396  'i_attackm'
  0x0099DF74  at 0074e3af  'i_equip'
direct callees (11):
  0x00710EB0  (unnamed)
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x00615CC0  CSWSItem::GetIcon
  0x00710CA0  (unnamed)
  0x006C1450  (unnamed)
  0x006A20F0  (unnamed)
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x006DAAC0  CSWSCreature::IsInTotalDefense
  0x0091DBA0  _memcpy
struct offsets touched (top 8):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x104 [dec 260]  x3  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x24 [dec 36]  x2  via ECX,EDX
  +0x5C [dec 92]  x2  via EDX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x74E74C [dec 7661388]  x1  via ECX
```

## Card B11b-2-42

GOG function: CSWGuiMainInterface::OnCombatYButton  gog 0x00518570  callconv=(blank)  param_size_bytes=4  gog_size=122  gog_retn=4
Candidate Steam address: 0x00746f40 (size 122, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x00518640:0.51>0.19
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00747090:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00518020 CSWGuiMainInterface::HandleInputEvent -> 0x007469f0  gate_class=REVIEW
- GOG-order neighbour after:  0x005185f0 CSWGuiMainInterface::DisplayToolTipMessage -> 0x00746fc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0x78*,0x7c*,0x1c7c*} | Steam {0x8*,0x78*,0x7c*,0x1c7c*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x3c*} | Steam {0x4*,0xc*,0x3c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00746f40: 16-byte aligned: YES
  00746f40  55             PUSH EBP
  00746f41  8B EC          MOV EBP,ESP
  00746f43  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00746f40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00746f40 FUN_00746f40 size=122 ret=4 callers=1 callees=0

int __thiscall FUN_00746f40(int param_1,int param_2)

{
  int local_8;
  
  if (*(int *)(param_1 + 0x1c7c + param_2 * 4) != -1) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x7c + param_2 * 0xc); local_8 = local_8 + 1) {
      if (*(int *)(*(int *)(param_1 + 0x78 + param_2 * 0xc) + 8 + local_8 * 0x3c) ==
          *(int *)(param_1 + 0x1c7c + param_2 * 4)) {
        return local_8;
      }
    }
  }
  return 0;
}
```

### decoy: 0x00747090 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00747090: 16-byte aligned: YES
  00747090  55             PUSH EBP
  00747091  8B EC          MOV EBP,ESP
  00747093  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00747090
string literals (0):
direct callees (2):
  0x00746FC0  CSWSItem::GetUpgradableType
  0x00747010  (unnamed)
struct offsets touched (top 1):
  +0x40 [dec 64]  x1  via EAX
```

## Card B11b-2-43

GOG function: CSWGuiMainInterface::PopulateMenus  gog 0x0051e760  callconv=(blank)  param_size_bytes=0  gog_size=1102  gog_retn=4
Candidate Steam address: 0x0074cfe0 (size 1102, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiMainInterface::UpdateTargetMenu | G7 mac: pass(0.60) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: HandleMouseClickInWorld 0x7b3ca0, RePopulateMainInterface 0x7ceb50); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074cc90:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0051e730 CSWGuiMainInterface::UpdateActionQueue -> 0x0074cfb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0051eca0 CSWGuiMainInterface::PopupActionMenu -> 0x0074d520  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x0042a5d0 CSWCCreature::GetPersonalActions -> Steam ?  in candidate callees: n/a
  0x00472100 CSWGuiInterfaceAction::Clear -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x00512c60 CSWGuiMainInterfaceAction::Show -> Steam ?  in candidate callees: n/a
  0x00512ea0 CSWGuiMainInterfaceAction::SetIcon -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x004608d0 CClientExoAppInternal::HandleMouseClickInWorld -> Steam 0x007b3ca0  in candidate callers: yes
  0x004df250 CGuiInGame::RePopulateMainInterface -> Steam 0x007ceb50  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 15 common): GOG {0x4*,0x8*,0xc*,0x20*,0x30*,0x68*,0x6c*,0x78*,0x7c*,0x94*,0x9c*,0x74d*,0x11ac*,0x1c7c*,...(+1)} | Steam {0x4*,0x8*,0xc*,0x20*,0x30*,0x68*,0x6c*,0x78*,0x7c*,0x94*,0x9c*,0x74d*,0x11ac*,0x1c7c*,...(+1)}
imm sets (* = in both, 8 common): GOG {0x4*,0x6*,0x7*,0xc*,0x3c*,0x68*,0xcc*,0x750*} | Steam {0x4*,0x6*,0x7*,0xc*,0x3c*,0x68*,0xcc*,0x750*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x0074cfe0: 16-byte aligned: YES
  0074cfe0  55             PUSH EBP
  0074cfe1  8B EC          MOV EBP,ESP
  0074cfe3  83 EC 68       SUB ESP,0x68
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074cfe0
string literals (0):
direct callees (13):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0073F4D0  CClientExoApp::GetGameObject
  0x0074DA40  CSWGuiMainInterface::UpdateStanceAction
  0x007ECFA0  ActionEntry::Reset
  0x0077E650  CSWCCreature::GetPersonalActions
  0x00743540  CSWGuiTargetActionMenu::Update
  0x00741660  (unnamed)
  0x007418A0  action-bar button icon set (`ret 16`)
  0x0074CC90  (unnamed)
  0x007469A0  (unnamed)
  0x00746950  (unnamed)
struct offsets touched (top 8):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x6C [dec 108]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x68 [dec 104]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EAX
  +0x74D [dec 1869]  x1  via ECX
  +0x11AC [dec 4524]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0074cfe0 FUN_0074cfe0 size=1102 ret=4 callers=4 callees=13

void __fastcall FUN_0074cfe0(int param_1)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  undefined4 local_6c;
  undefined4 local_68;
  int *local_64;
  int local_24;
  undefined4 local_18;
  int local_10;
  int local_8;
  
  FUN_0073fb90();
  iVar3 = FUN_007e5d80();
  if (iVar3 == 0) {
    local_64 = (int *)0x0;
  }
  else {
    local_64 = (int *)FUN_0077d800();
  }
  piVar4 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x68));
  local_18 = 0;
  local_10 = 1;
  if (iVar3 != 0) {
    if ((local_64 != (int *)0x0) && (iVar5 = (**(code **)(*local_64 + 0x94))(), iVar5 == 0)) {
      if (local_64[0x46b = byte +0x11ac] == 0) {
        bVar1 = false;
      }
      else {
        sVar2 = (**(code **)(*local_64 + 0x9c))(0);
        bVar1 = sVar2 < 1;
      }
      if (!bVar1) {
        local_10 = 0;
      }
    }
    if (*(int *)(iVar3 + 4) != *(int *)(param_1 + 0x6c)) {
      *(undefined4 *)(param_1 + 0x6c) = *(undefined4 *)(iVar3 + 4);
      FUN_0074da40();
    }
    if (piVar4 != (int *)0x0) {
      local_18 = (**(code **)(*piVar4 + 0xc))();
    }
    for (local_8 = 0; local_8 < 7; local_8 = local_8 + 1) {
      for (local_24 = 0; local_24 < *(int *)(param_1 + 0x7c + local_8 * 0xc);
          local_24 = local_24 + 1) {
        FUN_007ecfa0();
      }
      *(undefined4 *)(param_1 + 0x7c + local_8 * 0xc) = 0;
      FUN_0077e650(local_8,param_1 + 0x78 + local_8 * 0xc);
    }
    FUN_00743540(iVar3,local_10,local_18);
    for (local_8 = 0; local_8 < 6; local_8 = local_8 + 1) {
      bVar1 = false;
      for (local_24 = 0; local_24 < *(int *)(param_1 + 0x7c + local_8 * 0xc);
          local_24 = local_24 + 1) {
        if (*(int *)(*(int *)(param_1 + 0x78 + local_8 * 0xc) + 8 + local_24 * 0x3c) ==
            *(int *)(param_1 + 0x1c7c + local_8 * 4)) {
          FUN_00741660(0,1 < *(int *)(param_1 + 0x7c + local_8 * 0xc));
          if (((*(uint *)(*(int *)(param_1 + 0x78 + local_8 * 0xc) + 0x30 + local_24 * 0x3c) & 1) ==
               0) || (local_10 != 0)) {
            local_68 = 0;
          }
          else {
            local_68 = 1;
          }
          FUN_007418a0(*(int *)(param_1 + 0x78 + local_8 * 0xc) + 0x20 + local_24 * 0x3c,
                       local_24 * 0x3c + *(int *)(param_1 + 0x78 + local_8 * 0xc),local_68,0);
          bVar1 = true;
          break;
        }
      }
      if (!bVar1) {
        if (*(int *)(param_1 + 0x7c + local_8 * 0xc) < 1) {
          FUN_00741660(1,0);
/* ... 23 more lines truncated by --max-lines */
```

### decoy: 0x0074cc90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074cc90: 16-byte aligned: YES
  0074cc90  55             PUSH EBP
  0074cc91  8B EC          MOV EBP,ESP
  0074cc93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0074cc90
string literals (1):
  0x0099D78C  at 0074cead  '%s (%d)'
direct callees (7):
  0x00733540  CExoString::Constructor
  0x007408F0  CClientExoApp::GetGUIString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x0074CF00  (unnamed)
struct offsets touched (top 10):
  +0x34 [dec 52]  x2  via EAX,EDX
  +0x70 [dec 112]  x2  via EAX,ECX
  +0x1C98 [dec 7320]  x2  via EAX
  +0x4 [dec 4]  x1  via EDX
  +0x48 [dec 72]  x1  via ECX
  +0x74 [dec 116]  x1  via ECX
  +0x218 [dec 536]  x1  via ECX
  +0x3E8 [dec 1000]  x1  via EAX
  +0x5B8 [dec 1464]  x1  via EDX
  +0xA784 [dec 42884]  x1  via ECX
```

## Card B11b-2-44

GOG function: CGuiInGame::IsLoopingDialogStuntAnimation_2  gog 0x004dbe20  callconv=(blank)  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x007cb6e0 (size 55, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::IsAnimationPlayingInDialog | G7 mac: n/a | G8 mac_rotation: n/a
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
disp sets (* = in both, 0 common): GOG {0x68} | Steam {0x3c,0x64,0x84,0xfb4}
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

## Card B11b-2-45

GOG function: CSWGuiMainInterface::UpdatePortraits  gog 0x00520610  callconv=(blank)  param_size_bytes=0  gog_size=2732  gog_retn=0
Candidate Steam address: 0x0074ee90 (size 2732, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiInGameGameplay::UpdatePartyPortraits | G7 mac: pass(0.53) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 5 (callees: IsDebilitated 0x6daa00, GetMaxForcePoints 0x57eca0, GetExpNeededForLevelUp 0x6b72e0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074ec10:STR0/CALLEE0/SIG0.104/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00520390 CSWGuiMainInterface::SetCombatMessage -> 0x0074ec10  gate_class=REVIEW
- GOG-order neighbour after:  0x005210c0 CSWGuiMainInterfaceChar::SetVitality  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (23), mapped to Steam through trusted rows only:
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: yes
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x00493e00 CSWCCreatureStats::GetFirstName -> Steam ?  in candidate callees: n/a
  0x00493f00 CSWCCreatureStats::GetLastName -> Steam ?  in candidate callees: n/a
  ... 15 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 25 common): GOG {0x4*,0x8*,0x10*,0x14*,0x1c*,0x68*,0x94*,0x98*,0x9c*,0x130*,0x132*,0x148*,0x310*,0x458*,...(+11)} | Steam {0x4*,0x8*,0x10*,0x14*,0x1c*,0x68*,0x94*,0x98*,0x9c*,0x130*,0x132*,0x148*,0x310*,0x458*,...(+11)}
imm sets (* = in both, 15 common): GOG {0x2*,0x3*,0x4*,0x41*,0x64*,0x178*,0xde4*,0xffc*,0x48e0*,0x48f8*,0x4900*,0x4e9c*,0xbb2b*,0x1b2c7*,...(+1)} | Steam {0x2*,0x3*,0x4*,0x41*,0x64*,0x178*,0xde4*,0xffc*,0x48e0*,0x48f8*,0x4900*,0x4e9c*,0xbb2b*,0x1b2c7*,...(+1)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0074ee90: 16-byte aligned: YES
  0074ee90  55             PUSH EBP
  0074ee91  8B EC          MOV EBP,ESP
  0074ee93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0074ee90
string literals (0):
direct callees (23):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x00733540  CExoString::Constructor
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00849270  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00849370  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
  0x0074F940  (unnamed)
  0x0057ECA0  CSWSCreature::GetMaxForcePoints
  0x0074F9D0  (unnamed)
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x006B72E0  CSWSCreatureStats::GetExpNeededForLevelUp
  0x006B9790  CSWSCreatureStats::CanLevelUp
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x006BEDF0  CSWSCreatureStats::AutoLevelup
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
struct offsets touched (top 10):
  +0x1198 [dec 4504]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x68 [dec 104]  x4  via EAX,EDX
  +0x130 [dec 304]  x4  via EAX,ECX,EDX
  +0x310 [dec 784]  x4  via ECX,EDX
  +0xDE0 [dec 3552]  x4  via EAX,ECX,EDX
  +0x14 [dec 20]  x3  via EAX,ECX
  +0x9C [dec 156]  x3  via EAX,EDX
  +0x458 [dec 1112]  x3  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0074ee90 FUN_0074ee90 size=2732 ret=0 callers=1 callees=23

/* WARNING: Removing unreachable block (ram,0x0074f1bb) */
/* WARNING: Removing unreachable block (ram,0x0074f75b) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0074ee90(int param_1)

{
  short sVar1;
  bool bVar2;
  short sVar3;
  short sVar4;
  uint uVar5;
  undefined4 uVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  undefined1 *puVar10;
  undefined4 uVar11;
  int *local_16c;
  int local_164;
  int local_e4;
  undefined1 local_88 [16];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  undefined1 local_68 [16];
  undefined1 local_58 [8];
  undefined1 local_50 [8];
  int local_48;
  int *local_44;
  char local_3d;
  int *local_3c;
  byte local_35;
  int local_34;
  uint local_30;
  char local_29;
  int local_20;
  int local_1c;
  int *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00963208;
  local_10 = ExceptionList;
  uVar5 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_34 = 0;
  local_14 = (int *)FUN_0073fb90(uVar5);
  FUN_00733540();
  local_8 = 0;
  local_20 = 0;
  do {
    if (2 < local_20) {
      local_8 = 0xffffffff;
      FUN_00733780();
      ExceptionList = local_10;
      return;
    }
    local_18 = (int *)FUN_007e5da0(local_20);
    if (local_18 == (int *)0x0) {
      local_16c = (int *)0x0;
    }
    else {
      local_16c = (int *)FUN_0077d800();
    }
    local_3c = local_16c;
    local_30 = (uint)(local_16c != (int *)0x0);
    local_35 = 0;
    local_29 = '\0';
    local_3d = '\0';
    local_1c = local_20;
    if (*(float *)(param_1 + 0x56c4) <= (float)_DAT_00986b38) {
      if (*local_14 == 2) {
        if (local_20 == 1) {
          local_30 = 0;
          local_1c = 2;
        }
/* ... 170 more lines truncated by --max-lines */
```

### decoy: 0x0074ec10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074ec10: 16-byte aligned: YES
  0074ec10  55             PUSH EBP
  0074ec11  8B EC          MOV EBP,ESP
  0074ec13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0074ec10
string literals (0):
direct callees (5):
  0x00417010  CSWGuiTextParams::SetColor
  0x00740960  CClientExoApp::GetAutoPaused
  0x007408F0  CClientExoApp::GetGUIString
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x7334 [dec 29492]  x4  via EAX,ECX,EDX
  +0x7330 [dec 29488]  x3  via EAX,ECX
  +0x706C [dec 28780]  x1  via EAX
  +0x7338 [dec 29496]  x1  via EDX
```

## Card B11b-2-46

GOG function: CSWGuiMainInterface::UpdateGameMenu  gog 0x005211d0  callconv=(blank)  param_size_bytes=0  gog_size=205  gog_retn=0
Candidate Steam address: 0x0074fa50 (size 205, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.20) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: GetQuestJournal 0x73f070, GetPartyItems 0x5f7a30); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074fb20:STR0/CALLEE0/SIG0.088/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00521150 CSWGuiMainInterfaceChar::SetForcePoints  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005212a0 CSWGuiMainInterface::UpdateIndicator -> 0x0074fb20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040c530 CClientExoApp::GetQuestJournal -> Steam 0x0073f070  in candidate callees: yes
  0x00547970 CSWCJournal::HasNewEntries -> Steam ?  in candidate callees: n/a
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x006fd5a0 CSWPartyTable::GetPartyItems -> Steam 0x005f7a30  in candidate callees: yes
  0x00912dc0 CSWGuiBorderParams::SetPulsingAlpha -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xbcc0*,0xc400*} | Steam {0x4*,0x8*,0xbcc0*,0xc400*}
imm sets (* = in both, 4 common): GOG {0x2*,0x10*,0xbca0*,0xc3e0*} | Steam {0x2*,0x10*,0xbca0*,0xc3e0*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0074fa50: 16-byte aligned: YES
  0074fa50  55             PUSH EBP
  0074fa51  8B EC          MOV EBP,ESP
  0074fa53  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074fa50
string literals (0):
direct callees (5):
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005F7A30  CSWPartyTable::GetPartyItems
  0x004149F0  (unnamed)
  0x0073F070  CClientExoApp::GetQuestJournal
  0x0084C040  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xBCC0 [dec 48320]  x1  via EDX
  +0xC400 [dec 50176]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0074fa50 FUN_0074fa50 size=205 ret=0 callers=1 callees=5

void FUN_0074fa50(void)

{
  int iVar1;
  int extraout_var;
  int iVar2;
  undefined4 uVar3;
  
  uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  FUN_0051c8d0();
  iVar2 = extraout_var;
  iVar1 = FUN_005f7a30();
  if (*(int *)(iVar1 + 4) == 0) {
    FUN_004149f0(0,0,0);
  }
  else if ((*(uint *)(iVar2 + 0xbcc0) >> 2 & 1) == 0) {
    FUN_004149f0(1,0,1);
  }
  FUN_0073f070(iVar2,*(undefined4 *)(DAT_00a1b4a4 + 4),iVar1,uVar3);
  iVar1 = FUN_0084c040();
  if (iVar1 == 0) {
    FUN_004149f0(0,0,0);
  }
  else if ((*(uint *)(iVar2 + 0xc400) >> 2 & 1) == 0) {
    FUN_004149f0(1,0,1);
  }
  return;
}
```

### decoy: 0x0074fb20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074fb20: 16-byte aligned: YES
  0074fb20  55             PUSH EBP
  0074fb21  8B EC          MOV EBP,ESP
  0074fb23  81 EC 44 02 00 00 SUB ESP,0x244
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074fb20
string literals (9):
  0x0099DFD4  at 0074fe10  'friendlyreticle2'
  0x0099DFC4  at 0074fe24  'friendlyarrow'
  0x0099DFB4  at 0074fe5f  'combatreticle'
  0x0099DFA4  at 0074ff35  'hostilereticle2'
  0x0099DF94  at 0074ff49  'hostilearrow'
  0x0099DFD4  at 0075019a  'friendlyreticle2'
  0x0099DFA4  at 007501a6  'hostilereticle2'
  0x0099DFC4  at 007501cc  'friendlyarrow'
  0x0099DF94  at 007501d8  'hostilearrow'
direct callees (19):
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007107F0  (unnamed)
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00740E80  CClientExoApp::GetTargetChanging
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0091CB24  (unnamed)
  0x0091C860  _ftol2_sse
  0x00710EB0  (unnamed)
  0x00740860  CClientExoApp::GetCombatMode
  0x007F2540  CSWCObject::GetServerObject
  0x005E13C0  CSWSObject::GetReputation
  0x007425F0  CSWGuiTargetActionMenu::SetFriend
  0x00710CA0  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
  0x00752760  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x595C [dec 22876]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EDX
  +0x138 [dec 312]  x3  via EAX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x48 [dec 72]  x2  via EAX,ECX
  +0x1BAC [dec 7084]  x2  via EAX,EDX
  +0x14 [dec 20]  x1  via EDX
  +0x2C [dec 44]  x1  via EDX
```

## Card B11b-2-47

GOG function: CSWGuiMainInterface::HideHelp  gog 0x00517df0  callconv=(blank)  param_size_bytes=0  gog_size=279  gog_retn=4
Candidate Steam address: 0x007467c0 (size 279, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.25) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: SetPausedByCombat 0x740350, SetBackground 0x40f390); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00746670:STR0/CALLEE1/SIG0.236/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00517c60 CSWGuiHelpPanel::HandleInputEvent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00517f10 Global::PopHelpGui  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam 0x00740350  in candidate callees: yes
  0x00517490 CSWGuiMainInterface::AddHelpPanel -> Steam ?  in candidate callees: n/a
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x0090d810 CSWGuiPanel::SetBackground -> Steam 0x0040f390  in candidate callees: yes
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x48*} | Steam {0x4*,0x48*}
imm sets (* = in both, 5 common): GOG {0x4*,0x28*,0x148b4*,0x15060*,0xfffffffd*} | Steam {0x4*,0x28*,0x148b4*,0x15060*,0xfffffffd*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x007467c0: 16-byte aligned: YES
  007467c0  55             PUSH EBP
  007467c1  8B EC          MOV EBP,ESP
  007467c3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007467c0
string literals (0):
direct callees (7):
  0x00410670  (unnamed)
  0x007107F0  (unnamed)
  0x0040F390  CSWGuiPanel::SetBackground
  0x00745E60  (unnamed)
  0x0073FEE0  CClientExoApp::SetInputClass
  0x00740350  CClientExoApp::SetPausedByCombat
  0x009196EE  __security_check_cookie
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX
  +0x48 [dec 72]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007467c0 FUN_007467c0 size=279 ret=4 callers=2 callees=7

void __fastcall FUN_007467c0(int param_1)

{
  undefined1 local_1c [16];
  uint local_c;
  int local_8;
  
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  FUN_00410670(param_1 + 0x148b4);
  FUN_007107f0();
  FUN_0040f390(local_1c);
  *(uint *)(param_1 + 0x150a8) = *(uint *)(param_1 + 0x150a8) & 0xfffffffd;
  local_8 = 1;
  if ((DAT_00a7ead5 != '\0') && (DAT_00a7eadc = DAT_00a7eadc + 1, DAT_00a7eadc < 1)) {
    FUN_00745e60(DAT_00a7eadc,0);
  }
  if ((local_8 <= DAT_00a7eadc) || (DAT_00a7ead5 == '\0')) {
    DAT_00a7eadc = 0;
    DAT_00a7ead5 = '\0';
    FUN_0073fee0(DAT_00a7eacc,0);
    if (DAT_00a7ead8 == 0) {
      FUN_00740350(0,0,0);
    }
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x00746670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00746670: 16-byte aligned: YES
  00746670  55             PUSH EBP
  00746671  8B EC          MOV EBP,ESP
  00746673  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00746670
string literals (0):
direct callees (8):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0073F750  ClientApp::GetGuiInGame
  0x00410800  GuiManager::Contains
  0x00746780  (unnamed)
  0x00740330  CClientExoApp::GetPausedByCombat
  0x00740350  CClientExoApp::SetPausedByCombat
  0x0073FF00  CClientExoApp::GetInputClass
  0x0073FEE0  CClientExoApp::SetInputClass
struct offsets touched (top 3):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x98 [dec 152]  x1  via ECX
```

## Card B11b-2-48

GOG function: CGuiInGame::ClearDialogNonDependantList  gog 0x004e0b70  callconv=(blank)  param_size_bytes=0  gog_size=113  gog_retn=0
Candidate Steam address: 0x007d0470 (size 113, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007d03e0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0ae0 CGuiInGame::IsOnDialogNonDependantList -> 0x007d03e0  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0bf0 CGuiInGame::ShowDialogMessage -> 0x007d04f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x007c0300 CExoLinkedList_CAurObject_::RemoveHead -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x14*,0xf2c*} | Steam {0x4*,0x14*,0xf2c*}

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

### decoy: 0x007d03e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d03e0: 16-byte aligned: YES
  007d03e0  55             PUSH EBP
  007d03e1  8B EC          MOV EBP,ESP
  007d03e3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11b-2-49

GOG function: CSWGuiMainInterface::DisplayToolTipMessage  gog 0x005185f0  callconv=(blank)  param_size_bytes=0  gog_size=79  gog_retn=0
Candidate Steam address: 0x00746fc0 (size 79, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00746f40:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00518570 CSWGuiMainInterface::OnCombatYButton -> 0x00746f40  gate_class=REVIEW
- GOG-order neighbour after:  0x00518640 CSWGuiMainInterface::QueueActiveAction -> 0x00747010  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0xc*,0x20*,0x1d0*} | Steam {0xc*,0x20*,0x1d0*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00746fc0: 16-byte aligned: YES
  00746fc0  55             PUSH EBP
  00746fc1  8B EC          MOV EBP,ESP
  00746fc3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00746fc0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00746fc0 FUN_00746fc0 size=79 ret=0 callers=2 callees=0

int __fastcall FUN_00746fc0(int param_1)

{
  undefined4 local_c;
  undefined4 local_8;
  
  local_8 = -1;
  for (local_c = 0; local_c < 0x20; local_c = local_c + 1) {
    if ((*(uint *)(param_1 + 0x10074 + local_c * 0x1d0) & 1) != 0) {
      local_8 = local_c;
    }
  }
  return local_8;
}
```

### decoy: 0x00746f40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00746f40: 16-byte aligned: YES
  00746f40  55             PUSH EBP
  00746f41  8B EC          MOV EBP,ESP
  00746f43  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00746f40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B11b-2-50

GOG function: CSWGuiMainInterface::DoPersonalAction  gog 0x00522ed0  callconv=(blank)  param_size_bytes=4  gog_size=833  gog_retn=8
Candidate Steam address: 0x00751750 (size 833, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.45) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: GetAutoPaused 0x740960, ClearAllQueuedCombatActions 0x7f2500; callers: HandleInputEvent 0x7b12c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00751710:STR0/CALLEE1/SIG0.026/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00522e30 CSWGuiMainInterface::OnCharacterRightClicked -> 0x007516b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00523230 CSWGuiMainInterface::DoTargetAction -> 0x00751ab0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: yes
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x0040de20 CClientExoApp::GetAutoPaused -> Steam 0x00740960  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004776a0 CSWCObject::ClearAllQueuedCombatActions -> Steam 0x007f2500  in candidate callees: yes
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x004dc4b0 CGuiInGame::SetPaused -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 16 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x30*,0x70*,0x74*,0x78*,0x7c*,0x458*,0x1c7c*,...(+2)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x30*,0x70*,0x74*,0x78*,0x7c*,0x458*,0x1c7c*,...(+2)}
imm sets (* = in both, 17 common): GOG {0x2*,0x5*,0x6*,0x7*,0x8*,0xa*,0xc*,0xf*,0x3c*,0x4c*,0x750*,0x96d5*,0x96d6*,0x96d7*,...(+3)} | Steam {0x2*,0x5*,0x6*,0x7*,0x8*,0xa*,0xc*,0xf*,0x3c*,0x4c*,0x750*,0x96d5*,0x96d6*,0x96d7*,...(+3)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x00751750: 16-byte aligned: YES
  00751750  55             PUSH EBP
  00751751  8B EC          MOV EBP,ESP
  00751753  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751750
string literals (0):
direct callees (13):
  0x007523E0  (unnamed)
  0x00754E20  (unnamed)
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007F2500  CSWCObject::ClearAllQueuedCombatActions
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x00740960  CClientExoApp::GetAutoPaused
  0x0073F750  ClientApp::GetGuiInGame
  0x007CBDB0  (unnamed)
  0x00741780  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00541080  CSWSObject::ClearAllActions
struct offsets touched (top 10):
  +0x74 [dec 116]  x6  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EAX,ECX,EDX
  +0x1C [dec 28]  x3  via EAX,EDX
  +0x30 [dec 48]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x10 [dec 16]  x2  via ECX,EDX
  +0x458 [dec 1112]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00751750 FUN_00751750 size=833 ret=8 callers=4 callees=13

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00751750(int param_1,int param_2)

{
  bool bVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_48;
  int local_14;
  int local_8;
  
  if ((-1 < param_2) && (param_2 < 7)) {
    if (DAT_00a7fcc4 != 0) {
      FUN_007523e0();
    }
    FUN_00754e20();
    *(int *)(param_1 + 0x1c98) = param_2;
    if (*(int *)(param_1 + 0x7c + param_2 * 0xc) != 0) {
      FUN_0073fb90();
      iVar2 = FUN_007e5d80();
      local_8 = 0;
      if (*(int *)(param_1 + 0x1c7c + param_2 * 4) != -1) {
        for (local_14 = 0; local_14 < *(int *)(param_1 + 0x7c + param_2 * 0xc);
            local_14 = local_14 + 1) {
          if (*(int *)(*(int *)(param_1 + 0x78 + param_2 * 0xc) + 8 + local_14 * 0x3c) ==
              *(int *)(param_1 + 0x1c7c + param_2 * 4)) {
            local_8 = local_14 * 0x3c + *(int *)(param_1 + 0x78 + param_2 * 0xc);
            break;
          }
        }
      }
      if (local_8 == 0) {
        local_8 = *(int *)(param_1 + 0x78 + param_2 * 0xc);
      }
      if ((iVar2 != 0) && (local_8 != 0)) {
        piVar3 = (int *)FUN_0073f4d0(*(undefined4 *)(local_8 + 0x1c));
        if (piVar3 == (int *)0x0) {
          local_48 = 0;
        }
        else {
          local_48 = (**(code **)(*piVar3 + 0xc))();
        }
        if (local_48 != 0) {
          if ((*(uint *)(local_8 + 0x30) & 1) != 0) {
            if ((*(int *)(local_8 + 0xc) == 0) &&
               ((*(int *)(local_8 + 0xc) == 0 ||
                (((*(int *)(local_8 + 0x10) == 0 && (*(int *)(local_8 + 0x14) == 0)) &&
                 (*(int *)(local_8 + 0x18) == -1)))))) {
              bVar1 = false;
            }
            else {
              bVar1 = true;
            }
            if (bVar1) {
              if ((*(uint *)(iVar2 + 0x458) & 1) == 0) {
                FUN_007f2500();
                FUN_004122a0(6);
              }
              else {
                iVar4 = FUN_00740960();
                if (iVar4 != 0) {
                  uVar6 = 10;
                  uVar5 = 1;
                  FUN_0073f750(1,10);
                  FUN_007cbdb0(uVar5,uVar6);
                }
              }
              (**(code **)(local_8 + 0xc))(*(undefined4 *)(local_8 + 8),iVar2);
              FUN_00741780(1);
              if ((*(uint *)(iVar2 + 0x458) & 1) != 0) {
                return;
              }
              iVar2 = FUN_0077d800();
              if (iVar2 == 0) {
                return;
/* ... 36 more lines truncated by --max-lines */
```

### decoy: 0x00751710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751710: 16-byte aligned: YES
  00751710  55             PUSH EBP
  00751711  8B EC          MOV EBP,ESP
  00751713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751710
string literals (0):
direct callees (2):
  0x007523E0  (unnamed)
  0x007445D0  (unnamed)
struct offsets touched (top 0):
```

## Card B11b-2-51

GOG function: CSWGuiMainInterface::OnMenuButtonPressed  gog 0x00522d00  callconv=(blank)  param_size_bytes=4  gog_size=294  gog_retn=4
Candidate Steam address: 0x00751580 (size 294, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.18) | G8 mac_rotation: rotation:0x00522e30:0.54>0.33
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007516b0:STR0/CALLEE0/SIG0.188/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00522930 CSWGuiMainInterface::ClearOneAction -> 0x007511b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00522e30 CSWGuiMainInterface::OnCharacterRightClicked -> 0x007516b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x004da4f0 CGuiInGame::ShowSWInGameGui -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x50*,0x58*} | Steam {0x4*,0x50*,0x58*}
imm sets (* = in both, 15 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x18*,0x5d5c*,0xba4c*,0xbc1c*,0xbdec*,0xbfbc*,0xc18c*,0xc35c*,...(+1)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x18*,0x5d5c*,0xba4c*,0xbc1c*,0xbdec*,0xbfbc*,0xc18c*,0xc35c*,...(+1)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751580: 16-byte aligned: YES
  00751580  55             PUSH EBP
  00751581  8B EC          MOV EBP,ESP
  00751583  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751580
string literals (0):
direct callees (3):
  0x0073F750  ClientApp::GetGuiInGame
  0x007C9DF0  CGuiInGame::ShowSWInGameGui
  0x0073FEE0  CClientExoApp::SetInputClass
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x58 [dec 88]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00751580 FUN_00751580 size=294 ret=4 callers=1 callees=3

void __thiscall FUN_00751580(int param_1,int *param_2)

{
  int iVar1;
  undefined4 local_c;
  
  if (param_2[0x14 = byte +0x50] != 0) {
    iVar1 = (**(code **)(*param_2 + 0x58))();
    if (iVar1 == param_1 + 0xba4c) {
      local_c = 0;
    }
    else if (iVar1 == param_1 + 0xbc1c) {
      local_c = 1;
    }
    else if (iVar1 == param_1 + 0xbdec) {
      local_c = 2;
    }
    else if (iVar1 == param_1 + 0xbfbc) {
      local_c = 3;
    }
    else if (iVar1 == param_1 + 0xc18c) {
      local_c = 4;
    }
    else if (iVar1 == param_1 + 0xc35c) {
      local_c = 5;
    }
    else if ((iVar1 == param_1 + 0xc52c) || (iVar1 == param_1 + 0x5d5c)) {
      local_c = 6;
    }
    else {
      local_c = 7;
    }
    FUN_0073f750(local_c);
    iVar1 = FUN_007c9df0(local_c);
    if (iVar1 != 0) {
      FUN_0073fee0(2,1);
    }
  }
  return;
}
```

### decoy: 0x007516b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007516b0: 16-byte aligned: YES
  007516b0  55             PUSH EBP
  007516b1  8B EC          MOV EBP,ESP
  007516b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007516b0
string literals (0):
direct callees (2):
  0x00751750  (unnamed)
  0x00751710  (unnamed)
struct offsets touched (top 2):
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
```

## Card B11b-2-52

GOG function: CSWGuiMainInterface::GetActionIcon  gog 0x0051f830  callconv=(blank)  param_size_bytes=0  gog_size=1667  gog_retn=24
Candidate Steam address: 0x0074e0b0 (size 1667, retn 24)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.80) | G8 mac_rotation: pass
- anchors: A-STR 8 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'i_stancedefm'@0099df14; 'i_stancedef'@0099df24; 'i_swtchwpn'@0099df30; CALLEE 2 (callees: IsInTotalDefense 0x6daac0, GetIcon 0x615cc0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074e7f0:STR0/CALLEE0/SIG0.141/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0051f400 CSWGuiMainInterface::UpdateActionQueue_2 -> 0x0074dc80  gate_class=REVIEW
- GOG-order neighbour after:  0x0051ff70 CSWGuiMainInterface::ShowActionIcon -> 0x0074e7f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x004ed450 CSWSpellArray::GetSpell -> Steam ?  in candidate callees: n/a
  0x0050e730 CSWRules::GetFeat -> Steam ?  in candidate callees: n/a
  0x0061a5f0 CResRef::operator= -> Steam ?  in candidate callees: n/a
  0x0061a800 CResRef::operator=_4 -> Steam ?  in candidate callees: n/a
  0x0064bb00 CServerExoApp::GetItemByGameObjectID -> Steam 0x0051c0e0  in candidate callees: yes
  ... 3 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (11): 'i_attack'; 'i_stancedefm'; 'i_stancedef'; 'i_swtchwpn' ...
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0x10*,0x24*,0x5c*,0x64*,0x104*} | Steam {0x4*,0x8*,0x10*,0x24*,0x5c*,0x64*,0x104*}
imm sets (* = in both, 9 common): GOG {0x4*,0xa*,0xc*,0xd*,0x14*,0x18*,0x33*,0x38*,0x314*} | Steam {0x4*,0xa*,0xc*,0xd*,0x14*,0x18*,0x33*,0x38*,0x314*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=24 ; asm RET = 24
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 24)
```

### prologue
```
prologue for 0x0074e0b0: 16-byte aligned: YES
  0074e0b0  55             PUSH EBP
  0074e0b1  8B EC          MOV EBP,ESP
  0074e0b3  81 EC 14 03 00 00 SUB ESP,0x314
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074e0b0
string literals (30):
  0x0099DF88  at 0074e1b3  'i_attack'
  0x0099DF7C  at 0074e1c4  'i_attackm'
  0x0099DF74  at 0074e1da  'i_equip'
  0x0099DF68  at 0074e1eb  'i_equipm'
  0x0099DF5C  at 0074e204  'i_unequip'
  0x0099DF68  at 0074e218  'i_equipm'
  0x0099DF50  at 0074e27a  'i_useitemm'
  0x0099DF44  at 0074e2e2  'i_powerm'
  0x0099DF3C  at 0074e329  'i_featm'
  0x0099DF88  at 0074e382  'i_attack'
  0x0099DF7C  at 0074e396  'i_attackm'
  0x0099DF74  at 0074e3af  'i_equip'
direct callees (11):
  0x00710EB0  (unnamed)
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x00615CC0  CSWSItem::GetIcon
  0x00710CA0  (unnamed)
  0x006C1450  (unnamed)
  0x006A20F0  (unnamed)
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x006DAAC0  CSWSCreature::IsInTotalDefense
  0x0091DBA0  _memcpy
struct offsets touched (top 8):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x104 [dec 260]  x3  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x24 [dec 36]  x2  via ECX,EDX
  +0x5C [dec 92]  x2  via EDX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x74E74C [dec 7661388]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0074e0b0 FUN_0074e0b0 size=1667 ret=24 callers=1 callees=11

void FUN_0074e0b0(undefined4 param_1,undefined4 *param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  int local_318;
  undefined4 local_308;
  undefined1 local_2d8 [16];
  undefined1 local_2c8 [16];
  undefined1 local_2b8 [16];
  undefined1 local_2a8 [16];
  undefined1 local_298 [16];
  undefined1 local_288 [16];
  undefined1 local_278 [16];
  undefined1 local_268 [16];
  undefined1 local_258 [16];
  undefined1 local_248 [16];
  undefined1 local_238 [16];
  undefined1 local_228 [16];
  undefined1 local_218 [16];
  undefined1 local_208 [16];
  undefined1 local_1f8 [16];
  undefined1 local_1e8 [16];
  undefined1 local_1d8 [16];
  undefined1 local_1c8 [16];
  undefined1 local_1b8 [16];
  undefined1 local_1a8 [16];
  undefined1 local_198 [16];
  undefined1 local_188 [16];
  undefined1 local_178 [16];
  undefined1 local_168 [16];
  undefined1 local_158 [16];
  undefined1 local_148 [16];
  undefined1 local_138 [16];
  undefined1 local_128 [16];
  undefined1 local_118 [16];
  undefined1 local_108 [16];
  undefined1 local_f8 [16];
  undefined1 local_e8 [16];
  undefined1 local_d8 [16];
  undefined1 local_c8 [16];
  undefined1 local_b8 [16];
  undefined1 local_a8 [16];
  undefined1 local_98 [16];
  undefined1 local_88 [16];
  undefined1 local_78 [16];
  undefined1 local_68 [16];
  undefined1 local_58 [16];
  undefined1 local_48 [16];
  undefined1 local_38 [16];
  int local_28;
  int local_24;
  int local_20;
  undefined4 local_1c;
  uint local_18;
  int local_14;
  uint local_10;
  undefined4 local_c;
  int local_8;
  
  local_20 = 0;
  local_8 = 0;
  local_14 = 0;
  local_c = 0;
  local_10 = 0;
  FUN_00710eb0(local_38,&DAT_00986925);
  FUN_00710eb0(local_48,&DAT_00986925);
  if (param_2 == (undefined4 *)0x0) {
    local_308 = 0xc;
  }
  else {
    local_308 = *param_2;
  }
  local_1c = local_308;
  switch(local_308) {
  case 0xc:
    if (param_3 != 0) {
      local_18 = (uint)*(byte *)(param_3 + 0x10);
      local_10 = *(uint *)(param_3 + 0x5c);
/* ... 127 more lines truncated by --max-lines */
```

### decoy: 0x0074e7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074e7f0: 16-byte aligned: YES
  0074e7f0  55             PUSH EBP
  0074e7f1  8B EC          MOV EBP,ESP
  0074e7f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074e7f0
string literals (0):
direct callees (3):
  0x00710AD0  (unnamed)
  0x00710A10  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
struct offsets touched (top 2):
  +0x24 [dec 36]  x2  via EAX,EDX
  +0x48 [dec 72]  x2  via ECX,EDX
```

## Card B11b-2-53

GOG function: CSWGuiMainInterface::AddHelpPanel  gog 0x00517490  callconv=(blank)  param_size_bytes=5  gog_size=1955  gog_retn=8
Candidate Steam address: 0x00745e60 (size 1955, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.27) | G8 mac_rotation: pass
- anchors: A-STR 14 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR '%s%s%s'@0098ba58; 'cntrl_xb360_%s'@0099d904; 'panish'@0099d918; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00745d90:STR0/CALLEE0/SIG0.191/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00517390 CSWGuiMainInterfaceStatus::Update  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00517c60 CSWGuiHelpPanel::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00407260 unnamed -> Steam ?  in candidate callees: n/a
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam 0x0073fea0  in candidate callees: yes
  0x0042c6f0 Global::IsPsButtonIcons -> Steam ?  in candidate callees: n/a
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
  0x0086a900 Global::AurMakeCurrent -> Steam ?  in candidate callees: n/a
  0x008d5470 Global::NewCAurGUIImage -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x00912c10 CSWGuiBorderParams::SetFillImage -> Steam 0x00414840  in candidate callees: yes
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (15): '@cntrl_ps3_%s'; 'cntrl_xb360_%s'; 'panish'; 'erman' ...
disp sets (* = in both, 7 common): GOG {0x4*,0xc*,0x10*,0x20*,0x48*,0x6c*,0x6e*} | Steam {0x4*,0xc*,0x10*,0x20*,0x48*,0x6c*,0x6e*}
imm sets (* = in both, 20 common): GOG {0x2*,0x3*,0x4*,0x5*,0x7*,0x8*,0xc*,0x10*,0x14*,0x41*,0x324*,0x148b4*,0x14af0*,0x14b74*,...(+6)} | Steam {0x2*,0x3*,0x4*,0x5*,0x7*,0x8*,0xc*,0x10*,0x14*,0x41*,0x324*,0x148b4*,0x14af0*,0x14b74*,...(+6)}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00745e60: 16-byte aligned: YES
  00745e60  55             PUSH EBP
  00745e61  8B EC          MOV EBP,ESP
  00745e63  81 EC 24 03 00 00 SUB ESP,0x324
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00745e60
string literals (15):
  0x0099D99C  at 00745f83  'ios_help_1'
  0x0099D990  at 00745f93  'ios_help_2'
  0x0099D984  at 00745fa7  'ios_help_3'
  0x0099D978  at 00745fb7  'ios_help_4x'
  0x0099D96C  at 00745fc7  'ios_tut_1'
  0x0099D960  at 00745fdb  'ios_tut_2'
  0x0099D954  at 00745fe8  'ios_tut_3'
  0x0099D948  at 00745ff5  'ios_tut_4'
  0x0098BA58  at 00746032  '%s%s%s'
  0x0099D93C  at 00746095  'rench'
  0x0099D930  at 007460b9  'talian'
  0x0099D924  at 007460dd  'erman'
direct callees (12):
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x0091C860  _ftol2_sse
  0x00919671  (unnamed)
  0x00710840  (unnamed)
  0x00407260  (unnamed)
  0x0091D6F0  (unnamed)
  0x00780170  (unnamed)
  0x0047EB60  (unnamed)
  0x00478A20  Global::AurMakeCurrent
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
  0x00410530  CSWGuiManager::AddPanel
  0x009196EE  __security_check_cookie
struct offsets touched (top 9):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x20 [dec 32]  x4  via EAX,ECX,EDX
  +0x48 [dec 72]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via EDX
  +0x6C [dec 108]  x1  via EDX
  +0x6E [dec 110]  x1  via EAX
  +0x14AF0 [dec 84720]  x1  via EDX
  +0x15060 [dec 86112]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00745e60 FUN_00745e60 size=1955 ret=8 callers=2 callees=12

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00745e60(int param_1,undefined4 param_2,char param_3)

{
  char cVar1;
  int iVar2;
  char *pcVar3;
  undefined1 local_2e8 [16];
  undefined1 local_2d8 [16];
  undefined1 local_2c8 [16];
  undefined1 local_2b8 [16];
  int *local_2a8;
  int *local_2a4;
  float local_2a0;
  float local_29c;
  float local_298;
  float local_294;
  float local_290;
  float local_28c;
  float local_288;
  int *local_284;
  char *local_280;
  undefined4 local_27c;
  undefined *local_278;
  float local_274;
  undefined4 local_270;
  undefined4 local_26c;
  int local_268;
  int local_264;
  undefined *local_250;
  char local_24c [263];
  undefined1 local_145;
  char local_144 [260];
  uint local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  char local_2d;
  int local_2c;
  float local_28;
  float local_24;
  undefined4 local_20;
  undefined1 *local_1c;
  char *local_18;
  undefined1 local_11;
  float local_10;
  float local_c;
  char *local_8;
  
  local_40 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  iVar2 = FUN_0073fea0();
  local_274 = (float)(int)*(short *)(iVar2 + 0x6c);
  iVar2 = FUN_0073fea0();
  local_10 = (float)(int)*(short *)(iVar2 + 0x6e);
  local_270 = 0;
  local_26c = 0;
  local_268 = FUN_0091c860();
  local_264 = FUN_0091c860();
  local_c = 1.0;
  local_2c = 1;
  local_3c = 0;
  local_38 = 0;
  local_34 = 0;
  local_28 = 0.0;
  local_24 = 0.0;
  local_20 = 0;
  local_18 = "";
  local_2d = '\0';
  local_145 = 0;
  local_11 = 0;
  switch(param_2) {
  case 0:
    local_18 = "ios_help_1";
    local_145 = 1;
    break;
  case 1:
    local_18 = "ios_help_2";
    local_2d = '\x01';
/* ... 149 more lines truncated by --max-lines */
```

### decoy: 0x00745d90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00745d90: 16-byte aligned: YES
  00745d90  55             PUSH EBP
  00745d91  8B EC          MOV EBP,ESP
  00745d93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00745d90
string literals (0):
direct callees (2):
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
struct offsets touched (top 5):
  +0x48 [dec 72]  x3  via EAX,EDX
  +0x148 [dec 328]  x3  via EAX,ECX,EDX
  +0x14C [dec 332]  x3  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x150 [dec 336]  x1  via ECX
```

## Card B11b-2-54

GOG function: CGuiInGame::InitializeGlobal  gog 0x004ce110  callconv=(blank)  param_size_bytes=0  gog_size=515  gog_retn=0
Candidate Steam address: 0x007bde80 (size 515, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.70) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: GetModuleCamera 0x7819e0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007bdbc0:STR0/CALLEE0/SIG0.075/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004cde50 CGuiInGame::Destructor -> 0x00740070  gate_class=REJECT
- GOG-order neighbour after:  0x004ce320 CSWGuiPanel::GetActiveControl  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam 0x0073fea0  in candidate callees: yes
  0x0052c540 CSWGuiMessageBox::Constructor -> Steam ?  in candidate callees: n/a
  0x0052c960 CSWGuiMessageBox::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00586770 CSWGuiTutorialBox::Constructor -> Steam ?  in candidate callees: n/a
  0x0058a090 CSWGuiSkillInfoBox::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0042df60 CClientExoAppInternal::Initialize -> Steam 0x007819e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x38*,0xa0*,0xa4*,0xac*,0xb0*,0xb4*} | Steam {0x4*,0x38*,0xa0*,0xa4*,0xac*,0xb0*,0xb4*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x48*,0x9c8*,0x9dc*,0x4598*} | Steam {0x2*,0x3*,0x4*,0x48*,0x9c8*,0x9dc*,0x4598*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007bde80: 16-byte aligned: YES
  007bde80  55             PUSH EBP
  007bde81  8B EC          MOV EBP,ESP
  007bde83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007bde80 FUN_007bde80 size=515 ret=0 callers=1 callees=6

void __fastcall FUN_007bde80(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969757;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x38) == 0) {
    uVar1 = FUN_0073fea0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    *(undefined4 *)(param_1 + 0x38) = uVar1;
  }
  iVar2 = FUN_00919723(0x9c8);
  local_8 = 0;
  if (iVar2 == 0) {
    local_48 = 0;
  }
  else {
    local_48 = FUN_0075ae40(*(undefined4 *)(param_1 + 0x38));
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0xa0) = local_48;
  iVar2 = FUN_00919723(0x9c8);
  local_8 = 1;
  if (iVar2 == 0) {
    local_4c = 0;
  }
  else {
    local_4c = FUN_0075b370(*(undefined4 *)(param_1 + 0x38),1);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0xa4) = local_4c;
  iVar2 = FUN_00919723(0x4598);
  local_8 = 2;
  if (iVar2 == 0) {
    local_50 = 0;
  }
  else {
    local_50 = FUN_0089c5f0(*(undefined4 *)(param_1 + 0x38));
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0xac) = local_50;
  iVar2 = FUN_00919723(0x9dc);
  local_8 = 3;
  if (iVar2 == 0) {
    local_54 = 0;
  }
  else {
    local_54 = FUN_00898ca0(*(undefined4 *)(param_1 + 0x38));
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0xb0) = local_54;
  local_58 = (undefined4 *)FUN_00919723(0x9c8);
  local_8 = 4;
  if (local_58 == (undefined4 *)0x0) {
    local_58 = (undefined4 *)0x0;
  }
  else {
    FUN_0075ae40(*(undefined4 *)(param_1 + 0x38));
    *local_58 = CSWGuiControllerLossBox::vftable;
  }
  *(undefined4 **)(param_1 + 0xb4) = local_58;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007bdbc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bdbc0: 16-byte aligned: YES
  007bdbc0  55             PUSH EBP
  007bdbc1  8B EC          MOV EBP,ESP
  007bdbc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B11b-2-55

GOG function: CSWGuiMainInterface::OnCharacterRightClicked  gog 0x00522e30  callconv=(blank)  param_size_bytes=4  gog_size=84  gog_retn=4
Candidate Steam address: 0x007516b0 (size 84, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00751ab0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00522d00 CSWGuiMainInterface::OnMenuButtonPressed -> 0x00751580  gate_class=REVIEW
- GOG-order neighbour after:  0x00522ed0 CSWGuiMainInterface::DoPersonalAction -> 0x00751750  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00522e90 unnamed -> Steam ?  in candidate callees: n/a
  0x00522ed0 CSWGuiMainInterface::DoPersonalAction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x50*,0x5c*} | Steam {0x50*,0x5c*}
imm sets (* = in both, 3 common): GOG {0x4*,0x6*,0xc*} | Steam {0x4*,0x6*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007516b0: 16-byte aligned: YES
  007516b0  55             PUSH EBP
  007516b1  8B EC          MOV EBP,ESP
  007516b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007516b0
string literals (0):
direct callees (2):
  0x00751750  (unnamed)
  0x00751710  (unnamed)
struct offsets touched (top 2):
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007516b0 FUN_007516b0 size=84 ret=4 callers=0 callees=2

void FUN_007516b0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x50) != 0) {
    iVar1 = *(int *)(param_1 + 0x5c);
    if ((iVar1 < 0) || (5 < iVar1)) {
      FUN_00751710(iVar1 + -6,0);
    }
    else {
      FUN_00751750(iVar1,0);
    }
  }
  return;
}
```

### decoy: 0x00751ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751ab0: 16-byte aligned: YES
  00751ab0  55             PUSH EBP
  00751ab1  8B EC          MOV EBP,ESP
  00751ab3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751ab0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1C98 [dec 7320]  x2  via EAX,ECX
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
```

## Card B11b-2-56

GOG function: CSWGuiMainInterface::HandleButtonAction_3  gog 0x00523460  callconv=(blank)  param_size_bytes=4  gog_size=170  gog_retn=4
Candidate Steam address: 0x00751ce0 (size 170, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00751c30:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005233b0 CSWGuiActionMenuIos::HandleInputEvent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00523510 CSWGuiMainInterface::ShowActionQueue -> 0x00751d90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0040ddf0 CClientExoApp::SetAutoPaused -> Steam 0x00740930  in candidate callees: yes
  0x0040dee0 CClientExoApp::ShowTutorialWindow -> Steam 0x00740a20  in candidate callees: yes
  0x00523bd0 CSWGuiMainInterface::SelectNextPersonalAction -> Steam ?  in candidate callees: n/a
  0x005265a0 CSWGuiMainInterface::ResetPartyMemberItemUse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x50*,0x5c*} | Steam {0x4*,0x1c*,0x50*,0x5c*}
imm sets (* = in both, 6 common): GOG {0x4*,0x5*,0x7*,0xf*,0x1c*,0x7f000000*} | Steam {0x4*,0x5*,0x7*,0xf*,0x1c*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751ce0: 16-byte aligned: YES
  00751ce0  55             PUSH EBP
  00751ce1  8B EC          MOV EBP,ESP
  00751ce3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751ce0
string literals (0):
direct callees (5):
  0x00754E20  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x00740930  CClientExoApp::GetClientRangedAnimation
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x00752450  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x1C [dec 28]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
  +0x15414 [dec 87060]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00751ce0 FUN_00751ce0 size=170 ret=4 callers=0 callees=5

void __thiscall FUN_00751ce0(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    FUN_00754e20();
    uVar1 = *(undefined4 *)(param_2 + 0x5c);
    *(undefined4 *)(param_1 + 0x15414) = 1;
    iVar2 = FUN_0072fb00();
    if ((*(uint *)(iVar2 + 0x1c) >> 0xf & 1) != 0) {
      FUN_00740930(1,7);
    }
    FUN_00740a20(5,0x7f000000,0x7f000000,0);
    FUN_00752450(uVar1);
  }
  return;
}
```

### decoy: 0x00751c30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751c30: 16-byte aligned: YES
  00751c30  55             PUSH EBP
  00751c31  8B EC          MOV EBP,ESP
  00751c33  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11b-2-57

GOG function: CSWGuiMainInterface::UpdateActionQueue_2  gog 0x0051f400  callconv=(blank)  param_size_bytes=0  gog_size=1071  gog_retn=0
Candidate Steam address: 0x0074dc80 (size 1071, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0074e0b0:STR0/CALLEE0/SIG0.136/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051f1c0 CSWGuiMainInterface::RevertActionBarStates -> 0x0074da40  gate_class=REVIEW
- GOG-order neighbour after:  0x0051f830 CSWGuiMainInterface::GetActionIcon -> 0x0074e0b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x0051f830 CSWGuiMainInterface::GetActionIcon -> Steam ?  in candidate callees: n/a
  0x0051ff70 CSWGuiMainInterface::ShowActionIcon -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  ... 6 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0x48*,0xa84*,0xb00*,0xb18*,0x10dc*,0x5f2c*} | Steam {0x4*,0x8*,0x48*,0xa84*,0xb00*,0xb18*,0x10dc*,0x5f2c*}
imm sets (* = in both, 10 common): GOG {0x4*,0xc*,0xf*,0x2e*,0x3f*,0x9c*,0x100*,0x148*,0x5dc*,0xfffffffd*} | Steam {0x4*,0xc*,0xf*,0x2e*,0x3f*,0x9c*,0x100*,0x148*,0x5dc*,0xfffffffd*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0074dc80: 16-byte aligned: YES
  0074dc80  55             PUSH EBP
  0074dc81  8B EC          MOV EBP,ESP
  0074dc83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0074dc80
string literals (0):
direct callees (14):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00590880  CSWSCombatRound::GetCurrentAction
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x007107F0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0074E0B0  (unnamed)
  0x0074E7F0  (unnamed)
  0x0058C370  (unnamed)
  0x00733780  CExoString::~CExoString
  0x009196EE  __security_check_cookie
struct offsets touched (top 7):
  +0xB00 [dec 2816]  x4  via EAX,ECX,EDX
  +0x48 [dec 72]  x2  via ECX,EDX
  +0xA84 [dec 2692]  x2  via EAX,EDX
  +0xB18 [dec 2840]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x10DC [dec 4316]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0074dc80 FUN_0074dc80 size=1071 ret=0 callers=1 callees=14

void __fastcall FUN_0074dc80(int param_1)

{
  int iVar1;
  char cVar2;
  int local_ac;
  undefined4 local_88;
  int local_84;
  int local_80;
  int local_7c;
  undefined1 local_78 [16];
  int local_68[3 = byte +0xc];
  int local_5c;
  int *local_58;
  int local_4c;
  undefined4 local_48;
  undefined4 local_44;
  int local_40;
  int local_34;
  undefined4 local_30;
  undefined4 local_2c;
  int local_28;
  undefined1 local_24 [16];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009631b0;
  local_10 = ExceptionList;
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0073fb90(local_14);
  local_44 = FUN_007e5d80();
  local_30 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  local_84 = FUN_0077d800();
  local_5c = *(int *)(local_84 + 0x10dc);
  local_68[1 = byte +0x4] = 0x5dc;
  local_2c = *(undefined4 *)(local_5c + 0xa84);
  if ((*(int *)(local_5c + 0xa84) == 0) || (cVar2 = FUN_00590880(), cVar2 == '\0')) {
    local_ac = 0;
  }
  else {
    local_ac = 1;
  }
  local_4c = local_ac;
  local_68[2 = byte +0x8] = local_ac;
  local_7c = 0;
  local_58 = (int *)0x0;
  local_40 = 0;
  FUN_00733540();
  local_8 = 0;
  FUN_00733570(&DAT_00987fbc);
  local_8 = CONCAT31(local_8._1_3_,1);
  FUN_007107f0();
  FUN_007107f0();
  local_48 = 0;
  local_34 = 0;
  local_7c = local_84 + 0x100;
  local_28 = FUN_007a1720();
  local_80 = 0;
  while ((local_28 != 0 && (local_34 < 4))) {
    local_58 = (int *)FUN_00561430(local_28);
    if (local_58 != (int *)0x0) {
      if (((((*local_58 == 0x3f) && (local_4c != 0)) || (*local_58 == 0xc)) ||
          ((*local_58 == 0xf || (*local_58 == 0x2e)))) && (local_80 == 0)) {
        local_88 = 0xffffffff;
        local_40 = *(int *)(local_5c + 0xb18);
        if (local_40 == 0) {
          if (*local_58 != 0x3f) {
            FUN_0074e0b0(local_5c,local_58,0,&local_88,local_78,local_24);
            FUN_0074e7f0(&local_34,local_78,local_24,local_68[2 = byte +0x8],0);
            local_80 = 1;
          }
        }
        else {
          local_80 = 1;
          FUN_0074e0b0(local_5c,local_58,local_40,&local_88,local_78,local_24);
/* ... 37 more lines truncated by --max-lines */
```

### decoy: 0x0074e0b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074e0b0: 16-byte aligned: YES
  0074e0b0  55             PUSH EBP
  0074e0b1  8B EC          MOV EBP,ESP
  0074e0b3  81 EC 14 03 00 00 SUB ESP,0x314
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074e0b0
string literals (30):
  0x0099DF88  at 0074e1b3  'i_attack'
  0x0099DF7C  at 0074e1c4  'i_attackm'
  0x0099DF74  at 0074e1da  'i_equip'
  0x0099DF68  at 0074e1eb  'i_equipm'
  0x0099DF5C  at 0074e204  'i_unequip'
  0x0099DF68  at 0074e218  'i_equipm'
  0x0099DF50  at 0074e27a  'i_useitemm'
  0x0099DF44  at 0074e2e2  'i_powerm'
  0x0099DF3C  at 0074e329  'i_featm'
  0x0099DF88  at 0074e382  'i_attack'
  0x0099DF7C  at 0074e396  'i_attackm'
  0x0099DF74  at 0074e3af  'i_equip'
direct callees (11):
  0x00710EB0  (unnamed)
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x00615CC0  CSWSItem::GetIcon
  0x00710CA0  (unnamed)
  0x006C1450  (unnamed)
  0x006A20F0  (unnamed)
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x006DAAC0  CSWSCreature::IsInTotalDefense
  0x0091DBA0  _memcpy
struct offsets touched (top 8):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x104 [dec 260]  x3  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x24 [dec 36]  x2  via ECX,EDX
  +0x5C [dec 92]  x2  via EDX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x74E74C [dec 7661388]  x1  via ECX
```

## Card B11b-2-58

GOG function: CSWGuiMainInterface::InitializeMap  gog 0x00522390  callconv=(blank)  param_size_bytes=0  gog_size=577  gog_retn=0
Candidate Steam address: 0x00750c10 (size 577, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.31) | G8 mac_rotation: pass
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'lbl_map%s'@00993220; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00750790:STR0/CALLEE0/SIG0.391/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00521f10 CSWGuiMainInterface::DrawMap -> 0x00750790  gate_class=REVIEW
- GOG-order neighbour after:  0x00522830 CSWGuiLabel::`scalar  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: yes
  0x0061a160 CResRef::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x0061a280 CResRef::CopyToString -> Steam ?  in candidate callees: n/a
  0x0064b7b0 CServerExoApp::GetModule -> Steam 0x0051bd90  in candidate callees: yes
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'lbl_map%s'
disp sets (* = in both, 10 common): GOG {0x4*,0x8*,0x14*,0x1c*,0x20*,0x48*,0x238*,0x5ab4*,0x5d54*,0x5d58*} | Steam {0x4*,0x8*,0x14*,0x1c*,0x20*,0x48*,0x238*,0x5ab4*,0x5d54*,0x5d58*}
imm sets (* = in both, 11 common): GOG {0x2*,0x3*,0xc*,0x30*,0x70*,0x596c*,0x5ab4*,0x5b28*,0x5bfc*,0xfffffffc*,0xfffffffd*} | Steam {0x2*,0x3*,0xc*,0x30*,0x70*,0x596c*,0x5ab4*,0x5b28*,0x5bfc*,0xfffffffc*,0xfffffffd*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00750c10: 16-byte aligned: YES
  00750c10  55             PUSH EBP
  00750c11  8B EC          MOV EBP,ESP
  00750c13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00750c10 FUN_00750c10 size=577 ret=0 callers=2 callees=10

void __fastcall FUN_00750c10(int param_1)

{
  undefined4 uVar1;
  int local_7c;
  int local_74;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined1 local_40 [16];
  undefined4 local_30;
  int local_2c;
  int local_28;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00963240;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_2c = 0;
  local_14 = FUN_0051bd90(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  FUN_00733540();
  local_8 = 0;
  FUN_00733540();
  local_8 = CONCAT31(local_8._1_3_,1);
  FUN_00710930(local_1c);
  uVar1 = FUN_00734010();
  FUN_00734270(local_24,"lbl_map%s",uVar1);
  if (*(int *)(*(int *)(local_14 + 0x238) + 4) != 0) {
    local_30 = *(undefined4 *)(*(int *)(local_14 + 0x238) + 0x14);
    FUN_00710810(local_24);
    FUN_00414840(local_40,0);
    *(uint *)(param_1 + 0x5b48) = *(uint *)(param_1 + 0x5b48) & 0xfffffffc | 2;
    uVar1 = FUN_0091c860();
    local_48 = FUN_0091c860();
    local_50 = 0;
    local_4c = 0;
    local_44 = uVar1;
    (**(code **)(*(int *)(param_1 + 0x5ab4) + 4))(&local_50);
    local_2c = 1;
  }
  local_28 = FUN_0072fb00();
  if (local_28 != 0) {
    if ((local_2c == 0) || ((*(uint *)(local_28 + 0x1c) >> 3 & 1) == 0)) {
      local_74 = 0;
    }
    else {
      local_74 = 1;
    }
    *(uint *)(param_1 + 0x59b4) = *(uint *)(param_1 + 0x59b4) & 0xfffffffd | local_74 << 1;
    if ((local_2c == 0) || ((*(uint *)(local_28 + 0x1c) >> 3 & 1) == 0)) {
      local_7c = 0;
    }
    else {
      local_7c = 1;
    }
    *(uint *)(param_1 + 0x5c44) = *(uint *)(param_1 + 0x5c44) & 0xfffffffd | local_7c << 1;
  }
  local_8 = local_8 & 0xffffff00;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00750790 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00750790: 16-byte aligned: YES
  00750790  55             PUSH EBP
  00750791  8B EC          MOV EBP,ESP
  00750793  81 EC B4 00 00 00 SUB ESP,0xb4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00750790
string literals (0):
direct callees (15):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x0051BD90  CServerExoApp::GetModule
  0x00710C20  (unnamed)
  0x00750C10  (unnamed)
  0x005F7350  (unnamed)
  0x0091C860  _ftol2_sse
  0x0073F490  CClientExoApp::BeginIntro
  0x00463120  (unnamed)
  0x005F7170  (unnamed)
  0x004792C0  (unnamed)
  0x00479460  GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected)
  0x00751230  (unnamed)
  0x004799C0  (unnamed)
  0x004793F0  (unnamed)
struct offsets touched (top 10):
  +0x48 [dec 72]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x38 [dec 56]  x2  via EAX
  +0x5AB4 [dec 23220]  x2  via EDX
  +0x5D4C [dec 23884]  x2  via ECX
  +0x5D50 [dec 23888]  x2  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
```

## Card B11b-2-59

GOG function: CGuiInGame::HidePowersFeatsSkillsDebugMenu  gog 0x004dbc40  callconv=(blank)  param_size_bytes=0  gog_size=149  gog_retn=0
Candidate Steam address: 0x007cb4a0 (size 149, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CGuiInGame::HideLoadModuleDebugMenu | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.80 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb540:STR0/CALLEE1/SIG1/SRV0/VT0
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
disp sets (* = in both, 3 common): GOG {0x4*,0x38*,0x74,0x128*} | Steam {0x4*,0x38*,0x70,0x128*}
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

## Card B11b-2-60

GOG function: CSWGuiMainInterface::ClearOneAction  gog 0x00522930  callconv=(blank)  param_size_bytes=0  gog_size=119  gog_retn=0
Candidate Steam address: 0x007511b0 (size 119, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: RemoveLastAction 0x5904c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00750e60:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00522830 CSWGuiLabel::`scalar  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00522d00 CSWGuiMainInterface::OnMenuButtonPressed -> 0x00751580  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callees: yes
  0x00774b70 CSWSCombatRound::RemoveLastAction -> Steam 0x005904c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x10dc*} | Steam {0x4*,0x10dc*}
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
prologue for 0x007511b0: 16-byte aligned: YES
  007511b0  55             PUSH EBP
  007511b1  8B EC          MOV EBP,ESP
  007511b3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007511b0
string literals (0):
direct callees (5):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x005904C0  CSWSCombatRound::RemoveLastAction
  0x00541080  CSWSObject::ClearAllActions
struct offsets touched (top 2):
  +0x10DC [dec 4316]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007511b0 FUN_007511b0 size=119 ret=0 callers=2 callees=5

void __fastcall FUN_007511b0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  FUN_0073fb90(param_1);
  FUN_007e5d80();
  iVar1 = FUN_0077d800();
  if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10dc) != 0)) {
    iVar1 = FUN_005904c0(param_1,*(undefined4 *)(iVar1 + 0x10dc),*(int *)(iVar1 + 0x10dc),0,iVar1,
                         uVar2);
    if (iVar1 == 0) {
      FUN_00541080(1);
    }
  }
  return;
}
```

### decoy: 0x00750e60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00750e60: 16-byte aligned: YES
  00750e60  55             PUSH EBP
  00750e61  8B EC          MOV EBP,ESP
  00750e63  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00750e60
string literals (0):
direct callees (1):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```
