# Review cards B11b chunk 3

Answer each card with one TSV row in `steam_db_batches/B11b/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B11b-3-01

GOG function: CSWGuiMainInterface::OnCharacterExited  gog 0x00523a80  callconv=(blank)  param_size_bytes=0  gog_size=46  gog_retn=4
Candidate Steam address: 0x00752300 (size 46, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00752330:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00523a40 CSWGuiMainInterface::OnCharacterEntered -> 0x007522c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00523ab0 CSWGuiMainInterface::SetTarget -> 0x00752330  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x00752300: 16-byte aligned: YES
  00752300  55             PUSH EBP
  00752301  8B EC          MOV EBP,ESP
  00752303  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00752300
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x15230 [dec 86576]  x1  via EAX
  +0x15234 [dec 86580]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00752300 FUN_00752300 size=46 ret=4 callers=0 callees=0

void __fastcall FUN_00752300(int param_1)

{
  if (*(int *)(param_1 + 0x15230) != 0) {
    *(undefined4 *)(param_1 + 0x15234) = 0xffffffff;
  }
  return;
}
```

### decoy: 0x00752330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752330: 16-byte aligned: YES
  00752330  55             PUSH EBP
  00752331  8B EC          MOV EBP,ESP
  00752333  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752330
string literals (0):
direct callees (1):
  0x00752360  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via EAX
```

## Card B11b-3-02

GOG function: CSWGuiMainInterface::SetupPauseGuiExtent  gog 0x005246d0  callconv=(blank)  param_size_bytes=4  gog_size=192  gog_retn=4
Candidate Steam address: 0x00752f50 (size 192, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00753300:STR0/CALLEE0/SIG0.121/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00524620 CSWGuiMainInterface::AddFloatyText -> 0x00752ea0  gate_class=REVIEW
- GOG-order neighbour after:  0x00524790 CSWGuiMainInterface::ResetFont -> 0x00753010  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x008d03c0 Global::GetResolutionTextScalar -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x1c*,0x6c*,0xba54*,0xba5c*} | Steam {0x4*,0x8*,0x1c*,0x6c*,0xba54*,0xba5c*}
imm sets (* = in both, 3 common): GOG {0x4*,0x9*,0x1c*} | Steam {0x4*,0x9*,0x1c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00752f50: 16-byte aligned: YES
  00752f50  55             PUSH EBP
  00752f51  8B EC          MOV EBP,ESP
  00752f53  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00752f50 FUN_00752f50 size=192 ret=4 callers=1 callees=3

void __thiscall FUN_00752f50(int param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  int iVar2;
  bool bVar3;
  
  FUN_00479ab0();
  uVar1 = FUN_0091c860();
  param_2[1 = byte +0x4] = uVar1;
  iVar2 = FUN_0072fb00();
  if (iVar2 == 0) {
    bVar3 = true;
  }
  else {
    bVar3 = (*(uint *)(iVar2 + 0x1c) >> 9 & 1) == 0;
  }
  if (bVar3) {
    param_2[1 = byte +0x4] = *(int *)(param_1 + 0xba5c) + *(int *)(param_1 + 0xba54) + -1 + param_2[1 = byte +0x4];
  }
  uVar1 = FUN_0091c860((int)*(short *)(*(int *)(param_1 + 0x1c) + 0x6c) - param_2[2 = byte +0x8]);
  *param_2 = uVar1;
  return;
}
```

### decoy: 0x00753300 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00753300: 16-byte aligned: YES
  00753300  55             PUSH EBP
  00753301  8B EC          MOV EBP,ESP
  00753303  81 EC C0 00 00 00 SUB ESP,0xc0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00753300
string literals (0):
direct callees (7):
  0x00751140  (unnamed)
  0x0073F4D0  CClientExoApp::GetGameObject
  0x0073F490  CClientExoApp::BeginIntro
  0x00463120  (unnamed)
  0x0044B8A0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x0091C860  _ftol2_sse
struct offsets touched (top 10):
  +0x4 [dec 4]  x10  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x48 [dec 72]  x4  via EAX,ECX,EDX
  +0x148 [dec 328]  x4  via EAX,ECX,EDX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0x156 [dec 342]  x2  via EAX,ECX
  +0x158 [dec 344]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via EAX
```

## Card B11b-3-03

GOG function: CSWGuiMainInterface::ShowFlashingStatus  gog 0x00524590  callconv=(blank)  param_size_bytes=0  gog_size=139  gog_retn=0
Candidate Steam address: 0x00752e10 (size 139, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00752ea0:STR0/CALLEE1/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523ee0 CSWGuiMainInterface::UpdateStickyTarget -> 0x00752760  gate_class=REVIEW
- GOG-order neighbour after:  0x00524620 CSWGuiMainInterface::AddFloatyText -> 0x00752ea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0044cd50 CClientExoAppInternal::SetToolTipInstantMode -> Steam 0x007a0750  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x48*,0x732c*} | Steam {0x48*,0x732c*}
imm sets (* = in both, 5 common): GOG {0x10*,0x6f54*,0x709c*,0x71e4*,0xfffffffd*} | Steam {0x10*,0x6f54*,0x709c*,0x71e4*,0xfffffffd*}

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

## Card B11b-3-04

GOG function: CSWGuiMainInterface::PopulateMenus  gog 0x0051e760  callconv=(blank)  param_size_bytes=0  gog_size=1102  gog_retn=4
Candidate Steam address: 0x0074cf00 (size 162, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.14) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0074cc90:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0051e730 CSWGuiMainInterface::UpdateActionQueue -> 0x0074cfb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0051eca0 CSWGuiMainInterface::PopupActionMenu -> 0x0074d520  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: no
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: no
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x0042a5d0 CSWCCreature::GetPersonalActions -> Steam ?  in candidate callees: n/a
  0x00472100 CSWGuiInterfaceAction::Clear -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x00512c60 CSWGuiMainInterfaceAction::Show -> Steam ?  in candidate callees: n/a
  0x00512ea0 CSWGuiMainInterfaceAction::SetIcon -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x004608d0 CClientExoAppInternal::HandleMouseClickInWorld -> Steam 0x007b3ca0  in candidate callers: no
  0x004df250 CGuiInGame::RePopulateMainInterface -> Steam 0x007ceb50  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*,0x20,0x30,0x68,0x6c,0x78,0x7c,0x94,0x9c,0x74d,0x11ac,0x1c7c,...(+1)} | Steam {0x4*,0x8*,0xc*,0xa66c,0xa7b4,0xa8fc}
imm sets (* = in both, 1 common): GOG {0x4*,0x6,0x7,0xc,0x3c,0x68,0xcc,0x750} | Steam {0x4*,0x18,0xa66c,0xa670,0xa744,0xa75c,0xa7b4}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
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

## Card B11b-3-05

GOG function: CSWGuiMainInterface::UpdateStickyTarget  gog 0x00523ee0  callconv=(blank)  param_size_bytes=0  gog_size=1699  gog_retn=0
Candidate Steam address: 0x00752760 (size 1699, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.74) | G8 mac_rotation: pass
- anchors: A-STR 2 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'hostilereticle'@0099e13c; 'friendlyreticle'@0099e14c; CALLEE 1 (callees: GetServerObject 0x7f2540); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00752360:STR0/CALLEE0/SIG0.087/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523d50 CSWGuiMainInterface::SelectPrevPersonalAction -> 0x007525d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00524590 CSWGuiMainInterface::ShowFlashingStatus -> 0x00752e10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: yes
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x004776e0 CSWCObject::GetServerObject -> Steam 0x007f2540  in candidate callees: yes
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (2): 'hostilereticle'; 'friendlyreticle'
disp sets (* = in both, 17 common): GOG {0x4*,0x8*,0xc*,0x14*,0x2c*,0x3c*,0x48*,0x4c*,0x114*,0x118*,0x120*,0x128*,0x138*,0x13c*,...(+3)} | Steam {0x4*,0x8*,0xc*,0x14*,0x2c*,0x3c*,0x48*,0x4c*,0x114*,0x118*,0x120*,0x128*,0x138*,0x13c*,...(+3)}
imm sets (* = in both, 16 common): GOG {0x2*,0x5*,0x8*,0xa*,0xc*,0x10*,0x20*,0x24*,0x40*,0x41*,0xfc*,0x5810*,0x5814*,0x5884*,...(+2)} | Steam {0x2*,0x5*,0x8*,0xa*,0xc*,0x10*,0x20*,0x24*,0x40*,0x41*,0xfc*,0x5810*,0x5814*,0x5884*,...(+2)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00752760: 16-byte aligned: YES
  00752760  55             PUSH EBP
  00752761  8B EC          MOV EBP,ESP
  00752763  81 EC FC 00 00 00 SUB ESP,0xfc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00752760
string literals (2):
  0x0099E14C  at 00752d4e  'friendlyreticle'
  0x0099E13C  at 00752d5a  'hostilereticle'
direct callees (12):
  0x0073F4D0  CClientExoApp::GetGameObject
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0091CB24  (unnamed)
  0x0091C860  _ftol2_sse
  0x007F2540  CSWCObject::GetServerObject
  0x005E13C0  CSWSObject::GetReputation
  0x00710840  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
struct offsets touched (top 10):
  +0x4 [dec 4]  x7  via EAX,ECX
  +0x48 [dec 72]  x6  via EAX,ECX,EDX
  +0x4C [dec 76]  x6  via EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX
  +0x14 [dec 20]  x3  via EAX,EDX
  +0x138 [dec 312]  x3  via EAX,EDX
  +0x5958 [dec 22872]  x2  via EAX
  +0x2C [dec 44]  x1  via EDX
  +0x3C [dec 60]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00752760 FUN_00752760 size=1699 ret=0 callers=1 callees=12

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00752760(int param_1)

{
  int iVar1;
  int *piVar2;
  float10 fVar3;
  ulonglong uVar4;
  char *local_100;
  int *local_c8;
  undefined1 local_70 [16];
  int local_60;
  int local_5c;
  int local_58;
  int local_54;
  int local_50;
  float local_4c;
  undefined1 local_48 [12];
  undefined4 local_3c;
  undefined4 local_38;
  int local_34;
  int local_30;
  uint local_2c;
  float local_28;
  float local_24;
  undefined4 local_20;
  int local_1c;
  uint local_18;
  int *local_14;
  int local_10;
  uint local_c;
  int *local_8;
  
  local_c = DAT_00a7fcbc;
  local_14 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x5958));
  if (local_14 == (int *)0x0) {
    local_c8 = (int *)0x0;
  }
  else {
    local_c8 = (int *)(**(code **)(*local_14 + 0xc))();
  }
  local_8 = local_c8;
  iVar1 = FUN_0072fb00();
  if ((*(uint *)(iVar1 + 8) >> 1 & 1) != 0) {
    local_c = (uint)(local_c == 0);
  }
  if ((local_8 == (int *)0x0) || (local_c == 0)) {
    FUN_0073fb90();
    local_10 = FUN_007e5d80();
    if ((local_8 == (int *)0x0) || (local_10 == 0)) {
      *(uint *)(param_1 + 0x5858) = *(uint *)(param_1 + 0x5858) & 0xfffffffd;
    }
    else {
      local_3c = *(undefined4 *)(param_1 + 0x5814);
      local_38 = *(undefined4 *)(param_1 + 0x5818);
      local_34 = *(undefined4 *)(param_1 + 0x581c);
      local_30 = *(undefined4 *)(param_1 + 0x5820);
      local_28 = 0.0;
      local_24 = 0.0;
      local_20 = 0;
      local_2c = (**(code **)(*local_8 + 0x13c))(&local_28);
      local_2c = local_2c & 0xff;
      local_1c = 0x40;
      local_18 = 1;
      if (local_2c != 0) {
        FUN_004452b0(local_48,local_10 + 0x24,local_8 + 0x9 [dec 9] {int*: byte +0x24});
        fVar3 = (float10)FUN_0044b8a0(local_48,local_48);
        local_4c = (float)fVar3;
        iVar1 = (**(code **)(*local_8 + 0x2c))();
        if (iVar1 == 0) {
          if (_DAT_00987704 * _DAT_00987704 < local_4c ==
              (_DAT_00987704 * _DAT_00987704 == local_4c)) {
            if (_DAT_00986b68 * _DAT_00986b68 < local_4c) {
              FUN_0091cb24((double)local_4c);
              local_1c = FUN_0091c860();
              local_1c = 0x40 - local_1c;
            }
          }
/* ... 101 more lines truncated by --max-lines */
```

