# Review cards B0 chunk 13

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-13-01

GOG function: Gob::DisableForceAnimations  gog 0x0084c360  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x00459d80 (size 18, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459d60:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c360 Global::DisableForceAnimations -> 0x00459d80  gate_class=REVIEW
- GOG-order neighbour after:  0x0084c380 Gob::IsForceAnimated -> 0x00459da0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}
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
prologue for 0x00459d80: 16-byte aligned: YES
  00459d80  55             PUSH EBP
  00459d81  8B EC          MOV EBP,ESP
  00459d83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459d80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459d80 FUN_00459d80 size=18 ret=0 callers=0 callees=0

void __fastcall FUN_00459d80(int param_1)

{
  *(undefined1 *)(param_1 + 0x10) = 0;
  return;
}
```

### decoy: 0x00459d60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459d60: 16-byte aligned: YES
  00459d60  55             PUSH EBP
  00459d61  8B EC          MOV EBP,ESP
  00459d63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459d60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

## Card B0-13-02

GOG function: Gob::TurnOnShadows  gog 0x0084be50  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x00459870 (size 26, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459850:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084be30 Gob::TurnOffShadows -> 0x00459850  gate_class=REVIEW
- GOG-order neighbour after:  0x0084be70 Gob::IsAnimating -> 0x00459890  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xee*} | Steam {0xee*}
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
prologue for 0x00459870: 16-byte aligned: YES
  00459870  55             PUSH EBP
  00459871  8B EC          MOV EBP,ESP
  00459873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459870
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xEE [dec 238]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00459870 FUN_00459870 size=26 ret=0 callers=0 callees=0

void __fastcall FUN_00459870(int param_1)

{
  *(undefined2 *)(param_1 + 0xee) = 1;
  return;
}
```

### decoy: 0x00459850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459850: 16-byte aligned: YES
  00459850  55             PUSH EBP
  00459851  8B EC          MOV EBP,ESP
  00459853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459850
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xEE [dec 238]  x1  via ECX
```

## Card B0-13-03

GOG function: Gob::EnableFog  gog 0x0084c570  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00459f90 (size 31, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459f70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c550 Gob::DisableFog -> 0x00459f70  gate_class=REVIEW
- GOG-order neighbour after:  0x0084c590 Gob::GetBoundingBox -> 0x00459fb0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0084c4e0 Global::PartSetFog -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x98*} | Steam {0x98*}
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
prologue for 0x00459f90: 16-byte aligned: YES
  00459f90  55             PUSH EBP
  00459f91  8B EC          MOV EBP,ESP
  00459f93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459f90
string literals (0):
direct callees (1):
  0x00459F00  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459f90 FUN_00459f90 size=31 ret=0 callers=0 callees=1
THUNK -> 0x00459F00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00459f90(int param_1)

{
  FUN_00459f00(*(undefined4 *)(param_1 + 0x98),1);
  return;
}
```

### decoy: 0x00459f70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459f70: 16-byte aligned: YES
  00459f70  55             PUSH EBP
  00459f71  8B EC          MOV EBP,ESP
  00459f73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459f70
string literals (0):
direct callees (1):
  0x00459F00  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-13-04

GOG function: ConsoleFunc::Destructor  gog 0x00867920  callconv=(blank)  param_size_bytes=0  gog_size=72  gog_retn=0
Candidate Steam address: 0x00475a70 (size 53, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00475a00:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00867880 ConsoleFunc::Constructor_3 -> 0x00475910  gate_class=REVIEW
- GOG-order neighbour after:  0x00867a20 Global::AurResGet  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00866f10 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x50} | Steam {0x8}
imm sets (* = in both, 1 common): GOG {0x4*,0x10} | Steam {0x4*,0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00475a70: 16-byte aligned: YES
  00475a70  55             PUSH EBP
  00475a71  8B EC          MOV EBP,ESP
  00475a73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00475a70
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00475a70 FUN_00475a70 size=53 ret=0 callers=1 callees=1

void __fastcall FUN_00475a70(int param_1)

{
  if (*(int *)(param_1 + 8) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 8));
    *(undefined4 *)(param_1 + 8) = 0;
  }
  return;
}
```

### decoy: 0x00475a00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475a00: 16-byte aligned: YES
  00475a00  55             PUSH EBP
  00475a01  8B EC          MOV EBP,ESP
  00475a03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00475a00
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via EDX
```

## Card B0-13-05

