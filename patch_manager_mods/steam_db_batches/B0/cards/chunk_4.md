# Review cards B0 chunk 4

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-4-01

GOG function: CVirtualMachine::Destructor  gog 0x006409d0  callconv=(blank)  param_size_bytes=0  gog_size=137  gog_retn=0
Candidate Steam address: 0x006fd7b0 (size 137, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006fd700:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640920 CVirtualMachine::Constructor -> 0x006fd700  gate_class=REVIEW
- GOG-order neighbour after:  0x00640a60 CVirtualMachine::SetScriptParams -> 0x006fd840  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009217f0 Global::`eh_vector_destructor_iterator' -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
imm sets (* = in both, 3 common): GOG {0x8*,0x10*,0x14*} | Steam {0x8*,0x10*,0x14*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006fd7b0: 16-byte aligned: YES
  006fd7b0  55             PUSH EBP
  006fd7b1  8B EC          MOV EBP,ESP
  006fd7b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006fd7b0
string literals (0):
direct callees (1):
  0x00919BCC  `eh vector destructor iterator` (CRT)
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006fd7b0 FUN_006fd7b0 size=137 ret=0 callers=1 callees=1

void __fastcall FUN_006fd7b0(int param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0095ee26;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 0;
  if (*(undefined4 **)(param_1 + 0x1c) != (undefined4 *)0x0) {
    (**(code **)**(undefined4 **)(param_1 + 0x1c))(1,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  }
  local_8 = 0xffffffff;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]((void *)(param_1 + 0x14),8,1,FUN_00733780);
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x006fd700 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fd700: 16-byte aligned: YES
  006fd700  55             PUSH EBP
  006fd701  8B EC          MOV EBP,ESP
  006fd703  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006fd700
string literals (0):
direct callees (3):
  0x0091D67E  (unnamed)
  0x00919723  operator new
  0x006FF570  CVirtualMachineInternal::Constructor
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EDX
```

## Card B0-4-02

GOG function: CVirtualMachine::SetCommandImplementer  gog 0x00640ba0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fd980 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fd9a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640b80 CVirtualMachine::GetRunScriptReturnValue -> 0x006fd960  gate_class=REVIEW
- GOG-order neighbour after:  0x00640bc0 CVirtualMachine::StackPopInteger -> 0x006fd9a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00646ab0 CVirtualMachineInternal::SetCommandImplementer -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fd980: 16-byte aligned: YES
  006fd980  55             PUSH EBP
  006fd981  8B EC          MOV EBP,ESP
  006fd983  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fd980
string literals (0):
direct callees (1):
  0x00703810  CVirtualMachineInternal::SetCommandImplementer
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fd980 FUN_006fd980 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00703810 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fd980(undefined4 param_1)

{
  FUN_00703810(param_1);
  return;
}
```

### decoy: 0x006fd9a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fd9a0: 16-byte aligned: YES
  006fd9a0  55             PUSH EBP
  006fd9a1  8B EC          MOV EBP,ESP
  006fd9a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fd9a0
string literals (0):
direct callees (1):
  0x00702B60  CVirtualMachineInternal::StackPopInteger
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-4-03

GOG function: CResGFF::ReadFieldWORD  gog 0x00624c60  callconv=(blank)  param_size_bytes=14  gog_size=125  gog_retn=16
Candidate Steam address: 0x00718b00 (size 125, retn 16)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00718a80:STR0/CALLEE1/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00624be0 CResGFF::ReadFieldCHAR -> 0x00718a80  gate_class=REVIEW
- GOG-order neighbour after:  0x00624ce0 CResGFF::ReadFieldSHORT -> 0x00718b80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x00623a40 CResGFF::GetFieldByLabel -> Steam 0x007178e0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x2*,0xc*,0x10*} | Steam {0x2*,0xc*,0x10*}

### abi
```
GOG param bytes: 14; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(14/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 14 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x00718b00: 16-byte aligned: YES
  00718b00  55             PUSH EBP
  00718b01  8B EC          MOV EBP,ESP
  00718b03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00718b00
string literals (0):
direct callees (2):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00718b00 FUN_00718b00 size=125 ret=16 callers=24 callees=2

undefined2 FUN_00718b00(int param_1,int param_2,undefined4 *param_3,undefined2 param_4)

{
  undefined4 uVar1;
  int *piVar2;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 2)) {
      *param_3 = 0;
    }
    else {
      *param_3 = 1;
      param_4 = (undefined2)piVar2[2 = byte +0x8];
    }
  }
  return param_4;
}
```

### decoy: 0x00718a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00718a80: 16-byte aligned: YES
  00718a80  55             PUSH EBP
  00718a81  8B EC          MOV EBP,ESP
  00718a83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00718a80
string literals (0):
direct callees (2):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-4-04

GOG function: CSWCArea::AttachmentFromServerArea  gog 0x00457880  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x007aaec0 (size 372, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCArea::AddCorpse
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007aaea0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00457800 CSWCArea::GetServerArea -> 0x007aae00  gate_class=AUTO
- GOG-order neighbour after:  0x004578a0 CSWCArea::DetachFromServerArea -> 0x007aaea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x280} | Steam {0x4,0xe4,0x26c,0x27c,0x27d,0x3f4,0x49c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x1c,0x3e8,0xafc8,0x7f000000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007aaec0: 16-byte aligned: YES
  007aaec0  55             PUSH EBP
  007aaec1  8B EC          MOV EBP,ESP
  007aaec3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007aaec0
string literals (0):
direct callees (5):
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0073F5D0  CClientExoApp::GetPlaceableByGameObjectID
  0x00842D00  CSWCPlaceable::GetServerPlaceable
  0x007F1650  CSWCObject::SetDesiredFadeState
  0x007F18C0  (unnamed)
struct offsets touched (top 6):
  +0x27C [dec 636]  x7  via EAX,ECX,EDX
  +0x27D [dec 637]  x6  via EAX,ECX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0xE4 [dec 228]  x1  via EDX
  +0x3F4 [dec 1012]  x1  via EDX
  +0x49C [dec 1180]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007aaec0 FUN_007aaec0 size=372 ret=4 callers=1 callees=5

void __thiscall FUN_007aaec0(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  
  *(undefined4 *)(param_1 + 0x26c + (uint)*(byte *)(param_1 + 0x27d) * 4) = param_2;
  *(char *)(param_1 + 0x27d) = *(char *)(param_1 + 0x27d) + '\x01';
  if (3 < *(byte *)(param_1 + 0x27d)) {
    *(undefined1 *)(param_1 + 0x27d) = 0;
  }
  if (*(char *)(param_1 + 0x27c) == *(char *)(param_1 + 0x27d)) {
    uVar5 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    iVar1 = FUN_0073f550(*(undefined4 *)(param_1 + 0x26c + (uint)*(byte *)(param_1 + 0x27c) * 4));
    if (iVar1 != 0) {
      uVar4 = *(undefined4 *)(iVar1 + 0x3f4);
      uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 4);
      iVar2 = FUN_0073f5d0(uVar4);
      if (iVar2 != 0) {
        iVar2 = FUN_00842d00(param_1,uVar3,uVar4,uVar5);
        if (iVar2 != 0) {
          *(undefined4 *)(iVar2 + 0x49c) = 1;
        }
        FUN_007f1650(1,0,0,0);
      }
      FUN_007f18c0(1);
      FUN_007f1650(0,1,45000,1000);
      *(undefined4 *)(iVar1 + 0xe4) = 1;
    }
    *(undefined4 *)(param_1 + 0x26c + (uint)*(byte *)(param_1 + 0x27c) * 4) = 0x7f000000;
    *(char *)(param_1 + 0x27c) = *(char *)(param_1 + 0x27c) + '\x01';
    if (3 < *(byte *)(param_1 + 0x27c)) {
      *(undefined1 *)(param_1 + 0x27c) = 0;
    }
  }
  return;
}
```

### decoy: 0x007aaea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007aaea0: 16-byte aligned: YES
  007aaea0  55             PUSH EBP
  007aaea1  8B EC          MOV EBP,ESP
  007aaea3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007aaea0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x280 [dec 640]  x1  via EAX
```

## Card B0-4-05

GOG function: CExoResMan::UpdateKeyTable  gog 0x0061de00  callconv=(blank)  param_size_bytes=8  gog_size=198  gog_retn=8
Candidate Steam address: 0x007144a0 (size 198, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00714630:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061dc60 CExoResMan::Update -> 0x00714300  gate_class=AUTO
- GOG-order neighbour after:  0x0061dee0 CExoResMan::ReadRaw -> 0x00714580  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00606e70 CExoString::CompareNoCase -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x006304a0 CExoKeyTable::RebuildTable -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 7 common): GOG {0x3*,0x8*,0x10*,0x14*,0x18*,0x1c*,0x20*} | Steam {0x3*,0x8*,0x10*,0x14*,0x18*,0x1c*,0x20*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007144a0: 16-byte aligned: YES
  007144a0  55             PUSH EBP
  007144a1  8B EC          MOV EBP,ESP
  007144a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007144a0
string literals (0):
direct callees (5):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00734D60  CExoString::CompareNoCase
  0x00726A50  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007144a0 FUN_007144a0 size=198 ret=8 callers=1 callees=5

undefined4 __thiscall FUN_007144a0(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  int local_10;
  int local_c;
  int local_8;
  
  local_c = 0;
  switch(param_3) {
  case 1:
    local_8 = param_1 + 0x10;
    break;
  case 2:
    local_8 = param_1 + 0x14;
    break;
  case 3:
    local_8 = param_1 + 0x18;
    break;
  case 4:
    local_8 = param_1 + 0x1c;
    break;
  default:
    return 0;
  }
  local_10 = FUN_007a1720();
  if (local_10 != 0) {
    local_c = FUN_00561430(local_10);
  }
  while( true ) {
    if (local_10 == 0) {
      return 0;
    }
    if ((local_c != 0) && (iVar1 = FUN_00734d60(local_c + 0x20), iVar1 != 0)) break;
    local_c = FUN_0058c370(&local_10);
  }
  FUN_00726a50();
  return 1;
}
```

### decoy: 0x00714630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00714630: 16-byte aligned: YES
  00714630  55             PUSH EBP
  00714631  8B EC          MOV EBP,ESP
  00714633  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00714630
string literals (0):
direct callees (15):
  0x00733540  CExoString::CExoString(empty)
  0x00733780  CExoString::~CExoString
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00710930  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00919723  operator new
  0x007397F0  (unnamed)
  0x007398E0  (unnamed)
  0x00739880  (unnamed)
  0x009196FD  operator delete
  0x00739990  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 5):
  +0x14 [dec 20]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1A [dec 26]  x1  via EAX
  +0x28 [dec 40]  x1  via EDX
```

## Card B0-4-06

GOG function: CResGFF::GetTopLevelStruct  gog 0x00624b30  callconv=__thiscall  param_size_bytes=4  gog_size=37  gog_retn=4
Candidate Steam address: 0x007189d0 (size 37, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (LoadModuleFinish 0x55a650)
- best decoy (neighbour/other candidate) scores: 0x007186e0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00624ac0 CResGFF::GetStructFromStruct -> 0x00718960  gate_class=AUTO
- GOG-order neighbour after:  0x00624b60 CResGFF::ReadFieldBYTE -> 0x00718a00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007189d0: 16-byte aligned: YES
  007189d0  55             PUSH EBP
  007189d1  8B EC          MOV EBP,ESP
  007189d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007189d0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007189d0 FUN_007189d0 size=37 ret=4 callers=31 callees=0

bool FUN_007189d0(undefined4 *param_1)

{
  if (param_1 != (undefined4 *)0x0) {
    *param_1 = 0;
  }
  return param_1 != (undefined4 *)0x0;
}
```

### decoy: 0x007186e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007186e0: 16-byte aligned: YES
  007186e0  55             PUSH EBP
  007186e1  8B EC          MOV EBP,ESP
  007186e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007186e0
string literals (0):
direct callees (1):
  0x00717720  CResGFF::GetStruct
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B0-4-07