### decoy: 0x00752360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752360: 16-byte aligned: YES
  00752360  55             PUSH EBP
  00752361  8B EC          MOV EBP,ESP
  00752363  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752360
string literals (0):
direct callees (2):
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x007511B0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x68E4 [dec 26852]  x1  via EAX
```

## Card B11b-3-06

GOG function: CSWGuiMainInterface::OnPanelRemoved  gog 0x00524ea0  callconv=(blank)  param_size_bytes=0  gog_size=81  gog_retn=0
Candidate Steam address: 0x00753720 (size 81, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x00524a20:0.55>0.23
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: OnPanelRemoved 0x40f4c0); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00753680:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00524e00 CSWGuiMainInterface::PopupGamepadMenu -> 0x00753680  gate_class=REVIEW
- GOG-order neighbour after:  0x00524f00 CSWGamepadMenuIos::UpdateStealth  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: yes
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0xd588*,0xffb8*,0x148b4*} | Steam {0xd588*,0xffb8*,0x148b4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00753720: 16-byte aligned: YES
  00753720  55             PUSH EBP
  00753721  8B EC          MOV EBP,ESP
  00753723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00753720
string literals (0):
direct callees (2):
  0x00410670  (unnamed)
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00753720 FUN_00753720 size=81 ret=0 callers=0 callees=2

void __fastcall FUN_00753720(int param_1)

{
  FUN_00410670(param_1 + 0xd588);
  FUN_00410670(param_1 + 0xffb8);
  FUN_00410670(param_1 + 0x148b4);
  FUN_0040f4c0();
  return;
}
```

### decoy: 0x00753680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00753680: 16-byte aligned: YES
  00753680  55             PUSH EBP
  00753681  8B EC          MOV EBP,ESP
  00753683  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00753680
string literals (0):
direct callees (5):
  0x0073F750  ClientApp::GetGuiInGame
  0x00410800  GuiManager::Contains
  0x00410530  CSWGuiManager::AddPanel
  0x0073FEE0  CClientExoApp::SetInputClass
  0x00753850  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX
  +0xCC [dec 204]  x1  via EDX
```

## Card B11b-3-07

GOG function: CSWGuiMainInterface::GetActionsFromActionIndex  gog 0x00523ae0  callconv=(blank)  param_size_bytes=0  gog_size=75  gog_retn=0
Candidate Steam address: 0x00752360 (size 75, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: ShowTutorialWindow 0x740a20; callers: HandleInputEvent 0x7b12c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00752450:STR0/CALLEE1/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523ab0 CSWGuiMainInterface::SetTarget -> 0x00752330  gate_class=REVIEW
- GOG-order neighbour after:  0x00523b30 CSWGuiMainInterface::UpdateAndDrawFogOfWar -> 0x007523b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040dee0 CClientExoApp::ShowTutorialWindow -> Steam 0x00740a20  in candidate callees: yes
  0x00522930 CSWGuiMainInterface::ClearOneAction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x68e4*} | Steam {0x4*,0x68e4*}
imm sets (* = in both, 3 common): GOG {0x8*,0x20*,0x7f000000*} | Steam {0x8*,0x20*,0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00752360: 16-byte aligned: YES
  00752360  55             PUSH EBP
  00752361  8B EC          MOV EBP,ESP
  00752363  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00752360
string literals (0):
direct callees (2):
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x007511B0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x68E4 [dec 26852]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00752360 FUN_00752360 size=75 ret=0 callers=2 callees=2

void __fastcall FUN_00752360(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((*(uint *)(param_1 + 0x68e4) >> 1 & 1) != 0) {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    iVar1 = FUN_00740a20(0x20,0x7f000000,0x7f000000,0);
    if (iVar1 == 0) {
      FUN_007511b0(param_1,uVar2);
    }
  }
  return;
}
```

### decoy: 0x00752450 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752450: 16-byte aligned: YES
  00752450  55             PUSH EBP
  00752451  8B EC          MOV EBP,ESP
  00752453  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752450
string literals (0):
direct callees (3):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
```

## Card B11b-3-08

GOG function: CSWGuiMainInterface::SetTarget  gog 0x00523ab0  callconv=(blank)  param_size_bytes=4  gog_size=38  gog_retn=4
Candidate Steam address: 0x00752330 (size 38, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007523b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523a80 CSWGuiMainInterface::OnCharacterExited -> 0x00752300  gate_class=REVIEW
- GOG-order neighbour after:  0x00523ae0 CSWGuiMainInterface::GetActionsFromActionIndex -> 0x00752360  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00523ae0 CSWGuiMainInterface::GetActionsFromActionIndex -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x50*} | Steam {0x50*}
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
prologue for 0x00752330: 16-byte aligned: YES
  00752330  55             PUSH EBP
  00752331  8B EC          MOV EBP,ESP
  00752333  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00752330
string literals (0):
direct callees (1):
  0x00752360  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00752330 FUN_00752330 size=38 ret=4 callers=0 callees=1

void __thiscall FUN_00752330(undefined4 param_1,int param_2)

{
  if (*(int *)(param_2 + 0x50) != 0) {
    FUN_00752360(param_1);
  }
  return;
}
```