GOG function: CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionIncrease  gog 0x007c9840  callconv=__thiscall  param_size_bytes=12  gog_size=293  gog_retn=20
Candidate Steam address: 0x005b2650 (size 293, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionDecrease
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b2560:STR0/CALLEE0/SIG0.286/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9750 CSWSItemPropertyHandler::RemoveRegeneration -> 0x005b2340  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9970 CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionIncrease -> 0x005b2560  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*}
imm sets (* = in both, 5 common): GOG {0x4*,0x14*,0x2c*,0x8c*,0xfffffff8*} | Steam {0x4*,0x14*,0x2c*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b2650: 16-byte aligned: YES
  005b2650  55             PUSH EBP
  005b2651  8B EC          MOV EBP,ESP
  005b2653  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b2650
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2650 FUN_005b2650 size=293 ret=20 callers=0 callees=5

undefined4
FUN_005b2650(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  short sVar1;
  int iVar2;
  int local_3c;
  undefined2 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095315b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  sVar1 = *(short *)(param_2 + 6);
  if (sVar1 != 0) {
    iVar2 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar2 == 0) {
      local_3c = 0;
    }
    else {
      local_3c = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_28 = (undefined2)DAT_0099608c;
    *(undefined2 *)(local_3c + 8) = local_28;
    *(ushort *)(local_3c + 0xa [dec 10]) = *(ushort *)(local_3c + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,*(undefined2 *)(param_2 + 2));
    FUN_005e4e80(1,sVar1);
    FUN_00544210(local_3c,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b2560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2560: 16-byte aligned: YES
  005b2560  55             PUSH EBP
  005b2561  8B EC          MOV EBP,ESP
  005b2563  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2560
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

## Card B0-13-06

GOG function: Gob::GetPartLocalPosition  gog 0x00855260  callconv=(blank)  param_size_bytes=12  gog_size=121  gog_retn=12
Candidate Steam address: 0x00462cd0 (size 121, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00462b90:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00855260 Global::GetPartLocalPosition -> 0x00462cd0  gate_class=REVIEW
- GOG-order neighbour after:  0x008552e0 Gob::ReattachReference  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x10c*} | Steam {0x4*,0x8*,0xc*,0x10c*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0x14*} | Steam {0x8*,0xc*,0x14*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00462cd0: 16-byte aligned: YES
  00462cd0  55             PUSH EBP
  00462cd1  8B EC          MOV EBP,ESP
  00462cd3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00462cd0
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x10C [dec 268]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00462cd0 FUN_00462cd0 size=121 ret=12 callers=0 callees=0

undefined4 __thiscall
FUN_00462cd0(int *param_1,undefined4 param_2,undefined4 *param_3,undefined4 *param_4)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = (**(code **)(*param_1 + 0x10c))(param_2);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    if (param_3 != (undefined4 *)0x0) {
      *param_3 = *(undefined4 *)(iVar1 + 8);
      param_3[1 = byte +0x4] = *(undefined4 *)(iVar1 + 0xc);
      param_3[2 = byte +0x8] = *(undefined4 *)(iVar1 + 0x10);
    }
    if (param_4 != (undefined4 *)0x0) {
      *param_4 = *(undefined4 *)(iVar1 + 0x14);
      param_4[1 = byte +0x4] = *(undefined4 *)(iVar1 + 0x18);
      param_4[2 = byte +0x8] = *(undefined4 *)(iVar1 + 0x1c);
      param_4[3 = byte +0xc] = *(undefined4 *)(iVar1 + 0x20);
    }
    uVar2 = 1;
  }
  return uVar2;
}
```

### decoy: 0x00462b90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00462b90: 16-byte aligned: YES
  00462b90  55             PUSH EBP
  00462b91  8B EC          MOV EBP,ESP
  00462b93  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00462b90
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x10C [dec 268]  x1  via EDX
```

## Card B0-13-07

GOG function: Gob::EnableDistortion  gog 0x0084bfd0  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x004599f0 (size 21, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459a10:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bfb0 Gob::GetMaxBlurLength -> 0x004599d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0084bff0 Gob::DisableDistortion -> 0x00459a10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1a8*} | Steam {0x1a8*}
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
prologue for 0x004599f0: 16-byte aligned: YES
  004599f0  55             PUSH EBP
  004599f1  8B EC          MOV EBP,ESP
  004599f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004599f0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1A8 [dec 424]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004599f0 FUN_004599f0 size=21 ret=0 callers=0 callees=0

void __fastcall FUN_004599f0(int param_1)

{
  *(undefined1 *)(param_1 + 0x1a8) = 1;
  return;
}
```

### decoy: 0x00459a10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459a10: 16-byte aligned: YES
  00459a10  55             PUSH EBP
  00459a11  8B EC          MOV EBP,ESP
  00459a13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459a10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1A8 [dec 424]  x1  via EAX
```

## Card B0-13-08

GOG function: Scene::GetIgnoreVisibilityGraph  gog 0x00861da0  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x0046fb60 (size 21, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0046fb40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00861d80 Scene::DisableVisibilityGraph -> 0x0046fb40  gate_class=REVIEW
- GOG-order neighbour after:  0x00861dc0 Scene::SetVisibility  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xe4*} | Steam {0xe4*}
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
prologue for 0x0046fb60: 16-byte aligned: YES
  0046fb60  55             PUSH EBP
  0046fb61  8B EC          MOV EBP,ESP
  0046fb63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0046fb60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE4 [dec 228]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0046fb60 FUN_0046fb60 size=21 ret=0 callers=0 callees=0
THUNK -> unknown (Ghidra may omit __thiscall on thunks; ECX is passed through)

int __fastcall FUN_0046fb60(int param_1)

{
  return (int)*(short *)(param_1 + 0xe4);
}
```

### decoy: 0x0046fb40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0046fb40: 16-byte aligned: YES
  0046fb40  55             PUSH EBP
  0046fb41  8B EC          MOV EBP,ESP
  0046fb43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0046fb40
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE4 [dec 228]  x1  via ECX
```

## Card B0-13-09

GOG function: Gob::IsInCutscene  gog 0x0084c400  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00459e20 (size 17, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459e00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c3e0 Gob::DisableInCutscene -> 0x00459e00  gate_class=REVIEW
- GOG-order neighbour after:  0x0084c420 Gob::GetCutsceneDummyPosition -> 0x00459e40  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}
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
prologue for 0x00459e20: 16-byte aligned: YES
  00459e20  55             PUSH EBP
  00459e21  8B EC          MOV EBP,ESP
  00459e23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459e20
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459e20 FUN_00459e20 size=17 ret=0 callers=5 callees=0

undefined1 __fastcall FUN_00459e20(int param_1)

{
  return *(undefined1 *)(param_1 + 0x14);
}
```

### decoy: 0x00459e00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459e00: 16-byte aligned: YES
  00459e00  55             PUSH EBP
  00459e01  8B EC          MOV EBP,ESP
  00459e03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459e00
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via EAX
```

## Card B0-13-10

GOG function: Scene::RemoveFromSceneManager_2  gog 0x008646a0  callconv=(blank)  param_size_bytes=4  gog_size=71  gog_retn=0
Candidate Steam address: 0x00472460 (size 71, retn 0)
Candidate sources: branch+name_map:medium:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004723e0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00864620 Scene::RemoveFromSceneManager  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008646f0 Global::AurSetGlobalWind  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008730c0 List_T__::Remove -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00472460: 16-byte aligned: YES
  00472460  55             PUSH EBP
  00472461  8B EC          MOV EBP,ESP
  00472463  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00472460
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00472460 FUN_00472460 size=71 ret=0 callers=1 callees=2

void FUN_00472460(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_00476620(param_1);
  if (iVar1 == 0) {
    iVar1 = FUN_00476620(param_1);
    if (iVar1 != 0) {
      FUN_00449170(param_1);
    }
  }
  else {
    FUN_00449170(param_1);
  }
  return;
}
```

### decoy: 0x004723e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004723e0: 16-byte aligned: YES
  004723e0  55             PUSH EBP
  004723e1  8B EC          MOV EBP,ESP
  004723e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004723e0
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 2):
  +0x34 [dec 52]  x1  via EDX
  +0x54 [dec 84]  x1  via EDX
```

## Card B0-13-11

GOG function: CSWCArea::StopWeather  gog 0x00456230  callconv=(blank)  param_size_bytes=0  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a98d0 (size 116, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCArea::ApplyWeatherVisual
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a9860:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00456200 CSWCArea::StartWeather -> 0x007a9800  gate_class=REVIEW
- GOG-order neighbour after:  0x00456260 CSWCArea::SetLightningDone -> 0x007a9860  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x1f0,0x1f5} | Steam {0x4c,0x188}
imm sets (* = in both, 1 common): GOG {0x4*,0x1770} | Steam {0x2,0x4*,0xc}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x007a98d0: 16-byte aligned: YES
  007a98d0  55             PUSH EBP
  007a98d1  8B EC          MOV EBP,ESP
  007a98d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a98d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x188 [dec 392]  x3  via EAX,EDX
  +0x4C [dec 76]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007a98d0 FUN_007a98d0 size=116 ret=4 callers=1 callees=0

void __thiscall FUN_007a98d0(int param_1,char param_2)

{
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x188) != 0) {
    if (param_2 == '\0') {
      local_8 = DAT_0098ba8c;
    }
    else if (param_2 == '\x01') {
      local_8 = DAT_0098ba90;
    }
    else if (param_2 == '\x02') {
      local_8 = DAT_0098ba94;
    }
    (**(code **)(**(int **)(param_1 + 0x188) + 0x4c))(local_8);
  }
  return;
}
```

### decoy: 0x007a9860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a9860: 16-byte aligned: YES
  007a9860  55             PUSH EBP
  007a9861  8B EC          MOV EBP,ESP
  007a9863  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a9860
string literals (0):
direct callees (1):
  0x0091EB8B  (unnamed)
struct offsets touched (top 3):
  +0x1A8 [dec 424]  x3  via EAX,ECX
  +0x4C [dec 76]  x1  via EDX
  +0x1E4 [dec 484]  x1  via EDX
```

## Card B0-13-12

GOG function: Gob::AddAttachment  gog 0x008557b0  callconv=(blank)  param_size_bytes=4  gog_size=55  gog_retn=4
Candidate Steam address: 0x004632c0 (size 55, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00463300:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008556b0 Gob::CollectLights_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008557f0 Gob::RemoveAttachment -> 0x00463300  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x1e8*} | Steam {0x4*,0x1e8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004632c0: 16-byte aligned: YES
  004632c0  55             PUSH EBP
  004632c1  8B EC          MOV EBP,ESP
  004632c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004632c0
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004632c0 FUN_004632c0 size=55 ret=4 callers=1 callees=2

void FUN_004632c0(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_00476620(param_1);
  if (iVar1 == 0) {
    FUN_00473f40(param_1);
  }
  return;
}
```

### decoy: 0x00463300 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00463300: 16-byte aligned: YES
  00463300  55             PUSH EBP
  00463301  8B EC          MOV EBP,ESP
  00463303  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00463300
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 0):
```

## Card B0-13-13

GOG function: Gob::EnableInViewVolume  gog 0x0084c5e0  callconv=(blank)  param_size_bytes=0  gog_size=41  gog_retn=0
Candidate Steam address: 0x0045a000 (size 41, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR  (content level C1)
- evidence summary: STR 'propagateviewvolume'@0098b988
- best decoy (neighbour/other candidate) scores: 0x00459fb0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c590 Gob::GetBoundingBox -> 0x00459fb0  gate_class=AUTO
- GOG-order neighbour after:  0x0084c610 Gob::LoadAddInAnimations -> 0x0045a030  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (1): 'propagateviewvolume'
disp sets (* = in both, 2 common): GOG {0x11*,0x1b4*} | Steam {0x11*,0x1b4*}
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
prologue for 0x0045a000: 16-byte aligned: YES
  0045a000  55             PUSH EBP
  0045a001  8B EC          MOV EBP,ESP
  0045a003  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0045a000
string literals (1):
  0x0098B988  at 0045a010  'propagateviewvolume'
direct callees (0):
struct offsets touched (top 2):
  +0x11 [dec 17]  x1  via EAX
  +0x1B4 [dec 436]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0045a000 FUN_0045a000 size=41 ret=0 callers=0 callees=0

void __fastcall FUN_0045a000(int *param_1)

{
  *(undefined1 *)((int)param_1 + 0x11) = 1;
  (**(code **)(*param_1 + 0x1b4))("propagateviewvolume",0);
  return;
}
```

### decoy: 0x00459fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459fb0: 16-byte aligned: YES
  00459fb0  55             PUSH EBP
  00459fb1  8B EC          MOV EBP,ESP
  00459fb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459fb0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
```

## Card B0-13-14

GOG function: Scene::EnableAnimations  gog 0x00857590  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x004651b0 (size 21, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004651d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857150 Scene::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008575b0 Scene::DisableAnimations -> 0x004651d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xe6*} | Steam {0xe6*}
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
prologue for 0x004651b0: 16-byte aligned: YES
  004651b0  55             PUSH EBP
  004651b1  8B EC          MOV EBP,ESP
  004651b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004651b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE6 [dec 230]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004651b0 FUN_004651b0 size=21 ret=0 callers=0 callees=0

void __fastcall FUN_004651b0(int param_1)

{
  *(undefined1 *)(param_1 + 0xe6) = 1;
  return;
}
```

### decoy: 0x004651d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004651d0: 16-byte aligned: YES
  004651d0  55             PUSH EBP
  004651d1  8B EC          MOV EBP,ESP
  004651d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004651d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE6 [dec 230]  x1  via EAX
```

## Card B0-13-15

GOG function: Gob::GetModelName  gog 0x00853b60  callconv=(blank)  param_size_bytes=0  gog_size=39  gog_retn=0
Candidate Steam address: 0x004615d0 (size 39, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00461580:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00853b10 Gob::HitCheckHack  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00853b90 Global::SetBehavior  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x84*} | Steam {0x84*}
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
prologue for 0x004615d0: 16-byte aligned: YES
  004615d0  55             PUSH EBP
  004615d1  8B EC          MOV EBP,ESP
  004615d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004615d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x84 [dec 132]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004615d0 FUN_004615d0 size=39 ret=0 callers=0 callees=0

int __fastcall FUN_004615d0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x84) == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = *(int *)(param_1 + 0x84) + 8;
  }
  return iVar1;
}
```

### decoy: 0x00461580 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00461580: 16-byte aligned: YES
  00461580  55             PUSH EBP
  00461581  8B EC          MOV EBP,ESP
  00461583  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00461580
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x8C [dec 140]  x1  via EAX
```

## Card B0-13-16

GOG function: Gob::SetMotionBlurred  gog 0x0084bf50  callconv=(blank)  param_size_bytes=1  gog_size=25  gog_retn=4
Candidate Steam address: 0x00459970 (size 25, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459950:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bf30 Gob::SetRenderPersonalSpace -> 0x00459950  gate_class=REVIEW
- GOG-order neighbour after:  0x0084bf70 Gob::GetMotionBlurred -> 0x00459990  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x181*} | Steam {0x181*}
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
prologue for 0x00459970: 16-byte aligned: YES
  00459970  55             PUSH EBP
  00459971  8B EC          MOV EBP,ESP
  00459973  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459970
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x181 [dec 385]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459970 FUN_00459970 size=25 ret=4 callers=0 callees=0

