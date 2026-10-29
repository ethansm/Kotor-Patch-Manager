# Review cards B20 chunk 4

Answer each card with one TSV row in `steam_db_batches/B20/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B20-4-01

GOG function: PartLightsaber::Draw  gog 0x00890b90  callconv=(blank)  param_size_bytes=0  gog_size=25444  gog_retn=4
Candidate Steam address: 0x004a9ea0 (size 25748, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.88) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.96 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: RotationArc 0x5160c0); SIG 0.96
- best decoy (neighbour/other candidate) scores: 0x004a9d60:STR0/CALLEE0/SIG0.094/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00890a60 PartTriMesh::Mat  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008971d0 PartDanglyMesh::Draw  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x0080d220 Global::slerp -> Steam ?  in candidate callees: n/a
  0x0080df50 Global::RotationArc -> Steam 0x005160c0  in candidate callees: yes
  0x008971d0 PartDanglyMesh::Draw -> Steam ?  in candidate callees: n/a
  0x00897220 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 14 common): GOG {0x4*,0x8*,0xc*,0x24*,0x28*,0x2c*,0x98*,0xfc*,0x1a4*,0x1a8*,0x1ac*,0x228*,0x3b8*,0x3bc*} | Steam {0x4*,0x8*,0xc*,0x24*,0x28*,0x2c*,0x98*,0xfc*,0x1a4*,0x1a8*,0x1ac*,0x228*,0x3b8*,0x3bc*}
imm sets (* = in both, 11 common): GOG {0x2*,0x3*,0x4*,0x5*,0x7*,0xc*,0x10*,0x14*,0x1c*,0x44*,0xff*} | Steam {0x2*,0x3*,0x4*,0x5*,0x7*,0xc*,0x10*,0x14*,0x1c*,0x44*,0xff*,0xfe4}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x004a9ea0: 16-byte aligned: YES
  004a9ea0  55             PUSH EBP
  004a9ea1  8B EC          MOV EBP,ESP
  004a9ea3  81 EC E4 0F 00 00 SUB ESP,0xfe4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004a9ea0
string literals (0):
direct callees (9):
  0x00454480  (unnamed)
  0x004B0610  (unnamed)
  0x004B0660  (unnamed)
  0x005142F0  (unnamed)
  0x00436200  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00515390  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005160C0  Global::RotationArc
struct offsets touched (top 10):
  +0x4 [dec 4]  x95  via EAX,ECX,EDX
  +0x8 [dec 8]  x94  via EAX,ECX,EDX
  +0xC [dec 12]  x10  via EAX,ECX,EDX
  +0x988530 [dec 9995568]  x6  via EAX,ECX,EDX
  +0x9885F4 [dec 9995764]  x6  via ECX,EDX
  +0x9F4928 [dec 10438952]  x6  via EAX,ECX,EDX
  +0x3B8 [dec 952]  x4  via EAX,ECX
  +0x3BC [dec 956]  x3  via EDX
  +0x9885AC [dec 9995692]  x3  via EAX,EDX
  +0x24 [dec 36]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004a9ea0 FUN_004a9ea0 size=25748 ret=4 callers=0 callees=20

/* WARNING: Removing unreachable block (ram,0x004af73f) */
/* WARNING: Removing unreachable block (ram,0x004ae621) */
/* WARNING: Removing unreachable block (ram,0x004aba64) */
/* WARNING: Removing unreachable block (ram,0x004ac103) */
/* WARNING: Removing unreachable block (ram,0x004aba66) */
/* WARNING: Removing unreachable block (ram,0x004ac101) */
/* WARNING: Removing unreachable block (ram,0x004ae623) */
/* WARNING: Removing unreachable block (ram,0x004afe50) */
/* WARNING: Removing unreachable block (ram,0x004af73d) */
/* WARNING: Removing unreachable block (ram,0x004aed52) */
/* WARNING: Removing unreachable block (ram,0x004afe4e) */
/* WARNING: Removing unreachable block (ram,0x004aed50) */
/* WARNING: Switch with 1 destination removed at 0x004aec48 */
/* WARNING: Switch with 1 destination removed at 0x004af377 */
/* WARNING: Switch with 1 destination removed at 0x004afd64 */
/* WARNING: Switch with 1 destination removed at 0x004b0490 */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __fastcall FUN_004a9ea0(int *param_1)

{
  int *piVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  int iVar6;
  undefined4 uVar7;
  float *pfVar8;
  float10 fVar9;
  float *local_f90;
  float *local_f30;
  float *local_e7c;
  float *local_e28;
  float *local_d90;
  float *local_cdc;
  undefined1 local_c7c [12];
  undefined1 local_c70 [12];
  float local_c64;
  float local_c60;
  float local_c5c;
  float local_c58;
  float *local_c48;
  float *local_c44;
  float local_c40;
  float local_c3c;
  float local_c38;
  float local_c34;
  float local_c30;
  float local_c2c;
  float local_c28;
  float local_c24;
  float local_c20;
  float local_c1c;
  float local_c18;
  float local_c14;
  float local_c10;
  float local_c0c;
  float local_c08;
  float local_c04;
  float *local_be8;
  float local_be4;
  float local_be0;
  float local_bdc;
  float local_bd8;
  float local_bd4;
  float local_bd0;
  float local_bcc;
  float local_bc8;
  float local_bc4;
  float local_bc0;
  float local_bbc;
  float local_bb8;
  float local_bb4;
  float local_bb0;
  float local_bac;
  float *local_ba8;
  float *local_ba4;
/* ... 1932 more lines truncated by --max-lines */
```

### decoy: 0x004a9d60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004a9d60: 16-byte aligned: YES
  004a9d60  55             PUSH EBP
  004a9d61  8B EC          MOV EBP,ESP
  004a9d63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004a9d60
string literals (0):
direct callees (1):
  0x00919723  operator new
struct offsets touched (top 3):
  +0x9F6398 [dec 10445720]  x1  via ECX
  +0x9F639C [dec 10445724]  x1  via EAX
  +0x9F63A0 [dec 10445728]  x1  via EDX