### decoy: 0x007523b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007523b0: 16-byte aligned: YES
  007523b0  55             PUSH EBP
  007523b1  8B EC          MOV EBP,ESP
  007523b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007523b0
string literals (0):
direct callees (1):
  0x007523E0  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via EAX
```

## Card B11b-3-09

GOG function: CSWGuiMainInterface::AddFloatyText  gog 0x00524620  callconv=(blank)  param_size_bytes=0  gog_size=170  gog_retn=0
Candidate Steam address: 0x00752ea0 (size 170, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.12) | G8 mac_rotation: rotation:0x00524a20:0.62>0.26
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00752e10:STR0/CALLEE1/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00524590 CSWGuiMainInterface::ShowFlashingStatus -> 0x00752e10  gate_class=REVIEW
- GOG-order neighbour after:  0x005246d0 CSWGuiMainInterface::SetupPauseGuiExtent -> 0x00752f50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0044cd50 CClientExoAppInternal::SetToolTipInstantMode -> Steam 0x007a0750  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x48*,0x6f9c*,0x732c*,0xa904*} | Steam {0x48*,0x6f9c*,0x732c*,0xa904*}
imm sets (* = in both, 5 common): GOG {0x14*,0x6f54*,0x709c*,0x71e4*,0xfffffffd*} | Steam {0x14*,0x6f54*,0x709c*,0x71e4*,0xfffffffd*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00752ea0: 16-byte aligned: YES
  00752ea0  55             PUSH EBP
  00752ea1  8B EC          MOV EBP,ESP
  00752ea3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00752ea0
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x48 [dec 72]  x6  via EAX,ECX,EDX
  +0x6F9C [dec 28572]  x1  via EAX
  +0x732C [dec 29484]  x1  via ECX
  +0xA904 [dec 43268]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00752ea0 FUN_00752ea0 size=170 ret=0 callers=1 callees=0

void __fastcall FUN_00752ea0(int param_1)

{
  *(uint *)(param_1 + 0x722c) = *(uint *)(param_1 + 0x722c) & 0xfffffffd;
  if ((*(uint *)(param_1 + 0xa904) >> 1 & 1) != 0) {
    *(uint *)(param_1 + 0x6f9c) = *(uint *)(param_1 + 0x6f9c) & 0xfffffffd | 2;
  }
  *(uint *)(param_1 + 0x70e4) =
       *(uint *)(param_1 + 0x70e4) & 0xfffffffd | (*(uint *)(param_1 + 0x6f9c) >> 1 & 1) << 1;
  *(undefined4 *)(param_1 + 0x732c) = 0;
  return;
}
```

### decoy: 0x00752e10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752e10: 16-byte aligned: YES
  00752e10  55             PUSH EBP
  00752e11  8B EC          MOV EBP,ESP
  00752e13  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752e10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x48 [dec 72]  x6  via EAX,ECX,EDX
  +0x732C [dec 29484]  x1  via EAX
```

## Card B11b-3-10

GOG function: CSWGuiMainInterface::ResetFont  gog 0x00524790  callconv=(blank)  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x00753010 (size 33, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: ResetFont 0x40ffb0)
- best decoy (neighbour/other candidate) scores: 0x00752f50:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005246d0 CSWGuiMainInterface::SetupPauseGuiExtent -> 0x00752f50  gate_class=REVIEW
- GOG-order neighbour after:  0x005247c0 CSWGuiFloatyText::CreateEntry  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005160c0 CSWGuiTargetActionMenu::Destructor -> Steam ?  in candidate callees: n/a
  0x0090e430 CSWGuiPanel::ResetFont -> Steam 0x0040ffb0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0xcc*} | Steam {0xcc*}

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

## Card B11b-3-11

GOG function: CSWGuiMainInterface::OnStealthButtonPressed  gog 0x00523760  callconv=(blank)  param_size_bytes=4  gog_size=99  gog_retn=4
Candidate Steam address: 0x00751fe0 (size 99, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00751fa0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523720 CSWGuiMainInterface::OnSoloButtonPressed -> 0x00751fa0  gate_class=REVIEW
- GOG-order neighbour after:  0x005237d0 CSWGuiMainInterface::OnCharacterClicked -> 0x00752050  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00426fe0 CSWCCreature::PartyMenuStealth -> Steam ?  in candidate callees: n/a
  0x0042be70 CSWCCreature::IsStealthCapable -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x50*} | Steam {0x4*,0x50*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751fe0: 16-byte aligned: YES
  00751fe0  55             PUSH EBP
  00751fe1  8B EC          MOV EBP,ESP
  00751fe3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751fe0
string literals (0):
direct callees (5):
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0077FE50  CSWCCreature::IsStealthCapable
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077B020  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00751fe0 FUN_00751fe0 size=99 ret=4 callers=0 callees=5

void __thiscall FUN_00751fe0(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    FUN_0073f450(param_1);
    iVar1 = FUN_0077fe50();
    if (iVar1 != 0) {
      uVar3 = 0;
      uVar2 = 0;
      FUN_0073fb90(0,0,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4),uVar4);
      FUN_007e5d80();
      FUN_0077b020(uVar2,uVar3);
    }
  }
  return;
}
```

### decoy: 0x00751fa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751fa0: 16-byte aligned: YES
  00751fa0  55             PUSH EBP
  00751fa1  8B EC          MOV EBP,ESP
  00751fa3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751fa0
string literals (0):
direct callees (2):
  0x0073F750  ClientApp::GetGuiInGame
  0x007CD510  CGuiInGame::ShowSoloModeQuery
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B11b-3-12

GOG function: CSWGuiMainInterface::PopulateMenus  gog 0x0051e760  callconv=(blank)  param_size_bytes=0  gog_size=1102  gog_retn=4
Candidate Steam address: 0x0074d520 (size 927, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.20) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0074d430:STR0/CALLEE0/SIG0.246/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0051e730 CSWGuiMainInterface::UpdateActionQueue -> 0x0074cfb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0051eca0 CSWGuiMainInterface::PopupActionMenu -> 0x0074d520  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: no
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: no
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x0042a5d0 CSWCCreature::GetPersonalActions -> Steam ?  in candidate callees: n/a
  0x00472100 CSWGuiInterfaceAction::Clear -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x00512c60 CSWGuiMainInterfaceAction::Show -> Steam ?  in candidate callees: n/a
  0x00512ea0 CSWGuiMainInterfaceAction::SetIcon -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x004608d0 CClientExoAppInternal::HandleMouseClickInWorld -> Steam 0x007b3ca0  in candidate callers: no
  0x004df250 CGuiInGame::RePopulateMainInterface -> Steam 0x007ceb50  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8,0xc,0x20*,0x30,0x68,0x6c,0x78,0x7c,0x94,0x9c,0x74d*,0x11ac,0x1c7c,...(+1)} | Steam {0x4*,0x20*,0x34,0x40,0x48,0x74d*}
imm sets (* = in both, 3 common): GOG {0x4*,0x6*,0x7,0xc,0x3c*,0x68,0xcc,0x750} | Steam {0x2,0x3,0x4*,0x6*,0x10,0x20,0x3c*,0x44,0x1d0,0xffb8,0xfffffffc,0xfffffffd}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
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