void __thiscall FUN_00459970(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x181) = param_2;
  return;
}
```

### decoy: 0x00459950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459950: 16-byte aligned: YES
  00459950  55             PUSH EBP
  00459951  8B EC          MOV EBP,ESP
  00459953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459950
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x180 [dec 384]  x1  via EAX
```

## Card B0-13-17

GOG function: Scene::SetBlindLights  gog 0x008575f0  callconv=(blank)  param_size_bytes=4  gog_size=30  gog_retn=4
Candidate Steam address: 0x00465210 (size 30, retn 4)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00465230:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008575d0 Scene::IsAnimating -> 0x004651f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00857610 Global::FindScene  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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
prologue for 0x00465210: 16-byte aligned: YES
  00465210  55             PUSH EBP
  00465211  8B EC          MOV EBP,ESP
  00465213  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00465210
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00465210 FUN_00465210 size=30 ret=4 callers=0 callees=0

void FUN_00465210(undefined4 param_1)

{
  DAT_00a7be44 = param_1;
  return;
}
```

### decoy: 0x00465230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00465230: 16-byte aligned: YES
  00465230  55             PUSH EBP
  00465231  8B EC          MOV EBP,ESP
  00465233  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00465230
string literals (0):
direct callees (1):
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

## Card B0-13-18

GOG function: Scene::GetFogRange  gog 0x00857030  callconv=(blank)  param_size_bytes=8  gog_size=35  gog_retn=8
Candidate Steam address: 0x00464c50 (size 35, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00464c80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857010 Scene::SetFog -> 0x00464c30  gate_class=REVIEW
- GOG-order neighbour after:  0x00857060 Scene::SetFogRange -> 0x00464c80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x64*,0x68*} | Steam {0x64*,0x68*}
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
prologue for 0x00464c50: 16-byte aligned: YES
  00464c50  55             PUSH EBP
  00464c51  8B EC          MOV EBP,ESP
  00464c53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00464c50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x64 [dec 100]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00464c50 FUN_00464c50 size=35 ret=8 callers=0 callees=0

void __thiscall FUN_00464c50(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  *param_2 = *(undefined4 *)(param_1 + 0x64 [dec 100]);
  *param_3 = *(undefined4 *)(param_1 + 0x68);
  return;
}
```

### decoy: 0x00464c80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00464c80: 16-byte aligned: YES
  00464c80  55             PUSH EBP
  00464c81  8B EC          MOV EBP,ESP
  00464c83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00464c80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x64 [dec 100]  x1  via EAX
  +0x68 [dec 104]  x1  via ECX
```

## Card B0-13-19

GOG function: Scene::DisableVisibilityGraph  gog 0x00861d80  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x0046fb40 (size 26, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0046fb60:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00861d50 Scene::EnableVisibilityGraph -> 0x0046fb10  gate_class=REVIEW
- GOG-order neighbour after:  0x00861da0 Scene::GetIgnoreVisibilityGraph -> 0x0046fb60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xe4*} | Steam {0xe4*}
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
prologue for 0x0046fb40: 16-byte aligned: YES
  0046fb40  55             PUSH EBP
  0046fb41  8B EC          MOV EBP,ESP
  0046fb43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0046fb40
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE4 [dec 228]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0046fb40 FUN_0046fb40 size=26 ret=0 callers=0 callees=0

void __fastcall FUN_0046fb40(int param_1)

{
  *(undefined2 *)(param_1 + 0xe4) = 1;
  return;
}
```

### decoy: 0x0046fb60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0046fb60: 16-byte aligned: YES
  0046fb60  55             PUSH EBP
  0046fb61  8B EC          MOV EBP,ESP
  0046fb63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0046fb60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE4 [dec 228]  x1  via EAX
```

## Card B0-13-20

GOG function: CSWSScriptVarTable::DestroyString  gog 0x007f07d0  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x005e6f60 (size 27, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSScriptVarTable::DestroyObject
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e6f40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f07b0 CSWSScriptVarTable::DestroyFloat -> 0x005e6f20  gate_class=REVIEW
- GOG-order neighbour after:  0x007f07f0 CSWSScriptVarTable::DestroyObject -> 0x005e6f60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007effe0 CSWSScriptVarTable::DeleteIndex -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x3,0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005e6f60: 16-byte aligned: YES
  005e6f60  55             PUSH EBP
  005e6f61  8B EC          MOV EBP,ESP
  005e6f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e6f60
string literals (0):
direct callees (1):
  0x005E6750  CSWSScriptVarTable::DeleteIndex
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e6f60 FUN_005e6f60 size=27 ret=4 callers=1 callees=1
THUNK -> 0x005E6750 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_005e6f60(undefined4 param_1)

{
  FUN_005e6750(param_1,4);
  return;
}
```

### decoy: 0x005e6f40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e6f40: 16-byte aligned: YES
  005e6f40  55             PUSH EBP
  005e6f41  8B EC          MOV EBP,ESP
  005e6f43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e6f40
string literals (0):
direct callees (1):
  0x005E6750  CSWSScriptVarTable::DeleteIndex
struct offsets touched (top 0):
```

## Card B0-13-21

GOG function: Gob::EnableRenderBumpedOut  gog 0x0084e530  callconv=(blank)  param_size_bytes=8  gog_size=1343  gog_retn=8
Candidate Steam address: 0x0045bf50 (size 1343, retn 8)
Candidate sources: branch+mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CAurGob::ApplyTXIToNamedTexture
- anchors: A-STR 2 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'punchthrough'@0098b9b8; 'additive'@0098b9c8; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0045c490:STR0/CALLEE0/SIG0.304/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084e510 Global::PartHasMaterial  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084ea70 Gob::DisableRenderBumpedOut -> 0x0045c490  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (17), mapped to Steam through trusted rows only:
  0x00457b60 CExoArrayList_CSWCMapPin__::Allocate -> Steam ?  in candidate callees: n/a
  0x00819be0 Global::AurTextureGetReference -> Steam ?  in candidate callees: n/a
  0x00819da0 Global::AurLiteReleaseFunctionHolder -> Steam ?  in candidate callees: n/a
  0x00856330 CExoArrayList::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x008660c0 SafePointer_T_::operator= -> Steam ?  in candidate callees: n/a
  0x00867a20 Global::AurResGet -> Steam ?  in candidate callees: n/a
  0x00867e10 Global::AurResFree -> Steam ?  in candidate callees: n/a
  0x00867ee0 Global::AurResGetNextLine -> Steam ?  in candidate callees: n/a
  ... 9 more GOG callees
GOG strings (3): 'punchthrough'; 'additive'; 'blending'
disp sets (* = in both, 11 common): GOG {0xc*,0x15*,0x28*,0x34*,0x44*,0x94*,0x98*,0xa4*,0xa8*,0x110*,0x148*} | Steam {0xc*,0x15*,0x28*,0x34*,0x44*,0x94*,0x98*,0xa4*,0xa8*,0x110*,0x148*}
imm sets (* = in both, 12 common): GOG {0x2*,0x3*,0x4*,0x8*,0x9*,0xa*,0xc*,0x10*,0x20*,0x28*,0x34*,0x207c*} | Steam {0x2*,0x3*,0x4*,0x8*,0x9*,0xa*,0xc*,0x10*,0x20*,0x28*,0x34*,0x207c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0045bf50: 16-byte aligned: YES
  0045bf50  55             PUSH EBP
  0045bf51  8B EC          MOV EBP,ESP
  0045bf53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0045bf50
string literals (8):
  0x0098B9D4  at 0045c0d4  'blending'
  0x0098B9D4  at 0045c0e9  'blending'
  0x0098B9C8  at 0045c10a  'additive'
  0x0098B9B8  at 0045c12f  'punchthrough'
  0x0098B9D4  at 0045c26f  'blending'
  0x0098B9D4  at 0045c284  'blending'
  0x0098B9C8  at 0045c2a5  'additive'
  0x0098B9B8  at 0045c2ca  'punchthrough'
direct callees (17):
  0x0091E050  (unnamed)
  0x00463ED0  (unnamed)
  0x004274E0  Global::AurTextureGetReference
  0x00475AB0  (unnamed)
  0x00463B20  (unnamed)
  0x004762F0  ModelFile::ReadBlock(n, src)
  0x00475F70  (unnamed)
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x00484DC0  (unnamed)
  0x0091D780  (unnamed)
  0x00475EA0  CloseModelStream
  0x004741E0  SafePointer_T_::operator=
  0x004276A0  ReleaseTextureProxy (delete if back-ref count 0)
  0x00463BA0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x44 [dec 68]  x8  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x34 [dec 52]  x2  via EAX,EDX
  +0xA8 [dec 168]  x2  via EDX
  +0x15 [dec 21]  x1  via EAX
  +0x28 [dec 40]  x1  via EAX
  +0x94 [dec 148]  x1  via EDX
  +0x98 [dec 152]  x1  via EDX
  +0xA4 [dec 164]  x1  via EAX
  +0x110 [dec 272]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0045bf50 FUN_0045bf50 size=1343 ret=8 callers=0 callees=17

/* WARNING: Function: __alloca_probe replaced with injection: alloca_probe */

void __thiscall FUN_0045bf50(int *param_1,undefined4 param_2,int param_3)

{
  char cVar1;
  uint uVar2;
  undefined4 uVar3;
  char *pcVar4;
  int iVar5;
  size_t sVar6;
  int iVar7;
  int local_2074;
  char *local_2070;
  char local_2068 [8196];
  int local_64;
  int local_60;
  undefined4 local_5c;
  char local_58 [32];
  uint local_38;
  int local_34;
  undefined4 local_30;
  int local_2c;
  int *local_28;
  char *local_24;
  int local_20;
  int local_1c;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00949ee8;
  local_10 = ExceptionList;
  local_14 = 0x45bf6b;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *(undefined1 *)((int)param_1 + 0x15) = 1;
  local_38 = uVar2;
  FUN_00463ed0(0);
  local_8 = 0;
  (**(code **)(*param_1 + 0x110))(FUN_0045bf30,&local_20,uVar2);
  local_2c = 0;
  local_34 = 0;
  local_58[0] = '\0';
  local_28 = (int *)FUN_004274e0(param_2,0);
  local_5c = 0;
  local_30 = 1;
  cVar1 = (**(code **)(*local_28 + 0x34))();
  if (cVar1 == '\0') {
    local_60 = FUN_00475ab0(param_2,&DAT_009883e0,&local_2c,1);
    if (local_60 == 0) {
      (**(code **)(*local_28 + 0xa8))(1);
      local_8 = 0xffffffff;
      FUN_00463b20();
      goto LAB_0045c474;
    }
    if (local_2c < 0) {
      local_34 = -1 - local_2c;
    }
    else {
      local_34 = local_2c;
    }
    if (0 < local_34) {
      uVar3 = FUN_004762f0(0,local_60,local_34);
      (**(code **)(*local_28 + 0xa4))(uVar3);
      while (local_24 = (char *)FUN_00475f70(), local_24 != (char *)0x0) {
        for (; (*local_24 == ' ' || (*local_24 == '\t')); local_24 = local_24 + 1) {
        }
        pcVar4 = (char *)FUN_00484d10(local_24);
        iVar5 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("blending",pcVar4);
        if (iVar5 == 0) {
          pcVar4 = local_58;
          sVar6 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("blending");
          FUN_00484dc0(local_24 + sVar6,pcVar4);
        }
        if (local_58[0] != '\0') {
          iVar5 = _strcmp [GHIDRA-AUTO-LABEL: unreliable](local_58,"additive");
/* ... 93 more lines truncated by --max-lines */
```

