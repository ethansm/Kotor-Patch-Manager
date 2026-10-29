# Review cards B23 chunk 1

Answer each card with one TSV row in `steam_db_batches/B23/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B23-1-01

GOG function: CAuroraLayeredTexture::GetTextureDataPtr  gog 0x0090a510  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093feb0 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093fed0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090a4b0 CAuroraProcessedTexture::GetProcessedTextureAttrib  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090a550 CAuroraProcessedTexture::GetProcessedTextureDataSize -> 0x0093feb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x0090c1c0 CResTPC::GetTPCDataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x0090a8f0 CAuroraProcessedTexture::GetPtrToResHelperInfo -> Steam 0x009402d0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8} | Steam {}
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
prologue for 0x0093feb0: 16-byte aligned: YES
  0093feb0  55             PUSH EBP
  0093feb1  8B EC          MOV EBP,ESP
  0093feb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093feb0
string literals (0):
direct callees (1):
  0x009404A0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0093feb0 FUN_0093feb0 size=28 ret=0 callers=3 callees=1

void __fastcall FUN_0093feb0(undefined4 *param_1)

{
  *param_1 = CAuroraProcessedTexture::vftable;
  FUN_009404a0();
  return;
}
```

### decoy: 0x0093fed0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093fed0: 16-byte aligned: YES
  0093fed0  55             PUSH EBP
  0093fed1  8B EC          MOV EBP,ESP
  0093fed3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093fed0
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00945DB0  CResTPC::GetTPCAttrib
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX
```

### Static facts (script output, report-only)
- size 28 B, RET 0
- callers 3: 0x0093c300, 0x0093fe80, 0x00982cc0
- callees: 0x009404a0 (CALL)
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x0093feb0)

## Card B23-1-02

GOG function: CAuroraLayeredTexture::GetTextureDataPtr  gog 0x0090a510  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093ff70 (size 51, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093ff30:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090a4b0 CAuroraProcessedTexture::GetProcessedTextureAttrib  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090a550 CAuroraProcessedTexture::GetProcessedTextureDataSize -> 0x0093feb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x0090c1c0 CResTPC::GetTPCDataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x0090a8f0 CAuroraProcessedTexture::GetPtrToResHelperInfo -> Steam 0x009402d0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x0093ff70: 16-byte aligned: YES
  0093ff70  55             PUSH EBP
  0093ff71  8B EC          MOV EBP,ESP
  0093ff73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093ff70
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00945E70  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093ff70 FUN_0093ff70 size=51 ret=0 callers=1 callees=2

undefined4 __fastcall FUN_0093ff70(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((*(int *)(param_1 + 8) != 0) && (iVar1 = FUN_00711250(), 0 < iVar1)) {
    uVar2 = FUN_00945e70();
    return uVar2;
  }
  return 0;
}
```

### decoy: 0x0093ff30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093ff30: 16-byte aligned: YES
  0093ff30  55             PUSH EBP
  0093ff31  8B EC          MOV EBP,ESP
  0093ff33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093ff30
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00455820  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 51 B, RET 0
- callers 1: 0x0093bc00 ResourceProvider(name, ext, &size) (DAT_00A3082C)
- callees: 0x00711250 (CALL), 0x00945e70 (CALL)
- twins: 0x0093ff30 (distinguishing: 0x0093ff9a: CALL 0x00945e70)

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0093ff70: static only
- xref | no live-only facts | probable | rea_facts 0x0093ff70: checked

## Card B23-1-03

GOG function: CSWGuiPowersFeatsSkillsDebugMenu::CreateOption  gog 0x005b3a80  callconv=(blank)  param_size_bytes=12  gog_size=235  gog_retn=12
Candidate Steam address: 0x008bfe30 (size 235, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.83) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008bff20:STR0/CALLEE0/SIG0.171/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005b38e0 CSWGuiCreateItemSubMenu::HandleInputEvent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005b3b70 CSWGuiCreateItemSubMenu::LoadItems  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00512a40 CSWGuiButton::Constructor -> Steam ?  in candidate callees: n/a
  0x00915200 CSWGuiTextParams::SetText -> Steam 0x00416e30  in candidate callees: yes
  0x00916d90 CSWGuiControl::AddEvent -> Steam 0x00418af0  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x98*} | Steam {0x4*,0x8*,0xc*,0x98*}
