# Review cards B17 chunk 2

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-2-01

GOG function: CSWCModule::ZoomCamera  gog 0x0046b6d0  callconv=(blank)  param_size_bytes=4  gog_size=159  gog_retn=4
Candidate Steam address: 0x007f44b0 (size 159, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007f4550:STR0/CALLEE1/SIG0.183/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0046b440 CSWCModule::AcclTurnCamera -> 0x007f4220  gate_class=REVIEW
- GOG-order neighbour after:  0x0046b770 CSWCModule::ScrollCamera  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00438930 CClientExoAppInternal::UpdateCamera -> Steam 0x0078c350  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x1c*,0x34*,0x40*,0x80*,0x9c*,0x104*} | Steam {0x1c*,0x34*,0x40*,0x80*,0x9c*,0x104*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x44*} | Steam {0x4*,0xc*,0x44*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007f44b0: 16-byte aligned: YES
  007f44b0  55             PUSH EBP
  007f44b1  8B EC          MOV EBP,ESP
  007f44b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f44b0
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x34 [dec 52]  x3  via ECX,EDX
  +0x40 [dec 64]  x3  via EAX,ECX
  +0x80 [dec 128]  x3  via EAX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x9C [dec 156]  x1  via EAX
  +0x104 [dec 260]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007f44b0 FUN_007f44b0 size=159 ret=4 callers=1 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007f44b0(int param_1,float param_2)

{
  int *piVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 0x40) != 0) {
    piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0xffffffff);
    if (piVar1 != (int *)0x0) {
      iVar2 = (**(code **)(*piVar1 + 0x1c))();
      if (iVar2 != 0) {
        *(float *)(iVar2 + 0x34) = *(float *)(iVar2 + 0x34) + param_2;
        *(undefined4 *)(param_1 + 0x9c) = *(undefined4 *)(iVar2 + 0x34);
      }
    }
  }
  if (param_2 != (float)_DAT_00986b38) {
    *(undefined4 *)(param_1 + 0x104) = 1;
    *(uint *)(param_1 + 0x80) = *(uint *)(param_1 + 0x80) | 1;
  }
  return;
}
```

### decoy: 0x007f4550 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f4550: 16-byte aligned: YES
  007f4550  55             PUSH EBP
  007f4551  8B EC          MOV EBP,ESP
  007f4553  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f4550
string literals (0):
direct callees (1):
  0x0044B8A0  (unnamed)
struct offsets touched (top 6):
  +0x40 [dec 64]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x18 [dec 24]  x1  via EDX
  +0x20 [dec 32]  x1  via EDX
  +0x80 [dec 128]  x1  via ECX
```

### Static facts (script output, report-only)
- size 159 B, RET 4
- callers 1: 0x0078c350 CClientExoAppInternal::UpdateCamera
- indirect: 0x007f44d8 slot 0x80, 0x007f44ee slot 0x1c
- twins: none

## Card B17-2-02

GOG function: CSWGuiPartySelection::OnToggled  gog 0x0058c660  callconv=(blank)  param_size_bytes=4  gog_size=567  gog_retn=4
Candidate Steam address: 0x0089da90 (size 46, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0089cf30:STR0/CALLEE0/SIG0.103/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0058c530 CSWGuiPartySelection::OnPanelRemoved  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0058c8a0 CSWGuiPartySelection::OnDone  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: no
  0x0052d090 CSWGuiMessageBox::SetAllowCancel -> Steam ?  in candidate callees: n/a
  0x0052e030 CSWGuiMessageBox::SetCallback -> Steam ?  in candidate callees: n/a
  0x0058a930 CSWGuiPartySelectionButton::SetSelected -> Steam ?  in candidate callees: n/a
  0x0058aa20 CSWGuiPartySelectionButton::GetSelected -> Steam ?  in candidate callees: n/a
  0x0058d520 CSWGuiPartySelection::UpdateCount -> Steam ?  in candidate callees: n/a
  0x0058e370 CSWGuiPartySelection::SetAddRemoveLabel -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: no
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x1c,0x4c,0x50,0x58,0x5c,0x6c,0x70,0xa0,0x468,0x48f8,0x50ac,0x50b0} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2,0x4*,0x2d,0x3c,0x4728,0xa588,0xa5a6,0x1a789} | Steam {0x4*,0x18}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0089da90: 16-byte aligned: YES
  0089da90  55             PUSH EBP
  0089da91  8B EC          MOV EBP,ESP
  0089da93  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0089da90
string literals (0):
direct callees (2):
  0x0089DC30  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0089da90 FUN_0089da90 size=46 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0089da90(undefined4 param_1,uint param_2)

{
  FUN_0089dc30();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0089cf30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0089cf30: 16-byte aligned: YES
  0089cf30  55             PUSH EBP
  0089cf31  8B EC          MOV EBP,ESP
  0089cf33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0089cf30
string literals (22):
  0x009A6ECC  at 0089d0f0  'partyselect_p'
  0x009A6EC0  at 0089d130  'LBL_NA%d'
  0x0099D874  at 0089d16f  'LBL_CHAR%d'
  0x009A6EB4  at 0089d1ae  'BTN_NPC%d'
  0x009A6EA8  at 0089d362  'LBL_COUNT'
  0x009A6E98  at 0089d39f  'LBL_NPC_NAME'
  0x009A6E88  at 0089d3dc  'LBL_NPC_LEVEL'
  0x009A6E80  at 0089d418  'LBL_3D'
  0x009A1818  at 0089d455  'LBL_TITLE'
  0x009A6E70  at 0089d492  'LBL_AVAILABLE'
  0x009A6E64  at 0089d4ce  'LBL_BEVEL_L'
  0x009A6E58  at 0089d50b  'LBL_BEVEL_M'
direct callees (18):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x00733540  CExoString::Constructor
  0x0091D67E  (unnamed)
  0x00419740  LabelCtor
  0x0041A510  ButtonCtor
  0x00418990  CSWGuiControl::Constructor
  0x00417F40  (unnamed)
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00734270  CExoString::Format
  0x0040F620  CSWGuiPanel::InitControl
  0x00418AF0  CSWGuiControl::AddEvent
  0x004149F0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00733780  CExoString::~CExoString
  0x00418360  (unnamed)
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00760EA0  (unnamed)
struct offsets touched (top 10):
  +0x48 [dec 72]  x8  via EAX,ECX,EDX
  +0x70 [dec 112]  x2  via EAX,ECX
  +0x74 [dec 116]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x44 [dec 68]  x1  via EDX
  +0x78 [dec 120]  x1  via EAX
  +0x3C40 [dec 15424]  x1  via ECX
  +0x3C44 [dec 15428]  x1  via EDX
  +0x4348 [dec 17224]  x1  via EAX
```

### Static facts (script output, report-only)
- size 46 B, RET 4
- callees: 0x0089dc30 (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B17-2-03

GOG function: CExoFileInternal::GetSize  gog 0x006100a0  callconv=(blank)  param_size_bytes=0  gog_size=133  gog_retn=0
Candidate Steam address: 0x0073de00 (size 24, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073ddd0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00610070 CExoFileInternal::GetOffset  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00610130 CExoFileInternal::Read -> 0x0073dc30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00922455 unnamed -> Steam ?  in candidate callees: n/a
  0x009227a7 unnamed -> Steam ?  in candidate callees: n/a
  0x00928827 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0x2,0x4,0xc} | Steam {0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0073de00: 16-byte aligned: YES
  0073de00  55             PUSH EBP
  0073de01  8B EC          MOV EBP,ESP
  0073de03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073de00
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0073de00 FUN_0073de00 size=24 ret=0 callers=1 callees=0

undefined4 FUN_0073de00(void)

{
  return DAT_00a7ea98;
}
```

### decoy: 0x0073ddd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ddd0: 16-byte aligned: YES
  0073ddd0  55             PUSH EBP
  0073ddd1  8B EC          MOV EBP,ESP
  0073ddd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ddd0
string literals (0):
direct callees (1):
  0x0073D240  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 24 B, RET 0
- callers 1: 0x00739a10
- twins: none

## Card B17-2-04

GOG function: CSWCModule::TurnCamera  gog 0x0046b0c0  callconv=(blank)  param_size_bytes=8  gog_size=885  gog_retn=8
Candidate Steam address: 0x007f3ea0 (size 885, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.80) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007f4220:STR0/CALLEE2/SIG0.741/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0046b000 CSWCModule::AcclTiltCamera  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0046b440 CSWCModule::AcclTurnCamera -> 0x007f4220  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: yes
  0x0040dd20 CClientExoApp::GetCombatMode -> Steam 0x00740860  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam 0x0077d800  in candidate callees: yes
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x00574940 CSWBehaviorCameraCombat::SetTurn -> Steam ?  in candidate callees: n/a
  0x006aabf0 CSWSObject::SetOrientation -> Steam 0x00543f10  in candidate callees: yes
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
  0x0080d740 Global::Pitch -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00438930 CClientExoAppInternal::UpdateCamera -> Steam 0x0078c350  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 14 common): GOG {0x4*,0x8*,0xc*,0x14*,0x1c*,0x28*,0x2c*,0x40*,0x80*,0x88*,0x98*,0x100*,0x10c*,0x460*} | Steam {0x4*,0x8*,0xc*,0x14*,0x1c*,0x28*,0x2c*,0x40*,0x80*,0x88*,0x98*,0x100*,0x10c*,0x460*}
