# Review cards B17 chunk 13

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-13-01

GOG function: CAurGUIStringInternal::SetSizeBoundingRect  gog 0x008d0ae0  callconv=(blank)  param_size_bytes=8  gog_size=31  gog_retn=8
Candidate Steam address: 0x0047a1d0 (size 31, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0047a1a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d0ab0 CAurGUIStringInternal::GetSizeBoundingRect  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d0b00 CAurGUIStringInternal::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}
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
prologue for 0x0047a1d0: 16-byte aligned: YES
  0047a1d0  55             PUSH EBP
  0047a1d1  8B EC          MOV EBP,ESP
  0047a1d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0047a1d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0047a1d0 FUN_0047a1d0 size=31 ret=8 callers=0 callees=0

void __thiscall FUN_0047a1d0(int param_1,undefined4 param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 0xc) = param_2;
  *(undefined4 *)(param_1 + 0x10) = param_3;
  return;
}
```

### decoy: 0x0047a1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0047a1a0: 16-byte aligned: YES
  0047a1a0  55             PUSH EBP
  0047a1a1  8B EC          MOV EBP,ESP
  0047a1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0047a1a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
```

### Static facts (script output, report-only)
- size 31 B, RET 8
- twins: none

## Card B17-13-02

GOG function: Material::SetBlendingMode  gog 0x008f5380  callconv=(blank)  param_size_bytes=4  gog_size=234  gog_retn=4
Candidate Steam address: 0x004db1f0 (size 234, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004dae60:STR0/CALLEE1/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f5340 Material::BumpMapIsSpecular  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f5470 Material::SetGUIBlendingMode  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x0089aba0 Gob::Render -> Steam 0x004b3fe0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x28*,0x34*} | Steam {0x28*,0x34*}
imm sets (* = in both, 4 common): GOG {0x2*,0x3*,0x4*,0x14*} | Steam {0x2*,0x3*,0x4*,0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004db1f0: 16-byte aligned: YES
  004db1f0  55             PUSH EBP
  004db1f1  8B EC          MOV EBP,ESP
  004db1f3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004db1f0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x28 [dec 40]  x3  via EAX,EDX
  +0x34 [dec 52]  x3  via EAX,EDX
  +0x988560 [dec 9995616]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004db1f0 FUN_004db1f0 size=234 ret=4 callers=11 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004db1f0(int param_1,int param_2)

{
  glBlendFunc((&DAT_00988530)[*(int *)(*(int *)(param_1 + 0x28) + param_2 * 4)],
              (&DAT_00988530)[*(int *)(*(int *)(param_1 + 0x34) + param_2 * 4)]);
  if ((*(int *)(*(int *)(param_1 + 0x28) + param_2 * 4) == 0) &&
     (*(int *)(*(int *)(param_1 + 0x34) + param_2 * 4) == 2)) {
    glDepthMask(0);
  }
  else {
    glDepthMask(1);
  }
  if (DAT_00a049ec != 0) {
    if ((*(int *)(*(int *)(param_1 + 0x28) + param_2 * 4) == 2) &&
       (*(int *)(*(int *)(param_1 + 0x34) + param_2 * 4) == 3)) {
      glAlphaFunc(DAT_00988570,_DAT_00a049f0);
    }
    else {
      glAlphaFunc(DAT_00988570,0);
    }
  }
  return;
}
```

### decoy: 0x004dae60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004dae60: 16-byte aligned: YES
  004dae60  55             PUSH EBP
  004dae61  8B EC          MOV EBP,ESP
  004dae63  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004dae60
string literals (0):
direct callees (2):
  0x004DA750  GUI image NULL-texture fallback
  0x00426BE0  (unnamed)
struct offsets touched (top 2):
  +0x18 [dec 24]  x2  via ECX
  +0xB8 [dec 184]  x2  via EDX
```

### Static facts (script output, report-only)
- size 234 B, RET 4
- callers 11: 0x00465f90, 0x004662e0, 0x00466700, 0x004a79c0, 0x004b07d0, ...
- callees: 0x0000000f (CALL), 0x0000004c (CALL), 0x0000001f (CALL)
- imports: glAlphaFunc (OPENGL32.DLL), glBlendFunc (OPENGL32.DLL), glDepthMask (OPENGL32.DLL)
- twins: none

## Card B17-13-03

GOG function: CAurGUIStringInternal::GetFontTexture  gog 0x008d09d0  callconv=(blank)  param_size_bytes=0  gog_size=71  gog_retn=0
Candidate Steam address: 0x0047a0c0 (size 71, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0047a0a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d09b0 CAurGUIStringInternal::DisableCursor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d0a20 CAurGUIStringInternal::GetNumLines  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x18*,0x40*} | Steam {0x18*,0x40*}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0047a0c0: 16-byte aligned: YES
  0047a0c0  55             PUSH EBP
  0047a0c1  8B EC          MOV EBP,ESP
  0047a0c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0047a0c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x18 [dec 24]  x2  via EAX
  +0x40 [dec 64]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0047a0c0 FUN_0047a0c0 size=71 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_0047a0c0(int param_1)

{
  undefined4 local_18;
  
  if (*(int *)(param_1 + 0x18) == 0) {
    local_18 = 0;
  }
  else {
    local_18 = (**(code **)(**(int **)(param_1 + 0x18) + 0x40))();
  }
  return local_18;
}
```

### decoy: 0x0047a0a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0047a0a0: 16-byte aligned: YES
  0047a0a0  55             PUSH EBP
  0047a0a1  8B EC          MOV EBP,ESP
  0047a0a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0047a0a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x58 [dec 88]  x1  via ECX
```

### Static facts (script output, report-only)
- size 71 B, RET 0
- indirect: 0x0047a0f2 slot 0x40
- twins: none

## Card B17-13-04

GOG function: Global::ImageReadPLT  gog 0x008d8290  callconv=(blank)  param_size_bytes=12  gog_size=204  gog_retn=0
Candidate Steam address: 0x00483700 (size 204, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR '%s.plt'@0098c598; SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x004837d0:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008d8280 Global::ImageReadProcessedHeader  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d8360 Global::ImageInterpolate  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00921295 Global::sprintf -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x009241e3 unnamed -> Steam ?  in candidate callees: n/a
  0x0092471b unnamed -> Steam ?  in candidate callees: n/a
  0x0092866d unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '%s.plt'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 7 common): GOG {0x2*,0x4*,0x8*,0xc*,0x10*,0x18*,0x4c*} | Steam {0x2*,0x4*,0x8*,0xc*,0x10*,0x18*,0x4c*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x00483700: 16-byte aligned: YES
  00483700  55             PUSH EBP
  00483701  8B EC          MOV EBP,ESP
  00483703  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00483700
string literals (1):
  0x0098C598  at 0048371b  '%s.plt'
direct callees (6):
  0x00919671  (unnamed)
  0x0091CAEB  (unnamed)
  0x0091F3EF  (unnamed)
  0x00919723  operator new
  0x0091C5B3  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00483700 FUN_00483700 size=204 ret=0 callers=1 callees=6

void FUN_00483700(undefined4 param_1,int *param_2,int *param_3)

{
  undefined1 local_4c [16];
  int local_3c;
  int local_38;
  void *local_34;
  char local_30 [32];
  uint local_10;
  FILE *local_c;
  undefined4 local_8;
  
  local_10 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_8 = 2;
  _sprintf [GHIDRA-AUTO-LABEL: unreliable](local_30,"%s.plt",param_1);
  local_c = _fopen [GHIDRA-AUTO-LABEL: unreliable](local_30,"rb");
  if (local_c != (FILE *)0x0) {
    _fread [GHIDRA-AUTO-LABEL: unreliable](local_4c,0x18,1,local_c);
    *param_2 = local_3c;
    *param_3 = local_38;
    local_34 = (void *)FUN_00919723(local_3c * local_38 * 2);
    if (local_34 != (void *)0x0) {
      _fread [GHIDRA-AUTO-LABEL: unreliable](local_34,1,local_3c * local_38 * 2,local_c);
      _fclose [GHIDRA-AUTO-LABEL: unreliable](local_c);
    }
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_10 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x004837d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004837d0: 16-byte aligned: YES
  004837d0  55             PUSH EBP
  004837d1  8B EC          MOV EBP,ESP
  004837d3  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004837d0
string literals (0):
direct callees (2):
  0x0091ED90  (unnamed)
  0x0091C860  _ftol2_sse
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 204 B, RET 0
- callers 1: 0x00421bc0
- callees: 0x00919671 (CALL), 0x0091caeb (CALL), 0x0091f3ef (CALL), 0x00919723 operator new (CALL), 0x0091c5b3 (CALL), 0x009196ee __security_check_cookie (CALL)
- strings: 0x0098c598 "%s.plt"
- twins: none

## Card B17-13-05

GOG function: CRes::Dump  gog 0x0061ab90  callconv=(blank)  param_size_bytes=4  gog_size=32  gog_retn=4
Candidate Steam address: 0x00711170 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CRes::Destructor | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007110d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061ab60 CRes::ReadRaw  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061abb0 CRes::GetDemands  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061b7c0 CExoResMan::Dump -> Steam 0x00711e60  in candidate callees: no
GOG callers mapped through trusted rows (2 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x0061c970 CExoResMan::ReleaseResObject -> Steam 0x00713010  in candidate callers: no
  0x0062fda0 CExoKeyTable::DestroyTable -> Steam 0x00726350  in candidate callers: no
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

### Static facts (script output, report-only)
- size 44 B, RET 4
- callees: 0x007111a0 (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B17-13-06

GOG function: CSWGuiKeyMapButton::HitCheckMouse  gog 0x005f1910  callconv=(blank)  param_size_bytes=8  gog_size=76  gog_retn=8
Candidate Steam address: 0x00902c70 (size 127, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00902cf0:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005f18a0 CSWGuiKeyMapButton::SetCaptureVisual  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005f1960 CSWGuiLevelUpPanel::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00916fc0 CSWGuiControl::HitCheckMouse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x44,0x1d4} | Steam {0x3c,0xb8}
imm sets (* = in both, 1 common): GOG {0x8*,0x1d4} | Steam {0x8*,0x10,0x2c}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00902c70: 16-byte aligned: YES
  00902c70  55             PUSH EBP
  00902c71  8B EC          MOV EBP,ESP
  00902c73  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00902c70
string literals (1):
  0x009AA134  at 00902c93  'projectile'
direct callees (4):
  0x00710980  (unnamed)
  0x00462320  (unnamed)
  0x00710CA0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 2):
  +0xB8 [dec 184]  x2  via EAX,EDX
  +0x3C [dec 60]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00902c70 FUN_00902c70 size=127 ret=8 callers=0 callees=4

void __thiscall FUN_00902c70(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined1 local_2c [16];
  undefined1 local_1c [20];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  FUN_00710980(local_1c);
  uVar1 = FUN_00462320(local_1c,"projectile",0,0);
  *(undefined4 *)(param_1 + 0xb8) = uVar1;
  if (*(int *)(param_1 + 0xb8) != 0) {
    FUN_00710ca0(local_2c,param_2);
    *(undefined4 *)(param_1 + 0x3c) = 0;
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x00902cf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00902cf0: 16-byte aligned: YES
  00902cf0  55             PUSH EBP
  00902cf1  8B EC          MOV EBP,ESP
  00902cf3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00902cf0
string literals (5):
  0x00987564  at 00902d3b  'default'
  0x009AA164  at 00902d77  'travel01'
  0x009AA158  at 00902db4  'impact01'
  0x009AA14C  at 00902dee  'throwout'
  0x009AA140  at 00902e25  'throwback'
direct callees (0):
struct offsets touched (top 2):
  +0xB8 [dec 184]  x12  via EAX,ECX,EDX
  +0x18 [dec 24]  x6  via EDX
```

### Static facts (script output, report-only)
- size 127 B, RET 8
- callees: 0x00710980 (CALL), 0x00462320 (CALL), 0x00710ca0 (CALL), 0x009196ee __security_check_cookie (CALL)
- strings: 0x009aa134 "projectile"
- twins: none

## Card B17-13-07

GOG function: CAurGUIStringInternal::GetIdealPixelHeight  gog 0x008d15b0  callconv=(blank)  param_size_bytes=4  gog_size=636  gog_retn=4
Candidate Steam address: 0x0047aca0 (size 636, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.88) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0047af30:STR0/CALLEE0/SIG0.439/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d1110 CAurGUIStringInternal::DrawBoundingRect  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d1840 CAurGUIStringInternal::WrapStrings  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00865e20 unnamed -> Steam ?  in candidate callees: n/a
  0x00873000 List_T__::operator= -> Steam ?  in candidate callees: n/a
  0x008d0420 CAurGUIStringInternal::RefreshResolutionTextScalar -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x14*,0x18*,0x38*,0x4c*,0x54*} | Steam {0x4*,0x14*,0x18*,0x38*,0x4c*,0x54*}
imm sets (* = in both, 6 common): GOG {0x2*,0x3*,0x4*,0x3c*,0x48*,0x7c*} | Steam {0x2*,0x3*,0x4*,0x3c*,0x48*,0x7c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0047aca0: 16-byte aligned: YES
  0047aca0  55             PUSH EBP
  0047aca1  8B EC          MOV EBP,ESP
  0047aca3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0047aca0
string literals (0):
direct callees (5):
  0x00479B10  CAurGUIStringInternal::RefreshResolutionTextScalar
  0x0091C860  _ftol2_sse
  0x0049B760  (unnamed)
  0x00428FC0  (unnamed)
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
struct offsets touched (top 6):
  +0x4C [dec 76]  x6  via EAX,ECX
  +0x4 [dec 4]  x3  via ECX
  +0x14 [dec 20]  x3  via EDX
  +0x18 [dec 24]  x3  via EDX
  +0x38 [dec 56]  x3  via EDX
  +0x54 [dec 84]  x3  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0047aca0 FUN_0047aca0 size=636 ret=4 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __thiscall FUN_0047aca0(int *param_1,int param_2)

{
  int iVar1;
  undefined1 local_64 [12];
  int local_58;
  short local_54;
  int local_50;
  undefined1 local_4c [12];
  float local_40;
  undefined1 local_3c [12];
  undefined2 local_30;
  int local_2c;
  undefined1 local_28 [12];
  float local_1c;
  int local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094ada0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00479b10(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (param_2 == -1) {
    if (param_1[0x13 = byte +0x4c] == 0) {
      FUN_0049b760(param_1 + 0xf);
      local_8 = 0;
      FUN_0049b760(param_1 + 0x12);
      local_8._0_1_ = 1;
      (**(code **)(*param_1 + 0x4c))(0xffffffff);
      local_30 = (undefined2)param_1[0x13 = byte +0x4c];
      FUN_00428fc0(local_3c);
      FUN_00428fc0(local_28);
      local_2c = (**(code **)(*(int *)param_1[6 = byte +0x18] + 0x38))();
      local_1c = (*(float *)(local_2c + 4) + *(float *)(local_2c + 0x14)) * (float)_DAT_0098ba38 *
                 (float)param_1[0x15 = byte +0x54];
      iVar1 = FUN_0091c860();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00473e50();
      local_8 = 0xffffffff;
      FUN_00473e50();
    }
    else {
      local_18 = (**(code **)(*(int *)param_1[6 = byte +0x18] + 0x38))();
      local_14 = (*(float *)(local_18 + 4) + *(float *)(local_18 + 0x14)) * (float)_DAT_0098ba38 *
                 (float)param_1[0x15 = byte +0x54];
      iVar1 = FUN_0091c860();
    }
  }
  else {
    FUN_0049b760(param_1 + 0xf);
    local_8 = 2;
    FUN_0049b760(param_1 + 0x12);
    local_8._0_1_ = 3;
    (**(code **)(*param_1 + 0x4c))(param_2);
    local_54 = (short)param_1[0x13 = byte +0x4c];
    FUN_00428fc0(local_64);
    FUN_00428fc0(local_4c);
    local_50 = (**(code **)(*(int *)param_1[6 = byte +0x18] + 0x38))();
    local_40 = (*(float *)(local_50 + 4) + *(float *)(local_50 + 0x14)) * (float)_DAT_0098ba38;
    local_58 = FUN_0091c860();
    local_58 = FUN_0091c860();
    iVar1 = local_54 * local_58;
    local_8 = CONCAT31(local_8._1_3_,2);
    FUN_00473e50();
    local_8 = 0xffffffff;
    FUN_00473e50();
  }
  ExceptionList = local_10;
  return iVar1;
}
```

### decoy: 0x0047af30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0047af30: 16-byte aligned: YES
  0047af30  55             PUSH EBP
  0047af31  8B EC          MOV EBP,ESP
  0047af33  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0047af30
string literals (0):
direct callees (7):
  0x00479B10  CAurGUIStringInternal::RefreshResolutionTextScalar
  0x0091C860  _ftol2_sse
  0x00919F10  _strlen
  0x00733570  CExoString::CExoString_char
  0x00733780  CExoString::~CExoString
  0x004491C0  CExoPtrList::Add (doubling)
  0x0091F4E0  (unnamed)
struct offsets touched (top 10):
  +0xC [dec 12]  x11  via EAX,ECX,EDX
  +0x18 [dec 24]  x10  via EAX,ECX,EDX
  +0x24 [dec 36]  x9  via EAX,ECX,EDX
  +0x54 [dec 84]  x9  via EAX,ECX,EDX
  +0x10 [dec 16]  x7  via EAX,ECX,EDX
  +0x14 [dec 20]  x6  via EAX,ECX,EDX
  +0x40 [dec 64]  x2  via EAX,EDX
  +0x4C [dec 76]  x2  via EAX,ECX
  +0x1 [dec 1]  x1  via EAX
  +0x4 [dec 4]  x1  via EAX
```

### Static facts (script output, report-only)
- size 636 B, RET 4
- callees: 0x00479b10 (CALL), 0x0091c860 (CALL), 0x0049b760 (CALL), 0x00428fc0 (CALL), 0x00473e50 (CALL)
- indirect: 0x0047acfd slot 0x38, 0x0047ad73 slot 0x4c, 0x0047adb8 slot 0x38, 0x0047ae59 slot 0x4c, 0x0047aeaa slot 0x38
- twins: none

## Card B17-13-08

GOG function: CSWGuiLoadModuleDebugMenu::Constructor  gog 0x005b54b0  callconv=(blank)  param_size_bytes=4  gog_size=2532  gog_retn=4
Candidate Steam address: 0x008c0bd0 (size 172, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.05 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008c0c80:STR0/CALLEE0/SIG0.014/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005b5480 CSWGuiPowersFeatsSkillsDebugMenu::ReturnToMainAfterPick  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005b5ea0 CSWGuiCreateItemMenu::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (34), mapped to Steam through trusted rows only:
  0x00407340 Global::GetOverridePath -> Steam ?  in candidate callees: n/a
  0x0044dd60 CExoArrayList_CExoString_::CExoArrayList_2 -> Steam ?  in candidate callees: n/a
  0x0044dda0 CExoArrayList_CExoString_::Destructor -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x005340d0 CExoArrayList_CSWGuiControl__::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00534170 CExoArrayList::Add_2 -> Steam ?  in candidate callees: n/a
  0x005b60c0 CSWGuiLoadModuleDebugMenu::CreateOption -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  ... 26 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004ceb50 CGuiInGame::Initialize -> Steam 0x007be4c0  in candidate callers: no
GOG strings (10): 'live%d'; 'MODULES:'; '_s.rim'; 'zzzzzzzz.zzz' ...
disp sets (* = in both, 0 common): GOG {0xc,0x2cc,0x344,0x4a0} | Steam {0x8,0x50,0x1198}
imm sets (* = in both, 4 common): GOG {0x2,0x3*,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+23)} | Steam {0x3*,0x4*,0x1c*,0x68*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008c0bd0: 16-byte aligned: YES
  008c0bd0  55             PUSH EBP
  008c0bd1  8B EC          MOV EBP,ESP
  008c0bd3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008c0bd0
string literals (0):
direct callees (6):
  0x0041FF70  (unnamed)
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x006B81C0  CSWSCreatureStats::SetSkillRank
  0x008C0C80  (unnamed)
struct offsets touched (top 3):
  +0x1198 [dec 4504]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008c0bd0 FUN_008c0bd0 size=172 ret=4 callers=0 callees=6

void FUN_008c0bd0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  undefined4 extraout_var;
  undefined4 local_c;
  
  if (*(int *)(param_1 + 0x50) != 0) {
    local_c = 0;
    while( true ) {
      iVar1 = FUN_0041ff70(local_c);
      if (iVar1 == param_1) break;
      local_c = local_c + 1;
    }
    uVar2 = FUN_0051c8f0(extraout_var);
    iVar1 = FUN_0051c100(uVar2);
    if (iVar1 != 0) {
      uVar3 = FUN_006a4e00(0);
      FUN_006b81c0(local_c & 0xff,(uVar3 & 0xff) + 3);
    }
    FUN_008c0c80();
  }
  return;
}
```

### decoy: 0x008c0c80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008c0c80: 16-byte aligned: YES
  008c0c80  55             PUSH EBP
  008c0c81  8B EC          MOV EBP,ESP
  008c0c83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008c0c80
string literals (0):
direct callees (3):
  0x008C0010  (unnamed)
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CB540  CGuiInGame::HidePowersFeatsSkillsDebugMenu
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### Static facts (script output, report-only)
- size 172 B, RET 4
- callees: 0x0041ff70 (CALL), 0x0051c8f0 CServerExoApp::GetPlayerCreatureId (CALL), 0x0051c100 CServerExoApp::GetCreatureByGameObjectID (CALL), 0x006a4e00 CSWSCreatureStats::GetLevel (CALL), 0x006b81c0 CSWSCreatureStats::SetSkillRank (CALL), 0x008c0c80 (CALL)
- twins: none

## Card B17-13-09

GOG function: VertexPrimitiveFlat::ReconnectArraysVAR  gog 0x008e85f0  callconv=(blank)  param_size_bytes=4  gog_size=980  gog_retn=4
Candidate Steam address: 0x004ce460 (size 980, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.83) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004ce850:STR0/CALLEE0/SIG0.184/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008e84b0 VertexPrimitiveFlat::BuildVertexArrays  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008e89e0 VertexPrimitiveSkinned::ReconnectArraysVAR  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00846bb0 MdlNode::AsMdlNodeTriMesh -> Steam ?  in candidate callees: n/a
  0x00846f70 MdlNodeTriMesh::HasTangentBases -> Steam ?  in candidate callees: n/a
  0x0086b190 Global::AurBumpMapAvailable -> Steam ?  in candidate callees: n/a
  0x0086b4a0 Global::AurMultiTextureAvailable -> Steam ?  in candidate callees: n/a
  0x008f55b0 Material::HasBumpMap -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 17 common): GOG {0x4*,0x8*,0xc*,0x18*,0x2c*,0x44*,0x7c*,0x14c*,0x154*,0x158*,0x160*,0x164*,0x168*,0x16c*,...(+3)} | Steam {0x4*,0x8*,0xc*,0x18*,0x2c*,0x44*,0x7c*,0x14c*,0x154*,0x158*,0x160*,0x164*,0x168*,0x16c*,...(+3)}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0xc*,0xd*,0xe*,0x40*,0x1401*,0x1406*,0x84c0*} | Steam {0x2*,0x3*,0x4*,0xc*,0xd*,0xe*,0x40*,0x1401*,0x1406*,0x84c0*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004ce460: 16-byte aligned: YES
  004ce460  55             PUSH EBP
  004ce461  8B EC          MOV EBP,ESP
  004ce463  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004ce460
string literals (0):
direct callees (5):
  0x004545A0  (unnamed)
  0x00484710  (unnamed)
  0x004DB420  (unnamed)
  0x00454960  (unnamed)
  0x00484400  (unnamed)
struct offsets touched (top 10):
  +0x14C [dec 332]  x10  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x988658 [dec 9995864]  x5  via EAX,ECX,EDX
  +0x170 [dec 368]  x3  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x182 [dec 386]  x2  via EAX,EDX
  +0x2C [dec 44]  x1  via EDX
  +0x44 [dec 68]  x1  via ECX
  +0x7C [dec 124]  x1  via EDX
  +0x154 [dec 340]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004ce460 FUN_004ce460 size=980 ret=4 callers=1 callees=11

int __thiscall FUN_004ce460(int param_1,int param_2)

{
  char cVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar4 = (int)*(short *)(*(int *)(param_1 + 4) + 0x180);
  iVar2 = (**(code **)(**(int **)(param_1 + 8) + 0x2c))();
  iVar3 = FUN_004545a0();
  if (*(char *)(iVar2 + 0x7c) == '\0') {
    glDisableClientState(DAT_00988660);
  }
  else {
    glColorPointer(4,0x1401,0,param_2);
    param_2 = param_2 + iVar4 * 4;
    if (DAT_00a7be44 == 0) {
      glEnableClientState(DAT_00988660);
    }
    else {
      glDisableClientState(DAT_00988660);
    }
  }
  switch(*(undefined2 *)(*(int *)(param_1 + 4) + 0x182)) {
  case 4:
    iVar2 = FUN_00484710();
    if (iVar2 != 0) {
      (*DAT_00a32c38)(0x84c3);
    }
    DAT_00a1dd70 = 3;
    glTexCoordPointer(2,0x1406,*(undefined4 *)(iVar3 + 0x14c),param_2 + *(int *)(iVar3 + 0x16c));
  case 3:
    iVar2 = FUN_00484710();
    if (iVar2 != 0) {
      (*DAT_00a32c38)(0x84c2);
    }
    DAT_00a1dd70 = 2;
    glTexCoordPointer(2,0x1406,*(undefined4 *)(iVar3 + 0x14c),param_2 + *(int *)(iVar3 + 0x168));
  case 2:
    iVar2 = FUN_00484710();
    if (iVar2 != 0) {
      (*DAT_00a32c38)(0x84c1);
    }
    DAT_00a1dd70 = 1;
    glTexCoordPointer(2,0x1406,*(undefined4 *)(iVar3 + 0x14c),param_2 + *(int *)(iVar3 + 0x164));
  case 1:
    iVar2 = FUN_00484710();
    if (iVar2 != 0) {
      (*DAT_00a32c38)(0x84c0);
    }
    DAT_00a1dd70 = 0;
    glTexCoordPointer(2,0x1406,*(undefined4 *)(iVar3 + 0x14c),param_2 + *(int *)(iVar3 + 0x160));
  default:
    if (*(short *)(*(int *)(param_1 + 4) + 0x182) < 1) {
      glDisableClientState(DAT_00988658);
    }
    else {
      glEnableClientState(DAT_00988658);
    }
    glNormalPointer(0x1406,*(undefined4 *)(iVar3 + 0x14c),param_2 + *(int *)(iVar3 + 0x158));
    glVertexPointer(3,0x1406,*(undefined4 *)(iVar3 + 0x14c),param_2 + *(int *)(iVar3 + 0x154));
    cVar1 = FUN_004db420();
    if (((cVar1 != '\0') && (cVar1 = FUN_00454960(), cVar1 != '\0')) &&
       (iVar2 = FUN_00484400(), iVar2 != 0)) {
      if (DAT_009f62dc != 0) {
        (*DAT_00a32d70)(0xc,3,0x1406,0,*(undefined4 *)(iVar3 + 0x14c),
                        param_2 + *(int *)(iVar3 + 0x170));
      }
      if (DAT_009f62dc != 0) {
        (*DAT_00a32d70)(0xd,3,0x1406,0,*(undefined4 *)(iVar3 + 0x14c),
                        param_2 + 0xc + *(int *)(iVar3 + 0x170));
      }
      if (DAT_009f62dc != 0) {
        (*DAT_00a32d70)(0xe,3,0x1406,0,*(undefined4 *)(iVar3 + 0x14c),
                        param_2 + 0x18 + *(int *)(iVar3 + 0x170));
      }
      iVar2 = FUN_00484710();
      if (iVar2 != 0) {
/* ... 7 more lines truncated by --max-lines */
```

### decoy: 0x004ce850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004ce850: 16-byte aligned: YES
  004ce850  55             PUSH EBP
  004ce851  8B EC          MOV EBP,ESP
  004ce853  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004ce850
string literals (0):
direct callees (2):
  0x004CE460  (unnamed)
  0x00484820  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x180 [dec 384]  x1  via EDX
```

### Static facts (script output, report-only)
- size 980 B, RET 4
- callers 1: 0x004ce850
- callees: 0x004545a0 (CALL), 0x00000037 (CALL), 0x00000034 (CALL), 0x00000039 (CALL), 0x00484710 (CALL), 0x00000036 (CALL), ...
- indirect: 0x004ce48c slot 0x2c, 0x004ce55d unresolved, 0x004ce5ac unresolved, 0x004ce5fb unresolved, 0x004ce647 unresolved, ...
- imports: glColorPointer (OPENGL32.DLL), glDisableClientState (OPENGL32.DLL), glEnableClientState (OPENGL32.DLL), ...
- twins: none

## Card B17-13-10

GOG function: Global::ImageReadTGA  gog 0x008d8010  callconv=(blank)  param_size_bytes=16  gog_size=277  gog_retn=0
Candidate Steam address: 0x00483490 (size 277, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR '%s.tga'@0098c590; SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00483440:STR0/CALLEE0/SIG0.3/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008d7fc0 Global::ImageReadTGAData  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d8130 Global::ImageRead  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x008d6a10 Global::ImageSwapRedBlue -> Steam ?  in candidate callees: n/a
  0x008d7f70 Global::ImageReadTGAHeader -> Steam ?  in candidate callees: n/a
  0x008d7fc0 Global::ImageReadTGAData -> Steam ?  in candidate callees: n/a
  0x00921295 Global::sprintf -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x009241e3 unnamed -> Steam ?  in candidate callees: n/a
  0x0092471b unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '%s.tga'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x10*,0x10c*} | Steam {0x4*,0x8*,0xc*,0x10*,0x10c*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 16 bytes -> x86 slots 16; ASM RET 0)
```

### prologue
```
prologue for 0x00483490: 16-byte aligned: YES
  00483490  55             PUSH EBP
  00483491  8B EC          MOV EBP,ESP
  00483493  81 EC 0C 01 00 00 SUB ESP,0x10c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00483490
string literals (1):
  0x0098C590  at 004834f3  '%s.tga'
direct callees (7):
  0x00919671  (unnamed)
  0x0091CAEB  (unnamed)
  0x004833F0  (unnamed)
  0x00483440  (unnamed)
  0x0091C5B3  (unnamed)
  0x00481E90  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00483490 FUN_00483490 size=277 ret=0 callers=1 callees=7

void FUN_00483490(char *param_1,undefined4 *param_2,undefined4 *param_3,undefined4 *param_4)

{
  undefined4 uVar1;
  char local_10c [256];
  uint local_c;
  FILE *local_8;
  
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if ((param_1 != (char *)0x0) && (*param_1 != '\0')) {
    _sprintf [GHIDRA-AUTO-LABEL: unreliable](local_10c,"%s",param_1);
    local_8 = _fopen [GHIDRA-AUTO-LABEL: unreliable](local_10c,"rb");
    if (local_8 == (FILE *)0x0) {
      _sprintf [GHIDRA-AUTO-LABEL: unreliable](local_10c,"%s.tga",param_1);
      local_8 = _fopen [GHIDRA-AUTO-LABEL: unreliable](local_10c,"rb");
    }
    if (local_8 != (FILE *)0x0) {
      FUN_004833f0(local_8,param_2,param_3,param_4);
      uVar1 = FUN_00483440(local_8,*param_2,*param_3,*param_4);
      _fclose [GHIDRA-AUTO-LABEL: unreliable](local_8);
      FUN_00481e90(uVar1,*param_2,*param_3,*param_4);
    }
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x00483440 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00483440: 16-byte aligned: YES
  00483440  55             PUSH EBP
  00483441  8B EC          MOV EBP,ESP
  00483443  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00483440
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0091F3EF  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 277 B, RET 0
- callers 1: 0x004835b0
- callees: 0x00919671 (CALL), 0x0091caeb (CALL), 0x004833f0 (CALL), 0x00483440 (CALL), 0x0091c5b3 (CALL), 0x00481e90 (CALL), ...
- strings: 0x0098c590 "%s.tga"
- twins: none

## Card B17-13-11

GOG function: CExoSound::ReInitialize  gog 0x0063a950  callconv=(blank)  param_size_bytes=8  gog_size=129  gog_retn=8
Candidate Steam address: 0x0070b910 (size 39, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0070b8f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063a910 CExoSound::GetListenerPosition  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063a9e0 CExoSound::PlayOneShotSound  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00634140 CExoSoundInternal::Initialize -> Steam ?  in candidate callees: n/a
  0x0063a830 CExoSound::GetNumber2DVoices -> Steam ?  in candidate callees: n/a
  0x0063a860 CExoSound::GetNumber3DVoices -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0xc,0x10,0x18} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0070b910: 16-byte aligned: YES
  0070b910  55             PUSH EBP
  0070b911  8B EC          MOV EBP,ESP
  0070b913  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070b910
string literals (0):
direct callees (1):
  0x007081E0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0070b910 FUN_0070b910 size=39 ret=8 callers=1 callees=1

void __thiscall FUN_0070b910(int *param_1,undefined4 param_2,undefined4 param_3)

{
  if (*param_1 != 0) {
    FUN_007081e0(param_2,param_3);
  }
  return;
}
```

### decoy: 0x0070b8f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070b8f0: 16-byte aligned: YES
  0070b8f0  55             PUSH EBP
  0070b8f1  8B EC          MOV EBP,ESP
  0070b8f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070b8f0
string literals (0):
direct callees (1):
  0x00705D70  CExoSoundInternal::Render (per-frame)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 39 B, RET 8
- callers 1: 0x00783ee0 CClientExoAppInternal::UpdateSoundEngine
- callees: 0x007081e0 (CALL)
- twins: 0x0070bc60 (distinguishing: 0x0070b92c: CALL 0x007081e0)

## Card B17-13-12

GOG function: CSWRoomSurfaceMesh::ClippedLineSegmentWalkable  gog 0x007137e0  callconv=(blank)  param_size_bytes=28  gog_size=2068  gog_retn=28
Candidate Steam address: 0x00650be0 (size 348, retn 28)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.17) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.21 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00650ab0:STR0/CALLEE0/SIG0.208/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00713550 CSWRoomSurfaceMesh::CheckAABBNode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00714000 CSWRoomSurfaceMesh::ComputeClippedLineSegment -> 0x006518b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x0061a280 CResRef::CopyToString -> Steam ?  in candidate callees: n/a
  0x00714000 CSWRoomSurfaceMesh::ComputeClippedLineSegment -> Steam ?  in candidate callees: n/a
  0x00714580 CSWRoomSurfaceMesh::FindTriangle -> Steam ?  in candidate callees: n/a
  0x007158e0 CSWRoomSurfaceMesh::GetTriangleAdjacency -> Steam ?  in candidate callees: n/a
  0x00715a00 CSWRoomSurfaceMesh::GetTriangleVertices -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0xc,0xb0,0xd4,0xe4}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0xc,0x14,0x18,0x1c*,0x41,0x138,0x200} | Steam {0x8,0x10,0x1c*,0x40}

### abi
```
GOG param bytes: 28; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(28/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=28 ; asm RET = 28
Stack Slot Match: TRUE (GOG 28 bytes -> x86 slots 28; ASM RET 28)
```

### prologue
```
prologue for 0x00650be0: 16-byte aligned: YES
  00650be0  55             PUSH EBP
  00650be1  8B EC          MOV EBP,ESP
  00650be3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00650be0
string literals (0):
direct callees (3):
  0x00650D40  (unnamed)
  0x00650E00  (unnamed)
  0x005143C0  (unnamed)
struct offsets touched (top 6):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0xD4 [dec 212]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0xC [dec 12]  x1  via ECX
  +0xB0 [dec 176]  x1  via EAX
  +0xE4 [dec 228]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00650be0 FUN_00650be0 size=348 ret=28 callers=4 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_00650be0(int param_1,float param_2,float param_3,float param_4,float param_5,float param_6,
            float param_7,undefined4 *param_8)

{
  undefined4 uVar1;
  longlong lVar2;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  float local_28;
  float local_24;
  float local_20;
  longlong local_14;
  int local_8;
  
  if (*(int *)(param_1 + 0xb0) == 0) {
    uVar1 = 0;
  }
  else {
    local_14 = FUN_00650d40();
    *param_8 = *(undefined4 *)(param_1 + 0xe4);
    local_8 = FUN_00650e00(*(undefined4 *)(param_1 + 0xd4),&param_2,&param_5,param_8);
    if (param_8[3 = byte +0xc] == -1) {
      local_34 = 0x3f800000;
      local_30 = 0x3f800000;
      local_2c = 0;
      FUN_005143c0(&local_28,&local_34);
      local_28 = local_28 * _DAT_0098bd74;
      local_24 = local_24 * _DAT_0098bd74;
      local_20 = local_20 * _DAT_0098bd74;
      param_2 = param_2 + local_28;
      param_3 = param_3 + local_24;
      param_4 = param_4 + local_20;
      param_5 = param_5 + local_28;
      param_6 = param_6 + local_24;
      param_7 = param_7 + local_20;
      local_8 = FUN_00650e00(*(undefined4 *)(param_1 + 0xd4),&param_2,&param_5,param_8);
    }
    if (0 < local_8) {
      param_8[2 = byte +0x8] = 1;
      param_8[4 = byte +0x10] = param_5;
      param_8[5 = byte +0x14] = param_6;
      param_8[6 = byte +0x18] = param_7;
    }
    lVar2 = FUN_00650d40();
    _DAT_00a7e0f8 = (float)(lVar2 - local_14);
    uVar1 = param_8[2 = byte +0x8];
  }
  return uVar1;
}
```

### decoy: 0x00650ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00650ab0: 16-byte aligned: YES
  00650ab0  55             PUSH EBP
  00650ab1  8B EC          MOV EBP,ESP
  00650ab3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00650ab0
string literals (0):
direct callees (2):
  0x00650E00  (unnamed)
  0x005143C0  (unnamed)
struct offsets touched (top 6):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xD4 [dec 212]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0xC [dec 12]  x1  via ECX
  +0xB0 [dec 176]  x1  via EAX
  +0xE0 [dec 224]  x1  via EAX
```

### Static facts (script output, report-only)
- size 348 B, RET 28
- callers 4: 0x005ff6a0, 0x00651e30, 0x00652030, 0x00775aa0
- callees: 0x00650d40 (CALL), 0x00650e00 (CALL), 0x005143c0 (CALL)
- twins: none

## Card B17-13-13

GOG function: CSWGuiLoadModuleDebugMenu::Constructor  gog 0x005b54b0  callconv=(blank)  param_size_bytes=4  gog_size=2532  gog_retn=4
Candidate Steam address: 0x008c1930 (size 185, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008c19f0:STR0/CALLEE3/SIG0.531/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005b5480 CSWGuiPowersFeatsSkillsDebugMenu::ReturnToMainAfterPick  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005b5ea0 CSWGuiCreateItemMenu::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (34), mapped to Steam through trusted rows only:
  0x00407340 Global::GetOverridePath -> Steam ?  in candidate callees: n/a
  0x0044dd60 CExoArrayList_CExoString_::CExoArrayList_2 -> Steam ?  in candidate callees: n/a
  0x0044dda0 CExoArrayList_CExoString_::Destructor -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x005340d0 CExoArrayList_CSWGuiControl__::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00534170 CExoArrayList::Add_2 -> Steam ?  in candidate callees: n/a
  0x005b60c0 CSWGuiLoadModuleDebugMenu::CreateOption -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  ... 26 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004ceb50 CGuiInGame::Initialize -> Steam 0x007be4c0  in candidate callers: no
GOG strings (10): 'live%d'; 'MODULES:'; '_s.rim'; 'zzzzzzzz.zzz' ...
disp sets (* = in both, 0 common): GOG {0xc,0x2cc,0x344,0x4a0} | Steam {0x4,0x8,0x50}
imm sets (* = in both, 2 common): GOG {0x2,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+23)} | Steam {0x4*,0x1c*,0x178}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008c1930: 16-byte aligned: YES
  008c1930  55             PUSH EBP
  008c1931  8B EC          MOV EBP,ESP
  008c1933  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008c1930
string literals (0):
direct callees (6):
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CB4A0  CGuiInGame::HideLoadModuleDebugMenu
  0x007335F0  CExoString::CExoString_copy
  0x0051BEA0  CServerExoApp::SetMoveToModuleString
  0x0051BE50  CServerExoApp::SetMoveToModulePending
  0x00733780  CExoString::~CExoString
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008c1930 FUN_008c1930 size=185 ret=4 callers=0 callees=6

void __thiscall FUN_008c1930(undefined4 param_1,int param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00950448;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(int *)(param_2 + 0x50) != 0) {
    FUN_0073f750(uVar1,param_1);
    FUN_007cb4a0();
    FUN_007335f0(param_2 + 0x178);
    local_8 = 0;
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    FUN_0051bea0(local_18);
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    FUN_0051be50(1);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1,param_1,uVar2,uVar3);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008c19f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008c19f0: 16-byte aligned: YES
  008c19f0  55             PUSH EBP
  008c19f1  8B EC          MOV EBP,ESP
  008c19f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008c19f0
string literals (7):
  0x009A8200  at 008c1a77  'debug_p'
  0x009A18B8  at 008c1a97  'LB_OPTIONS'
  0x009A81F4  at 008c1ad1  'LBL_BUILD'
  0x009A81EC  at 008c1b19  'Build: '
  0x0099A890  at 008c1d05  'BASEITEMS'
  0x009983A8  at 008c1d50  'Label'
  0x009A83AC  at 008c1d95  'No StrRef: %s'
direct callees (27):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x0041CE90  (unnamed)
  0x00419740  LabelCtor
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00733DF0  CExoString::operator+
  0x00416E30  CSWGuiTextParams::SetText
  0x0084A640  (unnamed)
  0x00840EF0  (unnamed)
  0x008BFE30  (unnamed)
  0x00733540  CExoString::Constructor
  0x006D5200  (unnamed)
  0x006F9150  (unnamed)
  0x007337C0  CExoString::operator=
  0x00919723  operator new
  0x0071AD40  CRes2DA helper ctor (name, flag)
struct offsets touched (top 4):
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x2CC [dec 716]  x2  via ECX,EDX
  +0x344 [dec 836]  x1  via EDX
  +0x4A0 [dec 1184]  x1  via EDX
```

### Static facts (script output, report-only)
- size 185 B, RET 4
- callees: 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x007cb4a0 CGuiInGame::HideLoadModuleDebugMenu (CALL), 0x007335f0 CExoString::CExoString_copy (CALL), 0x0051bea0 CServerExoApp::SetMoveToModuleString (CALL), 0x0051be50 CServerExoApp::SetMoveToModulePending (CALL), 0x00733780 CExoString::Destructor (CALL)
- twins: none

## Card B17-13-14

GOG function: Global::ImageRead16bit  gog 0x008d7860  callconv=(blank)  param_size_bytes=20  gog_size=340  gog_retn=0
Candidate Steam address: 0x00482ce0 (size 340, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00482e40:STR0/CALLEE0/SIG0.3/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008d7750 Global::ImageWriteTGA  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d79c0 Global::ImageRead16bitHeader  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x009241e3 unnamed -> Steam ?  in candidate callees: n/a
  0x0092471b unnamed -> Steam ?  in candidate callees: n/a
  0x00924c20 Global::strcpy -> Steam ?  in candidate callees: n/a
  0x00924c30 unnamed -> Steam ?  in candidate callees: n/a
  0x0092866d unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x10*,0x114*} | Steam {0x4*,0x8*,0x10*,0x114*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 20 bytes -> x86 slots 20; ASM RET 0)
```

### prologue
```
prologue for 0x00482ce0: 16-byte aligned: YES
  00482ce0  55             PUSH EBP
  00482ce1  8B EC          MOV EBP,ESP
  00482ce3  81 EC 14 01 00 00 SUB ESP,0x114
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00482ce0
string literals (0):
direct callees (7):
  0x0091CFF0  (unnamed)
  0x0091CAEB  (unnamed)
  0x0091D000  (unnamed)
  0x0091F3EF  (unnamed)
  0x00919723  operator new
  0x0091C5B3  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00482ce0 FUN_00482ce0 size=340 ret=0 callers=1 callees=7

void FUN_00482ce0(undefined4 param_1,int *param_2,int *param_3,void *param_4,void *param_5)

{
  void *_DstBuf [GHIDRA-AUTO-LABEL: unreliable];
  FILE *local_110;
  char local_10c [260];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  FUN_0091cff0(local_10c,param_1);
  local_110 = _fopen [GHIDRA-AUTO-LABEL: unreliable](local_10c,"rb");
  if (local_110 == (FILE *)0x0) {
    FUN_0091d000(local_10c,&DAT_0098c580);
    local_110 = _fopen [GHIDRA-AUTO-LABEL: unreliable](local_10c,"rb");
  }
  if (local_110 != (FILE *)0x0) {
    _fread [GHIDRA-AUTO-LABEL: unreliable](param_2,4,1,local_110);
    _fread [GHIDRA-AUTO-LABEL: unreliable](param_3,4,1,local_110);
    _fread [GHIDRA-AUTO-LABEL: unreliable](param_4,4,1,local_110);
    _fread [GHIDRA-AUTO-LABEL: unreliable](param_5,4,1,local_110);
    _DstBuf [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_00919723(*param_2 * *param_3 * 2);
    _fread [GHIDRA-AUTO-LABEL: unreliable](_DstBuf [GHIDRA-AUTO-LABEL: unreliable],1,*param_2 * *param_3 * 2,local_110);
    _fclose [GHIDRA-AUTO-LABEL: unreliable](local_110);
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x00482e40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00482e40: 16-byte aligned: YES
  00482e40  55             PUSH EBP
  00482e41  8B EC          MOV EBP,ESP
  00482e43  81 EC 0C 01 00 00 SUB ESP,0x10c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00482e40
string literals (0):
direct callees (6):
  0x0091CFF0  (unnamed)
  0x0091CAEB  (unnamed)
  0x0091D000  (unnamed)
  0x0091F3EF  (unnamed)
  0x0091C5B3  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 340 B, RET 0
- callers 1: 0x004219a0
- callees: 0x0091cff0 (CALL), 0x0091caeb (CALL), 0x0091d000 (CALL), 0x0091f3ef (CALL), 0x00919723 operator new (CALL), 0x0091c5b3 (CALL), ...
- twins: none

## Card B17-13-15

GOG function: Global::ImageGetSubImage  gog 0x008d8c90  callconv=(blank)  param_size_bytes=8  gog_size=176  gog_retn=0
Candidate Steam address: 0x00484100 (size 176, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x004841b0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008d8a80 Global::ImageLowpassBlur  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d8d40 Global::AurImageApplyLowEndEnvMap  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x20*} | Steam {0x4*,0xc*,0x20*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00484100: 16-byte aligned: YES
  00484100  55             PUSH EBP
  00484101  8B EC          MOV EBP,ESP
  00484103  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484100
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0091DBA0  _memcpy
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00484100 FUN_00484100 size=176 ret=0 callers=2 callees=2

int FUN_00484100(int param_1,int param_2,undefined4 param_3,size_t param_4,int param_5,int param_6,
                int param_7,int param_8)

{
  int iVar1;
  undefined4 local_20;
  undefined4 local_10;
  
  iVar1 = FUN_00919723(param_7 * param_8 * param_4);
  for (local_10 = 0; local_10 < param_8; local_10 = local_10 + 1) {
    for (local_20 = 0; local_20 < param_7; local_20 = local_20 + 1) {
      _memcpy [GHIDRA-AUTO-LABEL: unreliable]((void *)(iVar1 + (local_10 * param_7 + local_20) * param_4),
              (void *)(param_1 + ((local_10 + param_6) * param_2 + local_20 + param_5) * param_4),
              param_4);
    }
  }
  return iVar1;
}
```

### decoy: 0x004841b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004841b0: 16-byte aligned: YES
  004841b0  55             PUSH EBP
  004841b1  8B EC          MOV EBP,ESP
  004841b3  81 EC A8 00 00 00 SUB ESP,0xa8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004841b0
string literals (0):
direct callees (2):
  0x0045CFE0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 5):
  +0x1 [dec 1]  x2  via EAX,ECX
  +0x2 [dec 2]  x2  via EAX,EDX
  +0x3 [dec 3]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
```

### Static facts (script output, report-only)
- size 176 B, RET 0
- callers 2: 0x00432fa0, 0x00433cd0
- callees: 0x00919723 operator new (CALL), 0x0091dba0 _memcpy (CALL)
- twins: none

## Card B17-13-16

GOG function: Gob::PlayOutOfOrderAnimation  gog 0x008f6ab0  callconv=(blank)  param_size_bytes=20  gog_size=238  gog_retn=20
Candidate Steam address: 0x004dc920 (size 238, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005043d0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f67a0 LookAtAnimation::BuildAnimation  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f6ba0 Gob::StopOutOfOrderAnimation  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x008f6fc0 OOAnimRun::Constructor -> Steam ?  in candidate callees: n/a
  0x008f70e0 Global::FindAnimation -> Steam ?  in candidate callees: n/a
  0x008fdd20 List_T__::Insert -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x8*,0x10*,0x14*,0x18*,0x54*,0x84*} | Steam {0x8*,0x10*,0x14*,0x18*,0x54*,0x84*}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0x14*,0x18*,0x34*,0xdc*,0x209*} | Steam {0x4*,0x8*,0x14*,0x18*,0x34*,0xdc*,0x209*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x004dc920: 16-byte aligned: YES
  004dc920  55             PUSH EBP
  004dc921  8B EC          MOV EBP,ESP
  004dc923  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004dc920
string literals (0):
direct callees (4):
  0x004DCF50  (unnamed)
  0x00919723  operator new
  0x004DCE30  (unnamed)
  0x0050C380  (unnamed)
struct offsets touched (top 6):
  +0x8 [dec 8]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x54 [dec 84]  x1  via EAX
  +0x84 [dec 132]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004dc920 FUN_004dc920 size=238 ret=20 callers=0 callees=4

void __thiscall
FUN_004dc920(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            ,undefined4 param_6)

{
  int iVar1;
  int iVar2;
  int local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095f1cb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_004dcf50(*(undefined4 *)(param_1 + 0x84),param_2,DAT_00a11f20 ^ (uint)&stack0xfffffffc
                      );
  if (iVar1 != 0) {
    iVar2 = FUN_00919723(0x34);
    local_8 = 0;
    if (iVar2 == 0) {
      local_28 = 0;
    }
    else {
      local_28 = FUN_004dce30(iVar1,param_3,param_4,param_5,param_6);
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(local_28 + 0x10) = 0;
    *(undefined4 *)(local_28 + 0x14) = 0;
    *(undefined4 *)(local_28 + 0x18) = *(undefined4 *)(iVar1 + 0x54);
    *(undefined4 *)(local_28 + 8) = 0x209;
    FUN_0050c380(local_28,0);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x005043d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005043d0: 16-byte aligned: YES
  005043d0  55             PUSH EBP
  005043d1  8B EC          MOV EBP,ESP
  005043d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005043d0
string literals (0):
direct callees (4):
  0x004E3B50  (unnamed)
  0x00473E80  (unnamed)
  0x009196FD  operator delete
  0x0049B840  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x108 [dec 264]  x1  via EDX
```

### Static facts (script output, report-only)
- size 238 B, RET 20
- callees: 0x004dcf50 (CALL), 0x00919723 operator new (CALL), 0x004dce30 (CALL), 0x0050c380 (CALL)
- twins: none

## Card B17-13-17

GOG function: CAurGUIStringInternal::SetSizeBoundingRect_2  gog 0x008d10b0  callconv=(blank)  param_size_bytes=8  gog_size=89  gog_retn=8
Candidate Steam address: 0x0047a7a0 (size 89, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0047a740:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d1050 CAurGUIStringInternal::GetSizeBoundingRect_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d1110 CAurGUIStringInternal::DrawBoundingRect  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}
imm sets (* = in both, 3 common): GOG {0x8*,0xa*,0xc*} | Steam {0x8*,0xa*,0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0047a7a0: 16-byte aligned: YES
  0047a7a0  55             PUSH EBP
  0047a7a1  8B EC          MOV EBP,ESP
  0047a7a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0047a7a0
string literals (0):
direct callees (1):
  0x0091C860  _ftol2_sse
struct offsets touched (top 4):
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via EDX
  +0xA309CC [dec 10684876]  x1  via EAX
  +0xA309CE [dec 10684878]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0047a7a0 FUN_0047a7a0 size=89 ret=8 callers=0 callees=1

void __fastcall FUN_0047a7a0(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  iVar2 = (int)*(short *)(&DAT_00a309cc + DAT_00a309c0 * 10);
  uVar1 = FUN_0091c860();
  *(undefined4 *)(param_1 + 0xc) = uVar1;
  uVar1 = FUN_0091c860((int)(short)(&DAT_00a309ce)[DAT_00a309c0 * 5],iVar2);
  *(undefined4 *)(param_1 + 0x10) = uVar1;
  return;
}
```

### decoy: 0x0047a740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0047a740: 16-byte aligned: YES
  0047a740  55             PUSH EBP
  0047a741  8B EC          MOV EBP,ESP
  0047a743  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0047a740
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0xA309CC [dec 10684876]  x1  via ECX
  +0xA309CE [dec 10684878]  x1  via EDX
```

### Static facts (script output, report-only)
- size 89 B, RET 8
- callees: 0x0091c860 (CALL)
- globals: 0x00a309c0 READ
- twins: none

## Card B17-13-18

GOG function: CExoArrayList::IndexOf  gog 0x0044dc90  callconv=__thiscall  param_size_bytes=4  gog_size=70  gog_retn=4
Candidate Steam address: 0x004144f0 (size 390, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWGuiBorderParams::operator= | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00414680:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044db90 CClientExoAppInternal::UnloadSuperModelSet -> 0x007a1590  trusted
- GOG-order neighbour after:  0x0044dce0 CExoArrayList_CExoString_::CExoArrayList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (9 of 36): the card's A-CALLEE credit may come from these, not from callees
  0x00446ae0 CClientExoAppInternal::DoPassiveSelection -> Steam 0x0079a4c0  in candidate callers: no
  0x005bdec0 CSWGuiUpgrade::OnUpgradeSelected -> Steam 0x008cdb00  in candidate callers: no
  0x005beec0 CSWGuiUpgrade::OnSlotSelected -> Steam 0x008ceb00  in candidate callers: no
  0x005bfed0 CSWGuiUpgrade::InsertUpgrade -> Steam 0x008cfb10  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0xc,0x10,0x20,0x54}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x2,0x3,0x4*,0x14,0x24,0x34,0x44,0x54,0xffffffef,0xfffffff7,0xfffffffb,0xfffffffc}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004144f0: 16-byte aligned: YES
  004144f0  55             PUSH EBP
  004144f1  8B EC          MOV EBP,ESP
  004144f3  83 EC 54       SUB ESP,0x54
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004144f0
string literals (0):
direct callees (3):
  0x00710CA0  (unnamed)
  0x00414980  CSWGuiBorder::GetDimension
  0x00414920  CSWGuiBorderParams::SetBorder
struct offsets touched (top 6):
  +0x20 [dec 32]  x12  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x10 [dec 16]  x2  via EAX,ECX
  +0x54 [dec 84]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004144f0 FUN_004144f0 size=390 ret=4 callers=5 callees=3

undefined4 * __thiscall FUN_004144f0(undefined4 *param_1,int param_2)

{
  undefined4 uVar1;
  undefined4 local_44;
  undefined1 local_34 [16];
  undefined1 local_24 [16];
  undefined1 local_14 [16];
  
  FUN_00710ca0(local_14,param_2 + 0x24);
  FUN_00710ca0(local_24,param_2 + 0x34);
  FUN_00710ca0(local_34,param_2 + 0x44);
  param_1[8 = byte +0x20] = param_1[8 = byte +0x20] & 0xfffffffc | *(uint *)(param_2 + 0x20) & 3;
  param_1[3 = byte +0xc] = *(undefined4 *)(param_2 + 0xc);
  uVar1 = FUN_00414980();
  *param_1 = uVar1;
  param_1[4 = byte +0x10] = *(undefined4 *)(param_2 + 0x10);
  param_1[8 = byte +0x20] = param_1[8 = byte +0x20] & 0xfffffffb | (*(uint *)(param_2 + 0x20) >> 2 & 1) << 2;
  param_1[5 = byte +0x14] = *(undefined4 *)(param_2 + 0x14);
  param_1[6 = byte +0x18] = *(undefined4 *)(param_2 + 0x18);
  param_1[7 = byte +0x1c] = *(undefined4 *)(param_2 + 0x1c);
  param_1[8 = byte +0x20] = param_1[8 = byte +0x20] & 0xfffffff7 | (*(uint *)(param_2 + 0x20) >> 3 & 1) << 3;
  param_1[8 = byte +0x20] = param_1[8 = byte +0x20] & 0xffffffef | (*(uint *)(param_2 + 0x20) >> 4 & 1) << 4;
  param_1[1 = byte +0x4] = *(undefined4 *)(param_2 + 4);
  if (*(int *)(param_2 + 8) == 0) {
    local_44 = *(undefined4 *)(param_2 + 4);
  }
  else {
    local_44 = *(undefined4 *)(param_2 + 8);
  }
  param_1[2 = byte +0x8] = local_44;
  FUN_00414920(param_1[0x15 = byte +0x54]);
  return param_1;
}
```

### decoy: 0x00414680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00414680: 16-byte aligned: YES
  00414680  55             PUSH EBP
  00414681  8B EC          MOV EBP,ESP
  00414683  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00414680
string literals (0):
direct callees (6):
  0x00710A70  ResRef16 != (memcmp of 16 bytes)
  0x00710CA0  (unnamed)
  0x00710A10  (unnamed)
  0x00710980  (unnamed)
  0x0047EB60  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0x54 [dec 84]  x5  via EAX,ECX,EDX
  +0x6C [dec 108]  x4  via ECX,EDX
  +0x4 [dec 4]  x1  via EDX
```

### Static facts (script output, report-only)
- size 390 B, RET 4
- callers 5: 0x00414c20 CSWGuiBorder::Initialize, 0x008a5ba0, 0x008b0fb0, 0x008b38e0, 0x008d0470
- callees: 0x00710ca0 (CALL), 0x00414980 (CALL), 0x00414920 CSWGuiBorderParams::SetBorder (CALL)
- twins: none

## Card B17-13-19

GOG function: CExoArrayList::IndexOf  gog 0x0044dc90  callconv=__thiscall  param_size_bytes=4  gog_size=70  gog_retn=4
Candidate Steam address: 0x00413fe0 (size 170, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00413f90:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044db90 CClientExoAppInternal::UnloadSuperModelSet -> 0x007a1590  trusted
- GOG-order neighbour after:  0x0044dce0 CExoArrayList_CExoString_::CExoArrayList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (9 of 36): the card's A-CALLEE credit may come from these, not from callees
  0x00446ae0 CClientExoAppInternal::DoPassiveSelection -> Steam 0x0079a4c0  in candidate callers: no
  0x005bdec0 CSWGuiUpgrade::OnUpgradeSelected -> Steam 0x008cdb00  in candidate callers: no
  0x005beec0 CSWGuiUpgrade::OnSlotSelected -> Steam 0x008ceb00  in candidate callers: no
  0x005bfed0 CSWGuiUpgrade::InsertUpgrade -> Steam 0x008cfb10  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {0x8,0x44,0x4c,0x50,0x70}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x3,0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00413fe0: 16-byte aligned: YES
  00413fe0  55             PUSH EBP
  00413fe1  8B EC          MOV EBP,ESP
  00413fe3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00413fe0
string literals (0):
direct callees (2):
  0x00420690  (unnamed)
  0x00413F90  (unnamed)
struct offsets touched (top 5):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x50 [dec 80]  x4  via ECX,EDX
  +0x4C [dec 76]  x2  via EAX,EDX
  +0x70 [dec 112]  x2  via EDX
  +0x44 [dec 68]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00413fe0 FUN_00413fe0 size=170 ret=4 callers=4 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00413fe0(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  if (param_2 == 0) {
    FUN_00413f90();
    *(undefined4 *)(param_1 + 0x50) = 0;
  }
  else {
    *(undefined4 *)(param_1 + 0x44) = _DAT_00987474;
    if (*(int *)(param_1 + 8) == 0) {
      *(undefined4 *)(param_1 + 0x50) = 0;
    }
    else {
      iVar1 = (**(code **)(**(int **)(param_1 + 8) + 0x70))();
      if (iVar1 == 0) {
        *(undefined4 *)(param_1 + 0x50) = *(undefined4 *)(param_1 + 8);
      }
      else {
        uVar2 = 0;
        (**(code **)(**(int **)(param_1 + 8) + 0x70))(0);
        uVar2 = FUN_00420690(uVar2);
        *(undefined4 *)(param_1 + 0x50) = uVar2;
      }
    }
  }
  *(int *)(param_1 + 0x4c) = param_2;
  return;
}
```

### decoy: 0x00413f90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00413f90: 16-byte aligned: YES
  00413f90  55             PUSH EBP
  00413f91  8B EC          MOV EBP,ESP
  00413f93  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00413f90
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via ECX
  +0x1C [dec 28]  x1  via EAX
  +0x44 [dec 68]  x1  via EDX
  +0x4C [dec 76]  x1  via EAX
```

### Static facts (script output, report-only)
- size 170 B, RET 4
- callers 4: 0x00412c00, 0x004131e0, 0x00413350, 0x004134a0
- callees: 0x00420690 (CALL), 0x00413f90 (CALL)
- indirect: 0x00414015 slot 0x70, 0x0041402e slot 0x70
- twins: none

## Card B17-13-20

GOG function: Global::AurCubeMapAvailable  gog 0x0086b700  callconv=(blank)  param_size_bytes=0  gog_size=100  gog_retn=0
Candidate Steam address: 0x009852c0 (size 15, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x009852b0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086b6a0 Global::AurATIFrameBufferModificationsAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086b770 Global::AurAnisotropicFilterAvailable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086b2e0 Global::AurBumpMapCardValidate -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x009852c0: 16-byte aligned: YES
  009852c0  55             PUSH EBP
  009852c1  8B EC          MOV EBP,ESP
  009852c3  B9 08 BE A7 00 MOV ECX,0xa7be08
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x009852c0
string literals (0):
direct callees (1):
  0x00463B20  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 009852c0 FUN_009852c0 size=15 ret=0 callers=0 callees=1
THUNK -> 0x00463B20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_009852c0(void)

{
  FUN_00463b20();
  return;
}
```

### decoy: 0x009852b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x009852b0: 16-byte aligned: YES
  009852b0  55             PUSH EBP
  009852b1  8B EC          MOV EBP,ESP
  009852b3  B9 D0 BC A7 00 MOV ECX,0xa7bcd0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x009852b0
string literals (0):
direct callees (1):
  0x00452F80  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 15 B, RET 0
- callees: 0x00463b20 (CALL)
- twins: 0x00984ec0, 0x00984ed0, 0x00984ee0, 0x00984ef0, 0x00985050, ... (distinguishing: 0x009852c3: MOV ECX,0xa7be08)

## Card B17-13-21

GOG function: Global::AurCubeMapAvailable  gog 0x0086b700  callconv=(blank)  param_size_bytes=0  gog_size=100  gog_retn=0
Candidate Steam address: 0x009852a0 (size 15, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00985240:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086b6a0 Global::AurATIFrameBufferModificationsAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086b770 Global::AurAnisotropicFilterAvailable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086b2e0 Global::AurBumpMapCardValidate -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x009852a0: 16-byte aligned: YES
  009852a0  55             PUSH EBP
  009852a1  8B EC          MOV EBP,ESP
  009852a3  B9 08 BC A7 00 MOV ECX,0xa7bc08
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x009852a0
string literals (0):
direct callees (1):
  0x00463B20  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 009852a0 FUN_009852a0 size=15 ret=0 callers=0 callees=1
THUNK -> 0x00463B20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_009852a0(void)

{
  FUN_00463b20();
  return;
}
```

### decoy: 0x00985240 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00985240: 16-byte aligned: YES
  00985240  55             PUSH EBP
  00985241  8B EC          MOV EBP,ESP
  00985243  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00985240
string literals (0):
direct callees (2):
  0x004D5A60  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 15 B, RET 0
- callees: 0x00463b20 (CALL)
- twins: 0x00984ec0, 0x00984ed0, 0x00984ee0, 0x00984ef0, 0x00985050, ... (distinguishing: 0x009852a3: MOV ECX,0xa7bc08)

## Card B17-13-22

GOG function: GLRender::EnableFragmentShadersARB  gog 0x008de4b0  callconv=(blank)  param_size_bytes=20  gog_size=11495  gog_retn=0
Candidate Steam address: 0x004b50c0 (size 11495, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.28) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004ebad0:STR0/CALLEE0/SIG0.06/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008dda80 Global::sInitializeARBFragmentShaders  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008e1290 GLRender::InitializeATIFragmentShaders  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00819a30 Global::AurTextureGetNormalizationCubeMap -> Steam ?  in candidate callees: n/a
  0x008253f0 GLRender::MakeLookUpTexture -> Steam ?  in candidate callees: n/a
  0x00829940 unnamed -> Steam ?  in candidate callees: n/a
  0x0086b4a0 Global::AurMultiTextureAvailable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0xb8*} | Steam {0xc*,0xb8*}
imm sets (* = in both, 30 common): GOG {0x2*,0x3*,0x4*,0x5*,0x7*,0x8*,0x5d*,0xf0*,0xb50*,0xc60*,0xc61*,0xc62*,0xde1*,0x2000*,...(+16)} | Steam {0x2*,0x3*,0x4*,0x5*,0x7*,0x8*,0x5d*,0xf0*,0xb50*,0xc60*,0xc61*,0xc62*,0xde1*,0x2000*,...(+16)}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 20 bytes -> x86 slots 20; ASM RET 0)
```

### prologue
```
prologue for 0x004b50c0: 16-byte aligned: YES
  004b50c0  55             PUSH EBP
  004b50c1  8B EC          MOV EBP,ESP
  004b50c3  81 EC F0 00 00 00 SUB ESP,0xf0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004b50c0
string literals (0):
direct callees (4):
  0x00484710  (unnamed)
  0x00437280  (unnamed)
  0x00432CE0  GLRender::MakeLookUpTexture
  0x00427330  Global::AurTextureGetNormalizationCubeMap
struct offsets touched (top 3):
  +0xB8 [dec 184]  x68  via EAX,EDX
  +0xC [dec 12]  x14  via EAX,EDX
  +0x4B7E40 [dec 4947520]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004b50c0 FUN_004b50c0 size=11495 ret=0 callers=1 callees=9

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004b50c0(undefined4 param_1,int *param_2,int *param_3,int *param_4,int *param_5)

{
  char cVar1;
  int *piVar2;
  int iVar3;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  switch(param_1) {
  case 0:
    (*DAT_00a32d18)(0x8804,0);
    glDisable(0x8804);
    glEnable(0xb50);
    DAT_00a1dd70 = 0;
    iVar3 = FUN_00484710();
    if (iVar3 != 0) {
      (*DAT_00a32da8)(DAT_00a1dd70 + 0x84c0);
    }
    glBindTexture(0xde1,0);
    DAT_00a1dd70 = 1;
    iVar3 = FUN_00484710();
    if (iVar3 != 0) {
      (*DAT_00a32da8)(DAT_00a1dd70 + 0x84c0);
    }
    glBindTexture(0xde1,0);
    DAT_00a1dd70 = 2;
    iVar3 = FUN_00484710();
/* ... 1128 more lines truncated by --max-lines */
```

### decoy: 0x004ebad0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004ebad0: 16-byte aligned: YES
  004ebad0  55             PUSH EBP
  004ebad1  8B EC          MOV EBP,ESP
  004ebad3  81 EC D8 00 00 00 SUB ESP,0xd8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004ebad0
string literals (0):
direct callees (3):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x0091CB24  (unnamed)
struct offsets touched (top 10):
  +0xD0 [dec 208]  x12  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x40 [dec 64]  x3  via EAX,ECX
  +0x78 [dec 120]  x3  via EAX,ECX
  +0x84 [dec 132]  x2  via ECX,EDX
  +0x90 [dec 144]  x2  via EAX,EDX
  +0x94 [dec 148]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via EAX,EDX
  +0x9C [dec 156]  x2  via EAX,ECX
  +0xAC [dec 172]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 11495 B, RET 0
- callers 1: 0x004b7f40
- callees: 0x0000001c (CALL), 0x0000000e (CALL), 0x00484710 (CALL), 0x00000022 (CALL), 0x00437280 (CALL), 0x00000028 (CALL), ...
- indirect: 0x004b50fa unresolved, 0x004b5134 unresolved, 0x004b5167 unresolved, 0x004b519a unresolved, 0x004b51cb unresolved, ...
- imports: glBindTexture (OPENGL32.DLL), glDisable (OPENGL32.DLL), glEnable (OPENGL32.DLL), ...
- twins: none