```

### Static facts (script output, report-only)
- size 25748 B, RET 4
- callees: 0x00454480 (CALL), 0x0000001c (CALL), 0x0000000f (CALL), 0x0000004c (CALL), 0x00000054 (CALL), 0x00000012 (CALL), ...
- indirect: 0x004a9f40 slot 0xc
- imports: glBegin (OPENGL32.DLL), glBlendFunc (OPENGL32.DLL), glColor4f (OPENGL32.DLL), ...
- twins: none

## Card B20-4-02

GOG function: CSWGuiObject::SetActiveControl  gog 0x005c0410  callconv=(blank)  param_size_bytes=0  gog_size=13  gog_retn=8
Candidate Steam address: 0x0042a220 (size 428, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=RequestPoolManagerVBO::ResizePool | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0042a1f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005c0340 CSWUpgradeItemEntry::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005c0420 CSWUpgradeItemEntry::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (12 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x004353b0 CClientExoAppInternal::StartServices -> Steam 0x00788d90  in candidate callers: no
  0x00437190 CClientExoAppInternal::StopServices -> Steam 0x0078ac40  in candidate callers: no
  0x0051eca0 CSWGuiMainInterface::PopupActionMenu -> Steam 0x0074d520  in candidate callers: no
  0x00687b00 CSWSCreature::EventHandler -> Steam 0x0056efd0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x2c,0x50,0x5c,0x68}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x4,0x8*,0xc,0x2c,0x8764,0x88b8,0x88e8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0042a220: 16-byte aligned: YES
  0042a220  55             PUSH EBP
  0042a221  8B EC          MOV EBP,ESP
  0042a223  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0042a220
string literals (0):
direct callees (3):
  0x0091D288  operator new[]
  0x0091DBA0  _memcpy
  0x0091C6AA  operator delete[]
struct offsets touched (top 4):
  +0x50 [dec 80]  x7  via EAX,EDX
  +0x2C [dec 44]  x5  via EAX,EDX
  +0x68 [dec 104]  x2  via EAX,EDX
  +0x5C [dec 92]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0042a220 FUN_0042a220 size=428 ret=8 callers=0 callees=3

void __thiscall FUN_0042a220(int param_1,int param_2,undefined4 param_3)

{
  undefined4 local_14;
  void *local_10;
  void *local_c;
  size_t local_8;
  
  if (param_2 != -1) {
    (*DAT_00a32d4c)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),
                    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4));
    (*DAT_00a32b18)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),0x8764,&local_8);
    local_10 = (void *)FUN_0091d288(local_8);
    local_c = (void *)(*DAT_00a32bac)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),35000)
    ;
    _memcpy [GHIDRA-AUTO-LABEL: unreliable](local_10,local_c,local_8);
    (*DAT_00a32c68)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4));
    local_14 = *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4);
    (*DAT_00a32d54)(1,&local_14);
    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4) = 0;
    (*DAT_00a32b28)(1,*(int *)(param_1 + 0x2c) + param_2 * 4);
    (*DAT_00a32d4c)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),
                    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4));
    (*DAT_00a32ab4)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),param_3,0,0x88e8);
    (*DAT_00a32ce8)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),0,local_8,local_10);
    FUN_0091c6aa(*(undefined4 *)(*(int *)(param_1 + 0x68) + param_2 * 4));
    *(undefined4 *)(*(int *)(param_1 + 0x68) + param_2 * 4) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x5c) + param_2 * 4) = param_3;
    FUN_0091c6aa(local_10);
  }
  return;
}
```

### decoy: 0x0042a1f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0042a1f0: 16-byte aligned: YES
  0042a1f0  55             PUSH EBP
  0042a1f1  8B EC          MOV EBP,ESP
  0042a1f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0042a1f0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x5C [dec 92]  x1  via EAX
