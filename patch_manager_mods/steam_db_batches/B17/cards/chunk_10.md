# Review cards B17 chunk 10

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-10-01

GOG function: CTlkTable::ParseStr  gog 0x00628b10  callconv=__thiscall  param_size_bytes=4  gog_size=2040  gog_retn=4
Candidate Steam address: 0x0071f4c0 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0071f4f0:STR0/CALLEE0/SIG0.242/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00628ac0 CTlkTable::ClearCustomTokens -> 0x0071eb80  gate_class=REVIEW
- GOG-order neighbour after:  0x00629310 CTlkTable::GetLanguageVersion  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605790 CExoString::Constructor_4 -> Steam 0x00733680  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: no
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG callers mapped through trusted rows (13 of 47): the card's A-CALLEE credit may come from these, not from callees
  0x00427c70 CSWCCreature::CreateUsableItemEntry -> Steam 0x0077bcf0  in candidate callers: no
  0x0044a810 CClientExoAppInternal::GetGUIString -> Steam 0x0079e210  in candidate callers: no
  0x004dc660 CGuiInGame::HandleDialogEntry -> Steam 0x007cbf60  in candidate callers: no
  0x004dccc0 CGuiInGame::HandleDialogReplies -> Steam 0x007cc5c0  in candidate callers: no
GOG strings (2): 'CUSTOM'; '<UNRECOGNIZED TOKEN>'
disp sets (* = in both, 0 common): GOG {0x1,0x4,0x40,0x44,0x48,0x4c,0x50} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0x24,0x30,0x39,...(+5)} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0071f4c0: 16-byte aligned: YES
  0071f4c0  55             PUSH EBP
  0071f4c1  8B EC          MOV EBP,ESP
  0071f4c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0071f4c0
string literals (0):
direct callees (2):
  0x0071F4F0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0071f4c0 FUN_0071f4c0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0071f4c0(undefined4 param_1,uint param_2)

