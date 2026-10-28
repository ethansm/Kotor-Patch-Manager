# Review cards B17 chunk 7

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-7-01

GOG function: CExoStreamingSoundSourceInternal::IsDone  gog 0x0063cc00  callconv=(blank)  param_size_bytes=0  gog_size=71  gog_retn=0
Candidate Steam address: 0x0070dc60 (size 71, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0070daa0:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063ca40 CExoStreamingSoundSourceInternal::Update3DStream -> 0x0070daa0  gate_class=REVIEW
- GOG-order neighbour after:  0x0063cc50 CExoStreamingSoundSourceInternal::SetLooping -> 0x0070dcb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x24*} | Steam {0xc*,0x24*}
imm sets (* = in both, 2 common): GOG {0x2*,0xc*} | Steam {0x2*,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0070dc60: 16-byte aligned: YES
  0070dc60  55             PUSH EBP
  0070dc61  8B EC          MOV EBP,ESP
  0070dc63  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070dc60
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0070dc60 FUN_0070dc60 size=71 ret=0 callers=1 callees=1

bool __fastcall FUN_0070dc60(int *param_1)

{
  int iVar1;
  bool bVar2;
  
  if ((*(int *)(param_1[9 = byte +0x24] + 0xc) == 0) || (*param_1 == 0)) {
    bVar2 = false;
  }
  else {
    iVar1 = _AIL_stream_status_4 [GHIDRA-AUTO-LABEL: unreliable](*param_1);
    bVar2 = iVar1 == 2;
  }
  return bVar2;
}
```

### decoy: 0x0070daa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070daa0: 16-byte aligned: YES
  0070daa0  55             PUSH EBP
  0070daa1  8B EC          MOV EBP,ESP
  0070daa3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070daa0
string literals (0):
direct callees (4):
  0x00710980  (unnamed)
  0x0093720B  (unnamed)
  0x0070BC90  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### Static facts (script output, report-only)
- size 71 B, RET 0
- callers 1: 0x0070b110
- callees: 0x0000007f (CALL)
- imports: _AIL_stream_status@4 (MSS32.DLL)
- twins: none

## Card B17-7-02

GOG function: Global::InitOpenGLWindow  gog 0x00408600  callconv=(blank)  param_size_bytes=4  gog_size=320  gog_retn=0
Candidate Steam address: 0x004087a0 (size 839, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00408af0:STR1/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004082b0 Global::InitGameApp  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00408740 Global::ProcessCommandLine  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (2): 'Render Window'; 'GroupIcon'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0xb*,0x48,0x65*,0x40000,0x2ca0000*,0x80000000*} | Steam {0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xb*,0xc,0xd,0xe,0xf,0x28,0x65*,...(+4)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x004087a0: 16-byte aligned: YES
  004087a0  55             PUSH EBP
  004087a1  8B EC          MOV EBP,ESP
  004087a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004087a0
string literals (3):
  0x00986BF0  at 0040884f  '    v'
  0x009F42C8  at 00408996  'KotOR2'
  0x009F42C8  at 004089d3  'KotOR2'
direct callees (7):
  0x00736390  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x00733780  CExoString::~CExoString
  0x00734010  CExoString::CStr
  0x0091C630  (unnamed)
  0x00408AF0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004087a0 FUN_004087a0 size=839 ret=0 callers=1 callees=12

HWND FUN_004087a0(HINSTANCE param_1,undefined4 param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  LPCSTR lpWindowName;
  BOOL BVar5;
  DWORD dwStyle;
  int X;
  int Y;
  int nWidth;
  int nHeight;
  HWND pHVar6;
  HMENU hMenu;
  HINSTANCE hInstance;
  LPVOID lpParam;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  tagMONITORINFO local_78;
  HWND local_50;
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  WNDCLASSA local_3c;
  HMONITOR local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00947f3a;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00736390(uVar1);
  FUN_00733570(&DAT_00986bfc);
  local_8 = 0;
  FUN_00733df0(local_88,local_80);
  local_8._0_1_ = 1;
  FUN_00733df0(local_4c,&DAT_00a1b7a4);
  local_8._0_1_ = 3;
  FUN_00733780();
  local_8._0_1_ = 4;
  FUN_00733780();
  FUN_00733570(&DAT_00986bf8);
  local_8._0_1_ = 5;
  FUN_00733570("    v");
  local_8._0_1_ = 6;
  uVar2 = FUN_00733df0(local_98,local_90);
  local_8._0_1_ = 7;
  uVar3 = FUN_00733df0(local_a0,&DAT_00a1b7bc);
  local_8._0_1_ = 8;
  uVar8 = uVar3;
  uVar4 = FUN_00733df0(local_b0,local_a8);
  local_8._0_1_ = 9;
  uVar7 = uVar4;
  FUN_00733df0(local_44,&DAT_00a1b7b4);
  local_8._0_1_ = 0xb;
  FUN_00733780(uVar1,uVar4,uVar7,uVar3,uVar8,uVar2);
  local_8._0_1_ = 0xc;
  FUN_00733780();
  local_8._0_1_ = 0xd;
  FUN_00733780();
  local_8._0_1_ = 0xe;
  FUN_00733780();
  local_8._0_1_ = 0xf;
  FUN_00733780();
  local_3c.style = 0xb;
  local_3c.lpfnWndProc = FUN_0040a750;
  local_3c.cbClsExtra = 0;
  local_3c.cbWndExtra = 0;
/* ... 53 more lines truncated by --max-lines */
```

### decoy: 0x00408af0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00408af0: 16-byte aligned: YES
  00408af0  55             PUSH EBP
  00408af1  8B EC          MOV EBP,ESP
  00408af3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00408af0
string literals (1):
  0x00986C00  at 00408b18  'Render Window'
direct callees (4):
  0x007335F0  CExoString::CExoString_copy
  0x00734010  CExoString::CStr
  0x00733780  CExoString::~CExoString
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 839 B, RET 0
- callers 1: 0x00407920
- callees: 0x00736390 (CALL), 0x00733570 CExoString::CStrConstructor (CALL), 0x00733df0 CExoString::operator+ (CALL), 0x00733780 CExoString::Destructor (CALL), 0x0000013a (CALL), 0x00000134 (CALL), ...
- strings: 0x00986bf0 "    v", 0x009f42c8 "KotOR2"
- globals: 0x00a1b480 WRITE
- imports: CreateWindowExA (USER32.DLL), GetMonitorInfoA (USER32.DLL), GetStockObject (GDI32.DLL), ...
- twins: none

## Card B17-7-03

GOG function: LightManager::GetShadowLights  gog 0x00901f50  callconv=(blank)  param_size_bytes=4  gog_size=1048  gog_retn=4
Candidate Steam address: 0x0050aa90 (size 214, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.25) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0050ab70:STR0/CALLEE0/SIG0.117/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00901970 LightManager::ControlLightsFade  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00902370 LightManager::GetBumpMapLights  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00856330 CExoArrayList::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00873000 List_T__::operator= -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
  0x008fdd20 List_T__::Insert -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x8,0xc,0x50,0x54,0x58,0x5c,0x60,0x68,0x70,0x80,0x90,0x98,0x9c,...(+1)} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x2,0x4*,0x5,0x6,0x41,0x44*,0x98,0xac,0xb8} | Steam {0x4*,0x1c,0x44*,0x74,0x4001}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0050aa90: 16-byte aligned: YES
  0050aa90  55             PUSH EBP
  0050aa91  8B EC          MOV EBP,ESP
  0050aa93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0050aa90
string literals (0):
direct callees (5):
  0x00463ED0  (unnamed)
  0x00473FB0  (unnamed)
  0x00476620  (unnamed)
  0x00463B20  (unnamed)
  0x004142E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0050aa90 FUN_0050aa90 size=214 ret=4 callers=1 callees=6