GOG function: CExoResMan::NukeDirectory  gog 0x0061f070  callconv=(blank)  param_size_bytes=12  gog_size=37  gog_retn=12
Candidate Steam address: 0x00715710 (size 37, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007156e0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061f040 CExoResMan::CleanDirectory -> 0x007156e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0061f0a0 CExoResMan::FileCopy -> 0x00715740  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061e990 CExoResMan::WipeDirectory -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x00715710: 16-byte aligned: YES
  00715710  55             PUSH EBP
  00715711  8B EC          MOV EBP,ESP
  00715713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00715710
string literals (0):
direct callees (1):
  0x00715030  CExoResMan::WipeDirectory
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00715710 FUN_00715710 size=37 ret=12 callers=6 callees=1
THUNK -> 0x00715030 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00715710(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_00715030(param_1,param_2,1,0,param_3);
  return;
}
```

### decoy: 0x007156e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007156e0: 16-byte aligned: YES
  007156e0  55             PUSH EBP
  007156e1  8B EC          MOV EBP,ESP
  007156e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007156e0
string literals (0):
direct callees (1):
  0x00715030  CExoResMan::WipeDirectory
struct offsets touched (top 0):
```

## Card B0-4-08

GOG function: CExoResMan::GetIsStaticType  gog 0x0061c7f0  callconv=(blank)  param_size_bytes=2  gog_size=86  gog_retn=4
Candidate Steam address: 0x00712e90 (size 86, retn 4)
Candidate sources: branch+name_map:high:gog_cexoresman_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00712e50:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061c7b0 CExoResMan::GetTableCount -> 0x00712e50  gate_class=REVIEW
- GOG-order neighbour after:  0x0061c890 CExoResMan::Malloc -> 0x00712f30  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x31*,0x7d9*,0xbbb*} | Steam {0x4*,0x8*,0x31*,0x7d9*,0xbbb*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00712e90: 16-byte aligned: YES
  00712e90  55             PUSH EBP
  00712e91  8B EC          MOV EBP,ESP
  00712e93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00712e90
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x712EF0 [dec 7417584]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00712e90 FUN_00712e90 size=86 ret=4 callers=1 callees=0

undefined4 FUN_00712e90(ushort param_1)

{
  undefined4 uVar1;
  
  if (param_1 < 0xbbc) {
    if (param_1 != 0xbbb) {
      switch(param_1) {
      case 0x7d9:
      case 0x7da:
      case 0x7dc:
      case 0x7e9:
      case 0x7eb:
      case 0x7ed:
      case 0x7ee:
      case 0x7f0:
      case 0x7f3:
      case 0x7f8:
      case 0x7fa:
      case 0x7fc:
      case 0x7fe:
      case 0x803:
      case 0x808:
      case 0x80a:
        break;
      default:
        goto switchD_00712ece_caseD_7db;
      }
    }
    uVar1 = 1;
  }
  else {
switchD_00712ece_caseD_7db:
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00712e50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00712e50: 16-byte aligned: YES
  00712e50  55             PUSH EBP
  00712e51  8B EC          MOV EBP,ESP
  00712e53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00712e50
string literals (0):
direct callees (2):
  0x00712D70  CExoResMan::GetTable
  0x00726050  (unnamed)
struct offsets touched (top 0):
```

## Card B0-4-09

GOG function: CResGFF::GetList  gog 0x006248c0  callconv=(blank)  param_size_bytes=12  gog_size=164  gog_retn=12
Candidate Steam address: 0x00718760 (size 164, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00718810:STR0/CALLEE2/SIG0.3/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00624870 CResGFF::GetFieldType -> 0x00718710  gate_class=REVIEW
- GOG-order neighbour after:  0x00624970 CResGFF::GetListCount -> 0x00718810  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x006239e0 CResGFF::GetDataLayoutList -> Steam 0x00717880  in candidate callees: yes
  0x00623a40 CResGFF::GetFieldByLabel -> Steam 0x007178e0  in candidate callees: yes
  0x00924d20 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0xc*,0xf*,0x10*,0x14*} | Steam {0x4*,0xc*,0xf*,0x10*,0x14*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00718760: 16-byte aligned: YES
  00718760  55             PUSH EBP
  00718761  8B EC          MOV EBP,ESP
  00718763  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00718760
string literals (0):
direct callees (4):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717880  CResGFF::GetDataLayoutList
  0x0091D0F0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00718760 FUN_00718760 size=164 ret=12 callers=56 callees=4

undefined4 FUN_00718760(undefined4 *param_1,undefined4 *param_2,char *param_3)

{
  undefined4 uVar1;
  uint local_10;
  int local_c;
  int *local_8;
  
  if (((param_1 == (undefined4 *)0x0) || (param_2 == (undefined4 *)0x0)) || (param_3 == (char *)0x0)
     ) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_2,param_3);
    local_8 = (int *)FUN_00717770(param_2,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 0xf)) {
      uVar1 = 0;
    }
    else {
      local_c = FUN_00717880(local_8,&local_10);
      if ((local_c == 0) || (local_10 < 4)) {
        uVar1 = 0;
      }
      else {
        *param_1 = *param_2;
        _strncpy [GHIDRA-AUTO-LABEL: unreliable]((char *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}),param_3,0x10);
        uVar1 = 1;
      }
    }
  }
  return uVar1;
}
```

### decoy: 0x00718810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00718810: 16-byte aligned: YES
  00718810  55             PUSH EBP
  00718811  8B EC          MOV EBP,ESP
  00718813  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00718810
string literals (0):
direct callees (3):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717880  CResGFF::GetDataLayoutList
struct offsets touched (top 0):
```

## Card B0-4-10

GOG function: CVirtualMachine::StackPopString  gog 0x00640c90  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fda70 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fda90:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640c60 CVirtualMachine::StackPushVector -> 0x006fda40  gate_class=AUTO
- GOG-order neighbour after:  0x00640cb0 CVirtualMachine::StackPushString -> 0x006fda90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00646070 CVirtualMachineInternal::StackPopString -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fda70: 16-byte aligned: YES
  006fda70  55             PUSH EBP
  006fda71  8B EC          MOV EBP,ESP
  006fda73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fda70
string literals (0):
direct callees (1):
  0x00702DD0  CVirtualMachineInternal::StackPopString
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fda70 FUN_006fda70 size=28 ret=4 callers=64 callees=1
THUNK -> 0x00702DD0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fda70(undefined4 param_1)

{
  FUN_00702dd0(param_1);
  return;
}
```

### decoy: 0x006fda90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fda90: 16-byte aligned: YES
  006fda90  55             PUSH EBP
  006fda91  8B EC          MOV EBP,ESP
  006fda93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fda90
string literals (0):
direct callees (1):
  0x00702E60  CVirtualMachineInternal::StackPushString
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-4-11

GOG function: CResGFF::OnResourceFreed  gog 0x00623710  callconv=(blank)  param_size_bytes=0  gog_size=124  gog_retn=0
Candidate Steam address: 0x007175b0 (size 124, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00717630:STR0/CALLEE0/SIG0.222/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00623460 CResGFF::OnResourceServiced -> 0x00717300  gate_class=AUTO
- GOG-order neighbour after:  0x00623790 CResGFF::ReleaseResource -> 0x00717630  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0x40*,0x44*,0x4c*,0x54*,0x5c*,0x64*,0x70*,0x94*,0x98*} | Steam {0x40*,0x44*,0x4c*,0x54*,0x5c*,0x64*,0x70*,0x94*,0x98*}
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
prologue for 0x007175b0: 16-byte aligned: YES
  007175b0  55             PUSH EBP
  007175b1  8B EC          MOV EBP,ESP
  007175b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007175b0
string literals (0):
direct callees (0):
struct offsets touched (top 9):
  +0x98 [dec 152]  x2  via EAX,EDX
  +0x40 [dec 64]  x1  via ECX
  +0x44 [dec 68]  x1  via EAX
  +0x4C [dec 76]  x1  via EDX
  +0x54 [dec 84]  x1  via ECX
  +0x5C [dec 92]  x1  via EAX
  +0x64 [dec 100]  x1  via EDX
  +0x70 [dec 112]  x1  via ECX
  +0x94 [dec 148]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007175b0 FUN_007175b0 size=124 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_007175b0(int param_1)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    *(undefined4 *)(param_1 + 0x70) = 0;
    *(undefined4 *)(param_1 + 0x64 [dec 100]) = 0;
    *(undefined4 *)(param_1 + 0x5c) = 0;
    *(undefined4 *)(param_1 + 0x54) = 0;
    *(undefined4 *)(param_1 + 0x4c) = 0;
    *(undefined4 *)(param_1 + 0x44) = 0;
    *(undefined4 *)(param_1 + 0x40) = 0;
    *(undefined4 *)(param_1 + 0x98) = 0;
  }
  *(undefined4 *)(param_1 + 0x94) = 0;
  return 1;
}
```

### decoy: 0x00717630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00717630: 16-byte aligned: YES
  00717630  55             PUSH EBP
  00717631  8B EC          MOV EBP,ESP
  00717633  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00717630
string literals (0):
direct callees (2):
  0x007112B0  CRes::Release
  0x00713010  CExoResMan::ReleaseResObject
struct offsets touched (top 10):
  +0x98 [dec 152]  x2  via EAX,ECX
  +0x9C [dec 156]  x2  via EAX,ECX
  +0x40 [dec 64]  x1  via EDX
  +0x44 [dec 68]  x1  via EAX
  +0x48 [dec 72]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x54 [dec 84]  x1  via ECX
  +0x58 [dec 88]  x1  via EDX
  +0x5C [dec 92]  x1  via EAX
```

## Card B0-4-12

GOG function: CResGFF::WriteFieldBYTE  gog 0x00625b30  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x007199d0 (size 97, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00719a40:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625aa0 CResGFF::AddStructToStruct -> 0x00719940  gate_class=AUTO
- GOG-order neighbour after:  0x00625ba0 CResGFF::WriteFieldCHAR -> 0x00719a40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam 0x00717aa0  in candidate callees: yes
  0x00624190 CResGFF::AddField -> Steam 0x00718030  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x8*,0x98*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x007199d0: 16-byte aligned: YES
  007199d0  55             PUSH EBP
  007199d1  8B EC          MOV EBP,ESP
  007199d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007199d0
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007199d0 FUN_007199d0 size=97 ret=12 callers=35 callees=2

undefined4 __thiscall FUN_007199d0(int param_1,int param_2,byte param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,0);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(uint *)(iVar2 + 8) = (uint)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00719a40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719a40: 16-byte aligned: YES
  00719a40  55             PUSH EBP
  00719a41  8B EC          MOV EBP,ESP
  00719a43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719a40
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-4-13

GOG function: CExoResMan::ServiceFromResFile  gog 0x0061d910  callconv=(blank)  param_size_bytes=8  gog_size=843  gog_retn=8
Candidate Steam address: 0x00713fb0 (size 843, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00713bf0:STR0/CALLEE0/SIG0.873/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061d7e0 CExoResMan::ServiceFromImage -> 0x00713e80  gate_class=AUTO
- GOG-order neighbour after:  0x0061dc60 CExoResMan::Update -> 0x00714300  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x006081d0 CExoBase::GetResourceExtension -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0061a300 CResRef::GetResRefStr -> Steam ?  in candidate callees: n/a
  0x0061c890 CExoResMan::Malloc -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 14 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1a*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x38*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1a*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x38*}
imm sets (* = in both, 11 common): GOG {0x4*,0x8*,0xe*,0x10*,0x14*,0x18*,0x3fff*,0xfc000*,0xfffffff*,0x3ff00000*,0xfffffffb*} | Steam {0x4*,0x8*,0xe*,0x10*,0x14*,0x18*,0x3fff*,0xfc000*,0xfffffff*,0x3ff00000*,0xfffffffb*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00713fb0: 16-byte aligned: YES
  00713fb0  55             PUSH EBP
  00713fb1  8B EC          MOV EBP,ESP
  00713fb3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00713fb0
string literals (0):
direct callees (7):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00712F30  CExoResMan::Malloc
  0x007109B0  (unnamed)
  0x007360A0  (unnamed)
  0x00734010  CExoString::CStr
  0x0058C370  (unnamed)
struct offsets touched (top 10):
  +0x30 [dec 48]  x23  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x14 [dec 20]  x6  via ECX,EDX
  +0xC [dec 12]  x3  via EAX
  +0x10 [dec 16]  x3  via EAX,EDX
  +0x18 [dec 24]  x3  via ECX,EDX
  +0x24 [dec 36]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x1A [dec 26]  x1  via EAX
  +0x20 [dec 32]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00713fb0 FUN_00713fb0 size=843 ret=8 callers=2 callees=7

int __thiscall FUN_00713fb0(int param_1,int *param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  int local_14;
  int local_10;
  int local_c;
  uint local_8;
  
  local_10 = 0;
  if ((param_2 != (int *)0x0) && ((param_2[3 = byte +0xc] & 4U) == 0)) {
    local_8 = (param_2[2 = byte +0x8] & 0x3ff00000U) >> 0x14;
    local_14 = FUN_007a1720();
    if (local_14 != 0) {
      local_10 = FUN_00561430(local_14);
    }
    while (local_14 != 0) {
      if ((local_10 != 0) &&
         ((*(uint *)(local_10 + 0x28) & 0xfffffff) == (param_2[2 = byte +0x8] & 0xfc000U) >> 0xe)) {
        if (param_3 != 0) {
          (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 8))();
          local_c = (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x24))
                              (param_2[2 = byte +0x8] & 0x3fff);
          param_2[6 = byte +0x18] = local_c;
          iVar1 = FUN_00712f30(param_2);
          if (iVar1 != 0) {
            uVar2 = (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x20))();
            *(undefined4 *)(param_1 + 0x2c) = uVar2;
            (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x38))
                      (param_2[2 = byte +0x8],param_2[4 = byte +0x10],local_c,0);
            return 1;
          }
          (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x18))();
          return 0;
        }
        (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 4))();
        local_c = (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x24))
                            (param_2[2 = byte +0x8] & 0x3fff);
        param_2[6 = byte +0x18] = local_c;
        iVar1 = FUN_00712f30(param_2);
        if (iVar1 != 0) {
          iVar1 = (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x34))
                            (param_2[2 = byte +0x8],param_2[4 = byte +0x10],local_c,0);
          if (iVar1 == local_c) {
            (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x14))();
            iVar1 = (**(code **)(*param_2 + 0x10))();
            if (iVar1 != 0) {
              FUN_007109b0();
              FUN_007360a0(*(undefined2 *)(param_2[5 = byte +0x14] + 0x1a));
              FUN_00734010();
              FUN_00734010();
              FUN_007109b0();
            }
            param_2[3 = byte +0xc] = param_2[3 = byte +0xc] & 0xfffffffbU | -(uint)(iVar1 != 0) & 4;
            return iVar1;
          }
          (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x14))();
          return 0;
        }
        (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x14))();
        return 0;
      }
      local_10 = FUN_0058c370(&local_14);
    }
  }
  return 0;
}
```

### decoy: 0x00713bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00713bf0: 16-byte aligned: YES
  00713bf0  55             PUSH EBP
  00713bf1  8B EC          MOV EBP,ESP
  00713bf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00713bf0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00712F30  CExoResMan::Malloc
  0x0058C370  (unnamed)
struct offsets touched (top 10):
  +0x30 [dec 48]  x22  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,ECX
  +0x10 [dec 16]  x3  via EAX,ECX,EDX
  +0x14 [dec 20]  x3  via EAX,EDX
  +0x18 [dec 24]  x3  via EAX,ECX
  +0x24 [dec 36]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x20 [dec 32]  x1  via EDX
  +0x28 [dec 40]  x1  via EDX
```

## Card B0-4-14

GOG function: CVirtualMachine::StackPushFloat  gog 0x00640c20  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x006fda00 (size 31, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fd9e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640c00 CVirtualMachine::StackPopFloat -> 0x006fd9e0  gate_class=REVIEW
- GOG-order neighbour after:  0x00640c40 CVirtualMachine::StackPopVector -> 0x006fda20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00645f50 CVirtualMachineInternal::StackPushFloat -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fda00: 16-byte aligned: YES
  006fda00  55             PUSH EBP
  006fda01  8B EC          MOV EBP,ESP
  006fda03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fda00
string literals (0):
direct callees (1):
  0x00702CB0  CVirtualMachineInternal::StackPushFloat
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006fda00 FUN_006fda00 size=31 ret=4 callers=27 callees=1
THUNK -> 0x00702CB0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fda00(undefined4 param_1)

{
  FUN_00702cb0(param_1);
  return;
}
```

### decoy: 0x006fd9e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fd9e0: 16-byte aligned: YES
  006fd9e0  55             PUSH EBP
  006fd9e1  8B EC          MOV EBP,ESP
  006fd9e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fd9e0
string literals (0):
direct callees (1):
  0x00702C30  CVirtualMachineInternal::StackPopFloat
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-4-15

GOG function: CVirtualMachine::StackPushInteger  gog 0x00640be0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fd9c0 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fdaf0:STR0/CALLEE3/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640bc0 CVirtualMachine::StackPopInteger -> 0x006fd9a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00640c00 CVirtualMachine::StackPopFloat -> 0x006fd9e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00645e80 CVirtualMachineInternal::StackPushInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fd9c0: 16-byte aligned: YES
  006fd9c0  55             PUSH EBP
  006fd9c1  8B EC          MOV EBP,ESP
  006fd9c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fd9c0
string literals (0):
direct callees (1):
  0x00702BE0  CVirtualMachineInternal::StackPushInteger
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fd9c0 FUN_006fd9c0 size=28 ret=4 callers=203 callees=1
THUNK -> 0x00702BE0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fd9c0(undefined4 param_1)

{
  FUN_00702be0(param_1);
  return;
}
```

### decoy: 0x006fdaf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fdaf0: 16-byte aligned: YES
  006fdaf0  55             PUSH EBP
  006fdaf1  8B EC          MOV EBP,ESP
  006fdaf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fdaf0
string literals (0):
direct callees (1):
  0x00703070  CVirtualMachineInternal::StackPopObject
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-4-16

GOG function: CExoResMan::ServiceFromEncapsulated  gog 0x0061d550  callconv=(blank)  param_size_bytes=8  gog_size=644  gog_retn=8
Candidate Steam address: 0x00713bf0 (size 644, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00713fb0:STR0/CALLEE0/SIG0.873/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061cd00 CExoResMan::ServiceFromDirectory -> 0x007133a0  gate_class=AUTO
- GOG-order neighbour after:  0x0061d7e0 CExoResMan::ServiceFromImage -> 0x00713e80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0061c890 CExoResMan::Malloc -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x38*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x38*}
imm sets (* = in both, 9 common): GOG {0x4*,0x8*,0xe*,0x14*,0x18*,0x3fff*,0xfc000*,0xfffffff*,0xfffffffb*} | Steam {0x4*,0x8*,0xe*,0x14*,0x18*,0x3fff*,0xfc000*,0xfffffff*,0xfffffffb*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00713bf0: 16-byte aligned: YES
  00713bf0  55             PUSH EBP
  00713bf1  8B EC          MOV EBP,ESP
  00713bf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00713bf0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00712F30  CExoResMan::Malloc
  0x0058C370  (unnamed)
struct offsets touched (top 10):
  +0x30 [dec 48]  x22  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,ECX
  +0x10 [dec 16]  x3  via EAX,ECX,EDX
  +0x14 [dec 20]  x3  via EAX,EDX
  +0x18 [dec 24]  x3  via EAX,ECX
  +0x24 [dec 36]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x20 [dec 32]  x1  via EDX
  +0x28 [dec 40]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00713bf0 FUN_00713bf0 size=644 ret=8 callers=2 callees=4

int __thiscall FUN_00713bf0(int param_1,int *param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  int local_10;
  int local_c;
  int local_8;
  
  local_c = 0;
  if ((param_2 != (int *)0x0) && ((param_2[3 = byte +0xc] & 4U) == 0)) {
    local_10 = FUN_007a1720();
    if (local_10 != 0) {
      local_c = FUN_00561430(local_10);
    }
    while (local_10 != 0) {
      if ((local_c != 0) &&
         ((*(uint *)(local_c + 0x28) & 0xfffffff) == (param_2[2 = byte +0x8] & 0xfc000U) >> 0xe)) {
        if (param_3 != 0) {
          (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 8))();
          local_8 = (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x24))
                              (param_2[2 = byte +0x8] & 0x3fff);
          param_2[6 = byte +0x18] = local_8;
          iVar1 = FUN_00712f30(param_2);
          if (iVar1 != 0) {
            uVar2 = (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x20))();
            *(undefined4 *)(param_1 + 0x2c) = uVar2;
            (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x38))
                      (param_2[2 = byte +0x8],param_2[4 = byte +0x10],local_8,0);
            return 1;
          }
          (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x18))();
          return 0;
        }
        (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 4))();
        local_8 = (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x24))
                            (param_2[2 = byte +0x8] & 0x3fff);
        param_2[6 = byte +0x18] = local_8;
        iVar1 = FUN_00712f30(param_2);
        if (iVar1 != 0) {
          iVar1 = (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x34))
                            (param_2[2 = byte +0x8],param_2[4 = byte +0x10],local_8,0);
          if (iVar1 == local_8) {
            (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x14))();
            iVar1 = (**(code **)(*param_2 + 0x10))();
            param_2[3 = byte +0xc] = param_2[3 = byte +0xc] & 0xfffffffbU | -(uint)(iVar1 != 0) & 4;
            return iVar1;
          }
          (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x14))();
          return 0;
        }
        (**(code **)(*(int *)**(undefined4 **)(local_c + 0x30) + 0x14))();
        return 0;
      }
      local_c = FUN_0058c370(&local_10);
    }
  }
  return 0;
}
```

### decoy: 0x00713fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00713fb0: 16-byte aligned: YES
  00713fb0  55             PUSH EBP
  00713fb1  8B EC          MOV EBP,ESP
  00713fb3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00713fb0
string literals (0):
direct callees (7):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00712F30  CExoResMan::Malloc
  0x007109B0  (unnamed)
  0x007360A0  (unnamed)
  0x00734010  CExoString::CStr
  0x0058C370  (unnamed)
struct offsets touched (top 10):
  +0x30 [dec 48]  x23  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x14 [dec 20]  x6  via ECX,EDX
  +0xC [dec 12]  x3  via EAX
  +0x10 [dec 16]  x3  via EAX,EDX
  +0x18 [dec 24]  x3  via ECX,EDX
  +0x24 [dec 36]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x1A [dec 26]  x1  via EAX
  +0x20 [dec 32]  x1  via EDX
```