{
  FUN_0071f4f0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0071f4f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0071f4f0: 16-byte aligned: YES
  0071f4f0  55             PUSH EBP
  0071f4f1  8B EC          MOV EBP,ESP
  0071f4f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0071f4f0
string literals (0):
direct callees (4):
  0x00944EE0  (unnamed)
  0x009196FD  operator delete
  0x0040C170  (unnamed)
  0x007208C0  (unnamed)
struct offsets touched (top 10):
  +0x34 [dec 52]  x2  via ECX
  +0x38 [dec 56]  x2  via EAX
  +0x3C [dec 60]  x2  via EDX
  +0x40 [dec 64]  x2  via EDX
  +0x44 [dec 68]  x2  via ECX
  +0x48 [dec 72]  x2  via EAX
  +0x4C [dec 76]  x2  via ECX
  +0x50 [dec 80]  x2  via EDX
  +0x54 [dec 84]  x2  via EAX
  +0x58 [dec 88]  x2  via ECX
```

### Static facts (script output, report-only)
- size 44 B, RET 4
- callees: 0x0071f4f0 (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B17-10-02

GOG function: CAurObject::AnimateExternal  gog 0x00844470  callconv=__thiscall  param_size_bytes=4  gog_size=129  gog_retn=4
Candidate Steam address: 0x00451e60 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: SetWeather 0x7a7c60); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00451e40:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00844430 MaxTree::GetType  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00844500 Global::MaxTreeDestructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x00454660 CSWCArea::SetWeather -> Steam 0x007a7c60  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x14*,0x50*,0x94*} | Steam {0x14*,0x50*,0x94*}
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
prologue for 0x00451e60: 16-byte aligned: YES
  00451e60  55             PUSH EBP
  00451e61  8B EC          MOV EBP,ESP
  00451e63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00451e60
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x94 [dec 148]  x5  via ECX,EDX
  +0x50 [dec 80]  x4  via EAX,EDX
  +0x14 [dec 20]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00451e60 FUN_00451e60 size=129 ret=4 callers=5 callees=0

void __thiscall FUN_00451e60(int *param_1,undefined4 param_2)

{
  if (param_1[0x25 = byte +0x94] == 0) {
    (**(code **)(*param_1 + 0x14))(param_2);
  }
  else {
    *(int *)(param_1[0x25 = byte +0x94] + 0x50) = *(int *)(param_1[0x25 = byte +0x94] + 0x50) + 1;
    (**(code **)(*param_1 + 0x14))(param_2);
    *(int *)(param_1[0x25 = byte +0x94] + 0x50) = *(int *)(param_1[0x25 = byte +0x94] + 0x50) + 1;
  }
  return;
}
```

### decoy: 0x00451e40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00451e40: 16-byte aligned: YES
  00451e40  55             PUSH EBP
  00451e41  8B EC          MOV EBP,ESP
  00451e43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00451e40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 129 B, RET 4
- callers 5: 0x00413650, 0x00413a80, 0x007a7c60 CSWCArea::SetWeather, 0x0085e650, 0x008edb50
- indirect: 0x00451ea8 slot 0x14, 0x00451ed9 slot 0x14
- twins: none

## Card B17-10-03

GOG function: C2DA::SkipNewLines  gog 0x00622a20  callconv=(blank)  param_size_bytes=8  gog_size=273  gog_retn=8
Candidate Steam address: 0x0071d360 (size 440, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=C2DA::Init | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0071d2b0:STR0/CALLEE0/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00621e70 C2DA::Load2DArray  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00622b40 C2DA::Unload2DArray  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8}
imm sets (* = in both, 1 common): GOG {0x8*,0x9,0xa,0xd,0x10,0x20} | Steam {0x4,0x8*,0xc,0x3c,0x40,0x7e1}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0071d360: 16-byte aligned: YES
  0071d360  55             PUSH EBP
  0071d361  8B EC          MOV EBP,ESP
  0071d363  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0071d360
string literals (0):
direct callees (12):
  0x00710B40  (unnamed)
  0x00710AD0  (unnamed)
  0x007111C0  CRes::CancelRequest
  0x00713010  CExoResMan::ReleaseResObject
  0x00710C20  (unnamed)
  0x00710EB0  (unnamed)
  0x00712970  (unnamed)
  0x00919723  operator new
  0x00729ED0  CRes2DA::CRes2DA
  0x00712B60  CExoResMan::SetResObject
  0x007112D0  CRes::Request
  0x00710CA0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0071d360 FUN_0071d360 size=440 ret=8 callers=1 callees=12

void __thiscall FUN_0071d360(int param_1,undefined4 param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  int local_50;
  undefined1 local_44 [16];
  CStatusBar *local_34;
  int local_30;
  undefined1 local_2c [16];
  undefined4 *local_1c;
  undefined4 *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094dc6b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_00710b40(param_1 + 0xc);
  if (iVar2 == 0) {
    if ((*(int *)(param_1 + 8) != 0) && (iVar2 = FUN_00710ad0(&DAT_00986925), iVar2 != 0)) {
      if (*(int *)(param_1 + 4) != 0) {
        FUN_007111c0(uVar1);
      }
      iVar2 = FUN_00713010(*(undefined4 *)(param_1 + 8));
      if (iVar2 == 0) {
        local_1c = *(undefined4 **)(param_1 + 8);
        local_18 = local_1c;
        if (local_1c != (undefined4 *)0x0) {
          (**(code **)*local_1c)(1);
        }
        *(undefined4 *)(param_1 + 8) = 0;
      }
    }
    iVar2 = FUN_00710c20(&DAT_00986925);
    if (iVar2 == 0) {
      local_14 = FUN_00712970(param_2,0x7e1);
      if (local_14 == 0) {
        local_34 = (CStatusBar *)FUN_00919723(0x3c);
        local_8 = 0;
        if (local_34 == (CStatusBar *)0x0) {
          local_50 = 0;
        }
        else {
          local_50 = CStatusBar::CStatusBar(local_34);
        }
        local_30 = local_50;
        local_8 = 0xffffffff;
        local_14 = local_50;
        FUN_00712b60(param_2,0x7e1,local_50);
      }
      if (local_14 != 0) {
        *(int *)(param_1 + 8) = local_14;
        if (param_3 != 0) {
          *(undefined4 *)(param_1 + 4) = 1;
          FUN_007112d0();
        }
        FUN_00710ca0(local_44,param_2);
      }
    }
    else {
      *(undefined4 *)(param_1 + 8) = 0;
      FUN_00710eb0(local_2c,&DAT_00986925);
    }
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0071d2b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0071d2b0: 16-byte aligned: YES
  0071d2b0  55             PUSH EBP
  0071d2b1  8B EC          MOV EBP,ESP
  0071d2b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0071d2b0
string literals (0):
direct callees (3):
  0x00710AD0  (unnamed)
  0x007111C0  CRes::CancelRequest
  0x00713010  CExoResMan::ReleaseResObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 440 B, RET 8
- callers 1: 0x0071ad40
- callees: 0x00710b40 (CALL), 0x00710ad0 (CALL), 0x007111c0 CRes::CancelRequest (CALL), 0x00713010 CExoResMan::ReleaseResObject (CALL), 0x00710c20 (CALL), 0x00710eb0 (CALL), ...
- indirect: 0x0071d408 slot 0x0
- globals: 0x00a1b490 READ
- twins: none

## Card B17-10-04

GOG function: CSWGuiSaveLoad::UnpackGame  gog 0x005d6e10  callconv=(blank)  param_size_bytes=8  gog_size=1040  gog_retn=12
Candidate Steam address: 0x008584d0 (size 95, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.02 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008584a0:STR0/CALLEE0/SIG0.043/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005d6a10 CSWGuiSaveLoadEntry::SetXboxTitle  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d7230 CSWGuiSaveLoad::HandleSaveButton  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (20), mapped to Steam through trusted rows only:
  0x0040d430 CClientExoApp::HideLoadScreen -> Steam 0x0073ff50  in candidate callees: no
  0x004e2040 CSWGuiSaveLoad::CopyGameToFutureGame -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: no
  ... 12 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: no
GOG strings (5): 'SAVES:'; 'CORRUPT'; '%06d - %s'; 'FUTUREGAME:' ...
disp sets (* = in both, 0 common): GOG {0x4,0xc} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc*,0xd,0xe,0xf,...(+9)} | Steam {0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x008584d0: 16-byte aligned: YES
  008584d0  55             PUSH EBP
  008584d1  8B EC          MOV EBP,ESP
  008584d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008584d0
string literals (0):
direct callees (2):
  0x008545C0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008584d0 FUN_008584d0 size=95 ret=12 callers=1 callees=2

void __thiscall FUN_008584d0(undefined4 param_1,int param_2)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00972d08;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8 = 0;
  if (param_2 == 1) {
    FUN_008545c0(&stack0x00000008);
  }
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,param_1);
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008584a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008584a0: 16-byte aligned: YES
  008584a0  55             PUSH EBP
  008584a1  8B EC          MOV EBP,ESP
  008584a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008584a0
string literals (0):
direct callees (1):
  0x008583F0  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via EAX
```

### Static facts (script output, report-only)
- size 95 B, RET 12
- callers 1: 0x00858be0
- callees: 0x008545c0 (CALL), 0x00733780 CExoString::Destructor (CALL)
- twins: none

## Card B17-10-05

GOG function: CAurInternalGL::Reshape  gog 0x0086a150  callconv=(blank)  param_size_bytes=8  gog_size=130  gog_retn=8
Candidate Steam address: 0x00478270 (size 130, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00478300:STR0/CALLEE0/SIG0.208/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00869870 CAurInternalGL::InitEditor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086a1e0 CAurInternalGL::ShutDown  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0xa0*,0xa4*,0xb4*} | Steam {0xa0*,0xa4*,0xb4*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00478270: 16-byte aligned: YES
  00478270  55             PUSH EBP
  00478271  8B EC          MOV EBP,ESP
  00478273  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00478270
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0xA0 [dec 160]  x1  via ECX
  +0xA4 [dec 164]  x1  via EAX
  +0xB4 [dec 180]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00478270 FUN_00478270 size=130 ret=8 callers=0 callees=1

void __thiscall FUN_00478270(int param_1,int param_2,int param_3)

{
  undefined4 local_c;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0xb4) != 0) {
    *(int *)(param_1 + 0xa0) = param_2;
    *(int *)(param_1 + 0xa4) = param_3;
    DAT_009f42a4 = param_2;
    DAT_009f42a8 = param_3;
    if (param_3 == 0) {
      local_c = 1;
    }
    else {
      local_c = param_3;
    }
    if (param_2 == 0) {
      local_8 = 1;
    }
    else {
      local_8 = param_2;
    }
    glViewport(0,0,local_8,local_c);
  }
  return;
}
```

### decoy: 0x00478300 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478300: 16-byte aligned: YES
  00478300  55             PUSH EBP
  00478301  8B EC          MOV EBP,ESP
  00478303  81 EC 90 00 00 00 SUB ESP,0x90
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478300
string literals (0):
direct callees (5):
  0x005064F0  (unnamed)
  0x005065D0  (unnamed)
  0x009196FD  operator delete
  0x008339D0  (unnamed)
  0x00435720  (unnamed)
struct offsets touched (top 4):
  +0xB4 [dec 180]  x3  via EAX,ECX
  +0xA8 [dec 168]  x2  via ECX,EDX
  +0x24 [dec 36]  x1  via EDX
  +0xAC [dec 172]  x1  via EAX
```

### Static facts (script output, report-only)
- size 130 B, RET 8
- callees: 0x0000004f (CALL)
- imports: glViewport (OPENGL32.DLL)
- twins: none

## Card B17-10-06

GOG function: Gob::HitCheck  gog 0x0084d7a0  callconv=(blank)  param_size_bytes=32  gog_size=3275  gog_retn=32
Candidate Steam address: 0x0045b1c0 (size 3275, retn 32)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.90) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0045a510:STR0/CALLEE0/SIG0.852/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084caf0 Gob::HitCheckPart  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084e470 Gob::SetWind  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x0080f6c0 Global::SphereIntersect -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 14 common): GOG {0x4*,0x8*,0xc*,0x1c*,0x50*,0x5d*,0x5e*,0x84*,0x98*,0x104*,0x108*,0x164*,0x1a9*,0x1e0*} | Steam {0x4*,0x8*,0xc*,0x1c*,0x50*,0x5d*,0x5e*,0x84*,0x98*,0x104*,0x108*,0x164*,0x1a9*,0x1e0*}
imm sets (* = in both, 12 common): GOG {0x8*,0xc*,0x10*,0x1c*,0x20*,0x40*,0x44*,0xa4*,0xb0*,0x100*,0x168*,0x1b0*} | Steam {0x8*,0xc*,0x10*,0x1c*,0x20*,0x40*,0x44*,0xa4*,0xb0*,0x100*,0x168*,0x1b0*}

### abi
```
GOG param bytes: 32; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(32/4)*4 = 32 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=32 ; asm RET = 32
Stack Slot Match: TRUE (GOG 32 bytes -> x86 slots 32; ASM RET 32)
```

### prologue
```
prologue for 0x0045b1c0: 16-byte aligned: YES
  0045b1c0  55             PUSH EBP
  0045b1c1  8B EC          MOV EBP,ESP
  0045b1c3  81 EC B0 01 00 00 SUB ESP,0x1b0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0045b1c0
string literals (0):
direct callees (3):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x00517830  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 10):
  +0x4 [dec 4]  x33  via EAX,ECX,EDX
  +0x8 [dec 8]  x33  via EAX,ECX,EDX
  +0xC [dec 12]  x12  via EAX,ECX,EDX
  +0x84 [dec 132]  x4  via EAX,ECX,EDX
  +0x98 [dec 152]  x3  via EAX,ECX,EDX
  +0x50 [dec 80]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x5D [dec 93]  x1  via EDX
  +0x5E [dec 94]  x1  via ECX
  +0x104 [dec 260]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0045b1c0 FUN_0045b1c0 size=3275 ret=32 callers=1 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

uint __thiscall
FUN_0045b1c0(uint param_1,float param_2,float param_3,float param_4,float param_5,float param_6,
            float param_7,undefined4 *param_8,undefined4 *param_9)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  float *pfVar13;
  int iVar14;
  undefined4 *puVar15;
  float *pfVar16;
  bool bVar17;
  float *local_168;
  float local_7c;
  float local_78;
  float local_74;
  undefined1 local_70 [12];
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  undefined1 local_3c [12];
  float local_30;
  float local_2c;
  float local_28;
  float local_24;
  float local_20;
  float local_1c;
  float local_18;
  undefined1 local_14 [12];
  int local_8;
  
  if (param_9 == (undefined4 *)0x0) {
    param_9 = &DAT_00a20100;
  }
  if (*(int *)(param_1 + 0x84) == 0) {
    param_1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x84) == 0) {
      bVar17 = false;
    }
    else {
      bVar17 = (*(ushort *)(*(int *)(param_1 + 0x84) + 0x50) & 0x100) != 0;
    }
    if (bVar17) {
      param_1 = 0;
    }
    else if ((*(char *)((int)param_9 + 0x5e) == '\0') || (*(char *)(param_1 + 0x1a9) == '\0')) {
      if (*(char *)((int)param_9 + 0x5d) != '\0') {
        if ((*(ushort *)(*(int *)(param_1 + 0x84) + 0x50) & 8) == 0) {
          if (*(float *)(param_1 + 0x104) <= _DAT_009f6544) {
            return 0;
          }
        }
        else if (*(float *)(param_1 + 0x108) <= _DAT_009f6544) {
          return 0;
        }
      }
      local_8 = 0;
/* ... 151 more lines truncated by --max-lines */
```

### decoy: 0x0045a510 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0045a510: 16-byte aligned: YES
  0045a510  55             PUSH EBP
  0045a511  8B EC          MOV EBP,ESP
  0045a513  81 EC B0 01 00 00 SUB ESP,0x1b0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0045a510
string literals (0):
direct callees (3):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x00517830  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 10):
  +0x4 [dec 4]  x33  via EAX,ECX,EDX
  +0x8 [dec 8]  x33  via EAX,ECX,EDX
  +0xC [dec 12]  x12  via EAX,ECX,EDX
  +0x84 [dec 132]  x4  via EAX,ECX,EDX
  +0x50 [dec 80]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x5D [dec 93]  x1  via EDX
  +0x5E [dec 94]  x1  via ECX
  +0x104 [dec 260]  x1  via ECX
  +0x108 [dec 264]  x1  via EAX
```

### Static facts (script output, report-only)
- size 3275 B, RET 32
- callers 1: 0x00480df0
- callees: 0x004452b0 (CALL), 0x00517830 (CALL), 0x0044c8a0 (CALL)
- indirect: 0x0045b8e7 slot 0x1c
- twins: none

## Card B17-10-07

GOG function: CTlkTable::ParseStr  gog 0x00628b10  callconv=__thiscall  param_size_bytes=4  gog_size=2040  gog_retn=4
Candidate Steam address: 0x0071e470 (size 118, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.05 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0071e4f0:STR0/CALLEE0/SIG0.193/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00628ac0 CTlkTable::ClearCustomTokens -> 0x0071eb80  gate_class=REVIEW
- GOG-order neighbour after:  0x00629310 CTlkTable::GetLanguageVersion  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605790 CExoString::Constructor_4 -> Steam 0x00733680  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: no
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG callers mapped through trusted rows (13 of 47): the card's A-CALLEE credit may come from these, not from callees
  0x00427c70 CSWCCreature::CreateUsableItemEntry -> Steam 0x0077bcf0  in candidate callers: no
  0x0044a810 CClientExoAppInternal::GetGUIString -> Steam 0x0079e210  in candidate callers: no
  0x004dc660 CGuiInGame::HandleDialogEntry -> Steam 0x007cbf60  in candidate callers: no
  0x004dccc0 CGuiInGame::HandleDialogReplies -> Steam 0x007cc5c0  in candidate callers: no
GOG strings (2): 'CUSTOM'; '<UNRECOGNIZED TOKEN>'
disp sets (* = in both, 0 common): GOG {0x1,0x4,0x40,0x44,0x48,0x4c,0x50} | Steam {0xc,0x10,0x20}
imm sets (* = in both, 2 common): GOG {0x2,0x3,0x4*,0x5,0x6,0x7,0x8*,0x9,0xa,0xb,0xc,0x24,0x30,0x39,...(+5)} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0071e470: 16-byte aligned: YES
  0071e470  55             PUSH EBP
  0071e471  8B EC          MOV EBP,ESP
  0071e473  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0071e470
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 2):
  +0xC [dec 12]  x2  via ECX,EDX
  +0x20 [dec 32]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0071e470 FUN_0071e470 size=118 ret=4 callers=1 callees=1

undefined4 * __thiscall FUN_0071e470(undefined4 *param_1,undefined4 *param_2)

{
  uint local_8;
  
  *param_1 = *param_2;
  FUN_007337c0(param_2 + 0x1 [dec 1] {undefined4*: byte +0x4});
  param_1[3 = byte +0xc] = param_2[3 = byte +0xc];
  for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
    param_1[local_8 + 4] = param_2[local_8 + 4];
  }
  param_1[8 = byte +0x20] = param_2[8 = byte +0x20];
  return param_1;
}
```

### decoy: 0x0071e4f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0071e4f0: 16-byte aligned: YES
  0071e4f0  55             PUSH EBP
  0071e4f1  8B EC          MOV EBP,ESP
  0071e4f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0071e4f0
string literals (0):
direct callees (10):
  0x00733A60  CExoString::operator==_2
  0x00739880  (unnamed)
  0x009196FD  operator delete
  0x00919723  operator new
  0x0071D550  (unnamed)
  0x007398E0  (unnamed)
  0x0071D5E0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 118 B, RET 4
- callers 1: 0x0071e010
- callees: 0x007337c0 CExoString::operator= (CALL)
- twins: none

## Card B17-10-08

GOG function: Global::SceneBSPRemoveGob  gog 0x0086d8c0  callconv=(blank)  param_size_bytes=8  gog_size=133  gog_retn=0
Candidate Steam address: 0x00496040 (size 50, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.38 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00495fb0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086d830 Global::SceneBSPAddGob -> 0x00495f20  gate_class=REVIEW
- GOG-order neighbour after:  0x0086d950 Global::NodeHasGob  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x008730c0 List_T__::Remove -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
  0x00945400 Global::BSPSearch -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00858000 Scene::ChildRemove -> Steam 0x00465c20  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x58*,0xac} | Steam {0x58*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*,0x18,0xa4} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00496040: 16-byte aligned: YES
  00496040  55             PUSH EBP
  00496041  8B EC          MOV EBP,ESP
  00496043  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00496040
string literals (0):
direct callees (1):
  0x00476620  (unnamed)
struct offsets touched (top 1):
  +0x58 [dec 88]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00496040 FUN_00496040 size=50 ret=0 callers=0 callees=1

undefined4 FUN_00496040(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  
  if (param_1 == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_00476620(param_2);
  }
  return uVar1;
}
```

### decoy: 0x00495fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00495fb0: 16-byte aligned: YES
  00495fb0  55             PUSH EBP
  00495fb1  8B EC          MOV EBP,ESP
  00495fb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00495fb0
string literals (0):
direct callees (3):
  0x0051A2B0  (unnamed)
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xAC [dec 172]  x2  via EAX
  +0x58 [dec 88]  x1  via EDX
```

### Static facts (script output, report-only)
- size 50 B, RET 0
- callees: 0x00476620 (CALL)
- twins: none

## Card B17-10-09

GOG function: Global::SceneBSPRemoveGob  gog 0x0086d8c0  callconv=(blank)  param_size_bytes=8  gog_size=133  gog_retn=0
Candidate Steam address: 0x00495f20 (size 135, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.42 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00495fb0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086d830 Global::SceneBSPAddGob -> 0x00495f20  gate_class=REVIEW
- GOG-order neighbour after:  0x0086d950 Global::NodeHasGob  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x008730c0 List_T__::Remove -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
  0x00945400 Global::BSPSearch -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00858000 Scene::ChildRemove -> Steam 0x00465c20  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4,0x8,0x58*,0xac*} | Steam {0x58*,0x80,0x84,0xac*}
imm sets (* = in both, 2 common): GOG {0x8,0xc*,0x18,0xa4*} | Steam {0xc*,0xa4*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00495f20: 16-byte aligned: YES
  00495f20  55             PUSH EBP
  00495f21  8B EC          MOV EBP,ESP
  00495f23  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00495f20
string literals (0):
direct callees (3):
  0x0051A030  (unnamed)
  0x00476620  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
struct offsets touched (top 4):
  +0x80 [dec 128]  x2  via ECX,EDX
  +0x84 [dec 132]  x2  via EAX,ECX
  +0xAC [dec 172]  x2  via EAX
  +0x58 [dec 88]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00495f20 FUN_00495f20 size=135 ret=0 callers=2 callees=3

void FUN_00495f20(int param_1,int param_2)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0xac) != 0) {
    FUN_0051a030(*(undefined4 *)(param_1 + 0xac),param_2 + 0xa4,
                 *(undefined4 *)(*(int *)(param_2 + 0x84) + 0x80),
                 *(undefined4 *)(*(int *)(param_2 + 0x84) + 0x80));
    iVar1 = FUN_00476620(param_2);
    if (iVar1 == 0) {
      FUN_00473f40(param_2);
    }
  }
  return;
}
```

### decoy: 0x00495fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00495fb0: 16-byte aligned: YES
  00495fb0  55             PUSH EBP
  00495fb1  8B EC          MOV EBP,ESP
  00495fb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00495fb0
string literals (0):
direct callees (3):
  0x0051A2B0  (unnamed)
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xAC [dec 172]  x2  via EAX
  +0x58 [dec 88]  x1  via EDX
```

### Static facts (script output, report-only)
- size 135 B, RET 0
- callers 2: 0x00465bb0 Scene::ChildAdd, 0x00499610
- callees: 0x0051a030 (CALL), 0x00476620 (CALL), 0x00473f40 (CALL)
- twins: none

## Card B17-10-10

GOG function: Global::GetResource  gog 0x00867dc0  callconv=(blank)  param_size_bytes=4  gog_size=71  gog_retn=0
Candidate Steam address: 0x00475e50 (size 71, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00475df0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00867cc0 Global::AurResGet_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00867e10 Global::AurResFree  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00475e50: 16-byte aligned: YES
  00475e50  55             PUSH EBP
  00475e51  8B EC          MOV EBP,ESP
  00475e53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00475e50
string literals (0):
direct callees (1):
  0x0049B840  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00475e50 FUN_00475e50 size=71 ret=0 callers=1 callees=1

void FUN_00475e50(int param_1)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (DAT_00a30844 <= local_8) {
      return;
    }
    if (param_1 == *(int *)(DAT_00a30840 + local_8 * 4)) break;
    local_8 = local_8 + 1;
  }
  FUN_0049b840(local_8);
  return;
}
```

### decoy: 0x00475df0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475df0: 16-byte aligned: YES
  00475df0  55             PUSH EBP
  00475df1  8B EC          MOV EBP,ESP
  00475df3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00475df0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 71 B, RET 0
- callers 1: 0x005122e0
- callees: 0x0049b840 (CALL)
- twins: none

## Card B17-10-11

GOG function: C2DA::SkipNewLines  gog 0x00622a20  callconv=(blank)  param_size_bytes=8  gog_size=273  gog_retn=8
Candidate Steam address: 0x0071c130 (size 75, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=C2DA::GetNextLineLength | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0071c180:STR0/CALLEE0/SIG0.357/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00621e70 C2DA::Load2DArray  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00622b40 C2DA::Unload2DArray  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x8*,0x9,0xa*,0xd*,0x10,0x20} | Steam {0x8*,0xa*,0xd*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0071c130: 16-byte aligned: YES
  0071c130  55             PUSH EBP
  0071c131  8B EC          MOV EBP,ESP
  0071c133  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0071c130
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0071c130 FUN_0071c130 size=75 ret=8 callers=1 callees=0

uint FUN_0071c130(char *param_1,uint param_2)

{
  uint local_8;
  
  local_8 = 0;
  for (; ((local_8 < param_2 && (*param_1 != '\n')) && (*param_1 != '\r')); param_1 = param_1 + 1) {
    local_8 = local_8 + 1;
  }
  return local_8;
}
```

### decoy: 0x0071c180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0071c180: 16-byte aligned: YES
  0071c180  55             PUSH EBP
  0071c181  8B EC          MOV EBP,ESP
  0071c183  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0071c180
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 75 B, RET 8
- callers 1: 0x0071c3d0
- twins: none

## Card B17-10-12

GOG function: Global::AurResGetNextLine  gog 0x00867ee0  callconv=(blank)  param_size_bytes=0  gog_size=69  gog_retn=0
Candidate Steam address: 0x00475f70 (size 69, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00475ea0:STR0/CALLEE3/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00867e10 Global::AurResFree  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00867fa0 Global::AurResFree_2 -> 0x00476030  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00867f30 unnamed -> Steam ?  in candidate callees: n/a
  0x00867fa0 Global::AurResFree_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 20): the card's A-CALLEE credit may come from these, not from callees
  0x00817210 CAurTextureBasic::Init -> Steam 0x00424b10  in candidate callers: yes
  0x0084e530 Gob::EnableRenderBumpedOut -> Steam 0x0045bf50  in candidate callers: yes
  0x00861e10 Scene::LoadVisibility -> Steam 0x0046fbd0  in candidate callers: yes
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
prologue for 0x00475f70: 16-byte aligned: YES
  00475f70  55             PUSH EBP
  00475f71  8B EC          MOV EBP,ESP
  00475f73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00475f70
string literals (0):
direct callees (2):
  0x00475FC0  (unnamed)
  0x00476030  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00475f70 FUN_00475f70 size=69 ret=0 callers=20 callees=2

undefined4 FUN_00475f70(void)

{
  undefined4 uVar1;
  
  if (DAT_00a30844 == 0) {
    uVar1 = 0;
  }
  else if (**(int **)(DAT_00a30840 + -4 + DAT_00a30844 * 4) == 0) {
    uVar1 = FUN_00476030(0);
  }
  else {
    uVar1 = FUN_00475fc0(0);
  }
  return uVar1;
}
```

### decoy: 0x00475ea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475ea0: 16-byte aligned: YES
  00475ea0  55             PUSH EBP
  00475ea1  8B EC          MOV EBP,ESP
  00475ea3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 69 B, RET 0
- callers 20: 0x00424b10 CAurTextureBasic::Init, 0x00429010, 0x0045bf50 Gob::EnableRenderBumpedOut, 0x0046fbd0 Scene::LoadVisibility, 0x00470000, ...
- callees: 0x00475fc0 (CALL), 0x00476030 (CALL)
- twins: none

## Card B17-10-13

GOG function: Global::AurSetVSyncState  gog 0x0086ac50  callconv=(blank)  param_size_bytes=1  gog_size=43  gog_retn=0
Candidate Steam address: 0x00478d70 (size 43, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00478d50:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086ac30 Global::AurSetAnisotropy  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ac80 Global::AurSetPauseOverlayColor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086b270 Global::AurBumpyShinyAvailable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 1 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00478d70: 16-byte aligned: YES
  00478d70  55             PUSH EBP
  00478d71  8B EC          MOV EBP,ESP
  00478d73  E8 68 B7 00 00 CALL 0x004844e0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00478d70
string literals (0):
direct callees (1):
  0x004844E0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00478d70 FUN_00478d70 size=43 ret=0 callers=1 callees=1

void FUN_00478d70(char param_1)

{
  char cVar1;
  
  cVar1 = FUN_004844e0();
  if (cVar1 != '\0') {
    (*DAT_00a32aa8)(param_1 != '\0');
    DAT_009f48f8 = param_1;
  }
  return;
}
```

### decoy: 0x00478d50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478d50: 16-byte aligned: YES
  00478d50  55             PUSH EBP
  00478d51  8B EC          MOV EBP,ESP
  00478d53  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478d50
string literals (0):
direct callees (1):
  0x00478FB0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 43 B, RET 0
- callers 1: 0x007bc520
- callees: 0x004844e0 (CALL)
- indirect: 0x00478d8a unresolved
- twins: none

## Card B17-10-14

GOG function: CSWGuiCreateItem::InitializeStoreType  gog 0x005c5330  callconv=(blank)  param_size_bytes=0  gog_size=108  gog_retn=4
Candidate Steam address: 0x008d50e0 (size 3391, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.21) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.05 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008d5e20:STR0/CALLEE0/SIG0.118/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005c4e90 CSWGuiCreateItem::DeleteItemEntries  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005c53a0 CSWGuiCreateItem::OnControlEntered  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005c69a0 CSWGuiCreateItem::ShowSellGUI -> Steam ?  in candidate callees: n/a
  0x005c6c20 CSWGuiCreateItem::ShowBuyGUI -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c,0x3634*,0x3ed8} | Steam {0x4,0x8,0x3c,0x50,0x58,0x5c,0x12c,0x130,0x13c,0x1d0,0x2c8,0x2cc,0x3b0,0x3b4,...(+3)}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x2,0x3,0x4*,0x5,0x6,0x7,0x8*,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+29)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008d50e0: 16-byte aligned: YES
  008d50e0  55             PUSH EBP
  008d50e1  8B EC          MOV EBP,ESP
  008d50e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008d50e0
string literals (3):
  0x009A8BC4  at 008d5351  'base_skill'
  0x009A8BC4  at 008d593a  'base_skill'
  0x009A8BD8  at 008d59b2  'skill'
direct callees (36):
  0x00919723  operator new
  0x00710810  (unnamed)
  0x0071AD40  CRes2DA helper ctor (name, flag)
  0x0071C3D0  C2DA::LoadText
  0x00733540  CExoString::Constructor
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x008D6B40  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x006B7BB0  CSWSCreatureStats::GetSkillRank
  0x006F9150  (unnamed)
  0x00734270  CExoString::Format
  0x00733DF0  CExoString::operator+
struct offsets touched (top 10):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x3634 [dec 13876]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x5C [dec 92]  x2  via EAX,ECX
  +0x12C [dec 300]  x2  via EAX,EDX
  +0x3B0 [dec 944]  x2  via ECX,EDX
  +0x3C [dec 60]  x1  via EDX
  +0x50 [dec 80]  x1  via EDX
  +0x58 [dec 88]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008d50e0 FUN_008d50e0 size=3391 ret=4 callers=0 callees=36

void __thiscall FUN_008d50e0(int param_1,int param_2)

{
  char cVar1;
  undefined1 uVar2;
  int iVar3;
  undefined1 auStack_27c [4];
  undefined4 uStack_278;
  undefined1 *puStack_274;
  undefined1 *puStack_270;
  uint *puStack_26c;
  uint uStack_268;
  undefined4 local_264;
  int local_260;
  undefined4 local_25c;
  undefined4 local_258;
  undefined4 local_254;
  undefined4 local_250;
  undefined4 local_24c;
  undefined4 local_248;
  undefined4 local_244;
  undefined4 local_240;
  undefined4 local_23c;
  undefined4 local_238;
  undefined4 local_234;
  undefined4 local_230;
  undefined4 local_22c;
  undefined4 local_228;
  undefined4 local_224;
  undefined4 local_220;
  undefined4 local_21c;
  undefined4 local_218;
  undefined4 local_214;
  undefined4 local_210;
  undefined4 local_20c;
  undefined4 local_208;
  undefined4 local_204;
  undefined4 uStack_200;
  undefined4 local_1fc;
  undefined4 local_1f8;
  undefined4 local_1f4;
  undefined4 local_1f0;
  undefined4 local_1ec;
  undefined4 local_1e8;
  undefined4 local_1e4;
  undefined4 local_1e0;
  undefined4 *local_1dc;
  int local_1d8;
  int local_1d4;
  int *local_1d0;
  int local_1cc;
  int local_1c8;
  undefined4 local_1c4;
  undefined4 local_1c0;
  undefined4 local_1bc;
  int local_1b8;
  int local_1b4;
  undefined4 local_1b0;
  undefined4 local_1ac;
  undefined4 local_1a8;
  undefined4 local_1a4;
  undefined4 local_1a0;
  int local_19c;
  undefined4 local_198;
  int local_194;
  int *local_190;
  undefined4 *local_18c;
  undefined4 *local_188;
  undefined1 local_184 [8];
  undefined1 local_17c [8];
  undefined1 local_174 [8];
  undefined1 local_16c [8];
  undefined1 local_164 [8];
  undefined1 local_15c [8];
  undefined1 local_154 [8];
  undefined1 local_14c [8];
  undefined1 local_144 [8];
  undefined1 local_13c [8];
  undefined1 local_134 [8];
/* ... 579 more lines truncated by --max-lines */
```

### decoy: 0x008d5e20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008d5e20: 16-byte aligned: YES
  008d5e20  55             PUSH EBP
  008d5e21  8B EC          MOV EBP,ESP
  008d5e23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008d5e20
string literals (0):
direct callees (14):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008D6B40  (unnamed)
  0x005815D0  CSWSCreature::SetComponent
  0x00919723  operator new
  0x005FFD60  CSWSItem::Constructor
  0x00606600  CSWSItem::CopyItem
  0x005BDB60  CSWSCreature::AcquireItem
  0x0041E870  (unnamed)
  0x00733700  CExoString::Constructor_5
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
struct offsets touched (top 7):
  +0x3EC4 [dec 16068]  x4  via EAX,ECX,EDX
  +0x389C [dec 14492]  x2  via EAX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x1C [dec 28]  x1  via EDX
  +0x38C2 [dec 14530]  x1  via ECX
  +0x3EDC [dec 16092]  x1  via EDX
```

### Static facts (script output, report-only)
- size 3391 B, RET 4
- callees: 0x00919723 operator new (CALL), 0x00710810 (CALL), 0x0071ad40 (CALL), 0x0071c3d0 (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), 0x0051c8f0 CServerExoApp::GetPlayerCreatureId (CALL), ...
- indirect: 0x008d5de4 slot 0x0
- globals: 0x00a1b4b0 READ, 0x00a1b4b4 READ, 0x00a1b4d0 READ, ...
- twins: none

## Card B17-10-15

GOG function: CAurInternal::RestoreDesktopDisplayMode  gog 0x008689c0  callconv=(blank)  param_size_bytes=0  gog_size=56  gog_retn=0
Candidate Steam address: 0x00476ae0 (size 56, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00476b20:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00868790 CAurInternal::GetValidMode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00868a00 CAurInternal::RestoreAuroraDisplayMode  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x24*,0xa8*} | Steam {0x24*,0xa8*}
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
prologue for 0x00476ae0: 16-byte aligned: YES
  00476ae0  55             PUSH EBP
  00476ae1  8B EC          MOV EBP,ESP
  00476ae3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00476ae0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x24 [dec 36]  x1  via EDX
  +0xA8 [dec 168]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00476ae0 FUN_00476ae0 size=56 ret=0 callers=0 callees=1

void __fastcall FUN_00476ae0(int *param_1)

{
  (**(code **)(*param_1 + 0x24))(0x3f800000,1);
  if (param_1[0x2a = byte +0xa8] != 0) {
    ChangeDisplaySettingsA((DEVMODEA *)0x0,0);
  }
  return;
}
```

### decoy: 0x00476b20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00476b20: 16-byte aligned: YES
  00476b20  55             PUSH EBP
  00476b21  8B EC          MOV EBP,ESP
  00476b23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00476b20
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x24 [dec 36]  x1  via EDX
  +0xA8 [dec 168]  x1  via ECX
  +0xB0 [dec 176]  x1  via EAX
```

### Static facts (script output, report-only)
- size 56 B, RET 0
- callees: 0x00000124 (CALL)
- indirect: 0x00476afa slot 0x24
- imports: ChangeDisplaySettingsA (USER32.DLL)
- twins: none

## Card B17-10-16

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

## Card B17-10-17

GOG function: CSWGuiCreateItem::InitializeStoreType  gog 0x005c5330  callconv=(blank)  param_size_bytes=0  gog_size=108  gog_retn=4
Candidate Steam address: 0x008d4ac0 (size 1184, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.14 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008d4f60:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005c4e90 CSWGuiCreateItem::DeleteItemEntries  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005c53a0 CSWGuiCreateItem::OnControlEntered  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005c69a0 CSWGuiCreateItem::ShowSellGUI -> Steam ?  in candidate callees: n/a
  0x005c6c20 CSWGuiCreateItem::ShowBuyGUI -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*,0x3634,0x3ed8} | Steam {0x4,0x8,0x1c*,0x4c,0x50,0x70,0x74,0xa0,0x1d0,0x3b4,0x1198,0x3ec4}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x2,0x3,0x4*,0x8*,0xc,0x32,0xa0,0x1c644,0x1c649,0x1e4d1}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008d4ac0: 16-byte aligned: YES
  008d4ac0  55             PUSH EBP
  008d4ac1  8B EC          MOV EBP,ESP
  008d4ac3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008d4ac0
string literals (0):
direct callees (21):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0073F750  CClientExoApp::GetInGameGui
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x0075BBE0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410530  CSWGuiManager::AddPanel
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x008D6B40  (unnamed)
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x00733540  CExoString::Constructor
  0x00733700  CExoString::Constructor_5
  0x0071E910  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00619C90  (unnamed)
  0x007337C0  CExoString::operator=
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0xA0 [dec 160]  x3  via ECX,EDX
  +0x70 [dec 112]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x74 [dec 116]  x1  via EDX
  +0x1D0 [dec 464]  x1  via ECX
  +0x3B4 [dec 948]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 008d4ac0 FUN_008d4ac0 size=1184 ret=4 callers=1 callees=21

void __thiscall FUN_008d4ac0(int param_1,int param_2)

{
  uint uVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 local_bc;
  uint local_b8;
  uint uStack_b4;
  undefined4 local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  int local_9c;
  undefined4 local_98;
  int local_94;
  undefined4 local_90;
  undefined4 local_8c;
  int local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  int local_68;
  undefined4 local_64;
  int local_60;
  undefined4 local_5c;
  int local_58;
  int local_54;
  undefined1 *local_50;
  undefined1 local_4c [24];
  int *local_34;
  undefined1 local_30 [8];
  int *local_28;
  int local_24;
  uint local_20;
  int *local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097c1a0;
  local_10 = ExceptionList;
  uStack_b4 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_54 = *(int *)(param_2 + 0x50);
  if (local_54 != 0) {
    local_58 = *(int *)(param_2 + 0x4c);
    local_9c = param_1;
    if (local_58 != 0) {
      local_5c = *(undefined4 *)(param_1 + 0x1c);
      local_b8 = 0;
      local_bc = 0x8d4b26;
      FUN_004122a0();
    }
    local_18 = param_2;
    local_60 = *(int *)(param_2 + 0x3b4);
    if (local_60 == 0) {
      local_64 = *(undefined4 *)(DAT_00a1b4a4 + 4);
      local_b8 = 0x8d4b55;
      local_68 = FUN_0073f750();
      local_1c = *(int **)(local_68 + 0xa0);
      local_b8 = 0;
      local_bc = 0x8d4b6e;
      FUN_0075bb00();
      local_b8 = 0;
      local_bc = 0;
      FUN_0075ca30(0);
      local_bc = DAT_00a7fdb0;
      local_b8 = DAT_00a7fdb4;
      FUN_0075bbe0(DAT_00a7fe3c,DAT_00a7fe40,DAT_00a7fe44,DAT_00a7fdac);
/* ... 130 more lines truncated by --max-lines */
```

### decoy: 0x008d4f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008d4f60: 16-byte aligned: YES
  008d4f60  55             PUSH EBP
  008d4f61  8B EC          MOV EBP,ESP
  008d4f63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008d4f60
string literals (0):
direct callees (3):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008D65D0  (unnamed)
  0x008D6850  (unnamed)
struct offsets touched (top 3):
  +0x3ED8 [dec 16088]  x3  via EAX,ECX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0x3634 [dec 13876]  x1  via EDX
```

### Static facts (script output, report-only)
- size 1184 B, RET 4
- callers 1: 0x008d2150
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x0075bb00 (CALL), 0x0075ca30 (CALL), 0x0075bbe0 (CALL), 0x0073fea0 CClientExoApp::GetSWGuiManager (CALL), ...
- indirect: 0x008d4bd1 slot 0x70, 0x008d4dbd slot 0x74, 0x008d4ec2 slot 0x70
- globals: 0x00a1b4b0 READ, 0x00a1b4b4 READ
- twins: none

## Card B17-10-18

GOG function: CAurInternal::ResetGammaRamp  gog 0x0086a750  callconv=(blank)  param_size_bytes=8  gog_size=356  gog_retn=8
Candidate Steam address: 0x00478870 (size 356, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.40) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004787f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086a6d0 CAurInternalGL::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086a8c0 Global::NewCAurora  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x00927220 Global::pow -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xb0*} | Steam {0xb0*}
imm sets (* = in both, 7 common): GOG {0x3*,0x8*,0x9*,0x10*,0x100*,0x628*,0xc00*} | Steam {0x3*,0x8*,0x9*,0x10*,0x100*,0x628*,0xc00*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00478870: 16-byte aligned: YES
  00478870  55             PUSH EBP
  00478871  8B EC          MOV EBP,ESP
  00478873  81 EC 28 06 00 00 SUB ESP,0x628
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00478870
string literals (0):
direct callees (2):
  0x0091E390  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 1):
  +0xB0 [dec 176]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00478870 FUN_00478870 size=356 ret=8 callers=0 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00478870(int param_1,float param_2,int param_3)

{
  HDC hDC;
  float10 fVar1;
  byte local_62c;
  int local_61c;
  int local_618;
  ushort auStack_60c [770];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if (param_3 == 0) {
    *(float *)(param_1 + 0xb0) = param_2;
  }
  for (local_618 = 0; local_618 < 3; local_618 = local_618 + 1) {
    for (local_61c = 0; local_61c < 0x100; local_61c = local_61c + 1) {
      fVar1 = (float10)FUN_0091e390((double)local_61c / _DAT_009884a0,(double)param_2);
      local_62c = (byte)(int)ROUND(fVar1 * (float10)_DAT_009884a0);
      auStack_60c[local_618 * 0x100 + local_61c] = (ushort)local_62c << 8 | (ushort)local_61c;
    }
  }
  hDC = GetDC(DAT_00a1b484);
  if (hDC != (HDC)0x0) {
    _DAT_009f5ef4 = param_2;
    ReleaseDC(DAT_00a1b484,hDC);
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x004787f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004787f0: 16-byte aligned: YES
  004787f0  55             PUSH EBP
  004787f1  8B EC          MOV EBP,ESP
  004787f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004787f0
string literals (0):
direct callees (6):
  0x00474AD0  (unnamed)
  0x00474970  (unnamed)
  0x00474460  (unnamed)
  0x00511D40  Global::AurLiteClearTimeTriggers
  0x00478300  (unnamed)
  0x00476870  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 356 B, RET 8
- callees: 0x0091e390 (CALL), 0x00000122 (CALL), 0x00000121 (CALL), 0x009196ee __security_check_cookie (CALL)
- globals: 0x00a1b484 READ, 0x00a1b484 READ
- imports: GetDC (USER32.DLL), ReleaseDC (USER32.DLL)
- twins: none

## Card B17-10-19

GOG function: Global::CheckExtension  gog 0x0086b8b0  callconv=(blank)  param_size_bytes=8  gog_size=130  gog_retn=0
Candidate Steam address: 0x00484b20 (size 130, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: InitExtensions 0x4317d0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00756380:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086b870 Global::AurTextureS3TCAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086b940 Global::AurGetStencilClearValue  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00921b40 Global::strlen -> Steam ?  in candidate callees: n/a
  0x00924b53 unnamed -> Steam ?  in candidate callees: n/a
  0x0092a700 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00823ef0 GLRender::InitExtensions -> Steam 0x004317d0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1*} | Steam {0x1*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x10*} | Steam {0x4*,0x8*,0xc*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00484b20: 16-byte aligned: YES
  00484b20  55             PUSH EBP
  00484b21  8B EC          MOV EBP,ESP
  00484b23  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484b20
string literals (0):
direct callees (3):
  0x00919F10  _strlen
  0x0091FA20  (unnamed)
  0x0091CF23  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00484b20 FUN_00484b20 size=130 ret=0 callers=2 callees=3

undefined4 FUN_00484b20(char *param_1,char *param_2)

{
  size_t sVar1;
  size_t sVar2;
  size_t _MaxCount [GHIDRA-AUTO-LABEL: unreliable];
  int iVar3;
  char *local_8;
  
  local_8 = param_2;
  sVar1 = _strlen [GHIDRA-AUTO-LABEL: unreliable](param_1);
  sVar2 = _strlen [GHIDRA-AUTO-LABEL: unreliable](param_2);
  while( true ) {
    if (param_2 + sVar2 <= local_8) {
      return 0;
    }
    _MaxCount [GHIDRA-AUTO-LABEL: unreliable] = _strcspn [GHIDRA-AUTO-LABEL: unreliable](local_8," ");
    if ((sVar1 == _MaxCount [GHIDRA-AUTO-LABEL: unreliable]) && (iVar3 = _strncmp [GHIDRA-AUTO-LABEL: unreliable](param_1,local_8,_MaxCount), iVar3 == 0)) break;
    local_8 = local_8 + _MaxCount [GHIDRA-AUTO-LABEL: unreliable] + 1;
  }
  return 1;
}
```

### decoy: 0x00756380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00756380: 16-byte aligned: YES
  00756380  55             PUSH EBP
  00756381  8B EC          MOV EBP,ESP
  00756383  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00756380
string literals (0):
direct callees (2):
  0x007563B0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 130 B, RET 0
- callers 2: 0x004317d0 GLRender::InitExtensions, 0x00441dd0
- callees: 0x00919f10 _strlen (CALL), 0x0091fa20 (CALL), 0x0091cf23 (CALL)
- twins: none

## Card B17-10-20

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

## Card B17-10-21

GOG function: Scene::DoGobShadows  gog 0x0085ab60  callconv=(blank)  param_size_bytes=0  gog_size=876  gog_retn=0
Candidate Steam address: 0x004687e0 (size 876, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.86) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00468b50:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0085aa60 Scene::DoBlurBuckets_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0085aed0 Scene::DoGobShadowsForThisLight  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x00856330 CExoArrayList::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00858070 Global::ProjectDynamicShadow -> Steam ?  in candidate callees: n/a
  0x00873000 List_T__::operator= -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
  0x00901f50 LightManager::GetShadowLights -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 11 common): GOG {0x4*,0x8*,0x14*,0x50*,0x58*,0x60*,0x84*,0x8c*,0xee*,0x192*,0x1b4*} | Steam {0x4*,0x8*,0x14*,0x50*,0x58*,0x60*,0x84*,0x8c*,0xee*,0x192*,0x1b4*}
imm sets (* = in both, 8 common): GOG {0x4*,0x5*,0x8*,0x41*,0x78*,0x94*,0xa4*,0xb4*} | Steam {0x4*,0x5*,0x8*,0x41*,0x78*,0x94*,0xa4*,0xb4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004687e0: 16-byte aligned: YES
  004687e0  55             PUSH EBP
  004687e1  8B EC          MOV EBP,ESP
  004687e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004687e0
string literals (0):
direct callees (6):
  0x00463ED0  (unnamed)
  0x0050A630  (unnamed)
  0x00473FB0  (unnamed)
  0x00463B20  (unnamed)
  0x005142F0  (unnamed)
  0x00465C90  Global::ProjectDynamicShadow
struct offsets touched (top 10):
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,EDX
  +0x1B4 [dec 436]  x3  via ECX,EDX
  +0x14 [dec 20]  x2  via EDX
  +0x60 [dec 96]  x2  via EDX
  +0x84 [dec 132]  x2  via EAX,ECX
  +0x50 [dec 80]  x1  via ECX
  +0x58 [dec 88]  x1  via EDX
  +0x8C [dec 140]  x1  via EAX
  +0xEE [dec 238]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004687e0 FUN_004687e0 size=876 ret=0 callers=0 callees=6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004687e0(void)

{
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  float10 fVar5;
  float local_40;
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094a5a8;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00463ed0(0);
  local_8 = 0;
  uVar3 = FUN_0050a630(0);
  FUN_00473fb0(uVar3);
  if (local_18 == 0) {
    local_8 = 0xffffffff;
    FUN_00463b20(uVar2);
  }
  else {
    local_20 = DAT_00a732d4;
LAB_00468855:
    iVar4 = local_20 + -1;
    if (local_20 != 0) {
      local_20 = iVar4;
      if ((DAT_009f5d40 != 0) && (DAT_00a2004c != '\0')) goto code_r0x0046887d;
      goto LAB_004688d2;
    }
    local_8 = 0xffffffff;
    local_20 = iVar4;
    FUN_00463b20(uVar2);
  }
  ExceptionList = local_10;
  return;
code_r0x0046887d:
  if (iVar4 < DAT_00a732d4) {
    if (*(int *)(*(int *)(DAT_00a732d0 + iVar4 * 4) + 0x1b4) != 0) {
      (**(code **)(**(int **)(DAT_00a732d0 + iVar4 * 4) + 0x14))(_DAT_009f62b0);
    }
    if (local_20 < DAT_00a732d4) {
LAB_004688d2:
      if ((*(short *)(*(int *)(DAT_00a732d0 + local_20 * 4) + 0xee) != 0) &&
         (*(char *)(*(int *)(DAT_00a732d0 + local_20 * 4) + 0x192) != '\0')) {
        if (DAT_009f5d54 != 0) {
          iVar4 = *(int *)(DAT_00a732d0 + local_20 * 4);
          local_34 = *(float *)(DAT_00a20020 + 0xb4) - *(float *)(iVar4 + 0xa4);
          local_30 = *(float *)(DAT_00a20020 + 0xb8) - *(float *)(iVar4 + 0xa8);
          local_2c = *(float *)(DAT_00a20020 + 0xbc) - *(float *)(iVar4 + 0xac);
          fVar5 = (float10)FUN_005142f0(&local_34);
          if ((float10)_DAT_009f5d50 < fVar5) goto LAB_00468855;
        }
        if (((*(int *)(*(int *)(DAT_00a732d0 + local_20 * 4) + 0x84) == 0) ||
            ((*(ushort *)(*(int *)(*(int *)(DAT_00a732d0 + local_20 * 4) + 0x84) + 0x50) & 4) != 0))
           || ((*(int *)(*(int *)(DAT_00a732d0 + local_20 * 4) + 0x1b4) != 0 &&
               (iVar4 = (**(code **)(**(int **)(*(int *)(DAT_00a732d0 + local_20 * 4) + 0x1b4) +
                                    0x14))(), iVar4 != 0)))) {
          for (local_24 = 0; local_24 < local_18; local_24 = local_24 + 1) {
            local_28 = *(float *)(*(int *)(local_1c + local_24 * 4) + 0x58);
            if ((float)_DAT_00986b38 < *(float *)(*(int *)(local_1c + local_24 * 4) + 0x60)) {
              local_28 = *(float *)(*(int *)(local_1c + local_24 * 4) + 0x60);
/* ... 18 more lines truncated by --max-lines */
```

### decoy: 0x00468b50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00468b50: 16-byte aligned: YES
  00468b50  55             PUSH EBP
  00468b51  8B EC          MOV EBP,ESP
  00468b53  81 EC 8C 00 00 00 SUB ESP,0x8c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00468b50
string literals (0):
direct callees (6):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x00459E20  Gob::IsInCutscene
  0x00456B00  (unnamed)
  0x005142F0  (unnamed)
  0x00465C90  Global::ProjectDynamicShadow
struct offsets touched (top 10):
  +0x50 [dec 80]  x2  via EAX,EDX
  +0x60 [dec 96]  x2  via ECX,EDX
  +0x84 [dec 132]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x58 [dec 88]  x1  via EAX
  +0x64 [dec 100]  x1  via EAX
  +0x98 [dec 152]  x1  via ECX
```

### Static facts (script output, report-only)
- size 876 B, RET 0
- callees: 0x00463ed0 (CALL), 0x0050a630 (CALL), 0x00473fb0 (CALL), 0x00463b20 (CALL), 0x005142f0 (CALL), 0x00465c90 (CALL)
- indirect: 0x004688c3 slot 0x14, 0x00468a12 slot 0x14
- twins: none

## Card B17-10-22

GOG function: CSWCollisionMesh::not_GetWalkableMaterialMask  gog 0x00713530  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x00650d40 (size 158, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00650de0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00713490 Global::QueryTimer -> 0x00650d40  gate_class=REVIEW
- GOG-order neighbour after:  0x00713550 CSWRoomSurfaceMesh::CheckAABBNode  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*,0xf4240}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00650d40: 16-byte aligned: YES
  00650d40  55             PUSH EBP
  00650d41  8B EC          MOV EBP,ESP
  00650d43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00650d40
string literals (0):
direct callees (3):
  0x00650DE0  (unnamed)
  0x0091FC30  (unnamed)
  0x00920670  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00650d40 FUN_00650d40 size=158 ret=0 callers=1 callees=4

void FUN_00650d40(void)

{
  undefined8 uVar1;
  LARGE_INTEGER local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  uVar1 = CONCAT44(DAT_00a7e104,DAT_00a7e100);
  puStack_c = &LAB_00958c1e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((DAT_00a7e108 & 1) == 0) {
    DAT_00a7e108 = DAT_00a7e108 | 1;
    local_8 = 0;
    uVar1 = FUN_00650de0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  }
  DAT_00a7e104 = (undefined4)((ulonglong)uVar1 >> 0x20);
  DAT_00a7e100 = (undefined4)uVar1;
  local_8 = 0xffffffff;
  QueryPerformanceCounter(&local_18);
  uVar1 = __allmul [GHIDRA-AUTO-LABEL: unreliable](local_18.s.LowPart,local_18.s.HighPart,1000000,0);
  __alldiv [GHIDRA-AUTO-LABEL: unreliable](uVar1,DAT_00a7e100,DAT_00a7e104);
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00650de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00650de0: 16-byte aligned: YES
  00650de0  55             PUSH EBP
  00650de1  8B EC          MOV EBP,ESP
  00650de3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00650de0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 158 B, RET 0
- callers 1: 0x00650be0
- callees: 0x00650de0 (CALL), 0x000000d5 (CALL), 0x0091fc30 (CALL), 0x00920670 (CALL)
- imports: QueryPerformanceCounter (KERNEL32.DLL)
- twins: none