imm sets (* = in both, 7 common): GOG {0x4*,0xc*,0x24*,0x27*,0x2d*,0x178*,0x1d0*} | Steam {0x4*,0xc*,0x24*,0x27*,0x2d*,0x178*,0x1d0*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x008bfe30: 16-byte aligned: YES
  008bfe30  55             PUSH EBP
  008bfe31  8B EC          MOV EBP,ESP
  008bfe33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008bfe30
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x0041A510  ButtonCtor
  0x00416E30  CSWGuiTextParams::SetText
  0x00418AF0  CSWGuiControl::AddEvent
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x98 [dec 152]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008bfe30 FUN_008bfe30 size=235 ret=12 callers=6 callees=4

int * __thiscall FUN_008bfe30(undefined4 param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  int iVar1;
  int *local_34;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097a7ab;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_24 = *(undefined4 *)(param_4 + 4);
  local_20 = *(undefined4 *)(param_4 + 8);
  local_1c = *(undefined4 *)(param_4 + 0xc);
  local_18 = *(undefined4 *)(param_4 + 0x10);
  iVar1 = FUN_00919723(0x1d0,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_34 = (int *)0x0;
  }
  else {
    local_34 = (int *)FUN_0041a510();
  }
  local_8 = 0xffffffff;
  local_14 = local_34;
  (**(code **)(*local_34 + 0x98))(&local_24,param_4);
  FUN_00416e30(param_2);
  FUN_00418af0(0x27,param_1,param_3);
  FUN_00418af0(0x2d,param_1,param_3);
  ExceptionList = local_10;
  return local_14;
}
```

### decoy: 0x008bff20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008bff20: 16-byte aligned: YES
  008bff20  55             PUSH EBP
  008bff21  8B EC          MOV EBP,ESP
  008bff23  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008bff20
string literals (0):
direct callees (4):
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CB540  CGuiInGame::HidePowersFeatsSkillsDebugMenu
  0x00623320  CSWSEncounter::AIUpdate
  0x008C0010  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x4A8 [dec 1192]  x1  via EAX
```

### Static facts (script output, report-only)
- size 235 B, RET 12
- callers 6: 0x008bf280, 0x008c0010, 0x008c01f0, 0x008c0460, 0x008c0680, ...
- callees: 0x00919723 operator new (CALL), 0x0041a510 (CALL), 0x00416e30 CSWGuiTextParams::SetText (CALL), 0x00418af0 CSWGuiControl::AddEvent (CALL)
- indirect: 0x008bfecf slot 0x98
- twins: none

### Live evidence (REA, report-only)
- xref | no live-only facts | probable | rea_facts 0x008bfe30: checked

## Card B23-1-04

GOG function: CSWGuiContainer::OnPanelRemoved  gog 0x005a72d0  callconv=(blank)  param_size_bytes=0  gog_size=30  gog_retn=0
Candidate Steam address: 0x008b3730 (size 30, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: OnPanelRemoved 0x40f4c0)
- best decoy (neighbour/other candidate) scores: 0x008b3630:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a71d0 CSWGuiContainer::OnPanelAdded -> 0x008b3630  trusted
- GOG-order neighbour after:  0x005a72f0 CSWGuiStoreItemEntry::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: yes
  0x00912600 CSWGuiManager::RestoreOldMousePosition -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x008b3730: 16-byte aligned: YES
  008b3730  55             PUSH EBP
  008b3731  8B EC          MOV EBP,ESP
  008b3733  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008b3730
string literals (0):
direct callees (2):
  0x00414180  (unnamed)
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008b3730 FUN_008b3730 size=30 ret=0 callers=0 callees=2

void __fastcall FUN_008b3730(undefined4 param_1)

{
  FUN_00414180(param_1);
  FUN_0040f4c0();
  return;
}
```

### decoy: 0x008b3630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b3630: 16-byte aligned: YES
  008b3630  55             PUSH EBP
  008b3631  8B EC          MOV EBP,ESP
  008b3633  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b3630
string literals (0):
direct callees (4):
  0x0073EEE0  (unnamed)
  0x00414150  (unnamed)
  0x00414230  (unnamed)
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX
  +0x8 [dec 8]  x2  via ECX
  +0xC [dec 12]  x2  via ECX
```

### Static facts (script output, report-only)
- size 30 B, RET 0
- callees: 0x00414180 (CALL), 0x0040f4c0 CSWGuiPanel::OnPanelRemoved (CALL)
- globals: 0x00a1b49c READ
- twins: none

### Live evidence (REA, report-only)
- xref | data refs 1: 0x9a7830 (first 8) | confirmed | xrefs 0x008b3730: 1 sources, 0 code call sites
- xref | no live run (static only) | probable | rea_facts 0x008b3730: static only

## Card B23-1-05

GOG function: CExoInput::UpdateMouseDelta  gog 0x00616990  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x0072fd10 (size 81, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.42 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0072fd70:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00616970 CExoInput::GetMouseDelta  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006169b0 CExoInput::GetJoystickNStick  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00615ec0 CExoInputInternal::UpdateMouseDelta -> Steam 0x0072f110  in candidate callees: no
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x188,0x18c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x10}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0072fd10: 16-byte aligned: YES
  0072fd10  55             PUSH EBP
  0072fd11  8B EC          MOV EBP,ESP
  0072fd13  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072fd10
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x188 [dec 392]  x1  via EDX
  +0x18C [dec 396]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0072fd10 FUN_0072fd10 size=81 ret=4 callers=1 callees=0

undefined4 __thiscall FUN_0072fd10(int param_1,int param_2)

{
  undefined4 local_14;
  
  if ((param_2 < *(int *)(*(int *)(param_1 + 4) + 0x18c)) &&
     (*(int *)(*(int *)(*(int *)(param_1 + 4) + 0x188) + param_2 * 4) != 0)) {
    local_14 = 1;
  }
  else {
    local_14 = 0;
  }
  return local_14;
}
```

### decoy: 0x0072fd70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072fd70: 16-byte aligned: YES
  0072fd70  55             PUSH EBP
  0072fd71  8B EC          MOV EBP,ESP
  0072fd73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0072fd70
string literals (0):
direct callees (1):
  0x0072ED40  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### Static facts (script output, report-only)
- size 81 B, RET 4
- callers 1: 0x00785df0 CClientExoAppInternal::SetEventDescriptions
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x0072fd10)

## Card B23-1-06

GOG function: CExoArrayList_CExoInputEvent_::Add  gog 0x00462840  callconv=(blank)  param_size_bytes=16  gog_size=124  gog_retn=16
Candidate Steam address: 0x0072f540 (size 124, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: ProcessInput 0x7ac470); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0072f500:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00460d20 CClientExoAppInternal::GetDefaultActions -> 0x007b40f0  trusted
- GOG-order neighbour after:  0x004628c0 CExoArrayList_CExoInputEvent_::Insert  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004629e0 CExoArrayList_CExoInputEvent_::Allocate -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x10*} | Steam {0x4*,0x8*,0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0072f540: 16-byte aligned: YES
  0072f540  55             PUSH EBP
  0072f541  8B EC          MOV EBP,ESP
  0072f543  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072f540
string literals (0):
direct callees (1):
  0x0072F640  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x5  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0xC [dec 12]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0072f540 FUN_0072f540 size=124 ret=16 callers=4 callees=1

void __thiscall
FUN_0072f540(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

{
  undefined4 *puVar1;
  int local_c;
  
  if (param_1[1 = byte +0x4] == param_1[2 = byte +0x8]) {
    if (param_1[2 = byte +0x8] == 0) {
      local_c = 0x10;
    }
    else {
      local_c = param_1[2 = byte +0x8] << 1;
    }
    FUN_0072f640(local_c);
  }
  puVar1 = (undefined4 *)(*param_1 + param_1[1 = byte +0x4] * 0x10);
  *puVar1 = param_2;
  puVar1[1 = byte +0x4] = param_3;
  puVar1[2 = byte +0x8] = param_4;
  puVar1[3 = byte +0xc] = param_5;
  param_1[1 = byte +0x4] = param_1[1 = byte +0x4] + 1;
  return;
}
```

### decoy: 0x0072f500 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072f500: 16-byte aligned: YES
  0072f500  55             PUSH EBP
  0072f501  8B EC          MOV EBP,ESP
  0072f503  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0072f500
string literals (0):
direct callees (1):
  0x0072F640  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

### Static facts (script output, report-only)
- size 124 B, RET 16
- callers 4: 0x0072c7e0 CExoInputInternal::GetEvents, 0x0072f480, 0x007ac470 CClientExoAppInternal::ProcessInput, 0x007b5c10
- callees: 0x0072f640 (CALL)
- twins: none

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0072f540: static only
- xref | no live-only facts | probable | rea_facts 0x0072f540: checked

## Card B23-1-07

GOG function: CAuroraTexture::GetTextureDataPtr  gog 0x00909e50  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093f7c0 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093f7e0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00909e00 CAuroraTexture::GetTextureAttrib  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00909e90 CAuroraTexture::Load  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x00943a40 CResTGA::GetTGADataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8} | Steam {}
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
prologue for 0x0093f7c0: 16-byte aligned: YES
  0093f7c0  55             PUSH EBP
  0093f7c1  8B EC          MOV EBP,ESP
  0093f7c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093f7c0