## Card B0-4-17

GOG function: CExoResMan::ServiceFromResFileRaw  gog 0x0061e5c0  callconv=(blank)  param_size_bytes=12  gog_size=370  gog_retn=12
Candidate Steam address: 0x00714c60 (size 370, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00714b30:STR0/CALLEE0/SIG0.764/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061e490 CExoResMan::ServiceFromImageRaw -> 0x00714b30  gate_class=REVIEW
- GOG-order neighbour after:  0x0061e740 CExoResMan::GetFreeDiskSpace -> 0x00714de0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0x14*,0x18*,0x24*,0x28*,0x30*,0x34*} | Steam {0x4*,0x8*,0x14*,0x18*,0x24*,0x28*,0x30*,0x34*}
imm sets (* = in both, 8 common): GOG {0xc*,0xe*,0x10*,0x14*,0x3fff*,0xfc000*,0xfffffff*,0x3ff00000*} | Steam {0xc*,0xe*,0x10*,0x14*,0x3fff*,0xfc000*,0xfffffff*,0x3ff00000*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00714c60: 16-byte aligned: YES
  00714c60  55             PUSH EBP
  00714c61  8B EC          MOV EBP,ESP
  00714c63  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00714c60
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 8):
  +0x30 [dec 48]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x24 [dec 36]  x1  via EDX
  +0x28 [dec 40]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00714c60 FUN_00714c60 size=370 ret=12 callers=1 callees=3

undefined4 __thiscall FUN_00714c60(undefined4 param_1,int param_2,int param_3,undefined4 param_4)

{
  int iVar1;
  int local_14;
  int local_10;
  int local_c;
  uint local_8;
  
  local_10 = 0;
  if (param_2 != 0) {
    local_8 = (*(uint *)(param_2 + 8) & 0x3ff00000) >> 0x14;
    local_14 = FUN_007a1720(param_1);
    if (local_14 != 0) {
      local_10 = FUN_00561430(local_14);
    }
    while (local_14 != 0) {
      if ((local_10 != 0) &&
         ((*(uint *)(local_10 + 0x28) & 0xfffffff) == (*(uint *)(param_2 + 8) & 0xfc000) >> 0xe)) {
        (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 4))();
        local_c = (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x24))
                            (*(uint *)(param_2 + 8) & 0x3fff);
        if (local_c < param_3) {
          param_3 = local_c;
        }
        *(int *)(param_2 + 0x18) = param_3;
        iVar1 = (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x34))
                          (*(undefined4 *)(param_2 + 8),param_4,param_3,0);
        if (iVar1 == param_3) {
          (**(code **)(**(int **)(*(int *)(local_10 + 0x30) + local_8 * 4) + 0x14))();
          return 1;
        }
        return 0;
      }
      local_10 = FUN_0058c370(&local_14);
    }
  }
  return 0;
}
```

### decoy: 0x00714b30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00714b30: 16-byte aligned: YES
  00714b30  55             PUSH EBP
  00714b31  8B EC          MOV EBP,ESP
  00714b33  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00714b30
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00712F30  CExoResMan::Malloc
  0x0058C370  (unnamed)
struct offsets touched (top 6):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EAX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x24 [dec 36]  x1  via EDX
  +0x28 [dec 40]  x1  via EAX
  +0x34 [dec 52]  x1  via EAX
```

## Card B0-4-18

GOG function: CResGFF::Constructor  gog 0x00622ff0  callconv=(blank)  param_size_bytes=0  gog_size=312  gog_retn=0
Candidate Steam address: 0x00716e60 (size 312, retn 0)
Candidate sources: branch+mapper:drift-0x30+name_map:high:steam-db-pr3-phase2+near    mapper tag: drift-0x30

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (CRes 0x7110d0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00716fa0:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00622b40 C2DA::Unload2DArray  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00623130 CResGFF::Constructor_2 -> 0x00716fd0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061aa20 CRes::Constructor -> Steam 0x007110d0  in candidate callees: yes
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 25 common): GOG {0x28*,0x2c*,0x30*,0x34*,0x38*,0x3c*,0x40*,0x44*,0x48*,0x4c*,0x50*,0x54*,0x58*,0x5c*,...(+11)} | Steam {0x28*,0x2c*,0x30*,0x34*,0x38*,0x3c*,0x40*,0x44*,0x48*,0x4c*,0x50*,0x54*,0x58*,0x5c*,...(+11)}
imm sets (* = in both, 8 common): GOG {0x4*,0xc*,0x62*,0x6e*,0x8d*,0x286*,0x72c*,0xfd4*} | Steam {0x4*,0xc*,0x62*,0x6e*,0x8d*,0x286*,0x72c*,0xfd4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00716e60: 16-byte aligned: YES
  00716e60  55             PUSH EBP
  00716e61  8B EC          MOV EBP,ESP
  00716e63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00716e60 FUN_00716e60 size=312 ret=0 callers=26 callees=2

undefined4 * __fastcall FUN_00716e60(undefined4 *param_1)

{
  FUN_007110d0();
  *param_1 = CResGFF::vftable;
  param_1[0x10 = byte +0x40] = 0;
  param_1[0x11 = byte +0x44] = 0;
  param_1[0x12 = byte +0x48] = 0;
  param_1[0x13 = byte +0x4c] = 0;
  param_1[0x14 = byte +0x50] = 0;
  param_1[0x15 = byte +0x54] = 0;
  param_1[0x16 = byte +0x58] = 0;
  param_1[0x17 = byte +0x5c] = 0;
  param_1[0x18 = byte +0x60] = 0;
  param_1[0x19 = byte +0x64] = 0;
  param_1[0x1a = byte +0x68] = 0;
  param_1[0x1b = byte +0x6c] = 0;
  param_1[0x1c = byte +0x70] = 0;
  param_1[0x1d = byte +0x74] = 0;
  param_1[0x1e = byte +0x78] = 0;
  param_1[10 = byte +0x28] = 0x6e;
  param_1[0xb = byte +0x2c] = 0x286;
  param_1[0xc = byte +0x30] = 0x62;
  param_1[0xd = byte +0x34] = 0x72c;
  param_1[0xe = byte +0x38] = 0xfd4;
  param_1[0xf = byte +0x3c] = 0xfd4;
  *(undefined1 *)(param_1 + 0x23) = 0;
  param_1[0x25 = byte +0x94] = 0;
  param_1[0x26 = byte +0x98] = 0;
  param_1[0x27 = byte +0x9c] = 0;
  _memset [GHIDRA-AUTO-LABEL: unreliable]((void *)((int)param_1 + 0x8d),0,4);
  return param_1;
}
```

### decoy: 0x00716fa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00716fa0: 16-byte aligned: YES
  00716fa0  55             PUSH EBP
  00716fa1  8B EC          MOV EBP,ESP
  00716fa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00716fa0
string literals (0):
direct callees (2):
  0x007171B0  CResGFF::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-4-19

GOG function: CResGFF::GetElementType  gog 0x006247f0  callconv=(blank)  param_size_bytes=4  gog_size=46  gog_retn=4
Candidate Steam address: 0x00718690 (size 46, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007186e0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006246e0 CResGFF::AddDataLayoutList -> 0x00718580  gate_class=AUTO
- GOG-order neighbour after:  0x00624820 CResGFF::GetFieldCount -> 0x007186c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00623880 CResGFF::GetStruct -> Steam ?  in candidate callees: n/a
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
prologue for 0x00718690: 16-byte aligned: YES
  00718690  55             PUSH EBP
  00718691  8B EC          MOV EBP,ESP
  00718693  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00718690
string literals (0):
direct callees (1):
  0x00717720  CResGFF::GetStruct
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00718690 FUN_00718690 size=46 ret=4 callers=14 callees=1

undefined4 FUN_00718690(undefined4 param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  
  puVar1 = (undefined4 *)FUN_00717720(param_1);
  if (puVar1 == (undefined4 *)0x0) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = *puVar1;
  }
  return uVar2;
}
```

### decoy: 0x007186e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007186e0: 16-byte aligned: YES
  007186e0  55             PUSH EBP
  007186e1  8B EC          MOV EBP,ESP
  007186e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007186e0
string literals (0):
direct callees (1):
  0x00717720  CResGFF::GetStruct
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B0-4-20

GOG function: CExoResMan::CacheResource  gog 0x00620700  callconv=(blank)  param_size_bytes=10  gog_size=59  gog_retn=12
Candidate Steam address: 0x00716da0 (size 59, retn 12)
Candidate sources: branch+name_map:high:gog_cexoresman_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00716d80:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006206e0 CExoResMan::FreeResourceData_2 -> 0x00716d80  gate_class=REVIEW
- GOG-order neighbour after:  0x00620740 CExoResMan::CacheResource_2 -> 0x00716de0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00620740 CExoResMan::CacheResource_2 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x00716da0: 16-byte aligned: YES
  00716da0  55             PUSH EBP
  00716da1  8B EC          MOV EBP,ESP
  00716da3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00716da0
string literals (0):
direct callees (1):
  0x00716DE0  CExoResMan::CacheResource_2
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00716da0 FUN_00716da0 size=59 ret=12 callers=6 callees=1

undefined4 FUN_00716da0(undefined4 param_1,undefined4 param_2,undefined2 param_3)

{
  undefined1 local_8 [4];
  
  FUN_00716de0(param_1,param_2,param_3,local_8);
  return param_1;
}
```

### decoy: 0x00716d80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00716d80: 16-byte aligned: YES
  00716d80  55             PUSH EBP
  00716d81  8B EC          MOV EBP,ESP
  00716d83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00716d80
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-4-21

GOG function: CGameEffect::Destructor  gog 0x00734960  callconv=(blank)  param_size_bytes=0  gog_size=300  gog_retn=0
Candidate Steam address: 0x005e47b0 (size 44, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4740:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734780 CGameEffect::Constructor_2 -> 0x005e4ac0  gate_class=REVIEW
- GOG-order neighbour after:  0x00734a90 CGameEffect::SetNumIntegers -> 0x005e4dd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x009217f0 Global::`eh_vector_destructor_iterator' -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x28,0x2c,0x34} | Steam {0x1,0xc,0x18}
imm sets (* = in both, 0 common): GOG {0x4,0x6,0x8,0x20,0x48} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005e47b0: 16-byte aligned: YES
  005e47b0  55             PUSH EBP
  005e47b1  8B EC          MOV EBP,ESP
  005e47b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e47b0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x2  via EAX,EDX
  +0x18 [dec 24]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005e47b0 FUN_005e47b0 size=44 ret=0 callers=2 callees=0

/* WARNING: Removing unreachable block (ram,0x005e47ca) */

undefined4 __fastcall FUN_005e47b0(int param_1)

{
  return *(undefined4 *)(param_1 + 0xc);
}
```

### decoy: 0x005e4740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4740: 16-byte aligned: YES
  005e4740  55             PUSH EBP
  005e4741  8B EC          MOV EBP,ESP
  005e4743  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4740
string literals (0):
direct callees (2):
  0x005E3FE0  (unnamed)
  0x0091DBA0  _memcpy
struct offsets touched (top 4):
  +0xC [dec 12]  x4  via EAX,EDX
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-4-22

GOG function: CResGFF::GetListCount  gog 0x00624970  callconv=(blank)  param_size_bytes=4  gog_size=124  gog_retn=4
Candidate Steam address: 0x00718810 (size 124, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00718890:STR0/CALLEE3/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006248c0 CResGFF::GetList -> 0x00718760  gate_class=REVIEW
- GOG-order neighbour after:  0x006249f0 CResGFF::GetListElement -> 0x00718890  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x006239e0 CResGFF::GetDataLayoutList -> Steam 0x00717880  in candidate callees: yes
  0x00623a40 CResGFF::GetFieldByLabel -> Steam 0x007178e0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0xf*,0x14*} | Steam {0x4*,0xf*,0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00718810: 16-byte aligned: YES
  00718810  55             PUSH EBP
  00718811  8B EC          MOV EBP,ESP
  00718813  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00718810
string literals (0):
direct callees (3):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717880  CResGFF::GetDataLayoutList
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00718810 FUN_00718810 size=124 ret=4 callers=56 callees=3

undefined4 FUN_00718810(int param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  uint local_10[2 = byte +0x8];
  int *local_8;
  
  if (param_1 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_1 + 4);
    local_8 = (int *)FUN_00717770(param_1,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 0xf)) {
      uVar1 = 0;
    }
    else {
      puVar2 = (undefined4 *)FUN_00717880(local_8,local_10);
      if ((puVar2 == (undefined4 *)0x0) || (local_10[0] < 4)) {
        uVar1 = 0;
      }
      else {
        uVar1 = *puVar2;
      }
    }
  }
  return uVar1;
}
```

### decoy: 0x00718890 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00718890: 16-byte aligned: YES
  00718890  55             PUSH EBP
  00718891  8B EC          MOV EBP,ESP
  00718893  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00718890
string literals (0):
direct callees (3):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717880  CResGFF::GetDataLayoutList
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via ECX
  +0x40 [dec 64]  x1  via EAX
```

## Card B0-4-23

GOG function: CResGFF::WriteFieldDWORD  gog 0x00625cf0  callconv=(blank)  param_size_bytes=12  gog_size=96  gog_retn=12
Candidate Steam address: 0x00719b90 (size 96, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00719b20:STR0/CALLEE2/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625c80 CResGFF::WriteFieldSHORT -> 0x00719b20  gate_class=REVIEW
- GOG-order neighbour after:  0x00625d50 CResGFF::WriteFieldINT -> 0x00719bf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam 0x00717aa0  in candidate callees: yes
  0x00624190 CResGFF::AddField -> Steam 0x00718030  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x8*,0x98*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00719b90: 16-byte aligned: YES
  00719b90  55             PUSH EBP
  00719b91  8B EC          MOV EBP,ESP
  00719b93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00719b90
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00719b90 FUN_00719b90 size=96 ret=12 callers=52 callees=2

undefined4 __thiscall FUN_00719b90(int param_1,int param_2,undefined4 param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,4);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(undefined4 *)(iVar2 + 8) = param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00719b20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719b20: 16-byte aligned: YES
  00719b20  55             PUSH EBP
  00719b21  8B EC          MOV EBP,ESP
  00719b23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719b20
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-4-24

GOG function: CVirtualMachine::Constructor  gog 0x00640920  callconv=(blank)  param_size_bytes=0  gog_size=163  gog_retn=0
Candidate Steam address: 0x006fd700 (size 163, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006fd7b0:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006408d0 CScriptCompiler::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006409d0 CVirtualMachine::Destructor -> 0x006fd7b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x006427b0 CVirtualMachineInternal::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x009258bf Global::`eh_vector_constructor_iterator' -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x10*,0x14*,0x3d8*} | Steam {0x4*,0x8*,0x10*,0x14*,0x3d8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006fd700: 16-byte aligned: YES
  006fd700  55             PUSH EBP
  006fd701  8B EC          MOV EBP,ESP
  006fd703  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006fd700
string literals (0):
direct callees (3):
  0x0091D67E  (unnamed)
  0x00919723  operator new
  0x006FF570  CVirtualMachineInternal::Constructor
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006fd700 FUN_006fd700 size=163 ret=0 callers=1 callees=3

int __fastcall FUN_006fd700(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095edf1;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]((void *)(param_1 + 0x14),8,1,FUN_00733540,FUN_00733780);
  local_8 = 0;
  iVar2 = FUN_00919723(0x3d8,uVar1);
  local_8 = CONCAT31(local_8._1_3_,1);
  if (iVar2 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_006ff570();
  }
  *(undefined4 *)(param_1 + 0x1c) = local_20;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x006fd7b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fd7b0: 16-byte aligned: YES
  006fd7b0  55             PUSH EBP
  006fd7b1  8B EC          MOV EBP,ESP
  006fd7b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006fd7b0
string literals (0):
direct callees (1):
  0x00919BCC  `eh vector destructor iterator` (CRT)
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EAX
```

## Card B0-4-25

GOG function: CExoResMan::FreeResourceData_2  gog 0x006206e0  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=16
Candidate Steam address: 0x00716d80 (size 18, retn 16)
Candidate sources: branch+name_map:high:gog_cexoresman_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00716de0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0061f560 CExoResMan::AsyncLoad -> 0x00715c00  gate_class=AUTO
- GOG-order neighbour after:  0x00620700 CExoResMan::CacheResource -> 0x00716da0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 16)
```

### prologue
```
prologue for 0x00716d80: 16-byte aligned: YES
  00716d80  55             PUSH EBP
  00716d81  8B EC          MOV EBP,ESP
  00716d83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00716d80
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00716d80 FUN_00716d80 size=18 ret=16 callers=3 callees=0

undefined4 FUN_00716d80(void)

{
  return 1;
}
```

### decoy: 0x00716de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00716de0: 16-byte aligned: YES
  00716de0  55             PUSH EBP
  00716de1  8B EC          MOV EBP,ESP
  00716de3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00716de0
string literals (0):
direct callees (2):
  0x007335F0  CExoString::CExoString_copy
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B0-4-26

GOG function: CResGFF::ReadFieldBYTE  gog 0x00624b60  callconv=__thiscall  param_size_bytes=12  gog_size=122  gog_retn=16
Candidate Steam address: 0x00718a00 (size 122, retn 16)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (GetFieldByLabel 0x7178e0,Load 0x415080,Load 0x41ffd0)
- best decoy (neighbour/other candidate) scores: 0x00718a80:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00624b30 CResGFF::GetTopLevelStruct -> 0x007189d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00624be0 CResGFF::ReadFieldCHAR -> 0x00718a80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x00623a40 CResGFF::GetFieldByLabel -> Steam 0x007178e0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x00718a00: 16-byte aligned: YES
  00718a00  55             PUSH EBP
  00718a01  8B EC          MOV EBP,ESP
  00718a03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00718a00
string literals (0):
direct callees (2):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00718a00 FUN_00718a00 size=122 ret=16 callers=50 callees=2

undefined1 FUN_00718a00(int param_1,int param_2,undefined4 *param_3,undefined1 param_4)

{
  undefined4 uVar1;
  int *piVar2;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 0)) {
      *param_3 = 0;
    }
    else {
      *param_3 = 1;
      param_4 = (undefined1)piVar2[2 = byte +0x8];
    }
  }
  return param_4;
}
```

### decoy: 0x00718a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00718a80: 16-byte aligned: YES
  00718a80  55             PUSH EBP
  00718a81  8B EC          MOV EBP,ESP
  00718a83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00718a80
string literals (0):
direct callees (2):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-4-27

GOG function: Scene::SetCurrentRoom_2  gog 0x008621f0  callconv=(blank)  param_size_bytes=4  gog_size=72  gog_retn=4
Candidate Steam address: 0x0046ff90 (size 25, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x0046ffb0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.67 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0046ffb0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008621d0 Scene::SetCurrentRoom  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00862240 Scene::SpawnRooms  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xd4,0xd8,0xe0*} | Steam {0xe0*}
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
prologue for 0x0046ff90: 16-byte aligned: YES
  0046ff90  55             PUSH EBP
  0046ff91  8B EC          MOV EBP,ESP
  0046ff93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0046ff90
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE0 [dec 224]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0046ff90 FUN_0046ff90 size=25 ret=4 callers=0 callees=0

void __thiscall FUN_0046ff90(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xe0) = param_2;
  return;
}
```

