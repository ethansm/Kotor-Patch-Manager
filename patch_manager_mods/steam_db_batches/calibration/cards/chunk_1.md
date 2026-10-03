# Review cards CAL chunk 1

Answer each card with one TSV row in `steam_db_batches/calibration/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card CAL-1-01

GOG function: CSWGuiButton::Destructor  gog 0x004ce630  callconv=__thiscall  param_size_bytes=0  gog_size=140  gog_retn=0
Candidate Steam address: 0x0041a3d0 (size 140, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0041a330:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ce600 CSWGuiControl::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ce6c0 CSWGuiNavigable::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam ?  in candidate callees: n/a
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x2*,0x70*,0xe8*,0x160*} | Steam {0x2*,0x70*,0xe8*,0x160*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0041a3d0: 16-byte aligned: YES
  0041a3d0  55             PUSH EBP
  0041a3d1  8B EC          MOV EBP,ESP
  0041a3d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0041a3d0
string literals (0):
direct callees (3):
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0041a3d0 FUN_0041a3d0 size=140 ret=0 callers=518 callees=3

void __fastcall FUN_0041a3d0(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_00948b41;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiButton::vftable;
  local_8 = 2;
  FUN_00417290(uVar1);
  local_8._0_1_ = 1;
  FUN_00414c60();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00414c60();
  local_8 = 0xffffffff;
  *param_1 = CSWGuiNavigable::vftable;
  FUN_00418640();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0041a330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0041a330: 16-byte aligned: YES
  0041a330  55             PUSH EBP
  0041a331  8B EC          MOV EBP,ESP
  0041a333  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0041a330
string literals (0):
direct callees (2):
  0x0041A510  (unnamed)
  0x00414A50  CSWGuiBorder::Constructor
struct offsets touched (top 3):
  +0x1D4 [dec 468]  x2  via EAX,ECX
  +0x58 [dec 88]  x1  via EDX
  +0x1D0 [dec 464]  x1  via ECX
```

## Card CAL-1-02