### decoy: 0x0045c490 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0045c490: 16-byte aligned: YES
  0045c490  55             PUSH EBP
  0045c491  8B EC          MOV EBP,ESP
  0045c493  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0045c490
string literals (0):
direct callees (4):
  0x00463ED0  (unnamed)
  0x004741E0  SafePointer_T_::operator=
  0x004276A0  ReleaseTextureProxy (delete if back-ref count 0)
  0x00463B20  (unnamed)
struct offsets touched (top 5):
  +0x15 [dec 21]  x2  via EAX,EDX
  +0x44 [dec 68]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EDX
  +0x110 [dec 272]  x1  via EDX
  +0x148 [dec 328]  x1  via EDX
```

## Card B0-13-22

GOG function: Scene::ChildRemove  gog 0x00858000  callconv=(blank)  param_size_bytes=4  gog_size=111  gog_retn=4
Candidate Steam address: 0x00465c20 (size 112, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00465bb0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857f90 Scene::ChildAdd -> 0x00465bb0  gate_class=REVIEW
- GOG-order neighbour after:  0x00858070 Global::ProjectDynamicShadow  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00857ec0 Scene::UnlistSpecialParts -> Steam ?  in candidate callees: n/a
  0x0086d8c0 Global::SceneBSPRemoveGob -> Steam ?  in candidate callees: n/a
  0x008730c0 List_T__::Remove -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x84*,0x98*,0x1a8*} | Steam {0x84*,0x98*,0x1a8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x54*,0x10c*} | Steam {0x4*,0x8*,0x54*,0x10c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00465c20: 16-byte aligned: YES
  00465c20  55             PUSH EBP
  00465c21  8B EC          MOV EBP,ESP
  00465c23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00465c20
string literals (0):
direct callees (3):
  0x00495FB0  (unnamed)
  0x00465AE0  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 3):
  +0x84 [dec 132]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
  +0x1A8 [dec 424]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00465c20 FUN_00465c20 size=112 ret=4 callers=0 callees=3

void __thiscall FUN_00465c20(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  if (param_2[0x21 = byte +0x84] != 0) {
    FUN_00495fb0(param_1,param_2);
    FUN_00465ae0(param_2[0x26 = byte +0x98]);
  }
  FUN_00449170(param_2);
  iVar1 = (**(code **)(*param_2 + 0x1a8))();
  if (iVar1 != 0) {
    FUN_00449170(param_2);
  }
  return;
}
```

### decoy: 0x00465bb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00465bb0: 16-byte aligned: YES
  00465bb0  55             PUSH EBP
  00465bb1  8B EC          MOV EBP,ESP
  00465bb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00465bb0
string literals (0):
direct callees (3):
  0x00495F20  (unnamed)
  0x004659F0  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
struct offsets touched (top 3):
  +0x84 [dec 132]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
  +0x1A8 [dec 424]  x1  via EDX
```

## Card B0-13-23

GOG function: Scene::SetFogRange  gog 0x00857060  callconv=(blank)  param_size_bytes=8  gog_size=31  gog_retn=8
Candidate Steam address: 0x00464c80 (size 31, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00464c50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857030 Scene::GetFogRange -> 0x00464c50  gate_class=REVIEW
- GOG-order neighbour after:  0x00857080 Scene::SetSceneFocus -> 0x00464ca0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x64*,0x68*} | Steam {0x64*,0x68*}
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
prologue for 0x00464c80: 16-byte aligned: YES
  00464c80  55             PUSH EBP
  00464c81  8B EC          MOV EBP,ESP
  00464c83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00464c80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x64 [dec 100]  x1  via EAX
  +0x68 [dec 104]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00464c80 FUN_00464c80 size=31 ret=8 callers=0 callees=0

void __thiscall FUN_00464c80(int param_1,undefined4 param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 0x64 [dec 100]) = param_2;
  *(undefined4 *)(param_1 + 0x68) = param_3;
  return;
}
```

### decoy: 0x00464c50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00464c50: 16-byte aligned: YES
  00464c50  55             PUSH EBP
  00464c51  8B EC          MOV EBP,ESP
  00464c53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00464c50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x64 [dec 100]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
```

## Card B0-13-24

GOG function: Gob::DisableAlwaysRender  gog 0x0084c3c0  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x00459de0 (size 18, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459dc0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c3a0 Gob::EnableAlwaysRender -> 0x00459dc0  gate_class=REVIEW
- GOG-order neighbour after:  0x0084c3e0 Gob::DisableInCutscene -> 0x00459e00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x12*} | Steam {0x12*}
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
prologue for 0x00459de0: 16-byte aligned: YES
  00459de0  55             PUSH EBP
  00459de1  8B EC          MOV EBP,ESP
  00459de3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459de0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x12 [dec 18]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459de0 FUN_00459de0 size=18 ret=0 callers=0 callees=0

void __fastcall FUN_00459de0(int param_1)

{
  *(undefined1 *)(param_1 + 0x12) = 0;
  return;
}
```

### decoy: 0x00459dc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459dc0: 16-byte aligned: YES
  00459dc0  55             PUSH EBP
  00459dc1  8B EC          MOV EBP,ESP
  00459dc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459dc0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x12 [dec 18]  x1  via EAX
```

## Card B0-13-25

GOG function: Gob::DisableRenderGobBBox  gog 0x0084e4f0  callconv=(blank)  param_size_bytes=0  gog_size=23  gog_retn=0
Candidate Steam address: 0x0045bf10 (size 23, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0045bed0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084e4b0 Gob::EnableRenderGobBBox -> 0x0045bed0  gate_class=AUTO
- GOG-order neighbour after:  0x0084e510 Global::PartHasMaterial  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xf4*} | Steam {0xf4*}
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
prologue for 0x0045bf10: 16-byte aligned: YES
  0045bf10  55             PUSH EBP
  0045bf11  8B EC          MOV EBP,ESP
  0045bf13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0045bf10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF4 [dec 244]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0045bf10 FUN_0045bf10 size=23 ret=0 callers=0 callees=0

void __fastcall FUN_0045bf10(int param_1)

{
  *(undefined2 *)(param_1 + 0xf4) = 0;
  return;
}
```

### decoy: 0x0045bed0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0045bed0: 16-byte aligned: YES
  0045bed0  55             PUSH EBP
  0045bed1  8B EC          MOV EBP,ESP
  0045bed3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0045bed0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xF4 [dec 244]  x1  via ECX
```

## Card B0-13-26

GOG function: Scene::ChildAdd  gog 0x00857f90  callconv=__thiscall  param_size_bytes=4  gog_size=112  gog_retn=4
Candidate Steam address: 0x00465bb0 (size 112, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00465c20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857ec0 Scene::UnlistSpecialParts  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00858000 Scene::ChildRemove -> 0x00465c20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00857dd0 Scene::ListSpecialPartsR -> Steam ?  in candidate callees: n/a
  0x0086d830 Global::SceneBSPAddGob -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x84*,0x98*,0x1a8*} | Steam {0x84*,0x98*,0x1a8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x54*,0x10c*} | Steam {0x4*,0x8*,0x54*,0x10c*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00465bb0: 16-byte aligned: YES
  00465bb0  55             PUSH EBP
  00465bb1  8B EC          MOV EBP,ESP
  00465bb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00465bb0
string literals (0):
direct callees (3):
  0x00495F20  (unnamed)
  0x004659F0  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
struct offsets touched (top 3):
  +0x84 [dec 132]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
  +0x1A8 [dec 424]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00465bb0 FUN_00465bb0 size=112 ret=4 callers=0 callees=3

void __thiscall FUN_00465bb0(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  if (param_2[0x21 = byte +0x84] != 0) {
    FUN_00495f20(param_1,param_2);
    FUN_004659f0(param_2[0x26 = byte +0x98]);
  }
  FUN_00473f40(param_2);
  iVar1 = (**(code **)(*param_2 + 0x1a8))();
  if (iVar1 != 0) {
    FUN_00473f40(param_2);
  }
  return;
}
```

### decoy: 0x00465c20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00465c20: 16-byte aligned: YES
  00465c20  55             PUSH EBP
  00465c21  8B EC          MOV EBP,ESP
  00465c23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00465c20