```

### Static facts (script output, report-only)
- size 428 B, RET 8
- callees: 0x0091d288 operator new[] (CALL), 0x0091dba0 _memcpy (CALL), 0x0091c6aa operator delete[] (CALL)
- indirect: 0x0042a256 unresolved, 0x0042a272 unresolved, 0x0042a2a7 unresolved, 0x0042a2d1 unresolved, 0x0042a2ec unresolved, ...
- twins: none

## Card B20-4-03

GOG function: CExoFileInternal::Read  gog 0x00610130  callconv=(blank)  param_size_bytes=8  gog_size=231  gog_retn=8
Candidate Steam address: 0x0073d710 (size 376, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073d650:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006100a0 CExoFileInternal::GetSize -> 0x0073dba0  trusted
- GOG-order neighbour after:  0x00610220 CExoFileInternal::Read_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00927e9f unnamed -> Steam ?  in candidate callees: n/a
  0x00927ed3 unnamed -> Steam ?  in candidate callees: n/a
  0x0092866d unnamed -> Steam ?  in candidate callees: n/a
  0x00928942 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x8,0x10}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*,0x10,0x18} | Steam {0x4*,0x5,0x8*,0x1c}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0073d710: 16-byte aligned: YES
  0073d710  55             PUSH EBP
  0073d711  8B EC          MOV EBP,ESP
  0073d713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0073d710
string literals (1):
  0x0099D540  at 0073d790  'saves'
direct callees (8):
  0x00733A60  CExoString::operator==_2
  0x00919F10  _strlen
  0x00733570  CExoString::CExoString_char
  0x00734FC0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::Constructor
  0x0091D3BA  (unnamed)
  0x0073D650  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073d710 FUN_0073d710 size=376 ret=8 callers=1 callees=8

undefined * __thiscall FUN_0073d710(undefined *param_1,int *param_2,size_t param_3)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  undefined1 local_28 [8];
  int local_20;
  int local_1c;
  size_t local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961de6;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar3 = FUN_00733a60(&DAT_00986925);
  if (iVar3 == 0) {
    local_18 = param_3;
    if (param_3 == 0) {
      if (*param_2 == 0) {
        local_18 = 0;
      }
      else {
        local_18 = _strlen [GHIDRA-AUTO-LABEL: unreliable]((char *)*param_2);
      }
    }
    if (DAT_00a7ea88 != '\0') {
      FUN_00733570("saves");
      local_8 = 0;
      local_20 = FUN_00734fc0(local_28,5);
      local_8 = 0xffffffff;
      FUN_00733780(uVar2);
      if (local_20 != 0) {
        if ((DAT_00a7eac4 & 1) == 0) {
          DAT_00a7eac4 = DAT_00a7eac4 | 1;
          local_8 = 1;
          FUN_00733540();
          _atexit [GHIDRA-AUTO-LABEL: unreliable](FUN_00985340);
          local_8 = 0xffffffff;
        }
        cVar1 = FUN_0073d650(&DAT_00a7eabc);
        if (cVar1 != '\0') {
          ExceptionList = local_10;
          return &DAT_00a7eabc;
        }
      }
    }
    local_1c = 0;
    local_14 = *(int *)(param_1 + 8);
    while ((local_1c == 0 && (local_14 != 0))) {
      iVar3 = FUN_00734fc0(param_2,local_18);
      if (iVar3 == 1) {
        local_1c = 1;
      }
      else {
        local_14 = *(int *)(local_14 + 0x10);
      }
    }
    if (local_1c != 0) {
      param_1 = (undefined *)(local_14 + 8);
    }
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0073d650 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073d650: 16-byte aligned: YES
  0073d650  55             PUSH EBP
  0073d651  8B EC          MOV EBP,ESP
  0073d653  81 EC 80 00 00 00 SUB ESP,0x80
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073d650
string literals (1):
  0x0099D538  at 0073d6ce  '%s%d/'
direct callees (5):
  0x0073D640  (unnamed)
  0x00919671  (unnamed)
  0x009216B0  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x009196EE  __security_check_cookie
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### Static facts (script output, report-only)
- size 376 B, RET 8
- callers 1: 0x00739790
- callees: 0x00733a60 (CALL), 0x00919f10 _strlen (CALL), 0x00733570 CExoString::CStrConstructor (CALL), 0x00734fc0 (CALL), 0x00733780 CExoString::Destructor (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), ...
- strings: 0x0099d540 "saves"
- twins: none

## Card B20-4-04

GOG function: Material::GetTextureTID  gog 0x008f5120  callconv=__thiscall  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x004dae60 (size 132, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=GUI | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004daef0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f50d0 Material::BindEnvMap  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f5150 Material::ReplaceTexture  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x008192e0 Global::AurTextureGetTID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x18,0xb8}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*,0x1c}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004dae60: 16-byte aligned: YES
  004dae60  55             PUSH EBP
  004dae61  8B EC          MOV EBP,ESP
  004dae63  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004dae60
string literals (0):
direct callees (2):
  0x004DA750  GUI image NULL-texture fallback
  0x00426BE0  (unnamed)
struct offsets touched (top 2):
  +0x18 [dec 24]  x2  via ECX
  +0xB8 [dec 184]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004dae60 FUN_004dae60 size=132 ret=0 callers=16 callees=2

void __fastcall FUN_004dae60(undefined4 *param_1)

{
  FUN_004da750(param_1);
  if ((DAT_009f62ec == 0) || (param_1[6 = byte +0x18] == 0)) {
    (**(code **)(*(int *)*param_1 + 0xb8))();
  }
  else {
    (**(code **)(*(int *)param_1[6 = byte +0x18] + 0xb8))();
  }
  FUN_00426be0(*param_1);
  return;
}
```

### decoy: 0x004daef0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004daef0: 16-byte aligned: YES
  004daef0  55             PUSH EBP
  004daef1  8B EC          MOV EBP,ESP
  004daef3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004daef0
string literals (0):
direct callees (2):
  0x004DA750  GUI image NULL-texture fallback
  0x00426BE0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0xB8 [dec 184]  x1  via EDX
```

### Static facts (script output, report-only)
- size 132 B, RET 0
- callers 16: 0x00465f90, 0x004662e0, 0x00466700, 0x0047d2f0, 0x0047ea60, ...
- callees: 0x004da750 (CALL), 0x00426be0 (CALL)
- indirect: 0x004daeaa slot 0xb8, 0x004daeca slot 0xb8
- twins: none

## Card B20-4-05

GOG function: CSWCCreature::`scalar  gog 0x00410e50  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00765940 (size 352, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCCreature::SetSoundSet | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007651b0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410e20 CSWCCreature::IsHostile -> 0x00765150  trusted
- GOG-order neighbour after:  0x00410e80 CSWCCreature::Destructor -> 0x007651b0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00410e80 CSWCCreature::Destructor -> Steam 0x007651b0  in candidate callees: no
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x314,0x384}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x1c,0x38,0xffff}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00765940: 16-byte aligned: YES
  00765940  55             PUSH EBP
  00765941  8B EC          MOV EBP,ESP
  00765943  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00765940
string literals (0):
direct callees (6):
  0x0073FC10  CClientExoApp::GetSoundSetList
  0x0085F460  (unnamed)
  0x00710C20  (unnamed)
  0x00919723  operator new
  0x0085ED60  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0x314 [dec 788]  x4  via EAX,ECX,EDX
  +0x384 [dec 900]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00765940 FUN_00765940 size=352 ret=4 callers=2 callees=6

void __thiscall FUN_00765940(int param_1,short param_2)