### decoy: 0x0046ffb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0046ffb0: 16-byte aligned: YES
  0046ffb0  55             PUSH EBP
  0046ffb1  8B EC          MOV EBP,ESP
  0046ffb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0046ffb0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0xE0 [dec 224]  x2  via ECX
  +0xD4 [dec 212]  x1  via EDX
  +0xD8 [dec 216]  x1  via EAX
```

## Card B0-4-28

GOG function: CResGFF::WriteFieldWORD  gog 0x00625c10  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x00719ab0 (size 97, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00719a40:STR0/CALLEE2/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625ba0 CResGFF::WriteFieldCHAR -> 0x00719a40  gate_class=REVIEW
- GOG-order neighbour after:  0x00625c80 CResGFF::WriteFieldSHORT -> 0x00719b20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam 0x00717aa0  in candidate callees: yes
  0x00624190 CResGFF::AddField -> Steam 0x00718030  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x8*,0x98*}
imm sets (* = in both, 3 common): GOG {0x2*,0x8*,0xc*} | Steam {0x2*,0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00719ab0: 16-byte aligned: YES
  00719ab0  55             PUSH EBP
  00719ab1  8B EC          MOV EBP,ESP
  00719ab3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00719ab0
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00719ab0 FUN_00719ab0 size=97 ret=12 callers=19 callees=2

undefined4 __thiscall FUN_00719ab0(int param_1,int param_2,ushort param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,2);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(uint *)(iVar2 + 8) = (uint)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00719a40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719a40: 16-byte aligned: YES
  00719a40  55             PUSH EBP
  00719a41  8B EC          MOV EBP,ESP
  00719a43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719a40
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-4-29

GOG function: CSWCArea::AttachmentFromServerArea  gog 0x00457880  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x007aabd0 (size 266, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCArea::GetRoom_3
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007aab90:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00457800 CSWCArea::GetServerArea -> 0x007aae00  gate_class=AUTO
- GOG-order neighbour after:  0x004578a0 CSWCArea::DetachFromServerArea -> 0x007aaea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x280} | Steam {0x4,0x8,0xc,0x260,0x264}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x20,0x2c,0x48}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007aabd0: 16-byte aligned: YES
  007aabd0  55             PUSH EBP
  007aabd1  8B EC          MOV EBP,ESP
  007aabd3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007aabd0
string literals (0):
direct callees (6):
  0x00733540  CExoString::CExoString(empty)
  0x00710930  (unnamed)
  0x00734010  CExoString::CStr
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00733780  CExoString::~CExoString
  0x009196EE  __security_check_cookie
struct offsets touched (top 5):
  +0x264 [dec 612]  x2  via ECX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x260 [dec 608]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007aabd0 FUN_007aabd0 size=266 ret=4 callers=1 callees=6

void __fastcall FUN_007aabd0(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_34 [8];
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  uint local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00968288;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = 0;
  local_18 = 0;
  local_1c = uVar1;
  do {
    if (*(int *)(param_1 + 0x260) <= local_18) {
LAB_007aacbc:
      ExceptionList = local_10;
      __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_1c ^ (uint)&stack0xfffffffc);
      return;
    }
    iVar2 = local_18 * 0x48 + *(int *)(param_1 + 0x264);
    local_2c = *(undefined4 *)(iVar2 + 0x20);
    local_28 = *(undefined4 *)(iVar2 + 0x24);
    local_24 = *(undefined4 *)(iVar2 + 0x28);
    local_20 = *(undefined4 *)(iVar2 + 0x2c);
    FUN_00733540(uVar1);
    local_8 = 0;
    FUN_00710930(local_34);
    uVar3 = FUN_00734010();
    iVar2 = FUN_00734f60(uVar3);
    if (iVar2 != 0) {
      local_14 = local_18 * 0x48 + *(int *)(param_1 + 0x264);
      local_8 = 0xffffffff;
      FUN_00733780();
      goto LAB_007aacbc;
    }
    local_8 = 0xffffffff;
    FUN_00733780();
    local_18 = local_18 + 1;
  } while( true );
}
```

### decoy: 0x007aab90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007aab90: 16-byte aligned: YES
  007aab90  55             PUSH EBP
  007aab91  8B EC          MOV EBP,ESP
  007aab93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007aab90
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x264 [dec 612]  x1  via ECX
```

## Card B0-4-30

GOG function: CResGFF::ReadFieldDWORD  gog 0x00624d60  callconv=__thiscall  param_size_bytes=16  gog_size=122  gog_retn=16
Candidate Steam address: 0x00718c00 (size 122, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00718b80:STR0/CALLEE1/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00624ce0 CResGFF::ReadFieldSHORT -> 0x00718b80  gate_class=AUTO
- GOG-order neighbour after:  0x00624de0 CResGFF::ReadFieldINT -> 0x00718c80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x00623a40 CResGFF::GetFieldByLabel -> Steam 0x007178e0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x10*} | Steam {0x4*,0xc*,0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x00718c00: 16-byte aligned: YES
  00718c00  55             PUSH EBP
  00718c01  8B EC          MOV EBP,ESP
  00718c03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00718c00
string literals (0):
direct callees (2):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00718c00 FUN_00718c00 size=122 ret=16 callers=67 callees=2

int FUN_00718c00(int param_1,int param_2,undefined4 *param_3,int param_4)

{
  undefined4 uVar1;
  int *piVar2;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 4)) {
      *param_3 = 0;
    }
    else {
      *param_3 = 1;
      param_4 = piVar2[2 = byte +0x8];
    }
  }
  return param_4;
}
```

### decoy: 0x00718b80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00718b80: 16-byte aligned: YES
  00718b80  55             PUSH EBP
  00718b81  8B EC          MOV EBP,ESP
  00718b83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00718b80
string literals (0):
direct callees (2):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-4-31

GOG function: CResGFF::GetField_2  gog 0x00623b60  callconv=(blank)  param_size_bytes=8  gog_size=156  gog_retn=8
Candidate Steam address: 0x00717a00 (size 156, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00717720:STR0/CALLEE1/SIG0.389/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00623a40 CResGFF::GetFieldByLabel -> 0x007178e0  gate_class=AUTO
- GOG-order neighbour after:  0x00623c00 CResGFF::InitializeForWriting -> 0x00717aa0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0x14*,0x2c*,0x40*,0x4c*,0x64*} | Steam {0x4*,0x8*,0x14*,0x2c*,0x40*,0x4c*,0x64*}
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
prologue for 0x00717a00: 16-byte aligned: YES
  00717a00  55             PUSH EBP
  00717a01  8B EC          MOV EBP,ESP
  00717a03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00717a00
string literals (0):
direct callees (0):
struct offsets touched (top 7):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x4C [dec 76]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x40 [dec 64]  x2  via ECX,EDX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0x14 [dec 20]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00717a00 FUN_00717a00 size=156 ret=8 callers=1 callees=0

int __thiscall FUN_00717a00(int param_1,int param_2,uint param_3)

{
  int iVar1;
  
  if (((param_2 != 0) && (*(int *)(param_1 + 0x4c) != 0)) && (param_3 < *(uint *)(param_2 + 8))) {
    if (*(int *)(param_2 + 8) == 1) {
      if (*(uint *)(param_2 + 4) < *(uint *)(*(int *)(param_1 + 0x40) + 0x14)) {
        return *(int *)(param_2 + 4) * 0xc + *(int *)(param_1 + 0x4c);
      }
    }
    else {
      iVar1 = *(int *)(param_2 + 4) + param_3 * 4;
      if ((*(int *)(param_1 + 0x64 [dec 100]) != 0) &&
         (iVar1 + 4U <= *(uint *)(*(int *)(param_1 + 0x40) + 0x2c))) {
        return *(int *)(*(int *)(param_1 + 0x64 [dec 100]) + iVar1) * 0xc + *(int *)(param_1 + 0x4c);
      }
    }
  }
  return 0;
}
```

### decoy: 0x00717720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00717720: 16-byte aligned: YES
  00717720  55             PUSH EBP
  00717721  8B EC          MOV EBP,ESP
  00717723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00717720
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x40 [dec 64]  x2  via EAX,EDX
  +0x44 [dec 68]  x2  via ECX
  +0xC [dec 12]  x1  via EAX
```

## Card B0-4-32