imm sets (* = in both, 15 common): GOG {0x3*,0x4*,0x5*,0x7*,0x8*,0xc*,0x10*,0x30*,0x41*,0x44*,0x9c*,0x106a*,0x106e*,0x1070*,...(+1)} | Steam {0x3*,0x4*,0x5*,0x7*,0x8*,0xc*,0x10*,0x30*,0x41*,0x44*,0x9c*,0x106a*,0x106e*,0x1070*,...(+1)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007f3ea0: 16-byte aligned: YES
  007f3ea0  55             PUSH EBP
  007f3ea1  8B EC          MOV EBP,ESP
  007f3ea3  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f3ea0
string literals (0):
direct callees (11):
  0x00740860  CClientExoApp::GetCombatMode
  0x008E5480  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x00515E60  (unnamed)
  0x00515A70  (unnamed)
  0x00515990  (unnamed)
  0x005158B0  (unnamed)
  0x00515620  (unnamed)
  0x00463120  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00543F10  CSWSObject::SetOrientation
struct offsets touched (top 10):
  +0x40 [dec 64]  x14  via EAX,ECX,EDX
  +0x80 [dec 128]  x7  via EAX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x100 [dec 256]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007f3ea0 FUN_007f3ea0 size=885 ret=8 callers=2 callees=11

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007f3ea0(int param_1,float param_2,float param_3)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  float10 fVar4;
  double local_9c;
  double local_94;
  double local_8c;
  undefined1 local_5c [12];
  undefined1 local_50 [12];
  undefined1 local_44 [20];
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  float local_20;
  int *local_1c;
  int local_18;
  int local_10;
  int *local_8;
  
  iVar1 = FUN_00740860();
  if (iVar1 == 0) {
    if (*(int *)(param_1 + 0xc) == 3) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106a);
      if (iVar1 != 0) {
        *(float *)(iVar1 + 0x100) = param_2;
        *(undefined4 *)(iVar1 + 0x10c) = 0;
      }
    }
    else if (*(int *)(param_1 + 0xc) == 5) {
      local_18 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106e);
      if (local_18 != 0) {
        if (param_2 * (float)_DAT_00987f08 <= (float)_DAT_009946f8) {
          local_8c = (double)(param_2 * (float)_DAT_00987f08);
        }
        else {
          local_8c = _DAT_009946f8;
        }
        if (_DAT_009a2420 <= local_8c) {
          if (param_2 * (float)_DAT_00987f08 <= (float)_DAT_009946f8) {
            local_9c = (double)(param_2 * (float)_DAT_00987f08);
          }
          else {
            local_9c = _DAT_009946f8;
          }
          local_94 = local_9c;
        }
        else {
          local_94 = _DAT_009a2420;
        }
        local_20 = -(float)local_94 * *(float *)(local_18 + 0x28) * param_3;
        local_1c = (int *)FUN_0073f450();
        if (local_1c != (int *)0x0) {
          local_1c[0x118 = byte +0x460] = 0;
          FUN_00515e60(&local_30,local_1c + 0xc);
          fVar4 = (float10)FUN_00515a70(local_30,local_2c,local_28,local_24);
          fVar4 = (float10)FUN_00515990(&local_30,(float)fVar4);
          fVar4 = (float10)FUN_005158b0(&local_30,(float)fVar4);
          puVar2 = (undefined4 *)FUN_00515620(local_44,(float)(fVar4 + (float10)local_20));
          local_30 = *puVar2;
          local_2c = puVar2[1 = byte +0x4];
          local_28 = puVar2[2 = byte +0x8];
          local_24 = puVar2[3 = byte +0xc];
          uVar3 = FUN_00463120(local_50);
          (**(code **)(*local_1c + 0x88))(uVar3);
          if ((local_1c != (int *)0x0) && (iVar1 = FUN_0077d800(), iVar1 != 0)) {
            uVar3 = FUN_00463120(local_5c);
            FUN_0077d800(uVar3);
            FUN_00543f10(uVar3);
          }
        }
      }
    }
    else if (*(int *)(param_1 + 0xc) == 7) {
/* ... 27 more lines truncated by --max-lines */
```

### decoy: 0x007f4220 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f4220: 16-byte aligned: YES
  007f4220  55             PUSH EBP
  007f4221  8B EC          MOV EBP,ESP
  007f4223  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f4220
string literals (0):
direct callees (3):
  0x00740860  CClientExoApp::GetCombatMode
  0x008E5480  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
struct offsets touched (top 10):
  +0x40 [dec 64]  x14  via EAX,ECX,EDX
  +0x80 [dec 128]  x7  via EAX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x100 [dec 256]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

### Static facts (script output, report-only)
- size 885 B, RET 8
- callers 2: 0x0078c350 CClientExoAppInternal::UpdateCamera, 0x0078cea0
- callees: 0x00740860 CClientExoApp::GetCombatMode (CALL), 0x008e5480 (CALL), 0x0073f450 CClientExoApp::GetPlayerCreature (CALL), 0x00515e60 (CALL), 0x00515a70 (CALL), 0x00515990 (CALL), ...
- indirect: 0x007f3edc slot 0x80, 0x007f3f29 slot 0x80, 0x007f3f76 slot 0x80, 0x007f4103 slot 0x88, 0x007f415c slot 0x80, ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B17-2-05

GOG function: CSWSpellArray::GetSpell  gog 0x004ed450  callconv=(blank)  param_size_bytes=4  gog_size=57  gog_retn=4
Candidate Steam address: 0x006c1450 (size 57, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 35 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 35 (callers: HandleConjuringCreature 0x767b20, GetEquipmentForcePowers_2 0x778ba0, IsItemUsableByCurrent 0x77b670); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006c13e0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ed3e0 CSWSpell::`scalar/vector  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ed490 CSWSpellArray::Load  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (35 of 62): the card's A-CALLEE credit may come from these, not from callees
  0x004138f0 CSWCCreature::HandleConjuringCreature -> Steam 0x00767b20  in candidate callers: yes
  0x00424b60 CSWCCreature::GetEquipmentForcePowers_2 -> Steam 0x00778ba0  in candidate callers: yes
  0x004275f0 CSWCCreature::IsLeaderUsableItem -> Steam 0x0077b670  in candidate callers: yes
  0x00445390 CClientExoAppInternal::ShowObject -> Steam 0x00798d70  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x1a8*} | Steam {0x4*,0x1a8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006c1450: 16-byte aligned: YES
  006c1450  55             PUSH EBP
  006c1451  8B EC          MOV EBP,ESP
  006c1453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006c1450
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006c1450 FUN_006c1450 size=57 ret=4 callers=62 callees=0

int __thiscall FUN_006c1450(int *param_1,int param_2)

{
  int iVar1;
  
  if (((param_1[1 = byte +0x4] == 0) || (param_2 < 0)) || (*param_1 <= param_2)) {
    iVar1 = 0;
  }
  else {
    iVar1 = param_2 * 0x1a8 + param_1[1 = byte +0x4];
  }
  return iVar1;
}
```

### decoy: 0x006c13e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006c13e0: 16-byte aligned: YES
  006c13e0  55             PUSH EBP
  006c13e1  8B EC          MOV EBP,ESP
  006c13e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006c13e0
string literals (0):
direct callees (3):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
  0x006C0FD0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 57 B, RET 4
- callers 62: 0x005433a0, 0x005482a0, 0x00548b10, 0x005648d0 CSWSCreature::AddItemCastSpellActions, 0x00565310 CSWSCreature::AddCastSpellActions, ...
- twins: none

## Card B17-2-06

GOG function: CClientOptions::SetVSync  gog 0x004691f0  callconv=(blank)  param_size_bytes=4  gog_size=53  gog_retn=4
Candidate Steam address: 0x007bc520 (size 53, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc4f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004691c0 CClientOptions::SetAnisotropy  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00469230 CClientOptions::SetGammaSetting  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086ac50 Global::AurSetVSyncState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x48*} | Steam {0x48*}
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
prologue for 0x007bc520: 16-byte aligned: YES
  007bc520  55             PUSH EBP
  007bc521  8B EC          MOV EBP,ESP
  007bc523  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bc520
string literals (0):
direct callees (1):
  0x00478D70  (unnamed)
struct offsets touched (top 1):
  +0x48 [dec 72]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007bc520 FUN_007bc520 size=53 ret=4 callers=2 callees=1

void __thiscall FUN_007bc520(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x48) = param_2;
  if (*(int *)(param_1 + 0x48) == 0) {
    FUN_00478d70(0);
  }
  else {
    FUN_00478d70(1);
  }
  return;
}
```

### decoy: 0x007bc4f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bc4f0: 16-byte aligned: YES
  007bc4f0  55             PUSH EBP
  007bc4f1  8B EC          MOV EBP,ESP
  007bc4f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bc4f0
string literals (0):
direct callees (1):
  0x00478D50  CSWCArea::SetGlobalLightAmbient
struct offsets touched (top 1):
  +0x3C [dec 60]  x1  via EAX
```

### Static facts (script output, report-only)
- size 53 B, RET 4
- callers 2: 0x007b88b0 ClientOptions ini reader ([Game Options] Auto Level Up NPCs etc.), 0x007d75f0
- callees: 0x00478d70 (CALL)
- twins: none

## Card B17-2-07

GOG function: Global::GetResource  gog 0x00867dc0  callconv=(blank)  param_size_bytes=4  gog_size=71  gog_retn=0
Candidate Steam address: 0x00475ea0 (size 192, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CloseModelStream | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00475e50:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00867cc0 Global::AurResGet_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00867e10 Global::AurResFree  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x18}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x10}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00475ea0: 16-byte aligned: YES
  00475ea0  55             PUSH EBP
  00475ea1  8B EC          MOV EBP,ESP
  00475ea3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00475ea0
string literals (0):
direct callees (5):
  0x0091C5B3  (unnamed)
  0x00476620  (unnamed)
  0x00449170  (unnamed)
  0x00475A70  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x18 [dec 24]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00475ea0 FUN_00475ea0 size=192 ret=0 callers=11 callees=5

void FUN_00475ea0(int *param_1,undefined4 param_2)

{
  int iVar1;
  
  if (param_1 != (int *)0x0) {
    if (*param_1 == 0) {
      (*DAT_00a30830)(param_1[6 = byte +0x18],param_2);
      param_1[6 = byte +0x18] = 0;
    }
    else {
      _fclose [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1);
      *param_1 = 0;
    }
    iVar1 = FUN_00476620(param_1);
    if (iVar1 != 0) {
      FUN_00449170(param_1);
    }
    if (param_1 != (int *)0x0) {
      FUN_00475a70();
      FUN_009196fd(param_1);
    }
  }
  return;
}
```

### decoy: 0x00475e50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475e50: 16-byte aligned: YES
  00475e50  55             PUSH EBP
  00475e51  8B EC          MOV EBP,ESP
  00475e53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00475e50
string literals (0):
direct callees (1):
  0x0049B840  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 192 B, RET 0
- callers 11: 0x00424b10 CAurTextureBasic::Init, 0x00452120, 0x0045bf50 Gob::EnableRenderBumpedOut, 0x0046fbd0 Scene::LoadVisibility, 0x00470000, ...
- callees: 0x0091c5b3 (CALL), 0x00476620 (CALL), 0x00449170 (CALL), 0x00475a70 (CALL), 0x009196fd operator delete (CALL)
- indirect: 0x00475eec unresolved
- twins: none

## Card B17-2-08

GOG function: Global::AurTextureBindTargetIndex  gog 0x008144e0  callconv=(blank)  param_size_bytes=8  gog_size=69  gog_retn=0
Candidate Steam address: 0x00421e30 (size 145, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CAurTexture::Constructor | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00421de0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008143f0 Global::AurTextureBindTID  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00814530 CAurTexture::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x00819810 CAurTextureBasic::AsNormalizeVectorCubeMap -> Steam 0x00427110  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x14}
imm sets (* = in both, 0 common): GOG {0x84fe} | Steam {0x4,0xc,0x10}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00421e30: 16-byte aligned: YES
  00421e30  55             PUSH EBP
  00421e31  8B EC          MOV EBP,ESP
  00421e33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00421e30
string literals (0):
direct callees (2):
  0x00449230  (unnamed)
  0x004741E0  SafePointer_T_::operator=
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00421e30 FUN_00421e30 size=145 ret=0 callers=3 callees=2

undefined4 * __fastcall FUN_00421e30(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00948e96;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00449230(0);
  *param_1 = CAurTexture::vftable;
  param_1[4 = byte +0x10] = 0;
  local_8 = 1;
  FUN_004741e0(0);
  param_1[5 = byte +0x14] = 0;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00421de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00421de0: 16-byte aligned: YES
  00421de0  55             PUSH EBP
  00421de1  8B EC          MOV EBP,ESP
  00421de3  8B 45 0C       MOV EAX,dword ptr [EBP + 0xc]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00421de0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 145 B, RET 0
- callers 3: 0x00423000 CAurTextureBasic::Constructor, 0x00427410, 0x00427520
- callees: 0x00449230 (CALL), 0x004741e0 (CALL)
- twins: none

## Card B17-2-09

GOG function: Global::AurTextureBindTargetIndex  gog 0x008144e0  callconv=(blank)  param_size_bytes=8  gog_size=69  gog_retn=0
Candidate Steam address: 0x00421cf0 (size 231, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00421de0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008143f0 Global::AurTextureBindTID  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00814530 CAurTexture::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x00819810 CAurTextureBasic::AsNormalizeVectorCubeMap -> Steam 0x00427110  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0x84fe} | Steam {0x2,0x4,0x8,0xc,0x14}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00421cf0: 16-byte aligned: YES
  00421cf0  55             PUSH EBP
  00421cf1  8B EC          MOV EBP,ESP
  00421cf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00421cf0
string literals (0):
direct callees (2):
  0x004849E0  (unnamed)
  0x00421DE0  (unnamed)
struct offsets touched (top 1):
  +0x9F4928 [dec 10438952]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00421cf0 FUN_00421cf0 size=231 ret=0 callers=2 callees=4

void FUN_00421cf0(uint param_1,char param_2,char param_3)

{
  int iVar1;
  bool bVar2;
  
  bVar2 = false;
  iVar1 = FUN_004849e0();
  if (iVar1 != 0) {
    bVar2 = param_2 != '\0';
    if (bVar2) {
      glEnable(DAT_009f4958);
    }
    else {
      glDisable(DAT_009f4958);
    }
  }
  if (param_3 == '\0') {
    glDisable(DAT_009f4978);
  }
  else {
    glEnable(DAT_009f4978);
  }
  if ((param_1 < 2) || (-1 < DAT_00a1bbd4)) {
    if (bVar2) {
      FUN_00421de0(0xc,param_1);
    }
    else if (param_3 == '\0') {
      FUN_00421de0(4,param_1);
    }
    else {
      FUN_00421de0(0x14,param_1);
    }
    DAT_00a1bbd8 = DAT_00a1bbd8 + 1;
  }
  return;
}
```

### decoy: 0x00421de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00421de0: 16-byte aligned: YES
  00421de0  55             PUSH EBP
  00421de1  8B EC          MOV EBP,ESP
  00421de3  8B 45 0C       MOV EAX,dword ptr [EBP + 0xc]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00421de0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 231 B, RET 0
- callers 2: 0x00422ed0, 0x004265e0 CAurTextureBasic::glImage
- callees: 0x004849e0 (CALL), 0x0000000e (CALL), 0x0000001c (CALL), 0x00421de0 (CALL)
- imports: glDisable (OPENGL32.DLL), glEnable (OPENGL32.DLL)
- twins: none

## Card B17-2-10

GOG function: CSWGuiCustomPanel::OnSelectFeatsButton  gog 0x005f42f0  callconv=(blank)  param_size_bytes=4  gog_size=203  gog_retn=4
Candidate Steam address: 0x00914f40 (size 198, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.71) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00914e70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005f4220 CSWGuiCustomPanel::OnSelectNameButton  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005f43c0 CSWGuiCustomPanel::OnSelectSkillsButton  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005f8c90 CSWGuiFeatsCharGen::Constructor -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x1c*,0x50*,0x68*,0x6c*,0x2120*} | Steam {0x1c*,0x50*,0x68*,0x6c*,0x2120*}
imm sets (* = in both, 3 common): GOG {0x3*,0x4*,0x18*,0x1c08} | Steam {0x2,0x3*,0x4*,0x18*,0x4d00}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00914f40: 16-byte aligned: YES
  00914f40  55             PUSH EBP
  00914f41  8B EC          MOV EBP,ESP
  00914f43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00914f40
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x0090C6A0  CSWGuiSkillsCharGen::Constructor
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 5):
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
  +0x6C [dec 108]  x1  via EAX
  +0x2120 [dec 8480]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00914f40 FUN_00914f40 size=198 ret=4 callers=0 callees=3

void __thiscall FUN_00914f40(int param_1,int param_2)

{
  int iVar1;
  int local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009821bb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((1 < *(int *)(param_1 + 0x2120)) && (*(int *)(param_2 + 0x50) != 0)) {
    iVar1 = FUN_00919723(0x4d00,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_28 = 0;
    }
    else {
      local_28 = FUN_0090c6a0(*(undefined4 *)(param_1 + 0x1c),*(undefined4 *)(param_1 + 0x68),0);
    }
    local_8 = 0xffffffff;
    *(int *)(local_28 + 0x6c) = param_1;
    FUN_00410530(local_28,3,1);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00914e70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00914e70: 16-byte aligned: YES
  00914e70  55             PUSH EBP
  00914e71  8B EC          MOV EBP,ESP
  00914e73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00914e70
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x00909E00  CSWGuiFeatsCharGen::Constructor
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 5):
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
  +0x6C [dec 108]  x1  via EAX
  +0x2120 [dec 8480]  x1  via EAX
```

### Static facts (script output, report-only)
- size 198 B, RET 4
- callees: 0x00919723 operator new (CALL), 0x0090c6a0 (CALL), 0x00410530 CSWGuiManager::AddPanel (CALL)
- twins: none

## Card B17-2-11

GOG function: CSWCModule::TiltCamera  gog 0x0046ae90  callconv=(blank)  param_size_bytes=4  gog_size=356  gog_retn=4
Candidate Steam address: 0x007f3c70 (size 356, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007f3de0:STR0/CALLEE1/SIG0.55/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0046ae40 CSWCModule::UpdateCameraModeBG  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0046b000 CSWCModule::AcclTiltCamera  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00438930 CClientExoAppInternal::UpdateCamera -> Steam 0x0078c350  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0xc*,0x18*,0x1c*,0x38*,0x40*,0x48*,0x80*,0x94*,0x104*} | Steam {0xc*,0x18*,0x1c*,0x38*,0x40*,0x48*,0x80*,0x94*,0x104*}
imm sets (* = in both, 9 common): GOG {0x3*,0x4*,0x5*,0x7*,0x1c*,0x44*,0x106a*,0x106e*,0x1071*} | Steam {0x3*,0x4*,0x5*,0x7*,0x1c*,0x44*,0x106a*,0x106e*,0x1071*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007f3c70: 16-byte aligned: YES
  007f3c70  55             PUSH EBP
  007f3c71  8B EC          MOV EBP,ESP
  007f3c73  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f3c70
string literals (0):
direct callees (0):
struct offsets touched (top 9):
  +0x40 [dec 64]  x9  via EAX,ECX,EDX
  +0x80 [dec 128]  x6  via ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x48 [dec 72]  x3  via ECX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x38 [dec 56]  x1  via ECX
  +0x94 [dec 148]  x1  via EAX
  +0x104 [dec 260]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007f3c70 FUN_007f3c70 size=356 ret=4 callers=1 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007f3c70(int param_1,float param_2)

{
  int iVar1;
  int *piVar2;
  uint local_20;
  
  if (*(int *)(param_1 + 0xc) == 3) {
    iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106a);
    if (iVar1 != 0) {
      *(float *)(iVar1 + 0x104) = param_2;
    }
  }
  else if (*(int *)(param_1 + 0xc) == 5) {
    iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106e);
    if (iVar1 != 0) {
      *(float *)(iVar1 + 0x38) = param_2;
    }
  }
  else if (*(int *)(param_1 + 0xc) == 7) {
    iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x1071);
    if (iVar1 != 0) {
      *(float *)(iVar1 + 0x18) = -param_2 * (float)_DAT_00991358;
    }
  }
  else {
    if (((*(int *)(param_1 + 0x40) != 0) &&
        (piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0xffffffff),
        piVar2 != (int *)0x0)) && (iVar1 = (**(code **)(*piVar2 + 0x1c))(), iVar1 != 0)) {
      *(float *)(iVar1 + 0x48) = *(float *)(iVar1 + 0x48) + param_2;
      *(undefined4 *)(param_1 + 0x94) = *(undefined4 *)(iVar1 + 0x48);
    }
    local_20 = (uint)(param_2 != (float)_DAT_00986b38);
    *(uint *)(param_1 + 0x80) = *(uint *)(param_1 + 0x80) | local_20;
  }
  return;
}
```

### decoy: 0x007f3de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f3de0: 16-byte aligned: YES
  007f3de0  55             PUSH EBP
  007f3de1  8B EC          MOV EBP,ESP
  007f3de3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f3de0
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0x40 [dec 64]  x6  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x80 [dec 128]  x3  via EDX
  +0x24 [dec 36]  x1  via ECX
  +0x104 [dec 260]  x1  via ECX
```