string literals (0):
direct callees (1):
  0x0093FB70  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0093f7c0 FUN_0093f7c0 size=28 ret=0 callers=3 callees=1

void __fastcall FUN_0093f7c0(undefined4 *param_1)

{
  *param_1 = CAuroraTexture::vftable;
  FUN_0093fb70();
  return;
}
```

### decoy: 0x0093f7e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093f7e0: 16-byte aligned: YES
  0093f7e0  55             PUSH EBP
  0093f7e1  8B EC          MOV EBP,ESP
  0093f7e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093f7e0
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x009453A0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 28 B, RET 0
- callers 3: 0x0093c250, 0x0093f790, 0x00982c90
- callees: 0x0093fb70 (CALL)
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x0093f7c0)

## Card B23-1-08

GOG function: CAuroraProcessedTexture::GetProcessedTextureDataSize  gog 0x0090a550  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093fb70 (size 164, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093fb20:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090a510 CAuroraLayeredTexture::GetTextureDataPtr -> 0x0093ff30  gate_class=REVIEW
- GOG-order neighbour after:  0x0090a590 CAuroraTexture::GetTextureDataSize -> 0x0093f870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x009444c0 CResTPC::GetTPCSize -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x4,0x8*}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc,0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0093fb70: 16-byte aligned: YES
  0093fb70  55             PUSH EBP
  0093fb71  8B EC          MOV EBP,ESP
  0093fb73  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093fb70
string literals (0):
direct callees (3):
  0x00710AD0  (unnamed)
  0x007111C0  CRes::CancelRequest
  0x00713010  CExoResMan::ReleaseResObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093fb70 FUN_0093fb70 size=164 ret=0 callers=3 callees=3

void __fastcall FUN_0093fb70(undefined4 *param_1)

{
  int iVar1;
  
  *param_1 = CResHelper<CResTGA,3>::vftable;
  if ((param_1[2 = byte +0x8] != 0) && (iVar1 = FUN_00710ad0(&DAT_00986925), iVar1 != 0)) {
    if (param_1[1 = byte +0x4] != 0) {
      FUN_007111c0();
      param_1[1 = byte +0x4] = 0;
    }
    iVar1 = FUN_00713010(param_1[2 = byte +0x8]);
    if (iVar1 == 0) {
      if ((undefined4 *)param_1[2 = byte +0x8] != (undefined4 *)0x0) {
        (*(code *)**(undefined4 **)param_1[2 = byte +0x8])(1);
      }
      param_1[2 = byte +0x8] = 0;
    }
  }
  return;
}
```