### decoy: 0x0074d430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074d430: 16-byte aligned: YES
  0074d430  55             PUSH EBP
  0074d431  8B EC          MOV EBP,ESP
  0074d433  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0074d430
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1C76 [dec 7286]  x1  via EAX
```

## Card B11b-3-13

GOG function: CSWGuiMainInterface::SelectNextPersonalAction  gog 0x00523bd0  callconv=(blank)  param_size_bytes=4  gog_size=371  gog_retn=4
Candidate Steam address: 0x00752450 (size 371, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00752360:STR0/CALLEE1/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523b60 CSWGuiMainInterface::OnClearAllButtonPressed -> 0x007523e0  gate_class=REVIEW
- GOG-order neighbour after:  0x00523d50 CSWGuiMainInterface::SelectPrevPersonalAction -> 0x007525d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x1c*,0x78*,0x7c*,0x1c7c*} | Steam {0x4*,0x8*,0x1c*,0x78*,0x7c*,0x1c7c*}
imm sets (* = in both, 6 common): GOG {0x4*,0x6*,0x7*,0xc*,0x28*,0x3c*} | Steam {0x4*,0x6*,0x7*,0xc*,0x28*,0x3c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00752450: 16-byte aligned: YES
  00752450  55             PUSH EBP
  00752451  8B EC          MOV EBP,ESP
  00752453  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00752450
string literals (0):
direct callees (3):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00752450 FUN_00752450 size=371 ret=4 callers=3 callees=3

void __thiscall FUN_00752450(int param_1,int param_2)

{
  int local_14;
  int local_8;
  
  if (((-1 < param_2) && (param_2 < 6)) && (*(int *)(param_1 + 0x7c + param_2 * 0xc) != 0)) {
    FUN_0073fb90();
    FUN_007e5d80();
    local_8 = 0;
    if (*(int *)(param_1 + 0x1c7c + param_2 * 4) != -1) {
      for (local_14 = 0; local_14 < *(int *)(param_1 + 0x7c + param_2 * 0xc);
          local_14 = local_14 + 1) {
        if (*(int *)(*(int *)(param_1 + 0x78 + param_2 * 0xc) + 8 + local_14 * 0x3c) ==
            *(int *)(param_1 + 0x1c7c + param_2 * 4)) {
          if (local_14 + 1 < *(int *)(param_1 + 0x7c + param_2 * 0xc)) {
            local_8 = (local_14 + 1) * 0x3c + *(int *)(param_1 + 0x78 + param_2 * 0xc);
          }
          else {
            local_8 = *(int *)(param_1 + 0x78 + param_2 * 0xc);
          }
          break;
        }
      }
    }
    if (1 < *(int *)(param_1 + 0x7c + param_2 * 0xc)) {
      if (local_8 == 0) {
        local_8 = *(int *)(param_1 + 0x78 + param_2 * 0xc) + 0x3c;
      }
      *(undefined4 *)(param_1 + 0x1c7c + param_2 * 4) = *(undefined4 *)(local_8 + 8);
      FUN_004122a0(7);
    }
  }
  return;
}
```

### decoy: 0x00752360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752360: 16-byte aligned: YES
  00752360  55             PUSH EBP
  00752361  8B EC          MOV EBP,ESP
  00752363  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752360
string literals (0):
direct callees (2):
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x007511B0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x68E4 [dec 26852]  x1  via EAX
```

## Card B11b-3-14

GOG function: CSWGuiMainInterface::OnClearAllButtonPressed  gog 0x00523b60  callconv=(blank)  param_size_bytes=0  gog_size=112  gog_retn=0
Candidate Steam address: 0x007523e0 (size 112, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: ClearAllActions 0x7f24c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00752450:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523b30 CSWGuiMainInterface::UpdateAndDrawFogOfWar -> 0x007523b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00523bd0 CSWGuiMainInterface::SelectNextPersonalAction -> 0x00752450  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x0042bd40 CSWCCreature::SetCombatMode -> Steam ?  in candidate callees: n/a
  0x00477660 CSWCObject::ClearAllActions -> Steam 0x007f24c0  in candidate callees: yes
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x70*,0x595c*} | Steam {0x4*,0x1c*,0x70*,0x595c*}
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
prologue for 0x007523e0: 16-byte aligned: YES
  007523e0  55             PUSH EBP
  007523e1  8B EC          MOV EBP,ESP
  007523e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007523e0
string literals (0):
direct callees (5):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077FD20  CSWCCreature::SetCombatMode
  0x007F24C0  CSWCObject::ClearAllActions
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x1C [dec 28]  x1  via EDX
  +0x70 [dec 112]  x1  via EDX
  +0x595C [dec 22876]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007523e0 FUN_007523e0 size=112 ret=0 callers=3 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_007523e0(int param_1)

{
  int iVar1;
  
  FUN_0073fb90();
  iVar1 = FUN_007e5d80();
  if (iVar1 != 0) {
    FUN_0077fd20(0);
    FUN_007f24c0();
    FUN_004122a0(0);
    *(undefined4 *)(param_1 + 0x595c) = _DAT_00987474;
    *(undefined4 *)(param_1 + 0x70) = _DAT_00987474;
  }
  return;
}
```

### decoy: 0x00752450 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752450: 16-byte aligned: YES
  00752450  55             PUSH EBP
  00752451  8B EC          MOV EBP,ESP
  00752453  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752450
string literals (0):
direct callees (3):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
```

## Card B11b-3-15

GOG function: CSWGuiMainInterface::PopupGamepadMenu  gog 0x00524e00  callconv=(blank)  param_size_bytes=0  gog_size=151  gog_retn=0
Candidate Steam address: 0x00753680 (size 151, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00753780:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00524a80 CSWGuiFloatyText::Update  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00524ea0 CSWGuiMainInterface::OnPanelRemoved -> 0x00753720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x00524fd0 CSWGamepadMenuIos::InitGamepadMenu -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0xcc*} | Steam {0x4*,0xcc*}
imm sets (* = in both, 3 common): GOG {0x2*,0x14*,0xd588*} | Steam {0x2*,0x14*,0xd588*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00753680: 16-byte aligned: YES
  00753680  55             PUSH EBP
  00753681  8B EC          MOV EBP,ESP
  00753683  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00753680
string literals (0):
direct callees (5):
  0x0073F750  ClientApp::GetGuiInGame
  0x00410800  GuiManager::Contains
  0x00410530  CSWGuiManager::AddPanel
  0x0073FEE0  CClientExoApp::SetInputClass
  0x00753850  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX
  +0xCC [dec 204]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00753680 FUN_00753680 size=151 ret=0 callers=1 callees=5

void FUN_00753680(void)

{
  int iVar1;
  int iVar2;
  int extraout_var;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 uVar6;
  
  uVar6 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  iVar1 = FUN_0073f750();
  iVar5 = *(int *)(iVar1 + 0xcc);
  if (((iVar5 == 0) && (DAT_00a7ead0 == 0)) &&
     (iVar3 = extraout_var,
     iVar2 = CMFCVisualManagerBitmapCache::IsCached(DAT_00a1b49c,(CSize *)(extraout_var + 0xd588)),
     iVar2 == 0)) {
    FUN_00410530(iVar3 + 0xd588,1,1);
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    FUN_0073fee0(2,0);
    FUN_00753850(iVar3,uVar4,iVar5,iVar1,uVar6);
  }
  return;
}
```

### decoy: 0x00753780 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00753780: 16-byte aligned: YES
  00753780  55             PUSH EBP
  00753781  8B EC          MOV EBP,ESP
  00753783  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11b-3-16

GOG function: CGuiInGame::SetTutorialReason  gog 0x004e00e0  callconv=(blank)  param_size_bytes=1  gog_size=48  gog_retn=4
Candidate Steam address: 0x007cf8f0 (size 112, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CGuiInGame::GetCanHeadFollowInDialog | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cf960:STR0/CALLEE1/SIG0.375/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0060 CGuiInGame::TutorialReasonWillShowWindow -> 0x007cf960  gate_class=AUTO
- GOG-order neighbour after:  0x004e0110 CGuiInGame::TutorialReasonHasShown -> 0x007cfa10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00586a00 CSWGuiTutorialBox::SetTutorialReason -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0044afd0 CClientExoAppInternal::ShowTutorialWindow -> Steam 0x0079e9d0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xb0} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0xc,0xf28,0x7f000000}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
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

### decoy: 0x007cf960 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cf960: 16-byte aligned: YES
  007cf960  55             PUSH EBP
  007cf961  8B EC          MOV EBP,ESP
  007cf963  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11b-3-17