### Static facts (script output, report-only)
- size 356 B, RET 4
- callers 1: 0x0078c350 CClientExoAppInternal::UpdateCamera
- indirect: 0x007f3c9b slot 0x80, 0x007f3cdb slot 0x80, 0x007f3d18 slot 0x80, 0x007f3d5a slot 0x80, 0x007f3d70 slot 0x1c
- twins: none

## Card B17-2-12

GOG function: CSWCMessage::HandleServerToPlayerGuiContainerObjectMessage  gog 0x004bf300  callconv=(blank)  param_size_bytes=1  gog_size=129  gog_retn=4
Candidate Steam address: 0x0082acb0 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: ShowContainerGui 0x7cacb0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00833a60:STR0/CALLEE0/SIG0.6/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004bf2c0 CSWCMessage::HandleServerToPlayerUpdate_Update_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004bf390 CSWCMessage::HandleServerToPlayerAreaOfEffectUpdate_Delete_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x004db3b0 CGuiInGame::ShowContainerGui -> Steam 0x007cacb0  in candidate callees: yes
  0x0054b0b0 CSWMessage::ReadINT -> Steam ?  in candidate callees: n/a
  0x0054b4c0 CSWMessage::MessageReadOverflow -> Steam ?  in candidate callees: n/a
  0x0054b520 CSWMessage::MessageReadUnderflow -> Steam ?  in candidate callees: n/a
  0x00562510 CSWCMessage::ReadOBJECTIDClient -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 4 common): GOG {0x4*,0x10*,0x20*,0x7f000000*} | Steam {0x4*,0x10*,0x20*,0x7f000000*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0082acb0: 16-byte aligned: YES
  0082acb0  55             PUSH EBP
  0082acb1  8B EC          MOV EBP,ESP
  0082acb3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0082acb0
string literals (0):
direct callees (6):
  0x008935A0  (unnamed)
  0x005E39D0  CSWMessage::ReadDWORD
  0x005E3DE0  (unnamed)
  0x005E3E40  (unnamed)
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CACB0  CGuiInGame::ShowContainerGui
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0082acb0 FUN_0082acb0 size=129 ret=4 callers=1 callees=6

undefined4 __thiscall FUN_0082acb0(undefined4 param_1,char param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  
  iVar1 = FUN_008935a0(param_1);
  uVar2 = FUN_005e39d0(0x20);
  iVar3 = FUN_005e3de0(1);
  if ((iVar3 == 0) && (iVar3 = FUN_005e3e40(1), iVar3 == 0)) {
    if ((param_2 == '\x01') && (iVar1 != 0x7f000000)) {
      FUN_0073f750(iVar1,uVar2,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
      FUN_007cacb0(iVar1,uVar2);
    }
    return 1;
  }
  return 0;
}
```

### decoy: 0x00833a60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00833a60: 16-byte aligned: YES
  00833a60  55             PUSH EBP
  00833a61  8B EC          MOV EBP,ESP
  00833a63  68 44 30 9A 00 PUSH 0x9a3044
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00833a60
string literals (2):
  0x009A3044  at 00833a63  'gunbank'
  0x009A3044  at 00833a71  'gunbank'
direct callees (2):
  0x00919F10  _strlen
  0x0093720B  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### Static facts (script output, report-only)
- size 129 B, RET 4
- callers 1: 0x00810cf0
- callees: 0x008935a0 (CALL), 0x005e39d0 (CALL), 0x005e3de0 (CALL), 0x005e3e40 (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x007cacb0 (CALL)
- globals: 0x00a1b4a4 READ
- twins: none

## Card B17-2-13

GOG function: CSWGuiCustomPanel::OnSelectFeatsButton  gog 0x005f42f0  callconv=(blank)  param_size_bytes=4  gog_size=203  gog_retn=4
Candidate Steam address: 0x00914da0 (size 204, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.57 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00914e70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005f4220 CSWGuiCustomPanel::OnSelectNameButton  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005f43c0 CSWGuiCustomPanel::OnSelectSkillsButton  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005f8c90 CSWGuiFeatsCharGen::Constructor -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x1c*,0x50*,0x68*,0x6c,0x2120*} | Steam {0x1c*,0x50*,0x68*,0x2120*}
imm sets (* = in both, 2 common): GOG {0x3,0x4*,0x18*,0x1c08} | Steam {0x2,0x4*,0x18*,0xa0c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00914da0: 16-byte aligned: YES
  00914da0  55             PUSH EBP
  00914da1  8B EC          MOV EBP,ESP
  00914da3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00914da0
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x00918B10  (unnamed)
  0x009192D0  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 4):
  +0x1C [dec 28]  x2  via EDX
  +0x50 [dec 80]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
  +0x2120 [dec 8480]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00914da0 FUN_00914da0 size=204 ret=4 callers=0 callees=4

void __thiscall FUN_00914da0(int param_1,int param_2)

{
  int iVar1;
  undefined4 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009821bb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((3 < *(int *)(param_1 + 0x2120)) && (*(int *)(param_2 + 0x50) != 0)) {
    iVar1 = FUN_00919723(0xa0c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_28 = 0;
    }
    else {
      local_28 = FUN_00918b10(*(undefined4 *)(param_1 + 0x1c),*(undefined4 *)(param_1 + 0x68));
    }
    local_8 = 0xffffffff;
    FUN_009192d0(param_1,2);
    FUN_00410530(local_28,1,1);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00914e70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00914e70: 16-byte aligned: YES
  00914e70  55             PUSH EBP
  00914e71  8B EC          MOV EBP,ESP
  00914e73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00914e70
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x00909E00  CSWGuiFeatsCharGen::Constructor
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 5):
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
  +0x6C [dec 108]  x1  via EAX
  +0x2120 [dec 8480]  x1  via EAX
```

### Static facts (script output, report-only)
- size 204 B, RET 4
- callees: 0x00919723 operator new (CALL), 0x00918b10 (CALL), 0x009192d0 (CALL), 0x00410530 CSWGuiManager::AddPanel (CALL)
- twins: none

## Card B17-2-14

GOG function: CSWCItem::LoadModel  gog 0x0047e830  callconv=(blank)  param_size_bytes=5  gog_size=326  gog_retn=12
Candidate Steam address: 0x007fd5b0 (size 326, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007fd700:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0047e5f0 CSWCItem::AIUpdate  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0047e980 CSWCItem::SetEquippedBy  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00472990 CSWCObject::GetAnimationBase -> Steam 0x007ed830  in candidate callees: yes
  0x004c76b0 CSWCAnimBase::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x8*,0xc*,0x68*,0x74*,0x7c*,0xa4*,0xc4*} | Steam {0x8*,0xc*,0x68*,0x74*,0x7c*,0xa4*,0xc4*}
imm sets (* = in both, 7 common): GOG {0x4*,0x5*,0xc*,0x10*,0x1c*,0xfc*,0xff*} | Steam {0x4*,0x5*,0xc*,0x10*,0x1c*,0xfc*,0xff*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 5 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x007fd5b0: 16-byte aligned: YES
  007fd5b0  55             PUSH EBP
  007fd5b1  8B EC          MOV EBP,ESP
  007fd5b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007fd5b0
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x0085F7E0  CSWCAnimBase::Constructor
  0x007ED830  CSWCObject::GetAnimationBase
struct offsets touched (top 7):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x74 [dec 116]  x1  via EDX
  +0x7C [dec 124]  x1  via EAX
  +0xA4 [dec 164]  x1  via EDX
  +0xC4 [dec 196]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007fd5b0 FUN_007fd5b0 size=326 ret=12 callers=0 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_007fd5b0(int param_1,undefined4 param_2,char param_3)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952aeb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x68) == 0) {
    iVar1 = FUN_00919723(0xfc,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_24 = 0;
    }
    else {
      local_24 = FUN_0085f7e0();
    }
    *(undefined4 *)(param_1 + 0x68) = local_24;
    if (*(int *)(param_1 + 0x68) == 0) {
      ExceptionList = local_10;
      return 0;
    }
  }
  local_8 = 0xffffffff;
  piVar2 = (int *)FUN_007ed830();
  iVar1 = (**(code **)(*piVar2 + 0xc))(param_2,param_3);
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    piVar2 = (int *)FUN_007ed830();
    piVar2 = (int *)(**(code **)(*piVar2 + 0x8 [dec 8] {int*: byte +0x20}))(param_3);
    if (piVar2 != (int *)0x0) {
      (**(code **)(*piVar2 + 0x74))(param_1 + -0x10);
      (**(code **)(*piVar2 + 0x7c))(DAT_00994475);
    }
    (**(code **)(*piVar2 + 0xc4))(5);
    if (param_3 == -1) {
      *(undefined4 *)(param_1 + 0xa4) = _DAT_0098c014;
    }
    uVar3 = 1;
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x007fd700 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007fd700: 16-byte aligned: YES
  007fd700  55             PUSH EBP
  007fd701  8B EC          MOV EBP,ESP
  007fd703  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007fd700
string literals (0):
direct callees (1):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
struct offsets touched (top 8):
  +0x18C [dec 396]  x4  via EAX,EDX
  +0x1A0 [dec 416]  x4  via EDX
  +0x160 [dec 352]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
  +0x98 [dec 152]  x1  via EDX
  +0x24C [dec 588]  x1  via EDX
  +0x254 [dec 596]  x1  via ECX
```

### Static facts (script output, report-only)
- size 326 B, RET 12
- callees: 0x00919723 operator new (CALL), 0x0085f7e0 (CALL), 0x007ed830 (CALL)
- indirect: 0x007fd657 slot 0xc, 0x007fd67f slot 0x8, 0x007fd69c slot 0x74, 0x007fd6b1 slot 0x7c, 0x007fd6c3 slot 0xc4
- twins: none

## Card B17-2-15

GOG function: CSWCMessage::HandleServerToPlayerUpdate_GuiContainer  gog 0x004aa610  callconv=(blank)  param_size_bytes=0  gog_size=647  gog_retn=0
Candidate Steam address: 0x00816020 (size 647, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00815ec0:STR0/CALLEE0/SIG0.678/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004aa4b0 CSWCMessage::HandleServerToPlayerUpdate_GuiRepository  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004aa8b0 CSWCMessage::HandleServerToPlayerUpdate_Update  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: yes
  0x0047ef40 CSWCItem::SetStackSize -> Steam ?  in candidate callees: n/a
  0x004a4470 CSWCMessage::HandleServerToPlayerItemUpdate_Add -> Steam ?  in candidate callees: n/a
  0x0054ae00 CSWMessage::ReadBYTE -> Steam ?  in candidate callees: n/a
  0x0054ae80 CSWMessage::ReadCHAR -> Steam ?  in candidate callees: n/a
  0x0054b030 CSWMessage::ReadDWORD -> Steam ?  in candidate callees: n/a
  0x0054b4c0 CSWMessage::MessageReadOverflow -> Steam ?  in candidate callees: n/a
  0x00562510 CSWCMessage::ReadOBJECTIDClient -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x15c*,0x15d*,0x168*,0x180*} | Steam {0x4*,0x15c*,0x15d*,0x168*,0x180*}
imm sets (* = in both, 7 common): GOG {0x8*,0x20*,0x41*,0x44*,0x4d*,0x50*,0x55*} | Steam {0x8*,0x20*,0x41*,0x44*,0x4d*,0x50*,0x55*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00816020: 16-byte aligned: YES
  00816020  55             PUSH EBP
  00816021  8B EC          MOV EBP,ESP
  00816023  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00816020
string literals (0):
direct callees (8):
  0x005E37A0  (unnamed)
  0x005E3DE0  (unnamed)
  0x005E3720  (unnamed)
  0x0080FE80  (unnamed)
  0x008935A0  (unnamed)
  0x005E3950  (unnamed)
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x007FDC90  CSWCItem::SetStackSize
struct offsets touched (top 5):
  +0x15C [dec 348]  x5  via EAX,ECX,EDX
  +0x15D [dec 349]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x168 [dec 360]  x1  via ECX
  +0x180 [dec 384]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00816020 FUN_00816020 size=647 ret=0 callers=1 callees=8

undefined4 FUN_00816020(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 extraout_var;
  int local_10[2 = byte +0x8];
  char local_7;
  undefined1 local_6;
  undefined1 local_5;
  
  local_7 = FUN_005e37a0(8);
  uVar2 = extraout_var;
  iVar1 = FUN_005e3de0(1);
  if (iVar1 != 0) {
    return 0;
  }
  if (local_7 == 'P') {
    FUN_005e3720(8);
  }
  else {
    if (local_7 == 'A') {
      local_6 = FUN_005e3720(8);
      local_5 = FUN_005e3720(8);
      iVar1 = FUN_005e3de0(1);
      if (iVar1 != 0) {
        return 0;
      }
      FUN_0080fe80(1,local_10);
      *(undefined1 *)(local_10[0] + 0x15c) = local_6;
      *(undefined1 *)(local_10[0] + 0x15d) = local_5;
      *(undefined4 *)(local_10[0] + 0x168) = 1;
      return 1;
    }
    if (local_7 == 'U') {
      uVar2 = FUN_008935a0(uVar2);
      uVar3 = FUN_005e3950(0x20);
      uVar4 = FUN_005e3950(0x20);
      iVar1 = FUN_005e3de0(1);
      if (iVar1 != 0) {
        return 0;
      }
      iVar1 = FUN_0073f530(uVar2);
      if (iVar1 != 0) {
        FUN_007fdc90(uVar3);
        *(undefined4 *)(iVar1 + 0x180) = uVar4;
      }
      return 1;
    }
    if (local_7 == 'D') {
      uVar2 = FUN_008935a0(uVar2);
      iVar1 = FUN_005e3de0(1);
      if (iVar1 != 0) {
        return 0;
      }
      FUN_0073f530(uVar2);
      return 1;
    }
    if (local_7 == 'M') {
      local_6 = FUN_005e3720(8);
      local_5 = FUN_005e3720(8);
      uVar2 = FUN_008935a0(uVar2);
      iVar1 = FUN_005e3de0(1);
      if (iVar1 != 0) {
        return 0;
      }
      iVar1 = FUN_0073f530(uVar2);
      if (iVar1 == 0) {
        return 0;
      }
      *(undefined1 *)(iVar1 + 0x15c) = local_6;
      *(undefined1 *)(iVar1 + 0x15d) = local_5;
      return 1;
    }
  }
  return 0;
}
```

### decoy: 0x00815ec0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00815ec0: 16-byte aligned: YES
  00815ec0  55             PUSH EBP
  00815ec1  8B EC          MOV EBP,ESP
  00815ec3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00815ec0
string literals (0):
direct callees (7):
  0x005E37A0  (unnamed)
  0x005E3DE0  (unnamed)
  0x0080FE80  (unnamed)
  0x008935A0  (unnamed)
  0x005E3950  (unnamed)
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x007FDC90  CSWCItem::SetStackSize
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x15C [dec 348]  x1  via EAX
  +0x15D [dec 349]  x1  via EDX
  +0x180 [dec 384]  x1  via EDX
```

### Static facts (script output, report-only)
- size 647 B, RET 0
- callers 1: 0x00815b00
- callees: 0x005e37a0 (CALL), 0x005e3de0 (CALL), 0x005e3720 (CALL), 0x0080fe80 (CALL), 0x008935a0 (CALL), 0x005e3950 (CALL), ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B17-2-16

GOG function: CSWGuiPanel::OnBButtonPressed  gog 0x004ce370  callconv=__thiscall  param_size_bytes=0  gog_size=30  gog_retn=4
Candidate Steam address: 0x008878d0 (size 30, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008878b0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004ce350 CSWGuiPanel::OnAButtonPressed  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ce390 CSWGuiPanel::OnXButtonPressed  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3c*} | Steam {0x3c*}
imm sets (* = in both, 2 common): GOG {0x4*,0x28*} | Steam {0x4*,0x28*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008878d0: 16-byte aligned: YES
  008878d0  55             PUSH EBP
  008878d1  8B EC          MOV EBP,ESP
  008878d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008878d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3C [dec 60]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008878d0 FUN_008878d0 size=30 ret=4 callers=0 callees=0

void __fastcall FUN_008878d0(int *param_1)

{
  (**(code **)(*param_1 + 0x3c))(0x28,1);
  return;
}
```

### decoy: 0x008878b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008878b0: 16-byte aligned: YES
  008878b0  55             PUSH EBP
  008878b1  8B EC          MOV EBP,ESP
  008878b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008878b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3C [dec 60]  x1  via EDX
```

### Static facts (script output, report-only)
- size 30 B, RET 4
- indirect: 0x008878e6 slot 0x3c
- twins: none

## Card B17-2-17

GOG function: CSWCAnimBase::LoadModel  gog 0x004c7ab0  callconv=(blank)  param_size_bytes=5  gog_size=556  gog_retn=8
Candidate Steam address: 0x0085fbe0 (size 556, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 2 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'CSWCAnimBase::LoadModel(): The headconju'@009a4648; 'CSWCAnimBase::LoadModel(): The handconju'@009a46f0; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0085fe10:STR0/CALLEE0/SIG0.311/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004c7a80 CSWCAnimBase::GetModel -> 0x0085fbb0  gate_class=REVIEW
- GOG-order neighbour after:  0x004c7ce0 CSWCAnimBase::ReplaceTexture  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: yes
  0x0061a2d0 CResRef::CopyToString_2 -> Steam ?  in candidate callees: n/a
  0x0061a5f0 CResRef::operator= -> Steam ?  in candidate callees: n/a
  0x008548b0 Global::NewCAurObject -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (4): '?headconjure'; 'handconjure'; 'CSWCAnimBase::LoadModel(): The headconju'; 'CSWCAnimBase::LoadModel(): The handconju'
disp sets (* = in both, 4 common): GOG {0x3c*,0xa0*,0xac*,0xb8*} | Steam {0x3c*,0xa0*,0xac*,0xb8*}
imm sets (* = in both, 7 common): GOG {0x8*,0xc*,0x10*,0x2c*,0x44*,0x58*,0xff*} | Steam {0x8*,0xc*,0x10*,0x2c*,0x44*,0x58*,0xff*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0085fbe0: 16-byte aligned: YES
  0085fbe0  55             PUSH EBP
  0085fbe1  8B EC          MOV EBP,ESP
  0085fbe3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0085fbe0
string literals (3):
  0x009A2C6C  at 0085fc85  'handconjure'
  0x009A46F0  at 0085fcfd  "CSWCAnimBase::LoadModel(): The handconjure dummy has an orientation....It shouldn't!!  The %s model needs to be fixed or else the spell visuals will not be correct."
  0x009A4648  at 0085fd9d  "CSWCAnimBase::LoadModel(): The headconjure dummy has an orientation....It shouldn't!!  The %s model needs to be fixed or else the spell visuals will not be correct."
direct callees (7):
  0x00710980  (unnamed)
  0x00462320  (unnamed)
  0x00733540  CExoString::Constructor
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
  0x00710CA0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 4):
  +0xB8 [dec 184]  x6  via ECX,EDX
  +0xA0 [dec 160]  x2  via EDX
  +0x3C [dec 60]  x1  via EDX
  +0xAC [dec 172]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0085fbe0 FUN_0085fbe0 size=556 ret=8 callers=1 callees=7

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0085fbe0(int *param_1,undefined4 param_2,char param_3)

{
  uint uVar1;
  int iVar2;
  undefined1 local_64 [16];
  undefined1 local_54 [8];
  undefined1 local_4c [8];
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  undefined1 local_34 [20];
  uint local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00973590;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_38 = 0.0;
  local_3c = 0.0;
  local_40 = 0.0;
  local_44 = 1.0;
  local_1c = 0;
  local_18 = 0;
  local_14 = 0;
  local_20 = uVar1;
  if (param_3 == -1) {
    FUN_00710980(local_34);
    iVar2 = FUN_00462320(local_34,&DAT_009a4798,0,0,uVar1);
    param_1[0x2e = byte +0xb8] = iVar2;
    if (param_1[0x2e = byte +0xb8] != 0) {
      iVar2 = (**(code **)(*(int *)param_1[0x2e = byte +0xb8] + 0xa0))("handconjure",&local_1c,&local_44);
      if ((iVar2 != 0) &&
         ((((local_40 != (float)_DAT_00986b38 || (local_3c != (float)_DAT_00986b38)) ||
           (local_38 != (float)_DAT_00986b38)) || (local_44 != (float)_DAT_00986b50)))) {
        FUN_00733540();
        local_8 = 0;
        FUN_00734270(local_4c,
                     "CSWCAnimBase::LoadModel(): The handconjure dummy has an orientation....It shouldn\'t!!  The %s model needs to be fixed or else the spell visuals will not be correct."
                     ,local_34);
        local_8 = 0xffffffff;
        FUN_00733780();
      }
      iVar2 = (**(code **)(*(int *)param_1[0x2e = byte +0xb8] + 0xa0))("headconjure",&local_1c,&local_44);
      if ((iVar2 != 0) &&
         (((local_40 != (float)_DAT_00986b38 || (local_3c != (float)_DAT_00986b38)) ||
          ((local_38 != (float)_DAT_00986b38 || (local_44 != (float)_DAT_00986b50)))))) {
        FUN_00733540();
        local_8 = 1;
        FUN_00734270(local_54,
                     "CSWCAnimBase::LoadModel(): The headconjure dummy has an orientation....It shouldn\'t!!  The %s model needs to be fixed or else the spell visuals will not be correct."
                     ,local_34);
        local_8 = 0xffffffff;
        FUN_00733780();
      }
      FUN_00710ca0(local_64,param_2);
      param_1[0xf = byte +0x3c] = 0;
      (**(code **)(*param_1 + 0xac))(0);
    }
  }
  ExceptionList = local_10;
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_20 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x0085fe10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0085fe10: 16-byte aligned: YES
  0085fe10  55             PUSH EBP
  0085fe11  8B EC          MOV EBP,ESP
  0085fe13  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0085fe10
string literals (0):
direct callees (4):
  0x00710980  (unnamed)
  0x00946714  (unnamed)
  0x00710CA0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0xB8 [dec 184]  x3  via EAX,ECX
  +0x34 [dec 52]  x1  via EDX
  +0x3C [dec 60]  x1  via EAX
```

### Static facts (script output, report-only)
- size 556 B, RET 8
- callers 1: 0x0086c260
- callees: 0x00710980 (CALL), 0x00462320 (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), 0x00734270 CExoString::Format (CALL), 0x00733780 CExoString::Destructor (CALL), 0x00710ca0 (CALL), ...
- indirect: 0x0085fca4 slot 0xa0, 0x0085fd44 slot 0xa0, 0x0085fdea slot 0xac
- strings: 0x009a4648 "CSWCAnimBase::LoadModel(): The headconjure dummy has an orientation....It shouldn't!!  The %s model needs to be fixed or else the spell visuals will not be correct.", 0x009a46f0 "CSWCAnimBase::LoadModel(): The handconjure dummy has an orientation....It shouldn't!!  The %s model needs to be fixed or else the spell visuals will not be correct."
- twins: none

## Card B17-2-18

GOG function: CSWCModule::AcclTurnCamera  gog 0x0046b440  callconv=(blank)  param_size_bytes=8  gog_size=643  gog_retn=8
Candidate Steam address: 0x007f4220 (size 643, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.71) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007f3ea0:STR0/CALLEE2/SIG0.741/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0046b0c0 CSWCModule::TurnCamera -> 0x007f3ea0  gate_class=REVIEW
- GOG-order neighbour after:  0x0046b6d0 CSWCModule::ZoomCamera -> 0x007f44b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: yes
  0x0040dd20 CClientExoApp::GetCombatMode -> Steam 0x00740860  in candidate callees: yes
  0x00574940 CSWBehaviorCameraCombat::SetTurn -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00438930 CClientExoAppInternal::UpdateCamera -> Steam 0x0078c350  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 12 common): GOG {0x4*,0xc*,0x14*,0x1c*,0x28*,0x2c*,0x40*,0x80*,0x98*,0x100*,0x10c*,0x460*} | Steam {0x4*,0xc*,0x14*,0x1c*,0x28*,0x2c*,0x40*,0x80*,0x98*,0x100*,0x10c*,0x460*}