bool __thiscall FUN_0050aa90(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094d3d8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00463ed0(0);
  local_8 = 0;
  if (*(int *)(param_1 + 4) == 0) {
    FUN_00473fb0(param_1 + 0x44);
  }
  else {
    FUN_00473fb0(param_1 + 0x74);
  }
  iVar2 = FUN_00476620(param_2);
  if (iVar2 != 0) {
    iVar3 = FUN_004142e0(param_2);
    glDisable(iVar3 + 0x4001);
    local_8 = 0xffffffff;
    FUN_00463b20();
  }
  else {
    local_8 = 0xffffffff;
    FUN_00463b20(uVar1);
  }
  ExceptionList = local_10;
  return iVar2 != 0;
}
```

### decoy: 0x0050ab70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0050ab70: 16-byte aligned: YES
  0050ab70  55             PUSH EBP
  0050ab71  8B EC          MOV EBP,ESP
  0050ab73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0050ab70
string literals (0):
direct callees (5):
  0x00463ED0  (unnamed)
  0x00473FB0  (unnamed)
  0x00476620  (unnamed)
  0x00463B20  (unnamed)
  0x004142E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### Static facts (script output, report-only)
- size 214 B, RET 4
- callers 1: 0x0050ac50
- callees: 0x00463ed0 (CALL), 0x00473fb0 (CALL), 0x00476620 (CALL), 0x00463b20 (CALL), 0x004142e0 (CALL), 0x0000001c (CALL)
- imports: glDisable (OPENGL32.DLL)
- twins: 0x0050ab70 (distinguishing: 0x0050ab36: CALL dword ptr [0x009862a0])

## Card B17-7-04

GOG function: LightManager::GetShadowLights  gog 0x00901f50  callconv=(blank)  param_size_bytes=4  gog_size=1048  gog_retn=4
Candidate Steam address: 0x00508f70 (size 314, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=LightManager::EnableHighestPriorityLights | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005090b0:STR0/CALLEE0/SIG0.163/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00901970 LightManager::ControlLightsFade  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00902370 LightManager::GetBumpMapLights  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00856330 CExoArrayList::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00873000 List_T__::operator= -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
  0x008fdd20 List_T__::Insert -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x50,0x54,0x58,0x5c,0x60,0x68,0x70,0x80,0x90,0x98,0x9c,...(+1)} | Steam {0x44,0x48,0x8c}
imm sets (* = in both, 1 common): GOG {0x2,0x4*,0x5,0x6,0x41,0x44,0x98,0xac,0xb8} | Steam {0x3,0x4*,0xc,0x14,0x40,0x4c,0x58,0x68,0x94,0x4001}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00508f70: 16-byte aligned: YES
  00508f70  55             PUSH EBP
  00508f71  8B EC          MOV EBP,ESP
  00508f73  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00508f70
string literals (0):
direct callees (2):
  0x00436200  (unnamed)
  0x00508350  (unnamed)
struct offsets touched (top 3):
  +0x44 [dec 68]  x5  via EAX,ECX,EDX
  +0x48 [dec 72]  x3  via EAX,ECX
  +0x8C [dec 140]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00508f70 FUN_00508f70 size=314 ret=4 callers=0 callees=4

void __thiscall FUN_00508f70(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int local_44;
  undefined1 local_1c [12];
  int local_10;
  int local_c;
  int local_8;
  
  if (*(int *)(param_1 + 0x48) != 0) {
    if (param_2 < *(int *)(param_1 + 0x48)) {
      local_44 = param_2;
    }
    else {
      local_44 = *(int *)(param_1 + 0x48);
    }
    local_8 = local_44;
    for (local_10 = 0; local_10 < local_8; local_10 = local_10 + 1) {
      glEnable(local_10 + 0x4001);
      iVar2 = *(int *)(param_1 + 0x44);
      iVar1 = local_10 * 4;
      uVar3 = FUN_00436200(local_1c,*(int *)(*(int *)(param_1 + 0x44) + local_10 * 4) + 0x4c,
                           *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x44) + local_10 * 4) + 0x8c)
                           ,*(int *)(*(int *)(param_1 + 0x44) + local_10 * 4) + 0x58,
                           *(int *)(*(int *)(param_1 + 0x44) + local_10 * 4) + 0x68);
      FUN_00508350(local_10,*(int *)(iVar2 + iVar1) + 0x94,uVar3);
    }
    for (local_c = local_10; local_c < 3; local_c = local_c + 1) {
      glDisable(local_c + 0x4001);
    }
  }
  return;
}
```

### decoy: 0x005090b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005090b0: 16-byte aligned: YES
  005090b0  55             PUSH EBP
  005090b1  8B EC          MOV EBP,ESP
  005090b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005090b0
string literals (0):
direct callees (11):
  0x00463ED0  (unnamed)
  0x0091D3BA  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
  0x004544E0  (unnamed)
  0x00476620  (unnamed)
  0x0049B840  (unnamed)
  0x00463B50  (unnamed)
  0x00451660  (unnamed)
  0x00473FB0  (unnamed)
  0x004516D0  (unnamed)
  0x00508350  (unnamed)
struct offsets touched (top 10):
  +0x20 [dec 32]  x17  via EAX,ECX,EDX
  +0x4 [dec 4]  x10  via EAX,ECX,EDX
  +0x1C [dec 28]  x6  via EAX,ECX,EDX
  +0x44 [dec 68]  x2  via EAX,EDX
  +0x48 [dec 72]  x2  via EAX,ECX
  +0x78 [dec 120]  x2  via EAX
  +0xA8 [dec 168]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x5C [dec 92]  x1  via ECX
  +0xDC [dec 220]  x1  via EAX