GOG function: CSWGuiMainInterface::RemoveFloatyText  gog 0x00524a20  callconv=(blank)  param_size_bytes=0  gog_size=94  gog_retn=0
Candidate Steam address: 0x007532a0 (size 94, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00753300:STR0/CALLEE0/SIG0.175/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005247c0 CSWGuiFloatyText::CreateEntry  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00524a80 CSWGuiFloatyText::Update  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x10*,0x154*} | Steam {0x4*,0x8*,0x10*,0x154*}
imm sets (* = in both, 2 common): GOG {0x4*,0x5*} | Steam {0x4*,0x5*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007532a0: 16-byte aligned: YES
  007532a0  55             PUSH EBP
  007532a1  8B EC          MOV EBP,ESP
  007532a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007532a0
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x154 [dec 340]  x3  via EAX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x10 [dec 16]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007532a0 FUN_007532a0 size=94 ret=0 callers=1 callees=0

bool __fastcall FUN_007532a0(int *param_1)

{
  byte bVar1;
  
  *(char *)(param_1 + 0x55) = (char)param_1[0x55 = byte +0x154] + '\x01';
  bVar1 = *(byte *)(param_1 + 0x55);
  if (bVar1 < 6) {
    param_1[2 = byte +0x8] = param_1[2 = byte +0x8] - param_1[4 = byte +0x10];
    (**(code **)(*param_1 + 0x4 [dec 4] {int*: byte +0x10}))(param_1 + 0x1 [dec 1] {int*: byte +0x4});
  }
  return bVar1 < 6;
}
```

### decoy: 0x00753300 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00753300: 16-byte aligned: YES
  00753300  55             PUSH EBP
  00753301  8B EC          MOV EBP,ESP
  00753303  81 EC C0 00 00 00 SUB ESP,0xc0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00753300
string literals (0):
direct callees (7):
  0x00751140  (unnamed)
  0x0073F4D0  CClientExoApp::GetGameObject
  0x0073F490  CClientExoApp::BeginIntro
  0x00463120  (unnamed)
  0x0044B8A0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x0091C860  _ftol2_sse
struct offsets touched (top 10):
  +0x4 [dec 4]  x10  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x48 [dec 72]  x4  via EAX,ECX,EDX
  +0x148 [dec 328]  x4  via EAX,ECX,EDX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0x156 [dec 342]  x2  via EAX,ECX
  +0x158 [dec 344]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via EAX
```

## Card B11b-3-18

GOG function: CSWGuiMainInterface::OnCancelPartyItemSelect  gog 0x00526570  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00754df0 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00754e50:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00526530 CSWGuiMainInterface::OnSwapWeaponsPressed -> 0x00754db0  gate_class=REVIEW
- GOG-order neighbour after:  0x005265a0 CSWGuiMainInterface::ResetPartyMemberItemUse -> 0x00754e20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005265a0 CSWGuiMainInterface::ResetPartyMemberItemUse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x50*} | Steam {0x50*}
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
prologue for 0x00754df0: 16-byte aligned: YES
  00754df0  55             PUSH EBP
  00754df1  8B EC          MOV EBP,ESP
  00754df3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00754df0
string literals (0):
direct callees (1):
  0x00754E20  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00754df0 FUN_00754df0 size=44 ret=4 callers=0 callees=1

void __thiscall FUN_00754df0(undefined4 param_1,int param_2)

{
  if ((param_2 != 0) && (*(int *)(param_2 + 0x50) != 0)) {
    FUN_00754e20(param_1);
  }
  return;
}
```

### decoy: 0x00754e50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00754e50: 16-byte aligned: YES
  00754e50  55             PUSH EBP
  00754e51  8B EC          MOV EBP,ESP
  00754e53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00754e50
string literals (0):
direct callees (1):
  0x0058C540  CExoArray<ActionEntry>::SetAllocated
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B11b-3-19

GOG function: CSWGuiMainInterface::OnMenuButtonPressed  gog 0x00522d00  callconv=(blank)  param_size_bytes=4  gog_size=294  gog_retn=4
Candidate Steam address: 0x007516b0 (size 84, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: rotation:0x00522e30:0.54>0.33
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.19 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00751580:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00522930 CSWGuiMainInterface::ClearOneAction -> 0x007511b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00522e30 CSWGuiMainInterface::OnCharacterRightClicked -> 0x007516b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: no
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: no
  0x004da4f0 CGuiInGame::ShowSWInGameGui -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x50*,0x58} | Steam {0x50*,0x5c}
imm sets (* = in both, 2 common): GOG {0x2,0x3,0x4*,0x5,0x6*,0x7,0x18,0x5d5c,0xba4c,0xbc1c,0xbdec,0xbfbc,0xc18c,0xc35c,...(+1)} | Steam {0x4*,0x6*,0xc}

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

### decoy: 0x00751580 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751580: 16-byte aligned: YES
  00751580  55             PUSH EBP
  00751581  8B EC          MOV EBP,ESP
  00751583  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11b-3-20

GOG function: CSWGuiMainInterface::OnMenuButtonPressed  gog 0x00522d00  callconv=(blank)  param_size_bytes=4  gog_size=294  gog_retn=4
Candidate Steam address: 0x00751230 (size 842, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x00522e30:0.54>0.33
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00751580:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00522930 CSWGuiMainInterface::ClearOneAction -> 0x007511b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00522e30 CSWGuiMainInterface::OnCharacterRightClicked -> 0x007516b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: no
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: no
  0x004da4f0 CGuiInGame::ShowSWInGameGui -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x50,0x58} | Steam {0x1,0x4*,0x8,0x20,0x238,0x5d4c,0x5d50,0x5d54,0x5d58,0xd584}
imm sets (* = in both, 2 common): GOG {0x2,0x3,0x4*,0x5*,0x6,0x7,0x18,0x5d5c,0xba4c,0xbc1c,0xbdec,0xbfbc,0xc18c,0xc35c,...(+1)} | Steam {0x4*,0x5*,0x1f,0x80,0x8000001f,0xffffffe0}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751230: 16-byte aligned: YES
  00751230  55             PUSH EBP
  00751231  8B EC          MOV EBP,ESP
  00751233  81 EC 80 00 00 00 SUB ESP,0x80
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00751230
string literals (0):
direct callees (4):
  0x0051BD90  CServerExoApp::GetModule
  0x005F7010  (unnamed)
  0x0091C860  _ftol2_sse
  0x0047EA60  GUI quad-batch flush (drains pending texture builds via 0x427F30)
struct offsets touched (top 9):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x5D4C [dec 23884]  x3  via EAX,ECX
  +0x5D50 [dec 23888]  x3  via EAX,ECX,EDX
  +0xD584 [dec 54660]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EAX
  +0x20 [dec 32]  x1  via EDX
  +0x238 [dec 568]  x1  via EDX
  +0x5D54 [dec 23892]  x1  via ECX
  +0x5D58 [dec 23896]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00751230 FUN_00751230 size=842 ret=4 callers=1 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00751230(int param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  float local_58;
  int local_54;
  int local_50;
  int local_4c;
  int local_48;
  uint local_44;
  uint local_40;
  float local_3c;
  int local_38;
  float local_34;
  float local_30;
  int local_2c;
  int local_28;
  float local_24;
  float local_20;
  int local_1c;
  int local_18;
  undefined1 local_14 [4];
  int local_10;
  int local_c;
  int local_8;
  
  iVar1 = FUN_0051bd90();
  local_8 = *(int *)(iVar1 + 0x238);
  if ((local_8 != 0) && (*(int *)(local_8 + 4) != 0)) {
    local_18 = 0;
    local_18 = FUN_005f7010(local_14,&local_c,&local_10);
    if (local_18 != 0) {
      local_24 = (float)*(int *)(param_1 + 0x5d54) / (float)local_c;
      local_58 = (float)*(int *)(param_1 + 0x5d58) / (float)local_10;
      local_3c = local_24 * (float)_DAT_009884a8;
      local_34 = local_58 * (float)_DAT_009884a8;
      iVar1 = FUN_0091c860();
      iVar2 = FUN_0091c860();
      local_38 = -iVar2 - iVar1;
      iVar1 = FUN_0091c860();
      iVar1 = iVar1 + local_38;
      iVar2 = FUN_0091c860();
      local_50 = iVar1 + iVar2 * 2;
      iVar1 = FUN_0091c860();
      iVar2 = FUN_0091c860();
      local_1c = -iVar2 - iVar1;
      iVar1 = FUN_0091c860();
      iVar1 = iVar1 + local_1c;
      iVar2 = FUN_0091c860();
      local_4c = iVar1 + iVar2 * 2;
      local_20 = (float)local_1c * local_58 + (float)param_2[1 = byte +0x4];
      local_54 = (local_c + 1) * local_1c;
      if (local_1c < 0) {
        local_20 = (float)-local_1c * local_58 + local_20;
        local_54 = (local_c + 1) * -local_1c + local_54;
        local_1c = 0;
      }
      if (local_10 < local_4c) {
        local_4c = local_10;
      }
      if (local_38 < 0) {
        local_38 = 0;
      }
      if (local_c < local_50) {
        local_50 = local_c;
      }
      for (local_28 = local_1c; local_28 <= local_4c; local_28 = local_28 + 1) {
        local_30 = (float)local_38 * local_24 + (float)*param_2;
        for (local_2c = local_38; local_2c <= local_50; local_2c = local_2c + 1) {
          local_44 = local_54 + local_2c & 0x8000001f;
          if ((int)local_44 < 0) {
            local_44 = (local_44 - 1 | 0xffffffe0) + 1;
          }
/* ... 24 more lines truncated by --max-lines */
```