GOG function: CSWGuiControl::Destructor  gog 0x004ce500  callconv=__thiscall  param_size_bytes=0  gog_size=126  gog_retn=0
Candidate Steam address: 0x00418640 (size 126, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00418600:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ce3f0 CSWGuiMessageBox::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ce580 CGuiInGame::ClearMapDisplays  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x2*,0x1c*,0x2c*,0x3c*} | Steam {0x2*,0x1c*,0x2c*,0x3c*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00418640: 16-byte aligned: YES
  00418640  55             PUSH EBP
  00418641  8B EC          MOV EBP,ESP
  00418643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00418640
string literals (0):
direct callees (2):
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00418640 FUN_00418640 size=126 ret=0 callers=432 callees=2

void __fastcall FUN_00418640(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0094892e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiControl::vftable;
  local_8 = 2;
  Free(uVar1);
  local_8._0_1_ = 1;
  FUN_00733780();
  local_8 = (uint)local_8._1_3_ << 8;
  Free();
  *param_1 = CSWGuiObject::vftable;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00418600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00418600: 16-byte aligned: YES
  00418600  55             PUSH EBP
  00418601  8B EC          MOV EBP,ESP
  00418603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00418600
string literals (0):
direct callees (2):
  0x00418640  CSWGuiControl::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card CAL-1-03

GOG function: CClientExoApp::GetAutoPaused  gog 0x0040de20  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x007409c0 (size 22, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::DisableVideoEffect
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007409a0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ddf0 CClientExoApp::SetAutoPaused  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040de40 CClientExoApp::SetLoadScreenHint  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x384} | Steam {0x4*}
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
prologue for 0x007409c0: 16-byte aligned: YES
  007409c0  55             PUSH EBP
  007409c1  8B EC          MOV EBP,ESP
  007409c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007409c0
string literals (0):
direct callees (1):
  0x0079E960  CClientExoAppInternal::DisableVideoEffect
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007409c0 FUN_007409c0 size=22 ret=0 callers=7 callees=1
THUNK -> 0x0079E960 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007409c0(undefined4 param_1)

{
  FUN_0079e960(param_1);
  return;
}
```

### decoy: 0x007409a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007409a0: 16-byte aligned: YES
  007409a0  55             PUSH EBP
  007409a1  8B EC          MOV EBP,ESP
  007409a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007409a0
string literals (0):
direct callees (1):
  0x0079E550  CClientExoAppInternal::EnableVideoEffect
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card CAL-1-04

GOG function: CGuiInGame::ShutDownGlobal  gog 0x004cdda0  callconv=(blank)  param_size_bytes=4  gog_size=173  gog_retn=4
Candidate Steam address: 0x0073ffc0 (size 28, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::SetPlayerCharacterName
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073ffa0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004cd090 CGuiInGame::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004cde50 CGuiInGame::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x009217f0 Global::`eh_vector_destructor_iterator' -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4}
imm sets (* = in both, 1 common): GOG {0x2,0x4*,0x8,0x10} | Steam {0x4*}

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

## Card CAL-1-05

GOG function: CClientExoApp::GetCreatureByGameObjectID  gog 0x0040ca10  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f550 (size 28, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f530:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c9f0 CClientExoApp::GetItemByGameObjectID  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040ca30 CClientExoApp::GetAreaByGameObjectID  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004383d0 CClientExoAppInternal::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f550: 16-byte aligned: YES
  0073f550  55             PUSH EBP
  0073f551  8B EC          MOV EBP,ESP
  0073f553  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f550
string literals (0):
direct callees (1):
  0x0078BDF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f550 FUN_0073f550 size=28 ret=4 callers=72 callees=1
THUNK -> 0x0078BDF0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f550(undefined4 param_1)

{
  FUN_0078bdf0(param_1);
  return;
}
```

### decoy: 0x0073f530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f530: 16-byte aligned: YES
  0073f530  55             PUSH EBP
  0073f531  8B EC          MOV EBP,ESP
  0073f533  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f530
string literals (0):
direct callees (1):
  0x0078BD80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card CAL-1-06

GOG function: CResGFF::WriteFieldCHAR  gog 0x00625ba0  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x00719a40 (size 97, retn 12)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007199d0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625b30 CResGFF::WriteFieldBYTE  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00625c10 CResGFF::WriteFieldWORD  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam ?  in candidate callees: n/a
  0x00624190 CResGFF::AddField -> Steam ?  in candidate callees: n/a
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

## Card CAL-1-07

GOG function: CResGFF::OnResourceFreed  gog 0x00623710  callconv=(blank)  param_size_bytes=0  gog_size=124  gog_retn=0
Candidate Steam address: 0x007175b0 (size 124, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00717630:STR0/CALLEE0/SIG0.222/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00623460 CResGFF::OnResourceServiced -> 0x00717300  trusted
- GOG-order neighbour after:  0x00623790 CResGFF::ReleaseResource -> 0x00717630  trusted

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

## Card CAL-1-08

GOG function: CSWSDoor::GetFirstName  gog 0x007623f0  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x00616f90 (size 500, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSDoor::Destructor_2
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00616f60:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007623b0 CSWSDoor::GetDialogResref -> 0x00616f00  trusted
- GOG-order neighbour after:  0x00762410 CSWSDoor::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x37c,0x3d0}
imm sets (* = in both, 1 common): GOG {0x3ec*} | Steam {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0xf,0x18,0x24,0x268,0x320,0x32c,0x3d8,...(+4)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00616f90: 16-byte aligned: YES
  00616f90  55             PUSH EBP
  00616f91  8B EC          MOV EBP,ESP
  00616f93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00616f90
string literals (0):
direct callees (13):
  0x005453C0  CSWSObject::GetArea
  0x0061D040  CSWSDoor::RemoveFromArea
  0x0051C280  (unnamed)
  0x0051E0C0  CServerAIMaster::RemoveObject
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x0055EA80  (unnamed)
  0x006F3F10  (unnamed)
  0x009196FD  operator delete
  0x00735360  CExoLocString::Destructor
  0x00733780  CExoString::~CExoString
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0053F050  CSWSObject::Destructor_2
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via EAX
  +0x3D0 [dec 976]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00616f90 FUN_00616f90 size=500 ret=0 callers=1 callees=13

void __fastcall FUN_00616f90(undefined4 *param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined *local_18;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_00957042;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWSDoor::vftable;
  local_8 = 8;
  iVar2 = FUN_005453c0(uVar1);
  if (iVar2 != 0) {
    FUN_0061d040();
  }
  puVar3 = param_1;
  FUN_0051c280(param_1);
  FUN_0051e0c0(puVar3);
  iVar2 = FUN_0051bd90();
  if (iVar2 != 0) {
    FUN_0055ea80(param_1 + 0x6 [dec 6] {undefined4*: byte +0x18},param_1[1 = byte +0x4]);
  }
  for (local_18 = PTR_0099b4b4; (int)local_18 <= DAT_0099b4bc; local_18 = local_18 + 1) {
    if (param_1[(int)(local_18 + 0xdf)] != 0) {
      iVar2 = param_1[(int)(local_18 + 0xdf)];
      if (iVar2 != 0) {
        FUN_006f3f10();
        FUN_009196fd(iVar2);
      }
      param_1[(int)(local_18 + 0xdf)] = 0;
    }
  }
  param_1[0xf4 = byte +0x3d0] = 0;
  local_8._0_1_ = 7;
  FUN_00735360();
  local_8._0_1_ = 6;
  FUN_00735360();
  local_8._0_1_ = 5;
  FUN_00735360();
  local_8._0_1_ = 4;
  FUN_00733780();
  local_8._0_1_ = 3;
  FUN_00733780();
  local_8._0_1_ = 2;
  Free();
  local_8._0_1_ = 1;
  FUN_00733780();
  local_8 = (uint)local_8._1_3_ << 8;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x9a,8,0xf,FUN_00733780);
  local_8 = 0xffffffff;
  FUN_0053f050();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00616f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00616f60: 16-byte aligned: YES
  00616f60  55             PUSH EBP
  00616f61  8B EC          MOV EBP,ESP
  00616f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00616f60
string literals (0):
direct callees (2):
  0x00616F90  CSWSDoor::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card CAL-1-09

GOG function: CSWSCreature::CanUseItem  gog 0x006ddba0  callconv=(blank)  param_size_bytes=12  gog_size=821  gog_retn=12
Candidate Steam address: 0x005d2f80 (size 821, retn 12)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005d2ea0:STR0/CALLEE0/SIG0.087/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006ddac0 CSWSCreature::AIActionStanceMeditative  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006ddee0 CSWSCreature::RemoveFromInvitationsOffered  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x004ecc00 CSWItem::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x0064b760 CServerExoApp::GetServerInfo -> Steam ?  in candidate callees: n/a
  0x0066a460 CSWSItem::GetMinEquipLevel -> Steam ?  in candidate callees: n/a
  0x006c8800 CSWSCreature::CheckProficiencies -> Steam ?  in candidate callees: n/a
  0x006ddee0 CSWSCreature::RemoveFromInvitationsOffered -> Steam ?  in candidate callees: n/a
  0x006de0a0 CSWSCreature::CheckItemAlignmentRestrictions -> Steam ?  in candidate callees: n/a
  0x006de270 CSWSCreature::CheckItemRaceRestrictions -> Steam ?  in candidate callees: n/a
  ... 6 more GOG callees
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0x9*,0xc*,0x1b*,0x34*,0x98*,0x9c*,0xac*,0xd0*,0x53c*,0x10e8*,0x1198*} | Steam {0x4*,0x8*,0x9*,0xc*,0x1b*,0x34*,0x98*,0x9c*,0xac*,0xd0*,0x53c*,0x10e8*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x8*,0xc*,0x14*,0x1a*,0x2d*,0x58*} | Steam {0x8*,0xc*,0x14*,0x1a*,0x2d*,0x58*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x005d2f80: 16-byte aligned: YES
  005d2f80  55             PUSH EBP
  005d2f81  8B EC          MOV EBP,ESP
  005d2f83  83 EC 58       SUB ESP,0x58
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d2f80
string literals (0):
direct callees (14):
  0x005D3480  CSWSCreature::CheckItemAlignmentRestrictions
  0x005D32C0  CSWSCreature::RemoveFromInvitationsOffered
  0x005D3650  CSWSCreature::CheckItemRaceRestrictions
  0x006D6E30  (unnamed)
  0x005D3A20  CSWSCreature::CheckItemFeatRestrictions
  0x005D3B60  CSWSCreature::CheckItemClassRestrictions
  0x005D3C90  CSWSCreature::CheckItemPCRestrictions
  0x005D3EE0  CSWSCreature::CheckItemAttributeRestrictions
  0x005D4190  CSWSCreature::CheckItemCNPCRestrictions
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x00606C30  CSWSItem::GetMinEquipLevel
  0x0051BD40  GetServerInfo/JoiningRestrictions accessor (CanLevelUp reads +0x94 MaxLevel)
  0x006D5200  (unnamed)
  0x005BDA40  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x34 [dec 52]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x9 [dec 9]  x1  via ECX
  +0x1B [dec 27]  x1  via EDX
  +0x98 [dec 152]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0xAC [dec 172]  x1  via ECX
  +0xD0 [dec 208]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005d2f80 FUN_005d2f80 size=821 ret=12 callers=8 callees=14

/* WARNING: Removing unreachable block (ram,0x005d3177) */

bool __thiscall FUN_005d2f80(int *param_1,int param_2,undefined4 param_3,int param_4)

{
  int iVar1;
  bool bVar2;
  byte bVar3;
  byte bVar4;
  short sVar5;
  short sVar6;
  int iVar7;
  int iVar8;
  bool bVar9;
  int local_c;
  
  if (param_2 == 0) {
    bVar9 = false;
  }
  else {
    iVar7 = FUN_005d3480(param_2);
    bVar9 = iVar7 != 0;
    if ((bVar9) && (iVar7 = FUN_005d32c0(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if (bVar9 == true) {
      iVar7 = FUN_005d3650(param_2,param_4);
      if (iVar7 == 0) {
        bVar9 = false;
      }
      else {
        iVar7 = FUN_006d6e30();
        if ((param_4 != 0) &&
           ((*(char *)(iVar7 + 0xac) == '-' || (*(char *)(iVar7 + 0xac) == '\x1a')))) {
          sVar5 = (**(code **)(*param_1 + 0x9c))(0);
          sVar6 = (**(code **)(*param_1 + 0x98))(1);
          if (sVar5 == sVar6) {
            bVar9 = false;
          }
        }
      }
    }
    if ((bVar9 == true) && (iVar7 = FUN_005d3a20(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if ((bVar9 == true) && (iVar7 = FUN_005d3b60(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if ((bVar9 == true) && (iVar7 = FUN_005d3c90(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if ((bVar9 == true) && (iVar7 = FUN_005d3ee0(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if ((bVar9 == true) && (iVar7 = FUN_005d4190(param_2), iVar7 == 0)) {
      bVar9 = false;
    }
    if (param_1[0x43a = byte +0x10e8] == 1) {
      bVar3 = FUN_006a4e00(0);
      bVar4 = FUN_00606c30();
      if ((bVar3 < bVar4) && (iVar7 = FUN_0051bd40(), *(int *)(iVar7 + 0xd0) != 0)) {
        bVar9 = false;
      }
    }
    if (bVar9 == true) {
      iVar7 = FUN_006d5200(*(undefined4 *)(param_2 + 0xc));
      if ((((iVar7 != 0) && (*(char *)(iVar7 + 8) != '\b')) && (*(char *)(iVar7 + 9) != '\0')) &&
         (iVar1 = param_1[0x14f = byte +0x53c], iVar8 = FUN_006d5200(*(undefined4 *)(param_2 + 0xc)),
         1 < (int)((uint)*(byte *)(iVar8 + 0x1b) - iVar1))) {
        bVar9 = false;
      }
      bVar2 = false;
      for (local_c = 0; local_c < 0x14; local_c = local_c + 1) {
        if ((1 << ((byte)local_c & 0x1f) & *(uint *)(iVar7 + 4)) != 0) {
          bVar2 = true;
          break;
        }
      }
      if ((bVar2) && (iVar7 = FUN_005bda40(param_2,1 << ((byte)local_c & 0x1f)), iVar7 == 0)) {
/* ... 6 more lines truncated by --max-lines */
```

### decoy: 0x005d2ea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d2ea0: 16-byte aligned: YES
  005d2ea0  55             PUSH EBP
  005d2ea1  8B EC          MOV EBP,ESP
  005d2ea3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d2ea0
string literals (0):
direct callees (3):
  0x00563D20  CSWSCreature::ActivityManager
  0x0057A990  CSWSCreature::RecomputeAmbientAnimationState
  0x005210F0  (unnamed)
struct offsets touched (top 4):
  +0x110C [dec 4364]  x3  via EAX,EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EAX
```

## Card CAL-1-10

GOG function: CSWGuiSaveLoad::LaunchMostRecentSave  gog 0x005d86f0  callconv=(blank)  param_size_bytes=0  gog_size=294  gog_retn=0
Candidate Steam address: 0x00859250 (size 24, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00859270:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d86d0 CSWGuiSaveLoad::SetEndGameMode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d8840 CExoArrayList_CSWGuiControl__::Insert  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00512940 unnamed -> Steam ?  in candidate callees: n/a
  0x005d1680 CSWGuiSaveLoadEntry::GetGameDirectory -> Steam ?  in candidate callees: n/a
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00606e70 CExoString::CompareNoCase -> Steam 0x00734f60  in candidate callees: no
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x0091df10 CSWGuiListBox::GetControl -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8} | Steam {0x1e40}
imm sets (* = in both, 0 common): GOG {0x2c,0x970} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00859250: 16-byte aligned: YES
  00859250  55             PUSH EBP
  00859251  8B EC          MOV EBP,ESP
  00859253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00859250
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1E40 [dec 7744]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00859250 FUN_00859250 size=24 ret=0 callers=2 callees=0

void __fastcall FUN_00859250(int param_1)

{
  *(undefined4 *)(param_1 + 0x1e40) = 1;
  return;
}
```

### decoy: 0x00859270 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00859270: 16-byte aligned: YES
  00859270  55             PUSH EBP
  00859271  8B EC          MOV EBP,ESP
  00859273  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00859270
string literals (0):
direct callees (7):
  0x0041FF70  (unnamed)
  0x00852340  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005355F0  (unnamed)
  0x00734D60  CExoString::CompareNoCase
  0x00853A00  CSWGuiSaveLoad::LoadGame
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

## Card CAL-1-11

GOG function: CClientExoApp::AddMoveToModuleMovie  gog 0x0040d640  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740180 (size 28, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740160:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d620 CClientExoApp::IsMoviePlaying  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d660 CClientExoApp::GetMoveToModuleMovies  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00445190 CClientExoAppInternal::AddMoveToModuleMovie -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740180: 16-byte aligned: YES
  00740180  55             PUSH EBP
  00740181  8B EC          MOV EBP,ESP
  00740183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740180
string literals (0):
direct callees (1):
  0x00798B70  CClientExoAppInternal::AddMoveToModuleMovie
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740180 FUN_00740180 size=28 ret=4 callers=3 callees=1
THUNK -> 0x00798B70 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740180(undefined4 param_1)

{
  FUN_00798b70(param_1);
  return;
}
```

### decoy: 0x00740160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740160: 16-byte aligned: YES
  00740160  55             PUSH EBP
  00740161  8B EC          MOV EBP,ESP
  00740163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740160
string literals (0):
direct callees (1):
  0x00798C00  IsMoviePlaying
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card CAL-1-12

GOG function: CGuiInGame::AppendToMsgBuffer  gog 0x004ce720  callconv=(blank)  param_size_bytes=9  gog_size=282  gog_retn=12
Candidate Steam address: 0x007be090 (size 282, retn 12)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiInGame::AddFeedbackLine
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007be1b0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ce6e0 CSWGuiNavigable::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ce840 CGuiInGame::AppendToCombatBuffer  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605c70 CExoString::operator!=_2 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0xc*,0x110*,0x11c*} | Steam {0x8*,0xc*,0x110*,0x11c*}
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
prologue for 0x007be090: 16-byte aligned: YES
  007be090  55             PUSH EBP
  007be091  8B EC          MOV EBP,ESP
  007be093  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007be090 FUN_007be090 size=282 ret=12 callers=9 callees=2

void __thiscall FUN_007be090(int param_1,undefined4 param_2,undefined4 param_3,undefined1 param_4)

{
  int iVar1;
  int iVar2;
  ushort local_8;
  
  iVar1 = FUN_00733b60(&DAT_00986925);
  if (iVar1 != 0) {
    if (0x3f < *(uint *)(param_1 + 0x11c)) {
      *(undefined4 *)(param_1 + 0x11c) = 0x3f;
      for (local_8 = 0; (uint)local_8 < *(uint *)(param_1 + 0x11c); local_8 = local_8 + 1) {
        iVar1 = (local_8 + 1) * 0x10 + *(int *)(param_1 + 0x110);
        iVar2 = (uint)local_8 * 0x10 + *(int *)(param_1 + 0x110);
        FUN_007337c0(iVar1);
        *(undefined4 *)(iVar2 + 8) = *(undefined4 *)(iVar1 + 8);
        *(undefined1 *)(iVar2 + 0xc) = *(undefined1 *)(iVar1 + 0xc);
      }
    }
    FUN_007337c0(param_2);
    *(undefined4 *)(*(int *)(param_1 + 0x110) + 8 + *(int *)(param_1 + 0x11c) * 0x10) = param_3;
    *(undefined1 *)(*(int *)(param_1 + 0x110) + 0xc + *(int *)(param_1 + 0x11c) * 0x10) = param_4;
    *(int *)(param_1 + 0x11c) = *(int *)(param_1 + 0x11c) + 1;
  }
  return;
}
```

### decoy: 0x007be1b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007be1b0: 16-byte aligned: YES
  007be1b0  55             PUSH EBP
  007be1b1  8B EC          MOV EBP,ESP
  007be1b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card CAL-1-13

GOG function: CSWSObject::GetAIStateReputation  gog 0x006bd870  callconv=(blank)  param_size_bytes=4  gog_size=159  gog_retn=4
Candidate Steam address: 0x005e17e0 (size 324, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.39 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e1720:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006bd5b0 CSWSObject::GetReputation  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006bd9d0 CSWSCreature::AIActionFlagMine  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam ?  in candidate callees: n/a
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x006bd5b0 CSWSObject::GetReputation -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0x10} | Steam {0x4*,0x8*}
imm sets (* = in both, 1 common): GOG {0x2,0x4*,0xa,0x1c,0x32,0x5a} | Steam {0x3,0x4*,0x8,0x28}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005e17e0: 16-byte aligned: YES
  005e17e0  55             PUSH EBP
  005e17e1  8B EC          MOV EBP,ESP
  005e17e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005e17e0
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x0091D67E  (unnamed)
  0x00735400  (unnamed)
  0x00401D90  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005e17e0 FUN_005e17e0 size=324 ret=4 callers=3 callees=4

void __thiscall FUN_005e17e0(int *param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  uint *local_34;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094a2cb;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar1 = *param_1;
  param_1[2 = byte +0x8] = param_2;
  uVar2 = param_1[2 = byte +0x8];
  uVar4 = -(uint)((int)((ulonglong)uVar2 * 8 >> 0x20) != 0) | (uint)((ulonglong)uVar2 * 8);
  local_34 = (uint *)FUN_00919723(-(uint)(0xfffffffb < uVar4) | uVar4 + 4,uVar3);
  local_8 = 0;
  if (local_34 == (uint *)0x0) {
    local_34 = (uint *)0x0;
  }
  else {
    *local_34 = uVar2;
    _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_34 + 0x1 [dec 1] {uint*: byte +0x4},8,uVar2,FUN_00735240,FUN_00735360);
    local_34 = local_34 + 0x1 [dec 1] {uint*: byte +0x4};
  }
  local_8 = 0xffffffff;
  *param_1 = (int)local_34;
  for (local_18 = 0; local_18 < param_1[1 = byte +0x4]; local_18 = local_18 + 1) {
    FUN_00735400(iVar1 + local_18 * 8);
  }
  if ((iVar1 != 0) && (iVar1 != 0)) {
    FID_conflict__vector_deleting_destructor_ [GHIDRA-AUTO-LABEL: unreliable](3);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x005e1720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e1720: 16-byte aligned: YES
  005e1720  55             PUSH EBP
  005e1721  8B EC          MOV EBP,ESP
  005e1723  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005e1720
string literals (0):
direct callees (3):
  0x005E17E0  (unnamed)
  0x00735400  (unnamed)
  0x00735360  CExoLocString::Destructor
struct offsets touched (top 2):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
```

## Card CAL-1-14

GOG function: CSWCObject::AnimationLooping  gog 0x00474660  callconv=(blank)  param_size_bytes=2  gog_size=197  gog_retn=4
Candidate Steam address: 0x007ef5d0 (size 207, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCObject::AnimationFireAndForget
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.62 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef500:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004745d0 CSWCObject::AnimationWalking  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00474730 CSWCObject::AnimationFireAndForget  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x8*,0x68*,0xcc*,0xe0*,0x130*} | Steam {0x8*,0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 3 common): GOG {0x4*,0x5*,0x10*,0x578,0x5f7,0x640,0x6bf} | Steam {0x4*,0x5*,0x9,0x10*,0x3e8,0x467,0x4b0,0x52f}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef5d0: 16-byte aligned: YES
  007ef5d0  55             PUSH EBP
  007ef5d1  8B EC          MOV EBP,ESP
  007ef5d3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef5d0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 5):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef5d0 FUN_007ef5d0 size=207 ret=4 callers=11 callees=1

bool __thiscall FUN_007ef5d0(int param_1,ushort param_2)

{
  int iVar1;
  bool bVar2;
  int local_8;
  
  if (((*(char *)(param_1 + 8) == '\x05') || (*(char *)(param_1 + 8) == '\t')) &&
     (((999 < param_2 && (param_2 < 0x468)) || ((0x4af < param_2 && (param_2 < 0x530)))))) {
    bVar2 = true;
  }
  else {
    local_8 = 0;
    if (*(int *)(param_1 + 0x68) != 0) {
      param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
    }
    iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe1c,&local_8);
    if (iVar1 == 0) {
      bVar2 = false;
    }
    else {
      bVar2 = local_8 != 0;
    }
  }
  return bVar2;
}
```

### decoy: 0x007ef500 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef500: 16-byte aligned: YES
  007ef500  55             PUSH EBP
  007ef501  8B EC          MOV EBP,ESP
  007ef503  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef500
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 5):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card CAL-1-15

GOG function: CGuiInGame::GetMiniMapVisible  gog 0x004e0c60  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x007d0560 (size 49, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::RemoveGUIPopUps
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007d0530:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0c30 CGuiInGame::SetCombatMessage  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004e0ca0 CGuiInGame::RemoveGUIPopUps -> 0x007d05a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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

## Card CAL-1-16

GOG function: CClientExoApp::SetPlayerCreature  gog 0x0040cdf0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0073f910 (size 28, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f8f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cdd0 CClientExoApp::EmptyHitcheckIgnoreList  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040ce10 CClientExoApp::HandleMessage  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444c80 CClientExoAppInternal::SetPlayerCreature -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f910: 16-byte aligned: YES
  0073f910  55             PUSH EBP
  0073f911  8B EC          MOV EBP,ESP
  0073f913  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f910
string literals (0):
direct callees (1):
  0x00798660  CClientExoAppInternal::SetPlayerCreature
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f910 FUN_0073f910 size=28 ret=4 callers=3 callees=1
THUNK -> 0x00798660 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f910(undefined4 param_1)

{
  FUN_00798660(param_1);
  return;
}
```

### decoy: 0x0073f8f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f8f0: 16-byte aligned: YES
  0073f8f0  55             PUSH EBP
  0073f8f1  8B EC          MOV EBP,ESP
  0073f8f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f8f0
string literals (0):
direct callees (1):
  0x0078D220  CClientExoAppInternal::EmptyHitcheckIgnoreList
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card CAL-1-17

GOG function: CSWCObject::AnimationWalking  gog 0x004745d0  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef3e0 (size 129, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef350:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474540 CSWCObject::AnimationHideEquippedItems -> 0x007ef910  trusted
- GOG-order neighbour after:  0x00474660 CSWCObject::AnimationLooping -> 0x007ef500  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xcc*,0xe0*,0x130*} | Steam {0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef3e0: 16-byte aligned: YES
  007ef3e0  55             PUSH EBP
  007ef3e1  8B EC          MOV EBP,ESP
  007ef3e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef3e0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef3e0 FUN_007ef3e0 size=129 ret=4 callers=1 callees=1

bool __thiscall FUN_007ef3e0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe30,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef350: 16-byte aligned: YES
  007ef350  55             PUSH EBP
  007ef351  8B EC          MOV EBP,ESP
  007ef353  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef350
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card CAL-1-18

GOG function: CSWSObject::GetLastName  gog 0x006a5ce0  callconv=__thiscall  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x0053f000 (size 18, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::GetPortraitId
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053efd0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a55e0 CSWSObject::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006a5d00 CSWSObject::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x30}
imm sets (* = in both, 0 common): GOG {0xc} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053f000: 16-byte aligned: YES
  0053f000  55             PUSH EBP
  0053f001  8B EC          MOV EBP,ESP
  0053f003  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053f000
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x30 [dec 48]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0053f000 FUN_0053f000 size=18 ret=0 callers=0 callees=0

undefined2 __fastcall FUN_0053f000(int param_1)

{
  return *(undefined2 *)(param_1 + 0x30);
}
```

### decoy: 0x0053efd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053efd0: 16-byte aligned: YES
  0053efd0  55             PUSH EBP
  0053efd1  8B EC          MOV EBP,ESP
  0053efd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053efd0
string literals (0):
direct callees (1):
  0x00710CA0  (unnamed)
struct offsets touched (top 0):
```

## Card CAL-1-19

GOG function: CResGFF::ReadFieldCExoString  gog 0x006250f0  callconv=(blank)  param_size_bytes=20  gog_size=322  gog_retn=20
Candidate Steam address: 0x00718f90 (size 322, retn 20)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007190e0:STR0/CALLEE0/SIG0.111/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00624fa0 CResGFF::ReadFieldCResRef -> 0x00718e40  trusted
- GOG-order neighbour after:  0x00625240 CResGFF::ReadFieldCExoLocString -> 0x007190e0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605790 CExoString::Constructor_4 -> Steam ?  in candidate callees: n/a
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x00623980 CResGFF::GetDataField -> Steam ?  in candidate callees: n/a
  0x00623a40 CResGFF::GetFieldByLabel -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0xa*,0x14*,0x1c*} | Steam {0x4*,0xa*,0x14*,0x1c*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x00718f90: 16-byte aligned: YES
  00718f90  55             PUSH EBP
  00718f91  8B EC          MOV EBP,ESP
  00718f93  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00718f90 FUN_00718f90 size=322 ret=20 callers=42 callees=5

undefined4
FUN_00718f90(undefined4 param_1,int param_2,int param_3,undefined4 *param_4,undefined4 param_5)

{
  undefined4 uVar1;
  uint local_14;
  int *local_10;
  int *local_c;
  int local_8;
  
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    FUN_007335f0(param_5);
  }
  else {
    uVar1 = FUN_007178e0(param_2,param_3);
    local_c = (int *)FUN_00717770(param_2,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 10)) {
      *param_4 = 0;
      FUN_007335f0(param_5);
    }
    else {
      local_10 = (int *)FUN_00717820(local_c,&local_14);
      if ((local_10 == (int *)0x0) || (local_14 < 4)) {
        *param_4 = 0;
        FUN_007335f0(param_5);
      }
      else {
        local_8 = *local_10;
        if (local_14 < local_8 + 4U) {
          *param_4 = 0;
          FUN_007335f0(param_5);
        }
        else {
          *param_4 = 1;
          FUN_00733680(local_10 + 0x1 [dec 1] {int*: byte +0x4},local_8);
        }
      }
    }
  }
  return param_1;
}
```

### decoy: 0x007190e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007190e0: 16-byte aligned: YES
  007190e0  55             PUSH EBP
  007190e1  8B EC          MOV EBP,ESP
  007190e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card CAL-1-20

GOG function: Gob::RemoveAnimationFlag  gog 0x008fd290  callconv=(blank)  param_size_bytes=4  gog_size=82  gog_retn=4
Candidate Steam address: 0x004e3160 (size 82, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004e3110:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008fd240 Gob::AddAnimationFlag  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008fd2f0 Global::GetCurrentRun  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0xdc*,0xe0*} | Steam {0x8*,0xdc*,0xe0*}
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
prologue for 0x004e3160: 16-byte aligned: YES
  004e3160  55             PUSH EBP
  004e3161  8B EC          MOV EBP,ESP
  004e3163  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004e3160
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xDC [dec 220]  x1  via EDX
  +0xE0 [dec 224]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004e3160 FUN_004e3160 size=82 ret=4 callers=0 callees=0

void __thiscall FUN_004e3160(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0xe0) + -1;
  if (((-1 < iVar1) &&
      (iVar1 = *(int *)(*(int *)(param_1 + 0xdc) + iVar1 * 4), *(int *)(iVar1 + 8) != 0)) &&
     (param_2 != 0)) {
    *(int *)(iVar1 + 8) = *(int *)(iVar1 + 8) - param_2;
  }
  return;
}
```

### decoy: 0x004e3110 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004e3110: 16-byte aligned: YES
  004e3110  55             PUSH EBP
  004e3111  8B EC          MOV EBP,ESP
  004e3113  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004e3110
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xDC [dec 220]  x1  via EDX
  +0xE0 [dec 224]  x1  via EAX
```

## Card CAL-1-21

GOG function: CSWCObject::GetCurrentAnimation  gog 0x00472930  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x007ed7d0 (size 18, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (ResetDialogAnimations 0x7c93e0)
- best decoy (neighbour/other candidate) scores: 0x007ed5f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004727b0 CSWCObject::Destructor_3  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00472950 CSWCObject::AttachToScene  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x48*} | Steam {0x48*}
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
prologue for 0x007ed7d0: 16-byte aligned: YES
  007ed7d0  55             PUSH EBP
  007ed7d1  8B EC          MOV EBP,ESP
  007ed7d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed7d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x48 [dec 72]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed7d0 FUN_007ed7d0 size=18 ret=0 callers=22 callees=0

undefined2 __fastcall FUN_007ed7d0(int param_1)

{
  return *(undefined2 *)(param_1 + 0x48);
}
```

### decoy: 0x007ed5f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed5f0: 16-byte aligned: YES
  007ed5f0  55             PUSH EBP
  007ed5f1  8B EC          MOV EBP,ESP
  007ed5f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007ed5f0
string literals (0):
direct callees (11):
  0x0073F8D0  CClientExoApp::AddObjectToHitcheckIgnoreList
  0x007F0A50  CSWCObject::KillAllVisualEffects
  0x0073F790  CClientExoApp::GetClientAIMaster
  0x007A2540  (unnamed)
  0x0070B080  StreamingSource scalar delete
  0x009196FD  operator delete
  0x0054A900  CSWSObject::DetachFromClientObject
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0072F5F0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x5C [dec 92]  x3  via ECX,EDX
  +0xC8 [dec 200]  x3  via EAX,ECX
  +0xCC [dec 204]  x3  via EAX,EDX
  +0xF8 [dec 248]  x3  via EAX,ECX,EDX
```

## Card CAL-1-22

GOG function: Gob::SetProcessFlag  gog 0x0084bb30  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00459560 (size 31, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459530:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bb00 CAurPathInfo::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084bb80 Global::return_void_0x4  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00855f80 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00459560: 16-byte aligned: YES
  00459560  55             PUSH EBP
  00459561  8B EC          MOV EBP,ESP
  00459563  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459560
string literals (0):
direct callees (1):
  0x00463A90  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00459560 FUN_00459560 size=31 ret=0 callers=2 callees=1

void __fastcall FUN_00459560(undefined4 *param_1)

{
  *param_1 = CAurObject::vftable;
  FUN_00463a90();
  return;
}
```

### decoy: 0x00459530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459530: 16-byte aligned: YES
  00459530  55             PUSH EBP
  00459531  8B EC          MOV EBP,ESP
  00459533  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459530
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card CAL-1-23

GOG function: CWorldTimer::GetWorldTimeHour  gog 0x0080b5e0  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x0051b0e0 (size 32, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::GetWorldTimeTimeOfDay
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051b100:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080b5c0 Global::GetWorldTimeTimeOfDay  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0080b620 CWorldTimer::GetWorldTimeMinute -> 0x0051b140  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0080b020 CWorldTimer::ConvertFromTimeOfDay -> Steam ?  in candidate callees: n/a
  0x0080b5c0 Global::GetWorldTimeTimeOfDay -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0x14} | Steam {0xc}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051b0e0: 16-byte aligned: YES
  0051b0e0  55             PUSH EBP
  0051b0e1  8B EC          MOV EBP,ESP
  0051b0e3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051b0e0
string literals (0):
direct callees (1):
  0x0051AD20  CWorldTimer::GetWorldTime
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051b0e0 FUN_0051b0e0 size=32 ret=0 callers=4 callees=1

undefined4 FUN_0051b0e0(void)

{
  undefined1 local_c [4];
  undefined4 local_8;
  
  FUN_0051ad20(local_c,&local_8);
  return local_8;
}
```

### decoy: 0x0051b100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051b100: 16-byte aligned: YES
  0051b100  55             PUSH EBP
  0051b101  8B EC          MOV EBP,ESP
  0051b103  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051b100
string literals (0):
direct callees (2):
  0x0051B0E0  Global::GetWorldTimeTimeOfDay
  0x0051AB40  CWorldTimer::ConvertFromTimeOfDay
struct offsets touched (top 0):
```

## Card CAL-1-24

GOG function: Global::AurTextureGetMaxTexID  gog 0x0081a150  callconv=(blank)  param_size_bytes=0  gog_size=10  gog_retn=0
Candidate Steam address: 0x00427a60 (size 10, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00427970:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0081a060 Global::AurTextureGetOrdering  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0081a160 Global::AurTextureCalculateNewTexMem  (not in this batch, no trusted mapping)

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
prologue for 0x00427a60: 16-byte aligned: YES
  00427a60  55             PUSH EBP
  00427a61  8B EC          MOV EBP,ESP
  00427a63  A1 84 BB A1 00 MOV EAX,[0x00a1bb84]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00427a60
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00427a60 FUN_00427a60 size=10 ret=0 callers=1 callees=0

undefined4 FUN_00427a60(void)

{
  return DAT_00a1bb84;
}
```

### decoy: 0x00427970 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00427970: 16-byte aligned: YES
  00427970  55             PUSH EBP
  00427971  8B EC          MOV EBP,ESP
  00427973  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00427970
string literals (0):
direct callees (1):
  0x004491C0  CExoPtrList::Add (doubling)
struct offsets touched (top 4):
  +0x40 [dec 64]  x4  via EAX,ECX,EDX
  +0xC6 [dec 198]  x1  via EDX
  +0xC8 [dec 200]  x1  via ECX
  +0xE5 [dec 229]  x1  via EDX
```

## Card CAL-1-25

GOG function: Gob::IsInCutscene  gog 0x0084c400  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00459e20 (size 17, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459e00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c3e0 Gob::DisableInCutscene -> 0x00459e00  trusted
- GOG-order neighbour after:  0x0084c420 Gob::GetCutsceneDummyPosition -> 0x00459e40  trusted

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

## Card CAL-1-26

GOG function: CSWGuiSaveLoad::LoadGame  gog 0x005d33b0  callconv=(blank)  param_size_bytes=4  gog_size=2611  gog_retn=4
Candidate Steam address: 0x00853a00 (size 2611, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 0.93 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 5 (SendLoadGameRequest 0x73f720,DisableInput 0x7402d0,SetLoadFromSaveGame 0x51ce00); SIG 0.93
- best decoy (neighbour/other candidate) scores: 0x00854440:STR0/CALLEE0/SIG0.226/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d2e00 CStatusSummary::LoadFromGFF  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d3df0 CSWGuiSaveLoad::PromptForSaveName  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (47), mapped to Steam through trusted rows only:
  0x00401b80 CAppManager::CreateServer -> Steam ?  in candidate callees: n/a
  0x00401c30 CAppManager::DestroyServer -> Steam ?  in candidate callees: n/a
  0x004099e0 Global::UpdateScreen -> Steam ?  in candidate callees: n/a
  0x0040c710 CClientExoApp::UnloadModule -> Steam ?  in candidate callees: n/a
  0x0040c7d0 CClientExoApp::GetClientName -> Steam ?  in candidate callees: n/a
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam ?  in candidate callees: n/a
  0x0040cbe0 CClientExoApp::SendLoadGameRequest -> Steam 0x0073f720  in candidate callees: yes
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  ... 39 more GOG callees
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0x10*,0x1c*,0x48*,0x68*,0x74*,0x98*,0xa0*,0x1d0*,0x1d4*,0x1e0*,0x1e1*,0x1b70} | Steam {0x4*,0x8*,0x10*,0x1c*,0x48*,0x68*,0x74*,0x98*,0xa0*,0x1d0*,0x1d4*,0x1e0*,0x1e1*,0x1e40}
imm sets (* = in both, 18 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x114*,0x1f4*,0x200*,...(+4)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x114*,0x1f4*,0x200*,...(+4)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00853a00: 16-byte aligned: YES
  00853a00  55             PUSH EBP
  00853a01  8B EC          MOV EBP,ESP
  00853a03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00853a00
string literals (0):
direct callees (47):
  0x0073F750  ClientApp::GetGuiInGame
  0x0070BC60  (unnamed)
  0x00740EE0  CClientExoApp::BusyWaitMenuMusic
  0x00740F00  CClientExoApp::BusyWaitMuteSound
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x0071DEE0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x0071E910  (unnamed)
  0x0071EBD0  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x00410530  CSWGuiManager::AddPanel
  0x005FF000  CSWPartyTable::ResetCurrentSessionStartTime
  0x007D0AF0  CGuiInGame::SetGlobalUnpauseableState
  0x004109C0  CSWGuiManager::PopModalPanel
  0x007CA060  CGuiInGame::HideSWInGameGui
  0x00740540  CClientExoApp::SetLoadScreenByModuleName
  0x00733570  CExoString::CExoString_char
struct offsets touched (top 10):
  +0x4 [dec 4]  x14  via EAX,ECX,EDX
  +0x48 [dec 72]  x12  via EAX,ECX,EDX
  +0x68 [dec 104]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via ECX,EDX
  +0x1C [dec 28]  x4  via ECX,EDX
  +0x1D4 [dec 468]  x4  via EAX,ECX,EDX
  +0x1D0 [dec 464]  x3  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EDX
  +0xA0 [dec 160]  x2  via ECX,EDX
  +0x1E0 [dec 480]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00853a00 FUN_00853a00 size=2611 ret=4 callers=2 callees=47

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00853a00(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined1 *puVar4;
  undefined1 local_78 [24];
  undefined1 local_60 [16];
  undefined1 *local_50;
  int local_44;
  SchedulerResourceManagement *local_40;
  int local_3c;
  uint local_38[2 = byte +0x8];
  uint local_30[2 = byte +0x8];
  uint *local_28;
  undefined1 local_24 [8];
  int *local_1c;
  int local_18;
  SchedulerResourceManagement *local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972546;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((((*(uint *)(param_1 + 0x68) >> 1 & 1) != 0) ||
      (iVar1 = FUN_0073f750(), (*(uint *)(*(int *)(iVar1 + 0xa0) + 0x68) & 1) != 0)) ||
     (*(int *)(param_1 + 0x1e40) != 0)) {
    FUN_0070bc60();
    if ((*(uint *)(param_1 + 0x68) >> 1 & 1) == 0) {
      FUN_00740f00();
    }
    else {
      FUN_00740ee0();
    }
    local_18 = param_2;
    local_14 = *(SchedulerResourceManagement **)(DAT_00a1b4a4 + 4);
    if (((*(uint *)(param_2 + 0x1d0) >> 5 & 1) == 0) || (DAT_00a7f098 != 0)) {
      FUN_005ff000();
      FUN_0073f750();
      FUN_007d0af0();
      if ((*(uint *)(local_18 + 0x1d0) >> 3 & 1) == 0) {
        if ((*(uint *)(local_18 + 0x1d0) >> 4 & 1) == 0) {
          if ((*(uint *)(param_1 + 0x68) >> 1 & 1) == 0) {
            local_40 = *(SchedulerResourceManagement **)(DAT_00a1b4a4 + 4);
            FUN_00740540();
            FUN_00733570();
            local_8 = 0xb;
            FUN_007405a0();
            local_8 = 0xffffffff;
            FUN_00733780();
            Concurrency::details::SchedulerResourceManagement::Statistics
                      (local_40,(uint *)0x0,(uint *)0x1,(uint *)0x0);
            FUN_00740d30();
            for (local_44 = 0; local_44 < 10; local_44 = local_44 + 1) {
              FUN_00740d90();
            }
            FUN_00740dc0();
            FUN_00740980();
            FUN_00740ce0();
            iVar1 = FUN_00857490(*(undefined4 *)(local_18 + 0x1d4));
            if (iVar1 != 0) {
              *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xffffff7f;
              *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffcff | 0x200;
              while (*(int *)(*(int *)(param_1 + 0x1c) + 0x98) != 0) {
                FUN_004109c0();
              }
              FUN_0073f750();
              FUN_007ca060();
              FUN_0051ce00();
              FUN_00740c00();
              Concurrency::details::SchedulerResourceManagement::Statistics
                        (*(SchedulerResourceManagement **)(DAT_00a1b4a4 + 4),
                         *(uint **)(local_18 + 0x1d4),(uint *)(local_18 + 0x1f4 [dec 500]),
/* ... 142 more lines truncated by --max-lines */
```

### decoy: 0x00854440 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00854440: 16-byte aligned: YES
  00854440  55             PUSH EBP
  00854441  8B EC          MOV EBP,ESP
  00854443  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00854440
string literals (0):
direct callees (9):
  0x00733540  CExoString::CExoString(empty)
  0x00733780  CExoString::~CExoString
  0x0073F750  ClientApp::GetGuiInGame
  0x007337C0  CExoString::operator=
  0x00733570  CExoString::CExoString_char
  0x00919723  operator new
  0x008586D0  (unnamed)
  0x00858A40  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 5):
  +0x4 [dec 4]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x68 [dec 104]  x1  via ECX
  +0xA0 [dec 160]  x1  via EDX
  +0x1D0 [dec 464]  x1  via ECX
```

## Card CAL-1-27

GOG function: CResGFF::GetList  gog 0x006248c0  callconv=(blank)  param_size_bytes=12  gog_size=164  gog_retn=12
Candidate Steam address: 0x00718760 (size 164, retn 12)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00718810:STR0/CALLEE1/SIG0.3/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00624870 CResGFF::GetFieldType -> 0x00718710  trusted
- GOG-order neighbour after:  0x00624970 CResGFF::GetListCount -> 0x00718810  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x006239e0 CResGFF::GetDataLayoutList -> Steam 0x00717880  in candidate callees: yes
  0x00623a40 CResGFF::GetFieldByLabel -> Steam ?  in candidate callees: n/a
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

## Card CAL-1-28

GOG function: CGuiInGame::IsAnimationPlayingInDialog  gog 0x004dbde0  callconv=(blank)  param_size_bytes=0  gog_size=55  gog_retn=0
Candidate Steam address: 0x007cb6e0 (size 55, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007cb6a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbda0 CGuiInGame::IsCameraDialog  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004dbe20 CGuiInGame::IsLoopingDialogStuntAnimation_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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

### decoy: 0x007cb6a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb6a0: 16-byte aligned: YES
  007cb6a0  55             PUSH EBP
  007cb6a1  8B EC          MOV EBP,ESP
  007cb6a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb6a0
string literals (0):
direct callees (1):
  0x008BB0E0  (unnamed)
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX
```

## Card CAL-1-29

GOG function: Gob::DisableInViewVolume  gog 0x0084be90  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x004598b0 (size 18, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004598d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084be70 Gob::IsAnimating  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084beb0 Gob::IsInViewVolume -> 0x004598d0  trusted

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

## Card CAL-1-30

GOG function: CSWSCreature::AddAttackActions  gog 0x00681170  callconv=(blank)  param_size_bytes=57  gog_size=1668  gog_retn=60
Candidate Steam address: 0x00568540 (size 1668, retn 60)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (GetAIStateReputation 0x5e1680); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00568480:STR0/CALLEE0/SIG0.301/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006810b0 CSWSCreature::AddCutsceneMoveActions  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00681800 CSWSCreature::AddToArea  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (27), mapped to Steam through trusted rows only:
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0064bbc0 CServerExoApp::GetDoorByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam 0x0051e170  in candidate callees: yes
  0x0067c8c0 CSWSCreature::ActionManager -> Steam ?  in candidate callees: n/a
  ... 19 more GOG callees
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0x30*,0xec*,0xfc*,0x520*,0x52c*,0x550*,0x564*,0xff5*,0x10dc*,0x10e8*,0x11ac*} | Steam {0x4*,0x8*,0x30*,0xec*,0xfc*,0x520*,0x52c*,0x550*,0x564*,0xff5*,0x10dc*,0x10e8*,0x11ac*}
imm sets (* = in both, 17 common): GOG {0x2*,0x3*,0x4*,0xa*,0xc*,0x10*,0x22*,0x34*,0x3c*,0x3f*,0xa4*,0xbb*,0x5dc*,0xffff*,...(+3)} | Steam {0x2*,0x3*,0x4*,0xa*,0xc*,0x10*,0x22*,0x34*,0x3c*,0x3f*,0xa4*,0xbb*,0x5dc*,0xffff*,...(+3)}

### abi
```
GOG param bytes: 57; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(57/4)*4 = 60 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=60 ; asm RET = 60
Stack Slot Match: TRUE (GOG 57 bytes -> x86 slots 60; ASM RET 60)
```

### prologue
```
prologue for 0x00568540: 16-byte aligned: YES
  00568540  55             PUSH EBP
  00568541  8B EC          MOV EBP,ESP
  00568543  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00568540
string literals (0):
direct callees (27):
  0x00541080  CSWSObject::ClearAllActions
  0x00590430  CSWSCombatRound::RemoveAllActions
  0x005908F0  CSWSCombatRound::ClearAllAttacks
  0x00590A50  CSWSCombatRound::AddAttackAction
  0x00563C90  CSWSCreature::ActionManager
  0x0053F560  (unnamed)
  0x00540CA0  CSWSObject::AddActionToFront
  0x005718A0  CSWSCreature::ComputeAIState
  0x005D1750  CSWSCreature::GetVisibleListElement
  0x0051C0A0  CServerExoApp::GetGameObject
  0x005E1680  CSWSObject::GetAIStateReputation
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x0053F7F0  CSWSObject::AddAction
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FE3B0  CSWPartyTable::GetIsLeader
  0x0057AC30  CSWSCreature::BroadcastCombatStateToParty
  0x0057A960  CSWSCreature::GetDetectMode
  0x0091EB8B  (unnamed)
  0x00579D00  CSWSCreature::BroadcastBattleCry
struct offsets touched (top 10):
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX
  +0xFF5 [dec 4085]  x4  via EAX,EDX
  +0x10DC [dec 4316]  x3  via EAX,ECX,EDX
  +0xFC [dec 252]  x2  via EAX,ECX
  +0x520 [dec 1312]  x2  via EAX,ECX
  +0x10E8 [dec 4328]  x2  via EAX,EDX
  +0x30 [dec 48]  x1  via EDX
  +0xEC [dec 236]  x1  via EAX
  +0x52C [dec 1324]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00568540 FUN_00568540 size=1668 ret=60 callers=9 callees=27

undefined4 __thiscall
FUN_00568540(int param_1,int param_2,undefined2 param_3,int param_4,undefined4 param_5,int param_6,
            int param_7,int param_8)

{
  char cVar1;
  int iVar2;
  int *piVar3;
  uint uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined2 *local_b4;
  undefined2 *local_b0;
  undefined4 local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00950a86;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (param_8 == 0) {
    if ((param_4 != 0) && (param_6 != 0)) {
      FUN_00541080(1);
      FUN_00590430();
      FUN_005908f0();
    }
    FUN_00590a50(param_2,param_3,param_4,0x5dc,1);
    if (*(int *)(param_1 + 0x520) == 0) {
      FUN_00563c90(4);
    }
    iVar2 = FUN_0053f560(0x3f);
    if (iVar2 == 0) {
      local_24 = 1;
      FUN_00540ca0(0x3f,0xffff,1,&local_24,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
    }
    ExceptionList = local_10;
    return 1;
  }
  local_1c = 0;
  local_14 = 0;
  FUN_005718a0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (((*(int *)(param_1 + 0xec) == 0) && (*(char *)(param_1 + 0xff5) != '\x01')) &&
     (*(char *)(param_1 + 0xff5) != '\x10')) {
    ExceptionList = local_10;
    return 0;
  }
  local_18 = FUN_005d1750(param_2);
  if ((local_18 != 0) && ((*(byte *)(local_18 + 4) >> 2 & 3) == 1)) {
    ExceptionList = local_10;
    return 0;
  }
  if (((param_2 != 0x7f000000) && (piVar3 = (int *)FUN_0051c0a0(param_2), piVar3 != (int *)0x0)) &&
     ((iVar2 = (**(code **)(*piVar3 + 0x30))(), iVar2 != 0 &&
      (((cVar1 = FUN_005e1680(param_2), cVar1 == '\x01' && (*(char *)(param_1 + 0xff5) != '\x01'))
       && (*(char *)(param_1 + 0xff5) != '\x10')))))) {
    FUN_00584da0(0xbb,0);
    ExceptionList = local_10;
    return 0;
  }
  if (param_6 != 0) {
    FUN_00541080(1);
  }
  if (param_2 == 0x7f000000) goto LAB_005689e5;
  if (param_7 == 0) {
    FUN_0053f7f0(0xc,0xffff,1,&stack0x00000030,3,&param_2,1,&stack0x00000020,1,&stack0x00000024,1,
                 &stack0x00000028,1,&stack0x0000002c,1,&param_3,1,&stack0x00000034,1,
                 &stack0x00000038,1,&stack0x0000003c,0,0,0,0,0,0);
  }
  else {
    FUN_00540ca0(0xc,0xffff,1,&stack0x00000030,3,&param_2,1,&stack0x00000020,1,&stack0x00000024,1,
                 &stack0x00000028,1,&stack0x0000002c,1,&param_3,1,&stack0x00000034,1,
/* ... 78 more lines truncated by --max-lines */
```

### decoy: 0x00568480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00568480: 16-byte aligned: YES
  00568480  55             PUSH EBP
  00568481  8B EC          MOV EBP,ESP
  00568483  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00568480
string literals (0):
direct callees (4):
  0x005919B0  CSWSCombatRound::AddCutsceneMoveActions
  0x00563C90  CSWSCreature::ActionManager
  0x0053F560  (unnamed)
  0x0053F7F0  CSWSObject::AddAction
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x520 [dec 1312]  x1  via EDX
  +0x10DC [dec 4316]  x1  via EAX
```

## Card CAL-1-31

GOG function: CSWCItem::GetName  gog 0x0047f250  callconv=(blank)  param_size_bytes=8  gog_size=618  gog_retn=8
Candidate Steam address: 0x007fdfa0 (size 618, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007fe210:STR0/CALLEE0/SIG0.05/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0047f1d0 CSWCItem::SetArea  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0047f4c0 CSWCItem::SetItemPropertyUsesLeft  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605f00 CExoString::operator+ -> Steam ?  in candidate callees: n/a
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0xc*,0x34*,0x114*} | Steam {0xc*,0x34*,0x114*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x8*,0x78*,0x18c*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x8*,0x78*,0x18c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007fdfa0: 16-byte aligned: YES
  007fdfa0  55             PUSH EBP
  007fdfa1  8B EC          MOV EBP,ESP
  007fdfa3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007fdfa0
string literals (0):
direct callees (11):
  0x00733540  CExoString::CExoString(empty)
  0x007107F0  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x00733780  CExoString::~CExoString
  0x00919F10  _strlen
  0x006D5200  (unnamed)
  0x0071DF90  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0x114 [dec 276]  x2  via EAX
  +0xC [dec 12]  x1  via ECX
  +0x34 [dec 52]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007fdfa0 FUN_007fdfa0 size=618 ret=8 callers=2 callees=11

void __thiscall FUN_007fdfa0(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  size_t local_60;
  undefined1 local_54 [8];
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  undefined4 *local_3c;
  undefined1 local_38 [28];
  uint local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096c289;
  local_10 = ExceptionList;
  local_1c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(local_1c);
  local_8 = 1;
  FUN_007107f0();
  local_8 = 2;
  FUN_00733540();
  local_8._0_1_ = 3;
  if (param_3 == 0) {
    FUN_007335f0(param_1 + 0x18c);
    local_8._0_1_ = 2;
    FUN_00733780();
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
  }
  else {
    if (*(int *)(param_1 + 0x18c) == 0) {
      local_60 = 0;
    }
    else {
      local_60 = _strlen [GHIDRA-AUTO-LABEL: unreliable](*(char **)(param_1 + 0x18c));
    }
    if (((int)local_60 < 1) || (*(int *)(param_1 + 0x114) == 0)) {
      local_3c = (undefined4 *)FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
      FUN_0071df90(*local_3c,local_38,DAT_0099c4a2);
      FUN_007337c0(local_38);
      if (*(int *)(param_1 + 0x114) == 0) {
        FUN_0071df90(0,local_38,DAT_0099c4a2);
        FUN_00733570(&DAT_00986bfc);
        local_8._0_1_ = 4;
        FUN_00733df0(local_4c,local_44);
        local_8._0_1_ = 5;
        uVar1 = FUN_00733df0(local_54,local_38);
        local_8._0_1_ = 6;
        FUN_007337c0(uVar1);
        local_8._0_1_ = 5;
        FUN_00733780();
        local_8._0_1_ = 4;
        FUN_00733780();
        local_8._0_1_ = 3;
        FUN_00733780();
      }
      FUN_007335f0(local_18);
      local_8._0_1_ = 2;
      FUN_00733780();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
    }
    else {
      FUN_007335f0(param_1 + 0x18c);
      local_8._0_1_ = 2;
      FUN_00733780();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
    }
  }
  ExceptionList = local_10;
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_1c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x007fe210 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007fe210: 16-byte aligned: YES
  007fe210  55             PUSH EBP
  007fe211  8B EC          MOV EBP,ESP
  007fe213  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007fe210
string literals (0):
direct callees (1):
  0x007FD970  CSWCItem::GetActiveProperty
struct offsets touched (top 1):
  +0x17C [dec 380]  x3  via EAX,ECX,EDX
```

## Card CAL-1-32

GOG function: CSWGuiControl::AddChildControl  gog 0x009175d0  callconv=__thiscall  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00419330 (size 28, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00419350:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00917540 CSWGuiControl::HandleRMouseUp  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009175f0 CSWGuiControl::GetIsChild -> 0x00419350  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00534170 CExoArrayList::Add_2 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x1c*} | Steam {0x4*,0x1c*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00419330: 16-byte aligned: YES
  00419330  55             PUSH EBP
  00419331  8B EC          MOV EBP,ESP
  00419333  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00419330
string literals (0):
direct callees (1):
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00419330 FUN_00419330 size=28 ret=4 callers=3 callees=1
THUNK -> 0x0083EA60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00419330(undefined4 param_1)

{
  FUN_0083ea60(param_1);
  return;
}
```

### decoy: 0x00419350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00419350: 16-byte aligned: YES
  00419350  55             PUSH EBP
  00419351  8B EC          MOV EBP,ESP
  00419353  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00419350
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via ECX
```

## Card CAL-1-33

GOG function: CSWGuiMainCharGen::BeginPlayGame  gog 0x005eeb30  callconv=(blank)  param_size_bytes=0  gog_size=30  gog_retn=0
Candidate Steam address: 0x00907340 (size 30, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00907310:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005eeb00 CSWGuiMainCharGen::ClearFeats  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005eeb50 CSWGuiMainCharGen::Draw  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005df060 CSWGuiClassSelection::BeginPlayGame -> Steam ?  in candidate callees: n/a
  0x005ed900 CSWGuiMainCharGen::RemoveSubPanels -> Steam ?  in candidate callees: n/a
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
prologue for 0x00907340: 16-byte aligned: YES
  00907340  55             PUSH EBP
  00907341  8B EC          MOV EBP,ESP
  00907343  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00907340
string literals (0):
direct callees (2):
  0x00906110  (unnamed)
  0x008FACA0  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00907340 FUN_00907340 size=30 ret=0 callers=1 callees=2

void __fastcall FUN_00907340(undefined4 param_1)

{
  FUN_00906110(param_1);
  FUN_008faca0();
  return;
}
```

### decoy: 0x00907310 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00907310: 16-byte aligned: YES
  00907310  55             PUSH EBP
  00907311  8B EC          MOV EBP,ESP
  00907313  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00907310
string literals (0):
direct callees (1):
  0x00844560  (unnamed)
struct offsets touched (top 2):
  +0x68 [dec 104]  x1  via EAX
  +0x310 [dec 784]  x1  via ECX
```

## Card CAL-1-34

GOG function: CGuiInGame::GetCanOrientInDialog  gog 0x004df1f0  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=4
Candidate Steam address: 0x007cf710 (size 112, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cf5d0:STR0/CALLEE0/SIG0.225/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004df190 CGuiInGame::GetBarkBubbleVisible  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004df250 CGuiInGame::RePopulateMainInterface  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0x9c} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x440} | Steam {0x4*,0xc,0xf24,0x7f000000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cf710: 16-byte aligned: YES
  007cf710  55             PUSH EBP
  007cf711  8B EC          MOV EBP,ESP
  007cf713  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cf710
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007cf710 FUN_007cf710 size=112 ret=4 callers=1 callees=3

undefined4 __thiscall FUN_007cf710(undefined4 param_1,int param_2)

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

### decoy: 0x007cf5d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cf5d0: 16-byte aligned: YES
  007cf5d0  55             PUSH EBP
  007cf5d1  8B EC          MOV EBP,ESP
  007cf5d3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cf5d0
string literals (0):
direct callees (7):
  0x0073F4D0  ClientApp::GetClientObject
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
  0x00919723  operator new
  0x0054AA10  CExoLinkedList::AddHead
  0x0064DDE0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
```

## Card CAL-1-35

GOG function: CResGFF::WriteFieldWORD  gog 0x00625c10  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x00719ab0 (size 97, retn 12)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00719a40:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625ba0 CResGFF::WriteFieldCHAR -> 0x00719a40  trusted
- GOG-order neighbour after:  0x00625c80 CResGFF::WriteFieldSHORT -> 0x00719b20  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam ?  in candidate callees: n/a
  0x00624190 CResGFF::AddField -> Steam ?  in candidate callees: n/a
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

## Card CAL-1-36

GOG function: CGuiInGame::AddFloatyText  gog 0x004df580  callconv=(blank)  param_size_bytes=16  gog_size=58  gog_retn=16
Candidate Steam address: 0x007cee80 (size 58, retn 16)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ceec0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004df550 CGuiInGame::GetGuiInGameScreenPanel  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004df5c0 CGuiInGame::InitializeMap  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005226b0 unnamed -> Steam ?  in candidate callees: n/a
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

## Card CAL-1-37

GOG function: CSWGuiInGameItemEntry::Constructor  gog 0x005a4960  callconv=(blank)  param_size_bytes=0  gog_size=250  gog_retn=0
Candidate Steam address: 0x008b0a60 (size 250, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008b09f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a48f0 CSWGuiInGameEquip::ItemExists  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005a4a80 CSWGuiInGameItemEntry::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00512a40 CSWGuiButton::Constructor -> Steam ?  in candidate callees: n/a
  0x00912e20 CSWGuiBorder::Constructor -> Steam 0x00414a50  in candidate callees: yes
  0x00915570 CSWGuiText::Constructor -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1d0*,0x3ac*,0x3b0*} | Steam {0x1d0*,0x3ac*,0x3b0*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x1d4*,0x24c*,0x2c4*,0x33c*,0x7f000000*,0xfffffffb*,0xfffffffd*,0xfffffffe*} | Steam {0x2*,0x3*,0x1d4*,0x24c*,0x2c4*,0x33c*,0x7f000000*,0xfffffffb*,0xfffffffd*,0xfffffffe*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008b0a60: 16-byte aligned: YES
  008b0a60  55             PUSH EBP
  008b0a61  8B EC          MOV EBP,ESP
  008b0a63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008b0a60
string literals (0):
direct callees (3):
  0x0041A510  (unnamed)
  0x00414A50  CSWGuiBorder::Constructor
  0x004171A0  (unnamed)
struct offsets touched (top 3):
  +0x3AC [dec 940]  x6  via EAX,EDX
  +0x1D0 [dec 464]  x1  via EDX
  +0x3B0 [dec 944]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008b0a60 FUN_008b0a60 size=250 ret=0 callers=4 callees=3

undefined4 * __fastcall FUN_008b0a60(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00978ba2;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0041a510(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  *param_1 = CSWGuiRadioButton::vftable;
  local_8 = 0;
  *param_1 = CSWGuiInGameItemEntry::vftable;
  FUN_00414a50();
  local_8._0_1_ = 1;
  FUN_00414a50();
  local_8._0_1_ = 2;
  FUN_00414a50();
  local_8 = CONCAT31(local_8._1_3_,3);
  FUN_004171a0();
  param_1[0x74 = byte +0x1d0] = 0x7f000000;
  param_1[0xeb = byte +0x3ac] = param_1[0xeb = byte +0x3ac] & 0xfffffffe;
  param_1[0xeb = byte +0x3ac] = param_1[0xeb = byte +0x3ac] & 0xfffffffd;
  param_1[0xeb = byte +0x3ac] = param_1[0xeb = byte +0x3ac] & 0xfffffffb;
  *(undefined1 *)(param_1 + 0xec) = 0;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x008b09f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b09f0: 16-byte aligned: YES
  008b09f0  55             PUSH EBP
  008b09f1  8B EC          MOV EBP,ESP
  008b09f3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b09f0
string literals (0):
direct callees (3):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX
  +0x38 [dec 56]  x1  via EDX
```

## Card CAL-1-38

GOG function: CSWCObject::AnimationAttack  gog 0x004749e0  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef880 (size 129, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef7f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474950 CSWCObject::AnimationParry  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00474a70 CSWCObject::AnimationRunning  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xcc*,0xe0*,0x130*} | Steam {0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef880: 16-byte aligned: YES
  007ef880  55             PUSH EBP
  007ef881  8B EC          MOV EBP,ESP
  007ef883  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef880
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef880 FUN_007ef880 size=129 ret=4 callers=1 callees=1

bool __thiscall FUN_007ef880(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe4c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef7f0: 16-byte aligned: YES
  007ef7f0  55             PUSH EBP
  007ef7f1  8B EC          MOV EBP,ESP
  007ef7f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef7f0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card CAL-1-39

GOG function: CAuroraProcessedTexture::Load  gog 0x0090a5d0  callconv=(blank)  param_size_bytes=0  gog_size=438  gog_retn=0
Candidate Steam address: 0x0093ffb0 (size 438, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (Demand 0x7111e0,Release 0x7112b0,GetTPCAttrib 0x945db0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00940170:STR0/CALLEE0/SIG0.303/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090a590 CAuroraTexture::GetTextureDataSize  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090a790 CAuroraProcessedTexture::ReadProcessedTextureHeader  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x005915c0 CResTPC::GetTPCTXISize -> Steam ?  in candidate callees: n/a
  0x0061ab40 CRes::Demand -> Steam 0x007111e0  in candidate callees: yes
  0x0061ac10 CRes::Release -> Steam 0x007112b0  in candidate callees: yes
  0x0090c1c0 CResTPC::GetTPCDataPtr -> Steam ?  in candidate callees: n/a
  0x00944400 CResTPC::GetTPCAttrib -> Steam 0x00945db0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x2*,0x4*,0x8*} | Steam {0x2*,0x4*,0x8*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x4*,0x6*,0x8*,0x10*,0x40*,0xbbf*} | Steam {0x2*,0x3*,0x4*,0x6*,0x8*,0x10*,0x40*,0xbbf*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0093ffb0: 16-byte aligned: YES
  0093ffb0  55             PUSH EBP
  0093ffb1  8B EC          MOV EBP,ESP
  0093ffb3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093ffb0
string literals (0):
direct callees (5):
  0x007111E0  CRes::Demand
  0x00945DB0  CResTPC::GetTPCAttrib
  0x007112B0  CRes::Release
  0x00455820  (unnamed)
  0x00945EA0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x2 [dec 2]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093ffb0 FUN_0093ffb0 size=438 ret=0 callers=1 callees=5

undefined4 __fastcall FUN_0093ffb0(undefined4 param_1)

{
  undefined2 *puVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  char local_2e;
  char local_2d;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  undefined1 local_10 [8];
  undefined1 local_8 [4];
  
  local_18 = 0;
  local_24 = 1;
  iVar2 = FUN_007111e0();
  if (iVar2 == 0) {
    uVar3 = 0;
  }
  else {
    FUN_00945db0(&local_20,&local_28,&local_2e,&local_14,&local_1c,local_10,local_8,&local_2d);
    if (((local_2e == '\x01') || (local_2e == '\x03')) || (local_2e == '\x04')) {
      local_2c = FUN_00455820();
      if (local_2c == 0) {
        FUN_007112b0();
        uVar3 = 0;
      }
      else {
        if (local_14 == 0) {
          local_18 = local_1c;
        }
        else {
          if (local_2e == '\x03') {
            local_40 = 8;
          }
          else if (local_2e == '\x04') {
            local_40 = 0x10;
          }
          local_34 = local_20;
          local_38 = local_28;
          if (local_28 / local_20 == 6) {
            local_24 = 6;
            local_38 = local_28 / 6;
          }
          local_18 = local_14;
          for (local_3c = 0; local_3c < local_2d + -1; local_3c = local_3c + 1) {
            local_34 = local_34 >> 1;
            local_38 = local_38 >> 1;
            local_18 = ((int)(local_34 + 3 + (local_34 + 3 >> 0x1f & 3U)) >> 2) *
                       ((int)(local_38 + 3 + (local_38 + 3 >> 0x1f & 3U)) >> 2) * local_40 +
                       local_18;
          }
          if (local_24 == 6) {
            local_18 = local_18 * 6;
          }
        }
        iVar4 = local_2c + local_18;
        iVar2 = FUN_00945ea0();
        puVar1 = (undefined2 *)(iVar4 + 4 + iVar2);
        *puVar1 = 0xbbf;
        *(undefined4 *)(puVar1 + 0x1 [dec 1] {undefined2*: byte +0x2}) = param_1;
        uVar3 = 1;
      }
    }
    else {
      FUN_007112b0();
      uVar3 = 0;
    }
  }
/* ... 2 more lines truncated by --max-lines */
```

### decoy: 0x00940170 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00940170: 16-byte aligned: YES
  00940170  55             PUSH EBP
  00940171  8B EC          MOV EBP,ESP
  00940173  81 EC 94 00 00 00 SUB ESP,0x94
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00940170
string literals (0):
direct callees (2):
  0x00711200  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card CAL-1-40

GOG function: CSWVarTable::SetLocalNumber  gog 0x0074cd80  callconv=(blank)  param_size_bytes=5  gog_size=39  gog_retn=8
Candidate Steam address: 0x005e9340 (size 39, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005e9370:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0074cd50 CSWVarTable::GetLocalNumber  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0074cdb0 CSWVarTable::LoadVarTable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}
imm sets (* = in both, 2 common): GOG {0x8*,0x20*} | Steam {0x8*,0x20*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005e9340: 16-byte aligned: YES
  005e9340  55             PUSH EBP
  005e9341  8B EC          MOV EBP,ESP
  005e9343  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e9340
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005e9340 FUN_005e9340 size=39 ret=8 callers=2 callees=0

void __thiscall FUN_005e9340(int param_1,int param_2,undefined1 param_3)

{
  if ((-1 < param_2) && (param_2 < 0x20)) {
    *(undefined1 *)(param_1 + param_2 + 0x14) = param_3;
  }
  return;
}
```

### decoy: 0x005e9370 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e9370: 16-byte aligned: YES
  005e9370  55             PUSH EBP
  005e9371  8B EC          MOV EBP,ESP
  005e9373  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e9370
string literals (5):
  0x00996970  at 005e9383  'SWVarTable'
  0x00996964  at 005e93a0  'BitArray'
  0x00996958  at 005e9430  'Variable'
  0x0099694C  at 005e944c  'ByteArray'
  0x00996958  at 005e94dc  'Variable'
direct callees (7):
  0x00718960  CResGFF::GetStructFromStruct
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00718C00  CResGFF::ReadFieldDWORD
  0x00718A00  CResGFF::ReadFieldBYTE
  0x009196EE  __security_check_cookie
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via ECX
```

## Card CAL-1-41

GOG function: CSWGuiSaveLoad::LaunchMostRecentSave  gog 0x005d86f0  callconv=(blank)  param_size_bytes=0  gog_size=294  gog_retn=0
Candidate Steam address: 0x00859470 (size 133, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008593c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d86d0 CSWGuiSaveLoad::SetEndGameMode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d8840 CExoArrayList_CSWGuiControl__::Insert  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00512940 unnamed -> Steam ?  in candidate callees: n/a
  0x005d1680 CSWGuiSaveLoadEntry::GetGameDirectory -> Steam ?  in candidate callees: n/a
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00606e70 CExoString::CompareNoCase -> Steam 0x00734f60  in candidate callees: no
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x0091df10 CSWGuiListBox::GetControl -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x4,0x8*,0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c}
imm sets (* = in both, 0 common): GOG {0x2c,0x970} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00859470: 16-byte aligned: YES
  00859470  55             PUSH EBP
  00859471  8B EC          MOV EBP,ESP
  00859473  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00859470
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x28 [dec 40]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00859470 FUN_00859470 size=133 ret=0 callers=1 callees=0

undefined4 * __fastcall FUN_00859470(undefined4 *param_1)

{
  *param_1 = 0;
  param_1[1 = byte +0x4] = 0;
  param_1[2 = byte +0x8] = 0;
  param_1[3 = byte +0xc] = 0;
  param_1[4 = byte +0x10] = 0;
  param_1[5 = byte +0x14] = 0;
  param_1[6 = byte +0x18] = 0;
  param_1[7 = byte +0x1c] = 0;
  param_1[8 = byte +0x20] = 0;
  param_1[9 = byte +0x24] = 0;
  param_1[10 = byte +0x28] = 0;
  param_1[0xb = byte +0x2c] = 0;
  return param_1;
}
```

### decoy: 0x008593c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008593c0: 16-byte aligned: YES
  008593c0  55             PUSH EBP
  008593c1  8B EC          MOV EBP,ESP
  008593c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008593c0
string literals (0):
direct callees (3):
  0x007339D0  CExoString::operator==
  0x00733780  CExoString::~CExoString
  0x00616A10  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via ECX
```

## Card CAL-1-42

GOG function: CGuiInGame::RemoveGUIPopUps  gog 0x004e0ca0  callconv=(blank)  param_size_bytes=0  gog_size=120  gog_retn=0
Candidate Steam address: 0x007d05a0 (size 120, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::GetMiniMapVisible
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007d0560:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0c60 CGuiInGame::GetMiniMapVisible -> 0x007d0560  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0d20 CGuiInGame::ResetInterfaceFont  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam ?  in candidate callees: n/a
  0x0040d580 CClientExoApp::GetMiniGame -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x98*,0x59b4*} | Steam {0x4*,0x1c*,0x98*,0x59b4*}
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
prologue for 0x007d05a0: 16-byte aligned: YES
  007d05a0  55             PUSH EBP
  007d05a1  8B EC          MOV EBP,ESP
  007d05a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d05a0
string literals (0):
direct callees (2):
  0x0072FB00  OptionsHolder::GetGameOptions (returns *(*(this+4)+4); +8 bit0 = Auto Level Up NPCs)
  0x007400A0  CClientExoApp::GetMiniGame
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x59B4 [dec 22964]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007d05a0 FUN_007d05a0 size=120 ret=0 callers=1 callees=2

uint FUN_007d05a0(void)

{
  int iVar1;
  uint uVar2;
  int extraout_var;
  int iVar3;
  undefined4 uVar4;
  
  uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  iVar1 = FUN_0072fb00();
  if ((*(uint *)(iVar1 + 0x1c) >> 3 & 1) == 0) {
    uVar2 = 0;
  }
  else {
    iVar3 = extraout_var;
    iVar1 = FUN_007400a0(extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 4),iVar1,uVar4);
    if (iVar1 == 0) {
      if (*(int *)(iVar3 + 0x98) == 0) {
        uVar2 = 1;
      }
      else {
        uVar2 = *(uint *)(*(int *)(iVar3 + 0x98) + 0x59b4) >> 1 & 1;
      }
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x007d0560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d0560: 16-byte aligned: YES
  007d0560  55             PUSH EBP
  007d0561  8B EC          MOV EBP,ESP
  007d0563  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d0560
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x3C [dec 60]  x3  via EAX,EDX
  +0x40 [dec 64]  x1  via ECX
  +0x84 [dec 132]  x1  via EDX
```

## Card CAL-1-43

GOG function: CSWCObject::GetPortrait  gog 0x004725e0  callconv=(blank)  param_size_bytes=5  gog_size=39  gog_retn=8
Candidate Steam address: 0x007ed480 (size 39, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed450:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004725b0 CSWCObject::GetCameraFollowPart  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00472610 CSWCObject::GetPortraitId  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00495600 CSWPortrait::GetPortrait -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xa8*} | Steam {0x8*,0xa8*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007ed480: 16-byte aligned: YES
  007ed480  55             PUSH EBP
  007ed481  8B EC          MOV EBP,ESP
  007ed483  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed480
string literals (0):
direct callees (1):
  0x0085E8C0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007ed480 FUN_007ed480 size=39 ret=8 callers=0 callees=1

undefined4 FUN_007ed480(undefined4 param_1,undefined1 param_2)

{
  FUN_0085e8c0(param_1,param_2);
  return param_1;
}
```

### decoy: 0x007ed450 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed450: 16-byte aligned: YES
  007ed450  55             PUSH EBP
  007ed451  8B EC          MOV EBP,ESP
  007ed453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed450
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
```

## Card CAL-1-44

GOG function: CSWSObject::GetDialogResref  gog 0x006637e0  callconv=(blank)  param_size_bytes=4  gog_size=29  gog_retn=4
Candidate Steam address: 0x004597d0 (size 25, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004597b0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006634d0 CSWSItem::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00663800 CSWSObject::GetMaxHitPoints  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x1d8}
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
prologue for 0x004597d0: 16-byte aligned: YES
  004597d0  55             PUSH EBP
  004597d1  8B EC          MOV EBP,ESP
  004597d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004597d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1D8 [dec 472]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004597d0 FUN_004597d0 size=25 ret=4 callers=0 callees=0

void __thiscall FUN_004597d0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x1d8) = param_2;
  return;
}
```

### decoy: 0x004597b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004597b0: 16-byte aligned: YES
  004597b0  55             PUSH EBP
  004597b1  8B EC          MOV EBP,ESP
  004597b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004597b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1D8 [dec 472]  x1  via EAX
```

## Card CAL-1-45

GOG function: CServerExoApp::LoadGame  gog 0x0064b840  callconv=__thiscall  param_size_bytes=16  gog_size=40  gog_retn=16
Candidate Steam address: 0x0051be20 (size 40, retn 16)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (LoadGame 0x52fb80)
- best decoy (neighbour/other candidate) scores: 0x0051bdf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: autolabel:DoHillClimbing
- GOG-order neighbour before: 0x0064b810 CServerExoApp::SaveGame -> 0x0051bdf0  trusted
- GOG-order neighbour after:  0x0064b870 CServerExoApp::SetMoveToModulePending  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2f00 CServerExoAppInternal::LoadGame -> Steam 0x0052fb80  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0051be20: 16-byte aligned: YES
  0051be20  55             PUSH EBP
  0051be21  8B EC          MOV EBP,ESP
  0051be23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051be20
string literals (0):
direct callees (1):
  0x0052FB80  CServerExoAppInternal::LoadGame (SAVES:%06d - %s; partytable, globals 0x656710, LoadModule)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051be20 Concurrency::details::SchedulerProxy::DoHillClimbing size=40 ret=16 callers=1 callees=1
THUNK -> 0x0052FB80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: unsigned int __thiscall Concurrency::details::SchedulerProxy::DoHillClimbing(unsigned
   int,unsigned int,unsigned int,unsigned int)
   
   Libraries: Visual Studio 2010 Debug, Visual Studio 2012 Debug, Visual Studio 2015 Debug, Visual
   Studio 2015 Release */

uint __thiscall
Concurrency::details::SchedulerProxy::DoHillClimbing
          (SchedulerProxy *this,uint param_1,uint param_2,uint param_3,uint param_4)

{
  uint uVar1;
  
  uVar1 = FUN_0052fb80(param_1,param_2,param_3,param_4);
  return uVar1;
}
```

### decoy: 0x0051bdf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bdf0: 16-byte aligned: YES
  0051bdf0  55             PUSH EBP
  0051bdf1  8B EC          MOV EBP,ESP
  0051bdf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bdf0
string literals (0):
direct callees (1):
  0x0052F610  StartSave(slot,name) (screenshot + save-state flags)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card CAL-1-46

GOG function: CClientExoApp::DisableVideoEffect  gog 0x0040de80  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007409c0 (size 22, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007409a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040de60 CClientExoApp::EnableVideoEffect  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040dea0 CClientExoApp::GetVideoEffectType  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044af60 CClientExoAppInternal::DisableVideoEffect -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x007409c0: 16-byte aligned: YES
  007409c0  55             PUSH EBP
  007409c1  8B EC          MOV EBP,ESP
  007409c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007409c0
string literals (0):
direct callees (1):
  0x0079E960  CClientExoAppInternal::DisableVideoEffect
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007409c0 FUN_007409c0 size=22 ret=0 callers=7 callees=1
THUNK -> 0x0079E960 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007409c0(undefined4 param_1)

{
  FUN_0079e960(param_1);
  return;
}
```

### decoy: 0x007409a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007409a0: 16-byte aligned: YES
  007409a0  55             PUSH EBP
  007409a1  8B EC          MOV EBP,ESP
  007409a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007409a0
string literals (0):
direct callees (1):
  0x0079E550  CClientExoAppInternal::EnableVideoEffect
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card CAL-1-47

GOG function: CGuiInGame::HideGalaxyMapGui  gog 0x004db020  callconv=(blank)  param_size_bytes=0  gog_size=224  gog_retn=None
Candidate Steam address: 0x007ca920 (size 239, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007ca730:STR0/CALLEE0/SIG0.312/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dae30 CGuiInGame::ShowGalaxyMapGui -> 0x007ca730  trusted
- GOG-order neighbour after:  0x004db100 CGuiInGame::ShowDebugMenu  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d7f0 CClientExoApp::GetPausedByCombat -> Steam ?  in candidate callees: n/a
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam ?  in candidate callees: n/a
  0x004da340 CGuiInGame::SetSWGuiStatus -> Steam ?  in candidate callees: n/a
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
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

## Card CAL-1-48

GOG function: CSWSObject::HasSpellEffectApplied  gog 0x006ae3a0  callconv=(blank)  param_size_bytes=4  gog_size=101  gog_retn=4
Candidate Steam address: 0x00413e90 (size 102, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.18 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00413f00:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006ae2e0 CSWSObject::GetScriptLocation -> 0x00547600  trusted
- GOG-order neighbour after:  0x006ae410 CSWSObject::RemoveEffectByType  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*,0x148,0x14c} | Steam {0x4,0x1c*,0x3c,0x4c}
imm sets (* = in both, 1 common): GOG {0x4*,0x10} | Steam {0x4*,0x8,0xa,0xc}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00413e90: 16-byte aligned: YES
  00413e90  55             PUSH EBP
  00413e91  8B EC          MOV EBP,ESP
  00413e93  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00413e90
string literals (0):
direct callees (3):
  0x0072FB00  OptionsHolder::GetGameOptions (returns *(*(this+4)+4); +8 bit0 = Auto Level Up NPCs)
  0x0075FF30  (unnamed)
  0x0075FF50  (unnamed)
struct offsets touched (top 4):
  +0x1C [dec 28]  x3  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x4C [dec 76]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00413e90 FUN_00413e90 size=102 ret=4 callers=2 callees=3

void __thiscall FUN_00413e90(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  if (*(int *)(param_1 + 0x4c) == 0) {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    iVar1 = FUN_0072fb00();
    if ((*(uint *)(iVar1 + 0x1c) >> 10 & 1) != 0) {
      *(uint *)(param_1 + 0x1c) = *(uint *)(param_1 + 0x1c) | 8;
      FUN_0075ff30(param_2);
      FUN_0075ff50(param_1,iVar1,uVar2);
    }
  }
  return;
}
```

### decoy: 0x00413f00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00413f00: 16-byte aligned: YES
  00413f00  55             PUSH EBP
  00413f01  8B EC          MOV EBP,ESP
  00413f03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00413f00
string literals (0):
direct callees (1):
  0x00413F90  (unnamed)
struct offsets touched (top 3):
  +0x40 [dec 64]  x3  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x48 [dec 72]  x2  via EAX,ECX
```

## Card CAL-1-49

GOG function: CSWSObject::GetPortrait  gog 0x00663840  callconv=__thiscall  param_size_bytes=4  gog_size=47  gog_retn=4
Candidate Steam address: 0x00459830 (size 25, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Gob::SetIgnoreHitCheck
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459810:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00663830 CSWSObject::GetLockOrientationToObject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00663870 CSWSObject::SetPortrait  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc} | Steam {0x1a9}
imm sets (* = in both, 1 common): GOG {0x4*,0x20} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
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

## Card CAL-1-50

GOG function: CClientExoApp::DisableInput  gog 0x0040d790  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007402d0 (size 22, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (LoadGame 0x853a00)
- best decoy (neighbour/other candidate) scores: 0x007402b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d770 CClientExoApp::StopSoundPlayingInDialog  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d7b0 CClientExoApp::EnableInput  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0045ff80 CClientExoAppInternal::DisableInput -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x007402d0: 16-byte aligned: YES
  007402d0  55             PUSH EBP
  007402d1  8B EC          MOV EBP,ESP
  007402d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007402d0
string literals (0):
direct callees (1):
  0x007B3370  CClientExoAppInternal::DisableInput
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007402d0 FUN_007402d0 size=22 ret=0 callers=3 callees=1
THUNK -> 0x007B3370 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007402d0(undefined4 param_1)

{
  FUN_007b3370(param_1);
  return;
}
```

### decoy: 0x007402b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007402b0: 16-byte aligned: YES
  007402b0  55             PUSH EBP
  007402b1  8B EC          MOV EBP,ESP
  007402b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007402b0
string literals (0):
direct callees (1):
  0x0079BE40  CClientExoAppInternal::StopSoundPlayingInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card CAL-1-51

GOG function: CClientExoApp::GetObjectName  gog 0x0040c5a0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073f0e0 (size 32, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f0b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c570 CClientExoApp::JournalUpdated  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040c5c0 CClientExoApp::AddFeedbackMessage  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043b150 CClientExoAppInternal::GetObjectName -> Steam ?  in candidate callees: n/a
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
prologue for 0x0073f0e0: 16-byte aligned: YES
  0073f0e0  55             PUSH EBP
  0073f0e1  8B EC          MOV EBP,ESP
  0073f0e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f0e0
string literals (0):
direct callees (1):
  0x0078EBA0  CClientExoAppInternal::FadeAndStopStreamingSoundSource
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f0e0 FUN_0073f0e0 size=32 ret=8 callers=1 callees=1
THUNK -> 0x0078EBA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073f0e0(undefined4 param_1,undefined4 param_2)

{
  FUN_0078eba0(param_1,param_2);
  return;
}
```

### decoy: 0x0073f0b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f0b0: 16-byte aligned: YES
  0073f0b0  55             PUSH EBP
  0073f0b1  8B EC          MOV EBP,ESP
  0073f0b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f0b0
string literals (0):
direct callees (1):
  0x00785330  CClientExoAppInternal::JournalUpdated
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card CAL-1-52

GOG function: CGuiInGame::HideLoadModuleDebugMenu  gog 0x004dbba0  callconv=(blank)  param_size_bytes=0  gog_size=149  gog_retn=0
Candidate Steam address: 0x007cb4a0 (size 149, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cb540:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbaf0 CGuiInGame::HideSubItemCreateMenu_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004dbc40 CGuiInGame::HidePowersFeatsSkillsDebugMenu  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d580 CClientExoApp::GetMiniGame -> Steam ?  in candidate callees: n/a
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
  0x0090ee40 CSWGuiManager::PopModalPanel -> Steam ?  in candidate callees: n/a
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

## Card CAL-1-53

GOG function: CServerAIMaster::SetAILevel  gog 0x0064db40  callconv=(blank)  param_size_bytes=8  gog_size=71  gog_retn=8
Candidate Steam address: 0x0051e040 (size 116, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CServerAIMaster::AddObject
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.58 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.58
- best decoy (neighbour/other candidate) scores: 0x0051e0c0:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064dae0 CServerAIMaster::RemoveObject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064db90 CServerAIMaster::AddEventDeltaTime -> 0x0051e170  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0064da60 CServerAIMaster::AddObject -> Steam 0x0051e040  in candidate callees: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x78*} | Steam {0x4,0x78*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0x10}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0051e040: 16-byte aligned: YES
  0051e040  55             PUSH EBP
  0051e041  8B EC          MOV EBP,ESP
  0051e043  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051e040
string literals (0):
direct callees (2):
  0x0051E0C0  CServerAIMaster::RemoveObject
  0x0051D150  (unnamed)
struct offsets touched (top 2):
  +0x78 [dec 120]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051e040 FUN_0051e040 size=116 ret=8 callers=9 callees=2

int FUN_0051e040(int param_1,int param_2)

{
  int iVar1;
  
  if (((param_2 < 5) && (-1 < param_2)) && (param_1 != 0)) {
    if (*(int *)(param_1 + 0x78) != -1) {
      FUN_0051e0c0(param_1);
    }
    iVar1 = FUN_0051d150(*(undefined4 *)(param_1 + 4));
    if (iVar1 == 1) {
      *(int *)(param_1 + 0x78) = param_2;
    }
  }
  else {
    iVar1 = 0;
  }
  return iVar1;
}
```

### decoy: 0x0051e0c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051e0c0: 16-byte aligned: YES
  0051e0c0  55             PUSH EBP
  0051e0c1  8B EC          MOV EBP,ESP
  0051e0c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051e0c0
string literals (0):
direct callees (1):
  0x0051D170  (unnamed)
struct offsets touched (top 2):
  +0x78 [dec 120]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
```

## Card CAL-1-54

GOG function: CClientExoApp::SetGameOver  gog 0x0040d880  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x007403c0 (size 28, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007403a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d860 CClientExoApp::RunDeathSequence  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d8a0 CClientExoApp::GetGameOver  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448830 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x007403c0: 16-byte aligned: YES
  007403c0  55             PUSH EBP
  007403c1  8B EC          MOV EBP,ESP
  007403c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007403c0
string literals (0):
direct callees (1):
  0x0079C230  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007403c0 FUN_007403c0 size=28 ret=4 callers=3 callees=1
THUNK -> 0x0079C230 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007403c0(undefined4 param_1)

{
  FUN_0079c230(param_1);
  return;
}
```

### decoy: 0x007403a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007403a0: 16-byte aligned: YES
  007403a0  55             PUSH EBP
  007403a1  8B EC          MOV EBP,ESP
  007403a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007403a0
string literals (0):
direct callees (1):
  0x0079C0E0  CClientExoAppInternal::RunDeathSequence
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card CAL-1-55

GOG function: CSWSObject::SetKeepCorpse  gog 0x006b1480  callconv=(blank)  param_size_bytes=4  gog_size=153  gog_retn=4
Candidate Steam address: 0x0054a700 (size 153, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::SetDeleteNoFade
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0054a7a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b13e0 CSWSObject::SetDeleteNoFade  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006b1520 CSWSObject::GetClientObject  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> Steam ?  in candidate callees: n/a
  0x0064c310 CServerExoApp::GetPlayerCreatureId -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x2c*,0x158,0x234} | Steam {0x4*,0x8*,0xc*,0x2c*,0x154,0x230}
imm sets (* = in both, 2 common): GOG {0x4*,0x18*} | Steam {0x4*,0x18*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0054a700: 16-byte aligned: YES
  0054a700  55             PUSH EBP
  0054a701  8B EC          MOV EBP,ESP
  0054a703  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054a700
string literals (0):
direct callees (5):
  0x0051C8F0  (unnamed)
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 6):
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x2C [dec 44]  x1  via ECX
  +0x154 [dec 340]  x1  via EAX
  +0x230 [dec 560]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0054a700 FUN_0054a700 size=153 ret=4 callers=1 callees=5

void __thiscall FUN_0054a700(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_8;
  
  uVar1 = FUN_0051c8f0();
  iVar2 = FUN_0051c600(uVar1);
  if (iVar2 != 0) {
    local_8 = FUN_007a1720();
    while (local_8 != 0) {
      iVar2 = FUN_00561430(local_8);
      if (*(int *)(iVar2 + 0x2c) == *(int *)(param_1 + 4)) {
        *(undefined4 *)(iVar2 + 0x154) = param_2;
        return;
      }
      FUN_0058c370(&local_8);
    }
  }
  *(undefined4 *)(param_1 + 0x230) = param_2;
  return;
}
```

### decoy: 0x0054a7a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054a7a0: 16-byte aligned: YES
  0054a7a0  55             PUSH EBP
  0054a7a1  8B EC          MOV EBP,ESP
  0054a7a3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054a7a0
string literals (0):
direct callees (5):
  0x0051C8F0  (unnamed)
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 6):
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x2C [dec 44]  x1  via ECX
  +0x158 [dec 344]  x1  via EAX
  +0x234 [dec 564]  x1  via ECX
```

## Card CAL-1-56

GOG function: CGuiInGame::GetCanHeadFollowInDialog  gog 0x004dfff0  callconv=(blank)  param_size_bytes=4  gog_size=112  gog_retn=4
Candidate Steam address: 0x007cf8f0 (size 112, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007cf780:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dfe80 CGuiInGame::SetLockHeadFollowInDialog  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004e0060 CGuiInGame::TutorialReasonWillShowWindow  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
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
  0x0073F4D0  ClientApp::GetClientObject
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

## Card CAL-1-57

GOG function: Gob::IsAlwaysRendering  gog 0x0084c4c0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00459e20 (size 17, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Gob::IsInCutscene
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459e00:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c420 Gob::GetCutsceneDummyPosition -> 0x00459e40  trusted
- GOG-order neighbour after:  0x0084c4e0 Global::PartSetFog  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x12} | Steam {0x14}
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

## Card CAL-1-58

GOG function: CRes::Constructor  gog 0x0061aa20  callconv=(blank)  param_size_bytes=0  gog_size=151  gog_retn=0
Candidate Steam address: 0x007111a0 (size 30, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00711170:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a890 CResRef::operator+=  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061aac0 CSWMGTargettingGunBank::IsTargetting  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x6,0x8,0xc,0x10,0x14,0x18,0x1c,0x20,0x24} | Steam {}
imm sets (* = in both, 0 common): GOG {0xfffffff7,0xfffffffb} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007111a0: 16-byte aligned: YES
  007111a0  55             PUSH EBP
  007111a1  8B EC          MOV EBP,ESP
  007111a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007111a0
string literals (0):
direct callees (1):
  0x00711230  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007111a0 FUN_007111a0 size=30 ret=0 callers=21 callees=1

void __fastcall FUN_007111a0(undefined4 *param_1)

{
  *param_1 = CRes::vftable;
  FUN_00711230(1);
  return;
}
```

### decoy: 0x00711170 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711170: 16-byte aligned: YES
  00711170  55             PUSH EBP
  00711171  8B EC          MOV EBP,ESP
  00711173  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711170
string literals (0):
direct callees (2):
  0x007111A0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card CAL-1-59

GOG function: CSWCObject::SetLoopingAnimation  gog 0x00472ab0  callconv=(blank)  param_size_bytes=2  gog_size=42  gog_retn=4
Candidate Steam address: 0x007ed950 (size 42, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (AnimationLooping 0x7ef500)
- best decoy (neighbour/other candidate) scores: 0x007ed910:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472a70 CSWCObject::GetAnimationLength  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00472ae0 CSWCObject::GetLoopingAnimation -> 0x007ed980  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00474660 CSWCObject::AnimationLooping -> Steam 0x007ef500  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x58*} | Steam {0x58*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ed950: 16-byte aligned: YES
  007ed950  55             PUSH EBP
  007ed951  8B EC          MOV EBP,ESP
  007ed953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed950
string literals (0):
direct callees (1):
  0x007EF500  CSWCObject::AnimationLooping
struct offsets touched (top 1):
  +0x58 [dec 88]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed950 FUN_007ed950 size=42 ret=4 callers=6 callees=1

void __thiscall FUN_007ed950(int param_1,undefined2 param_2)

{
  int iVar1;
  
  iVar1 = FUN_007ef500(param_2);
  if (iVar1 == 1) {
    *(undefined2 *)(param_1 + 0x58) = param_2;
  }
  return;
}
```

### decoy: 0x007ed910 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed910: 16-byte aligned: YES
  007ed910  55             PUSH EBP
  007ed911  8B EC          MOV EBP,ESP
  007ed913  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed910
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x38 [dec 56]  x1  via EDX
```

## Card CAL-1-60

GOG function: CClientExoApp::ChangeCharacter  gog 0x0040d840  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x00740380 (size 32, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740350:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d810 CClientExoApp::SetPausedByCombat  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d860 CClientExoApp::RunDeathSequence  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00444550 CClientExoAppInternal::ChangeCharacter -> Steam ?  in candidate callees: n/a
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
prologue for 0x00740380: 16-byte aligned: YES
  00740380  55             PUSH EBP
  00740381  8B EC          MOV EBP,ESP
  00740383  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740380
string literals (0):
direct callees (1):
  0x00797F30  CClientExoAppInternal::ChangeCharacter
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00740380 FUN_00740380 size=32 ret=8 callers=6 callees=1
THUNK -> 0x00797F30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740380(undefined4 param_1,undefined4 param_2)

{
  FUN_00797f30(param_1,param_2);
  return;
}
```

### decoy: 0x00740350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740350: 16-byte aligned: YES
  00740350  55             PUSH EBP
  00740351  8B EC          MOV EBP,ESP
  00740353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740350
string literals (0):
direct callees (1):
  0x0079BF40  CClientExoAppInternal::SetPausedByCombat
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```