string literals (0):
direct callees (3):
  0x00495FB0  (unnamed)
  0x00465AE0  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 3):
  +0x84 [dec 132]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
  +0x1A8 [dec 424]  x1  via EDX
```

## Card B0-13-27

GOG function: Scene::SetCurrentRoom_2  gog 0x008621f0  callconv=(blank)  param_size_bytes=4  gog_size=72  gog_retn=4
Candidate Steam address: 0x0046ffb0 (size 72, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0046ff30:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008621d0 Scene::SetCurrentRoom  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00862240 Scene::SpawnRooms  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0xd4*,0xd8*,0xe0*} | Steam {0xd4*,0xd8*,0xe0*}
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
prologue for 0x0046ffb0: 16-byte aligned: YES
  0046ffb0  55             PUSH EBP
  0046ffb1  8B EC          MOV EBP,ESP
  0046ffb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0046ffb0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0xE0 [dec 224]  x2  via ECX
  +0xD4 [dec 212]  x1  via EDX
  +0xD8 [dec 216]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0046ffb0 FUN_0046ffb0 size=72 ret=4 callers=0 callees=0

void __thiscall FUN_0046ffb0(int param_1,int param_2)

{
  if ((param_2 < 0) || (*(int *)(param_1 + 0xd8) <= param_2)) {
    *(undefined4 *)(param_1 + 0xe0) = 0;
  }
  else {
    *(undefined4 *)(param_1 + 0xe0) = *(undefined4 *)(*(int *)(param_1 + 0xd4) + param_2 * 4);
  }
  return;
}
```

### decoy: 0x0046ff30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0046ff30: 16-byte aligned: YES
  0046ff30  55             PUSH EBP
  0046ff31  8B EC          MOV EBP,ESP
  0046ff33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0046ff30
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xD4 [dec 212]  x1  via EDX
  +0xD8 [dec 216]  x1  via EAX
```

## Card B0-13-28

GOG function: Gob::RemoveAttachment  gog 0x008557f0  callconv=(blank)  param_size_bytes=4  gog_size=53  gog_retn=4
Candidate Steam address: 0x00463300 (size 53, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004632c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008557b0 Gob::AddAttachment -> 0x004632c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00855830 Gob::SetObjectScale -> 0x00463340  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008730c0 List_T__::Remove -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x1e8*} | Steam {0x4*,0x1e8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00463300: 16-byte aligned: YES
  00463300  55             PUSH EBP
  00463301  8B EC          MOV EBP,ESP
  00463303  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00463300
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00463300 FUN_00463300 size=53 ret=4 callers=1 callees=2

void FUN_00463300(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_00476620(param_1);
  if (iVar1 != 0) {
    FUN_00449170(param_1);
  }
  return;
}
```

### decoy: 0x004632c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004632c0: 16-byte aligned: YES
  004632c0  55             PUSH EBP
  004632c1  8B EC          MOV EBP,ESP
  004632c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004632c0
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
struct offsets touched (top 0):
```

## Card B0-13-29

GOG function: Gob::IsAlwaysRendering  gog 0x0084c4c0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00459ee0 (size 17, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459e40:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c420 Gob::GetCutsceneDummyPosition -> 0x00459e40  gate_class=AUTO
- GOG-order neighbour after:  0x0084c4e0 Global::PartSetFog  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x12*} | Steam {0x12*}
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
prologue for 0x00459ee0: 16-byte aligned: YES
  00459ee0  55             PUSH EBP
  00459ee1  8B EC          MOV EBP,ESP
  00459ee3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459ee0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x12 [dec 18]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459ee0 FUN_00459ee0 size=17 ret=0 callers=0 callees=0

undefined1 __fastcall FUN_00459ee0(int param_1)

{
  return *(undefined1 *)(param_1 + 0x12);
}
```

### decoy: 0x00459e40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459e40: 16-byte aligned: YES
  00459e40  55             PUSH EBP
  00459e41  8B EC          MOV EBP,ESP
  00459e43  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459e40
string literals (0):
direct callees (1):
  0x00456B00  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via ECX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-13-30

GOG function: CGuiInGame::IsAnimationPlayingInDialog  gog 0x004dbde0  callconv=(blank)  param_size_bytes=0  gog_size=55  gog_retn=0
Candidate Steam address: 0x007cb5e0 (size 190, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::UnlockDialogParticipantOrientations
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cb540:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbda0 CGuiInGame::IsCameraDialog -> 0x007cb6a0  gate_class=REVIEW
- GOG-order neighbour after:  0x004dbe20 CGuiInGame::IsLoopingDialogStuntAnimation_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x3c,0x64,0x84,0xfb4} | Steam {0x4,0x2c,0x190,0x194}
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
  0x0073F4D0  ClientApp::GetClientObject
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

## Card B0-13-31

GOG function: CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionIncrease  gog 0x007c9840  callconv=__thiscall  param_size_bytes=12  gog_size=293  gog_retn=20
Candidate Steam address: 0x005b21c0 (size 369, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplyRegeneration
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.69 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.69
- best decoy (neighbour/other candidate) scores: 0x005b2340:STR0/CALLEE0/SIG0.286/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9750 CSWSItemPropertyHandler::RemoveRegeneration -> 0x005b2340  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9970 CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionIncrease -> 0x005b2560  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x2,0x6*,0x8*,0xa*,0x14*} | Steam {0x6*,0x8*,0xa*,0x14*}
imm sets (* = in both, 4 common): GOG {0x4*,0x14*,0x2c,0x8c*,0xfffffff8*} | Steam {0x4*,0x14*,0x34,0x8c*,0x1770,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b21c0: 16-byte aligned: YES
  005b21c0  55             PUSH EBP
  005b21c1  8B EC          MOV EBP,ESP
  005b21c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b21c0
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 4):
  +0xA [dec 10]  x2  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b21c0 FUN_005b21c0 size=369 ret=20 callers=0 callees=7