### decoy: 0x00751580 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751580: 16-byte aligned: YES
  00751580  55             PUSH EBP
  00751581  8B EC          MOV EBP,ESP
  00751583  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11b-3-21

GOG function: CSWGuiMainInterface::OnCharacterEntered  gog 0x00523a40  callconv=(blank)  param_size_bytes=4  gog_size=63  gog_retn=4
Candidate Steam address: 0x007522c0 (size 63, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00752300:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005237d0 CSWGuiMainInterface::OnCharacterClicked -> 0x00752050  gate_class=REVIEW
- GOG-order neighbour after:  0x00523a80 CSWGuiMainInterface::OnCharacterExited -> 0x00752300  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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
prologue for 0x007522c0: 16-byte aligned: YES
  007522c0  55             PUSH EBP
  007522c1  8B EC          MOV EBP,ESP
  007522c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007522c0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x5C [dec 92]  x1  via EDX
  +0x15230 [dec 86576]  x1  via EAX
  +0x15234 [dec 86580]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007522c0 FUN_007522c0 size=63 ret=4 callers=0 callees=0

void __thiscall FUN_007522c0(int param_1,int param_2)

{
  if ((*(int *)(param_1 + 0x15230) != 0) && (param_2 != 0)) {
    *(int *)(param_1 + 0x15234) = *(int *)(param_2 + 0x5c) + -1;
  }
  return;
}
```

### decoy: 0x00752300 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752300: 16-byte aligned: YES
  00752300  55             PUSH EBP
  00752301  8B EC          MOV EBP,ESP
  00752303  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752300
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x15230 [dec 86576]  x1  via EAX
  +0x15234 [dec 86580]  x1  via EDX
```

## Card B11b-3-22

GOG function: CSWGuiMainInterface::UpdateActionDescription  gog 0x0051e410  callconv=(blank)  param_size_bytes=0  gog_size=613  gog_retn=0
Candidate Steam address: 0x0074ad90 (size 1551, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.16) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0074ad10:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051daa0 CSWGuiMainInterface::Update -> 0x0074c320  gate_class=REVIEW
- GOG-order neighbour after:  0x0051e680 CSWGuiMainInterface::SetActionDescription -> 0x0074cf00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam 0x007408f0  in candidate callees: no
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x0051e680 CSWGuiMainInterface::SetActionDescription -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: no
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '%s (%d)'
disp sets (* = in both, 1 common): GOG {0x4*,0x8,0x34,0x48,0x70,0x74,0x78,0x7c,0x218,0x3e8,0x5b8,0x1c7c,0x1c98,0x733c,...(+1)} | Steam {0x4*,0xd584}
imm sets (* = in both, 6 common): GOG {0x5*,0xc*,0x10*,0x3c*,0x41*,0x44,0x750*} | Steam {0x2,0x3,0x4,0x5*,0x6,0x7,0x8,0x9,0xa,0xb,0xc*,0xd,0xe,0xf,...(+114)}

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

### decoy: 0x0074ad10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0074ad10: 16-byte aligned: YES
  0074ad10  55             PUSH EBP
  0074ad11  8B EC          MOV EBP,ESP
  0074ad13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0074ad10
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x0041A3D0  CSWGuiButton::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B11b-3-23

GOG function: CSWGuiMainInterface::UpdateActionQueue  gog 0x0051e730  callconv=(blank)  param_size_bytes=4  gog_size=48  gog_retn=8
Candidate Steam address: 0x0074d930 (size 203, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.20) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0074d8f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0051e680 CSWGuiMainInterface::SetActionDescription -> 0x0074cf00  gate_class=REVIEW
- GOG-order neighbour after:  0x0051e760 CSWGuiMainInterface::PopulateMenus -> 0x0074cfe0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x68,0x595c} | Steam {0x8,0xf0,0x1c76,0x1c7c,0x1c98}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x6,0x8*,0xc,0x3c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
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

## Card B11b-3-24

GOG function: CGuiInGame::SetTutorialReason  gog 0x004e00e0  callconv=(blank)  param_size_bytes=1  gog_size=48  gog_retn=4
Candidate Steam address: 0x007cf9e0 (size 48, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cf960:STR0/CALLEE1/SIG0.375/SRV0/VT0
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
disp sets (* = in both, 1 common): GOG {0xb0*} | Steam {0xb0*}
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
prologue for 0x007cf9e0: 16-byte aligned: YES
  007cf9e0  55             PUSH EBP
  007cf9e1  8B EC          MOV EBP,ESP
  007cf9e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cf9e0
string literals (0):
direct callees (1):
  0x00898F60  (unnamed)
struct offsets touched (top 1):
  +0xB0 [dec 176]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007cf9e0 FUN_007cf9e0 size=48 ret=4 callers=1 callees=1

undefined4 __thiscall FUN_007cf9e0(int param_1,undefined1 param_2)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0xb0) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_00898f60(param_2);
  }
  return uVar1;
}
```

### decoy: 0x007cf960 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cf960: 16-byte aligned: YES
  007cf960  55             PUSH EBP
  007cf961  8B EC          MOV EBP,ESP
  007cf963  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B11b-3-25

GOG function: CGuiInGame::ShowGameMenuChangeButtons  gog 0x005a4790  callconv=(blank)  param_size_bytes=4  gog_size=80  gog_retn=4
Candidate Steam address: 0x00894110 (size 46, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00893950:STR0/CALLEE0/SIG0.086/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a4650 CSWGuiInGameEquip::ShowCantEquipMessage  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005a47e0 CSWGuiInGameEquip::CopyItem  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: no
  0x004e2670 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0xc} | Steam {0x4*,0x14}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00894110: 16-byte aligned: YES
  00894110  55             PUSH EBP
  00894111  8B EC          MOV EBP,ESP
  00894113  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00894110
string literals (0):
direct callees (2):
  0x00894140  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00894110 FUN_00894110 size=46 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00894110(undefined4 param_1,uint param_2)

{
  FUN_00894140();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x00893950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00893950: 16-byte aligned: YES
  00893950  55             PUSH EBP
  00893951  8B EC          MOV EBP,ESP
  00893953  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00893950
string literals (13):
  0x009A67E0  at 00893a81  'map_p'
  0x009A67D8  at 00893aa1  'LBL_Map'
  0x009A67CC  at 00893adb  'LBL_MapNote'
  0x009A67C0  at 00893b18  'LBL_Area'
  0x0099E554  at 00893b54  'BTN_EXIT'
  0x009A67B8  at 00893ba7  'BTN_UP'
  0x009A67AC  at 00893be4  'BTN_DOWN'
  0x0099E548  at 00893c21  'LBL_BAR1'
  0x0099E53C  at 00893c5d  'LBL_BAR2'
  0x0099E530  at 00893c9a  'LBL_BAR3'
  0x0099E524  at 00893cd7  'LBL_BAR4'
  0x0099E518  at 00893d13  'LBL_BAR5'
direct callees (18):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x00419740  (unnamed)
  0x0041A510  (unnamed)
  0x0091D67E  (unnamed)
  0x00895620  CSWGuiMapHider::Constructor
  0x00416290  CSWGuiImage::Constructor
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x00760EA0  (unnamed)
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x0091C860  _ftol2_sse
  0x0040FD80  CSWGuiPanel::AddControl
  0x00418AF0  CSWGuiControl::AddEvent
  0x004149F0  (unnamed)
struct offsets touched (top 8):
  +0x48 [dec 72]  x8  via EAX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x1410 [dec 5136]  x4  via EAX,ECX
  +0x6C [dec 108]  x1  via EAX
  +0x6E [dec 110]  x1  via EDX
  +0x1160 [dec 4448]  x1  via EDX
  +0x13B8 [dec 5048]  x1  via EAX
  +0x140C [dec 5132]  x1  via EAX
```

## Card B11b-3-26

GOG function: CSWGuiMainInterface::UpdateActionQueue  gog 0x0051e730  callconv=(blank)  param_size_bytes=4  gog_size=48  gog_retn=8
Candidate Steam address: 0x0074b3a0 (size 68, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0074b3f0:STR0/CALLEE0/SIG0.012/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0051e680 CSWGuiMainInterface::SetActionDescription -> 0x0074cf00  gate_class=REVIEW
- GOG-order neighbour after:  0x0051e760 CSWGuiMainInterface::PopulateMenus -> 0x0074cfe0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x68,0x595c} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*,0xcc}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
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