```

### Static facts (script output, report-only)
- size 314 B, RET 4
- callees: 0x0000000e (CALL), 0x00436200 (CALL), 0x00508350 (CALL), 0x0000001c (CALL)
- imports: glDisable (OPENGL32.DLL), glEnable (OPENGL32.DLL)
- twins: none

## Card B17-7-05

GOG function: CSWGuiObject::SetActiveControl  gog 0x005c0410  callconv=(blank)  param_size_bytes=0  gog_size=13  gog_retn=8
Candidate Steam address: 0x0042a220 (size 428, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
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

## Card B17-7-06

GOG function: CExoStreamingSoundSourceInternal::Update3DStream  gog 0x0063ca40  callconv=(blank)  param_size_bytes=0  gog_size=438  gog_retn=0
Candidate Steam address: 0x0070daa0 (size 438, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0070dc60:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063c970 CExoStreamingSoundSourceInternal::SetPriorityGroup  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063cc00 CExoStreamingSoundSourceInternal::IsDone -> 0x0070dc60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0061a2d0 CResRef::CopyToString_2 -> Steam ?  in candidate callees: n/a
  0x0063ac30 CExoSound::GetSoundMode -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x0093e9ce Global::strncmp -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 8): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x24*} | Steam {0xc*,0x24*}
imm sets (* = in both, 5 common): GOG {0x3*,0x4*,0xc*,0xd*,0x40*} | Steam {0x3*,0x4*,0xc*,0xd*,0x40*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0070daa0: 16-byte aligned: YES
  0070daa0  55             PUSH EBP
  0070daa1  8B EC          MOV EBP,ESP
  0070daa3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070daa0
string literals (0):
direct callees (4):
  0x00710980  (unnamed)
  0x0093720B  (unnamed)
  0x0070BC90  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0070daa0 FUN_0070daa0 size=438 ret=0 callers=8 callees=5

void __fastcall FUN_0070daa0(int *param_1)

{
  int iVar1;
  char local_34 [20];
  char local_20 [20];
  uint local_c;
  undefined4 local_8;
  
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if ((*(int *)(param_1[9 = byte +0x24] + 0xc) == 0) || (*param_1 == 0)) {
    if (DAT_00a7e119 != '\0') {
      FUN_00710980(local_34);
      iVar1 = __strnicmp [GHIDRA-AUTO-LABEL: unreliable](local_34,"al_",3);
      if ((((iVar1 != 0) && (iVar1 = __strnicmp [GHIDRA-AUTO-LABEL: unreliable](local_34,"as_",3), iVar1 != 0)) &&
          (iVar1 = __strnicmp [GHIDRA-AUTO-LABEL: unreliable](local_34,"mus_",4), iVar1 != 0)) &&
         ((iVar1 = __strnicmp [GHIDRA-AUTO-LABEL: unreliable](local_34,"amb_",4), iVar1 != 0 &&
          (iVar1 = __strnicmp [GHIDRA-AUTO-LABEL: unreliable](local_34,"bed_",4), iVar1 != 0)))) {
        FUN_0070bc90();
      }
    }
  }
  else if (DAT_00a7e119 == '\0') {
    _AIL_stream_status_4 [GHIDRA-AUTO-LABEL: unreliable](*param_1);
  }
  else {
    FUN_00710980(local_20);
    local_8 = _AIL_stream_status_4 [GHIDRA-AUTO-LABEL: unreliable](*param_1);
    iVar1 = __strnicmp [GHIDRA-AUTO-LABEL: unreliable](local_20,"al_",3);
    if (((iVar1 != 0) && (iVar1 = __strnicmp [GHIDRA-AUTO-LABEL: unreliable](local_20,"as_",3), iVar1 != 0)) &&
       ((iVar1 = __strnicmp [GHIDRA-AUTO-LABEL: unreliable](local_20,"mus_",4), iVar1 != 0 &&
        (iVar1 = __strnicmp [GHIDRA-AUTO-LABEL: unreliable](local_20,"amb_",4), iVar1 != 0)))) {
      __strnicmp [GHIDRA-AUTO-LABEL: unreliable](local_20,"bed_",4);
    }
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x0070dc60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070dc60: 16-byte aligned: YES
  0070dc60  55             PUSH EBP
  0070dc61  8B EC          MOV EBP,ESP
  0070dc63  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070dc60
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### Static facts (script output, report-only)
- size 438 B, RET 0
- callers 8: 0x007051c0, 0x00705d70, 0x00707050, 0x0070b0e0, 0x0070dcb0, ...
- callees: 0x00710980 (CALL), 0x0000007f (CALL), 0x0093720b (CALL), 0x0070bc90 (CALL), 0x009196ee __security_check_cookie (CALL)
- globals: 0x00a1b494 READ
- imports: _AIL_stream_status@4 (MSS32.DLL)
- twins: none

## Card B17-7-07

GOG function: CServerAIBodyBagInfo::LoadBodyBag  gog 0x006509e0  callconv=(blank)  param_size_bytes=8  gog_size=147  gog_retn=8
Candidate Steam address: 0x00520fc0 (size 147, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00520f40:STR1/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00650960 CServerAIBodyBagInfo::SaveBodyBag  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00650a80 CSWGlobalVariableTable::Constructor -> 0x00653bc0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00624d60 CResGFF::ReadFieldDWORD -> Steam 0x00718c00  in candidate callees: yes
  0x00624e60 CResGFF::ReadFieldFLOAT -> Steam 0x00718d00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (4): 'PositionZ'; 'PositionY'; 'PositionX'; 'BodyBagId'
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
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
prologue for 0x00520fc0: 16-byte aligned: YES
  00520fc0  55             PUSH EBP
  00520fc1  8B EC          MOV EBP,ESP
  00520fc3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00520fc0
string literals (4):
  0x00992EF0  at 00520fcf  'BodyBagId'
  0x00992EE4  at 00520fef  'PositionX'
  0x00992ED8  at 00521010  'PositionY'
  0x00992ECC  at 00521031  'PositionZ'
direct callees (2):
  0x00718C00  CResGFF::ReadFieldDWORD
  0x00718D00  CResGFF::ReadFieldFLOAT
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00520fc0 FUN_00520fc0 size=147 ret=8 callers=1 callees=2

undefined4 __thiscall FUN_00520fc0(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  float10 fVar2;
  undefined1 local_8 [4];
  
  uVar1 = FUN_00718c00(param_3,"BodyBagId",local_8,0);
  *param_1 = uVar1;
  fVar2 = (float10)FUN_00718d00(param_3,"PositionX",local_8,0);
  param_1[1 = byte +0x4] = (float)fVar2;
  fVar2 = (float10)FUN_00718d00(param_3,"PositionY",local_8,0);
  param_1[2 = byte +0x8] = (float)fVar2;
  fVar2 = (float10)FUN_00718d00(param_3,"PositionZ",local_8,0);
  param_1[3 = byte +0xc] = (float)fVar2;
  return 1;
}
```

### decoy: 0x00520f40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00520f40: 16-byte aligned: YES
  00520f40  55             PUSH EBP
  00520f41  8B EC          MOV EBP,ESP
  00520f43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00520f40
string literals (4):
  0x00992EF0  at 00520f47  'BodyBagId'
  0x00992EE4  at 00520f5e  'PositionX'
  0x00992ED8  at 00520f79  'PositionY'
  0x00992ECC  at 00520f94  'PositionZ'
direct callees (2):
  0x00719B90  CResGFF::WriteFieldDWORD
  0x00719CE0  CResGFF::WriteFieldFLOAT
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
```

### Static facts (script output, report-only)
- size 147 B, RET 8
- callers 1: 0x005205f0
- callees: 0x00718c00 CResGFF::ReadFieldDWORD (CALL), 0x00718d00 CResGFF::ReadFieldFLOAT (CALL)
- twins: none

## Card B17-7-08

GOG function: CExoSoundInternal::SetListenerOrientation  gog 0x00637160  callconv=(blank)  param_size_bytes=8  gog_size=232  gog_retn=8
Candidate Steam address: 0x007081e0 (size 232, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007082d0:STR0/CALLEE0/SIG0.571/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00636ff0 CExoSoundInternal::PlaySourceOn2DVoice  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00637250 CExoSoundInternal::SetListenerPosition -> 0x007082d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080f810 Global::equal -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x10*,0x84*,0x90*} | Steam {0x8*,0x10*,0x84*,0x90*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007081e0: 16-byte aligned: YES
  007081e0  55             PUSH EBP
  007081e1  8B EC          MOV EBP,ESP
  007081e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007081e0
string literals (0):
direct callees (1):
  0x00517980  Vec3::Equal
struct offsets touched (top 2):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007081e0 FUN_007081e0 size=232 ret=8 callers=2 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007081e0(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  int iVar1;
  undefined4 local_8;
  
  iVar1 = FUN_00517980(param_1 + 0x84,param_2);
  if ((iVar1 == 0) || (iVar1 = FUN_00517980(param_1 + 0x90,param_3), iVar1 == 0)) {
    *(undefined4 *)(param_1 + 0x84) = *param_2;
    *(undefined4 *)(param_1 + 0x88) = param_2[1 = byte +0x4];
    *(undefined4 *)(param_1 + 0x8c) = param_2[2 = byte +0x8];
    *(undefined4 *)(param_1 + 0x90) = *param_3;
    *(undefined4 *)(param_1 + 0x94) = param_3[1 = byte +0x4];
    *(undefined4 *)(param_1 + 0x98) = param_3[2 = byte +0x8];
    _AIL_quick_handles_12 [GHIDRA-AUTO-LABEL: unreliable](&local_8,0,0);
    _AIL_set_listener_3D_orientation_28 [GHIDRA-AUTO-LABEL: unreliable]
              (local_8,*param_2,param_2[1 = byte +0x4],(float)param_2[2 = byte +0x8] * (float)_DAT_00986d98,*param_3,
               param_3[1 = byte +0x4],(float)param_3[2 = byte +0x8] * (float)_DAT_00986d98);
  }
  return;
}
```

### decoy: 0x007082d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007082d0: 16-byte aligned: YES
  007082d0  55             PUSH EBP
  007082d1  8B EC          MOV EBP,ESP
  007082d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007082d0
string literals (0):
direct callees (1):
  0x00517980  Vec3::Equal
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 232 B, RET 8
- callers 2: 0x00705d70, 0x0070b910
- callees: 0x00517980 (CALL), 0x00000062 (CALL), 0x00000087 (CALL)
- imports: _AIL_quick_handles@12 (MSS32MIDI.DLL), _AIL_set_listener_3D_orientation@28 (MSS32.DLL)
- twins: none

## Card B17-7-09