undefined4
FUN_005b21c0(int param_1,undefined2 *param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

{
  short sVar1;
  int iVar2;
  int local_40;
  undefined2 local_34;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a0eb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar2 == 0) {
    local_40 = 0;
  }
  else {
    local_40 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  sVar1 = param_2[3];
  if (sVar1 == 0) {
    if (local_40 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(local_40);
    }
  }
  else {
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    local_34 = (undefined2)DAT_00995f38;
    *(undefined2 *)(local_40 + 8) = local_34;
    *(ushort *)(local_40 + 0xa [dec 10]) = *(ushort *)(local_40 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e4e80(0,sVar1);
    FUN_005e4e80(1,6000);
    FUN_005e4e80(4,*param_2);
    FUN_00544210(local_40,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b2340 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2340: 16-byte aligned: YES
  005b2340  55             PUSH EBP
  005b2341  8B EC          MOV EBP,ESP
  005b2343  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2340
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

## Card B0-13-32

GOG function: Scene::SetObjectScale  gog 0x008647f0  callconv=(blank)  param_size_bytes=8  gog_size=46  gog_retn=8
Candidate Steam address: 0x004725b0 (size 46, retn 8)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004723e0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00864770 Global::AurSetGlobalWindVariation  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00864820 Scene::SetGeomFadeProperties -> 0x004725e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xcc*} | Steam {0xcc*}
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
prologue for 0x004725b0: 16-byte aligned: YES
  004725b0  55             PUSH EBP
  004725b1  8B EC          MOV EBP,ESP
  004725b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004725b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xCC [dec 204]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004725b0 FUN_004725b0 size=46 ret=8 callers=0 callees=0

void __thiscall FUN_004725b0(undefined4 param_1,int *param_2,undefined4 param_3)

{
  (**(code **)(*param_2 + 0xcc))(param_3,1,param_1);
  return;
}
```

### decoy: 0x004723e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004723e0: 16-byte aligned: YES
  004723e0  55             PUSH EBP
  004723e1  8B EC          MOV EBP,ESP
  004723e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004723e0
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 2):
  +0x34 [dec 52]  x1  via EDX
  +0x54 [dec 84]  x1  via EDX
```

## Card B0-13-33

GOG function: Gob::RemoveAddInAnimations  gog 0x0084c730  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x0045a150 (size 54, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0045a030:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c610 Gob::LoadAddInAnimations -> 0x0045a030  gate_class=AUTO
- GOG-order neighbour after:  0x0084c770 Gob::Destructor_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00845300 Model::DelRef -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x90*} | Steam {0x90*}
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
prologue for 0x0045a150: 16-byte aligned: YES
  0045a150  55             PUSH EBP
  0045a151  8B EC          MOV EBP,ESP
  0045a153  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0045a150
string literals (0):
direct callees (1):
  0x00452CF0  CAurModel::Release (refcount `+0x54`)
struct offsets touched (top 1):
  +0x90 [dec 144]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0045a150 FUN_0045a150 size=54 ret=0 callers=0 callees=1

void __fastcall FUN_0045a150(int param_1)

{
  if (*(int *)(param_1 + 0x90) != 0) {
    FUN_00452cf0(param_1);
  }
  *(undefined4 *)(param_1 + 0x90) = 0;
  return;
}
```

### decoy: 0x0045a030 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0045a030: 16-byte aligned: YES
  0045a030  55             PUSH EBP
  0045a031  8B EC          MOV EBP,ESP
  0045a033  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0045a030
string literals (0):
direct callees (8):
  0x0048E160  Global::FindModel
  0x0091CFF0  (unnamed)
  0x0091D000  (unnamed)
  0x0091CAEB  (unnamed)
  0x0048E2A0  Global::LoadModel_2
  0x0044B440  (unnamed)
  0x00452D40  CAurModel::AddRef
  0x009196EE  __security_check_cookie
struct offsets touched (top 4):
  +0x90 [dec 144]  x5  via ECX,EDX
  +0x84 [dec 132]  x4  via EAX,ECX
  +0x58 [dec 88]  x1  via ECX
  +0x5C [dec 92]  x1  via EDX
```

## Card B0-13-34

GOG function: CSWCArea::StopWeather  gog 0x00456230  callconv=(blank)  param_size_bytes=0  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a9800 (size 38, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCArea::StartWeather
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a9830:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00456200 CSWCArea::StartWeather -> 0x007a9800  gate_class=REVIEW
- GOG-order neighbour after:  0x00456260 CSWCArea::SetLightningDone -> 0x007a9860  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1f0*,0x1f5*} | Steam {0x1f0*,0x1f5*}
imm sets (* = in both, 2 common): GOG {0x4*,0x1770*} | Steam {0x4*,0x1770*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
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

## Card B0-13-35

GOG function: Gob::SetIgnoreHitCheck  gog 0x0084be10  callconv=(blank)  param_size_bytes=1  gog_size=25  gog_retn=4
Candidate Steam address: 0x00459830 (size 25, retn 4)
Candidate sources: branch+name_map:medium:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459810:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bdf0 Gob::SetExternalDataType  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084be30 Gob::TurnOffShadows -> 0x00459850  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1a9*} | Steam {0x1a9*}
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
prologue for 0x00459830: 16-byte aligned: YES
  00459830  55             PUSH EBP
  00459831  8B EC          MOV EBP,ESP
  00459833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459830
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1A9 [dec 425]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459830 FUN_00459830 size=25 ret=4 callers=0 callees=0

void __thiscall FUN_00459830(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x1a9) = param_2;
  return;
}
```

### decoy: 0x00459810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459810: 16-byte aligned: YES
  00459810  55             PUSH EBP
  00459811  8B EC          MOV EBP,ESP
  00459813  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459810
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1DC [dec 476]  x1  via EAX
```

## Card B0-13-36

GOG function: Gob::IsInViewVolume  gog 0x0084beb0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x004598d0 (size 17, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004598b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084be90 Gob::DisableInViewVolume -> 0x004598b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0084bed0 Gob::SetAsCharacter -> 0x004598f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x11*} | Steam {0x11*}
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
prologue for 0x004598d0: 16-byte aligned: YES
  004598d0  55             PUSH EBP
  004598d1  8B EC          MOV EBP,ESP
  004598d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004598d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x11 [dec 17]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004598d0 FUN_004598d0 size=17 ret=0 callers=0 callees=0

undefined1 __fastcall FUN_004598d0(int param_1)

{
  return *(undefined1 *)(param_1 + 0x11);
}
```

### decoy: 0x004598b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004598b0: 16-byte aligned: YES
  004598b0  55             PUSH EBP
  004598b1  8B EC          MOV EBP,ESP
  004598b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004598b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x11 [dec 17]  x1  via EAX
```

## Card B0-13-37

GOG function: Gob::IsForceAnimated  gog 0x0084c380  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00459da0 (size 17, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459d80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c360 Gob::DisableForceAnimations -> 0x00459d80  gate_class=REVIEW
- GOG-order neighbour after:  0x0084c3a0 Gob::EnableAlwaysRender -> 0x00459dc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}
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
prologue for 0x00459da0: 16-byte aligned: YES
  00459da0  55             PUSH EBP
  00459da1  8B EC          MOV EBP,ESP
  00459da3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459da0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459da0 FUN_00459da0 size=17 ret=0 callers=0 callees=0

undefined1 __fastcall FUN_00459da0(int param_1)

{
  return *(undefined1 *)(param_1 + 0x10);
}
```

### decoy: 0x00459d80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459d80: 16-byte aligned: YES
  00459d80  55             PUSH EBP
  00459d81  8B EC          MOV EBP,ESP
  00459d83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459d80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

## Card B0-13-38

GOG function: Gob::IsAnimating  gog 0x0084be70  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00459890 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459870:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084be50 Gob::TurnOnShadows -> 0x00459870  gate_class=REVIEW
- GOG-order neighbour after:  0x0084be90 Gob::DisableInViewVolume -> 0x004598b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1aa*} | Steam {0x1aa*}
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
prologue for 0x00459890: 16-byte aligned: YES
  00459890  55             PUSH EBP
  00459891  8B EC          MOV EBP,ESP
  00459893  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459890
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1AA [dec 426]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459890 FUN_00459890 size=20 ret=0 callers=0 callees=0

undefined1 __fastcall FUN_00459890(int param_1)

{
  return *(undefined1 *)(param_1 + 0x1aa);
}
```

### decoy: 0x00459870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459870: 16-byte aligned: YES
  00459870  55             PUSH EBP
  00459871  8B EC          MOV EBP,ESP
  00459873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459870
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xEE [dec 238]  x1  via ECX
```

## Card B0-13-39

GOG function: Scene::DisableAnimations  gog 0x008575b0  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x004651d0 (size 21, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004651b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857590 Scene::EnableAnimations -> 0x004651b0  gate_class=REVIEW
- GOG-order neighbour after:  0x008575d0 Scene::IsAnimating -> 0x004651f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xe6*} | Steam {0xe6*}
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
prologue for 0x004651d0: 16-byte aligned: YES
  004651d0  55             PUSH EBP
  004651d1  8B EC          MOV EBP,ESP
  004651d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004651d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE6 [dec 230]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004651d0 FUN_004651d0 size=21 ret=0 callers=0 callees=0

void __fastcall FUN_004651d0(int param_1)

{
  *(undefined1 *)(param_1 + 0xe6) = 0;
  return;
}
```

### decoy: 0x004651b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004651b0: 16-byte aligned: YES
  004651b0  55             PUSH EBP
  004651b1  8B EC          MOV EBP,ESP
  004651b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004651b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE6 [dec 230]  x1  via EAX
```

## Card B0-13-40

GOG function: Gob::EnableForceAnimations  gog 0x0084c340  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x00459d60 (size 18, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459d80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c340 Global::EnableForceAnimations -> 0x00459d60  gate_class=REVIEW
- GOG-order neighbour after:  0x0084c360 Global::DisableForceAnimations -> 0x00459d80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}
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
prologue for 0x00459d60: 16-byte aligned: YES
  00459d60  55             PUSH EBP
  00459d61  8B EC          MOV EBP,ESP
  00459d63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459d60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459d60 FUN_00459d60 size=18 ret=0 callers=0 callees=0

void __fastcall FUN_00459d60(int param_1)

{
  *(undefined1 *)(param_1 + 0x10) = 1;
  return;
}
```

### decoy: 0x00459d80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459d80: 16-byte aligned: YES
  00459d80  55             PUSH EBP
  00459d81  8B EC          MOV EBP,ESP
  00459d83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459d80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

## Card B0-13-41

GOG function: ConsoleFunc::Destructor  gog 0x00867920  callconv=(blank)  param_size_bytes=0  gog_size=72  gog_retn=0
Candidate Steam address: 0x004756f0 (size 216, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004757d0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00867880 ConsoleFunc::Constructor_3 -> 0x00475910  gate_class=REVIEW
- GOG-order neighbour after:  0x00867a20 Global::AurResGet  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00866f10 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x50} | Steam {0x8}
imm sets (* = in both, 1 common): GOG {0x4*,0x10} | Steam {0x4*,0x20,0x3d,0x8c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004756f0: 16-byte aligned: YES
  004756f0  55             PUSH EBP
  004756f1  8B EC          MOV EBP,ESP
  004756f3  81 EC 8C 00 00 00 SUB ESP,0x8c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004756f0
string literals (0):
direct callees (2):
  0x00475020  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004756f0 FUN_004756f0 size=216 ret=0 callers=1 callees=2

void FUN_004756f0(char *param_1)

{
  int *piVar1;
  char local_8c [128];
  uint local_c;
  int local_8;
  
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  DAT_00a30320 = 0;
  local_8 = 0;
  for (; ((*param_1 != '\0' && (*param_1 != ' ')) && (*param_1 != '=')); param_1 = param_1 + 1) {
    local_8c[local_8] = *param_1;
    local_8 = local_8 + 1;
  }
  local_8c[local_8] = '\0';
  for (; (*param_1 == '=' || (*param_1 == ' ')); param_1 = param_1 + 1) {
  }
  piVar1 = (int *)FUN_00475020(local_8c);
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 0x8 [dec 8] {int*: byte +0x20}))(param_1);
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x004757d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004757d0: 16-byte aligned: YES
  004757d0  55             PUSH EBP
  004757d1  8B EC          MOV EBP,ESP
  004757d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004757d0
string literals (0):
direct callees (4):
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x00476570  (unnamed)
  0x00474F10  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

## Card B0-13-42

GOG function: Gob::DisableFog  gog 0x0084c550  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00459f70 (size 31, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459f90:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c4e0 Global::PartSetFog  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084c570 Gob::EnableFog -> 0x00459f90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0084c4e0 Global::PartSetFog -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x98*} | Steam {0x98*}
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
prologue for 0x00459f70: 16-byte aligned: YES
  00459f70  55             PUSH EBP
  00459f71  8B EC          MOV EBP,ESP
  00459f73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459f70
string literals (0):
direct callees (1):
  0x00459F00  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459f70 FUN_00459f70 size=31 ret=0 callers=0 callees=1
THUNK -> 0x00459F00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00459f70(int param_1)

{
  FUN_00459f00(*(undefined4 *)(param_1 + 0x98),0);
  return;
}
```

### decoy: 0x00459f90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459f90: 16-byte aligned: YES
  00459f90  55             PUSH EBP
  00459f91  8B EC          MOV EBP,ESP
  00459f93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459f90
string literals (0):
direct callees (1):
  0x00459F00  (unnamed)
struct offsets touched (top 1):
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-13-43

GOG function: Gob::DisableDistortion  gog 0x0084bff0  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x00459a10 (size 21, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004599f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bfd0 Gob::EnableDistortion -> 0x004599f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0084c010 Gob::SetForcePoints  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1a8*} | Steam {0x1a8*}
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
prologue for 0x00459a10: 16-byte aligned: YES
  00459a10  55             PUSH EBP
  00459a11  8B EC          MOV EBP,ESP
  00459a13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459a10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1A8 [dec 424]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459a10 FUN_00459a10 size=21 ret=0 callers=0 callees=0

void __fastcall FUN_00459a10(int param_1)

{
  *(undefined1 *)(param_1 + 0x1a8) = 0;
  return;
}
```

### decoy: 0x004599f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004599f0: 16-byte aligned: YES
  004599f0  55             PUSH EBP
  004599f1  8B EC          MOV EBP,ESP
  004599f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004599f0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1A8 [dec 424]  x1  via EAX
```

## Card B0-13-44

GOG function: Gob::DisableInViewVolume  gog 0x0084be90  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x004598b0 (size 18, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004598d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084be70 Gob::IsAnimating -> 0x00459890  gate_class=REVIEW
- GOG-order neighbour after:  0x0084beb0 Gob::IsInViewVolume -> 0x004598d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x11*} | Steam {0x11*}
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
prologue for 0x004598b0: 16-byte aligned: YES
  004598b0  55             PUSH EBP
  004598b1  8B EC          MOV EBP,ESP
  004598b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004598b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x11 [dec 17]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004598b0 FUN_004598b0 size=18 ret=0 callers=0 callees=0

void __fastcall FUN_004598b0(int param_1)

{
  *(undefined1 *)(param_1 + 0x11) = 0;
  return;
}
```

### decoy: 0x004598d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004598d0: 16-byte aligned: YES
  004598d0  55             PUSH EBP
  004598d1  8B EC          MOV EBP,ESP
  004598d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004598d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x11 [dec 17]  x1  via EAX
```

## Card B0-13-45

GOG function: Gob::TurnOffShadows  gog 0x0084be30  callconv=(blank)  param_size_bytes=0  gog_size=23  gog_retn=0
Candidate Steam address: 0x00459850 (size 23, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459870:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084be10 Gob::SetIgnoreHitCheck -> 0x00459830  gate_class=REVIEW
- GOG-order neighbour after:  0x0084be50 Gob::TurnOnShadows -> 0x00459870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xee*} | Steam {0xee*}
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
prologue for 0x00459850: 16-byte aligned: YES
  00459850  55             PUSH EBP
  00459851  8B EC          MOV EBP,ESP
  00459853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459850
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xEE [dec 238]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00459850 FUN_00459850 size=23 ret=0 callers=0 callees=0

void __fastcall FUN_00459850(int param_1)

{
  *(undefined2 *)(param_1 + 0xee) = 0;
  return;
}
```

### decoy: 0x00459870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459870: 16-byte aligned: YES
  00459870  55             PUSH EBP
  00459871  8B EC          MOV EBP,ESP
  00459873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459870
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xEE [dec 238]  x1  via ECX
```

## Card B0-13-46

GOG function: Gob::DisableInCutscene  gog 0x0084c3e0  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x00459e00 (size 18, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459e20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c3c0 Gob::DisableAlwaysRender -> 0x00459de0  gate_class=REVIEW
- GOG-order neighbour after:  0x0084c400 Gob::IsInCutscene -> 0x00459e20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}
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
prologue for 0x00459e00: 16-byte aligned: YES
  00459e00  55             PUSH EBP
  00459e01  8B EC          MOV EBP,ESP
  00459e03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459e00
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459e00 FUN_00459e00 size=18 ret=0 callers=0 callees=0

void __fastcall FUN_00459e00(int param_1)

{
  *(undefined1 *)(param_1 + 0x14) = 0;
  return;
}
```

### decoy: 0x00459e20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459e20: 16-byte aligned: YES
  00459e20  55             PUSH EBP
  00459e21  8B EC          MOV EBP,ESP
  00459e23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459e20
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via EAX
```

## Card B0-13-47

GOG function: Gob::SetRenderPersonalSpace  gog 0x0084bf30  callconv=(blank)  param_size_bytes=1  gog_size=25  gog_retn=4
Candidate Steam address: 0x00459950 (size 25, retn 4)
Candidate sources: branch+name_map:medium:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459970:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bf10 Gob::GetScene  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084bf50 Gob::SetMotionBlurred -> 0x00459970  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x180*} | Steam {0x180*}
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
prologue for 0x00459950: 16-byte aligned: YES
  00459950  55             PUSH EBP
  00459951  8B EC          MOV EBP,ESP
  00459953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459950
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x180 [dec 384]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459950 FUN_00459950 size=25 ret=4 callers=0 callees=0

void __thiscall FUN_00459950(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x180) = param_2;
  return;
}
```

### decoy: 0x00459970 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459970: 16-byte aligned: YES
  00459970  55             PUSH EBP
  00459971  8B EC          MOV EBP,ESP
  00459973  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459970
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x181 [dec 385]  x1  via EAX
```

## Card B0-13-48

GOG function: Scene::EnableVisibilityGraph  gog 0x00861d50  callconv=(blank)  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x0046fb10 (size 44, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0046fb40:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00861ce0 CAurRoom_Scene::FindRoom  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00861d80 Scene::DisableVisibilityGraph -> 0x0046fb40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xe0*,0xe4*} | Steam {0xe0*,0xe4*}
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
prologue for 0x0046fb10: 16-byte aligned: YES
  0046fb10  55             PUSH EBP
  0046fb11  8B EC          MOV EBP,ESP
  0046fb13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0046fb10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xE0 [dec 224]  x1  via EDX
  +0xE4 [dec 228]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0046fb10 FUN_0046fb10 size=44 ret=0 callers=0 callees=0

bool __fastcall FUN_0046fb10(int param_1)

{
  *(undefined2 *)(param_1 + 0xe4) = 0;
  return *(int *)(param_1 + 0xe0) != 0;
}
```

### decoy: 0x0046fb40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0046fb40: 16-byte aligned: YES
  0046fb40  55             PUSH EBP
  0046fb41  8B EC          MOV EBP,ESP
  0046fb43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0046fb40
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE4 [dec 228]  x1  via ECX
```

## Card B0-13-49

GOG function: Gob::AttachToScene  gog 0x0085f680  callconv=__thiscall  param_size_bytes=4  gog_size=38  gog_retn=4
Candidate Steam address: 0x0046d440 (size 38, retn 4)
Candidate sources: branch+name_map:medium:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0046d230:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0085f680 Global::AttachToScene -> 0x0046d440  gate_class=REVIEW
- GOG-order neighbour after:  0x0085f750 Scene::InitBSP  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00854030 Gob::SetScene_2 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0046d440: 16-byte aligned: YES
  0046d440  55             PUSH EBP
  0046d441  8B EC          MOV EBP,ESP
  0046d443  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0046d440
string literals (0):
direct callees (1):
  0x00461AA0  Gob::SetScene
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0046d440 FUN_0046d440 size=38 ret=4 callers=0 callees=1

undefined4 FUN_0046d440(undefined4 param_1)

{
  FUN_00461aa0(param_1);
  return 1;
}
```

### decoy: 0x0046d230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0046d230: 16-byte aligned: YES
  0046d230  55             PUSH EBP
  0046d231  8B EC          MOV EBP,ESP
  0046d233  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0046d230
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x58 [dec 88]  x4  via ECX,EDX
  +0x14 [dec 20]  x3  via EAX,EDX
  +0x54 [dec 84]  x3  via EAX,ECX
  +0x110 [dec 272]  x2  via EAX,EDX
  +0x16 [dec 22]  x1  via EAX
  +0x10C [dec 268]  x1  via EDX
```

## Card B0-13-50

GOG function: Scene::IsGeomFadeEnabled  gog 0x008570b0  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x00464cd0 (size 25, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00464ca0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857080 Scene::SetSceneFocus -> 0x00464ca0  gate_class=AUTO
- GOG-order neighbour after:  0x008570d0 Scene::GetObjectCount  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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
prologue for 0x00464cd0: 16-byte aligned: YES
  00464cd0  55             PUSH EBP
  00464cd1  8B EC          MOV EBP,ESP
  00464cd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00464cd0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00464cd0 FUN_00464cd0 size=25 ret=0 callers=0 callees=0
THUNK -> unknown (Ghidra may omit __thiscall on thunks; ECX is passed through)

undefined4 FUN_00464cd0(void)

{
  return CONCAT31((int3)((uint)DAT_009f5d28 >> 8),DAT_009f5d28 == DAT_0098ba8c);
}
```

### decoy: 0x00464ca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00464ca0: 16-byte aligned: YES
  00464ca0  55             PUSH EBP
  00464ca1  8B EC          MOV EBP,ESP
  00464ca3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00464ca0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
```

## Card B0-13-51

GOG function: Scene::IsAnimating  gog 0x008575d0  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x004651f0 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004651d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008575b0 Scene::DisableAnimations -> 0x004651d0  gate_class=REVIEW
- GOG-order neighbour after:  0x008575f0 Scene::SetBlindLights -> 0x00465210  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xe6*} | Steam {0xe6*}
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
prologue for 0x004651f0: 16-byte aligned: YES
  004651f0  55             PUSH EBP
  004651f1  8B EC          MOV EBP,ESP
  004651f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004651f0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE6 [dec 230]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004651f0 FUN_004651f0 size=20 ret=0 callers=0 callees=0

undefined1 __fastcall FUN_004651f0(int param_1)

{
  return *(undefined1 *)(param_1 + 0xe6);
}
```

### decoy: 0x004651d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004651d0: 16-byte aligned: YES
  004651d0  55             PUSH EBP
  004651d1  8B EC          MOV EBP,ESP
  004651d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004651d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE6 [dec 230]  x1  via EAX
```

## Card B0-13-52

GOG function: Gob::EnableAlwaysRender  gog 0x0084c3a0  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x00459dc0 (size 18, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459de0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c380 Gob::IsForceAnimated -> 0x00459da0  gate_class=REVIEW
- GOG-order neighbour after:  0x0084c3c0 Gob::DisableAlwaysRender -> 0x00459de0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x12*} | Steam {0x12*}
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
prologue for 0x00459dc0: 16-byte aligned: YES
  00459dc0  55             PUSH EBP
  00459dc1  8B EC          MOV EBP,ESP
  00459dc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459dc0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x12 [dec 18]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459dc0 FUN_00459dc0 size=18 ret=0 callers=0 callees=0

void __fastcall FUN_00459dc0(int param_1)

{
  *(undefined1 *)(param_1 + 0x12) = 1;
  return;
}
```

### decoy: 0x00459de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459de0: 16-byte aligned: YES
  00459de0  55             PUSH EBP
  00459de1  8B EC          MOV EBP,ESP
  00459de3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459de0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x12 [dec 18]  x1  via EAX
```

## Card B0-13-53

GOG function: CSWSScriptVarTable::DestroyString  gog 0x007f07d0  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x005e6f20 (size 27, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSScriptVarTable::DestroyFloat
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e6f40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f07b0 CSWSScriptVarTable::DestroyFloat -> 0x005e6f20  gate_class=REVIEW
- GOG-order neighbour after:  0x007f07f0 CSWSScriptVarTable::DestroyObject -> 0x005e6f60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007effe0 CSWSScriptVarTable::DeleteIndex -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x3,0x4*} | Steam {0x2,0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005e6f20: 16-byte aligned: YES
  005e6f20  55             PUSH EBP
  005e6f21  8B EC          MOV EBP,ESP
  005e6f23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e6f20
string literals (0):
direct callees (1):
  0x005E6750  CSWSScriptVarTable::DeleteIndex
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e6f20 FUN_005e6f20 size=27 ret=4 callers=1 callees=1
THUNK -> 0x005E6750 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_005e6f20(undefined4 param_1)

{
  FUN_005e6750(param_1,2);
  return;
}
```

### decoy: 0x005e6f40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e6f40: 16-byte aligned: YES
  005e6f40  55             PUSH EBP
  005e6f41  8B EC          MOV EBP,ESP
  005e6f43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e6f40
string literals (0):
direct callees (1):
  0x005E6750  CSWSScriptVarTable::DeleteIndex
struct offsets touched (top 0):
```

## Card B0-13-54

GOG function: Gob::BehaviorMessage  gog 0x00855bc0  callconv=__thiscall  param_size_bytes=4  gog_size=54  gog_retn=4
Candidate Steam address: 0x004636d0 (size 54, retn 4)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00463710:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: autolabel:SerializeTabWindow
- GOG-order neighbour before: 0x00855b00 TileData::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00855c00 Gob::EnumParts  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x1b4*} | Steam {0xc*,0x1b4*}
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
prologue for 0x004636d0: 16-byte aligned: YES
  004636d0  55             PUSH EBP
  004636d1  8B EC          MOV EBP,ESP
  004636d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004636d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1B4 [dec 436]  x3  via EAX,ECX,EDX
  +0xC [dec 12]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004636d0 CBaseTabbedPane::SerializeTabWindow size=54 ret=4 callers=1 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall CBaseTabbedPane::SerializeTabWindow(class CArchive &)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CBaseTabbedPane::SerializeTabWindow(CBaseTabbedPane *this,CArchive *param_1)

{
  if (*(int *)(this + 0x1b4) != 0) {
    (**(code **)(**(int **)(this + 0x1b4) + 0xc))(param_1);
  }
  return;
}
```

### decoy: 0x00463710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00463710: 16-byte aligned: YES
  00463710  55             PUSH EBP
  00463711  8B EC          MOV EBP,ESP
  00463713  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00463710
string literals (0):
direct callees (2):
  0x00473F40  vector<T*>::push_back (`ret 4`)
  0x00463710  (unnamed)
struct offsets touched (top 2):
  +0x30 [dec 48]  x1  via EDX
  +0x34 [dec 52]  x1  via ECX
```

## Card B0-13-55

GOG function: Gob::SetMaxBlurLength  gog 0x0084bf90  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x004599b0 (size 25, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004599d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bf70 Gob::GetMotionBlurred -> 0x00459990  gate_class=REVIEW
- GOG-order neighbour after:  0x0084bfb0 Gob::GetMaxBlurLength -> 0x004599d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1ac*} | Steam {0x1ac*}
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
prologue for 0x004599b0: 16-byte aligned: YES
  004599b0  55             PUSH EBP
  004599b1  8B EC          MOV EBP,ESP
  004599b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004599b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1AC [dec 428]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004599b0 FUN_004599b0 size=25 ret=4 callers=0 callees=0

void __thiscall FUN_004599b0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x1ac) = param_2;
  return;
}
```

### decoy: 0x004599d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004599d0: 16-byte aligned: YES
  004599d0  55             PUSH EBP
  004599d1  8B EC          MOV EBP,ESP
  004599d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004599d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1AC [dec 428]  x1  via EAX
```

## Card B0-13-56

GOG function: Scene::SetFog  gog 0x00857010  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00464c30 (size 22, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00464bf0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00856fd0 Global::CAurScene_scalar_deleting_destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00857030 Scene::GetFogRange -> 0x00464c50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x60*} | Steam {0x60*}
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
prologue for 0x00464c30: 16-byte aligned: YES
  00464c30  55             PUSH EBP
  00464c31  8B EC          MOV EBP,ESP
  00464c33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00464c30
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x60 [dec 96]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00464c30 FUN_00464c30 size=22 ret=4 callers=0 callees=0

void __thiscall FUN_00464c30(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x60) = param_2;
  return;
}
```

### decoy: 0x00464bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00464bf0: 16-byte aligned: YES
  00464bf0  55             PUSH EBP
  00464bf1  8B EC          MOV EBP,ESP
  00464bf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00464bf0
string literals (0):
direct callees (2):
  0x00473EB0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-13-57

GOG function: Scene::GetNumRooms  gog 0x008621b0  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x0046ff70 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0046ff30:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00862170 Scene::GetRoom  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008621d0 Scene::SetCurrentRoom  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xd8*} | Steam {0xd8*}
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
prologue for 0x0046ff70: 16-byte aligned: YES
  0046ff70  55             PUSH EBP
  0046ff71  8B EC          MOV EBP,ESP
  0046ff73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0046ff70
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xD8 [dec 216]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0046ff70 FUN_0046ff70 size=20 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_0046ff70(int param_1)

{
  return *(undefined4 *)(param_1 + 0xd8);
}
```

### decoy: 0x0046ff30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0046ff30: 16-byte aligned: YES
  0046ff30  55             PUSH EBP
  0046ff31  8B EC          MOV EBP,ESP
  0046ff33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0046ff30
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xD4 [dec 212]  x1  via EDX
  +0xD8 [dec 216]  x1  via EAX
```

## Card B0-13-58

GOG function: Gob::GetMaxBlurLength  gog 0x0084bfb0  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x004599d0 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004599b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bf90 Gob::SetMaxBlurLength -> 0x004599b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0084bfd0 Gob::EnableDistortion -> 0x004599f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1ac*} | Steam {0x1ac*}
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
prologue for 0x004599d0: 16-byte aligned: YES
  004599d0  55             PUSH EBP
  004599d1  8B EC          MOV EBP,ESP
  004599d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004599d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1AC [dec 428]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004599d0 FUN_004599d0 size=20 ret=0 callers=0 callees=0
THUNK -> unknown (Ghidra may omit __thiscall on thunks; ECX is passed through)

float10 __fastcall FUN_004599d0(int param_1)

{
  return (float10)*(float *)(param_1 + 0x1ac);
}
```

### decoy: 0x004599b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004599b0: 16-byte aligned: YES
  004599b0  55             PUSH EBP
  004599b1  8B EC          MOV EBP,ESP
  004599b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004599b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1AC [dec 428]  x1  via EAX
```

## Card B0-13-59

GOG function: Gob::GetMotionBlurred  gog 0x0084bf70  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00459990 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459970:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bf50 Gob::SetMotionBlurred -> 0x00459970  gate_class=REVIEW
- GOG-order neighbour after:  0x0084bf90 Gob::SetMaxBlurLength -> 0x004599b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x181*} | Steam {0x181*}
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
prologue for 0x00459990: 16-byte aligned: YES
  00459990  55             PUSH EBP
  00459991  8B EC          MOV EBP,ESP
  00459993  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459990
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x181 [dec 385]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459990 FUN_00459990 size=20 ret=0 callers=0 callees=0