imm sets (* = in both, 11 common): GOG {0x3*,0x5*,0x7*,0x8*,0x41*,0x44*,0x48*,0x106a*,0x106e*,0x1070*,0x1071*} | Steam {0x3*,0x5*,0x7*,0x8*,0x41*,0x44*,0x48*,0x106a*,0x106e*,0x1070*,0x1071*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007f4220: 16-byte aligned: YES
  007f4220  55             PUSH EBP
  007f4221  8B EC          MOV EBP,ESP
  007f4223  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f4220
string literals (0):
direct callees (3):
  0x00740860  CClientExoApp::GetCombatMode
  0x008E5480  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
struct offsets touched (top 10):
  +0x40 [dec 64]  x14  via EAX,ECX,EDX
  +0x80 [dec 128]  x7  via EAX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x100 [dec 256]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f4220 FUN_007f4220 size=643 ret=8 callers=2 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007f4220(int param_1,float param_2)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = FUN_00740860();
  if (iVar1 == 0) {
    if (*(int *)(param_1 + 0xc) == 3) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106a);
      if (iVar1 != 0) {
        *(undefined4 *)(iVar1 + 0x100) = 0;
        *(float *)(iVar1 + 0x10c) = param_2;
      }
    }
    else if (*(int *)(param_1 + 0xc) == 5) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106e);
      if ((iVar1 != 0) && (iVar1 = FUN_0073f450(), iVar1 != 0)) {
        *(float *)(iVar1 + 0x460) = -param_2;
      }
    }
    else if (*(int *)(param_1 + 0xc) == 7) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x1071);
      if (iVar1 != 0) {
        *(float *)(iVar1 + 0x14) = -param_2 * (float)_DAT_00991358;
      }
    }
    else {
      if (((*(int *)(param_1 + 0x40) != 0) &&
          (piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0xffffffff),
          piVar2 != (int *)0x0)) && (iVar1 = (**(code **)(*piVar2 + 0x1c))(), iVar1 != 0)) {
        *(float *)(iVar1 + 0x40) = *(float *)(iVar1 + 0x40) + param_2;
        *(undefined4 *)(param_1 + 0x98) = *(undefined4 *)(iVar1 + 0x40);
      }
      if (param_2 != (float)_DAT_00986b38) {
        *(undefined4 *)(param_1 + 0x100) = 1;
        *(uint *)(param_1 + 0x80) = *(uint *)(param_1 + 0x80) | 1;
      }
    }
  }
  else {
    iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x1070);
    if (iVar1 != 0) {
      FUN_008e5480(0);
      *(float *)(iVar1 + 0x2c) = -param_2;
    }
  }
  return;
}
```

### decoy: 0x007f3ea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f3ea0: 16-byte aligned: YES
  007f3ea0  55             PUSH EBP
  007f3ea1  8B EC          MOV EBP,ESP
  007f3ea3  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f3ea0
string literals (0):
direct callees (11):
  0x00740860  CClientExoApp::GetCombatMode
  0x008E5480  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x00515E60  (unnamed)
  0x00515A70  (unnamed)
  0x00515990  (unnamed)
  0x005158B0  (unnamed)
  0x00515620  (unnamed)
  0x00463120  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00543F10  CSWSObject::SetOrientation
struct offsets touched (top 10):
  +0x40 [dec 64]  x14  via EAX,ECX,EDX
  +0x80 [dec 128]  x7  via EAX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x100 [dec 256]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via ECX
```