## Card B11b-3-27

GOG function: CSWGuiMainInterface::OnSwapWeaponsPressed  gog 0x00526530  callconv=(blank)  param_size_bytes=0  gog_size=55  gog_retn=4
Candidate Steam address: 0x00754db0 (size 55, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: HandleInputEvent 0x7b12c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00754df0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005260d0 CSWGamepadMenuIos::HandleInputEvent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00526570 CSWGuiMainInterface::OnCancelPartyItemSelect -> 0x00754df0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: yes
  0x004299d0 CSWCCreature::ActionMenuSwitchWeapons -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x00754db0: 16-byte aligned: YES
  00754db0  55             PUSH EBP
  00754db1  8B EC          MOV EBP,ESP
  00754db3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00754db0
string literals (0):
direct callees (2):
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0077DA50  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00754db0 FUN_00754db0 size=55 ret=4 callers=1 callees=2

void __fastcall FUN_00754db0(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_0073f450(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  if (iVar1 != 0) {
    FUN_0077da50(0,0);
  }
  return;
}
```

### decoy: 0x00754df0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00754df0: 16-byte aligned: YES
  00754df0  55             PUSH EBP
  00754df1  8B EC          MOV EBP,ESP
  00754df3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00754df0
string literals (0):
direct callees (1):
  0x00754E20  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via EAX
```

## Card B11b-3-28

GOG function: CSWGuiMainInterface::SelectPrevPersonalAction  gog 0x00523d50  callconv=(blank)  param_size_bytes=4  gog_size=387  gog_retn=4
Candidate Steam address: 0x007525d0 (size 387, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00752450:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523bd0 CSWGuiMainInterface::SelectNextPersonalAction -> 0x00752450  gate_class=REVIEW
- GOG-order neighbour after:  0x00523ee0 CSWGuiMainInterface::UpdateStickyTarget -> 0x00752760  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x1c*,0x78*,0x7c*,0x1c7c*} | Steam {0x4*,0x8*,0x1c*,0x78*,0x7c*,0x1c7c*}
imm sets (* = in both, 6 common): GOG {0x4*,0x6*,0x7*,0xc*,0x2c*,0x3c*} | Steam {0x4*,0x6*,0x7*,0xc*,0x2c*,0x3c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007525d0: 16-byte aligned: YES
  007525d0  55             PUSH EBP
  007525d1  8B EC          MOV EBP,ESP
  007525d3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007525d0
string literals (0):
direct callees (3):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007525d0 FUN_007525d0 size=387 ret=4 callers=2 callees=3

void __thiscall FUN_007525d0(int param_1,int param_2)

{
  int local_14;
  int local_8;
  
  if (((-1 < param_2) && (param_2 < 6)) && (*(int *)(param_1 + 0x7c + param_2 * 0xc) != 0)) {
    FUN_0073fb90();
    FUN_007e5d80();
    local_8 = 0;
    if (*(int *)(param_1 + 0x1c7c + param_2 * 4) != -1) {
      for (local_14 = 0; local_14 < *(int *)(param_1 + 0x7c + param_2 * 0xc);
          local_14 = local_14 + 1) {
        if (*(int *)(*(int *)(param_1 + 0x78 + param_2 * 0xc) + 8 + local_14 * 0x3c) ==
            *(int *)(param_1 + 0x1c7c + param_2 * 4)) {
          if (local_14 < 1) {
            local_8 = (*(int *)(param_1 + 0x7c + param_2 * 0xc) + -1) * 0x3c +
                      *(int *)(param_1 + 0x78 + param_2 * 0xc);
          }
          else {
            local_8 = (local_14 + -1) * 0x3c + *(int *)(param_1 + 0x78 + param_2 * 0xc);
          }
          break;
        }
      }
    }
    if (1 < *(int *)(param_1 + 0x7c + param_2 * 0xc)) {
      if (local_8 == 0) {
        local_8 = (*(int *)(param_1 + 0x7c + param_2 * 0xc) + -1) * 0x3c +
                  *(int *)(param_1 + 0x78 + param_2 * 0xc);
      }
      *(undefined4 *)(param_1 + 0x1c7c + param_2 * 4) = *(undefined4 *)(local_8 + 8);
      FUN_004122a0(7);
    }
  }
  return;
}
```

### decoy: 0x00752450 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752450: 16-byte aligned: YES
  00752450  55             PUSH EBP
  00752451  8B EC          MOV EBP,ESP
  00752453  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752450
string literals (0):
direct callees (3):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
```

## Card B11b-3-29

GOG function: CSWGuiMainInterface::OnSoloButtonPressed  gog 0x00523720  callconv=(blank)  param_size_bytes=4  gog_size=59  gog_retn=4
Candidate Steam address: 0x00751fa0 (size 59, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: ShowSoloModeQuery 0x7cd510); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00751fe0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523670 CSWGuiMainInterface::OnPauseButtonPressed -> 0x00751ef0  gate_class=REVIEW
- GOG-order neighbour after:  0x00523760 CSWGuiMainInterface::OnStealthButtonPressed -> 0x00751fe0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x004ddc10 CGuiInGame::ShowSoloModeQuery -> Steam 0x007cd510  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x50*} | Steam {0x4*,0x50*}
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

### decoy: 0x00751fe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751fe0: 16-byte aligned: YES
  00751fe0  55             PUSH EBP
  00751fe1  8B EC          MOV EBP,ESP
  00751fe3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751fe0
string literals (0):
direct callees (5):
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0077FE50  CSWCCreature::IsStealthCapable
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077B020  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B11b-3-30

GOG function: CSWGuiMainInterface::OnPauseButtonPressed  gog 0x00523670  callconv=(blank)  param_size_bytes=4  gog_size=172  gog_retn=4
Candidate Steam address: 0x00751ef0 (size 172, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: rotation:0x00523760:0.89>0.62
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 5 (callees: GetPausedByCombat 0x740330, SetPausedByCombat 0x740350, GetClientRangedAnimation 0x740930); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00751e40:STR0/CALLEE2/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005235c0 CSWGuiMainInterface::OnClearOneButtonPressed -> 0x00751e40  gate_class=REVIEW
- GOG-order neighbour after:  0x00523720 CSWGuiMainInterface::OnSoloButtonPressed -> 0x00751fa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d7f0 CClientExoApp::GetPausedByCombat -> Steam 0x00740330  in candidate callees: yes
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam 0x00740350  in candidate callees: yes
  0x0040ddf0 CClientExoApp::SetAutoPaused -> Steam 0x00740930  in candidate callees: yes
  0x0040de20 CClientExoApp::GetAutoPaused -> Steam 0x00740960  in candidate callees: yes
  0x0040dee0 CClientExoApp::ShowTutorialWindow -> Steam 0x00740a20  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x50*} | Steam {0x4*,0x50*}
imm sets (* = in both, 4 common): GOG {0x4*,0x6*,0x20*,0x7f000000*} | Steam {0x4*,0x6*,0x20*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00751ef0: 16-byte aligned: YES
  00751ef0  55             PUSH EBP
  00751ef1  8B EC          MOV EBP,ESP
  00751ef3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00751ef0 FUN_00751ef0 size=172 ret=4 callers=0 callees=5

void __thiscall FUN_00751ef0(undefined4 param_1,int param_2)

{
  int iVar1;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    iVar1 = FUN_00740330(param_1);
    FUN_00740350(iVar1 == 0,4,0);
    iVar1 = FUN_00740960();
    if (iVar1 != 0) {
      FUN_00740930(0,0);
    }
    FUN_00740a20(6,0x7f000000,0x7f000000,0);
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

## Card B11b-3-31

GOG function: CSWGuiMainInterface::ResetPartyMemberItemUse  gog 0x005265a0  callconv=(blank)  param_size_bytes=0  gog_size=37  gog_retn=0
Candidate Steam address: 0x00754e20 (size 37, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00754df0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00526570 CSWGuiMainInterface::OnCancelPartyItemSelect -> 0x00754df0  gate_class=REVIEW
- GOG-order neighbour after:  0x005265d0 CSWGuiInGameMenu::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 10): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00754e20: 16-byte aligned: YES
  00754e20  55             PUSH EBP
  00754e21  8B EC          MOV EBP,ESP
  00754e23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00754e20
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x15230 [dec 86576]  x1  via ECX
  +0x15234 [dec 86580]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00754e20 FUN_00754e20 size=37 ret=0 callers=10 callees=0

void __fastcall FUN_00754e20(int param_1)

{
  *(undefined4 *)(param_1 + 0x15234) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x15230) = 0;
  return;
}
```

### decoy: 0x00754df0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00754df0: 16-byte aligned: YES
  00754df0  55             PUSH EBP
  00754df1  8B EC          MOV EBP,ESP
  00754df3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00754df0
string literals (0):
direct callees (1):
  0x00754E20  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via EAX
```

## Card B11b-3-32

GOG function: CSWGuiMainInterface::OnCharacterClicked  gog 0x005237d0  callconv=(blank)  param_size_bytes=4  gog_size=614  gog_retn=4
Candidate Steam address: 0x00752050 (size 614, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.60) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: ChangeCharacter 0x740380, CanLevelUp 0x6b9790); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00751fe0:STR0/CALLEE0/SIG0.188/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523760 CSWGuiMainInterface::OnStealthButtonPressed -> 0x00751fe0  gate_class=REVIEW
- GOG-order neighbour after:  0x00523a40 CSWGuiMainInterface::OnCharacterEntered -> 0x007522c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x0040d840 CClientExoApp::ChangeCharacter -> Steam 0x00740380  in candidate callees: yes
  0x00428700 CSWCCreature::PartyMenuUseItemSelect -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x00522d00 CSWGuiMainInterface::OnMenuButtonPressed -> Steam ?  in candidate callees: n/a
  0x005265a0 CSWGuiMainInterface::ResetPartyMemberItemUse -> Steam ?  in candidate callees: n/a
  0x006f5c10 CSWSCreatureStats::CanLevelUp -> Steam 0x006b9790  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x50*,0x5c*,0x9c*,0x1198*,0x11ac*,0xba9c*,0xbe3c*} | Steam {0x4*,0x50*,0x5c*,0x9c*,0x1198*,0x11ac*,0xba9c*,0xbe3c*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x60*,0xba4c*,0xbdec*,0x40000000*} | Steam {0x2*,0x3*,0x4*,0x60*,0xba4c*,0xbdec*,0x40000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00752050: 16-byte aligned: YES
  00752050  55             PUSH EBP
  00752051  8B EC          MOV EBP,ESP
  00752053  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00752050
string literals (0):
direct callees (8):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0077C780  CSWCCreature::PartyMenuUseItemSelect
  0x00754E20  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x006B9790  CSWSCreatureStats::CanLevelUp
  0x00751580  (unnamed)
  0x00740380  CClientExoApp::ChangeCharacter
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x5C [dec 92]  x3  via EAX,ECX
  +0x50 [dec 80]  x1  via EAX
  +0x9C [dec 156]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EAX
  +0x11AC [dec 4524]  x1  via ECX
  +0xBA9C [dec 47772]  x1  via ECX
  +0xBE3C [dec 48700]  x1  via EDX
  +0x15230 [dec 86576]  x1  via EDX
  +0x15234 [dec 86580]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00752050 FUN_00752050 size=614 ret=4 callers=0 callees=8

void __thiscall FUN_00752050(int param_1,int param_2)

{
  bool bVar1;
  short sVar2;
  int *piVar3;
  int iVar4;
  undefined4 uVar5;
  int local_14;
  int local_10;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    if (*(int *)(param_1 + 0x15230) == 0) {
      if (*(int *)(param_2 + 0x5c) == 1) {
        uVar5 = 0;
        FUN_0073fb90(0);
        iVar4 = FUN_007e5da0(uVar5);
        if ((iVar4 != 0) && (iVar4 = FUN_0077d800(), iVar4 != 0)) {
          iVar4 = FUN_006b9790();
          if (iVar4 == 0) {
            *(undefined4 *)(param_1 + 0xba9c) = 1;
            FUN_00751580(param_1 + 0xba4c);
          }
          else {
            *(undefined4 *)(param_1 + 0xbe3c) = 1;
            FUN_00751580(param_1 + 0xbdec);
          }
        }
      }
      else if (*(int *)(param_2 + 0x5c) + -1 < 3) {
        local_10 = *(int *)(param_2 + 0x5c) + -1;
        piVar3 = (int *)FUN_0073fb90();
        if (*piVar3 < 3) {
          local_10 = 1;
        }
        iVar4 = local_10;
        FUN_0073fb90(local_10);
        iVar4 = FUN_007e5da0(iVar4);
        if ((iVar4 != 0) && (iVar4 = FUN_0077d800(), iVar4 != 0)) {
          piVar3 = (int *)FUN_0077d800();
          if (piVar3[0x46b = byte +0x11ac] == 0) {
            bVar1 = false;
          }
          else {
            sVar2 = (**(code **)(*piVar3 + 0x9c))(0);
            bVar1 = sVar2 < 1;
          }
          if (!bVar1) {
            FUN_00740380(local_10,1);
          }
        }
      }
    }
    else {
      local_14 = *(int *)(param_2 + 0x5c) + -1;
      if ((local_14 == 2) && (piVar3 = (int *)FUN_0073fb90(), *piVar3 != 3)) {
        local_14 = 1;
      }
      *(int *)(param_1 + 0x15234) = local_14;
      uVar5 = 0;
      FUN_0073fb90(0);
      iVar4 = FUN_007e5da0(uVar5);
      if (iVar4 == 0) {
        FUN_00754e20();
      }
      else {
        FUN_0077c780(*(uint *)(param_1 + 0x15238) | 0x40000000,0);
      }
    }
  }
  return;
}
```

### decoy: 0x00751fe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00751fe0: 16-byte aligned: YES
  00751fe0  55             PUSH EBP
  00751fe1  8B EC          MOV EBP,ESP
  00751fe3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00751fe0
string literals (0):
direct callees (5):
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0077FE50  CSWCCreature::IsStealthCapable
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077B020  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B11b-3-33

GOG function: CSWGuiMainInterface::UpdateActionDescription  gog 0x0051e410  callconv=(blank)  param_size_bytes=0  gog_size=613  gog_retn=0
Candidate Steam address: 0x0074d8c0 (size 38, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0074d520:STR0/CALLEE0/SIG0.177/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0051daa0 CSWGuiMainInterface::Update -> 0x0074c320  gate_class=REVIEW
- GOG-order neighbour after:  0x0051e680 CSWGuiMainInterface::SetActionDescription -> 0x0074cf00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam 0x007408f0  in candidate callees: no
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x0051e680 CSWGuiMainInterface::SetActionDescription -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: no
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '%s (%d)'
disp sets (* = in both, 0 common): GOG {0x4,0x8,0x34,0x48,0x70,0x74,0x78,0x7c,0x218,0x3e8,0x5b8,0x1c7c,0x1c98,0x733c,...(+1)} | Steam {}
imm sets (* = in both, 0 common): GOG {0x5,0xc,0x10,0x3c,0x41,0x44,0x750} | Steam {0xffb8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0074d8c0: 16-byte aligned: YES
  0074d8c0  55             PUSH EBP
  0074d8c1  8B EC          MOV EBP,ESP
  0074d8c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0074d8c0
string literals (0):
direct callees (1):
  0x00410670  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0074d8c0 FUN_0074d8c0 size=38 ret=0 callers=1 callees=1

void __fastcall FUN_0074d8c0(int param_1)

{
  FUN_00410670(param_1 + 0xffb8);
  DAT_00a7ead6 = 0;
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

## Card B11b-3-34

GOG function: CSWGuiMainInterface::UpdateAndDrawFogOfWar  gog 0x00523b30  callconv=(blank)  param_size_bytes=4  gog_size=38  gog_retn=4
Candidate Steam address: 0x007523b0 (size 38, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00752330:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00523ae0 CSWGuiMainInterface::GetActionsFromActionIndex -> 0x00752360  gate_class=REVIEW
- GOG-order neighbour after:  0x00523b60 CSWGuiMainInterface::OnClearAllButtonPressed -> 0x007523e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00523b60 CSWGuiMainInterface::OnClearAllButtonPressed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x50*} | Steam {0x50*}
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
prologue for 0x007523b0: 16-byte aligned: YES
  007523b0  55             PUSH EBP
  007523b1  8B EC          MOV EBP,ESP
  007523b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007523b0
string literals (0):
direct callees (1):
  0x007523E0  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007523b0 FUN_007523b0 size=38 ret=4 callers=0 callees=1

void __thiscall FUN_007523b0(undefined4 param_1,int param_2)

{
  if (*(int *)(param_2 + 0x50) != 0) {
    FUN_007523e0(param_1);
  }
  return;
}
```

### decoy: 0x00752330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00752330: 16-byte aligned: YES
  00752330  55             PUSH EBP
  00752331  8B EC          MOV EBP,ESP
  00752333  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00752330
string literals (0):
direct callees (1):
  0x00752360  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via EAX
```