GOG function: Scene::SetCurrentRoom_2  gog 0x008621f0  callconv=(blank)  param_size_bytes=4  gog_size=72  gog_retn=4
Candidate Steam address: 0x00470000 (size 644, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x0046ffb0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0046ffb0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008621d0 Scene::SetCurrentRoom  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00862240 Scene::SpawnRooms  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xd4,0xd8*,0xe0} | Steam {0x4,0x8,0x4c,0x5c,0x70,0x94,0xd8*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0x9,0xc,0x10,0x18,0x20,0xd8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00470000: 16-byte aligned: YES
  00470000  55             PUSH EBP
  00470001  8B EC          MOV EBP,ESP
  00470003  81 EC D8 00 00 00 SUB ESP,0xd8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00470000
string literals (6):
  0x0098BDE8  at 004700a7  '%s %f%f%f'
  0x0098BDDC  at 0047011f  'roomcount'
  0x0098BDDC  at 00470134  'roomcount'
  0x0098BDE8  at 0047016a  '%s %f%f%f'
  0x0098BDD0  at 0047020a  'trackcount'
  0x0098BDD0  at 0047021f  'trackcount'
direct callees (10):
  0x00475AB0  (unnamed)
  0x00475F70  (unnamed)
  0x0091C7AD  (unnamed)
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x00484E20  (unnamed)
  0x00462320  (unnamed)
  0x00475EA0  CloseModelStream
  0x009196EE  __security_check_cookie
struct offsets touched (top 7):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x4C [dec 76]  x1  via EAX
  +0x5C [dec 92]  x1  via EDX
  +0x70 [dec 112]  x1  via EAX
  +0x94 [dec 148]  x1  via EDX
  +0xD8 [dec 216]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00470000 FUN_00470000 size=644 ret=4 callers=0 callees=10

void __thiscall FUN_00470000(int *param_1,undefined4 param_2)

{
  char *pcVar1;
  int iVar2;
  size_t sVar3;
  int *piVar4;
  undefined1 local_d8 [12];
  int *local_cc;
  int local_c8;
  undefined4 local_c4;
  int local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined1 local_ac [132];
  uint local_28;
  char *local_24;
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int local_10;
  undefined4 local_c;
  int local_8;
  
  local_28 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_10 = FUN_00475ab0(param_2,&DAT_0098bdf4,0,1);
  if (local_10 != 0) {
    local_20 = 0;
    local_8 = 0;
    local_1c = 0;
    local_18 = 0;
    local_14 = 0;
    while (local_24 = (char *)FUN_00475f70(), local_24 != (char *)0x0) {
      for (; (*local_24 == ' ' || (*local_24 == '\t')); local_24 = local_24 + 1) {
      }
      if (0 < local_20) {
        local_c0 = FID_conflict__sscanf [GHIDRA-AUTO-LABEL: unreliable](local_24,"%s %f%f%f",local_ac,&local_1c,&local_18,&local_14)
        ;
        local_b0 = 0;
        local_b4 = 0;
        local_b8 = 0;
        local_bc = 0x3f800000;
        local_c4 = (**(code **)(*param_1 + 0x70))(local_ac,&local_1c,&local_bc);
        local_20 = local_20 + -1;
      }
      pcVar1 = (char *)FUN_00484d10(local_24);
      iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("roomcount",pcVar1);
      if (iVar2 == 0) {
        piVar4 = &local_20;
        sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("roomcount");
        FUN_00484e20(local_24 + sVar3,piVar4);
      }
      if (0 < local_8) {
        local_c8 = FID_conflict__sscanf [GHIDRA-AUTO-LABEL: unreliable](local_24,"%s %f%f%f",local_ac,&local_1c,&local_18,&local_14)
        ;
        local_cc = (int *)FUN_00462320(local_ac,local_ac,0,0);
        if (local_cc != (int *)0x0) {
          (**(code **)(*local_cc + 0x4c))(param_1);
          (**(code **)(*local_cc + 0x5c))(local_d8,local_1c,local_18,local_14);
        }
        local_8 = local_8 + -1;
      }
      pcVar1 = (char *)FUN_00484d10(local_24);
      iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("trackcount",pcVar1);
      if (iVar2 == 0) {
        piVar4 = &local_8;
        sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("trackcount");
        FUN_00484e20(local_24 + sVar3,piVar4);
      }
    }
    FUN_00475ea0(local_10,0);
    local_c = (**(code **)(*param_1 + 0x94))(param_2);
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_28 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x0046ffb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0046ffb0: 16-byte aligned: YES
  0046ffb0  55             PUSH EBP
  0046ffb1  8B EC          MOV EBP,ESP
  0046ffb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0046ffb0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0xE0 [dec 224]  x2  via ECX
  +0xD4 [dec 212]  x1  via EDX
  +0xD8 [dec 216]  x1  via EAX
```

## Card B0-4-33

GOG function: CExoResMan::CacheResource_2  gog 0x00620740  callconv=(blank)  param_size_bytes=8  gog_size=119  gog_retn=16
Candidate Steam address: 0x00716de0 (size 119, retn 16)
Candidate sources: branch+name_map:high:gog_cexoresman_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00716d80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00620700 CExoResMan::CacheResource -> 0x00716da0  gate_class=REVIEW
- GOG-order neighbour after:  0x006207c0 CExoLinkedListInternal::Remove_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x00716de0: 16-byte aligned: YES
  00716de0  55             PUSH EBP
  00716de1  8B EC          MOV EBP,ESP
  00716de3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00716de0
string literals (0):
direct callees (2):
  0x007335F0  CExoString::CExoString_copy
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00716de0 FUN_00716de0 size=119 ret=16 callers=1 callees=2

undefined4 FUN_00716de0(undefined4 param_1,undefined4 param_2)

{
  uint uVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  uint uVar3;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972a61;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  uVar3 = 0;
  FUN_007335f0(param_2);
  local_8 = 1;
  uVar2 = extraout_var;
  FUN_007335f0(local_18);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780(uVar1,uVar2,uVar3 | 1);
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00716d80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00716d80: 16-byte aligned: YES
  00716d80  55             PUSH EBP
  00716d81  8B EC          MOV EBP,ESP
  00716d83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00716d80
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-4-34

GOG function: CSWSEncounter::Destructor  gog 0x007e94f0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00622c90 (size 894, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEncounter::Constructor
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00622c10:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e94d0 CSWSEncounter::GetFirstName -> 0x00623010  gate_class=REVIEW
- GOG-order neighbour after:  0x007e9520 CSWSEncounter::Destructor_2 -> 0x00623060  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007e9520 CSWSEncounter::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x8,0xac,0xb0,0x26c,0x27c,0x280,0x284,0x288,0x290,0x294,0x2a4,0x2a8,0x2ac,0x2b0,...(+27)}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x3,0x4*,0x5,0x8,0xd,0x10,0x30,0x32,0x3c,0x270,0x298,0x328,0x7f000000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00622c90: 16-byte aligned: YES
  00622c90  55             PUSH EBP
  00622c91  8B EC          MOV EBP,ESP
  00622c93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00622c90
string literals (0):
direct callees (14):
  0x0053E830  CSWSObject::Constructor
  0x00735240  (unnamed)
  0x0084A640  (unnamed)
  0x0091D67E  (unnamed)
  0x0051C280  (unnamed)
  0x0051E040  CServerAIMaster::AddObject
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x00919723  operator new
  0x006288F0  (unnamed)
  0x0058C290  (unnamed)
  0x00627E50  CSWSEncounter::LoadFractionalCRData
  0x00627F60  CSWSEncounter::SetActive
  0x00543F50  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x2  via EAX
  +0x2C0 [dec 704]  x2  via EDX
  +0xAC [dec 172]  x1  via ECX
  +0xB0 [dec 176]  x1  via EAX
  +0x26C [dec 620]  x1  via ECX
  +0x27C [dec 636]  x1  via EDX
  +0x280 [dec 640]  x1  via EAX
  +0x284 [dec 644]  x1  via ECX
  +0x288 [dec 648]  x1  via EDX
  +0x290 [dec 656]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00622c90 FUN_00622c90 size=894 ret=4 callers=2 callees=14

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_00622c90(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  float10 fVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009576cc;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0053e830(0xd,param_2,0);
  local_8 = 0;
  *param_1 = CSWSEncounter::vftable;
  FUN_00735240(uVar1);
  local_8._0_1_ = 1;
  FUN_0084a640(0);
  local_8._0_1_ = 2;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0xca,8,5,FUN_00733540,FUN_00733780);
  local_8 = CONCAT31(local_8._1_3_,3);
  uVar5 = 0;
  puVar3 = param_1;
  FUN_0051c280(param_1,0);
  FUN_0051e040(puVar3,uVar5);
  puVar3 = &local_14;
  puVar4 = &local_18;
  FUN_0051c370(puVar4,puVar3);
  FUN_0051ad20(puVar4,puVar3);
  param_1[0x2b = byte +0xac] = local_18;
  param_1[0x2c = byte +0xb0] = local_14;
  param_1[0xac = byte +0x2b0] = 0;
  param_1[0xad = byte +0x2b4] = 0;
  param_1[0xab = byte +0x2ac] = 0;
  param_1[0xaa = byte +0x2a8] = 0;
  param_1[0xbd = byte +0x2f4] = 0;
  param_1[0xbc = byte +0x2f0] = 0;
  param_1[0xb0 = byte +0x2c0] = 0x10;
  local_20 = FUN_00919723(-(uint)((int)((ulonglong)(uint)param_1[0xb0 = byte +0x2c0] * 4 >> 0x20) != 0) |
                          (uint)((ulonglong)(uint)param_1[0xb0 = byte +0x2c0] * 4));
  param_1[0xb1 = byte +0x2c4] = local_20;
  param_1[0xb2 = byte +0x2c8] = 0;
  param_1[0xb3 = byte +0x2cc] = 0;
  param_1[0xb4 = byte +0x2d0] = 0;
  param_1[0xb6 = byte +0x2d8] = 0;
  param_1[0xb5 = byte +0x2d4] = 0;
  param_1[0xb7 = byte +0x2dc] = 0;
  param_1[200 = byte +0x320] = 0x7f000000;
  param_1[0xc0 = byte +0x300] = 0;
  param_1[0xbf = byte +0x2fc] = 0;
  param_1[0xc2 = byte +0x308] = 0;
  param_1[0xa9 = byte +0x2a4] = 0;
  param_1[0xc6 = byte +0x318] = 0;
  param_1[199 = byte +0x31c] = 0;
  param_1[0xc3 = byte +0x30c] = 0;
  param_1[0xc4 = byte +0x310] = 0;
  param_1[0xc9 = byte +0x324] = 0x7f000000;
  param_1[0xbe = byte +0x2f8] = 0;
  param_1[0xae = byte +0x2b8] = 0;
  for (local_1c = 0; local_1c < 0x33; local_1c = local_1c + 1) {
    fVar2 = (float10)FUN_006288f0(_DAT_0098b5c0,local_1c);
    FUN_0058c290((float)fVar2);
  }
  FUN_00627e50();
  FUN_00627f60(1);
/* ... 19 more lines truncated by --max-lines */
```

### decoy: 0x00622c10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00622c10: 16-byte aligned: YES
  00622c10  55             PUSH EBP
  00622c11  8B EC          MOV EBP,ESP
  00622c13  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00622c10
string literals (0):
direct callees (3):
  0x0051C8F0  (unnamed)
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FB070  CSWPartyTable::GetIsMember_2
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x354 [dec 852]  x2  via ECX,EDX
  +0x328 [dec 808]  x1  via EAX
```

## Card B0-4-35

GOG function: CExoResMan::ServiceFromImageRaw  gog 0x0061e490  callconv=(blank)  param_size_bytes=12  gog_size=292  gog_retn=12
Candidate Steam address: 0x00714b30 (size 292, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00714c60:STR0/CALLEE0/SIG0.764/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061e370 CExoResMan::ServiceFromEncapsulatedRaw -> 0x00714a10  gate_class=AUTO
- GOG-order neighbour after:  0x0061e5c0 CExoResMan::ServiceFromResFileRaw -> 0x00714c60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0061c890 CExoResMan::Malloc -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x8*,0x18*,0x24*,0x28*,0x30*,0x34*} | Steam {0x8*,0x18*,0x24*,0x28*,0x30*,0x34*}
imm sets (* = in both, 8 common): GOG {0xc*,0xe*,0x14*,0x1c*,0x3fff*,0xfc000*,0xfffffff*,0x3ff00000*} | Steam {0xc*,0xe*,0x14*,0x1c*,0x3fff*,0xfc000*,0xfffffff*,0x3ff00000*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00714b30: 16-byte aligned: YES
  00714b30  55             PUSH EBP
  00714b31  8B EC          MOV EBP,ESP
  00714b33  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00714b30
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00712F30  CExoResMan::Malloc
  0x0058C370  (unnamed)
struct offsets touched (top 6):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EAX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x24 [dec 36]  x1  via EDX
  +0x28 [dec 40]  x1  via EAX
  +0x34 [dec 52]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00714b30 FUN_00714b30 size=292 ret=12 callers=1 callees=4

undefined4 __thiscall FUN_00714b30(undefined4 param_1,int param_2,int param_3,undefined4 param_4)

{
  int iVar1;
  int local_14;
  int local_10;
  undefined4 local_c;
  uint local_8;
  
  local_10 = 0;
  if (param_2 != 0) {
    local_8 = (*(uint *)(param_2 + 8) & 0x3ff00000) >> 0x14;
    local_14 = FUN_007a1720(param_1);
    if (local_14 != 0) {
      local_10 = FUN_00561430(local_14);
    }
    while (local_14 != 0) {
      if ((local_10 != 0) &&
         ((*(uint *)(local_10 + 0x28) & 0xfffffff) == (*(uint *)(param_2 + 8) & 0xfc000) >> 0xe)) {
        local_c = (**(code **)(*(int *)**(undefined4 **)(local_10 + 0x30) + 0x24))
                            (*(uint *)(param_2 + 8) & 0x3fff);
        *(int *)(param_2 + 0x18) = param_3;
        iVar1 = FUN_00712f30(param_2);
        if (iVar1 == 0) {
          return 0;
        }
        iVar1 = (**(code **)(*(int *)**(undefined4 **)(local_10 + 0x30) + 0x34))
                          (*(undefined4 *)(param_2 + 8),param_4,param_3,0);
        if (iVar1 == param_3) {
          return 1;
        }
        return 0;
      }
      local_10 = FUN_0058c370(&local_14);
    }
  }
  return 0;
}
```

### decoy: 0x00714c60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00714c60: 16-byte aligned: YES
  00714c60  55             PUSH EBP
  00714c61  8B EC          MOV EBP,ESP
  00714c63  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00714c60
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 8):
  +0x30 [dec 48]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x24 [dec 36]  x1  via EDX
  +0x28 [dec 40]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

## Card B0-4-36

GOG function: CResGFF::GetFieldCount  gog 0x00624820  callconv=(blank)  param_size_bytes=4  gog_size=30  gog_retn=4
Candidate Steam address: 0x007186c0 (size 30, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (GetFieldByLabel 0x7178e0)
- best decoy (neighbour/other candidate) scores: 0x007186e0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006247f0 CResGFF::GetElementType -> 0x00718690  gate_class=REVIEW
- GOG-order neighbour after:  0x00624840 CResGFF::GetFieldCount_2 -> 0x007186e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x007186c0: 16-byte aligned: YES
  007186c0  55             PUSH EBP
  007186c1  8B EC          MOV EBP,ESP
  007186c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007186c0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007186c0 FUN_007186c0 size=30 ret=4 callers=1 callees=0

undefined4 FUN_007186c0(int param_1)

{
  undefined4 uVar1;
  
  if (param_1 == 0) {
    uVar1 = 0xffffffff;
  }
  else {
    uVar1 = *(undefined4 *)(param_1 + 8);
  }
  return uVar1;
}
```

### decoy: 0x007186e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007186e0: 16-byte aligned: YES
  007186e0  55             PUSH EBP
  007186e1  8B EC          MOV EBP,ESP
  007186e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007186e0
string literals (0):
direct callees (1):
  0x00717720  CResGFF::GetStruct
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B0-4-37

GOG function: CResGFF::WriteFieldINT  gog 0x00625d50  callconv=(blank)  param_size_bytes=12  gog_size=96  gog_retn=12
Candidate Steam address: 0x00719bf0 (size 96, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00719b90:STR0/CALLEE2/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625cf0 CResGFF::WriteFieldDWORD -> 0x00719b90  gate_class=REVIEW
- GOG-order neighbour after:  0x00625db0 CResGFF::WriteFieldDWORD64 -> 0x00719c50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam 0x00717aa0  in candidate callees: yes
  0x00624190 CResGFF::AddField -> Steam 0x00718030  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x8*,0x98*}
imm sets (* = in both, 3 common): GOG {0x5*,0x8*,0xc*} | Steam {0x5*,0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00719bf0: 16-byte aligned: YES
  00719bf0  55             PUSH EBP
  00719bf1  8B EC          MOV EBP,ESP
  00719bf3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00719bf0
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00719bf0 FUN_00719bf0 size=96 ret=12 callers=32 callees=2

undefined4 __thiscall FUN_00719bf0(int param_1,int param_2,undefined4 param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,5);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(undefined4 *)(iVar2 + 8) = param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00719b90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719b90: 16-byte aligned: YES
  00719b90  55             PUSH EBP
  00719b91  8B EC          MOV EBP,ESP
  00719b93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719b90
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-4-38

GOG function: CExoResMan::GetResTypeFromFile  gog 0x0061c5f0  callconv=(blank)  param_size_bytes=4  gog_size=219  gog_retn=4
Candidate Steam address: 0x00712c90 (size 219, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00712be0:STR0/CALLEE2/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061c540 CExoResMan::GetResRefFromFile -> 0x00712be0  gate_class=REVIEW
- GOG-order neighbour after:  0x0061c6d0 CExoResMan::GetTable -> 0x00712d70  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006062a0 CExoString::Find_2 -> Steam ?  in candidate callees: n/a
  0x00606b10 CExoString::SubString -> Steam 0x00734a00  in candidate callees: yes
  0x006081f0 CExoBase::GetResTypeFromExtension -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x3*,0x4*,0x28*,0x2e*,0xffff*} | Steam {0x3*,0x4*,0x28*,0x2e*,0xffff*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00712c90: 16-byte aligned: YES
  00712c90  55             PUSH EBP
  00712c91  8B EC          MOV EBP,ESP
  00712c93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00712c90
string literals (0):
direct callees (6):
  0x00733540  CExoString::CExoString(empty)
  0x00734190  CExoString::Find_2
  0x00733780  CExoString::~CExoString
  0x00734A00  CExoString::SubString
  0x007337C0  CExoString::operator=
  0x007360C0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00712c90 FUN_00712c90 size=219 ret=4 callers=2 callees=6

undefined2 FUN_00712c90(void)

{
  undefined2 uVar1;
  undefined4 uVar2;
  undefined1 local_28 [10];
  undefined2 local_1e;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ec40;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  local_14 = FUN_00734190(0x2e,0);
  if (local_14 < 0) {
    local_1e = 0xffff;
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar1 = local_1e;
  }
  else {
    uVar2 = FUN_00734a00(local_28,local_14 + 1,3);
    local_8._0_1_ = 1;
    FUN_007337c0(uVar2);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    uVar1 = FUN_007360c0(local_1c);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return uVar1;
}
```

### decoy: 0x00712be0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00712be0: 16-byte aligned: YES
  00712be0  55             PUSH EBP
  00712be1  8B EC          MOV EBP,ESP
  00712be3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00712be0
string literals (0):
direct callees (5):
  0x00734190  CExoString::Find_2
  0x00710EB0  (unnamed)
  0x00734A00  CExoString::SubString
  0x00710DC0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B0-4-39

GOG function: CResGFF::GetFieldType  gog 0x00624870  callconv=(blank)  param_size_bytes=12  gog_size=75  gog_retn=12
Candidate Steam address: 0x00718710 (size 75, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00718760:STR0/CALLEE1/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00624840 CResGFF::GetFieldCount_2 -> 0x007186e0  gate_class=AUTO
- GOG-order neighbour after:  0x006248c0 CResGFF::GetList -> 0x00718760  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x00623a40 CResGFF::GetFieldByLabel -> Steam 0x007178e0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00718710: 16-byte aligned: YES
  00718710  55             PUSH EBP
  00718711  8B EC          MOV EBP,ESP
  00718713  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00718710
string literals (0):
direct callees (2):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00718710 FUN_00718710 size=75 ret=12 callers=1 callees=2

undefined4 FUN_00718710(undefined4 param_1,int param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  
  if (param_2 != 0) {
    param_3 = FUN_007178e0(param_1,param_2);
  }
  puVar1 = (undefined4 *)FUN_00717770(param_1,param_3);
  if (puVar1 == (undefined4 *)0x0) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = *puVar1;
  }
  return uVar2;
}
```

### decoy: 0x00718760 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00718760: 16-byte aligned: YES
  00718760  55             PUSH EBP
  00718761  8B EC          MOV EBP,ESP
  00718763  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00718760
string literals (0):
direct callees (4):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717880  CResGFF::GetDataLayoutList
  0x0091D0F0  (unnamed)
struct offsets touched (top 0):
```

## Card B0-4-40

GOG function: CExoResMan::GetTableCount  gog 0x0061c7b0  callconv=(blank)  param_size_bytes=8  gog_size=53  gog_retn=8
Candidate Steam address: 0x00712e50 (size 53, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00712be0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061c6d0 CExoResMan::GetTable -> 0x00712d70  gate_class=AUTO
- GOG-order neighbour after:  0x0061c7f0 CExoResMan::GetIsStaticType -> 0x00712e90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061c6d0 CExoResMan::GetTable -> Steam ?  in candidate callees: n/a
  0x0062faa0 CExoKeyTable::GetEntryCount -> Steam ?  in candidate callees: n/a
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
prologue for 0x00712e50: 16-byte aligned: YES
  00712e50  55             PUSH EBP
  00712e51  8B EC          MOV EBP,ESP
  00712e53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00712e50
string literals (0):
direct callees (2):
  0x00712D70  CExoResMan::GetTable
  0x00726050  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00712e50 FUN_00712e50 size=53 ret=8 callers=1 callees=2

undefined4 FUN_00712e50(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_00712d70(param_1);
  if (iVar1 == 0) {
    uVar2 = 0xffffffff;
  }
  else {
    uVar2 = FUN_00726050(param_2);
  }
  return uVar2;
}
```

### decoy: 0x00712be0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00712be0: 16-byte aligned: YES
  00712be0  55             PUSH EBP
  00712be1  8B EC          MOV EBP,ESP
  00712be3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00712be0
string literals (0):
direct callees (5):
  0x00734190  CExoString::Find_2
  0x00710EB0  (unnamed)
  0x00734A00  CExoString::SubString
  0x00710DC0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B0-4-41

GOG function: CSWSItem::GetPropertyByTypeExists  gog 0x00667600  callconv=(blank)  param_size_bytes=4  gog_size=245  gog_retn=4
Candidate Steam address: 0x00603ee0 (size 63, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00603dd0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00667380 CSWSItem::GetUsedActivePropertyUsesLeft -> 0x00603b50  gate_class=AUTO
- GOG-order neighbour after:  0x00667710 CSWSItem::RestoreUsedActiveProperties -> 0x00603ee0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00667a90 CSWSItem::GetActiveProperty -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x6,0xc,0x10,0x288} | Steam {}
imm sets (* = in both, 3 common): GOG {0x2,0x3,0x4*,0x5,0x8*,0xa*,0x20,0xff} | Steam {0x4*,0x8*,0xa*,0x2b}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00603ee0: 16-byte aligned: YES
  00603ee0  55             PUSH EBP
  00603ee1  8B EC          MOV EBP,ESP
  00603ee3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00603ee0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x603F28 [dec 6307624]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00603ee0 FUN_00603ee0 size=63 ret=4 callers=1 callees=0

undefined4 FUN_00603ee0(undefined2 param_1)

{
  undefined4 uVar1;
  
  switch(param_1) {
  case 10:
  case 0x25:
  case 0x2e:
  case 0x35:
    uVar1 = 1;
    break;
  default:
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00603dd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00603dd0: 16-byte aligned: YES
  00603dd0  55             PUSH EBP
  00603dd1  8B EC          MOV EBP,ESP
  00603dd3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00603dd0
string literals (0):
direct callees (1):
  0x00604260  CSWSItem::GetActiveProperty
struct offsets touched (top 4):
  +0x10 [dec 16]  x6  via EAX,ECX,EDX
  +0x6 [dec 6]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x288 [dec 648]  x1  via ECX
```

## Card B0-4-42

GOG function: CResGFF::WriteFieldSHORT  gog 0x00625c80  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x00719b20 (size 97, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00719ab0:STR0/CALLEE2/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625c10 CResGFF::WriteFieldWORD -> 0x00719ab0  gate_class=REVIEW
- GOG-order neighbour after:  0x00625cf0 CResGFF::WriteFieldDWORD -> 0x00719b90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam 0x00717aa0  in candidate callees: yes
  0x00624190 CResGFF::AddField -> Steam 0x00718030  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x8*,0x98*}
imm sets (* = in both, 3 common): GOG {0x3*,0x8*,0xc*} | Steam {0x3*,0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00719b20: 16-byte aligned: YES
  00719b20  55             PUSH EBP
  00719b21  8B EC          MOV EBP,ESP
  00719b23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00719b20
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00719b20 FUN_00719b20 size=97 ret=12 callers=6 callees=2

undefined4 __thiscall FUN_00719b20(int param_1,int param_2,short param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,3);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(int *)(iVar2 + 8) = (int)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00719ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719ab0: 16-byte aligned: YES
  00719ab0  55             PUSH EBP
  00719ab1  8B EC          MOV EBP,ESP
  00719ab3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719ab0
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-4-43

GOG function: CVirtualMachineStack::LoadStack  gog 0x006421c0  callconv=(blank)  param_size_bytes=8  gog_size=455  gog_retn=None
Candidate Steam address: 0x006fed80 (size 487, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: review:held_by=CVirtualMachineStack::SaveStack
- anchors: A-STR 3 | A-CALLEE 0 | A-SIG 0.37 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fef80:STR3/CALLEE1/SIG0.536/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00641fc0 CVirtualMachineStack::SaveStack -> 0x006fed80  gate_class=AUTO
- GOG-order neighbour after:  0x00642387 CVirtualMachineStack::LoadStack_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x006248c0 CResGFF::GetList -> Steam 0x00718760  in candidate callees: no
  0x006249f0 CResGFF::GetListElement -> Steam 0x00718890  in candidate callees: no
  0x00624be0 CResGFF::ReadFieldCHAR -> Steam 0x00718a80  in candidate callees: no
  0x00624de0 CResGFF::ReadFieldINT -> Steam 0x00718c80  in candidate callees: no
  0x00624e60 CResGFF::ReadFieldFLOAT -> Steam 0x00718d00  in candidate callees: no
  0x00642387 CVirtualMachineStack::LoadStack_2 -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (5): 'Value'; 'Stack'; 'TotalSize'; 'StackPointer' ...
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x10*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14,0x24,0x3d4}
imm sets (* = in both, 1 common): GOG {0x3*,0x4,0x68} | Steam {0x3*,0x8,0x10,0x30}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006fed80: 16-byte aligned: YES
  006fed80  55             PUSH EBP
  006fed81  8B EC          MOV EBP,ESP
  006fed83  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fed80
string literals (8):
  0x0099B640  at 006fed93  'BasePointer'
  0x0099B630  at 006fedab  'StackPointer'
  0x0099B624  at 006fedc2  'TotalSize'
  0x0099B61C  at 006fede7  'Stack'
  0x00992E8C  at 006fee7c  'Value'
  0x00992E8C  at 006fee9f  'Value'
  0x00992E8C  at 006feec5  'Value'
  0x00992E8C  at 006feee5  'Value'
direct callees (8):
  0x00719BF0  CResGFF::WriteFieldINT
  0x00719740  CResGFF::AddList
  0x00719800  CResGFF::AddListElement
  0x00719A40  CResGFF::WriteFieldCHAR
  0x00719CE0  CResGFF::WriteFieldFLOAT
  0x00719E50  CResGFF::WriteFieldCExoString
  0x00719B90  CResGFF::WriteFieldDWORD
  0x009196EE  __security_check_cookie
struct offsets touched (top 7):
  +0x10 [dec 16]  x5  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX
  +0x4 [dec 4]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x24 [dec 36]  x1  via EAX
  +0x3D4 [dec 980]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006fed80 FUN_006fed80 size=487 ret=8 callers=1 callees=8

void __thiscall FUN_006fed80(int *param_1,undefined4 param_2,undefined4 param_3)

{
  int local_24;
  undefined1 local_20 [4];
  undefined1 local_1c [20];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  FUN_00719bf0(param_3,param_1[1 = byte +0x4],"BasePointer");
  FUN_00719bf0(param_3,*param_1,"StackPointer");
  FUN_00719bf0(param_3,param_1[2 = byte +0x8],"TotalSize");
  if (0 < param_1[2 = byte +0x8]) {
    FUN_00719740(local_1c,param_3,"Stack");
    for (local_24 = 0; local_24 < *param_1; local_24 = local_24 + 1) {
      FUN_00719800(local_20,local_1c,local_24);
      FUN_00719a40(local_20,*(undefined1 *)(param_1[3 = byte +0xc] + local_24),&DAT_00994500);
      switch(*(undefined1 *)(param_1[3 = byte +0xc] + local_24)) {
      case 3:
        FUN_00719bf0(local_20,*(undefined4 *)(param_1[4 = byte +0x10] + local_24 * 4),"Value");
        break;
      case 4:
        FUN_00719ce0(local_20,*(undefined4 *)(param_1[4 = byte +0x10] + local_24 * 4),"Value");
        break;
      case 5:
        FUN_00719e50(local_20,*(undefined4 *)(param_1[4 = byte +0x10] + local_24 * 4),"Value");
        break;
      case 6:
        FUN_00719b90(local_20,*(undefined4 *)(param_1[4 = byte +0x10] + local_24 * 4),"Value");
        break;
      default:
        (**(code **)(**(int **)(param_1[5 = byte +0x14] + 0x3d4) + 0x24))
                  (*(char *)(param_1[3 = byte +0xc] + local_24) + -0x10,
                   *(undefined4 *)(param_1[4 = byte +0x10] + local_24 * 4),param_2,local_20);
      }
    }
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x006fef80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fef80: 16-byte aligned: YES
  006fef80  55             PUSH EBP
  006fef81  8B EC          MOV EBP,ESP
  006fef83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006fef80
string literals (8):
  0x0099B640  at 006fefb1  'BasePointer'
  0x0099B630  at 006fefce  'StackPointer'
  0x0099B624  at 006fefea  'TotalSize'
  0x0099B61C  at 006ff054  'Stack'
  0x00992E8C  at 006ff0f3  'Value'
  0x00992E8C  at 006ff11f  'Value'
  0x00992E8C  at 006ff1a4  'Value'
  0x00992E8C  at 006ff204  'Value'
direct callees (13):
  0x00718C80  CResGFF::ReadFieldINT
  0x00919723  operator new
  0x00718760  CResGFF::GetList
  0x00718890  CResGFF::GetListElement
  0x00718A80  CResGFF::ReadFieldCHAR
  0x00718D00  CResGFF::ReadFieldFLOAT
  0x00733540  CExoString::CExoString(empty)
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00718C00  CResGFF::ReadFieldDWORD
  0x009196EE  __security_check_cookie
struct offsets touched (top 7):
  +0x10 [dec 16]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x28 [dec 40]  x1  via EDX
  +0x3D4 [dec 980]  x1  via ECX
```

## Card B0-4-44

GOG function: CResGFF::ReadFieldVOID  gog 0x006257a0  callconv=(blank)  param_size_bytes=24  gog_size=254  gog_retn=24
Candidate Steam address: 0x00719640 (size 254, retn 24)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00719520:STR0/CALLEE2/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625680 CResGFF::ReadFieldVector -> 0x00719520  gate_class=AUTO
- GOG-order neighbour after:  0x006258a0 CResGFF::AddList -> 0x00719740  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x00623980 CResGFF::GetDataField -> Steam 0x00717820  in candidate callees: yes
  0x00623a40 CResGFF::GetFieldByLabel -> Steam 0x007178e0  in candidate callees: yes
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0xc*,0xd*,0x18*,0x1c*} | Steam {0x4*,0xc*,0xd*,0x18*,0x1c*}

### abi
```
GOG param bytes: 24; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(24/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=24 ; asm RET = 24
Stack Slot Match: TRUE (GOG 24 bytes -> x86 slots 24; ASM RET 24)
```

### prologue
```
prologue for 0x00719640: 16-byte aligned: YES
  00719640  55             PUSH EBP
  00719641  8B EC          MOV EBP,ESP
  00719643  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00719640
string literals (0):
direct callees (4):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717820  CResGFF::GetDataField
  0x0091DBA0  _memcpy
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00719640 FUN_00719640 size=254 ret=24 callers=6 callees=4

void * FUN_00719640(int param_1,void *param_2,uint param_3,int param_4,undefined4 *param_5,
                   void *param_6)

{
  undefined4 uVar1;
  uint local_20;
  uint local_14;
  uint *local_10;
  int *local_c;
  uint local_8;
  
  if (((param_1 == 0) || (param_4 == 0)) || (param_2 == (void *)0x0)) {
    *param_5 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_4);
    local_c = (int *)FUN_00717770(param_1,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 0xd)) {
      *param_5 = 0;
    }
    else {
      local_10 = (uint *)FUN_00717820(local_c,&local_14);
      if ((local_10 == (uint *)0x0) || (local_14 < 4)) {
        *param_5 = 0;
      }
      else {
        local_8 = *local_10;
        if (local_14 < local_8 + 4) {
          *param_5 = 0;
        }
        else {
          *param_5 = 1;
          local_20 = local_8;
          if (param_3 <= local_8) {
            local_20 = param_3;
          }
          _memcpy [GHIDRA-AUTO-LABEL: unreliable](param_2,local_10 + 0x1 [dec 1] {uint*: byte +0x4},local_20);
          param_6 = param_2;
        }
      }
    }
  }
  return param_6;
}
```

### decoy: 0x00719520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719520: 16-byte aligned: YES
  00719520  55             PUSH EBP
  00719521  8B EC          MOV EBP,ESP
  00719523  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719520
string literals (0):
direct callees (3):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717820  CResGFF::GetDataField
struct offsets touched (top 2):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
```

## Card B0-4-45

GOG function: CVirtualMachine::GetRunScriptReturnValue  gog 0x00640b80  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x006fd960 (size 32, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fdab0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640b50 CVirtualMachine::RunScriptSituation -> 0x006fd930  gate_class=REVIEW
- GOG-order neighbour after:  0x00640ba0 CVirtualMachine::SetCommandImplementer -> 0x006fd980  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00645c50 CVirtualMachineInternal::GetRunScriptReturnValue -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fd960: 16-byte aligned: YES
  006fd960  55             PUSH EBP
  006fd961  8B EC          MOV EBP,ESP
  006fd963  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fd960
string literals (0):
direct callees (1):
  0x007029B0  CVirtualMachineInternal::GetRunScriptReturnValue
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006fd960 FUN_006fd960 size=32 ret=8 callers=1 callees=1
THUNK -> 0x007029B0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fd960(undefined4 param_1,undefined4 param_2)

{
  FUN_007029b0(param_1,param_2);
  return;
}
```

### decoy: 0x006fdab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fdab0: 16-byte aligned: YES
  006fdab0  55             PUSH EBP
  006fdab1  8B EC          MOV EBP,ESP
  006fdab3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fdab0
string literals (0):
direct callees (1):
  0x00702F20  CVirtualMachineInternal::StackPopEngineStructure
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EDX
```

## Card B0-4-46

GOG function: CExoResMan::CleanDirectory  gog 0x0061f040  callconv=(blank)  param_size_bytes=12  gog_size=37  gog_retn=12
Candidate Steam address: 0x007156e0 (size 37, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (DoQuickLoad 0x7d0f50)
- best decoy (neighbour/other candidate) scores: 0x00715710:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061e990 CExoResMan::WipeDirectory -> 0x00715030  gate_class=AUTO
- GOG-order neighbour after:  0x0061f070 CExoResMan::NukeDirectory -> 0x00715710  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061e990 CExoResMan::WipeDirectory -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007156e0: 16-byte aligned: YES
  007156e0  55             PUSH EBP
  007156e1  8B EC          MOV EBP,ESP
  007156e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007156e0
string literals (0):
direct callees (1):
  0x00715030  CExoResMan::WipeDirectory
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007156e0 FUN_007156e0 size=37 ret=12 callers=10 callees=1
THUNK -> 0x00715030 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007156e0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_00715030(param_1,1,0,param_3,param_2);
  return;
}
```

### decoy: 0x00715710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00715710: 16-byte aligned: YES
  00715710  55             PUSH EBP
  00715711  8B EC          MOV EBP,ESP
  00715713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00715710
string literals (0):
direct callees (1):
  0x00715030  CExoResMan::WipeDirectory
struct offsets touched (top 0):
```

## Card B0-4-47

GOG function: CResGFF::WriteFieldCHAR  gog 0x00625ba0  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x00719a40 (size 97, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007199d0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625b30 CResGFF::WriteFieldBYTE -> 0x007199d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00625c10 CResGFF::WriteFieldWORD -> 0x00719ab0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam 0x00717aa0  in candidate callees: yes
  0x00624190 CResGFF::AddField -> Steam 0x00718030  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x8*,0x98*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00719a40: 16-byte aligned: YES
  00719a40  55             PUSH EBP
  00719a41  8B EC          MOV EBP,ESP
  00719a43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00719a40
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00719a40 FUN_00719a40 size=97 ret=12 callers=6 callees=2

undefined4 __thiscall FUN_00719a40(int param_1,int param_2,char param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,1);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(int *)(iVar2 + 8) = (int)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x007199d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007199d0: 16-byte aligned: YES
  007199d0  55             PUSH EBP
  007199d1  8B EC          MOV EBP,ESP
  007199d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007199d0
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-4-48

GOG function: CResGFF::ReadFieldCExoLocString  gog 0x00625240  callconv=(blank)  param_size_bytes=20  gog_size=741  gog_retn=20
Candidate Steam address: 0x007190e0 (size 741, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00718f90:STR0/CALLEE2/SIG0.111/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006250f0 CResGFF::ReadFieldCExoString -> 0x00718f90  gate_class=AUTO
- GOG-order neighbour after:  0x00625530 CResGFF::ReadFieldQuaternion -> 0x007193d0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00605790 CExoString::Constructor_4 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00607350 CExoLocString::Constructor -> Steam ?  in candidate callees: n/a
  0x006073e0 CExoLocString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00607470 CExoLocString::Destructor -> Steam 0x00735360  in candidate callees: yes
  0x00607550 CExoLocString::AddString -> Steam ?  in candidate callees: n/a
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x00623980 CResGFF::GetDataField -> Steam 0x00717820  in candidate callees: yes
  ... 1 more GOG callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 7 common): GOG {0x2*,0x4*,0xc*,0x14*,0x48*,0x80000001*,0xfffffffe*} | Steam {0x2*,0x4*,0xc*,0x14*,0x48*,0x80000001*,0xfffffffe*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x007190e0: 16-byte aligned: YES
  007190e0  55             PUSH EBP
  007190e1  8B EC          MOV EBP,ESP
  007190e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007190e0
string literals (0):
direct callees (9):
  0x00735240  (unnamed)
  0x007352D0  (unnamed)
  0x00735360  CExoLocString::Destructor
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717820  CResGFF::GetDataField
  0x00733680  CExoString::Constructor_4
  0x00735440  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007190e0 FUN_007190e0 size=741 ret=20 callers=16 callees=9

undefined4
FUN_007190e0(undefined4 param_1,int param_2,int param_3,undefined4 *param_4,undefined4 param_5)

{
  uint uVar1;
  uint uVar2;
  undefined4 uVar3;
  uint uVar4;
  uint local_30;
  undefined1 local_2c [4];
  int local_28;
  uint *local_24;
  uint local_20;
  uint local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ff89;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00735240(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 1;
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    FUN_007352d0(param_5);
    local_8 = local_8 & 0xffffff00;
    FUN_00735360();
  }
  else {
    uVar3 = FUN_007178e0(param_2,param_3);
    local_18 = (int *)FUN_00717770(param_2,uVar3);
    if ((local_18 == (int *)0x0) || (*local_18 != 0xc)) {
      *param_4 = 0;
      FUN_007352d0(param_5);
      local_8 = local_8 & 0xffffff00;
      FUN_00735360();
    }
    else {
      local_24 = (uint *)FUN_00717820(local_18,&local_30);
      if ((local_24 == (uint *)0x0) || (local_30 < 4)) {
        *param_4 = 0;
        FUN_007352d0(param_5);
        local_8 = local_8 & 0xffffff00;
        FUN_00735360();
      }
      else {
        local_14 = *local_24;
        if (local_30 < local_14 + 4U) {
          *param_4 = 0;
          FUN_007352d0(param_5);
          local_8 = local_8 & 0xffffff00;
          FUN_00735360();
        }
        else {
          local_28 = local_24[1 = byte +0x4];
          uVar1 = local_24[2 = byte +0x8];
          local_24 = local_24 + 0x3 [dec 3] {uint*: byte +0xc};
          local_14 = local_14 + -8;
          for (local_1c = 0; local_1c < uVar1; local_1c = local_1c + 1) {
            local_20 = *local_24;
            uVar2 = local_24[1 = byte +0x4];
            local_24 = local_24 + 0x2 [dec 2] {uint*: byte +0x8};
            local_14 = local_14 + -8;
            uVar3 = FUN_00733680(local_24,uVar2);
            local_8._0_1_ = 2;
            uVar4 = local_20 & 0x80000001;
            if ((int)uVar4 < 0) {
              uVar4 = (uVar4 - 1 | 0xfffffffe) + 1;
            }
            FUN_00735440((int)local_20 / 2,uVar3,uVar4);
            local_8 = CONCAT31(local_8._1_3_,1);
            FUN_00733780();
            local_24 = (uint *)((int)local_24 + uVar2);
            local_14 = local_14 - uVar2;
          }
/* ... 19 more lines truncated by --max-lines */
```

### decoy: 0x00718f90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00718f90: 16-byte aligned: YES
  00718f90  55             PUSH EBP
  00718f91  8B EC          MOV EBP,ESP
  00718f93  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00718f90
string literals (0):
direct callees (5):
  0x007335F0  CExoString::CExoString_copy
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717820  CResGFF::GetDataField
  0x00733680  CExoString::Constructor_4
struct offsets touched (top 0):
```

## Card B0-4-49

GOG function: CVirtualMachine::StackPopInteger  gog 0x00640bc0  callconv=__thiscall  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fd9a0 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fdaf0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640ba0 CVirtualMachine::SetCommandImplementer -> 0x006fd980  gate_class=REVIEW
- GOG-order neighbour after:  0x00640be0 CVirtualMachine::StackPushInteger -> 0x006fd9c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00645e00 CVirtualMachineInternal::StackPopInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fd9a0: 16-byte aligned: YES
  006fd9a0  55             PUSH EBP
  006fd9a1  8B EC          MOV EBP,ESP
  006fd9a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fd9a0
string literals (0):
direct callees (1):
  0x00702B60  CVirtualMachineInternal::StackPopInteger
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fd9a0 FUN_006fd9a0 size=28 ret=4 callers=299 callees=1
THUNK -> 0x00702B60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fd9a0(undefined4 param_1)

{
  FUN_00702b60(param_1);
  return;
}
```

### decoy: 0x006fdaf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fdaf0: 16-byte aligned: YES
  006fdaf0  55             PUSH EBP
  006fdaf1  8B EC          MOV EBP,ESP
  006fdaf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fdaf0
string literals (0):
direct callees (1):
  0x00703070  CVirtualMachineInternal::StackPopObject
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-4-50

GOG function: CVirtualMachine::StackPopVector  gog 0x00640c40  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fda20 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fd9c0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640c20 CVirtualMachine::StackPushFloat -> 0x006fda00  gate_class=REVIEW
- GOG-order neighbour after:  0x00640c60 CVirtualMachine::StackPushVector -> 0x006fda40  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00645fa0 CVirtualMachineInternal::StackPopVector -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fda20: 16-byte aligned: YES
  006fda20  55             PUSH EBP
  006fda21  8B EC          MOV EBP,ESP
  006fda23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fda20
string literals (0):
direct callees (1):
  0x00702D00  CVirtualMachineInternal::StackPopVector
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fda20 FUN_006fda20 size=28 ret=4 callers=14 callees=1
THUNK -> 0x00702D00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fda20(undefined4 param_1)

{
  FUN_00702d00(param_1);
  return;
}
```

### decoy: 0x006fd9c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fd9c0: 16-byte aligned: YES
  006fd9c0  55             PUSH EBP
  006fd9c1  8B EC          MOV EBP,ESP
  006fd9c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fd9c0
string literals (0):
direct callees (1):
  0x00702BE0  CVirtualMachineInternal::StackPushInteger
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-4-51

GOG function: CVirtualMachine::StackPopFloat  gog 0x00640c00  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fd9e0 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fd9c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640be0 CVirtualMachine::StackPushInteger -> 0x006fd9c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00640c20 CVirtualMachine::StackPushFloat -> 0x006fda00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00645ed0 CVirtualMachineInternal::StackPopFloat -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fd9e0: 16-byte aligned: YES
  006fd9e0  55             PUSH EBP
  006fd9e1  8B EC          MOV EBP,ESP
  006fd9e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fd9e0
string literals (0):
direct callees (1):
  0x00702C30  CVirtualMachineInternal::StackPopFloat
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fd9e0 FUN_006fd9e0 size=28 ret=4 callers=43 callees=1
THUNK -> 0x00702C30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fd9e0(undefined4 param_1)

{
  FUN_00702c30(param_1);
  return;
}
```

### decoy: 0x006fd9c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fd9c0: 16-byte aligned: YES
  006fd9c0  55             PUSH EBP
  006fd9c1  8B EC          MOV EBP,ESP
  006fd9c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fd9c0
string literals (0):
direct callees (1):
  0x00702BE0  CVirtualMachineInternal::StackPushInteger
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-4-52

GOG function: CExoResMan::Exists_2  gog 0x0061b890  callconv=(blank)  param_size_bytes=12  gog_size=150  gog_retn=12
Candidate Steam address: 0x00711f30 (size 150, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00711ed0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061b830 CExoResMan::Exists -> 0x00711ed0  gate_class=AUTO
- GOG-order neighbour after:  0x0061b940 CExoResMan::Free -> 0x00711fe0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061bc20 CExoResMan::GetKeyEntryFromTable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x3*,0xc*,0x10*,0x14*,0x18*,0x1c*} | Steam {0x3*,0xc*,0x10*,0x14*,0x18*,0x1c*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00711f30: 16-byte aligned: YES
  00711f30  55             PUSH EBP
  00711f31  8B EC          MOV EBP,ESP
  00711f33  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711f30
string literals (0):
direct callees (1):
  0x007122C0  CExoResMan::GetKeyEntryFromTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00711f30 FUN_00711f30 size=150 ret=12 callers=1 callees=1

undefined4 __thiscall
FUN_00711f30(int param_1,undefined4 param_2,undefined2 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  switch(param_4) {
  case 1:
    local_10 = param_1 + 0x10;
    break;
  case 2:
    local_10 = param_1 + 0x14;
    break;
  case 3:
    local_10 = param_1 + 0x18;
    break;
  case 4:
    local_10 = param_1 + 0x1c;
  }
  if (local_10 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_007122c0(local_10,param_2,param_3,&local_c,&local_8,0);
  }
  return uVar1;
}
```

### decoy: 0x00711ed0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711ed0: 16-byte aligned: YES
  00711ed0  55             PUSH EBP
  00711ed1  8B EC          MOV EBP,ESP
  00711ed3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711ed0
string literals (0):
direct callees (1):
  0x00712360  CExoResMan::GetKeyEntry
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EDX
```

## Card B0-4-53

GOG function: CResGFF::WriteFieldFLOAT  gog 0x00625e40  callconv=(blank)  param_size_bytes=12  gog_size=96  gog_retn=12
Candidate Steam address: 0x00719ce0 (size 96, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00719c50:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625db0 CResGFF::WriteFieldDWORD64 -> 0x00719c50  gate_class=AUTO
- GOG-order neighbour after:  0x00625ea0 CResGFF::WriteFieldCResRef -> 0x00719d40  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam 0x00717aa0  in candidate callees: yes
  0x00624190 CResGFF::AddField -> Steam 0x00718030  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x8*,0x98*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00719ce0: 16-byte aligned: YES
  00719ce0  55             PUSH EBP
  00719ce1  8B EC          MOV EBP,ESP
  00719ce3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00719ce0
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00719ce0 FUN_00719ce0 size=96 ret=12 callers=26 callees=2

undefined4 __thiscall FUN_00719ce0(int param_1,int param_2,undefined4 param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,8);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(undefined4 *)(iVar2 + 8) = param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00719c50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719c50: 16-byte aligned: YES
  00719c50  55             PUSH EBP
  00719c51  8B EC          MOV EBP,ESP
  00719c53  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719c50
string literals (0):
direct callees (3):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
  0x00718370  CResGFF::AddDataField
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x5C [dec 92]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-4-54

GOG function: CVirtualMachine::RunScriptSituation  gog 0x00640b50  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x006fd930 (size 36, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fd8d0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x00640af0 CVirtualMachine::RunScript -> 0x006fd8d0  gate_class=AUTO
- GOG-order neighbour after:  0x00640b80 CVirtualMachine::GetRunScriptReturnValue -> 0x006fd960  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00645830 CVirtualMachineInternal::RunScriptSituation -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fd930: 16-byte aligned: YES
  006fd930  55             PUSH EBP
  006fd931  8B EC          MOV EBP,ESP
  006fd933  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fd930
string literals (0):
direct callees (1):
  0x00702590  CVirtualMachineInternal::RunScriptSituation
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006fd930 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=10 callees=1
THUNK -> 0x00702590 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00702590(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x006fd8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fd8d0: 16-byte aligned: YES
  006fd8d0  55             PUSH EBP
  006fd8d1  8B EC          MOV EBP,ESP
  006fd8d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fd8d0
string literals (1):
  0x0099B5F4  at 006fd8f6  'a_createhk47'
direct callees (3):
  0x00734010  CExoString::CStr
  0x0091D6F0  (unnamed)
  0x007022A0  CVirtualMachineInternal::RunScript
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-4-55

GOG function: CSWSItem::GetPropertyByTypeExists  gog 0x00667600  callconv=(blank)  param_size_bytes=4  gog_size=245  gog_retn=4
Candidate Steam address: 0x00603b50 (size 574, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItem::GetUsedActivePropertyUsesLeft
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00603dd0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00667380 CSWSItem::GetUsedActivePropertyUsesLeft -> 0x00603b50  gate_class=AUTO
- GOG-order neighbour after:  0x00667710 CSWSItem::RestoreUsedActiveProperties -> 0x00603ee0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00667a90 CSWSItem::GetActiveProperty -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x6*,0xc*,0x10*,0x288} | Steam {0x6*,0xc*,0x10*,0x298,0x2cc}
imm sets (* = in both, 5 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8,0xa*,0x20,0xff} | Steam {0x2*,0x3*,0x4*,0x5*,0xa*,0x11,0x5c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00603b50: 16-byte aligned: YES
  00603b50  55             PUSH EBP
  00603b51  8B EC          MOV EBP,ESP
  00603b53  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00603b50
string literals (0):
direct callees (1):
  0x00604260  CSWSItem::GetActiveProperty
struct offsets touched (top 6):
  +0x298 [dec 664]  x10  via EAX,ECX,EDX
  +0xC [dec 12]  x8  via EAX,EDX
  +0x10 [dec 16]  x2  via EDX
  +0x6 [dec 6]  x1  via EDX
  +0x2CC [dec 716]  x1  via EDX
  +0x603DB4 [dec 6307252]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00603b50 FUN_00603b50 size=574 ret=4 callers=4 callees=1

ushort __thiscall FUN_00603b50(int param_1,undefined1 param_2)

{
  short *psVar1;
  ushort local_54;
  byte local_18;
  ushort local_10;
  
  local_10 = 0;
  psVar1 = (short *)FUN_00604260(param_2);
  if ((psVar1 != (short *)0x0) && (*psVar1 == 10)) {
    switch(psVar1[3]) {
    case 1:
      if ((char)psVar1[6] != '\0') {
        local_18 = (byte)*(undefined4 *)(param_1 + 0x2cc);
        local_10 = (ushort)local_18;
      }
      break;
    case 2:
      if (((char)psVar1[6] != '\0') && (0 < *(int *)(param_1 + 0x298))) {
        local_10 = (ushort)(*(int *)(param_1 + 0x298) / 5);
      }
      break;
    case 3:
      if (((char)psVar1[6] != '\0') && (0 < *(int *)(param_1 + 0x298))) {
        local_10 = (ushort)((int)(*(int *)(param_1 + 0x298) +
                                 (*(int *)(param_1 + 0x298) >> 0x1f & 3U)) >> 2);
      }
      break;
    case 4:
      if (((char)psVar1[6] != '\0') && (0 < *(int *)(param_1 + 0x298))) {
        local_10 = (ushort)(*(int *)(param_1 + 0x298) / 3);
      }
      break;
    case 5:
      if (((char)psVar1[6] != '\0') && (0 < *(int *)(param_1 + 0x298))) {
        local_10 = (ushort)(*(int *)(param_1 + 0x298) / 2);
      }
      break;
    case 6:
      if (((char)psVar1[6] != '\0') && (0 < *(int *)(param_1 + 0x298))) {
        local_54 = (ushort)*(undefined4 *)(param_1 + 0x298);
        local_10 = local_54;
      }
      break;
    case 8:
    case 9:
    case 10:
    case 0xb:
    case 0xc:
      if (((char)psVar1[6] != '\0') && ((char)psVar1[8] != '\0')) {
        local_10 = (ushort)*(byte *)(psVar1 + 0x8 [dec 8] {short*: byte +0x10});
      }
      break;
    case 0xe:
    case 0xf:
    case 0x10:
    case 0x11:
    case 0x12:
      if ((char)psVar1[6] != '\0') {
        local_10 = 1;
      }
    }
  }
  return local_10;
}
```

### decoy: 0x00603dd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00603dd0: 16-byte aligned: YES
  00603dd0  55             PUSH EBP
  00603dd1  8B EC          MOV EBP,ESP
  00603dd3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00603dd0
string literals (0):
direct callees (1):
  0x00604260  CSWSItem::GetActiveProperty
struct offsets touched (top 4):
  +0x10 [dec 16]  x6  via EAX,ECX,EDX
  +0x6 [dec 6]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x288 [dec 648]  x1  via ECX
```

## Card B0-4-56

GOG function: CExoResMan::GetResOfType  gog 0x0061bf10  callconv=(blank)  param_size_bytes=8  gog_size=875  gog_retn=8
Candidate Steam address: 0x007125b0 (size 875, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00712470:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061bdd0 CExoResMan::GetResOfTypeFromTable -> 0x00712470  gate_class=AUTO
- GOG-order neighbour after:  0x0061c280 CExoResMan::GetResOfType_2 -> 0x00712920  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0060d090 CExoStringList::Destructor -> Steam ?  in candidate callees: n/a
  0x0060d2f0 CExoStringList::Merge -> Steam ?  in candidate callees: n/a
  0x0061bdd0 CExoResMan::GetResOfTypeFromTable -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0x10*,0x14*,0x18*,0x1c*,0x70*} | Steam {0x4*,0x8*,0x10*,0x14*,0x18*,0x1c*,0x70*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007125b0: 16-byte aligned: YES
  007125b0  55             PUSH EBP
  007125b1  8B EC          MOV EBP,ESP
  007125b3  83 EC 70       SUB ESP,0x70
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007125b0
string literals (0):
direct callees (4):
  0x00712470  CExoResMan::GetResOfTypeFromTable
  0x0073AC70  (unnamed)
  0x0073AA30  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007125b0 FUN_007125b0 size=875 ret=8 callers=1 callees=4

/* WARNING: Removing unreachable block (ram,0x007125ca) */
/* WARNING: Removing unreachable block (ram,0x0071262c) */
/* WARNING: Removing unreachable block (ram,0x00712606) */
/* WARNING: Removing unreachable block (ram,0x00712633) */
/* WARNING: Removing unreachable block (ram,0x0071266b) */
/* WARNING: Removing unreachable block (ram,0x00712645) */
/* WARNING: Removing unreachable block (ram,0x00712672) */

int __thiscall FUN_007125b0(int param_1,undefined2 param_2,int param_3)

{
  int iVar1;
  int iVar2;
  int local_10;
  
  iVar2 = FUN_00712470(param_1 + 0x18,param_2);
  if (param_3 == 0) {
    if (iVar2 == 0) {
      local_10 = FUN_00712470(param_1 + 0x1c,param_2);
    }
    else {
      iVar1 = FUN_00712470(param_1 + 0x1c,param_2);
      local_10 = FUN_0073ac70(iVar1,0,1);
      if (iVar2 != 0) {
        FUN_0073aa30();
        FUN_009196fd(iVar2);
      }
      if (iVar1 != 0) {
        FUN_0073aa30();
        FUN_009196fd(iVar1);
      }
    }
    if (local_10 == 0) {
      local_10 = FUN_00712470(param_1 + 0x14,param_2);
    }
    else {
      iVar2 = FUN_00712470(param_1 + 0x14,param_2);
      iVar1 = FUN_0073ac70(iVar2,0,1);
      if (local_10 != 0) {
        FUN_0073aa30();
        FUN_009196fd(local_10);
      }
      local_10 = iVar1;
      if (iVar2 != 0) {
        FUN_0073aa30();
        FUN_009196fd(iVar2);
      }
    }
    if (local_10 == 0) {
      iVar2 = FUN_00712470(param_1 + 0x10,param_2);
    }
    else {
      iVar1 = FUN_00712470(param_1 + 0x10,param_2);
      iVar2 = FUN_0073ac70(iVar1,0,1);
      if (local_10 != 0) {
        FUN_0073aa30();
        FUN_009196fd(local_10);
      }
      if (iVar1 != 0) {
        FUN_0073aa30();
        FUN_009196fd(iVar1);
      }
    }
  }
  return iVar2;
}
```

### decoy: 0x00712470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00712470: 16-byte aligned: YES
  00712470  55             PUSH EBP
  00712471  8B EC          MOV EBP,ESP
  00712473  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00712470
string literals (0):
direct callees (7):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00726790  (unnamed)
  0x0073AC70  (unnamed)
  0x0073AA30  (unnamed)
  0x009196FD  operator delete
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-4-57

GOG function: CExoResMan::FileCopy  gog 0x0061f0a0  callconv=(blank)  param_size_bytes=14  gog_size=404  gog_retn=16
Candidate Steam address: 0x00715740 (size 404, retn 16)
Candidate sources: branch+name_map:high:gog_cexoresman_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007158e0:STR0/CALLEE0/SIG0.045/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061f070 CExoResMan::NukeDirectory -> 0x00715710  gate_class=REVIEW
- GOG-order neighbour after:  0x0061f240 CExoResMan::RemoveFile -> 0x007158e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00605ae0 CExoString::operator== -> Steam ?  in candidate callees: n/a
  0x0060b880 CExoFile::Constructor -> Steam ?  in candidate callees: n/a
  0x0060b910 CExoFile::Destructor -> Steam ?  in candidate callees: n/a
  0x0060b970 CExoFile::FileOpened -> Steam ?  in candidate callees: n/a
  0x0060b990 CExoFile::Flush -> Steam ?  in candidate callees: n/a
  0x0060ba00 CExoFile::GetSize -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x5*,0x10*,0x30*,0xffff*} | Steam {0x2*,0x3*,0x4*,0x5*,0x10*,0x30*,0xffff*}

### abi
```
GOG param bytes: 14; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(14/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 14 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x00715740: 16-byte aligned: YES
  00715740  55             PUSH EBP
  00715741  8B EC          MOV EBP,ESP
  00715743  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00715740
string literals (0):
direct callees (12):
  0x007339D0  CExoString::operator==
  0x00733570  CExoString::CExoString_char
  0x007397F0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007398E0  (unnamed)
  0x00739880  (unnamed)
  0x00739970  (unnamed)
  0x00919723  operator new
  0x00739990  (unnamed)
  0x00739A30  (unnamed)
  0x00739900  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00715740 FUN_00715740 size=404 ret=16 callers=2 callees=12

undefined4 FUN_00715740(undefined4 param_1,undefined4 param_2,short param_3,short param_4)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 extraout_var;
  undefined1 local_2c [8];
  undefined1 local_24 [16];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095fc70;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_007339d0(param_2);
  if (iVar2 == 0) {
    if (param_4 == -1) {
      param_4 = param_3;
    }
    uVar3 = extraout_var;
    FUN_00733570(&DAT_0098bedc);
    local_8 = 0;
    FUN_007397f0(param_1,param_3,local_24);
    local_8._0_1_ = 2;
    FUN_00733780(uVar1,uVar3);
    FUN_00733570(&DAT_0098ba1c);
    local_8._0_1_ = 3;
    FUN_007397f0(param_2,param_4,local_2c);
    local_8._0_1_ = 5;
    FUN_00733780();
    iVar2 = FUN_007398e0();
    if ((iVar2 != 0) && (iVar2 = FUN_007398e0(), iVar2 != 0)) {
      uVar3 = FUN_00739970();
      local_14 = FUN_00919723(uVar3);
      uVar4 = 1;
      uVar3 = FUN_00739970(1);
      FUN_00739990(local_14,uVar3,uVar4);
      uVar4 = 1;
      uVar3 = FUN_00739970(1);
      FUN_00739a30(local_14,uVar3,uVar4);
      FUN_00739900();
      FUN_009196fd(local_14);
      local_8 = CONCAT31(local_8._1_3_,2);
      FUN_00739880();
      local_8 = 0xffffffff;
      FUN_00739880();
      ExceptionList = local_10;
      return 1;
    }
    local_8 = CONCAT31(local_8._1_3_,2);
    FUN_00739880();
    local_8 = 0xffffffff;
    FUN_00739880();
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x007158e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007158e0: 16-byte aligned: YES
  007158e0  55             PUSH EBP
  007158e1  8B EC          MOV EBP,ESP
  007158e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007158e0
string literals (0):
direct callees (9):
  0x00733540  CExoString::CExoString(empty)
  0x007397B0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00919F10  _strlen
  0x00734010  CExoString::CStr
  0x00920EB6  (unnamed)
  0x009464EE  (unnamed)
  0x00920DAC  (unnamed)
struct offsets touched (top 1):
  +0xC [dec 12]  x1  via EAX
```

## Card B0-4-58

GOG function: CExoResMan::GetResRefFromFile  gog 0x0061c540  callconv=(blank)  param_size_bytes=8  gog_size=175  gog_retn=8
Candidate Steam address: 0x00712be0 (size 175, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_cexoresman_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00712c90:STR0/CALLEE2/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061c4c0 CExoResMan::SetResObject -> 0x00712b60  gate_class=AUTO
- GOG-order neighbour after:  0x0061c5f0 CExoResMan::GetResTypeFromFile -> 0x00712c90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006062a0 CExoString::Find_2 -> Steam ?  in candidate callees: n/a
  0x00606b10 CExoString::SubString -> Steam 0x00734a00  in candidate callees: yes
  0x0061a710 CResRef::operator=_3 -> Steam ?  in candidate callees: n/a
  0x0061a800 CResRef::operator=_4 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0x10*,0x2e*,0x38*} | Steam {0x8*,0x10*,0x2e*,0x38*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00712be0: 16-byte aligned: YES
  00712be0  55             PUSH EBP
  00712be1  8B EC          MOV EBP,ESP
  00712be3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00712be0
string literals (0):
direct callees (5):
  0x00734190  CExoString::Find_2
  0x00710EB0  (unnamed)
  0x00734A00  CExoString::SubString
  0x00710DC0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00712be0 FUN_00712be0 size=175 ret=8 callers=1 callees=5

void FUN_00712be0(void)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  undefined1 local_3c [16];
  undefined1 local_2c [8];
  undefined1 local_24 [16];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00950378;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = FUN_00734190(0x2e,0);
  if (local_14 < 1) {
    FUN_00710eb0(local_24,&DAT_00986925);
  }
  else {
    if (0x10 < local_14) {
      local_14 = 0x10;
    }
    uVar4 = extraout_var;
    uVar2 = FUN_00734a00(local_2c,0,local_14);
    local_8 = 0;
    uVar3 = uVar2;
    FUN_00710dc0(local_3c,uVar2);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1,uVar2,uVar3,uVar4);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00712c90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00712c90: 16-byte aligned: YES
  00712c90  55             PUSH EBP
  00712c91  8B EC          MOV EBP,ESP
  00712c93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00712c90
string literals (0):
direct callees (6):
  0x00733540  CExoString::CExoString(empty)
  0x00734190  CExoString::Find_2
  0x00733780  CExoString::~CExoString
  0x00734A00  CExoString::SubString
  0x007337C0  CExoString::operator=
  0x007360C0  (unnamed)
struct offsets touched (top 0):
```

## Card B0-4-59

GOG function: CResGFF::ReadFieldCHAR  gog 0x00624be0  callconv=(blank)  param_size_bytes=13  gog_size=122  gog_retn=16
Candidate Steam address: 0x00718a80 (size 122, retn 16)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00718a00:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00624b60 CResGFF::ReadFieldBYTE -> 0x00718a00  gate_class=REVIEW
- GOG-order neighbour after:  0x00624c60 CResGFF::ReadFieldWORD -> 0x00718b00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x00623a40 CResGFF::GetFieldByLabel -> Steam 0x007178e0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}

### abi
```
GOG param bytes: 13; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(13/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 13 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x00718a80: 16-byte aligned: YES
  00718a80  55             PUSH EBP
  00718a81  8B EC          MOV EBP,ESP
  00718a83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00718a80
string literals (0):
direct callees (2):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00718a80 FUN_00718a80 size=122 ret=16 callers=5 callees=2

undefined1 FUN_00718a80(int param_1,int param_2,undefined4 *param_3,undefined1 param_4)

{
  undefined4 uVar1;
  int *piVar2;
  
  if ((param_1 == 0) || (param_2 == 0)) {
    *param_3 = 0;
  }
  else {
    uVar1 = FUN_007178e0(param_1,param_2);
    piVar2 = (int *)FUN_00717770(param_1,uVar1);
    if ((piVar2 == (int *)0x0) || (*piVar2 != 1)) {
      *param_3 = 0;
    }
    else {
      *param_3 = 1;
      param_4 = (undefined1)piVar2[2 = byte +0x8];
    }
  }
  return param_4;
}
```

### decoy: 0x00718a00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00718a00: 16-byte aligned: YES
  00718a00  55             PUSH EBP
  00718a01  8B EC          MOV EBP,ESP
  00718a03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00718a00
string literals (0):
direct callees (2):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-4-60

GOG function: CSWSEncounter::Destructor  gog 0x007e94f0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x006232b0 (size 98, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00623060:STR0/CALLEE0/SIG0.05/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e94d0 CSWSEncounter::GetFirstName -> 0x00623010  gate_class=REVIEW
- GOG-order neighbour after:  0x007e9520 CSWSEncounter::Destructor_2 -> 0x00623060  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007e9520 CSWSEncounter::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x4*,0x28}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006232b0: 16-byte aligned: YES
  006232b0  55             PUSH EBP
  006232b1  8B EC          MOV EBP,ESP
  006232b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006232b0
string literals (0):
direct callees (2):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006232b0 FUN_006232b0 size=98 ret=4 callers=4 callees=2

void * __thiscall FUN_006232b0(void *param_1,uint param_2)

{
  if ((param_2 & 2) == 0) {
    if ((param_2 & 1) != 0) {
      FUN_009196fd(param_1);
    }
  }
  else {
    _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1,0x28,*(int *)((int)param_1 + -4),FUN_00623320);
    if ((param_2 & 1) != 0) {
      FUN_009196fd((int)param_1 + -4);
    }
    param_1 = (void *)((int)param_1 + -4);
  }
  return param_1;
}
```

### decoy: 0x00623060 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00623060: 16-byte aligned: YES
  00623060  55             PUSH EBP
  00623061  8B EC          MOV EBP,ESP
  00623063  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00623060
string literals (0):
direct callees (12):
  0x006239D0  CSWSEncounter::RemoveFromArea
  0x0051C280  (unnamed)
  0x0051E0C0  CServerAIMaster::RemoveObject
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x0055EA80  (unnamed)
  0x006232B0  (unnamed)
  0x009196FD  operator delete
  0x00628840  CSWSEncounter::ClearSpawnList
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00735360  CExoLocString::Destructor
  0x0053F050  CSWSObject::Destructor_2
struct offsets touched (top 8):
  +0x2C4 [dec 708]  x3  via EAX,ECX
  +0x2D0 [dec 720]  x3  via EAX,EDX
  +0x2D8 [dec 728]  x3  via ECX,EDX
  +0x2F0 [dec 752]  x3  via ECX,EDX
  +0x2F8 [dec 760]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via EAX
  +0x350 [dec 848]  x1  via EDX
```