### decoy: 0x0093fb20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093fb20: 16-byte aligned: YES
  0093fb20  55             PUSH EBP
  0093fb21  8B EC          MOV EBP,ESP
  0093fb23  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093fb20
string literals (0):
direct callees (2):
  0x007107F0  (unnamed)
  0x00710EB0  CResRef::operator=_4
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

### Static facts (script output, report-only)
- size 164 B, RET 0
- callers 3: 0x0093f7c0, 0x0093fde0, 0x00982ec0
- callees: 0x00710ad0 (CALL), 0x007111c0 CRes::CancelRequest (CALL), 0x00713010 CExoResMan::ReleaseResObject (CALL)
- indirect: 0x0093fbf8 slot 0x0
- globals: 0x00a1b490 READ
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x0093fb70)

## Card B23-1-09

GOG function: CSWGuiExtent::Intersection  gog 0x0090c8f0  callconv=(blank)  param_size_bytes=12  gog_size=285  gog_retn=0
Candidate Steam address: 0x0040df30 (size 64, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040def0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090c850 CSWGuiExtent::Load  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090ca10 CSWGuiExtent::Union -> 0x0040e480  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 10): the card's A-CALLEE credit may come from these, not from callees
  0x005c04e0 CSWUpgradeItemEntry::SetExtent -> Steam 0x008d0110  in candidate callers: no
  0x00918170 CSWGuiButton::SetExtent -> Steam 0x00419ed0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc} | Steam {}