undefined1 __fastcall FUN_00459990(int param_1)

{
  return *(undefined1 *)(param_1 + 0x181);
}
```

### decoy: 0x00459970 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459970: 16-byte aligned: YES
  00459970  55             PUSH EBP
  00459971  8B EC          MOV EBP,ESP
  00459973  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459970
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x181 [dec 385]  x1  via EAX
```

## Card B0-13-60

GOG function: CSWPartyTable::RemoveNPC  gog 0x00700490  callconv=(blank)  param_size_bytes=4  gog_size=52  gog_retn=4
Candidate Steam address: 0x005fa960 (size 39, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWPartyTable::GetIsNPCAvailable
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fa920:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00700400 CSWPartyTable::AssignPUP -> 0x005fa890  gate_class=AUTO
- GOG-order neighbour after:  0x007004d0 CSWPartyTable::GetIsNPCAvailable -> 0x005fa960  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007004d0 CSWPartyTable::GetIsNPCAvailable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4c*} | Steam {0x4c*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0xc}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005fa960: 16-byte aligned: YES
  005fa960  55             PUSH EBP
  005fa961  8B EC          MOV EBP,ESP
  005fa963  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fa960
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fa960 FUN_005fa960 size=39 ret=4 callers=21 callees=0

undefined4 __thiscall FUN_005fa960(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if ((param_2 < 0) || (0xb < param_2)) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined4 *)(param_1 + 0x4c + param_2 * 4);
  }
  return uVar1;
}
```

### decoy: 0x005fa920 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fa920: 16-byte aligned: YES
  005fa920  55             PUSH EBP
  005fa921  8B EC          MOV EBP,ESP
  005fa923  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fa920
string literals (0):
direct callees (1):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
struct offsets touched (top 0):
```