GOG function: Global::InitOpenGLWindow  gog 0x00408600  callconv=(blank)  param_size_bytes=4  gog_size=320  gog_retn=0
Candidate Steam address: 0x00408c30 (size 462, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.05 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00408af0:STR1/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004082b0 Global::InitGameApp  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00408740 Global::ProcessCommandLine  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (2): 'Render Window'; 'GroupIcon'
disp sets (* = in both, 0 common): GOG {} | Steam {0x4}
imm sets (* = in both, 1 common): GOG {0x4*,0xb,0x48,0x65,0x40000,0x2ca0000,0x80000000} | Steam {0x4*,0xc,0x1e,0x24}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00408c30: 16-byte aligned: YES
  00408c30  55             PUSH EBP
  00408c31  8B EC          MOV EBP,ESP
  00408c33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00408c30
string literals (2):
  0x00986C28  at 00408c8e  'capture'
  0x00986C28  at 00408d0d  'capture'
direct callees (8):
  0x0091CF23  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x007338D0  CExoString::operator=_2
  0x0073FA50  CClientExoApp::SetCaptureEvents
  0x00733780  CExoString::~CExoString
  0x0073FA70  CClientExoApp::SetCaptureEventPlayBack
  0x0091CF18  (unnamed)
  0x0073FA90  CClientExoApp::SetCaptureEventPlayBackRate
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00408c30 FUN_00408c30 size=462 ret=0 callers=1 callees=8

void FUN_00408c30(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 local_28;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00947fa0;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  for (local_14 = 1; local_14 < param_1; local_14 = local_14 + 1) {
    iVar2 = _strncmp [GHIDRA-AUTO-LABEL: unreliable](*(char **)(param_2 + local_14 * 4),"-ecf",4);
    if (iVar2 == 0) {
      local_14 = local_14 + 1;
      FUN_00733570("capture");
      local_8 = 0;
      if (local_14 < param_1) {
        FUN_007338d0(*(undefined4 *)(param_2 + local_14 * 4));
      }
      FUN_0073fa50(local_1c);
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    else {
      iVar2 = _strncmp [GHIDRA-AUTO-LABEL: unreliable](*(char **)(param_2 + local_14 * 4),"-epf",4);
      if (iVar2 == 0) {
        local_14 = local_14 + 1;
        FUN_00733570("capture");
        local_8 = 1;
        if (local_14 < param_1) {
          FUN_007338d0(*(undefined4 *)(param_2 + local_14 * 4));
        }
        FUN_0073fa70(local_24);
        local_8 = 0xffffffff;
        FUN_00733780();
      }
      else {
        iVar2 = _strncmp [GHIDRA-AUTO-LABEL: unreliable](*(char **)(param_2 + local_14 * 4),"-epr",4);
        if (iVar2 == 0) {
          local_14 = local_14 + 1;
          local_28 = 0x1e;
          if (local_14 < param_1) {
            local_28 = FUN_0091cf18(*(undefined4 *)(param_2 + local_14 * 4),uVar1);
          }
          FUN_0073fa90(local_28);
        }
        else {
          _strncmp [GHIDRA-AUTO-LABEL: unreliable](*(char **)(param_2 + local_14 * 4),"-epm",4);
        }
      }
    }
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00408af0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00408af0: 16-byte aligned: YES
  00408af0  55             PUSH EBP
  00408af1  8B EC          MOV EBP,ESP
  00408af3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00408af0
string literals (1):
  0x00986C00  at 00408b18  'Render Window'
direct callees (4):
  0x007335F0  CExoString::CExoString_copy
  0x00734010  CExoString::CStr
  0x00733780  CExoString::~CExoString
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 462 B, RET 0
- callers 1: 0x00407920
- callees: 0x0091cf23 (CALL), 0x00733570 CExoString::CStrConstructor (CALL), 0x007338d0 (CALL), 0x0073fa50 CClientExoApp::SetCaptureEvents (CALL), 0x00733780 CExoString::Destructor (CALL), 0x0073fa70 CClientExoApp::SetCaptureEventPlayBack (CALL), ...
- strings: 0x00986c28 "capture"
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B17-7-10

GOG function: CExoSoundInternal::SetListenerPosition  gog 0x00637250  callconv=(blank)  param_size_bytes=4  gog_size=135  gog_retn=4
Candidate Steam address: 0x007082d0 (size 135, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007081e0:STR0/CALLEE0/SIG0.571/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00637160 CExoSoundInternal::SetListenerOrientation -> 0x007081e0  gate_class=REVIEW
- GOG-order neighbour after:  0x006372e0 CExoSoundInternal::PlayOneShotSound  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080f810 Global::equal -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x9c*} | Steam {0x4*,0x8*,0xc*,0x9c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007082d0: 16-byte aligned: YES
  007082d0  55             PUSH EBP
  007082d1  8B EC          MOV EBP,ESP
  007082d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007082d0
string literals (0):
direct callees (1):
  0x00517980  Vec3::Equal
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007082d0 FUN_007082d0 size=135 ret=4 callers=2 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007082d0(int param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 local_8;
  
  iVar1 = FUN_00517980(param_1 + 0x9c,param_2);
  if (iVar1 == 0) {
    *(undefined4 *)(param_1 + 0x9c) = *param_2;
    *(undefined4 *)(param_1 + 0xa0) = param_2[1 = byte +0x4];
    *(undefined4 *)(param_1 + 0xa4) = param_2[2 = byte +0x8];
    _AIL_quick_handles_12 [GHIDRA-AUTO-LABEL: unreliable](&local_8,0,0);
    _AIL_set_listener_3D_position_16 [GHIDRA-AUTO-LABEL: unreliable]
              (local_8,*param_2,param_2[1 = byte +0x4],(float)param_2[2 = byte +0x8] * (float)_DAT_00986d98);
  }
  return;
}
```

### decoy: 0x007081e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007081e0: 16-byte aligned: YES
  007081e0  55             PUSH EBP
  007081e1  8B EC          MOV EBP,ESP
  007081e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007081e0
string literals (0):
direct callees (1):
  0x00517980  Vec3::Equal
struct offsets touched (top 2):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 135 B, RET 4
- callers 2: 0x00705d70, 0x0070b940
- callees: 0x00517980 (CALL), 0x00000062 (CALL), 0x00000086 (CALL)
- imports: _AIL_quick_handles@12 (MSS32MIDI.DLL), _AIL_set_listener_3D_position@16 (MSS32.DLL)
- twins: none

## Card B17-7-11

GOG function: CExoSound::ReInitialize  gog 0x0063a950  callconv=(blank)  param_size_bytes=8  gog_size=129  gog_retn=8
Candidate Steam address: 0x0070b9b0 (size 129, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0070ba40:STR0/CALLEE0/SIG0.125/SRV0/VT0
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
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0x10*,0x18*} | Steam {0x8*,0xc*,0x10*,0x18*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0070b9b0: 16-byte aligned: YES
  0070b9b0  55             PUSH EBP
  0070b9b1  8B EC          MOV EBP,ESP
  0070b9b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070b9b0
string literals (0):
direct callees (3):
  0x0070B8C0  (unnamed)
  0x0070B890  (unnamed)
  0x007051C0  CExoSoundInternal::Initialize/ReInitialize (ret 16)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0070b9b0 FUN_0070b9b0 size=129 ret=8 callers=1 callees=3

void __thiscall FUN_0070b9b0(int *param_1,undefined4 param_2,undefined4 param_3)

{
  byte bVar1;
  undefined1 uVar2;
  undefined1 uVar3;
  
  if (*param_1 != 0) {
    bVar1 = FUN_0070b8c0();
    if (bVar1 < 0x10) {
      uVar2 = 0x10;
    }
    else {
      uVar2 = FUN_0070b8c0();
    }
    bVar1 = FUN_0070b890();
    if (bVar1 < 0x18) {
      uVar3 = 0x18;
    }
    else {
      uVar3 = FUN_0070b890();
    }
    FUN_007051c0(uVar3,uVar2,param_2,param_3);
  }
  return;
}
```

### decoy: 0x0070ba40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070ba40: 16-byte aligned: YES
  0070ba40  55             PUSH EBP
  0070ba41  8B EC          MOV EBP,ESP
  0070ba43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070ba40
string literals (0):
direct callees (1):
  0x00708360  CExoSoundInternal::PlayOneShotSound (ret 24)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 129 B, RET 8
- callers 1: 0x007db680
- callees: 0x0070b8c0 (CALL), 0x0070b890 (CALL), 0x007051c0 (CALL)
- twins: none

## Card B17-7-12

GOG function: CExoSoundSource::SetResRef  gog 0x00639ec0  callconv=(blank)  param_size_bytes=8  gog_size=90  gog_retn=8
Candidate Steam address: 0x0070af20 (size 90, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0070aee0:STR0/CALLEE0/SIG0.35/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00639e80 CExoSoundSource::SetDistance  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00639f20 CExoSoundSource::Stop  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00634cf0 CExoSoundInternal::Render -> Steam ?  in candidate callees: n/a
  0x00639850 unnamed -> Steam ?  in candidate callees: n/a
  0x00639f20 CExoSoundSource::Stop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x4c*,0x60*} | Steam {0x4*,0x4c*,0x60*}
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
prologue for 0x0070af20: 16-byte aligned: YES
  0070af20  55             PUSH EBP
  0070af21  8B EC          MOV EBP,ESP
  0070af23  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070af20
string literals (0):
direct callees (3):
  0x0070AF80  (unnamed)
  0x00705D70  CExoSoundInternal::Render (per-frame)
  0x0070A8D0  Source::LoadWaveResource (ret 8)
struct offsets touched (top 3):
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x4C [dec 76]  x1  via EDX
  +0x60 [dec 96]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0070af20 FUN_0070af20 size=90 ret=8 callers=2 callees=3

void __thiscall FUN_0070af20(int param_1,undefined4 param_2,undefined4 param_3)

{
  if (*(int *)(param_1 + 4) != 0) {
    Decref();
    *(undefined4 *)(*(int *)(param_1 + 4) + 0x60) = 0;
    FUN_00705d70(param_1,*(undefined4 *)(*(int *)(param_1 + 4) + 0x4c));
    FUN_0070a8d0(param_2,param_3);
  }
  return;
}
```

### decoy: 0x0070aee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070aee0: 16-byte aligned: YES
  0070aee0  55             PUSH EBP
  0070aee1  8B EC          MOV EBP,ESP
  0070aee3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070aee0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x74 [dec 116]  x1  via ECX
  +0x78 [dec 120]  x1  via EAX
```

### Static facts (script output, report-only)
- size 90 B, RET 8
- callers 2: 0x00801090, 0x00835980
- callees: 0x0070af80 (CALL), 0x00705d70 (CALL), 0x0070a8d0 (CALL)
- twins: none

## Card B17-7-13

GOG function: CSWGuiOptionsGraphicsAdvanced::OnTexQualLeft  gog 0x0056d040  callconv=(blank)  param_size_bytes=4  gog_size=84  gog_retn=4
Candidate Steam address: 0x007d76e0 (size 65, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.29 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d7730:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0056cff0 CSWGuiOptionsGraphicsAdvanced::OnDefault  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0056d0a0 CSWGuiOptionsGraphicsAdvanced::OnTexQualRight  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0056d100 CSWGuiOptionsGraphicsAdvanced::SetTexQualText -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c,0x50*,0x2920} | Steam {0x4,0x50*}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*,0xc}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007d76e0: 16-byte aligned: YES
  007d76e0  55             PUSH EBP
  007d76e1  8B EC          MOV EBP,ESP
  007d76e3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d76e0
string literals (0):
direct callees (3):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007BBDC0  (unnamed)
  0x007D8190  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007d76e0 FUN_007d76e0 size=65 ret=4 callers=1 callees=3

void __thiscall FUN_007d76e0(undefined4 param_1,int param_2)

{
  if (*(int *)(param_2 + 0x50) != 0) {
    FUN_0072fb00(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
    FUN_007bbdc0();
    FUN_007d8190();
  }
  return;
}
```

### decoy: 0x007d7730 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d7730: 16-byte aligned: YES
  007d7730  55             PUSH EBP
  007d7731  8B EC          MOV EBP,ESP
  007d7733  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d7730
string literals (0):
direct callees (2):
  0x007D77F0  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 3):
  +0x2920 [dec 10528]  x3  via EAX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
```

### Static facts (script output, report-only)
- size 65 B, RET 4
- callers 1: 0x007d7290
- callees: 0x0072fb00 CClientExoApp::GetClientOptions (CALL), 0x007bbdc0 (CALL), 0x007d8190 (CALL)
- twins: none

## Card B17-7-14

GOG function: Scene::DoGobShadows  gog 0x0085ab60  callconv=(blank)  param_size_bytes=0  gog_size=876  gog_retn=0
Candidate Steam address: 0x004691d0 (size 220, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00468de0:STR0/CALLEE0/SIG0.222/SRV0/VT0
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
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x14,0x50,0x58,0x60,0x84,0x8c*,0xee,0x192,0x1b4} | Steam {0x64,0x8c*,0xa4}
imm sets (* = in both, 0 common): GOG {0x4,0x5,0x8,0x41,0x78,0x94,0xa4,0xb4} | Steam {0x2,0x1c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004691d0: 16-byte aligned: YES
  004691d0  55             PUSH EBP
  004691d1  8B EC          MOV EBP,ESP
  004691d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004691d0
string literals (0):
direct callees (3):
  0x0050A630  (unnamed)
  0x0049B6F0  (unnamed)
  0x00463B20  (unnamed)
struct offsets touched (top 3):
  +0x8C [dec 140]  x2  via EAX
  +0x64 [dec 100]  x1  via EDX
  +0xA4 [dec 164]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004691d0 FUN_004691d0 size=220 ret=0 callers=0 callees=3

void __fastcall FUN_004691d0(int param_1)

{
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  int local_20;
  int *local_1c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094a608;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar1 = *(int *)(*(int *)(param_1 + 0x8c) + 0xa4);
  uVar3 = FUN_0050a630(1);
  FUN_0049b6f0(uVar3);
  local_8 = 0;
  if ((local_18 == 1) && (iVar1 != *local_1c)) {
    local_8 = 0xffffffff;
    FUN_00463b20(uVar2);
  }
  else {
    for (local_20 = 0; local_20 < DAT_00a73348; local_20 = local_20 + 1) {
      (**(code **)(**(int **)(DAT_00a73344 + local_20 * 4) + 0x64 [dec 100]))(0);
    }
    local_8 = 0xffffffff;
    FUN_00463b20(uVar2);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00468de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00468de0: 16-byte aligned: YES
  00468de0  55             PUSH EBP
  00468de1  8B EC          MOV EBP,ESP
  00468de3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00468de0
string literals (0):
direct callees (9):
  0x0091D288  operator new[]
  0x0091EBAD  (unnamed)
  0x0091CB24  (unnamed)
  0x004844C0  (unnamed)
  0x00484850  (unnamed)
  0x004848B0  (unnamed)
  0x0091EA50  (unnamed)
  0x0091E770  (unnamed)
  0x004316B0  (unnamed)
struct offsets touched (top 10):
  +0xBC [dec 188]  x4  via EAX,ECX,EDX
  +0x988530 [dec 9995568]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x988588 [dec 9995656]  x3  via EAX,ECX,EDX
  +0x9F4928 [dec 10438952]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x60 [dec 96]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x58 [dec 88]  x1  via EAX
  +0x94 [dec 148]  x1  via EAX
```

### Static facts (script output, report-only)
- size 220 B, RET 0
- callees: 0x0050a630 (CALL), 0x0049b6f0 (CALL), 0x00463b20 (CALL)
- indirect: 0x0046928a slot 0x64
- twins: none

## Card B17-7-15

GOG function: CPathfindInformation::Initialize  gog 0x006470b0  callconv=(blank)  param_size_bytes=0  gog_size=1447  gog_retn=0
Candidate Steam address: 0x006f9440 (size 1447, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006f99f0:STR0/CALLEE0/SIG0.127/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00647090 CPathfindInformation::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00647660 CPathfindInformation::Shutdown  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 97 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x38*,0x40*,0x44*,...(+83)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x38*,0x40*,0x44*,...(+83)}
imm sets (* = in both, 9 common): GOG {0x4*,0x44*,0x108*,0x114*,0x170*,0x17c*,0x258*,0x2bc*,0x7f000000*} | Steam {0x4*,0x44*,0x108*,0x114*,0x170*,0x17c*,0x258*,0x2bc*,0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006f9440: 16-byte aligned: YES
  006f9440  55             PUSH EBP
  006f9441  8B EC          MOV EBP,ESP
  006f9443  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006f9440
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x18C [dec 396]  x3  via EAX,EDX
  +0x194 [dec 404]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006f9440 FUN_006f9440 size=1447 ret=0 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_006f9440(undefined4 *param_1)

{
  *param_1 = 0;
  param_1[1 = byte +0x4] = _DAT_0098bd78;
  param_1[2 = byte +0x8] = _DAT_0098bd78;
  param_1[4 = byte +0x10] = _DAT_0098bd78;
  param_1[5 = byte +0x14] = _DAT_0098c014;
  param_1[7 = byte +0x1c] = 0;
  param_1[8 = byte +0x20] = 0;
  param_1[9 = byte +0x24] = 0;
  param_1[0x65 = byte +0x194] = 0;
  param_1[0xb = byte +0x2c] = 0x7f000000;
  param_1[0xc = byte +0x30] = 0x7f000000;
  param_1[0x10 = byte +0x40] = 0;
  param_1[0x11 = byte +0x44] = 0;
  param_1[0x12 = byte +0x48] = 0;
  param_1[0x13 = byte +0x4c] = 0;
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
  param_1[0x1f = byte +0x7c] = 0;
  param_1[0x20 = byte +0x80] = 0;
  param_1[0x21 = byte +0x84] = 0;
  param_1[0x22 = byte +0x88] = 1;
  param_1[0x23 = byte +0x8c] = 0;
  param_1[0x24 = byte +0x90] = 0;
  param_1[0x25 = byte +0x94] = 0;
  param_1[0x26 = byte +0x98] = 0;
  param_1[0x27 = byte +0x9c] = 0;
  param_1[0x28 = byte +0xa0] = 0;
  param_1[0x29 = byte +0xa4] = 0;
  param_1[0x2a = byte +0xa8] = 0;
  param_1[0x2b = byte +0xac] = 0;
  param_1[0x2c = byte +0xb0] = 0;
  param_1[0x2d = byte +0xb4] = 0;
  param_1[0x2e = byte +0xb8] = 0;
  param_1[0x30 = byte +0xc0] = 0;
  param_1[0x31 = byte +0xc4] = 0;
  param_1[0x32 = byte +0xc8] = 0;
  param_1[0x33 = byte +0xcc] = 0;
  param_1[0x34 = byte +0xd0] = 0;
  param_1[0x35 = byte +0xd4] = 0;
  param_1[0x36 = byte +0xd8] = 0;
  param_1[0x37 = byte +0xdc] = 0;
  param_1[0x38 = byte +0xe0] = 0;
  param_1[0x39 = byte +0xe4] = 0;
  param_1[0x3c = byte +0xf0] = 0;
  param_1[0x3d = byte +0xf4] = 0;
  param_1[0x3a = byte +0xe8] = 0;
  param_1[0x3b = byte +0xec] = 0;
  param_1[10 = byte +0x28] = 0;
  param_1[0x3e = byte +0xf8] = 0;
  *(undefined1 *)(param_1 + 0x40) = 0;
  param_1[0x41 = byte +0x104] = 0;
  param_1[0x42 = byte +0x108] = 0;
  param_1[0x43 = byte +0x10c] = 0;
  param_1[0x44 = byte +0x110] = 0;
  param_1[0x45 = byte +0x114] = 0x3f800000;
  param_1[0x46 = byte +0x118] = 0;
  param_1[0x47 = byte +0x11c] = 0;
  param_1[0x59 = byte +0x164] = _DAT_00994784;
  param_1[0x5a = byte +0x168] = 0;
  param_1[0x5b = byte +0x16c] = 0;
  param_1[0x5c = byte +0x170] = 0;
  param_1[0x5d = byte +0x174] = 0;
  param_1[0x5e = byte +0x178] = 0;
  param_1[0x5f = byte +0x17c] = 0;
  param_1[0x60 = byte +0x180] = 0;
  param_1[0x61 = byte +0x184] = 0;
/* ... 45 more lines truncated by --max-lines */
```

### decoy: 0x006f99f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006f99f0: 16-byte aligned: YES
  006f99f0  55             PUSH EBP
  006f99f1  8B EC          MOV EBP,ESP
  006f99f3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006f99f0
string literals (0):
direct callees (5):
  0x009196FD  operator delete
  0x006FA2D0  (unnamed)
  0x0054F990  (unnamed)
  0x0054FA10  (unnamed)
  0x0054FA90  (unnamed)
struct offsets touched (top 10):
  +0x1AC [dec 428]  x4  via EAX,ECX,EDX
  +0xE8 [dec 232]  x3  via ECX,EDX
  +0xEC [dec 236]  x3  via EAX,ECX
  +0x16C [dec 364]  x3  via EAX,EDX
  +0x238 [dec 568]  x3  via EAX,ECX
  +0x90 [dec 144]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via ECX,EDX
  +0xAC [dec 172]  x2  via EAX,EDX
  +0xB0 [dec 176]  x2  via EAX,ECX
  +0xB8 [dec 184]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 1447 B, RET 0
- callers 1: 0x006f9200
- callees: 0x009196fd operator delete (CALL)
- twins: none

## Card B17-7-16

GOG function: Scene::DoGobShadows  gog 0x0085ab60  callconv=(blank)  param_size_bytes=0  gog_size=876  gog_retn=0
Candidate Steam address: 0x004686e0 (size 242, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.29) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004687e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0085aa60 Scene::DoBlurBuckets_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0085aed0 Scene::DoGobShadowsForThisLight  (not in this batch, no trusted mapping)
- server Steam name at this address: ::DoBlurBuckets

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
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x14,0x50,0x58,0x60,0x84,0x8c*,0xee,0x192,0x1b4} | Steam {0x20,0x38,0x8c*,0x14c,0x164}
imm sets (* = in both, 1 common): GOG {0x4,0x5,0x8,0x41,0x78,0x94,0xa4*,0xb4} | Steam {0x3,0x10,0xa4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004686e0: 16-byte aligned: YES
  004686e0  55             PUSH EBP
  004686e1  8B EC          MOV EBP,ESP
  004686e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004686e0
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0x8C [dec 140]  x6  via EAX,ECX,EDX
  +0x20 [dec 32]  x1  via EDX
  +0x38 [dec 56]  x1  via EDX
  +0x14C [dec 332]  x1  via EDX
  +0x164 [dec 356]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004686e0 FUN_004686e0 size=242 ret=0 callers=0 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004686e0(int param_1)

{
  undefined4 local_8;
  
  _DAT_00a20044 = DAT_00a7333c;
  for (local_8 = 0; local_8 < DAT_00a7333c; local_8 = local_8 + 1) {
    (*(code *)PTR_FUN_009f5d24)();
    (**(code **)(**(int **)(param_1 + 0x8c) + 0x38))
              (*(int *)(DAT_00a73338 + local_8 * 4) + 0xa4,
               *(undefined4 *)(*(int *)(DAT_00a73338 + local_8 * 4) + 0x164));
    (**(code **)**(undefined4 **)(param_1 + 0x8c))(*(int *)(DAT_00a73338 + local_8 * 4) + 0xa4,0);
    (**(code **)(**(int **)(param_1 + 0x8c) + 0x20))(3);
    (**(code **)(**(int **)(DAT_00a73338 + local_8 * 4) + 0x14c))();
  }
  return;
}
```

### decoy: 0x004687e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004687e0: 16-byte aligned: YES
  004687e0  55             PUSH EBP
  004687e1  8B EC          MOV EBP,ESP
  004687e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

### Static facts (script output, report-only)
- size 242 B, RET 0
- callees: 0x008339d0 (CALL)
- indirect: 0x00468761 slot 0x38, 0x0046878d slot 0x0, 0x004687a8 slot 0x20, 0x004687c7 slot 0x14c
- twins: none

## Card B17-7-17

GOG function: CExoStreamingSoundSourceInternal::SetLooping  gog 0x0063cc50  callconv=(blank)  param_size_bytes=4  gog_size=83  gog_retn=4
Candidate Steam address: 0x0070dcb0 (size 83, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0070d9d0:STR0/CALLEE0/SIG0.229/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063cc00 CExoStreamingSoundSourceInternal::IsDone -> 0x0070dc60  gate_class=REVIEW
- GOG-order neighbour after:  0x0063ccb0 CExoStreamingSoundSourceInternal::Stop  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0063ca40 CExoStreamingSoundSourceInternal::Update3DStream -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x0070dcb0: 16-byte aligned: YES
  0070dcb0  55             PUSH EBP
  0070dcb1  8B EC          MOV EBP,ESP
  0070dcb3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070dcb0
string literals (0):
direct callees (1):
  0x0070DAA0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0070dcb0 FUN_0070dcb0 size=83 ret=4 callers=1 callees=2

void __thiscall FUN_0070dcb0(undefined4 *param_1,undefined4 param_2)

{
  int iVar1;
  
  param_1[2 = byte +0x8] = param_2;
  iVar1 = FUN_0070daa0();
  if (iVar1 != 0) {
    _AIL_set_stream_loop_count_8 [GHIDRA-AUTO-LABEL: unreliable](*param_1,param_1[2 = byte +0x8] == 0);
  }
  return;
}
```

### decoy: 0x0070d9d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070d9d0: 16-byte aligned: YES
  0070d9d0  55             PUSH EBP
  0070d9d1  8B EC          MOV EBP,ESP
  0070d9d3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070d9d0
string literals (0):
direct callees (2):
  0x00709010  (unnamed)
  0x0070DFE0  (unnamed)
struct offsets touched (top 8):
  +0xC [dec 12]  x3  via EAX,ECX
  +0x24 [dec 36]  x2  via EAX,EDX
  +0x7 [dec 7]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x14A [dec 330]  x1  via ECX
  +0x14C [dec 332]  x1  via EAX
  +0x150 [dec 336]  x1  via ECX
  +0x154 [dec 340]  x1  via EDX
```

### Static facts (script output, report-only)
- size 83 B, RET 4
- callers 1: 0x0070b240
- callees: 0x0070daa0 (CALL), 0x00000081 (CALL)
- imports: _AIL_set_stream_loop_count@8 (MSS32.DLL)
- twins: none

## Card B17-7-18

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

## Card B17-7-19

GOG function: CVirtualMachineInternal::ExecuteCode_5  gog 0x00645160  callconv=(blank)  param_size_bytes=12  gog_size=89  gog_retn=0
Candidate Steam address: 0x0044b210 (size 89, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: CreateArrays 0x50ebc0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00449750:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0064461e Global::thunk_FUN_00645091  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006451c0 CVirtualMachineInternal::PopInstructionPtr -> 0x00701f20  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x0089da10 CAurTriangleBin::CreateArrays -> Steam 0x0050ebc0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 3 common): GOG {0xc*,0x10*,0x44*} | Steam {0xc*,0x10*,0x44*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0044b210: 16-byte aligned: YES
  0044b210  55             PUSH EBP
  0044b211  8B EC          MOV EBP,ESP
  0044b213  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0044b210
string literals (0):
direct callees (1):
  0x00436200  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0044b210 FUN_0044b210 size=89 ret=0 callers=5 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * FUN_0044b210(undefined4 *param_1,undefined4 *param_2,float param_3)

{
  if (param_3 == (float)_DAT_00986b38) {
    *param_1 = *param_2;
    param_1[1 = byte +0x4] = param_2[1 = byte +0x4];
    param_1[2 = byte +0x8] = param_2[2 = byte +0x8];
  }
  else {
    FUN_00436200(param_1,param_2,1.0 / param_3);
  }
  return param_1;
}
```

### decoy: 0x00449750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00449750: 16-byte aligned: YES
  00449750  55             PUSH EBP
  00449751  8B EC          MOV EBP,ESP
  00449753  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00449750
string literals (0):
direct callees (14):
  0x00449230  (unnamed)
  0x00449640  (unnamed)
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x00428F80  (unnamed)
  0x00451800  (unnamed)
  0x00429330  (unnamed)
  0x00451840  (unnamed)
  0x00452830  (unnamed)
  0x00432EE0  (unnamed)
  0x00474030  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x0044B210  (unnamed)
  0x00514340  (unnamed)
  0x00432F10  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x50  via EAX,ECX,EDX
  +0x4 [dec 4]  x47  via EAX,ECX,EDX
  +0xC [dec 12]  x32  via EAX,ECX,EDX
  +0x14C [dec 332]  x4  via ECX,EDX
  +0x19C [dec 412]  x4  via EAX,ECX
  +0xAC [dec 172]  x2  via EAX
  +0x154 [dec 340]  x2  via ECX
  +0x158 [dec 344]  x2  via EDX
  +0x180 [dec 384]  x2  via ECX
  +0x4C [dec 76]  x1  via ECX
```

### Static facts (script output, report-only)
- size 89 B, RET 0
- callers 5: 0x00449750, 0x0046ac20, 0x00471c30, 0x0050ebc0 CAurTriangleBin::CreateArrays, 0x006ff9f0 CVirtualMachineInternal::ExecuteCode
- callees: 0x00436200 (CALL)
- twins: none

## Card B17-7-20

GOG function: CExoSoundInternal::StopAllOneShots  gog 0x00638c70  callconv=(blank)  param_size_bytes=0  gog_size=262  gog_retn=0
Candidate Steam address: 0x00709cf0 (size 262, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00709be0:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00638b60 CExoSoundInternal::ApplyNewVolume  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00638d80 CExoSoundInternal::PauseAllSounds  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x00637850 CExoSoundInternal::RemoveFromPlayingList -> Steam ?  in candidate callees: n/a
  0x00639a10 CExoLinkedList_T_::Contains -> Steam ?  in candidate callees: n/a
  0x0063e8f0 CExoSoundSourceInternal::IsPlaying -> Steam ?  in candidate callees: n/a
  0x007c02e0 CExoLinkedList_CSWSPlayerLUOInventoryItem_::AddHead -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x28*} | Steam {0x28*}
imm sets (* = in both, 3 common): GOG {0x1c*,0x44*,0x4c*} | Steam {0x1c*,0x44*,0x4c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00709cf0: 16-byte aligned: YES
  00709cf0  55             PUSH EBP
  00709cf1  8B EC          MOV EBP,ESP
  00709cf3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00709cf0
string literals (0):
direct callees (8):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x007088D0  (unnamed)
  0x0070F950  Source::IsPlaying
  0x0070AA90  (unnamed)
  0x0054AA10  CExoLinkedList::AddHead
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00709cf0 FUN_00709cf0 size=262 ret=0 callers=1 callees=8

void FUN_00709cf0(void)

{
  int iVar1;
  int local_c;
  undefined4 *local_8;
  
  local_c = FUN_007a1720();
  local_8 = (undefined4 *)FUN_00521060();
  while (local_c != 0) {
    if (local_8 != (undefined4 *)0x0) {
      if (local_8[10 = byte +0x28] == 0) {
        local_8 = (undefined4 *)FUN_0058c370(&local_c);
      }
      else {
        local_c = FUN_007088d0(local_8);
        iVar1 = FUN_0070f950();
        if (iVar1 == 0) {
          iVar1 = FUN_0070aa90(local_8);
          if ((iVar1 == 0) && (local_8 != (undefined4 *)0x0)) {
            (**(code **)*local_8)(1);
          }
        }
        else {
          iVar1 = FUN_0070aa90(local_8);
          if (iVar1 == 0) {
            FUN_0054aa10(local_8);
          }
        }
        local_8 = (undefined4 *)0x0;
        if (local_c != 0) {
          local_8 = (undefined4 *)FUN_00561430(local_c);
        }
      }
    }
  }
  return;
}
```

### decoy: 0x00709be0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00709be0: 16-byte aligned: YES
  00709be0  55             PUSH EBP
  00709be1  8B EC          MOV EBP,ESP
  00709be3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00709be0
string literals (0):
direct callees (6):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0078EB80  (unnamed)
  0x0070DFE0  (unnamed)
  0x0058C370  (unnamed)
  0x0070FDC0  Source::ApplyVolume
struct offsets touched (top 3):
  +0x1C [dec 28]  x1  via EDX
  +0x8D [dec 141]  x1  via ECX
  +0x14A [dec 330]  x1  via ECX
```

### Static facts (script output, report-only)
- size 262 B, RET 0
- callers 1: 0x0070bb00
- callees: 0x007a1720 (CALL), 0x00521060 (CALL), 0x007088d0 (CALL), 0x0070f950 (CALL), 0x0070aa90 (CALL), 0x0054aa10 (CALL), ...
- indirect: 0x00709dac slot 0x0
- twins: none

## Card B17-7-21

GOG function: CSWGuiOptionsGraphicsAdvanced::OnTexQualLeft  gog 0x0056d040  callconv=(blank)  param_size_bytes=4  gog_size=84  gog_retn=4
Candidate Steam address: 0x007d7790 (size 94, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.88 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d7730:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0056cff0 CSWGuiOptionsGraphicsAdvanced::OnDefault  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0056d0a0 CSWGuiOptionsGraphicsAdvanced::OnTexQualRight  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0056d100 CSWGuiOptionsGraphicsAdvanced::SetTexQualText -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1c*,0x50*,0x2920*} | Steam {0x1c*,0x50*,0x2920*,0x2924}
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
prologue for 0x007d7790: 16-byte aligned: YES
  007d7790  55             PUSH EBP
  007d7791  8B EC          MOV EBP,ESP
  007d7793  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d7790
string literals (0):
direct callees (2):
  0x007D77F0  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 4):
  +0x2920 [dec 10528]  x3  via ECX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x2924 [dec 10532]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007d7790 FUN_007d7790 size=94 ret=4 callers=0 callees=2

void __thiscall FUN_007d7790(int param_1,int param_2)

{
  if ((*(int *)(param_2 + 0x50) != 0) && (*(int *)(param_1 + 0x2920) < *(int *)(param_1 + 0x2924)))
  {
    *(int *)(param_1 + 0x2920) = *(int *)(param_1 + 0x2920) + 1;
    FUN_007d77f0();
    FUN_004122a0(1);
  }
  return;
}
```

### decoy: 0x007d7730 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d7730: 16-byte aligned: YES
  007d7730  55             PUSH EBP
  007d7731  8B EC          MOV EBP,ESP
  007d7733  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d7730
string literals (0):
direct callees (2):
  0x007D77F0  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 3):
  +0x2920 [dec 10528]  x3  via EAX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
```

### Static facts (script output, report-only)
- size 94 B, RET 4
- callees: 0x007d77f0 (CALL), 0x004122a0 CSWGuiManager::PlayGuiSound (CALL)
- twins: none

## Card B17-7-22

GOG function: CAvoidCreature::ComputeHexPoints  gog 0x00648370  callconv=(blank)  param_size_bytes=12  gog_size=3646  gog_retn=12
Candidate Steam address: 0x006fa580 (size 3646, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: rotation:0x00649370:0.68>0.37
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: ComputeHeight 0x51ca40, GetArea 0x557ef0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006fa530:STR0/CALLEE0/SIG0.181/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00648320 CPathfindInformation::ComputeStepTolerance  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006491b0 CAvoidCreature::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x006491b0 CAvoidCreature::Constructor -> Steam ?  in candidate callees: n/a
  0x00649350 unnamed -> Steam ?  in candidate callees: n/a
  0x0064b7b0 CServerExoApp::GetModule -> Steam 0x0051bd90  in candidate callees: yes
  0x0064c460 CServerExoApp::ComputeHeight -> Steam 0x0051ca40  in candidate callees: yes
  0x0072a6a0 CSWSModule::GetArea -> Steam 0x00557ef0  in candidate callees: yes
  0x0080c170 Global::normalize -> Steam ?  in candidate callees: n/a
  0x00811430 Quaternion::Constructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 9 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x44*,0x104*,0x2c0*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x44*,0x104*,0x2c0*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x006fa580: 16-byte aligned: YES
  006fa580  55             PUSH EBP
  006fa581  8B EC          MOV EBP,ESP
  006fa583  81 EC C0 02 00 00 SUB ESP,0x2c0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fa580
string literals (0):
direct callees (8):
  0x005CC6D0  (unnamed)
  0x005143C0  (unnamed)
  0x005195A0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x0051BD90  CServerExoApp::GetModule
  0x00557EF0  CSWSModule::GetArea
  0x0051CA40  CServerExoApp::ComputeHeight
  0x006FB3C0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x29  via EAX,ECX,EDX
  +0x4 [dec 4]  x19  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006fa580 FUN_006fa580 size=3646 ret=12 callers=1 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void FUN_006fa580(undefined4 param_1,float *param_2,float param_3)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  float10 fVar4;
  int extraout_var;
  int iVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined1 local_c8 [12];
  float local_bc;
  float local_b8;
  float local_b4;
  undefined1 local_b0 [12];
  float local_a4;
  float local_a0;
  float local_9c;
  undefined1 local_98 [12];
  float local_8c;
  float local_88;
  float local_84;
  undefined1 local_80 [12];
  float local_74;
  float local_70;
  float local_6c;
  undefined1 local_68 [12];
  float local_5c;
  float local_58;
  float local_54;
  undefined1 local_50 [12];
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  float local_38;
  float local_34;
  float local_30;
  undefined4 local_2c;
  undefined4 local_28;
  float local_24;
  float local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  float local_c;
  float local_8;
  
  FUN_005cc6d0();
  local_38 = -*param_2;
  local_34 = -param_2[1 = byte +0x4];
  local_30 = -param_2[2 = byte +0x8];
  iVar5 = extraout_var;
  FUN_005143c0(&local_20,&local_38);
  param_3 = param_3 / _DAT_0099b5c8;
  local_20 = local_20 * param_3;
  local_1c = local_1c * param_3;
  local_18 = local_18 * param_3;
  local_44 = 0;
  local_40 = 0;
  local_3c = 0x3f800000;
  FUN_005195a0(0,0,0x3f800000,_DAT_0099b5dc);
  local_2c = 0;
  local_28 = 0;
  local_24 = 0.0;
  puVar1 = (undefined4 *)FUN_0044c8a0(local_50,param_1,&local_20);
  local_2c = *puVar1;
  local_28 = puVar1[1 = byte +0x4];
  local_24 = (float)puVar1[2 = byte +0x8];
  *(undefined4 *)(iVar5 + 4) = local_2c;
  *(undefined4 *)(iVar5 + 8) = local_28;
  *(float *)(iVar5 + 0xc) = local_24;
  if ((DAT_00a0f190 == 0) ||
     ((local_14 != (float)_DAT_00986b50 && (local_14 != (float)_DAT_00986d98)))) {
    local_5c = (1.0 - (local_8 * local_8 + local_c * local_c) * (float)_DAT_00987490) * local_20 +
/* ... 160 more lines truncated by --max-lines */
```

### decoy: 0x006fa530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fa530: 16-byte aligned: YES
  006fa530  55             PUSH EBP
  006fa531  8B EC          MOV EBP,ESP
  006fa533  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fa530
string literals (0):
direct callees (1):
  0x0091C860  _ftol2_sse
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x68 [dec 104]  x2  via EAX,ECX
  +0x6C [dec 108]  x2  via EDX
```

### Static facts (script output, report-only)
- size 3646 B, RET 12
- callers 1: 0x006fc5d0
- callees: 0x005cc6d0 (CALL), 0x005143c0 (CALL), 0x005195a0 (CALL), 0x0044c8a0 (CALL), 0x0051bd90 CServerExoApp::GetModule (CALL), 0x00557ef0 CSWSModule::GetArea (CALL), ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none