{
  int iVar1;
  undefined4 local_48;
  undefined1 local_24 [16];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00964a6b;
  local_10 = ExceptionList;
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (param_2 != *(short *)(param_1 + 0x314)) {
    *(short *)(param_1 + 0x314) = param_2;
    if (*(int *)(param_1 + 0x384 [dec 900]) != 0) {
      if (*(undefined4 **)(param_1 + 0x384 [dec 900]) != (undefined4 *)0x0) {
        (**(code **)**(undefined4 **)(param_1 + 0x384 [dec 900]))(1,local_14);
      }
      *(undefined4 *)(param_1 + 0x384 [dec 900]) = 0;
    }
    if ((*(short *)(param_1 + 0x314) != -1) && (iVar1 = FUN_0073fc10(), iVar1 != 0)) {
      FUN_0085f460(local_24,*(undefined2 *)(param_1 + 0x314));
      iVar1 = FUN_00710c20(&DAT_00986925);
      if (iVar1 == 0) {
        iVar1 = FUN_00919723(0x1c);
        local_8 = 0;
        if (iVar1 == 0) {
          local_48 = 0;
        }
        else {
          local_48 = basic_filebuf<>(local_24);
        }
        local_8 = 0xffffffff;
        *(undefined4 *)(param_1 + 0x384 [dec 900]) = local_48;
      }
    }
  }
  ExceptionList = local_10;
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_14 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x007651b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007651b0: 16-byte aligned: YES
  007651b0  55             PUSH EBP
  007651b1  8B EC          MOV EBP,ESP
  007651b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007651b0
string literals (0):
direct callees (9):
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x009196FD  operator delete
  0x00859500  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
  0x0076D190  CSWCCreature::UnloadModel
  0x007645D0  (unnamed)
  0x00773970  CSWCCreature::SetSpecialWalkAnimation_2
  0x00740A50  CClientExoApp::ClearCachedCreatures
  0x007ED5F0  CSWCObject::Destructor_2
struct offsets touched (top 10):
  +0x178 [dec 376]  x4  via EAX,ECX
  +0x164 [dec 356]  x3  via EAX,ECX,EDX
  +0x184 [dec 388]  x3  via EAX,ECX,EDX
  +0x310 [dec 784]  x3  via EAX,ECX,EDX
  +0x324 [dec 804]  x3  via ECX,EDX
  +0x384 [dec 900]  x3  via EAX,ECX,EDX
  +0x4C [dec 76]  x2  via EAX,EDX
  +0x12C [dec 300]  x2  via EAX,EDX
  +0x130 [dec 304]  x2  via ECX,EDX
  +0x134 [dec 308]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 352 B, RET 4
- callers 2: 0x00804f10, 0x008f92b0 CSWGuiClassSelection::Constructor
- callees: 0x0073fc10 CClientExoApp::GetSoundSetList (CALL), 0x0085f460 (CALL), 0x00710c20 (CALL), 0x00919723 operator new (CALL), 0x0085ed60 (CALL), 0x009196ee __security_check_cookie (CALL)
- indirect: 0x007659c0 slot 0x0
- globals: 0x00a1b4a4 READ
- twins: none

## Card B20-4-06

GOG function: CSWCCreature::`scalar  gog 0x00410e50  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00765110 (size 61, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCCreature::GetName | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007650c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410e20 CSWCCreature::IsHostile -> 0x00765150  trusted
- GOG-order neighbour after:  0x00410e80 CSWCCreature::Destructor -> 0x007651b0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00410e80 CSWCCreature::Destructor -> Steam 0x007651b0  in candidate callees: no
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x310}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0xc,0x14}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00765110: 16-byte aligned: YES
  00765110  55             PUSH EBP
  00765111  8B EC          MOV EBP,ESP
  00765113  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00765110
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 1):
  +0x310 [dec 784]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00765110 FUN_00765110 size=61 ret=4 callers=0 callees=1

undefined4 __thiscall FUN_00765110(int param_1,undefined4 param_2)

{
  FUN_007335f0(*(int *)(param_1 + 0x310) + 0x14);
  return param_2;
}
```

### decoy: 0x007650c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007650c0: 16-byte aligned: YES
  007650c0  55             PUSH EBP
  007650c1  8B EC          MOV EBP,ESP
  007650c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007650c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x310 [dec 784]  x2  via EAX,EDX
  +0x124 [dec 292]  x1  via ECX
```

### Static facts (script output, report-only)
- size 61 B, RET 4
- callees: 0x007335f0 CExoString::CExoString_copy (CALL)
- twins: none

## Card B20-4-07

GOG function: CPathfindInformation::Destructor  gog 0x00647090  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x006f9420 (size 19, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: Destructor_2 0x5634c0, Destructor_2 0x5219d0)
- best decoy (neighbour/other candidate) scores: 0x006f9200:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00646e70 CPathfindInformation::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006470b0 CPathfindInformation::Initialize -> 0x006f9440  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00647660 CPathfindInformation::Shutdown -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0067c0f0 CSWSCreature::Destructor_2 -> Steam 0x005634c0  in candidate callers: yes
  0x00716b80 CSWSArea::Destructor_2 -> Steam 0x005219d0  in candidate callers: yes
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
prologue for 0x006f9420: 16-byte aligned: YES
  006f9420  55             PUSH EBP
  006f9421  8B EC          MOV EBP,ESP
  006f9423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006f9420
string literals (0):
direct callees (1):
  0x006F99F0  CPathfindInformation::Shutdown (leaks +0x18C/+0x1B4/+0x1D8/+0x200)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006f9420 FUN_006f9420 size=19 ret=0 callers=2 callees=1
THUNK -> 0x006F99F0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_006f9420(undefined4 param_1)

{
  FUN_006f99f0(param_1);
  return;
}
```

### decoy: 0x006f9200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006f9200: 16-byte aligned: YES
  006f9200  55             PUSH EBP
  006f9201  8B EC          MOV EBP,ESP
  006f9203  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006f9200
string literals (0):
direct callees (1):
  0x006F9440  CPathfindInformation::Initialize
struct offsets touched (top 4):
  +0x4 [dec 4]  x14  via EAX,ECX,EDX
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0x34 [dec 52]  x1  via EAX
  +0x18C [dec 396]  x1  via EDX
```

### Static facts (script output, report-only)
- size 19 B, RET 0
- callers 2: 0x005219d0 CSWSArea::Destructor_2, 0x005634c0 CSWSCreature::Destructor_2
- callees: 0x006f99f0 (CALL)
- twins: none

## Card B20-4-08

GOG function: Global::AuroraUseARBShaders  gog 0x0086ba30  callconv=(blank)  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x00484ca0 (size 33, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: RenderSkinnedVP 0x4c3430)
- best decoy (neighbour/other candidate) scores: 0x00484c80:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086ba10 Global::AurCheckFrameBufferModificationsAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ba60 Global::AurClearActiveBins  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x00878e50 Global::RenderSkinnedVP -> Steam 0x004c3430  in candidate callers: yes
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
prologue for 0x00484ca0: 16-byte aligned: YES
  00484ca0  55             PUSH EBP
  00484ca1  8B EC          MOV EBP,ESP
  00484ca3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484ca0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00484ca0 FUN_00484ca0 size=33 ret=0 callers=12 callees=0

bool FUN_00484ca0(void)

{
  return (DAT_009f60e4 & DAT_00a32df8) == DAT_009f60e4;
}
```

### decoy: 0x00484c80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484c80: 16-byte aligned: YES
  00484c80  55             PUSH EBP
  00484c81  8B EC          MOV EBP,ESP
  00484c83  E8 88 FC FF FF CALL 0x00484910
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484c80
string literals (0):
direct callees (2):
  0x00484910  GL capability test #1 (always returns 0)
  0x00484970  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 33 B, RET 0
- callers 12: 0x00437bd0, 0x00438320, 0x0043bb50, 0x0043c290, 0x0043e1c0, ...
- twins: none

## Card B20-4-09

GOG function: Global::FindGob  gog 0x0084b050  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=0
Candidate Steam address: 0x00458a80 (size 90, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00458ae0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0084af50 PartLight::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084b0b0 Gob::SetScene  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0084b0b0 Gob::SetScene -> Steam ?  in candidate callees: n/a
  0x0093e852 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x64*} | Steam {0x4*,0x8*,0x64*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00458a80: 16-byte aligned: YES
  00458a80  55             PUSH EBP
  00458a81  8B EC          MOV EBP,ESP
  00458a83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00458a80
string literals (0):
direct callees (2):
  0x00458AE0  (unnamed)
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00458a80 FUN_00458a80 size=90 ret=0 callers=4 callees=2

undefined4 FUN_00458a80(char *param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = FUN_00458ae0(param_1);
  if ((iVar1 < DAT_00a1ffa0) &&
     (iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]((char *)(*(int *)(DAT_00a1ff9c + iVar1 * 4) + 0x64 [dec 100]),param_1), iVar2 == 0)) {
    return *(undefined4 *)(DAT_00a1ff9c + iVar1 * 4);
  }
  return 0;
}
```

### decoy: 0x00458ae0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00458ae0: 16-byte aligned: YES
  00458ae0  55             PUSH EBP
  00458ae1  8B EC          MOV EBP,ESP
  00458ae3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00458ae0
string literals (0):
direct callees (1):
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 90 B, RET 0
- callers 4: 0x004289d0, 0x00458b70, 0x00463900, 0x00464500
- callees: 0x00458ae0 (CALL), 0x009370c7 _stricmp (CALL)
- twins: none

## Card B20-4-10

GOG function: GLRender::RenderDrawableSquares  gog 0x00827df0  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x00435720 (size 1885, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004356e0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00827dd0 GLRender::RemoveSquareToRender  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00827e30 GLRender::FrameBufferModifications  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x28}
imm sets (* = in both, 0 common): GOG {0xc,0x18} | Steam {0x6,0x14}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00435720: 16-byte aligned: YES
  00435720  55             PUSH EBP
  00435721  8B EC          MOV EBP,ESP
  00435723  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00435720
string literals (0):
direct callees (2):
  0x004848B0  (unnamed)
  0x00484970  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00435720 FUN_00435720 size=1885 ret=0 callers=1 callees=6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00435720(int param_1)

{
  int iVar1;
  int local_c;
  int local_8;
  
  if (DAT_00a1dd7c != (int *)0x0) {
    (**(code **)(*DAT_00a1dd7c + 0x28))();
  }
  if (DAT_00a1ddc4 != (HDC)0x0) {
    iVar1 = FUN_004848b0();
    if (iVar1 == 0) {
      wglMakeCurrent(DAT_00a1ddc4,DAT_00a1ddc8);
      glDeleteTextures(1,&DAT_00a1ddcc);
      wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);
      wglDeleteContext(DAT_00a1ddc8);
      (*DAT_00a32ae8)(DAT_00a1ddc0,DAT_00a1ddc4);
      (*DAT_00a32b0c)(DAT_00a1ddc0);
      DAT_00a1ddc4 = (HDC)0x0;
      DAT_00a1ddc8 = (HGLRC)0x0;
      DAT_00a1ddc0 = 0;
      DAT_00a1ddcc = 0;
      wglMakeCurrent(DAT_00a1ddd8,DAT_00a1dddc);
      glDeleteTextures(1,&DAT_00a1dde0);
      wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);
      wglDeleteContext(DAT_00a1dddc);
      (*DAT_00a32ae8)(DAT_00a1ddd4,DAT_00a1ddd8);
      (*DAT_00a32b0c)(DAT_00a1ddd4);
      DAT_00a1ddd8 = (HDC)0x0;
      DAT_00a1dddc = (HGLRC)0x0;
      DAT_00a1ddd4 = 0;
      DAT_00a1dde0 = 0;
      glDeleteTextures(1,&DAT_00a1ddd0);
      DAT_00a1ddd0 = 0;
    }
    else {
      (*DAT_00a32db4)(DAT_00a1ddc4,DAT_00a1ddc4,DAT_00a1ddc8);
      glDeleteTextures(1,&DAT_00a1ddcc);
      (*DAT_00a32db4)(DAT_00a1bcb8,DAT_00a1bcb8,DAT_00a1dd24);
      (*DAT_00a32ae8)(DAT_00a1ddc0,DAT_00a1ddc4);
      (*DAT_00a32b0c)(DAT_00a1ddc0);
      DAT_00a1ddc4 = (HDC)0x0;
      DAT_00a1ddc8 = (HGLRC)0x0;
      DAT_00a1ddc0 = 0;
      DAT_00a1ddcc = 0;
      (*DAT_00a32db4)(DAT_00a1ddd8,DAT_00a1ddd8,DAT_00a1dddc);
      glDeleteTextures(1,&DAT_00a1dde0);
      (*DAT_00a32db4)(DAT_00a1bcb8,DAT_00a1bcb8,DAT_00a1dd24);
      (*DAT_00a32ae8)(DAT_00a1ddd4,DAT_00a1ddd8);
      (*DAT_00a32b0c)(DAT_00a1ddd4);
      DAT_00a1ddd8 = (HDC)0x0;
      DAT_00a1dddc = (HGLRC)0x0;
      DAT_00a1ddd4 = 0;
      DAT_00a1dde0 = 0;
      glDeleteTextures(1,&DAT_00a1ddd0);
      DAT_00a1ddd0 = 0;
    }
  }
  if (DAT_00a1dde4 != 0) {
    (*DAT_00a32af4)(1,&DAT_00a1dde4);
    DAT_00a1dde4 = 0;
  }
  if (DAT_00a1dde8 != 0) {
    (*DAT_00a32af4)(1,&DAT_00a1dde8);
    DAT_00a1dde8 = 0;
  }
  if (DAT_00a1ddd0 != 0) {
    glDeleteTextures(1,&DAT_00a1ddd0);
    DAT_00a1ddd0 = 0;
  }
  if (DAT_00a1dd44 != 0) {
    glDeleteTextures(1,&DAT_00a1dd44);
    DAT_00a1dd44 = 0;
  }
  if (DAT_00a1dd38 != 0) {
    glDeleteTextures(1,&DAT_00a1dd38);
/* ... 111 more lines truncated by --max-lines */
```

### decoy: 0x004356e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004356e0: 16-byte aligned: YES
  004356e0  55             PUSH EBP
  004356e1  8B EC          MOV EBP,ESP
  004356e3  FF 15 3C 63 98 00 CALL dword ptr [0x0098633c]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004356e0
string literals (0):
direct callees (1):
  0x0091C630  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 1885 B, RET 0
- callers 1: 0x00478300
- callees: 0x004848b0 (CALL), 0x00000023 (CALL), 0x00000045 (CALL), 0x00000044 (CALL), 0x00484970 (CALL), 0x00000121 (CALL)
- indirect: 0x0043573f slot 0x28, 0x0043576f unresolved, 0x00435796 unresolved, 0x004357aa unresolved, 0x004357b6 unresolved, ...
- globals: 0x00a1b484 READ
- imports: ReleaseDC (USER32.DLL), glDeleteTextures (OPENGL32.DLL), wglDeleteContext (OPENGL32.DLL), ...
- twins: none

## Card B20-4-11

GOG function: CExoIni::ReadIniEntry  gog 0x00608450  callconv=(blank)  param_size_bytes=16  gog_size=39  gog_retn=16
Candidate Steam address: 0x00736350 (size 39, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00736320:STR0/CALLEE7/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006083f0 CExoIni::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00608480 CExoIni::WriteIniEntry  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0060e100 CExoIniInternal::ReadIniEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 18): the card's A-CALLEE credit may come from these, not from callees
  0x00427150 CSWCCreature::LipSync -> Steam 0x0077b190  in candidate callers: no
  0x00432410 CClientExoAppInternal::SetEventDescriptions -> Steam 0x00785df0  in candidate callers: yes
  0x004353b0 CClientExoAppInternal::StartServices -> Steam 0x00788d90  in candidate callers: yes
  0x00436cf0 CClientExoAppInternal::ReadVideoModeSettings -> Steam 0x0078a7a0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x00736350: 16-byte aligned: YES
  00736350  55             PUSH EBP
  00736351  8B EC          MOV EBP,ESP
  00736353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00736350
string literals (0):
direct callees (1):
  0x0073C0E0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00736350 FUN_00736350 size=39 ret=16 callers=12 callees=1
THUNK -> 0x0073C0E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00736350(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  FUN_0073c0e0(param_1,param_2,param_3,param_4);
  return;
}
```

### decoy: 0x00736320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00736320: 16-byte aligned: YES
  00736320  55             PUSH EBP
  00736321  8B EC          MOV EBP,ESP
  00736323  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00736320
string literals (0):
direct callees (1):
  0x0073BA80  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 39 B, RET 16
- callers 12: 0x00407920, 0x0069ba00 Global::ExecuteCommandUnlockAllMusic, 0x00785df0 CClientExoAppInternal::SetEventDescriptions, 0x00788d90 CClientExoAppInternal::StartServices, 0x00796e00 CClientExoAppInternal::InitializeClientOptions, ...
- callees: 0x0073c0e0 (CALL)
- twins: 0x00736320 (distinguishing: 0x0073636c: CALL 0x0073c0e0)

## Card B20-4-12

GOG function: Material::GetTextureTID  gog 0x008f5120  callconv=__thiscall  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x004daf40 (size 80, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004daf90:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f50d0 Material::BindEnvMap  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f5150 Material::ReplaceTexture  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x008192e0 Global::AurTextureGetTID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x14,0xb8}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*,0x10,0x14}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004daf40: 16-byte aligned: YES
  004daf40  55             PUSH EBP
  004daf41  8B EC          MOV EBP,ESP
  004daf43  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004daf40
string literals (0):
direct callees (2):
  0x004DA750  GUI image NULL-texture fallback
  0x00426BE0  (unnamed)
struct offsets touched (top 2):
  +0x14 [dec 20]  x2  via ECX,EDX
  +0xB8 [dec 184]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004daf40 FUN_004daf40 size=80 ret=0 callers=1 callees=2

void __fastcall FUN_004daf40(int param_1)

{
  FUN_004da750(param_1 + 0x14);
  (**(code **)(**(int **)(param_1 + 0x14) + 0xb8))();
  FUN_00426be0(*(undefined4 *)(param_1 + 0x14));
  return;
}
```

### decoy: 0x004daf90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004daf90: 16-byte aligned: YES
  004daf90  55             PUSH EBP
  004daf91  8B EC          MOV EBP,ESP
  004daf93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004daf90
string literals (0):
direct callees (1):
  0x00426BE0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 80 B, RET 0
- callers 1: 0x0049d340
- callees: 0x004da750 (CALL), 0x00426be0 (CALL)
- indirect: 0x004daf75 slot 0xb8
- twins: none

## Card B20-4-13

GOG function: Global::AurEnableShadows  gog 0x0086ab80  callconv=(blank)  param_size_bytes=0  gog_size=15  gog_retn=0
Candidate Steam address: 0x00984730 (size 37, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00478ca0:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086ab70 Global::AurInitExtensions  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ab90 Global::AurDisableShadows  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00468f00 CClientOptions::SetShadows -> Steam 0x007bc230  in candidate callers: no
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
prologue for 0x00984730: 16-byte aligned: YES
  00984730  55             PUSH EBP
  00984731  8B EC          MOV EBP,ESP
  00984733  D9 EE          FLDZ
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00984730
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00984730 FUN_00984730 size=37 ret=0 callers=0 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00984730(void)

{
  DAT_00a7fd88 = 0;
  DAT_00a7fd8c = _DAT_0099e240;
  DAT_00a7fd90 = _DAT_009875a4;
  return;
}
```

### decoy: 0x00478ca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478ca0: 16-byte aligned: YES
  00478ca0  55             PUSH EBP
  00478ca1  8B EC          MOV EBP,ESP
  00478ca3  C7 05 30 5D 9F 00 01 00 00 00 MOV dword ptr [0x009f5d30],0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478ca0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 37 B, RET 0
- twins: 0x00984690, 0x00984a30, 0x00984a60 (distinguishing: 0x00984735: FSTP float ptr [0x00a7fd88])

## Card B20-4-14

GOG function: CSWGuiObject::SetActiveControl  gog 0x005c0410  callconv=(blank)  param_size_bytes=0  gog_size=13  gog_retn=8
Candidate Steam address: 0x0042a6e0 (size 205, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0042a600:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005c0340 CSWUpgradeItemEntry::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005c0420 CSWUpgradeItemEntry::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (12 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x004353b0 CClientExoAppInternal::StartServices -> Steam 0x00788d90  in candidate callers: no
  0x00437190 CClientExoAppInternal::StopServices -> Steam 0x0078ac40  in candidate callers: no
  0x0051eca0 CSWGuiMainInterface::PopupActionMenu -> Steam 0x0074d520  in candidate callers: no
  0x00687b00 CSWSCreature::EventHandler -> Steam 0x0056efd0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x20,0x24}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x4,0x8*,0xc,0x14,0xfff,0xfffff000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0042a6e0: 16-byte aligned: YES
  0042a6e0  55             PUSH EBP
  0042a6e1  8B EC          MOV EBP,ESP
  0042a6e3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0042a6e0
string literals (0):
direct callees (3):
  0x0091D288  operator new[]
  0x0091DBA0  _memcpy
  0x0091C6AA  operator delete[]
struct offsets touched (top 2):
  +0x24 [dec 36]  x4  via EAX,ECX,EDX
  +0x20 [dec 32]  x2  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0042a6e0 FUN_0042a6e0 size=205 ret=8 callers=0 callees=3

void __thiscall FUN_0042a6e0(int param_1,int param_2,int param_3)

{
  void *_Src [GHIDRA-AUTO-LABEL: unreliable];
  void *_Dst [GHIDRA-AUTO-LABEL: unreliable];
  
  DAT_00a1dd80 = (param_3 - *(int *)(*(int *)(param_1 + 0x24) + param_2 * 4)) + DAT_00a1dd80;
  DAT_00a1dd84 = ((param_3 + 0xfffU & 0xfffff000) -
                 (*(int *)(*(int *)(param_1 + 0x24) + param_2 * 4) + 0xfffU & 0xfffff000)) +
                 DAT_00a1dd84;
  _Src [GHIDRA-AUTO-LABEL: unreliable] = *(void **)(*(int *)(param_1 + 0x20) + param_2 * 4);
  _Dst [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_0091d288(param_3);
  _memcpy [GHIDRA-AUTO-LABEL: unreliable](_Dst [GHIDRA-AUTO-LABEL: unreliable],_Src [GHIDRA-AUTO-LABEL: unreliable],*(size_t *)(*(int *)(param_1 + 0x24) + param_2 * 4));
  FUN_0091c6aa(_Src [GHIDRA-AUTO-LABEL: unreliable]);
  *(int *)(*(int *)(param_1 + 0x24) + param_2 * 4) = param_3;
  *(void **)(*(int *)(param_1 + 0x20) + param_2 * 4) = _Dst [GHIDRA-AUTO-LABEL: unreliable];
  return;
}
```

### decoy: 0x0042a600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0042a600: 16-byte aligned: YES
  0042a600  55             PUSH EBP
  0042a601  8B EC          MOV EBP,ESP
  0042a603  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0042a600
string literals (0):
direct callees (2):
  0x0091C6AA  operator delete[]
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
struct offsets touched (top 3):
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### Static facts (script output, report-only)
- size 205 B, RET 8
- callees: 0x0091d288 operator new[] (CALL), 0x0091dba0 _memcpy (CALL), 0x0091c6aa operator delete[] (CALL)
- twins: none

## Card B20-4-15

GOG function: MaxTree::FreeTheFreeList_2  gog 0x00844e00  callconv=(blank)  param_size_bytes=4  gog_size=17  gog_retn=0
Candidate Steam address: 0x004527f0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00452730:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00844d40 MaxTree::FreeTheFreeList  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00844e20 MaxTree::InternalDestructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x004527f0: 16-byte aligned: YES
  004527f0  55             PUSH EBP
  004527f1  8B EC          MOV EBP,ESP
  004527f3  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004527f0
string literals (0):
direct callees (1):
  0x00919723  operator new
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004527f0 FUN_004527f0 size=17 ret=0 callers=2 callees=1
THUNK -> 0x00919723 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_004527f0(undefined4 param_1)

{
  FUN_00919723(param_1);
  return;
}
```

### decoy: 0x00452730 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00452730: 16-byte aligned: YES
  00452730  55             PUSH EBP
  00452731  8B EC          MOV EBP,ESP
  00452733  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00452730
string literals (0):
direct callees (5):
  0x004544E0  (unnamed)
  0x004553D0  (unnamed)
  0x00463BA0  (unnamed)
  0x00432F50  (unnamed)
  0x00474030  (unnamed)
struct offsets touched (top 4):
  +0x3C [dec 60]  x2  via ECX,EDX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via ECX
  +0x40 [dec 64]  x1  via EAX
```

### Static facts (script output, report-only)
- size 17 B, RET 0
- callers 2: 0x00464500, 0x004dc610
- callees: 0x00919723 operator new (CALL)
- twins: none

## Card B20-4-16

GOG function: Global::AurCubeMapAvailable  gog 0x0086b700  callconv=(blank)  param_size_bytes=0  gog_size=100  gog_retn=0
Candidate Steam address: 0x009852b0 (size 15, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x009852a0:STR0/CALLEE0/SIG0/SRV0/VT0
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
prologue for 0x009852b0: 16-byte aligned: YES
  009852b0  55             PUSH EBP
  009852b1  8B EC          MOV EBP,ESP
  009852b3  B9 D0 BC A7 00 MOV ECX,0xa7bcd0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x009852b0
string literals (0):
direct callees (1):
  0x00452F80  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 009852b0 FUN_009852b0 size=15 ret=0 callers=0 callees=1
THUNK -> 0x00452F80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_009852b0(void)

{
  FUN_00452f80();
  return;
}
```

### decoy: 0x009852a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x009852a0: 16-byte aligned: YES
  009852a0  55             PUSH EBP
  009852a1  8B EC          MOV EBP,ESP
  009852a3  B9 08 BC A7 00 MOV ECX,0xa7bc08
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x009852a0
string literals (0):
direct callees (1):
  0x00463B20  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 15 B, RET 0
- callees: 0x00452f80 (CALL)
- twins: 0x00984ec0, 0x00984ed0, 0x00984ee0, 0x00984ef0, 0x00985050, ... (distinguishing: 0x009852b3: MOV ECX,0xa7bcd0)

## Card B20-4-17

GOG function: CExoArrayList::DelIndex  gog 0x00903df0  callconv=__thiscall  param_size_bytes=4  gog_size=75  gog_retn=4
Candidate Steam address: 0x0049baf0 (size 29, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0049bb10:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00903d10 TimeTrigger::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00903e40 IODispatcher::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (27 of 89): the card's A-CALLEE credit may come from these, not from callees
  0x0041f840 CSWCCreature::SetSpecialWalkAnimation_2 -> Steam 0x00773970  in candidate callers: no
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callers: no
  0x00453160 CSWCArea::Update -> Steam 0x007a6760  in candidate callers: no
  0x00456ce0 CSWCArea::RemovePlaceableObjectLight -> Steam 0x007aa2e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0x14}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0049baf0: 16-byte aligned: YES
  0049baf0  55             PUSH EBP
  0049baf1  8B EC          MOV EBP,ESP
  0049baf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0049baf0
string literals (0):
direct callees (1):
  0x00474110  std::allocator<32B>::allocate(n) (map/set node)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0049baf0 FUN_0049baf0 size=29 ret=4 callers=1 callees=1
THUNK -> 0x00474110 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __thiscall FUN_0049baf0(undefined4 param_1,undefined4 param_2)

{
  FUN_00474110(param_2,0,param_1);
  return;
}
```

### decoy: 0x0049bb10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0049bb10: 16-byte aligned: YES
  0049bb10  55             PUSH EBP
  0049bb11  8B EC          MOV EBP,ESP
  0049bb13  81 EC 90 00 00 00 SUB ESP,0x90
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0049bb10
string literals (0):
direct callees (3):
  0x0091A0F8  (unnamed)
  0x0049BC20  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x18 [dec 24]  x5  via EAX,ECX,EDX
  +0x1C [dec 28]  x1  via EDX
```

### Static facts (script output, report-only)
- size 29 B, RET 4
- callers 1: 0x0049b9a0
- callees: 0x00474110 (CALL)
- twins: none

## Card B20-4-18

GOG function: CExoArrayList::DelIndex  gog 0x00903df0  callconv=__thiscall  param_size_bytes=4  gog_size=75  gog_retn=4
Candidate Steam address: 0x0049b820 (size 25, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0049b840:STR0/CALLEE25/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00903d10 TimeTrigger::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00903e40 IODispatcher::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (27 of 89): the card's A-CALLEE credit may come from these, not from callees
  0x0041f840 CSWCCreature::SetSpecialWalkAnimation_2 -> Steam 0x00773970  in candidate callers: no
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callers: no
  0x00453160 CSWCArea::Update -> Steam 0x007a6760  in candidate callers: no
  0x00456ce0 CSWCArea::RemovePlaceableObjectLight -> Steam 0x007aa2e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {}
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
prologue for 0x0049b820: 16-byte aligned: YES
  0049b820  55             PUSH EBP
  0049b821  8B EC          MOV EBP,ESP
  0049b823  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0049b820
string literals (0):
direct callees (1):
  0x00449170  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0049b820 FUN_0049b820 size=25 ret=4 callers=5 callees=1
THUNK -> 0x00449170 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0049b820(undefined4 param_1)

{
  FUN_00449170(param_1);
  return;
}
```

### decoy: 0x0049b840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0049b840: 16-byte aligned: YES
  0049b840  55             PUSH EBP
  0049b841  8B EC          MOV EBP,ESP
  0049b843  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0049b840
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via EAX,EDX
```

### Static facts (script output, report-only)
- size 25 B, RET 4
- callers 5: 0x00463ff0, 0x00473e80, 0x004741e0, 0x0049b7d0, 0x00504140
- callees: 0x00449170 (CALL)
- twins: none