imm sets (* = in both, 0 common): GOG {0x28} | Steam {0x3,0x10}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040df30: 16-byte aligned: YES
  0040df30  55             PUSH EBP
  0040df31  8B EC          MOV EBP,ESP
  0040df33  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040df30
string literals (0):
direct callees (1):
  0x0040C170  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0040df30 FUN_0040df30 size=64 ret=0 callers=85 callees=1

void __fastcall FUN_0040df30(int *param_1)

{
  if (*param_1 != 0) {
    FID_conflict__vector_deleting_destructor_ [GHIDRA-AUTO-LABEL: unreliable](3);
  }
  *param_1 = 0;
  return;
}
```

### decoy: 0x0040def0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040def0: 16-byte aligned: YES
  0040def0  55             PUSH EBP
  0040def1  8B EC          MOV EBP,ESP
  0040def3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040def0
string literals (0):
direct callees (1):
  0x0040E170  CExoArrayList<CExoString>::SetAllocated
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

### Static facts (script output, report-only)
- size 64 B, RET 0
- callers 85: 0x0040c9f0 CExoMoviePlayerInternal::Destructor, 0x005219d0 CSWSArea::Destructor_2, 0x005308b0 CServerExoAppInternal::LoadCharacterStart, 0x005334e0 CServerExoAppInternal::SetEstimatedSaveSize, 0x0053af20 CServerExoAppInternal::DoPCAutosave, ...
- callees: 0x0040c170 (CALL)
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x0040df30)

## Card B23-1-10

GOG function: CAuroraTexture::GetTextureDataPtr  gog 0x00909e50  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093f870 (size 51, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093f830:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00909e00 CAuroraTexture::GetTextureAttrib  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00909e90 CAuroraTexture::Load  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x00943a40 CResTGA::GetTGADataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x0093f870: 16-byte aligned: YES
  0093f870  55             PUSH EBP
  0093f871  8B EC          MOV EBP,ESP
  0093f873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093f870
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00945EA0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093f870 FUN_0093f870 size=51 ret=0 callers=2 callees=2

undefined4 __fastcall FUN_0093f870(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((*(int *)(param_1 + 8) != 0) && (iVar1 = FUN_00711250(), 0 < iVar1)) {
    uVar2 = FUN_00945ea0();
    return uVar2;
  }
  return 0;
}
```

### decoy: 0x0093f830 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093f830: 16-byte aligned: YES
  0093f830  55             PUSH EBP
  0093f831  8B EC          MOV EBP,ESP
  0093f833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093f830
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x009453F0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 51 B, RET 0
- callers 2: 0x0093bc00 ResourceProvider(name, ext, &size) (DAT_00A3082C), 0x009402d0 CAuroraProcessedTexture::GetPtrToResHelperInfo
- callees: 0x00711250 (CALL), 0x00945ea0 (CALL)
- twins: 0x0093f830 (distinguishing: 0x0093f89a: CALL 0x00945ea0)

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0093f870: static only
- xref | no live-only facts | probable | rea_facts 0x0093f870: checked

## Card B23-1-11