### Static facts (script output, report-only)
- size 643 B, RET 8
- callers 2: 0x0078c350 CClientExoAppInternal::UpdateCamera, 0x0078cea0
- callees: 0x00740860 CClientExoApp::GetCombatMode (CALL), 0x008e5480 (CALL), 0x0073f450 CClientExoApp::GetPlayerCreature (CALL)
- indirect: 0x007f4259 slot 0x80, 0x007f42a6 slot 0x80, 0x007f42f3 slot 0x80, 0x007f43ea slot 0x80, 0x007f442c slot 0x80, ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B17-2-19

GOG function: CExoFileInternal::GetSize  gog 0x006100a0  callconv=(blank)  param_size_bytes=0  gog_size=133  gog_retn=0
Candidate Steam address: 0x0073db70 (size 37, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073dba0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00610070 CExoFileInternal::GetOffset  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00610130 CExoFileInternal::Read -> 0x0073dc30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00922455 unnamed -> Steam ?  in candidate callees: n/a
  0x009227a7 unnamed -> Steam ?  in candidate callees: n/a
  0x00928827 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2,0x4*,0xc} | Steam {0x4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0073db70: 16-byte aligned: YES
  0073db70  55             PUSH EBP
  0073db71  8B EC          MOV EBP,ESP
  0073db73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073db70
string literals (0):
direct callees (1):
  0x009205FC  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0073db70 FUN_0073db70 size=37 ret=0 callers=1 callees=1

long __fastcall FUN_0073db70(int *param_1)

{
  long lVar1;
  
  if (*param_1 == 0) {
    lVar1 = 0;
  }
  else {
    lVar1 = _ftell [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1);
  }
  return lVar1;
}
```

### decoy: 0x0073dba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073dba0: 16-byte aligned: YES
  0073dba0  55             PUSH EBP
  0073dba1  8B EC          MOV EBP,ESP
  0073dba3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073dba0
string literals (0):
direct callees (3):
  0x0091AB77  (unnamed)
  0x009205FC  (unnamed)
  0x0091A825  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 37 B, RET 0
- callers 1: 0x00739950
- callees: 0x009205fc (CALL)
- twins: none

## Card B17-2-20

GOG function: PartSkin::Draw  gog 0x00882da0  callconv=__thiscall  param_size_bytes=1  gog_size=229  gog_retn=4
Candidate Steam address: 0x004cd470 (size 96, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004cd380:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00882d60 PartSkin::DrawBumpedOutShell  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00882ef0 List_SkinVertexWeight_::Add  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0086b250 Global::AurNV2xLevelShadersAvailable -> Steam ?  in candidate callees: n/a
  0x00877a20 Global::RenderSkinned -> Steam ?  in candidate callees: n/a
  0x00878e50 Global::RenderSkinnedVP -> Steam ?  in candidate callees: n/a
  0x0087f060 Global::RenderSkinnedVPBlur -> Steam ?  in candidate callees: n/a
  0x00882d60 PartSkin::DrawBumpedOutShell -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x40,0x70,0x148} | Steam {0x4}
imm sets (* = in both, 2 common): GOG {0x2,0x4*,0x8*,0x44} | Steam {0x4*,0x8*,0xd,0x34}

### abi
```
GOG param bytes: 1; callconv: thiscall
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004cd470: 16-byte aligned: YES
  004cd470  55             PUSH EBP
  004cd471  8B EC          MOV EBP,ESP
  004cd473  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004cd470
string literals (0):
direct callees (1):
  0x0048F5A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004cd470 FUN_004cd470 size=96 ret=4 callers=1 callees=1

int __thiscall FUN_004cd470(int param_1,int *param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 auStack_48[13 = byte +0x34];
  int local_8;
  
  *(undefined4 *)(param_1 + 4) = 0;
  for (local_8 = 0; local_8 < param_2[1 = byte +0x4]; local_8 = local_8 + 1) {
    puVar2 = (undefined4 *)(*param_2 + local_8 * 0x34);
    puVar3 = auStack_48;
    for (iVar1 = 0xd; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar3 = *puVar2;
      puVar2 = puVar2 + 0x1 [dec 1] {undefined4*: byte +0x4};
      puVar3 = puVar3 + 0x1 [dec 1] {undefined4*: byte +0x4};
    }
    FUN_0048f5a0();
  }
  return param_1;
}
```

### decoy: 0x004cd380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004cd380: 16-byte aligned: YES
  004cd380  55             PUSH EBP
  004cd381  8B EC          MOV EBP,ESP
  004cd383  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004cd380
string literals (0):
direct callees (5):
  0x004844C0  (unnamed)
  0x004C3430  (unnamed)
  0x004C9640  (unnamed)
  0x004C2000  (unnamed)
  0x004CD340  (unnamed)
struct offsets touched (top 5):
  +0x70 [dec 112]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EDX
  +0x40 [dec 64]  x2  via ECX,EDX
  +0x148 [dec 328]  x1  via EAX
  +0x988658 [dec 9995864]  x1  via ECX
```

### Static facts (script output, report-only)
- size 96 B, RET 4
- callers 1: 0x004c09e0
- callees: 0x0048f5a0 (CALL)
- twins: none

## Card B17-2-21

GOG function: CSWCAnimBase::GetModel  gog 0x004c7a80  callconv=(blank)  param_size_bytes=1  gog_size=37  gog_retn=4
Candidate Steam address: 0x0085fbb0 (size 37, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0085fb70:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004c7a40 CSWCAnimBase::AttachToScene  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004c7ab0 CSWCAnimBase::LoadModel -> 0x0085fbe0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xb8*} | Steam {0xb8*}
imm sets (* = in both, 2 common): GOG {0x4*,0xff*} | Steam {0x4*,0xff*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0085fbb0: 16-byte aligned: YES
  0085fbb0  55             PUSH EBP
  0085fbb1  8B EC          MOV EBP,ESP
  0085fbb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0085fbb0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xB8 [dec 184]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0085fbb0 FUN_0085fbb0 size=37 ret=4 callers=1 callees=0

undefined4 __thiscall FUN_0085fbb0(int param_1,char param_2)

{
  undefined4 uVar1;
  
  if (param_2 == -1) {
    uVar1 = *(undefined4 *)(param_1 + 0xb8);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x0085fb70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0085fb70: 16-byte aligned: YES
  0085fb70  55             PUSH EBP
  0085fb71  8B EC          MOV EBP,ESP
  0085fb73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0085fb70
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xB8 [dec 184]  x3  via EAX,ECX,EDX
  +0x4C [dec 76]  x1  via EDX
```

### Static facts (script output, report-only)
- size 37 B, RET 4
- callers 1: 0x0086c210
- twins: none

## Card B17-2-22

GOG function: CSWGuiPartySelection::OnToggled  gog 0x0058c660  callconv=(blank)  param_size_bytes=4  gog_size=567  gog_retn=4
Candidate Steam address: 0x0089ed90 (size 602, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.41) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.30 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0089eb50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0058c530 CSWGuiPartySelection::OnPanelRemoved  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0058c8a0 CSWGuiPartySelection::OnDone  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0052d090 CSWGuiMessageBox::SetAllowCancel -> Steam ?  in candidate callees: n/a
  0x0052e030 CSWGuiMessageBox::SetCallback -> Steam ?  in candidate callees: n/a
  0x0058a930 CSWGuiPartySelectionButton::SetSelected -> Steam ?  in candidate callees: n/a
  0x0058aa20 CSWGuiPartySelectionButton::GetSelected -> Steam ?  in candidate callees: n/a
  0x0058d520 CSWGuiPartySelection::UpdateCount -> Steam ?  in candidate callees: n/a
  0x0058e370 CSWGuiPartySelection::SetAddRemoveLabel -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x1c*,0x4c*,0x50*,0x58,0x5c,0x6c,0x70*,0xa0*,0x468,0x48f8,0x50ac*,0x50b0} | Steam {0x4*,0x1c*,0x30,0x4c*,0x50*,0x70*,0x74,0x88,0xa0*,0x4ec,0x4f0,0x50ac*}
imm sets (* = in both, 3 common): GOG {0x2*,0x4*,0x2d*,0x3c,0x4728,0xa588,0xa5a6,0x1a789} | Steam {0x2*,0x4*,0xc,0x27,0x2d*,0x38,0x478,0x95b8,0x95b9}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0089ed90: 16-byte aligned: YES
  0089ed90  55             PUSH EBP
  0089ed91  8B EC          MOV EBP,ESP
  0089ed93  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0089ed90
string literals (0):
direct callees (10):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0089CF10  (unnamed)
  0x0089FBD0  (unnamed)
  0x0073FEE0  CClientExoApp::SetInputClass
  0x0073F750  CClientExoApp::GetInGameGui
  0x0075BAA0  (unnamed)
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x0089CE20  (unnamed)
struct offsets touched (top 9):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x1C [dec 28]  x2  via EAX
  +0x4C [dec 76]  x2  via ECX,EDX
  +0x70 [dec 112]  x2  via EAX,EDX
  +0x30 [dec 48]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
  +0x74 [dec 116]  x1  via EDX
  +0xA0 [dec 160]  x1  via ECX
  +0x50AC [dec 20652]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0089ed90 FUN_0089ed90 size=602 ret=4 callers=1 callees=10

void __thiscall FUN_0089ed90(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  int local_14;
  char local_b;
  byte local_a;
  byte local_9;
  int local_8;
  
  if ((param_2 == 0) || (*(int *)(param_2 + 0x50) != 0)) {
    if ((param_2 != 0) && ((*(int *)(param_2 + 0x4c) == 0x2d || (*(int *)(param_2 + 0x4c) == 0x27)))
       ) {
      FUN_004122a0(0);
    }
    if (*(int *)(param_1 + 0x74) == 0) {
      if (*(int *)(param_1 + 0x50ac) == 0) {
        for (local_14 = 0; local_14 < 0xc; local_14 = local_14 + 1) {
          FUN_0089ce20(*(int *)(param_1 + 0x4f0 + local_14 * 0x478) != -1);
        }
      }
      FUN_0089fbd0(1);
    }
    else {
      local_9 = 0;
      local_b = '\0';
      local_a = 0;
      for (local_8 = 0; local_8 < 0xc; local_8 = local_8 + 1) {
        if ((*(uint *)(param_1 + 0x4ec + local_8 * 0x478) & 1) != 0) {
          local_a = local_a + 1;
        }
        if ((*(uint *)(param_1 + 0x4ec + local_8 * 0x478) >> 2 & 1) != 0) {
          local_b = local_b + '\x01';
        }
        iVar2 = FUN_0089cf10();
        if (iVar2 != 0) {
          local_9 = local_9 + 1;
        }
      }
      if ((local_a == 0) || (local_b == '\x02')) {
        FUN_0089fbd0(0);
        FUN_0073fee0(0,1);
        iVar2 = FUN_0073f750();
        *(undefined4 *)(iVar2 + 0x30) = 0;
        FUN_0075baa0(1);
      }
      else if (local_a != 0) {
        uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 4);
        iVar2 = FUN_0073f750();
        piVar1 = *(int **)(iVar2 + 0xa0);
        FUN_0075bb00(1);
        if ((local_a < 2) || (1 < local_9)) {
          (**(code **)(*piVar1 + 0x70))(0x95b8,param_1,iVar2,uVar3);
        }
        else {
          (**(code **)(*piVar1 + 0x70))(0x95b9);
        }
        FUN_0075ca30(param_1,FUN_008a07d0,0);
        FUN_00410530(piVar1,1,1);
      }
    }
  }
  return;
}
```

### decoy: 0x0089eb50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0089eb50: 16-byte aligned: YES
  0089eb50  55             PUSH EBP
  0089eb51  8B EC          MOV EBP,ESP
  0089eb53  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0089eb50
string literals (0):
direct callees (9):
  0x0073F750  CClientExoApp::GetInGameGui
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0089CF10  (unnamed)
  0x0089CE20  (unnamed)
  0x008A0860  (unnamed)
  0x0089FA10  (unnamed)
struct offsets touched (top 10):
  +0x6C [dec 108]  x5  via EAX,ECX,EDX
  +0x70 [dec 112]  x4  via EDX
  +0x1C [dec 28]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xA0 [dec 160]  x2  via EAX,EDX
  +0x468 [dec 1128]  x2  via ECX,EDX
  +0x48F8 [dec 18680]  x2  via EAX,EDX
  +0x4C [dec 76]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
  +0x58 [dec 88]  x1  via EAX
```

### Static facts (script output, report-only)
- size 602 B, RET 4
- callers 1: 0x0089de70
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x0089cf10 (CALL), 0x0089fbd0 (CALL), 0x0073fee0 CClientExoApp::SetInputClass (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x0075baa0 (CALL), ...
- indirect: 0x0089ef44 slot 0x70, 0x0089ef58 slot 0x70
- twins: none