GOG function: CSWGuiExtent::Intersection  gog 0x0090c8f0  callconv=(blank)  param_size_bytes=12  gog_size=285  gog_retn=0
Candidate Steam address: 0x0040e480 (size 285, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040e360:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090c850 CSWGuiExtent::Load  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090ca10 CSWGuiExtent::Union -> 0x0040e480  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 10): the card's A-CALLEE credit may come from these, not from callees
  0x005c04e0 CSWUpgradeItemEntry::SetExtent -> Steam 0x008d0110  in candidate callers: no
  0x00918170 CSWGuiButton::SetExtent -> Steam 0x00419ed0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 1 common): GOG {0x28*} | Steam {0x28*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040e480: 16-byte aligned: YES
  0040e480  55             PUSH EBP
  0040e481  8B EC          MOV EBP,ESP
  0040e483  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040e480
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040e480 FUN_0040e480 size=285 ret=0 callers=1 callees=0

int * FUN_0040e480(int *param_1,int *param_2,int *param_3)

{
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  
  if (*param_2 < *param_3) {
    local_20 = *param_2;
  }
  else {
    local_20 = *param_3;
  }
  if (param_2[1 = byte +0x4] < param_3[1 = byte +0x4]) {
    local_24 = param_2[1 = byte +0x4];
  }
  else {
    local_24 = param_3[1 = byte +0x4];
  }
  if (*param_3 + param_3[2 = byte +0x8] < *param_2 + param_2[2 = byte +0x8]) {
    local_28 = *param_2 + param_2[2 = byte +0x8];
  }
  else {
    local_28 = *param_3 + param_3[2 = byte +0x8];
  }
  if (param_3[1 = byte +0x4] + param_3[3 = byte +0xc] < param_2[1 = byte +0x4] + param_2[3 = byte +0xc]) {
    local_2c = param_2[1 = byte +0x4] + param_2[3 = byte +0xc];
  }
  else {
    local_2c = param_3[1 = byte +0x4] + param_3[3 = byte +0xc];
  }
  *param_1 = local_20;
  param_1[1 = byte +0x4] = local_24;
  param_1[2 = byte +0x8] = local_28 - local_20;
  param_1[3 = byte +0xc] = local_2c - local_24;
  return param_1;
}
```

### decoy: 0x0040e360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040e360: 16-byte aligned: YES
  0040e360  55             PUSH EBP
  0040e361  8B EC          MOV EBP,ESP
  0040e363  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040e360
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

### Static facts (script output, report-only)
- size 285 B, RET 0
- callers 1: 0x008690a0
- twins: none

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0040e480: static only
- xref | no live-only facts | probable | rea_facts 0x0040e480: checked

## Card B23-1-12

GOG function: CSWGuiSaveNamePanel::OnPanelRemoved  gog 0x005d7fe0  callconv=(blank)  param_size_bytes=0  gog_size=30  gog_retn=0
Candidate Steam address: 0x00858b60 (size 30, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: OnPanelRemoved 0x40f4c0)
- best decoy (neighbour/other candidate) scores: 0x00858a70:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d7ef0 CSWGuiSaveNamePanel::OnPanelAdded  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d8000 CSWGuiSaveNamePanel::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: yes
  0x00912600 CSWGuiManager::RestoreOldMousePosition -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00858b60: 16-byte aligned: YES
  00858b60  55             PUSH EBP
  00858b61  8B EC          MOV EBP,ESP
  00858b63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00858b60
string literals (0):
direct callees (2):
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
  0x00414180  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00858b60 FUN_00858b60 size=30 ret=0 callers=0 callees=2

void __fastcall FUN_00858b60(undefined4 param_1)

{
  FUN_0040f4c0(param_1);
  FUN_00414180();
  return;
}
```

### decoy: 0x00858a70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00858a70: 16-byte aligned: YES
  00858a70  55             PUSH EBP
  00858a71  8B EC          MOV EBP,ESP
  00858a73  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00858a70
string literals (0):
direct callees (5):
  0x004215E0  (unnamed)
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
  0x0073EEE0  (unnamed)
  0x00414150  (unnamed)
  0x00414230  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via ECX
```

### Static facts (script output, report-only)
- size 30 B, RET 0
- callees: 0x0040f4c0 CSWGuiPanel::OnPanelRemoved (CALL), 0x00414180 (CALL)
- globals: 0x00a1b49c READ
- twins: none

### Live evidence (REA, report-only)
- xref | data refs 1: 0x9a4310 (first 8) | confirmed | xrefs 0x00858b60: 1 sources, 0 code call sites
- xref | no live run (static only) | probable | rea_facts 0x00858b60: static only

## Card B23-1-13

GOG function: CClientAIMaster::RemoveObject  gog 0x0044eee0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2540 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: UnloadArea 0x7a5fa0, Destructor_2 0x7ed5f0)
- best decoy (neighbour/other candidate) scores: 0x007a2510:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044eeb0 CClientAIMaster::AddObject -> 0x007a2510  gate_class=REVIEW
- GOG-order neighbour after:  0x0044ef10 Global::PlayRoomSoundEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044eb90 CClientAIList::RemoveObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callers: yes
  0x004727b0 CSWCObject::Destructor_3 -> Steam 0x007ed5f0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007a2540: 16-byte aligned: YES
  007a2540  55             PUSH EBP
  007a2541  8B EC          MOV EBP,ESP
  007a2543  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2540
string literals (0):
direct callees (1):
  0x007A21D0  CClientExoAppInternal::SetGameOver
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a2540 FUN_007a2540 size=36 ret=4 callers=3 callees=1
THUNK -> 0x007A21D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

undefined4 FUN_007a2540(undefined4 param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_007a21d0(param_1);
  return uVar1;
}
```

### decoy: 0x007a2510 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2510: 16-byte aligned: YES
  007a2510  55             PUSH EBP
  007a2511  8B EC          MOV EBP,ESP
  007a2513  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a2510
string literals (0):
direct callees (1):
  0x007A2110  CClientExoAppInternal::AddMoveToModuleMovie_2
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 36 B, RET 4
- callers 3: 0x007a5fa0 CSWCArea::UnloadArea, 0x007ed5f0, 0x007fd370
- callees: 0x007a21d0 (CALL)
- twins: 0x007a2510 (distinguishing: 0x007a2553: CALL 0x007a21d0)

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x007a2540: static only
- xref | no live-only facts | probable | rea_facts 0x007a2540: checked

## Card B23-1-14

GOG function: CAuroraProcessedTexture::GetProcessedTextureDataSize  gog 0x0090a550  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093ff30 (size 51, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093fed0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090a510 CAuroraLayeredTexture::GetTextureDataPtr -> 0x0093ff30  gate_class=REVIEW
- GOG-order neighbour after:  0x0090a590 CAuroraTexture::GetTextureDataSize -> 0x0093f870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x009444c0 CResTPC::GetTPCSize -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x0093ff30: 16-byte aligned: YES
  0093ff30  55             PUSH EBP
  0093ff31  8B EC          MOV EBP,ESP
  0093ff33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093ff30
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00455820  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093ff30 FUN_0093ff30 size=51 ret=0 callers=5 callees=2

undefined4 __fastcall FUN_0093ff30(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((*(int *)(param_1 + 8) != 0) && (iVar1 = FUN_00711250(), 0 < iVar1)) {
    uVar2 = FUN_00455820();
    return uVar2;
  }
  return 0;
}
```

### decoy: 0x0093fed0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093fed0: 16-byte aligned: YES
  0093fed0  55             PUSH EBP
  0093fed1  8B EC          MOV EBP,ESP
  0093fed3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093fed0
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00945DB0  CResTPC::GetTPCAttrib
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX
```

### Static facts (script output, report-only)
- size 51 B, RET 0
- callers 5: 0x0093b860, 0x0093b9c0, 0x0093bc00 ResourceProvider(name, ext, &size) (DAT_00A3082C), 0x0093f3d0, 0x009402d0 CAuroraProcessedTexture::GetPtrToResHelperInfo
- callees: 0x00711250 (CALL), 0x00455820 (CALL)
- twins: 0x0093ff70 (distinguishing: 0x0093ff5a: CALL 0x00455820)

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0093ff30: static only
- xref | no live-only facts | probable | rea_facts 0x0093ff30: checked

## Card B23-1-15

GOG function: CExoInput::UpdateMouseDelta  gog 0x00616990  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x0072fe30 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0072fe10:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00616970 CExoInput::GetMouseDelta  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006169b0 CExoInput::GetJoystickNStick  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00615ec0 CExoInputInternal::UpdateMouseDelta -> Steam 0x0072f110  in candidate callees: no
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x410}
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
prologue for 0x0072fe30: 16-byte aligned: YES
  0072fe30  55             PUSH EBP
  0072fe31  8B EC          MOV EBP,ESP
  0072fe33  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072fe30
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x410 [dec 1040]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0072fe30 FUN_0072fe30 size=36 ret=4 callers=5 callees=0

void __thiscall FUN_0072fe30(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x410) = param_2;
  return;
}
```

### decoy: 0x0072fe10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072fe10: 16-byte aligned: YES
  0072fe10  55             PUSH EBP
  0072fe11  8B EC          MOV EBP,ESP
  0072fe13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0072fe10
string literals (0):
direct callees (1):
  0x0072F0F0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### Static facts (script output, report-only)
- size 36 B, RET 4
- callers 5: 0x008fff20 CSWGuiInGameOptKeyMappings::Constructor, 0x009007a0 CSWGuiInGameOptKeyMappings::HandleInputEvent, 0x00901310, 0x009014b0, 0x00901590 CSWGuiInGameOptKeyMappings::Update
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x0072fe30)

## Card B23-1-16

GOG function: CClientAIMaster::AddObject  gog 0x0044eeb0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2510 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 7 (callers: HandleServerToPlayerCreatureUpdate_Add 0x804ad0, HandleServerToPlayerCreatureUpdate_VisEffect 0x806960, HandleServerToPlayerPlaceableUpdate_Add 0x80d050)
- best decoy (neighbour/other candidate) scores: 0x007a2540:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ed40 CClientAIMaster::UpdateState  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044eee0 CClientAIMaster::RemoveObject -> 0x007a2540  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ead0 CClientAIList::AddObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 13): the card's A-CALLEE credit may come from these, not from callees
  0x004990c0 CSWCMessage::HandleServerToPlayerCreatureUpdate_Add -> Steam 0x00804ad0  in candidate callers: yes
  0x0049af50 CSWCMessage::HandleServerToPlayerCreatureUpdate_VisEffect -> Steam 0x00806960  in candidate callers: yes
  0x004a1640 CSWCMessage::HandleServerToPlayerPlaceableUpdate_Add -> Steam 0x0080d050  in candidate callers: yes
  0x004bdea0 CSWCMessage::HandleServerToPlayerSafeProjectileSpell -> Steam 0x00829850  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007a2510: 16-byte aligned: YES
  007a2510  55             PUSH EBP
  007a2511  8B EC          MOV EBP,ESP
  007a2513  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2510
string literals (0):
direct callees (1):
  0x007A2110  CClientExoAppInternal::AddMoveToModuleMovie_2
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a2510 FUN_007a2510 size=36 ret=4 callers=13 callees=1
THUNK -> 0x007A2110 (Ghidra may omit __thiscall on thunks; ECX is passed through)

undefined4 FUN_007a2510(undefined4 param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_007a2110(param_1);
  return uVar1;
}
```

### decoy: 0x007a2540 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2540: 16-byte aligned: YES
  007a2540  55             PUSH EBP
  007a2541  8B EC          MOV EBP,ESP
  007a2543  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a2540
string literals (0):
direct callees (1):
  0x007A21D0  CClientExoAppInternal::SetGameOver
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 36 B, RET 4
- callers 13: 0x007fddf0, 0x00800cb0 CSWCSoundObject::AddToArea, 0x00804ad0 CSWCMessage::HandleServerToPlayerCreatureUpdate_Add, 0x00806960 CSWCMessage::HandleServerToPlayerCreatureUpdate_VisEffect, 0x0080c6b0, ...
- callees: 0x007a2110 (CALL)
- twins: 0x007a2540 (distinguishing: 0x007a2523: CALL 0x007a2110)

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x007a2510: static only
- xref | no live-only facts | probable | rea_facts 0x007a2510: checked
